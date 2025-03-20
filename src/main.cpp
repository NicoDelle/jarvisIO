#include <Arduino.h>
#include <PDM.h>
#include <WiFiNINA.h>

#include "jarvis.h"
#include "jarvis100.h"
#include "melFilterbank.h"
#include "melspectrogram.h"

#define BUFFER_SIZE 8000

constexpr int kTensorArenaSize = 32000;
alignas(16) uint8_t tensorArena[kTensorArenaSize];

static const char channels = 1;
short sampleBuffer[BUFFER_SIZE];
short audio[SAMPLE_RATE];
volatile int writeIdx = 0;
volatile int availableSamples = 0;
volatile bool bufferReady = false;

void onPDMdata() {
    int bytesAvailable = PDM.available();
    int samplesToRead = bytesAvailable / 2;
    
    // Calculate space left until buffer wrap
    int spaceToEnd = BUFFER_SIZE - writeIdx;
    
    if (samplesToRead <= spaceToEnd) {
        PDM.read(&sampleBuffer[writeIdx], bytesAvailable);
        writeIdx += samplesToRead;
    } else {
        PDM.read(&sampleBuffer[writeIdx], spaceToEnd * 2);
        PDM.read(&sampleBuffer[0], (samplesToRead - spaceToEnd) * 2);
        writeIdx = samplesToRead - spaceToEnd;
    }
    
    availableSamples += samplesToRead;
    if (availableSamples >= SAMPLE_RATE) {
        bufferReady = true;
        availableSamples = SAMPLE_RATE;  // Prevent overflow
    }
}

bool getAudio() {
    if (!bufferReady) {
        return false;
    }
    
    // Copy with overlap
    memcpy(audio, &audio[availableSamples], 
           (SAMPLE_RATE - availableSamples) * sizeof(short));
    
    // Get newest samples
    int readIdx = (writeIdx - availableSamples + BUFFER_SIZE) % BUFFER_SIZE;
    for (int i = 0; i < availableSamples; i++) {
        audio[SAMPLE_RATE - availableSamples + i] = 
            sampleBuffer[(readIdx + i) % BUFFER_SIZE];
    }
    
    bufferReady = false;
    availableSamples = 0;
    return true;
}


void printSpectrogram(float melspectrogramDst[N_MELS][NUM_FRAMES])
{
    Serial.println("mel_energies = np.array([");
    for (int i = 0; i < N_MELS; i++)
    {
        Serial.print("[");
        for (int j = 0; j < NUM_FRAMES; j++)
        {
            Serial.print(melspectrogramDst[i][j]);
            if (j < NUM_FRAMES - 1)
            {
                Serial.print(", ");
            }
        }
        Serial.print("]");
        if (i < N_MELS - 1)
        {
            Serial.println(",");
        }
    }
    Serial.println("])");
}

void printAudio()
{
    Serial.println("Audio registered successfully");
    Serial.println("audio = np.array([");
    for (int i = 0; i < SAMPLE_RATE; i++)
    {
        Serial.print(audio[i]);
        if (i < SAMPLE_RATE - 1)
        {
            Serial.print(", ");
        }
        if ((i + 1) % 20 == 0)
        {
            Serial.println();
        }
    }
    Serial.println("])");
}

void setup()
{
    // Initialize serial communications at 9600 baud rate
    Serial.begin(9600);
    while (!Serial)
        ; // Wait for the serial port to connect

    // Model setup
    if (!modelInit(jarvis1_0_0_tflite, tensorArena, kTensorArenaSize))
    {
        Serial.println("Failed to initialize the model!");
        while (1)
        {
        }
    }
    Serial.println("Model initialized!");

    // Countdown
    PDM.onReceive(onPDMdata);
    for (int i = 3; i > 0; i--)
    {
        Serial.println(i);
        delay(1000); // Wait for 1 second
    }
    if (!PDM.begin(channels, SAMPLE_RATE))
    {
        Serial.println("Failed to start PDM!");
        while (1)
        ;
    }
}

void loop()
{
    if (!getAudio()) return;
    
    float melspectrogramDst[N_MELS][NUM_FRAMES] = {0};

    melspectrogram(audio, melspectrogramDst, melFilterBank);

    modelSetInput(melspectrogramDst);
    modelRunInference();
    float output = modelGetOutput();
    Serial.println(output);

}