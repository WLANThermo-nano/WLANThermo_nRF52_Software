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

#include "BleSensorBase.h"

#define LOWEST_VALUE -31
#define HIGHEST_VALUE 999

uint8_t BleSensorBase::globalIndexTracker = 0u;

BleSensorBase::BleSensorBase(ble_gap_addr_t *peerAddress, uint8_t numOfTemperatures = NUM_OF_TEMPERATURES_DEFAULT, boolean beaconOnly = false)
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
  this->beaconOnly = beaconOnly;

  for (uint8_t index = 0u; index < this->valueCount; index++)
    this->currentValue[index] = INACTIVEVALUE;
}

BleSensorBase::~BleSensorBase()
{
  this->globalIndexTracker--;
}

float BleSensorBase::getValue(uint8_t index = 0u)
{
  float value = INACTIVEVALUE;

  if ((index < this->valueCount) && this->connected)
    value = (float)((int)(this->currentValue[index] * 10)) / 10; //limit float

  return value;
}

uint8_t BleSensorBase::getGlobalIndex()
{
  return this->globalIndex;
}

String BleSensorBase::getPeerAddressString()
{
  char peerAddressArray[18] = {0};
  uint8_t *b = this->peerAddress.addr;

  sprintf(peerAddressArray, "%02x:%02x:%02x:%02x:%02x:%02x", b[5], b[4], b[3], b[2], b[1], b[0]);

  return peerAddressArray;
}

void BleSensorBase::update()
{
  if(beaconOnly != true)
  {
    BLEConnection *bleConnection = Bluefruit.Connection(this->bleConnHdl);

    /* Update signal strength */
    if (bleConnection != NULL)
    {
      rssi = bleConnection->getRssi();
    }

    /* Disconnect when requested and num of temperatures is known */
    if ((bleConnection != NULL) && (false == enabled) && (valueCount > 0u))
    {
      bleConnection->disconnect();
    }
  }

  lastSeen++;
}

void BleSensorBase::advReceived(uint8_t *advData, uint8_t advDataLength)
{
  lastSeen = 0u;
  
  if(true == beaconOnly)
  {
    this->readBeacon(advData, advDataLength);
  }
  else if((false == connected) && (true == enabled))
  {
    Bluefruit.Central.connect(&peerAddress);
  }
};
