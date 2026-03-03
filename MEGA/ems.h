#ifndef EMS_H
#define EMS_H

#include <Wire.h>
#include <Arduino.h>
#include "k_bus.h"

const uint8_t EMS_RESET_PIN PROGMEM = 31;
extern String currentDebugModule;
extern byte debugMode;
extern uint8_t LightByte1;
extern uint8_t LightByte2;
extern uint8_t LightByte3;
extern uint8_t adress;
extern int counter;
extern uint8_t zmienna;
extern uint8_t zmienna1;
extern uint8_t state_counter;

char receivedFromElectronicModuleStandby = ' ';
unsigned long lastEMStime = 0;
const unsigned long EMStimeoutInterval = 300000;  // 5 minut

//==================STEROWANIE LAMPKAMI SPALONYCH ZAROWEK (OMIJA RESZTĘ KODU)- dobre do debug.
uint8_t zarowka_lewa_tyl = 0;
uint8_t zarowka_prawa_przod = 0;
uint8_t zarowka_prawa_tyl = 0;
uint8_t zarowka_wnetrze = 0;
void zarowki(byte lewa_przod, byte lewa_tyl, byte prawa_przod, byte prawa_tyl, byte wnetrze) {
  adress = 0xD0;
  LightByte3 = 0x00;
  //byte zmienna;
  if (wnetrze == 1) {
    zmienna = 0x80;
  }
  if (lewa_przod == 1) {
    zmienna += 0x20;
  }
  if (lewa_tyl == 1) {
    zmienna += 0x08;
  }
  if (prawa_przod == 1) {
    zmienna += 0x10;
  }
  if (prawa_tyl == 1) {
    zmienna += 0x04;
  }
  //else {
  //  LightByte2 = 0x00;
  // LightByte3 = 0x00;
  // LightByte1 = 0x00; -tego nie rusz bo wylaczysz inne kontrolki z innych funkcji
  //}
  LightByte2 = zmienna;

  //LightByte2 = 0x04;
  //0x00 off
  //0x80 samo wnętrze
  //0x04 wnętrze i tył prawo
  //0x08 wnętrze i tył lewo
  //0x10 wnętrze i przód prawy
  //0x20 wnętrze i lewy przód
  //0x24 wnętrze + lewy przód +prawy tył
  //0x28 wnętrze + lewy przód +lewy tył
  //0x30 wnętrze i oba przody
  //0x34 wnętrze + oba przód + prawy tył
  //0x38 wnętrze + oba przód + lewy tył
  //0x40 off
  //0x44 wnętrze i tył prawo
  //0x48 wnętrze i tył lewo
  //0x50 wnętrze i przód prawy
  //0x54 wnętrze + prawy przód +prawy tył
  //0x58 wnętrze + prawy przód +lewy tył
  //0x60 wnętrze i lewy przód
  //0x64 wnętrze + lewy przód +prawy tył
  //0x68 wnętrze + lewy przód +lewy tył
  //0x70 wnętrze i oba przody
  //0x74 wnętrze + oba przód + prawy tył
  //0x78 wnętrze + oba przód + lewy tył
}


void i2cDoorsEMS() {
  char EMS_byte1, EMS_byte2, EMS_byte3, EMS_byte4, EMS_byte5, EMS_byte6, EMS_byte7;
  Wire.requestFrom(9, 7); // Oczekujemy 7 bajtów
  EMS_byte1 = Wire.read();
  EMS_byte2 = Wire.read();
  EMS_byte3 = Wire.read();
  EMS_byte4 = Wire.read();
  EMS_byte5 = Wire.read();
  EMS_byte6 = Wire.read();
  EMS_byte7 = Wire.read();
  byte zmienna = 0;
  byte zmienna1 = 0;
  if (EMS_byte3 == 'W') {
    lastEMStime = millis();//jesli odebrano W lub K zrestartuj czas diagnostyki MLC
    zmienna += 0x01;
  }
  if (EMS_byte3 == 'K') {
    lastEMStime = millis();
  }
  if (EMS_byte4 == 'W') {
    zmienna += 0x02;
  }
  //  if (EMS_byte4 == 'K') {
  //   zmienna -= 2;
  //  }
  if (EMS_byte5 == 'W') {
    zmienna += 0x04;
  }
  // if (EMS_byte5 == 'K') {
  //   zmienna -= 4;
  // }
  if (EMS_byte6 == 'W') {
    zmienna += 0x08;
  }
  // if (EMS_byte6 == 'K') {
  //   zmienna -= 8;
  //  }
  if (EMS_byte7 == 'W') {
    zmienna1 = 0x32;
  }
  // if (EMS_byte7 == 'K') {
  //   zmienna1 = 0x00;
  // }

  byte mes1[] = {0x00, 0x05, 0xBF, 0x7A, zmienna, zmienna1, 0x00, 0x00, 0x58, 0x00};
  byte *data;
  data = mes1;
  int end_i = data[1] + 2 ;
  data[end_i - 1] = iso_checksum(data, end_i - 1);
  kbus.write(data, end_i + 1);


  if (currentDebugModule == "ems" || currentDebugModule == "EMS") {
    Serial.print(F("From Nano EMS: "));
    Serial.print(EMS_byte1);
    Serial.print(EMS_byte2);
    Serial.print(EMS_byte3);
    Serial.print(EMS_byte4);
    Serial.print(EMS_byte5);
    Serial.print(EMS_byte6);
    Serial.println(EMS_byte7);
  }
}
//WYCIĄGNIĘCIE TEGO DO OSOBNEJ FUNKCJI W OGÓLE DZIAŁA,
//W PRZECIWIEŃSTWIE DO UMIESZCZENIA TEGO KODU W FUNKCJI GŁÓWNEJ
//(SEPARACJA CZASOWA WYSYŁANIA KOMEND)
void EMSautodiagnostic() {
  if (millis() - lastEMStime >= EMStimeoutInterval) {
    Serial.println(F("EMS ERROR"));
   // sd.println(F("EMS ERROR"));
    byte zmienna; //= 0x0F ;
    byte zmienna1;
    if (state_counter == 0) {
      zmienna = 0x0F ;
      zmienna1 = 0x32;
    }
    if (state_counter == 1) {
      zmienna = 0x00 ;
      zmienna1 = 0x00;
    }

    //   zmienna = random(0, 10);
    //   zmienna1 = random(0, 0x32));
    byte mes1[] = {0x00, 0x05, 0xBF, 0x7A, zmienna, zmienna1, 0x00, 0x00, 0x58, 0x00};
    byte *data;
    data = mes1;
    int end_i = data[1] + 2 ;
    data[end_i - 1] = iso_checksum(data, end_i - 1);
    kbus.write(data, end_i + 1);
    //    byte mes12[] = {0xD0, 0x08, 0xBF, 0x5B, 0x60, 0x00, 0x00, 0x00, 0x00, 0x58, 0x00};//jeszcze awaryjne spod innego adresu
    //    data = mes12;
    //    int end_ii = data[1] + 2 ;
    //    data[end_ii - 1] = iso_checksum(data, end_i - 1);
    //    kbus.write(data, end_i + 1);
  }
}

void ems_debug_menu(){
if (currentDebugModule == "ems" || currentDebugModule == "EMS") {
  Serial.println(F("==============EMS=============="));
  i2cDoorsEMS();
  Serial.println(F("--> MAIN - to main menu"));
  Serial.println(F("--> EXIT - to normal mode"));
  currentDebugModule = "dupa";
}
}
#endif 
