# Building Modicom (historical DOS tree)

The sources currently on `main` match the Modicom **1.4** historical
tree. They are intended to be built with the original DOS toolchain,
not a modern compiler.

Frozen, version-specific copies of these notes also live on the
historic branches (`modicom-v1.3`, `modicom-v1.4`). See
`docs/history.md` for the version timeline.

Compared with 1.3, version 1.4 is a small behavioural fix: extended
(non-character) keys are no longer inserted into the edit buffer as
junk characters. The version banner in `MODICOM.C` reads v1.4.

## Original toolset

| Role | Tool |
|------|------|
| Host OS | MS-DOS (or a compatible DOS environment) |
| Compiler / IDE | Borland C++ 3.1 |
| Assembler | Turbo Assembler (TASM), for `VIDEO.ASM` |
| Project file | `MODICOM.PRJ` (Turbo C / Borland IDE project format) |
| Desktop state | `MODICOM.DSK` (Borland IDE desktop file) |

### Compiler version

This version uses the same toolchain as 1.3: **Borland C++ 3.1** on
DOS, with Turbo Assembler for `VIDEO.ASM`. Version 1.3 was
successfully rebuilt with that toolset; the 1.4 sources are the same
project layout with the input-handling change above.

The shipped artifacts embed `Borland C++ - Copyright 1991 Borland
Intl.`, and `MODICOM.PRJ` is a Turbo C–format project file (also used
by Borland C++).

An earlier origin in Turbo C is still possible. Borland C++ continued
to read Turbo C project files, so a later migration would be
consistent with what remains. That earlier history is not verified
from the surviving sources.

### Application copyright

Source files in this version carry:

```text
Copyright (c) 1994, Orkney Software Ltd
```

## How to build

Open `MODICOM.PRJ` in the Borland C++ 3.1 IDE and build the project
from there. The project references:

- `MODICOM.C`
- `EDITOR.C`
- `STRINGS.C`
- `KEYBOARD.C`
- `VIDEO.ASM` (assembled with TASM)

Include and library paths in the recovered project point at a local
`C:\C\INCLUDE` and `C:\C\LIB` layout. These needed updating to the
Borland C++ 3.1 installation paths for a successful rebuild of 1.3;
use the same adjustment for 1.4.

The compiler binary directory also needed to be on `PATH` so the IDE
could find TASM when assembling `VIDEO.ASM`.

## Scope of this document

This file describes the **historical** DOS toolset for the sources
present on `main` (currently 1.4). Modern platform build instructions
will be added separately as ports land.
