#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>

#include "../include/ella.h"

#define BUFFER_SIZE 1024
#define PAGE_SIZE   2097152

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
<address>ews/0.1</address>\n\
</body></html>";

char *page501 = "\
<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n\
<html><head>\n\
<title>501 - Not implemented</title>\n\
</head><body>\n\
<h1>Method not implemented</h1>\n\
<p>The request method isn't implemented.</p>\n\
<hr>\n\
<address>ews/0.1</address>\n\
</body></html>";

char *autoindex_header = "\
<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n\
<html><head>\n\
<title>Index of %s</title>\n\
</head><body>\n\
<h1>Index of %s</h1>\n\
<hr>\n\
<center><table>\n\
<tr><th>Name</th><th>Last modified</th><th>Size</th></tr>\n\
<tr><td colspan=\"3\"><a href=\"%s\">Parent Dir</a></td></tr>\n";

char *autoindex_entry = "<tr><td><a href=\"%s\">%s</a></td><td>%s</td><td align=\"right\">%s</td></tr>\n";

char *autoindex_footer = "\
</table></center>\n\
<hr>\n\
<address>ews/0.1</address>\n\
</body></html>";


void http10_get_status( char *s ) {
    // TODO: especificar en "s" el estado del módulo
    strcpy(s, "HTTP 1.0 - RFC1945 - Process module without dynamic information.");
}

// TODO: estos mensajes deben de ser páginas en directorios
void http10_error_page( int code, char *message, char *page, requestHTTP *rh, responseHTTP *rs, int method ) {
    char buffer[BUFFER_SIZE];

    bzero(buffer, BUFFER_SIZE);

    rs->code = code;
    strcpy(rs->message, message);
    strcpy(rs->version, "1.0");
    rs->headers = ews_new_header("Server", "ews/0.1", 0);
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
    char *modified = ews_get_header_value(rh, "If-Modified-Since", 0);
    char *path;
    char buffer[BUFFER_SIZE] = { 0 }, date[80] = { 0 };
    int i, j, f, method = 0;
    char page[PAGE_SIZE] = { 0 };

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
        // Try autoindex
        if (!http10_autoindex(page, rh, hl)) { // 404 - Not found
            ews_verbose(LOG_LEVEL_ERROR, "fichero %s no encontrado.", buffer);
            http10_error_page(404, "Not found", page404, rh, rs, method);
            return MODULE_RETURN_OK;
        } else {
            ews_verbose(LOG_LEVEL_INFO, "enviando página autoindex");
            modified = NULL; // doesn't use cache in autoindex
        }
    } else {
        ews_verbose(LOG_LEVEL_INFO, "enviando fichero %s", buffer);
    }

    strcpy(rs->version, "1.0");
    rs->headers = ews_new_header("Server", "ews/0.1", 0);
    hh = rs->headers;
    set_current_date(date);
    hh->next = ews_new_header("Date", date, 0);
    set_file_date(date, buffer);
    hh = hh->next;
    if (modified != NULL && compare_date(date, modified) >= 0) {
        rs->code = 304;
        strcpy(rs->message, "Not Modified");
    } else {
        rs->code = 200;
        strcpy(rs->message, "OK");
        hh->next = ews_new_header("Last-Modified", date, 0);
    }
    if (page[0] != 0 && rs->code == 200) {
        ews_set_response_content(rs, HEADER_CONTENT_STRING, page);
    } else if (method != METHOD_HEAD && rs->code == 200) {
        ews_set_response_content(rs, HEADER_CONTENT_FILE, buffer);
    }

    // TODO: implementar la gestión de cabeceras según el RFC1945
    return MODULE_RETURN_PROC_STOP;
}

int http10_autoindex( char *page, requestHTTP *rh, hostLocation *hl ) {
    char *path = ews_get_detail_value(hl->details, "path", 0);
    char *autoindex = ews_get_detail_value(hl->details, "autoindex", 0);
    DIR *d;
    struct dirent *dp;
    struct stat st;
    struct tm *ft;
    char linea[BUFFER_SIZE] = { 0 };
    char file[BUFFER_SIZE] = { 0 };
    char file_uri[BUFFER_SIZE] = { 0 };
    char dir[BUFFER_SIZE] = { 0 };
    char date[50] = { 0 };
    char size[32] = { 0 };

    bzero(page, PAGE_SIZE);

    if (strcmp(autoindex, "on") == 0) {
        sprintf(dir, "%s/%s", path, rh->uri + (strlen(hl->base_uri)));
        ews_verbose(LOG_LEVEL_INFO, "Directorio para autoindex: %s", dir);
        d = opendir(dir);
        if (d == NULL)
            return 0;
        // TODO: configurar el parent dir
        sprintf(page, autoindex_header, rh->uri, rh->uri, "");
        for (dp = readdir(d); dp != NULL; dp = readdir(d)) {
            if (strcmp(dp->d_name, ".") == 0) {
                // Nothing to do (self dir)
            } else if (strcmp(dp->d_name, "..") == 0) {
                // Nothing to do (parent dir)
            } else {
                sprintf(file, "%s/%s", path, dp->d_name);
                sprintf(file_uri, "%s/%s", rh->uri, dp->d_name);
                stat(file, &st);
                ft = gmtime(&st.st_mtime);
                sprintf(date, "%02d/%02d/%02d %02d:%02d:%02d",
                    ft->tm_mday, ft->tm_mon + 1, ft->tm_year + 1900,
                    ft->tm_hour, ft->tm_min, ft->tm_sec
                );
                if (dp->d_type == DT_REG) {
                    sprintf(size, "%d", st.st_size);
                } else {
                    strcpy(size, "-");
                }
                sprintf(linea, autoindex_entry, file_uri, dp->d_name, date, size);
                strcat(page, linea);
            }
        }
        strcat(page, autoindex_footer);
        closedir(d);
        return 1;
    }
    return 0;
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

int http10_cli_info( int pipe, char *params ) {
    char buffer[80];
    http10_get_status(buffer);
    ews_verbose_to(pipe, LOG_LEVEL_INFO, buffer);
    return 1;
}

void http10_init( moduleTAD *module, cliCommand **cc ) {
    strcpy(module->name, "http10");
    module->type = MODULE_TYPE_PROC;
    module->priority = 50;

    module->load = NULL;
    module->unload = NULL;
    module->reload = NULL;
    module->get_status = http10_get_status;
    module->run = http10_run;

    ews_cli_add_command(cc, "http10-info", "info about HTTP 1.0 module", NULL, http10_cli_info);
}
