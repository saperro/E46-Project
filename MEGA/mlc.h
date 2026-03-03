#ifndef MLC_H
#define MLC_H

#include <Wire.h>
#include <Arduino.h>
#include "k_bus.h"

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

char receivedFromModuleLightControl = ' ';
extern unsigned long lastRequestTime1;
extern unsigned long requestInterval1;
extern byte iso_checksum(byte* data, byte len);

const uint8_t MLC_RESET_PIN PROGMEM = 30;
unsigned long lastMLCtime;
const unsigned long MLCtimeoutInterval;

//=============STEROWANIE KONTROLKAMI KIERUNKOWSKAZÓW I ŚWIATEŁ 
//- dobre do debug w połączeniu z funkcjami void drzwi, void kontrolki_swiatel
uint8_t lewy_kierunkowskaz = 0;
uint8_t prawy_kierunkowskaz = 0;
uint8_t hazard = 0;
uint8_t dlugie_swiatla = 0;
uint8_t przeciwmgielne_przednie = 0;
uint8_t przeciwmgielne_tylne = 0;

//STEROWANIE LAMPKAMI DRZWI
uint8_t prawe_przednie_drzwi = 0;
uint8_t prawe_tylne_drzwi = 0;
uint8_t lewe_przednie_drzwi = 0;
uint8_t lewe_tylne_drzwi = 0;
uint8_t bagaznik = 0;

//Nie używane w głównej funkcji, do debug
void drzwi(byte prawe_przednie, byte prawe_tylne, byte lewe_przednie, byte lewe_tylne, byte bagaznik) {
  adress = 0x00;
  LightByte2 = 0x00;
  //byte zmienna;
  if (bagaznik == 1) {
    LightByte3 = 0x32;
  }
  if (prawe_przednie == 1) {
    zmienna += 2;
  }
  if (prawe_tylne == 1) {
    zmienna += 8;
  }
  if (lewe_przednie == 1) {
    zmienna += 1;
  }
  if (lewe_tylne == 1) {
    zmienna += 4;
  }
  //else {
  //  LightByte2 = 0x00;
  //  LightByte3 = 0x00;
  //  LightByte1 = 0x00;
  // }
  LightByte1 = zmienna;
  //LightByte2 = 0x00; // to zawsze ma zero
  // LightByte3 = 0x32; // jeśli bagażnik otwarty a poza tym 0x00
  //byte LightByte1 = 0x00; OFF
  //byte LightByte1 = 0x01; lewe przednie
  //byte LightByte1 = 0x02; prawe przednie
  //byte LightByte1 = 0x03; oba przednie
  //byte LightByte1 = 0x04; lewe tylne
  //byte LightByte1 = 0x05; oba lewe
  //byte LightByte1 = 0x06; przednie prawe, lewe tylne
  //byte LightByte1 = 0x07; prawe przednie i oba lewe
  //byte LightByte1 = 0x08; prawe tylne
  //byte LightByte1 = 0x09; lewe przednie, prawe tylne
  //byte LightByte1 = 0x10; oba prawe
  //LightByte3 = 0x32; // jeśli bagażnik otwarty
}
//Nie używane w głównej funkcji, do debug
void kontrolki_swiatel(byte turnleft_beam, byte turnright_beam, byte highbeam_beam, byte foglight_beam_front, byte foglight_beam_rear) {
  adress = 0xD0;
  //byte zmienna1;
  if (highbeam_beam == 1) {
    zmienna1 += 0x04;
  }
  if (turnright_beam == 1) {
    zmienna1 += 0x40;
  }
  if (turnleft_beam == 1) {
    zmienna1 += 0x20;
  }
  if (foglight_beam_front == 1) {
    zmienna1 += 0x10;
  }
  if (foglight_beam_rear == 1) {
    zmienna1 += 0x08;
  }
  LightByte1 = zmienna1;
}

void i2cLightsMLC() {
  char MLC_byte1, MLC_byte2, MLC_byte3, MLC_byte4, MLC_byte5, MLC_byte6, MLC_byte7, MLC_byte8, MLC_byte9;
  Wire.requestFrom(8, 9); //ADRES 8, LICZBA BAJTÓW 9
  MLC_byte1 = Wire.read();
  MLC_byte2 = Wire.read();
  MLC_byte3 = Wire.read();
  MLC_byte4 = Wire.read();
  MLC_byte5 = Wire.read();
  MLC_byte6 = Wire.read();
  MLC_byte7 = Wire.read();
  MLC_byte8 = Wire.read();
  MLC_byte9 = Wire.read();
  adress = 0xD0;
  byte zmienna1 = 0;//zera, bo ma liczyć z każą pętlą od nowa
  byte zmienna2 = 0;
  if (MLC_byte7 == 'W') {
    zmienna1 += 0x04;
    lastMLCtime = millis();//jesli odebrano W lub K zrestartuj czas diagnostyki MLC
  }
  if (MLC_byte7 == 'K') {
    lastMLCtime = millis();//jesli odebrano W lub K zrestartuj czas diagnostyki MLC
  }
  if (hazard == 1) {
    zmienna1 = 0x60;
  }
  if (hazard == 0) {
    if (MLC_byte6 == 'W') {
      zmienna1 += 0x40;
    }
    if (MLC_byte5 == 'W') {
      zmienna1 += 0x20;
    }
  }
  if (MLC_byte9 == 'W') {
    zmienna1 += 0x10;
  }
  if (MLC_byte8 == 'W') {
    zmienna1 += 0x08;
  }


  //if (wnetrze == 1) {
  //  zmienna = 0x80;
  // }
  if (MLC_byte1 == 'W') {
    zmienna2 += 0x20;
  }
  if (MLC_byte3 == 'W') {
    zmienna2 += 0x08;
  }
  if (MLC_byte2 == 'W') {
    zmienna2 += 0x10;
  }
  if (MLC_byte4 == 'W') {
    zmienna2 += 0x04;
  }

  byte *data;
  byte a;
  byte b;
  byte c;
  byte d;
  byte e;
  a = 0xD0;
  b = 0x08;
  c = 0xBF;
  d = 0x5B;
  e = 0x58;
  LightByte3 = 0x00;

  byte mes1[] = {a, b, c, d, zmienna1, LightByte3, 0x00, zmienna2, 0x00, e, 0x00};

  data = mes1;
  int end_i = data[1] + 2 ;
  data[end_i - 1] = iso_checksum(data, end_i - 1);
  kbus.write(data, end_i + 1);
  if (currentDebugModule == "mlc" || currentDebugModule == "MLC") {
    if (millis() - lastRequestTime1 >= requestInterval1) {
      lastRequestTime1 = millis();
      Serial.print(F("From Nano MLC: K = ON, W = OFF"));
      Serial.println(F("leftFrontBulbErr"));
      Serial.print(MLC_byte1);
      Serial.println(F("rightFrontBulbErr"));
      Serial.print(MLC_byte2);
      Serial.println(F("leftRearBulbErr"));
      Serial.print(MLC_byte3);
      Serial.println(F("rightRearBulbErr"));
      Serial.print(MLC_byte4);
      Serial.println(F("turnLeftSignalState"));
      Serial.print(MLC_byte5);
      Serial.println(F("turnRightSignalState"));
      Serial.print(MLC_byte6);
      Serial.println(F("highBeamLightState"));
      Serial.print(MLC_byte7);
      Serial.println(F("fogRearLightState"));
      Serial.print(MLC_byte8);
      Serial.println(F("fogFrontLightState"));
      Serial.println(MLC_byte9);
    }
  }
}
//WYCIĄGNIĘCIE TEGO DO OSOBNEJ FUNKCJI W OGÓLE DZIAŁA,
//W PRZECIWIEŃSTWIE DO UMIESZCZENIA TEGO KODU W FUNKCJI GŁÓWNEJ
//(SEPARACJA CZASOWA WYSYŁANIA KOMEND)
void MLCautodiagnostic() {
  if (millis() - lastMLCtime >= MLCtimeoutInterval) {
    byte *data;
    byte a;
    byte b;
    byte c;
    byte d;
    byte e;
    byte h;
    byte g;
    a = 0xD0;
    b = 0x08;
    c = 0xBF;
    d = 0x5B;
    e = 0x58;
    //0x3C
    //g=0x3C // suma wszystkich
    if (state_counter == 0) { //TEN COUNTER JEST W LOOPIE I UŁATWIA MIGANIE CZYMKOLWIEK
      g = 0x3C; // WSZYSTKIE WŁĄCZONE
    }
    if (state_counter == 1) {
      g = 0x00; //WSZYSTKIE WYŁĄCZONE
    }
    // g = random(0, 120);
    //0x7C
    // h=0x7C;
    //h = random(0, 120);
    // h = 0x60;
    h = 0x00; //kierunki

    //h+=3;
    byte mes1[] = {a, b, c, d, h, 0x00, 0x00, g, 0x00, e, 0x00};
    //byte mes1[] = {0xD0, 0x08, 0xBF, 0x5B, 0x60, 0x00, 0x00, 0x00, 0x00, x58, 0x00};
    data = mes1;
    int end_i = data[1] + 2 ;
    data[end_i - 1] = iso_checksum(data, end_i - 1);
    kbus.write(data, end_i + 1);
    //delay(50);
    //h=0x00;
  }
}
//Propozycja czata: (można scalić diagnostykę z główną funkcją)
//void i2cLightsMLC() {
//  // Sprawdź czy moduł odpowiada
//  if (millis() - lastMLCtime >= MLCtimeoutInterval) {
//    // MODUŁ SIĘ ROZŁĄCZYŁ - AUTODIAGNOSTYKA
//    byte *data;
//    byte a = 0xD0;
//    byte b = 0x08;
//    byte c = 0xBF;
//    byte d = 0x5B;
//    byte e = 0x58;
//    byte h = 0x00; //kierunki
//    byte g;
//
//    if (state_counter == 0) {
//      g = 0x3C; // WSZYSTKIE WŁĄCZONE
//    } else {
//      g = 0x00; // WSZYSTKIE WYŁĄCZONE
//    }
//
//    byte mes1[] = {a, b, c, d, h, 0x00, 0x00, g, 0x00, e, 0x00};
//    data = mes1;
//    int end_i = data[1] + 2;
//    data[end_i - 1] = iso_checksum(data, end_i - 1);
//    kbus.write(data, end_i + 1);
//    return; // Koniec - nie wykonuj reszty
//  }
//
//  // NORMALNY TRYB - MODUŁ POŁĄCZONY
//  char MLC_byte1, MLC_byte2, MLC_byte3, MLC_byte4, MLC_byte5, MLC_byte6, MLC_byte7, MLC_byte8, MLC_byte9;
//  Wire.requestFrom(8, 9);
//  MLC_byte1 = Wire.read();
//  MLC_byte2 = Wire.read();
//  MLC_byte3 = Wire.read();
//  MLC_byte4 = Wire.read();
//  MLC_byte5 = Wire.read();
//  MLC_byte6 = Wire.read();
//  MLC_byte7 = Wire.read();
//  MLC_byte8 = Wire.read();
//  MLC_byte9 = Wire.read();
//
//  adress = 0xD0;
//  byte zmienna1 = 0;
//  byte zmienna2 = 0;
//
//  if (MLC_byte7 == 'W' || MLC_byte7 == 'K') {
//    lastMLCtime = millis(); // Reset timera
//  }
//
//  if (MLC_byte7 == 'W') {
//    zmienna1 += 0x04;
//  }
//
//  if (hazard == 1) {
//    zmienna1 = 0x60;
//  }
//  if (hazard == 0) {
//    if (MLC_byte6 == 'W') {
//      zmienna1 += 0x40;
//    }
//    if (MLC_byte5 == 'W') {
//      zmienna1 += 0x20;
//    }
//  }
//  if (MLC_byte9 == 'W') {
//    zmienna1 += 0x10;
//  }
//  if (MLC_byte8 == 'W') {
//    zmienna1 += 0x08;
//  }
//
//  if (MLC_byte1 == 'W') {
//    zmienna2 += 0x20;
//  }
//  if (MLC_byte3 == 'W') {
//    zmienna2 += 0x08;
//  }
//  if (MLC_byte2 == 'W') {
//    zmienna2 += 0x10;
//  }
//  if (MLC_byte4 == 'W') {
//    zmienna2 += 0x04;
//  }
//
//  byte *data;
//  byte a = 0xD0;
//  byte b = 0x08;
//  byte c = 0xBF;
//  byte d = 0x5B;
//  byte e = 0x58;
//  LightByte3 = 0x00;
//
//  byte mes1[] = {a, b, c, d, zmienna1, LightByte3, 0x00, zmienna2, 0x00, e, 0x00};
//  data = mes1;
//  int end_i = data[1] + 2;
//  data[end_i - 1] = iso_checksum(data, end_i - 1);
//  kbus.write(data, end_i + 1);
//
//  if (millis() - lastRequestTime1 >= requestInterval1) {
//    lastRequestTime1 = millis();
//    Serial.print(F("From Nano MLC: "));
//    Serial.print(MLC_byte1);
//    Serial.print(MLC_byte2);
//    Serial.print(MLC_byte3);
//    Serial.print(MLC_byte4);
//    Serial.print(MLC_byte5);
//    Serial.print(MLC_byte6);
//    Serial.print(MLC_byte7);
//    Serial.print(MLC_byte8);
//    Serial.println(MLC_byte9);
//  }
//}

void mlc_debug_menu() {
  if (currentDebugModule == "mlc" || currentDebugModule == "MLC") {
    Serial.println(F("==============MLC=============="));
    i2cLightsMLC();
    Serial.println(F("--> MLC - refresh"));
    Serial.println(F("--> MLCRESET - restart module"));
    Serial.println(F("--> MAIN - to main menu"));
    Serial.println(F("--> EXIT - to normal mode"));
    currentDebugModule = "dupa";
  }
  if (currentDebugModule == "MLCRESET" || currentDebugModule == "mlcreset") {
    Serial.println(F("Restarting MLC module"));
    digitalWrite(MLC_RESET_PIN, HIGH);
    delay(50);
    digitalWrite(MLC_RESET_PIN, LOW);
    delay(1000);
    currentDebugModule = "main";
  }
}
#endif
