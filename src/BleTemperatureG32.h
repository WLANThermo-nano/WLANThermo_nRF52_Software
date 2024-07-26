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

//service_uuid: 'dc0f41ea-b6ae-46a8-a19e-1a3bf4342bcb'
//charact_uuid: 'dc0f41e2-b6ae-46a8-a19e-1a3bf4342bcb'

const uint8_t SERV_UUID_G32[16] = {0xcb, 0x2b, 0x34, 0xf4, 0x3b, 0x1a, 0x9e, 0xa1, 0xa8, 0x46, 0xae, 0xb6, 0xea, 0x41, 0x0f, 0xdc};
const uint8_t CHAR_UUID_G32[16] = {0xcb, 0x2b, 0x34, 0xf4, 0x3b, 0x1a, 0x9e, 0xa1, 0xa8, 0x46, 0xae, 0xb6, 0xe2, 0x41, 0x0f, 0xdc};

class BleTemperatureG32 : public BleSensorBase
{
public:
  BleTemperatureG32(ble_gap_addr_t *peerAddress);
  void virtual connect(uint16_t bleConnHdl);
  void notify(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len);
  void disconnect(uint16_t conn_handle, uint8_t reason);

private:
  BLEClientService *bleServ;
  BLEClientCharacteristic *bleChar;
};
