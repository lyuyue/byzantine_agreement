#include <stdlib.h>
#include <stdio.h>

struct ByzantineMessage {
    uint32_t type;      // must equal to 1
    uint32_t size;      // size of message in bytes
    uint32_t round_n;     // round number
    uint32_t order;     // the order (retreat = 0 and attack = 1)
};

struct Ack{
    uint32_t type;
    uint32_t size;
    uint32_t round_n;
};