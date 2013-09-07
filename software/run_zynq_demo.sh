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
cat ../bitstream/heston_sl_6x.bin > /dev/xdevcfg

# init rng
sudo bin/init_rng ../bitstream/heston_sl_6x.json

# run acc & cpu demo
sudo bin/run_both parameters/params_zynq_demo_both.json \
		../bitstream/heston_sl_6x.json

# run acc only demo
sudo taskset -c 1 bin/run_acc parameters/params_zynq_demo_acc.json \
		../bitstream/heston_sl_6x.json

