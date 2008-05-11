/* -*- mode:C; coding:utf-8 -*- */

#include "../include/ella.h"

#define EWS_MAX_CONNECTS 128
#define EWS_STACKSIZE 240 * 1024

struct console {
    int fd;      /*!< File descriptor */
    int p[2];    /*!< Pipe */
    pthread_t t; /*!< Thread of handler */
    int mute;    /*!< Is the console muted for logs */
};

struct console consoles[EWS_MAX_CONNECTS];
static int ews_socket = -1;     /*!< UNIX Socket for allowing remote control */
static pthread_t lthread;

static int fdprint(int fd, const char *s) {
    return write(fd, s, strlen(s) + 1);
}

static void *console(void *vconsole) {
    struct console *con = (struct console *)vconsole;
    char hostname[MAXHOSTNAMELEN] = "";
    char tmp[512] = { 0 };
    int res;
    struct pollfd fds[2];

    if (gethostname(hostname, sizeof(hostname)-1))
        strncpy(hostname, "<Unknown>", sizeof(hostname));
    //snprintf(tmp, sizeof(tmp), "%s/%ld/%s\n", hostname, (long)ews_mainpid, EWS_VERSION);
    printf("INFO: con->mute=%d\n", con->mute);
    snprintf(tmp, sizeof(tmp), "%s/%s\n", hostname, EWS_VERSION);
    fdprint(con->fd, tmp);
    for(;;) {
        fds[0].fd = con->fd;
        fds[0].events = POLLIN;
        fds[0].revents = 0;
        fds[1].fd = con->p[0];
        fds[1].events = POLLIN;
        fds[1].revents = 0;

        res = poll(fds, 2, -1);
        if (res < 0) {
            if (errno != EINTR)
                printf("WARNING: poll returned < 0: %s\n", strerror(errno));
            continue;
        }
        if (fds[0].revents) {
            res = read(con->fd, tmp, sizeof(tmp));
            if (res < 1) {
                break;
            }
            tmp[res] = 0;
            // TODO: cli_command(con->fd, tmp);
            printf("INFO: ejecuta %s\n", tmp);
        }
        if (fds[1].revents) {
            res = read(con->p[0], tmp, sizeof(tmp));
            if (res < 1) {
                printf("ERROR: read returned %d\n", res);
                break;
            }
            res = write(con->fd, tmp, res);
            if (res < 1)
                break;
        }
    }
    printf("INFO: Remote UNIX connection disconnected\n");
    close(con->fd);
    close(con->p[0]);
    close(con->p[1]);
    con->fd = -1;

    return NULL;
}

static void *console_launch(void *unused) {
    struct sockaddr_un sunaddr;
    int s;
    socklen_t len;
    int x;
    int flags;
    fd_set rfds;

    for (;;) {
        if (ews_socket < 0)
            return NULL;
        FD_ZERO(&rfds);
        FD_SET(ews_socket, &rfds);
        select(1, &rfds, NULL, NULL, NULL);

        printf("INFO: recibida petición de conexión por socket de consola.\n");
        pthread_testcancel();

        len = sizeof(sunaddr);
        s = accept(ews_socket, (struct sockaddr *)&sunaddr, &len);
        if (s < 0) {
            if (errno != EINTR)
                printf("WARNING: Accept returned %d: %s\n", s, strerror(errno));
        } else {
            for (x = 0; x < EWS_MAX_CONNECTS; x++) {
                if (consoles[x].fd < 0) {
                    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, consoles[x].p)) {
                        printf("ERROR: Unable to create pipe: %s\n", strerror(errno));
                        consoles[x].fd = -1;
                        fdprint(s, "Server failed to create pipe\n");
                        close(s);
                        break;
                    }
                    flags = fcntl(consoles[x].p[1], F_GETFL);
                    fcntl(consoles[x].p[1], F_SETFL, flags | O_NONBLOCK);
                    consoles[x].fd = s;
                    // consoles[x].mute = ews_opt_mute; // TODO: realizar mute
                    if (pthread_create(&consoles[x].t, NULL, console, &consoles[x])) {
                        printf("WARNING: Unable to spawn thread to handle connection: %s\n", strerror(errno));
                        close(consoles[x].p[0]);
                        close(consoles[x].p[1]);
                        consoles[x].fd = -1;
                        fdprint(s, "Server failed to spawn thread\n");
                        close(s);
                    }
                    break;
                }
            }
            if (x >= EWS_MAX_CONNECTS) {
                fdprint(s, "No more connections allowed\n");
                printf("WARNING: No more connections allowed\n");
                close(s);
            } else if (consoles[x].fd > -1) {
                printf("INFO: Remote UNIX connection\n");
            }
        }
    }
    return NULL;
}

int console_make_socket(void) {
    struct sockaddr_un sunaddr;
    int res;
    int x;
    uid_t uid = -1;
    gid_t gid = -1;

    for (x = 0; x < EWS_MAX_CONNECTS; x++)
        consoles[x].fd = -1;
    unlink(__CONSOLE_SOCKET);
    ews_socket = socket(PF_LOCAL, SOCK_STREAM, 0);
    if (ews_socket < 0) {
        printf("WARNING: Unable to create control socket: %s\n", strerror(errno));
        return -1;
    }
    bzero(&sunaddr, sizeof(sunaddr));
    sunaddr.sun_family = AF_LOCAL;
    strncpy(sunaddr.sun_path, __CONSOLE_SOCKET, sizeof(sunaddr.sun_path));
    res = bind(ews_socket, (struct sockaddr *)&sunaddr, sizeof(sunaddr));
    if (res) {
        printf("WARNING: Unable to bind socket to %s: %s\n", __CONSOLE_SOCKET, strerror(errno));
        close(ews_socket);
        ews_socket = -1;
        return -1;
    }
    res = listen(ews_socket, 2);
    if (res < 0) {
        printf("WARNING: Unable to listen on socket %s: %s\n", __CONSOLE_SOCKET, strerror(errno));
        close(ews_socket);
        ews_socket = -1;
        return -1;
    }
//     ast_register_verbose(network_verboser); // TODO: hacer la parte de envío de logs a consola.
    printf("INFO: lanzado hilo para consola\n");
    pthread_create(&lthread, NULL, console_launch, NULL);

// TODO: configurar permisos para el fichero socket de la consola.
//     if (!ast_strlen_zero(ast_config_AST_CTL_OWNER)) {
//         struct passwd *pw;
//         if ((pw = getpwnam(ast_config_AST_CTL_OWNER)) == NULL) {
//             ast_log(LOG_WARNING, "Unable to find uid of user %s\n", ast_config_AST_CTL_OWNER);
//         } else {
//             uid = pw->pw_uid;
//         }
//     }
//
//     if (!ast_strlen_zero(ast_config_AST_CTL_GROUP)) {
//         struct group *grp;
//         if ((grp = getgrnam(ast_config_AST_CTL_GROUP)) == NULL) {
//             ast_log(LOG_WARNING, "Unable to find gid of group %s\n", ast_config_AST_CTL_GROUP);
//         } else {
//             gid = grp->gr_gid;
//         }
//     }
//
//     if (chown(ast_config_AST_SOCKET, uid, gid) < 0)
//         ast_log(LOG_WARNING, "Unable to change ownership of %s: %s\n", ast_config_AST_SOCKET, strerror(errno));
//
//     if (!ast_strlen_zero(ast_config_AST_CTL_PERMISSIONS)) {
//         int p1;
//         mode_t p;
//         sscanf(ast_config_AST_CTL_PERMISSIONS, "%o", &p1);
//         p = p1;
//         if ((chmod(ast_config_AST_SOCKET, p)) < 0)
//             ast_log(LOG_WARNING, "Unable to change file permissions of %s: %s\n", ast_config_AST_SOCKET, strerror(errno));
//     }

    return 0;
}
