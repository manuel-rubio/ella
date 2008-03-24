#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define NUM_THREADS    10
#define PORT         3550
#define BACKLOG        10
#define MAX_BUFFER   8192

int conexiones[NUM_THREADS] = { 0 };
char *pagina = "\
<html>\n\
<head>\n\
    <title>Tornasauce</title>\n\
</head>\n\
<body>\n\
    <h1>Tornasauce</h1>\n\
    <hr />\n\
    <p>Servidor de Bosque Viejo</p>\n\
</body>\n\
</html>";

void *gestor( void *t ) {
    char buffer[MAX_BUFFER] = { 0 };
    sprintf(buffer,"\
HTTP/1.0 200 OK\n\
Date: Sun, 16 Mar 2008 19:55:06 GMT\n\
Server: Tornasauce/0.1\n\
Last-Modified: Thu, 03 Jan 2008 11:30:47 GMT\n\
Accept-Ranges: bytes\n\
Content-Length: %d\n\
Content-Type: text/html\n\
\n\
%s", strlen(pagina), pagina);
    send(conexiones[(int)t], buffer, strlen(buffer), 0);
    shutdown(conexiones[(int)t], SHUT_RD);
    close(conexiones[(int)t]);
    pthread_exit(NULL);
}

int main( int argc, char **argv ) {
    int fd;
    struct sockaddr_in server;
    struct sockaddr_in client;
    int sin_size;
    pthread_t threads[NUM_THREADS];
    int rc, t;

    if ((fd=socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {
        printf("error en socket()\n");
        exit(-1);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server.sin_zero),8);

    if(bind(fd,(struct sockaddr*)&server,
            sizeof(struct sockaddr))==-1) {
        printf("error en bind() \n");
        exit(-1);
    }

    if(listen(fd,BACKLOG) == -1) {
        printf("error en listen()\n");
        exit(-1);
    }

    for (t=0; t<NUM_THREADS; t++) {
        sin_size = sizeof(struct sockaddr_in);
        if ((conexiones[t] = accept(fd,(struct sockaddr *)&client, &sin_size))==-1) {
            printf("error en accept()\n");
            exit(-1);
        }

        printf("Conexión %d desde %s\n", t, inet_ntoa(client.sin_addr));
        rc = pthread_create(&threads[t], NULL, gestor, (void *)t);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
        printf("Terminada conexión %d desde %s\n", t, inet_ntoa(client.sin_addr));
    }
    pthread_exit(NULL);
}
