// featureProvider.h
#ifndef FEATURE_PROVIDER_H
#define FEATURE_PROVIDER_H

#define SLICE_SIZE 256
#define SLICE_OVERLAP 56//chosen so that the non-overlapping part lasts 25ms
#define SAMPLE_RATE 8000
#define FEATURE_COLUMNS 43

#include <stdint.h>
#include <arduinoFFT.h>

//Provides the spectrogram of 1s of audio, in the form of a 43x40 matrix.
//The matrix is stored in the feature array, which needs to be passed as a parameter.
int getFeature(float feature[40][43], short ssampleBuffer[512], volatile int* samplesRead);

//Helper function to process the FFT output into the weighed magnitudes
void getFeatureRow(float* vMagnitude, float averagedBuckets[FEATURE_COLUMNS]);

//Provides a formatted slice from raw mic samples.
//Requires a pointer to the memory to fill with the slice and a pointer to the number of samples read from the mic
void getSlice(float* slice, short sampleBuffer[512], volatile int* samplesRead);

//Fills a slice with a number of samples equal to samplesRead (or to the slice size), starting from the lastRead-th sample.
short fillSlice(float slice[SLICE_SIZE], short sampleBuffer[512], int samplesRead, short lastRead);

//Writes the audio data saved in the overlap array into the slice array.
//Returns the number of samples written.
short reportOverlap(float slice[SLICE_SIZE], float overlap[SLICE_OVERLAP]);

//Saves the last SLICE_OVERLAP samples of the slice array into the overlap array, for the reportOverlap function to use.
void saveOverlap(float slice[SLICE_SIZE], float overlap[SLICE_OVERLAP]);

//Computes the FFT of a slice of audio data.
//The slice needs to be passed as vReal, the resulting frequency buckets overwrite the array.
void computeFFT(float* vReal);



#endif // FEATURE_PROVIDER_H