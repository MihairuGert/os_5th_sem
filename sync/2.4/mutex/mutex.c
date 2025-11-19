#include "mutex.h"
#include <stdatomic.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <linux/futex.h>

static int cmpxchg(atomic_int *atom, int expected, int desired) {
    int old = expected;
    atomic_compare_exchange_strong(atom, &old, desired);
    return old;
}

int mutex_init(mutex_t *mutex) {
    atomic_store(&mutex->state, 0);
    return 0;
}

int mutex_lock(mutex_t *mutex) {
    int c = cmpxchg(&mutex->state, 0, 1);
    if (c != 0) {
        do {
            if (c == 2 || cmpxchg(&mutex->state, 1, 2) != 0) {
                syscall(SYS_futex, &mutex->state, FUTEX_WAIT_PRIVATE, 2, NULL, NULL, 0);
            }
        } while ((c = cmpxchg(&mutex->state, 0, 2)) != 0);
    }
    return 0;
}

int mutex_trylock(mutex_t *mutex) {
    int expected = 0;
    return atomic_compare_exchange_strong(&mutex->state, &expected, 1) ? 1 : 0;
}

int mutex_unlock(mutex_t *mutex) {
    if (atomic_fetch_sub(&mutex->state, 1) != 1) {
        atomic_store(&mutex->state, 0);
        syscall(SYS_futex, &mutex->state, FUTEX_WAKE_PRIVATE, 1, NULL, NULL, 0);
    }
    return 0;
}
