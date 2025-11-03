#ifndef MUTEX_H
#define MUTEX_H

#define _GNU_SOURCE

#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdatomic.h>
#include <errno.h>
#include <time.h>

#define FUTEX_WAIT        0
#define FUTEX_WAKE        1
#define FUTEX_PRIVATE_FLAG 128

#define FUTEX_WAIT_PRIVATE (FUTEX_WAIT | FUTEX_PRIVATE_FLAG)
#define FUTEX_WAKE_PRIVATE (FUTEX_WAKE | FUTEX_PRIVATE_FLAG)

#define MBUSY 0

#define ERR_FUTEX_WAIT -1

typedef struct mutex
{
    /* 
     * State meanings:
     * 0: unlocked
     * 1: locked, no waiters
     * 2: locked, with waiters
     */
    atomic_int state;
} mutex_t;

/* 
 * Function to initialize a mutex.
 */
int mutex_init(mutex_t *mutex);

/* 
 * Function to lock a mutex.
 */
int mutex_lock(mutex_t *mutex);

/* 
 * Function to try to lock a mutex.
 * If mutex is already locked returns MBUSY,
 * else returns 1.
 */
int mutex_trylock(mutex_t *mutex);

/* 
 * Function to unlock a mutex.
 */
int mutex_unlock(mutex_t *mutex);
#endif
