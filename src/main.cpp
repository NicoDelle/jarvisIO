#include <Arduino.h>
#include <PDM.h>
#include "featureProvider.h"
#include <time.h>

// PDM variables
static const char channels = 1;
short sampleBuffer[512];
volatile int samplesRead;

void onPDMdata()
{
  int bytesAvailable = PDM.available();
  PDM.read(sampleBuffer, bytesAvailable);
  samplesRead = bytesAvailable / sizeof(short);
}

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;

  // PDM setup
  PDM.onReceive(onPDMdata);
  PDM.begin(channels, SAMPLE_RATE);
}

void loop()
{
  float feature[40][43];
  int timeTaken;
  timeTaken = getFeature(feature, sampleBuffer, &samplesRead); //takes 1.6s to run: need to cut off AT LEAST 600ms 
  Serial.println(timeTaken);
}