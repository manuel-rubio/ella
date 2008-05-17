#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>

#include "../include/config.h"

int main(void) {
    int s, len, i;
    struct sockaddr_un remote;
    char str[4096], *aux;
    fd_set rfds;
    struct timeval t;
    static int exitEWS = 0;

    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, __CONSOLE_SOCKET);
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
    printf("\nElla Web Server %s\nCopyright 2008, Bosque Viejo, S.L.\n\n", aux);
    printf("Server: %s\nConnected.\n\n", str);
    printf("CLI> ");
    fflush(stdout);

    while (!exitEWS) {
        t.tv_sec = 0;
        t.tv_usec = 500;

        FD_ZERO(&rfds);
        FD_SET(s, &rfds);
        FD_SET(STDIN_FILENO, &rfds);
        select(s+1, &rfds, NULL, NULL, &t);

        if (FD_ISSET(s, &rfds)) {
            len = recv(s, str, sizeof(str), 0);
            if (len > 0) {
                str[len] = '\0';
                printf("\r%s", str);
            } else {
                if (len < 0) {
                    perror("recv");
                } else {
                    printf("Server closed connection\n");
                }
                exit(1);
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
/*            printf("\rCLI> ");
            fflush(stdout);*/
        }
    }
    close(s);
    return 0;
}
