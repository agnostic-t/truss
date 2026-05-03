#ifndef P2P_NET_PACKETS_H
#define P2P_NET_PACKETS_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define DATAGRAM_MAX_SIZE 1200

typedef struct {
    uint8_t data[DATAGRAM_MAX_SIZE];
    size_t  size;
} datagram;

int dg_make(uint8_t *data, size_t sz, datagram *out){
    if (sz > DATAGRAM_MAX_SIZE) return -1;

    memset(out->data, 0, DATAGRAM_MAX_SIZE);
    memcpy(out->data, data, sz);
    out->size = sz;

    return 0;
}

int dg_serial(datagram *dg, uint8_t *output, size_t *out_size){
    if (!dg || !output || !out_size) return -1;

    ;

    return 0;
}

int dg_deserial(datagram *out, uint8_t *input, size_t size){
    if (!out || !input) return -1;

    ;

    return 0;
}

#endif
