/* -*- mode:C; coding:utf-8 -*- */

#if !defined __HEADER_H
#define __HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "config.h"
#include "memory.h"
#include "logger.h"

/**
 *  Estructura de cabecera.
 *
 *  Contendrá las cabeceras HTTP separadas entre la clave,
 *  su valor y, en caso que sean valores separados por comas,
 *  habrá varios con índices que indicarán su orden.
 */
struct header {
    char key[100];    //!< nombre de la cabecera.
    char value[1024]; //!< valor de la cabecera.
    int index;        //!< índice, en caso de datos multivaluados.
    struct header *next;
};

typedef struct header headerHTTP;

/**
 *  Estructura de solicitud.
 *
 *  Contendrá las cabeceras, URI solicitada y la versión de HTTP,
 *  así como el contenido, en caso de una petición POST.
 */
struct request {
    char request[5];        //!< solicitud: GET, POST o HEAD.
    char uri[1024];         //!< URI de solicitud.
    char version[4];        //!< versión de HTTP: 1.0 ó 1.1.
    struct header *headers; //!< cabeceras.
    char *content;          //!< contenido en caso de PUT o POST.
};

typedef struct request requestHTTP;

/**
 *  Tipos de Contenidos.
 *
 *  El puntero genérico puede ofrecer estos tipos de contenidos distintos.
 */
enum {
    HEADER_CONTENT_NONE,
    HEADER_CONTENT_STRING,
    HEADER_CONTENT_FILE
};

/**
 *  Estructura de respuesta.
 *
 *  Contendrá las cabeceras, código de respuesta, mensaje de respuesta,
 *  las cabeceras y el contenido, o página a retornar.
 */
struct response {
    int code;               //!< código de error (200, 3xx, 4xx, 5xx...)
    char message[50];       //!< mensaje.
    char version[4];        //!< versión de HTTP: 1.0 ó 1.1.
    struct header *headers; //!< cabeceras.
    void *content;          //!< contenido a retornar
    int content_type;       //!< tipo de contenido que se retorna.
};

typedef struct response responseHTTP;

/**
 *  Crea una estructura de solicitud.
 *
 *  Pasados los parámetros de solicitud, uri y versión,
 *  crea una estructura de solicitud.
 *
 *  @param request solicitud (GET, PUT o POST).
 *  @param uri uri de solicitud.
 *  @param version la versión de HTTP: 1.0 ó 1.1.
 *  @return estructura requestHTTP nueva o NULL.
 */
requestHTTP* ews_new_request( char *request, char *uri, char *version );

/**
 *  Crea una estructura de respuesta.
 *
 *  Pasados los parámetros de código, mensaje y versión,
 *  crea una estructura de respuesta.
 *
 *  @param code código de respuesta.
 *  @param message mensaje de respuesta.
 *  @param version la versión HTTP: 1.0 ó 1.1.
 *  @return estructura responseHTTP nueva o NULL.
 */
responseHTTP* ews_new_response( int code, char *message, char *version );

/**
 *  Crea una estructura de cabecera.
 *
 *  Pasados los parámetros de clave, valor e índica,
 *  crea una estructura de cabecera.
 *
 *  @param key clave de la cabecera.
 *  @param value valor de la cabecera.
 *  @param index índice en caso de dato multivaluado.
 *  @return estructura header nueva o NULL.
 */
headerHTTP* ews_new_header( char *key, char *value, int index );

/**
 *  Configura en la respuesta el contenido.
 *
 *  Pasada la estructura de respuesta HTTP, anexiona el contenido
 *  y agrega una cabecera de tipo Content-Length.
 *
 *  @param rs estructura de respuesta HTTP.
 *  @param s contenido a anexionar.
 */
void ews_set_response_content( responseHTTP *rs, int type, void *s );

/**
 *  Libera una solicitud y sus cabeceras.
 *
 *  @param rh estructura de solicitud a liberar.
 */
void ews_free_request( requestHTTP *rh );

/**
 *  Libera una respuesta y sus cabeceras.
 *
 *  @param rs estructura de respuesta a liberar.
 */
void ews_free_response( responseHTTP *rs );

/**
 *  Libera una lista de cabeceras.
 *
 *  @param h puntero a headerHTTP de cabecera a liberar.
 */
void ews_free_header( headerHTTP *h );

/**
 *  Toma el valor dado el nombre e índice de una cabecera.
 *
 *  @param rh puntero a estructura de solicitud.
 *  @param key clave a buscar.
 *  @param index índice del valor a rescatar.
 *  @return valor de la cabecera en forma de cadena de caracteres.
 */
char* ews_get_header_value( requestHTTP *rh, char *key, int index );

/**
 *  Toma el número de valores que hay bajo una clave.
 *
 *  @param rh puntero a estructura de solicitud.
 *  @param key clave para el conteo.
 *  @return número de elementos con la misma clave, encontrados.
 */
int ews_get_header_indexes( requestHTTP *rh, char *key );

/**
 *  Convierte un texto de solicitud en una estructura de solicitud.
 *
 *  @param s cadena de solicitud.
 *  @return estructura requestHTTP con el contenido convertido.
 */
requestHTTP* ews_parse_request( char *s );

/**
 *  Convierte una estructura de respuesta en texto.
 *
 *  @param rs estructura de tipo respuesta.
 *  @return cadena de texto con el contenido de la estructura.
 */
char* ews_gen_response( responseHTTP *rs );

#endif
