FLAGS = -I. -std=c++11 -O3 -march=native -ffast-math -mfpu=neon
# -ftree-vectorizer-verbose=1 

all: init_rng run_cpu run_acc run_both read_fifo read_rng

init_rng: init_rng.o mt19937ar.o jsoncpp.o iodev.hpp
	g++ $(FLAGS) $^ -o $@

run_cpu: run_cpu.cpp heston_sl_cpu.o heston_sl_cpu.hpp jsoncpp.o mt19937ar.o runner.hpp
	g++ $(FLAGS) $^ -o $@ -lpthread

run_acc: run_acc.cpp heston_sl_acc.o heston_sl_acc.hpp jsoncpp.o iodev.hpp runner.hpp
	g++ $(FLAGS) $^ -o $@ -lpthread

run_both: run_both.cpp heston_sl_acc.o heston_sl_acc.hpp heston_sl_cpu.o heston_sl_cpu.hpp jsoncpp.o mt19937ar.o iodev.hpp runner.hpp
	g++ $(FLAGS) $^ -o $@ -lpthread

read_fifo: read_fifo.cpp iodev.hpp
	g++ $(FLAGS) $^ -o $@
	
read_rng: read_rng.o mt19937ar.o iodev.hpp
	g++ $(FLAGS) $^ -o $@

%.o: %.cpp
	g++ $(FLAGS) -c $< -o $@
