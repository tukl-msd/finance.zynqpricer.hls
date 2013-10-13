"""
Copyright (C) 2013 University of Kaiserslautern
Microelectronic Systems Design Research Group

Christian Brugger (brugger@eit.uni-kl.de)
10. October 2013
"""

import math
import random
import struct
import os
import sys
import json
import subprocess
import shlex
import shutil

import numpy as np
import scipy.stats
import sklearn.gaussian_process

### KEY PARAMETERS ###
regression_cnt = 2
rmse_goal = 0.05
repetition_limit = 5**2
######################


if len(sys.argv) != 3:
    print("ERROR: usage: {} <input_folder> <output_folder>".format(sys.argv[0]))
    sys.exit(-1)
input_folder = sys.argv[1]
output_folder = sys.argv[2]
os.mkdir(output_folder)

####


class Sample:
    def __init__(self, reversion_rate, long_term_avg_vola,
                 vol_of_vol):
        self.reversion_rate = reversion_rate # kappa
        self.long_term_avg_vola = long_term_avg_vola # theta
        self.vol_of_vol = vol_of_vol # sigma

    def get_feller_violation_factor(self):
        return self.vol_of_vol**2 / (2 * self.reversion_rate * 
                self.long_term_avg_vola)
    
sample_params = {}
sample_result = {}


##############################################################################

print("Reading Inputs")

for filename in sorted(os.listdir(input_folder)):
    file_path = os.path.join(input_folder, filename)
    
    if filename.endswith('.json'):
        # params0000000001.json
        i = int(filename.strip('params').strip('.json'))
        with open(file_path) as f:
            params = json.load(f)
        sample_params[i] = params
    elif filename.endswith('.out'):
        # params0000000001.json.0000000000.out
        i, j = map(int, filename.strip('params').strip('.out').split('.json.'))
        with open(file_path) as f:
            data = json.load(f)
        sample_result.setdefault(i, {})[j] = data

##############################################################################

print("Calculating new jobs")


with open(os.path.join(output_folder, 'submit.sh'), 'w') as f_sub:
    f_sub.write("set -e\n\n")
    f_sub.write("cd ../../software\n\n")

    total_job_cnt = 0

    for i, results in sample_result.items():
        d_long = {}
        d_short = {}

        for j, data in results.items():
            params = sample_params[i]
            sample = Sample(params['heston']['reversion_rate'], 
                    params['heston']['long_term_avg_vola'], 
                    params['heston']['vol_of_vol'])

            step_cnts = [elem['step_cnt'] for elem in data['multi-level']]
            variances = [elem['stats']['variance'] for elem in 
                    data['multi-level']]
            log_step_cnt = np.log(step_cnts)
            log_variances = np.log(variances)

            long_slope = scipy.stats.linregress(log_step_cnt,
                    log_variances)[0]
            short_slope = scipy.stats.linregress(log_step_cnt[1:],
                    log_variances[1:])[0]

            #print(long_slope, short_slope)
            d_long[j] = long_slope
            d_short[j] = short_slope

        params_name = 'params{:010d}.json'.format(i)
        for d in [d_long, d_short]:
            vars = list(d.values())
            mean = np.mean(vars)
            rmse = np.sqrt(np.var(vars, ddof=1))
            req_repetitions = (rmse / rmse_goal)**2
            if rmse <= rmse_goal:
                print("+ goal reached", i, rmse)
                # copy files to result folder
                shutil.copyfile(os.path.join(input_folder, params_name),
                        os.path.join(output_folder, params_name))
                for j in d:
                    res_name = '{}.{:010d}.out'.format(params_name, j)
                    shutil.copyfile(os.path.join(input_folder, res_name),
                            os.path.join(output_folder, res_name))
            elif req_repetitions <= repetition_limit:
                last_path_cnt = params["simulation_eval"]["path_cnt"]
                last_level = params["simulation_eval"]["stop_level"] + \
                        params["simulation_eval"]["ml_start_level"]
                new_path_cnt = int(math.ceil(last_path_cnt * req_repetitions))
                new_time = 4**(last_level) * 1.33 * new_path_cnt / (0.85E8)
                parallelism_real = new_time / (90 * 60 * 0.95)
                parallelism = int(math.ceil(parallelism_real))
                new_path_cnt = int(math.ceil(new_path_cnt * 
                        parallelism / parallelism_real))
                print("! repeat", i, rmse, req_repetitions, parallelism)

                params_path = os.path.join(output_folder, params_name)
                new_params = sample_params[i]
                new_params['simulation_eval']['path_cnt'] = new_path_cnt
                with open(params_path, 'w') as f:
                    json.dump(new_params, f, indent='\t')
                for j in range(regression_cnt):
                    res_path = '{}.{:010d}.out'.format(params_path, j)
                    exe_path = "zynq/software/bin/eval_heston"
                    eval_cmd = "{} -ml {}".format(exe_path, params_path)
                    submit_cmd = ('bsub -q short -a openmpi -W 1:30 -n {} '
                            '-R "select[model==XEON_E5_2670]" '
                            '-R "rusage[mem=2048]" -o {} -e {} mpirun {}'.
                            format(parallelism, res_path, res_path + '.err', 
                            eval_cmd))
                    f_sub.write(submit_cmd + "\n")
                    total_job_cnt += parallelism
            else:
                print("  rmse to high:", i, rmse, req_repetitions)
            break

    print(total_job_cnt)
    f_sub.write("# total_job_cnt: {}\n".format(total_job_cnt))


##############################################################################

#TODO(brugger): asian option
print('done')

