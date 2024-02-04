//------------------------HEADER FILE-----------------------------//
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <string.h>
#include <stdlib.h>
#include <rom/rtc.h> 
#include <HTTPClient.h>
#include <ArduinoJson.h>

//-----------MEMORY CARD PRE-PROCESSOR DIRECTIVE-----------------//
#include "SPI.h"
#include "time.h"
#include <DS323x.h>
#include <MFRC522.h>

//----------------SETTING UP THE PIN IN ESP----------------------// 
#define SS_PIN 5
#define RST_PIN 2
#define RELAY_PIN1 0 
#define RELAY_PIN2 4
#define GREEN_LED 14
#define RED_LED 12
#define MAC_ID "RFID_001"

//------------------------json Variable-------------------------//
String jsonBuffer;
int httpResponseCode;


//---------------------WiFi Credentials-----------------------//
const char* ssid = "B2 706";
const char* password = "9560894222";
IPAddress local_IP(192, 168, 2, 11);
IPAddress gateway(192, 168, 3, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(192, 168, 3, 7);

//---------------------- WiFi Function----------------------------//
void wifi_reconnect()
{
  if (WiFi.status() != WL_CONNECTED) {
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS)) {
      Serial.println("STA Failed to configure");
    }
    WiFi.begin(ssid, password);
    Serial.println("Connecting");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("Connected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
}

void TaskOne(void *pvParameters)
{
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());
  for (;;)
  {
    // Here, write the code for Processor Core 1 Task
    wifi_reconnect();
    // machine_operation();
    delay(1000); // Delay between iterations
  }
}

//-------------------------------SETUP--------------------------------//
void setup()
{
  Serial.begin(115200);
  
  //------------------------- IO Declaration---------------------------//
  pinMode(RELAY_PIN1, OUTPUT);
  pinMode(RELAY_PIN2, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  //-------------------------Connect to WiFi-------------------------//
  Wire.begin();
  SPI.begin();
  wifi_reconnect();

  xTaskCreatePinnedToCore(
    TaskOne,     // Task function 
    "Task1",     // Name of task 
    8192,        // Stack size of task (adjust as needed)
    NULL,        // Parameter of the task 
    1,           // Priority of the task 
    NULL,        // Task handle to keep track of created task 
    1            // Pin task to core 1
  );
}

//----------------------------------LOOP--------------------------------------//
void loop()
{
  // Perform any necessary operations in the loop if required
  delay(100); // Add a small delay to prevent excessive CPU usage
}
