/* -*- mode:C; coding:utf-8 -*- */

#include "../include/memory.h"

pthread_mutex_t memory_allocation = PTHREAD_MUTEX_INITIALIZER;
long ews_memory_allocated = 0;
long ews_memory_freed = 0;
long ews_memory = 0;

int ews_memory_ptr = 0;
int ews_memory_data[MAX_MEM_ALLOCS][2] = { 0 };

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
        printf("WARNING: %s intento de liberar memoria no reservada (%d)\n", name, (int)ptr);
    }

    pthread_mutex_unlock(&memory_allocation);
}

void ews_free_all( void ) {
    long freed, freed_size, size;
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
    printf("INFO: liberadas %d reservas y %ld bytes\n", freed, freed_size);
}

void ews_memory_print_units( long units ) {
    double u = (double) units;
    if (units < 1024)
        printf("%.1lf bytes\n", u);
    else if (units < (1024 * 1024))
        printf("%.1lf kbytes\n", u / 1024);
    else if (units < (1024 * 1024 * 1024))
        printf("%.1lf Mbytes\n", u / (1024 * 1024));
}

void ews_memory_stats( void ) {
    pthread_mutex_lock(&memory_allocation);
    printf("INFO: Memoria reservada: ");
    ews_memory_print_units(ews_memory_allocated);
    printf("INFO: Memoria liberada: ");
    ews_memory_print_units(ews_memory_freed);
    printf("INFO: Descuadre (memoria perdida): ");
    ews_memory_print_units(ews_memory);
    printf("INFO: Máximo número de reservas simultáneas: %d\n", ews_max_simult_allocs);
    printf("INFO: Máxima cantidad de memory en uso: ");
    ews_memory_print_units(ews_max_memory_in_use);
    pthread_mutex_unlock(&memory_allocation);
}
