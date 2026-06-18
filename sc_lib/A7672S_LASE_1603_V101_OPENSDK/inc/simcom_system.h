/**
 * \simcom_system.h
 *
 * \brief system/sleep functions.
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#ifndef _SIMCOM_SYSTEM_H_
#define _SIMCOM_SYSTEM_H_


#ifdef __cplusplus
extern "C" {
#endif

#define CUS_VERSION       "NULL"

enum SCsystemSF
{
    SC_SYS_SUCCESS = 0,
    SC_SYS_FAIL
};


typedef struct
{
    char *manufacture_namestr;
    char *Module_modelstr;
    char *Revision;
    char *GCAP;
    char *cgmr;
    char *internal_verstr;
    char *version_tagstr;
    char *SDK_Version;
} SIMComVersion;

typedef struct
{
    char sdkversion[50];
} SDKVersion;

typedef struct
{
    char rfversion1[50];
    char rfversion2[50];
} RFVersion;

typedef struct
{
    char cusversion[50];
} CUSVersion;

typedef enum
{
    SC_SYSTEM_SLEEP_DISABLE = 0,
    SC_SYSTEM_SLEEP_ENABLE  = 1
} SC_SYSTEM_SLEEP_FLAG;

typedef struct
{
    SC_SYSTEM_SLEEP_FLAG sleep_flag;
    unsigned char time;
} SC_SleepEx_str;

typedef enum
{
    SC_RXWAKE_NO_EDGE = 0,
    SC_RXWAKE_RISE_EDGE,
    SC_RXWAKE_FALL_EDGE,
    SC_RXWAKE_TWO_EDGE,
}SC_RXWAKE_EDGE_TYPE;

#ifdef CUS_TBDTU

char *sAPI_GetKernelVersion(void);

char *sAPI_GetManuVersion(void);

char *sAPI_GetProjtVersion(void);

char *sAPI_GetHwMajorVersion(void);

char *sAPI_GetHwChildVersion(void);

char *sAPI_GetHwCorrectionVersion(void);

#endif

void sAPI_SysGetVersion(SIMComVersion *simcominfo);
void sAPI_SysGetRFVersion(RFVersion *RFinfo);
void sAPI_SysGetCusVersion(CUSVersion *CUSinfo);
unsigned int sAPI_SysGetImei(char *ImeiValue);
void sAPI_GetSystemInfo(unsigned int *cpuUsedRate, unsigned int *heapFreeSize);
#ifndef NO_AUDIO
#ifdef CUS_CWXT
int sAPI_SetMicGain(unsigned char micgain);
int sAPI_GetMicGain(void);
#endif
#endif
/*****************************************************************************
* FUNCTION
*  sAPI_GetCpuUid
*
* DESCRIPTION
* To get the unique id of CPU
*
* PARAMETERS
*  None
*
* RETURNS
*  unique id
*
* NOTE
*
*****************************************************************************/
unsigned long long sAPI_GetCpuUid(void);

/*****************************************************************************
* FUNCTION
*  sAPI_SystemSleepSet
*
* DESCRIPTION
*  Module will enter into sleep mode after setting flag to 1, module will exit
*from sleep mode after setting flag to 0
*
* PARAMETERS
*  flag     the flag of the system sleep mode
*
* RETURNS
*  0 is OK.
*
* NOTE
*
*****************************************************************************/
int sAPI_SystemSleepSet(SC_SYSTEM_SLEEP_FLAG flag);
/*****************************************************************************
* FUNCTION
*  sAPI_SystemSleepGet
*
* DESCRIPTION
*  Get the flag of the system sleep mode
*
* PARAMETERS
*  None
*
* RETURNS
*  flag
*
* NOTE
*
*****************************************************************************/
SC_SYSTEM_SLEEP_FLAG sAPI_SystemSleepGet(void);

int sAPI_SystemSleepExSet(SC_SYSTEM_SLEEP_FLAG flag, unsigned char time);

SC_SleepEx_str sAPI_SystemSleepExGet(void);

/*****************************************************************************
* FUNCTION
*  sAPI_SystemAlarmClock2Wakeup
*
* DESCRIPTION
* The module will enter sleep mode ,and exit sleep mode after time
*
* PARAMETERS
*  time(ms)     The amount of sleep
*
* RETURNS
*  0 is OK.
*
* NOTE
*
*****************************************************************************/
int sAPI_SystemAlarmClock2Wakeup(unsigned long time);

/*****************************************************************************
* FUNCTION
*  sAPI_SystemSleepRxWakeSet
*
* DESCRIPTION
*  This API is only used for the main serial port!
*  Module will enter into sleep mode after setting flag to 1, module will exit
*  from sleep mode after setting flag to 0 or WAKEUP BY UATR_RX.
*  In this mode, the first frame of data from the serial port serves as a wake-up module,
*  so it cannot receive the first frame of data from the serial port during wake-up!
*
* PARAMETERS
*  flag     the flag of the system sleep mode
*
* RETURNS
*  0 is OK.
*
* NOTE
*  After the module goes into sleep and wakes up, it will notify the MCU side through URC.
*  For example, refer to SC_URC_UART_RX_WAKE_MASK in cus_urc.c.
*****************************************************************************/
int sAPI_SystemSleepRxWakeSet(SC_SYSTEM_SLEEP_FLAG flag);

/*****************************************************************************
* FUNCTION
*  sAPI_SystemSleepRxWakeSetEx
*
* DESCRIPTION
*  Module will enter into sleep mode after setting flag to 1, module will exit
*  from sleep mode after setting flag to 0 or WAKEUP BY UATR_RX.
*  In this mode, the first frame of data from the serial port serves as a wake-up module,
*  so it cannot receive the first frame of data from the serial port during wake-up!
*
* PARAMETERS
*  port       the port ID for serial port wakeups,in port number in SC_Uart_Port_Number enumeration
*  edge_type  the edge trigger method for wakeup
*  flag       the flag of the system sleep mode
*
* RETURNS
*  0 is OK.
*
* NOTE
*  After the module goes into sleep and wakes up, it will notify the MCU side through URC.
*  For example, refer to SC_URC_UART_RX_WAKE_MASK in cus_urc.c.
*****************************************************************************/
int sAPI_SystemSleepRxWakeSetEx(int port, SC_RXWAKE_EDGE_TYPE edge_type, SC_SYSTEM_SLEEP_FLAG flag);

int sAPI_GetSocTemprature(void);

#ifdef __cplusplus
}
#endif

#endif
