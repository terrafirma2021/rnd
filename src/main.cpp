#include "Arduino.h"

// Define DAC pin
#define DAC_PIN 25  // DAC1 on GPIO 25

// Parameters for the siren effect
const int lowFreq = 500;       // 500 Hz low tone
const int highFreq = 1200;     // 1200 Hz high tone
const float sweepTime = 0.5;   // 0.5 seconds to sweep up or down
const int sampleRate = 5000;   // Sample rate in Hz (5 kHz)
const int sweepSteps = sampleRate * sweepTime; // Number of steps in the sweep

// Task handle
TaskHandle_t sirenTaskHandle;

void sirenTask(void *parameter);
void generateTone(int frequency);

void setup() {
  // Create the siren task (one line as you prefer)
  xTaskCreate(sirenTask, "SirenTask", 1000, NULL, 1, NULL);
}

void loop() {
  // Nothing here, as the task handles everything
}

// Function to generate the siren effect
void sirenTask(void *parameter) {
  while (1) {
    // Rising sweep from 500 Hz to 1200 Hz over 0.5 seconds
    for (int i = 0; i <= sweepSteps; i++) {
      float t = (float)i / sweepSteps; // Normalized time [0, 1] for the sweep
      int freq = lowFreq + t * (highFreq - lowFreq); // Linearly interpolate frequency
      generateTone(freq);
      delayMicroseconds(1000000 / sampleRate);  // Control the frequency sweep rate
    }

    // Falling sweep from 1200 Hz to 500 Hz over 0.5 seconds
    for (int i = 0; i <= sweepSteps; i++) {
      float t = (float)i / sweepSteps; // Normalized time [0, 1] for the sweep
      int freq = highFreq - t * (highFreq - lowFreq); // Linearly interpolate frequency
      generateTone(freq);
      delayMicroseconds(1000000 / sampleRate);  // Control the frequency sweep rate
    }
  }
}

// Function to generate a single tone using the DAC
void generateTone(int frequency) {
  // Use the ESP32 hardware timer for more precise time
  int amplitude = 255;  // Maximum DAC output (8-bit resolution)
  int waveValue = (sin(2 * PI * esp_timer_get_time() * frequency / 1000000.0) + 1) * (amplitude / 2); 
  dacWrite(DAC_PIN, waveValue);
}
