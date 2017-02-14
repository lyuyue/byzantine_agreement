#include <stdlib.h>
#include <stdio.h>

typedef struct {
    uint32_t type;      // must equal to 1
    uint32_t size;      // size of message in bytes
    uint32_t round_n;     // round number
    uint32_t order;     // the order (retreat = 0 and attack = 1)
} ByzantineMesage;

typedef struct {
    uint32_t type;
    uint32_t size;
    uint32_t round_n;
} Ack;