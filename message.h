#include <stdlib.h>
#include <stdio.h>

typedef struct ByzantineMesage {
    uint32_t type;      // must equal to 1
    uint32_t size;      // size of message in bytes
    uint32_t round;     // round number
    uint32_t order;     // the order (retreat = 0 and attack = 1)
    uint32_t ids[];     // ids of the senders of this message
};

typedef struct Ack {
    uint32_t type;      // must be equal to 2
    uint32_t type;      // size of message in byte
    uint32_t round;     // round number
};