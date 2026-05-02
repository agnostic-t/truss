#include "quic/quic.h"
#include "threading/daemons.h"
#include <p2pnet/socket.h>

static bool _quic_core_daemon(void *_args){
    quic_core *core = _args;

    while (quic_core_running(core)){
        quic_core_senditer(core);

        if (quic_wait_recviter(core) > 0) {
            quic_core_recviter(core);
        }
    }

    return false;
}

int p2p_sock_create(
    p2p_socket *sock,
    ln_socket  *usock,
    naddr_t     peer_addr,
    naddr_t     my_addr,

    const char *path_to_certificate,
    const char *path_to_private_key
){
    if (!sock) return -1;
    memset(sock, 0, sizeof(p2p_socket));

    nnet_fd nfd1 = ln_netfdq(&peer_addr);
    nnet_fd nfd2 = ln_netfdq(&my_addr);

    sock->p_sock = usock;
    sock->is_qserver = ln_nfd2hash(&nfd1) > ln_nfd2hash(&nfd2);
    printf("[p2p] choosen as %s\n", sock->is_qserver ? "server": "client");

    sock->stream_id = sock->is_qserver ? 1 : 0;

    if (sock->is_qserver){
        printf("[p2p] current server address: %s:%d\n", ln_gip(&usock->addr), ln_gport(&usock->addr));
        if (0 > quic_serv_init(
            &sock->qcore,
            path_to_certificate,
            path_to_private_key,
            sock->p_sock)
        ){ return -1; }
    } else {

        sock->p_sock->addr = peer_addr;
        sock->p_sock->fd.addr = nfd1.addr;
        sock->p_sock->fd.addr_len = nfd1.addr_len;
        // ln_netfd(&sock->p_sock->addr, &sock->p_sock->fd);

        if (0 > quic_cli_init(
            &sock->qcore,
            sock->p_sock)
        ){ return -1; }
    }

    return 0;
}

int p2p_sock_close(p2p_socket *sock){
    quic_core_wait_done(&sock->qcore, -1);
    quic_core_stop(&sock->qcore);
    daemon_stop(&sock->daemon);
    quic_core_clear(&sock->qcore);

    return 0;
}

int p2p_sock_connect(p2p_socket *sock, int timeout){
    if (!sock->is_qserver){
        if (0 > quic_cli_start(&sock->qcore)){
            return -1;
        }
    }

    daemon_run(&sock->daemon, false, _quic_core_daemon, &sock->qcore);
    return quic_wait_session(&sock->qcore, &sock->session, timeout);
}


int p2p_sock_send(p2p_socket *sock, const quic_pkt *pkt){
    return quic_send(&sock->qcore, sock->session, pkt);
}

int p2p_sock_recv(p2p_socket *sock, quic_pkt *pkt){
    return quic_recv(sock->session, pkt);
}

int p2p_sock_wait(p2p_socket *sock, int timeout){
    return quic_wait_incpkt(&sock->qcore, timeout);
}
