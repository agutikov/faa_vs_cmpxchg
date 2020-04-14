# fetch_sub/fetch_add vs. compare_exchange

Benchmark C++ standard atomic operations fetch_sub/fetch_add vs. compare_exchange.

Two types of bencmarks:
- *refcount* - aggressive concurrent usage of shared_ptr, implemented with:
  - `std::atomic::compare_exchange_weak()`
  - `std::atomic::compare_exchange_strong()`
  - `std::atomic_fetch_sub()`
  - with compare to `std::shared_ptr`
- *spinlock* - aggressive concurrent usage of spinlock, implemented with:
  - `std::atomic::compare_exchange_weak()`
  - `std::atomic::compare_exchange_strong()`
  - `std::atomic_fetch_add()`
  - with compare to `std::mutex`



Build:  
`$ make`

Run benchmark:  
`$ make bench`

Show results:  
`$ make draw`

