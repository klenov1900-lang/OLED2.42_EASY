# 📺 OLED2.42_EASY

**语言:** [Русский](README.md) · 中文

[![Version](https://img.shields.io/badge/version-2.0.0-blue)](https://github.com/klenov1900-lang/OLED2.42_EASY/releases)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)
[![Arduino](https://img.shields.io/badge/Arduino-Nano%20%7C%20Uno%20%7C%20Pro%20Mini-teal)](https://www.arduino.cc/)
[![Platform](https://img.shields.io/badge/platform-AVR-orange)](https://www.microchip.com/)
[![Status](https://img.shields.io/badge/status-stable-brightgreen)](https://github.com/klenov1900-lang/OLED2.42_EASY)
[![Library Manager](https://img.shields.io/badge/Arduino%20Library%20Manager-approved-blue)](https://www.arduino.cc/reference/en/libraries/)

适用于 **2.42" OLED** 显示器（**SSD1309** 控制器，I2C，ATmega328P）的简易库。使用 `I2C_EASY` 作为底层 TWI 层。**不依赖 `Wire.h`。** 属于 `*_EASY` 系列。

## 🖼️ 演示

所有示例均在 [Wokwi](https://wokwi.com/) 模拟器和真实硬件 **Arduino Nano + OLED 2.42" (SSD1309, I2C)** 上测试通过。

| 时钟 (`Clock_With_Labels`) | 传感器 (`FullDemo`) | Terminus 字体 (`Text_SmallFont`) |
|:---:|:---:|:---:|
| ![Clock With Labels](images/Clock_With_Labels.png) | ![Full Demo](images/Demo.png) | ![Text SmallFont](images/Text_SmallFont.png) |

> 🧪 *库在 Wokwi 上基于 SSD1306 测试通过 —— 命令与 SSD1309 兼容，因此代码在真实硬件 **Arduino Nano + OLED 2.42" (SSD1309, I2C)** 上无需修改即可运行。*

## ✨ 特性

- ✅ **使用 `I2C_EASY`** — 不依赖 `Wire.h`
- ✅ **直接操作 TWI** — 仅使用 ATmega328P 寄存器
- ✅ **Framebuffer** — 1024 字节（8 页 × 128 字节）
- ✅ **Terminus 8×12** — 5 行 × 12 像素（西里尔字母 + Unicode，227 个字符）
- ✅ **Blocky 15×23** — 大号数字（高度 23 像素）
- ✅ **多语言标题** — 俄语、English、中文
- ✅ **局部刷新** — `oledShowPage()` — 仅刷新改变的页面
- ✅ **`*Full` + `*Update`** — 坐标只需设置一次
- ✅ **TWI 超时** — 显示器掉线不会卡死

> **注意：** 硬件 framebuffer 为 8 页（64 像素）。
> Terminus 8×12 字体高 12 像素，因此屏幕上可显示 **5 行**文本。
> Blocky 15×23 — 23 像素，占据 3 页 framebuffer。

## 🏗️ 架构

```
OLED2.42_EASY
    ↓
I2C_EASY          ← 底层 TWI 层
    ↓
ATmega328P TWI
```

**工作原理：**
- `OLED2.42_EASY` 生成 SSD1309 的命令和数据。
- `I2C_EASY` — 通过 I2C 总线传输。
- **直接操作 TWI 寄存器** — 不依赖 `Wire.h`。

**优势：**
- ✅ 整个 `*_EASY` 系列共用同一个 TWI 层。
- ✅ 与 `DS3231_EASY`、`PCFKeypad_EASY` 等无冲突。
- ✅ 超时 + 诊断 — 由 `I2C_EASY` 提供。
- ✅ 代码精简 — 只包含 SSD1309 逻辑。

## ⚙️ 要求与兼容性

- **微控制器：** ATmega328P（8 位 AVR）。
- **开发板：** Arduino Uno、Nano、Pro Mini。
- **显示器：** OLED 2.42"，控制器 SSD1309。
- **接口：** I2C（默认地址 0x3C）。
- **依赖：** [`I2C_EASY`](https://github.com/klenov1900-lang/I2C_EASY)。
- **其他平台：** ESP8266、ESP32、STM32 — **不支持**。

## 📦 安装

### 方式 1：Arduino IDE 库管理器

1. 打开 Arduino IDE
2. 进入 **项目 → 加载库 → 管理库…**
3. 搜索 `OLED2.42_EASY`
4. 点击 **安装**

> **注意：** `I2C_EASY` 将作为依赖自动安装。

### 方式 2：手动（ZIP）

1. 下载 [最新版本](https://github.com/klenov1900-lang/OLED2.42_EASY/archive/refs/heads/main.zip) 的 ZIP 压缩包
2. Arduino IDE：**项目 → 加载库 → 添加 .ZIP 库…**
3. 选择下载的压缩包

> **注意：** 还需要手动安装 [`I2C_EASY`](https://github.com/klenov1900-lang/I2C_EASY)。

### 方式 3：通过 Git

```bash
cd ~/Arduino/libraries/
git clone https://github.com/klenov1900-lang/I2C_EASY.git
git clone https://github.com/klenov1900-lang/OLED2.42_EASY.git
```

## 🚀 快速开始

```c
#include <I2C_EASY.h>
#include <OLED2.42_EASY.h>

void setup() {
    // 1. 初始化 I2C
    i2c_init(400000);

    // 2. 初始化 OLED
    oledBegin();
    oledClear();

    // 3. 输出文本（Terminus 8×12）
    oledText("OLED 2.42\"", 0, 0);
    oledText("SSD1309 I2C", 0, 1);

    // 4. 输出数值（Blocky 15×23）
    oledTempFull(23.5, 0, 24, 0, 40, 0);   // 23.5 °C，俄语

    // 5. 发送到显示器
    oledShow();
}

void loop() {}
```

## 📖 示例

| 示例 | 描述 | 依赖 |
|---|---|---|
| **Text_SmallFont** | 使用 Terminus 8×12 字体 | `OLED2.42_EASY` |
| **FullDemo** | 演示 `*Full` + `*Update` | `OLED2.42_EASY` |
| **Clock_With_Labels** | 带标题的时钟（ru/en/cn） | `OLED2.42_EASY` |

## 📚 API

### 初始化

| 函数 | 描述 |
|---|---|
| `oledBegin()` | 初始化（默认地址 0x3C） |
| `oledBeginAddr(addr)` | 使用其他地址初始化 |

### 显示器控制

| 函数 | 描述 |
|---|---|
| `oledOn()` / `oledOff()` | 开 / 关 |
| `oledContrast(v)` | 对比度（0..255） |
| `oledInvert(e)` | 反色（0/1） |

### Framebuffer

| 函数 | 描述 |
|---|---|
| `oledClear()` | 清空整个缓冲区 |
| `oledClearPage(p)` | 清空第 0..7 页 |
| `oledShow()` | 发送整个缓冲区 |
| `oledShowPage(p)` | 发送第 0..7 页 |

### 图形

| 函数 | 描述 |
|---|---|
| `oledPixel(x, y)` | 像素 |
| `oledLine(x0, y0, x1, y1)` | 线 |
| `oledRect(x, y, w, h)` | 矩形 |
| `oledFillRect(x, y, w, h)` | 填充矩形 |
| `oledCircle(x, y, r)` | 圆 |
| `oledFillCircle(x, y, r)` | 填充圆 |
| `oledBitmap(bmp, x, y, w, h)` | 位图 |
| `oledInvertRect(x, y, w, h)` | 区域反色 |

### 文本

| 函数 | 描述 |
|---|---|
| `oledText(str, x, line)` | Terminus 8×12，第 0..4 行 |
| `oledValue(str, x, pos)` | Blocky，pos = 1/2 |
| `oledValueXY(str, x, y)` | Blocky，像素 Y |

### 带单位的值（pos = 1/2）

| 函数 | 格式 |
|---|---|
| `oledTemp(t, x, pos)` | `+23.5°C` |
| `oledHumid(h, x, pos)` | `67%` |
| `oledPressure(p, x, pos)` | `1013hPa` |
| `oledVolt(v, x, pos)` | `+3.1U` |
| `oledAmper(a, x, pos)` | `+1.2A` |
| `oledTime(h, m, s, x, pos)` | `12:34:56` |
| `oledDate(d, m, y, x, pos)` | `25.09.26` |

### 带像素坐标的值

| 函数 | 格式 |
|---|---|
| `oledTempXY(t, x, y)` | `+23.5°C` |
| `oledHumidXY(h, x, y)` | `67%` |
| `oledPressXY(p, x, y)` | `1013hPa` |
| `oledVoltXY(v, x, y)` | `+3.1U` |
| `oledAmperXY(a, x, y)` | `+1.2A` |
| `oledTimeXY(h, m, s, x, y)` | `12:34:56` |
| `oledDateXY(d, m, y, x, y)` | `25.09.26` |

### 完整函数（`*Full`）

| 函数 | 参数 |
|---|---|
| `oledTempFull(t, hx, hy, vx, vy, lang)` | 6 |
| `oledHumidFull(h, hx, hy, vx, vy, lang)` | 6 |
| `oledPressFull(p, hx, hy, vx, vy, lang)` | 6 |
| `oledVoltFull(v, hx, hy, vx, vy, lang)` | 6 |
| `oledAmperFull(a, hx, hy, vx, vy, lang)` | 6 |
| `oledTimeFull(h, m, s, hx, hy, vx, vy, lang)` | 8 |
| `oledDateFull(d, m, y, hx, hy, vx, vy, lang)` | 8 |

### UPDATE 函数（`*Update`）

| 函数 | 描述 |
|---|---|
| `oledTempUpdate(t)` | 更新温度 |
| `oledHumidUpdate(h)` | 更新湿度 |
| `oledPressUpdate(p)` | 更新气压 |
| `oledVoltUpdate(v)` | 更新电压 |
| `oledAmperUpdate(a)` | 更新电流 |
| `oledTimeUpdate(h, m, s)` | 更新时间 |
| `oledDateUpdate(d, m, y)` | 更新日期 |

## 📊 错误码

| 代码 | 常量 | 描述 |
|---|---|---|
| 0 | `I2C_OK` | 成功 |
| 1 | `I2C_ERR_TIMEOUT` | TWINT 超时 |
| 2 | `I2C_ERR_START` | START 错误 |
| 3 | `I2C_ERR_ADDR_NACK` | 设备无应答 |
| 4 | `I2C_ERR_DATA_NACK` | 数据 NACK |
| 5 | `I2C_ERR_BUS` | 总线状态错误 |
| 6 | `I2C_ERR_ARB_LOST` | 仲裁丢失 |
| 7 | `I2C_ERR_BUS_LOCKED` | SDA 被拉低 |

## 🔤 字体

| 字体 | 尺寸 | 用途 | 西里尔字母 |
|---|---|---|---|
| 🔤 **Terminus 8×12** | 8×12 | 文本、标题 | ✅ |
| 🔢 **Blocky 15×23** | 15×23 | 数值（数字） | ❌ |

## 💾 资源占用

| 资源 | 已用 | 剩余 |
|---|---|---|
| ⚡ **Flash** | ~7.8 КБ (25%) | ~23 КБ |
| 💾 **RAM** | ~1.1 КБ (52%) | ~0.9 КБ |

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
├── images/
│   ├── Clock_With_Labels.png
│   ├── Demo.png
│   └── Text_SmallFont.png
├── library.properties
├── keywords.txt
├── README.md
├── README_CN.md
└── LICENSE
```

## 📝 版本历史

| 版本 | 日期 | 变更 |
|---|---|---|
| **v2.0.0** | 2026-10-16 | 迁移到 `I2C_EASY`。移除对 `util/twi.h` 的依赖。 |
| v1.0.1 | 2026-10-05 | 新增示例和演示。 |
| v1.0.0 | 2026-10-04 | 首次发布。 |

## 📄 许可证

MIT License。参见 [LICENSE](LICENSE)。

## 🙏 Credits

| 组件 | 作者 | 许可证 |
|---|---|---|
| **Blocky 15×23** | klenov1900-lang | MIT |
| **中文标题** (`labels.h`) | klenov1900-lang | MIT |
| **Terminus 8×12**（改编） | klenov1900-lang | MIT |
| *原始 Terminus* | Dimitar Zhekov | SIL OFL 1.1 |

> **注意：** Terminus 8×12 字体经过 Python 脚本两次处理
>（转换和优化字形表）。最终字体保留了 Terminus 的风格和几何形状，
>但是由本库作者准备的改编版本。

## 📧 联系方式

- **Email:** gray_wolf19@mail.ru
- **GitHub:** [klenov1900-lang](https://github.com/klenov1900-lang)

---

⭐ 如果这个库对您有用，请在 GitHub 上点个星！⭐
