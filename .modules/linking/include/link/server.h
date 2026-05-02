#ifndef LINK_SERVER_H
#define LINK_SERVER_H

#include <lownet/udp_sock.h>
#include "listing.h"

typedef struct {
    ln_socket     *p_sock;
    listing_peers  listing;
    int64_t        dead_timeout;
} link_server;

int link_server_init(link_server *serv, ln_socket *sock, int64_t dead_timeout);
int link_server_end (link_server *serv);

int link_server_iter(link_server *serv, int iter_timeout);

#endif
