/* -*- mode:C; coding:utf-8 -*- */

#include "../include/config/config.h"
#include "../include/connector/connector.h"
#include "../include/util/string.h"
#include "../include/util/header.h"
#include "../include/modules/modules.h"

#define MAX_BUFFER   8192

int main() {
    configFuncs cf;
    configBlock *cb, *cb_modules;
    moduleTAD *modules;
    bindConnect *bc, *pbc;
    int rc;

    cf = tor_get_initial_conf();
    cb = cf.read();
    bc = tor_connector_parse_bind(cb);
    cb_modules = tor_get_block(cb, "modules", NULL);
    modules = tor_modules_load(cb_modules);

    for (pbc = bc; pbc != NULL; pbc = pbc->next) {
        rc = pthread_create(&bindThreads[bindThreadCounter++], NULL, tor_connector_launch, (void *)pbc);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
        }
    }
    pthread_exit(NULL);

    tor_free_blocks(cb);
    tor_connector_bind_free(bc);
}
