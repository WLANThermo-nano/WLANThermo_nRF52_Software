/*************************************************** 
    Copyright (C) 2021  Martin Koerner

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

const uint16_t SERV_UUID_TY530 = 0xFB00u;
const uint16_t CHAR_UUID_TY530 = 0xFB02u;

class BleTemperatureTY530 : public BleSensorBase
{
public:
  BleTemperatureTY530(ble_gap_addr_t *peerAddress);
  void virtual connect(uint16_t bleConnHdl);
  void notify(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len);
  void disconnect(uint16_t conn_handle, uint8_t reason);

private:
  BLEClientService *bleServ;
  BLEClientCharacteristic *bleChar;
};
