#include "Arduino.h"
#include "Comms.h"

void requestData(uint16_t timeout)
{
  Serial1.setTimeout(timeout);

  // flush input buffer

  Serial1.write('n');

  // wait for data or timeout
  uint32_t start = millis();
  uint32_t end = start;
  while (Serial1.available() < 3 && (end - start) < timeout)
  {
    end = millis();
    delay(1); 
  }

  // if within timeout, read data
  if (end - start < timeout && Serial1.available() >= 3)
  {
    // skip first two bytes
    Serial1.read(); // 'n'
    Serial1.read(); // 0x32
    uint8_t dataLen = Serial1.read();
    if (dataLen <= DATA_LEN) {
      Serial1.readBytes(buffer, dataLen);
    } else {
      // Tindakan error, misalnya data terlalu panjang
      Serial.println("Data overflow: Invalid data length");
      Serial.println(dataLen);
    }
  }
}

bool getBit(uint16_t address, uint8_t bit) {
  if (address < DATA_LEN) {
    return bitRead(buffer[address], bit);
  }
  return false; // jika alamat melebihi buffer
}
uint8_t getByte(uint16_t address) {
  if (address < DATA_LEN) {
    return buffer[address];
  }
  return 0; // default value jika address melebihi buffer
}

uint16_t getWord(uint16_t address) {
  if (address < DATA_LEN - 1) {
    return makeWord(buffer[address + 1], buffer[address]);
  }
  return 0; // default value jika address melebihi buffer
}