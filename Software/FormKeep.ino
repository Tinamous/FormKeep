// This #include statement was automatically added by the Particle IDE.
#include "IdleState.h"

// This #include statement was automatically added by the Particle IDE.
#include "TimerRunningState.h"

// This #include statement was automatically added by the Particle IDE.
#include "adafruit-ina219/adafruit-ina219.h"

// This #include statement was automatically added by the Particle IDE.
#include "neopixel/neopixel.h"

// This #include statement was automatically added by the Particle IDE.
#include "OneWire/OneWire.h"

//////////////////////////////////////////////////////////////////////////
// FORM [1/1+/2] KEEP. UV box control + monitoring.
//////////////////////////////////////////////////////////////////////////
// V2.02  board
//////////////////////////////////////////////////////////////////////////
int BoardVersion = 3;

// D0/D1 - I2C only

// USV String 1
int uvString1Pin = A4; // Channel 1
int uvString2Pin = A5; // Channel 2
int whiteLedsPin = RX; // Channel 3
int fanPin = TX; // Channel 4

// User panel switch
// Door sensor hall effect switch input
int switchPin = D6;
// Door sensor hall effect Power.
int switchDrivePin = D7;

int vInPin = A3;
OneWire ds = OneWire(A2);  // 1-wire signal on pin D4
int lightLevelPin = A1;
int pirPin = A0;



// byte sensor1[] = {0x28, 0xD6, 0xA9, 0x4E, 0x07, 0x00, 0x00, 0x27};
// onboard temperature sensor address.
// TODO: Allow for UV box temperature, Upper box temperature and external temperature.
byte sensor[8];

// Current sensor
Adafruit_INA219 ina219;

//////////////////////////////////////////////////////////////////////////
// Particle Functions
//////////////////////////////////////////////////////////////////////////
int on(String command);
int off(String command);
int startTimer(String command);
int stopTimer(String args);
int setTimer(String args);

//////////////////////////////////////////////////////////////////////////
// State
//////////////////////////////////////////////////////////////////////////
// State:
// 0: Idel
// 1: timer running
// 10: timer complete, waiting for user to remove the itme
int currentState = 0;

//////////////////////////////////////////////////////////////////////////
// Interrup flags
//////////////////////////////////////////////////////////////////////////
volatile bool doorOpened = false;
volatile bool trayDoorOpened = false;


// Setup NeoPixel (clock) circle.
#define PIXEL_PIN D5
#define PIXEL_COUNT 12
#define PIXEL_TYPE WS2812B

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

// Monitoring variables
float temperatureCelsius = 0;
double currentMilliAmps = 0;
double voltsIn = 0; // 5V rail monitor.
double supplyVoltage = 0; // the 12V in (monitored via current sensor)

// How many seconds to wait before turning off the lights.
double lightsOffInSeconds = 0;

// Measurement timer. Every n seconds.
Timer countDownTimer(1000, countDownTimerTick);

void setup() {
    
    // Setup NeoPixel LED (strip).
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'
    
    // Smile.
    for (int i=2; i<=8; i++ ) {
        strip.setColorDimmed(i, 0, 255, 0, 128);
    }
    strip.setColorDimmed(0, 0, 0, 255, 128);
    strip.setColorDimmed(10, 0, 0, 255, 128);
    strip.show();
    
    // Power drives.
    pinMode(uvString1Pin, OUTPUT);
    pinMode(uvString2Pin, OUTPUT);
    pinMode(whiteLedsPin, OUTPUT);
    pinMode(fanPin, OUTPUT);
    
    // Switch
    pinMode(switchPin, INPUT_PULLUP); // Interrupt.
    pinMode(switchDrivePin, OUTPUT);
    
    // Sensors
    pinMode(vInPin, INPUT);
    pinMode(lightLevelPin, INPUT);
    // PIR input must be reduced to guard against 5v.
    pinMode(pirPin, INPUT); // Hall effect #2 on upper case
    
     
    digitalWrite(uvString1Pin, false);
    digitalWrite(uvString2Pin, false);
    digitalWrite(whiteLedsPin, false);
    digitalWrite(fanPin, false);
    digitalWrite(switchDrivePin, true);
     
    Particle.function("startTimer", startTimer);
    Particle.function("stopTimer", stopTimer);
    Particle.function("on", on); // Args: "Fan", "UV", "White"
    Particle.function("off", off);// Args: "Fan", "UV", "White"
    Particle.function("setTimer", setTimer); // Args: time in seconds for the standard timer.
    
    // Initialize the INA219.
    // By default the initialization will use the largest range (32V, 2A).  However
    // you can call a setCalibration function to change this range (see comments).
    // NB: Check which resistor is to be used. Probably 0R05 due to high current.
    ina219.begin();

    // interrupt on the falling edge of A7 (switch pulled low)
    attachInterrupt(switchPin, doorOpenedIsr, FALLING);
    
    // attachInterrup to PIR (Hall effect #2) pin
    attachInterrupt(pirPin, upperDoorOpenedIsr, RISING);
    
    listTemperatureSensors();

    Particle.publish("Status", "Form Keep. Version: 0.2.1.");
    Particle.publish("Version", "0.2.1");
    
    
    //countDownTimer.start();
}

int loopCounter = 0;
// the last value of lightsOffInSeconds that was used for setting the clock.
int lastLightsOffInSeconds = 0;
bool doorOpen = false;
bool trayDoorOpen = false;
bool uvLedsOn = false;
bool whiteLedsOn = false;
bool fanOn = false;

void loop() {
    if (currentState == 1 && lightsOffInSeconds <=0) {
        stopUVTimer();
    }
    
    loopCounter++;
    if (loopCounter > 200) {
        loopCounter = 0;
        //takeMeasurements();
    }
    
    if (doorOpened) {
        Particle.publish("Status", "Door opened.", 60, PRIVATE);
        
        // If currently idle waiting for blah...
        if (currentState == 0) {
            // Immediatly start the fan to keep IPA vapours low
            switchFanOn();
            switchWhiteLedsOn();
            waitForDoorClose();
            startUVTimer();
        }
        
        // If currently running UV Timer
        if (currentState == 1) {
            // leave lights on.
            // Don't switch the lights on when opened during UV exposure.
            // switchWhiteLedsOn();
        }
        
        // Finished waiting for user to open door.
        if (currentState == 10) {
            Particle.publish("Status", "State = 10, moving to state 0", 60, PRIVATE);
            // user is taking thing out.
            waitForDoorClose();
            switchWhiteLedsOff();
            // restore state to idle.
            currentState = 0;
        }
        
        // Debouce.
        delay(500);
        doorOpened = false;
    }
    
    // if the tray door was opened and it is not already open
    if (trayDoorOpened) {
        Particle.publish("Status", "Tray door opened.", 60, PRIVATE);
        trayDoorOpen = true;
        trayDoorOpened = false;
    }
    
    if (trayDoorOpen) {
        // Check to see if it has now been closed.
        // if so, we can set trayDoorOpen to false.
        // If it's been open for some time we
        // need to beep......
    }
    
    if (lightsOffInSeconds != lastLightsOffInSeconds ) {
        lastLightsOffInSeconds = lightsOffInSeconds;
        
        // Update Neopixel clock.
        for (int i=0; i<12; i++) {
            // uint16_t aLedNumber, byte aRed, byte aGreen, byte aBlue, byte aBrightness
            if (lightsOffInSeconds > ((11-i) * 10)) { // each pixel worth 10 seconds
                strip.setColorDimmed(i, 0, 255, 255, 128);
            } else {
                strip.setColorDimmed(i, 0, 0, 0, 0);
            }
        }
        strip.show();
    }
    
    if (!isDoorClosed()) {
        switchWhiteLedsOn();
    }
    
    if (whiteLedsOn && isDoorClosed()) {
        // Ensure the White LED is switched off if the door was opened but
        // is now closed.
        switchWhiteLedsOff();
    }
    
    updateFanSpeed(!isDoorClosed());
    
    delay(50);
}

void waitForDoorClose() {
    // TODO: Read switch input until or high...

    // switchPin is LOW when door is open.
    while (!digitalRead(switchPin)) {
        delay(10);
    }
    //Particle.publish("Status", "Door closed.", 60, PRIVATE);
}

// Timer.
void countDownTimerTick() {
    lightsOffInSeconds--;   
}

void startUVTimer() {
    // TODO: Use EEPROM value
    lightsOffInSeconds = 120;
    
    switchUVOn();
    switchWhiteLedsOff();
    switchFanOn();
    currentState = 1;
    
    // Update Neopixel clock to show full time
    for (int i=0; i<12; i++) {
        strip.setColorDimmed(i, 0, 255, 0, 128);
    } 
    strip.show();
            
    countDownTimer.start();
    
    Particle.publish("Status", "Timer Started", 60, PRIVATE);
}

void stopUVTimer() {
    countDownTimer.stop();
    
    // UV Exposure finished. Now waiting for user to open door to 
    // remove their thing.
    currentState = 10;
    
    switchUVOff();
    switchFanOff();
    
    for (int i=0; i<12; i++) {
        strip.setColorDimmed(i, 0, 0, 0, 0);
    }
    strip.show();
    
    // TODO: Use EEPROM value
    if (lightsOffInSeconds = 0) {
        Particle.publish("Status", "UV Exposure Complete.", 60, PRIVATE);
    } else {
        Particle.publish("Status", "Timer Stopped", 60, PRIVATE);
    }
}

void switchUVOn() {
    if (uvLedsOn) {
        return;
    }
    Particle.publish("Status", "UV On", 60, PRIVATE);
    digitalWrite(uvString1Pin, true);
    digitalWrite(uvString2Pin, true);
    uvLedsOn = true;
}

void switchUVOff() {
    if (!uvLedsOn) {
        return;
    }
    Particle.publish("Status", "UV Off", 60, PRIVATE);
    digitalWrite(uvString1Pin, false);
    digitalWrite(uvString2Pin, false);
    uvLedsOn = false;
}

void switchWhiteLedsOn() {
    if (whiteLedsOn) {
        return;
    }
    digitalWrite(whiteLedsPin, true);
    whiteLedsOn = true;
    Particle.publish("Status", "White LEDs On", 60, PRIVATE);
}

void switchWhiteLedsOff() {
    if (!whiteLedsOn) {
        return;
    }
    digitalWrite(whiteLedsPin, false);
    whiteLedsOn = false;
    Particle.publish("Status", "White LEDs Off", 60, PRIVATE);
}

void updateFanSpeed(bool doorOpen) {
    // If the door is open, fan runs 100%.
    
    // If running 
        
    if (doorOpen) {
        if (!fanOn) {
            switchFanOn();
        }
        analogWrite(fanPin, 255, 25000);
        return;
    } 
    
    // If not currently in the timer then switch the fan off when the door is closed.
    if (currentState!=1) {
        //switchFanOff();
        return;
    } 
    
    // If within the first x of the timer, keep the fan running low.
            
    // After a preset time the vapoures should have gone
    // so we can kill the fan.
    if (lightsOffInSeconds < 10) { // 90
        switchFanOff();
        return;
    }
    
    // Otherwise run the fan slowly
    analogWrite(fanPin, 80, 25000);

}

void switchFanOn() {
    if (fanOn) {
        return;
    }
    Particle.publish("Status", "Fan On", 60, PRIVATE);
    
    // Start it fast to ensure it starts.
    analogWrite(fanPin, 255);

    //digitalWrite(fanPin, true);
    fanOn = true;
}

void switchFanOff() {
    if (!fanOn) {
        return;
    }
    Particle.publish("Status", "Fan Off", 60, PRIVATE);
    digitalWrite(fanPin, false);
    fanOn = false;
}

bool isDoorClosed() {
    // High indicates door is closed.
    return digitalRead(switchPin);   
}

// *******************************************************
// Sensor reading
// *******************************************************
void takeMeasurements() {
    if (BoardVersion >= 2) {
        // Read the temperature.
        // Show the temperature from the first sensor.
        showTemperature(sensor);
        showCurrent();
        readVin();
        
        Particle.publish("senml", "{e:[{'n':'boardTemperature','v':'" + String(temperatureCelsius) + "'},{'n':'current','v':'" + String(currentMilliAmps) + "'},{'n':'supplyVoltage','v':'" + String(supplyVoltage) + "'},{'n':'lightState','v':'" + String(currentState) + "'},{'n':'vin','v':'" + String(voltsIn) + "'}]}", 60, PRIVATE);
    }
}

void showCurrent () {
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;

  float shuntvoltagemV = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V(); // at VIN- pin.
  // Adafruit library assumes 0.1R.
  current_mA = (ina219.getCurrent_mA() * 1);
  loadvoltage = busvoltage + (shuntvoltagemV / 1000);
  
  Particle.publish("Status", "Bus: " + String(busvoltage) + "V, Supply: " + String(loadvoltage) + "V, Current: " + String(current_mA) + " mA");
  
  currentMilliAmps = current_mA;
  supplyVoltage = loadvoltage;
  
  //Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
  //Serial.print("Shunt Voltage: "); Serial.print(shuntvoltagemV); Serial.println(" mV");
  //Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
  //Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
  //Serial.println("");
}

void listTemperatureSensors() {
  //Serial.println("------------------------------------------");
  //Serial.println("Sensors Discovered:");
  
  int sensorNumber = 0;
    
  do {   
    byte addr[8];
    
    if ( !ds.search(addr)) {
      ds.reset_search();
      return;
    }
  
    //Serial.print("ROM =");
    byte i;
    for( i = 0; i < 8; i++) {
      //Serial.write(' ');
      //Serial.print(addr[i], HEX);
      sensor[i] = addr[i];
    }
  
    if (OneWire::crc8(addr, 7) != addr[7]) {
        Serial.println("CRC is not valid!");
        return;
    } 
    
    // the first ROM byte indicates which chip
    switch (addr[0]) {
      case 0x10:
        Particle.publish("Status", "Found  DS18S20 ");
        break;
      case 0x28:
        Particle.publish("Status", "Found  DS18B20 :-) ");
        break;
      case 0x22:
        Particle.publish("Status", "Found  DS1822");
        break;
      default:
        Particle.publish("Status", "Found non a DS18x20 family device ");
        return;
    } 
  } while (true);
}

void showTemperature(byte sensorAddress[]) {
    byte i;
    byte present = 0;
    byte data[12];

    //  showAddress(sensorAddress); 

    // Start conversion.  
    ds.reset();
    ds.select(sensorAddress);
    ds.write(0x44);

    delay(1000);   // Delay to ensure conversion has happened. This might be update 750ms for 12bit. 375 (11 bit), 187 (10bit), 93 (9 bit)

    // Read conversion
    present = ds.reset();
    ds.select(sensorAddress);    
    ds.write(0xBE);         // Read Scratchpad
  
      // Read data
    for ( i = 0; i < 9; i++) {           // we need 9 bytes
        data[i] = ds.read();
    }
    //showData( present, data);
  
    //Serial.print(" CRC=");
    //Serial.print(OneWire::crc8(data, 8), HEX);
    //Serial.println();
  
    temperatureCelsius = computeTemperature(data);
    
    //Particle.publish("Status", "Temperature read as " + String(temperatureCelsius));

    return;
}

float computeTemperature(byte data[]) {
  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];

  byte cfg = (data[4] & 0x60);
  // at lower res, the low bits are undefined, so let's zero them
  if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
  else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
  else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
  //// default is 12 bit resolution, 750 ms conversion time
    
  float celsius;
  celsius = (float)raw / 16.0;
  return celsius;
}

void readVin() {
    int vInAdc = analogRead(vInPin);
    // convert to ADC bits to millivolts
    // then x2 as it's a potential divider.
    voltsIn = (vInAdc * 0.8 * 2);
}

// *******************************************************
// Neopixel functions
// *******************************************************
void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
    //for(i=0; i<4; i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

// *******************************************************
// Particle function functions (i.e. Internet exposed
// *******************************************************)

// Args: "Fan", "UV", "White"
int on(String args) {
    
    if (args == "Fan") {
        switchFanOn();
        return 1;
    } else if (args == "UV") {
        switchUVOn();
        return 2;
    } else if (args == "White") {
        switchWhiteLedsOn();
        return 3;
    }
    
    return 255;
}
// Args: "Fan", "UV", "White"
int off(String args) {
    
    if (args == "Fan") {
        switchFanOff();
        return 1;
    } else if (args == "UV") {
        switchUVOff();
        return 2;
    } else if (args == "White") {
        switchWhiteLedsOn();
        return 3;
    }

    return 0;
}

int startTimer(String args) {
    Particle.publish("Status", "Starting timer");
    startUVTimer();
    return 1;
}

int stopTimer(String args) {
    Particle.publish("Status", "Stopping timer");
    stopUVTimer();
    return 1;
}

// arg should be timer time.
int setTimer(String args) {
    Particle.publish("Status", "Setting timer time to: ...");
    
    return 0;
}

// *******************************************************
// Helpers
// *******************************************************


// *******************************************************
// Interrup service routines.
// *******************************************************
void doorOpenedIsr() {
    doorOpened = true;
}

// ISR for PIR sensor.
void upperDoorOpenedIsr() {
    trayDoorOpened = true;
}