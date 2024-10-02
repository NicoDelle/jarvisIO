#include <ArduTFLite.h>
#include "model.h"
#include "hcData.h"

#define SAMPLE_RATE 8000
#define SPECTROGRAM_ROWS 64
#define SPECTROGRAM_COLS 16

#define FEATURE FTuk037

constexpr int kTensorArenaSize = 16 * 1024;
alignas(16) uint8_t tensor_arena[kTensorArenaSize];

void setup()
{
  // Initialize serial communications and wait for Serial Monitor to be opened
  Serial.begin(9600);
  while (!Serial)
    ;

  Serial.println("Initializing TensorFlow Lite Micro Interpreter...");
  if (!modelInit(jarvis0_1_tflite, tensor_arena, kTensorArenaSize))
  {
    Serial.println("Model initialization failed!");
    while (true)
      ;
  } 

  int counter = 0;
  for (int i = 0; i < 64; i++)
  {
    for (int j = 0; j < 16; j++)
    {
      modelSetInput(FEATURE[i][j], counter++);
    }
  }

  Serial.println("Model initialization done.");
}

void loop()
{
  if (!modelRunInference())
  {
    Serial.println("RunInference Failed!");
    return;
  }

  // The expected prediction is 0.36
  float y = modelGetOutput(0);
  Serial.println(y);
  while (1 == 1)
  {
  }
}