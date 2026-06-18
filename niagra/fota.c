#include "simcom_api.h"
#include "simcom_fota.h"
#include "stdio.h"
#include "simcom_file_system.h"
#include "simcom_uart.h"
#include "platform.h"
#include "ModemConfig.h"

 
#define APP_OTA_URL "ftp://niagara:niagara321@52.221.191.39:21/customer_app.bin" 
// #define APP_OTA_URL "ftp://mdas:mdas123@183.82.123.227:21/customer_app.bin" 

static sTaskRef fotaProcesser;
static UINT8 fotaProcesserStack[3*1024];
static char FTP_URL_BUF[100]; 
static sFlagRef g_flg1 = NULL;

static void sAPP_AppDownloadProcesser(void * arg)
{
    SCAppDownloadPram pram;
    SCAppDwonLoadReturnCode ret;
    SCAppPackageInfo gAppUpdateInfo = {0};
    int pGreg = 0;
    sAPI_UartPrintf("\n\rFOTA PROC INITED...\n\r");
    while(1)
    {
        sAPI_NetworkGetCgreg(&pGreg);
        if(1 != pGreg)
        {
            sAPI_UartPrintf("FOTA NETWORK STATUS IS [%d]",pGreg);
            sAPI_TaskSleep(3* 200);
        }
        else
        {
            sAPI_UartPrintf("FOTA NETWORK STATUS IS NORMAL");
            break;
        }
    }
     while(1)
    {
		sAPI_UartPrintf("\n\rfota entry");
		sAPI_TaskSleep(2*200);
		if(fotaflsg==1)		
		{
			//pram.url =APP_OTA_URL;
            sprintf(FTP_URL_BUF,"ftp://%s:%s@%s/customer_app.bin",DeviceConfig.ftpUserName,DeviceConfig.ftpPassword,DeviceConfig.FtpServerIP);			
			sAPI_UartPrintf(FTP_URL_BUF);			 
			pram.url=FTP_URL_BUF;			
			pram.mod = SC_APP_DOWNLOAD_FTP_MOD;
			pram.recvtimeout = 25000;

			ret = sAPI_AppDownload(&pram);
			sAPI_UartPrintf("sc_app_download_test download .bin ret[%d] ... ",ret);


			ret = sAPI_AppPackageCrc(&gAppUpdateInfo);
			if(SC_APP_DOWNLOAD_SUCESSED == ret)
			{
				sAPI_UartPrintf("\n\rFOTA Done gonna reset the module\n\r");
				sAPI_TaskSleep(2*200);
				sAPI_SysReset();
			}
			else
			{
		    sAPI_UartPrintf("app package crc fail, errcode = [%d]", ret);
			fotaflsg=0;
			fotafailflag=1;			 
			}
		}
	}


}

void FOTA_Task(void)
{
    SC_STATUS status;
#ifdef USE_FLAG	
	sAPI_FlagCreate(&g_flg1);
#endif
    status = sAPI_TaskCreate(&fotaProcesser, fotaProcesserStack,3*1024,150, "FOTA_Task",sAPP_AppDownloadProcesser,(void *)0);
	sAPI_UartPrintf("FOTA_Task",status);
	
	if(SC_SUCCESS != status)
    {
        sAPI_UartPrintf("Task create fail,status ");
    }        
}