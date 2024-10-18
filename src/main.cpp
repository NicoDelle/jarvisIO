#include <Arduino.h>
#include <WiFiNINA.h>
#include <PDM.h>

#include "audioProvider.h"

/**
#include <ArduTFLite.h>
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

int buttonPin = 2;
char ssid[6] = "delle";
char pass[12] = "Iusearchbtw";
char servername[] = "192.168.15.2";
int serverport = 2023;
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
    AudioProvider audioProvider;
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