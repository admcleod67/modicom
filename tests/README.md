# ModiCom Tests

This directory contains the initial automated tests for ModiCom.
They are deliberately small and are intended to provide a regression
baseline for work on `main`; they are not yet a complete test suite
for the editor.

## Scope

The automated tests currently cover:

- the string routines in `src/STRINGS.C` (`TSTRINGS.EXE`):
  `replace`, `rtrim`, and `linelen`, including CR and xBase soft
  carriage return (`0x8d`) line endings;
- editor file loading and saving in `src/EDITOR.C` (`TEDITIO.EXE`),
  described below.

The test harness in `TEST.H` is a small set of Borland-compatible
assertion macros. It uses no external test framework.

The tests are not part of the production build. They are not listed
in the root `MAKEFILE` release objects or in `MODICOM.PRJ`, and no
test object is linked into `MODICOM.EXE`.

## Building and Running in DOSBox

From the repository root in DOS, with the Borland C++ 3.1 binary
directory on `PATH`, run:

```text
MAKE -f TESTS\TESTS.MAK rebuild
BUILD\TESTS\TSTRINGS
BUILD\TESTS\TEDITIO
```

If Borland C++ is not installed in `C:\BORLANDC`, adjust `BORLANDC`
in `tests/TESTS.MAK`.

The test program prints the number of assertions and failures. It
exits with status 0 when all tests pass and status 1 when any test
fails. In DOS you can check this immediately afterwards with:

```text
IF ERRORLEVEL 1 ECHO TESTS FAILED
```

To remove only the test build outputs, run:

```text
MAKE -f TESTS\TESTS.MAK clean
```

Test objects and executables are written under `build/tests/`,
which is covered by the repository's existing `/build/` ignore
rule.

## Editor IO Tests (`TEDITIO.EXE`)

`TEDITIO.EXE` links the production `EDITOR.OBJ` together with
`KEYBOARD.OBJ` and `STRINGS.OBJ`, but deliberately not
`VIDEO.OBJ`. Instead, the test file provides no-op `showline` and
`showpage` stubs, so the file loading and saving code can run
without the assembler display routines.

The harness does not call `textedit`. It allocates an edit buffer
of `MAXBYTES` bytes with a guard region on each side, fills the
guards with a known pattern, and points the editor globals at the
buffer. After each case it checks that the guards are unchanged
and that the buffer pointers remain within bounds. This matters
because DOS real mode has no memory protection: an out-of-bounds
write usually corrupts memory silently rather than faulting, so an
unchanged guard is the evidence that an operation stayed within
the buffer.

With no command-line argument, `TEDITIO` runs the safe cases:
loading empty, one-byte, CRLF, LF-only, soft-CR, no-trailing-
newline, and Ctrl+Z-terminated files, and a save round-trip for
the `VERSION_M` build. These are expected to pass.

### Boundary probes

A single boundary probe can be run by passing its identifier, for
example:

```text
BUILD\TESTS\TEDITIO P5
```

Each probe runs in its own process, so a hang or memory corruption
in one probe cannot hide the results of the others. The probes
assert the behaviour a correct implementation should have; a
failing probe is a reproducible finding to be investigated and
fixed separately, not a test to silence.

| ID | Probe |
|----|-------|
| P1 | Empty file with a Ctrl+Z byte immediately before the buffer |
| P2 | Load a 32767-byte file |
| P3 | Load a 32768-byte file |
| P4 | Load a 32769-byte file |
| P5 | Load a file of exactly `MAXBYTES` (49152) bytes |
| P6 | Load a file of `MAXBYTES` + 1 bytes |
| P7 | Load a 50000-byte file |
| P8 | Load a 5000-byte single line |

The nonexistent-file and read/write-error paths are not covered:
on error the editor displays a message and waits for a keypress,
which would hang an unattended test.

## Manual Smoke Test

Automated editor-level tests do not exist yet. Before publishing or
after changes that affect the editor, work through this checklist
in DOSBox using disposable copies of files for any operation that
saves:

1. Run a clean production build with `MAKE clean` followed by
   `MAKE`.
2. Run `MODICOM /?` and confirm the expected `v2.0m` banner and
   `Copyright (c) 1994-2026 Allan McLeod` text.
3. Open a period example file.
4. Toggle help with F1 and back.
5. Exercise PgUp, PgDn, and cursor movement.
6. Insert and delete characters and lines, then abandon the edit
   without saving.
7. Save a disposable copy of a file.
8. Repeat the save check for the non-`VERSION_M` build when that
   variant is next built, because it appends Ctrl+Z whereas the
   `m` variant does not.

Do not modify the files under `examples/` while smoke testing.

## Deferred Test Work

The following are intentionally not covered yet:

- editor cursor and editing-operation tests;
- maximum-length prompt and search-string input;
- randomized or fuzz testing of editing operations;
- fixing any defect exposed by a boundary probe, and the wider
  investigation of the historically reported infrequent crash;
- the nonexistent-file and read/write-error paths (see above);
- the non-`VERSION_M` save variant;
- Borland compiler warning cleanup;
- restructuring `EDITOR.C`.

Those items should be introduced separately, after this baseline,
so that any failure can be attributed to a specific new test or
production change.
