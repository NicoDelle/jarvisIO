#include <WiFiNINA.h>
#include <PDM.h>
#include "audioProvider.h"


static const char channels = 1;
short sampleBuffer[BUFFER_LENGTH];
volatile int samplesRead = 0;
int totalSamplesRead = 0;

void onPDMdata()
{
    int bytesAvailable = PDM.available();
    PDM.read(sampleBuffer, bytesAvailable);
    samplesRead = bytesAvailable / 2;
    totalSamplesRead += samplesRead;
}

int PDMsetup()
{
    PDM.onReceive(onPDMdata);
    if (!PDM.begin(channels, SR))
    {
        return 0;
    }

    return 1;
}

AudioProvider::AudioProvider()
{
    int totalRead = 0;
    int nextTotalRead;
    while (totalRead < SR)
    {
        Serial.println("AudioProvider constructor called");
        if (!samplesRead) continue;

        nextTotalRead = totalRead + samplesRead;
        if (nextTotalRead > SR) {nextTotalRead = SR;}
        AudioProvider::_getNewData(totalRead, nextTotalRead); //una batch di dati (32 campioni) arriva dalla DAQ ogni --- ms

        samplesRead = nextTotalRead - totalRead;
        totalRead = nextTotalRead;
    }
}

int AudioProvider::update()
{
    AudioProvider::totalRead = totalSamplesRead;
    int offset = SR - AudioProvider::totalRead;
    AudioProvider::_shift(offset);
    AudioProvider::_getNewData(offset, SR);
    totalSamplesRead = AudioProvider::totalRead = 0;

    int rv = totalRead;
    totalRead = 0;
    return rv;
}

void AudioProvider::_shift(int offset)
{
    for (int i = 0; i < offset; i++)
    {
        AudioProvider::audio[i] = AudioProvider::audio[i + AudioProvider::totalRead];
    }
}

void AudioProvider::_getNewData(int offset, int upperLimit)
{
    for (int i = offset; i < upperLimit; i++)
    {
        AudioProvider::audio[i] = sampleBuffer[i - offset] / 32768.0f;
    }
}

void AudioProvider::printAudio()
{
    for (int i=0; i<400; i++)
  {
    for (int j=0; j<20; j++)
    {
      Serial.print(audio[(i+1)*j], 6); //all data is just empty...
      Serial.print(", ");
    }
    Serial.println();
  }
}