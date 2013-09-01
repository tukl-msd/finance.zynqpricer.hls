#
# Copyright (C) 2013 University of Kaiserslautern
# Microelectronic Systems Design Research Group
# 
# Christian Brugger (brugger@eit.uni-kl.de)
# 30. August 2013
#

set -e

if [[ ! ( -f "bin/init_rng" && -f "bin/run_both" ) ]]; then
	echo "ERROR: Cannot find binaries, please first run: ./clean-zynq-build.sh"
	exit 1
fi

echo "Running Zynq Demo."

sync
sudo chown root:admin /dev/xdevcfg; sudo chmod 660 /dev/xdevcfg
sudo chmod 660 /dev/mem

echo "Reconfiguring heston single-level accelerators."
cat ../bitstream/heston_sl_3x.bin > /dev/xdevcfg

sudo bin/init_rng ../bitstream/heston_sl_3x.json
sudo bin/run_both parameters/params_zynq_demo.json \
		../bitstream/heston_sl_3x.json
