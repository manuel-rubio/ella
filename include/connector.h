/* -*- mode:C; coding:utf-8 -*- */

#if !defined __CONNECTOR_H
#define __CONNECTOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include "configurator.h"
#include "header.h"
#include "modules.h"
#include "memory.h"

/**
 *  Hosts and/or domains alias structure.
 */
struct Host_Alias {
    char alias[80];  //!< alias name.
    struct Host_Alias* next;
};

typedef struct Host_Alias hostAlias;

/**
 *  Host localization or URI structure.
 */
struct Host_Location {
    char base_uri[512];
    configDetail* details;
    struct Host_Location* next;
};

typedef struct Host_Location hostLocation;

/**
 *  Virtual host structure.
 */
struct Virtual_Host {
    char host_name[80];
    struct Host_Location* locations;
    struct Host_Alias* aliases;
    struct Virtual_Host* next;
};

typedef struct Virtual_Host virtualHost;

/**
 *  Connections structure.
 */
struct Bind_Connect {
    char host[80];               //!< IP address to bind requests.
    int port;                    //!< port to bind requests.
    pthread_t thread;            //!< server thread.
    struct Module* modules;
    struct Virtual_Host* vhosts;
    struct Bind_Connect* next;
};

typedef struct Bind_Connect bindConnect;

/**
 *  Requests structure.
 */
struct Bind_Request {
    bindConnect* bc;
    requestHTTP* request;
    pthread_t thread;          //!< client thread.
    struct sockaddr_in client; //!< client data.
    int fd_client;
    int conn_next_status;      //!< to know if connection will be closed.
    int conn_timeout;          //!< time to wait in "keep-alive" case.
};

typedef struct Bind_Request bindRequest;

enum {
    EWS_CON_CLOSE,       //!< close connection
    EWS_CON_KEEPALIVE    //!< keep connection open (alive)
};

/**
 *  Parse config blocks to build a bindConnection.
 *
 *  @param cb head pointer to configBlock.
 *  @param modules head pointer to moduleTAD.
 *  @return a bindConnect structure or NULL.
 */
bindConnect* ews_connector_parse_bind( configBlock *cb, moduleTAD *modules );

/**
 *  Returns a virtual host structure searching for name.
 *
 *  @param vh head pointer to virtualHost.
 *  @param name virtual host name to search.
 *  @return pointer to virtualHost or NULL.
 */
virtualHost* ews_connector_find_vhost( virtualHost *vh, char *name );

/**
 *  Returns a host location structure searching for URL.
 *
 *  @param vh head pointer to virtualHost.
 *  @param loc path to search.
 *  @return pointer to hostLocation or NULL.
 */
hostLocation* ews_connector_find_location( hostLocation *vh, char *loc );

/**
 *  Fills an virtual host structure.
 *
 *  @param cb head pointer to configBlock.
 *  @param aliases pointer to aliases configBlock.
 *  @param pvh pointer to pointer to virtualHost (pointer by reference)
 */
void ews_connector_parse_vhost( configBlock *cb, configBlock *aliases, virtualHost **pvh );

/**
 *  Fills virtual host locations.
 *
 *  @param cb pointer to configBlock, where are configurations.
 *  @param vh pointer to virtualHost, where build locations.
 */
void ews_connector_parse_location( configBlock* cb, virtualHost* vh );

/**
 *  Free all bind connection structures.
 *
 *  @param bc head pointer to bindConnect.
 */
void ews_connector_bind_free( bindConnect* bc );

/**
 *  Free bind request structure.
 *
 *  @param br pointer to bindRequest.
 */
void ews_connector_bindrequest_free( bindRequest* br );

/**
 *  Free virtual host structure.
 *
 *  @param vh pointer to virtualHost.
 */
void ews_connector_vhost_free( virtualHost* vh );

/**
 *  Free all localizations structures.
 *
 *  @param hl head pointer to hostLocation.
 */
void ews_connector_location_free( hostLocation* hl );

/**
 *  Free all aliases structures.
 *
 *  @param ha head pointer to hostAlias.
 */
void ews_connector_alias_free( hostAlias* ha );

#endif
