#include "spinlock.h"

int spinlock_init(spinlock_t *splock) 
{
    #ifndef __GNUC__
        return ERR_UNSUPPORTED_COMPILER;
    #endif

    splock->is_acquired = 0;
    return 0;
}

int spinlock_lock(spinlock_t *splock) 
{
    #ifndef __GNUC__
        return ERR_UNSUPPORTED_COMPILER;
    #endif

    int old_value = 0;
    int new_value = 1;
    int actual_value = -1;
    
    while (actual_value != 0) 
    {
        /* 
         * Atomically sets *splock->is_acquired to new_value if its current value is old_value.
         * Returns the initial value of *splock->is_acquired.
         */
        actual_value = __sync_val_compare_and_swap(&splock->is_acquired, old_value, new_value);
        if (actual_value != 0 && actual_value != 1)
            return ERR_CAS_FUNC;
    }

    return 0;
}

int spinlock_trylock(spinlock_t *splock) 
{
    #ifndef __GNUC__
        return ERR_UNSUPPORTED_COMPILER;
    #endif

    int old_value = 0;
    int new_value = 1;
    int actual_value = -1;
    
    actual_value = __sync_val_compare_and_swap(&splock->is_acquired, old_value, new_value);

    if (actual_value != 0 && actual_value != 1)
        return ERR_CAS_FUNC;
    if (actual_value == 1) 
        return SLBUSY;

    return 1;
}

int spinlock_unlock(spinlock_t *splock) 
{
    #ifndef __GNUC__
        return ERR_UNSUPPORTED_COMPILER;
    #endif

    int actual_value;
    
    actual_value = __sync_val_compare_and_swap(&splock->is_acquired, 1, 0);
    if (actual_value != 0 && actual_value != 1)
        return ERR_CAS_FUNC;
    if (actual_value == 0)
        return ERR_TRY_UNLOCK_NOT_LOCKED;
    return 0;
}
