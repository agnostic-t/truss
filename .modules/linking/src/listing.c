#include <link/listing.h>

int listing_init(listing_peers *list){
    if (!list) return -1;

    return 0;
}

int listing_add_peer(listing_peers *list, naddr_t conn_info){
    if (!list) return -1;

    return 0;
}

int listing_clear(listing_peers *list){
    if (!list) return -1;

    return 0;
}


int listing_random_pick(listing_peers *list, naddr_t *output){
    if (!list) return -1;

    return 0;
}


int listing_serial(const listing_peers *list, uint8_t **output, size_t *malloced){
    if (!list) return -1;

    return 0;
}

int listing_deserial(const uint8_t *input, size_t dsize, listing_peers *output){
    if (!input) return -1;

    return 0;
}
