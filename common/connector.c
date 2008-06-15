/* -*- mode:C; coding:utf-8 -*- */

#include "../include/connector.h"

bindConnect* ews_connector_parse_bind( configBlock *cb, moduleTAD* modules ) {
    bindConnect *bc = NULL,
                *pbc = NULL,
                *pibc = NULL;
    configBlock *pcb = NULL,
                *aliases = NULL;
    virtualHost *vh = NULL;
    char        host[80] = { 0 };
    int         port = 0;

    for (pcb = cb; pcb != NULL; pcb = pcb->next) {
        if (pcb->lastname[0] != '\0') {
            if (bc == NULL) {
                bc = (bindConnect *)ews_malloc(sizeof(bindConnect));
                pbc = bc;
                ews_get_bindhost(pcb, "bind", 0, pbc->host);
                pbc->port = ews_get_bindport(pcb, "bind", 0);
                pbc->proto = ews_get_bindproto(pcb, "proto", 0);
                pbc->vhosts = NULL;
                pbc->modules = modules;
                pbc->next = NULL;
            } else {
                ews_get_bindhost(pcb, "bind", 0, host);
                port = ews_get_bindport(pcb, "bind", 0);
                for (pibc = bc; pibc != NULL; pbc = pibc, pibc = pibc->next) {
                    if (strcmp(host, pibc->host) == 0 && port == pibc->port)
                        break;
                }
                if (pibc == NULL) {
                    pbc->next = (bindConnect *)ews_malloc(sizeof(bindConnect));
                    pbc = pbc->next;
                    pbc->vhosts = NULL;
                    pbc->modules = modules;
                    strcpy(pbc->host, host);
                    pbc->port = port;
                    pbc->next = NULL;
                }
            }
            if (pbc->vhosts != NULL) {
                vh = ews_connector_find_vhost(bc->vhosts, pcb->name);
                ews_connector_parse_vhost(pcb, aliases, &vh);
            } else {
                vh = NULL;
                ews_connector_parse_vhost(pcb, aliases, &pbc->vhosts);
            }
        } else if (strcmp("aliases", pcb->name) == 0) {
            aliases = pcb;
        }
    }
    return bc;
}

virtualHost* ews_connector_find_vhost( virtualHost *vh, char *name ) {
    virtualHost *pvh;

    for (pvh = vh; pvh != NULL; pvh = pvh->next) {
        if (strcmp(pvh->host_name, name) == 0) {
            return pvh;
        }
    }
    return NULL;
}

hostLocation* ews_connector_find_location( hostLocation *hl, char *loc ) {
    hostLocation *phl, *bhl;
    int size, bsize;

    bhl = NULL;
    bsize = 0;
    for (phl = hl; phl != NULL; phl = phl->next) {
        size = strlen(phl->base_uri);
        if (strlen(loc) >= size && strncmp(phl->base_uri, loc, size) == 0) {
            if (size > bsize) {
                bhl = phl;
                bsize = size;
            }
        }
    }
    return bhl;
}

void ews_connector_parse_vhost( configBlock *cb, configBlock *aliases, virtualHost **pvh ) {
    hostAlias     *pha = NULL;
    int           indexes, i;
    virtualHost   *vh = *pvh;

    if (*pvh == NULL) {
        *pvh = (virtualHost *)ews_malloc(sizeof(virtualHost));
        vh = *pvh;
        strcpy(vh->host_name, cb->name);
        vh->aliases = NULL;
        vh->next = NULL;

        // configuring alias for virtual host
        if (aliases != NULL) {
            indexes = ews_get_detail_values(aliases->details, vh->host_name);
            for (i=0; i<indexes; i++) {
                if (vh->aliases == NULL) {
                    vh->aliases = (hostAlias *)ews_malloc(sizeof(hostAlias));
                    pha = vh->aliases;
                } else {
                    pha->next = (hostAlias *)ews_malloc(sizeof(hostAlias));
                    pha = pha->next;
                }
                strcpy(pha->alias, ews_get_detail_key(aliases->details, vh->host_name, i));
            }
        }
    }
    ews_connector_parse_location(cb, vh);
}

void ews_connector_parse_location( configBlock* cb, virtualHost* vh ) {
    hostLocation *phl = NULL;
    configDetail *pcd = NULL,
                 *phl_cd = NULL;

    if (vh->locations == NULL) {
        vh->locations = (hostLocation *)ews_malloc(sizeof(hostLocation));
        phl = vh->locations;
    } else {
        for (phl=vh->locations; phl->next!=NULL; phl=phl->next)
            ;
        phl->next = (hostLocation *)ews_malloc(sizeof(hostLocation));
        phl = phl->next;
    }

    phl->next = NULL;
    strcpy(phl->base_uri, cb->lastname);
    for (pcd = cb->details; pcd != NULL; pcd = pcd->next) {
        if (phl_cd == NULL) {
            phl->details = ews_new_detail(pcd->key, pcd->value, pcd->index);
            phl_cd = phl->details;
        } else {
            phl_cd->next = ews_new_detail(pcd->key, pcd->value, pcd->index);
            phl_cd = phl_cd->next;
        }
    }
}

void ews_connector_bind_free( bindConnect* bc ) {
    if (bc == NULL)
        return;
    if (bc->next != NULL) {
        ews_connector_bind_free(bc->next);
        bc->next = NULL;
    }
    if (bc->vhosts != NULL) {
        ews_connector_vhost_free(bc->vhosts);
        bc->vhosts = NULL;
    }
    ews_free(bc, "ews_connector_bind_free");
}

void ews_connector_bindrequest_free( bindRequest* br ) {
    if (br == NULL)
        return;
    if (br->request != NULL) {
        ews_free_request(br->request);
        br->request = NULL;
    }
    ews_free(br, "ews_connector_bindrequest_free");
}

void ews_connector_vhost_free( virtualHost* vh ) {
    if (vh == NULL)
        return;
    if (vh->next != NULL) {
        ews_connector_vhost_free(vh->next);
        vh->next = NULL;
    }
    if (vh->locations != NULL) {
        ews_connector_location_free(vh->locations);
        vh->locations = NULL;
    }
    if (vh->aliases != NULL) {
        ews_connector_alias_free(vh->aliases);
        vh->aliases = NULL;
    }
    ews_free(vh, "ews_connector_vhost_free");
}

void ews_connector_location_free( hostLocation* hl ) {
    if (hl == NULL)
        return;
    if (hl->next != NULL) {
        ews_connector_location_free(hl->next);
        hl->next = NULL;
    }
    if (hl->details != NULL) {
        ews_free_details(hl->details);
        hl->details = NULL;
    }
    ews_free(hl, "ews_connector_location_free");
}

void ews_connector_alias_free( hostAlias* ha ) {
    if (ha == NULL)
        return;
    if (ha->next != NULL) {
        ews_connector_alias_free(ha->next);
        ha->next = NULL;
    }
    ews_free(ha, "ews_connector_alias_free");
}
