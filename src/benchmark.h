#pragma once

#include <string>
#include <map>
#include <functional>
#include <cstdint>
#include <thread>
#include <chrono>
#include <ctime>
#include <cstdio>

using benchmark_workload_t = std::function<void(void)>;

using benchmark_workload_factory_t = std::function<benchmark_workload_t(int64_t)>;

using benchmarks_table_t = std::map<
                               std::string,
                               std::map<
                                   std::string,
                                   benchmark_workload_factory_t
                               >
                           >;

void run_multithread_benchmark(
                        size_t n_threads,
                        const std::string& type,
                        const std::string& mode,
                        int64_t counter,
                        benchmark_workload_t work
                    )
{
    using clock = std::chrono::steady_clock;

    std::vector<std::thread> threads;
    threads.reserve(n_threads);

    clock::time_point started = clock::now();
    std::clock_t c_started = std::clock();

    for (size_t i = 0; i < n_threads; i++) {
        threads.push_back(std::thread(work));
    }
    for (auto& thread : threads) {
        thread.join();
    }

    std::clock_t c_elapsed = std::clock() - c_started;
    using dd_t = std::chrono::duration<double, std::nano>;
    double elapsed_ns = std::chrono::duration_cast<dd_t>(clock::now() - started).count();

    printf("%lu, %s, %s, %.10f, %lu\n",
           n_threads, 
           type.c_str(),
           mode.c_str(),
           elapsed_ns / counter,
           c_elapsed * 1000 / counter);

    fflush(stdout);
}


void run_benchmarks(const benchmarks_table_t& benchmarks, int64_t counter)
{
    printf("n_threads, bench_type, bench_mode, elapsed time (ns) / iteration, clocks * 1000 / iteration\n");
    fflush(stdout);

    int ncpu = std::thread::hardware_concurrency();

    for (size_t n = 1; n <= ncpu; n++) {
        for (const auto& [bench_type, bench_modes] : benchmarks) {
            for (const auto& [bench_mode, bench_factory] : bench_modes) {
                run_multithread_benchmark(n, bench_type, bench_mode, counter, bench_factory(counter / n));
            }
        }
    }
}
