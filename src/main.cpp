#include <Arduino.h>
#include <WiFiNINA.h>
#include <PDM.h>
#include <ArduTFLite.h>
#include "jarvis026.h"

#define SR 8000
#define BUFFER_LENGTH 2048

int buttonPin = 2;   // Digital pin 2
int buttonState = 0; // Variable to store the button state
int recording = 0;

short sampleBuffer[BUFFER_LENGTH];
volatile int samplesRead = 0;
static const char channels = 1;
int totalSamplesRead = 0;

short audio[SR];

void onPDMdata()
{
  int bytesAvailable = PDM.available();
  PDM.read(sampleBuffer, bytesAvailable);
  samplesRead = bytesAvailable / 2;
  totalSamplesRead += samplesRead;
}

int PDMsetup()
{
  Serial.println("Setting up PDM...");
  PDM.onReceive(onPDMdata);
  if (!PDM.begin(channels, SR))
  {
    return 0;
  }

  Serial.println("Recording started!");
  return 1;
}

constexpr int kTensorArenaSize = 70000;
alignas(16) uint8_t tensor_arena[kTensorArenaSize];

void runInference() {
    modelRunInference();
    int pred = modelGetOutput(0);

    Serial.print("Prediction: ");
    Serial.println(pred);
}

void setup()
{
  // Initialize serial communications at 9600 baud rate
  Serial.begin(9600);
  while (!Serial)
    ; // Wait for the serial port to connect

  // Set the button pin as an input with an internal pull-up resistor
  pinMode(buttonPin, INPUT_PULLUP);
  Serial.println("Initializing TensorFlow Lite Micro Interpreter...");
    if (!modelInit(utils_models_jarvis0_2_6_tflite, tensor_arena, kTensorArenaSize)) {
        Serial.println("Model initialization failed!");
        while (true);
    }
    Serial.println("Model initialized!");
}

void loop()
{
  // Read the state of the button
  buttonState = digitalRead(buttonPin);

  // Print the button state to the Serial Monitor
  if (buttonState == LOW)
  {
    PDMsetup();
    recording = 1;
  }

  int limit;
  int index = 0; // Add an index variable
  short tempVal;
  while (recording)
  {
    if (!samplesRead) continue;

    limit = samplesRead;
    samplesRead = 0;
    for (int i = 0; i < limit; i++)
    { 
      tempVal = sampleBuffer[i];
      audio[index] = tempVal; // Use index to access the array
      modelSetInput(tempVal, index);
      index++;
    }

    if (totalSamplesRead >= SR)
    {
      PDM.end();
      recording = 0;
      for (int i : audio)
      {
        Serial.print(i);
        Serial.print(", ");
      }

      runInference();
    }
  }
}