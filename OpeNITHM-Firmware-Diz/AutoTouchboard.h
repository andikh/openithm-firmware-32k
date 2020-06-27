// Touchboard.h

#ifndef _TOUCHBOARD_h
#define _TOUCHBOARD_h

#include "Config.h"
#include "Output.h"
#include "PinConfig.h"
#ifndef TEENSY
#include "CapacitiveSensor.h"
#endif
#include <EEPROM.h>
#include <FastLED.h>

#define CALIBRATION_SAMPLES 75
#define CALIBRATION_DETECTION_THRESHOLD 8
#define CALIBRATION_FLAG 0xFF
#define DEFAULT_SENSITIVITY 97
extern CRGB leds[31];

class AutoTouchboard
{
  private:
#ifndef TEENSY
    CapacitiveSensor sensor;
#endif
    uint8_t sensitivity;
    uint16_t key_values[32];
    uint16_t lowest_key_values[32];
    uint16_t single_thresholds[32];
    uint16_t double_thresholds[32];

  public:
    AutoTouchboard();
    void scan();
    void loadConfig();
    void saveConfig();
    KeyState update(int key);
    uint16_t getRawValue(int key);
    void calibrateKeys(bool forceCalibrate = false);
    void setSensitivity(uint8_t sensitivity);
    uint8_t getSensitivity();
};

#endif
