#ifndef MAS_H
#define MAS_H

#include <Wire.h>
#include <Arduino.h>

const uint8_t MAS_RESET_PIN PROGMEM = 36;
extern String currentDebugModule;
extern byte debugMode;

void mas_debug_menu(){
     if (currentDebugModule == "mas" || currentDebugModule == "MAS") {
      Serial.println(F("==============MAS=============="));
      Serial.println(F("--> MAIN - to main menu"));
      Serial.println(F("--> EXIT - to normal mode"));
      currentDebugModule = "dupa";
    }
}
#endif 
