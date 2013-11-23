#!/bin/sh
#
# Copyright (C) 2013 University of Kaiserslautern
# Microelectronic Systems Design Research Group
# 
# Christian Brugger (brugger@eit.uni-kl.de)
# 18. July 2013
#

set -e

bsub -q short -a openmpi -W 1:30 -n 512 -R "select[model==XEON_E5_2670]" \
		-R "rusage[mem=2048]" mpirun zynq/software/bin/eval_heston \
		params.json

