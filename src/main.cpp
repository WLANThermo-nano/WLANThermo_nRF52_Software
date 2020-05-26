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


#define SERIAL_CMD_GET_DEVICES    "getDevices"
#define SERIAL_CMD_GET_VERSION    "getVersion"
#define SERIAL_CMD_SET_LOG_LEVEL  "setLogLevel"

void setup()
{
  Serial.begin(115200);
  Log.begin(LOG_LEVEL_FATAL, &Serial, false);

  Serial.println("@@Application");

  gBleTemperatureGrp.init();
}

void loop()
{
  static uint32_t prevMillis = millis();
  
  if(Serial.available())
  {
    String command = Serial.readStringUntil('\n');

    command.replace("\n", "");
    Log.notice("Received command: %s" CR,  command.c_str());

    if(command.startsWith(SERIAL_CMD_GET_DEVICES))
    {
      String json = gBleTemperatureGrp.getDevicesJson();
      Serial.println(json);
    }
    else if(command.startsWith(SERIAL_CMD_GET_VERSION))
    {
      Serial.println(FIRMWAREVERSION);
    }
    else if(command.startsWith(SERIAL_CMD_SET_LOG_LEVEL))
    {
      int indexOfEqual = command.indexOf("=");

      if((indexOfEqual > 0) && (command.length() - (indexOfEqual + 1)) > 0)
      {
        String logLevelString = command.substring(indexOfEqual + 1);
        Log.begin(logLevelString.toInt(), &Serial, false);
      }
    }
  }

  if(millis() - prevMillis > 1000u)
  {
    gBleTemperatureGrp.update();
    prevMillis = millis();
  }
}


