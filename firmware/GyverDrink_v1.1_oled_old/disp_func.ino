
void dispNum(int num, uint8_t type) {
  u8g2.setFontDirection(0);

  char outStr[] = "";

  if (type == ml) {
    uint8_t gap = 0; // u8g2.getStrWidth(mlStr);

    sprintf_P(outStr, (PGM_P)F("%d"), num);
    u8g2.setFont(u8g2_font_fur35_tn); //u8g2_font_7Segments_26x42_mn //u8g2_font_fur35_tn
    u8g2.setCursor( (128 - u8g2.getStrWidth(outStr) - gap) / 2, 64);
    u8g2.print(outStr);
    u8g2.setFont(u8g2_font_10x20_t_cyrillic);
    u8g2.print(F("ml"));
  }

  else if (type == ms) {
    uint8_t gap = 0; // u8g2.getStrWidth(mlStr);

    sprintf_P(outStr, (PGM_P)F("%d"), num);
    u8g2.setFont(u8g2_font_fur35_tn); //u8g2_font_7Segments_26x42_mn //u8g2_font_fur35_tn
    u8g2.setCursor( (128 - u8g2.getStrWidth(outStr) - gap) / 2, 64);
    u8g2.print(outStr);
    u8g2.setFont(u8g2_font_10x20_t_cyrillic);
    u8g2.print(F("ms"));
  }

  else if (type == deg) {
    uint8_t gap = 0; // u8g2.getStrWidth(mlStr);

    sprintf_P(outStr, (PGM_P)F("%d"), num);
    u8g2.setFont(u8g2_font_fur35_tn); //u8g2_font_7Segments_26x42_mn //u8g2_font_fur35_tn
    u8g2.setCursor( (128 - u8g2.getStrWidth(outStr) - gap) / 2, 64);
    u8g2.print(outStr);
    u8g2.setFont(u8g2_font_10x20_t_cyrillic);
    u8g2.print(F("°"));
  }
  else {
    uint8_t gap = 0; // u8g2.getStrWidth(mlStr);

    sprintf_P(outStr, (PGM_P)F("%d"), num);
    u8g2.setFont(u8g2_font_fur35_tn); //u8g2_font_7Segments_26x42_mn //u8g2_font_fur35_tn
    u8g2.setCursor( (128 - u8g2.getStrWidth(outStr) - gap) / 2, 64);
    u8g2.print(outStr);
    //    u8g2.setFont(u8g2_font_10x20_t_cyrillic);
    //    u8g2.print(F("°"));
  }

  u8g2.sendBuffer();
}
