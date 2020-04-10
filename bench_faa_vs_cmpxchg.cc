
#include <atomic>
#include <cstdio>
#include <cstring>
#include <string>
#include <charconv>
#include <set>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <thread>
#include <chrono>
#include <sstream>
#include <cstdarg>
#include <ctime>
#include <memory>
#include <mutex>
#include <map>


template<typename T>
std::string to_string(const T& v)
{
    std::ostringstream ss;
    ss << v;
    return ss.str();
}

void tprintf(const char* fmt, ...)
{
    auto tid = to_string(std::this_thread::get_id());
    printf("%s ", tid.c_str());

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


struct faa_spinlock
{
    std::atomic<int> locked = 0;

    void lock()
    {
        while (std::atomic_fetch_add(&locked, 1) != 0) {
            locked--;
        }
    }

    void unlock()
    {
        locked--;
    }
};


struct cmpxchg_spinlock_base
{
    std::atomic<int> locked = 0;

    void unlock()
    {
        locked = 0;
    }
};


struct cmpxchg_spinlock_weak : cmpxchg_spinlock_base
{
    void lock()
    {
        int v;
        for (;;) {
            v = locked.load();
            if (v == 0) {
                if (std::atomic_compare_exchange_weak(&locked, &v, 1)) {
                    break;
                }
            }
        };
    }
};


struct cmpxchg_spinlock_strong : cmpxchg_spinlock_base
{
    void lock()
    {
        int v;
        for (;;) {
            v = locked.load();
            if (v == 0) {
                if (std::atomic_compare_exchange_strong(&locked, &v, 1)) {
                    break;
                }
            }
        };
    }
};


size_t global_value1;
uint8_t padding[8192];
size_t global_value2;


template<typename spinlock_t>
void bench_spinlock_bench(spinlock_t* slock, int64_t counter, size_t id)
{
    while (counter > 0) {
        slock->lock();

        size_t v1 = global_value1;
        global_value1 = id;

        size_t v2 = global_value2;
        global_value2 = id;

        slock->unlock();

        if (v1 != v2) {
            fprintf(stderr, "ERROR %s %d %lu != %lu\n", __func__, __LINE__, v1, v2);
            std::abort();
        }

        counter--;
    }
}


template<typename spinlock_t>
std::function<void(void)> get_spinlock_bench(int64_t counter)
{
    spinlock_t* slock = new spinlock_t;

    global_value1 = 0;
    global_value2 = 0;

    return [slock, counter] () {
        std::hash<std::thread::id> hasher;
        size_t id = hasher(std::this_thread::get_id());
        bench_spinlock_bench<spinlock_t>(slock, counter, id);
    };
}

// Just creates function pointer to intantiated template function
template<typename spinlock_t>
std::function<std::function<void(void)>(int64_t)> get_spinlock_bench_factory()
{
    return [](int64_t counter)
    {
        return get_spinlock_bench<spinlock_t>(counter);
    };
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


template<typename T, typename Decref>
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
            return Decref()(&refcount);
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

    base_shared_ptr(const base_shared_ptr<T, Decref>& r)
    {
        block = r.block;
        block->inc();
    }

    base_shared_ptr(base_shared_ptr<T, Decref>&& r)
    {
        block = r.block;
        block->inc();
        r.reset();
    }

    ~base_shared_ptr()
    {
        reset();
    }

    base_shared_ptr<T, Decref>& operator =(const base_shared_ptr<T, Decref>& r)
    {
        reset();
        block = r.block;
        block->inc();
        return *this;
    }

    base_shared_ptr<T, Decref>& operator =(base_shared_ptr<T, Decref>&& r)
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


struct faa_decref
{
    bool operator() (std::atomic<int>* r)
    {
        return std::atomic_fetch_sub(r, 1) == 1;
    }
};
template<typename T>
using faa_shared_ptr = base_shared_ptr<T, faa_decref>;


struct cmpxchg_strong_decref
{
    bool operator() (std::atomic<int>* r)
    {
        int v;
        do {
            v = r->load();
        } while (!std::atomic_compare_exchange_strong(r, &v, v-1));
        return v == 1;
    }
};
template<typename T>
using cmpxchg_strong_shared_ptr = base_shared_ptr<T, cmpxchg_strong_decref>;


struct cmpxchg_weak_decref
{
    bool operator() (std::atomic<int>* r)
    {
        int v;
        do {
            v = r->load();
        } while (!std::atomic_compare_exchange_weak(r, &v, v-1));
        return v == 1;
    }
};
template<typename T>
using cmpxchg_weak_shared_ptr = base_shared_ptr<T, cmpxchg_weak_decref>;


template< template<typename T> typename shared_ptr_t >
void dump(const shared_ptr_t<int>& p)
{
    if (p.block) {
        tprintf("%p->{%p->{%d}, %d}\n", p.block, p.block->value, *p.block->value, p.block->refcount.load());
    } else {
        tprintf("%p\n", p.block);
    }
}


template< template<typename T> typename shared_ptr_t >
void bench_shared_ptr_bench(shared_ptr_t<int> p, int64_t counter)
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
std::function<void(void)> get_shared_ptr_bench(int64_t counter)
{
    shared_ptr_t<int> p(new int(1));

    return [p, counter] () {
        bench_shared_ptr_bench<shared_ptr_t>(p, counter);
    };
}

// Just creates function pointer to intantiated template function
template< template<typename T> typename shared_ptr_t >
std::function<std::function<void(void)>(int64_t)> get_shared_ptr_bench_factory()
{
    return [](int64_t counter)
    {
        return get_shared_ptr_bench<shared_ptr_t>(counter);
    };
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void bench_multithread(
                        size_t n_threads,
                        const std::string& type,
                        const std::string& mode,
                        size_t counter,
                        std::function<void(void)> work
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
    using dd_t = std::chrono::duration<double>;
    double elapsed = std::chrono::duration_cast<dd_t>(clock::now() - started).count();

    printf("%lu, %s, %s, %f, %lu, %.10f, %lu\n", n_threads, type.c_str(), mode.c_str(), elapsed, c_elapsed, elapsed / counter, c_elapsed * 1000 / counter);
}

std::map<
    std::string,
    std::map<
        std::string,
        std::function<std::function<void(void)>(int64_t)>
    >
> benchmarks {
    {"refcount", {
        {"faa", get_shared_ptr_bench_factory<faa_shared_ptr>()},
        {"cmpxchg_strong", get_shared_ptr_bench_factory<cmpxchg_strong_shared_ptr>()},
        {"cmpxchg_weak", get_shared_ptr_bench_factory<cmpxchg_weak_shared_ptr>()},
        {"std::shared_ptr", get_shared_ptr_bench_factory<std::shared_ptr>()}
    }},
    {"spinlock", {
        {"faa", get_spinlock_bench_factory<faa_spinlock>()},
        {"cmpxchg_strong", get_spinlock_bench_factory<cmpxchg_spinlock_strong>()},
        {"cmpxchg_weak", get_spinlock_bench_factory<cmpxchg_spinlock_weak>()},
        {"std::mutex", get_spinlock_bench_factory<std::mutex>()}
    }},
};

// Can't beleave this is C++!

int main(int argc, const char* argv[])
{
    size_t counter = 1024*1024*32;
    if (argc == 2) {
        counter = strtol(argv[1], 0, 0);
    }

    printf("n_threads, bench_type, bench_mode, elapsed_real_time_s, elapsed_clocks, s/iteration, clocks*1000/iteration\n");

    int ncpu = std::thread::hardware_concurrency();

    for (size_t n = 1; n <= ncpu; n *= 2) {
        for (const auto& [bench_type, bench_modes] : benchmarks) {
            for (const auto& [bench_mode, bench_factory] : bench_modes) {
                bench_multithread(n, bench_type, bench_mode, counter, bench_factory(counter / n));
            }
        }
    }

    return 0;
}
























