//
//  BLE_SP.cpp
//  BLE_SP
//
//  Created by Matt Mcpartlan on 4/29/18.
//  Copyright © 2018 Matt Mcpartlan. All rights reserved.
//
// This library goes with the Drink Machine 2.0 main logic board.
// It adds shift 16-bit register functionalty (implemented in the Shift class)
// and

#include "Arduino.h"
#include "USB_COMM.h"
#include <math.h>

// Params are PIN NUMBERS (SCLK and MISO for temp comms) (THERM1, THERM2, THERM3 for temp chip select)
USB_COMM::USB_COMM(int baud) {
  Serial.begin(baud);
};

bool USB_COMM::checkForMsg() {
  String incommingSignal = "";
  int stringIndex = 0;
  while (Serial.available() > 0) {
    incommingSignal = incommingSignal + (char) Serial.read();
    stringIndex += 1;
    if (stringIndex > 14) {
      // Clear the register after collecting data
      while (Serial.available()) {
        Serial.read();
      }
    }
  }
  return inputHandler(incommingSignal);
}

bool USB_COMM::inputHandler(String strObjInput) {
    char inputString[16];
    strObjInput.toCharArray(inputString, 16);
    if (strlen(inputString) != 15) {
        _exitCode = 2;
        return false;
    } else {
        int spaceCounter = 0;
        for (int i = 0; i < 15; i++) {
          // Look for spaces (ASCII #32)
            if ((int) inputString[i] == 32) {
                spaceCounter += 1;
            }
            if (((int) inputString[i] < (int) '0' || ((int) inputString[i] > (int) '9')) && (int) inputString[i] > (int) ' ') {
                // SPACES ARE NOT INTS
                _exitCode = 1;
                return false;
            }
        }
        if (spaceCounter != 3) {
            _exitCode = 2;
            return false;
        } else {
            int convert[3] = { 0, 0, 0 };
            int tempInt;

            // Read operation code
            convert[0] = inputString[0] - '0';
            convert[1] = inputString[1] - '0';
            convert[2] = inputString[2] - '0';
            _opCode = convert[0] * 100 + convert[1] * 10 + convert[2];

            // Read temp arguments
            int tempArrayIndex = 0;
            for (int i = 4; i < 15; i += 4) {
                convert[0] = inputString[i] - '0';
                convert[1] = inputString[i + 1] - '0';
                convert[2] = inputString[i + 2] - '0';
                tempInt = convert[0] * 100 + convert[1] * 10 + convert[2];
                if (_opCode == 0) {
                  // Set new desired temperatures
                  switch (tempArrayIndex) {
                    case 0:
                      _setTemp1 = tempInt;
                      break;
                      case 1:
                      _setTemp2 = tempInt;
                      break;
                      case 2:
                      // For now, this should always be 0
                      _setTemp3 = tempInt;
                      if (_setTemp3 != 0) {
                        _exitCode = 3;
                        Serial.println(generateTransmissionString(2));
                        _opCode = 0;
                        return false;
                      }
                    }
                }
                tempArrayIndex++;
            }
        }
    }
    Serial.println(generateTransmissionString(_opCode));
    return true;
}

int USB_COMM::getTemp1() {
    return _realTemp1;
}

int USB_COMM::getTemp2() {
    return _realTemp2;
}

int USB_COMM::getTemp3() {
    return _realTemp3;
}

int USB_COMM::getSetTemp1() {
    return _setTemp1;
}

int USB_COMM::getSetTemp2() {
    return _setTemp2;
}

int USB_COMM::getSetTemp3() {
    return _setTemp3;
}

void USB_COMM::setDesiredTemps(int set1, int set2, int set3) {
  _setTemp1 = set1;
  _setTemp2 = set2;
  _setTemp3 = set3;
}

void USB_COMM::setRealTemps(int set1, int set2, int set3) {
  _realTemp1 = set1;
  _realTemp2 = set2;
  _realTemp3 = set3;
}

int USB_COMM::getOpCode() {
    return _opCode;
}

int USB_COMM::getExitCode() {
    return _exitCode;
}

bool USB_COMM::setTemps(int temp1, int temp2, int temp3) {
  _setTemp1 = temp1;
  _setTemp2 = temp2;
  _setTemp3 = temp3;
  return true;
}

String USB_COMM::to3String(double number) {
  char hundreds = char((floor(number / 100.0) + 48));
  char tens = char((floor(number / 10.0) - ((hundreds - 48) * 10) + 48));
  char ones = char((number - ((hundreds - 48) * 100) - ((tens - 48) * 10) + 48));
  String output = "";
  output = output + hundreds + tens + ones;
  return output;
}

String USB_COMM::generateTransmissionString(int opCode) {
  String opCodeS;
  String arg1S;
  String arg2S;
  String arg3S;
    switch (opCode) {
      case 0:
        opCodeS = "000";
        arg1S = to3String(_setTemp1);
        arg2S = to3String(_setTemp2);
        arg3S = to3String(_setTemp3);
        break;
      case 1:
        opCodeS = "001";
        arg1S = to3String(_realTemp1);
        arg2S = to3String(_realTemp2);
        arg3S = to3String(_realTemp3);
        break;
      case 2:
        opCodeS = "002";
        arg1S = to3String(_exitCode);
        arg2S = "***";
        arg3S = "***";
    }
    String retStr = opCodeS + " " + arg1S + " " + arg2S + " " + arg3S;
    return retStr;
}
