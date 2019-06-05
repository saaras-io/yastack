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
kmod \
pciutils \
vim \
&& rm -rf /var/lib/apt/lists/* 

# Install cmake
RUN wget https://github.com/Kitware/CMake/releases/download/v3.14.2/cmake-3.14.2.tar.gz
RUN tar -xvzf cmake-3.14.2.tar.gz
RUN cd cmake-3.14.2/ && ./bootstrap && make && make install

# Copy static binaries protoc and protoc-gen-validate
COPY protoc-gen-validate /bin 

# DPDK uses kernel headers to build igb_uio
# Compiling YAStack on host system ensures correct drivers are built
# Download yastack and compile it
RUN cd / && git clone https://github.com/saaras-io/yastack.git
RUN cd /yastack/ev/external/protobuf-3.5.0 && ./autogen.sh && ./configure && make && make check && make install && ldconfig
RUN cd /yastack && mkdir -p build && cd /yastack/build && cmake ..
RUN cd /yastack/build/ && make -s
