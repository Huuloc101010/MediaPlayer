#!/bin/bash

sudo -v
sudo apt update
sudo apt install -y \
build-essential \
pkg-config \
libsdl2-dev \
ffmpeg \
libavcodec-dev \
libavformat-dev \
libavutil-dev \
libswscale-dev \
libswresample-dev \


echo "prepare environment success"
