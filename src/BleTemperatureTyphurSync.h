/*************************************************** 
    Copyright (C) 2026 Steffen Ochs

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

/* Typhur Sync probe (advertised name: WPTBBD03...)
 * Typhur Sync One / Dual (old version)
 * Service 0x00FF
 *   0xFF01  NOTIFY, READ            -> measurement data, 22 byte payload
 *   0xFF02  WRITE NO RESPONSE       -> command channel, unused so far
 * A second service 0xEE00 exists (config / OTA?) and is not used here.
 */
const uint16_t SERV_UUID_TYPHURSYNC = 0x00FFu;
const uint16_t CHAR_UUID_TYPHURSYNC = 0xFF01u;

/* Expected payload length. Anything else is discarded. */
#define TYPHURSYNC_PAYLOAD_LEN 20u

class BleTemperatureTyphurSync : public BleSensorBase
{
public:
  BleTemperatureTyphurSync(ble_gap_addr_t *peerAddress);
  void virtual connect(uint16_t bleConnHdl);
  void notify(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len);
  void disconnect(uint16_t conn_handle, uint8_t reason);
 
private:
  static uint16_t readU16(const uint8_t *data, uint8_t offset);
  BLEClientService *bleServ;
  BLEClientCharacteristic *bleChar;
};