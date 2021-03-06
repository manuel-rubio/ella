#include <stdio.h>
#include <string.h>

#include "../include/ella.h"

struct Mime_Types {
    char mime[80];
    char extension[5];
    struct Mime_Types *next;
};

char mime_file[128] = { 0 };
int mime_types_loaded = 0;
struct Mime_Types *mime_types = NULL;


void mime_get_status( char *s ) {
    sprintf(s, "MIME module - %d types loaded from %s", mime_types_loaded, mime_file);
}

char* mime_find_type( char *file ) {
    struct Mime_Types *mt;
    int i, z;

    z = strlen(file);
    for (i=z; i>0 && file[i]!='.'; i--)
        ;
    if (i==0)
        return NULL;
    i++;
    for (mt=mime_types; mt!=NULL; mt=mt->next) {
        if (strcmp(file+i, mt->extension) == 0) {
            ews_verbose(LOG_LEVEL_INFO, "    found [%s] for [%s]", mt->extension, file);
            return mt->mime;
        }
    }
    ews_verbose(LOG_LEVEL_INFO, "    not found mime type for [%s]", file);
    return NULL;
}

int mime_run( struct Bind_Request *br, responseHTTP *rs ) {
    char *type;

    if (rs->headers == NULL) {
        ews_verbose(LOG_LEVEL_ERROR, "    headers not found!");
        return MODULE_RETURN_FAIL;
    }

    switch (rs->content_type) {
        case HEADER_CONTENT_STRING:
            ews_add_header(&rs->headers, "Content-Type", "text/html", 0);
            break;
        case HEADER_CONTENT_FILE:
            type = mime_find_type(rs->content);
            if (type != NULL) {
                ews_add_header(&rs->headers, "Content-Type", type, 0);
            } else {
                ews_add_header(&rs->headers, "Content-Type", "text/plain", 0);
            }
            break;
    }
    return MODULE_RETURN_OK;
}

void mime_load( void ) {
    struct Mime_Types **mt = &mime_types;
    char buffer[100] = { 0 }, *ext;
    FILE *f;
    int i, j, k;

    f = fopen(mime_file, "rt");
    if (!f) {
        ews_verbose(LOG_LEVEL_ERROR, "    types file %s not found or can't be opened.", mime_file);
        return;
    }
    while (fgets(buffer, sizeof(buffer), f)) {
        ews_chomp(buffer);
        for (i=0; buffer[i]!='\0' && buffer[i]!='\t'; i++)
            ;
        if (buffer[i] == '\t')
            for (j=i; buffer[j]=='\t'; j++)
                ;
        else
            continue;
        ext = buffer + j;
        if (strchr(ext, ' ')) {
            do {
                if (*mt != NULL)
                    mt = &(*mt)->next;
                (*mt) = (struct Mime_Types *)ews_malloc(sizeof(struct Mime_Types));
                strncpy((*mt)->mime, buffer, i);
                (*mt)->mime[i] = '\0';
                for (k=0; ext[k]!=' ' && ext[k]!='\0'; k++)
                    (*mt)->extension[k] = ext[k];
                (*mt)->extension[k] = '\0';
                for (j=0; ext[j]!=' ' && ext[j]!='\0'; j++)
                    ;
                ext = (ext[j]==' ') ? ext + j + 1 : ext + j;
                mime_types_loaded++;
            } while (ext[0]!='\0');
        } else {
            if (*mt != NULL)
                mt = &(*mt)->next;
            (*mt) = (struct Mime_Types *)ews_malloc(sizeof(struct Mime_Types));
            strncpy((*mt)->mime, buffer, i);
            (*mt)->mime[i] = '\0';
            strcpy((*mt)->extension, ext);
            mime_types_loaded++;
        }
    }
    fclose(f);
}

void mime_free_types( struct Mime_Types *mt ) {
    if (mt == NULL)
        return;

    if (mt->next != NULL)
        mime_free_types(mt->next);

    ews_free(mt, "mime_free_types");
    mime_types_loaded--;
}

void mime_unload( void ) {
    mime_free_types(mime_types);
}

void mime_error( char *s ) {
    strcpy(s, "Not loaded, you should configure [mime] and types detail.");
}

void mime_reload( configBlock *cb ) {
    configBlock *mime = ews_get_block(cb, "mime", NULL);
    char *types;

    if (mime == NULL) {
        ews_verbose(LOG_LEVEL_ERROR, "    mime module not loaded, you should configure [mime] block.");
        return;
    }

    types = ews_get_detail_value(mime->details, "types", 0);
    if (types == NULL) {
        ews_verbose(LOG_LEVEL_ERROR, "    mime.types file not defined.");
        return;
    }
    strcpy(mime_file, types);

    mime_unload();
    mime_load();
}

void mime_init( moduleTAD *module, cliCommand **cc ) {
    char *types;

    strcpy(module->name, "mime");
    module->type = MODULE_TYPE_HEAD;
    module->priority = 10;

    module->load = NULL;
    module->unload = mime_unload;
    module->reload = mime_reload;
    module->run = NULL;
    module->get_status = mime_error;

    if (!module->details) {
        ews_verbose(LOG_LEVEL_ERROR, "    mime module not loaded, you should configure [mime] block.");
        return;
    }

    types = ews_get_detail_value(module->details, "types", 0);
    if (types == NULL) {
        ews_verbose(LOG_LEVEL_ERROR, "    mime.types file not defined.");
        return;
    }

    module->load = mime_load;
    module->unload = mime_unload;
    module->run = mime_run;
    module->get_status = mime_get_status;
    strcpy(mime_file, types);
}
