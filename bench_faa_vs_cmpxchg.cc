
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
    //std::atomic<int> locked = 0;
    void lock()
    {

    }
    void unlock()
    {

    }
};

struct cmpxchg_spinlock
{
    
    void lock()
    {

    }
    void unlock()
    {
        
    }
};


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
    static std::string name()
    {
        return Decref::name();
    }
};

struct faa_decref
{
    bool operator() (std::atomic<int>* r)
    {
        return std::atomic_fetch_sub(r, 1) == 1;
    }
    static std::string name() {
        return "fetch_sub";
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
    static std::string name() {
        return "cmpxchg_strong";
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
    static std::string name() {
        return "cmpxchg_weak";
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
void bench_shared_ptr_thread_workload(shared_ptr_t<int> p, int64_t counter)
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
std::function<void(void)> get_shared_ptr_thread_workload(int64_t counter)
{
    shared_ptr_t<int> p(new int(1));

    return [p, counter] () {
        bench_shared_ptr_thread_workload<shared_ptr_t>(p, counter);
    };
}

template< template<typename T> typename shared_ptr_t >
void bench_shared_ptr(size_t n)
{
    using clock = std::chrono::steady_clock;

    std::vector<std::thread> threads;
    threads.reserve(n);

    int64_t counter = 1024*1024*16 / n;

    auto workload = get_shared_ptr_thread_workload<shared_ptr_t>(counter);

    clock::time_point started = clock::now();
    std::clock_t c_started = std::clock();

    for (size_t i = 0; i < n; i++) {
        threads.push_back(std::thread(workload));
    }
    for (auto& thread : threads) {
        thread.join();
    }

    std::clock_t c_elapsed = std::clock() - c_started;
    using dd_t = std::chrono::duration<double>;
    double elapsed = std::chrono::duration_cast<dd_t>(clock::now() - started).count();

    printf("%lu, shared_ptr, %s, %f, %lu\n", n, shared_ptr_t<int>::name().c_str(), elapsed, c_elapsed);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int main(int argc, const char* argv[])
{
    printf("n_threads, bench_type, bench_mode, elapsed_real_time_s, elapsed_clocks\n");
    size_t n = 1;
    for (size_t i = 0; i < 5; i++) {
        bench_shared_ptr<faa_shared_ptr>(n);
        bench_shared_ptr<cmpxchg_strong_shared_ptr>(n);
        bench_shared_ptr<cmpxchg_weak_shared_ptr>(n);
        n *= 2;
    }




    return 0;
}
























