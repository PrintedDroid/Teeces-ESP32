# Teeces ESP32 Logic Display Controller

**ESP32-basierte Teeces-Controller für R2-D2 Logic Displays, PSI-Indikatoren und Holoprojektoren — mit voller JawaLite-Protokoll-Unterstützung.**

Dieses Repository enthält **zwei eigenständige Arduino-Sketche** für unterschiedliche
Hardware-Plattformen. Beide bringen die Logic Displays, PSIs (und – in der neueren
Variante – Holoprojektoren) eines Astromech mit flüssigen Animationen, dynamischer
Farbsteuerung und einem interaktiven Konfigurationsmenü zum Leben.

| | **ESP32-S3 Zero** | **ESP32-C3 Mini** |
|---|---|---|
| **Firmware** | v4.7 | v4.3 |
| **Status** | Aktuell / empfohlen | Stabil / Vorgänger |
| **Logic Displays** | TFLD, BFLD, RLD (MAX7219) | TFLD, BFLD, RLD (MAX7219) |
| **PSI** | Analog (MAX7219) **oder** Digital (NeoPixel) | Analog (MAX7219) **oder** Digital (NeoPixel) |
| **Holoprojektoren** | ✅ 3 NeoPixel-Holos, 9 Effekte | ❌ nicht enthalten |
| **MarcDuino-Holo-Befehle** | ✅ `*ON`/`*OF`/`*ST`/`*RD` | ❌ |
| **FlthyHP-Protokoll** | ✅ A/F/R/T/S | ❌ |
| **JawaLite-Erweiterungen** | L / C / H / T99 | L / C / T99 |
| **Adressbereich** | 0–8 (inkl. Holos 6/7/8) | 0–5 |
| **Profile** | 5 (2 fix, 3 frei) im Flash | 5 (2 fix, 3 frei) im Flash |
| **Config-CLI** | ✅ inkl. Wizard, Diagnose, Presets | ✅ inkl. Wizard, Diagnose, Presets |
| **Sketch-Ordner** | `Teeces_S3Mini_v4.6/` | `Teeces_ESP32_C3_Mini_v4.2/` |
| **README** | [Teeces_S3Mini_v4.6/README.md](Teeces_S3Mini_v4.6/README.md) | [Teeces_ESP32_C3_Mini_v4.2/README.md](Teeces_ESP32_C3_Mini_v4.2/README.md) |

> **Kurz gesagt:** Wer Holoprojektoren ansteuern will oder ein neues Board kauft,
> nimmt **v4.7 (ESP32-S3 Zero)**. Wer bereits ein ESP32-C3-Mini-Board verbaut hat
> und keine Holos braucht, bleibt bei **v4.3 (ESP32-C3 Mini)**.

---

## Welche Variante für mich?

- **Du baust neu / willst Holoprojektoren:** → `Teeces_S3Mini_v4.6`
  Der S3 hat mehr GPIOs und Flash, die zusätzlichen NeoPixel-Holo-Ausgänge und die
  Holo-Protokolle (MarcDuino `*`-Befehle, FlthyHP) gibt es nur hier.
- **Du hast schon ein ESP32-C3-Mini-Board / keine Holos nötig:** → `Teeces_ESP32_C3_Mini_v4.2`
  Funktionsgleich bei Displays und PSIs, nur ohne Holo-Teil.

> ⚠️ **Die Sketche sind nicht pin-kompatibel.** Die Pinbelegung unterscheidet sich
> zwischen den Boards — vor dem Flashen die Pin-Tabelle im jeweiligen Ordner-README
> prüfen.

---

## Gemeinsame Features (beide Sketche)

- **Multi-Display-Steuerung** – Front Logic Displays (TFLD/BFLD) und Rear Logic Display (RLD) über daisy-chained MAX7219.
- **Duale PSI-Unterstützung** – Analog (MAX7219) oder Digital (NeoPixel/WS2812B) mit Echtzeit-Farbsteuerung.
- **JawaLite-Protokoll** – volle Kompatibilität plus Erweiterungen:
  - `L` – direkte Helligkeitssteuerung (0–15)
  - `C` – digitale PSI-Farbe (Pattern + Farbindex)
  - `T99` – alle Effekte stoppen
- **Effekte** – Alarm, Leia, March, Failure, Random, Bargraph, Text-Scrolling (Latin + Aurabesh).
- **Profilverwaltung** – 5 Profile (Profil 1+2 fix, 3–5 frei editierbar) persistent im Flash.
- **Interaktives Config-Menü** – Zugriff über `*` im seriellen Monitor (9600 Baud): `show`, `set`, `wizard`, `diagnostics`, `presets`, Shorthands (`p1`–`p5`, `s`, `w`, `d`, `q`), Verbose-Debug-Modus.
- **Setup-Wizard & Hardware-Diagnose** für die Erstinbetriebnahme und Fehlersuche.

### Farbpalette (für `C`-Command, Index 0–11)

| Index | Farbe | Index | Farbe |
| :---: | :--- | :---: | :--- |
| 0 | RED | 6 | ORANGE |
| 1 | GREEN | 7 | PURPLE |
| 2 | BLUE | 8 | PINK |
| 3 | YELLOW | 9 | LIME |
| 4 | CYAN | 10 | SKYBLUE |
| 5 | MAGENTA | 11 | WHITE |

---

## Schnellstart

1. Arduino IDE mit **ESP32-Board-Support** einrichten
   (Board Manager URL: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`).
2. Bibliotheken installieren: **Adafruit NeoPixel** (1.10.0+), **LedControl** (1.0.6+).
3. Passenden Ordner öffnen und Board wählen:
   - S3 Zero (v4.7) → „ESP32S3 Dev Module" (oder „Waveshare ESP32-S3-Zero"), **USB CDC On Boot: Enabled**
   - C3 Mini (v4.3) → „ESP32C3 Dev Module", **USB CDC On Boot: Enabled**
4. Flashen, seriellen Monitor mit **9600 Baud** öffnen, `??` für Hilfe oder `*` fürs Config-Menü.

Details (Pinbelegung, vollständige Befehlsreferenz, Troubleshooting) im jeweiligen
Ordner-README.

---

## Changelog-Highlights v4.7 / v4.3

Eine Code-Review beider Sketche brachte mehrere Fehler und Diskrepanzen zutage. Die
folgenden Punkte wurden in **v4.7** (S3 Zero) bzw. **v4.3** (C3 Mini) behoben:

**Beide Sketche**
- ✅ **Adressvalidierung** – ungültige Adressen (`>8` bzw. `>5`) werden jetzt mit Fehlermeldung quittiert statt stillschweigend verschluckt.
- ✅ **Farbpalette-Doku** – README- und Header-Tabellen entsprechen jetzt der echten `colorPalette[]` (jeder `C`-Command-Index lieferte zuvor eine andere als die dokumentierte Farbe).
- ✅ **Watchdog** – WDT-Reset in jeder Diagnose-Stufe (S3 zusätzlich in der Boot-Sequenz); blockierende Wizard-Delays reduziert.
- ✅ **Versionsstrings** in Menü/Wizard/Boot vereinheitlicht; Magic Numbers durch benannte Konstanten ersetzt; `const`-Korrektheit verbessert.

**Nur v4.7 (S3 Zero)**
- ✅ **Out-of-Bounds-Lesezugriff** auf das Holo-State-Namensarray bei States 5–8 behoben (Array auf 9 Einträge + Bereichsprüfung).
- ✅ **H-Command** unterstützt jetzt den vollen Bereich **0–8** (vorher 0–4; States 5–8 nur über FlthyHP).
- ✅ **`HOLO_COLOR` (`xH2`)** behält die zuletzt gesetzte Farbe statt sie mit Weiß zu überschreiben.
- ✅ **FlthyHP-Fallback** verschluckt keine gültigen JawaLite-Befehle mehr (fehlendes `\r` ergänzt).
- ✅ **GPIO1/GPIO3 (UART0)** – Compile-Time-Warnung, falls „USB CDC On Boot" nicht aktiv ist.

**Nur v4.3 (C3 Mini)**
- ✅ **Forward-Declarations** für `doCcommand`/`doLcommand` ergänzt.
- ✅ **`L4`/`L5`** setzen jetzt auch im Digital-Modus die NeoPixel-Helligkeit (vorher nur analog wirksam).
- ✅ **M-Command** sanitisiert den Text wie Boot-Texte (nicht-druckbare Bytes gefiltert).
- ✅ **Irreführende Farb-Aliase** (`BLUE == YELLOW`, `RED == GREEN`) entfernt – Code nutzt jetzt `PSI_COLOR1`/`PSI_COLOR2`.
- ✅ **`buildCommand`-Puffer** wird beim Wechsel in/aus dem Config-Menü zurückgesetzt.

---

## Credits & Lizenz

- **Software:** Printed-Droid.com
- **Hardware:** Printed Droid Teeces32 Boards (ESP32-S3 / C3 Mini, MAX7219, NeoPixel)
- **Bibliotheken:** LedControl, Adafruit NeoPixel, ESP32 Arduino Core

> ⚠️ **Sicherheitshinweis:** Dieses Projekt umfasst elektrische Komponenten und
> LED-Displays. Für korrekte Verkabelung, ausreichend dimensionierte Stromversorgung
> und sicheren Betrieb ist der Erbauer selbst verantwortlich. **Bau auf eigenes Risiko.**

---

*Aktuelle Updates und Community-Support: https://github.com/PrintedDroid/Teeces-ESP32*

**May the Force be with your build!**
