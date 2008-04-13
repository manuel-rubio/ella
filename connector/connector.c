/* -*- mode:C; coding:utf-8 -*- */

#include "../include/connector/connector.h"

pthread_t bindThreads[CONNECTOR_MAX_THREADS];
int bindThreadCounter = 0;
char bindThreadExit = 0;

// TODO: eliminar esta función. Es temporal.
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

void* tor_connector_launch( void* ptr_bc ) {
    bindConnect *bc;
    bindRequest *br;
    struct sockaddr_in server, client;
    int fd_server, fd_client;
    int rc;
    int i, count = 0;
    char request[8192], buffer[2048];

    bc = (bindConnect *)ptr_bc;
    printf("Lanzando conexión: %s:%d\n", bc->host, bc->port);
    bzero(&server, sizeof(server));
    fd_server = tor_server_start(&server, bc->host, bc->port);

    // TODO: cargar módulos

    while (!bindThreadExit) {
        bzero(&client, sizeof(client));
        fd_client = tor_server_accept(&server, &client, fd_server);

        bzero(request, sizeof(request));
        bzero(buffer, sizeof(buffer));
        while (recv(fd_client, buffer, sizeof(buffer), 0) != -1) {
            for (i=0; buffer[i]!='\0' && count<2; i++) {
                if (buffer[i] == '\n') {
                    count++;
                } else if (buffer[i] == '\r') {
                    continue;
                } else {
                    count = 0;
                }
            }
            tor_concat(request, buffer);
            bzero(buffer, sizeof(buffer));
            if (count == 2)
                break;
        }
        br = (bindRequest *)malloc(sizeof(bindRequest));
        br->request = tor_parse_request(request);
        br->fd_client = fd_client;
        br->bc = bc;
        bcopy(&client, &(br->client), sizeof(client));
        rc = pthread_create(&br->thread, NULL, tor_connector_client_launch, (void *)br);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
        }
    }
    pthread_exit(NULL);
    return NULL;
}

void* tor_connector_client_launch( void* ptr_br ) {
    bindRequest *br;
    char buffer[8192] = { 0 };

    br = (bindRequest *)ptr_br;
    printf("Conexión desde: %s\n", inet_ntoa((br->client).sin_addr));

    // TODO: proceso de paso de módulos, uno a uno, para procesamiento de petición.
    prepare_page(buffer);
    send(br->fd_client, buffer, sizeof(buffer), 0);

    // cerramos todo antes de salir
    shutdown(br->fd_client, SHUT_RD);
    close(br->fd_client);
    free(ptr_br);
    return NULL;
}

int tor_server_start( struct sockaddr_in *server, char *host, int port ) {
    int fd;

    if ((fd=socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {
        printf("error en socket()\n");
        exit(-1);
    }

    server->sin_family = AF_INET;
    server->sin_port = htons(port);
    server->sin_addr.s_addr = inet_addr(host);
    bzero(&(server->sin_zero),8);

    if (bind(fd,(struct sockaddr*)server, sizeof(struct sockaddr))==-1) {
        printf("error en bind() puerto %d\n", port);
        exit(-1);
    }

    if (listen(fd,MAX_CONNS) == -1) {
        printf("error en listen()\n");
        exit(-1);
    }
    return fd;
}

int tor_server_accept( struct sockaddr_in* server, struct sockaddr_in* client, int sfd ) {
    int sin_size, fd;

    sin_size = sizeof(struct sockaddr_in);
    if ((fd = accept(sfd, (struct sockaddr *)&client, &sin_size))==-1) {
        printf("error en accept()\n");
        exit(-1);
    }
    return fd;
}

bindConnect* tor_connector_parse_bind( configBlock *cb ) {
    bindConnect *bc = NULL,
                *pbc = NULL,
                *pibc = NULL;
    configBlock *pcb = NULL,
                *aliases = NULL;
    virtualHost *vh = NULL;
    char        host[80];
    int         port;

    for (pcb = cb; pcb != NULL; pcb = pcb->next) {
        if (tor_compare("modules", pcb->name) == 0) {
            // este bloque no se procesará aquí
            continue;
        } else if (tor_compare("aliases", pcb->name) == 0) {
            aliases = pcb;
        } else {
            if (bc == NULL) {
                bc = (bindConnect *)malloc(sizeof(bindConnect));
                pbc = bc;
                tor_get_bindhost(pcb, "bind", 0, pbc->host);
                pbc->port = tor_get_bindport(pcb, "bind", 0);
                pbc->vhosts = NULL;
                pbc->next = NULL;
            } else {
                tor_get_bindhost(pcb, "bind", 0, host);
                port = tor_get_bindport(pcb, "bind", 0);
                for (pibc = bc; pibc != NULL; pbc = pibc, pibc = pibc->next) {
                    if (tor_compare(host, pibc->host) == 0 && port == pibc->port)
                        break;
                }
                if (pibc == NULL) {
                    pbc->next = (bindConnect *)malloc(sizeof(bindConnect));
                    pbc = bc->next;
                    pbc->vhosts = NULL;
                    pbc->next = NULL;
                }
            }
            if (pbc->vhosts != NULL) {
                vh = tor_connector_find_vhost(bc->vhosts, pcb->name);
                tor_connector_parse_vhost(pcb, aliases, &vh);
            } else {
                vh = NULL;
                tor_connector_parse_vhost(pcb, aliases, &pbc->vhosts);
            }
        }
    }
    return bc;
}

virtualHost* tor_connector_find_vhost( virtualHost *vh, char *name ) {
    virtualHost *pvh;

    for (pvh = vh; pvh != NULL; pvh = pvh->next) {
        if (tor_compare(pvh->host_name, name) == 0) {
            return pvh;
        }
    }
    return NULL;
}

void tor_connector_parse_vhost( configBlock *cb, configBlock *aliases, virtualHost **pvh ) {
    hostAlias     *pha;
    int           indexes, i;
    virtualHost   *vh = *pvh;

    if (*pvh == NULL) {
        *pvh = (virtualHost *)malloc(sizeof(virtualHost));
        vh = *pvh;
        tor_copy(cb->name, vh->host_name);
        vh->aliases = NULL;
        vh->next = NULL;

        // configuramos los alias para el virtual host
        if (aliases != NULL) {
            indexes = tor_get_detail_values(aliases, vh->host_name);
            for (i=0; i<indexes; i++) {
                if (vh->aliases == NULL) {
                    vh->aliases = (hostAlias *)malloc(sizeof(hostAlias));
                    pha = vh->aliases;
                } else {
                    pha->next = (hostAlias *)malloc(sizeof(hostAlias));
                    pha = pha->next;
                }
                tor_copy(tor_get_detail_key(aliases, vh->host_name, i), pha->alias);
            }
        }
    }
    tor_connector_parse_location(cb, vh);
}

void tor_connector_parse_location( configBlock* cb, virtualHost* vh ) {
    hostLocation *phl = NULL;
    configDetail *pcd = NULL,
                 *phl_cd = NULL;

    if (vh->locations == NULL) {
        vh->locations = (hostLocation *)malloc(sizeof(hostLocation));
        phl = vh->locations;
    } else {
        phl = vh->locations;
        phl->next = (hostLocation *)malloc(sizeof(hostLocation));
        phl = phl->next;
    }

    tor_copy(cb->lastname, phl->base_uri);
    for (pcd = cb->details; pcd != NULL; pcd = pcd->next) {
        if (tor_compare(pcd->key, "bind") != 0) {
            if (phl_cd == NULL) {
                phl->details = tor_new_detail(pcd->key, pcd->value, pcd->index);
                phl_cd = phl->details;
            } else {
                phl_cd->next = tor_new_detail(pcd->key, pcd->value, pcd->index);
                phl_cd = phl_cd->next;
            }
        }
    }
}

void tor_connector_bind_free( bindConnect* bc ) {
    if (bc == NULL)
        return;
    if (bc->next != NULL)
        tor_connector_bind_free(bc->next);
    if (bc->vhosts != NULL)
        tor_connector_vhost_free(bc->vhosts);
    free(bc);
}

void tor_connector_vhost_free( virtualHost* vh ) {
    if (vh == NULL)
        return;
    if (vh->next != NULL)
        tor_connector_vhost_free(vh->next);
    if (vh->locations != NULL)
        tor_connector_location_free(vh->locations);
    if (vh->aliases != NULL)
        tor_connector_alias_free(vh->aliases);
    free(vh);
}

void tor_connector_location_free( hostLocation* hl ) {
    if (hl == NULL)
        return;
    if (hl->next != NULL)
        tor_connector_location_free(hl->next);
    if (hl->details != NULL)
        tor_free_details(hl->details);
    free(hl);
}

void tor_connector_alias_free( hostAlias* ha ) {
    if (ha == NULL)
        return;
    if (ha->next != NULL)
        tor_connector_alias_free(ha->next);
    free(ha);
}
