# fetch_add vs. compare_exchange

Benchmark C++ standard atomic operations `fetch_add`, `fetch_or`, and `compare_exchange`.

Two types of benchmarks:
- *refcount* — aggressive concurrent usage of `shared_ptr`, implemented with:
  - `std::atomic_compare_exchange_weak()`
  - `std::atomic_compare_exchange_strong()`
  - `std::atomic_fetch_sub()`
  - baseline: `std::shared_ptr`
- *spinlock* — aggressive concurrent usage of a spinlock, implemented with:
  - `std::atomic_compare_exchange_weak()`
  - `std::atomic_compare_exchange_weak()` + `pause`
  - `std::atomic_compare_exchange_strong()`
  - `std::atomic_compare_exchange_strong()` + `pause`
  - `std::atomic_fetch_add()`
  - `std::atomic_fetch_or()` + `pause`
  - baseline: `std::mutex`



Build:
```
make
```

Run benchmark (produces `docs/result.csv`):
```sh
make bench
```

Set up the Python venv for plotting (one-time):
```sh
./venv_install.sh
```

Show charts on screen:
```sh
make show
```

Render charts to PNG files:
```sh
make png
```

See [docs/article.md](docs/article.md) for the full write-up and analysis.

