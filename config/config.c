/* -*- mode:C; coding:utf-8 -*- */

#include "config.h"
#include "../util/string.h"

configFuncs get_initial_conf()
{
    configFuncs cf;
#if defined __CONFIG_STATIC
    cf.name = "INI";
    cf.read = ini_read;
#else
    // TODO
#endif
    return cf;
}

configBlock* new_block( char *name, char *lastname ) {
    configBlock* cb = (configBlock *)malloc(sizeof(configBlock));
    cb->details = NULL;
    copy(name, cb->name);
    copy(lastname, cb->lastname);
    cb->next = NULL;
    return cb;
}

configDetail* new_detail( char *key, char *value, int index ) {
    configDetail* cd = (configDetail *)malloc(sizeof(configDetail));
    copy(key, cd->key);
    copy(value, cd->value);
    cd->index = index;
    cd->next = NULL;
    return cd;
}

void free_blocks( configBlock *cb ) {
    if (cb == NULL) {
        return;
    }
    if (cb->details != NULL) {
        free_details(cb->details);
    }
    if (cb->next != NULL) {
        free_blocks(cb->next);
    }
    free(cb);
}

void free_details( configDetail *cd ) {
    if (cd == NULL) {
        return;
    }
    if (cd->next != NULL) {
        free_details(cd->next);
    }
    free(cd);
}

configBlock* get_block( configBlock *cb, char *name, char *lastname ) {
    for (;cb != NULL; cb = cb->next) {
        if (lastname) {
            if (compare(cb->name, name) == 0 && compare(cb->lastname, lastname) == 0)
                return cb;
        } else {
            if (compare(cb->name, name) == 0)
                return cb;
        }
    }
    return NULL;
}

char* get_detail_value( configBlock *cb, char *key, int index ) {
    configDetail *cd;
    if (cb == NULL || cb->details == NULL)
        return NULL;
    for (cd=cb->details; cd != NULL; cd = cd->next)
        if (compare(cd->key, key) == 0 && cd->index == index)
            return cd->value;
    return NULL;
}

int get_detail_indexes( configBlock *cb, char *key ) {
    configDetail *cd;
    int indexes = 0;
    if (cb == NULL || cb->details == NULL)
        return 0;
    for (cd=cb->details; cd != NULL; cd = cd->next)
        if (compare(cd->key, key) == 0)
            indexes++;
    return indexes;
}

/* INI Method */

configBlock* ini_read() {
    FILE *f;
    char buffer[1024], clave[512], valor[512];
    int bs = 0, i, j, index;
    configBlock *cb = NULL, *pcb = NULL;
    configDetail *pcd = NULL;
    int name_flag = 0;
    char key[STRING_SIZE];

    f = fopen(__CONFIG_DIR "/http.ini", "rt");
    if (f != NULL) {
        do {
            fgets(buffer, sizeof(buffer), f);
            if (!feof(f)) {
                chomp(buffer);
                trim(buffer);
                if (buffer[0] == '[') {
                    // seccion
                    if (pcb == NULL) {
                        cb = (configBlock *)malloc(sizeof(configBlock));
                        pcb = cb;
                    } else {
                        pcb->next = (configBlock *)malloc(sizeof(configBlock));
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
                        printf("Error en fichero de configuración, detalle fuera de bloque.\n");
                        exit(1);
                    }
                    if (pcb->details == NULL) {
                        pcb->details = (configDetail *)malloc(sizeof(configDetail));
                        pcd = pcb->details;
                    } else {
                        for (pcd = pcb->details; pcd->next != NULL; pcd = pcd->next)
                            ;
                        pcd->next = (configDetail *)malloc(sizeof(configDetail));
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
                                trim(pcd->key);
                                copy(pcd->key, key);
                                j = -1;
                                continue;
                            }
                            pcd->key[j] = buffer[i];
                        } else {
                            if (buffer[i] == ',') {
                                index = pcd->index;
                                pcd->value[j] = '\0';
                                trim(pcd->value);
                                j = -1;
                                pcd->next = (configDetail *)malloc(sizeof(configDetail));
                                pcd = pcd->next;
                                copy(key, pcd->key);
                                pcd->index = ++index;
                                continue;
                            }
                            pcd->value[j] = buffer[i];
                        }
                    }
                    pcd->value[j] = '\0';
                    trim(pcd->value);
                }
            }
        } while(!feof(f));
        fclose(f);
    }
    return cb;
}
