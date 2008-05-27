/* -*- mode:C; coding:utf-8 -*- */

#if !defined __EWS_STRING_H
#define __EWS_STRING_H

#include <string.h>

/**
 *  Erase right blanks.
 *
 *  @param s string to trim.
 *  @return pointer to same string.
 */
char* ews_rtrim( char* s );

/**
 *  Erase left blanks.
 *
 *  @param s string to trim.
 *  @return pointer to same string.
 */
char* ews_ltrim( char* s );

/**
 *  Erase both right and left blanks.
 *
 *  @param s string to trim.
 *  @return pointer to same string.
 */
char* ews_trim( char* s );

/**
 *  Erase string end return (\\n y \\r).
 *
 *  @param s string to chop.
 *  @return pointer to same string.
 */
char* ews_chomp( char *s );

#endif
