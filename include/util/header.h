#if !defined __HEADER_H
#define __HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include "../config.h"

struct header {
    char key[100];
    char value[1024];
    int index;
    struct header *next;
};

typedef struct header headerHTTP;

struct request {
    char request[5];
    char uri[1024];
    char version[4];
    struct header *headers;
};

typedef struct request requestHTTP;

requestHTTP* tor_new_request( char *request, char *uri, char *version );
headerHTTP* tor_new_header( char *key, char *value, int index );

void tor_free_request( requestHTTP *rh );
void tor_free_header( headerHTTP *h );

char* tor_get_header_value( requestHTTP *rh, char *key, int index );
int tor_get_header_indexes( requestHTTP *rh, char *key );

requestHTTP* tor_parse_request( char *s );

#endif
