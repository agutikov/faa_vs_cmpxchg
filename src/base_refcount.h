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

        void inc()
        {
            refcount++;
        }

        bool dec()
        {
            return D::decref(&refcount) == 1;
        }
    };

    shared_ptr_block* block = nullptr;

    void reset()
    {
        if (block) {
            if (block->dec()) {
                delete block;
            }
        }
        block = nullptr;
    }

    base_shared_ptr(T* v)
    {
        block = new shared_ptr_block(v);
    }

    base_shared_ptr(const base_shared_ptr<T, D>& r)
    {
        block = r.block;
        block->inc();
    }

    base_shared_ptr(base_shared_ptr<T, D>&& r)
    {
        block = r.block;
        block->inc();
        r.reset();
    }

    ~base_shared_ptr()
    {
        reset();
    }

    base_shared_ptr<T, D>& operator =(const base_shared_ptr<T, D>& r)
    {
        reset();
        block = r.block;
        block->inc();
        return *this;
    }

    base_shared_ptr<T, D>& operator =(base_shared_ptr<T, D>&& r)
    {
        reset();
        block = r.block;
        block->inc();
        r.reset();
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

