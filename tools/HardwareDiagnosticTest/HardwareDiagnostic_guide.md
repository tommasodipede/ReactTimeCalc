# 🎙️ Hardware Diagnostic Test: Sound Sensor & OLED

This project serves as a diagnostic tool to verify the wiring, power, and communication of a digital sound sensor (such as the KY-037 or LM393 modules) and an I2C OLED display (SSD1306).

It is highly recommended to run this code immediately after wiring your components to ensure everything is functioning correctly before moving on to more complex logic.

## 🛠️ What This Test Verifies

1. **The Display (I2C):** Confirms that the SSD1306 OLED is receiving power, the SDA/SCL lines are wired correctly, and the screen can successfully render text.

2. **The Sensor (Digital Read):** Confirms that the microphone module is powered and can successfully send a `HIGH` digital signal to the microcontroller (Pin 4) when a sound threshold is met.

## ⚠️ Crucial Step: Calibrating the KY-037 Sensor

Sound sensors like the **KY-037** do not work perfectly out of the box; **they require manual calibration**. The sensor determines what counts as a "loud noise" based on the position of its onboard potentiometer.

If your display always says "NOISE!" or never reacts to your claps, you need to adjust it:

1. **Locate the Potentiometer:** Find the small blue box with a tiny brass or silver screw on top of the sensor module.

2. **Run This Code:** Keep the microcontroller plugged in and running this diagnostic script.

3. **Adjust the Screw:** Use a small flathead or Phillips screwdriver to turn the screw.

   * *If the sensor is too sensitive (always triggering):* Turn the screw counter-clockwise.

   * *If the sensor is not sensitive enough:* Turn the screw clockwise.

4. **Test Live:** Clap your hands or snap your fingers near the microphone while turning the screw. Stop adjusting as soon as the OLED reliably switches from "Listening..." to "NOISE!" exactly when you clap.