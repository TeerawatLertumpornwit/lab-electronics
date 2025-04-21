#include <Wire.h>
#include <Adafruit_TSL2561_U.h>
#include <WiFi.h>
#include <time.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <PubSubClient.h>
#include "credentials.h"
#include "iot_iconset_16x16.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define PIR_PIN 36  // GPIO36 สำหรับเซ็นเซอร์ PIR
#define OUTPUT_1 25 // RED LED (output)
#define OUTPUT_2 26 // YELLOW LED (output)
#define OUTPUT_3 27 // GREEn LED (output)
#define BUTTON_1 33
#define BUTTON_2 32

// NTP Time settings (GMT+7 for Thailand)
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600 * 7;
const int daylightOffset_sec = 0;

Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
WiFiClient espClient;
const char* mqtt_server = "broker.netpie.io";
const int mqtt_port = 1883;
PubSubClient client(espClient);

int pirState = 0;
float lux = 0.0;
String led_color = "OFF";
char msg[100];
// ตัวแปรสถานะหน้าจอและสถานะปุ่มก่อนหน้า
int Switch_State = 0;
bool lastB1 = HIGH, lastB2 = HIGH;


void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(PIR_PIN, INPUT);
  pinMode(OUTPUT_1, OUTPUT);
  pinMode(OUTPUT_2, OUTPUT);
  pinMode(OUTPUT_3, OUTPUT);

  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);

  while (!tsl.begin()) {
    Serial.println(F("TSL2561 failed to begin"));
    delay(100);
  }
  
  while(!oled.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
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

  WiFi.begin(ssid, password);
  delay(100);

  client.setServer(mqtt_server, mqtt_port);
  client.connect(mqtt_client, mqtt_username, mqtt_password);
  Serial.println(F("NETPIE CONNECTED"));
  delay(100);

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  xTaskCreate(vTaskPIR, "PIR", 4096, NULL, 1, NULL);
  xTaskCreate(vTaskLUX, "LUX", 4096, NULL, 1, NULL);
  xTaskCreate(vTaskTraffic, "Traffic", 4096, NULL, 1, NULL);
  xTaskCreate(vTaskConnectionCheck, "ConnectionCheck", 4096, NULL, 1, NULL);
  xTaskCreate(vTaskOLED, "OLED", 8192, NULL, 1, NULL);
}

void loop() {
  // ใช้ FreeRTOS
}

// Task 1: Read PIR, send to NETPIE every 1 second
void vTaskPIR(void * pvParameters) {
  for (;;) {
    pirState = digitalRead(PIR_PIN); // อ่านค่า PIR
    
    if (client.connected()) {
      String payload = "{\"data\": {\"PIR\": " + String(pirState ? "true" : "false") + "}}";
      payload.toCharArray(msg, (payload.length() + 1));
      client.publish("@shadow/data/update", msg);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS); //delay 1 seconds
  }
}

// Task 2: Read LUX, send to NETPIE every 1 second
void vTaskLUX(void * pvParameters) {
  for (;;) {
    sensors_event_t event;
    tsl.getEvent(&event); // อ่านค่า LUX จากเซนเซอร์
    lux = event.light; // เก็บค่าแสงในตัวแปร
    if (client.connected()) {
      String payload = "{\"data\": {\"LUX\": " + String(event.light, 2) + "}}";
      payload.toCharArray(msg, (payload.length() + 1));
      client.publish("@shadow/data/update", msg);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS); //delay 1 seconds
  }
}

// Task 3: Control LEDs based on PIR and LUX every 1 second
void vTaskTraffic(void * pvParameters) {
  for (;;) {
    sensors_event_t event;
    tsl.getEvent(&event);

    Serial.print("lux Sensor: ");
    Serial.println(lux);
    Serial.print("PIR Sensor: ");
    Serial.println(pirState);

    if (lux < 50 && pirState == HIGH) { 
        digitalWrite(OUTPUT_1, HIGH);
        digitalWrite(OUTPUT_2, LOW);
        digitalWrite(OUTPUT_3, LOW);
        led_color = "RED";
        Serial.println("LED RED ON");
    } else if (lux >= 50 && pirState == HIGH) {
        digitalWrite(OUTPUT_1, LOW);
        digitalWrite(OUTPUT_2, HIGH);
        digitalWrite(OUTPUT_3, LOW);
        led_color = "YELLOW";
        Serial.println("LED YELLOW ON");
    } else {
        digitalWrite(OUTPUT_1, LOW);
        digitalWrite(OUTPUT_2, LOW);
        digitalWrite(OUTPUT_3, HIGH);
        led_color = "GREEN";
        Serial.println("LED GREEN ON");
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
// Task 4: Check WiFi and NETPIE connection every 1 seconds
void vTaskConnectionCheck(void * pvParameters) {
  for (;;) {
  if(client.connected()) {
    client.loop();
    client.publish("@shadow/data/update", msg);

  } else {
    if(WiFi.status() == WL_CONNECTED) {
      client.disconnect();
      client.connect(mqtt_client, mqtt_username, mqtt_password);
    } else {
      WiFi.disconnect();
      WiFi.begin(ssid, password);
    }
  }
    vTaskDelay(1000 / portTICK_PERIOD_MS);  //delay 1 seconds
  }
}

// Task 5: Display data on OLED every 0.5 second
void vTaskOLED(void *pvParameters) {
  for (;;) {
    bool b1 = digitalRead(BUTTON_1);
    bool b2 = digitalRead(BUTTON_2);
    
    // ตรวจจับขอบ HIGH → LOW เมื่อปุ่มถูกกด
    if (lastB1 == HIGH && b1 == LOW) {
      Switch_State = 1;
    } else if (lastB2 == HIGH && b2 == LOW) {
      Switch_State = 2;
    }

    // อัปเดตสถานะปุ่มล่าสุด
    lastB1 = b1;
    lastB2 = b2;
    delay(100);

    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(0, 0);

    if (Switch_State == 1) {
      // Display PIR with icon
      oled.drawBitmap(0, 0, warning_icon16x16, 16, 16, 1);  
      oled.setCursor(20, 0);
      oled.print(F("PIR: "));
      oled.println(pirState);

      // Display LUX with icon
      oled.drawBitmap(0, 16, sun_icon16x16, 16, 16, 1);  
      oled.setCursor(20, 16);
      oled.print(F("LUX: "));
      oled.println(lux, 2);

      // Display Traffic with icon
      oled.drawBitmap(0, 32, siren_icon16x16, 16, 16, 1);  
      oled.setCursor(20, 32);
      oled.print(F("Traffic: "));
      oled.println(led_color);

      // Display WiFi Status and Icon
      if (WiFi.status() == WL_CONNECTED) {
        oled.drawBitmap(112, 48, wifi1_icon16x16, 16, 16, 1);  
        oled.setCursor(64, 56);
        oled.println("WiFi: OK");
      } else {
        oled.setCursor(64, 56);
        oled.println("WiFi: ...");
      }

    } else if (Switch_State == 2) {
      // Display Traffic with icon
      oled.drawBitmap(0, 0, siren_icon16x16, 16, 16, 1);  
      oled.setCursor(20, 0);
      oled.print(F("LED: "));
      oled.println(led_color);

      struct tm timeinfo;
      if (getLocalTime(&timeinfo)) {

        // Display Time with icon
        oled.drawBitmap(0, 16, clock_icon16x16, 16, 16, 1);  
        oled.setCursor(20, 16);
        char timeString[9];
        strftime(timeString, sizeof(timeString), "%H:%M:%S", &timeinfo);
        oled.println(timeString); // แสดงเวลา HH:MM:SS

        // Display Date with icon
        oled.drawBitmap(0, 32, wallplug_icon16x16, 16, 16, 1);  
        oled.setCursor(20, 32);
        char dateString[20];
        strftime(dateString, sizeof(dateString), "%d/%B/%Y", &timeinfo); 
        oled.println(dateString);

      } else {
        // Display Time Error
        oled.setCursor(20, 16);
        oled.println(F("Time Error"));
      }
      
      // Display WiFi Status and Icon
      if (WiFi.status() == WL_CONNECTED) {
        oled.drawBitmap(112, 48, wifi1_icon16x16, 16, 16, 1);  
        oled.setCursor(64, 56);
        oled.println("WiFi: OK");
      } else {
        oled.setCursor(64, 56);
        oled.println("WiFi: ...");
      }

    } else {
      oled.println(F("Press Button 1 or 2"));
    }

    oled.display();

    vTaskDelay(500 / portTICK_PERIOD_MS); //delay 0.5 seconds
  }
}
