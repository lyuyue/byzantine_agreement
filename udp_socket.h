#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include "message.h"
#include "constants.h"

int socket_init(char *hostname, int portno, struct sockaddr_in *serveraddr) {
    struct hostent *server;
    server = gethostbyname(hostname);
    if (! server) {
        perror("ERROR invalid server");
        return -1;
    }

    bzero((char *) serveraddr, sizeof(struct sockaddr_in));
    serveraddr->sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &serveraddr->sin_addr, server->h_length);

    serveraddr->sin_port = htons(portno);

    return 0;
}

int socket_connect(struct sockaddr_in *self_sockaddr) {
    struct timeval tv;
    tv.tv_sec = 1;

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        return -1;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("Error setting timeout");
        return -1;
    }

    if (bind(sockfd, (struct sockaddr *) self_sockaddr, sizeof(struct sockaddr_in)) < 0) {
        perror("ERROR bind socket");
        return -1;
    }

    return sockfd;
}

int socket_send(int sockfd, struct sockaddr_in *serveraddr, char *buf, int buf_size) {
    int serverlen = sizeof(struct sockaddr_in);

    int bytes_sent = sendto(sockfd, buf, buf_size, 0, serveraddr, serverlen);
    if (bytes_sent < 0) {
        perror("ERROR sendto");   
        return -1;
    }

    return 0;
}

int socket_recv(int sockfd, struct sockaddr_in *serveraddr, char *recv_buf) {
    int serverlen = sizeof(struct sockaddr_in);
    int bytes_recv = recvfrom(sockfd, recv_buf, BUF_SIZE, 0, (struct sockaddr *) serveraddr, &serverlen);
    return 0;
}

// int reliable_send(int sockfd, struct sockaddr_in *serveraddr, char *send_buf, int send_buf_size char *recv_buf) {
//     int buf_size = strlen(send_buf);
//     int bytes_sent = 0;
//     int bytes_recv = 0;
//     int tle_count = 0;

//     // send a packet
//     while (tle_count < 3) {
//         bytes_sent = socket_send(sockfd, serveraddr, send_buf, send_buf_size);
//         if (byte_sent == -1) {
//             perror("ERROR ByzantineMessage send");
//         }

//         struct timeval tv;
//         tv.tv_sec = TIMEOUT_SEC;

//         if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
//             perror("ERROR set recv_timeout");
//         }

//         // wait for ack
//         bzero((char *) recv_buf, BUF_SIZE);
//         bytes_recv = recvfrom(sockfd, recv_buf, BUF_SIZE, 0, (struct sockaddr *) serveraddr, &serverlen);
//         if (bytes_recv < 0) {
//             tle_count ++;
//             continue;
//         }

//         struct Ack *ack_msg = (struct Ack *) recv_buf;

//     }
// }

// int ack_send(int sockfd, struct sockaddr_in *serveraddr, int round_n) {
//     // typedef struct {
//     //     uint32_t type;  // must equal to 2
//     //     uint32_t size;  // size of message in bytes
//     //     uint32_t round_n; // round number
//     // } Ack;

//     struct Ack *msg = (struct Ack *) malloc(sizeof(struct Ack));
//     msg->type = 2;
//     msg->size = sizeof(struct Ack);
//     msg->round_n = round_n;

//     int serverlen = sizeof(struct sockaddr_in)

//     int bytes_sent = 0;
//     do {
//        bytes_sent = sendto(sockfd, (char *) msg, data[1], 0, serveraddr, &serverlen);
//     } while (bytes_sent > 0);

//     if (bytes_sent < 0) {
//         perror("ERROR Ack send");
//         return -1;
//     }

//     return 0;
// }