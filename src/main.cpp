#include <Arduino.h>
#include <WiFiNINA.h>
#include <PDM.h>

#include <ArduTFLite.h>
#include "jarvis026.h" 
#include "hcData.h"

#define DATA unknown_66_2

float runInference(float data[8000])
{
  for (int i = 0; i < 8000; i++)
  {
      modelSetInput(data[i], i); //the input matrix is flattened to a 1D, 64*16 array
  }
  modelRunInference();
  return modelGetOutput(0);
}

//define an area inside memory for the model to perform undisutrbed operations in
constexpr int kTensorArenaSize = 70000;
alignas(16) uint8_t tensor_arena[kTensorArenaSize];


//all code to be executed once goes here
void setup()
{
  // Initialize serial communications and wait for Serial Monitor to be opened
  Serial.begin(9600);
  while (!Serial)
    ;

  Serial.println("Initializing TensorFlow Lite Micro Interpreter...");
  if (!modelInit(utils_models_jarvis0_2_6_tflite, tensor_arena, kTensorArenaSize))
  {
    Serial.println("Model initialization failed!");
    while (true)
      ;
  }

}

void loop()
{
  runInference(DATA);
}