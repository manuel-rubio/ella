#include "../../util/string.c"
#include "../../config/config.c"

int main() {
    configBlock *cb = NULL, *pcb = NULL;
    int indexes = 0, i;

    cb = ini_read();
    pcb = get_block(cb, "bosqueviejo.net", "/");
    if (pcb) {
        printf("%d indices encontrados\n", indexes = get_detail_indexes(pcb, "index"));
        for (i=0; i<indexes; i++)
            printf("index = %s\n", get_detail_value(pcb, "index", i));
    } else
        printf("No encontrado 'tornasauce'\n");
    free_blocks(cb);
}

