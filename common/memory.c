/* -*- mode:C; coding:utf-8 -*- */

#include "../include/ella.h"

pthread_mutex_t memory_allocation = PTHREAD_MUTEX_INITIALIZER;
long ews_memory_allocated = 0;
long ews_memory_freed = 0;
long ews_memory = 0;

int ews_memory_ptr = 0;
int ews_memory_data[MAX_MEM_ALLOCS][2] = { { 0 } };

int ews_max_simult_allocs = 0;
long ews_max_memory_in_use = 0;

void* ews_malloc( int size ) {
    void *data = NULL;

    pthread_mutex_lock(&memory_allocation);

    ews_memory_allocated += (long)size;
    ews_memory += (long)size;
    data = malloc(size);

    ews_memory_data[ews_memory_ptr][0] = (int)data;
    ews_memory_data[ews_memory_ptr][1] = size;
    ews_memory_ptr ++;

    if (ews_memory_ptr > ews_max_simult_allocs)
        ews_max_simult_allocs ++;

    if ((ews_memory_allocated - ews_memory_freed) > ews_max_memory_in_use)
        ews_max_memory_in_use = (ews_memory_allocated - ews_memory_freed);

    pthread_mutex_unlock(&memory_allocation);

    return data;
}

void ews_free( void *ptr, const char *name ) {
    int i, j, size;
    pthread_mutex_lock(&memory_allocation);

    for (i=0, j=-1; i<ews_memory_ptr; i++) {
        if (ews_memory_data[i][0] == (int)ptr) {
            j = i;
            break;
        }
    }
    if (j != -1) {
        size = ews_memory_data[j][1];
        ews_memory_freed += (long)size;
        ews_memory -= (long)size;
        free(ptr);
        for (i=j; i<MAX_MEM_ALLOCS-1; i++) {
            ews_memory_data[i][0] = ews_memory_data[i+1][0];
            ews_memory_data[i][1] = ews_memory_data[i+1][1];
        }
        ews_memory_data[MAX_MEM_ALLOCS-1][0] = 0;
        ews_memory_data[MAX_MEM_ALLOCS-1][1] = 0;
        ews_memory_ptr --;
    } else {
        ews_verbose(LOG_LEVEL_WARN, "%s try to free not allocate memory (%d)", name, (int)ptr);
    }

    pthread_mutex_unlock(&memory_allocation);
}

void ews_free_all( void ) {
    long freed = 0, freed_size = 0, size = 0;
    int i;

    pthread_mutex_lock(&memory_allocation);
    for (i=0, freed=0; i<MAX_MEM_ALLOCS; i++) {
        if (ews_memory_data[i][0] != 0) {
            size = ews_memory_data[i][1];
            ews_memory_freed += (long)size;
            ews_memory -= (long)size;
            freed ++;
            freed_size += size;
            free((void *)ews_memory_data[i][0]);
            ews_memory_data[i][0] = 0;
            ews_memory_data[i][1] = 0;
        }
    }
    ews_memory_ptr = 0;
    pthread_mutex_unlock(&memory_allocation);
    ews_verbose(LOG_LEVEL_INFO, "%ld allocations freed for %ld bytes", freed, freed_size);
}

void ews_memory_print_units( char *buffer, long units ) {
    double u = (double) units;
    if (units < 1024)
        sprintf(buffer, "%.1lf bytes", u);
    else if (units < (1024 * 1024))
        sprintf(buffer, "%.1lf kbytes", u / 1024);
    else if (units < (1024 * 1024 * 1024))
        sprintf(buffer, "%.1lf Mbytes", u / (1024 * 1024));
}

void ews_memory_stats( void ) {
    char buffer[256];
    pthread_mutex_lock(&memory_allocation);

    strcpy(buffer, "Allocated memory: ");
    ews_memory_print_units(buffer + strlen(buffer), ews_memory_allocated);
    ews_verbose(LOG_LEVEL_INFO, buffer);

    strcpy(buffer, "Freed memory: ");
    ews_memory_print_units(buffer + strlen(buffer), ews_memory_freed);
    ews_verbose(LOG_LEVEL_INFO, buffer);

    strcpy(buffer, "Lost memory: ");
    ews_memory_print_units(buffer + strlen(buffer), ews_memory);
    ews_verbose(LOG_LEVEL_INFO, buffer);

    ews_verbose(LOG_LEVEL_INFO, "Max. simultaneous allocations: %d", ews_max_simult_allocs);

    strcpy(buffer, "Max. memory in use: ");
    ews_memory_print_units(buffer + strlen(buffer), ews_max_memory_in_use);
    ews_verbose(LOG_LEVEL_INFO, buffer);

    pthread_mutex_unlock(&memory_allocation);
}

int ews_memory_cli_stats( int pipe, char *params ) {
    char buffer[512];
    int i;
    pthread_mutex_lock(&memory_allocation);

    if (params != NULL) {
        if (strncmp(params, "reset", 5) == 0) {
            ews_memory_allocated -= ews_memory_freed;
            ews_memory_freed = 0;
            ews_max_simult_allocs = 0;
            for (i=0; i<MAX_MEM_ALLOCS; i++)
                if (ews_memory_data[i][0] != 0)
                    ews_max_simult_allocs ++;
            ews_max_memory_in_use = ews_memory;
            ews_verbose_to(pipe, LOG_LEVEL_INFO, "memory reset complete.");
        }
    } else {
        strcpy(buffer, "Allocated memory: ");
        ews_memory_print_units(buffer + strlen(buffer), ews_memory_allocated);
        ews_verbose_to(pipe, LOG_LEVEL_INFO, buffer);

        strcpy(buffer, "Freed memory: ");
        ews_memory_print_units(buffer + strlen(buffer), ews_memory_freed);
        ews_verbose_to(pipe, LOG_LEVEL_INFO, buffer);

        strcpy(buffer, "Lost memory: ");
        ews_memory_print_units(buffer + strlen(buffer), ews_memory);
        ews_verbose_to(pipe, LOG_LEVEL_INFO, buffer);

        ews_verbose_to(pipe, LOG_LEVEL_INFO, "Max. simultaneous allocations: %d", ews_max_simult_allocs);

        strcpy(buffer, "Max. memory in use: ");
        ews_memory_print_units(buffer + strlen(buffer), ews_max_memory_in_use);
        ews_verbose_to(pipe, LOG_LEVEL_INFO, buffer);
    }

    pthread_mutex_unlock(&memory_allocation);
    return 1;
}
