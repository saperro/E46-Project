#ifndef MTC_H
#define MTC_H

#include <Wire.h>
#include <Arduino.h>

extern String currentDebugModule;
extern byte debugMode;

void mtc_debug_menu(){
     if (currentDebugModule == "mtc" || currentDebugModule == "MTC") {
      Serial.println(F("==============MTC=============="));
      Serial.println(F("--> MAIN - to main menu"));
      Serial.println(F("--> EXIT - to normal mode"));
      currentDebugModule = "dupa";
    }
}
#endif 
