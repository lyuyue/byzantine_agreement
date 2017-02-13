#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "udp_socket.h"
#include "message.h"
#include "constant.h"

struct node *hostlist = NULL;

char *hostfile;
char *order;
int faulty = 0;
int port = 0;
int commander_id = LOCALHOST;
int sockfd = -1;

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
            order = argv[arg_itr];
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
    if (commander_id == LOCALHOST) {
       

        struct node *list_itr = hostlist;
        while (node != NULL) {

        }
        return 0; 
    }

    // Lieutenant


    return 0;
}
