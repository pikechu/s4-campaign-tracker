# Phase 6A.1 public sparse campaign association build report

Date: 2026-07-16 (Asia/Shanghai)

## Decision

The Phase 6A.1 implementation and audited diagnostic candidate are **GREEN
for deployment review**, but are not deployed. This checkpoint closes only
the build-time public sparse-cache and bounded association design. A live run
must still prove the ordered click, MapInit, and same-session SU-relative
association while the database remains byte-identical.

This decision does not admit an internal menu reader, campaign marker
rendering, victory handling, persistence, database repair, or any use of a
display/save name as map identity.

## Implemented boundary

The page-21 public GUI stream is retained as a bounded page-residency cache.
Empty callback intervals do not erase it. An exact duplicate is ignored and
only `effects` may update for an existing control key; any other structural
disagreement, overflow, or page-admission disagreement fails closed.

A unique exact labeled control may arm a 30-second launch lease. The lease
survives an invalid click redraw and the page-21 to briefing transition until
MapInit. Another page-21 click, abandoned re-entry, timeout, ineligible origin,
wrong session, empty `identity.relative`, or shutdown clears it. Only the exact
same-session confirmed `identity.relative` can be emitted.

## GREEN checkpoint

Commit `beb2d9d88d49b17f7a640aa74d4c7c03d1a0e123` is the complete checkpoint.
Before push, the changed production and test translation units compiled for
Win32 with strict warnings, the source-only read-only policy passed, and
`git diff --check` passed.

Authoritative Windows Release workflow run `29478420685`, job `87556431317`,
completed successfully for that exact head. Archive integration, Win32
configure/build with `/W4 /WX`, binary and source policy, all mutation
fixtures, tests, package construction, PE32/i386 and export verification, and
artifact upload all passed.

## Audited candidate

The independently downloaded artifact is frozen as:

- artifact ID `8367516201`, `CampaignCompletionDebug-Win32`, 236232 bytes;
- GitHub digest and downloaded outer SHA-256
  `9eaac55a64aceb91bdb58a49984629d5c673775e671db2137c5861f7f2eb096e`;
- embedded `CampaignCompletionDebug.zip`, 236199 bytes, SHA-256
  `ebc49ae4db5ee234f3c1bea8b299f5c1bf58b55bc20a9fd7d9777466ce9e93b9`;
- `Plugins/CampaignCompletionDebug.asi`, 420864 bytes, SHA-256
  `75f63da915be89373902262d3ff34b635c4a177443c3bb8844852fa853f1f637`;
- `Plugins/CampaignCompletion/CampaignCompletionDebug.ini`, 895 bytes,
  SHA-256
  `26667d7af68b4919c65232dcd90989c087d9886f3e80a049d2e53746eebca0ec`.

The inner package contains exactly those two entries. Independent inspection
identifies the ASI as PE32/i386 with the single
`CampaignCompletionDebugStop` export. The packaged INI equals the committed
INI after normalizing the Windows checkout's CRLF line endings.

## Remaining gate

The candidate has not replaced the Settlers United archive. Deployment
requires both protected applications to be closed and a fresh, explicit
approval naming this Phase 6A.1 candidate. Live acceptance remains limited to
the association subproblem; the no-hover full campaign catalog remains an
evidence gap even if the association succeeds.
