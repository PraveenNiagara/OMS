#include "ModemConfig.h"
#include "app_at_cmd_envelope.h"
#include "livedata.h"
#include "platform.h"
#include "app_utility.h"
#include "app_custom.h"

#include "simcom_common.h"
#include "simcom_debug.h"
#include "simcom_api.h"
#include "eat_type.h"

#define GPRS_TCP_DISABLE // http
// #define GPRS_TCP_ENABLE
 UINT8 PostData[1300];
 UINT32 PrevCurrentAddr=0;
 //UINT8 SCK_buf[500]={0};//500
  UINT8 SCK_buf[100]={0};//500
 unsigned char cmdlen; 
 unsigned char HttpInPrograssfailcount=0,httpinitfailcount=0;
//static UINT8 wifi_rx_buf[2048] = {0};//200
static UINT8 wifi_rx_buf[50] = {0};//200
//WifiCmd WifiCmdState = CMD;
//WifiState wifiState = INIT;       
eat_bool WifiReady = EAT_FALSE;
eat_bool Enable_AP_Mode = EAT_FALSE;
eat_bool testbool = EAT_TRUE; // just for testing post and get
eat_bool HttpInPrograssState = EAT_FALSE;

UINT8 MqttInPrograssState=0;
sTimerRef timerRef;

void callBackRoutine();

#if 0
 const WfCmdT WtCmd[]={
		{"$$$","CMD"},
        {"set ip dhcp 0\r ","AOK"},
		{"set wlan ssid","AOK"},
		{"set wlan phrase","AOK"},
		{"set wlan auth 4\r ","AOK"},
		{"set wlan join 1\r","AOK"}, // 1
		{"set ip protocol 8\r","AOK"}, // 8		
		{"set ip host","AOK"},
		{"set ip remote","AOK"},
		{"set uart baudrate 115200 \r","AOK"},
		{"set uart mode 0x00 \r","AOK"},
		{"set comm idle 20 \r","AOK"},
		{"set apmode ssid ","AOK"},
		{"set apmode passphrase ","AOK"},
		{"set ip address","AOK"},
		{"set ip localport","AOK"},
		{"set comm remote 0\r","AOK"},
		{"save\r ","AOK"},
		{"reboot\r ","AOK"},		
       {"set apmode channel 1\r ","AOK"},
		{"set wlan join 1\r","AOK"}, // 1
		{"set ip protocol 2\r","AOK"}, // 8		
		{"save\r ","AOK"},
		{"reboot\r ","AOK"},		
        {"set apmode channel 6\r ","AOK"},
		{"set wlan join 7\r","AOK"}, // 1
		{"set ip protocol 2\r","AOK"}, // 8		
		{"save\r ","AOK"},
		{"reboot\r ","AOK"},	
		{"open","*OPEN*"}, //open <ip><port>
		{"close\r","*CLOS*"},
        {"exit\r","EXIT"},
		{"$$$","CMD"},
		{"open","*OPEN*"}, //open <ip><port>
		{"close\r","*CLOS*"},
        
    };
#endif
UINT8 DIS_BUF[80]="";
UINT8 VAL_BUF[80]="";
UINT8 SMS_BUF[350]="";//200

eat_bool LedData_rate=EAT_FALSE;
eat_bool Egprs=0;
eat_bool Sapbr = 0;
EatRtc_st Datetime={0};
UINT8 Aletrt_retry_count=0;
/* eat_bool BLK_DATA = EAT_FALSE;   //dg_nsdk
eat_bool IGN_OFF_STATUS = EAT_FALSE;
eat_bool ACTIVE_MODE = EAT_TRUE; */
// @SR_NUM_5,OK,csum_3$   -- > @2,0,0$
eat_bool SCK_CON=0,SCK_CLOSE=1,BLK_CON=0,BLK_CLOSE=0,CCK_CON=0,CCK_CLOSE=0;
//UINT8 *ServerAckTable[] = {"$ERR#","$OK#","$CNFK@","$CNFR@"};
//{Server_ERROR,Server_OK,Server_CONFIG,Server_ALERT,Server_REQ,Server_FUEL,Server_ROUTE,Server_DCONFIG,Server_PROFILE};
/*UINT32 DevicePrevRAddr=0x10300000;
const UINT32 DeviceFlashStartAddr = 0x10300000; //UINT32 DeviceFlash.StartAddr
const UINT32 DeviceFlashBank0EndAddr = 0x10350FFF;
const UINT32 DeviceFlashBank1StartAddr =0X10360000;
const UINT32 DeviceFlashEndAddr =0x103A1FFF; // UINT32 DeviceFlash.EndAddr*/

UINT8 SOCKET_ERROR_COUNT;
s8 bsid=-1;
s8 csid=-1;

#if 0
sockaddr_struct g_server_address =
{
    SOC_SOCK_STREAM,
    4,
    50025,              //port
    {110,225,25,85} // 109.203.99.216 - 110.225.25.85
};

UINT8 *SOC_EVENT[]=
{
    "SOC_READ",
    "SOC_WRITE",  
    "SOC_ACCEPT", 
    "SOC_CONNECT",
    "SOC_CLOSE", 
    "SOC_ACKED"
};

UINT8 *BEARER_STATE[]=
{
    "DEACTIVATED",
    "ACTIVATING",
    "ACTIVATED",
    "DEACTIVATING",
    "CSD_AUTO_DISC_TIMEOUT",
    "GPRS_AUTO_DISC_TIMEOUT",
    "NWK_NEG_QOS_MODIFY",
    "CBM_WIFI_STA_INFO_MODIF",
};
#endif
eat_bool simcom_fota_update(UINT8 *username, UINT8 *pwd, UINT8* getName,UINT8* getPath, UINT8* serv, UINT8 port);
s8 simcom_connect_server(sockaddr_struct *addr);
s32 simcom_send_to_server(s8 sid, const void *buf, s32 len);
//static void WifiWriteCmd(WifiCmd Cmd);


ResultNotifyCb ftpgettofs_final_cb(eat_bool result)
{
    sAPI_Debug("ftpgettofs_cb final result = %d\r\n", result);
    if (result)
    {       
		sAPI_Debug("\r\nAPP UPDATE...\r\n");
     //   app_update("c:\\User\\Ftp\\app.bin");
    }
    else
        sAPI_Debug("FTP download fail");
}

ResultNotifyCb ftpgettofs_cb(eat_bool result)
{
    sAPI_Debug("ftpgettofs_cb result = %d\r\n", result);
    if(!result)             
       sAPI_Debug("ftpgettofs fail");
	   else
	   ftpgettofs_cb_flag=1;
}

eat_bool simcom_fota_update(UINT8 *username, UINT8 *pwd, UINT8* getName,UINT8* getPath, UINT8* serv, UINT8 port)
{
	sAPI_Debug("\r\nAPP DOWNLOAD...\r\n");
    simcom_ftp_down_file(username, pwd, getName, getPath,serv, port, "app.bin", ftpgettofs_cb, ftpgettofs_final_cb);
}

#if 0
void soc_notify_cb(s8 s,soc_event_enum event,eat_bool result, UINT16 ack_size)
{
    UINT8 buffer[10] = {0};
    UINT8 id = 0; 
	UINT8 *ServerAckTable[] = {"$ERR#","$OK#","$CNFK@","$CNFR@"};
    if(event & SOC_READ)
    {
        INT16 len = 0;
        UINT8  buf[150]={0};  
        len = eat_soc_recv(s,buf,sizeof(buf));
	if(len>0)
		{
			UINT8  *p = EAT_NULL;
			UINT8 i;
			s8 rspType= -1;
			sAPI_Debug("Recv id=%d,len=%d,buf= %s",s,len,buf);
			p = buf;
			//sAPI_Debug("ServerAckTable[i]=%d",sizeof(ServerAckTable));
			//sAPI_Debug("ServerAckTable[i]=%d",sizeof(ServerAckTable[0]));
			//for (i = 0; i < 4; i++)
			//for (i = 0; i < sizeof(ServerAckTable); i++)
			
			//{
			//	sAPI_Debug("ServerAckTable[%d]=%d",i,ServerAckTable[i]);
			//}
			for (i = 0; i < sizeof(ServerAckTable) / sizeof(ServerAckTable[0]); i++)
			//for (i = 0; i < sizeof(ServerAckTable); i++)
			
			{
				sAPI_Debug("ServerAckTable[%d]=%s",i,ServerAckTable[i]);
				if (!strncmp(ServerAckTable[i], p, strlen(ServerAckTable[i])))
				{
					rspType = i;
					break;
				}
			}
             sAPI_Debug("rspType =%d",rspType);
			if(rspType >=0)
			{
				char * pch,j;
				char PARAM[25]={0};
				switch(rspType)
				{
					case Server_ERROR:
						sAPI_TimerStop(OK_TIMER);
						if(errorcount++>3)
				        {
				       // Nooftcpprocessed++;
				        errorcount=0;
				        }
                       //Nooftcpprocessed++;	
                   sAPI_Debug("ERRORrspType =%d",rspType);					   
					break;
					case Server_OK:
						sAPI_TimerStop(OK_TIMER);
                        sAPI_TimerStop(BLOCKOUT_Response_TIMER);
                        //strcpy((char *)wifiStrNumber[Noofsettingsrecvd].Wifistr,(char*)PARAM);
				        //Noofsettingsrecvd++;
						sAPI_Debug("OKrspType =%d",rspType);
						//Nooftcpprocessed++;
										
					break;
					case Server_CONFIG: // $CNF@DATA@ // Settings
						sAPI_TimerStop(OK_TIMER);
						sAPI_TimerStop(BLOCKOUT_Response_TIMER);
						sscanf(buf,"$CNFK@%s@",PARAM);
						strcpy((char *)wifiStrNumber[Noofsettingsrecvd].Wifistr,(char*)PARAM);
				        Noofsettingsrecvd++;
				
						// pch = strtok(&buf[5], ",#");
						// TRIGGER HERE TO SEND DATA OVER UART
					break;
					case Server_CONFIGR:
						sAPI_TimerStop(OK_TIMER);
						sscanf(buf,"$CNFR@%s@",PARAM);
						strcpy((char *)wifiStrNumber[Noofsettingsrecvd].Wifistr,(char*)PARAM);
                        Noofsettingsrecvd++;
					break;
				}				
			}			
		}
    }
    else if (event&SOC_WRITE) id = 1;
    else if (event&SOC_ACCEPT) id = 2;
    else if (event&SOC_CONNECT)
    {
        id = 3;   
		SCK_CON = 1;      
		sAPI_TimerStop(RESPONSE_TIMER);
    }
    else if (event&SOC_CLOSE){ 
        id = 4;
		SCK_CLOSE = 1;
		SCK_CON = 0;
        eat_soc_close(s);
    }
    else if (event&SOC_ACKED) id = 5;
    if (id == 5)
        sprintf(buffer,"SOC_NOTIFY:%d,%d,%d\r\n",s,SOC_EVENT[id],ack_size);
    else 
        sprintf(buffer,"SOC_NOTIFY:%d,%d,%d\r\n",s,SOC_EVENT[id],result);
    sAPI_Debug(buffer,strlen(buffer));
    sAPI_Debug("soc_notify_cb");
}
#endif
#if 0
s8 simcom_create_server(UINT16 port)
{
    UINT8 val;    
    s8 ret;
    s8 server_socket;    
    sockaddr_struct address={0};
 //eat_soc_notify_register(soc_notify_cb); wait
    server_socket = eat_soc_create(SOC_SOCK_STREAM,0);
    if(server_socket < 0)
        sAPI_Debug("eat_soc_create() return error :%d",server_socket);
    val = TRUE;
    ret = eat_soc_setsockopt(server_socket, SOC_NBIO, &val, sizeof(val));
    if (ret != SOC_SUCCESS)
        sAPI_Debug("eat_soc_setsockopt() return error :%d",ret);
    val = (SOC_READ | SOC_WRITE | SOC_CLOSE | SOC_CONNECT | SOC_ACCEPT);
    ret = eat_soc_setsockopt(server_socket,SOC_ASYNC,&val,sizeof(val));
    if (ret != SOC_SUCCESS)
        sAPI_Debug("eat_soc_setsockopt() return error :%d",ret);
    address.port = port;
    eat_soc_bind(server_socket,&address);
    eat_soc_listen(server_socket,1);
    return server_socket;
}
#endif
 
/* void bear_notify_cb(cbm_bearer_state_enum state,UINT8 ip_addr[4])
{
    UINT8 buffer[128] = {0};
    UINT8 id = 0;
    sAPI_Debug("bear_notify_cb");
    if (state & CBM_DEACTIVATED) id = 0;
    else if (state & CBM_ACTIVATING) id = 1;
    else if (state & CBM_ACTIVATED) 
	{
		id = 2;
		sprintf(buffer,"BEAR_NOTIFY:%d,%d:%d:%d:%d\r\n",BEARER_STATE[id],ip_addr[0],ip_addr[1],ip_addr[2],ip_addr[3]);
		eat_gprs_bearer_hold(); // eat_gprs_bearer_release(); 
		if(!Egprs)
			gethostbyname(DeviceConfig.TcpServerIP);	
	}
    else if (state & CBM_DEACTIVATING) id = 3; //GPRS_DIC=1;		
    else if (state & CBM_CSD_AUTO_DISC_TIMEOUT) id = 4;
    else if (state & CBM_GPRS_AUTO_DISC_TIMEOUT) id = 5;
    else if (state & CBM_NWK_NEG_QOS_MODIFY) id = 6;
    else if (state & CBM_WIFI_STA_INFO_MODIFY) id = 7;    
	sprintf(buffer,"BEAR_NOTIFY:%d\r\n",BEARER_STATE[id]);		
	sAPI_Debug(buffer,strlen(buffer));
}
 */

/* void gprs_start(UINT8* apn,UINT8* userName,UINT8* password)
{
  // eat_gprs_bearer_open(apn, userName, password,bear_notify_cb); wait
} */

#if 0
s8 simcom_connect_server(sockaddr_struct *addr)
{
    UINT8 val = 0;
    s8 ret = 0;
    s8 sid;
	//eat_soc_notify_register(soc_notify_cb); //register socket callback
    sid = eat_soc_create(SOC_SOCK_STREAM,0);	
	sAPI_Debug("eat_soc_create :%d , Port %d",sid,g_server_address.port);
    if(sid < 0)
	{
        sAPI_Debug("eat_soc_create return error :%d",sid);
	}
    val = (SOC_READ | SOC_WRITE | SOC_CLOSE | SOC_CONNECT | SOC_ACCEPT);
    ret = eat_soc_setsockopt(sid,SOC_ASYNC,&val,sizeof(val));
    if (ret != SOC_SUCCESS)
        sAPI_Debug("eat_soc_setsockopt 1 return error :%d",ret);
    
    val = TRUE; // FALSE
    ret = eat_soc_setsockopt(sid, SOC_NBIO, &val, sizeof(val));
    if (ret != SOC_SUCCESS)
        sAPI_Debug("eat_soc_setsockopt 2 return error :%d",ret);
    
    val = TRUE; // FALSE
    ret = eat_soc_setsockopt(sid, SOC_NODELAY, &val, sizeof(val));
    if (ret != SOC_SUCCESS)
        sAPI_Debug("eat_soc_setsockopt 3 return error :%d",ret);
		
    ret = eat_soc_connect(sid,addr); 
    if(ret >= 0){
        sAPI_Debug("NEW Connection ID is :%d",ret);
    }
    else if (ret == SOC_WOULDBLOCK) {
        sAPI_Debug("Connection is in progressing");
    }
    else {
        sAPI_Debug("Connect return error:%d",ret);
    }
    return sid;
}

s32 simcom_send_to_server(s8 sid, const void *buf, s32 len)
{
    s32 ret = 0;
    ret = eat_soc_send(sid,buf,len);
    if (ret < 0)
	{
        sAPI_Debug("eat_soc_send return error :%d",ret);
		switch(ret) // SOC_NOTCONN(-12) socket is not connected  , -4 invalid socket , -10 invalid argument , -13 msg is too long
		{	
			case SOC_NOTCONN:// -12
				SCK_CLOSE=1;
				eat_soc_close(sid);
				//sid=-1;
				break;
			case SOC_INVALID_SOCKET: //-4
				break;
			case SOC_INVAL: // -10
				break;
				default:
				break;
		}
	}
    else
        sAPI_Debug("eat_soc_send success :%d",ret);
    return ret;
}

s32 simcom_recv_from_server(s8 sid, void *buf, s32 len)
{
    s32 ret = 0;
    ret = eat_soc_recv(sid,buf,len);
    if(ret == SOC_WOULDBLOCK){
        sAPI_Debug("eat_soc_recv no data available");
    }
    else if(ret > 0) {
        sAPI_Debug("eat_soc_recv data:%s",buf);
    }
    else{
        sAPI_Debug("eat_soc_recv return error:%d",ret);
    }
}

void hostname_notify_cb(UINT32 request_id,eat_bool result,UINT8 ip_addr[4])
{
  
	if(!result)
	{
			gethostbyname(DeviceConfig.TcpServerIP);
	}
	else
	{
		Egprs = 1;
		g_server_address.addr[0]=ip_addr[0];                
		g_server_address.addr[1]=ip_addr[1];
		g_server_address.addr[2]=ip_addr[2];
		g_server_address.addr[3]=ip_addr[3];
		sAPI_Debug("hostname_notify_cb:,%d,%d,%d,%d",ip_addr[0],ip_addr[1],ip_addr[2],ip_addr[3]); 
	}
		
		
}
#endif

/*wait
eat_bool gethostbyname(const UINT8 *domain_name)
{ 
    UINT8 len;
    UINT8 ipaddr[4];    
    s32 result = 0;
    eat_bool ret = EAT_FALSE;     
    eat_soc_gethost_notify_register(hostname_notify_cb);
    result = eat_soc_gethostbyname(domain_name,ipaddr,&len,1234);
    if (SOC_SUCCESS == result){
       // UINT8 buffer[128] = {0};
        sAPI_Debug("eat_soc_gethostbyname success");
      //  sprintf(buffer,"HOSTNAME:%d,%d:%d:%d:%d\r\n",ipaddr[0],ipaddr[1],ipaddr[2],ipaddr[3]);
       // sAPI_UartWrite(EAT_UART_1,buffer,strlen(buffer));
        ret = EAT_TRUE;
    } else if(SOC_WOULDBLOCK == result){
        sAPI_Debug("eat_soc_gethostbyname wait callback function");
    } else
        sAPI_Debug("eat_soc_gethostbyname error");
    return ret;
}
*/

void gprs_connect(void)
{
//	if(!strcmp(DeviceConfig.apnName,"")) 
	//{
//		gprs_start(DeviceConfig.apnName,DeviceConfig.apnUserName,DeviceConfig.apnPassword);
	//}
//	else if(!strcmp(apn,""))
//	{
		gprs_start("recomweb",DeviceConfig.apnUserName,DeviceConfig.apnPassword);
//	}
//	else
//	{
//;
//	}
}
#if 0		
void InitState()
{
	switch(WifiCmdState)
	{
		case SETIPDHCP:
		WifiWriteCmd(SETWLANSSID);
		break;
		case SETWLANSSID:
		WifiWriteCmd(SETWLANPHARSE);
		break;					
		case SETWLANPHARSE:
		WifiWriteCmd(SETWLANAUTH);
		break;										
		case SETWLANAUTH:
		WifiWriteCmd(SETWLANJOINONE); 
		break;
		case SETWLANJOINONE: 
		WifiWriteCmd(SETIPPROTOCOLEIGHT); 
		break;
		case SETIPPROTOCOLEIGHT: 
		WifiWriteCmd(SETIPHOST);
		break;
		case SETIPHOST:
		WifiWriteCmd(SETIPREMOTEPORT);
		break;
		case SETIPREMOTEPORT:
		WifiWriteCmd(SETBAUD);
		break;
		case SETBAUD:
		WifiWriteCmd(SETUARTMODE);
		break;
        case SETUARTMODE:
		WifiWriteCmd(IDLETIME);
		break;			
		case IDLETIME:
		WifiWriteCmd(SETAPMODESSID);
		break;					
		case SETAPMODESSID:
		WifiWriteCmd(SETAPMODEPASSPHRASE);
		break;										
		case SETAPMODEPASSPHRASE:
		WifiWriteCmd(SETWPSIPADDRESS);
		break;
		case SETWPSIPADDRESS:
		WifiWriteCmd(SETWPSLOCALPORT);
		break;
		case SETWPSLOCALPORT:
		WifiWriteCmd(SETCOMMREMOTE);
		break;
		case SETCOMMREMOTE:
       // wifimodesetfalg=0;	
        rebootflag1=1;	
        tcpdcounter1=0;	
		//wifiState=INITTCP;
   		//wifiState==INITTCP;
		WifiWriteCmd(SAVE);
		//wifimodesetfalg=0;
		break;
		
		default:
		break;
	}
}

 void InitTCPState()
{
	switch(WifiCmdState)
	{
		case SETWLANJOINONE: 
		WifiWriteCmd(SETIPPROTOCOLEIGHT); 
		break;
		case SETIPPROTOCOLEIGHT: 
		//wifimodesetfalg=0;
		rebootflag=1;
		tcpdcounter1=0;
		WifiWriteCmd(SAVE);
		//wifimodesetfalg=0;
		break;
		
		default:
		break;
	}
}
       
void InitSAPState()
{
	switch(WifiCmdState)
	{
		case SETAPMODECHANNELSIX:
		WifiWriteCmd(SETAPWLANJOINSEVEN); 
		break;
		case SETAPWLANJOINSEVEN: 
		WifiWriteCmd(SETAPIPPROTOCOLTWO); 
		break;
		case SETAPIPPROTOCOLTWO: 
         //wifimodesetfalg=0;
		rebootflag=1;
		tcpdcounter1=0;
		WifiWriteCmd(SAVE);
		//wifimodesetfalg=0;
		break;
		
		default:
		break;
	}
}
		
void InitWPSState()
{
	switch(WifiCmdState)
	{
		case SETWPSMODECHANNELONE:
		WifiWriteCmd(SETWPSWLANJOINONE); 
		break;
		case SETWPSWLANJOINONE: 
		WifiWriteCmd(SETWPSIPPROTOCOLTWO);
		break;
		case SETWPSIPPROTOCOLTWO:
//wifimodesetfalg=0;
		rebootflag=1;
		tcpdcounter1=0;
		WifiWriteCmd(SAVE);
		//wifimodesetfalg=0;
		break;
		
		default:
		break;
	}
}
 
static void WifiWriteCmd(WifiCmd Cmd)
{
	char WriteCmdBuf[40] = "";
	WifiCmdState = Cmd;
	//commandflag=1;
	switch(WifiCmdState)
	{
		case SETWLANSSID:
		sprintf(WriteCmdBuf,"%s %s\r",WtCmd[Cmd].p_WtCmdStr,DeviceConfig.wifiSSID);
		break;
		case SETWLANPHARSE:
		sprintf(WriteCmdBuf,"%s %s\r",WtCmd[Cmd].p_WtCmdStr,DeviceConfig.wifiPassword);
		break;
		case SETIPHOST:
		sprintf(WriteCmdBuf,"%s %s\r",WtCmd[Cmd].p_WtCmdStr,DeviceConfig.TcpServerIP);
		break;
		case SETIPREMOTEPORT:
		sprintf(WriteCmdBuf,"%s %d\r",WtCmd[Cmd].p_WtCmdStr,DeviceConfig.SocketPort);
		break;
		case SETWPSIPADDRESS:
		sprintf(WriteCmdBuf,"%s %s\r",WtCmd[Cmd].p_WtCmdStr,DeviceConfig.DeviceIP);
		break;
		case SETWPSLOCALPORT:
		sprintf(WriteCmdBuf,"%s %d\r",WtCmd[Cmd].p_WtCmdStr,DeviceConfig.DevicePort);
		break;
		//case JOINAPMODE:
		//sprintf(WriteCmdBuf,"%s %s %s\r",WtCmd[Cmd].p_WtCmdStr,DeviceConfig.wifiSSID,DeviceConfig.wifiPassword);
		//break;
		case SETAPMODESSID:
		sprintf(WriteCmdBuf,"%s %s\r",WtCmd[Cmd].p_WtCmdStr,DeviceConfig.ApSSID);
		break;
		case SETAPMODEPASSPHRASE:
		sprintf(WriteCmdBuf,"%s %s\r",WtCmd[Cmd].p_WtCmdStr,DeviceConfig.ApPassword);
		break;
		/*case SETWPSMODESSID:
		sprintf(WriteCmdBuf,"%s %s\r",WtCmd[Cmd].p_WtCmdStr,DeviceConfig.ApSSID);
		break;
		case SETWPSMODEPASSPHRASE:
		sprintf(WriteCmdBuf,"%s %s\r",WtCmd[Cmd].p_WtCmdStr,DeviceConfig.ApPassword);
		break;
		*/
		case CLIENTOPEN:
			sprintf(WriteCmdBuf,"%s %s %d\r",WtCmd[Cmd].p_WtCmdStr,DeviceConfig.TcpServerIP,DeviceConfig.SocketPort);
		//sprintf(WriteCmdBuf,"%s \r",WtCmd[Cmd].p_WtCmdStr);
		//wifiopenflag=1;
		break;
		case CLIENTCMD:
		sAPI_UartWrite(eat_uart_wifi, WtCmd[Cmd].p_WtCmdStr,3);
		break;
		case CMD:
			sAPI_Debug("Write CMD $$$\n");
			sAPI_UartWrite(eat_uart_wifi, WtCmd[Cmd].p_WtCmdStr,3);
		break;
		default:
			sAPI_Debug("Default cmd\n");
			//memset(WriteCmdBuf,0,40);
			sprintf(WriteCmdBuf,"%s",WtCmd[Cmd].p_WtCmdStr);
			sAPI_Debug("Wcmd=%s, buf %s",WtCmd[Cmd].p_WtCmdStr,WriteCmdBuf);
		break;
	}
	cmdlen=strlen(WriteCmdBuf);
	sAPI_Debug("wifi idx %d ,cmd=%s,%d",Cmd,WriteCmdBuf,cmdlen);
	if(CMD!=Cmd || Cmd != CLIENTCMD)
		sAPI_UartWrite(eat_uart_wifi, WriteCmdBuf,strlen(WriteCmdBuf));
}

void Wifi_Entry_CMD_Mode()
{
	WifiWriteCmd(CMD);
}

void Wifi_Entry_Client_Mode()
{
	//wifiState = CLIENTCONFIG;
	WifiWriteCmd(CLIENTCMD);
}
void Set_AP_Mode()
{
	Enable_AP_Mode = EAT_TRUE;
}
#endif

void Send_Packet(s8 sid)
{
	//UINT8 SCK_buf[500]={0};
	s32 Resp = 0;
    //UINT8 cmd_idx=0;
	//s8 sid=-1;
	
	eat_get_rtc(&Datetime); 
	/*if(!strcmp(DIS_BUF,""))
		strcpy(DIS_BUF,"0");
	if(!strcmp(VAL_BUF,""))
		strcpy(VAL_BUF,"0");
	if(!strcmp(SMS_BUF,""))
		strcpy(SMS_BUF,"0");
	sprintf(SCK_buf,"{%s(9845200686#WMS#20%02d-%d-%d#%d:%d:%d#%s#%s#$D,%s)}",IMEI,Datetime.year,Datetime.mon,\
	Datetime.day,Datetime.hour,Datetime.min,Datetime.sec,DIS_BUF,VAL_BUF,SMS_BUF);*/
	strcpy(SCK_buf,TCPwifiStrNumber[Nooftcpprocessed].TCPWifistr);

	sAPI_Debug("SCK: %s",SCK_buf);										
	if(DeviceConfig.interface == WIFI)				
	{	
        if(sid==0)
		{
			//sAPI_UartWrite(eat_uart_wifi, SCK_buf, strlen(SCK_buf));
			//sAPI_TimerStart(OK_TIMER,OK_TIMER_PERIOD);
		}
	   // else
		//FlashWrite(SCK_buf);
			
	}
	else
	{
		Resp= simcom_send_to_server(sid,&SCK_buf, strlen(SCK_buf));  
	}  
	memset(&DIS_BUF,0,80);
	memset(&VAL_BUF,0,80);
	memset(&SMS_BUF,0,200);
	if(Resp<0)
	{
		// to be add later
		//FlashWrite(SCK_buf);
	}
	else
	{ 	if(DeviceConfig.interface == GPRS)
		sAPI_TimerStart(timerRef,OK_TIMER_PERIOD,RESPONSE_TIMER_PERIOD,*callBackRoutine,OK_TIMER); 
	}
}

#if 0
void wifi_rx_proc(const EatEvent_st* event)
{
    UINT16 len;
	int dricomdelay=0;
	UINT8* msg_ptr=EAT_NULL; 
    SC_Uart_Port_Number uart = event->data.uart.uart;
	sAPI_Debug("\r\n******************* wuart :%d\r\n",uart);
    if(eat_uart_wifi == uart)
	{
		len = sAPI_UartRead(uart, wifi_rx_buf, EAT_UART_RX_BUF_LEN_MAX);
		if(len != 0)
		{
			wifi_rx_buf[len] = '\0';
			sAPI_Debug("[%s] uart(%d) rx: %s %d", __FUNCTION__, uart, wifi_rx_buf,len);
			if(wifimodesetfalg==1)
			{
				if(len<(cmdlen+13))
				{
				 //while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
				 //WifiWriteCmd(WifiCmdState);
				commandflag=0;
			    }
			}
			
			if((strstr(wifi_rx_buf,"WIFLY")!= 0)||(strstr(wifi_rx_buf,"<W")!= 0)||(strstr(wifi_rx_buf,">")!= 0))
			{
					commandflag=1;
					
			}
			
            // WifiCmdState			
		    //msg_ptr = eat_mem_alloc(strlen(uart_read_buf));    
		   // memcpy(uart_read_buf, wifi_rx_buf, len); 
		    //strcpy((char *)uart_read_buf,(char*)wifi_rx_buf);
			/*if(strstr(wifi_rx_buf,"WIFLY")!= 0)
				{
					commandflag=1;
					
				}*/
		   /* if(strstr(wifi_rx_buf,"DHCP")!= 0 )
			{		
			commandflag=0;
	        }*/
			
            if(WifiCmdState == SAVE ||WifiCmdState==SETWPSSAVE||WifiCmdState==SETAPSAVE)
			{
				if(strstr(wifi_rx_buf,"WIFLY")!= 0)
				{
					//WifiWriteCmd(REBOOT);
					commandflag=0;
					if(s_nMSettings.m_Enter>=70 && wpsmodeon==0 && apmodeon==0&&tcpopenflag==0 )
					{
				    tcpopenflag=1;
					wifiopenflag=0;
					}

					//wifimodesetfalg=0;
				}
				else if((strstr(wifi_rx_buf,"W")!= 0)||(strstr(wifi_rx_buf,"<")!= 0))
				{
					//WifiWriteCmd(REBOOT);
					commandflag=0;
					if(s_nMSettings.m_Enter>=70 && wpsmodeon==0 && apmodeon==0&&tcpopenflag==0 )
					{
				    tcpopenflag=1;
					wifiopenflag=0;
					}        //wifimodesetfalg=0;
				}
			}
			
		    else if(strstr(wifi_rx_buf,"$OK#")!= 0)
			{
				if(tcpopenflag==1)
				{
				sAPI_TimerStop(OK_TIMER);
				//Nooftcpprocessed++;
				errorcount=0;
				//sAPI_TimerStop(BLOCKOUT_Response_TIMER);
				//wifiState = CLIENTONCLOSE;
				}
			}
			else if((strstr(wifi_rx_buf,"$ERR#")!= 0))
			{
				/*if(tcpopenflag==1)
				{
					wifiopenflag=1;
				//sAPI_TimerStop(OK_TIMER);
				//Send_Packet(0);
				}*/
				if(errorcount++>3)
				{
				//Nooftcpprocessed++;
				errorcount=0;
				}
			}
			else if((strstr(wifi_rx_buf,"$CNFK@")!= 0))
			{
				char PARAM1[] ="";
				if(tcpopenflag==1)
				{
				sAPI_TimerStop(OK_TIMER);
				//Noofsettingsrecvd++;
				sscanf(wifi_rx_buf,"$CNFK@%s@",PARAM1);
				strcpy((char *)wifiStrNumber[Noofsettingsrecvd].Wifistr,(char*)PARAM1);
				Noofsettingsrecvd++;
				}
			}
			else if((strstr(wifi_rx_buf,"$CNFR@")!= 0))
			{
				char PARAM1[] ="";
				if(tcpopenflag==1)
				{
				sAPI_TimerStop(OK_TIMER);
				sscanf(wifi_rx_buf,"$CNFR@%s@",PARAM1);
				strcpy((char *)wifiStrNumber[Noofsettingsrecvd].Wifistr,(char*)PARAM1);
				Noofsettingsrecvd++;
				//Send_Packet(0);
				}
			}
			else if(strstr(wifi_rx_buf,"$$$")!= 0)
			{  
		     char textBuf[30];
		 
			 sprintf(textBuf,"exit\r");
             sAPI_UartWrite(eat_uart_wifi, textBuf, strlen(textBuf));
	         sAPI_UartWrite(eat_uart_wifi, textBuf, strlen(textBuf));
			 sAPI_UartWrite(eat_uart_wifi, textBuf, strlen(textBuf));
			}
			else if(strstr(wifi_rx_buf,"AOK")!= 0)
			{ 
		       commandflag=0;
				switch(wifiState)
				{
					case INIT:
					while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
					InitState();
					//while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
					
					break;
					
					case INITTCP:
					while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
					InitTCPState();
					//while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
					
					break;
					case INITSAP:
					while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
					InitSAPState();
					//while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
					
					break;
					case INITWPS:
					while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
					InitWPSState();
					//while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
					
					break;					
					case CLIENTOPENED:
					break;
					default:
					break;
				}
			}
		 	  
			else if(strstr(wifi_rx_buf,"disconnected")!=0)
			{
				
				wifiState = DISCONNECTED;
				wifiopenflag=0;
				commandflag=0;
				tcpopenflag=0;
				//sAPI_Debug("reced open\n");
			}
			else if(strstr(wifi_rx_buf,"EXIT")!=0)
			{
				commandflag=0;
				wifiopenflag=0;
			wifiState = CONFIG;
		    SCK_CON = 1;
			//WifiWriteCmd(CLIENTOPEN);
                  
			}
			else if((strstr(wifi_rx_buf,"*OPEN*")!=0)||(strstr(wifi_rx_buf,"*O")!=0))
			{
				//wifiopenflag=1;
				commandflag=0;
				if(s_nMSettings.m_Enter>=70 && wpsmodeon==0 && apmodeon==0 )
				    tcpopenflag=1;
				if(tcpopenflag==1)
				{
					wifiopenflag=1;
				//sAPI_TimerStop(OK_TIMER);
				//Send_Packet(0);
				}
				if(tcpopenflag==1)
				{
				if(WifiCmdState == CLIENTOPEN)
				{
					wifiState = CLIENTOPENED;
					//Send_Packet(0);
				}
				sAPI_Debug("reced open\n");
				}
				else
				wifiopenflag=1;	
			}
			else if((strstr(wifi_rx_buf,"*CLOS*")!=0)||(strstr(wifi_rx_buf,"close")!=0)||(strstr(wifi_rx_buf,"*CLOS")!=0))
			{
				wifiopenflag=0;
				commandflag=0;
				if(wifiState == CLIENTONCLOSE )
				{
				 // wifiState = CONFIG;
				 // SCK_CON = 1;
				  WifiWriteCmd(EXIT);
				// WifiWriteCmd(OPEN);	
				}
				/*if(tcpopenflag==1)
				{
				if(wifiState == CLIENTOPENED)
				{
				  //Send_Packet(1);
				  wifiState = CONFIG;
				  SCK_CON = 1;
				  WifiWriteCmd(CLIENTOPEN);
                  				  
				}
				else if(wifiState == CLIENTONCLOSE )
				{
				  wifiState = CONFIG;
				  SCK_CON = 1;
				  WifiWriteCmd(CLIENTOPEN);
					
				}
				else if(wifiState==CLIENTONOPEN )
				{
				//Send_Packet(1);
				  
				  WifiWriteCmd(EXIT);
				  
				  
				}
				}*/
				
			}
            else if(strstr(wifi_rx_buf,"ERR TCP Send")!=0)
			{
				wifiopenflag=0;
				commandflag=0;
			}			
			else if(strstr(wifi_rx_buf,"*READY*")!= 0 && WifiReady == EAT_FALSE)
			{
				sAPI_Debug("[%s]: Wifi is ready", __FUNCTION__);
				WifiReady = EAT_TRUE;
				//Wifi_Entry_CMD_Mode();
				//commandflag=1;
			}
			else if(strstr(wifi_rx_buf,"*READY*")!= 0 )
			{
				commandflag=0;
				tcpopenflag=0;   //dg_changed
				//Wifi_Entry_CMD_Mode();
				//commandflag=1;
				
			}
			/*else if(strstr(wifi_rx_buf,"DHCP")!= 0 )
			{		
			commandflag=0;
			wifiopenflag=0;
	        }*/
			else if(strstr(wifi_rx_buf,"DHCP")!= 0 )
			{
				wifimodesetfalg=0;
				wifiresetflag1=0;
				//wpsmodeon = 1;
		//tcpopenflag=0;
		//apmodeon = 0;
		
				if(s_nMSettings.m_Enter>=70 && wpsmodeon==0 && apmodeon==0 )
				{tcpopenflag=1;wifiresetflag=1;}
            if(strstr(wifi_rx_buf,"IPv4")!= 0 )	
            {				
			char StrTokStr[50][50];
	        int StrTokStrVer = 0;
	        char *Pch = NULL;
			char *Pch1 = NULL;
			unsigned char j;
			commandflag=0;
			/*k = wifi_rx_buf;
			len=strlen(wifi_rx_buf);
			for (i = 0; i <len; i++) 
			{
				if(*k=='\n')
				*k = ' '; 
				else if(*k=='\r')
				*k = ' ';
					k++;
			}*/
	        Pch = strtok((char *)wifi_rx_buf, (char *)" : "  );
		    StrTokStrVer = 0;
		    while( Pch != NULL )
		    {
		    strcpy(StrTokStr[StrTokStrVer],Pch);
		    StrTokStrVer++;
		    Pch = strtok( NULL, " '\n' '\r' " );
		    }
			for(Tp=0;Tp<=StrTokStrVer;Tp++)
			{
			//sAPI_Debug("\n pod1=%s\r",StrTokStr[Tp]);
			if(strstr(StrTokStr[Tp],"IPv4")!= 0 )
			{
			sAPI_Debug("\n IPv4=%s\r",StrTokStr[Tp]);
			j=Tp;
			break;
			}
			}
			Pch1 = strtok((char *)StrTokStr[j], (char *)"  = : " );
			
			//sAPI_Debug("\n pod1=%s\r",StrTokStr[j]);
		
		    StrTokStrVer = 0;
			
		    while( Pch1 != NULL )
		    {
		    strcpy(StrTokStr[StrTokStrVer],Pch1);
		    StrTokStrVer++;
		    Pch1 = strtok( NULL, " =  : " );
		    }
		    
		    sAPI_Debug("\n pod1=%s\r",StrTokStr[1]);
    
			 memset(DeviceConfig.ApServerIP, 0x00, sizeof(DeviceConfig.ApServerIP));
		     strcpy(DeviceConfig.ApServerIP,StrTokStr[1]);
		     app_nvram_save(MCONFIG_AT_INDEX, (UINT8*)&DeviceConfig, sizeof(DeviceConfig));
			 if(wifiState==INITTCP||wifiState==INIT)
				tcpopenflag=1; 
			 wifiState = CONFIG;
		     SCK_CON = 1;
			 //Wifi_Entry_CMD_Mode();
				  //sid=0;
		     //WifiWriteCmd(CLIENTOPEN);
			}
			 wifiopenflag=0;
			 commandflag=0;
			//wifiopenflag=0;       
				
			}			
			else if(strstr(wifi_rx_buf,"CMD")!= 0)
			{
				commandflag=0;
				 sAPI_Debug("[%s]: Cmd mode is ready", __FUNCTION__);
				 if(WifiCmdState==CMD && wifiState==INIT)
					WifiWriteCmd(SETIPDHCP);
				 else if(WifiCmdState==CMD && wifiState==INITTCP)
				    WifiWriteCmd(SETWLANJOINONE); 
				else if(WifiCmdState==CMD && wifiState==INITSAP)
					WifiWriteCmd(SETAPMODECHANNELSIX);
				else if(WifiCmdState==CMD && wifiState==INITWPS)
					WifiWriteCmd(SETWPSMODECHANNELONE);
				//else if(WifiCmdState==CMD && wifiState==SETAPMODE)
				//	WifiWriteCmd(JOINAPMODE);				
				else if(WifiCmdState==CLIENTCMD && wifiState == CONFIG)
				{
					wifiState=CLIENTONOPEN;
				    WifiWriteCmd(CLIENTOPEN);
					
				}
				else if (wifiState==CLIENTONCLOSE)
				{
					WifiWriteCmd(CLIENTCLOSE);
					//WifiWriteCmd(EXIT);
				}
			}
			else if(strstr(wifi_rx_buf,"html")!= 0)
			{
			livedataflag=0;	
			}
			else if(strstr(wifi_rx_buf,"#live")!= 0)
		{
		livedataflag=1;
		errorcount=0;
		}
			else if((strstr(wifi_rx_buf,"HTTP")!= 0)||(strstr(wifi_rx_buf,"sentSms")!= 0))
			{
			
			char StrTokStr[100][50];    // [500][50]
	        int StrTokStrVer = 0;
	        char *Pch = NULL;
			char buff[200];
			Nooftcpsendcount=0;

			if((strstr(wifi_rx_buf,"Successfully")!= 0)||(strstr(wifi_rx_buf,"Already Inserted")!= 0))
				{
					if(livedataflag==0 && livedataflag1==0&& Nooftcprecvd1==0 && Http_send_flag == 1 )
					{
		             Nooftcpprocessed++;
					 Http_send_flag=0;
					}
		
				//Nooftcpprocessed++;
				errorcount=0;
				livedataflag=0;
				}
				else if(strstr(wifi_rx_buf,"#live")!= 0)
		{
		livedataflag=1;
		errorcount=0;
		}
				else if(strstr(wifi_rx_buf,"Error")!= 0)
				{
				if((Nooftcprecvd1==0)&& (Http_send_flag == 1 ))
				{
				Http_send_flag=0;
				Nooftcpprocessed++;
				}
				errorcount=0;
				}
			    
			    else if(strstr(wifi_rx_buf,"400 Bad")!= 0)
				{
				//if(errorcount++>3)
				//{
			    if(( Nooftcprecvd1==0)&& (Http_send_flag == 1 ))
				{
				Http_send_flag=0;
				Nooftcpprocessed++;
				//errorcount=0;
				//}
				}
				}
				else if(strstr(wifi_rx_buf,"405")!= 0)
				{
				if(errorcount++>3)
				{
				if(( Nooftcprecvd1==0)&& (Http_send_flag == 1 ))
				{
				Http_send_flag=0;
				Nooftcpprocessed++;
				}
				errorcount=0;
				}
				}
				else if(strstr(wifi_rx_buf,"Already")!= 0)
				{
				if(errorcount++>3)
				{
				if(( Nooftcprecvd1==0)&& (Http_send_flag == 1 ))
				{
				Http_send_flag=0;
				Nooftcpprocessed++;
				}
				errorcount=0;
				livedataflag=0;
				}
				}
				else if((strstr(wifi_rx_buf,"Setting")!= 0))
			    {
					livedataflag=0;
			    }
			     else
				 {
			commandflag=0;
	        Pch = strtok((char *)wifi_rx_buf, (char *)"{}" );
		    StrTokStrVer = 0;
		    while( Pch != NULL )
		    {
		    strcpy(StrTokStr[StrTokStrVer],Pch);
		//	sAPI_Debug("");
		    StrTokStrVer++;
		    Pch = strtok( NULL,"{ }" );
		    }
			sAPI_Debug("\n\rSt=%d \n\r",StrTokStrVer);
			//for(Tp=0;Tp<=StrTokStrVer;Tp++)
			//{
				sAPI_Debug("\n\rSt=%s\n\r",StrTokStr[1]);
				
			//}
			//memset(buff,0,200);
			//sAPI_Debug("\n\rSt=%s \n\r",StrTokStr[2]);
			strcpy(buff,StrTokStr[1]);
			sAPI_Debug("\n\rbuff=%s \n\r",buff);
			Pch = strtok((char *)buff, (char *)"\"\"" );
		    StrTokStrVer = 0;
		    while( Pch != NULL )
		    {
		    strcpy(StrTokStr[StrTokStrVer],Pch);
		//	sAPI_Debug("StrTokStr[StrTokStrVer]=%s StrTokStrVer = %d",StrTokStr[StrTokStrVer],StrTokStrVer);

		    StrTokStrVer++;
		    Pch = strtok( NULL,"\"\"" );
		    }
			for(Tp=0;Tp<=StrTokStrVer;Tp++)
			{
				sAPI_Debug("\n\rSt=%s  pos=%d\n\r",StrTokStr[Tp],Tp);
				
			}
			
		   // sAPI_Debug("\n pod1=%s\r",StrTokStr[2]);
    
			strcpy((char *)wifiStrNumber[Noofsettingsrecvd].Wifistr,(char*)StrTokStr[2]);
			Noofsettingsrecvd++;
			if(Noofsettingsrecvd>200)
			Noofsettingsrecvd=0;
				 }
			}
		    
			else
			{
			//if((strstr(wifi_rx_buf,"1")!= 0)||(strstr(wifi_rx_buf,"2")!= 0))
			//{}
		   // else 
			if((strstr(wifi_rx_buf,"Setting")!= 0))
			{
				livedataflag=0;
			}
		    if(strstr(wifi_rx_buf,"#live")!= 0)
		{
		livedataflag=1;
		//errorcount=0;
		}
		    else if((strstr(wifi_rx_buf,"Err! Too many chars")!= 0)||(strstr(wifi_rx_buf,"ERR TCP Send Buf Full")!= 0))
			{
		    wifiresetflag=1;
		    wifiresetcounter=0;
		
			}
			
			else if((strlen(wifi_rx_buf)<=160&&tcpopenflag==0))
			{				
			strcpy((char *)wifiStrNumber[Noofsettingsrecvd].Wifistr,(char*)wifi_rx_buf);
			Noofsettingsrecvd++;
			if(Noofsettingsrecvd>200)
			Noofsettingsrecvd=0;
			}
			}	
		}
	}
	else
	{
		// if not wifi uart
	}
}
#endif

//UINT8 SCK_buf[500]={0};

#if 0
void ServerTask(void *data)
{
	UINT8 numb =0;
	eat_bool ret = EAT_FALSE;
	UINT16 ACT_MODE_TIME_OUT=0;
	s32 Resp = 0;
    UINT8 cmd_idx=0;
	s8 sid=-1;
	UINT8 SCK_buf[50]={0};
	//UINT8 SCK_RFDbuf[500]="";
	UINT8 user_msg[210]={0};
	//UINT8 user_msg1[40]={0};
    //UINT8 user_msg1flag=0;
    EatEvent_st event;
	SCK_CLOSE=1;
	eat_soc_notify_register(soc_notify_cb); //register socket callback
	//sAPI_TimerStart(SOCKET_TIMER, DeviceConfig.Server_Interval_Amode); 
	sAPI_TimerStart(timerRef,20000,RESPONSE_TIMER_PERIOD,*callBackRoutine,SOCKET_TIMER); 
	sAPI_TimerStart(timerRef,SOCKET_TIC_PERIOD,RESPONSE_TIMER_PERIOD,*callBackRoutine,SOCKET_TIC); 
	//sAPI_TimerStart(REC_TIMER, REC_TIMER_PERIOD); 
	
    while(EAT_TRUE)
    {
        eat_get_event_for_user(EAT_USER_2, &event);
        switch(event.event)
        {			
			case EAT_EVENT_USER_MSG:
				sprintf(user_msg,"%s\0",event.data.user_msg.data);  
				sAPI_Debug("ServerTask Msg: %s", user_msg);
				if(!strncmp(user_msg,"FW$",3))
				{
					eat_uart_close(eat_uart_app );
					eat_uart_close(eat_uart_wifi );
					sAPI_TimerStop(SOCKET_TIMER);
					sAPI_TimerStop(SOCKET_TIC);
					simcom_fota_update(DeviceConfig.ftpUserName, DeviceConfig.ftpPassword, "WMS",DeviceConfig.firmwareFolderName, DeviceConfig.FtpServerIP, 21);
				}
			//	else if(!strncmp(user_msg,"APP",3)){app_update("c:\\User\\Ftp\\app.bin");}
				else if(!strncmp(user_msg,"CCK",3))
				{	
					sAPI_Debug("connect_config %d",csid); 
				}
				else if(!strncmp(user_msg,"CONRBT",6))
				{	
			     commandflag=1;
	             WifiWriteCmd(REBOOT);
                 }
				if(!strncmp(user_msg,"$BearerDeact",12))
				{
					Sapbr = EAT_FALSE;
					if(DeviceConfig.interface==GPRS)
						Bearer_enable(BearerCallback);
				}
				else if(!strncmp(user_msg,"$HttpInit",9))
				{
					//eat_sleep(5000);
					HttpInit(HttpInitCallback);
				}
				else if(!strncmp(user_msg,"$HttpRead",9))
				{
					sAPI_Debug("Data on Read");
				}
				else if(!strncmp(user_msg,"CONTCP",6))
					
				{
					wifimodesetfalg=1;
					 sAPI_Debug("/n/r************SWITCHING TCP*****************/n/r");
					 //wifiresetflag=1;
		             //wifiresetcounter=0;
		              wifiopenflag=0;
					 wifiState=INITTCP;
					 //wifiState=INIT;
					 //tcpopenflag=1;
					 if(commandflag==0 )
				     Wifi_Entry_CMD_Mode();
				     else
					 WifiWriteCmd(SETWLANJOINONE);

				}
				else if(!strncmp(user_msg,"SENDTCP",7))
				{
				sAPI_Debug("/n/r************SEND TCP*****************/n/r");
					 
				
				strcpy(SendSMSString,TCPwifiStrNumber[Nooftcpprocessed].TCPWifistr);
				sAPI_UartWrite(eat_uart_wifi, SendSMSString,strlen(SendSMSString));	
				}
							
				else if(!strncmp(user_msg,"CLOSE",5))
				{
					if(s_nMSettings.m_Enter>70)
					{
					 sAPI_Debug("/n/r************SWITCHING TCP*****************/n/r");
					 //wifiState=INITTCP;
					 //tcpopenflag=1;
					// if(commandflag==0)'
					if(tcpopenflag==1)
					{
						wifiopenflag=0;
					wifiState = CLIENTONCLOSE;
				    Wifi_Entry_Client_Mode();
					}
				    // else
					// WifiWriteCmd(CLIENTOPEN);
					}

				}
				else if(!strncmp(user_msg,"CONFIG",6))
				{
					if(s_nMSettings.m_Enter>70)
					{
					 sAPI_Debug("/n/r************SWITCHING TCP*****************/n/r");
					 //wifiState=INITTCP;
					 //tcpopenflag=1;
					// if(commandflag==0)'
					if(tcpopenflag==1)
					{
						wifiState = CONFIG;
				    Wifi_Entry_Client_Mode();
					}
				    // else
					// WifiWriteCmd(CLIENTOPEN);
					}
				}
				else if(!strncmp(user_msg,"CONGPRS",7))
				{
					if(DeviceConfig.interface == GPRS && Sapbr == EAT_FALSE)
						Bearer_enable(BearerCallback);
				}				
				else if(!strncmp(user_msg,"CONSAP",6))
				{
					wifimodesetfalg=1;
					 sAPI_Debug("/n/r************SWITCHING TCP*****************/n/r");
					 wifiState=INITSAP;
					 tcpopenflag=0;
				     if(commandflag==0 )
				     Wifi_Entry_CMD_Mode();
				     else
					 WifiWriteCmd(SETAPMODECHANNELSIX);
			

				}
				else if(!strncmp(user_msg,"CONWPS",6))
				{
					wifimodesetfalg=1;
					 sAPI_Debug("/n/r************SWITCHING TCP*****************/n/r");
					 wifiState=INITWPS;
					 tcpopenflag=0;
				     if(commandflag==0)
				     Wifi_Entry_CMD_Mode();
				     else
					 WifiWriteCmd(SETWPSMODECHANNELONE);
			

				}
				else if(!strncmp(user_msg,"CONINIT",7))
				{
					wifimodesetfalg=1;
					 sAPI_Debug("/n/r************SWITCHING TCP*****************/n/r");
					 wifiState=INIT;
					 //tcpopenflag=0;
				     if(commandflag==0||tcpopenflag==0)
				     Wifi_Entry_CMD_Mode();
				     else
					 WifiWriteCmd(SETIPDHCP);
			         //tcpopenflag=1;

				}
                else if(!strncmp(user_msg,"FBKD",4))
				{
					//WifiReady == EAT_FALSE;
					//if (wifiState==CLIENTONCLOSE) 
					//sAPI_UartWrite(eat_uart_wifi, FBKBuf, strlen(FBKBuf));
			      sAPI_Debug("/n/r************SENDING UART DATA*****************/n/r");

				}
                else if(!strncmp(user_msg,"SGET",4))
				{
					//if(Egprs && CGATT && s_nMSettings.m_Enter>=70 ) 
                    //if((Egprs && CGATT && DeviceConfig.interface == GPRS)&& s_nMSettings.m_Enter>=70)  
						 if(( CGATT && DeviceConfig.interface == GPRS)&& s_nMSettings.m_Enter>=70 && livedataflag==0 && livedataflag1==0)
						{ 
					        //sid=0;
							//sAPI_TimerStop(SOCKET_TIMER);
						#ifdef GPRS_TCP_DISABLE
						
						if(CGATT && s_nMSettings.m_Enter>=70)
						{
							if(HttpInitStatus)
							{ 
								sAPI_TimerStop(SOCKET_TIMER);
								if(!HttpInPrograssState)
								{
									sAPI_Debug("Http GET");
									//memset(&PostData,0,500);
									//HttpInPrograssfailcount=0;
									//httpinitfailcount=0;
									//sprintf(PostData,"{\r\n\"cC\":\"%s\",\r\n\"cM\":\"hello2\",\r\n\"cL\":\"hai\",\r\n\"cZ\":\"%s\",\r\n\"cD\":\"%d/%d/20%02d\",\r\n\"cT\":\"%d:%d:%d\",\r\n\"mC\":\"SMS\"\r\n}",IMEI,VAL_BUF,datetime.day,datetime.mon,datetime.year,datetime.hour,datetime.min,datetime.sec);
								
									//strcpy(PostData,TCPwifiStrNumber[Nooftcpprocessed].TCPWifigprsstr);
									//sprintf(PostData,"{\r\n\"ctrlQrcode\" : \"3456789023456\",\r\n\"ctrlMsg\": \"Hello\",\r\n\"ctrlDate\": \"24/04/2017\",\r\n\"ctrlTime\":\"00:03:31\"\r\n}");
									//sprintf(PostData,"%s",TCPwifiStrNumber[Nooftcpprocessed].TCPWifigprsstr);
									//sAPI_Debug("\n\r%d-%d\n\r",strlen(PostData),strlen(TCPwifiStrNumber[Nooftcpprocessed].TCPWifigprsstr));
									if(livedataflag ==0 && livedataflag1==0)
									{
									tcpdcounter3++;
									sAPI_Debug("\n\rclearcounter=%d",tcpdcounter3);
									if(tcpdcounter3<=100)
									{
										tcpdcounter=0;
									HttpInPrograssfailcount=0;
									httpinitfailcount=0;
									HttpGet(); // get
									// post
									}
									else
									{
									tcpdcounter3=0;
									httpinitfailcount=3;
									eat_send_msg_to_user(EAT_USER_2, EAT_USER_1, EAT_FALSE, 5, "CLEAR", EAT_NULL);
									}
									}
									sAPI_TimerStart(timerRef,20000,RESPONSE_TIMER_PERIOD,*callBackRoutine,SOCKET_TIMER); 
									//free(PostData);
									//HttpInPrograssState=EAT_TRUE;
									sAPI_Debug("HttpInPrograssStateEAT_TRUE;");
								}
								else
								{
									//sAPI_TimerStart(SOCKET_TIMER, 20000);
									sAPI_Debug("HttpInPrograssfailcount=%d",HttpInPrograssfailcount);
									if(HttpInPrograssfailcount++>=4)
									{
										sAPI_Debug("Http POST1 - waiting to complete");
									HttpInPrograssfailcount=0;
	                                //HttpInPrograssState = EAT_FALSE;
				                    eat_send_msg_to_user(EAT_USER_2, EAT_USER_1, EAT_FALSE, 8, "FUNCLEAR", EAT_NULL);
									}

									sAPI_Debug("Http POST - waiting to complete");
									sAPI_TimerStart(timerRef,20000,RESPONSE_TIMER_PERIOD,*callBackRoutine,SOCKET_TIMER); 
								}
							}
							else
							{
							sAPI_Debug("HttpInitStatus false=%d",httpinitfailcount);
							if(httpinitfailcount++>3)
							{
							 //httpinitfailcount=0;
                             //if(Sapbr)
							// HttpInit(HttpInitCallback); //vaz
						    /*if(Sapbr)
								{
									Sapbr = EAT_FALSE;
								Bearer_enable(BearerCallback);
								}*/
							   if(httpinitfailcount>3&&httpinitfailcount<5)
							   {
                               eat_send_msg_to_user(EAT_USER_2, EAT_USER_1, EAT_FALSE, 8, "FUNCLEAR", EAT_NULL);
							   }
						       else
							   {
							   httpinitfailcount=0;
							   Sapbr = EAT_FALSE;
					//if(DeviceConfig.interface==GPRS)
						       Bearer_enable(BearerCallback);
							   }

																	
							}
								//else
								//Bearer_enable(BearerCallback);
								sAPI_TimerStart(timerRef,20000,RESPONSE_TIMER_PERIOD,*callBackRoutine,SOCKET_TIMER); 
					            break;
														
							//HttpInit(HttpInitCallback);
							//sAPI_TimerStart(SOCKET_TIMER, 12000);
							}
							//sAPI_TimerStart(SOCKET_TIMER, 10000);
						}
						#else
							if(SCK_CON==1 && sid>=0 &&ftpgettofs_cb_flag==0) 
							{
								//strcpy(SendSMSString,TCPwifiStrNumber[Nooftcpprocessed].TCPWifistr);

								//Send_Packet(sid);
							}
						#endif
						}	
				}						
				else if(!strncmp(user_msg,"SMST",4))
				{
					//if(Egprs && CGATT && s_nMSettings.m_Enter>=70 ) 
                    //if((Egprs && CGATT && DeviceConfig.interface == GPRS)&& s_nMSettings.m_Enter>=70)  
						 if(( CGATT && DeviceConfig.interface == GPRS)&& s_nMSettings.m_Enter>=70)
						{ 
					        //sid=0;
							//sAPI_TimerStop(SOCKET_TIMER);
						#ifdef GPRS_TCP_DISABLE
						
						if(CGATT && s_nMSettings.m_Enter>=70)
						{
							if(HttpInitStatus)
							{ 
								sAPI_TimerStop(SOCKET_TIMER);
								if(!HttpInPrograssState)
								{
									sAPI_Debug("Http Post");
									memset(&PostData,0,500);
									HttpInPrograssfailcount=0;
									httpinitfailcount=0;
									//sprintf(PostData,"{\r\n\"cC\":\"%s\",\r\n\"cM\":\"hello2\",\r\n\"cL\":\"hai\",\r\n\"cZ\":\"%s\",\r\n\"cD\":\"%d/%d/20%02d\",\r\n\"cT\":\"%d:%d:%d\",\r\n\"mC\":\"SMS\"\r\n}",IMEI,VAL_BUF,datetime.day,datetime.mon,datetime.year,datetime.hour,datetime.min,datetime.sec);
								
									//strcpy(PostData,TCPwifiStrNumber[Nooftcpprocessed].TCPWifigprsstr);
									//sprintf(PostData,"{\r\n\"ctrlQrcode\" : \"3456789023456\",\r\n\"ctrlMsg\": \"Hello\",\r\n\"ctrlDate\": \"24/04/2017\",\r\n\"ctrlTime\":\"00:03:31\"\r\n}");
									//sprintf(PostData,"%s",TCPwifiStrNumber[Nooftcpprocessed].TCPWifigprsstr);
									//sAPI_Debug("\n\r%d-%d\n\r",strlen(PostData),strlen(TCPwifiStrNumber[Nooftcpprocessed].TCPWifigprsstr));
									if(livedataflag ==0 && livedataflag1==0)
									{
										
									sprintf(PostData,"%s",TCPwifiStrNumber[Nooftcpprocessed].TCPWifigprsstr);
							    	sAPI_Sprintf(buf,"\n\rPOST LENGTH =%d-%d\n\r",strlen(PostData),strlen(TCPwifiStrNumber[Nooftcpprocessed].TCPWifigprsstr));
									sAPI_UartPrintf(buf);
								//	sAPI_Debug("\n\rPOST data =%s\n\r",PostData);
									MQTTpublish(strlen(PostData),PostData); // post   dg_blocked
									
									}
								    else
									{
									sprintf(PostData,"%s",TCPWifigprsstrBUFF);
									sAPI_Sprintf(buf,"\n\rPOST LENGTH =%d-%d\n\r",strlen(PostData),strlen(TCPWifigprsstrBUFF));
									sAPI_UartPrintf(buf);
									if(strlen(PostData)>10)
									MQTTpublish(strlen(PostData),PostData); // post   //dg_blocked
									
									} 
									sAPI_TimerStart(timerRef,20000,RESPONSE_TIMER_PERIOD,*callBackRoutine,SOCKET_TIMER); 
									//free(PostData);
									//HttpInPrograssState=EAT_TRUE;
									sAPI_Debug("HttpInPrograssStateEAT_TRUE;");
								}
								else
								{
									//sAPI_TimerStart(SOCKET_TIMER, 20000);
									sAPI_Debug("HttpInPrograssfailcount=%d",HttpInPrograssfailcount);
									if(HttpInPrograssfailcount++>=4)
									{
										sAPI_Debug("Http get1 - waiting to complete");
									HttpInPrograssfailcount=0;
	                                //HttpInPrograssState = EAT_FALSE;
				                    eat_send_msg_to_user(EAT_USER_2, EAT_USER_1, EAT_FALSE, 8, "FUNCLEAR", EAT_NULL);
									}

									sAPI_Debug("Http get - waiting to complete");
									sAPI_TimerStart(timerRef,20000,RESPONSE_TIMER_PERIOD,*callBackRoutine,SOCKET_TIMER); 
								}
							}
							else
							{
							sAPI_Debug("HttpInitStatus false=%d",httpinitfailcount);
							if(httpinitfailcount++>3)
							{
							 //httpinitfailcount=0;
                             //if(Sapbr)
							// HttpInit(HttpInitCallback); //vaz
						    /*if(Sapbr)
								{
									Sapbr = EAT_FALSE;
								Bearer_enable(BearerCallback);
								}*/
							   if(httpinitfailcount>3&&httpinitfailcount<5)
							   {
                               eat_send_msg_to_user(EAT_USER_2, EAT_USER_1, EAT_FALSE, 8, "FUNCLEAR", EAT_NULL);
							   }
						       else
							   {
							   httpinitfailcount=0;
							   Sapbr = EAT_FALSE;
					//if(DeviceConfig.interface==GPRS)
						       Bearer_enable(BearerCallback);
							   }

																	
							}
								//else
								//Bearer_enable(BearerCallback);
								sAPI_TimerStart(timerRef,20000,RESPONSE_TIMER_PERIOD,*callBackRoutine,SOCKET_TIMER); 
					            break;
														
							//HttpInit(HttpInitCallback);
							//sAPI_TimerStart(SOCKET_TIMER, 12000);
							}
							//sAPI_TimerStart(SOCKET_TIMER, 10000);
						}
						#else
							if(SCK_CON==1 && sid>=0 &&ftpgettofs_cb_flag==0) 
							{
								//strcpy(SendSMSString,TCPwifiStrNumber[Nooftcpprocessed].TCPWifistr);

								//Send_Packet(sid);
							}
						#endif
						}
						else if(DeviceConfig.interface == WIFI && s_nMSettings.m_Enter>=70)
						{
							sid=0;
							// call client
							//if(wifiState != OPENED)
							//if(WifiCmdState==CLIENTOPEN && wifiState == CONFIG)
							//if(wifiopenflag==0 && commandflag==0)
							//Wifi_Entry_Client_Mode();
                            //else if (wifiState==CLIENTONCLOSE) 
							//Send_Packet(0);	
						   // else if (wifiState==DISCONNECTED) 
							//Send_Packet(1);	
							
							sAPI_Debug("connect wifi client mode\n");
						}
																
				}
				break;
			case EAT_EVENT_TIMER:
				switch(event.data.timer.timer_id) 
				{
					case SOCKET_TIC:					
						//if(Egprs==1 && CGATT==1&& s_nMSettings.m_Enter>=70)  // GPRS connected
						if((Egprs && CGATT && DeviceConfig.interface == GPRS) && s_nMSettings.m_Enter>=70 && ringflag==0)
						{
							if(SCK_CLOSE==1)
							{
								switch(DeviceConfig.interface)
								{
									case GPRS:
										g_server_address.port=DeviceConfig.SocketPort;
										sid = simcom_connect_server(&g_server_address); 
										//sid1=sid;
										sAPI_Debug("connect_server: %d,port: %d",sid,g_server_address.port);
										if(sid < 0) // Server Error
										{
										   sAPI_Debug("connect_server return error :%d",sid);	
										}	
									break;
									case WIFI:
									/*	sid=0;
										if(wifiState == INIT && WifiReady == EAT_FALSE) // first time boot
										{
											Wifi_Entry_CMD_Mode();
											sAPI_Debug("Initilaze wifi\n");
											
										}*/
										//if(wifiopenflag==0 && commandflag==0)
							           // Wifi_Entry_Client_Mode();
                            
									break;
									default:									
									break;
								}		
								SCK_CLOSE=0;
								sAPI_TimerStart(timerRef,1000,RESPONSE_TIMER_PERIOD,*callBackRoutine,RESPONSE_TIMER); 
							}
						}
						else
						{
						if(s_nMSettings.m_Enter>=70)
						sAPI_Debug("TIC: EGPRS %d,CGATT %d, SCK_CLOSE %d",Egprs,CGATT,SCK_CLOSE);
						}
						sAPI_TimerStart(timerRef,1000,SOCKET_TIC_PERIOD,*callBackRoutine,SOCKET_TIC); 
						break;
					case SOCKET_TIMER:// Sending time interval 10sec
						
					        //sid=0;
						    //sAPI_Debug("Http socket entry");	
						#ifdef GPRS_TCP_DISABLE
						//if((HttpInitStatus && CGATT && DeviceConfig.interface == GPRS)&& s_nMSettings.m_Enter>=70)
						if(CGATT &&  DeviceConfig.interface == GPRS && s_nMSettings.m_Enter>=70 && CallConnected==0 && ringflag==0)
						{
						if(MqttInitStatus)
							{
								
								sAPI_Debug("Http Get");								
								if(!MqttInPrograssState)
								{
								   
									if(sgetflag==1)
									{
									sAPI_Debug("Http Post");
									memset(&PostData,0,500);
									HttpInPrograssfailcount=0;
									httpinitfailcount=0;
									if(livedataflag ==0 && livedataflag1==0)
									{
									if(Nooftcprecvd1>Nooftcpprocessed1)
									{
									Http_send_flag=0;
									sprintf(PostData,"%s",TCPwifiStrNumber1[Nooftcpprocessed1].TCPWifigprsstr1);
									Nooftcpprocessed1++;
									}
									else
									{
									Http_send_flag=1;
									sprintf(PostData,"%s",TCPwifiStrNumber[Nooftcpprocessed].TCPWifigprsstr);
									Nooftcprecvd1=Nooftcpprocessed1=0;
									}
							    	sAPI_Sprintf(buf,"\n\rPOST LENGTH =%d-%d\n\r",strlen(PostData),strlen(TCPwifiStrNumber[Nooftcpprocessed].TCPWifigprsstr));
									sAPI_UartPrintf(buf);
								//	sAPI_Debug("\n\rPOST data =%s\n\r",PostData);
									MQTTpublish(strlen(PostData),PostData); // post
									}
								    else
									{
									Http_send_flag=0;
									sprintf(PostData,"%s",TCPWifigprsstrBUFF);
									sAPI_Sprintf(buf,"\n\rPOST LENGTH =%d-%d\n\r",strlen(PostData),strlen(TCPWifigprsstrBUFF));
									sAPI_UartPrintf(buf);
									if(strlen(PostData)>10)
									MQTTpublish(strlen(PostData),PostData); // post  //dg_blocked
									
									
									} 
									sgetflag=0;
									}
									else if(sgetflag==2&& gprsgeton==1)
									{
									tcpdcounter=0;
									HttpInPrograssfailcount=0;
									httpinitfailcount=0;
									HttpGet(); // get
									sgetflag=0;
									}
									else
									{
										sgetflag=0;
									//	sAPI_Debug("\n\r no post or get ");
									}
									
								}
								else
								{
									sAPI_TimerStart(timerRef,1000,20000,*callBackRoutine,SOCKET_TIMER); 
									if(HttpInPrograssfailcount++>=4)
									{
									HttpInPrograssfailcount=0;
	                                //HttpInPrograssState = EAT_FALSE;
				                    eat_send_msg_to_user(EAT_USER_2, EAT_USER_1, EAT_FALSE, 8, "FUNCLEAR", EAT_NULL);
									}
									sAPI_Debug("Http Post - waiting to complete");
								}
									
							}
							else
							{
								sAPI_Debug("Http Get - HttpInit false=%d",httpinitfailcount); // No ip
								if(httpinitfailcount++>10)
								{
									//httpinitfailcount=0;
							//	if(Sapbr)
							// HttpInit(HttpInitCallback); //vaz
						    /*if(Sapbr)
								{
									Sapbr = EAT_FALSE;
								Bearer_enable(BearerCallback);*/
								 if(httpinitfailcount>10&&httpinitfailcount<12)		
								 {
                               //eat_send_msg_to_user(EAT_USER_2, EAT_USER_1, EAT_FALSE, 8, "FUNCLEAR", EAT_NULL);
								 }
						       else
							   {
							   httpinitfailcount=0;
							   Sapbr = EAT_FALSE;
					//if(DeviceConfig.interface==GPRS)
						       Bearer_enable(BearerCallback);
							   }
	
								}	
							   sAPI_TimerStart(timerRef,1000,10000,*callBackRoutine,SOCKET_TIMER); 
					            break;
								
							}
						}
						#else
						if((Egprs && CGATT && DeviceConfig.interface == GPRS)&& s_nMSettings.m_Enter>=70)  
						{ 
							if(SCK_CON==1 && sid>=0 &&ftpgettofs_cb_flag==0) 
							{
								if(Nooftcpprocessed !=Nooftcprecvd && getdataflag==0)
								//Send_Packet(sid);
							}
								
						}
						#endif
						else if(DeviceConfig.interface == WIFI && s_nMSettings.m_Enter>=70)
						{
							sid=0;
						/*	if(tcpopenflag==1 && wifiopenflag==1 && getdataflag==0 )
							{
								unsigned char buffer_get_request[] = {"GET /api/v1/controller/88899923/message/ HTTP/1.1\r\nHost: 35.165.15.51:8080\r\nUser-Agent: curl/7.52.1\r\nAccept: application/json\r\nContent-Type: application/json\r\n\r\n"};
								sAPI_UartWrite(eat_uart_wifi, buffer_get_request,strlen(buffer_get_request));							
							}*/
							// call client
							//if(wifiState != OPENED)
							//if(wifiopenflag==0 && commandflag==0)
							//Wifi_Entry_Client_Mode();
                            
							//Wifi_Entry_Client_Mode();
						    //else if (wifiState==CLIENTONCLOSE) 
							//Send_Packet(0);	
						    //else if (wifiState==DISCONNECTED) 
							//Send_Packet(1);							
							  sAPI_Debug("connect wifi client mode\n");
							}
						//sAPI_TimerStart(SOCKET_TIMER, DeviceConfig.Server_Interval_Amode);/6000
						sAPI_TimerStart(timerRef,1000,20000,*callBackRoutine,SOCKET_TIMER); 
					break;
					case OK_TIMER: // timout without response 
						//FlashWrite(SCK_buf);
						eat_soc_close(sid);
						sid=-1;
						SCK_CLOSE=1;	
						SCK_CON=0;		
						sAPI_Debug("OK TIMEOUT");						
					break;
					case RESPONSE_TIMER:	//  timeout to open socket/connect server	
						eat_soc_close(sid);
						sid=-1;
						SCK_CLOSE=1;	
						SCK_CON=0;							
						sAPI_Debug("RSP TIMEOUT");	
						break;
					case BLOCKOUT_Response_TIMER:
							eat_soc_close(sid);
							sid=-1;							
							sAPI_Debug("BL TIMEOUT");
							//DeviceFlash.CurrentRAddr = PrevCurrentAddr;
						break;
					case REC_TIMER:
					/*
						if(((Egprs==1 && CGATT==1 && DeviceConfig.interface == GPRS)&& s_nMSettings.m_Enter>=70)|| (DeviceConfig.interface == WIFI && s_nMSettings.m_Enter>=70))
						{
							PrevCurrentAddr = DeviceFlash.CurrentRAddr;
							if( DeviceFlash.CurrentWAddr != DeviceFlash.CurrentRAddr)
							{
								UINT8 Block_send[1000]={0},eos=0;		 //1024
								UINT16 i=1,iT=0,tpm=0;
								eat_bool roll_back=EAT_FALSE;
								UINT8* ppp = NULL;
								DevicePrevRAddr = DeviceFlash.CurrentRAddr;
								ppp = (UINT8 *) DeviceFlash.CurrentRAddr;							
								sAPI_Debug("Start PData: %c, Addr 0x%x\r",*ppp,ppp);		
								//LedData_rate = EAT_TRUE;
								while(i<1000)
								{
									ppp++;
									i++;
									if(*ppp == '}') 
									{eos++;iT=i;}
									else if(*ppp == 255)
									{i=iT;break;}
									if(eos > 10)
										break;
									if((UINT32) ppp == DeviceFlashEndAddr || (UINT32) ppp == DeviceFlashBank0EndAddr)
									{
										if(*ppp != '}')
											roll_back=EAT_TRUE;
										break;
									}
									sAPI_Debug("%c",*ppp);
								}
								sAPI_Debug("End PData: %c, PAddr 0x%x , i= %d\r",*ppp, ppp,i);
								strncat(Block_send,(UINT8 *)DeviceFlash.CurrentRAddr,i);
								DeviceFlash.CurrentRAddr =  DeviceFlash.CurrentRAddr + i; 
								if(DeviceFlash.CurrentRAddr ==  DeviceFlashEndAddr)
									DeviceFlash.CurrentRAddr = DeviceFlashStartAddr;	
								else if(DeviceFlash.CurrentRAddr == DeviceFlashBank0EndAddr)
									DeviceFlash.CurrentRAddr = DeviceFlashBank1StartAddr; 
								if(roll_back)
								{
									i=0;
									ppp = (UINT8 *) DeviceFlash.CurrentRAddr;
									while(*ppp != ';')
									{
										ppp++;
										i++;
									}
									sAPI_Debug("Roll Back PData: %c, PAddr 0x%x , i= %d\r",*ppp, ppp,i);
									strncat(Block_send,(UINT8 *)DeviceFlash.CurrentRAddr,i);
								}
								if(DeviceConfig.interface == WIFI)				
								{	
                                    if (wifiState==CLIENTONCLOSE)
									{										
									sAPI_UartWrite(eat_uart_wifi, Block_send, strlen(Block_send));
									Resp=0;
									}
								    else
									Resp=-1;
								}
								else
								{
									Resp= simcom_send_to_server(sid,&Block_send, strlen(Block_send));  
								}					
								if(Resp<0)
								{
								DeviceFlash.CurrentRAddr=PrevCurrentAddr;	
									//FindNextPkt();
								//	LedData_rate = EAT_FALSE;
								//	app_nvram_save(MCONFIG_FLASH, (UINT8*)&DeviceFlash, sizeof(DeviceFlash));
								}
								sAPI_TimerStart(REC_TIMER, REC_TIMER_PERIOD);
								sAPI_TimerStart(BLOCKOUT_Response_TIMER,BLOCKOUT_Response_TIMER_PERIOD);
							}	
							else 
								sAPI_TimerStart(REC_TIMER, REC_TIMER_PERIOD);
						}
						else
							sAPI_TimerStart(REC_TIMER, REC_TIMER_PERIOD); //5sec*/
					break;
				}					
			break;
			default:
                break;
        }	
    }
}
#endif
 

