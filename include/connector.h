/* -*- mode:C; coding:utf-8 -*- */

#if !defined __CONNECTOR_H
#define __CONNECTOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include "configurator.h"
#include "header.h"
#include "modules.h"

struct Host_Alias {
    char alias[80];
    struct Host_Alias* next;
};

typedef struct Host_Alias hostAlias;

struct Host_Location {
    char base_uri[512];
    configDetail* details;
    struct Host_Location*  next;
};

typedef struct Host_Location hostLocation;

struct Virtual_Host {
    char host_name[80];
    struct Host_Location* locations;
    struct Host_Alias* aliases;
    struct Virtual_Host* next;
};

typedef struct Virtual_Host virtualHost;

struct Bind_Connect {
    char host[80];
    int port;
    pthread_t thread;
    struct Module* modules;
    struct Virtual_Host* vhosts;
    struct Bind_Connect* next;
};

typedef struct Bind_Connect bindConnect;

struct Bind_Request {
    bindConnect* bc;
    requestHTTP* request;
    pthread_t thread;
    struct sockaddr_in client;
    int fd_client;
};

typedef struct Bind_Request bindRequest;

extern char bindThreadExit;

void* tor_connector_launch( void* ptr_bc );
void* tor_connector_client_launch( void* ptr_br );

int tor_server_start( struct sockaddr_in *server, char *host, int port, int max_clients );
int tor_server_accept( struct sockaddr_in* server, struct sockaddr_in* client, int sfd );

bindConnect* tor_connector_parse_bind( configBlock *cb, moduleTAD *modules );
virtualHost* tor_connector_find_vhost( virtualHost *vh, char *name );
void tor_connector_parse_vhost( configBlock *cb, configBlock *aliases, virtualHost **pvh );
void tor_connector_parse_location( configBlock* cb, virtualHost* vh );
void tor_connector_bind_free( bindConnect* bc );
void tor_connector_vhost_free( virtualHost* vh );
void tor_connector_location_free( hostLocation* hl );
void tor_connector_alias_free( hostAlias* ha );

#endif
