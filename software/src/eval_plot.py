"""
Copyright (C) 2013 University of Kaiserslautern
Microelectronic Systems Design Research Group

Christian Brugger (brugger@eit.uni-kl.de)
07. October 2013
"""

import sys
import subprocess
import json
import shlex
import platform
from math import sqrt

import numpy as np
from matplotlib import pyplot as plt



if platform.system() == "Windows":
    exe_path = "bin/eval_heston.exe"
    is_posix = False
elif platform.system() == "Linux":
    exe_path = "bin/eval_heston"
    is_posix = True
else:
    raise Exception("Unknown platform: " + platform.system())


def plot_eval(cmd, params, fmt):
    ml_start_level = params['simulation_eval']['ml_start_level']
    ml_constant = params['simulation_eval']['ml_constant']

    raw = subprocess.check_output(
            shlex.split(cmd, posix=is_posix)).decode("utf-8")
    print(raw)
    data = json.loads(raw)

    step_cnts = [elem['step_cnt'] for elem in data['multi-level']]
    variances = [elem['stats']['variance'] for elem in data['multi-level']]

    opt_n = np.array([sqrt(variances[l]/step_cnts[l])
                for l in range(len(variances))]) * \
            sum(sqrt(variances[l]*step_cnts[l]) 
                for l in range(len(variances)))

    efforts = []
    for l, time_step_cnt in enumerate(step_cnts):
        do_singlelevel = (time_step_cnt == ml_constant**ml_start_level)
        weight = 1 if do_singlelevel else (1 + 1/ml_constant)
        efforts.append(opt_n[l] * time_step_cnt * weight)
    efforts = np.array(efforts)
    efforts = efforts / np.max(efforts)

    plt.subplot(211)
    plt.yscale('log')
    plt.xscale('log')
    plt.plot(step_cnts, variances, fmt)

    plt.subplot(212)
    plt.xscale('log')
    plt.plot(step_cnts, efforts, fmt)


for _ in range(2):
    params_paths = sys.argv[1:]
    if len(params_paths) == 0:
        raise Exception("Provide params.json files as arguments.")

    colors = "bgrcmykw"
    for p_path, color in zip(params_paths, colors):
        with open(p_path, encoding='utf-8') as f:
            params = json.loads(f.read())
        plot_eval("{} -ml {}".format(exe_path, p_path), params, color + 'o-')

plt.show()
