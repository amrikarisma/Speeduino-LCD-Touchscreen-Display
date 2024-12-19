#include "Arduino.h"
#include "Comms.h"

void requestData(uint16_t timeout)
{
  Serial.setTimeout(timeout);

  // flush input buffer

  Serial.write('n');

  // wait for data or timeout
  uint32_t start = millis();
  uint32_t end = start;
  while (Serial.available() < 3 && (end - start) < timeout)
  {
    end = millis();
  }

  // if within timeout, read data
  if (end - start < timeout && Serial.available() >= 3)
  {
    // skip first two bytes
    Serial.read(); // 'n'
    Serial.read(); // 0x32
    uint8_t dataLen = Serial.read();
    if (dataLen <= DATA_LEN) {
      Serial.readBytes(buffer, dataLen);
    }
  }
}

bool getBit(uint16_t address, uint8_t bit) {
  if (address < DATA_LEN) {
    return bitRead(buffer[address], bit);
  }
  return false;
}
uint8_t getByte(uint16_t address) {
  if (address < DATA_LEN) {
    return buffer[address];
  }
  return 0;
}

uint16_t getWord(uint16_t address) {
  if (address < DATA_LEN - 1) {
    return makeWord(buffer[address + 1], buffer[address]);
  }
  return 0;
}