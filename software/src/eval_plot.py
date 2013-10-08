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

import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt

def plot_eval(cmd, fmt):
    raw = subprocess.check_output(shlex.split(cmd)).decode("utf-8")
    print(raw)
    data = json.loads(raw)

    X = [elem['step_cnt'] for elem in data['multi-level']]

    Y = [elem['stats']['variance'] for elem in data['multi-level']]
    Y_err = [elem['var_sigma'] for elem in data['multi-level']]

    plt.errorbar(X, Y, yerr=Y_err, fmt=fmt)


plt.yscale('log')
plt.xscale('log')

for _ in range(3):
    plot_eval("bin/eval_heston.exe -ml parameters/p1.json", 'bo-')
    plot_eval("bin/eval_heston.exe -ml parameters/p2.json", 'go-')

plt.show()
