# This is temp build script, this script will be refactor

#!/bin/bash

RED='\033[31m'
GREEN='\033[32m'
YELLOW='\033[33m'
RESET='\033[0m'

g++-13 \
./src/*.cpp \
--std=c++20 `pkg-config --cflags --libs libavformat libavcodec libavutil libswresample` \
-g -o0 \
-I./inc \
`pkg-config --cflags --libs sdl2` \
-o ./a.out
ret=$?
echo "**************************************"
if((ret == 0)); then
    echo -e "**********${GREEN}Build code success${RESET}**********"
else
    echo -e "************${RED}Build code fail${RESET}***********"
fi
echo "**************************************"
