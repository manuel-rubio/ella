#include <sys/stat.h>

#include "../include/ella.h"

#define GATEWAY_VER "CGI/1.1"

void cgi_get_status( char *s ) {
    sprintf(s, GATEWAY_VER " - RFC 3875");
}

int cgi_run( struct Bind_Request *br, responseHTTP *rs ) {
    requestHTTP *rh = br->request;
    virtualHost *vh = NULL;
    hostLocation *hl = NULL;
    char *host = ews_get_header_value(rh, "Host", 0);
    char host_name[80];
    char remote_port[10];
    char *port = NULL; // port 80 by default
    char *path = NULL;
    char *cgi = NULL;
    char buffer[BUFFER_SIZE] = { 0 };
    char tmp_exec[BUFFER_SIZE] = { 0 };
    char uri[BUFFER_SIZE] = { 0 };
    char http_version[10] = { 0 };
    char *content = NULL, *aux = NULL, *get = NULL;
    char *admin = NULL, *http_accept = NULL, *content_length = NULL;
    FILE *cmd = NULL;
    int size = 0, len = 0, ptr = 0, f, i;
    struct stat st;

    if (host != NULL) {
        strcpy(host_name, host);
        for (i=0; host_name[i]!='\0'; i++) {
            if (host_name[i] == ':') {
                host_name[i] = '\0';
                port = host_name + i + 1;
                break;
            }
        }
    }
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
    admin = ews_get_detail_value(hl->details, "admin", 0);
    http_accept = ews_get_header_value(rh, "Accept", EWS_HEADER_GET_ALL);
    strcpy(http_version, "HTTP/");
    strcat(http_version, rh->version);
    content_length = ews_get_header_value(rh, "Content-Length", 0);

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
            sprintf(remote_port, "%d", (br->client).sin_port);
            // TODO: set environment vars
            setenv("TERM", "dumb", 1);
            setenv("SCRIPT_NAME", uri, 1);
            setenv("SCRIPT_FILENAME", buffer, 1);
            setenv("DOCUMENT_ROOT", path, 1);
            setenv("GATEWAY_INTEFACE", GATEWAY_VER, 1);
            setenv("HTTP_ACCEPT", http_accept, 1);
            setenv("HTTP_HOST", host, 1);
            setenv("HTTP_USER_AGENT", ews_get_header_value(rh, "User-Agent", 0), 1);
            setenv("REMOTE_ADDR", inet_ntoa((br->client).sin_addr), 1);
            setenv("REMOTE_PORT", remote_port, 1);
            setenv("REQUEST_METHOD", rh->request, 1);
            setenv("REQUEST_URI", rh->uri, 1);
            // FIXME: server 0.0.0.0 isn't correct value, this should be 127.0.0.1, 192.168.73.24...
            setenv("SERVER_ADDR", br->bc->host, 1);
            setenv("SERVER_ADMIN", (admin == NULL) ? "webmaster@localhost" : admin, 1);
            setenv("SERVER_NAME", host_name, 1);
            setenv("SERVER_PORT", port, 1);
            setenv("SERVER_PROTOCOL", http_version, 1);
            setenv("SERVER_SIGNATURE", PACKAGE_NAME "/" PACKAGE_VERSION, 1);
            setenv("SERVER_SOFTWARE", PACKAGE_NAME "/" PACKAGE_VERSION, 1);
            setenv("QUERY_STRING", (get == NULL) ? "" : get, 1);
            if (content_length != NULL) {
                setenv("CONTENT_LENGTH", content_length, 1);
            }

            if (http_accept != NULL) {
                ews_free(http_accept, "cgi_run");
            }
            if (rh->content != NULL) {
                // FIXME: doesn't work, try to another thing to send POST data to CGI
                sprintf(tmp_exec, "echo '%s ' | %s", rh->content, buffer);
                ews_verbose(LOG_LEVEL_DEBUG, "send to exec: %s (%s)", tmp_exec, content_length);
            } else {
                strcpy(tmp_exec, buffer);
            }
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
                if (ptr == 0) {
                    ews_free(content, "cgi_run");
                    rs->code = 500;
                } else {
                    ews_free(rs->content, "cgi_run");
                    rs->content_type = HEADER_CONTENT_RAW;
                    rs->content = content;
                    rs->code = 200;
                    strcpy(rs->message, "OK");
                    strcpy(rs->version, "1.0");
                    ews_add_header(&rs->headers, "Server", "ews/0.1", 0);
                }
            } else {
                rs->code = 500;
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
