#!/bin/bash

interface_list_cmd="lspci -k | grep -i Ethernet"

print_usage() {
    echo "This script uses usertools/dpdk-devbind.py to attach igb_uio/rte_kni to interfaces"
    echo "./quick_start_dpdk_setup.sh 
        --dpdkpath <path_to_dpdk (parent of directory usertools)>
        --interface-1 <00:03.0>
        --interface-2 <00:05.0>
        --igbuio-path <path to igb_uio.ko>
        --rtekni-path <path to rte_kni.ko>"
     echo "Use lspci -k to determine interface id to use. Available interfaces -"
     eval $interface_list_cmd
}

POSITIONAL=()
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    --interface-1)
    INTERFACE1="$2"
    shift # past argument
    shift # past value
    ;;
    --interface-2)
    INTERFACE2="$2"
    shift # past argument
    shift # past value
    ;;
    --igbuio-path)
    IGBUIOPATH="$2"
    shift # past argument
    shift # past value
    ;;
    --rtekni-path)
    RTEKNIPATH="$2"
    shift # past argument
    shift # past value
    ;;
    -d|--dpdkpath)
    DPDKPATH="$2"
    shift # past argument
    shift # past value
    ;;
    *)    # unknown option
    POSITIONAL+=("$1") # save it in an array for later
    shift # past argument
    ;;
esac
done
set -- "${POSITIONAL[@]}" # restore positional parameters

echo INTERFACE-1 = "${INTERFACE1}"
echo INTERFACE-2 = "${INTERFACE2}"
echo DPDK PATH   = "${DPDKPATH}"
echo IGBUIO PATH = "${IGBUIOPATH}"
echo RTEKNI PATH = "${RTEKNIPATH}"

if [ -z "${INTERFACE1}" ] && [ -z "${INTERFACE2}" ] ; then
    echo Need --interface-1 or --interface-2 or both
    print_usage
    exit 1
fi

if [ -z "${DPDKPATH}" ] ; then
    echo Need --dpdkpath
    print_usage
    exit 1
fi

if [ -z "${IGBUIOPATH}" ] || [ -z "${RTEKNIPATH}" ] ; then
    echo Need --igbuio-path and --rtekni-path
    print_usage
    exit 1
fi

echo 0 | sudo tee /proc/sys/kernel/randomize_va_space
echo 1024 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
##echo 1024 > /sys/kernel/mm/hugepages/hugepages-1048576kB/nr_hugepages
mkdir -p /mnt/huge
mount -t hugetlbfs nodev /mnt/huge
cat /proc/meminfo | grep -i huge 
modprobe uio
insmod ${IGBUIOPATH}
insmod ${RTEKNIPATH}
#TODO: Read the name <ens3> by matching device-id in args with ifconfig output
#ifconfig ens3 down
python ${DPDKPATH}/usertools/dpdk-devbind.py --status
python ${DPDKPATH}/usertools/dpdk-devbind.py --bind igb_uio ${INTERFACE1}
python ${DPDKPATH}/usertools/dpdk-devbind.py --status
#TODO: Read the name <ens3> by matching device-id in args with ifconfig output
#ifconfig ens5 down
python ${DPDKPATH}/usertools/dpdk-devbind.py --status
python ${DPDKPATH}/usertools/dpdk-devbind.py --bind igb_uio ${INTERFACE2}
python ${DPDKPATH}/usertools/dpdk-devbind.py --status
lspci -k
