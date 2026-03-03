#ifndef MSWF_H
#define MSWF_H

#include <Wire.h>
#include <Arduino.h>

const uint8_t MSWF_RESET_PIN PROGMEM = 35;
extern unsigned long lastRequestTime1;
extern byte cruise;
extern String currentDebugModule;
extern byte debugMode;
extern unsigned long requestInterval1;

char receivedFromModuleSteeringWheelFunctional = ' ';
unsigned long lastMSWFtime = 0;// moduł ostatnio widziany
const unsigned long MSWFtimeoutInterval = 300000; // ile minut do alarm po odłązeniu modułu

void i2cMSWF() {
  char MSWF_byte1, MSWF_byte2, MSWF_byte3, MSWF_byte4, MSWF_byte5, MSWF_byte6, MSWF_byte7, MSWF_byte8, MSWF_byte9, MSWF_byte10, MSWF_byte11, MSWF_byte12;
  Wire.requestFrom(10, 12); // Oczekujemy 12 bajtów
  MSWF_byte1 = Wire.read();
  MSWF_byte2 = Wire.read();
  MSWF_byte3 = Wire.read();
  MSWF_byte4 = Wire.read();
  MSWF_byte5 = Wire.read();
  MSWF_byte6 = Wire.read();
  MSWF_byte7 = Wire.read();
  MSWF_byte8 = Wire.read();
  MSWF_byte9 = Wire.read();
  MSWF_byte10 = Wire.read();
  MSWF_byte11 = Wire.read();
  MSWF_byte12 = Wire.read();
  if (MSWF_byte1 == 'W') {
    if (currentDebugModule == "mswf" || currentDebugModule == "MSWF"){
    Serial.println(F("MSWF SYGNAŁ ZMNIEJSZENIA PRĘDKOŚCI TEMPOMATU O 1KM/H" ));
  }
    //
  }
  if (MSWF_byte1 == 'B') {
    if (currentDebugModule == "mswf" || currentDebugModule == "MSWF"){
    Serial.println(F("MSWF SYGNAŁ ZMNIEJSZENIA PRĘDKOŚCI TEMPOMATU O 5KM/H" ));
    }
  }
  if (MSWF_byte1 == 'K') {
    //
  }
  if (MSWF_byte2 == 'W') {
    if (currentDebugModule == "mswf" || currentDebugModule == "MSWF"){
    Serial.println(F("MSWF SYGNAŁ WYŁĄCZENIA TEMPOMATU" ));
    }
    cruise = 0;
    //
  }
  if (MSWF_byte2 == 'K') {
    //
  }
  if (MSWF_byte3 == 'W') {
    if (currentDebugModule == "mswf" || currentDebugModule == "MSWF"){
    Serial.println(F("MSWF SYGNAŁ ZWIĘKSZENIA PRĘDKOŚCI TEMPOMATU O 1KM/H" ));
    }
    //
  }
  if (MSWF_byte3 == 'B') {
    if (currentDebugModule == "mswf" || currentDebugModule == "MSWF"){
    Serial.println(F("MSWF SYGNAŁ ZWIĘKSZENIA PRĘDKOŚCI TEMPOMATU O 5KM/H" ));
    }
    //
  }
  if (MSWF_byte3 == 'K') {
    //
  }
  if (MSWF_byte4 == 'W') {
    if (currentDebugModule == "mswf" || currentDebugModule == "MSWF"){
    Serial.println(F("MSWF SYGNAŁ WŁĄCZENIA TEMPOMATU" ));
    }
    cruise = 1;
    //
  }
  if (MSWF_byte4 == 'K') {
    //
  }
  if (MSWF_byte5 == 'W') {
    if (currentDebugModule == "mswf" || currentDebugModule == "MSWF"){
    Serial.println(F("MSWF SYGNAŁ VOLUME-" ));
    }
    //
  }
  if (MSWF_byte5 == 'K') {
    //
  }
  if (MSWF_byte6 == 'W') {
    if (currentDebugModule == "mswf" || currentDebugModule == "MSWF"){
    Serial.println(F("MSWF SYGNAŁ MENU DOWN" ));
    }
    //
  }
  if (MSWF_byte6 == 'K') {
    //
  }

  if (MSWF_byte7 == 'W') {
    if (currentDebugModule == "mswf" || currentDebugModule == "MSWF"){
    Serial.println(F("MSWF SYGNAŁ MUTE" ));
    }
    lastMSWFtime = millis();//jesli odebrano W lub K zrestartuj czas diagnostyki MSWF
  }
  if (MSWF_byte7 == 'B') {
    if (currentDebugModule == "mswf" || currentDebugModule == "MSWF"){
    Serial.println(F("MSWF SYGNAŁ UNMUTE" ));
    }
    lastMSWFtime = millis();//jesli odebrano W lub K zrestartuj czas diagnostyki MSWF
  }
  if (MSWF_byte7 == 'K') {
    lastMSWFtime = millis();//jesli odebrano W lub K zrestartuj czas diagnostyki MSWF
    //
  }
  if (MSWF_byte8 == 'W') {
    if (currentDebugModule == "mswf" || currentDebugModule == "MSWF"){
    Serial.println(F("MSWF SYGNAŁ ENTER" ));
    }
    //
  }
  if (MSWF_byte8 == 'B') {
    if (currentDebugModule == "mswf" || currentDebugModule == "MSWF"){
    Serial.println(F("MSWF SYGNAŁ PRZEŁĄCZENIA FUNKCJI" ));
    }
    //
  }
  if (MSWF_byte8 == 'K') {
    //
  }
  if (MSWF_byte9 == 'W') {
  if (currentDebugModule == "mswf" || currentDebugModule == "MSWF"){
    Serial.println(F("MSWF SYGNAŁ CONNECT" ));
  }
    //
  }
  if (MSWF_byte9 == 'B') {
    if (currentDebugModule == "mswf" || currentDebugModule == "MSWF"){
    Serial.println(F("MSWF SYGNAŁ DISCONNECT" ));
    }
    //
  }
  if (MSWF_byte9 == 'K') {
    //
  }
  if (MSWF_byte10 == 'W') {
    if (currentDebugModule == "mswf" || currentDebugModule == "MSWF"){
    Serial.println(F("MSWF SYGNAŁ MENU UP" ));
  }
  }
  if (MSWF_byte10 == 'K') {
    //
  }
  if (MSWF_byte11 == 'W') {
    if (currentDebugModule == "mswf" || currentDebugModule == "MSWF"){
    Serial.println(F("MSWF SYGNAŁ VOLUME+" ));
    }
    //
  }
  if (MSWF_byte11 == 'K') {
    //
  }
  if (MSWF_byte12 == 'W') {
    //
  }
  if (MSWF_byte12 == 'K') {
    //
  }

  if (millis() - lastMSWFtime >= MSWFtimeoutInterval) {// jeśli od dłuższego czasu nie odebrano nic z MSWF, obsłuż błąd
    cruise = !cruise;
    digitalWrite(MSWF_RESET_PIN, HIGH);
    delay(10);
    digitalWrite(MSWF_RESET_PIN, LOW);
    Serial.println(F("MSWF RESET Z POWODU PRZEKROCZENIA CZASU ODPOWIEDZI"));
    lastMSWFtime = millis();
  }
 if (millis() - lastRequestTime1 >= requestInterval1) {
    lastRequestTime1 = millis();
    Serial.print(F("From Nano MSWF: "));
    Serial.print(MSWF_byte1);
    Serial.print(MSWF_byte2);
    Serial.print(MSWF_byte3);
    Serial.print(MSWF_byte4);
    Serial.print(MSWF_byte5);
    Serial.print(MSWF_byte6);
    Serial.print(MSWF_byte7);
    Serial.print(MSWF_byte8);
    Serial.print(MSWF_byte9);
    Serial.print(MSWF_byte10);
    Serial.print(MSWF_byte11);
    Serial.println(MSWF_byte12);
  }
}


void mswf_debug_menu() {
  if (currentDebugModule == "mswf" || currentDebugModule == "MSWF") {
    Serial.println(F("==============MSWF=============="));
    Serial.println(F("HOLD SWITCH AND REFRESH TYPING \"MSWF\""));
    requestInterval1 = 1; //reset zegara, żeby wywołać wypisanie zmiennych
    i2cMSWF();
    
    Serial.println(F("--> MSWF - refresh"));
    Serial.println(F("--> MSWFRESET - restart module"));
    Serial.println(F("--> MAIN - to main menu"));
    Serial.println(F("--> EXIT - to normal mode"));
    delay (2000);
  }
  if (currentDebugModule == "MSWFRESET" || currentDebugModule == "mswfreset") {
    Serial.println(F("Restarting MSWF module"));
    digitalWrite(MSWF_RESET_PIN, HIGH);
    delay(50);
    digitalWrite(MSWF_RESET_PIN, LOW);
    //delay(1000);
    currentDebugModule = "main";
  }
}
#endif
