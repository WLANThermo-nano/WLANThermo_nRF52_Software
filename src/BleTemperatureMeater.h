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

const uint8_t SERV_UUID_MEATER[16] = {0x04, 0x3a, 0xb6, 0x08, 0xbc, 0x2d, 0x2a, 0xac, 0x8f, 0x48, 0x56, 0xc9, 0xfc, 0xc7, 0x5c, 0xa7};
const uint8_t CHAR_UUID_MEATER[16] = {0x76, 0x28, 0x1a, 0x99, 0xd1, 0x45, 0x9b, 0x90, 0xbf, 0x4b, 0x5e, 0x04, 0x74, 0xa7, 0xdd, 0x7e};

class BleTemperatureMeater : public BleTemperatureBase
{
public:
  BleTemperatureMeater(ble_gap_addr_t *peerAddress);
  void virtual connect(uint16_t bleConnHdl);
  void notify(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len);
  void disconnect(uint16_t conn_handle, uint8_t reason);
  void update();

private:
  float readTipTemperature(uint8_t *data);
  float readAmbientTemperature(uint8_t *data);
  BLEClientService *bleServ;
  BLEClientCharacteristic *bleChar;
};
