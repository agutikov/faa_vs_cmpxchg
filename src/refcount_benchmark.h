#pragma once

#include "benchmark.h"


template< template<typename T> typename shared_ptr_t >
void shared_ptr_benchmark(shared_ptr_t<int> p, int64_t counter)
{
    while (counter > 0) {
        shared_ptr_t<int> p1(p);
        shared_ptr_t<int> p2(std::move(p1));
        if (!p1 && p2) {
            counter -= *p2;
        } else {
            fprintf(stderr, "ERROR %s %d\n", __func__, __LINE__);
            std::abort();
        }
    }
}

template< template<typename T> typename shared_ptr_t >
benchmark_workload_t get_shared_ptr_benchmark(int64_t counter)
{
    shared_ptr_t<int> p(new int(1));

    return [p, counter] () {
        shared_ptr_benchmark<shared_ptr_t>(p, counter);
    };
}
