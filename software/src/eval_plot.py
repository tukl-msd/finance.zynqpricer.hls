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

import numpy as np
from matplotlib import pyplot as plt

def plot_eval(cmd, fmt):
    raw = subprocess.check_output(shlex.split(cmd)).decode("utf-8")
    print(raw)
    data = json.loads(raw)

    step_cnt = [elem['step_cnt'] for elem in data['multi-level']]
    variance = [elem['stats']['variance'] for elem in data['multi-level']]

    effort = []
    for cnt, var in zip(step_cnt, variance):
        effort.append(cnt * var) #TODO(brugger): weighting factor for ml levels
    effort = np.array(effort)
    effort = effort / np.max(effort)

    plt.subplot(211)
    plt.yscale('log')
    plt.xscale('log')
    plt.plot(step_cnt, variance, fmt)

    plt.subplot(212)
    plt.xscale('log')
    plt.plot(step_cnt, effort, fmt)# range(len(step_cnt)))


for _ in range(2):
    plot_eval("bin/eval_heston.exe -ml parameters/p1.json", 'bo-')
    plot_eval("bin/eval_heston.exe -ml parameters/p2.json", 'go-')

plt.show()
