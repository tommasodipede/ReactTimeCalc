#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define MIC_PIN 4 

void showListeningScreen() {
  display.clearDisplay();             
  display.setTextSize(1);             
  display.setTextColor(SSD1306_WHITE); 
  
  display.setCursor(0, 0);            
  display.println(F("Mic status:"));
  
  display.setCursor(0, 25);           
  display.println(F("Listening..."));
  
  display.display();                  
}

void setup() {
  Serial.begin(115200);
  delay(500); 
  
  pinMode(MIC_PIN, INPUT);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("OLED ERROR"));
    for(;;); 
  }

  Serial.println("System started.");
  showListeningScreen();
}

void loop() {
  int noiseDetected = digitalRead(MIC_PIN);
  
  if (noiseDetected == HIGH) {
    Serial.println("👏 NOISE DETECTED!");
    
    display.clearDisplay();
    display.setTextSize(2);             
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(15, 25); 
    display.println(F("NOISE!"));
    display.display();
    
    delay(500); 
    showListeningScreen();
  }
  
  delay(1); 
}