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
bench: result.csv

result.csv: $(APP)
	./$(APP) | tee $@


PLOT := python3 plot2d.py  -x '\# threads' -l bench_mode
REFCOUNT_PLOT := $(PLOT) -q 'bench_type == "refcount"' 
SPINLOCK_PLOT := $(PLOT) -q 'bench_type == "spinlock"'
CHART := -B

.PHONY: show
show: result.csv plot2d.py
	$(REFCOUNT_PLOT) $(CHART) -y clock_ns_per_iter -r 'std::shared_ptr' $<
	$(REFCOUNT_PLOT) $(CHART) -y latency_ns -r 'std::shared_ptr' $<
	#$(PLOT) -q 'bench_type == "spinlock" & bench_mode != "fetch_add"' -y clock_ns_per_iter -H $<
	#$(REFCOUNT_PLOT) $< -t 'Refcount'
	#$(SPINLOCK_PLOT) $< -t 'Spinlock'
	#$(PLOT) -q 'bench_type == "spinlock" & bench_mode != "fetch_add"' $< -t 'Spinlock'

.PHONY: png
png: result.csv plot2d.py
	$(REFCOUNT_PLOT) $(CHART) -y clock_ns_per_iter $< refcount_clock.png
	$(REFCOUNT_PLOT) $(CHART) -y latency_ns $< refcount_time.png
	$(SPINLOCK_PLOT) $(CHART) -y clock_ns_per_iter $< spinlock_clock.png
	$(SPINLOCK_PLOT) $(CHART) -y latency_ns $< spinlock_time.png
	$(PLOT) $(CHART) -q 'bench_type == "spinlock" & bench_mode != "fetch_add"' -y clock_ns_per_iter $< spinlock_clock_fast.png
	$(PLOT) $(CHART) -q 'bench_type == "spinlock" & bench_mode != "fetch_add"' -y latency_ns $< spinlock_time_fast.png
	$(REFCOUNT_PLOT) $(CHART) -y clock_ns_per_iter -r 'std::shared_ptr' $< refcount_clock_base.png
	$(REFCOUNT_PLOT) $(CHART) -y latency_ns -r 'std::shared_ptr' $< refcount_time_base.png
	$(SPINLOCK_PLOT) $(CHART) -y clock_ns_per_iter -r 'std::mutex' $< spinlock_clock_base.png
	$(SPINLOCK_PLOT) $(CHART) -y latency_ns -r 'std::mutex' $< spinlock_time_base.png
	$(PLOT) $(CHART) -q 'bench_type == "spinlock" & bench_mode != "fetch_add"' -y clock_ns_per_iter -r 'std::mutex' $< spinlock_clock_fast_base.png
	$(PLOT) $(CHART) -q 'bench_type == "spinlock" & bench_mode != "fetch_add"' -y latency_ns -r 'std::mutex' $< spinlock_time_fast_base.png

