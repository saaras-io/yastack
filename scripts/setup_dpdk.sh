#!/bin/bash

root_dir="`pwd`/.."

echo "root dir - $root_dir"

#set hugepage    
echo 1024 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
mkdir -p /mnt/huge
mount -t hugetlbfs nodev /mnt/huge

#TODO - build directory is hard coded here

# insmod ko
modprobe uio
modprobe hwmon
insmod $root_dir/build/kmod/igb_uio.ko
insmod $root_dir/build/kmod/rte_kni.ko
