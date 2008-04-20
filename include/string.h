/* -*- mode:C; coding:utf-8 -*- */

#include <string.h>

/**
 *  Elimina espacios en blanco por la derecha.
 *
 *  @param s cadena a modificar.
 *  @return puntero a la misma cadena.
 */
char* tor_rtrim( char* s );

/**
 *  Elimina espacios en blanco por la izquierda.
 *
 *  @param s cadena a modificar.
 *  @return puntero a la misma cadena.
 */
char* tor_ltrim( char* s );

/**
 *  Elimina espacios en blanco por la derecha e izquierda.
 *
 *  @param s cadena a modificar.
 *  @return puntero a la misma cadena.
 */
char* tor_trim( char* s );

/**
 *  Elimina el retorno de carro (\\n y \\r) del final de la cadena.
 *
 *  @param s cadena a modificar.
 *  @return puntero a la misma cadena.
 */
char* tor_chomp( char *s );
