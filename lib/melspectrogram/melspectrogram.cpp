#include <arduinoFFT.h>
#include <PDM.h>
//#include <Arduino.h>
//#include <WiFiNINA.h>
#include "melspectrogram.h"


float vReal[FFT_SIZE];
float vImag[FFT_SIZE];
ArduinoFFT<float> FFT(vReal, vImag, FFT_SIZE, SAMPLE_RATE);

void computeFFT(float audio[SAMPLE_RATE], float spectrogram[SPECTROGRAM_ROWS][SPECTROGRAM_COLS]) {
  int frameCount = 0;
  for (int start = 0; start + FRAME_SIZE <= SAMPLE_RATE; start += HOP_SIZE) {
    // Copy the frame data
    for (int i = 0; i < FRAME_SIZE; i++) {
      vReal[i] = audio[start + i];
      vImag[i] = 0;
    }

    // Apply FFT
    FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.compute(FFT_FORWARD);
    FFT.complexToMagnitude(vReal, vImag, FFT_SIZE);

    // Store the magnitude of the FFT result in the spectrogram
    for (int i = 0; i < SPECTROGRAM_ROWS; i++) {
      spectrogram[i][frameCount] = vReal[i];
    }

    frameCount++;
    if (frameCount >= SPECTROGRAM_COLS) {
      break;
    }
  }
}