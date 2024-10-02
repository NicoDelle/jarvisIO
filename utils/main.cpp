#include <WiFiNINA.h>
#include <PDM.h>
#include <arduinoFFT.h>
#include <TensorFlowLite.h>
#include "model_data.h"

#define SAMPLE_RATE 16000
#define BUFFER_SIZE 512
#define FFT_SIZE 256
#define MEL_BINS 64
#define MEL_MIN_FREQ 300
#define MEL_MAX_FREQ 8000
#define SPECTROGRAM_ROWS 64
#define SPECTROGRAM_COLS 16

short sampleBuffer[BUFFER_SIZE];
volatile int samplesRead = 0;
float vReal[FFT_SIZE];
float vImag[FFT_SIZE];
ArduinoFFT<float> FFT;
double spectrogram[SPECTROGRAM_ROWS][SPECTROGRAM_COLS];

// Globals, used for compatibility with Arduino-style sketches.
namespace {
  tflite::MicroErrorReporter micro_error_reporter;
  tflite::ErrorReporter* error_reporter = &micro_error_reporter;

  const tflite::Model* model = nullptr;
  tflite::MicroInterpreter* interpreter = nullptr;

  // Create an area of memory to use for input, output, and intermediate arrays.
  constexpr int kTensorArenaSize = 2 * 1024;
  uint8_t tensor_arena[kTensorArenaSize];
}  // namespace

void onPDMdata() {
  int bytesAvailable = PDM.available();
  PDM.read(sampleBuffer, bytesAvailable);
  samplesRead = bytesAvailable / 2; // since each sample is 2 bytes
}

void setup() {
  Serial.begin(9600);
  PDM.onReceive(onPDMdata);
  PDM.begin(1, SAMPLE_RATE);

  // Load the model.
  model = tflite::GetModel(model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model provided is schema version");
    return;
  }

  // Create an interpreter to run the model.
  static tflite::MicroMutableOpResolver<10> micro_op_resolver;
  tflite::MicroInterpreter static_interpreter(
      model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    Serial.println("AllocateTensors() failed");
    return;
  }
}

void loop() {
  if (samplesRead > 0) {
    generateSpectrogram();
    runInference();
    samplesRead = 0;
  }
}

void computeFFT() {
  for (int i = 0; i < FFT_SIZE; i++) {
    vReal[i] = sampleBuffer[i];
    vImag[i] = 0;
  }
  FFT.Windowing(vReal, FFT_SIZE, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, FFT_SIZE, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, FFT_SIZE);
}

void computeMelSpectrogram(double* fftOutput, double* melOutput) {
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

void preprocessInput(float* input, double spectrogram[SPECTROGRAM_ROWS][SPECTROGRAM_COLS]) {
  for (int i = 0; i < SPECTROGRAM_ROWS; i++) {
    for (int j = 0; j < SPECTROGRAM_COLS; j++) {
      input[i * SPECTROGRAM_COLS + j] = static_cast<float>(spectrogram[i][j]);
    }
  }
}

void runInference() {
  TfLiteTensor* input_tensor = interpreter->input(0);
  TfLiteTensor* output_tensor = interpreter->output(0);

  preprocessInput(input_tensor->data.f, spectrogram);

  TfLiteStatus invoke_status = interpreter->Invoke();
  if (invoke_status != kTfLiteOk) {
    Serial.println("Invoke failed");
    return;
  }

  float* output = output_tensor->data.f;
  for (int i = 0; i < output_tensor->dims->data[0]; i++) {
    Serial.print("Output[");
    Serial.print(i);
    Serial.print("]: ");
    Serial.println(output[i]);
  }
}