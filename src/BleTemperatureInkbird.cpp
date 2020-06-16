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
#include "BleTemperatureInkbird.h"
#include <ArduinoLog.h>

#define INKBIRD_NUM_OF_TEMERATURES NUM_OF_TEMPERATURES_DEFAULT
#define INBIRD_INACTIVEVALUE 0xFFF6u

static const uint8_t credentials[] = {0x21, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0xb8, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t enableRealtimeData[] = {0x0B, 0x01, 0x00, 0x00, 0x00, 0x00};
static const uint8_t unitCelsius[] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x00};

union SplitTwoBytes {
  uint16_t value;
  struct
  {
    uint8_t lowByte;
    uint8_t highByte;
  };
};

BleTemperatureInkbird::BleTemperatureInkbird(ble_gap_addr_t *peerAddress) : BleTemperatureBase(peerAddress, INKBIRD_NUM_OF_TEMERATURES)
{
  bleServ = new BLEClientService(BLEUuid(SERV_UUID_INKBIRD));
  bleCharLogin = new BLEClientCharacteristic(BLEUuid(CHAR_UUID_INKBIRD_LOGIN));
  bleCharRealtimeData = new BLEClientCharacteristic(BLEUuid(CHAR_UUID_INKBIRD_REALTIMEDATA));
  bleCharRealtimeData->setNotifyCallback(BleTemperatureGrp::notifyCb);
  bleCharSettings = new BLEClientCharacteristic(BLEUuid(CHAR_UUID_INKBIRD_SETTINGS));

  bleServ->begin();
  bleCharLogin->begin();
  bleCharRealtimeData->begin();
  bleCharSettings->begin();

  valueCount = 0u;

  Bluefruit.Central.connect(peerAddress);
}

void BleTemperatureInkbird::update()
{
  BleTemperatureBase::update();
}

void BleTemperatureInkbird::connect(uint16_t bleConnHdl)
{
  char buffer[30] = {0};

  this->bleConnHdl = bleConnHdl;

  BLEConnection *bleConnection = Bluefruit.Connection(bleConnHdl);

  // get device name
  if (bleConnection != NULL)
  {
    if (bleConnection->getPeerName(buffer, sizeof(buffer)))
    {
      name = buffer;
    }
  }

  // Check for service
  Log.notice("Discovering Inkbird service ... ");
  if (!bleServ->discover(bleConnHdl))
  {
    Log.notice("failed!" CR);
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);

  // Check characteristic
  Log.notice("Discovering Inkbird LOGIN characteristic ... ");
  if (!bleCharLogin->discover())
  {
    Log.notice("failed!" CR);
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);

  // iBBQ login
  Log.notice("Writing iBBQ credentials ... ");
  if (!bleCharLogin->write(credentials, sizeof(credentials)))
  {
    Log.notice("failed!" CR);
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);

  // Enable realtime data
  Log.notice("Set inkbird unit to celsius: ");
  if (!writeSettings(unitCelsius, sizeof(unitCelsius)))
  {
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  // Check characteristic
  Log.notice("Discovering Inkbird REALTIMEDATA characteristic ... ");
  if (!bleCharRealtimeData->discover())
  {
    Log.notice("failed!" CR);
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);

  // Enable notification
  Log.notice("Enabling Inkbird notification ... ");
  if (!bleCharRealtimeData->enableNotify())
  {
    Log.notice("failed!" CR);
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);

  // Enable realtime data
  Log.notice("Enable Inkbird realtime data: ");
  if (!writeSettings(enableRealtimeData, sizeof(enableRealtimeData)))
  {
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);

  this->connected = true;
}

uint16_t littleEndianInt(uint8_t *pData)
{
  uint16_t val = (pData[1] << 8) | pData[0];
  return val;
}

void BleTemperatureInkbird::notify(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len)
{
  this->lastSeen = 0u;

  Log.notice("----------- Inkbird data -----------" CR);

  u_int8_t numOfTemperatures = len / 2u;

  // get number of temperatures for initial connect
  if ((numOfTemperatures <= NUM_OF_TEMPERATURES_DEFAULT) && (0u == valueCount))
  {
    valueCount = numOfTemperatures;
    Log.notice("Number of temperatures: %d" CR, valueCount);

    // disconnect if disabled
    if (false == enabled)
    {
      Bluefruit.disconnect(bleConnHdl);
    }
  }

  uint8_t probeId = 0;
  for (uint8_t i = 0u; (i < len) && (probeId < valueCount); i += 2u)
  {
    uint16_t rawVal = (data[i + 1] << 8) | data[i];

    currentValue[probeId] = (INBIRD_INACTIVEVALUE == rawVal) ? INACTIVEVALUE : (rawVal / 10);

    Log.notice("Probe %d has value %F" CR, probeId, currentValue[probeId]);
    probeId++;
  }

  Log.verbose("Raw data: ");

  for (uint8_t i = 0u; i < len; i++)
    Log.verbose("%X ", data[i]);

  Log.verbose(CR);
}

boolean BleTemperatureInkbird::writeSettings(const uint8_t *data, uint32_t length)
{
  // Check characteristic
  Log.notice("Discovering Inkbird SETTINGS characteristic ... ");
  if (!bleCharSettings->discover())
  {
    Log.notice("failed!" CR);
    return false;
  }

  Log.notice("success" CR);

  // Write settings
  Log.notice("Write Inkbird SETTINGS characteristic ... ");
  if (!bleCharSettings->write(data, length))
  {
    Log.notice("failed!" CR);
    return false;
  }

  Log.notice("success" CR);

  return true;
}

void BleTemperatureInkbird::disconnect(uint16_t conn_handle, uint8_t reason)
{
  this->bleConnHdl = INVALID_BLE_CONN_HANDLE;
  this->connected = false;
}
