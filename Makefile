CXX = g++
CXX_FLAGS = -std=c++17 -O2 -I./src/
LD_FLAGS = -lpthread

H_SRC := $(wildcard ./src/*.h)

bench_faa_vs_cmpxchg: ./src/bench.cc $(H_SRC) Makefile
	$(CXX) $(CXX_FLAGS) $< -o $@ $(LD_FLAGS)

.PHONY: clean
clean:
	rm -f bench_faa_vs_cmpxchg *.csv *.png

.PHONY: bench
bench: refcount.png spinlock.png

result.csv: bench
	./bench | tee $@

refcount.png: result.csv

spinlock.png: result.csv

