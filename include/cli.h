/* -*- mode:C; coding:utf-8 -*- */

#if !defined __CLI_H
#define __CLI_H

#define EWS_MAX_COMMANDS 128
#define EWS_COMMAND_NAME 20
#define EWS_COMMAND_DESC 100

struct cli_command {
    char name[EWS_COMMAND_NAME];
    char description[EWS_COMMAND_DESC];
    char *long_description;
    int (*cli_command)( int, char * );
    struct cli_command *next;
};

typedef struct cli_command cliCommand;

int ews_cli_add_command( cliCommand **cc, char *name, char *description, char *long_description, int (*cli_command)( int, char * ) );

int ews_cli_del_command( cliCommand **cc, char *name );

void ews_cli_free( cliCommand **c );

#endif
