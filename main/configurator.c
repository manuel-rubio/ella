/* -*- mode:C; coding:utf-8 -*- */

#include "../include/ella.h"

configFuncs ews_get_initial_conf() {
    configFuncs cf;
#if defined EWS_CONFIG_STATIC
    cf.name = "INI";
    cf.read = ews_ini_read;
#else
    // TODO: load system to dynamic configuration.
#endif
    return cf;
}

/* INI Method */

configBlock* ews_ini_read() {
    FILE *f;
    char buffer[1024] = { 0 },
         clave[512] = { 0 },
         valor[512] = { 0 },
         key[STRING_SIZE] = { 0 };
    int  bs = 0,
         i, j, index,
         name_flag = 0;
    configBlock *cb = NULL, *pcb = NULL;
    configDetail *pcd = NULL;

    f = fopen(EWS_CONFIG_DIR "/http.ini", "rt");
    if (f != NULL) {
        do {
            fgets(buffer, sizeof(buffer), f);
            if (!feof(f)) {
                ews_chomp(buffer);
                ews_trim(buffer);
                if (buffer[0] == '[') {
                    // seccion
                    if (pcb == NULL) {
                        cb = (configBlock *)ews_malloc(sizeof(configBlock));
                        pcb = cb;
                    } else {
                        pcb->next = (configBlock *)ews_malloc(sizeof(configBlock));
                        pcb = pcb->next;
                    }
                    pcb->next = NULL;
                    pcb->details = NULL;
                    name_flag = 0;
                    for (i=1, j=0; buffer[i]!=']' && buffer[i]!='\0'; i++, j++) {
                        if (buffer[i] == ':') {
                            name_flag = 1;
                            pcb->name[j] = '\0';
                            j = -1;
                            continue;
                        }
                        if (!name_flag)
                            pcb->name[j] = buffer[i];
                        else
                            pcb->lastname[j] = buffer[i];
                    }
                    if (name_flag)
                        pcb->lastname[j] = '\0';
                } else if ((buffer[0] >= 'A' && buffer[0] <= 'Z') || (buffer[0] >= 'a' && buffer[0] <= 'z') || buffer[0] == '_') {
                    // clave
                    if (pcb == NULL) {
                        ews_verbose(LOG_LEVEL_ERROR, "config file, detail out of block.");
                        exit(1);
                    }
                    if (pcb->details == NULL) {
                        pcb->details = (configDetail *)ews_malloc(sizeof(configDetail));
                        pcd = pcb->details;
                    } else {
                        for (pcd = pcb->details; pcd->next != NULL; pcd = pcd->next)
                            ;
                        pcd->next = (configDetail *)ews_malloc(sizeof(configDetail));
                        pcd = pcd->next;
                    }
                    pcd->next = NULL;
                    pcd->index = 0;
                    name_flag = 0;
                    for (i=0, j=0; buffer[i]!='\0'; i++, j++) {
                        if (!name_flag) {
                            if (buffer[i] == '=') {
                                name_flag = 1;
                                pcd->key[j] = '\0';
                                ews_trim(pcd->key);
                                strcpy(key, pcd->key);
                                j = -1;
                                continue;
                            }
                            pcd->key[j] = buffer[i];
                        } else {
                            if (buffer[i] == ',') {
                                index = pcd->index;
                                pcd->value[j] = '\0';
                                ews_trim(pcd->value);
                                j = -1;
                                pcd->next = (configDetail *)ews_malloc(sizeof(configDetail));
                                pcd = pcd->next;
                                strcpy(pcd->key, key);
                                pcd->index = ++index;
                                continue;
                            }
                            pcd->value[j] = buffer[i];
                        }
                    }
                    pcd->value[j] = '\0';
                    ews_trim(pcd->value);
                }
            }
        } while(!feof(f));
        fclose(f);
    }
    return cb;
}
