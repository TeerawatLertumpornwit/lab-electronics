#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Clear display buffer
  display.clearDisplay();
}

void loop() {
  // หน้าจอแรก
  display.clearDisplay();
  display.setTextSize(1);      // ตั้งขนาดตัวอักษร
  display.setTextColor(SSD1306_WHITE);  // ตั้งสีตัวอักษร
  display.setCursor(0, 0);     // ตั้งตำแหน่งของตัวอักษร
  display.println(F("IoT 2102447"));
  display.println(F("Group 08"));
  display.println(F("CUEE 1-2024"));
  display.display();           // แสดงผล
  delay(1000);                 // แสดงค้างไว้ 1 วินาที

  // หน้าจอที่สอง
  display.clearDisplay();
  display.setTextSize(1);      // ตั้งขนาดตัวอักษร
  display.setTextColor(SSD1306_WHITE);  // ตั้งสีตัวอักษร
  display.setCursor(0, 0);     // ตั้งตำแหน่งของตัวอักษร
  display.println(F("OLED Testing"));
  display.println(F("Broken Beam"));
  display.println(F("Group 08"));
  display.display();           // แสดงผล
  delay(1000);                 // แสดงค้างไว้ 1 วินาที
}
