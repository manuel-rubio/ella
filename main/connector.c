/* -*- mode:C; coding:utf-8 -*- */

#include "../include/ella.h"

// TODO: get this data from config file
#define MAX_CLIENTS 10

char bindThreadExit = 0;

void* ews_connector_launch( void* ptr_bc ) {
    bindConnect *bc;
    bindRequest *br;
    struct sockaddr_in server, client;
    int fd_server, fd_client;
    int rc;

    bc = (bindConnect *)ptr_bc;
    ews_verbose(LOG_LEVEL_INFO, "Listening at %s:%d", bc->host, bc->port);
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

        br = (bindRequest *)ews_malloc(sizeof(bindRequest));
        br->fd_client = fd_client;
        br->bc = bc;
        bcopy(&client, &(br->client), sizeof(client));
        br->conn_next_status = EWS_CON_CLOSE;
        br->conn_timeout = 300; // default time (300 sec = 5 min)

        rc = pthread_create(&br->thread, NULL, ews_connector_client_launch, (void *)br);
        if (rc) {
            ews_verbose(LOG_LEVEL_ERROR, "thread creation failed: %d", rc);
        }
    }
    ews_verbose(LOG_LEVEL_INFO, "program exit");
    pthread_exit(NULL);
}

int ews_server_read( bindRequest *br, int timeout ) {
    char request[8192], buffer[2048];
    int i, count = 0, res;
    struct timeval t;
    fd_set rfds;

    t.tv_sec = timeout;
    t.tv_usec = 0;
    FD_ZERO(&rfds);
    FD_SET(br->fd_client, &rfds);
    res = select((br->fd_client + 1), &rfds, NULL, NULL, &t);

    if (res <= 0) {
        return 0;
    }

    bzero(request, sizeof(request));
    bzero(buffer, sizeof(buffer));
    while (recv(br->fd_client, buffer, sizeof(buffer), 0) != -1) {
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
    br->request = ews_parse_request(request);
    return 1;
}

void* ews_connector_client_launch( void* ptr_br ) {
    bindRequest *br;
    responseHTTP *rs;
    moduleTAD *pmt;
    char *buffer, bf[BUFFER_SIZE];
    int res, f, bf_size, bucle = 1;
    int mod_proc = 1, mod_sec = 1, mod_head = 1;

    br = (bindRequest *)ptr_br;
    ews_verbose(LOG_LEVEL_INFO, "Connection from: %s", inet_ntoa((br->client).sin_addr));

    rs = (responseHTTP *)ews_malloc(sizeof(responseHTTP));

    if (br->bc == NULL) {
        ews_verbose(LOG_LEVEL_FATAL, "Haven't BindConnect in BindRequest to attend the request");
    } else {
        if (br->bc->modules == NULL) {
            ews_verbose(LOG_LEVEL_FATAL, "Haven't ModuleTAD in BindConnect from BindRequest to attend the request");
        } else {
            do {
                rs->code = 0;
                rs->message[0] = '\0';
                rs->version[0] = '\0';
                rs->headers = NULL;
                rs->content = NULL;
                rs->content_type = HEADER_CONTENT_NONE;

                if (!ews_server_read(br, br->conn_timeout)) {
                    break;
                }

                for (pmt = br->bc->modules; pmt!=NULL; pmt=pmt->next) {
                    ews_verbose(LOG_LEVEL_INFO, "running %s module", pmt->name);
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
                                ews_verbose(LOG_LEVEL_ERROR, "%s module have a running error", pmt->name);
                                break;
                            case MODULE_RETURN_PROC_STOP:
                                ews_verbose(LOG_LEVEL_INFO, "%s module request process PROC stop", pmt->name);
                                mod_proc = 0;
                                break;
                            case MODULE_RETURN_SEC_STOP:
                                ews_verbose(LOG_LEVEL_INFO, "%s module request process SEC stop", pmt->name);
                                mod_sec = 0;
                                break;
                            case MODULE_RETURN_STOP:
                                ews_verbose(LOG_LEVEL_INFO, "%s module request process stop", pmt->name);
                                pmt = NULL; /* end "for" loop. */
                                break;
                        }
                    } else {
                        ews_verbose(LOG_LEVEL_ERROR, "'run' method from %s module not defined", pmt->name);
                    }
                }
                buffer = ews_gen_response(rs);
                send(br->fd_client, buffer, strlen(buffer), 0);
                ews_free(buffer, "ews_connector_client_launch");
                switch (rs->content_type) {
                    case HEADER_CONTENT_STRING:
                    case HEADER_CONTENT_RAW:
                        send(br->fd_client, rs->content, strlen(rs->content), 0);
                        break;
                    case HEADER_CONTENT_FILE:
                        f = open(rs->content, O_RDONLY);
                        if (f == -1) {
                            ews_verbose(LOG_LEVEL_ERROR, "can't open file (%d).", errno);
                        } else {
                            while (bucle && (bf_size = read(f, bf, BUFFER_SIZE))) {
                                if (bf_size == -1) {
                                    ews_verbose(LOG_LEVEL_ERROR, "error while read file %s.", rs->content);
                                    bucle = 0;
                                } else {
                                    send(br->fd_client, bf, bf_size, 0);
                                }
                            }
                            if (bucle) {
                                ews_verbose(LOG_LEVEL_INFO, "%s file send successfully", rs->content);
                            }
                            close(f);
                        }
                }
            } while (br->conn_next_status == EWS_CON_KEEPALIVE);
        }
    }

    // cerramos todo antes de salir
    ews_free_response(rs);
    ews_verbose(LOG_LEVEL_INFO, "finish process from: %s", inet_ntoa((br->client).sin_addr));
    shutdown(br->fd_client, SHUT_RD);
    close(br->fd_client);
    ews_connector_bindrequest_free(br);
    pthread_exit(NULL);
}

int ews_server_start( struct sockaddr_in *server, char *host, int port, int max_clients ) {
    int fd;
    int reuse_addr = 1;

    if ((fd=socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {
        ews_verbose(LOG_LEVEL_ERROR, "in init server (socket) for %s:%d", host, port);
        bindThreadExit = 1;
        return -1;
    }

    /* To re-bind the socket without TIME_WAIT problems */
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));

    server->sin_family = AF_INET;
    server->sin_port = htons(port);
    server->sin_addr.s_addr = inet_addr(host);
    bzero(&(server->sin_zero),8);

    if (bind(fd,(struct sockaddr*)server, sizeof(struct sockaddr))==-1) {
        ews_verbose(LOG_LEVEL_ERROR, "port %d is busy", port);
        exit(-1);
    }

    if (listen(fd, max_clients) == -1) {
        ews_verbose(LOG_LEVEL_ERROR, "can't do 'listen'");
        exit(-1);
    }
    return fd;
}

int ews_server_accept( struct sockaddr_in* server, struct sockaddr_in* client, int sfd ) {
    int fd;
    struct timeval t;
    fd_set rfds;
    socklen_t sin_size;

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
        ews_verbose(LOG_LEVEL_ERROR, "can't accept incoming connection");
        exit(-1);
    }
    return fd;
}
