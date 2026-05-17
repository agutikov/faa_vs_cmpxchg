CXX = g++
CXX_FLAGS = -std=c++17 -O2 -I./src/
LD_FLAGS = -lpthread

H_SRC := $(wildcard ./src/*.h)
APP := bench_faa_vs_cmpxchg

$(APP): ./src/main.cc $(H_SRC)
	$(CXX) $(CXX_FLAGS) $< -o $@ $(LD_FLAGS)

CSV := docs/result.csv
CHARTS_DIR := docs/charts

.PHONY: clean
clean:
	rm -f $(APP) $(CSV) $(CHARTS_DIR)/*.png

.PHONY: bench
bench: $(APP)
	./$(APP) | tee $(CSV)

$(CSV): $(APP)
	./$(APP) | tee $@


VENV := ./.venv
PYTHON := $(VENV)/bin/python3

$(PYTHON):
	./venv_install.sh

.PHONY: venv
venv: $(PYTHON)

PLOT := $(PYTHON) plot2d.py  -x '\# threads' -l bench_mode
REFCOUNT_PLOT := $(PLOT) -q 'bench_type == "refcount"'
SPINLOCK_PLOT := $(PLOT) -q 'bench_type == "spinlock"'
CHART :=

.PHONY: show
show: $(CSV) plot2d.py $(PYTHON)
	$(REFCOUNT_PLOT) $(CHART) -y clock_ns_per_iter -r 'std::shared_ptr' $<
	$(REFCOUNT_PLOT) $(CHART) -y latency_ns -r 'std::shared_ptr' $<
	#$(PLOT) -q 'bench_type == "spinlock" & bench_mode != "fetch_add"' -y clock_ns_per_iter -H $<
	#$(REFCOUNT_PLOT) $< -t 'Refcount'
	#$(SPINLOCK_PLOT) $< -t 'Spinlock'
	#$(PLOT) -q 'bench_type == "spinlock" & bench_mode != "fetch_add"' $< -t 'Spinlock'

.PHONY: png
png: $(CSV) plot2d.py $(PYTHON)
	@mkdir -p $(CHARTS_DIR)
	$(REFCOUNT_PLOT) $(CHART) -y clock_ns_per_iter $< $(CHARTS_DIR)/refcount_clock.png
	$(REFCOUNT_PLOT) $(CHART) -y latency_ns $< $(CHARTS_DIR)/refcount_time.png
	$(SPINLOCK_PLOT) $(CHART) -y clock_ns_per_iter $< $(CHARTS_DIR)/spinlock_clock.png
	$(SPINLOCK_PLOT) $(CHART) -y latency_ns $< $(CHARTS_DIR)/spinlock_time.png
	$(PLOT) $(CHART) -q 'bench_type == "spinlock" & bench_mode != "fetch_add"' -y clock_ns_per_iter $< $(CHARTS_DIR)/spinlock_clock_fast.png
	$(PLOT) $(CHART) -q 'bench_type == "spinlock" & bench_mode != "fetch_add"' -y latency_ns $< $(CHARTS_DIR)/spinlock_time_fast.png
	$(REFCOUNT_PLOT) $(CHART) -y clock_ns_per_iter -r 'std::shared_ptr' $< $(CHARTS_DIR)/refcount_clock_base.png
	$(REFCOUNT_PLOT) $(CHART) -y latency_ns -r 'std::shared_ptr' $< $(CHARTS_DIR)/refcount_time_base.png
	$(SPINLOCK_PLOT) $(CHART) -y clock_ns_per_iter -r 'std::mutex' $< $(CHARTS_DIR)/spinlock_clock_base.png
	$(SPINLOCK_PLOT) $(CHART) -y latency_ns -r 'std::mutex' $< $(CHARTS_DIR)/spinlock_time_base.png
	$(PLOT) $(CHART) -q 'bench_type == "spinlock" & bench_mode != "fetch_add"' -y clock_ns_per_iter -r 'std::mutex' $< $(CHARTS_DIR)/spinlock_clock_fast_base.png
	$(PLOT) $(CHART) -q 'bench_type == "spinlock" & bench_mode != "fetch_add"' -y latency_ns -r 'std::mutex' $< $(CHARTS_DIR)/spinlock_time_fast_base.png

