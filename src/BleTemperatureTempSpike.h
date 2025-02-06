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

const uint8_t SERV_UUID_TEMPSPIKE[16] = {0x0F, 0x33, 0xF9, 0x4D, 0x4F, 0xA2, 0x54, 0x23, 0xA9, 0xC5, 0x14, 0xEB, 0xAB, 0x7C, 0xBB, 0x93};
const uint8_t SERV_UUID_TEMPSPIKE_ADVERTISER[16] = {0xbd, 0x42, 0xd9, 0xfd, 0xe6, 0x2e, 0x55, 0xdb, 0xba, 0xd1, 0x6b, 0x6f, 0x31, 0xb6, 0xfb, 0x72};
const uint8_t CHAR_UUID_TEMPSPIKE[16] = {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x01, 0xff, 0x00, 0x00};

// Fix for TempSpike
class BLEClientCharacteristicTempSpike : public BLEClientCharacteristic
{
public:
  BLEClientCharacteristicTempSpike(BLEUuid bleuuid) : BLEClientCharacteristic(bleuuid){};
  bool writeCCCD(uint16_t value) override
  // diese Funktion ueberschreibt die urspruengliche Funktion in der BLEClientCharacteristic, dazu muss die urspruengliche Funktion um ein "virtual" ergaenzt werden
  // zusaetzlich muessen die privaten Variablen der BLEClientCharacteristic auf protected umgestellt werden
  {
    const uint16_t conn_handle = _service->connHandle();

    ble_gattc_write_params_t param =
        {
            .write_op = BLE_GATT_OP_WRITE_REQ,
            .flags = BLE_GATT_EXEC_WRITE_FLAG_PREPARED_WRITE,
            .handle = _cccd_handle,
            .offset = 0,
            .len = 2,
            .p_value = (uint8_t *)&value};

    // TODO only Write without response consume a TX buffer
    BLEConnection *conn = Bluefruit.Connection(conn_handle);
    VERIFY(conn && conn->getWriteCmdPacket());

    VERIFY_STATUS(sd_ble_gattc_write(conn_handle, &param), false);

    return true;
  };
};


class BleTemperatureTempSpike : public BleSensorBase
{
public:
  BleTemperatureTempSpike(ble_gap_addr_t *peerAddress);
  void virtual connect(uint16_t bleConnHdl);
  void notify(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len);
  void disconnect(uint16_t conn_handle, uint8_t reason);

private:
  float readTipTemperature(uint8_t *data);
  float readAmbientTemperature(uint8_t *data);
  uint8_t indicateDuoChannel(uint8_t *data);
  BLEClientService *bleServ;
  BLEClientCharacteristicTempSpike *bleChar;
};
