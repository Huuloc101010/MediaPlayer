# This is temp build script, this script will be refactor

#!/bin/bash
g++ \
main.cpp \
player.cpp \
videooutput.cpp \
audiooutput.cpp \
--std=c++20 `pkg-config --cflags --libs libavformat libavcodec libavutil libswresample` \
`pkg-config --cflags --libs sdl2` \
-o a.out
echo "Build code success"