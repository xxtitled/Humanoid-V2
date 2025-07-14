import cv2
import mediapipe as mp
import serial
import time

# ─── 1) ESP32 시리얼 초기화 ────────────────────────────────
esp32 = serial.Serial('COM6', 115200, timeout=1) # 연결 시 포트 확인 필수 
time.sleep(2)  # 연결 안정화

# ─── 2) MediaPipe Hands 초기화 ────────────────────────────
mp_hands = mp.solutions.hands
hands = mp_hands.Hands(
    static_image_mode=False,
    max_num_hands=1,
    min_detection_confidence=0.5,
    min_tracking_confidence=0.5
)
mp_draw = mp.solutions.drawing_utils

# ─── 3) 손가락 끝(팁) 및 관절 인덱스 ───────────────────────
FINGER_TIPS = [4, 8, 12, 16, 20]  # thumb, index, middle, ring, pinky

# ─── 4) VideoCapture 초기화 ─────────────────────────────────
cap = cv2.VideoCapture(0)

last_cmd = None

def count_fingers(frame, results):
    """
    MediaPipe 결과에서 손가락이 펴진 개수 카운팅 후 반환.
    1) 엄지는 x 좌표 비교, 나머지는 y 좌표 비교로 판별
    """
    if not results.multi_hand_landmarks:
        return 0

    lm = results.multi_hand_landmarks[0]
    h, w, _ = frame.shape

    fingers = []
    # 엄지: tip.x > ip.x 면 펼침
    tip_x = int(lm.landmark[FINGER_TIPS[0]].x * w)
    ip_x  = int(lm.landmark[FINGER_TIPS[0] - 1].x * w)
    fingers.append(1 if tip_x > ip_x else 0)

    # 검지~소지: tip.y < pip.y 면 펼침
    for tip_id in FINGER_TIPS[1:]:
        tip_y = int(lm.landmark[tip_id].y * h)
        pip_y = int(lm.landmark[tip_id - 2].y * h)
        fingers.append(1 if tip_y < pip_y else 0)

    return sum(fingers)

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = cv2.flip(frame, 1)
    rgb   = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    results = hands.process(rgb)

    # 손 랜드마크 그리기
    if results.multi_hand_landmarks:
        mp_draw.draw_landmarks(frame, results.multi_hand_landmarks[0], mp_hands.HAND_CONNECTIONS)

    # 손가락 개수 계산
    cnt = count_fingers(frame, results)

    # 디버깅용: 화면에 카운트 표시
    cv2.rectangle(frame, (10, 10), (170, 80), (0, 0, 255), cv2.FILLED)
    cv2.putText(frame, f'Fingers: {cnt}', (20, 60),
                cv2.FONT_HERSHEY_SIMPLEX, 1.5, (255, 255, 255), 3)

    # ─── ESP32로 명령 전송──────────────────────────
    # 0 --> b'0', 1 --> b'1', … 5 --> b'5'
    # 카운트 1 --> Stop / 카운트 2 --> 전진 / 카운트 3 --> 일어나기 / 카운트 4 --> 앉기 / 카운트 5 --> 원 투

    cmd = f'{cnt}'.encode()
    if cmd != last_cmd:
        esp32.write(cmd)
        print(f"Sent to ESP32: {cnt}")
        last_cmd = cmd

    cv2.imshow("Finger Counter", frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
esp32.close()
