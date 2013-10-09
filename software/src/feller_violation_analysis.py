"""
Copyright (C) 2013 University of Kaiserslautern
Microelectronic Systems Design Research Group

Christian Brugger (brugger@eit.uni-kl.de)
10. October 2013
"""

import math
import platform
import random
import struct
import os
import sys
import json
import subprocess
import shlex

import numpy as np
import scipy.stats
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt


if platform.system() == "Windows":
    exe_path = "bin/eval_heston.exe"
    is_posix = False
elif platform.system() == "Linux":
    exe_path = "bin/eval_heston"
    is_posix = True
else:
    raise Exception("Unknown platform: " + platform.system())


if len(sys.argv) != 2:
    print("ERROR: usage: {} <result_foldername>".format(sys.argv[0]))
    sys.exit(-1)
foldername = sys.argv[1]
os.mkdir(foldername)


seed = struct.unpack('I', os.urandom(4))[0]
print("seed = {}".format(seed))
np.random.seed(seed)
with open(os.path.join(foldername, 'seed'), 'w') as f:
    f.write('seed = {}\n'.format(seed))


initial_sample_cnt = 100000
fvf_min = 0.25 # feller violation factor minimum
fvf_max = 100 # feller violation factor maximum
bin_cnt = 100


class Sample:
    def __init__(self):
        self.reversion_rate = np.random.uniform(0.1, 10.) # kappa
        self.long_term_avg_vola = np.random.uniform(0.001, 5.) # theta
        self.vol_of_vol = np.random.uniform(0.1, 10.) # sigma

    def get_feller_violation_factor(self):
        return self.vol_of_vol**2 / (2 * self.reversion_rate * 
                self.long_term_avg_vola)

while True:
    samples = [Sample() for _ in range(initial_sample_cnt)]
    samples = [sample for sample in samples 
            if fvf_min < sample.get_feller_violation_factor() < fvf_max]
    sorted_samples = sorted(samples, key=Sample.get_feller_violation_factor)

    next_bin = math.log(fvf_min)
    bin_size = (math.log(fvf_max) - math.log(fvf_min)) / bin_cnt
    binned_samples = []
    for sample in sorted_samples:
        if math.log(sample.get_feller_violation_factor()) > next_bin:
            binned_samples.append(sample)
            next_bin += bin_size
    #random.shuffle(binned_samples)
    if len(binned_samples) == bin_cnt:
        break
    else:
        print("WARNING: binned samples {} < bin_cnt {}, trying again".format(
                len(binned_samples), bin_cnt))

#TODO(brugger): asian option


if False:
    m1 = [sample.get_feller_violation_factor() for sample in binned_samples]
    plt.yscale('log')
    plt.plot(range(len(m1)), m1, '+')
    plt.show()



####

    
d_long = {}
d_short = {}

for i, sample in enumerate(binned_samples):
    p_path = os.path.join(foldername, 'params{:010d}.json'.format(i))
    
    params = {
        "heston": {
            # simply scales the result, no impact on variance decay rate
		    "spot_price": 1,
		    "reversion_rate": sample.reversion_rate,
		    "long_term_avg_vola": sample.long_term_avg_vola,
		    "vol_of_vol": sample.vol_of_vol,
		    "riskless_rate": np.random.uniform(0, 0.25),
		    "vola_0": np.random.uniform(0.0001, 10.),
		    "correlation": np.random.uniform(-1., 1.),
		    "time_to_maturity": np.random.uniform(0.1, 10.), # impact ???
		    "strike_price": 1 # impact ????
	    },
	    "barrier values": {
		    "lower": 1E-10, # should be disabled
		    "upper": 1E100 # should be disabled
	    },
	    "simulation_eval": {
		    "start_level": 4,
		    "stop_level": 7,
		    "ml_start_level": 1,
		    "ml_constant": 4,
		    "path_cnt": 100000
	    }
    }

    with open(p_path, 'w') as f:
        json.dump(params, f, indent='  ')

    cmd = "{} -ml {}".format(exe_path, p_path)

    for j in range(3):
        raw = subprocess.check_output(
                shlex.split(cmd, posix=is_posix)).decode("utf-8")
        print(raw)
        data = json.loads(raw)

        res_path = '{}.{:010d}.out'.format(p_path, j)
        with open(res_path, 'w') as f:
            json.dump(data, f, indent='  ')
        
        step_cnts = [elem['step_cnt'] for elem in data['multi-level']]
        variances = [elem['stats']['variance'] for elem in data['multi-level']]
        log_step_cnt = np.log(step_cnts)
        log_variances = np.log(variances)

        long_slope = scipy.stats.linregress(log_step_cnt,
                log_variances)[0]
        short_slope = scipy.stats.linregress(log_step_cnt[1:],
                log_variances[1:])[0]

        print(long_slope, short_slope)
        d_long.setdefault(sample.get_feller_violation_factor(), []).\
                append(long_slope)
        d_short.setdefault(sample.get_feller_violation_factor(), []).\
                append(short_slope)

        label = 'all shown = {:.3f}\nall except the first = {:.3f}'.format(
                long_slope, short_slope)
        plt.plot(step_cnts, variances, 'o-', label=label)

    plt.yscale('log')
    plt.xscale('log')
    plt.xlabel('Time Step Count')
    plt.ylabel('Multi-Level Variance')
    plt.title('Feller Condition Violation Factor = {}'.format(
            sample.get_feller_violation_factor()))
    plt.legend(prop={'size':10})
    plt.savefig(p_path + '.png')
    plt.clf()



for d, color, label in zip([d_long, d_short], 'rb', 
        ['all variances', 'all except first']):
    for k, v in d.items():
        x = [k] * len(v)
        plt.plot(x, v, color + '-o', label=label)
        label = None
plt.xscale('log')
plt.xlabel('Feller Condition Violation Factor')
plt.ylabel('Slope of Multi-Level Variance')
#plt.title('')
plt.legend(prop={'size':10})
plt.savefig(os.path.join(foldername, 'output.png'))
plt.clf()

#TODO(brugger): asian option

