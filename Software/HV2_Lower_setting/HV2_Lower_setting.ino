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

// [] 그룹에 묶어서 제어하는 다리 수 --> 즉 A그룹에서는 Left Leg 한 개, B 그룹에서는 Right Leg 한 개; 따라서 GROUP_SIZE = 1
const uint8_t GROUP_SIZE = 1;

// 그룹 A: Left 다리 채널 (골반, 힙, 무릎, 발목)
const uint8_t PELVIS_A[GROUP_SIZE] = {3};
const uint8_t HIP_A[GROUP_SIZE]    = {2};
const uint8_t KNEE_A[GROUP_SIZE]   = {1};
const uint8_t ANKLE_A[GROUP_SIZE]  = {0};

// 그룹 B: Right 다리 채널 (골반, 힙, 무릎, 발목)
const uint8_t PELVIS_B[GROUP_SIZE] = {7};
const uint8_t HIP_B[GROUP_SIZE]    = {6};
const uint8_t KNEE_B[GROUP_SIZE]   = {5};
const uint8_t ANKLE_B[GROUP_SIZE]  = {4};

// PWM 서보 각도 설정
void setAngle(uint8_t ch, float angle) {
  angle = constrain(angle, 0, 180);
  uint16_t pulse = map((int)angle, 0, 180, SERVOMIN, SERVOMAX);
  pwm.setPWM(ch, 0, pulse);
}

// 그룹 A에 각도 적용 (골반, 힙, 무릎, 발목)
void setGroupA(float pelvisAng, float hipAng, float kneeAng, float ankleAng) {
  setAngle(PELVIS_A[0], pelvisAng);
  setAngle(HIP_A[0],    hipAng);
  setAngle(KNEE_A[0],   kneeAng);
  setAngle(ANKLE_A[0],  ankleAng);
}

// 그룹 B에 각도 적용 (골반, 힙, 무릎, 발목)
void setGroupB(float pelvisAng, float hipAng, float kneeAng, float ankleAng) {
  setAngle(PELVIS_B[0], pelvisAng);
  setAngle(HIP_B[0],    hipAng);
  setAngle(KNEE_B[0],   kneeAng);
  setAngle(ANKLE_B[0],  ankleAng);
}

void setup() {
  // 시리얼 통신, I2C, PWM 드라이버 초기화
  Serial.begin(115200);
  delay(100);
  Wire.begin(SDA_PIN, SCL_PIN); 
  pwm.begin();
  pwm.setPWMFreq(SERVO_FREQ);
  delay(10);

  Serial.println("Control Ready");

  // 초기 포즈 (Humanoid standing)
  setGroupA(90, 90, 90, 90);  
  setGroupB(90, 90, 90, 90);  
}

void loop() {
  if (!Serial.available()) return;
  char cmd = Serial.read();
  switch (cmd) {

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

    /* ------------ 보행 -------------------- */
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
      Serial.println("Preset Z applied");
      break;

    /* --------------각도 초기화---------------------- */
    case '1': // --> 초기 각도
      setGroupA(90, 90, 90, 90);
      setGroupB(90, 90, 90, 90);
      Serial.println("Preset 1 applied");
      break;

    default:
      // 기타 입력 무시
      break;
  }
}