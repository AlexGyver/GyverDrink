

/*
 *
 * Pixel
 *
 * created with FontCreator
 * written by F. Maximilian Thiele
 *
 * http://www.apetech.de/fontCreator
 * me@apetech.de
 *
 * File Name           : Pixel.h
 * Date                : 26.09.2020
 * Font size in bytes  : 24
 * Font width          : 4
 * Font height         : -8
 * Font first char     : 48
 * Font last char      : 50
 * Font used chars     : 2
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

#ifndef PIXEL_H
#define PIXEL_H

#define PIXEL_WIDTH 4
#define PIXEL_HEIGHT 8

static const uint8_t Pixel[] PROGMEM = {
    0x00, 0x18, // size
    0x04, // width
    0x08, // height
    0x30, // first char
    0x02, // char count
    
    // char widths
    0x01, 0x01, 
    
    // font data
    0x40, // 48
    0x70 // 49
    
};

#endif
