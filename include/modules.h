/* -*- mode:C; coding:utf-8 -*- */

#if !defined __MODULES_H
#define __MODULES_H

#include <dlfcn.h>
#include "header.h"
#include "configurator.h"

/**
 *  Tipos de módulos que puede haber, y en orden de prioridad.
 */
enum {
    MODULE_TYPE_SEC,
    MODULE_TYPE_PROC,
    MODULE_TYPE_LOG
};

/**
 *  Tipos de retornos para la función "run".
 */
enum {
    MODULE_RETURN_OK,
    MODULE_RETURN_FAIL,
    MODULE_RETURN_STOP
};

struct Bind_Request;

/**
 *  Estructura de Módulos.
 *
 *  Se carga un módulo externo (en forma de librería) y se anexionan los
 *  métodos para usarse en procesamiento automático.
 */
struct Module {
    char name[80];    //!< nombre del módulo.
    int  type;        //!< tipo del módulo.
    int  priority;    //!< prioridad de ejecución del módulo (0-99)

    void (*load)();   //!< ejecuta en la carga del módulo.
    void (*unload)(); //!< ejecuta en la descarga del módulo.
    void (*reload)(); //!< para la recarga del módulo.
    void (*get_status)( char * ); //!< retorna el estado del módulo.
    int (*run)( struct Bind_Request *, responseHTTP * ); //!< ejecuta el módulo.

    void *handle;          //!< manejador de la librería cargada.
    configDetail *details; //!< lista enlazada de todos los detalles.
    struct Module *next;
};

typedef struct Module moduleTAD;

/**
 *  Carga los módulos de la configuración.
 *
 *  Toma los módulos a cargar a través de un configBlock y va
 *  cargando uno a uno cada módulo en una lista enlazada que
 *  será retornada.
 *
 *  @param cb puntero a configBlock de cabecera.
 *  @return una lista enlazada de moduleTAD.
 */
moduleTAD* tor_modules_load( configBlock *cb );

/**
 *  Reordena los módulos por tipo y prioridad.
 *
 *  @param modules lista de módulos a ordenar.
 */
moduleTAD* tor_modules_sort( moduleTAD *modules );

/**
 *  Libera los recursos reservador de la carga de módulos.
 *
 *  @param modules puntero a moduleTAD de cabecera.
 */
void tor_modules_free( moduleTAD *modules );

#endif
