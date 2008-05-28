/* -*- mode:C; coding:utf-8 -*- */

#if !defined __MODULES_H
#define __MODULES_H

#include <dlfcn.h>
#include "header.h"
#include "configurator.h"
#include "memory.h"
#include "cli.h"

/**
 *  Module types, in priority order.
 */
enum {
    MODULE_TYPE_SEC,  /*!< security modules (highest priority). */
    MODULE_TYPE_PROC, /*!< request process modules (http 1.0, http 1.1...) */
    MODULE_TYPE_HEAD, /*!< response header process modules (mime, SSL encrypt...) */
    MODULE_TYPE_LOG   /*!< log and stadistics modules */
};

/**
 *  Return types for "run" functions.
 */
enum {
    MODULE_RETURN_OK,
    MODULE_RETURN_FAIL,
    MODULE_RETURN_STOP,
    MODULE_RETURN_SEC_STOP,
    MODULE_RETURN_PROC_STOP,
    MODULE_RETURN_HEAD_STOP
};

struct Bind_Request;

/**
 *  Modules structure.
 *
 *  Loads an extern module (in library fashion) and append methods to use in
 *  automatic process.
 */
struct Module {
    char name[80];    //!< module name.
    int  type;        //!< module type.
    int  priority;    //!< module run priority (0-99)

    void (*load)();   //!< run in module load.
    void (*unload)(); //!< run in module unload.
    void (*reload)(); //!< run in module reload.
    void (*get_status)( char * ); //!< return module status.
    int (*run)( struct Bind_Request *, responseHTTP * ); //!< run the module.

    void *handle;          //!< loaded library handler.
    configDetail *details; //!< config details for module.
    struct Module *next;
};

typedef struct Module moduleTAD;

/**
 *  Loads modules listed in configuration.
 *
 *  Gets modules to load throught a configBlock and load them
 *  in a dynamic list to return at function exit.
 *
 *  @param cb head pointer to configBlock.
 *  @param cc head pointer to cliCommands.
 *  @return dynamic list moduleTAD.
 */
moduleTAD* ews_modules_load( configBlock *cb, cliCommand **cc );

/**
 *  Sort modules by type and priority.
 *
 *  @param modules dynamic list for sort.
 */
moduleTAD* ews_modules_sort( moduleTAD *modules );

/**
 *  Free all resources allocated in modules load.
 *
 *  @param modules head pointer to moduleTAD.
 */
void ews_modules_free( moduleTAD *modules );

#endif
