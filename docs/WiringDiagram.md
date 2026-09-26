# Quick Wiring Diagram: ESP32 + SPI/I2C Sensors

## 1. ESP32 (The Brain)[cite: 1]
* **POWER:** Power Bank via USB Type-C
* **Pin 3V3** --> Common Positive (+) line
* **Pin GND** --> Common Negative (-) line

## 2. KY-037 Microphone (The Starter)[cite: 1]
* **+ / VCC** --> Common Positive (+) line
* **G / GND** --> Common Negative (-) line
* **D0 (Digital)** --> ESP32 Pin D4

## 3. BMI160 Accelerometer (The Detector - via SPI)[cite: 1]
* **VCC** --> Common Positive (+) line
* **GND** --> Common Negative (-) line
* **CS** --> ESP32 Pin D5 (Chip Select)
* **SCL (SCK)** --> ESP32 Pin D18 (SPI Clock)
* **SDA (MOSI)** --> ESP32 Pin D23 (SPI Master Out / Slave In)
* **SAO (MISO)** --> ESP32 Pin D19 (SPI Master In / Slave Out)

## 4. OLED Display (The Screen - via I2C)[cite: 1]
* **VCC** --> Common Positive (+) line
* **GND** --> Common Negative (-) line
* **SCL** --> ESP32 Pin D22 (I2C Clock)
* **SDA** --> ESP32 Pin D21 (I2C Data)

---
### ⚠️ Technical Notes
* Ensure all components share the same GND (Ground)
* The Positive (+) line MUST be powered strictly by the 3.3V pin from the ESP32
* Keep the SPI cables (BMI160) as short as possible to maintain high-frequency signal stability