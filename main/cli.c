/* -*- mode:C; coding:utf-8 -*- */

#include "../include/ella.h"

static cliCommand **commands = NULL;

int ews_cli_help( int pipe, char *params ) {
    cliCommand *pcc;

    if (params != NULL) {
        for (pcc=*commands; pcc!=NULL; pcc=pcc->next) {
            if (strcmp(pcc->name, params) == 0) {
                if (pcc->long_description != NULL) {
                    ews_verbose_to(pipe, LOG_LEVEL_INFO, pcc->long_description);
                    break;
                } else {
                    ews_verbose_to(pipe, LOG_LEVEL_INFO, "this command hasn't long description");
                }
            }
        }
        if (pcc == NULL) {
            ews_verbose_to(pipe, LOG_LEVEL_INFO, "command not found");
        }
    } else {
        ews_verbose_to(pipe, LOG_LEVEL_INFO, "%-20s %s", "Command", "Description");
        ews_verbose_to(pipe, LOG_LEVEL_INFO, "-------------------- ---------------------------------------------------------");
        for (pcc=*commands; pcc!=NULL; pcc=pcc->next) {
            ews_verbose_to(pipe, LOG_LEVEL_INFO, "%-20s %s", pcc->name, pcc->description);
        }
    }
    return 1;
}

int ews_cli_shutdown( int pipe, char *params ) {
    bindThreadExit = 1;
    return 1;
}

void ews_cli_init( cliCommand **cc ) {
    if (commands == NULL)
        commands = cc;
    ews_cli_add_command(commands, "memory-stats", "show memory stadistics", "\n\
Sintaxis: memory-stats [reset]\n\
Description: see memory stats about use of memory. You can use 'reset' option\n\
             for reset counters.\n", ews_memory_cli_stats);
    ews_cli_add_command(commands, "quit", "close console session.", "\n\
Sintaxis: quit\n\
Description: exists from ews console.\n", NULL);
    ews_cli_add_command(commands, "exit", "close console session.", "\n\
Sintaxis: exit\n\
Description: same as quit. Exists from ews console.\n", NULL);
    ews_cli_add_command(commands, "help", "use help with another topic to see more information.", "\n\
Sintaxis: help [module]\n\
Description: show all help commands available.\n", ews_cli_help);
    ews_cli_add_command(commands, "shutdown", "shutdown ews", "\n\
Sintaxis: shutdown\n\
Description: send a shutdown signal to ews daemon.\n", ews_cli_shutdown);
}

int ews_cli_command( int pipe, char *request ) {
    char buffer[512], *cmd, *params = NULL;
    int i, ok = -1;
    cliCommand *pcc;

    strcpy(buffer, request);
    ews_chomp(buffer);
    if (strcmp(buffer, "") == 0) {
        return -2;
    }
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
