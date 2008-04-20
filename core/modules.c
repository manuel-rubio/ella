/* -*- mode:C; coding:utf-8 -*- */

#include "../include/modules.h"

moduleTAD* tor_modules_load( configBlock *cb ) {
    int indexes, i;
    char *module, lib[50], *autoload;
    moduleTAD *pmt = NULL, *mt = NULL, *tmt = NULL;
    void (*init_module)( moduleTAD* );
    configBlock *pcb;

    autoload = tor_get_detail_value(cb, "autoload", 0);
    if (autoload != NULL && strcmp(autoload, "yes") == 0) {
        printf("INFO: AutoCarga: sí (no implementado aún)\n");
    } else {
        printf("INFO: AutoCarga: no\n");
    }

    indexes = tor_get_detail_indexes(cb, "load");
    for (i=0; i<indexes; i++) {
        module = tor_get_detail_value(cb, "load", i);
        sprintf(lib, "modules/lib%s.so", module);
        if (mt == NULL) {
            mt = (moduleTAD *)malloc(sizeof(moduleTAD));
            pmt = mt;
            pmt->prev = NULL;
        } else {
            pmt->next = (moduleTAD *)malloc(sizeof(moduleTAD));
            pmt->next->prev = pmt;
            pmt = pmt->next;
        }
        pmt->next = NULL;
        pmt->handle = dlopen(lib, RTLD_LAZY);
        if (pmt->handle == 0) {
            printf("ERROR: en carga de %s: %s", module, dlerror());
            if (mt->next == NULL) {
                free(mt);
                pmt = mt = NULL;
            } else {
                for (tmt=mt; tmt->next!=pmt; tmt=tmt->next)
                    ;
                free(tmt->next);
                tmt->next = NULL;
                pmt = tmt;
            }
        } else {
            sprintf(lib, "%s_init", module);
            init_module = dlsym(pmt->handle, lib);
            if (init_module == NULL) {
                printf("ERROR: en ejecución de %s_init: %s", module, dlerror());
                dlclose(pmt->handle);
                if (mt->next == NULL) {
                    free(mt);
                    pmt = mt = NULL;
                } else {
                    for (tmt=mt; tmt->next!=pmt; tmt=tmt->next)
                        ;
                    free(tmt->next);
                    tmt->next = NULL;
                    pmt = tmt;
                }
            } else {
                pcb = tor_get_block(cb, module, "");
                pmt->details = (pcb != NULL) ? pcb->details : NULL;
                init_module(pmt);
                printf("INFO: cargado módulo: %s\n", module);
            }
        }
    }

    printf("INFO: NoLoad (no implementado aún)\n");
    indexes = tor_get_detail_indexes(cb, "noload");
    for (i=0; i<indexes; i++) {
        // TODO: este no tiene sentido, a menos que se tenga "autoload"
        printf("INFO: Ignorando módulo %s", tor_get_detail_value(cb, "unload", i));
    }

    if (mt != NULL) {
        tor_modules_sort(&mt);
    }
    return mt;
}

void tor_modules_sort( moduleTAD **modules ) {
    // TODO: listas enlazadas.
}

void tor_modules_free( moduleTAD *modules ) {
    if (modules == NULL)
        return;

    if (modules->next != NULL)
        tor_modules_free(modules->next);

    tor_free_details(modules->details);
    dlclose(modules->handle);
    free(modules);
}
