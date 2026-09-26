# KY-037 Acoustic Sensor Calibration Tool 

## Overview 📖
This code is a standalone utility designed to test and calibrate the acoustic sensor (KY-037) used in the Reaction Time System. 

Unlike the IMU sensor which relies on software thresholds, the KY-037 microphone module relies on a **hardware threshold**. It has an onboard potentiometer (a small screw) that dictates at what decibel level the digital pin goes `HIGH`. This tool provides instant visual feedback to help you tune that physical screw perfectly, ensuring it only triggers on loud sounds (like a starter gun or a loud clap) and ignores background noise.

## How It Works ⚙️
1.  **Idle State:** The ESP32 monitors the digital pin (GPIO 4) connected to the microphone. The OLED displays **"Listening..."**.
2.  **Trigger Detection:** The moment the sound exceeds the physical threshold set on the KY-037, the digital pin goes `HIGH`.
3.  **Visual Feedback:** The OLED immediately flashes **"NOISE!"** and the serial monitor logs the event.
4.  **Cooldown:** A brief 500ms cooldown is applied to prevent screen flickering before it returns to listening mode.

## Calibration Procedure 🛠️

### 1. Setup 🔌
* Flash this code to the ESP32.
* Ensure the KY-037 microphone is connected to **Pin D4** as per the wiring diagram.

### 2. Initial Test 👏
* Power on the system. The screen should read **"Listening..."**.
* Clap your hands loudly near the microphone. If the screen flashes **"NOISE!"**, the sensor is working.

### 3. Physical Tuning (The Potentiometer) 🪛
Grab a small Phillips or flathead screwdriver. Locate the small brass/blue screw on the KY-037 module.
*  **If it triggers too easily** (e.g., from talking or light wind): Turn the screw to *decrease* sensitivity.
*  **If it doesn't trigger** (e.g., even when clapping loudly): Turn the screw to *increase* sensitivity.

### 4. Final Verification ✅
* Take the system to your track or testing environment.
* Simulate a race start with your actual starting gun, clapper board, or whistle.
* Adjust the screw until the system reliably catches **every single start command**, while ignoring ambient chatter and footsteps. Once set, the hardware is ready for the main race firmware!
