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

import numpy as np
import scipy.stats
import sklearn.gaussian_process
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages


if len(sys.argv) != 2:
    print("ERROR: usage: {} <result_foldername>".format(sys.argv[0]))
    sys.exit(-1)
foldername = sys.argv[1]

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
    
d_long = {}
d_short = {}

sample_params = {}
sample_result = {}


##############################################################################

print("Reading Inputs")

for filename in sorted(os.listdir(foldername)):
    file_path = os.path.join(foldername, filename)
    if not filename.startswith('params'):
        continue

    if filename.endswith('.json'):
        # params0000000001.json
        i = int(filename.strip('params').strip('.json'))
        with open(file_path) as f:
            params = json.load(f)
        sample_params[i] = params
    elif filename.endswith('.out'):
        # params0000000001.json.0000000000.out
        i, j = map(int, filename.strip('params').strip('.out').split('.json.'))
        try:
            with open(file_path) as f:
                data = json.load(f)
        except ValueError:
            print("Ignoring file: ", filename)
        else:
            sample_result.setdefault(i, {})[j] = data

##############################################################################

print("Generating individual graphs")

for i, results in sample_result.items():
    p_path = os.path.join(foldername, 'params{:010d}.json'.format(i))
    png_path = p_path + '.png'
    do_gen_png = not os.path.isfile(png_path)
    
    for j, data in results.items():
        params = sample_params[i]
        sample = Sample(params['heston']['reversion_rate'], 
                params['heston']['long_term_avg_vola'], 
                params['heston']['vol_of_vol'])

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

        if do_gen_png:
            label = 'all shown = {:.3f}\nall except the first = {:.3f}'.format(
                    long_slope, short_slope)
            plt.plot(step_cnts, variances, 'o-', label=label)

    if do_gen_png:
        plt.yscale('log')
        plt.xscale('log')
        plt.xlabel('Time Step Count')
        plt.ylabel('Multi-Level Variance')
        plt.title('Feller Condition Violation Factor = {}'.format(
                sample.get_feller_violation_factor()))
        plt.legend(prop={'size':10})
        plt.savefig(png_path)
        plt.clf()

##############################################################################

print("writing threshold plots")

data = {}

for d, title_info in zip([d_long, d_short],
        ['all_variances', 'all_except_first']):
    pp = PdfPages(os.path.join(foldername, 
            'output_prediction_{}.pdf'.format(title_info)))
    data[title_info] = {}
    for cutoff in np.arange(0., 1., 0.05):

        print("Gaussian Fit:", title_info)

        X_list = []
        Y = []
        Y_err = []

        label = 'Observations'
        label_err = 'Mean and Variance of Observations'
        for k, v in d.items():
            x = [k] * len(v)
            mean = np.mean(v)
            rmse = np.sqrt(np.var(v, ddof=1))
            if rmse > cutoff > 0:
                continue

            X_list.append(k)
            Y.append(mean)
            Y_err.append(rmse)

        
            plt.errorbar(k, mean, rmse, fmt='ro', label=label_err)
            plt.plot(x, v, 'rx', label=label)
            label = None
            label_err = None

        #####

        data[title_info][str(cutoff)] = {'X': X_list, 'Y': Y, 'Y_err': Y_err}

        do_prediction = False

        if do_prediction:
            X = np.array([X_list]).T

            # Instanciate a Gaussian Process model
            gp = sklearn.gaussian_process.GaussianProcess(
                    corr='squared_exponential', 
                    theta0=1e-1, thetaL=1e-3, thetaU=1,
                    #nugget=(dy / y) ** 2,
                    nugget = (np.array(Y_err) / np.array(Y))**2, 
                    random_start=100)

            # Fit to data using Maximum Likelihood Estimation of the parameters
            gp.fit(X, Y)

            # Mesh the input space for evaluations of the real function, the 
            # prediction and its MSE
            x = np.atleast_2d(np.linspace(0.25, 100, 1000)).T

            # Make the prediction on the meshed x-axis (ask for MSE as well)
            y_pred, MSE = gp.predict(x, eval_MSE=True)
            sigma = np.sqrt(MSE)

            plt.plot(x, y_pred, 'b-', label=u'Prediction (Gaussian process)')
            plt.fill(np.concatenate([x, x[::-1]]),
                    np.concatenate([y_pred - 1.9600 * sigma,
                                   (y_pred + 1.9600 * sigma)[::-1]]),
                    alpha=.5, fc='b', ec='None', 
                    label='95% confidence interval')
        plt.xscale('log')
        plt.xlabel('Feller Condition Violation Factor')
        plt.ylabel('Slope of Multi-Level Variance')
        plt.xlim(0.25, 100)
        plt.ylim(-1.5, 0.5)

        #plt.plot([1, 1], [-4, 4], 'k-', lw=1)
        plt.fill_between([0.25, 1], [4, 4], -4, facecolor='green', alpha=0.1)
        plt.fill_between([1, 100], [4, 4], -4, facecolor='red', alpha=0.1)

        plt.grid(True)
        plt.legend(loc=4, fontsize='small', numpoints=1)
        plt.title('Feller Violation Analysis')
        #plt.savefig(os.path.join(foldername, 
        #        'output_prediction_{}_{:.2f}.pdf'.format(title_info, cutoff)))
        plt.savefig(pp, format='pdf')
        #plt.show()
        plt.clf()

    pp.savefig()
    pp.close()

with open(os.path.join(foldername, 
             'output_prediction_data.json'.format(title_info)), 'w') as f:
    json.dump(data, f, indent='\t')

#TODO(brugger): asian option
print('done')

