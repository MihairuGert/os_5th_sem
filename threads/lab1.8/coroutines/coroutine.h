#ifndef COROUTINE_H
#define COROUTINE_H

#define _GNU_SOURCE
#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "coro_map.h"
#include "coro_queue.h"

#define STACK_SIZE      8 * 1024
#define SCHEDULER_MAGIC 10000

#define GROUP_CAPACITY_MAGIC 5

#define STACK_MALLOC_ERR    -1
#define CONTEXT_DESTROY_ERR -2
#define CORO_MALLOC_ERR     -3
#define CREATE_CORO_ERR     -4
#define CREATE_SCHED_ERR    -5
#define CORO_INIT_ERR       -6
#define CORO_FINI_ERR       -7

extern coro_map_t cur_coro_map;
extern int        coro_map_init_cnt;

int coro_init();
int coro_fini();

int create_coroutine(coroutine_t *coro,
                    scheduler_t *scheduler, 
                    void (*routine)(void*), 
                    void *arg);
int yeild();

int create_scheduler(scheduler_t *scheduler);
int run_scheduler(scheduler_t *scheduler);

#endif
