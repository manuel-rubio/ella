/* -*- mode:C; coding:utf-8 -*- */

#include "../include/main.h"

static moduleTAD *modules = NULL;

int ews_modules_cli_info( int pipe, char *params ) {
    moduleTAD *pmt = NULL;
    char buffer[BUFFER_SIZE];
    char *aux, *tmp;
    int i;

    if (params != NULL) {
        for (pmt=modules; pmt!=NULL; pmt=pmt->next) {
            if (strcmp(pmt->name, params) == 0) {
                if (pmt->get_status != NULL) {
                    pmt->get_status(buffer);
                    aux = buffer;
                    tmp = buffer;
                    for (i=0; aux[i]!='\0'; i++) {
                        if (aux[i] == '\n') {
                            aux[i] = '\0';
                            ews_verbose_to(pipe, LOG_LEVEL_INFO, tmp);
                            tmp = aux + i + 1;
                        }
                    }
                    ews_verbose_to(pipe, LOG_LEVEL_INFO, tmp);
                    break;
                } else {
                    ews_verbose_to(pipe, LOG_LEVEL_INFO, "this module hasn't status command");
                }
            }
        }
        if (pmt == NULL) {
            ews_verbose_to(pipe, LOG_LEVEL_INFO, "module not found");
        }
    } else {
        ews_verbose_to(pipe, LOG_LEVEL_INFO, "Sintaxis: info <module>");
    }
    return 1;
}

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
        sprintf(lib, EWS_MODULES_DIR "/lib%s" EWS_DYN_EXT, module);
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

    ews_cli_add_command(cc, "info", "use info with a module name for see information about module.", "\n\
Sintaxis: info <module>\n\
Description: show module status.\n", ews_modules_cli_info);

    modules = mt;
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

int ews_modules_reload( configBlock *cb, int pipe, char *name ) {
    moduleTAD *pmt = NULL;

    if (name != NULL) {
        for (pmt=modules; pmt!=NULL; pmt=pmt->next) {
            if (strcmp(pmt->name, name) == 0) {
                if (pmt->reload != NULL) {
                    ews_verbose(LOG_LEVEL_INFO, "reloading %s module", pmt->name);
                    pmt->reload(cb);
                    break;
                } else {
                    ews_verbose_to(pipe, LOG_LEVEL_INFO, "this modules hasn't reload function");
                }
            }
        }
        if (pmt == NULL) {
            // TODO: special reload parts.
            ews_verbose_to(pipe, LOG_LEVEL_INFO, "module not found");
        }
    } else {
        ews_verbose_to(pipe, LOG_LEVEL_INFO, "Sintax: reload <module>");
    }
    return 1;
}

int ews_modules_cli_list( int pipe, char *params ) {
    moduleTAD *pmt = NULL;

    ews_verbose_to(pipe, LOG_LEVEL_INFO, "%-20s", "Module Available");
    ews_verbose_to(pipe, LOG_LEVEL_INFO, "--------------------");
    for (pmt=modules; pmt!=NULL; pmt=pmt->next) {
        ews_verbose_to(pipe, LOG_LEVEL_INFO, "%-20s", pmt->name);
    }
    return 1;
}

void ews_modules_free( moduleTAD *mt ) {
    if (mt == NULL)
        return;

    if (mt->next != NULL) {
        ews_modules_free(mt->next);
        mt->next = NULL;
    }

    if (mt->unload != NULL)
        mt->unload();

    ews_verbose(LOG_LEVEL_INFO, "module free [%s].", mt->name);

    // details will be freed in free blocks
    mt->details = NULL;

    dlclose(mt->handle);
    ews_free(mt, "ews_modules_free");
}
