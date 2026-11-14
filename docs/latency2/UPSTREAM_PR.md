# Upstream PR draft

**Title** — Preserve the native display handoff across HOLD entry and display resume on NW-A55

**Target** — `unknown321/wampy:master`, based on v1.14.6 (`a954cd9`). Compare against the current base again before opening. Use the runtime-only branch for the PR. The release branch adds downstream documentation and a host replay.

## Problem

On a Mainland China NW-A55 running Walkman One 23.09, a faster downstream handoff experiment could leave the native player visible after first enabling HOLD. Native playback and touch could continue. An owner subsequently reported successful use of the recovery build that restored the prior timing.

## Changes

- Have the client wait for the current generation's native visibility and render-thread quiescence before drawing.
- Preserve HOLD intent across display power transitions, with finite socket deadlines and bounded retries.
- Keep explicit native-window restoration and conservative timing, including a fresh status query after Hide.
- Include the existing regression tests. The full historical runtime branch also contains clock-resource hardening and build changes. These can be reviewed as separate commits or split further at the maintainer's request.

## Validation

The host suite exercised disabled/unknown display, retained intent, stale replies, retries, USB export and native takeover. A replay with substitute Qt/GPU interfaces reproduced a first-entry failure when Hide ran before simulated GPU readiness; this is a plausible explanation, not a measured cause on hardware. The owner report covers one Mainland China NW-A55 with Walkman One 23.09. Other models and firmware combinations remain unverified.

## Review questions

- Does the maintainer prefer the native handoff, clock hardening and build changes as separate PRs?
- Is the downstream generation/visibility handshake suitable for the supported Qt backends?
- Are there device variants where the native compositor should use a different ownership check?

Source and tests only. The PR should not include downstream firmware packaging, raw device logs, or build products.
