/***************************************************************************
* File Name: SEN-39002_Lightning_Emulator.ino
* Processor/Platform: Arduino Uno R3 (tested)
* Development Environment: Arduino 1.6.5
*
* Designed for use with with Playing With Fusion Lightning "Emulator"
* (SEN-39002), which can be used with AMS Lightning Sensor AS3935 to test
* and develop your application.
*
*   SEN-39002 (universal applications)
*   ---> https://www.playingwithfusion.com/productview.php?pdid=55
*   SEN-39001 (universal applications)
*   ---> http://playingwithfusion.com/productview.php?pdid=22
*   SEN-39003 (qwiic lightning sensor)
*   ---> https://www.playingwithfusion.com/productview.php?pdid=135
*
* Copyright © 2017 Playing With Fusion, Inc.
* SOFTWARE LICENSE AGREEMENT: This code is released under the MIT License.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
* **************************************************************************
* REVISION HISTORY:
* Author        Date        Comments
* J. Steinlage  2017Jan17   Original release
* J. Steinlage  2024Feb25   Update to support additional I2C addresses
* N. Johnson    2025Apr26   Add selectable automatic test feature
* 
* Playing With Fusion, Inc. invests time and resources developing open-source
* code. Please support Playing With Fusion and continued open-source 
* development by buying products from Playing With Fusion!
*
* **************************************************************************
* APPLICATION SPECIFIC NOTES (READ THIS!!!):
* - This code configures an Arduino to interface with SEN-39002
*    - Configure Arduino (I2C, UART, etc)
*    - Perform initial verification and configuration of the DAC
* - I2C specific: This example uses the I2C interface via the Wire library
* 
* Circuit:
*    Arduino Uno   Arduino Mega  -->  SEN-39002
*    SDA:    SDA        SDA      -->  SDA
*    SCL:    SCL        SCL      -->  SCL
*    GND:    GND        ''       -->  GND
*    5V:     5V         ''       -->  VDD 
*    note1: VDD should match voltage of IO, and can be between 3.3 and 5V
*    note2: this board has been tested with 5V Arduinos
*    note3: download PWFusion_MCP4725 library from www.github.com/playingwithfusion to use with this code
**************************************************************************/
// I2C library - standard Wire lib
#include "Wire.h"
// include Playing With Fusion MCP4725 lib (https://github.com/PlayingWithFusion/PWFusion_MCP4725)
#include "PWFusion_MCP4725_12DAC.h"

// declare sensor object
#define DAC_ADD 0x62  // uses A1 version
#define DAC_ADD_ALT 0x64 // uses A2 version
PWFusion_MCP4725 dac0(DAC_ADD);
PWFusion_MCP4725 dac1(DAC_ADD_ALT);

// Test type selection
#define MANUAL_TEST       1
#define AUTOMATIC_TEST    0

void setup()
{
  Serial.begin(115200);
  Serial.println("Playing With Fusion: SEN-39002, Lightning Emulator Shield");
  
  // set up DAC
  dac0.begin();
  dac1.begin();

  // setup DAC, set DAC to 'off' and pulled low
  // DAC output / write to NV memory / power down DAC, pull to gnd
  dac0.setOutput(0,false,true);
  dac1.setOutput(0,false,true);
  
  // give the Arduino time to start up
  delay(100); 

  pinMode(9, INPUT_PULLUP);   // FAR PB
  pinMode(8, OUTPUT);         // FAR
  digitalWrite(8, HIGH); 
  pinMode(7, INPUT_PULLUP);   // MID PB
  pinMode(6, OUTPUT);         // MID
  digitalWrite(6, HIGH); 
  pinMode(5, INPUT_PULLUP);   // CLO PB
  pinMode(4, OUTPUT);         // CLO
  digitalWrite(4, HIGH); 

  uint8_t su_tim = 50;
  volatile uint8_t su_x = 4;
  volatile uint8_t su_cnt;
  // startup LED sequence
  for(su_cnt = 0; su_cnt < su_x; su_cnt++){
    digitalWrite(4, LOW);
    delay(su_tim);
    digitalWrite(4, HIGH);
    digitalWrite(6, LOW);
    delay(su_tim);
    digitalWrite(6, HIGH);
    digitalWrite(8, LOW);
    delay(su_tim);
    digitalWrite(8, HIGH);
    digitalWrite(6, LOW);
    delay(su_tim);
    digitalWrite(6, HIGH);
  }
  digitalWrite(4, LOW);
  delay(su_tim);
  digitalWrite(4, HIGH);
}

// as calibrated, this profile works from around 7cm and 15cm from inductor to inductor
uint16_t out_array[20] = { 103, 73, 52, 37, 27, 20, 15, 11, 9, 7, 6, 5, 4, 4, 4, 3, 3, 3, 3, 3};
volatile uint8_t i, j, j_cnt;

// Define j_cnt values for ranged strikes
int close = 3;
int mid = 2;
int far = 1;

void loop() {

  if(MANUAL_TEST) {

    while(digitalRead(5) & digitalRead(7) & digitalRead(9)); // Wait for any button push
    if (!digitalRead(5)) {
      emulate(close, 1);
    }
    else if(!digitalRead(7)) {
      emulate(mid, 1);
    }
    else {
      emulate(far, 1);
    }

  } 
  else if(AUTOMATIC_TEST) {

    // emulate(distance, number of strikes)
    // modify to customize an automated test
    emulate(far, 50);
    emulate(mid, 50);
    emulate(close, 50);
    emulate(far, 100);
    delay(10000);

  }

}

void emulate(int j_count, int requestedStrikes) {
  for (int a=0; a < requestedStrikes; a++) {
    // Turn on the porper LED
    switch (j_count) {
      case 1:
        digitalWrite(8, LOW); break;
      case 2:
        digitalWrite(6, LOW); break;
      case 3:
        digitalWrite(4, LOW); 
        Serial.println("Far Strike");
        break;
    }

    for (int j=0; j < j_count; j++) {
      for (int i=0; i < 19; i++) {
        dac0.setOutput(out_array[i], false, false);         // set new command value
        dac1.setOutput(out_array[i], false, false);         // set new command value
        delayMicroseconds(30);    // change drive every 30 microseconds
      }
    }
    delay(20); // end by delaying 4 milliseconds

    // turn off DAC
    dac0.setOutput(0, true, true);
    dac1.setOutput(0, true, true);
    // hold state for one sec - give sensor time to pick up, report signal
    delay(1000);
    // turn off all LEDs
    digitalWrite(4,HIGH);
    digitalWrite(6,HIGH);
    digitalWrite(8,HIGH);
    delay(100);
  }
  
}
