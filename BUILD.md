# Building Modicom 1.3

This branch preserves Modicom version 1.3 as recovered from period
archives. It is intended to be built with the original DOS toolchain,
not a modern compiler.

## Original toolset

| Role | Tool |
|------|------|
| Host OS | MS-DOS (or a compatible DOS environment) |
| Compiler / IDE | Borland C++ |
| Assembler | Turbo Assembler (TASM), for `VIDEO.ASM` |
| Project file | `MODICOM.PRJ` (Turbo C / Borland IDE project format) |
| Desktop state | `MODICOM.DSK` (Borland IDE desktop file) |

### Compiler version

This snapshot was built with **Borland C++**. The shipped executable
embeds the string `Borland C++ - Copyright 1991 Borland Intl.`, and
`MODICOM.PRJ` is a Turbo C–format project file (also used by Borland
C++).

A machine archive from the same period suggests **Borland C++ 3.1**
(c. 1992). That version is the best current match for rebuilding this
tree, but it has not been independently confirmed beyond the surviving
artifacts and period context.

An earlier origin in Turbo C is possible. Borland C++ continued to
read Turbo C project files, so a later migration to Borland C++ would
be consistent with what remains. That earlier history is not verified
from the sources on this branch.

### Application copyright

Source files in this version carry:

```text
Copyright (c) 1994, Orkney Software Ltd
```

## How it was built

Open `MODICOM.PRJ` in the Borland C++ IDE and build the project from
there. The project references:

- `MODICOM.C`
- `EDITOR.C`
- `STRINGS.C`
- `KEYBOARD.C`
- `VIDEO.ASM` (assembled with TASM)

Include and library paths in the recovered project point at a local
`C:\C\INCLUDE` and `C:\C\LIB` layout; adjust these to match your
Borland installation if you rebuild.

## Scope of this document

This file describes the **historical** toolset for branch
`modicom-v1.3` only. Modern build instructions belong on `main`.
