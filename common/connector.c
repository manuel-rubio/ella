/* -*- mode:C; coding:utf-8 -*- */

#include "../include/connector.h"

bindConnect* tor_connector_parse_bind( configBlock *cb, moduleTAD* modules ) {
    bindConnect *bc = NULL,
                *pbc = NULL,
                *pibc = NULL;
    configBlock *pcb = NULL,
                *aliases = NULL;
    virtualHost *vh = NULL;
    char        host[80] = { 0 };
    int         port = 0;

    for (pcb = cb; pcb != NULL; pcb = pcb->next) {
        if (strcmp("modules", pcb->name) == 0) {
            // este bloque no se procesará aquí
            continue;
        } else if (strcmp("aliases", pcb->name) == 0) {
            aliases = pcb;
        } else if (pcb->lastname[0] != '\0') {
            if (bc == NULL) {
                bc = (bindConnect *)tor_malloc(sizeof(bindConnect));
                pbc = bc;
                tor_get_bindhost(pcb, "bind", 0, pbc->host);
                pbc->port = tor_get_bindport(pcb, "bind", 0);
                pbc->vhosts = NULL;
                pbc->modules = modules;
                pbc->next = NULL;
            } else {
                tor_get_bindhost(pcb, "bind", 0, host);
                port = tor_get_bindport(pcb, "bind", 0);
                for (pibc = bc; pibc != NULL; pbc = pibc, pibc = pibc->next) {
                    if (strcmp(host, pibc->host) == 0 && port == pibc->port)
                        break;
                }
                if (pibc == NULL) {
                    pbc->next = (bindConnect *)tor_malloc(sizeof(bindConnect));
                    pbc = pbc->next;
                    pbc->vhosts = NULL;
                    pbc->modules = modules;
                    strcpy(pbc->host, host);
                    pbc->port = port;
                    pbc->next = NULL;
                }
            }
            if (pbc->vhosts != NULL) {
                vh = tor_connector_find_vhost(bc->vhosts, pcb->name);
                tor_connector_parse_vhost(pcb, aliases, &vh);
            } else {
                vh = NULL;
                tor_connector_parse_vhost(pcb, aliases, &pbc->vhosts);
            }
        }
    }
    return bc;
}

virtualHost* tor_connector_find_vhost( virtualHost *vh, char *name ) {
    virtualHost *pvh;

    for (pvh = vh; pvh != NULL; pvh = pvh->next) {
        if (strcmp(pvh->host_name, name) == 0) {
            return pvh;
        }
    }
    return NULL;
}

hostLocation* tor_connector_find_location( hostLocation *hl, char *loc ) {
    hostLocation* phl;

    for (phl = hl; phl != NULL; phl = phl->next) {
        if (strncmp(phl->base_uri, loc, strlen(phl->base_uri)) == 0) {
            return phl;
        }
    }
    return NULL;
}

void tor_connector_parse_vhost( configBlock *cb, configBlock *aliases, virtualHost **pvh ) {
    hostAlias     *pha;
    int           indexes, i;
    virtualHost   *vh = *pvh;

    if (*pvh == NULL) {
        *pvh = (virtualHost *)tor_malloc(sizeof(virtualHost));
        vh = *pvh;
        strcpy(vh->host_name, cb->name);
        vh->aliases = NULL;
        vh->next = NULL;

        // configuramos los alias para el virtual host
        if (aliases != NULL) {
            indexes = tor_get_detail_values(aliases->details, vh->host_name);
            for (i=0; i<indexes; i++) {
                if (vh->aliases == NULL) {
                    vh->aliases = (hostAlias *)tor_malloc(sizeof(hostAlias));
                    pha = vh->aliases;
                } else {
                    pha->next = (hostAlias *)tor_malloc(sizeof(hostAlias));
                    pha = pha->next;
                }
                strcpy(pha->alias, tor_get_detail_key(aliases->details, vh->host_name, i));
            }
        }
    }
    tor_connector_parse_location(cb, vh);
}

void tor_connector_parse_location( configBlock* cb, virtualHost* vh ) {
    hostLocation *phl = NULL;
    configDetail *pcd = NULL,
                 *phl_cd = NULL;

    if (vh->locations == NULL) {
        vh->locations = (hostLocation *)tor_malloc(sizeof(hostLocation));
        phl = vh->locations;
    } else {
        phl = vh->locations;
        phl->next = (hostLocation *)tor_malloc(sizeof(hostLocation));
        phl = phl->next;
    }

    strcpy(phl->base_uri, cb->lastname);
    for (pcd = cb->details; pcd != NULL; pcd = pcd->next) {
        if (strcmp(pcd->key, "bind") != 0) {
            if (phl_cd == NULL) {
                phl->details = tor_new_detail(pcd->key, pcd->value, pcd->index);
                phl_cd = phl->details;
            } else {
                phl_cd->next = tor_new_detail(pcd->key, pcd->value, pcd->index);
                phl_cd = phl_cd->next;
            }
        }
    }
}

void tor_connector_bind_free( bindConnect* bc ) {
    if (bc == NULL)
        return;
    if (bc->next != NULL)
        tor_connector_bind_free(bc->next);
    if (bc->vhosts != NULL)
        tor_connector_vhost_free(bc->vhosts);
    tor_free(bc, "tor_connector_bind_free");
}

void tor_connector_bindrequest_free( bindRequest* br ) {
    if (br == NULL)
        return;
    if (br->request != NULL)
        tor_free_request(br->request);
    tor_free(br, "tor_connector_bindrequest_free");
}

void tor_connector_vhost_free( virtualHost* vh ) {
    if (vh == NULL)
        return;
    if (vh->next != NULL)
        tor_connector_vhost_free(vh->next);
    if (vh->locations != NULL)
        tor_connector_location_free(vh->locations);
    if (vh->aliases != NULL)
        tor_connector_alias_free(vh->aliases);
    tor_free(vh, "tor_connector_vhost_free");
}

void tor_connector_location_free( hostLocation* hl ) {
    if (hl == NULL)
        return;
    if (hl->next != NULL)
        tor_connector_location_free(hl->next);
    if (hl->details != NULL)
        tor_free_details(hl->details);
    tor_free(hl, "tor_connector_location_free");
}

void tor_connector_alias_free( hostAlias* ha ) {
    if (ha == NULL)
        return;
    if (ha->next != NULL)
        tor_connector_alias_free(ha->next);
    tor_free(ha, "tor_connector_alias_free");
}
