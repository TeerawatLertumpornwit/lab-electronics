#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Pin Definitions
const int buttonPin1 = 27;  // ปุ่ม K1 
const int buttonPin2 = 26;  // ปุ่ม K2
const int buttonPin3 = 25;  // ปุ่ม K3 
const int buttonPin4 = 33;  // ปุ่ม K4 

// OLED Display Settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 200;  // เวลา debounce

int lastButtonState1 = HIGH, lastButtonState2 = HIGH, lastButtonState3 = HIGH, lastButtonState4 = HIGH; // สถานะปุ่มก่อนหน้า
int buttonState1, buttonState2, buttonState3, buttonState4;             // สถานะปุ่มปัจจุบัน

void setup() {
  Serial.begin(9600);

  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  pinMode(buttonPin3, INPUT_PULLUP);
  pinMode(buttonPin4, INPUT_PULLUP);

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("Ready!"));
  display.display();
}

void loop() {
  int reading1 = digitalRead(buttonPin1);
  int reading2 = digitalRead(buttonPin2);
  int reading3 = digitalRead(buttonPin3);
  int reading4 = digitalRead(buttonPin4);

  Serial.println("..............");
  Serial.println(reading1);
  Serial.println(reading2);
  Serial.println(reading3);
  Serial.println(reading4);
  
  if (reading1 == LOW && lastButtonState1 == HIGH && (millis() - lastDebounceTime) > debounceDelay) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("IoT 2102447"));//4
    display.display();
    lastDebounceTime = millis();
  }

  if (reading2 == LOW && lastButtonState2 == HIGH && (millis() - lastDebounceTime) > debounceDelay) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Electrical CU"));//1
    display.display();
    lastDebounceTime = millis();
  }

  if (reading3 == LOW && lastButtonState3 == HIGH && (millis() - lastDebounceTime) > debounceDelay) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("OLED is work!"));//3
    display.display();
    lastDebounceTime = millis();
  }

  if (reading4 == LOW && lastButtonState4 == HIGH && (millis() - lastDebounceTime) > debounceDelay) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Your Message"));//2
    display.display();
    lastDebounceTime = millis();

  }
  
  lastButtonState1 = reading1;
  lastButtonState2 = reading2;
  lastButtonState3 = reading3;
  lastButtonState4 = reading4;
  delay(100);
}


