#include <OLED2.42_EASY.h>


void setup() {
    oledBegin();
    oledClear();
    oledText("OLED 2.42 SSD1309",  7, 0);
    // Задаём координаты ОДИН РАЗ
    // Заголовок: hx=0, hy=0, значение: vx=8, vy=32, язык: 2 (中文)
    oledTimeFull(0, 0, 0, 0, 12, 8, 32, 2);
    
    oledShow();
}

void loop() {
    static uint8_t hh = 0, mm = 0, ss = 0;
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

        // БЕЗ координат — они уже сохранены
        oledTimeUpdate(hh, mm, ss);
    }
}
