FLAGS = -std=c++11 -Ofast -march=native

all: init_rng heston_sl

init_rng: init_rng.o mt19937ar.o
	g++ $(FLAGS) $^ -o $@

heston_sl: heston_sl.cpp
	g++ $(FLAGS) $^ -o $@ -lpthread
	
%.o: %.cpp
	g++ $(FLAGS) -c $< -o $@
