#!/bin/sh
#
# Copyright (C) 2013 University of Kaiserslautern
# Microelectronic Systems Design Research Group
# 
# Christian Brugger (brugger@eit.uni-kl.de)
# 18. July 2013
#

set -e

git clone ../.. zynq
cd zynq
git checkout 87c6d514d071a8812c49df6ec5814b5035eceeae
rm -rf .git
cd software
./new-cluster-build.sh
cd ../..

