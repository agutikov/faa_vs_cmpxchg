#pragma once

#include <atomic>


struct spinlock1
{
    std::atomic<bool> locked = false;

    void unlock()
    {
        locked = false;
    }

    void lock()
    {
        bool v = false;
        if (!std::atomic_compare_exchange_weak(&locked, &v, true)) {
            for (;;) {
                __asm ("pause");
                v = locked.load();
                if (!v) {
                    if (std::atomic_compare_exchange_weak(&locked, &v, true)) {
                        break;
                    }
                }
            }
        }
    }
};

struct spinlock2
{
    std::atomic<bool> locked = false;

    void unlock()
    {
        locked = false;
    }

    void lock()
    {
        bool v = false;
        if (!std::atomic_compare_exchange_weak(&locked, &v, true)) {
            for (;;) {
                v = locked.load();
                if (!v) {
                    if (std::atomic_compare_exchange_weak(&locked, &v, true)) {
                        break;
                    }
                }
                __asm ("pause");
            }
        }
    }
};

template<typename T, typename N>
struct base_spinlock
{
    std::atomic<int> locked = false;

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
    static bool trylock(std::atomic<int>* lock)
    {
        int v = 0;
        return std::atomic_compare_exchange_weak(lock, &v, 1);
    }
};

struct cmpxchg_strong_trylock
{
    static bool trylock(std::atomic<int>* lock)
    {
        int v = 0;
        return std::atomic_compare_exchange_strong(lock, &v, 1);
    }
};

using cmpxchg_weak_spinlock = base_spinlock<cmpxchg_weak_trylock, empty_nop>;

using cmpxchg_strong_spinlock = base_spinlock<cmpxchg_strong_trylock, empty_nop>;

using cmpxchg_weak_pause_spinlock = base_spinlock<cmpxchg_weak_trylock, pause_nop>;

using cmpxchg_strong_pause_spinlock = base_spinlock<cmpxchg_strong_trylock, pause_nop>;




