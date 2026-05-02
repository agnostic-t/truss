#include <link/listing.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <threading/time.h>

int listing_init(listing_peers *list){
    if (!list) return -1;

    if (0 > prot_table_create(
        sizeof(naddr_t), sizeof(int64_t),
        DYN_OWN_BOTH, &list->connected_peers
    )) return -1;

    return 0;
}

int listing_add_peer(listing_peers *list, naddr_t conn_info){
    if (!list) return -1;

    int64_t ms_now = mt_time_get_millis();
    prot_table_set(&list->connected_peers, &conn_info, &ms_now);

    return 0;
}

int listing_remove_peer(listing_peers *list, naddr_t conn_info){
    if (!list) return -1;

    prot_table_remove(&list->connected_peers, &conn_info);
    return 0;
}

int listing_clear(listing_peers *list){
    if (!list) return -1;

    prot_table_end(&list->connected_peers);

    return 0;
}


int listing_random_pick(listing_peers *list, naddr_t *output){
    if (!list) return -1;

    prot_table_lock(&list->connected_peers);

    int rnd_inx = rand() % list->connected_peers.table.array.len;
    *output = *(naddr_t*)(
        ((dyn_pair*)dyn_array_at(&list->connected_peers.table.array, rnd_inx))->first
    );

    prot_table_unlock(&list->connected_peers);

    return 0;
}


int listing_serial(listing_peers *list, uint8_t **output, size_t *malloced){
    if (!list) return -1;

    prot_table_lock(&list->connected_peers);
    size_t el_size = sizeof(naddr_t) + sizeof(int64_t);
    *output = malloc(
        el_size * list->connected_peers.table.array.len
    );
    *malloced = el_size * list->connected_peers.table.array.len;

    for (size_t i = 0; i < list->connected_peers.table.array.len; i++){
        dyn_pair *pair = dyn_array_at(&list->connected_peers.table.array, i);

        memcpy((*output) + (el_size * i), pair->first, sizeof(naddr_t));
        memcpy((*output) + (sizeof(naddr_t) + el_size * i), pair->second, sizeof(int64_t));
    }

    prot_table_unlock(&list->connected_peers);

    return 0;
}

int listing_deserial(const uint8_t *input, size_t dsize, listing_peers *output, naddr_t self_addr){
    if (!input) return -1;

    size_t el_size = sizeof(naddr_t) + sizeof(int64_t);
    if (dsize % el_size != 0) {
        fprintf(stderr, "[listing] deserial is not possible, size is not X * el_size: %zu\n", dsize);
        return -1;
    }

    size_t entry_num = dsize / el_size;
    for (size_t i = 0; i < entry_num; i++){
        const uint8_t *curr_mem = input + el_size * i;

        naddr_t addr; int64_t was;
        memcpy(&addr, curr_mem, sizeof(addr));
        memcpy(&was, curr_mem + sizeof(naddr_t), sizeof(was));

        if (ln_addrcmp(&addr, &self_addr)) continue;
        prot_table_set(&output->connected_peers, &addr, &was);
    }

    return 0;
}
