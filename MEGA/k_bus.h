#ifndef K_BUS_H
#define K_BUS_H

#include <Wire.h>
#include <Arduino.h>
// Set kBus- TX2
#define kbus Serial2
// K-Bus Variables
uint8_t LightByte1 = 0x00;
uint8_t LightByte2 = 0x00;
uint8_t LightByte3 = 0x00;
uint8_t adress = 0x00;
int counter = 0;
uint8_t zmienna = 0;
uint8_t zmienna1 = 0;

byte iso_checksum(byte * data, byte len) //len is the number of bytes (not the # of last byte)
{
  byte crc = 0;
  for (byte i = 0; i < len; i++)
  {
    crc = crc ^ data[i];
  }
  return crc;
}
void sendKbus()
{
  // Begin K-Bus-Data
  // For Lightbyte1
  // 0x04=Indicator Highbeam
  // 0x08=Indicator Foglight rear
  // 0x10=Indicator Foglight front

  // 0x14=Fogrear+Highbeam
  // 0x20=Indicator Left
  // 0x24=Left+Highbeam
  // 0x28=Left+Fogrear
  // 0x30=Left+Fogfront
  // 0x34=Left+Fogfront+Highbeam
  // 0x38=Left+Fogfront+Fogrear
  // 0x40=Indicator Right
  // 0x44=Right+Highbeam
  // 0x50=Right+Fogfront
  // 0x54=Right+Fogfront+Highbeam
  // 0x58=Right+Fogfront+Fogrear
  // 0x60=Both Indicators
  // 0x64=Both+Highbeam
  // 0x70=Both+Fogfront
  // 0x74=Both+Fogrfront+Highbeam
  // 0x78=Both+Fogfront+Fogrear
  // For Lightbyte2
  //wnętrze świeci zawsze kiedy błąd
  //0x00 off
  //0x04 wnętrze i tył prawo
  //0x08 wnętrze i tył lewo
  //0x10 wnętrze i przód prawy
  //0x14 wnętrze + prawy przód +prawy tył
  //0x18 wnętrze + prawy przód +lewy tył
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
  //0x80 samo wnętrze
  //LightByte3
  //0x32 bagaznik

  byte *data;
  byte a;
  byte b;
  byte c;
  byte d;
  byte e;
  if (adress == 0x00) {
    a = 0x00;
    b = 0x05;
    c = 0xBF;
    d = 0x7A;
    e = 0x00;
  }
  if (adress == 0xD0) {
    a = 0xD0;
    b = 0x08;
    c = 0xBF;
    d = 0x5B;
    e = 0x58;
    LightByte3 = 0x00;
  }

  byte mes1[] = {a, b, c, d, LightByte1, LightByte3, 0x00, LightByte2, 0x00, e, 0x00};

  data = mes1;
  int end_i = data[1] + 2 ;
  data[end_i - 1] = iso_checksum(data, end_i - 1);
  kbus.write(data, end_i + 1);
}


#endif
