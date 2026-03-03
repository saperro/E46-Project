#ifndef ATCM_H
#define ATCM_H
#include <Wire.h>
#include <Arduino.h>

//const uint8_t ATCM_RESET_PIN PROGMEM = xx; - wybrać i wpisać pin
extern String currentDebugModule;
extern byte debugMode;

void atcm_debug_menu(){
     if (currentDebugModule == "atcm" || currentDebugModule == "ATCM") {
      Serial.println(F("==============ATCM=============="));
      Serial.println(F("--> MAIN - to main menu"));
      Serial.println(F("--> EXIT - to normal mode"));
      currentDebugModule = "dupa";
    }
}
#endif 
