# Modicom Tests

This directory contains the initial automated tests for Modicom.
They are deliberately small and are intended to provide a regression
baseline for work on `main`; they are not yet a complete test suite
for the editor.

## Scope

The automated tests currently cover the string routines in
`src/STRINGS.C`:

- `replace`
- `rtrim`
- `linelen`, including CR and xBase soft carriage return (`0x8d`)
  line endings

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

The following are intentionally not covered by this initial suite:

- editor-operation tests;
- an editor-level DOS harness with stubbed `showline`/`showpage`;
- guard-byte tests for reads or writes outside the edit buffer;
- boundary tests for empty files and files around 32767 and
  49152 bytes;
- maximum-length prompt and search-string input;
- randomized or fuzz testing of editing operations;
- investigation of the historically reported infrequent crash;
- Borland compiler warning cleanup;
- restructuring `EDITOR.C`.

Those items should be introduced separately, after this baseline,
so that any failure can be attributed to a specific new test or
production change.
