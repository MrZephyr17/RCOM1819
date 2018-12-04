#!/bin/bash
ifconfig eth0 up 
ifconfig
ifconfig eth0 192.168.0.1/16 
route add -net 192.168.1.0/24 gw 172.16.4.254
route add default gw 192.168.1.1 
route -n
echo 1 > /proc/sys/net/ipv4/ip_forward
echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts