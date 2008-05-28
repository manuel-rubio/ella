/* -*- mode:C; coding:utf-8 -*- */

#if !defined __LOGGER_H
#define __LOGGER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <pthread.h>

#include "config.h"
#include "string.h"

/**
 *  Levels for log messages.
 */
typedef enum {
    LOG_LEVEL_DEBUG,   //!< minimum level, used to conflict algorithm messages.
    LOG_LEVEL_INFO,    //!< lightweight messages to inform about conditional and conflict codes.
    LOG_LEVEL_WARN,    //!< warns about possible errors.
    LOG_LEVEL_ERROR,   //!< errors in configuration or code.
    LOG_LEVEL_FATAL    //!< errors in system (asserts), this errors never you should see.
} log_t;

/**
 *  Send a verbose message to all consoles and log systems.
 *
 *  @param type log level.
 *  @param format use a format like printf.
 *  @param ... add vars to see in log message.
 */
void ews_verbose( log_t type, const char *format, ... );

/**
 *  Send a verbose message to a specific console.
 *
 *  @param pipe console to send message.
 *  @param type log level.
 *  @param format use a format like printf.
 *  @param ... add vars to see in log message.
 */
void ews_verbose_to( int pipe, log_t type, const char *format, ... );

/**
 *  Set date format to all verbose messages.
 *
 *  @param s format to verbose messages.
 */
void set_date_format( const char *s );

/**
 *  Set log level for all verbose messages.
 *
 *  @param type minimum type to will be showed.
 */
void set_log_level( int type );

/**
 *  Register a logger.
 *
 *  @param fd file descriptor for the logger.
 */
int logger_register( int fd );

/**
 *  Unregister a logger.
 *
 *  @param fd file descriptor for the logger.
 */
int logger_unregister( int fd );

#endif
