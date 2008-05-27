/* -*- mode:C; coding:utf-8 -*- */

#if !defined __CONFIGURATOR_H
#define __CONFIGURATOR_H

#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "string.h"
#include "memory.h"
#include "logger.h"

#define STRING_SIZE 80

/**
 *  Data structure with its key.
 */
struct Config_Detail {
    char key[STRING_SIZE];   //!< detail key.
    char value[STRING_SIZE]; //!< detail value.
    int  index;              //!< value index, in multivaluated case.
    struct Config_Detail *next;
};

typedef struct Config_Detail configDetail;

/**
 *  Data structure for config blocks.
 */
struct Config_Block {
    char name[STRING_SIZE];     //!< block name.
    char lastname[STRING_SIZE]; //!< block lastname.
    configDetail *details;      //!< block details.
    struct Config_Block *next;
};

typedef struct Config_Block configBlock;

// TODO: this structure will be used when "ews_get_initial_conf" will be implemented
/**
 *  Data structure for configuration load functions.
 */
struct Config_Funcs {
    configBlock* (*read)();  //!< config read function.
    char *name;              //!< function name.
};

typedef struct Config_Funcs configFuncs;


/**
 *  Builds a config block.
 *
 *  @param name block name.
 *  @param lastname block lastname.
 *  @return configBlock structure type.
 */
configBlock* ews_new_block( char *name, char *lastname );

/**
 *  Builds a config detail.
 *
 *  @param key detail key.
 *  @param value content value.
 *  @param index in multivaluated case, sort number.
 *  @return configDetail structure type.
 */
configDetail* ews_new_detail( char *key, char *value, int index );

/**
 *  Free all config blocks.
 *
 *  @param cb head pointer to configBlock.
 */
void ews_free_blocks( configBlock *cb );

/**
 *  Free all config details.
 *
 *  @param cd head pointer to configDetail.
 */
void ews_free_details( configDetail *cd );

/**
 *  Gets a pointer to requested configBlock.
 *
 *  @param cb head pointer to configBlock.
 *  @param name name to search.
 *  @param lastname lastname to search (can be NULL to be ignored)
 *  @return pointer to requested configBlock or NULL.
 */
configBlock* ews_get_block( configBlock *cb, char *name, char *lastname );

/**
 *  Gets a config requested detail key.
 *
 *  @param details head pointer to configDetail.
 *  @param value value to search.
 *  @param index index to search (by default is 0).
 *  @return pointer to requested detail key or NULL.
 */
char* ews_get_detail_key( configDetail *details, char *value, int index );

/**
 *  Gets a config requested detail value.
 *
 *  @param details head pointer to configDetail.
 *  @param key key to search.
 *  @param index index to search (by default is 0).
 *  @return pointer to requested detail value or NULL.
 */
char* ews_get_detail_value( configDetail *details, char *key, int index );

/**
 *  Gets values count.
 *
 *  @param details head pointer to configDetail.
 *  @param value value to search.
 *  @return found values number.
 */
int ews_get_detail_values( configDetail *details, char *value );

/**
 *  Gets values number by key.
 *
 *  @param details head pointer to configDetail.
 *  @param key key to search.
 *  @return found values number.
 */
int ews_get_detail_indexes( configDetail *details, char *key );

/**
 *  Gets IP address value from a string like 0.0.0.0:80.
 *
 *  @param cb pointer to a specific configBlock.
 *  @param key key name to get IP address.
 *  @param index value index to get IP address.
 *  @param s string to save IP adress.
 */
void ews_get_bindhost( configBlock *cb, char *key, int index, char *s );

/**
 *  Gets port value from a string like 0.0.0.0:80.
 *
 *  @param cb pointer to a specific configBlock.
 *  @param key key name to get IP address.
 *  @param index value index to get IP address.
 *  @return number port.
 */
int ews_get_bindport( configBlock *cb, char *key, int index );

#endif
