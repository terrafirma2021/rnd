#include "Arduino.h"
#include "esp_system.h"
#include "rom/spi_flash.h"

#define DAC_PIN 25  // DAC1 on GPIO 25
#define RAW_AUDIO_OFFSET 0x200000
#define MARKER_SIZE 4              // Marker is 4 bytes (0xDEADBEEF)

void playAudioFromFlash();

uint8_t audioBuffer[256];

void setup() {
  Serial.begin(115200);
  Serial.println("Starting audio playback...");

  playAudioFromFlash();
}

void loop() {

}

bool isEndMarker(uint8_t* buffer, int length) {
  const uint8_t marker[MARKER_SIZE] = {0xDE, 0xAD, 0xBE, 0xEF};

  if (length >= MARKER_SIZE) {
    for (int i = 0; i < MARKER_SIZE; i++) {
      if (buffer[length - MARKER_SIZE + i] != marker[i]) {
        return false;
      }
    }
    return true;
  }
  return false;
}

void playAudioFromFlash() {
  int currentOffset = RAW_AUDIO_OFFSET;
  bool markerFound = false;

  while (!markerFound) {

    spi_flash_read(currentOffset, audioBuffer, sizeof(audioBuffer));

    markerFound = isEndMarker(audioBuffer, sizeof(audioBuffer));

    if (markerFound) {
      Serial.println("End of audio detected (marker found).");
      break;
    }

    for (int i = 0; i < sizeof(audioBuffer); i++) {
      dacWrite(DAC_PIN, audioBuffer[i]);
      delayMicroseconds(62);
    }

    currentOffset += sizeof(audioBuffer);
  }

  Serial.println("Audio playback finished.");
}
