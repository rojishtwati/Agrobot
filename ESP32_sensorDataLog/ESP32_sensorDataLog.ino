/*
  ESP32 Soil Moisture Sensor and DHT 11 for humidity and Temperature and LDR sensor for light level monitoring
  Author: Rojish Twati AKA Hunter
  Deployment ID
  AKfycbzySCEFZkg0AVnhDNbUqR0Gx0z5jPe19WMAxieKdQ43kk9Tn3yYi4CD7f9BkEhb6CU4

  URl
  https://script.google.com/macros/s/AKfycbzySCEFZkg0AVnhDNbUqR0Gx0z5jPe19WMAxieKdQ43kk9Tn3yYi4CD7f9BkEhb6CU4/exec

  javascript code 
  example
  ?switch (param) {
        case 'soilMoistureValue'://parameter
          rowData[2] = value; //value in cloumn B
          result='Written on column B'
          break;
        case 'soilMoisture': //paramter
          rowData[3] = value; //value in cloumn B
          result+='Written on column C'
          break;
        case 'temperature': //Parameter
          rowData[4] = value; //Value in column E
          result += 'Written on column E';
          break;
        case 'humidity': //Parameter
          rowData[5] = value; //Value in column F
          result += ' ,Written on column F';
          break;  
          
*/
#include <WiFi.h>
// #include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

const char* ssid = "RoboHunt";
const char* password = "Robohunt123";
// const char* googleScriptURL = "https://script.google.com/macros/s/AKfycbyAMo9Gk14Ql5rkCH1LLgNbEEFaRab5YfcVUJNZwL2wI2AT2x1-1E4MDB2BsERBgFr1/exec";
const char* host = "script.google.com";
const int httpsPort = 443;

String GAS_ID = "AKfycbzySCEFZkg0AVnhDNbUqR0Gx0z5jPe19WMAxieKdQ43kk9Tn3yYi4CD7f9BkEhb6CU4";

#include <DHT11.h>  // include DHT11 library for DHT sensor 

DHT11 dht11(4); // define pin for DHT sensor GPIO pin 4 that is D4

int moistureValue, moistureValuePercentage, temperature = 0, humidity = 0, lightIntensityValue,lightIntensityPercentage;
const int moistureSensorPin = 36; //define pin for soil sensor pin analog pin 

const int luxPin=39; // define pin for light sensor analog pin

WiFiClientSecure client;


// Timing variables
unsigned long previousMillis = 0;
const long interval =600*1000;  // 10 minutes in milliseconds

void setup() {
  // dht.begin();
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
      Serial.print(temperature);
      Serial.print(" °C\tHumidity: ");
      Serial.print(humidity);
      Serial.println(" %");
    } else {
      // Print error message based on the error code.
      Serial.println(DHT11::getErrorString(result));
    }
    moistureValue = analogRead(moistureSensorPin);
    Serial.print("Moisture value:  ");
    Serial.print(moistureValue);
    Serial.print("\t Moisture %:  ");
    moistureValuePercentage = (100 - map(moistureValue, 0, 4096, 0, 100));
    Serial.print(moistureValuePercentage);
    Serial.println("%");

    lightIntensityValue=analogRead(luxPin);
    Serial.print("LUX value:  ");
    Serial.print(lightIntensityValue);
    Serial.print("\t LUX %:  ");
    lightIntensityPercentage = (map(lightIntensityValue, 0, 4096, 0, 100));
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
  /*
  StaticJsonDocument<200> jsonDocument;
  jsonDocument["soilMoistureValue"] = soilMosVal;
  jsonDocument["soilMoisture"] = soilMos;
  jsonDocument["temperature"] = tem;
  jsonDocument["humidity"] = hum;


  String jsonString;
  serializeJson(jsonDocument, jsonString);  // Convert JSON to string

  // Send HTTP POST request
    HTTPClient http;
    http.begin(googleScriptURL);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(jsonString);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("HTTP Response Code: " + String(httpResponseCode));
      Serial.println("Response: " + response);
    } else {
      Serial.println("Error on sending POST: " + String(httpResponseCode));
    }

    http.end(); // Free resources
   

*/
}