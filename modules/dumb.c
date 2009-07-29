#include <string.h>
#include "../include/ella.h"

void dumb_get_status( char *s ) {
    strcpy(s, "OK");
}

int dumb_run( struct Bind_Request *br, responseHTTP *rs ) {
    headerHTTP *hh;
    requestHTTP *rh = br->request;

    char *pagina = "\
<html>\n\
<head>\n\
    <title>Ella Web Server</title>\n\
</head>\n\
<body>\n\
    <h1>Ella Web Server</h1>\n\
    <hr />\n\
    <p>Ella web server in dumb test.</p>\n\
    <hr />\n\
    <form method=\"post\">\n\
    <input type=\"text\" name=\"hi\" />\n\
    <input type=\"submit\" />\n\
    </form>\n\
</body>\n\
</html>";

    rs->code = 200;
    strcpy(rs->message, "OK");
    strcpy(rs->version, "1.0");
    ews_add_header(&rs->headers, "Date", "Sun, 16 Mar 2008 19:55:06 GMT", 0);
    ews_add_header(&rs->headers, "Server", "ews/0.1", 0);
    ews_add_header(&rs->headers, "Last-Modified", "Thu, 03 Jan 2008 11:30:47 GMT", 0);
    ews_add_header(&rs->headers, "Accept-Ranges", "bytes", 0);
    ews_add_header(&rs->headers, "Content-Type", "text/html", 0);
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
