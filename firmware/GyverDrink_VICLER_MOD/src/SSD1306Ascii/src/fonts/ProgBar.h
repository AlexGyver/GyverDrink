

/*
 *
 * ProgBar
 *
 * created with FontCreator
 * written by F. Maximilian Thiele
 *
 * http://www.apetech.de/fontCreator
 * me@apetech.de
 *
 * File Name           : ProgBar.h
 * Date                : 01.10.2020
 * Font size in bytes  : 48
 * Font width          : 1
 * Font height         : 8
 * Font first char     : 32
 * Font last char      : 50
 * Font used chars     : 18
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

#ifndef PROGBAR_H
#define PROGBAR_H

#define PROGBAR_WIDTH 1
#define PROGBAR_HEIGHT 8

static const uint8_t ProgBar[] PROGMEM = {
    0x00, 0x30, // size
    0x01, // width
    0x08, // height
    0x20, // first char
    0x12, // char count
    
    // char widths
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 
    
    // font data
    0x00, // 32
    0x70, // 45
    0x40, // 46
    
};

#endif
