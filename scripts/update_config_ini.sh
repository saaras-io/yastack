#!/bin/bash

#FS='f-stack'
FS='fs-new'


root_dir="`pwd`/.."


echo "Updating: $root_dir/${FS}/config.ini"

#update addresses

sed "s/addr=192.168.1.2/addr=${myaddr}/" -i $root_dir/${FS}/config.ini
sed "s/netmask=255.255.255.0/netmask=${mymask}/" -i $root_dir/${FS}/config.ini
sed "s/broadcast=192.168.1.255/broadcast=${mybc}/" -i $root_dir/${FS}/config.ini
sed "s/gateway=192.168.1.1/gateway=${mygw}/" -i $root_dir/${FS}/config.ini


#setup kni

#sed "s/#\[kni\]/\[kni\]/" -i $root_dir/${FS}/config.ini
#sed "s/#enable=1/enable=1/" -i $root_dir/${FS}/config.ini
#sed "s/#method=reject/method=reject/" -i $root_dir/${FS}/config.ini
#sed "s/#tcp_port=80/tcp_port=80/" -i $root_dir/${FS}/config.ini
#sed "s/#vlanstrip=1/vlanstrip=1/" -i $root_dir/${FS}/config.ini
