#pragma once

#include <string>
#include <map>
#include <functional>
#include <cstdint>
#include <thread>
#include <chrono>
#include <ctime>
#include <cstdio>
#include <tuple>
#include <list>

using benchmark_workload_t = std::function<void(void)>;

using benchmark_workload_factory_t = std::function<benchmark_workload_t(int64_t)>;

using benchmarks_table_t = std::map<
                                std::string, // bench_type
                                std::list<
                                    std::tuple<
                                        std::string, // bench_mode
                                        benchmark_workload_factory_t, // workload_factory
                                        size_t // thread number limit
                                    >
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

    double cpu_elapsed_ns = (double (std::clock() - c_started)) * 1000000000 / CLOCKS_PER_SEC;
    using dd_t = std::chrono::duration<double, std::nano>;
    double elapsed_ns = std::chrono::duration_cast<dd_t>(clock::now() - started).count();

    printf("%lu, %s, %s, %f, %.10f, %f, %f\n",
           n_threads, 
           type.c_str(),
           mode.c_str(),
           elapsed_ns,
           elapsed_ns / counter,
           cpu_elapsed_ns,
           cpu_elapsed_ns / counter);

    fflush(stdout);
}


void run_benchmarks(const benchmarks_table_t& benchmarks, int64_t counter)
{
    printf("# threads, bench_type, bench_mode, elapsed_ns, latency_ns, elapsed_clock_ns, clock_ns_per_iter\n");
    fflush(stdout);

    int ncpu = std::thread::hardware_concurrency();

    for (size_t n = 1; n <= ncpu; n++) {
        for (const auto& [bench_type, bench_modes] : benchmarks) {
            for (const auto& [bench_mode, bench_factory, threads_limit] : bench_modes) {
                if (threads_limit > 0 && n > threads_limit) {
                    continue;
                }
                run_multithread_benchmark(n, bench_type, bench_mode, counter, bench_factory(counter / n));
            }
        }
    }
}
