#ifndef DRAWING_UTILS_H
#define DRAWING_UTILS_H

#include "colors.h"
#include "text_utils.h"

void drawRoundedBox(int x, int y, int w, int h, uint16_t color) {
  const int radius = 3;
  
  // Draw rounded corners
  // display.drawCircle(x + radius, y + radius, radius, color);
  // display.drawCircle(x + w - radius - 1, y + radius, radius, color);
  // display.drawCircle(x + radius, y + h - radius - 1, radius, color);
  // display.drawCircle(x + w - radius - 1, y + h - radius - 1, radius, color);
  
  // // Draw straight edges
  // display.drawFastHLine(x + radius, y, w - (2 * radius), color);
  // display.drawFastHLine(x + radius, y + h - 1, w - (2 * radius), color);
  // display.drawFastVLine(x, y + radius, h - (2 * radius), color);
  // display.drawFastVLine(x + w - 1, y + radius, h - (2 * radius), color);
  
  // // Draw inner border
  // display.drawRoundRect(x + 1, y + 1, w - 2, h - 2, radius - 1, color);
}

void drawCenteredText(int x, int y, int w, int h, const char* text, int textSize, uint16_t color) {
  if (isTextNotInList(text)) {
    display.fillRect(x+10, y-5, w-20, 35, BLACK);
  }
  display.setTextSize(textSize);
  display.setTextColor(color, BLACK);
  int textX = getCenteredX(x, w, text, textSize);
  int textY = getCenteredY(y, h-5, textSize);
  display.setCursor(textX, textY);
  display.print(text);
}

void drawCenteredTextSmall(int x, int y, int w, int h, const char* text, int textSize, uint16_t color) {
  display.setTextSize(textSize);
  display.setTextColor(color, BLACK);
  int textX = getCenteredX(x, w, text, textSize);
  int textY = getCenteredY(y, h, textSize);
  display.setCursor(textX, textY);
  display.print(text);
}

void drawSmallButton(int x, int y, const char* label, bool value) {
  const int BTN_WIDTH = 60;
  const int BTN_HEIGHT = 35;
  uint16_t color = WHITE;

  if (value) {
    color = GREEN;
  }
  
  drawRoundedBox(x, y, BTN_WIDTH, BTN_HEIGHT, color);
  drawCenteredTextSmall(x, y, BTN_WIDTH, BTN_HEIGHT, label, 2, color);
}

void drawRPMBarBlocks(int rpm, int maxRPM = 6000) {

  // Area bar chart
  int startX = 120;     // Posisi X awal
  int startY[30] = {80, 75, 70, 65, 60, 57, 54, 51, 48, 46, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45};
  int blockWidth = 6; // Lebar setiap blok
  int blockHeight = 70; // Tinggi setiap blok
  int spacing = 2;     // Jarak antar blok
  int numBlocks = 30;  // Jumlah maksimum blok

  // Hitung jumlah blok berdasarkan nilai RPM
  int filledBlocks = map(rpm, 0, maxRPM, 0, numBlocks);
  // Gambar blok satu per satu
  for (int i = 0; i < numBlocks; i++) {
    int x = startX + i * (blockWidth + spacing);
    int y = startY[i];

    uint16_t color = BLACK; // Default warna blok kosong
    if (i < filledBlocks) {
      if (i < numBlocks * 0.6) {
        color = GREEN;
      } else if (i < numBlocks * 0.8) {
        color = YELLOW;
      } else {
        color = RED;
      }
 
    } else {

    }

    if (color == BLACK) {
      display.fillRect(x, y, blockWidth, blockHeight, color);
    } else {
      display.fillRect(x, y, blockWidth, blockHeight, color);
    }

    // display.drawRect(x, y, blockWidth, blockHeight, WHITE); // Outline blok
  }
}

#endif