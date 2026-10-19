# Modicom

Modicom is a standalone text editor inspired by the `MODIFY COMMAND`
editor found in dBASE III+.

Originally written in C, the editor was commissioned by a Clipper
developer who was highly familiar with the dBASE III+ editing
environment but wanted to edit source code without having to start
dBASE itself. The result was a standalone editor that reproduced much
of the editing experience of `MODIFY COMMAND` while operating as an
independent application.

## Project Goals

This repository serves two purposes:

1. Preservation of the original historical source code.
2. Ongoing development and porting to modern platforms.

The recovered historical sources have been imported onto dedicated
branches and merged into `main`. Version **2.0** is the last of that
historic series; further work on `main` is modernisation and porting
(unless additional archives are found later).

## Repository Structure

### Historical Branches

The following branches contain frozen snapshots of recovered
versions:

- `modicom-v1.3` — Modicom 1.3 (tag `v1.3.0`)
- `modicom-v1.4` — Modicom 1.4 (tag `v1.4.0`)
- `modicom-v2.0` — Modicom 2.0 (tag `v2.0.0`)

These branches are intended to remain unchanged and provide reference
points for the evolution of the project.

### Main Branch

The `main` branch is the active development line. It currently
matches the Modicom 2.0 historical tree and may go on to include:

- Portability improvements
- Linux support
- Build system modernisation
- Bug fixes
- Documentation updates

## Original Environment

The original versions were developed for DOS and later Windows
environments.

Period toolsets and rebuild notes are in `BUILD.md` (reflecting the
sources on `main`, currently 2.0). Each historic branch also keeps its
own copy of those notes. Version timeline and release artifacts are
documented in `docs/history.md`.

## Building

To rebuild the historical DOS sources with the original toolchain,
see `BUILD.md`.

Original DOS executables for tagged releases (`v1.3.0`, `v1.4.0`,
`v2.0.0`) are published as GitHub Release assets, not tracked in the
repository tree.

Build instructions for currently supported modern platforms will be
documented here as ports land on `main`.

## Historical Note

Modicom was created at a time when many Clipper developers were
comfortable using the dBASE III+ editor for source code development.
Rather than launching dBASE solely to edit program files, Modicom
provided a familiar editing environment as a dedicated standalone tool.
