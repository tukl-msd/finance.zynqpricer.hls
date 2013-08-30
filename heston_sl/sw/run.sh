set -e
sync
make init_rng run_both
sudo chown root:admin /dev/xdevcfg; sudo chmod 660 /dev/xdevcfg
sudo chmod 660 /dev/mem
cat ../bin/heston_sl_3x.bin > /dev/xdevcfg
sudo ./init_rng ../bin/heston_sl_3x.json
sudo taskset -c 1 ./run_both params_run.json
