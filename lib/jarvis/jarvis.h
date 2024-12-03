#include <Arduino.h>
#include <common.h>
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>

#define ROWS 32
#define COLS 21

bool modelInit(const unsigned char* model, byte* tensorArena, int tensorArenaSize);

//input dims: (1,32,21)
/**
 * @brief Sets the input in place. Input dimensions: `(1, 32, 21)`
 *
*/
bool modelSetInput(float melspectrogram[ROWS][COLS]);
bool modelRunInference();
float modelGetOutput();