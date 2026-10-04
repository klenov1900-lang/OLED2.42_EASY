# 📺 OLED2.42_EASY

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![License](https://img.shields.io/badge/license-MIT-green)
![Arduino](https://img.shields.io/badge/Arduino-Nano%20%7C%20Uno%20%7C%20Pro%20Mini-teal)
![Platform](https://img.shields.io/badge/platform-AVR-orange)
![Status](https://img.shields.io/badge/status-stable-brightgreen)

**Языки:** [Русский](README.md) · [中文](README_CN.md)

---

Простая библиотека для **OLED 2.42"** на контроллере **SSD1309** (I2C, ATmega328P).

Прямая работа с регистрами **TWI**. Без внешних зависимостей.

---

## 📦 Установка

### 1️⃣ Через менеджер библиотек Arduino IDE (рекомендуется)

1. Откройте Arduino IDE
2. Перейдите в **Скетч → Подключить библиотеку → Управлять библиотеками...**
3. В поиске введите `OLED2.42_EASY`
4. Нажмите **Установить**

### 2️⃣ Вручную (через ZIP)

1. Скачайте ZIP-архив с [последней версией](https://github.com/klenov1900-lang/OLED2.42_EASY/releases/latest)
2. В Arduino IDE: **Скетч → Подключить библиотеку → Добавить .ZIP библиотеку...**
3. Выберите скачанный архив

### 3️⃣ Через Git

```bash
cd ~/Arduino/libraries/
git clone https://github.com/klenov1900-lang/OLED2.42_EASY.git
```

---

## ✨ Особенности

| Возможность | Описание |
|---|---|
| 🔌 **Прямая работа с TWI** | Без `Wire.h`, только регистры ATmega328P |
| 💾 **Framebuffer** | 1024 байта (8 страниц по 128 байт) |
| 🔤 **Terminus 8×12** | Кириллица + Unicode (227 символов) |
| 🔢 **Blocky 15×23** | Цифры, знаки, `hPa`, `%`, `°C`, `U`, `A` |
| 🌐 **Мультиязычность** | Заголовки: русский, English, 中文 |
| ⚡ **Частичное обновление** | `oledShowPage()` — только изменённые страницы |
| 🎯 **`*Full` + `*Update`** | Координаты задаются один раз, обновление — без них |
| ⏱️ **Timeout в TWI** | Не зависает при отвале дисплея |

---

## 🔌 Подключение

| OLED | Arduino Nano |
|---|---|
| **VCC** | 5V |
| **GND** | GND |
| **SCL** | A5 |
| **SDA** | A4 |

> ✅ На модуле **встроенный стабилизатор 5V → 3.3V**, поэтому питание от **5V** Arduino **безопасно**.

---

## 🚀 Быстрый старт

```c
#include <OLED2.42_EASY.h>

void setup() {
    oledBegin();
    oledClear();

    oledTimeFull(12, 34, 56, 0, 0, 0, 16, 0);
    oledShow();
}

void loop() {
    static uint8_t hh = 12, mm = 34, ss = 56;
    static uint32_t lastUpdate = 0;
    static uint8_t firstRun = 1;

    if (firstRun || (millis() - lastUpdate >= 1000)) {
        lastUpdate = millis();
        firstRun = 0;

        if (++ss >= 60) {
            ss = 0;
            if (++mm >= 60) {
                mm = 0;
                if (++hh >= 24) hh = 0;
            }
        }

        oledTimeUpdate(hh, mm, ss);
    }
}
```

---

## 📖 Три способа вывода значения

### 1️⃣ Просто — фиксированные Y (`pos = 1/2`)

```c
oledTemp(23.5, 0, 1);   // Y = 8
oledHumid(67, 0, 2);    // Y = 40
oledShow();
```

| `pos` | Y |
|---|---|
| `1` | 8 |
| `2` | 40 |

### 2️⃣ Гибко — любая пиксельная Y

```c
oledTempXY(23.5, 0, 16);   // Y = 16
oledHumidXY(67, 0, 40);    // Y = 40
oledShow();
```

### 3️⃣ С заголовком — `*Full` запоминает координаты

```c
// Заголовок Y = 0, значение Y = 16, язык = 0 (рус)
oledTempFull(23.5, 0, 0, 0, 16, 0);
oledShow();

// Обновление — БЕЗ координат
oledTempUpdate(23.6);
```

---

## 🌐 Языки

| Код | Язык |
|---|---|
| `0` | 🇷🇺 Русский |
| `1` | 🇬🇧 English |
| `2` | 🇨🇳 中文 |

```c
oledTempFull(23.5, 0, 0, 0, 16, 0);   // русский
oledTempFull(23.5, 0, 0, 0, 16, 1);   // English
oledTempFull(23.5, 0, 0, 0, 16, 2);   // 中文
```

---

## 📋 Полный список функций

### ⚙️ Инициализация

| Функция | Описание |
|---|---|
| `oledBegin()` | Инициализация |
| `oledBeginAddr(addr)` | Инициализация с другим адресом |

### 🖥️ Управление дисплеем

| Функция | Описание |
|---|---|
| `oledOn()` / `oledOff()` | Вкл / выкл |
| `oledContrast(v)` | Контраст (0..255) |
| `oledInvert(e)` | Инверсия (0/1) |

### 💾 Framebuffer

| Функция | Описание |
|---|---|
| `oledClear()` | Очистка всего буфера |
| `oledClearPage(p)` | Очистка страницы 0..7 |
| `oledShow()` | Отправка всего буфера |
| `oledShowPage(p)` | Отправка страницы 0..7 |

### 🎨 Графика

| Функция | Описание |
|---|---|
| `oledPixel(x, y)` | Пиксель |
| `oledLine(x0, y0, x1, y1)` | Линия |
| `oledRect(x, y, w, h)` | Прямоугольник |
| `oledFillRect(x, y, w, h)` | Залитый прямоугольник |
| `oledCircle(x, y, r)` | Круг |
| `oledFillCircle(x, y, r)` | Залитый круг |
| `oledBitmap(bmp, x, y, w, h)` | Битмап |
| `oledInvertRect(x, y, w, h)` | Инверсия области |

### ✍️ Текст

| Функция | Описание |
|---|---|
| `oledText(str, x, line)` | Terminus 8×12, строка 0..4 |
| `oledValue(str, x, pos)` | Blocky, pos = 1/2 |
| `oledValueXY(str, x, y)` | Blocky, пиксельная Y |

### 📊 Значения с единицами (`pos = 1/2`)

| Функция | Формат |
|---|---|
| `oledTemp(t, x, pos)` | `+23.5°C` |
| `oledHumid(h, x, pos)` | `67%` |
| `oledPressure(p, x, pos)` | `1013hPa` |
| `oledVolt(v, x, pos)` | `+3.1U` |
| `oledAmper(a, x, pos)` | `+1.2A` |
| `oledTime(h, m, s, x, pos)` | `12:34:56` |
| `oledDate(d, m, y, x, pos)` | `25.09.26` |

### 📐 Значения с пиксельными координатами

| Функция | Формат |
|---|---|
| `oledTempXY(t, x, y)` | `+23.5°C` |
| `oledHumidXY(h, x, y)` | `67%` |
| `oledPressXY(p, x, y)` | `1013hPa` |
| `oledVoltXY(v, x, y)` | `+3.1U` |
| `oledAmperXY(a, x, y)` | `+1.2A` |
| `oledTimeXY(h, m, s, x, y)` | `12:34:56` |
| `oledDateXY(d, m, y, x, y)` | `25.09.26` |

### 🎯 Полные функции (`*Full`)

| Функция | Параметры |
|---|---|
| `oledTempFull(t, hx, hy, vx, vy, lang)` | 6 |
| `oledHumidFull(h, hx, hy, vx, vy, lang)` | 6 |
| `oledPressFull(p, hx, hy, vx, vy, lang)` | 6 |
| `oledVoltFull(v, hx, hy, vx, vy, lang)` | 6 |
| `oledAmperFull(a, hx, hy, vx, vy, lang)` | 6 |
| `oledTimeFull(h, m, s, hx, hy, vx, vy, lang)` | 8 |
| `oledDateFull(d, m, y, hx, hy, vx, vy, lang)` | 8 |

### 🔄 UPDATE-функции (`*Update`)

| Функция | Описание |
|---|---|
| `oledTempUpdate(t)` | Обновление температуры |
| `oledHumidUpdate(h)` | Обновление влажности |
| `oledPressUpdate(p)` | Обновление давления |
| `oledVoltUpdate(v)` | Обновление напряжения |
| `oledAmperUpdate(a)` | Обновление тока |
| `oledTimeUpdate(h, m, s)` | Обновление времени |
| `oledDateUpdate(d, m, y)` | Обновление даты |

---

## 🔤 Шрифты

| Шрифт | Размер | Назначение | Кириллица |
|---|---|---|---|
| 🔤 **Terminus 8×12** | 8×12 | Текст, заголовки | ✅ |
| 🔢 **Blocky 15×23** | 15×23 | Значения (цифры) | ❌ |

---

## 💾 Потребление ресурсов

| Ресурс | Занято | Свободно |
|---|---|---|
| ⚡ **Flash** | ~7.8 КБ (25%) | ~23 КБ |
| 💾 **ОЗУ** | ~1.1 КБ (52%) | ~0.9 КБ |

---

## 📂 Структура

```
OLED2.42_EASY/
├── src/
│   ├── OLED2.42_EASY.h
│   ├── OLED2.42_EASY.cpp
│   ├── fonts.h
│   ├── blocky_font.cpp
│   ├── fonts_terminus_8x12.c
│   └── labels.h
├── examples/
│   ├── Clock_With_Labels/
│   ├── FullDemo/
│   └── Text_SmallFont/
├── library.properties
├── keywords.txt
├── README.md
└── LICENSE
```

---

## 📜 Лицензия

**MIT** — свободно для коммерческого и некоммерческого использования.

---

## 🙏 Credits

### 🔢 Blocky 15×23

**Автор:** klenov1900-lang
**Лицензия:** MIT

### 🈶 中文 заголовки (`labels.h`)

**Автор:** klenov1900-lang
**Лицензия:** MIT

### 🔤 Terminus 8×12

**Автор:** Dimitar Zhekov
**Лицензия:** SIL Open Font License 1.1
**Источник:** [files.ax86.net/terminus-ttf](https://files.ax86.net/terminus-ttf/)
