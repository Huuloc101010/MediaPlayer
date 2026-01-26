# This is temp build script, this script will be refactor

#!/bin/bash
#g++ player.cpp \
#main.cpp \
#player.cpp \
#--std=c++20 `pkg-config --cflags --libs libavformat libavcodec libavutil` -o a.out
g++ --std=c++20 player.cpp \
    `pkg-config --cflags --libs libavformat libavcodec libavutil` \
    -o a.out
echo "Build code success"