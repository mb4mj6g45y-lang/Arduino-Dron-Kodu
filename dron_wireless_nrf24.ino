#include <Wire.h>
#include <MPU6050.h>
#include <Servo.h>
#include <DHT.h>
#include <SPI.h>
#include <RF24.h>

// ====================
// DRON WIRELESS QƏBULEDICISI
// ====================
// Bu kod NRF24L01 vasitəsilə pultdan siqnal qəbul edir

// ====================
// NRF24 PIN KONFIGURASYONU
// ====================

#define CE_PIN 10     // Chip Enable
#define CSN_PIN 53    // Chip Select (Arduino Mega)

// ====================
// RF24 OBYEKTİ
// ====================

RF24 radio(CE_PIN, CSN_PIN);
const byte address[6] = "00001";

// ====================
// DATA STRUKTURASı (Pultu ilə eyni olmalı)
// ====================

struct ControlData {
  int throttle;    // Gas (1000-2000)
  int yaw;         // Sağa-Sola (1000-2000)
  int pitch;       // İrəli-Geri (1000-2000)
  int roll;        // Yuxarı-Aşağı (1000-2000)
  bool armed;      // ARM statusu
};

ControlData controlData = {1000, 1500, 1500, 1500, false};

// ====================
// MOTOR PIN KONFIGURASYONU
// ====================

#define MOTOR1_PIN 3   // Front-Left
#define MOTOR2_PIN 5   // Front-Right
#define MOTOR3_PIN 6   // Back-Left
#define MOTOR4_PIN 9   // Back-Right

// ====================
// SENSOR PIN KONFIGURASYONU
// ====================

#define DHTPIN 2
#define DHTTYPE DHT22
#define ULTRASONIC_TRIG 13
#define ULTRASONIC_ECHO 4

// ====================
// SERVO OBYEKTLƏRİ
// ====================

Servo motor1, motor2, motor3, motor4;
MPU6050 mpu;
DHT dht(DHTPIN, DHTTYPE);

// ====================
// SENSOR DEĞİŞKƏNLƏRİ
// ====================

int16_t accelX, accelY, accelZ;
int16_t gyroX, gyroY, gyroZ;
float angleX = 0, angleY = 0, angleZ = 0;
float distance = 0;
float temperature = 0;
float humidity = 0;

// ====================
// MOTOR KONTROL DEĞİŞKƏNLƏRİ
// ====================

int motor1_speed = 1000;
int motor2_speed = 1000;
int motor3_speed = 1000;
int motor4_speed = 1000;

// ====================
// PID KONTROL
// ====================

struct PID {
  float kp, ki, kd;
  float error, prev_error;
  float integral;
  float output;
};

PID pid_x = {1.5, 0.1, 0.8, 0, 0, 0, 0};
PID pid_y = {1.5, 0.1, 0.8, 0, 0, 0, 0};
PID pid_z = {2.0, 0.2, 0.5, 0, 0, 0, 0};

// ====================
// FAILSAFE TIMEOUT
// ====================

#define FAILSAFE_TIMEOUT 1000  // 1 saniyə
unsigned long last_signal_time = 0;
bool failsafe_active = false;

// ====================
// SETUP FUNKSİYASı
// ====================

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  // ==== NRF24 İNİSİALİZASİYA ====
  Serial.println("===== DRON BAŞLANIYOR =====");
  Serial.println("NRF24L01 başladılıyor...");
  
  if (!radio.begin()) {
    Serial.println("ERROR: NRF24 tapılmadı!");
    while (1) {
      delay(100);
    }
  }
  
  radio.openReadingPipe(1, address);      // Oxuma üçün adres
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(76);
  radio.setRetries(3, 5);
  radio.startListening();                 // RX (Qəbuledicisi) modu
  
  Serial.println("✓ NRF24 hazır!");
  
  // ==== MOTOR BAŞLATMA ====
  Serial.println("Motorlar başladılıyor...");
  motor1.attach(MOTOR1_PIN);
  motor2.attach(MOTOR2_PIN);
  motor3.attach(MOTOR3_PIN);
  motor4.attach(MOTOR4_PIN);
  
  armMotors();
  
  // ==== MPU6050 BAŞLATMA ====
  Serial.println("MPU6050 başladılıyor...");
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("ERROR: MPU6050 tapılmadı!");
    while (1);
  }
  Serial.println("✓ MPU6050 hazır!");
  
  // ==== DHT BAŞLATMA ====
  Serial.println("DHT sensoru başladılıyor...");
  dht.begin();
  Serial.println("✓ DHT hazır!");
  
  // ==== PIN KONFIGURASYONU ====
  pinMode(ULTRASONIC_TRIG, OUTPUT);
  pinMode(ULTRASONIC_ECHO, INPUT);
  
  Serial.println("===== DRON HAZIR =====");
  Serial.println("Pultdan siqnal gözlənilir...");
  
  last_signal_time = millis();
  delay(2000);
}

// ====================
// MOTOR ARM ETMƏ
// ====================

void armMotors() {
  motor1.writeMicroseconds(1000);
  motor2.writeMicroseconds(1000);
  motor3.writeMicroseconds(1000);
  motor4.writeMicroseconds(1000);
  delay(2000);
  Serial.println("✓ Motorlar hazırlandı!");
}

// ====================
// NRF24 SİNYAL QƏBULU
// ====================

void receiveControlSignal() {
  if (radio.available()) {
    radio.read(&controlData, sizeof(controlData));
    last_signal_time = millis();
    failsafe_active = false;
    
    // Debug çıxışı
    static unsigned long last_print = 0;
    if (millis() - last_print >= 500) {
      last_print = millis();
      
      Serial.print("✓ SİNYAL QƏBUL: ");
      Serial.print("Gas:");
      Serial.print(controlData.throttle);
      Serial.print(" | Yaw:");
      Serial.print(controlData.yaw);
      Serial.print(" | Pitch:");
      Serial.print(controlData.pitch);
      Serial.print(" | Roll:");
      Serial.print(controlData.roll);
      Serial.print(" | Armed:");
      Serial.println(controlData.armed ? "YES" : "NO");
    }
  } else {
    // Failsafe: Siqnal 1 saniyədən çox gəlməzse
    if (millis() - last_signal_time > FAILSAFE_TIMEOUT) {
      if (!failsafe_active) {
        Serial.println("⚠️ XƏTA: Pultdan siqnal alınmır - FAILSAFE AKTIV!");
        failsafe_active = true;
      }
      
      // Motorları söndür
      controlData.throttle = 1000;
      controlData.armed = false;
    }
  }
}

// ====================
// SENSOR OXUTMASI
// ====================

void readSensors() {
  // MPU6050 oxut
  mpu.getAcceleration(&accelX, &accelY, &accelZ);
  mpu.getRotation(&gyroX, &gyroY, &gyroZ);
  
  // Açıları hesabla
  angleX = atan2(accelY, accelZ) * 180 / PI;
  angleY = atan2(-accelX, sqrt(accelY * accelY + accelZ * accelZ)) * 180 / PI;
  
  // Hündürlük oxut
  measureDistance();
  
  // İstilik/Rütubət oxut
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
}

// ====================
// ULTRASONIC HÜNDÜRLÜK
// ====================

void measureDistance() {
  digitalWrite(ULTRASONIC_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG, LOW);
  
  unsigned long pulse_time = pulseIn(ULTRASONIC_ECHO, HIGH, 30000);
  distance = pulse_time * 0.034 / 2;
}

// ====================
// PID HESABLAMA
// ====================

float calculatePID(PID &pid, float target, float current) {
  pid.error = target - current;
  pid.integral += pid.error;
  
  if (pid.integral > 100) pid.integral = 100;
  if (pid.integral < -100) pid.integral = -100;
  
  float derivative = pid.error - pid.prev_error;
  pid.prev_error = pid.error;
  
  pid.output = (pid.kp * pid.error) + (pid.ki * pid.integral) + (pid.kd * derivative);
  
  return pid.output;
}

// ====================
// MOTOR KONTROL
// ====================

void controlMotors() {
  // Pultdan gələn hədəf dəyərləri
  float target_roll = map(controlData.roll, 1000, 2000, -45, 45);
  float target_pitch = map(controlData.pitch, 1000, 2000, -45, 45);
  float target_yaw_rate = map(controlData.yaw, 1000, 2000, -100, 100);
  
  // PID hesablamaları
  float pid_roll = calculatePID(pid_x, target_roll, angleX);
  float pid_pitch = calculatePID(pid_y, target_pitch, angleY);
  float pid_yaw = calculatePID(pid_z, target_yaw_rate, gyroZ / 131.0);
  
  // Baza hızı (gas)
  int throttle = controlData.throttle;
  
  // X-konfigurasi motor hızları
  motor1_speed = throttle + pid_roll + pid_pitch - pid_yaw;
  motor2_speed = throttle - pid_roll + pid_pitch + pid_yaw;
  motor3_speed = throttle + pid_roll - pid_pitch + pid_yaw;
  motor4_speed = throttle - pid_roll - pid_pitch - pid_yaw;
  
  // Hədləri tətbiq et
  motor1_speed = constrain(motor1_speed, 1000, 2000);
  motor2_speed = constrain(motor2_speed, 1000, 2000);
  motor3_speed = constrain(motor3_speed, 1000, 2000);
  motor4_speed = constrain(motor4_speed, 1000, 2000);
  
  // Motorları idarə et
  if (controlData.armed) {
    motor1.writeMicroseconds(motor1_speed);
    motor2.writeMicroseconds(motor2_speed);
    motor3.writeMicroseconds(motor3_speed);
    motor4.writeMicroseconds(motor4_speed);
  } else {
    // Motorları söndür
    motor1.writeMicroseconds(1000);
    motor2.writeMicroseconds(1000);
    motor3.writeMicroseconds(1000);
    motor4.writeMicroseconds(1000);
  }
}

// ====================
// DEBUG ÇIXıŞı
// ====================

void printDebugInfo() {
  static unsigned long last_print = 0;
  if (millis() - last_print >= 1000) {
    last_print = millis();
    
    Serial.print("Açı: X=");
    Serial.print(angleX, 1);
    Serial.print("° Y=");
    Serial.print(angleY, 1);
    Serial.print("° | ");
    
    Serial.print("Hündürlük: ");
    Serial.print(distance, 1);
    Serial.print("sm | ");
    
    Serial.print("T:");
    Serial.print(temperature, 1);
    Serial.print("°C H:");
    Serial.print(humidity, 1);
    Serial.print("% | ");
    
    Serial.print("Motorlar: ");
    Serial.print(motor1_speed);
    Serial.print(",");
    Serial.print(motor2_speed);
    Serial.print(",");
    Serial.print(motor3_speed);
    Serial.print(",");
    Serial.println(motor4_speed);
  }
}

// ====================
// MAIN LOOP
// ====================

void loop() {
  // Pultdan siqnal qəbul et
  receiveControlSignal();
  
  // Sensorları oxu
  readSensors();
  
  // Motorları kontrol et
  controlMotors();
  
  // Debug məlumatı
  printDebugInfo();
  
  // Kontrol tezliyi
  delay(20);  // 50Hz
}
