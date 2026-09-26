#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Hardware pins 
const int PIN_CS_BMI160 = 5;
const int PIN_SCK = 18;
const int PIN_MISO = 19;
const int PIN_MOSI = 23;

Adafruit_SSD1306 display(128, 64, &Wire, -1);

// Baseline and energy variables
long base_x = 0, base_y = 0, base_z = 0; 
int64_t current_peak_energy = 0;
int64_t instant_energy = 0;

// Timers
unsigned long last_sampling_us = 0;
unsigned long display_timer_ms = 0;
unsigned long peak_reset_timer_ms = 0;

void writeBMI160Register(uint8_t reg, uint8_t value) {
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(PIN_CS_BMI160, LOW);
  SPI.transfer(reg & 0x7F);
  SPI.transfer(value);
  digitalWrite(PIN_CS_BMI160, HIGH);
  SPI.endTransaction();
}

void readAccelerometerSPI(int16_t* accelData) {
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(PIN_CS_BMI160, LOW);
  SPI.transfer(0x12 | 0x80); 
  
  uint8_t x_lsb = SPI.transfer(0x00);
  uint8_t x_msb = SPI.transfer(0x00);
  uint8_t y_lsb = SPI.transfer(0x00);
  uint8_t y_msb = SPI.transfer(0x00);
  uint8_t z_lsb = SPI.transfer(0x00);
  uint8_t z_msb = SPI.transfer(0x00);

  digitalWrite(PIN_CS_BMI160, HIGH);
  SPI.endTransaction();

  accelData[0] = (int16_t)((x_msb << 8) | x_lsb);
  accelData[1] = (int16_t)((y_msb << 8) | y_lsb);
  accelData[2] = (int16_t)((z_msb << 8) | z_lsb);
}

void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED Error");
    while (true);
  }
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(0, 20);
  display.println("DO NOT");
  display.println("TOUCH!");
  display.display();
  
  delay(3000); // 3-second window to step away

  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, -1); 
  pinMode(PIN_CS_BMI160, OUTPUT);
  digitalWrite(PIN_CS_BMI160, HIGH);
  delay(20); 

  SPI.beginTransaction(SPISettings(400000, MSBFIRST, SPI_MODE0));
  digitalWrite(PIN_CS_BMI160, LOW);
  SPI.transfer(0x7F | 0x80);
  SPI.transfer(0x00);
  digitalWrite(PIN_CS_BMI160, HIGH);
  SPI.endTransaction();
  delay(50); 

  writeBMI160Register(0x7E, 0x11);
  delay(50);
  writeBMI160Register(0x40, 0x2C); // 1600 Hz ODR
  delay(50);

  // --- Gravity Baseline Calibration (200 samples) ---
  display.clearDisplay();
  display.setCursor(0, 20);
  display.setTextSize(1);
  display.println("Calibrating...");
  display.display();

  long sum_x = 0, sum_y = 0, sum_z = 0;
  int16_t accelDataTemp[3];
  
  for(int i = 0; i < 200; i++) {
    readAccelerometerSPI(accelDataTemp);
    sum_x += accelDataTemp[0];
    sum_y += accelDataTemp[1];
    sum_z += accelDataTemp[2];
    delay(2); 
  }
  
  base_x = sum_x / 200;
  base_y = sum_y / 200;
  base_z = sum_z / 200;

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.println("READY!");
  display.display();
  delay(1000);

  last_sampling_us = micros();
}

void loop() {
  
  // 1. High-frequency sampling (1600Hz)
  if (micros() - last_sampling_us >= 625) {
    last_sampling_us += 625;
    
    int16_t accelData[3];
    readAccelerometerSPI(accelData);

    long delta_x = accelData[0] - base_x;
    long delta_y = accelData[1] - base_y;
    long delta_z = accelData[2] - base_z;

    instant_energy = ((int64_t)delta_x * delta_x) + 
                     ((int64_t)delta_y * delta_y) + 
                     ((int64_t)delta_z * delta_z);

    // Update peak energy
    if (instant_energy > current_peak_energy) {
      current_peak_energy = instant_energy;
      peak_reset_timer_ms = millis(); 
    }
  }

  // 2. Auto-reset peak after 5 seconds of inactivity
  if (millis() - peak_reset_timer_ms > 5000) {
    current_peak_energy = 0;
  }

  // 3. Low-frequency display update (4Hz)
  if (millis() - display_timer_ms >= 250) {
    display_timer_ms = millis();

    display.clearDisplay();
    
    // Live energy (divided by 1000 for readability)
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Live: ");
    display.print((long)(instant_energy / 1000));
    display.println(" k");

    // Max peak registered during push-off
    display.setTextSize(2);
    display.setCursor(0, 25);
    display.println("MAX PEAK:");
    
    display.setTextSize(3);
    display.setCursor(0, 45);
    if (current_peak_energy > 0) {
      display.print((long)(current_peak_energy / 1000));
    } else {
      display.print("0");
    }
    
    display.display();
  }
}