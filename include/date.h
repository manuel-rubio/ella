/* -*- mode:C; coding:utf-8 -*- */

#if !defined __DATE_H
#define __DATE_H

#include <time.h>
#include <stdio.h>
#include <sys/stat.h>

/**
 *  Set current date in RFC-1125 format.
 *
 *  @param s string to set date.
 */
void set_current_date( char *s );

/**
 *  Set file date in RFC-1125 format.
 *
 *  @param s string to set date.
 *  @param file file to get date.
 */
void set_file_date( char *s, char *file );

/**
 *  Compare between two dates and return a negative value
 *  if date1 is lesser than date2 and a positive value
 *  else.
 *
 *  @param date1 date in RFC-1125 format.
 *  @param date2 date in RFC-1125 format.
 */
int compare_date( char *date1, char *date2 );

#endif
