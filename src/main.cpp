#include "Arduino.h"
#define DAC_PIN 25  // DAC1 on GPIO 25

const int lowFreq = 500;       // 500 Hz low tone
const int highFreq = 1200;     // 1200 Hz high tone
const float sweepTime = 0.5;   // 0.5 seconds to sweep up or down
const int sampleRate = 5000;   // Sample rate in Hz (5 kHz)
const int sweepSteps = sampleRate * sweepTime; // Number of steps in the sweep

TaskHandle_t sirenTaskHandle;
void sirenTask(void *parameter);
void generateTone(int frequency);

void setup() {
  xTaskCreate(sirenTask, "SirenTask", 1024, NULL, 1, NULL);
}

void loop() {

}

void sirenTask(void *parameter) {
  while (1) {
    for (int i = 0; i <= sweepSteps; i++) {
      float t = (float)i / sweepSteps;
      int freq = lowFreq + t * (highFreq - lowFreq);
      generateTone(freq);
      delayMicroseconds(1000000 / sampleRate);
    }

    for (int i = 0; i <= sweepSteps; i++) {
      float t = (float)i / sweepSteps;
      int freq = highFreq - t * (highFreq - lowFreq);
      generateTone(freq);
      delayMicroseconds(1000000 / sampleRate);
    }
  }
}

void generateTone(int frequency) {
  int amplitude = 255;  // Maximum DAC output (8-bit resolution)
  int waveValue = (sin(2 * PI * esp_timer_get_time() * frequency / 1000000.0) + 1) * (amplitude / 2); 
  dacWrite(DAC_PIN, waveValue);
}
