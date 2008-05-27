/* -*- mode:C; coding:utf-8 -*- */

#include "../include/cli.h"
#include "../include/ella.h"

int ews_cli_add_command( cliCommand **cc, char *name, char *description, char *long_description, int (*cli_command)( int, char * ) ) {
    cliCommand *pcc, *new_cc;

    new_cc = (cliCommand *)ews_malloc(sizeof(cliCommand));
    strcpy(new_cc->name, name);
    strcpy(new_cc->description, description);
    new_cc->long_description = long_description;
    new_cc->cli_command = cli_command;
    new_cc->next = NULL;
    ews_verbose(LOG_LEVEL_INFO, "registro de [%s] ([%d]->[%d] commands)", name, cc, *cc);

    if ((*cc) == NULL) {
        *cc = new_cc;
        ews_verbose(LOG_LEVEL_INFO, "asignado y retornando");
        return 1;
    }
    for (pcc=*cc; pcc->next!=NULL; pcc=pcc->next)
        ;
    pcc->next = new_cc;
    return 1;
}

int ews_cli_del_command( cliCommand **cc, char *name ) {
    cliCommand *pcc, *tmp_cc;
    if (strcmp((*cc)->name, name)) {
        pcc = (*cc);
        (*cc) = (*cc)->next;
        ews_free(pcc, "ews_cli_del_command");
        return 1;
    }
    for (pcc=*cc; pcc->next!=NULL; pcc=pcc->next) {
        if (strcmp(pcc->next->name, name) == 0) {
            tmp_cc = pcc->next;
            pcc->next = tmp_cc->next;
            ews_free(tmp_cc, "ews_cli_del_command");
            return 1;
        }
    }
    return 0;
}

void ews_cli_free( cliCommand **c ) {
    if (c == NULL)
        return;
    if (*c == NULL)
        return;
    if ((*c)->next != NULL)
        ews_cli_free(&((*c)->next));
    ews_free(*c, "ews_cli_free");
    *c = NULL;
}

