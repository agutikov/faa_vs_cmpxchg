#pragma once

#include <atomic>


// test-and-test-and-set spinlock parameterized by trylock primitive (weak/strong CAS)
// and spin-wait hint (no-op or `pause`).
template<typename T, typename N>
struct base_spinlock
{
    std::atomic<bool> locked = false;

    void unlock()
    {
        // release: critical-section writes happen-before the next lock holder's acquire
        locked.store(false, std::memory_order_release);
    }

    void lock()
    {
        // fast path: try once without spinning
        if (!T::trylock(&locked)) {
            for (;;) {
                // outer test is just a hint - relaxed avoids cache-line ping-pong while spinning;
                // the CAS below does the real synchronization
                if (locked.load(std::memory_order_relaxed) == 0) {
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
        // x86 pause hint: yields the pipeline and signals a spin-wait to the CPU
        __asm ("pause");
    }
};

struct cmpxchg_weak_trylock
{
    static bool trylock(std::atomic<bool>* lock)
    {
        bool v = false;
        // success=acquire (enter critical section); failure=relaxed (will be retried)
        // weak: may fail spuriously, but trylock callers tolerate that
        return std::atomic_compare_exchange_weak_explicit(lock, &v, true,
                std::memory_order_acquire, std::memory_order_relaxed);
    }
};

struct cmpxchg_strong_trylock
{
    static bool trylock(std::atomic<bool>* lock)
    {
        bool v = false;
        // strong: no spurious failures - one-shot trylock semantics
        return std::atomic_compare_exchange_strong_explicit(lock, &v, true,
                std::memory_order_acquire, std::memory_order_relaxed);
    }
};

using cmpxchg_weak_spinlock = base_spinlock<cmpxchg_weak_trylock, empty_nop>;

using cmpxchg_strong_spinlock = base_spinlock<cmpxchg_strong_trylock, empty_nop>;

using cmpxchg_weak_pause_spinlock = base_spinlock<cmpxchg_weak_trylock, pause_nop>;

using cmpxchg_strong_pause_spinlock = base_spinlock<cmpxchg_strong_trylock, pause_nop>;




