#ifndef READINGS_H
#define READINGS_H
#include "can_.h" 

// --- WSPÓLNE ZMIENNE STATYCZNE DLA CAŁEGO PLIKU ---
static unsigned long last_odometer_km = 0; 
static byte last_fuel_level = 0;
static unsigned char last_received_data_613[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/**
 * @brief Główna funkcja monitorująca ruch CAN. Odczytuje ramkę 0x613 raz na cykl.
 */
void monitor_can_messages() {
    unsigned char receivedData[8];
    const long unsigned int ODOMETER_ID = 0x613;

    if (CanRead(ODOMETER_ID, receivedData)) {
        
        // Zapisz odebrane dane do bufora statycznego
        for (int i = 0; i < 8; i++) {
            last_received_data_613[i] = receivedData[i];
        }
        
        // 1. Obliczenia przebiegu (B0 LSB, B1 MSB)
        unsigned int raw_odometer_value = (unsigned int)receivedData[0] | ((unsigned int)receivedData[1] << 8);
        last_odometer_km = (unsigned long)raw_odometer_value * 10;

        // 2. Odczyt poziomu paliwa (B2)
        last_fuel_level = receivedData[2];
    }
    //PRZYKŁAD DLA DODATKOWYCH DANYCH
    /*
     //  Sprawdzenie, czy odebrano ramkę 0x350
    if (CanRead(0x350, receivedData)) {
        // --- LOGIKA OBLICZEŃ 0x350 ---
        for (int i = 0; i < 8; i++) last_received_data_350[i] = receivedData[i];
        
        // PRZYKŁAD: Załóżmy, że B4 to surowa temperatura w C° + 40
        int raw_temp = receivedData[4];
        last_engine_temp_c = raw_temp - 40; 
        return; // Jeśli odczytano, wychodzimy
    }
     */
}

/**
 * @brief Zwraca przebieg.
 */
unsigned long odometer() {
    return last_odometer_km;
}

/**
 * @brief Zwraca poziom paliwa.
 */
byte fuel_level() {
    return last_fuel_level;
}

/**
 * @brief Funkcja debugowania. Wyświetla dane przez wywołanie wcześniej zrobionych funkcji.
 */
void can_readings_debug_menu(){
  
  if (currentDebugModule == "odometer" || currentDebugModule == "ODOMETER") {
    // wysyłanie otrzymanych danych na serial
    Serial.print(F("Przebieg (Km): "));
    Serial.println(odometer()); 
  }
  
  if (currentDebugModule == "fuel" || currentDebugModule == "FUEL") {
    // wysyłanie otrzymanych danych na serial
    Serial.print(F("Paliwo (HEX): 0x"));
    Serial.println(fuel_level(), HEX); 
  }
}
#endif
