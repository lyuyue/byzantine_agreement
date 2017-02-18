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

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        return -1;
    }

    if (bind(sockfd, (struct sockaddr *) self_sockaddr, sizeof(struct sockaddr_in)) < 0) {
        perror("ERROR bind socket");
        return -1;
    }

    return sockfd;
}