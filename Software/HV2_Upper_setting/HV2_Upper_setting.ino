#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

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

// [] 그룹에 묶어서 제어하는 팔 수 or 허리 
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

void loop() {
  if (!Serial.available()) return;
  char cmd = Serial.read();
  switch (cmd) {

    /* ------------- LOWER ------------- */

    case 'q': case 'Q': // --> 완전히 일어나기
      setGroupA(90, 45, 135, 90);     // Left Leg
      setGroupB(90, 135, 45, 90);     // Right Leg
      Serial.println("Preset Q applied");
      break;

    case 'w': case 'W': // --> 일어나기
      setGroupA(90, 60, 120, 90);
      setGroupB(90, 120, 60, 90);
      Serial.println("Preset W applied");
      break;

    case 'e': case 'E': // --> 완전히 앉기
      setGroupA(90, 150, 30, 90);
      setGroupB(90, 30, 150, 90);
      Serial.println("Preset E applied");
      break;

    case 'r': case 'R': // --> 앉기
      setGroupA(90, 135, 45, 90);
      setGroupB(90, 45, 135, 90);
      Serial.println("Preset R applied");
      break;

    /* ------------ 보행 ------------ */
    case 'a': case 'A': // --> Left up
      setGroupA(85, 70, 110, 95);
      setGroupB(95, 90, 90, 95); // R
      Serial.println("Preset A applied");
      break;

    case 's': case 'S': // -->  Right up
      setGroupA(85, 90, 90, 85); // L
      setGroupB(95, 110, 70, 85);
      Serial.println("Preset S applied");
      break;
    /* ------------------------------------ */

    case 'z': case 'Z': // --> 다리 찢기
      setGroupA(50, 90, 90, 120);
      setGroupB(130, 90, 90, 60);
      setGroupC(90, 135, 90);  // L
      setGroupD(90, 45, 90);    // R
      Serial.println("Preset Z applied");
      break;

    /* ------------- UPPER ------------- */

    case 'd': case 'D': // --> 팔 X축과 평행하게 펴기
      setGroupC(180, 90, 80);  // L
      setGroupD(0, 90, 100);   // R
      setGroupE(90);
      Serial.println("Preset D applied");
      break;

    case 'f': case 'F': // --> 팔 Y축과 평행하게 펴기
      setGroupC(90, 180, 90);  // L
      setGroupD(90, 0, 90);    // R
      setGroupE(90);
      Serial.println("Preset  applied");
      break;

    case 'c': case 'C': // --> 왼손 잽
      setGroupC(180, 90, 80);  // L
      setGroupD(90, 90, 90);   // R
      setGroupE(90);
      Serial.println("Preset D applied");
      break;

    case 'v': case 'V': // --> 오른손 잽
      setGroupC(90, 90, 90);  // L
      setGroupD(0, 90, 100);   // R
      setGroupE(90);   //
      Serial.println("Preset  applied");
      break;

    /* -------------- 각도 초기화 -------------- */

    case '1': // --> 초기 각도
      setGroupA(90, 90, 90, 90);
      setGroupB(90, 90, 90, 90);
      setGroupC(90, 90, 90);
      setGroupD(90, 90, 90);
      setGroupE(90);
      Serial.println("Preset 1 applied");
      break;

    default:
      // 기타 입력 무시
      break;
  }
}