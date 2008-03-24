/* -*- mode:C; coding:utf-8 -*- */

#if !defined __CONFIG_CONFIG_H
#define __CONFIG_CONFIG_H

#include <stdio.h>
#include "../config.h"

struct Config_Detail {
    char *key;
    char **values;
    int  nvalues;
    struct Config_Detail *next;
};

typedef struct Config_Detail configDetail;

struct Config_Block {
    char *name;
    char *lastname;
    configDetail *details;
    struct Config_Block *next;
};

typedef struct Config_Block configBlock;

struct Config_Funcs {
    configBlock* (*read)();
    char *name;
};

typedef struct Config_Funcs configFuncs;

configFuncs get_initial_conf();

/* INI method */
configBlock* ini_read();

#endif
