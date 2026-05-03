#include "quic/quic.h"
#include "threading/daemons.h"
#include <link/client.h>
#include <netinet/in.h>
#include <p2pnet/socket.h>
#include <npunch/nat.h>

typedef struct {
    quic_core   *core;
    link_client *lcli;
    ln_socket   *psock;
    naddr_t link_server;

    mt_eventsock new_pkt;
    prot_queue   sys_packets;
} run_context;

typedef struct {
    uint8_t buf[1200];
    ssize_t size;
    nnet_fd from;
} sys_packet;

bool custom_packet_daemon(void *_args){
    run_context *ctx = _args;

    printf("[cust_pkt_daemon] requesting linking in server %s:%u\n",
            ln_gip(&ctx->link_server), ln_gport(&ctx->link_server));

    if (0 > link_client_ask(ctx->lcli, "\x00REQ", 4, ctx->link_server)){
        fprintf(stderr, "[cust_pkt_daemon] failed to make request\n");
        return false;
    }

    // wait for new packet
    if (mt_evsock_wait(&ctx->new_pkt, -1) <= 0){
        fprintf(stderr, "[cust_pkt_daemon] failed to wait packets\n");
        return false;
    }

    sys_packet spack;
    prot_queue_pop(&ctx->sys_packets, &spack);

    if (0 != link_client_recv(ctx->lcli, spack.buf, spack.size, spack.from)){
        fprintf(stderr, "[cust_pkt_daemon] failed to init link connection\n");
        return true;
    }

    naddr_t peer_addr;
    if (ctx->lcli->known_list.connected_peers.table.array.len != 0){
        listing_random_pick(&ctx->lcli->known_list, &peer_addr);
    }

    sleep(1);

    printf("[main] will try to connect to %s:%u\n", ln_gip(&peer_addr), ln_gport(&peer_addr));
    return true;
}

void read_udp_socket(run_context *ctx) {
    uint8_t buf[2048];
    struct sockaddr_storage from_addr;
    socklen_t addr_len = sizeof(from_addr);

    while (true) {
        ssize_t recv_len = recvfrom(ctx->psock->fd.rfd, buf, sizeof(buf), MSG_DONTWAIT,
                                    (struct sockaddr*)&from_addr, &addr_len);
        if (recv_len < 0) break; // EAGAIN
        if (recv_len == 0) continue;

        if (buf[0] == 0x00) {
            nnet_fd from_fd = {
                .addr = from_addr,
                .addr_len = addr_len,
                .rfd = ctx->psock->fd.rfd
            };
            sys_packet spack = {
                .from = from_fd,
                .size = recv_len - 1
            };
            memcpy(spack.buf, buf + 1, spack.size);

            prot_queue_push(&ctx->sys_packets, &spack);
            mt_evsock_notify(&ctx->new_pkt);
        } else {
            uint64_t now = mt_time_get_micros_monocoarse();
            picoquic_incoming_packet(ctx->core->ctx, buf, (size_t)recv_len,
                                     (struct sockaddr*)&from_addr,
                                     (struct sockaddr*)&ctx->psock->addr,
                                     0, 0, now);
        }
    }
}

// make context structutre
bool custom_quic_core_daemon(void *_args){
    run_context *ctx = _args;

    while (true){
        quic_core_senditer(ctx->core);

        if (quic_wait_recviter(ctx->core) > 0)
            read_udp_socket(ctx);
    }
}

int main(){
    srand(time(NULL));

    ln_socket sock;
    ln_usock_new(&sock);

    naddr_t link_serv = ln_uniq("147.45.137.91", 9001);
    stun_addr stuns[] = {
        {"stun.sipnet.ru", 3478},
        {"stun.ekiga.net", 3478}
    };

    nat_type nt = nat_parallel_req(&sock, stuns, sizeof(stuns) / sizeof(stuns[0]));

    if (nt == NAT_ERROR){
        fprintf(stderr, "[main] failed to resolve NAT type\n");
        ln_usock_close(&sock);
        return -1;
    }

    printf("[main] NAT type: %s\n", strnattype(nt));
    if (nt == NAT_SYMMETRIC){
        fprintf(stderr, "[main] P2P is impossible when NAT is symmetric\n");
        ln_usock_close(&sock);
        return -1;
    }

    link_client lcli;
    link_client_init(&lcli, &sock);

    printf("[main] my address: %s:%u\n", ln_gip(&sock.addr), ln_gport(&sock.addr));

    quic_core core;
    if (0 > quic_serv_init( &core, "./keys/cert.pem", "./keys/key.pem", &sock)){
        fprintf(stderr, "[qcore] failed to init\n");
        return -1;
    }

    run_context ctx;
    ctx.link_server = link_serv;
    ctx.psock = &sock;
    ctx.lcli = &lcli;
    ctx.core = &core;

    if (0 > mt_evsock_new(&ctx.new_pkt)){
        fprintf(stderr, "[main] failed to init mt_eventsock\n");
        return -1;
    }

    if (0 > prot_queue_create(sizeof(sys_packet), &ctx.sys_packets)){
        fprintf(stderr, "[main] failed to init prot_queue for sys packets\n");
        return -1;
    }

    mdaemon custom_sys, custom_qcore;
    daemon_run(&custom_sys, true, custom_packet_daemon, &ctx);
    daemon_run(&custom_qcore, false, custom_quic_core_daemon, &ctx);

    sleep(1000);

    daemon_stop(&custom_sys);
    daemon_stop(&custom_qcore);

    quic_core_stop(&core);

    // wait for connection
    // send/recv "hello"
    // listen for new connections


    link_client_end(&lcli);
    ln_usock_close(&sock);
}
