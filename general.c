#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include "udp_socket.h"
#include "constants.h"

// List node of host info
struct node {
    int id; // host id
    struct sockaddr_in data;    // address info of this host
    struct node *next;  // pointer to next node
};

struct node *hostlist_head = NULL;  // head of host list

struct sockaddr_in hostlist[MAX_HOSTS]; // host address list
int hostlist_len = 0;   // host list length
char *hostfile; // hostfile route
char *order_txt; // original order
int faulty = 0; // maximum number of faulty tolerance
int port = 0;   // port number
int commander_id = LOCALHOST;   // commander host index in hostlist
int malicious_flag = 0; // if this flag is set to 1, this host is malicious

int self_id = 0;    // self host index in hostlist 
struct sockaddr_in self_sockaddr;   // self host address info
char self_hostname[BUF_SIZE];   // self hostname

int sockfd = -1;    // socket file descriptor
int round_n = 0;    // current round
int order = -1;     // order
socklen_t serverlen = ADDR_SIZE;    //  Size of struct sockaddr_in

int value_set[2];  // received value set
int multicast_list[MAX_HOSTS][MAX_HOSTS];   // multicast_list[i][j]: send ByzantineMessage to host j in round i
uint32_t multicast_order[MAX_HOSTS];    // multicast_order[i]: order to be sent in round i
uint32_t multicast_ids[MAX_HOSTS][MAX_HOSTS];   // multicast_ids[i][j]: the j-th id to be sent in round i
int multicast_listlen[MAX_HOSTS];   // multicast_listlen[i]: length of ids to be sent in round i

int optval = 1; // flag value for setsockopt

// Convert a string into int
int stoi(char *data) {
    int result = 0;
    for (int i = 0; i < strlen(data); i++) {
        int tmp = data[i] - 48;
        result = result * 10 + tmp;
    }
    return result;
}

// Make choice based on value_set
int choice() {
    if (value_set[1] == 1 && value_set[0] == 0) {
        return 1;
    }
    return 0;
}

// print the result based on choice() function
void print_result() {
    int result = choice();
    if (result == 0) {
        printf("%d: Agreed on retreat\n", self_id);
    } else {
        printf("%d: Agreed on attack\n", self_id);
    }
}

// fetch list of host from hostfile and build sockaddr_in
void get_hostlist() {
    hostlist_len = 0;

    char *line_buffer = (char *) malloc(sizeof(char) * BUF_SIZE);

    FILE *fp;
    fp = fopen(hostfile, "r");
    if (fp < 0) {
        perror("ERROR invalid hostfile");
        return;
    }
    
    while (fgets(line_buffer, BUF_SIZE, (FILE *) fp)) {
        *(line_buffer + strlen(line_buffer) - 1) = '\0';

        // ignore localhost
        if (strcmp(line_buffer, self_hostname) == 0) {
            self_id = hostlist_len;
            hostlist_len ++;
            continue;
        }

        // allocate space for new node
        struct node *tmp = (struct node *) malloc(sizeof(struct node));
        tmp->id = hostlist_len;
        tmp->next = NULL;

        int result = -1;
        printf("Waiting for %s\n", line_buffer);
        do {
            result = socket_init(line_buffer, port, &tmp->data);
        } while (result != 0);
        printf("Success\n");

        // insert new node into head of list
        tmp->next = hostlist_head;
        hostlist_head = tmp;

        hostlist_len ++; 
    }

    printf("Self-id: %d\n", self_id);
    printf("Length of host list: %d\n", hostlist_len);

    return;
}

void set_timeout() {
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("Error setting timeout");
    }
    return;
}

int main(int argc, char *argv[]) {
    // initialize
    bzero((char *) &hostlist[0], sizeof(struct sockaddr_in) * MAX_HOSTS);
    bzero((char *) &multicast_list[0][0], sizeof(int) * MAX_HOSTS * MAX_HOSTS);
    bzero((char *) &multicast_order[0], sizeof(uint32_t) * MAX_HOSTS);
    bzero((char *) &multicast_ids[0][0], sizeof(uint32_t) * MAX_HOSTS * MAX_HOSTS);
    bzero((char *) &multicast_listlen[0], sizeof(int) * MAX_HOSTS);
    srand(time(NULL));

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

        if (strcmp(argv[arg_itr], "-m") == 0) {
            arg_itr ++;
            malicious_flag = 1;
            continue;
        }
    }

    // build localhost sockaddr_in
    memset(&self_sockaddr, 0, sizeof(struct sockaddr_in));
    self_sockaddr.sin_family = AF_INET;
    self_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    self_sockaddr.sin_port = htons((unsigned short) port);

    gethostname(self_hostname, BUF_SIZE);
    printf("Hostname: %s\n", self_hostname);

    // build a socket and bind it to localhost
    sockfd = socket_connect(&self_sockaddr);
    if (sockfd == -1) {
        return -1;
    }

    get_hostlist();
    
    // receiving buffer
    char recv_buf[BUF_SIZE];

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
        // mark every one in the network as UNDELIVERED
        for (int i = 0; i < hostlist_len + 1; i++) {
            if (i == self_id) multicast_list[0][i] = DELIVERED;
            multicast_list[0][i] = UNDELIVERED;
        }

        value_set[order] = 1;
        set_timeout();

        printf("This is Commander.\n");
        struct ByzantineMessage *msg = (struct ByzantineMessage *) malloc(sizeof(struct ByzantineMessage) + sizeof(uint32_t));
        msg->type = 1;
        msg->size = (uint32_t) (BYZ_SIZE + UINT32_SIZE);
        msg->round_n = (uint32_t) 0;
        msg->order = (uint32_t) order;
        uint32_t *ids = (uint32_t *) ((struct ByzantineMessage *) msg + 1);
        *ids = (uint32_t) self_id;

        int tle_count = 0;  // total count of timeouts
        int resend_flag = 0; // a flag of UNDELIVERED ByzantineMessage

        do {
            struct node *hostlist_itr = hostlist_head;
            while (hostlist_itr != NULL) {
                if (malicious_flag) msg->order = (uint32_t) rand()%2;
                // bypass host where ByzantineMessage is delivered 
                if (multicast_list[0][hostlist_itr->id] == DELIVERED) {
                    hostlist_itr = hostlist_itr->next;
                    continue;
                }

                int bytes_sent = sendto(sockfd, (char *) msg, BYZ_SIZE + UINT32_SIZE, 0,
                        (struct sockaddr *) &hostlist_itr->data, serverlen);
                if (bytes_sent < 0) {
                    perror("ERROR send v_0");
                }
                printf("[BYZ_SEND] Round %d, send order %d to %d \n", 
                        msg->round_n, msg->order, hostlist_itr->id);
                
                int bytes_recv = recvfrom(sockfd, (char *)recv_buf, BUF_SIZE, 0,
                        (struct sockaddr *) &hostlist_itr->data, &serverlen);

                // Received Ack, reset timeout count
                if (bytes_recv > 0) {
                    printf("Received Ack\n");
                    tle_count = 0;
                    multicast_list[0][hostlist_itr->id] = DELIVERED;
                }

                hostlist_itr = hostlist_itr->next;
            }

            resend_flag = 0;
            for (int i = 0; i < hostlist_len; i++) {
                if (multicast_list[0][i] == UNDELIVERED) {
                    tle_count ++;
                    resend_flag = 1;
                    break;
                }
            }

            if (tle_count < MAX_TLE && resend_flag == 1) continue;

            // MAX_TLE count reached or all ByzantineMessage delivered
            break;

        } while (resend_flag == 0);

        print_result();

        return 0; 
    }

    // Lieutenant
    // if malicious, keep silent
    if (malicious_flag == 1) {
        return 0;
    }

    int tle_count = 0;
    for (int i = 0; i < MAX_HOSTS; i++) {
        multicast_list[0][i] = DELIVERED;
        multicast_list[i][commander_id] = DELIVERED;
    }
    multicast_list[0][commander_id] = UNDELIVERED;

    while (round_n < faulty + 1) {
        // set up socket timeout
        if (round_n == 1) set_timeout();

        printf("Round %d\n", round_n);

        struct node *hostlist_itr = hostlist_head;
        if (round_n > 0) {
            int msg_size = sizeof(struct ByzantineMessage) + multicast_listlen[round_n] * UINT32_SIZE;
            struct ByzantineMessage *cur_msg = (struct ByzantineMessage *) malloc(msg_size);
            cur_msg->type = (uint32_t) 1;
            cur_msg->size = (uint32_t) msg_size;
            cur_msg->round_n = (uint32_t) round_n;
            cur_msg->order = (uint32_t) multicast_order[round_n];

            uint32_t *ids_itr = (uint32_t *) ((struct ByzantineMessage*) cur_msg + 1);
            for (int i = 0; i < multicast_listlen[round_n]; i++) {
                *(ids_itr + i) = (uint32_t) multicast_ids[round_n][i];
            }

            for (int i = 0; i < 4 + multicast_listlen[round_n]; i++) {
                uint32_t *tmp = (uint32_t*) cur_msg;
                printf("%d ", *(tmp + i));
            }
            printf("\n");

            while (hostlist_itr != NULL) {
                if (hostlist_itr->id == commander_id || 
                    multicast_list[round_n][hostlist_itr->id] == DELIVERED) {
                    hostlist_itr = hostlist_itr->next;
                    continue;
                }

                int bytes_sent = sendto(sockfd, (char *) cur_msg, msg_size, 0,
                    (struct sockaddr *) &hostlist_itr->data, serverlen);
                if (bytes_sent < 0) {
                    perror("ERROR send ByzantineMessage");
                }
                printf("[BYZ_SEND] Round %d, send order %d to %d \n", 
                    cur_msg->round_n, cur_msg->order, hostlist_itr->id);
                hostlist_itr = hostlist_itr->next;
            }
        }

        hostlist_itr = hostlist_head;
        while (hostlist_itr != NULL) {
            if (round_n == 0 && hostlist_itr->id != commander_id) {
                hostlist_itr = hostlist_itr->next;
                continue;
            } 

            if (round_n > 0 && hostlist_itr->id == commander_id) {
                hostlist_itr = hostlist_itr->next;
                continue;
            }

            printf("Listening from %d\n", hostlist_itr->id);

            struct sockaddr_in *cur_addr = &hostlist_itr->data;
            int cur_id = hostlist_itr->id;

            bzero(recv_buf, BUF_SIZE);
            int bytes_recv = recvfrom(sockfd, recv_buf, BUF_SIZE, 0, (struct sockaddr *) cur_addr, &serverlen);
            if (bytes_recv < 0) {
                perror("ERROR receive msg");
            }

            uint32_t *msg_type = (uint32_t *) recv_buf;
            printf("Received something in round %d type %d\n", round_n, *msg_type);

            if (*msg_type == 1) {
                // ByzantineMessage
                struct ByzantineMessage *cur_msg = (struct ByzantineMessage *) recv_buf;

                if (round_n == 0 && cur_msg->round_n == 0)  multicast_list[cur_msg->round_n][cur_id] = DELIVERED;

                printf("[BYZ_RECV] Round %d, receive order %d from %d\n", cur_msg->round_n, cur_msg->order, hostlist_itr->id); 
                
                // TODO: send ACK;
                struct Ack *cur_ack = (struct Ack *) malloc(ACK_SIZE);
                cur_ack->type = ACK_TYPE;
                cur_ack->size = ACK_SIZE;
                cur_ack->round_n = round_n;

                int bytes_sent = sendto(sockfd, (char *) cur_ack, ACK_SIZE, 0, (struct sockaddr *) cur_addr, serverlen);
                if (bytes_sent < 0) {
                    perror("ERROR send Ack");
                }

                printf("[ACK_SEND] Round %d, send ACK to %d\n", cur_msg->round_n, cur_id);

                // ignore out-of-data msg
                if (cur_msg->round_n != round_n) continue;
                // ignore existing msg
                if (value_set[cur_msg->order] == 1) continue;

                value_set[cur_msg->order] = 1;
                multicast_order[cur_msg->round_n + 1] = cur_msg->order;
                uint32_t msg_size = cur_msg->size - (uint32_t) BYZ_SIZE;
                int ids_count = (int) msg_size / sizeof(uint32_t);

                cur_msg += 1;
                uint32_t *itr = (uint32_t *) cur_msg;
                for (int j = 0; j < ids_count; j++) {
                    multicast_list[round_n + 1][* (itr + j)] = DELIVERED;
                    multicast_ids[round_n + 1][j] = * (itr + j);
                }
                multicast_ids[round_n + 1][ids_count] = self_id;
                multicast_listlen[round_n + 1] = ids_count + 1;
            } 

            if (*msg_type == 2) {
                // Ack
                struct Ack *cur_ack = (struct Ack *) recv_buf;
                uint32_t ack_round_n = cur_ack->round_n;
                // mark the message sent to cur_id is DELIVERED
                multicast_list[ack_round_n][cur_id] = DELIVERED;
                printf("[ACK_RECV] Round %d, receive ack from %d\n", ack_round_n, hostlist_itr->id);
                // reset timeout count
                tle_count = 0;
            }

            hostlist_itr = hostlist_itr->next;
        }

        int multicast_flag = 0;

        if (round_n == 0 && multicast_list[0][commander_id] == DELIVERED) {
            round_n ++;
            tle_count = 0;
            continue;
        }

        // check if all Ack received
        for (int i = 0; i < hostlist_len; i++) {
            if (multicast_list[round_n][i] == UNDELIVERED && i != self_id) {
                tle_count ++;
                multicast_flag = 1;
                break;
            }
        }

        if (multicast_flag == 1 && tle_count < MAX_TLE) {
            continue;
        }

        // TODO: remove invalid addr

        round_n ++;
        tle_count = 0;
    }

    print_result();

    return 0;
}
