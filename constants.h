#define LOCALHOST       -1
#define BUF_SIZE        256
#define TIMEOUT_SEC     1
#define MAX_HOSTS       10
#define MAX_TLE         3
#define UNDELIVERED     1
#define DELIVERED       0
#define BYZ_SIZE        sizeof(struct ByzantineMessage)
#define ACK_SIZE        sizeof(struct Ack)
#define ADDR_SIZE       sizeof(struct sockaddr_in)
#define UINT32_SIZE     sizeof(uint32_t)

#define BYZ_TYPE        1
#define ACK_TYPE        2