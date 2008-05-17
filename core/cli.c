/* -*- mode:C; coding:utf-8 -*- */

#include "../include/ella.h"

#define EWS_MAX_COMMANDS 128

struct command {
    char name[20];
    char description[100];
    char *long_description;
    int (*cli_command)( int, char * );
};

static struct command commands[EWS_MAX_COMMANDS];
static pthread_mutex_t cli_commands_mutex = PTHREAD_MUTEX_INITIALIZER;

int ews_cli_add_command( char *name, char *description, char *long_description, int (*cli_command)( int, char * ) ) {
    int i, ok = 0;
    pthread_mutex_lock(&cli_commands_mutex);
    for (i=0; i<EWS_MAX_COMMANDS && commands[i].name[0] != '\0'; i++)
        ;
    if (i<EWS_MAX_COMMANDS) {
        strcpy(commands[i].name, name);
        strcpy(commands[i].description, description);
        commands[i].long_description = long_description;
        commands[i].cli_command = cli_command;
        ews_verbose(LOG_LEVEL_DEBUG, "Agregada función a consola (%d): %s", i, name);
        ok = 1;
    } else {
        ews_verbose(LOG_LEVEL_WARN, "No queda espacio para más comandos. Registro de %s fallido.", name);
    }
    pthread_mutex_unlock(&cli_commands_mutex);
    return ok;
}

int ews_cli_help( int pipe, char *params ) {
    int i, len;
    char buffer[4096] = { 0 };
    sprintf(buffer, "\n%-20s %s\n-------------------- ---------------------------------------------------------\n",
        "Command", "Description");
    len = strlen(buffer);
    for (i=0; i<EWS_MAX_COMMANDS; i++) {
        if (commands[i].name[0] != 0) {
            sprintf(buffer + len, "%-20s %s\n", commands[i].name, commands[i].description);
            len = strlen(buffer);
        }
    }
    ews_verbose_to(pipe, LOG_LEVEL_INFO, buffer);
    // TODO: hacer ayuda en profundidad (long_description)
    return 1;
}

void ews_cli_init() {
    int i;
    ews_verbose(LOG_LEVEL_DEBUG, "inicializando CLI para console");
    pthread_mutex_lock(&cli_commands_mutex);
    for (i=0; i<EWS_MAX_COMMANDS; i++) {
        commands[i].name[0] = 0;
        commands[i].description[0] = 0;
        commands[i].long_description = NULL;
        commands[i].cli_command = NULL;
    }
    pthread_mutex_unlock(&cli_commands_mutex);

    ews_cli_add_command("quit", "close console session.", NULL, NULL);
    ews_cli_add_command("exit", "close console session.", NULL, NULL);
    ews_cli_add_command("help", "use help with another topic to see more information.", NULL, ews_cli_help);
    ews_verbose(LOG_LEVEL_DEBUG, "terminada inicialización de CLI");
}

int ews_cli_command( int pipe, char *request ) {
    char buffer[512], *cmd, *params = NULL;
    int i, ok = -1;

    strcpy(buffer, request);
    ews_chomp(buffer);
    cmd = (char *) buffer;
    for (i=0; buffer[i]!='\0' && buffer[i]!=' '; i++)
        ;
    if (buffer[i]==' ') {
        cmd[i] = '\0';
        params = cmd + i + 1;
    }
    pthread_mutex_lock(&cli_commands_mutex);
    for (i=0; i<EWS_MAX_COMMANDS; i++) {
        if (commands[i].name[0] != 0 && strncmp(cmd, commands[i].name, strlen(commands[i].name)) == 0) {
            ok = commands[i].cli_command(pipe, params);
            break;
        }
    }
    pthread_mutex_unlock(&cli_commands_mutex);
    return ok;
}
