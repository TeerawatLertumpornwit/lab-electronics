/*
  Project Name: IoT Lab3 Template
  File name: IoT-3.ino
  Last modified on: November, 2024
*/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>  // for SSD1306
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>  // for BME280
#include <WiFi.h>  // for ESP32 WiFi
#include <PubSubClient.h>  // for NETPIE
#include "credentials.h"  // WiFi and NETPIE credentials
#include "iot_iconset_16x16.h"

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET -1  // OLED reset pin (or -1 if sharing Arduino reset pin)
#define OLED_ADDRESS  0x3C  // OLED I2C address (Adafruit = 0x3D China 0x3C)
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define SEALEVELPRESSURE_HPA (1013.25)
#define BME_ADDRESS 0x76  // BME I2C address (Red module = 0x77 Purple module = 0x76)
Adafruit_BME280 bme;  // I2C

WiFiClient espClient;

const char* mqtt_server = "broker.netpie.io";
const int mqtt_port = 1883;
PubSubClient client(espClient);

#define LED_BUILTIN 5

float pressure,altitude;
char msg[100];
bool led;

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i=0; i < length; i++) {
    message = message + (char)payload[i];
  }
//  Serial.println(message);
  // Warning! see in your lab sheet before change a value  in digitalWrite();
  if(String(topic) == "@msg/led") {
    if (message == "on") {
      digitalWrite(LED_BUILTIN,0);
      client.publish("@shadow/data/update", "{\"data\" : {\"led\" : \"on\"}}");
      led = true;
	  } else if (message == "off") {
      digitalWrite(LED_BUILTIN,1);
      client.publish("@shadow/data/update", "{\"data\" : {\"led\" : \"off\"}}");
	    led = false;
	  }
  	oled.fillRect(16,48,96,16,0);
  	oled.setCursor(16,48);
  	if (led == true) {
  	  oled.println(" on");
  	} else {
  	  oled.println(" off");
  	}
  	oled.display();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN,1); // For switch off on board led of IOXESP32
  delay(100);

  while(!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F("OLED failed to begin"));
    delay(100);
  }
  oled.clearDisplay(); // clears the screen and buffer
  oled.setTextSize(1);
  oled.setTextColor(WHITE, BLACK);
  oled.setCursor(0,0);
  oled.println(F("OLED passed"));
  oled.display();
  delay(100);

  while(!bme.begin(BME_ADDRESS)) {
    oled.setCursor(0,oled.getCursorY());
    oled.print(F("BME failed to begin"));
    oled.display();
    delay(100);
  }
  oled.setCursor(0,oled.getCursorY());
  oled.println(F("BME passed         "));
  oled.display();
  delay(100);

  WiFi.begin(ssid, password);
  delay(1000);

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  client.connect(mqtt_client, mqtt_username, mqtt_password);
  client.subscribe("@msg/led");
  delay(1000);
}

void loop() {

  pressure = bme.readPressure()/100.0f;  // pressure in hPa
  altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);  // altitude in m

  oled.clearDisplay();  // clears the screen and buffer
  oled.setTextSize(2);
  oled.drawBitmap(0, 0, altitude_icon16x16, 16, 16, 1);
  oled.setCursor(16,0);
  oled.println(" " + String(altitude,2) + " m");
  oled.drawBitmap(0, 48, bulb_icon16x16, 16, 16, 1);
  oled.setCursor(16,48);
  
  if (led == true) {
	  oled.println(" on");
  } else {
	  oled.println(" off");
  }
  if(WiFi.status() == WL_CONNECTED) {
    oled.drawBitmap(112, 48, wifi1_icon16x16, 16, 16, 1);
  }
  oled.display();
  
  if(client.connected()) {
    client.loop();
    
    String payload = "{\"data\": {";
    payload.concat("\"pressure\":" + String(pressure,4));
    payload.concat(", ");  // separator between data
    payload.concat("\"altitude\":" + String(0));
    payload.concat("}}");
  
    payload.toCharArray(msg, (payload.length()+1)); 
    client.publish("@shadow/data/update", msg);

  } else {
    if(WiFi.status() == WL_CONNECTED) {
      client.disconnect();
      client.connect(mqtt_client, mqtt_username, mqtt_password);
      client.subscribe("@msg/led");
    } else {
      WiFi.disconnect();
      WiFi.begin(ssid, password);
    }
  }
  delay(3000);
}
