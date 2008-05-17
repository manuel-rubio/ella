/* -*- mode:C; coding:utf-8 -*- */

#if !defined __LOGGER_H
#define __LOGGER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#include "string.h"

typedef enum {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL
} log_t;

void ews_verbose( log_t type, const char *format, ... );
void set_date_format( const char *s );
void set_debug_level( int type );

#endif
