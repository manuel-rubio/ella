#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#include "../include/modules.h"
#include "../include/header.h"
#include "../include/connector.h"
#include "../include/configurator.h"

#define BUFFER_SIZE 1024

#define METHOD_GET     1
#define METHOD_POST    2
#define METHOD_HEAD    3

char *page404 = "\
<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n\
<html><head>\n\
<title>404 - Not found</title>\n\
</head><body>\n\
<h1>Not found</h1>\n\
<p>The requested URL %s was not found on this server.</p>\n\
<hr>\n\
<address>Ella Web Server/0.1</address>\n\
</body></html>";

char *page501 = "\
<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n\
<html><head>\n\
<title>501 - Not implemented</title>\n\
</head><body>\n\
<h1>Method not implemented</h1>\n\
<p>The request method isn't implemented.</p>\n\
<hr>\n\
<address>Ella Web Server/0.1</address>\n\
</body></html>";


void http10_get_status( char *s ) {
    // TODO: especificar en "s" el estado del módulo
    strcpy(s, "HTTP 1.0 - RFC1945 - Process module without dynamic information.");
}

void http10_setcurrentdate( char *s ) {
    struct tm *ft;
    time_t tt;
    char *wdays[] = {
        "Sun", "Mon", "Tue", "Wed",
        "Thu", "Fri", "Sat", "Sun"
    };
    char *month_names[] = {
        "Jan", "Feb", "Mar", "Apr",
        "May", "Jun", "Jul", "Aug",
        "Sep", "Oct", "Nov", "Dec"
    };

    tt = time(NULL);
    ft = gmtime(&tt);
    sprintf(s, "%s, %02d %s %4d %02d:%02d:%02d GMT",
        wdays[ft->tm_wday],
        ft->tm_mday,
        month_names[ft->tm_mon],
        ft->tm_year + 1900,
        ft->tm_hour,
        ft->tm_min,
        ft->tm_sec
    );
}

void http10_setdate( char *s, char *file ) {
    struct stat status;
    struct tm *ft;
    char *wdays[] = {
        "Sun", "Mon", "Tue", "Wed",
        "Thu", "Fri", "Sat", "Sun"
    };
    char *month_names[] = {
        "Jan", "Feb", "Mar", "Apr",
        "May", "Jun", "Jul", "Aug",
        "Sep", "Oct", "Nov", "Dec"
    };

    stat(file, &status);
    ft = gmtime(&status.st_mtime);
    sprintf(s, "%s, %02d %s %4d %02d:%02d:%02d GMT",
        wdays[ft->tm_wday],
        ft->tm_mday,
        month_names[ft->tm_mon],
        ft->tm_year + 1900,
        ft->tm_hour,
        ft->tm_min,
        ft->tm_sec
    );
}

// TODO: corregir comparación de fechas
int http10_compare_date( char *date1, char *date2 ) {
    int dia1, dia2, mes1, mes2, agno1, agno2;
    int hora1, hora2, min1, min2, seg1, seg2;
    int gmt1, gmt2;
    int i;
    char m1[4] = { 0 }, m2[4] = { 0 };
    char *month_names[] = {
        "Jan", "Feb", "Mar", "Apr",
        "May", "Jun", "Jul", "Aug",
        "Sep", "Oct", "Nov", "Dec"
    };

    if (date1 == NULL)
        return -1;
    if (date2 == NULL)
        return 1;
    agno1 = ((date1[12] - '0') * 1000) + ((date1[13] - '0') * 100) + ((date1[14] - '0') * 10) + (date1[15] - '0');
    agno2 = ((date2[12] - '0') * 1000) + ((date2[13] - '0') * 100) + ((date2[14] - '0') * 10) + (date2[15] - '0');
    if (agno1 > agno2)
        return 1;
    if (agno1 < agno2)
        return -1;
    for (i=0; i<3; i++) {
        m1[i] = date1[i+8];
        m2[i] = date2[i+8];
    }
    for (i=0; i<12; i++) {
        if (strcmp(m1, month_names[i]) == 0)
            mes1 = i + 1;
        if (strcmp(m2, month_names[i]) == 0)
            mes2 = i + 1;
    }
    if (mes1 > mes2)
        return 1;
    if (mes1 < mes2)
        return -1;
    dia1 = ((date1[5] - '0') * 10) + (date1[6] - '0');
    dia2 = ((date2[5] - '0') * 10) + (date2[6] - '0');
    if (dia1 > dia2)
        return 1;
    if (dia1 < dia2)
        return -1;
    hora1 = ((date1[17] - '0') * 10) + (date1[18] - '0');
    hora2 = ((date2[17] - '0') * 10) + (date2[18] - '0');
    gmt1 = ((date1[27] - '0') * 10) + (date1[28] - '0');
    if (date1[26] == '-')
        hora1 -= gmt1;
    else
        hora1 += gmt1;
    gmt2 = ((date2[27] - '0') * 10) + (date2[28] - '0');
    if (date2[26] == '-')
        hora2 -= gmt2;
    else
        hora2 += gmt2;
    if (hora1 > hora2)
        return 1;
    if (hora1 < hora2)
        return -1;
    min1 = ((date1[20] - '0') * 10) + (date1[21] - '0');
    min2 = ((date2[20] - '0') * 10) + (date2[21] - '0');
    if (min1 > min2)
        return 1;
    if (min1 < min2)
        return -1;
    seg1 = ((date1[23] - '0') * 10) + (date1[24] - '0');
    seg2 = ((date2[23] - '0') * 10) + (date2[24] - '0');
    if (seg1 > seg2)
        return 1;
    if (seg1 < seg2)
        return -1;
    return 0;
}

// TODO: estos mensajes deben de ser páginas en directorios
void http10_error_page( int code, char *message, char *page, requestHTTP *rh, responseHTTP *rs, int method ) {
    char buffer[BUFFER_SIZE];

    bzero(buffer, BUFFER_SIZE);

    rs->code = code;
    strcpy(rs->message, message);
    strcpy(rs->version, "1.0");
    rs->headers = ews_new_header("Server", "Ella Web Server/0.1", 0);
    sprintf(buffer, page, rh->uri);
    if (method != METHOD_HEAD) {
        ews_set_response_content(rs, HEADER_CONTENT_STRING, buffer);
    }
}

int http10_run( struct Bind_Request *br, responseHTTP *rs ) {
    requestHTTP *rh = br->request;
    virtualHost *vh = NULL;
    hostLocation *hl = NULL;
    headerHTTP *hh = NULL;
    char *host_name = ews_get_header_value(rh, "Host", 0);
    char *path;
    char buffer[BUFFER_SIZE], date[80];
    int i, j, f, method = 0;

    bzero(buffer, BUFFER_SIZE);
    bzero(date, 80);

    if (strcmp(br->request->request, "GET") == 0) {
        method = METHOD_GET;
    } else if (strcmp(br->request->request, "POST") == 0) {
        method = METHOD_POST;
    } else if (strcmp(br->request->request, "HEAD") == 0) {
        method = METHOD_HEAD;
    } else {
        ews_verbose(LOG_LEVEL_ERROR, "method %s not implemented.", br->request->request);
        http10_error_page(501, "Not implemented", page501, rh, rs, METHOD_GET);
        return MODULE_RETURN_OK;
    }

    if (host_name != NULL) {
        vh = ews_connector_find_vhost((virtualHost *)br->bc->vhosts, host_name);
    }
    if (host_name == NULL || vh == NULL) {
        // tomamos default, el primer vhost que haya
        vh = (virtualHost *)br->bc->vhosts;
    }
    hl = ews_connector_find_location(vh->locations, rh->uri);
    if (hl == NULL) { // 404 - Not found
        ews_verbose(LOG_LEVEL_ERROR, "location no casa con ninguna de las configuradas.");
        http10_error_page(404, "Not found", page404, rh, rs, method);
        return MODULE_RETURN_OK;
    }
    if (!http10_find_file(buffer, rh, hl)) { // 404 - Not found
        ews_verbose(LOG_LEVEL_ERROR, "fichero %s no encontrado.", buffer);
        http10_error_page(404, "Not found", page404, rh, rs, method);
        return MODULE_RETURN_OK;
    }

    ews_verbose(LOG_LEVEL_INFO, "enviando fichero %s", buffer);
    strcpy(rs->version, "1.0");
    rs->headers = ews_new_header("Server", "ews/0.1", 0);
    hh = rs->headers;
    http10_setcurrentdate(date);
    hh->next = ews_new_header("Date", date, 0);
    http10_setdate(date, buffer);
    hh = hh->next;
    if (http10_compare_date(date, ews_get_header_value(rh, "If-Modified-Since", 0)) >= 0) {
        rs->code = 304;
        strcpy(rs->message, "Not Modified");
    } else {
        rs->code = 200;
        strcpy(rs->message, "OK");
        hh->next = ews_new_header("Last-Modified", date, 0);
    }
    if (method != METHOD_HEAD && rs->code == 200) {
        ews_set_response_content(rs, HEADER_CONTENT_FILE, buffer);
    }

    // TODO: implementar la gestión de cabeceras según el RFC1945
    return MODULE_RETURN_PROC_STOP;
}

int http10_find_file( char *buffer, requestHTTP *rh, hostLocation *hl ) {
    char *path = ews_get_detail_value(hl->details, "path", 0);
    int indexes = ews_get_detail_indexes(hl->details, "index");
    char *index;
    int i, j, k, f;
    struct stat st;

    bzero(buffer, BUFFER_SIZE);

    for (k=-1; k<indexes; k++) {
        for (j=0; path[j]!='\0'; j++) {
            buffer[j] = path[j];
        }
        if (buffer[j-1] != '/') {
            buffer[j++] = '/';
        }
        for (i=strlen(hl->base_uri); rh->uri[i]!='\0'; i++, j++) {
            buffer[j] = rh->uri[i];
        }
        if (k >= 0) {
            if (buffer[j-1] != '/') {
                buffer[j++] = '/';
            }
            index = ews_get_detail_value(hl->details, "index", k);
            for (i=0; index[i]!='\0'; i++, j++)
                buffer[j] = index[i];
        }
        buffer[j] = '\0';
        ews_verbose(LOG_LEVEL_INFO, "ruta %s", buffer);
        f = stat(buffer, &st);
        if (f != -1 && S_ISREG(st.st_mode)) {
            return 1;
        }
    }
    return 0;
}

void http10_init( moduleTAD *module ) {
    strcpy(module->name, "http10");
    module->type = MODULE_TYPE_PROC;
    module->priority = 50;

    module->load = NULL;
    module->unload = NULL;
    module->reload = NULL;
    module->get_status = http10_get_status;
    module->run = http10_run;
}
