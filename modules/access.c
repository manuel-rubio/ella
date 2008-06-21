#include <pthread.h>

#include "../include/ella.h"

enum {
    MASK_TYPE_NUM,
    MASK_TYPE_NET
};

pthread_mutex_t
    access_stats_mutex = PTHREAD_MUTEX_INITIALIZER;

long
    access_granted = 0,
    access_denied = 0;

void access_get_status( char *s ) {
    pthread_mutex_lock(&access_stats_mutex);
    sprintf(s, "Access\n\n\
Total access: %6ld\n\
     granted: %6ld\n\
      denied: %6ld",
    access_granted + access_denied,
    access_granted,
    access_denied);
    pthread_mutex_unlock(&access_stats_mutex);
}

int in_network( const char *address, const char *network ) {
    struct in_addr addr,
                   addr_mask,
                   addr_src;
    char net[16] = { 0 };
    char mask[16] = { 0 };
    int masktype = MASK_TYPE_NUM;
    uint32_t masknum = 0;
    int i, j, nm, flag = 0;

    for (i=0, j=0; network[i]!='\0'; i++) {
        if (!flag) {
            net[i] = network[i];
            if (net[i] == '/') {
                net[i] = '\0';
                flag = 1;
            }
        } else {
            mask[j] = network[i];
            if (mask[j] == '.')
                masktype = MASK_TYPE_NET;
            j++;
        }
    }
    if (masktype == MASK_TYPE_NUM) {
        nm = atoi(mask);
        for (i=0, j=0; i<32; i++, j++) {
            masknum <<= 1;
            if (j>=(32 - nm))
                masknum += 1;
        }
    } else {
        if (!inet_aton(mask, &addr_mask))
            return -1;
        masknum = addr_mask.s_addr;
    }
    if (!inet_aton(net, &addr))
        return -1;
    addr.s_addr &= masknum;
    if (!inet_aton(address, &addr_src))
        return -1;
    addr_src.s_addr &= masknum;
    if (addr_src.s_addr == addr.s_addr) {
        ews_verbose(LOG_LEVEL_DEBUG, "compare [%s] to [%s]: equals", address, network);
        return 1;
    }
    ews_verbose(LOG_LEVEL_DEBUG, "compare [%s] to [%s]: NOT equals!", address, network);
    return 0;
}

int access_run( struct Bind_Request *br, responseHTTP *rs ) {
    requestHTTP *rh = br->request;
    virtualHost *vh = NULL;
    hostLocation *hl = NULL;
    char *host_name = ews_get_header_value(rh, "Host", 0);
    char *host = inet_ntoa((br->client).sin_addr);
    char *policy = NULL;
    char *allow = NULL;
    char *deny = NULL;
    int allow_values = 0;
    int deny_values = 0;
    int i;
    int access = 0;

    if (host_name != NULL) {
        vh = ews_connector_find_vhost((virtualHost *)br->bc->vhosts, host_name);
    }
    if (host_name == NULL || vh == NULL) {
        // gets default, the first vhost
        vh = (virtualHost *)br->bc->vhosts;
    }
    hl = ews_connector_find_location(vh->locations, rh->uri);
    policy = ews_get_detail_value(hl->details, "policy", 0);
    allow_values = ews_get_detail_indexes(hl->details, "allow");
    deny_values = ews_get_detail_indexes(hl->details, "deny");
    if (policy == NULL || strcmp(policy, "allow") == 0) {
        /* allow by default */
        ews_verbose(LOG_LEVEL_DEBUG, "policy configure to [allow]");
        access = 1;
        for (i=0; i<allow_values; i++) {
            allow = ews_get_detail_value(hl->details, "allow", i);
            if (allow && in_network(host, allow) == 1) {
                access = 1;
            }
        }
        for (i=0; i<deny_values; i++) {
            deny = ews_get_detail_value(hl->details, "deny", i);
            if (deny && in_network(host, deny) == 1) {
                access = 0;
            }
        }
    } else if (strcmp(policy, "deny") == 0) {
        /* deny by default */
        ews_verbose(LOG_LEVEL_DEBUG, "policy configure to [deny] with %d allow values and %d deny values");
        access = 0;
        for (i=0; i<deny_values; i++) {
            deny = ews_get_detail_value(hl->details, "deny", i);
            if (deny && in_network(host, deny) == 1) {
                access = 0;
            }
        }
        for (i=0; i<allow_values; i++) {
            allow = ews_get_detail_value(hl->details, "allow", i);
            if (allow && in_network(host, allow) == 1) {
                access = 1;
            }
        }
    } else {
        ews_verbose(LOG_LEVEL_INFO, "policy value isn't valid (%s), it should be: allow or deny.", policy);
    }
    pthread_mutex_lock(&access_stats_mutex);
    if (!access) {
        rs->code = 403;
        access_denied++;
        ews_verbose(LOG_LEVEL_INFO, "access denied for %s to %s", host, host_name);
    } else {
        // code value shouldn't be modified in this case (200)
        access_granted++;
        ews_verbose(LOG_LEVEL_INFO, "access granted for %s to %s", host, host_name);
    }
    pthread_mutex_unlock(&access_stats_mutex);
    return MODULE_RETURN_OK;
}

int access_cli_info( int pipe, char *params ) {
    if (params != NULL) {
        if (strncmp(params, "reset", strlen(params)) == 0) {
            pthread_mutex_lock(&access_stats_mutex);
            access_granted = 0;
            access_denied = 0;
            pthread_mutex_unlock(&access_stats_mutex);
            ews_verbose_to(pipe, LOG_LEVEL_INFO, "reset complete.");
        }
    } else {
        pthread_mutex_lock(&access_stats_mutex);
        ews_verbose_to(pipe, LOG_LEVEL_INFO, "Access Control");
        ews_verbose_to(pipe, LOG_LEVEL_INFO, "total access: %6ld", access_granted + access_denied);
        ews_verbose_to(pipe, LOG_LEVEL_INFO, "     granted: %6ld", access_granted);
        ews_verbose_to(pipe, LOG_LEVEL_INFO, "      denied: %6ld", access_denied);
        pthread_mutex_unlock(&access_stats_mutex);
    }
    return 1;
}

void access_init( moduleTAD *module, cliCommand **cc ) {
    strcpy(module->name, "access");
    module->type = MODULE_TYPE_SEC;
    module->priority = 50;

    module->load = NULL;
    module->unload = NULL;
    module->reload = NULL;
    module->get_status = access_get_status;
    module->run = access_run;

    ews_cli_add_command(cc, "access-info", "info about access control stats", "\n\
Sintaxis: access-info [reset]\n\
Description: show stats for access control, granted and denied access.\n", access_cli_info);

}
