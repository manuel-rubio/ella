/* -*- mode:C; coding:utf-8 -*- */

#if !defined __EWS_MEMORY_H
#define __EWS_MEMORY_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "config.h"
#include "logger.h"

#define MAX_MEM_ALLOCS 8192

void* ews_malloc( int size );
void ews_free( void *ptr, const char *name );
void ews_free_all( void );
void ews_memory_stats( void );

#endif
