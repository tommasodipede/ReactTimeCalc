# ReactTimeCalc
An ESP32-based embedded system to measure track and field sprint reaction times with millisecond precision.

## 🚀 Features
* **Zero Blind-Spot Measurement:** Uses a non-blocking hardware metronome (1600Hz ODR) for instant physical detection.
* **Acoustic Sync:** Listens for the starter gun commands ("On your marks", "Set", "BANG").
* **False Start Detection:** Automatically flags movements occurring before the gun or under the physiological limit of 100ms.
* **OLED Feedback:** Immediate visual feedback of the Reaction Time (RT) or False Start status.

## 📁 Repository Structure
* `/src` - Contains the main race firmware.
* `/tools` - Contains calibration tools (e.g., the threshold calibrator to measure block energy).
* `/docs` - Hardware components and wiring diagrams.

## 🛠️ Hardware Requirements & Setup
Please see the [Hardware Documentation](docs/HardwareComponents.md) for the full list of components and the [Wiring Diagram](docs/WiringDiagram.md) for pinout connections.

## 💻 Software Setup
1. Install the ESP32 board in the Arduino IDE.
2. Install the following libraries via the Library Manager:
   * `Adafruit GFX Library`
   * `Adafruit SSD1306`
3. Flash the `bmi160_threshold_calibrator` from the `/tools` folder first to find your optimal block energy threshold (Read the tool's guide for instructions).
4. Enter the calculated threshold into the main firmware.
5. Flash the main firmware from `/src/reaction_time_main`.

## ⚠️ Known Limitations & Challenges
While this system provides high precision and speed, it is a DIY embedded project. Please be aware of the following environmental limitations during track use:

* 🎤 **Acoustic Interference (Track Confusion):** The KY-037 microphone detects raw *volume* (decibels), not specific sound profiles. Loud shouts, starting guns from adjacent tracks, or even strong wind blowing directly into the microphone can cause false phase triggers. 
* 📳 **Track Vibrations:** The BMI160 IMU is highly sensitive. If the starting blocks are not firmly anchored to the track, or if people are jumping/walking heavily immediately next to the blocks during the "Set" phase, the vibrations might exceed the energy threshold and trigger a false start.
* ⚡ **Wiring Sensitivity:** High-frequency SPI communication (used by the accelerometer) requires very short cables. Using long or loose jumper wires can cause data corruption or freeze the ESP32. Keep the wiring as compact as possible.
* 🌦️ **Environmental Factors:** This setup uses exposed electronics. Rain, high humidity, or track dirt can damage the components or alter the sensor readings. (Tip: Consider designing a 3D-printed enclosure for long-term outdoor use!).
