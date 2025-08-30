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
      setGroupA(90, 45, 135, 90);     // L Leg //pelvis, hip, knee, ankle
      setGroupB(90, 135, 45, 90);     // R Leg
      setGroupC(90, 100, 135);        // L Hand // chest, shoulder, elbow
      setGroupD(90, 80, 45);          // R Hand
      setGroupE(90);                  // Waist
      Serial.println("Q: Stand Up");
      break;

    case 'w': case 'W': // --> 일어나기
      setGroupA(90, 60, 120, 90);
      setGroupB(90, 120, 60, 90);
      setGroupC(90, 100, 135);       // L
      setGroupD(90, 80, 45);         // R
      setGroupE(90);
      Serial.println("W: Up");
      break;

    case 'e': case 'E': // --> 완전히 앉기 & 팔 굽히기
      setGroupA(90, 150, 30, 90);
      setGroupB(90, 30, 150, 90);
      setGroupC(90, 100, 135);  // L
      setGroupD(90, 80, 45);   // R
      setGroupE(90);
      Serial.println("E: Sit Down");
      break;

    case 'r': case 'R': // --> 힙 접고, 무릎 펴기, 팔 하프 굽히기
      setGroupA(90, 150, 90, 90);
      setGroupB(90, 30, 90, 90);
      setGroupC(90, 100, 120);  // L
      setGroupD(90, 80, 60);   // R
      setGroupE(90);
      Serial.println("R: Squat");
      break;

    /* ------------ 보행 ------------ */
    
    case 'a': case 'A': // --> 전진
      setGroupA(90, 60, 120, 90);
      setGroupB(90, 120, 60, 90);         
      delay(500); // 초기화
      setGroupA(90, 30, 120, 90);       // 오른발 앞으로
      setGroupB(90, 105, 60, 90);       // R
      setGroupC(105, 100, 105);         // L_ARM
      setGroupD(105, 80, 90);         
      setGroupE(95);
      delay(500);
      setGroupA(90, 75, 110, 90);      // 왼발 앞으로
      setGroupB(90, 150, 60, 90);      // --> Hip & Knee 조금 더 뒤로 셋백
      setGroupC(75, 100, 90);          
      setGroupD(75, 80, 75);          // R_ARM
      setGroupE(85);
      delay(500);
      setGroupA(90, 30, 120, 90);       // 오른발 앞으로
      setGroupB(90, 105, 60, 90);       // R
      setGroupC(105, 100, 105);         // L_ARM
      setGroupD(105, 80, 90);         
      setGroupE(95);
      delay(500);
      setGroupA(90, 75, 110, 90);      // 왼발 앞으로
      setGroupB(90, 150, 60, 90);      // --> Hip & Knee 조금 더 뒤로 셋백
      setGroupC(75, 100, 90);          
      setGroupD(75, 80, 75);          // R_ARM
      setGroupE(85);
      delay(500);
      setGroupA(90, 60, 120, 90);
      setGroupB(90, 120, 60, 90);
      setGroupE(90);      
      delay(500); // 초기화
      setGroupA(90, 30, 120, 90);       // 오른발 앞으로
      setGroupB(90, 105, 60, 90);       // R
      setGroupC(105, 100, 105);         // L_ARM
      setGroupD(105, 80, 90);         
      setGroupE(95);
      delay(500);
      setGroupA(90, 75, 110, 90);      // 왼발 앞으로
      setGroupB(90, 150, 60, 90);      // --> Hip & Knee 조금 더 뒤로 셋백
      setGroupC(75, 100, 90);          
      setGroupD(75, 80, 75);          // R_ARM
      setGroupE(85);
      delay(500);
      setGroupA(90, 30, 120, 90);       // 오른발 앞으로
      setGroupB(90, 105, 60, 90);       // R
      setGroupC(105, 100, 105);         // L_ARM
      setGroupD(105, 80, 90);         
      setGroupE(95);
      delay(500);
      setGroupA(90, 75, 110, 90);      // 왼발 앞으로
      setGroupB(90, 150, 60, 90);      // --> Hip & Knee 조금 더 뒤로 셋백
      setGroupC(75, 100, 90);          
      setGroupD(75, 80, 75);          // R_ARM
      setGroupE(85);
      delay(400); // 초기 자세로 회귀
      setGroupA(90, 90, 90, 90);
      setGroupB(90, 90, 90, 90);
      setGroupC(90, 100, 90); // 
      setGroupD(90, 80, 90); // 
      setGroupE(90);
      Serial.println("A: Go Ahead");
      break;


    case '2'://  오른발 앞으로 = Right up --> # 발바닥 높이 차이가 중요. 높이 차에 따라 로봇이 회전. 허리 회전에 신경써야 한다. 허리 회전과 팔 회전에 따라 로봇 전체가 회전
      setGroupA(90, 30, 120, 90);
      setGroupB(90, 105, 60, 90);       // R
      setGroupC(105, 100, 105);         // L_ARM
      setGroupD(105, 80, 90);         
      setGroupE(95);
      Serial.println("Preset 2 applied");
      break;

    case '3': //  왼발 앞으로 = -->  Left up
      setGroupA(90, 75, 110, 90);      // L Knee 앞으로
      setGroupB(90, 150, 60, 90);      // --> Hip & Knee 조금 더 뒤로 셋백
      setGroupC(75, 100, 90);          
      setGroupD(75, 80, 75);          // R_ARM
      setGroupE(85);
      Serial.println("Preset 3 applied");
      break;


    //=============== More dynamic =================
    case '4'://  --> Right up --> 
      setGroupA(90, 30, 60, 90);
      setGroupB(90, 70, 80, 90);       // R
      setGroupC(105, 100, 105);        // L_ARM
      setGroupD(105, 80, 90);         
      setGroupE(95);
      Serial.println("Preset 4 applied");
      break;

    case '5': //  -->  Left up
      setGroupA(90, 110, 110, 90);      // L Knee 앞으로
      setGroupB(90, 150, 120, 90);      // 
      setGroupC(75, 100, 90);          
      setGroupD(75, 80, 75);           // R_ARM
      setGroupE(85);
      Serial.println("Preset 5 applied");
      break;

    case '0': // --> 보행 각도 세팅
      setGroupA(90, 55, 125, 90);
      setGroupB(90, 125, 55, 90);
      setGroupC(90, 100, 90);
      setGroupD(90, 80, 90);
      setGroupE(90);
      Serial.println("Preset 0 applied");
      break;

    /* ------------------------------------ */

    //========= 물체 잡기 ========= 

    case 'z': case 'Z': // --> 앉고 물병 잡기
      setGroupA(90, 150, 30, 90); // 앉기
      setGroupB(90, 30, 150, 90);
      delay(250);
      setGroupC(140, 70, 160);  // L 가드 자세
      setGroupD(40, 110, 20);   // R
      delay(400);
      setGroupC(140, 50, 90);   // L 물병 잡기
      setGroupD(40, 130, 90);   // R
      setGroupE(90);
      Serial.println("Z: ");
      break;

     case 'x': case 'X': // --> 물병 잡은 상태로 일어서기
      setGroupA(90, 90, 90, 90);
      setGroupB(90, 90, 90, 90);
      setGroupC(140, 50, 90);   // L
      setGroupD(40, 130, 90);   // R
      setGroupE(90);
      Serial.println("X: ");
      break;
    

    // ====== Y축 이동 ======

    case 't': case 'T': // --> Y축 왼쪽 이동 ====> Right Leg의 움직임이 중요 
      setGroupB(120, 90, 90, 80); // --> 골반; 오른쪽 다리 몸 밖으로 30도 이동 / 발목; 왼쪽으로  10도 --80
      setGroupA(95, 90, 90, 110); // --> 골반; 왼쪽 다리 몸 중심으로 5도 이동 / 발목; 왼쪽으로 20 -- 110
      setGroupC(90, 115, 85);  // L
      setGroupD(90, 65, 95);   // R
      setGroupE(90);
      delay(1000);
      setGroupA(90, 90, 90, 90);
      setGroupB(90, 90, 90, 90);
      setGroupC(90, 100, 90); // 
      setGroupD(90, 80, 90); // 
      setGroupE(90);
      Serial.println("T: Left side step");
      break;

    case 'y': case 'Y': // --> Y축 오른쪽 이동
      setGroupA(60, 90, 90, 100); // --> 골반; 왼쪽 다리 몸 밖으로 30도 이동 / 발목; 오른쪽으로  10도
      setGroupB(85, 90, 90, 70);  // --> 골반; 오른쪽 다리 몸 중심으로 5도 이동 / 발목; 오른쪽으로 20도
      setGroupC(90, 115, 85);  // L
      setGroupD(90, 65, 95);   // R
      setGroupE(90);
      delay(1000);
      setGroupA(90, 90, 90, 90);
      setGroupB(90, 90, 90, 90);
      setGroupC(90, 100, 90); // 
      setGroupD(90, 80, 90); // 
      setGroupE(90);
      Serial.println("Y: right side step");
      break;


    /* ------------- UPPER ------------- */

    case 'd': case 'D': // --> 팔 X축과 평행하게 펴기
      setGroupC(180, 90, 80);  // L
      setGroupD(0, 90, 100);   // R
      setGroupE(90);
      Serial.println("X: arm extension along the X-axis");
      break;

    case 'f': case 'F': // --> 팔 Y축과 평행하게 펴기
      setGroupC(90, 180, 90);  // L
      setGroupD(90, 0, 90);    // R
      setGroupE(90);
      Serial.println("F: arm extension along the Y-axis");
      break;

    case 'c': case 'C': // --> 왼손 스트레이트
      setGroupC(180, 90, 80);  // L
      setGroupD(45, 90, 5);   // R --> 가드
      setGroupE(115);
      Serial.println("C: Straight Left");
      break;

    case 'v': case 'V': // --> 오른손 스트레이트
      setGroupC(135, 90, 175);  // L --> 가드
      setGroupD(0, 90, 100);    // R
      setGroupE(65);   //
      Serial.println("V: Straight Right");
      break;

    case 'b': case 'B': // --> 가드
      setGroupC(145, 70, 175);  // L
      setGroupD(35, 110, 5);   // R
      setGroupE(90);   //
      Serial.println("B: Guard");
      break;  

    case 'n': case 'N': // --> 푸시업
      setGroupA(90, 65, 105, 90); // --> 다리 약간 펴기
      setGroupB(90, 105, 65, 90);
      setGroupC(165, 95, 45);  // L
      setGroupD(15, 85, 135);    // R
      setGroupE(90);
      delay(1000);
      setGroupC(90, 95, 140);  // --> Down
      setGroupD(90, 85, 40); 
      delay(1000);
      setGroupC(165, 95, 45);   // --> Up
      setGroupD(15, 85, 135); 
      Serial.println("N: Push-Up");
      break;

    /* -------------- 각도 초기화 -------------- */

    case '1': // --> 초기 각도
      setGroupA(90, 90, 90, 90);
      setGroupB(90, 90, 90, 90);
      setGroupC(90, 100, 90); // 
      setGroupD(90, 80, 90); // 
      setGroupE(90);
      Serial.println("1: Reset");
      break;

    default:
      // 기타 입력 무시
      break;
  }
}