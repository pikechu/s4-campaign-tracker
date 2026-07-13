# S4 Campaign Tracker

An in-game completion tracking plugin for **The Settlers IV: History Edition + Settlers United**.

The project is being developed to record completed single-player missions and
display completion indicators directly in the game UI.

The goal is to provide a unified progress tracker for all Settlers IV campaigns, fixed scenarios, and single-player missions.

---

## Features

The following list describes the intended release feature set, not the current
diagnostic build:

- In-game completion indicators
- Automatic victory detection
- Campaign progress tracking
- Fixed single-player scenario tracking
- Persistent completion database
- Language-independent mission identification
- Manual completion marking
- JSON-based local storage

Supported content:

- The Three Races
- The Dark Tribe
- The New World
- The Trojans
- Great Crusades
- Other single-player campaigns
- Fixed single-player scenarios

---

## Supported Environment

Target environment:

- The Settlers IV: History Edition
- Version 3.15
- S4_Main.exe
- File version: 2.50.1516.0
- Settlers United
- Windows 10 / Windows 11
- Win32 executable

---

## Completion Rules

The plugin records completion when:

```
Local Player Victory
        +
Not Multiplayer
        +
Not Random Generated Map
        =
Record Completion
```

Recorded:

- Campaign missions
- Fixed single-player maps
- Scripted missions with victory conditions

Ignored:

- Multiplayer games
- Random generated maps

---

## Technical Design

Implemented as:

- Win32 ASI plugin
- C++17
- S4ModApi integration
- Optional game hooks when required

Architecture:

```
+----------------------+
| CampaignCompletion   |
|       .asi           |
+----------+-----------+
           |
           |
+----------v-----------+
| Victory Detection    |
+----------+-----------+
           |
           |
+----------v-----------+
| Completion Database  |
+----------+-----------+
           |
           |
+----------v-----------+
| UI Completion Marker |
+----------------------+
```

---

## Phase 2 Diagnostic Build

`CampaignCompletionDebug` is a diagnostic-only bootstrap. It inventories loaded
modules and records events delivered through public S4ModApi listeners. It does
not detect victory, save completion state, create completion JSON, or render
completion markers. It installs no internal game hooks.

The CI artifact has this layout:

```
TheSettlers4/
|
|-- Plugins/
|   `-- CampaignCompletionDebug.asi
|
`-- CampaignCompletion/
    `-- CampaignCompletionDebug.ini
```

When the game runs, the diagnostic plugin may create
`CampaignCompletion/CampaignCompletion.log`. To remove this diagnostic build,
delete only `Plugins/CampaignCompletionDebug.asi` and, if desired, the diagnostic
configuration and log under `CampaignCompletion`. Do not copy research scripts
or build files into the game directory.

Example game location:

```
F:\Program Files (x86)\Ubisoft\Ubisoft Game Launcher\games\thesettlers4
```

Launch the game through Settlers United.

---

## Development Roadmap

Current status: Phase 1 research is complete and the Phase 2 diagnostic bootstrap
is under validation. No completion-tracking Release plugin exists yet.

Research and planning documents:

- [Approved design specification](docs/superpowers/specs/2026-07-13-campaign-completion-design.md)
- [Phase 1 execution plan](docs/superpowers/plans/2026-07-13-phase-1-investigation.md)
- [Phase 1 investigation report](docs/research/phase-1-investigation-report.md)
- [Phase 2 diagnostic bootstrap plan](docs/superpowers/plans/2026-07-13-phase-2-diagnostic-bootstrap.md)

### Phase 1 - Research

- [x] Environment analysis
- [x] S4ModApi detection
- [x] API investigation
- [x] Symbol analysis

### Phase 2 - Core System

- [ ] Victory detection
- [ ] Map identification
- [ ] Completion database

### Phase 3 - UI Integration

- [ ] Campaign menu markers
- [ ] Scenario list markers
- [ ] Support different campaign layouts

### Phase 4 - Release

- [ ] Stable release
- [ ] Documentation
- [ ] Installer package

---

## Compatibility Goals

The plugin should:

- Avoid modifying original game files
- Avoid modifying save files
- Avoid affecting multiplayer
- Avoid affecting random maps
- Work with Settlers United
- Work with HD patches

---

## Credits

Created for the Settlers IV community.

Thanks to:

- Settlers United developers and contributors
- S4ModApi developers and contributors
- The Settlers IV modding community
- ChatGPT for project planning, architecture design, and development assistance

---

## License

This project is licensed under the MIT License.

See [LICENSE](LICENSE) for details.
