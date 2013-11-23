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
import math

import numpy as np
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt



if platform.system() == "Windows":
    exe_path = "bin/eval_heston.exe"
    is_posix = False
elif platform.system() == "Linux":
    exe_path = "mpirun -n 32 bin/eval_heston"
    is_posix = True
else:
    raise Exception("Unknown platform: " + platform.system())


def plot_eval(cmd, params, index):
    colors = "bgrcmykw"
    color = colors[math.floor(index) % len(colors)]

    ml_start_level = params['simulation_eval']['ml_start_level']
    ml_constant = params['simulation_eval']['ml_constant']

    raw = subprocess.check_output(
            shlex.split(cmd, posix=is_posix)).decode("utf-8")
    print(raw)
    data = json.loads(raw)

    step_cnts = [elem['step_cnt'] for elem in data['multi-level']]
    variances = [elem['stats']['variance'] for elem in data['multi-level']]

    last_correction = abs(data['multi-level'][-1]['stats']['mean'])
    sigma_cnt = last_correction / data['multi-level'][-1]['sigma']
    if sigma_cnt < 6:
        print("WARNING: need more samples for " + cmd)
    print("INFO: need at least {} samples".format(
            6**2 * variances[-1] / last_correction**2))
    epsilon = last_correction
    last_sl_variance = data['single-level'][-1]['stats']['variance']
    opt_path_cnt_sl = 2 * last_sl_variance / epsilon**2
    sl_effort = step_cnts[-1] * opt_path_cnt_sl
    print("opt path_cnt single-level = {}, for epsilon {} ".format(
            opt_path_cnt_sl, epsilon))

    efforts = []
    for time_step_cnt, variance in zip(step_cnts, variances):
        do_singlelevel = (time_step_cnt == ml_constant**ml_start_level)
        weight = 1 if do_singlelevel else (1 + 1/ml_constant)
        opt_path_cnt = 2 / epsilon**2 * math.sqrt(variance/time_step_cnt) * \
                sum(math.sqrt(variances[l] * step_cnts[l])
                for l in range(len(variances)))
        efforts.append(opt_path_cnt * time_step_cnt * weight)
    efforts = np.array(efforts)
    efforts_normalized = efforts / np.max(efforts)
    ml_effort = sum(efforts)

    print("single-level effort: {}".format(sl_effort))
    print("multi-level effort : {}".format(ml_effort))
    print("multi-level speedup: {}".format(sl_effort / ml_effort))

    plt.subplot(221)
    plt.yscale('log')
    plt.xscale('log')
    plt.plot(step_cnts, variances, color + 'o-')

    plt.subplot(223)
    plt.xscale('log')
    plt.plot(step_cnts, efforts_normalized, color + 'o-')

    plt.subplot(224)
    last_effort = 0
    for effort in efforts:
        plt.bar(index, effort, bottom=last_effort, align='center',
                width=0.35)
        last_effort += effort



for i in range(2):
    params_paths = sys.argv[1:]
    if len(params_paths) == 0:
        raise Exception("Provide params.json files as arguments.")

    for index, p_path in enumerate(params_paths):
        with open(p_path, encoding='utf-8') as f:
            params = json.loads(f.read())
        plot_eval("{} -both {}".format(exe_path, p_path), params, index+0.5*i)

plt.show()
