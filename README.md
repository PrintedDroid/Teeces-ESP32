# Teeces ESP32 Logic Display Controller

**ESP32-based Teeces controllers for R2-D2 logic displays, PSI indicators, and holoprojectors — with full JawaLite protocol support.**

This repository contains **two standalone Arduino sketches** for different hardware
platforms. Both bring an astromech's logic displays, PSIs (and — in the newer
variant — holoprojectors) to life with smooth animations, dynamic color control,
and an interactive configuration menu.

| | **ESP32-S3 Zero** | **ESP32-C3 Mini** |
|---|---|---|
| **Firmware** | v4.7 | v4.3 |
| **Status** | Current / recommended | Stable / predecessor |
| **Logic Displays** | TFLD, BFLD, RLD (MAX7219) | TFLD, BFLD, RLD (MAX7219) |
| **PSI** | Analog (MAX7219) **or** Digital (NeoPixel) | Analog (MAX7219) **or** Digital (NeoPixel) |
| **Holoprojectors** | ✅ 3 NeoPixel holos, 9 effects | ❌ not included |
| **MarcDuino holo commands** | ✅ `*ON`/`*OF`/`*ST`/`*RD` | ❌ |
| **FlthyHP protocol** | ✅ A/F/R/T/S | ❌ |
| **JawaLite extensions** | L / C / H / T99 | L / C / T99 |
| **Address range** | 0–8 (incl. holos 6/7/8) | 0–5 |
| **Profiles** | 5 (2 fixed, 3 free) in flash | 5 (2 fixed, 3 free) in flash |
| **Config CLI** | ✅ incl. wizard, diagnostics, presets | ✅ incl. wizard, diagnostics, presets |
| **Sketch folder** | `Teeces_ESP32_S3_Zero_v4.7/` | `Teeces_ESP32_C3_Mini_v4.3/` |
| **README** | [Teeces_ESP32_S3_Zero_v4.7/README.md](Teeces_ESP32_S3_Zero_v4.7/README.md) | [Teeces_ESP32_C3_Mini_v4.3/README.md](Teeces_ESP32_C3_Mini_v4.3/README.md) |

> **In short:** If you want holoprojectors or are buying a new board, use
> **v4.7 (ESP32-S3 Zero)**. If you already have an ESP32-C3 Mini board and don't
> need holos, stay on **v4.3 (ESP32-C3 Mini)**.

---

## Which variant should I use?

- **Building new / want holoprojectors:** → `Teeces_ESP32_S3_Zero_v4.7`
  The S3 has more GPIOs and flash; the extra NeoPixel holo outputs and the holo
  protocols (MarcDuino `*` commands, FlthyHP) exist only here.
- **Already have an ESP32-C3 Mini board / no holos needed:** → `Teeces_ESP32_C3_Mini_v4.3`
  Functionally identical for displays and PSIs, just without the holo part.

> ⚠️ **The sketches are not pin-compatible.** Pin assignments differ between the
> boards — check the pin table in the respective folder README before flashing.

---

## Shared features (both sketches)

- **Multi-display control** – Front Logic Displays (TFLD/BFLD) and Rear Logic Display (RLD) via daisy-chained MAX7219.
- **Dual PSI support** – Analog (MAX7219) or Digital (NeoPixel/WS2812B) with real-time color control.
- **JawaLite protocol** – full compatibility plus extensions:
  - `L` – direct brightness control (0–15)
  - `C` – digital PSI color (pattern + color index)
  - `T99` – stop all effects
- **Effects** – Alarm, Leia, March, Failure, Random, Bargraph, text scrolling (Latin + Aurabesh).
- **Profile management** – 5 profiles (profiles 1+2 fixed, 3–5 freely editable) persisted in flash.
- **Interactive config menu** – via `*` in the serial monitor (9600 baud): `show`, `set`, `wizard`, `diagnostics`, `presets`, shortcuts (`p1`–`p5`, `s`, `w`, `d`, `q`), verbose debug mode.
- **Setup wizard & hardware diagnostics** for first-time setup and troubleshooting.

### Color palette (for the `C` command, index 0–11)

| Index | Color | Index | Color |
| :---: | :--- | :---: | :--- |
| 0 | RED | 6 | ORANGE |
| 1 | GREEN | 7 | PURPLE |
| 2 | BLUE | 8 | PINK |
| 3 | YELLOW | 9 | LIME |
| 4 | CYAN | 10 | SKYBLUE |
| 5 | MAGENTA | 11 | WHITE |

---

## Quick start

1. Set up the Arduino IDE with **ESP32 board support**
   (Board Manager URL: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`).
2. Install libraries: **Adafruit NeoPixel** (1.10.0+), **LedControl** (1.0.6+).
3. Open the matching folder and select the board:
   - S3 Zero (v4.7) → "ESP32S3 Dev Module" (or "Waveshare ESP32-S3-Zero"), **USB CDC On Boot: Enabled**
   - C3 Mini (v4.3) → "ESP32C3 Dev Module", **USB CDC On Boot: Enabled**
4. Flash, open the serial monitor at **9600 baud**, type `??` for help or `*` for the config menu.

Details (pin assignments, full command reference, troubleshooting) are in the
respective folder README.

---

## Changelog highlights v4.7 / v4.3

A code review of both sketches surfaced several bugs and discrepancies. The
following were fixed in **v4.7** (S3 Zero) and **v4.3** (C3 Mini):

**Both sketches**
- ✅ **Address validation** – invalid addresses (`>8` / `>5` respectively) now return an error instead of being silently dropped.
- ✅ **Color palette docs** – README and header tables now match the actual `colorPalette[]` (previously every `C`-command index produced a different color than documented).
- ✅ **Digital PSI brightness** now follows the 0–15 setting (incl. `L4`/`L5`), applied at boot, and **defaults to 10/15** (previously digital PSIs always ran at full brightness).
- ✅ **One-time migration** forces the PSI output to digital on the first boot of this firmware (prevents dark PSIs from a leftover analog value in flash); a later deliberate analog choice is preserved.
- ✅ **Watchdog** – WDT reset in every diagnostics step (S3 also in the boot sequence); blocking wizard delays reduced.
- ✅ **Version strings** unified across menu/wizard/boot; magic numbers replaced with named constants; `const`-correctness improved.

**Only v4.7 (S3 Zero)**
- ✅ **Out-of-bounds read** on the holo state-name array for states 5–8 fixed (array extended to 9 entries + range check).
- ✅ **H-command** now supports the full range **0–8** (previously 0–4; states 5–8 only via FlthyHP).
- ✅ **`HOLO_COLOR` (`xH2`)** keeps the last set color instead of forcing white.
- ✅ **FlthyHP fallback** no longer swallows valid JawaLite commands (missing `\r` added).
- ✅ **GPIO1/GPIO3 (UART0)** – compile-time warning if "USB CDC On Boot" is not enabled.
- ℹ️ **Rear chain CLK/CS pins** are now `CLK=GPIO5, CS=GPIO6` (swapped vs. earlier) to match an SN74AHCT125-based level-shifter wiring; see the folder README pin table.

**Only v4.3 (C3 Mini)**
- ✅ **Forward declarations** for `doCcommand`/`doLcommand` added.
- ✅ **M-command** sanitizes text like boot texts (non-printable bytes filtered).
- ✅ **Misleading color aliases** (`BLUE == YELLOW`, `RED == GREEN`) removed – the code now uses `PSI_COLOR1`/`PSI_COLOR2`.
- ✅ **`buildCommand` buffer** is reset when entering/leaving the config menu.

---

## Credits & License

- **Software:** Printed-Droid.com
- **Hardware:** Printed Droid Teeces32 boards (ESP32-S3 Zero / C3 Mini, MAX7219, NeoPixel)
- **Libraries:** LedControl, Adafruit NeoPixel, ESP32 Arduino Core

> ⚠️ **Safety notice:** This project involves electrical components and LED displays.
> Correct wiring, an adequately rated power supply, and safe operation are the
> builder's responsibility. **Build at your own risk.**

---

*Latest updates and community support: https://github.com/PrintedDroid/Teeces-ESP32*

**May the Force be with your build!**
