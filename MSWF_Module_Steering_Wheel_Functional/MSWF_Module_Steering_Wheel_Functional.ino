#include <Wire.h>

// Definicje pinów
#define TEMPOMAT_DOWN 2
#define TEMPOMAT_OFF 3
#define TEMPOMAT_UP 4
#define TEMPOMAT_ON 5
#define VOLUME_DOWN 6
#define MENU_DOWN 7
#define MUTE_ 8
#define ENTER_ 9
#define CONNECT_DISCONNECT 10
#define MENU_UP 11
#define VOLUME_UP 12

// Zmienne znakowe przycisków
char tempomatDown = 'K';
char tempomatOff = 'K';
char tempomatUp = 'K';
char tempomatOn = 'K';
char volumeDown = 'K';
char menuDown = 'K';
char mute = 'K';
char enter = 'K';
char connectDisconnet = 'K';
char menuUp = 'K';
char volumeUp = 'K';
char pusty = 'K';

// Struktura dla przycisków z obsługą krótkiego/długiego wciśnięcia
struct Button {
  int pin;
  bool isPressed;
  unsigned long pressStartTime;
  char* resultChar;
  bool hasEvent;
};

// Tylko te 4 przyciski mają obsługę długości wciśnięcia
Button buttons[] = {
  {TEMPOMAT_DOWN, false, 0, &tempomatDown, false},
  {TEMPOMAT_UP,   false, 0, &tempomatUp, false},
  {ENTER_,        false, 0, &enter, false},
  {MUTE_,         false, 0, &mute, false}
};

const int numButtons = sizeof(buttons) / sizeof(buttons[0]);

void setup() {
  Wire.begin(10); // adres I2C = 10
  Wire.setClock(400000);
  Wire.onRequest(requestEvent);

  // Ustawienie wszystkich pinów jako INPUT_PULLUP
  pinMode(TEMPOMAT_DOWN, INPUT_PULLUP);
  pinMode(TEMPOMAT_OFF, INPUT_PULLUP);
  pinMode(TEMPOMAT_UP, INPUT_PULLUP);
  pinMode(TEMPOMAT_ON, INPUT_PULLUP);
  pinMode(VOLUME_DOWN, INPUT_PULLUP);
  pinMode(MENU_DOWN, INPUT_PULLUP);
  pinMode(MUTE_, INPUT_PULLUP);
  pinMode(ENTER_, INPUT_PULLUP);
  pinMode(CONNECT_DISCONNECT, INPUT_PULLUP);
  pinMode(MENU_UP, INPUT_PULLUP);
  pinMode(VOLUME_UP, INPUT_PULLUP);
}

void loop() {
  pinsState();
  delay(10); // Dodane opóźnienie zapobiegające zawieszaniu
}

// Funkcja wywoływana przy żądaniu danych I2C
void requestEvent() {
  char data[12] = {
    tempomatDown,
    tempomatOff,
    tempomatUp,
    tempomatOn,
    volumeDown,
    menuDown,
    mute,
    enter,
    connectDisconnet,
    menuUp,
    volumeUp,
    pusty
  };

  Wire.write(data, 12);

  // Reset flag po odczytaniu stanu przez mastera
  tempomatDown = 'K';
  tempomatOff = 'K';
  tempomatUp = 'K';
  tempomatOn = 'K';
  volumeDown = 'K';
  menuDown = 'K';
  mute = 'K';
  enter = 'K';
  connectDisconnet = 'K';
  menuUp = 'K';
  volumeUp = 'K';
  pusty = 'K';
  
  // Reset flag zdarzeń - KLUCZOWA ZMIANA
  for (int i = 0; i < numButtons; i++) {
    buttons[i].hasEvent = false;
  }
}

// Obsługa stanu przycisków
void pinsState() {
  unsigned long now = millis();

  for (int i = 0; i < numButtons; i++) {
    bool state = digitalRead(buttons[i].pin);

    if (!buttons[i].isPressed && state == LOW) {
      buttons[i].pressStartTime = now;
      buttons[i].isPressed = true;
    }

    if (buttons[i].isPressed && state == HIGH) {
      unsigned long duration;
      
      // Zabezpieczenie przed przepełnieniem millis()
      if (now < buttons[i].pressStartTime) {
        buttons[i].pressStartTime = now;
        duration = 0;
      } else {
        duration = now - buttons[i].pressStartTime;
      }

      if (duration >= 1000) {
        *(buttons[i].resultChar) = 'B';
      } else if (duration >= 20) {
        *(buttons[i].resultChar) = 'W';
      }

      buttons[i].hasEvent = true;
      buttons[i].isPressed = false;
    }

    if (!buttons[i].isPressed && state == HIGH && !buttons[i].hasEvent) {
      *(buttons[i].resultChar) = 'K';
    }
  }

  // Pozostałe przyciski - tylko W/K
  if (connectDisconnet == 'K') { //JEŚLI JEST W TO MA TAKIE ZOSTAĆ AZ DO ODEBRANIA DANYCH
    connectDisconnet = digitalRead(CONNECT_DISCONNECT) == LOW ? 'W' : 'K';
  }
  if (tempomatOff == 'K') {
    tempomatOff = digitalRead(TEMPOMAT_OFF) == LOW ? 'W' : 'K';
  }
  if (tempomatOn == 'K') {
    tempomatOn = digitalRead(TEMPOMAT_ON) == LOW ? 'W' : 'K';
  }
  if (volumeDown == 'K') {
    volumeDown = digitalRead(VOLUME_DOWN) == LOW ? 'W' : 'K';
  }
  if (menuDown == 'K') {
    menuDown = digitalRead(MENU_DOWN) == LOW ? 'W' : 'K';
  }
  if (menuUp == 'K') {
    menuUp = digitalRead(MENU_UP) == LOW ? 'W' : 'K';
  }
  if (volumeUp == 'K') {
    volumeUp = digitalRead(VOLUME_UP) == LOW ? 'W' : 'K';
  }
  pusty = 'K';
}
