#include <OLED2.42_EASY.h>

void setup() {
    oledBegin();
    oledClear();

    // Заголовок страницы (Terminus 8×12)
    oledText("OLED 2.42 SSD1309", 7, 0);

    // Одно значение: температура
    // Заголовок: hx=0, hy=12, значение: vx=8, vy=32, язык: 0 (рус)
    oledTempFull(23.5, 0, 12, 8, 32, 0);

    oledShow();
}

void loop() {
    static uint32_t lastUpdate = 0;

    if (millis() - lastUpdate >= 2000) {
        lastUpdate = millis();

        // Обновление значения без координат
        oledTempUpdate(23.5 + (random(-10, 10) / 10.0));
    }
}
