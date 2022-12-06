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

#include "BleSensorGrp.h"
#include "BleTemperatureLYWSD03.h"
#include <ArduinoLog.h>

#define LYWSD03_NUM_OF_TEMPERATURES 1u

BleTemperatureLYWSD03::BleTemperatureLYWSD03(ble_gap_addr_t *peerAddress) : BleSensorBase(peerAddress, LYWSD03_NUM_OF_TEMPERATURES, false)
{
  bleServ = new BLEClientService(BLEUuid(SERV_UUID_LYWSD03));
  bleChar = new BLEClientCharacteristic(BLEUuid(CHAR_UUID_LYWSD03));
  bleChar->setNotifyCallback(BleSensorGrp::notifyCb);

  bleServ->begin();
  bleChar->begin();

  Bluefruit.Central.connect(peerAddress);
}

void BleTemperatureLYWSD03::connect(uint16_t bleConnHdl)
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

  // disconnect if disabled
  if (false == enabled)
  {
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("Discovering LYWSD03 service ... ");

  // Check for service
  if (!bleServ->discover(bleConnHdl))
  {
    Log.notice("failed!" CR);
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);

  // Check characteristic
  Log.notice("Discovering LYWSD03 characteristic ... ");
  if (!bleChar->discover())
  {
    Log.notice("failed!" CR);
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);

  // Enable notification
  Log.notice("Enabling LYWSD03 notification ... ");
  if (!bleChar->enableNotify())
  {
    Log.notice("failed!" CR);
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);

  this->connected = true;
}

void BleTemperatureLYWSD03::notify(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len)
{
  currentValue[0] = ((float(data[0] | (data[1] << 8u))) * 0.01f);  // temperarure
  currentValue[1] = (float(data[2]));  // humidity
  this->lastSeen = 0u;

  Log.notice("----------- LYWSD03 data -----------" CR);
  Log.notice("temperature: %F" CR, currentValue[0]);
  Log.notice("humidity: %F" CR, currentValue[1]);

  Log.verbose("Raw data: ");

  for (uint8_t i = 0u; i < len; i++)
    Log.verbose("%x ", data[i]);

  Log.verbose(CR);
}

void BleTemperatureLYWSD03::disconnect(uint16_t conn_handle, uint8_t reason)
{
  this->bleConnHdl = INVALID_BLE_CONN_HANDLE;
  this->connected = false;
}
