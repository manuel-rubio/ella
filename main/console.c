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

static struct console consoles[EWS_MAX_CONNECTS];
static int ews_socket = -1;     /*!< UNIX Socket for allowing remote control */
static pthread_t lthread;

extern char bindThreadExit;

static int fdprint(int fd, const char *s) {
    return write(fd, s, strlen(s) + 1);
}

static void *console(void *vconsole) {
    struct console *con = (struct console *)vconsole;
    char hostname[MAXHOSTNAMELEN] = "";
    char request[512] = { 0 };
    char response[512] = { 0 };
    int res;
    fd_set rfds;
    struct timeval t;
    int highsock;
    int i;

    if (gethostname(hostname, sizeof(hostname)-1))
        strncpy(hostname, "<Unknown>", sizeof(hostname));
    snprintf(response, sizeof(response), "%s/%.1f", hostname, PACKAGE_VERSION);
    fdprint(con->fd, response);
    while (!bindThreadExit) {
        FD_ZERO(&rfds);
        FD_SET(con->fd, &rfds);
        FD_SET(con->p[0], &rfds);
        t.tv_sec = 0;
        t.tv_usec = 500;

        highsock = (con->fd > con->p[0]) ? con->fd : con->p[0];
        select(highsock+1, &rfds, NULL, NULL, &t);

        if (FD_ISSET(con->fd, &rfds)) {
            res = read(con->fd, request, sizeof(request));
            if (res < 1) {
                break;
            }
            request[res] = 0;
            for (i=0; request[i]!='\0'; i++)
                request[i] = tolower(request[i]);
            if (strncmp("quit", request, 4) == 0 || strncmp("exit", request, 4) == 0) {
                ews_verbose_to(con->p[1], LOG_LEVEL_INFO, "Exit from the console");
                break;
            }
            switch (ews_cli_command(con->p[1], request)) {
                case -1:
                    ews_verbose_to(con->p[1], LOG_LEVEL_ERROR, "while running %s", request);
                    break;
                case 0:
                    ews_verbose_to(con->p[1], LOG_LEVEL_WARN, "application exists incorrect");
                    break;
                default:
                    ews_verbose_to(con->p[1], LOG_LEVEL_DEBUG, "application exists successfully");
            }
        }
        if (FD_ISSET(con->p[0], &rfds)) {
            res = read(con->p[0], request, sizeof(request));
            if (res < 1) {
                ews_verbose(LOG_LEVEL_ERROR, "read returned %d", res);
                break;
            }
            res = write(con->fd, request, res);
            if (res < 1)
                break;
        }
    }
    logger_unregister(con->p[1]);
    ews_verbose(LOG_LEVEL_INFO, "Remote UNIX connection disconnected");
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
    pthread_attr_t attr;
    fd_set rfds;
    struct timeval t;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    while (!bindThreadExit) {
        if (ews_socket < 0)
            return NULL;

        t.tv_sec = 0;
        t.tv_usec = 500;

        FD_ZERO(&rfds);
        FD_SET(ews_socket, &rfds);
        select(ews_socket+1, &rfds, NULL, NULL, &t);
        pthread_testcancel();

        if (!FD_ISSET(ews_socket, &rfds))
            continue;

        len = sizeof(sunaddr);
        s = accept(ews_socket, (struct sockaddr *)&sunaddr, &len);
        ews_verbose(LOG_LEVEL_INFO, "received connection request via console socket.");
        if (s < 0) {
            if (errno != EINTR)
                ews_verbose(LOG_LEVEL_WARN, "Accept returned %d: %s", s, strerror(errno));
        } else {
            for (x = 0; x < EWS_MAX_CONNECTS; x++) {
                if (consoles[x].fd < 0) {
                    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, consoles[x].p)) {
                        ews_verbose(LOG_LEVEL_ERROR, "Unable to create pipe: %s", strerror(errno));
                        consoles[x].fd = -1;
                        fdprint(s, "Server failed to create pipe\n");
                        close(s);
                        break;
                    }
                    flags = fcntl(consoles[x].p[1], F_GETFL);
                    fcntl(consoles[x].p[1], F_SETFL, flags | O_NONBLOCK);
                    consoles[x].fd = s;
                    logger_register(consoles[x].p[1]);
                    if (pthread_create(&consoles[x].t, &attr, console, &consoles[x])) {
                        ews_verbose(LOG_LEVEL_WARN, "Unable to spawn thread to handle connection: %s", strerror(errno));
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
                ews_verbose(LOG_LEVEL_WARN, "No more connections allowed");
                close(s);
            } else if (consoles[x].fd > -1) {
                ews_verbose(LOG_LEVEL_INFO, "Remote UNIX connection");
            }
        }
    }
    return NULL;
}

int console_make_socket( cliCommand **cc ) {
    struct sockaddr_un sunaddr;
    int res;
    int x;
    uid_t uid = -1;
    gid_t gid = -1;
    pthread_attr_t lattr;

    for (x = 0; x < EWS_MAX_CONNECTS; x++)
        consoles[x].fd = -1;
    ews_cli_init(cc);
    unlink(EWS_CONSOLE_SOCKET);
    ews_socket = socket(PF_LOCAL, SOCK_STREAM, 0);
    if (ews_socket < 0) {
        ews_verbose(LOG_LEVEL_WARN, "Unable to create control socket: %s", strerror(errno));
        return -1;
    }
    bzero(&sunaddr, sizeof(sunaddr));
    sunaddr.sun_family = AF_LOCAL;
    strncpy(sunaddr.sun_path, EWS_CONSOLE_SOCKET, sizeof(sunaddr.sun_path));
    res = bind(ews_socket, (struct sockaddr *)&sunaddr, sizeof(sunaddr));
    if (res) {
        ews_verbose(LOG_LEVEL_WARN, "Unable to bind socket to %s: %s", EWS_CONSOLE_SOCKET, strerror(errno));
        close(ews_socket);
        ews_socket = -1;
        return -1;
    }
    res = listen(ews_socket, 2);
    if (res < 0) {
        ews_verbose(LOG_LEVEL_WARN, "Unable to listen on socket %s: %s", EWS_CONSOLE_SOCKET, strerror(errno));
        close(ews_socket);
        ews_socket = -1;
        return -1;
    }

    ews_verbose(LOG_LEVEL_INFO, "console thread launched");
    pthread_attr_init(&lattr);
    pthread_attr_setstacksize(&lattr, EWS_STACKSIZE);
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
