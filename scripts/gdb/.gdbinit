#set $fs=/home/ubuntu/falcon/fs
#set $dpdk=/home/ubuntu/falcon/dpdk

define break_ff_eth
    break ff_veth_attach
    break ff_veth_config
    break ff_veth_detach
    break ff_veth_init
    break ff_veth_input
    break ff_veth_ioctl
    break ff_veth_process_packet
    break ff_veth_qflush
    break ff_veth_set_gateway
    break ff_veth_setaddr
    break ff_veth_setup_interface
    break ff_veth_softc
    break ff_veth_start
    break ff_veth_stop
    break ff_veth_transmit
end

define break_ena_debug
    break ff_dpdk_init
    break ena_start
    break ena_com_set_hash_function
    break ena_com_fill_hash_function
    break ena_com_set_hash_function
    break ena_rss_init_default
    break ena_com_indirect_table_set
    break ena_com_set_hash_function
    break ena_com_set_hash_ctrl
    break rte_eth_dev_configure
    break ff_rss_check
    break ena_com_get_feature_ex
    break ena_com_check_supported_feature_id
    break ena_com_get_dev_attr_feat
end

directory /home/ubuntu/falcon/fs/tools/netstat:/home/ubuntu/falcon/fs/tools:/home/ubuntu/falcon/fs:/home/ubuntu/falcon/dpdk:/home/ubuntu/falcon/dpdk/lib:/home/ubuntu/falcon/fs/example:/home/ubuntu/falcon/fs/lib:/home/ubuntu/falcon/fs/app:/home/ubuntu/falcon/fs/app/libevent:/home/ubuntu/falcon/fs/app/libevent/sample

# source shared_ptr.gdb
