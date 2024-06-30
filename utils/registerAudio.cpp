#include <Arduino.h>
#include <WiFiNINA.h>
#include <PDM.h>

// Hardware setup
int sendPin = 2;
int disconnectPin = 3;

// PDM variables
static const char channels = 1;
static const int frequency = 16000;
short sampleBuffer[512];
volatile int samplesRead = 0;

// WiFi variables
char ssid[6] = "delle";
char pass[12] = "Iusearchbtw";
char servername[] = "192.168.212.2";
int serverport = 2023;
WiFiClient client;

void onPDMdata()
{
  int bytesAvailable = PDM.available();
  PDM.read(sampleBuffer, bytesAvailable);
  samplesRead = bytesAvailable / sizeof(short);
}

//sends all data in the given buffer to the configured destination
void sendData(WiFiClient client, short sampleBuffer[512], volatile int samplesRead)
{
  if (samplesRead)
  {
    // Print a sample on serial
    client.write((uint8_t*) sampleBuffer, sizeof(short) * samplesRead);
  }
}

void setup()
{
  Serial.begin(9600);
  pinMode(sendPin, INPUT_PULLUP);
  pinMode(disconnectPin, INPUT_PULLUP);
  while (!Serial)
    ;

  // PDM setup
  PDM.onReceive(onPDMdata);

  //WiFI setup
  WiFi.begin(ssid, pass);
  for (int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++)
  {
    delay(20000);
    Serial.println("Connecting to WiFi..");
    WiFi.begin(ssid, pass);
  }
  
  client.connect(servername, serverport);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Setup finished!");
}

void loop()
{
  int sendButtonState = digitalRead(sendPin);
  int disconnectButtonState = digitalRead(disconnectPin);
  int captureStartTime = 0;
  int totalSamples = 0;
  static int disconnect = 0;


  // guard conditions
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi not connected");
    delay(1000);
    return;
  }
  if (disconnect) {
    return;
  }
  else if (disconnectButtonState == LOW) {
    Serial.println("Disconnect button pressed!");
    disconnect = 1;
    client.stop();
    return;
  }
  if (!client.connected())
  {
    Serial.println("Waiting for server");
    client.connect(servername, serverport);
    return;
  }



  if (sendButtonState == LOW) //button pressed: gonna read for a second from PDM
  {
    Serial.println("Button pressed");
    
    if (!PDM.begin(channels, frequency))
    {
      Serial.println("Failed to start PDM!");
      return;//failed to start PDM
    }

    captureStartTime = millis();
    while (millis() - captureStartTime < 1025)
    {
      sendData(client, sampleBuffer, samplesRead);

      totalSamples += samplesRead;
      samplesRead = 0;
    }
    
    PDM.end();
    client.stop();
    Serial.println("Sent ");
    Serial.print(totalSamples);
    Serial.println(" samples");
    
  } //end button
}