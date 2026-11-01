# Host / modern tooling build (planned)

This note sketches how ModiCom on `main` could be built and
edited with modern tooling (for example CLion driving Make) on
a host OS, without replacing the Borland / DOSBox production
path. It is a plan, not implemented work. For the current DOS
architecture see `docs/design.md`; for Borland rebuild steps
see `BUILD.md`.

## Goal

- Open and build the project under a modern IDE using Make (or
  a thin CMake wrapper that still invokes Make).
- Produce a host binary that can exercise the editor session on
  a modern terminal, for day-to-day development.
- Keep the existing Borland `MAKEFILE`, `MODICOM.PRJ`, and
  DOSBox green suites as the source of truth for DOS behaviour.

## What stays DOS-only

- `src/VIDEO.ASM` assembled with TASM.
- The root Borland `MAKEFILE` and project file.
- Automated greens under `tests/` as they exist today (DOSBox +
  Borland). A host test story is separate follow-on work.

## Why a C video module is required

The live display API is `showline` / `showpage` in `VIDEO.H`,
implemented by `VIDEO.ASM` writing colour text RAM. Host
toolchains will not assemble that file with the period TASM
workflow.

An early C page drawer (`SHOWPAGE.C`) survives only on the
historic branches. It is **not** a drop-in for the current API:

- Historic signature: `showpage(char *sce, int x, int y)`.
- Live signature: `showpage(char *str, char *endptr)` (and a
  separate `showline`), with soft CR and `<` / `+` markers as
  pinned by `TVIDEO`.

A host build should therefore **conditionally compile a new C
video backend** that implements today’s `VIDEO.H` contract. The
historic file is useful only as a sketch of markers and line
walking. Suggested shape: keep `VIDEO.ASM` for DOS objects;
add something like `src/video_host.c` (name TBD) selected by the
host makefile.

## Console and DOS surface

Almost every editor `.C` uses Borland `conio` and related DOS
services, not only the video module. A first host build needs a
compatibility layer (or gradual call-site replacement) for at
least:

| Area | Examples in the tree |
|------|----------------------|
| Screen | `gotoxy`, `window`, `clrscr`, `clreol`, `cputs`, `cprintf`, `putch`, `textattr`, `insline`, `delline` |
| Input | `bioskey` (keyboard, insert latch, Ins/Num/Caps lamps) |
| Process | `_wscroll` in `MODICOM.C` |
| Files | Borland `open` / `read` / `write`, `fnsplit`, `dir.h` paths |

A thin shim over PDCurses, ncurses, or ANSI terminal I/O is
likely less churn for a first cut than rewriting every call
site. Modifier lamps and exact BIOS key codes will be
approximate on a host keyboard.

## Suggested approach

1. Leave the Borland/DOSBox path unchanged and green.
2. Add a **host** makefile (or Make target) that does not run
   TASM and links a C video object instead of `VIDEO.OBJ`.
3. Implement `showline` / `showpage` in C against the chosen
   terminal backend, matching live behaviour closely enough for
   interactive use (full `TVIDEO` parity can wait).
4. Introduce a small conio / `bioskey` / path compatibility
   layer so the rest of `src/` compiles with minimal edits
   (for example behind `MODICOM_HOST` or makefile `CPPFLAGS`).
5. Only later replace aliases with native APIs if a polished
   Unix product is desired.

## Non-goals for a first cut

- Dropping or rewriting the Borland production build.
- Shipping a finished cross-platform product UI.
- Porting the existing DOSBox test matrix to the host in the
  same change set.
- Restoring historic `SHOWPAGE.C` unchanged into `main`.

## Related docs

- `docs/design.md` — current DOS module layout
- `docs/history.md` — note on historic `SHOWPAGE.C`
- `BUILD.md` — Borland rebuild
- `tests/README.md` — DOS automated tests
