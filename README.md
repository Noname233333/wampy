# Wampy M2 Latency2 for the Sony NW-A55

An unofficial source-available fork of [unknown321/wampy v1.14.6](https://github.com/unknown321/wampy/tree/v1.14.6) for testing HOLD and display resume on the Sony NW-A55. Wampy is an interface add-on for Walkman players. See the [original upstream README](UPSTREAM_README.md) for the original project.

Latency2 restores the IdentityFix1 client and matching Qt/EGLFS handoff after Latency1 introduced a first-entry regression. It retains the conservative handoff timing used by IdentityFix1. The Sony interface can briefly appear during wake. Reports of faster wake on the tested unit were subjective and have no measured timing.

| Hardware and software | Evidence |
| --- | --- |
| Mainland China Sony NW-A55, Walkman One 23.09, Wampy v1.14.6 base | The owner reported successful use of the private Latency2 build. See the [test report](docs/latency2/TEST_REPORT.md) for the scope of the observation. |
| Other NW-A55 regional variants, stock firmware, other Walkman One releases, and every other model | Unverified for this fork. |

This pre-release provides source and patches. The GitHub source ZIP and the patch archive are developer inputs. Flashing a locally prepared update can cause boot failure or data loss. Review the [installation guide](docs/latency2/INSTALL.md) and [recovery guide](docs/latency2/RECOVERY.md) before any device work.

- [Changes and version provenance](CHANGELOG.md)
- [Local build and host testing](docs/latency2/BUILD.md)
- [Licensing and redistribution scope](docs/latency2/REDISTRIBUTION.md)
- [Proposed upstream PR](docs/latency2/UPSTREAM_PR.md)

The upstream [LICENSE](LICENSE) is retained verbatim. Wampy is distributed by its upstream author under GNU GPLv3 with the Commons Clause condition. Component-specific notices remain in the source and applicable dependencies. Sony, Walkman, and Walkman One identify the target platform. This project is unofficial.
