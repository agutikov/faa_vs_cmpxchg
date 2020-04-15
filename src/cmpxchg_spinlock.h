#pragma once

#include <atomic>


template<typename T, typename N>
struct base_spinlock
{
    std::atomic<bool> locked = false;

    void unlock()
    {
        locked = false;
    }

    void lock()
    {
        if (!T::trylock(&locked)) {
            for (;;) {
                if (locked.load() == 0) {
                    if (T::trylock(&locked)) {
                        break;
                    }
                }
                N::nop();
            }
        }
    }
};

struct empty_nop
{
    static void nop() {}
};

struct pause_nop
{
    static void nop()
    {
        __asm ("pause");
    }
};

struct cmpxchg_weak_trylock
{
    static bool trylock(std::atomic<bool>* lock)
    {
        bool v = false;
        return std::atomic_compare_exchange_weak(lock, &v, true);
    }
};

struct cmpxchg_strong_trylock
{
    static bool trylock(std::atomic<bool>* lock)
    {
        bool v = false;
        return std::atomic_compare_exchange_strong(lock, &v, true);
    }
};

using cmpxchg_weak_spinlock = base_spinlock<cmpxchg_weak_trylock, empty_nop>;

using cmpxchg_strong_spinlock = base_spinlock<cmpxchg_strong_trylock, empty_nop>;

using cmpxchg_weak_pause_spinlock = base_spinlock<cmpxchg_weak_trylock, pause_nop>;

using cmpxchg_strong_pause_spinlock = base_spinlock<cmpxchg_strong_trylock, pause_nop>;




