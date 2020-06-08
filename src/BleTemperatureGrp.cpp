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
#include "BleTemperatureWlanthermo.h"
#include "BleTemperatureInkbird.h"
#include <ArduinoJson.h>
#include <ArduinoLog.h>

#define BLE_JSON_DEVICE "d"
#define BLE_JSON_NAME "n"
#define BLE_JSON_ADDRESS "a"
#define BLE_JSON_STATUS "s"
#define BLE_JSON_COUNT "c"
#define BLE_JSON_TEMPERATURES "t"
#define BLE_JSON_LAST_SEEN "ls"
#define BLE_JSON_RSSI "r"

#define BLE_CENTRAL_DEVICE_COUNT 4u

const BLEUuid filterBleUuids[] = {
    BLEUuid(SERV_UUID_MEATER),
    BLEUuid(SERV_UUID_INKBIRD),
    BLEUuid(SERV_UUID_WLANTHERMO)};

BleTemperatureBase *BleTemperatureGrp::temperatures[MAX_TEMPERATURES];

BleTemperatureGrp::BleTemperatureGrp()
{
  this->addIndex = 0u;

  for (uint8_t i = 0u; i < MAX_TEMPERATURES; i++)
    temperatures[i] = NULL;
}

void BleTemperatureGrp::init()
{
  if (Bluefruit.begin(0, BLE_CENTRAL_DEVICE_COUNT) == false)
    Log.fatal("Bluefruit init failed!" CR);

  Bluefruit.setName("Wlanthermo");

  // Callbacks for Central
  Bluefruit.Central.setDisconnectCallback(BleTemperatureGrp::disconnectCb);
  Bluefruit.Central.setConnectCallback(BleTemperatureGrp::connectCb);

  Bluefruit.Scanner.setRxCallback(BleTemperatureGrp::scanCb);
  Bluefruit.Scanner.setInterval(160, 80); // in unit of 0.625 ms
  Bluefruit.Scanner.filterUuid((BLEUuid *)filterBleUuids, sizeof(filterBleUuids) / sizeof(BLEUuid));
  Bluefruit.Scanner.useActiveScan(true);
  Bluefruit.Scanner.start(0);
}

void BleTemperatureGrp::add(BleTemperatureBase *temperature)
{
  temperatures[addIndex++] = temperature;
}

void BleTemperatureGrp::update()
{
  if (Bluefruit.Scanner.isRunning() == false)
    Bluefruit.Scanner.start(0);

  for (uint8_t i = 0u; i < addIndex; i++)
  {
    if (temperatures[i] != NULL)
    {
      temperatures[i]->update();
    }
  }
}

BleTemperatureBase *BleTemperatureGrp::getTemperature(uint16_t conn_handle)
{
  BleTemperatureBase *temp = NULL;

  for (uint8_t i = 0u; i < MAX_TEMPERATURES; i++)
  {
    if (temperatures[i] != NULL)
    {
      if (temperatures[i]->getBleConnHdl() == conn_handle)
      {
        temp = temperatures[i];
        break;
      }
    }
  }

  return temp;
}

BleTemperatureBase *BleTemperatureGrp::getTemperature(ble_gap_addr_t *peerAddress)
{
  BleTemperatureBase *temp = NULL;

  for (uint8_t i = 0u; i < MAX_TEMPERATURES; i++)
  {
    if (temperatures[i] != NULL)
    {
      if (0u == memcmp(peerAddress, temperatures[i]->getPeerAddress(), sizeof(ble_gap_addr_t)))
      {
        temp = temperatures[i];
        break;
      }
    }
  }

  return temp;
}

String BleTemperatureGrp::getDevicesJson()
{
  DynamicJsonDocument doc(2048);
  String retVal;

  JsonArray devices = doc.createNestedArray(BLE_JSON_DEVICE);
  for (uint8_t i = 0u; i < this->count(); i++)
  {
    JsonObject object = devices.createNestedObject();
    object[BLE_JSON_NAME] = this->temperatures[i]->getName();
    object[BLE_JSON_ADDRESS] = this->temperatures[i]->getPeerAddressString();
    object[BLE_JSON_STATUS] = (uint8_t)this->temperatures[i]->isConnected();
    object[BLE_JSON_COUNT] = this->temperatures[i]->getValueCount();
    object[BLE_JSON_LAST_SEEN] = this->temperatures[i]->getLastSeen();
    object[BLE_JSON_RSSI] = this->temperatures[i]->getRssi();
    JsonArray temperaturesArray = object.createNestedArray(BLE_JSON_TEMPERATURES);
    for (uint8_t temps = 0u; temps < this->temperatures[i]->getValueCount(); temps++)
    {
      temperaturesArray.add(this->temperatures[i]->getValue(temps));
    }
  }

  serializeJson(doc, retVal);

  return retVal;
}

void BleTemperatureGrp::enable(uint32_t enable)
{
  for (uint8_t i = 0u; i < this->count(); i++)
  {
    this->temperatures[i]->enable((enable & (1u << i)) > 0u);
  }
}

uint8_t BleTemperatureGrp::count()
{
  return this->addIndex;
}

BleTemperatureBase *BleTemperatureGrp::operator[](int index)
{
  return (((uint16_t)index) < MAX_TEMPERATURES) ? temperatures[index] : NULL;
}

void BleTemperatureGrp::scanCb(ble_gap_evt_adv_report_t *report)
{
  BleTemperatureBase *temp = gBleTemperatureGrp.getTemperature(&report->peer_addr);

  if (NULL == temp)
  {
    if (Bluefruit.Scanner.checkReportForUuid(report, SERV_UUID_MEATER))
    {
      Log.notice("Meater %s received" CR, (true == report->type.scan_response) ? "scan response" : "advertising");
      BleTemperatureMeater *temp = new BleTemperatureMeater(&report->peer_addr);
      gBleTemperatureGrp.add(temp);
    }
    else if (Bluefruit.Scanner.checkReportForUuid(report, SERV_UUID_WLANTHERMO))
    {
      Log.notice("Wlanthermo %s received" CR, (true == report->type.scan_response) ? "scan response" : "advertising");
      BleTemperatureWlanthermo *temp = new BleTemperatureWlanthermo(&report->peer_addr);
      gBleTemperatureGrp.add(temp);
    }
    else if (Bluefruit.Scanner.checkReportForUuid(report, SERV_UUID_INKBIRD))
    {
      Log.notice("Inkbird %s received" CR, (true == report->type.scan_response) ? "scan response" : "advertising");
      BleTemperatureInkbird *temp = new BleTemperatureInkbird(&report->peer_addr);
      gBleTemperatureGrp.add(temp);
    }
  }
  else
  {
    temp->advReceived();
  }

  Bluefruit.Scanner.resume();
}

void BleTemperatureGrp::connectCb(uint16_t conn_handle)
{
  Log.notice("Connected" CR);

  BLEConnection *bleConnection = Bluefruit.Connection(conn_handle);

  if (bleConnection != NULL)
  {
    ble_gap_addr_t peerAddress = bleConnection->getPeerAddr();
    BleTemperatureBase *temp = gBleTemperatureGrp.getTemperature(&peerAddress);

    if (temp != NULL)
    {
      bleConnection->monitorRssi();
      temp->connect(conn_handle);
    }
  }
}

void BleTemperatureGrp::disconnectCb(uint16_t conn_handle, uint8_t reason)
{
  Log.notice("Disconnected" CR);

  BleTemperatureBase *temp = gBleTemperatureGrp.getTemperature(conn_handle);

  if (temp != NULL)
  {
    temp->disconnect(conn_handle, reason);
  }
}

void BleTemperatureGrp::notifyCb(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len)
{
  BleTemperatureBase *temp = gBleTemperatureGrp.getTemperature(chr->connHandle());

  if (temp != NULL)
  {
    temp->notify(chr, data, len);
  }
}

void BleTemperatureGrp::indicateCb(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len)
{
  BleTemperatureBase *temp = gBleTemperatureGrp.getTemperature(chr->connHandle());

  if (temp != NULL)
  {
    temp->indicate(chr, data, len);
  }
}

BleTemperatureGrp gBleTemperatureGrp;
