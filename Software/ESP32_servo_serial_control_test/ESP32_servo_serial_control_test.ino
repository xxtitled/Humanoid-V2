#include <ESP32Servo.h>  // ESP32용 Servo 라이브러리

Servo servo1;

const int motorPin = 2;  // ESP32 GPIO18에 연결
int angle1 = 0;          // 초기 각도

void setup() {
  // 50Hz(20ms) 서보 표준 주기 설정
  servo1.setPeriodHertz(50);
  // attach(pin, minPulse, maxPulse) 
  servo1.attach(motorPin, 500, 2500);

  // 초기 위치
  servo1.write(angle1);

  Serial.begin(115200);
  delay(100);
  Serial.println("Single Servo Control");
  Serial.println("w: +30°   s: -30°");
  Serial.println("Enter w/s:");
}

void loop() {
  if (Serial.available() > 0) {
    char input = Serial.read();

    if (input == 'w') {
      Serial.print("+30 -> ");
      for (int i = 0; i < 30; i++) {
        angle1 = angle1 + 1;
        if (angle1 > 180) angle1 = 180;
        servo1.write(angle1);
        delay(10);
      }
      Serial.println(angle1);

    } else if (input == 's') {
      Serial.print("-30 -> ");
      for (int i = 0; i < 30; i++) {
        angle1 = angle1 - 1;
        if (angle1 < 0) angle1 = 0;
        servo1.write(angle1);
        delay(10);
      }
      Serial.println(angle1);

    } else if (input != '\n' && input != '\r') {
      Serial.print("Unknown cmd '");
      Serial.print(input);
      Serial.println("'. Use w/s.");
    }

    Serial.println("Enter w/s:");
  }
}
