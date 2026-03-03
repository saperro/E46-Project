#include <Wire.h>
#define LEFT_FRONT_BULB_ERR_DET_INPUT_NON_PULUP 2
#define RIGHT_FRONT_BULB_ERR_INPUT_NON_PULLUP 3
#define LEFT_REAR_BULB_ERR_INPUT_NON_PULLUP 4
#define RIGHT_REAR_BULB_ERR_INPUT_NON_PULLUP 5
#define IKE_CLUSTER_BACKLIGHT_OUTPUT 10
#define BUTTONS_BACKLIGHT_OUTPUT 11
#define COMBINATION_SWITCH_POWER_OUTPUT 12
#define MAIN_LIGHTS_POWER_LINE_OUTPUT 13
#define LEFT_TURN_SIGNAL_INPUT_PULLUP A0
#define RIGHT_TURN_SIGNAL_INPUT_PULLUP A1
#define HIGH_BEAM_INPUT_PULLUP A2
#define FOG_REAR_LIGHT_INPUT_PULLUP A3
#define FOG_FRONT_LIGHT_INPUT_PULLUP A6
#define NIGHT_MODE_INPUT_PULLUP A7

char leftFrontBulbErr = ' ';
char rightFrontBulbErr = ' ';
char leftRearBulbErr = ' ';
char rightRearBulbErr = ' ';
char turnLeftSignalState = ' ';
char turnRightSignalState = ' ';
char highBeamLightState = ' ';
char fogFrontLightState = ' ';
char fogRearLightState = ' ';
char nightModeState = ' ';

int debug_counter = 0;
unsigned long lastPrintTime = 0;
const unsigned long interval = 2000;

void requestEvent() {
  char data[9] = {leftFrontBulbErr, rightFrontBulbErr, leftRearBulbErr, rightRearBulbErr, turnLeftSignalState, turnRightSignalState, highBeamLightState, fogRearLightState, fogFrontLightState};
  Wire.write(data, 9); // Wysyłamy całą tablicę

}
void setup() {
  Wire.begin(8); // Ustawienie adresu I2C = 9
  Wire.setClock(400000);
  Wire.onRequest(requestEvent);
  pinMode(LEFT_FRONT_BULB_ERR_DET_INPUT_NON_PULUP, INPUT);
  pinMode(RIGHT_FRONT_BULB_ERR_INPUT_NON_PULLUP, INPUT);
  pinMode(LEFT_REAR_BULB_ERR_INPUT_NON_PULLUP, INPUT);
  pinMode(RIGHT_REAR_BULB_ERR_INPUT_NON_PULLUP, INPUT);
  digitalWrite(IKE_CLUSTER_BACKLIGHT_OUTPUT, HIGH);
  pinMode(IKE_CLUSTER_BACKLIGHT_OUTPUT, OUTPUT);
  pinMode(BUTTONS_BACKLIGHT_OUTPUT, OUTPUT);
  pinMode(COMBINATION_SWITCH_POWER_OUTPUT, OUTPUT);
  pinMode(MAIN_LIGHTS_POWER_LINE_OUTPUT, OUTPUT);
  pinMode(LEFT_TURN_SIGNAL_INPUT_PULLUP, INPUT_PULLUP);
  pinMode(RIGHT_TURN_SIGNAL_INPUT_PULLUP, INPUT_PULLUP);
  pinMode(HIGH_BEAM_INPUT_PULLUP, INPUT_PULLUP);
  pinMode(FOG_REAR_LIGHT_INPUT_PULLUP, INPUT_PULLUP);
  pinMode(FOG_FRONT_LIGHT_INPUT_PULLUP, INPUT_PULLUP);
  pinMode(NIGHT_MODE_INPUT_PULLUP, INPUT_PULLUP);
}

void loop() {
  pinsState();
  fuck_logic();

}

void pinsState() {
  unsigned long currentMillis = millis();

  turnLeftSignalState = digitalRead(LEFT_TURN_SIGNAL_INPUT_PULLUP) == LOW ? 'W' : 'K';
  turnRightSignalState = digitalRead(RIGHT_TURN_SIGNAL_INPUT_PULLUP) == LOW ? 'W' : 'K';
  highBeamLightState = digitalRead(HIGH_BEAM_INPUT_PULLUP) == LOW ? 'W' : 'K';
  fogRearLightState = digitalRead(FOG_REAR_LIGHT_INPUT_PULLUP)== LOW ? 'W' : 'K';
  fogFrontLightState  = analogRead(FOG_FRONT_LIGHT_INPUT_PULLUP) < 180 ? 'W' : 'K';
  nightModeState = analogRead(NIGHT_MODE_INPUT_PULLUP) < 180 ? 'W' : 'K';
  if (nightModeState == 'W') {
    leftFrontBulbErr = 'W';
    rightFrontBulbErr = 'W';
    leftRearBulbErr = 'W';
    rightRearBulbErr = 'W';
  }
  if (nightModeState == 'K') {
    leftFrontBulbErr = digitalRead(LEFT_FRONT_BULB_ERR_DET_INPUT_NON_PULUP) == LOW ? 'K' : 'W';
    rightFrontBulbErr = digitalRead(RIGHT_FRONT_BULB_ERR_INPUT_NON_PULLUP) == LOW ? 'K' : 'W';
    leftRearBulbErr = digitalRead(LEFT_REAR_BULB_ERR_INPUT_NON_PULLUP) == LOW ? 'K' : 'W';
    rightRearBulbErr = digitalRead(RIGHT_REAR_BULB_ERR_INPUT_NON_PULLUP) == LOW ? 'K' : 'W';
  }
}
void fuck_logic() {
  if (nightModeState == 'W') {// LOW
    digitalWrite(BUTTONS_BACKLIGHT_OUTPUT, HIGH);
    digitalWrite(COMBINATION_SWITCH_POWER_OUTPUT, LOW);
    digitalWrite(MAIN_LIGHTS_POWER_LINE_OUTPUT, LOW);
  }
  if (nightModeState == 'K') {//HIGH
    digitalWrite(BUTTONS_BACKLIGHT_OUTPUT, LOW);
    digitalWrite(COMBINATION_SWITCH_POWER_OUTPUT, HIGH);
    digitalWrite(MAIN_LIGHTS_POWER_LINE_OUTPUT, HIGH);
  }
}
