# Arduino Dron Layihəsi - Başlangıc Təlimatı

## 🎯 Layihənin Məqsədi

Bu layihə Arduino mikrokontrolerləri istifadə edərək **simsiz idarəetmə sistəminə** sahib bir **dörtrotorlu dron (Quadcopter)** yaratmaqdir. 

## 📦 Layihə Dosyaları

Repositoriyada aşağıdakı fayllar var:

| Fayl | Məqsəd |
|------|--------|
| `dron_kontrolu.ino` | Ana dron kontrolu - Motorlar, sensorlar, PID |
| `rc_pult_kontrolu.ino` | Pult kontrolu - Joystick ilə siqnal göndərəcəyi (kabel) |
| `pult_wireless_nrf24.ino` | Wireless pult - NRF24 üzərindən siqnal göndərən |
| `dron_wireless_nrf24.ino` | Wireless dron - NRF24 üzərindən siqnal qəbul edən |
| `README.md` | Kütüphana qurulum təlimatı |
| `PULT_KONTROLU_TELIMATI.md` | Pult quruluşu və bağlantısı |
| `NRF24L01_KURULUM.md` | Wireless modul qurulum təlimatı |

## ⚙️ Hardware Tələbləri

### Mikrokontroller
- **Arduino Mega 2560** (tövsiyyə edilir - çox pin var)
- Alternativ: Arduino Uno (daha az pinə malik)

### Motorlar və ESC
- 4x Brushless DC Motor (3S-4S Lipo)
- 4x ESC (30A minimum)
- 4x Propeller

### Sensorlar
- **MPU6050** - 6-axis gyroscope/accelerometer
- **DHT22** - Istilik və rütubət sensoru
- **HC-SR04** - Ultrasonic hündürlük sensoru

### Wireless Modullar
- 2x **NRF24L01** (pult və dron üçün)

### Digər
- Lipo Batareyası (2S-4S)
- Şarj cihazı
- Jumper kabelləri
- 10kΩ rezistorlar
- 10µF kapasitorlar
- Joystick modulları (2x)
- Düymələr (2x)
- LED-lər

## 🚀 Başlangıc Addımları

### 1️⃣ Arduino IDE Qurulum

1. [Arduino IDE](https://www.arduino.cc/en/software) yüklə
2. Board Manager-da Arduino Mega 2560 seç
3. Lazım olan kütüphanələri quraş:
   - **Wire** (I2C) - Standart
   - **Servo** - Standart
   - **SPI** - Standart
   - **MPU6050** - Elektroda
   - **DHT** - Adafruit
   - **RF24** - TMRh20

### 2️⃣ Hardware Montajı

#### Dron Taxtası
```
1. Arduino Mega-nı dronun gövdəsinə quraş
2. MPU6050-ni SDA (A4), SCL (A5) pinlərinə qoşu
3. DHT22-ni A0-a qoşu
4. HC-SR04 sensoru: TRIG (Pin 13), ECHO (Pin 4)
5. ESC-ləri motor pinlərinə qoşu (3, 5, 6, 9)
6. Batareyaları qoşu
```

#### Pult Taxtası
```
1. Arduino Uno/Mega taxtasını pultun kutusunda saxla
2. Sol Joystick: A2 (Y), A1 (X)
3. Sağ Joystick: A0 (Y), A3 (X)
4. ARM düyməsi: Pin 2
5. DISARM düyməsi: Pin 4
6. LED-lər: Pin 11, 12, 13
7. NRF24L01: CE (10), CSN (53)
```

#### NRF24L01 Bağlantısı
```
XƏBƏRDARLIQ: VCC 3.3V olmalı! 5V yazmayacaq!

Pult NRF24:
  GND → GND
  VCC → 3.3V + 10µF Kapasitor
  CSN → Pin 53
  CE → Pin 10
  SPI: MOSI (51), MISO (50), SCK (52)

Dron NRF24:
  GND → GND
  VCC → 3.3V + 10µF Kapasitor
  CSN → Pin 53
  CE → Pin 10
  SPI: MOSI (51), MISO (50), SCK (52)
```

### 3️⃣ Kod Yüklə

#### Dronə Kod Yüklə
```
1. Arduino IDE aç
2. dron_wireless_nrf24.ino aç
3. Tools → Board → Arduino Mega 2560
4. Tools → Port → Dronun COM portu seç
5. Upload düyməsinə basıb sürüklə
```

#### Pulta Kod Yüklə
```
1. Arduino IDE aç
2. pult_wireless_nrf24.ino aç
3. Tools → Board → Arduino Mega 2560
4. Tools → Port → Pultun COM portu seç
5. Upload düyməsinə basıb sürüklə
```

### 4️⃣ Sınaq

#### Serial Monitor Sınaq
```
1. Dronun Serial Monitor-u açıq tutun (115200 baud)
2. Pultun Serial Monitor-u açıq tutun (115200 baud)
3. Pultda Joystick-ləri tərpət edin
4. Dron monitorda siqnal alıp-almadığını yoxla
```

#### Hardware Sınaq
```
1. Propellerləri çıxarın!
2. Batareyaları qoşun
3. Dronun NRF24 işləyib-işləmədiyini yoxla (LED yanmalı)
4. Pultda Joystick-ləri testlə
5. ARM düyməsinə basıb motorları sınaqla
```

## 🎮 Kontrol Xəritəsi

### Sol Joystick
- **X (Sağa-Sola):** Dronun dönməsi (YAW)
- **Y (Yuxarı-Aşağı):** İrəli-geri hərəkəti (PITCH)

### Sağ Joystick
- **X (Sağa-Sola):** Yanlış hərəkəti (ROLL)
- **Y (Yuxarı-Aşağı):** Hündürlüyü dəyişdirmə (THROTTLE)

### Düymələr
- **ARM:** Motorları başlat (Gas = MIN olmalı)
- **DISARM:** Motorları söndür

## 📊 Sensor Məlumatları

Dron aşağıdakı məlumatları ölçür:

- **Əyilmə Açıları:** X, Y oxuları (Roll, Pitch)
- **Hündürlük:** Ultrasonic sensoru ilə
- **Istilik:** DHT22 vasitəsilə
- **Rütubət:** DHT22 vasitəsilə
- **Gyro Məlumatları:** MPU6050-dən

## ⚙️ PID Ayarlaması

Əgər dron uçmasa, `dron_wireless_nrf24.ino` faylında PID parametrlərini dəyiş:

```cpp
PID pid_x = {1.5, 0.1, 0.8, 0, 0, 0, 0};  // Roll
PID pid_y = {1.5, 0.1, 0.8, 0, 0, 0, 0};  // Pitch
PID pid_z = {2.0, 0.2, 0.5, 0, 0, 0, 0};  // Yaw
```

- **KP:** Artır - daha hızlı cavab
- **KI:** Artır - uzun müddətli sapmaları düzəlt
- **KD:** Artır - titrəməni azalt

## 🔧 Xətaların Həlli

| Xəta | Həlli |
|-----|------|
| NRF24 tapılmadı | Kabel bağlantısını yoxla, VCC 3.3V olmalı |
| MPU6050 tapılmadı | SDA/SCL kablolarını yoxla |
| DHT tapılmadı | Rezistor bağlantısını yoxla (10kΩ) |
| Motorlar işləmir | ESC kalibrasyonunu yenilə |
| Joystick cavab vermir | Analog pinləri yoxla |
| Pultdan siqnal alınmır | NRF24 pinlərini yoxla, aynı kanal istifadə et |

## ⚠️ Təhlükəsizlik Qaydasından

**ÖNƏMLİ:**
- ✅ Propellerləri quraşdırmazdan ƏVVƏL testlə
- ✅ Batareyaları qeyd edin (Lipo patlamağa meyllidir)
- ✅ Qoruma gözlüyü taxın
- ✅ Açıq sahədə sınaqla
- ✅ İnsanların yanında sınaqlamayın
- ✅ Motorları ARM etməzdən əvvəl gas MIN olmalı

## 📚 Əlavə Mənbələr

- [Arduino Rəsmi Sayt](https://www.arduino.cc)
- [MPU6050 Dokumentasiyası](https://invensense.tdk.com/products/motion-tracking/6-axis/)
- [RF24 GitHub](https://github.com/tmrh20/RF24)
- [DHT Sensor](https://github.com/adafruit/DHT-sensor-library)

## 🎓 Öyrənmə Məqsədi

Bu layihə vasitəsilə aşağıdakıları öyrənəcəksiniz:

1. **Mikrokontroller Proqramlaması** - Arduino C/C++
2. **Sensor İntaqrasiyonu** - I2C, Analog, Digital sensorlar
3. **Motor Kontrol** - PWM, ESC iletişim
4. **Wireless Əlaqə** - RF24 protokolu
5. **PID Kontrol Sistemi** - Feeeback kontrol
6. **Real-time Sistem** - Zamanlamalı işlər

## 🤝 Dəstək və Sorular

Əgər problemlə rastlaşsanız:

1. Serial Monitor-da xəta mesajlarını yoxla
2. GitHub-da açıq issue yarat
3. Arduino forumunda soruş
4. Kabel bağlantılarını yenidən yoxla

## 📝 Versiya Tarixçəsi

| Versiya | Tarix | Dəyişikliklər |
|---------|-------|---------------|
| 1.0 | 2026-09-04 | İlkin versiya |

## 📄 Lisenziya

Bu layihə açıq qaynaq (Open Source) layihədir. Azad istifadə edilə bilər.

---

**Sağlıqlı kodlama! 🚁**

Suallarınız olsa, GitHub-da issue açın və ya diskusiyalara qatılın.
