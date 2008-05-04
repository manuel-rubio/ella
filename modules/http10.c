#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "../include/modules.h"
#include "../include/header.h"
#include "../include/connector.h"
#include "../include/configurator.h"

#define BUFFER_SIZE 1024

char *page404 = "\
<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n\
<html><head>\n\
<title>404 - Not found</title>\n\
</head><body>\n\
<h1>Not found</h1>\n\
<p>The requested URL %s was not found on this server.</p>\n\
<hr>\n\
<address>Tornasauce/0.1</address>\n\
</body></html>";

void http10_get_status( char *s ) {
    // TODO: especificar en "s" el estado del módulo
    strcpy(s, "OK");
}

void http10_page404( requestHTTP *rh, responseHTTP *rs ) {
    char buffer[BUFFER_SIZE];

    bzero(buffer, BUFFER_SIZE);

    rs->code = 404;
    strcpy(rs->message, "Not found");
    strcpy(rs->version, "1.0");
    rs->headers = tor_new_header("Server", "Tornasauce/0.1", 0);
    sprintf(buffer, page404, rh->uri);
    tor_set_response_content(rs, HEADER_CONTENT_STRING, buffer);
}

int http10_run( struct Bind_Request *br, responseHTTP *rs ) {
    requestHTTP *rh = br->request;
    virtualHost *vh = NULL;
    hostLocation *hl = NULL;
    char *host_name = tor_get_header_value(rh, "Host", 0);
    char *path;
    char buffer[BUFFER_SIZE];
    int i, j, f;

    bzero(buffer, BUFFER_SIZE);

    if (host_name != NULL) {
        vh = tor_connector_find_vhost((virtualHost *)br->bc->vhosts, host_name);
    }
    if (host_name == NULL || vh == NULL) {
        // tomamos default, el primer vhost que haya
        vh = (virtualHost *)br->bc->vhosts;
    }
    hl = tor_connector_find_location(vh->locations, rh->uri);
    if (hl == NULL) { // 404 - Not found
        printf("ERROR: location no casa con ninguna de las configuradas.\n");
        http10_page404(rh, rs);
        return MODULE_RETURN_OK;
    }
    if (!http10_find_file(buffer, rh, hl)) { // 404 - Not found
        printf("ERROR: fichero %s no encontrado.\n", buffer);
        http10_page404(rh, rs);
        return MODULE_RETURN_OK;
    }

    printf("INFO: enviando fichero %s\n", buffer);
    rs->code = 200;
    strcpy(rs->message, "OK");
    strcpy(rs->version, "1.0");
    rs->headers = tor_new_header("Server", "Tornasauce/0.1", 0);
    tor_set_response_content(rs, HEADER_CONTENT_FILE, buffer);

    // TODO: implementar la gestión de cabeceras según el RFC1945
    return MODULE_RETURN_OK;
}

int http10_find_file( char *buffer, requestHTTP *rh, hostLocation *hl ) {
    char *path = tor_get_detail_value(hl->details, "path", 0);
    int indexes = tor_get_detail_indexes(hl->details, "index");
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
            index = tor_get_detail_value(hl->details, "index", k);
            for (i=0; index[i]!='\0'; i++, j++)
                buffer[j] = index[i];
        }
        buffer[j] = '\0';
        printf("INFO: ruta %s\n", buffer);
        f = stat(buffer, &st);
        if (f != -1 && S_ISREG(st.st_mode)) {
            return 1;
        }
    }
    return 0;
}

void http10_init( moduleTAD *module ) {
    strcpy(module->name, "HTTP 1.0 - RFC1945");
    module->type = MODULE_TYPE_PROC;
    module->priority = 50;

    module->load = NULL;
    module->unload = NULL;
    module->reload = NULL;
    module->get_status = http10_get_status;
    module->run = http10_run;
}
