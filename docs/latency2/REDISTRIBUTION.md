# Licensing and release scope

The root [LICENSE](../../LICENSE) is byte-identical to v1.14.6. It names unknown321 as licensor and includes GNU GPLv3 and the Commons Clause v1.0 condition. Retain both texts and existing file-level notices. Use **source-available** in public descriptions because the Commons Clause restricts selling and the [Open Source Definition](https://opensource.org/osd) requires free redistribution. This branch conveys no new license exception or permission to sell software or related services. Component-specific grants and notices should be reviewed individually. GNU GPLv3 section 7 addresses further restrictions; if maintainers or contributors' terms appear to conflict, seek clarification before making broader licensing claims. See the [Commons Clause text and FAQ](https://commonsclause.com/) and [GNU GPLv3](https://www.gnu.org/licenses/gpl-3.0.html).

| Material | Public source preview |
| --- | --- |
| Wampy runtime patches and tests | Source only. Preserves upstream attribution, license text, and pinned submodule references. |
| Upstream documentation and media already present in the original fork | Inherited unchanged except where explicitly documented. The standalone patch attachment does not rebundle upstream media. No additional permission is asserted for those individual assets. |
| Sony firmware and extracted system images, Hgrm, system Qt binaries, Mali GPU userspace libraries or runtime sysroots | Excluded. This work did not establish independent public redistribution rights for those payloads. |
| Sony-published Qt source, other source dependencies, NSIS and other tooling | Refer to official originals. Their specific terms apply independently. No vendor archive, executable, or generated SDK is included. |
| Private UPG/EXE installers and recovery archives, patched runtime binaries and test objects | Excluded. Any future binary release needs a separate component inventory, authorization and corresponding-source review, including applicable notices and installation information. |
| WBRT backups, configuration/NVP dumps, raw logs and screenshots | Excluded. Use a short factual report with model/firmware and synthetic test results. Do not upload unique serials, storage identifiers, local paths or account details. |

The GitHub tag's automatically generated source archives contain files tracked on the reviewed branch. They should be checked independently from manually attached assets.
