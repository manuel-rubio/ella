/* -*- mode:C; coding:utf-8 -*- */

#if !defined __CONFIG_CONFIG_H
#define __CONFIG_CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include "../config.h"

#define STRING_SIZE 80

/**
 *  Estructura de datos con su clave.
 *
 *  Este conjunto de datos, formado por una clave, un valor
 *  y su índice (para el caso de que sea un dato multivaluado),
 *  es el átomo que formará parte de los bloques de configuración,
 *  así como otras estructuras a lo largo del código.
 */
struct Config_Detail {
    char key[STRING_SIZE];
    char value[STRING_SIZE];
    int  index;
    struct Config_Detail *next;
};

typedef struct Config_Detail configDetail;

/**
 *  Estructura de datos para bloques de configuración.
 *
 *  Se entiende por un bloque de configuración, un conjunto
 *  de valores con su clave, que se almacena bajo un nombre
 *  y subnombre específico.
 */
struct Config_Block {
    char name[STRING_SIZE];
    char lastname[STRING_SIZE];
    configDetail *details;
    struct Config_Block *next;
};

typedef struct Config_Block configBlock;

// TODO: esta estructura se usará cuando se implemente "tor_get_initial_conf"
struct Config_Funcs {
    configBlock* (*read)();
    char *name;
};

typedef struct Config_Funcs configFuncs;


/**
 *  Inicializa el sistema de lectura de configuración.
 *
 *  Esta función carga las funciones esenciales para proceder
 *  con la carga de los valores de configuración del servidor.
 */
configFuncs tor_get_initial_conf();

/**
 *  Crea un bloque de configuración.
 *
 *  Se encarga de crear un bloque de configuración, reservando
 *  memoria para él, y asignándole los valores pasados como
 *  parámetros.
 *
 *  @param name nombre del bloque.
 *  @param lastname subnombre del bloque.
 *  @return estructura de tipo configBlock.
 */
configBlock* tor_new_block( char *name, char *lastname );

/**
 *  Crea un detalle de configuración.
 *
 *  Se encarga de crear un detalle de configuración, reservando
 *  memoria para él, y asignándole los valores pasados como
 *  parámetros.
 *
 *  @param key clave del detalle.
 *  @param value valor contenido.
 *  @param index en caso de multivaluado, su número de orden.
 *  @return estructura de tipo configDetail.
 */
configDetail* tor_new_detail( char *key, char *value, int index );

/**
 *  Libera los bloques de configuración.
 *
 *  Se encarga de liberar, recursivamente, todos los bloques de
 *  configuración enlazados, que se hayan pasado como parámetro.
 *
 *  @param cb puntero a configBlock de cabecera.
 */
void tor_free_blocks( configBlock *cb );

/**
 *  Libera los detalles de configuración.
 *
 *  Se encarga de liberar, recursivamente, todos los detalles de
 *  configuración enlazados, que se hayan pasado como parámetro.
 *
 *  @param cd puntero a configDetail de cabecera.
 */
void tor_free_details( configDetail *cd );

/**
 *  Toma un puntero al configBlock solicitado.
 *
 *  Busca en la lista enlazada el configBlock por nombre y subnombre,
 *  pasados como parámetros, y retorna un puntero al bloque encontrado
 *  o NULL en caso de no encontrarlo.
 *
 *  @param cb puntero a configBlock de cabecera.
 *  @param name valor del nombre por el que buscar.
 *  @param lastname valor del subnombre por el que buscar (puese ser NULL)
 *  @return puntero a configBlock solicitado o NULL.
 */
configBlock* tor_get_block( configBlock *cb, char *name, char *lastname );

/**
 *  Toma la clave de un detalle solicitado de un bloque de configuración.
 *
 *  Busca entre los detalles de configuración de un bloque por el valor
 *  y el índice, y retorna su clave.
 *
 *  @param cb puntero a configBlock de cabecera.
 *  @param value valor por el que buscar.
 *  @param index índice en el que buscar (por defecto es 0).
 *  @return puntero a la clave del detalle solicitado o NULL.
 */
char* tor_get_detail_key( configBlock *cb, char *value, int index );

char* tor_get_detail_value( configBlock *cb, char *key, int index );

int tor_get_detail_values( configBlock *cb, char *value );

int tor_get_detail_indexes( configBlock *cb, char *key );

void tor_get_bindhost( configBlock *cb, char *key, int index, char *s );

int tor_get_bindport( configBlock *cb, char *key, int index );

/* INI method */
configBlock* tor_ini_read();

#endif
