#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)
out=${1:-/tmp/wampy-display-tests}
mkdir -p "$out"
python3 "$root/tests/display/extract.py" "$root" "$out"
proto="$root/server/armv5-unknown-linux-gnueabihf"
"${PROTOC:-protoc}" -I"$proto" --cpp_out="$out" "$proto/command.proto"
${CXX:-c++} -std=c++11 -g -O1 -pthread -fsanitize=address,undefined -fno-omit-frame-pointer \
    ${PROTOBUF_CFLAGS:-$(pkg-config --cflags protobuf)} -I"$out" "$root/tests/display/replay.cpp" "$out/command.pb.cc" \
    ${PROTOBUF_LIBS:-$(pkg-config --libs protobuf)} -Wl,--wrap=nanosleep -o "$out/replay"
"$out/replay"
${CXX:-c++} -std=c++11 -Wall -Wextra -Werror -g -O1 -pthread -fsanitize=address,undefined -fno-omit-frame-pointer \
    "$root/tests/display/idle.cpp" -o "$out/idle"
"$out/idle"
