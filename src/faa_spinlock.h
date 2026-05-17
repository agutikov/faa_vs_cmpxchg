#pragma once

#include <atomic>

// Pathological FAA-based spinlock: every contended attempt does a destructive
// increment + rollback. Kept here to demonstrate why fetch_add is the wrong primitive.
struct faa_spinlock
{
    std::atomic<int> locked = 0;

    void lock()
    {
        // acquire on the lock-taking RMW; relaxed on the rollback (no critical-section enter)
        while (std::atomic_fetch_add_explicit(&locked, 1, std::memory_order_acquire) != 0) {
            locked.fetch_sub(1, std::memory_order_relaxed);
        }
    }

    void unlock()
    {
        // release: critical-section writes happen-before the next lock holder's acquire
        locked.fetch_sub(1, std::memory_order_release);
    }
};

// fetch_or-based spinlock: OR-ing 1 into an already-locked value is idempotent,
// so failed lock attempts cost a single RMW with no rollback (unlike faa_spinlock).
struct for_spinlock
{
    std::atomic<int> locked = 0;

    void lock()
    {
        // returns 0 iff we transitioned 0 -> 1 (i.e. won the lock)
        while (std::atomic_fetch_or_explicit(&locked, 1, std::memory_order_acquire) != 0) {
            __asm("pause");
        }
    }

    void unlock()
    {
        locked.store(0, std::memory_order_release);
    }
};

