# Changelog

## M2 Latency2 source preview

- Restore IdentityFix1 runtime behavior after Latency1 could leave the Sony player on screen on the first HOLD transition.
- Preserve the original window visibility, GPU quiescence and generation checks, including the post-hide native status query. Retain conservative retry timing.
- The private migration distributed for owner testing accepted the exact Latency1 client/plugin pair and installed the byte-identical IdentityFix1 pair. Its standalone recovery restored that same IdentityFix1 pair. This public preview distributes the runtime source and regression tests.
- Add an English build and recovery guide, a compatibility statement and a repeatable host replay.

## Earlier development

- **IdentityFix1** is the previously tested runtime baseline for Latency2. Its changes against upstream include a bounded native-display handoff, POWER/HOLD handling, finite IPC and animation sleep/wake. The inherited branch also includes separate clock-resource hardening and build/test changes.
- **Latency1** accelerated successful handoff transitions for both initial HOLD entry and resume. An owner reported that HOLD no longer brought Wampy to the foreground on the test unit after the upgrade.

## Revisions

| Revision | Meaning |
| --- | --- |
| `a954cd9d9b348756c6ba1453d9d1c12c3329809d` | Upstream Wampy v1.14.6 |
| `0c218343f9dee4ff12eb8533dcb9746eb5ada299` | IdentityFix1 runtime used by the private Latency2 package |
| `472b799d398ddaff9016e6a59c530bfcee5280aa` | Historical Latency1 experiment, outside this branch |
