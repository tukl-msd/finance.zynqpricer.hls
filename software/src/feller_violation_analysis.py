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

import numpy as np
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


seed = struct.unpack('I', os.urandom(4))[0]
print("seed = {}".format(seed))
np.random.seed(seed)


initial_sample_cnt = 100000
fvf_min = 0.25 # feller violation factor minimum
fvf_max = 100 # feller violation factor maximum
bin_cnt = 1000

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

spot_price = 1 # simply scales the result, no impact on variance decay rate
riskless_rate = np.random.uniform(0, 0.25, len(binned_samples)) # r
vola_0 = np.random.uniform(0.0001, 10., len(binned_samples))
correlation = np.random.uniform(-1., 1., len(binned_samples))
time_to_maturity = np.random.uniform(0.1, 10., len(binned_samples))
strike_price = 1
#TODO(brugger): asian option


if True:
    m1 = [sample.get_feller_violation_factor() for sample in binned_samples]
    plt.yscale('log')
    plt.plot(range(len(m1)), m1, '+')
    plt.show()



####

sys.exit(0)
if len(sys.argv) != 2:
    print("ERROR: usage: {} <result_foldername>".format(sys.argv[0]))
    sys.exit(-1)
foldername = sys.argv[1]
os.mkdir(foldername)

for i in range(len(binned_samples)):
    filename = os.path.join(foldername, 'params{:010i}.json'.format(i))
    print(filename)