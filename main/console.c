/* -*- mode:C; coding:utf-8 -*- */

#include "../include/main.h"

#define EWS_MAX_CONNECTS 128
#define EWS_STACKSIZE (240 * 1024)

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
    snprintf(response, sizeof(response), "%s/%s", hostname, PACKAGE_VERSION);
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
                case -2: // pressed enter without command
                    break;
                case -1:
                    ews_verbose_to(con->p[1], LOG_LEVEL_ERROR, "while running %s", request);
                    break;
                case 0:
                    ews_verbose_to(con->p[1], LOG_LEVEL_WARN, "application exits incorrect");
                    break;
                default:
                    ews_verbose_to(con->p[1], LOG_LEVEL_DEBUG, "application exits successfully");
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
    pthread_attr_t lattr;

    for (x = 0; x < EWS_MAX_CONNECTS; x++)
        consoles[x].fd = -1;
    ews_cli_init(cc);
    unlink(EWS_CONSOLE_SOCKET);
    ews_socket = socket(PF_UNIX, SOCK_STREAM, 0);
    if (ews_socket < 0) {
        ews_verbose(LOG_LEVEL_WARN, "Unable to create control socket: %s", strerror(errno));
        return -1;
    }
    bzero(&sunaddr, sizeof(sunaddr));
    sunaddr.sun_family = AF_UNIX;
    strncpy(sunaddr.sun_path, EWS_CONSOLE_SOCKET, sizeof(sunaddr.sun_path));
    res = bind(ews_socket, (struct sockaddr *)&sunaddr, strlen(sunaddr.sun_path) + sizeof(sunaddr.sun_family));
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

    return 0;
}
