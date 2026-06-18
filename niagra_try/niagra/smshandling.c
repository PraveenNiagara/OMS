

#include <string.h>
#include "platform.h"
#include "app_custom.h"
#include "ModemConfig.h"
#include "smshandling.h"
#include "simcom_api.h"
#include "simcom_sms.h"
 

#define MAX_SMS_STRING_LEN          160
#define MASTER_MOBILE_NUMBER_LEN    24

#define URC_PROCESS_TASK_STACK_SIZE (1024*2) //(1024*8)
//#define URC_PROCESS_TASK_PRIORITY (150)
 
#define FALSE 0
#define TRUE 1
#define SMS_URC_RECIVE_TIME_OUT 1000
#define MAX_SMS_INPUT_DATA_LEN 400

#define  SLAVE_ADDR 0xC8
 
static UINT8 gUrcProcessTaskStack[URC_PROCESS_TASK_STACK_SIZE] = {0xA5};
static UINT8 gSmsUrcProcessTaskStack[1024*5];//[1024*5]
static sTaskRef gSmsUrcProcessTask = NULL;

SIM_MSG_T msg;
static sTaskRef SMSProcessTask =NULL;
static sMsgQRef gUrcMsgQueue = NULL;
static sTaskRef gUrcProcessTask = NULL;
sMsgQRef g_sms_demo_msgQ;
sMsgQRef g_sms_demo_urc_process_msgQ;
sMsgQRef g_sms_demo_urc_rsp_msgQ;
sMsgQRef g_call_msgQ;
sMsgQRef g_AT_urc_msgQ;
//char printbuf[700]={0};
//INT8 ack_status[300]={0};
UINT8 dtm[25];
UINT8 SMSMno[15];
//UINT8 CalUrcbuf[500];
extern INT8 smsbuffer[160]="";
extern INT8 ph_num[14]={0};
extern INT8 Cph_num[14]={0};
sMsgQRef ntpUIResp_msgq; 
extern UINT8 topicc[100]={0};
extern UINT8 topicc1[100]={0};
extern INT8 sms[250]={0};
extern INT32 tempStr[10];
char UNIT_PHNO[10]={0};
extern void SmsDemoInit(void);
extern void smsMsgRead(void);
extern void smsRead(void);
extern void SmsDelAll(void);
extern char PhoneNumber_1[20] = "0000000000";
 
UINT8 Netdisflag;
//extern sMsgQRef httpsUIResp_msgq;
extern void PrintfResp(INT8* format);
extern void Https_post(void);

sMsgQRef ntpUIResp_msgq;
extern INT8 hidden_num[14];
int creg,cgreg;
static INT8 reg_stat,reset,timer_on;
INT8 explicit_pdp_deact=0;
sTimerRef timerRef;
SC_STATUS status;

int Netcon=0;

int Tp;
long value;
long value1;
long TpHr;
long TpMin;
long TpSec;
long TpHr1;
long TpMin1;
long TpSec1;

typedef enum{
    SC_SMS_INIT,
    SC_SMS_EXIT
}SC_SMS_INIT_BRANCH;

INT32 msgIndex=1;
 
extern sockaddr_struct g_server_address;
extern unsigned char foggerRTCCheckFirstTime,nightlightRTCCheckFirstTime,fanRTCCheckFirstTime,lightRTCCheckFirstTime;
typedef enum{PDU,TEXT}SMS_FORMAT;
typedef struct
{
    SMS_FORMAT formatType;
    ascii sc_number[MASTER_MOBILE_NUMBER_LEN+1];
    ascii phone_number[MASTER_MOBILE_NUMBER_LEN+1];
    UINT8 sms_string[MAX_SMS_STRING_LEN];
    UINT8 mt;      //mt = 2, flash message
} SIMCOM_SMS_INFO;

 
const SIMCOM_SMS_INFO g_sms_info =
{
    TEXT,
    "",             //sc number
    "9845200686",
    "9845200686",
    1
};



/*  static void eat_sms_delete_cb(BOOL result)
{
   
   sprintf(buf,"eat_sms_delete_cb, result=%d",result);
   PrintfResp(buf);
} */
 
/*  BOOL simcom_sms_msg_delete(UINT16 msgIndex)
{
    sprintf(buf,"simcom_sms_msg_delete index = %d", msgIndex);
	PrintfResp(buf);
    return sAPI_SmsDelOneMsg(msgIndex,g_sms_msgQ); 
} */
 
/*   static eat_sms_flash_message_cb()//EatSmsReadCnf_st smsReadCnfContent
{
    UINT8 format =0;
    //eat_get_sms_format(&format);
    sprintf(buf,"eat_sms_flash_message_cb, format=%d",format);
     PrintfResp(buf);
	if(1 == format)//TEXT mode
    {
         sprintf(buf,"eat_sms_read_cb, msg=%s",msg.arg3);
         //sprintf(buf,"eat_sms_read_cb, datetime=%s",smsReadCnfContent.datetime);
        //sprintf(buf,"eat_sms_read_cb, name=%s",smsReadCnfContent.name);
        //sprintf(buf,"eat_sms_read_cb, status=%d",smsReadCnfContent.status);
        //sprintf(buf,"eat_sms_read_cb, len=%d",smsReadCnfContent.len);
        sprintf(buf,"eat_sms_read_cb, number=%s",ph_num); 
    }
    else//PDU mode
    {
        sprintf(buf,"eat_sms_read_cb, msg=%s",msg.arg3);
        //sprintf(buf,"eat_sms_read_cb, len=%d",smsReadCnfContent.len);
    }
}
 */
/* static void eat_sms_send_cb(BOOL result)
{
	sprintf(buf,"eat_sms_send_cb, result=%d ",result);
	//SendSMS=28;

}
 */
 
 
BOOL simcom_sms_send(UINT8* number, UINT8* sms, UINT16 smsLen)
{
	INT32 ret;
    if(NULL == number) 
		PrintfResp("number false");    
  
    if(smsLen > 160) 
		PrintfResp("smsLen False");        
	
	sAPI_SmsSendMsg(1,sms,smsLen,number,g_call_msgQ);       //API for send SMS
     
        if(ret == SC_SMS_SUCESS)
			{
				memset(&msg,0,sizeof(msg));
				sAPI_MsgQRecv(g_call_msgQ,&msg,20000);				 
				sprintf(buf,"\r\n\r\nsAPI_SmsSendMsg:\r\n\tprimID[%d] \r\n\tresultCode[%d]\r\n\trspStr[%s]\r\n",msg.arg1,msg.arg2, (INT8*)msg.arg3);
				sAPI_Free(msg.arg3);
				sAPI_UartWriteString(SC_UART2, (UINT8*)buf);
			}
		else
			{
				sAPI_UartWriteString(SC_UART2, (UINT8*)"\r\n\tERROR: SMS Send msg failed!\r\n");
			}
    
}

/* BOOL simcom_sms_sc_set(UINT8* number)
{
    if(NULL == number){
        return FALSE;
    }
    sprintf(buf,"simcom_sms_sc_set Number=%s",number);
    return eat_set_sms_sc(number);
} */

/* BOOL simcom_sms_format_set(UINT8 nFormatType)
{
    sprintf(buf,"simcom_sms_format_set FormatType=%d", nFormatType);//SCsmsFormatMode fmatmode;
    return eat_set_sms_format(nFormatType);
} */

/* BOOL simcom_sms_cnmi_set(UINT8 mt)
{
    sprintf(buf,"simcom_sms_cnmi_set %d", mt);
    return eat_set_sms_cnmi(2,mt,0,0,0);
} */
#if 0
void sTask_SmsUrcProcessor(void) 
{  
    SIM_MSG_T  msg;
	SCsmsFormatMode fmatmode;
	int index=10;
	char name[10]="SMSM";
	char at_buf[100];			
	fmatmode = 1;
    UINT8 format =0,i=0,j=0;
	INT8 mobindx = -1;
	INT8 rspType = -1;
	unsigned int Tp,Tp1,Tp2,Tp3,len;
//	UINT8 smsbufferasitis[160]="";
	UINT8  *p = EAT_NULL;
	char *Pch1= EAT_NULL;
	int StrTokStrVer=0;
	char StrTokStr1[15][20];
	char BigSMS2[10]=""; 
	  
	 char StrTokStr[20][20];
	 char *Pch=NULL;
    char rsp_buff[400] = {0};
    SC_SMSReturnCode ret = SC_SMS_SUCESS;
    SC_STATUS status;
    int retval = -1;
    SIM_MSG_T SmsUrcProcessReq;
    SIM_MSG_T msg_rsp;
    SIM_MSG_T smsurcdata;



		while(1) 
			{	 
				memset(&smsurcdata, 0, sizeof(SIM_MSG_T));				 
				sAPI_MsgQRecv(g_sms_demo_urc_process_msgQ, &smsurcdata, SC_SUSPEND);
				sprintf(buf,"\n\rSMS URC RESP:%s, ",smsurcdata.arg3);
				PrintfResp(buf);
				memcpy(rsp_buff,smsurcdata.arg3,strlen(smsurcdata.arg3));
				sAPI_Free(smsurcdata.arg3);
			   
 			    memset(ph_num,0,sizeof(ph_num));
				memset(smsbuffer,0,sizeof(smsbuffer));
				if(strstr(rsp_buff,"+CMGR") != 0 )    //[+CMGR: "REC UNREAD","+918110054669","","22/02/17,14:20:58+20"<CR><LF>]
				 {
				Pch = strtok((char *)rsp_buff, (char *)"\"\"" );
				int StrTokStrVer = 0;
				while( Pch != NULL )
				{
				strcpy(StrTokStr[StrTokStrVer],Pch);
				StrTokStrVer++;
				Pch = strtok( NULL,"\"\"" );
				}
				for(Tp=0;Tp<=4;Tp++)
				{
					// sprintf(buf,"\n\rSMS St=%s  pos=%d\n\r",StrTokStr[Tp],Tp);
					// PrintfResp(buf);
					
				} 
				strcpy(ph_num,StrTokStr[3]);				 
				sprintf(buf,"\n\rSMS Phonenumber:%s\n\r",ph_num);
				PrintfResp(buf);
				strncpy(dtm,(rsp_buff+42),17);
				sprintf(buf,"\n\rSMS dtm str:%s\n\r",dtm);
				PrintfResp(buf);
				strncpy(smsbuffer,(rsp_buff+65),strlen(rsp_buff+63));					 
				sprintf(buf,"\n\rSMS str:%s\n\r",smsbuffer);
				PrintfResp(buf);
				 }


	
	 if(1 == fmatmode && strstr(rsp_buff,"+CMGR") != 0)                    //TEXT mode
    { 
			p=ph_num;                          //phone_number;
			if(strstr(smsbuffer,"FW$"))
			{
				fotaflsg=1;
				PrintfResp("\n\rfota sms triggered");
			} 
			else if(strstr(smsbuffer,"delete") != 0 )
			{
				Delete_Settings_files();			 
		    }		
		    for (i = 0; i <= 10; i++) //(i = 0; i <= 10; i++)
		    {
			/* sprintf(buf,"recived no %s , Reg number %s",p+j,SmsNumber[i]);
			PrintfResp(buf); */
			j=strlen(StoredPhoneSmscode[i]);   
			sprintf(buf,"j value:%d",j);
			PrintfResp(buf);
			sprintf(buf,"\n\rRecived no:%s , Reg number:%s,%d",p+j,SmsNumber[i],j);
			PrintfResp(buf);
			if (!strncmp(StoredPhoneNumber[i], p+j, strlen(StoredPhoneNumber[i])) && strlen(StoredPhoneNumber[i])>0)
			{
				mobindx = i;        
				strcpy(PhoneNumber,p+j);
				sprintf(buf,"\n\rPhoneNumber> %s",PhoneNumber);
				PrintfResp(buf);
				if(strstr(smsbuffer,"sm#8185"))
				str_remov_last(PhoneNumber,10); //dg_added for only 10 digit number
				strcpy(PhoneNumber_1,PhoneNumber);
				sprintf(buf,"\n\rPhoneNumber_1 is %s",PhoneNumber_1);
				PrintfResp(buf);
				//sms_data_flag=1;
				break;
			}
		}   
		//strcpy(PhoneNumber,p+3);
		  sprintf(buf,"\n\rmobindx:%d, HowManyNumberFound %d",mobindx,HowManyNumberFound);
		  PrintfResp(buf);
		p = smsbuffer;
		for (i = 0; i < strlen(smsbuffer); i++) // convert to Lcaps
		{
			if(*p>='A' && *p<='Z')
			*p = (*p+'a')-'A';
			p++;
		} 
		
		 sprintf(buf,"\n\rLsmsData %s\n",smsbuffer);
		 PrintfResp(buf);
		p = ph_num;
		if(strstr(smsbuffer,"n+"))
		{
		UINT8 SMSMnumberStor[15] = "";
	if(!strlen(StoredPhoneNumber[0])<8 && (!strcmp(StoredPhoneNumber[0],"0000000000")))
	{
		/* Pch1 = strtok((char *)smsbuffer, (char *)"," );
		StrTokStrVer = 0;
		while( Pch1 != NULL )
		{
		strcpy(StrTokStr1[StrTokStrVer],Pch1);
		StrTokStrVer++;
		Pch1 = strtok( NULL, "," );
		}
		eat_trace("StrTokStr1[0] is %s",StrTokStr1[0]);
		eat_trace("StrTokStr1[1] is %s",StrTokStr1[1]);
		
		 if(strcmp(StrTokStr1[1],"<CR><LF>")!=0)
		{
		strcpy((char *)StoredPhoneSmscode[0],(char *)StrTokStr1[1]);
		eat_trace("StoredPhoneSmscode[0] is %s",StoredPhoneSmscode[0]);
		}  */
		
		//	eat_trace("\n\rSMSMnumber: %s \n\r",SMSMnumberStor);
		if((strstr(DEFAULTNUMBER,p)==0)|| (strstr(DEFAULTNUMBER1,p)==0)|| (strstr(DEFAULTNUMBER2,p)==0)|| (strstr(DEFAULTNUMBER3,p)==0)|| (strstr(DEFAULTNUMBER4,p)==0)|| (strstr(DEFAULTNUMBER5,p)==0)|| (strstr(DEFAULTNUMBER6,p)==0)|| (strstr(DEFAULTNUMBER7,p)==0)|| (strstr(DEFAULTNUMBER8,p)==0)|| (strstr(DEFAULTNUMBER9,p)==0)|| (strstr(DEFAULTNUMBER10,p)==0)|| (strstr(DEFAULTNUMBER11,p)==0)|| (strstr(DEFAULTNUMBER12,p)==0))

			{
				str_remov_last(p,10);
				strcpy(SMSMnumberStor,p);
				len = strlen(smsbuffer);
				Tp1 = 0;

				sAPI_UartPrintf("\n\r1SMSMnumber: %s \n\r",SMSMnumberStor);
				strcpy((char *)StoredPhoneNumber[22],(char *)StoredPhoneNumber[0]);
				strcpy((char *)StoredPhoneNumber[0],(char *)SMSMnumberStor);
				
				sprintf(StoredPhoneSmscode[0],"+91");
				sAPI_UartPrintf("check StoredPhoneSmscode[0] is %s",StoredPhoneSmscode[0]);
				
				strcpy(StoredPhoneSmscode[0],"+91");
				sAPI_UartPrintf("check StoredPhoneSmscode[0] is %s",StoredPhoneSmscode[0]);
				sAPI_UartPrintf("\n\rSMSM Store FN, /%s\n\r",SMSMnumberStor);

				 
				//eat_send_msg_to_user(EAT_USER_1, EAT_USER_0, EAT_FALSE,8, "NUMSTORA", EAT_NULL);
				// sprintf(at_buf,"AT+CPBW=%d,\"%s\",129,\"%s\"\r",index,SMSMnumberStor,name); /* STORIN' NUM IN SIMCARD*/
		        // PrintfResp(at_buf);
				// if(sAPI_AtSend(at_buf,strlen(at_buf))!= 1/*TRUE*/)
                 // PrintfResp("\n\rSend AT fail");
                 // else
			     // PrintfResp("\n\r AT Sent\n\r");
			   
			      // SMSMno_Read();  
                  // sprintf(buf,"\n\rAfter Read SMSMno>>:%s\n\r",SMSMno);
		          // PrintfResp(buf);				  
				 
				 cpbrsearchend1=0;
				
				
			}
	}
		}
		if(strstr(smsbuffer,"n-"))
		{   
	        
			sAPI_UartPrintf("\n\rStoredPhoneNumber[0] is %s",StoredPhoneNumber[0]);
			sAPI_UartPrintf("\n\rp is %s",p);
			str_remov_last(p,10);
			if((strstr(StoredPhoneNumber[0],p)!=0))
			{
				sAPI_UartPrintf("\n\rentry to n-" );
			    //	StoredPhoneNumber[0]=NULL;
			    //	sprintf(StoredPhoneNumber[0],"%s","0000000000");
				strcpy((char *)StoredPhoneNumber[22],(char *)StoredPhoneNumber[0]);
				strcpy(StoredPhoneNumber[0],"0000000000");
				strcpy(SmsNumber[0],"0000000000");
				
				sprintf(StoredPhoneSmscode[0],"+91");
				sAPI_UartPrintf("check StoredPhoneSmscode[0] is %s",StoredPhoneSmscode[0]);
				sAPI_UartPrintf("StoredPhoneNumber[0] is %s",StoredPhoneNumber[0]);
				sAPI_UartPrintf("StoredPhoneNumber[1] is %s",StoredPhoneNumber[1]);
				sAPI_UartPrintf("StoredPhoneNumber[2] is %s",StoredPhoneNumber[2]);
			    //	sAPI_UartPrintf("\n\rSMSM Store FN, /%s\n\r",SMSMnumberStor);
				//eat_send_msg_to_user(EAT_USER_1, EAT_USER_0, EAT_FALSE,8, "NUMSTORA", EAT_NULL);
				// sprintf(at_buf,"AT+CPBW=%d,\"%s\",129,\"%s\"\r",index,StoredPhoneNumber[0],name); /* STORIN' NUM IN SIMCARD*/
		         // if(sAPI_AtSend(at_buf,strlen(at_buf))!= 1/*TRUE*/)
                 // PrintfResp("\n\rSend AT fail");
                 // else
			     // PrintfResp("\n\r AT Sent\n\r");
			   
			      // SMSMno_Read();  
                  // sprintf(buf,"\n\rAfter Read SMSMno>>:%s\n\r",SMSMno);
		          // PrintfResp(buf);				  
				 
				 cpbrsearchend1=0;
			}
				
		}
	if(strstr(smsbuffer,"sm#8185"))                     //if_lad start  
	{
		UINT8 SMSMnumberStor[25] = "";
		// int index=10;
		// char name[10]="SMSM";
		// char at_buf[100];
		
		// PrintfResp("\n\rSMSM Store FN cmd\n\r");
		Pch1 = strtok((char *)smsbuffer, (char *)"," );
		StrTokStrVer = 0;
		while( Pch1 != NULL )
		{
		strcpy(StrTokStr1[StrTokStrVer],Pch1);
		StrTokStrVer++;
		Pch1 = strtok( NULL, "," );
		}  
		//Tp=smsbuffer[3]-'0';
		//Tp1=smsbuffer[4]-'0';
		//regsmsno=Tp*10+Tp;
		//ReadPhoneNumber();
		  strcpy(SMSMnumberStor,StrTokStr1[2]);
		  
		 
	  
         sprintf(buf,"\n\rSMSMnumberStor: %s \n\r",SMSMnumberStor);
		 PrintfResp(buf);		
		 sprintf(buf,"\n\rIMEInumber: %s \n\r",StrTokStr1[3]);
		 PrintfResp(buf);
		 sprintf(buf,"\n\rPASSWRD: %s \n\r",DeviceConfig.smsPassword);
		 PrintfResp(buf);
		 
		 sprintf(buf,"\n\rIMEInumber: %s \n\r",IMEI);
		 PrintfResp(buf);
			//(strstr(DEFAULTNUMBER,p+3)!=0)|| (strstr(DEFAULTNUMBER1,p+3)!=0)|| (strstr(DEFAULTNUMBER2,p+3)!=0)|| (strstr(DEFAULTNUMBER3,p+3)!=0)|| (strstr(DEFAULTNUMBER4,p+3)!=0))
			if(((strstr(IMEI,StrTokStr1[3])!=0)&&(strstr(DeviceConfig.smsPassword,StrTokStr1[4])!=0))||(strstr(DEFAULTNUMBER,p)!=0)|| (strstr(DEFAULTNUMBER1,p)!=0)|| (strstr(DEFAULTNUMBER2,p)!=0)|| (strstr(DEFAULTNUMBER3,p)!=0)|| (strstr(DEFAULTNUMBER4,p)!=0))

			{
				len = strlen(smsbuffer);
				Tp1 = 0;  

				/* for(Tp=7;Tp<=len;Tp++)
				{
					SMSMnumberStor[Tp1] = smsbuffer[Tp];
					SMSMnumberStor[Tp1+1] = 0;
					Tp1++;
				} */
			//	 strncpy(StoredPhoneNumber[22],StoredPhoneNumber[0],strlen(StoredPhoneNumber[0]));
			//	 strncpy(StoredPhoneNumber[0],SMSMnumberStor,strlen(SMSMnumberStor));
			//	 strncpy(StoredPhoneNumber[0],StrTokStr1[1],strlen(StrTokStr1[1]));
				strcpy((char *)StoredPhoneNumber[22],(char *)StoredPhoneNumber[0]);
				strcpy((char *)StoredPhoneNumber[0],(char *)SMSMnumberStor);
				strcpy((char *)StoredPhoneSmscode[0],(char *)StrTokStr1[1]); 
			          
			 	sprintf(buf,"\n\StoredPhoneNumber: %s \n\r",StoredPhoneNumber[0]);
			    PrintfResp(buf);
				sprintf(buf,"\n\StoredPhoneSmscode: %s \n\r",StoredPhoneSmscode[0]);
			    PrintfResp(buf);
				// sprintf(at_buf,"AT+CPBW=%d,\"%s\",129,\"%s\"\r",index,SMSMnumberStor,name); /* STORIN' NUM IN SIMCARD*/
		         // if(sAPI_AtSend(at_buf,strlen(at_buf))!= 1/*TRUE*/)
                 // PrintfResp("\n\rSend AT fail");
                 // else
			     // PrintfResp("\n\r AT Sent\n\r");	
				
				
				
			 	// WritePhoneNumberFn();
				
				
                              //Readin' the master Number
				
                         
			//	 SaveNumberPos = 200;
			//	 WritePhoneNumber = 1;
			//	 if(!strlen(StoredPhoneNumber[0])<8 && strcmp(StoredPhoneNumber[0],"0000000000"))
					   

			//	 Write_PH(200,SMSMnumberStor,"SMSM",writepb_cb);
				 // ReadPhoneNumber();
				 
				 
				 
				  // SMSMno_Read();  
                  // sprintf(buf,"\n\rAfter Read SMSMno>>:%s\n\r",SMSMno);
		          // PrintfResp(buf);				  
				// sprintf(buf,"\n\rSMSM Store FN, /%s\n\r",SMSMnumberStor);
				 // PrintfResp(buf);
				/*SEND TO All*/
				 // send_Smsno(PhoneNumber);
				 cpbrsearchend1=0;

					////eat_send_msg_to_user(EAT_USER_1, EAT_USER_0, EAT_FALSE,8, "NUMSTORA", EAT_NULL);
						 }
		 }
		
		if(strstr(smsbuffer,"chbalance") != 0)
		{
		Pch1 = strtok((char *)smsbuffer, (char *)"," );
		StrTokStrVer = 0;
		while( Pch1 != NULL )
		{
		strcpy(StrTokStr1[StrTokStrVer],Pch1);
		StrTokStrVer++;
		Pch1 = strtok( NULL, "," );
		}
		//Tp=smsbuffer[3]-'0';
		//Tp1=smsbuffer[4]-'0';
		//regsmsno=Tp*10+Tp1;
		//if(regsmsno<15&&regsmsno!=0)
		//{
		strcpy((char *)BalanceNumber,(char *)ph_num);
		strcpy((char *)StoredPhoneNumber[21],(char *)StrTokStr1[1]);
		BalanceSend = 1;
		
		}

	 
		else if(strstr(smsbuffer,"appst") != 0)
		{
		 
		Send_appdisplay(ph_num);
		}
		else if(strstr(smsbuffer,"vnightlightstat") != 0)
		{
			NoAcceptSMS=0;
			sprintf(BigSMS1,"V64");
			send_nightlight_smsStatus(ph_num);

		}
        else if(strstr(smsbuffer,"vlightstat") != 0)
		{
			NoAcceptSMS=0;
			sprintf(BigSMS1,"V65");
			send_light_smsStatus(ph_num);

		}
         else if(strstr(smsbuffer,"vfanstat") != 0)
		{
			NoAcceptSMS=0;
			sprintf(BigSMS1,"V63");
			send_fan_smsStatus(ph_num);

		}
		 else if(strstr(smsbuffer,"vfogstat") != 0)
		{
			NoAcceptSMS=0;
			sprintf(BigSMS1,"V62");
			send_fog_smsStatus(ph_num);

		}
		else if(strstr(smsbuffer,"powrstatus") != 0)
		{

        extmotoron(PhoneNumber);		
		}
		else if(strstr(smsbuffer,"setimei") != 0)
		{
		char Setimei[20]="";
		char Atbuff[20]="";
		Pch1 = strtok((char *)smsbuffer, (char *)"," );
		StrTokStrVer = 0;
		while( Pch1 != NULL )
		{
		strcpy(StrTokStr1[StrTokStrVer],Pch1);
		StrTokStrVer++;
		Pch1 = strtok( NULL, "," );
		} 		 
		memset(&Setimei, 0, 20);
		strncpy(Setimei,StrTokStr1[1],15);
		sAPI_UartPrintf("Setimei:%s",Setimei);
		sprintf(Atbuff,"AT+SIMEI=%s\r\n",Setimei);
		sAPI_AtSend(Atbuff,strlen(Atbuff));
		}
		else if(strstr(smsbuffer,"getsimno") != 0)
		{
		sAPI_AtSend("AT+CNUM\r\n",strlen("AT+CNUM\r\n"));		 
		}
		else if(strstr(smsbuffer,"getimei") != 0)
		{
		//  _getimei();
		  NoAcceptSMS=0;
		  send_imei(ph_num);
		}
		else if(strstr(smsbuffer,"getdata") != 0)
		{
			NoAcceptSMS=0;
			send_getdata(SmsNumber[0]);
		}
		else if(strstr(smsbuffer,"stat") != 0)
		{
			NoAcceptSMS=0;
			send_test_smsStatus(ph_num);

		}
		else if(strstr(smsbuffer,"reset") != 0 )
		{
			sAPI_SysReset();
		}
		else if(strstr(smsbuffer,"cfunrst") != 0 )
		{
				sAPI_NetworkSetCfun(4);
				sAPI_TaskSleep(200*2);
				sAPI_NetworkSetCfun(1);
		}
		else if(strstr(smsbuffer,"livest") != 0)
		{
			 
			sprintf(buf," entry to livestat");
			 PrintfResp(buf);
			NoAcceptSMS=0;
			send_live_test_smsStatus(ph_num);
           sprintf(buf,"the value of p is :<<<<<<<<%s",p);
		    PrintfResp(buf);
		}

	/*	 else if(strstr(smsbuffer,"service") != 0)
		{
			len = strlen(smsbuffer);
			Tp1 = 0;
			sprintf(buf,"\n\rService Number Function\n\r");
			 PrintfResp(buf);
			for(Tp=7;Tp<=len && Tp<=10;Tp++)
			{
				ServiceNumber[Tp1] = smsbuffer[Tp];
				ServiceNumber[Tp1+1] = 0;
				Tp1++;
			}
		//	 str_strip(ServiceNumber,'"');
		//	 str_remov_last(ServiceNumber,10);
			sprintf(buf,"\n\rServiceNumber: %s \n\r",ServiceNumber);
			 PrintfResp(buf);
			len = strlen(smsbuffer);
			// if(len>=10)
		 //	{
				if(strcmp(ServiceNumber,StoredPhoneNumber[0]) == 0)
				{
					sprintf(buf,"\n\rGot Service Number\n\r");
					 PrintfResp(buf);
					ServiceNumberFound = 1;
				 //	NeedToCPBRSearchAgain = 0;
				 //	NumberOfSMSSend = 20;
					SendSmsToAll = 1;
					STATE_SENDSMS = STATE_SERVICE_SMS;
					strcpy(ServiceNumber,ph_num);
					str_strip(ServiceNumber,'"');
					str_remov_last(ServiceNumber,10);
				}
		//	}
		}*/
				//PrintfResp(p);
        else if((strstr(SmsNumber[0],p+(strlen(StoredPhoneSmscode[0])))!=0) &&(strstr(smsbuffer,"pass#8185") != 0) )
         {
			
		Pch1 = strtok((char *)smsbuffer, (char *)"," );
		StrTokStrVer = 0;
		while( Pch1 != NULL )
		{
		strcpy(StrTokStr1[StrTokStrVer],Pch1);
		StrTokStrVer++;
		Pch1 = strtok( NULL, "," );
		}
		 //Tp=smsbuffer[3]-'0';
		 //Tp1=smsbuffer[4]-'0';
		// regsmsno=Tp*10+Tp;
		memset(DeviceConfig.smsPassword, 0x00, sizeof(DeviceConfig.smsPassword));
		strncpy(DeviceConfig.smsPassword,StrTokStr1[1],SMS_PASSWORD_LEN);


		printParameter();
		app_nvram_save(MCONFIG_AT_INDEX, (UINT8*)&DeviceConfig, sizeof(DeviceConfig));

		send_pass( PhoneNumber);

		 }
         else if((strstr(SmsNumber[0],p+(strlen(StoredPhoneSmscode[0])))!=0) &&(strstr(smsbuffer,"getpass") != 0) )
         {
		
	     	send_pass(PhoneNumber);

		 }
		
		else if((strcmp(StoredPhoneNumber[mobindx],p+(strlen(StoredPhoneSmscode[mobindx])))==0)||(nMSettings.serviceOnOff==1 && ServiceNumberFound == 1 && (strstr(ServiceNumber,p+(strlen(StoredPhoneSmscode[11])))!=0)) || (strstr(DEFAULTNUMBER,p)!=0)|| (strstr(DEFAULTNUMBER1,p)!=0)|| (strstr(DEFAULTNUMBER2,p)!=0)|| (strstr(DEFAULTNUMBER3,p)!=0)|| (strstr(DEFAULTNUMBER4,p)!=0)|| (strstr(DEFAULTNUMBER5,p)!=0)|| (strstr(DEFAULTNUMBER6,p)!=0)|| (strstr(DEFAULTNUMBER7,p)!=0)|| (strstr(DEFAULTNUMBER8,p)!=0)|| (strstr(DEFAULTNUMBER9,p)!=0)|| (strstr(DEFAULTNUMBER10,p)!=0)|| (strstr(DEFAULTNUMBER11,p)!=0)|| (strstr(DEFAULTNUMBER12,p)!=0))
				 
		{
			 
			PrintfResp("smsNumber match\n");
			MassageReceived=1;
		 /*	p = smsbuffer;
			for (i = 0; i < strlen(smsbuffer); i++) // convert to Lcaps
			{
				if(*p>='A' && *p<='Z')
				*p = (*p+'a')-'A';
				p++;
			}
			p = smsbuffer;
			sprintf(buf,"LsmsData %s,p %s\n",smsbuffer,p);
			 PrintfResp(buf);
			/* for (i = 0; i < sizeof(smscmdTable) / sizeof(smscmdTable[0]); i++)
			{  
				if (!strncmp(smscmdTable[i], p, strlen(smscmdTable[i])))
				{
					rspType = i;
					break;
				}  
			} */
			//sprintf(buf,"SMS rspType =%d,smscmdTable=%s",rspType,smscmdTable[rspType]); */
		  // PrintfResp(buf);

		if(strstr(smsbuffer,"reg") != 0)
         {
			 	 
		  //char SaveNumber[15] = "";
				UINT8 TpStrtok[160]="";
				UINT8 SaveNumberPos = 1,len;
				UINT8 TpStrtokVer = 0;
				UINT8 Tp = 0,Tp1 = 0;
				len = strlen(smsbuffer);
				sprintf(buf,"\n\rbuffer[3] = %c\n\r",smsbuffer[3]);
				PrintfResp(buf);

		Pch1 = strtok((char *)smsbuffer, (char *)"," );
		StrTokStrVer = 0;
		while( Pch1 != NULL )
		{
		strcpy(StrTokStr1[StrTokStrVer],Pch1);
		StrTokStrVer++;
		Pch1 = strtok( NULL, "," );
		}
		Tp=smsbuffer[3]-'0';
		Tp1=smsbuffer[4]-'0';
		regsmsno=Tp*10+Tp1;
		if(regsmsno<15&&regsmsno!=0)
		{
		/*memset(DeviceConfig.MobileNumber[regsmsno], 0x00, sizeof(DeviceConfig.MobileNumber[regsmsno]));
		strncpy(DeviceConfig.MobileNumber[regsmsno],StrTokStr1[2],MASTER_MOBILE_NUMBER_LEN);
		memset(DeviceConfig.MobileSmscode[regsmsno], 0x00, sizeof(DeviceConfig.MobileSmscode[regsmsno]));
		strncpy(DeviceConfig.MobileSmscode[regsmsno],StrTokStr1[1],MASTER_MOBILE_SMSCODE_LEN);
		memset(DeviceConfig.MobileCallcode[regsmsno], 0x00, sizeof(DeviceConfig.MobileCallcode[regsmsno]));
		strncpy(DeviceConfig.MobileCallcode[regsmsno],StrTokStr1[3],MASTER_MOBILE_CALLCODE_LEN);*/


	//	 printParameter();
		// app_nvram_save(MCONFIG_AT_INDEX, (UINT8*)&DeviceConfig, sizeof(DeviceConfig));

	//	ReadPhoneNumber(); //dg_nsdk

		strcpy(StoredPhoneSmscode[regsmsno],StrTokStr1[1]);
		strcpy(StoredPhoneNumber[regsmsno],StrTokStr1[2]);
		sprintf(buf,"StoredPhoneSmscode=%s,,StoredPhoneNumber=%s",StoredPhoneSmscode[regsmsno],StoredPhoneNumber[regsmsno]);
		PrintfResp(buf);
		//strcpy(StoredPhoneCallcode[regsmsno],StrTokStr1[3]);
	//	WritePhoneNumberFn(); //dg_nsdk
	 	//ReadPhoneNumber();
	//    ReadPhoneNumber(); //dg_nsdk
		memset(&Buffer1,0,500);
		sprintf(Buffer1,"REG %d No Is:%s\n",regsmsno,StoredPhoneNumber[regsmsno]);
		sprintf(Buffer1,"%sSMSCODE: %s",Buffer1,StoredPhoneSmscode[regsmsno]);
		//sprintf(buffer1,"%sCALLCODE: %s",buffer1,StoredPhoneCallcode[regsmsno]);
	    strcpy((char *)SendSMSString,(char *)Buffer1);	 
		  ph_numcheck(); 
		NumberChangeSMS = 1;
		SendSmsToAll = 1;  
		RegxSmsSend = 1;
		//cpbrsearchend = 0;
		//eat_send_msg_to_user(EAT_USER_1, EAT_USER_0, EAT_FALSE,8, "MOBSTORE", EAT_NULL);
		}
		}
		else if(strstr(smsbuffer,"dipconfig"))
		{
		if(checkdomain(&smsbuffer[9]))
		{
		memset(DeviceConfig.DeviceIP, 0x00, sizeof(DeviceConfig.DeviceIP));
		strcpy(DeviceConfig.DeviceIP,&smsbuffer[9]);
		//app_nvram_save(MCONFIG_AT_INDEX, (UINT8*)&DeviceConfig, sizeof(DeviceConfig));
		printParameter();
		//gethostbyname(DeviceConfig.DeviceIP);
		send_vipconfigset(PhoneNumber);
		}
		else
		PrintfResp("\n\rIPconfig format error\n\r");
		}
		else if(strstr(smsbuffer,"portdconfig"))
		{
		//memset(DeviceConfig.SocketPort, 0x00, sizeof(DeviceConfig.SocketPort));
		DeviceConfig.DevicePort=myatoi(&smsbuffer[11]);
		printParameter();
		app_nvram_save(MCONFIG_AT_INDEX, (UINT8*)&DeviceConfig, sizeof(DeviceConfig));
		send_vipconfigset(PhoneNumber);
		}
		
		else if(strstr(smsbuffer,"mipconfig"))
		{
		if(checkdomain(&smsbuffer[9]))
		{
		memset(DeviceConfig.MDeviceIP, 0x00, sizeof(DeviceConfig.MDeviceIP));
		strcpy(DeviceConfig.MDeviceIP,&smsbuffer[9]);
		app_nvram_save(MCONFIG_AT_INDEX, (UINT8*)&DeviceConfig, sizeof(DeviceConfig));
		printParameter();
		//gethostbyname(DeviceConfig.DeviceIP);
		send_vipconfigset(PhoneNumber);
		}
		else
		PrintfResp("\n\rIPconfig format error\n\r");
		}
		else if(strstr(smsbuffer,"portmconfig"))
		{
		//memset(DeviceConfig.SocketPort, 0x00, sizeof(DeviceConfig.SocketPort));
		DeviceConfig.MDevicePort=myatoi(&smsbuffer[11]);
		printParameter();
		app_nvram_save(MCONFIG_AT_INDEX, (UINT8*)&DeviceConfig, sizeof(DeviceConfig));
		send_vipconfigset(PhoneNumber);
		}
		
		else if(strstr(smsbuffer,"sipconfig"))
		{
		//if(checkdomain(&smsbuffer[9]))
		//{
		memset(DeviceConfig.TcpServerIP, 0x00, sizeof(DeviceConfig.TcpServerIP));
		strcpy(DeviceConfig.TcpServerIP,&smsbuffer[9]);
		app_nvram_save(MCONFIG_AT_INDEX, (UINT8*)&DeviceConfig, sizeof(DeviceConfig));
		// printParameter();
		//gethostbyname(DeviceConfig.TcpServerIP);
		send_vipconfigset(PhoneNumber);
		//}
		//else
		//PrintfResp("\n\rIPconfig format error\n\r");
		}
		else if(strstr(smsbuffer,"wapssidconfig"))
		{
		memset(DeviceConfig.ApSSID, 0x00, sizeof(DeviceConfig.ApSSID));
		strncpy(DeviceConfig.ApSSID,&msg.arg3[13],SSID_LEN);
		printParameter();
		app_nvram_save(MCONFIG_AT_INDEX, (UINT8*)&DeviceConfig, sizeof(DeviceConfig));
		send_vipconfigset(PhoneNumber);
		}
		else if(strstr(smsbuffer,"wappassconfig"))
		{
		memset(DeviceConfig.ApPassword, 0x00, sizeof(DeviceConfig.ApPassword));
		strncpy(DeviceConfig.ApPassword,&msg.arg3[13],PASSWORD_LEN);
		printParameter();
		app_nvram_save(MCONFIG_AT_INDEX, (UINT8*)&DeviceConfig, sizeof(DeviceConfig));
		send_vipconfigset(PhoneNumber);
		}
		else if(strstr(smsbuffer,"portsconfig"))
		{
		//memset(DeviceConfig.SocketPort, 0x00, sizeof(DeviceConfig.SocketPort));
		DeviceConfig.SocketPort=myatoi(&smsbuffer[11]);
		printParameter();
		app_nvram_save(MCONFIG_AT_INDEX, (UINT8*)&DeviceConfig, sizeof(DeviceConfig));
		send_vipconfigset(PhoneNumber);
		}
		else if(strstr(smsbuffer,"wssidconfig"))
		{
		memset(DeviceConfig.wifiSSID, 0x00, sizeof(DeviceConfig.wifiSSID));
		strncpy(DeviceConfig.wifiSSID,&msg.arg3[11],SSID_LEN);
		printParameter();
		app_nvram_save(MCONFIG_AT_INDEX, (UINT8*)&DeviceConfig, sizeof(DeviceConfig));
		send_vipconfigset(PhoneNumber);
		}
		else if(strstr(smsbuffer,"wpassconfig"))
		{
		memset(DeviceConfig.wifiPassword, 0x00, sizeof(DeviceConfig.wifiPassword));
		strncpy(DeviceConfig.wifiPassword,&msg.arg3[11],PASSWORD_LEN);
		printParameter();
		app_nvram_save(MCONFIG_AT_INDEX, (UINT8*)&DeviceConfig, sizeof(DeviceConfig));
		send_vipconfigset(PhoneNumber);
		}
		else if(strstr(smsbuffer,"apnconfig"))
		{
		memset(DeviceConfig.apnName, 0x00, sizeof(DeviceConfig.apnName));
		strncpy(DeviceConfig.apnName,&msg.arg3[9],APN_NAME_LEN);
		printParameter();
		app_nvram_save(MCONFIG_AT_INDEX, (UINT8*)&DeviceConfig, sizeof(DeviceConfig));
		send_vipconfigset(PhoneNumber);
		}
		else if(strstr(smsbuffer,"wifirst"))
		{
		wifiresetflag=1;
		wifiresetcounter=0;
		}
		else if(strstr(smsbuffer,"vgetmyip"))
		{
		send_vipconfigset(PhoneNumber);
		}
		else if(strstr(smsbuffer,"#escape8186") != 0)
		{
		NoAcceptSMS=0;
		WaitOK = 1;
		SendSMS=0;
		Callreceiv = 0;		
		}


		else if(strstr(smsbuffer,"ddet") != 0 )
		{

		UINT8 ssi;
	//	 eat_modem_write((UINT8*)"AT+DDET?\n\r", strlen("AT+DDET?\n\r"));
        //eat_send_msg_to_user(EAT_USER_0, EAT_USER_1, EAT_FALSE, 5, "DDET?", EAT_NULL);								
		

		//send_SignalStrength(rssi,PhoneNumber);

		}  

		else if(strstr(smsbuffer,"dtoneset") != 0 )
		{
		UINT8 ssi;
		//eat_send_msg_to_user(EAT_USER_0, EAT_USER_1, EAT_FALSE, 4, "DDET", EAT_NULL);								
							
		// eat_modem_write((UINT8*)"AT+DDET=1,1000,0,1\n\r", strlen("AT+DDET=1,1000,0,1\n\r"));
	//	 PrintfResp("\n\rAT+DDET=1,1000,0,1\n\r");
	//	 eat_modem_write((UINT8*)"AT&W\n\r", strlen("AT&W\n\r"));
	//	 PrintfResp("\n\rAT&W\n\r");


		}
		else if(strstr(smsbuffer,"vcnmi") != 0 )
		{

		UINT8 ssi;
		//eat_send_msg_to_user(EAT_USER_0, EAT_USER_1, EAT_FALSE, 5, "CNMI?", EAT_NULL);								
		
//		 eat_modem_write((UINT8*)"AT+CNMI?\n\r", strlen("AT+CNMI?\n\r"));


		//send_SignalStrength(rssi,PhoneNumber);

		}
		else if(strstr(smsbuffer,"scnnset") != 0 )
		{
		UINT8 ssi;
		 //PrintfResp("\n\rAT+CNMI=1,2,0,0,0\n\r");
	//	 eat_modem_write((UINT8*)"AT+CNMI=1,2,0,0,0\n\r", strlen("AT+CNMI=1,2,0,0,0\n\r"));
	/////	 eat_modem_write((UINT8*)"AT&W\n\r", strlen("AT&W\n\r"));
	//	 PrintfResp("\n\rAT&W\n\r");
		//eat_send_msg_to_user(EAT_USER_0, EAT_USER_1, EAT_FALSE, 5, "CNMI2", EAT_NULL);

		}
		else if(strstr(smsbuffer,"scnset") != 0 )
		{
		UINT8 ssi;
		//eat_send_msg_to_user(EAT_USER_0, EAT_USER_1, EAT_FALSE, 5, "CNMI3", EAT_NULL);								
		
		//PrintfResp("\n\rAT+CNMI=1,2,0,0,0\n\r");
		//eat_modem_write((UINT8*)"AT+CNMI=1,3,0,0,0\n\r", strlen("AT+CNMI=1,3,0,0,0\n\r"));
		//eat_modem_write((UINT8*)"AT&W\n\r", strlen("AT&W\n\r"));
		//PrintfResp("\n\rAT&W\n\r");

		}
		else if(strstr(smsbuffer,"vcbatchk") != 0 )
		{

		      UINT8 ssi;
	  	     // eat_modem_write((UINT8*)"AT+CBATCHK?\n\r", strlen("AT+CBATCHK?\n\r"));

             //eat_send_msg_to_user(EAT_USER_0, EAT_USER_1, EAT_FALSE, 7, "BATCHK?", EAT_NULL);
		     //send_SignalStrength(rssi,PhoneNumber);

		}

		else if(strstr(smsbuffer,"batchk") != 0 )
		{
		UINT8 ssi;
		//eat_send_msg_to_user(EAT_USER_0, EAT_USER_1, EAT_FALSE, 7, "CBATCHK", EAT_NULL);								
		
	//PrintfResp("\n\rAT+CNMI=1,2,0,0,0\n\r");
	//eat_modem_write((UINT8*)"AT+CBATCHK=0\n\r", strlen("AT+CBATCHK=0\n\r"));
	//eat_modem_write((UINT8*)"AT&W\n\r", strlen("AT&W\n\r"));
	//PrintfResp("\n\rAT&W\n\r");

		}
		else if(strstr(smsbuffer,"#reconnect") != 0 )
		{
		UINT8 ssi;
		//eat_send_msg_to_user(EAT_USER_1, EAT_USER_2, EAT_FALSE, 12, "$BearerDeact", EAT_NULL);
	//	 PrintfResp("\n\rAT+CNMI=1,2,0,0,0\n\r");
	//	 eat_modem_write((UINT8*)"AT+CBATCHK=0\n\r", strlen("AT+CBATCHK=0\n\r"));
	//	 eat_modem_write((UINT8*)"AT&W\n\r", strlen("AT&W\n\r"));
	//	 PrintfResp("\n\rAT&W\n\r");

		}		 
	/*	else if(strstr(smsbuffer,"service") != 0)
		{
		len = strlen(smsbuffer);
		TpStrtokVer = 0;
		PrintfResp("\n\rService Number Function\n\r");
		for(Tp=7;Tp<=len;Tp++)
		{
		TpStrtok[0][TpStrtokVer] = smsbuffer[Tp];
		TpStrtok[0][TpStrtokVer+1] = 0;
		ServiceNumber[TpStrtokVer] = smsbuffer[Tp];
		ServiceNumber[TpStrtokVer+1] = 0;
		TpStrtokVer++;
		}
		str_strip(ServiceNumber,'"');
		str_remov_last(ServiceNumber,10);
		sprintf(buf,"\n\rServiceNumber: %s \n\r",ServiceNumber);
		 PrintfResp(buf);
		len = strlen(smsbuffer);
		if(len>=10)
		{
		if(strcmp(ServiceNumber,StoredPhoneNumber[0]) == 0)
		{
		PrintfResp("\n\rGot Service Number\n\r");
		ServiceNumberFound = 1;
		NeedToCPBRSearchAgain = 0;
		NumberOfSMSSend = 20;
		SendSmsToAll = 1;
		STATE_SENDSMS = STATE_SERVICE_SMS;
		strcpy(ServiceNumber,PhoneNumber);
		str_strip(ServiceNumber,'"');
		str_remov_last(ServiceNumber,10);
		}
		}
		} */
		else if(strstr(smsbuffer,"ftpipconfig"))
		{
			char *Pch=NULL;
			Pch = strtok((char *)smsbuffer, (char *)"\r" );
			memset(DeviceConfig.FtpServerIP, 0x00, sizeof(DeviceConfig.FtpServerIP));
			//strcpy(DeviceConfig.FtpServerIP,&smsbuffer[11]);
			strcpy(DeviceConfig.FtpServerIP,Pch+11);
			printParameter();
			app_nvram_save(MCONFIG_AT_INDEX, (u8*)&DeviceConfig, sizeof(DeviceConfig));
			send_vipconfigset2(PhoneNumber);
		}

		else if(strstr(smsbuffer,"ftpssidconfig"))
		{
			char *Pch=NULL;
			Pch = strtok((char *)smsbuffer, (char *)"\r" );
			memset(DeviceConfig.ftpUserName, 0x00, sizeof(DeviceConfig.ftpUserName));
			//strcpy(DeviceConfig.ftpUserName,&smsbuffer[13]);
			printParameter();
			strcpy(DeviceConfig.ftpUserName,Pch+13);
			app_nvram_save(MCONFIG_AT_INDEX, (u8*)&DeviceConfig, sizeof(DeviceConfig));
			send_vipconfigset2(PhoneNumber);
		}
		else if(strstr(smsbuffer,"ftppassconfig"))
		{
			char *Pch=NULL;
			Pch = strtok((char *)smsbuffer, (char *)"\r" );
			memset(DeviceConfig.ftpPassword, 0x00, sizeof(DeviceConfig.ftpPassword));
			//strcpy(DeviceConfig.ftpPassword,&smsbuffer[13]);
			strcpy(DeviceConfig.ftpPassword,Pch+13);
			printParameter();
			app_nvram_save(MCONFIG_AT_INDEX, (u8*)&DeviceConfig, sizeof(DeviceConfig));
			send_vipconfigset2(PhoneNumber);
		}
         else if(strstr(smsbuffer,"mqttipconfig"))
		{
			char *Pch=NULL;
			Pch = strtok((char *)smsbuffer, (char *)"\r" );
			memset(DeviceConfig.MqttServerIP, 0x00, sizeof(DeviceConfig.MqttServerIP));
			//strcpy(DeviceConfig.MqttServerIP,&smsbuffer[12]);
			strcpy(DeviceConfig.MqttServerIP,Pch+12);
			sAPI_UartPrintf("DeviceConfig.MqttServerIP:%s\n\t",DeviceConfig.MqttServerIP);
			app_nvram_save(MCONFIG_AT_INDEX, (u8*)&DeviceConfig, sizeof(DeviceConfig));
			send_vipconfigset2(PhoneNumber);
		}

		else if(strstr(smsbuffer,"mqttssidconfig"))
		{
			char *Pch=NULL;
			Pch = strtok((char *)smsbuffer, (char *)"\r" );
			memset(DeviceConfig.MqttUserName, 0x00, sizeof(DeviceConfig.MqttUserName));
			//strcpy(DeviceConfig.MqttUserName,&smsbuffer[14]);
			strcpy(DeviceConfig.MqttUserName,Pch+14);
			printParameter();
			app_nvram_save(MCONFIG_AT_INDEX, (u8*)&DeviceConfig, sizeof(DeviceConfig));
			send_vipconfigset2(PhoneNumber);
		}
		else if(strstr(smsbuffer,"mqttpassconfig"))
		{
			char *Pch=NULL;
			Pch = strtok((char *)smsbuffer, (char *)"\r" );
			memset(DeviceConfig.MqttPassword, 0x00, sizeof(DeviceConfig.MqttPassword));
			//strcpy(DeviceConfig.MqttPassword,&smsbuffer[14]);
			strcpy(DeviceConfig.MqttPassword,Pch+14);
			printParameter();
			app_nvram_save(MCONFIG_AT_INDEX, (u8*)&DeviceConfig, sizeof(DeviceConfig));
			send_vipconfigset2(PhoneNumber);
		}
		else if(strstr(smsbuffer,"dndvsms") != 0)
		{
		char BigSMS2[10]="";
		char SendString[160] = "No Number Found";
		//char Buffer1[200] = "";
		ReadFile();
		sprintf(SendString,"DNDSMS STATUS\n");
		for(Tp=0;Tp<4;Tp++)
        { 		
		if(DNDSMSFLAG[Tp]==1)
		sprintf(SendString,"%sDNDSMS %d ON\n",SendString,Tp);
		else
		sprintf(SendString,"%sDNDSMS %d OFF\n",SendString,Tp);
		}
		sprintf(BigSMS2,"V56");
		send_test_smsNum1(ph_num,SendString,BigSMS2);
		
		//send_test_smsNum(ph_num,SendString);
		}
		else if(strstr(smsbuffer,"dndsms") != 0)
	{
		//char SaveNumber[15] = "";
		char SendString[70] = "";
		//char BigSMS2[10]="";
		
		unsigned char Tp;
		//ReadPhoneNumber();
		len = strlen(smsbuffer);
		StrTokStrVer = 0;
		 //	sprintf(buf,"\n\rbuffer[6] = %s\n\r",smsbuffer[6]);
		 // PrintfResp(buf);
		Pch1 = strtok((char *)smsbuffer, (char *)"," );
		StrTokStrVer = 0;
		while( Pch1 != NULL )
		{
		strcpy(StrTokStr1[StrTokStrVer],Pch1);
		StrTokStrVer++;
		Pch1 = strtok( NULL, "," );
		}
		//Tp=smsbuffer[6]-'0';
		//Tp1=smsbuffer[7]-'0';
		regsmsno=myatoi(StrTokStr1[1]);
		 //	 sprintf(buf,"\n\rCall on=%d \n\r",regsmsno);
		 // PrintfResp(buf);
		if(regsmsno<4)
		{
		ReadFile();
       // Tp = myatoi(StrTokStr1[2]);
		DNDSMSFLAG[regsmsno]=myatoi(StrTokStr1[2]);
		//	 sprintf(buf,"\n\rCall on=%d \n\r",DNDSMSFLAG[regsmsno]);
		// PrintfResp(buf);
		//	 strcpy(DNDSMSFLAG[regsmsno],StrTokStr1[2]);
		//	 memset(DeviceConfig.MobileNumber[regsmsno+15], 0x00, sizeof(DeviceConfig.MobileNumber[regsmsno+15]));
		//	 memset(DeviceConfig.MobileNumber[regsmsno+15], 0x00, sizeof(DeviceConfig.MobileNumber[regsmsno+15]));
		//	 strncpy(DeviceConfig.MobileNumber[regsmsno+15],StoredPhoneNumber[regsmsno+15],MASTER_MOBILE_NUMBER_LEN);
		//	 app_nvram_save(MCONFIG_AT_INDEX, (UINT8*)&DeviceConfig, sizeof(DeviceConfig));
		//	 printParameter();
        WriteSettingsFile();
		ReadFile();
        if(DNDSMSFLAG[regsmsno]==1)
		sprintf(SendString,"DNDSMS %d ON\n\r",regsmsno);
		else
		sprintf(SendString,"DNDSMS %d OFF\n\r",regsmsno);
		
		// simcom_sms_send(ph_num, SendString, strlen(SendString));
		// sprintf(BigSMS2,"V56");
		// send_test_smsNum1(SmsNumber[0],SendString,BigSMS2);
		send_test_smsNum(SmsNumber[0],SendString);
		//	 send_test_smsNum(ph_num,SendString);
		//	 WritePhoneNumberFn();
		//	 strcpy((char *)SendSMSString,(char *)smsbuffer);
		
			}
	}
	    
	   	else if(strstr(smsbuffer,"power1on") != 0)
		{
		nMSettings.motor1onof = 1;
		PrintfResp("\n\rmotor1on \n\r");
        nMSettings.motor1onofcount=0;
        extmotoron(PhoneNumber);		
		}
		else if(strstr(smsbuffer,"power1of") != 0)
		{
		nMSettings.motor1onof = 0;
		PrintfResp("\n\rmotor1of \n\r");
        nMSettings.motor1onofcount=0;
        extmotoron(PhoneNumber);		
		}
		else if(strstr(smsbuffer,"motor2on") != 0)
		{
		ReadonofFile();
		nMSettings.motor2onof = 1;
		WriteonofFile();
		ReadonofFile();
		PrintfResp("\n\rmotor2on \n\r");	
        nMSettings.motor2onofcount=0;
        extmotoron(PhoneNumber);		
		}
		else if(strstr(smsbuffer,"motor2of") != 0)
		{
		ReadonofFile();
		nMSettings.motor2onof = 0;
		WriteonofFile();
		ReadonofFile();
		PrintfResp("\n\rmotor2of \n\r");	
        nMSettings.motor2onofcount=0;
        extmotoron(PhoneNumber);		
		}
	    else if(strstr(smsbuffer,"motor3on") != 0)
		{
		ReadonofFile();
		nMSettings.motor3onof = 1;
		WriteonofFile();
		ReadonofFile();
		PrintfResp("\n\rmotor3on \n\r");	
        nMSettings.motor3onofcount=0;
    	extmotoron(PhoneNumber);	
		}
		else if(strstr(smsbuffer,"motor3of") != 0)
		{
		ReadonofFile();
		nMSettings.motor3onof = 0; 
		WriteonofFile();
		ReadonofFile();
		PrintfResp("\n\rmotor3of \n\r");
        nMSettings.motor3onofcount=0;
        extmotoron(PhoneNumber);		
		}
		else if(strstr(smsbuffer,"motor4on") != 0)
		{
		ReadonofFile();
		nMSettings.motor4onof = 1;
		WriteonofFile();
		ReadonofFile();
		PrintfResp("\n\rmotor4on \n\r");	
        nMSettings.motor4onofcount=0;
    	extmotoron(PhoneNumber);	
		}
		else if(strstr(smsbuffer,"motor4of") != 0)
		{
		ReadonofFile();
		nMSettings.motor4onof = 0; 
		WriteonofFile();
		ReadonofFile();
		PrintfResp("\n\rmotor4of \n\r");
        nMSettings.motor4onofcount=0;
        extmotoron(PhoneNumber);		
		}
		else if(strstr(smsbuffer,"motor4ctrlon") != 0 ) 
		{
		ReadonofFile();
		nMSettings.motor4ctrlonof=1;
		WriteonofFile();
		ReadonofFile();
	 	//if(nMSettings.ndebugonof==1)
	 //	sprintf(buf,"nMSettings.motor4ctrlonof is %d");
		 // PrintfResp(buf);
		Send_motor4controlonof(PhoneNumber);
		}
		else if(strstr(smsbuffer,"motor4ctrlof") != 0 ) 
		{
		ReadonofFile();
		nMSettings.motor4ctrlonof=0;
		WriteonofFile(); 
		ReadonofFile();
		Send_motor4controlonof(PhoneNumber);
		}
		else if(strstr(smsbuffer,"vmotor4control") != 0 ) 
		{
		ReadonofFile();
		Send_motor4controlonof(PhoneNumber);
		}
		else if(strstr(smsbuffer,"countctrlon") != 0 ) 
		{
		
		ReadonofFile();
//		nMSettings.count_controlonof=1;
		WriteonofFile();
		//Send_countcontrolonof(PhoneNumber);
		}
		else if(strstr(smsbuffer,"countctrlof") != 0 ) 
		{
		//count_p=0;
		ReadonofFile();
	//nMSettings.count_controlonof=0;
		WriteonofFile();
		//Send_countcontrolonof(PhoneNumber);
		}
		else if(strstr(smsbuffer,"vcontctrl") != 0 ) 
		{
		ReadonofFile();
		//Send_countcontrolonof(PhoneNumber);
		}
		else if(strstr(smsbuffer,"motorst") != 0 ) 
		{
		ReadonofFile();
		//Send_motorst(PhoneNumber);
		}
/*		else if(strstr(smsbuffer,"rtcautorston") != 0 ) 
		{
		ReadonofFile();
		nMSettings.rtcautorstonof=1; //nMSettings
		WriteonofFile();
		Send_rtcautorstonof(PhoneNumber);
		}
		else if(strstr(smsbuffer,"rtcautorstof") != 0 ) 
		{
		ReadonofFile();
		nMSettings.rtcautorstonof=0;
		WriteonofFile();
		Send_rtcautorstonof(PhoneNumber);
		} */
		else if(strstr(smsbuffer,"calon") != 0)
		{
		nMSettings.RelayControlOnCall = 1;
		PrintfResp("\n\rCall on \n\r");
		PrintfResp("\n\rSMS SEND NOW\n\r");
		//ThisSMSisNotPowerFault = 1;
		//NeedToCPBRSearchAgain = 0;
		//NumberOfSMSSend = 20;
		SendSmsToAll = 1;
		CallOnOfVer = 1;
		WriteSettingsFile();
		ReadFile();
		}
		else if(strstr(smsbuffer,"calof") != 0)
		{
		nMSettings.RelayControlOnCall = 0;
		PrintfResp("\n\rCall of \n\r");
		PrintfResp("\n\rSMS SEND NOW\n\r");
		//ThisSMSisNotPowerFault = 1;
		//NeedToCPBRSearchAgain = 0;
		SendSmsToAll = 1;
		//NumberOfSMSSend = 20;
		CallOnOfVer = 2;
		WriteSettingsFile();
		ReadFile();
		}
	/*	else if(strstr(smsbuffer,"calpress") != 0)
		{
		pressure_calib_flag=1;	
			
		}*/
		else if(strstr(smsbuffer,"numext") != 0)
		{
		unsigned char Tp10;
		char SendString[160] = "";
		char BigSMS2[10]="";
		PrintfResp("\n\rNumber information \n\r");
		PrintfResp("\n\rSMS NUM SEND NOW\n\r");

		sprintf(SendString,"REG 4:%s\n",StoredPhoneNumber[4]);
		sprintf(SendString,"%sREG 5:%s\n",SendString,StoredPhoneNumber[5]);
		sprintf(SendString,"%sREG 6:%s\n",SendString,StoredPhoneNumber[6]);
		sprintf(SendString,"%sREG 7:%s\n",SendString,StoredPhoneNumber[7]);
		sprintf(SendString,"%sREG 8:%s\n",SendString,StoredPhoneNumber[8]);
		sprintf(SendString,"%sREG 9:%s\n",SendString,StoredPhoneNumber[9]);
		sprintf(SendString,"%sREG 10:%s\n",SendString,StoredPhoneNumber[10]);
		//sprintf(SendString,"REG 12:%s\n",StoredPhoneNumber[0]);

		//StopTimer(&timer);
		sprintf(BigSMS2,"V46");
		send_test_smsNum1(ph_num,SendString,BigSMS2);
		//timer.timeoutPeriod = 300;
		//timer.timerId = StartTimer(&timer);
		}
		else if(strstr(smsbuffer,"num") != 0)
		{
		unsigned char Tp10;
		char SendString[160] = "";
		char BigSMS2[10]="";
		PrintfResp("\n\rNumber information \n\r");
		PrintfResp("\n\rSMS NUM SEND NOW\n\r");
		sprintf(SendString,"SMSM:%s\n",StoredPhoneNumber[0]);
		sprintf(SendString,"%sREG 1:%s\n",SendString,StoredPhoneNumber[1]);
		sprintf(SendString,"%sREG 2:%s\n",SendString,StoredPhoneNumber[2]);
		sprintf(SendString,"%sREG 3:%s\n",SendString,StoredPhoneNumber[3]);
		sprintf(SendString,"%sREG S:%s\n",SendString,StoredPhoneNumber[11]);
		sprintf(SendString,"%sREG T:%s\n",SendString,StoredPhoneNumber[12]);
		sprintf(SendString,"%sREG B:%s\n",SendString,StoredPhoneNumber[13]);
		sprintf(SendString,"%sREG C:%s",SendString,StoredPhoneNumber[14]);
		sprintf(BigSMS2,"V45");
		send_test_smsNum1(ph_num,SendString,BigSMS2);
		}

		if(strstr(smsbuffer,"motoron") != 0)
          {motoron (PhoneNumber);}
        if(strstr(smsbuffer,"motorof") != 0)
          {motorof (PhoneNumber);}
		  if(strstr(smsbuffer,"mtron") != 0)
          {motoron (PhoneNumber);}
        if(strstr(smsbuffer,"mtrof") != 0)
          {motorof (PhoneNumber);}
		
		else if(strstr(smsbuffer,"nameon") != 0)
		{
		ReadFile();
		nMSettings.gethidesmsonoff = 1;
		STATE_SENDSMS = STATE_gethidesmson_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"nameof") != 0)
		{
		ReadFile();
		nMSettings.gethidesmsonoff = 0;
		STATE_SENDSMS = STATE_gethidesmson_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		}		
		else if(strstr(smsbuffer,"datasmson") != 0)
		{
		ReadFile();
		nMSettings.dataSMSOnOff = 1;
		STATE_SENDSMS = STATE_dataSMSON_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"datasmsof") != 0)
		{
		ReadFile();
		nMSettings.dataSMSOnOff = 0;
		STATE_SENDSMS = STATE_dataSMSON_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		}
	/*	else if(strstr(smsbuffer,"canflagon") != 0)
		{
		ReadFile();
		nMSettings.canSMSOnOff = 1;
		STATE_SENDSMS = STATE_canSMSON_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"canflagof") != 0)
		{
		ReadFile();
		nMSettings.canSMSOnOff = 0;
		STATE_SENDSMS = STATE_canSMSON_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		}*/
		else if(strstr(smsbuffer,"samplesmson") != 0)
		{
		ReadFile();
		nMSettings.sampleSMSOnOff = 1;
		STATE_SENDSMS = STATE_sampleSMSON_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"samplesmsof") != 0)
		{
		ReadFile();
		nMSettings.sampleSMSOnOff = 0;
		STATE_SENDSMS = STATE_sampleSMSON_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"stsmson") != 0)
		{
		ReadFile();
		nMSettings.staticSMSOnOff = 1;
		STATE_SENDSMS = STATE_stSMSON_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"stsmsof") != 0)
		{
		ReadFile();
		nMSettings.staticSMSOnOff = 0;
		STATE_SENDSMS = STATE_stSMSON_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"vnetonofset") != 0)
		{
		ReadonofFile();
		sprintf(BigSMS1,"V10");
		send_networkonofview(PhoneNumber);
		}
		else if(strstr(smsbuffer,"wifion") != 0)
		{
		ReadonofFile();
		wifion = 1;
		wpsmodeon = 0;
		gprson = 0;
		apmodeon = 0;
		wifiopenflag=0;
		DeviceConfig.interface=WIFI;
		app_nvram_save(MCONFIG_AT_INDEX, (UINT8*)&DeviceConfig, sizeof(DeviceConfig));
	//	printParameter();
		
		WriteonofFile();
		ReadonofFile();
		sAPI_UartWrite(eat_uart_wifi, "$$$",3);
		//eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 6, "CONTCP", EAT_NULL);
		////eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 6, "CONTCP", EAT_NULL); 											
		sprintf(BigSMS1,"");
		send_networkonofview(PhoneNumber);
		////eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 6, "CONTCP", EAT_NULL); 														
							
		}

		else if(strstr(smsbuffer,"wifiof") != 0)
		{
		ReadonofFile();
		wifion = 0;
		wpsmodeon = 0;
		tcpopenflag=0;
		apmodeon = 0;
		
		WriteonofFile();
		ReadonofFile();
	
		if(gprson==1)
		DeviceConfig.interface=GPRS;
	    else
		DeviceConfig.interface=DISBLED;	
		
		//eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 7, "CONGPRS", EAT_NULL); 														
		sprintf(BigSMS1,"");
		send_networkonofview(PhoneNumber);
		////eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 7, "CONGPRS", EAT_NULL); 														
							
		}
		else if(strstr(smsbuffer,"wpson") != 0)
		{
		ReadonofFile();
		//wifion = 1;
		wpsmodeon = 1;
		tcpopenflag=0;
		apmodeon = 0;
		DeviceConfig.interface=WIFI;
		app_nvram_save(MCONFIG_AT_INDEX, (UINT8*)&DeviceConfig, sizeof(DeviceConfig));
		
		WriteonofFile();
		ReadonofFile();
		sprintf(BigSMS1,"");
		send_networkonofview(PhoneNumber);
		sAPI_UartWrite(eat_uart_wifi, "$$$",3);
		//eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 6, "CONWPS", EAT_NULL);
        //send_networkonofview(PhoneNumber);		
							
		}

		else if(strstr(smsbuffer,"wpsof") != 0)
		{
		ReadonofFile();
		wpsmodeon = 0;
		if(gprson==1)
		DeviceConfig.interface=GPRS;
	    else if(wifion==1)
		DeviceConfig.interface=WIFI;    
	    else
		DeviceConfig.interface=DISBLED;	
		
		WriteonofFile();
		ReadonofFile();
		if(DeviceConfig.interface==WIFI)
		//eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 6, "CONTCP", EAT_NULL); 
		sprintf(BigSMS1,"");
		send_networkonofview(PhoneNumber);
		////eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 6, "CONTCP", EAT_NULL); 														
							
		}
		else if(strstr(smsbuffer,"wapmodeon") != 0)
		{
		ReadonofFile();
		wpsmodeon = 0;
		apmodeon = 1;
		tcpopenflag=0;
		WriteonofFile();
		ReadonofFile();
		DeviceConfig.interface=WIFI;
		app_nvram_save(MCONFIG_AT_INDEX, (UINT8*)&DeviceConfig, sizeof(DeviceConfig));
		
	//	//eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 6, "CONSAP", EAT_NULL);
		sprintf(BigSMS1,"");
		send_networkonofview(PhoneNumber);
		sAPI_UartWrite(eat_uart_wifi, "$$$",3);
	    //eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 6, "CONSAP", EAT_NULL); 														
							
		}

		else if(strstr(smsbuffer,"wapmodeof") != 0)
		{
		ReadonofFile();
		apmodeon = 0;
		WriteonofFile();
		ReadonofFile();
		
		//eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 6, "CONTCP", EAT_NULL); 														
		sprintf(BigSMS1,"");
		send_networkonofview(PhoneNumber);
	//	//eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 6, "CONTCP", EAT_NULL); 														
							
		}
		else if(strstr(smsbuffer,"gprson") != 0)
		{
		ReadonofFile();
		gprson = 1;
		wifion=0;
		DeviceConfig.interface=GPRS;
		app_nvram_save(MCONFIG_AT_INDEX, (UINT8*)&DeviceConfig, sizeof(DeviceConfig));
	//	printParameter();
		
		WriteonofFile();
		ReadonofFile();
		//eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 7, "CONGPRS", EAT_NULL); 														
		sprintf(BigSMS1,"");
		send_networkonofview(PhoneNumber);
	//	//eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 7, "CONGPRS", EAT_NULL); 														
		 
		}
		
/*		else if(strstr(smsbuffer,"pswitchcnt") != 0)   //	pswitchcnt,mmss
		{
		ReadonofFile();
		Tp = smsbuffer[11]-'0';
		Tp1 = smsbuffer[12]-'0';
		Tp2 = smsbuffer[13]-'0';
		Tp3 = smsbuffer[14]-'0';
		nVaTr.pswdelaymin=(Tp*10+Tp1);
		nVaTr.pswdelaysec=(Tp2*10+Tp3);
		nVaTr.pswdelay=nVaTr.pswdelaymin*60+nVaTr.pswdelaysec;
		/* if(RecheckCounterUT_cnt>60)
			RecheckCounterUT_cnt=5; */
	/*	WriteonofFile();
	//	ReadonofFile();
		sprintf(buf,"nVaTr.pswdelay is %d",nVaTr.pswdelay);
		 // PrintfResp(buf);
		send_pswitchcount(PhoneNumber);
		}
		
		else if(strstr(smsbuffer,"vpswcnt") != 0)   //	pswitchcnt,mmss
		{
		send_pswitchcount(PhoneNumber);
		}*/

		else if(strstr(smsbuffer,"gprsof") != 0)
		{
		ReadonofFile();
		gprson = 0;
		if(wifion==1)
		DeviceConfig.interface=WIFI;
	    else
		DeviceConfig.interface=DISBLED;	
		app_nvram_save(MCONFIG_AT_INDEX, (UINT8*)&DeviceConfig, sizeof(DeviceConfig));
	//	printParameter();
		
		WriteonofFile();
		ReadonofFile();
		//eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 6, "CONTCP", EAT_NULL); 														
		sprintf(BigSMS1,"");
		send_networkonofview(PhoneNumber);
		// //eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 6, "CONTCP", EAT_NULL); 														
							
		}
		else if(strstr(smsbuffer,"#gprsgeton") != 0)
		{
		ReadonofFile();
		gprsgeton = 1;
		WriteonofFile();
		ReadonofFile();
		sprintf(BigSMS1,"");
		send_networkonofview(PhoneNumber);
		 
		}

		else if(strstr(smsbuffer,"#gprsgetof") != 0)
		{
		ReadonofFile();
		gprsgeton = 0;
		WriteonofFile();
		ReadonofFile();
		sprintf(BigSMS1,"");
		send_networkonofview(PhoneNumber);
							
		}

       else if(strstr(smsbuffer,"sappmodeon") != 0)
		{
		ReadonofFile();
		Appmodeon = 1;
		WriteonofFile();
		ReadonofFile();
		sprintf(BigSMS1,"");
		send_networkonofview(PhoneNumber);
		}

		else if(strstr(smsbuffer,"sappmodeof") != 0)
		{
		ReadonofFile();
		Appmodeon = 0;
		WriteonofFile();
		ReadonofFile();
		sprintf(BigSMS1,"");
		send_networkonofview(PhoneNumber);
		}
		else if(strstr(smsbuffer,"ndebugon") != 0 )
		{			 
		nMSettings.ndebugonof = 1;		 
		}  
		 else if(strstr(smsbuffer,"ndebugof") != 0 )
		{			 
		nMSettings.ndebugonof = 0;		 
		}
		else if(strstr(smsbuffer,"masterno") != 0)
		{
		send_Smsno(SmsNumber[0]);
		}		
		else if(strstr(smsbuffer,"mqttrecon") != 0)
		{		 
		MQTT_Reconnect();		 
		}
		
		else if(strstr(smsbuffer,"smson") != 0)
		{
		ReadFile();
		nMSettings.SMSOnOff = 1;
		STATE_SENDSMS = STATE_SMSON_SMS;
		//sprintf(smsbuffer,"SMS ON");
		WriteSettingsFile();
		SendSmsToAll = 1;
		//simcom_sms_send(ph_num,smsbuffer,strlen(smsbuffer));
		ReadFile();
		}
       /*else if(strstr(smsbuffer,"test") != 0)
		{
		ReadFile();
		nMSettings.SMSOnOff = 1;
		STATE_SENDSMS = STATE_SMSON_SMS;
		//sprintf(smsbuffer,"SMS ON");
		WriteSettingsFile();
		SendSmsToAll = 1;
		//simcom_sms_send(ph_num,smsbuffer,strlen(smsbuffer));
		ReadFile();
		}*/

		else if(strstr(smsbuffer,"smsof") != 0)
		{
		char SendString[70] = "";
		ReadFile();
		nMSettings.SMSOnOff = 0;
		//STATE_SENDSMS = STATE_SMSON_SMS;
		sprintf(SendString,"SMS OFF\n\r");
//		simcom_sms_send(ph_num, SendString, strlen(SendString));
		WriteSettingsFile();
		//SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr((char *)smsbuffer,"csq") != 0 )
		{
		char SendString[70] = "";
		float SignalStrength=0;
		int sstrength=0;
		INT32 ret;
		
		
		SignalStrength = (float)(CSQ*3.2258);
		if(SignalStrength>100)
		SignalStrength = 100;
		else if(SignalStrength<0)
		SignalStrength = 0;
		sstrength=SignalStrength;
		
		value1 = nVaTr.ActRefreshOfDelay;
		TpHr1 = value1/3600;
		value1 = value1%3600;
		TpMin1 = value1/60;
		value1 = value1%60;
		TpSec1 = value1;
		//sprintf(SendString,"Signal Strength = %03d percent RSSI= %02d 4G AG4 BOD PCGv%s\n\r",sstrength,CSQ,Version);
		sprintf(SendString,"Signal Strength = %03d percent RSSI= %02d 4G AG4 BOD PCGv%s,Build:%s\n\r",sstrength,CSQ,Version,Build);
//		simcom_sms_send(ph_num, SendString, strlen(SendString));
		sAPI_UartPrintf("\n\r smsbuffer %s\n\r",SendString);		 
		}
		else if(strstr(smsbuffer,"viewsmsset") != 0 )
		{
			ReadFile();
			STATE_SENDSMS=	STATE_VLIMITSMSSET_SMS;
			SendSmsToAll = 1;
			//ReadFile();
		}
		else if(strstr(smsbuffer,"limitsmsset") != 0 )
		{
		ReadFile();
		Tp = smsbuffer[11]-'0';
		Tp1 = smsbuffer[12]-'0';
		Tp2 = smsbuffer[13]-'0';
		limitsmsset=(Tp*100)+(Tp1*10)+Tp2;
		WriteSettingsFile();
		STATE_SENDSMS=	STATE_LIMITSMSSET_SMS;
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"smslimiton") != 0)
		{
		ReadFile();
		limitsmsonof = 1;
		STATE_SENDSMS = STATE_LIMIT_SMSON_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"smslimitof") != 0)
		{
		ReadFile();
		limitsmsonof = 0;
		STATE_SENDSMS = STATE_LIMIT_SMSON_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"sfbon") != 0)
		{
		ReadFile();
		nMSettings.SfbOnOff = 1;
		STATE_SENDSMS = STATE_SFB_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"sfbof") != 0)
		{
		ReadFile();
		nMSettings.SfbOnOff = 0;
		STATE_SENDSMS = STATE_SFB_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"pressureon") != 0)
		{
		ReadFile();
		nMSettings.PressureOnOff = 1;
		STATE_SENDSMS = STATE_PRESSURE_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"pressureof") != 0)
		{
		ReadFile();
		nMSettings.PressureOnOff = 0;
		STATE_SENDSMS = STATE_PRESSURE_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"getresponsecode") != 0)		
		{
				getresponsecode(PhoneNumber);
		}		
		else if(strstr(smsbuffer,"manualswitchon") != 0)
		{
		ReadFile();
		nMSettings.ManualswitchOnOff = 1;
		STATE_SENDSMS = STATE_MANUALSWITCH_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"manualswitchof") != 0)
		{
		ReadFile();
		nMSettings.ManualswitchOnOff = 0;
		STATE_SENDSMS = STATE_MANUALSWITCH_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		}
		
		else if(strstr(smsbuffer,"tankon") != 0)
		{
		ReadFile();
		nMSettings.TankOnOff = 1;
		STATE_SENDSMS = STATE_TANK_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"tankof") != 0)
		{
		ReadFile();
		nMSettings.TankOnOff = 0;
		STATE_SENDSMS = STATE_TANK_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"sumpon") != 0)
		{
		ReadFile();
		nMSettings.SumpOnOff = 1;
		STATE_SENDSMS = STATE_SUMP_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"sumpof") != 0)
		{
		ReadFile();
		nMSettings.SumpOnOff = 0;
		STATE_SENDSMS = STATE_SUMP_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"sumprston") != 0)
		{
		ReadFile();
		nMSettings.SumprstOnOff = 1;
		STATE_SENDSMS = STATE_SUMP_RST_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"sumprstof") != 0)
		{
		ReadFile();
		nMSettings.SumprstOnOff = 0;
		STATE_SENDSMS = STATE_SUMP_RST_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"dryrunon") != 0)
		{
		ReadFile();
		nMSettings.DryRunOnOff = 1;
		STATE_SENDSMS = STATE_DRYRUN_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"dryrunof") != 0)
		{
		ReadFile();
		nMSettings.DryRunOnOff = 0;
		STATE_SENDSMS = STATE_DRYRUN_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"targeton") != 0)
		{
		ReadFile();
		nMSettings.TargetOnOff = 1;
		STATE_SENDSMS = STATE_TARGET_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"targetof") != 0)
		{
		ReadFile();
		nMSettings.TargetOnOff = 0;
		STATE_SENDSMS = STATE_TARGET_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"serviceon") != 0)
		{
		char SendString[70] = "";
		ReadFile();
		nMSettings.serviceOnOff = 1;
		// STATE_SENDSMS = STATE_SERVICE_SMS;
		WriteSettingsFile();
		// SendSmsToAll = 1;
		ReadFile();
		sprintf(SendString,"SERVICE NUMBER ON\n");
		sprintf(SendString,"%sREG S NO:%s\n",SendString,StoredPhoneNumber[11]);
//		simcom_sms_send(ph_num, SendString, strlen(SendString));
		sprintf(buf,"\n\r smsbuffer %s\n\r",SendString);
		 PrintfResp(buf);
		}
		else if(strstr(smsbuffer,"serviceof") != 0)
		{
		char SendString[70] = "";
		ReadFile();
		nMSettings.serviceOnOff = 0;
		// STATE_SENDSMS = STATE_SERVICE_SMS;
		WriteSettingsFile();
		// SendSmsToAll = 1;
		ReadFile();
		sprintf(SendString,"SERVICE NUMBER OFF\n");
		sprintf(SendString,"%sREG S NO:%s\n",SendString,StoredPhoneNumber[11]);
//		simcom_sms_send(ph_num, SendString, strlen(SendString));
		sprintf(buf,"\n\r smsbuffer %s\n\r",SendString);
		 PrintfResp(buf);
		}
		else if(strstr(smsbuffer,"pwrvbon") != 0)
		{
		char SendString[70] = "";
		ReadFile();
		nMSettings.pwrvfbOnOf = 1;
		STATE_SENDSMS = STATE_PWRVFB_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		// sprintf(SendString,"POWER VFB ON\n");
		 //sprintf(SendString,"%sREG S NO:%s\n",SendString,StoredPhoneNumber[11]);
		// simcom_sms_msg_send(ph_num, SendString, strlen(SendString),SmSCallback);
		// sprintf(buf,"\n\r smsbuffer %s\n\r",SendString);
		 // PrintfResp(buf);
		}
		else if(strstr(smsbuffer,"pwrvbof") != 0)
		{
		char SendString[70] = "";
		ReadFile();
		nMSettings.pwrvfbOnOf = 0;
		STATE_SENDSMS = STATE_PWRVFB_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
	//	 sprintf(SendString,"POWER VFB OFF\n");
		//simcom_sms_msg_send(ph_num, SendString, strlen(SendString),SmSCallback);
		// sprintf(buf,"\n\r smsbuffer %s\n\r",SendString);
		 // PrintfResp(buf);
		}
		
		else if(strstr(smsbuffer,"vfbon") != 0)
		{
		ReadFile();
		nMSettings.VBFOnOff = 1;
		STATE_SENDSMS = STATE_VFB_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"vfbof") != 0)
		{
		ReadFile();
		nMSettings.VBFOnOff = 0;
		STATE_SENDSMS = STATE_VFB_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"ondelay") != 0)
		{
		ReadTimerSettings();
		Tp = smsbuffer[7]-'0';
		Tp3 = smsbuffer[8]-'0';
		nTimerSettings.POnHr = (Tp*10)+Tp3;

		Tp = smsbuffer[9]-'0';
		Tp3 = smsbuffer[10]-'0';
		nTimerSettings.POnMin = (Tp*10)+Tp3;

		Tp = smsbuffer[11]-'0';
		Tp3 = smsbuffer[12]-'0';

		nTimerSettings.POnSec = (Tp*10)+Tp3;

		WriteTimerSettings();
		//cpbrsearchend =0;
		STATE_SENDSMS=STATE_ONDELAY_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();

		}
		else if(strstr(smsbuffer,"sfbdelay") != 0)
		{
		ReadFile();
		Tp = smsbuffer[8]-'0';
		Tp3 = smsbuffer[9]-'0';
		nTimerSettings.SfbHr = (Tp*10)+Tp3;

		Tp = smsbuffer[10]-'0';
		Tp3 = smsbuffer[11]-'0';
		nTimerSettings.SfbMin = (Tp*10)+Tp3;

		Tp = smsbuffer[12]-'0';
		Tp3 = smsbuffer[13]-'0';

		nTimerSettings.SfbSec = (Tp*10)+Tp3;

		WriteSettingsFile();

		STATE_SENDSMS=STATE_SFBDELAY_SMS;
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"sddelay") != 0)
		{
		ReadTimerSettings();
		Tp = smsbuffer[7]-'0';
		Tp3 = smsbuffer[8]-'0';
		nTimerSettings.SDHr = (Tp*10)+Tp3;

		Tp = smsbuffer[9]-'0';
		Tp3 = smsbuffer[10]-'0';
		nTimerSettings.SDMin = (Tp*10)+Tp3;

		Tp = smsbuffer[11]-'0';
		Tp3 = smsbuffer[12]-'0';

		nTimerSettings.SDSec = (Tp*10)+Tp3;

		WriteTimerSettings();

		STATE_SENDSMS=STATE_SDDELAY_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"drrestarton") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.DrReOnOf = 1;
		WriteTimerSettings();

		STATE_SENDSMS=STATE_DRRSTONOF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"drrestartof") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.DrReOnOf = 0;
		WriteTimerSettings();

		STATE_SENDSMS=STATE_DRRSTONOF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"drrestart") != 0)
		{
		ReadTimerSettings();
		Tp = smsbuffer[9]-'0';
		Tp3 = smsbuffer[10]-'0';
		nTimerSettings.DrReHr = (Tp*10)+Tp3;

		Tp = smsbuffer[11]-'0';
		Tp3 = smsbuffer[12]-'0';
		nTimerSettings.DrReMin = (Tp*10)+Tp3;

		Tp = smsbuffer[13]-'0';
		Tp3 = smsbuffer[14]-'0';

		nTimerSettings.DrReSec = (Tp*10)+Tp3;

		WriteTimerSettings();

		STATE_SENDSMS=STATE_DRRST_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"drscan") != 0)
		{
		ReadTimerSettings();
		Tp = smsbuffer[6]-'0';
		Tp3 = smsbuffer[7]-'0';
		nTimerSettings.DrScHr = (Tp*10)+Tp3;

		Tp = smsbuffer[8]-'0';
		Tp3 = smsbuffer[9]-'0';
		nTimerSettings.DrScMin = (Tp*10)+Tp3;

		Tp = smsbuffer[10]-'0';
		Tp3 = smsbuffer[11]-'0';

		nTimerSettings.DrScSec = (Tp*10)+Tp3;

		WriteTimerSettings();

		STATE_SENDSMS=STATE_DRSCAN_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"maxtimon") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.MaxRnOnOf = 1;
		WriteTimerSettings();

		STATE_SENDSMS=STATE_MAXTIMONOF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"maxtimof") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.MaxRnOnOf = 0;
		WriteTimerSettings();

		STATE_SENDSMS=STATE_MAXTIMONOF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"maxtim") != 0)
		{
		ReadTimerSettings();
		Tp = smsbuffer[6]-'0';
		Tp3 = smsbuffer[7]-'0';
		nTimerSettings.MaxRnHr = (Tp*10)+Tp3;

		Tp = smsbuffer[8]-'0';
		Tp3 = smsbuffer[9]-'0';
		nTimerSettings.MaxRnMin = (Tp*10)+Tp3;

		Tp = smsbuffer[10]-'0';
		Tp3 = smsbuffer[11]-'0';

		nTimerSettings.MaxRnSec = (Tp*10)+Tp3;

		WriteTimerSettings();

		STATE_SENDSMS=STATE_MAXTIM_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"cyclicon") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.CycLicOnOf = 1;
		WriteTimerSettings();

		STATE_SENDSMS=STATE_CYCLICONOF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"cyclicof") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.CycLicOnOf = 0;
		WriteTimerSettings();

		STATE_SENDSMS=STATE_CYCLICONOF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"dltaudio") != 0 )        /*IT WILL DELETE ALL AUDIO FILES INSIDE*/ 
		{  

		    char dir_name[20]  = {0};int ret;int i=0;
			memset(dir_name,NULL,sizeof(dir_name));
			for(;i<=55;i++)  {
				sprintf(dir_name,"c:/Tr_%d.amr",i);							 

				ret = sAPI_remove(dir_name);
				if(ret != 0){
					sAPI_UartPrintf("\n\rsAPI_FsDirRemove del err,ERROR code: %d", ret);
					}
				else
					sAPI_UartPrintf("\r\n[%s] FILE REMOVED\r\n",dir_name);
			}
			sAPI_UartPrintf("\r\nFILE SYSTEM REMOVE dir SUCC\r\n"); 
		}
		else if(strstr((char *)smsbuffer,"listaudio") != 0 )         /*IT WILL LIST ALL AUDIO FILES INSIDE*/                    
			{			 
				 int ret;
				 char pTemBuffer[100];
				 SCDIR *dir_hdl = NULL;
				 struct dirent *info_dir = NULL;
				 dir_hdl = sAPI_opendir("c:/");
				if(dir_hdl == NULL)
				{
					sAPI_UartPrintf("sAPI_FsDirOpen OPEN err");								  
				}
				 while((info_dir = sAPI_readdir(dir_hdl)) != NULL)
				{
					sprintf(pTemBuffer, "\r\n[name]-[filesize]-[type]\r\n");
					sprintf(buf, "[%s]-[%ld]-[%d]\r\n",info_dir->name,info_dir->size,info_dir->type);
					strcat(pTemBuffer,buf);
					sAPI_UartWrite(SC_UART2,pTemBuffer,strlen(pTemBuffer));
					memset(pTemBuffer,0,sizeof(pTemBuffer));
				}

			ret = sAPI_closedir(dir_hdl);

			}
	 /*	else if(strstr(smsbuffer,"cyclictimof") != 0)
		{
		ReadTimerSettings();
		Tp = smsbuffer[11]-'0';
		Tp3 = smsbuffer[12]-'0';
		nTimerSettings.CycLicOfHr = (Tp*10)+Tp3;

		Tp = smsbuffer[13]-'0';
		Tp3 = smsbuffer[14]-'0';
		nTimerSettings.CycLicOfMin = (Tp*10)+Tp3;

		Tp = smsbuffer[15]-'0';
		Tp3 = smsbuffer[16]-'0';

		nTimerSettings.CycLicOfSec = (Tp*10)+Tp3;

		WriteTimerSettings();

		STATE_SENDSMS=STATE_CYCLICTIMOF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"cyclictimon") != 0)
		{
		ReadTimerSettings();
		Tp = smsbuffer[11]-'0';
		Tp3 = smsbuffer[12]-'0';
		nTimerSettings.CycLicOnHr = (Tp*10)+Tp3;

		Tp = smsbuffer[13]-'0';
		Tp3 = smsbuffer[14]-'0';
		nTimerSettings.CycLicOnMin = (Tp*10)+Tp3;

		Tp = smsbuffer[15]-'0';
		Tp3 = smsbuffer[16]-'0';

		nTimerSettings.CycLicOnSec = (Tp*10)+Tp3;

		WriteTimerSettings();

		STATE_SENDSMS=STATE_CYCLICTIMON_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}*/
		
else if(strstr(smsbuffer,"rtctimonof") != 0 ) 
{
i = smsbuffer[10]-'0';



if(i>=1&&i<=4)
{
ReadTimerSettings();
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, ","  );
}

Tp1= StrTokStr1[1][0]-'0';
Tp2= StrTokStr1[1][1]-'0';
nTimerSettings.RTCOnHr[i]=(Tp1*10)+Tp2;
Tp1= StrTokStr1[1][2]-'0';
Tp2= StrTokStr1[1][3]-'0';
nTimerSettings.RTCOnMin[i]=(Tp1*10)+Tp2;
Tp1= StrTokStr1[2][0]-'0';
Tp2= StrTokStr1[2][1]-'0';
nTimerSettings.RTCOfHr[i]=(Tp1*10)+Tp2;
Tp1= StrTokStr1[2][2]-'0';
Tp2= StrTokStr1[2][3]-'0';
nTimerSettings.RTCOfMin[i]=(Tp1*10)+Tp2;

WriteTimerSettings();
ReadTimerSettings();
if(i==1)
STATE_SENDSMS=STATE_RTCTIMON1_SMS;
if(i==2)
STATE_SENDSMS=STATE_RTCTIMON2_SMS;
if(i==3)
STATE_SENDSMS=STATE_RTCTIMON3_SMS;
if(i==4)
STATE_SENDSMS=STATE_RTCTIMON4_SMS;
SendSmsToAll = 1;
}
}
/*else if(strstr(smsbuffer,"fertcyclictimonof") != 0 ) 
{
ReadTimerSettings();
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, ","  );
}

Tp1= StrTokStr1[1][0]-'0';
Tp2= StrTokStr1[1][1]-'0';
nTimerSettings.fertCycLicOnMin=(Tp1*10)+Tp2;
Tp1= StrTokStr1[1][2]-'0';
Tp2= StrTokStr1[1][3]-'0';
nTimerSettings.fertCycLicOnSec=(Tp1*10)+Tp2;
Tp1= StrTokStr1[2][0]-'0';
Tp2= StrTokStr1[2][1]-'0';
nTimerSettings.fertCycLicOfMin=(Tp1*10)+Tp2;
Tp1= StrTokStr1[2][2]-'0';
Tp2= StrTokStr1[2][3]-'0';
nTimerSettings.fertCycLicOfSec=(Tp1*10)+Tp2;

WriteTimerSettings();
ReadTimerSettings();

STATE_SENDSMS=STATE_FERTCYCLICONOF_SMS;
SendSmsToAll = 1;

}*/
else if(strstr(smsbuffer,"cyclictimonof") != 0 ) 
{
ReadTimerSettings();
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, ","  );
}

Tp1= StrTokStr1[1][0]-'0';
Tp2= StrTokStr1[1][1]-'0';
nTimerSettings.CycLicOnHr=(Tp1*10)+Tp2;
Tp1= StrTokStr1[1][2]-'0';
Tp2= StrTokStr1[1][3]-'0';
nTimerSettings.CycLicOnMin=(Tp1*10)+Tp2;
Tp1= StrTokStr1[2][0]-'0';
Tp2= StrTokStr1[2][1]-'0';
nTimerSettings.CycLicOfHr=(Tp1*10)+Tp2;
Tp1= StrTokStr1[2][2]-'0';
Tp2= StrTokStr1[2][3]-'0';
nTimerSettings.CycLicOfMin=(Tp1*10)+Tp2;
nTimerSettings.CycLicOnSec=0;
nTimerSettings.CycLicOfSec=0;
WriteTimerSettings();
ReadTimerSettings();

STATE_SENDSMS=STATE_CYCLICONOF_SMS;
SendSmsToAll = 1;

}

	 /*	else if(strstr(smsbuffer,"dt") != 0)
		{
		unsigned int dd,mm,yy,hr,tm_min,tm_sec;
		sscanf(smsReadCnfContent.datetime,"%d/%d/%d,%d:%d:%d+%*s",&yy,&mm,&dd,&hr,&tm_min,&tm_sec);
		datetime.tm_year = yy;
		datetime.tm_mon = mm;
		datetime.tm_mday = dd;
		datetime.tm_hour = hr;
		datetime.tm_min = tm_min;
		datetime.tm_sec = tm_sec;
		sprintf(buf,"GetLocalTime:%d/%d/%d/ %d:%d:%d\r\n",datetime.tm_year, datetime.tm_mon, datetime.tm_mday,datetime.tm_hour, datetime.tm_min, datetime.tm_sec);
		 PrintfResp(buf);
		eat_set_rtc(&datetime);
		//Ql_sprintf(textBuf,"Ql_SetLocalTime()=%d\r\n",ret);
		//Ql_SendToUart(ql_uart_port1,(UINT8 *)textBuf,Ql_strlen(textBuf));
		STATE_SENDSMS=STATE_DATETIME_SMS;
		SendSmsToAll = 1;

		}*/

		else if(strstr(smsbuffer,"vtest") != 0)
		{
		ReadFile();
		ReadTimerSettings();
		NoAcceptSMS=0;
		//StopTimer(&timer);
		send_testpoint(ph_num);
		//timer.timeoutPeriod = 300;
		//timer.timerId = StartTimer(&timer);
		}
		else if(strstr(smsbuffer,"vset1") != 0)
		{
		ReadFile();
		ReadTimerSettings();
		//StopTimer(&timer);
		send_ondelaytimers(ph_num);
		//timer.timeoutPeriod = 300;
		//timer.timerId = StartTimer(&timer);
		}
		else if(strstr(smsbuffer,"vset2") != 0)
		{
		ReadFile();
		ReadTimerSettings();
		//StopTimer(&timer);
		send_timers(ph_num);
		//timer.timeoutPeriod = 300;
		//timer.timerId = StartTimer(&timer);
		}
		else if(strstr(smsbuffer,"vset3") != 0)
		{
		ReadFile();
		ReadTimerSettings();
		//StopTimer(&timer);
		send_onoff(ph_num);
		//timer.timeoutPeriod = 300;
		//timer.timerId = StartTimer(&timer);
		}
		else if(strstr(smsbuffer,"vset4") != 0)
		{
		ReadFile();
		ReadTimerSettings();
		//StopTimer(&timer);
		send_3phVoltages(ph_num);
		//timer.timeoutPeriod = 300;
		//timer.timerId = StartTimer(&timer);
		}
		else if(strstr(smsbuffer,"vset5") != 0)
		{
		ReadFile();
		ReadTimerSettings();
		send_2phVoltages(ph_num);
		//StopTimer(&timer);
		//timer.timeoutPeriod = 300;
		//timer.timerId = StartTimer(&timer);

		}
		else if(strstr(smsbuffer,"vset6") != 0)
		{

		ReadFile();
		ReadTimerSettings();
		//StopTimer(&timer);
		send_3Current(ph_num);
		//timer.timeoutPeriod = 300;
		//timer.timerId = StartTimer(&timer);
		}
		else if(strstr(smsbuffer,"vset7") != 0)
		{
		ReadFile();
		ReadTimerSettings();
		//StopTimer(&timer);
		send_2Current(ph_num);
		//timer.timeoutPeriod = 300;
		//timer.timerId = StartTimer(&timer);
		}
		else if(strstr(smsbuffer,"vset8") != 0)
		{
		ReadDprevSettings();
		//ReadDripSettings();
		send_2DripSystem(PhoneNumber);

		}
		else if(strstr(smsbuffer,"vset9") != 0)
		{
		ReadDprevSettings();
		//ReadDripSettings();
		SendCALCVSMSViewCal(PhoneNumber);

		}
		else if(strstr(smsbuffer,"vset010") != 0)
		{
		ReadonofFile();
		Sendviewset(PhoneNumber);
		}
		
	/*	else if(strstr(smsbuffer,"vgroupfertskip") != 0)
		{
		ReadDprevSettings();
		STATE_SENDSMS=	STATE_FERTSKIPGROUPDETAILS_SMS;
		SendSmsToAll = 1;
		}
		else if(strstr(smsbuffer,"vsplitfertskip") != 0)
		{
		ReadDprevSettings();
		STATE_SENDSMS=	STATE_FERTSKIPDETAILS_SMS;
		SendSmsToAll = 1;

		}*/
		else if(strstr(smsbuffer,"secon") != 0)
		{
		ReadFile();
		nMSettings.SecOnOf = 1;
		STATE_SENDSMS = STATE_SECONOF_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"poscddelayon") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.PoScrDlOnOff = 1;
		WriteTimerSettings();

		STATE_SENDSMS=STATE_POSCRDLONOF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();

		}
		else if(strstr(smsbuffer,"poscddelayof") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.PoScrDlOnOff = 0;
		WriteTimerSettings();

		STATE_SENDSMS=STATE_POSCRDLONOF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();

		}
		else if(strstr(smsbuffer,"poscddelay") != 0)
		{
		ReadTimerSettings();
		Tp = smsbuffer[10]-'0';
		Tp3 = smsbuffer[11]-'0';
		nTimerSettings.PoScrDlHr = (Tp*10)+Tp3;

		Tp = smsbuffer[12]-'0';
		Tp3 = smsbuffer[13]-'0';
		nTimerSettings.PoScrDlMin = (Tp*10)+Tp3;

		Tp = smsbuffer[14]-'0';
		Tp3 = smsbuffer[15]-'0';

		nTimerSettings.PoScrDlSec = (Tp*10)+Tp3;

		WriteTimerSettings();

		STATE_SENDSMS=STATE_POSCRDEL_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"secof") != 0)
		{
		ReadFile();
		nMSettings.SecOnOf = 0;
		STATE_SENDSMS = STATE_SECONOF_SMS;
		WriteSettingsFile();
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"scrdelayon") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.ScrDlOnOff = 1;
		WriteTimerSettings();

		STATE_SENDSMS=STATE_SCRDLONOF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();

		}
		else if(strstr(smsbuffer,"scrdelayof") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.ScrDlOnOff = 0;
		WriteTimerSettings();

		STATE_SENDSMS=STATE_SCRDLONOF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"scrdelay") != 0)
		{
		ReadTimerSettings();
		Tp = smsbuffer[8]-'0';
		Tp3 = smsbuffer[9]-'0';
		nTimerSettings.ScrDlHr = (Tp*10)+Tp3;

		Tp = smsbuffer[10]-'0';
		Tp3 = smsbuffer[11]-'0';
		nTimerSettings.ScrDlMin = (Tp*10)+Tp3;

		Tp = smsbuffer[12]-'0';
		Tp3 = smsbuffer[13]-'0';

		nTimerSettings.ScrDlSec = (Tp*10)+Tp3;

		WriteTimerSettings();

		STATE_SENDSMS=STATE_SCRDEL_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"lowvolton") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.LowVoltOnOff = 1;
		WriteTimerSettings();

		STATE_SENDSMS=STATE_LOWVOLTONOFF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();

		}
		else if(strstr(smsbuffer,"lowvoltof") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.LowVoltOnOff = 0;
		WriteTimerSettings();

		STATE_SENDSMS=STATE_LOWVOLTONOFF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"lowvolt2") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.LowVoltII = smsbuffer[8]-'0';
		Tp = smsbuffer[9]-'0';
		Tp3 = smsbuffer[10]-'0';

		nTimerSettings.LowVoltII = (nTimerSettings.LowVoltII*100)+(Tp*10)+Tp3;
		WriteTimerSettings();
		STATE_SENDSMS=STATE_LOWVOLTII_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"lowvolt3") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.LowVoltIII = smsbuffer[8]-'0';
		Tp = smsbuffer[9]-'0';
		Tp3 = smsbuffer[10]-'0';

		nTimerSettings.LowVoltIII = (nTimerSettings.LowVoltIII*100)+(Tp*10)+Tp3;
		WriteTimerSettings();
		STATE_SENDSMS=STATE_LOWVOLTIII_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"highvolton") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.HighVoltOnOff = 1;
		WriteTimerSettings();

		STATE_SENDSMS=STATE_HIGHVOLTONOFF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();

		}
		else if(strstr(smsbuffer,"highvoltof") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.HighVoltOnOff = 0;
		WriteTimerSettings();

		STATE_SENDSMS=STATE_HIGHVOLTONOFF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();

		}
		else if(strstr(smsbuffer,"highvolt2") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.HighVoltII = smsbuffer[9]-'0';
		Tp = smsbuffer[10]-'0';
		Tp3 = smsbuffer[11]-'0';

		nTimerSettings.HighVoltII = (nTimerSettings.HighVoltII*100)+(Tp*10)+Tp3;
		WriteTimerSettings();
		STATE_SENDSMS=STATE_HIGHVOLTII_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"highvolt3") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.HighVoltIII = smsbuffer[9]-'0';
		Tp = smsbuffer[10]-'0';
		Tp3 = smsbuffer[11]-'0';

		nTimerSettings.HighVoltIII = (nTimerSettings.HighVoltIII*100)+(Tp*10)+Tp3;
		WriteTimerSettings();
		STATE_SENDSMS=STATE_HIGHVOLTIII_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"hidiffvolt2") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.HiDiffVoltII = smsbuffer[11]-'0';
		Tp = smsbuffer[12]-'0';
		Tp3 = smsbuffer[13]-'0';

		nTimerSettings.HiDiffVoltII = (nTimerSettings.HiDiffVoltII*100)+(Tp*10)+Tp3;
		WriteTimerSettings();
		STATE_SENDSMS=STATE_HIDIFFVOLTII_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"hidiffvolt3") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.HiDiffVoltIII = smsbuffer[11]-'0';
		Tp = smsbuffer[12]-'0';
		Tp3 = smsbuffer[13]-'0';
		sprintf(buf,"\n\r High Volt = %d %d %d\n\r",nTimerSettings.HiDiffVoltIII,Tp,Tp3);
		 PrintfResp(buf);
		nTimerSettings.HiDiffVoltIII = (nTimerSettings.HiDiffVoltIII*100)+(Tp*10)+Tp3;

		sprintf(buf,"\n\r--------------- %d %d %d -------------------\n\r",nTimerSettings.HiDiffVoltIII,Tp,Tp3);
		WriteTimerSettings();
		STATE_SENDSMS=STATE_HIDIFFVOLTIII_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"diffvolt2") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.DiffVoltII = smsbuffer[9]-'0';
		Tp = smsbuffer[10]-'0';
		Tp3 = smsbuffer[11]-'0';

		nTimerSettings.DiffVoltII = (nTimerSettings.DiffVoltII*100)+(Tp*10)+Tp3;
		WriteTimerSettings();
		STATE_SENDSMS=STATE_DIFFVOLTII_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"diffvolt3") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.DiffVoltIII = smsbuffer[9]-'0';
		Tp = smsbuffer[10]-'0';
		Tp3 = smsbuffer[11]-'0';

		nTimerSettings.DiffVoltIII = (nTimerSettings.DiffVoltIII*100)+(Tp*10)+Tp3;
		WriteTimerSettings();
		STATE_SENDSMS=STATE_DIFFVOLTIII_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"pfcvolt") != 0)
		{
		ReadFile();
		nTimerSettings.pfcvolt = smsbuffer[7]-'0';
		Tp = smsbuffer[8]-'0';
		Tp3 = smsbuffer[9]-'0';

		nTimerSettings.pfcvolt = (nTimerSettings.pfcvolt*100)+(Tp*10)+Tp3;
		WriteSettingsFile();
		STATE_SENDSMS=STATE_PFCVOLT_SMS;
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"imbvolt") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.ImbVolt = smsbuffer[7]-'0';
		Tp = smsbuffer[8]-'0';
		Tp3 = smsbuffer[9]-'0';

		nTimerSettings.ImbVolt = (nTimerSettings.ImbVolt*100)+(Tp*10)+Tp3;
		WriteTimerSettings();
		STATE_SENDSMS=STATE_IMBVOLT_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"reversephaseon") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.RvePhOnoff = 1;
		WriteTimerSettings();
		STATE_SENDSMS=STATE_REVPHASE_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"reversephaseof") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.RvePhOnoff = 0;
		WriteTimerSettings();
		STATE_SENDSMS=STATE_REVPHASE_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"volsppon") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.SppOnoff = 1;
		WriteTimerSettings();
		STATE_SENDSMS=STATE_SPP_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"volsppof") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.SppOnoff = 0;
		WriteTimerSettings();
		STATE_SENDSMS=STATE_SPP_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"currentsppon") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.CurSppOnOff = 1;
		WriteTimerSettings();
		STATE_SENDSMS=STATE_CURSPP_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"currentsppof") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.CurSppOnOff = 0;
		WriteTimerSettings();
		STATE_SENDSMS=STATE_CURSPP_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"lowpreson") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.lowpressOnOff = 1;
		WriteTimerSettings();
		STATE_SENDSMS=STATE_LOWPRESSONOF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"lowpresof") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.lowpressOnOff = 0;
		WriteTimerSettings();
		STATE_SENDSMS=STATE_LOWPRESSONOF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"highpreson") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.highpressOnOff = 1;
		WriteTimerSettings();
		STATE_SENDSMS=STATE_HIGHPRESSONOF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"highpresof") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.highpressOnOff = 0;
		WriteTimerSettings();
		STATE_SENDSMS=STATE_HIGHPRESSONOF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		
		else if(strstr(smsbuffer,"olon") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.OlOnOff = 1;
		WriteTimerSettings();
		STATE_SENDSMS=STATE_OLINOFF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"olof") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.OlOnOff = 0;
		WriteTimerSettings();
		STATE_SENDSMS=STATE_OLINOFF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"olscan") != 0)
		{
		ReadTimerSettings();
		Tp = smsbuffer[6]-'0';
		Tp3 = smsbuffer[7]-'0';
		nTimerSettings.OlScanHr = (Tp*10)+Tp3;

		Tp = smsbuffer[8]-'0';
		Tp3 = smsbuffer[9]-'0';
		nTimerSettings.OlScanMin = (Tp*10)+Tp3;

		Tp = smsbuffer[10]-'0';
		Tp3 = smsbuffer[11]-'0';

		nTimerSettings.OlScanSec = (Tp*10)+Tp3;

		WriteTimerSettings();

		STATE_SENDSMS=STATE_OLSCAN_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"lowpress") != 0)
		{
		ReadTimerSettings();
		
		Tp1 = smsbuffer[8]-'0';
		Tp2 = smsbuffer[9]-'0';
		Tp3 = smsbuffer[11]-'0';
		nTimerSettings.lowpress = (Tp1*100)+(Tp2*10)+Tp3;
		//nTimerSettings.OlAmpsII = (nTimerSettings.OlAmpsII*100)+(Tp*10)+Tp3;
		nTimerSettings.lowpress = nTimerSettings.lowpress/10;

		//sprintf(buf,"\n\r Got Value of textBuf = %s\n\r",textBuf);
 // PrintfResp(buf);
		sprintf(buf,"\n\r Got Value of nTimerSettings.OlAmpsII = %2.1f\n\r",nTimerSettings.lowpress);
         PrintfResp(buf);
		WriteTimerSettings();

		STATE_SENDSMS=STATE_LOWPRESS_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"highpress") != 0)
		{
		ReadTimerSettings();
		
		Tp1 = smsbuffer[9]-'0';
		Tp2 = smsbuffer[10]-'0';
		Tp3 = smsbuffer[12]-'0';
		nTimerSettings.highpress = (Tp1*100)+(Tp2*10)+Tp3;
		nTimerSettings.highpress = nTimerSettings.highpress/10;



		sprintf(buf,"\n\r Got Value of nTimerSettings.OlAmpsII = %0.1f\n\r",nTimerSettings.highpress);
 PrintfResp(buf);

		WriteTimerSettings();

		STATE_SENDSMS=STATE_HIGHPRESS_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}

		else if(strstr(smsbuffer,"olamps2") != 0)
		{
		ReadTimerSettings();
		Tp = smsbuffer[7]-'0';
		Tp1 = smsbuffer[8]-'0';
		Tp2 = smsbuffer[9]-'0';
		Tp3 = smsbuffer[11]-'0';
		nTimerSettings.OlAmpsII = (Tp*1000)+(Tp1*100)+(Tp2*10)+Tp3;
		//nTimerSettings.OlAmpsII = (nTimerSettings.OlAmpsII*100)+(Tp*10)+Tp3;
		nTimerSettings.OlAmpsII = nTimerSettings.OlAmpsII/10;

		//sprintf(buf,"\n\r Got Value of textBuf = %s\n\r",textBuf);
 // PrintfResp(buf);
		sprintf(buf,"\n\r Got Value of nTimerSettings.OlAmpsII = %2.1f\n\r",nTimerSettings.OlAmpsII);
 PrintfResp(buf);
		WriteTimerSettings();

		STATE_SENDSMS=STATE_OLAMPSII_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"olamps3") != 0)
		{
		ReadTimerSettings();
		Tp = smsbuffer[7]-'0';
		Tp1 = smsbuffer[8]-'0';
		Tp2 = smsbuffer[9]-'0';
		Tp3 = smsbuffer[11]-'0';
		nTimerSettings.OlAmpsIII = (Tp*1000)+(Tp1*100)+(Tp2*10)+Tp3;
		nTimerSettings.OlAmpsIII = nTimerSettings.OlAmpsIII/10;



		sprintf(buf,"\n\r Got Value of nTimerSettings.OlAmpsII = %0.1f\n\r",nTimerSettings.OlAmpsII);
 PrintfResp(buf);

		WriteTimerSettings();

		STATE_SENDSMS=STATE_OLAMPSIII_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}

		else if(strstr(smsbuffer,"dramps2") != 0)
		{
		ReadTimerSettings();
		Tp = smsbuffer[7]-'0';
		Tp1 = smsbuffer[8]-'0';
		Tp2 = smsbuffer[9]-'0';
		Tp3 = smsbuffer[11]-'0';
		nTimerSettings.DrAmpsII = (Tp*1000)+(Tp1*100)+(Tp2*10)+Tp3;
        nTimerSettings.DrAmpsII = nTimerSettings.DrAmpsII/10;

		//sprintf(buf,"\n\r Got Value of textBuf = %s\n\r",textBuf);
 // PrintfResp(buf);
		sprintf(buf,"\n\r Got Value of nTimerSettings.DrAmpsII = %0.1f\n\r",nTimerSettings.DrAmpsII);
 PrintfResp(buf);

		WriteTimerSettings();

		STATE_SENDSMS=STATE_DRAMPII_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();

		}
		else if(strstr(smsbuffer,"calflow3") != 0)
		{
		ReadTimerSettings();
		Tp = smsbuffer[8]-'0';
		Tp1 = smsbuffer[9]-'0';
		Tp2 = smsbuffer[10]-'0';
		Tp3 = smsbuffer[12]-'0';
		nTimerSettings.calflow3 = (Tp*1000)+(Tp1*100)+(Tp2*10)+Tp3;
		//sprintf(buf,"\n\r Got Value of nTimerSettings.DrAmpsIII = %0.1f\n\r",nTimerSettings.DrAmpsIII);
         // PrintfResp(buf);
		nTimerSettings.calflow3 = (nTimerSettings.calflow3)/10;
		//sprintf(buf,"\n\r Got Value of nTimerSettings.DrAmpsIII = %0.1f\n\r",nTimerSettings.DrAmpsIII);
 // PrintfResp(buf);
		//sprintf(buf,"\n\r Got Value of textBuf = %s\n\r",textBuf);
 // PrintfResp(buf);
		sprintf(buf,"\n\r Got Value of nTimerSettings.calflow3 = %0.1f\n\r",nTimerSettings.calflow3);
		 PrintfResp(buf);
		WriteTimerSettings();
		STATE_SENDSMS=STATE_CALFLOWIII_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"calflow2") != 0)
		{
		ReadTimerSettings();
		Tp = smsbuffer[8]-'0';
		Tp1 = smsbuffer[9]-'0';
		Tp2 = smsbuffer[10]-'0';
		Tp3 = smsbuffer[12]-'0';
		nTimerSettings.calflow2 = (Tp*1000)+(Tp1*100)+(Tp2*10)+Tp3;
		//sprintf(buf,"\n\r Got Value of nTimerSettings.DrAmpsIII = %0.1f\n\r",nTimerSettings.DrAmpsIII);
         // PrintfResp(buf);
		nTimerSettings.calflow2 = (nTimerSettings.calflow2)/10;
		//sprintf(buf,"\n\r Got Value of nTimerSettings.DrAmpsIII = %0.1f\n\r",nTimerSettings.DrAmpsIII);
 // PrintfResp(buf);
		//sprintf(buf,"\n\r Got Value of textBuf = %s\n\r",textBuf);
 // PrintfResp(buf);
		sprintf(buf,"\n\r Got Value of nTimerSettings.calflow2 = %0.1f\n\r",nTimerSettings.calflow2);
		 PrintfResp(buf);
		WriteTimerSettings();
		STATE_SENDSMS=STATE_CALFLOWII_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		
		else if(strstr(smsbuffer,"dramps3") != 0)
		{
		ReadTimerSettings();
		Tp = smsbuffer[7]-'0';
		Tp1 = smsbuffer[8]-'0';
		Tp2 = smsbuffer[9]-'0';
		Tp3 = smsbuffer[11]-'0';
		nTimerSettings.DrAmpsIII = (Tp*1000)+(Tp1*100)+(Tp2*10)+Tp3;
		//sprintf(buf,"\n\r Got Value of nTimerSettings.DrAmpsIII = %0.1f\n\r",nTimerSettings.DrAmpsIII);
         // PrintfResp(buf);
		nTimerSettings.DrAmpsIII = (nTimerSettings.DrAmpsIII)/10;
		//sprintf(buf,"\n\r Got Value of nTimerSettings.DrAmpsIII = %0.1f\n\r",nTimerSettings.DrAmpsIII);
 // PrintfResp(buf);
		//sprintf(buf,"\n\r Got Value of textBuf = %s\n\r",textBuf);
 // PrintfResp(buf);
		sprintf(buf,"\n\r Got Value of nTimerSettings.DrAmpsIII = %0.1f\n\r",nTimerSettings.DrAmpsIII);
		 PrintfResp(buf);
		WriteTimerSettings();
		STATE_SENDSMS=STATE_DRAMPIII_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"2phaseon") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.AutoStIIOnOff = 1;
		WriteTimerSettings();
		STATE_SENDSMS=STATE_AUTOSTII_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"2phaseof") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.AutoStIIOnOff = 0;
		WriteTimerSettings();
		STATE_SENDSMS= STATE_AUTOSTII_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"olrston") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.AutoOlDrRstIIOnOff = 1;
		WriteTimerSettings();
		STATE_SENDSMS=STATE_AUTOOLDRRST_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"olrstof") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.AutoOlDrRstIIOnOff = 0;
		WriteTimerSettings();
		STATE_SENDSMS= STATE_AUTOOLDRRST_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"pfcseton") != 0)
		{
		ReadFile();
		nTimerSettings.pfcOnOff = 1;
		WriteSettingsFile();
		STATE_SENDSMS=STATE_POWERFACTOR_SMS;
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"pfcsetof") != 0)
		{
		ReadFile();
		nTimerSettings.pfcOnOff = 0;
		WriteSettingsFile();
		STATE_SENDSMS=STATE_POWERFACTOR_SMS;
		SendSmsToAll = 1;
		ReadFile();
		}
		else if(strstr(smsbuffer,"drrston") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.AutoDrRunRstIIOnOff = 1;
		WriteTimerSettings();
		STATE_SENDSMS=STATE_AUTODRRUNRST_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"drrstof") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.AutoDrRunRstIIOnOff = 0;
		WriteTimerSettings();
		STATE_SENDSMS= STATE_AUTODRRUNRST_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"manualon") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.ManualOnOff = 1;
		WriteTimerSettings();
		STATE_SENDSMS=STATE_MANULONOF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"manualof") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.ManualOnOff = 0;
		WriteTimerSettings();
		STATE_SENDSMS=STATE_MANULONOF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"olrestartvolton") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.OlRstVolOnoff= 1;
		WriteTimerSettings();
		STATE_SENDSMS=STATE_OLRSTVOLONOFF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"olrestartvoltof") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.OlRstVolOnoff = 0;
		WriteTimerSettings();
		STATE_SENDSMS=STATE_OLRSTVOLONOFF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"olrestartvolt") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.OlRstVol = smsbuffer[13]-'0';
		Tp = smsbuffer[14]-'0';
		Tp3 = smsbuffer[15]-'0';
		nTimerSettings.OlRstVol = (nTimerSettings.OlRstVol*100)+(Tp*10)+Tp3;
		WriteTimerSettings();
		STATE_SENDSMS=STATE_OLRSTVOL_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"droccuron") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.DrOccurOnOff= 1;
		WriteTimerSettings();
		STATE_SENDSMS=STATE_DROCCONOFF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"droccurof") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.DrOccurOnOff = 0;
		WriteTimerSettings();
		STATE_SENDSMS=STATE_DROCCONOFF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"droccurtim") != 0)
		{
		ReadTimerSettings();
		Tp = smsbuffer[10]-'0';
		Tp3 = smsbuffer[11]-'0';
		nTimerSettings.DrOccurTimHr = (Tp*10)+Tp3;

		Tp = smsbuffer[12]-'0';
		Tp3 = smsbuffer[13]-'0';
		nTimerSettings.DrOccurTimMin = (Tp*10)+Tp3;

		Tp = smsbuffer[14]-'0';
		Tp3 = smsbuffer[15]-'0';

		nTimerSettings.DrOccurTimSec = (Tp*10)+Tp3;

		WriteTimerSettings();

		STATE_SENDSMS=STATE_DROCCTIM_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"droccurnbr") != 0)
		{
		ReadTimerSettings();
		Tp = smsbuffer[10]-'0';
		Tp3 = smsbuffer[11]-'0';
		nTimerSettings.DrOccurNum = (Tp*10)+Tp3;
		WriteTimerSettings();
		STATE_SENDSMS=STATE_DROCCNUM_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"prvlotcal") != 0)
		{
		ReadTimerSettings();

		nTimerSettings.CalRVoltage = (239.999/nCurretnCond.RVoltage)*nTimerSettings.CalRVoltage;
		nTimerSettings.CalYVoltage = (239.999/nCurretnCond.YVoltage)*nTimerSettings.CalYVoltage;
		nTimerSettings.CalBVoltage = (239.999/nCurretnCond.BVoltage)*nTimerSettings.CalBVoltage;
		WriteTimerSettings();
		//StopTimer(&timer);
		SendSMSViewCal(1,ph_num);
		//timer.timeoutPeriod = 300;
		//timer.timerId = StartTimer(&timer);
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"prcurcal") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.CalRCurrent = (14.999/nCurretnCond.Rcurrent)*nTimerSettings.CalRCurrent;
		nTimerSettings.CalYCurrent = (14.999/nCurretnCond.Ycurrent)*nTimerSettings.CalYCurrent;
		nTimerSettings.CalBCurrent = (14.999/nCurretnCond.Bcurrent)*nTimerSettings.CalBCurrent;
		WriteTimerSettings();
		//StopTimer(&timer);
		SendSMSViewCal(2,ph_num);
		//timer.timeoutPeriod = 300;
		//timer.timerId = StartTimer(&timer);
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"vcalv") != 0)
		{
		ReadTimerSettings();

		//StopTimer(&timer);
		SendSMSViewCal(1,ph_num);
		//timer.timeoutPeriod = 300;
		//timer.timerId = StartTimer(&timer);
		}
		else if(strstr(smsbuffer,"vcalc") != 0)
		{
		ReadTimerSettings();
		//StopTimer(&timer);
		SendSMSViewCal(2,ph_num);
		//timer.timeoutPeriod = 300;
		//timer.timerId = StartTimer(&timer);
		}
		else if(strstr(smsbuffer,"voltagcal") != 0)
		{
		float TpVoltR,TpVoltY,TpVoltB;
		float TpTmp;

		ReadTimerSettings();
		Pch1 = strtok((char *)smsbuffer, (char *)"," );
		StrTokStrVer = 0;
		while( Pch1 != NULL )
		{
		strcpy(StrTokStr1[StrTokStrVer],Pch1);
		StrTokStrVer++;
		Pch1 = strtok( NULL, "," );
		}
		for(Tp=0;Tp<=StrTokStrVer;Tp++)
		{
		sprintf(buf,"\n\rSt=%s  pos=%d\n\r",StrTokStr1[Tp],Tp);
 PrintfResp(buf);
		}
		TpVoltR = myatof(StrTokStr1[1]);
		TpVoltY = myatof(StrTokStr1[2]);
		TpVoltB = myatof(StrTokStr1[3]);

		TpTmp = (TpVoltR/nCurretnCond.RVoltage);
		nTimerSettings.CalRVoltage = (TpTmp)*nTimerSettings.CalRVoltage;
		TpTmp = (TpVoltY/nCurretnCond.YVoltage);
		nTimerSettings.CalYVoltage = (TpTmp)*nTimerSettings.CalYVoltage;
		TpTmp = (TpVoltB/nCurretnCond.BVoltage);
		nTimerSettings.CalBVoltage = (TpTmp)*nTimerSettings.CalBVoltage;

		WriteTimerSettings();
		//StopTimer(&timer);
		SendSMSViewCal(1,ph_num);
		//timer.timeoutPeriod = 300;
		//timer.timerId = StartTimer(&timer);
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"2phasevoltagecal") != 0)
		{
		ReadTimerSettings();
		Pch1 = strtok((char *)smsbuffer, (char *)"," );
		StrTokStrVer = 0;
		while( Pch1 != NULL )
		{
		strcpy(StrTokStr1[StrTokStrVer],Pch1);
		StrTokStrVer++;
		Pch1 = strtok( NULL, "," );
		}
		for(Tp=0;Tp<=StrTokStrVer;Tp++)
		{
		sprintf(buf,"\n\rSt=%s  pos=%d\n\r",StrTokStr1[Tp],Tp);
 PrintfResp(buf);
		}
		nTimerSettings.R2PhaseToPhaseFactor = myatof(StrTokStr1[1]);
		WriteTimerSettings();
		//StopTimer(&timer);
		SendSMSViewPhCal(1,ph_num);
		//timer.timeoutPeriod = 300;
		//timer.timerId = StartTimer(&timer);
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"phasevoltagecal") != 0)
		{
		ReadTimerSettings();
		Pch1 = strtok((char *)smsbuffer, (char *)"," );
		StrTokStrVer = 0;
		while( Pch1 != NULL )
		{
		strcpy(StrTokStr1[StrTokStrVer],Pch1);
		StrTokStrVer++;
		Pch1 = strtok( NULL, "," );
		}
		for(Tp=0;Tp<=StrTokStrVer;Tp++)
		{
		sprintf(buf,"\n\rSt=%s  pos=%d\n\r",StrTokStr1[Tp],Tp);
 PrintfResp(buf);
		}
		nTimerSettings.RPhaseToPhaseFactor = myatof(StrTokStr1[1]);
		nTimerSettings.YPhaseToPhaseFactor = myatof(StrTokStr1[2]);
		nTimerSettings.BPhaseToPhaseFactor = myatof(StrTokStr1[3]);
		WriteTimerSettings();

		//StopTimer(&timer);
		SendSMSViewPhCal(0,ph_num);
		//timer.timeoutPeriod = 300;
		//timer.timerId = StartTimer(&timer);
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"viewphasevoltagecal") != 0)
		{
		ReadTimerSettings();

		//StopTimer(&timer);
		SendSMSViewPhCal(1,ph_num);
		//timer.timeoutPeriod = 300;
		//timer.timerId = StartTimer(&timer);
		}
		else if(strstr(smsbuffer,"voltcal") != 0)
		{
		ReadTimerSettings();
		Pch1 = strtok((char *)smsbuffer, (char *)"," );
		StrTokStrVer = 0;
		while( Pch1 != NULL )
		{
		strcpy(StrTokStr1[StrTokStrVer],Pch1);
		StrTokStrVer++;
		Pch1 = strtok( NULL, "," );
		}
		for(Tp=0;Tp<=StrTokStrVer;Tp++)
		{
		sprintf(buf,"\n\rSt=%s  pos=%d\n\r",StrTokStr1[Tp],Tp);
 PrintfResp(buf);
		}
		nTimerSettings.CalRVoltage = myatof(StrTokStr1[1]);
		nTimerSettings.CalYVoltage = myatof(StrTokStr1[2]);
		nTimerSettings.CalBVoltage = myatof(StrTokStr1[3]);
		WriteTimerSettings();

		//StopTimer(&timer);
		SendSMSViewCal(1,ph_num);
		//timer.timeoutPeriod = 300;
		//timer.timerId = StartTimer(&timer);
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"currentcal") != 0)
		{
		float TpCurR,TpCurY,TpCurB;
		float TpTmp;

		ReadTimerSettings();
		Pch1 = strtok((char *)smsbuffer, (char *)"," );
		StrTokStrVer = 0;
		while( Pch1 != NULL )
		{
		strcpy(StrTokStr1[StrTokStrVer],Pch1);
		StrTokStrVer++;
		Pch1 = strtok( NULL, "," );
		}
		for(Tp=0;Tp<=StrTokStrVer;Tp++)
		{
		sprintf(buf,"\n\rSt=%s  pos=%d\n\r",StrTokStr1[Tp],Tp);
 PrintfResp(buf);
		}
		TpCurR = myatof(StrTokStr1[1]);
		TpCurY = myatof(StrTokStr1[2]);
		TpCurB = myatof(StrTokStr1[3]);

		TpTmp = (TpCurR/nCurretnCond.Rcurrent);
		nTimerSettings.CalRCurrent = (TpTmp)*nTimerSettings.CalRCurrent;
		TpTmp = (TpCurY/nCurretnCond.Ycurrent);
		nTimerSettings.CalYCurrent = (TpTmp)*nTimerSettings.CalYCurrent;
		TpTmp = (TpCurB/nCurretnCond.Bcurrent);
		nTimerSettings.CalBCurrent = (TpTmp)*nTimerSettings.CalBCurrent;

		WriteTimerSettings();

		//StopTimer(&timer);
		SendSMSViewCal(2,ph_num);
		//timer.timeoutPeriod = 300;
		//timer.timerId = StartTimer(&timer);
		ReadTimerSettings();
		}
        
		else if(strstr(smsbuffer,"curcal") != 0)
		{
		sprintf(buf,"\n\rbuffer = %s\n\r",smsbuffer);
		 PrintfResp(buf);
		ReadTimerSettings();

		Pch1 = strtok((char *)smsbuffer, (char *)"," );
		StrTokStrVer = 0;
		while( Pch1 != NULL )
		{
		strcpy(StrTokStr1[StrTokStrVer],Pch1);
		StrTokStrVer++;
		Pch1 = strtok( NULL, "," );
		}
		for(Tp=0;Tp<=StrTokStrVer;Tp++)
		{
		sprintf(buf,"\n\rSt=%s  pos=%d\n\r",StrTokStr1[Tp],Tp);
 PrintfResp(buf);
		}
		nTimerSettings.CalRCurrent = myatof(StrTokStr1[1]);
		nTimerSettings.CalYCurrent = myatof(StrTokStr1[2]);
		nTimerSettings.CalBCurrent = myatof(StrTokStr1[3]);
		WriteTimerSettings();
		//StopTimer(&timer);
		SendSMSViewCal(2,ph_num);
		//timer.timeoutPeriod = 300;
		//timer.timerId = StartTimer(&timer);
		ReadTimerSettings();
		}
		//else if(strstr(smsbuffer,"vgrncal") != 0)
	/*	else if(strstr(smsbuffer,"getdta") != 0)

		{
		sprintf(buf,"\n\rbuffer = %s\n\r",smsbuffer);
		 // PrintfResp(buf);
		readidcomset();
		
		SendSMSGRNViewCal(PhoneNumber);
		}*/
		else if(strstr(smsbuffer,"dt") != 0)
		{
		    int retv;
         	t_rtc timev;		
         	t_rtc timeval;
             
          
			
		  sprintf(buf,"\n\rD&T:[%s]\n\r",dtm);      
		  PrintfResp(buf);
	      sscanf((char* )dtm, "%2d/%2d/%2d,%2d:%2d:%2d",&timeval.tm_year,&timeval.tm_mon,&timeval.tm_mday,&timeval.tm_hour,&timeval.tm_min,&timeval.tm_sec);
		  timeval.tm_year = timeval.tm_year >= 70 ? (1900 + timeval.tm_year) : (2000 + timeval.tm_year);
		  retv = sAPI_SetRealTimeClock(&timeval);
			if(retv < 0)
			{
				PrintfResp("\r\n {set time failed 1}!\r\n");
				NTPokFlag=0;
			}
			else
			{   
				 NTPokFlag=1;
				PrintfResp("\r\n [set time successed!]\r\n");
			} 
		
		sAPI_GetRealTimeClock(&timev);
		sprintf(buf,"\r\n Get RTC time: %d/%02d/%02d,%02d:%02d:%02d \r\n", timev.tm_year, timev.tm_mon, timev.tm_mday,
		timev.tm_hour, timev.tm_min, timev.tm_sec);
		PrintfResp(buf);
		 
		//eat_set_rtc(&datetime);
		//Ql_sprintf(textBuf,"Ql_SetLocalTime()=%d\r\n",ret);
		//Ql_SendToUart(ql_uart_port1,(UINT8 *)textBuf,Ql_strlen(textBuf));
		STATE_SENDSMS=STATE_DATETIME_SMS;
		SendSmsToAll = 1;

		}

	/*	else if(strstr(smsbuffer,"vfertflowcal") != 0)
		{
		sprintf(buf,"\n\rbuffer = %s\n\r",smsbuffer);
		
		 PrintfResp(buf);
		 readidcomset();
		sprintf(BigSMS1,"V44");
		SendSMSFERTFLOWViewCal(PhoneNumber);
		}
		else if(strstr(smsbuffer,"grncal") != 0)
		{
		sprintf(buf,"\n\rbuffer = %s\n\r",smsbuffer);
		 PrintfResp(buf);
		readidcomset();
		

		Pch1 = strtok((char *)smsbuffer, (char *)"," );
		StrTokStrVer = 0;
		while( Pch1 != NULL )
		{
		strcpy(StrTokStr1[StrTokStrVer],Pch1);
		StrTokStrVer++;
		Pch1 = strtok( NULL, "," );
		}
		for(Tp=0;Tp<=StrTokStrVer;Tp++)
		{
		sprintf(buf,"\n\rSt=%s  pos=%d\n\r",StrTokStr1[Tp],Tp);
 PrintfResp(buf);
		}
		zonecom.humcal1=myatof(StrTokStr1[1]);
		zonecom.tempcal2=myatof(StrTokStr1[2]);
		zonecom.ldenscal3=myatof(StrTokStr1[3]);
		zonecom.extcal4=myatof(StrTokStr1[4]);
		writeidcomset();
		readidcomset();
		SendSMSGRNViewCal(PhoneNumber);
		}
		else if(strstr(smsbuffer,"vghtlevel") != 0)
		{
		sprintf(buf,"\n\rbuffer = %s\n\r",smsbuffer);
		 PrintfResp(buf);
		readidcomset();
		sprintf(BigSMS1,"V61");
		SendSMSghtlViewCal(PhoneNumber);
		}
		
       else if(strstr(smsbuffer,"ldensonoflevel") != 0)
		{
		sprintf(buf,"\n\rbuffer = %s\n\r",smsbuffer);
		 PrintfResp(buf);
		readidcomset();
		

		Pch1 = strtok((char *)smsbuffer, (char *)"," );
		StrTokStrVer = 0;
		while( Pch1 != NULL )
		{
		strcpy(StrTokStr1[StrTokStrVer],Pch1);
		StrTokStrVer++;
		Pch1 = strtok( NULL, "," );
		}
		for(Tp=0;Tp<=StrTokStrVer;Tp++)
		{
		sprintf(buf,"\n\rSt=%s  pos=%d\n\r",StrTokStr1[Tp],Tp);
 PrintfResp(buf);
		}
		zonecom.ldensonlevel=myatof(StrTokStr1[1]);
		zonecom.ldensoflevel=myatof(StrTokStr1[2]);
		writeidcomset();
		readidcomset();
		sprintf(BigSMS1,"");
		SendSMSghtlViewCal(PhoneNumber);
		}
		else if(strstr(smsbuffer,"temponoflevel") != 0)
		{
		sprintf(buf,"\n\rbuffer = %s\n\r",smsbuffer);
		 PrintfResp(buf);
		readidcomset();
		

		Pch1 = strtok((char *)smsbuffer, (char *)"," );
		StrTokStrVer = 0;
		while( Pch1 != NULL )
		{
		strcpy(StrTokStr1[StrTokStrVer],Pch1);
		StrTokStrVer++;
		Pch1 = strtok( NULL, "," );
		}
		for(Tp=0;Tp<=StrTokStrVer;Tp++)
		{
		sprintf(buf,"\n\rSt=%s  pos=%d\n\r",StrTokStr1[Tp],Tp);
 PrintfResp(buf);
		}
		zonecom.temponlevel=myatof(StrTokStr1[1]);
		zonecom.tempoflevel=myatof(StrTokStr1[2]);
		writeidcomset();
		readidcomset();
		sprintf(BigSMS1,"");
		SendSMSghtlViewCal(PhoneNumber);
		}
		else if(strstr(smsbuffer,"humonoflevel") != 0)
		{
		sprintf(buf,"\n\rbuffer = %s\n\r",smsbuffer);
		 PrintfResp(buf);
		readidcomset();
		

		Pch1 = strtok((char *)smsbuffer, (char *)"," );
		StrTokStrVer = 0;
		while( Pch1 != NULL )
		{
		strcpy(StrTokStr1[StrTokStrVer],Pch1);
		StrTokStrVer++;
		Pch1 = strtok( NULL, "," );
		}
		for(Tp=0;Tp<=StrTokStrVer;Tp++)
		{
		sprintf(buf,"\n\rSt=%s  pos=%d\n\r",StrTokStr1[Tp],Tp);
 PrintfResp(buf);
		}
		zonecom.humonlevel=myatof(StrTokStr1[1]);
		zonecom.humoflevel=myatof(StrTokStr1[2]);
		writeidcomset();
		readidcomset();
		sprintf(BigSMS1,"");
		SendSMSghtlViewCal(PhoneNumber);
		}*/
		else if(strstr(smsbuffer,"vctptratio") != 0)
		{
		sprintf(buf,"\n\rbuffer = %s\n\r",smsbuffer);
		 PrintfResp(buf);
		readidcomset();
        sprintf(BigSMS1,"V43");
		SendSMSctptViewCal(PhoneNumber);
		}
		else if(strstr(smsbuffer,"valcond") != 0)
		{
			
			send_val_smsStatus(PhoneNumber);
			
		}
		else if(strstr(smsbuffer,"vsettarget") != 0)
	   {
		
		SendTARGETVIEW(PhoneNumber);

		
	}
		else if(strstr(smsbuffer,"ctptratio") != 0)
		{
		sprintf(buf,"\n\rbuffer = %s\n\r",smsbuffer);
		 PrintfResp(buf);
		readidcomset();
		

		Pch1 = strtok((char *)smsbuffer, (char *)"," );
		StrTokStrVer = 0;
		while( Pch1 != NULL )
		{
		strcpy(StrTokStr1[StrTokStrVer],Pch1);
		StrTokStrVer++;
		Pch1 = strtok( NULL, "," );
		}
		for(Tp=0;Tp<=StrTokStrVer;Tp++)
		{
		sprintf(buf,"\n\rSt=%s  pos=%d\n\r",StrTokStr1[Tp],Tp);
 PrintfResp(buf);
		}
		zonecom.ctratio=myatof(StrTokStr1[1]);
		zonecom.ptratio=myatof(StrTokStr1[2]);
		writeidcomset();
		readidcomset();
		sprintf(BigSMS1,"");
		SendSMSctptViewCal(PhoneNumber);
		}
		else if(strstr(smsbuffer,"fertflowcal") != 0)
		{
		sprintf(buf,"\n\rbuffer = %s\n\r",smsbuffer);
		 PrintfResp(buf);
		readidcomset();
		

		Pch1 = strtok((char *)smsbuffer, (char *)"," );
		StrTokStrVer = 0;
		while( Pch1 != NULL )
		{
		strcpy(StrTokStr1[StrTokStrVer],Pch1);
		StrTokStrVer++;
		Pch1 = strtok( NULL, "," );
		}
		for(Tp=0;Tp<=StrTokStrVer;Tp++)
		{
		sprintf(buf,"\n\rSt=%s  pos=%d\n\r",StrTokStr1[Tp],Tp);
 PrintfResp(buf);
		}
		zonecom.fertflowcal1=myatof(StrTokStr1[1]);
		zonecom.fertflowcal2=myatof(StrTokStr1[2]);
		zonecom.fertflowcal3=myatof(StrTokStr1[3]);
		zonecom.fertflowcal4=myatof(StrTokStr1[4]);
	
		writeidcomset();
		readidcomset();
		sprintf(BigSMS1,"");
		SendSMSFERTFLOWViewCal(PhoneNumber);
		}
		else if(strstr(smsbuffer,"ctron") != 0)
		{
		ReadTimerSettings();

		nTimerSettings.CTRonoff = 1;

		WriteTimerSettings();
		STATE_SENDSMS=STATE_CTRON_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"ctrof") != 0)
		{
		ReadTimerSettings();

		nTimerSettings.CTRonoff = 0;

		WriteTimerSettings();
		STATE_SENDSMS=	STATE_CTROF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"ctyon") != 0)
		{
		ReadTimerSettings();

		nTimerSettings.CTYonoff = 1;

		WriteTimerSettings();
		STATE_SENDSMS=STATE_CTYON_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"ctyof") != 0)
		{
		ReadTimerSettings();

		nTimerSettings.CTYonoff = 0;

		WriteTimerSettings();
		STATE_SENDSMS=	STATE_CTYOF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"ctbon") != 0)
		{
		ReadTimerSettings();

		nTimerSettings.CTBonoff = 1;

		WriteTimerSettings();
		STATE_SENDSMS=STATE_CTBON_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"ctbof") != 0)
		{
		ReadTimerSettings();

		nTimerSettings.CTBonoff = 0;

		WriteTimerSettings();
		STATE_SENDSMS=	STATE_CTBOF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"autorst2on") != 0)
		{
		ReadTimerSettings();

		nTimerSettings.AutoRst2On = 1;

		WriteTimerSettings();
		STATE_SENDSMS=	STATE_AUTORST2ONOFF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"autorst2of") != 0)
		{
		ReadTimerSettings();

		nTimerSettings.AutoRst2On = 0;

		WriteTimerSettings();
		STATE_SENDSMS=	STATE_AUTORST2ONOFF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}

		else if(strstr(smsbuffer,"mobilerston") != 0)
		{
		ReadTimerSettings();

		nTimerSettings.AutoRstOn = 1;

		WriteTimerSettings();
		STATE_SENDSMS=	STATE_AUTORSTONOFF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"mobilerstof") != 0)
		{
		ReadTimerSettings();

		nTimerSettings.AutoRstOn = 0;

		WriteTimerSettings();
		STATE_SENDSMS=	STATE_AUTORSTONOFF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		/*else if(strstr(smsbuffer,"settankheight"))
		{
		ReadonofFile();
		Pch1 = strtok((char *)smsbuffer, (char *)"," );
		StrTokStrVer = 0;
		while( Pch1 != NULL )
		{
		strcpy(StrTokStr1[StrTokStrVer],Pch1);
		StrTokStrVer++;
		Pch1 = strtok( NULL, "," );
		}
		settank_height=myatof(StrTokStr1[1]);
		WriteonofFile();
		sprintf(buf,"settankheight is %f",settank_height);
		 PrintfResp(buf);
		send_tankheight(SmsNumber[0]);
		}*/
		
	/*	else if(strstr(smsbuffer,"cregrston") != 0 ) 
	
		{
			nVaTr.cregrstonof=1;
			WriteonofFile();
			send_cregrstonof(PhoneNumber);
		}

		else if(strstr(smsbuffer,"cregrstof") != 0 ) 
			
		{
			nVaTr.cregrstonof=0;
			WriteonofFile();
			send_cregrstonof(PhoneNumber);
		}
		else if(strstr(smsbuffer,"cregcount") != 0 ) 
		{
				ReadonofFile();
				Tp = smsbuffer[9]-'0';
				Tp1 = smsbuffer[10]-'0';
				Tp2 = smsbuffer[11]-'0';
				Tp3 = smsbuffer[12]-'0';
				nVaTr.cregdelayHr=(Tp*10+Tp1);
				nVaTr.cregdelayMin=(Tp2*10+Tp3);
				nVaTr.cregdelay=nVaTr.cregdelayHr*3600+nVaTr.cregdelayMin*60;
				WriteonofFile();
				sprintf(buf,"nVaTr.cregdelay is %ld",nVaTr.cregdelay);
				 PrintfResp(buf);
				send_cregcount(PhoneNumber);
				
				
			
		}

	else if(strstr(smsbuffer,"cregvcount") != 0 )
	{
		ReadonofFile();
		send_vcregcount(PhoneNumber);
		
		
	}*/

		else if(strstr(smsbuffer,"prodset") != 0 )
		{
		ReadFile();
		ReadTimerSettings();
		Appmodeon=1;
		nMSettings.canSMSOnOff = 1;
		nTimerSettings.PoScrDlHr=0;
		nTimerSettings.PoScrDlMin=0;
		nTimerSettings.PoScrDlSec=0;
		nMSettings.gethidesmsonoff=0;
		nMSettings.SMSOnOff = 0;
		nMSettings.staticSMSOnOff = 0;
		nMSettings.SfbOnOff = 0;
		nMSettings.PressureOnOff =0;
	    nMSettings.ManualswitchOnOff = 0;
		nMSettings.TankOnOff = 0;
		nMSettings.SumpOnOff = 0;
		nMSettings.DryRunOnOff = 0;
		nMSettings.TargetOnOff = 0;
		nMSettings.VBFOnOff = 0;
		nMSettings.RelayControlOnCall = 0;
		nMSettings.SecOnOf = 0;


		nTimerSettings.POnHr = 0;
		nTimerSettings.POnMin = 0;
		nTimerSettings.POnSec = 0;
		nTimerSettings.SDHr = 0;
		nTimerSettings.SDMin = 0;
		nTimerSettings.SDSec = 0;
		nTimerSettings.SfbHr = 0;
		nTimerSettings.SfbMin = 0;
		nTimerSettings.SfbSec = 0;

		nTimerSettings.DrReOnOf = 0;
		nTimerSettings.DrReHr = 0;
		nTimerSettings.DrReMin = 0;
		nTimerSettings.DrReSec = 0;
		nTimerSettings.DrScOnOf = 0;
		nTimerSettings.DrScHr = 0;
		nTimerSettings.DrScMin = 0;
		nTimerSettings.DrScSec = 0;
		nTimerSettings.MaxRnOnOf = 0;
		nTimerSettings.MaxRnHr = 0;
		nTimerSettings.MaxRnMin = 0;
		nTimerSettings.MaxRnSec = 0;
		nTimerSettings.CycLicOnOf = 0;
		nTimerSettings.CycLicOnHr = 0;
		nTimerSettings.CycLicOnMin = 0;
		nTimerSettings.CycLicOnSec = 0;
		nTimerSettings.CycLicOfHr = 0;
		nTimerSettings.CycLicOfMin = 0;
		nTimerSettings.CycLicOfSec = 0;
		nTimerSettings.fertCycLicOfMin=0;
		nTimerSettings.fertCycLicOfSec=0;
		nTimerSettings.fertCycLicOnMin=0;
		nTimerSettings.fertCycLicOnSec=0;
		
		nTimerSettings.RTCOnOf = 0;
      for(i=1;i<5;i++)
		{
		nTimerSettings.RTCOnHr[i] = 0;
		nTimerSettings.RTCOnMin[i] = 0;
		nTimerSettings.RTCOnSec[i] = 0;
		nTimerSettings.RTCOfHr[i] = 0;
		nTimerSettings.RTCOfMin[i] = 0;
		nTimerSettings.RTCOfSec[i] = 0;
		}
		

		nTimerSettings.ScrDlOnOff = 0;
		nTimerSettings.ScrDlHr = 0;
		nTimerSettings.ScrDlMin = 0;
		nTimerSettings.ScrDlSec = 0;
		nTimerSettings.LowVoltOnOff = 0;
		nTimerSettings.LowVoltII = 0;
		nTimerSettings.LowVoltIII = 0;
		nTimerSettings.HighVoltOnOff = 0;
		nTimerSettings.HighVoltII = 0;
		nTimerSettings.HighVoltIII = 0;
		nTimerSettings.ImbVolt = 0;
		nTimerSettings.RvePhOnoff = 0;
		nTimerSettings.SppOnoff = 0;
		nTimerSettings.CurSppOnOff = 0;
		nTimerSettings.OlOnOff = 0;
		nTimerSettings.OlAmpsII = 0;
		nTimerSettings.OlAmpsIII = 0;
		nTimerSettings.OlScanHr = 0;
		nTimerSettings.OlScanMin = 0;
		nTimerSettings.OlScanSec = 0;

		nTimerSettings.DrAmpsII = 0;
		nTimerSettings.DrAmpsIII = 0;


		//nTimerSettings.AutoStIIOnOff = 0;
		nTimerSettings.AutoStIIIOnOff = 0;
		nTimerSettings.AutoOlDrRstIIOnOff = 0;
		nTimerSettings.AutoDrRunRstIIOnOff=0;
		nTimerSettings.ManualOnOff = 0;

		nTimerSettings.OlRstVolOnoff = 0;
		nTimerSettings.OlRstVol = 0;

		nTimerSettings.DrOccurOnOff = 0;
		nTimerSettings.DrOccurTimHr = 0;
		nTimerSettings.DrOccurTimMin = 0;
		nTimerSettings.DrOccurTimSec = 0;
		nTimerSettings.DrOccurNum = 0;

		nTimerSettings.DiffVoltII = 0;
		nTimerSettings.DiffVoltIII = 0;
		nTimerSettings.CalRVoltage = 0.625;
		nTimerSettings.CalYVoltage = 0.625;
		nTimerSettings.CalBVoltage = 0.625;
		nTimerSettings.CalRCurrent = 0.69;
		nTimerSettings.CalYCurrent = 0.69;
		nTimerSettings.CalBCurrent = 0.69;

		limitsmsonof = 1;
		limitsmsset=80;
		nMSettings.SMSOnOff = 1;
		nMSettings.SfbOnOff = 1;
		nMSettings.DryRunOnOff = 1;

		nTimerSettings.POnHr = 0;
		nTimerSettings.POnMin = 0;
		nTimerSettings.POnSec = 10;
		nTimerSettings.SDHr = 0;
		nTimerSettings.SDMin = 0;
		nTimerSettings.SDSec = 5;
		nTimerSettings.SfbHr = 0;
		nTimerSettings.SfbMin = 0;
		nTimerSettings.SfbSec = 20;

		nTimerSettings.DrScOnOf = 1;
		nTimerSettings.DrScHr = 0;
		nTimerSettings.DrScMin = 0;
		nTimerSettings.DrScSec = 20;
		nTimerSettings.OlOnOff = 1;

		nTimerSettings.OlScanHr = 0;
		nTimerSettings.OlScanMin = 0;
		nTimerSettings.OlScanSec = 10;

		nTimerSettings.OlAmpsII = 10.0;//40.0;
		nTimerSettings.OlAmpsIII = 8.0;//40.0;
		nTimerSettings.DrAmpsII = 6;
		nTimerSettings.DrAmpsIII = 5;

		nTimerSettings.DrReHr = 1;
		nTimerSettings.DrReMin = 0;
		nTimerSettings.DrReSec = 0;

		nTimerSettings.ScrDlOnOff = 1;
		nTimerSettings.ScrDlHr = 0;
		nTimerSettings.ScrDlMin = 0;
		nTimerSettings.ScrDlSec = 7;
		nTimerSettings.LowVoltOnOff = 1;
		nTimerSettings.LowVoltII = 370;
		nTimerSettings.LowVoltIII = 280;
		nTimerSettings.HighVoltOnOff = 1;
		nTimerSettings.HighVoltII = 560;
		nTimerSettings.HighVoltIII = 500;
		nTimerSettings.ImbVolt = 80;
		nTimerSettings.RvePhOnoff = 1;
		nTimerSettings.SppOnoff = 1;
		nTimerSettings.CurSppOnOff = 1;
		nTimerSettings.DiffVoltII = 80;
		nTimerSettings.DiffVoltIII = 40;
		nTimerSettings.HiDiffVoltIII = 20;
		nTimerSettings.HiDiffVoltII =20;
		nTimerSettings.CTRonoff = 1;
		nTimerSettings.CTYonoff = 1;
		nTimerSettings.CTBonoff = 1;
		nTimerSettings.AutoRst2On = 1;
		nTimerSettings.AutoRstOn = 1;
		nTimerSettings.AutoOlDrRstIIOnOff = 1;
		nTimerSettings.AutoDrRunRstIIOnOff=1;
		nTimerSettings.RPhaseToPhaseFactor = 1.0;
		nTimerSettings.pfcOnOff = 0;
		nTimerSettings.pfcvolt = 475;
		nTimerSettings.YPhaseToPhaseFactor = 1.0;
		nTimerSettings.BPhaseToPhaseFactor = 1.0;
		nTimerSettings.R2PhaseToPhaseFactor = 1.15;
		//zoneid[2].idcal=1.210;
		GREENT[1].startfrom=1;
		GREENT[2].startfrom=1;
		GREENT[3].startfrom=1;
		GREENT[4].startfrom=1;
		nVaTr.pswdelay=1;
		nMSettings.motor4ctrlonof=0;
		
		nMSettings.PressureOnOff =0;  // dg_changed
		nMSettings.TankOnOff = 0;	// dg_changed
		nMSettings.SumpOnOff = 0;	// dg_changed
							
		zonecom.valdelaymin=1;
		zonecom.valdelaysec=0;
		zonecom.fbkdelmin=1;
		zonecom.fbkdelsec=0;
		
		nVaTr.cregdelayHr=0;
		nVaTr.cregdelayMin=30;
		nVaTr.cregrstonof=1;
	//	nMSettings.count_controlonof=1;

		WriteTimerSettings();
		WriteSettingsFile();
		WriteonofFile();
		writeidcomset();

		}
		else if(strstr(smsbuffer,"ofcond") != 0 )
		{

		ReadTripConditions();
		//StopTimer(&timer);
		SendSmsRec(PhoneNumber);
		//timer.timeoutPeriod = 300;
		//timer.timerId = StartTimer(&timer);

		}
		else if(strstr(smsbuffer,"fixd#8185") != 0)
		{
		if( MakeRealyOn==0 )
		nDripSettings.startfrom =7;
		nDripSettings.prvstp=0;
		nVaTr.fertv1onof=0;
		nVaTr.fertv2onof=0;
		nVaTr.fertv3onof=0;
		nDripSettings.driponof=1;
		nVaTr.cycrestartonof=0;
		nVaTr.valvefeedbackonof=0;
		nVaTr.valvefbkTimHr=0;
		nVaTr.valvefbkTimMin=1;
		nVaTr.valvefbkTimSec=0;
		nVaTr.refreshvalveonof = 1;
		nVaTr.RefreshTimonHr =0;
		nVaTr.RefreshTimonMin =2;
		nVaTr.RefreshTimonSec =0;
		nVaTr.RefreshTimofHr =2;
		nVaTr.RefreshTimofMin =0;
		nVaTr.RefreshTimofSec =0;
		nDripSettings.tstartfrom=7;
		nDripSettings.prvstartfrom=7;
		nDripSettings.decidelast=32;
		nDripSettings.decidefirst=7;
		WriteDprevSettings();
		ReadDprevSettings();
		}
		else if(strstr(smsbuffer,"getdvset") != 0)
		{
		send_test_driptimsetsms(PhoneNumber);
		}
	/*	else if(strstr(smsbuffer,"valveon") != 0)
		{
		if( MakeRealyOn==0)
		{
		ReadDprevSettings();
		nDripSettings.driponof=1;
		nDripSettings.checkagain=1;
		WriteDprevSettings();
		STATE_SENDSMS=STATE_VALVEONOF_SMS;
		SendSmsToAll = 1;
		ReadDprevSettings();
		}
		}
		else if(strstr(smsbuffer,"valveof") != 0)
		{
		if( MakeRealyOn==0 )
		{
		ReadDprevSettings();
		nDripSettings.driponof=0;
		nDripSettings.checkagain=1;

		WriteDprevSettings();
		STATE_SENDSMS=STATE_VALVEONOF_SMS;
		SendSmsToAll = 1;
		ReadDprevSettings();
		}
		}
		else if(strstr(smsbuffer,"GOTOCYCLECOMPLETED") != 0 )
		{
		nVaTr.cyclecompleted=1;
		}
		else if(strstr(smsbuffer,"getskipdaycount") != 0 )
		{
		//nDripSettings.dripgapdaycount=nDripSettings.dripgapdaycount+1;
		//WriteDprevSettings();
		STATE_SENDSMS=	STATE_SKIPDAYCOUNT_SMS;
		SendSmsToAll = 1;
		}
		else if(strstr(smsbuffer,"skipdaycountplus") != 0 )
		{
		nDripSettings.dripgapdaycount=nDripSettings.dripgapdaycount+1;
		WriteDprevSettings();
		STATE_SENDSMS=	STATE_SKIPDAYCOUNT_SMS;
		SendSmsToAll = 1;
		}
		else if(strstr(smsbuffer,"skipdaycountminus") != 0 )
		{
		ReadDprevSettings();
		nDripSettings.dripgapdaycount=nDripSettings.dripgapdaycount-1;
		WriteDprevSettings();
		STATE_SENDSMS=	STATE_SKIPDAYCOUNT_SMS;
		SendSmsToAll = 1;
		}
		else if(strstr(smsbuffer,"skipdays") != 0 )
		{
		//if( nDripSettings.entersetting ==1 )
		//if(  MakeRealyOn==0 )
		if((MakeRealyOn==0) || (nVaTr.cyclecompleted ==1))
		{
		Tp = smsbuffer[8]-'0';
		nDripSettings.dripgapdays=Tp;
		WriteDprevSettings();
		ReadDprevSettings();
		STATE_SENDSMS=	STATE_GAPDAYS_SMS;
		SendSmsToAll = 1;
		}
		else
		{
		ReadDprevSettings();
		send_2DripSystem(PhoneNumber);
		}
		}

		else if(strstr(smsbuffer,"skipdayon") != 0 )
		{
		//if( nDripSettings.entersetting ==1)
		{
		ReadDprevSettings();
		nDripSettings.dripgapdayonof=1;
		WriteDprevSettings();
		STATE_SENDSMS=STATE_DRIPGAPDAYONOF_SMS;
		SendSmsToAll = 1;
		ReadDprevSettings();
		}
		}
		else if(strstr(smsbuffer,"skipdayof") != 0 )
		{
		//if( nDripSettings.entersetting ==1)
		//if(  MakeRealyOn==0 )
		if((MakeRealyOn==0) || (nVaTr.cyclecompleted ==1))
		{
		ReadDprevSettings();
		nDripSettings.dripgapdayonof=0;
		WriteDprevSettings();
		STATE_SENDSMS=STATE_DRIPGAPDAYONOF_SMS;
		SendSmsToAll = 1;
		ReadDprevSettings();
		}
		}
		else if(strstr(smsbuffer,"enterdripseton") != 0)  
		{
		ReadDprevSettings();
		nDripSettings.entersetting=1;
		WriteDprevSettings();
		STATE_SENDSMS=	STATE_ENTERDRIPSETON_SMS;
		SendSmsToAll = 1;
		ReadDprevSettings();

		}
		else if(strstr(smsbuffer,"enterdripsetof") != 0)
		{

		ReadDprevSettings();
		nDripSettings.entersetting=0;
		WriteDprevSettings();
		STATE_SENDSMS=	STATE_ENTERDRIPSETON_SMS;
		SendSmsToAll = 1;
		ReadDprevSettings();

		}

	/*	else if(strstr(smsbuffer,"startfrom") != 0 )
		{
		//if( nDripSettings.entersetting ==1 && MakeRealyOn==0 && nDripSettings.driponof ==0)
		if( MakeRealyOn==0 )
		{
		ReadDprevSettings();
		Tp = smsbuffer[9]-'0';
		Tp1 = smsbuffer[10]-'0';
		Tp2 = smsbuffer[11]-'0';
		nDripSettings.tstartfrom=(Tp*100)+(Tp1*10)+Tp2;
		sprintf(buf,"tstart =%ld ",nDripSettings.tstartfrom );
		 PrintfResp(buf);
		if(nDripSettings.tstartfrom<7)
		nDripSettings.tstartfrom=7;
		if(nDripSettings.tstartfrom>128)
		nDripSettings.tstartfrom=128;
		nDripSettings.startcontrol=1;
		nDripSettings.startcontrolt=1;
		sprintf(buf,"tstartwb =%ld ",nDripSettings.tstartfrom );
		 PrintfResp(buf);
		WriteDprevSettings();
		ReadDprevSettings();
		sprintf(buf,"tstartra =%ld ",nDripSettings.tstartfrom );
		 PrintfResp(buf);
		}
		else
		{
		//ReadDprevSettings();
		send_2DripSystem(PhoneNumber);
		}
		}*/
		
		/* else if(strstr(smsbuffer,"changefrom") != 0 )
		{
		//if( nDripSettings.entersetting ==1 && MakeRealyOn==0 && nDripSettings.driponof ==0)
		//if( MakeRealyOn==0 )
		//{
	//	readidcomset();
	//	if(zonecom.standalonemodeonof==0)
	//	{
		ReadDprevSettings();
		Tp = smsbuffer[10]-'0';
		Tp1 = smsbuffer[11]-'0';
		Tp2 = smsbuffer[12]-'0';
		nDripSettings.changefrom=(Tp*100)+(Tp1*10)+Tp2;
		nDripSettings.changefromflag=1;
		STATE_SENDSMS=	STATE_CHANGEFROM_SMS;
		SendSmsToAll = 1;
	//	}

		//}
		//else
		//{
		//ReadDprevSettings();
		//send_2DripSystem(PhoneNumber);
		//}
		} */
		/* else if(strstr(smsbuffer,"fertonof") != 0)		//fertonof,10,21,31
		{
			PrintfResp("<<<<<<<<<<<<<<<<<<<<<<<<<FERT ON OFF>>>>>>>>>>>>>>>>>>>>>>");
		ReadDprevSettings();
		nVaTr.fertv1onof=smsbuffer[10]-'0';
		nVaTr.fertv2onof=smsbuffer[13]-'0';
		nVaTr.fertv3onof=smsbuffer[16]-'0';
		nVaTr.fertv4onof=smsbuffer[19]-'0';
		WriteDprevSettings();
		ReadDprevSettings();
		Sendfert(PhoneNumber);

		} */
		
		
		/* else if(strstr(smsbuffer,"lastfertskip") != 0 )
		{
		//if( nDripSettings.entersetting ==1 && MakeRealyOn==0 && nDripSettings.driponof ==0)
	   if( MakeRealyOn==0 )
	//	if((MakeRealyOn==0) || (nVaTr.cyclecompleted ==1))

		{
		ReadDprevSettings();
		Tp3 = smsbuffer[12]-'0';
		Tp = smsbuffer[14]-'0';
		Tp1 = smsbuffer[15]-'0';
		Tp2 = smsbuffer[16]-'0';
		if(Tp3==1)
		nDripSettings.fertskiplast1=(Tp*100)+(Tp1*10)+Tp2;
		if(Tp3==2)
		nDripSettings.fertskiplast2=(Tp*100)+(Tp1*10)+Tp2;
		if(Tp3==3)
		nDripSettings.fertskiplast3=(Tp*100)+(Tp1*10)+Tp2;
		WriteDprevSettings();

		STATE_SENDSMS=	STATE_FERTSKIPGROUPDETAILS_SMS;
		SendSmsToAll = 1;
		ReadDprevSettings();
		}
		else
		{
		ReadDprevSettings();
		send_2DripSystem(PhoneNumber);
		}
		} */
		/* else if(strstr(smsbuffer,"firstfertskip") != 0 )
		{
		//if( nDripSettings.entersetting ==1 && MakeRealyOn==0 && nDripSettings.driponof ==0)
		//if( MakeRealyOn==0 )
		if((MakeRealyOn==0) || (nVaTr.cyclecompleted ==1))

		{
		ReadDprevSettings();
		Tp3 = smsbuffer[13]-'0';
		Tp = smsbuffer[15]-'0';
		Tp1 = smsbuffer[16]-'0';
		Tp2 = smsbuffer[17]-'0';
		if(Tp3==1)
		nDripSettings.fertskipfirst1=(Tp*100)+(Tp1*10)+Tp2;
		if(Tp3==2)
		nDripSettings.fertskipfirst2=(Tp*100)+(Tp1*10)+Tp2;
		if(Tp3==3)
		nDripSettings.fertskipfirst3=(Tp*100)+(Tp1*10)+Tp2;
		WriteDprevSettings();

		STATE_SENDSMS=	STATE_FERTSKIPGROUPDETAILS_SMS;
		SendSmsToAll = 1;
		ReadDprevSettings();
		}
		else
		{
		ReadDprevSettings();
		send_2DripSystem(PhoneNumber);
		}
		} */
		/* else if(strstr(smsbuffer,"splitfertskip") != 0 )
		{
		//if( nDripSettings.entersetting ==1 && MakeRealyOn==0 && nDripSettings.driponof ==0)
		//if(  MakeRealyOn==0 )
		if((MakeRealyOn==0) || (nVaTr.cyclecompleted ==1))

		{
		Tp3=smsbuffer[13]-'0';
		ReadDprevSettings();
		if(Tp3==0)
		{
		Tp = smsbuffer[15]-'0';
		Tp1 = smsbuffer[16]-'0';
		Tp2 = smsbuffer[17]-'0';
		nDripSettings.fertskip1 = (Tp*100)+(Tp1*10)+Tp2;

		Tp = smsbuffer[19]-'0';
		Tp1 = smsbuffer[20]-'0';
		Tp2 = smsbuffer[21]-'0';
		nDripSettings.fertskip2 = (Tp*100)+(Tp1*10)+Tp2;
		Tp = smsbuffer[23]-'0';
		Tp1 = smsbuffer[24]-'0';
		Tp2 = smsbuffer[25]-'0';
		nDripSettings.fertskip3 = (Tp*100)+(Tp1*10)+Tp2;
		Tp = smsbuffer[27]-'0';
		Tp1 = smsbuffer[28]-'0';
		Tp2 = smsbuffer[29]-'0';
		nDripSettings.fertskip4 = (Tp*100)+(Tp1*10)+Tp2;
		Tp = smsbuffer[31]-'0';
		Tp1 = smsbuffer[32]-'0';
		Tp2 = smsbuffer[33]-'0';
		nDripSettings.fertskip5 = (Tp*100)+(Tp1*10)+Tp2;
		Tp = smsbuffer[35]-'0';
		Tp1 = smsbuffer[36]-'0';
		Tp2 = smsbuffer[37]-'0';
		nDripSettings.fertskip6 = (Tp*100)+(Tp1*10)+Tp2;
		Tp = smsbuffer[39]-'0';
		Tp1 = smsbuffer[40]-'0';
		Tp2 = smsbuffer[41]-'0';
		nDripSettings.fertskip7 = (Tp*100)+(Tp1*10)+Tp2;
		Tp = smsbuffer[43]-'0';
		Tp1 = smsbuffer[44]-'0';
		Tp2 = smsbuffer[45]-'0';
		nDripSettings.fertskip8 = (Tp*100)+(Tp1*10)+Tp2;
		WriteDprevSettings();
		ReadDprevSettings();
		STATE_SENDSMS=	STATE_FERTSKIPDETAILS_SMS;
		SendSmsToAll = 1;

		}
		}
		else
		{
		ReadDprevSettings();
		send_2DripSystem(PhoneNumber);
		}
		} */
		/* else if(strstr(smsbuffer,"decidelast") != 0 )
		{
		//if( nDripSettings.entersetting ==1 && MakeRealyOn==0 && nDripSettings.driponof ==0)
		//if( MakeRealyOn==0 )

		//{
		ReadDprevSettings();
		Tp = smsbuffer[10]-'0';
		Tp1 = smsbuffer[11]-'0';
		Tp2 = smsbuffer[12]-'0';
		nDripSettings.decidelast=(Tp*100)+(Tp1*10)+Tp2;

        if(nDripSettings.decidelast<1)
		nDripSettings.decidelast=64;
		if(nDripSettings.decidelast>64)
		nDripSettings.decidelast=64;
		WriteDprevSettings();
		ReadDprevSettings(); 
		if(nDripSettings.decidelast<nDripSettings.startfrom)
		{
		nDripSettings.tstartfrom=1;
		nDripSettings.startcontrolt=1;
		STATE_SENDSMS=	STATE_DECIDELAST_SMS;
		SendSmsToAll = 1;

		}
		} */
		/* else if(strstr(smsbuffer,"decidefblast") != 0 )
		{
		//if(  nDripSettings.entersetting ==1 && MakeRealyOn==0 &&  nDripSettings.driponof ==0)
		//if( MakeRealyOn==0 )

		//{
		ReadDprevSettings();
		Tp = smsbuffer[12]-'0';
		Tp1 = smsbuffer[13]-'0';
		Tp2 = smsbuffer[14]-'0';
		 nDripSettings.decidefblast=(Tp*100)+(Tp1*10)+Tp2;

		 if( nDripSettings.decidefblast<1)
		 nDripSettings.decidefblast=255;
		 if( nDripSettings.decidefblast>=255)
		 nDripSettings.decidefblast=1;
		WriteDprevSettings();
		ReadDprevSettings();
		
		STATE_SENDSMS=	STATE_DECIDELAST_SMS;
		SendSmsToAll = 1;

		} */
		/* else if(strstr(smsbuffer,"decidefirst") != 0 )
		{
		//if( nDripSettings.entersetting ==1 && MakeRealyOn==0 && nDripSettings.driponof ==0)
		if( MakeRealyOn==0 )
		{
		ReadDprevSettings();
		Tp = smsbuffer[11]-'0';
		Tp1 = smsbuffer[12]-'0';
		Tp2 = smsbuffer[13]-'0';
		nDripSettings.decidefirst=(Tp*100)+(Tp1*10)+Tp2;

		if(nDripSettings.decidefirst<7)
		nDripSettings.decidefirst=7;
		if(nDripSettings.decidefirst>128)
		nDripSettings.decidefirst=7;
		WriteDprevSettings();
		ReadDprevSettings();
		if(	nDripSettings.decidefirst>nDripSettings.startfrom)
		{
		nDripSettings.tstartfrom=7;
		nDripSettings.startcontrolt=1;
		}

		STATE_SENDSMS=	STATE_DECIDEFIRST_SMS;
		SendSmsToAll = 1;

		}
		else
		{
		ReadDprevSettings();
		send_2DripSystem(PhoneNumber);
		}
		} */
		/* else if(strstr((char *)smsbuffer,"fogcycrston") != 0 )
		{
		readidcomset();
		nVaTr.fogcycrestartonof=1;
		nVaTr.fogcyclecompleted=0;
		writeidcomset();
		 //STATE_SENDSMS=	STATE_CYCRESTARTONOF_SMS;
		 //SendSmsToAll = 1;
		readidcomset();
		send_vfogstatset(PhoneNumber);
		foggeron(PhoneNumber);
		} */
		/* else if(strstr((char *)smsbuffer,"foggeron") != 0 )
		{
		PrintfResp("=============FOGGER ON SUCCESSFULLY=============");	
		foggeron(PhoneNumber);
		} */
		/* else if(strstr((char *)smsbuffer,"fogcycrstof") != 0 )
		{
		readidcomset();
		nVaTr.fogcycrestartonof=0;
		nVaTr.fogcyclecompleted=0;
		writeidcomset();
		//STATE_SENDSMS=	STATE_CYCRESTARTONOF_SMS;
		//SendSmsToAll = 1;
		readidcomset();
		send_vfogstatset(PhoneNumber);
		} */


		/* else if(strstr(smsbuffer,"cycrston") != 0)
		{
		ReadDprevSettings();
		nVaTr.cycrestartonof=1;
		nVaTr.cyclecompleted=0;
		WriteonofFile();

		WriteDprevSettings();
		STATE_SENDSMS=	STATE_CYCRESTARTONOF_SMS;
		SendSmsToAll = 1;
		ReadDprevSettings();
		} */
		/* else if(strstr(smsbuffer,"cycrstof") != 0)
		{
		ReadDprevSettings();
		nVaTr.cycrestartonof=0;
		nVaTr.cyclecompleted=0;
		WriteonofFile();

		WriteDprevSettings();
		STATE_SENDSMS=	STATE_CYCRESTARTONOF_SMS;
		SendSmsToAll = 1;
		ReadDprevSettings();
		} */
/* else if(strstr(smsbuffer,"querston") != 0)
		{
		ReadDprevSettings();
		nVaTr.querestartonof=1;
		//nVaTr.cyclecompleted=0;
		WriteDprevSettings();
		STATE_SENDSMS=	STATE_QUERESTARTONOF_SMS;
		SendSmsToAll = 1;
		ReadDprevSettings();
		} */
		/* else if(strstr(smsbuffer,"querstof") != 0)
		{
		ReadDprevSettings();
		nVaTr.querestartonof=0;
		//nVaTr.cyclecompleted=0;
		WriteDprevSettings();
		STATE_SENDSMS=	STATE_QUERESTARTONOF_SMS;
		SendSmsToAll = 1;
		ReadDprevSettings();
		} */

		/* else if(strstr(smsbuffer,"progsel") != 0)
		{
		//ReadDripSettings1();
		//ReadDripSettings2();
		ReadDprevSettings();
		Tp = smsbuffer[7]-'0';
		//Tp3 = smsbuffer[9]-'0';
		if(Tp>4|| Tp<1)
		Tp=1;
	    motorof (PhoneNumber);
		GREENT[nVaTr.programselection].stpstartfrom=nDripSettings.stp;     //dg_added
		GREENT[nVaTr.programselection].ststpstartfrom=nDripSettings.ststp;	//dg_added
		nVaTr.programselection=Tp;
		nDripSettings.stp=GREENT[nVaTr.programselection].stpstartfrom;
		if(nDripSettings.stp<=0)
			nDripSettings.stp=1;
		nDripSettings.ststp=GREENT[nVaTr.programselection].ststpstartfrom;
	    if(nDripSettings.ststp<=0)
			nDripSettings.ststp=1;//dg_added
		nDripSettings.changeinstp1=0;
		nDripSettings.changeinstp=0;

		nDripSettings.changeincalc=0;
		nVaTr.Currentvalve = nDripSettings.stp;
		dripcyclecount=0;
		dripcycledate=0;
		nVaTr.REMTIM=0;
		nDripSettings.checkagain=1;//dg_added
		sprintf(buf,"\n pos=%d\n\r",nVaTr.programselection);
		 PrintfResp(buf);
		WriteDprevSettings();
		sprintf(buf,"\n pos=%d\n\r",nVaTr.programselection);
		 PrintfResp(buf);
		ReadDprevSettings();
		sprintf(buf,"\n pos=%d\n\r",nVaTr.programselection);
		 PrintfResp(buf);
		STATE_SENDSMS=STATE_PROGRAMSEL_SMS;
		SendSmsToAll = 1;
		Enter=1;
		//WriteValremtimSettings();
		//ReadDripSettings(nVaTr.programselection);
		} */


		 /*else if(strstr(smsbuffer,"fertonof") != 0)		//fertonof,10,21,31
		{
		ReadDprevSettings();
		nVaTr.fertv1onof=smsbuffer[10]-'0';
		nVaTr.fertv2onof=smsbuffer[13]-'0';
		nVaTr.fertv3onof=smsbuffer[16]-'0';
		WriteDprevSettings();
		//ReadDprevSettings();
		Sendfert(PhoneNumber);

		}*/
		/* else if(strstr(smsbuffer,"calflowrateon") != 0)
		{
		ReadDprevSettings();
		nVaTr.calflowrateonof=1;
		WriteDprevSettings();
		STATE_SENDSMS=STATE_CALFLOWONOF_SMS;
		SendSmsToAll = 1;
		ReadDprevSettings();
		}
		else if(strstr(smsbuffer,"calflowrateof") != 0)
		{
		ReadDprevSettings();
		nVaTr.calflowrateonof=0;
		WriteDprevSettings();
		STATE_SENDSMS=STATE_CALFLOWONOF_SMS;
		SendSmsToAll = 1;
		ReadDprevSettings();
		}
		else if(strstr(smsbuffer,"delvalfbon") != 0)
		{
		ReadDprevSettings();
		nVaTr.delvalvefeedbackonof=1;
		WriteDprevSettings();
		STATE_SENDSMS=STATE_DELVALFBONOF_SMS;
		SendSmsToAll = 1;
		ReadDprevSettings();
		}
		else if(strstr(smsbuffer,"delvalfbof") != 0)
		{
		ReadDprevSettings();
		nVaTr.delvalvefeedbackonof=0;
		WriteDprevSettings();
		STATE_SENDSMS=STATE_DELVALFBONOF_SMS;
		SendSmsToAll = 1;
		ReadDprevSettings();
		}
		else if(strstr(smsbuffer,"valfbon") != 0)
		{
		ReadDprevSettings();
		nVaTr.valvefeedbackonof=1;
		WriteDprevSettings();
		STATE_SENDSMS=STATE_VALFBONOF_SMS;
		SendSmsToAll = 1;
		ReadDprevSettings();
		}
		else if(strstr(smsbuffer,"valfbof") != 0)
		{
		ReadDprevSettings();
		nVaTr.valvefeedbackonof=0;
		WriteDprevSettings();
		STATE_SENDSMS=STATE_VALFBONOF_SMS;
		SendSmsToAll = 1;
		ReadDprevSettings();
		}
		
		else if(strstr(smsbuffer,"valfbktim") != 0)
		{
		ReadDprevSettings();
		Tp = smsbuffer[9]-'0';
		Tp3 = smsbuffer[10]-'0';
		nVaTr.valvefbkTimHr = (Tp*10)+Tp3;

		Tp = smsbuffer[11]-'0';
		Tp3 = smsbuffer[12]-'0';
		nVaTr.valvefbkTimMin = (Tp*10)+Tp3;

		Tp = smsbuffer[13]-'0';
		Tp3 = smsbuffer[14]-'0';

		nVaTr.valvefbkTimSec = (Tp*10)+Tp3;

		WriteDprevSettings();

		STATE_SENDSMS=STATE_VALFBKTIM_SMS;
		SendSmsToAll = 1;
		ReadDprevSettings();
		} */
		/*  else if(strstr(smsbuffer,"refreshon") != 0)
		{

		ReadDprevSettings();
		nVaTr.refreshvalveonof=1;
		WriteDprevSettings();
		STATE_SENDSMS=STATE_REFRESHONOF_SMS;
		SendSmsToAll = 1;
		ReadDprevSettings();
		}  
		
		
		else if(strstr(smsbuffer,"refreshonof") != 0)		//fertonof,10,21,31 
		{
		ReadDprevSettings();   //refreshonof,10,21,31,40
		nVaTr.refreshvalveonof1=smsbuffer[13]-'0';
		nVaTr.refreshvalveonof2=smsbuffer[16]-'0';
		nVaTr.refreshvalveonof3=smsbuffer[19]-'0';
		nVaTr.refreshvalveonof4=smsbuffer[22]-'0';
		if(nVaTr.refreshvalveonof1==1)
		Last_filter=1;
		if(nVaTr.refreshvalveonof2==1)
		Last_filter=3;
		if(nVaTr.refreshvalveonof3==1)
		Last_filter=5;
		if(nVaTr.refreshvalveonof4==1)
		Last_filter=7;
		sprintf(buf,"Last_filter is %d",Last_filter);
		 PrintfResp(buf);
		WriteDprevSettings();
		ReadDprevSettings();
		SendRefreshValve(PhoneNumber);

		}
		
		
		 else if(strstr(smsbuffer,"refreshof") != 0)
		{
		ReadDprevSettings();
		nVaTr.refreshvalveonof=0;
		WriteDprevSettings();
		STATE_SENDSMS=STATE_REFRESHONOF_SMS;
		SendSmsToAll = 1;
		ReadDprevSettings();
		}  
		else if(strstr(smsbuffer,"reftimon") != 0)
		{
		ReadDprevSettings();
		Tp = smsbuffer[8]-'0';
		Tp3 = smsbuffer[9]-'0';
		nVaTr.RefreshTimonHr1 = (Tp*10)+Tp3;

		Tp = smsbuffer[10]-'0';
		Tp3 = smsbuffer[11]-'0';
		nVaTr.RefreshTimonMin1 = (Tp*10)+Tp3;

		Tp = smsbuffer[12]-'0';
		Tp3 = smsbuffer[13]-'0';

		nVaTr.RefreshTimonSec1 = (Tp*10)+Tp3;
		
		
		Tp = smsbuffer[15]-'0';
		Tp3 = smsbuffer[16]-'0';
		nVaTr.RefreshTimonHr2 = (Tp*10)+Tp3;

		Tp = smsbuffer[17]-'0';
		Tp3 = smsbuffer[18]-'0';
		nVaTr.RefreshTimonMin2 = (Tp*10)+Tp3;

		Tp = smsbuffer[19]-'0';
		Tp3 = smsbuffer[20]-'0';

		nVaTr.RefreshTimonSec2 = (Tp*10)+Tp3;
		
		Tp = smsbuffer[22]-'0';
		Tp3 = smsbuffer[23]-'0';
		nVaTr.RefreshTimonHr3 = (Tp*10)+Tp3;

		Tp = smsbuffer[24]-'0';
		Tp3 = smsbuffer[25]-'0';
		nVaTr.RefreshTimonMin3 = (Tp*10)+Tp3;

		Tp = smsbuffer[26]-'0';
		Tp3 = smsbuffer[27]-'0';

		nVaTr.RefreshTimonSec3 = (Tp*10)+Tp3;
		
		Tp = smsbuffer[29]-'0';
		Tp3 = smsbuffer[30]-'0';
		nVaTr.RefreshTimonHr4 = (Tp*10)+Tp3;

		Tp = smsbuffer[31]-'0';
		Tp3 = smsbuffer[32]-'0';
		nVaTr.RefreshTimonMin4 = (Tp*10)+Tp3;

		Tp = smsbuffer[33]-'0';
		Tp3 = smsbuffer[34]-'0';

		nVaTr.RefreshTimonSec4 = (Tp*10)+Tp3;

		WriteDprevSettings();
		nVaTr.RefOnDelay[1] = (nVaTr.RefreshTimonHr1*3600)+(nVaTr.RefreshTimonMin1*60)+nVaTr.RefreshTimonSec1;
		nVaTr.RefOnDelay[3] = (nVaTr.RefreshTimonHr2*3600)+(nVaTr.RefreshTimonMin2*60)+nVaTr.RefreshTimonSec2;
		nVaTr.RefOnDelay[5] = (nVaTr.RefreshTimonHr3*3600)+(nVaTr.RefreshTimonMin3*60)+nVaTr.RefreshTimonSec3;
		nVaTr.RefOnDelay[7] = (nVaTr.RefreshTimonHr4*3600)+(nVaTr.RefreshTimonMin4*60)+nVaTr.RefreshTimonSec4;
		
		sprintf(buf,"RefOnDelay1 is %ld ",nVaTr.RefOnDelay[1]);
		 PrintfResp(buf);
		sprintf(buf,"RefOnDelay2 is %ld ",nVaTr.RefOnDelay[3]);
		 PrintfResp(buf);
		sprintf(buf,"RefOnDelay3 is %ld ",nVaTr.RefOnDelay[5]);
		 PrintfResp(buf);
		sprintf(buf,"RefOnDelay4 is %ld ",nVaTr.RefOnDelay[7]);
 PrintfResp(buf);
		STATE_SENDSMS=STATE_VREFRSHTIMON_SMS;
		SendSmsToAll = 1;
		ReadDprevSettings();
		}
		else if(strstr(smsbuffer,"reftimof") != 0)
		{
		ReadDprevSettings();
		Tp = smsbuffer[8]-'0';
		Tp3 = smsbuffer[9]-'0';
		nVaTr.RefreshTimofHr = (Tp*10)+Tp3;

		Tp = smsbuffer[10]-'0';
		Tp3 = smsbuffer[11]-'0';
		nVaTr.RefreshTimofMin = (Tp*10)+Tp3;

		Tp = smsbuffer[12]-'0';
		Tp3 = smsbuffer[13]-'0';

		nVaTr.RefreshTimofSec = (Tp*10)+Tp3;

		WriteDprevSettings();
		nVaTr.RefOfDelay = (nVaTr.RefreshTimofHr*3600)+(nVaTr.RefreshTimofMin*60)+nVaTr.RefreshTimofSec;
		sprintf(buf,"RefOfDelay is %ld ",nVaTr.RefOfDelay);
		 PrintfResp(buf);
		STATE_SENDSMS=STATE_VREFRSHTIMOF_SMS;
		SendSmsToAll = 1;
		ReadDprevSettings();
		}
		else if(strstr(smsbuffer,"vidzoneselp") != 0)
{
Tp1 = smsbuffer[11]-'0';
program= Tp1;
sprintf(buf,"\n pos=%d\n\r",program);
 PrintfResp(buf);
Tp1 = smsbuffer[12]-'0';
Tp2 = smsbuffer[13]-'0';
Tp3 = smsbuffer[14]-'0';
i = (Tp1*100)+(Tp2*10)+Tp3;
 
if(program<=4 && i<=64)
{
 readidzoneset(program,1);
 readidzoneset(program,2);
 readidzoneset(program,3);
 readidzoneset(program,4);

 
//StopTimer(&timer);
sprintf(BigSMS1,"V02");
send_vzoneidset(PhoneNumber,program,i);
//timer.timeoutPeriod = 300;
//timer.timerId = StartTimer(&timer);
}
}
else if(strstr(smsbuffer,"vreftim") !=0 )
{
	ReadDprevSettings();
	sprintf(BigSMS1,"V01");
	send_vreftim(PhoneNumber);
	
}
else if(strstr(smsbuffer,"vidset") != 0 ) 
{


Tp1 = smsbuffer[6]-'0';
Tp2 = smsbuffer[7]-'0';
Tp3 = smsbuffer[8]-'0';
i = (Tp1*100)+(Tp2*10)+Tp3;
sprintf(buf,"\n pos=%d\n\r",i);
 PrintfResp(buf);
if(i<=254)
{
 readidset(1);
 readidset(2);
 readidset(3);
 readidset(4);
 sprintf(BigSMS1,"V01");
send_vidset(PhoneNumber,i);
}

}

else if(strstr(smsbuffer,"valvedelmsgtim") != 0 ) 
{
		ReadDprevSettings();
		Tp = smsbuffer[14]-'0';
		Tp3 = smsbuffer[15]-'0';
		nDripSettings.strunsmscountHr = (Tp*10)+Tp3;

		Tp = smsbuffer[16]-'0';
		Tp3 = smsbuffer[17]-'0';
		nDripSettings.strunsmscountMin = (Tp*10)+Tp3;

		Tp = smsbuffer[18]-'0';
		Tp3 = smsbuffer[19]-'0';

		nDripSettings.strunsmscountSec = (Tp*10)+Tp3;
		
nDripSettings.strunsmscounter  = (nDripSettings.strunsmscountHr*3600)+(nDripSettings.strunsmscountMin*60)+nDripSettings.strunsmscountSec;
WriteDprevSettings();
ReadDprevSettings();
send_stflagcount(PhoneNumber);


}

else if(strstr(smsbuffer,"idset") != 0 ) 
{


Tp1 = smsbuffer[5]-'0';
Tp2 = smsbuffer[6]-'0';
Tp3 = smsbuffer[7]-'0';
i = (Tp1*100)+(Tp2*10)+Tp3;
sprintf(buf,"\n pos=%d\n\r",i);
 PrintfResp(buf);
if(i<=254)
{
 readidset(1);
 readidset(2);
 readidset(3);
 readidset(4);
 Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, ","  );
}


sprintf(buf,"\n pos=%d\n\r",i);
 PrintfResp(buf);
sprintf(buf,"\n pod=%d\n\r",StrTokStrVer);
 PrintfResp(buf);

zoneid[i].ida1 =myatoi(StrTokStr1[1]);
sprintf(buf,"\n pos=%d\n\r",zoneid[i].ida1);
 PrintfResp(buf);
zoneid[i].ida2 = myatoi(StrTokStr1[2]);

sprintf(buf,"\n pos=%d\n\r",zoneid[i].ida2);
 PrintfResp(buf);
zoneid[i].ida3 = myatoi(StrTokStr1[3]);
sprintf(buf,("\n pos=%d\n\r",zoneid[i].ida3);
 PrintfResp(buf);
zoneid[i].ida4 = myatoi(StrTokStr1[4]);
sprintf(buf,"\n pos=%d\n\r",zoneid[i].ida4);
 PrintfResp(buf);

if(i<=64)
{
writeidset(1);
readidset(1);
}
if(i>=65 && i<=128)
{
writeidset(2);
readidset(2);
}
if(i>=129 && i<=192)
{
writeidset(3);
readidset(3);
}
if(i>=193 && i<=256)
{
writeidset(4);
readidset(4);
}

sprintf(BigSMS1,"");
send_vidset(PhoneNumber,i);
}

}
else if(strstr(smsbuffer,"vidcalset") != 0 ) 
{


Tp1 = smsbuffer[9]-'0';
Tp2 = smsbuffer[10]-'0';
Tp3 = smsbuffer[11]-'0';
i = (Tp1*100)+(Tp2*10)+Tp3;
sprintf(buf,"\n pos=%d\n\r",i);
 PrintfResp(buf);
if(i<=254)
{
if(i<129)
{
readidcalset(1);
}
if(i>128)
{
readidcalset(2);
}
sprintf(BigSMS1,"V31");
send_vidcalset(PhoneNumber,i);
}

}

else if(strstr(smsbuffer,"idcalset") != 0 ) 
{


Tp1 = smsbuffer[8]-'0';
Tp2 = smsbuffer[9]-'0';
Tp3 = smsbuffer[10]-'0';
i = (Tp1*100)+(Tp2*10)+Tp3;
sprintf(buf,"\n pos=%d\n\r",i);
 PrintfResp(buf);
if(i<=254)
{
if(i<129)
{
readidcalset(1);
}
if(i>128)
{
readidcalset(2);
}
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, ","  );
}


sprintf(buf,"\n pos=%s\n\r",StrTokStr1[1]);
 PrintfResp(buf);
zoneid[i].idcal = myatof(StrTokStr1[1]);
sprintf(buf,"\n pos=%0.03f\n\r",zoneid[i].idcal);
 PrintfResp(buf);

if(i<129)
{
writeidcalset(1);
readidcalset(1);
}
if(i>128)
{
writeidcalset(2);
readidcalset(2);
}
sprintf(BigSMS1,"");
send_vidcalset(PhoneNumber,i);
}

}

else if(strstr(smsbuffer,"idzoneselp") != 0 ) 
{


Tp1 = smsbuffer[10]-'0';
program= Tp1;
sprintf(buf,"\n pos=%d\n\r",program);
 PrintfResp(buf);
Tp1 = smsbuffer[11]-'0';
Tp2 = smsbuffer[12]-'0';
Tp3 = smsbuffer[13]-'0';
i = (Tp1*100)+(Tp2*10)+Tp3;

if(program<=4 && i<=64)
{
 
 readidzoneset(program,1);
 readidzoneset(program,2);
 readidzoneset(program,3);
 readidzoneset(program,4);
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, ","  );
}


sprintf(buf,"\n pos=%d\n\r",i);
 PrintfResp(buf);
sprintf(buf,"\n pod=%d\n\r",StrTokStrVer);
 PrintfResp(buf);
for(Tp=0;Tp<=StrTokStrVer;Tp++)
{
 j=(Tp+1);
//sprintf(buf,"\n pod=%s\n\r",StrTokStr1[j]);
 PrintfResp(buf);
Tp1 = StrTokStr1[j][0]-'0';
//sprintf(buf,"\n pod=%d\n\r",Tp1);
 PrintfResp(buf);
Tp2 = StrTokStr1[j][1]-'0';
//sprintf(buf,"\n pod=%d\n\r",Tp2);
 PrintfResp(buf);
Tp3 = StrTokStr1[j][2]-'0';
//sprintf(buf,"\n pod=%d\n\r",Tp3);
 PrintfResp(buf);

if(Tp==0)
{
if(program==1)
zone[i].p1v1id = myatoi(StrTokStr1[1]);
if(program==2)
zone[i].p2v1id = myatoi(StrTokStr1[1]);
if(program==3)
zone[i].p3v1id = myatoi(StrTokStr1[1]);
if(program==4)
zone[i].p4v1id = myatoi(StrTokStr1[1]);

}
if(Tp==1)
{
if(program==1)
zone[i].p1v2id = myatoi(StrTokStr1[2]);
if(program==2)
zone[i].p2v2id = myatoi(StrTokStr1[2]);
if(program==3)
zone[i].p3v2id = myatoi(StrTokStr1[2]);
if(program==4)
zone[i].p4v2id = myatoi(StrTokStr1[2]);

}
if(Tp==2)
{
if(program==1)
zone[i].p1v3id = myatoi(StrTokStr1[3]);
if(program==2)
zone[i].p2v3id = myatoi(StrTokStr1[3]);
if(program==3)
zone[i].p3v3id = myatoi(StrTokStr1[3]);
if(program==4)
zone[i].p4v3id = myatoi(StrTokStr1[3]);

}
if(Tp==3)
{
if(program==1)
zone[i].p1v4id = myatoi(StrTokStr1[4]);
if(program==2)
zone[i].p2v4id = myatoi(StrTokStr1[4]);
if(program==3)
zone[i].p3v4id = myatoi(StrTokStr1[4]);
if(program==4)
zone[i].p4v4id = myatoi(StrTokStr1[4]);

}



 
if(Tp==0)
{
if(program==1)
sprintf(buf,"\n pos=%d,\n\r",zone[i].p1v1id);
 PrintfResp(buf);
if(program==2)
sprintf(buf,"\n pos=%d,\n\r",zone[i].p2v1id);
 PrintfResp(buf);
if(program==3)
sprintf(buf,"\n pos=%d,\n\r",zone[i].p3v1id);
 PrintfResp(buf);
if(program==4)
sprintf(buf,"\n pos=%d,\n\r",zone[i].p4v1id);
 PrintfResp(buf);
}
if(Tp==1)
{
if(program==1)
sprintf(buf,"\n pos=%d,\n\r",zone[i].p1v2id);
 PrintfResp(buf);
if(program==2)
sprintf(buf,"\n pos=%d,\n\r",zone[i].p2v2id);
 PrintfResp(buf);
if(program==3)
sprintf(buf,"\n pos=%d,\n\r",zone[i].p3v2id);
 PrintfResp(buf);
if(program==4)
sprintf(buf,"\n pos=%d,\n\r",zone[i].p4v2id);
 PrintfResp(buf);
}
if(Tp==2)
{
if(program==1)
sprintf(buf,"\n pos=%d,\n\r",zone[i].p1v3id);
 PrintfResp(buf);
if(program==2)
sprintf(buf,"\n pos=%d,\n\r",zone[i].p2v3id);
 PrintfResp(buf);
if(program==3)
sprintf(buf,"\n pos=%d,\n\r",zone[i].p3v3id);
 PrintfResp(buf);
if(program==4)
sprintf(buf,"\n pos=%d,\n\r",zone[i].p4v3id);
 PrintfResp(buf);
}
if(Tp==3)
{
if(program==1)
sprintf(buf,"\n pos=%d,\n\r",zone[i].p1v4id);
 PrintfResp(buf);
if(program==2)
sprintf(buf,"\n pos=%d,\n\r",zone[i].p2v4id);
 PrintfResp(buf);
if(program==3)
sprintf(buf,"\n pos=%d,\n\r",zone[i].p3v4id);
 PrintfResp(buf);
if(program==4)
sprintf(buf,"\n pos=%d,\n\r",zone[i].p4v4id);
 PrintfResp(buf);
}
 
}

 writeidzoneset(program,1);
 writeidzoneset(program,2);
 writeidzoneset(program,3);
 writeidzoneset(program,4);
 

 readidzoneset(program,1);
 readidzoneset(program,2);
 readidzoneset(program,3);
 readidzoneset(program,4);
 sprintf(BigSMS1,"");
 send_vzoneidset(PhoneNumber,program,i);
}
}
else if(strstr(smsbuffer,"vidfertset") != 0 ) 
{
readidcomset();
sprintf(BigSMS1,"V06");
send_vidfertset(PhoneNumber); 
}
else if(strstr(smsbuffer,"idfertset") != 0 ) 
{
readidcomset();
 
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, ","  );
}


sprintf(buf,"\n pos=%d\n\r",i);
 PrintfResp(buf);
sprintf(buf,"\n pod=%d\n\r",StrTokStrVer);
 PrintfResp(buf);
for(Tp=0;Tp<=StrTokStrVer;Tp++)
{
 j=(Tp+1);
//sprintf(buf,"\n pod=%s\n\r",StrTokStr1[j]);
 PrintfResp(buf);
Tp1 = StrTokStr1[j][0]-'0';
//sprintf(buf,"\n pod=%d\n\r",Tp1);
 PrintfResp(buf);
Tp2 = StrTokStr1[j][1]-'0';
//sprintf(buf,"\n pod=%d\n\r",Tp2);
 PrintfResp(buf);
Tp3 = StrTokStr1[j][2]-'0';
//sprintf(buf,"\n pod=%d\n\r",Tp3); 
 PrintfResp(buf);

if(Tp==0)
{
zonecom.f1id = myatoi(StrTokStr1[1]);
}
if(Tp==1)
{
zonecom.f2id = myatoi(StrTokStr1[2]);
}
if(Tp==2)
{
zonecom.f3id = myatoi(StrTokStr1[3]);
}
if(Tp==3)
{
zonecom.f4id = myatoi(StrTokStr1[4]);
}


j++;
}
sprintf(buf,"\n pos=%d,\n\r",zonecom.f1id);
 PrintfResp(buf);

sprintf(buf,"\n pos=%d,\n\r",zonecom.f2id);
 PrintfResp(buf);
sprintf(buf,"\n pos=%d,\n\r",zonecom.f3id);
 PrintfResp(buf);

sprintf(buf,"\n pos=%d,\n\r",zonecom.f4id);
 PrintfResp(buf);

 writeidcomset();

readidcomset();
sprintf(BigSMS1,"");
send_vidfertset(PhoneNumber);
}
else if(strstr(smsbuffer,"vidflowset") != 0 ) 
{
readidcomset();
sprintf(BigSMS1,"V08");
send_vidflowset(PhoneNumber); 
}


else if(strstr(smsbuffer,"idflowset") != 0 ) 
{
readidcomset();
 
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, ","  );
}


sprintf(buf,"\n pos=%d\n\r",i);
 PrintfResp(buf);
sprintf(buf,"\n pod=%d\n\r",StrTokStrVer);
 PrintfResp(buf);
for(Tp=0;Tp<=StrTokStrVer;Tp++)
{
 j=(Tp+1);
//sprintf(buf,"\n pod=%s\n\r",StrTokStr1[j]);
 PrintfResp(buf);
Tp1 = StrTokStr1[j][0]-'0';
//sprintf(buf,"\n pod=%d\n\r",Tp1);
 PrintfResp(buf);
Tp2 = StrTokStr1[j][1]-'0';
//sprintf(buf,"\n pod=%d\n\r",Tp2);
 PrintfResp(buf);
Tp3 = StrTokStr1[j][2]-'0';
//sprintf(buf,"\n pod=%d\n\r",Tp3); 
 PrintfResp(buf);

if(Tp==0)
{
zonecom.flowid = myatoi(StrTokStr1[1]);
}
if(Tp==1)
{
zonecom.sumpid = myatoi(StrTokStr1[2]);
}
if(Tp==2)
{
zonecom.tankid = myatoi(StrTokStr1[3]);
}
if(Tp==3)
{
zonecom.extid = myatoi(StrTokStr1[4]);
}


j++;
}
sprintf(buf,"\n pos=%d,\n\r",zonecom.f1id);
 PrintfResp(buf);

sprintf(buf,"\n pos=%d,\n\r",zonecom.f2id);
 PrintfResp(buf);
sprintf(buf,"\n pos=%d,\n\r",zonecom.f3id);
 PrintfResp(buf);
sprintf(buf,"\n pos=%d,\n\r",zonecom.f4id);
 PrintfResp(buf);
 writeidcomset();

readidcomset();
sprintf(BigSMS1,"");
send_vidflowset(PhoneNumber);
}
else if(strstr(smsbuffer,"vidfogset") != 0 ) 
{
readidcomset();
sprintf(BigSMS1,"V05");
send_vidfogset(PhoneNumber); 
}
else if(strstr(smsbuffer,"idfogset") != 0 ) 
{
readidcomset();
 
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, ","  );
}


sprintf(buf,"\n pos=%d\n\r",i);
 PrintfResp(buf);
sprintf(buf,"\n pod=%d\n\r",StrTokStrVer);
 PrintfResp(buf);
for(Tp=0;Tp<=StrTokStrVer;Tp++)
{
 j=(Tp+1);
//sprintf(buf,"\n pod=%s\n\r",StrTokStr1[j]);
 PrintfResp(buf);
Tp1 = StrTokStr1[j][0]-'0';
//sprintf(buf,"\n pod=%d\n\r",Tp1);
 PrintfResp(buf);
Tp2 = StrTokStr1[j][1]-'0';
//sprintf(buf,"\n pod=%d\n\r",Tp2);
 PrintfResp(buf);
Tp3 = StrTokStr1[j][2]-'0';
//sprintf(buf,"\n pod=%d\n\r",Tp3); 
 PrintfResp(buf);

if(Tp==0)
{
zonecom.fog1id = myatoi(StrTokStr1[1]);
}
if(Tp==1)
{
zonecom.fog2id = myatoi(StrTokStr1[2]);
}
if(Tp==2)
{
zonecom.fog3id = myatoi(StrTokStr1[3]);
}
if(Tp==3)
{
zonecom.fog4id = myatoi(StrTokStr1[4]);
}


j++;
}
sprintf(buf,"\n pos=%d,\n\r",zonecom.f1id);
 PrintfResp(buf);
sprintf(buf,"\n pos=%d,\n\r",zonecom.f2id);
 PrintfResp(buf);
sprintf(buf,"\n pos=%d,\n\r",zonecom.f3id);
 PrintfResp(buf);
sprintf(buf,"\n pos=%d,\n\r",zonecom.f4id);
 PrintfResp(buf);
 writeidcomset();

readidcomset();
sprintf(BigSMS1,"");
send_vidfogset(PhoneNumber);
}
else if(strstr(smsbuffer,"vidhumtempset") != 0 ) 
{
readidcomset();
sprintf(BigSMS1,"V07");
send_vidhumtempset(PhoneNumber); 
}
else if(strstr(smsbuffer,"idhumtempset") != 0 ) 
{
readidcomset();
 
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, ","  );
}


sprintf(buf,"\n pos=%d\n\r",i);
 PrintfResp(buf);
sprintf(buf,("\n pod=%d\n\r",StrTokStrVer);
 PrintfResp(buf);
for(Tp=0;Tp<=StrTokStrVer;Tp++)
{
 j=(Tp+1);
//sprintf(buf,"\n pod=%s\n\r",StrTokStr1[j]);
 PrintfResp(buf);
Tp1 = StrTokStr1[j][0]-'0';
//sprintf(buf,"\n pod=%d\n\r",Tp1);
 PrintfResp(buf);
Tp2 = StrTokStr1[j][1]-'0';
//sprintf(buf,"\n pod=%d\n\r",Tp2);
 PrintfResp(buf);
Tp3 = StrTokStr1[j][2]-'0';
//sprintf(buf,"\n pod=%d\n\r",Tp3);
 PrintfResp(buf); 

if(Tp==0)
{
zonecom.humtemp1id = myatoi(StrTokStr1[1]);
}
if(Tp==1)
{
zonecom.humtemp2id = myatoi(StrTokStr1[2]);
}
if(Tp==2)
{
zonecom.humtemp3id = myatoi(StrTokStr1[3]);
}
if(Tp==3)
{
zonecom.humtemp4id = myatoi(StrTokStr1[4]);
}


j++;
}
sprintf(buf,"\n pos=%d,\n\r",zonecom.f1id);
 PrintfResp(buf);
sprintf(buf,"\n pos=%d,\n\r",zonecom.f2id);
 PrintfResp(buf);
sprintf(buf,"\n pos=%d,\n\r",zonecom.f3id);
 PrintfResp(buf);
sprintf(buf,"\n pos=%d,\n\r",zonecom.f4id);
 PrintfResp(buf);
 writeidcomset();

readidcomset();
sprintf(BigSMS1,"");
send_vidhumtempset(PhoneNumber);
}
else if(strstr(smsbuffer,"videmsset") != 0 ) 
{
readidcomset();
send_videmsset(PhoneNumber);
}

else if(strstr(smsbuffer,"idemsset") != 0 ) 
{
readidcomset();
 
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, ","  );
}


sprintf(buf,"\n pos=%d\n\r",i);
 PrintfResp(buf);
sprintf(buf,"\n pod=%d\n\r",StrTokStrVer);
 PrintfResp(buf);
for(Tp=0;Tp<=StrTokStrVer;Tp++)
{
 j=(Tp+1);
//sprintf(buf,"\n pod=%s\n\r",StrTokStr1[j]);
 PrintfResp(buf);
Tp1 = StrTokStr1[j][0]-'0';
//sprintf(buf,"\n pod=%d\n\r",Tp1);
 PrintfResp(buf);
Tp2 = StrTokStr1[j][1]-'0';
//sprintf(buf,"\n pod=%d\n\r",Tp2);
 PrintfResp(buf);
Tp3 = StrTokStr1[j][2]-'0';
//sprintf(buf,"\n pod=%d\n\r",Tp3); 
 PrintfResp(buf);

if(Tp==0)
{
zonecom.EMS1id = myatoi(StrTokStr1[1]);
}
if(Tp==1)
{
zonecom.EMS2id = myatoi(StrTokStr1[2]);
}
if(Tp==2)
{
zonecom.EMS3id = myatoi(StrTokStr1[3]);
}
if(Tp==3)
{
zonecom.EMS4id = myatoi(StrTokStr1[4]);
}


//j++;
}
sprintf(buf,"\n pos=%d,\n\r",zonecom.f1id);
 PrintfResp(buf);
sprintf(buf,"\n pos=%d,\n\r",zonecom.f2id);
 PrintfResp(buf);
sprintf(buf,"\n pos=%d,\n\r",zonecom.f3id);
 PrintfResp(buf);
sprintf(buf,"\n pos=%d,\n\r",zonecom.f4id);
 PrintfResp(buf);
 writeidcomset();

readidcomset();
send_videmsset(PhoneNumber);
}

else if(strstr(smsbuffer,"vidlightfanset") != 0 ) 
{
readidcomset();
sprintf(BigSMS1,"V04");
send_vidlightfanset(PhoneNumber); 
}
else if(strstr(smsbuffer,"idlightfanset") != 0 ) 
{
readidcomset();
 
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, ","  );
}


sprintf(buf,"\n pos=%d\n\r",i);
 PrintfResp(buf);
sprintf(buf,"\n pod=%d\n\r",StrTokStrVer);
 PrintfResp(buf);
for(Tp=0;Tp<=StrTokStrVer;Tp++)
{
j=(Tp+1);
//sprintf(buf,"\n pod=%s\n\r",StrTokStr1[j]);
 PrintfResp(buf);
Tp1 = StrTokStr1[j][0]-'0';
//sprintf(buf,"\n pod=%d\n\r",Tp1);
 PrintfResp(buf);
Tp2 = StrTokStr1[j][1]-'0';
//sprintf(buf,"\n pod=%d\n\r",Tp2);
 PrintfResp(buf);
Tp3 = StrTokStr1[j][2]-'0';
//sprintf(buf,"\n pod=%d\n\r",Tp3); 
 PrintfResp(buf);

if(Tp==0)
{
zonecom.light1id = myatoi(StrTokStr1[1]);
}
if(Tp==1)
{
zonecom.light2id = myatoi(StrTokStr1[2]);
}
if(Tp==2)
{
zonecom.fan1id = myatoi(StrTokStr1[3]);
}
if(Tp==3)
{
zonecom.fan2id = myatoi(StrTokStr1[4]);
}


j++;
}
sprintf(buf,"\n pos=%d,\n\r",zonecom.f1id);
 PrintfResp(buf);
sprintf(buf,"\n pos=%d,\n\r",zonecom.f2id);
 PrintfResp(buf);
sprintf(buf,"\n pos=%d,\n\r",zonecom.f3id);
 PrintfResp(buf);
sprintf(buf,"\n pos=%d,\n\r",zonecom.f4id);
 PrintfResp(buf);
 writeidcomset();

readidcomset();
sprintf(BigSMS1,"");
send_vidlightfanset(PhoneNumber);
}


else if(strstr(smsbuffer,"vflowsetp") != 0 ) 
{

Tp1 = smsbuffer[9]-'0';
program= Tp1;
Tp1 = smsbuffer[10]-'0';
Tp3 = smsbuffer[11]-'0';
step = (Tp1*10)+Tp3;
if(program>=1&&program<=4&&step>=1&&step<=8)
{

readzoneflowset(program);
sprintf(BigSMS1,"V33");
send_vflowset(PhoneNumber,program,step);
}

}
else if(strstr(smsbuffer,"flowsetp") != 0 ) 
{
Tp1 = smsbuffer[8]-'0';
program= Tp1;
Tp1 = smsbuffer[9]-'0';
Tp3 = smsbuffer[10]-'0';
step = (Tp1*10)+Tp3;
if(program>=1&&program<=4&&step>=1&&step<=8)
{

readzoneflowset(program);
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, "," );
}
if(step==1){tm_min=1;max=8;j=1;}
if(step==2){tm_min=9;max=16;j=1;}

if(step==3){tm_min=17;max=24;j=1;}
if(step==4){tm_min=25;max=32;j=1;}

if(step==5){tm_min=33;max=40;j=1;}
if(step==6){tm_min=41;max=48;j=1;}

if(step==7){tm_min=49;max=56;j=1;}
if(step==8){tm_min=57;max=64;j=1;}

										
for(Tp=tm_min;Tp<=max;Tp++)
{
if(program==1)
zone[Tp].p1flowrate = myatoi(StrTokStr1[j]);
if(program==2)
zone[Tp].p2flowrate = myatoi(StrTokStr1[j]);
if(program==3)
zone[Tp].p3flowrate = myatoi(StrTokStr1[j]);
if(program==4)
zone[Tp].p4flowrate = myatoi(StrTokStr1[j]);

j++;

}
writezoneflowset(program);
readzoneflowset(program);
sprintf(BigSMS1,"");
send_vflowset(PhoneNumber,program,step);
}
}
else if(strstr(smsbuffer,"vflowfertsetp") != 0 ) 
{
Tp1 = smsbuffer[13]-'0';
program= Tp1;
Tp1 = smsbuffer[15]-'0';
fert= Tp1;

Tp1 = smsbuffer[16]-'0';
Tp3 = smsbuffer[17]-'0';
step = (Tp1*10)+Tp3;
if(program>=1&&program<=4&&step>=1&&step<=8&&fert>=1&&fert<=4)
{

readfertflowset(program,fert);
sprintf(BigSMS1,"V37");
send_vfertflowset(PhoneNumber,program ,fert,step);
}
}

else if(strstr(smsbuffer,"flowfertsetp") != 0 ) 
{
Tp1 = smsbuffer[12]-'0';
program= Tp1;
Tp1 = smsbuffer[14]-'0';
fert= Tp1;

Tp1 = smsbuffer[15]-'0';
Tp3 = smsbuffer[16]-'0';
step = (Tp1*10)+Tp3;
if(program>=1&&program<=4&&step>=1&&step<=8&&fert>=1&&fert<=4)
{

readfertflowset(program,fert);
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, "," );
}

if(step==1){tm_min=1;max=8;j=1;}
if(step==2){tm_min=9;max=16;j=1;}

if(step==3){tm_min=17;max=24;j=1;}
if(step==4){tm_min=25;max=32;j=1;}

if(step==5){tm_min=33;max=40;j=1;}
if(step==6){tm_min=41;max=48;j=1;}

if(step==7){tm_min=49;max=56;j=1;}
if(step==8){tm_min=57;max=64;j=1;}


for(Tp=tm_min;Tp<=max;Tp++)
{
if(fert==1)
{
if(program==1)
zone[Tp].p1f1flow = myatoi(StrTokStr1[j]);
if(program==2)
zone[Tp].p2f1flow = myatoi(StrTokStr1[j]);
if(program==3)
zone[Tp].p3f1flow = myatoi(StrTokStr1[j]);
if(program==4)
zone[Tp].p4f1flow = myatoi(StrTokStr1[j]);
}
if(fert==2)
{
if(program==1)
zone[Tp].p1f2flow = myatoi(StrTokStr1[j]);
if(program==2)
zone[Tp].p2f2flow = myatoi(StrTokStr1[j]);
if(program==3)
zone[Tp].p3f2flow = myatoi(StrTokStr1[j]);
if(program==4)
zone[Tp].p4f2flow = myatoi(StrTokStr1[j]);
}
if(fert==3)
{
if(program==1)
zone[Tp].p1f3flow = myatoi(StrTokStr1[j]);
if(program==2)
zone[Tp].p2f3flow = myatoi(StrTokStr1[j]);
if(program==3)
zone[Tp].p3f3flow = myatoi(StrTokStr1[j]);
if(program==4)
zone[Tp].p4f3flow = myatoi(StrTokStr1[j]);
}
if(fert==4)
{
if(program==1)
zone[Tp].p1f4flow = myatoi(StrTokStr1[j]);
if(program==2)
zone[Tp].p2f4flow = myatoi(StrTokStr1[j]);
if(program==3)
zone[Tp].p3f4flow = myatoi(StrTokStr1[j]);
if(program==4)
zone[Tp].p4f4flow = myatoi(StrTokStr1[j]);
}
j++;
}
writefertflowset(program,fert);
readfertflowset(program,fert);
sprintf(BigSMS1,"");
send_vfertflowset(PhoneNumber,program ,fert,step);

}

}

else if(strstr(smsbuffer,"vferttimersetp") != 0 ) 
{
Tp1 = smsbuffer[14]-'0';
program= Tp1;
Tp1 = smsbuffer[16]-'0';
fert= Tp1;

Tp1 = smsbuffer[17]-'0';
Tp3 = smsbuffer[18]-'0';
step = (Tp1*10)+Tp3;
if(program>=1&&program<=4&&step>=1&&step<=8&&fert>=1&&fert<=4)
{
readferttimerset(program,fert);
sprintf(BigSMS1,"V36");
send_vferttimerset(PhoneNumber,program ,fert,step);
}
}

else if(strstr(smsbuffer,"ferttimersetp") != 0 ) 
{
Tp1 = smsbuffer[13]-'0';
program= Tp1;
Tp1 = smsbuffer[15]-'0';
fert= Tp1;

Tp1 = smsbuffer[16]-'0';
Tp3 = smsbuffer[17]-'0';
step = (Tp1*10)+Tp3;
if(program>=1&&program<=4&&step>=1&&step<=8&&fert>=1&&fert<=4)
{

readferttimerset(program,fert);
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, "," );
}

if(step==1){tm_min=1;max=8;j=1;}
if(step==2){tm_min=9;max=16;j=1;}

if(step==3){tm_min=17;max=24;j=1;}
if(step==4){tm_min=25;max=32;j=1;}

if(step==5){tm_min=33;max=40;j=1;}
if(step==6){tm_min=41;max=48;j=1;}

if(step==7){tm_min=49;max=56;j=1;}
if(step==8){tm_min=57;max=64;j=1;}


for(Tp=tm_min;Tp<=max;Tp++)
{
Tp1 = StrTokStr1[j][0]-'0';
Tp3 = StrTokStr1[j][1]-'0';
if(fert==1)
{
if(program==1)
zone[Tp].p1f1min = (Tp1*10)+Tp3;
if(program==2)
zone[Tp].p2f1min = (Tp1*10)+Tp3;
if(program==3)
zone[Tp].p3f1min = (Tp1*10)+Tp3;
if(program==4)
zone[Tp].p4f1min = (Tp1*10)+Tp3;
}
if(fert==2)
{
if(program==1)
zone[Tp].p1f2min = (Tp1*10)+Tp3;
if(program==2)
zone[Tp].p2f2min = (Tp1*10)+Tp3;
if(program==3)
zone[Tp].p3f2min = (Tp1*10)+Tp3;
if(program==4)
zone[Tp].p4f2min = (Tp1*10)+Tp3;
}
if(fert==3)
{
if(program==1)
zone[Tp].p1f3min = (Tp1*10)+Tp3;
if(program==2)
zone[Tp].p2f3min = (Tp1*10)+Tp3;
if(program==3)
zone[Tp].p3f3min = (Tp1*10)+Tp3;
if(program==4)
zone[Tp].p4f3min = (Tp1*10)+Tp3;
}
if(fert==4)
{
if(program==1)
zone[Tp].p1f4min = (Tp1*10)+Tp3;
if(program==2)
zone[Tp].p2f4min = (Tp1*10)+Tp3;
if(program==3)
zone[Tp].p3f4min = (Tp1*10)+Tp3;
if(program==4)
zone[Tp].p4f4min = (Tp1*10)+Tp3;
}
Tp1 = StrTokStr1[j][2]-'0';
Tp3 = StrTokStr1[j][3]-'0';

if(fert==1)
{
if(program==1)
zone[Tp].p1f1sec = (Tp1*10)+Tp3;
if(program==2)
zone[Tp].p2f1sec = (Tp1*10)+Tp3;
if(program==3)
zone[Tp].p3f1sec = (Tp1*10)+Tp3;
if(program==4)
zone[Tp].p4f1sec = (Tp1*10)+Tp3;
}
if(fert==2)
{
if(program==1)
zone[Tp].p1f2sec = (Tp1*10)+Tp3;
if(program==2)
zone[Tp].p2f2sec = (Tp1*10)+Tp3;
if(program==3)
zone[Tp].p3f2sec = (Tp1*10)+Tp3;
if(program==4)
zone[Tp].p4f2sec = (Tp1*10)+Tp3;
}
if(fert==3)
{
if(program==1)
zone[Tp].p1f3sec = (Tp1*10)+Tp3;
if(program==2)
zone[Tp].p2f3sec = (Tp1*10)+Tp3;
if(program==3)
zone[Tp].p3f3sec = (Tp1*10)+Tp3;
if(program==4)
zone[Tp].p4f3sec = (Tp1*10)+Tp3;
}
if(fert==4)
{
if(program==1)
zone[Tp].p1f4sec = (Tp1*10)+Tp3;
if(program==2)
zone[Tp].p2f4sec = (Tp1*10)+Tp3;
if(program==3)
zone[Tp].p3f4sec = (Tp1*10)+Tp3;
if(program==4)
zone[Tp].p4f4sec = (Tp1*10)+Tp3;
}
j++;

}
writeferttimerset(program,fert);
readferttimerset(program,fert);
sprintf(BigSMS1,"");
send_vferttimerset(PhoneNumber,program ,fert,step);
}
}

else if(strstr(smsbuffer,"vzonetimersetp") != 0 ) 
{
Tp1 = smsbuffer[14]-'0';
program= Tp1;
Tp1 = smsbuffer[15]-'0';
Tp3 = smsbuffer[16]-'0';
step = (Tp1*10)+Tp3;
if(program>=1&&program<=4&&step>=1&&step<=8)
{
readzonetimerset(program);
sprintf(BigSMS1,"V32");
send_vzonetimerset(PhoneNumber,program,step);
}
}

else if(strstr(smsbuffer,"zonetimersetp") != 0 ) 
{
Tp1 = smsbuffer[13]-'0';
program= Tp1;
Tp1 = smsbuffer[14]-'0';
Tp3 = smsbuffer[15]-'0';
step = (Tp1*10)+Tp3;
if(program>=1&&program<=4&&step>=1&&step<=8)
{

readzonetimerset(program);
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, "," );
}


if(step==1){tm_min=1;max=8;j=1;}
if(step==2){tm_min=9;max=16;j=1;}

if(step==3){tm_min=17;max=24;j=1;}
if(step==4){tm_min=25;max=32;j=1;}

if(step==5){tm_min=33;max=40;j=1;}
if(step==6){tm_min=41;max=48;j=1;}

if(step==7){tm_min=49;max=56;j=1;}
if(step==8){tm_min=57;max=64;j=1;}


for(Tp=tm_min;Tp<=max;Tp++)
{
Tp1 = StrTokStr1[j][0]-'0';
Tp3 = StrTokStr1[j][1]-'0';
if(program==1)
zone[Tp].p1thr = (Tp1*10)+Tp3;
if(program==2)
zone[Tp].p2thr = (Tp1*10)+Tp3;
if(program==3)
zone[Tp].p3thr = (Tp1*10)+Tp3;
if(program==4)
zone[Tp].p4thr = (Tp1*10)+Tp3;

sprintf(buf,"\n pos=%d\n\r",zone[Tp].p1thr);
 PrintfResp(buf);
Tp1 = StrTokStr1[j][2]-'0';
Tp3 = StrTokStr1[j][3]-'0';
if(program==1)
zone[Tp].p1tmin = (Tp1*10)+Tp3;
if(program==2)
zone[Tp].p2tmin = (Tp1*10)+Tp3;
if(program==3)
zone[Tp].p3tmin = (Tp1*10)+Tp3;
if(program==4)
zone[Tp].p4tmin = (Tp1*10)+Tp3;

sprintf(buf,"\n pos=%d\n\r",zone[Tp].p1tmin);
 PrintfResp(buf);
j++;

}

for(Tp=0;Tp<=StrTokStrVer;Tp++)
{
sprintf(buf,"\n\rSt=%s  pos=%d\n\r",StrTokStr1[Tp],Tp);
 PrintfResp(buf);

}
writezonetimerset(program);
readzonetimerset(program);
sprintf(BigSMS1,"");
send_vzonetimerset(PhoneNumber,program,step);
}
}

else if(strstr(smsbuffer,"vidzlmsetp") != 0 ) 
{


Tp1 = smsbuffer[10]-'0';
program= Tp1;
sprintf(buf,"\n pos=%d\n\r",program);
 PrintfResp(buf);
Tp1 = smsbuffer[11]-'0';
Tp2 = smsbuffer[12]-'0';
Tp3 = smsbuffer[13]-'0';
i = (Tp1*100)+(Tp2*10)+Tp3;
if(program>=1&&program<=4&&i>=1&&i<=64)
{
readidlmset(program);
sprintf(BigSMS1,"V03");
send_vidzlmset(PhoneNumber,program,i);
}
}

else if(strstr(smsbuffer,"idzlmsetp") != 0 ) 
{


Tp1 = smsbuffer[9]-'0';
program= Tp1;
sprintf(buf,"\n pos=%d\n\r",program);
 PrintfResp(buf);
Tp1 = smsbuffer[10]-'0';
Tp2 = smsbuffer[11]-'0';
Tp3 = smsbuffer[12]-'0';
i = (Tp1*100)+(Tp2*10)+Tp3;
if(program>=1&&program<=4&&i>=1&&i<=64)
{
readidlmset(program);
 
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, ","  );
}


sprintf(buf,"\n pos=%d\n\r",i);
 PrintfResp(buf);
sprintf(buf,"\n pod=%d\n\r",StrTokStrVer);
 PrintfResp(buf);
for(Tp=0;Tp<=4;Tp++)
{
  j=(Tp+1);
//sprintf(buf,"\n pod=%s\n\r",StrTokStr1[j]);
 PrintfResp(buf);
Tp1 = StrTokStr1[j][0]-'0';
//sprintf(buf,"\n pod=%d\n\r",Tp1);
 PrintfResp(buf);
Tp2 = StrTokStr1[j][1]-'0';
//sprintf(buf,"\n pod=%d\n\r",Tp2);
 PrintfResp(buf);
Tp3 = StrTokStr1[j][2]-'0';
//sprintf(buf,"\n pod=%d\n\r",Tp3); 
 PrintfResp(buf); 

if(Tp==0)
{
if(program==1)
zone[i].p1l1id = myatoi(StrTokStr1[1]);
if(program==2)
zone[i].p2l1id = myatoi(StrTokStr1[1]);
if(program==3)
zone[i].p3l1id = myatoi(StrTokStr1[1]);
if(program==4)
zone[i].p4l1id = myatoi(StrTokStr1[1]);

}
if(Tp==1)
{
if(program==1)
zone[i].p1l2id = myatoi(StrTokStr1[2]);
if(program==2)
zone[i].p2l2id = myatoi(StrTokStr1[2]);
if(program==3)
zone[i].p3l2id = myatoi(StrTokStr1[2]);
if(program==4)
zone[i].p4l2id = myatoi(StrTokStr1[2]);

}
if(Tp==2)
{
if(program==1)
zone[i].p1m1id = myatoi(StrTokStr1[3]);
if(program==2)
zone[i].p2m1id = myatoi(StrTokStr1[3]);
if(program==3)
zone[i].p3m1id = myatoi(StrTokStr1[3]);
if(program==4)
zone[i].p4m1id = myatoi(StrTokStr1[3]);
}
if(Tp==3)
{
if(program==1)
zone[i].p1m2id = myatoi(StrTokStr1[4]);
if(program==2)
zone[i].p2m2id = myatoi(StrTokStr1[4]);
if(program==3)
zone[i].p3m2id = myatoi(StrTokStr1[4]);
if(program==4)
zone[i].p4m2id = myatoi(StrTokStr1[4]);
}


 
if(Tp==0)
{
if(program==1)
sprintf(buf,"\n pos=%d,\n\r",zone[i].p1v1id);
 PrintfResp(buf);
if(program==2)
sprintf(buf,"\n pos=%d,\n\r",zone[i].p2v1id);
 PrintfResp(buf);
if(program==3)
sprintf(buf,"\n pos=%d,\n\r",zone[i].p3v1id);
 PrintfResp(buf);
if(program==4)
sprintf(buf,"\n pos=%d,\n\r",zone[i].p4v1id);
 PrintfResp(buf);
}
if(Tp==1)
{
if(program==1)
sprintf(buf,"\n pos=%d,\n\r",zone[i].p1v2id);
 PrintfResp(buf);
if(program==2)
sprintf(buf,"\n pos=%d,\n\r",zone[i].p2v2id);
 PrintfResp(buf);
if(program==3)
sprintf(buf,"\n pos=%d,\n\r",zone[i].p3v2id);
 PrintfResp(buf);
if(program==4)
sprintf(buf,"\n pos=%d,\n\r",zone[i].p4v2id);
 PrintfResp(buf);
}
if(Tp==2)
{
if(program==1)
sprintf(buf,"\n pos=%d,\n\r",zone[i].p1v3id);
 PrintfResp(buf);
if(program==2)
sprintf(buf,"\n pos=%d,\n\r",zone[i].p2v3id);
 PrintfResp(buf);
if(program==3)
sprintf(buf,"\n pos=%d,\n\r",zone[i].p3v3id);
 PrintfResp(buf);
if(program==4)
sprintf(buf,"\n pos=%d,\n\r",zone[i].p4v3id);
 PrintfResp(buf);
}
if(Tp==3)
{
if(program==1)
sprintf(buf,"\n pos=%d,\n\r",zone[i].p1v4id);
 PrintfResp(buf);
if(program==2)
sprintf(buf,"\n pos=%d,\n\r",zone[i].p2v4id);
 PrintfResp(buf);
if(program==3)
sprintf(buf,"\n pos=%d,\n\r",zone[i].p3v4id);
 PrintfResp(buf);
if(program==4)
sprintf(buf,"\n pos=%d,\n\r",zone[i].p4v4id);
 PrintfResp(buf);
}
  
}
writeidlmset(program);
readidlmset(program);
sprintf(BigSMS1,"");
send_vidzlmset(PhoneNumber,program,i);
}
}
else if(strstr(smsbuffer,"vadjpercentp") != 0 ) 
{
Tp1 = smsbuffer[12]-'0';
program= Tp1;
sprintf(buf,"\n pos=%d\n\r",program);
if(program>=1&&program<=4)
{
readmodeonofset();
sprintf(BigSMS1,"V41");
send_vadjpercentset(PhoneNumber,program); 
}
}
else if(strstr(smsbuffer,"adjpercentp") != 0 ) 
{
Tp1 = smsbuffer[11]-'0';
program= Tp1;
sprintf(buf,"\n pos=%d\n\r",program);
 PrintfResp(buf);
if(program>=1&&program<=4)
{
readmodeonofset();
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, ","  );
}


sprintf(buf,"\n pos=%d\n\r",i);
 PrintfResp(buf);
sprintf(buf,"\n pod=%d\n\r",StrTokStrVer);
 PrintfResp(buf);

zoneonof[program].timerpercent=myatoi(StrTokStr1[1]);
zoneonof[program].flowpercent=myatoi(StrTokStr1[2]);
zoneonof[program].moisturepercent=myatoi(StrTokStr1[3]);
zoneonof[program].fertpercent=myatoi(StrTokStr1[4]);
zoneonof[program].fertdelaypercent=myatoi(StrTokStr1[5]);

writemodeonofset();
readmodeonofset();
sprintf(BigSMS1,"");
send_vadjpercentset(PhoneNumber,program); 
}
}
else if(strstr(smsbuffer,"vphlevlset") != 0 ) 
{

readidcomset();
sprintf(BigSMS1,"V39");
send_vphvalueset(PhoneNumber); 
}
else if(strstr(smsbuffer,"phlevlset") != 0 ) 
{
 //Tp1 = smsbuffer[11]-'0';
 //program= Tp1;
sprintf(buf,"\n pos=%d\n\r",program);
 PrintfResp(buf);
//if(program>=1&&program<=4)
//{
readidcomset();
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, ","  );
}


sprintf(buf,"\n pos=%d\n\r",i);
 PrintfResp(buf);
sprintf(buf,"\n pod=%d\n\r",StrTokStrVer);
 PrintfResp(buf);
zoneonof[1].phlevel=myatoi(StrTokStr1[1]);
zoneonof[2].phlevel=myatoi(StrTokStr1[2]);
zoneonof[3].phlevel=myatoi(StrTokStr1[3]);
zoneonof[4].phlevel=myatoi(StrTokStr1[4]);

writeidcomset();
readidcomset();
sprintf(BigSMS1,"");
send_vphvalueset(PhoneNumber); 
}

else if(strstr(smsbuffer,"vmoisturelevelset") != 0 ) 
{
readidcomset();
sprintf(BigSMS1,"V34");
send_vmoisturelevelset(PhoneNumber); 
}

else if(strstr(smsbuffer,"moisturelevelsetp") != 0 ) 
{
readidcomset();
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, ","  );
}

i=smsbuffer[17]-'0';
sprintf(buf,"\n pos=%d\n\r",i);
 PrintfResp(buf);
sprintf(buf,"\n pod=%d\n\r",StrTokStrVer);
 PrintfResp(buf);

//zoneonof[program].timerpercent=myatoi(StrTokStr1[1]);
zoneonof[i].moistureonlevel=myatoi(StrTokStr1[1]);
zoneonof[i].moistureoflevel=myatoi(StrTokStr1[2]);

writeidcomset();
readidcomset();
sprintf(BigSMS1,"");
send_vmoisturelevelset(PhoneNumber); 
}
else if(strstr(smsbuffer,"vaquaonlevelP") != 0 ) 
{
readidcomset();
sprintf(BigSMS1,"V35");
send_vaqualevelset(PhoneNumber); 
}
else if(strstr(smsbuffer,"aquaonlevelp") != 0 ) 
{
readidcomset();
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, ","  );
}

i=smsbuffer[12]-'0';
sprintf(buf,"\n pos=%d\n\r",i);
 PrintfResp(buf);
sprintf(buf,"\n pod=%d\n\r",StrTokStrVer);
 PrintfResp(buf);

//zoneonof[program].timerpercent=myatoi(StrTokStr1[1]);
zoneonof[i].aquaonlevel=myatoi(StrTokStr1[1]);
zoneonof[i].aquaoflevel=myatoi(StrTokStr1[2]);

writeidcomset();
readidcomset();
sprintf(BigSMS1,"");
send_vaqualevelset(PhoneNumber); 
}

else if(strstr(smsbuffer,"vdelvalfertset") != 0 ) 
{
readidcomset();
sprintf(BigSMS1,"V40");
send_vdelvalfertset(PhoneNumber); 
}
else if(strstr(smsbuffer,"delfbktim") != 0 ) 
{
readidcomset();
Tp1= smsbuffer[10]-'0';
Tp2= smsbuffer[11]-'0';
zonecom.fbkdelmin=(Tp1*10)+Tp2;
Tp1= smsbuffer[12]-'0';
Tp2= smsbuffer[13]-'0';
zonecom.fbkdelsec=(Tp1*10)+Tp2;

writeidcomset();
readidcomset();
sprintf(BigSMS1,"");
send_vdelvalfertset(PhoneNumber); 
}
else if(strstr(smsbuffer,"delmixertim") != 0 ) 
{
readidcomset();
Tp1= smsbuffer[12]-'0';
Tp2= smsbuffer[13]-'0';
zonecom.mixdelaymin=(Tp1*10)+Tp2;
Tp1= smsbuffer[14]-'0';
Tp2= smsbuffer[15]-'0';
zonecom.mixdelaysec=(Tp1*10)+Tp2;

writeidcomset();
readidcomset();
sprintf(BigSMS1,"");
send_vdelvalfertset(PhoneNumber); 
}
else if(strstr(smsbuffer,"daycountrtctim") != 0 ) 
{
readidcomset();
Tp1= smsbuffer[15]-'0';
Tp2= smsbuffer[16]-'0';
zonecom.daycountmin=(Tp1*10)+Tp2;
Tp1= smsbuffer[17]-'0';
Tp2= smsbuffer[18]-'0';
zonecom.daycountsec=(Tp1*10)+Tp2;

writeidcomset();
readidcomset();
sprintf(BigSMS1,"");
send_vdelvalfertset(PhoneNumber); 
}

else if(strstr(smsbuffer,"delvaltim") != 0 ) 
{
readidcomset();
Tp1= smsbuffer[10]-'0';
Tp2= smsbuffer[11]-'0';
zonecom.valdelaymin=(Tp1*10)+Tp2;
Tp1= smsbuffer[12]-'0';
Tp2= smsbuffer[13]-'0';
zonecom.valdelaysec=(Tp1*10)+Tp2;

writeidcomset();
readidcomset();
sprintf(BigSMS1,"");
send_vdelvalfertset(PhoneNumber); 
}
else if(strstr(smsbuffer,"delferttim") != 0 ) 
{
readidcomset();
Tp1= smsbuffer[11]-'0';
Tp2= smsbuffer[12]-'0';
zonecom.fertdelaymin=(Tp1*10)+Tp2;
Tp1= smsbuffer[13]-'0';
Tp2= smsbuffer[14]-'0';
zonecom.fertdelaysec=(Tp1*10)+Tp2;

writeidcomset();
readidcomset();
sprintf(BigSMS1,"");
send_vdelvalfertset(PhoneNumber); 
}

  else if(strstr(smsbuffer,"fertdelaypercent") != 0 ) 
{
readidcomset();
Tp1= smsbuffer[17]-'0';
Tp2= smsbuffer[18]-'0';
Tp3= smsbuffer[19]-'0';
//zonecom.fertdelaypercent=(Tp1*100)+(Tp2*10)+Tp3;
writeidcomset();
readidcomset();
sprintf(BigSMS1,"");
send_fertpercentset(PhoneNumber); 
}  

else if(strstr(smsbuffer,"vmodeonofp") != 0 ) 
{
Tp1 = smsbuffer[10]-'0';
program= Tp1;
sprintf(buf,"\n pos=%d\n\r",program);
 PrintfResp(buf);
if(program>=1&&program<=4)
{
 //readidcomset();
readmodeonofset();
sprintf(BigSMS1,"V42");
send_vmodeonofset(PhoneNumber,program); 
}
}
else if(strstr(smsbuffer,"flowonp") != 0 ) 
{
Tp1 = smsbuffer[7]-'0';
program= Tp1;
sprintf(buf,"\n pos=%d\n\r",program);
 PrintfResp(buf);
if(program>=1&&program<=4)
{
readmodeonofset();
zoneonof[program].flowonof=1;
writemodeonofset();
readmodeonofset();
// readidcomset();
sprintf(BigSMS1,"");
send_vmodeonofset(PhoneNumber,program); 
}
}
else if(strstr(smsbuffer,"flowofp") != 0 ) 
{
Tp1 = smsbuffer[7]-'0';
program= Tp1;
sprintf(buf,"\n pos=%d\n\r",program);
 PrintfResp(buf);
if(program>=1&&program<=4)
{
readmodeonofset();
zoneonof[program].flowonof=0;
writemodeonofset();
readmodeonofset();
//readidcomset();
sprintf(BigSMS1,"");
send_vmodeonofset(PhoneNumber,program); 
}
}
else if(strstr(smsbuffer,"flowfertonp") != 0 ) 
{
Tp1 = smsbuffer[11]-'0';
program= Tp1;
sprintf(buf,"\n pos=%d\n\r",program);
 PrintfResp(buf);
if(program>=1&&program<=4)
{
readmodeonofset();
zoneonof[program].flowfertonof=1;
writemodeonofset();
readmodeonofset();
//readidcomset();
sprintf(BigSMS1,"");
send_vmodeonofset(PhoneNumber,program); 
}
}
else if(strstr(smsbuffer,"flowfertofp") != 0 ) 
{
Tp1 = smsbuffer[11]-'0';
program= Tp1;
sprintf(buf,"\n pos=%d\n\r",program);
 PrintfResp(buf);
if(program>=1&&program<=4)
{
readmodeonofset();
zoneonof[program].flowfertonof=0;
writemodeonofset();
readmodeonofset();
//readidcomset();
sprintf(BigSMS1,"");
send_vmodeonofset(PhoneNumber,program); 
}
}
else if(strstr(smsbuffer,"humidityonp") != 0 ) 
{
Tp1 = smsbuffer[11]-'0';
program= Tp1;
sprintf(buf,"\n pos=%d\n\r",program);
 PrintfResp(buf);
if(program>=1&&program<=4)
{
readmodeonofset();
zoneonof[program].humidityonof=1;
writemodeonofset();
readmodeonofset();
//readidcomset();
sprintf(BigSMS1,"");
send_vmodeonofset(PhoneNumber,program);
} 
}
else if(strstr(smsbuffer,"humidityofp") != 0 ) 
{
Tp1 = smsbuffer[11]-'0';
program= Tp1;
sprintf(buf,"\n pos=%d\n\r",program);
 PrintfResp(buf);
if(program>=1&&program<=4)
{
readmodeonofset();
zoneonof[program].humidityonof=0;
writemodeonofset();
readmodeonofset();
//readidcomset();
sprintf(BigSMS1,"");
send_vmodeonofset(PhoneNumber,program); 
}
}
else if(strstr(smsbuffer,"temponp") != 0 ) 
{
Tp1 = smsbuffer[7]-'0';
program= Tp1;
sprintf(buf,"\n pos=%d\n\r",program);
 PrintfResp(buf);
if(program>=1&&program<=4)
{
readmodeonofset();
zoneonof[program].temponof=1;
writemodeonofset();
readmodeonofset();
//readidcomset();
sprintf(BigSMS1,"");
send_vmodeonofset(PhoneNumber,program); 
}
}
else if(strstr(smsbuffer,"tempofp") != 0 ) 
{
Tp1 = smsbuffer[7]-'0';
program= Tp1;
sprintf(buf,"\n pos=%d\n\r",program);
 PrintfResp(buf);
if(program>=1&&program<=4)
{
readmodeonofset();
zoneonof[program].temponof=0;
writemodeonofset();
readmodeonofset();
//readidcomset();
sprintf(BigSMS1,"");
send_vmodeonofset(PhoneNumber,program); 
}
}
else if(strstr(smsbuffer,"moistureonp") != 0 ) 
{
Tp1 = smsbuffer[11]-'0';
program= Tp1;
sprintf(buf,"\n pos=%d\n\r",program);
 PrintfResp(buf);
if(program>=1&&program<=4)
{
readmodeonofset();
zoneonof[program].moistureonof=1;
writemodeonofset();
readmodeonofset();
//readidcomset();
sprintf(BigSMS1,"");
send_vmodeonofset(PhoneNumber,program); 
}
}
else if(strstr(smsbuffer,"moistureofp") != 0 ) 
{
Tp1 = smsbuffer[11]-'0';
program= Tp1;
sprintf(buf,"\n pos=%d\n\r",program);
 PrintfResp(buf);
if(program>=1&&program<=4)
{
readmodeonofset();
zoneonof[program].moistureonof=0;
writemodeonofset();
readmodeonofset();
//readidcomset();
sprintf(BigSMS1,"");
send_vmodeonofset(PhoneNumber,program); 
}
}
else if(strstr(smsbuffer,"timermodeonp") != 0 ) 
{
Tp1 = smsbuffer[12]-'0';
program= Tp1;
sprintf(buf,"\n pos=%d\n\r",program);
 PrintfResp(buf);
if(program>=1&&program<=4)
{
readmodeonofset();
zoneonof[program].timermodeonof=1;
zoneonof[program].flowmodeonof=0;
zoneonof[program].moisturemodeonof=0;
zoneonof[program].levelmodeonof=0;
//zoneonof[program].standalonemodeof=0;
writemodeonofset();
readmodeonofset();
//readidcomset();
sprintf(BigSMS1,"");
send_vmodeonofset(PhoneNumber,program);
} 
}
else if(strstr(smsbuffer,"flowmodeonp") != 0 ) 
{
Tp1 = smsbuffer[11]-'0';
program= Tp1;
sprintf(buf,"\n pos=%d\n\r",program);
 PrintfResp(buf);
if(program>=1&&program<=4)
{
readmodeonofset();
zoneonof[program].timermodeonof=0;
zoneonof[program].flowmodeonof=1;
zoneonof[program].moisturemodeonof=0;
zoneonof[program].levelmodeonof=0;
//zoneonof[program].standalonemodeof=0;
writemodeonofset();
readmodeonofset();
//readidcomset();
sprintf(BigSMS1,"");
send_vmodeonofset(PhoneNumber,program);
} 
}
else if(strstr(smsbuffer,"moisturemodeonp") != 0 ) 
{
Tp1 = smsbuffer[15]-'0';
program= Tp1;
sprintf(buf,"\n pos=%d\n\r",program);
 PrintfResp(buf);
if(program>=1&&program<=4)
{

readmodeonofset();
zoneonof[program].timermodeonof=0;
zoneonof[program].flowmodeonof=0;
zoneonof[program].moisturemodeonof=1;
zoneonof[program].levelmodeonof=0;
//zoneonof[program].standalonemodeof=0;
writemodeonofset();
readmodeonofset();
//readidcomset();
sprintf(BigSMS1,"");
send_vmodeonofset(PhoneNumber,program); 
}
}
else if(strstr(smsbuffer,"levelmodeonp") != 0 ) 
{
Tp1 = smsbuffer[12]-'0';
program= Tp1;
sprintf(buf,"\n pos=%d\n\r",program);
 PrintfResp(buf);
if(program>=1&&program<=4)
{
readmodeonofset();
zoneonof[program].timermodeonof=0;
zoneonof[program].flowmodeonof=0;
zoneonof[program].moisturemodeonof=0;
zoneonof[program].levelmodeonof=1;
//zoneonof[program].standalonemodeof=0;
writemodeonofset();
readmodeonofset();
//readidcomset();
sprintf(BigSMS1,"");
send_vmodeonofset(PhoneNumber,program); 
}
}
else if(strstr(smsbuffer,"vstandalone") != 0 ) 
{
readidcomset();
//readmodeonofset();
//zonecom.standalonemodeonof=1;
writeidcomset();
readidcomset();
send_vsmodeonofset(PhoneNumber); 
}

else if(strstr(smsbuffer,"standalonemodeon") != 0 ) 
{
//if(MakeRealyOn==0)
if((MakeRealyOn==0) || (nVaTr.cyclecompleted ==1))
{
readidcomset();
//readmodeonofset();
zonecom.standalonemodeonof=1;
writeidcomset();
readidcomset();
send_vsmodeonofset(PhoneNumber); 
nDripSettings.strunsmsflag=1;
nDripSettings.runsmsflag=0;
}
}
else if(strstr(smsbuffer,"dripstandaloneon") !=0 )
{
	readidcomset();
	//nDripSettings.dripstandalone=1;
	writeidcomset();
	send_dripstandaloneonofset(PhoneNumber);
}

else if(strstr(smsbuffer,"dripstandaloneof") !=0 )
{
	readidcomset();
//	nDripSettings.dripstandalone=0;
	writeidcomset();
	send_dripstandaloneonofset(PhoneNumber);
	
}

else if(strstr(smsbuffer,"standalonemodeof") != 0 ) 
{
//if(MakeRealyOn==0) 
if((MakeRealyOn==0) || (nVaTr.cyclecompleted ==1))
{

readidcomset();
//readmodeonofset();
zonecom.standalonemodeonof=0;
writeidcomset();														
readidcomset();
send_vsmodeonofset(PhoneNumber); 
nDripSettings.strunsmsflag=0;
nDripSettings.runsmsflag=1;
}
}

else if(strstr(smsbuffer,"coilfbkon") !=0 )
{
	readidcomset();
	//nDripSettings.coilfbkonof=1;
	writeidcomset();
	send_coilfbkonof(PhoneNumber);
	
}
else if(strstr(smsbuffer,"coilfbkof") !=0 )
{
	readidcomset();
//	nDripSettings.coilfbkonof=0;
	writeidcomset();
	send_coilfbkonof(PhoneNumber);
	
}
else if(strstr(smsbuffer,"vcoilfbk") !=0 )
{
	readidcomset();
	send_coilfbkonof(PhoneNumber);
	
}
else if(strstr(smsbuffer,"gdf") != 0 ) 
{
{
//readidcomset();
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, ","  );
}
sprintf(buf,"\n gdf=%d\n\r",StrTokStrVer);
 PrintfResp(buf);
i= myatoi(StrTokStr1[1]);
//if(valvetimercounter>=370||valvetimercounter==185||MakeRealyOn==0)
//{
if(i<65)
nDripSettings.getsmsfeedback = myatoi(StrTokStr1[1]);
//GREENT[2].startfrom = myatoi(StrTokStr1[2]);
//GREENT[3].startfrom = myatoi(StrTokStr1[3]);
//GREENT[4].startfrom = myatoi(StrTokStr1[4]);
//writeidcomset();
//readidcomset();

nDripSettings.stchangefromflag=1;
sendfeedbacksmsalert(SmsNumber[0]);
//}
//STATE_SENDSMS = STATE_STANDZON_SMS;
//SendSmsToAll = 1;

//send_vstartfromset(PhoneNumber);
}
}

else if(strstr(smsbuffer,"gof") != 0 ) 
{
//sendallfeedbacksmsalert(SmsNumber[0]);
sendallfeedbacksmsalert_flag=1;
sendallfeedbacksmsalert_flag_1=0;
flag_count=0;

}

else if(strstr(smsbuffer,"valvemsgon") != 0 ) 
{
ReadonofFile();
valve_onof_msg=1;
WriteonofFile();

send_valveonof(PhoneNumber);
}

else if(strstr(smsbuffer,"valvemsgeof") != 0 ) 
{
ReadonofFile();
valve_onof_msg=0;
WriteonofFile();
send_valveonof(PhoneNumber);	
}
else if(strstr(smsbuffer,"nocommon") != 0 ) 
{
ReadonofFile();
nVaTr.reset_nocom_flag=1;
WriteonofFile();
send_nocommonof(PhoneNumber);	
}
else if(strstr(smsbuffer,"nocommof") != 0 ) 
{
ReadonofFile();
nVaTr.reset_nocom_flag=0;
WriteonofFile();
send_nocommonof(PhoneNumber);	
}

else if(strstr(smsbuffer,"lowtankrestarton") != 0 ) 
{
ReadonofFile();
nVaTr.lotank_cyc_restart_flag=1;
WriteonofFile();
send_lowtankrestartonof(PhoneNumber);	
}

else if(strstr(smsbuffer,"lowtankrestartof") != 0 ) 
{
ReadonofFile();
nVaTr.lotank_cyc_restart_flag=0;
WriteonofFile();
send_lowtankrestartonof(PhoneNumber);	
}

else if(strstr(smsbuffer,"stz") != 0 ) 
{
{
//readidcomset();
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, ","  );
}
sprintf(buf,"\n pod=%d\n\r",StrTokStrVer);
 PrintfResp(buf);
i= myatoi(StrTokStr1[1]);
//if(valvetimercounter>=370||valvetimercounter==185||MakeRealyOn==0)
if(valvetimercounter>=370||valvetimercounter==185||MakeRealyOn==0 || nVaTr.cyclecompleted ==1)

{
if(i<65)
nDripSettings.stchangefrom = myatoi(StrTokStr1[1]);
//GREENT[2].startfrom = myatoi(StrTokStr1[2]);
//GREENT[3].startfrom = myatoi(StrTokStr1[3]);
//GREENT[4].startfrom = myatoi(StrTokStr1[4]);
//writeidcomset();
//readidcomset();

nDripSettings.stchangefromflag=1;

STATE_SENDSMS = STATE_STANDZON_SMS;
SendSmsToAll = 1;

//send_vstartfromset(PhoneNumber);
}


else if(strstr(smsbuffer,"rstbckwash") != 0 ) 
{
nVaTr.resetbckwash_flag=1;
send_rstbckwashonof(PhoneNumber);
}
else if(strstr(smsbuffer,"vrefonof") != 0 ) 
{

ReadDprevSettings();
send_vrefreshonof(PhoneNumber);
}

else if(strstr(smsbuffer,"greenrtcon") != 0 ) 
{
readgreenonofset();
zonecom.greenRTCOnOf=1;
writegreenonofset();
readgreenonofset();
send_vgreenonofset(PhoneNumber);
}
else if(strstr(smsbuffer,"greenrtcof") != 0 ) 
{
readgreenonofset();
zonecom.greenRTCOnOf=0;
writegreenonofset();
readgreenonofset();
send_vgreenonofset(PhoneNumber);
}
else if(strstr(smsbuffer,"foggerrtcon") != 0 ) 
{
readgreenonofset();
zonecom.foggerRTCOnOf=1;
foggerRTCCheckFirstTime=0;
writegreenonofset();
readgreenonofset();
send_vfogstatset(PhoneNumber);
}
else if(strstr(smsbuffer,"foggerrtcof") != 0 ) 
{
readgreenonofset();
zonecom.foggerRTCOnOf=0;
writegreenonofset();
readgreenonofset();
send_vfogstatset(PhoneNumber);
}
else if(strstr(smsbuffer,"fanrtcon") != 0 ) 
{
readgreenonofset();
zonecom.fanRTCOnOf=1;
writegreenonofset();
readgreenonofset();
send_vfanstatset(PhoneNumber);
}
else if(strstr(smsbuffer,"fanrtcof") != 0 ) 
{
readgreenonofset();
zonecom.fanRTCOnOf=0;
writegreenonofset();
readgreenonofset();
send_vfanstatset(PhoneNumber);
}
else if(strstr(smsbuffer,"nightlightrtcon") != 0 ) 
{
readgreenonofset();
zonecom.nightlightRTCOnOf=1;
writegreenonofset();
readgreenonofset();
send_vnightlightstatset(PhoneNumber);
}
else if(strstr(smsbuffer,"nightlightrtcof") != 0 ) 
{
readgreenonofset();
zonecom.nightlightRTCOnOf=0;
writegreenonofset();
readgreenonofset();
send_vnightlightstatset(PhoneNumber);
}
else if(strstr(smsbuffer,"lightrtcon") != 0 ) 
{
readgreenonofset();
zonecom.lightRTCOnOf=1;
writegreenonofset();
readgreenonofset();
send_vlightstatset(PhoneNumber);
}
else if(strstr(smsbuffer,"lightrtcof") != 0 ) 
{
readgreenonofset();
zonecom.lightRTCOnOf=0;
writegreenonofset();
readgreenonofset();
send_vlightstatset(PhoneNumber);
}*/
else if(strstr(smsbuffer,"rtcon") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.RTCOnOf = 1;
		WriteTimerSettings();

		STATE_SENDSMS=STATE_RTCONOF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}
		else if(strstr(smsbuffer,"rtcof") != 0)
		{
		ReadTimerSettings();
		nTimerSettings.RTCOnOf = 0;
		WriteTimerSettings();

		STATE_SENDSMS=STATE_RTCONOF_SMS;
		SendSmsToAll = 1;
		ReadTimerSettings();
		}

/*else if(strstr(smsbuffer,"lightcyclictim") != 0 ) 
{
lightRTCCheckFirstTime=0;
readgreenonofset();
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, ","  );
}

Tp1= StrTokStr1[1][0]-'0';
Tp2= StrTokStr1[1][1]-'0';
zonecom.lightCycLicOnHr=(Tp1*10)+Tp2;
Tp1= StrTokStr1[1][2]-'0';
Tp2= StrTokStr1[1][3]-'0';
zonecom.lightCycLicOnMin=(Tp1*10)+Tp2;
Tp1= StrTokStr1[2][0]-'0';
Tp2= StrTokStr1[2][1]-'0';
zonecom.lightCycLicOfHr=(Tp1*10)+Tp2;
Tp1= StrTokStr1[2][2]-'0';
Tp2= StrTokStr1[2][3]-'0';
zonecom.lightCycLicOfMin=(Tp1*10)+Tp2;

writegreenonofset();
readgreenonofset();
send_vlightstatset(PhoneNumber);
}
else if(strstr(smsbuffer,"fogrestarttim") != 0 ) 
{
foggerRTCCheckFirstTime=0;
readgreenonofset();
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, ","  );
}

Tp1= StrTokStr1[1][0]-'0';
Tp2= StrTokStr1[1][1]-'0';
zonecom.foggerCycLicOnHr=(Tp1*10)+Tp2;
Tp1= StrTokStr1[1][2]-'0';
Tp2= StrTokStr1[1][3]-'0';
zonecom.foggerCycLicOnMin=(Tp1*10)+Tp2;

writegreenonofset();
readgreenonofset();
send_vfogstatset(PhoneNumber);
}

else if(strstr(smsbuffer,"fancyclictim") != 0 ) 
{
fanRTCCheckFirstTime=0;
readgreenonofset();
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, ","  );
}

Tp1= StrTokStr1[1][0]-'0';
Tp2= StrTokStr1[1][1]-'0';
zonecom.fanCycLicOnHr=(Tp1*10)+Tp2;
Tp1= StrTokStr1[1][2]-'0';
Tp2= StrTokStr1[1][3]-'0';
zonecom.fanCycLicOnMin=(Tp1*10)+Tp2;
Tp1= StrTokStr1[2][0]-'0';
Tp2= StrTokStr1[2][1]-'0';
zonecom.fanCycLicOfHr=(Tp1*10)+Tp2;
Tp1= StrTokStr1[2][2]-'0';
Tp2= StrTokStr1[2][3]-'0';
zonecom.fanCycLicOfMin=(Tp1*10)+Tp2;


writegreenonofset();
readgreenonofset();
send_vfanstatset(PhoneNumber);
}
else if(strstr(smsbuffer,"foggerrtctim") != 0 ) 
{
foggerRTCCheckFirstTime=0;
readgreenonofset();
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, ","  );
}

Tp1= StrTokStr1[1][0]-'0';
Tp2= StrTokStr1[1][1]-'0';
zonecom.foggerRTCOnHr=(Tp1*10)+Tp2;
Tp1= StrTokStr1[1][2]-'0';
Tp2= StrTokStr1[1][3]-'0';
zonecom.foggerRTCOnMin=(Tp1*10)+Tp2;
Tp1= StrTokStr1[2][0]-'0';
Tp2= StrTokStr1[2][1]-'0';
zonecom.foggerRTCOfHr=(Tp1*10)+Tp2;
Tp1= StrTokStr1[2][2]-'0';
Tp2= StrTokStr1[2][3]-'0';
zonecom.foggerRTCOfMin=(Tp1*10)+Tp2;

writegreenonofset();
readgreenonofset();
send_vfogstatset(PhoneNumber);
}

else if(strstr(smsbuffer,"fogvaltim") != 0 ) 
{
foggerRTCCheckFirstTime=0;
readgreenonofset();
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, ","  );
}
for(Tp=1;Tp<=4;Tp++)
{

Tp1= StrTokStr1[Tp][0]-'0';
Tp2= StrTokStr1[Tp][1]-'0';
GREENT[Tp].fogthr=(Tp1*10)+Tp2;
Tp1= StrTokStr1[Tp][2]-'0';
Tp2= StrTokStr1[Tp][3]-'0';
GREENT[Tp].fogtmin=(Tp1*10)+Tp2;
}

writegreenonofset();
readgreenonofset();
send_vfogstatset(PhoneNumber);
}
else if(strstr(smsbuffer,"vfanst") != 0 ) 
{
readgreenonofset();
send_vfanstatset(PhoneNumber);
}
else if(strstr(smsbuffer,"vfogst") != 0 ) 
{
readgreenonofset();
send_vfogstatset(PhoneNumber);
}
else if(strstr(smsbuffer,"vlightst") != 0 ) 
{
readgreenonofset();
send_vlightstatset(PhoneNumber);
}
else if(strstr(smsbuffer,"vnightlightst") != 0 ) 
{
readgreenonofset();
send_vnightlightstatset(PhoneNumber);
}

else if(strstr(smsbuffer,"fanrtctim") != 0 ) 
{
fanRTCCheckFirstTime=0;
readgreenonofset();
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, ","  );
}

Tp1= StrTokStr1[1][0]-'0';
Tp2= StrTokStr1[1][1]-'0';
zonecom.fanRTCOnHr=(Tp1*10)+Tp2;
Tp1= StrTokStr1[1][2]-'0';
Tp2= StrTokStr1[1][3]-'0';
zonecom.fanRTCOnMin=(Tp1*10)+Tp2;
Tp1= StrTokStr1[2][0]-'0';
Tp2= StrTokStr1[2][1]-'0';
zonecom.fanRTCOfHr=(Tp1*10)+Tp2;
Tp1= StrTokStr1[2][2]-'0';
Tp2= StrTokStr1[2][3]-'0';
zonecom.fanRTCOfMin=(Tp1*10)+Tp2;

writegreenonofset();
readgreenonofset();
send_vfanstatset(PhoneNumber);
}
else if(strstr(smsbuffer,"nightlightrtctim") != 0 ) 
{
nightlightRTCCheckFirstTime=0;
readgreenonofset();
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, ","  );
}

Tp1= StrTokStr1[1][0]-'0';
Tp2= StrTokStr1[1][1]-'0';
zonecom.nightlightRTCOnHr=(Tp1*10)+Tp2;
Tp1= StrTokStr1[1][2]-'0';
Tp2= StrTokStr1[1][3]-'0';
zonecom.nightlightRTCOnMin=(Tp1*10)+Tp2;
Tp1= StrTokStr1[2][0]-'0';
Tp2= StrTokStr1[2][1]-'0';
zonecom.nightlightRTCOfHr=(Tp1*10)+Tp2;
Tp1= StrTokStr1[2][2]-'0';
Tp2= StrTokStr1[2][3]-'0';
zonecom.nightlightRTCOfMin=(Tp1*10)+Tp2;

writegreenonofset();
readgreenonofset();
send_vnightlightstatset(PhoneNumber);
}

else if(strstr(smsbuffer,"lightrtctim") != 0 ) 
{
lightRTCCheckFirstTime=0;
readgreenonofset();
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, ","  );
}

Tp1= StrTokStr1[1][0]-'0';
Tp2= StrTokStr1[1][1]-'0';
zonecom.lightRTCOnHr=(Tp1*10)+Tp2;
Tp1= StrTokStr1[1][2]-'0';
Tp2= StrTokStr1[1][3]-'0';
zonecom.lightRTCOnMin=(Tp1*10)+Tp2;
Tp1= StrTokStr1[2][0]-'0';
Tp2= StrTokStr1[2][1]-'0';
zonecom.lightRTCOfHr=(Tp1*10)+Tp2;
Tp1= StrTokStr1[2][2]-'0';
Tp2= StrTokStr1[2][3]-'0';
zonecom.lightRTCOfMin=(Tp1*10)+Tp2;

writegreenonofset();
readgreenonofset();
send_vlightstatset(PhoneNumber);
}						
else if(strstr(smsbuffer,"greenrtctim") != 0 ) 
{
Tp1 = smsbuffer[11]-'0';
Tp2 = smsbuffer[12]-'0';

i = (Tp1*10)+(Tp2);
if(i>=1&&i<=12)
{
readgreenrtcset();
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, ","  );
}

Tp1= StrTokStr1[1][0]-'0';
Tp2= StrTokStr1[1][1]-'0';
GREENT[i].GRTCOnHr=(Tp1*10)+Tp2;
Tp1= StrTokStr1[1][2]-'0';
Tp2= StrTokStr1[1][3]-'0';
GREENT[i].GRTCOnMin=(Tp1*10)+Tp2;
Tp1= StrTokStr1[2][0]-'0';
Tp2= StrTokStr1[2][1]-'0';
GREENT[i].GRTCOfHr=(Tp1*10)+Tp2;
Tp1= StrTokStr1[2][2]-'0';
Tp2= StrTokStr1[2][3]-'0';
GREENT[i].GRTCOfMin=(Tp1*10)+Tp2;

writegreenrtcset();
readgreenrtcset();
send_vgreenrtcset(PhoneNumber,i);
}
}
else if(strstr(smsbuffer,"vprogcycle") != 0 ) 
{

readgreenrtcset();
sprintf(BigSMS1,"V57");
send_vgreencycleset(PhoneNumber);
}

else if(strstr(smsbuffer,"progcycle") != 0 ) 
{
readgreenrtcset();
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, ","  );
}
sprintf(buf,"\n pod=%d\n\r",StrTokStrVer);
 PrintfResp(buf);
for(Tp=1;Tp<=12;Tp++)
{
GREENT[Tp].programcycle = myatoi(StrTokStr1[Tp]);
}
writegreenrtcset();
readgreenrtcset();
sprintf(BigSMS1,"");
send_vgreencycleset(PhoneNumber);
}
else if(strstr(smsbuffer,"#vprogstart") != 0)
//else if(strstr(uart_read_buf,"#progstart") != 0)
{
readidcomset();
sprintf(BigSMS1,"V58");
send_vstartfromset(PhoneNumber);
}
else if(strstr(smsbuffer,"proqueon") != 0 ) 
{
ReadonofFile();
progqueonof =1;
WriteonofFile();
ReadonofFile();
readgreenrtcset();
sprintf(BigSMS1,"");
send_vprogqueset(PhoneNumber);
}
else if(strstr(smsbuffer,"proqueof") != 0 ) 
{
ReadonofFile();
progqueonof =0;
WriteonofFile();
ReadonofFile();
readgreenrtcset();
sprintf(BigSMS1,"");
send_vprogqueset(PhoneNumber);
}

else if(strstr(smsbuffer,"vproque") != 0 ) 
{

readgreenrtcset();
ReadonofFile();
sprintf(BigSMS1,"V38");
send_vprogqueset(PhoneNumber);
}
else if(strstr(smsbuffer,"proque") != 0 ) 
{
readgreenrtcset();
Pch1 = strtok((char *)smsbuffer, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
StrTokStrVer++;
Pch1 = strtok( NULL, ","  );
}
sprintf(buf,"\n pod=%d\n\r",StrTokStrVer);
 PrintfResp(buf);
for(Tp=1;Tp<=6;Tp++)
{
GREENT[Tp].progque = myatoi(StrTokStr1[Tp]);
}
writegreenrtcset();
readgreenrtcset();
ReadonofFile();
sprintf(BigSMS1,"");
send_vprogqueset(PhoneNumber);
}
else if(strstr(smsbuffer,"#vprogstart") != 0)
//else if(strstr(uart_read_buf,"#progstart") != 0)
{
readidcomset();
sprintf(BigSMS1,"V58");
send_vstartfromset(PhoneNumber);
}

else if(strstr(smsbuffer,"#progstart") != 0)
//else if(strstr(uart_read_buf,"#progstart") != 0)
{
char sbuff[30]="/0";
strcpy(sbuff,&smsbuffer[9]);
readidcomset();
Pch1 = strtok((char *)sbuff, (char *)"," );
StrTokStrVer = 0;
while( Pch1 != NULL )
{
strcpy(StrTokStr1[StrTokStrVer],Pch1);
//sprintf(buf,"\n pod=%d\n\r",StrTokStrVer);
 PrintfResp(buf);
//sprintf(buf,"\n pod=%s\n\r",StrTokStr1[StrTokStrVer]);
 PrintfResp(buf);
StrTokStrVer++;
Pch1 = strtok( NULL, ","  );
}
 sprintf(buf,"\n pod=%d\n\r",StrTokStrVer);
  PrintfResp(buf);
i= myatoi(StrTokStr1[0]);  
if(i<5)
{
GREENT[i].startfrom = myatoi(StrTokStr1[1]);
//GREENT[2].startfrom = myatoi(StrTokStr1[2]);
//GREENT[3].startfrom = myatoi(StrTokStr1[3]);
//GREENT[4].startfrom = myatoi(StrTokStr1[4]);
writeidcomset();
readidcomset();
sprintf(BigSMS1,"");
send_vstartfromset(PhoneNumber);
}
else if(strstr(smsbuffer,"getmoisturestat") != 0 ) 
{
for(Tp=1;Tp<=64;Tp++)
{
zone[Tp].actmoisture1=0;
zone[Tp].actmoisture2=0;
}
getmoistureflag=1;
}
else if(strstr(smsbuffer,"getlevelstat") != 0 ) 
{
for(Tp=1;Tp<=64;Tp++)
{
zone[Tp].actlevel1=0;
zone[Tp].actlevel2=0;
}
getlevelflag=1;
}
else if(strstr(smsbuffer,"getmoisture") != 0 ) 
{
Tp = smsbuffer[11]-'0';
Tp2 = smsbuffer[12]-'0';
program = (Tp*10)+Tp2;

send_vmoisturestat(PhoneNumber,program);
}
else if(strstr(smsbuffer,"getlevel") != 0 ) 
{
Tp = smsbuffer[11]-'0';
Tp2 = smsbuffer[12]-'0';
program = (Tp*10)+Tp2;

send_vlevelstat(PhoneNumber,program);
}
  else if(strstr(smsbuffer,"getdta") != 0 ) 
{
       Pch1 = strtok((char *)smsbuffer, (char *)"," );
		StrTokStrVer = 0;
		while( Pch1 != NULL )
		{
		strcpy(StrTokStr1[StrTokStrVer],Pch1);
		StrTokStrVer++;
		Pch1 = strtok( NULL, "," );
		}
		for(Tp=0;Tp<=StrTokStrVer;Tp++)
		{
		sprintf(buf,"\n\rSt=%s  pos=%d\n\r",StrTokStr1[Tp],Tp);
		 PrintfResp(buf);

		}
		zonecom.getdtaid = myatoi(StrTokStr1[1]);
		//zonecom.fog1id = myatoi(StrTokStr1[1]);
getdataflag=1;
}  
else if(strstr(smsbuffer,"#setserial") != 0 ) 
{ 
		setserialflag=1;
		sprintf(buf,"setserialflag set******************** \n\r ");
}

else if(strstr(smsbuffer,"#serialset") != 0 ) 
{ 
		Tp1 = smsbuffer[10]-'0';
		Tp2 = smsbuffer[11]-'0';
		Tp3 = smsbuffer[12]-'0';
		setserial1_val = (Tp1*100)+(Tp2*10)+Tp3;
		if(setserial1_val>255)
		setserial1_val=0;
		setserialflag_1=1;
		sprintf(buf,"serialset flag set******************** \n\r ");
		 PrintfResp(buf);
}

else if(strstr(smsbuffer,"getwlevel") != 0 ) 
{
	sprintf(buf,"getwlevel entry");
	 PrintfResp(buf);
	send_vwlevelstat(PhoneNumber);
	
}
else if(strstr(smsbuffer,"#getmos") != 0 ) 
{ 
	  char textBuf[160];
	int dricomdelay;
     sprintf(textBuf,"$D,M,1,\n\r ");
	 sAPI_UartWrite(eat_uart_app,textBuf, strlen(textBuf));
	 sprintf(buf,"%s\n\r ",textBuf);while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
	  PrintfResp(buf);
	 sAPI_UartWrite(eat_uart_app,textBuf, strlen(textBuf));
	 sprintf(buf,"%s\n\r ",textBuf);while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
	  PrintfResp(buf);
	 sAPI_UartWrite(eat_uart_app,textBuf, strlen(textBuf));
	 sprintf(buf,"%s\n\r ",textBuf);while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;  
	  PrintfResp(buf);
 	getdataflag_1=1;
	getdataflag=1;
}
else if(strstr(smsbuffer,"gethumidity") != 0 ) 
{

send_vhumiditystat(PhoneNumber);
}
else if(strstr(smsbuffer,"gettemp") != 0 ) 
{

send_vtempstat(PhoneNumber);
}
else if(strstr(smsbuffer,"getlightdensity") != 0 ) 
{

send_vlightdensitystat(PhoneNumber);
}
		else if(strstr((char *)smsbuffer,"valfogfbon") != 0 )
		{
		readidcomset();
		nVaTr.valvefogfeedbackonof=1;
		writeidcomset();
		 //STATE_SENDSMS=STATE_VALFBONOF_SMS;
		// SendSmsToAll = 1;
		readidcomset();
		send_vfogstatset(PhoneNumber);
		}
		else if(strstr((char *)smsbuffer,"valfogfbof") != 0 )
		{
		readidcomset();
		nVaTr.valvefogfeedbackonof=0;
		writeidcomset();
	//	 STATE_SENDSMS=STATE_VALFBONOF_SMS;
	//	 SendSmsToAll = 1;
		readidcomset();
		send_vfogstatset(PhoneNumber);

		}*/

		
		else if(strstr(smsbuffer,"timrun") != 0 )
		{
		//StopTimer(&timer);
		SendRunTime(ph_num);
		//timer.timeoutPeriod = 300;
		//timer.timerId = StartTimer(&timer);

		}
		/*else if(strstr(smsbuffer,"vinst") != 0 && ServiceNumberFound == 1)
		{

		ReadInstDate();
		//StopTimer(&timer);
		SendInstDate(ph_num);
		//timer.timeoutPeriod = 300;
		//timer.timerId = StartTimer(&timer);

		}
		else if(strstr(smsbuffer,"inst") != 0 && ServiceNumberFound == 1)
		{
		ReadInstDate();
		Pch1 = strtok((char *)smsbuffer, (char *)"," );
		StrTokStrVer = 0;
		while( Pch1 != NULL )
		{
		strcpy(StrTokStr1[StrTokStrVer],Pch1);
		StrTokStrVer++;
		Pch1 = strtok( NULL, "," );
		}
		for(Tp=0;Tp<=StrTokStrVer;Tp++)
		{
		sprintf(buf,"\n\rSt=%s  pos=%d\n\r",StrTokStr1[Tp],Tp);
		 PrintfResp(buf);

		}
		strcpy(InstDateName[0],StrTokStr1[1]);
		strcpy(InstDateName[1],StrTokStr1[2]);
		WriteInstDate();
		ReadInstDate();
		//StopTimer(&timer);
		SendInstDate(ph_num);
		//timer.timeoutPeriod = 300;
		//timer.timerId = StartTimer(&timer);
		}*/
		else if(strstr(smsbuffer,"sign#8185") != 0 )
		{
		ReadInstDate();
		Pch1 = strtok((char *)smsbuffer, (char *)"," );
		StrTokStrVer = 0;
		while( Pch1 != NULL )
		{
		strcpy(StrTokStr1[StrTokStrVer],Pch1);
		StrTokStrVer++;
		Pch1 = strtok( NULL, "," );
		}
		for(Tp=0;Tp<=StrTokStrVer;Tp++)
		{
		sprintf(buf,"\n\rSt=%s  pos=%d\n\r",StrTokStr1[Tp],Tp);
		 PrintfResp(buf);

		}
		strcpy(InstDateName[3],StrTokStr1[1]);
		WriteInstDate();
		ReadInstDate();
		//StopTimer(&timer);
		signaturename(ph_num);
		//timer.timeoutPeriod = 300;
		//timer.timerId = StartTimer(&timer);
		}
		else if(strstr(smsbuffer,"security#8185") != 0 && ServiceNumberFound == 1)
		{
		// security#8185,1234,9952835082
	//	 ReadInstDate();
		Pch1 = strtok((char *)smsbuffer, (char *)"," );
		StrTokStrVer = 0;
		while( Pch1 != NULL )
		{
		strcpy(StrTokStr1[StrTokStrVer],Pch1);
		StrTokStrVer++;
		Pch1 = strtok( NULL, "," );
		}
		for(Tp=0;Tp<=StrTokStrVer;Tp++) 
		{
		sprintf(buf,"\n\rSt=%s  pos=%d\n\r",StrTokStr1[Tp],Tp);
		PrintfResp(buf);

		}
		strcpy(SecuredPhoneNumber[0],StrTokStr1[1]);

		strcpy(SecuredPhoneNumber[1],StrTokStr1[2]);
		//WriteInstDate();
		//ReadInstDate();
		SendSecurity(ph_num);
		}
		else if(strstr(smsbuffer,"vsed") != 0 && ServiceNumberFound == 1)
		{
		ReadServiceDate();
		//StopTimer(&timer);
		SendServiceDate(ph_num);
		//timer.timeoutPeriod = 300;
		//timer.timerId = StartTimer(&timer);

		}

		else if(strstr(smsbuffer,"sed") != 0 && ServiceNumberFound == 1)
		{

		ReadServiceDate();

		Pch1 = strtok((char *)smsbuffer, (char *)"," );
		StrTokStrVer = 0;
		while( Pch1 != NULL )
		{
		strcpy(StrTokStr1[StrTokStrVer],Pch1);
		StrTokStrVer++;
		Pch1 = strtok( NULL, "," );
		}
		for(Tp=0;Tp<=StrTokStrVer;Tp++)
		{
		sprintf(buf,"\n\rSt=%s  pos=%d\n\r",StrTokStr1[Tp],Tp);
		PrintfResp(buf);

		}
		if(strcmp(ServiDateName[0],"no Date") ==0)
		{
		strcpy(ServiDateName[0],StrTokStr1[1]);
		strcpy(ServiDateName[1],StrTokStr1[2]);
		}
		else if(strcmp(ServiDateName[2],"no Date") ==0)
		{
		strcpy(ServiDateName[2],StrTokStr1[1]);
		strcpy(ServiDateName[3],StrTokStr1[2]);
		}
		else if(strcmp(ServiDateName[4],"no Date") ==0)
		{
		strcpy(ServiDateName[4],StrTokStr1[1]);
		strcpy(ServiDateName[5],StrTokStr1[2]);
		}
		else if(strcmp(ServiDateName[6],"no Date") ==0)
		{
		strcpy(ServiDateName[6],StrTokStr1[1]);
		strcpy(ServiDateName[7],StrTokStr1[2]);
		}
		else if(strcmp(ServiDateName[8],"no Date") ==0)
		{
		strcpy(ServiDateName[8],StrTokStr1[1]);
		strcpy(ServiDateName[9],StrTokStr1[2]);
		}
		else
		{
		strcpy(ServiDateName[0],StrTokStr1[1]);
		strcpy(ServiDateName[1],StrTokStr1[2]);
		}
		  WriteServiceDate();
		ReadServiceDate();
		//StopTimer(&timer);
		SendServiceDate(ph_num);
		//timer.timeoutPeriod = 300;
		//timer.timerId = StartTimer(&timer);
		 }
		 
         }
  		  else
		{
			PrintfResp("Number not match\n");
		}  
		memset(ph_num,0,sizeof(ph_num));
		memset(smsbuffer,0,sizeof(smsbuffer));		
		memset(rsp_buff,0,sizeof(rsp_buff));		
		memset(dtm,0,sizeof(dtm));		
			 
		 
	   }
    }
}
#endif
  
  void SmsDelAll(void)
  {    
				SIM_MSG_T msg;
				SC_SMSReturnCode ret;
				
				ret = sAPI_SmsDelAllMsg(g_sms_demo_msgQ);
                 
                if(ret == SC_SMS_SUCESS)
                {
                    memset(&msg,0,sizeof(msg));
                    status = sAPI_MsgQRecv(g_sms_demo_msgQ,&msg,SMS_URC_RECIVE_TIME_OUT);
                   // sprintf(buf,"\r\n\r\nsAPI_SmsDelAllMsg:\r\n\tprimID[%d] \r\n\tresultCode[%d]\r\n\trspStr[%s]\r\n",msg.arg1,msg.arg2, (INT8*)msg.arg3);
                   // sAPI_UartWriteString(SC_UART2, (UINT8*)buf);
				   PrintfResp("\n\rSMS Deleted\n\r");
                    sAPI_Free(msg.arg3);
                }
                else
                {
                    sAPI_UartWriteString(SC_UART2, (UINT8*)"\r\n\tDelete all msg failed!!\r\n");
                }
                
  }
  
  
  
 

 void sTask_UrcProcesser1(void *ptr)
{  
	char StrTokStr[20][20];
	char *Pch= NULL;	
    char rsp_buff[400] = {0};
    INT32 ret = 0;
    SC_STATUS osStatus = SC_FAIL;
    SIM_MSG_T msg = {0};
    INT32 tempStr[10];
    INT8 *Pbbuf = NULL;
    INT8 *Netbuf = NULL;
    INT8 *SMSbuf = NULL;
	INT8 *WIFIbuf = NULL;
    INT8 *GPSbuf = NULL;
    INT8 *GNSSbuf = NULL;
    INT8 *NMEADATAbuf = NULL;
    INT8 *AUDIObuf = NULL;
	char buf[400];
    osStatus = sAPI_MsgQCreate(&gUrcMsgQueue, "URC QUEUE", sizeof(SIM_MSG_T), 20, SC_FIFO);
    if (SC_SUCCESS != osStatus)
    {
        return;
    }

    ret = sAPI_UrcRefRegister(gUrcMsgQueue, SC_MODULE_ALL);
    if (0 != ret)
    {
        sAPI_MsgQDelete(gUrcMsgQueue);
        return;
    }
	//SmsDemoInit();                                         //invoking SMS receive Thread manually
    while(1)
    {
		
        if (sAPI_MsgQRecv(gUrcMsgQueue, &msg, SC_SUSPEND) != SC_SUCCESS)
        {
            continue;
        }

        if (SRV_URC != msg.msg_id)
        {
            sAPI_Free(msg.arg3);
            continue;
        }

		 module= msg.arg1;
		 urc=msg.arg2;
        // sprintf(buf,"module:%d, urc:%d", msg.arg1, msg.arg2);   //for sms it is 2,4
         sprintf(buf,"\n\rmodule:%d, urc:%d\n\r", module, urc);   //for sms it is 2,4
		 PrintfResp(buf); 
        switch(msg.arg1)
        {
            case SC_URC_PB_MASK:        // 0
                PrintfResp("URC_PB_MASK process!!");
                if(msg.arg2 == SC_URC_PBDOWN)
                {
                    Pbbuf = (INT8 *)msg.arg3;
                    sprintf(buf,"URC_PB_MASK:%s",Pbbuf);
					PrintfResp(buf);
					sAPI_AtSend("AT+CPIN?\r\n",strlen("AT+CPIN?\r\n"));
                }
            break;

            case SC_URC_SMS_MASK:    // 1
            {   
			
			 
			 
                if(msg.arg2 == SC_URC_FLASH_MSG)    //5
                { 
					PrintfResp("\n\r/*flash msg received */");
                    /*flash msg recved, +CMT*/
                    INT8 *p_msgpload = (INT8*)msg.arg3;
                    SIM_MSG_T SmsUrcProcessReq = {0};
                    if(p_msgpload != NULL)
                    {
                        sprintf(buf,"SC_URC_FLASH_MSG, new msg[%s]",p_msgpload);
						PrintfResp(buf);
                    }
                     
                }
                else if(msg.arg2 == SC_URC_NEW_MSG_IND)  //4
                {  
			         PrintfResp("\n\r/*New Msg Received*/");
                    /*new msg recved*/
                    INT8 *p_smsindex = (INT8*)msg.arg3;
					UINT8 at_buffer[30];
                    SIM_MSG_T SmsUrcProcessReq = {0};
                    if(p_smsindex != NULL)
                    {
                        sAPI_UartPrintf("SC_URC_NEW_MSG_IND, new sms string[%s]",p_smsindex);						
                        memset(tempStr, 0, 10);
                        memcpy(tempStr, p_smsindex+12,2);//remove +CMTI: "SM", get index string
                        msgIndex=atoi(tempStr);
						sprintf(at_buffer,"AT+CMGR=%d\r\n",msgIndex);
						sAPI_AtSend( at_buffer,strlen(at_buffer));    
						
						sprintf(at_buffer,"AT+CMGD=%d\r\n",msgIndex);
						sAPI_AtSend( at_buffer,strlen(at_buffer));   /* Delete msg buffer */ 
						 
                        // /*send msg to process task and read this msg*/
                        // SmsUrcProcessReq.msg_id = SC_URC_NEW_MSG_IND; //msgid
                        // SmsUrcProcessReq.arg1 = atoi(tempStr); //msg index
                        // sAPI_UartPrintf("SC_URC_NEW_MSG_IND, index[%d],msg_id[%d]",SmsUrcProcessReq.arg1, SmsUrcProcessReq.msg_id);
                        // /*Trigger urc process thread*/
                        // if(NULL != g_sms_demo_urc_process_msgQ)
						// {
                            // osStatus = sAPI_MsgQSend(g_sms_demo_urc_process_msgQ, &SmsUrcProcessReq);
                        // }
                        // if(osStatus != SC_SUCCESS)
                        // {
                            // sAPI_UartPrintf( "ERROR: %s, %d, send sms msg rsp failed, osaStatus=%d", __func__, __LINE__, osStatus);
                          
						// }

                    }
                }
                else if(msg.arg2 == SC_URC_STATUS_REPORT)  //3
                {
                    /*sms send status, +CDS*/
                    INT8 *p_smsStatus = (INT8*)msg.arg3;
                    if(p_smsStatus != NULL)
                    {
                        sAPI_UartPrintf( "SC_URC_STATUS_REPORT, p_smsStatus[%s]",p_smsStatus);
						 
                    }
                }
                else if(msg.arg2 == SC_URC_SMSDWON)  //1
                {
                    /*SMS DONE?*/
                    SMSbuf = (INT8*)msg.arg3;
                     sprintf(buf,"SC_URC_SMSDWON, msgbuf[%s]",SMSbuf);
					 PrintfResp(buf);
                }
                else if(msg.arg2 == SC_URC_SMSFULL)  //2
                {
                    /*SMS FULL*/
                    INT8 *p_msgpload = (INT8*)msg.arg3;
                    if(p_msgpload != NULL)
                    {
                        sAPI_UartPrintf("\n\rSC_URC_SMSFULL,p_msgpload[%s]",p_msgpload);
						ret = sAPI_SmsDelAllMsg(g_sms_demo_msgQ);
					    sAPI_UartPrintf("[sms], sAPI_SmsDelAllMsg, primID[%d] resultCode[%d] ",msg.arg1,msg.arg2);
						sAPI_AtSend("AT+CMGD=1,4\r\n",strlen("AT+CMGD=1,4\r\n"));   /* Delete msg buffer */  
						
						
                    }
                }

                break;
            }
					  
            
            case SC_URC_SIM_MASK:
            {
                sAPI_UartPrintf("SC_URC_SIM_MASK process!! pin status:%d",msg.arg2);
                switch(msg.arg2)
                {
                    case SC_URC_CPIN_READY:
					ModemIsReady=1;
                    /*add usercode here*/
                        break;

                    case SC_URC_CPIN_REMOVED:
                    /*add usercode here*/
                        break;

                    default:
                        break;
                }
                break;
            }

            /*for SIM2*/
            case SC_URC_SIM2_MASK:
            {
                sAPI_UartPrintf("SC_URC_SIM2_MASK process!! pin status:%d",msg.arg2);
                switch(msg.arg2)
                {
                    case SC_URC_CPIN_READY:
					ModemIsReady=1;
                    /*add usercode here*/
                        break;

                    case SC_URC_CPIN_REMOVED:
                    /*add usercode here*/
                        break;

                    default:
                        break;
                }
                break;
            }
            
            case SC_URC_NETSTATUE_MASK:
            {
                // PrintfResp("URC_NETSTATUE_MASK process!!");
                switch(msg.arg2)
                {
                    case SC_URC_NETACTED:
                    {
                        Netbuf = (INT8 *)msg.arg3;
                        sprintf(buf,"URC_NET_MASK NETACTED:%s",Netbuf);
						PrintfResp(buf);
						//int count=0;
                        /*add usercode here*/
						 
						 /* if(Netcon++>=1)
						 {
							// ntp(); 
							sAPI_TaskSleep((15) * 200);           //sleep 30 sec before disconn
							ret= sAPI_MqttDisConnect(0,NULL, 0, 60);
							sAPI_UartPrintf("8-----ret = %d", ret);
							
							ret= sAPI_MqttRel(0);
							sAPI_UartPrintf("9-----ret = %d", ret);
							
							ret= sAPI_MqttStop();
							sAPI_UartPrintf("10-----ret = %d", ret);
							 
							 sAPI_TaskSleep(200*5);
	                         ret = sAPI_MqttStart(-1);
							if(SC_MQTT_RESULT_SUCCESS == ret)    
								 PrintfResp("\r\nMQTT Init@ URC  Successful!\r\n");
							   
							else                                    
								PrintfResp("\r\nMQTT Init@ URC  Fail!\r\n");
                  
                
							ret= sAPI_MqttAccq(0, NULL, 0,IMEI, 0, urc_mqtt_msgq_1);
							if(SC_MQTT_RESULT_SUCCESS == ret)
								PrintfResp("\r\nMQTT accquire @ URC Successful!\r\n"); 
							 
							else						  
								PrintfResp("\r\nMQTT accquire@ URC  Fail!\r\n");
								 
						  
						
								ret= sAPI_MqttCfg(0, NULL, 0, 0, 0);
								 
									if(SC_MQTT_RESULT_SUCCESS == ret)                 
										PrintfResp("\r\nMQTT config@ URC  Successful!\r\n");
												
									else       
									  PrintfResp("\r\nMQTT config Fail@ URC !\r\n");
					 
								ret= sAPI_MqttConnect(0, NULL,0, "tcp://13.229.229.198:1883", 60, 1, "niagara", "niagara@123");
								if(SC_MQTT_RESULT_SUCCESS == ret)
								   {				 
								   PrintfResp("\r\nMQTT connect @ URC Successful!\r\n");
								    MqttInitStatus=1;count=0;MQTTReconflag=0; 
                                  }
								else
								{  
										sprintf(buf,"\n\rMQTT connect Fail,ERRCODE=== [%d]",ret);
										PrintfResp(buf);
										MqttInitStatus=0;					
										MQTTReconflag=1;count=0;
												
							
							          // while(!MqttInitStatus)						
							           while(count<30)						
										{
											 
											if(MQTTReconflag==1)
											{
											 sAPI_UartPrintf("\n\rMQTT Reconnect attempt:%d",count+1);
											 MQTT_Reconnect();											  
											}
											else
											{ 
											  PrintfResp("\r\nMQTT Reconnect Successful!\r\n");					 
											   MQTTReconflag=0;MqttInitStatus=1; 
											}
											count++;
										}
								}									
								  
							sprintf(topicc,"tweet-response/%s",IMEI);
							PrintfResp(topicc);				 
							ret = sAPI_MqttSub(0,topicc,strlen(topicc), 0, 0);
							if(SC_MQTT_RESULT_SUCCESS == ret)          
								PrintfResp("\r\nMQTT subscribe @ URC Successful!\r\n");  
							
							sprintf(topicc1,"get-tweet-response/%s",IMEI);
							PrintfResp(topicc1);				 
							ret = sAPI_MqttSub(0,topicc1,strlen(topicc1), 0, 0);
							if(SC_MQTT_RESULT_SUCCESS == ret)              				 
								 PrintfResp("\r\nMQTT subscribe @ URC Successful!\r\n");				  
								  
								  
						     	  
						 } */  
						 
						
                        break;
                    }
                    case SC_URC_NETDIS:
                    {
                        Netbuf = (INT8 *)msg.arg3;
                       // Netdisflag=1;
						MqttInitStatus=0;
						sprintf(buf,"URC_NET_MASK NETDIS:%s",Netbuf);
						PrintfResp(buf);
                        /*add usercode here*/
                        break;
                    }
                    case SC_URC_PDP_ACTIVE:
                    {
                        Netbuf = (INT8 *)msg.arg3;                        
						// sprintf(buf,"SC_URC_PDP_ACTIVE:%s",Netbuf);
						// PrintfResp(buf);
                        /*add usercode here*/
                        break;
                    }   
                    case SC_URC_PDP_DEACT:
                    {
                        UINT8 *p_cid = (UINT8*)(msg.arg3);
                        // sprintf(buf,"SC_URC_PDP_DEACT cid[%d]",*p_cid);
						// PrintfResp(buf);
                        break;
                    }
                        
                    default:
                        break;
                }
                break;

            }
            case SC_URC_INTERNAL_AT_RESP_MASK:
            {
                INT8 * respstr = (INT8 *)msg.arg3;
				SIM_MSG_T urcdata = {0};
				SIM_MSG_T smsurcdata = {0};
                char StrTokStr[20][20];
				char *Pch= NULL;				
				sAPI_UartPrintf("AT_RESP_MASK:%s",respstr);
				if(respstr != NULL)     
                { 		            
                    /*send msg to process thread*/
                    // urcdata.msg_id = SC_URC_FLASH_MSG; //msgid
                    urcdata.arg3 = sAPI_Malloc(strlen(msg.arg3)+1);//msg.arg3 will free after this process
					sAPI_UartPrintf("URC resp str Address before copy: %p\n", (void*)urcdata.arg3);
                    memset(urcdata.arg3, 0, (strlen(msg.arg3)+1));
                    // memcpy(urcdata.arg3, (INT8*)msg.arg3,strlen(msg.arg3)); //msg string
                    memcpy(urcdata.arg3,respstr,strlen(respstr)); //msg string
                    /*Trigger urc process thread*/
                    if(NULL != g_AT_urc_msgQ){
                        osStatus = sAPI_MsgQSend(g_AT_urc_msgQ, &urcdata);
                    }
                    if(osStatus != SC_SUCCESS)
                    {
                        PrintfResp("\n\rERROR:send sms msg rsp failed ");
                    }
                }	
				if(respstr != NULL)     
                { 		            
                    /*send msg to process thread*/
                    
                    smsurcdata.arg3 = sAPI_Malloc(strlen(msg.arg3)+1);//msg.arg3 will free after this process
                    memset(smsurcdata.arg3, 0, (strlen(msg.arg3)+1));                   
                    memcpy(smsurcdata.arg3,respstr,strlen(respstr)); //msg string
                    /*Trigger urc process thread*/
                    if(NULL != g_sms_demo_urc_process_msgQ){
                        osStatus = sAPI_MsgQSend(g_sms_demo_urc_process_msgQ, &smsurcdata);
                    }
                    if(osStatus != SC_SUCCESS)
                    {
                        PrintfResp("\n\rERROR:send sms msg rsp failed ");
                    }
                }
				/*if(strstr(respstr,"+CEREG: 1") != 0 )
				{
				    if(Netcon++>=1)
						 {
						   sAPI_UartPrintf("reconnecting mqtt");
						   MQTT_Reconnect();	 
						 } 
				}
				if(strstr(respstr,"+CEREG: 0") != 0 )
				{   
					 sAPI_UartPrintf("disconnecting mqtt");
				    MqttInitStatus=0;
					 
				}*/
			/*GET SIMCRAD NUMBER */
			 if(strstr(respstr,"+CNUM") != 0 )     
			 {  
		       memset(UNIT_PHNO,NULL,11);
			   strncpy(UNIT_PHNO,(respstr+15),10);			    
			   sAPI_UartPrintf("UNIT SIM NUMBER:{%s}",UNIT_PHNO);
			//   send_simno(ph_num);  //dg_nsdk
			 }
			 if(strstr(respstr,"+CPBR") != 0 )    //+CPBR: 10,"8110054669",129,"SMSM"
			 {
			Pch = strtok((char *)respstr, (char *)"\"\"" );
		    int StrTokStrVer = 0;
		    while( Pch != NULL )
		    {
		    strcpy(StrTokStr[StrTokStrVer],Pch);
		    StrTokStrVer++;
		    Pch = strtok( NULL,"\"\"" );
		    }
			for(Tp=0;Tp<=2;Tp++)
			{
				sprintf(buf,"\n\rSt=%s  pos=%d\n\r",StrTokStr[Tp],Tp);
				PrintfResp(buf);
				
			}
	          sAPI_UartPrintf("SMSM,%s,%s",StrTokStr[1],StoredPhoneNumber[0]);
			if(strlen(StrTokStr[1])>8)
			if(strcmp(StrTokStr[1],StoredPhoneNumber[0])==0)
			{
				strcpy(SmsNumber[0],(char *)StrTokStr[1]);
				sprintf(buf,"SmsNumber[0]:%s",SmsNumber[0]);
				PrintfResp(buf);				 
			//	ReadPhoneNumber();  //dg_nsdk
				strcpy((char *)StoredPhoneNumber[22],(char *)StoredPhoneNumber[0]);
				// sAPI_Debug("%s",StoredPhoneNumber[22]);
				strcpy((char *)StoredPhoneNumber[0],(char *)StrTokStr[1]);
				// sAPI_Debug("%s",StoredPhoneNumber[0]);
				//strcpy((char *)StoredPhoneSmscode[0],(char *)StrTokStr1[1]);
				
				//strcpy(StoredPhoneNumber[22],StoredPhoneNumber[0]);
				//strcpy(StoredPhoneNumber[0],SMSM);
			//	WritePhoneNumberFn(); //  dg_nsdk
				HowManyNumberFound=1;		 
				 
			}
			
		    }	
				break; 
			}
			case SC_URC_WIFI_SCAN_MASK:
			{
				WIFIbuf = (INT8 *)msg.arg3;
				/*add usercode here*/

                sAPI_UartWrite(SC_UART,(UINT8*)WIFIbuf,strlen(WIFIbuf)); //for WIFI UIDemo

				if(WIFIbuf != NULL)
                    sprintf(buf,"The result of wifi scan:[%s]",msg.arg3);
				 PrintfResp(buf);
				break;
			}

            case SC_URC_GNSS_MASK:
            {
                 switch(msg.arg2)
                {
                    case SC_URC_GPS_INFO:
                    {
                        GPSbuf = (INT8 *)msg.arg3;
                        sprintf(buf,"GPS INFO:%s",GPSbuf);
						 PrintfResp(buf);
                        
                        /*add usercode here*/
                        sAPI_UartWrite(SC_UART,(UINT8*)GPSbuf,strlen((UINT8*)GPSbuf)); //for GPSINFO UIDemo

                        break;
                    }
                    case SC_URC_GNSS_INFO:
                    {
                        GNSSbuf = (INT8 *)msg.arg3;
                        sprintf(buf,"GNSS INFO:%s",GNSSbuf);
						 PrintfResp(buf);

                        /*add usercode here*/
                        sAPI_UartWrite(SC_UART,(UINT8*)GNSSbuf,strlen((UINT8*)GNSSbuf)); //for GNSSINFO UIDemo
                        break;
                    }
                    case SC_URC_NMEA_DATA:  //get the orignal NMEA data
                    {
                        NMEADATAbuf = (INT8 *)msg.arg3;
                        sprintf(buf,"NMEA data:%s",NMEADATAbuf);
						PrintfResp(buf);

                        /*add usercode here*/
                        sAPI_UartWrite(SC_UART,(UINT8*)NMEADATAbuf,strlen((UINT8*)NMEADATAbuf)); //for NMEA data UIDemo
                        break;
                    }
                    default:
                        break;
                }
                break;
            }

            case SC_URC_CALL_MASK:
            {
                sprintf(buf,"SC_URC_CALL_MASK [%d]!!",msg.arg2);
				 PrintfResp(buf);
                switch(msg.arg2)
                {
                    case SC_URC_RING_IND:
                    {
                      INT8 *pCallBuf = (INT8*)msg.arg3;
                      if(pCallBuf != NULL)
                      {
                          sprintf(buf,"\n\rSC_URC_RING_IND[%s]",pCallBuf);
						  PrintfResp(buf);
						 // strncpy(Cph_num,(pCallBuf+18),12);
						 strncpy(CallUrcbuf,pCallBuf,5);
						  strncpy(Cph_num,(pCallBuf+20),10);
						  sprintf(buf,"\n\rMOBILE NO:[%s]",Cph_num);
						  PrintfResp(buf);
						  sprintf(buf,"\n\rCALL STR:[%s]",CallUrcbuf);
						  PrintfResp(buf);
                      }
                      break;
                    }
                    case SC_URC_DTMF_IND:
                    {
                      INT8 *pDtmfBuf = (INT8*)msg.arg3;
                      if(pDtmfBuf != NULL)
                      {
                        sprintf(buf,"SC_URC_DTMF_IND[%s]",pDtmfBuf);
						PrintfResp(buf);
						memset(DTMF_buf,0,15);
						strcpy(DTMF_buf,pDtmfBuf);          
						 
                      }
                      break;
                    }

                }
                break;
            }
            case SC_URC_AUDIO_MASK:
            {
                PrintfResp("SC_URC_AUDIO_MASK process!!");
                switch(msg.arg2)
                {
                    case SC_URC_AUDIO_START:
                    {
                        AUDIObuf = (INT8 *)msg.arg3;
                        sprintf(buf,"SC_URC_AUDIO_MASK:%s", AUDIObuf);
						 PrintfResp(buf);
                         austat=1;
                        sAPI_UartWrite(SC_UART2,(UINT8 *)AUDIObuf, strlen((UINT8 *)AUDIObuf));
                        break;
                    }
                    case SC_URC_AUDIO_STOP:
                    {
                        AUDIObuf = (INT8 *)msg.arg3;
                        sprintf(buf,"SC_URC_AUDIO_MASK:%s", AUDIObuf);
						PrintfResp(buf);
                         austat=0;
                        sAPI_UartWrite(SC_UART2,(UINT8 *)AUDIObuf, strlen((UINT8 *)AUDIObuf));
                        break;
                    }
                    case SC_URC_CREC_START:
                    {
                        AUDIObuf = (INT8 *)msg.arg3;
                        sprintf(buf,"SC_URC_AUDIO_MASK:%s", AUDIObuf);
						PrintfResp(buf);
                        
                        sAPI_UartWrite(SC_UART2,(UINT8 *)AUDIObuf, strlen((UINT8 *)AUDIObuf));
                        break;
                    }
                    case SC_URC_CREC_STOP:
                    {
                        AUDIObuf = (INT8 *)msg.arg3;
                        sprintf(buf,"SC_URC_AUDIO_MASK:%s", AUDIObuf);
						PrintfResp(buf);
                        
                        sAPI_UartWrite(SC_UART2,(UINT8 *)AUDIObuf, strlen((UINT8 *)AUDIObuf));
                        break;
                    }
                    case SC_URC_CREC_FIFULL:
                    {
                        AUDIObuf = (INT8 *)msg.arg3;
                        sprintf(buf,"SC_URC_AUDIO_MASK:%s", AUDIObuf);
						PrintfResp(buf);
                        
                        sAPI_UartWrite(SC_UART2,(UINT8 *)AUDIObuf, strlen((UINT8 *)AUDIObuf));
                        break;
                    }
                    default:
                        break;
                }

                break;
            }
            case SC_URC_TTS_MASK:
            {
                PrintfResp("SC_URC_TTS_IND process!!");
                if(msg.arg2 == SC_URC_TTS_IND)
                {
                    AUDIObuf = (INT8 *)msg.arg3;
                    sprintf(buf,"AUDIObuf INFO:%s",AUDIObuf);
					PrintfResp(buf);
                    
                    sAPI_UartWrite(SC_UART2,(UINT8*)AUDIObuf,strlen((UINT8*)AUDIObuf)); //for AUDIObuf UIDemo
                }
                break;
            }

            default:
                PrintfResp("default process!!");
                break;
        }

        sAPI_Free(msg.arg3);
        msg.arg3 = NULL;
    }

}
 
/* void ntp()
{
	SCsysTime_t currUtcTime;
    INT8 buff[220]={0};
	INT8 *resp = NULL;
	UINT32 ret = 0;
	SIM_MSG_T ntp_result = {SC_SRV_NONE, -1, 0, NULL};
	 if(NULL == ntpUIResp_msgq)
     {
        SC_STATUS status;
        status = sAPI_MsgQCreate(&ntpUIResp_msgq, "htpUIResp_msgq", sizeof(SIM_MSG_T), 4, SC_FIFO);
        if(SC_SUCCESS != status)
        {
          sAPI_Debug("[CNTP]msgQ create fail");
          resp = "\r\nNTP Update Fail!\r\n";
          sAPI_UartWrite(SC_UART2,(UINT8*)resp,strlen(resp));
         // break;
        }
      }

                memset(&currUtcTime,0,sizeof(currUtcTime));

                sAPI_GetSysLocalTime(&currUtcTime);
                sAPI_UartPrintf("[CNTP] sAPI_GetSysLocalTime %d - %d - %d - %d : %d : %d   %d",currUtcTime.tm_year,currUtcTime.tm_mon,currUtcTime.tm_mday,
                  currUtcTime.tm_hour,currUtcTime.tm_min,currUtcTime.tm_sec,currUtcTime.tm_wday);
                // ret = sAPI_NtpUpdate(SC_NTP_OP_SET, "ntp3.aliyun.com", 32, NULL);                        //Unavailable addr may cause long time suspend
                ret = sAPI_NtpUpdate(SC_NTP_OP_SET, "pool.ntp.org", 22, NULL);                        //Unavailable addr may cause long time suspend
                sAPI_UartPrintf("[CNTP] func[%s] line[%d] ret[%d]", __FUNCTION__,__LINE__,ret);

                ret = sAPI_NtpUpdate(SC_NTP_OP_GET, buff, 0, NULL);
                sAPI_UartPrintf("[CNTP] func[%s] line[%d] ret[%d] buff[%s]", __FUNCTION__,__LINE__,ret, buff);
            
                ret = sAPI_NtpUpdate(SC_NTP_OP_EXC, NULL, 0, ntpUIResp_msgq);
                sAPI_UartPrintf("[CNTP] func[%s] line[%d] ret[%d] ", __FUNCTION__,__LINE__,ret );
                do
                {
                    sAPI_MsgQRecv(ntpUIResp_msgq, &ntp_result, SC_SUSPEND);

                    if(SC_SRV_NTP != ntp_result.msg_id )                  //wrong msg received 
                    {
                        sAPI_Debug("[CNTP] ntp_result.msg_id =[%d], ntp_result.msg_id ");
                        ntp_result.msg_id = SC_SRV_NONE;                   //para reset
                        ntp_result.arg1 = -1;
                        ntp_result.arg3 = NULL;
                        continue;
                    }
                    if(SC_NTP_OK == ntp_result.arg1)                        //it means update succeed
                    {
                        sAPI_Debug("[CNTP] successfully update time! ");
                       PrintfResp("\r\nNTP Update Time Successful!\r\n");
					   NTPokFlag=1;
                        break;
                    }
                    else
                    {
                        sAPI_Debug("[CNTP] failed to update time! result code: %d", ntp_result.arg1);
                       PrintfResp("\r\nNTP Update Time Failed!\r\n");
                       NTPokFlag=0; 
						break;
                    }
                }while(1);

                memset(&currUtcTime,0,sizeof(currUtcTime));
                sAPI_GetSysLocalTime(&currUtcTime);
                sAPI_UartPrintf("[CNTP] sAPI_GetSysLocalTime %d - %d - %d - %d : %d : %d   %d",currUtcTime.tm_year,currUtcTime.tm_mon,currUtcTime.tm_mday,
                    currUtcTime.tm_hour,currUtcTime.tm_min,currUtcTime.tm_sec,currUtcTime.tm_wday);

	
} */ 
 
 
 #if 0
 void SmsDemoInit(void)
{
     PrintfResp(" \n\rSmsDemoInit init");
     SC_SMSReturnCode retval = SC_SMS_SUCESS;
     __attribute__((__unused__)) SC_STATUS status;
	 
   //      sAPI_SysGetImei(IMEI);
         status = sAPI_MsgQCreate(&g_sms_demo_msgQ, "g_sms_demo_msgQ", sizeof(SIM_MSG_T), 10, SC_FIFO);
         PrintfResp("\n\renter: sAPP_SmsTaskDemo init2");
         if(status != SC_SUCCESS){
            PrintfResp("ERROR: message queue creat err!\n");
            retval = SC_SMS_FAIL;
         }

        /*msgQ prepared for urc processor*/
         if(g_sms_demo_urc_process_msgQ == NULL)
             status = sAPI_MsgQCreate(&g_sms_demo_urc_process_msgQ, "g_sms_demo_urc_process_msgQ", sizeof(SIM_MSG_T), 20, SC_FIFO);
         if(status != SC_SUCCESS){
            PrintfResp("ERROR: message queue g_sms_demo_urc_process_msgQ creat err!\n");
            retval = SC_SMS_FAIL;
         }

         /*msgQ for urc process response*/
         status = sAPI_MsgQCreate(&g_sms_demo_urc_rsp_msgQ, "g_sms_demo_urc_rsp_msgQ", sizeof(SIM_MSG_T),20, SC_FIFO);
         if(status != SC_SUCCESS){
            PrintfResp("ERROR: message queue g_sms_demo_urc_rsp_msgQ creat err!\n");
            retval = SC_SMS_FAIL;
         }


         if(NULL == gSmsUrcProcessTask){
              /*creat a thread, prepared for */
              if(sAPI_TaskCreate(&gSmsUrcProcessTask, gSmsUrcProcessTaskStack, 1024*5,140, (char*)"SmsurcProcesser", sTask_SmsUrcProcessor,(void *)0))
             {
                 gSmsUrcProcessTask = NULL;
                 PrintfResp("Ping-------ERROR, Task Create error!\n");
             }
        }
		
 }
#endif

void sAPP_UrcTask1(void)
{
	
	 sAPI_UartPrintf("%s  line %d\n", __func__,__LINE__);
    if (NULL != gUrcProcessTask)
    {
		 sAPI_UartPrintf("%s  line %d\n", __func__,__LINE__);
        return;
    }
 sAPI_UartPrintf("%s  line %d\n", __func__,__LINE__);
    if (sAPI_TaskCreate(&gUrcProcessTask,
        gUrcProcessTaskStack,
        URC_PROCESS_TASK_STACK_SIZE,
        150,
        (char*)"urc processer" ,
        sTask_UrcProcesser1,
        (void *)0) != SC_SUCCESS)
    {
        gUrcProcessTask = NULL;
        PrintfResp("sAPI_TaskCreate error!\n");
		 sAPI_UartPrintf("%s  line %d\n",__func__,__LINE__);
    }
	sAPI_UartPrintf("%s  line %d\n",__func__,__LINE__);
}
// 

 #if 0
 void SmsDemoInit1(void)
{
     PrintfResp(" \n\rSmsDemoInit init");
     SC_SMSReturnCode retval = SC_SMS_SUCESS;
     __attribute__((__unused__)) SC_STATUS status;
	 
   //      sAPI_SysGetImei(IMEI);
         status = sAPI_MsgQCreate(&g_sms_demo_msgQ, "g_sms_demo_msgQ", sizeof(SIM_MSG_T), 10, SC_FIFO);
         PrintfResp("\n\renter: sAPP_SmsTaskDemo init2");
         if(status != SC_SUCCESS){
            PrintfResp("ERROR: message queue creat err!\n");
            retval = SC_SMS_FAIL;
         }

        /*msgQ prepared for urc processor*/
         if(g_sms_demo_urc_process_msgQ == NULL)
             status = sAPI_MsgQCreate(&g_sms_demo_urc_process_msgQ, "g_sms_demo_urc_process_msgQ", sizeof(SIM_MSG_T), 20, SC_FIFO);
         if(status != SC_SUCCESS){
            PrintfResp("ERROR: message queue g_sms_demo_urc_process_msgQ creat err!\n");
            retval = SC_SMS_FAIL;
         }

         /*msgQ for urc process response*/
         status = sAPI_MsgQCreate(&g_sms_demo_urc_rsp_msgQ, "g_sms_demo_urc_rsp_msgQ", sizeof(SIM_MSG_T),20, SC_FIFO);
         if(status != SC_SUCCESS){
            PrintfResp("ERROR: message queue g_sms_demo_urc_rsp_msgQ creat err!\n");
            retval = SC_SMS_FAIL;
         }


         if(NULL == gSmsUrcProcessTask){
              /*creat a thread, prepared for */
              if(sAPI_TaskCreate(&gSmsUrcProcessTask, gSmsUrcProcessTaskStack, 1024*5,140, (char*)"SmsurcProcesser", sTask_SmsUrcProcessor,(void *)0))
             {
                 gSmsUrcProcessTask = NULL;
                 PrintfResp("Ping-------ERROR, Task Create error!\n");
             }
        }
		
 }
#endif





