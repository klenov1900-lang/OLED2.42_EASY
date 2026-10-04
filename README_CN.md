# 📺 OLED2.42_EASY

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![License](https://img.shields.io/badge/license-MIT-green)
![Arduino](https://img.shields.io/badge/Arduino-Nano%20%7C%20Uno%20%7C%20Pro%20Mini-teal)
![Platform](https://img.shields.io/badge/platform-AVR-orange)
![Status](https://img.shields.io/badge/status-stable-brightgreen)

**语言:** [Русский](README.md) · [中文](README_CN.md)

---

适用于 **OLED 2.42"**（控制器 **SSD1309**，I2C，ATmega328P）的简易库。

直接操作 **TWI** 寄存器。无外部依赖。

---

## 📦 安装

### 1️⃣ 通过 Arduino IDE 库管理器（推荐）

1. 打开 Arduino IDE
2. 转到 **Sketch（草图） → Include Library（包含库） → Manage Libraries（管理库）...**
3. 在搜索框中输入 `OLED2.42_EASY`
4. 点击 **Install（安装）**

### 2️⃣ 手动安装（通过 ZIP）

1. 从 [最新版本](https://github.com/klenov1900-lang/OLED2.42_EASY/releases/latest) 下载 ZIP 压缩包
2. 在 Arduino IDE 中：**Sketch（草图） → Include Library（包含库） → Add .ZIP Library...（添加 .ZIP 库）...**
3. 选择下载的压缩包

### 3️⃣ 通过 Git

```bash
cd ~/Arduino/libraries/
git clone https://github.com/klenov1900-lang/OLED2.42_EASY.git
```

---

## ✨ 特点

| 功能 | 描述 |
|---|---|
| 🔌 **直接操作 TWI** | 无需 `Wire.h`，仅使用 ATmega328P 寄存器 |
| 💾 **Framebuffer** | 1024 字节（8 页，每页 128 字节） |
| 🔤 **Terminus 8×12** | 支持西里尔字母 + Unicode（227 个字符） |
| 🔢 **Blocky 15×23** | 数字、符号、`hPa`、`%`、`°C`、`U`、`A` |
| 🌐 **多语言** | 标题支持：俄语、English、中文 |
| ⚡ **局部刷新** | `oledShowPage()` — 仅发送已修改的页面 |
| 🎯 **`*Full` + `*Update`** | 坐标仅设置一次，更新时无需重复 |
| ⏱️ **TWI 超时** | 屏幕断开时不会卡死 |

---

## 🔌 接线

| OLED | Arduino Nano |
|---|---|
| **VCC** | 5V |
| **GND** | GND |
| **SCL** | A5 |
| **SDA** | A4 |

> ✅ 模块内置 **5V → 3.3V 稳压器**，因此直接使用 Arduino **5V** 供电是**安全的**。

---

## 🚀 快速上手

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

## 📖 三种输出数值的方式

### 1️⃣ 简单 — 固定 Y (`pos = 1/2`)

```c
oledTemp(23.5, 0, 1);   // Y = 8
oledHumid(67, 0, 2);    // Y = 40
oledShow();
```

| `pos` | Y |
|---|---|
| `1` | 8 |
| `2` | 40 |

### 2️⃣ 灵活 — 任意像素 Y

```c
oledTempXY(23.5, 0, 16);   // Y = 16
oledHumidXY(67, 0, 40);    // Y = 40
oledShow();
```

### 3️⃣ 带标题 — `*Full` 记住坐标

```c
// 标题 Y = 0，数值 Y = 16，语言 = 0 (俄文)
oledTempFull(23.5, 0, 0, 0, 16, 0);
oledShow();

// 更新 — 无需坐标
oledTempUpdate(23.6);
```

---

## 🌐 语言

| 代码 | 语言 |
|---|---|
| `0` | 🇷🇺 俄文 |
| `1` | 🇬🇧 English |
| `2` | 🇨🇳 中文 |

```c
oledTempFull(23.5, 0, 0, 0, 16, 0);   // 俄文
oledTempFull(23.5, 0, 0, 0, 16, 1);   // English
oledTempFull(23.5, 0, 0, 0, 16, 2);   // 中文
```

---

## 📋 完整函数列表

### ⚙️ 初始化

| 函数 | 描述 |
|---|---|
| `oledBegin()` | 初始化 |
| `oledBeginAddr(addr)` | 使用其他地址初始化 |

### 🖥️ 显示控制

| 函数 | 描述 |
|---|---|
| `oledOn()` / `oledOff()` | 开 / 关 |
| `oledContrast(v)` | 对比度 (0..255) |
| `oledInvert(e)` | 反转 (0/1) |

### 💾 Framebuffer

| 函数 | 描述 |
|---|---|
| `oledClear()` | 清空整个缓冲区 |
| `oledClearPage(p)` | 清空页面 0..7 |
| `oledShow()` | 发送整个缓冲区 |
| `oledShowPage(p)` | 发送页面 0..7 |

### 🎨 图形

| 函数 | 描述 |
|---|---|
| `oledPixel(x, y)` | 像素 |
| `oledLine(x0, y0, x1, y1)` | 线 |
| `oledRect(x, y, w, h)` | 矩形 |
| `oledFillRect(x, y, w, h)` | 填充矩形 |
| `oledCircle(x, y, r)` | 圆 |
| `oledFillCircle(x, y, r)` | 填充圆 |
| `oledBitmap(bmp, x, y, w, h)` | 位图 |
| `oledInvertRect(x, y, w, h)` | 区域反转 |

### ✍️ 文本

| 函数 | 描述 |
|---|---|
| `oledText(str, x, line)` | Terminus 8×12，行 0..4 |
| `oledValue(str, x, pos)` | Blocky，pos = 1/2 |
| `oledValueXY(str, x, y)` | Blocky，像素 Y |

### 📊 带单位的值 (`pos = 1/2`)

| 函数 | 格式 |
|---|---|
| `oledTemp(t, x, pos)` | `+23.5°C` |
| `oledHumid(h, x, pos)` | `67%` |
| `oledPressure(p, x, pos)` | `1013hPa` |
| `oledVolt(v, x, pos)` | `+3.1U` |
| `oledAmper(a, x, pos)` | `+1.2A` |
| `oledTime(h, m, s, x, pos)` | `12:34:56` |
| `oledDate(d, m, y, x, pos)` | `25.09.26` |

### 📐 带像素坐标的值

| 函数 | 格式 |
|---|---|
| `oledTempXY(t, x, y)` | `+23.5°C` |
| `oledHumidXY(h, x, y)` | `67%` |
| `oledPressXY(p, x, y)` | `1013hPa` |
| `oledVoltXY(v, x, y)` | `+3.1U` |
| `oledAmperXY(a, x, y)` | `+1.2A` |
| `oledTimeXY(h, m, s, x, y)` | `12:34:56` |
| `oledDateXY(d, m, y, x, y)` | `25.09.26` |

### 🎯 完整函数 (`*Full`)

| 函数 | 参数 |
|---|---|
| `oledTempFull(t, hx, hy, vx, vy, lang)` | 6 |
| `oledHumidFull(h, hx, hy, vx, vy, lang)` | 6 |
| `oledPressFull(p, hx, hy, vx, vy, lang)` | 6 |
| `oledVoltFull(v, hx, hy, vx, vy, lang)` | 6 |
| `oledAmperFull(a, hx, hy, vx, vy, lang)` | 6 |
| `oledTimeFull(h, m, s, hx, hy, vx, vy, lang)` | 8 |
| `oledDateFull(d, m, y, hx, hy, vx, vy, lang)` | 8 |

### 🔄 UPDATE 函数 (`*Update`)

| 函数 | 描述 |
|---|---|
| `oledTempUpdate(t)` | 更新温度 |
| `oledHumidUpdate(h)` | 更新湿度 |
| `oledPressUpdate(p)` | 更新压力 |
| `oledVoltUpdate(v)` | 更新电压 |
| `oledAmperUpdate(a)` | 更新电流 |
| `oledTimeUpdate(h, m, s)` | 更新时间 |
| `oledDateUpdate(d, m, y)` | 更新日期 |

---

## 🔤 字体

| 字体 | 大小 | 用途 | 西里尔字母 |
|---|---|---|---|
| 🔤 **Terminus 8×12** | 8×12 | 文本、标题 | ✅ |
| 🔢 **Blocky 15×23** | 15×23 | 数值（数字） | ❌ |

---

## 💾 资源占用

| 资源 | 已占用 | 可用 |
|---|---|---|
| ⚡ **Flash** | ~7.8 КБ (25%) | ~23 КБ |
| 💾 **ОЗУ** | ~1.1 КБ (52%) | ~0.9 КБ |

---

## 📂 结构

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
├── README_CN.md
└── LICENSE
```

---

## 📜 许可证

**MIT** — 可自由用于商业和非商业用途。

---

## 🙏 Credits

### 🔢 Blocky 15×23

**作者:** klenov1900-lang
**许可证:** MIT

### 🈶 中文标题 (`labels.h`)

**作者:** klenov1900-lang
**许可证:** MIT

### 🔤 Terminus 8×12

**作者:** Dimitar Zhekov
**许可证:** SIL Open Font License 1.1
**来源:** [files.ax86.net/terminus-ttf](https://files.ax86.net/terminus-ttf/)
