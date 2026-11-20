/***************************************************
    Copyright (C) 2026  fsedarkalex

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
#include "BleTemperatureOoni.h"
#include <ArduinoLog.h>

// Channel order: 0 = stone 1, 1 = stone 2, 2 = P1 (meat, left), 3 = P2 (meat, right)
#define OONI_NUM_OF_TEMPERATURES 4u

// Frame layout (22 bytes @ 1 Hz, little endian):
//   0      bitmask of plugged probes (bit0 stone 1, bit1 stone 2, bit2 P1, bit3 P2, bit4 always set)
//   1      0x00
//   2..9   4 x int16 temperature in whole degrees celsius (0 if probe not plugged)
//   10     battery in percent
//   11     0x01 (unknown)
//   12..17 own MAC address, reversed byte order
//   18..21 unknown
#define OONI_FRAME_MIN_LEN 10u   // flags + reserved + 4 x int16
#define OONI_OFFSET_FLAGS 0u     // bit n set = probe n plugged in
#define OONI_OFFSET_TEMP 2u      // 4 x int16, whole degrees celsius
#define OONI_OFFSET_BATTERY 10u  // battery in percent

// The hub rejects the CCCD write (ATT error 0x0E "Unlikely Error") and drops the link
// when notifications are enabled right after connecting. Measured: 1/5 connects succeed
// without delay, 5/5 with 1.5 s delay. Wait this long after the connection is up.
#define OONI_NOTIFY_DELAY_MS 2000u

BleTemperatureOoni::BleTemperatureOoni(ble_gap_addr_t *peerAddress) : BleSensorBase(peerAddress, OONI_NUM_OF_TEMPERATURES, false)
{
  name = OONI_HUB_NAME;

  bleServ = new BLEClientService(BLEUuid(SERV_UUID_OONI));
  bleChar = new BLEClientCharacteristic(BLEUuid(CHAR_UUID_OONI));
  bleChar->setNotifyCallback(BleSensorGrp::notifyCb);

  bleServ->begin();
  bleChar->begin();

  Bluefruit.Central.connect(peerAddress);
}

boolean BleTemperatureOoni::isOoniHub(ble_gap_evt_adv_report_t *report)
{
  char buffer[32] = {0};
  uint8_t len = Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME, (uint8_t *)buffer, sizeof(buffer) - 1u);

  if (0u == len)
  {
    len = Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME, (uint8_t *)buffer, sizeof(buffer) - 1u);
  }

  return (len > 0u) && (0 == strncmp(buffer, OONI_HUB_NAME, sizeof(OONI_HUB_NAME) - 1u));
}

void BleTemperatureOoni::connect(uint16_t bleConnHdl)
{
  char buffer[30] = {0};
  uint32_t connectStart = millis();

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

  Log.notice("Discovering Ooni service ... ");

  if (!bleServ->discover(bleConnHdl))
  {
    Log.notice("failed!" CR);
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);

  Log.notice("Discovering Ooni characteristic ... ");
  if (!bleChar->discover())
  {
    Log.notice("failed!" CR);
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);

  // Give the hub time before the CCCD write, see OONI_NOTIFY_DELAY_MS
  while ((millis() - connectStart) < OONI_NOTIFY_DELAY_MS)
  {
    if (!Bluefruit.connected(bleConnHdl))
    {
      Log.notice("Ooni disconnected while waiting" CR);
      return;
    }
    delay(50);
  }

  // Should the hub still reject the CCCD write, it drops the link.
  // Disconnecting here lets advReceived() reconnect on the next advertisement.
  Log.notice("Enabling Ooni notification ... ");
  if (!bleChar->enableNotify())
  {
    Log.notice("failed!" CR);
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);

  this->connected = true;
}

void BleTemperatureOoni::notify(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len)
{
  logRAW(data, len);

  if (len < OONI_FRAME_MIN_LEN)
  {
    Log.warning("Ooni frame too short: %d" CR, len);
    return;
  }

  uint8_t flags = data[OONI_OFFSET_FLAGS];

  for (uint8_t i = 0u; i < OONI_NUM_OF_TEMPERATURES; i++)
  {
    if (flags & (1u << i))
    {
      uint8_t offset = OONI_OFFSET_TEMP + (i * 2u);
      int16_t raw = (int16_t)((uint16_t)data[offset] | ((uint16_t)data[offset + 1u] << 8u));
      currentValue[i] = (float)raw;
    }
    else
    {
      currentValue[i] = INACTIVEVALUE;
    }
  }

  this->lastSeen = 0u;

  Log.notice("----------- Ooni data -----------" CR);
  Log.notice("Stone 1: %F, Stone 2: %F, P1: %F, P2: %F" CR, currentValue[0], currentValue[1], currentValue[2], currentValue[3]);

  if (len > OONI_OFFSET_BATTERY)
  {
    Log.notice("Battery: %d%%" CR, data[OONI_OFFSET_BATTERY]);
  }
}

void BleTemperatureOoni::disconnect(uint16_t conn_handle, uint8_t reason)
{
  this->bleConnHdl = INVALID_BLE_CONN_HANDLE;
  this->connected = false;
}
