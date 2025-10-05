// 在 Arduino Uno R3、Arduino Nano、ESP32-S3 N16R8、NodeMCU 1.0 (ESP-12E) 上测试

// 标准音体（简谱定义）
unsigned short NOTE_L5 = 311;   // Eb4
unsigned short NOTE_LS5 = 330;  // E4
unsigned short NOTE_L6 = 349;   // F4
unsigned short NOTE_L7 = 392;   // G4
unsigned short NOTE_1 = 415;    // Ab4
unsigned short NOTE_2 = 466;    // Bb4
unsigned short NOTE_3 = 523;    // C5
unsigned short NOTE_5 = 622;    // Eb5
unsigned short NOTE_6 = 698;    // F5
unsigned short NOTE_7 = 784;    // G5
unsigned short NOTE_H1 = 831;   // Ab5
const unsigned short REST = 0;  //休止符，设为常量

// 全局变量
const unsigned short buzzerPin = 13;  //Pin设为常量
unsigned short tempo = 140;           //速度不设为常量，因为考虑变速

float wholenote = (60000 * 4) / tempo;  // 全音符的时程由音乐速度计算得到
float duration = 0;                     // 初始化某个音符的时程

void playNote(unsigned short noteMelody, float noteDurationValue) {  //播放函数
  duration = 0;                                                      // 某个音符的时程

  if (noteDurationValue > 0) {  // 几分音符就传入几
    duration = wholenote / noteDurationValue;
  } else if (noteDurationValue < 0) {  // 用负号表示附点音符
    duration = wholenote / abs(noteDurationValue);
    duration *= 1.5;  // 处理附点
  }

  tone(buzzerPin, noteMelody, duration * 0.9);  // 播放的时程为0.9，略略停顿，不然音乐会粘在一起

  delay(duration);  // 但是占据的时程是得给全的

  noTone(buzzerPin);  // 记得闭嘴
}

void chapterAPart(unsigned short times) {
  for (unsigned short i = 1; i <= times; i++) {
    playNote(NOTE_L6, 8);
    playNote(NOTE_L6, 16);
    playNote(NOTE_L6, 8);
    playNote(NOTE_L6, 16);
    playNote(NOTE_L5, 16);
    playNote(NOTE_L6, 16);
  }
}

void chapterA() {
  chapterAPart(3);
  playNote(NOTE_L6, 8);
  playNote(NOTE_L6, 16);
  playNote(NOTE_1, 16);
  playNote(NOTE_2, 8);
  playNote(NOTE_1, 16);
  playNote(NOTE_2, 16);
  chapterAPart(3);
  playNote(NOTE_2, 8);
  playNote(NOTE_1, 16);
  playNote(NOTE_2, 16);
  playNote(NOTE_1, 8);
  playNote(NOTE_L6, 16);
  playNote(NOTE_1, 16);
}

void chapterBPart() {
  playNote(NOTE_L6, 8);
  playNote(NOTE_L7, 8);
  playNote(NOTE_1, 8);
  playNote(NOTE_2, 8);
  playNote(NOTE_3, -8);
  playNote(REST, 16);
  playNote(NOTE_6, 8);
  playNote(NOTE_5, 8);
  playNote(NOTE_3, -8);
  playNote(REST, 16);
  playNote(NOTE_L6, -8);
  playNote(REST, 16);
  playNote(NOTE_3, 8);
  playNote(NOTE_2, 8);
  playNote(NOTE_1, 8);
  playNote(NOTE_L7, 8);
  playNote(NOTE_L6, 8);
  playNote(NOTE_L7, 8);
  playNote(NOTE_1, 8);
  playNote(NOTE_2, 8);
  playNote(NOTE_3, -8);
  playNote(REST, 16);
  playNote(NOTE_2, 8);
  playNote(NOTE_1, 8);
}

void chapterB() {
  chapterBPart();
  playNote(NOTE_L7, 8);
  playNote(NOTE_L6, 8);
  playNote(NOTE_L7, 8);
  playNote(NOTE_1, 8);
  playNote(NOTE_L7, 8);
  playNote(NOTE_L6, 8);
  playNote(NOTE_LS5, 8);
  playNote(NOTE_L7, 8);
  chapterBPart();
  playNote(NOTE_L7, 8);
  playNote(REST, 8);
  playNote(NOTE_1, 8);
  playNote(REST, 8);
  playNote(NOTE_2, 8);
  playNote(REST, 8);
  playNote(NOTE_3, 8);
  playNote(REST, 8);
}

void chapterCMain(unsigned short times) {
  for (unsigned short i = 1; i <= times; i++) {
    playNote(NOTE_5, 8);
    playNote(NOTE_6, 8);
    playNote(NOTE_3, 8);
    playNote(NOTE_2, 8);
    playNote(NOTE_3, 8);
    playNote(REST, 8);
    playNote(NOTE_2, 8);
    playNote(NOTE_3, 8);
  }
}

void chapterCPart() {
  for (int i = 1; i <= 3; i++) {
    chapterCMain(2);

    playNote(NOTE_2, 8);
    playNote(NOTE_1, 8);
    playNote(NOTE_L7, 8);
    playNote(NOTE_L5, 8);
    playNote(NOTE_L6, 8);
    playNote(REST, 8);
    playNote(NOTE_L5, 8);
    playNote(NOTE_L6, 8);

    playNote(NOTE_L7, 8);
    playNote(NOTE_1, 8);
    playNote(NOTE_2, 8);
    playNote(NOTE_3, 8);
    playNote(NOTE_L6, 8);
    playNote(REST, 8);
    playNote(NOTE_3, 8);
    playNote(NOTE_5, 8);
  }
}

void chapterC() {
  chapterCPart();

  chapterCMain(1);

  playNote(NOTE_5, 8);
  playNote(NOTE_6, 8);
  playNote(NOTE_3, 8);
  playNote(NOTE_2, 8);
  playNote(NOTE_3, 8);
  playNote(REST, 8);
  playNote(NOTE_6, 8);
  playNote(NOTE_7, 8);

  playNote(NOTE_H1, 8);
  playNote(NOTE_7, 8);
  playNote(NOTE_6, 8);
  playNote(NOTE_5, 8);
  playNote(NOTE_3, 8);
  playNote(REST, 8);
  playNote(NOTE_2, 8);
  playNote(NOTE_3, 8);

  playNote(NOTE_2, 8);
  playNote(NOTE_1, 8);
  playNote(NOTE_L7, 8);
  playNote(NOTE_L5, 8);
}

void setup() {
  pinMode(48, OUTPUT);  // 给 ESP32-S3关灯，太亮了
  digitalWrite(48, LOW);
}

void loop() {
  // 转回去，不然再次播放就乱了
  NOTE_L5 = 311;   // Eb4
  NOTE_LS5 = 330;  // E4
  NOTE_L6 = 349;   // F4
  NOTE_L7 = 392;   // G4
  NOTE_1 = 415;    // Ab4
  NOTE_2 = 466;    // Bb4
  NOTE_3 = 523;    // C5
  NOTE_5 = 622;    // Eb5
  NOTE_6 = 698;    // F5
  NOTE_7 = 784;    // G5
  NOTE_H1 = 831;   // Ab5
  chapterA();
  chapterA();
  chapterB();
  chapterB();
  chapterC();
  playNote(NOTE_2, 4);
  playNote(NOTE_1, 4);
  chapterA();
  chapterA();
  chapterB();
  chapterB();
  chapterC();
  playNote(NOTE_L6, 2);
  // 转调
  NOTE_L5 = 330;   // E4
  NOTE_LS5 = 349;  // F4
  NOTE_L6 = 370;   // F#4
  NOTE_L7 = 415;   // Ab4
  NOTE_1 = 440;    // A4
  NOTE_2 = 494;    // B4
  NOTE_3 = 554;    // C#5
  NOTE_5 = 659;    // E5
  NOTE_6 = 740;    // F#5
  NOTE_7 = 831;    // Ab5
  NOTE_H1 = 880;   // A5
  chapterC();
  playNote(NOTE_L6, 2);
  chapterA();
  chapterA();
  playNote(NOTE_L6, 0.5);
  delay(2000);
}
