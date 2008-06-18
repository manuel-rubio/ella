/* -*- mode:C; coding:utf-8 -*- */

#include "../include/ella.h"

moduleTAD* ews_modules_load( configBlock *cb, cliCommand **cc ) {
    int  indexes, i;
    char *module = NULL,
         lib[80] = { 0 },
         *autoload = NULL;
    moduleTAD *pmt = NULL, *mt = NULL, *tmt = NULL;
    void (*init_module)( moduleTAD*, cliCommand** ) = NULL;
    configBlock *pcb = NULL, *cb_modules = NULL;

    cb_modules = ews_get_block(cb, "modules", NULL);
    autoload = ews_get_detail_value(cb_modules->details, "autoload", 0);
    if (autoload != NULL && strcmp(autoload, "yes") == 0) {
        // TODO: autoloader to implement
        ews_verbose(LOG_LEVEL_INFO, "AutoLoad: yes (not yet implemented)");
    } else {
        ews_verbose(LOG_LEVEL_INFO, "AutoLoad: no");
    }

    indexes = ews_get_detail_indexes(cb_modules->details, "load");
    for (i=0; i<indexes; i++) {
        module = ews_get_detail_value(cb_modules->details, "load", i);
        sprintf(lib, EWS_MODULES_DIR "/lib%s.so", module);
        ews_verbose(LOG_LEVEL_DEBUG, "try to load: %s.", lib);
        if (mt == NULL) {
            mt = (moduleTAD *)ews_malloc(sizeof(moduleTAD));
            pmt = mt;
        } else {
            pmt->next = (moduleTAD *)ews_malloc(sizeof(moduleTAD));
            pmt = pmt->next;
        }
        pmt->next = NULL;
        pmt->handle = dlopen(lib, RTLD_LAZY);
        if (pmt->handle == 0) {
            ews_verbose(LOG_LEVEL_ERROR, "loading [%s]: %s", module, dlerror());
            if (mt->next == NULL) {
                ews_free(mt, "ews_modules_load (load error)");
                pmt = mt = NULL;
            } else {
                for (tmt=mt; tmt->next!=pmt; tmt=tmt->next)
                    ;
                ews_free(tmt->next, "ews_modules_load (load error)");
                tmt->next = NULL;
                pmt = tmt;
            }
        } else {
            sprintf(lib, "%s_init", module);
            init_module = dlsym(pmt->handle, lib);
            if (init_module == NULL) {
                ews_verbose(LOG_LEVEL_ERROR, "running [%s_init]: %s", module, dlerror());
                dlclose(pmt->handle);
                if (mt->next == NULL) {
                    ews_free(mt, "ews_modules_load (load error)");
                    pmt = mt = NULL;
                } else {
                    for (tmt=mt; tmt->next!=pmt; tmt=tmt->next)
                        ;
                    ews_free(tmt->next, "ews_modules_load (load error)");
                    tmt->next = NULL;
                    pmt = tmt;
                }
            } else {
                pcb = ews_get_block(cb, module, NULL);
                pmt->details = (pcb != NULL) ? pcb->details : NULL;
                init_module(pmt, cc);
                if (pmt->load != NULL) {
                    pmt->load();
                }
                ews_verbose(LOG_LEVEL_INFO, "module loaded: %s", module);
            }
        }
    }

    ews_verbose(LOG_LEVEL_INFO, "NoLoad (not yet implemented)");
    indexes = ews_get_detail_indexes(cb_modules->details, "noload");
    for (i=0; i<indexes; i++) {
        // TODO: needs autoload implementation to do it
        ews_verbose(LOG_LEVEL_INFO, "Module ignored [%s]", ews_get_detail_value(cb_modules->details, "unload", i));
    }

    if (mt != NULL) {
        mt = ews_modules_sort(mt);
    }
    return mt;
}

moduleTAD* ews_modules_sort( moduleTAD *mt ) {
    moduleTAD *sort_mt[512] = { 0 };
    moduleTAD *pmt = NULL;
    int tam = 0, i, j, k;

    for (pmt=mt; pmt!=NULL; pmt=pmt->next) {
        sort_mt[tam++] = pmt;
    }

    for (i=0; i<(tam-1); i++) {
        k = i;
        for (j=i+1; j<tam; j++) {
            if (sort_mt[k]->type > sort_mt[j]->type || (sort_mt[k]->type == sort_mt[j]->type && sort_mt[k]->priority >= sort_mt[j]->priority)) {
                k = j;
            }
        }
        if (k != i) {
            pmt = sort_mt[k];
            sort_mt[k] = sort_mt[i];
            sort_mt[i] = pmt;
        }
    }

    for (i=0; i<(tam-1); i++) {
        sort_mt[i]->next = sort_mt[i+1];
    }
    sort_mt[tam-1]->next = NULL;
    mt = sort_mt[0];
    return mt;
}

void ews_modules_free( moduleTAD *modules ) {
    if (modules == NULL)
        return;

    if (modules->next != NULL) {
        ews_modules_free(modules->next);
        modules->next = NULL;
    }

    if (modules->unload != NULL)
        modules->unload();

    ews_verbose(LOG_LEVEL_INFO, "module free [%s].", modules->name);

    // Los detalles se liberan con los bloques
    modules->details = NULL;

    dlclose(modules->handle);
    ews_free(modules, "ews_modules_free");
}
