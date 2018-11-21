#!/bin/bash          
# offload NIC（if there is only one NIC，the follow commands must run in a script）

interface="ens4"
interface2="ens5"
root_dir="`pwd`/.."

# dpdk take over nic
#ifconfig $interface down
python $root_dir/dpdk/usertools/dpdk-devbind.py --bind=igb_uio $interface

#ifconfig $interface down
python $root_dir/dpdk/usertools/dpdk-devbind.py --bind=igb_uio $interface2

## start Nginx
cd $root_dir/f-stack/
./start.sh -b $root_dir/build/nginx/nginx -c $root_dir/f-stack/config.ini
#
## start kni
sleep 30
ifconfig veth0 ${myaddr}  netmask ${mymask}  broadcast ${mybc} hw ether ${myhw}
route add -net 0.0.0.0 gw ${mygw} dev veth0
