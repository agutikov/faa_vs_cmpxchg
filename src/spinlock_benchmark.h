#pragma once

#include "benchmark.h"


struct spinlock_benchmark_data_t
{
    size_t value1 = 0;
    uint8_t padding[8192];
    size_t value2 = 0;
};

template<typename spinlock_t>
void spinlock_benchmark(spinlock_t* slock, int64_t counter, size_t id, spinlock_benchmark_data_t* global_data)
{
    while (counter > 0) {
        slock->lock();

        size_t v1 = global_data->value1;
        global_data->value1 = id;

        size_t v2 = global_data->value2;
        global_data->value2 = id;

        slock->unlock();

        if (v1 != v2) {
            fprintf(stderr, "ERROR %s %d %lu != %lu\n", __func__, __LINE__, v1, v2);
            std::abort();
        }

        counter--;
    }
}

template<typename spinlock_t>
benchmark_workload_t get_spinlock_benchmark(int64_t counter)
{
    spinlock_t* slock = new spinlock_t;

    spinlock_benchmark_data_t* global_data = new spinlock_benchmark_data_t;

    return [slock, counter, global_data] () {
        std::hash<std::thread::id> hasher;
        size_t id = hasher(std::this_thread::get_id());
        spinlock_benchmark<spinlock_t>(slock, counter, id, global_data);
    };
}

