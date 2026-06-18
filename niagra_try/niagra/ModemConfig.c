
#include "ModemConfig.h"
#include "simcom_common.h"
#include "simcom_debug.h"
#include "simcom_api.h"
#include "simcom_os.h"
#include "eat_type.h"

ModemConfigContext DeviceConfig =
{
    "0000000000",//1
    "0000000000",//2
	"0000000000",//3
	"0000000000",//4
	"0000000000",//5
	"0000000000",//6
	"0000000000",//7
	"0000000000",//8
	"0000000000",//9
	"0000000000",//10
	"0000000000",//11
	"0000000000",//12
	"0000000000",//13
	"0000000000",//14
	"0000000000",//15
	"0000000000",//16
	"0000000000",//17
	"0000000000",//18
	"0000000000",//19
	"0000000000",//20
	"+91",
	"+91",
	"+91",
	"+91",
	"+91",
	"+91",
	"+91",
	"+91",
	"+91",
	"+91",
	"+91",
	"+91",
	"+91",
	"+91",
	"+91",
	"+91",
	"+91",
	"1234",
	GPRS,
	"www",//"www",//"recomweb",//"aircelgprs.po", // APN
	"", // un
	"", // pwd
	"NIAGARA_OFF",//"airtel_AEC2F5",//"JioFi3_3863E7",//"SATHU",//"JioFi3_8354A0",//"JioFi3_3863E7",//"JioFi3_8354A0",//"NIAGARA", //wifiSSID
	"934450934450",//"myniagara", //wifiPassword
	"utham", //ApSSID
	"93445056", //ApPassword
	"192.168.1.102",//App Server IP
	"3.1.62.165",//TcpServer IP 
	8080,// SocketPort
	"192.168.1.10",//DeviceIp
	20025,//Deviceport
	"192.168.1.10",//MDeviceIp
	20025,//MDeviceport	
	60000, // Server_Interval_Amode in ms
	60000, // Server_Interval_IAmode 
	"", //ICCID
	"54.255.62.34:21",               //FTP IP
	"niagara",						  //FTP USER NAME
	"niagara321",                     //FTP PASSWORD
	"\"/FOTA/\"",
	"52.172.214.208:1883",   	 		  //MQTT IP
	"imsmqtt",               		  //MQTT USER NAME
	"2L9((WonMr",            		  //MQTT PASSWORD
	"FirmwareToApp",                 //MqttPublishTopic
	"AppToFirmware",                  //MqttSubscribeTopic
	"FirmwareToApp",                  //MqttServerTopic
	
};

const ModemConfigContext DeviceConfigDefault =
{ 
    "0000000000",
	"0000000000",
    "0000000000",
	"0000000000",
	"0000000000",	
    "0000000000",
    "0000000000",
	"0000000000",
	"0000000000",
	"0000000000",
	"0000000000",
	"0000000000",
	"0000000000",
	"0000000000",
	"0000000000",
	"0000000000",
	"0000000000",
	"0000000000",
	"0000000000",
	"0000000000",
	"+91",
	"+91",
	"+91",
	"+91",
	"+91",
	"+91",
	"+91",
	"+91",
	"+91",
	"+91",
	"+91",
	"+91",
	"+91",
	"+91",
	"+91",
	"+91",
	"+91",
	"1234",
    GPRS,
	"www",//"www",//"airtelgprs.com",//"recomweb",//"aircelgprs.po", // APN
	"", // un
	"", // pwd
	"NIAGARA_OFF",//"airtel_AEC2F5",//"JioFi3_3863E7",//"SATHU",//"JioFi3_8354A0",//"JioFi3_3863E7",//"JioFi3_8354A0",//"NIAGARA", //wifiSSID
	"934450934420",//"myniagara", //wifiPassword
	"utham", //ApSSID
	"93445056", //ApPassword
	"192.168.1.102",
	"3.1.62.165",//"13.229.211.13",//"35.160.217.50",//"api.agritel.in",//"192.168.225.84",//"52.74.255.105",//"52.74.255.105",//"100.124.9.201",//"52.74.255.105",//"api.agritel.in",//"52.74.224.192", // Server IP 
	8080,// SocketPort
	"192.168.1.10",//DeviceIp
	20025,//Deviceport
	"192.168.1.10",//MDeviceIp
	20025,//MDeviceport
	60000, // Server_Interval_Amode in ms
	60000, // Server_Interval_IAmode 
	"", //ICCID
	"54.255.62.34:21",              //FTP IP
	"niagara",						 //FTP USER NAME
	"niagara321",                    //FTP PASSWORD
	"\"/FOTA/\"",
	"52.172.214.208:1883",   	 		  //MQTT IP
	"imsmqtt",               		  //MQTT USER NAME
	"2L9((WonMr",            		  //MQTT PASSWORD
	"FirmwareToApp",                 //MqttPublishTopic
	"AppToFirmware",                  //MqttSubscribeTopic
	"FirmwareToApp",                  //MqttServerTopic
};
/*
ModemFlashContext DeviceFlash=
{
	0x10322101, // StartAddr -  0x1032000
	0x103A2FFF, // EndAddr
	0x10300000,//0x10322000,// CurrentWAddr
	0x10300000,//0x10322000,// CurrentRAddr
};

const ModemFlashContext DeviceFlashDefault=
{
	0x10322101, // StartAddr
	0x103A2FFF, // EndAddr
	0x10300000,//0x10322000,// CurrentWAddr
	0x10300000,//0x10322000,// CurrentRAddr
};*/
/*
void printFlashParameter(void)
{
	sAPI_Debug("Flash Start Addr:%x\r\n",DeviceFlash.Resvr1);
	sAPI_Debug("Flash End Addr:%x\r\n",DeviceFlash.Resvr2);
	sAPI_Debug("Flash Current WAddr:%x\r\n",DeviceFlash.CurrentWAddr);
	sAPI_Debug("Flash Current RAddr:%x\r\n",DeviceFlash.CurrentRAddr);
}*/

void printParameter(void)
{
    sprintf(buf,"MobileNumber 1:%s\r\n",&DeviceConfig.MobileNumber[SMSN]);
	sAPI_UartPrintf(buf);
    sprintf(buf,"MobileNumber 2:%s\r\n",&DeviceConfig.MobileNumber[SMSX]);
	sAPI_UartPrintf(buf);
	sprintf(buf,"MobileNumber 3:%s\r\n",&DeviceConfig.MobileNumber[SMSY]);
	sAPI_UartPrintf(buf);
    sprintf(buf,"MobileNumber 4:%s\r\n",&DeviceConfig.MobileNumber[SMSZ]);
	sAPI_UartPrintf(buf);
	sprintf(buf,"MobileNumber 5:%s\r\n",&DeviceConfig.MobileNumber[SMSS]);
	sAPI_UartPrintf(buf);
    sprintf(buf,"MobileNumber 6:%s\r\n",&DeviceConfig.MobileNumber[SMS1]);
	sAPI_UartPrintf(buf);
	sprintf(buf,"MobileNumber 7:%s\r\n",&DeviceConfig.MobileNumber[SMS2]);
	sAPI_UartPrintf(buf);
    sprintf(buf,"MobileNumber 8:%s\r\n",&DeviceConfig.MobileNumber[SMS3]);
	sAPI_UartPrintf(buf);
	sprintf(buf,"MobileNumber 9:%s\r\n",&DeviceConfig.MobileNumber[SMS4]);
	sAPI_UartPrintf(buf);
    sprintf(buf,"MobileNumber 10:%s\r\n",&DeviceConfig.MobileNumber[SMS5]);
	sprintf(buf,"MobileNumber 11:%s\r\n",&DeviceConfig.MobileNumber[10]);
	sAPI_UartPrintf(buf);
    sprintf(buf,"MobileNumber 12:%s\r\n",&DeviceConfig.MobileNumber[11]);
	sAPI_UartPrintf(buf);
	sprintf(buf,"MobileNumber 13:%s\r\n",&DeviceConfig.MobileNumber[12]);
	sAPI_UartPrintf(buf);
	sprintf(buf,"MobileNumber 14:%s\r\n",&DeviceConfig.MobileNumber[13]);	
	sAPI_UartPrintf(buf);
	sprintf(buf,"MobileNumber 15:%s\r\n",&DeviceConfig.MobileNumber[14]);
	sAPI_UartPrintf(buf);
	sprintf(buf,"MobileNumber 16:%s\r\n",&DeviceConfig.MobileNumber[15]);
	sAPI_UartPrintf(buf);
	sprintf(buf,"MobileNumber 17:%s\r\n",&DeviceConfig.MobileNumber[16]);
	sAPI_UartPrintf(buf);
	sprintf(buf,"MobileNumber 18:%s\r\n",&DeviceConfig.MobileNumber[17]);
	sAPI_UartPrintf(buf);
	sprintf(buf,"MobileNumber 19:%s\r\n",&DeviceConfig.MobileNumber[18]);
	sAPI_UartPrintf(buf);
	sprintf(buf,"MobileNumber 20:%s\r\n",&DeviceConfig.MobileNumber[19]);
	sAPI_UartPrintf(buf);
	sprintf(buf,"MobileNumber 1:%s\r\n",&DeviceConfig.MobileSmscode[SMSN]);
	sAPI_UartPrintf(buf);
    sprintf(buf,"MobileNumber 2:%s\r\n",&DeviceConfig.MobileSmscode[SMSX]);
	sAPI_UartPrintf(buf);
	sprintf(buf,"MobileNumber 3:%s\r\n",&DeviceConfig.MobileSmscode[SMSY]);
	sAPI_UartPrintf(buf);
    sprintf(buf,"MobileNumber 4:%s\r\n",&DeviceConfig.MobileSmscode[SMSZ]);
	sAPI_UartPrintf(buf);
	sprintf(buf,"MobileNumber 5:%s\r\n",&DeviceConfig.MobileSmscode[SMSS]);
	sAPI_UartPrintf(buf);
    sprintf(buf,"MobileNumber 6:%s\r\n",&DeviceConfig.MobileSmscode[SMS1]);
	sAPI_UartPrintf(buf);
	sprintf(buf,"MobileNumber 7:%s\r\n",&DeviceConfig.MobileSmscode[SMS2]);
	sAPI_UartPrintf(buf);
    sprintf(buf,"MobileNumber 8:%s\r\n",&DeviceConfig.MobileSmscode[SMS3]);
	sAPI_UartPrintf(buf);
	sprintf(buf,"MobileNumber 9:%s\r\n",&DeviceConfig.MobileSmscode[SMS4]);
	sAPI_UartPrintf(buf);
    sprintf(buf,"MobileNumber 10:%s\r\n",&DeviceConfig.MobileSmscode[SMS5]);
	sAPI_UartPrintf(buf);
	sprintf(buf,"MobileNumber 11:%s\r\n",&DeviceConfig.MobileSmscode[10]);
	sAPI_UartPrintf(buf);
    sprintf(buf,"MobileNumber 12:%s\r\n",&DeviceConfig.MobileSmscode[11]);
	sAPI_UartPrintf(buf);
	sprintf(buf,"MobileNumber 13:%s\r\n",&DeviceConfig.MobileSmscode[12]);
	sAPI_UartPrintf(buf);
	sprintf(buf,"MobileNumber 14:%s\r\n",&DeviceConfig.MobileSmscode[13]);
	sAPI_UartPrintf(buf);
	sprintf(buf,"MobileNumber 15:%s\r\n",&DeviceConfig.MobileSmscode[14]);
	sAPI_UartPrintf(buf);
	sprintf(buf,"MobileNumber 16:%s\r\n",&DeviceConfig.MobileSmscode[15]);
	sAPI_UartPrintf(buf);
	sprintf(buf,"MobileNumber 17:%s\r\n",&DeviceConfig.MobileSmscode[16]);
	sAPI_UartPrintf(buf);
	
	sprintf(buf,"smsPassword:%s\r\n",DeviceConfig.smsPassword);
	sAPI_UartPrintf(buf);
    sprintf(buf,"CCID:%s\r\n",DeviceConfig.ICCID);
	sAPI_UartPrintf(buf);
    sprintf(buf,"apnName:%s\r\n",DeviceConfig.apnName);
	sAPI_UartPrintf(buf);
    sprintf(buf,"apnUserName:%s\r\n",DeviceConfig.apnUserName);
	sAPI_UartPrintf(buf);
    sprintf(buf,"apnPassword:%s\r\n",DeviceConfig.apnPassword);
	sAPI_UartPrintf(buf);
	sprintf(buf,"Wifi SSID Name:%s\r\n",DeviceConfig.wifiSSID);
	sAPI_UartPrintf(buf);
    sprintf(buf,"Wifi Password:%s\r\n",DeviceConfig.wifiPassword);
	sAPI_UartPrintf(buf);
    sprintf(buf,"Server Addr:%s\r\n",DeviceConfig.TcpServerIP);
	sAPI_UartPrintf(buf);
	sprintf(buf,"Socket Port:%d\r\n",DeviceConfig.SocketPort);
	sAPI_UartPrintf(buf);
	sprintf(buf,"Server_Interval_Active :%d Mins\r\n",DeviceConfig.Server_Interval_Amode);
	sAPI_UartPrintf(buf);
	sprintf(buf,"FTP SERVER IP:[%s]\r\n",DeviceConfig.FtpServerIP);
	sAPI_UartPrintf(buf);
	sprintf(buf,"FTP SERVER USER NAME :[%s]\r\n",DeviceConfig.ftpUserName);
	sAPI_UartPrintf(buf);
	sprintf(buf,"FTP SERVER PASSWORD :[%s]\r\n",DeviceConfig.ftpPassword);
	sAPI_UartPrintf(buf);
	sprintf(buf,"MQTT SERVER IP:[%s]\r\n",DeviceConfig.MqttServerIP);
	sAPI_UartPrintf(buf);
	sprintf(buf,"MQTT SERVER USER NAME :[%s]\r\n",DeviceConfig.MqttUserName);
	sAPI_UartPrintf(buf);
	sprintf(buf,"MQTT SERVER PASSWORD :[%s]\r\n",DeviceConfig.MqttPassword);
	sAPI_UartPrintf(buf);
	
}

INT32 InitParameter(void)
{
   char bufferConfigContext[1200];
    memset(bufferConfigContext, 0x00, sizeof(bufferConfigContext));
    memcpy((char*)&DeviceConfig,(char*)&DeviceConfigDefault,sizeof(ModemConfigContext));
	
	sAPI_UartPrintf("InitParameter]]\n\r");
    if (app_nvram_read(MCONFIG_AT_INDEX, (void*)bufferConfigContext, sizeof(ModemConfigContext)) < 0)
    {
		sAPI_UartPrintf("Writing defaults\n\r");
        if (app_nvram_save(MCONFIG_AT_INDEX, (void*)&DeviceConfig, sizeof(ModemConfigContext)) < 0)
        {
            return EAT_FALSE;
        }
    }
    else
    {  
          // sprintf(buf,"bufferConfigContext:%s",bufferConfigContext);
		  // sAPI_UartPrintf(buf);
          sAPI_UartPrintf("runnin' else part\n\r");   
          memcpy((char*)&DeviceConfig,bufferConfigContext,sizeof(ModemConfigContext));
    }
    printParameter();
    return EAT_TRUE;
}
/*
INT32 InitFlash(void)
{
   char bufferFlashContext[100];
    memset(bufferFlashContext, 0x00, sizeof(bufferFlashContext));
    memcpy((char*)&DeviceFlash,(char*)&DeviceFlashDefault,sizeof(ModemFlashContext));
    if (app_nvram_read(MCONFIG_FLASH, (void*)bufferFlashContext, sizeof(ModemFlashContext)) < 0)
    {
        if (app_nvram_save(MCONFIG_FLASH, (void*)&DeviceFlash, sizeof(ModemFlashContext)) < 0)
        {
            return EAT_FALSE;
        }
    }
    else
    {
        memcpy((char*)&DeviceFlash,bufferFlashContext,sizeof(ModemFlashContext));
    }
    printFlashParameter();
    return EAT_TRUE;
}*/

void WriteConfig(void *data)
{
	UINT8 Psw[4]={0},cmd_idx=0,config_data[21]={0};
	UINT8 user_msg[160];
	UINT8 i;
	UINT8 *ptr;
	eat_bool update = 0;
    EatEvent_st event;
	while(EAT_TRUE)
    {
	    eat_get_event_for_user(EAT_USER_2, &event);
		switch(event.event)
		{
		case EAT_EVENT_USER_MSG:
			sprintf(user_msg,"%s\0",event.data.user_msg.data);   
			sAPI_Debug("User Msg2: %s", user_msg);
			sscanf(user_msg,"%d %*s %s",&cmd_idx,&config_data);
			sAPI_Debug("idx:%d data:%s",cmd_idx,config_data);	
			switch(cmd_idx)  
			{
				case APN:
					memset(DeviceConfig.apnName, 0x00, sizeof(DeviceConfig.apnName));
                    strncpy(DeviceConfig.apnName,config_data,APN_NAME_LEN);
				break;
				default:
                break;				
			}
			printParameter();
			app_nvram_save(MCONFIG_AT_INDEX, (UINT8*)&DeviceConfig, sizeof(DeviceConfig));
			if(update)
			{
				eat_send_msg_to_user(EAT_USER_5, EAT_USER_4, EAT_FALSE, 3, "CCK", EAT_NULL);
				update=0;
			}
		break;	
		default:
		break;	
		}
	}
}