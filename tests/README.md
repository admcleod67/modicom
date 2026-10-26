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
- editor file loading and saving in `src/FILEIO.C` (`TFILEIO.EXE`),
  described below;
- cursor and paging operations in `src/NAVIGATE.C`
  (`TNAVIGAT.EXE`), also described below;
- editing operations in `src/BUFFEDIT.C` (`TBUFFEDT.EXE`), also
  described below;
- keyboard input in `src/KEYBOARD.C`, `prompt` in `src/PROMPT.C`,
  and `findprompt` in `src/SEARCH.C` (`TEDITIN.EXE`), also
  described below;
- status UI helpers in `src/STATUS.C` (`TSTATUS.EXE`), also
  described below;
- Core `ctrlkey` dispatch in `src/TEXTEDIT.C` (`TCTRLKEY.EXE`),
  also described below;
- Core `textedit` / session abort in `src/TEXTEDIT.C`
  (`TSESSION.EXE`), also described below;
- video display routines in `src/VIDEO.ASM` (`TVIDEO.EXE`), also
  described below.

The test harness in `TEST.H` is a small set of Borland-compatible
assertion macros. It uses no external test framework.

`TFILEIO`, `TNAVIGAT`, `TBUFFEDT`, `TSTATUS`, `TCTRLKEY`, and
`TSESSION` also link `THARNESS.OBJ` (`tests/THARNESS.C` /
`THARNESS.H`) for no-op `showline` / `showpage` stubs (and, for
the buffer suites, the shared guarded edit buffer and document
install helpers). `TSESSION` does not call `harness_alloc`:
`textedit` owns its own buffer. `TEDITIN` keeps its own
destination-buffer guards and does not use `THARNESS`.

The tests are not part of the production build. They are not listed
in the root `MAKEFILE` release objects or in `MODICOM.PRJ`, and no
test object is linked into `MODICOM.EXE`.

## Building and Running in DOSBox

From the repository root in DOS, with the Borland C++ 3.1 binary
directory on `PATH`, run:

```text
MAKE -f TESTS\TESTS.MAK rebuild
BUILD\TESTS\TSTRINGS
BUILD\TESTS\TFILEIO
BUILD\TESTS\TNAVIGAT
BUILD\TESTS\TBUFFEDT
BUILD\TESTS\TEDITIN
BUILD\TESTS\TSTATUS
BUILD\TESTS\TCTRLKEY
BUILD\TESTS\TSESSION
BUILD\TESTS\TVIDEO
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

## File I/O Tests (`TFILEIO.EXE`)

`TFILEIO.EXE` exercises File I/O (`loadfile` / `textwrite` in
`src/FILEIO.C`). It links Core plus Status, Navigate, Edit, File
I/O, Search, Prompt, Keyboard, Strings, and `THARNESS`, but
deliberately not `VIDEO.OBJ`. `THARNESS` provides the guarded
edit buffer and no-op `showline` / `showpage` stubs, so the file
loading and saving code can run without the assembler display
routines.

The harness does not call `textedit`. It allocates an edit buffer
of `MAXBYTES` bytes with a guard region on each side, fills the
guards with a known pattern, and points the editor globals at the
buffer. After each case it checks that the guards are unchanged
and that the buffer pointers remain within bounds. This matters
because DOS real mode has no memory protection: an out-of-bounds
write usually corrupts memory silently rather than faulting, so an
unchanged guard is the evidence that an operation stayed within
the buffer.

With no command-line argument, `TFILEIO` runs the safe cases:
loading empty, one-byte, CRLF, LF-only, soft-CR, no-trailing-
newline, and Ctrl+Z-terminated files, an empty file with a Ctrl+Z
byte immediately before the buffer, full and oversized loads at
the `MAXBYTES` limit (with correct `toobig` capping), and a save
round-trip for the `VERSION_M` build. These are expected to pass.

### Boundary probes

A single boundary probe can be run by passing its identifier, for
example:

```text
BUILD\TESTS\TFILEIO P8
```

Each probe runs in its own process, so a hang or memory corruption
in one probe cannot hide the results of the others. The probes
assert the behaviour a correct implementation should have; a
failing probe is a reproducible finding to be investigated and
fixed separately, not a test to silence.

| ID | Probe |
|----|-------|
| P2 | Load a 32767-byte file |
| P3 | Load a 32768-byte file |
| P4 | Load a 32769-byte file |
| P8 | Load a 5000-byte single line |

The nonexistent-file and read/write-error paths are not covered:
on error the editor displays a message and waits for a keypress,
which would hang an unattended test.

## Navigate Tests (`TNAVIGAT.EXE`)

`TNAVIGAT.EXE` uses the same Core-plus-modules and `THARNESS`
design as `TFILEIO.EXE`, but instead of loading files it installs
small documents directly in the editor's internal format (lines
separated by carriage return and NUL, as produced by `loadfile`
from a CRLF file), places the cursor by setting the editor
globals, and calls individual Navigate functions such as
`cursdown`, `nextword`, and `pagedown`. After each operation it
asserts the exact buffer contents, the cursor position, the line
number and count, and the guard bytes.

These tests verify buffer and cursor state only. The display
routines are stubbed here; their cell output is covered by
`TVIDEO.EXE` below, and interactive appearance by the manual
smoke test.

With no command-line argument, `TNAVIGAT` runs the green cases:

| Prefix | Group |
|--------|-------|
| `N` | Cursor navigation: character, line, word, and column movement |
| `V` | Paging: `pageup` and `pagedown` on a six-line document |

A single boundary probe can be run by passing its identifier, for
example `BUILD\TESTS\TNAVIGAT X1`. As with the IO probes, a
failing probe is a reproducible finding to be fixed separately.

| ID | Probe |
|----|-------|
| X1 | Beginning of line on the first line |
| X2 | Last word at the start of the buffer |

## Buffedit Tests (`TBUFFEDT.EXE`)

`TBUFFEDT.EXE` uses the same Core-plus-modules and `THARNESS`
design as `TNAVIGAT.EXE`, calling Buffedit functions such as
`inschar`, `del`, and `delword` against installed documents and
asserting buffer, cursor, and guard state after each operation.

With no command-line argument, `TBUFFEDT` runs the green cases:

| Prefix | Group |
|--------|-------|
| `E` | Editing: insert (including near-full buffer), overwrite, newline, delete (including at buffer end), backspace, tab |

A single boundary probe can be run by passing its identifier, for
example `BUILD\TESTS\TBUFFEDT X3`. As with the other probes, a
failing probe is a reproducible finding to be fixed separately.

| ID | Probe |
|----|-------|
| X3 | Delete word at the final line ending |

Interactive dispatch (`ctrlkey`, Core `abortedit`, and the `textedit`
loop itself) is not covered by these suites; keyboard input to
`prompt` and `getline` is covered by `TEDITIN.EXE`, below.

## Editor Input Tests (`TEDITIN.EXE`)

`TEDITIN.EXE` links the same Core-plus-modules set as the other
editor harnesses (plus Keyboard and Strings; no Video) and tests
`getline` in `src/KEYBOARD.C` together with `prompt` in
`src/PROMPT.C` and `findprompt` in `src/SEARCH.C`.

The test cannot stub `getkey`: `KEYBOARD.OBJ` already defines it,
so a second definition would fail the link. Instead, keystrokes
are scripted through the real BIOS keyboard path by placing key
words in the BIOS keyboard buffer. Short scripts fit in the
15-key buffer and are loaded before the call; longer scripts are
topped up by an `INT 1Ch` timer-tick feeder installed for the
duration of the call. A tick deadline injects Escape and flags a
timeout if feeding stalls, so a scripting failure reports as a
test failure rather than hanging. Every script ends with Return
or Escape, because `getline` busy-waits for a key.

Each case writes into a guard-banded destination buffer and
asserts the return value, the exact resulting string, and the
guard bytes. `getline` accepts at most `maxlen` characters and
then writes its NUL terminator at the current position, so a
caller must provide `maxlen + 1` writable bytes; several cases
exist to pin down that contract.

With no command-line argument, `TEDITIN` runs the green cases:

| Prefix | Group |
|--------|-------|
| `L` | `getline`: typing, Escape, backspace, ignored keys, length limits |
| `R` | `prompt` / `findprompt`: production messages, trimming, return value, file-name and max-length Find limits, Find cancel (`R11`) |

Green `L10` pins the `maxlen + 1` terminator contract for
`getline`; `R10` covers a 73-character Find string into a
`MAXSEARCH + 1` destination matching production `srchstr`.
`R4` pins raw `prompt` Escape behaviour (typed text left in the
caller buffer); Find cancel-safety is covered by `findprompt` in
`R11`.

A single boundary probe can be run by passing its identifier, for
example `BUILD\TESTS\TEDITIN B3`. As with the other suites, a
failing probe is a reproducible finding to be fixed separately.

| ID | Probe |
|----|-------|
| B3 | Non-character keys (Up, F1) in `getline` |

A `prompt` message long enough to make its computed length limit
zero or negative is out of scope: no production message is longer
than 17 characters, so that path cannot be reached today.

## Status Tests (`TSTATUS.EXE`)

`TSTATUS.EXE` links the same Core-plus-modules and `THARNESS`
set as the buffer suites and exercises Status helpers in
`src/STATUS.C`. Display stubs come from `THARNESS` (no
`VIDEO.OBJ`). Status-line text is checked by reading colour text
RAM after each call; `helptoggle` is checked via `maxrow` / `y`
only (help-screen content is not asserted).

`msg` waits for Space: the suite pushes one Space into the BIOS
keyboard buffer before the call (same path as `TEDITIN`, without
the timer-tick feeder).

With no command-line argument, `TSTATUS` runs the green cases:

| ID | Case |
|----|------|
| `S1` | `helptoggle` switches `maxrow` between 17 and 24; `y` becomes 1 |
| `S2` | `showpos` shows the current line and column on the status line |
| `S3` | `scoreboard` with a short file name shows `Edit: ` plus the name |
| `S4` | `scoreboard` when `toobig` shows the truncated-file message |
| `S5` | `statmsg` shows Ins/Caps (and blank Num) for constructed bits |
| `S6` | `msg` returns after a scripted Space |

`helpscreen` content and `showstatus` (live modifier latch) are
not covered yet.

## Core ctrlkey Tests (`TCTRLKEY.EXE`)

`TCTRLKEY.EXE` links the same Core-plus-modules and `THARNESS`
set as the buffer suites and calls `ctrlkey()` directly (the
production `K_CTRL_K` entry in `textedit` is not driven here).
Command and prompt keys are pushed into the BIOS keyboard
buffer before each call, as in `TSTATUS` / `TEDITIN`, without
the timer-tick feeder.

Cases pin Find / Find-again dispatch and the process-static
search string via buffer and cursor globals. `readfile` /
`writefile` via `ctrlkey` remain out of scope here (session
save/abort is covered by `TSESSION`).

With no command-line argument, `TCTRLKEY` runs the green cases:

| ID | Case |
|----|------|
| `C1` | Find-again with empty search string misses and returns after Space |
| `C2` | Find hit moves the cursor to the match |
| `C3` | Find Escape leaves the cursor unchanged |
| `C4` | Find miss leaves the cursor unchanged (Space clears `msg`) |
| `C5` | Find then Find-again advances to the second match |
| `C6` | Lowercase `f` Find alias behaves like `F` |
| `C7` | Unknown second key is a no-op |

## Core textedit / session Tests (`TSESSION.EXE`)

`TSESSION.EXE` links the same Core-plus-modules and `THARNESS`
set (stubs only; no `harness_alloc`) and calls `textedit()` with
a short BIOS keyboard script. Observables after return are the
return code, the `changed` flag, and disk side effects — not the
edit buffer (`textedit` frees it before returning).

Paths use a temporary file under `BUILD\TESTS\` with an
extension so load does not append `.prg`. Missing files give an
empty session without a Space-wait. Ctrl-K, Find, file prompts,
and help-screen content are out of scope.

With no command-line argument, `TSESSION` runs the green cases:

| ID | Case |
|----|------|
| `E1` | Clean Esc: return 0, `changed` false, no file written |
| `E2` | Dirty Esc+Y: abort without writing; `changed` stays true |
| `E3` | Dirty Esc+N then Esc+Y: decline abort, then abort; no file |
| `E4` | Dirty Ctrl-End: writes one byte `A` (`VERSION_M`) and quits |

## Video Display Tests (`TVIDEO.EXE`)

`TVIDEO.EXE` links the production `VIDEO.OBJ` — assembled from
`src/VIDEO.ASM` by the rule in `TESTS.MAK` — and calls `showline`
and `showpage` directly. No other production objects are linked
and there are no display stubs: the assembler routines are the
subject under test.

The routines write character/attribute cells to colour text-mode
video RAM, deriving the start address from the BIOS cursor
position. Each case therefore sets BIOS mode 3, fills the
visible 4000-byte page and the memory after it with a sentinel
cell, places the hardware cursor with the BIOS, and copies the
region to a snapshot buffer immediately after the call, before
any assertion can print to the same screen. Assertions inspect
the snapshot only. The original video mode is restored on exit.

Monochrome mode 7 is not covered: the video segment is cached
inside `VIDEO.ASM` after first use and cannot be reset from C,
and the editor's DOSBox environment is colour. Mono display
remains covered by the manual smoke test.

A rendered line ends with a `<` cell in column 79 when more
buffer follows, and a space when the line is the last in the
buffer; tests set the `endptr` argument deliberately to match
the editor call being characterised.

`TVIDEO` runs the green cases only (all former video boundary
probes have been promoted):

| Prefix | Group |
|--------|-------|
| `S` | `showline`: end markers, CR and soft-CR, overlong lines, offsets, column 79, exact fit (`S10`), empty at column 79 (`S11`) |
| `P` | `showpage`: multi-line pages, offsets, clearing, empty buffer, bottom-row bounds (`P7`–`P9`) |

## Manual Smoke Test

Full interactive tests do not exist yet. Before publishing or
after changes that affect the editor, work through this checklist
in DOSBox using disposable copies of files for any operation that
saves:

1. Run a clean production build with `MAKE clean` followed by
   `MAKE`.
2. Run `MODICOM /?` and confirm the expected `v2.1m` banner and
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

- randomized or fuzz testing of editing operations;
- fixing any defect exposed by a boundary probe, and the wider
  investigation of the historically reported infrequent crash;
- the nonexistent-file and read/write-error paths (see above);
- the non-`VERSION_M` save variant;
- Borland compiler warning cleanup;
- folding `TEDITIN` destination-buffer guards into `THARNESS`
  (buffer suites already share `THARNESS`; input suite does not);
- renaming green/probe case IDs (`N` / `E` / `V` / `X` and
  peers) if clearer module-aligned names are wanted;
- remaining Status gaps such as `showstatus` / help-screen
  content; `ctrlkey` read/write file paths (Find paths are in
  `TCTRLKEY`; session save/abort is in `TSESSION`).

Those items should be introduced separately, after this baseline,
so that any failure can be attributed to a specific new test or
production change.
