// ============================================================
// fonts.h
// ============================================================
#ifndef O242_FONTS_H
#define O242_FONTS_H

#include <avr/pgmspace.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Обычный растровый шрифт (Blocky)
typedef struct {
    const uint8_t* data;
    uint8_t width;
    uint8_t height;
    uint8_t first_char;
    uint8_t num_chars;
    uint8_t bytes_per_char;
    uint8_t is_fixed_width;
    uint8_t pages;
    uint8_t unicode;              // 0 = обычный, 1 = Unicode
} o242_font_t;

// Запись в таблице переходов (для Blocky)
typedef struct {
    const uint8_t* ptr;
    uint8_t w;
} glyph_entry_t;

// Запись в Unicode-таблице (для Terminus)
typedef struct {
    uint16_t code;                // Unicode-код символа
    const uint8_t* glyph;         // Указатель на массив байт
} o242_unicode_entry_t;

// Unicode-шрифт (Terminus)
typedef struct {
    const o242_unicode_entry_t* table;
    uint16_t count;
    uint8_t  width;
    uint8_t  height;
} o242_unicode_font_t;

// Внешние шрифты
extern const o242_font_t o242_font_blocky;
extern const o242_font_t o242_font_terminus_8x12;

// Unicode-таблица Terminus (нужна для бинарного поиска)
extern const o242_unicode_entry_t o242_terminus_table[];
extern const uint16_t o242_terminus_count;
extern const o242_unicode_font_t o242_terminus_unicode;

#ifdef __cplusplus
}
#endif

#endif