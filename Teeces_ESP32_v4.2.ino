/**
 * ===============================================================================================
 * Printed Droid Teeces Logic Display Controller - ESP32 Enhanced Version 4.2
 * ===============================================================================================
 *
 * This document summarizes the complete features of the enhanced ESP32-based controller for
 * R2-D2 style Logic Displays (FLD, RLD) and Process State Indicators (PSI).
 * The code has evolved significantly from its base version to include advanced configuration,
 * stability features, expanded hardware support, and JawaLite protocol extensions.
 *
 *
 * ===============================================================================================
 * 1. CORE FUNCTIONALITY & HARDWARE
 * ===============================================================================================
 *
 * PURPOSE:
 * - Controls Front Logic Displays (FLD), Rear Logic Display (RLD), and PSI lights.
 *
 * CORE TECHNOLOGY:
 * - The primary control method uses MAX7219 LED matrix drivers via SPI communication.
 * - The system has been extended to support digital (NeoPixel/WS2812B) PSIs as a
 * configurable alternative to the classic MAX7219-driven "analog" PSIs.
 *
 * HARDWARE REQUIREMENTS:
 * - ESP32-C3 Mini development board.
 * - MAX7219 LED matrix controllers (multiple devices in a daisy chain).
 * - 5x9 LED matrices for Front Logic Displays (FLD).
 * - 5x27 LED matrix for Rear Logic Display (RLD).
 * - LED arrays for front and rear "analog" PSIs.
 * - Optional: Two 26-LED digital strips for front and rear "digital" PSIs.
 *
 * WIRING (ESP32-C3 Mini):
 * Rear Chain (RLD + Rear PSI):      DATA=GPIO7, CLK=GPIO6, CS=GPIO5
 * Front Chain (TFLD + BFLD + PSI):  DATA=GPIO4, CLK=GPIO3, CS=GPIO2
 * Digital PSI 1 (configurable as Front/Rear): DATA=GPIO8
 * Digital PSI 2 (configurable as Front/Rear): DATA=GPIO9
 *
 *
 * ===============================================================================================
 * 2. FEATURE SET EVOLUTION
 * ===============================================================================================
 *
 * BASE FEATURES (v1.5):
 * - JawaLite protocol compatible serial commands at 9600 baud.
 * - Multiple display modes: Random, Text, Bargraph, Test, Off.
 * - Special effects: Alarm, March, Leia, Failure.
 * - Dual alphabet support: Latin and Aurabesh.
 * - Analog PSI color animation with random "stuck" behavior.
 *
 * ENHANCEMENTS (v3.0+):
 * - PERSISTENT CONFIGURATION: All settings are saved to the ESP32's flash memory
 * (using the Preferences library) and are retained after power loss.
 * - INTERACTIVE SERIAL MENU: A powerful configuration menu is accessible by sending '*'
 * via the serial monitor, allowing live changes to all settings.
 * - WATCHDOG TIMER (WDT): For maximum stability, a WDT is implemented to automatically
 * reboot the controller if the software freezes.
 * - PROFILE MANAGEMENT SYSTEM: 5 distinct profiles for all settings.
 * - Profile 1 (Standard): A fixed, non-editable default profile.
 * - Profile 2 (Custom): A fixed profile showcasing digital PSIs in White & Pink.
 * - Profiles 3, 4, 5 (User): Fully customizable and saved persistently.
 *
 * BUGFIXES (v3.5):
 * - Fixed buffer overflow in command parsing
 * - Added boot sequence timeout to prevent infinite loops on hardware failure
 * - Increased watchdog timeout to prevent spurious resets in config menu
 * - Improved code maintainability with helper functions and named constants
 * - Added error handling for flash memory operations
 *
 * CHANGES IN v4.0:
 * - PSI output mode is now GLOBAL (applies to all profiles)
 * - Default PSI mode is DIGITAL
 * - Profile 2: White/Pink on both PSI strips
 * - On-demand profile loading (memory optimized)
 * - Magic numbers replaced with constants
 * - Improved validation and error handling
 * - Better serial monitor interface
 *
 * NEW IN v4.1:
 * - ✨ Setup Wizard for first-time configuration (command: wizard)
 * - 🔧 Hardware Diagnostics tool (command: diagnostics)
 * - ⚡ 5 Quick Presets: Bright, Dim, KT, Classic, Rainbow (command: presets)
 * - 💡 Smart Suggestions with context-aware tips
 * - ✓ Enhanced parameter validation with helpful hints
 * - Status line in config menu showing unsaved changes
 * - First-time user detection with welcome message
 *
 * NEW IN v4.2:
 * - ⚡ L-Command: Direct brightness control (0L10 = all displays brightness 10)
 * - 🎨 C-Command: Digital PSI color control (4C111 = front PSI color 11/white)
 * - ⏹  T99-Command: Stop all effects (0T99 = stop all effects immediately)
 * - 💬 Enhanced CLI: Descriptive error messages instead of silent BEL tones
 * - ❓ Quick Help: Type ?? or help for complete JawaLite reference
 * - ⌨  Shortcuts: p1-p5, s, w, d, q for faster configuration
 * - 🐛 Verbose Mode: Toggle with "verbose on/off" for debug output
 * - ✓ Command Confirmation: All new commands provide visual feedback
 * ===============================================================================================
 * 3. SERIAL COMMAND REFERENCE (JAWALITE PROTOCOL)
 * ===============================================================================================
 *
 * Format: [address][command][argument]<CR>
 *
 * ADDRESSES:
 * 0 = All displays      4 = Front PSI
 * 1 = Top FLD           5 = Rear PSI
 * 2 = Bottom FLD        
 * 3 = Rear LD
 *
 * T COMMANDS (DISPLAY STATES):
 * T0   = Test mode (all LEDs on)
 * T1   = Random mode
 * T2/3/5 = Alarm effect
 * T4   = Failure effect
 * T6   = Leia effect
 * T10  = Star Wars text
 * T11  = March effect
 * T20  = Off
 * T92  = Bargraph mode
 * T99  = Stop all effects [NEW v4.2]
 * T100 = Text mode
 *
 * M COMMANDS (TEXT):
 * M[text] = Set display text (e.g., "1MHELLO WORLD")
 *
 * P COMMANDS (ALPHABET):
 * P60 = Latin alphabet
 * P61 = Aurabesh alphabet
 *
 * R COMMANDS (RANDOM STYLE):
 * R[0-6] = Set random pattern style (0=sparse, 6=dense)
 *
 * S COMMANDS (PSI STATE):
 * S0 = Test (all on)    S3 = Color 2
 * S1 = Random mode      S4 = Off
 * S2 = Color 1
 *
 * L COMMANDS (BRIGHTNESS CONTROL) [NEW v4.2]:
 * L[0-15] = Set brightness (e.g., "0L10" = all displays brightness 10)
 * Addresses: 0=all, 1=TFLD, 2=BFLD, 3=RLD, 4=FPSI, 5=RPSI
 *
 * C COMMANDS (DIGITAL PSI COLOR) [NEW v4.2]:
 * C[pattern][colorIndex] = Set PSI color (e.g., "4C111" = Front PSI pattern 1 color 11/white)
 * Format: [address 4 or 5]C[pattern 1 or 2][color 0-11]
 * Color palette: 0=RED, 1=ORANGE, 2=YELLOW, 3=GREEN, 4=BLUE, 5=INDIGO,
 *                6=CYAN, 7=PURPLE, 8=PINK, 9=MAGENTA, 10=LIME, 11=WHITE
 *
 *
 * ===============================================================================================
 * 4. INTERACTIVE CONFIGURATION MENU GUIDE
 * ===============================================================================================
 *
 * 1. Open the Serial Monitor at 9600 baud.
 * 2. Send a single '*' character to enter the menu.
 *
 * MENU COMMANDS:
 * - profile show: Displays the currently active profile number (1-5).
 * - profile load <1-5>: Loads and activates the specified profile.
 * - profile save: Saves any changes made to the current user profile (3, 4, or 5).
 * - profile reset <3-5>: Resets a user profile back to the standard (Profile 1) settings.
 *
 * - show: Displays all settings for the currently active profile.
 * - set <param> <value>: Changes a setting for the active profile.
 * (e.g., "set scroll_speed 50", "set psi_output 1")
 * - colors: Lists the 12 available colors and their index numbers for digital PSI configuration.
 * - wizard: Runs first-time setup assistant
 * - diagnostics: Hardware testing tool
 * - presets: Show quick presets, preset <1-5>: Apply preset
 * - exit: Exits the configuration menu and resumes normal operation.
 *
 * SHORTCUTS [NEW v4.2]:
 * - p1-p5: Quick profile load (e.g., "p3" = profile load 3)
 * - s: Show all settings (shorthand for "show")
 * - w: Run wizard (shorthand for "wizard")
 * - d: Run diagnostics (shorthand for "diagnostics")
 * - q: Quit config menu (shorthand for "exit")
 *
 * DEBUG MODE [NEW v4.2]:
 * - verbose on: Enable detailed debug output for command processing
 * - verbose off: Disable debug output
 * - verbose: Check current verbose mode status
 *
 */


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <Preferences.h>
#include <esp_task_wdt.h>
#include <LedControl.h>
#include <Adafruit_NeoPixel.h>
#include "font_data.h"

// =======================================================================================
// CONSTANTS
// =======================================================================================

#define NUM_PROFILES 5
#define PROFILE_MAGIC 0x52324432
#define NUM_PRESETS 5

// Pin Definitions
#define REAR_DATA_PIN   7
#define REAR_CLK_PIN    6
#define REAR_CS_PIN     5
#define FRONT_DATA_PIN  4
#define FRONT_CLK_PIN   3
#define FRONT_CS_PIN    2
#define DIGITAL_PSI1_PIN 8
#define DIGITAL_PSI2_PIN 9

// Device Configuration
#define FDEV    3
#define FPSIDEV 2
#define RPSIDEV 3
#define DIGITAL_PSI_LED_COUNT 26

// Display Dimensions
#define RLD_WIDTH 27
#define FLD_WIDTH 9

// Text & Command Constants
#define LETTERWIDTH   5
#define MAXSTRINGSIZE 64
#define CMD_MAX_LENGTH 64
#define BOOT_TEXT_SIZE 32

// Timeouts
#define BOOT_SEQUENCE_TIMEOUT_MS 10000
#define WDT_TIMEOUT_MS 10000
#define DIAGNOSTICS_DELAY_MS 500

// PSI Constants
#define HPROW 5
#define HPPAT1 0b10101010
#define HPPAT2 0b01010101

// Default Delays
#define DEFAULT_LOGIC_DELAY 150
#define DEFAULT_SCROLL_SPEED 55
#define DEFAULT_PSI_WIPE_DELAY 75

// Grid Patterns
#define GRID_ALL_ON  0xFFFFFFFFL
#define GRID_ALL_OFF 0x00000000L
#define GRID_LEFT_HALF_FLD  31L
#define GRID_RIGHT_HALF_FLD (~15L)
#define GRID_LEFT_HALF_RLD  16383L
#define GRID_RIGHT_HALF_RLD (~8191L)

// Enums
enum DisplayEffect { EFF_NORM, EFF_ALARM, EFF_MARCH, EFF_LEIA, EFF_FAILURE };
enum DisplayState { STATE_RANDOM, STATE_TEXT, STATE_BARGRAPH, STATE_TEST, STATE_OFF };
enum PsiState { PSI_RANDOM = 0, PSI_COLOR1 = 1, PSI_COLOR2 = 2, PSI_TEST = 3, PSI_OFF = 4 };
enum Alphabet { ALPHA_LATIN, ALPHA_AURABESH };
enum PsiOutputType { PSI_ANALOG, PSI_DIGITAL };

// Color Aliases
#define BLUE   PSI_COLOR1
#define RED    PSI_COLOR2
#define YELLOW PSI_COLOR1
#define GREEN  PSI_COLOR2

// Color Palette
struct ColorEntry {
  const char* name;
  uint32_t value;
};

const ColorEntry colorPalette[] PROGMEM = {
  {"RED", 0xFF0000}, {"GREEN", 0x00FF00}, {"BLUE", 0x0000FF}, {"YELLOW", 0xFFFF00},
  {"CYAN", 0x00FFFF}, {"MAGENTA", 0xFF00FF}, {"ORANGE", 0xFF4500}, {"PURPLE", 0x800080},
  {"PINK", 0xFF1493}, {"LIME", 0x32CD32}, {"SKYBLUE", 0x87CEEB}, {"WHITE", 0xFFFFFF}
};
const int numColors = sizeof(colorPalette) / sizeof(colorPalette[0]);

// Preset configurations
struct PresetConfig {
  const char* name;
  const char* description;
  byte brightness;
  PsiOutputType psiMode;
  byte psiColor1;
  byte psiColor2;
  byte logicStyle;
  unsigned int scrollSpeed;
};

const PresetConfig presets[NUM_PRESETS] PROGMEM = {
  {"Bright", "Maximum brightness, fast scrolling", 15, PSI_DIGITAL, 11, 2, 4, 40},
  {"Dim", "Low brightness for dark environments", 3, PSI_DIGITAL, 2, 0, 4, 55},
  {"KT", "Colorful KT mode (White/Pink PSI)", 8, PSI_DIGITAL, 11, 8, 4, 45},
  {"Classic", "Original analog PSI look", 5, PSI_ANALOG, 0, 0, 4, 55},
  {"Rainbow", "Colorful digital PSI rotation", 10, PSI_DIGITAL, 4, 6, 5, 50}
};

// PSI Patterns (PROGMEM)
const uint8_t psi_pattern1_indices[] PROGMEM = {0,2,4,6,8,11,13,15,16,18,20,22,24};
const uint8_t psi_pattern2_indices[] PROGMEM = {1,3,5,7,9,10,12,14,17,19,21,23,25};
const uint8_t psi_row0[] PROGMEM = {0,1,2,3};
const uint8_t psi_row1[] PROGMEM = {4,5,6,7,8,9};
const uint8_t psi_row2[] PROGMEM = {10,11,12,13,14,15};
const uint8_t psi_row3[] PROGMEM = {16,17,18,19,20,21};
const uint8_t psi_row4[] PROGMEM = {22,23,24,25};
const uint8_t* const psi_row_indices[] PROGMEM = {psi_row0, psi_row1, psi_row2, psi_row3, psi_row4};
const uint8_t psi_row_lengths[] PROGMEM = {4,6,6,6,4};
const byte psiPatterns[] PROGMEM = {HPPAT1,HPPAT2,HPPAT1,HPPAT2,HPPAT1,HPPAT2,HPPAT1,HPPAT2,HPPAT1,HPPAT2};

// Bit reversal lookup
uint8_t revlookup[16] PROGMEM = {0x0,0x8,0x4,0xC,0x2,0xA,0x6,0xE,0x1,0x9,0x5,0xD,0x3,0xB,0x7,0xF};

// =======================================================================================
// STRUCTURES
// =======================================================================================

struct ConfigSettings {
  unsigned long magic;
  byte rldBrightness, rpsiBrightness, fldBrightness, fpsiBrightness;
  byte logicRandomStyle;
  unsigned int logicUpdateDelay;
  unsigned int psiStuckHowLong;
  byte psiWipeDelay;
  byte psiStuckHowOften;
  bool fpsiGetsStuck;
  bool rpsiGetsStuck;
  unsigned int leiaSpeed, alarmSpeed, marchSpeed, failureSpeed;
  byte failureLoops;
  long leiaDuration, alarmDuration, marchDuration, failureDuration;
  byte scrollSpeed;
  char TFLDtext[BOOT_TEXT_SIZE];
  char BFLDtext[BOOT_TEXT_SIZE];
  char RLDtext[BOOT_TEXT_SIZE];
  bool digitalPsiWiringSwapped;
  byte digitalPsi1_color1_index;
  byte digitalPsi1_color2_index;
  byte digitalPsi2_color1_index;
  byte digitalPsi2_color2_index;
};

struct PsiConfig {
  Adafruit_NeoPixel* frontStrip;
  Adafruit_NeoPixel* rearStrip;
  byte frontColor1;
  byte frontColor2;
  byte rearColor1;
  byte rearColor2;
};

template<typename T>
struct PsiRandomState {
  unsigned long lastColorChange;
  unsigned long lastSwipe;
  unsigned long nextColorDelay;
  byte currentPattern;
  byte currentSwipeRow;
  byte isStuck;
  T* device;
};

// =======================================================================================
// GLOBAL VARIABLES
// =======================================================================================

Preferences preferences;
ConfigSettings currentSettings;
byte activeProfileIndex = 0;

PsiOutputType globalPsiOutput = PSI_DIGITAL;

// Display State
unsigned long LEDgrid[3][5];
int scrollPositions[3] = {FLD_WIDTH, FLD_WIDTH, RLD_WIDTH};
long textScrollCount[3];
Alphabet alphabetType[3];
DisplayEffect currentEffect = EFF_NORM;
DisplayState displayState[3];
PsiState psiState[2];
byte randomStyle[3];
byte effectRunning = 0;
bool configMode = false;

char logicText[3][MAXSTRINGSIZE + 1];
char cmdString[CMD_MAX_LENGTH];

// Hardware
LedControl lcRear = LedControl(REAR_DATA_PIN, REAR_CLK_PIN, REAR_CS_PIN, 4);
LedControl lcFront = LedControl(FRONT_DATA_PIN, FRONT_CLK_PIN, FRONT_CS_PIN, FDEV);
Adafruit_NeoPixel digitalPsi1(DIGITAL_PSI_LED_COUNT, DIGITAL_PSI1_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel digitalPsi2(DIGITAL_PSI_LED_COUNT, DIGITAL_PSI2_PIN, NEO_GRB + NEO_KHZ800);

PsiRandomState<LedControl> fpsiAnalogState = {0,0,0,0,0,0,&lcFront};
PsiRandomState<LedControl> rpsiAnalogState = {0,0,0,0,0,0,&lcRear};
PsiRandomState<Adafruit_NeoPixel> fpsiDigitalState = {0,0,0,0,0,0,&digitalPsi1};
PsiRandomState<Adafruit_NeoPixel> rpsiDigitalState = {0,0,0,0,0,0,&digitalPsi2};

// NEW: v4.1 tracking
bool hasUnsavedChanges = false;
bool firstTimeSetup = false;

// NEW: v4.2 features
bool verboseMode = false;

// =======================================================================================
// FORWARD DECLARATIONS
// =======================================================================================

void loadProfileDefaults(byte profileNum, ConfigSettings &cfg);
bool validateProfileData(const ConfigSettings &cfg);
void loadProfile(byte profileNum);
void saveCurrentProfile();
void resetProfile(byte profileNum);
void initializeHardware();
void applyBrightnessSettings();
PsiConfig getPsiConfig();
uint32_t getColorFromPalette(byte index);
bool isValidColorIndex(byte index);
void setDigitalPsi(Adafruit_NeoPixel &strip, PsiState mode, byte c1, byte c2);
void setAnalogPsi(LedControl &lc, byte deviceIndex, PsiState mode);
void randomDigitalPsi(PsiRandomState<Adafruit_NeoPixel> &state, byte c1, byte c2, bool getsStuck);
void randomAnalogPsi(PsiRandomState<LedControl> &state, byte deviceIndex, bool getsStuck);
void randomFPSI();
void randomRPSI();
void setFPSI(PsiState mode);
void setRPSI(PsiState mode);
void turnOffDigitalPSIs();
void turnOffAnalogPSIs();
bool isValidDisplayIndex(byte disp);
void randomDisplay(byte disp);
void textDisplay(byte disp);
void testDisplay(byte disp);
void offDisplay(byte disp);
void bargraphDisplay(byte disp);
void scrollText(byte disp, const char* text);
void showGrid(byte disp);
void clearGrid(byte disp);
void leiaDisplay(unsigned long playTime);
void alarmDisplay(unsigned long playTime);
void marchDisplay(unsigned long playTime);
void failureDisplay(unsigned long playTime);
void exitEffects();
void resetDisplays();
void runNormalOperation();
byte buildCommand(char ch, char* output_str);
void parseCommand(char* inputStr);
void printConfigMenu();
void handleConfigCommands();
void showSettings();
void parseAndSetSetting(const char* cmd);
long randomRow(byte randomMode);
void fillColumn(byte disp, byte column, byte data);
void resetText(byte display);
void resetAllText();
uint8_t rev(uint8_t n);
void setText(byte disp, const char* message);
void getLatinLetter(int* letterBitmap, char let);
void getAurabeshLetter(int* letterBitmap, char let);
void drawLetter(byte display, char let, int shift);
void showFailure(byte style);
void doMcommand(int address, char* message);
void doTcommand(int address, int argument);
void doDcommand(int address);
void doPcommand(int address, int argument);
void doRcommand(int address, int argument);
void doScommand(int address, int argument);

// NEW: v4.1 functions
void runDiagnostics();
void showSmartSuggestion(const char* context, const char* param = nullptr);
bool validateParameterWithHints(const char* paramName, int value, int currentValue);
void runSetupWizard();
void applyPreset(byte presetNum);
void showPresets();
void printProfileHelp();
void printPsiHelp();
void printDisplayHelp();
void printBootTextHelp();
void printFullHelp();

// Helper functions for safe serial input
bool readSerialLineWithTimeout(char* buffer, size_t bufferSize, unsigned long timeoutMs);
void trimString(char* str);
int strcasecmp_safe(const char* s1, const char* s2);
bool startsWith(const char* str, const char* prefix);

// =======================================================================================
// SETUP
// =======================================================================================

void setup() {
  Serial.begin(9600);
  while (!Serial && millis() < 3000) delay(10);

  // Initialize watchdog early to prevent infinite loops
  esp_task_wdt_deinit();
  esp_task_wdt_config_t wdt_config = {
    .timeout_ms = WDT_TIMEOUT_MS,
    .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
    .trigger_panic = true
  };
  ESP_ERROR_CHECK(esp_task_wdt_init(&wdt_config));
  ESP_ERROR_CHECK(esp_task_wdt_add(NULL));

  // Try to initialize flash storage with retry logic
  bool flashInitialized = false;
  for (int retry = 0; retry < 3; retry++) {
    esp_task_wdt_reset();
    if (preferences.begin("teeces-config", false)) {
      flashInitialized = true;
      break;
    }
    Serial.print(F("Flash init attempt "));
    Serial.print(retry + 1);
    Serial.println(F(" failed, retrying..."));
    delay(500);
  }

  if (!flashInitialized) {
    Serial.println(F("ERROR: Failed to initialize flash storage after 3 attempts!"));
    Serial.println(F("System will restart in 5 seconds..."));
    for (int i = 5; i > 0; i--) {
      esp_task_wdt_reset();
      Serial.print(i);
      Serial.print(F("... "));
      delay(1000);
    }
    Serial.println(F("\nRestarting..."));
    ESP.restart();
  }

  Serial.println(F("\n╔══════════════════════════════════════════════════════════╗"));
  Serial.println(F("║     ESP32 Teeces v4.2 - Enhanced R2-D2 Controller       ║"));
  Serial.println(F("╚══════════════════════════════════════════════════════════╝"));
  Serial.println(F(""));
  Serial.println(F("NEW in v4.2:"));
  Serial.println(F("  ⚡ L-Command for direct brightness control"));
  Serial.println(F("  🎨 C-Command for PSI color changes"));
  Serial.println(F("  ⏹  T99 to stop all effects"));
  Serial.println(F("  🐛 Verbose debug mode + shortcuts (p1-p5, s, w, d, q)"));
  Serial.println(F("  ✨ Better error messages in JawaLite protocol"));
  Serial.println(F(""));

  // Check if this is first-time setup
  firstTimeSetup = !preferences.getBool("setupDone", false);

  // Load GLOBAL PSI output mode
  globalPsiOutput = (PsiOutputType)preferences.getUChar("psiOutput", PSI_DIGITAL);
  Serial.print(F("Global PSI Mode: "));
  Serial.println(globalPsiOutput == PSI_ANALOG ? F("ANALOG") : F("DIGITAL"));

  // Initialize profiles if needed
  for (int i = 0; i < NUM_PROFILES; i++) {
    char key[12];
    snprintf(key, sizeof(key), "profile%d", i + 1);
    ConfigSettings tempCfg;
    size_t read = preferences.getBytes(key, &tempCfg, sizeof(ConfigSettings));
    if (read != sizeof(ConfigSettings) || !validateProfileData(tempCfg)) {
      Serial.print(F("Initializing Profile "));
      Serial.println(i + 1);
      loadProfileDefaults(i + 1, tempCfg);
      preferences.putBytes(key, &tempCfg, sizeof(ConfigSettings));
    }
  }

  byte lastProfile = preferences.getUChar("lastProfile", 1);
  if (lastProfile < 1 || lastProfile > NUM_PROFILES) lastProfile = 1;
  loadProfile(lastProfile);
  
  Serial.println();
  Serial.println(F("Quick Commands:"));
  Serial.println(F("  *           - Enter configuration menu"));
  Serial.println(F("  diagnostics - Test all hardware (in config menu)"));
  Serial.println(F("  wizard      - Run setup wizard (in config menu)"));
  Serial.println(F(""));

  initializeHardware();
  randomSeed(esp_random());
  applyBrightnessSettings();
  randomStyle[0] = randomStyle[1] = randomStyle[2] = currentSettings.logicRandomStyle;

  // Boot sequence
  Serial.println(F("Starting boot sequence..."));
  unsigned long bootStart = millis();
  while ((textScrollCount[0] < 1) || (textScrollCount[1] < 1) || (textScrollCount[2] < 1)) {
    if (millis() - bootStart > BOOT_SEQUENCE_TIMEOUT_MS) {
      Serial.println(F("BOOT SEQUENCE TIMEOUT"));
      break;
    }
    if (textScrollCount[0] < 1) scrollText(0, currentSettings.TFLDtext);
    if (textScrollCount[1] < 1) scrollText(1, currentSettings.BFLDtext);
    if (textScrollCount[2] < 1) scrollText(2, currentSettings.RLDtext);
  }

  resetDisplays();

  // Initialize PSIs to start in random mode
  psiState[0] = PSI_RANDOM;
  psiState[1] = PSI_RANDOM;

  // Activate PSIs immediately at startup
  if (globalPsiOutput == PSI_DIGITAL) {
    PsiConfig psiCfg = getPsiConfig();

    // Show initial pattern immediately using setDigitalPsi instead of random animation
    setDigitalPsi(*psiCfg.frontStrip, PSI_COLOR1, psiCfg.frontColor1, psiCfg.frontColor2);
    setDigitalPsi(*psiCfg.rearStrip, PSI_COLOR2, psiCfg.rearColor1, psiCfg.rearColor2);

    Serial.println(F("✓ Digital PSIs activated (GPIO 8 & 9)."));
  } else {
    setFPSI(PSI_COLOR1);
    setRPSI(PSI_COLOR2);
    Serial.println(F("✓ Analog PSIs activated."));
  }

  Serial.println(F("✓ Ready for commands.")); 
  
  // NEW: First-time welcome
  if (firstTimeSetup) {
    showSmartSuggestion("first_time");
  }

  Serial.print(F("Watchdog enabled ("));
  Serial.print(WDT_TIMEOUT_MS / 1000);
  Serial.println(F("s timeout)."));
  Serial.println();
  Serial.print(F("> "));
}

// =======================================================================================
// MAIN LOOP
// =======================================================================================

void loop() {
  esp_task_wdt_reset();
  
  if (Serial.available()) { 
    char ch = Serial.peek(); 
    if (ch == '*') { 
      Serial.read(); 
      configMode = true; 
      printConfigMenu(); 
    } 
  }
  
  if (configMode) { 
    handleConfigCommands(); 
  } else { 
    runNormalOperation(); 
  }
}

void runNormalOperation() {
  if (Serial.available()) { 
    char ch = Serial.read(); 
    Serial.print(ch); 
    if (buildCommand(ch, cmdString)) { 
      parseCommand(cmdString); 
      Serial.println(); 
      Serial.print(F("> ")); 
    } 
  }

  switch (currentEffect) {
    case EFF_ALARM: alarmDisplay(currentSettings.alarmDuration); break;
    case EFF_MARCH: marchDisplay(currentSettings.marchDuration); break;
    case EFF_LEIA: leiaDisplay(currentSettings.leiaDuration); break;
    case EFF_FAILURE: failureDisplay(currentSettings.failureDuration); break;
    default:
      for (byte disp = 0; disp < 3; disp++) { 
        if (!isValidDisplayIndex(disp)) continue;
        switch (displayState[disp]) { 
          case STATE_RANDOM: randomDisplay(disp); break; 
          case STATE_TEXT: textDisplay(disp); break; 
          case STATE_TEST: testDisplay(disp); break; 
          case STATE_OFF: offDisplay(disp); break; 
          case STATE_BARGRAPH: bargraphDisplay(disp); break; 
        } 
      }

      if (globalPsiOutput == PSI_ANALOG) {
        turnOffDigitalPSIs();
        if (psiState[0] == PSI_RANDOM) randomFPSI(); else setFPSI(psiState[0]);
        if (psiState[1] == PSI_RANDOM) randomRPSI(); else setRPSI(psiState[1]);
      } else {
        turnOffAnalogPSIs();
        PsiConfig psiCfg = getPsiConfig();
        if (psiState[0] == PSI_RANDOM) {
          randomDigitalPsi(fpsiDigitalState, psiCfg.frontColor1, psiCfg.frontColor2, currentSettings.fpsiGetsStuck);
        } else {
          setDigitalPsi(*psiCfg.frontStrip, psiState[0], psiCfg.frontColor1, psiCfg.frontColor2);
        }
        if (psiState[1] == PSI_RANDOM) {
          randomDigitalPsi(rpsiDigitalState, psiCfg.rearColor1, psiCfg.rearColor2, currentSettings.rpsiGetsStuck);
        } else {
          setDigitalPsi(*psiCfg.rearStrip, psiState[1], psiCfg.rearColor1, psiCfg.rearColor2);
        }
      }
  }
}

// =======================================================================================
// PROFILE MANAGEMENT
// =======================================================================================

void loadProfileDefaults(byte profileNum, ConfigSettings &cfg) {
  cfg.magic = PROFILE_MAGIC;
  cfg.rldBrightness = 3; cfg.rpsiBrightness = 15;
  cfg.fldBrightness = 3; cfg.fpsiBrightness = 15;
  cfg.logicRandomStyle = 4;
  cfg.logicUpdateDelay = DEFAULT_LOGIC_DELAY;
  cfg.scrollSpeed = DEFAULT_SCROLL_SPEED;
  cfg.psiWipeDelay = DEFAULT_PSI_WIPE_DELAY;
  cfg.fpsiGetsStuck = true; cfg.rpsiGetsStuck = true;
  cfg.psiStuckHowLong = 7000; cfg.psiStuckHowOften = 10;
  cfg.leiaDuration = 34000; cfg.leiaSpeed = 100;
  cfg.alarmDuration = 4000; cfg.alarmSpeed = 100;
  cfg.marchDuration = 47000; cfg.marchSpeed = 555;
  cfg.failureDuration = 10000; cfg.failureLoops = 5; cfg.failureSpeed = 75;
  cfg.digitalPsiWiringSwapped = false;
  cfg.digitalPsi1_color1_index = 1;
  cfg.digitalPsi1_color2_index = 3;
  cfg.digitalPsi2_color1_index = 0;
  cfg.digitalPsi2_color2_index = 2;
  
  if (profileNum == 1) {
    strncpy(cfg.TFLDtext, "PROFILE 1", BOOT_TEXT_SIZE - 1);
    strncpy(cfg.BFLDtext, " STANDARD", BOOT_TEXT_SIZE - 1);
    strncpy(cfg.RLDtext, "BOOTING ASTROMECH   ", BOOT_TEXT_SIZE - 1);
  } else if (profileNum == 2) {
    strncpy(cfg.TFLDtext, "PROFILE 2", BOOT_TEXT_SIZE - 1);
    strncpy(cfg.BFLDtext, "  CUSTOM ", BOOT_TEXT_SIZE - 1);
    strncpy(cfg.RLDtext, "WHITE PINK MODE     ", BOOT_TEXT_SIZE - 1);
    cfg.digitalPsi1_color1_index = 11;
    cfg.digitalPsi1_color2_index = 8;
    cfg.digitalPsi2_color1_index = 11;
    cfg.digitalPsi2_color2_index = 8;
  } else {
    snprintf(cfg.TFLDtext, BOOT_TEXT_SIZE, "PROFILE %d", profileNum);
    strncpy(cfg.BFLDtext, "   USER  ", BOOT_TEXT_SIZE - 1);
    strncpy(cfg.RLDtext, "USER PROFILE        ", BOOT_TEXT_SIZE - 1);
  }
  // Ensure null-termination
  cfg.TFLDtext[BOOT_TEXT_SIZE - 1] = '\0';
  cfg.BFLDtext[BOOT_TEXT_SIZE - 1] = '\0';
  cfg.RLDtext[BOOT_TEXT_SIZE - 1] = '\0';
}

bool validateProfileData(const ConfigSettings &cfg) {
  if (cfg.magic != PROFILE_MAGIC) return false;
  if (cfg.rldBrightness > 15) return false;
  if (cfg.fldBrightness > 15) return false;
  if (cfg.logicRandomStyle > 6) return false;
  if (!isValidColorIndex(cfg.digitalPsi1_color1_index)) return false;
  if (!isValidColorIndex(cfg.digitalPsi1_color2_index)) return false;
  if (!isValidColorIndex(cfg.digitalPsi2_color1_index)) return false;
  if (!isValidColorIndex(cfg.digitalPsi2_color2_index)) return false;
  return true;
}

void loadProfile(byte profileNum) {
  if (profileNum < 1 || profileNum > NUM_PROFILES) {
    Serial.println(F("ERROR: Invalid profile number"));
    return;
  }
  char key[12];
  snprintf(key, sizeof(key), "profile%d", profileNum);
  size_t read = preferences.getBytes(key, &currentSettings, sizeof(ConfigSettings));
  if (read != sizeof(ConfigSettings) || !validateProfileData(currentSettings)) {
    Serial.print(F("Profile ")); Serial.print(profileNum);
    Serial.println(F(" corrupted, loading defaults"));
    loadProfileDefaults(profileNum, currentSettings);
  }
  activeProfileIndex = profileNum - 1;
  applyBrightnessSettings();
  preferences.putUChar("lastProfile", profileNum);
  Serial.print(F("Profile ")); Serial.print(profileNum);
  Serial.println(F(" loaded and active."));
}

void saveCurrentProfile() {
  if (activeProfileIndex < 2) {
    Serial.println(F("ERROR: Profiles 1-2 are read-only"));
    return;
  }
  char key[12];
  snprintf(key, sizeof(key), "profile%d", activeProfileIndex + 1);
  size_t written = preferences.putBytes(key, &currentSettings, sizeof(ConfigSettings));
  if (written != sizeof(ConfigSettings)) {
    Serial.println(F("ERROR: Failed to save profile!"));
  } else {
    Serial.print(F("Profile ")); Serial.print(activeProfileIndex + 1);
    Serial.println(F(" saved successfully."));
  }
}

void resetProfile(byte profileNum) {
  if (profileNum < 3 || profileNum > NUM_PROFILES) {
    Serial.println(F("ERROR: Only profiles 3-5 can be reset"));
    return;
  }
  ConfigSettings tempCfg;
  loadProfileDefaults(profileNum, tempCfg);
  char key[12];
  snprintf(key, sizeof(key), "profile%d", profileNum);
  preferences.putBytes(key, &tempCfg, sizeof(ConfigSettings));
  Serial.print(F("Profile ")); Serial.print(profileNum);
  Serial.println(F(" reset to defaults."));
  if (activeProfileIndex == profileNum - 1) {
    loadProfile(profileNum);
  }
}

void initializeHardware() {
  for (int dev = 0; dev < lcRear.getDeviceCount(); dev++) { 
    lcRear.shutdown(dev, false); 
    lcRear.clearDisplay(dev); 
  }
  for (int dev = 0; dev < lcFront.getDeviceCount(); dev++) { 
    lcFront.shutdown(dev, false); 
    lcFront.clearDisplay(dev); 
  }
  digitalPsi1.begin(); digitalPsi1.clear(); digitalPsi1.show();
  digitalPsi2.begin(); digitalPsi2.clear(); digitalPsi2.show();
}

void applyBrightnessSettings() {
  lcRear.setIntensity(0, currentSettings.rldBrightness);
  lcRear.setIntensity(1, currentSettings.rldBrightness);
  lcRear.setIntensity(2, currentSettings.rldBrightness);
  lcRear.setIntensity(3, currentSettings.rpsiBrightness);
  lcFront.setIntensity(0, currentSettings.fldBrightness);
  lcFront.setIntensity(1, currentSettings.fldBrightness);
  lcFront.setIntensity(FPSIDEV, currentSettings.fpsiBrightness);
}

// =======================================================================================
// PSI FUNCTIONS 
// =======================================================================================

PsiConfig getPsiConfig() {
  PsiConfig cfg;
  bool isSwapped = currentSettings.digitalPsiWiringSwapped;
  cfg.frontStrip = isSwapped ? &digitalPsi2 : &digitalPsi1;
  cfg.rearStrip = isSwapped ? &digitalPsi1 : &digitalPsi2;
  cfg.frontColor1 = isSwapped ? currentSettings.digitalPsi2_color1_index : currentSettings.digitalPsi1_color1_index;
  cfg.frontColor2 = isSwapped ? currentSettings.digitalPsi2_color2_index : currentSettings.digitalPsi1_color2_index;
  cfg.rearColor1 = isSwapped ? currentSettings.digitalPsi1_color1_index : currentSettings.digitalPsi2_color1_index;
  cfg.rearColor2 = isSwapped ? currentSettings.digitalPsi1_color2_index : currentSettings.digitalPsi2_color2_index;
  return cfg;
}

uint32_t getColorFromPalette(byte index) {
  if (index >= numColors) index = 0;
  return pgm_read_dword(&(colorPalette[index].value));
}

bool isValidColorIndex(byte index) {
  return (index < numColors);
}

void setDigitalPsi(Adafruit_NeoPixel &strip, PsiState mode, byte color1_idx, byte color2_idx) {
  strip.clear();
  uint32_t c1 = getColorFromPalette(color1_idx);
  uint32_t c2 = getColorFromPalette(color2_idx);
  
  switch (mode) {
    case PSI_TEST:
      strip.fill(getColorFromPalette(11), 0, DIGITAL_PSI_LED_COUNT);
      break;
    case PSI_COLOR1:
      for (uint8_t i = 0; i < 13; i++) {
        uint8_t idx = pgm_read_byte(&psi_pattern1_indices[i]);
        strip.setPixelColor(idx, c1);
      }
      break;
    case PSI_COLOR2:
      for (uint8_t i = 0; i < 13; i++) {
        uint8_t idx = pgm_read_byte(&psi_pattern2_indices[i]);
        strip.setPixelColor(idx, c2);
      }
      break;
    case PSI_OFF:
    default:
      break;
  }
  strip.show();
}

void setAnalogPsi(LedControl &lc, byte deviceIndex, PsiState mode) {
  for (byte row = 0; row < HPROW; row++) {
    switch (mode) {
      case PSI_OFF: lc.setRow(deviceIndex, row, 0x00); break;
      case PSI_TEST: lc.setRow(deviceIndex, row, 0xFF); break;
      case PSI_COLOR1: lc.setRow(deviceIndex, row, pgm_read_byte(&psiPatterns[row])); break;
      case PSI_COLOR2: lc.setRow(deviceIndex, row, pgm_read_byte(&psiPatterns[row + 5])); break;
      default: break;
    }
  }
}

void randomDigitalPsi(PsiRandomState<Adafruit_NeoPixel> &state, byte color1_idx, byte color2_idx, bool getsStuck) {
  unsigned long currentMillis = millis();
  uint32_t c1 = getColorFromPalette(color1_idx);
  uint32_t c2 = getColorFromPalette(color2_idx);
  
  if (state.isStuck == 1) {
    state.lastColorChange = currentMillis;
  } else {
    if (currentMillis - state.lastColorChange > state.nextColorDelay * 500) {
      state.lastColorChange = currentMillis;
      state.nextColorDelay = random(1, 11);
      if (state.currentPattern == 0) {
        state.currentPattern = 1;
        state.currentSwipeRow = 0;
      } else {
        state.currentPattern = 0;
        state.currentSwipeRow = HPROW - 1;
      }
    }
  }
  
  if (state.isStuck == 1) {
    if (currentMillis - state.lastSwipe < currentSettings.psiStuckHowLong) return;
  } else {
    if (currentMillis - state.lastSwipe < currentSettings.psiWipeDelay) return;
  }
  
  if (state.isStuck) state.isStuck = 0;
  state.lastSwipe = currentMillis;
  uint8_t row = state.currentSwipeRow;
  
  if (state.currentPattern == 1 && row < HPROW) {
    uint8_t rowLen = pgm_read_byte(&psi_row_lengths[row]);
    const uint8_t* rowIndices = (const uint8_t*)pgm_read_ptr(&psi_row_indices[row]);
    for (uint8_t i = 0; i < rowLen; i++) {
      uint8_t idx = pgm_read_byte(&rowIndices[i]);
      bool is_p2 = false;
      for (uint8_t p = 0; p < 13; p++) {
        if (pgm_read_byte(&psi_pattern2_indices[p]) == idx) {
          is_p2 = true;
          break;
        }
      }
      state.device->setPixelColor(idx, is_p2 ? c2 : 0);
    }
    state.currentSwipeRow++;
    state.device->show();
  } else if (state.currentPattern == 0 && row < HPROW) {
    uint8_t rowLen = pgm_read_byte(&psi_row_lengths[row]);
    const uint8_t* rowIndices = (const uint8_t*)pgm_read_ptr(&psi_row_indices[row]);
    for (uint8_t i = 0; i < rowLen; i++) {
      uint8_t idx = pgm_read_byte(&rowIndices[i]);
      bool is_p1 = false;
      for (uint8_t p = 0; p < 13; p++) {
        if (pgm_read_byte(&psi_pattern1_indices[p]) == idx) {
          is_p1 = true;
          break;
        }
      }
      state.device->setPixelColor(idx, is_p1 ? c1 : 0);
    }
    state.currentSwipeRow--;
    state.device->show();
  }
  
  if (getsStuck && state.currentSwipeRow == 2 && state.currentPattern == 1) {
    if (random(currentSettings.psiStuckHowOften) == 1) {
      state.isStuck = 1;
    }
  }
}

void randomAnalogPsi(PsiRandomState<LedControl> &state, byte deviceIndex, bool getsStuck) {
  unsigned long currentMillis = millis();
  
  if (state.isStuck == 1) {
    state.lastColorChange = currentMillis;
  } else {
    if (currentMillis - state.lastColorChange > state.nextColorDelay * 500) {
      state.lastColorChange = currentMillis;
      state.nextColorDelay = random(1, 11);
      if (state.currentPattern == 0) {
        state.currentPattern = 5;
        state.currentSwipeRow = 0;
      } else {
        state.currentPattern = 0;
        state.currentSwipeRow = HPROW - 1;
      }
    }
  }
  
  if (state.isStuck == 1) {
    if (currentMillis - state.lastSwipe < currentSettings.psiStuckHowLong) return;
  } else {
    if (currentMillis - state.lastSwipe < currentSettings.psiWipeDelay) return;
  }
  
  if (state.isStuck) state.isStuck = 0;
  
  if (state.currentSwipeRow < HPROW && state.currentPattern == 5) {
    state.lastSwipe = currentMillis;
    state.device->setRow(deviceIndex, state.currentSwipeRow, 
                         pgm_read_byte(&psiPatterns[state.currentSwipeRow + state.currentPattern]));
    state.currentSwipeRow++;
  } else if (state.currentPattern == 0) {
    state.lastSwipe = currentMillis;
    state.device->setRow(deviceIndex, state.currentSwipeRow, 
                         pgm_read_byte(&psiPatterns[state.currentSwipeRow + state.currentPattern]));
    state.currentSwipeRow--;
  }
  
  if (getsStuck && state.currentSwipeRow == 2 && state.currentPattern == 5) {
    if (random(currentSettings.psiStuckHowOften) == 1) {
      state.isStuck = 1;
    }
  }
}

void randomFPSI() {
  randomAnalogPsi(fpsiAnalogState, FPSIDEV, currentSettings.fpsiGetsStuck);
}

void randomRPSI() {
  randomAnalogPsi(rpsiAnalogState, RPSIDEV, currentSettings.rpsiGetsStuck);
}

void setFPSI(PsiState mode) {
  setAnalogPsi(lcFront, FPSIDEV, mode);
}

void setRPSI(PsiState mode) {
  setAnalogPsi(lcRear, RPSIDEV, mode);
}

void turnOffDigitalPSIs() {
  digitalPsi1.clear(); digitalPsi1.show();
  digitalPsi2.clear(); digitalPsi2.show();
}

void turnOffAnalogPSIs() {
  setFPSI(PSI_OFF);
  setRPSI(PSI_OFF);
}

// =======================================================================================
// DISPLAY FUNCTIONS
// =======================================================================================

bool isValidDisplayIndex(byte disp) {
  return (disp < 3);
}

void randomDisplay(byte disp) {
  static unsigned long previousUpdate[3] = {0, 0, 0};
  unsigned long currentMillis = millis();
  if (!isValidDisplayIndex(disp)) return;
  if (currentMillis - previousUpdate[disp] < currentSettings.logicUpdateDelay) return;
  previousUpdate[disp] = currentMillis;
  
  if (disp == 2) {
    for (int dev = 0; dev < 3; dev++) {
      for (int row = 0; row < 6; row++) {
        lcRear.setRow(dev, row, randomRow(randomStyle[2]));
      }
    }
  } else {
    byte deviceIndex = disp;
    for (int row = 0; row < 6; row++) {
      lcFront.setRow(deviceIndex, row, randomRow(randomStyle[disp]));
    }
  }
}

long randomRow(byte randomMode) {
  switch (randomMode) {
    case 0: return (random(256) & random(256) & random(256) & random(256));
    case 1: return (random(256) & random(256) & random(256));
    case 2: return (random(256) & random(256));
    case 3: return random(256);
    case 4: return (random(256) | random(256));
    case 5: return (random(256) | random(256) | random(256));
    case 6: return (random(256) | random(256) | random(256) | random(256));
    default: return random(256);
  }
}

void textDisplay(byte disp) {
  if (!isValidDisplayIndex(disp)) return;
  scrollText(disp, logicText[disp]);
}

void testDisplay(byte disp) {
  if (!isValidDisplayIndex(disp)) return;
  for (byte i = 0; i < 5; i++) {
    LEDgrid[disp][i] = GRID_ALL_ON;
  }
  showGrid(disp);
}

void offDisplay(byte disp) {
  if (!isValidDisplayIndex(disp)) return;
  for (byte i = 0; i < 5; i++) {
    LEDgrid[disp][i] = GRID_ALL_OFF;
  }
  showGrid(disp);
}

void bargraphDisplay(byte disp) {
  static byte bargraphdata[3][27];
  static unsigned long previousUpdate[3] = {0, 0, 0};
  if (!isValidDisplayIndex(disp)) return;
  unsigned long currentMillis = millis();
  if (currentMillis - previousUpdate[disp] < 50) return;
  previousUpdate[disp] = currentMillis;
  byte maxcol = (disp == 2) ? RLD_WIDTH : FLD_WIDTH;
  for (byte column = 0; column < maxcol; column++) {
    int variation = random(0, 3);
    int value = (int)bargraphdata[disp][column];
    if (value == 5) value = 3;
    else value += (variation - 1);
    if (value < 0) value = 0;
    if (value > 5) value = 5;
    bargraphdata[disp][column] = (byte)value;
    byte data = 0;
    for (int i = 0; i <= value; i++) {
      data |= 0x01 << i;
    }
    fillColumn(disp, column, data);
  }
  showGrid(disp);
}

void fillColumn(byte disp, byte column, byte data) {
  if (!isValidDisplayIndex(disp)) return;
  if (disp == 2 && column > 26) return;
  if (disp != 2 && column > 8) return;
  for (byte row = 0; row < 5; row++) {
    if (data & (1 << row))
      LEDgrid[disp][4 - row] |= (1L << column);
    else
      LEDgrid[disp][4 - row] &= ~(1L << column);
  }
}

void clearGrid(byte display) {
  if (!isValidDisplayIndex(display)) return;
  for (byte row = 0; row < 5; row++) {
    LEDgrid[display][row] = GRID_ALL_OFF;
  }
}

// Display the LED grid on the physical hardware
// The grid is stored as a long value per row, with each bit representing an LED
// Complex bit operations handle the mapping from logical grid to physical display
void showGrid(byte display) {
  if (!isValidDisplayIndex(display)) return;

  // Overflow columns (9th, 18th, 27th) need special handling
  unsigned char col8 = 0, col17 = 0, col26 = 0;

  switch (display) {
    case 0:  // Top Front Logic Display (TFLD)
      for (byte row = 0; row < 5; row++) {
        // Extract first 8 bits, reverse them (hardware wiring), and send to device
        lcFront.setRow(0, row, rev(LEDgrid[display][row] & 255L));
        // Check if 9th column (bit 8) is set and accumulate into overflow column
        if ((LEDgrid[display][row] & (1L << 8)) == (1L << 8))
          col8 += 128 >> row;  // Build vertical column byte from row bits
      }
      lcFront.setRow(0, 5, col8);  // Send overflow column to row 5
      break;

    case 1:  // Bottom Front Logic Display (BFLD)
      for (byte row = 0; row < 5; row++) {
        // Extract bits 1-8 (shift right by 1) and send to device in reverse row order
        lcFront.setRow(1, 4 - row, (LEDgrid[display][row] & (255L << 1)) >> 1);
        // Check if 1st column (bit 0) is set for overflow
        if ((LEDgrid[display][row] & 1L) == 1L)
          col8 += 8 << row;  // Build vertical column starting at bit 3
      }
      lcFront.setRow(1, 5, col8);  // Send overflow column
      break;

    case 2:  // Rear Logic Display (RLD) - 3 chained devices, 27 columns total
      for (byte row = 0; row < 5; row++) {
        // Split 27-bit grid across 3 MAX7219 devices (9 columns each)
        for (byte dev = 0; dev < 3; dev++) {
          // Extract 8 bits for this device (offset by dev*9), reverse, and send
          lcRear.setRow(dev, row, rev((LEDgrid[display][row] & (255L << (9 * dev))) >> (9 * dev)));
        }
        // Handle overflow columns (9th, 18th, 27th) for each device
        if ((LEDgrid[display][row] & (1L << 8)) == (1L << 8)) col8 += 128 >> row;
        if ((LEDgrid[display][row] & (1L << 17)) == (1L << 17)) col17 += 128 >> row;
        if ((LEDgrid[display][row] & (1L << 26)) == (1L << 26)) col26 += 128 >> row;
      }
      // Send overflow columns to each device
      lcRear.setRow(0, 5, col8);
      lcRear.setRow(1, 5, col17);
      lcRear.setRow(2, 5, col26);
      break;
  }
}

// Reverse bit order of a byte using lookup table
// Required because LED matrix hardware is wired with reversed bit order
uint8_t rev(uint8_t n) {
  // Swap nibbles and reverse bits within each using lookup table
  return (pgm_read_byte(&revlookup[n & 0x0F]) << 4) | pgm_read_byte(&revlookup[n >> 4]);
}

void setText(byte disp, const char* message) {
  if (!isValidDisplayIndex(disp)) return;
  strncpy(logicText[disp], message, MAXSTRINGSIZE);
  logicText[disp][MAXSTRINGSIZE] = '\0';  // Ensure null-termination
}

// =======================================================================================
// TEXT AND FONT FUNCTIONS 
// =======================================================================================

void getLatinLetter(int* letterBitmap, char let) {
  const int* pLetter;
  switch (let) {
    case 'A': pLetter = cA; break; case 'B': pLetter = cB; break; case 'C': pLetter = cC; break;
    case 'D': pLetter = cD; break; case 'E': pLetter = cE; break; case 'F': pLetter = cF; break;
    case 'G': pLetter = cG; break; case 'H': pLetter = cH; break; case 'I': pLetter = cI; break;
    case 'J': pLetter = cJ; break; case 'K': pLetter = cK; break; case 'L': pLetter = cL; break;
    case 'M': pLetter = cM; break; case 'N': pLetter = cN; break; case 'O': pLetter = cO; break;
    case 'P': pLetter = cP; break; case 'Q': pLetter = cQ; break; case 'R': pLetter = cR; break;
    case 'S': pLetter = cS; break; case 'T': pLetter = cT; break; case 'U': pLetter = cU; break;
    case 'V': pLetter = cV; break; case 'W': pLetter = cW; break; case 'X': pLetter = cX; break;
    case 'Y': pLetter = cY; break; case 'Z': pLetter = cZ; break;
    case '0': pLetter = c0; break; case '1': pLetter = c1; break; case '2': pLetter = c2; break;
    case '3': pLetter = c3; break; case '4': pLetter = c4; break; case '5': pLetter = c5; break;
    case '6': pLetter = c6; break; case '7': pLetter = c7; break; case '8': pLetter = c8; break;
    case '9': pLetter = c9; break;
    case '*': pLetter = ch; break; case '#': pLetter = ct; break; case '@': pLetter = cr; break;
    case '-': pLetter = cd; break; case '|': pLetter = cf; break; case '.': pLetter = cdot; break;
    case ' ': pLetter = cb; break; case '<': pLetter = cu; break; case '>': pLetter = cn; break;
    default: pLetter = cb; break;
  }
  for (byte i = 0; i < 5; i++) {
    letterBitmap[i] = (int)pgm_read_word(&(pLetter[i]));
  }
}

void getAurabeshLetter(int* letterBitmap, char let) {
  const int* pLetter;
  switch (let) {
    case '2': pLetter = a2; break; case 'A': pLetter = aA; break; case 'B': pLetter = aB; break;
    case 'C': pLetter = aC; break; case 'D': pLetter = aD; break; case 'E': pLetter = aE; break;
    case 'F': pLetter = aF; break; case 'G': pLetter = aG; break; case 'H': pLetter = aH; break;
    case 'I': pLetter = aI; break; case 'J': pLetter = aJ; break; case 'K': pLetter = aK; break;
    case 'L': pLetter = aL; break; case 'M': pLetter = aM; break; case 'N': pLetter = aN; break;
    case 'O': pLetter = aO; break; case 'P': pLetter = aP; break; case 'Q': pLetter = aQ; break;
    case 'R': pLetter = aR; break; case 'S': pLetter = aS; break; case 'T': pLetter = aT; break;
    case 'U': pLetter = aU; break; case 'V': pLetter = aV; break; case 'W': pLetter = aW; break;
    case 'X': pLetter = aX; break; case 'Y': pLetter = aY; break; case 'Z': pLetter = aZ; break;
    case ' ': pLetter = aZZ; break;
    default: pLetter = aZZ; break;
  }
  for (byte i = 0; i < 5; i++) {
    letterBitmap[i] = (int)pgm_read_word(&(pLetter[i]));
  }
}

void drawLetter(byte display, char let, int shift) {
  if (!isValidDisplayIndex(display)) return;
  if (shift < -LETTERWIDTH || shift > RLD_WIDTH) return;
  int letterBitmap[5];
  switch (alphabetType[display]) {
    case ALPHA_LATIN: getLatinLetter(letterBitmap, let); break;
    case ALPHA_AURABESH: getAurabeshLetter(letterBitmap, let); break;
    default: getLatinLetter(letterBitmap, let); break;
  }
  for (byte i = 0; i < 5; i++) {
    if (shift >= 0)
      LEDgrid[display][i] |= ((long)letterBitmap[i]) << shift;
    else
      LEDgrid[display][i] |= ((long)letterBitmap[i]) >> -shift;
  }
}

void scrollText(byte disp, const char* text) {
  static unsigned long previousTextScroll[3];
  unsigned long currentMillis = millis();
  if (!isValidDisplayIndex(disp)) return;
  if ((currentMillis - previousTextScroll[disp]) < currentSettings.scrollSpeed) return;
  previousTextScroll[disp] = currentMillis;
  clearGrid(disp);
  unsigned int text_len = strlen(text);
  for (unsigned int i = 0; i < text_len; i++) {
    int shift = i * LETTERWIDTH + scrollPositions[disp];
    drawLetter(disp, text[i], shift);
  }
  scrollPositions[disp]--;
  if (scrollPositions[disp] < -LETTERWIDTH * (int)text_len) {
    scrollPositions[disp] = (disp == 2) ? RLD_WIDTH : FLD_WIDTH;
    textScrollCount[disp]++;
  }
  showGrid(disp);
}

void resetText(byte display) {
  if (!isValidDisplayIndex(display)) return;
  scrollPositions[display] = (display == 2 ? RLD_WIDTH : FLD_WIDTH);
  textScrollCount[display] = 0;
}

void resetAllText() {
  for (byte disp = 0; disp < 3; disp++) {
    resetText(disp);
  }
}

void exitEffects() {
  currentEffect = EFF_NORM;
  effectRunning = 0;
}

void resetDisplays() {
  resetAllText();
  exitEffects();
  for (byte disp = 0; disp < 3; disp++) {
    alphabetType[disp] = ALPHA_LATIN;
    displayState[disp] = STATE_RANDOM;
  }
}

// =======================================================================================
// SPECIAL EFFECTS
// =======================================================================================

void leiaDisplay(unsigned long playTime) {
  static byte a = 0, b = 0;
  static unsigned long swtchMillis, enterMillis;
  unsigned long currentMillis = millis();
  
  if (effectRunning == 0) {
    enterMillis = currentMillis;
    effectRunning = 1;
    clearGrid(0); showGrid(0);
    clearGrid(1); showGrid(1);
    clearGrid(2); showGrid(2);
  }
  
  if (playTime && (currentMillis - enterMillis > playTime)) {
    effectRunning = 0;
    currentEffect = EFF_NORM;
    return;
  }
  
  if (globalPsiOutput == PSI_ANALOG) {
    randomFPSI();
    randomRPSI();
  } else {
    PsiConfig psiCfg = getPsiConfig();
    randomDigitalPsi(fpsiDigitalState, psiCfg.frontColor1, psiCfg.frontColor2, currentSettings.fpsiGetsStuck);
    randomDigitalPsi(rpsiDigitalState, psiCfg.rearColor1, psiCfg.rearColor2, currentSettings.rpsiGetsStuck);
  }
  
  if (currentMillis - swtchMillis > currentSettings.leiaSpeed) {
    swtchMillis = currentMillis;
    for (int dev = 0; dev < 3; dev++) {
      lcRear.setRow(dev, a, 255);
      lcRear.setLed(dev, 5, a, true);
      lcRear.setRow(dev, b, 0);
      lcRear.setLed(dev, 5, b, false);
    }
    lcFront.setRow(0, a, 255);
    lcFront.setLed(0, 5, a, true);
    lcFront.setRow(0, b, 0);
    lcFront.setLed(0, 5, b, false);
    lcFront.setRow(1, 4 - a, 255);
    lcFront.setLed(1, 5, 4 - a, true);
    lcFront.setRow(1, 4 - b, 0);
    lcFront.setLed(1, 5, 4 - b, false);
    b = a;
    a++;
    if (a > 4) a = 0;
  }
}

void alarmDisplay(unsigned long playTime) {
  static byte swtch = 0;
  static unsigned long swtchMillis, enterMillis;
  unsigned long currentMillis = millis();
  
  if (effectRunning == 0) {
    enterMillis = currentMillis;
    effectRunning = 1;
  }
  
  if (playTime && (currentMillis - enterMillis > playTime)) {
    effectRunning = 0;
    currentEffect = EFF_NORM;
    return;
  }
  
  if (currentMillis - swtchMillis > currentSettings.alarmSpeed) {
    swtchMillis = currentMillis;
    
    if (swtch == 0) {
      for (int row = 0; row < 5; row++) {
        LEDgrid[0][row] = GRID_ALL_ON;
        LEDgrid[1][row] = GRID_ALL_ON;
        LEDgrid[2][row] = GRID_ALL_ON;
      }
      showGrid(0); showGrid(1); showGrid(2);
      
      if (globalPsiOutput == PSI_ANALOG) {
        setFPSI(RED);
        setRPSI(YELLOW);
      } else {
        PsiConfig psiCfg = getPsiConfig();
        setDigitalPsi(*psiCfg.frontStrip, PSI_COLOR2, psiCfg.frontColor1, psiCfg.frontColor2);
        setDigitalPsi(*psiCfg.rearStrip, PSI_COLOR1, psiCfg.rearColor1, psiCfg.rearColor2);
      }
      swtch = 1;
      
    } else {
      clearGrid(0); clearGrid(1); clearGrid(2);
      showGrid(0); showGrid(1); showGrid(2);
      
      if (globalPsiOutput == PSI_ANALOG) {
        setFPSI(PSI_OFF);
        setRPSI(PSI_OFF);
      } else {
        turnOffDigitalPSIs();
      }
      swtch = 0;
    }
  }
}

void marchDisplay(unsigned long playTime) {
  static byte swtch = 0;
  static unsigned long swtchMillis, enterMillis;
  unsigned long currentMillis = millis();
  
  if (effectRunning == 0) {
    enterMillis = currentMillis;
    effectRunning = 1;
  }
  
  if (playTime && (currentMillis - enterMillis > playTime)) {
    effectRunning = 0;
    currentEffect = EFF_NORM;
    return;
  }
  
  if (currentMillis - swtchMillis > currentSettings.marchSpeed) {
    swtchMillis = currentMillis;
    
    if (swtch == 0) {
      for (int row = 0; row < 5; row++) {
        LEDgrid[0][row] = GRID_LEFT_HALF_FLD;
        LEDgrid[1][row] = GRID_LEFT_HALF_FLD;
        LEDgrid[2][row] = GRID_LEFT_HALF_RLD;
      }
      showGrid(0); showGrid(1); showGrid(2);
      
      if (globalPsiOutput == PSI_ANALOG) {
        setFPSI(RED);
        setRPSI(YELLOW);
      } else {
        PsiConfig psiCfg = getPsiConfig();
        setDigitalPsi(*psiCfg.frontStrip, PSI_COLOR2, psiCfg.frontColor1, psiCfg.frontColor2);
        setDigitalPsi(*psiCfg.rearStrip, PSI_COLOR1, psiCfg.rearColor1, psiCfg.rearColor2);
      }
      swtch = 1;
      
    } else {
      for (int row = 0; row < 5; row++) {
        LEDgrid[0][row] = GRID_RIGHT_HALF_FLD;
        LEDgrid[1][row] = GRID_RIGHT_HALF_FLD;
        LEDgrid[2][row] = GRID_RIGHT_HALF_RLD;
      }
      showGrid(0); showGrid(1); showGrid(2);
      
      if (globalPsiOutput == PSI_ANALOG) {
        setFPSI(BLUE);
        setRPSI(GREEN);
      } else {
        PsiConfig psiCfg = getPsiConfig();
        setDigitalPsi(*psiCfg.frontStrip, PSI_COLOR1, psiCfg.frontColor1, psiCfg.frontColor2);
        setDigitalPsi(*psiCfg.rearStrip, PSI_COLOR2, psiCfg.rearColor1, psiCfg.rearColor2);
      }
      swtch = 0;
    }
  }
}

void showFailure(byte style) {
  for (int row = 0; row < 6; row++)
    lcFront.setRow(0, row, randomRow(style));
  for (int row = 0; row < 6; row++)
    lcFront.setRow(1, row, randomRow(style));
  for (int dev = 0; dev < 3; dev++) {
    for (int row = 0; row < 6; row++)
      lcRear.setRow(dev, row, randomRow(style));
  }
  
  if (globalPsiOutput == PSI_ANALOG) {
    for (int row = 0; row < HPROW; row++)
      lcFront.setRow(FPSIDEV, row, randomRow(style));
    for (int row = 0; row < HPROW; row++)
      lcRear.setRow(RPSIDEV, row, randomRow(style));
  } else {
    digitalPsi1.fill(digitalPsi1.Color(random(255), random(255), random(255)), 0, DIGITAL_PSI_LED_COUNT);
    digitalPsi1.show();
    digitalPsi2.fill(digitalPsi2.Color(random(255), random(255), random(255)), 0, DIGITAL_PSI_LED_COUNT);
    digitalPsi2.show();
  }
}

void failureDisplay(unsigned long playTime) {
  static unsigned int loopCount = 0;
  static unsigned long lastMillis, enterMillis;
  static unsigned long blinkSpeed;
  unsigned long currentMillis = millis();
  
  if (effectRunning == 0) {
    blinkSpeed = currentSettings.failureSpeed;
    loopCount = 0;
    enterMillis = currentMillis;
    effectRunning = 1;
  }
  
  if (playTime && (currentMillis - enterMillis > playTime)) {
    effectRunning = 0;
    currentEffect = EFF_NORM;
    return;
  }
  
  if (currentMillis - lastMillis < blinkSpeed) return;
  lastMillis = currentMillis;
  loopCount++;
  
  if (loopCount < currentSettings.failureLoops) {
    blinkSpeed = currentSettings.failureSpeed;
    showFailure(4);
  } else if (loopCount < 2 * currentSettings.failureLoops) {
    blinkSpeed = 2 * currentSettings.failureSpeed;
    showFailure(3);
  } else if (loopCount < 3 * currentSettings.failureLoops) {
    blinkSpeed = 3 * currentSettings.failureSpeed;
    showFailure(2);
  } else if (loopCount < 4 * currentSettings.failureLoops) {
    blinkSpeed = 4 * currentSettings.failureSpeed;
    showFailure(1);
  } else if (loopCount < 5 * currentSettings.failureLoops) {
    showFailure(0);
  }
}

// =======================================================================================
// COMMAND PROCESSING (JawaLite Protocol)
// =======================================================================================

byte buildCommand(char ch, char* output_str) {
  static uint8_t pos = 0;
  
  if (ch == '\r') {
    output_str[pos] = '\0';
    pos = 0;
    return true;
  } else {
    if (pos < CMD_MAX_LENGTH - 1) {
      output_str[pos] = ch;
      pos++;
    } else {
      // Send BEL (bell) character to indicate buffer overflow
      Serial.write(0x7);  // ASCII BEL - audible alert for terminal
    }
  }
  return false;
}

void parseCommand(char* inputStr) {
  byte hasArgument = false;
  int argument = 0, address = 0;
  byte pos = 0;
  byte length = strlen(inputStr);
  unsigned long startTime = millis();

  // NEW v4.2: Verbose mode debugging
  if (verboseMode) {
    Serial.print(F("\n[DEBUG] Received: '"));
    Serial.print(inputStr);
    Serial.print(F("' ("));
    Serial.print(length);
    Serial.println(F(" bytes)"));
  }

  // NEW v4.2: Quick help command
  if (strcmp(inputStr, "?" "?") == 0 || strcmp(inputStr, "help") == 0) {
    Serial.println(F("\n╔══════════════════════════════════════════════════════════╗"));
    Serial.println(F("║              JawaLite Protocol Quick Reference           ║"));
    Serial.println(F("╚══════════════════════════════════════════════════════════╝"));
    Serial.println(F("\nFormat: [address][command][argument]"));
    Serial.println(F("  Address: 0=All, 1=TopFLD, 2=BottomFLD, 3=RLD, 4=FPSI, 5=RPSI\n"));
    Serial.println(F("Commands:"));
    Serial.println(F("  T[arg]  - Display state (0=test, 1=random, 20=off, 99=stop effects, 100=text)"));
    Serial.println(F("  M[text] - Display text message"));
    Serial.println(F("  P[arg]  - Alphabet (60=Latin, 61=Aurabesh)"));
    Serial.println(F("  R[arg]  - Random style (0-6)"));
    Serial.println(F("  S[arg]  - PSI state (0=test, 1=random, 2=color1, 3=color2, 4=off)"));
    Serial.println(F("  L[arg]  - Brightness (0-15) [NEW v4.2]"));
    Serial.println(F("  C[p][c] - PSI color: pattern(1-2) + color(0-11) [NEW v4.2]"));
    Serial.println(F("  D       - Dimension command\n"));
    Serial.println(F("Examples:"));
    Serial.println(F("  0T1      → All displays to random mode"));
    Serial.println(F("  1MHELLO  → Show 'HELLO' on top FLD"));
    Serial.println(F("  0L10     → Set all brightness to 10"));
    Serial.println(F("  4C111    → Front PSI pattern 1 = color 11 (white)"));
    Serial.println(F("  0T99     → Stop all effects\n"));
    Serial.println(F("Type '*' to enter configuration menu"));
    return;
  }

  // Validation: minimum length
  if (length < 2) {
    Serial.println(F("\nERROR: Command too short (min 2 chars). Format: [addr][cmd][arg]"));
    Serial.write(0x7);  // BEL for backwards compatibility
    return;
  }

  // Parse address (1 or 2 digits)
  char addrStr[3];
  if (!isdigit(inputStr[pos])) {
    Serial.println(F("\nERROR: Command must start with address (0-5)"));
    Serial.write(0x7);
    return;
  }
  addrStr[0] = inputStr[pos];
  pos++;

  if (pos < length && isdigit(inputStr[pos])) {
    addrStr[1] = inputStr[pos];
    addrStr[2] = '\0';
    pos++;
  } else {
    addrStr[1] = '\0';
  }
  address = atoi(addrStr);

  if (verboseMode) {
    Serial.print(F("[DEBUG] Parsed address: "));
    Serial.println(address);
  }

  // Validation: must have command character
  if (pos >= length) {
    Serial.println(F("\nERROR: Missing command character (T/M/P/R/S/L/C/D)"));
    Serial.write(0x7);
    return;
  }

  // Handle 'M' command (text message) separately - it doesn't require numeric argument
  if (inputStr[pos] == 'M') {
    pos++;
    if (pos >= length) {
      Serial.println(F("\nERROR: M command requires text. Format: [addr]M[text]"));
      Serial.write(0x7);
      return;
    }
    doMcommand(address, inputStr + pos);
    return;
  }

  // Handle 'C' command (color) separately - format: [addr]C[pattern][colorIndex]
  // Example: 4C111 = Address 4, Pattern 1, Color 11
  if (inputStr[pos] == 'C') {
    pos++;
    if (pos >= length || !isdigit(inputStr[pos])) {
      Serial.println(F("\nERROR: C command format: [addr]C[pattern][color]. Example: 4C111"));
      Serial.write(0x7);
      return;
    }
    // First digit after 'C' is the pattern (1 or 2)
    int pattern = inputStr[pos] - '0';
    pos++;
    // Remaining digits are the color index
    if (pos >= length) {
      Serial.println(F("\nERROR: Missing color index in C command"));
      Serial.write(0x7);
      return;
    }
    for (byte i = pos; i < length; i++) {
      if (!isdigit(inputStr[i])) {
        Serial.println(F("\nERROR: Color index must be numeric (0-11)"));
        Serial.write(0x7);
        return;
      }
    }
    int colorIndex = atoi(inputStr + pos);
    doCcommand(address, pattern, colorIndex);
    return;
  }

  // Store command character
  char command = inputStr[pos];
  pos++;

  if (verboseMode) {
    Serial.print(F("[DEBUG] Command: "));
    Serial.println(command);
  }

  // Parse numeric argument if present
  if (pos >= length) {
    hasArgument = false;
  } else {
    // Validate that remaining characters are digits
    for (byte i = pos; i < length; i++) {
      if (!isdigit(inputStr[i])) {
        Serial.println(F("\nERROR: Argument must be numeric"));
        Serial.write(0x7);
        return;
      }
    }
    argument = atoi(inputStr + pos);
    hasArgument = true;
  }

  if (verboseMode && hasArgument) {
    Serial.print(F("[DEBUG] Argument: "));
    Serial.println(argument);
  }

  // Execute command based on type
  if (verboseMode) {
    Serial.println(F("[DEBUG] Executing command..."));
  }

  switch (command) {
    case 'T':
      if (hasArgument) {
        doTcommand(address, argument);
      } else {
        Serial.println(F("\nERROR: T command requires argument. Example: 0T1"));
        Serial.write(0x7);
      }
      break;
    case 'D':
      doDcommand(address);
      break;
    case 'P':
      if (hasArgument) {
        doPcommand(address, argument);
      } else {
        Serial.println(F("\nERROR: P command requires argument (60=Latin, 61=Aurabesh)"));
        Serial.write(0x7);
      }
      break;
    case 'R':
      if (hasArgument) {
        doRcommand(address, argument);
      } else {
        Serial.println(F("\nERROR: R command requires argument (0-6 for random style)"));
        Serial.write(0x7);
      }
      break;
    case 'S':
      if (hasArgument) {
        doScommand(address, argument);
      } else {
        Serial.println(F("\nERROR: S command requires argument (0-4 for PSI state)"));
        Serial.write(0x7);
      }
      break;
    case 'L':
      // NEW v4.2: L-Command for brightness control
      if (hasArgument) {
        doLcommand(address, argument);
      } else {
        Serial.println(F("\nERROR: L command requires argument (0-15 for brightness)"));
        Serial.write(0x7);
      }
      break;
    default:
      Serial.print(F("\nERROR: Unknown command '"));
      Serial.print(command);
      Serial.println(F("'. Valid: T/M/P/R/S/L/C/D. Type '?" "?' for help."));
      Serial.write(0x7);
      break;
  }

  // NEW v4.2: Verbose mode execution time
  if (verboseMode) {
    unsigned long elapsed = millis() - startTime;
    Serial.print(F("[DEBUG] Command completed in "));
    Serial.print(elapsed);
    Serial.println(F(" ms"));
  }
}

void doMcommand(int address, char* message) {
  Serial.print(F("\nCommand: M, Address: "));
  Serial.print(address);
  Serial.print(F(", Message: "));
  Serial.print(message);
  
  if (address == 0) {
    setText(0, message);
    setText(1, message);
    setText(2, message);
    resetAllText();
  }
  if (address == 1) {
    setText(0, message);
    resetText(0);
  }
  if (address == 2) {
    setText(1, message);
    resetText(1);
  }
  if (address == 3) {
    setText(2, message);
    resetText(2);
  }
}

void doTcommand(int address, int argument) {
  Serial.print(F("\nCommand: T, Address: "));
  Serial.print(address);
  Serial.print(F(", Arg: "));
  Serial.print(argument);
  
  switch (argument) {
    case 0:
      exitEffects();
      if (address == 0) {
        displayState[0] = displayState[1] = displayState[2] = STATE_TEST;
        psiState[0] = psiState[1] = PSI_TEST;
        resetAllText();
      }
      if (address == 1) { displayState[0] = STATE_TEST; resetText(0); }
      if (address == 2) { displayState[1] = STATE_TEST; resetText(1); }
      if (address == 3) { displayState[2] = STATE_TEST; resetText(2); }
      if (address == 4) psiState[0] = PSI_TEST;
      if (address == 5) psiState[1] = PSI_TEST;
      break;
      
    case 1:
      exitEffects();
      if (address == 0) {
        displayState[0] = displayState[1] = displayState[2] = STATE_RANDOM;
        psiState[0] = psiState[1] = PSI_RANDOM;
        resetAllText();
      }
      if (address == 1) { displayState[0] = STATE_RANDOM; resetText(0); }
      if (address == 2) { displayState[1] = STATE_RANDOM; resetText(1); }
      if (address == 3) { displayState[2] = STATE_RANDOM; resetText(2); }
      if (address == 4) psiState[0] = PSI_RANDOM;
      if (address == 5) psiState[1] = PSI_RANDOM;
      break;
      
    case 2:
    case 3:
    case 5:
      exitEffects();
      currentEffect = EFF_ALARM;
      break;
      
    case 4:
      exitEffects();
      currentEffect = EFF_FAILURE;
      break;
      
    case 6:
      exitEffects();
      currentEffect = EFF_LEIA;
      break;
      
    case 10:
      exitEffects();
      for (byte disp = 0; disp < 3; disp++) {
        resetText(disp);
        alphabetType[disp] = ALPHA_LATIN;
        displayState[disp] = STATE_TEXT;
      }
      setText(0, "STAR    ");
      setText(1, "    WARS");
      setText(2, "STAR WARS   ");
      break;
      
    case 11:
      exitEffects();
      currentEffect = EFF_MARCH;
      break;
      
    case 20:
      exitEffects();
      if (address == 0) {
        displayState[0] = displayState[1] = displayState[2] = STATE_OFF;
        psiState[0] = psiState[1] = PSI_OFF;
        resetAllText();
      }
      if (address == 1) { displayState[0] = STATE_OFF; resetText(0); }
      if (address == 2) { displayState[1] = STATE_OFF; resetText(1); }
      if (address == 3) { displayState[2] = STATE_OFF; resetText(2); }
      if (address == 4) psiState[0] = PSI_OFF;
      if (address == 5) psiState[1] = PSI_OFF;
      break;
      
    case 92:
      exitEffects();
      if (address == 0) {
        displayState[0] = displayState[1] = displayState[2] = STATE_BARGRAPH;
        resetAllText();
      }
      if (address == 1) { displayState[0] = STATE_BARGRAPH; resetText(0); }
      if (address == 2) { displayState[1] = STATE_BARGRAPH; resetText(1); }
      if (address == 3) { displayState[2] = STATE_BARGRAPH; resetText(2); }
      break;
      
    case 100:
      exitEffects();
      if (address == 0) {
        displayState[0] = displayState[1] = displayState[2] = STATE_TEXT;
        resetAllText();
      }
      if (address == 1) { displayState[0] = STATE_TEXT; resetText(0); }
      if (address == 2) { displayState[1] = STATE_TEXT; resetText(1); }
      if (address == 3) { displayState[2] = STATE_TEXT; resetText(2); }
      break;

    case 99:
      // NEW v4.2: T99 - Stop all effects, return to normal operation
      exitEffects();
      Serial.print(F(" [Effects stopped, returning to normal]"));
      break;

    default:
      exitEffects();
      break;
  }
}

void doDcommand(int address) {
  Serial.print(F("\nCommand: D, Address: "));
  Serial.print(address);
}

void doPcommand(int address, int argument) {
  Serial.print(F("\nCommand: P, Address: "));
  Serial.print(address);
  Serial.print(F(", Arg: "));
  Serial.print(argument);
  
  if (argument == 60) {
    if (address == 0) {
      alphabetType[0] = alphabetType[1] = alphabetType[2] = ALPHA_LATIN;
    }
    if (address == 1) alphabetType[0] = ALPHA_LATIN;
    if (address == 2) alphabetType[1] = ALPHA_LATIN;
    if (address == 3) alphabetType[2] = ALPHA_LATIN;
  } else if (argument == 61) {
    if (address == 0) {
      alphabetType[0] = alphabetType[1] = alphabetType[2] = ALPHA_AURABESH;
    }
    if (address == 1) alphabetType[0] = ALPHA_AURABESH;
    if (address == 2) alphabetType[1] = ALPHA_AURABESH;
    if (address == 3) alphabetType[2] = ALPHA_AURABESH;
  }
}

void doRcommand(int address, int argument) {
  Serial.print(F("\nCommand: R, Address: "));
  Serial.print(address);
  Serial.print(F(", Style: "));
  Serial.print(argument);
  
  if (argument > 6) argument = 6;
  
  if (address == 0) {
    randomStyle[0] = randomStyle[1] = randomStyle[2] = argument;
  }
  if (address == 1) randomStyle[0] = argument;
  if (address == 2) randomStyle[1] = argument;
  if (address == 3) randomStyle[2] = argument;
}

void doScommand(int address, int argument) {
  Serial.print(F("\nCommand: S, Address: "));
  Serial.print(address);
  Serial.print(F(", State: "));
  Serial.print(argument);

  PsiState state;
  switch (argument) {
    case 0: state = PSI_TEST; break;
    case 1: state = PSI_RANDOM; break;
    case 2: state = PSI_COLOR1; break;
    case 3: state = PSI_COLOR2; break;
    case 4: state = PSI_OFF; break;
    default: return;
  }

  if (address == 0) {
    psiState[0] = psiState[1] = state;
  }
  if (address == 4) psiState[0] = state;
  if (address == 5) psiState[1] = state;
}

// =======================================================================================
// NEW v4.2: L-Command - Brightness/Lightness Control
// =======================================================================================
void doLcommand(int address, int argument) {
  Serial.print(F("\nCommand: L, Address: "));
  Serial.print(address);
  Serial.print(F(", Brightness: "));
  Serial.print(argument);

  // Validate brightness range (0-15)
  if (argument > 15) {
    Serial.print(F(" [ERROR: Brightness must be 0-15]"));
    return;
  }

  // Apply brightness to specified display(s) and PSI(s)
  if (address == 0) {
    // Address 0: Set all displays and both analog PSIs
    currentSettings.rldBrightness = argument;
    currentSettings.fldBrightness = argument;
    currentSettings.rpsiBrightness = argument;
    currentSettings.fpsiBrightness = argument;
    Serial.print(F(" [All displays & PSIs]"));
  }
  else if (address == 1) {
    // Address 1: Top FLD
    currentSettings.fldBrightness = argument;
    Serial.print(F(" [Top FLD]"));
  }
  else if (address == 2) {
    // Address 2: Bottom FLD
    currentSettings.fldBrightness = argument;
    Serial.print(F(" [Bottom FLD]"));
  }
  else if (address == 3) {
    // Address 3: RLD
    currentSettings.rldBrightness = argument;
    Serial.print(F(" [RLD]"));
  }
  else if (address == 4) {
    // Address 4: Front PSI (analog only)
    currentSettings.fpsiBrightness = argument;
    Serial.print(F(" [Front PSI]"));
  }
  else if (address == 5) {
    // Address 5: Rear PSI (analog only)
    currentSettings.rpsiBrightness = argument;
    Serial.print(F(" [Rear PSI]"));
  }

  // Apply the new brightness settings immediately
  applyBrightnessSettings();
}

// =======================================================================================
// NEW v4.2: C-Command - Digital PSI Color Control
// =======================================================================================
void doCcommand(int address, int pattern, int colorIndex) {
  Serial.print(F("\nCommand: C, Address: "));
  Serial.print(address);
  Serial.print(F(", Pattern: "));
  Serial.print(pattern);
  Serial.print(F(", Color: "));
  Serial.print(colorIndex);

  // Validate color index (0-11)
  if (!isValidColorIndex(colorIndex)) {
    Serial.print(F(" [ERROR: Color index must be 0-11]"));
    return;
  }

  // Validate address (only 4 and 5 are valid for PSI)
  if (address != 4 && address != 5) {
    Serial.print(F(" [ERROR: Address must be 4 (Front PSI) or 5 (Rear PSI)]"));
    return;
  }

  // Validate pattern (1 or 2)
  if (pattern != 1 && pattern != 2) {
    Serial.print(F(" [ERROR: Pattern must be 1 or 2]"));
    return;
  }

  // Apply color to specified PSI pattern
  if (address == 4) {
    // Front PSI
    if (pattern == 1) {
      currentSettings.digitalPsi1_color1_index = colorIndex;
      Serial.print(F(" [Front PSI Color1]"));
    } else {
      currentSettings.digitalPsi1_color2_index = colorIndex;
      Serial.print(F(" [Front PSI Color2]"));
    }
  }
  else if (address == 5) {
    // Rear PSI
    if (pattern == 1) {
      currentSettings.digitalPsi2_color1_index = colorIndex;
      Serial.print(F(" [Rear PSI Color1]"));
    } else {
      currentSettings.digitalPsi2_color2_index = colorIndex;
      Serial.print(F(" [Rear PSI Color2]"));
    }
  }

  // Color changes will be applied on next PSI update cycle
  Serial.print(F(" (will apply on next update)"));
}

// =======================================================================================
// NEW v4.1: DIAGNOSTICS MODE
// =======================================================================================

void runDiagnostics() {
  Serial.println(F("\n╔══════════════════════════════════════════════════════════╗"));
  Serial.println(F("║         HARDWARE DIAGNOSTICS - Testing Components        ║"));
  Serial.println(F("╚══════════════════════════════════════════════════════════╝\n"));
  
  bool allPassed = true;
  
  Serial.print(F("[ 1/8 ] Flash Memory (Preferences)... "));
  if (preferences.isKey("psiOutput")) {
    Serial.println(F("✓ OK"));
  } else {
    Serial.println(F("✗ FAIL - Storage not initialized"));
    allPassed = false;
  }
  delay(DIAGNOSTICS_DELAY_MS);
  
  Serial.print(F("[ 2/8 ] Rear Logic Display (GPIO7/6/5)... "));
  for (int dev = 0; dev < 3; dev++) {
    lcRear.setLed(dev, 0, 0, true);
    delay(50);
    lcRear.setLed(dev, 0, 0, false);
  }
  Serial.println(F("✓ OK (check visual confirmation)"));
  delay(DIAGNOSTICS_DELAY_MS);
  
  Serial.print(F("[ 3/8 ] Front Logic Displays (GPIO4/3/2)... "));
  for (int dev = 0; dev < 2; dev++) {
    lcFront.setLed(dev, 0, 0, true);
    delay(50);
    lcFront.setLed(dev, 0, 0, false);
  }
  Serial.println(F("✓ OK (check visual confirmation)"));
  delay(DIAGNOSTICS_DELAY_MS);
  
  Serial.print(F("[ 4/8 ] Rear Analog PSI... "));
  lcRear.setRow(RPSIDEV, 0, 0xFF);
  delay(200);
  lcRear.setRow(RPSIDEV, 0, 0x00);
  Serial.println(F("✓ OK (check if LED lit up)"));
  delay(DIAGNOSTICS_DELAY_MS);
  
  Serial.print(F("[ 5/8 ] Front Analog PSI... "));
  lcFront.setRow(FPSIDEV, 0, 0xFF);
  delay(200);
  lcFront.setRow(FPSIDEV, 0, 0x00);
  Serial.println(F("✓ OK (check if LED lit up)"));
  delay(DIAGNOSTICS_DELAY_MS);
  
  Serial.print(F("[ 6/8 ] Digital PSI 1 (GPIO8)... "));
  digitalPsi1.clear();
  digitalPsi1.setPixelColor(0, digitalPsi1.Color(255, 0, 0));
  digitalPsi1.show();
  delay(200);
  digitalPsi1.clear();
  digitalPsi1.show();
  Serial.println(F("✓ OK (check if first LED lit RED)"));
  delay(DIAGNOSTICS_DELAY_MS);
  
  Serial.print(F("[ 7/8 ] Digital PSI 2 (GPIO9)... "));
  digitalPsi2.clear();
  digitalPsi2.setPixelColor(0, digitalPsi2.Color(0, 255, 0));
  digitalPsi2.show();
  delay(200);
  digitalPsi2.clear();
  digitalPsi2.show();
  Serial.println(F("✓ OK (check if first LED lit GREEN)"));
  delay(DIAGNOSTICS_DELAY_MS);
  
  Serial.print(F("[ 8/8 ] Watchdog Timer... "));
  esp_task_wdt_reset();
  Serial.println(F("✓ OK"));
  delay(DIAGNOSTICS_DELAY_MS);
  
  Serial.println(F("\n═══════════════════════════════════════════════════════════"));
  if (allPassed) {
    Serial.println(F("✓ DIAGNOSTICS COMPLETE - All automated tests passed"));
  } else {
    Serial.println(F("⚠ DIAGNOSTICS COMPLETE - Some tests failed (see above)"));
  }
  Serial.println(F("ℹ Visual tests require manual confirmation"));
  Serial.println(F("ℹ If any display didn't light up, check wiring and power"));
  Serial.println(F("═══════════════════════════════════════════════════════════\n"));
  
  Serial.println(F("TROUBLESHOOTING TIPS:"));
  Serial.println(F("  • No lights at all? Check 5V power supply"));
  Serial.println(F("  • RLD not working? Check GPIO 7/6/5 connections"));
  Serial.println(F("  • FLD not working? Check GPIO 4/3/2 connections"));
  Serial.println(F("  • Digital PSI issues? Check GPIO 8/9 and 5V power"));
  Serial.println(F("  • Analog PSI issues? Ensure they're in the daisy chain"));
  Serial.println(F("  • Random crashes? Check for loose connections\n"));
}

// =======================================================================================
// NEW v4.1: SMART SUGGESTIONS
// =======================================================================================

void showSmartSuggestion(const char* context, const char* param) {
  if (strcmp(context, "psi_output_digital") == 0) {
    Serial.println(F("\nℹ SMART TIP:"));
    Serial.println(F("  Digital PSI activated! Next steps:"));
    Serial.println(F("  1. Type 'colors' to see all 12 color options"));
    Serial.println(F("  2. Try: set psi1_color1 11  (sets front to WHITE)"));
    Serial.println(F("  3. Or load preset: preset 3  (KT mode)"));
    
  } else if (strcmp(context, "psi_output_analog") == 0) {
    Serial.println(F("\nℹ SMART TIP:"));
    Serial.println(F("  Analog PSI activated! This uses MAX7219 chips."));
    Serial.println(F("  Color changes via hardware (red/blue LEDs)."));
    Serial.println(F("  Adjust brightness: set fpsi_bright <0-15>"));
    
  } else if (strcmp(context, "profile_loaded") == 0) {
    Serial.println(F("\nℹ SMART TIP:"));
    Serial.print(F("  Profile "));
    Serial.print(activeProfileIndex + 1);
    Serial.println(F(" is now active."));
    Serial.println(F("  • Type 'show' to see all settings"));
    if (activeProfileIndex >= 2) {
      Serial.println(F("  • Make changes, then: profile save"));
    } else {
      Serial.println(F("  • This profile is read-only (load 3-5 to edit)"));
    }
    
  } else if (strcmp(context, "brightness_changed") == 0) {
    Serial.println(F("\nℹ SMART TIP:"));
    Serial.println(F("  Brightness changed! Effects apply immediately."));
    Serial.println(F("  Too dim? Try values 8-12 for indoor use."));
    Serial.println(F("  Too bright? Use 2-5 for dark environments."));
    
  } else if (strcmp(context, "color_changed") == 0) {
    Serial.println(F("\nℹ SMART TIP:"));
    Serial.println(F("  PSI color updated! Send '4S1' to test (front PSI random)."));
    Serial.println(F("  Set both colors for best effect:"));
    Serial.println(F("    set psi1_color1 <num>  (pattern 1)"));
    Serial.println(F("    set psi1_color2 <num>  (pattern 2)"));
    
  } else if (strcmp(context, "unsaved_changes") == 0) {
    Serial.println(F("\n⚠ REMINDER:"));
    Serial.println(F("  You have unsaved changes!"));
    Serial.println(F("  Type 'profile save' to keep them."));
    Serial.println(F("  Or 'exit' to discard changes."));
    
  } else if (strcmp(context, "first_time") == 0) {
    Serial.println(F("\n👋 WELCOME TO TEECES v4.1!"));
    Serial.println(F("  First time here? Type 'wizard' for guided setup."));
    Serial.println(F("  Or jump right in with 'help' for all commands."));
    Serial.println(F("  Quick test: Send '0T1' to see random displays!"));
    
  } else if (strcmp(context, "preset_applied") == 0) {
    Serial.println(F("\nℹ SMART TIP:"));
    Serial.println(F("  Preset applied! These are starting points."));
    Serial.println(F("  Adjust any setting with 'set <param> <value>'."));
    Serial.println(F("  Save your tweaks: profile save (on profiles 3-5)."));
    
  } else if (strcmp(context, "wizard_complete") == 0) {
    Serial.println(F("\n✓ SETUP COMPLETE!"));
    Serial.println(F("  Your droid is configured and ready."));
    Serial.println(F("  • Use JawaLite commands (e.g. '0T1' for random)"));
    Serial.println(F("  • Type '*' anytime to return to config menu"));
    Serial.println(F("  • Type 'show' to review all settings"));
  }
}

// =======================================================================================
// NEW v4.1: ENHANCED PARAMETER VALIDATION WITH HINTS
// =======================================================================================

bool validateParameterWithHints(const char* paramName, int value, int currentValue) {
  if (strcmp(paramName, "rld_bright") == 0 || strcmp(paramName, "rpsi_bright") == 0 ||
      strcmp(paramName, "fld_bright") == 0 || strcmp(paramName, "fpsi_bright") == 0) {
    if (value < 0 || value > 15) {
      Serial.println(F("\n✗ OUT OF RANGE!"));
      Serial.println(F("  Valid range: 0-15"));
      Serial.print(F("  Current value: ")); Serial.println(currentValue);
      Serial.println(F("  Suggested values:"));
      Serial.println(F("    • 2-5   = Very dim (dark room)"));
      Serial.println(F("    • 6-9   = Medium (normal lighting)"));
      Serial.println(F("    • 10-15 = Bright (outdoor)"));
      return false;
    }
    return true;
  }
  
  if (strcmp(paramName, "logic_style") == 0) {
    if (value < 0 || value > 6) {
      Serial.println(F("\n✗ OUT OF RANGE!"));
      Serial.println(F("  Valid range: 0-6"));
      Serial.print(F("  Current value: ")); Serial.println(currentValue);
      Serial.println(F("  Style guide:"));
      Serial.println(F("    • 0 = Very sparse (few LEDs)"));
      Serial.println(F("    • 3 = Balanced"));
      Serial.println(F("    • 6 = Very dense (many LEDs)"));
      return false;
    }
    return true;
  }
  
  if (strcmp(paramName, "logic_delay") == 0) {
    if (value < 1 || value > 60000) {
      Serial.println(F("\n✗ OUT OF RANGE!"));
      Serial.println(F("  Valid range: 1-60000 ms"));
      Serial.print(F("  Current value: ")); Serial.println(currentValue);
      Serial.println(F("  Typical values:"));
      Serial.println(F("    • 50-100  = Very fast updates"));
      Serial.println(F("    • 150-250 = Normal speed (default: 150)"));
      Serial.println(F("    • 300-500 = Slow, deliberate"));
      return false;
    }
    return true;
  }
  
  if (strcmp(paramName, "scroll_speed") == 0) {
    if (value < 1 || value > 60000) {
      Serial.println(F("\n✗ OUT OF RANGE!"));
      Serial.println(F("  Valid range: 1-60000 ms"));
      Serial.print(F("  Current value: ")); Serial.println(currentValue);
      Serial.println(F("  Typical values:"));
      Serial.println(F("    • 20-40  = Fast scroll"));
      Serial.println(F("    • 50-70  = Normal (default: 55)"));
      Serial.println(F("    • 80-100 = Slow, readable"));
      return false;
    }
    return true;
  }
  
  if (strcmp(paramName, "psi_wipe_delay") == 0) {
    if (value < 1 || value > 1000) {
      Serial.println(F("\n✗ OUT OF RANGE!"));
      Serial.println(F("  Valid range: 1-1000 ms"));
      Serial.print(F("  Current value: ")); Serial.println(currentValue);
      Serial.println(F("  Typical values:"));
      Serial.println(F("    • 30-50  = Fast PSI animation"));
      Serial.println(F("    • 75-100 = Normal (default: 75)"));
      Serial.println(F("    • 120-200 = Slow, smooth"));
      return false;
    }
    return true;
  }
  
  if (strcmp(paramName, "psi1_color1") == 0 || strcmp(paramName, "psi1_color2") == 0 ||
      strcmp(paramName, "psi2_color1") == 0 || strcmp(paramName, "psi2_color2") == 0) {
    if (value < 0 || value >= numColors) {
      Serial.println(F("\n✗ OUT OF RANGE!"));
      Serial.print(F("  Valid range: 0-")); Serial.println(numColors - 1);
      Serial.println(F("  Type 'colors' to see the full palette."));
      Serial.println(F("  Popular choices:"));
      Serial.println(F("    • 0=RED, 1=GREEN, 2=BLUE"));
      Serial.println(F("    • 8=PINK, 11=WHITE"));
      return false;
    }
    return true;
  }
  
  if (strcmp(paramName, "psi_output") == 0) {
    if (value < 0 || value > 1) {
      Serial.println(F("\n✗ OUT OF RANGE!"));
      Serial.println(F("  Valid values: 0 or 1"));
      Serial.print(F("  Current: ")); 
      Serial.println(globalPsiOutput == PSI_ANALOG ? F("0 (ANALOG)") : F("1 (DIGITAL)"));
      Serial.println(F("  Options:"));
      Serial.println(F("    • 0 = Analog PSI (MAX7219 hardware)"));
      Serial.println(F("    • 1 = Digital PSI (NeoPixel strips)"));
      return false;
    }
    return true;
  }
  
  if (strcmp(paramName, "psi_stuck_freq") == 0) {
    if (value < 1 || value > 100) {
      Serial.println(F("\n✗ OUT OF RANGE!"));
      Serial.println(F("  Valid range: 1-100"));
      Serial.print(F("  Current value: ")); Serial.println(currentValue);
      Serial.println(F("  Higher number = more frequent stuck effect"));
      Serial.println(F("  Recommended: 5-20 for occasional, 1 for rare"));
      return false;
    }
    return true;
  }
  
  if (strcmp(paramName, "psi_stuck_time") == 0) {
    if (value < 100 || value > 60000) {
      Serial.println(F("\n✗ OUT OF RANGE!"));
      Serial.println(F("  Valid range: 100-60000 ms"));
      Serial.print(F("  Current value: ")); Serial.println(currentValue);
      Serial.println(F("  How long PSI stays 'stuck' before resuming."));
      Serial.println(F("  Recommended: 3000-7000 ms"));
      return false;
    }
    return true;
  }
  
  return true;
}

#pragma GCC diagnostic pop

// =======================================================================================
// NEW v4.1: PRESET SYSTEM
// =======================================================================================

void showPresets() {
  Serial.println(F("\n╔══════════════════════════════════════════════════════════╗"));
  Serial.println(F("║                    QUICK PRESETS                         ║"));
  Serial.println(F("╚══════════════════════════════════════════════════════════╝\n"));
  
  Serial.println(F("Presets are pre-configured settings for common use cases.\n"));
  
  for (int i = 0; i < NUM_PRESETS; i++) {
    Serial.print(F("  "));
    Serial.print(i + 1);
    Serial.print(F(". "));
    Serial.println((const __FlashStringHelper*)pgm_read_ptr(&(presets[i].name)));
    Serial.print(F("     "));
    Serial.println((const __FlashStringHelper*)pgm_read_ptr(&(presets[i].description)));
    Serial.println();
  }
  
  Serial.println(F("Usage: preset <1-5>"));
  Serial.println(F("Example: preset 3  (applies KT preset)"));
  Serial.println(F("\nℹ Presets modify your current profile. Use 'profile save' to keep changes."));
}

void applyPreset(byte presetNum) {
  if (presetNum < 1 || presetNum > NUM_PRESETS) {
    Serial.println(F("✗ Invalid preset number. Use 1-5."));
    Serial.println(F("  Type 'presets' to see all options."));
    return;
  }
  
  byte idx = presetNum - 1;
  
  byte brightness = pgm_read_byte(&(presets[idx].brightness));
  PsiOutputType psiMode = (PsiOutputType)pgm_read_byte(&(presets[idx].psiMode));
  byte psiColor1 = pgm_read_byte(&(presets[idx].psiColor1));
  byte psiColor2 = pgm_read_byte(&(presets[idx].psiColor2));
  byte logicStyle = pgm_read_byte(&(presets[idx].logicStyle));
  unsigned int scrollSpeed = pgm_read_word(&(presets[idx].scrollSpeed));
  
  currentSettings.rldBrightness = brightness;
  currentSettings.fldBrightness = brightness;
  currentSettings.rpsiBrightness = brightness;
  currentSettings.fpsiBrightness = brightness;
  currentSettings.logicRandomStyle = logicStyle;
  currentSettings.scrollSpeed = scrollSpeed;
  
  globalPsiOutput = psiMode;
  preferences.putUChar("psiOutput", psiMode);
  
  if (psiMode == PSI_DIGITAL) {
    currentSettings.digitalPsi1_color1_index = psiColor1;
    currentSettings.digitalPsi1_color2_index = psiColor2;
    currentSettings.digitalPsi2_color1_index = psiColor1;
    currentSettings.digitalPsi2_color2_index = psiColor2;
  }
  
  applyBrightnessSettings();
  hasUnsavedChanges = true;
  
  Serial.print(F("\n✓ Preset '"));
  Serial.print((const __FlashStringHelper*)pgm_read_ptr(&(presets[idx].name)));
  Serial.println(F("' applied!"));
  
  Serial.println(F("\nApplied settings:"));
  Serial.print(F("  Brightness: ")); Serial.println(brightness);
  Serial.print(F("  PSI Mode: ")); Serial.println(psiMode == PSI_ANALOG ? F("ANALOG") : F("DIGITAL"));
  if (psiMode == PSI_DIGITAL) {
    Serial.print(F("  PSI Colors: "));
    Serial.print((const __FlashStringHelper*)pgm_read_ptr(&(colorPalette[psiColor1].name)));
    Serial.print(F(" / "));
    Serial.println((const __FlashStringHelper*)pgm_read_ptr(&(colorPalette[psiColor2].name)));
  }
  Serial.print(F("  Logic Style: ")); Serial.println(logicStyle);
  Serial.print(F("  Scroll Speed: ")); Serial.print(scrollSpeed); Serial.println(F(" ms"));
  
  showSmartSuggestion("preset_applied");
}

// =======================================================================================
// HELPER FUNCTIONS FOR SAFE SERIAL INPUT
// =======================================================================================

// Trim leading and trailing whitespace from a string
void trimString(char* str) {
  if (str == nullptr || *str == '\0') return;

  // Trim leading whitespace
  char* start = str;
  while (*start && (*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n')) {
    start++;
  }

  // Trim trailing whitespace
  char* end = start + strlen(start) - 1;
  while (end > start && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')) {
    *end = '\0';
    end--;
  }

  // Shift string if needed
  if (start != str) {
    memmove(str, start, strlen(start) + 1);
  }
}

// Case-insensitive string comparison
int strcasecmp_safe(const char* s1, const char* s2) {
  while (*s1 && *s2) {
    char c1 = tolower(*s1);
    char c2 = tolower(*s2);
    if (c1 != c2) return c1 - c2;
    s1++;
    s2++;
  }
  return tolower(*s1) - tolower(*s2);
}

// Check if string starts with prefix
bool startsWith(const char* str, const char* prefix) {
  size_t lenstr = strlen(str);
  size_t lenprefix = strlen(prefix);
  if (lenstr < lenprefix) return false;
  return strncmp(str, prefix, lenprefix) == 0;
}

// Sanitize text input - allow only printable ASCII characters (32-126)
void sanitizeText(char* str) {
  if (str == nullptr) return;

  for (char* p = str; *p; p++) {
    // Replace non-printable characters with space
    if (*p < 32 || *p > 126) {
      *p = ' ';
    }
  }
}

// Read a line from serial with timeout (replaces String.readStringUntil)
// Returns true if successful, false on timeout
bool readSerialLineWithTimeout(char* buffer, size_t bufferSize, unsigned long timeoutMs) {
  unsigned long startTime = millis();
  size_t index = 0;

  buffer[0] = '\0';  // Start with empty string

  while (true) {
    esp_task_wdt_reset();

    // Check timeout
    if (millis() - startTime > timeoutMs) {
      buffer[index] = '\0';
      return false;  // Timeout
    }

    if (Serial.available()) {
      char c = Serial.read();

      // Line endings
      if (c == '\n' || c == '\r') {
        buffer[index] = '\0';
        trimString(buffer);
        return true;
      }

      // Store character if there's space
      if (index < bufferSize - 1) {
        buffer[index++] = c;
      }
    }

    delay(10);  // Small delay to prevent tight loop
  }
}

// =======================================================================================
// NEW v4.1: SETUP WIZARD
// =======================================================================================

void runSetupWizard() {
  Serial.println(F("\n╔══════════════════════════════════════════════════════════╗"));
  Serial.println(F("║         WELCOME TO TEECES v4.1 - SETUP WIZARD           ║"));
  Serial.println(F("╚══════════════════════════════════════════════════════════╝\n"));
  
  Serial.println(F("This wizard will guide you through basic configuration.\n"));
  Serial.println(F("Press ENTER after each answer.\n"));
  
  delay(1000);
  
  // Step 1: PSI Type
  Serial.println(F("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"));
  Serial.println(F("STEP 1/4: PSI Type"));
  Serial.println(F("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"));
  Serial.println(F("\nWhich PSI lights do you have installed?"));
  Serial.println(F("  1 = Analog PSI (traditional MAX7219 with red/blue LEDs)"));
  Serial.println(F("  2 = Digital PSI (NeoPixel/WS2812B strips with any color)"));
  Serial.println(F("\nRecommended: Digital PSI (more features)"));
  Serial.print(F("\nYour choice (1 or 2): "));

  char psiChoice[16];
  if (!readSerialLineWithTimeout(psiChoice, sizeof(psiChoice), 60000)) {
    // Timeout - use default
    strcpy(psiChoice, "2");
    Serial.println(F("Timeout - using default: 2 (Digital)"));
  } else {
    Serial.println(psiChoice);
  }

  if (strcmp(psiChoice, "2") == 0) {
    globalPsiOutput = PSI_DIGITAL;
    preferences.putUChar("psiOutput", PSI_DIGITAL);
    Serial.println(F("✓ Digital PSI selected!"));
  } else {
    globalPsiOutput = PSI_ANALOG;
    preferences.putUChar("psiOutput", PSI_ANALOG);
    Serial.println(F("✓ Analog PSI selected!"));
  }
  
  delay(1000);
  
  // Step 2: Brightness
  Serial.println(F("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"));
  Serial.println(F("STEP 2/4: Brightness"));
  Serial.println(F("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"));
  Serial.println(F("\nWhere will your droid be displayed?"));
  Serial.println(F("  1 = Dark room (brightness 3-5)"));
  Serial.println(F("  2 = Normal lighting (brightness 6-9)"));
  Serial.println(F("  3 = Bright/outdoor (brightness 10-15)"));
  Serial.print(F("\nYour choice (1-3): "));

  char brightChoice[16];
  if (!readSerialLineWithTimeout(brightChoice, sizeof(brightChoice), 60000)) {
    // Timeout - use default
    strcpy(brightChoice, "2");
    Serial.println(F("Timeout - using default: 2 (Normal)"));
  } else {
    Serial.println(brightChoice);
  }

  byte brightness = 8;
  if (strcmp(brightChoice, "1") == 0) brightness = 4;
  else if (strcmp(brightChoice, "2") == 0) brightness = 8;
  else if (strcmp(brightChoice, "3") == 0) brightness = 12;
  
  currentSettings.rldBrightness = brightness;
  currentSettings.fldBrightness = brightness;
  currentSettings.rpsiBrightness = brightness;
  currentSettings.fpsiBrightness = brightness;
  applyBrightnessSettings();
  
  Serial.print(F("✓ Brightness set to "));
  Serial.println(brightness);
  
  delay(1000);
  
  // Step 3: Colors (only for digital PSI)
  if (globalPsiOutput == PSI_DIGITAL) {
    Serial.println(F("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"));
    Serial.println(F("STEP 3/4: PSI Colors"));
    Serial.println(F("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"));
    Serial.println(F("\nChoose a color scheme:"));
    Serial.println(F("  1 = Classic (Green/Yellow)"));
    Serial.println(F("  2 = Movie Accurate (Red/Blue)"));
    Serial.println(F("  3 = Modern (White/Pink)"));
    Serial.println(F("  4 = Rainbow (Cyan/Magenta)"));
    Serial.print(F("\nYour choice (1-4): "));

    char colorChoice[16];
    if (!readSerialLineWithTimeout(colorChoice, sizeof(colorChoice), 60000)) {
      // Timeout - use default
      strcpy(colorChoice, "1");
      Serial.println(F("Timeout - using default: 1 (Classic)"));
    } else {
      Serial.println(colorChoice);
    }

    byte color1 = 1, color2 = 3;
    if (strcmp(colorChoice, "1") == 0) { color1 = 1; color2 = 3; }
    else if (strcmp(colorChoice, "2") == 0) { color1 = 0; color2 = 2; }
    else if (strcmp(colorChoice, "3") == 0) { color1 = 11; color2 = 8; }
    else if (strcmp(colorChoice, "4") == 0) { color1 = 4; color2 = 5; }
    
    currentSettings.digitalPsi1_color1_index = color1;
    currentSettings.digitalPsi1_color2_index = color2;
    currentSettings.digitalPsi2_color1_index = color1;
    currentSettings.digitalPsi2_color2_index = color2;
    
    Serial.print(F("✓ Colors set to "));
    Serial.print((const __FlashStringHelper*)pgm_read_ptr(&(colorPalette[color1].name)));
    Serial.print(F(" / "));
    Serial.println((const __FlashStringHelper*)pgm_read_ptr(&(colorPalette[color2].name)));
  } else {
    Serial.println(F("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"));
    Serial.println(F("STEP 3/4: PSI Colors (Skipped - Analog Mode)"));
    Serial.println(F("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"));
    Serial.println(F("ℹ Analog PSI uses hardware colors (red/blue LEDs)"));
  }
  
  delay(1000);
  
  // Step 4: Profile Selection
  Serial.println(F("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"));
  Serial.println(F("STEP 4/4: Save Configuration"));
  Serial.println(F("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"));
  Serial.println(F("\nSave these settings to a profile?"));
  Serial.println(F("  3 = User Profile 1 (recommended)"));
  Serial.println(F("  4 = User Profile 2"));
  Serial.println(F("  5 = User Profile 3"));
  Serial.print(F("\nYour choice (3-5): "));

  char profileChoice[16];
  if (!readSerialLineWithTimeout(profileChoice, sizeof(profileChoice), 60000)) {
    // Timeout - use default
    strcpy(profileChoice, "3");
    Serial.println(F("Timeout - using default: 3"));
  } else {
    Serial.println(profileChoice);
  }

  byte profileNum = atoi(profileChoice);
  if (profileNum >= 3 && profileNum <= 5) {
    loadProfile(profileNum);
    
    currentSettings.rldBrightness = brightness;
    currentSettings.fldBrightness = brightness;
    currentSettings.rpsiBrightness = brightness;
    currentSettings.fpsiBrightness = brightness;
    applyBrightnessSettings();
    
    saveCurrentProfile();
    
    Serial.print(F("✓ Settings saved to Profile "));
    Serial.println(profileNum);
  } else {
    Serial.println(F("⚠ Invalid choice - using Profile 3"));
    loadProfile(3);
    saveCurrentProfile();
  }
  
  delay(1000);
  
  // Summary
  Serial.println(F("\n╔══════════════════════════════════════════════════════════╗"));
  Serial.println(F("║                  SETUP COMPLETE! 🎉                      ║"));
  Serial.println(F("╚══════════════════════════════════════════════════════════╝\n"));
  
  Serial.println(F("Your configuration:"));
  Serial.print(F("  PSI Mode: "));
  Serial.println(globalPsiOutput == PSI_ANALOG ? F("Analog") : F("Digital"));
  Serial.print(F("  Brightness: ")); Serial.println(brightness);
  Serial.print(F("  Active Profile: ")); Serial.println(activeProfileIndex + 1);
  Serial.println();
  
  Serial.println(F("Next steps:"));
  Serial.println(F("  1. Test your displays: Send '0T1' for random mode"));
  Serial.println(F("  2. Test PSI: Send '0S1' for random PSI animation"));
  Serial.println(F("  3. See all settings: Type 'show'"));
  Serial.println(F("  4. Get help anytime: Type 'help'"));
  Serial.println();
  
  showSmartSuggestion("wizard_complete");
  
  preferences.putBool("setupDone", true);
  firstTimeSetup = false;
}

// =======================================================================================
// ENHANCED CONFIGURATION MENU
// =======================================================================================

void printConfigMenu() {
  Serial.println(F("\n"));
  Serial.println(F("╔══════════════════════════════════════════════════════════╗"));
  Serial.println(F("║         TEECES v4.1 - CONFIGURATION MENU                ║"));
  Serial.println(F("╚══════════════════════════════════════════════════════════╝"));
  Serial.println(F(""));
  
  Serial.print(F("[Profile "));
  Serial.print(activeProfileIndex + 1);
  Serial.print(F(" | PSI: "));
  Serial.print(globalPsiOutput == PSI_DIGITAL ? F("DGTL") : F("ANLG"));
  Serial.print(F(" | Changes: "));
  Serial.print(hasUnsavedChanges ? F("YES") : F("NO"));
  Serial.println(F("]"));
  Serial.println(F(""));
  
  Serial.println(F("Quick Commands:"));
  Serial.println(F("  help       - Show detailed help and examples"));
  Serial.println(F("  show       - Display all current settings"));
  Serial.println(F("  colors     - List available PSI colors (0-11)"));
  Serial.println(F("  wizard     - Run first-time setup wizard"));
  Serial.println(F("  diagnostics- Test all hardware components"));
  Serial.println(F("  presets    - Show quick configuration presets"));
  Serial.println(F("  exit       - Return to normal operation"));
  Serial.println(F(""));
  Serial.println(F("Main Settings:"));
  Serial.println(F("  1. Profile Management    (load/save profiles 1-5)"));
  Serial.println(F("  2. PSI Configuration     (mode, colors, swap)"));
  Serial.println(F("  3. Display Settings      (brightness, speed)"));
  Serial.println(F("  4. Boot Text             (startup messages)"));
  Serial.println(F(""));
  Serial.println(F("Type a number (1-4) or command name:"));
  Serial.print(F("Config> "));
}

void printProfileHelp() {
  Serial.println(F("\n--- PROFILE MANAGEMENT ---"));
  Serial.println(F("Profiles store all display and PSI settings."));
  Serial.println(F(""));
  Serial.println(F("Available Profiles:"));
  Serial.println(F("  1 - STANDARD     (read-only default)"));
  Serial.println(F("  2 - WHITE/PINK   (read-only, digital PSI KT)"));
  Serial.println(F("  3 - USER SLOT 1  (customizable)"));
  Serial.println(F("  4 - USER SLOT 2  (customizable)"));
  Serial.println(F("  5 - USER SLOT 3  (customizable)"));
  Serial.println(F(""));
  Serial.println(F("Commands:"));
  Serial.println(F("  profile show           - Show active profile"));
  Serial.println(F("  profile load <1-5>     - Switch to profile"));
  Serial.println(F("  profile save           - Save changes (slots 3-5 only)"));
  Serial.println(F("  profile reset <3-5>    - Reset to defaults"));
  Serial.println(F(""));
  Serial.println(F("Example: profile load 2"));
}

void printPsiHelp() {
  Serial.println(F("\n--- PSI CONFIGURATION ---"));
  Serial.println(F("PSI (Process State Indicator) lights can be analog or digital."));
  Serial.println(F(""));
  Serial.println(F("Global Setting (applies to ALL profiles):"));
  Serial.println(F("  set psi_output 0       - Use analog PSI (MAX7219)"));
  Serial.println(F("  set psi_output 1       - Use digital PSI (NeoPixel)"));
  Serial.println(F(""));
  Serial.println(F("Digital PSI Colors (per profile):"));
  Serial.println(F("  set psi1_color1 <0-11> - Front PSI, pattern 1"));
  Serial.println(F("  set psi1_color2 <0-11> - Front PSI, pattern 2"));
  Serial.println(F("  set psi2_color1 <0-11> - Rear PSI, pattern 1"));
  Serial.println(F("  set psi2_color2 <0-11> - Rear PSI, pattern 2"));
  Serial.println(F(""));
  Serial.println(F("Wiring:"));
  Serial.println(F("  set psi_swap 0         - Normal wiring"));
  Serial.println(F("  set psi_swap 1         - Swapped (front<->rear)"));
  Serial.println(F(""));
  Serial.println(F("Type 'colors' to see all color options."));
  Serial.println(F("Example: set psi_output 1"));
  Serial.println(F("Example: set psi1_color1 11"));
}

void printDisplayHelp() {
  Serial.println(F("\n--- DISPLAY SETTINGS ---"));
  Serial.println(F("Control brightness, speed, and behavior."));
  Serial.println(F(""));
  Serial.println(F("Brightness (0-15):"));
  Serial.println(F("  set rld_bright <0-15>  - Rear Logic Display"));
  Serial.println(F("  set fld_bright <0-15>  - Front Logic Displays"));
  Serial.println(F("  set rpsi_bright <0-15> - Rear PSI"));
  Serial.println(F("  set fpsi_bright <0-15> - Front PSI"));
  Serial.println(F(""));
  Serial.println(F("Speed & Timing (milliseconds):"));
  Serial.println(F("  set logic_delay <ms>   - Random update speed (50-500)"));
  Serial.println(F("  set scroll_speed <ms>  - Text scroll speed (20-100)"));
  Serial.println(F("  set psi_wipe_delay <ms>- PSI animation speed (50-200)"));
  Serial.println(F(""));
  Serial.println(F("Style:"));
  Serial.println(F("  set logic_style <0-6>  - Random density (0=sparse, 6=dense)"));
  Serial.println(F(""));
  Serial.println(F("Example: set fld_bright 10"));
  Serial.println(F("Example: set scroll_speed 40"));
}

void printBootTextHelp() {
  Serial.println(F("\n--- BOOT TEXT ---"));
  Serial.println(F("Customize the startup messages on each display."));
  Serial.println(F(""));
  Serial.println(F("Commands:"));
  Serial.println(F("  set boot_tfld <text>   - Top Front Logic Display"));
  Serial.println(F("  set boot_bfld <text>   - Bottom Front Logic Display"));
  Serial.println(F("  set boot_rld <text>    - Rear Logic Display"));
  Serial.println(F(""));
  Serial.println(F("Tips:"));
  Serial.println(F("  - Max 31 characters"));
  Serial.println(F("  - Spaces are allowed"));
  Serial.println(F("  - Text scrolls on startup"));
  Serial.println(F(""));
  Serial.println(F("Example: set boot_tfld HELLO R2"));
}

void printFullHelp() {
  Serial.println(F("\n"));
  Serial.println(F("══════════════════════════════════════════════════════════"));
  Serial.println(F("                 TEECES v4.2 QUICK START"));
  Serial.println(F("══════════════════════════════════════════════════════════"));
  Serial.println(F(""));
  Serial.println(F("NEW IN v4.2:"));
  Serial.println(F("  ⚡ L-Command   - Direct brightness control (0L10)"));
  Serial.println(F("  🎨 C-Command   - PSI color control (4C111)"));
  Serial.println(F("  ⏹  T99        - Stop all effects command"));
  Serial.println(F("  🐛 verbose     - Debug mode (verbose on/off)"));
  Serial.println(F("  ⌨  Shortcuts   - p1-p5, s, w, d, q for quick access"));
  Serial.println(F(""));
  Serial.println(F("v4.1 Features:"));
  Serial.println(F("  ✨ wizard      - First-time setup assistant"));
  Serial.println(F("  🔧 diagnostics - Hardware testing tool"));
  Serial.println(F("  ⚡ presets     - Quick configurations (5 options)"));
  Serial.println(F("  💡 Smart hints - Contextual suggestions after commands"));
  Serial.println(F(""));
  Serial.println(F("═══════════════════════════════════════════════════════════"));
  Serial.println(F("              QUICK CONFIGURATION EXAMPLES"));
  Serial.println(F("═══════════════════════════════════════════════════════════"));
  Serial.println(F(""));
  Serial.println(F("1. FIRST TIME SETUP"));
  Serial.println(F("   Command: wizard"));
  Serial.println(F("   Guides you through all basic settings step-by-step."));
  Serial.println(F(""));
  Serial.println(F("2. TEST YOUR HARDWARE"));
  Serial.println(F("   Command: diagnostics"));
  Serial.println(F("   Checks all displays and PSI strips automatically."));
  Serial.println(F(""));
  Serial.println(F("3. APPLY A QUICK PRESET"));
  Serial.println(F("   Command: presets       (see all options)"));
  Serial.println(F("   Command: preset 3      (applies KT mode)"));
  Serial.println(F(""));
  Serial.println(F("4. SWITCH TO DIGITAL PSI"));
  Serial.println(F("   Command: set psi_output 1"));
  Serial.println(F("   Then:    colors        (see available colors)"));
  Serial.println(F("   Then:    set psi1_color1 11  (white)"));
  Serial.println(F(""));
  Serial.println(F("5. ADJUST BRIGHTNESS"));
  Serial.println(F("   Command: set fld_bright 10"));
  Serial.println(F("   Command: set rld_bright 10"));
  Serial.println(F(""));
  Serial.println(F("6. SAVE YOUR CHANGES"));
  Serial.println(F("   Command: profile save  (works on profiles 3-5)"));
  Serial.println(F(""));
  Serial.println(F("═══════════════════════════════════════════════════════════"));
  Serial.println(F("           JAWALITE SERIAL COMMANDS (9600 baud)"));
  Serial.println(F("═══════════════════════════════════════════════════════════"));
  Serial.println(F(""));
  Serial.println(F("Format: [Address][Command][Value]"));
  Serial.println(F(""));
  Serial.println(F("ADDRESSES:"));
  Serial.println(F("  0 = All displays    4 = Front PSI"));
  Serial.println(F("  1 = Top FLD         5 = Rear PSI"));
  Serial.println(F("  2 = Bottom FLD"));
  Serial.println(F("  3 = Rear LD"));
  Serial.println(F(""));
  Serial.println(F("COMMON COMMANDS:"));
  Serial.println(F("  0T1      - Random mode (all displays)"));
  Serial.println(F("  0T0      - Test mode (all LEDs on)"));
  Serial.println(F("  0T20     - Turn everything off"));
  Serial.println(F("  4S1      - Front PSI random animation"));
  Serial.println(F("  0T6      - Leia effect"));
  Serial.println(F("  1MHELLO  - Display 'HELLO' on top FLD"));
  Serial.println(F(""));
  Serial.println(F("For detailed help on config topics, type:"));
  Serial.println(F("  1 or 'profile'  - Profile management"));
  Serial.println(F("  2 or 'psi'      - PSI configuration"));
  Serial.println(F("  3 or 'display'  - Display settings"));
  Serial.println(F("  4 or 'boot'     - Boot text"));
  Serial.println(F("══════════════════════════════════════════════════════════"));
}

void showSettings() {
  Serial.print(F("\n--- Active Profile: "));
  Serial.print(activeProfileIndex + 1);
  if (activeProfileIndex < 2) {
    Serial.println(F(" (read-only) ---"));
  } else {
    Serial.println(F(" (editable) ---"));
  }
  
  if (hasUnsavedChanges) {
    Serial.println(F("\n⚠ WARNING: You have unsaved changes!"));
  }
  
  Serial.println(F("\n-- GLOBAL SETTINGS --"));
  Serial.print(F("  psi_output:     "));
  Serial.print(globalPsiOutput == PSI_ANALOG ? F("ANALOG") : F("DIGITAL"));
  Serial.println(F(" (0=Analog, 1=Digital)"));
  
  Serial.println(F("\n-- BRIGHTNESS (0-15) --"));
  Serial.print(F("  rld_bright:     ")); Serial.println(currentSettings.rldBrightness);
  Serial.print(F("  rpsi_bright:    ")); Serial.println(currentSettings.rpsiBrightness);
  Serial.print(F("  fld_bright:     ")); Serial.println(currentSettings.fldBrightness);
  Serial.print(F("  fpsi_bright:    ")); Serial.println(currentSettings.fpsiBrightness);
  
  Serial.println(F("\n-- BEHAVIOR & TIMING --"));
  Serial.print(F("  logic_style:    ")); Serial.println(currentSettings.logicRandomStyle);
  Serial.print(F("  logic_delay:    ")); Serial.print(currentSettings.logicUpdateDelay); Serial.println(F(" ms"));
  Serial.print(F("  scroll_speed:   ")); Serial.print(currentSettings.scrollSpeed); Serial.println(F(" ms"));
  Serial.print(F("  psi_wipe_delay: ")); Serial.print(currentSettings.psiWipeDelay); Serial.println(F(" ms"));
  
  Serial.println(F("\n-- PSI STUCK EFFECT --"));
  Serial.print(F("  fpsi_stuck:     ")); Serial.println(currentSettings.fpsiGetsStuck ? F("YES") : F("NO"));
  Serial.print(F("  rpsi_stuck:     ")); Serial.println(currentSettings.rpsiGetsStuck ? F("YES") : F("NO"));
  Serial.print(F("  psi_stuck_time: ")); Serial.print(currentSettings.psiStuckHowLong); Serial.println(F(" ms"));
  Serial.print(F("  psi_stuck_freq: ")); Serial.println(currentSettings.psiStuckHowOften);
  
  Serial.println(F("\n-- DIGITAL PSI CONFIG --"));
  Serial.print(F("  psi_swap:       "));
  Serial.println(currentSettings.digitalPsiWiringSwapped ? F("SWAPPED") : F("NORMAL"));
  
  Serial.print(F("  psi1_color1:    "));
  Serial.print((const __FlashStringHelper*)pgm_read_ptr(&(colorPalette[currentSettings.digitalPsi1_color1_index].name)));
  Serial.print(F(" (")); Serial.print(currentSettings.digitalPsi1_color1_index); Serial.println(F(")"));
  
  Serial.print(F("  psi1_color2:    "));
  Serial.print((const __FlashStringHelper*)pgm_read_ptr(&(colorPalette[currentSettings.digitalPsi1_color2_index].name)));
  Serial.print(F(" (")); Serial.print(currentSettings.digitalPsi1_color2_index); Serial.println(F(")"));
  
  Serial.print(F("  psi2_color1:    "));
  Serial.print((const __FlashStringHelper*)pgm_read_ptr(&(colorPalette[currentSettings.digitalPsi2_color1_index].name)));
  Serial.print(F(" (")); Serial.print(currentSettings.digitalPsi2_color1_index); Serial.println(F(")"));
  
  Serial.print(F("  psi2_color2:    "));
  Serial.print((const __FlashStringHelper*)pgm_read_ptr(&(colorPalette[currentSettings.digitalPsi2_color2_index].name)));
  Serial.print(F(" (")); Serial.print(currentSettings.digitalPsi2_color2_index); Serial.println(F(")"));
  
  Serial.println(F("\n-- BOOT TEXT --"));
  Serial.print(F("  boot_tfld:      '")); Serial.print(currentSettings.TFLDtext); Serial.println(F("'"));
  Serial.print(F("  boot_bfld:      '")); Serial.print(currentSettings.BFLDtext); Serial.println(F("'"));
  Serial.print(F("  boot_rld:       '")); Serial.print(currentSettings.RLDtext); Serial.println(F("'"));
  Serial.println(F("-------------------------------"));
}

// =======================================================================================
// ENHANCED CONFIGURATION COMMAND HANDLER
// =======================================================================================

void handleConfigCommands() {
  esp_task_wdt_reset();
  
  // Continue display updates in config mode
  switch (currentEffect) {
    case EFF_ALARM: alarmDisplay(currentSettings.alarmDuration); break;
    case EFF_MARCH: marchDisplay(currentSettings.marchDuration); break;
    case EFF_LEIA: leiaDisplay(currentSettings.leiaDuration); break;
    case EFF_FAILURE: failureDisplay(currentSettings.failureDuration); break;
    default:
      for (byte disp = 0; disp < 3; disp++) {
        switch (displayState[disp]) {
          case STATE_RANDOM: randomDisplay(disp); break;
          case STATE_TEXT: textDisplay(disp); break;
          case STATE_TEST: testDisplay(disp); break;
          case STATE_OFF: offDisplay(disp); break;
          case STATE_BARGRAPH: bargraphDisplay(disp); break;
        }
      }
      
      if (globalPsiOutput == PSI_ANALOG) {
        if (psiState[0] == PSI_RANDOM) randomFPSI(); else setFPSI(psiState[0]);
        if (psiState[1] == PSI_RANDOM) randomRPSI(); else setRPSI(psiState[1]);
      } else {
        PsiConfig psiCfg = getPsiConfig();
        if (psiState[0] == PSI_RANDOM) randomDigitalPsi(fpsiDigitalState, psiCfg.frontColor1, psiCfg.frontColor2, currentSettings.fpsiGetsStuck);
        else setDigitalPsi(*psiCfg.frontStrip, psiState[0], psiCfg.frontColor1, psiCfg.frontColor2);
        if (psiState[1] == PSI_RANDOM) randomDigitalPsi(rpsiDigitalState, psiCfg.rearColor1, psiCfg.rearColor2, currentSettings.rpsiGetsStuck);
        else setDigitalPsi(*psiCfg.rearStrip, psiState[1], psiCfg.rearColor1, psiCfg.rearColor2);
      }
  }
  
  if (Serial.available()) {
    char command[128];
    // Read command with carriage return as terminator
    size_t idx = 0;
    unsigned long startTime = millis();
    bool commandReady = false;

    while (millis() - startTime < 100 && !commandReady) {
      if (Serial.available()) {
        char c = Serial.read();
        if (c == '\r' || c == '\n') {
          command[idx] = '\0';
          commandReady = true;
        } else if (idx < sizeof(command) - 1) {
          command[idx++] = c;
        }
      }
    }

    if (!commandReady) {
      command[idx] = '\0';
    }

    trimString(command);
    if (strlen(command) == 0) {
      Serial.print(F("Config> "));
      return;
    }
    Serial.println(command);

    // NEW v4.2: Verbose mode toggle
    if (strcasecmp_safe(command, "verbose on") == 0) {
      verboseMode = true;
      Serial.println(F("✓ Verbose mode ON - Debug output enabled"));
    }
    else if (strcasecmp_safe(command, "verbose off") == 0) {
      verboseMode = false;
      Serial.println(F("✓ Verbose mode OFF"));
    }
    else if (strcasecmp_safe(command, "verbose") == 0) {
      Serial.print(F("Verbose mode is currently "));
      Serial.println(verboseMode ? F("ON") : F("OFF"));
      Serial.println(F("Usage: verbose on | verbose off"));
    }
    // NEW v4.2: Profile shortcuts (p1-p5)
    else if (strcmp(command, "p1") == 0) {
      loadProfile(1);
      showSmartSuggestion("profile_loaded");
    }
    else if (strcmp(command, "p2") == 0) {
      loadProfile(2);
      showSmartSuggestion("profile_loaded");
    }
    else if (strcmp(command, "p3") == 0) {
      loadProfile(3);
      showSmartSuggestion("profile_loaded");
    }
    else if (strcmp(command, "p4") == 0) {
      loadProfile(4);
      showSmartSuggestion("profile_loaded");
    }
    else if (strcmp(command, "p5") == 0) {
      loadProfile(5);
      showSmartSuggestion("profile_loaded");
    }
    // NEW v4.2: Shorthand commands
    else if (strcmp(command, "s") == 0) {
      showSettings();
    }
    else if (strcmp(command, "w") == 0) {
      runSetupWizard();
    }
    else if (strcmp(command, "d") == 0) {
      runDiagnostics();
    }
    else if (strcmp(command, "q") == 0) {
      if (hasUnsavedChanges) {
        showSmartSuggestion("unsaved_changes");
        Serial.println(F("Type 'q' again to discard, or 'profile save' first."));
        hasUnsavedChanges = false;
      } else {
        configMode = false;
        Serial.println(F("\n✓ Exiting config menu. Resuming normal operation."));
        Serial.print(F("> "));
      }
      return;
    }
    // Number shortcuts for help topics
    else if (strcmp(command, "1") == 0 || strcasecmp_safe(command, "profile") == 0) {
      printProfileHelp();
    }
    else if (strcmp(command, "2") == 0 || strcasecmp_safe(command, "psi") == 0) {
      printPsiHelp();
    }
    else if (strcmp(command, "3") == 0 || strcasecmp_safe(command, "display") == 0) {
      printDisplayHelp();
    }
    else if (strcmp(command, "4") == 0 || strcasecmp_safe(command, "boot") == 0) {
      printBootTextHelp();
    }
    // Wizard command
    else if (strcasecmp_safe(command, "wizard") == 0) {
      runSetupWizard();
    }
    // Diagnostics command
    else if (strcasecmp_safe(command, "diagnostics") == 0 || strcasecmp_safe(command, "diag") == 0) {
      runDiagnostics();
    }
    // NEW: Presets commands
    else if (strcasecmp_safe(command, "presets") == 0) {
      showPresets();
    }
    else if (startsWith(command, "preset ")) {
      int presetNum = atoi(command + 7);
      applyPreset(presetNum);
    }
    // Profile commands
    else if (startsWith(command, "profile ")) {
      const char* subCommand = command + 8;
      if (startsWith(subCommand, "load ")) {
        int profileNum = atoi(subCommand + 5);
        if (profileNum >= 1 && profileNum <= NUM_PROFILES) {
          loadProfile(profileNum);
          showSmartSuggestion("profile_loaded");
        } else {
          Serial.println(F("✗ Invalid profile number. Use 1-5."));
        }
      } else if (strcmp(subCommand, "save") == 0) {
        saveCurrentProfile();
        hasUnsavedChanges = false;
      } else if (startsWith(subCommand, "reset ")) {
        int profileNum = atoi(subCommand + 6);
        resetProfile(profileNum);
      } else if (strcmp(subCommand, "show") == 0) {
        Serial.print(F("Active profile: "));
        Serial.print(activeProfileIndex + 1);
        Serial.println(activeProfileIndex < 2 ? F(" (read-only)") : F(" (editable)"));
      } else {
        Serial.println(F("✗ Unknown profile command. Type '1' for help."));
      }
    }
    // Show settings
    else if (strcmp(command, "show") == 0) {
      showSettings();
    }
    // Set parameter (with NEW validation and suggestions)
    else if (startsWith(command, "set ")) {
      parseAndSetSetting(command);
    }
    // Colors list
    else if (strcmp(command, "colors") == 0) {
      Serial.println(F("\n╔══════════════════════════════════════════╗"));
      Serial.println(F("║     DIGITAL PSI COLOR PALETTE            ║"));
      Serial.println(F("╚══════════════════════════════════════════╝"));
      for (int i = 0; i < numColors; i++) {
        Serial.print(F("  "));
        if (i < 10) Serial.print(F(" "));
        Serial.print(i);
        Serial.print(F(": "));
        Serial.println((const __FlashStringHelper*)pgm_read_ptr(&(colorPalette[i].name)));
      }
      Serial.println(F("\nUsage: set psi1_color1 <number>"));
    }
    // Help
    else if (strcmp(command, "help") == 0 || strcmp(command, "?") == 0) {
      printFullHelp();
    }
    // Exit
    else if (strcmp(command, "exit") == 0 || strcmp(command, "quit") == 0) {
      if (hasUnsavedChanges) {
        showSmartSuggestion("unsaved_changes");
        Serial.println(F("Type 'exit' again to discard, or 'profile save' first."));
        hasUnsavedChanges = false;
      } else {
        configMode = false;
        Serial.println(F("\n✓ Exiting config menu. Resuming normal operation."));
        Serial.print(F("> "));
      }
      return;
    }
    // Unknown command
    else {
      Serial.println(F("✗ Unknown command. Type 'help' for all commands."));
    }
    
    Serial.print(F("Config> "));
  }
}

// =======================================================================================
// ENHANCED PARAMETER SETTING WITH SMART SUGGESTIONS
// =======================================================================================

void parseAndSetSetting(const char* cmd) {
  const char* firstSpace = strchr(cmd, ' ');
  if (!firstSpace) {
    Serial.println(F("Invalid format. Use: set <param> <value>"));
    return;
  }
  
  const char* param = firstSpace + 1;
  const char* secondSpace = strchr(param, ' ');
  
  char paramName[32];
  char valueStr[64];
  
  bool isTextParam = (strncmp(param, "boot_", 5) == 0);
  
  if (isTextParam) {
    if (secondSpace) {
      size_t len = secondSpace - param;
      if (len >= sizeof(paramName)) len = sizeof(paramName) - 1;
      strncpy(paramName, param, len);
      paramName[len] = '\0';
      strncpy(valueStr, secondSpace + 1, sizeof(valueStr) - 1);
      valueStr[sizeof(valueStr) - 1] = '\0';
    } else {
      Serial.println(F("Missing value for boot text"));
      return;
    }
  } else {
    if (!secondSpace) {
      Serial.println(F("Invalid format. Missing value."));
      return;
    }
    size_t len = secondSpace - param;
    if (len >= sizeof(paramName)) len = sizeof(paramName) - 1;
    strncpy(paramName, param, len);
    paramName[len] = '\0';
    strncpy(valueStr, secondSpace + 1, sizeof(valueStr) - 1);
    valueStr[sizeof(valueStr) - 1] = '\0';
  }
  
  int val = atoi(valueStr);
  
  // Track if we need to show specific suggestions
  bool isBrightnessParam = false;
  bool isColorParam = false;
  
  // *** GLOBAL PSI OUTPUT MODE ***
  if (strcmp(paramName, "psi_output") == 0) {
    if (!validateParameterWithHints(paramName, val, globalPsiOutput)) return;
    
    globalPsiOutput = (PsiOutputType)val;
    preferences.putUChar("psiOutput", val);
    
    Serial.print(F("\n✓ Global PSI output set to: "));
    Serial.println(val == 0 ? F("ANALOG") : F("DIGITAL"));
    Serial.println(F("(Saved globally, applies to all profiles)"));
    
    showSmartSuggestion(val == 0 ? "psi_output_analog" : "psi_output_digital");
    hasUnsavedChanges = false;
    return;
  }
  
  // BRIGHTNESS SETTINGS (0-15)
  if (strcmp(paramName, "rld_bright") == 0 || strcmp(paramName, "rpsi_bright") == 0 ||
      strcmp(paramName, "fld_bright") == 0 || strcmp(paramName, "fpsi_bright") == 0) {
    int currentVal = 0;
    if (strcmp(paramName, "rld_bright") == 0) currentVal = currentSettings.rldBrightness;
    else if (strcmp(paramName, "rpsi_bright") == 0) currentVal = currentSettings.rpsiBrightness;
    else if (strcmp(paramName, "fld_bright") == 0) currentVal = currentSettings.fldBrightness;
    else if (strcmp(paramName, "fpsi_bright") == 0) currentVal = currentSettings.fpsiBrightness;
    
    if (!validateParameterWithHints(paramName, val, currentVal)) return;
    
    if (strcmp(paramName, "rld_bright") == 0) currentSettings.rldBrightness = val;
    else if (strcmp(paramName, "rpsi_bright") == 0) currentSettings.rpsiBrightness = val;
    else if (strcmp(paramName, "fld_bright") == 0) currentSettings.fldBrightness = val;
    else if (strcmp(paramName, "fpsi_bright") == 0) currentSettings.fpsiBrightness = val;
    
    applyBrightnessSettings();
    isBrightnessParam = true;
  }
  // LOGIC STYLE (0-6)
  else if (strcmp(paramName, "logic_style") == 0) {
    if (!validateParameterWithHints(paramName, val, currentSettings.logicRandomStyle)) return;
    currentSettings.logicRandomStyle = val;
  }
  // DELAY SETTINGS
  else if (strcmp(paramName, "logic_delay") == 0) {
    if (!validateParameterWithHints(paramName, val, currentSettings.logicUpdateDelay)) return;
    currentSettings.logicUpdateDelay = val;
  }
  else if (strcmp(paramName, "scroll_speed") == 0) {
    if (!validateParameterWithHints(paramName, val, currentSettings.scrollSpeed)) return;
    currentSettings.scrollSpeed = val;
  }
  else if (strcmp(paramName, "psi_wipe_delay") == 0) {
    if (!validateParameterWithHints(paramName, val, currentSettings.psiWipeDelay)) return;
    currentSettings.psiWipeDelay = val;
  }
  else if (strcmp(paramName, "psi_stuck_time") == 0) {
    if (!validateParameterWithHints(paramName, val, currentSettings.psiStuckHowLong)) return;
    currentSettings.psiStuckHowLong = val;
  }
  // PSI STUCK FREQUENCY (1-100)
  else if (strcmp(paramName, "psi_stuck_freq") == 0) {
    if (!validateParameterWithHints(paramName, val, currentSettings.psiStuckHowOften)) return;
    currentSettings.psiStuckHowOften = val;
  }
  // BOOLEAN SETTINGS
  else if (strcmp(paramName, "fpsi_stuck") == 0) {
    currentSettings.fpsiGetsStuck = (val != 0);
  }
  else if (strcmp(paramName, "rpsi_stuck") == 0) {
    currentSettings.rpsiGetsStuck = (val != 0);
  }
  else if (strcmp(paramName, "psi_swap") == 0) {
    currentSettings.digitalPsiWiringSwapped = (val != 0);
  }
  // BOOT TEXT SETTINGS
  else if (strcmp(paramName, "boot_tfld") == 0) {
    sanitizeText(valueStr);  // Remove non-printable characters
    strncpy(currentSettings.TFLDtext, valueStr, BOOT_TEXT_SIZE - 1);
    currentSettings.TFLDtext[BOOT_TEXT_SIZE - 1] = '\0';
  }
  else if (strcmp(paramName, "boot_bfld") == 0) {
    sanitizeText(valueStr);  // Remove non-printable characters
    strncpy(currentSettings.BFLDtext, valueStr, BOOT_TEXT_SIZE - 1);
    currentSettings.BFLDtext[BOOT_TEXT_SIZE - 1] = '\0';
  }
  else if (strcmp(paramName, "boot_rld") == 0) {
    sanitizeText(valueStr);  // Remove non-printable characters
    strncpy(currentSettings.RLDtext, valueStr, BOOT_TEXT_SIZE - 1);
    currentSettings.RLDtext[BOOT_TEXT_SIZE - 1] = '\0';
  }
  // COLOR INDEX SETTINGS (0-11)
  else if (strcmp(paramName, "psi1_color1") == 0 || strcmp(paramName, "psi1_color2") == 0 ||
           strcmp(paramName, "psi2_color1") == 0 || strcmp(paramName, "psi2_color2") == 0) {
    int currentVal = 0;
    if (strcmp(paramName, "psi1_color1") == 0) currentVal = currentSettings.digitalPsi1_color1_index;
    else if (strcmp(paramName, "psi1_color2") == 0) currentVal = currentSettings.digitalPsi1_color2_index;
    else if (strcmp(paramName, "psi2_color1") == 0) currentVal = currentSettings.digitalPsi2_color1_index;
    else if (strcmp(paramName, "psi2_color2") == 0) currentVal = currentSettings.digitalPsi2_color2_index;
    
    if (!validateParameterWithHints(paramName, val, currentVal)) return;
    
    if (strcmp(paramName, "psi1_color1") == 0) currentSettings.digitalPsi1_color1_index = val;
    else if (strcmp(paramName, "psi1_color2") == 0) currentSettings.digitalPsi1_color2_index = val;
    else if (strcmp(paramName, "psi2_color1") == 0) currentSettings.digitalPsi2_color1_index = val;
    else if (strcmp(paramName, "psi2_color2") == 0) currentSettings.digitalPsi2_color2_index = val;
    
    isColorParam = true;
  }
  else {
    Serial.print(F("Unknown parameter: "));
    Serial.println(paramName);
    return;
  }
  
  Serial.print(F("\n✓ Set '"));
  Serial.print(paramName);
  Serial.print(F("' to '"));
  Serial.print(valueStr);
  Serial.println(F("'"));
  
  hasUnsavedChanges = true;
  
  // Show context-appropriate smart suggestions
  if (isBrightnessParam) {
    showSmartSuggestion("brightness_changed");
  } else if (isColorParam) {
    showSmartSuggestion("color_changed");
  }
  
  if (activeProfileIndex >= 2) {
    Serial.println(F("ℹ Use 'profile save' to keep this change."));
  } else {
    Serial.println(F("ℹ This profile is read-only. Load profile 3-5 to save changes."));
  }
}