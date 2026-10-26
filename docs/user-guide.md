# ModiCom user guide

How to run and use the DOS editor on `main` (banner `v2.1` /
`v2.1m`). For rebuilding from source, see `BUILD.md`. For
architecture, see `docs/design.md`.

## Starting and leaving

From a DOS prompt (or DOSBox with the project mounted):

```text
MODICOM <filename>
MODICOM /?
```

Exactly one file name is required. `/?` prints the banner and
usage and exits. If the file does not exist, the session opens
an empty buffer; saving will create the file. If the name has
no extension, ModiCom appends `.prg` when loading.

| Action | Keys |
|--------|------|
| Save and exit | Ctrl+W, or Ctrl+End |
| Abort without saving | Esc — if you have changed the file, confirm with Y (or any key except N); N returns to editing |
| Toggle help strip | F1 |

Save-and-exit writes only when the buffer is dirty. There is no
separate “save without leaving” key outside Ctrl+K write-file
(below). Abort never writes the main file.

## Screen layout

- **Row 1 — status.** File name (`Edit: …`), Ins / Num / Caps
  lamps, and line / column (`L` / `C`). If the loaded file was
  larger than the edit buffer, the name is replaced by a
  “File too large…” warning.
- **Rows 2–8 — help (optional).** Shown when the edit area is
  17 lines tall; F1 hides it and enlarges the text area.
- **Remaining rows — text.** The current page of the file.

Messages such as “Not found” or I/O errors appear on the status
row and wait for **Space** before restoring the scoreboard.

## Editing keys

These match the on-screen help strip (F1). `^` means Ctrl.

### Cursor and paging

| Action | Keys |
|--------|------|
| Character left / right | Left / Right |
| Line up / down | Up / Down |
| Word left / right | Home / End |
| Start / end of line | Ctrl+Left / Ctrl+Right |
| Page up / down | PgUp / PgDn |

### Insert and delete

| Action | Keys |
|--------|------|
| Toggle insert / overwrite | Ins |
| Delete character under cursor | Del |
| Backspace | Backspace |
| Delete word | Ctrl+T |
| Delete line | Ctrl+Y |
| Insert blank line | Ctrl+N |
| New line | Enter |
| Tab | Tab |

Ordinary printable keys insert (or overwrite) at the cursor.
Ctrl+C rings the bell and does nothing else.

### Find and files (Ctrl+K)

Press **Ctrl+K**, then a second key:

| Second key | Action |
|------------|--------|
| F (or f, or Ctrl+F) | Find — prompt `Find? `, then search forward |
| L (or l, or Ctrl+L) | Find again — repeat the last search string |
| R (or r, or Ctrl+R) | Read file — insert another file at the cursor |
| W (or w, or Ctrl+W) | Write file — prompt for a name and write the buffer |

Escape cancels a Find or file-name prompt. A failed Find shows
`** Not found **` until you press Space. Read/write prompts that
fail similarly wait for Space after an error message.

## File and buffer behaviour

- **Capacity.** The edit buffer holds up to 49152 bytes. Larger
  files load truncated with the status warning above.
- **Line endings.** On disk, lines end with LF. Soft carriage
  return (`0x8d`), used in some xBase text, is treated as a line
  end on screen and in navigation.
- **`v2.1m` vs `v2.1`.** The `m` build (default on this tree)
  does not append Ctrl+Z when saving. The non-`m` build does.
  The banner shows which you are running.

## Environment

ModiCom is a DOS real-mode program. Period PCs and DOSBox are
the intended hosts. Colour text mode is assumed for normal use.
Rebuild steps, DOSBox mounts, and the dual `VERSION_M` switch
are documented in `BUILD.md`. Sample period files for manual
checks live under `examples/` — prefer disposable copies when
saving.
