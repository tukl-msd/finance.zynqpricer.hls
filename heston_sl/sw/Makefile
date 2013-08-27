FLAGS = -std=c++11 -Ofast -march=native -I .

all: init_rng heston_sl

init_rng: init_rng.o mt19937ar.o
	g++ $(FLAGS) $^ -o $@

heston_sl: heston_sl.cpp iodev.hpp jsoncpp.o
	g++ $(FLAGS) $^ -o $@ -lpthread

read_fifo: read_fifo.cpp
	g++ $(FLAGS) $^ -o $@
	
read_rng: read_rng.o mt19937ar.o
	g++ $(FLAGS) $^ -o $@

%.o: %.cpp
	g++ $(FLAGS) -c $< -o $@
