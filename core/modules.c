/* -*- mode:C; coding:utf-8 -*- */

#include "../include/ella.h"

moduleTAD* ews_modules_load( configBlock *cb ) {
    int  indexes, i;
    char *module = NULL,
         lib[80] = { 0 },
         *autoload = NULL;
    moduleTAD *pmt = NULL, *mt = NULL, *tmt = NULL;
    void (*init_module)( moduleTAD* ) = NULL;
    configBlock *pcb = NULL, *cb_modules = NULL;

    cb_modules = ews_get_block(cb, "modules", NULL);
    autoload = ews_get_detail_value(cb_modules->details, "autoload", 0);
    if (autoload != NULL && strcmp(autoload, "yes") == 0) {
        printf("INFO: AutoCarga: sí (no implementado aún)\n");
    } else {
        printf("INFO: AutoCarga: no\n");
    }

    indexes = ews_get_detail_indexes(cb_modules->details, "load");
    for (i=0; i<indexes; i++) {
        module = ews_get_detail_value(cb_modules->details, "load", i);
        sprintf(lib, __MODULES_DIR "/lib%s.so", module);
        printf("DEBUG: probando a cargar %s.\n", lib);
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
            printf("ERROR: en carga de %s: %s", module, dlerror());
            if (mt->next == NULL) {
                ews_free(mt, "ews_modules_load (error en carga 1)");
                pmt = mt = NULL;
            } else {
                for (tmt=mt; tmt->next!=pmt; tmt=tmt->next)
                    ;
                ews_free(tmt->next, "ews_modules_load (error en carga 2)");
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
                    ews_free(mt, "ews_modules_load (error en ejecución 1)");
                    pmt = mt = NULL;
                } else {
                    for (tmt=mt; tmt->next!=pmt; tmt=tmt->next)
                        ;
                    ews_free(tmt->next, "ews_modules_load (error en ejecución 2)");
                    tmt->next = NULL;
                    pmt = tmt;
                }
            } else {
                pcb = ews_get_block(cb, module, NULL);
                pmt->details = (pcb != NULL) ? pcb->details : NULL;
                init_module(pmt);
                if (pmt->load != NULL) {
                    pmt->load();
                }
                printf("INFO: cargado módulo: %s\n", module);
            }
        }
    }

    printf("INFO: NoLoad (no implementado aún)\n");
    indexes = ews_get_detail_indexes(cb_modules->details, "noload");
    for (i=0; i<indexes; i++) {
        // TODO: este no tiene sentido, a menos que se tenga "autoload"
        printf("INFO: Ignorando módulo %s", ews_get_detail_value(cb_modules->details, "unload", i));
    }

    if (mt != NULL) {
        mt = ews_modules_sort(mt);
    }
    return mt;
}

moduleTAD* ews_modules_sort( moduleTAD *mt ) {
    moduleTAD *pmt = NULL, // pointer to moduleTAD
              *tmt = NULL, // temp moduleTAD
              *rmt = NULL; // result moduleTAD

    for (; mt!=NULL; mt=tmt) {
        tmt = mt->next;
        mt->next = NULL;
        if (rmt == NULL) {
            rmt = mt;
        } else {
            pmt = rmt;
            while ((pmt->type > mt->type || (pmt->type == mt->type && pmt->priority > mt->priority)) && pmt->next != NULL) {
                pmt = pmt->next;
            }
            if ((pmt->type < mt->type || (pmt->type == mt->type && pmt->priority < mt->priority)) && pmt->next == NULL) {
                pmt->next = mt;
            } else {
                mt->next = pmt;
                pmt = mt;
                if (pmt->next == rmt) {
                    rmt = pmt;
                }
            }
        }
    }
    return rmt;
}

void ews_modules_free( moduleTAD *modules ) {
    if (modules == NULL)
        return;

    if (modules->next != NULL)
        ews_modules_free(modules->next);

    if (modules->unload != NULL)
        modules->unload();

    printf("INFO: liberando módulo %s.\n", modules->name);
    ews_free_details(modules->details);
    dlclose(modules->handle);
    ews_free(modules, "ews_modules_free");
}
