#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <string.h>
#include "udp_socket.h"
#include "constants.h"

struct sockaddr_in hostlist[MAX_HOSTS]; // host address list
int hostlist_len = 0;   // host list length
char *hostfile; // hostfile route
char *order_txt; // original order
int faulty = 0; // maximum number of faulty tolerance
int port = 0;   // port number
int commander_id = LOCALHOST;   // commander host index in hostlist
int self_id = 0;    // self host index in hostlist 
struct sockaddr_in self_sockaddr;   // self host address info
int sockfd = -1;    // socket file descriptor
int round_n = 0;    // current round
int order = -1;     // order
int value_set[2] = {0, 0};  // received value set
int multicast_list[MAX_HOSTS][MAX_HOSTS];   // multicast_list[i][j]: send ByzantineMessage to host j in round i
pthread_t multicast_tid[MAX_HOSTS][MAX_HOSTS];
uint32_t multicast_data[MAX_HOSTS];
int multicast_datalen[MAX_HOSTS];

int optval; // flag value for setsockopt

int stoi(char *data) {
    int result = 0;
    for (int i = 0; i < strlen(data); i++) {
        int tmp = data[i] - 48;
        result = result * 10 + tmp;
    }
    return result;
}

int choice() {
    if (value_set[1] == 1 && value_set[0] == 0) {
        return 1;
    }
    return 0;
}

int get_hostlist() {
    hostlist_len = 0;

    char *line_buffer = (char *) malloc(sizeof(char) * BUF_SIZE);

    FILE *fp;
    fp = fopen(hostfile, "r");
    if (fp < 0) {
        perror("ERROR invalid hostfile");
        return -1;
    }
    
    while (fgets(line_buffer, BUF_SIZE, (FILE *) fp)) {
        int result = socket_init(line_buffer, port, &hostlist[hostlist_len]);
        
        if (result != 0) {
            perror("ERROR invalid hostname");
            continue;
        }

        if (self_sockaddr.sin_addr.s_addr == hostlist[hostlist_len].sin_addr.s_addr) {
            self_id = hostlist_len;
        }

        hostlist_len ++; 
    }

    return hostlist_len;
}

int main(int argc, char *argv[]) {
    // initialize
    bzero((char *) &hostlist[0], sizeof(struct sockaddr_in) * MAX_HOSTS);
    bzero((char *) &multicast_list[0][0], sizeof(int) * MAX_HOSTS * MAX_HOSTS);
    bzero((char *) &multicast_data[0], sizeof(uint32_t) * MAX_HOSTS);
    bzero((char *) &multicast_tid[0][0], sizeof(pthread_t) * MAX_HOSTS * MAX_HOSTS);
    bzero((char *) &multicast_datalen[0], sizeof(int) * MAX_HOSTS);

    // parse arguments

    int arg_itr = 1;
    for (; arg_itr < argc; arg_itr ++) {
        if (strcmp(argv[arg_itr], "-p") == 0) {
            arg_itr ++;
            port = stoi(argv[arg_itr]);
            continue;
        }

        if (strcmp(argv[arg_itr], "-h") == 0) {
            arg_itr ++;
            hostfile = argv[arg_itr];
            continue;
        }

        if (strcmp(argv[arg_itr], "-f") == 0) {
            arg_itr ++;
            faulty = stoi(argv[arg_itr]);
            continue;
        }

        if (strcmp(argv[arg_itr], "-C") == 0) {
            arg_itr ++;
            commander_id = stoi(argv[arg_itr]);
            continue;
        }

        if (strcmp(argv[arg_itr], "-o") == 0) {
            arg_itr ++;
            order_txt = argv[arg_itr];
            if (strcmp(order_txt, "attack") == 0) {
                order = 1;
            } else {
                order = 0;
            }
            continue;
        }
    }

    memset(&self_sockaddr, 0, sizeof(struct sockaddr_in));
    self_sockaddr.sin_family = AF_INET;
    self_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    self_sockaddr.sin_port = htons(port);

    int host_count = get_hostlist();
    sockfd = socket_connect(&self_sockaddr);
    if (sockfd == -1) {
        return -1;
    }

    // Commander
    // Select a value v
    // Send v_0 to every lietenant

    // typedef struct {
    //     uint32_t type;      // must equal to 1
    //     uint32_t size;      // size of message in bytes
    //     uint32_t round_n;     // round number
    //     uint32_t order;     // the order (retreat = 0 and attack = 1)
    //     uint32_t ids[];     // ids of the senders of this message
    // } ByzantineMessage;

    if (commander_id == LOCALHOST) {
        printf("This is Commander.\n");
        struct ByzantineMessage *msg = (struct ByzantineMessage *) malloc(sizeof(struct ByzantineMessage) + sizeof(uint32_t));
        msg->type = 1;
        msg->size = sizeof(struct ByzantineMessage) + sizeof(uint32_t);
        msg->round_n = 0;
        msg->order = (uint32_t) order;
        uint32_t *ids = (uint32_t) ((struct ByzantineMessage *) msg + 1);
        *ids = (uint32_t) commander_id;

        char ack_buf[BUF_SIZE];

        for (int i = 0; i < hostlist_len; i ++) {
            if (self_id == i) continue;
            printf("[BYZ_SEND] Round %d, send order %d to %d \n", msg->round_n, msg->order, i);
            // TODO: send
        }
        return 0; 
    }

    // Lieutenant
    char recv_buf[BUF_SIZE];
    int bytes_recv = 0;
    int commander_serverlen = sizeof(struct sockaddr_in);

    struct ByzantineMessage *msg = (struct ByzantineMessage *) recv_buf;
    uint32_t order_recv = msg->order;
    value_set[order_recv] = 1;

    int tle_count = 0;

    while (round_n < faulty + 1) {
        if (round_n > 0) {
            for (int i = 0; i < hostlist_len; i++) {
                if (multicast_list[round_n - 1][i] == 0) continue;
                // TODO: pthread send ByzantineMessage

            }
        }

        for (int i = 0; i < hostlist_len; i++) {
            if (i == self_id) continue;
            bzero(recv_buf, BUF_SIZE);
            int serverlen = sizeof(struct sockaddr_in);
            int bytes_recv = recvfrom(sockfd, recv_buf, BUF_SIZE, 0, (struct sockaddr *) &hostlist[i], &serverlen);
            uint32_t *msg_type = (uint32_t *) recv_buf;
            if (*msg_type == 1) {
                // ByzantineMessage
                struct ByzantineMessage *cur_msg = (struct ByzantineMessage *) recv_buf;

                // TODO: send ACK;
                printf("[ACK_SEND] Round %d, send ACK to %d\n", cur_msg->round_n, i);

                // ignore out-of-data msg
                if (cur_msg->round_n < round_n) continue;
                // ignore existing order
                if (value_set[cur_msg->order] == 1) continue;

                printf("[BYZ_RECV] Round %d, receive order %d from ", cur_msg->round_n, cur_msg->order); 
                value_set[cur_msg->order] = 1;
                multicast_data[cur_msg->round_n + 1] = cur_msg->order;
                uint32_t msg_size = cur_msg->size - (uint32_t) sizeof(struct ByzantineMessage);
                int ids_count = (int) msg_size / sizeof(uint32_t);

                cur_msg += 1;
                uint32_t *itr = (uint32_t *) cur_msg;
                for (int j = 0; j < ids_count; j++) {
                    printf("%d ->", j);
                    multicast_list[round_n + 1][* (itr + j)] = 1;
                }
                printf("\n");
            } 

            if (*msg_type == 2) {
                // Ack
                struct Ack *cur_ack = (struct Ack *) recv_buf;
                uint32_t ack_round_n = cur_ack->round_n;
                multicast_list[ack_round_n][i] = 0;
                printf("[ACK_RECV] Round %d, receive ack from %d\n", ack_round_n, i);
            }
        }

        int multicast_flag = 0;

        // check if all Ack received
        for (int i = 0; i < hostlist_len; i++) {
            if (multicast_list[round_n][i] == 1) {
                tle_count ++;
                multicast_flag = 1;
                break;
            }
        }

        if (multicast_flag == 1 && tle_count < MAX_TLE) {
            continue;
        }

        round_n ++;
        tle_count = 0;
    }

    int result = choice();


    return 0;
}
