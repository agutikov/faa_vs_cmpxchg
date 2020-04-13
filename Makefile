CXX = g++
CXX_FLAGS = -std=c++17 -O2 -I./src/
LD_FLAGS = -lpthread

H_SRC := $(wildcard ./src/*.h)
APP := bench_faa_vs_cmpxchg

$(APP): ./src/main.cc $(H_SRC)
	$(CXX) $(CXX_FLAGS) $< -o $@ $(LD_FLAGS)

.PHONY: clean
clean:
	rm -f $(APP) *.csv *.png

.PHONY: bench
bench: refcount.png spinlock.png

result.csv: $(APP)
	./$(APP) | tee $@

refcount.png: result.csv plot2d.py
	python3 plot2d.py --select 'bench_type == "refcount"' --x-axis n_threads --labels bench_mode $< $@ --title 'Refcount'

spinlock.png: result.csv plot2d.py
	python3 plot2d.py --select 'bench_type == "spinlock"' --x-axis n_threads --labels bench_mode $< $@ --title 'Spinlock'

