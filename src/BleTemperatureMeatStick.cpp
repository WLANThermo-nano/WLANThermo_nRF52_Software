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
#include "BleTemperatureMeatStick.h"
#include <ArduinoLog.h>

#define MEATSTICK_NUM_OF_TEMERATURES 2u
#define MEATSTICK_BEACON_MANUFACTURER 0x004Cu
#define MEATSTICK_BEACON_TYPE 0x02u
#define MEATSTICK_BEACON_LENGTH 21u
#define MEATSTICK_CONNECTION_TIMEOUT 60u

BleTemperatureMeatStick::BleTemperatureMeatStick(ble_gap_addr_t *peerAddress, BeaconType *beacon) : BleSensorBase(peerAddress, MEATSTICK_NUM_OF_TEMERATURES, true)
{
  this->name = "The MeatStick";
}

void BleTemperatureMeatStick::update()
{
  BleSensorBase::update();

  this->connected = (this->lastSeen < MEATSTICK_CONNECTION_TIMEOUT) ? true : false;
}

void BleTemperatureMeatStick::readBeacon(uint8_t *advData, uint8_t advDataLength)
{
  BeaconType beacon;
  boolean updatedValue = false;

  if(Bluefruit.Scanner.parseReportByType(advData, advDataLength, 0xFF, (uint8_t*)&beacon, sizeof(BeaconType)) == sizeof(BeaconType))
  {
    if ((MEATSTICK_BEACON_MANUFACTURER == beacon.manufacturer) &&
          (MEATSTICK_BEACON_TYPE == beacon.beaconType) &&
          (MEATSTICK_BEACON_LENGTH == beacon.beaconLen))
      {
        /*Serial.println();
        Serial.printBuffer((uint8_t*)&beacon, sizeof(BeaconType), '-');
        Serial.println();*/

        if(0u == memcmp(beacon.uuid128, CHAR_UUID_MEATSTICK_T0, sizeof(CHAR_UUID_MEATSTICK_T0)))
        {
          currentValue[0] = (float(beacon.minor.highByte - 0x01u)) / 2.0;
          currentValue[1] = (float(beacon.minor.lowByte - 0x02u)) / 2.0;
          updatedValue = true;
        }
        else if(0u == memcmp(beacon.uuid128, CHAR_UUID_MEATSTICK_T1, sizeof(CHAR_UUID_MEATSTICK_T1)))
        {
          currentValue[0] = (float(beacon.minor.highByte - 0x01u)) / 2.0;
          currentValue[1] = (float(beacon.minor.lowByte - 0x03u + 0xFFu)) / 2.0;
          updatedValue = true;
        }
        else if(0u == memcmp(beacon.uuid128, CHAR_UUID_MEATSTICK_T2, sizeof(CHAR_UUID_MEATSTICK_T2)))
        {
          currentValue[0] = (float(beacon.minor.highByte - 0x01u)) / 2.0;
          currentValue[1] = (float(beacon.minor.lowByte - 0x04u + 0x1FEu)) / 2.0;
          updatedValue = true;
        }

        if(true == updatedValue)
        {
          Log.notice("----------- MeatStick data -----------" CR);
          Log.notice("Tip temperature: %F" CR, currentValue[0]);
          Log.notice("Ambient temperature: %F" CR, currentValue[1]);
        }
      }
  }
}

boolean BleTemperatureMeatStick::hasMeatStickData(BeaconType *beacon)
{
  boolean hasData = false;

  if ((MEATSTICK_BEACON_MANUFACTURER == beacon->manufacturer) &&
      (MEATSTICK_BEACON_TYPE == beacon->beaconType) &&
      (MEATSTICK_BEACON_LENGTH == beacon->beaconLen))
  {
    if(0u == memcmp(beacon->uuid128, CHAR_UUID_MEATSTICK_T0, sizeof(CHAR_UUID_MEATSTICK_T0)))
    {
      hasData = true;
    }
    else if(0u == memcmp(beacon->uuid128, CHAR_UUID_MEATSTICK_T1, sizeof(CHAR_UUID_MEATSTICK_T1)))
    {
      hasData = true;
    }
    else if(0u == memcmp(beacon->uuid128, CHAR_UUID_MEATSTICK_T2, sizeof(CHAR_UUID_MEATSTICK_T2)))
    {
      hasData = true;
    }
  }

  return hasData;
}
