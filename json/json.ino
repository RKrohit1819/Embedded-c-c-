#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "PLANT_BK";
const char* password = "Best@#123#";

//Your Domain name with URL path or IP address with path
const char* serverUrl = "http://192.168.3.19/eol/module/autoprn/API/api/dataview.php?device_id=Device_12345";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 5000;

String sensorReadings;
int sensorReadingsArr[3];

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
  
   HTTPClient http;
  http.begin(serverUrl);
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.println(payload);
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
    } else {
      const char* message = doc["message"];
      int value = doc["value"];
      float temperature = doc["temperature"];
      bool isTrue = doc["isTrue"];
      Serial.print("Received message: ");
      Serial.println(message);
      Serial.print("Received value: ");
      Serial.println(value);
      Serial.print("Received temperature: ");
      Serial.println(temperature);
      Serial.print("Received boolean value: ");
      Serial.println(isTrue);
      
    }
  } else {
    Serial.print("HTTP GET request failed with error code ");
    Serial.println(httpCode);
  }
  
  
  http.end();
  
  
}


void loop(){

}