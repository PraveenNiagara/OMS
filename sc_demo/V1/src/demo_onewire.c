/**
  ******************************************************************************
  * @file    demo_helloworld.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of helloworld.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */
 
/* Includes ------------------------------------------------------------------*/
#include "simcom_os.h"
#include "simcom_debug.h"
#include "stdio.h"
#include "simcom_onewire.h"

//#define ONE_WIRE_DEMO_ENABLE
sTaskRef oneWireProcesser = NULL;
static UINT8 oneWireProcesserStack[1024*3];

#ifdef ONE_WIRE_DEMO_ENABLE
static int ct1820b_send_capture_cmd(void)
{
	int ret = sAPI_OneWireSetupReset();
	if( ret < 1 )
	{
		sAPI_Debug("ct1820b: no device detected, %d\r\n", ret);
		return -1;
	}
	if(sAPI_OneWireWriteByte(0xcc) || sAPI_OneWireWriteByte(0x44))
	{
		sAPI_Debug("ct1820b: send capture cmd failed!\r\n");
		return -2;
	}

	sAPI_TaskSleep(10);//50ms;

	return 0;
}


int ct1820b_get_temparature(unsigned int *temprature)
{
    unsigned int read_data[9];

	if(ct1820b_send_capture_cmd())
	{
		sAPI_Debug("ct1820b: send commands failed!\r\n");
		return -1;
	}

	if(sAPI_OneWireRead(read_data,9))
	{
		sAPI_Debug("ct1820b: onew_read_data failed!\r\n");
		return -2;
	}

	*temprature = read_data[0];
	
	return 0;

}

#endif
void sTask_OneWireProcesser(void* argv)
{
    sAPI_Debug("sTask_OneWireProcesser's task runs successfully");
#ifdef ONE_WIRE_DEMO_ENABLE
    sAPI_OneWireInit();

    //unsigned int temprature;
    for(;;)
    {
        sAPI_TaskSleep(200);//1s
#if 0
        if(ct1820b_get_temparature(&temprature) == 0)
        {
        sAPI_Debug("ct1711,temp_value:%f\r\n",temprature);
        }
#else
        sAPI_OneWireWriteByte(0xab);
#endif
    }
#endif
}

/**
  * @brief  helloworld task initial
  * @param  void
  * @note   
  * @retval void
  */
void OneWireDemo(void)
{
    SC_STATUS status = SC_SUCCESS;

    if(oneWireProcesser == NULL)
    {
        status = sAPI_TaskCreate(&oneWireProcesser, oneWireProcesserStack, 1024*3, 150, "oneWireProcesser",sTask_OneWireProcesser,(void *)0);
        if(SC_SUCCESS != status)
        {
            sAPI_Debug("oneWire Task create fail,status = [%d]",status);
        }
    }
}
