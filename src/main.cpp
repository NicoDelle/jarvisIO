#include <Arduino.h>
#include <PDM.h>
#include <WiFiNINA.h>

#include "jarvis.h"
#include "jarvis030.h"

#include "melspectrogram.h"

static const char channels = 1;
short sampleBuffer[512];
volatile int samplesRead = 0;
short audio[SAMPLE_RATE];

void onPDMdata()
{
  int bytesAvailable = PDM.available();
  PDM.read(sampleBuffer, bytesAvailable);
  samplesRead = bytesAvailable / 2;
}

constexpr int kTensorArenaSize = 50000;
alignas(16) uint8_t tensorArena[kTensorArenaSize];

static const float melFilterBank[N_MELS][N_FFT / 2 + 1] = {0};

void printSpectrogram(float melspectrogramDst[N_MELS][NUM_FRAMES])
{
  Serial.println("melspectrogramDst = np.array([");
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

void setup()
{
  // Initialize serial communications at 9600 baud rate
  Serial.begin(9600);
  while (!Serial)
    ; // Wait for the serial port to connect

  // Model setup
  if (!modelInit(jarvis0_3_0_tflite, tensorArena, kTensorArenaSize))
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
  Serial.println("Registering now!");

  int count = 0;
  while (count < SAMPLE_RATE)
  {
    if (samplesRead > 0)
    {
      for (int i = 0; i < samplesRead; i++)
      {
        if (count < SAMPLE_RATE)
        {
          audio[count] = sampleBuffer[i];
          count++;
        }
      }
      samplesRead = 0;
    }
  }

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

  float melspectrogramDst[N_MELS][NUM_FRAMES] = {0};
  
  createMelFilterbank(const_cast<float(*)[N_FFT/2 + 1]>(melFilterBank));
  melspectrogram(audio, melspectrogramDst, melFilterBank);

  modelSetInput(melspectrogramDst);
  modelRunInference();
  modelGetOutput();
}

void loop()
{
  return;
}