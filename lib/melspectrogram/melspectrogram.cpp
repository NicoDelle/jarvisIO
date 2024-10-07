#include <arduinoFFT.h>
#include <PDM.h>
#include "melspectrogram.h"

float vReal[FFT_SIZE];
float vImag[FFT_SIZE];
ArduinoFFT<float> FFT(vReal, vImag, FFT_SIZE, SAMPLE_RATE);


float _hzToMel(float hz) {
  return 2595 * log10(1 + hz / 700.0);
}

float _melToHz(float mel) {
  return 700 * (pow(10, mel / 2595) - 1);
}

void computeFilterBanks(float melFilterBanks[SPECTROGRAM_ROWS][FFT_SIZE / 2 + 1]) {
  float melMin = _hzToMel(0);
  float melMax = _hzToMel(SAMPLE_RATE / 2);
  float melStep = (melMax - melMin) / (SPECTROGRAM_ROWS + 1);

  float melCenters[SPECTROGRAM_ROWS + 2];
  for (int i = 0; i < SPECTROGRAM_ROWS + 2; i++) {
    melCenters[i] = melMin + i * melStep;
  }

  float hzCenters[SPECTROGRAM_ROWS + 2];
  for (int i = 0; i < SPECTROGRAM_ROWS + 2; i++) {
    hzCenters[i] = _melToHz(melCenters[i]);
  }

  int binCenters[SPECTROGRAM_ROWS + 2];
  for (int i = 0; i < SPECTROGRAM_ROWS + 2; i++) {
    binCenters[i] = (int)(hzCenters[i] * FFT_SIZE / SAMPLE_RATE);
  }

  for (int i = 1; i <= SPECTROGRAM_ROWS; i++) {
    for (int j = binCenters[i - 1]; j < binCenters[i]; j++) {
      melFilterBanks[i - 1][j] = (float)(j - binCenters[i - 1]) / (binCenters[i] - binCenters[i - 1]);
    }
    for (int j = binCenters[i]; j < binCenters[i + 1]; j++) {
      melFilterBanks[i - 1][j] = (float)(binCenters[i + 1] - j) / (binCenters[i + 1] - binCenters[i]);
    }
  }
}

void computeFFT(float audio[SAMPLE_RATE], float spectrogram[SPECTROGRAM_ROWS][SPECTROGRAM_COLS], float melFilterBanks[SPECTROGRAM_ROWS][FFT_SIZE / 2 + 1]) {
  int frameCount = 0;
  float ref = 0;
  for (int start = 0; start + FRAME_SIZE <= SAMPLE_RATE; start += HOP_SIZE) {
    // Copy the frame data
    for (int i = 0; i < FRAME_SIZE; i++) {
      vReal[i] = audio[start + i];
      vImag[i] = 0;
    }

    // Apply FFT
    FFT.windowing(FFT_WIN_TYP_HANN, FFT_FORWARD);
    FFT.compute(FFT_FORWARD);
    FFT.complexToMagnitude(vReal, vImag, FFT_SIZE);

    // Apply mel filter banks and store the result in the spectrogram
    for (int i = 0; i < SPECTROGRAM_ROWS; i++) {
      float melValue = 0;
      for (int j = 0; j < FFT_SIZE / 2 + 1; j++) {
        melValue += vReal[j] * melFilterBanks[i][j];
      }
      spectrogram[i][frameCount] = melValue; // Logarithmic compression
      if (melValue > ref) ref = melValue;
    }

    frameCount++;
    if (frameCount >= SPECTROGRAM_COLS) {
      break;
    }
  }

  Serial.println(ref);
  // Normalize the sooectrogram
  for (int i = 0; i < SPECTROGRAM_ROWS; i++) {
    for (int j = 0; j < SPECTROGRAM_COLS; j++) {
      spectrogram[i][j] = 10*log10(spectrogram[i][j] / ref + 0.01);
    }
  }
}