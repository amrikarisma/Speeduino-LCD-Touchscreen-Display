#ifndef TEXT_UTILS_H
#define TEXT_UTILS_H

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

#endif