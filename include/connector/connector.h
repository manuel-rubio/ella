#if !defined __CONNECTOR_H
#define __CONNECTOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_CONNS 2

int tor_server_start( struct sockaddr_in *server, int port );
int tor_server_accept( struct sockaddr_in* server, int sfd );

#endif
