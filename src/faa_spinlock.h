#pragma once

#include <atomic>

struct faa_spinlock
{
    std::atomic<int> locked = 0;

    void lock()
    {
        while (std::atomic_fetch_add(&locked, 1) != 0) {
            locked--;
        }
    }

    void unlock()
    {
        locked--;
    }
};

struct for_spinlock
{
    std::atomic<int> locked = 0;

    void lock()
    {
        while (std::atomic_fetch_or(&locked, 1) != 0) {
            __asm("pause");
        }
    }

    void unlock()
    {
        locked = 0;
    }
};

