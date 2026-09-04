#include <SPI.h>
#include <RF24.h>

// ====================
// NRF24L01 WIRELESS KÜTÜPHANƏSİ
// ====================
// Bu kod pult ilə dron arasında simsiz əlaqə yaratır
// NRF24L01 modulu istifadə edir (2.4GHz frekansı)

// ====================
// NRF24 PIN KONFIGURASYONU
// ====================

#define CE_PIN 7      // Chip Enable
#define CSN_PIN 8     // Chip Select

// ====================
// RF24 OBYEKTİ
// ====================

RF24 radio(CE_PIN, CSN_PIN);
const byte address[6] = "00001";  // 5 hərfli adres

// ====================
// DATA STRUKTURASı
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
// SETUP FUNKSİYASI
// ====================

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("===== NRF24L01 İNİSİALİZASİYA =====");
  
  // NRF24 başlat
  if (!radio.begin()) {
    Serial.println("ERROR: NRF24 tapılmadı!");
    while (1) {
      delay(100);
    }
  }
  
  // NRF24 konfigurasyonu
  radio.openWritingPipe(address);        // Yazmaq üçün adres
  radio.setPALevel(RF24_PA_MIN);         // Güc səviyyəsi (MIN/LOW/HIGH/MAX)
  radio.setDataRate(RF24_250KBPS);       // Sürət (250KBPS, 1MBPS, 2MBPS)
  radio.setChannel(76);                  // Kanal (0-125, 2.4GHz)
  radio.setRetries(3, 5);                // Retry sayı
  radio.stopListening();                 // TX (Göndərici) modu
  
  Serial.println("NRF24 hazır!");
  Serial.print("Adres: ");
  Serial.println((const char*)address);
  Serial.println("Pult → Dron siqnalı göndərilir...");
  delay(1000);
}

// ====================
// NRF24 SIQNAL GÖNDƏR
// ====================

void sendControlSignal() {
  // Kontrol verisi göndər
  if (radio.write(&controlData, sizeof(controlData))) {
    Serial.print("✓ Siqnal göndərildi | ");
  } else {
    Serial.print("✗ XƏTA: Siqnal göndərilə bilmədi | ");
  }
  
  // Debug məlumatı
  Serial.print("Throttle: ");
  Serial.print(controlData.throttle);
  Serial.print(" | Yaw: ");
  Serial.print(controlData.yaw);
  Serial.print(" | Pitch: ");
  Serial.print(controlData.pitch);
  Serial.print(" | Roll: ");
  Serial.print(controlData.roll);
  Serial.print(" | Armed: ");
  Serial.println(controlData.armed ? "YES" : "NO");
}

// ====================
// PULT VERİSİ OXU
// ====================

void readPultData() {
  // Joystick analog dəyərləri oxu (0-1023)
  int throttle_raw = analogRead(A0);
  int yaw_raw = analogRead(A1);
  int pitch_raw = analogRead(A2);
  int roll_raw = analogRead(A3);
  
  // Kalibre edilən dəyərləri PWM-ə çevir (1000-2000)
  controlData.throttle = map(throttle_raw, 0, 1023, 1000, 2000);
  controlData.yaw = map(yaw_raw, 0, 1023, 1000, 2000);
  controlData.pitch = map(pitch_raw, 0, 1023, 1000, 2000);
  controlData.roll = map(roll_raw, 0, 1023, 1000, 2000);
  
  // Düymə statusu oxu
  bool arm_button = !digitalRead(2);      // ARM (Pull-up, HIGH = basılmamış)
  bool disarm_button = !digitalRead(4);   // DISARM
  
  // ARM/DISARM əməliyyatları
  if (arm_button && controlData.throttle < 1050) {
    controlData.armed = true;
  }
  
  if (disarm_button) {
    controlData.armed = false;
    controlData.throttle = 1000;  // Gas sıfırla
  }
  
  // DISARM olsa, throttle sıfırla
  if (!controlData.armed) {
    controlData.throttle = 1000;
  }
}

// ====================
// MAIN LOOP
// ====================

void loop() {
  // Pult verisi oxu
  readPultData();
  
  // Dronə siqnal göndər
  sendControlSignal();
  
  // Kontrol tezliyi (50Hz = 20ms)
  delay(20);
}
