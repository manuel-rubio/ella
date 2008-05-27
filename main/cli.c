/* -*- mode:C; coding:utf-8 -*- */

#include "../include/ella.h"

static cliCommand **commands = NULL;

int ews_cli_help( int pipe, char *params ) {
    cliCommand *pcc;
    int len;
    char buffer[4096] = { 0 };
    sprintf(buffer, "\n%-20s %s\n-------------------- ---------------------------------------------------------\n",
        "Command", "Description");
    len = strlen(buffer);
    for (pcc=*commands; pcc!=NULL; pcc=pcc->next) {
        sprintf(buffer + len, "%-20s %s\n", pcc->name, pcc->description);
        len = strlen(buffer);
    }
    ews_verbose_to(pipe, LOG_LEVEL_INFO, buffer);
    // TODO: do help in deep (long_description)
    return 1;
}

int ews_cli_shutdown( int pipe, char *params ) {
    bindThreadExit = 1;
    return 1;
}

void ews_cli_init( cliCommand **cc ) {
    if (commands == NULL)
        commands = cc;
    ews_verbose(LOG_LEVEL_DEBUG, "CLI init for console");
    ews_cli_add_command(commands, "quit", "close console session.", NULL, NULL);
    ews_cli_add_command(commands, "exit", "close console session.", NULL, NULL);
    ews_cli_add_command(commands, "help", "use help with another topic to see more information.", NULL, ews_cli_help);
    ews_cli_add_command(commands, "shutdown", "shutdown ews", NULL, ews_cli_shutdown);
    ews_verbose(LOG_LEVEL_DEBUG, "end CLI init");
}

int ews_cli_command( int pipe, char *request ) {
    char buffer[512], *cmd, *params = NULL;
    int i, ok = -1;
    cliCommand *pcc;

    strcpy(buffer, request);
    ews_chomp(buffer);
    cmd = (char *) buffer;
    for (i=0; buffer[i]!='\0' && buffer[i]!=' '; i++)
        ;
    if (buffer[i]==' ') {
        cmd[i] = '\0';
        params = cmd + i + 1;
    }
    for (pcc=*commands; pcc!=NULL; pcc=pcc->next) {
        if (pcc->name[0] != 0 && strncmp(cmd, pcc->name, strlen(pcc->name)) == 0) {
            ok = pcc->cli_command(pipe, params);
            break;
        }
    }
    return ok;
}
