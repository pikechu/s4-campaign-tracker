# Phase 3A Public Victory UI Calibration Report

## Build and frozen artifact evidence

- Source commit: `a3cda38ec334ebf7da7275294f00973569ad7c37`.
- GitHub Actions workflow: `build-debug-asi`; run: `29256442353`.
- Successful jobs: `86837923430` (attempt 1) and `86838335883` (attempt 2).
- Both attempts passed the Settlers United archive integration test, Win32 configure and build, public-calibration policy verifier, all forbidden-behavior fixtures, CTest, package layout and PE32 checks, and artifact upload.
- Frozen artifact: ID `8281509763`; GitHub digest `sha256:513045c5eba9aa5f8739b62f2d56417306b989c63ec66d9064243e6de55dd450`; downloaded size `534309` bytes.
- Downloaded artifact SHA-256: `513045c5eba9aa5f8739b62f2d56417306b989c63ec66d9064243e6de55dd450`, exactly equal to the GitHub digest.
- Inner diagnostic package SHA-256: `a1240dd6eab61fbca4777b4ef3b040d53991617966d82f185ba4b3d51f285f89`; size `534545` bytes.
- Frozen ASI SHA-256: `5d5080965be4b0a95ce41ddfe9002e6a062f5192ea16663de9de9d267ec8b193`; size `1625088` bytes; PE machine `0x014c` (`IMAGE_FILE_MACHINE_I386`).
- Frozen INI SHA-256: `25f418f3f7de191dffb525a8fccd142df647a279c70594d7b4a321409b97036b`; size `552` bytes.
- The inner package contains exactly `Plugins/CampaignCompletionDebug.asi` and `CampaignCompletion/CampaignCompletionDebug.ini`.
- The packaged INI identifies version `0.3.0`, `DiagnosticMode=VictoryUiCalibration`, public settlement UI and launch/load-origin tracking, and zero internal victory Hook, code patch, Lua write, game-data write, completion detection, storage, or marker behavior. Its `CaptureTraceRoot` is empty.
- The frozen files are retained below the ignored project directory `artifacts/phase-3a-ci/29256442353-attempt-2/`.

## Enforced calibration boundary

- Production sources linked into the ASI are scanned transitively through local headers.
- The verifier rejects internal/default game-end checks, the Lua victory-condition handler, the internal victory-screen class, Lua mutation APIs, process-write/page-protection APIs, and Hook/patch frameworks.
- CI mutation fixtures prove rejection of `GameDefaultGameEndCheck`, `DefaultGameEndCheck`, `VICTORY_CONDITION_CHECK`, `CStateVictoryScreen`, `lua_setglobal`, `lua_settable`, `WriteProcessMemory`, `VirtualProtect`, and `hlib::CallPatch`.
- Runtime traces are admitted only below a pre-existing non-reparse project root. Each process creates a unique directory with four fixed channels and bounded allowlisted records.
- GUI capture stores numeric public callback fields only. It does not dereference GUI text, tooltip text, or extra tooltip text pointers.
- Offline single-player Multiplayer Maps remain eligible evidence. Online multiplayer and random-map sources are explicitly excluded; ordinary map loads remain `load-map-unresolved` until live evidence resolves their source.

## Pre-live scope statement

This calibration build does not decide victory, persist completion, or render markers. It records bounded public GUI and source evidence only.

No game-directory deployment or live evidence collection has occurred for this frozen artifact. Deployment remains gated on a closed game, re-verification of the immutable original archive backup, and explicit user approval.

## Live evidence gate

The following evidence is still required before Phase 3A can produce a GO decision:

1. Eligible-map voluntary-exit settlement sample.
2. Eligible-map victory settlement sample.
3. Eligible-map defeat settlement sample.
4. New-game source samples for campaign, fixed maps, offline Multiplayer Maps, random maps, and online multiplayer.
5. Load-game source samples sufficient to distinguish eligible ordinary saves from random-map and online-multiplayer saves.
6. Controlled-stop evidence showing all listeners removed, trace flush success, and normal game responsiveness.

No victory/defeat/exit classifier will be implemented until those samples provide stable, reviewable signatures.
