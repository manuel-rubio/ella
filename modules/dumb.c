#include <string.h>
#include "../include/modules.h"
#include "../include/header.h"
#include "../include/connector.h"

void dumb_get_status( char *s ) {
    strcpy(s, "OK");
}

int dumb_run( struct Bind_Request *br, responseHTTP *rs ) {
    headerHTTP *hh;
    requestHTTP *rh = br->request;

    char *pagina = "\
<html>\n\
<head>\n\
    <title>Tornasauce</title>\n\
</head>\n\
<body>\n\
    <h1>Tornasauce</h1>\n\
    <hr />\n\
    <p>Servidor de Bosque Viejo</p>\n\
    <hr />\n\
    <form method=\"post\">\n\
    <input type=\"text\" name=\"hola\" />\n\
    <input type=\"submit\" />\n\
    </form>\n\
</body>\n\
</html>";

    rs->code = 200;
    strcpy(rs->message, "OK");
    strcpy(rs->version, "1.0");
    rs->headers = ews_new_header("Date", "Sun, 16 Mar 2008 19:55:06 GMT", 0);
    hh = rs->headers;
    hh->next = ews_new_header("Server", "Ella Web Server/0.1", 0);
    hh = hh->next;
    hh->next = ews_new_header("Last-Modified", "Thu, 03 Jan 2008 11:30:47 GMT", 0);
    hh = hh->next;
    hh->next = ews_new_header("Accept-Ranges", "bytes", 0);
    hh = hh->next;
    hh->next = ews_new_header("Content-Type", "text/html", 0);
    ews_set_response_content(rs, HEADER_CONTENT_STRING, (void *)pagina);
    return MODULE_RETURN_STOP;
}

void dumb_init( moduleTAD *module, cliCommand **cc ) {
    strcpy(module->name, "dumb");
    module->type = MODULE_TYPE_PROC;
    module->priority = 75;

    module->load = NULL;
    module->unload = NULL;
    module->reload = NULL;
    module->get_status = dumb_get_status;
    module->run = dumb_run;
}
