#include "app_utility.h"
#include "platform.h"

#define MASTER_MOBILE_NUMBER_LEN			20
#define MASTER_MOBILE_SMSCODE_LEN			10
#define MASTER_MOBILE_CALLCODE_LEN			10
#define APN_NAME_LEN						30 
#define SMS_NAME_LEN						30 
#define APN_USER_NAME_LEN					20
#define APN_PASSWORD_LEN					20
//#define SMS_REGB_LEN                        10
#define SMS_PASSWORD_LEN					10
#define TcpServerIP_LEN						30
#define ICCID_LEN							20
#define FtpServerIP_LEN						30 
#define FTP_SERVER_USER_NAME_LEN			20 
#define FTP_PASSWORD_LEN					20 
#define FIRMWARE_FOLDER_NAME_LEN			20
#define SSID_LEN           			        20
#define PASSWORD_LEN					    20

#define MQTT_SERVER_IP_LEN					30
#define MQTT_SERVER_USER_NAME_LEN			20 
#define MQTT_PASSWORD_LEN					20 
#define MQTT_TOPIC_LEN                      25
typedef enum{SMSN,SMSX,SMSY,SMSZ,SMSS,SMS1,SMS2,SMS3,SMS4,SMS5,SMS6,SMS7}temp_niagara21;
/*
typedef struct ModemFlashContextTag
{
	u32 Resvr1;
	u32 Resvr2;
	u32 CurrentWAddr;
	u32 CurrentRAddr;
}ModemFlashContext;*/
/*
typedef struct ModemConfigMOB
{
	ascii MobileNumber[12][MASTER_MOBILE_NUMBER_LEN+1];
}ModemConfigmob;
extern ModemConfigMOB DeviceMob;
*/
typedef enum{DISBLED,GPRS,WIFI}Interface;
typedef struct ModemConfigContextTag
{
	ascii MobileNumber[20][MASTER_MOBILE_NUMBER_LEN+1];
	//ascii MobileNumber1[2][MASTER_MOBILE_NUMBER_LEN+1];
	ascii MobileSmscode[17][MASTER_MOBILE_SMSCODE_LEN+1];
	//ascii MobileSmscode1[2][MASTER_MOBILE_SMSCODE_LEN+1];
	//ascii MobileCallcode[3][MASTER_MOBILE_CALLCODE_LEN+1];
	//ascii vfbcallnumber[2][MASTER_MOBILE_NUMBER_LEN+1];
	//ascii MobileCallcode1[2][MASTER_MOBILE_CALLCODE_LEN+1];
	//ascii regbnumber[SMS_REGB_LEN+1];
	ascii smsPassword[SMS_PASSWORD_LEN+1];
	Interface interface;
	ascii apnName[APN_NAME_LEN+1];
	ascii apnUserName[APN_USER_NAME_LEN+1];
	ascii apnPassword[APN_PASSWORD_LEN+1];
	ascii wifiSSID[SSID_LEN+1];
	ascii wifiPassword[PASSWORD_LEN+1];
	ascii ApSSID[SSID_LEN+1];
	ascii ApPassword[PASSWORD_LEN+1];
	ascii ApServerIP[TcpServerIP_LEN+1];
	ascii TcpServerIP[TcpServerIP_LEN+1];
	u16 SocketPort;
	ascii DeviceIP[TcpServerIP_LEN+1];
	u16 DevicePort;
	ascii MDeviceIP[TcpServerIP_LEN+1];
	u16 MDevicePort;
	u32 Server_Interval_Amode; // city mode = 1 min , HIghWAy : 5 min ,
	u32 Server_Interval_IAmode;
	u8 ICCID[ICCID_LEN+1];
	ascii FtpServerIP[FtpServerIP_LEN+1]; 
	ascii ftpUserName[FTP_SERVER_USER_NAME_LEN+1]; 
	ascii ftpPassword[FTP_PASSWORD_LEN+1]; 
	ascii firmwareFolderName[FIRMWARE_FOLDER_NAME_LEN+1];
	ascii MqttServerIP[MQTT_SERVER_IP_LEN+1];
	ascii MqttUserName[MQTT_SERVER_USER_NAME_LEN+1];
	ascii MqttPassword[MQTT_PASSWORD_LEN+1];
	ascii MqttPublishTopic[MQTT_TOPIC_LEN+1];
	ascii MqttSubscribeTopic[MQTT_TOPIC_LEN+1];
	ascii MqttServerTopic[MQTT_TOPIC_LEN+1];
	 
	
}ModemConfigContext;

//extern ModemFlashContext DeviceFlash;
//extern const ModemFlashContext DeviceFlashDefault;
extern ModemConfigContext DeviceConfig;
extern const ModemConfigContext DeviceConfigDefault;
extern void WriteConfig(void *data);