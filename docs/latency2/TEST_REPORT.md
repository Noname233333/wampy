# Test report

## Real device

The owner reported that the privately installed M2 Latency2 worked on one Mainland China Sony NW-A55 with Walkman One 23.09 and the Wampy v1.14.6 base, following a first-HOLD regression in Latency1. The owner also described it as feeling faster. This is a qualitative observation. No frame timing, power measurement, or independent hardware reproduction was supplied. The earlier IdentityFix1 check on that unit included first HOLD entry/exit, audio and physical controls, and POWER resume after 10 and 60 seconds. Those detailed checks are recorded for the previous baseline. The deployed Latency2 runtime files were byte-identical to IdentityFix1. A newly compiled client, other regional variants, and all other device or firmware combinations remain unverified.

## Offline regression review

| Check | Outcome and limits |
| --- | --- |
| Host resume suite, ASan/UBSan | Covers display and intent, timeout, native takeover, socketpair I/O, simulated backlight, worker gating, fonts and UTF-8. |
| Host replay, ASan/UBSan | Executes verbatim excerpts from the production input callback, client loop and Qt controller with substitute I/O and GPU/Qt interfaces. Tests first HOLD, exit, repeated HOLD, 10/60-second resume, slow Hide, stale acknowledgment and visible native window. |
| Historical ARM QEMU replay | Same virtual first-entry result as the host; the GPU and panel are simulated. |
| Historical ARM ABI check | The previously deployed IdentityFix1 runtime pair was ELF32 ARMv7 EABI5 hard-float, with no missing dependencies or unresolved strong symbols in the reviewed stock and Walkman One library sets. Qt 5.3.2 loaded the plugin under QEMU without opening an EGL window. |
| Historical recovery and host packaging checks | 13 fixture transactions with exact input/output hashes, including a partial commit and rollback. 40 PowerShell cases with mock Windows/USB behavior; separate NW-A55 allow and NW-A56/NW-A57 refusal fixtures. |

The same virtual GPU-ready threshold (80 ms) reproduced first HOLD failure with Latency1 and success with IdentityFix1. This supports a plausible timing mechanism: an early Hide can lose the GPU drain signal, leaving the native interface active after fallback. The 80 ms is a test fixture, and the device-side cause remains unconfirmed. This branch restores conservative behavior. It does not claim to eliminate the wake flash or reduce measured wake time.

The release kit provides a sanitized host-run summary. Historical checks are summarized from private artifacts. Those archives, raw hardware logs, build paths and screenshots are omitted.
