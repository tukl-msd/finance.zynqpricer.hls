set -e
sync
make
sudo chown root:admin /dev/xdevcfg; sudo chmod 660 /dev/xdevcfg
sudo chmod 660 /dev/mem
cat hw_heston_sl.bin > /dev/xdevcfg
sudo ./init_rng
sudo taskset 2 ./heston_sl 100
