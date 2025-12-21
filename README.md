# Calming Night Light for ATTiny85

A feature-rich night light project using an ATTiny85 microcontroller, NeoPixels, and various controls for creating soothing lighting effects.

## Features

- **4 Lighting Modes**: Candlelight, Twinkle, Independent Pulse, and Synchronized Pulse
- **Brightness Control**: 10k linear potentiometer for adjustable brightness
- **Mode Cycling**: Button to cycle through different lighting modes
- **Color Cycling**: Button to cycle through color schemes within each mode
- **Sleep Timer**: Automatically enters sleep mode after configurable timeout
- **Wake on Button Press**: Interrupt-based wake-up from sleep mode
- **Configurable**: Extensive configuration options in `config.h`

## Hardware Requirements

- ATTiny85 microcontroller
- NeoPixel strip/array (configurable number of pixels)
- 10k linear potentiometer
- 2 push buttons (momentary, normally open)
- Power supply appropriate for your NeoPixels
- Arduino board (for programming via Arduino as ISP)

## Pin Connections (ATTiny85)

| Component | Pin | Description |
|-----------|-----|-------------|
| NeoPixels | PB0 | Data signal |
| Potentiometer | PB2 (A1) | Analog brightness control |
| Mode Button | PB1 | Cycle through modes |
| Color Button | PB3 | Cycle through colors |

## Wiring Diagram

```
ATTiny85
+------+
|      |
| PB5  | -- (Reset - for programming)
| PB3  | -- Color Button -- GND
| PB4  | -- (Unused)
| GND  | -- GND
| PB0  | -- NeoPixel Data
| PB1  | -- Mode Button -- GND
| PB2  | -- Potentiometer Wiper
| VCC  | -- +5V
+------+

Potentiometer: One end to GND, other end to +5V, wiper to PB2
Buttons: One side to pin, other side to GND (internal pullup used)
```

## Installation

1. **Install Arduino IDE** and ATTiny85 support:
   - Add ATTiny board support via Board Manager: `http://drazzy.com/package_drazzy.com_index.json`
   - Install "ATTinyCore" by Spence Konde

2. **Install Libraries**:
   - Adafruit NeoPixel library (via Library Manager)

3. **Configure** the project:
   - Edit `config.h` to match your hardware setup
   - Adjust NeoPixel array dimensions, pin assignments, timing, etc.

4. **Program ATTiny85**:
   - Connect Arduino as ISP programmer
   - Select "ATTiny85" board with 8MHz internal clock
   - Upload the sketch

## Configuration

All configuration is done in `config.h`. Key settings:

### Hardware Configuration
- `NUM_ROWS`, `NUM_COLS`: NeoPixel array dimensions
- `ORIENTATION`: 0=row-major, 1=column-major
- Pin assignments for buttons and potentiometer

### Behavior Configuration
- `MAX_BRIGHTNESS`, `MIN_BRIGHTNESS`: Brightness range when potentiometer is on
- `SLEEP_TIMEOUT`: Time before auto-sleep when potentiometer is on (milliseconds)
- `OFF_THRESHOLD`: Potentiometer value below which device enters sleep (0-1023)
- `ON_THRESHOLD`: Potentiometer value above which device wakes from sleep (0-1023)
- `PULSE_SPEED_DEFAULT`: Pulse animation speed
- `DEFAULT_MODE`: Starting mode after power-on

### Mode Enable/Disable
- `ENABLE_CANDLELIGHT`, `ENABLE_TWINKLE`, etc.
- Set to 0 to disable unwanted modes

### Color Definitions
- RGB values for all colors used in modes
- Lightness and brightness spectrum ranges

## Usage

1. **Power On**: Light starts in default mode with default color scheme
2. **Brightness Control**: Turn potentiometer to adjust brightness (MIN to MAX range)
3. **Manual Off**: Turn potentiometer down below OFF_THRESHOLD to enter sleep mode
4. **Wake Up**: Turn potentiometer up above ON_THRESHOLD to wake from sleep mode
5. **Mode Cycling**: Press mode button to cycle through enabled modes (only when ON)
6. **Color Cycling**: Press color button to cycle through color schemes in current mode (only when ON)
7. **Auto Sleep**: After timeout with no button presses AND potentiometer in ON zone, light fades and enters sleep mode

## Power States with Hysteresis

- **ON Zone**: Potentiometer > ON_THRESHOLD - normal operation, buttons work, auto-sleep active
- **OFF Zone**: Potentiometer < OFF_THRESHOLD - sleep mode, buttons ignored for wake-up
- **Dead Zone**: Between OFF_THRESHOLD and ON_THRESHOLD - maintains current power state (hysteresis)
- **Transition ON→OFF**: Only when crossing below OFF_THRESHOLD
- **Transition OFF→ON**: Only when crossing above ON_THRESHOLD

This hysteresis prevents rapid on/off switching when the potentiometer is near the threshold.

## Lighting Modes

### 1. Candlelight Mode
- Simulates flickering candle flame
- **Color Schemes**:
  - Orange/yellow flame (default)
  - Blue lightness spectrum
  - Green lightness spectrum
  - White/yellow blend
  - Rainbow spectrum

### 2. Twinkle Mode
- Random pixels twinkle at different brightness levels
- **Color Schemes**:
  - White/light-blue stars (default)
  - Red/green/blue/purple lightness-brightness spectra
  - Rainbow spectrum

### 3. Independent Pulse Mode
- Each pixel pulses independently at different phases
- **Colors**: Blue (default), Red, Orange, Yellow, Green, Purple

### 4. Synchronized Pulse Mode
- All pixels pulse together in sync
- **Variations**:
  - Rainbow cycle - changes each pulse (default)
  - Rainbow cycle - cycles through all colors each pulse
  - Single colors: Red, Orange, Yellow, Green, Blue, Purple

## Memory Optimization

The code is optimized for ATTiny85's limited memory:
- Uses fixed-point math instead of floating point
- Minimizes global variables
- Uses uint8_t where possible
- No string operations
- Efficient color calculations

## Troubleshooting

- **No lights**: Check NeoPixel connections and power supply
- **Buttons not responding**: Verify button wiring and pin assignments
- **Brightness not working**: Check potentiometer connections
- **Sleep not working**: Verify interrupt pins and sleep configuration
- **Compilation errors**: Ensure ATTiny85 board is selected and libraries are installed

### Visual Debug Mode (Recommended for ATTiny85)

The visual debug system uses the first 3 NeoPixels as status indicators - no additional hardware required!

#### **Enabling Visual Debug**
Set in `config.h`:
```c
#define ENABLE_VISUAL_DEBUG 1    // Enable visual debugging
#define DEBUG_MODE DEBUG_MODE_NORMAL  // or DEBUG_MODE_THRESHOLD
```

#### **Debug Display Sequence**

1. **Three-Stage Initialization Sequence** (shown once at startup):
   - **Stage 1 (Pixel 0)**: Hardware init - Yellow blink (2s) → Green solid (complete)
   - **Stage 2 (Pixel 1)**: NeoPixels init - Yellow blink (2s) → Green solid (complete)
   - **Stage 3 (Pixel 2)**: Random seed init - Yellow blink (2s) → Green solid (complete)
   - **Completion**: All pixels blink green 3 times (init successful)

2. **Normal Debug Mode** (`DEBUG_MODE_NORMAL`):
   - **Pixel 0**: Current mode (solid color, always shown)
   - **Pixel 1**: Status (active/sleep/manual-off, solid color)
   - **Pixel 2**: Button presses (brief yellow/cyan flashes)

3. **Threshold Testing Mode** (`DEBUG_MODE_THRESHOLD`):
   - Shows potentiometer threshold status continuously
   - Great for tuning OFF_THRESHOLD and ON_THRESHOLD values

#### **Visual Debug Code Lookup Table**

##### **Initialization Codes** (Three-Stage Process)
| Pixel | Color Pattern | Meaning | Duration |
|-------|---------------|---------|----------|
| **0** | 🟡 blinking | Hardware init in progress | 2s |
| **0** | 🟢 solid | Hardware init complete | Until next stage |
| **1** | 🟡 blinking | NeoPixels init in progress | 2s |
| **1** | 🟢 solid | NeoPixels init complete | Until next stage |
| **2** | 🟡 blinking | Random seed init in progress | 2s |
| **2** | 🟢 solid | Random seed init complete | Until completion |
| **All** | 🟢 blinking | Initialization complete! | 3 blinks |

*Random seed initialization ensures different animation patterns each power-on for more natural lighting effects.*

##### **Runtime Debug Codes** (Normal Mode)
| Pixel | Color Pattern | Meaning |
|-------|---------------|---------|
| **0** | 🟠 solid | Candlelight mode active |
| **0** | ⚪ solid | Twinkle mode active |
| **0** | 🔵 solid | Independent pulse mode active |
| **0** | 🟣 solid | Sync pulse mode active |
| **1** | 🟢 solid | Device active/ON |
| **1** | 🟣 solid | Auto-sleep mode |
| **1** | 🔴 solid | Manual OFF mode |
| **2** | 🟡 flash | Mode button pressed |
| **2** | 🟦 flash | Color button pressed |

##### **Threshold Testing Codes**
| Color Pattern | Meaning | Potentiometer Position |
|---------------|---------|----------------------|
| 🔴⚫⚫ | OFF zone | < OFF_THRESHOLD |
| 🟡⚫⚫ | Dead zone | Between OFF_THRESHOLD and ON_THRESHOLD |
| 🟢⚫⚫ | ON zone | > ON_THRESHOLD |

#### **Debug Configuration**
```c
#define DEBUG_INIT_HOLD_TIME    5000  // Hold init complete (ms)
#define DEBUG_CODE_DISPLAY_TIME 2000  // Show each code (ms)
#define DEBUG_OFF_TIME          3000  // Lights off between cycles (ms)
```

#### **Using Threshold Testing Mode**
1. Set `DEBUG_MODE DEBUG_MODE_THRESHOLD` in config.h
2. Upload code and watch the first 3 pixels
3. Turn potentiometer to find your OFF position (red pixels)
4. Turn to ON position (green pixels)
5. Adjust OFF_THRESHOLD and ON_THRESHOLD in config.h
6. Switch back to normal mode when done

#### **Serial Debug Option** (Alternative)
For ATTiny85 with serial adapter:
```c
#define ENABLE_SERIAL_DEBUG 1   // Connect PB4 to serial RX
```
Uses ~1KB extra memory. Use Arduino Serial Monitor at 9600 baud.

## Customization

To add new modes or modify behavior:
1. Add mode constant in `config.h`
2. Implement mode function following existing patterns
3. Add case in main loop switch statement
4. Update `getMaxColorSchemes()` function
5. Add color scheme handling in mode function

## License

This project is open source. Feel free to modify and distribute.