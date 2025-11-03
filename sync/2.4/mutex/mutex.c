#include "mutex.h"

static int futex(int *uaddr, int futex_op, int val, int *uaddr2, int val3) {
    return syscall(SYS_futex, uaddr, futex_op, val, NULL, uaddr2, val3);
}

int mutex_init(mutex_t *mutex)
{
    atomic_store(&mutex->state, 0);
    return 0;
}

int mutex_lock(mutex_t *mutex)
{
    int state;
    
    if (atomic_compare_exchange_strong(&mutex->state, &(int){0}, 1))
        return 0;

    while ((state = atomic_exchange(&mutex->state, 2)) != 0) 
    {
    
        if (state == 2 || 
            atomic_compare_exchange_strong(&mutex->state, &(int){1}, 2)) 
            {
            do 
            {
                if (futex((int*)&mutex->state, FUTEX_WAIT_PRIVATE, 2, NULL, 0) == -1) {
                    if (errno != EAGAIN)
                        return ERR_FUTEX_WAIT;
                }
                state = atomic_load(&mutex->state);
            } while (state == 2);
        }
    }
    return 0;
}

int mutex_trylock(mutex_t *mutex)
{
    int expected = 0;
    return atomic_compare_exchange_strong((int*)&mutex->state, &expected, 1) ? 1 : 0;
}

int mutex_unlock(mutex_t *mutex)
{
    int old = atomic_exchange(&mutex->state, 0);
    
    if (old == 2)
        futex((int*)&mutex->state, FUTEX_WAKE_PRIVATE, 1, NULL, 0);
    return 0;
}
