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

# sync filesystem, bugs in the driver may freze the zynq
sync
sudo chown root:admin /dev/xdevcfg; sudo chmod 660 /dev/xdevcfg
sudo chmod 660 /dev/mem
cat ../bin/heston_sl_3x.bin > /dev/xdevcfg

sudo bin/init_rng ../bin/heston_sl_3x.json
sudo taskset -c 1 bin/run_both parameters/params_run.json
