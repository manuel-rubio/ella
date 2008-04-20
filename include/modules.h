/* -*- mode:C; coding:utf-8 -*- */

#if !defined __MODULES_H
#define __MODULES_H

#include <dlfcn.h>
#include "header.h"
#include "configurator.h"

enum {
    MODULE_TYPE_LOG,
    MODULE_TYPE_PROC,
    MODULE_TYPE_SEC
};

struct Module {
    char name[80];
    int  type;

    void (*load)();
    void (*unload)();
    void (*reload)();
    void (*get_status)( char * );
    int (*run)( requestHTTP *, responseHTTP ** );

    void *handle;
    configDetail *details;
    struct Module *next;
};

typedef struct Module moduleTAD;

moduleTAD* tor_modules_load( configBlock *cb );
void tor_modules_free( moduleTAD *modules );

#endif
