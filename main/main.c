/* -*- mode:C; coding:utf-8 -*- */

#include "../include/config/config.h"
#include "../include/connector/connector.h"
#include "../include/util/string.h"
#include "../include/util/header.h"
#include "../include/modules/modules.h"
#include <signal.h>

#define MAX_BUFFER   8192

void stop_tornasauce( int d ) {
    bindThreadExit = 1;
}

int main() {
    configFuncs cf;
    configBlock *cb, *cb_modules;
    moduleTAD *modules;
    bindConnect *bc, *pbc;
    int rc;

    cf = tor_get_initial_conf();
    cb = cf.read();

    cb_modules = tor_get_block(cb, "modules", NULL);
    modules = tor_modules_load(cb_modules);
    bc = tor_connector_parse_bind(cb, modules);

    signal(SIGTERM, stop_tornasauce);
    signal(SIGINT, stop_tornasauce);
    signal(SIGQUIT, stop_tornasauce);
    signal(SIGKILL, stop_tornasauce);

    for (pbc = bc; pbc != NULL; pbc = pbc->next) {
        rc = pthread_create(&bindThreads[bindThreadCounter++], NULL, tor_connector_launch, (void *)pbc);
        if (rc) {
            printf("ERROR: al crear hilo: %d\n", rc);
        }
    }
    pthread_exit(NULL);

    tor_free_blocks(cb);
    tor_connector_bind_free(bc);
}
