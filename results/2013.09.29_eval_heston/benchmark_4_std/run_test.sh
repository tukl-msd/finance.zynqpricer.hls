#!/bin/sh
#
# Copyright (C) 2013 University of Kaiserslautern
# Microelectronic Systems Design Research Group
# 
# Christian Brugger (brugger@eit.uni-kl.de)
# 18. July 2013
#

set -e

git clone ../../.. zynq
cd zynq
git checkout 0521c3151e5b92eefc8534f7b8a29d18fb0307d6
rm -rf .git
cd software
./new-cluster-build.sh
cd ../..

submit.sh

