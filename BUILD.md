# Building Modicom 1.3

This branch preserves Modicom version 1.3 as recovered from period
archives. It is intended to be built with the original DOS toolchain,
not a modern compiler.

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
`VIDEO.ASM`.

That matches the surviving artifacts: the shipped executable embeds
`Borland C++ - Copyright 1991 Borland Intl.`, and `MODICOM.PRJ` is a
Turbo C–format project file (also used by Borland C++). A period
machine archive likewise pointed at Borland C++ 3.1 (c. 1992).

An earlier origin in Turbo C is still possible. Borland C++ continued
to read Turbo C project files, so a later migration would be
consistent with what remains. That earlier history is not verified
from the sources on this branch.

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
Borland C++ 3.1 installation paths for a successful rebuild.

The compiler binary directory also needed to be on `PATH` so the IDE
could find TASM when assembling `VIDEO.ASM`.

## Scope of this document

This file describes the **historical** toolset for branch
`modicom-v1.3` only. Modern build instructions belong on `main`.
