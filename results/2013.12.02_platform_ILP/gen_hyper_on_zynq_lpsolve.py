"""
Copyright (C) 2013 University of Kaiserslautern
Microelectronic Systems Design Research Group

Christian Brugger (brugger@eit.uni-kl.de)
23. November 2013
"""

import copy

from jinja2 import Environment
import pandas as pd
import numpy as np

FILE = "hyper_on_zynq_{device}_{level}.lp"


class FPGAResource(pd.Series):
    """ bram - 18k BRAM """
    def __new__(cls, lut, ff, bram, dsp):
        return super().__new__(cls, (lut, ff, bram, dsp), 
                index=['LUT', 'FF', 'BRAM', 'DSP'])


def write_program(level, N_max, K_max, l0=1, device='7020'):
    #
    # define variables
    #
    M = 4
    l = level
    assert l >= l0 and l0 >= 0
    if l == l0:
        cycles_per_feature = M**l
    else:
        cycles_per_feature = (M**l + M**(l-1)) / 2
    zynq_area = 6
    freq = 10**8

    #TODO: fixme
    cycles_per_feature = M**l

    #
    # Optimization Variables
    #
    K = list(range(1, K_max+1))
    N = list(range(1, N_max+1))
    Omega = ['Serializer', 'Exp', 'Payoff', 'MLDiff', 'Stats']
    Psi = ['ConfigBus', 'StreamingFifo', 'DMA']
    Phi = ['LUT', 'FF', 'BRAM', 'DSP']

    #
    # Ressource usage Data for Zynq
    #
    D = FPGAResource
    FPGA = {
        '7020': D(53200, 106400, 280, 220),
        '7100': D(277400, 554800, 1510, 2020)
    }
    Area = {
        'Zynq': FPGA[device],
        'alpha': D(0.8, 0.5, 1, 1),
        'Frontend': D(
            301 + 451 + 228 + (4153 if l==l0 else 5607) + 180 + 50+30*3 + 30,
            323 + 592 + 258 + (4241 if l==l0 else 5326) + 158 + 40+ 2*3 + 65,
              4 +   4 +   0 + (   2 if l==l0 else    6) +   0,
              0 +   1 +   0 + (  38 if l==l0 else   43) +   0),
    }
    print("N_max, K_max <= " + str(
            np.floor(np.min(FPGA[device] / Area['Frontend']))))
    Area['ConfigBus_Fixed'] = D(50, 40, 0, 0)
    Area['ConfigBus_Variable'] = D(30, 2, 0, 0)
    # backend based on hw/sw split
    Area['Serializer'] = D(65, 45, 0, 0)
    Area['Exp'] = Area['Serializer'] + D(900, 384, 0, 7)
    Area['Payoff'] = Area['Exp'] + D(400, 396, 0, 2) + D(30, 2, 0, 0)
    Area['MLDiff'] = Area['Payoff'] + (D(0,0,0,0) if l==l0
            else D(372, 355, 0, 2)) + D(30, 2, 0, 0)
    Area['Stats'] = Area['MLDiff'] + (D(2170, 1612, 4, 9) if l==l0
            else D(1454, 1164, 2, 6)) + D(30, 2, 0, 0)
    # communication core
    Area['ConfigBus'] = D(0, 0, 0, 0)
    Area['StreamingFifo'] = D(654, 611, 4, 0)
    Area['DMA'] = D(1864, 3122, 4, 0)

    Load = {}
    Load['Stats'] = 0
    Load['MLDiff'] = Load['Stats'] + (6E-9 if l==l0 else 3E-9)
    Load['Payoff'] = Load['MLDiff'] + (0 if l==l0 else 5E-9)
    Load['Exp'] = Load['Payoff'] + 6E-9
    Load['Serializer'] = Load['Exp'] + 250E-9

    Cores = {
        'Zynq': 2
    }
    Bandwidth = {
        'ConfigBus': 1 * 1024**2, 
        'StreamingFifo': 20 * 1024**2, 
        'DMA': 350 * 1024**2
    }
    Datawidth = {
        'Stats': 0,
        'MLDiff': (4 if l==l0 else 2),
        'Payoff': 4,
        'Exp': 4,
        'Serializer': 4
    }

    #
    # lp program helpers
    #
    
    scope = locals()
    lines = []
    env = Environment(lstrip_blocks=True, trim_blocks=True,
            variable_start_string='{', variable_end_string='}',
            block_start_string='[', block_end_string=']')
    n = k = phi = psi = omega = 0
    def dyn_scope():
        scope.update({'n': n, 'k': k, 'phi': phi, 'psi': psi, 'omega': omega})
        return scope
    def lp(template):
        lines.append(env.from_string(template).render(dyn_scope()))
    definitions = []
    def var(template):
        definitions.append(env.from_string(template).render(dyn_scope()))

    #
    # lp program
    #

    lp("max: hyper_performance"
            " - {freq/10} hyper_load"
            " - {freq/N_max/10} N;")
    for phi in Phi:
        lp("hyper_area_{phi} <= {Area['alpha'][phi] * Area['Zynq'][phi]};")
    lp("hyper_load <= 1;")
    lp("hyper_bandwidth <= 0 [for psi in Psi] + {Bandwidth[psi]} z_comm_{psi} [endfor];")
    
    var('int N;')
    for n in N:
        # define selection variables for all frontends of instance n
        var('int K_{n};')
        for k in K:
            var('int z_frontend_{n}_{k};')
            lp("z_frontend_{n}_{k} <= 1;")
            lp("{k} z_frontend_{n}_{k} <= K_{n};")
            lp("{n} z_frontend_{n}_{k} <= N;")
        lp("0 [for k in K] + z_frontend_{n}_{k} [endfor] = K_{n};")

        # define selection for hw/sw split in backend
        for omega in Omega:
            var("int z_backend_{n}_{omega};")
            lp("z_backend_{n}_{omega} <= 1;")
        lp("0[for omega in Omega] + z_backend_{n}_{omega} [endfor] = 1;")
        
        # define beta as utilization factor
        for omega in Omega:
            lp("beta_{n}_{omega} <= 1;")
            for k in K:
                lp("z_rate_{n}_{k}_{omega} <= beta_{n}_{omega};")
                lp("z_rate_{n}_{k}_{omega} >= beta_{n}_{omega}"
                        " - 1 + z_frontend_{n}_{k};")
                lp("z_rate_{n}_{k}_{omega} <= z_frontend_{n}_{k};")
                lp("z_rate_{n}_{k}_{omega} <= z_backend_{n}_{omega};")
        
        # limit frontend rate with feature rate of backend
        for omega in Omega:
            lp('0 [for k in K] + {1/cycles_per_feature} '
                    'z_rate_{n}_{k}_{omega}[endfor] <= 1;')

    # define N
    lp("0 [for n in N] + z_frontend_{n}_{1} [endfor] = N;")

    # define communication core
    for psi in Psi:
        var('int z_comm_{psi};')
        lp("z_comm_{psi} <= 1;")
    lp("0 [for psi in Psi] + z_comm_{psi} [endfor] = 1;")
    

    # performance
    lp('hyper_performance = 0 [for n in N][for k in K][for omega in Omega] '
            '+ {freq} z_rate_{n}_{k}_{omega} [endfor][endfor][endfor];')
    # area
    for phi in Phi:
        lp("hyper_area_{phi} = 0 "
                # frontend
                "[for n in N][for k in K]"
                "+ {Area['Frontend'][phi]} z_frontend_{n}_{k} [endfor][endfor]"
                # backend
                "[for n in N][for omega in Omega]"
                "+ {Area[omega][phi]} z_backend_{n}_{omega} [endfor][endfor]"
                # comm core
                "[for psi in Psi] + {Area[psi][phi]} z_comm_{psi} [endfor]"
                # config bus
                "+ {Area['ConfigBus_Variable'][phi]} N "
                "+ {Area['ConfigBus_Fixed'][phi]};")
    # load
    lp("hyper_load = 0 [for n in N][for k in K][for omega in Omega] + "
            "{freq * Load[omega] / Cores['Zynq'] / cycles_per_feature} "
            "z_rate_{n}_{k}_{omega} [endfor][endfor][endfor];")
    # bandwidth
    lp("hyper_bandwidth = 0 [for n in N][for k in K][for omega in Omega] + "
            "{freq * Datawidth[omega] / cycles_per_feature} "
            "z_rate_{n}_{k}_{omega} [endfor][endfor][endfor];")


    #
    # write program to file
    #

    with open(FILE.format(level=level, device=device), 'w') as f:
        data = '\n'.join(lines) + '\n'*5 + '\n'.join(definitions)
        f.write('\n'.join(line.strip() for line in data.split('\n')))


L = 5
for l in range(1, L+1):
    write_program(level=l, N_max=5, K_max=5, device='7020')
    #write_program(level=l, N_max=10, K_max=32, device='7100')


