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

