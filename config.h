// Night Light Configuration File
// This file contains all configurable parameters for the night light project
// Modify these values to customize your night light behavior

#ifndef CONFIG_H
#define CONFIG_H

// ========== HARDWARE CONFIGURATION ==========
// Pin assignments for ATTiny85 (adjust based on your wiring)
#define NEOPIXEL_PIN    0   // PB0 - Data pin for NeoPixels
#define POTENTIOMETER_PIN A1 // PB2 - Analog input for brightness control
#define BUTTON_MODE_PIN 1   // PB1 - Button for cycling through modes
#define BUTTON_COLOR_PIN 3  // PB3 - Button for cycling through colors

// NeoPixel array configuration
#define NUM_ROWS        4   // Number of rows in the rectangular array
#define NUM_COLS        2   // Number of columns in the rectangular array
#define ORIENTATION     0   // 0 = row-major (left to right, top to bottom)
                          // 1 = column-major (top to bottom, left to right)

// Total number of NeoPixels (calculated automatically)
#define NUM_PIXELS      (NUM_ROWS * NUM_COLS)

// ========== BRIGHTNESS AND POWER CONFIGURATION ==========
#define MAX_BRIGHTNESS  64   // Maximum brightness level (0-255)
#define MIN_BRIGHTNESS  5    // Minimum brightness level (0-255)
#define DEFAULT_BRIGHTNESS 32 // Default brightness when potentiometer is centered

// Off/On thresholds with hysteresis - prevents rapid switching at threshold boundary
// OFF_THRESHOLD: When potentiometer goes below this value, enter sleep mode
// ON_THRESHOLD: When potentiometer goes above this value, wake from sleep mode
// Set OFF_THRESHOLD < ON_THRESHOLD to create hysteresis (dead zone)
// Example: OFF at 40, ON at 60 creates 20-unit dead zone
// To find values: enable debug mode, test min/max positions, set thresholds accordingly
#define OFF_THRESHOLD 40  // Below this = OFF (sleep mode) - adjust experimentally
#define ON_THRESHOLD  60  // Above this = ON (active mode) - adjust experimentally

// Sleep timer configuration (in milliseconds)
#define SLEEP_TIMEOUT   30000L // 30 seconds (30,000 ms) - time before sleep
#define SLEEP_FADE_TIME 2000L   // 2 seconds - time to fade out before sleep

// ========== MODE CONFIGURATION ==========
#define NUM_MODES       4     // Total number of available modes

// Mode indices (don't change these)
#define MODE_CANDLELIGHT    0
#define MODE_TWINKLE        1
#define MODE_INDEPENDENT_PULSE 2
#define MODE_SYNC_PULSE      3

// Default starting mode
#define DEFAULT_MODE    MODE_CANDLELIGHT

// Enable/disable modes (set to 1 to enable, 0 to disable)
#define ENABLE_CANDLELIGHT      1
#define ENABLE_TWINKLE          1
#define ENABLE_INDEPENDENT_PULSE 1
#define ENABLE_SYNC_PULSE       1

// ========== COLOR SCHEME CONFIGURATION ==========

// Color definitions (RGB values)
#define COLOR_RED       255, 0, 0
#define COLOR_ORANGE    255, 165, 0
#define COLOR_YELLOW    255, 255, 0
#define COLOR_GREEN     0, 255, 0
#define COLOR_BLUE      0, 0, 255
#define COLOR_PURPLE    128, 0, 128
#define COLOR_WHITE     255, 255, 255
#define COLOR_BLACK     0, 0, 0
#define COLOR_LIGHT_BLUE 173, 216, 230

// Candlelight mode color schemes
#define CANDLELIGHT_SCHEMES 5
#define CANDLELIGHT_DEFAULT 0

// Twinkle mode color schemes
#define TWINKLE_SCHEMES 8
#define TWINKLE_DEFAULT 0

// Independent pulse mode colors
#define INDEPENDENT_PULSE_COLORS 6
#define INDEPENDENT_PULSE_DEFAULT 0

// Sync pulse mode variations
#define SYNC_PULSE_VARIATIONS 7
#define SYNC_PULSE_DEFAULT 0

// ========== PULSE AND ANIMATION CONFIGURATION ==========

// Independent pulse mode
#define PULSE_SPEED_MIN     50    // Minimum pulse speed (milliseconds)
#define PULSE_SPEED_MAX     500   // Maximum pulse speed (milliseconds)
#define PULSE_SPEED_DEFAULT 200   // Default pulse speed

// Twinkle mode
#define TWINKLE_FADE_SPEED 50    // Speed of twinkle fading (lower = faster)
#define TWINKLE_DENSITY    0.3   // Fraction of pixels that twinkle at once (0.0-1.0)

// Candlelight mode
#define CANDLE_FLICKER_SPEED 100 // Speed of candle flicker animation

// Sync pulse mode
#define SYNC_PULSE_DURATION 1000  // Duration of each pulse cycle (milliseconds)

// ========== LIGHTNESS AND BRIGHTNESS SPECTRA ==========

// Lightness ranges for different modes (min, max brightness multipliers)
#define LIGHTNESS_LOW_MIN    0.1
#define LIGHTNESS_LOW_MAX    0.4
#define LIGHTNESS_MED_MIN    0.3
#define LIGHTNESS_MED_MAX    0.7
#define LIGHTNESS_HIGH_MIN   0.6
#define LIGHTNESS_HIGH_MAX   1.0

// Brightness spectrum ranges
#define BRIGHTNESS_LOW_MIN   0.1
#define BRIGHTNESS_LOW_MAX   0.3
#define BRIGHTNESS_MED_MIN   0.4
#define BRIGHTNESS_MED_MAX   0.7
#define BRIGHTNESS_HIGH_MIN  0.8
#define BRIGHTNESS_HIGH_MAX  1.0

// ========== ADVANCED FEATURES ==========

// ========== DEBUG CONFIGURATION ==========

// Enable debug output methods (can enable multiple)
// Note: Visual debug uses first 3 NeoPixels. Serial debug requires SoftwareSerial on PB4.
#define ENABLE_VISUAL_DEBUG     0   // Use NeoPixels for visual debugging (recommended for ATTiny85)
#define ENABLE_SERIAL_DEBUG     0   // Use SoftwareSerial on PB4 (requires serial adapter)

// Debug mode selection (for visual debug)
#define DEBUG_MODE_NORMAL       0     // Normal debug display (current status)
#define DEBUG_MODE_THRESHOLD    1     // Threshold testing mode
#define DEBUG_MODE              DEBUG_MODE_NORMAL  // Select debug mode when visual debug enabled

// ========== VISUAL DEBUG COLOR CODES ==========
// Debug uses first 3 NeoPixels as status indicators
// Format: {Pixel0_R, Pixel0_G, Pixel0_B, Pixel1_R, Pixel1_G, Pixel1_B, Pixel2_R, Pixel2_G, Pixel2_B}
// NOTE: When ENABLE_VISUAL_DEBUG=1, lighting modes skip pixels 0-2 to avoid conflicts

// Initialization states (shown during startup)
// Three-stage initialization with progress indicators
#define DEBUG_INIT_STAGE1_IN_PROGRESS {255, 255, 0, 0, 0, 0, 0, 0, 0}   // Yellow blink - hardware init in progress
#define DEBUG_INIT_STAGE1_SUCCESS     {0, 255, 0, 0, 0, 0, 0, 0, 0}     // Green solid - hardware init complete

#define DEBUG_INIT_STAGE2_IN_PROGRESS {0, 0, 0, 255, 255, 0, 0, 0, 0}   // Yellow blink - NeoPixel init in progress
#define DEBUG_INIT_STAGE2_SUCCESS     {0, 0, 0, 0, 255, 0, 0, 0, 0}     // Green solid - NeoPixel init complete

#define DEBUG_INIT_STAGE3_IN_PROGRESS {0, 0, 0, 0, 0, 0, 255, 255, 0}   // Yellow blink - random seed init in progress
#define DEBUG_INIT_STAGE3_SUCCESS     {0, 0, 0, 0, 0, 0, 0, 255, 0}     // Green solid - random seed init complete (provides different animation patterns each power-on)

#define DEBUG_INIT_COMPLETE           {0, 255, 0, 0, 255, 0, 0, 255, 0} // All green blink 3x - init complete

// Runtime debug codes (Pixel 0: Mode, Pixel 1: Status, Pixel 2: Buttons)
// Pixel 0 - Current Mode (solid colors)
#define DEBUG_MODE_CANDLE       {255, 165, 0, 0, 0, 0, 0, 0, 0}   // Orange solid - Candlelight mode
#define DEBUG_MODE_TWINKLE      {255, 255, 255, 0, 0, 0, 0, 0, 0} // White solid - Twinkle mode
#define DEBUG_MODE_PULSE_I      {0, 0, 255, 0, 0, 0, 0, 0, 0}     // Blue solid - Independent pulse mode
#define DEBUG_MODE_PULSE_S      {255, 0, 255, 0, 0, 0, 0, 0, 0}   // Magenta solid - Sync pulse mode

// Pixel 1 - Status (active/sleep/manual-off)
#define DEBUG_STATUS_ACTIVE     {0, 0, 0, 0, 255, 0, 0, 0, 0}     // Green solid - active/on
#define DEBUG_STATUS_SLEEPING   {0, 0, 0, 128, 0, 128, 0, 0, 0}   // Purple solid - auto sleep
#define DEBUG_STATUS_MANUAL_OFF {0, 0, 0, 255, 0, 0, 0, 0, 0}     // Red solid - manual off

// Pixel 2 - Button presses (brief flashes)
#define DEBUG_BUTTON_MODE       {0, 0, 0, 0, 0, 0, 255, 255, 0}   // Yellow flash - mode button pressed
#define DEBUG_BUTTON_COLOR      {0, 0, 0, 0, 0, 0, 0, 255, 255}   // Cyan flash - color button pressed

// Threshold testing colors (shown continuously during threshold testing)
#define DEBUG_THRESHOLD_OFF     {255, 0, 0, 0, 0, 0, 0, 0, 0}     // Red - below OFF threshold
#define DEBUG_THRESHOLD_DEAD    {255, 255, 0, 0, 0, 0, 0, 0, 0}   // Yellow - in dead zone
#define DEBUG_THRESHOLD_ON      {0, 255, 0, 0, 0, 0, 0, 0, 0}     // Green - above ON threshold

// Button debouncing
#define DEBOUNCE_DELAY      50    // Milliseconds for button debouncing

// Random seed for animations (set to 0 for truly random, or fixed value for repeatable patterns)
#define RANDOM_SEED         0

// ========== UTILITY MACROS ==========

// Helper macros for color manipulation
#define DIM_COLOR(r, g, b, brightness) \
    ((uint8_t)((r) * (brightness) / 255)), \
    ((uint8_t)((g) * (brightness) / 255)), \
    ((uint8_t)((b) * (brightness) / 255))

#define MIX_COLORS(r1, g1, b1, r2, g2, b2, ratio) \
    ((uint8_t)((r1) * (1.0f - ratio) + (r2) * ratio)), \
    ((uint8_t)((g1) * (1.0f - ratio) + (g2) * ratio)), \
    ((uint8_t)((b1) * (1.0f - ratio) + (b2) * ratio))

#endif // CONFIG_H
