/* -*- mode:C; coding:utf-8 -*- */

#if !defined __EWS_H
#define __EWS_H

#include <signal.h>
#include <poll.h>
#include <rpc/rpc.h>
#include <ctype.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "config.h"
#include "configurator.h"
#include "connector.h"
#include "string.h"
#include "header.h"
#include "modules.h"
#include "memory.h"
#include "cli.h"
#include "date.h"
#include "logger.h"

extern char bindThreadExit;  //!< switch to keep server running.

/**
 *  Init configs read system.
 *
 *  This function loads essential functions to procced with
 *  server config values load.
 */
configFuncs ews_get_initial_conf();

/**
 *  Gets config from a INI file.
 *
 *  @return a head pointer to configBlock or NULL.
 */
configBlock* ews_ini_read();

/**
 *  Server running launch.
 *
 *  @param ptr_bc pointer to a bindConnect structure.
 */
void* ews_connector_launch( void* ptr_bc );

/**
 *  Client running launch.
 *
 *  @param ptr_br pointer to a bindRequest structure.
 */
void* ews_connector_client_launch( void* ptr_br );

/**
 *  Inits a server in a specific IP address and port.
 *
 *  @param server structure to fill.
 *  @param host host to listen connections.
 *  @param port port to listen connections.
 *  @param max_clients max simultaneous clients.
 */
int ews_server_start( struct sockaddr_in *server, char *host, int port, int max_clients );

/**
 *  Receive a request from a socket.
 *
 *  @param server server data structure.
 *  @param client client data structure.
 *  @param sfd server file descriptor (socket).
 */
int ews_server_accept( struct sockaddr_in* server, struct sockaddr_in* client, int sfd );

/**
 *  Console system launch.
 *
 *  @param cc head pointer to loaded commands list.
 *  @return ???
 */
int console_make_socket( struct cli_command **cc );

/**
 *  Inits CLI structure. Adds basic commands (exit, quit, help, shutdown...)
 *
 *  @param cc head pointer to loaded commands list.
 */
void ews_cli_init( struct cli_command **cc );

/**
 *  Run a command.
 *
 *  @param pipe console pipe to send response.
 *  @param request console command.
 *  @return zero if command fails, negative value if command not found and positive value in success.
 */
int ews_cli_command( int pipe, char *request );

#endif
