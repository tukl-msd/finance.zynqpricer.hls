//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 22. November 2013
//
// Using: Xilinx Vivado HLS 2013.2
//

#include <hls_stream.h>
#include <stdint.h>
#include <ap_axi_sdata.h>

struct params_payoff {
	// both knockout
	float log_upper_barrier_value;
	float log_lower_barrier_value;

	float barrier_correction_factor_fine;
	float barrier_correction_factor_coarse;
};

struct path {
	float stock;
	float vola;
};

float to_float(ap_axis<32,1,1,1> bus_value) {
	ap_int<32> data = bus_value.data;
	return *(reinterpret_cast<float*>(&data));
}

void set_float(ap_axis<32,1,1,1> &bus_value, float val) {
	typedef ap_int<32> data_t;
	bus_value.data = *(reinterpret_cast<data_t*>(&val));
}

path to_path(ap_axis<64,1,1,1> bus_value) {
	ap_int<64> data = bus_value.data;
	return *(reinterpret_cast<path*>(&data));
}

ap_axis<32,1,1,1> init_from(ap_axis<64,1,1,1> bus_value) {
	ap_axis<32,1,1,1> res;
	res.dest = bus_value.dest;
	res.id = bus_value.id;
	res.keep = bus_value.keep;
	res.last = bus_value.last;
	res.strb = bus_value.strb;
	res.user = bus_value.user;
	return res;
}

float get_barrier_payoff_simple(params_payoff params, path p,
		bool last, bool do_fine) {
	static bool barrier_hit = false;

	barrier_hit |= p.stock > params.log_upper_barrier_value;

	float payoff = barrier_hit ? -std::numeric_limits<float>::infinity() :
			p.stock;

	if (last) {
		barrier_hit = false;
	}
	return payoff;
}

float get_barrier_payoff_continuity_correction(params_payoff params, path p,
		bool last, bool do_fine) {
	static bool barrier_hit = false;

	float barrier_correction = p.vola *
			(do_fine ? params.barrier_correction_factor_fine :
					params.barrier_correction_factor_coarse);
	barrier_hit |= (p.stock <
			params.log_lower_barrier_value + barrier_correction) ||
			(p.stock > params.log_upper_barrier_value -
			barrier_correction);

	float payoff = barrier_hit ? -std::numeric_limits<float>::infinity() :
			p.stock;

	if (last) {
		barrier_hit = false;
	}
	return payoff;
}

void barrier_payoff(const params_payoff params,
		hls::stream<ap_axis<64,1,1,1> > &s_path,
		hls::stream<ap_axis<32,1,1,1> > &s_payoff) {
	#pragma HLS interface ap_none port=params
	#pragma HLS resource core=AXI4LiteS metadata="-bus_bundle params" \
			variable=params

	#pragma HLS interface ap_fifo port=s_path
	#pragma HLS RESOURCE variable=s_path core=AXIS metadata="-bus_bundle S_PATH"

	#pragma HLS interface ap_fifo port=s_payoff
	#pragma HLS RESOURCE variable=s_payoff core=AXIS \
			metadata="-bus_bundle S_PAYOFF"

	#pragma HLS interface ap_ctrl_none port=return

	#pragma HLS PIPELINE II=1
	ap_axis<64,1,1,1> path_data = s_path.read();
	path p = to_path(path_data);

	float payoff = get_barrier_payoff_simple(params, p, path_data.last,
			path_data.id == 0);

	ap_axis<32,1,1,1> res = init_from(path_data);
	set_float(res, payoff);
	if (path_data.last) {
		s_payoff.write(res);
	}
}
