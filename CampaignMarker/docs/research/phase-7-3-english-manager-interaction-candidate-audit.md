# Phase 7.3 English manager interaction candidate audit

Date: 2026-07-18

## Result

Phase 7.3 is **GREEN for the audited offline `0.13.3` candidate**.

Live acceptance of deployed `0.13.2` confirmed:

- correct text rendering
- the classified tree
- the right-hand mission/map list
- Refresh, Apply, and Close controls

The user requested an English-only interface and the public label
`Great Crusades` in place of `New World 2`. Read-only Win32 inspection also
proved that Apply was enabled. With no pending checkbox change, Apply returned
`Unchanged`, but the following refresh immediately replaced that result with
normal guidance, making the click appear ineffective.

The `0.13.3` candidate:

- converts every completion-manager UI string to English
- labels the immutable New World 2 descriptor group as
  `Campaigns / Great Crusades`
- enables Apply only after an editable checkbox actually changes
- preserves the apply result after refreshing the view

It does not change descriptor groups, map identities, completion
transactions, persistence, marker rendering, or RD classification.

## Source checkpoint

- commit: `4c6467f494cc66c6ad050ce986ab9ffb5cdf7535`
- version: `0.13.3`

## Live read-only evidence

External Win32 inspection of the deployed `0.13.2` manager reported:

- tree ID `7001`: enabled and visible
- list ID `7002`: enabled and visible
- Refresh ID `7003`: enabled and visible
- Apply ID `7004`: enabled and visible
- Close ID `7005`: enabled and visible

The installed fixed-map directories contained 44 single-player, 53
multiplayer-list, and 157 custom maps. All three exact roots existed, and the
combined count and bounded string units were below the discovery limits.

## Local validation

- Win32 MinGW compile with project warnings as errors:
  `CompletionManagerWindow.cpp`, `CompletionManagerCatalog.cpp`,
  `DiagnosticRuntime.cpp`, and `RuntimePolicyTests.cpp`: success
- the third-party S4ModApi `uuid` attribute warning was the only local warning
  and was not promoted to an error
- completion-manager window and catalog UI sources are English-only ASCII
- generated Win32 object contains exact UTF-16LE English title and button text
- source-only zero-process-patch and read-only-Lua verification: success
- `git diff --check`: success

## Authoritative Windows CI

- workflow run: `29645683732`
- job: `88083422329`
- commit: `4c6467f494cc66c6ad050ce986ab9ffb5cdf7535`

All workflow steps succeeded:

- Settlers United archive integration
- Win32 Release configure and build
- complete PE/source policy verification
- all 11 forbidden-behavior mutation fixtures
- full CTest suite
- package
- PE32 and exact package-layout verification
- artifact upload

## Artifact audit

- artifact ID: `8429981159`
- GitHub artifact size: `313925`
- outer artifact SHA-256:
  `900e3c39c486ee1fc9741db9086b34c65de763a9e2509f2b8b4bc0947df1403b`
- inner deployment ZIP SHA-256:
  `9e7d06cef34445d996e3c02ee35921081e12b3ef355c59e982043f9750f7f740`
- ASI SHA-256:
  `b50666194524a39732095b23ca4721c387acfab354d9c99282a102b6e0a41e02`
- packaged INI SHA-256:
  `aed0c44b8428f84ee52a13cdee89fecd9c857feedf249b73f2acaec336f9ea88`

The artifact contains exactly one inner deployment ZIP. That ZIP contains
exactly:

- `Plugins/CampaignCompletionDebug.asi`
- `Plugins/CampaignCompletion/CampaignCompletionDebug.ini`

The ASI is PE32 x86 and exports exactly
`CampaignCompletionDebugStop`. The packaged INI is source-equivalent after
line-ending normalization and declares `Version=0.13.3`.

Binary inspection found exact UTF-16LE forms of:

- `Campaign and Map Completion Manager`
- `Refresh`
- `Apply`
- `Close`
- `Campaigns / Great Crusades`
- the normal checkbox guidance
- the successful atomic-save result

The former Chinese title/button strings and
`Campaigns / New World 2` are absent.

## Deployment boundary

This audit does not deploy `0.13.3`. Deployment requires both `S4_Main` and
Settlers United to be closed normally by the user and a fresh explicit
approval. A guarded combined-archive replacement may change only
`Plugins/CampaignCompletionDebug.asi`; `PileChainRepair.asi` and every other
entry must remain byte-for-byte identical.

RD handling remains fail-closed. Only exact same-session
`identity.relative` is admitted. An exact `RD_` relative remains random,
marker-hidden, non-editable, and is never displayed or named by the manager.
