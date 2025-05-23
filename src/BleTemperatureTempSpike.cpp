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
#include "BleTemperatureTempSpike.h"
#include <ArduinoLog.h>

#define TEMPSPIKE_NUM_OF_TEMPERATURES 4u  // Duo
#define DUOCHANNEL 0x01
#define SENDDATA 0x10
#define SENDSTATUS 0x17
#define PROBEINSERT 0x01

BleTemperatureTempSpike::BleTemperatureTempSpike(ble_gap_addr_t *peerAddress) : BleSensorBase(peerAddress, TEMPSPIKE_NUM_OF_TEMPERATURES, false)
{

  bleServ = new BLEClientService(BLEUuid(SERV_UUID_TEMPSPIKE));
  bleChar = new BLEClientCharacteristicTempSpike(BLEUuid(CHAR_UUID_TEMPSPIKE));
  bleChar->setNotifyCallback(BleSensorGrp::notifyCb);

  bleServ->begin();
  bleChar->begin();

  // Bluefruit.Scanner.stop();

  Bluefruit.Central.connect(peerAddress);
}

void BleTemperatureTempSpike::connect(uint16_t bleConnHdl)
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

  Log.notice("Discovering TempSpike service ... ");

  // Check for service
  if (!bleServ->discover(bleConnHdl))
  {
    Log.notice("failed!" CR);
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);

  // Check characteristic
  Log.notice("Discovering TempSpike characteristic ... ");
  if (!bleChar->discover())
  {
    Log.notice("failed!" CR);
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);

  // Enable notification
  Log.notice("Enabling TempSpike notification ... ");
  if (!bleChar->enableNotify())
  {
    Log.notice("failed!" CR);
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);
  this->connected = true;
}

float BleTemperatureTempSpike::readTipTemperature(uint8_t *data)
{
  SplitTwoBytes tip;
  tip.lowByte = data[2];
  tip.highByte = data[3];
  return (float(tip.value) - 30);
}

float BleTemperatureTempSpike::readAmbientTemperature(uint8_t *data)
{

  SplitTwoBytes ambient;
  ambient.lowByte = data[6];
  ambient.highByte = data[7];

  return (float(ambient.value) - 30);
}

void BleTemperatureTempSpike::notify(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len)
{

  this->lastSeen = 0u;
  logRAW(data, len);

  if (data[0] == SENDDATA)
  {
    if (data[1] == DUOCHANNEL)  // Send data
    {
      currentValue[2] = readTipTemperature(data);
      currentValue[3] = readAmbientTemperature(data);
    }
    else
    {
      currentValue[0] = readTipTemperature(data);
      currentValue[1] = readAmbientTemperature(data);
    }
  } else // if (data[0] == SENDSTATUS && data[2] == PROBEINSERT) // Insert box
  {
  if (data[1] == DUOCHANNEL)
  {
    currentValue[2] = INACTIVEVALUE;
    currentValue[3] = INACTIVEVALUE;
  } else
  {
    currentValue[0] = INACTIVEVALUE;
    currentValue[1] = INACTIVEVALUE;
  }
  }

  Log.notice("----------- TempSpike data -----------" CR);
  Log.notice("Tip 1 temperature: %F" CR, currentValue[0]);
  Log.notice("Ambient 1 temperature: %F" CR, currentValue[1]);
  Log.notice("Tip 2 temperature: %F" CR, currentValue[2]);
  Log.notice("Ambient 2 temperature: %F" CR, currentValue[3]);

  
}

void BleTemperatureTempSpike::disconnect(uint16_t conn_handle, uint8_t reason)
{
  this->bleConnHdl = INVALID_BLE_CONN_HANDLE;
  this->connected = false;
}
