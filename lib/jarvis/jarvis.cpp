#include "jarvis.h"

tflite::AllOpsResolver tflOpsResolver;
const tflite::Model *tflModel = nullptr;
tflite::MicroInterpreter *tflInterpreter = nullptr;
TfLiteTensor *tflInputTensor = nullptr;
TfLiteTensor *tflOutputTensor = nullptr;

const char *__getTensorTypeName(TfLiteType type)
{
  switch (type)
  {
  case kTfLiteFloat32:
    return "kTfLiteFloat32";
  case kTfLiteInt8:
    return "kTfLiteInt8";
  case kTfLiteUInt8:
    return "kTfLiteUInt8";
  case kTfLiteInt16:
    return "kTfLiteInt16";
  case kTfLiteInt32:
    return "kTfLiteInt32";
  case kTfLiteUInt32:
    return "kTfLiteUInt32";
  case kTfLiteFloat16:
    return "kTfLiteFloat16";
  case kTfLiteBool:
    return "kTfLiteBool";
  case kTfLiteComplex64:
    return "kTfLiteComplex64";
  case kTfLiteNoType:
    return "kTfLiteNoType";
  default:
    return "Unknown type";
  }
}

bool modelInit(const unsigned char *model, byte *tensorArena, int tensorArenaSize)
{
  tflModel = tflite::GetModel(model);
  if (tflModel->version() != TFLITE_SCHEMA_VERSION)
  {
    Serial.println("Model schema version mismatch!");
    return false;
  }

  tflInterpreter = new tflite::MicroInterpreter(
      tflModel,
      tflOpsResolver,
      tensorArena,
      tensorArenaSize);
  tflInterpreter->AllocateTensors();
  tflInputTensor = tflInterpreter->input(0);   // https://ai.google.dev/edge/litert/microcontrollers/get_started#6_instantiate_operations_resolver
  tflOutputTensor = tflInterpreter->output(0); //?

  return true;
}

bool modelSetInput(float melspectrogram[ROWS][COLS])
{
  if (tflInputTensor == nullptr) return 0;

  float inputBuffer[ROWS * COLS];
  int index = 0;
  for (int i = 0; i < ROWS; ++i)
  {
    for (int j = 0; j < COLS; ++j)
    {
      inputBuffer[index++] = static_cast<float>(melspectrogram[i][j]);
    }
  }

  // Copy the data into the input tensor
  memcpy(tflInputTensor->data.f, inputBuffer, sizeof(inputBuffer));

  return 1;
}

bool modelRunInference()
{
  // Run inference
  if (tflInterpreter->Invoke() != kTfLiteOk)
  {
    Serial.println("Error: Inference failed.");
    return 0;
  }

  return 1;
}

float modelGetOutput()
{
  // Get the output tensor
  tflOutputTensor = tflInterpreter->output(0);

  float outputValue = tflOutputTensor->data.f[0];

  return outputValue;
}