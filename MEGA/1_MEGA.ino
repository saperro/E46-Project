#include <Arduino.h>
#include "mpc.h"
#include "ems.h"
#include "door.h"
#include "mlc.h"
#include "emp.h"
#include "mswf.h"
#include "mcc.h"
#include "multimedia.h"
#include "mtc.h"
#include "mas.h"
#include "atcm.h"
#include "ike.h"
#include "ignition.h"
#include "debug_menu.h"
#include "k_bus.h"
#include "can_.h"
#include "can_readings.h"
#include "sd_card.h"

//PINS
const uint8_t ACC_PIN PROGMEM = 22;
const uint8_t OIL_PIN PROGMEM = 11;
//TIME VARIABLES
unsigned long lastRequestTime = 0;
unsigned long lastRequestTime1 = 0;
unsigned long lastMainUpdate = 0; //ZMIENNA POMOCNICZA DO "TAKTOWANIA" LOOP





// Zmienne stanu
uint8_t state_counter = 0; //TEN COUNTER JEST W LOOPIE I UŁATWIA MIGANIE CZYMKOLWIEK
const unsigned long requestInterval = 1000;
unsigned long requestInterval1 = 1000;

void setup() {
  Wire.begin(); // Inicjalizacja I2C jako Master
  Wire.setClock(400000); // Ustawienie I2C na 400 kHz (Fast Mode)
  Serial.begin(9600);
  kbus.begin(9600, SERIAL_8E1);
  CAN.begin(CAN_500KBPS);
  if (!sd_card_start()) {
    Serial.println("NO SD CARD");
  }
  //================================
  pinMode(POSITION1, INPUT_PULLUP);
  pinMode(POSITION2, INPUT_PULLUP);
  pinMode(POSITION3, INPUT_PULLUP);
  pinMode(POSITION1_OUTPUT_PIN, OUTPUT);
  pinMode(SPEED_PIN, OUTPUT);
  pinMode(ACC_PIN, OUTPUT);
  pinMode(MLC_RESET_PIN, OUTPUT);
  pinMode(EMS_RESET_PIN, OUTPUT);
  pinMode(EMP_RESET_PIN, OUTPUT);
  pinMode(MCC_RESET_PIN, OUTPUT);
  pinMode(MPC_RESET_PIN, OUTPUT);
  pinMode(MSWF_RESET_PIN, OUTPUT);
  pinMode(MAS_RESET_PIN, OUTPUT);
  //USTALIĆ I DODAĆ RESZTĘ RESETÓW

  digitalWrite(POSITION1_OUTPUT_PIN, HIGH);
  digitalWrite(ACC_PIN, HIGH);
  delay(10);
  digitalWrite(ACC_PIN, LOW);
  //  digitalWrite(POSITION1_OUTPUT_PIN, LOW));
  digitalWrite(MLC_RESET_PIN, HIGH);
  digitalWrite(MLC_RESET_PIN, LOW);
  digitalWrite(EMS_RESET_PIN, HIGH);
  digitalWrite(EMS_RESET_PIN, LOW);
  digitalWrite(EMP_RESET_PIN, HIGH);
  digitalWrite(EMP_RESET_PIN, LOW);
  digitalWrite(MCC_RESET_PIN, HIGH);
  digitalWrite(MCC_RESET_PIN, LOW);
  digitalWrite(MPC_RESET_PIN, HIGH);
  digitalWrite(MPC_RESET_PIN, LOW);
  digitalWrite(MAS_RESET_PIN, HIGH);
  digitalWrite(MAS_RESET_PIN, LOW);
  digitalWrite(MSWF_RESET_PIN, HIGH);
  digitalWrite(MSWF_RESET_PIN, LOW);
  delay(1500);
  //tu jest problem. Jeśli resetujemy mswf to trzeba dodać delay
  //można nie resetowć tu mswf i z bani xd
  //zdaje się, że opóźnienie powodowane jest przez bootloader,
  //================================

}

//==LOOP==LOOP==LOOP==LOOP==LOOP==LOOP==LOOP==LOOP==LOOP==LOOP==LOOP==LOOP==LOOP==LOOP==LOOP==LOOP==LOOP==LOOP
void loop() {
  unsigned long now = millis();
  state_counter = !state_counter; // potrzebne do pulsowania kontrolkami

  //  // MAIN LOOP - normalny tryb pracy
  //  if (now - lastMainUpdate >= 50) {
  //    lastMainUpdate = now;
  //    switch (counter) {
  //      case 0: i2cMPC(); counter++; break;
  //      case 1: i2cLightsMLC(); counter++; break;
  //      case 2: i2cDoorsEMS(); counter++; break;
  //      case 3: i2cMSWF(); counter++; break;
  //      case 4: MPCautodiagnostic(); counter++; break;
  //      case 5: MLCautodiagnostic(); counter++; break;
  //      case 6: EMSautodiagnostic(); counter++; break;
  //      case 7: counter = 0; break;
  //    }
  //  }
  debugMenu();// samo sprawdza czy może się wyświetlić
  monitor_can_messages();//słucha i odczytuje przebieg i poziom paliwa

}
