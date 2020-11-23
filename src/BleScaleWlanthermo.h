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

#include "BleSensorBase.h"

const uint8_t SERV_UUID_SCALE_WLANTHERMO[UUID128_SIZE] = {0xd1, 0x52, 0x20, 0xfb, 0xf0, 0xee, 0xd3, 0xbf, 0x75, 0x41, 0x73, 0xd7, 0x00, 0x14, 0x0a, 0xab};
const uint8_t CHAR_UUID_SCALE_WLANTHERMO[UUID128_SIZE] = {0xd1, 0x52, 0x20, 0xfb, 0xf0, 0xee, 0xd3, 0xbf, 0x75, 0x41, 0x73, 0xd7, 0x01, 0x14, 0x0a, 0xab};

class BleScaleWlanthermo : public BleSensorBase
{
  public:
    BleScaleWlanthermo(ble_gap_addr_t *peerAddress);
    void virtual connect(uint16_t bleConnHdl);
    void notify(BLEClientCharacteristic* chr, uint8_t* data, uint16_t len);
    void disconnect(uint16_t conn_handle, uint8_t reason);
    float getValue(uint8_t index);
  
  private:
    BLEClientService        *bleServ;
    BLEClientCharacteristic *bleChar;
};
