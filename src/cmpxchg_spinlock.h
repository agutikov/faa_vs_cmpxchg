#pragma once

#include <atomic>

struct cmpxchg_spinlock_base
{
    std::atomic<int> locked = 0;

    void unlock()
    {
        locked = 0;
    }
};

struct cmpxchg_spinlock_weak : cmpxchg_spinlock_base
{
    void lock()
    {
        int v;
        for (;;) {
            v = locked.load();
            if (v == 0) {
                if (std::atomic_compare_exchange_weak(&locked, &v, 1)) {
                    break;
                }
            }
        };
    }
};

struct cmpxchg_spinlock_strong : cmpxchg_spinlock_base
{
    void lock()
    {
        int v;
        for (;;) {
            v = locked.load();
            if (v == 0) {
                if (std::atomic_compare_exchange_strong(&locked, &v, 1)) {
                    break;
                }
            }
        };
    }
};



