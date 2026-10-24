# ModiCom version history

This document records recovered historical versions and how they are
preserved in the repository. It is maintained on `main`.

The planned historic import series ends at **ModiCom 2.0**. The
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
- Product banner: ModiCom xBase Programmer's Editor; period
  copyright notice: Orkney Software Ltd, `1994-95`

Rebuild verified with Borland C++ 3.1 and TASM. See `BUILD.md` on
that branch (and on `main`) for toolset and `VERSION_M` details.

Original DOS builds are attached to the GitHub release for `v2.0.0`:
`MODICOM.20` is **v2.0**; `MODICOM.20M` and `MODICOM.EXE` are the
same **v2.0m** image.

## Unreleased development: 2.1

After the deterministic test suites were completed, `main` was
designated the unreleased **2.1** development line. The program
banner on `main` reads `v2.1` (or `v2.1m` with `VERSION_M`, as
this tree builds). There is no `modicom-v2.1` branch and no
`v2.1.0` tag or release yet; 2.1 will be tagged only if and when
it is released. The historic 2.0 series above is unchanged.

The defects those suites exposed have been fixed on `main`. In
brief:

- **Load and edit buffer.** Empty files no longer crash when the
  byte before the buffer happens to be Ctrl+Z. Delete at the very
  end of the file no longer corrupts editor state. Full and
  oversized loads, and inserts into a nearly full buffer, reserve a
  terminator byte (`malloc(MAXBYTES + 1)`) so the NUL is not written
  past the allocation; large reads use unsigned chunked I/O so
  counts above 32767 work on 16-bit Borland.
- **Find.** The search string buffer is sized for a terminator.
  Cancelling Find (Escape) no longer overwrites or clears the
  previous search; Find prompts into a scratch buffer and copies
  back only on acceptance.
- **Display.** `showline` at column 79 no longer sweeps video RAM.
  `showpage` stops at the bottom visible row instead of writing past
  the 4000-byte colour page. The `+` truncation marker appears only
  when characters are still hidden; an exactly fitting line and an
  empty line at column 79 use the normal end-of-line marker.

Each fix was covered by promoting the failing probe into its suite's
green run. All green suites report 0 failures on the current tree.

The former monolithic `EDITOR.C` was then split, without intentional
behaviour change, into a thin Core (`textedit`, `ctrlkey`, session
abort) plus Status, Navigate, Edit, File I/O, Search, and Prompt
modules; Keyboard, Strings, and Video remain as before. Aligning the
green suites more closely with those module boundaries is follow-on
work.

## Development note: `SHOWPAGE.C`

`SHOWPAGE.C` appears to be an early C implementation of page display
that was superseded by the `showpage` routine in `VIDEO.ASM` before
the earliest recovered snapshot (1.3). The C version drew through
Borland console functions; the assembler version writes directly to
video RAM and keeps the same `<` / `+` line markers. The author
recalls screen handling being slow and resorting to assembler, which
supports that reading. The C file is preserved as `SHOWPAGE.C` on
the `modicom-v1.3`, `modicom-v1.4`, and `modicom-v2.0` branches,
but it has not been part of the Borland project in any recovered
version. It has since been removed from `main`, where only the live
`VIDEO.ASM` implementation is retained.

## Repository line

Historic branches remain frozen references with the recovered flat
layout. After 2.0 was imported, `main` was restructured so sources
live under `src/` and DOS build output under `build/`; the Borland
project at the repository root was updated for those paths and
rebuilds successfully with Borland C++ 3.1. The superseded
`SHOWPAGE.C` was subsequently removed from `main` while remaining
preserved on the historic branches. Before the repository was made
public, the main-line copyright notice was updated from Orkney
Software Ltd to Allan McLeod, to whom the company's software rights
had been transferred; historic branches, tags, and period release
artifacts retain their original notices. Content on `main` was the
2.0 baseline for modern ports and build-system work until it became
the unreleased 2.1 development line (see above). Any later
archive discovery would be handled as a new historic branch rather
than extending this closed series.
