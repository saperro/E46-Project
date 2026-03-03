#include <Wire.h>
#include <EEPROM.h>

// ============================================================================
// DEFINICJE PINÓW ZGODNIE Z DOKUMENTACJĄ
// ============================================================================

// Wyjścia zgodnie z dokumentacją
#define BATTERY_LAMP_PIN 2        // D2 - Kontrolka akumulatora (miganie przy braku komunikacji)
#define REL_INV_12VSB_PIN 3       // D3 - Przekaźnik przetwornicy +12VSB  
#define PWR_ON_EMS_PIN 4          // D4 - Wzbudzenie EMS
#define FEEDBACK_EMS_PIN 5        // D5 - Feedback z EMS
#define CHANGING_AKU_C_PIN 6      // D6 - Ładowanie akumulatora C (TYLKO tryb samowzbudzenia)
#define DFM_PIN 7                 // D7 - Duty cycle alternatora (INPUT)
#define REL_SEL_B_PIN 8           // D8 - Selektor ładowania B
#define REL_A_B_PIN 9             // D9 - Przekaźnik A/B
#define DOOR_SENSOR_PIN 10        // D10 - Sensor drzwi
#define REL_BYPASS_PIN 12         // D12 - Przekaźnik bypass
#define REL_LOAD_PIN 13           // D13 - Przekaźnik load

// Pomiary napięć
#define POM_ALT_B_PIN A0          // A0 - Alternator B
#define POM_INS_B_PIN A1          // A1 - Instalacja B
#define POM_INS_A_PIN A2          // A2 - Instalacja A  
#define POM_AKU_C_PIN A3          // A3 - Akumulator C
#define POM_AKU_B_PIN A6          // A6 - Akumulator B
#define POM_AKU_A_PIN A7          // A7 - Akumulator A

// ============================================================================
// ZMIENNE GLOBALNE - ZOPTYMALIZOWANE
// ============================================================================

// Flagi w strukturach bitowych
struct {
  uint8_t debugMode:1;
  uint8_t integrationMode:1;
  uint8_t selfWakeMode:1;
  uint8_t systemActive:1;
  uint8_t batteryLampBlinking:1;
  uint8_t emsFeedbackActive:1;
  uint8_t doorOpen:1;
  uint8_t highEfficiencyModeB:1;
} flags;

struct {
  uint8_t communicationWithMega:1;
  uint8_t emsWakeActive:1;
  uint8_t d3AlwaysOn:1;
  uint8_t unused:5;
} commFlags;

// Podstawowe zmienne
uint8_t ignitionPosition = 0;
uint8_t systemMode = 0;
uint8_t outputStates = 0;
uint8_t lastCommand = 0;
uint8_t autoOffTimeMinutes = 10;

// Pomiary napięć (w setnych wolta)
uint16_t voltageAkuA, voltageAkuB, voltageAkuC;
uint16_t voltageAltB, voltageInsA, voltageInsB;

// Timery - tylko najważniejsze
uint32_t emsWakeTime = 0;
uint32_t integrationStartTime = 0;
uint32_t lastMeasurement = 0;
uint32_t lastMegaCommunication = 0;
uint32_t highEfficiencyDisabledTime = 0;

// Stałe w PROGMEM
const uint16_t MEASUREMENT_INTERVAL PROGMEM = 100;
const uint16_t STREAMING_INTERVAL PROGMEM = 500;
const uint16_t BATTERY_LAMP_INTERVAL PROGMEM = 500;
const uint32_t MEGA_COMM_TIMEOUT PROGMEM = 300000UL;
const uint32_t MIN_HIGH_EFF_DISABLE_TIME PROGMEM = 60000UL;

// ============================================================================
// SETUP
// ============================================================================
void setup() {
  Serial.begin(9600);
  Wire.begin(0x20);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  
  // Wczytaj ustawienia z EEPROM
  uint8_t savedTime = EEPROM.read(0);
  if (savedTime == 2 || savedTime == 5 || savedTime == 10 || savedTime == 15) {
    autoOffTimeMinutes = savedTime;
  }
  
  // Konfiguracja pinów
  pinMode(BATTERY_LAMP_PIN, OUTPUT);
  pinMode(REL_INV_12VSB_PIN, OUTPUT);
  pinMode(PWR_ON_EMS_PIN, OUTPUT);
  pinMode(CHANGING_AKU_C_PIN, OUTPUT);
  pinMode(REL_SEL_B_PIN, OUTPUT);
  pinMode(REL_A_B_PIN, OUTPUT);
  pinMode(REL_BYPASS_PIN, OUTPUT);
  pinMode(REL_LOAD_PIN, OUTPUT);
  
  pinMode(FEEDBACK_EMS_PIN, INPUT);
  pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP);
  pinMode(DFM_PIN, INPUT);
  
  // D3 ZAWSZE HIGH - zasilanie przetwornicy i MPC
  digitalWrite(REL_INV_12VSB_PIN, HIGH);
  commFlags.d3AlwaysOn = 1;
  
  // Pozostałe wyjścia OFF
  setAllOutputsOffExceptD3();
  
  Serial.println(F("MPC v0.6 Ready"));
  
  delay(1000);
  readMeasurements();
  lastMegaCommunication = millis();
  commFlags.communicationWithMega = 1;
}

// ============================================================================
// GŁÓWNA PĘTLA
// ============================================================================
void loop() {
  uint32_t currentTime = millis();
  
  // KRYTYCZNE: D3 zawsze musi być HIGH (chyba że koniec ładowania/brak EMS)
  if (!digitalRead(REL_INV_12VSB_PIN) && commFlags.d3AlwaysOn) {
    digitalWrite(REL_INV_12VSB_PIN, HIGH);
  }
  
  // Regularne pomiary
  if (currentTime - lastMeasurement >= pgm_read_word(&MEASUREMENT_INTERVAL)) {
    readMeasurements();
    lastMeasurement = currentTime;
  }
  
  // Streaming w trybie debug
  static uint32_t lastStream = 0;
  if (flags.debugMode && (currentTime - lastStream >= pgm_read_word(&STREAMING_INTERVAL))) {
    sendStreamingData();
    lastStream = currentTime;
  }
  
  // Miganie kontrolki (brak komunikacji lub debug)
  if (flags.batteryLampBlinking) {
    static uint32_t lastToggle = 0;
    if (currentTime - lastToggle >= pgm_read_word(&BATTERY_LAMP_INTERVAL)) {
      digitalWrite(BATTERY_LAMP_PIN, !digitalRead(BATTERY_LAMP_PIN));
      lastToggle = currentTime;
    }
  }
  
  // Sprawdzenie komunikacji z MEGA
  if (currentTime - lastMegaCommunication > pgm_read_dword(&MEGA_COMM_TIMEOUT)) {
    commFlags.communicationWithMega = 0;
  }
  
  // Normalna praca MPC
  if (!flags.debugMode) {
    normalMPCOperation();
  }
  
  delay(10);
}

// ============================================================================
// OBSŁUGA I2C - ODBIÓR
// ============================================================================
void receiveEvent(int howMany) {
  if (howMany < 1) return;
  
  lastCommand = Wire.read();
  lastMegaCommunication = millis();
  commFlags.communicationWithMega = 1;
  
  switch (lastCommand) {
    case 0x01: break; // GET_VOLTAGES
      
    case 0x02: // SET_IGNITION
      if (Wire.available()) {
        ignitionPosition = Wire.read();
      }
      break;
      
    case 0x10: // RESET - sprzętowy przez MEGA
      Serial.println(F("Reset by MEGA"));
      break;
      
    case 0x11: // DEBUG_MODE_ON
      flags.debugMode = 1;
      setAllOutputsOffExceptD3();
      break;
      
    case 0x12: // DEBUG_MODE_OFF
      flags.debugMode = 0;
      break;
      
    case 0x20: // OUTPUT_CONTROL
      if (flags.debugMode && Wire.available() >= 2) {
        uint8_t pin = Wire.read();
        uint8_t state = Wire.read();
        setOutputPin(pin, state);
      }
      break;
      
    case 0x21: // ALL_OUTPUTS_OFF
      if (flags.debugMode) setAllOutputsOffExceptD3();
      break;
      
    case 0x22: break; // GET_OUTPUTS_STATUS
      
    case 0x30: // SET_AUTO_OFF_TIME
      if (Wire.available()) {
        uint8_t newTime = Wire.read();
        if (newTime == 2 || newTime == 5 || newTime == 10 || newTime == 15) {
          autoOffTimeMinutes = newTime;
          EEPROM.write(0, autoOffTimeMinutes);
        }
      }
      break;
      
    case 0x31: // SET_CHARGING_TIME_C (przygotowane na przyszłość)
      if (Wire.available()) {
        uint8_t newTime = Wire.read();
        if (newTime == 70 || newTime == 90 || newTime == 120 || newTime == 150) {
          EEPROM.write(1, newTime);
        }
      }
      break;
      
    case 0x32: // BATTERY_LAMP_BLINK_ON (sterowane przez MEGA w trybie debug)
      if (flags.debugMode) {
        flags.batteryLampBlinking = 1;
      }
      break;
      
    case 0x33: // BATTERY_LAMP_BLINK_OFF
      if (flags.debugMode) {
        flags.batteryLampBlinking = 0;
        digitalWrite(BATTERY_LAMP_PIN, LOW);
      }
      break;
  }
}

// ============================================================================
// OBSŁUGA I2C - WYSYŁANIE
// ============================================================================
void requestEvent() {
  if (lastCommand == 0x22) {
    updateOutputStates();
    Wire.write(outputStates);
  } else {
    // Pakiet danych pomiarowych (16 bajtów)
    Wire.write(voltageAkuA & 0xFF);
    Wire.write((voltageAkuA >> 8) & 0xFF);
    Wire.write(voltageAkuB & 0xFF);
    Wire.write((voltageAkuB >> 8) & 0xFF);
    Wire.write(voltageAkuC & 0xFF);
    Wire.write((voltageAkuC >> 8) & 0xFF);
    Wire.write(voltageAltB & 0xFF);
    Wire.write((voltageAltB >> 8) & 0xFF);
    Wire.write(voltageInsA & 0xFF);
    Wire.write((voltageInsA >> 8) & 0xFF);
    Wire.write(voltageInsB & 0xFF);
    Wire.write((voltageInsB >> 8) & 0xFF);
    
    // Status byte
    uint8_t statusByte = systemMode & 0x0F;
    if (flags.systemActive) statusByte |= 0x80;
    if (commFlags.emsWakeActive) statusByte |= 0x40;
    if (flags.debugMode) statusByte |= 0x20;
    if (flags.highEfficiencyModeB) statusByte |= 0x10;
    Wire.write(statusByte);
    
    // Flags byte
    uint8_t flagsByte = 0;
    if (flags.doorOpen) flagsByte |= 0x01;
    if (flags.emsFeedbackActive) flagsByte |= 0x02;
    flagsByte |= (ignitionPosition << 2) & 0x3C;
    if (!commFlags.communicationWithMega) flagsByte |= 0x80;
    Wire.write(flagsByte);
  }
}

// ============================================================================
// FUNKCJE STEROWANIA
// ============================================================================
void setOutputPin(uint8_t pin, uint8_t state) {
  bool pinState = (state != 0);
  switch (pin) {
    case 3: 
      // D3 może być sterowane w trybie debug
      if (flags.debugMode) {
        digitalWrite(REL_INV_12VSB_PIN, pinState);
        commFlags.d3AlwaysOn = pinState;
      }
      break;
    case 4: digitalWrite(PWR_ON_EMS_PIN, pinState); break;
    case 6: digitalWrite(CHANGING_AKU_C_PIN, pinState); break;
    case 8: digitalWrite(REL_SEL_B_PIN, pinState); break;
    case 9: digitalWrite(REL_A_B_PIN, pinState); break;
    case 12: digitalWrite(REL_BYPASS_PIN, pinState); break;
    case 13: digitalWrite(REL_LOAD_PIN, pinState); break;
  }
  updateOutputStates();
}

void updateOutputStates() {
  outputStates = 0;
  if (digitalRead(REL_LOAD_PIN)) outputStates |= 0x01;
  if (digitalRead(REL_A_B_PIN)) outputStates |= 0x02;
  if (digitalRead(REL_BYPASS_PIN)) outputStates |= 0x04;
  if (digitalRead(REL_INV_12VSB_PIN)) outputStates |= 0x08;
  if (digitalRead(PWR_ON_EMS_PIN)) outputStates |= 0x10;
  if (digitalRead(REL_SEL_B_PIN)) outputStates |= 0x20;
  if (digitalRead(CHANGING_AKU_C_PIN)) outputStates |= 0x40;
}

void setAllOutputsOffExceptD3() {
  digitalWrite(BATTERY_LAMP_PIN, LOW);
  // D3 pozostaje w swoim stanie!
  digitalWrite(PWR_ON_EMS_PIN, LOW);
  digitalWrite(CHANGING_AKU_C_PIN, LOW);
  digitalWrite(REL_SEL_B_PIN, LOW);
  digitalWrite(REL_A_B_PIN, LOW);
  digitalWrite(REL_BYPASS_PIN, LOW);
  digitalWrite(REL_LOAD_PIN, LOW);
  
  flags.batteryLampBlinking = 0;
  flags.highEfficiencyModeB = 0;
  updateOutputStates();
}

// ============================================================================
// FUNKCJE POMIAROWE
// ============================================================================
void readMeasurements() {
  // Wzór: (ADC * 1717) / 1000 dla dzielnika 68k/27k
  voltageAkuA = ((uint32_t)analogRead(POM_AKU_A_PIN) * 1717) / 1000;
  voltageAkuB = ((uint32_t)analogRead(POM_AKU_B_PIN) * 1717) / 1000;
  voltageAkuC = ((uint32_t)analogRead(POM_AKU_C_PIN) * 1717) / 1000;
  voltageAltB = ((uint32_t)analogRead(POM_ALT_B_PIN) * 1717) / 1000;
  voltageInsA = ((uint32_t)analogRead(POM_INS_A_PIN) * 1717) / 1000;
  voltageInsB = ((uint32_t)analogRead(POM_INS_B_PIN) * 1717) / 1000;
  
  flags.doorOpen = !digitalRead(DOOR_SENSOR_PIN);
  flags.emsFeedbackActive = digitalRead(FEEDBACK_EMS_PIN);
}

// ============================================================================
// REGULACJA ALTERNATORA B
// ============================================================================
void manageAlternatorB() {
  bool canChange = (highEfficiencyDisabledTime == 0) || 
    (millis() - highEfficiencyDisabledTime >= pgm_read_dword(&MIN_HIGH_EFF_DISABLE_TIME));
    
  if (!flags.highEfficiencyModeB && canChange) {
    // Włącz tryb wysokiej wydajności gdy:
    // - napięcie instalacji < 13.4V
    // - różnica instalacja-akumulator ±0.5V
    if (voltageInsB < 1340 && abs((int)voltageInsB - (int)voltageAkuB) <= 50) {
      flags.highEfficiencyModeB = 1;
      digitalWrite(REL_SEL_B_PIN, HIGH);
    }
  }
  
  if (flags.highEfficiencyModeB) {
    bool shouldDisable = false;
    
    // Zabezpieczenie przed przeładowaniem (>16.2V)
    if (voltageAkuB > 1620) shouldDisable = true;
    
    // Napięcia wyrównane (±0.5V między wszystkimi punktami)
    if (abs((int)voltageInsB - (int)voltageAltB) <= 50 &&
        abs((int)voltageInsB - (int)voltageAkuB) <= 50 &&
        abs((int)voltageAltB - (int)voltageAkuB) <= 50) {
      shouldDisable = true;
    }
    
    if (shouldDisable) {
      flags.highEfficiencyModeB = 0;
      digitalWrite(REL_SEL_B_PIN, LOW);
      highEfficiencyDisabledTime = millis();
    }
  }
}

void sendStreamingData() {
  Serial.println(F("=== MPC DATA ==="));
  Serial.print(F("AKU_A:")); Serial.println(voltageAkuA / 100.0, 2);
  Serial.print(F("AKU_B:")); Serial.println(voltageAkuB / 100.0, 2);
  Serial.print(F("AKU_C:")); Serial.println(voltageAkuC / 100.0, 2);
  Serial.print(F("ALT_B:")); Serial.println(voltageAltB / 100.0, 2);
  Serial.print(F("INS_A:")); Serial.println(voltageInsA / 100.0, 2);
  Serial.print(F("INS_B:")); Serial.println(voltageInsB / 100.0, 2);
  Serial.print(F("IGN:")); Serial.println(ignitionPosition);
  Serial.print(F("DOOR:")); Serial.println(flags.doorOpen);
  Serial.print(F("EMS:")); Serial.println(flags.emsFeedbackActive);
  Serial.print(F("ALT_MODE:")); Serial.println(flags.highEfficiencyModeB ? F("HIGH") : F("STD"));
  Serial.print(F("MODE:")); 
  if (flags.selfWakeMode) Serial.println(F("SELFWAKE"));
  else if (flags.integrationMode) Serial.println(F("INTEGRATION"));
  else Serial.println(F("NORMAL"));
  Serial.print(F("SYS_ACTIVE:")); Serial.println(flags.systemActive);
  Serial.print(F("COMM_MEGA:")); Serial.println(commFlags.communicationWithMega);
  Serial.println(F("================"));
}

// ============================================================================
// TRYB NORMALNY
// ============================================================================
void normalMPCOperation() {
  uint32_t currentTime = millis();
  
  // TRYB INTEGRACJI - drzwi otwarte + AKU_C > 10.8V
  if (!flags.integrationMode && !flags.selfWakeMode && flags.doorOpen && voltageAkuC > 1080) {
    flags.integrationMode = 1;
    integrationStartTime = currentTime;
    // D3 już jest HIGH od setup()
    digitalWrite(PWR_ON_EMS_PIN, HIGH);
    emsWakeTime = currentTime;
    commFlags.emsWakeActive = 1;
  }
  
  // TRYB SAMOWZBUDZENIA - AKU_C <= 10.8V
  // JEDYNE MIEJSCE GDZIE D6 JEST WŁĄCZANE
  if (!flags.integrationMode && !flags.selfWakeMode && voltageAkuC <= 1080) {
    flags.selfWakeMode = 1;
    integrationStartTime = currentTime;
    // D3 już jest HIGH od setup()
    digitalWrite(CHANGING_AKU_C_PIN, HIGH); // ← TYLKO TUTAJ D6 = HIGH
  }
  
  // WZBUDZENIE EMS (3 sekundy)
  if (commFlags.emsWakeActive && emsWakeTime > 0 && (currentTime - emsWakeTime >= 3000)) {
    digitalWrite(PWR_ON_EMS_PIN, LOW);
    emsWakeTime = 0;
    commFlags.emsWakeActive = 0;
  }
  
  // STEROWANIE WEDŁUG STACYJKI
  static uint8_t lastIgnition = 255;
  if (commFlags.communicationWithMega && ignitionPosition != lastIgnition) {
    lastIgnition = ignitionPosition;
    
    switch (ignitionPosition) {
      case 0: // OFF
        systemMode = 0;
        flags.systemActive = 0;
        digitalWrite(REL_LOAD_PIN, LOW);
        digitalWrite(REL_BYPASS_PIN, LOW);
        digitalWrite(REL_A_B_PIN, LOW);
        // D6 wyłączane gdy nie jest tryb samowzbudzenia
        if (!flags.selfWakeMode) {
          digitalWrite(CHANGING_AKU_C_PIN, LOW);
        }
        break;
        
      case 1: // POS1 - silnik nie pracuje
        systemMode = 1;
        flags.systemActive = 1;
        digitalWrite(REL_LOAD_PIN, HIGH);
        digitalWrite(REL_BYPASS_PIN, LOW);
        digitalWrite(REL_A_B_PIN, HIGH);
        // D6 wyłączane - nie używamy w normalnych trybach
        if (!flags.selfWakeMode) {
          digitalWrite(CHANGING_AKU_C_PIN, LOW);
        }
        break;
        
      case 2: // POS2 - silnik pracuje
        systemMode = 2;
        flags.systemActive = 1;
        digitalWrite(REL_LOAD_PIN, HIGH);
        digitalWrite(REL_BYPASS_PIN, LOW);
        digitalWrite(REL_A_B_PIN, HIGH);
        // D6 wyłączane - nie używamy w normalnych trybach
        if (!flags.selfWakeMode) {
          digitalWrite(CHANGING_AKU_C_PIN, LOW);
        }
        break;
        
      case 3: // START - kręci rozrusznikiem
        systemMode = 3;
        flags.systemActive = 1;
        digitalWrite(REL_LOAD_PIN, LOW);
        digitalWrite(REL_BYPASS_PIN, LOW);
        digitalWrite(REL_A_B_PIN, LOW);
        // D6 wyłączane
        if (!flags.selfWakeMode) {
          digitalWrite(CHANGING_AKU_C_PIN, LOW);
        }
        break;
    }
  }
  
  // AWARIE ŁADOWANIA (TYLKO gdy silnik pracuje - pozycja 2)
  if (ignitionPosition == 2 && flags.systemActive && commFlags.communicationWithMega) {
    static uint32_t lastChargingCheck = 0;
    if (currentTime - lastChargingCheck >= 2000) {
      lastChargingCheck = currentTime;
      
      // Awaria ładowania systemowego
      // Warunek: (A1 < A6) LUB (A1 == A6 I A1 <= 13.6V)
      if (voltageInsB < voltageAkuB || 
          (voltageInsB == voltageAkuB && voltageInsB <= 1360)) {
        digitalWrite(REL_A_B_PIN, HIGH);
        // MEGA otrzyma informację analizując napięcia
      }
      
      // Awaria ładowania rozruchowego
      // Warunek: (A2 < A7) LUB (A2 == A7 I A2 <= 13.6V)
      if (voltageInsA < voltageAkuA || 
          (voltageInsA == voltageAkuA && voltageInsA <= 1360)) {
        digitalWrite(REL_BYPASS_PIN, HIGH);
        // MEGA otrzyma informację analizując napięcia
      }
    }
    
    // Regulacja alternatora B tylko gdy silnik pracuje
    manageAlternatorB();
  }
  
  // TIMEOUT TRYBU INTEGRACJI
  if (flags.integrationMode) {
    // Reset timeoutu gdy feedback EMS aktywny
    if (flags.emsFeedbackActive) {
      integrationStartTime = currentTime;
    }
    
    // Timeout gdy brak feedbacku
    if (!flags.emsFeedbackActive && 
        (currentTime - integrationStartTime >= ((uint32_t)autoOffTimeMinutes * 60000UL))) {
      // KONIEC trybu integracji
      flags.integrationMode = 0;
      
      // D3 = LOW gdy brak feedbacku EMS
      digitalWrite(REL_INV_12VSB_PIN, LOW);
      commFlags.d3AlwaysOn = 0;
      
      setAllOutputsOffExceptD3();
    }
  }
  
  // TIMEOUT TRYBU SAMOWZBUDZENIA
  if (flags.selfWakeMode) {
    // Timeout 2 godziny
    if (currentTime - integrationStartTime >= 7200000UL) {
      flags.selfWakeMode = 0;
      digitalWrite(CHANGING_AKU_C_PIN, LOW); // Wyłącz D6
      
      // D3 = LOW po zakończeniu ładowania
      digitalWrite(REL_INV_12VSB_PIN, LOW);
      commFlags.d3AlwaysOn = 0;
    }
    
    // Przejście na tryb integracji gdy drzwi otwarte
    if (flags.doorOpen) {
      flags.selfWakeMode = 0;
      flags.integrationMode = 1;
      digitalWrite(CHANGING_AKU_C_PIN, LOW); // Wyłącz D6
      digitalWrite(PWR_ON_EMS_PIN, HIGH);    // Wzbudź EMS
      emsWakeTime = currentTime;
      commFlags.emsWakeActive = 1;
      integrationStartTime = currentTime; // Reset czasu
    }
  }
  
  // TRYB AWARYJNY - utrata komunikacji z MEGA
  if (!commFlags.communicationWithMega) {
    handleCommunicationLoss();
  }
}

void handleCommunicationLoss() {
  static bool emergencyMode = false;
  
  if (!emergencyMode) {
    emergencyMode = true;
    // Miganie kontrolki TYLKO przy braku komunikacji
    flags.batteryLampBlinking = 1;
  }
  
  // Analiza według feedbacku EMS
  if (flags.emsFeedbackActive) {
    // EMS aktywny = system włączony
    flags.systemActive = 1;
    digitalWrite(REL_LOAD_PIN, HIGH);
    digitalWrite(REL_BYPASS_PIN, HIGH);
    digitalWrite(REL_A_B_PIN, HIGH);
    systemMode = 1; // Bezpieczna pozycja
  } else {
    // EMS nieaktywny = system wyłączony
    systemMode = 0;
    flags.systemActive = 0;
    setAllOutputsOffExceptD3();
  }
  
  // Powrót z trybu awaryjnego
  if (commFlags.communicationWithMega && emergencyMode) {
    emergencyMode = false;
    flags.batteryLampBlinking = 0;
    digitalWrite(BATTERY_LAMP_PIN, LOW);
  }
}
