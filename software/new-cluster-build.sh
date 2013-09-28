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
module load gcc/4.7.2
module load intel/13.0.1

rm -rf build
mkdir build

cd build
#cmake -DCMAKE_C_COMPILER=icc -DCMAKE_CXX_COMPILER=icpc -DCMAKE_INSTALL_PREFIX=.. ..
cmake -DCMAKE_C_COMPILER=mpiicc -DCMAKE_CXX_COMPILER=mpiicpc -DCMAKE_INSTALL_PREFIX=.. ..
make run_heston
make install
