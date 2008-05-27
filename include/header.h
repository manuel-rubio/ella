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
 *  Header structure.
 */
struct header {
    char key[100];    //!< header name.
    char value[1024]; //!< header value.
    int index;        //!< index, in multivaluated case.
    struct header *next;
};

typedef struct header headerHTTP;

/**
 *  Request structure.
 */
struct request {
    char request[5];        //!< request: GET, POST o HEAD.
    char uri[1024];         //!< requested URI.
    char version[4];        //!< HTTP version: 1.0 or 1.1.
    struct header *headers;
    char *content;          //!< in PUT or POST case.
};

typedef struct request requestHTTP;

/**
 *  Content types.
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
    int code;               //!< error code (200, 3xx, 4xx, 5xx...)
    char message[50];
    char version[4];        //!< HTTP version: 1.0 or 1.1.
    struct header *headers;
    void *content;
    int content_type;
};

typedef struct response responseHTTP;

/**
 *  Builds a request structure.
 *
 *  @param request request (GET, PUT o POST).
 *  @param uri request uri.
 *  @param version HTTP version: 1.0 or 1.1.
 *  @return new requestHTTP structure or NULL.
 */
requestHTTP* ews_new_request( char *request, char *uri, char *version );

/**
 *  Crea una estructura de respuesta.
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
 *  @param key clave de la cabecera.
 *  @param value valor de la cabecera.
 *  @param index índice en caso de dato multivaluado.
 *  @return estructura header nueva o NULL.
 */
headerHTTP* ews_new_header( char *key, char *value, int index );

/**
 *  Configura en la respuesta el contenido.
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
