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
Please see the [Hardware Documentation](docs/BOM.md) for the full list of components and the [Wiring Diagram](docs/wiring_diagram.md) for pinout connections.

## 💻 Software Setup
1. Install the ESP32 board in the Arduino IDE.
2. Install the following libraries via the Library Manager:
   * `Adafruit GFX Library`
   * `Adafruit SSD1306`
3. Flash the `bmi160_threshold_calibrator` from the `/tools` folder first to find your optimal block energy threshold (Read the tool's guide for instructions).
4. Enter the calculated threshold into the main firmware.
5. Flash the main firmware from `/src/reaction_time_main`.
