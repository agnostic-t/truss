#include "quic/quic.h"
#include <link/client.h>
#include <netinet/in.h>
#include <p2pnet/socket.h>
#include <npunch/nat.h>

bool custom_packet_daemon(){
    // wait for new packet

    printf("[main] requesting linking in server %s:%u\n",
            ln_gip(&link_serv), ln_gport(&link_serv));

    link_client_ask(&lcli, "\x00REQ", link_serv);
    ln_wait_netfd(&sock.fd, POLLIN, -1);

    if (0 != link_client_recv(&lcli)){
        fprintf(stderr, "[main] failed to init link connection\n");
        link_client_end(&lcli);
        return -1;
    }

    if (lcli.known_list.connected_peers.table.array.len != 0){
        listing_random_pick(&lcli.known_list, &peer_addr);
        break;
    }

    sleep(1);


    printf("[main] will try to connect to %s:%u\n", ln_gip(&peer_addr), ln_gport(&peer_addr));
}

void read_udp_socket(ln_socket *sock, quic_core *core) {
    uint8_t buf[2048];
    struct sockaddr_storage from_addr;
    socklen_t addr_len = sizeof(from_addr);

    while (true) {
        ssize_t recv_len = recvfrom(sock->fd.rfd, buf, sizeof(buf), MSG_DONTWAIT,
                                    (struct sockaddr*)&from_addr, &addr_len);
        if (recv_len < 0) break; // EAGAIN
        if (recv_len == 0) continue;

        if (buf[0] == 0x00) {
            handle_custom_p2p_packet(buf + 1, recv_len - 1, &from_addr);
            /*
             * you need to add it to queue and notify event socket
             * in daemon upper you need to process this request
             */
        } else {
            uint64_t now = mt_time_get_micros_monocoarse();
            picoquic_incoming_packet(core->ctx, buf, (size_t)recv_len,
                                     (struct sockaddr*)&from_addr,
                                     (struct sockaddr*)&sock->addr,
                                     0, 0, now);
        }
    }
}

// make context structutre
bool custom_quic_core_daemon(void *_args){
    quic_core *core = _args;

    while (true){
        quic_core_senditer(core);

        if (quic_wait_recviter(core) > 0) {
            read_udp_socket(&sock, core);
        }
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
    naddr_t peer_addr;

    quic_core core;
    if (0 > quic_serv_init( &core, "./keys/cert.pem", "./keys/key.pem", &sock)){
        fprintf(stderr, "[qcore] failed to init\n");
        return -1;
    }

    // wait for connection
    // send/recv "hello"
    // listen for new connections


    link_client_end(&lcli);
    ln_usock_close(&sock);
}
