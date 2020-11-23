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

#include "BleSensorGrp.h"
#include "BleScaleWlanthermo.h"
#include <ArduinoLog.h>

#define SCALE_WLANTHERMO_NUM_OF_TEMERATURES 1u

BleScaleWlanthermo::BleScaleWlanthermo(ble_gap_addr_t *peerAddress) : BleSensorBase(peerAddress, SCALE_WLANTHERMO_NUM_OF_TEMERATURES, false)
{
  bleServ = new BLEClientService(BLEUuid(SERV_UUID_SCALE_WLANTHERMO));
  bleChar = new BLEClientCharacteristic(BLEUuid(CHAR_UUID_SCALE_WLANTHERMO));
  bleChar->setNotifyCallback(BleSensorGrp::notifyCb);

  bleServ->begin();
  bleChar->begin();

  this->name = "WT Scale";
  this->unit = "kg";
}

void BleScaleWlanthermo::connect(uint16_t bleConnHdl)
{
  char buffer[30] = {0};

  this->bleConnHdl = bleConnHdl;

  BLEConnection *bleConnection = Bluefruit.Connection(bleConnHdl);

  if (bleConnection != NULL)
  {
    if (bleConnection->getPeerName(buffer, sizeof(buffer)))
    {
      name = buffer;
    }
  }

  Log.notice("Discovering Wlanthermo service ... ");

  // Check for service
  if (!bleServ->discover(bleConnHdl))
  {
    Log.notice("failed!" CR);
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);

  // Check characteristic
  Log.notice("Discovering Wlanthermo characteristic ... ");
  if (!bleChar->discover())
  {
    Log.notice("failed!" CR);
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);

  // Enable notification
  Log.notice("Enabling Wlanthermo notification ... ");
  if (!bleChar->enableNotify())
  {
    Log.notice("failed!" CR);
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);

  this->connected = true;
}

static uint32_t uint32_decode(const uint8_t *encodedData)
{
  return ((((uint32_t)((uint8_t *)encodedData)[0]) << 0u) |
          (((uint32_t)((uint8_t *)encodedData)[1]) << 8u) |
          (((uint32_t)((uint8_t *)encodedData)[2]) << 16u) |
          (((uint32_t)((uint8_t *)encodedData)[3]) << 24u));
}

void BleScaleWlanthermo::notify(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len)
{
  this->lastSeen = 0u;

  int32_t value = uint32_decode(data);

  currentValue[0] = (float)value / 1000;

  Log.notice("----------- Wlanthermo data -----------" CR);
  Log.notice("Weight: %d" CR, value);
  Log.notice("Weight: %F" CR, currentValue[0]);

  Log.verbose("Raw data: ");

  for (uint8_t i = 0u; i < len; i++)
    Log.verbose("%X ", data[i]);

  Log.verbose(CR);

  BLEConnection *bleConnection = Bluefruit.Connection(chr->connHandle());

  /* Update signal strength */
  if (bleConnection != NULL)
  {
    Log.notice("RSSI: %i dB" CR, bleConnection->getRssi());
  }
}

void BleScaleWlanthermo::disconnect(uint16_t conn_handle, uint8_t reason)
{
  this->bleConnHdl = INVALID_BLE_CONN_HANDLE;
  this->connected = false;
}

float BleScaleWlanthermo::getValue(uint8_t index = 0u)
{
  float value = INACTIVEVALUE;

  if ((index < this->valueCount) && this->connected)
    value = (float)((int)(this->currentValue[index] * 1000)) / 1000; //limit float

  return value;
}
