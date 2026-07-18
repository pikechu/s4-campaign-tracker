# Phase 6D gap-only campaign catalog candidate audit

Date: 2026-07-18 (Asia/Shanghai)

## Decision

The implementation checkpoint
`d9d33457520ddc3120d96f4351b31d9c53b138f5` is **STATIC GO** for a guarded
Phase 6D gap-only public catalog batch. This is not deployment approval and is
not Phase 6D geometry or rendering GO.

The candidate remains read-only. Completion detection, completion storage,
completion markers, native victory events, internal menu access, hooks,
patches, Lua writes, game-data writes, database access, and save access all
remain disabled.

## Exact capture contract

- GUI-element callbacks sample the deterministic public campaign owner before
  the first UI-frame callback can finalize the page epoch.
- Only exact statically proven mission-control IDs are retained. Navigation,
  decoration, labels, display names, save names, and neighboring controls are
  excluded.
- Selector page 4 admits only control `1919`, whose accepted executable chain
  is exact event `0x1B5F`, kind `0x0A`, and fixed relative
  `Map\Campaign\md_bonus.map`.
- The composed New World owner remains page 6 and admits only controls
  `1825–1844` and `2939–2954`.
- Pages 11–21 use their frozen exact mission-control ranges.
- Runtime classification remains restricted to exact confirmed same-session
  `identity.relative`. `identity.name`, display/save names, and `RD` text are
  not classification evidence.

## Local gates

- Focused 32-bit Windows capture and runtime-policy tests: PASS.
- `tools/verify_no_process_patch.ps1 -SourceOnly`: PASS.
- `git diff --check`: PASS.
- Manual artifact inspection: PE32/i386 and exactly one
  `CampaignCompletionDebugStop` export.

The local host does not provide Visual Studio `dumpbin.exe`, so the full binary
policy command stopped at that tool-availability check. The authoritative
Windows workflow ran the same binary verifier with `dumpbin` and passed.

## CI correction

Initial workflow run `29495391260` compiled the production ASI successfully
but failed the test target under `/W4 /WX`. A test table initialized `WORD`
members from unsigned integer literals, producing MSVC C4244. The approved
focused correction stores fixture IDs as `DWORD` and explicitly narrows only
at the tested `WORD` API boundary. No production source or runtime behavior
changed in the correction commit.

## Authoritative Windows CI

- Workflow: `build-debug-asi`.
- Run: `29633998241`.
- Job: `88052902693`.
- Head SHA: `d9d33457520ddc3120d96f4351b31d9c53b138f5`.
- Started: `2026-07-18T06:26:33Z`.
- Completed: `2026-07-18T06:29:02Z`.
- Result: PASS.

Every authoritative step passed: Settlers United archive integration, Win32
Release configuration, MSVC build, full no-process-patch/read-only policy,
all forbidden-behavior mutation fixtures, the complete test suite, packaging,
PE32/export verification, and artifact upload.

## Artifact audit

- Artifact ID: `8426458086`.
- Artifact name: `CampaignCompletionDebug-Win32`.
- GitHub-reported size: `241210` bytes.
- GitHub service digest and downloaded wrapper SHA-256:
  `d29d2b6775018d15ef9ac7c848ebb9a2cc0c78dfbb4f3379249f0eaffd49cd27`.
- Candidate `CampaignCompletionDebug.zip` size: `241153` bytes.
- Candidate ZIP SHA-256:
  `8bb237761d6c0b6ae9d71633ab3c41840d6ee28d7bd26ebb7b0c9fed33e78446`.
- Packaged ASI size: `432640` bytes.
- Packaged ASI SHA-256:
  `de36bfca5cfd3f0cf36a063e7717fe1a20c20754bff2f1bcc6d39158dc113a6c`.
- Packaged INI size: `1270` bytes.
- Packaged INI SHA-256:
  `4a9835f5ea2228aea2780e2d393f001d87e09c35dd87f6525969cc69a0b01e38`.
- Source INI SHA-256:
  `561005feb2fb66603258aebd982c85fcda8fc4bef96c5db15677c11da629ce33`.

The wrapper contains exactly one candidate ZIP. The candidate ZIP contains
exactly the project ASI and INI at their expected plugin paths. The packaged
INI is content-identical to the source after the expected LF-to-CRLF package
normalization. It identifies version `0.10.0`, the Phase 6D gap-only catalog
mode, exact static mission-control filtering, and public pre-frame page
priming, while keeping every storage, marker, internal-menu, native-event, and
write capability disabled.

## Boundary and next action

No game or Settlers United process was started, stopped, or inspected. No
installed archive, plugin file, database, backup, save, or campaign-progress
state was read or modified. The untracked `docs/handoffs/` directory was
excluded from staging.

Deployment requires a fresh explicit approval naming this exact Phase 6D
gap-only catalog candidate after the user has normally closed both protected
applications. The guarded live acceptance is one uninterrupted, no-launch
traversal of page 4 and all accessible campaign pages/scroll states, followed
by one completion report and normal shutdown.
