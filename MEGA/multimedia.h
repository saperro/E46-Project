#ifndef MULTIMEDIA_H
#define MULTIMEDIA_H

#include <Wire.h>
#include <Arduino.h>

extern String currentDebugModule;
extern byte debugMode;

void multimedia_debug_menu(){
     if (currentDebugModule == "multimedia" || currentDebugModule == "MULTIMEDIA") {
      Serial.println(F("==============MULTIMEDIA=============="));
      Serial.println(F("--> MAIN - to main menu"));
      Serial.println(F("--> EXIT - to normal mode"));
      currentDebugModule = "dupa";
    }
}
#endif 
