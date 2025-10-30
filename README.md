# Printed Droid Teeces32 Logic Display Controller
## ESP32 Enhanced Version 4.1 User Manual

---

### 1. Introduction

This manual provides a complete guide for the **Printed Droid Teeces Logic Display Controller - ESP32 Enhanced Version 4.1**. This advanced controller is designed to run on an ESP32, providing a stable, feature-rich, and highly configurable solution for your R2-D2's logic displays.

The system controls Front Logic Displays (FLD), Rear Logic Display (RLD), and Process State Indicators (PSI). It has been enhanced from its base version to include persistent settings (saved to flash memory), a comprehensive interactive serial menu for configuration, a watchdog timer for stability, and a powerful profile management system.

Version 4.1 introduces user-friendly features like a **Setup Wizard**, a **Hardware Diagnostics** tool, and **Quick Presets** to make configuration easier than ever.

### 2. Hardware Requirements & Wiring

#### 2.1. Required Components
* **Controller:** ESP32-C3 Mini development board.
* **Display Drivers:** MAX7219 LED matrix controllers (daisy-chained).
* **LED Matrices:** 5x9 matrices for Front Logic Displays (FLD) and a 5x27 matrix for the Rear Logic Display (RLD).
* **PSIs (Choose one):**
    * **Analog PSIs:** LED arrays driven by the MAX7219 chain.
    * **Digital PSIs:** Two 26-LED digital strips (e.g., NeoPixel/WS2812B). The default PSI mode for this firmware is **DIGITAL**.

#### 2.2. Default Wiring (ESP32-C3 Mini)
The firmware is pre-configured for the following pinout:

* **Rear Chain (RLD + Rear Analog PSI):**
    * DATA: GPIO7, CLK: GPIO6, CS: GPIO5
* **Front Chain (TFLD + BFLD + Front Analog PSI):**
    * DATA: GPIO4, CLK: GPIO3, CS: GPIO2
* **Digital PSI 1:**
    * DATA: GPIO8
* **Digital PSI 2:**
    * DATA: GPIO9

### 3. Getting Started (First-Time Use)

#### 3.1. Connect to the Controller
1.  Connect to your ESP32-C3 Mini via USB.
2.  Open a serial monitor (like the one in the Arduino IDE).
3.  Set the baud rate to **9600**.
4.  You should see the welcome message for Teeces v4.1. If it's your first time, you'll see a special message.

#### 3.2. Run the Setup Wizard
For the easiest first-time setup, the controller includes a guided wizard.

1.  In the serial monitor, type `*` and press Enter to enter the configuration menu.
2.  At the `Config>` prompt, type `wizard` and press Enter.
3.  The wizard will guide you through the following steps:
    * **Step 1: PSI Type:** Choose between Analog (MAX7219) or Digital (NeoPixel) PSIs.
    * **Step 2: Brightness:** Select a brightness level for dark, normal, or bright environments.
    * **Step 3: PSI Colors:** If you selected Digital PSI, choose a color scheme (e.g., Classic, Movie Accurate, Modern).
    * **Step 4: Save Profile:** Save your settings to one of the user profiles (3, 4, or 5).
4.  After completion, your droid is configured and ready. The wizard will also mark the first-time setup as complete.

### 4. Normal Operation (JawaLite Serial Commands)

The controller responds to standard "JawaLite" protocol commands at 9600 baud. This allows it to be controlled by other droid components (like a Marcduino) or via the serial monitor.

**Format:** `[address][command][argument]<CR>` (where `<CR>` is a carriage return).

#### 4.1. Addresses
| Address | Target |
| :--- | :--- |
| `0` | All displays |
| `1` | Top FLD |
| `2` | Bottom FLD |
| `3` | Rear LD |
| `4` | Front PSI |
| `5` | Rear PSI |

#### 4.2. T Commands (Display State)
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
| `T100` | Text mode (displays text set by `M` command) |

#### 4.3. M Commands (Text)
* `M[text]`: Sets the text to be displayed.
* **Example:** `1MHELLO WORLD` sets the Top FLD to scroll "HELLO WORLD".

#### 4.4. P Commands (Alphabet)
| Command | Function |
| :--- | :--- |
| `P60` | Use Latin alphabet |
| `P61` | Use Aurabesh alphabet |

#### 4.5. R Commands (Random Style)
* `R[0-6]`: Sets the density of the random display mode.
* `0` = Sparse, `6` = Dense.

#### 4.6. S Commands (PSI State)
| Command | Function |
| :--- | :--- |
| `S0` | Test (all on) |
| `S1` | Random mode |
| `S2` | Color 1 |
| `S3` | Color 2 |
| `S4` | Off |

### 5. Interactive Configuration Menu

This is the most powerful feature of the controller. It allows you to customize and save every aspect of your displays.

**To access:** Send a single `*` character in the serial monitor.

You will see a `Config>` prompt. From here, you can enter commands to view, change, and save settings. The status line shows your active profile, PSI mode, and if you have unsaved changes.

#### 5.1. Main Commands
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

#### 5.2. Profile Management
The controller supports 5 profiles for all settings.
* **Profile 1 (Standard):** A fixed, non-editable default profile.
* **Profile 2 (Custom):** A fixed profile set up for digital PSIs in White & Pink.
* **Profiles 3, 4, 5 (User):** Fully customizable and saved persistently.

| Command | Function |
| :--- | :--- |
| `profile show` | Displays the currently active profile number (1-5). |
| `profile load <1-5>`| Loads and activates the specified profile. |
| `profile save` | Saves changes to the current **user profile** (3, 4, or 5 only). |
| `profile reset <3-5>`| Resets a user profile (3, 4, or 5) back to default settings. |

#### 5.3. Quick Presets
Presets are pre-configured settings for common use cases. Applying a preset modifies your *current* profile. You must use `profile save` to keep the changes (on profiles 3-5).

| Preset | Name | Description |
| :--- | :--- | :--- |
| 1 | Bright | Maximum brightness, fast scrolling |
| 2 | Dim | Low brightness for dark environments |
| 3 | KT | Colorful KT mode (White/Pink PSI) |
| 4 | Classic | Original analog PSI look |
| 5 | Rainbow | Colorful digital PSI rotation |

**Usage:**
1.  Type `presets` to see the list.
2.  Type `preset 3` to apply the "KT" preset.
3.  Type `profile save` to save these new settings to your user profile.

#### 5.4. Hardware Diagnostics
If you are troubleshooting, the `diagnostics` command will test all components.
* Flash Memory
* Rear Logic Display
* Front Logic Displays
* Analog PSIs (Rear and Front)
* Digital PSI 1 (GPIO8) and 2 (GPIO9)
* Watchdog Timer

The tool also provides troubleshooting tips for common wiring issues
