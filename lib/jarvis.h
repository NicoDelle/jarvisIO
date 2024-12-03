#include <Arduino.h>
#include <common.h>
#include <Chirale_TensorFlowLite.h>
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>

#define ROWS 32
#define COLS 21

bool modelInit(const unsigned char* model, byte* tensorArena, int tensorArenaSize);
bool modelSetInput(double melspectrogram[ROWS][COLS]);
bool modelRunInference();
float modelGetOutput(int index);