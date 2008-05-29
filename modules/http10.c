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

enum {
    EWS_HTTP_PAGE_403,
    EWS_HTTP_PAGE_404,
    EWS_HTTP_PAGE_501,
    EWS_HTTP_PAGES
};

static char *page_names[EWS_HTTP_PAGES] = {
    "error403",
    "error404",
    "error501"
};
static char pages[EWS_HTTP_PAGES][PAGE_SIZE];


static char autoindex_page[PAGE_SIZE];
static char
    *autoindex_header,
    *autoindex_entry,
    *autoindex_footer;


int http10_page_set_var( char *page, char *var, char *val ) {
    char temp[PAGE_SIZE] = { 0 };
    char varname[80];
    char *v;

    sprintf(varname, "<!%s>", var);
    v = strstr(page, varname);
    if (v == NULL) {
        ews_verbose(LOG_LEVEL_WARN, "var %s doesn't exists in page", varname);
        return 0;
    }
    do {
        v[0] = '\0';
        v += strlen(varname);
        strcpy(temp, v);
        strcat(page, val);
        strcat(page, temp);
    } while ((v = strstr(page, varname)) != NULL);
    return 1;
}

int http10_autoindex_prepare( char *autoindex ) {
    FILE *f;

    if (autoindex != NULL) {
        if (f = fopen(autoindex, "r")) {
            fread(autoindex_page, PAGE_SIZE, 1, f);
            fclose(f);
        } else {
            ews_verbose(LOG_LEVEL_WARN, "page %s can't open to load", autoindex);
        }
    }

    // setting header
    autoindex_header = autoindex_page;
    // searching for entry
    autoindex_entry = strstr(autoindex_page, "<!>");
    if (autoindex_entry == NULL) {
        ews_verbose(LOG_LEVEL_ERROR, "autoindex page is incorrect at entry");
        return 0;
    }
    autoindex_entry[0] = '\0';
    autoindex_entry += 3;
    // searching for footer
    autoindex_footer = strstr(autoindex_entry, "<!>");
    if (autoindex_footer == NULL) {
        ews_verbose(LOG_LEVEL_ERROR, "autoindex page is incorrect at footer");
        return 0;
    }
    autoindex_footer[0] = '\0';
    autoindex_footer += 3;
    return 1;
}

void http10_get_status( char *s ) {
    strcpy(s, "HTTP 1.0 - RFC1945 - Process module without dynamic information.");
}

// TODO: this messages should be webs in dirs
void http10_error_page( int code, char *message, char *page, requestHTTP *rh, responseHTTP *rs, int method ) {
    char buffer[BUFFER_SIZE];

    bzero(buffer, BUFFER_SIZE);

    rs->code = code;
    strcpy(rs->message, message);
    strcpy(rs->version, "1.0");
    rs->headers = ews_new_header("Server", "ews/0.1", 0);
    strcpy(buffer, page);
    http10_page_set_var(buffer, "URI", rh->uri);
    http10_page_set_var(buffer, "METHOD", rh->request);
    http10_page_set_var(buffer, "SERVER", "ews/0.1");
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
        http10_error_page(501, "Not implemented", pages[EWS_HTTP_PAGE_501], rh, rs, METHOD_GET);
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
        ews_verbose(LOG_LEVEL_ERROR, "location mismatch with configurations.");
        http10_error_page(404, "Not found", pages[EWS_HTTP_PAGE_404], rh, rs, method);
        return MODULE_RETURN_OK;
    }
    if (!http10_find_file(buffer, rh, hl)) { // 404 - Not found
        // Try autoindex
        switch (http10_autoindex(page, rh, hl)) {
            case 404:
                ews_verbose(LOG_LEVEL_ERROR, "file %s not found.", buffer);
                http10_error_page(404, "Not found", pages[EWS_HTTP_PAGE_404], rh, rs, method);
                return MODULE_RETURN_OK;
            case 403:
                ews_verbose(LOG_LEVEL_ERROR, "can't acces to %s.", buffer);
                http10_error_page(403, "Forbidden", pages[EWS_HTTP_PAGE_403], rh, rs, method);
                return MODULE_RETURN_OK;
            default:
                ews_verbose(LOG_LEVEL_INFO, "sending autoindex page");
                modified = NULL; // doesn't use cache in autoindex
        }
    } else {
        ews_verbose(LOG_LEVEL_INFO, "sending file [%s]", buffer);
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

    // TODO: compliant RFC1945
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
    char parent_dir[128] = { 0 };

    bzero(page, PAGE_SIZE);

    if (strcmp(autoindex, "on") == 0) {
        sprintf(dir, "%s/%s", path, rh->uri + (strlen(hl->base_uri)));
        if (strncmp(path, dir, strlen(path)) != 0) { // 403 Forbidden
            return 403;
        }
        ews_verbose(LOG_LEVEL_INFO, "Directory for autoindex: %s", dir);
        d = opendir(dir);
        if (d == NULL)
            return 404;
        sprintf(parent_dir, "%s/..", rh->uri);
        strcpy(page, autoindex_header);
        http10_page_set_var(page, "URI", rh->uri);
        http10_page_set_var(page, "PARENT_DIR", parent_dir);
        for (dp = readdir(d); dp != NULL; dp = readdir(d)) {
            if (strcmp(dp->d_name, ".") == 0) {
                // Nothing to do (self dir)
            } else if (strcmp(dp->d_name, "..") == 0) {
                // Nothing to do (parent dir)
            } else {
                sprintf(file, "%s/%s", dir, dp->d_name);
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
                strcpy(linea, autoindex_entry);
                http10_page_set_var(linea, "ICON", (dp->d_type==DT_DIR)?"[DIR]":(dp->d_type==DT_LNK)?"[LNK]":"");
                http10_page_set_var(linea, "LINK", file_uri);
                http10_page_set_var(linea, "FILE", dp->d_name);
                http10_page_set_var(linea, "DATE", date);
                http10_page_set_var(linea, "SIZE", size);
                strcat(page, linea);
            }
        }
        strcat(page, autoindex_footer);
        http10_page_set_var(page, "SERVER", "ews/0.1");
        closedir(d);
        return 200;
    }
    return 403;
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
    FILE *f;
    char *filename;
    int i;

    strcpy(module->name, "http10");
    module->type = MODULE_TYPE_PROC;
    module->priority = 50;

    module->load = NULL;
    module->unload = NULL;
    module->reload = NULL;
    module->get_status = http10_get_status;
    module->run = http10_run;

    ews_cli_add_command(cc, "http10-info", "info about HTTP 1.0 module", NULL, http10_cli_info);

    http10_autoindex_prepare(ews_get_detail_value(module->details, "autoindex_page", 0));

    for (i=0; i<EWS_HTTP_PAGES; i++) {
        filename = ews_get_detail_value(module->details, page_names[i], 0);
        if (filename != NULL) {
            if (f = fopen(filename, "r")) {
                fread(pages[i], PAGE_SIZE, 1, f);
                fclose(f);
            } else {
                ews_verbose(LOG_LEVEL_WARN, "page %s can't open to load", filename);
            }
        }
    }
}
