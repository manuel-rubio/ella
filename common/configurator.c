/* -*- mode:C; coding:utf-8 -*- */

#include "../include/configurator.h"

configBlock* tor_new_block( char *name, char *lastname ) {
    configBlock* cb = (configBlock *)tor_malloc(sizeof(configBlock));
    cb->details = NULL;
    strcpy(cb->name, name);
    strcpy(cb->lastname, lastname);
    cb->next = NULL;
    return cb;
}

configDetail* tor_new_detail( char *key, char *value, int index ) {
    configDetail* cd = (configDetail *)tor_malloc(sizeof(configDetail));
    strcpy(cd->key, key);
    strcpy(cd->value, value);
    cd->index = index;
    cd->next = NULL;
    return cd;
}

void tor_free_blocks( configBlock *cb ) {
    if (cb == NULL) {
        return;
    }
    if (cb->details != NULL) {
        tor_free_details(cb->details);
    }
    if (cb->next != NULL) {
        tor_free_blocks(cb->next);
    }
    tor_free(cb, "tor_free_blocks");
}

void tor_free_details( configDetail *cd ) {
    if (cd == NULL) {
        return;
    }
    if (cd->next != NULL) {
        tor_free_details(cd->next);
    }
    tor_free(cd, "tor_free_details");
}

configBlock* tor_get_block( configBlock *cb, char *name, char *lastname ) {
    for (;cb != NULL; cb = cb->next) {
        if (lastname) {
            if (strcmp(cb->name, name) == 0 && strcmp(cb->lastname, lastname) == 0)
                return cb;
        } else {
            if (strcmp(cb->name, name) == 0)
                return cb;
        }
    }
    return NULL;
}

char* tor_get_detail_value( configDetail *details, char *key, int index ) {
    configDetail *cd;
    if (details == NULL)
        return NULL;
    for (cd=details; cd != NULL; cd = cd->next)
        if (strcmp(cd->key, key) == 0 && cd->index == index)
            return cd->value;
    return NULL;
}

char* tor_get_detail_key( configDetail *details, char *value, int index ) {
    configDetail *cd;
    int count = 0;
    if (details == NULL)
        return NULL;
    for (cd=details; cd != NULL; cd = cd->next) {
        if (strcmp(cd->value, value) == 0) {
            if (count == index)
                return cd->key;
            count++;
        }
    }
    return NULL;
}

int tor_get_detail_values( configDetail *details, char *value ) {
    configDetail *cd;
    int indexes = 0;
    if (details == NULL)
        return 0;
    for (cd=details; cd != NULL; cd = cd->next)
        if (strcmp(cd->value, value) == 0)
            indexes++;
    return indexes;
}

int tor_get_detail_indexes( configDetail *details, char *key ) {
    configDetail *cd;
    int indexes = 0;
    if (details == NULL)
        return 0;
    for (cd=details; cd != NULL; cd = cd->next)
        if (strcmp(cd->key, key) == 0)
            indexes++;
    return indexes;
}

void tor_get_bindhost( configBlock *cb, char *key, int index, char *s ) {
    char *host = tor_get_detail_value(cb->details, key, index);
    int i, capture;

    if (host == NULL)
        return;
    for (i=0; host[i]!='\0' && host[i]!=':'; i++)
        s[i] = host[i];
    s[i] = '\0';
}

int tor_get_bindport( configBlock *cb, char *key, int index ) {
    char *host = tor_get_detail_value(cb->details, key, index);
    char s[10];
    int i, j, capture;

    if (host == NULL)
        return;
    for (i=0, j=0, capture=0; host[i]!='\0'; i++) {
        if (capture)
            s[j++] = host[i];
        else if (host[i] == ':')
            capture = 1;
    }
    s[j] = '\0';
    return atoi(s);
}
