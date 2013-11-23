#!/bin/sh
#
# Copyright (C) 2013 University of Kaiserslautern
# Microelectronic Systems Design Research Group
#
# Christian Brugger (brugger@eit.uni-kl.de)
# 30 August 2013
#

set -e

module unload gcc
module unload intel
module unload openmpi
module load gcc/4.7.2
module load intel/2013.0.028
module load openmpi/1.7.2-intel-2013.0.028

rm -rf bin
rm -rf build
mkdir build

cd build
cmake -DCMAKE_C_COMPILER=mpiicc -DCMAKE_CXX_COMPILER=mpiicpc -DCMAKE_INSTALL_PREFIX=.. ..
make run_heston eval_heston
make install

echo Run benchmark with: mpirun -n 32 bin/run_heston -sl -cpu parameters/params_acc_bench1.json
