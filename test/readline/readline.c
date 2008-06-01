#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

int main() {
    static char *linea = NULL;

    do {
        if (linea) {
            free(linea);
            linea = NULL;
        }
        linea = readline("CLI> ");
        if (linea && *linea)
            add_history(linea);
    } while (strcmp(linea, "exit") != 0);
}
