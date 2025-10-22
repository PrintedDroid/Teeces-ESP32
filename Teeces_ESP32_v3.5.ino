/**
 * ===============================================================================================
 * Printed Droid Teeces Logic Display Controller - ESP32 Enhanced Version 4.0
 * ===============================================================================================
 *
 * This document summarizes the complete features of the enhanced ESP32-based controller for
 * R2-D2 style Logic Displays (FLD, RLD) and Process State Indicators (PSI).
 * The code has evolved significantly from its base version to include advanced configuration,
 * stability features, and expanded hardware support.
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
 * - exit: Exits the configuration menu and resumes normal operation.
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

// *** GLOBAL PSI OUTPUT MODE (not per-profile!) ***
PsiOutputType globalPsiOutput = PSI_DIGITAL;  // Default to digital

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

// Forward Declarations
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
// =======================================================================================
// SETUP
// =======================================================================================

void setup() {
  Serial.begin(9600);
  while (!Serial && millis() < 3000) delay(10);
  
  if (!preferences.begin("teeces-config", false)) {
    Serial.println(F("ERROR: Failed to initialize flash storage!"));
    while(1) delay(1000);
  }

  Serial.println(F("--- ESP32 Teeces v4.0 (Optimized Edition) ---"));
  Serial.println(F("Major changes: Global PSI mode, memory optimization"));

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
  
  Serial.println(F("Send '*' to enter config menu."));

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
  Serial.println(F("Ready for commands.")); 
  Serial.print(F("> "));

  // Watchdog
  esp_task_wdt_deinit();
  esp_task_wdt_config_t wdt_config = {
    .timeout_ms = WDT_TIMEOUT_MS,
    .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
    .trigger_panic = true
  };
  ESP_ERROR_CHECK(esp_task_wdt_init(&wdt_config));
  ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
  Serial.print(F("Watchdog enabled ("));
  Serial.print(WDT_TIMEOUT_MS / 1000);
  Serial.println(F("s timeout)."));
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

      // PSI handling based on GLOBAL output mode
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
  cfg.digitalPsi1_color1_index = 1; // GREEN
  cfg.digitalPsi1_color2_index = 3; // YELLOW
  cfg.digitalPsi2_color1_index = 0; // RED
  cfg.digitalPsi2_color2_index = 2; // BLUE
  
  if (profileNum == 1) {
    strncpy(cfg.TFLDtext, "PROFILE 1", BOOT_TEXT_SIZE);
    strncpy(cfg.BFLDtext, " STANDARD", BOOT_TEXT_SIZE);
    strncpy(cfg.RLDtext, "BOOTING ASTROMECH   ", BOOT_TEXT_SIZE);
  } else if (profileNum == 2) {
    strncpy(cfg.TFLDtext, "PROFILE 2", BOOT_TEXT_SIZE);
    strncpy(cfg.BFLDtext, "  CUSTOM ", BOOT_TEXT_SIZE);
    strncpy(cfg.RLDtext, "WHITE PINK MODE     ", BOOT_TEXT_SIZE);
    // Both PSI strips: White/Pink
    cfg.digitalPsi1_color1_index = 11; // WHITE
    cfg.digitalPsi1_color2_index = 8;  // PINK
    cfg.digitalPsi2_color1_index = 11; // WHITE
    cfg.digitalPsi2_color2_index = 8;  // PINK
  } else {
    snprintf(cfg.TFLDtext, BOOT_TEXT_SIZE, "PROFILE %d", profileNum);
    strncpy(cfg.BFLDtext, "   USER  ", BOOT_TEXT_SIZE);
    strncpy(cfg.RLDtext, "USER PROFILE        ", BOOT_TEXT_SIZE);
  }
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

void showGrid(byte display) {
  if (!isValidDisplayIndex(display)) return;
  unsigned char col8 = 0, col17 = 0, col26 = 0;
  switch (display) {
    case 0:
      for (byte row = 0; row < 5; row++) {
        lcFront.setRow(0, row, rev(LEDgrid[display][row] & 255L));
        if ((LEDgrid[display][row] & (1L << 8)) == (1L << 8))
          col8 += 128 >> row;
      }
      lcFront.setRow(0, 5, col8);
      break;
    case 1:
      for (byte row = 0; row < 5; row++) {
        lcFront.setRow(1, 4 - row, (LEDgrid[display][row] & (255L << 1)) >> 1);
        if ((LEDgrid[display][row] & 1L) == 1L)
          col8 += 8 << row;
      }
      lcFront.setRow(1, 5, col8);
      break;
    case 2:
      for (byte row = 0; row < 5; row++) {
        for (byte dev = 0; dev < 3; dev++) {
          lcRear.setRow(dev, row, rev((LEDgrid[display][row] & (255L << (9 * dev))) >> (9 * dev)));
        }
        if ((LEDgrid[display][row] & (1L << 8)) == (1L << 8)) col8 += 128 >> row;
        if ((LEDgrid[display][row] & (1L << 17)) == (1L << 17)) col17 += 128 >> row;
        if ((LEDgrid[display][row] & (1L << 26)) == (1L << 26)) col26 += 128 >> row;
      }
      lcRear.setRow(0, 5, col8);
      lcRear.setRow(1, 5, col17);
      lcRear.setRow(2, 5, col26);
      break;
  }
}

uint8_t rev(uint8_t n) {
  return (pgm_read_byte(&revlookup[n & 0x0F]) << 4) | pgm_read_byte(&revlookup[n >> 4]);
}

void setText(byte disp, const char* message) {
  if (!isValidDisplayIndex(disp)) return;
  strncpy(logicText[disp], message, MAXSTRINGSIZE);
  logicText[disp][MAXSTRINGSIZE] = 0;
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
  
  // PSI animation using GLOBAL mode
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
      
      // PSI using GLOBAL mode
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
      
      // PSI using GLOBAL mode
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
  
  // PSI using GLOBAL mode
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
      Serial.write(0x7);  // Beep on buffer overflow
    }
  }
  return false;
}

void parseCommand(char* inputStr) {
  byte hasArgument = false;
  int argument, address;
  byte pos = 0;
  byte length = strlen(inputStr);
  
  if (length < 2) goto beep;
  
  char addrStr[3];
  if (!isdigit(inputStr[pos])) goto beep;
  addrStr[pos] = inputStr[pos];
  pos++;
  
  if (isdigit(inputStr[pos])) {
    addrStr[pos] = inputStr[pos];
    pos++;
  }
  addrStr[pos] = '\0';
  address = atoi(addrStr);
  
  if (pos >= length) goto beep;
  
  if (inputStr[pos] == 'M') {
    pos++;
    if (pos >= length) goto beep;
    doMcommand(address, inputStr + pos);
    return;
  }
  
  pos++;
  if (pos >= length) {
    hasArgument = false;
  } else {
    for (byte i = pos; i < length; i++) {
      if (!isdigit(inputStr[i])) goto beep;
    }
    argument = atoi(inputStr + pos);
    hasArgument = true;
  }
  
  switch (inputStr[pos - 1]) {
    case 'T':
      if (hasArgument) doTcommand(address, argument);
      else goto beep;
      break;
    case 'D':
      doDcommand(address);
      break;
    case 'P':
      if (hasArgument) doPcommand(address, argument);
      else goto beep;
      break;
    case 'R':
      if (hasArgument) doRcommand(address, argument);
      else goto beep;
      break;
    case 'S':
      if (hasArgument) doScommand(address, argument);
      else goto beep;
      break;
    default:
      goto beep;
  }
  return;
  
beep:
  Serial.write(0x7);
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
    case 0:  // Test mode
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
      
    case 1:  // Random mode
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
      
    case 2:  // Alarm effect
    case 3:
    case 5:
      exitEffects();
      currentEffect = EFF_ALARM;
      break;
      
    case 4:  // Failure effect
      exitEffects();
      currentEffect = EFF_FAILURE;
      break;
      
    case 6:  // Leia effect
      exitEffects();
      currentEffect = EFF_LEIA;
      break;
      
    case 10:  // Star Wars text
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
      
    case 11:  // March effect
      exitEffects();
      currentEffect = EFF_MARCH;
      break;
      
    case 20:  // Off
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
      
    case 92:  // Bargraph mode
      exitEffects();
      if (address == 0) {
        displayState[0] = displayState[1] = displayState[2] = STATE_BARGRAPH;
        resetAllText();
      }
      if (address == 1) { displayState[0] = STATE_BARGRAPH; resetText(0); }
      if (address == 2) { displayState[1] = STATE_BARGRAPH; resetText(1); }
      if (address == 3) { displayState[2] = STATE_BARGRAPH; resetText(2); }
      break;
      
    case 100:  // Text mode
      exitEffects();
      if (address == 0) {
        displayState[0] = displayState[1] = displayState[2] = STATE_TEXT;
        resetAllText();
      }
      if (address == 1) { displayState[0] = STATE_TEXT; resetText(0); }
      if (address == 2) { displayState[1] = STATE_TEXT; resetText(1); }
      if (address == 3) { displayState[2] = STATE_TEXT; resetText(2); }
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
  
  if (argument == 60) {  // Latin alphabet
    if (address == 0) {
      alphabetType[0] = alphabetType[1] = alphabetType[2] = ALPHA_LATIN;
    }
    if (address == 1) alphabetType[0] = ALPHA_LATIN;
    if (address == 2) alphabetType[1] = ALPHA_LATIN;
    if (address == 3) alphabetType[2] = ALPHA_LATIN;
  } else if (argument == 61) {  // Aurabesh alphabet
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
// IMPROVED CONFIGURATION MENU
// =======================================================================================

void printConfigMenu() {
  Serial.println(F("\n"));
  Serial.println(F("╔════════════════════════════════════════════════════════════╗"));
  Serial.println(F("║         TEECES v4.0 - CONFIGURATION MENU                  ║"));
  Serial.println(F("╚════════════════════════════════════════════════════════════╝"));
  Serial.println(F(""));
  Serial.println(F("Quick Commands:"));
  Serial.println(F("  help     - Show detailed help and examples"));
  Serial.println(F("  show     - Display all current settings"));
  Serial.println(F("  colors   - List available PSI colors (0-11)"));
  Serial.println(F("  exit     - Return to normal operation"));
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
  Serial.println(F("  2 - WHITE/PINK   (read-only, digital PSI demo)"));
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
  Serial.println(F("════════════════════════════════════════════════════════════"));
  Serial.println(F("                    QUICK START GUIDE"));
  Serial.println(F("════════════════════════════════════════════════════════════"));
  Serial.println(F(""));
  Serial.println(F("1. VIEW CURRENT SETTINGS"));
  Serial.println(F("   Command: show"));
  Serial.println(F(""));
  Serial.println(F("2. SWITCH PSI TO DIGITAL"));
  Serial.println(F("   Command: set psi_output 1"));
  Serial.println(F("   Then:    colors           (see color options)"));
  Serial.println(F("   Then:    set psi1_color1 11  (white)"));
  Serial.println(F(""));
  Serial.println(F("3. LOAD A PROFILE"));
  Serial.println(F("   Command: profile load 2   (loads white/pink demo)"));
  Serial.println(F(""));
  Serial.println(F("4. ADJUST BRIGHTNESS"));
  Serial.println(F("   Command: set fld_bright 8"));
  Serial.println(F(""));
  Serial.println(F("5. SAVE YOUR CHANGES"));
  Serial.println(F("   Command: profile save     (only works for profiles 3-5)"));
  Serial.println(F(""));
  Serial.println(F("════════════════════════════════════════════════════════════"));
  Serial.println(F("           JAWALITE SERIAL COMMANDS (9600 baud)"));
  Serial.println(F("════════════════════════════════════════════════════════════"));
  Serial.println(F(""));
  Serial.println(F("Format: [Address][Command][Value]"));
  Serial.println(F(""));
  Serial.println(F("ADDRESSES:"));
  Serial.println(F("  0 = All displays    4 = Front PSI"));
  Serial.println(F("  1 = Top FLD         5 = Rear PSI"));
  Serial.println(F("  2 = Bottom FLD"));
  Serial.println(F("  3 = Rear LD"));
  Serial.println(F(""));
  Serial.println(F("DISPLAY COMMANDS (T):"));
  Serial.println(F("  1T1      - Random mode on Top FLD"));
  Serial.println(F("  0T0      - Test mode (all on) on all displays"));
  Serial.println(F("  0T20     - Turn all off"));
  Serial.println(F("  0T6      - Leia effect"));
  Serial.println(F("  0T11     - March effect"));
  Serial.println(F(""));
  Serial.println(F("PSI COMMANDS (S):"));
  Serial.println(F("  4S1      - Front PSI random"));
  Serial.println(F("  5S2      - Rear PSI color 1"));
  Serial.println(F("  0S4      - All PSI off"));
  Serial.println(F(""));
  Serial.println(F("TEXT COMMANDS (M):"));
  Serial.println(F("  1MHELLO  - Show 'HELLO' on Top FLD"));
  Serial.println(F(""));
  Serial.println(F("For detailed help on config topics, type:"));
  Serial.println(F("  1 or 'profile'  - Profile management"));
  Serial.println(F("  2 or 'psi'      - PSI configuration"));
  Serial.println(F("  3 or 'display'  - Display settings"));
  Serial.println(F("  4 or 'boot'     - Boot text"));
  Serial.println(F("════════════════════════════════════════════════════════════"));
}

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
    String command = Serial.readStringUntil('\r');
    command.trim();
    if (command.length() == 0) {
      Serial.print(F("Config> "));
      return;
    }
    Serial.println(command);
    
    // Number shortcuts for help topics
    if (command == "1" || command.equalsIgnoreCase("profile")) {
      printProfileHelp();
    }
    else if (command == "2" || command.equalsIgnoreCase("psi")) {
      printPsiHelp();
    }
    else if (command == "3" || command.equalsIgnoreCase("display")) {
      printDisplayHelp();
    }
    else if (command == "4" || command.equalsIgnoreCase("boot")) {
      printBootTextHelp();
    }
    // Profile commands
    else if (command.startsWith("profile ")) {
      String subCommand = command.substring(8);
      if (subCommand.startsWith("load ")) {
        int profileNum = subCommand.substring(5).toInt();
        if (profileNum >= 1 && profileNum <= NUM_PROFILES) {
          loadProfile(profileNum);
          Serial.println(F("✓ Profile loaded. Type 'show' to see settings."));
        } else {
          Serial.println(F("✗ Invalid profile number. Use 1-5."));
        }
      } else if (subCommand == "save") {
        saveCurrentProfile();
      } else if (subCommand.startsWith("reset ")) {
        int profileNum = subCommand.substring(6).toInt();
        resetProfile(profileNum);
      } else if (subCommand == "show") {
        Serial.print(F("Active profile: "));
        Serial.print(activeProfileIndex + 1);
        Serial.println(activeProfileIndex < 2 ? F(" (read-only)") : F(" (editable)"));
      } else {
        Serial.println(F("✗ Unknown profile command. Type '1' for help."));
      }
    }
    // Show settings
    else if (command == "show") {
      showSettings();
    }
    // Set parameter
    else if (command.startsWith("set ")) {
      parseAndSetSetting(command.c_str());
    }
    // Colors list
    else if (command == "colors") {
      Serial.println(F("\n╔═══════════════════════════════════════╗"));
      Serial.println(F("║     DIGITAL PSI COLOR PALETTE         ║"));
      Serial.println(F("╚═══════════════════════════════════════╝"));
      for (int i = 0; i < numColors; i++) {
        Serial.print(F("  "));
        if (i < 10) Serial.print(F(" "));
        Serial.print(i);
        Serial.print(F(": "));
        Serial.println((const __FlashStringHelper*)pgm_read_ptr(&(colorPalette[i].name)));
      }
      Serial.println(F(""));
      Serial.println(F("Usage: set psi1_color1 <number>"));
    }
    // Help
    else if (command == "help" || command == "?") {
      printFullHelp();
    }
    // Exit
    else if (command == "exit" || command == "quit") {
      configMode = false;
      Serial.println(F("\n✓ Exiting config menu. Resuming normal operation."));
      Serial.print(F("> "));
      return;
    }
    // Unknown command
    else {
      Serial.println(F("✗ Unknown command. Type 'help' for all commands."));
    }
    
    Serial.print(F("Config> "));
  }
}

void showSettings() {
  Serial.print(F("\n--- Active Profile: "));
  Serial.print(activeProfileIndex + 1);
  Serial.println(F(" ---"));
  
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
  
  // *** GLOBAL PSI OUTPUT MODE ***
  if (strcmp(paramName, "psi_output") == 0) {
    if (val < 0 || val > 1) {
      Serial.println(F("Error: psi_output must be 0 (Analog) or 1 (Digital)"));
      return;
    }
    globalPsiOutput = (PsiOutputType)val;
    preferences.putUChar("psiOutput", val);
    Serial.print(F("Global PSI output set to: "));
    Serial.println(val == 0 ? F("ANALOG") : F("DIGITAL"));
    Serial.println(F("(Saved globally, applies to all profiles)"));
    return;
  }
  
  // BRIGHTNESS SETTINGS (0-15)
  if (strcmp(paramName, "rld_bright") == 0 || strcmp(paramName, "rpsi_bright") == 0 ||
      strcmp(paramName, "fld_bright") == 0 || strcmp(paramName, "fpsi_bright") == 0) {
    if (val < 0 || val > 15) {
      Serial.println(F("Error: Brightness must be 0-15"));
      return;
    }
    if (strcmp(paramName, "rld_bright") == 0) currentSettings.rldBrightness = val;
    else if (strcmp(paramName, "rpsi_bright") == 0) currentSettings.rpsiBrightness = val;
    else if (strcmp(paramName, "fld_bright") == 0) currentSettings.fldBrightness = val;
    else if (strcmp(paramName, "fpsi_bright") == 0) currentSettings.fpsiBrightness = val;
    applyBrightnessSettings();
  }
  // LOGIC STYLE (0-6)
  else if (strcmp(paramName, "logic_style") == 0) {
    if (val < 0 || val > 6) {
      Serial.println(F("Error: Style must be 0-6"));
      return;
    }
    currentSettings.logicRandomStyle = val;
  }
  // DELAY SETTINGS
  else if (strcmp(paramName, "logic_delay") == 0) {
    if (val < 1 || val > 60000) {
      Serial.println(F("Error: Delay must be 1-60000 ms"));
      return;
    }
    currentSettings.logicUpdateDelay = val;
  }
  else if (strcmp(paramName, "scroll_speed") == 0) {
    if (val < 1 || val > 60000) {
      Serial.println(F("Error: Speed must be 1-60000 ms"));
      return;
    }
    currentSettings.scrollSpeed = val;
  }
  else if (strcmp(paramName, "psi_wipe_delay") == 0) {
    if (val < 1 || val > 1000) {
      Serial.println(F("Error: Delay must be 1-1000 ms"));
      return;
    }
    currentSettings.psiWipeDelay = val;
  }
  else if (strcmp(paramName, "psi_stuck_time") == 0) {
    if (val < 100 || val > 60000) {
      Serial.println(F("Error: Time must be 100-60000 ms"));
      return;
    }
    currentSettings.psiStuckHowLong = val;
  }
  // PSI STUCK FREQUENCY (1-100)
  else if (strcmp(paramName, "psi_stuck_freq") == 0) {
    if (val < 1 || val > 100) {
      Serial.println(F("Error: Frequency must be 1-100"));
      return;
    }
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
    strncpy(currentSettings.TFLDtext, valueStr, BOOT_TEXT_SIZE - 1);
    currentSettings.TFLDtext[BOOT_TEXT_SIZE - 1] = '\0';
  }
  else if (strcmp(paramName, "boot_bfld") == 0) {
    strncpy(currentSettings.BFLDtext, valueStr, BOOT_TEXT_SIZE - 1);
    currentSettings.BFLDtext[BOOT_TEXT_SIZE - 1] = '\0';
  }
  else if (strcmp(paramName, "boot_rld") == 0) {
    strncpy(currentSettings.RLDtext, valueStr, BOOT_TEXT_SIZE - 1);
    currentSettings.RLDtext[BOOT_TEXT_SIZE - 1] = '\0';
  }
  // COLOR INDEX SETTINGS (0-11)
  else if (strcmp(paramName, "psi1_color1") == 0 || strcmp(paramName, "psi1_color2") == 0 ||
           strcmp(paramName, "psi2_color1") == 0 || strcmp(paramName, "psi2_color2") == 0) {
    if (!isValidColorIndex(val)) {
      Serial.print(F("Error: Color index must be 0-"));
      Serial.println(numColors - 1);
      Serial.println(F("Use 'colors' command to see valid indices"));
      return;
    }
    if (strcmp(paramName, "psi1_color1") == 0) currentSettings.digitalPsi1_color1_index = val;
    else if (strcmp(paramName, "psi1_color2") == 0) currentSettings.digitalPsi1_color2_index = val;
    else if (strcmp(paramName, "psi2_color1") == 0) currentSettings.digitalPsi2_color1_index = val;
    else if (strcmp(paramName, "psi2_color2") == 0) currentSettings.digitalPsi2_color2_index = val;
  }
  else {
    Serial.print(F("Unknown parameter: "));
    Serial.println(paramName);
    return;
  }
  
  Serial.print(F("Set '"));
  Serial.print(paramName);
  Serial.print(F("' to '"));
  Serial.print(valueStr);
  Serial.println(F("'"));
  Serial.println(F("(Use 'profile save' to persist changes)"));
}

#pragma GCC diagnostic pop