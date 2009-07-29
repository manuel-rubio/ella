#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>

#include "../include/ella/config.h"

int main(void) {
    int s, len, i;
    struct sockaddr_un remote;
    char str[4096], *aux = "0.1";
    char *ptr = NULL;
    fd_set rfds;
    struct timeval t;
    static int exitEWS = 0;
    int ok = 0;
    int flags;
    int size = 0;

    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, EWS_CONSOLE_SOCKET);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(s, (struct sockaddr *)&remote, len) == -1) {
        perror("connect");
        exit(1);
    }

    recv(s, str, sizeof(str), 0);
    for (i=0; str[i]!='\0' && str[i]!='/'; i++)
        ;
    if (str[i]=='/') {
        aux = str + i + 1;
        str[i] = '\0';
    }
    printf("\nElla Web Server %s\nCopyright 2008-2009, Bosque Viejo, S.L.\n\n", aux);
    printf("Server: %s\nConnected.\n\n", str);
    printf("CLI> ");
    fflush(stdout);

    flags = fcntl(s, F_GETFL);
    fcntl(s, F_SETFL, flags | O_NONBLOCK);

    while (!exitEWS) {
        t.tv_sec = 0;
        t.tv_usec = 500;

        FD_ZERO(&rfds);
        FD_SET(s, &rfds);
        FD_SET(STDIN_FILENO, &rfds);
        select(s+1, &rfds, NULL, NULL, &t);

        if (FD_ISSET(s, &rfds)) {
            ok = 0;
            while ((len = recv(s, str, sizeof(str) - 1, 0)) > 0) {
                ptr = str;
                size = 0;
                do {
                    printf("\r%s", ptr);
                    size += strlen(ptr) + 1;
                    ptr = str + size;
                } while (size < len);
                ok = 1;
                bzero(str, sizeof(str));
            }
            if (!ok) {
                if (len < 0) {
                    perror("recv");
                } else {
                    printf("Server closed connection\n");
                }
                break;
            }
            printf("\rCLI> ");
            fflush(stdout);
        }
        if (FD_ISSET(STDIN_FILENO, &rfds)) {
            fgets(str, sizeof(str)-1, stdin);
            if (send(s, str, strlen(str), 0) == -1) {
                perror("send");
                exit(1);
            }
            printf("\rCLI> ");
        }
    }
    close(s);
    return 0;
}
