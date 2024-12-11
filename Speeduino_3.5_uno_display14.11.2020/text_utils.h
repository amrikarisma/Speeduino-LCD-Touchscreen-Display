#ifndef TEXT_UTILS_H
#define TEXT_UTILS_H

bool isTextNotInList(const String& text) {
  const char* validList[] = {"IAT", "Coolant", "ADV","AFR", "MAP","Voltage","TPS","FPS"}; // Daftar teks valid
  const int listSize = sizeof(validList) / sizeof(validList[0]);  // Hitung ukuran array

    for (int i = 0; i < listSize; i++) {
        if (text.equals(validList[i])) {
            return false; // Teks ditemukan dalam daftar
        }
    }
    return true; // Teks tidak ditemukan dalam daftar
}

// More accurate text width calculation
int getTextWidth(const char* text, int textSize) {
  int width = 0;
  for(int i = 0; text[i] != '\0'; i++) {
    if(text[i] >= '0' && text[i] <= '9') {
      width += 6 * textSize; // Numbers are typically 6 pixels wide
    } else if(text[i] == '.') {
      width += 2 * textSize; // Decimal point is narrower
    } else if(text[i] == '%') {
      width += 7 * textSize; // Percent sign is wider
    } else {
      width += 6 * textSize; // Standard character width
    }
  }
  return width;
}

// Center text horizontally within a given width
int getCenteredX(int containerX, int containerWidth, const char* text, int textSize) {
  int textWidth = getTextWidth(text, textSize);
  return containerX + (containerWidth - textWidth) / 2;
}

// Center text vertically within a given height
int getCenteredY(int containerY, int containerHeight, int textSize) {
  int charHeight = 8 * textSize; // Character height is typically 8 pixels
  return containerY + (containerHeight - charHeight) / 2;
}

void clearBuffer(char *buf)
{
  for (uint8_t i = 0; i < strlen(buf); i++)
  {
    buf[i] = '\0';
  }
}

uint8_t formatValue(char *buf, int32_t value, uint8_t decimal)
{
  // static char temp[STRING_LENGTH];
  // clearBuffer(temp);

  clearBuffer(buf);
  snprintf(buf, 22, "%d", value);
  uint8_t len = strlen(buf);

  if (decimal != 0)
  {
    uint8_t target = decimal + 1;
    uint8_t numLen = len - ((value < 0) ? 1 : 0);
    while (numLen < target)
    {
      for (uint8_t i = 0; i < numLen + 1; i++)
      // if negative, skip negative sign
      {
        buf[len - i + 1] = buf[len - i];
        buf[len - i] = '0';
      }
      buf[len + 1] = '\0';
      numLen++;
      len++;
    }
    // insert
    for (uint8_t i = 0; i < decimal + 1; i++)
    {
      buf[len - i + 1] = buf[len - i];
      buf[len - i] = '.';
    }
    // clearBuffer(buf);
    // snprintf(buf, STRING_LENGTH, "%d", target);
  }
  return strlen(buf);
}

#endif