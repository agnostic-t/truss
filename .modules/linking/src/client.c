#include "link/listing.h"
#include <link/client.h>
#include <stdio.h>

#include <lownet/udp_sock.h>

int link_client_init(link_client *cli, ln_socket *psock){
    if (!cli || !psock) return -1;

    cli->p_sock = psock;
    if (0 > listing_init(&cli->known_list)){
        fprintf(stderr, "[linkcli] failed to init listing\n");
        return -1;
    }

    cli->last_req_serv = (nnet_fd){0};
    return 0;
}

int link_client_end (link_client *cli){
    if (!cli) return -1;

    listing_clear(&cli->known_list);
    return 0;
}


int link_client_ask(link_client *cli, const uint8_t *request, size_t req_len, naddr_t link_serv){
    if (!cli) return -1;

    nnet_fd serv = ln_netfdq(&link_serv);
    cli->last_req_serv = serv;
    ln_usock_send(cli->p_sock, request, req_len, &serv);
    return 0;
}

int link_client_recv(link_client *cli, uint8_t *buf, ssize_t dsize, nnet_fd from){
    if (!cli) return -1;

    if (dsize <= 0) {
        fprintf(stderr, "[linkcli] failed to get message\n");
        return -1;
    }

    if (ln_nfd2hash(&from) != ln_nfd2hash(&cli->last_req_serv)){
        fprintf(stderr, "[linkcli] got message not from server\n");
        return 1;
    }

    size_t original = cli->known_list.connected_peers.table.array.len;
    if (0 > listing_deserial(buf, dsize, &cli->known_list, cli->p_sock->addr)){
        fprintf(stderr, "[linkcli] failed to deserial data to peers\n");
        return -1;
    }

    prot_table_lock(&cli->known_list.connected_peers);
    if (original != cli->known_list.connected_peers.table.array.len){
        printf(
            "[linkcli] database is now at size: %zu\n",
            cli->known_list.connected_peers.table.array.len
        );
    }
    prot_table_unlock(&cli->known_list.connected_peers);
    return 0;
}
