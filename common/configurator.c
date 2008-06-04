/* -*- mode:C; coding:utf-8 -*- */

#include "../include/configurator.h"

configBlock* ews_new_block( char *name, char *lastname ) {
    configBlock* cb = (configBlock *)ews_malloc(sizeof(configBlock));
    cb->details = NULL;
    strcpy(cb->name, name);
    strcpy(cb->lastname, lastname);
    cb->next = NULL;
    return cb;
}

configDetail* ews_new_detail( char *key, char *value, int index ) {
    configDetail* cd = (configDetail *)ews_malloc(sizeof(configDetail));
    strcpy(cd->key, key);
    strcpy(cd->value, value);
    cd->index = index;
    cd->next = NULL;
    return cd;
}

void ews_free_blocks( configBlock *cb ) {
    if (cb == NULL) {
        return;
    }
    if (cb->details != NULL) {
        ews_free_details(cb->details);
        cb->details = NULL;
    }
    if (cb->next != NULL) {
        ews_free_blocks(cb->next);
        cb->next = NULL;
    }
    ews_free(cb, "ews_free_blocks");
}

void ews_free_details( configDetail *cd ) {
    if (cd == NULL) {
        return;
    }
    if (cd->next != NULL) {
        ews_free_details(cd->next);
        cd->next = NULL;
    }
    ews_free(cd, "ews_free_details");
}

configBlock* ews_get_block( configBlock *cb, char *name, char *lastname ) {
    for (;cb != NULL; cb = cb->next) {
        if (lastname) {
            if (strcmp(cb->name, name) == 0 && strcmp(cb->lastname, lastname) == 0)
                return cb;
        } else {
            if (strcmp(cb->name, name) == 0)
                return cb;
        }
    }
    if (lastname) {
        ews_verbose(LOG_LEVEL_DEBUG, "block [%s:%s] not found", name, lastname);
    } else {
        ews_verbose(LOG_LEVEL_DEBUG, "block [%s] not found", name);
    }
    return NULL;
}

char* ews_get_detail_value( configDetail *details, char *key, int index ) {
    configDetail *cd;
    if (details == NULL) {
        ews_verbose(LOG_LEVEL_DEBUG, "details not found!");
        return 0;
    }
    for (cd=details; cd != NULL; cd = cd->next)
        if (strcmp(cd->key, key) == 0 && cd->index == index)
            return cd->value;
    ews_verbose(LOG_LEVEL_DEBUG, "detail not found: %s[%d]", key, index);
    return NULL;
}

char* ews_get_detail_key( configDetail *details, char *value, int index ) {
    configDetail *cd;
    int count = 0;
    if (details == NULL) {
        ews_verbose(LOG_LEVEL_DEBUG, "details not found!");
        return 0;
    }
    for (cd=details; cd != NULL; cd = cd->next) {
        if (strcmp(cd->value, value) == 0) {
            if (count == index)
                return cd->key;
            count++;
        }
    }
    ews_verbose(LOG_LEVEL_DEBUG, "key for value [%s] in index [%d] not found", value, index);
    return NULL;
}

int ews_get_detail_values( configDetail *details, char *value ) {
    configDetail *cd;
    int indexes = 0;
    if (details == NULL) {
        ews_verbose(LOG_LEVEL_DEBUG, "details not found!");
        return 0;
    }
    for (cd=details; cd != NULL; cd = cd->next)
        if (strcmp(cd->value, value) == 0)
            indexes++;
    return indexes;
}

int ews_get_detail_indexes( configDetail *details, char *key ) {
    configDetail *cd;
    int indexes = 0;
    if (details == NULL) {
        ews_verbose(LOG_LEVEL_DEBUG, "details not found!");
        return 0;
    }
    for (cd=details; cd != NULL; cd = cd->next)
        if (strcmp(cd->key, key) == 0)
            indexes++;
    return indexes;
}

void ews_get_bindhost( configBlock *cb, char *key, int index, char *s ) {
    char *host = ews_get_detail_value(cb->details, key, index);
    int i, capture;

    if (host == NULL) {
        ews_verbose(LOG_LEVEL_DEBUG, "host not found! in [%s:%s]", cb->name, cb->lastname);
        return;
    }
    for (i=0; host[i]!='\0' && host[i]!=':'; i++)
        s[i] = host[i];
    s[i] = '\0';
}

int ews_get_bindport( configBlock *cb, char *key, int index ) {
    char *host = ews_get_detail_value(cb->details, key, index);
    char s[10];
    int i, j, capture;

    if (host == NULL) {
        ews_verbose(LOG_LEVEL_DEBUG, "host not found! in [%s:%s]", cb->name, cb->lastname);
        return;
    }
    for (i=0, j=0, capture=0; host[i]!='\0'; i++) {
        if (capture) {
            if (host[i] == '/') {
                break;
            }
            s[j++] = host[i];
        } else if (host[i] == ':') {
            capture = 1;
        }
    }
    s[j] = '\0';
    return atoi(s);
}

int ews_get_bindproto( configBlock *cb, char *key, int index ) {
    char *host = ews_get_detail_value(cb->details, key, index);
    char *aux;
    int len;

    if (host != NULL) {
        len = strlen(host) - 3;
        if (len > 0) {
            aux = host + len;
            if (strcmp("tcp", aux) == 0) {
                return IPPROTO_TCP;
            } else if (strcmp("udp", aux) == 0) {
                return IPPROTO_UDP;
            }
        }
        // TCP is default behaviour
        return IPPROTO_TCP;
    }
    return -1;
}
