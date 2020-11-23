/*************************************************** 
    Copyright (C) 2020  Martin Koerner

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    
    HISTORY: Please refer Github History
    
****************************************************/
#pragma once

#include <bluefruit.h>

#define INACTIVEVALUE 999
#define INVALID_BLE_CONN_HANDLE 0xFFFFu
#define NUM_OF_TEMPERATURES_DEFAULT 8u
#define UUID16_SIZE 2u
#define UUID128_SIZE 16u

union SplitTwoBytes {
  uint16_t value;
  struct ATTR_PACKED
  {
    uint8_t lowByte;
    uint8_t highByte;
  };
};

typedef struct ATTR_PACKED
{
  uint16_t manufacturer;
  uint8_t beaconType;
  uint8_t beaconLen;
  uint8_t uuid128[16];
  SplitTwoBytes major;
  SplitTwoBytes minor;
  int8_t rssi;
} BeaconType;

class BleSensorBase
{
public:
  BleSensorBase(ble_gap_addr_t *peerAddress, uint8_t numOfTemperatures, boolean beaconOnly);
  ~BleSensorBase();
  virtual float getValue(uint8_t index);
  uint8_t getValueCount() { return valueCount; };
  uint8_t getGlobalIndex();
  uint16_t getBleConnHdl() { return bleConnHdl; };
  String getName() { return name; };
  virtual String getUnit(uint8_t index = 0u) { return unit; };
  boolean isConnected() { return connected; };
  ble_gap_addr_t *getPeerAddress() { return &peerAddress; };
  String getPeerAddressString();
  uint32_t getLastSeen() { return lastSeen; };
  int8_t getRssi() { return rssi; };
  void advReceived(uint8_t *advData, uint8_t advDataLength);
  void enable(boolean enable) { enabled = enable; };
  void virtual connect(uint16_t bleConnHdl){};
  virtual void update();
  void virtual notify(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len){};
  void virtual indicate(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len){};
  void virtual disconnect(uint16_t conn_handle, uint8_t reason){};
  void virtual readBeacon(uint8_t *advData, uint8_t advDataLength){};

protected:
  uint8_t localIndex;
  uint8_t globalIndex;
  float *currentValue;
  uint16_t bleConnHdl;
  uint8_t valueCount;
  uint32_t lastSeen;
  boolean connected;
  boolean enabled;
  boolean beaconOnly;
  ble_gap_addr_t peerAddress;
  String name;
  String unit;
  int8_t rssi;

private:
  static uint8_t globalIndexTracker;
};