#include <WiFi.h>
#include <time.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// WiFi Credentials
const char* ssid     = "First_2.4G";
const char* password = "0868389225";

// NTP Time settings (GMT+7 for Thailand)
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600 * 7;
const int daylightOffset_sec = 0;

// OLED Display Settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// Push Button Settings
#define BUTTON_PIN 27  

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool showIP = false; // Toggle flag for switching screen

unsigned long lastDebounceTime = 0;
const int debounceDelay = 250;  // ป้องกันการกดซ้ำเร็วเกินไป

int lastButtonState = HIGH;  // สถานะปุ่มก่อนหน้า
int buttonState;             // สถานะปุ่มปัจจุบัน

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT_PULLUP); // ตั้งค่า Pull-up

  // Connect to WiFi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi connected.");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }

  display.clearDisplay();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
  // อ่านค่าปุ่ม
  buttonState = digitalRead(BUTTON_PIN);

  // ตรวจจับขอบสัญญาณ (Falling Edge Detection)
  if (lastButtonState == HIGH && buttonState == LOW && millis() - lastDebounceTime > debounceDelay) {
    showIP = !showIP;  // Toggle screen state
    lastDebounceTime = millis();  // บันทึกเวลา
    Serial.println(F("Button Pressed!"));
  }

  lastButtonState = buttonState;  // บันทึกค่าปุ่มก่อนหน้า

  // อัปเดตหน้าจอ OLED
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  if (showIP) {
    Serial.println(F("BUTTON_PIN == switch1"));
    // Show IP Address
    display.setTextSize(1);
    display.println("ESP32 IP is");
    display.println(WiFi.localIP());
    display.setTextSize(1);
    display.println(F("Group: No.8")); 
  } else {
    Serial.println(F("BUTTON_PIN == switch2"));
    // Show Time & Date
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      display.setTextSize(1);
      char timeString[9];
      strftime(timeString, sizeof(timeString), "%H:%M:%S", &timeinfo);
      display.println(timeString); // แสดงเวลา HH:MM:SS

      char dateString[20];
      strftime(dateString, sizeof(dateString), "%d/%B/%Y", &timeinfo); 
      display.println(dateString);
    } else {
      display.println(F("Time Error"));
    }
    display.setTextSize(1);
    display.println(F("Group: No.8")); 
  }

  display.display();
}