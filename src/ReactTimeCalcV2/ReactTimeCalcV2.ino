#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Hardware pins
const int PIN_MIC = 4;
const int PIN_CS_BMI160 = 5;
const int PIN_SCK = 18;
const int PIN_MISO = 19;
const int PIN_MOSI = 23;

Adafruit_SSD1306 display(128, 64, &Wire, -1);

// Race parameters
const int64_t ENERGY_THRESHOLD = 800000;   // Calibrate based on starting block
const unsigned long FALSE_START_LIMIT_US = 100000; // 100 ms false start limit
const float FILTER_ALPHA = 0.05;         // IIR smoothing for baseline

enum RacePhase {
  INITIAL,
  ON_YOUR_MARKS,
  SET_AND_BASELINE,
  AWAIT_GUN_AND_MOVE,
  RESULT
};
RacePhase current_phase = INITIAL;

// Interrupt variables protected by MUX
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED; 
volatile int sound_count = 0;
volatile unsigned long time_on_your_marks_ms = 0;
volatile unsigned long time_set_ms = 0;  
volatile unsigned long time_gun_us = 0;      
volatile bool gun_fired = false;
volatile bool mic_enabled = false;

float buffer_x = 0.0, buffer_y = 0.0, buffer_z = 0.0;
unsigned long movement_time_us = 0;
unsigned long display_on_time = 0;
unsigned long timer_set_start = 0;
unsigned long potential_movement_start = 0;
int samples_above_threshold = 0;

String result_message = "";
float final_reaction_time = 0.0;
bool display_updated_flag = false; 

void writeBMI160Register(uint8_t reg, uint8_t value) {
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(PIN_CS_BMI160, LOW);
  SPI.transfer(reg & 0x7F);
  SPI.transfer(value);
  digitalWrite(PIN_CS_BMI160, HIGH);
  SPI.endTransaction();
}

void readSPIAccelerometer(int16_t* accelData) {
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

void IRAM_ATTR detectGunshot() {
  if (!mic_enabled) return;

  unsigned long now_us = micros();
  unsigned long now_ms = now_us / 1000;

  // Prevent data corruption during interrupt
  portENTER_CRITICAL_ISR(&mux);
  
  if (sound_count == 0) {
    time_on_your_marks_ms = now_ms;
    sound_count = 1;
  } 
  else if (sound_count == 1) {
    if (now_ms - time_on_your_marks_ms > 1500) {
      time_set_ms = now_ms;
      sound_count = 2;
    }
  }
  else if (sound_count == 2) {
    if (now_ms - time_set_ms > 800) {
      time_gun_us = now_us; 
      gun_fired = true;
      sound_count = 3;
    }
  }
  
  portEXIT_CRITICAL_ISR(&mux);
}

void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Error: OLED Screen not found");
    while (true);
  }
  display.setTextColor(WHITE);

  pinMode(PIN_MIC, INPUT);

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

  attachInterrupt(digitalPinToInterrupt(PIN_MIC), detectGunshot, RISING);
  
  current_phase = INITIAL;
}

void loop() {
  
  switch (current_phase) {
    
    case INITIAL:
      if (!display_updated_flag) {
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(10, 25);
        display.println("LISTENING");
        display.display();
        
        // Atomic reset
        portENTER_CRITICAL(&mux);
        mic_enabled = false; 
        sound_count = 0;
        gun_fired = false;
        portEXIT_CRITICAL(&mux);

        movement_time_us = 0;
        potential_movement_start = 0;
        samples_above_threshold = 0;
        
        delay(10); // Acoustic debounce
        mic_enabled = true; 
        
        display_updated_flag = true;
      }
      
      if (sound_count >= 1) {
        display_updated_flag = false;
        current_phase = ON_YOUR_MARKS;
      }
      break;

    case ON_YOUR_MARKS:
      if (!display_updated_flag) {
        display.clearDisplay();
        display.setCursor(15, 15);
        display.println("ON YOUR");
        display.setCursor(35, 40);
        display.println("MARKS");
        display.display();
        Serial.println("Phase: ON YOUR MARKS");
        display_updated_flag = true;
      }

      if (millis() - time_on_your_marks_ms > 30000) {
        Serial.println("On Your Marks Timeout. Reset.");
        display_updated_flag = false;
        current_phase = INITIAL;
      } 
      else if (sound_count >= 2) {
        display_updated_flag = false;
        current_phase = SET_AND_BASELINE;
      }
      break;

    case SET_AND_BASELINE:
      if (!display_updated_flag) {
        display.clearDisplay();
        display.setTextSize(3);
        display.setCursor(35, 20);
        display.println("SET");
        display.display();
        Serial.println("Phase: SET. Continuous Baseline Calculation (Zero Blind Spot)...");
        
        timer_set_start = millis();
        
        // Seed low-pass filter to prevent starting from 0
        int16_t seedData[3];
        readSPIAccelerometer(seedData);
        buffer_x = seedData[0];
        buffer_y = seedData[1];
        buffer_z = seedData[2];

        display_updated_flag = true;
        current_phase = AWAIT_GUN_AND_MOVE;
      }
      break;

    case AWAIT_GUN_AND_MOVE:
    {
      static unsigned long last_sampling_time = 0;
      if (last_sampling_time == 0) last_sampling_time = micros();

      bool is_stabilizing = (millis() - timer_set_start < 1200);

      // Safety timeout
      if (millis() - time_set_ms > 10000) {
         Serial.println("Starter Timeout. Reset.");
         last_sampling_time = 0; 
         display_updated_flag = false;
         current_phase = INITIAL;
         break;
      }

      // Hardware metronome for fixed sampling rate
      if (micros() - last_sampling_time >= 625) {
        last_sampling_time += 625; 
        
        int16_t accelData[3];
        readSPIAccelerometer(accelData);

        // Update baseline smoothly during lift (1.2s)
        if (is_stabilizing) {
           buffer_x = (1.0 - FILTER_ALPHA) * buffer_x + (FILTER_ALPHA * accelData[0]);
           buffer_y = (1.0 - FILTER_ALPHA) * buffer_y + (FILTER_ALPHA * accelData[1]);
           buffer_z = (1.0 - FILTER_ALPHA) * buffer_z + (FILTER_ALPHA * accelData[2]);
        } 
        else {
           long delta_x = accelData[0] - (long)buffer_x;
           long delta_y = accelData[1] - (long)buffer_y;
           long delta_z = accelData[2] - (long)buffer_z;

           int64_t energy = ((int64_t)delta_x * delta_x) + 
                            ((int64_t)delta_y * delta_y) + 
                            ((int64_t)delta_z * delta_z);

           if (energy > ENERGY_THRESHOLD) {
             samples_above_threshold++;
             
             // Log first threshold crossing
             if (samples_above_threshold == 1) {
                potential_movement_start = micros();
             }
             
             // Anti-spike: requires 3 consecutive samples
             if (samples_above_threshold >= 3) {
                movement_time_us = potential_movement_start;
                
                last_sampling_time = 0; 
                samples_above_threshold = 0;
                
                // Atomic read to prevent race conditions
                portENTER_CRITICAL(&mux);
                bool snap_gun_fired = gun_fired;
                unsigned long snap_time_gun = time_gun_us;
                portEXIT_CRITICAL(&mux);

                // Detect physical movement before the gunshot or missing shot
                if (!snap_gun_fired || (movement_time_us < snap_time_gun)) { 
                   result_message = "FALSE:\nEARLY";
                   Serial.println("FALSE START: Movement before gun (or Ghost-Shot).");
                } 
                else {
                   unsigned long raw_rt_us = movement_time_us - snap_time_gun;
                   
                   if (raw_rt_us < FALSE_START_LIMIT_US) {
                      result_message = "FALSE:\n< 0.100s";
                      Serial.print("FALSE START (Impossible Reaction): ");
                      Serial.print(raw_rt_us / 1000000.0, 4);
                      Serial.println(" s");
                   } 
                   else {
                      final_reaction_time = raw_rt_us / 1000000.0;
                      result_message = "VALID";
                      Serial.print("VALID START! RT: ");
                      Serial.println(final_reaction_time, 4);
                   }
                }
                
                display_updated_flag = false;
                current_phase = RESULT;
             }
           } else {
             // Reset if transient noise
             samples_above_threshold = 0;
           }
        }
      }
      break; 
    }

    case RESULT:
      if (!display_updated_flag) {
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(0, 10);
        
        if (result_message == "VALID") {
          display.println("RT (sec):");
          display.setTextSize(3);
          display.setCursor(0, 35);
          display.print(final_reaction_time, 3);
        } else {
          display.println(result_message); 
        }
        display.display();
        
        display_on_time = millis();
        display_updated_flag = true;
      }
      
      if (millis() - display_on_time >= 10000) {
        Serial.println("System Reset.");
        display_updated_flag = false;
        current_phase = INITIAL;
      }
      break;
  }
}