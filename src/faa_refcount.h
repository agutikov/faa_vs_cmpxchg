#pragma once

#include "base_refcount.h"

struct faa_decref
{
    static int decref(std::atomic<int>* r)
    {
        // release: writes through the shared object happen-before the decrement other threads observe
        return std::atomic_fetch_sub_explicit(r, 1, std::memory_order_release);
    }
};

template<typename T>
using faa_shared_ptr = base_shared_ptr<T, faa_decref>;

