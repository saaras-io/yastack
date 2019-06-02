ARG DPDK_VERSION=18.05
ARG OS_VER=16.04
FROM ubuntu:${OS_VER}

WORKDIR /

# Install prerequisite packages
RUN apt-get update && apt-get install -y \
libnuma-dev \
autoconf \
linux-headers-$(uname -r) \
make \
automake \
g++ \
ethtool \
net-tools \
wget \
unzip \
tar \
tree \
git \
libunwind8 \
apt-transport-https \
libtool \
gawk \
libc-ares-dev \
python \
libz-dev \
golang-go \
curl \
&& rm -rf /var/lib/apt/lists/* 

# Install cmake
RUN wget https://github.com/Kitware/CMake/releases/download/v3.14.2/cmake-3.14.2.tar.gz
RUN tar -xvzf cmake-3.14.2.tar.gz
RUN cd cmake-3.14.2/ && ./bootstrap && make && make install

# Copy static binaries protoc and protoc-gen-validate
COPY protoc /bin
COPY protoc-gen-validate /bin 

# DPDK uses kernel headers to build igb_uio
# Compiling YAStack on host system ensures correct drivers are built
# Download yastack and compile it
RUN cd / && git clone https://github.com/saaras-io/yastack.git
RUN cd /yastack/ev/external/protobuf-3.5.0 && ./autogen.sh && ./configure && make && make check && make install && ldconfig
RUN cd /yastack && mkdir -p build && cd /yastack/build && cmake ..
#RUN cd /yastack/build/ && make -s








































# Download and compile DPDK
#ARG DPDK_VER
#RUN cd /usr/src/ &&  wget http://git.dpdk.org/dpdk/snapshot/dpdk-${DPDK_VER}.zip && unzip dpdk-${DPDK_VER}.zip 
#ENV DPDK_DIR=/usr/src/dpdk-${DPDK_VER}  DPDK_TARGET=x86_64-native-linuxapp-gcc DPDK_BUILD=$DPDK_DIR/$DPDK_TARGET
#
## Enable compilation of Amazon ENA PMD driver
#RUN cd $DPDK_DIR && sed -i 's/\(CONFIG_RTE_LIBRTE_ENA_PMD=\)n/\1y/g' $DPDK_DIR/config/common_base
#RUN cd $DPDK_DIR && sed -i 's/\(CONFIG_RTE_LIBRTE_ENA_DEBUG_RX=\)n/\1y/g' $DPDK_DIR/config/common_base
#RUN cd $DPDK_DIR && sed -i 's/\(CONFIG_RTE_LIBRTE_ENA_DEBUG_TX=\)n/\1y/g' $DPDK_DIR/config/common_base
#RUN cd $DPDK_DIR && sed -i 's/\(CONFIG_RTE_LIBRTE_ENA_DEBUG_TX_FREE=\)n/\1y/g' $DPDK_DIR/config/common_base
#RUN cd $DPDK_DIR && sed -i 's/\(CONFIG_RTE_LIBRTE_ENA_DEBUG_DRIVER=\)n/\1y/g' $DPDK_DIR/config/common_base
#RUN cd $DPDK_DIR && sed -i 's/\(CONFIG_RTE_LIBRTE_ENA_COM_DEBUG=\)n/\1y/g' $DPDK_DIR/config/common_base
#
## Enable compilation of KNI
#RUN cd $DPDK_DIR && sed -i 's/\(CONFIG_RTE_LIBRTE_KNI=\)n/\1y/g' $DPDK_DIR/config/common_base
#RUN cd $DPDK_DIR && sed -i 's/\(CONFIG_RTE_LIBRTE_PMD_KNI=\)n/\1y/g' $DPDK_DIR/config/common_base
#RUN cd $DPDK_DIR && sed -i 's/\(CONFIG_RTE_KNI_KMOD=\)n/\1y/g' $DPDK_DIR/config/common_base
#RUN cd $DPDK_DIR && sed -i 's/\(CONFIG_RTE_KNI_KMOD_ETHTOOL=\)n/\1y/g' $DPDK_DIR/config/common_base
#RUN cd $DPDK_DIR && sed -i 's/\(CONFIG_RTE_KNI_PREEMPT_DEFAULT=\)n/\1y/g' $DPDK_DIR/config/common_base
#
## Uncomment to compile ixgbe driver
##RUN cd $DPDK_DIR && sed -i 's/\(CONFIG_RTE_LIBRTE_IXGBE_PMD=\)n/\1y/g' $DPDK_DIR/config/common_base
#
#RUN cd $DPDK_DIR && make install T=$DPDK_TARGET DESTDIR=install


#RUN wget https://github.com/Kitware/CMake/releases/download/v3.14.5/cmake-3.14.5-Linux-x86_64.tar.gz
#RUN tar -xzf cmake-3.14.5-Linux-x86_64.tar.gz
#RUN cp cmake-3.14.5-Linux-x86_64/bin/* /bin
