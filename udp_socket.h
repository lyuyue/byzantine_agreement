#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

int socket_connect(char *hostname, int port, struct sockaddr_in *serveraddr) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        return -1;
    }

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

    return sockfd;
}

int socket_send(int sockfd, struct sockaddr_in *serveraddr) {
    int serverlen = sizeof(serveraddr);

    return 0;
}

int socket_recv(int sockfd, struct sockaddr_in *serveraddr) {
    int serverlen = sizeof(serveraddr);

    return 0;
}