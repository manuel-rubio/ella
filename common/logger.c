/* -*- mode:C; coding:utf-8 -*- */

#include "../include/logger.h"

static int debug_level = LOG_LEVEL_INFO;
static char dateformat[256] = "%d/%m/%Y %H:%M:%S";

char *log_names[] = {
    "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

void set_date_format( const char *s ) {
    strcpy(dateformat, s);
}

void set_debug_level( int type ) {
    debug_level = type;
}

void ews_verbose( log_t type, const char *format, ... ) {
    char buffer[4096] = { 0 }, date[40] = { 0 }, datefmt[4096] = { 0 };
    va_list ap;
    time_t t;
    struct tm tm;
    int len;

    if (type < debug_level)
        return;

    time(&t);
    localtime_r(&t, &tm);
    strftime(date, sizeof(date), dateformat, &tm);
    sprintf(&datefmt, "[%s] %s: %s", date, log_names[type], format);
    format = &datefmt;

    va_start(ap, format);
    len = strlen(buffer);
    vsnprintf(buffer + len, sizeof(buffer) - len, format, ap);
    va_end(ap);

    // TODO: habrá que direccionar esto a fichero
    ews_chomp(buffer);
    printf("%s\n", buffer);

    // TODO: redireccionar a módulos de tipo log
}
