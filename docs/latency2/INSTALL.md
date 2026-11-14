# Installation scope

This is a **source-only pre-release**. Apply the patch to a local clone as shown in the [build guide](BUILD.md). The source archive, patches and checksums contain no image that can be installed through the Walkman updater.

The owner-tested private migration was for the Mainland China NW-A55 with Walkman One 23.09 and the matching Latency1 client and plugin. It verified both input hashes and used a paired transaction. The published source is useful for review and local development. Builds from it and changes to any updater require their own verification before device use.

If you independently have a locally vetted, compatible update and matching recovery, keep your existing firmware recovery and music/config backups offline. Confirm the exact model, region, firmware, starting files, and checksums before using any updater. Use the updater's non-writing preflight when available, maintain power during updates, and confirm the device-side result after reboot. A Windows request alone does not establish that the device committed an update. Test native playback, first HOLD entry/exit, then POWER resume. Flashing an incompatible image or losing power during an update can leave the player unbootable.
