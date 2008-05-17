#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>
#include <rpc/rpc.h>

#define SOCK_PATH "echo_socket"

static void* cli( void *socket ) {
    int s2 = (int)socket;
    char str[100];
    int done, n;

    pthread_detach(pthread_self());
    printf("Connected.\n");

    done = 0;
    do {
        n = recv(s2, str, sizeof(str), 0);
        if (n <= 0) {
            if (n < 0) perror("recv");
            done = 1;
        }

        if (!done)
            if (send(s2, str, n, 0) < 0) {
                perror("send");
                done = 1;
            }
    } while (!done);

    close(s2);
    return NULL;
}

int main(void) {
    int s, s2, t, len;
    struct sockaddr_un local, remote;
    pthread_attr_t attr;
    pthread_t pth;
    void *thread_result;
    struct pollfd fds[1];

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    if ((s = socket(PF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    local.sun_family = AF_UNIX;
    strcpy(local.sun_path, SOCK_PATH);
    unlink(local.sun_path);
    len = strlen(local.sun_path) + sizeof(local.sun_family);
    if (bind(s, (struct sockaddr *)&local, len) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(s, 5) == -1) {
        perror("listen");
        exit(1);
    }

    for(;;) {
        printf("Waiting for a connection...\n");
        fds[0].fd = s;
        fds[0].events = POLLIN;
        s2 = poll(fds, 1, -1);
        printf("INFO: recibida petición de conexión por socket de consola.\n");
        pthread_testcancel();
        if (s2 < 0) {
            if (errno != EINTR)
                printf("WARNING: Accept returned %d: %s\n", s, strerror(errno));
            continue;
        }
        t = sizeof(remote);
        if ((s2 = accept(s, (struct sockaddr *)&remote, &t)) == -1) {
            perror("accept");
            exit(1);
        }

        pthread_create(&pth, &attr, cli, (void *)s2);
        //pthread_join(pth, &thread_result);
    }
    pthread_attr_destroy(&attr);
    pthread_exit(NULL);
    return 0;
}
