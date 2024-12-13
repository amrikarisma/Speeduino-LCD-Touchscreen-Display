#include "TFT_eSPI.h"
#ifndef DRAWING_UTILS_H
#define DRAWING_UTILS_H

#include "text_utils.h"

void drawRoundedBox(int x, int y, int w, int h, uint16_t color) {
  const int radius = 3;

  // Draw rounded corners
  display.fillCircle(x + radius, y + radius, radius, color);
  display.fillCircle(x + w - radius - 1, y + radius, radius, color);
  display.fillCircle(x + radius, y + h - radius - 1, radius, color);
  display.fillCircle(x + w - radius - 1, y + h - radius - 1, radius, color);
  
  // Draw straight edges
  display.fillRect(x + radius, y, w - (2 * radius), radius, color); // Top edge
  display.fillRect(x + radius, y + h - radius, w - (2 * radius), radius, color); // Bottom edge
  display.fillRect(x, y + radius, radius, h - (2 * radius), color); // Left edge
  display.fillRect(x + w - radius, y + radius, radius, h - (2 * radius), color); // Right edge
  
  // Draw inner border
  display.drawRoundRect(x + 1, y + 1, w - 2, h - 2, radius - 1, color);
}

void drawCenteredText(int x, int y, int w, int h, const char* text, int textSize, uint16_t color) {
  if (isTextNotInList(text)) {
    display.fillRect(x + 10, y - 5, w - 20, 35, TFT_BLACK); // Clear previous text area if not in the list
  }
  display.setTextSize(textSize);
  display.setTextColor(color, TFT_BLACK);
  int textX = getCenteredX(x, w, text, textSize);
  int textY = getCenteredY(y, h - 15, textSize);
  display.drawString(text, textX, textY);  // Use drawString instead of setCursor + print
}

void drawCenteredTextSmall(int x, int y, int w, int h, const char* text, int textSize, uint16_t color) {
  display.setTextSize(textSize);
  display.setTextColor(color, TFT_BLACK);
  int textX = getCenteredX(x, w, text, textSize);
  int textY = getCenteredY(y, h, textSize);
  display.drawString(text, textX, textY);  // Use drawString instead of setCursor + print
}

void drawSmallButton(int x, int y, const char* label, bool value) {
  const int BTN_WIDTH = 60;
  const int BTN_HEIGHT = 35;
  uint16_t color = TFT_WHITE;

  if (value) {
    color = TFT_GREEN;
  }
  
  // drawRoundedBox(x, y, BTN_WIDTH, BTN_HEIGHT, color);
  drawCenteredTextSmall(x, y, BTN_WIDTH, BTN_HEIGHT, label, 2, color);
}

void drawRPMBarBlocks(int rpm, int maxRPM = 6000) {
  int startX = 120;     // Starting X position
  int startY[30] = {80, 75, 70, 65, 60, 57, 54, 51, 48, 46, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45};
  int blockWidth = 6; // Width of each block
  int blockHeight = 70; // Height of each block
  int spacing = 2;     // Spacing between blocks
  int numBlocks = 30;  // Total number of blocks

  // Calculate number of filled blocks based on RPM value
  int filledBlocks = map(rpm, 0, maxRPM, 0, numBlocks);
  
  // Draw the blocks one by one
  for (int i = 0; i < numBlocks; i++) {
    int x = startX + i * (blockWidth + spacing);
    int y = startY[i];
    uint16_t color = TFT_BLACK; // Default color for empty block
    
    // Assign color based on block's position and RPM value
    if (i < filledBlocks) {
      if (i < numBlocks * 0.6) {
        color = TFT_GREEN;
      } else if (i < numBlocks * 0.8) {
        color = TFT_YELLOW;
      } else {
        color = TFT_RED;
      }
    }

    display.fillRect(x, y, blockWidth, blockHeight, color);
  }
}

#endif