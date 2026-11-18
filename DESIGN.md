# Easy Delay — UI rules

Scope: the plugin editor only (a fixed-size VST3 panel, not a web page).
These rules were written after a de-slop pass; keep them when touching the UI.

## Palette

- Background `#17181b`, single hairline `#2b2d33`, knob face `#232529`.
- Text: primary `#e8e8ea`, muted `#85878e`.
- One accent only: amber `#e8a33c`, used for the live delay readout, the filled
  part of each knob arc, and the "DELAYED" status. Nothing else gets colour.
- No gradients, no glows, no glass, no drop shadows.

## Type

- Three sizes only: 32 px (readout number), 15 px (panel title, knob captions),
  11 px (unit, status). Nothing below 11 px.
- Weights: title and readout bold, everything else regular.
- Live numbers must be monospaced-feeling and right-align their unit, or the
  layout jumps when the delay glides.

## Layout

- 520 x 300 default, resizable between 420 x 250 and 1040 x 600.
- Header: title on the left, "Reset" as a quiet text button on the right.
- One hairline under the header. No other dividers.
- Readout row: big number + "ms" on the left, status on the right.
- Three knobs in one row, captions under each knob, centred under the knob.
- No cards, no rounded panels, no coloured side bars.

## Copy

- Title says what it is ("EASY DELAY"); the one-line subtitle states the one
  behaviour that is unusual ("zero PDC - delayed listening"). Nothing repeats
  the title.
- Buttons name the action ("Reset").
- Status text reports state, not marketing: "DELAYED" / "DRY - BUFFERING".

## Motion

- Knobs only move when the user moves them; no idle animation, no pulses.
- Delay time changes glide (audio behaviour), the UI just follows the value.

## Text and source encoding

- Every tracked text file is UTF-8 **without** BOM.
- Never rewrite text files with PowerShell 5.1 `Get-Content` / `Set-Content`: it
  decodes UTF-8 as the system code page (GBK on this machine) and silently turns
  Chinese into mojibake. Use the editor tools, or .NET `File.ReadAllText` /
  `WriteAllText` with `UTF8Encoding($false)`.
- `node tools/check-encoding.mjs` runs in CI and fails on invalid UTF-8, a BOM,
  or mojibake characters.

Reason: one pass through PowerShell corrupted `README.md`; the damage only
showed up on GitHub, after the release had already been tagged.

