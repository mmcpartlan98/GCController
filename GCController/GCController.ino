#include <SPI.h>
#include "USB_COMM.h"

#define THERM1 9
#define THERM2 7
#define THERM3 8
#define TRIAC1 5
#define TRIAC2 3
#define INTERRUPT_PIN 2
uint16_t blankSend = 0;
uint16_t digitMask = 0x1FFE;

// Delays are in microseconds
unsigned long maxDelay = 8500;
unsigned long ISRDelay1 = maxDelay;
unsigned long ISRDelay2 = maxDelay;
unsigned long ZCPeakOffset = 0;
volatile unsigned long lastZC = 0;
volatile int interruptCounter = 0;
volatile int shouldTrigger = 0;

int setTemp1;
int setTemp2;

double tempDelayTable[101];

USB_COMM thermalCont(9600);

void recordZC() {
  lastZC = micros();
  shouldTrigger = 1;
}

void setup() {
  // put your setup code here, to run once:
  pinMode(THERM1, OUTPUT);
  pinMode(THERM2, OUTPUT);
  pinMode(THERM3, OUTPUT);
  pinMode(TRIAC1, OUTPUT);
  pinMode(TRIAC2, OUTPUT);
  pinMode(INTERRUPT_PIN, INPUT);
  digitalWrite(THERM1, HIGH);
  digitalWrite(THERM2, HIGH);
  digitalWrite(THERM3, HIGH);

  Serial.begin(115200);
  SPI.begin();
  SPI.beginTransaction(SPISettings(400000, MSBFIRST, SPI_MODE1));
  delay(1000);

  for (int i = 0; i < 101; i++) {
    if (i < 50) {
      tempDelayTable[i] = 0;
    } else if (i < 100) {
      tempDelayTable[i] = 0.5 + ((double)i / 100 - 0.8);
    } else {
      tempDelayTable[i] = 1;
    }
  }

  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), recordZC, RISING);
}

void loop() {
  // Tell comm manager to look for message and update set temps if found
  thermalCont.checkForMsg();
  
  if (shouldTrigger == 1) {
    if (lastZC + ISRDelay1 + ZCPeakOffset <= micros()) {
      digitalWrite(TRIAC1, HIGH);
    }
    if (lastZC + ISRDelay2 + ZCPeakOffset <= micros()) {
      digitalWrite(TRIAC2, HIGH);
    }
    digitalWrite(TRIAC1, LOW);
    digitalWrite(TRIAC2, LOW);
    shouldTrigger = 0;
  }

  interruptCounter += 1;
  if (interruptCounter >= 20000) {
    digitalWrite(THERM1, LOW);
    int thermalSPI1 = ((uint16_t)SPI.transfer16(blankSend) & digitMask) >> 5;
    digitalWrite(THERM1, HIGH);
    digitalWrite(THERM2, LOW);
    int thermalSPI2 = ((uint16_t)SPI.transfer16(blankSend) & digitMask) >> 5;
    digitalWrite(THERM2, HIGH);
    digitalWrite(THERM3, LOW);
    int thermalSPI3 = ((uint16_t)SPI.transfer16(blankSend) & digitMask) >> 5;
    digitalWrite(THERM3, HIGH);
    
    // Tell the communication manager what the temps are...
    thermalCont.setRealTemps(thermalSPI1, thermalSPI2, thermalSPI3);

    // Refresh temperatures set by raspberry pi
    setTemp1 = thermalCont.getSetTemp1();
    setTemp2 = thermalCont.getSetTemp2();
    
    int lookupTableIndex1 = round(((double)thermalSPI1 / (double)setTemp1) * 100);
    int lookupTableIndex2 = round(((double)thermalSPI2 / (double)setTemp2) * 100);

    if (lookupTableIndex1 > 99) {
      ISRDelay1 = maxDelay * tempDelayTable[100];
    } else {
      ISRDelay1 = maxDelay * tempDelayTable[lookupTableIndex1];
    }

    if (lookupTableIndex2 > 99) {
      ISRDelay2 = maxDelay * tempDelayTable[100];
    } else {
      ISRDelay2 = maxDelay * tempDelayTable[lookupTableIndex2];
    }
  }
}
