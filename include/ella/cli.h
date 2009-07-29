/* -*- mode:C; coding:utf-8 -*- */

#if !defined __CLI_H
#define __CLI_H

#define EWS_MAX_COMMANDS 128

#define EWS_COMMAND_NAME 20
#define EWS_COMMAND_DESC 100

/**
 *  Command Line Interpreter Commands. Dynamic list for commands for CLI.
 */
struct cli_command {
    char name[EWS_COMMAND_NAME];         //!< command name
    char description[EWS_COMMAND_DESC];  //!< command description
    char *long_description;              //!< long description for extended help
    int (*cli_command)( int, char * );   //!< function to execute for command
    struct cli_command *next;            //!< next command in list
};

typedef struct cli_command cliCommand;

/**
 *  Add a command to the list.
 *
 *  @param cc pointer to head of cli commands.
 *  @param name command name to add.
 *  @param description command description.
 *  @param long_description pointer to command long description (will be dynamic text).
 *  @param cli_command command to run.
 */
int ews_cli_add_command( cliCommand **cc, char *name, char *description, char *long_description, int (*cli_command)( int, char * ) );

/**
 *  Delete a command from the list.
 *
 *  @param cc pointer to head of cli commands.
 *  @param name command name to erase.
 */
int ews_cli_del_command( cliCommand **cc, char *name );

/**
 *  Free all nodes in list.
 *
 *  @param cc pointer to head of cli commands.
 */
void ews_cli_free( cliCommand **cc );

#endif
