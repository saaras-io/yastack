#!/usr/bin/env bash

docker build -f Dockerfile-yastack-image \
--build-arg DPDK_VER=18.05 \
--build-arg YASTACK_BASE_IMAGE=yastack/ubuntu \
--build-arg BASE_VER=18.04 \
-t ubuntu/yastack:0.1alpha .
