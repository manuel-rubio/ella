/* -*- mode:C; coding:utf-8 -*- */

#if !defined __EWS_MEMORY_H
#define __EWS_MEMORY_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "config.h"
#include "logger.h"

#define MAX_MEM_ALLOCS 8192

/**
 *  Allocate memory.
 *
 *  @param size size for allocation memory.
 *  @return pointer to allocated memory.
 */
void* ews_malloc( int size );

/**
 *  Free memory.
 *
 *  @param ptr pointer to allocated memory.
 *  @param name function name (to see in log messages in error case).
 */
void ews_free( void *ptr, const char *name );

/**
 *  Free all allocated memory.
 */
void ews_free_all( void );

/**
 *  Show memory stats as LOG_LEVEL_INFO.
 */
void ews_memory_stats( void );

/**
 *  Show memory stats in a console as LOG_LEVEL_INFO.
 */
int ews_memory_cli_stats( int pipe, char *params );

#endif
