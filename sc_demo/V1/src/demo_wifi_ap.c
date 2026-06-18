/**
  ******************************************************************************
  * @file    demo_wifi.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of wifi scan funtion,it is not full function wifi, just wifi RX for nearby hotpot scaning, mostly for wifi location.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "simcom_os.h"
#include "simcom_wifi_ap.h"
#include "simcom_common.h"
#include "simcom_debug.h"
#include "simcom_uart.h"
#include "uart_api.h"


enum
{

    SC_WIFI_DEMO_WIFIAP_TEST              = 1,
    SC_WIFI_DEMO_MAX                      = 99
};

extern void PrintfOptionMenu(char *options_list[], int array_size);
extern void PrintfResp(char *format);

/**
  * @brief  Event handler for wifi scan.
  * @param  param
  * @note   Will output scaned MAC\channel number\rssi.
  * @retval void
  */

/**
  * @brief  UI demo for WIFI scan.
  * @param  void
  * @note
  * @retval void
  */
void WIFIAPDemo(void)
{
    char flag = 1;
    char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
    char *options_list[] =
    {
        "1. wifiap test",
        "99. back",
    };

    while (flag)
    {
        PrintfResp(note);
        PrintfOptionMenu(options_list, sizeof(options_list) / sizeof(options_list[0]));

        unsigned char opt = UartReadValue();

        switch (opt)
        {
            case SC_WIFI_DEMO_WIFIAP_TEST:
            {
                char SSID[100]={0};

                SC_WIFI_AUTH auth;
                SC_WIFI_ENCRYPT encrypt;
                SC_WIFI_80211_MODE net_mode;
                int wifichannel;
                char password[100]={0};
                //SSID

                sAPI_WiFiStatusSet(API_WIFI_OPEN);
                strcat(SSID,"ssid_demo");
                sAPI_WiFiSSIDSet(SSID);

                memset(SSID,0,100);
                sAPI_WiFiSSIDGet(SSID);
                sAPI_UartPrintf("ssid = %s\r\n",SSID);


                // auth encrypt password
                sAPI_WiFiAuthGet(&auth,&encrypt, password);
                sAPI_UartPrintf("auth = %d, encrypt = %d, password = %s\r\n",auth,encrypt,password);
                memset(password,0,100);
                strcat(password,"1234567890");
                sAPI_WiFiAuthSet(SC_WIFI_WAPI_WPA2,SC_WIFI_TKIP, password);
                memset(password,0,100);
                sAPI_WiFiAuthGet(&auth,&encrypt, password);
                sAPI_UartPrintf("auth = %d, encrypt = %d, password = %s\r\n",auth,encrypt,password);



                //net_mode wifichannel
                sAPI_WiFiMochGet(&net_mode,&wifichannel);
                sAPI_UartPrintf("net_mode = %d, wifichannel = %d\r\n",net_mode,wifichannel);
                sAPI_WiFiMochSet(SC_WIFI_802_11bg,9);
                sAPI_WiFiMochGet(&net_mode,&wifichannel);
                sAPI_UartPrintf("net_mode = %d, wifichannel = %d\r\n",net_mode,wifichannel);



                //mac address ,client number
                unsigned char cnt=sAPI_WiFiCLICNT();
                char **mac=(char **)malloc((cnt+1)*sizeof(char*));
                for (int i = 0; i < cnt+1; i++)
                {
                    mac[i] = (char *)malloc(18 * sizeof(char));
                    if (mac[i] == NULL)
                    {
                        for (int j = 0; j < i; j++)
                        {
                            free(mac[j]);
                        }
                        free(mac);
                        return;
                    }
                 }

                sAPI_WiFiMACAddrGet(mac);
                sAPI_UartPrintf("mac0 = %s\r\n", mac[0]);
                if(mac[0]!= NULL)
                {
                    free(mac[0]);
                    mac[0] = NULL;
                }

                sAPI_WiFiPSMreset();
                break;
            }
            case SC_WIFI_DEMO_MAX:
            {
                flag = 0;
                PrintfResp("\r\nReturn to the previous menu!\r\n");
                break;
            }

            default:
                PrintfResp("\r\nPlease select again:\r\n");
        }
    }
}
