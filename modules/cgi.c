#include <sys/stat.h>

#include "../include/ella.h"

#define GATEWAY_VER "CGI/1.1"

void cgi_get_status( char *s ) {
    sprintf(s, "CGI 1.1 - RFC 3875");
}

int cgi_run( struct Bind_Request *br, responseHTTP *rs ) {
    requestHTTP *rh = br->request;
    virtualHost *vh = NULL;
    hostLocation *hl = NULL;
    char *host_name = ews_get_header_value(rh, "Host", 0);
    char *path = NULL;
    char *cgi = NULL;
    char buffer[BUFFER_SIZE] = { 0 };
    char uri[BUFFER_SIZE] = { 0 };
    char *content = NULL, *aux = NULL, *get = NULL;
    FILE *cmd = NULL;
    int size = 0, len = 0, ptr = 0, f, i;
    struct stat st;

    if (host_name != NULL) {
        vh = ews_connector_find_vhost((virtualHost *)br->bc->vhosts, host_name);
    }
    if (host_name == NULL || vh == NULL) {
        // gets default, the first vhost
        vh = (virtualHost *)br->bc->vhosts;
    }
    hl = ews_connector_find_location(vh->locations, rh->uri);
    cgi = ews_get_detail_value(hl->details, "cgi", 0);
    path = ews_get_detail_value(hl->details, "path", 0);

    if (cgi != NULL && strcmp(cgi, "on") == 0) {
        sprintf(buffer, "%s/%s", path, rh->uri + (strlen(hl->base_uri)) + 1);
        for (i=0; buffer[i]!='\0'; i++) {
            if (buffer[i] == '?') {
                buffer[i] = '\0';
                get = buffer + i + 1;
                break;
            }
        }
        for (i=0; rh->uri[i]!='\0'; i++) {
            if (rh->uri[i] == '?') {
                uri[i] = '\0';
                break;
            }
            uri[i] = rh->uri[i];
        }
        f = stat(buffer, &st);
        if (f == -1) {
            ews_verbose(LOG_LEVEL_ERROR, "file not found (404) [%s]", buffer);
            rs->code = 404;
        } else if (!(S_IXOTH & st.st_mode)) {
            ews_verbose(LOG_LEVEL_ERROR, "execute permission isn't set to this file [%s]", buffer);
            rs->code = 500;
        } else {
            // TODO: set environment vars
            setenv("TERM", "dumb", 1);
            setenv("SCRIPT_NAME", uri, 1);
            setenv("SCRIPT_FILENAME", buffer, 1);
            setenv("DOCUMENT_ROOT", path, 1);
            setenv("GATEWAY_INTEFACE", GATEWAY_VER, 1);
            // FIXME: Accept must be all values concatenated (some value, -1, to get all?)
            setenv("HTTP_ACCEPT", ews_get_header_value(rh, "Accept", 0), 1);
            setenv("HTTP_HOST", host_name, 1);
            setenv("HTTP_USER_AGENT", ews_get_header_value(rh, "User-Agent", 0), 1);
            // setenv("REMOTE_ADDR", xxx, 1);
            // setenv("REMOTE_PORT", xxx, 1);
            // setenv("REQUEST_METHOD", xxx, 1);
            // setenv("REQUEST_URI", rh->uri, 1);
            // setenv("SERVER_ADDR", xxx, 1);
            // setenv("SERVER_ADMIN", xxx, 1);
            // setenv("SERVER_NAME", xxx, 1);
            // setenv("SERVER_PROTOCOL", xxx, 1);
            setenv("SERVER_SIGNATURE", PACKAGE_NAME "/" PACKAGE_VERSION, 1);
            setenv("SERVER_SOFTWARE", PACKAGE_NAME "/" PACKAGE_VERSION, 1);

            cmd = popen(buffer, "r");
            if (cmd) {
                content = (char *)ews_malloc(BUFFER_SIZE);
                size = BUFFER_SIZE;
                while (!feof(cmd)) {
                    len = fread(buffer, 1, BUFFER_SIZE, cmd);
                    if (ptr + len > size) {
                        size += BUFFER_SIZE;
                        aux = (char *)ews_malloc(size);
                        bcopy(content, aux, ptr);
                        ews_free(content, "cgi_run");
                        content = aux;
                    }
                    bcopy(buffer, content + ptr, len);
                    ptr += len;
                }
                content[ptr] = '\0';
                pclose(cmd);
                ews_free(rs->content, "cgi_run");
                rs->content_type = HEADER_CONTENT_RAW;
                rs->content = content;
                rs->code = 200;
                strcpy(rs->message, "OK");
                strcpy(rs->version, "1.0");
                ews_add_header(&rs->headers, "Server", "ews/0.1", 0);
            }
        }
    }

    return MODULE_RETURN_OK;
}

void cgi_init( moduleTAD *module, cliCommand **cc ) {
    strcpy(module->name, "cgi");
    module->type = MODULE_TYPE_PROC;
    module->priority = 25;

    module->load = NULL;
    module->unload = NULL;
    module->reload = NULL;
    module->get_status = cgi_get_status;
    module->run = cgi_run;
}
