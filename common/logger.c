/* -*- mode:C; coding:utf-8 -*- */

#include "../include/ella.h"

#define EWS_LOGGER_MAX_FDS 128

static int log_level = LOG_LEVEL_DEBUG;
static char dateformat[256] = "%d/%m/%Y %H:%M:%S";

static int logger_fds[EWS_LOGGER_MAX_FDS];
static pthread_mutex_t logger_fds_mutex = PTHREAD_MUTEX_INITIALIZER;

static char *log_names[] = {
    "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

void logger_init() {
    int i;
    pthread_mutex_lock(&logger_fds_mutex);
    for (i=0; i<EWS_LOGGER_MAX_FDS; i++) {
        logger_fds[i] = -1;
    }
    pthread_mutex_unlock(&logger_fds_mutex);
}

void logger_config( struct Config_Block *cb ) {
    char *tmp;
    int level;

    if (cb == NULL || cb->details == NULL)
        return;

    tmp = ews_get_detail_value(cb->details, "dateformat", 0);
    if (tmp != NULL) {
        strcpy(dateformat, tmp);
    }
    tmp = ews_get_detail_value(cb->details, "loglevel", 0);
    if (tmp != NULL) {
        for (level = 0; level < 5; level++) {
            if (strcmp(log_names[level], tmp) == 0) {
                log_level = level;
                break;
            }
        }
    }
}

int logger_register( int fd ) {
    int i, ok = 0;
    pthread_mutex_lock(&logger_fds_mutex);
    for (i=0; i<EWS_LOGGER_MAX_FDS; i++) {
        if (logger_fds[i] == -1) {
            logger_fds[i] = fd;
            ok = 1;
            break;
        }
    }
    pthread_mutex_unlock(&logger_fds_mutex);
    return ok;
}

int logger_unregister( int fd ) {
    int i, ok = 0;
    pthread_mutex_lock(&logger_fds_mutex);
    for (i=0; i<EWS_LOGGER_MAX_FDS; i++) {
        if (logger_fds[i] == fd) {
            logger_fds[i] = -1;
            ok = 1;
            break;
        }
    }
    pthread_mutex_unlock(&logger_fds_mutex);
    return ok;
}

void ews_verbose( log_t type, const char *format, ... ) {
    char      buffer[4096] = { 0 },
              date[40] = { 0 },
              datefmt[4096] = { 0 };
    va_list   ap;
    time_t    t;
    struct tm tm;
    int       len,
              i;

    if (type < log_level)
        return;

    time(&t);
    localtime_r(&t, &tm);
    strftime(date, 40, dateformat, &tm);
    sprintf(datefmt, "[%s] %s: %s", date, log_names[type], format);
    format = (char *)datefmt;

    va_start(ap, format);
    len = strlen(buffer);
    vsnprintf(buffer + len, 4096 - len, format, ap);
    va_end(ap);

    ews_chomp(buffer);
    strcat(buffer, "\n");
#if defined EWS_VERBOSE_STDOUT
    printf("%s", buffer);
#endif

    // TODO: redireccionar a módulos de tipo log

    pthread_mutex_lock(&logger_fds_mutex);
    for (i=0; i<EWS_LOGGER_MAX_FDS; i++) {
        if (logger_fds[i] > -1) {
            write(logger_fds[i], buffer, strlen(buffer) + 1);
        }
    }
    pthread_mutex_unlock(&logger_fds_mutex);
}

void ews_verbose_to( int pipe, log_t type, const char *format, ... ) {
    char      buffer[4096] = { 0 },
              date[40] = { 0 },
              datefmt[4096] = { 0 };
    va_list   ap;
    time_t    t;
    struct tm tm;
    int       len;

    if (type < log_level)
        return;

    time(&t);
    localtime_r(&t, &tm);
    strftime(date, 40, dateformat, &tm);
    sprintf(datefmt, "[%s] %s: %s", date, log_names[type], format);
    format = (char *)datefmt;

    va_start(ap, format);
    len = strlen(buffer);
    vsnprintf(buffer + len, 4096 - len, format, ap);
    va_end(ap);

    ews_chomp(buffer);
    strcat(buffer, "\n");
#if defined EWS_VERBOSE_STDOUT
    printf("%s", buffer);
#endif

    pthread_mutex_lock(&logger_fds_mutex);
    write(pipe, buffer, strlen(buffer) + 1);
    pthread_mutex_unlock(&logger_fds_mutex);
}
