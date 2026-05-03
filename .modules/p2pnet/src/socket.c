#include "link/client.h"
#include "link/listing.h"
#include "p2pnet/events.h"
#include "p2pnet/peers.h"
#include "quic/quic.h"
#include <p2pnet/socket.h>

int p2p_ctx_init(
    p2p_sock_ctx *ctx,

    quic_core   *qcore,
    ln_socket   *psock,
    link_client *lcli
){
    if (!ctx) return -1;

    ctx->qcore = qcore;
    ctx->psocket = psock;
    ctx->lcli = lcli;
    ctx->last_req_time = 0;

    if (0 > mt_evsock_new(&ctx->new_spack)) return -1;
    if (0 > mt_evsock_new(&ctx->new_event)) return -1;
    if (0 > prot_queue_create(sizeof(p2p_sys_packet), &ctx->spackets)) return -1;
    if (0 > prot_queue_create(sizeof(p2p_sevent), &ctx->events)) return -1;

    return 0;
}

int p2p_sock_init(p2p_sock *sock, p2p_sock_ctx *ctx){
    if (!sock) return -1;

    sock->rctx = ctx;

    if (0 > mt_evsock_new(&sock->new_client)) return -1;
    if (0 > prot_array_create(sizeof(naddr_t), &sock->linking_servers)) return -1;
    if (0 > prot_table_create(sizeof(naddr_t), sizeof(p2p_peer), DYN_OWN_BOTH, &sock->known_peers)) return -1;
    if (0 > prot_queue_create(sizeof(naddr_t), &sock->new_clients)) return -1;

    return 0;
}

int p2p_ctx_destroy(p2p_sock_ctx *ctx){
    if (!ctx) return -1;

    mt_evsock_close(&ctx->new_spack);
    mt_evsock_close(&ctx->new_event);
    prot_queue_end(&ctx->spackets);
    prot_queue_end(&ctx->events);

    return 0;
}

int p2p_sock_destroy(p2p_sock *sock){
    if (!sock) return -1;

    mt_evsock_close(&sock->new_client);
    prot_queue_end(&sock->new_clients);
    prot_array_end(&sock->linking_servers);
    prot_table_end(&sock->known_peers);

    return 0;
}

static bool _qcore_daemon(void *_args);
static bool _custom_handler_daemon(void *_args);
int p2p_sock_run(p2p_sock *sock){
    ;
}

int p2p_sock_conn_link_serv(p2p_sock *sock, naddr_t link_server){
    if (!sock) return -1;

    // add to array address

    return 0;
}

int p2p_sock_wait_event(p2p_sock *sock, int timeout){
    if (!sock) return -1;

    // mt_evsock_wait with timeout

    return 0;
}

int p2p_sock_poll_events(p2p_sock *sock, p2p_sevent *out_event){
    if (!sock) return -1;

    // if any events present - queue_pop

    return 0;
}


int p2p_sock_send(p2p_sock *sock, naddr_t address, quic_pkt data){
    if (!sock) return -1;

    // ???

    return 0;
}

int p2p_sock_wait(p2p_sock *sock, naddr_t address, int timeout){
    if (!sock) return -1;

    // ???

    return 0;
}

int p2p_sock_recv(p2p_sock *sock, naddr_t address, quic_pkt *output){
    if (!sock) return -1;

    // ???

    return 0;
}

int p2p_sock_recvany(p2p_sock *sock, quic_pkt *output){
    if (!sock) return -1;

    // ???

    return 0;
}

// statics

static bool punch_nat_iter(p2p_sock_ctx *ctx, const p2p_sys_packet *spack){
    ;
}

static bool _qcore_daemon(void *_args){
    p2p_sock_ctx *ctx = _args;
}

static bool _custom_handler_daemon(void *_args){
    p2p_sock_ctx *ctx = _args;
}
