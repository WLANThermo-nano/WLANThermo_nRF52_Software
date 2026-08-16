/***************************************************
    Copyright (C) 2026 Steffen Ochs

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
#include "BleTemperatureTyphurSync.h"
#include <ArduinoLog.h>

#define TYPHURSYNC_NUM_OF_TEMPERATURES 6u

/* ---------------------------------------------------------------------------
 * Payload layout, 22 byte, all words uint16 little endian
 *
 *  offset  content                                status
 *  ------  ------------------------------------   -------------------------
 *   0- 1   constant 0x0BE2                        header / device marker
 *   2- 3   core sensor 1, raw / 100 = degC        VERIFIED
 *   4- 5   core sensor 2                          VERIFIED
 *   6- 7   core sensor 3                          VERIFIED
 *   8- 9   core sensor 4                          VERIFIED
 *  10-11   core sensor 5                          VERIFIED
 *  12-13   ambient sensor, raw / 10 = degC        VERIFIED
 *  14-15   bit-identical duplicate of 12-13       duplicate, unused
 *  16-17   270..272 over the whole range          NOT a temperature
 *  18      constant 0x00                          unknown
 *  19      constant 0x1F (=31)                    battery candidate
 *  20-21   packet counter, +1 per notification,   VERIFIED
 *          resets on reconnect
 * ------------------------------------------------------------------------- */

#define TYPHURSYNC_CORE_DIVISOR 100.0f
#define TYPHURSYNC_AMBIENT_DIVISOR 10.0f

#define TYPHURSYNC_OFFSET_CORE 2u    /* first of five uint16, stride 2 */
#define TYPHURSYNC_OFFSET_AMBIENT 12u
#define TYPHURSYNC_OFFSET_BATTERY 16u
#define TYPHURSYNC_OFFSET_COUNTER 20u

#define TYPHURSYNC_NUM_OF_CORE_SENSORS 5u
#define TYPHURSYNC_AMBIENT_INDEX 5u

BleTemperatureTyphurSync::BleTemperatureTyphurSync(ble_gap_addr_t *peerAddress) : BleSensorBase(peerAddress, TYPHURSYNC_NUM_OF_TEMPERATURES, false)
{
  bleServ = new BLEClientService(BLEUuid(SERV_UUID_TYPHURSYNC));
  bleChar = new BLEClientCharacteristic(BLEUuid(CHAR_UUID_TYPHURSYNC));
  bleChar->setNotifyCallback(BleSensorGrp::notifyCb);

  bleServ->begin();
  bleChar->begin();

  Bluefruit.Central.connect(peerAddress);
}

void BleTemperatureTyphurSync::connect(uint16_t bleConnHdl)
{
  char buffer[30] = {0};

  this->bleConnHdl = bleConnHdl;

  BLEConnection *bleConnection = Bluefruit.Connection(bleConnHdl);

  if (bleConnection != NULL)
  {
    if (bleConnection->getPeerName(buffer, sizeof(buffer)))
    {
      // name = buffer;
      name = "TYPHUR";
    }
  }

  // disconnect if disabled
  if (false == enabled)
  {
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  /* Die MTU muss pro Verbindung ausgehandelt werden. configCentralConn()
   * hebt nur die Obergrenze im SoftDevice an, der Austausch selbst passiert
   * hier. Ohne ihn bleibt es bei 23 Byte, also 20 Byte Nutzlast. */
  if (bleConnection != NULL)
  {
    bleConnection->requestMtuExchange(25);

    /* Die Antwort kommt asynchron, deshalb kurz warten. */
    for (uint8_t i = 0u; (i < 50u) && (bleConnection->getMtu() < 25u); i++)
    {
      delay(10);
    }

    Log.notice("TyphurSync MTU: %d" CR, bleConnection->getMtu());
  }

  Log.notice("Discovering TyphurSync service ... ");

  // Check for service
  if (!bleServ->discover(bleConnHdl))
  {
    Log.notice("failed!" CR);
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);

  // Check characteristic
  Log.notice("Discovering TyphurSync characteristic ... ");
  if (!bleChar->discover())
  {
    Log.notice("failed!" CR);
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);

  Log.notice("TyphurSync value handle: 0x%x" CR, bleChar->valueHandle());

  // Enable notification
  Log.notice("Enabling TyphurSync notification ... ");
  if (!bleChar->enableNotify())
  {
    Log.notice("failed!" CR);
    Bluefruit.disconnect(bleConnHdl);
    return;
  }

  Log.notice("success" CR);

  this->connected = true;
}

uint16_t BleTemperatureTyphurSync::readU16(const uint8_t *data, uint8_t offset)
{
  return (uint16_t)((data[offset + 1u] << 8u) + data[offset]);
}

void BleTemperatureTyphurSync::notify(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len)
{
  /* // Guard: the probe may send other frame types on this characteristic. 
  if (len != TYPHURSYNC_PAYLOAD_LEN)
  {
    Log.warning("TyphurSync: unexpected length %d, ignored" CR, len);
    logRAW(data, len);
    return;
  }
  */
  logRAW(data, len);
  this->lastSeen = 0u;

  Log.notice("----------- TyphurSync data -----------" CR);

  /* Five core sensors along the probe, offsets 2, 4, 6, 8, 10.
   * Sensor 0 responds fastest to a step change and is most likely the tip. */
  for (uint8_t probeId = 0u; probeId < TYPHURSYNC_NUM_OF_CORE_SENSORS; probeId++)
  {
    uint16_t raw = readU16(data, (uint8_t)(TYPHURSYNC_OFFSET_CORE + (probeId * 2u)));
    currentValue[probeId] = ((float)raw) / TYPHURSYNC_CORE_DIVISOR;
    Log.notice("Probe %d has value %F" CR, probeId, currentValue[probeId]);
  }

  /* Ambient sensor at the handle end, 0.1 degC resolution. */
  uint16_t ambientRaw = readU16(data, TYPHURSYNC_OFFSET_AMBIENT);
  currentValue[TYPHURSYNC_AMBIENT_INDEX] = ((float)ambientRaw) / TYPHURSYNC_AMBIENT_DIVISOR;
  Log.notice("Ambient has value %F" CR, currentValue[TYPHURSYNC_AMBIENT_INDEX]);
}

void BleTemperatureTyphurSync::disconnect(uint16_t conn_handle, uint8_t reason)
{
  this->bleConnHdl = INVALID_BLE_CONN_HANDLE;
  this->connected = false;
}