#include <Arduino.h>
#include <EEPROM.h>
#include <BTS7960.h>

/*
  Snoke Exhaust Controller

  This program use smoke sensor to drive an exhaust fan based on BTS7960B

  Author: Victor Barros Halla
  E-mail: victor.halla@gmail.com
  Date: 06/07/2022
  Version: 1.0

  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
  Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License
*/

// Setup PINS
const uint8_t SMOKE = A0;
const uint8_t L_EN = 7;
const uint8_t R_EN = 8;
const uint8_t L_PWM = 9;
const uint8_t R_PWM = 10;

// Setup Default Speed
int speed_val = 255;
// Setup Smoke sensor value
int sensor_val = 0;
// Ratio between Sensor and Speed
float speedRatio = 1;

const byte numChars = 32;
char receivedChars[numChars]; // an array to store the received data
const char HELP = 'h';
const char SHOW = 's';
const char MEM = 'n';
const char MIN_SP = 'i';
const char MAX_SP = 'a';
//const char MIN_VAL = 'b';
//const char MAX_VAL = 'e';
boolean newData = false;
int MIN_SP_addr = 0;
int MAX_SP_addr = 1;
int MIN_SP_value;
int MAX_SP_value;
//int MIN_VAL_addr = 2;
//int MAX_VAL_addr = 3;
String paramValue;

// Define Controller
BTS7960 motorController(L_EN, R_EN, L_PWM, R_PWM);

void setup() {
  // Setup Serial Port
  Serial.begin(9600);

  // Define Led Pin
  pinMode(LED_BUILTIN, OUTPUT);

  // Start Main Menu
  startMenu();

  // Enable Motor
  motorController.Enable();

  // Read Minimal Fan Speed
  MIN_SP_value = EEPROM.read(MIN_SP_addr);
  speed_val = MIN_SP_value;

  // Read Minimal Fan Speed
  MAX_SP_value = EEPROM.read(MAX_SP_addr);
}

void loop() {
  // Read Serial data
  recvWithEndMarker();
  showNewData();

  // Execute command based on serial data
  runCommand();

  // Define Speed Ratio
  speedRatio = (float)MAX_SP_value / 255;

  // Read Smoke Sensor
  sensor_val = analogRead(SMOKE)/4;

  // Calculate Speed
  speed_val = sensor_val * speedRatio;

  // Check min and max speed value
  if(speed_val < MIN_SP_value) speed_val = MIN_SP_value;
  if(speed_val > MAX_SP_value) speed_val = MAX_SP_value;

  // Start Motor
  motorController.TurnLeft(speed_val);

  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);
}

void startMenu() {
  Serial.println("Exhaust Fan Controller v1.0");
  Serial.println("Victor Halla <victor.halla@gmail.com>");
  Serial.println("=====================================");
  Serial.println("Type <h> for help...");  
}

void showHelp() {
  Serial.println("<s> show smoke sensor (0-1023) and fan (0-255)");
  Serial.println("<n> show memory values");
  Serial.println("<i speed> set minimal exhaust fan speed (0-255)");
  Serial.println("<a speed> set maximun exhaust fan speed (0-255)");
//  Serial.println("<b value> set minimal smoke value (0-255)");
//  Serial.println("<e value> set maximun smoke value (0-255)");
  Serial.println("<h> for available commands"); 
}

void recvWithEndMarker() {
  static byte ndx = 0;
  char endMarker = '\n';
  char rc;
 
  // if (Serial.available() > 0) {
  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();

    if (rc != endMarker) {
      receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    } else {
      receivedChars[ndx] = '\0'; // terminate the string
      ndx = 0;
      newData = true;
    }
  }
}

void showNewData() {
  if (newData == true) {
    Serial.print("console# ");
    Serial.println(receivedChars);
    //newData = false;
  }
}

void showSensors() {
  Serial.print("Smoke sensor: ");
  Serial.println(sensor_val);
  Serial.print("Exhaust Fan Ratio: ");
  Serial.println(speedRatio);
  Serial.print("Exhaust Fan Speed: ");
  Serial.println(speed_val);
}

void showMemory() {
  Serial.print("Minimal Exhaust Fan Speed Value: ");
  Serial.println(EEPROM.read(MIN_SP_addr));
  Serial.print("Maximum Exhaust Fan Speed Value: ");
  Serial.println(EEPROM.read(MAX_SP_addr));
  //Serial.print("Minimal Smoke Sensor Value: ");
  //Serial.println(EEPROM.read(MIN_VAL_addr));
  //Serial.print("Maximum Smoke Sensor Value: ");
  //Serial.println(EEPROM.read(MAX_VAL_addr));
}

void setMinSpeed() {
  Serial.print("Old Minimal Speed Value: ");
  Serial.println(EEPROM.read(MIN_SP_addr));
  EEPROM.write(MIN_SP_addr, paramValue.toInt());
  Serial.print("New Minimal Speed Value: ");
  Serial.println(EEPROM.read(MIN_SP_addr));
}

void setMaxSpeed() {
  Serial.print("Old Maximum Speed Value: ");
  Serial.println(EEPROM.read(MAX_SP_addr));
  EEPROM.write(MAX_SP_addr, paramValue.toInt());
  Serial.print("New Maximum Speed Value: ");
  Serial.println(EEPROM.read(MAX_SP_addr));
}

//void setMinSensor() {
  //Serial.print("Old Minimal Smoke Sensor Value: ");
  //Serial.println(EEPROM.read(MIN_VAL_addr));
  //EEPROM.write(MIN_VAL_addr, paramValue.toInt());
  //Serial.print("New Minimal Smoke Sensor Value: ");
  //Serial.println(EEPROM.read(MIN_VAL_addr));
//}

//void setMaxSensor() {
  //Serial.print("Old Maximum Smoke Sensor Value: ");
  //Serial.println(EEPROM.read(MAX_VAL_addr));
  //EEPROM.write(MAX_VAL_addr, paramValue.toInt());
  //Serial.print("New Maximum Smoke Sensor Value: ");
  //Serial.println(EEPROM.read(MAX_VAL_addr));
//}

void runCommand() {
  if (newData == true) {
    String myCommand(receivedChars);
    paramValue = myCommand.substring(2, 6);
    switch (receivedChars[0]) {
      case HELP:
        showHelp();
        break;
      case SHOW:
        showSensors();
        break;
      case MEM:
        showMemory();
        break;
      case MIN_SP:
        setMinSpeed();
        break;
      case MAX_SP:
        setMaxSpeed();
        break;
      //case MIN_VAL:
        //setMinSensor();
        //break;
      //case MAX_VAL:
        //setMaxSensor();
        //break;
    }
    newData = false;
  }
}
