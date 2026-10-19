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

The original source code has been preserved in dedicated branches,
representing significant versions recovered from the project's history.

## Repository Structure

### Historical Branches

The following branches contain preserved historical versions of the
editor:

- `modicom-v1.3` — Modicom 1.3
- `modicom-v1.4` — Modicom 1.4
- `modicom-v2.0` — Modicom 2.0 (this branch)

These branches are intended to remain unchanged and provide reference
points for the evolution of the project. Further versions will be
added as they are recovered.

### Main Branch

The `main` branch represents the active development line. It begins
with the historical codebase and may include:

- Portability improvements
- Linux support
- Build system modernisation
- Bug fixes
- Documentation updates

## Original Environment

The original versions were developed for DOS and later Windows
environments.

This historic branch (`modicom-v2.0`) documents the period toolset and
structural changes for version 2.0 in `BUILD.md`. Broader version
history across releases is collected in `docs/history.md` on the
active development line.

## Building

To rebuild this version with the original toolchain, see `BUILD.md`.

Original DOS executables for tagged releases are published as GitHub
Release assets, not tracked in the repository tree.

Build instructions for currently supported modern platforms will be
documented here as ports land on `main`.

## Historical Note

Modicom was created at a time when many Clipper developers were
comfortable using the dBASE III+ editor for source code development.
Rather than launching dBASE solely to edit program files, Modicom
provided a familiar editing environment as a dedicated standalone tool.
