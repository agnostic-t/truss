#include "link/listing.h"
#include <link/server.h>
#include <stdio.h>
#include <sys/poll.h>
#include <threading/time.h>

int link_server_init(link_server *serv, ln_socket *sock, int64_t dead_timeout){
    if (!serv || !sock) return -1;

    serv->p_sock = sock;
    serv->dead_timeout = dead_timeout;

    if (0 > listing_init(&serv->listing)){
        fprintf(stderr, "[linkserv] failed to init listing\n");
        return -1;
    }

    return 0;
}

int link_server_end (link_server *serv){
    if (!serv) return -1;

    listing_clear(&serv->listing);
    return 0;
}

int link_server_iter(link_server *serv, int iter_timeout){
    if (!serv) return -1;

    int pr = ln_wait_netfd(&serv->p_sock->fd, POLLIN, iter_timeout);

    int64_t curr_ms = mt_time_get_millis();
    prot_table_lock(&serv->listing.connected_peers);
    for (size_t i = 0; i < serv->listing.connected_peers.table.array.len; ){
        dyn_pair *pair = dyn_array_at(&serv->listing.connected_peers.table.array, i);
        int64_t *time = pair->second;

        if (curr_ms - (*time) >= serv->dead_timeout) {
            naddr_t *addr = pair->first;

            printf("[db] removing %s:%u due timeout\n", ln_gip(addr), ln_gport(addr));
            dyn_array_remove(&serv->listing.connected_peers.table.array, i);
            continue;
        }

        i++;
    }
    prot_table_unlock(&serv->listing.connected_peers);

    if (pr <= 0) return pr;

    uint8_t buf[4]; nnet_fd from;
    ln_usock_recv(serv->p_sock, buf, 4, &from);

    if (strncmp((char*)buf, "\x00REQ", 4) == 0){

        naddr_t addr = ln_nfd2addr(&from);
        listing_add_peer(&serv->listing, addr);
        printf("[serv] new req from %s:%d\n", ln_gip(&addr), ln_gport(&addr));

        prot_table_lock(&serv->listing.connected_peers);
        printf("[db] listing size: %zu\n", serv->listing.connected_peers.table.array.len);
        prot_table_unlock(&serv->listing.connected_peers);

        uint8_t *output = NULL;
        size_t   malloced = 0;
        if (0 > listing_serial(&serv->listing, &output, &malloced)){
            ln_usock_send(serv->p_sock, "\x00ERR", 4, &from);
            return 2;
        }

        ln_usock_send(serv->p_sock, output, malloced, &from);
        free(output);
    } else {
        ln_usock_send(serv->p_sock, "\x00UNK", 4, &from);
    }

    return 1;
}
