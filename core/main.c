/* -*- mode:C; coding:utf-8 -*- */

#include "../include/ella.h"

void stop_ews( int d ) {
    bindThreadExit = 1;
}

int main() {
    configFuncs cf;
    configBlock *cb = NULL;
    moduleTAD *modules = NULL;
    bindConnect *bc = NULL, *pbc = NULL;
    int rc, status;

    cf = tor_get_initial_conf();
    cb = cf.read();

    modules = tor_modules_load(cb);
    bc = tor_connector_parse_bind(cb, modules);

    signal(SIGTERM, stop_ews);
    signal(SIGINT, stop_ews);
    signal(SIGQUIT, stop_ews);
    signal(SIGKILL, stop_ews);

    for (pbc = bc; pbc != NULL; pbc = pbc->next) {
        rc = pthread_create(&pbc->thread, NULL, tor_connector_launch, (void *)pbc);
        if (rc) {
            printf("ERROR: al crear hilo: %d\n", rc);
        }
    }

    for (pbc = bc; pbc != NULL; pbc = pbc->next) {
        if (pthread_join(pbc->thread, (void **) &status)) {
            printf("ERROR: no se pudo realizar 'join' en hilos.\n");
        }
        printf("INFO: el hilo terminó con el estado %d.\n", status);
    }

    tor_modules_free(modules);
    tor_free_blocks(cb);
    tor_connector_bind_free(bc);
    tor_memory_stats();
    tor_free_all();
    pthread_exit(NULL);
}
