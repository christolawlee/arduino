// Calming Night Light for ATTiny85
// Features: Multiple lighting modes, brightness control, sleep timer, button controls
//
// Hardware Requirements:
// - ATTiny85 microcontroller
// - NeoPixel strip/array (configurable size)
// - 10k linear potentiometer for brightness
// - 2 push buttons (mode cycling, color cycling)
// - Power supply (appropriate for NeoPixels)
//
// Programming: Use Arduino as ISP to program ATTiny85

#include <Adafruit_NeoPixel.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#if ENABLE_SERIAL_DEBUG
#include <SoftwareSerial.h>
// Debug serial on PB4 (TX only - connect to serial adapter RX)
SoftwareSerial debugSerial(-1, 4); // RX=-1 (not used), TX=PB4
#endif

#include "config.h"

// ========== GLOBAL VARIABLES ==========

// NeoPixel object
Adafruit_NeoPixel pixels(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// State variables
uint8_t currentMode = DEFAULT_MODE;
uint8_t currentColorScheme = 0;
uint8_t globalBrightness = DEFAULT_BRIGHTNESS;
bool isSleeping = false;
bool wasPotOff = true; // Track previous potentiometer state

// Debug state variables
#if ENABLE_VISUAL_DEBUG
unsigned long debugLastUpdate = 0;
uint8_t debugInitStep = 0;
bool debugInitComplete = false;
#endif

// Button state tracking
bool lastButtonModeState = HIGH;
bool lastButtonColorState = HIGH;
unsigned long lastButtonModeTime = 0;
unsigned long lastButtonColorTime = 0;

// Sleep timer
unsigned long lastActivityTime = 0;
unsigned long sleepStartTime = 0;

// Animation variables
unsigned long lastUpdateTime = 0;
uint16_t animationCounter = 0;

// Individual pixel states for independent animations
uint8_t pixelPhases[NUM_PIXELS];
uint8_t pixelColors[NUM_PIXELS];

// ========== FUNCTION DECLARATIONS ==========

// Core functions
void setupHardware();
void enterSleepMode();
void wakeFromSleep();
void updateBrightness();
void handleButtons();

// Mode functions
void runCandlelightMode();
void runTwinkleMode();
void runIndependentPulseMode();
void runSyncPulseMode();

// Utility functions
uint32_t getColorFromScheme(uint8_t mode, uint8_t scheme);
uint8_t getRandomLightness(uint8_t minRange, uint8_t maxRange);
void fadeToBlack(uint8_t pixel, uint16_t duration);
void setPixelColor(uint8_t pixel, uint32_t color);
uint32_t dimColor(uint32_t color, uint8_t brightness);
void playLedTimeline(const uint8_t *timeline, uint16_t steps, uint8_t numLeds, uint16_t stepDelayMs, uint32_t onColor);

// ========== SETUP FUNCTION ==========

void setup() {
    // Initialize random seed
#if RANDOM_SEED == 0
    randomSeed(analogRead(POTENTIOMETER_PIN));
#else
    randomSeed(RANDOM_SEED);
#endif

    // Setup hardware (pins, NeoPixels, interrupts)
    setupHardware();

    // Initialize pixel phases for independent animations
    for (uint8_t i = 0; i < NUM_PIXELS; i++) {
        pixelPhases[i] = random(256);
        pixelColors[i] = random(256);
    }

    // Set initial activity time
    lastActivityTime = millis();

#if ENABLE_SERIAL_DEBUG
    debugSerial.begin(9600);
    debugSerial.println("Night Light Initialized");
#endif
}

// ========== MAIN LOOP ==========

void loop() {
    unsigned long currentTime = millis();

    // Handle brightness control
    updateBrightness();

    // Handle button inputs
    handleButtons();

    // Check sleep timer - only when potentiometer is on and no recent activity
    if (!isSleeping && !wasPotOff && (currentTime - lastActivityTime > SLEEP_TIMEOUT)) {
        // Start sleep fade
        if (sleepStartTime == 0) {
            sleepStartTime = currentTime;
        }

        // Fade out over SLEEP_FADE_TIME
        uint16_t fadeProgress = (currentTime - sleepStartTime) * 255 / SLEEP_FADE_TIME;
        if (fadeProgress >= 255) {
            enterSleepMode();
        } else {
            // Dim all pixels during fade
            for (uint8_t i = 0; i < NUM_PIXELS; i++) {
                pixels.setPixelColor(i, dimColor(pixels.getPixelColor(i), 255 - fadeProgress));
            }
            pixels.show();
        }
    } else {
        // Reset sleep timer if activity detected or potentiometer is off
        sleepStartTime = 0;

        // Run current mode animation only when not sleeping
        if (!isSleeping) {
            switch (currentMode) {
                case MODE_CANDLELIGHT:
                    runCandlelightMode();
                    break;
                case MODE_TWINKLE:
                    runTwinkleMode();
                    break;
                case MODE_INDEPENDENT_PULSE:
                    runIndependentPulseMode();
                    break;
                case MODE_SYNC_PULSE:
                    runSyncPulseMode();
                    break;
            }
        }
    }

    // Update visual debug display (if enabled)
    #if ENABLE_VISUAL_DEBUG
    updateVisualDebug();
    #endif

    // Small delay to prevent overwhelming the processor
    delay(10);
}

// ========== HARDWARE SETUP ==========

void setupHardware() {
    // Configure pins
    pinMode(BUTTON_MODE_PIN, INPUT_PULLUP);
    pinMode(BUTTON_COLOR_PIN, INPUT_PULLUP);
    pinMode(POTENTIOMETER_PIN, INPUT);

    // Initialize NeoPixels
    pixels.begin();
    pixels.setBrightness(globalBrightness);
    pixels.clear();
    pixels.show();

    // Configure sleep mode
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    // Enable pin change interrupts for buttons (PB1 and PB3)
    GIMSK |= (1 << PCIE);    // Enable pin change interrupts
    PCMSK |= (1 << PCINT1) | (1 << PCINT3); // Enable interrupts on PB1 and PB3
}

// ========== SLEEP MANAGEMENT ==========

void enterSleepMode() {
    isSleeping = true;

#if ENABLE_SERIAL_DEBUG
    debugSerial.println("Entering sleep mode...");
#endif

    // Turn off all pixels
    pixels.clear();
    pixels.show();

    // Enter sleep mode
    sleep_enable();
    sei(); // Ensure interrupts are enabled
    sleep_cpu();

    // Code resumes here after wake-up
    sleep_disable();
    wakeFromSleep();
}

void wakeFromSleep() {
    isSleeping = false;
    lastActivityTime = millis();
    sleepStartTime = 0;

#if ENABLE_SERIAL_DEBUG
    debugSerial.println("Woken from sleep");
#endif
}

// ========== BRIGHTNESS CONTROL ==========

void updateBrightness() {
    // Read potentiometer (0-1023 range)
    int potValue = analogRead(POTENTIOMETER_PIN);

    // Determine state transitions using hysteresis thresholds
    bool shouldBeOff = (potValue < OFF_THRESHOLD);
    bool shouldBeOn = (potValue > ON_THRESHOLD);

    // Handle state transitions with hysteresis
    if (!wasPotOff && shouldBeOff) {
        // Currently ON, but potentiometer went below OFF_THRESHOLD - turn OFF
        if (!isSleeping) {
            enterSleepMode();
        }
        wasPotOff = true;
        return; // Don't update brightness while sleeping
    } else if (wasPotOff && shouldBeOn) {
        // Currently OFF, but potentiometer went above ON_THRESHOLD - turn ON
        if (isSleeping) {
            wakeFromSleep();
        }
        // Reset sleep timer on wake-up
        lastActivityTime = millis();
        wasPotOff = false;
    }
    // If in hysteresis zone (between thresholds), maintain current state

    // If currently in OFF state, don't update brightness
    if (wasPotOff) {
        return;
    }

    // Map to brightness range (only when potentiometer is in ON state)
    globalBrightness = map(potValue, 0, 1023, MIN_BRIGHTNESS, MAX_BRIGHTNESS);

    // Apply brightness to NeoPixels
    pixels.setBrightness(globalBrightness);
}

// ========== BUTTON HANDLING ==========

void handleButtons() {
    unsigned long currentTime = millis();

    // Read button states
    bool buttonModeState = digitalRead(BUTTON_MODE_PIN);
    bool buttonColorState = digitalRead(BUTTON_COLOR_PIN);

    // Handle mode button (with debouncing)
    if (buttonModeState != lastButtonModeState) {
        lastButtonModeTime = currentTime;
    }
    if ((currentTime - lastButtonModeTime) > DEBOUNCE_DELAY) {
        if (buttonModeState == LOW && lastButtonModeState == HIGH) {
            // Mode button pressed
            if (isSleeping) {
                // Only wake up if potentiometer is in on position
                if (!wasPotOff) {
                    wakeFromSleep();
                }
            } else {
                // Cycle to next enabled mode
                do {
                    currentMode = (currentMode + 1) % NUM_MODES;
                } while (!isModeEnabled(currentMode));

                currentColorScheme = 0; // Reset color scheme when changing modes
            }
            lastActivityTime = currentTime;

#if ENABLE_VISUAL_DEBUG
            flashButtonDebug(true); // Mode button flash
#endif
#if ENABLE_SERIAL_DEBUG
            debugSerial.print("Mode changed to: ");
            debugSerial.println(currentMode);
#endif
        }
    }
    lastButtonModeState = buttonModeState;

    // Handle color button (with debouncing)
    if (buttonColorState != lastButtonColorState) {
        lastButtonColorTime = currentTime;
    }
    if ((currentTime - lastButtonColorTime) > DEBOUNCE_DELAY) {
        if (buttonColorState == LOW && lastButtonColorState == HIGH) {
            // Color button pressed
            if (isSleeping) {
                // Only wake up if potentiometer is in on position
                if (!wasPotOff) {
                    wakeFromSleep();
                }
            } else {
                // Cycle through color schemes for current mode
                uint8_t maxSchemes = getMaxColorSchemes(currentMode);
                currentColorScheme = (currentColorScheme + 1) % maxSchemes;
            }
            lastActivityTime = currentTime;

#if ENABLE_VISUAL_DEBUG
            flashButtonDebug(false); // Color button flash
#endif
#if ENABLE_SERIAL_DEBUG
            debugSerial.print("Color scheme changed to: ");
            debugSerial.println(currentColorScheme);
#endif
        }
    }
    lastButtonColorState = buttonColorState;
}

// ========== INTERRUPT SERVICE ROUTINES ==========

ISR(PCINT0_vect) {
    // Pin change interrupt - wake from sleep if sleeping
    if (isSleeping) {
        // Don't do anything here - just wake up
        // The main loop will handle the wake-up process
    }
}

// ========== UTILITY FUNCTIONS ==========

bool isModeEnabled(uint8_t mode) {
    switch (mode) {
        case MODE_CANDLELIGHT: return ENABLE_CANDLELIGHT;
        case MODE_TWINKLE: return ENABLE_TWINKLE;
        case MODE_INDEPENDENT_PULSE: return ENABLE_INDEPENDENT_PULSE;
        case MODE_SYNC_PULSE: return ENABLE_SYNC_PULSE;
        default: return false;
    }
}

uint8_t getMaxColorSchemes(uint8_t mode) {
    switch (mode) {
        case MODE_CANDLELIGHT: return CANDLELIGHT_SCHEMES;
        case MODE_TWINKLE: return TWINKLE_SCHEMES;
        case MODE_INDEPENDENT_PULSE: return INDEPENDENT_PULSE_COLORS;
        case MODE_SYNC_PULSE: return SYNC_PULSE_VARIATIONS;
        default: return 1;
    }
}

uint32_t dimColor(uint32_t color, uint8_t brightness) {
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    r = (r * brightness) / 255;
    g = (g * brightness) / 255;
    b = (b * brightness) / 255;

    return pixels.Color(r, g, b);
}

// Plays a sequence of on/off states across N time steps.
void playLedTimeline(const uint8_t *timeline, uint16_t steps, uint8_t numLeds, uint16_t stepDelayMs, uint32_t onColor) {
    if (timeline == nullptr || steps == 0 || numLeds == 0) {
        return;
    }

    for (uint16_t step = 0; step < steps; step++) {
        for (uint8_t led = 0; led < numLeds; led++) {
            uint32_t index = static_cast<uint32_t>(step) * numLeds + led;
            uint32_t color = timeline[index] ? onColor : 0;
            pixels.setPixelColor(led, color);
        }
        pixels.show();
        delay(stepDelayMs);
    }
}

uint8_t getRandomLightness(uint8_t minBrightness, uint8_t maxBrightness) {
    return minBrightness + random(maxBrightness - minBrightness + 1);
}

void setPixelColor(uint8_t pixel, uint32_t color) {
    pixels.setPixelColor(pixel, color);
}

void fadeToBlack(uint8_t pixel, uint16_t duration) {
    // This function could be used for smooth transitions
    // Implementation would require tracking fade state per pixel
    // For now, just set to black immediately
    pixels.setPixelColor(pixel, pixels.Color(0, 0, 0));
}

// ========== VISUAL DEBUG FUNCTIONS ==========

#if ENABLE_VISUAL_DEBUG

void showDebugColor(const uint8_t colors[9]) {
    // Set first 3 pixels to debug colors, leave others unchanged
    for (uint8_t i = 0; i < 3 && i < NUM_PIXELS; i++) {
        uint32_t color = pixels.Color(colors[i*3], colors[i*3+1], colors[i*3+2]);
        pixels.setPixelColor(i, color);
    }
    pixels.show();
}

void updateVisualDebug() {
    unsigned long currentTime = millis();

    if (!debugInitComplete) {
        // Three-stage initialization sequence with progress indicators
        updateInitSequence(currentTime);
    } else {
        // Normal operation - show debug codes or threshold testing
        if (DEBUG_MODE == DEBUG_MODE_THRESHOLD) {
            updateThresholdDebug();
        } else {
            updateNormalDebug(currentTime);
        }
    }
}

void updateInitSequence(unsigned long currentTime) {
    static bool blinkState = false;
    static unsigned long blinkTimer = 0;
    static uint8_t completionBlinkCount = 0;

    // Handle blinking for in-progress states
    if (currentTime - blinkTimer >= 250) { // 250ms blink interval
        blinkState = !blinkState;
        blinkTimer = currentTime;
    }

    switch (debugInitStep) {
        case 0: // Stage 1: Hardware initialization
            if (blinkState) {
                showDebugColor(DEBUG_INIT_STAGE1_IN_PROGRESS);
            } else {
                // Show nothing during blink off
                uint8_t off[9] = {0,0,0,0,0,0,0,0,0};
                showDebugColor(off);
            }
            if (currentTime - debugLastUpdate >= 2000) { // 2 seconds for stage 1
                showDebugColor(DEBUG_INIT_STAGE1_SUCCESS);
                debugLastUpdate = currentTime;
                debugInitStep++;
            }
            break;

        case 1: // Stage 2: NeoPixel initialization
            if (blinkState) {
                showDebugColor(DEBUG_INIT_STAGE2_IN_PROGRESS);
            } else {
                uint8_t off[9] = {0,0,0,0,0,0,0,0,0};
                showDebugColor(off);
            }
            if (currentTime - debugLastUpdate >= 2000) { // 2 seconds for stage 2
                showDebugColor(DEBUG_INIT_STAGE2_SUCCESS);
                debugLastUpdate = currentTime;
                debugInitStep++;
            }
            break;

        case 2: // Stage 3: Random seed initialization
            if (blinkState) {
                showDebugColor(DEBUG_INIT_STAGE3_IN_PROGRESS);
            } else {
                uint8_t off[9] = {0,0,0,0,0,0,0,0,0};
                showDebugColor(off);
            }
            if (currentTime - debugLastUpdate >= 2000) { // 2 seconds for stage 3
                showDebugColor(DEBUG_INIT_STAGE3_SUCCESS);
                debugLastUpdate = currentTime;
                debugInitStep++;
            }
            break;

        case 3: // Completion: All pixels blink green 3 times
            if (completionBlinkCount < 6) { // 3 blinks = 6 state changes
                if (blinkState) {
                    showDebugColor(DEBUG_INIT_COMPLETE);
                } else {
                    uint8_t off[9] = {0,0,0,0,0,0,0,0,0};
                    showDebugColor(off);
                }
                if (currentTime - blinkTimer >= 250) {
                    completionBlinkCount++;
                    if (completionBlinkCount >= 6) {
                        debugInitComplete = true;
                        debugLastUpdate = currentTime;
                    }
                }
            }
            break;
    }
}

void updateNormalDebug(unsigned long currentTime) {
    static unsigned long buttonFlashEnd = 0;
    static bool showingButtonFlash = false;

    // Create debug display array
    uint8_t debugColors[9] = {0,0,0, 0,0,0, 0,0,0};

    // Pixel 0: Current mode (always shown)
    switch (currentMode) {
        case MODE_CANDLELIGHT:
            debugColors[0] = 255; debugColors[1] = 165; debugColors[2] = 0; // Orange
            break;
        case MODE_TWINKLE:
            debugColors[0] = 255; debugColors[1] = 255; debugColors[2] = 255; // White
            break;
        case MODE_INDEPENDENT_PULSE:
            debugColors[0] = 0; debugColors[1] = 0; debugColors[2] = 255; // Blue
            break;
        case MODE_SYNC_PULSE:
            debugColors[0] = 255; debugColors[1] = 0; debugColors[2] = 255; // Magenta
            break;
    }

    // Pixel 1: Status (active/sleep/manual-off)
    if (isSleeping) {
        if (wasPotOff) {
            // Manual off
            debugColors[3] = 255; debugColors[4] = 0; debugColors[5] = 0; // Red
        } else {
            // Auto sleep
            debugColors[3] = 128; debugColors[4] = 0; debugColors[5] = 128; // Purple
        }
    } else {
        // Active/on
        debugColors[3] = 0; debugColors[4] = 255; debugColors[5] = 0; // Green
    }

    // Pixel 2: Button presses (temporary flashes override normal display)
    if (showingButtonFlash && currentTime < buttonFlashEnd) {
        // Keep current button flash display
    } else if (showingButtonFlash) {
        // Button flash ended
        showingButtonFlash = false;
        buttonFlashEnd = 0;
    }

    showDebugColor(debugColors);
}

void updateThresholdDebug() {
    // Read potentiometer
    int potValue = analogRead(POTENTIOMETER_PIN);

    // Show threshold status
    if (potValue < OFF_THRESHOLD) {
        showDebugColor(DEBUG_THRESHOLD_OFF);
    } else if (potValue > ON_THRESHOLD) {
        showDebugColor(DEBUG_THRESHOLD_ON);
    } else {
        showDebugColor(DEBUG_THRESHOLD_DEAD);
    }
}

// Call this function when buttons are pressed to show visual feedback
void flashButtonDebug(bool isModeButton) {
    #if ENABLE_VISUAL_DEBUG
    if (DEBUG_MODE == DEBUG_MODE_NORMAL) {
        if (isModeButton) {
            showDebugColor(DEBUG_BUTTON_MODE);
        } else {
            showDebugColor(DEBUG_BUTTON_COLOR);
        }
        // The flash will be overridden by the next normal debug update
    }
    #endif
}

#endif // ENABLE_VISUAL_DEBUG

// ========== MODE IMPLEMENTATIONS ==========

void runCandlelightMode() {
    static unsigned long lastFlickerTime = 0;
    unsigned long currentTime = millis();

    // Update flicker animation at defined intervals
    if (currentTime - lastFlickerTime >= CANDLE_FLICKER_SPEED) {
        lastFlickerTime = currentTime;

        uint8_t startPixel = 0;
        #if ENABLE_VISUAL_DEBUG
        startPixel = 3; // Skip first 3 pixels used for debug
        #endif

        for (uint8_t pixel = startPixel; pixel < NUM_PIXELS; pixel++) {
            uint32_t baseColor = getCandlelightColor(currentColorScheme);
            uint8_t flickerIntensity = getRandomLightness(180, 255); // 70-100% brightness for flicker

            uint32_t flickeredColor = dimColor(baseColor, flickerIntensity);
            setPixelColor(pixel, flickeredColor);
        }

        pixels.show();
        animationCounter++;
    }
}

uint32_t getCandlelightColor(uint8_t scheme) {
    switch (scheme) {
        case 0: // Orange/yellow flame color-spectrum (default)
            return getFlameColorSpectrum();
        case 1: // Blue lightness-spectrum
            return getLightnessSpectrumColor(pixels.Color(COLOR_BLUE), LIGHTNESS_MED_MIN, LIGHTNESS_MED_MAX);
        case 2: // Green lightness-spectrum
            return getLightnessSpectrumColor(pixels.Color(COLOR_GREEN), LIGHTNESS_MED_MIN, LIGHTNESS_MED_MAX);
        case 3: // White/yellow color-spectrum
            return getColorSpectrum(pixels.Color(COLOR_WHITE), pixels.Color(COLOR_YELLOW));
        case 4: // Rainbow color-spectrum
            return pixels.Color(
                (animationCounter * 5) % 256,      // Red
                (animationCounter * 5 + 85) % 256, // Green
                (animationCounter * 5 + 170) % 256 // Blue
            );
        default:
            return pixels.Color(COLOR_ORANGE);
    }
}

uint32_t getFlameColorSpectrum() {
    // Create realistic flame colors by mixing orange and yellow
    uint8_t ratio = random(256);
    return pixels.Color(
        255,                                    // Red (always high for flame)
        165 + (255 - 165) * ratio / 255,        // Green (orange to yellow)
        random(50)                              // Blue (small amount for realism)
    );
}

uint32_t getLightnessSpectrumColor(uint32_t baseColor, float minLightness, float maxLightness) {
    uint8_t r = (baseColor >> 16) & 0xFF;
    uint8_t g = (baseColor >> 8) & 0xFF;
    uint8_t b = baseColor & 0xFF;

    // Random lightness variation
    float lightness = minLightness + random(1000) * (maxLightness - minLightness) / 1000.0f;

    return pixels.Color(
        (uint8_t)(r * lightness),
        (uint8_t)(g * lightness),
        (uint8_t)(b * lightness)
    );
}

uint32_t getColorSpectrum(uint32_t color1, uint32_t color2) {
    uint8_t ratio = random(256);

    uint8_t r1 = (color1 >> 16) & 0xFF;
    uint8_t g1 = (color1 >> 8) & 0xFF;
    uint8_t b1 = color1 & 0xFF;

    uint8_t r2 = (color2 >> 16) & 0xFF;
    uint8_t g2 = (color2 >> 8) & 0xFF;
    uint8_t b2 = color2 & 0xFF;

    return pixels.Color(
        r1 + (r2 - r1) * ratio / 255,
        g1 + (g2 - g1) * ratio / 255,
        b1 + (b2 - b1) * ratio / 255
    );
}

void runTwinkleMode() {
    static unsigned long lastTwinkleTime = 0;
    unsigned long currentTime = millis();

    // Update twinkle animation at defined intervals
    if (currentTime - lastTwinkleTime >= TWINKLE_FADE_SPEED) {
        lastTwinkleTime = currentTime;

        uint8_t startPixel = 0;
        #if ENABLE_VISUAL_DEBUG
        startPixel = 3; // Skip first 3 pixels used for debug
        #endif

        for (uint8_t pixel = startPixel; pixel < NUM_PIXELS; pixel++) {
            // Decide if this pixel should twinkle based on density
            if (random(1000) < (TWINKLE_DENSITY * 1000)) {
                // This pixel is twinkling
                uint32_t baseColor = getTwinkleColor(currentColorScheme);

                // Create brightness variation for twinkling effect
                uint8_t brightness = getRandomBrightness();
                uint32_t twinkledColor = dimColor(baseColor, brightness);

                setPixelColor(pixel, twinkledColor);
            } else {
                // This pixel stays at base color
                uint32_t baseColor = getTwinkleColor(currentColorScheme);
                setPixelColor(pixel, dimColor(baseColor, BRIGHTNESS_LOW_MAX * 255));
            }
        }

        pixels.show();
        animationCounter++;
    }
}

uint32_t getTwinkleColor(uint8_t scheme) {
    switch (scheme) {
        case 0: // White/light-blue stars color-brightness-spectrum (default)
            return getColorSpectrum(pixels.Color(COLOR_WHITE), pixels.Color(COLOR_LIGHT_BLUE));
        case 1: // Red lightness-brightness-spectrum
            return getLightnessBrightnessSpectrum(pixels.Color(COLOR_RED));
        case 2: // Orange lightness-brightness-spectrum
            return getLightnessBrightnessSpectrum(pixels.Color(COLOR_ORANGE));
        case 3: // Yellow lightness-brightness-spectrum
            return getLightnessBrightnessSpectrum(pixels.Color(COLOR_YELLOW));
        case 4: // Green lightness-brightness-spectrum
            return getLightnessBrightnessSpectrum(pixels.Color(COLOR_GREEN));
        case 5: // Blue lightness-brightness-spectrum
            return getLightnessBrightnessSpectrum(pixels.Color(COLOR_BLUE));
        case 6: // Purple lightness-brightness-spectrum
            return getLightnessBrightnessSpectrum(pixels.Color(COLOR_PURPLE));
        case 7: // Rainbow color-spectrum
            return pixels.Color(
                (animationCounter * 3) % 256,
                (animationCounter * 3 + 85) % 256,
                (animationCounter * 3 + 170) % 256
            );
        default:
            return pixels.Color(COLOR_WHITE);
    }
}

uint32_t getLightnessBrightnessSpectrum(uint32_t baseColor) {
    uint8_t r = (baseColor >> 16) & 0xFF;
    uint8_t g = (baseColor >> 8) & 0xFF;
    uint8_t b = baseColor & 0xFF;

    // Combine lightness and brightness variations
    float lightness = LIGHTNESS_LOW_MIN + random(1000) * (LIGHTNESS_HIGH_MAX - LIGHTNESS_LOW_MIN) / 1000.0f;
    float brightness = BRIGHTNESS_LOW_MIN + random(1000) * (BRIGHTNESS_HIGH_MAX - BRIGHTNESS_LOW_MIN) / 1000.0f;

    float combined = lightness * brightness;

    return pixels.Color(
        (uint8_t)(r * combined),
        (uint8_t)(g * combined),
        (uint8_t)(b * combined)
    );
}

uint8_t getRandomBrightness() {
    // Create a brightness spectrum with peaks at different levels
    uint8_t rand = random(256);
    if (rand < 128) {
        // Low brightness range
        return BRIGHTNESS_LOW_MIN * 255 + random((BRIGHTNESS_LOW_MAX - BRIGHTNESS_LOW_MIN) * 255);
    } else if (rand < 224) {
        // Medium brightness range
        return BRIGHTNESS_MED_MIN * 255 + random((BRIGHTNESS_MED_MAX - BRIGHTNESS_MED_MIN) * 255);
    } else {
        // High brightness range
        return BRIGHTNESS_HIGH_MIN * 255 + random((BRIGHTNESS_HIGH_MAX - BRIGHTNESS_HIGH_MIN) * 255);
    }
}

void runIndependentPulseMode() {
    static unsigned long lastPulseTime = 0;
    unsigned long currentTime = millis();

    // Update pulse animation at defined intervals
    if (currentTime - lastPulseTime >= PULSE_SPEED_DEFAULT) {
        lastPulseTime = currentTime;

        uint8_t startPixel = 0;
        #if ENABLE_VISUAL_DEBUG
        startPixel = 3; // Skip first 3 pixels used for debug
        #endif

        for (uint8_t pixel = startPixel; pixel < NUM_PIXELS; pixel++) {
            // Each pixel has its own phase and color
            uint8_t phase = pixelPhases[pixel];
            uint32_t baseColor = getIndependentPulseColor(currentColorScheme);

            // Create sine wave pulse effect using fixed-point math (avoid floating point)
            // sin(x) approximation using lookup table would be ideal, but for now use simple triangle wave
            int8_t pulse_val;
            if (phase < 64) {
                pulse_val = phase * 2;
            } else if (phase < 128) {
                pulse_val = 127 - (phase - 64) * 2;
            } else if (phase < 192) {
                pulse_val = -127 + (phase - 128) * 2;
            } else {
                pulse_val = (255 - phase) * 2;
            }

            uint8_t brightness = 128 + pulse_val; // 0-255 range

            uint32_t pulsedColor = dimColor(baseColor, brightness);
            setPixelColor(pixel, pulsedColor);

            // Advance phase for next frame
            pixelPhases[pixel] = (phase + 1) % 256;
        }

        pixels.show();
        animationCounter++;
    }
}

uint32_t getIndependentPulseColor(uint8_t scheme) {
    switch (scheme) {
        case 0: return pixels.Color(COLOR_BLUE);    // Default
        case 1: return pixels.Color(COLOR_RED);
        case 2: return pixels.Color(COLOR_ORANGE);
        case 3: return pixels.Color(COLOR_YELLOW);
        case 4: return pixels.Color(COLOR_GREEN);
        case 5: return pixels.Color(COLOR_PURPLE);
        default: return pixels.Color(COLOR_BLUE);
    }
}

void runSyncPulseMode() {
    static unsigned long lastPulseTime = 0;
    static uint8_t pulsePhase = 0;
    unsigned long currentTime = millis();

    // Update pulse animation at defined intervals
    if (currentTime - lastPulseTime >= SYNC_PULSE_DURATION / 256) { // Smooth pulse over duration
        lastPulseTime = currentTime;

        // Create triangle wave pulse using fixed-point math (avoid floating point)
        int8_t pulse;
        if (pulsePhase < 64) {
            pulse = pulsePhase * 2;
        } else if (pulsePhase < 128) {
            pulse = 127 - (pulsePhase - 64) * 2;
        } else if (pulsePhase < 192) {
            pulse = -127 + (pulsePhase - 128) * 2;
        } else {
            pulse = (255 - pulsePhase) * 2;
        }

        uint8_t brightness = 128 + pulse; // 0-255 range

        uint32_t baseColor = getSyncPulseColor(currentColorScheme, pulsePhase);

        // Set all pixels to the same color and brightness
        uint8_t startPixel = 0;
        #if ENABLE_VISUAL_DEBUG
        startPixel = 3; // Skip first 3 pixels used for debug
        #endif

        for (uint8_t pixel = startPixel; pixel < NUM_PIXELS; pixel++) {
            uint32_t pulsedColor = dimColor(baseColor, brightness);
            setPixelColor(pixel, pulsedColor);
        }

        pixels.show();

        // Advance phase
        pulsePhase = (pulsePhase + 1) % 256;

        // Change color for rainbow modes
        if (currentColorScheme <= 1 && pulsePhase == 0) {
            animationCounter++;
        }

        animationCounter++;
    }
}

uint32_t getSyncPulseColor(uint8_t scheme, uint8_t phase) {
    switch (scheme) {
        case 0: // Rainbow cycle - change each pulse (default)
            return pixels.Color(
                (animationCounter * 10) % 256,
                (animationCounter * 10 + 85) % 256,
                (animationCounter * 10 + 170) % 256
            );
        case 1: // Rainbow cycle - cycle through all colors each pulse
            return pixels.Color(
                (phase * 6) % 256,
                (phase * 6 + 85) % 256,
                (phase * 6 + 170) % 256
            );
        case 2: return pixels.Color(COLOR_RED);
        case 3: return pixels.Color(COLOR_ORANGE);
        case 4: return pixels.Color(COLOR_YELLOW);
        case 5: return pixels.Color(COLOR_GREEN);
        case 6: return pixels.Color(COLOR_BLUE);
        case 7: return pixels.Color(COLOR_PURPLE);
        default: return pixels.Color(COLOR_BLUE);
    }
}
