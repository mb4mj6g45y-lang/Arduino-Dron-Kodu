# NRF24L01 Wireless Kütüphanəsi Qurulum Təlimatı

## NRF24L01 Modulu Nədir?

NRF24L01, 2.4GHz frekansında simsiz əlaqə yaratmaq üçün istifadə olunan bir radio modulu. Pult ilə dron arasında siqnal göndərməsi üçün istifadə olunur.

## Lazım olan Kütüphanə

**RF24** kütüphanəsi - TMRh20 tərəfindən hazırlanmış

## Qurulum Addımları

### 1. Arduino IDE-də Kütüphanə Quraş

```
Arduino IDE açın → Sketch → Include Library → Manage Libraries
```

### 2. RF24 Axtarış və Quraş

```
Search: "RF24"
Müəllif: TMRh20
Ən yeni versiya quraş
```

### 3. Alternativ: Manuel Qurulum

Əgər otomatik qurulum işləməzsə:

```
1. GitHub-dan endirin: https://github.com/tmrh20/RF24
2. Arduino/libraries/ qovluğuna yerləşdirin
3. Arduino IDE-ni yenidən başladın
```

## NRF24L01 Hardware Bağlantısı

### Pin Bağlantıları

| NRF24 Pini | Arduino Pini | Qeyd |
|-----------|------------|------|
| GND | GND | Yer |
| VCC | 3.3V | Qida (3.3V LAZIMDIR!) |
| CE | 10 | Chip Enable |
| CSN | 53 | Chip Select (Mega) / 8 (Uno) |
| SCK | 52 | SPI Clock (Mega) / 13 (Uno) |
| MOSI | 51 | SPI Data Out (Mega) / 11 (Uno) |
| MISO | 50 | SPI Data In (Mega) / 12 (Uno) |
| IRQ | (opsional) | Interrupt |

### Wiring Diaqramı (Arduino Mega)

```
NRF24L01
┌─────────────┐
│ GND ─────── GND (Arduino)
│ VCC ─────── 3.3V (Arduino) + 10µF Kapasitor
│ CSN ─────── Pin 53
│ CE  ─────── Pin 10
│ SCK ─────── Pin 52
│ MOSI ────── Pin 51
│ MISO ────── Pin 50
└─────────────┘
```

### Wiring Diaqramı (Arduino Uno)

```
NRF24L01
┌─────────────┐
│ GND ─────── GND (Arduino)
│ VCC ─────── 3.3V (Arduino) + 10µF Kapasitor
│ CSN ─────── Pin 8
│ CE  ─────── Pin 7
│ SCK ─────── Pin 13
│ MOSI ────── Pin 11
│ MISO ────── Pin 12
└─────────────┘
```

## ÖNƏMLİ XƏBƏRDARLIQLAR

⚠️ **VCC VOLTAJ**
- NRF24L01 **3.3V** ilə işləməlidir!
- Arduino 5V-dan birbaşa bağlamayın - MODULU XARAB EDƏR!
- **Kapasitor** (10µF) istifadə edin (VCC və GND arasında)

⚠️ **GND BAĞLANTISI**
- Arduino və NRF24 ortaq GND-ye bağlı olmalı!
- Zəif bağlantılar səhvlərə səbəb olar

## Kodda Kütüphanə Daxil Etmə

```cpp
#include <SPI.h>
#include <RF24.h>

RF24 radio(CE_PIN, CSN_PIN);
const byte address[6] = "00001";

void setup() {
  if (!radio.begin()) {
    Serial.println("NRF24 tapılmadı!");
  }
}
```

## NRF24L01 Konfigurasyonu

```cpp
// Qəbuledicisi modu
radio.openReadingPipe(1, address);
radio.setPALevel(RF24_PA_MIN);        // Güc: MIN, LOW, HIGH, MAX
radio.setDataRate(RF24_250KBPS);      // 250KBPS, 1MBPS, 2MBPS
radio.setChannel(76);                  // Kanal (0-125)
radio.startListening();                // RX modu

// Göndəricisi modu
radio.openWritingPipe(address);
radio.stopListening();                 // TX modu
radio.write(&data, sizeof(data));
```

## Pult İçin Kodda Lazım olan Dəyişikliklər

Arduino UNO istifadə edirsinizsə `pult_wireless_nrf24.ino` faylında:

```cpp
// Dəyiş:
#define CE_PIN 7
#define CSN_PIN 8

// Əvəzinə:
#define CE_PIN 10
#define CSN_PIN 53
```

## Dron İçin Kodda Lazım olan Dəyişikliklər

Arduino UNO istifadə edirsinizsə `dron_wireless_nrf24.ino` faylında:

```cpp
// Dəyiş:
#define CE_PIN 10
#define CSN_PIN 53

// Əvəzinə:
#define CE_PIN 7
#define CSN_PIN 8
```

## Sınaq Kodu

NRF24L01-in düzgün işləyib-işləmədiyini yoxlamaq üçün:

```cpp
#include <SPI.h>
#include <RF24.h>

RF24 radio(7, 8);  // CE, CSN
const byte address[6] = "00001";

void setup() {
  Serial.begin(115200);
  
  if (!radio.begin()) {
    Serial.println("ERROR: NRF24 tapılmadı!");
    while (1);
  }
  
  radio.openReadingPipe(1, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(76);
  radio.startListening();
  
  Serial.println("NRF24 hazır!");
}

void loop() {
  if (radio.available()) {
    char text[32] = "";
    radio.read(&text, sizeof(text));
    Serial.print("Qəbul: ");
    Serial.println(text);
  }
}
```

## Xətaların Həlli

| Problem | Həlli |
|---------|------|
| "NRF24 tapılmadı" | Kabel bağlantısını yoxla, VCC 3.3V olmalı |
| Siqnal qəbul olunmur | SPI pinlərini yoxla (MOSI, MISO, SCK) |
| Zəif əlaqə | Kapasitor əlavə et, güç səviyyəsini artır |
| Module açılmır | 3.3V voltajını yoxla (5V xarabilik yarada bilər) |
| CE/CSN pin problemi | Kodda pin nömrələrini yoxla |

## Kanal Seçimi

NRF24L01 2.4GHz Wi-Fi ilə əlaqədar ola bilər:

- **Kanal 76-100:** Az tezlikli (tövsiyyə edilir)
- **Kanal 1-50:** Wi-Fi ilə təsadüf ola bilər

## Performance Optimizasiyonu

```cpp
// Ən sürətli (ən az məsafə)
radio.setDataRate(RF24_2MBPS);
radio.setPALevel(RF24_PA_MAX);

// Ən uzaq (ən az sürət)
radio.setDataRate(RF24_250KBPS);
radio.setPALevel(RF24_PA_MAX);
```

## Data Struktur

Pult və dron arasında eyni struktur istifadə etmək lazımdır:

```cpp
struct ControlData {
  int throttle;
  int yaw;
  int pitch;
  int roll;
  bool armed;
};
```

---

**Versiya:** 1.0  
**Son yenilənmə:** 2026-09-04  
**Tövsiyyə:** Sınaqdan əvvəl propellerləri çıxarın!
