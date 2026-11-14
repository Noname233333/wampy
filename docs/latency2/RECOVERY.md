# Recovery

Keep a private copy of your known-good client and matching Qt plugin, your configuration, and your model-specific recovery materials before considering a device update. The client and EGLFS plugin form a pair. Restoring one file from a different build can break the display handoff.

The previously tested Latency2 installation used the byte-identical IdentityFix1 client and plugin. In private offline tests, the paired recovery restored those exact bytes after a partial two-file commit. Independent recovery from a healthy, complete pair and refusal of unrecognized files were also tested offline. The public source release contains no recovery image.

If a local update fails while the Walkman can still enter its approved update path, use the exact recovery paired with that local build and confirm both restored hashes. Preserve logs privately and record the final device-side status. If the device cannot boot or receive updates, consult the exact model's recovery procedure. A WBRT stock backup returns the device to the saved state, which can predate Walkman One and Wampy. Keep backups offline. Never use another unit's backup.
