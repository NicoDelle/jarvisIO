#include <Arduino.h>
#include <WiFiNINA.h>
#include <PDM.h>

#include <ArduTFLite.h>
#include "jarvis025.h" // here the model (jarvis 0.1.tflite) is compiled into a matrix, to be used by the interpreter to run inference. To see how the model was obtained, check the notebooks! 
#include "hcData.h"

#define DATA RDj097


void timeAndPredict()
{
  int t0 = millis();
  if (!modelRunInference())
  {
    Serial.println("RunInference Failed!");
    return;
  }

  // Run inference on the given data
  float y = modelGetOutput(0);
  int t1 = millis();
  Serial.print("Output: ");
  Serial.println(y);
  Serial.print("Time taken: ");
  Serial.print(t1 - t0);
  Serial.println(" ms");
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
  if (!modelInit(utils_models_jarvis0_2_5_tflite, tensor_arena, kTensorArenaSize))
  {
    Serial.println("Model initialization failed!");
    while (true)
      ;
  }

  //Place all input data inside the input tensor of the model, using the helper function
  for (int i = 0; i < 8000; i++)
  {
      modelSetInput(DATA[i], i); //the input matrix is flattened to a 1D, 64*16 array
  }

  Serial.println("Model initialization done.");
}

void loop()
{
  timeAndPredict();
  while (1 == 1)
  {
  }
}