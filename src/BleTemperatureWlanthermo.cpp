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

#include "BleTemperatureGrp.h"
#include "BleTemperatureWlanthermo.h"
#include <ArduinoLog.h>

#define WLANTHERMO_NUM_OF_TEMERATURES 2u

BleTemperatureWlanthermo::BleTemperatureWlanthermo(ble_gap_addr_t *peerAddress) : BleTemperatureBase(peerAddress, WLANTHERMO_NUM_OF_TEMERATURES)
{
  bleServ = new BLEClientService(BLEUuid(SERV_UUID_WLANTHERMO));
  bleChar = new BLEClientCharacteristic(BLEUuid(CHAR_UUID_WLANTHERMO));
  bleChar->setNotifyCallback(BleTemperatureGrp::notifyCb);

  bleServ->begin();
  bleChar->begin();
}

void BleTemperatureWlanthermo::update()
{
  BleTemperatureBase::update();
}

void BleTemperatureWlanthermo::connect(uint16_t bleConnHdl)
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

void BleTemperatureWlanthermo::notify(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len)
{
  currentValue[0] = ((float)(data[0] + (data[1] << 8))) / 10.0;
  currentValue[1] = ((float)(data[2] + (data[3] << 8))) / 10.0;
  this->lastSeen = 0u;

  Log.notice("----------- Wlanthermo data -----------" CR);
  Log.notice("Temperature 0: %F" CR, currentValue[0]);
  Log.notice("Temperature 1: %F" CR, currentValue[1]);

  Log.verbose("Raw data: ");

  for (uint8_t i = 0u; i < len; i++)
    Log.verbose("%X ", data[i]);

  Log.verbose(CR);
}

void BleTemperatureWlanthermo::disconnect(uint16_t conn_handle, uint8_t reason)
{
  this->bleConnHdl = INVALID_BLE_CONN_HANDLE;
  this->connected = false;
}
