#include <Arduino.h>
#include <WiFiNINA.h>
#include <PDM.h>

#include "audioProvider.h"
#include "config.h"

/**
#include <ArduTFLite.h>const char* ssid = "YOUR_SSID";
const char* pass = "YOUR_PASSWORD";
const char* servername = "YOUR_SERVERNAME";
const int serverport = 2023;
#include "jarvis026.h" 
*constexpr int kTensorArenaSize = 70000;
*alignas(16) uint8_t tensor_arena[kTensorArenaSize];
*float data[SR];

float runInference(float data[SR])
{
  for (int i = 0; i < SR; i++)
  {
      modelSetInput(data[i], i); //the input matrix is flattened to a 1D, 64*16 array
  }
  modelRunInference();
  return modelGetOutput(0);
}
*/

AudioProvider audioProvider;

int buttonPin = 2;

WiFiClient client;

//sends all data in the given buffer to the configured destination
void sendData(WiFiClient& client, AudioProvider& audioProvider)
{

  // Print a sample on serial
  client.write((uint8_t*) audioProvider.audio, sizeof(float) * SR);
}

//all code to be executed once goes here
void setup()
{
  // Initialize serial communications and wait for Serial Monitor to be opened
  Serial.begin(9600);
  while (!Serial)
    ;

  /**
  Serial.println("Initializing TensorFlow Lite Micro Interpreter...");
  if (!modelInit(utils_models_jarvis0_2_6_tflite, tensor_arena, kTensorArenaSize))
  {
    Serial.println("Model initialization failed!");
    while (true)
      ;
  }
  Serial.println("Model initialized!");
  */
  //PDM setup
  if (!PDMsetup())
  {
    while (1) {};
  }
  //WiFI setup
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  if (client.connect(servername, serverport)) {
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    Serial.println("Setup finished!");
    Serial.println("Starting in:");
    for (int i = 0; i < 3; i++)
    {
      Serial.println(3 - i);
      delay(1000);
    }
    audioProvider.init();
    sendData(client, audioProvider);
  } else {
    Serial.println("Connection to server failed!");
  }
}

void loop()
{

  //basta copiare i dati in ordine da un buffer a un array. Probabilmente conviene poi shiftarli e ripiazzare quelli nuovi in coda, ma dipende dai tempi di esecuzione del modello

  
  //runInference(DATA);
  return;
}