/*
  ESP32 Soil Moisture Sensor and DHT 11 for humidity and Temperature and LDR sensor for light level monitoring
  Author: Rojish Twati AKA Hunter
  */
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <DHT11.h>  // include DHT11 library for DHT sensor


const char* ssid = "yourWi-Fi_SSID"; // change yourWi-Fi_SSID with your WiFi name
const char* password = "yourWi-Fi_PSW"; // change yourWi-Fi_PSW with your WiFi password
const char* host = "script.google.com";
const int httpsPort = 443;

String GAS_ID = "*****"; //google excel ID

int moistureValue, moistureValuePercentage, temperature = 0, humidity = 0, lightIntensityValue,lightIntensityPercentage;

DHT11 dht11(4); // define pin for DHT sensor GPIO pin 4 that is D4

const int moistureSensorPin = 36; //define pin for soil sensor pin analog pin 

const int luxPin=39; // define pin for light sensor analog pin

WiFiClientSecure client;

// Timing variables
unsigned long previousMillis = 0;
const long interval =600*1000;  // 10 minutes in milliseconds

void setup() {
  Serial.begin(9600); 
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  client.setInsecure();
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  // Save the last time data was sent
    int result = dht11.readTemperatureHumidity(temperature, humidity);
    // Check the results of the readings.
    // If the reading is successful, print the temperature and humidity values.
    // If there are errors, print the appropriate error messages.
    if (result == 0) {
      Serial.print("Temperature: "); 
      Serial.print(temperature); //display temp 
      Serial.print(" °C\tHumidity: ");
      Serial.print(humidity); // display humidity
      Serial.println(" %");
    } else {
      // Print error message based on the error code.
      Serial.println(DHT11::getErrorString(result));
    }
    //Check the result of the moisture reading
    moistureValue = analogRead(moistureSensorPin);
    Serial.print("Moisture value:  ");
    Serial.print(moistureValue); //display mosture 
    Serial.print("\t Moisture %:  ");
    moistureValuePercentage = (100 - map(moistureValue, 0, 4096, 0, 100)); //convert moisture value into percentage
    Serial.print(moistureValuePercentage);
    Serial.println("%");
    //Check the result of the light intensity reading
    lightIntensityValue=analogRead(luxPin);
    Serial.print("LUX value:  ");
    Serial.print(lightIntensityValue); //display light value
    Serial.print("\t LUX %:  ");
    lightIntensityPercentage = (map(lightIntensityValue, 0, 4096, 0, 100)); //convert light intensity value into percentage
    Serial.print(lightIntensityPercentage);
    Serial.println("%");
    // if (WiFi.status() == WL_CONNECTED) {
      sendData(moistureValue, moistureValuePercentage, temperature, humidity,lightIntensityPercentage);
    // }
  }
}



void sendData(int soilMosVal, int soilMos, int tem, int hum,int lux) {
  Serial.println("==========");
  Serial.print("connecting to ");
  Serial.println(host);
  
  //----------------------------------------Connect to Google host 
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
    String string_soilMoistureValue = String(soilMosVal, DEC);
    String string_soilMoisture = String(soilMos, DEC);
    String string_temperature = String(tem, DEC);
    String string_humidity = String(hum, DEC);
    String string_lux =String(lux,DEC);
    String url = "https://script.google.com/macros/s/" + GAS_ID + "/exec?temperature=" + string_temperature + "&humidity=" + string_humidity + "&soilMoistureValue=" + string_soilMoistureValue + "&soilMoisture=" + string_soilMoisture +"&lux=" +string_lux ;
    Serial.print("requesting URL: ");
    Serial.println(url);

    client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "User-Agent: BuildFailureDetectorESP32\r\n" + "Connection: close\r\n\r\n");

    Serial.println("request sent");
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        Serial.println("headers received");
        break;
      }
    }
    String line = client.readStringUntil('\n');
    if (line.startsWith("{\"state\":\"success\"")) {
      Serial.println("esp32/Arduino CI successfull!");
    } else {
      Serial.println("esp32/Arduino CI has failed");
    }
    Serial.println("reply was:");
    Serial.println("==========");
    Serial.println(line);
    Serial.println("==========");
    Serial.println("closing connection");
    client.stop();
}
