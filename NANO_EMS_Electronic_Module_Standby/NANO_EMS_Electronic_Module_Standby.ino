#include <Wire.h>
#define SLEEP_RELAY_OUT 1
#define ALARM_DETECTION_INPUT 0
#define MAN_DETECTION_INPUT 2
#define REMOTE_DETECTION_INPUT 3
#define MULTIFUNCTION_MODULE_WAKE_UP_OUT 4
#define AIR_SUSPENSION_POWERING_UP_OUT 5
#define AIR_SUSPENSION_STEERING_OUT 6
#define MAIN_CONTROL_UNIT_POWERING_UP_OUT 7
#define LEFT_FRONT_DOOR_OPEN_DETECTION_INPUT 8
#define RIGHT_FRONT_DOOR_OPEN_DETECTION_INPUT 9
#define LEFT_REAR_DOOR_OPEN_DETECTION_INPUT 10
#define RIGHT_REAR_DOOR_OPEN_DETECTION_INPUT 11
#define TRUNK_DOOR_OPEN_DETECTION_INPUT 12
#define TIMER_ACTIVATED_LED_OUT 13
#define SWITCHES_BACKLIGHT_OUT A3
#define TRUNK_LIGHT_OUT A6
#define INDOOR_LIGHT_OUT A7

char alarmDetectionState = ' ';
char manDetectionState = ' ';
char remoteDetectionState = ' ';
char leftFrontDoorState = ' ';
char rightFrontDoorState = ' ';
char leftRearDoorState = ' ';
char rightRearDoorState = ' ';
char trunkDoorState = ' ';

int debug_counter = 0;
unsigned long lastPrintTime = 0;
const unsigned long interval = 2000; 

void requestEvent() {
  char data[7] = {alarmDetectionState, manDetectionState, leftFrontDoorState, rightFrontDoorState, leftRearDoorState, rightRearDoorState, trunkDoorState};
  Wire.write(data, 7); // Wysyłamy całą tablicę
  //Wire.write('B'); // Wysyłamy znak 'A' gdy Master pyta
}

void setup() {
  Wire.begin(9); // Ustawienie adresu I2C = 8
  Wire.setClock(400000); // Ustawienie I2C na 400 kHz (Fast Mode)
  Wire.onRequest(requestEvent); // Obsługa zapytań od Mastera
  Serial.begin(9600);
  pinMode(SLEEP_RELAY_OUT, OUTPUT);
  pinMode(ALARM_DETECTION_INPUT, INPUT_PULLUP);
  pinMode(MAN_DETECTION_INPUT, INPUT_PULLUP);
  pinMode(REMOTE_DETECTION_INPUT, INPUT_PULLUP);
  pinMode(MULTIFUNCTION_MODULE_WAKE_UP_OUT, OUTPUT);
  pinMode(AIR_SUSPENSION_POWERING_UP_OUT, OUTPUT);
  pinMode(AIR_SUSPENSION_STEERING_OUT , OUTPUT);
  pinMode(MAIN_CONTROL_UNIT_POWERING_UP_OUT , OUTPUT);
  pinMode(LEFT_FRONT_DOOR_OPEN_DETECTION_INPUT , INPUT_PULLUP);
  pinMode(RIGHT_FRONT_DOOR_OPEN_DETECTION_INPUT , INPUT_PULLUP);
  pinMode(LEFT_REAR_DOOR_OPEN_DETECTION_INPUT , INPUT_PULLUP);
  pinMode(RIGHT_REAR_DOOR_OPEN_DETECTION_INPUT , INPUT_PULLUP);
  pinMode(TRUNK_DOOR_OPEN_DETECTION_INPUT , INPUT_PULLUP);
  pinMode(TIMER_ACTIVATED_LED_OUT, OUTPUT);
  pinMode(SWITCHES_BACKLIGHT_OUT , OUTPUT);
  pinMode(TRUNK_LIGHT_OUT , OUTPUT);
  pinMode(INDOOR_LIGHT_OUT , OUTPUT);
}
void loop() {
  pinsState();

}

void pinsState() {
  unsigned long currentMillis = millis();

  // Aktualizacja stanów (ciągła)
  alarmDetectionState = digitalRead(ALARM_DETECTION_INPUT) == LOW ? 'W' : 'K';
  manDetectionState = digitalRead(MAN_DETECTION_INPUT) == LOW ? 'W' : 'K';
  remoteDetectionState = digitalRead(REMOTE_DETECTION_INPUT) == LOW ? 'W' : 'K';
  leftFrontDoorState = digitalRead(LEFT_FRONT_DOOR_OPEN_DETECTION_INPUT) == LOW ? 'W' : 'K';
  rightFrontDoorState = digitalRead(RIGHT_FRONT_DOOR_OPEN_DETECTION_INPUT) == LOW ? 'W' : 'K';
  leftRearDoorState = digitalRead(LEFT_REAR_DOOR_OPEN_DETECTION_INPUT) == LOW ? 'W' : 'K';
  rightRearDoorState = digitalRead(RIGHT_REAR_DOOR_OPEN_DETECTION_INPUT) == LOW ? 'W' : 'K';
  trunkDoorState = digitalRead(TRUNK_DOOR_OPEN_DETECTION_INPUT) == LOW ? 'W' : 'K';

 /* // Wypisywanie na Serial co 1 sekundę
  if (currentMillis - lastPrintTime >= interval) {
    lastPrintTime = currentMillis;

    Serial.println("----- System Status -----");
    if (alarmDetectionState == 'W') Serial.println("Alarm Detection Activated");
    else Serial.println("Alarm Detection Deactivated");

    if (manDetectionState == 1) Serial.println("Man Detected");
    else Serial.println("Man Not Detected");

    if (remoteDetectionState == 1) Serial.println("Remote Detected");
    else Serial.println("Remote Undetected");

    if (leftFrontDoorState == 'W') Serial.println("Left Front Door Open");
    else Serial.println("Left Front Door Closed");

    if (rightFrontDoorState == 'W') Serial.println("Right Front Door Open");
    else Serial.println("Right Front Door Closed");

    if (leftRearDoorState == 'W') Serial.println("Left Rear Door Open");
    else Serial.println("Left Rear Door Closed");

    if (rightRearDoorState == 'W') Serial.println("Right Rear Door Open");
    else Serial.println("Right Rear Door Closed");

    if (trunkDoorState == 'W') Serial.println("Trunk Door Open");
    else Serial.println("Trunk Door Closed");

    Serial.println("-------------------------");
   
  }*/
}
