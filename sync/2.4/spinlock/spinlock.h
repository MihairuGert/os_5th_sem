#ifndef SPINLOCK_H
#define SPINLOCK_H

#define ERR_UNSUPPORTED_COMPILER    -1
#define ERR_TRY_UNLOCK_NOT_LOCKED   -2
#define ERR_CAS_FUNC                -3

#define SLBUSY 0

typedef struct spinlock
{
    int is_acquired;    
} spinlock_t;

/* 
 * Function to initialize a spinlock.
 */
int spinlock_init(spinlock_t *splock);

/* 
 * Function to acquire a spinlock.
 */
int spinlock_lock(spinlock_t *splock);

/* 
 * Function to try to acquire a spinlock.
 * If spinlock is acquired returns SLBUSY,
 * else returns 1.
 */
int spinlock_trylock(spinlock_t *splock);

/* 
 * Function to release a spinlock.
 */
int spinlock_unlock(spinlock_t *splock);
#endif
