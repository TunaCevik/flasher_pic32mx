# Devlog

Dated, narrative debugging entries — what broke, how it was found, and the PulseView
evidence that proved it. Separate from `SubMachines`/`HSI`/`HardwareBindings`/`Diagrams`,
which describe how the system is *designed* to work; this folder documents what actually
happened while bringing it up.

## Convention

- One file per investigation: `YYYY-MM-DD-short-slug.md` (e.g. `2026-10-10-uart-timing-bug.md`).
- Screenshots/exports referenced by a note go in `assets/`, named with the same date+slug
  prefix as the note that uses them (e.g. `2026-10-10-uart-timing-bug-ch1.png`).
- Each note starts with Obsidian frontmatter tags for cross-cutting search, independent of
  which folder it's in:

  ```yaml
  ---
  tags: [debug, uart, timing-bug]
  ---
  ```

- Raw PulseView `.sr` session captures are not committed by default (see root `.gitignore`)
  — they're large binary files a reviewer would need to open in PulseView to make sense of.
  Export the relevant channel(s) as a PNG/SVG from PulseView and commit that instead; it's
  what actually reads as evidence in a GitHub-rendered markdown file.
