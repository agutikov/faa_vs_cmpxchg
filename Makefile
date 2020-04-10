CXX=g++
CXX_FLAGS= -std=c++17 -O2
LD_FLAGS= -lpthread




bench: bench_faa_vs_cmpxchg.cc Makefile
	$(CXX) $(CXX_FLAGS) $< -o $@ $(LD_FLAGS)



