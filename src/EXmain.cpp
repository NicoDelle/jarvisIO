#include <Arduino.h>
#include <WiFiNINA.h>
#include <PDM.h>
#include <ArduTFLite.h>
#include "jarvis026.h"

#define BUFFER_LENGTH 2048
#define SR 8000

short sampleBuffer[BUFFER_LENGTH];
volatile int samplesRead = 0;
static const char channels = 1;
int totalSamplesRead = 0;

int buttonPin = 2;
bool isRecording = false;

constexpr int kTensorArenaSize = 70000;
alignas(16) uint8_t tensor_arena[kTensorArenaSize];

void onPDMdata() {
    int bytesAvailable = PDM.available();
    PDM.read(sampleBuffer, bytesAvailable);
    samplesRead = bytesAvailable / 2;
}

int PDMsetup() {
    Serial.println("Setting up PDM...");
    PDM.onReceive(onPDMdata);
    if (!PDM.begin(channels, SR)) {
        return 0;
    }

    Serial.println("Recording started!");
    return 1;
}

void PDMend() {
    PDM.end();
    Serial.println("Recording stopped!");
}

void runInference() {
    modelRunInference();
    int pred = modelGetOutput(0);

    Serial.print("Prediction: ");
    Serial.println(pred);
}

// All code to be executed once goes here
void setup() {
    // Initialize serial communications and wait for Serial Monitor to be opened
    Serial.begin(9600);
    while (!Serial);

    // Set the button pin mode
    pinMode(buttonPin, INPUT);

    Serial.println("Initializing TensorFlow Lite Micro Interpreter...");
    if (!modelInit(utils_models_jarvis0_2_6_tflite, tensor_arena, kTensorArenaSize)) {
        Serial.println("Model initialization failed!");
        while (true);
    }
    Serial.println("Model initialized!");
}

void loop() {
    int buttonState = digitalRead(buttonPin);
    delay(100);
    Serial.println(buttonState);
    if (buttonState == HIGH && !isRecording) {
        Serial.println("Button pressed!");
        isRecording = true;
        totalSamplesRead = 0;
        PDMsetup();
    }

    if (isRecording) {
        Serial.println("Recording...");
        int nextTotalSamplesRead;
        float normalizedValue;
        while (totalSamplesRead < SR) {
            if (!samplesRead) 
            {
              Serial.println("No samples read");
              continue;
            }
            nextTotalSamplesRead = totalSamplesRead + samplesRead;
            if (nextTotalSamplesRead > SR) nextTotalSamplesRead = SR;
            for (int i = totalSamplesRead; i < nextTotalSamplesRead; i++) {
                normalizedValue = (float) sampleBuffer[i] / 32768.0f;
                Serial.print(normalizedValue, 6);
                Serial.print(", ");
                modelSetInput(normalizedValue, i); // The input matrix is flattened to a 1D, 64*16 array
            }
            samplesRead = 0;
            totalSamplesRead = nextTotalSamplesRead;
        }

        if (totalSamplesRead == SR) {
            runInference();
            PDMend();
            isRecording = false;
        }
    }
}