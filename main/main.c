/* -*- mode:C; coding:utf-8 -*- */

#include "../include/config/config.h"
#include "../include/connector/connector.h"
#include "../include/util/string.h"
#include "../include/util/header.h"
#include "../include/modules/modules.h"

#define MAX_BUFFER   8192

void prepare_page( char *s ) {
    char *pagina = "\
<html>\n\
<head>\n\
    <title>Tornasauce</title>\n\
</head>\n\
<body>\n\
    <h1>Tornasauce</h1>\n\
    <hr />\n\
    <p>Servidor de Bosque Viejo</p>\n\
    <hr />\n\
    <form method=\"post\">\n\
    <input type=\"text\" name=\"hola\" />\n\
    <input type=\"submit\" />\n\
    </form>\n\
</body>\n\
</html>";
    sprintf(s,"\
HTTP/1.0 200 OK\n\
Date: Sun, 16 Mar 2008 19:55:06 GMT\n\
Server: Tornasauce/0.1\n\
Last-Modified: Thu, 03 Jan 2008 11:30:47 GMT\n\
Accept-Ranges: bytes\n\
Content-Length: %d\n\
Content-Type: text/html\n\
\n\
%s", tor_length(pagina), pagina);
}

int main() {
    configFuncs cf = tor_get_initial_conf();
    configBlock *cb, *pcb, *cb_modules;
    moduleTAD *modules;
    requestHTTP *rh;
    struct sockaddr_in server;
    char buffer[MAX_BUFFER] = { 0 };
    char buffer2[MAX_BUFFER] = { 0 };
    char request[MAX_BUFFER] = { 0 };
    int fd_client, fd_server, i, count;
    int port;
    bindConnect *bc, *pbc;

    cb = cf.read();
    bc = tor_connector_parse_bind(cb);
    cb_modules = tor_get_block(cb, "modules", NULL);
    modules = tor_modules_load(cb_modules);

    for (pbc = bc; pbc != NULL; pbc = pbc->next) {
        tor_connector_launch((void *)pbc);
    }
    pthread_exit(NULL);

    tor_free_blocks(cb);
    tor_connector_bind_free(bc);
/*
    pcb = tor_get_block(cb, "tornasauce", NULL);
    port = tor_get_bindport(pcb, "bind", 0);
    prepare_page(buffer);

    fd_server = tor_server_start(&server, port);
    while (1) {
        fd_client = tor_server_accept(&server, fd_server);
        bzero(buffer2, sizeof(buffer2));
        bzero(request, sizeof(request));
        while (recv(fd_client, request, sizeof(request), 0) != -1) {
            printf("%s", request);
            for (i=0, count=0; request[i]!='\0' && count<2; i++) {
                if (request[i]=='\n') {
                    count++;
                } else if (request[i] == '\r') {
                    continue;
                } else {
                    count = 0;
                }
            }
            strcat(buffer2, request);
            if (count == 2)
                break;
        }
        rh = tor_parse_request(buffer2);
        printf("Petición: %s\nURI: %s\nVersion: %s\nContent-Length: %s\n", rh->request, rh->uri, rh->version, tor_get_header_value(rh, "Content-Length", 0));
        tor_free_request(rh);
        rh = NULL;
        send(fd_client, buffer, tor_length(buffer), 0);
        shutdown(fd_client, SHUT_RD);
        close(fd_client);
    }
*/
}
