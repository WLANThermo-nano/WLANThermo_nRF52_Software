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
#include "BleTemperatureTY530.h"
#include <ArduinoLog.h>

#define TY530_NUM_OF_TEMERATURES 2u

BleTemperatureTY530::BleTemperatureTY530(ble_gap_addr_t *peerAddress) : BleSensorBase(peerAddress, TY530_NUM_OF_TEMERATURES, false)
{
  bleServ = new BLEClientService(BLEUuid(SERV_UUID_TY530));
  bleChar = new BLEClientCharacteristic(BLEUuid(CHAR_UUID_TY530));
  bleChar->setNotifyCallback(BleSensorGrp::notifyCb);

  bleServ->begin();
  bleChar->begin();

  Bluefruit.Central.connect(peerAddress);
}

void BleTemperatureTY530::connect(uint16_t bleConnHdl)
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

  Log.notice("Discovering TY530 service ... ");

  // Check for service
  if (!bleServ->discover(bleConnHdl))
  {
    Log.notice("failed!" CR);
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);

  // Check characteristic
  Log.notice("Discovering TY530 characteristic ... ");
  if (!bleChar->discover())
  {
    Log.notice("failed!" CR);
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);

  // Enable notification
  Log.notice("Enabling TY530 notification ... ");
  if (!bleChar->enableNotify())
  {
    Log.notice("failed!" CR);
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);

  this->connected = true;
}

void BleTemperatureTY530::notify(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len)
{
  currentValue[0] = ((float((data[3] << 8u) + data[2])) / 10.0f) - 40.0f;
  currentValue[1] = ((float((data[5] << 8u) + data[4])) / 10.0f) - 40.0f;
  this->lastSeen = 0u;

  Log.notice("----------- TY530 data -----------" CR);
  Log.notice("Tip temperature: %F" CR, currentValue[0]);
  Log.notice("Ambient temperature: %F" CR, currentValue[1]);

  Log.verbose("Raw data: ");

  logRAW(data, len);
}

void BleTemperatureTY530::disconnect(uint16_t conn_handle, uint8_t reason)
{
  this->bleConnHdl = INVALID_BLE_CONN_HANDLE;
  this->connected = false;
}
