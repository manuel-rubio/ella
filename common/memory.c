/* -*- mode:C; coding:utf-8 -*- */

#include "../include/memory.h"

pthread_mutex_t memory_allocation = PTHREAD_MUTEX_INITIALIZER;
long tor_memory_allocated = 0;
long tor_memory_freed = 0;
long tor_memory = 0;

int tor_memory_ptr = 0;
int tor_memory_data[MAX_MEM_ALLOCS][2] = { 0 };

int tor_max_simult_allocs = 0;
long tor_max_memory_in_use = 0;

void* tor_malloc( int size ) {
    void *data = NULL;

    pthread_mutex_lock(&memory_allocation);

    tor_memory_allocated += (long)size;
    tor_memory += (long)size;
    data = malloc(size);

    tor_memory_data[tor_memory_ptr][0] = (int)data;
    tor_memory_data[tor_memory_ptr][1] = size;
    tor_memory_ptr ++;

    if (tor_memory_ptr > tor_max_simult_allocs)
        tor_max_simult_allocs ++;

    if ((tor_memory_allocated - tor_memory_freed) > tor_max_memory_in_use)
        tor_max_memory_in_use = (tor_memory_allocated - tor_memory_freed);

    pthread_mutex_unlock(&memory_allocation);

    return data;
}

void tor_free( void *ptr, const char *name ) {
    int i, j, size;
    pthread_mutex_lock(&memory_allocation);

    for (i=0, j=-1; i<tor_memory_ptr; i++) {
        if (tor_memory_data[i][0] == (int)ptr) {
            j = i;
            break;
        }
    }
    if (j != -1) {
        size = tor_memory_data[j][1];
        tor_memory_freed += (long)size;
        tor_memory -= (long)size;
        free(ptr);
        for (i=j; i<MAX_MEM_ALLOCS-1; i++) {
            tor_memory_data[i][0] = tor_memory_data[i+1][0];
            tor_memory_data[i][1] = tor_memory_data[i+1][1];
        }
        tor_memory_data[MAX_MEM_ALLOCS-1][0] = 0;
        tor_memory_data[MAX_MEM_ALLOCS-1][1] = 0;
        tor_memory_ptr --;
    } else {
        printf("WARNING: %s intento de liberar memoria no reservada (%d)\n", name, (int)ptr);
    }

    pthread_mutex_unlock(&memory_allocation);
}

void tor_free_all( void ) {
    long freed, freed_size, size;
    int i;

    pthread_mutex_lock(&memory_allocation);
    for (i=0, freed=0; i<MAX_MEM_ALLOCS; i++) {
        if (tor_memory_data[i][0] != 0) {
            size = tor_memory_data[i][1];
            tor_memory_freed += (long)size;
            tor_memory -= (long)size;
            freed ++;
            freed_size += size;
            free((void *)tor_memory_data[i][0]);
            tor_memory_data[i][0] = 0;
            tor_memory_data[i][1] = 0;
        }
    }
    tor_memory_ptr = 0;
    pthread_mutex_unlock(&memory_allocation);
    printf("INFO: liberadas %d reservas y %ld bytes\n", freed, freed_size);
}

void tor_memory_print_units( long units ) {
    double u = (double) units;
    if (units < 1024)
        printf("%.1lf bytes\n", u);
    else if (units < (1024 * 1024))
        printf("%.1lf kbytes\n", u / 1024);
    else if (units < (1024 * 1024 * 1024))
        printf("%.1lf Mbytes\n", u / (1024 * 1024));
}

void tor_memory_stats( void ) {
    pthread_mutex_lock(&memory_allocation);
    printf("INFO: Memoria reservada: ");
    tor_memory_print_units(tor_memory_allocated);
    printf("INFO: Memoria liberada: ");
    tor_memory_print_units(tor_memory_freed);
    printf("INFO: Descuadre (memoria perdida): ");
    tor_memory_print_units(tor_memory);
    printf("INFO: Máximo número de reservas simultáneas: %d\n", tor_max_simult_allocs);
    printf("INFO: Máxima cantidad de memory en uso: ");
    tor_memory_print_units(tor_max_memory_in_use);
    pthread_mutex_unlock(&memory_allocation);
}
