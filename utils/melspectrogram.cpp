#include <PDM.h>
#include <arduinoFFT.h>
#include <Arduino.h>
#include <WiFiNINA.h>
#include "hcData.h"

#define SAMPLE_RATE 16000
#define BUFFER_SIZE 512
#define FFT_SIZE 256
#define MEL_BINS 40
#define MEL_MIN_FREQ 300
#define MEL_MAX_FREQ 8000
#define SPECTROGRAM_ROWS 40
#define SPECTROGRAM_COLS 43

short sampleBuffer[BUFFER_SIZE];
volatile int samplesRead = 0;
float vReal[FFT_SIZE];
float vImag[FFT_SIZE];
ArduinoFFT<float> FFT;
double spectrogram[SPECTROGRAM_ROWS][SPECTROGRAM_COLS];

void onPDMdata() {
  int bytesAvailable = PDM.available();
  PDM.read(sampleBuffer, bytesAvailable);
  samplesRead = bytesAvailable / 2; // since each sample is 2 bytes
}

void setup() {
  Serial.begin(9600);
  PDM.onReceive(onPDMdata);
  PDM.begin(1, SAMPLE_RATE);
}

void loop() {
  if (samplesRead > 0) {
    generateSpectrogram();
    samplesRead = 0;
  }
}

void computeFFT() {
  for (int i = 0; i < FFT_SIZE; i++) {
    vReal[i] = sampleBuffer[i];
    vImag[i] = 0;
  }
  FFT.windowing(vReal, FFT_SIZE, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(vReal, vImag, FFT_SIZE, FFT_FORWARD);
  FFT.complexToMagnitude(vReal, vImag, FFT_SIZE);
}

void computeMelSpectrogram(float* fftOutput, double* melOutput) {
  double melFilterBanks[MEL_BINS][FFT_SIZE / 2 + 1] = {0};

  for (int i = 0; i < MEL_BINS; i++) {
    double melFreq = MEL_MIN_FREQ + (MEL_MAX_FREQ - MEL_MIN_FREQ) * i / (MEL_BINS - 1);
    int bin = (int)(melFreq / (SAMPLE_RATE / 2) * (FFT_SIZE / 2));
    for (int j = bin - 1; j <= bin + 1; j++) {
      if (j >= 0 && j < FFT_SIZE / 2 + 1) {
        melFilterBanks[i][j] = 1.0 - abs(j - bin) / 1.0;
      }
    }
  }

  for (int i = 0; i < MEL_BINS; i++) {
    melOutput[i] = 0;
    for (int j = 0; j < FFT_SIZE / 2 + 1; j++) {
      melOutput[i] += fftOutput[j] * melFilterBanks[i][j];
    }
  }
}

void generateSpectrogram() {
  double melOutput[MEL_BINS];
  for (int i = 0; i < SPECTROGRAM_ROWS; i++) {
    computeFFT();
    computeMelSpectrogram(vReal, melOutput);
    for (int j = 0; j < SPECTROGRAM_COLS; j++) {
      spectrogram[i][j] = melOutput[j];
    }
  }
}