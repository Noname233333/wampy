# Build and test locally

## Apply the patch series without hardware

Use the release branch directly or apply the mail-formatted series to a fresh upstream checkout.

```sh
git clone https://github.com/unknown321/wampy.git
cd wampy
git switch --detach a954cd9d9b348756c6ba1453d9d1c12c3329809d
git switch -c nw-a55-m2-latency2
git am /path/to/wampy-m2-latency2-source.mbox
```

## Host checks

On Linux, run `sh tests/resume/run.sh` with G++ and ASan/UBSan support. That runner skips the Protobuf adapter if its optional build inputs are absent. With Python 3, `pkg-config`, `protoc`, and matching Protobuf development headers/libraries (tested with 3.21.12), run `sh tests/latency2/run.sh` for the controller replay. This compiles excerpts extracted verbatim from Git and substitutes I/O, Qt, and GPU interfaces. It neither sends commands to a device nor creates a hardware EGL surface.

## ARM build reference

Use upstream [BUILD.md](../../BUILD.md) and the [server notes](../../server/README.md) for the toolchain, source preparation and local cross-build. The previously reviewed ARM build used Linaro GCC 4.9.4, Sony's separately obtained Qt 5.3.2 source, and Protobuf 3.21.12. Acquire firmware and any required SDK or runtime inputs independently from an authorized source and keep them outside any public checkout or export. Check their individual terms before use. Keep the client's protobuf schema and plugin's generated code in sync.

Before packaging a local build, inspect the client and plugin with `readelf -h -A -d -V`. Compare architecture, float ABI, dynamic dependencies and symbol versions with the known compatible runtime. An ARM build is a new artifact even when built from the same source. This source preview does not reproduce the tested binary hashes by itself and does not qualify generic upstream firmware targets for this device.
