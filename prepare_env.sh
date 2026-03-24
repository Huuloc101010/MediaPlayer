#!/bin/bash

sudo -v
sudo apt update
sudo apt install -y \
make \
g++-13 \
pkg-config \
libsdl2-dev \
libsdl2-image-dev \
ffmpeg \
libavcodec-dev \
libavformat-dev \
libavutil-dev \
libswscale-dev \
libswresample-dev \


echo "prepare environment success"
