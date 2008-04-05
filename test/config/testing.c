#include "../../util/string.c"
#include "../../config/config.c"

int main() {
    configBlock *cb = NULL, *pcb = NULL;
    int indexes = 0, i;
    char host[50];
    int port;

    cb = tor_ini_read();
    pcb = tor_get_block(cb, "tornasauce", NULL);
    if (pcb) {
        printf("%d binds\n", indexes = tor_get_detail_indexes(pcb, "bind"));
        for (i=0; i<indexes; i++) {
            tor_get_bindhost(pcb, "bind", i, host);
            port = tor_get_bindport(pcb, "bind", i);
            printf("bind\n\thost: %s\n\tport: %d\n", host, port);
        }
    } else
        printf("No encontrado 'tornasauce'\n");
    free_blocks(cb);
}
