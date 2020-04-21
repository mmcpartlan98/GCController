//
//  BLE_SP.h
//
//  Created by Matt Mcpartlan on 4/25/18.
//  Copyright © 2018 Matt Mcpartlan. All rights reserved.
//
// Example transmission: "000 000 333 100"
// Transmission length: 15

#ifndef USB_COMM_h
#define USB_COMM_h

#include "Arduino.h"

class USB_COMM {
public:
    USB_COMM(int baud);
    bool inputHandler(String strObjInput);
    bool setTemps(int temp1, int temp2, int temp3);
    bool checkForMsg();
    int getTemp1();
    int getTemp2();
    int getTemp3();
    int getSetTemp1();
    int getSetTemp2();
    int getSetTemp3();
    int getOpCode();
    int getExitCode();
    String to3String(double number);
    String generateTransmissionString(int opCode);
    void setRealTemps(int set1, int set2, int set3);
    void setDesiredTemps(int set1, int set2, int set3);
private:
    // Pin assignments
    int _SCLK;
    int _MISO;
    int _THERM1;
    int _THERM2;
    int _THERM3;

    // Temperature storage
    int _setTemp1 = 0;
    int _setTemp2 = 0;
    // Do NOT use setTemp3, the hardware does not support it (yet)
    int _setTemp3 = 0;
    int _realTemp1 = 0;
    int _realTemp2 = 0;
    int _realTemp3 = 0;

    // Arduino command codes (recieved by Arduino):
      // 000 - Verify set temperature
      //       Raspberry pi should call this once to make that the Arduino understood the command
      // 001 - Read temperature
      //         Raspberry pi should call this to ask for the current sensor readings
      // 002 - Error
      //         Arduino will return error if unable to execute raspberry pi commands (usually overtemperature protection)
    int _opCode = 0;
    int _exitCode = -1;
};

#endif
