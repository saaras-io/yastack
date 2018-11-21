Install dependencies
--------------------

	sudo apt-get install autoconf
	sudo apt-get install make
	sudo apt-get install libunwind8 apt-transport-https
	sudo apt-get install -y git autoconf automake libtool make g++ unzip
	sudo apt-get install libnuma-dev
	sudo apt-get install gawk
	sudo apt-get install linux-headers-$(uname -r)

Install latest version of cmake (https://cmake.org/download/)


Install protoc-gen-validate
---------------------------

protoc-gen-validate is a tool that is written in golang.

If you are using the latest version of golang as of writing this [2018-11],
you don't need to set environment variable GOPATH. Golang uses the directory
$HOME/go by default.

Fetch the repository in $HOME/go

	go get -d github.com/lyft/protoc-gen-validate

Change directory to -

	$HOME/go/src/github.com/lyft/protoc-gen-validate

Checkout 0.0.6 version of the repository

	git checkout v0.0.6

Build the project
	
	make build 

Check binary is present in $HOME/go/bin

Update path to make the binary available to other programs

	export PATH=$PATH:$HOME/go/bin


Build yastack source code
--------------------------

clone yastack
	
	git clone https://github.com/saaras-io/yastack.git

Go to the root directory of yastack.

yastack uses CMake. Create a directory to build the image.

	mkdir -p build

Change directory to newly created directory build

	cd build

Invoke cmake to generate Makefiles

	cmake ..

Run make to build source
	
	make VERBOSE=1

Generate certs for listener
---------------------------
	export PATH_TO_YASTACK_ROOT=/home/ubuntu/yastack
	$PATH_TO_YASTACK_ROOT/scripts/self_signed_cert.sh yastack.app 2048

This should create files yastack.app.key and yastack.app.cert
Copy these files to -
	$PATH_TO_YASTACK_ROOT/build/ev/source/exe

Edit the envoy config to use key and certificate
------------------------------------------------
	admin:
	  access_log_path: /tmp/admin_access.log
	  address:
	    socket_address: { address: 0.0.0.0, port_value: 9995, provider: HOST }
	
	static_resources:
	  listeners:
	  - name: listener_0
	    address:
	        socket_address: { address: 0.0.0.0, port_value: 10000, provider: FP}
	    filter_chains:
	    - filter_chain_match:
	      tls_context:
	        common_tls_context:
	            tls_certificates:
	                - certificate_chain: { filename: "yastack.app.cert" }
	                  private_key: {filename: "yastack.app.key" }
	      filters:
	      - name: envoy.http_connection_manager
	        config:
	          stat_prefix: ingress_http
	          codec_type: AUTO
	          route_config:
	            name: local_route
	            virtual_hosts:
	            - name: local_service
	              domains: ["*"]
	              routes:
	              - match: { prefix: "/" }
	                route: { cluster: service_local}
	          http_filters:
	          - name: envoy.router
	  clusters:
	  - name: service_local
	    connect_timeout: 0.25s
	    type: STATIC
	    dns_lookup_family: V4_ONLY
	    lb_policy: ROUND_ROBIN
	    hosts: [ { socket_address: { address: 172.31.40.255, port_value: 8000 }}, { socket_address: { address: 172.31.40.255, port_value: 8001 }}]


Setup dpdk config
-----------------

	[dpdk]
	## Hexadecimal bitmask of cores to run on.
	lcore_mask=3
	channel=4
	promiscuous=1
	numa_on=1
	## TCP segment offload, default: disabled.
	tso=0
	## HW vlan strip, default: enabled.
	vlan_strip=1
	soft_dispatch=1
	debug=1
	
	# enabled port list
	#
	# EBNF grammar:
	#
	#    exp      ::= num_list {"," num_list}
	#    num_list ::= <num> | <range>
	#    range    ::= <num>"-"<num>
	#    num      ::= '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'
	#
	# examples
	#    0-3       ports 0, 1,2,3 are enabled
	#    1-3,4,7   ports 1,2,3,4,7 are enabled
	port_list=0
	
	[system]
	# one core that only does dispatching
	dispatch_only_core=0
	
	## Port config section
	## Correspond to dpdk.port_list's index: port0, port1...
	#[port0]
	#addr=172.31.20.127
	#netmask=255.255.240.0
	#broadcast=172.31.31.255
	#gateway=172.31.16.1
	
	[port0]
	addr=172.31.9.84
	netmask=255.255.240.0
	broadcast=172.31.15.255
	gateway=172.31.16.1
	hardware_rss=0
	
	## lcore list used to handle this port
	## the format is same as port_list
	lcore_list=0,1
	
	## Packet capture path, this will hurt performance
	#pcap=./a.pcap
	
	## Kni config: if enabled and method=reject,
	## all packets that do not belong to the following tcp_port and udp_port
	## will transmit to kernel; if method=accept, all packets that belong to
	## the following tcp_port and udp_port will transmit to kernel.
	#[kni]
	#enable=1
	#method=reject
	### The format is same as port_list
	#tcp_port=80,443
	#udp_port=53
	
	## FreeBSD network performance tuning configurations.
	## Most native FreeBSD configurations are supported.
	[freebsd.boot]
	hz=100
	
	## Block out a range of descriptors to avoid overlap
	## with the kernel's descriptor space.
	## You can increase this value according to your app.
	fd_reserve=1024
	kern.ipc.maxsockets=262144
	
	net.inet.tcp.syncache.hashsize=4096
	net.inet.tcp.syncache.bucketlimit=100
	
	net.inet.tcp.tcbhashsize=65536
	
	[freebsd.sysctl]
	kern.ipc.somaxconn=32768
	kern.ipc.maxsockbuf=16777216
	
	net.link.ether.inet.maxhold=5
	
	net.inet.tcp.fast_finwait2_recycle=1
	net.inet.tcp.sendspace=16384
	net.inet.tcp.recvspace=8192
	net.inet.tcp.nolocaltimewait=1
	net.inet.tcp.cc.algorithm=cubic
	net.inet.tcp.sendbuf_max=16777216
	net.inet.tcp.recvbuf_max=16777216
	net.inet.tcp.sendbuf_auto=1
	net.inet.tcp.recvbuf_auto=1
	net.inet.tcp.sendbuf_inc=16384
	net.inet.tcp.recvbuf_inc=524288
	net.inet.tcp.sack.enable=1
	net.inet.tcp.blackhole=1
	net.inet.tcp.msl=2000
	net.inet.tcp.delayed_ack=0
	
	net.inet.udp.blackhole=1
	net.inet.ip.redirect=0	

Note the [port0] config above. This is the config that will be used to setup the interface.

Run HTTP server on port 8000/8001
---------------------------------

	python -m SimpleHTTPServer 8000
	python -m SimpleHTTPServer 8001


Enable aws enhanced networking
------------------------------
https://docs.aws.amazon.com/AWSEC2/latest/UserGuide/enhanced-networking-ena.html
https://docs.aws.amazon.com/AWSEC2/latest/UserGuide/sriov-networking.html

This is how interfaces on my c4.xlarge aws instance look with ena enabled

	ubuntu@ip-172-31-27-43:~$ lspci -k
	00:00.0 Host bridge: Intel Corporation 440FX - 82441FX PMC [Natoma] (rev 02)
	        Subsystem: Red Hat, Inc Qemu virtual machine
	00:01.0 ISA bridge: Intel Corporation 82371SB PIIX3 ISA [Natoma/Triton II]
	        Subsystem: Red Hat, Inc Qemu virtual machine
	00:01.1 IDE interface: Intel Corporation 82371SB PIIX3 IDE [Natoma/Triton II]
	        Subsystem: XenSource, Inc. 82371SB PIIX3 IDE [Natoma/Triton II]
	        Kernel driver in use: ata_piix
	00:01.3 Bridge: Intel Corporation 82371AB/EB/MB PIIX4 ACPI (rev 01)
	        Subsystem: Red Hat, Inc Qemu virtual machine
	00:02.0 VGA compatible controller: Cirrus Logic GD 5446
	        Subsystem: XenSource, Inc. GD 5446
	        Kernel modules: cirrusfb
	00:03.0 Ethernet controller: Intel Corporation 82599 Ethernet Controller Virtual Function (rev 01)
	        Kernel driver in use: ixgbevf
	        Kernel modules: ixgbevf
	00:04.0 Ethernet controller: Intel Corporation 82599 Ethernet Controller Virtual Function (rev 01)
	        Kernel driver in use: ixgbevf
	        Kernel modules: ixgbevf
	00:05.0 Ethernet controller: Intel Corporation 82599 Ethernet Controller Virtual Function (rev 01)
	        Kernel driver in use: igb_uio
	        Kernel modules: ixgbevf
	00:06.0 Ethernet controller: Intel Corporation 82599 Ethernet Controller Virtual Function (rev 01)
	        Kernel driver in use: igb_uio
	        Kernel modules: ixgbevf
	00:1f.0 Unassigned class [ff80]: XenSource, Inc. Xen Platform Device (rev 01)
	        Subsystem: XenSource, Inc. Xen Platform Device
	        Kernel driver in use: xen-platform-pci
	ubuntu@ip-172-31-27-43:~$


Bind dpdk drivers with NICs
---------------------------
From the build directory run -

	../scripts/quick_start_dpdk_setup.sh --dpdkpath ../dpdk/ --interface-1 00:05.0 --interface-2 00:06.0 --igbuio-path ./dpdk/kmod/igb_uio.ko --rtekni-path ./dpdk/kmod/rte_kni.ko


Run envoy binary on two cores
-----------------------------

	cd build/ev/source/exe
	./ev-source-exe -p 0 -t primary -f $PATH_TO_YASTACK_ROOT/fs/config/config_2_core_ena5.ini -c /home/ubuntu/bootstrap.yaml -l trace --service-cluster cluster0 --service-node node0
	./ev-source-exe -p 1 -t secondary -f $PATH_TO_YASTACK_ROOT/fs/config/config_2_core_ena5.ini -c /home/ubuntu/bootstrap.yaml -l trace --service-cluster cluster0 --service-node node0


From an external machine, send traffic to dpdk interface
--------------------------------------------------------
	curl -k -vvv  -H "Host: yastack.app:10000" --resolve yastack.app:10000:172.31.9.84 https://yastack.app:10000/

Sample output
-------------

	ubuntu@ip-172-31-20-127:~$ curl -k -vvv  -H "Host: yastack.app:10000" --resolve yastack.app:10000:172.31.9.84 https://yastack.app:10000/
	* Added yastack.app:10000:172.31.9.84 to DNS cache
	* Hostname yastack.app was found in DNS cache
	*   Trying 172.31.9.84...
	* Connected to yastack.app (172.31.9.84) port 10000 (#0)
	* found 148 certificates in /etc/ssl/certs/ca-certificates.crt
	* found 592 certificates in /etc/ssl/certs
	* ALPN, offering http/1.1
	* SSL connection using TLS1.2 / ECDHE_RSA_AES_128_GCM_SHA256
	*        server certificate verification SKIPPED
	*        server certificate status verification SKIPPED
	*        common name: yastack.app (matched)
	*        server certificate expiration date OK
	*        server certificate activation date OK
	*        certificate public key: RSA
	*        certificate version: #3
	*        subject: C=US,ST=Denial,L=Springfield,O=Dis,CN=yastack.app
	*        start date: Thu, 22 Nov 2018 01:57:20 GMT
	*        expire date: Sat, 29 Oct 2118 01:57:20 GMT
	*        issuer: C=US,ST=Denial,L=Springfield,O=Dis,CN=yastack.app
	*        compression: NULL
	* ALPN, server did not agree to a protocol
	> GET / HTTP/1.1
	> Host: yastack.app:10000
	> User-Agent: curl/7.47.0
	> Accept: */*
	>
	< HTTP/1.1 200 OK
	< server: envoy
	< date: Thu, 22 Nov 2018 02:23:14 GMT
	< content-type: text/html
	< content-length: 612
	< last-modified: Tue, 11 Sep 2018 17:16:25 GMT
	< etag: "5b97f869-264"
	< accept-ranges: bytes
	< x-envoy-upstream-service-time: 0
	<
	<!DOCTYPE html>
	<html>
	<head>
	</head>
	<body>
	</body>
	</html>
	* Connection #0 to host yastack.app left intact

Check envoy stats
-----------------
	curl localhost:9995/stats
	curl localhost:9996/stats
