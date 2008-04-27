/* -*- mode:C; coding:utf-8 -*- */

#include "../include/connector.h"

/* TODO: definimos los clientes máximos a 10, temporalmente */
#define MAX_CLIENTS 10

char bindThreadExit = 0;

void* tor_connector_launch( void* ptr_bc ) {
    bindConnect *bc;
    bindRequest *br;
    struct sockaddr_in server, client;
    int fd_server, fd_client;
    int rc;
    int i, count = 0;
    char request[8192], buffer[2048];

    bc = (bindConnect *)ptr_bc;
    printf("INFO: Lanzando conexión: %s:%d\n", bc->host, bc->port);
    bzero(&server, sizeof(server));
    fd_server = tor_server_start(&server, bc->host, bc->port, MAX_CLIENTS);
    fcntl(fd_server, F_SETFL, fcntl(fd_server, F_GETFL, 0) | O_NONBLOCK);

    while (!bindThreadExit) {
        bzero(&client, sizeof(client));
        do {
            fd_client = tor_server_accept(&server, &client, fd_server);
        } while (fd_client < 0 && (errno == EWOULDBLOCK || errno == EAGAIN) && !bindThreadExit);

        if (bindThreadExit)
            break;

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
            strcat(request, buffer);
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
            printf("ERROR: al crear hilo: %d\n", rc);
        }
    }
    printf("INFO: saliendo del programa\n");
    pthread_exit(NULL);
}

void* tor_connector_client_launch( void* ptr_br ) {
    bindRequest *br;
    responseHTTP rs;
    moduleTAD *pmt;
    char *buffer, bf[1024];
    int res, f, bf_size, bucle = 1;

    br = (bindRequest *)ptr_br;
    printf("INFO: Conexión desde: %s\n", inet_ntoa((br->client).sin_addr));

    rs.code = 0;
    rs.message[0] = '\0';
    rs.version[0] = '\0';
    rs.headers = NULL;
    rs.content = NULL;
    rs.content_type = HEADER_CONTENT_NONE;

    if (br->bc == NULL) {
        printf("FATAL: No hay BindConnect en BindRequest para atender la petición\n");
    } else {
        if (br->bc->modules == NULL) {
            printf("FATAL: No hay ModuleTAD en BindConnect de BindRequest para atender la petición\n");
        } else {
            for (pmt = br->bc->modules; pmt!=NULL; pmt=pmt->next) {
                printf("INFO: ejecutando módulo %s\n", pmt->name);
                if (pmt->run != NULL) {
                    res = pmt->run(br, &rs);
                    if (res == MODULE_RETURN_FAIL) {
                        printf("ERROR: módulo %s tuvo un error de ejecución\n", pmt->name);
                    } else if (res == MODULE_RETURN_STOP) {
                        printf("INFO: módulo %s requiere parada de procesamiento\n", pmt->name);
                        break;
                    }
                } else {
                    printf("FATAL: método 'run' del módulo %s no definido\n", pmt->name);
                }
            }
            buffer = tor_gen_response(&rs);
            send(br->fd_client, buffer, strlen(buffer), 0);
            switch (rs.content_type) {
                case HEADER_CONTENT_STRING:
                    send(br->fd_client, rs.content, strlen(rs.content), 0);
                    break;
                case HEADER_CONTENT_FILE:
                    f = open(rs.content, O_RDONLY);
                    if (f == -1) {
                        printf("FATAL: fichero no se pudo abrir (%d).\n", errno);
                    } else {
                        while (bucle && (bf_size = read(f, bf, 1024))) {
                            if (bf_size == -1) {
                                printf("FATAL: error durante lectura del fichero %s.\n", rs.content);
                                bucle = 0;
                            } else {
                                send(br->fd_client, bf, bf_size, 0);
                            }
                        }
                        if (bucle) {
                            printf("INFO: Fichero %s enviado correctamente\n", rs.content);
                        }
                        close(f);
                    }
            }
            free(buffer);
        }
    }

    // cerramos todo antes de salir
    printf("INFO: finalizado procesamiento desde: %s\n", inet_ntoa((br->client).sin_addr));
    shutdown(br->fd_client, SHUT_RD);
    close(br->fd_client);
    free(ptr_br);
    pthread_exit(NULL);
}

int tor_server_start( struct sockaddr_in *server, char *host, int port, int max_clients ) {
    int fd;

    if ((fd=socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {
        printf("ERROR: al iniciar el servidor (socket)\n");
        exit(-1);
    }

    server->sin_family = AF_INET;
    server->sin_port = htons(port);
    server->sin_addr.s_addr = inet_addr(host);
    bzero(&(server->sin_zero),8);

    if (bind(fd,(struct sockaddr*)server, sizeof(struct sockaddr))==-1) {
        printf("ERROR: el puerto %d está ocupado\n", port);
        exit(-1);
    }

    if (listen(fd, max_clients) == -1) {
        printf("ERROR: no es posible hacer 'listen'\n");
        exit(-1);
    }
    return fd;
}

int tor_server_accept( struct sockaddr_in* server, struct sockaddr_in* client, int sfd ) {
    int sin_size, fd;
    struct timeval t;
    fd_set rfds;

    // TODO: estudiar si es mejor select(), poll() o epoll()
    t.tv_sec = 0;
    t.tv_usec = 100;
    FD_ZERO(&rfds);
    FD_SET(sfd, &rfds);
    select(1, &rfds, NULL, NULL, &t);

    sin_size = sizeof(struct sockaddr_in);
    if ((fd = accept(sfd, (struct sockaddr *)client, &sin_size))==-1) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            return fd;
        }
        printf("ERROR: imposible aceptar conexión entrante\n");
        exit(-1);
    }
    return fd;
}

bindConnect* tor_connector_parse_bind( configBlock *cb, moduleTAD* modules ) {
    bindConnect *bc = NULL,
                *pbc = NULL,
                *pibc = NULL;
    configBlock *pcb = NULL,
                *aliases = NULL;
    virtualHost *vh = NULL;
    char        host[80];
    int         port;

    for (pcb = cb; pcb != NULL; pcb = pcb->next) {
        if (strcmp("modules", pcb->name) == 0) {
            // este bloque no se procesará aquí
            continue;
        } else if (strcmp("aliases", pcb->name) == 0) {
            aliases = pcb;
        } else {
            if (bc == NULL) {
                bc = (bindConnect *)malloc(sizeof(bindConnect));
                pbc = bc;
                tor_get_bindhost(pcb, "bind", 0, pbc->host);
                pbc->port = tor_get_bindport(pcb, "bind", 0);
                pbc->vhosts = NULL;
                pbc->modules = modules;
                pbc->next = NULL;
            } else {
                tor_get_bindhost(pcb, "bind", 0, host);
                port = tor_get_bindport(pcb, "bind", 0);
                for (pibc = bc; pibc != NULL; pbc = pibc, pibc = pibc->next) {
                    if (strcmp(host, pibc->host) == 0 && port == pibc->port)
                        break;
                }
                if (pibc == NULL) {
                    pbc->next = (bindConnect *)malloc(sizeof(bindConnect));
                    pbc = bc->next;
                    pbc->vhosts = NULL;
                    pbc->modules = modules;
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
        if (strcmp(pvh->host_name, name) == 0) {
            return pvh;
        }
    }
    return NULL;
}

hostLocation* tor_connector_find_location( hostLocation *hl, char *loc ) {
    hostLocation* phl;

    for (phl = hl; phl != NULL; phl = phl->next) {
        if (strncmp(phl->base_uri, loc, strlen(phl->base_uri)) == 0) {
            return phl;
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
        strcpy(vh->host_name, cb->name);
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
                strcpy(pha->alias, tor_get_detail_key(aliases, vh->host_name, i));
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

    strcpy(phl->base_uri, cb->lastname);
    for (pcd = cb->details; pcd != NULL; pcd = pcd->next) {
        if (strcmp(pcd->key, "bind") != 0) {
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
