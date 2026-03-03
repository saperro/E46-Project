//KOMUNIKACJA Z MPC
//POZYCJE STACYJKI
#ifndef MPC_H
#define MPC_H

#include <Wire.h>
#include <Arduino.h>
#include "ignition.h"

//Piny
const uint8_t MPC_RESET_PIN PROGMEM = 34;


//Zmienne zewnętrzne
extern String currentDebugModule;
extern byte debugMode;
extern unsigned long lastRequestTime;
extern const unsigned long longrequestInterval;
extern const unsigned long requestInterval;
extern void positions();
extern uint8_t position1State;
extern uint8_t position2State;
extern uint8_t position3State;


// Zmienne MPC
char receivedFromModulePowerControl = ' ';
const unsigned long MPCtimeoutInterval = 300000; // 5 minut
unsigned long lastMPCtime = 0;
float MPC_voltage_aku_a = 0.0;
float MPC_voltage_aku_b = 0.0;
float MPC_voltage_aku_c = 0.0;
float MPC_voltage_alt_b = 0.0;
float MPC_voltage_ins_a = 0.0;
float MPC_voltage_ins_b = 0.0;
uint8_t MPC_system_mode = 0;
bool MPC_debug_mode_active = false;
bool MPC_alternator_high_efficiency = false;
bool MPC_communication_lost = false;
bool MPC_communication_ok = true;
bool MPC_system_active = false;
bool MPC_ems_wake_active = false;
bool MPC_door_open = false;
bool MPC_ems_feedback_active = false;
uint8_t MPC_ignition_position = 0;



// ============================================================================
// FUNKCJA KONWERSJI NUMERU TRYBU NA NAZWĘ
// ============================================================================
String getMPCModeName(uint8_t mode) {
  switch (mode) {
    case 0: return "OFF";
    case 1: return "POS1_STANDBY";
    case 2: return "POS2_RUNNING";
    case 3: return "START";
    default: return "RESERVED_" + String(mode);
  }
}
// ============================================================================
// FUNKCJA WYSYŁANIA POZYCJI STACYJKI DO MPC
// ============================================================================
void sendIgnitionToMPC() {
  static unsigned long lastIgnitionSend = 0;
  static uint8_t lastSentPosition = 255; // Wymuszenie pierwszego wysłania

  if (millis() - lastIgnitionSend >= 500) { // Wysyłaj co 500ms
    uint8_t current_position = 0;

    // Określ pozycję na podstawie hardware inputs
    if (position1State == 1 && position2State == 0 && position3State == 0) {
      current_position = 1; // Pozycja 1 - silnik nie pracuje
    } else if (position1State == 1 && position2State == 1 && position3State == 0) {
      current_position = 2; // Pozycja 2 - silnik pracuje
    } else if (position3State == 1) {
      current_position = 3; // Start - kręci rozrusznikiem
    } else {
      current_position = 0; // OFF
    }

    // Wyślij tylko jeśli się zmieniła
    if (current_position != lastSentPosition) {
      Wire.beginTransmission(0x20);
      Wire.write(0x02); // CMD_SET_IGNITION
      Wire.write(current_position);
      uint8_t result = Wire.endTransmission();

      if (result == 0) {
        Serial.print(F("MPC: Wysłano pozycję stacyjki: "));
        Serial.print(current_position);
        Serial.print(F(" ("));
        Serial.print(getMPCModeName(current_position));
        Serial.println(F(")"));
        lastSentPosition = current_position;
      } else {
        Serial.print(F("MPC: Błąd wysyłania pozycji stacyjki, kod: "));
        Serial.println(result);
      }
    }

    lastIgnitionSend = millis();
  }
}
// ============================================================================
// FUNKCJA i2cMPC()
// ============================================================================
void i2cMPC() {
  positions();//sprawdź stacyjkę
  sendIgnitionToMPC();// wyślij stan stacyjki do mpc
  // Wyślij komendę żądania danych
  Wire.beginTransmission(0x20);
  Wire.write(0x01); // CMD_GET_VOLTAGES
  Wire.endTransmission();

  delay(10); // Pauza na odpowiedź
  Wire.requestFrom(0x20, 16); // NANO wysyła dokładnie 16 bajtów

  if (Wire.available() >= 16) {
    // Odczytaj napięcia (2 bajty każde, LSB pierwsz Â)
    uint16_t voltage_aku_a_raw = Wire.read() | (Wire.read() << 8);
    uint16_t voltage_aku_b_raw = Wire.read() | (Wire.read() << 8);
    uint16_t voltage_aku_c_raw = Wire.read() | (Wire.read() << 8);
    uint16_t voltage_alt_b_raw = Wire.read() | (Wire.read() << 8);
    uint16_t voltage_ins_a_raw = Wire.read() | (Wire.read() << 8);
    uint16_t voltage_ins_b_raw = Wire.read() | (Wire.read() << 8);

    uint8_t status_byte = Wire.read();
    uint8_t flags_byte = Wire.read();

    // Konwertuj napięcia (NANO wysyła w setnych wolta)
    MPC_voltage_aku_a = voltage_aku_a_raw / 100.0;
    MPC_voltage_aku_b = voltage_aku_b_raw / 100.0;
    MPC_voltage_aku_c = voltage_aku_c_raw / 100.0;
    MPC_voltage_alt_b = voltage_alt_b_raw / 100.0;
    MPC_voltage_ins_a = voltage_ins_a_raw / 100.0;
    MPC_voltage_ins_b = voltage_ins_b_raw / 100.0;

    // Dekoduj status byte
    MPC_system_mode = status_byte & 0x0F;
    MPC_system_active = (status_byte & 0x80) != 0;
    MPC_ems_wake_active = (status_byte & 0x40) != 0;
    MPC_debug_mode_active = (status_byte & 0x20) != 0;
    MPC_alternator_high_efficiency = (status_byte & 0x10) != 0;

    // Dekoduj flags byte - POPRAWIONE
    MPC_door_open = (flags_byte & 0x01) != 0;
    MPC_ems_feedback_active = (flags_byte & 0x02) != 0;
    MPC_ignition_position = (flags_byte >> 2) & 0x0F; // Bity 2-5
    MPC_communication_lost = (flags_byte & 0x80) != 0;

    lastMPCtime = millis(); // Reset timeout
    MPC_communication_ok = true;

    // ========================================================================
    // ALARMY SĄ OBSŁUGIWANE PRZEZ NANO - MEGA TYLKO WYŚWIETLA
    // ========================================================================

    // Sprawdź czy NANO zgłasza alarmy poprzez niskie napięcia
    static bool alarm_aku_a_shown = false;
    static bool alarm_aku_b_shown = false;
    static bool alarm_aku_c_shown = false;

    if (MPC_voltage_aku_a < 12.3 && !alarm_aku_a_shown) {
      Serial.println(F("MPC ALARM: NISKIE NAPIĘCIE AKUMULATORA ROZRUCHOWEGO"));
      // showErrorDisplay("niskie_napiecie_aku_rozruchowego.jpg"));
      // playSound("gong.mp3"));
      alarm_aku_a_shown = true;
    } else if (MPC_voltage_aku_a >= 12.3) {
      alarm_aku_a_shown = false;
    }

    if (MPC_voltage_aku_b < 12.1 && !alarm_aku_b_shown) {
      Serial.println(F("MPC ALARM: NISKIE NAPIĘCIE AKUMULATORA SYSTEMOWEGO"));
      // showErrorDisplay("niskie_napiecie_aku_systemowego.jpg"));
      // playSound("gong.mp3"));
      alarm_aku_b_shown = true;
    } else if (MPC_voltage_aku_b >= 12.1) {
      alarm_aku_b_shown = false;
    }

    if (MPC_voltage_aku_c < 11.8 && !alarm_aku_c_shown) {
      Serial.println(F("MPC ALARM: NISKIE NAPIĘCIE AKUMULATORA POTRZYMUJĄCEGO"));
      // showErrorDisplay("niskie_napiecie_aku_potrzymujacego.jpg"));
      // playSound("gong.mp3"));
      alarm_aku_c_shown = true;
    } else if (MPC_voltage_aku_c >= 11.8) {
      alarm_aku_c_shown = false;
    }

    // Sprawdź pozycję stacyjki i awarie ładowania (tylko dla pozycji 2)
    uint8_t current_position = 0;
    if (position1State == 1 && position2State == 0 && position3State == 0) current_position = 1;
    else if (position1State == 1 && position2State == 1 && position3State == 0) current_position = 2;
    else if (position3State == 1) current_position = 3;

    static bool charging_system_alarm_shown = false;
    static bool charging_starter_alarm_shown = false;

    if (current_position == 2) { // Tylko gdy silnik pracuje
      // Awaria ładowania systemowego
      if ((MPC_voltage_ins_b <= MPC_voltage_aku_b || MPC_voltage_ins_b <= 13.6) && !charging_system_alarm_shown) {
        Serial.println(F("MPC ALARM: AWARIA ŁADOWANIA SYSTEMOWEGO"));
        // showErrorDisplay("awaria_ladowania_systemowego.jpg"));
        // playSound("gong.mp3"));
        charging_system_alarm_shown = true;
      } else if (MPC_voltage_ins_b > MPC_voltage_aku_b && MPC_voltage_ins_b > 13.6) {
        charging_system_alarm_shown = false;
      }

      // Awaria ładowania rozruchowego
      if ((MPC_voltage_ins_a <= MPC_voltage_aku_a || MPC_voltage_ins_a <= 13.6) && !charging_starter_alarm_shown) {
        Serial.println(F("MPC ALARM: AWARIA ŁADOWANIA ROZRUCHOWEGO"));
        // showErrorDisplay("awaria_ladowania_rozruchowego.jpg"));
        // playSound("gong.mp3"));
        charging_starter_alarm_shown = true;
      } else if (MPC_voltage_ins_a > MPC_voltage_aku_a && MPC_voltage_ins_a > 13.6) {
        charging_starter_alarm_shown = false;
      }
    }

  } else {
    Serial.println(F("MPC: Nie otrzymano odpowiedzi lub niepełne dane"));
    MPC_communication_ok = false;
  }

  // ========================================================================
  // TIMEOUT I AUTODIAGNOSTYKA
  // ========================================================================
  if (millis() - lastMPCtime >= MPCtimeoutInterval) {
    if (MPC_communication_ok) {
      Serial.println(F("MPC TIMEOUT - AWARIA KOMUNIKACJI Z MODUŁEM MPC"));
      // showErrorDisplay("awaria_modulu_MPC.jpg"));
      // playSound("gong.mp3"));

      // Mrugaj wszystkimi kontrolkami które są wyzwalane przez MPC
      static bool mpc_lamp_state = false;
      static unsigned long last_blink = 0;
      if (millis() - last_blink >= 500) {
        mpc_lamp_state = !mpc_lamp_state;
        // digitalWrite(BATTERY_LAMP_PIN, mpc_lamp_state));
        // digitalWrite(OTHER_MPC_CONTROLLED_LAMPS, mpc_lamp_state));
        last_blink = millis();
      }

      MPC_communication_ok = false;
    }
  }

  // ========================================================================
  // DEBUG OUTPUT
  // ========================================================================
  if (millis() - lastRequestTime >= requestInterval && currentDebugModule == "mpc") {
    lastRequestTime = millis();
    Serial.println(F("=== MPC STATUS ==="));
    Serial.print(F("Battery A: ")); Serial.print(MPC_voltage_aku_a); Serial.println(F("V"));
    Serial.print(F("Battery B: ")); Serial.print(MPC_voltage_aku_b); Serial.println(F("V"));
    Serial.print(F("Battery C: ")); Serial.print(MPC_voltage_aku_c); Serial.println(F("V"));
    Serial.print(F("Alternator B: ")); Serial.print(MPC_voltage_alt_b); Serial.println(F("V"));
    Serial.print(F("Installation A: ")); Serial.print(MPC_voltage_ins_a); Serial.println(F("V"));
    Serial.print(F("Installation B: ")); Serial.print(MPC_voltage_ins_b); Serial.println(F("V"));
    Serial.print(F("Mode: ")); Serial.println(getMPCModeName(MPC_system_mode));
    Serial.print(F("Door: ")); Serial.println(MPC_door_open ? "OPEN" : "CLOSED");
    Serial.print(F("EMS Feedback: ")); Serial.println(MPC_ems_feedback_active ? "ACTIVE" : "INACTIVE");
    Serial.print(F("Ignition Position: ")); Serial.println(MPC_ignition_position);
    Serial.print(F("Alternator B: ")); Serial.println(MPC_alternator_high_efficiency ? "HIGH EFFICIENCY" : "STANDARD");
    Serial.print(F("Communication: ")); Serial.println(MPC_communication_ok ? "OK" : "LOST");
    Serial.println(F("=================="));
  }
}

// ============================================================================
// FUNKCJA MPCautodiagnostic()
// ============================================================================
void MPCautodiagnostic() {
  // Wyślij pozycję stacyjki do MPC
  sendIgnitionToMPC();

  // Regularny health check - sprawdź czy MPC odpowiada
  static unsigned long lastMPCHealthCheck = 0;
  if (millis() - lastMPCHealthCheck >= 300000) { // Co 5 minut
    Wire.beginTransmission(0x20);
    Wire.write(0x01); // CMD_GET_VOLTAGES jako ping
    uint8_t result = Wire.endTransmission();

    if (result == 0) {
      // Spróbuj odczytać odpowiedź
      delay(10);
      Wire.requestFrom(0x20, 1);
      if (Wire.available()) {
        Wire.read(); // Odczytaj choć jeden bajt
        Serial.println(F("MPC Health Check: OK"));
        lastMPCtime = millis(); // Reset timeout
      } else {
        Serial.println(F("MPC Health Check: No response"));
      }
    } else {
      Serial.print(F("MPC Health Check: I2C Error "));
      Serial.println(result);
    }

    lastMPCHealthCheck = millis();
  }
}

// ============================================================================
// FUNKCJA RESETOWANIA MPC (jeśli masz pin resetu)
// ============================================================================
void resetMPC() {
  // Jeśli masz pin do sprzętowego resetu MPC
  
    digitalWrite(MPC_RESET_PIN, HIGH);
    delay(100);
    digitalWrite(MPC_RESET_PIN, LOW);
    //delay(1000);
    Serial.println(F("MPC hardware reset completed"));
  

  // Alternatywnie: programowy reset przez I2C
  Wire.beginTransmission(0x20);
  Wire.write(0x10); // CMD_RESET
  uint8_t result = Wire.endTransmission();

  if (result == 0) {
    Serial.println(F("MPC software reset command sent"));
  } else {
    Serial.println(F("MPC reset command failed"));
  }
}

void mpc_debug_menu(){
      if (currentDebugModule == "mpc" || currentDebugModule == "MPC") {
      Serial.println(F("==============MPC=============="));
      i2cMPC(); // Wyświetl aktualne pomiary
      
      // Wyświetl dodatkowe informacje o stanie MPC
      Wire.beginTransmission(0x20);
      Wire.write(0x01); // CMD_GET_VOLTAGES
      Wire.endTransmission();
      
      delay(10);
      Wire.requestFrom(0x20, 16);
      if (Wire.available() >= 16) {
        // Przeskocz 12 bajtów napięć (już wyświetlone w i2cMPC)
        for (int i = 0; i < 12; i++) Wire.read();
        
        // Odczytaj bajt statusu
        uint8_t statusByte = Wire.read();
        bool systemActive = (statusByte & 0x80) != 0;
        bool emsWakeActive = (statusByte & 0x40) != 0;
        bool debugModeActive = (statusByte & 0x20) != 0;
        bool highEfficiencyMode = (statusByte & 0x10) != 0;
        uint8_t systemMode = statusByte & 0x0F;
        
        Serial.print(F("System aktywny: ")); Serial.println(systemActive ? "TAK" : "NIE");
        Serial.print(F("EMS wake aktywny: ")); Serial.println(emsWakeActive ? "TAK" : "NIE");
        Serial.print(F("MPC debug mode: ")); Serial.println(debugModeActive ? "TAK" : "NIE");
        Serial.print(F("Alternator B: ")); Serial.println(highEfficiencyMode ? "WYSOKA WYDAJNOŚĆ" : "STANDARDOWA WYDAJNOŚĆ");
        Serial.print(F("Tryb systemu: ")); Serial.println(systemMode);
        
        // Odczytaj bajt flag
        uint8_t flagsByte = Wire.read();
        bool doorOpen = (flagsByte & 0x01) != 0;
        bool emsFeedback = (flagsByte & 0x02) != 0;
        uint8_t ignitionPos = (flagsByte >> 2) & 0x0F;
        bool commLost = (flagsByte & 0x80) != 0;
        
        Serial.print(F("Drzwi: ")); Serial.println(doorOpen ? "OTWARTE" : "ZAMKNIĘTE");
        Serial.print(F("EMS feedback: ")); Serial.println(emsFeedback ? "AKTYWNY" : "NIEAKTYWNY");
        Serial.print(F("Pozycja stacyjki: ")); Serial.println(ignitionPos);
        Serial.print(F("Komunikacja z MPC: ")); Serial.println(commLost ? "UTRACONA" : "OK");
      }
      
      Serial.println();
      Serial.println(F("--> MPC - refresh measurements"));
      Serial.println(F("--> MPCRESET - restart module (hardware reset)"));
      Serial.println(F("--> MPCSTOP - stop MPC operation (debug mode)"));
      Serial.println(F("--> MPCSTART - resume normal MPC operation"));
      Serial.println(F("--> RELAY12VSBON - turn ON relay +12VSB (D3)"));
      Serial.println(F("--> RELAY12VSBOFF - turn OFF relay +12VSB (D3)"));
      Serial.println(F("--> RELAYLOADON - turn ON relay LOAD (D13)"));
      Serial.println(F("--> RELAYLOADOFF - turn OFF relay LOAD (D13)"));
      Serial.println(F("--> RELAYABON - turn ON relay A/B (D9)"));
      Serial.println(F("--> RELAYABOFF - turn OFF relay A/B (D9)"));
      Serial.println(F("--> RELAYBYPASSON - turn ON relay BYPASS (D12)"));
      Serial.println(F("--> RELAYBYPASSOFF - turn OFF relay BYPASS (D12)"));
      Serial.println(F("--> RELALTSBON - turn ON alternator B selector (D8)"));
      Serial.println(F("--> RELALTSBOFF - turn OFF alternator B selector (D8)"));
      Serial.println(F("--> CHARGINGCRON - turn ON charging AKU C (D6)"));
      Serial.println(F("--> CHARGINGCROFF - turn OFF charging AKU C (D6)"));
      Serial.println(F("--> EMSWAKON - turn ON EMS wake (D4)"));
      Serial.println(F("--> EMSWAKOFF - turn OFF EMS wake (D4)"));
      Serial.println(F("--> MPCALLOFF - turn OFF all outputs"));
      Serial.println(F("--> MPCSTATUS - show all outputs status"));
      Serial.println(F("--> MPCBATTLAMPON - turn ON blinking battery lamp"));
      Serial.println(F("--> MPCBATTLAMPOFF - turn OFF blinking battery lamp"));
      Serial.println(F("------- BATTERY C CHARGING TIME -------"));
      Serial.println(F("--> SET-C-70 - set accu c charging time 70 minutes"));
      Serial.println(F("--> SET-C-90 - set accu c charging time 90 minutes"));
      Serial.println(F("--> SET-C-120 - set accu c charging time 120 minutes"));
      Serial.println(F("--> SET-C-150 - set accu c charging time 150 minutes"));
      Serial.println(F("------- AUTO OFF TIME -------"));
      Serial.println(F("--> SET-TIME-2 - set auto off time 2 minutes"));
      Serial.println(F("--> SET-TIME-5 - set auto off time 5 minutes"));
      Serial.println(F("--> SET-TIME-10 - set auto off time 10 minutes (default)"));
      Serial.println(F("--> SET-TIME-15 - set auto off time 15 minutes"));
      Serial.println(F("--> MAIN - to main menu"));
      Serial.println(F("--> EXIT - to normal mode"));
      delay(100);
      currentDebugModule = "dupa";
    }
    else if (currentDebugModule == "MPCRESET" || currentDebugModule == "mpcreset") {
      Serial.println(F("Restarting MPC module via hardware reset"));
      Wire.beginTransmission(0x20);
      Wire.write(0x10); // CMD_RESET - informacyjne, rzeczywisty reset przez tranzystor
      Wire.endTransmission();
      // Tu MEGA powinno aktywować tranzystor resetu MPC
      // digitalWrite(MPC_RESET_PIN, HIGH));
      // delay(100);
      // digitalWrite(MPC_RESET_PIN, LOW));
      delay(1000);
      Serial.println(F("MPC module restarted"));
      currentDebugModule = "main";
    }
    else if (currentDebugModule == "MPCSTOP" || currentDebugModule == "mpcstop") {
      Serial.println(F("Stopping MPC operation - entering debug mode"));
      Wire.beginTransmission(0x20);
      Wire.write(0x11); // CMD_DEBUG_MODE_ON
      Wire.endTransmission();
      Serial.println(F("MPC is now in debug mode - normal operation suspended"));
      Serial.println(F("MPC will start streaming diagnostic data automatically"));
      currentDebugModule = "mpc";
    }
    else if (currentDebugModule == "MPCSTART" || currentDebugModule == "mpcstart") {
      Serial.println(F("Resuming normal MPC operation"));
      Wire.beginTransmission(0x20);
      Wire.write(0x12); // CMD_DEBUG_MODE_OFF
      Wire.endTransmission();
      Serial.println(F("MPC resumed normal operation - debug mode disabled"));
      currentDebugModule = "mpc";
    }
    else if (currentDebugModule == "RELAY12VSBON" || currentDebugModule == "relay12vsbon") {
      Serial.println(F("Turning ON MPC Relay +12VSB (D3)"));
      Wire.beginTransmission(0x20);
      Wire.write(0x20); // CMD_OUTPUT_CONTROL
      Wire.write(3);    // D3 - REL_INV_12VSB
      Wire.write(0x01); // ON
      Wire.endTransmission();
      Serial.println(F("Relay +12VSB turned ON"));
      currentDebugModule = "mpc";
    }
    else if (currentDebugModule == "RELAY12VSBOFF" || currentDebugModule == "relay12vsboff") {
      Serial.println(F("Turning OFF MPC Relay +12VSB (D3)"));
      Wire.beginTransmission(0x20);
      Wire.write(0x20); // CMD_OUTPUT_CONTROL
      Wire.write(3);    // D3 - REL_INV_12VSB
      Wire.write(0x00); // OFF
      Wire.endTransmission();
      Serial.println(F("Relay +12VSB turned OFF"));
      currentDebugModule = "mpc";
    }
    else if (currentDebugModule == "RELAYLOADON" || currentDebugModule == "relayloadon") {
      Serial.println(F("Turning ON MPC Relay LOAD (D13)"));
      Wire.beginTransmission(0x20);
      Wire.write(0x20); // CMD_OUTPUT_CONTROL
      Wire.write(13);   // D13 - REL_LOAD
      Wire.write(0x01); // ON
      Wire.endTransmission();
      Serial.println(F("Relay LOAD turned ON"));
      currentDebugModule = "mpc";
    }
    else if (currentDebugModule == "RELAYLOADOFF" || currentDebugModule == "relayloadoff") {
      Serial.println(F("Turning OFF MPC Relay LOAD (D13)"));
      Wire.beginTransmission(0x20);
      Wire.write(0x20); // CMD_OUTPUT_CONTROL
      Wire.write(13);   // D13 - REL_LOAD
      Wire.write(0x00); // OFF
      Wire.endTransmission();
      Serial.println(F("Relay LOAD turned OFF"));
      currentDebugModule = "mpc";
    }
    else if (currentDebugModule == "RELAYABON" || currentDebugModule == "relayabon") {
      Serial.println(F("Turning ON MPC Relay A/B (D9)"));
      Wire.beginTransmission(0x20);
      Wire.write(0x20); // CMD_OUTPUT_CONTROL
      Wire.write(9);    // D9 - REL_A_B
      Wire.write(0x01); // ON
      Wire.endTransmission();
      Serial.println(F("Relay A/B turned ON"));
      currentDebugModule = "mpc";
    }
    else if (currentDebugModule == "RELAYABOFF" || currentDebugModule == "relayaboff") {
      Serial.println(F("Turning OFF MPC Relay A/B (D9)"));
      Wire.beginTransmission(0x20);
      Wire.write(0x20); // CMD_OUTPUT_CONTROL
      Wire.write(9);    // D9 - REL_A_B
      Wire.write(0x00); // OFF
      Wire.endTransmission();
      Serial.println(F("Relay A/B turned OFF"));
      currentDebugModule = "mpc";
    }
    else if (currentDebugModule == "RELAYBYPASSON" || currentDebugModule == "relaybypasson") {
      Serial.println(F("Turning ON MPC Relay BYPASS (D12)"));
      Wire.beginTransmission(0x20);
      Wire.write(0x20); // CMD_OUTPUT_CONTROL
      Wire.write(12);   // D12 - REL_BYPASS
      Wire.write(0x01); // ON
      Wire.endTransmission();
      Serial.println(F("Relay BYPASS turned ON"));
      currentDebugModule = "mpc";
    }
    else if (currentDebugModule == "RELAYBYPASSOFF" || currentDebugModule == "relaybypassoff") {
      Serial.println(F("Turning OFF MPC Relay BYPASS (D12)"));
      Wire.beginTransmission(0x20);
      Wire.write(0x20); // CMD_OUTPUT_CONTROL
      Wire.write(12);   // D12 - REL_BYPASS
      Wire.write(0x00); // OFF
      Wire.endTransmission();
      Serial.println(F("Relay BYPASS turned OFF"));
      currentDebugModule = "mpc";
    }
    else if (currentDebugModule == "RELALTSBON" || currentDebugModule == "relaltsbon") {
      Serial.println(F("Turning ON MPC Alternator B Selector (D8)"));
      Wire.beginTransmission(0x20);
      Wire.write(0x20); // CMD_OUTPUT_CONTROL
      Wire.write(8);    // D8 - REL_SEL_B
      Wire.write(0x01); // ON
      Wire.endTransmission();
      Serial.println(F("Alternator B Selector turned ON (High Efficiency Mode)"));
      currentDebugModule = "mpc";
    }
    else if (currentDebugModule == "RELALTSBOFF" || currentDebugModule == "relaltsboff") {
      Serial.println(F("Turning OFF MPC Alternator B Selector (D8)"));
      Wire.beginTransmission(0x20);
      Wire.write(0x20); // CMD_OUTPUT_CONTROL
      Wire.write(8);    // D8 - REL_SEL_B
      Wire.write(0x00); // OFF
      Wire.endTransmission();
      Serial.println(F("Alternator B Selector turned OFF (Standard Mode)"));
      currentDebugModule = "mpc";
    }
    else if (currentDebugModule == "CHARGINGCRON" || currentDebugModule == "chargingcron") {
      Serial.println(F("Turning ON MPC Charging AKU C (D6)"));
      Wire.beginTransmission(0x20);
      Wire.write(0x20); // CMD_OUTPUT_CONTROL
      Wire.write(6);    // D6 - CHANGING_AKU_C
      Wire.write(0x01); // ON
      Wire.endTransmission();
      Serial.println(F("Charging AKU C turned ON"));
      currentDebugModule = "mpc";
    }
    else if (currentDebugModule == "CHARGINGCROFF" || currentDebugModule == "chargingcroff") {
      Serial.println(F("Turning OFF MPC Charging AKU C (D6)"));
      Wire.beginTransmission(0x20);
      Wire.write(0x20); // CMD_OUTPUT_CONTROL
      Wire.write(6);    // D6 - CHANGING_AKU_C
      Wire.write(0x00); // OFF
      Wire.endTransmission();
      Serial.println(F("Charging AKU C turned OFF"));
      currentDebugModule = "mpc";
    }
    else if (currentDebugModule == "EMSWAKON" || currentDebugModule == "emswakon") {
      Serial.println(F("Turning ON MPC EMS Wake (D4)"));
      Wire.beginTransmission(0x20);
      Wire.write(0x20); // CMD_OUTPUT_CONTROL
      Wire.write(4);    // D4 - PWR_ON_EMS
      Wire.write(0x01); // ON
      Wire.endTransmission();
      Serial.println(F("EMS Wake turned ON"));
      currentDebugModule = "mpc";
    }
    else if (currentDebugModule == "EMSWAKOFF" || currentDebugModule == "emswakoff") {
      Serial.println(F("Turning OFF MPC EMS Wake (D4)"));
      Wire.beginTransmission(0x20);
      Wire.write(0x20); // CMD_OUTPUT_CONTROL
      Wire.write(4);    // D4 - PWR_ON_EMS
      Wire.write(0x00); // OFF
      Wire.endTransmission();
      Serial.println(F("EMS Wake turned OFF"));
      currentDebugModule = "mpc";
    }
    else if (currentDebugModule == "MPCALLOFF" || currentDebugModule == "mpcalloff") {
      Serial.println(F("Turning OFF all MPC outputs"));
      Wire.beginTransmission(0x20);
      Wire.write(0x21); // CMD_ALL_OUTPUTS_OFF
      Wire.endTransmission();
      Serial.println(F("All MPC outputs turned OFF"));
      currentDebugModule = "mpc";
    }
    else if (currentDebugModule == "MPCBATTLAMPON" || currentDebugModule == "mpcbattlampon") {
      Serial.println(F("Turning ON MPC blinking battery lamp"));
      Wire.beginTransmission(0x20);
      Wire.write(0x32); // CMD_BATTERY_LAMP_BLINK_ON
      Wire.endTransmission();
      Serial.println(F("Battery lamp blinking started"));
      currentDebugModule = "mpc";
    }
    else if (currentDebugModule == "MPCBATTLAMPOFF" || currentDebugModule == "mpcbattlampoff") {
      Serial.println(F("Turning OFF MPC blinking battery lamp"));
      Wire.beginTransmission(0x20);
      Wire.write(0x33); // CMD_BATTERY_LAMP_BLINK_OFF
      Wire.endTransmission();
      Serial.println(F("Battery lamp blinking stopped"));
      currentDebugModule = "mpc";
    }
    else if (currentDebugModule == "SET-C-70" || currentDebugModule == "set-c-70") {
      Serial.println(F("Setting MPC charging time C to 70 minutes"));
      Wire.beginTransmission(0x20);
      Wire.write(0x31); // CMD_SET_CHARGING_TIME_C
      Wire.write(70);   // 70 minutes
      Wire.endTransmission();
      Serial.println(F("Charging time C set to 70 minutes and saved to EEPROM"));
      currentDebugModule = "mpc";
    }
    else if (currentDebugModule == "SET-C-90" || currentDebugModule == "set-c-90") {
      Serial.println(F("Setting MPC charging time C to 90 minutes"));
      Wire.beginTransmission(0x20);
      Wire.write(0x31); // CMD_SET_CHARGING_TIME_C
      Wire.write(90);   // 90 minutes
      Wire.endTransmission();
      Serial.println(F("Charging time C set to 90 minutes and saved to EEPROM"));
      currentDebugModule = "mpc";
    }
    else if (currentDebugModule == "SET-C-120" || currentDebugModule == "set-c-120") {
      Serial.println(F("Setting MPC charging time C to 120 minutes"));
      Wire.beginTransmission(0x20);
      Wire.write(0x31); // CMD_SET_CHARGING_TIME_C
      Wire.write(120);  // 120 minutes
      Wire.endTransmission();
      Serial.println(F("Charging time C set to 120 minutes and saved to EEPROM"));
      currentDebugModule = "mpc";
    }
    else if (currentDebugModule == "SET-C-150" || currentDebugModule == "set-c-150") {
      Serial.println(F("Setting MPC charging time C to 150 minutes"));
      Wire.beginTransmission(0x20);
      Wire.write(0x31); // CMD_SET_CHARGING_TIME_C
      Wire.write(150);  // 150 minutes
      Wire.endTransmission();
      Serial.println(F("Charging time C set to 150 minutes and saved to EEPROM"));
      currentDebugModule = "mpc";
    }
    else if (currentDebugModule == "SET-TIME-2" || currentDebugModule == "set-time-2") {
      Serial.println(F("Setting MPC auto off time to 2 minutes"));
      Wire.beginTransmission(0x20);
      Wire.write(0x30); // CMD_SET_AUTO_OFF_TIME
      Wire.write(2);    // 2 minutes
      Wire.endTransmission();
      Serial.println(F("Auto off time set to 2 minutes and saved to EEPROM"));
      currentDebugModule = "mpc";
    }
    else if (currentDebugModule == "SET-TIME-5" || currentDebugModule == "set-time-5") {
      Serial.println(F("Setting MPC auto off time to 5 minutes"));
      Wire.beginTransmission(0x20);
      Wire.write(0x30); // CMD_SET_AUTO_OFF_TIME
      Wire.write(5);    // 5 minutes
      Wire.endTransmission();
      Serial.println(F("Auto off time set to 5 minutes and saved to EEPROM"));
      currentDebugModule = "mpc";
    }
    else if (currentDebugModule == "SET-TIME-10" || currentDebugModule == "set-time-10") {
      Serial.println(F("Setting MPC auto off time to 10 minutes"));
      Wire.beginTransmission(0x20);
      Wire.write(0x30); // CMD_SET_AUTO_OFF_TIME
      Wire.write(10);   // 10 minutes
      Wire.endTransmission();
      Serial.println(F("Auto off time set to 10 minutes and saved to EEPROM"));
      currentDebugModule = "mpc";
    }
    else if (currentDebugModule == "SET-TIME-15" || currentDebugModule == "set-time-15") {
      Serial.println(F("Setting MPC auto off time to 15 minutes"));
      Wire.beginTransmission(0x20);
      Wire.write(0x30); // CMD_SET_AUTO_OFF_TIME
      Wire.write(15);   // 15 minutes
      Wire.endTransmission();
      Serial.println(F("Auto off time set to 15 minutes and saved to EEPROM"));
      currentDebugModule = "mpc";
    }
    else if (currentDebugModule == "MPCSTATUS" || currentDebugModule == "mpcstatus") {
      Serial.println(F("Requesting MPC outputs status"));
      Wire.beginTransmission(0x20);
      Wire.write(0x22); // CMD_GET_OUTPUTS_STATUS
      Wire.endTransmission();
      
      delay(50); // Czekaj na odpowiedź
      
      Wire.requestFrom(0x20, 1);
      if (Wire.available() ){
        uint8_t status = Wire.read();
        Serial.println(F("=== MPC OUTPUTS STATUS ==="));
        Serial.print(F("D3  REL_INV_12VSB:   ")); Serial.println((status & 0x08) ? "ON" : "OFF");
        Serial.print(F("D4  PWR_ON_EMS:      ")); Serial.println((status & 0x10) ? "ON" : "OFF");
        Serial.print(F("D6  CHARGING_AKU_C:  ")); Serial.println((status & 0x40) ? "ON" : "OFF");
        Serial.print(F("D8  REL_SEL_B:       ")); Serial.println((status & 0x20) ? "ON" : "OFF");
        Serial.print(F("D9  REL_A_B:         ")); Serial.println((status & 0x02) ? "ON" : "OFF");
        Serial.print(F("D12 REL_BYPASS:      ")); Serial.println((status & 0x04) ? "ON" : "OFF");
        Serial.print(F("D13 REL_LOAD:        ")); Serial.println((status & 0x01) ? "ON" : "OFF");
        Serial.println(F("=========================="));
        Serial.print(F("RAW STATUS: 0x")); Serial.println(status, HEX);
      } else {
        Serial.println(F("Failed to get outputs status - no response from MPC"));
      }
      currentDebugModule = "mpc";
    }
}

#endif
