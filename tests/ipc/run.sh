#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)
out=${1:-/tmp/wampy-ipc-tests}
mkdir -p "$out"
${CXX:-c++} -std=c++11 -Wall -Wextra -Werror -O1 -g -pthread -fsanitize=address,undefined -fno-omit-frame-pointer \
    "$root/tests/ipc/socket.cpp" -o "$out/socket"
"$out/socket"
