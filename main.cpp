/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mbed.h"
#include "ble/BLE.h"

#include "ble/services/UARTService.h"

#define NEED_CONSOLE_OUTPUT 1 /* Set this if you need debug messages on the console;
                               * it will have an impact on code-size and power consumption. */

#if NEED_CONSOLE_OUTPUT
Serial pc(p10,p11);
#define DEBUG(...) { pc.printf(__VA_ARGS__); }
#else
#define DEBUG(...) /* nothing */
#endif /* #if NEED_CONSOLE_OUTPUT */



BLEDevice  ble;
DigitalOut L0(p9);
DigitalOut L1(p16);
DigitalOut R0(p17);
DigitalOut R1(p18);

UARTService *uartServicePtr;

enum Command_Control{
  STOP = '0',
  NEXT,
  BACK,
  LEFT,
  RIGH,
};
volatile uint8_t cmd_cur = STOP;
volatile uint8_t cmd_for = STOP;




void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params)
{
    DEBUG("Disconnected!\n\r");
    DEBUG("Restarting the advertising process\n\r");
    ble.startAdvertising();
}

void onDataWritten(const GattWriteCallbackParams *params)
{
    if ((uartServicePtr != NULL) && (params->handle == uartServicePtr->getTXCharacteristicHandle())) {
        uint16_t bytesRead = params->len;
        DEBUG("received %u bytes\n\r", bytesRead);
        ble.updateCharacteristicValue(uartServicePtr->getRXCharacteristicHandle(), params->data, bytesRead);
        cmd_cur = params->data[0];
    }
}
void ctrMotor(uint8_t command);
void action(void);

int main(void)
{
    #if NEED_CONSOLE_OUTPUT
    pc.baud(115200);
    DEBUG("Initialising the nRF51822\n\r");
    #endif //NEED_CONSOLE_OUTPUT

    ble.init();
    ble.onDisconnection(disconnectionCallback);
    ble.onDataWritten(onDataWritten);

    /* setup advertising */
    ble.accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED);
    ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::SHORTENED_LOCAL_NAME,
                                     (const uint8_t *)"BLE CAR", sizeof("BLE CAR") - 1);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_128BIT_SERVICE_IDS,
                                     (const uint8_t *)UARTServiceUUID_reversed, sizeof(UARTServiceUUID_reversed));

    ble.setAdvertisingInterval(1000); /* 1000ms; in multiples of 0.625ms. */
    ble.startAdvertising();

    UARTService uartService(ble);
    uartServicePtr = &uartService;

    while (true) {
     //   ble.waitForEvent();
        action();
    }
}
void ctrMotor(uint8_t command){
  switch(command){
    case STOP:
    L0 = 0;
    L1 = 0;
    R0 = 0;
    R1 = 0;
    break; 

    case NEXT:
    L0 = 1;
    L1 = 0;
    R0 = 1;
    R1 = 0;
    break; 
    case BACK:
    L0 = 0;
    L1 = 1;
    R0 = 0;
    R1 = 1;
    break;     
    case LEFT:
    L0 = 1;
    L1 = 0;
    R0 = 0;
    R1 = 0;
    break;     
    case RIGH:
    L0 = 0;
    L1 = 0;
    R0 = 1;
    R1 = 0;
    break; 
    default:
    break;    
  }
}
void action(void){
  if(cmd_cur != cmd_for){
    ctrMotor(cmd_cur);
    cmd_for = cmd_cur;
    DEBUG("changed state to : %s \n\r",cmd_for);
  }
}