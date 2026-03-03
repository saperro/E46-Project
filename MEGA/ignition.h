#ifndef IGNITION_H
#define IGNITION_H

#include <Arduino.h>

const uint8_t POSITION1_OUTPUT_PIN PROGMEM = 23;
const uint8_t POSITION3 PROGMEM = 24;
const uint8_t POSITION1 PROGMEM = 25;
const uint8_t POSITION2 PROGMEM = 26;
unsigned long lastPositionsTime = 0;
unsigned long lastStartProcedureTime = 0;
const unsigned long positionsInterval = 1000;

// Pozycje - używamy uint8_t
uint8_t position1State = 0;
uint8_t position2State = 0;
uint8_t position3State = 0;

// ============================================================================
// FUNKCJA USTALAJĄCA POZYCJĘ STACYJKI
// ============================================================================
void positions() {
  if (digitalRead(POSITION1) == LOW) {
    position1State = 1;
  }
  if (digitalRead(POSITION2) == LOW) {
    position2State = 1;
  }
  if (digitalRead(POSITION3) == LOW) {
    position3State = 1;
  }
  if (digitalRead(POSITION1) == HIGH) {
    position1State = 0;
  }
  if (digitalRead(POSITION2) == HIGH) {
    position2State = 0;
  }
  if (digitalRead(POSITION3) == HIGH) {
    position3State = 0;
  }
  if ((position1State == 1) && (position3State == 0)) {
    digitalWrite(POSITION1_OUTPUT_PIN, LOW);
  }
  if ((position1State == 0) || (position3State == 1)) {
    digitalWrite(POSITION1_OUTPUT_PIN, HIGH);
  }
  //   if ((position1State == 1) && (position3State == 1)) {
  //    digitalWrite(ACC_PIN, HIGH));
  //  }
  if (millis() - lastPositionsTime >= positionsInterval) {
    Serial.print(F("POZYCJA STACYJKI 1 2 3:"));
    Serial.print(position1State);
    Serial.print(position2State);
    Serial.println(position3State);
    lastPositionsTime = millis();
  }
}


#endif
