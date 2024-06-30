#include <stdint.h>
#include <arduinoFFT.h>
#include <time.h>
#include "featureProvider.h"


int getFeature(float feature[40][43], short sampleBuffer[512], volatile int* samplesRead)
{
    int t0 = millis();
    static float slice[SLICE_SIZE];

    for (int i = 0; i < 40; i++)
    {
        getSlice(slice, sampleBuffer, samplesRead);
        computeFFT(slice);
        getFeatureRow(slice, feature[i]);
    }

    return millis() - t0;
}

void getFeatureRow(float *vMagnitude, float averagedBuckets[FEATURE_COLUMNS])
{
    int bucketIndex = 0;

    float sum;
    for (int i = 0; i < SLICE_SIZE; i += 6)
    {
        sum = 0;
        for (int j = i; j < i + 6 && j < 256; j++)
        {
            sum += vMagnitude[j];
        }
        averagedBuckets[bucketIndex++] = sum / 6;
    }

    int remainingBuckets = 256 % 6;
    sum = 0;
    for (int i = 0; i < remainingBuckets; ++i)
    {
      sum += averagedBuckets[FEATURE_COLUMNS - 1];
    }
    averagedBuckets[FEATURE_COLUMNS - 1] = sum / remainingBuckets;
}

void getSlice(float slice[SLICE_SIZE], short sampleBuffer[512], volatile int *samplesRead)
{
    short sliceSize = 0;
    int samplesWritten = 0;
    static float overlap[SLICE_OVERLAP];

    sliceSize = reportOverlap(slice, overlap);

    while (sliceSize != SLICE_SIZE) // read #samplesRead from buffer
    {
        if (!*samplesRead)
            continue;

        samplesWritten = fillSlice(slice, sampleBuffer, *samplesRead, sliceSize);
        sliceSize += samplesWritten;
        *samplesRead -= samplesWritten;
    }

    saveOverlap(slice, overlap);
}

short fillSlice(float slice[SLICE_SIZE], short sampleBuffer[512], int samplesRead, short lastRead)
{
    short limit = ((samplesRead + lastRead) < SLICE_SIZE) ? samplesRead + lastRead : SLICE_SIZE;

    for (int i = lastRead; i < limit; i++)
    {
        slice[i] = (float)sampleBuffer[i - lastRead];
    }

    return (limit - lastRead);
}

short reportOverlap(float slice[SLICE_SIZE], float overlap[SLICE_OVERLAP])
{
    static short isFirstTime = 1;
    if (isFirstTime)
    {
        isFirstTime = 0;
        return 0;
    }

    for (int i = 0; i < SLICE_OVERLAP; i++)
    {

        slice[i] = overlap[i];
    }

    return SLICE_OVERLAP;
}

void saveOverlap(float slice[SLICE_SIZE], float overlap[SLICE_OVERLAP])
{
    float *startAddress = slice + SLICE_SIZE - SLICE_OVERLAP;
    memcpy(overlap, startAddress, SLICE_OVERLAP * sizeof(float));
}


void computeFFT(float *vReal)
{
    float vImag[SLICE_SIZE] = {0};
    ArduinoFFT<float> FFT(vReal, vImag, SLICE_SIZE, SAMPLE_RATE);
    FFT.compute(FFT_FORWARD);

    float maxFreq = 0;
    for (int i = 0; i < SLICE_SIZE; i++)
    {
        vReal[i] = sqrt(vReal[i] * vReal[i] + vImag[i] * vImag[i]);
        if (vReal[i] > maxFreq)
        {
            maxFreq = vReal[i];
        }
    }

    float ratio = maxFreq / 255;
    for (int i = 0; i < SLICE_SIZE; i++)
    {
        vReal[i] = vReal[i] / ratio;
    }
}