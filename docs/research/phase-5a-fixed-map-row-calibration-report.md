# Phase 5A Fixed-Map Row Calibration Report

## Result

The `0.5.0` fixed-map row-calibration candidate passed its Win32 build,
policy, test, packaging, and artifact checks at commit
`b314be045c1a16a1d4bafda78d886a153d290ba3`.

This is a calibration-only build. It observes candidate fixed-map rows and
writes one bounded project-owned trace below the existing
`Plugins\CampaignCompletion` data directory. It does not draw a completion
marker, create a UI control, add an internal hook, change completion admission,
or modify game data. `CompletionMarkers` remains `0`.

The candidate audit is **GO for guarded deployment and live Phase 5A
calibration**. The row-signature/rendering decision remains pending live
evidence; this report does not authorize Phase 5B rendering.

## Final CI evidence

- Workflow: `build-debug-asi`
- Run: `29309353803`
- Job: `87009603527`
- Head SHA: `b314be045c1a16a1d4bafda78d886a153d290ba3`
- Run URL: <https://github.com/pikechu/s4-campaign-tracker/actions/runs/29309353803>
- Conclusion: `success`
- Artifact name: `CampaignCompletionDebug-Win32`
- Artifact ID: `8301472389`
- Artifact API size: `617462` bytes
- Artifact API digest:
  `sha256:22f42e955bbd3e870ca33099895d6726298d40e0dde7875b211025ceeb47d4dd`
- CTest: `1/1` target passed in `0.87 s`; total time `0.89 s`

The workflow passed all of these gates against the final commit and artifact:

- Settlers United archive integration;
- Win32 `/W4 /WX` build;
- PE32 completion-persistence and zero-process-patch policy verification;
- rejection of forbidden process-patch, Lua-write, end-check, internal-hook,
  and related mutation fixtures;
- the complete CTest suite;
- package creation;
- PE32 machine and exact two-entry package verification; and
- artifact upload.

## Downloaded artifact inspection

The downloaded inner package and its extracted inspection copy are stored only
in this untracked project-owned directory:

```text
artifacts/phase-5a-row-calibration/run-29309353803/
```

| Item | Bytes | SHA-256 |
|---|---:|---|
| `CampaignCompletionDebug.zip` | 617781 | `d598056ba32934a675a83150fc75804b73808da269b6f8e4021d8a761f10d685` |
| `CampaignCompletionDebug.asi` | 1847808 | `c2177bc473dfdcb30b9f2f1d08c3257cbe858ec618e2844da2cddc2b47dc3326` |
| `CampaignCompletionDebug.ini` | 613 | `d90a2a0014769567abbcec4be8a2eab6a66289a1732578c3a828ed31f9a2a220` |

The inner package contains exactly:

```text
Plugins/CampaignCompletion/CampaignCompletionDebug.ini
Plugins/CampaignCompletionDebug.asi
```

Independent read-only inspection identified a six-section PE32 Intel 80386 DLL.
Reading the COFF machine field at PE offset `280` produced `0x014c`, matching
`IMAGE_FILE_MACHINE_I386`.

The temporary-archive Settlers United integration suite was also rerun locally
and passed. The host has no Visual Studio `dumpbin.exe`, so the full binary
policy script could not be rerun locally; its prerequisite failure was recorded
as `Win32 dumpbin.exe is unavailable`. The exact script ran successfully
against this final ASI in CI job `87009603527`, which reported:
`Verified PE32 completion persistence ASI, zero process patches, and read-only
Lua bridge.`

## Packaged policy

The packaged INI was extracted and read back with these material fields:

```ini
Version=0.5.0
CaptureTraceRoot=
DiagnosticMode=FixedMapRowCalibration
MarkerCalibration=1
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

The marker-calibration trace uses unique `CREATE_NEW` files named from the
current process ID, admits only the calibration record allowlist, and is capped
at `262144` bytes. No absolute game path, pointer, module base, arbitrary memory,
or unrelated UI text is admitted to that trace.

## Calibration implementation review

The final candidate builds an immutable fixed-map candidate index from the
successfully loaded completion-store snapshot. The index accepts only fixed,
offline records whose source/category pair and relative map directory agree;
random, campaign, online, malformed, and ambiguous identities fail closed.

The runtime forwards only public S4ModApi UI-frame, page-snapshot, approved-tab,
and GUI-element observations. GUI text is copied through a bounded SEH helper;
the coordinator receives value fields rather than retained game pointers. It
requires the exact fixed-map page set and known tab attribution, validates row
geometry against the reported surface, and records only candidate-name matches
plus their following frame boundary. Phase 5A contains no drawing API or marker
renderer.

Shutdown ordering was verified: calibration is disabled before public listener
removal, in-flight callbacks quiesce, the completion worker drains, and only
then are the calibration coordinator/index released and its trace closed.

During the GREEN run, the version bump exposed an important schema-1
compatibility defect: the decoder initially required every stored
`plugin_version` to equal the current build. That would have rejected the
existing valid `0.4.0` Aeneas and Antares records. Commit `a68182d` now admits
only persistence versions `0.4.0` and current `0.5.0`; legacy records remain
byte-preserved, new records identify `0.5.0`, and pre-persistence or unknown
versions remain rejected. The final CI suite covers this behavior.

No unresolved build, policy, persistence-compatibility, or package finding
remains. The GitHub Actions Node 20 deprecation annotation concerns the hosted
actions runtime and did not affect the plugin result.

## Deployment and live gate

No game or Settlers United file was changed during this build audit. The game
was running, so no deployment was attempted.

Guarded deployment requires the user to close the game and Settlers United and
explicitly approve installation of the exact audited ASI/INI above. After
deployment, the live test must establish repeatable Aeneas and Antares row
signatures, frame order, scrolling behavior, category isolation, trace bounds,
responsiveness, and normal shutdown before Phase 5A can be declared live GO.

Until that evidence exists, `CompletionMarkers` stays `0` and no Phase 5B
rendering constants may be guessed.
