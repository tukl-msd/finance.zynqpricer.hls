set -e
sync
make
sudo chown root:admin /dev/xdevcfg; sudo chmod 660 /dev/xdevcfg
sudo chmod 660 /dev/mem
cat heston_sl_3x.bin > /dev/xdevcfg
sudo ./init_rng
sudo taskset -c 1 ./heston_sl params_run.json
