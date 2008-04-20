/* -*- mode:C; coding:utf-8 -*- */

#include "../include/util/header.h"

requestHTTP* tor_new_request( char *request, char *uri, char *version ) {
    requestHTTP *rh = (requestHTTP *)malloc(sizeof(requestHTTP));
    rh->headers = NULL;
    tor_copy(request, rh->request);
    tor_copy(uri, rh->uri);
    tor_copy(version, rh->version);
    return rh;
}

responseHTTP* tor_new_response( int code, char *message, char *version ) {
    responseHTTP *rs = (responseHTTP *)malloc(sizeof(responseHTTP));
    rs->headers = NULL;
    rs->code = code;
    tor_copy(message, rs->message);
    tor_copy(version, rs->version);
    return rs;
}

headerHTTP* tor_new_header( char *key, char *value, int index ) {
    headerHTTP *h = (headerHTTP *)malloc(sizeof(headerHTTP));
    tor_copy(key, h->key);
    tor_copy(value, h->value);
    h->index = index;
    h->next = NULL;
    return h;
}

void tor_set_response_content( responseHTTP *rs, char *s ) {
    char size[9];
    headerHTTP* ph;

    sprintf(size, "%d", tor_length(s));
    rs->content = s;
    if (rs->headers == NULL) {
        rs->headers = tor_new_header("Content-Length", size, 0);
    } else {
        for (ph = rs->headers; ph->next!=NULL; ph=ph->next)
            ;
        ph->next = tor_new_header("Content-Length", size, 0);
    }
}

void tor_free_request( requestHTTP *rh ) {
    if (rh == NULL) {
        return;
    }
    if (rh->headers != NULL) {
        tor_free_header(rh->headers);
    }
    free(rh);
}

void tor_free_response( responseHTTP *rs ) {
    if (rs == NULL) {
        return;
    }
    if (rs->headers != NULL) {
        tor_free_header(rs->headers);
    }
    free(rs);
}

void tor_free_header( headerHTTP *h ) {
    if (h == NULL) {
        return;
    }
    if (h->next != NULL) {
        tor_free_header(h->next);
    }
    free(h);
}

char* tor_get_header_value( requestHTTP *rh, char *key, int index ) {
    headerHTTP *h;
    if (rh == NULL || rh->headers == NULL)
        return NULL;
    for (h = rh->headers; h != NULL; h = h->next)
        if (tor_compare(h->key, key) == 0 && h->index == index)
            return h->value;
    return NULL;
}

int tor_get_header_indexes( requestHTTP *rh, char *key ) {
    headerHTTP *h;
    int indexes = 0;
    if (rh == NULL || rh->headers == NULL)
        return 0;
    for (h = rh->headers; h != NULL; h = h->next)
        if (tor_compare(h->key, key) == 0)
            indexes++;
    return indexes;
}

requestHTTP* tor_parse_request( char *s ) {
    int i, j, capture;
    requestHTTP *rh = NULL;
    headerHTTP *h = NULL;

    rh = (requestHTTP *)malloc(sizeof(requestHTTP));
    rh->headers = NULL;
    // getting request
    for (i=0; s[i]!='\0' && s[i]!=' '; i++)
        rh->request[i] = s[i];
    rh->request[i] = '\0';

    // getting URI
    for (i = i+1, j=0; s[i]!='\0' && s[i]!=' '; i++, j++)
        rh->uri[j] = s[i];
    rh->uri[j] = '\0';

    // getting version
    for (i = i+1, j=0, capture=0; s[i]!='\0' && s[i]!='\r' && s[i]!='\n'; i++) {
        if (capture)
            rh->version[j++] = s[i];
        else if (s[i] == '/' || s[i] == '1')
            capture = 1;
    }
    rh->version[j] = '\0';

    //getting headers
    for (i=0; s[i]!='\0'; i++) {
        // skip \r and \n
        while ((s[i] == '\r' || s[i] == '\n') && s[i]!='\0')
            i++;
        if (s[i]=='\0')
            break;
        // get header
        if (rh->headers == NULL) {
            rh->headers = (headerHTTP *)malloc(sizeof(headerHTTP));
            h = rh->headers;
        } else {
            h->next = (headerHTTP *)malloc(sizeof(headerHTTP));
            h = h->next;
        }
        h->next = NULL;
        h->index = 0;
        // get key header
        for (j=0; s[i]!='\0' && s[i]!=':'; j++, i++)
            h->key[j] = s[i];
        h->key[j] = '\0';
        tor_trim(h->key);
        // get value header
        for (j=0, i=i+1; s[i]!='\0' && s[i]!='\r' && s[i]!='\n'; j++, i++) {
            if (s[i] == ',') { // new index
                h->value[j] = '\0';
                tor_trim(h->value);
                h->next = (headerHTTP *)malloc(sizeof(headerHTTP));
                tor_copy(h->key, h->next->key);
                h->next->index = h->index + 1;
                h = h->next;
                h->next = NULL;
                j = -1;
            } else {
                h->value[j] = s[i];
            }
        }
        h->value[j] = '\0';
        tor_trim(h->value);
    }
    return rh;
}

char* tor_gen_response( responseHTTP *rs ) {
    char *s, *tmp, buffer[512];
    int size = 8192, content = 0;
    headerHTTP *ph;

    s = (char *)malloc(size);
    sprintf(s, "HTTP/%s %d %s\r\n", rs->version, rs->code, rs->message);
    for (ph = rs->headers; ph!=NULL; ph=ph->next) {
        // TODO: hacer la concatenación de los valores por los índices
        sprintf(buffer, "%s: %s\r\n", ph->key, ph->value);
        if (tor_length(buffer) + tor_length(s) > size) {
            size += 8192;
            tmp = (char *)malloc(size);
            free(s);
            s = tmp;
        }
        tor_concat(s, buffer);
        if (!content && tor_compare(ph->key, "Content-Length") == 0) {
            content = 1;
        }
    }
    if (content) {
        tor_concat(s, "\r\n");
        if (tor_length(buffer) + tor_length(rs->content) > size) {
            size += tor_length(rs->content);
            tmp = (char *)malloc(size);
            free(s);
            s = tmp;
        }
        tor_concat(s, rs->content);
    }
    return s;
}
