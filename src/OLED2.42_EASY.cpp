// ============================================================
// OLED2.42_EASY.cpp
// Реализация библиотеки. Прямая работа с TWI на ATmega328P.
// ============================================================
#include "OLED2.42_EASY.h"
#include "fonts.h"
#include "labels.h"
#include <util/twi.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

// ============================================================
// ВНУТРЕННИЕ ПЕРЕМЕННЫЕ
// ============================================================
static uint8_t o242_address = O242_ADDR;
static uint8_t o242_framebuffer[O242_WIDTH * 8];
static const o242_font_t* o242_current_font = 0;

// ============================================================
// СОХРАНЁННЫЕ КООРДИНАТЫ ДЛЯ UPDATE
// ============================================================
typedef struct {
    int16_t vx, vy;
    uint8_t active;
} o242_update_pos_t;

static o242_update_pos_t o242_pos_temp  = {0, 0, 0};
static o242_update_pos_t o242_pos_humid = {0, 0, 0};
static o242_update_pos_t o242_pos_press = {0, 0, 0};
static o242_update_pos_t o242_pos_volt  = {0, 0, 0};
static o242_update_pos_t o242_pos_amper = {0, 0, 0};
static o242_update_pos_t o242_pos_time  = {0, 0, 0};
static o242_update_pos_t o242_pos_date  = {0, 0, 0};

// ============================================================
// ВЫБОР ЗАГОЛОВКА ПО ЯЗЫКУ
// ============================================================
static const uint8_t* o242_select_header(const uint8_t* ru, const uint8_t* en, const uint8_t* cn, uint8_t lang) {
    switch (lang) {
        case 0:  return ru;
        case 1:  return en;
        case 2:  return cn;
        default: return en;
    }
}

// ============================================================
// НИЗКОУРОВНЕВЫЙ TWI (с timeout)
// ============================================================
void o242_twi_init(void) {
    DDRC |= (1 << PC5); DDRC &= ~(1 << PC4); PORTC |= (1 << PC4);
    for (int i = 0; i < 9; i++) {
        PORTC |= (1 << PC5); _delay_us(5);
        PORTC &= ~(1 << PC5); _delay_us(5);
    }
    DDRC |= (1 << PC4); PORTC &= ~(1 << PC4); _delay_us(5);
    PORTC |= (1 << PC5); _delay_us(5); PORTC |= (1 << PC4); _delay_us(5);
    DDRC &= ~((1 << PC4) | (1 << PC5)); PORTC |= ((1 << PC4) | (1 << PC5));
    TWBR = ((F_CPU / O242_TWI_FREQ) - 16) / 2;
    TWSR = 0;
    TWCR = (1 << TWEN);
    _delay_ms(1);
}

// Таймаут для TWI (≈ 310 мкс при 16 МГц)
#define O242_TWI_TIMEOUT  5000

// Ожидание завершения операции TWI
// Возвращает 1 = ок, 0 = таймаут
static inline uint8_t o242_twi_wait(void) {
    uint16_t timeout = O242_TWI_TIMEOUT;
    while (!(TWCR & (1<<TWINT))) {
        if (--timeout == 0) return 0;
    }
    return 1;
}

// STOP-условие на шине
static inline void o242_twi_stop(void) {
    TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
    _delay_us(10);
}

static void o242_cmd(uint8_t c) {
    // START
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    if (!o242_twi_wait()) return;

    // Адрес + запись
    TWDR = (o242_address << 1);
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
    if (!o242_twi_wait()) { o242_twi_stop(); return; }

    // Байт 0x00 (Command mode)
    TWDR = 0x00;
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
    if (!o242_twi_wait()) { o242_twi_stop(); return; }

    // Байт команды
    TWDR = c;
    TWCR = (1<<TWINT)|(1<<TWEN);
    if (!o242_twi_wait()) { o242_twi_stop(); return; }

    // STOP
    o242_twi_stop();
}

static void o242_set_cursor(uint8_t x, uint8_t page) {
    o242_cmd(0x00 + (x & 0x0F));
    o242_cmd(0x10 + ((x >> 4) & 0x0F));
    o242_cmd(0xB0 + page);
}

// ============================================================
// ИНИЦИАЛИЗАЦИЯ SSD1309
// ============================================================
void o242_init(void) {
    o242_twi_init();
    _delay_ms(100);
    o242_cmd(0xAE); o242_cmd(0xD5); o242_cmd(0x80);
    o242_cmd(0xA8); o242_cmd(0x3F);
    o242_cmd(0xD3); o242_cmd(0x00);
    o242_cmd(0x40);
    o242_cmd(0x8D); o242_cmd(0x14);
    o242_cmd(0x20); o242_cmd(0x02);
    o242_cmd(0xA1); o242_cmd(0xC8);
    o242_cmd(0xDA); o242_cmd(0x12);
    o242_cmd(0x81); o242_cmd(0xCF);
    o242_cmd(0xD9); o242_cmd(0xF1);
    o242_cmd(0xDB); o242_cmd(0x40);
    o242_cmd(0xA4); o242_cmd(0xA6);
    o242_cmd(0xAF);
}

void o242_init_addr(uint8_t addr) { o242_address = addr; o242_init(); }
void o242_display_on(void)  { o242_cmd(0xAF); }
void o242_display_off(void) { o242_cmd(0xAE); }
void o242_set_contrast(uint8_t v) { o242_cmd(0x81); o242_cmd(v); }
void o242_invert(uint8_t e) { o242_cmd(e ? 0xA7 : 0xA6); }

// ============================================================
// FRAMEBUFFER
// ============================================================
void o242_fb_clear(void) { memset(o242_framebuffer, 0, sizeof(o242_framebuffer)); }

void o242_fb_clear_page(uint8_t page) {
    if (page > 7) return;
    memset(&o242_framebuffer[page * O242_WIDTH], 0, O242_WIDTH);
}

void o242_fb_flush(void) { for (uint8_t p = 0; p < 8; p++) o242_fb_flush_page(p); }

void o242_fb_flush_page(uint8_t page) {
    if (page > 7) return;
    o242_set_cursor(0, page);

    // START
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    if (!o242_twi_wait()) return;

    // Адрес + запись
    TWDR = (o242_address << 1);
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
    if (!o242_twi_wait()) { o242_twi_stop(); return; }

    // Data mode
    TWDR = 0x40;
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
    if (!o242_twi_wait()) { o242_twi_stop(); return; }

    // Данные страницы
    for (uint8_t col = 0; col < O242_WIDTH; col++) {
        TWDR = o242_framebuffer[page * O242_WIDTH + col];
        if (col < O242_WIDTH - 1) {
            TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
        } else {
            TWCR = (1<<TWINT)|(1<<TWEN);
        }
        if (!o242_twi_wait()) { o242_twi_stop(); return; }
    }

    // STOP
    o242_twi_stop();
}

// ============================================================
// ГРАФИКА
// ============================================================
void o242_draw_pixel(int16_t x, int16_t y) {
    if (x < 0 || x >= O242_WIDTH || y < 0 || y >= O242_HEIGHT) return;
    o242_framebuffer[(y >> 3) * O242_WIDTH + x] |= (1 << (y & 7));
}

void o242_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    int16_t dx = abs(x1 - x0), dy = abs(y1 - y0);
    int16_t sx = (x0 < x1) ? 1 : -1, sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;
    while (1) {
        o242_draw_pixel(x0, y0);
        if (x0 == x1 && y0 == y1) break;
        int16_t e2 = err << 1;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 <  dx) { err += dx; y0 += sy; }
    }
}

void o242_draw_rect(int16_t x, int16_t y, int16_t w, int16_t h) {
    o242_draw_line(x, y, x+w-1, y);
    o242_draw_line(x, y+h-1, x+w-1, y+h-1);
    o242_draw_line(x, y, x, y+h-1);
    o242_draw_line(x+w-1, y, x+w-1, y+h-1);
}

void o242_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h) {
    for (int16_t i = 0; i < h; i++)
        for (int16_t j = 0; j < w; j++)
            o242_draw_pixel(x+j, y+i);
}

void o242_draw_circle(int16_t x0, int16_t y0, int16_t r) {
    int16_t f = 1-r, ddF_x = 1, ddF_y = -2*r, x = 0, y = r;
    o242_draw_pixel(x0, y0+r); o242_draw_pixel(x0, y0-r);
    o242_draw_pixel(x0+r, y0); o242_draw_pixel(x0-r, y0);
    while (x < y) {
        if (f >= 0) { y--; ddF_y += 2; f += ddF_y; }
        x++; ddF_x += 2; f += ddF_x;
        o242_draw_pixel(x0+x, y0+y); o242_draw_pixel(x0-x, y0+y);
        o242_draw_pixel(x0+x, y0-y); o242_draw_pixel(x0-x, y0-y);
        o242_draw_pixel(x0+y, y0+x); o242_draw_pixel(x0-y, y0+x);
        o242_draw_pixel(x0+y, y0-x); o242_draw_pixel(x0-y, y0-x);
    }
}

void o242_fill_circle(int16_t x0, int16_t y0, int16_t r) {
    o242_draw_line(x0-r, y0, x0+r, y0);
    for (int16_t dy = 1; dy <= r; dy++) {
        int16_t dx = 0;
        while (dx*dx + dy*dy <= r*r) dx++; dx--;
        o242_draw_line(x0-dx, y0+dy, x0+dx, y0+dy);
        o242_draw_line(x0-dx, y0-dy, x0+dx, y0-dy);
    }
}

void o242_draw_bitmap(const uint8_t* bmp, int16_t x, int16_t y, int16_t w, int16_t h) {
    for (int16_t j = 0; j < h; j++)
        for (int16_t i = 0; i < w; i++)
            if (pgm_read_byte(&bmp[(j*((w+7)/8))+(i/8)]) & (0x80>>(i&7)))
                o242_draw_pixel(x+i, y+j);
}

void o242_invert_rect(int16_t x, int16_t y, int16_t w, int16_t h) {
    for (int16_t j = 0; j < h; j++)
        for (int16_t i = 0; i < w; i++) {
            int16_t px = x+i, py = y+j;
            if (px >= 0 && px < O242_WIDTH && py >= 0 && py < O242_HEIGHT)
                o242_framebuffer[(py>>3)*O242_WIDTH+px] ^= (1<<(py&7));
        }
}

// ============================================================
// ТЕКСТ
// ============================================================
void o242_set_font(const o242_font_t* font) { o242_current_font = font; }

// ============================================================
// БИНАРНЫЙ ПОИСК В UNICODE-ТАБЛИЦЕ
// ВНИМАНИЕ: функция работает только с o242_terminus_table.
// При добавлении второго Unicode-шрифта — переделать на поиск по uf->table.
// ============================================================
static const o242_unicode_entry_t* o242_unicode_find(uint16_t code) {
    uint16_t lo = 0;
    uint16_t hi = o242_terminus_count - 1;

    while (lo <= hi) {
        uint16_t mid = lo + (hi - lo) / 2;
        uint16_t c = pgm_read_word(&o242_terminus_table[mid].code);

        if (c == code) return &o242_terminus_table[mid];
        if (c < code)  lo = mid + 1;
        else           hi = mid - 1;
    }
    return 0;
}

uint8_t o242_draw_char_unicode(uint16_t code, int16_t x, int16_t y) {
    if (!o242_current_font || !o242_current_font->unicode) return 0;

    const o242_unicode_entry_t* e = o242_unicode_find(code);
    if (!e) return 0;

    const uint8_t* glyph = (const uint8_t*)pgm_read_ptr(&e->glyph);
    const o242_unicode_font_t* uf = (const o242_unicode_font_t*)o242_current_font->data;
    uint8_t w = uf->width;
    uint8_t h = uf->height;

    for (uint8_t row = 0; row < h; row++) {
        uint8_t bits = pgm_read_byte(&glyph[row]);
        for (uint8_t col = 0; col < w; col++) {
            if (bits & (0x80 >> col)) {
                o242_draw_pixel(x + col, y + row);
            }
        }
    }
    return w - 1;
}

uint8_t o242_draw_char(char c, int16_t x, int16_t y) {
    if (!o242_current_font) return 0;
    const o242_font_t* f = o242_current_font;

    if (f->unicode) {
        return o242_draw_char_unicode((uint16_t)(uint8_t)c, x, y);
    }

    if (!f->is_fixed_width && f->height > 8) {
        const glyph_entry_t* table = (const glyph_entry_t*)f->data;
        uint8_t idx = 0xFF;

        if (c >= '0' && c <= '9')      idx = c - '0';
        else if (c == 'A')             idx = 10;
        else if (c == 'C')             idx = 11;
        else if (c == 'F')             idx = 12;
        else if (c == 'U')             idx = 13;
        else if (c == '-')             idx = 14;
        else if (c == '+')             idx = 15;
        else if (c == (char)0xB0)      idx = 16;
        else if (c == 'o' || c == 'O') idx = 16;
        else if (c == '.')             idx = 17;
        else if (c == ':')             idx = 18;
        else if (c == 'h')             idx = 19;
        else if (c == 'P')             idx = 20;
        else if (c == 'a')             idx = 21;
        else if (c == '%')             idx = 22;

        if (idx == 0xFF || idx >= f->num_chars) return 0;

        const uint8_t* glyphData = (const uint8_t*)pgm_read_ptr(&table[idx].ptr);
        uint8_t glyphWidth = pgm_read_byte(&table[idx].w);
        uint8_t bytesPerRow = (glyphWidth + 7) / 8;

        for (int16_t row = 0; row < f->height; row++) {
            for (int16_t col = 0; col < glyphWidth; col++) {
                uint16_t byteIndex = row * bytesPerRow + (col / 8);
                uint8_t bitMask = 0x80 >> (col & 7);
                if (pgm_read_byte(&glyphData[byteIndex]) & bitMask)
                    o242_draw_pixel(x + col, y + row);
            }
        }
        return glyphWidth + 2;
    }

    return 0;
}

uint8_t o242_draw_string(const char* str, int16_t x, int16_t y) {
    int16_t start_x = x;
    while (*str) {
        uint8_t w = o242_draw_char(*str++, x, y);
        if (w == 0) w = 8;
        x += w;
    }
    return x - start_x;
}

uint8_t o242_draw_char_line(char c, int16_t x, uint8_t line) { return o242_draw_char(c, x, line * 12); }
uint8_t o242_draw_string_line(const char* str, int16_t x, uint8_t line) { return o242_draw_string(str, x, line * 12); }
uint8_t o242_draw_string_utf8_line(const char* str, int16_t x, uint8_t line) { return o242_draw_string_utf8(str, x, line * 12); }

// ============================================================
// UTF-8 → Unicode
// ============================================================
static uint16_t o242_utf8_to_unicode(const char** pstr) {
    const uint8_t* s = (const uint8_t*)*pstr;
    uint8_t c = *s++;

    if (c < 0x80) {
        *pstr = (const char*)s;
        return c;
    } else if ((c & 0xE0) == 0xC0) {
        uint8_t c2 = *s++;
        *pstr = (const char*)s;
        return ((uint16_t)(c & 0x1F) << 6) | (c2 & 0x3F);
    } else if ((c & 0xF0) == 0xE0) {
        uint8_t c2 = *s++;
        uint8_t c3 = *s++;
        *pstr = (const char*)s;
        return ((uint16_t)(c & 0x0F) << 12) | ((uint16_t)(c2 & 0x3F) << 6) | (c3 & 0x3F);
    } else {
        *pstr = (const char*)s;
        return '?';
    }
}

uint8_t o242_draw_string_utf8(const char* str, int16_t x, int16_t y) {
    int16_t start_x = x;
    while (*str) {
        uint16_t code = o242_utf8_to_unicode(&str);
        uint8_t w;
        if (o242_current_font && o242_current_font->unicode) {
            w = o242_draw_char_unicode(code, x, y);
        } else {
            unsigned char c = (unsigned char)code;
            if (code > 0xFF) c = '?';
            w = o242_draw_char((char)c, x, y);
        }
        if (w == 0) w = 8;
        x += w;
    }
    return x - start_x;
}

// ============================================================
// СПЕЦИАЛИЗИРОВАННЫЕ
// ============================================================
void o242_draw_hh_mm(uint8_t hh, uint8_t mm, int16_t x, int16_t y) {
    int16_t cx = x;
    cx += o242_draw_char('0'+(hh/10), cx, y);
    cx += o242_draw_char('0'+(hh%10), cx, y);
    cx += o242_draw_char(':', cx, y);
    cx += o242_draw_char('0'+(mm/10), cx, y);
    cx += o242_draw_char('0'+(mm%10), cx, y);
}

void o242_draw_hh_mm_ss(uint8_t hh, uint8_t mm, uint8_t ss, int16_t x, int16_t y) {
    int16_t cx = x;
    cx += o242_draw_char('0'+(hh/10), cx, y);
    cx += o242_draw_char('0'+(hh%10), cx, y);
    cx += o242_draw_char(':', cx, y);
    cx += o242_draw_char('0'+(mm/10), cx, y);
    cx += o242_draw_char('0'+(mm%10), cx, y);
    cx += o242_draw_char(':', cx, y);
    cx += o242_draw_char('0'+(ss/10), cx, y);
    cx += o242_draw_char('0'+(ss%10), cx, y);
}

// ============================================================
// ВНУТРЕННЯЯ: float → строка с 1 знаком после запятой
// ============================================================
static uint8_t o242_format_float1(float number, char* buf, uint8_t buf_size) {
    uint8_t pos = 0;
    bool is_neg = (number < 0);
    float abs_val = is_neg ? -number : number;
    long n_scaled = lroundf(abs_val * 10.0f);

    char temp[12];
    int8_t tpos = 11;
    temp[tpos--] = '\0';

    temp[tpos--] = '0' + (n_scaled % 10); n_scaled /= 10;
    temp[tpos--] = '.';

    if (n_scaled == 0) {
        temp[tpos--] = '0';
    } else {
        while (n_scaled > 0 && tpos >= 0) {
            temp[tpos--] = '0' + (n_scaled % 10);
            n_scaled /= 10;
        }
    }

    if (is_neg) temp[tpos--] = '-';
    else        temp[tpos--] = '+';

    tpos++;
    while (temp[tpos] && pos < buf_size - 1) {
        buf[pos++] = temp[tpos++];
    }
    buf[pos] = '\0';
    return pos;
}

// ============================================================
// ВНУТРЕННЯЯ: определение страниц по Y и высоте
// ============================================================
static uint8_t o242_page_start(int16_t y) {
    if (y < 0) return 0;
    return (uint8_t)(y >> 3);
}

static uint8_t o242_page_end(int16_t y, uint8_t h) {
    int16_t end = y + h - 1;
    if (end < 0) return 0;
    if (end >= 64) return 7;
    return (uint8_t)(end >> 3);
}

// ============================================================
// ОБЁРТКИ ДЛЯ КОНЕЧНОГО ПОЛЬЗОВАТЕЛЯ (pos = 1/2)
// ============================================================
static int16_t o242_pos_to_y(uint8_t pos) {
    return (pos == 1) ? 8 : 40;
}

void oledText(const char* str, int16_t x, uint8_t line) {
    o242_set_font(&o242_font_terminus_8x12);
    o242_draw_string_utf8(str, x, line * 12);
}

void oledValue(const char* str, int16_t x, uint8_t pos) {
    if (pos < 1 || pos > 2) return;
    o242_set_font(&o242_font_blocky);
    o242_draw_string(str, x, o242_pos_to_y(pos));
}

void oledTemp(float temp, int16_t x, uint8_t pos) {
    if (pos < 1 || pos > 2) return;
    char buf[12];
    o242_format_float1(temp, buf, sizeof(buf));
    o242_set_font(&o242_font_blocky);
    int16_t y = o242_pos_to_y(pos);
    uint8_t w = o242_draw_string(buf, x, y);
    o242_draw_string("\xB0" "C", x + w, y);
}

void oledAmper(float amp, int16_t x, uint8_t pos) {
    if (pos < 1 || pos > 2) return;
    char buf[12];
    o242_format_float1(amp, buf, sizeof(buf));
    o242_set_font(&o242_font_blocky);
    int16_t y = o242_pos_to_y(pos);
    uint8_t w = o242_draw_string(buf, x, y);
    o242_draw_char('A', x + w, y);
}

void oledVolt(float volt, int16_t x, uint8_t pos) {
    if (pos < 1 || pos > 2) return;
    char buf[12];
    o242_format_float1(volt, buf, sizeof(buf));
    o242_set_font(&o242_font_blocky);
    int16_t y = o242_pos_to_y(pos);
    uint8_t w = o242_draw_string(buf, x, y);
    o242_draw_char('U', x + w, y);
}

void oledPressure(uint16_t press, int16_t x, uint8_t pos) {
    if (pos < 1 || pos > 2) return;
    char buf[8];
    uint8_t idx = 0;
    if (press >= 10000) buf[idx++] = '0' + (press / 10000) % 10;
    if (press >= 1000)  buf[idx++] = '0' + (press / 1000) % 10;
    if (press >= 100)   buf[idx++] = '0' + (press / 100) % 10;
    if (press >= 10)    buf[idx++] = '0' + (press / 10) % 10;
    buf[idx++] = '0' + press % 10;
    buf[idx] = '\0';

    o242_set_font(&o242_font_blocky);
    int16_t y = o242_pos_to_y(pos);
    uint8_t w = o242_draw_string(buf, x, y);
    o242_draw_string("hPa", x + w, y);
}

void oledHumid(uint8_t humid, int16_t x, uint8_t pos) {
    if (pos < 1 || pos > 2) return;
    char buf[6];
    uint8_t idx = 0;
    if (humid >= 100) {
        buf[idx++] = '1'; buf[idx++] = '0'; buf[idx++] = '0';
    } else {
        if (humid >= 10) buf[idx++] = '0' + (humid / 10);
        buf[idx++] = '0' + (humid % 10);
    }
    buf[idx] = '\0';

    o242_set_font(&o242_font_blocky);
    int16_t y = o242_pos_to_y(pos);
    uint8_t w = o242_draw_string(buf, x, y);
    o242_draw_char('%', x + w, y);
}

void oledTime(uint8_t hh, uint8_t mm, uint8_t ss, int16_t x, uint8_t pos) {
    if (pos < 1 || pos > 2) return;
    o242_set_font(&o242_font_blocky);
    o242_draw_hh_mm_ss(hh, mm, ss, x, o242_pos_to_y(pos));
}

void oledDate(uint8_t dd, uint8_t mm, uint8_t yy, int16_t x, uint8_t pos) {
    if (pos < 1 || pos > 2) return;
    o242_set_font(&o242_font_blocky);
    int16_t y = o242_pos_to_y(pos);

    int16_t cx = x;
    cx += o242_draw_char('0' + (dd / 10), cx, y);
    cx += o242_draw_char('0' + (dd % 10), cx, y);
    cx += o242_draw_char('.', cx, y);
    cx += o242_draw_char('0' + (mm / 10), cx, y);
    cx += o242_draw_char('0' + (mm % 10), cx, y);
    cx += o242_draw_char('.', cx, y);
    cx += o242_draw_char('0' + (yy / 10), cx, y);
    cx += o242_draw_char('0' + (yy % 10), cx, y);
}

// ============================================================
// ЗНАЧЕНИЯ С ПИКСЕЛЬНЫМИ КООРДИНАТАМИ (только значение)
// ============================================================
void oledValueXY(const char* str, int16_t x, int16_t y) {
    o242_set_font(&o242_font_blocky);
    o242_draw_string(str, x, y);
}

void oledTempXY(float temp, int16_t x, int16_t y) {
    char buf[12];
    o242_format_float1(temp, buf, sizeof(buf));
    o242_set_font(&o242_font_blocky);
    uint8_t w = o242_draw_string(buf, x, y);
    o242_draw_string("\xB0" "C", x + w, y);
}

void oledHumidXY(uint8_t humid, int16_t x, int16_t y) {
    char buf[6];
    uint8_t idx = 0;
    if (humid >= 100) {
        buf[idx++] = '1'; buf[idx++] = '0'; buf[idx++] = '0';
    } else {
        if (humid >= 10) buf[idx++] = '0' + (humid / 10);
        buf[idx++] = '0' + (humid % 10);
    }
    buf[idx] = '\0';
    o242_set_font(&o242_font_blocky);
    uint8_t w = o242_draw_string(buf, x, y);
    o242_draw_char('%', x + w, y);
}

void oledPressXY(uint16_t press, int16_t x, int16_t y) {
    char buf[8];
    uint8_t idx = 0;
    if (press >= 10000) buf[idx++] = '0' + (press / 10000) % 10;
    if (press >= 1000)  buf[idx++] = '0' + (press / 1000) % 10;
    if (press >= 100)   buf[idx++] = '0' + (press / 100) % 10;
    if (press >= 10)    buf[idx++] = '0' + (press / 10) % 10;
    buf[idx++] = '0' + press % 10;
    buf[idx] = '\0';
    o242_set_font(&o242_font_blocky);
    uint8_t w = o242_draw_string(buf, x, y);
    o242_draw_string("hPa", x + w, y);
}

void oledVoltXY(float volt, int16_t x, int16_t y) {
    char buf[12];
    o242_format_float1(volt, buf, sizeof(buf));
    o242_set_font(&o242_font_blocky);
    uint8_t w = o242_draw_string(buf, x, y);
    o242_draw_char('U', x + w, y);
}

void oledAmperXY(float amp, int16_t x, int16_t y) {
    char buf[12];
    o242_format_float1(amp, buf, sizeof(buf));
    o242_set_font(&o242_font_blocky);
    uint8_t w = o242_draw_string(buf, x, y);
    o242_draw_char('A', x + w, y);
}

void oledTimeXY(uint8_t hh, uint8_t mm, uint8_t ss, int16_t x, int16_t y) {
    o242_set_font(&o242_font_blocky);
    o242_draw_hh_mm_ss(hh, mm, ss, x, y);
}

void oledDateXY(uint8_t dd, uint8_t mm, uint8_t yy, int16_t x, int16_t y) {
    o242_set_font(&o242_font_blocky);
    int16_t cx = x;
    cx += o242_draw_char('0' + (dd / 10), cx, y);
    cx += o242_draw_char('0' + (dd % 10), cx, y);
    cx += o242_draw_char('.', cx, y);
    cx += o242_draw_char('0' + (mm / 10), cx, y);
    cx += o242_draw_char('0' + (mm % 10), cx, y);
    cx += o242_draw_char('.', cx, y);
    cx += o242_draw_char('0' + (yy / 10), cx, y);
    cx += o242_draw_char('0' + (yy % 10), cx, y);
}

// ============================================================
// ПОЛНЫЕ ФУНКЦИИ — заголовок + значение (запоминают координаты)
// ============================================================

void oledTempFull(float temp, int16_t hx, int16_t hy, int16_t vx, int16_t vy, uint8_t lang) {
    oledBitmap(o242_select_header(epd_bitmap_temp_ru, epd_bitmap_temp_en, epd_bitmap_temp_cn, lang), hx, hy, 128, 16);

    char buf[12];
    o242_format_float1(temp, buf, sizeof(buf));
    o242_set_font(&o242_font_blocky);
    uint8_t w = o242_draw_string(buf, vx, vy);
    o242_draw_string("\xB0" "C", vx + w, vy);

    o242_pos_temp.vx = vx;
    o242_pos_temp.vy = vy;
    o242_pos_temp.active = 1;
}

void oledHumidFull(uint8_t humid, int16_t hx, int16_t hy, int16_t vx, int16_t vy, uint8_t lang) {
    oledBitmap(o242_select_header(epd_bitmap_humid_ru, epd_bitmap_humid_en, epd_bitmap_humid_cn, lang), hx, hy, 128, 16);

    char buf[6];
    uint8_t idx = 0;
    if (humid >= 100) {
        buf[idx++] = '1'; buf[idx++] = '0'; buf[idx++] = '0';
    } else {
        if (humid >= 10) buf[idx++] = '0' + (humid / 10);
        buf[idx++] = '0' + (humid % 10);
    }
    buf[idx] = '\0';
    o242_set_font(&o242_font_blocky);
    uint8_t w = o242_draw_string(buf, vx, vy);
    o242_draw_char('%', vx + w, vy);

    o242_pos_humid.vx = vx;
    o242_pos_humid.vy = vy;
    o242_pos_humid.active = 1;
}

void oledPressFull(uint16_t press, int16_t hx, int16_t hy, int16_t vx, int16_t vy, uint8_t lang) {
    oledBitmap(o242_select_header(epd_bitmap_press_ru, epd_bitmap_press_en, epd_bitmap_press_cn, lang), hx, hy, 128, 16);

    char buf[8];
    uint8_t idx = 0;
    if (press >= 10000) buf[idx++] = '0' + (press / 10000) % 10;
    if (press >= 1000)  buf[idx++] = '0' + (press / 1000) % 10;
    if (press >= 100)   buf[idx++] = '0' + (press / 100) % 10;
    if (press >= 10)    buf[idx++] = '0' + (press / 10) % 10;
    buf[idx++] = '0' + press % 10;
    buf[idx] = '\0';
    o242_set_font(&o242_font_blocky);
    uint8_t w = o242_draw_string(buf, vx, vy);
    o242_draw_string("hPa", vx + w, vy);

    o242_pos_press.vx = vx;
    o242_pos_press.vy = vy;
    o242_pos_press.active = 1;
}

void oledVoltFull(float volt, int16_t hx, int16_t hy, int16_t vx, int16_t vy, uint8_t lang) {
    oledBitmap(o242_select_header(epd_bitmap_volt_ru, epd_bitmap_volt_en, epd_bitmap_volt_cn, lang), hx, hy, 128, 16);

    char buf[12];
    o242_format_float1(volt, buf, sizeof(buf));
    o242_set_font(&o242_font_blocky);
    uint8_t w = o242_draw_string(buf, vx, vy);
    o242_draw_char('U', vx + w, vy);

    o242_pos_volt.vx = vx;
    o242_pos_volt.vy = vy;
    o242_pos_volt.active = 1;
}

void oledAmperFull(float amp, int16_t hx, int16_t hy, int16_t vx, int16_t vy, uint8_t lang) {
    oledBitmap(o242_select_header(epd_bitmap_current_ru, epd_bitmap_current_en, epd_bitmap_current_cn, lang), hx, hy, 128, 16);

    char buf[12];
    o242_format_float1(amp, buf, sizeof(buf));
    o242_set_font(&o242_font_blocky);
    uint8_t w = o242_draw_string(buf, vx, vy);
    o242_draw_char('A', vx + w, vy);

    o242_pos_amper.vx = vx;
    o242_pos_amper.vy = vy;
    o242_pos_amper.active = 1;
}

void oledTimeFull(uint8_t hh, uint8_t mm, uint8_t ss, int16_t hx, int16_t hy, int16_t vx, int16_t vy, uint8_t lang) {
    oledBitmap(o242_select_header(epd_bitmap_time_ru, epd_bitmap_time_en, epd_bitmap_time_cn, lang), hx, hy, 128, 16);

    o242_set_font(&o242_font_blocky);
    o242_draw_hh_mm_ss(hh, mm, ss, vx, vy);

    o242_pos_time.vx = vx;
    o242_pos_time.vy = vy;
    o242_pos_time.active = 1;
}

void oledDateFull(uint8_t dd, uint8_t mm, uint8_t yy, int16_t hx, int16_t hy, int16_t vx, int16_t vy, uint8_t lang) {
    oledBitmap(o242_select_header(epd_bitmap_date_ru, epd_bitmap_date_en, epd_bitmap_date_cn, lang), hx, hy, 128, 16);

    o242_set_font(&o242_font_blocky);
    int16_t cx = vx;
    cx += o242_draw_char('0' + (dd / 10), cx, vy);
    cx += o242_draw_char('0' + (dd % 10), cx, vy);
    cx += o242_draw_char('.', cx, vy);
    cx += o242_draw_char('0' + (mm / 10), cx, vy);
    cx += o242_draw_char('0' + (mm % 10), cx, vy);
    cx += o242_draw_char('.', cx, vy);
    cx += o242_draw_char('0' + (yy / 10), cx, vy);
    cx += o242_draw_char('0' + (yy % 10), cx, vy);

    o242_pos_date.vx = vx;
    o242_pos_date.vy = vy;
    o242_pos_date.active = 1;
}

// ============================================================
// UPDATE-ФУНКЦИИ (используют сохранённые координаты)
// ============================================================
void oledTempUpdate(float temp) {
    if (!o242_pos_temp.active) return;
    int16_t vx = o242_pos_temp.vx;
    int16_t vy = o242_pos_temp.vy;

    uint8_t p1 = o242_page_start(vy);
    uint8_t p2 = o242_page_end(vy, 23);
    for (uint8_t p = p1; p <= p2; p++) o242_fb_clear_page(p);
    oledTempXY(temp, vx, vy);
    for (uint8_t p = p1; p <= p2; p++) o242_fb_flush_page(p);
}

void oledHumidUpdate(uint8_t humid) {
    if (!o242_pos_humid.active) return;
    int16_t vx = o242_pos_humid.vx;
    int16_t vy = o242_pos_humid.vy;

    uint8_t p1 = o242_page_start(vy);
    uint8_t p2 = o242_page_end(vy, 23);
    for (uint8_t p = p1; p <= p2; p++) o242_fb_clear_page(p);
    oledHumidXY(humid, vx, vy);
    for (uint8_t p = p1; p <= p2; p++) o242_fb_flush_page(p);
}

void oledPressUpdate(uint16_t press) {
    if (!o242_pos_press.active) return;
    int16_t vx = o242_pos_press.vx;
    int16_t vy = o242_pos_press.vy;

    uint8_t p1 = o242_page_start(vy);
    uint8_t p2 = o242_page_end(vy, 23);
    for (uint8_t p = p1; p <= p2; p++) o242_fb_clear_page(p);
    oledPressXY(press, vx, vy);
    for (uint8_t p = p1; p <= p2; p++) o242_fb_flush_page(p);
}

void oledVoltUpdate(float volt) {
    if (!o242_pos_volt.active) return;
    int16_t vx = o242_pos_volt.vx;
    int16_t vy = o242_pos_volt.vy;

    uint8_t p1 = o242_page_start(vy);
    uint8_t p2 = o242_page_end(vy, 23);
    for (uint8_t p = p1; p <= p2; p++) o242_fb_clear_page(p);
    oledVoltXY(volt, vx, vy);
    for (uint8_t p = p1; p <= p2; p++) o242_fb_flush_page(p);
}

void oledAmperUpdate(float amp) {
    if (!o242_pos_amper.active) return;
    int16_t vx = o242_pos_amper.vx;
    int16_t vy = o242_pos_amper.vy;

    uint8_t p1 = o242_page_start(vy);
    uint8_t p2 = o242_page_end(vy, 23);
    for (uint8_t p = p1; p <= p2; p++) o242_fb_clear_page(p);
    oledAmperXY(amp, vx, vy);
    for (uint8_t p = p1; p <= p2; p++) o242_fb_flush_page(p);
}

void oledTimeUpdate(uint8_t hh, uint8_t mm, uint8_t ss) {
    if (!o242_pos_time.active) return;
    int16_t vx = o242_pos_time.vx;
    int16_t vy = o242_pos_time.vy;

    uint8_t p1 = o242_page_start(vy);
    uint8_t p2 = o242_page_end(vy, 23);
    for (uint8_t p = p1; p <= p2; p++) o242_fb_clear_page(p);
    oledTimeXY(hh, mm, ss, vx, vy);
    for (uint8_t p = p1; p <= p2; p++) o242_fb_flush_page(p);
}

void oledDateUpdate(uint8_t dd, uint8_t mm, uint8_t yy) {
    if (!o242_pos_date.active) return;
    int16_t vx = o242_pos_date.vx;
    int16_t vy = o242_pos_date.vy;

    uint8_t p1 = o242_page_start(vy);
    uint8_t p2 = o242_page_end(vy, 23);
    for (uint8_t p = p1; p <= p2; p++) o242_fb_clear_page(p);
    oledDateXY(dd, mm, yy, vx, vy);
    for (uint8_t p = p1; p <= p2; p++) o242_fb_flush_page(p);
}