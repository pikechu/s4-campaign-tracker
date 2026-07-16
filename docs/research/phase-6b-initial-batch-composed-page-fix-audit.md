# Phase 6B initial batch and composed-page fix audit

Date: 2026-07-16 (Asia/Shanghai)

## Decision

The first all-campaign catalog pass accepted the public sparse capture and
page isolation for pages 11 through 21, with zero invalid snapshots. It exposed
an evidence gap on selectors 3/4 and the shared New World page set 5/6: those
screens draw companion public layers, and candidate `0.8.0` cleared its single
page cache when a non-campaign companion frame arrived.

The focused `0.8.1` composed-page fix is GREEN for deployment review. It has not
been deployed. The running `0.8.0` process remains read-only, and final database
postflight is pending normal user-controlled shutdown.

## Initial live slice

- installed candidate: `0.8.0`, audited ASI SHA-256
  `3eeac2946731e368a66739bc7f382580fccc57f5d5985d472c2fe29973428c54`;
- frozen log range: offsets `[12686712,12741849)`;
- range length: 55137 bytes;
- range SHA-256:
  `60d2c47a785c91f237d9c080458d6f6f86df7eaed3bee47175ab70ba66acfb93`;
- exact executable compatibility: accepted;
- runtime boundary: storage, native events, markers, and internal menu adapter
  disabled.

The batch emitted no campaign click or identity association because the user
correctly performed only the catalog pass.

| Page | Successful snapshots | Maximum retained features | Invalid |
| ---: | ---: | ---: | ---: |
| 11 | 6 | 2 | 0 |
| 12 | 2 | 1 | 0 |
| 13 | 3 | 2 | 0 |
| 14 | 2 | 1 | 0 |
| 15 | 2 | 1 | 0 |
| 16 | 5 | 3 | 0 |
| 17 | 2 | 1 | 0 |
| 18 | 3 | 2 | 0 |
| 19 | 3 | 1 | 0 |
| 20 | 4 | 3 | 0 |
| 21 | 4 | 4 | 0 |

Observed mission controls include Add-on Trojan `91`, Mayan `34`, Mission CD
Roman `1903/1904`, Mission CD Mayan `1889`, original `2039/2040`, and Dark
Tribe `2083/2084/2085`. Sparse absence on other entries remains absence, not an
inferred mapping.

Selectors 3/4 and pages 5/6 were visibly active but emitted no catalog
snapshot. Page histories proved the composition:

- Add-on selector: page set includes `3,24`;
- Mission CD selector: page set includes `4,22,23,25`;
- New World screens: page set stabilizes as `5,6`.

## Focused fix

Commit `de76e04e190f4cdc7296dd4fde8c39e04ec33f6b` introduces one immutable sorted
campaign page list and selects the highest active admitted page as the stable
catalog owner. Consequences:

- page 3 survives companion page 24;
- page 4 survives companion pages 22/23/25;
- the simultaneous 5/6 set uses deterministic owner 6;
- child pages 11 through 21 outrank their selector during transitions;
- leaving all admitted campaign pages still clears the cache;
- changing owner still isolates snapshots and clears an unbound cross-page
  click candidate.

The 5/6 owner is an observation key, not map identity. New World versus New
World 2 must still be distinguished by a later same-session confirmed
`identity.relative`, never by label or save name.

## Authoritative CI and artifact

- workflow run: `29484738722`;
- job: `87576414825`;
- result: success;
- artifact ID: `8369955051`;
- service digest:
  `sha256:4680b8967918f0b64b9c8c8e3da757eb712c4736b293a21bdffcb30910a3a508`;
- downloaded ZIP SHA-256:
  `38b7990299d5163541b14ee2b566db94ef5a3eaed54da41b0d4bcda648992f0f`;
- ASI: 421888 bytes, SHA-256
  `19293da03095ae9f4179778509a39de5291060afc92578caf49500f8186e9886`;
- INI: 938 bytes, SHA-256
  `bdab7dc5a685ad04c45e1af48c545b18a5f7a0a8bda81f88306c57b2e651af95`.

All archive integration, Win32 Release build, policy, mutation, CTest,
packaging, PE32, export, and package checks passed. The ASI has exactly one
`CampaignCompletionDebugStop` export, and the packaged INI is source-identical
after CRLF normalization.

## Remaining live work

After normal shutdown and zero-write postflight, deployment of `0.8.1` requires
a fresh explicit approval. The follow-up catalog pass needs only selectors 3/4
and the two visits to the 5/6 composite; accepted pages 11 through 21 do not
need to be repeated. No mission launch is authorized by this audit.
