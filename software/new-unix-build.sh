#!/bin/sh
#
# Copyright (C) 2013 University of Kaiserslautern
# Microelectronic Systems Design Research Group
#
# Christian Brugger (brugger@eit.uni-kl.de)
# 30 August 2013
#

set -e

rm -rf bin
rm -rf build
mkdir build

cd build
cmake -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_INSTALL_PREFIX=.. ..
make run_heston eval_heston
make install
