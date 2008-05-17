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
 *  Estructura de datos con su clave.
 *
 *  Este conjunto de datos, formado por una clave, un valor
 *  y su índice (para el caso de que sea un dato multivaluado),
 *  es el átomo que formará parte de los bloques de configuración,
 *  así como otras estructuras a lo largo del código.
 */
struct Config_Detail {
    char key[STRING_SIZE];   //!< clave del detalle.
    char value[STRING_SIZE]; //!< valor del detalle.
    int  index;              //!< índice del valor, en caso de multivaluado.
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
    char name[STRING_SIZE];     //!< nombre del bloque.
    char lastname[STRING_SIZE]; //!< subnombre del bloque.
    configDetail *details;      //!< detalles del bloque.
    struct Config_Block *next;
};

typedef struct Config_Block configBlock;

// TODO: esta estructura se usará cuando se implemente "ews_get_initial_conf"
/**
 *  Estructura de datos para funciones de carga de configuración.
 *
 *  En este código se encuentra la función estática por defecto para
 *  la lectura de ficheros de tipo INI. Para anexionar un módulo, se
 *  tendrá que crear una función de tipo "read" tal y como se indica
 *  en esta estructura.
 */
struct Config_Funcs {
    configBlock* (*read)();  //!< función de lectura de configuración.
    char *name;              //!< nombre de la función.
};

typedef struct Config_Funcs configFuncs;


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
configBlock* ews_new_block( char *name, char *lastname );

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
configDetail* ews_new_detail( char *key, char *value, int index );

/**
 *  Libera los bloques de configuración.
 *
 *  Se encarga de liberar, recursivamente, todos los bloques de
 *  configuración enlazados, que se hayan pasado como parámetro.
 *
 *  @param cb puntero a configBlock de cabecera.
 */
void ews_free_blocks( configBlock *cb );

/**
 *  Libera los detalles de configuración.
 *
 *  Se encarga de liberar, recursivamente, todos los detalles de
 *  configuración enlazados, que se hayan pasado como parámetro.
 *
 *  @param cd puntero a configDetail de cabecera.
 */
void ews_free_details( configDetail *cd );

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
configBlock* ews_get_block( configBlock *cb, char *name, char *lastname );

/**
 *  Toma la clave de un detalle solicitado de un bloque de configuración.
 *
 *  Busca entre los detalles de configuración de un bloque por el valor
 *  y el índice, y retorna su clave.
 *
 *  @param details puntero a configDetail de cabecera.
 *  @param value valor por el que buscar.
 *  @param index índice en el que buscar (por defecto es 0).
 *  @return puntero a la clave del detalle solicitado o NULL.
 */
char* ews_get_detail_key( configDetail *details, char *value, int index );

/**
 *  Toma el valor de un detalle solicitado de un bloque de configuración.
 *
 *  Busca entre las claves de configuración de un bloque por el valor
 *  y el índice, y retorna su valor.
 *
 *  @param details puntero a configDetail de cabecera.
 *  @param key clave por la que buscar.
 *  @param index índie en el que buscar (por defecto es 0).
 *  @return puntero al valor del detalle solicitado o NULL.
 */
char* ews_get_detail_value( configDetail *details, char *key, int index );

/**
 *  Toma el número de valores.
 *
 *  Cuenta todos los valores coincidentes con el parámetro pasado
 *  como parámetro.
 *
 *  @param details puntero a configDetail de cabecera.
 *  @param value valor por el que buscar los coincidentes.
 *  @return el número de valores encontrados.
 */
int ews_get_detail_values( configDetail *details, char *value );

/**
 *  Toma el número de valores que tiene una clave.
 *
 *  Cuenta los valores que tiene una clave determinada.
 *
 *  @param details puntero a configDetail de cabecera.
 *  @param key clave de la que tomar los valores.
 *  @return el número de valores encontrados.
 */
int ews_get_detail_indexes( configDetail *details, char *key );

/**
 *  Toma el valor de la IP de una cadena del tipo 0.0.0.0:80.
 *
 *  Disecciona un valor que contiene una dirección IP y un puerto
 *  anexionado, tomando solo el valor de la dirección IP.
 *
 *  @param cb puntero a configBlock específico.
 *  @param key clave de la que tomar la dirección IP.
 *  @param index índice del que tomar el valor.
 *  @param s cadena en la que guardar la dirección IP.
 */
void ews_get_bindhost( configBlock *cb, char *key, int index, char *s );

/**
 *  Toma el valor del puerto de una cadena del tipo 0.0.0.0:80.
 *
 *  Disecciona un valor que contiene una dirección IP y un puerto
 *  anexionado, tomando solo el valor del puerto.
 *
 *  @param cb puntero a configBlock específico.
 *  @param key clave de la que tomar el puerto.
 *  @param index índice del que tomar el valor.
 *  @return el puerto tomado.
 */
int ews_get_bindport( configBlock *cb, char *key, int index );

#endif
