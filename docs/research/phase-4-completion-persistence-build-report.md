# Phase 4 Completion Persistence Build Report

## Result

The `0.4.0` completion-persistence implementation passed its final Win32
build, policy, test, packaging, and artifact checks at commit
`b550375e9d321b981f35384929a781e31f8a44ef`.

No deployment was performed. The game must remain closed until the user
approves guarded deployment of this verified `0.4.0` artifact.

The currently deployed `0.3.3` binary still resolves the former root-level
configuration location and must not be used with the migrated directory.

## Final CI evidence

- Workflow: `build-debug-asi`
- Run: `29304084827`
- Job: `86993786685`
- Head SHA: `b550375e9d321b981f35384929a781e31f8a44ef`
- Run URL: <https://github.com/pikechu/s4-campaign-tracker/actions/runs/29304084827>
- Conclusion: `success`
- Artifact name: `CampaignCompletionDebug-Win32`
- Artifact ID: `8299610109`
- CTest: `1/1` target passed in `0.65 s`, containing 33 directly invoked test
  suites (32 calls plus the final runtime-policy suite)

The workflow also passed the Settlers United archive integration tests, the
PE32 and package-layout gate, the zero-process-patch/read-only-Lua verifier,
and every forbidden-behavior mutation fixture.

## Downloaded artifact inspection

The downloaded inner package is stored only in the project-owned inspection
directory:

```text
artifacts/phase-4-completion-persistence-ci/29304084827/
```

Hashes and sizes:

| Item | Bytes | SHA-256 |
|---|---:|---|
| `CampaignCompletionDebug.zip` | 602448 | `072b313d9e8726033cee09b046c9530a07f0099b88b715c8d5aa536b2da32cae` |
| `CampaignCompletionDebug.asi` | 1802752 | `b50fc1c9ae49ddeecedfa83496a1b59cc4ddcc6f47e9fb555af45502ec675096` |
| `CampaignCompletionDebug.ini` | 591 | `573a99ce24026b43901b0c4914b1b06ae6a6eb08f82826926695c88544ef5b2a` |

The package contains exactly:

```text
Plugins/CampaignCompletion/CampaignCompletionDebug.ini
Plugins/CampaignCompletionDebug.asi
```

Independent inspection identified the ASI as a six-section PE32 Intel 80386
DLL and found exactly the expected `CampaignCompletionDebugStop` export. The
project's temporary-archive Settlers United integration suite was rerun
locally and passed. The exact Visual Studio `dumpbin` verifier could not be
rerun locally because the host has no Win32 Visual Studio toolchain; the same
verifier ran successfully against this final build in job `86993786685`.

## Packaged policy

The packaged INI was read back from the downloaded ZIP and contains:

```ini
Version=0.4.0
CaptureTraceRoot=
DiagnosticMode=CompletionPersistence
NativeEventSubscription=1
NativeTerminalEventId=609
InternalVictoryHook=0
HookCount=0
CodePatchBytes=0
GameDefaultGameEndCheckCalls=0
LuaWrites=0
GameDataWrites=0
CompletionDetection=1
CompletionStorage=1
CompletionMarkers=0
```

Completion detection and persistence are enabled. Completion-marker rendering
remains disabled for this phase.

## Review findings

The review covered the implementation range from the pre-design baseline
through `b550375`, including completion identity, JSON parsing, transaction
file operations, the worker, runtime lifecycle, listener integration,
configuration, packaging, and policy verification.

The review specifically checked:

- exact session matching and reset on MapInit;
- local `won` event `609` admission and rejection of loss/online sessions;
- direct/load stable-ID deduplication and recordable random-map data;
- strict UTF-8/schema/count/size validation;
- duplicate completion as a zero-write operation;
- stale temporary-file nonpromotion;
- corrupt-main/valid-backup read-only behavior with manual-action logging;
- complete temporary-file validation before atomic replace or first move;
- bounded queue admission and 5000 ms controlled drain;
- resource retention after drain timeout and process-lifetime runtime ownership;
- absence of new hooks, code patches, Lua writes, game-data writes, and
  `GameDefaultGameEndCheck` calls;
- module-relative writable paths below `Plugins\CampaignCompletion` and no
  persisted save/profile/absolute user paths.

One important issue was found and fixed during review: the stable-ID index was
previously rebuilt after the file commit. Commit `b550375` now prepares that
index before any file write and uses compile-time nothrow-move checks, so an
allocation failure cannot produce a new disk snapshot with stale in-memory
state.

No unresolved critical or important review findings remain. The CI annotation
about GitHub Actions Node 20 deprecation is external to the plugin build and
did not affect the result.

## Deployment boundary

This report and all downloaded artifacts are project files only. No game file,
live ASI, live configuration, or Settlers United archive was changed. A future
deployment must separately confirm the game is closed, preserve and verify the
existing immutable archive backup and migrated configuration backup, and use
the already guarded exact archive-replacement workflow.
