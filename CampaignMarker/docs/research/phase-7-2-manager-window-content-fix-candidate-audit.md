# Phase 7.2 manager-window content fix candidate audit

Date: 2026-07-18

## Result

Phase 7.2 is **GREEN for the audited offline `0.13.2` candidate**.

Live acceptance of deployed `0.13.1` proved that `Ctrl+Shift+M` now opens the
manager from the main menu. The opened window exposed two presentation
defects:

- Chinese window text was corrupted because the authoritative MSVC build did
  not explicitly decode UTF-8 source files as UTF-8.
- The window contained no controls because `WM_CREATE` ran before the return
  value of `CreateWindowExW` had been assigned to the `window_` member, while
  child controls used that still-null member as their parent.

The `0.13.2` candidate adds the C++-only MSVC `/utf-8` option and binds the
actual `HWND` at the start of `WM_CREATE`, before creating controls. It does
not change catalog construction, completion identity, persistence,
transaction, marker rendering, or RD handling.

## Source checkpoint

- feature commit: `66eb6a806ea4173c8cbc6f058b5e0c1d8e77bf6b`
- newline-agnostic regression-test commit:
  `1171fa6ddf747a30b9201bfea1d0f412956edd30`
- version: `0.13.2`

The first authoritative run correctly rejected an exact-LF structural test on
Windows CRLF checkout:

- failed run: `29644804844`
- job: `88081158843`
- product build and all policy/mutation steps before CTest: success
- failure: the test searched for a literal LF sequence

The test was corrected to enforce the ordered tokens
`WM_CREATE -> window_ = window -> CreateControls()` without depending on line
endings. No product code was weakened to make the test pass.

## Local validation

- Win32 MinGW compile with project warnings as errors:
  `CompletionManagerWindow.cpp`, `DiagnosticRuntime.cpp`, and
  `RuntimePolicyTests.cpp`: success
- the third-party S4ModApi `uuid` attribute warning was the only warning
  locally and was not promoted to an error
- generated Win32 object contains exact UTF-16LE forms of the Chinese title
  and button labels
- source-only zero-process-patch and read-only-Lua verification: success
- `git diff --check`: success

## Authoritative Windows CI

- workflow run: `29644920664`
- job: `88081453719`
- commit: `1171fa6ddf747a30b9201bfea1d0f412956edd30`

All steps succeeded:

- Settlers United archive integration tests
- Win32 Release configure and build
- complete PE/source policy verification
- all 11 forbidden-behavior mutation fixtures
- full CTest suite
- package
- PE32 and exact package-layout verification
- artifact upload

## Artifact audit

- artifact ID: `8429766712`
- GitHub artifact size: `313682`
- outer artifact SHA-256:
  `c24d42d7dbd324a0e2308f2e90fffb1770088b00317dc577e4414be9aa7c26bc`
- inner deployment ZIP SHA-256:
  `6f942be7c17d26afe1c8f8f13369274b121a9e9231276f41406df43a9bfc789d`
- ASI SHA-256:
  `baaca814d1852022a1881347de7289a4219e600f31941b13532418f3d5f66a35`
- packaged INI SHA-256:
  `08250ca296d868404804e7c714aabf519b6f5c95ed483ad62195c2419b3e8f82`

The outer artifact contains exactly `CampaignCompletionDebug.zip`. The inner
ZIP contains exactly:

- `Plugins/CampaignCompletionDebug.asi`
- `Plugins/CampaignCompletion/CampaignCompletionDebug.ini`

The ASI is PE32 x86 (`IMAGE_FILE_MACHINE_I386`) and exports exactly
`CampaignCompletionDebugStop`. The packaged INI is source-equivalent after
line-ending normalization and declares `Version=0.13.2` and
`OpenHotkey=Ctrl+Shift+M`.

Binary inspection found exact UTF-16LE forms of:

- `战役/地图完成标识管理器`
- `刷新`
- `应用`
- `关闭`
- the normal writable-store status guidance

## Deployment boundary

The audited `0.13.2` artifact is not deployed by this audit. Deployment
requires both `S4_Main` and Settlers United to be closed normally by the user
and a fresh explicit deployment approval. A guarded replacement may change
only `Plugins/CampaignCompletionDebug.asi` in the combined Settlers United
archive. `PileChainRepair.asi` and every other entry must remain byte-for-byte
identical.

RD identity remains fail-closed: only exact same-session
`identity.relative` is admitted. An exact relative path beginning with `RD_`
remains random, is never displayed or named by the manager, and is never
eligible for manual marker state.
