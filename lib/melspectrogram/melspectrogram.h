#include <arduinoFFT.h>
#include <math.h>

#define SAMPLE_RATE 8000
#define N_MELS 32
#define HOP_LENGTH 400
#define N_FFT 256

float* frame(float *audio, float *buffer, float *end);
void extractPowerSpectrum(float *audio, float powerSpectrum[SAMPLE_RATE / HOP_LENGTH + 1][N_FFT/2 + 1]);
void extractMelEnergies(float *audio, float melEnergies[SAMPLE_RATE / HOP_LENGTH + 1][N_MELS]);
void powerTodB(float melspectrogram[SAMPLE_RATE / HOP_LENGTH + 1][N_MELS]);
void melspectrogram(float *audio, float melspectrogram[SAMPLE_RATE / HOP_LENGTH + 1][N_MELS]);