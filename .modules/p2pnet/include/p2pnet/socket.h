#ifndef P2PNET_SOCKET_H
#define P2PNET_SOCKET_H

#include <stdbool.h>
#include <quic/quic.h>
#include <lownet/udp_sock.h>
#include <threading/daemons.h>

/*
 * QUIC-wrapping structure
 *
 * abstracts server and client to just client
 */
typedef struct {
    bool is_qserver;
    mdaemon daemon;

    int stream_id;
    quic_core qcore;
    quic_session *session;

    ln_socket *p_sock;
} p2p_socket;

int p2p_sock_create(
    p2p_socket *sock,
    ln_socket  *usock,
    naddr_t     peer_addr,
    naddr_t     my_addr,

    const char *path_to_certificate,
    const char *path_to_private_key
);

int p2p_sock_close(p2p_socket *sock);
int p2p_sock_connect(p2p_socket *sock, int timeout);

int p2p_sock_send(p2p_socket *sock, const quic_pkt *pkt);
int p2p_sock_recv(p2p_socket *sock, quic_pkt *pkt);
int p2p_sock_wait(p2p_socket *sock, int timeout);

#endif
