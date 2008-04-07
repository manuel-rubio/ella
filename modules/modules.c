/* -*- mode:C; coding:utf-8 -*- */

#include "../include/modules/modules.h"

moduleTAD* tor_modules_load( configBlock *cb ) {
    int indexes, i;

    if (tor_compare(tor_get_detail_value(cb, "autoload", 0), "yes") == 0) {
        printf("AutoCarga: sí\n");
    } else {
        printf("AutoCarga: no\n");
    }

    indexes = tor_get_detail_indexes(cb, "load");
    printf("Load: ");
    for (i=0; i<indexes; i++) {
        printf("[%s] ", tor_get_detail_value(cb, "load", i));
    }

    printf("\nNoLoad: ");
    indexes = tor_get_detail_indexes(cb, "noload");
    for (i=0; i<indexes; i++) {
        printf("[%s] ", tor_get_detail_value(cb, "unload", i));
    }

    printf("\n");
    return NULL;
}
