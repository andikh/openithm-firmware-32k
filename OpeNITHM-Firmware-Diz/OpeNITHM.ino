#include "PinConfig.h"

#ifdef USB
#include "USBOutput.h"
#else
#include "SerialOutput.h"
#endif

#include "AirSensor.h"
#include "AutoTouchboard.h"
#include "SerialLeds.h"
#include "SerialProcessor.h"
#include "HelperClass.h"
#include <FastLED.h>

SerialProcessor serialProcessor;

KeyState key_states[32];
#ifndef KEY_DIVIDERS
CRGB leds[16];
#else
CRGB leds[31];
#endif
byte serialBuffer[200];
bool updateLeds = false;
bool useSerialLeds = false;
bool halfSerialLeds = false;
int serialLightsCounter;

CRGB led_on;
CRGB led_off;

float lightIntensity[32];
bool activated = true;

AutoTouchboard *touchboard;
AirSensor *sensor;
Output *output;
SerialLeds *serialLeds;

void initializeController();

void setup() {
  Serial.begin(115200);
  for(int i = 0; i < 128; i++) EEPROM.put(i, 0);
  #ifndef KEY_DIVIDERS
  FastLED.addLeds<LED_TYPE, RGBPIN, LED_ORDER>(leds, 16);
  #else
  //Uncomment and tune this value if you're having power issues
  //FastLED.setMaxPowerInVoltsAndMilliamps(5,400);
  FastLED.addLeds<LED_TYPE, RGBPIN, LED_ORDER>(leds, 31);
  #endif

  initializeController();
  // Uncomment this to clear EEPROM, flash once, then comment and re-flash
}

void initializeController() {
  // Flash LEDs orange 3 times
  for (int i = 0; i < 3; i++)
  {
    for (CRGB& led : leds)
      led = CRGB::Orange;
      
    FastLED.show();
    delay(1000);
    
    for (CRGB& led : leds)
      led = CRGB::Black;
      
    FastLED.show();
    delay(1000);
  }
  if (touchboard != NULL) delete touchboard;
  touchboard = new AutoTouchboard();

  // Initialize the serial LED processor
  if (serialLeds != NULL) delete serialLeds;
  serialLeds = new SerialLeds();

   for (CRGB& led : leds)
      led = CRGB::Orange;
  // Initialize air sensor
  if (sensor != NULL) delete sensor;
  sensor = new AirSensor(100);

  // Display the number of air sensors that were calibrated
//  for (CRGB& led : leds)
//    led = CRGB::Black;
    
  for (int i = 0; i < 6; i++)
  {
    if (sensor->getSensorCalibrated(i))
#ifndef KEY_DIVIDERS
      leds[i] = CRGB::Green;
#else
      leds[i*2] = CRGB::Green;
#endif
    else
#ifndef KEY_DIVIDERS
      leds[i] = CRGB::Red;
#else
      leds[i*2] = CRGB::Red;
#endif
  }
  
  FastLED.show();
  delay(3000); 

  // Initialize and calibrate touch sensors
  

  // Initialize the reactive lighting colors, and if they're not in EEPROM, save them
  byte lightsFlag;
  EEPROM.get(67, lightsFlag);

  if (lightsFlag == LIGHTS_FLAG) 
  {
    serialLeds->loadLights();
  } 
  else 
  {
    led_on = CRGB::Purple;
    led_off = CRGB::Yellow;
    serialLeds->saveLights();
  }


  serialLightsCounter = 0;

  // Initialize relevant output method / USB or serial
  if (output != NULL) delete output;
  
#ifdef USB
  output = new USBOutput();
#else
  output = new SerialOutput();
#endif
}

void loop() {
 // halfSerialLeds = !halfSerialLeds;
  // Check for serial messages
  if (Serial.available() >= 200)
  {
    Serial.readBytes(serialBuffer, 200);
    serialProcessor.processBulk(serialBuffer);
  }
  else 
  {
    serialLightsCounter++;  
  }

  // If currently paused through a config command, do not execute main loop
  if (!activated) 
    return;

  // If we haven't received any serial light updates in 5 seconds, just fallback to reactive lighting
  if (serialLightsCounter > 300) 
    useSerialLeds = false;
 
  // Scan touch keyboard and update lights
  touchboard->scan();
  int index = 0;
  
  for (int i = 0; i < 16; i++)
  {
#ifndef LED_REVERSE
    index = i;
#else
    index = 15 - i;
#endif

    KeyState keyState = touchboard->update(i * 2);
    KeyState keyState2 = touchboard->update(i * 2 + 1);

    // handle changing key colors for non-serial LED updates
    if (!useSerialLeds)
    {
      if (lightIntensity[index] > 0.05f)
        lightIntensity[index] -= 0.05f;
  
      // If the key is currently being held, set its color to the on color
      if (keyState == SINGLE_PRESS && keyState2 == SINGLE_PRESS)
      {
        lightIntensity[index] = 1.0f;
#ifndef KEY_DIVIDERS
        leds[index].setRGB(min(led_on.r / 2 + led_on.r / 2 * lightIntensity[index], 255), min(led_on.g / 2 + led_on.g / 2 * lightIntensity[index], 255), min(led_on.b / 2 + led_on.b / 2 * lightIntensity[index], 255));
#else
        leds[index*2] = CRGB::Purple;
#endif
      }
      else if (keyState == SINGLE_PRESS) {
        leds[index*2] = CRGB::Red;
      } 
      else if (keyState2 == SINGLE_PRESS) {
        leds[index*2] = CRGB::Silver;
      }
      else
      {
        // If not, make it the off color
#ifndef KEY_DIVIDERS
        leds[index].setRGB(led_off.r / 2, led_off.g / 2, led_off.b / 2);
#else
        leds[index*2].setRGB(led_off.r / 2, led_off.g / 2, led_off.b / 2);
#endif
      }

#ifdef KEY_DIVIDERS     
      lightIntensity[index] = 1.0f;
      leds[index*2-1].setRGB(min(led_on.r / 2 + led_on.r / 2 * lightIntensity[index], 255), min(led_on.g / 2 + led_on.g / 2 * lightIntensity[index], 255), min(led_on.b / 2 + led_on.b / 2 * lightIntensity[index], 255));
#endif

      updateLeds = true;  
    }
    // handle changing key colors for serial LED updates
    else 
    {
      if (updateLeds)
      {
       // if(!halfSerialLeds) {}
     //   else {    
          RGBLed temp = serialLeds->getKey(i);
  #ifndef KEY_DIVIDERS
          leds[index].setRGB(temp.r, temp.g, temp.b);
  #else
          leds[index*2].setRGB(temp.r, temp.g, temp.b);
          if (i < 15){
            temp = serialLeds->getDivider(i);
            index = HelperClass::getDividerIndex(i);
            leds[index].setRGB(temp.r, temp.g, temp.b);
          }
  #endif
        //}
      }
    }

#if !defined(SERIAL_PLOT) && defined(USB)
    #ifndef TOUCH_16_ONLY
      if (key_states[i * 2] != keyState)
        output->sendKeyEvent(i * 2, keyState);
      if (key_states[i * 2 + 1] != keyState2)
        output -> sendKeyEvent(i * 2 + 1, keyState2);
    #else
      if (keyState == SINGLE_PRESS || keyState2 == SINGLE_PRESS)
        output->sendKeyEvent(i * 2, SINGLE_PRESS);
      else output->sendKeyEvent(i * 2, UNPRESSED);
    #endif
#endif

    key_states[i * 2] = keyState;
    key_states[i * 2 + 1] = keyState2;
  }

#ifdef SERIAL_PLOT
  if (PLOT_PIN == -1)
  {
    for (int i = 28; i < 28; i++)
    {
#ifdef SERIAL_RAW_VALUES
      // Print values
      Serial.print(touchboard->getRawValue(i));
#else
      // Print normalized values
      Serial.print(touchboard->getRawValue(i) - touchboard->getNeutralValue(i));
#endif
      Serial.print("\t");
    }
    Serial.println();
  }
  else
  {
    Serial.print(touchboard->getRawValue(PLOT_PIN));
    Serial.println();
  }
#endif

  // Process air sensor hand position
#if !defined(SERIAL_PLOT) && defined(USB)
#ifdef IR_SENSOR_KEY
    output->sendSensor(sensor->getSensorReadings());
#else
    output->sendSensorEvent2(sensor->getSensorReadings(), true);
#endif
#endif

  // Send update
#if !defined(SERIAL_PLOT) && defined(USB)
  output->sendUpdate();
#endif
//
  #if defined(SERIAL_PLOT)
    Serial.print("\t");
    Serial.println(sensor->getSensorReadings());
    //Serial.println(sensor->getValue(0, true));  
  #endif

  // If the air sensor is calibrated, update lights. The lights will stay red as long as the air sensor is not calibrated.
  if (sensor->isCalibrated() && updateLeds)
  {
    FastLED.show();
    updateLeds = false;
  }
}
