# ModiCom

ModiCom is a standalone text editor inspired by the `MODIFY COMMAND`
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
historic series; `main` is now the unreleased **2.1** development
line, with further work on `main` being defect fixes, modernisation,
and porting (unless additional archives are found later).

## Repository Structure

### Historical Branches

The following branches contain frozen snapshots of recovered
versions:

- `modicom-v1.3` — ModiCom 1.3 (tag `v1.3.0`)
- `modicom-v1.4` — ModiCom 1.4 (tag `v1.4.0`)
- `modicom-v2.0` — ModiCom 2.0 (tag `v2.0.0`)

These branches are intended to remain unchanged and provide reference
points for the evolution of the project. They retain the recovered,
mostly flat source layout.

### Main Branch

The `main` branch is the active development line. Its code is based on
the ModiCom 2.0 historical tree, but the working layout has since been
restructured:

- `src/` — C, header, and assembly sources
- `build/` — generated object files and executables (not source)
- `examples/` — period files used to exercise the editor
- `docs/` — project history and documentation
- `tests/` — automated tests and manual smoke-test notes
- `MODICOM.PRJ` / `MODICOM.DSK` — Borland project files at the root

Future work on `main` may include:

- Portability improvements
- Linux support
- Build system modernisation
- Bug fixes
- Documentation updates

## Original Environment

The original versions were developed for DOS and later Windows
environments.

Period toolsets and rebuild notes are in `BUILD.md`. On `main`, those
notes describe the restructured 2.0-based source tree; each historic
branch keeps notes matching its preserved layout. Version timeline
and release artifacts are documented in `docs/history.md`.

## Building

To rebuild the DOS version from the current `src/` layout with the
original Borland toolchain, see `BUILD.md`. Build output is placed
under `build/`.

A small automated test suite lives in `tests/`. It builds
separately from the editor and no test code is linked into
`MODICOM.EXE`. See `tests/README.md` for instructions on building
and running the tests in DOSBox, together with a manual smoke-test
checklist.

Original DOS executables for tagged releases (`v1.3.0`, `v1.4.0`,
`v2.0.0`) are published as GitHub Release assets, not tracked in the
repository tree.

Build instructions for currently supported modern platforms will be
documented here as ports land on `main`.

## Historical Note

ModiCom was created at a time when many Clipper developers were
comfortable using the dBASE III+ editor for source code development.
Rather than launching dBASE solely to edit program files, ModiCom
provided a familiar editing environment as a dedicated standalone tool.

## Copyright and Provenance

ModiCom was written by Allan McLeod and originally carried Orkney
Software Ltd copyright notices. Orkney Software Ltd has since
ceased to exist, and its software rights were transferred to Allan
McLeod, who holds the copyright and licenses this project under
the MIT License (see `LICENSE`).

On `main`, the program banner and source-header notices now name
Allan McLeod. Historic branches, tags, period examples, and the
original DOS release artifacts retain their original Orkney
Software Ltd notices.
