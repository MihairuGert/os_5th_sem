#ifndef MASTER_H
#define MASTER_H

#include <stdbool.h>
#include <stdio.h>

#include "errors.h"

#include "../../lib/thread_pool/thread_pool.h"

typedef struct master_thread {
    /* 
     * 
     */
    thread_pool_t child_threads;

    bool was_init;
    
} master_thread_t;

int init_master_thread(master_thread_t *master);

int start_master_thread(master_thread_t *master);

int fini_master_thread(master_thread_t *master);
#endif
