/*************************************************** 
    Copyright (C) 2024  Steffen Ochs

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

#include "BleSensorBase.h"

const uint8_t SERV_UUID_MEATER2[16] = {0x8b, 0xcf, 0x55, 0x45, 0xe5, 0xe1, 0xDD, 0xa0, 0x54, 0x4e, 0xf1, 0x59, 0x6c, 0x74, 0xe2, 0xc9};
const uint8_t CHAR_UUID_MEATER2[16] = {0x76, 0x28, 0x1a, 0x99, 0xd1, 0x45, 0x9b, 0x90, 0xbf, 0x4b, 0x5e, 0x04, 0x74, 0xa7, 0xdd, 0x7e};

class BleTemperatureMeater2 : public BleSensorBase
{
public:
  BleTemperatureMeater2(ble_gap_addr_t *peerAddress);
  void virtual connect(uint16_t bleConnHdl);
  void notify(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len);
  void disconnect(uint16_t conn_handle, uint8_t reason);

private:
  float readAmbientTemperature(uint8_t *data);
  BLEClientService *bleServ;
  BLEClientCharacteristic *bleChar;
};
