/* -*- mode:C; coding:utf-8 -*- */

#if !defined __MODULES_H
#define __MODULES_H

#include <dlfcn.h>
#include "header.h"
#include "configurator.h"
#include "memory.h"
#include "cli.h"

/**
 *  Tipos de módulos que puede haber, y en orden de prioridad.
 */
enum {
    MODULE_TYPE_SEC,  /*!< módulos de seguridad (prioritarios). */
    MODULE_TYPE_PROC, /*!< módulos de procesado de peticiones (http 1.0, http 1.1...) */
    MODULE_TYPE_HEAD, /*!< módulos de proceso de cabecera de respuesta (mime, encriptación SSL...) */
    MODULE_TYPE_LOG   /*!< módulos de anotaciones y estadísticas */
};

/**
 *  Tipos de retornos para la función "run".
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
 *  @param cc puntero a cabecera de cliCommands.
 *  @return una lista enlazada de moduleTAD.
 */
moduleTAD* ews_modules_load( configBlock *cb, cliCommand **cc );

/**
 *  Reordena los módulos por tipo y prioridad.
 *
 *  @param modules lista de módulos a ordenar.
 */
moduleTAD* ews_modules_sort( moduleTAD *modules );

/**
 *  Libera los recursos reservador de la carga de módulos.
 *
 *  @param modules puntero a moduleTAD de cabecera.
 */
void ews_modules_free( moduleTAD *modules );

#endif
