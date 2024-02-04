#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
//#include <ArduinoJson.h>

int temperature, humidity;
String serverURL = "http://thirdumpire-baseball.com/iot/api/sensor.php";
int data_update_flag;

#define DHTPIN 13        
#define DHTTYPE DHT11 
#define device_id "1234"
#define sensor "DHT11"

DHT dht(DHTPIN, DHTTYPE);
//---------------------WiFi Creadientials-----------------------//
const char*  ssid = "edge 30";
const char*  password = "01234567";
IPAddress    local_IP(192, 168, 2, 11);
IPAddress    gateway(192, 168, 3, 1);
IPAddress    subnet(255, 255, 255, 0);
IPAddress    primaryDNS(192, 168, 3, 7);

//---------------------- WiFi Function----------------------------//
// void wifi_connect()
// {
//   if(WiFi.status() != WL_CONNECTED) {
//     if (!WiFi.config(local_IP, gateway, subnet, primaryDNS)) {
//     Serial.println("STA Failed to configure");
//   }
//   WiFi.begin(ssid, password);
//   Serial.println("Connecting");
//   delay(500);
//   Serial.print(".");
//   }
// Serial.print("Connected to WiFi With IP Address:");
// Serial.println(WiFi.localIP());
// }
void wifi_connect() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println();
  }

  Serial.print("Connected to WiFi with IP Address: ");
  Serial.println(WiFi.localIP());
}

//temperature and Humidity
void readTemperatureHumidity(){
  temperature = dht.readTemperature();  
  humidity = dht.readHumidity();        

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read data from DHT sensor!");
    return;
  }
  
  Serial.print("Temperature:");
  Serial.print(temperature);
  Serial.print("°C  ");
  Serial.print("  Humidity:");
  Serial.print(humidity);
  Serial.println("%");
 
}

/* -----------------------------HTTP Request---------------------------------------*/

void uploadSensorReading(){
  HTTPClient http;
  String dataURL="";
  dataURL=serverURL+"?"+"device_id="+(String)device_id +"&sensor_type="+(String)sensor+"&temp="+(String)temperature+"&humidity="+(String)humidity;
  Serial.println(dataURL);
  http.begin(dataURL);
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.println(payload);
    
  } else {
    data_update_flag=0;
    Serial.print("HTTP GET request failed with error code ");
    Serial.println(httpCode);
  }
  http.end();
}


void setup() {
  Serial.begin(115200);
  wifi_connect();
  dht.begin();

}

void loop() {
  delay(10000); 
  wifi_connect();
  readTemperatureHumidity();
  uploadSensorReading();
}
