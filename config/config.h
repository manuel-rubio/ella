/* -*- mode:C; coding:utf-8 -*- */

#if !defined __CONFIG_CONFIG_H
#define __CONFIG_CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include "../config.h"

#define STRING_SIZE 80

struct Config_Detail {
    char key[STRING_SIZE];
    char value[STRING_SIZE];
    int  index;
    struct Config_Detail *next;
};

typedef struct Config_Detail configDetail;

struct Config_Block {
    char name[STRING_SIZE];
    char lastname[STRING_SIZE];
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

configBlock* new_block( char *name, char *lastname );
configDetail* new_detail( char *key, char *value, int index );

void free_blocks( configBlock *cb );
void free_details( configDetail *cd );

configBlock* get_block( configBlock *cb, char *name, char *lastname );
char* get_detail_value( configBlock *cb, char *key, int index );
int get_detail_indexes( configBlock *cb, char *key );

/* INI method */
configBlock* ini_read();

#endif
