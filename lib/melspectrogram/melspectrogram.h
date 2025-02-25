#include <arduinoFFT.h>
#include <math.h>

#define SAMPLE_RATE 8000
#define N_MELS 32
#define HOP_LENGTH 400
#define N_FFT 256
#define NUM_FRAMES (SAMPLE_RATE / HOP_LENGTH + 1)

short* frame(short *audio, float *buffer, short *end);
void extractPowerSpectrum(short *audio, float powerSpectrum[SAMPLE_RATE / HOP_LENGTH + 1][N_FFT/2 + 1]);
void createMelFilterbank(float filterbank[N_MELS][N_FFT/2 + 1]);
void extractMelEnergies(short *audio, float melEnergies[N_MELS][NUM_FRAMES], const float melFilterbank[N_MELS][N_FFT/2 +1]);
void powerTodB(float melspectrogram[N_MELS][NUM_FRAMES]);
void melspectrogram(short *audio, float melspectrogram[N_MELS][NUM_FRAMES], const float melFilterbank[N_MELS][N_FFT/2 +1]);