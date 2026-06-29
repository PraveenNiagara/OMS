#include "simcom_api.h"
#include "string.h"
#include "stdlib.h"
#include "platform.h"
#include "app_at_cmd_envelope.h" 
#include "smshandling.h"
#include "ModemConfig.h"
#include "livedata.h"
#include "app_utility.h"
#include "math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//extern UINT8 errorcount=0;
//extern void sAPP_MqttTask(void);
void MQTT_Reconnect(void);
UINT8 MQTTpublish_server(UINT16 len,UINT8 *data);
static sTaskRef gMqttProcessTask = NULL;
static UINT8 gMqttProcessTaskStack[1024*5] = {0xA5};   //[1024*7] = {0xA5};
UINT8 MQbuffer[500] = {0};
static sTaskRef gMqttRecvTask = NULL;
static UINT8 gMqttRecvTaskStack[1024*3]= {0xA5};
UINT8 MQTTReconflag=0;
UINT8 MQTTRepubflag=0;
UINT8 mqttTimerflag=0; 
UINT8 mqttconnected=0; 
UINT8 MqttInProgressState=0; 
UINT8 Mqtt_send_flag=0; 
UINT8 Nooftcpprocessed_count; 
UINT8 Retryflag=1;
UINT8 MqttInitStatus=0; 
unsigned char MqttInPrograssfailcount=0,Mqttinitfailcount=0;
int Resendcount=0;   
int failcount=0; 
INT8 payload[1050] = {0};  //1000//900//800//600
INT32 payload_len = 0;
INT8 PostData[1050];//1000//950
sMsgQRef urc_mqtt_msgq_1;
sMsgQRef uart_mqtt_msgq_1; 
 INT8 pTopic[100] = {0};
 INT8 topic[100] = {0};
 INT8 topic1[100] = {0};
sTimerRef MqttTimer;
 SC_STATUS status;
int cGreg=0,cgatt=0;
INT8 count=0; 
SCAPNact SCact[8] = {0};	
static sTimerRef timer3;
INT8 MqttServerIP[50]={0};

static sFlagRef g_flg1 = NULL;

//#define USE_FLAG 1

#define TIMER1_OUT_EVENT_MASK (0x1<<1)

void timerRoutine3(UINT32 argv)
{
	sAPI_Debug("timerRoutine3 ");
  
#ifdef USE_FLAG	

	sAPI_FlagSet(g_flg1,TIMER1_OUT_EVENT_MASK,SC_FLAG_OR);
#endif  
}

static void sAPP_Timer3(void* argv)
{
    SC_STATUS status;
	int ret; 
	status=sAPI_TimerCreate(&timer3);
    status=sAPI_TimerStart(timer3,200,200,timerRoutine3,0x1234);
	while(1)
	{
//	sAPI_UartPrintf("\n\rIn Timer1 while  loop\n\r");
	
#ifdef USE_FLAG	
		UINT32 event = 0;
		status = sAPI_FlagWait(g_flg1, TIMER1_OUT_EVENT_MASK, SC_FLAG_OR_CLEAR, &event,/*200*/ SC_SUSPEND);
		sAPI_Debug("status[%d] event[%d]",status,event);
				
					if(status == SC_SUCCESS)
						   {		 
										  
								sAPI_NetworkGetCgreg(&cGreg);	
						if(nMSettings.ndebugonof==1)								
						sAPI_UartPrintf("\n\rMQTT TEST,%d,%d,%d sgetflag_1 %d\n\r",MqttInitStatus,MqttInProgressState,cGreg,sgetflag_1);
						sprintf(buf,"\n\rlivedataflag]:%d,livedataflag1]:%d",livedataflag,livedataflag1);
								sAPI_UartPrintf(buf);
								sAPI_UartPrintf("\n\rNooftcprecvd:[%d],Nooftcpprocessed:[%d]",Nooftcprecvd,Nooftcpprocessed);
								sAPI_UartPrintf("\n\rNooftcprecvd1:[%d],Nooftcpprocessed1:[%d]",Nooftcprecvd1,Nooftcpprocessed1);
				  // if(Enter>=70&&cGreg==1&& CallConnected==0 && ringflag==0 )
				    if(s_nMSettings.m_Enter>=70 && CallConnected==0 && DeviceConfig.interface == GPRS)
					{ 				
				
						if(MqttInitStatus)
								{	

						       if(!MqttInProgressState)

								  {
					             if(nMSettings.ndebugonof==1)
                                 sAPI_UartPrintf("MQTT init okay line %d\n\r",__LINE__);								 
								//sprintf(buf,"\n\rlivedataflag]:%d,livedataflag1]:%d",livedataflag,livedataflag1);
								sAPI_UartPrintf(buf);
								if(sgetflag_1==1)
								{	

							   if(livedataflag ==0 && livedataflag1==0 && LogFlag == 0)					   
								{
								if(nMSettings.ndebugonof==1){
								sAPI_UartPrintf("\n\rNooftcprecvd:[%d],Nooftcpprocessed:[%d]",Nooftcprecvd,Nooftcpprocessed);
								sAPI_UartPrintf("\n\rNooftcprecvd1:[%d],Nooftcpprocessed1:[%d]",Nooftcprecvd1,Nooftcpprocessed1);
								}
							if(Nooftcprecvd1>Nooftcpprocessed1)
								{ 
							 
								sprintf(PostData,"%s",TCPwifiStrNumber1[Nooftcpprocessed1].TCPWifigprsstr1);
								Nooftcpprocessed1++;
								if(strlen(PostData)>10)
								{
								MQTTpublish_server(strlen(PostData),PostData);
								sAPI_UartPrintf("\n\rPUBLISH Length=%d-%d line %d\n\r",strlen(PostData),strlen(TCPwifiStrNumber1[Nooftcpprocessed1].TCPWifigprsstr1),__LINE__);
								}
								}
								else
								{
								 
								sprintf(PostData,"%s",TCPwifiStrNumber[Nooftcpprocessed].TCPWifigprsstr);			 
							     memset(PostData,NULL,sizeof(PostData));
							//	 memcpy(PostData,TCPwifiStrNumber[Nooftcpprocessed].TCPWifigprsstr,sizeof(TCPwifiStrNumber[Nooftcpprocessed].TCPWifigprsstr));
								memcpy(PostData,TCPwifiStrNumber[Nooftcpprocessed].TCPWifigprsstr,strlen(TCPwifiStrNumber[Nooftcpprocessed].TCPWifigprsstr));
								// sAPI_UartPrintf("\n\rPOST LENGTH =%d-%d\n\r",strlen(PostData),strlen(TCPwifiStrNumber[Nooftcpprocessed].TCPWifigprsstr));
								 
								Nooftcprecvd1=Nooftcpprocessed1=0;
								if(Nooftcpprocessed<Nooftcprecvd)
								{
									Mqtt_send_flag=1;
									if(strlen(PostData)>10)
									MQTTpublish_server(strlen(PostData),PostData);
								}						
								
								}    
								   
								
								} 
								
								 else
								{
								 
								//sprintf(PostData,"%s",TCPWifigprsstrBUFF);
								memset(PostData,NULL,sizeof(PostData));
							//	memcpy(PostData,TCPWifigprsstrBUFF,sizeof(TCPWifigprsstrBUFF));
								memcpy(PostData,TCPWifigprsstrBUFF,strlen(TCPWifigprsstrBUFF));
								sAPI_UartPrintf("\n\rPUBLISH LENGTH =%d-%d line %d %d \n\r",strlen(PostData),strlen(TCPWifigprsstrBUFF),__LINE__,sizeof(TCPWifigprsstrBUFF));
								if(strlen(PostData)>10)
								{
									// Mqtt_send_flag=0;
							     //	 Mqtt_data_local_flag=1;
								//	if(Nooftcpprocessed<Nooftcprecvd)  
									MQTTpublish_server(strlen(PostData),PostData);// post
								//	MQTTpublish_get(strlen(PostData),7000);
									
								}								 
								}  
								
								sgetflag_1=0;
								sAPI_UartPrintf("\n\r sgetflag_1=0 line %d ",__LINE__);
								}		 									
								
					}
					else

					{

						sgetflag_1=0;
						sAPI_UartPrintf("\n\r sgetflag_1=0 line %d ",__LINE__);

						//	sAPI_UartPrintf("\n\r no post or get ");

					}
				 	MqttInPrograssfailcount=0; 			
				 }
				 else

					{
                        
						sAPI_UartPrintf("Mqtt publish - waiting to complete, MqttInPrograssfailcount:%d \n", MqttInPrograssfailcount);
						if(MqttInPrograssfailcount++>=100)
						{							 
							MqttInPrograssfailcount=0;	
						}
                        if(cGreg==1 && Retryflag==0)
						 {  
					        sAPI_UartPrintf("\n\rMQTT_RECONNECTING....line 187");
							MQTTReconflag=1;count=0;
							while(count<15)												
							{
								 
								if(MQTTReconflag==1)
								{
								 sAPI_UartPrintf("\n\rMQTT_Reconnect attempt:%d",count+1);					
							sAPI_UartPrintf("\n\r MQTT_Reconnect line 195");
								 MQTT_Reconnect();	
								}
								else
								{ 
								  sAPI_UartPrintf("\r\nMQTT_Reconnect Successful!\r\n");					 
								  MQTTReconflag=0;MqttInitStatus=1; 
								  break;
								}
								count++;
								MqttInPrograssfailcount++;
							}
							 
						 }
					}
				}
		    }
			  #endif
		}
		
		
}









UINT8 MQTTpublish_server(UINT16 len,UINT8 *data)
{ 
			int ret = 0;  
			MqttInProgressState=1;
			int client_index=0;
			memset(payload,0,1050);
			memcpy(payload,data,strlen(data));
			payload_len=len;
				client_index =0;
			// sprintf(buf,"Payload:%s,payload_len:%d",payload,payload_len);
			// sAPI_UartPrintf(buf);	
					
	//	 sAPI_SysGetImei(IMEI);		
		 //sprintf(pTopic,"FirmwareToApp/%s",IMEI);
		 sprintf(pTopic,"%s/%s",DeviceConfig.MqttPublishTopic,IMEI);
		 sAPI_UartPrintf(pTopic);
		 ret= sAPI_MqttTopic(client_index,pTopic, strlen(pTopic));  
		 ret= sAPI_MqttPayload(client_index, payload, payload_len);
                
			ret= sAPI_MqttPub(client_index, 1, 60, 0, 0);
                if(SC_MQTT_RESULT_SUCCESS == ret)
                {
					sAPI_Debug("publish SUCCESS");
                    sAPI_UartPrintf("\r\nMQTT publish Successful!\r\n");
					Nooftcpsendcount=0;
					LogFlag=livedataflag=0;
					Resendcount++;
					sAPI_UartPrintf("Resendcount:%d,Mqtt_send_flag:%d",Resendcount,Mqtt_send_flag);
					
					if((strstr(payload,"LD01")!=0))
					Resendcount=0;
				
					if(Resendcount>=1 && Mqtt_send_flag==1) //Resendcount>=2
					{  
				       Nooftcpprocessed++;
                       Resendcount=0;
					}
                  
                }
                else
                {    
				    
                    sAPI_UartPrintf(buf,"\r\nMQTT Publish FAIL,ERRCODE = [%d]",ret);
					sAPI_UartPrintf(buf);
                       MqttInitStatus=0;MQTTReconflag=1;count=0;					
						sAPI_NetworkGetCgreg(&cGreg);
						sAPI_UartPrintf("\n\r cGreg:%d",cGreg);
						ret = sAPI_NetworkGetCgatt(&cgatt);
						if(ret == SC_NET_SUCCESS)
						sAPI_UartPrintf("Get Cgatt success. Cgatt=%d! ret %d",cgatt,ret);
						else
						sAPI_UartPrintf("Get Cgatt falied! ret%d",ret);
						
                       if(cGreg==1)
						 {  
					        sAPI_UartPrintf("\n\rMQTT RECONNECTING BCZ OF PUB FAIL...");
							 
						//	while(count<30)
							if(count<30)
							{
								 
								if(MQTTReconflag==1)
								{
								 sAPI_UartPrintf("\n\rMQTT Reconnect attempt:%d",count+1);					
								 sAPI_UartPrintf("\n\r MQTT_Reconnect line 288");
								 MQTT_Reconnect();	
								}
								else
								{ 
								  sAPI_UartPrintf("\r\nMQTT Reconnect Successful!\r\n");					 
								   MQTTReconflag=0;MqttInitStatus=1; 
								}
								count++;
							}
							/* if(MqttInitStatus)
							{    
						         sAPI_UartPrintf("\r\nMQTT publishing again!\r\n");	
								 sprintf(pTopic,"FirmwareToApp/%s",IMEI);
								 ret= sAPI_MqttTopic(client_index,pTopic, strlen(pTopic));  
								 ret= sAPI_MqttPayload(client_index, payload, payload_len);											
								 ret= sAPI_MqttPub(client_index, 1, 60, 0, 0);
							}  */ 
						 }
				 	 
					 
				}
				
				
				
			 
				//ret= sAPI_MqttTopic(client_index,"FirmwareToApp", strlen("FirmwareToApp"));
				ret= sAPI_MqttTopic(client_index,DeviceConfig.MqttServerTopic,strlen(DeviceConfig.MqttServerTopic));
				ret= sAPI_MqttPayload(client_index, payload, payload_len);
              

		   //if(!(strstr(payload,"LD04")!=0))
		    //{			  
			    ret= sAPI_MqttPub(client_index, 1, 60, 0, 0);
                if(SC_MQTT_RESULT_SUCCESS == ret)
                {
					sAPI_Debug("publish SUCCESS");
                    sAPI_UartPrintf("\r\nMQTT publish Successful! for Tweet\r\n");
                    
                }
                else
                {    
				    
                    sprintf(buf,"publish FAIL,ERRCODE = [%d]",ret);
					sAPI_UartPrintf(buf);
                    sAPI_UartPrintf("\r\nMQTT publish Fail!\r\n");
                    
					 
				}               
		   // }    
            MqttInProgressState=0;
}

void MQTT_Reconnect(void)
{ 
                            int ret = 0;  
					        sAPI_UartPrintf("\n\rMQTT RECONNECTING...");
					/* 		 sAPI_TaskSleep((10) * 200);
							 if(ret == SC_NET_SUCCESS)
							sAPI_UartPrintf("345Get Cgatt success. Cgatt=%d! ret %d",cgatt,ret);
							else
							sAPI_UartPrintf("347Get Cgatt falied! ret%d",ret);
							sprintf(topic,"tweet-response/%s",IMEI);
							sAPI_UartPrintf(topic);	
							ret = sAPI_NetworkGetCgatt(&cgatt);
							
							ret = sAPI_MqttUnsub(0,topic,strlen(topic), 0);
							sAPI_UartPrintf("22-----ret = %d", ret);
							if(SC_MQTT_RESULT_SUCCESS == ret)          
								sAPI_UartPrintf("\r\nMQTT Unsubscribe1#   Successful!\r\n");   */
							sAPI_TaskSleep((10) * 200);                  //sleep 30 sec before disconn							 
							ret = sAPI_NetworkGetCgatt(&cgatt);
							if(ret == SC_NET_SUCCESS)
							sAPI_UartPrintf("360Get Cgatt success. Cgatt=%d! ret %d",cgatt,ret);
							else
							sAPI_UartPrintf("362Get Cgatt falied! ret%d",ret);
							//sprintf(topic1,"AppToFirmware/%s",IMEI);//AppToFirmware
							sprintf(topic1,"%s/%s",DeviceConfig.MqttSubscribeTopic,IMEI);//AppToFirmware
						//	sprintf(topic1,"get-tweet-response/%s",IMEI);
							sAPI_UartPrintf(topic1);	
							
							ret = sAPI_MqttUnsub(0,topic1,strlen(topic1), 0);
							sAPI_UartPrintf("23-----ret = %d", ret);
							if(SC_MQTT_RESULT_SUCCESS == ret)              				 
							sAPI_UartPrintf("\r\nMQTT Unsubscribe2#  Successful!\r\n");
	                       
							sAPI_TaskSleep((10) * 600);                  //sleep 30 sec before disconn	
							ret = sAPI_NetworkGetCgatt(&cgatt);
							if(ret == SC_NET_SUCCESS)
							sAPI_UartPrintf("374Get Cgatt success. Cgatt=%d! ret %d",cgatt,ret);
							else
							sAPI_UartPrintf("376Get Cgatt falied! ret%d",ret);
							ret= sAPI_MqttDisConnect(0,NULL, 0, 60);
							sAPI_UartPrintf("8-----ret = %d", ret);
							
							ret= sAPI_MqttRel(0);
							sAPI_UartPrintf("9-----ret = %d", ret);
							
							ret= sAPI_MqttStop();
							sAPI_UartPrintf("10-----ret = %d", ret);
							 
							 sAPI_TaskSleep(100);
	                         ret = sAPI_MqttStart(-1);
							if(SC_MQTT_RESULT_SUCCESS == ret)    
								 sAPI_UartPrintf("\r\nMQTT Init#  Successful!\r\n");
							   
							else                                    
								sAPI_UartPrintf("\r\nMQTT Init#  Fail!\r\n");
                  
							ret = sAPI_NetworkGetCgatt(&cgatt);
							if(ret == SC_NET_SUCCESS)
							sAPI_UartPrintf("396Get Cgatt success. Cgatt=%d! ret %d",cgatt,ret);
							else
							sAPI_UartPrintf("398Get Cgatt falied! ret%d",ret);
							ret= sAPI_MqttAccq(0, NULL, 0,IMEI, 0, urc_mqtt_msgq_1);
							if(SC_MQTT_RESULT_SUCCESS == ret)
								sAPI_UartPrintf("\r\nMQTT accquire#  Successful!\r\n"); 
							 
							else						  
								sAPI_UartPrintf("\r\nMQTT accquire#  Fail!\r\n");
								 
						  
						
								ret= sAPI_MqttCfg(0, NULL, 0, 0, 0);
								 
									if(SC_MQTT_RESULT_SUCCESS == ret)                 
										sAPI_UartPrintf("\r\nMQTT config#   Successful!\r\n");
												
									else       
									  sAPI_UartPrintf("\r\nMQTT config Fail#  !\r\n"); 
								  
								ret = sAPI_NetworkGetCgatt(&cgatt);
							if(ret == SC_NET_SUCCESS)
							sAPI_UartPrintf("418Get Cgatt success. Cgatt=%d! ret %d",cgatt,ret);
							else
							sAPI_UartPrintf("420Get Cgatt falied! ret%d",ret);
								
									if(1==cgatt)
										{
					             sprintf(MqttServerIP,"tcp://%s",DeviceConfig.MqttServerIP);					             
								//ret= sAPI_MqttConnect(0, NULL,0, "tcp://13.229.229.198:1883", 60, 1, "niagara", "niagara@123");
								sAPI_UartPrintf("MQTT CREDENCIALS-IP:%s,UserName:%s,PassWord:%s",MqttServerIP,DeviceConfig.MqttUserName,DeviceConfig.MqttPassword);
								ret= sAPI_MqttConnect(0, NULL,0,MqttServerIP, 60, 1,DeviceConfig.MqttUserName,DeviceConfig.MqttPassword);
								sAPI_UartPrintf("\n\r11-----ret = %d", ret);
								if(SC_MQTT_RESULT_SUCCESS == ret)
								   {				 
								   sAPI_UartPrintf("\r\nMQTT Reconnect#  Successful!\r\n");
								   MqttInitStatus=1;MQTTReconflag=0; 
								   /* sprintf(topic,"tweet-response/%s",IMEI);
									sAPI_UartPrintf(topic);				 
									ret = sAPI_MqttSub(0,topic,strlen(topic), 0, 0);
									if(SC_MQTT_RESULT_SUCCESS == ret)          
										sAPI_UartPrintf("\r\nMQTT subscribe#   Successful!\r\n");   */
									
									//sprintf(topic1,"AppToFirmware/%s",IMEI);//AppToFirmware
									sprintf(topic1,"%s/%s",DeviceConfig.MqttSubscribeTopic,IMEI);//AppToFirmware
									
									sAPI_UartPrintf(topic1);				 
									ret = sAPI_MqttSub(0,topic1,strlen(topic1), 0, 0);
									if(SC_MQTT_RESULT_SUCCESS == ret)              				 
										 sAPI_UartPrintf("\r\nMQTT subscribe#  Successful!\r\n");	
								   
                                  }
								  else
								  { 
									sAPI_UartPrintf("\n\rMQTT Reconnect Fail again,ERRCODE=== [%d]",ret);
									/* if(failcount++>2)
									{
									sAPI_NetworkSetCfun(4);
									sAPI_TaskSleep(200*2);
									sAPI_NetworkSetCfun(1);
									failcount=0;
									} */
									MqttInitStatus=0;MQTTReconflag=1;
								  }
								}
					/*		sAPI_SysGetImei(IMEI);			  
							sprintf(topic,"tweet-response/%s",IMEI);
							sAPI_UartPrintf(topic);				 
							ret = sAPI_MqttSub(0,topic,strlen(topic), 0, 0);
							if(SC_MQTT_RESULT_SUCCESS == ret)          
								sAPI_UartPrintf("\r\nMQTT subscribe#   Successful!\r\n");  
							
							sprintf(topic1,"get-tweet-response/%s",IMEI);
							sAPI_UartPrintf(topic1);				 
							ret = sAPI_MqttSub(0,topic1,strlen(topic1), 0, 0);
							if(SC_MQTT_RESULT_SUCCESS == ret)              				 
								 sAPI_UartPrintf("\r\nMQTT subscribe#  Successful!\r\n");	*/			  
								  
							 
						     	  
						 
	
}


void MqttReadRspCb( UINT8* strData, UINT32 len)
{
	unsigned char StrTokStr[10][100];    // [50] [500]
	char StrTokStrVer1 = 0;
	char *Pch = NULL;
	unsigned char buff[250];
	 

	sprintf(buf,"%s:,len = %d\n", __func__, len);
	 sAPI_UartPrintf(buf);
	//sprintf(buf,"data %s\n",strData);
	//sAPI_UartPrintf(buf);	                                   // get data 
	memset(buff,0,250);
	
	 
		strcpy(buff,strData);
//		 sprintf(buf,"\n\rbuff=%s \n\r",buff);
//		 sAPI_UartPrintf(buf);
		Pch = strtok((char *)buff, (char *)"\"\"" );  //"{}"   // "\"\""  //removes {}    
		sprintf(buf,"Pch is %s",Pch);
		sAPI_UartPrintf(buf);
		StrTokStrVer1 = 0;
		 while(( Pch != NULL ) && (StrTokStrVer1<=3))   //3
		 
		{
			
			strcpy(StrTokStr[StrTokStrVer1],Pch);
//		   sprintf(buf,"\n\rStrTokStr[%d] is %s",StrTokStrVer1,StrTokStr[StrTokStrVer1]);
//		   sAPI_UartPrintf(buf);
		//	sAPI_Debug("\n\r StrTokStr[%d] is %s",StrTokStrVer1,StrTokStr[StrTokStrVer1]);
			StrTokStrVer1++;
		//	sAPI_Debug("StrTokStrVer1 is %d",StrTokStrVer1);
		 	Pch = strtok( NULL,"\"\"" );
		//	sAPI_Debug("StrTokStrVer1 is %d",StrTokStrVer1);
			 
		}  
		

		sprintf(buf,"\n\r StrTokStr[3] is %s",StrTokStr[3]); 
		sAPI_UartPrintf(buf);

	 if((strstr(StrTokStr[3],"Successfully")!= 0)||(strstr(StrTokStr[3],"Already Inserted")!= 0)||(strstr(StrTokStr[3],"Mac Address")!= 0)) 
	{
	    sprintf(buf,"\n\r line is %s",__LINE__); 
		sAPI_UartPrintf(buf);
	  if(livedataflag==0 && livedataflag1==0  && Nooftcpprocessed1==0)
		{
			if(Mqtt_send_flag)
			{
			sprintf(buf,"\n\r line is %s",__LINE__); 
		sAPI_UartPrintf(buf);
			Nooftcpprocessed++;
			Mqtt_send_flag=0;
			Resendcount=0;
			// Nooftcpprocessed_count=0;
			}  
	    }  
          //	sAPI_Debug("Nooftcpprocessed is %d",Nooftcpprocessed );
		Nooftcpprocessed_count=0;
		errorcount=0;
		livedataflag=0;
		Resendcount=0;
	}
	else if(strstr(StrTokStr[3],"#live")!= 0)
	{
		 
		livedataflag=1;
		errorcount=0;
		 
		 
	}
	else if(strstr(StrTokStr[3],"Error")!= 0)
	{
//		sAPI_Debug("\n\rdg_entry to line no %d ",__LINE__);
		if(Nooftcpprocessed1==0)
		{
		Nooftcpprocessed++;
		Nooftcpprocessed_count=0;
		Resendcount=0;
		}
		errorcount=0;
	} 
	  else if(strstr(StrTokStr[3],"400 Bad")!= 0)
	{
//		sAPI_Debug("\n\rdg_entry to line no %d ",__LINE__);
		if(errorcount++>3)
		{
		if(Nooftcpprocessed1==0)
		{
			Nooftcpprocessed++;
			Nooftcpprocessed_count=0;
			Resendcount=0;
		}
			errorcount=0;
		}
	}  
	  /* else if(strstr(StrTokStr[3],"405")!= 0)
	{
		//sAPI_Debug("\n\rdg_entry to line no %d ",__LINE__);
		if(errorcount++>3)
		{
		if(Nooftcpprocessed1==0)
		{
			Nooftcpprocessed++;
			errorcount=0;
		}
		}
	}  */ 
 	else if(strstr(StrTokStr[3],"Already")!= 0)
	{
//		sAPI_Debug("\n\rdg_entry to line no %d ",__LINE__);
		if(errorcount++>3)
		{
		if(Nooftcpprocessed1==0)
		{
			Nooftcpprocessed++;
			Nooftcpprocessed_count=0;
			Resendcount=0;
		}
			errorcount=0;
				livedataflag=0;
		}
	}  
	  else if((strstr(StrTokStr[3],"Setting")!= 0))
		{
          //sAPI_Debug("\n\rdg_entry to line no %d ",__LINE__);
			livedataflag=0;}
	
	    else
	   {  
//		sAPI_Debug("\n\rstrlen(StrTokStr[3]) is %d",strlen(StrTokStr[3]));
		if((strlen(StrTokStr[3])<=160) && (strlen(StrTokStr[3])>1))  //dg_changed from 3
		{
			 
			 
//			sAPI_Debug("\n\rdg_entry to line no %d Noofsettingsrecvd is %d",__LINE__,Noofsettingsrecvd);
			 // strcpy((char *)wifiStrNumber[Noofsettingsprocessed].Wifistr,(char*)StrTokStr[3]);  //cmt aj_ch
			strcpy((char *)wifiStrNumber[Noofsettingsrecvd].Wifistr,(char*)StrTokStr[3]);
			 sprintf(buf,"\n\rwifistr:%s",wifiStrNumber[Noofsettingsrecvd].Wifistr);
			 sAPI_UartPrintf(buf);
			 Noofsettingsrecvd++;
			sAPI_UartPrintf("Noofsettingsrecvd:%d",Noofsettingsrecvd);
		     if(Noofsettingsrecvd>200)
				Noofsettingsrecvd=0;
//			sAPI_Debug("\n\rdg_entry to line no %d Noofsettingsrecvd is %d",__LINE__,Noofsettingsrecvd);
			memset(buff,0,250);
			memset(StrTokStr[3],0,100);  
//			sAPI_Debug("\n\rentry to line no %d",__LINE__);
//			sAPI_Debug("\n\rdg_setting loaded");
	    	 
		    
		}
		
	   } 
   
	   
}





 void sTask_MqttProcesser1(void)                 //thread of control
 {		 
	 SCcgpaddrParm cgpaddrParm; 
	 int creg,cgreg,cnmp,cgatt;
	 char cpin[50]={0};
	 int ret = 0;    
	 INT8 buf[150];
	 INT32 pGreg = 0;
	 INT32 client_index = 0;
	 INT8 clientID[256+1] = { 0 };
	 INT32 serverType = 0;
	 INT32 config_type = 0;
	 INT32 config_value = 0;
	 INT8 serverAdd[100+1] = { 0 };
	 INT8 usrName[100+1] = { 0 };
	 INT8 usrPwd[100+1] = { 0 };

	
	 
	 INT32 topic_len = 0;
	
	 
	 SIM_MSG_T optionMsg ={0,0,0,NULL};
	 //memcpy(DeviceConfig.MqttServerIP,"13.229.229.198:1883",strlen("13.229.229.198:1883"));
	 //memcpy(DeviceConfig.MqttServerIP,"3.0.229.165:1883",strlen("3.0.229.165:1883"));
	 //memcpy(DeviceConfig.MqttUserName,"niagara",strlen("niagara"));
	// memcpy(DeviceConfig.MqttPassword,"niagara@123",strlen("niagara@123"));
		 
	 
	   while(1)
    { 
       
	   
	   sAPI_NetworkGetCgreg(&pGreg);
        if(1 != pGreg)
        {
            sprintf(buf,"\n\rMQTT NETWORK STATUS IS [%d]",pGreg);
			sAPI_UartPrintf(buf);
            sAPI_TaskSleep(400);
        }
        else
        {
            sAPI_UartPrintf("MQTT NETWORK STATUS IS NORMAL");
            break;
        }
    }  
	
   
	 
	 while(!mqttconnected)
	 {
				ret = sAPI_MqttStart(-1);
				 sAPI_UartPrintf("sAPI_MqttStart ret A %d",ret);
				if(SC_MQTT_RESULT_SUCCESS == ret)
                {
					 
                    sAPI_UartPrintf("\r\nMQTT Init Successful!\r\n");
					
                }
                else
                {
					sAPI_UartPrintf("sAPI_MqttStart ret B %d",ret);
                    sprintf(buf,"Init FAIL,ERRCODE = [%d]",ret);
					sAPI_UartPrintf(buf);
                    sAPI_UartPrintf("\r\nMQTT Init Fail!\r\n");
                  
                }
		 
			//	 sAPI_SysGetImei(IMEI);  //dg_eco
	        //    sAPI_UartPrintf(IMEI); 	//dg_eco			
			 	 client_index = 0;	 	
				 memcpy(clientID,IMEI,strlen(IMEI));     //MODULE IMEI NUM   
				 ret= sAPI_MqttAccq(0, NULL, client_index,clientID, 0, urc_mqtt_msgq_1);
				if(SC_MQTT_RESULT_SUCCESS == ret)
                {
					 
                    sAPI_UartPrintf("\r\nMQTT accquire Successful!\r\n");
					 
                }
                else
                {
                    sprintf(buf,"Init FAIL,ERRCODE = [%d]",ret);
					sAPI_UartPrintf(buf);
                    sAPI_UartPrintf("\r\nMQTT accquire Fail!\r\n");
					 
               }

			 
				client_index = 0;

				ret= sAPI_MqttCfg(0, NULL, client_index, 0, 0);
				if(SC_MQTT_RESULT_SUCCESS == ret)
                {
					sAPI_Debug("config SUCCESS");
                    sAPI_UartPrintf("\r\nMQTT config Successful!\r\n");
				 
                }
                else
                {
                    sAPI_Debug("config FAIL,ERRCODE = [%d]",ret);
                    sAPI_UartPrintf("\r\nMQTT config Fail!\r\n");
				}

			 
				 
		 		 	 
				// memcpy(serverAdd,"tcp://13.229.229.198:1883", strlen("tcp://13.229.229.198:1883"));
				//memcpy(serverAdd,"tcp://3.0.229.165:1883", strlen("tcp://3.0.229.165:1883"));
				// memcpy(usrName,"niagara", strlen("niagara"));
		 		// memcpy(usrPwd, "niagara@123", strlen("niagara@123"));			 
				sprintf(MqttServerIP,"tcp://%s",DeviceConfig.MqttServerIP);					             
				//ret= sAPI_MqttConnect(0, NULL,0, "tcp://13.229.229.198:1883", 60, 1, "niagara", "niagara@123");
				sAPI_UartPrintf("MQTT CREDENCIALS-IP:%s,UserName:%s,PassWord:%s",MqttServerIP,DeviceConfig.MqttUserName,DeviceConfig.MqttPassword);
				ret= sAPI_MqttConnect(0, NULL,0,MqttServerIP, 60, 1,DeviceConfig.MqttUserName,DeviceConfig.MqttPassword);
								
				// ret= sAPI_MqttConnect(0, NULL,0, serverAdd, 60, 1, usrName, usrPwd);
				if(SC_MQTT_RESULT_SUCCESS == ret)
                {				 
                    sAPI_UartPrintf("\r\nMQTT connect Successful!\r\n");
                     MqttInitStatus=1;Retryflag=0; 
                }
                else                                        
                {   sprintf(buf,"\n\rMQTT connect Fail,ERRCODE=== [%d]",ret);
					sAPI_UartPrintf(buf);
					MqttInitStatus=0;MQTTReconflag=1;count=0;Retryflag=0;                    
					 
			    }
	
			 
				
				 
//----------------------------------SUB TOPIC1--------------------------------------------------------------------------	 		
                client_index =  0;	 		 
				/* sprintf(topic,"tweet-response/%s",IMEI);
				sAPI_UartPrintf(topic);				 
				ret = sAPI_MqttSub(client_index,topic,strlen(topic), 0, 0);
				if(SC_MQTT_RESULT_SUCCESS == ret)          
                    sAPI_UartPrintf("\r\nMQTT subscribe Successful!\r\n");               
               else
                {
                    sprintf(buf,"\n\r [%s]subscribe FAIL,ERRCODE = [%d]",topic,ret);
					sAPI_UartPrintf(buf);
                    sAPI_UartPrintf("\r\nMQTT subscribe Fail!\r\n");                    
                } */
//----------------------------------SUB TOPIC2-----------------------------------------------------------------------				
				//sprintf(topic1,"AppToFirmware/%s",IMEI);//AppToFirmware
				sprintf(topic1,"%s/%s",DeviceConfig.MqttSubscribeTopic,IMEI);//AppToFirmware
				sAPI_UartPrintf(topic1);				 
				ret = sAPI_MqttSub(client_index,topic1,strlen(topic1), 0, 0);
				if(SC_MQTT_RESULT_SUCCESS == ret)              				 
                     sAPI_UartPrintf("\r\nMQTT subscribe Successful!\r\n");                 				 
                else
                {
                    sprintf(buf,"\n\r [%s]subscribe FAIL,ERRCODE = [%d]",topic1,ret);
					sAPI_UartPrintf(buf);
                    sAPI_UartPrintf("\r\nMQTT subscribe Fail!\r\n");                    
                } 
		mqttconnected=1;
		 break;
	 }			
 	 
 }
void sTask_MqttRecvProcesser1(void)
{   
       
	 
	 while(1)
    {     
        SIM_MSG_T msgQ_data_recv = {SIM_MSG_INIT, 0, -1, NULL};                 //NULL pointer for msgQ_data_recv.arg3 is necessary!
        SCmqttData *sub_data = NULL;
        /*=======================================================================================================================
        *
        *   NOTE: if this data reception cycle too long may cause data loss(data processing slower than data receive from server)
        *
        **=======================================================================================================================*/
        //recv the subscribed topic data, from the message queue: urc_mqtt_msgq_1, it is set buy sAPI_MqttAccq                                          //*===================*//
        sAPI_MsgQRecv(urc_mqtt_msgq_1, &msgQ_data_recv, SC_SUSPEND);                                                                                    //                     //
        if((SC_SRV_MQTT != msgQ_data_recv.msg_id) || (0 != msgQ_data_recv.arg1) || (NULL == msgQ_data_recv.arg3))   //wrong msg received                //                     //
            continue;                                                                                                                                   //                     //
        sub_data = (SCmqttData *)(msgQ_data_recv.arg3);                                                                                                  //                     //
        /*this part just for test, the payload msg just ascii string*/                                                                                  //                     //          
        /* sAPI_Debug("MQTT TEST----------index: [%d]; tpoic_len: [%d]; tpoic: [%s]", sub_data->client_index, sub_data->topic_len, sub_data->topic_P);     //                     //
                                                                                                                                                        //                     //
        sAPI_Debug("MQTT TEST----------payload_len: [%d]", sub_data->payload_len); */                                                                      //   rception cycle    //
		/* sAPI_UartWrite(SC_UART2,(UINT8*)"recieve topic: ",strlen("recieve topic: "));
		sAPI_UartWrite(SC_UART2,(UINT8*)sub_data->topic_P,sub_data->topic_len);
		sAPI_UartWrite(SC_UART2,(UINT8*)"\r\n",2);
		sAPI_UartWrite(SC_UART2,(UINT8*)"recieve payload: ",strlen("recieve payload: "));
		sAPI_UartWrite(SC_UART2,(UINT8*)sub_data->payload_P,sub_data->payload_len);  */
		strcpy(MQbuffer,(UINT8*)sub_data->payload_P);
        MqttReadRspCb(MQbuffer, strlen(MQbuffer));		               //MQTT data split processing func 
  //      sAPI_UartWrite(SC_UART2,(UINT8*)"\r\n",2);                                                                                                               //                     //
        /*************************************************************************************/                                                         //                     //
        /*************************************************************************************/                                                         //                     //
        /*these msg pointer must be free after using, don not change the free order*/                                                                   //                     //
            sAPI_Free(sub_data->topic_P);                                                                                                               //                     //
            sAPI_Free(sub_data->payload_P);                                                                                                             //                     //
            sAPI_Free(sub_data);                                                                                                                        //                     //
        /*************************************************************************************/                                                         //                     //
        /*************************************************************************************/                                                         //*===================*//
    }
}
 
 
 void sAPP_MqttTask(void)
 {
	 SC_STATUS status;
     sAPI_UartPrintf("%s Task2 creation completed!!\n", __func__);
     if ((NULL != gMqttProcessTask) || (NULL != gMqttRecvTask))
     {
         return;
     }
     sAPI_UartPrintf("%s Task2 creation completed!!\n", __func__);
     /*creat msgq*/     
     /*==================================================================================================================
     *  urc_mqtt_msgq_1: queue for receiving the subsribed topic data, data is cached in the queue
     *
     *  MQTT_MSGQ_QUEUE_SIZE: Maximum cached messages in the queue
     *
     *  NOTE: If the data receiving speed is less than the sending speed, the following data will be lost. 
     *        If the reading "-----MSG SEND ERROR-----, The message queue of client-[0/1] is full" in the log record, 
     *        please increase the length of the queue(normal value is 4) or speed up the receiving speed 
     **==================================================================================================================*/
     status = sAPI_MsgQCreate(&urc_mqtt_msgq_1, "urc_mqtt_msgq_1", (sizeof(SIM_MSG_T)),20, SC_FIFO);        //msgQ for subscribed data transfer
     if(status != SC_SUCCESS)
        sAPI_UartPrintf("message queue creat err!\n");
sAPI_UartPrintf("message queue1\n");
	 /* status = sAPI_MsgQCreate(&uart_mqtt_msgq_1, "uart_mqtt_msgq_1", (sizeof(SIM_MSG_T)), 4, SC_FIFO);		//msgQ for subscribed data transfer
	 if(status != SC_SUCCESS)
		sAPI_UartPrintf("message queue creat err!\n");
	 sAPI_UartPrintf("message queue2n"); */
     /*creat a thread*/
	 if (NULL != gMqttProcessTask)
    {
		 sAPI_UartPrintf("%s  line %d\n", __func__,__LINE__);
        return;
    }
     if(sAPI_TaskCreate(&gMqttProcessTask, gMqttProcessTaskStack, 1024*5,90, (char*)"mqttProcesser", 
	 sTask_MqttProcesser1,(void *)0)) //init the demo task
     {
        gMqttProcessTask = NULL;
        sAPI_UartPrintf("Task Create error!\n");
     }
     sAPI_UartPrintf("message queue3\n");
	 if (NULL != gMqttRecvTask)
    {
		 sAPI_UartPrintf("%s  line %d\n", __func__,__LINE__);
        return;
    }
     if(sAPI_TaskCreate(&gMqttRecvTask, gMqttRecvTaskStack, 1024*3, 85, (char*)"mqttProcesser", 
	 sTask_MqttRecvProcesser1,(void *)0)) //init the demo task
     {
        gMqttRecvTask = NULL;
        sAPI_UartPrintf("Task Create error!\n");
     }
	   sAPI_UartPrintf("message queu4\n");
	   /*status=sAPI_TimerCreate(&MqttTimer);                
	   status = sAPI_TimerStart(MqttTimer,2000,2000,MqttTimerRoutine,0x1234); 
	   MqttTimer_task();*/ 
	 
 }
 
 
static sTaskRef mqttTimerProcesser;

static UINT8 mqttTimerProcesserStack[6*1024];  //[6*1024];

void mqttTimer_init(void)
{
    SC_STATUS status;
#ifdef USE_FLAG	
	sAPI_FlagCreate(&g_flg1);
#endif
	 if (NULL != mqttTimerProcesser)
    {
		 sAPI_UartPrintf("%s  line %d\n", __func__,__LINE__);
        return;
    }
    status = sAPI_TaskCreate(&mqttTimerProcesser, mqttTimerProcesserStack, 5*1024, 150, "mqttTimer",sAPP_Timer3,(void *)0);
	sAPI_Debug("mqttTimer",status);
	sAPI_UartPrintf("%s Task2 creation completed!!\n", __func__);
	if(SC_SUCCESS != status)
    {
        sAPI_UartPrintf("Task create fail");
    }        
} 
 


