#ifndef P2PNET_PEERS_H
#define P2PNET_PEERS_H

#include <stdint.h>
#include "quic/quic.h"

typedef enum {
    UNKNOWN_STATE = -1,
    PUNCHING,
    PUNCHED,
    CONNECTED,
    QUIC_CONNECTED
} p2p_peer_state;

typedef struct {
    int64_t last_attempt;
    int     punch_attempts;

    int64_t last_active;
    p2p_peer_state state;

    naddr_t addr;
} p2p_peer;

#endif
