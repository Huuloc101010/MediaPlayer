# This is temp build script, this script will be refactor

#!/bin/bash

RED='\033[31m'
GREEN='\033[32m'
YELLOW='\033[33m'
RESET='\033[0m'
make -j8
ret=$?
echo "**************************************"
if((ret == 0)); then
    echo -e "**********${GREEN}Build code success${RESET}**********"
else
    echo -e "************${RED}Build code fail${RESET}***********"
fi
echo "**************************************"
