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
#include "BleTemperatureG32.h"
#include <ArduinoLog.h>

#define G32_NUM_OF_TEMPERATURES 8u

BleTemperatureG32::BleTemperatureG32(ble_gap_addr_t *peerAddress) : BleSensorBase(peerAddress, G32_NUM_OF_TEMPERATURES, false)
{
  bleServ = new BLEClientService(BLEUuid(SERV_UUID_G32));
  bleChar = new BLEClientCharacteristic(BLEUuid(CHAR_UUID_G32));
  bleChar->setNotifyCallback(BleSensorGrp::notifyCb);

  bleServ->begin();
  bleChar->begin();

  Bluefruit.Central.connect(peerAddress);
}

void BleTemperatureG32::connect(uint16_t bleConnHdl)
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

  Log.notice("Discovering G32 service ... ");

  // Check for service
  if (!bleServ->discover(bleConnHdl))
  {
    Log.notice("failed!" CR);
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);

  // Check characteristic
  Log.notice("Discovering G32 characteristic ... ");
  if (!bleChar->discover())
  {
    Log.notice("failed!" CR);
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);

  // Enable notification
  Log.notice("Enabling G32 notification ... ");
  if (!bleChar->enableNotify())
  {
    Log.notice("failed!" CR);
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);

  this->connected = true;
}

void BleTemperatureG32::notify(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len)
{
  this->lastSeen = 0u;
  Log.notice("----------- G32 data -----------" CR);

  // https://github.com/fschwarz86/g32
  // Sensor 1: (x[6]*100.0+x[7])/10.0
  // Sensor 2: (x[8]*100.0+x[9])/10.0
  // Sensor 3: (x[10]*100.0+x[11])/10.0
  // Sensor 4: (x[12]*100.0+x[13])/10.0
  // ExSensor 1: (x[14]*100.0+x[15])/10.0
  // ExSensor 2: (x[16]*100.0+x[17])/10.0
  // ExSensor 3: (x[18]*100.0+x[19])/10.0
  // ExSensor 4: (x[20]*100.0+x[21])/10.0

  uint8_t probeId = 0;
  for (uint8_t i = 6u; i <= 20; i = i + 2)
  {
    uint16_t t = 0;
    t = data[i]*100+data[i+1];
    currentValue[probeId] = t / 10.0;

    if (currentValue[probeId] == 1500)
    {
      currentValue[probeId] = 999.0;
    }

    Log.notice("Probe %d has value %F" CR, probeId, currentValue[probeId]);
    probeId++;
  }

  Log.verbose("Raw data: ");

  for (uint8_t i = 0u; i < len; i++)
    Log.verbose("%x ", data[i]);

  Log.verbose(CR);
}

void BleTemperatureG32::disconnect(uint16_t conn_handle, uint8_t reason)
{
  this->bleConnHdl = INVALID_BLE_CONN_HANDLE;
  this->connected = false;
}
