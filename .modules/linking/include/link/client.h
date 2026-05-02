#ifndef LINK_CLIENT_H
#define LINK_CLIENT_H

#include "listing.h"

typedef struct {
    ln_socket *p_sock;
    listing_peers known_list;

    nnet_fd last_req_serv;
} link_client;

int link_client_init(link_client *cli, ln_socket *psock);
int link_client_end (link_client *cli);

int link_client_ask(link_client *cli, const char *request, naddr_t link_serv);
int link_client_recv(link_client *cli);


#endif
