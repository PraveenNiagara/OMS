#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "eat_interface.h"
//#include "simcom_A7670C_gpio.h"
#include "simcom_api.h"
#include "math.h"
#include "eat_type.h"
#define DDEBUG
#define Store_Not_Txt_File               

#define DEBUG_ENABLE
#define SMS_ENABLE
#define TCP_Task_Enable
#define FALSE       0
#define TRUE        1

#define   Version        "PG25.2.4" //2-stands for gsm
#define   Build          __DATE__
/*CRC Related*/
#define CRC8_POLY 0x07 //31     x^8 + x^5 + x^4 + 1
#define CRC8_INIT 0x00 // Initial value
#define USE_FLAG 1

//static sFlagRef g_flg1 = NULL;

//#define TOFF 0
//#define TON 1


typedef struct {
    unsigned char sec; /* [0, 59] */
    unsigned char min; /* [0,59]  */
    unsigned char hour; /* [0,23]  */
    unsigned char day; /* [1,31]  */
    unsigned char mon; /* [1,12] */
    unsigned char wday; /* [1,7] */
    unsigned char year; /* [0,127] */
} EatRtc_st;

// extern char uart_read_buf[200];
// extern UINT16 uart_data_len;
extern long TpHr1,TpMin1,TpSec1,value1;
extern BOOL HttpInPrograssState;

extern UINT8 NTPokFlag;


extern BOOL ModemIsReady;
extern unsigned char Change_Date_Flag,day_send_flag,Date_Change_Flag;
extern const long int data_sec;
extern volatile long int seconds_data;
extern int ten_mins_timer;
extern char uart_read_buf[200];
extern unsigned char MassageReceived;
extern unsigned char sendtoMcuack,sendtoMcu;	
extern char Buffer1[500];
//extern char UartBuffer[500]; //dg_nsdk
extern INT8 buf[1050]; //500
//extern UINT8 MQ[200]; //dg_nsdk
extern UINT8 DTMF_buf[15];
//extern UINT8 CalUrcbuf[500]; //dg_nsdk
extern UINT8 SMSMno[15];
extern UINT8 CallUrcbuf[10];
extern int module,urc;
extern UINT8 MqttInitStatus;
extern UINT8 MQTTReconflag;
extern unsigned char MassageReceived;
//extern char PhoneNumber_1[20] = "0000000000";
//extern INT8 *pDtmfBuf;
//extern char BalanceStr[250]; //dg_nsdk
extern char BigSMS1[10];
extern unsigned int valvetimercounter;
extern char count_msg;   //dg_changed
extern char Balanceflag;
extern unsigned char CallConnected ,ringflag;
extern unsigned char sendallfeedbacksmsalert_flag,flag_count,sendallfeedbacksmsalert_flag_1;
extern BOOL Http_send_flag;
extern SCFILE *ret;//changed
extern SCFILE *file_hdl;//changed
extern char pfile[20];//changed
extern sMsgQRef urc_mqtt_msgq_1;
extern  BOOL austat;
extern UINT8 fotafailflag;
extern int Netcon;
extern char UNIT_PHNO[10];
extern long int SIM_NO;
extern unsigned int RST_count;

#define eat_uart_app SC_UART 
//#ifdef DDEBUG
//#else
#define eat_uart_wifi SC_UART3
//#endif

//#define NULL 0
#define MCONFIG_AT_INDEX			1
#define MCONFIG_FLASH				2
#define MCONFIG_FOTA				3
#define FOTASPACE 0x102F3000

#define SPP_PIN     			SC_MODULE_GPIO_00    

#define NETLIGHT	 		    SC_MODULE_GPIO_09 
#define STATUS_PIN				SC_MODULE_GPIO_10


#define DEFAULTNUMBER		"+919842505100"  //ravikumar
#define DEFAULTNUMBER1		"+919845200686"  //"9865265200" 
#define DEFAULTNUMBER2		"+919344205677"  //utham sir
#define DEFAULTNUMBER3		"+919865705100"  //solution service
#define DEFAULTNUMBER4		"+919942988218"  //agritel
#define DEFAULTNUMBER5		"+918012505677"  //agritel
#define DEFAULTNUMBER6		"+917373745677"  //gopal
#define DEFAULTNUMBER7		"+919865705101"  //sabareesh
#define DEFAULTNUMBER8		"+917373705105"  //Selvam
#define DEFAULTNUMBER9		"+919865566200"  //Madhurai Veeran
#define DEFAULTNUMBER10		"+919994444349"  //Askar  
#define DEFAULTNUMBER11		"+919845200686"   
#define DEFAULTNUMBER12		"+919865265200"   
extern unsigned char DNDSMSFLAG[5];
extern unsigned char ftpgettofs_cb_flag;
extern unsigned char valve_onof_msg;
extern int Tp,Tp1,Tp2,Tp3,Tp4,min,max,step,fert,i,j,program;
extern unsigned char livedataflag,livedataflag1,livedataflagcount1,livedataflagcount,Appmodeon,wifion,gprson,gprsgeton,apmodeon,getdataflag,tcpdcounter,tcpdcounter3,tcpdcounter1,tcpdcounterflag,fotaflsg,setserialflag,progqueonof,wpsmodeon,setserialflag_1,setserial1_val,getdataflag_1;


extern BOOL pressure_calib_flag;
 

extern char filter_sms_flag;
extern char filter_sms_flag_count;
extern unsigned char WaitOK;
extern unsigned char SendSMS;
extern unsigned char getmoistureflag;
extern unsigned char getlevelflag;
extern long int dripcycledate;
extern long int dripcyclecount;
extern unsigned char Noofsettingsrecvd,Noofsettingsprocessed;
extern unsigned char Nooftcprecvd,Nooftcpprocessed,Nooftcprecvd1,Nooftcpprocessed1;
extern unsigned char resetflag,resendflag,commandflag;
extern unsigned char resetflagcounter,resendflagcounter;
extern unsigned char uartflag;
extern unsigned char wifiopenflag,errorcount,tcpopenflag,rebootflag,rebootflag1,tcpcopyflag,wifimodesetfalg,wifiresetflag,wifiresetflag1,wifiresetcounter,sgetflag,sgetflag_1;
//extern char delims[] = ",";
//extern char TpStrtok[30][20];  //dg_nsdk
extern int TpStrtokVer;
extern int NumberOfSMSSend,Nooftcpsendcount;
extern UINT8 Enter;
extern const unsigned char ver[13];
//extern const u32 DeviceFlashStartAddr;
extern char StoredPhoneNumber[25][15];
extern char StoredPhoneSmscode[16][15];
extern unsigned char CallOnOfVer;
extern unsigned char SendSMS;
extern char regsmsno;
extern char PhoneNumber[20] ;
extern char SendSMSString[350];
extern unsigned char ThisSMSisNotPowerFault;
extern unsigned char NeedToCPBRSearchAgain;
extern unsigned char SendSmsToAll;
extern unsigned char CallOnOfVer;
extern unsigned char NumberChangeSMS;
extern unsigned char RegxSmsSend;
extern UINT8 CREG,CGATT,CSQ;
// char BigSMS2[10];
extern UINT8 IMEI[16];
extern UINT8 SMSM[15];
extern UINT8 COPS[21];
extern UINT8 apn[30];
extern UINT8 Egprs;
extern UINT8 Sapbr;
extern UINT8 CCID[21];
//extern UINT8 smssendbuf[160];  ////dg_nsdk
extern UINT8 GSMInitDone;
extern BOOL HttpInitStatus;
extern UINT8 DIS_BUF[80];
extern UINT8 VAL_BUF[80];
extern UINT8 SMS_BUF[350]; //200
extern char TCPWifigprsstrBUFF[1350];//800//600
//extern char FBKBuf[2024];
//extern EatRtc_st datetime;
extern t_rtc datetime;
extern unsigned char MakeRealyOn ;
extern float leveltank,settank_height;
extern unsigned int levelpercent;
extern unsigned char creg_count,creg_reset_flag;


extern BOOL creg_reg_flag,creg_reg_flag_1;
extern unsigned char Last_filter;
//extern BOOL sand_filter_flag,disc_filter_flag,sand_filter_completed,disc_filter_completed;
typedef unsigned char bool;
typedef unsigned char Boolean;
typedef unsigned char u8;
typedef signed char s8;
typedef signed char ascii;
typedef unsigned short int u16;
typedef signed short int s16;
typedef unsigned int u32;
typedef signed int s32;
typedef unsigned long long u64;
typedef signed long long s64;
typedef unsigned char uint8;
typedef signed char int8;
typedef unsigned short int uint16;
typedef signed short int int16;
typedef unsigned int uint32;
typedef signed int int32;
//typedef enum{TON,TOFF}Boolean;
typedef enum{LOC,TON,TOFF,STATUS,ALERTON,ALERTOFF,RECONON,RECONOFF,IGNALTON,IGNALTOFF,RLYON,RLYOFF,LIST}temp_niagara6;
typedef enum{High,Low}temp_niagara5;
//typedef enum{Server_ERROR,Server_OK,Server_CONFIG,Server_CONFIGR};
typedef enum{APN,ADDC,DELC,INF$,IP,FTP}temp_niagara4;
typedef enum{DISABLE,ENABLE}temp_niagara3;
typedef enum{OFF,ON}temp_niagara2;

struct __WIFIstrnumber
{
	char Wifistr[130];//100 //80
//	char wifinumber[15];
};
struct __TCPWIFIstrnumber
{
	char TCPWifistr[100];  //520
	char TCPWifigprsstr[600];  //520	
//	char TCPwifinumber[30];
};
struct __TCPWIFIstrnumber1
{
//	char TCPWifistr1[420];
	char TCPWifigprsstr1[1000];
	//char TCPwifinumber1[30];
};


extern unsigned char responsecode;


/* struct MoTr
{	
long ActnopowerRunTimer;
long Act2powerRunTimer
long Act3powerRunTimer
} */

struct ConfigMaker
{
	unsigned char Sno,Object,Input_No,Output_No,Status;
};
struct Constant
{
	unsigned char Sno,Object;
	float FlowRate;//uint32_t FlowRate;
	float LiterPerPluse,Pressure;
};
struct zone
{
	unsigned char Sno,ValveNo[4],MainValve,IrrigationMethod,Duration[3],ProgramNo;
	float FlowRate;//uint32_t FlowRate;
	
	
};
struct Program
{
	unsigned char Sno,DelayBtwZone[3],Schedule,StartDate[3],SelectedDate[10],DayCount,EndDate[3],Rtc[3],
	Alaram,ProgramNo,Status,NoofZones,ControlSeclection;
	float ScaleFact,SetPressure,SetTolerence;
	uint32_t SetFlowRate;
	struct zone nZone[8];

};

struct ProgramProcess
{
	unsigned char Sno,ProgramEnable,ProgramStatus,ZoneNo,NoofZones,ZoneFlag,ZoneOnFlag,ValveON,
	MainValveOn,Status,ControlSeclection;
	uint32_t StopSec,StartSec,RemaningSec,RemainingFlow;
	float SetValue,SetTolerence;
	
};


// struct OMSValvefeedback
// {
//     unsigned char Sno,Object,Input_No,Output_No;
// };
// struct OMSPressurefeedback
// {
//     unsigned char Sno,Object,Input_No,Output_No;
// };
// struct OMSMainValvefeedback
// {
//     unsigned char Sno,Object,Input_No,Output_No;
// };
struct OMSfeedback{
	unsigned char Sno,Object,Input_No,Output_No,MainValvenofbk;
	float Pressure,Current,Voltage;
	uint32_t Cummulative;
	int LPS;
};
struct MSettings
{
    unsigned char staticSMSOnOff,sampleSMSOnOff,canSMSOnOff,dataSMSOnOff;
	unsigned char SMSOnOff,ndebugonof;
	unsigned char SfbOnOff;
	unsigned char TankOnOff,PressureOnOff,ManualswitchOnOff;
	unsigned char SumpOnOff;
	unsigned char DryRunOnOff;
	unsigned char TargetOnOff;
	unsigned char serviceOnOff;
	unsigned char pwrvfbOnOf;
	unsigned char VBFOnOff;
	unsigned char RelayControlOnCall,motor1onof,motor2onof,motor3onof,motor4onof,motor1onofcount,motor2onofcount,motor3onofcount,motor4onofcount,motorallonofcount,motor1onof_flag,motor2onof_flag,motor3onof_flag;
	unsigned char SecOnOf;
	unsigned char gethidesmsonoff;
	unsigned char SumprstOnOff,motor4ctrlonof;
	unsigned char CONT_VER[3];
	unsigned char act_rem_DrHr[3],act_rem_DrMin[3],act_rem_DrSec[3];
};
extern char BalanceNumber[20] ;
typedef enum
{
    STATE_NO_SMS,
}nSTATE_SENDSMS;
#if 0
struct Dripsettings
{
	unsigned char calconof;		//**********Drip variable declarartion**************
	unsigned char stchangefrom,stchangefromflag,strunflag,entersetting,changefrom,changefromflag,startfrom,cyclestartfrom,startfogfrom,tstartfrom,prvstartfrom,startcontrol,driponof,dripgapdayonof,startcontrolt,decidelast,decidefirst,dripgapdays,dripgapdaycount,fertskipfirst1,fertskipfirst2,fertskipfirst3,fertskiplast1,fertskiplast2,fertskiplast3,fertskip1,fertskip2,fertskip3,fertskip4,fertskip5,fertskip6,fertskip7,fertskip8,decidefblast;
	unsigned char dripcode,ddelcode,checkagain,checkfogagain,strunsmsflag,runsmsflag,getsmsfeedback;
	unsigned int strunsmsflagcounter,runsmsflagcounter;
	unsigned char dripcode1onof,dripcode2onof,dripcode3onof,
	dripcode4onof,dripcode5onof,dripcode6onof,dripcode7onof,
	dripcode8onof,dripcode9onof,dripcode10onof,dripcode11onof,
	dripcode12onof,dripcode13onof,dripcode14onof,
	dripcode15onof;
	
	int stp,calc,prvstp,changeinstp,changeinstp1,stchangeincalc,changeincalc,ststp,prvststp,changeinststp ;
	int fogstp,fogcalc,prvfogstp,changeinfogstp,changeinfogcalc ;
	unsigned char strunsmscountHr,strunsmscountMin,strunsmscountSec;
	long strunsmscounter,strunsmscounter_1;
	unsigned char fbk_open_flag,fbk_close_flag,fbk_open_open_flag;
	
	//unsigned char delfert111Hr,delfert111Min,delfert111Sec;
	
};

typedef enum
{
    STATE_NO_SMS,
	STATE_SMSON_SMS,
	STATE_STANDZON_SMS,
	STATE_LIMIT_SMSON_SMS,
	STATE_LIMITSMSSET_SMS,
	STATE_VLIMITSMSSET_SMS,
	STATE_stSMSON_SMS,
	STATE_sampleSMSON_SMS,
	STATE_canSMSON_SMS,
	STATE_dataSMSON_SMS,
	STATE_gethidesmson_SMS,
    STATE_SFB_SMS,
	STATE_PRESSURE_SMS,
	STATE_MANUALSWITCH_SMS,
    STATE_TANK_SMS,
    STATE_SUMP_SMS,
    STATE_DRYRUN_SMS,
    STATE_TARGET_SMS,
	STATE_VFB_SMS,
	STATE_PWRVFB_SMS,
	STATE_ONDELAY_SMS,
	STATE_SDDELAY_SMS,
	STATE_SFBDELAY_SMS,
	STATE_DRRSTONOF_SMS,
	STATE_DRRST_SMS,
	STATE_DRSCANONOF_SMS,
	STATE_DRSCAN_SMS,
	STATE_MAXTIMONOF_SMS,
	STATE_MAXTIM_SMS,
	STATE_CYCLICONOF_SMS,
	STATE_FERTCYCLICONOF_SMS,
	STATE_CYCLICTIMON_SMS,
	STATE_CYCLICTIMOF_SMS,
	STATE_VALVEONOF_SMS,
	STATE_GAPDAYS_SMS,
	STATE_SKIPDAYCOUNT_SMS,
	STATE_DRIPGAPDAYONOF_SMS,
	STATE_VALVEVIEW_SMS,
	STATE_FIRST_VALVEVIEW_SMS,
	STATE_STVALVEVIEW_SMS,
	STATE_FIRST_STVALVEVIEW_SMS,
	STATE_CYCLE_COMPLETED_SMS,
	STATE_FERTON_SMS,
	STATE_FERT2ON_SMS,
	STATE_FERT3ON_SMS,
	STATE_FERT1OF_SMS,
	STATE_FERT2OF_SMS,
	STATE_FERT3OF_SMS,
	STATE_VALVESKIPVIEW_SMS,
	STATE_ENTERDRIPSETON_SMS,
	STATE_CYCRESTARTONOF_SMS,
	STATE_QUERESTARTONOF_SMS,
	STATE_CALFLOWONOF_SMS,
	STATE_VALFBONOF_SMS,
	STATE_DELVALFBONOF_SMS,
	STATE_VALFBKTIM_SMS,
	STATE_REFRESHONOF_SMS,
	STATE_VREFRSHTIMON_SMS,
	STATE_VREFRSHTIMOF_SMS,
	STATE_STARTFROM_SMS,
	STATE_CHANGEFROM_SMS,
	STATE_DELFERT_SMS,
	STATE_DECIDELAST_SMS,
	STATE_FERTSKIPDETAILS_SMS,
	STATE_FERTSKIPGROUPDETAILS_SMS,
	STATE_DECIDEFIRST_SMS,
	STATE_VALVENO_SMS,
	STATE_VALVENODEL_SMS,
	STATE_RTCONOF_SMS,
	STATE_RTCTIMON1_SMS,
	STATE_RTCTIMOF1_SMS,	
	STATE_RTCTIMON2_SMS,
	STATE_RTCTIMOF2_SMS,
	STATE_RTCTIMON3_SMS,
	STATE_RTCTIMOF3_SMS,
	STATE_RTCTIMON4_SMS,
	STATE_RTCTIMOF4_SMS,
	STATE_DATETIME_SMS,
	STATE_PROGRAMSEL_SMS,
	STATE_VISET_SMS,	
	STATE_SECONOF_SMS,
	STATE_SCRDLONOF_SMS,
	STATE_SCRDEL_SMS,
	STATE_POSCRDLONOF_SMS,
	STATE_POSCRDEL_SMS,
	STATE_LOWVOLTONOFF_SMS,
	STATE_LOWVOLTII_SMS,
	STATE_LOWVOLTIII_SMS,
	STATE_HIGHVOLTONOFF_SMS,
	STATE_HIGHVOLTII_SMS,
	STATE_HIGHVOLTIII_SMS,
	STATE_IMBVOLT_SMS,
	STATE_PFCVOLT_SMS,
	STATE_REVPHASE_SMS,
	STATE_CURSPP_SMS,
	STATE_SPP_SMS,
	STATE_CALFLOWII_SMS,
	STATE_CALFLOWIII_SMS,
	STATE_DRAMPII_SMS,
	STATE_DRAMPIII_SMS,
	STATE_OLINOFF_SMS,
	STATE_LOWPRESS_SMS,
	STATE_HIGHPRESS_SMS,
	STATE_HIGHPRESSONOF_SMS,
	STATE_LOWPRESSONOF_SMS,
	STATE_OLAMPSII_SMS,
	STATE_OLAMPSIII_SMS,
	STATE_OLSCAN_SMS,
	STATE_AUTOSTII_SMS,
	STATE_AUTOSTIII_SMS,
	STATE_AUTOOLDRRST_SMS,
	STATE_AUTODRRUNRST_SMS,
	STATE_POWERFACTOR_SMS,
	STATE_MANULONOF_SMS,
	STATE_OLRSTVOLONOFF_SMS,
	STATE_OLRSTVOL_SMS,
	STATE_DROCCONOFF_SMS,
	STATE_DROCCTIM_SMS,
	STATE_DROCCNUM_SMS,
	STATE_DIFFVOLTII_SMS,
	STATE_DIFFVOLTIII_SMS,
	STATE_CALPV_SMS,
	STATE_CALPC_SMS,
	STATE_CALV_SMS,
	STATE_CALC_SMS,
	STATE_VIEWCALV_SMS,
	STATE_VIEWCALC_SMS,
	STATE_CTRON_SMS,
	STATE_CTROF_SMS,
	STATE_CTYON_SMS,
	STATE_CTYOF_SMS,
	STATE_CTBON_SMS,
	STATE_CTBOF_SMS,
	STATE_SERVICE_SMS,
	STATE_INSTALLATIONDATE_SMS,
	STATE_VINSTALLATIONDATE_SMS,
	STATE_AUTORST2ONOFF_SMS,
	STATE_AUTORSTONOFF_SMS,
	STATE_HIDIFFVOLTII_SMS,
	STATE_HIDIFFVOLTIII_SMS,
	STATE_REFRESH1ONOF_SMS,
	STATE_REFRESH2ONOF_SMS,
	STATE_REFRESH3ONOF_SMS,
	STATE_REFRESH4ONOF_SMS,
	STATE_SUMP_RST_SMS,
	STATE_FOTA_SMS,
	STATE_MQTT_STATUS_SMS,
}nSTATE_SENDSMS;
#endif
struct MoTr
{	
	unsigned long RTCON[6];
	unsigned long RTCOf[6];
	long ActnopowerRunTimer;
	long Act2powerRunTimer;
	long Act3powerRunTimer;
	long ActonpowerRunTimer;
	long ActlnightlightRunTimer;
	long ActllightRunTimer;
/*	long OnDelay;
	long StarDetaDelay;
	long SfbDelay;
	long DrRunRestart;
	long DrRunScan;
	long CycLicOnDelay;
	long CycLicOfDelay;
	long MaxRunTimer;
	
	long Scrdl;
	long PoScrdl;
	long DrOccurTim;
	long DrOccurNum;
	long OlScan;
	
	long ActOnDelay;
	long ActDrRunRestart;
	long ActDrRunScan;
	long ActStarDetaDelay;
	long ActSfbDelay;
	long ActCycLicOnDelay;
	long ActlightCycLicOnDelay;
	long ActfanCycLicOnDelay;
	long ActfoggerCycLicOnDelay;
	long ActnightlightCycLicOnDelay;
	long ActCycLicOfDelay;
	long ActMaxRunTimer;
	long Act2powerRunTimer;
	long ActonpowerRunTimer;
	long ActofpowerRunTimer;
	long Act3powerRunTimer;
	long ActnopowerRunTimer;
	long Actl2powerRunTimer;
	long Actl3powerRunTimer;
	long ActlnopowerRunTimer;
	long ActlF1RunTimer;
	long ActlF2RunTimer;
	long ActlF3RunTimer;
	long ActlF4RunTimer;
	long ActlnightlightRunTimer;
	long ActllightRunTimer;
	long ActlfogRunTimer;
	long ActlfanRunTimer;
	long ActRTCON1;
	long ActRTCOf1;
	long ActRTCON2;
	long ActRTCOf2;
	long ActRTCON3;
	long ActRTCOf3;
	long ActRTCON4;
	long ActRTCOf4;
	long ActScrdl;
	long ActPoScrdl;
	long ActDrOccurTim;
	long ActDrOccurNum;
	long ActOlScan,ActHPScan,ActLPScan;
	*/
};
struct TimerSettings
{
	unsigned char RTCOnHr[5],RTCOnMin[5],RTCOnSec[5];
	unsigned char RTCOfHr[5],RTCOfMin[5],RTCOfSec[5];
	
	unsigned char POnHr,POnMin,POnSec;
	unsigned char SDHr,SDMin,SDSec;
	unsigned char SfbHr,SfbMin,SfbSec;
	unsigned char DrReOnOf,Drrestartpoweronof,Driprestartpoweronof;
	unsigned char DrReHr,DrReMin,DrReSec;
	unsigned char DrScOnOf;
	unsigned char DrScHr,DrScMin,DrScSec;
	unsigned char MaxRnOnOf;
	unsigned char MaxRnHr,MaxRnMin,MaxRnSec;
	unsigned char CycLicOnOf;
	unsigned char CycLicOnHr,CycLicOnMin,CycLicOnSec,fertCycLicOnMin,fertCycLicOnSec;
	unsigned char CycLicOfHr,CycLicOfMin,CycLicOfSec,fertCycLicOfMin,fertCycLicOfSec;
	unsigned char RTCOnOf;
	
	//unsigned char RTCOnHr2,RTCOnMin2,RTCOnSec2;
	//unsigned char RTCOfHr2,RTCOfMin2,RTCOfSec2;
	//unsigned char RTCOnHr3,RTCOnMin3,RTCOnSec3;
	//unsigned char RTCOfHr3,RTCOfMin3,RTCOfSec3;
	//unsigned char RTCOnHr4,RTCOnMin4,RTCOnSec4;
	//unsigned char RTCOfHr4,RTCOfMin4,RTCOfSec4;
	//unsigned char valv111Hr,valv111Min,valv111Sec;		//valve variable declarartion
	unsigned char ScrDlOnOff;
	unsigned char ScrDlHr,ScrDlMin,ScrDlSec;
	unsigned char PoScrDlOnOff;
	unsigned char PoScrDlHr,PoScrDlMin,PoScrDlSec;
	unsigned char LowVoltOnOff;
	int LowVoltII,LowVoltIII;
	unsigned char HighVoltOnOff;
	int HighVoltII,HighVoltIII;
	int DiffVoltII,DiffVoltIII;
	int ImbVolt,pfcvolt,vpfcvolt;
	unsigned char RvePhOnoff;
	unsigned char SppOnoff;
	unsigned char CurSppOnOff;
	unsigned char OlOnOff,lowpressOnOff,highpressOnOff;
	double DrAmpsII,DrAmpsIII,calflow2,calflow3;
	double OlAmpsII,OlAmpsIII,lowpress,highpress;
	int wrDrAmpsII,wrDrAmpsIII,wrlowpress,wrhighpress;
	int wrOlAmpsII,wrOlAmpsIII;
	unsigned char  OlScanHr,OlScanMin,OlScanSec;
	unsigned char AutoStIIOnOff;
	unsigned char AutoStIIIOnOff;
	unsigned char AutoOlDrRstIIOnOff,AutoDrRunRstIIOnOff,pfcOnOff;
	unsigned char ManualOnOff;
	unsigned char OlRstVolOnoff;
	unsigned char OlRstVol;
	unsigned char DrOccurOnOff;
	unsigned char DrOccurTimHr,DrOccurTimMin,DrOccurTimSec;
	unsigned char DrOccurNum;
	double CalRVoltage,CalYVoltage,CalBVoltage;
	double CalRCurrent,CalYCurrent,CalBCurrent;
	int wrCalRVoltage,wrCalYVoltage,wrCalBVoltage;
	int wrCalRCurrent,wrCalYCurrent,wrCalBCurrent;
	unsigned char CTRonoff,CTYonoff,CTBonoff;
	unsigned char AutoRst2On;
	unsigned char AutoRstOn;
	int HiDiffVoltII,HiDiffVoltIII;
	double RPhaseToPhaseFactor;
	double YPhaseToPhaseFactor;
	double BPhaseToPhaseFactor; 
	double R2PhaseToPhaseFactor;
	int wrRPhaseToPhaseFactor;
	int wrYPhaseToPhaseFactor;
	int wrBPhaseToPhaseFactor; 
	int wrR2PhaseToPhaseFactor;
	
};

struct CurrentCond
{
	double RVoltage,YVoltage,BVoltage;
	double RYVoltage,YBVoltage,BRVoltage;
	double Rcurrent,Ycurrent,Bcurrent;
	
};
/* struct dripid{
    unsigned int ida1,ida2,ida3,ida4;
    double idcal;
	int wridcal;
	long fsno,fbk,fbkval,datatype;
}; */
#if 0
struct dripcom{
    unsigned char f1id;//s
	unsigned char f2id;//s
	unsigned char f3id;//s
	unsigned char f4id;//s
	
	
	unsigned char flowid;//s
	unsigned char sumpid;//s
	unsigned char tankid;//s
	unsigned char extid;//s
	
	unsigned char fog1id;//s
	unsigned char fog2id;//s
	unsigned char fog3id;//s
	unsigned char fog4id;//s
	unsigned char getdtaid;//s
	unsigned char EMS1id;//s
	unsigned char EMS2id;//s
	unsigned char EMS3id;//s
	unsigned char EMS4id;//s
	
	
	unsigned char humtemp1id;//s
	unsigned char humtemp2id;//s
	unsigned char humtemp3id;//s
	unsigned char humtemp4id;//s
	
	unsigned char light1id;//s
	unsigned char light2id;//s
	
	unsigned char fan1id;//s
	unsigned char fan2id;//s
	
	double humcal1,tempcal2,ldenscal3,extcal4;//s
	double fertflowcal1,fertflowcal2,fertflowcal3,fertflowcal4;//s
	int wrhumcal1,wrtempcal2,wrldenscal3,wrextcal4;//s
	int wrfertflowcal1,wrfertflowcal2,wrfertflowcal3,wrfertflowcal4;//s
	int ctratio,ptratio,humonlevel,humoflevel,temponlevel,tempoflevel,ldensonlevel,ldensoflevel;
	unsigned char standalonemodeonof,flowfertonof;
	unsigned char valdelaymin,valdelaysec,fertdelaymin,fertdelaysec,fbkdelmin,fbkdelsec,mixdelaymin,mixdelaysec,daycountmin,daycountsec;
	//unsigned char moistureonlevel,moistureoflevel;
	unsigned char greenRTCOnOf,foggerRTCOnOf,fanRTCOnOf,lightRTCOnOf,nightlightRTCOnOf,fanCycLicOnOf,lightCycLicOnOf;
	unsigned char lightCycLicOnHr,lightCycLicOnMin,lightCycLicOfHr,lightCycLicOfMin;
	unsigned char fanCycLicOnHr,fanCycLicOnMin,fanCycLicOfHr,fanCycLicOfMin;	
	unsigned char foggerRTCOnHr,foggerRTCOnMin,foggerRTCOfHr,foggerRTCOfMin;
	unsigned char fanRTCOnHr,fanRTCOnMin,fanRTCOfHr,fanRTCOfMin;
	unsigned char lightRTCOnHr,lightRTCOnMin,lightRTCOfHr,lightRTCOfMin;
	unsigned char nightlightRTCOnHr,nightlightRTCOnMin,nightlightRTCOfHr,nightlightRTCOfMin;
	unsigned char foggerCycLicOnHr,foggerCycLicOnMin;
	long int lightcyclicOnDelay,lightcyclicOfDelay;	
	long int fancyclicOnDelay,fancyclicOfDelay;
	long int foggerRTCON,foggerRTCOF;
	long int fanRTCON,fanRTCOF;
	long int lightRTCON,lightRTCOF;
	long int nightlightRTCON,nightlightRTCOF;
	long int foggercyclicOnDelay;
	unsigned char fog1feedback,fog2feedback,fog3feedback,fog4feedback;
	};
#endif
#if 0
struct driponof
{
 	
	unsigned char timermodeonof,flowmodeonof,moisturemodeonof,levelmodeonof;
    unsigned char moistureonof,temponof,humidityonof,flowonof,flowfertonof;
	unsigned char timerpercent,flowpercent,moisturepercent,fertpercent,fertdelaypercent;
	unsigned char humidity,temperature,lightdensity;
	unsigned char moistureonlevel,moistureoflevel;
	unsigned char aquaonlevel,aquaoflevel;
	unsigned char phlevel;
};
struct dripgreen
{	
	long int GRTCON,GRTCOF;
    unsigned char GRTCOnHr,GRTCOnMin,GRTCOfHr,GRTCOfMin;
	unsigned char programcycle,FirstTimerinProgram,progque;
	unsigned char startfrom,fogthr,fogtmin,stpstartfrom,ststpstartfrom;
};

struct drip{

    unsigned int actv1id;
	unsigned int actv2id;
	unsigned int actv3id;
	unsigned int actv4id;
	
    unsigned int p1v1id;
	unsigned int p1v2id;
	unsigned int p1v3id;
	unsigned int p1v4id;
	
	unsigned int p2v1id;
	unsigned int p2v2id;
	unsigned int p2v3id;
	unsigned int p2v4id;
	
	unsigned int p3v1id;
	unsigned int p3v2id;
	unsigned int p3v3id;
	unsigned int p3v4id;
	
	unsigned int p4v1id;
	unsigned int p4v2id;
	unsigned int p4v3id;
	unsigned int p4v4id;
	
	unsigned char actl1id;
	unsigned char actl2id;
	unsigned char actm1id;
	unsigned char actm2id;
	unsigned char p1l1id;
	unsigned char p1l2id;
	
	unsigned char p1m1id;
	unsigned char p1m2id;
	unsigned char p2l1id;
	unsigned char p2l2id;
	
	unsigned char p2m1id;
	unsigned char p2m2id;
	unsigned char p3l1id;
	unsigned char p3l2id;
	
	unsigned char p3m1id;
	unsigned char p3m2id;
	unsigned char p4l1id;
	unsigned char p4l2id;
	
	unsigned char p4m1id;
	unsigned char p4m2id;
	unsigned char comid;
	unsigned char actthr,acttmin;
	unsigned char p1thr,p1tmin;
	unsigned char p2thr,p2tmin;
	unsigned char p3thr,p3tmin;
	unsigned char p4thr,p4tmin;
	unsigned char actf1min,actf1sec;	
	unsigned char p1f1min,p1f1sec;
	unsigned char p1f2min,p1f2sec;
	unsigned char p1f3min,p1f3sec;
	unsigned char p1f4min,p1f4sec;
	unsigned char actf2min,actf2sec;
	unsigned char p2f1min,p2f1sec;
	unsigned char p2f2min,p2f2sec;
	unsigned char p2f3min,p2f3sec;
	unsigned char p2f4min,p2f4sec;
	unsigned char actf3min,actf3sec;
	unsigned char p3f1min,p3f1sec;
	unsigned char p3f2min,p3f2sec;
	unsigned char p3f3min,p3f3sec;
	unsigned char p3f4min,p3f4sec;
	unsigned char actf4min,actf4sec;
	unsigned char p4f1min,p4f1sec;
	unsigned char p4f2min,p4f2sec;
	unsigned char p4f3min,p4f3sec;
	unsigned char p4f4min,p4f4sec;
	int actf1flow;
	int actf2flow;
	int actf3flow;
	int actf4flow;
	
	
	int p1f1flow;
	int p1f2flow;
	int p1f3flow;
	int p1f4flow;
	
	int p2f1flow;
	int p2f2flow;
	int p2f3flow;
	int p2f4flow;
	
	int p3f1flow;
	int p3f2flow;
	int p3f3flow;
	int p3f4flow;
	
	int p4f1flow;
	int p4f2flow;
	int p4f3flow;
	int p4f4flow;
	
	int actlevel1;
	int actlevel2; 
	
	int actmoisture1;
	int actmoisture2;
	long stopmoisture1;
	long stopmoisture2;
	long stoptime;
	long actflowrate,Actflowrate;
	float runflowrate,stopflowrate,runpressure,stoppressure,remflowrate,remfflowrate,outpressure;
	long zstartflowrate;
	long zstopflowrate;
	int p1flowrate,p2flowrate,p3flowrate,p4flowrate;
	unsigned char v1feedback,v2feedback,v3feedback,v4feedback;
	unsigned char actv1feedback,actv2feedback,actv3feedback,actv4feedback;
	
	
};
struct VaTr
{	
	long STEP1,FOGSTEP1;
	long STEP2,FOGSTEP2;
    unsigned long REMTIM,FOGREMTIM;
	long TSTEP1,TSTEP2,FSTEP2,MOSSTEP1,FLOWSTEP1,FOGTSTEP1,FOGTSTEP2,FOGFSTEP2;
	long CurrentSec;
	long FERT1ON,FERT2ON,FERT3ON,FERT4ON;
	long FERT1OF,FERT2OF,FERT3OF,FERT4OF;
	long FERT1,FERT2,FERT3,FERT4,FERT5,FERT6;
	long FERF1FLOW,FERF2FLOW,FERF3FLOW,FERF4FLOW;
	long CurrentF1flow,CurrentF2flow,CurrentF3flow,CurrentF4flow,Currentlevel,Currentmoisture1,Currentmoisture2,samplemoisture1,samplemoisture2,Currentflow,watthour,pwrfactor;	
	unsigned char Currentvalve,Skipvalve,Currentstvalve;
	unsigned char Currentfogvalve,Skipfogvalve;
	unsigned char fertv1onof,fertv2onof,fertv3onof,fertv4onof;
	unsigned char fertv1Smsonof,fertv2Smsonof,fertv3Smsonof ,fertv4Smsonof;
	unsigned char refreshv1Smsof,refreshv2Smsof,refreshv3Smsof,refreshv4Smsof;
	unsigned char refreshv1Smson,refreshv2Smson,refreshv3Smson,refreshv4Smson;
	unsigned char cyclecompleted,cycrestartonof,querestartonof,fogcycrestartonof,fogcyclecompleted;
	unsigned char gotfeedback,gotfeedback1,valvefeedbackonof,delvalvefeedbackonof,calflowrateonof,valvefbkTimHr,valvefbkTimMin,valvefbkTimSec;
	unsigned char gotfogfeedback,valvefogfeedbackonof;
	
	unsigned char refreshvalveonof,refreshvalveonof1,refreshvalveonof2,refreshvalveonof3,refreshvalveonof4,RefreshTimonHr,RefreshTimonMin,RefreshTimonSec,RefreshTimofHr,RefreshTimofMin,RefreshTimofSec;
	unsigned char RefreshTimonHr1,RefreshTimonMin1,RefreshTimonSec1;
	unsigned char RefreshTimonHr2,RefreshTimonMin2,RefreshTimonSec2;
	unsigned char RefreshTimonHr3,RefreshTimonMin3,RefreshTimonSec3;
	unsigned char RefreshTimonHr4,RefreshTimonMin4,RefreshTimonSec4;
	unsigned char resetbckwash_flag,reset_nocom_flag,lotank_cyc_restart_flag;
	//long RefOfDelay,RefOnDelay,RefOnDelay1,RefOnDelay2,RefOnDelay3,RefOnDelay4,RefOfDelay1,RefOfDelay2,RefOfDelay3,RefOfDelay4;
	long RefOfDelay,RefOnDelay[8];
	 long ActRefreshOnDelay,ActRefreshOfDelay;
	 unsigned char RefreshOndelayCompleted;
	int sendremaintime,sendcheckagain,sendremainfogtime,sendcheckfogagain;
	long struntime,prvstruntime;
	long strunflow,prvstrunflow;
	unsigned char programselection;
	unsigned char no_fbk_flag;
	unsigned char pswdelaymin,pswdelaysec;
	unsigned int pswdelay;
	unsigned char cregdelayHr,cregdelayMin;
	long cregdelay;
	unsigned char cregrstonof;
};

	
typedef struct 
{
	long int Motor_Runtime,Motor_Idletime,Power_off_time,Dry_run_trip_time,cyclic_trip_time,other_trip_time,day_flow;
	char Motor_RuntimeHr,Motor_RuntimeMin;
	char Motor_IdletimeHr,Motor_IdletimeMin;
	char Power_off_timeHr,Power_off_timeMin;
	char Dry_run_trip_timeHr,Dry_run_trip_timeMin;
	char cyclic_trip_timeHr,cyclic_trip_timeMin;
	char other_trip_timeHr,other_trip_timeMin;
}timer;
#endif
struct VaTr
{
	unsigned char cregrstonof;
	unsigned char cregdelayHr,cregdelayMin;
	long cregdelay;
	long struntime,prvstruntime;
//	unsigned int pswdelay;
};
struct dripcom{
unsigned char nightlightRTCOnOf,lightRTCOnOf,standalonemodeonof;
};
struct ntimer
{
	long m_M1_Runtime,m_M2_Runtime,m_M3_Runtime;
	long m_M1_Idletime,m_M2_Idletime,m_M3_Idletime,Power_off_time;
	long m_M1Dry_run_trip_time,m_M2Dry_run_trip_time,m_M3Dry_run_trip_time;	
	long m_M1cyclic_trip_time,m_M2cyclic_trip_time,m_M3cyclic_trip_time;
	long m_M1other_trip_time,m_M2other_trip_time,m_M3other_trip_time;
	
};

struct settings{
	
	char m_DryRunOnOff[4],m_MaxRnOnOf[4], m_ScrDlOnOff[4], m_PoScrDlOnOff[4], m_DrOccurOnOff[4];
	char m_Driprestartpoweronof[4], m_SumpOnOff[4], m_AutoDrRunRstIIOnOff[4],m_AutoStIIOnOff[4],m_AutoOlDrRstIIOnOff[3];
	char m_OlOnOff[4],m_pfcOnOff[4], m_ndebugonof[4], m_RvePhOnoff,m_SppOnoff,m_ManualswitchOnOff[4];
	char m_LowVoltOnOff,m_highpressOnOff[4],m_lowpressOnOff[4];
	char m_HighVoltOnOff;
	char m_SfbOnOff[4];
	char m_CurSppOnOff;
	char m_DrReOnOf[4];
	char m_Drrestartpoweronof[4];
	char m_OlRstVolOnoff[4];
	char m_DrScOnOf[4];
	char m_RTCOnOf[4];
	char m_CycLicOnOf[4];
	char m_RelayControlOnCall;
	char m_SecOnOf,m_PoScrdlOnOff;
	char m_settings_req_flag,m_settings_count,m_pumpno_send,m_Enter,m_uart_send_flag,calibration_flag;
	char m_CTRonoff,m_CTYonoff,m_CTBonoff;
	char m_NlightOnOf,m_NlightRTCOnHr,m_NlightRTCOnMin,m_NlightRTCOnSec,m_NlightRTCOfHr,m_NlightRTCOfMin,m_NlightRTCOfSec;
	char m_peakHourOnOf,m_peakOnHr,m_peakOnMin,m_peakOnSec,m_peakOfHr,m_peakOfMin,m_peakOfSec;
	
	float m_Flow_calfactor,m_Press_calfactor;
	double m_CalRVoltage,m_CalYVoltage,m_CalBVoltage;
	double m_CalRCurrent,m_CalYCurrent,m_CalBCurrent;
	double m_CalRVoltage_Rx,m_CalYVoltage_Rx,m_CalBVoltage_Rx;
	double m_CalRCurrent_Rx,m_CalYCurrent_Rx,m_CalBCurrent_Rx;
	char m_ValveTimerHr[10],m_ValveTimerMin[10];
	char CycLicIntervelHr,CycLicIntervelMin,CycLicIntervelSec,ValveOnOff,CyclicOnOff,CyclicLimit;
	
};

/* struct pump
{
	char m_Sump_on_off;
	char m_Tank_on_off;
	char m_no_of_sump_pins;
	char m_no_of_tank_pins;
	char m_sump_pin_no[2];
	char m_tank_pin_no[2];
	char m_Level_on_off;
	char m_pressureonof;
	char m_flowonof;
}; */

extern unsigned int myatoi(char *s);
extern double myatof( char *s);
extern struct MoTr nMoTr;
extern struct VaTr nVaTr;
//extern struct dripid zoneid[257	];
extern struct dripcom zonecom;
//extern struct dripgreen GREENT[13];
//extern struct driponof zoneonof[6];
//extern struct drip zone[65];
//extern struct Dripsettings nDripSettings;
extern struct CurrentCond nCurretnCond;
extern struct TimerSettings nTimerSettings;
extern struct MSettings nMSettings;
extern struct ConfigMaker nConfig[10];
extern struct Constant nConstant[10];
extern struct Program nProgram;
extern struct ProgramProcess nProgramProcess;
//extern struct zone nZone[4];
extern struct OMSfeedback s_nOMSfeedback[10];
extern struct settings s_nMSettings;
extern struct __WIFIstrnumber wifiStrNumber[40]; //60
extern struct __TCPWIFIstrnumber TCPwifiStrNumber[50];  //60
extern struct __TCPWIFIstrnumber1 TCPwifiStrNumber1[10];   //13
 //extern struct timer tday[7];
//extern timer tday[7];
//extern struct pump s_npump[3];

extern u32 APP_DATA_STORAGE_BASE;
extern int NoOfConstant,NoOfObject,NoOfZone,NoOfProgram;
typedef void (*func_ptr) (void*);
void ReadPhoneNumber(void);
void send_test_smsNum(char *PhoneNumber,char *String);
extern void WriteSettingsFile(void);
extern void ReadFile(void);
		
#define ASSERT(expr) assert_internal(expr, __FILE__, __LINE__)
extern void assert_internal(bool expr, const char *file, int line);

#define EAT_UART_RX_BUF_LEN_MAX (2000) 

#define TRACE_DEBUG(...) eat_trace(__VA_ARGS__)


typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short int u16;
typedef signed short int s16;

typedef unsigned int u32;
typedef signed int s32;

typedef unsigned long long u64;
typedef signed long long s64;

#endif

