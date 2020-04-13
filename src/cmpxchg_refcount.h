#pragma once

#include "base_refcount.h"

struct cmpxchg_strong_decref
{
    static bool decref(std::atomic<int>* r)
    {
        int v;
        do {
            v = r->load();
        } while (!std::atomic_compare_exchange_strong(r, &v, v-1));
        return v == 1;
    }
};

struct cmpxchg_weak_decref
{
    static bool decref(std::atomic<int>* r)
    {
        int v;
        do {
            v = r->load();
        } while (!std::atomic_compare_exchange_weak(r, &v, v-1));
        return v == 1;
    }
};

template<typename T>
using cmpxchg_strong_shared_ptr = base_shared_ptr<T, cmpxchg_strong_decref>;

template<typename T>
using cmpxchg_weak_shared_ptr = base_shared_ptr<T, cmpxchg_weak_decref>;
