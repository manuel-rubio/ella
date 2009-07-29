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

#define EWS_HEADER_GET_ALL -1

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
    char request[10];       //!< request: GET, POST o HEAD.
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
    HEADER_CONTENT_FILE,
    HEADER_CONTENT_RAW
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
 *  Builds a response structure.
 *
 *  @param code response code.
 *  @param message response message.
 *  @param version HTTP version: 1.0 ó 1.1.
 *  @return new responseHTTP structure or NULL.
 */
responseHTTP* ews_new_response( int code, char *message, char *version );

/**
 *  Builds a header structure.
 *
 *  @param key header key.
 *  @param value header value.
 *  @param index in multivaluated case, index for value.
 *  @return new header structure or NULL.
 */
headerHTTP* ews_new_header( char *key, char *value, int index );

/**
 *  Add a new structure header to header list.
 *
 *  @param hh header pointer to headerHTTP.
 *  @param key header key.
 *  @param value header value.
 *  @param index in multivaluated case, index for value.
 */
void ews_add_header( headerHTTP** hh, char *key, char *value, int index );

/**
 *  Configure content in request.
 *
 *  @param rs HTTP request structure.
 *  @param s content to append.
 */
void ews_set_response_content( responseHTTP *rs, int type, void *s );

/**
 *  Free a request and its headers.
 *
 *  @param rh request structure to freed.
 */
void ews_free_request( requestHTTP *rh );

/**
 *  Free a response strcutre and its headers.
 *
 *  @param rs response structure to freed.
 */
void ews_free_response( responseHTTP *rs );

/**
 *  Free a headers list.
 *
 *  @param h header pointer to headerHTTP for free.
 */
void ews_free_header( headerHTTP *h );

/**
 *  Gets given value, name and index from a header.
 *
 *  @param rh pointer to request structure.
 *  @param key key to search.
 *  @param index value index to search.
 *  @return string with header value.
 */
char* ews_get_header_value( requestHTTP *rh, char *key, int index );

/**
 *  Gets values account for a key.
 *
 *  @param rh pointer to request structure.
 *  @param key key for accounting.
 *  @return elements number with the same key found.
 */
int ews_get_header_indexes( requestHTTP *rh, char *key );

/**
 *  Convert a request text into request structure.
 *
 *  @param s request string.
 *  @return requestHTTP structure with converted content.
 */
requestHTTP* ews_parse_request( char *s );

/**
 *  Convert a response structure into response text.
 *
 *  @param rs response structure.
 *  @return string with converted content.
 */
char* ews_gen_response( responseHTTP *rs );

#endif
