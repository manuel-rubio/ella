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

    cf = ews_get_initial_conf();
    cb = cf.read();

    modules = ews_modules_load(cb);
    bc = ews_connector_parse_bind(cb, modules);

    signal(SIGTERM, stop_ews);
    signal(SIGINT, stop_ews);
    signal(SIGQUIT, stop_ews);
    signal(SIGKILL, stop_ews);

    for (pbc = bc; pbc != NULL; pbc = pbc->next) {
        rc = pthread_create(&pbc->thread, NULL, ews_connector_launch, (void *)pbc);
        if (rc) {
            printf("ERROR: al crear hilo: %d\n", rc);
        }
    }

//     if (console_make_socket() < 0) {
//         printf("ERROR: in console launch\n");
//     }

    for (pbc = bc; pbc != NULL; pbc = pbc->next) {
        if (pthread_join(pbc->thread, (void **) &status)) {
            printf("ERROR: no se pudo realizar 'join' en hilos.\n");
        }
        printf("INFO: el hilo terminó con el estado %d.\n", status);
    }

    ews_modules_free(modules);
    ews_free_blocks(cb);
    ews_connector_bind_free(bc);
    ews_memory_stats();
    ews_free_all();
    pthread_exit(NULL);
}
