/* -*- mode:C; coding:utf-8 -*- */

#if !defined __TORNASAUCE_H
#define __TORNASAUCE_H

#include <signal.h>
#include "config.h"
#include "configurator.h"
#include "connector.h"
#include "string.h"
#include "header.h"
#include "modules.h"
#include "memory.h"

extern char bindThreadExit;  //!< interruptor para mantener la ejecución o no.

/**
 *  Inicializa el sistema de lectura de configuración.
 *
 *  Esta función carga las funciones esenciales para proceder
 *  con la carga de los valores de configuración del servidor.
 */
configFuncs tor_get_initial_conf();

/**
 *  Función para tomar la configuración de fichero INI.
 *
 *  Se encarga de leer el fichero de tipo INI para crear toda la
 *  estructura de tipo configBlock y configDetail.
 *
 *  @return un puntero a la cabecera de configBlock creada o NULL.
 */
configBlock* tor_ini_read();

/**
 *  Lanzador de ejecución del servidor.
 *
 *  Se encarga de tomar los datos de conexión, establecer el puerto para
 *  escucha y lanzar las peticiones a otros hilos para su atención. Esta
 *  función será lanzada por pthread.
 *
 *  @param ptr_bc puntero a una estructura bindConnect.
 */
void* tor_connector_launch( void* ptr_bc );

/**
 *  Lanzador de ejecución del cliente.
 *
 *  Se encarga de atender una petición y generar una respuesta tras la
 *  ejecución de todos los módulos necesarios.
 *
 *  @param ptr_br puntero a una estructura bindRequest.
 */
void* tor_connector_client_launch( void* ptr_br );

/**
 *  Inicia servidor en IP y puerto determinado.
 *
 *  Se encarga de establecer un socket servidor para la recepción de
 *  peticiones, pasando la estructura server, el host donde escuchar,
 *  el puerto y el máximo de clientes.
 *
 *  @param server estructura a rellenar.
 *  @param host máquina de la que escuchar peticiones.
 *  @param port puerto en el que escuchar peticiones.
 *  @param max_clients máximo de clientes a soportar.
 */
int tor_server_start( struct sockaddr_in *server, char *host, int port, int max_clients );

/**
 *  Recibe una petición desde socket.
 *
 *  Se encarga de aceptar una petición entrante.
 *
 *  @param server estructura de datos del servidor.
 *  @param client estructura de datos del cliente.
 *  @param sfd descriptor de fichero del servidor.
 */
int tor_server_accept( struct sockaddr_in* server, struct sockaddr_in* client, int sfd );

#endif
