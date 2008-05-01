/* -*- mode:C; coding:utf-8 -*- */

#if !defined __TOR_MEMORY_H
#define __TOR_MEMORY_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "config.h"

#define MAX_MEM_ALLOCS 8192

void* tor_malloc( int size );
void tor_free( void *ptr, const char *name );
void tor_free_all( void );
void tor_memory_stats( void );

#endif
