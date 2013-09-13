#!/bin/sh
#
# Copyright (C) 2013 University of Kaiserslautern
# Microelectronic Systems Design Research Group
#
# Christian Brugger (brugger@eit.uni-kl.de)
# 30 August 2013
#

set -e

rm -rf build
mkdir build

cd build
cmake -DCMAKE_INSTALL_PREFIX=.. ..
make -j 2 init_rng run_heston
make install
#ctest --output-on-failure .

