#include <JC_Button.h>
#include <U8g2lib.h>

U8G2_SH1106_128X64_NONAME_2_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);

const int     CURRENT [] = {0,55,108,160,214,267,320,374,427,480,532};
const int     PWM_STEP = 3;
const int     PWM_MAX = (sizeof(CURRENT)/sizeof(CURRENT[0]) - 1) * PWM_STEP;
float         VCC_CALIBRATION = 4.8; // Real voltage of Arduino 5V pin
const float   CUTOFF_BATTERY_VOLTAGE = 2.6;
const byte    PWM_PIN = 10;
const int     BAT_PIN = A0;

int           pwmValue = 0;
unsigned long startTime = 0;
bool          calc = false;

Button        upButton   (2, 25, true, true);
Button        downButton (3, 25, true, true);

void setup () {
  pinMode(PWM_PIN, OUTPUT);
  analogWrite(PWM_PIN, pwmValue);
  float BAT_Voltage = measureBatteryVoltage();

  upButton.begin();
  downButton.begin();
  
  display.begin();
  display.setFont(u8g2_font_helvR08_tr);
  display.firstPage();
  do {
    display.drawStr(12, 30, "Batery Capacity Tester");
  } while (display.nextPage());

  delay(3000);
}

void loop() {
  upButton.read();
  downButton.read();

  float BAT_Voltage = analogRead(BAT_PIN) * VCC_CALIBRATION / 1024.0;

  if (upButton.wasReleased() && pwmValue < PWM_MAX && calc == false) {
    pwmValue = pwmValue + PWM_STEP;
  }

  if (downButton.wasReleased() && pwmValue > 0 && calc == false) {
    pwmValue = pwmValue - PWM_STEP;
  }

  display.firstPage();
  do {
    display.drawStr( 0, 15, "Battery:");
    display.drawStr(45, 15, BAT_Voltage > 0.2f ? (String(BAT_Voltage) + " V").c_str() : "Disconnected");
    display.drawStr( 0, 30, "Current:");
    display.drawStr(45, 30, (String(CURRENT[pwmValue/PWM_STEP]) + " mA").c_str());
    display.drawStr( 0, 45, "To start:");
    display.drawStr(45, 45, "long press UP");
    display.drawStr( 0, 60, "To stop:");
    display.drawStr(45, 60, "long press DOWN");
  } while (display.nextPage());

  if (upButton.pressedFor(1000) && calc == false) {
    timerInterrupt();
  }
}

float measureBatteryVoltage () {
  float sample = 0;
  for (int i=0; i < 100; i++) {
    sample = sample + analogRead(BAT_PIN);
    delay(2);
  }
  sample = sample/100;

  return sample * VCC_CALIBRATION / 1024.0;
}

void timerInterrupt () {
  bool Done = false;
  byte Hour = 0, Minute = 0, Second = 0;

  calc = true;
  startTime = millis();
  analogWrite(PWM_PIN, pwmValue);

  while (Done == false) {
    unsigned long secondsFromStart = (millis() - startTime) / 1000;
    unsigned long minutesFromStart = secondsFromStart / 60;

    Second = secondsFromStart % 60;
    Minute = minutesFromStart % 60;
    Hour = minutesFromStart / 60;

    float BAT_Voltage = measureBatteryVoltage();

    unsigned long Capacity = secondsFromStart * CURRENT[pwmValue/PWM_STEP] / 3600;

    display.firstPage();
    do {
      display.drawStr( 0, 15, "Time:");
      display.drawStr(60, 15, (String(Hour) + ":" + (Minute < 10 ? "0" : "") + String(Minute) + ":" + (Second < 10 ? "0" : "") + String(Second)).c_str());
      display.drawStr( 0, 30, "Current:");
      display.drawStr(60, 30, (String(CURRENT[pwmValue/PWM_STEP]) + " mA").c_str());
      display.drawStr( 0, 45, "Voltage:");
      display.drawStr(60, 45, (String(BAT_Voltage) + " V").c_str());
      display.drawStr( 0, 60, "Capacity:");
      display.drawStr(60, 60, (String(Capacity) + " mAh").c_str());
    } while (display.nextPage());

    downButton.read();

    if (BAT_Voltage < CUTOFF_BATTERY_VOLTAGE || downButton.pressedFor(1000)) {
      display.firstPage();
      do {
        display.drawStr( 0, 15, "Finished!");
        display.drawStr( 0, 30, "Total time:");
        display.drawStr(60, 30, (String(Hour) + ":" + (Minute < 10 ? "0" : "") + String(Minute) + ":" + (Second < 10 ? "0" : "") + String(Second)).c_str());
        display.drawStr( 0, 45, "Capacity:");
        display.drawStr(60, 45, (String(Capacity) + " mAh").c_str());
      } while (display.nextPage());
      pwmValue = 0;
      analogWrite(PWM_PIN, pwmValue);
      Done = true;
      do {
        // the flow ends here
      } while (true);
    }
  }
}
