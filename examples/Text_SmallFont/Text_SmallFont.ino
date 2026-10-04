#include <OLED2.42_EASY.h>

void setup() {
    oledBegin();
    oledClear();

    // Terminus 8×12, строки 0..4
    oledText("Terminus 8x12",      0, 0);
    oledText("Кириллица: Привет!", 0, 1);
    oledText("Unicode: °C, %, hPa", 0, 2);
    oledText("English: Hello!",    0, 3);
    oledText("Symbols: +-*/=<>",   0, 4);

    oledShow();
}

void loop() {
    // Статичный текст — ничего не делаем
}
