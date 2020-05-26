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

const uint8_t SERV_UUID_WLANTHERMO[16] = {0x71, 0x4d, 0x0e, 0xb3, 0x19, 0x4e, 0x9b, 0xb6, 0x08, 0x4d, 0x95, 0x8f, 0x00, 0x14, 0x0c, 0xd6};
const uint8_t CHAR_UUID_WLANTHERMO[16] = {0x71, 0x4d, 0x0e, 0xb3, 0x19, 0x4e, 0x9b, 0xb6, 0x08, 0x4d, 0x95, 0x8f, 0x01, 0x14, 0x0c, 0xd6};

class BleTemperatureWlanthermo : public BleTemperatureBase
{
  public:
    BleTemperatureWlanthermo(ble_gap_addr_t *peerAddress);
    void virtual connect(uint16_t bleConnHdl);
    void notify(BLEClientCharacteristic* chr, uint8_t* data, uint16_t len);
    void disconnect(uint16_t conn_handle, uint8_t reason);
    void update();
  private:
    BLEClientService        *bleServ;
    BLEClientCharacteristic *bleChar;
};
