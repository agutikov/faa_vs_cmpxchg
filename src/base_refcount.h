#pragma once

#include "utils.h"
#include <atomic>



template<typename T, typename D>
struct base_shared_ptr
{
    struct shared_ptr_block
    {
        T* value;
        std::atomic<int> refcount;

        shared_ptr_block(T* v)
        : value(v), refcount(1)
        {}

        ~shared_ptr_block()
        {
            delete value;
            value = nullptr;
        }

        inline void acquire()
        {
            // relaxed: refcount increment doesn't need to synchronize anything
            refcount.fetch_add(1, std::memory_order_relaxed);
        }

        inline bool release()
        {
            // returns true on the final decrement (pre-decrement value == 1)
            return D::decref(&refcount) == 1;
        }
    };

    shared_ptr_block* block = nullptr;

    void reset()
    {
        if (block != nullptr) {
            if (block->release()) {
                // acquire fence pairs with the release-decrements from all other threads,
                // ensuring their writes happen-before ~shared_ptr_block runs
                std::atomic_thread_fence(std::memory_order_acquire);
                delete block;
            }
            block = nullptr;
        }
    }

    base_shared_ptr(T* v)
    {
        block = new shared_ptr_block(v);
    }

    base_shared_ptr(const base_shared_ptr<T, D>& r)
    {
        // copy: share the block, bump the refcount
        block = r.block;
        block->acquire();
    }

    base_shared_ptr(base_shared_ptr<T, D>&& r)
    {
        // move: steal the block - no refcount change needed
        block = r.block;
        r.block = nullptr;
    }

    ~base_shared_ptr()
    {
        reset();
    }

    base_shared_ptr<T, D>& operator =(const base_shared_ptr<T, D>& r)
    {
        reset();
        block = r.block;
        block->acquire();
        return *this;
    }

    base_shared_ptr<T, D>& operator =(base_shared_ptr<T, D>&& r)
    {
        // move-assign: drop ours, steal theirs - again no refcount change for the steal
        reset();
        block = r.block;
        r.block = nullptr;
        return *this;
    }

    operator bool() const
    {
        return block != nullptr;
    }

    T& operator *() const
    {
        return *block->value;
    }
};




template< template<typename T> typename shared_ptr_t >
void dump(const shared_ptr_t<int>& p)
{
    if (p.block) {
        tprintf("%p->{%p->{%d}, %d}\n", p.block, p.block->value, *p.block->value, p.block->refcount.load());
    } else {
        tprintf("%p\n", p.block);
    }
}

