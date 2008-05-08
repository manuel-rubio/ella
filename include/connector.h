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
 *  Estructura de alias de máquinas y/o dominios.
 *
 *  Lista enlazada para nombres de alias, que se enlazará
 *  dentro de la estructura Virtual Host.
 */
struct Host_Alias {
    char alias[80];  //!< nombre del alias.
    struct Host_Alias* next;
};

typedef struct Host_Alias hostAlias;

/**
 *  Estructura de localización o URI para una máquina.
 *
 *  Lista enlazada para nombres de localizaciones, que se
 *  enlazará dentro de la estructura Virtual Host.
 */
struct Host_Location {
    char base_uri[512];    //!< base URI para la máquina.
    configDetail* details; //!< detalles de configuración.
    struct Host_Location* next;
};

typedef struct Host_Location hostLocation;

/**
 *  Estructura de máquina virtual.
 *
 *  Lista enlazada para máquinas virtuales, que se enlazará
 *  dentro de Bind Connect.
 */
struct Virtual_Host {
    char host_name[80];              //!< nombre de la máquina.
    struct Host_Location* locations; //!< localizaciones.
    struct Host_Alias* aliases;      //!< aliases.
    struct Virtual_Host* next;
};

typedef struct Virtual_Host virtualHost;

/**
 *  Estructura de conexiones.
 *
 *  Lista enlazada con las conexiones especificas que se
 *  detallan en la estructura.
 */
struct Bind_Connect {
    char host[80];               //!< IP en la que escuchar peticiones.
    int port;                    //!< puerto en el que escuchar peticiones.
    pthread_t thread;            //!< hilo del servidor.
    struct Module* modules;      //!< modulos a ejecutar en cada petición.
    struct Virtual_Host* vhosts; //!< nombre de las máquinas virtuales.
    struct Bind_Connect* next;
};

typedef struct Bind_Connect bindConnect;

/**
 *  Estructura de peticiones.
 *
 *  Se usará para pasar la información sobre la petición recibida.
 */
struct Bind_Request {
    bindConnect* bc;           //!< una estructura de conexión.
    requestHTTP* request;      //!< las cabeceras de la solicitud.
    pthread_t thread;          //!< hilo para la ejecución del cliente.
    struct sockaddr_in client; //!< datos de cliente.
    int fd_client;             //!< descriptor de fichero para socket.
};

typedef struct Bind_Request bindRequest;

/**
 *  Rellena una estructura de conexión con los datos de configuración.
 *
 *  Toma todos los datos de configBlock relacionados con la conexión que
 *  se procesa y los almacena, junto con los módulos (modules) en la
 *  estructura de conexión.
 *
 *  @param cb puntero a una estructura de conexión.
 *  @param modules puntero a moduleTAD de cabecera.
 *  @return una estructura de tipo conexión (bindConnect) o NULL.
 */
bindConnect* ews_connector_parse_bind( configBlock *cb, moduleTAD *modules );

/**
 *  Retorna una estructura de máquina virtual por nombre.
 *
 *  Busca una estructura de máquina virtual por nombre.
 *
 *  @param vh puntero a virtualHost de cabecera.
 *  @param name nombre de la máquina virtual a buscar.
 *  @return puntero a virtualHost encontrado o NULL.
 */
virtualHost* ews_connector_find_vhost( virtualHost *vh, char *name );

/**
 *  Retorna una estructura de localización para una URL dada.
 *
 *  @param vh puntero a virtualHost de cabecera.
 *  @param loc ruta a buscar entre los localizations.
 *  @return puntero a hostLocation encontrado o NULL.
 */
hostLocation* ews_connector_find_location( hostLocation *vh, char *loc );

/**
 *  Rellena una estructura de máquina virtual.
 *
 *  Toma los datos de una estructura de configBlock relacionados con
 *  una máquina virtual.
 *
 *  @param cb puntero a configBlock a cabecera.
 *  @param aliases puntero a configBlock de aliases.
 *  @param pvh puntero a puntero de virtualHost (puntero pasado por referencia)
 */
void ews_connector_parse_vhost( configBlock *cb, configBlock *aliases, virtualHost **pvh );

/**
 *  Rellena locations de una estructura de máquina virtual.
 *
 *  @param cb puntero a configBlock, donde están las configuraciones.
 *  @param vh puntero a virtualHost, donde crear las locations.
 */
void ews_connector_parse_location( configBlock* cb, virtualHost* vh );

/**
 *  Libera una estructura de conexión.
 *
 *  @param bc puntero bindConnect de cabecera a liberar.
 */
void ews_connector_bind_free( bindConnect* bc );

/**
 *  Libera una estructura de conexión en petición.
 *
 *  @param br puntero bindRequest a liberar.
 */
void ews_connector_bindrequest_free( bindRequest* br );

/**
 *  Libera una estructura de máquinas virtuales.
 *
 *  @param vh puntero virtualHost de cabecera a liberar.
 */
void ews_connector_vhost_free( virtualHost* vh );

/**
 *  Libera una estructura de localizaciones.
 *
 *  @param hl puntero hostLocation de cabecera a liberar.
 */
void ews_connector_location_free( hostLocation* hl );

/**
 *  Libera una estructura de aliases.
 *
 *  @param ha puntero hostAlias de cabecera a liberar.
 */
void ews_connector_alias_free( hostAlias* ha );

#endif
