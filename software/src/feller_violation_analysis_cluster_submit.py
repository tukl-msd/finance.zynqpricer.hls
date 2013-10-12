"""
Copyright (C) 2013 University of Kaiserslautern
Microelectronic Systems Design Research Group

Christian Brugger (brugger@eit.uni-kl.de)
10. October 2013
"""


### KEY PARAMETERS ###
bin_cnt = 1
path_cnt = 1000
regression_cnt = 1
######################

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

if platform.system() == "Windows":
    exe_path = "bin/eval_heston.exe"
    is_posix = False
elif platform.system() == "Linux":
    exe_path = "bin/eval_heston"
    is_posix = True
    matplotlib.use('Agg')
else:
    raise Exception("Unknown platform: " + platform.system())

import matplotlib.pyplot as plt

if len(sys.argv) != 2:
    print("ERROR: usage: {} <result_foldername>".format(sys.argv[0]))
    sys.exit(-1)
foldername = sys.argv[1]

class Sample:
    def __init__(self):
        self.reversion_rate = np.random.uniform(0.1, 10.) # kappa
        self.long_term_avg_vola = np.random.uniform(0.001, 5.) # theta
        self.vol_of_vol = np.random.uniform(0.1, 10.) # sigma

    def get_feller_violation_factor(self):
        return self.vol_of_vol**2 / (2 * self.reversion_rate * 
                self.long_term_avg_vola)

os.mkdir(foldername)

seed = struct.unpack('I', os.urandom(4))[0]
print("seed = {}".format(seed))
np.random.seed(seed)
with open(os.path.join(foldername, 'seed'), 'w') as f:
    f.write('seed = {}\n'.format(seed))

initial_sample_cnt = 100000
fvf_min = 0.25 # feller violation factor minimum
fvf_max = 100 # feller violation factor maximum


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
        print("WARNING: binned samples {} < bin_cnt {}, trying again"
                .format(len(binned_samples), bin_cnt))

binned_params = []
for sample in binned_samples:
    binned_params.append({
        "heston": {
            # simply scales the result, no impact on variance decay rate
            "spot_price": 1,
            "reversion_rate": sample.reversion_rate,
            "long_term_avg_vola": sample.long_term_avg_vola,
            "vol_of_vol": sample.vol_of_vol,
            "riskless_rate": np.random.uniform(0., 0.05),
            "vola_0": np.random.uniform(0.001, 10.),
            "correlation": np.random.uniform(-1., 1.),
            "time_to_maturity": 1., # impact ???
            "strike_price": 1., # impact ????
            "feller-violation-factor": sample.get_feller_violation_factor()
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
            "path_cnt": path_cnt
        }
    })

####
    
#for i, sample in enumerate(binned_samples):
for i in range(len(binned_samples)):
    sample = binned_samples[i]
    params = binned_params[i]
    p_path = os.path.join(foldername, 'params{:010d}.json'.format(i))
    
    with open(p_path, 'w') as f:
        json.dump(params, f, indent='\t')

    for j in range(regression_cnt):
        python_job = 'src/feller_violation_analysis_cluster_job.py'
        res_path = '{}.{:010d}.out'.format(p_path, j)
        cmd = ('bsub -q short -W 1:30 -n 1 -R "select[model==XEON_E5_2670]" '
                '-R "rusage[mem=2048]" python3 {} {} {}'.format(python_job,
                p_path, res_path))
        subprocess.check_call(shlex.split(cmd))

#TODO(brugger): asian option

