python ./dpdk-2-2-2018/usertools/dpdk-devbind.py --status
echo 0 | sudo tee /proc/sys/kernel/randomize_va_space
echo 1024 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
mount -t hugetlbfs nodev /mnt/huge
cat /proc/meminfo | grep -i huge 
modprobe uio
python ./dpdk-2-2-2018/usertools/dpdk-devbind.py -u 0000:00:03.0
rmmod igb_uio
rmmod rte_kni
ifconfig ens3 down
python ./dpdk-2-2-2018/usertools/dpdk-devbind.py --bind ena 0000:00:03.0
python ./dpdk-2-2-2018/usertools/dpdk-devbind.py --status
ifconfig ens3 up
