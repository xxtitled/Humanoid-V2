#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <BluetoothSerial.h> // 블루투스 연결

BluetoothSerial SerialBT;

// ESP32 I2C 핀 정의
#define SDA_PIN    21
#define SCL_PIN    22

// PCA9685 16채널 서보 드라이버
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
#define SERVOMIN   150
#define SERVOMAX   600
#define SERVO_FREQ 50

/* ------------- LOWER ------------- */

// [] 그룹에 묶어서 제어하는 다리 수
// 즉 A에서는 Left Leg 한 개, B에서는 Right Leg 한 개 --> LEG_SIZE = 1
const uint8_t LOWER_SIZE = 1;

// A: Left Leg (골반, 힙, 무릎, 발목)
const uint8_t PELVIS_A[LOWER_SIZE] = {3};
const uint8_t HIP_A[LOWER_SIZE] = {2};
const uint8_t KNEE_A[LOWER_SIZE] = {1};
const uint8_t ANKLE_A[LOWER_SIZE] = {0};

// B: Right Leg (골반, 힙, 무릎, 발목)
const uint8_t PELVIS_B[LOWER_SIZE] = {7};
const uint8_t HIP_B[LOWER_SIZE] = {6};
const uint8_t KNEE_B[LOWER_SIZE] = {5};
const uint8_t ANKLE_B[LOWER_SIZE] = {4};

/* ------------- UPPER ------------- */

// [] 그룹에 묶어서 제어하는 팔 or 허리 수 
const uint8_t UPPER_SIZE = 1;

// C: Left Arm (가슴, 어깨, 팔꿈치)
const uint8_t CHEST_C[UPPER_SIZE] = {10};
const uint8_t SHOULDER_C[UPPER_SIZE] = {9};
const uint8_t ELBOW_C[UPPER_SIZE] = {8};

// D: Right Arm (가슴, 어깨, 팔꿈치)
const uint8_t CHEST_D[UPPER_SIZE] = {13};
const uint8_t SHOULDER_D[UPPER_SIZE] = {12};
const uint8_t ELBOW_D[UPPER_SIZE] = {11};

// E: WAIST (허리)
const uint8_t WAIST_E[UPPER_SIZE] = {14};

// PWM 서보 각도 설정
void setAngle(uint8_t ch, float angle) {
  angle = constrain(angle, 0, 180);
  uint16_t pulse = map((int)angle, 0, 180, SERVOMIN, SERVOMAX);
  pwm.setPWM(ch, 0, pulse);
}

// ============= LOWER ============= 
void setGroupA(float pelvis, float hip, float knee, float ankle) {    // L
  setAngle(PELVIS_A[0], pelvis);
  setAngle(HIP_A[0], hip);
  setAngle(KNEE_A[0], knee);
  setAngle(ANKLE_A[0], ankle);
}
void setGroupB(float pelvis, float hip, float knee, float ankle) {    // R
  setAngle(PELVIS_B[0], pelvis);
  setAngle(HIP_B[0], hip);
  setAngle(KNEE_B[0], knee);
  setAngle(ANKLE_B[0], ankle);
}

// ============= UPPER ============= 
void setGroupC(float chest, float shoulder, float elbow) {    // L
  setAngle(CHEST_C[0], chest);
  setAngle(SHOULDER_C[0], shoulder);
  setAngle(ELBOW_C[0], elbow);
}
void setGroupD(float chest, float shoulder, float elbow) {    // R
  setAngle(CHEST_D[0], chest);
  setAngle(SHOULDER_D[0], shoulder);
  setAngle(ELBOW_D[0], elbow);
}
void setGroupE(float waist) {   // Core
  setAngle(WAIST_E[0], waist);
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("VIN_ESP32");
  Serial.println("Bluetooth Started! Device name: VIN_ESP32");
  delay(100);

  Wire.begin(SDA_PIN, SCL_PIN);
  pwm.begin();
  pwm.setPWMFreq(SERVO_FREQ);
  delay(10);

  Serial.println("Humanoid Control Ready");

  // 초기 포즈: 다리 & 팔 모두 중립(90°)
  setGroupA(90, 90, 90, 90);
  setGroupB(90, 90, 90, 90);
  setGroupC(90, 90, 90);
  setGroupD(90, 90, 90);
  setGroupE(90);
}

void stop() { // 멈추기
  setGroupA(90, 90, 90, 90);
  setGroupB(90, 90, 90, 90);
  setGroupC(90, 100, 90);
  setGroupD(90, 80, 90);
  setGroupE(90);
  Serial.println("Stop");
}

void go() { // 전진
  setGroupA(90, 30, 120, 90);
  setGroupB(90, 105, 60, 90);       // R
  setGroupC(105, 100, 105);         // L_ARM
  setGroupD(105, 80, 90);         
  setGroupE(95);
  delay(1000);
  setGroupA(90, 75, 110, 90);      // L Knee 앞으로
  setGroupB(90, 150, 60, 90);      // --> Hip & Knee 조금 더 뒤로 셋백
  setGroupC(75, 100, 90);          
  setGroupD(75, 80, 75);          // R_ARM
  setGroupE(85);
  Serial.println("Go-ahead"); // 앞으로 가기
}

void standup() { // 일어나기
  setGroupA(90, 45, 135, 90);     // Left Leg
  setGroupB(90, 135, 45, 90);     // Right Leg
  setGroupC(90, 100, 90);
  setGroupD(90, 80, 90);
  setGroupE(90);
  Serial.println("Full stand"); // 완전히 일어나기
  Serial.println("Stand-up");
}

void sitdown() { // 앉기
  setGroupA(90, 150, 30, 90);
  setGroupB(90, 30, 150, 90);
  setGroupC(90, 100, 135);  // L
  setGroupD(90, 80, 45);   // R
  setGroupE(90); 
  Serial.println("Sit-down"); //  완전히 앉기 & 팔 굽히기
}

void left() { // 왼손 스트레이트
  setGroupA(90, 90, 90, 90);
  setGroupB(90, 90, 90, 90);
  setGroupC(180, 90, 80);  // L
  setGroupD(45, 90, 5);   // R --> 가드
  setGroupE(115);
  Serial.println("Straight-left");
}

void right() { // 오른손 스트레이트
  setGroupA(90, 90, 90, 90);
  setGroupB(90, 90, 90, 90);
  setGroupC(135, 90, 175);  // L --> 가드
  setGroupD(0, 90, 100);    // R
  setGroupE(65);   //
  Serial.println("Straight-right");
}

void guard() { // 가드
  setGroupA(90, 90, 90, 90);
  setGroupB(90, 90, 90, 90);
  setGroupC(145, 70, 175);  // L
  setGroupD(35, 110, 5);   // R
  setGroupE(90);   //
  Serial.println("Guard");
}


void loop() {
  if (SerialBT.available()) {
    String cmd = SerialBT.readStringUntil('\n');
    cmd.trim();
    Serial.print("CMD> "); Serial.println(cmd);

    if (cmd.equalsIgnoreCase("stop") || cmd.equals("멈춰") {
      stop();
    }
    else if (cmd.equalsIgnoreCase("go") || cmd.equals("앞으로가") || cmd.equals("전진")) {
      go();
    }
    else if (cmd.equalsIgnoreCase("standup") || cmd.equalsIgnoreCase("일어나") {
      standup();
    }
    else if (cmd.equalsIgnoreCase("sitdown") || cmd.equals("앉아")) {
      sitdown();
    }
    else if (cmd.equalsIgnoreCase("left") || cmd.equals("왼손")) || cmd.equals("투")) {
      left();
    }
    else if (cmd.equalsIgnoreCase("right") || cmd.equals("오른손")) || cmd.equals("원")){
      right();
    }
    else if (cmd.equalsIgnoreCase("guard") || cmd.equals("가드")) {
      guard();
    }
    else {
      Serial.println("Unknown command");
    }
  }
  delay(20);
}