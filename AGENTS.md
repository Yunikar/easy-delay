# AGENTS.md

Guidance for coding agents working in this repository. Read it before changing
code. The user-facing docs are [README.md](README.md); the UI rules are in
[DESIGN.md](DESIGN.md).

## What this is

**Easy Delay** is a Windows x64 VST3 delay plugin built with JUCE + CMake. Its
defining contract: it reports **0 samples of latency** to the host, so the host
never applies plug-in delay compensation and never shifts the track. The delay
happens entirely inside the plugin. Do not change this to report real latency —
that would silently destroy the product.

## Commands

Dependencies are not committed. Fetch them once (JUCE 9 + portable CMake into
`third_party/`, which is gitignored):

```powershell
powershell -File tools/fetch-deps.ps1
```

```powershell
$cmake = "third_party\cmake-src\cmake-3.31.6-windows-x86_64\bin\cmake.exe"
& $cmake -S . -B build -G "Visual Studio 17 2022" -A x64
& $cmake --build build --config Release --target EasyDelay_VST3
```

| Task | Command |
|---|---|
| Encoding gate | `node tools/check-encoding.mjs` |
| Build + run validation | `& $cmake --build build --config Release --target EasyDelayCheck`, then run `build\tools\PluginCheck\EasyDelayCheck_artefacts\Release\EasyDelayCheck.exe` |
| Render UI preview | build `EasyDelayPreview` and run it; writes `build/editor-preview.png` |
| DSP-only probe | `EasyDelayProbe` (no VST3 host layer) |

## Rules

- **Text encoding: UTF-8, no BOM.** Never rewrite text files with PowerShell 5.1
  `Get-Content` / `Set-Content`; it decodes UTF-8 as the system code page (GBK
  here) and silently produces mojibake. Use editor tools, or .NET
  `File.ReadAllText` / `WriteAllText` with `UTF8Encoding($false)`. CI runs
  `tools/check-encoding.mjs` and fails otherwise.
- **Do not commit** `build/`, `dist/`, or `third_party/`.
- **UI work follows [DESIGN.md](DESIGN.md)** — one accent colour, three type
  sizes, no gradients/cards/glow. Keep the canvas dark and quiet.
- **Read [docs/decisions-easy-delay.md](docs/decisions-easy-delay.md)** before
  changing latency reporting, startup behaviour, delay-time glide, feedback, or
  the installer. Those decisions have reasons recorded.
- **Verify, don't claim.** Any DSP or host-facing change must leave
  `EasyDelayCheck` green; paste its output in the summary. The checker asserts
  0 latency, dry passthrough at Mix=0, an internally delayed copy matching the
  parameter, and editor creation.
- **Known-intended behaviour:** after playback starts the line is not yet full,
  so the plugin passes dry first and switches to delayed audio. That is the
  zero-PDC trade-off, not a bug.
- **Renderer / real-time code:** no allocations, locks, logging, or file I/O in
  `processBlock`; keep it per-sample cheap.

## Layout

```
Source/            plugin processor + editor
tools/PluginCheck/ VST3 host-side validation (the important test)
tools/DspProbe/    processor-only audio probe
tools/EditorPreview/ offscreen editor screenshot
tools/fetch-deps.ps1, tools/check-encoding.mjs
installer/package/ install.cmd + install.ps1 (system VST3 folder + uninstall entry)
docs/              decisions and test report
```

## Environment notes

- GitHub access from the bundled shell is blocked by TLS; publishing is done
  through the GitHub REST API instead of `git push`.
- The install step writes to `C:\Program Files\Common Files\VST3` and needs
  elevation, so it is a user action, not something an agent can validate here.
