

/*
 *
 * CenturyNum
 *
 * created with FontCreator
 * written by F. Maximilian Thiele
 *
 * http://www.apetech.de/fontCreator
 * me@apetech.de
 *
 * File Name           : CenturyNum
 * Date                : 17.06.2020
 * Font size in bytes  : 6782
 * Font width          : 10
 * Font height         : 34
 * Font first char     : 48
 * Font last char      : 58
 * Font used chars     : 10
 *
 * The font data are defined as
 *
 * struct _FONT_ {
 *     uint16_t   font_Size_in_Bytes_over_all_included_Size_it_self;
 *     uint8_t    font_Width_in_Pixel_for_fixed_drawing;
 *     uint8_t    font_Height_in_Pixel_for_all_characters;
 *     unit8_t    font_First_Char;
 *     uint8_t    font_Char_Count;
 *
 *     uint8_t    font_Char_Widths[font_Last_Char - font_First_Char +1];
 *                  // for each character the separate width in pixels,
 *                  // characters < 128 have an implicit virtual right empty row
 *
 *     uint8_t    font_data[];
 *                  // bit field of all characters
 */

#include <inttypes.h>
#include <avr/pgmspace.h>

#ifndef CENTURYNUM_H
#define CENTURYNUM_H

#define CENTURYNUM_WIDTH 10
#define CENTURYNUM_HEIGHT 34

GLCDFONTDECL(CenturyNum) = {
    0x1A, 0x7E, // size
    0x0A,       // width
    0x22,       // height
    0x30,       // first char
    0x0A,       // char count

    // char widths
    0x16, 0x08, 0x16, 0x16, 0x16, 0x15, 0x14, 0x14, 0x16, 0x14,

    // font data
    0x00, 0x80, 0xE0, 0xF8, 0x7C, 0x3C, 0x1E, 0x0E, 0x0F, 0x07, 0x07, 0x07, 0x07, 0x0F, 0x0E, 0x1E, 0x3C, 0xF8, 0xF0, 0xE0, 0x80, 0x00, 0xFC, 0xFF, 0xFF, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xFF, 0xFF, 0xF8, 0xFF, 0xFF, 0xFF, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xFF, 0xFF, 0x7F, 0x00, 0x07, 0x1F, 0x7F, 0xF8, 0xF0, 0xE0, 0xC0, 0xC0, 0x80, 0x80, 0x80, 0x80, 0xC0, 0xC0, 0xE0, 0xF0, 0x7C, 0x3F, 0x1F, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x40, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 48
    0x08, 0x0C, 0x0E, 0x0E, 0x0F, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x40, 0x40,                                                                                                                                                                                                                                                                                                                                                                                                                                     // 49
    0x00, 0x00, 0xE0, 0xF0, 0xF8, 0x3C, 0x1E, 0x0E, 0x0E, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x0E, 0x1E, 0x3E, 0x7C, 0xF8, 0xF0, 0x80, 0x00, 0x07, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xF0, 0xFF, 0x7F, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0x78, 0x3C, 0x1E, 0x0F, 0x07, 0x03, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xDE, 0xCF, 0xC7, 0xC3, 0xC1, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, // 50
    0x00, 0xC0, 0xF0, 0xF8, 0x7C, 0x1E, 0x0E, 0x0F, 0x07, 0x07, 0x07, 0x07, 0x07, 0x0F, 0x0E, 0x1E, 0x3C, 0xF8, 0xF0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0, 0xC0, 0xE0, 0xE0, 0xE0, 0xF0, 0xF8, 0x3F, 0x1F, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x03, 0x03, 0x03, 0x07, 0x0F, 0x3E, 0xFC, 0xF8, 0xE0, 0x06, 0x1E, 0x7E, 0xFC, 0xF0, 0xE0, 0xC0, 0xC0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xC0, 0xC0, 0xE0, 0xF0, 0x7C, 0x3F, 0x1F, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x40, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0x40, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, // 51
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xF0, 0xF8, 0xFE, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xE0, 0xF0, 0x7C, 0x3E, 0x0F, 0x07, 0x01, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xE0, 0xF8, 0xFC, 0x9F, 0x8F, 0x83, 0x81, 0x80, 0x80, 0x80, 0x80, 0x80, 0xFF, 0xFF, 0xFF, 0x80, 0x80, 0x80, 0x80, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0xFF, 0xFF, 0xFF, 0x03, 0x03, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00, // 52
    0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xFE, 0xFE, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xFE, 0xFF, 0xFF, 0x71, 0x70, 0x70, 0x70, 0x70, 0x70, 0xF0, 0xE0, 0xE0, 0xC0, 0xC0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x0F, 0xFF, 0xFE, 0xF8, 0x06, 0x1E, 0x7E, 0xF8, 0xF0, 0xE0, 0xC0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xC0, 0xC0, 0xE0, 0xF8, 0x7C, 0x3F, 0x1F, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x40, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0x40, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,                               // 53
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0x7C, 0x3E, 0x0F, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xF0, 0xF8, 0xBE, 0xDF, 0xC7, 0xC3, 0xC0, 0xC0, 0xC0, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xFC, 0xFE, 0x1F, 0x0F, 0x07, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0xFE, 0xFC, 0xE0, 0x07, 0x1F, 0x7F, 0xF8, 0xF0, 0xE0, 0xC0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0x7F, 0x1F, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x40, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,                                                             // 54
    0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x8E, 0xEE, 0xFE, 0xFE, 0x3E, 0x0E, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xE0, 0xF8, 0xFE, 0x3F, 0x0F, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xE0, 0xF8, 0xFE, 0x3F, 0x0F, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xE0, 0xF8, 0xFE, 0x3F, 0x0F, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xC0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                                                             // 55
    0x00, 0x00, 0xC0, 0xF0, 0xF8, 0x3C, 0x1E, 0x0E, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x0E, 0x1E, 0x3C, 0xF8, 0xF0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x07, 0x1F, 0x3F, 0x7C, 0xF0, 0xE0, 0xE0, 0xC0, 0xC0, 0xC0, 0xC0, 0xE0, 0xE0, 0xF0, 0x7C, 0x3F, 0x1F, 0x07, 0x00, 0x00, 0xE0, 0xF8, 0xFC, 0x3E, 0x0E, 0x07, 0x03, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x03, 0x03, 0x07, 0x0E, 0x3E, 0xFC, 0xF8, 0xE0, 0x07, 0x1F, 0x7F, 0xF8, 0xF0, 0xE0, 0xC0, 0xC0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xC0, 0xC0, 0xE0, 0xF0, 0xFC, 0x7F, 0x1F, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x40, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, // 56
    0x80, 0xE0, 0xF8, 0x7C, 0x3C, 0x1E, 0x0E, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x0E, 0x1E, 0x3C, 0x7C, 0xF8, 0xE0, 0x80, 0x1F, 0xFF, 0xFF, 0xE0, 0xC0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xE0, 0xFF, 0x7F, 0x1F, 0x00, 0x00, 0x01, 0x03, 0x07, 0x07, 0x0F, 0x0E, 0x0E, 0x0E, 0xCE, 0xEE, 0xF6, 0x7F, 0x3F, 0x0F, 0x07, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xF0, 0xF8, 0x3E, 0x1F, 0x07, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0xC0, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00                                                              // 57

};

#endif
