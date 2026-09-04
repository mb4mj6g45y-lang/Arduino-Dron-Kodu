#include <Wire.h>
#include <MPU6050.h>
#include <Servo.h>
#include <DHT.h>

// ====================
// PIN TƏYİNATLARı
// ====================

// Motor PWM Pinləri
#define MOTOR1_PIN 3   // Front-Left
#define MOTOR2_PIN 5   // Front-Right
#define MOTOR3_PIN 6   // Back-Left
#define MOTOR4_PIN 9   // Back-Right

// RC Qəbuledicisi Pinləri
#define CH1_PIN 8      // Gas (Throttle)
#define CH2_PIN 7      // Sağa-Sola (Yaw)
#define CH3_PIN 12     // İrəli-Geri (Pitch)
#define CH4_PIN 11     // Yuxarı-Aşağı (Roll)

// Sensorlar
#define DHTPIN 2       // İstilik/Rütubət Sensoru
#define DHTTYPE DHT22
#define ULTRASONIC_TRIG 13
#define ULTRASONIC_ECHO 4

// ====================
// OBYEKTLƏRİN YARADILMASI
// ====================

Servo motor1, motor2, motor3, motor4;
MPU6050 mpu;
DHT dht(DHTPIN, DHTTYPE);

// ====================
// DEĞİŞKƏNLƏR
// ====================

// RC Qəbuledicisi Dəyərləri
volatile unsigned long ch1_start, ch2_start, ch3_start, ch4_start;
volatile int ch1_value = 1000; // Gas
volatile int ch2_value = 1500; // Sağa-Sola
volatile int ch3_value = 1500; // İrəli-Geri
volatile int ch4_value = 1500; // Yuxarı-Aşağı

// MPU6050 Dəyərləri
int16_t accelX, accelY, accelZ;
int16_t gyroX, gyroY, gyroZ;
float angleX = 0, angleY = 0, angleZ = 0;

// Hündürlük Sensorunun Dəyərləri
float distance = 0;
unsigned long pulse_time;

// İstilik/Rütubət Sensorunun Dəyərləri
float temperature = 0;
float humidity = 0;

// Motor Hızları
int motor1_speed = 1000;
int motor2_speed = 1000;
int motor3_speed = 1000;
int motor4_speed = 1000;

// Zaman
unsigned long last_time = 0;
unsigned long current_time = 0;

// ====================
// PID Kontrolü
// ====================

struct PID {
  float kp, ki, kd;
  float error, prev_error;
  float integral;
  float output;
};

PID pid_x = {1.5, 0.1, 0.8, 0, 0, 0, 0};   // Roll (X oxu)
PID pid_y = {1.5, 0.1, 0.8, 0, 0, 0, 0};   // Pitch (Y oxu)
PID pid_z = {2.0, 0.2, 0.5, 0, 0, 0, 0};   // Yaw (Z oxu)

// ====================
// SETUP FUNKSİYASI
// ====================

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  // Motorları başlat
  motor1.attach(MOTOR1_PIN);
  motor2.attach(MOTOR2_PIN);
  motor3.attach(MOTOR3_PIN);
  motor4.attach(MOTOR4_PIN);
  
  // Motorları ARM etmə (ESC kalibrasyonu)
  armMotors();
  
  // MPU6050 başlat
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 bağlanmadı!");
    while (1);
  }
  
  // DHT başlat
  dht.begin();
  
  // Ultrasonic sensor pinləri
  pinMode(ULTRASONIC_TRIG, OUTPUT);
  pinMode(ULTRASONIC_ECHO, INPUT);
  
  // RC Qəbuledicisi Interrupts
  attachInterrupt(digitalPinToInterrupt(CH1_PIN), readCH1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(CH2_PIN), readCH2, CHANGE);
  attachInterrupt(digitalPinToInterrupt(CH3_PIN), readCH3, CHANGE);
  attachInterrupt(digitalPinToInterrupt(CH4_PIN), readCH4, CHANGE);
  
  Serial.println("Dron başladı!");
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
  Serial.println("Motorlar hazırlandı!");
}

// ====================
// RC QƏBULEDICISI INTERRUPT FUNKSİYALARI
// ====================

void readCH1() {
  if (digitalRead(CH1_PIN) == HIGH) {
    ch1_start = micros();
  } else {
    ch1_value = micros() - ch1_start;
  }
}

void readCH2() {
  if (digitalRead(CH2_PIN) == HIGH) {
    ch2_start = micros();
  } else {
    ch2_value = micros() - ch2_start;
  }
}

void readCH3() {
  if (digitalRead(CH3_PIN) == HIGH) {
    ch3_start = micros();
  } else {
    ch3_value = micros() - ch3_start;
  }
}

void readCH4() {
  if (digitalRead(CH4_PIN) == HIGH) {
    ch4_start = micros();
  } else {
    ch4_value = micros() - ch4_start;
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
  
  // Hündürlük oxut (Ultrasonic)
  measureDistance();
  
  // İstilik/Rütubət oxut
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
}

// ====================
// ULTRASONIC HÜNDÜRLÜK SENSORUꓤ
// ====================

void measureDistance() {
  // Trigger pulse göndər
  digitalWrite(ULTRASONIC_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG, LOW);
  
  // Echo vaxtını oxut
  pulse_time = pulseIn(ULTRASONIC_ECHO, HIGH, 30000);
  distance = pulse_time * 0.034 / 2;  // sm-də
}

// ====================
// PID KONTROLÜ HESABLA
// ====================

float calculatePID(PID &pid, float target, float current) {
  pid.error = target - current;
  pid.integral += pid.error;
  
  // İntegralı sınırla
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
  // RC girdilərindən hədəf açıları al
  float target_roll = map(ch4_value, 1000, 2000, -45, 45);
  float target_pitch = map(ch3_value, 1000, 2000, -45, 45);
  float target_yaw_rate = map(ch2_value, 1000, 2000, -100, 100);
  
  // PID hesablamaları
  float pid_roll = calculatePID(pid_x, target_roll, angleX);
  float pid_pitch = calculatePID(pid_y, target_pitch, angleY);
  float pid_yaw = calculatePID(pid_z, target_yaw_rate, gyroZ / 131.0);
  
  // Baza hızı (gas)
  int throttle = ch1_value;
  
  // Motorlar üçün hızları hesabla (X-konfigurasi)
  motor1_speed = throttle + pid_roll + pid_pitch - pid_yaw;
  motor2_speed = throttle - pid_roll + pid_pitch + pid_yaw;
  motor3_speed = throttle + pid_roll - pid_pitch + pid_yaw;
  motor4_speed = throttle - pid_roll - pid_pitch - pid_yaw;
  
  // Hədləri tətbiq et (1000-2000 mikrosan)
  motor1_speed = constrain(motor1_speed, 1000, 2000);
  motor2_speed = constrain(motor2_speed, 1000, 2000);
  motor3_speed = constrain(motor3_speed, 1000, 2000);
  motor4_speed = constrain(motor4_speed, 1000, 2000);
  
  // Motorları idarə et
  motor1.writeMicroseconds(motor1_speed);
  motor2.writeMicroseconds(motor2_speed);
  motor3.writeMicroseconds(motor3_speed);
  motor4.writeMicroseconds(motor4_speed);
}

// ====================
// SERIYA MONİTOR ÇIXIŞI
// ====================

void printDebugInfo() {
  static unsigned long last_print = 0;
  if (millis() - last_print >= 500) {  // Hər 500ms çap et
    last_print = millis();
    
    Serial.print("Gas:");
    Serial.print(ch1_value);
    Serial.print(" | Sağa-Sola:");
    Serial.print(ch2_value);
    Serial.print(" | İrəli-Geri:");
    Serial.print(ch3_value);
    Serial.print(" | Yuxarı-Aşağı:");
    Serial.println(ch4_value);
    
    Serial.print("Açı X:");
    Serial.print(angleX, 2);
    Serial.print("° | Açı Y:");
    Serial.print(angleY, 2);
    Serial.println("°");
    
    Serial.print("Hündürlük:");
    Serial.print(distance, 2);
    Serial.print("sm | İstilik:");
    Serial.print(temperature, 1);
    Serial.print("°C | Rütubət:");
    Serial.print(humidity, 1);
    Serial.println("%");
    
    Serial.print("Motor1:");
    Serial.print(motor1_speed);
    Serial.print(" | Motor2:");
    Serial.print(motor2_speed);
    Serial.print(" | Motor3:");
    Serial.print(motor3_speed);
    Serial.print(" | Motor4:");
    Serial.println(motor4_speed);
    
    Serial.println("---");
  }
}

// ====================
// MAIN LOOP
// ====================

void loop() {
  current_time = millis();
  
  // Sensorları oxu
  readSensors();
  
  // Motorları kontrol et
  controlMotors();
  
  // Debug məlumatını çap et
  printDebugInfo();
}
