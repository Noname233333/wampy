#!/bin/sh
# Host-only replay. Keep generated source and binaries outside the checkout.
set -eu
repo=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
revision=${1:-HEAD}
mode=${2:-}
case "$mode" in ""|expect-regression) ;; *) exit 2 ;; esac
proto_dir="$repo/server/armv5-unknown-linux-gnueabihf"
"${PROTOC:-protoc}" --proto_path="$proto_dir" --cpp_out="$tmp" "$proto_dir/command.proto"
python3 "$repo/tests/latency2/generate_replay.py" "$repo" "$revision" "$tmp"
cflags=$(pkg-config --cflags protobuf)
libs=$(pkg-config --libs protobuf)
# pkg-config supplies compiler and linker argument lists.
# shellcheck disable=SC2086
"${CXX:-g++}" -std=c++11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer \
    $cflags -I"$tmp" "$repo/tests/latency2/replay.cpp" "$tmp/command.pb.cc" $libs -o "$tmp/replay"
if [ -n "$mode" ]; then
    ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=0} "$tmp/replay" "$mode"
else
    ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=0} "$tmp/replay"
fi
