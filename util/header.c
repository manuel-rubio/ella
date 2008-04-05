#include "header.h"

requestHTTP* tor_new_request( char *request, char *uri, char *version ) {
    requestHTTP *rh = (requestHTTP *)malloc(sizeof(requestHTTP));
    rh->headers = NULL;
    tor_copy(request, rh->request);
    tor_copy(uri, rh->uri);
    tor_copy(version, rh->version);
    return rh;
}

headerHTTP* tor_new_header( char *key, char *value, int index ) {
    headerHTTP *h = (headerHTTP *)malloc(sizeof(headerHTTP));
    tor_copy(key, h->key);
    tor_copy(value, h->value);
    h->index = index;
    h->next = NULL;
    return h;
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
