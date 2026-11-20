/***************************************************
    Copyright (C) 2026  fsedarkalex

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

// Ooni Pizza Hub ("Ooni_DT_Hub")
// The hub does not advertise any service UUID, it is detected by its local name.
#define OONI_HUB_NAME "Ooni_DT_Hub"

// 69400001-b5a3-f393-e0a9-e50e24dcca99 (little endian)
const uint8_t SERV_UUID_OONI[16] = {0x99, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x01, 0x00, 0x40, 0x69};
// 69400002-b5a3-f393-e0a9-e50e24dcca99 (little endian), notify, 22 byte frame @ 1 Hz
const uint8_t CHAR_UUID_OONI[16] = {0x99, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x02, 0x00, 0x40, 0x69};

class BleTemperatureOoni : public BleSensorBase
{
public:
  BleTemperatureOoni(ble_gap_addr_t *peerAddress);
  static boolean isOoniHub(ble_gap_evt_adv_report_t *report);
  void virtual connect(uint16_t bleConnHdl);
  void notify(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len);
  void disconnect(uint16_t conn_handle, uint8_t reason);

private:
  BLEClientService *bleServ;
  BLEClientCharacteristic *bleChar;
};
