// AirSensor.h

#ifndef _AIRSENSOR_h
#define _AIRSENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include <FastLED.h>
#include "AutoTouchboard.h"
#include "PinConfig.h"
#include <EEPROM.h>
extern CRGB leds[31];
extern AutoTouchboard *touchboard;

#define DEFAULT_SENSITIVITY 50

class AirSensor
{
  private:
    void changeLight(int light);
    void turnOffLight();

    uint8_t analogSensitivity;
    int thresholds[6];
    int highestthresholds[6];
    int samplesToAcquire;
    int samplesToSkip;

    bool calibrated[6];
    bool digitalMode;
    
  public:
    AirSensor(int requiredSamples);
    AirSensor(int requiredSamples, int skippedSamples);
    void analogCalibrate();
    void turnOnAllLight();
    void calibrateAnalogv2();
    bool isCalibrated();
    bool isDigital();
    bool getSensorState(int sensor);
    bool getSensorStateV2(int sensor);
    int getValue(int sensor, bool light);
    int getValue(int sensor);
    float getHandPosition();
    uint8_t getSensorReadings();
    uint8_t getSensorReadingsV2();
    bool getSensorCalibrated(int i);
    void setAnalogSensitivity(uint8_t analogSensitivity);
    uint8_t getAnalogSensitivity();
    void recalibrate();
};


#endif
