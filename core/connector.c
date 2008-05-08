/* -*- mode:C; coding:utf-8 -*- */

#include "../include/ella.h"

/* TODO: definimos los clientes máximos a 10, temporalmente */
#define MAX_CLIENTS 10

char bindThreadExit = 0;

void* ews_connector_launch( void* ptr_bc ) {
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
    fd_server = ews_server_start(&server, bc->host, bc->port, MAX_CLIENTS);
    if (fd_server > 0) {
        fcntl(fd_server, F_SETFL, fcntl(fd_server, F_GETFL, 0) | O_NONBLOCK);
    }

    while (!bindThreadExit) {
        bzero(&client, sizeof(client));
        do {
            fd_client = ews_server_accept(&server, &client, fd_server);
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
        br = (bindRequest *)ews_malloc(sizeof(bindRequest));
        br->request = ews_parse_request(request);
        br->fd_client = fd_client;
        br->bc = bc;
        bcopy(&client, &(br->client), sizeof(client));
        rc = pthread_create(&br->thread, NULL, ews_connector_client_launch, (void *)br);
        if (rc) {
            printf("ERROR: al crear hilo: %d\n", rc);
        }
    }
    printf("INFO: saliendo del programa\n");
    pthread_exit(NULL);
}

void* ews_connector_client_launch( void* ptr_br ) {
    bindRequest *br;
    responseHTTP *rs;
    moduleTAD *pmt;
    char *buffer, bf[1024];
    int res, f, bf_size, bucle = 1;
    int mod_proc = 1, mod_sec = 1, mod_head = 1;

    br = (bindRequest *)ptr_br;
    printf("INFO: Conexión desde: %s\n", inet_ntoa((br->client).sin_addr));

    rs = (responseHTTP *)ews_malloc(sizeof(responseHTTP));
    rs->code = 0;
    rs->message[0] = '\0';
    rs->version[0] = '\0';
    rs->headers = NULL;
    rs->content = NULL;
    rs->content_type = HEADER_CONTENT_NONE;

    if (br->bc == NULL) {
        printf("FATAL: No hay BindConnect en BindRequest para atender la petición\n");
    } else {
        if (br->bc->modules == NULL) {
            printf("FATAL: No hay ModuleTAD en BindConnect de BindRequest para atender la petición\n");
        } else {
            for (pmt = br->bc->modules; pmt!=NULL; pmt=pmt->next) {
                printf("INFO: ejecutando módulo %s\n", pmt->name);
                if (pmt->run != NULL) {
                    if (pmt->type == MODULE_TYPE_SEC && !mod_sec)
                        continue;
                    if (pmt->type == MODULE_TYPE_PROC && !mod_proc)
                        continue;
                    if (pmt->type == MODULE_TYPE_HEAD && !mod_head)
                        continue;
                    res = pmt->run(br, rs);
                    switch (res) {
                        case MODULE_RETURN_FAIL:
                            printf("ERROR: módulo %s tuvo un error de ejecución\n", pmt->name);
                            break;
                        case MODULE_RETURN_PROC_STOP:
                            printf("INFO: módulo %s pide parada de procesamiento en PROC\n", pmt->name);
                            mod_proc = 0;
                            break;
                        case MODULE_RETURN_SEC_STOP:
                            printf("INFO: módulo %s pide parada de procesamiento en SEC\n", pmt->name);
                            mod_sec = 0;
                            break;
                        case MODULE_RETURN_STOP:
                            printf("INFO: módulo %s requiere parada de procesamiento\n", pmt->name);
                            pmt = NULL; /* terminamos el bucle for. */
                            break;
                    }
                } else {
                    printf("ERROR: método 'run' del módulo %s no definido\n", pmt->name);
                }
            }
            buffer = ews_gen_response(rs);
            send(br->fd_client, buffer, strlen(buffer), 0);
            ews_free(buffer, "ews_connector_client_launch");
            switch (rs->content_type) {
                case HEADER_CONTENT_STRING:
                    send(br->fd_client, rs->content, strlen(rs->content), 0);
                    break;
                case HEADER_CONTENT_FILE:
                    f = open(rs->content, O_RDONLY);
                    if (f == -1) {
                        printf("ERROR: fichero no se pudo abrir (%d).\n", errno);
                    } else {
                        while (bucle && (bf_size = read(f, bf, 1024))) {
                            if (bf_size == -1) {
                                printf("ERROR: error durante lectura del fichero %s.\n", rs->content);
                                bucle = 0;
                            } else {
                                send(br->fd_client, bf, bf_size, 0);
                            }
                        }
                        if (bucle) {
                            printf("INFO: Fichero %s enviado correctamente\n", rs->content);
                        }
                        close(f);
                    }
            }
        }
    }

    // cerramos todo antes de salir
    ews_free_response(rs);
    printf("INFO: finalizado procesamiento desde: %s\n", inet_ntoa((br->client).sin_addr));
    shutdown(br->fd_client, SHUT_RD);
    close(br->fd_client);
    ews_connector_bindrequest_free(br);
    pthread_exit(NULL);
}

int ews_server_start( struct sockaddr_in *server, char *host, int port, int max_clients ) {
    int fd;

    if ((fd=socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {
        printf("ERROR: al iniciar el servidor (socket)\n");
        bindThreadExit = 1;
        return -1;
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

int ews_server_accept( struct sockaddr_in* server, struct sockaddr_in* client, int sfd ) {
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
