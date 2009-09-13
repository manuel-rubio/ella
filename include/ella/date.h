/* -*- mode:C; coding:utf-8 -*- */

#if !defined __DATE_H
#define __DATE_H

#include <string.h>
#include <time.h>
#include <stdio.h>
#include <sys/stat.h>

enum {
    EWS_DATE_RFC1123,
    EWS_DATE_RFC1036,
    EWS_DATE_ANSIC
};

/**
 *  Get current date in RFC-1125 format.
 *
 *  @param s string to set date.
 */
void get_current_date( char *s );

/**
 *  Get file date in RFC-1125 format.
 *
 *  @param s string to set date.
 *  @param file file to get date.
 */
void get_file_date( char *s, char *file );

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
