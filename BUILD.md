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
  (see below). Period binaries `MODICOM.20` and `MODICOM.20M`
  correspond to the `v2.0` and `v2.0m` variants.
- Copyright text in `MODICOM.C` reads `1994-95`.

### The `VERSION_M` (2.0 / 2.0m) switch

`VERSION_M` is a compile-time switch. In this recovered tree it is
turned on with a `#define VERSION_M` in `EDITOR.H`. The Borland
project file (`MODICOM.PRJ`) does not store a matching
`-DVERSION_M` (or IDE “Defines”) entry; rebuilding in the IDE simply
honours whatever is in that header.

| | Without `VERSION_M` (`v2.0`) | With `VERSION_M` (`v2.0m`) |
|--|------------------------------|----------------------------|
| Banner | ModiCom … Editor v2.0 | ModiCom … Editor v2.0m |
| On save | Appends a DOS Ctrl+Z (`0x1A`) EOF byte | Writes buffer only — no Ctrl+Z |

The save-path logic is in `EDITOR.C`. This tree ships with
`VERSION_M` defined, so a straight rebuild produces the **2.0m**
variant.

## Original toolset

| Role | Tool |
|------|------|
| Host OS | MS-DOS (or a compatible DOS environment) |
| Compiler / IDE | Borland C++ 3.1 |
| Assembler | Turbo Assembler (TASM), for `VIDEO.ASM` |
| Project file | `MODICOM.PRJ` (Turbo C / Borland IDE project format) |
| Desktop state | `MODICOM.DSK` (Borland IDE desktop file) |

### Compiler version

This version has been successfully rebuilt from the sources on this
branch using **Borland C++ 3.1** on DOS, with Turbo Assembler for
`VIDEO.ASM` — the same toolset verified for 1.3 and 1.4.

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

If TASM reports `Error writing object file`, delete any existing
`VIDEO.OBJ` in the project directory and assemble again. A stale or
unwritable object file can cause that failure even when TASM is on
`PATH`.

To build the non-`m` (`v2.0`) variant, remove or comment out
`#define VERSION_M` in `EDITOR.H`, then rebuild. Restore the define
and rebuild for `v2.0m`. (Borland C++ 3.1 could also pass
`-DVERSION_M` via Options → Compiler → Defines; that approach is not
used in the surviving project file.)

## Scope of this document

This file describes the **historical** toolset for branch
`modicom-v2.0` only. Modern build instructions belong on `main`.
Broader version history across releases is collected in
`docs/history.md` on the active development line.
