/* -*- mode:C; coding:utf-8 -*- */

#include "../include/ella.h"

void stop_ews( int d ) {
    bindThreadExit = 1;
}

int main() {
    configFuncs cf;
    configBlock *cb = NULL;
    moduleTAD *modules = NULL;
    cliCommand *commands = NULL;
    bindConnect *bc = NULL, *pbc = NULL;
    int rc;

    logger_init();
    cf = ews_get_initial_conf();
    cb = cf.read();
    logger_config(ews_get_block(cb, "logger", NULL));

    modules = ews_modules_load(cb, &commands);
    bc = ews_connector_parse_bind(cb, modules);

    signal(SIGTERM, stop_ews);
    signal(SIGINT, stop_ews);
    signal(SIGQUIT, stop_ews);
    signal(SIGKILL, stop_ews);

    for (pbc = bc; pbc != NULL; pbc = pbc->next) {
        rc = pthread_create(&pbc->thread, NULL, ews_connector_launch, (void *)pbc);
        if (rc) {
            ews_verbose(LOG_LEVEL_ERROR, "in thread creation: %d", rc);
        }
    }

    if (console_make_socket(&commands) < 0) {
        ews_verbose(LOG_LEVEL_ERROR, "console launch failed");
    }

    for (pbc = bc; pbc != NULL; pbc = pbc->next) {
        if (pthread_join(pbc->thread, NULL)) {
            ews_verbose(LOG_LEVEL_ERROR, "can't do 'join' in threads");
        }
    }

    ews_modules_free(modules);
    ews_cli_free(&commands);
    ews_free_blocks(cb);
    ews_connector_bind_free(bc);
    ews_memory_stats();
    ews_free_all();
    pthread_exit(NULL);
}
