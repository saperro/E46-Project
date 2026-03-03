#ifndef MCC_H
#define MCC_H

#include <Wire.h>
#include <Arduino.h>

const uint8_t MCC_RESET_PIN PROGMEM = 33;
extern String currentDebugModule;
extern byte debugMode;

void mcc_debug_menu(){
     if (currentDebugModule == "mcc" || currentDebugModule == "MCC") {
      Serial.println(F("==============MCC=============="));
      Serial.println(F("--> MAIN - to main menu"));
      Serial.println(F("--> EXIT - to normal mode"));
      currentDebugModule = "dupa";
    }
}
#endif 
