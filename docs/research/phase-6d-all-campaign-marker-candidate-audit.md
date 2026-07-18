# Phase 6D all-campaign marker candidate audit

Date: 2026-07-18 (Asia/Shanghai)

## Decision

Implementation checkpoint
`2de918fe6e02196cdef2e6770a7838657861b017` is **STATIC GO** for a separately
approved guarded deployment of the Phase 6D all-campaign read-only marker
candidate. This audit is not deployment approval.

The candidate freezes all 107 `CLOSED` descriptors, restores the accepted
fixed-map marker path, and adds a separate campaign index and public snapshot
observer. The shared marker frame is exactly 36 commands, matching the maximum
simultaneously resident controls on composed page 6.

Completion detection, completion storage writes, native victory events, hooks,
patches, Lua writes, game-data writes, and campaign-progress writes remain
disabled.

## Read-only and identity boundary

- Startup calls the bounded file interface exactly once for the main
  `completed_maps.json`.
- Missing, unavailable, or malformed main data fails closed to an empty owned
  snapshot. The candidate does not read the backup, normalize, create,
  replace, or write any database file.
- Campaign index admission requires `Fixed`, `Campaign`, strict UTF-8, an
  exact self-consistent stable ID, and one exact admitted descriptor relative.
- Duplicate valid records make that descriptor ambiguous and suppress its
  marker.
- Display/save values are ignored. In particular, `RD_PlayerSave` does not
  change classification. Only the exact relative identifier is authoritative;
  case, separator, prefix, source, type, or stable-ID changes fail closed.
- Runtime descriptor/session validation continues to use confirmed
  same-MapInit-session `identity.relative`; it never uses `identity.name`.

## Local gates

- Full 98-source test target cross-compiled and linked as Win32 PE32: PASS.
- Focused compilation of all changed production translation units: PASS.
- `git diff --check`: PASS.
- Intermediate RED remained local and was squashed with GREEN before push.
- The untracked `docs/handoffs/` directory was excluded.

The local host does not provide CMake/MSVC or a Windows execution layer, so
the authoritative workflow supplies the MSVC build and executable test result.

## Authoritative Windows CI

- Workflow: `build-debug-asi`.
- Run: `29635655670`.
- Job: `88057416571`.
- Head SHA: `2de918fe6e02196cdef2e6770a7838657861b017`.
- Started: `2026-07-18T07:23:36Z`.
- Completed: `2026-07-18T07:26:17Z`.
- Result: PASS.

All steps passed: Settlers United archive integration, Win32 Release
configuration, MSVC build, marker/runtime policy, all forbidden-behavior
mutation fixtures, the complete test suite, packaging, PE32/export and package
verification, and artifact upload.

## Artifact audit

- Artifact ID: `8427025352`.
- Artifact name: `CampaignCompletionDebug-Win32`.
- GitHub-reported size: `264486` bytes.
- GitHub service digest and downloaded wrapper SHA-256:
  `d68b3b497f21e9b7cf3962e198339464c02b5ea8e0a8eab01c6b64be9f663c2e`.
- Candidate `CampaignCompletionDebug.zip` size: `264458` bytes.
- Candidate ZIP SHA-256:
  `99c3dceaa46057500b63b4c0853b63bbdd68ec45d5c36705ec4105b0e2c933eb`.
- Packaged ASI size: `484864` bytes.
- Packaged ASI SHA-256:
  `720566b1b0c8bab874a61b879f0f0c1a796562e7a4abede4437d79e5cc342d17`.
- Packaged INI size: `1355` bytes.
- Packaged INI SHA-256:
  `3e4bca3799f6a8ef5eb68dd8a42603b6ce241db5abe9145b22e2a57ef73a1b18`.
- Source INI size: `1311` bytes.
- Source INI SHA-256:
  `e7c356449183cc299bba3d328b097627fa6f032267c53618702ba6ee800e6cd7`.

The wrapper contains exactly one candidate ZIP. The candidate ZIP contains
exactly the project ASI and INI at their expected plugin paths. The packaged
INI is content-identical to the source after expected LF-to-CRLF packaging
normalization. The ASI is PE32/i386; the workflow independently verified its
export and package policy.

## Boundary and next action

No game or Settlers United process was started, stopped, or inspected. No
installed archive, plugin file, live database, backup, save, or campaign
progress was read or modified.

Deployment requires a fresh explicit approval naming this exact audited Phase
6D candidate after both protected applications have been normally closed.
Deployment must use the audited hashes above and the established guarded
rollback procedure. A later live acceptance may read the database only within
its separately approved bound and does not authorize a completion write or
victory transaction.
