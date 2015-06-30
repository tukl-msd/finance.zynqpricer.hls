"""
Copyright (C) 2013 University of Kaiserslautern
Microelectronic Systems Design Research Group

Christian Brugger (brugger@eit.uni-kl.de)
23. November 2013
"""

import numpy as np
import pandas as pd
import functools

class FPGAResource(pd.Series):
    """ bram - 18k BRAM """
    def __new__(cls, lut, ff, bram, dsp):
        return super().__new__(cls, (lut, ff, bram, dsp), 
                index=['lut', 'ff', 'bram', 'dsp'])


DMA_BANDWIDTH = 400 * 1024**2 # in MB/s
FIFO_BANDWIDTH = 20 * 1024**2 # in MB/s

class PricerSetup:
    def __init__(self, feature):
        self.feature = feature # instance of Feature

class HW_SW_Split:
    def __init__(self, name):
        self.name = name
    def __repr__(self):
        return self.name
ALL_HW_SW_SPLITS = [
    HW_SW_Split("AfterSerializer"), 
    HW_SW_Split("AfterExp"), 
    HW_SW_Split("AfterPayoff"), 
    HW_SW_Split("AfterDiff"), 
    HW_SW_Split("AfterStats")
]
CONSIDERED_HW_SW_SPLITS = [
    HW_SW_Split("AfterSerializer"),
    HW_SW_Split("AfterExp"), 
    HW_SW_Split("AfterStats")
]

# frontend
IncrementGenerator = FPGAResource(980, 1202, 4, 1)
SingellevelKernel = FPGAResource(4153, 4241, 2, 38)
MultilevelKernel = FPGAResource(5607, 5326, 6, 43)
#MLPathSerializer = FPGAResource(124,112,33,0)
MLPathSerializer = FPGAResource(0,0,0,0)
class FeatureSerializer(FPGAResource):
    def __new__(cls, n):
        return super().__new__(cls, 65+30*n, 45+65*n, 0, 0)
# features
class Feature:
    def __init__(self, is_log_feature, fpga_ressources):
        self.is_log_feature = is_log_feature
        self._fpga_ressources = FPGAResource(*fpga_ressources)
    def get_fpga_ressources(self):
        return self._fpga_ressources
BarrierFeature = Feature(True, (180, 158, 0, 0))
# backend
class ConfigBus(FPGAResource):
    def __new__(cls, k):
        return super().__new__(cls, 50+30*k, 40+2*k, 0, 0)
Exponential = FPGAResource(900, 384, 0, 7)
CallPayoff = FPGAResource(422, 210, 0, 2)
MLDifference = FPGAResource(372, 355, 0, 2)
StatisticsII1 = FPGAResource(2170, 1612, 4, 9)
StatisticsII2 = FPGAResource(1454, 1164, 2, 6)
# interconnect
StreamingFifo = FPGAResource(654, 611, 4, 0)
DMACore = FPGAResource(1864, 3122, 2, 0)

class FPGA(FPGAResource):
    def __new__(cls, resources, core_cnt):
        inst = super().__new__(cls, *resources)
        inst.core_cnt = core_cnt
        return inst
Zynq7Z020 = FPGA((53200, 106400, 280, 220), 2)
SynthesisWeight = FPGAResource(0.8, 0.5, 1, 1)

class HyperInstance:
    frequency = 1E8 # 100 MHz
    def __init__(self, k_frontend, hw_sw_split, do_single_level, 
            feature, stats_ii):
        self.k_frontend = k_frontend # number of Hyper frontends
        self.hw_sw_split = hw_sw_split # one of HWSESplit
        self.do_single_level = do_single_level # boolean
        self.feature = feature # instance of Feature
        self.stats_ii = stats_ii # integer

    def __repr__(self):
        return "<Hyper:k={},{},stats_ii={}>".format(
                self.k_frontend, self.hw_sw_split, self.stats_ii)

    @functools.lru_cache()
    def get_fpga_ressources(self):
        # frontend
        res = IncrementGenerator
        if self.do_single_level:
            res += SingellevelKernel 
        else :
            res += MultilevelKernel + MLPathSerializer
        res += self.feature.get_fpga_ressources()
        res += ConfigBus(3)
        res *= self.k_frontend
        # backend
        while True:
            k_config = 1
            res += FeatureSerializer(self.k_frontend)
            if self.hw_sw_split.name == 'AfterSerializer':
                break
            if self.feature.is_log_feature:
                res += Exponential
            if self.hw_sw_split.name == 'AfterExp':
                break
            # todo payoff
            k_config += 1
            if self.hw_sw_split.name == 'AfterPayoff':
                break
            # todo diff
            if self.hw_sw_split.name == 'AfterDiff':
                break
            if not self.do_single_level:
                res += MLDifference
            if self.stats_ii == 1:
                res += StatisticsII1
            else:
                assert(self.stats_ii == 2)
                res += StatisticsII2
            k_config += 1
            assert(self.hw_sw_split.name == 'AfterStats')
            break
        res += ConfigBus(k_config)
        return res

    @functools.lru_cache()
    def get_fpga_bandwidth(self, step_cnt):
        """ available bandwidth in steps/second """
        # frontend
        rate = self.k_frontend
        # stats bottleneck
        if self.hw_sw_split.name == 'AfterStats':
            rate = min(rate / step_cnt, 1 / self.stats_ii) * step_cnt
        return rate * self.frequency

    @functools.lru_cache()
    def get_cpu_load(self, step_cnt, core_cnt):
        """ returns cpu load for available fpga bandwidth """
        time = 0 # in seconds
        feature_rate = self.get_fpga_bandwidth(step_cnt) / step_cnt
        while True:
            if self.hw_sw_split.name == 'AfterStats':
                break
            time += feature_rate * 5e-9
            if self.hw_sw_split.name == 'AfterDiff':
                break
            time += feature_rate * 5e-9
            if self.hw_sw_split.name == 'AfterPayoff':
                break
            time += feature_rate * 5e-9
            if self.hw_sw_split.name == 'AfterExp':
                break
            time += feature_rate * 250e-9
            assert(self.hw_sw_split.name == 'AfterSerializer')
            break
        return time / core_cnt

    @functools.lru_cache()
    def get_bandwidth_load(self, step_cnt, avail_speed):
        """ get FPGA/CPU bandwidth load for available fpga bandwidth """
        data_size = 4 # 4 bytes
        feature_rate = self.get_fpga_bandwidth(step_cnt) / step_cnt
        bandwith = 0
        while True:
            if self.hw_sw_split.name == 'AfterStats':
                break
            bandwith += feature_rate
            if self.hw_sw_split.name == 'AfterDiff':
                break
            if self.hw_sw_split.name == 'AfterPayoff':
                break
            if self.hw_sw_split.name == 'AfterExp':
                break
            assert(self.hw_sw_split.name == 'AfterSerializer')
            break
        return bandwith * data_size / avail_speed


class HyperArchitecture:
    def __init__(self, hyper_instances, has_dma):
        self.hyper_instances = hyper_instances
        self.has_dma = has_dma

    def __repr__(self):
        return "<HyPER: " + repr(self.hyper_instances) + "," + \
                ("DMA" if self.has_dma else "FIFO") + ">"
    
    @functools.lru_cache()
    def get_fpga_ressources(self):
        res = sum(instance.get_fpga_ressources() 
                for instance in self.hyper_instances) 
        res += ConfigBus(len(self.hyper_instances))
        need_streaming_core = any(instance.hw_sw_split.name != 'AfterStats'
                for instance in self.hyper_instances)
        if need_streaming_core:
            if self.has_dma:
                res += DMACore
            else:
                res += StreamingFifo
        return res
    
    @functools.lru_cache()
    def get_total_cpu_load(self, step_cnt, core_cnt):
        return sum(instance.get_cpu_load(step_cnt, core_cnt) 
                for instance in self.hyper_instances)
    
    @functools.lru_cache()
    def get_total_bandwidth_load(self, step_cnt):
        if self.has_dma:
            avail_bandwidth = DMA_BANDWIDTH
        else:
            avail_bandwidth = FIFO_BANDWIDTH
        return sum(instance.get_bandwidth_load(step_cnt, avail_bandwidth) 
                for instance in self.hyper_instances)

    @functools.lru_cache()
    def get_effective_throughput(self, step_cnt, core_cnt):
        fpga_bandwidth = sum(instance.get_fpga_bandwidth(step_cnt) 
                for instance in self.hyper_instances)
        cpu_load = self.get_total_cpu_load(step_cnt, core_cnt)
        bandwidth_load = self.get_total_bandwidth_load(step_cnt)
        #TODO: distribute load
        max_load = max([cpu_load, bandwidth_load, 1])
        throughput = fpga_bandwidth / max_load
        return throughput


class ArchitectureOptimizer:
    def __init__(self, setup, fpga):
        self.setup = setup
        self.fpga = fpga

    def _get_possible_hyper_instances(self, do_multilevel):
        """get list of hyper instances that fit on fpga"""
        instances = []
        for split in CONSIDERED_HW_SW_SPLITS:
            if split.name == 'AfterStats':
                possible_ii = [1, 2]
            else:
                possible_ii = [1]
            for stats_ii in possible_ii:
                n = 1
                while True:
                    instance = HyperInstance(n, split, do_multilevel, 
                            self.setup.feature, stats_ii)
                    if all(instance.get_fpga_ressources() <= 
                           self.fpga * SynthesisWeight):
                        instances.append(instance)
                    else:
                        break
                    n += 1
        return instances
    
    def find_best_architecture(self, do_multilevel, step_cnt):
        hyper_instances = self._get_possible_hyper_instances(do_multilevel)
        hyper_instances = sorted(hyper_instances, 
                key=lambda x: x.k_frontend, reverse=True)

        def eval_instances(root_instances):
            arch_does_fit = False
            for has_dma in [False, True]:
                arch = HyperArchitecture(root_instances, has_dma)
                if all(arch.get_fpga_ressources() <= self.fpga * SynthesisWeight):
                    yield arch
                    arch_does_fit = True
            if arch_does_fit: # eval sub architectures
                for instance in hyper_instances:
                    sub_inst = root_instances + [instance]
                    #if sub_inst[0].k_frontend > 2:
                    for item in eval_instances(sub_inst):
                        yield item
        
        max_throughput = 0
        max_arch = []
        for arch in eval_instances([]):
            throughput = arch.get_effective_throughput(
                    step_cnt, self.fpga.core_cnt)
            if throughput > max_throughput:
                max_arch = []
            if throughput >= max_throughput:
                max_throughput = throughput
                max_arch.append(arch)
            #print(arch, max_throughput)
        #arch = HyperArchitecture(hyper_instances)
        #print(arch.get_fpga_ressources() <= self.fpga)

        print(max_arch)
        best_cpu_load = sorted(max_arch, key=lambda x: x.get_total_cpu_load(
                step_cnt, self.fpga.core_cnt))
        best_bandwidth_load = sorted(max_arch, key=lambda x: 
                x.get_total_bandwidth_load(step_cnt))

        print(step_cnt)
        print("Lowest CPU load:", best_cpu_load, max_throughput, 
                "cpu_load:", 
                best_cpu_load[0].get_total_cpu_load(step_cnt, self.fpga.core_cnt),
                "bandwidth_load", 
                best_cpu_load[0].get_total_bandwidth_load(step_cnt),
                "fpga_res:", max(best_cpu_load[0].get_fpga_ressources() / 
                    (self.fpga * SynthesisWeight)))
        print("Lowest Bandwidth load:", best_bandwidth_load, max_throughput, 
                "cpu_load:", best_bandwidth_load[0].get_total_cpu_load(step_cnt, 
                self.fpga.core_cnt), "bandwidth_load", 
                best_bandwidth_load[0].get_total_bandwidth_load(step_cnt),
                "fpga_res:", max(best_bandwidth_load[0].get_fpga_ressources() / 
                    (self.fpga * SynthesisWeight)))
        return best_cpu_load, best_bandwidth_load


def check():
    fpga = Zynq7Z020
    setup = PricerSetup(BarrierFeature)

    architectures = {
        #1: HyperArchitecture([
        #    HyperInstance(1, HW_SW_Split("AfterStats"), True, setup.feature, 1),
        #    HyperInstance(1, HW_SW_Split("AfterStats"), True, setup.feature, 1),
        #    HyperInstance(1, HW_SW_Split("AfterStats"), True, setup.feature, 1),
        #    HyperInstance(1, HW_SW_Split("AfterStats"), True, setup.feature, 1)
        #    ]),
        4: HyperArchitecture([
            HyperInstance(4, HW_SW_Split("AfterStats"), True, setup.feature, 1),
            HyperInstance(1, HW_SW_Split("AfterExp"), True, setup.feature, 1)
            ], True),
        16: HyperArchitecture([
            HyperInstance(4, HW_SW_Split("AfterStats"), False, setup.feature, 2)
            ], False),
        64: HyperArchitecture([
            HyperInstance(5, HW_SW_Split("AfterSerializer"), False, setup.feature, 1)
            ], True),
        256: HyperArchitecture([
            HyperInstance(5, HW_SW_Split("AfterSerializer"), False, setup.feature, 1)
            ], False),
        1024: HyperArchitecture([
            HyperInstance(5, HW_SW_Split("AfterSerializer"), False, setup.feature, 1)
            ], False),
        }
    for step_cnt in sorted(architectures):
        arch = architectures[step_cnt]
        print(step_cnt, arch)
        print(step_cnt, "bandwidth:", arch.get_effective_throughput(step_cnt, fpga.core_cnt),
                "cpu_load:", arch.get_total_cpu_load(step_cnt, fpga.core_cnt), 
                "bandwidth_load", arch.get_total_bandwidth_load(step_cnt),
                "fpga_util", max(arch.get_fpga_ressources() / 
                        (fpga * SynthesisWeight)))
        print(arch.get_fpga_ressources() / fpga)


def main():
    fpga = Zynq7Z020
    setup = PricerSetup(BarrierFeature)

    optimizer = ArchitectureOptimizer(setup, fpga)
    
    for level in reversed(range(5)):
        step_cnt = 4 ** (level + 1)
        do_multilevel = level == 0
        optimizer.find_best_architecture(do_multilevel, step_cnt)
        
    #optimizer.find_best_architecture(do_multilevel=False, step_cnt=1000)

if __name__ == "__main__":
    check()
    main()
