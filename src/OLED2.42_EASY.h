// ============================================================
// OLED2.42_EASY.h
// Библиотека для OLED 2.42" на SSD1309 (I2C, ATmega328P)
// ============================================================
#ifndef OLED2_42_EASY_H
#define OLED2_42_EASY_H

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <string.h>

#include "fonts.h"
#include "labels.h"  

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================
// КОНСТАНТЫ
// ============================================================
#define O242_WIDTH       128
#define O242_HEIGHT      64
#define O242_ADDR        0x3C
#define O242_TWI_FREQ    400000UL

// ============================================================
// ИНИЦИАЛИЗАЦИЯ
// ============================================================
void o242_twi_init(void);
void o242_init(void);
void o242_init_addr(uint8_t addr);

// ============================================================
// УПРАВЛЕНИЕ ДИСПЛЕЕМ
// ============================================================
void o242_display_on(void);
void o242_display_off(void);
void o242_set_contrast(uint8_t value);
void o242_invert(uint8_t enable);

// ============================================================
// FRAMEBUFFER
// ============================================================
void o242_fb_clear(void);
void o242_fb_clear_page(uint8_t page);
void o242_fb_flush(void);
void o242_fb_flush_page(uint8_t page);

// ============================================================
// ГРАФИКА
// ============================================================
void o242_draw_pixel(int16_t x, int16_t y);
void o242_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
void o242_draw_rect(int16_t x, int16_t y, int16_t w, int16_t h);
void o242_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h);
void o242_draw_circle(int16_t x0, int16_t y0, int16_t r);
void o242_fill_circle(int16_t x0, int16_t y0, int16_t r);
void o242_draw_bitmap(const uint8_t* bitmap, int16_t x, int16_t y, int16_t w, int16_t h);
void o242_invert_rect(int16_t x, int16_t y, int16_t w, int16_t h);

// ============================================================
// ТЕКСТ
// ============================================================
void o242_set_font(const o242_font_t* font);
uint8_t o242_draw_char(char c, int16_t x, int16_t y);
uint8_t o242_draw_char_unicode(uint16_t code, int16_t x, int16_t y);
uint8_t o242_draw_string(const char* str, int16_t x, int16_t y);
uint8_t o242_draw_string_utf8(const char* str, int16_t x, int16_t y);

uint8_t o242_draw_char_line(char c, int16_t x, uint8_t line);
uint8_t o242_draw_string_line(const char* str, int16_t x, uint8_t line);
uint8_t o242_draw_string_utf8_line(const char* str, int16_t x, uint8_t line);

// ============================================================
// СПЕЦИАЛИЗИРОВАННЫЕ
// ============================================================
void o242_draw_hh_mm(uint8_t hh, uint8_t mm, int16_t x, int16_t y);
void o242_draw_hh_mm_ss(uint8_t hh, uint8_t mm, uint8_t ss, int16_t x, int16_t y);

// ============================================================
// ОБЁРТКИ ДЛЯ КОНЕЧНОГО ПОЛЬЗОВАТЕЛЯ
// ============================================================

// --- Инициализация ---
static inline void oledBegin(void)            { o242_init(); }
static inline void oledBeginAddr(uint8_t a)   { o242_init_addr(a); }

// --- Управление дисплеем ---
static inline void oledOn(void)               { o242_display_on(); }
static inline void oledOff(void)              { o242_display_off(); }
static inline void oledContrast(uint8_t v)    { o242_set_contrast(v); }
static inline void oledInvert(uint8_t e)      { o242_invert(e); }

// --- Framebuffer ---
static inline void oledClear(void)            { o242_fb_clear(); }
static inline void oledClearPage(uint8_t p)   { o242_fb_clear_page(p); }
static inline void oledShow(void)             { o242_fb_flush(); }
static inline void oledShowPage(uint8_t p)    { o242_fb_flush_page(p); }

// --- Графика ---
static inline void oledPixel(int16_t x, int16_t y) { o242_draw_pixel(x, y); }
static inline void oledLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1) { o242_draw_line(x0, y0, x1, y1); }
static inline void oledRect(int16_t x, int16_t y, int16_t w, int16_t h) { o242_draw_rect(x, y, w, h); }
static inline void oledFillRect(int16_t x, int16_t y, int16_t w, int16_t h) { o242_fill_rect(x, y, w, h); }
static inline void oledCircle(int16_t x, int16_t y, int16_t r) { o242_draw_circle(x, y, r); }
static inline void oledFillCircle(int16_t x, int16_t y, int16_t r) { o242_fill_circle(x, y, r); }
static inline void oledBitmap(const uint8_t* b, int16_t x, int16_t y, int16_t w, int16_t h) { o242_draw_bitmap(b, x, y, w, h); }
static inline void oledInvertRect(int16_t x, int16_t y, int16_t w, int16_t h) { o242_invert_rect(x, y, w, h); }

// --- Текст ---
void oledText(const char* str, int16_t x, uint8_t line);
void oledValue(const char* str, int16_t x, uint8_t pos);

// --- Значения с единицами (pos = 1/2) ---
void oledTemp(float temp, int16_t x, uint8_t pos);
void oledAmper(float amp, int16_t x, uint8_t pos);
void oledVolt(float volt, int16_t x, uint8_t pos);
void oledPressure(uint16_t press, int16_t x, uint8_t pos);
void oledHumid(uint8_t humid, int16_t x, uint8_t pos);
void oledTime(uint8_t hh, uint8_t mm, uint8_t ss, int16_t x, uint8_t pos);
void oledDate(uint8_t dd, uint8_t mm, uint8_t yy, int16_t x, uint8_t pos);

// --- Значения с пиксельными координатами (только значение) ---
void oledValueXY(const char* str, int16_t x, int16_t y);
void oledTempXY(float temp, int16_t x, int16_t y);
void oledHumidXY(uint8_t humid, int16_t x, int16_t y);
void oledPressXY(uint16_t press, int16_t x, int16_t y);
void oledVoltXY(float volt, int16_t x, int16_t y);
void oledAmperXY(float amp, int16_t x, int16_t y);
void oledTimeXY(uint8_t hh, uint8_t mm, uint8_t ss, int16_t x, int16_t y);
void oledDateXY(uint8_t dd, uint8_t mm, uint8_t yy, int16_t x, int16_t y);

// --- Полные функции (запоминают координаты) ---
void oledTempFull(float temp, int16_t hx, int16_t hy, int16_t vx, int16_t vy, uint8_t lang);
void oledHumidFull(uint8_t humid, int16_t hx, int16_t hy, int16_t vx, int16_t vy, uint8_t lang);
void oledPressFull(uint16_t press, int16_t hx, int16_t hy, int16_t vx, int16_t vy, uint8_t lang);
void oledVoltFull(float volt, int16_t hx, int16_t hy, int16_t vx, int16_t vy, uint8_t lang);
void oledAmperFull(float amp, int16_t hx, int16_t hy, int16_t vx, int16_t vy, uint8_t lang);
void oledTimeFull(uint8_t hh, uint8_t mm, uint8_t ss, int16_t hx, int16_t hy, int16_t vx, int16_t vy, uint8_t lang);
void oledDateFull(uint8_t dd, uint8_t mm, uint8_t yy, int16_t hx, int16_t hy, int16_t vx, int16_t vy, uint8_t lang);

// --- UPDATE-функции (используют сохранённые координаты) ---
void oledTempUpdate(float temp);
void oledHumidUpdate(uint8_t humid);
void oledPressUpdate(uint16_t press);
void oledVoltUpdate(float volt);
void oledAmperUpdate(float amp);
void oledTimeUpdate(uint8_t hh, uint8_t mm, uint8_t ss);
void oledDateUpdate(uint8_t dd, uint8_t mm, uint8_t yy);

#ifdef __cplusplus
}
#endif

#endif // OLED2_42_EASY_H