/***************************************************
    Copyright (C) 2024  Steffen Ochs

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
#include "BleTemperatureMeater2.h"
#include <ArduinoLog.h>

#define MEATER2_NUM_OF_TEMERATURES 6u

BleTemperatureMeater2::BleTemperatureMeater2(ble_gap_addr_t *peerAddress) : BleSensorBase(peerAddress, MEATER2_NUM_OF_TEMERATURES, false)
{
  bleServ = new BLEClientService(BLEUuid(SERV_UUID_MEATER2));
  bleChar = new BLEClientCharacteristic(BLEUuid(CHAR_UUID_MEATER2));
  bleChar->setNotifyCallback(BleSensorGrp::notifyCb);

  bleServ->begin();
  bleChar->begin();

  Bluefruit.Central.connect(peerAddress);
}

void BleTemperatureMeater2::connect(uint16_t bleConnHdl)
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

  Log.notice("Discovering Meater2 service ... ");

  // Check for service
  if (!bleServ->discover(bleConnHdl))
  {
    Log.notice("failed!" CR);
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);

  // Check characteristic
  Log.notice("Discovering Meater2 characteristic ... ");
  if (!bleChar->discover())
  {
    Log.notice("failed!" CR);
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);

  // Enable notification
  Log.notice("Enabling Meater2 notification ... ");
  if (!bleChar->enableNotify())
  {
    Log.notice("failed!" CR);
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);

  this->connected = true;
}

float BleTemperatureMeater2::readAmbientTemperature(uint8_t *data)
{
  uint16_t t = 0;
  float temp = 0;
  t = data[11] << 8;
  t += data[10];
  temp = t / 32.0;
  if (temp > 2000)
  {
    temp = temp - 2048;
  }
  return temp;
}

void BleTemperatureMeater2::notify(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len)
{
  this->lastSeen = 0u;
  Log.notice("----------- Meater2 data -----------" CR);

  uint8_t probeId = 0;
  for (uint8_t i = 0u; i <= 8; i = i + 2)
  {
    uint16_t t = 0;
    t = data[i + 1] << 8;
    t += data[i];
    currentValue[probeId] = t / 32.0;
    if (currentValue[probeId] > 2000)
    {
      currentValue[probeId] = currentValue[probeId] - 2048;
    }

    Log.notice("Probe %d has value %F" CR, probeId, currentValue[probeId]);
    probeId++;
  }

  currentValue[5] = readAmbientTemperature(data);
  Log.notice("Ambient has value %F" CR, currentValue[5]);

  logRAW(data, len);
}

void BleTemperatureMeater2::disconnect(uint16_t conn_handle, uint8_t reason)
{
  this->bleConnHdl = INVALID_BLE_CONN_HANDLE;
  this->connected = false;
}
