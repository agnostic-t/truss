#ifndef P2PNET_SOCKET_H
#define P2PNET_SOCKET_H

#include "link/client.h"
#include "link/listing.h"
#include "quic/quic.h"
#include "events.h"
#include "threading/daemons.h"

typedef struct {
    uint8_t buf[1200];
    ssize_t size;
    nnet_fd from;
} p2p_sys_packet;

typedef struct {
    mt_eventsock new_spack;
    prot_queue   spackets;
    int64_t      last_req_time;

    prot_queue   events;
    mt_eventsock new_event;

    quic_core   *qcore;
    ln_socket   *psocket;
    link_client *lcli;
} p2p_sock_ctx;

typedef struct {
    p2p_sock_ctx *rctx;
    prot_array linking_servers;

    prot_table   known_peers; // naddr_t: p2p_peer
    mt_eventsock new_client;
    prot_queue   new_clients;

    mdaemon qcore_daemon;
    mdaemon custom_handler_daemon;
} p2p_sock;

int p2p_ctx_init(
    p2p_sock_ctx *ctx,

    quic_core   *qcore,
    ln_socket   *psock,
    link_client *lcli
);

int p2p_sock_init(p2p_sock *sock, p2p_sock_ctx *ctx);
int p2p_sock_run(p2p_sock *sock);

int p2p_sock_conn_link_serv(p2p_sock *sock, naddr_t link_server);
int p2p_sock_wait_event(p2p_sock *sock, int timeout);
int p2p_sock_poll_events(p2p_sock *sock, p2p_sevent *out_event);

int p2p_sock_send(p2p_sock *sock, naddr_t address, quic_pkt data);
int p2p_sock_wait(p2p_sock *sock, naddr_t address, int timeout);
int p2p_sock_recv(p2p_sock *sock, naddr_t address, quic_pkt *output);
int p2p_sock_recvany(p2p_sock *sock, quic_pkt *output);

int p2p_ctx_destroy(p2p_sock_ctx *ctx);
int p2p_sock_destroy(p2p_sock *sock);

#endif
