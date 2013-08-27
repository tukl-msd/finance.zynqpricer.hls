FLAGS = -I . -std=c++11 -Ofast -march=native

all: init_rng run_both

init_rng: init_rng.o mt19937ar.o
	g++ $(FLAGS) $^ -o $@

run_cpu: run_cpu.cpp heston_sl_cpu.o jsoncpp.o mt19937ar.o
	g++ $(FLAGS) $^ -o $@ -lpthread

run_acc: run_acc.cpp heston_sl_acc.o jsoncpp.o 
	g++ $(FLAGS) $^ -o $@ -lpthread

run_both: run_both.cpp heston_sl_acc.o heston_sl_cpu.o jsoncpp.o mt19937ar.o
	g++ $(FLAGS) $^ -o $@ -lpthread

read_fifo: read_fifo.cpp
	g++ $(FLAGS) $^ -o $@
	
read_rng: read_rng.o mt19937ar.o
	g++ $(FLAGS) $^ -o $@

%.o: %.cpp
	g++ $(FLAGS) -c $< -o $@
