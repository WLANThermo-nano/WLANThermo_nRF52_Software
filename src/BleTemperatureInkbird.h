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

#include "BleTemperatureBase.h"

/*const uint8_t SERV_UUID_INKBIRD[16]              = {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xf0, 0xff, 0x00, 0x00};
const uint8_t CHAR_UUID_INKBIRD_LOGIN [16]       = {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xf2, 0xff, 0x00, 0x00};
const uint8_t CHAR_UUID_INKBIRD_REALTIMEDATA[16] = {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xf4, 0xff, 0x00, 0x00};
const uint8_t CHAR_UUID_INKBIRD_SETTINGS [16]    = {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xf5, 0xff, 0x00, 0x00};*/

const uint16_t SERV_UUID_INKBIRD = 0xFFF0u;
const uint16_t CHAR_UUID_INKBIRD_LOGIN = 0xFFF2u;
const uint16_t CHAR_UUID_INKBIRD_REALTIMEDATA = 0xFFF4u;
const uint16_t CHAR_UUID_INKBIRD_SETTINGS = 0xFFF5u;

class BleTemperatureInkbird : public BleTemperatureBase
{
public:
  BleTemperatureInkbird(ble_gap_addr_t *peerAddress);
  void virtual connect(uint16_t bleConnHdl);
  void notify(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len);
  void disconnect(uint16_t conn_handle, uint8_t reason);
  void update();

private:
  boolean writeSettings(const uint8_t *data, uint32_t length);
  BLEClientService *bleServ;
  BLEClientCharacteristic *bleCharLogin;
  BLEClientCharacteristic *bleCharRealtimeData;
  BLEClientCharacteristic *bleCharSettings;
};
