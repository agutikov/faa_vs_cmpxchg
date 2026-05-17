#pragma once

#include "base_refcount.h"

struct cmpxchg_strong_decref
{
    static int decref(std::atomic<int>* r)
    {
        // initial load only - failed CAS already writes the observed value back into v
        int v = r->load(std::memory_order_relaxed);
        // success=release pairs with the acquire fence in reset(); failure=relaxed (just retry)
        while (!std::atomic_compare_exchange_strong_explicit(r, &v, v-1, std::memory_order_release, std::memory_order_relaxed));
        return v;
    }
};

struct cmpxchg_weak_decref
{
    static int decref(std::atomic<int>* r)
    {
        // initial load only - failed CAS already writes the observed value back into v
        int v = r->load(std::memory_order_relaxed);
        // weak may fail spuriously even when *r == v; the retry loop absorbs that
        while (!std::atomic_compare_exchange_weak_explicit(r, &v, v-1, std::memory_order_release, std::memory_order_relaxed));
        return v;
    }
};

template<typename T>
using cmpxchg_strong_shared_ptr = base_shared_ptr<T, cmpxchg_strong_decref>;

template<typename T>
using cmpxchg_weak_shared_ptr = base_shared_ptr<T, cmpxchg_weak_decref>;
