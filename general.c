#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "udp_socket.h"
#include "message.h"
#include "constant.h"

struct sockaddr_in hostlist[MAX_HOSTS];
int hostlist_len = 0;
char *hostfile;
char *order_txt;
int faulty = 0;
int port = 0;
int commander_id = LOCALHOST;
int self_id = 0;
struct sockaddr_in commander_sockaddr;
struct sockaddr_in self_sockaddr;
int sockfd = -1;
int round_n = 0;
int order = -1;
int value_set[2] = {0, 0};
int multicast_list[MAX_HOSTS][MAX_HOSTS];
uint32_t multicast_data[MAX_HOSTS];

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

int parse_byzantine(struct ByzantineMessage *data) {


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
        bzero(temp, sizeof(struct node));

        int result = socket_init(line_buffer, port, &hostlist[list_len]);
        
        if (result != 0) {
            perror("ERROR invalid hostname");
            continue;
        }

        if (self_sockaddr.sin_addr == hostlist[list_len].sin_addr) {
            self_id = list_len;
        }

        hostlist_len ++; 
    }

    return hostlist_len;
}

int main(int argc, char *argv[]) {
    // initialize
    bzeros((char *) &hostlist[0], sizeof(struct sockaddr_in) * MAX_HOSTS);
    bzeros((char *) &multicast_list[0][0], sizeof(int) * MAX_HOSTS * MAX_HOSTS);
    bzeros((char *) &multicast_data[0], sizeof(uint32_t) * MAX_HOSTS));

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
    sockfd = socket_connect();
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
    // } ByzantineMesage;

    if (commander_id == LOCALHOST) {
        struct ByzantineMesage *msg = (struct ByzantineMesage *) malloc(sizeof(struct ByzantineMesage) + sizeof(uint32_t));
        msg->type = 1;
        msg->size = sizeof(struct ByzantineMesage) + sizeof(uint32_t);
        msg->round = 0;
        msg->order = order;
        uint32_t *ids = (uint32_t) ((struct ByzantineMesage *) msg + 1);
        *ids = commander_id;

        char ack_buf[BUF_SIZE];

        for 
            reliable_send(sockfd, &hostlist->sockaddr, (char *) msg, ack_buf);
            list_itr = list_itr->next;
        }
        return 0; 
    }

    // Lieutenant
    char recv_buf[BUF_SIZE];
    int bytes_recv = 0;
    int commander_serverlen = sizeof(struct sockaddr_in);

    // round 0: receiving v_0
    bytes_recv = recvfrom(sockfd, recv_buf, BUF_SIZE, 0, (struct sockaddr *) commander_sockaddr, &commander_serverlen);
    if (bytes_recv < 0) {
        perror("ERROR recv v_0");
        return 0;
    }
    struct ByzantineMessage *msg = (struct ByzantineMessage *) recv_buf;
    uint32_t order_recv = msg->order;
    value_set[order_recv] = 1;

    while (round_n < faulty + 1) {
        if (round_n > 0) {
            for (int i = 0; i < hostlist_len; i++) {
                if (multicast_list)
            }
        }

        while () {
            for (int i = 0; i < hostlist_len; i++) {
                if (i == self_id) continue;
                bzero(recv_buf, BUF_SIZE);
                int serverlen = sizeof(struct sockaddr_in);
                int bytes_recv = recvfrom(sockfd, recv_buf, BUF_SIZE, 0, (struct sockaddr *) &hostlist[i], &serverlen);
                uint32_t *msg_type = (uint32_t *) recv_buf;
                if (*msg_type == 1) {
                    // TODO: send ACK;

                    // ByzantineMessage
                    struct ByzantineMessage *cur_msg = (struct ByzantineMessage *) recv_buf;
                    // ignore out-of-data msg
                    if (cur_msg->round_n < round_n) continue;
                    // ignore existing order
                    if (value_set[cur_msg->order] == 1) continue;

                    value_set[cur_msg->order] = 1;
                    multicast_data[round_n + 1] = cur_msg->order;
                    uint32_t msg_size = cur_msg->size - (uint32_t) sizeof(struct ByzantineMessage);
                    int ids_count = int (msg_size / sizeof(uint32_t));

                    cur_msg += 1;
                    uint32_t *itr = (uint32_t *) cur_msg;
                    for (int j = 0; j < ids_count; j++) {
                        multicast_list[round_n + 1][* (itr + j)] = 1;
                    }
                } 

                if (*msg_type == 2) {
                    // Ack
                    struct Ack *cur_ack = (struct Ack *) recv_buf;
                    uint32_t ack_round_n = cur_ack->round_n;
                    multicast_list[ack_round_n][i] = 0;

                }
            }
        }

        round_n ++;
    }

    int result = choice();


    return 0;
}
