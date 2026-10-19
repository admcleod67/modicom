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

The following branches will contain preserved historical versions of 
the editor:

- `modicom-v1`
- `modicom-v2`
- `modicom-v3`
- `modicom-v4`

These branches are intended to remain unchanged and provide reference
points for the evolution of the project.

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

This historic branch (`modicom-v1.4`) documents the period toolset in
`BUILD.md`. Broader version history across releases will be collected
in `docs/history.md` on the active development line.

## Building

To rebuild this version with the original toolchain, see `BUILD.md`.

Build instructions for currently supported platforms can be found on
the `main` branch.

## Historical Note

Modicom was created at a time when many Clipper developers were
comfortable using the dBASE III+ editor for source code development.
Rather than launching dBASE solely to edit program files, Modicom
provided a familiar editing environment as a dedicated standalone tool.

