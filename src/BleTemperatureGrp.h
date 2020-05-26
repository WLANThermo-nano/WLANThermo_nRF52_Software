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
#include "BleTemperatureBase.h"

#define MAX_TEMPERATURES 12u

class BleTemperatureGrp
{
public:
  BleTemperatureGrp();
  void init();
  void virtual update();
  void add(BleTemperatureBase *temperature);
  BleTemperatureBase* getTemperature(uint16_t conn_handle);
  BleTemperatureBase* getTemperature(ble_gap_addr_t *peerAddress);
  String getDevicesJson();
  BleTemperatureBase* operator[](int index);
  uint8_t count();
  static void notifyCb(BLEClientCharacteristic* chr, uint8_t* data, uint16_t len);
  static void indicateCb(BLEClientCharacteristic* chr, uint8_t* data, uint16_t len);

private:
  static void scanCb(ble_gap_evt_adv_report_t* report);
  static void connectCb(uint16_t conn_handle);
  static void disconnectCb(uint16_t conn_handle, uint8_t reason);

  static BleTemperatureBase *temperatures[MAX_TEMPERATURES];
  uint8_t addIndex;
};

extern BleTemperatureGrp gBleTemperatureGrp;