
#include <hls_stream.h>
#include <hls_math.h>
#include <ap_fixed.h>

float max_0(float x) {
	ap_uint<32> x_bits = *reinterpret_cast<ap_uint<32>*>(&x);
	if (x_bits[31] == 1)
		return 0;
	else
		return x;
}

void pricing(hls::stream<float> in, hls::stream<float> out, hls::stream<float> out2, float strike_price) {
	//#pragma HLS PIPELINE II=1

	const int BLOCK = 64;
	static ap_uint<32> res_cnt[BLOCK];
	static float res_sum[BLOCK];
	static float res_prod[BLOCK];

	for (int i = 0; i < BLOCK; ++i) {
		#pragma HLS PIPELINE II=1
		float path = in.read();
		float res = max_0(hls::expf(path) - strike_price);


		ap_uint<32> l_cnt = res_cnt[i];
		float l_sum = res_sum[i];
		float l_prod = res_prod[i];

		ap_uint<32> n_cnt = l_cnt + 1;
		float delta = res - l_sum;
		float n_sum = l_sum + delta / n_cnt;
		float n_prod = l_prod + delta * (res - n_sum);

		res_cnt[i] = n_cnt;
		res_sum[i] = n_sum;
		res_prod[i] = n_prod;


		out.write(res_sum[i]);
		out2.write(res_prod[i]);
	}

	//std::max(0.f, hls::expf(path) - strike_price);
}
