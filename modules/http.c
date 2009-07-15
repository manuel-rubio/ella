#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>

#include "../include/ella.h"

#define METHOD_GET     1
#define METHOD_POST    2
#define METHOD_HEAD    3

enum {
    EWS_HTTP_PAGE_403,
    EWS_HTTP_PAGE_404,
    EWS_HTTP_PAGE_500,
    EWS_HTTP_PAGE_501,
    EWS_HTTP_PAGES
};

static char *page_names[EWS_HTTP_PAGES] = {
    "error403",
    "error404",
    "error500",
    "error501"
};
static char pages[EWS_HTTP_PAGES][PAGE_SIZE];


static pthread_mutex_t
    http_pages_mutex = PTHREAD_MUTEX_INITIALIZER;

static char autoindex_page[PAGE_SIZE] = { 0 };
static char
    *autoindex_header,
    *autoindex_entry,
    *autoindex_footer;

static int
    http_pages_200 = 0,
    http_pages_304 = 0,
    http_pages_403 = 0,
    http_pages_404 = 0,
    http_pages_500 = 0,
    http_pages_501 = 0;

void http_get_status( char *s );
int http_page_set_var( char *page, char *var[][100], int size );
int http_autoindex_prepare( char *autoindex );
void http_error_page( int code, char *message, char *page, requestHTTP *rh, responseHTTP *rs, int method );
int http_run( struct Bind_Request *br, responseHTTP *rs );
int http_autoindex( char *page, requestHTTP *rh, hostLocation *hl );
int http_find_file( char *buffer, requestHTTP *rh, hostLocation *hl );
int http_cli_info( int pipe, char *params );
void http_init( moduleTAD *module, cliCommand **cc );

int http_send_error_page( int code, requestHTTP *rh, responseHTTP *rs, int method ) {
    char buffer[BUFFER_SIZE] = { 0 };
    switch (code) {
        case 404:
            ews_verbose(LOG_LEVEL_ERROR, "(404) file %s not found.", buffer);
            http_error_page(404, "Not found", pages[EWS_HTTP_PAGE_404], rh, rs, method);
            break;
        case 403:
            ews_verbose(LOG_LEVEL_ERROR, "(403) can't acces to %s.", buffer);
            http_error_page(403, "Forbidden", pages[EWS_HTTP_PAGE_403], rh, rs, method);
            break;
        case 500:
            ews_verbose(LOG_LEVEL_ERROR, "(500) internal server error!");
            http_error_page(500, "Internal Server Error", pages[EWS_HTTP_PAGE_500], rh, rs, method);
            break;
        default:
            return -1;
    }
    return 0;
}

void http_add_page_count( int code ) {
    pthread_mutex_lock(&http_pages_mutex);
    switch (code) {
        case 200: http_pages_200++; break;
        case 304: http_pages_304++; break;
        case 403: http_pages_403++; break;
        case 404: http_pages_404++; break;
        case 500: http_pages_500++; break;
        case 501: http_pages_501++; break;
        default:
            ews_verbose(LOG_LEVEL_ERROR, "    unknown code! %d", code);
    }
    pthread_mutex_unlock(&http_pages_mutex);
}

void http_get_status( char *s ) {
    pthread_mutex_lock(&http_pages_mutex);
    sprintf(s, "HTTP 1.0 - RFC1945\n\n\
Pages sent: %6d\n\
       200: %6d\n\
       304: %6d\n\
       403: %6d\n\
       404: %6d\n\
       500: %6d\n\
       501: %6d",
    http_pages_200 + http_pages_304 + http_pages_403 + http_pages_404 + + http_pages_500 + http_pages_501,
    http_pages_200,
    http_pages_304,
    http_pages_403,
    http_pages_404,
    http_pages_500,
    http_pages_501);
    pthread_mutex_unlock(&http_pages_mutex);
}

int http_page_set_var( char *page, char *var[][100], int size ) {
    char *temp = NULL;
    register int i = 0, j = 0, k = 0, x = 0, y = 0;
    char varname[80] = { 0 };
    int begin = 0;

    temp = (char *)ews_malloc(PAGE_SIZE);
    for (i=0; i<PAGE_SIZE && page[i]!='\0'; i++)
        temp[i] = page[i];
    for (i=0, k=0; temp[i]!='\0'; i++, k++) {
        if (temp[i] == '<' && temp[i+1] == '!') {
            begin = i;
            i += 2;
            for (j=0; ((temp[i]>='A' && temp[i]<='Z') || temp[i] == '_') && (j - 1) < 80; j++, i++) {
                varname[j] = temp[i];
            }
            varname[j] = '\0';
            if (temp[i] != '>') {
                i = begin;
                page[k] = temp[i];
                continue;
            }
            // search varname
            for (x=0; x<size; x++) {
                for (y=0; var[x][0][y]==varname[y] && var[x][0][y]!='\0' && varname[y]!='\0'; y++)
                    ;
                if (!varname[y] && !var[x][0][y]) {
                    for (y=0; var[x][1][y]!='\0'; y++)
                        page[k+y] = var[x][1][y];
                    k += (y - 1);
                    break;
                }
            }
        } else {
            page[k] = temp[i];
        }
    }
    page[k]='\0';
    ews_free(temp, "http_page_set_var");
    return 0;
}

int http_autoindex_prepare( char *autoindex ) {
    FILE *f;

    if (autoindex != NULL) {
        if ((f = fopen(autoindex, "r"))) {
            fread(autoindex_page, PAGE_SIZE, 1, f);
            fclose(f);
        } else {
            ews_verbose(LOG_LEVEL_WARN, "    page %s can't open to load", autoindex);
        }
    }

    // setting header
    autoindex_header = autoindex_page;
    // searching for entry
    autoindex_entry = strstr(autoindex_page, "<!>");
    if (autoindex_entry == NULL) {
        ews_verbose(LOG_LEVEL_ERROR, "    autoindex page is incorrect at entry");
        return 0;
    }
    autoindex_entry[0] = '\0';
    autoindex_entry += 3;
    // searching for footer
    autoindex_footer = strstr(autoindex_entry, "<!>");
    if (autoindex_footer == NULL) {
        ews_verbose(LOG_LEVEL_ERROR, "    autoindex page is incorrect at footer");
        return 0;
    }
    autoindex_footer[0] = '\0';
    autoindex_footer += 3;
    return 1;
}

void http_error_page( int code, char *message, char *page, requestHTTP *rh, responseHTTP *rs, int method ) {
    char *buffer;
    char *var[][100] = { { "URI", rh->uri }, { "METHOD", rh->request }, { "SERVER", PACKAGE_NAME "/" PACKAGE_VERSION } };

    buffer = (char *)ews_malloc(PAGE_SIZE);
    bzero(buffer, PAGE_SIZE);

    ews_verbose(LOG_LEVEL_DEBUG, "    sending error page for code %d: %s", code, message);
    rs->code = code;
    strcpy(rs->message, message);
    strcpy(rs->version, "1.0");
    ews_add_header(&rs->headers, "Server", PACKAGE_NAME "/" PACKAGE_VERSION, 0);
    strcpy(buffer, page);
    http_page_set_var(buffer, var, 3);
    if (method != METHOD_HEAD) {
        ews_set_response_content(rs, HEADER_CONTENT_STRING, buffer);
    } else {
        ews_set_response_content(rs, HEADER_CONTENT_NONE, NULL);
    }
    http_add_page_count(rs->code);
    ews_free(buffer, "http_error_page");
}

int http_run( struct Bind_Request *br, responseHTTP *rs ) {
    requestHTTP *rh = br->request;
    virtualHost *vh = NULL;
    hostLocation *hl = NULL;
    char *host_name = ews_get_header_value(rh, "Host", 0);
    char *modified = ews_get_header_value(rh, "If-Modified-Since", 0);
    char buffer[BUFFER_SIZE] = { 0 }, date[80] = { 0 };
    int method = 0;
    char *page = NULL;
    int code;

    if (strcmp(br->request->request, "GET") == 0) {
        method = METHOD_GET;
    } else if (strcmp(br->request->request, "POST") == 0) {
        method = METHOD_POST;
    } else if (strcmp(br->request->request, "HEAD") == 0) {
        method = METHOD_HEAD;
    } else {
        ews_verbose(LOG_LEVEL_ERROR, "    method %s not implemented.", br->request->request);
        http_error_page(501, "Not implemented", pages[EWS_HTTP_PAGE_501], rh, rs, METHOD_GET);
        return MODULE_RETURN_OK;
    }

    if (rs->code >= 400 && rs->code < 700) {
        /* some module send us a error code to show'em */
        if (http_send_error_page(rs->code, rh, rs, method) == 0) {
            return MODULE_RETURN_OK;
        }
        ews_verbose(LOG_LEVEL_INFO, "    unknown code %d in previous code page, ignoring...", rs->code);
    } else if (rs->code >= 200 && rs->code <= 299) {
        return MODULE_RETURN_OK;
    }

    if (host_name != NULL) {
        vh = ews_connector_find_vhost((virtualHost *)br->bc->vhosts, host_name);
    }
    if (host_name == NULL || vh == NULL) {
        // gets default, the first vhost
        vh = (virtualHost *)br->bc->vhosts;
    }
    hl = ews_connector_find_location(vh->locations, rh->uri);
    if (hl == NULL) { // 404 - Not found
        ews_verbose(LOG_LEVEL_ERROR, "    location mismatch with configurations.");
        http_error_page(404, "Not found", pages[EWS_HTTP_PAGE_404], rh, rs, method);
        return MODULE_RETURN_OK;
    }
    if (!http_find_file(buffer, rh, hl)) { // 404 - Not found
        // Try autoindex
        page = (char *)ews_malloc(PAGE_SIZE);
        bzero(page, PAGE_SIZE);
        code = http_autoindex(page, rh, hl);
        if (http_send_error_page(code, rh, rs, method) == 0) {
            ews_free(page, "http_run");
            return MODULE_RETURN_OK;
        }
        ews_verbose(LOG_LEVEL_INFO, "    unknown code %d in autoindex generation, ignoring...", code);
        modified = NULL;
    } else {
        ews_verbose(LOG_LEVEL_INFO, "    sending file [%s]", buffer);
    }

    strcpy(rs->version, "1.0");
    ews_add_header(&rs->headers, "Server", PACKAGE_NAME "/" PACKAGE_VERSION, 0);
    set_current_date(date);
    ews_add_header(&rs->headers, "Date", date, 0);
    set_file_date(date, buffer);
    if (modified != NULL && compare_date(date, modified) >= 0) {
        rs->code = 304;
        strcpy(rs->message, "Not Modified");
    } else {
        rs->code = 200;
        strcpy(rs->message, "OK");
        ews_add_header(&rs->headers, "Last-Modified", date, 0);
    }
    if (page != NULL && rs->code == 200) {
        ews_set_response_content(rs, HEADER_CONTENT_STRING, page);
    } else if (method != METHOD_HEAD && rs->code == 200) {
        ews_set_response_content(rs, HEADER_CONTENT_FILE, buffer);
    }
    http_add_page_count(rs->code);

    if (page != NULL) {
        ews_free(page, "http_run");
    }
    return MODULE_RETURN_OK;
}

int http_autoindex( char *page, requestHTTP *rh, hostLocation *hl ) {
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
    char parent_dir[128] = { 0 };
    char *var_header[][100] = {
        { "URI", NULL },
        { "PARENT_DIR", NULL }
    };
    char *var_entry[][100] = {
        { "ICON", NULL },
        { "LINK", NULL },
        { "FILE", NULL },
        { "DATE", NULL },
        { "SIZE", NULL }
    };
    char *var_footer[][100] = {
        { "SERVER", PACKAGE_NAME "/" PACKAGE_VERSION }
    };
    int root = !strcmp(rh->uri, "/");

    bzero(page, PAGE_SIZE);

    if (autoindex != NULL && strcmp(autoindex, "on") == 0) {
        if (autoindex_header == NULL || autoindex_entry == NULL || autoindex_footer == NULL) {
            ews_verbose(LOG_LEVEL_ERROR, "    AutoIndex pages not found or we can't load'em!");
            return 500;
        }
        sprintf(dir, "%s/%s", path, rh->uri + (strlen(hl->base_uri)));
        if (strncmp(path, dir, strlen(path)) != 0) { // 403 Forbidden
            ews_verbose(LOG_LEVEL_WARN, "    Access to '%s' denied!", path);
            return 403;
        }
        ews_verbose(LOG_LEVEL_INFO, "    Directory for autoindex: %s", dir);
        d = opendir(dir);
        if (d == NULL)
            return 404;
        sprintf(parent_dir, "%s/..", rh->uri);
        strcpy(page, autoindex_header);
        var_header[0][1] = rh->uri;
        var_header[1][1] = parent_dir;
        http_page_set_var(page, var_header, 2);
        for (dp = readdir(d); dp != NULL; dp = readdir(d)) {
            if (strcmp(dp->d_name, ".") == 0) {
                // Nothing to do (self dir)
            } else if (strcmp(dp->d_name, "..") == 0) {
                // Nothing to do (parent dir)
            } else {
                sprintf(file, "%s/%s", dir, dp->d_name);
                if (!root) {
                    sprintf(file_uri, "%s/%s", rh->uri, dp->d_name);
                } else {
                    sprintf(file_uri, "/%s", dp->d_name);
                }
                stat(file, &st);
                ft = gmtime(&st.st_mtime);
                sprintf(date, "%02d/%02d/%02d %02d:%02d:%02d",
                    ft->tm_mday, ft->tm_mon + 1, ft->tm_year + 1900,
                    ft->tm_hour, ft->tm_min, ft->tm_sec
                );
                if (S_ISREG(st.st_mode)) {
                    sprintf(size, "%d", (int)st.st_size);
                } else {
                    strcpy(size, "-");
                }
                strcpy(linea, autoindex_entry);
                var_entry[0][1] = (S_ISDIR(st.st_mode))?"[DIR]":(S_ISLNK(st.st_mode))?"[LNK]":"";
                var_entry[1][1] = file_uri;
                var_entry[2][1] = dp->d_name;
                var_entry[3][1] = date;
                var_entry[4][1] = size;
                http_page_set_var(linea, var_entry, 5);
                strcat(page, linea);
            }
        }
        strcat(page, autoindex_footer);
        http_page_set_var(page, var_footer, 1);
        closedir(d);
        return 200;
    }
    return 403;
}

int http_find_file( char *buffer, requestHTTP *rh, hostLocation *hl ) {
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
        ews_verbose(LOG_LEVEL_INFO, "    ruta %s", buffer);
        f = stat(buffer, &st);
        if (f != -1 && S_ISREG(st.st_mode)) {
            return 1;
        }
    }
    return 0;
}

int http_cli_info( int pipe, char *params ) {
    if (params != NULL) {
        if (strncmp(params, "reset", strlen(params)) == 0) {
            pthread_mutex_lock(&http_pages_mutex);
            http_pages_200 = 0;
            http_pages_304 = 0;
            http_pages_403 = 0;
            http_pages_404 = 0;
            http_pages_501 = 0;
            pthread_mutex_unlock(&http_pages_mutex);
            ews_verbose_to(pipe, LOG_LEVEL_INFO, "reset complete.");
        }
    } else {
        pthread_mutex_lock(&http_pages_mutex);
        ews_verbose_to(pipe, LOG_LEVEL_INFO, "HTTP 1.0 - RFC1945");
        ews_verbose_to(pipe, LOG_LEVEL_INFO, "Pages sent: %6d",
            http_pages_200 + http_pages_304 + http_pages_403 +
            http_pages_404 + http_pages_501);
        ews_verbose_to(pipe, LOG_LEVEL_INFO, "       200: %6d", http_pages_200);
        ews_verbose_to(pipe, LOG_LEVEL_INFO, "       304: %6d", http_pages_304);
        ews_verbose_to(pipe, LOG_LEVEL_INFO, "       403: %6d", http_pages_403);
        ews_verbose_to(pipe, LOG_LEVEL_INFO, "       404: %6d", http_pages_404);
        ews_verbose_to(pipe, LOG_LEVEL_INFO, "       501: %6d", http_pages_501);
        pthread_mutex_unlock(&http_pages_mutex);
    }
    return 1;
}

void http_load( configDetail* details ) {
    int i;
    char *filename;
    FILE *f;

    if (details != NULL) {
        http_autoindex_prepare(ews_get_detail_value(details, "autoindex_page", 0));
        for (i=0; i<EWS_HTTP_PAGES; i++) {
            filename = ews_get_detail_value(details, page_names[i], 0);
            if (filename != NULL) {
                if ((f = fopen(filename, "r"))) {
                    fread(pages[i], PAGE_SIZE, 1, f);
                    fclose(f);
                } else {
                    ews_verbose(LOG_LEVEL_WARN, "    page [%s] can't open to load", filename);
                }
            } else {
                ews_verbose(LOG_LEVEL_WARN, "    page [%s] can't found in config file", page_names[i]);
            }
        }
    }
}

void http_reload( configBlock *cb ) {
    configBlock *module = ews_get_block(cb, "http", NULL);

    if (module != NULL) {
        http_load(module->details);
    }
}

void http_init( moduleTAD *module, cliCommand **cc ) {

    strcpy(module->name, "http");
    module->type = MODULE_TYPE_PROC;
    module->priority = 50;

    module->load = NULL;
    module->unload = NULL;
    module->reload = http_reload;
    module->get_status = http_get_status;
    module->run = http_run;

    ews_cli_add_command(cc, "http-info", "info about HTTP 1.0 module", "\n\
Sintaxis: http-info [reset]\n\
Description: show stats for incoming requests and outgoing type responses.\n", http_cli_info);

    if (module != NULL) {
        http_load(module->details);
    }
}
