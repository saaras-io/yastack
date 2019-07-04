#!/usr/bin/env bash

docker run --network host -i \
	--privileged --cap-add=ALL \
	-v /dev:/dev \
	-v /lib/modules:/lib/modules \
	-t ubuntu/yastack:0.1alpha /bin/bash
