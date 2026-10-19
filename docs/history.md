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

## Repository line

Historic branches remain frozen references. With 2.0 imported,
`main` carries that tree as the baseline for modern ports and
build-system work. Any later archive discovery would be handled as a
new historic branch rather than extending this closed series.
