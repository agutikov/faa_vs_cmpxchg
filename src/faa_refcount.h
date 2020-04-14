#pragma once

#include "base_refcount.h"

struct faa_decref
{
    static int decref(std::atomic<int>* r)
    {
        return std::atomic_fetch_sub(r, 1);
    }
};

template<typename T>
using faa_shared_ptr = base_shared_ptr<T, faa_decref>;
