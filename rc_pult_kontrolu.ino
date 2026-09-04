#include <Wire.h>
#include <Servo.h>

// ====================
// RC PULT KONTROL KODU
// ====================
// Bu kod RC pult ilə dron idarə etmə üçündür
// Pult 4 kanalı əmr göndərəcəkdir

// ====================
// PIN TƏYİNATLARI
// ====================

// Joystick Analog Pinləri
#define CH1_POT A0    // Gas (Throttle) - Sağ Joystick Vertical
#define CH2_POT A1    // Sağa-Sola (Yaw) - Sol Joystick Horizontal
#define CH3_POT A2    // İrəli-Geri (Pitch) - Sol Joystick Vertical
#define CH4_POT A3    // Yuxarı-Aşağı (Roll) - Sağ Joystick Horizontal

// Transmitter PWM Output Pinləri (4 kanal)
#define TX_CH1_PIN 3  // Motor 1 sinyal göndər
#define TX_CH2_PIN 5  // Motor 2 sinyal göndər
#define TX_CH3_PIN 6  // Motor 3 sinyal göndər
#define TX_CH4_PIN 9  // Motor 4 sinyal göndər

// Aydınlatma LED-ləri
#define LED_POWER 13   // Qida LED
#define LED_TX 12      // Transmitter LED
#define LED_ARM 11     // ARM statusu LED

// Düymələr
#define ARM_BUTTON 2   // Dron ARM etmə düyməsi
#define DISARM_BUTTON 4 // Dron DISARM etmə düyməsi

// ====================
// DEĞİŞKƏNLƏR
// ====================

// Joystick dəyərləri (0-1023)
int ch1_raw = 512;  // Gas
int ch2_raw = 512;  // Sağa-Sola
int ch3_raw = 512;  // İrəli-Geri
int ch4_raw = 512;  // Yuxarı-Aşağı

// PWM dəyərləri (1000-2000 mikrosan)
int ch1_pwm = 1000; // Gas
int ch2_pwm = 1500; // Sağa-Sola (orta)
int ch3_pwm = 1500; // İrəli-Geri (orta)
int ch4_pwm = 1500; // Yuxarı-Aşağı (orta)

// Dron ARM statusu
volatile bool is_armed = false;

// Calibration dəyərləri
int ch1_min = 0, ch1_max = 1023;
int ch2_min = 0, ch2_max = 1023;
int ch3_min = 0, ch3_max = 1023;
int ch4_min = 0, ch4_max = 1023;

// Zaman
unsigned long last_read = 0;
unsigned long last_display = 0;

// ====================
// SERVO OBYEKTLƏRİ
// ====================

Servo servo1, servo2, servo3, servo4;

// ====================
// SETUP FUNKSİYASI
// ====================

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  // Pin konfigurasyonu
  pinMode(LED_POWER, OUTPUT);
  pinMode(LED_TX, OUTPUT);
  pinMode(LED_ARM, OUTPUT);
  pinMode(ARM_BUTTON, INPUT_PULLUP);
  pinMode(DISARM_BUTTON, INPUT_PULLUP);
  
  // LED-ləri testlə
  digitalWrite(LED_POWER, HIGH);
  delay(500);
  digitalWrite(LED_POWER, LOW);
  
  // Servo PWM outputlarını başlat
  servo1.attach(TX_CH1_PIN);
  servo2.attach(TX_CH2_PIN);
  servo3.attach(TX_CH3_PIN);
  servo4.attach(TX_CH4_PIN);
  
  // Default PWM dəyərləri
  servo1.writeMicroseconds(1000);
  servo2.writeMicroseconds(1500);
  servo3.writeMicroseconds(1500);
  servo4.writeMicroseconds(1500);
  
  // Interrupt-ləri quraş (düymələr)
  attachInterrupt(digitalPinToInterrupt(ARM_BUTTON), armDrone, FALLING);
  attachInterrupt(digitalPinToInterrupt(DISARM_BUTTON), disarmDrone, FALLING);
  
  Serial.println("===== RC PULT BAŞLANDI =====");
  Serial.println("Joystick kalibrasyonu...");
  delay(1000);
  
  // Joystick kalibrasyonu
  calibrateJoysticks();
  
  Serial.println("Kalibrasyon tamamlandı!");
  Serial.println("Pult hazır. Dron ARM etmə üçün ARM düyməsinə basın.");
  delay(2000);
}

// ====================
// CALIBRATION FUNKSİYASI
// ====================

void calibrateJoysticks() {
  Serial.println("Kalibrasyon başladı - Joystickləri tərpət edin...");
  
  for (int i = 0; i < 100; i++) {
    int val1 = analogRead(CH1_POT);
    int val2 = analogRead(CH2_POT);
    int val3 = analogRead(CH3_POT);
    int val4 = analogRead(CH4_POT);
    
    // Min/Max dəyərləri tapın
    if (val1 < ch1_min) ch1_min = val1;
    if (val1 > ch1_max) ch1_max = val1;
    
    if (val2 < ch2_min) ch2_min = val2;
    if (val2 > ch2_max) ch2_max = val2;
    
    if (val3 < ch3_min) ch3_min = val3;
    if (val3 > ch3_max) ch3_max = val3;
    
    if (val4 < ch4_min) ch4_min = val4;
    if (val4 > ch4_max) ch4_max = val4;
    
    delay(20);
  }
  
  Serial.print("CH1 (Gas): ");
  Serial.print(ch1_min);
  Serial.print(" - ");
  Serial.println(ch1_max);
  
  Serial.print("CH2 (Yaw): ");
  Serial.print(ch2_min);
  Serial.print(" - ");
  Serial.println(ch2_max);
  
  Serial.print("CH3 (Pitch): ");
  Serial.print(ch3_min);
  Serial.print(" - ");
  Serial.println(ch3_max);
  
  Serial.print("CH4 (Roll): ");
  Serial.print(ch4_min);
  Serial.print(" - ");
  Serial.println(ch4_max);
}

// ====================
// ARM / DISARM FUNKSİYALARI
// ====================

void armDrone() {
  static unsigned long last_arm_time = 0;
  if (millis() - last_arm_time > 500) {  // Debounce
    if (ch1_pwm < 1050) {  // Gas minimum olmalı
      is_armed = true;
      digitalWrite(LED_ARM, HIGH);
      Serial.println(">>> DRON ARMED <<<");
      last_arm_time = millis();
    } else {
      Serial.println("Dron ARM edilə bilmədi - Gas SIFIR olmalı!");
    }
  }
}

void disarmDrone() {
  static unsigned long last_disarm_time = 0;
  if (millis() - last_disarm_time > 500) {  // Debounce
    is_armed = false;
    digitalWrite(LED_ARM, LOW);
    
    // Motorları minimum hızda saxla
    servo1.writeMicroseconds(1000);
    servo2.writeMicroseconds(1000);
    servo3.writeMicroseconds(1000);
    servo4.writeMicroseconds(1000);
    
    Serial.println(">>> DRON DISARMED <<<");
    last_disarm_time = millis();
  }
}

// ====================
// JOYSTICK OXUMASı
// ====================

void readJoysticks() {
  // Analog dəyərləri oxu
  ch1_raw = analogRead(CH1_POT);
  ch2_raw = analogRead(CH2_POT);
  ch3_raw = analogRead(CH3_POT);
  ch4_raw = analogRead(CH4_POT);
  
  // Kalibrasiya edilmiş dəyərləri PWM-ə çevir
  ch1_pwm = map(ch1_raw, ch1_min, ch1_max, 1000, 2000);
  ch2_pwm = map(ch2_raw, ch2_min, ch2_max, 1000, 2000);
  ch3_pwm = map(ch3_raw, ch3_min, ch3_max, 1000, 2000);
  ch4_pwm = map(ch4_raw, ch4_min, ch4_max, 1000, 2000);
  
  // Hədləri tətbiq et
  ch1_pwm = constrain(ch1_pwm, 1000, 2000);
  ch2_pwm = constrain(ch2_pwm, 1000, 2000);
  ch3_pwm = constrain(ch3_pwm, 1000, 2000);
  ch4_pwm = constrain(ch4_pwm, 1000, 2000);
  
  // DISARM olsun, motor sinyal göndərmə
  if (!is_armed) {
    ch1_pwm = 1000;
  }
}

// ====================
// TRANSMITTER SİNYAL GÖNDƏR
// ====================

void sendSignalToDrone() {
  if (is_armed) {
    digitalWrite(LED_TX, HIGH);
    
    servo1.writeMicroseconds(ch1_pwm);
    servo2.writeMicroseconds(ch2_pwm);
    servo3.writeMicroseconds(ch3_pwm);
    servo4.writeMicroseconds(ch4_pwm);
    
    digitalWrite(LED_TX, LOW);
  } else {
    servo1.writeMicroseconds(1000);
    servo2.writeMicroseconds(1500);
    servo3.writeMicroseconds(1500);
    servo4.writeMicroseconds(1500);
  }
}

// ====================
// DEBUG MƏLUMAT ÇIXIŞI
// ====================

void printDebugInfo() {
  static unsigned long last_print = 0;
  if (millis() - last_print >= 500) {
    last_print = millis();
    
    Serial.print("Status: ");
    Serial.print(is_armed ? "ARMED" : "DISARMED");
    Serial.print(" | ");
    
    Serial.print("Gas:");
    Serial.print(ch1_pwm);
    Serial.print(" | Yaw:");
    Serial.print(ch2_pwm);
    Serial.print(" | Pitch:");
    Serial.print(ch3_pwm);
    Serial.print(" | Roll:");
    Serial.print(ch4_pwm);
    
    Serial.print(" | Raw: ");
    Serial.print(ch1_raw);
    Serial.print(",");
    Serial.print(ch2_raw);
    Serial.print(",");
    Serial.print(ch3_raw);
    Serial.print(",");
    Serial.println(ch4_raw);
  }
}

// ====================
// MAIN LOOP
// ====================

void loop() {
  // Joystick-ləri oxu
  readJoysticks();
  
  // Dronə siqnal göndər
  sendSignalToDrone();
  
  // Debug məlumatını çap et
  printDebugInfo();
  
  // Kiçik delay
  delay(20);  // 50Hz kontrol tezliyi
}
