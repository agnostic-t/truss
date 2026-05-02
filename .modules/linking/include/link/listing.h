#ifndef LINK_LISTING_H
#define LINK_LISTING_H

#include <lownet/core.h>
#include <base/prot_table.h>

// listing for linking servers
// contains IP:PORT of connected clients
typedef struct {
    prot_table connected_peers;
} listing_peers;

int listing_init(listing_peers *list);
int listing_add_peer(listing_peers *list, naddr_t conn_info);
int listing_clear(listing_peers *list);

int listing_random_pick(listing_peers *list, naddr_t *output);
int listing_remove_peer(listing_peers *list, naddr_t conn_info);

int listing_serial  (listing_peers *list, uint8_t **output, size_t *malloced);
int listing_deserial(const uint8_t *input, size_t dsize, listing_peers *output, naddr_t self_addr);

#endif
