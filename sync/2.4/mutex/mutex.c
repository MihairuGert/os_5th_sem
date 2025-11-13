#include "mutex.h"
#include <errno.h>

static int futex(int *uaddr, int futex_op, int val, int *uaddr2, int val3) {
    return syscall(SYS_futex, uaddr, futex_op, val, NULL, uaddr2, val3);
}

int mutex_init(mutex_t *mutex)
{
    atomic_store(&mutex->state, 0);
    return 0;
}

int mutex_lock(mutex_t *mutex) {
    while (1) {
        int expected = 0;
        if (atomic_compare_exchange_strong(&mutex->state, &expected, 1)) {
            return 0;
        }
        
        if (expected == 1) {
            if (atomic_compare_exchange_strong(&mutex->state, &expected, 2)) {
                expected = 2;
            } else {
                continue;
            }
        }

        if (expected == 2) {
            int current_state = atomic_load(&mutex->state);
            if (current_state == 2) {
                futex((int*)&mutex->state, FUTEX_WAIT_PRIVATE, 2, NULL, 0);
            }
        }
    }
}

int mutex_trylock(mutex_t *mutex)
{
    int expected = 0;
    return atomic_compare_exchange_strong(&mutex->state, &expected, 1) ? 1 : 0;
}

int mutex_unlock(mutex_t *mutex)
{
    int old = atomic_exchange(&mutex->state, 0);
    
    if (old == 2) {
        futex((int*)&mutex->state, FUTEX_WAKE_PRIVATE, 1, NULL, 0);
    }
    return 0;
}
