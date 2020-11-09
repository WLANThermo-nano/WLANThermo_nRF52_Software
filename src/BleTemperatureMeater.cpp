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
#include "BleTemperatureMeater.h"
#include <ArduinoLog.h>

#define MEATER_NUM_OF_TEMERATURES 2u

BleTemperatureMeater::BleTemperatureMeater(ble_gap_addr_t *peerAddress) : BleTemperatureBase(peerAddress, MEATER_NUM_OF_TEMERATURES, false)
{
  bleServ = new BLEClientService(BLEUuid(SERV_UUID_MEATER));
  bleChar = new BLEClientCharacteristic(BLEUuid(CHAR_UUID_MEATER));
  bleChar->setNotifyCallback(BleTemperatureGrp::notifyCb);

  bleServ->begin();
  bleChar->begin();

  Bluefruit.Central.connect(peerAddress);
}

void BleTemperatureMeater::connect(uint16_t bleConnHdl)
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
  }

  Log.notice("Discovering Meater service ... ");

  // Check for service
  if (!bleServ->discover(bleConnHdl))
  {
    Log.notice("failed!" CR);
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);

  // Check characteristic
  Log.notice("Discovering Meater characteristic ... ");
  if (!bleChar->discover())
  {
    Log.notice("failed!" CR);
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);

  // Enable notification
  Log.notice("Enabling Meater notification ... ");
  if (!bleChar->enableNotify())
  {
    Log.notice("failed!" CR);
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);

  this->connected = true;
}

float BleTemperatureMeater::readTipTemperature(uint8_t *data)
{
  SplitTwoBytes tip;
  tip.lowByte = data[0];
  tip.highByte = data[1];
  return (float(tip.value) + 8.0) / 16.0;
}

float BleTemperatureMeater::readAmbientTemperature(uint8_t *data)
{
  SplitTwoBytes tip;
  SplitTwoBytes ambientRa;
  SplitTwoBytes ambientOa;
  tip.lowByte = data[0];
  tip.highByte = data[1];
  ambientRa.lowByte = data[2];
  ambientRa.highByte = data[3];
  ambientOa.lowByte = data[4];
  ambientOa.highByte = data[5];
  return ((tip.value + (max(0, ((((ambientRa.value - min(48, ambientOa.value)) * 16) * 589)) / 1487))) + 8.0) / 16.0;
}

void BleTemperatureMeater::notify(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len)
{
  currentValue[0] = readTipTemperature(data);
  currentValue[1] = readAmbientTemperature(data);
  this->lastSeen = 0u;

  Log.notice("----------- Meater data -----------" CR);
  Log.notice("Tip temperature: %F" CR, currentValue[0]);
  Log.notice("Ambient temperature: %F" CR, currentValue[1]);

  Log.verbose("Raw data: ");

  for (uint8_t i = 0u; i < len; i++)
    Log.verbose("%x ", data[i]);

  Log.verbose(CR);
}

void BleTemperatureMeater::disconnect(uint16_t conn_handle, uint8_t reason)
{
  this->bleConnHdl = INVALID_BLE_CONN_HANDLE;
  this->connected = false;
}
