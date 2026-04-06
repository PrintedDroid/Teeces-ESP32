# Teeces ESP32 Logic Display Controller v4.6
**Advanced ESP32-S3 Mini based Teeces display controller for R2-D2 logic displays, PSI indicators, and holoprojectors**

## Project Overview

This enhanced controller brings your R2-D2's logic displays to life with smooth animations, dynamic PSI control, holoprojector effects, and comprehensive JawaLite protocol support. Designed for builders who demand professional results with maximum flexibility.

### Key Features

- **Multi-Display Control** - Front Logic Displays (TFLD/BFLD), Rear Logic Display (RLD)
- **Dual PSI Support** - Analog (MAX7219) or Digital (NeoPixel/WS2812B) with real-time color control
- **3 Holoprojector Outputs** - NeoPixel strips with 9 LED effects (rainbow, flicker, Leia, etc.)
- **JawaLite Protocol** - Full compatibility + extensions (L/C/H/T99 commands)
- **MarcDuino Holo Commands** - *ON/*OF/*ST/*RD for holo control
- **FlthyHP Protocol** - A/F/R/T/S command parsing for holo integration
- **Dynamic PSI Colors** - 12-color palette with instant switching via C-Command
- **Brightness Control** - Direct L-Command for real-time intensity adjustment
- **Advanced Effects** - Alarm, Leia, March, Failure animations
- **Profile Management** - 5 persistent profiles with flash storage
- **Interactive Config Menu** - Complete serial CLI with shortcuts and verbose mode
- **Hardware Diagnostics** - Built-in testing for all components
- **Text Scrolling** - Latin + Aurabesh alphabets
- **Setup Wizard** - Guided first-time configuration

---

## Changelog

### Version 4.6 (2026-04-06)

**Holoprojectors, Pin Correction & Platform Upgrade**

#### Holoprojector Support
- 3 NeoPixel Holoprojector outputs (Front, Rear, Top) with 9 LED effects
- H-Command: Direct Holo control (6H1=on, 0H0=off, 7H3=rainbow)
- MarcDuino * commands for Holo control (*ON00-03, *OF00-03, *ST00, *RD)
- FlthyHP protocol parser (A/F/R/T/S commands)
- Holo auto-twitch animation
- Configurable LEDs per Holo (default: 7 NeoPixels each)

#### Platform & Pin Changes
- Upgraded from ESP32-C3 Mini to ESP32-S3 Mini
- Corrected all pin assignments to match actual S3 Mini board wiring
- I2C pins (SDA=GPIO9, SCL=GPIO10) defined as reserved for future expansion
- All GPIO references in diagnostics and documentation updated

#### Breaking Changes
None! All existing JawaLite commands work unchanged. Holoprojector commands are additive.

### Version 4.2 (2025-11-05)

**JawaLite Protocol Extensions & CLI Enhancements**

This major update extends the JawaLite protocol with three powerful new commands and significantly improves the user experience with better error handling, shortcuts, and debugging tools.

#### New JawaLite Commands

**1. L-Command - Brightness Control**
- Direct brightness control without entering config mode
- Format: `[address]L[0-15]`
- Examples:
  - `0L10` - Set all displays to brightness 10
  - `3L15` - Set RLD to maximum brightness
  - `4L5` - Set Front PSI to brightness 5
- **Use Case**: Dynamic brightness adjustment during lightsaber battles, environmental changes
- **Performance**: Instant application via `applyBrightnessSettings()`

**2. C-Command - Digital PSI Color Control**
- Real-time color changes for digital PSI strips
- Format: `[address]C[pattern][colorIndex]`
- Examples:
  - `4C111` - Front PSI Pattern 1 = White (11)
  - `5C28` - Rear PSI Pattern 2 = Pink (8)
  - `4C14` - Front PSI Pattern 1 = Blue (4)
- **Color Palette**: 12 colors (RED, ORANGE, YELLOW, GREEN, BLUE, INDIGO, CYAN, PURPLE, PINK, MAGENTA, LIME, WHITE)
- **Use Case**: Dynamic color synchronization with events, music, droid behaviors

**3. T99-Command - Stop All Effects**
- Explicit command to stop all running effects
- Format: `0T99`
- Returns displays to normal operation (EFF_NORM)
- Cleaner alternative to implicit state changes

#### Enhanced CLI Features

**4. Descriptive Error Messages**
- **Before**: Silent BEL tone (0x07)
- **After**: Clear error messages with format examples
  ```
  ERROR: Unknown command 'X'. Valid: T/M/P/R/S/L/C/D/H. Type '??' for help.
  ERROR: L command requires argument (0-15 for brightness)
  ERROR: Color index must be 0-11
  ```
- Includes BEL tone for backwards compatibility

**5. Quick Help System**
- Type `??` or `help` in normal operation mode
- Displays complete JawaLite protocol reference
- Shows format, addresses, commands, and examples
- No need to consult documentation

**6. Command Confirmation Feedback**
- All new commands provide visual confirmation
  ```
  Command: L, Address: 0, Brightness: 10 [All displays & PSIs]
  Command: C, Address: 4, Pattern: 1, Color: 11 [Front PSI Color1]
  ```

**7. Shorthand Commands**
Power-user shortcuts for faster configuration:
- `p1-p5` - Load profiles instantly (instead of `profile load X`)
- `s` - Show settings (instead of `show`)
- `w` - Run wizard (instead of `wizard`)
- `d` - Run diagnostics (instead of `diagnostics`)
- `q` - Quit config menu (instead of `exit`)

**8. Verbose Debug Mode**
- Toggle with `verbose on/off` in config menu
- Real-time command processing diagnostics
- Shows:
  - Command received (string + length)
  - Address parsing
  - Command character extraction
  - Argument parsing
  - Execution timing (milliseconds)
- Example output:
  ```
  [DEBUG] Received: '0T1' (3 bytes)
  [DEBUG] Parsed address: 0
  [DEBUG] Command: T
  [DEBUG] Argument: 1
  [DEBUG] Executing command...
  Command: T, Address: 0, Arg: 1
  [DEBUG] Command completed in 2 ms
  ```
- **Use Cases**: Troubleshooting parsing errors, monitoring execution time, learning protocol structure

#### Implementation Details

- **Global Variable**: `verboseMode` (bool) tracks debug state
- **Enhanced Parser**: `parseCommand()` with debug checkpoints
- **Special Parsing**: C-Command uses pattern + colorIndex extraction
- **Shortcuts**: 14 new aliases in `handleConfigCommands()`
- **Zero Performance Impact**: Debug code only executes when verbose mode enabled

#### Feature Metrics

| Feature | Lines of Code | Performance Impact |
|---------|---------------|-------------------|
| L-Command | 50 | None (applies brightness directly) |
| C-Command | 52 | None (updates on next PSI cycle) |
| T99-Command | 5 | None (calls existing exitEffects()) |
| Error Messages | 45 | Minimal (only on errors) |
| Verbose Mode | 60 | Zero when disabled |
| Shortcuts | 70 | None (simple command aliases) |
| **Total** | **282** | **Zero when not in use** |

---

### Version 4.1 (2025-10-20)

**User Experience & Configuration Management**

#### User-Friendly Features

**1. Setup Wizard**
- Guided first-time configuration
- PSI type selection (Analog/Digital)
- Brightness level setup
- Color scheme configuration
- Profile saving

**2. Hardware Diagnostics**
- Tests all components automatically
- Flash memory validation
- Display testing (RLD, TFLD, BFLD)
- PSI validation (Analog & Digital)
- Watchdog timer check
- Troubleshooting tips for wiring issues

**3. Quick Presets**
- Preset 1: Bright - Maximum brightness, fast scrolling
- Preset 2: Dim - Low brightness for dark environments
- Preset 3: KT - Colorful KT mode (White/Pink PSI)
- Preset 4: Classic - Original analog PSI look
- Preset 5: Rainbow - Colorful digital PSI rotation

**4. Smart Suggestions**
- Context-aware tips after commands
- Configuration hints
- Next-step recommendations

**5. Enhanced Parameter Validation**
- Range checking with helpful messages
- Invalid value prevention
- Current value comparison

#### Profile Management

- **Profile 1 (Standard)**: Read-only default profile
- **Profile 2 (White/Pink)**: Read-only digital PSI preset
- **Profiles 3-5 (User)**: Fully customizable, persistently saved

#### Performance Improvements

- Optimized EEPROM wear leveling
- Reduced unnecessary flash writes
- Improved boot time
- Better memory management

---

## Hardware Requirements

### Core Components
- **ESP32-S3 Mini** development board
- **MAX7219 LED Matrix Controllers** (daisy-chained)
  - 5x9 matrices for Front Logic Displays (TFLD/BFLD)
  - 5x27 matrix for Rear Logic Display (RLD)
- **PSIs** (choose one):
  - **Analog**: LED arrays driven by MAX7219 chain
  - **Digital**: Two 26-LED NeoPixel/WS2812B strips (default)
- **3x NeoPixel Holoprojector strips** (7 LEDs each, configurable)
- **5V Power Supply** (adequate for displays - minimum 2A recommended)

### Recommended Carrier Board
- **Teeces Logic Display Board** - Professional carrier board

---

## Pin Configuration

ESP32-S3 Mini Pin Assignments:

| GPIO | Function | Type |
|------|----------|------|
| 1 | Front Chain DATA | MAX7219 SPI |
| 2 | Front Chain CS | MAX7219 SPI |
| 3 | Front Chain CLK | MAX7219 SPI |
| 4 | Rear Chain DATA | MAX7219 SPI |
| 5 | Rear Chain CS | MAX7219 SPI |
| 6 | Rear Chain CLK | MAX7219 SPI |
| 7 | Digital PSI 1 (Front) | NeoPixel |
| 8 | Digital PSI 2 (Rear) | NeoPixel |
| 9 | SDA (reserved) | I2C |
| 10 | SCL (reserved) | I2C |
| 11 | Front Holo | NeoPixel |
| 12 | Rear Holo | NeoPixel |
| 13 | Top Holo | NeoPixel |
| RX/TX | Serial Terminal | UART 9600 |

---

## Installation

### Method 1: Arduino IDE Setup

1. **Install ESP32 board support:**
   - File > Preferences > Additional Board Manager URLs
   - Add: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Tools > Board > Board Manager > Search "ESP32" > Install

2. **Board Configuration for ESP32-S3 Mini:**
   - Board: "ESP32S3 Dev Module" (or "Lolin S3 Mini")
   - Upload Speed: "921600"
   - USB CDC On Boot: "Enabled"
   - Flash Size: "4MB" or "8MB"
   - Partition Scheme: "Default"

3. **Required Libraries:**
   Install via Arduino Library Manager:
   - Adafruit NeoPixel (1.10.0+)
   - LedControl (1.0.6+)

### Method 2: Upload Pre-Compiled Binary

1. Download latest `.bin` file from releases
2. Use ESP Flash Download Tool or esptool.py
3. Flash at address 0x0

---

## 4. Normal Operation (JawaLite Serial Commands)

The controller responds to standard "JawaLite" protocol commands at 9600 baud. This allows it to be controlled by other droid components (like a Marcduino) or via the serial monitor.

**Format:** `[address][command][argument]<CR>` (where `<CR>` is a carriage return).

### 4.1. Addresses
| Address | Target |
| :--- | :--- |
| `0` | All displays |
| `1` | Top FLD |
| `2` | Bottom FLD |
| `3` | Rear LD |
| `4` | Front PSI |
| `5` | Rear PSI |
| `6` | Front Holo |
| `7` | Rear Holo |
| `8` | Top Holo |

### 4.2. T Commands (Display State)
| Command | Function |
| :--- | :--- |
| `T0` | Test mode (all LEDs on) |
| `T1` | Random mode |
| `T2` / `T3` / `T5` | Alarm effect |
| `T4` | Failure effect |
| `T6` | Leia effect |
| `T10` | Star Wars text |
| `T11` | March effect |
| `T20` | Off (display blank) |
| `T92` | Bargraph mode |
| `T99` | Stop all effects, return to normal operation |
| `T100` | Text mode (displays text set by `M` command) |

### 4.3. M Commands (Text)
- `M[text]`: Sets the text to be displayed.
- **Example:** `1MHELLO WORLD` sets the Top FLD to scroll "HELLO WORLD".

### 4.4. P Commands (Alphabet)
| Command | Function |
| :--- | :--- |
| `P60` | Use Latin alphabet |
| `P61` | Use Aurabesh alphabet |

### 4.5. R Commands (Random Style)
- `R[0-6]`: Sets the density of the random display mode.
- `0` = Sparse, `6` = Dense.

### 4.6. S Commands (PSI State)
| Command | Function |
| :--- | :--- |
| `S0` | Test (all on) |
| `S1` | Random mode |
| `S2` | Color 1 |
| `S3` | Color 2 |
| `S4` | Off |

### 4.7. L Commands (Brightness Control)

The L-Command provides direct, immediate brightness control without entering the configuration menu. This is especially useful for dynamic lighting adjustments (e.g., during lightsaber battles or environmental changes).

**Format:** `[address]L[brightness]`

| Parameter | Description | Valid Range |
| :--- | :--- | :--- |
| `address` | Target component | 0-5 (see section 4.1) |
| `brightness` | Intensity level | 0 (Off) to 15 (Maximum) |

**Examples:**
```
0L10   → Set all displays and PSIs to brightness 10
3L15   → Set Rear Logic Display to maximum brightness
4L5    → Set Front PSI to brightness 5
0L0    → Turn off all displays
```

**Address Mapping:**
- `0` - All displays and both analog PSIs
- `1` - Top FLD (Front Logic Display)
- `2` - Bottom FLD
- `3` - RLD (Rear Logic Display)
- `4` - Front PSI (analog only)
- `5` - Rear PSI (analog only)

**Use Cases:**
- **Dynamic Environments**: Adjust brightness based on ambient light
- **Lightsaber Combat**: Dim displays during battles
- **Power Saving**: Reduce brightness when idle
- **Show Synchronization**: Match brightness to audio/visual cues

### 4.8. C Commands (Digital PSI Color Control)

The C-Command allows real-time color changes for digital PSI strips without entering configuration mode. This enables dynamic color synchronization with events, music, or other droid behaviors.

**Format:** `[address]C[pattern][colorIndex]`

| Parameter | Description | Valid Range |
| :--- | :--- | :--- |
| `address` | PSI target | 4 (Front PSI) or 5 (Rear PSI) |
| `pattern` | Color slot | 1 (Color1) or 2 (Color2) |
| `colorIndex` | Color from palette | 0-11 (see color table below) |

**Color Palette:**
| Index | Color | Index | Color |
| :---: | :--- | :---: | :--- |
| 0 | RED | 6 | CYAN |
| 1 | ORANGE | 7 | PURPLE |
| 2 | YELLOW | 8 | PINK |
| 3 | GREEN | 9 | MAGENTA |
| 4 | BLUE | 10 | LIME |
| 5 | INDIGO | 11 | WHITE |

**Examples:**
```
4C111  → Front PSI, Pattern 1 = White (11)
5C28   → Rear PSI, Pattern 2 = Pink (8)
4C14   → Front PSI, Pattern 1 = Blue (4)
5C10   → Rear PSI, Pattern 1 = Red (0)
```

**Use Cases:**
- **Movie Accurate**: `4C111` + `5C111` (White on both)
- **KT Mode**: `4C111` + `4C28` (White + Pink on Front)
- **Rainbow Mode**: Cycle through color indices dynamically
- **Event Sync**: Change colors based on droid mood or actions

**Note:** Color changes apply to **digital PSI strips only**. For analog PSI brightness, use the L-Command (4.7).

### 4.9. H Commands (Holoprojector Control)

The H-Command provides direct control over the 3 NeoPixel holoprojector outputs.

**Format:** `[address]H[state]`

| Parameter | Description | Valid Range |
| :--- | :--- | :--- |
| `address` | Holo target | 0=all, 6=Front, 7=Rear, 8=Top |
| `state` | Effect | 0-8 (see table below) |

**States:**
| State | Effect |
| :---: | :--- |
| 0 | Off |
| 1 | On (white) |
| 2 | Color |
| 3 | Rainbow cycle |
| 4 | Flicker |
| 5 | Leia effect |
| 6 | Color cycle |
| 7 | Dim pulse |
| 8 | Short circuit |

**Examples:**
```
0H1    → All holos on (white)
6H3    → Front holo rainbow
7H0    → Rear holo off
8H4    → Top holo flicker
0H0    → All holos off
```

### 4.10. MarcDuino Holo Commands

Standard MarcDuino holo protocol is supported for compatibility with existing systems.

| Command | Function |
| :--- | :--- |
| `*ON00` | All holos on |
| `*ON01` | Front holo on |
| `*ON02` | Rear holo on |
| `*ON03` | Top holo on |
| `*OF00` | All holos off |
| `*OF01` | Front holo off |
| `*OF02` | Rear holo off |
| `*OF03` | Top holo off |
| `*ST00` | Reset all holos |
| `*RD` | Random holo flicker |

### 4.11. Quick Help

Type `??` or `help` in normal operation mode to display a quick reference of all JawaLite commands directly in the serial monitor.

**Example Output:**
```
Format: [address][command][argument]
  Address: 0=All, 1=TopFLD, 2=BottomFLD, 3=RLD, 4=FPSI, 5=RPSI
           6=FrontHolo, 7=RearHolo, 8=TopHolo

Commands:
  T[arg]  - Display state (0=test, 1=random, 20=off, 99=stop effects, 100=text)
  M[text] - Display text message
  P[arg]  - Alphabet (60=Latin, 61=Aurabesh)
  R[arg]  - Random style (0-6)
  S[arg]  - PSI state (0=test, 1=random, 2=color1, 3=color2, 4=off)
  L[arg]  - Brightness (0-15)
  C[p][c] - PSI color: pattern(1-2) + color(0-11)
  H[arg]  - Holo (0=off, 1=on, 2=color, 3=rainbow, 4=flicker)

MarcDuino Holo commands:
  *ON00-03  Holo on (0=all, 1=front, 2=rear, 3=top)
  *OF00-03  Holo off
  *ST00     Reset all holos
  *RD       Random flicker

Examples:
  0T1      All displays to random mode
  1MHELLO  Show 'HELLO' on top FLD
  0L10     Set all brightness to 10
  4C111    Front PSI pattern 1 = color 11 (white)
  6H3      Front holo rainbow
  0H1      All holos on
```

---

## 5. Interactive Configuration Menu

This is the most powerful feature of the controller. It allows you to customize and save every aspect of your displays.

**To access:** Send a single `*` character in the serial monitor.

You will see a `Config>` prompt. From here, you can enter commands to view, change, and save settings. The status line shows your active profile, PSI mode, and if you have unsaved changes.

### 5.1. Main Commands
| Command | Function |
| :--- | :--- |
| `help` | Shows a detailed list of commands and examples. |
| `show` | Displays all settings for the currently active profile. |
| `wizard` | Runs the first-time setup assistant. |
| `diagnostics` | Runs a hardware test on all displays and PSIs. |
| `presets` | Shows a list of 5 Quick Presets. |
| `preset <1-5>` | Applies a Quick Preset (e.g., `preset 3`). |
| `colors` | Lists the 12 available colors for digital PSIs and their index numbers. |
| `set <param> <val>` | Changes a setting (see section 5.4). |
| `profile ...` | Manages profiles (see section 5.2). |
| `exit` | Exits the config menu and resumes normal operation. |

### 5.2. Profile Management

The controller supports 5 profiles for all settings.
- **Profile 1 (Standard):** A fixed, non-editable default profile.
- **Profile 2 (Custom):** A fixed profile set up for digital PSIs in White & Pink.
- **Profiles 3, 4, 5 (User):** Fully customizable and saved persistently.

| Command | Function |
| :--- | :--- |
| `profile show` | Displays the currently active profile number (1-5). |
| `profile load <1-5>`| Loads and activates the specified profile. |
| `profile save` | Saves changes to the current **user profile** (3, 4, or 5 only). |
| `profile reset <3-5>`| Resets a user profile (3, 4, or 5) back to default settings. |

### 5.3. Quick Presets

Presets are pre-configured settings for common use cases. Applying a preset modifies your *current* profile. You must use `profile save` to keep the changes (on profiles 3-5).

| Preset | Name | Description |
| :--- | :--- | :--- |
| 1 | Bright | Maximum brightness, fast scrolling |
| 2 | Dim | Low brightness for dark environments |
| 3 | KT | Colorful KT mode (White/Pink PSI) |
| 4 | Classic | Original analog PSI look |
| 5 | Rainbow | Colorful digital PSI rotation |

**Usage:**
```
Config> presets         ← Show list
Config> preset 3        ← Apply KT preset
Config> profile save    ← Save to current profile
```

### 5.4. Hardware Diagnostics

If you are troubleshooting, the `diagnostics` command will test all components.
- Flash Memory
- Rear Logic Display (GPIO 4/6/5)
- Front Logic Displays (GPIO 1/3/2)
- Analog PSIs (Rear and Front)
- Digital PSI 1 (GPIO7) and 2 (GPIO8)
- Holoprojectors (GPIO 11/12/13)
- Watchdog Timer

The tool also provides troubleshooting tips for common wiring issues.

### 5.5. Shorthand Commands

For faster configuration, convenient shorthand commands:

| Shorthand | Full Command | Description |
| :--- | :--- | :--- |
| `p1` | `profile load 1` | Load profile 1 |
| `p2` | `profile load 2` | Load profile 2 |
| `p3` | `profile load 3` | Load profile 3 |
| `p4` | `profile load 4` | Load profile 4 |
| `p5` | `profile load 5` | Load profile 5 |
| `s` | `show` | Display all settings |
| `w` | `wizard` | Run setup wizard |
| `d` | `diagnostics` | Run hardware diagnostics |
| `q` | `quit` / `exit` | Exit config menu |

**Example Usage:**
```
Config> p3        ← Loads profile 3
Config> s         ← Shows all settings
Config> d         ← Runs diagnostics
Config> q         ← Exits config menu
```

### 5.6. Verbose Debug Mode

The verbose mode provides detailed debug information about command processing, which is useful for troubleshooting and development.

**Commands:**
```
Config> verbose on      ← Enable debug output
Config> verbose off     ← Disable debug output
Config> verbose         ← Check current status
```

**Debug Output Example:**
```
Config> verbose on
✓ Verbose mode ON - Debug output enabled

Config> exit
> 0T1

[DEBUG] Received: '0T1' (3 bytes)
[DEBUG] Parsed address: 0
[DEBUG] Command: T
[DEBUG] Argument: 1
[DEBUG] Executing command...

Command: T, Address: 0, Arg: 1
[DEBUG] Command completed in 2 ms
```

**Use Cases:**
- **Troubleshooting**: Identify parsing errors or invalid commands
- **Development**: Monitor command execution timing
- **Learning**: Understand how JawaLite protocol is processed
- **Integration**: Debug communication with external controllers

**Note:** Verbose mode affects both normal operation (JawaLite commands) and the config menu. Disable it for cleaner output during normal use.

---

## Troubleshooting

### Common Issues Quick Reference

| Problem | Quick Fix |
|---------|-----------|
| No serial output | Check baud rate (9600), verify USB connection |
| Displays not working | Verify MAX7219 wiring, check power supply |
| PSI not responding | Check PSI mode (analog/digital), verify pin connections |
| Holos not working | Check GPIO 11/12/13, verify 5V power to NeoPixel strips |
| Random crashes | Update to latest version, check power supply stability |
| Configuration lost | Use `profile save` command, check EEPROM status |

### Hardware Issues

**Displays Not Working:**
- Check MAX7219 wiring: Front (DATA=GPIO1, CLK=GPIO3, CS=GPIO2), Rear (DATA=GPIO4, CLK=GPIO6, CS=GPIO5)
- Verify power supply (5V, adequate current)
- Test with `diagnostics` command
- Check SPI connections

**PSI Not Responding:**
- Verify PSI mode (analog/digital) with `show` command
- Check pin connections (GPIO7/GPIO8 for digital)
- Test with `4S1` (Front PSI random) command
- Verify NeoPixel power (5V) for digital PSIs

**Holos Not Working:**
- Check NeoPixel wiring (Front=GPIO11, Rear=GPIO12, Top=GPIO13)
- Verify 5V power to holo strips
- Test with `0H1` (all holos on) or `*ON00`
- Check LED count configuration (default: 7 per holo)

### Software Issues

**Configuration Problems:**
```
Config> show            ← Verify settings
Config> profile reset 3 ← Reset profile to defaults
Config> save            ← Save configuration
```

**Serial Communication Issues:**
- Set baud rate to 9600
- Select correct COM port
- Check USB cable quality
- Press EN/RST button if ESP32 doesn't respond

---

## System Monitoring

### Status Information

```
Config> show            ← Complete configuration display
Config> profile show    ← Active profile
Config> colors          ← Color palette reference
```

### Debug Mode

```
Config> verbose on      ← Enable detailed logging
> 0T1                   ← Test commands
Config> verbose off     ← Disable logging
```

---

## Performance Optimization

### Response Time
- L-Command: Instant brightness application
- C-Command: Applied on next PSI update cycle
- H-Command: Instant holo state change
- T99-Command: Immediate effect stop

### Memory Usage
- Flash: ~380KB program + 64KB EEPROM
- RAM: Dynamic allocation for displays + holo animations
- No memory leaks

---

## Future Expansion

### Planned Features
- I2C PCA9685 servo control for holo pan/tilt
- CAN bus integration for AstroCan Pro X system
- WebUI control via AstroCan Gateway
- Enhanced animation effects
- OTA (Over-The-Air) updates

---

## Support

### Getting Help

1. **Check this README** for common solutions
2. **Review Serial Monitor** output (9600 baud) for error messages
3. **Use `diagnostics`** command to test hardware
4. **Enable verbose mode** for detailed debugging

### Diagnostic Commands

```
Config> diagnostics     ← Complete hardware test
Config> show            ← Configuration verification
Config> verbose on      ← Enable debug mode
```

---

## License & Credits

### Project Credits
- **Software Development**: Printed-Droid.com
- **Hardware Compatibility**: Printed Droid Teeces32 Boards (ESP32-S3 Mini, MAX7219, NeoPixel)

### Open Source Libraries
- **LedControl**: MAX7219 LED control
- **Adafruit NeoPixel**: WS2812B/NeoPixel LED control
- **ESP32 Arduino Core**: ESP32-S3 support

### Disclaimer

**IMPORTANT SAFETY NOTICE**

This project involves electrical components and LED displays. Users are responsible for:

- Proper electrical safety and insulation
- Adequate power supply sizing and protection
- Safe assembly and operation
- Compliance with local electrical codes
- Testing all functions before final installation

**BUILD AT YOUR OWN RISK.** Ensure proper knowledge of electronics and safety practices. The authors assume no responsibility for damage, injury, or malfunction resulting from use of this design.

---

**May the Force be with your build!**

*For the latest updates and community support, visit: https://github.com/PrintedDroid/Teeces-ESP32*
