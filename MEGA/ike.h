#ifndef IKE_H
#define IKE_H
#include <Arduino.h>
#include "can_.h"
#include "k_bus.h"
#include "ems.h"
#include "mlc.h"

extern String currentDebugModule;
extern byte debugMode;

/*Funkcje wysyłania komend po CAN do IKE służą wyłącznie debugowi,
  normalnie tą częścią wskazań IKE steruje oryginalny sterownik silnika BMW.
  Debug powinien działać poprzez wpisanie wartości do zmiennych i uruchomienie odpowiednich funkcji.
  Pozostałe kontrolki uruchamiane są z K-Bus, więc nie trzeba niczego wysyłać na CAN.
  Z*/
unsigned long lastEconomizerUpdate = 0;
unsigned long economizer_value = 0;
uint8_t AT_Gear = 0x02;
uint8_t LSBdata;//ekonomizer
uint8_t MSBdata;//ekonomizer
const int MaxRPM = 7000;
int rpm_value = 3500;
uint8_t DME4_Load0 = 0x00;
uint8_t DME4_Load3 = 0x00;
uint8_t DME4_Load5 = 0x00;
uint8_t DME6_Load1 = 0x00;
unsigned long aktualnyCzas = 0;//TEST
unsigned long zapamietanyCzas = 0;//TEST
unsigned long roznicaCzasu = 0;//TEST
uint8_t dupa = 0; //TEST
int receivedByte; //TEST

//TEMPERATURE - DEBUG ONLY
int motor_temp_value = 0;
uint8_t przegrzanie_chlodziwa = 0;
uint8_t zolta_kontrolka_oleju_malfunction = 0;

//OTHER BULBS - DEBUG ONLY
byte cruise = 0; //sterowane również z fizycznego pinu
uint8_t checkEngine = 0;
uint8_t eml = 0;
uint8_t trakcja = 0; // domyślnie świeci

//SENDING DATA TO CAN - DEBUG ONLY
extern void CanSend(short address, byte a, byte b, byte c, byte d, byte e, byte f, byte g, byte h); //do wysyłki na CN
//SPEEDOMETER - DEBUG ONLY
const uint8_t SPEED_PIN PROGMEM = 7;
int speed_value = 120;

//ECONOMIZER - DEBUG ONLY
void economizer(unsigned long a) {
  unsigned long now1 = millis();
  if (now1 - lastEconomizerUpdate >= 20) {
    lastEconomizerUpdate = now1;
    a = (economizer_value);
  }
  if (LSBdata + a > 0xFF) { // Check overflow
    MSBdata += 0x01;  // Increment MSB if overflow occurs
  }
  LSBdata += a; // Increment LSB based on injection input
}
void zolta_kontrolka_oleju_czerwona_kontrolka_chlodziwa(byte poziom, byte przegrzanie) {
  byte a = 0;
  if (poziom == 1) {
    a += 0x02;
  }
  if (przegrzanie == 1) {
    a += 0x08;
  }
  DME4_Load3 = a;
}
void cruise_checkEngine_eml(byte cruisea, byte checkEnginea, byte emla) {
  byte a = 0;
  if (cruisea == 1) {
    a += 0x08;
  }
  if (checkEnginea == 1) {
    a += 0x02;
  }
  if (emla == 1) {
    a += 0x14;
  }

  DME4_Load0 = a;
}

void trakcja_(byte a) {
  if (a == 1) {
    DME6_Load1 = 0x01;
  }
  if (a == 0) {
    DME6_Load1 = 0x00;
  }
  CanSend(0x153, 0x00, DME6_Load1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
}

void setRPM(int a) {
  // RPM Scaling to Float based on MaxRPM

  // Clamp the RPM to the defined maximum
  if (a > MaxRPM)
  {
    a = MaxRPM;
  }
  // Map the RPM to a value for the cluster
  float RPMSendVal = map(a, 0, 7000, 0, 175.00f); // 0.00f = 0rpm, 175.00f = 7000rpm

  // RPM
  // L2 = RPM LSB
  // L3 = RPM MSB from 0.00f to 175.00f
  // unsigned char message1[8] = {0x05, 0x62, 0xFF, RPMSendVal, 0x65, 0x12, 0, 62};
  // CAN.sendMsgBuf(0x316, 0, 8, message1);
  CanSend(0x316, 0x05, 0x62, 0xFF, RPMSendVal, 0x65, 0x12, 0, 62);
}
void CAN_DME4_SEND() {
  // DME 4
  // L0 = 0x08=Cruise Control, 0x02=Motor Light
  // L1 = ? Probably L/100km -> Maybe the Change of Rate is needed to calculate in the cluster
  // L3 = 0x08=Overheat Light, 0x02=Oillight
  // L4 = ? Probably Oiltemp -> not as analog in the cluster, so ignored for now
  // L5 = Charging Light 0x00=off, 0x01=on
  CanSend(0x545, DME4_Load0, LSBdata, MSBdata, DME4_Load3, 0x00, DME4_Load5, 0x00, 0x00);
}
void AT_Display(int AT_Gear) {
  CanSend(0x43F, 0x00, AT_Gear, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00);
}
void czyszczenie_pizzy() {
  CanSend(0x153, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
}
void temperaturaSilnika(int a) {
  // Coolant Temperature
  // Calculation, maxTemp=260.00f, minTemp=50.00f
  // Temp Scaling to Float based on temperaturerange from 50 to 130 Degree Celsius

  // Map the Temp to a value for the cluster
  if (a >= 130) {
    a = 125;  // Keep Temp in save Value
  }
  if (a <= 50) {
    a = 50;
  }
  if (a >= 110) {
    przegrzanie_chlodziwa = 1;
  }
  else {
    przegrzanie_chlodziwa = 0;
  }
  float TempSendVal = map(a, 50, 130, 50.00f, 260.00f);
  CanSend(0x329, 0x00, TempSendVal, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
}

void Speedometer(int a)//predkosc a nie wartosc PWM: 0-250km/h
{
  if (a == 0)
  {
    noTone(SPEED_PIN);
  } else {
    int SpeedSendVal = map(a, 0, 250, 0, 1680);
    tone(SPEED_PIN, SpeedSendVal); // 250KmH=1680, 0KmH=0
  }
}
void DME4_Load0Test()
{
  // if (Serial.available() > 0) {
  //  String inputString = Serial.readStringUntil('\n'));  // Odczytanie całej linii do znaku nowej linii
  //  byte receivedByte = inputString.toInt(); // Konwersja na liczbę dziesiętną (np. "10" → 10)
  // }
  aktualnyCzas = millis();
  roznicaCzasu = aktualnyCzas - zapamietanyCzas;
  if (roznicaCzasu >= 5000UL) {
    zapamietanyCzas = aktualnyCzas;
    receivedByte += 1;
    Serial.print(F("TEST:"));
    Serial.println(receivedByte);
    dupa = receivedByte;

    //byte *data;
    //  byte mes1[] = {0x00, 0x05, 0xBF, 0x7A, LightByte1, LightByte3, 0x00, 0x00, 0x00, 0x00, 0x00}; //bagaznik 0x32
    // data = mes1;
    // int end_i = data[1] + 2;
    //  data[end_i - 1] = iso_checksum(data, end_i - 1);
    //  kbus.write(data, end_i + 1);
    if (receivedByte >= 255) {
      receivedByte = 0;
    }
  }
  //CanSend(0x545, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00);
  //CanSend(0x545, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFE, 0xFF));
  //CanSend(0x338, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF));
  //CanSend(0x329, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF));
  //CanSend(0x613, 0x00, 0x00, dupa, 0x00, 0x00, 0x00, 0x00, 0x00);//- POZIOM PALIWA - wstępne testy zawiodły
  CanSend(0x338, dupa, dupa, dupa, dupa, dupa, dupa, dupa, dupa);
  //CanSend(0x153, 0x00, 0x00, 0x00, dupa, 0x00, 0x00, 0x00, 0x00);
  //delay(50);
}

void ike_debug_menu() {
  if (currentDebugModule == "ike" || currentDebugModule == "IKE") {
    Serial.println(F("==============IKE (CAN/K-Bus)=============="));
    Serial.println(F("--- WSKAŹNIKI I KONTROLKI CAN (DME/EGS) ---"));
    // Opcje dla TEMPERATURE
    Serial.println(F("--> MOTORTEMP [0-130] Set motor temperature"));
    Serial.println(F("--> CHLODZIWO_ON / OFF Turn on/off red coolant light"));
    Serial.println(F("--> OLEJ_ON / OFF Turn on/off yellow oil light"));

    // Opcje dla OTHER BULBS
    Serial.println(F("--> CRUISE_ON / OFF Turn on/off cruise control light"));
    Serial.println(F("--> CHECKENGINE_ON / OFF Turn on/off check engine light"));
    Serial.println(F("--> EML_ON / OFF Turn on/off EML light"));
    Serial.println(F("--> TRAKCJA_ON / OFF Turn on/off traction control light"));

    // Opcje dla wskaźników
    Serial.println(F("--> RPM [0-7000] Set RPM value"));
    Serial.println(F("--> SPEED [0-250] Set Speed value (km/h)"));
    Serial.println(F("--> AT_GEAR [0-15] Set AT gear display value (0x00-0x0F)"));
    Serial.println(F("--> ECONOMIZER [0-255] Set Economizer value"));

    // Opcje systemowe
    Serial.println(F("--> DME4_SEND Send CAN_DME4_SEND command"));
    Serial.println(F("--> CZYSZCZENIE_PIZZY Send czyszczenie_pizzy command"));
    
    // -------------------------------------------
    // --- KONTROLKI K-BUS (EMS/MLC) ---
    Serial.println(F("--- KONTROLKI K-BUS (Drzwi, Światła, Żarówki) ---"));
    // Opcje Drzwi (MLC)
    Serial.println(F("--> LPDRZWI_ON / OFF - Lewe przednie drzwi (0x01)"));
    Serial.println(F("--> PPDRZWI_ON / OFF - Prawe przednie drzwi (0x02)"));
    Serial.println(F("--> LTDRZWI_ON / OFF - Lewe tylne drzwi (0x04)"));
    Serial.println(F("--> PTDRZWI_ON / OFF - Prawe tylne drzwi (0x08)"));
    Serial.println(F("--> BAGAZNIK_ON / OFF - Bagażnik (0x32)"));
    // Opcje Kierunkowskazów/Świateł (MLC)
    Serial.println(F("--> LEWY_ON / OFF - Lewy kierunkowskaz (0x20)"));
    Serial.println(F("--> PRAWY_ON / OFF - Prawy kierunkowskaz (0x40)"));
    Serial.println(F("--> HAZARD_ON / OFF - Światła awaryjne (0x60)"));
    Serial.println(F("--> DLUGIE_ON / OFF - Światła drogowe (0x04)"));
    Serial.println(F("--> PRZECIWMGIALNEPRZOD_ON / OFF - Mgielne przód (0x10)"));
    Serial.println(F("--> PRZECIWMGIALNETYL_ON / OFF - Mgielne tył (0x08)"));
    // Opcje Kontrolek Żarówek (EMS)
    Serial.println(F("--> LZAR_ON / OFF - Lewa przednia żarówka (0x20)"));
    Serial.println(F("--> LTZAR_ON / OFF - Lewa tylna żarówka (0x08)"));
    Serial.println(F("--> PZAR_ON / OFF - Prawa przednia żarówka (0x10)"));
    Serial.println(F("--> PTZAR_ON / OFF - Prawa tylna żarówka (0x04)"));
    Serial.println(F("--> WNETRZE_ON / OFF - Oświetlenie wnętrza (0x80)"));

    Serial.println(F("--> MAIN - to main menu"));
    Serial.println(F("--> EXIT - to normal mode"));
    currentDebugModule = "dupa";
  }
  // --- Opcje CAN (bez zmian) ---
  else if (currentDebugModule.startsWith("MOTORTEMP")) {
    int temp = currentDebugModule.substring(9).toInt(); // MOTORTEMPXXX
    temperaturaSilnika(temp);
    motor_temp_value = temp;
    // Aktualizujemy stan kontrolki chłodziwa
    zolta_kontrolka_oleju_czerwona_kontrolka_chlodziwa(zolta_kontrolka_oleju_malfunction, przegrzanie_chlodziwa);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "CHLODZIWO_ON") {
    przegrzanie_chlodziwa = 1;
    zolta_kontrolka_oleju_czerwona_kontrolka_chlodziwa(zolta_kontrolka_oleju_malfunction, przegrzanie_chlodziwa);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "CHLODZIWO_OFF") {
    przegrzanie_chlodziwa = 0;
    zolta_kontrolka_oleju_czerwona_kontrolka_chlodziwa(zolta_kontrolka_oleju_malfunction, przegrzanie_chlodziwa);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "OLEJ_ON") {
    zolta_kontrolka_oleju_malfunction = 1;
    zolta_kontrolka_oleju_czerwona_kontrolka_chlodziwa(zolta_kontrolka_oleju_malfunction, przegrzanie_chlodziwa);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "OLEJ_OFF") {
    zolta_kontrolka_oleju_malfunction = 0;
    zolta_kontrolka_oleju_czerwona_kontrolka_chlodziwa(zolta_kontrolka_oleju_malfunction, przegrzanie_chlodziwa);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "CRUISE_ON") {
    cruise = 1;
    cruise_checkEngine_eml(cruise, checkEngine, eml);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "CRUISE_OFF") {
    cruise = 0;
    cruise_checkEngine_eml(cruise, checkEngine, eml);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "CHECKENGINE_ON") {
    checkEngine = 1;
    cruise_checkEngine_eml(cruise, checkEngine, eml);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "CHECKENGINE_OFF") {
    checkEngine = 0;
    cruise_checkEngine_eml(cruise, checkEngine, eml);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "EML_ON") {
    eml = 1;
    cruise_checkEngine_eml(cruise, checkEngine, eml);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "EML_OFF") {
    eml = 0;
    cruise_checkEngine_eml(cruise, checkEngine, eml);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "TRAKCJA_ON") {
    trakcja = 1;
    trakcja_(trakcja);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "TRAKCJA_OFF") {
    trakcja = 0;
    trakcja_(trakcja);
    currentDebugModule = "main";
  }
  else if (currentDebugModule.startsWith("RPM")) {
    int rpm = currentDebugModule.substring(3).toInt(); // RPMXXXX
    setRPM(rpm);
    rpm_value = rpm;
    currentDebugModule = "main";
  }
  else if (currentDebugModule.startsWith("SPEED")) {
    int speed = currentDebugModule.substring(5).toInt(); // SPEEDXXX
    Speedometer(speed);
    speed_value = speed;
    currentDebugModule = "main";
  }
  else if (currentDebugModule.startsWith("AT_GEAR")) {
    int gear = currentDebugModule.substring(7).toInt(); // AT_GEARX
    AT_Display(gear);
    AT_Gear = (uint8_t)gear;
    currentDebugModule = "main";
  }
  else if (currentDebugModule.startsWith("ECONOMIZER")) {
    unsigned long econ = currentDebugModule.substring(10).toInt(); // ECONOMIZERXXX
    economizer_value = econ; // Ustawia wartość, która będzie używana w funkcji economizer
    economizer(econ);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "DME4_SEND") {
    CAN_DME4_SEND();
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "CZYSZCZENIE_PIZZY") {
    czyszczenie_pizzy();
    currentDebugModule = "main";
  }
  
  // --- NOWE OPCJE K-BUS (MLC: Drzwi/Światła) ---
  else if (currentDebugModule == "LPDRZWI_ON") {
    lewe_przednie_drzwi = 1;
    drzwi(prawe_przednie_drzwi, prawe_tylne_drzwi, lewe_przednie_drzwi, lewe_tylne_drzwi, bagaznik);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "LPDRZWI_OFF") {
    lewe_przednie_drzwi = 0;
    drzwi(prawe_przednie_drzwi, prawe_tylne_drzwi, lewe_przednie_drzwi, lewe_tylne_drzwi, bagaznik);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "PPDRZWI_ON") {
    prawe_przednie_drzwi = 1;
    drzwi(prawe_przednie_drzwi, prawe_tylne_drzwi, lewe_przednie_drzwi, lewe_tylne_drzwi, bagaznik);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "PPDRZWI_OFF") {
    prawe_przednie_drzwi = 0;
    drzwi(prawe_przednie_drzwi, prawe_tylne_drzwi, lewe_przednie_drzwi, lewe_tylne_drzwi, bagaznik);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "LTDRZWI_ON") {
    lewe_tylne_drzwi = 1;
    drzwi(prawe_przednie_drzwi, prawe_tylne_drzwi, lewe_przednie_drzwi, lewe_tylne_drzwi, bagaznik);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "LTDRZWI_OFF") {
    lewe_tylne_drzwi = 0;
    drzwi(prawe_przednie_drzwi, prawe_tylne_drzwi, lewe_przednie_drzwi, lewe_tylne_drzwi, bagaznik);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "PTDRZWI_ON") {
    prawe_tylne_drzwi = 1;
    drzwi(prawe_przednie_drzwi, prawe_tylne_drzwi, lewe_przednie_drzwi, lewe_tylne_drzwi, bagaznik);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "PTDRZWI_OFF") {
    prawe_tylne_drzwi = 0;
    drzwi(prawe_przednie_drzwi, prawe_tylne_drzwi, lewe_przednie_drzwi, lewe_tylne_drzwi, bagaznik);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "BAGAZNIK_ON") {
    bagaznik = 1;
    drzwi(prawe_przednie_drzwi, prawe_tylne_drzwi, lewe_przednie_drzwi, lewe_tylne_drzwi, bagaznik);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "BAGAZNIK_OFF") {
    bagaznik = 0;
    drzwi(prawe_przednie_drzwi, prawe_tylne_drzwi, lewe_przednie_drzwi, lewe_tylne_drzwi, bagaznik);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "LEWY_ON") {
    lewy_kierunkowskaz = 1;
    kontrolki_swiatel(lewy_kierunkowskaz, prawy_kierunkowskaz, dlugie_swiatla, przeciwmgielne_przednie, przeciwmgielne_tylne);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "LEWY_OFF") {
    lewy_kierunkowskaz = 0;
    kontrolki_swiatel(lewy_kierunkowskaz, prawy_kierunkowskaz, dlugie_swiatla, przeciwmgielne_przednie, przeciwmgielne_tylne);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "PRAWY_ON") {
    prawy_kierunkowskaz = 1;
    kontrolki_swiatel(lewy_kierunkowskaz, prawy_kierunkowskaz, dlugie_swiatla, przeciwmgielne_przednie, przeciwmgielne_tylne);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "PRAWY_OFF") {
    prawy_kierunkowskaz = 0;
    kontrolki_swiatel(lewy_kierunkowskaz, prawy_kierunkowskaz, dlugie_swiatla, przeciwmgielne_przednie, przeciwmgielne_tylne);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "HAZARD_ON") {
    hazard = 1;
    kontrolki_swiatel(lewy_kierunkowskaz, prawy_kierunkowskaz, dlugie_swiatla, przeciwmgielne_przednie, przeciwmgielne_tylne);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "HAZARD_OFF") {
    hazard = 0;
    kontrolki_swiatel(lewy_kierunkowskaz, prawy_kierunkowskaz, dlugie_swiatla, przeciwmgielne_przednie, przeciwmgielne_tylne);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "DLUGIE_ON") {
    dlugie_swiatla = 1;
    kontrolki_swiatel(lewy_kierunkowskaz, prawy_kierunkowskaz, dlugie_swiatla, przeciwmgielne_przednie, przeciwmgielne_tylne);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "DLUGIE_OFF") {
    dlugie_swiatla = 0;
    kontrolki_swiatel(lewy_kierunkowskaz, prawy_kierunkowskaz, dlugie_swiatla, przeciwmgielne_przednie, przeciwmgielne_tylne);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "PRZECIWMGIALNEPRZOD_ON") {
    przeciwmgielne_przednie = 1;
    kontrolki_swiatel(lewy_kierunkowskaz, prawy_kierunkowskaz, dlugie_swiatla, przeciwmgielne_przednie, przeciwmgielne_tylne);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "PRZECIWMGIALNEPRZOD_OFF") {
    przeciwmgielne_przednie = 0;
    kontrolki_swiatel(lewy_kierunkowskaz, prawy_kierunkowskaz, dlugie_swiatla, przeciwmgielne_przednie, przeciwmgielne_tylne);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "PRZECIWMGIALNETYL_ON") {
    przeciwmgielne_tylne = 1;
    kontrolki_swiatel(lewy_kierunkowskaz, prawy_kierunkowskaz, dlugie_swiatla, przeciwmgielne_przednie, przeciwmgielne_tylne);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "PRZECIWMGIALNETYL_OFF") {
    przeciwmgielne_tylne = 0;
    kontrolki_swiatel(lewy_kierunkowskaz, prawy_kierunkowskaz, dlugie_swiatla, przeciwmgielne_przednie, przeciwmgielne_tylne);
    currentDebugModule = "main";
  }

  // --- NOWE OPCJE K-BUS (EMS: Żarówki) ---
  else if (currentDebugModule == "LZAR_ON") {
    // Używa 'zarowka_lewa_tyl' jako placeholder, ponieważ 'zarowka_lewa_przod' nie jest zadeklarowana.
    zarowka_lewa_tyl = 1; 
    zarowki(1, zarowka_lewa_tyl, zarowka_prawa_przod, zarowka_prawa_tyl, zarowka_wnetrze);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "LZAR_OFF") {
    zarowka_lewa_tyl = 0;
    zarowki(0, zarowka_lewa_tyl, zarowka_prawa_przod, zarowka_prawa_tyl, zarowka_wnetrze);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "LTZAR_ON") {
    zarowka_lewa_tyl = 1;
    zarowki(0, zarowka_lewa_tyl, zarowka_prawa_przod, zarowka_prawa_tyl, zarowka_wnetrze);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "LTZAR_OFF") {
    zarowka_lewa_tyl = 0;
    zarowki(0, zarowka_lewa_tyl, zarowka_prawa_przod, zarowka_prawa_tyl, zarowka_wnetrze);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "PZAR_ON") {
    zarowka_prawa_przod = 1;
    zarowki(0, zarowka_lewa_tyl, zarowka_prawa_przod, zarowka_prawa_tyl, zarowka_wnetrze);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "PZAR_OFF") {
    zarowka_prawa_przod = 0;
    zarowki(0, zarowka_lewa_tyl, zarowka_prawa_przod, zarowka_prawa_tyl, zarowka_wnetrze);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "PTZAR_ON") {
    zarowka_prawa_tyl = 1;
    zarowki(0, zarowka_lewa_tyl, zarowka_prawa_przod, zarowka_prawa_tyl, zarowka_wnetrze);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "PTZAR_OFF") {
    zarowka_prawa_tyl = 0;
    zarowki(0, zarowka_lewa_tyl, zarowka_prawa_przod, zarowka_prawa_tyl, zarowka_wnetrze);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "WNETRZE_ON") {
    zarowka_wnetrze = 1;
    zarowki(0, zarowka_lewa_tyl, zarowka_prawa_przod, zarowka_prawa_tyl, zarowka_wnetrze);
    currentDebugModule = "main";
  }
  else if (currentDebugModule == "WNETRZE_OFF") {
    zarowka_wnetrze = 0;
    zarowki(0, zarowka_lewa_tyl, zarowka_prawa_przod, zarowka_prawa_tyl, zarowka_wnetrze);
    currentDebugModule = "main";
  }
}
#endif
