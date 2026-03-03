/*
  UŻYCIE:
  if (!sd_card_start()) {
   Serial.println("SD BRAK – lecimy dalej bez niej!");
  }
  sd.println(String("Loop: ") + millis());
*/


#ifndef SD_CARD_H
#define SD_CARD_H

#include <Arduino.h>
#include <SD.h>

class SDLogger {
  public:
    SDLogger(uint8_t csPin, uint32_t maxSizeBytes = 2UL * 1024UL * 1024UL)
      : cs(csPin), maxSize(maxSizeBytes) {}

    bool start() {
      pinMode(cs, OUTPUT);
      digitalWrite(cs, HIGH);

      if (!SD.begin(cs)) {
        file = File(); // pusty plik = SD niedostępne
        return false;
      }

      currentIndex = findLastLogIndex();
      if (currentIndex == 0) currentIndex = 1;

      openFile(currentIndex);
      return (bool)file;
    }

    bool available() {
      return (bool)file;
    }
    String currentFilename() {
      if (!file) return "";
      return makeFilename(currentIndex);
    }
    // PRINT
    void print(const String &s) {
      if (!file) return;
      rotateIfNeeded();
      file.print(s);
      file.flush();
    }

    void print(const __FlashStringHelper *s) {
      if (!file) return;
      rotateIfNeeded();
      file.print(s);
      file.flush();
    }

    // PRINTLN
    void println(const String &s) {
      if (!file) return;
      rotateIfNeeded();
      file.println(s);
      file.flush();
    }

    void println(const __FlashStringHelper *s) {
      if (!file) return;
      rotateIfNeeded();
      file.println(s);
      file.flush();
    }

    void println() {
      if (!file) return;
      rotateIfNeeded();
      file.println();
      file.flush();
    }

    // liczby
    void println(int v)      {
      if (file) {
        rotateIfNeeded();
        file.println(v);
        file.flush();
      }
    }
    void println(long v)     {
      if (file) {
        rotateIfNeeded();
        file.println(v);
        file.flush();
      }
    }
    void println(float v)    {
      if (file) {
        rotateIfNeeded();
        file.println(v);
        file.flush();
      }
    }
    void println(unsigned v) {
      if (file) {
        rotateIfNeeded();
        file.println(v);
        file.flush();
      }
    }

  private:
    File file;
    uint8_t cs;
    uint32_t maxSize;
    int currentIndex = 1;

    String makeFilename(int index) {
      char buf[20];
      sprintf(buf, "log%04d.txt", index);
      return String(buf);
    }

    int findLastLogIndex() {
      for (int i = 9999; i >= 1; i--) {
        if (SD.exists(makeFilename(i).c_str())) return i;
      }
      return 0;
    }

    void openFile(int index) {
      file = SD.open(makeFilename(index).c_str(), FILE_WRITE);
    }

    void rotateIfNeeded() {
      if (!file) return;

      if (file.size() >= maxSize) {
        file.close();
        currentIndex++;
        openFile(currentIndex);
      }
    }
};

// GLOBALNY OBIEKT
SDLogger sd(4);

// FUNKCJA STARTUJĄCA
bool sd_card_start() {
  return sd.start();
}
void sd_card_debug_menu() {
  if (currentDebugModule == "sd" || currentDebugModule == "SD") {
  Serial.println(F("==============SD_CARD=============="));
    if (!sd_card_start()) {
      Serial.println("NO SD CARD");
    } else {
      Serial.print("SD CARD OK ");
      Serial.println(sd.currentFilename());
    }
  Serial.println(F("--> MAIN - to main menu"));
  Serial.println(F("--> EXIT - to normal mode"));
  currentDebugModule = "dupa";
  }
}
#endif
