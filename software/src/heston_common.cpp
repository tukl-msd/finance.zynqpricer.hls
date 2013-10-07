//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 10. September 2013
//

#include "heston_common.hpp"

#include <cmath>


Statistics::Statistics() : mean(0), variance(0), cnt(0) {
}


Statistics::Statistics(double mean, double variance, uint64_t cnt)
		 : mean(mean), variance(variance), cnt(cnt) {
}

Statistics& Statistics::operator+=(const Statistics &rhs) {
	uint64_t new_cnt = cnt + rhs.cnt;
	double new_mean = (cnt * mean + rhs.mean * rhs.cnt) / new_cnt;
	variance = ((cnt - 1) * variance + 
			rhs.variance * (rhs.cnt - 1) + 
			cnt * rhs.cnt / new_cnt *
			std::pow(mean - rhs.mean, 2)) / (new_cnt - 1);
	mean = new_mean;
	cnt = new_cnt;
	return *this;
}

Statistics& Statistics::operator*=(const double &rhs) {
	mean *= rhs;
	variance *= rhs * rhs;
	return *this;
}

std::ostream& operator<<(std::ostream& o, const Statistics &s) {
	return o << "{\"mean\": " << s.mean << ", \"variance\": " <<
			s.variance << ", \"cnt\": " << s.cnt << "}";
}


