# This is temp build script, this script will be refactor

#!/bin/bash
g++ \
main.cpp \
player.cpp \
--std=c++20 `pkg-config --cflags --libs libavformat libavcodec libavutil` \
-o a.out
echo "Build code success"