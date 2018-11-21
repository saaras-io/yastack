#!/bin/bash          
interface="ens3"

export myaddr=`ifconfig $interface | grep "inet" | grep -v "inet6" | awk -F ' '  '{print $2}' | awk -F ':' '{print $2}'`
export mymask=`ifconfig $interface | grep "inet" | grep -v "inet6" | awk -F ' '  '{print $4}' | awk -F ':' '{print $2}'`
export mybc=`ifconfig $interface | grep "inet" | grep -v "inet6" | awk -F ' '  '{print $3}' | awk -F ':' '{print $2}'`
export myhw=`ifconfig $interface | grep "Link" | grep -v "inet6" | awk -F ' '  '{print $5}'`
export mygw=`route -n | grep 0.0.0.0 | grep $interface | grep UG | awk -F ' ' '{print $2}'`
