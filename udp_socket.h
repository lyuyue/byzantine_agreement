#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "message.h"
#include "constant.h"

typedef struct node {
    struct sockaddr_in sockaddr;
    struct node *next;
};

int socket_init(char *hostname, int port, struct sockaddr_in *serveraddr) {
    struct hostent *server;
    server = gethostbyname(hostname);
    if (! server) {
        perror(ERROR parsing hostname);
        return -1;
    }

    bzero((char *) serveraddr, sizeof(sockaddr_in));
    serveraddr->sin_family = AF_INET;
    bcopy(
        (char *) server->h_addr, 
        (char *) &serveraddr->sin_addr, 
        server->h_length
    );

    serveraddr->sin_port = htons(portno);

    return 0;
}

int socket_connect() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        return -1;
    }
    return sockfd;
}

int socket_send(int sockfd, struct sockaddr_in *serveraddr, char *buf, int msg_flag) {
    int serverlen = sizeof(serveraddr);
    int buf_size = strlen(buf);

    int bytes_sent = sendto(sockfd, buf, buf_size, 0, serveraddr, serverlen);
    if (bytes_sent < 0) {
        perror("ERROR sendto");   
        return -1;
    }

    return 0;
}

int socket_recv(int sockfd, struct sockaddr_in *serveraddr, char *data) {
    int serverlen = sizeof(serveraddr);

    return 0;
}

int reliable_send(int sockfd, struct sockaddr_in *serveraddr, char *send_buf, char *recv_buf, int msg_flag) {
    int buf_size = strlen(send_buf);
    int bytes_sent = 0;

    // send a packet
    do {
        bytes_sent = socket_send(sockfd, serveraddr, send_buf, msg_flag);
    } while (bytes_sent == -1);

    // wait for ack
    bzero((char *) recv_buf, BUF_SIZE);
    int bytes_recv = recvfrom(sockfd, recv_buf, BUF_SIZE, 0, (struct sockaddr *) serveraddr, &serverlen)

}