/* -*- mode:C; coding:utf-8 -*- */

#include "../include/connector/connector.h"

int tor_server_start( struct sockaddr_in *server, int port ) {
    int fd;

    if ((fd=socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {
        printf("error en socket()\n");
        exit(-1);
    }

    server->sin_family = AF_INET;
    server->sin_port = htons(port);
    server->sin_addr.s_addr = INADDR_ANY;
    bzero(&(server->sin_zero),8);

    if (bind(fd,(struct sockaddr*)server, sizeof(struct sockaddr))==-1) {
        printf("error en bind() puerto %d\n", port);
        exit(-1);
    }

    if (listen(fd,MAX_CONNS) == -1) {
        printf("error en listen()\n");
        exit(-1);
    }
    return fd;
}

int tor_server_accept( struct sockaddr_in* server, int sfd ) {
    int sin_size, fd;
    struct sockaddr_in client;

    sin_size = sizeof(struct sockaddr_in);
    if ((fd = accept(sfd,(struct sockaddr *)&client, &sin_size))==-1) {
        printf("error en accept()\n");
        exit(-1);
    }
    return fd;
}
