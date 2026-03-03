#ifndef DEBUG_MENU_H
#define DEBUG_MENU_H
#include "sd_card.h"
#include <Arduino.h>

byte debugMode = 0;
String currentDebugModule = "dupa";
bool subMenuDisplayed = false;

void debugMenu() {
  // Sprawdzanie komend serial, oczekiwanie na "debug" / "DEBUG"
  if (Serial.available() > 0) {
    String word = Serial.readString();
    word.trim(); // ważne - usuwa znaki nowej linii

    if ((word == "debug") || (word == "DEBUG")) {
      debugMode = 1;
      currentDebugModule = "main";
      Serial.println(F("Debug mode enabled"));
    }
  }
  if (debugMode == 1) {
    if (currentDebugModule == "main") {
      Serial.println(F("\n=============DEBUG MODE============="));
      Serial.println(F("TYPE NAME OF A MODULE EG.\"MCC\" AND PRESS ENTER"));
      Serial.println(F("1 MPC "));
      Serial.println(F("2 EMS"));
      Serial.println(F("3 DOOR"));
      Serial.println(F("4 IKE"));
      Serial.println(F("5 MLC"));
      Serial.println(F("6 MSWF"));
      Serial.println(F("7 MCC"));
      Serial.println(F("8 MULTIMEDIA"));
      Serial.println(F("9 MTC"));
      Serial.println(F("10 MAS"));
      Serial.println(F("11 ATCM"));
      Serial.println(F("12 CANRAW"));
      Serial.println(F("13 CANREAD"));
      Serial.println(F("14 SD"));
      Serial.println(F("15 EXIT"));
      currentDebugModule = "dupa";
    }

    if (Serial.available()) {
      currentDebugModule = Serial.readStringUntil('\n');
      currentDebugModule.trim();

      if (currentDebugModule == "main") {
        Serial.println(F("\n=======DEBUG MODE======="));
        Serial.println(F("TYPE NAME OF A MODULE EG.\"MCC\" AND PRESS ENTER"));
        Serial.println(F("1 MPC "));
        Serial.println(F("2 EMS"));
        Serial.println(F("3 DOOR"));
        Serial.println(F("4 IKE"));
        Serial.println(F("5 MLC"));
        Serial.println(F("6 MSWF"));
        Serial.println(F("7 MCC"));
        Serial.println(F("8 MULTIMEDIA"));
        Serial.println(F("9 MTC"));
        Serial.println(F("10 MAS"));
        Serial.println(F("11 ATCM"));
        Serial.println(F("12 CANRAW"));
        Serial.println(F("13 CANREAD"));
        Serial.println(F("14 SD"));
        Serial.println(F("15 EXIT"));
        currentDebugModule = "dupa";
      }
      atcm_debug_menu();
      door_debug_menu();
      emp_debug_menu();
      ems_debug_menu();
      mas_debug_menu();
      mcc_debug_menu();
      mlc_debug_menu();
      mpc_debug_menu();
      mswf_debug_menu();
      mtc_debug_menu();
      multimedia_debug_menu();
      ike_debug_menu();
      can_debug_menu();
      sd_card_debug_menu();

      if (currentDebugModule == "exit" || currentDebugModule == "EXIT") {
        debugMode = 0;
        currentDebugModule = "dupa";
        Serial.println(F("Exiting debug mode - returning to normal operation"));
      }
      if (currentDebugModule == "main" || currentDebugModule == "MAIN") {
        currentDebugModule = "main";
      }
      else {
        Serial.print(F("Unknown command: "));
        Serial.println(currentDebugModule);
        Serial.println(F("Type 'main' for menu or 'exit' to quit debug mode"));
        currentDebugModule = "dupa";
      }
    }
  }
}

#endif
