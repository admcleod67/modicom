# Modicom version history

This document records recovered historical versions and how they are
preserved in the repository. It is maintained on `main`.

The planned historic import series ends at **Modicom 2.0**. The
branches and tags below are the complete set recovered so far.

## Preserved versions

### 1.3 (`modicom-v1.3`, tag `v1.3.0`)

First recovered snapshot imported into the repository. Sources and
Borland project files target DOS; rebuild verified with Borland C++
3.1 and Turbo Assembler. See `BUILD.md` on that branch for toolset
details.

Original DOS builds are attached to the GitHub release for `v1.3.0`.

### 1.4 (`modicom-v1.4`, tag `v1.4.0`)

Small follow-on release. Extends 1.3 by ignoring extended
(non-character) keys in the default insert path so they are not typed
into the edit buffer as junk characters. Version banner updated to
v1.4. Same Borland C++ 3.1 / TASM toolset as 1.3.

Original DOS builds are attached to the GitHub release for `v1.4.0`.
`MODICOM.EXE` and `MODICOM.14` from that release are the same image;
`MODICOM.14M` is a second period build of the same version.

### 2.0 (`modicom-v2.0`, tag `v2.0.0`)

Last recovered historic version. Structural and xBase-focused release
relative to 1.4:

- Editor definitions moved to `EDITOR.H`
- String helpers gain `iseol` / `linelen`, including soft CR `0x8d`
- `VIDEO.ASM` updated for xBase line-ending display
- Dual builds via `VERSION_M`: **v2.0** appends Ctrl+Z on save;
  **v2.0m** does not
- Product banner: ModiCom xBase Programmer's Editor; copyright
  `1994-95`

Rebuild verified with Borland C++ 3.1 and TASM. See `BUILD.md` on
that branch (and on `main`) for toolset and `VERSION_M` details.

Original DOS builds are attached to the GitHub release for `v2.0.0`:
`MODICOM.20` is **v2.0**; `MODICOM.20M` and `MODICOM.EXE` are the
same **v2.0m** image.

## Development note: `SHOWPAGE.C`

`SHOWPAGE.C` appears to be an early C implementation of page display
that was superseded by the `showpage` routine in `VIDEO.ASM` before
the earliest recovered snapshot (1.3). The C version drew through
Borland console functions; the assembler version writes directly to
video RAM and keeps the same `<` / `+` line markers. The author
recalls screen handling being slow and resorting to assembler, which
supports that reading. The C file is preserved on the historic
branches and under `src/` on `main`, but it has not been part of the
Borland project in any recovered version.

## Repository line

Historic branches remain frozen references with the recovered flat
layout. After 2.0 was imported, `main` was restructured so sources
live under `src/` and DOS build output under `build/`; the Borland
project at the repository root was updated for those paths and
rebuilds successfully with Borland C++ 3.1. Content on `main` remains
the 2.0 baseline for modern ports and build-system work. Any later
archive discovery would be handled as a new historic branch rather
than extending this closed series.
