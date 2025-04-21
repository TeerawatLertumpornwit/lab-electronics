/*
  Project Name: IoT Lab2 Multitasking Example
  File name: MTask.ino
  Last modified on: March, 2025
*/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>  // for SSD1306 OLED

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET -1  // OLED reset pin (or -1 if sharing Arduino reset pin)
#define OLED_ADDRESS  0x3C  // OLED I2C address (Adafruit = 0x3D China 0x3C)
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);  // OLED
#define LED_BUILTIN 5 // IOXESP32 LED on board pin

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
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

  xTaskCreate(
    vTaskLED
    ,  "LED"   // A name just for humans
    ,  4096  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL);
    delay(100);
  
  xTaskCreate(
    vTaskOLED
    ,  "OLED"   // A name just for humans
    ,  8192  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL);
  delay(100);
}

void vTaskLED(void * pvParameters){
  for(;;){
    digitalWrite(LED_BUILTIN,0);
    vTaskDelay(350);
    digitalWrite(LED_BUILTIN,1);
    vTaskDelay(350);
  }
}

void vTaskOLED(void * pvParameters){
  for(;;){
    oled.clearDisplay();  // clears the screen and buffer
    oled.fillCircle(44,26,8,1);
    oled.fillCircle(56,26,8,1);
    oled.fillCircle(70,24,10,1);
    oled.fillCircle(80,22,10,1);
    oled.fillCircle(62,16,16,1);
    for(uint8_t i=0; i<4; i++) {
      oled.drawLine(44+12*i,38,44+12*i,38+random(2,10),1);
    }
    oled.display();
    vTaskDelay(1000);
  }
}

void loop() {

}
