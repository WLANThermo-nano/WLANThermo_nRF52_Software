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
#include "BleTemperatureMeater.h"
#include "BleTemperatureWlanthermo.h"
#include "BleScaleWlanthermo.h"
#include "BleTemperatureInkbird.h"
#include "BleTemperatureMeatStick.h"
#include <ArduinoJson.h>
#include <ArduinoLog.h>

#define BLE_JSON_DEVICE "d"
#define BLE_JSON_NAME "n"
#define BLE_JSON_ADDRESS "a"
#define BLE_JSON_STATUS "s"
#define BLE_JSON_COUNT "c"
#define BLE_JSON_LAST_SEEN "ls"
#define BLE_JSON_RSSI "r"
#define BLE_JSON_SENSORS "t"
#define BLE_JSON_SENSORS_VALUE "v"
#define BLE_JSON_SENSORS_UNIT "u"

#define BLE_CENTRAL_DEVICE_COUNT 4u

const BLEUuid filterBleUuids[] = {
    BLEUuid(SERV_UUID_MEATER),
    BLEUuid(SERV_UUID_INKBIRD),
    BLEUuid(SERV_UUID_TEMPERATURE_WLANTHERMO)};

BleSensorBase *BleSensorGrp::sensors[MAX_TEMPERATURES];

BleSensorGrp::BleSensorGrp()
{
  this->addIndex = 0u;

  for (uint8_t i = 0u; i < MAX_TEMPERATURES; i++)
    sensors[i] = NULL;
}

void BleSensorGrp::init()
{
  if (Bluefruit.begin(0, BLE_CENTRAL_DEVICE_COUNT) == false)
    Log.fatal("Bluefruit init failed!" CR);

  Bluefruit.setName("Wlanthermo");
  Bluefruit.setTxPower(BLE_TX_POWER);

  // Callbacks for Central
  Bluefruit.Central.setDisconnectCallback(BleSensorGrp::disconnectCb);
  Bluefruit.Central.setConnectCallback(BleSensorGrp::connectCb);

  Bluefruit.Scanner.setRxCallback(BleSensorGrp::scanCb);
  Bluefruit.Scanner.setInterval(160, 80); // in unit of 0.625 ms
  //Bluefruit.Scanner.filterUuid((BLEUuid *)filterBleUuids, sizeof(filterBleUuids) / sizeof(BLEUuid));
  Bluefruit.Scanner.useActiveScan(true);
  Bluefruit.Scanner.start(0);
}

void BleSensorGrp::add(BleSensorBase *sensor)
{
  sensors[addIndex++] = sensor;
}

void BleSensorGrp::update()
{
  if (Bluefruit.Scanner.isRunning() == false)
    Bluefruit.Scanner.start(0);

  for (uint8_t i = 0u; i < addIndex; i++)
  {
    if (sensors[i] != NULL)
    {
      sensors[i]->update();
    }
  }
}

BleSensorBase *BleSensorGrp::getSensor(uint16_t conn_handle)
{
  BleSensorBase *temp = NULL;

  for (uint8_t i = 0u; i < MAX_TEMPERATURES; i++)
  {
    if (sensors[i] != NULL)
    {
      if (sensors[i]->getBleConnHdl() == conn_handle)
      {
        temp = sensors[i];
        break;
      }
    }
  }

  return temp;
}

BleSensorBase *BleSensorGrp::getSensor(ble_gap_addr_t *peerAddress)
{
  BleSensorBase *temp = NULL;

  for (uint8_t i = 0u; i < MAX_TEMPERATURES; i++)
  {
    if (sensors[i] != NULL)
    {
      if (0u == memcmp(peerAddress, sensors[i]->getPeerAddress(), sizeof(ble_gap_addr_t)))
      {
        temp = sensors[i];
        break;
      }
    }
  }

  return temp;
}

String BleSensorGrp::getDevicesJson()
{
  DynamicJsonDocument doc(2048);
  String retVal;

  JsonArray devices = doc.createNestedArray(BLE_JSON_DEVICE);
  for (uint8_t deviceIndex = 0u; deviceIndex < this->count(); deviceIndex++)
  {
    JsonObject object = devices.createNestedObject();
    object[BLE_JSON_NAME] = this->sensors[deviceIndex]->getName();
    object[BLE_JSON_ADDRESS] = this->sensors[deviceIndex]->getPeerAddressString();
    object[BLE_JSON_STATUS] = (uint8_t)this->sensors[deviceIndex]->isConnected();
    object[BLE_JSON_COUNT] = this->sensors[deviceIndex]->getValueCount();
    object[BLE_JSON_LAST_SEEN] = this->sensors[deviceIndex]->getLastSeen();
    object[BLE_JSON_RSSI] = this->sensors[deviceIndex]->getRssi();
    JsonArray sensorsArray = object.createNestedArray(BLE_JSON_SENSORS);
    for (uint8_t sensorIndex = 0u; sensorIndex < this->sensors[deviceIndex]->getValueCount(); sensorIndex++)
    {
      JsonObject sensorObject = sensorsArray.createNestedObject();
      sensorObject[BLE_JSON_SENSORS_VALUE] = this->sensors[deviceIndex]->getValue(sensorIndex);
      String unit =  this->sensors[deviceIndex]->getUnit();
      if(unit.length() > 0u)
      {
        sensorObject[BLE_JSON_SENSORS_UNIT] = unit;
      }
    }
  }

  serializeJson(doc, retVal);

  return retVal;
}

void BleSensorGrp::enable(uint32_t enable)
{
  for (uint8_t i = 0u; i < this->count(); i++)
  {
    this->sensors[i]->enable((enable & (1u << i)) > 0u);
  }
}

uint8_t BleSensorGrp::count()
{
  return this->addIndex;
}

BleSensorBase *BleSensorGrp::operator[](int index)
{
  return (((uint16_t)index) < MAX_TEMPERATURES) ? sensors[index] : NULL;
}

void BleSensorGrp::scanCb(ble_gap_evt_adv_report_t *report)
{
  BeaconType beacon;
  BleSensorBase *temp = gBleSensorGrp.getSensor(&report->peer_addr);

  if (NULL == temp)
  {
    if (Bluefruit.Scanner.checkReportForUuid(report, SERV_UUID_MEATER))
    {
      Log.notice("Meater %s received" CR, (true == report->type.scan_response) ? "scan response" : "advertising");
      BleTemperatureMeater *temp = new BleTemperatureMeater(&report->peer_addr);
      gBleSensorGrp.add(temp);
    }
    else if (Bluefruit.Scanner.checkReportForUuid(report, SERV_UUID_TEMPERATURE_WLANTHERMO))
    {
      Log.notice("Wlanthermo %s received" CR, (true == report->type.scan_response) ? "scan response" : "advertising");
      BleTemperatureWlanthermo *temp = new BleTemperatureWlanthermo(&report->peer_addr);
      gBleSensorGrp.add(temp);
    }
    else if (Bluefruit.Scanner.checkReportForUuid(report, SERV_UUID_SCALE_WLANTHERMO))
    {
      Log.notice("Wlanthermo %s received" CR, (true == report->type.scan_response) ? "scan response" : "advertising");
      BleScaleWlanthermo *temp = new BleScaleWlanthermo(&report->peer_addr);
      gBleSensorGrp.add(temp);
    }
    else if (Bluefruit.Scanner.checkReportForUuid(report, SERV_UUID_INKBIRD))
    {
      Log.notice("Inkbird %s received" CR, (true == report->type.scan_response) ? "scan response" : "advertising");
      BleTemperatureInkbird *temp = new BleTemperatureInkbird(&report->peer_addr);
      gBleSensorGrp.add(temp);
    }
    else if (Bluefruit.Scanner.parseReportByType(report, 0xFFu, (uint8_t *)&beacon, sizeof(BeaconType)) == sizeof(BeaconType))
    {
      if(BleTemperatureMeatStick::hasMeatStickData(&beacon))
      {
        Log.notice("MeatStick %s received" CR, (true == report->type.scan_response) ? "scan response" : "advertising");
        BleTemperatureMeatStick *temp = new BleTemperatureMeatStick(&report->peer_addr, &beacon);
        gBleSensorGrp.add(temp);
      }
    }
  }
  else
  {
    temp->advReceived(report->data.p_data, report->data.len);
  }

  Bluefruit.Scanner.resume();
}

void BleSensorGrp::connectCb(uint16_t conn_handle)
{
  Log.notice("Connected" CR);

  BLEConnection *bleConnection = Bluefruit.Connection(conn_handle);

  if (bleConnection != NULL)
  {
    ble_gap_addr_t peerAddress = bleConnection->getPeerAddr();
    BleSensorBase *temp = gBleSensorGrp.getSensor(&peerAddress);

    if (temp != NULL)
    {
      bleConnection->monitorRssi();
      temp->connect(conn_handle);
    }
  }
}

void BleSensorGrp::disconnectCb(uint16_t conn_handle, uint8_t reason)
{
  Log.notice("Disconnected" CR);

  BleSensorBase *temp = gBleSensorGrp.getSensor(conn_handle);

  if (temp != NULL)
  {
    temp->disconnect(conn_handle, reason);
  }
}

void BleSensorGrp::notifyCb(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len)
{
  BleSensorBase *temp = gBleSensorGrp.getSensor(chr->connHandle());

  if (temp != NULL)
  {
    temp->notify(chr, data, len);
  }
}

void BleSensorGrp::indicateCb(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len)
{
  BleSensorBase *temp = gBleSensorGrp.getSensor(chr->connHandle());

  if (temp != NULL)
  {
    temp->indicate(chr, data, len);
  }
}

BleSensorGrp gBleSensorGrp;
