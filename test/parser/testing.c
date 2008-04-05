#include "util/string.c"
#include "util/header.c"

int main() {
    requestHTTP *rh;
    int i, accepts;
    char *buffer2 = "\
POST / HTTP/1.1\n\
Host: localhost:8080\n\
User-Agent: Mozilla/5.0 (X11; U; Linux i686; es-ES; rv:1.8.1.13) Gecko/20080325 Ubuntu/7.10 (gutsy) Firefox/2.0.0.13\n\
Accept: text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5\n\
Accept-Language: es-es,es;q=0.8,en-us;q=0.5,en;q=0.3\n\
Accept-Encoding: gzip,deflate\n\
Accept-Charset: UTF-8,*\n\
Keep-Alive: 300\n\
Connection: keep-alive\n\
Referer: http://localhost:8080/\n\
Cache-Control: max-age=0\n\
Content-Type: application/x-www-form-urlencoded\n\
Content-Length: 9\n\
\n";

    rh = tor_parse_request(buffer2);
    printf("Petición: %s\nURI: %s\nVersion: %s\nNavegador: %s\n", rh->request, rh->uri, rh->version, tor_get_header_value(rh, "User-Agent", 0));
    printf("Content-Length: %s\n", tor_get_header_value(rh, "Content-Length", 0));
    printf("%d Accepts\n", accepts = tor_get_header_indexes(rh, "Accept"));
    for (i=0; i<accepts; i++)
        printf("\tAccept: %s\n", tor_get_header_value(rh, "Accept", i));
    tor_free_request(rh);
    return 0;
}
