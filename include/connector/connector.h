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
#include "../config/config.h"

#define MAX_CONNS 2
#define CONNECTOR_MAX_THREADS 10

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
    struct Virtual_Host* vhosts;
    struct Bind_Connect* next;
};

typedef struct Bind_Connect bindConnect;

extern int bindThreads[CONNECTOR_MAX_THREADS];
extern int bindThreadCounter;

void tor_connector_launch( void* ptr_bc );

int tor_server_start( struct sockaddr_in *server, char *host, int port );
int tor_server_accept( struct sockaddr_in* server, int sfd );

bindConnect* tor_connector_parse_bind( configBlock *cb );
virtualHost* tor_connector_find_vhost( virtualHost *vh, char *name );
void tor_connector_parse_vhost( configBlock *cb, configBlock *aliases, virtualHost **pvh );
void tor_connector_parse_location( configBlock* cb, virtualHost* vh );
void tor_connector_bind_free( bindConnect* bc );
void tor_connector_vhost_free( virtualHost* vh );
void tor_connector_location_free( hostLocation* hl );
void tor_connector_alias_free( hostAlias* ha );

#endif
