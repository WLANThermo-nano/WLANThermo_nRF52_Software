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

#include <bluefruit.h>
#include <ArduinoLog.h>
#include "BleTemperatureGrp.h"
#include "Version.h"

#define SERIAL_CMD_GET_DEVICES "getDevices"
#define SERIAL_CMD_GET_VERSION "getVersion"
#define SERIAL_CMD_SET_LOG_LEVEL "setLogLevel"

void mainTask(void *arg);

TaskHandle_t mainTaskHandle = NULL;

void rxIsr(void)
{
  vTaskNotifyGiveFromISR(mainTaskHandle, NULL);
}

void setup()
{
  Serial.begin(115200);
  /*while (!Serial)
    delay(10); // for nrf52840 with native usb*/
  Log.begin(LOG_LEVEL_FATAL, &Serial, false);

  Serial.printf("@@Application: %d\n", BUILD_TIMESTAMP);

  xTaskCreate(mainTask, "mainTask", 4 * 256, NULL, 2, &mainTaskHandle);
}

void mainTask(void *arg)
{
  Serial.attachRxIsr(rxIsr);
  Serial.println("test");
  gBleTemperatureGrp.init();

  while (1)
  {
    static uint32_t prevMillis = millis();

    if (Serial.available())
    {
      String command = Serial.readStringUntil('\n');

      command.replace("\n", "");
      Log.notice("Received command: %s" CR, command.c_str());

      if (command.startsWith(SERIAL_CMD_GET_DEVICES))
      {
        int indexOfEqual = command.indexOf("=");

        if ((indexOfEqual > 0) && (command.length() - (indexOfEqual + 1)) > 0)
        {
          String enableString = command.substring(indexOfEqual + 1);
          gBleTemperatureGrp.enable(enableString.toInt());
          String json = gBleTemperatureGrp.getDevicesJson();
          Serial.println(json);
        }
      }
      else if (command.startsWith(SERIAL_CMD_GET_VERSION))
      {
        Serial.println(FIRMWAREVERSION);
      }
      else if (command.startsWith(SERIAL_CMD_SET_LOG_LEVEL))
      {
        int indexOfEqual = command.indexOf("=");

        if ((indexOfEqual > 0) && (command.length() - (indexOfEqual + 1)) > 0)
        {
          String logLevelString = command.substring(indexOfEqual + 1);
          Log.begin(logLevelString.toInt(), &Serial, false);
        }
      }
    }

    if (millis() - prevMillis > 1000u)
    {
      gBleTemperatureGrp.update();
      prevMillis = millis();
    }

    // This function will block until notify or timeout
    if (ulTaskNotifyTake(pdTRUE, ms2tick(2000)) != 0)
    {
      vTaskDelay(10);
    }
  }
}

void loop()
{
  vTaskDelay(1000);
}
