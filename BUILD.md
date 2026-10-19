# Building Modicom 2.0

This branch preserves Modicom version 2.0 as recovered from period
archives. It is intended to be built with the original DOS toolchain,
not a modern compiler.

## Changes in 2.0

Compared with 1.4, this is a structural and xBase-focused release
rather than a small patch:

- Product identity becomes **ModiCom xBase Programmer's Editor**
  (version banner `v2.0` / `v2.0m`).
- Editor constants and the `textedit` prototype move into a new
  header, `EDITOR.H`.
- String helpers gain `iseol` and `linelen` in `STRINGS.H` /
  `STRINGS.C`. End-of-line recognition includes Clipper/xBase soft
  carriage return (`0x8d`) as well as CR and NUL.
- `VIDEO.ASM` display logic is updated for those line endings and
  related markers.
- Dual builds are selected with the `VERSION_M` preprocessor symbol
  (defined in `EDITOR.H` in this tree): without it the banner is
  `v2.0`; with it, `v2.0m`. Period binaries `MODICOM.20` and
  `MODICOM.20M` correspond to those variants.
- Copyright text in `MODICOM.C` reads `1994-95`.

## Original toolset

| Role | Tool |
|------|------|
| Host OS | MS-DOS (or a compatible DOS environment) |
| Compiler / IDE | Borland C++ 3.1 |
| Assembler | Turbo Assembler (TASM), for `VIDEO.ASM` |
| Project file | `MODICOM.PRJ` (Turbo C / Borland IDE project format) |
| Desktop state | `MODICOM.DSK` (Borland IDE desktop file) |

### Compiler version

This version continues the Borland C++ toolset used for 1.3/1.4.
Earlier releases were successfully rebuilt with **Borland C++ 3.1**
on DOS and Turbo Assembler for `VIDEO.ASM`; use the same environment
for 2.0 unless a later note records otherwise.

The shipped artifacts embed `Borland C++ - Copyright 1991 Borland
Intl.`, and `MODICOM.PRJ` is a Turbo C–format project file (also used
by Borland C++).

An earlier origin in Turbo C is still possible. Borland C++ continued
to read Turbo C project files, so a later migration would be
consistent with what remains. That earlier history is not verified
from the surviving sources.

### Application copyright

```text
Copyright (c) 1994-95 by Orkney Software Ltd
```

(`VIDEO.ASM` still carries a 1994 Orkney copyright line.)

## How to build

Open `MODICOM.PRJ` in the Borland C++ 3.1 IDE and build the project
from there. The project references:

- `MODICOM.C`
- `EDITOR.C` / `EDITOR.H`
- `STRINGS.C` / `STRINGS.H`
- `KEYBOARD.C` / `KEYBOARD.H`
- `VIDEO.ASM` (assembled with TASM)

Include and library paths in the recovered project point at a local
`C:\C\INCLUDE` and `C:\C\LIB` layout. These needed updating to the
Borland C++ 3.1 installation paths for earlier rebuilds; use the same
adjustment for 2.0.

The compiler binary directory also needed to be on `PATH` so the IDE
could find TASM when assembling `VIDEO.ASM`.

To build the non-`m` variant, remove or undefine `VERSION_M` in
`EDITOR.H` before compiling.

## Scope of this document

This file describes the **historical** toolset for branch
`modicom-v2.0` only. Modern build instructions belong on `main`.
Broader version history across releases is collected in
`docs/history.md` on the active development line.
