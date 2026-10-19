# Modicom version history

This document records recovered historical versions and how they are
preserved in the repository. It is maintained on `main`.

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

## Repository line

After each historic import is merged, `main` advances with that
tree. Historic branches remain frozen references. Modern ports and
build-system work continue only on `main`.
