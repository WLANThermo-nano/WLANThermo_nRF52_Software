/*************************************************** 
    Copyright (C) 2021  Martin Koerner, Steffen Ochs

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

// UUIDs für Custom Firmware
//const uint16_t IDEN_UUID_LYWSD03 = 0x180Au;
//const uint16_t SERV_UUID_LYWSD03 = 0x181Au;
//const uint16_t CHAR_UUID_LYWSD03 = 0x2A1Fu;

const uint8_t IDEN_UUID_LYWSD03[16] = {0xa6, 0xa3, 0x7d, 0x99, 0xf2, 0x6f, 0x1a, 0x8a, 0x0c, 0x4b, 0x0a, 0x7a, 0xb0, 0xcc, 0xe0, 0xeb};
const uint8_t SERV_UUID_LYWSD03[16] = {0xa6, 0xa3, 0x7d, 0x99, 0xf2, 0x6f, 0x1a, 0x8a, 0x0c, 0x4b, 0x0a, 0x7a, 0xb0, 0xcc, 0xe0, 0xeb};
const uint8_t CHAR_UUID_LYWSD03[16] = {0xa6, 0xa3, 0x7d, 0x99, 0xf2, 0x6f, 0x1a, 0x8a, 0x0c, 0x4b, 0x0a, 0x7a, 0xc1, 0xcc, 0xe0, 0xeb};

// UUIDs für Hersteller Firmware
//static BLEUUID serviceUUID("ebe0ccb0-7a0a-4b0c-8a1a-6ff2997da3a6");
//static BLEUUID    charUUID("ebe0ccc1-7a0a-4b0c-8a1a-6ff2997da3a6");

class BleTemperatureLYWSD03 : public BleSensorBase
{
public:
  BleTemperatureLYWSD03(ble_gap_addr_t *peerAddress);
  void virtual connect(uint16_t bleConnHdl);
  void notify(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len);
  void disconnect(uint16_t conn_handle, uint8_t reason);

private:
  BLEClientService *bleServ;
  BLEClientCharacteristic *bleChar;
};
