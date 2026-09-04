# Arduino Pult Kontrolü - Detaylı Təlimat

## Pult Aparatı Nədir?

Arduino pult, dron ilə Kabelsiz əlaqə yaratmaq üçün istifadə olunan bir kontrol cihazıdır. Bu pult joystick-lərdən, düymələrdən və sensorlardan istifadə edərək dronə əmrləri göndərir.

## Pult Quruluşu

```
┌─────────────────────────────────────┐
│        RC PULT KONTROL              │
│                                      │
│  SOL JOYSTICK    SAĞ JOYSTICK       │
│  (Pitch/Yaw)     (Throttle/Roll)    │
│   ┌───┐           ┌───┐             │
│   │ ◯ │           │ ◯ │             │
│   └───┘           └───┘             │
│                                      │
│  [ARM] [DISARM]                     │
│                                      │
│  LED: ●Power ●TX ●Armed             │
└─────────────────────────────────────┘
```

## Pult Pin Konfigurasyonu

| Pin | Funksiya | Qeyd |
|-----|----------|------|
| A0 | Gas (Throttle) | Sağ Joystick Vertical |
| A1 | Sağa-Sola (Yaw) | Sol Joystick Horizontal |
| A2 | İrəli-Geri (Pitch) | Sol Joystick Vertical |
| A3 | Yuxarı-Aşağı (Roll) | Sağ Joystick Horizontal |
| 3 | CH1 PWM Çıxış | Motor 1 sinyal |
| 5 | CH2 PWM Çıxış | Motor 2 sinyal |
| 6 | CH3 PWM Çıxış | Motor 3 sinyal |
| 9 | CH4 PWM Çı xış | Motor 4 sinyal |
| 2 | ARM Düyməsi | Dron ARM etmə |
| 4 | DISARM Düyməsi | Dron DISARM etmə |
| 13 | LED Power | Qida göstəricisi |
| 12 | LED TX | Transmitter göstəricisi |
| 11 | LED ARM | ARM statusu göstəricisi |

## Pult Kəbləsi Bağlantısı

### Joystick Bağlantısı (Analog)

```
Sol Joystick:
  VCC → 5V
  GND → GND
  X (Horizontal - Yaw) → A1
  Y (Vertical - Pitch) → A2

Sağ Joystick:
  VCC → 5V
  GND → GND
  X (Horizontal - Roll) → A3
  Y (Vertical - Throttle) → A0
```

### Düymə Bağlantısı

```
ARM Düyməsi:
  1-tərəfi → 5V
  2-tərəfi → Pin 2 və 10kΩ rezistor üstündən GND-yə

DISARM Düyməsi:
  1-tərəfi → 5V
  2-tərəfi → Pin 4 və 10kΩ rezistor üstündən GND-yə
```

### LED Bağlantısı

```
Power LED (Qırmızı):
  Pozitif → Pin 13
  Neqativ → Rezistor (330Ω) üstündən GND-yə

TX LED (Yaşıl):
  Pozitif → Pin 12
  Neqativ → Rezistor (330Ω) üstündən GND-yə

ARM LED (Mavi):
  Pozitif → Pin 11
  Neqativ → Rezistor (330Ω) üstündən GND-yə
```

### PWM Çıxış (Dronə Siqnal)

```
Pult Arduino → Dron Arduino

Pin 3 (CH1) → Dron Pin 8 (Ch1 Qəbuledicisi)
Pin 5 (CH2) → Dron Pin 7 (Ch2 Qəbuledicisi)
Pin 6 (CH3) → Dron Pin 12 (Ch3 Qəbuledicisi)
Pin 9 (CH4) → Dron Pin 11 (Ch4 Qəbuledicisi)

GND → GND (Ortaq qabı)
```

## Joystick Kontrol Xəritəsi

### Sol Joystick
- **X Ekseni (Sağa-Sola):** YAW kontrol - Dronun dönməsi
- **Y Ekseni (Yuxarı-Aşağı):** PITCH kontrol - Dronun irəli/geri hərəkəti

### Sağ Joystick
- **X Ekseni (Sağa-Sola):** ROLL kontrol - Dronun yanlış hərəkəti
- **Y Ekseni (Yuxarı-Aşağı):** THROTTLE kontrol - Hündürlüyü artırma/azaltma

## Pult Qurulum Addımları

### 1. Hardwareu Montaj Et
```
1. Arduino Uno/Mega taxtasını götür
2. Joystick-ləri A0-A3 pinlərinə qoşu
3. Düymələri Pin 2 və 4-ə qoşu
4. LED-ləri Pin 11, 12, 13-ə qoşu
5. Dronun siqnal giriş pinlərinə 3, 5, 6, 9 pinlərini qoşu
6. GND ortaq kabel qoşu
```

### 2. Arduino IDE-yə Kodu Yüklə
```
1. Arduino IDE aç
2. rc_pult_kontrolu.ino faylını aç
3. Board seç: Arduino Uno/Mega
4. Port seç: COM portu
5. Upload düyməsinə basın
```

### 3. Kalibrasyon
```
1. Pult başladığında "Kalibrasyon başladı" mesajı çıxacaq
2. Joystick-ləri tamamilə sağa, sola, yuxarı, aşağı tərpət et (5-10 saniyə)
3. Kalibrasyon bitəndə dəyərləri serial monitorda görcəksiniz
```

### 4. Dron ARM Etmə
```
1. Gas joystick-ə (Sağ Y) tamamilə aşağı endir
2. ARM düyməsinə basıp saxla (2-3 saniyə)
3. Mavi LED yanacaq - Dron ARMED
4. Joystick-ləri hərəkit etdikdə motorlar idarə olunacaq
```

## Serial Monitor Çıxışı Nümunəsi

```
===== RC PULT BAŞLANDI =====
Joystick kalibrasyonu...
Kalibrasyon başladı - Joystickləri tərpət edin...
CH1 (Gas): 50 - 1000
CH2 (Yaw): 45 - 995
CH3 (Pitch): 40 - 1010
CH4 (Roll): 55 - 1005
Kalibrasyon tamamlandı!
Pult hazır. Dron ARM etmə üçün ARM düyməsinə basın.

Status: DISARMED | Gas:1000 | Yaw:1500 | Pitch:1500 | Roll:1500 | Raw: 512,512,512,512
>>> DRON ARMED <<<
Status: ARMED | Gas:1200 | Yaw:1500 | Pitch:1480 | Roll:1520 | Raw: 600,512,495,520
```

## Pult idarəetməsi - Praktiki Bələdçi

### Uçuşa Hazırlıq
1. ✓ Pultun batareyasını yoxla
2. ✓ Arduino-ları bir-birinə qoşu
3. ✓ Serial Monitor aç - "Pult hazır" mesajını gözlə
4. ✓ Joystick-ləri test et

### Uçuş Əvvəlində
1. Gas joystick-ini aşağı endir (1000μs)
2. ARM düyməsinə basıb saxla
3. Mavi LED yanmalı
4. 3 saniyə gözlə

### Uçuş Zamanı
- **Sol Joystick Sol/Sağ:** Dronun dönməsi (Yaw)
- **Sol Joystick Yuxarı/Aşağı:** İrəli-geri hərəkəti (Pitch)
- **Sağ Joystick Sol/Sağ:** Yanlış hərəkəti (Roll)
- **Sağ Joystick Yuxarı/Aşağı:** Hündürlüyü artırma/azaltma (Throttle)

### Uçuş Sona Çatdırma
1. Gas joystick-ini yavaş-yavaş aşağı endir
2. Dron yerə dəymədən əvvəl DISARM düyməsinə basıb saxla
3. Mavi LED sönsün

## Xətaların Həlli

| Problem | Həlli |
|---------|------|
| Pult siqnal göndərməyir | Serial Monitor-da "Pult hazır" mesajını yoxla |
| Joystick çevrilmiş | Kalibrasyon yenidən et |
| Motorlar işləmir | GND ortaq kabelini yoxla |
| LED yanmır | LED rezistorunun bağlantısını yoxla |
| Dron ARM olunmur | Gas tamamilə aşağıda olmalı |

## Təhlükəsizlik Xəbərdarlıqları

⚠️ **ÖNƏMLİ:**
- Pultun batareyasını həmişə yoxla
- Wireless bağlantı sındırılıbsa, dron avtomatik DISARM olmalı
- Propellərləri quraşdırmazdan əvvəl testlə
- Qoruma gözlüyü taxın
- Açıq sahədə sınaqla

---
**Versiya:** 1.0  
**Son yenilənmə:** 2026-09-04
