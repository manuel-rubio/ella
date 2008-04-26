#include <string.h>
#include "../include/modules.h"
#include "../include/header.h"

void http10_get_status( char *s ) {
    // TODO: especificar en "s" el estado del módulo
    strcpy(s, "OK");
}

int http10_run( requestHTTP *rh, responseHTTP **rs ) {
    if ((*rs)->code > 403) { // Forbidden
    // TODO: implementar la gestión de cabeceras según el RFC1945
    return MODULE_RETURN_OK;
}

void http10_init( moduleTAD *module ) {
    strcpy(module->name, "HTTP 1.0 - RFC1945");
    module->type = MODULE_TYPE_PROC;
    module->priority = 50;

    module->load = NULL;
    module->unload = NULL;
    module->reload = NULL;
    module->get_status = http10_get_status;
    module->run = http10_run;
}
