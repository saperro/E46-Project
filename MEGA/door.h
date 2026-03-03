
#ifndef DOOR_H
#define DOOR_H

#include <Wire.h>
#include <Arduino.h>

//const uint8_t DOOR_RESET_PIN PROGMEM = XX;
extern String currentDebugModule;
extern byte debugMode;

void door_debug_menu(){
     if (currentDebugModule == "door" || currentDebugModule == "DOOR") {
      Serial.println(F("==============DOOR=============="));
      Serial.println(F("--> MAIN - to main menu"));
      Serial.println(F("--> EXIT - to normal mode"));
      currentDebugModule = "dupa";
    }
}
#endif 
