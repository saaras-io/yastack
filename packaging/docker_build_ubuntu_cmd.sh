#!/usr/bin/env bash

OS_VER=18.04
docker build -f ./Dockerfile-yastack-ubuntu \
--build-arg OS_VER=${OS_VER} \
-t yastack/ubuntu:${OS_VER} .
