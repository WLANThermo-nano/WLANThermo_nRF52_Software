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

#include "BleTemperatureBase.h"

#define LOWEST_VALUE -31
#define HIGHEST_VALUE 999

uint8_t BleTemperatureBase::globalIndexTracker = 0u;

BleTemperatureBase::BleTemperatureBase(ble_gap_addr_t *peerAddress, uint8_t numOfTemperatures = NUM_OF_TEMPERATURES_DEFAULT)
{
  this->globalIndex = this->globalIndexTracker++;
  this->bleConnHdl = INVALID_BLE_CONN_HANDLE;
  this->connected = false;
  memcpy(&this->peerAddress, peerAddress, sizeof(ble_gap_addr_t));
  this->valueCount = numOfTemperatures;
  this->currentValue = new float[numOfTemperatures]();
  this->lastSeen = 0u;
  this->rssi = 0;
  this->enabled = false;
  this->prevEnabled = false;

  for (uint8_t index = 0u; index < this->valueCount; index++)
    this->currentValue[index] = INACTIVEVALUE;
}

BleTemperatureBase::~BleTemperatureBase()
{
  this->globalIndexTracker--;
}

float BleTemperatureBase::getValue(uint8_t index = 0u)
{
  float value = INACTIVEVALUE;

  if ((index < this->valueCount) && this->connected)
    value = (float)((int)(this->currentValue[index] * 10)) / 10; //limit float

  return value;
}

uint8_t BleTemperatureBase::getGlobalIndex()
{
  return this->globalIndex;
}

String BleTemperatureBase::getPeerAddressString()
{
  char peerAddressArray[18] = {0};
  uint8_t *b = this->peerAddress.addr;

  sprintf(peerAddressArray, "%02x:%02x:%02x:%02x:%02x:%02x", b[5], b[4], b[3], b[2], b[1], b[0]);

  return peerAddressArray;
}

void BleTemperatureBase::update()
{
  BLEConnection *bleConnection = Bluefruit.Connection(this->bleConnHdl);

  /* Update signal strength */
  if (bleConnection != NULL)
  {
    rssi = bleConnection->getRssi();
  }

  /* Connect or disconnect when requested */
  if ((false == prevEnabled) && (true == enabled))
  {
    prevEnabled = true;

    if (NULL == bleConnection)
    {
      Bluefruit.Central.connect(&this->peerAddress);
    }
  }
  else if ((true == prevEnabled) && (false == enabled))
  {
    prevEnabled = false;

    if (bleConnection != NULL)
    {
      bleConnection->disconnect();
    }
  }

  lastSeen++;
}