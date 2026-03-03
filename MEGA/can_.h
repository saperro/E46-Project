#ifndef CAN_H
#define CAN_H
#include <Arduino.h>
#include "mcp2515_can.h" 
#include <SPI.h> 

const int SPI_CS_PIN = 9; // Wybór pinu CS dla SPI
mcp2515_can CAN(SPI_CS_PIN); // Inicjalizacja obiektu CAN

extern byte debugMode;
extern String currentDebugModule; 
// Deklaracje funkcji
void CanSend(short address, byte a, byte b, byte c, byte d, byte e, byte f, byte g, byte h);

/**
 * @brief Odczytuje ramkę CAN o określonym ID. Poprawiona dla biblioteki Seeed/mcp2515_can.
 * @param targetId - ID, które ma zostać odczytane (np. 0x613).
 * @param outputBuf - Tablica 8 bajtów, do której zostaną zapisane odczytane dane.
 * @return true, jeśli ramka o podanym ID została odebrana i dane zapisane.
 */
bool CanRead(long unsigned int targetId, unsigned char* outputBuf) {
  static long unsigned int rxId; 
  unsigned char len = 0;
  unsigned char rxBuf[8]; // Bufor tymczasowy
  
  if (CAN.checkReceive() == CAN_MSGAVAIL) {
    
    // 1. Odczytaj wiadomość do bufora (funkcja readMsgBuf z 2 argumentami)
    CAN.readMsgBuf(&len, rxBuf); 
    
    // 2. Pobierz ID ostatnio odczytanej ramki
    rxId = CAN.getCanId(); 

    // 3. Sprawdź, czy ID ramki jest zgodne z oczekiwanym
    if (rxId == targetId) {
      // Skopiuj odczytane bajty do bufora wyjściowego
      for (int i = 0; i < len; i++) {
        outputBuf[i] = rxBuf[i];
      }
      return true; // Sukces
    }
  }
  return false; // Ramka nie odebrana lub ID niezgodne
}

void CanSend(short address, byte a, byte b, byte c, byte d, byte e, byte f, byte g, byte h) {
  unsigned char DataToSend[8] = {a, b, c, d, e, f, g, h};
  CAN.sendMsgBuf(address, 0, 8, DataToSend);
}
void can_debug_menu(){
  if (currentDebugModule == "canraw" || currentDebugModule == "CANRAW") {
  //tu zrobić jakiś debug działania can
  }
}
#endif
