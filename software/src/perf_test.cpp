//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 20. August 2013
//

#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <string>
#include <thread>
#include <future>

//
// timing helper functions
// 

#define time std::chrono::steady_clock::now
typedef std::chrono::duration<float> duration;
template<typename T>
float get_delta(T t1) {
	std::chrono::duration<float> delta = time() - t1;
	return delta.count();
}
template<typename T>
void print_performance(T t1, int cnt) {
	float delta = get_delta(t1);
	std::cout << cnt << " values processed in " << delta << " seconds "
			<< "(" << delta / cnt << " seconds / value)" << std::endl;
}
template<typename T>
void time_test_function_serial(std::string name, T f, 
		const std::vector<float> &data) {
	std::cout << name << ":" << std::endl;
	auto t1 = time();
	double res = f(data);
	print_performance(t1, data.size());
	std::cout << "Result: " << res << std::endl;
	std::cout << std::endl;
}

template<typename T>
void time_test_function_parallel(std::string name, T f,
		const std::vector<float> & data) {
	int nt = std::thread::hardware_concurrency();
	std::cout << name << " (" << nt << " cores):" << std::endl;
	std::vector<std::future<double> > futures;
	std::vector<std::vector<float> > local_data;
	for (int i = 0; i < nt; ++i) {
		std::vector<float> l_data;
		int block_size = data.size() / nt;
		for (int j = 0; j < block_size; ++j) {
			l_data.push_back(data[i * block_size + j]);
		}
		local_data.push_back(l_data);
	}

	auto t1 = time();
	for (int i = 0; i < nt; ++i) {
		futures.push_back(std::async(std::launch::async,
					f, local_data[i]));
	}
	std::vector<double> res;
	for (int i = 0; i < nt; ++i) {
		res.push_back(futures[i].get());
	}
	print_performance(t1, data.size());

	for (int i = 0; i < nt; ++i) {
		std::cout << "Result: " << res[i] << " (" << local_data[i].size() <<
				" values)" << std::endl;
	}
	std::cout << std::endl;
}

template<typename T>
void time_test_function(std::string name, T f, const std::vector<float> &data) {
	time_test_function_serial(name, f, data);
	time_test_function_parallel(name, f, data);
}

//
// test functions
//

std::vector<float> get_path_data(int length) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::normal_distribution<> d;

	std::vector<float> paths;
	for (int i = 0; i < length; ++i) {
		paths.push_back(d(gen));
	}
	return paths;
}


double test_simple_sum(const std::vector<float> &data) {
	float res = 0;
	for (auto val: data)
		res += val;
	return res;
}

double test_simple_sum_double(const std::vector<float> &data) {
	double res = 0;
	for (auto val: data)
		res += val;
	return res;
}

double test_exponential(const std::vector<float> &data) {
	float res = 0;
	for (auto val: data)
		res += std::exp(val);
	return res;
}

double test_exponential_double(const std::vector<float> &data) {
	double res = 0;
	for (auto val: data)
		res += std::exp((double)val);
	return res;
}

double test_simple_barrier(const std::vector<float> &data) {
	bool hit = false;
	for (auto val: data)
		hit |= (val > 5.f);
	return hit;
}

double test_call(const std::vector<float> &data) {
	float res = 0;
	for (auto val: data)
		res += std::max(0.f, val - 0.5f);
	return res;
}

double test_statistics(const std::vector<float> &data) {
	float sum = 0, prod_sum = 0;
	for (auto val: data) {
		sum += val;
		prod_sum += val * val;
	}
	return (prod_sum - sum * sum / data.size()) / data.size();
}

int main(int argc, char *argv[]) {
	const int CNT = 30000000;
	std::cout << "Memory usage: " << CNT * 4. / 1024 / 1024 << " MB" 
			<< std::endl;

	std::cout << "Initialization:" << std::endl;
	auto t1 = time();
	std::vector<float> paths = get_path_data(CNT);
	print_performance(t1, CNT);
	std::cout << std::endl;

	time_test_function("Simple Sum", test_simple_sum, paths);
	time_test_function("Simple Sum Double", test_simple_sum_double, paths);
	time_test_function("Exponential", test_exponential, paths);
	time_test_function("Exponential Double", test_exponential_double, paths);
	time_test_function("Simple Barrier", test_simple_barrier, paths);
	time_test_function("Call", test_call, paths);
	time_test_function("Statistics", test_statistics, paths);

	return 0;
}

