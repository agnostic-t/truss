#include "link/listing.h"
#include "quic/quic.h"
#include <link/client.h>
#include <netinet/in.h>
#include <p2pnet/socket.h>
#include <npunch/nat.h>
#include <argparse/parser.h>

int main(int argc, const char *argv[]){
    srand(time(NULL));

    ln_socket sock;
    ln_usock_new(&sock);

    stun_addr stuns[] = {
        {"stun.sipnet.ru", 3478},
        {"stun.ekiga.net", 3478}
    };

    naddr_t link_serv = ln_uniq("147.45.137.91", 9001);

    nat_type nt = nat_parallel_req(
        &sock, stuns, sizeof(stuns) / sizeof(stuns[0])
    );

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
    // printf("[main] enter other's ip:port:");
    // char ip[INET_ADDRSTRLEN];
    // int  port;
    // scanf("%[^:]:%d", ip, &port);
    // naddr_t peer_addr = ln_uniq(ip, port);
    naddr_t peer_addr;

    while (true) {
        printf(
            "[main] requesting linking in server %s:%u\n",
            ln_gip(&link_serv), ln_gport(&link_serv)
        );

        link_client_ask(&lcli, "REQ", link_serv);
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
    }

    link_client_end(&lcli);
    printf("[main] will try to connect to %s:%u\n", ln_gip(&peer_addr), ln_gport(&peer_addr));

    nnet_fd peer_nfd  = ln_netfdq(&peer_addr);
    for (int i = 0; i < 5; i++){
        ln_usock_send(&sock, "PCH", 3, &peer_nfd);

        if (0 >= ln_wait_netfd(&sock.fd, POLLIN, 1000)){
            fprintf(stderr, "[main][punch] failed to get anything from peer (%i/%i)\n", i + 1, 5);
            continue;
        }

        char buf[3];
        ln_usock_recv(&sock, buf, 3, &peer_nfd);
        if (strncmp(buf, "ACK", 3) != 0)
            ln_usock_send(&sock, "ACK", 3, &peer_nfd);
        break;
    }

    printf("[main] NAT punched\n");

    p2p_socket ps;
    if (0 > p2p_sock_create(
        &ps, &sock, peer_addr, sock.addr,
        "./keys/cert.pem",
        "./keys/key.pem"
    )){ fprintf(stderr, "[main] failed to initiate QUIC core\n"); return -1; }

    printf("[+] QUIC connecting to peer\n");
    p2p_sock_connect(&ps, -1);

    printf("[+] sending packet to peer\n");
    quic_pkt pkt = quic_packet(ps.stream_id, (uint8_t*)"hello", 5, true);
    p2p_sock_send(&ps, &pkt);
    quic_packet_free(&pkt);

    sleep(2);

    printf("[+] waiting packets from peer\n");
    quic_pkt recv_pkt;
    p2p_sock_wait(&ps, -1);
    p2p_sock_recv(&ps, &recv_pkt);

    printf("[+] got: %.*s\n", (int)recv_pkt.msg_len, (char*)recv_pkt.msg);
    quic_packet_free(&recv_pkt);

    p2p_sock_close(&ps);
    return 0;
}
