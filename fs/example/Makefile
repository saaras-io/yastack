TOPDIR=..

ifeq ($(FF_PATH),)
	FF_PATH=${TOPDIR}
endif

LIBS+= -Wl,--whole-archive -lrte_eal -lrte_pmd_vmxnet3_uio -lrte_pmd_i40e -lrte_pmd_ixgbe -lrte_pmd_e1000 -lrte_pmd_ring
LIBS+= -Wl,--whole-archive -lrte_hash -lrte_kvargs -Wl,-lrte_mbuf -lrte_ethdev -lrte_pci -lrte_bus_pci -lrte_bus_vdev -lrte_mempool
LIBS+= -L${FF_PATH}/lib -L${DPDK_LIBS} -Wl,--whole-archive,-lfstack,--no-whole-archive
LIBS+= -Wl,--whole-archive -lrte_ring -lrte_cmdline -lrte_cfgfile -lrte_net -lrte_mempool_ring -lrte_kni -lrte_timer -Wl,-lrte_pmd_virtio -lrte_pmd_ena -Wl,--no-whole-archive
LIBS+= -Wl,--no-whole-archive -lnuma -lrt -lm -ldl -lcrypto -pthread


TARGET="helloworld"
all:
	cc -O0 -gdwarf-2 -g3 -I${FF_LIB_INCLUDES} -L${FF_LIBS} -L${OPENSSL_LIBS} -o ${TARGET} main.c ${LIBS}
	cc -O0 -gdwarf-2 -g3 -I${FF_LIB_INCLUDES} -L${FF_LIBS} -L${OPENSSL_LIBS} -o ${TARGET}_epoll main_epoll.c ${LIBS}

.PHONY: clean
clean:
	rm -f *.o ${TARGET} ${TARGET}_epoll
