#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "udp_socket.h"
#include "message.h"
#include "constant.h"

struct node *hostlist = NULL;

char *hostfile;
char *order_txt;
int faulty = 0;
int port = 0;
int commander_id = LOCALHOST;
int sockfd = -1;
int round_n = 0;
int order = -1;
int value_set[2] = {0, 0};

int stoi(char *data) {
    int result = 0;
    for (int i = 0; i < strlen(data); i++) {
        int tmp = data[i] - 48;
        result = result * 10 + tmp;
    }
    return result;
}

int choice() {
    return 0;
}

int get_hostlist(char *hostfile) {
    int list_len = 0;

    char *line_buffer = (char *) malloc(sizeof(char) * BUF_SIZE);

    FILE *fp;
    fp = fopen(hostfile, "r");
    if (fp < 0) {
        perror("ERROR invalid hostfile");
        return NULL;
    }
    
    while (fgets(line_buffer, BUF_SIZE, (FILE *) fp)) {
        struct node *temp = (struct node *) malloc(sizeof(struct node));
        bzero(temp, sizeof(struct node));

        int result = socket_init(line_buffer, port, temp->sockaddr);
        
        if (result != 0) {
            perror("ERROR invalid hostname");
            continue;
        }

        list_len ++; 

        if (! hostlist) {
            hostlist = temp;
            continue;
        }

        temp->next = hostlist->next;
        hostlist->next = temp;
    }

    return list_len;
}

int main(int argc, char *argv[]) {
    // initialize
    bzeros((char *) &sockfds[0], sizeof(int) * MAX_HOSTS);

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
    //     uint32_t round;     // round number
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

        struct node *list_itr = hostlist;
        while (list_itr != NULL) {
            reliable_send(sockfd, &hostlist->sockaddr, send_buf, ack_buf);
            list_itr = list_itr->next;
        }
        return 0; 
    }

    // Lieutenant

    while (round_n < faulty + 1) {

    }

    int result = choice();


    return 0;
}
