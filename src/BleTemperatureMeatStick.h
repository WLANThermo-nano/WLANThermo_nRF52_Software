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

const uint8_t CHAR_UUID_MEATSTICK_T0[16] = {0x11, 0x88, 0x55, 0x00, 0x44, 0x66, 0x77, 0x88, 0x99, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
const uint8_t CHAR_UUID_MEATSTICK_T1[16] = {0x22, 0x88, 0x55, 0x00, 0x44, 0x11, 0x00, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11};
const uint8_t CHAR_UUID_MEATSTICK_T2[16] = {0x33, 0x88, 0x55, 0x00, 0x44, 0x88, 0x99, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x11, 0x22};

class BleTemperatureMeatStick : public BleTemperatureBase
{
public:
  BleTemperatureMeatStick(ble_gap_addr_t *peerAddress, BeaconType *beacon);
  void update();
  void readBeacon(uint8_t *advData, uint8_t advDataLength);
  static boolean hasMeatStickData(BeaconType *beacon);
};
