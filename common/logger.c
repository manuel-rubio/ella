/* -*- mode:C; coding:utf-8 -*- */

#include "../include/logger.h"

static int debug_level = LOG_LEVEL_INFO;
static char dateformat[256] = "%d/%m/%Y %H:%M:%S";

static int logger_fds[128] = { 0 };
pthread_mutex_t logger_fds_mutex = PTHREAD_MUTEX_INITIALIZER;

char *log_names[] = {
    "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

void set_date_format( const char *s ) {
    strcpy(dateformat, s);
}

void set_debug_level( int type ) {
    debug_level = type;
}

int logger_register( int fd ) {
    int i, ok = 0;
    pthread_mutex_lock(&logger_fds_mutex);
    for (i=0; i<sizeof(logger_fds); i++) {
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
    for (i=0; i<sizeof(logger_fds); i++) {
        if (logger_fds[i] == fd) {
            logger_fds[i] = -1;
            ok = 1;
            break;
        }
    }
    pthread_mutex_unlock(&logger_fds_mutex);
    return ok;
}

static int fdprint(int fd, const char *s) {
    return write(fd, s, strlen(s) + 1);
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

    if (type < debug_level)
        return;

    time(&t);
    localtime_r(&t, &tm);
    strftime(date, sizeof(date), dateformat, &tm);
    sprintf(datefmt, "[%s] %s: %s", date, log_names[type], format);
    format = &datefmt;

    va_start(ap, format);
    len = strlen(buffer);
    vsnprintf(buffer + len, sizeof(buffer) - len, format, ap);
    va_end(ap);

    // TODO: habrá que direccionar esto a fichero
    ews_chomp(buffer);
    strcat(buffer, "\n");
    printf("%s", buffer);

    // TODO: redireccionar a módulos de tipo log
    for (i=0; i<sizeof(logger_fds); i++) {
        if (logger_fds[i] > 0) {
            fdprint(logger_fds[i], buffer);
        }
    }
}
