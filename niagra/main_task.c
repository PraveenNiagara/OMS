#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "userspaceConfig.h"
#include "simcom_debug.h"
#include "simcom_os.h"
#include "simcom_gpio.h"
#include "A7678_V102_GPIO.h"
#include "simcom_network.h"
#include "simcom_uart.h"

#include "app_at_cmd_envelope.h"
#include "app_custom.h"
#include "smshandling.h"
#include "ModemConfig.h"
#include "livedata.h"
#include "app_utility.h"
#include "math.h"
#include "platform.h"
#include "stdarg.h"

#include "simcom_api.h" //
#include "eat_type.h"
#include "eat_fs_type.h"

#include "simcom_common.h"
#include "simcom_sms.h"
#include "simcom_file.h"
#include "uart_api.h"

#define UART_RX_PROCESS_TASK_STACK_SIZE (1024 * 2) /* UART proc*/ //(1024 * 6)

#define FlowBased 1
#define TimeBased 2
#define PressureBased 2

#define UART_RX_BUFFER_SIZE  128 //200 // RX buffer can not more than 2048
#define UART_TX_BUFFER_SIZE 200
#define MCONFIG_AT_CMD_MAX_LEN 128

static sMsgQRef gUart_msgq = NULL;
static sMsgQRef gUart3_msgq = NULL;
static sTaskRef gUartRxProcessTask1 = NULL;
static sTaskRef gUart2RxProcessTask = NULL;
static sTaskRef gUart3RxProcessTask = NULL;
static sTaskRef onesectimerProcesser;
static sTaskRef timer1Processer;
static sTaskRef NetlightProcesser;

sMsgQRef simcomUI_msgq;
sMsgQRef uart2_msgq;
int zone[10][10][10];
struct ConfigMaker nConfig[10];
struct Constant nConstant[10];
struct Program nProgram;
struct ProgramProcess nProgramProcess;
//struct zone nZone[4];
struct OMSfeedback s_nOMSfeedback[10];
int NoOfConstant,NoOfObject,NoOfZone,NoOfProgram,ProgramNo;
char ZoneOnFlag=0,ProgramFlag_1=0,ProgramFlag_2=0,ValveOnFlag=0,DelayTime=0,Timer=0,ValvenOnNo=0,ValvenOnRef=0,MainValveFlag=0,MainValveNo=0,ProgramStartFlag_1=0,ProgramStartFlag_2=0;
// #define NETLIGHT SC_MODULE_GPIO_09





// static UINT8 timer1ProcesserStack[10*1024];

static UINT8 timer1ProcesserStack[2 * 1024]; // dg_delete
static UINT8 onesecProcesserStack[1 * 1024];
static UINT8 NetlightProcesserStack[1 * 1024];
static UINT8 gUartRxProcessTask1Stack[5 * 1024] = {0xA5}; //[5 * 1024] = {0xA5};
static UINT8 gUart2RxProcessTaskStack[3 * 1024] = {0xA5};
static UINT8 gUart3RxProcessTaskStack[UART_RX_PROCESS_TASK_STACK_SIZE] = {0xA5};


//void Uart2CBFunc(SC_Uart_Port_Number portNumber, void *para);
void Delete_Settings_files(void);
void SMSMno_Read(void);
void ReadTimerSettings(void);

void WritePhoneNumberFn(void);
void WritectsetFile(void);

void UartCBFunc1(SC_Uart_Port_Number portNumber, void *para);
void SmSCallback(BOOL result);
extern void timerRoutine1(UINT32);
extern void sTask_MainProcesser(void *data);
extern void sAPP_Timer1(void *data);
extern void sTask_CallProcesser(void);

UINT8 IMEI[16] = {0x00};

static sTimerRef timer1;
static sTimerRef timer2;
sTimerRef Tasktimer;


t_rtc timeval;
t_rtc datetime = {0};

UINT32 timerflag1 = 1;

UINT16 OnehrPrvSec = 0, Onehr_sec_timer = 0;
UINT16 avg_adc = 0, avg_adc_total = 0;

// UINT8 Motoronflag=-1;
UINT8 fotafailflag = 0;
UINT8 hash = 0;
UINT8 CGATT = 0;
UINT8 Cpin;
UINT8 CSQ;
UINT8 onehour_time = 0;
UINT8 f_Pump_Settings_view = 0;
UINT8 onehour_send_flag = 0;
UINT8 vbatflag = 0;
UINT8 NTPokFlag = 0;
UINT8 Queue_count=0;


eat_bool GSMInitDone = EAT_FALSE;
eat_bool GSMInitStatus = EAT_FALSE;
BOOL View_Settings_flag = 0, date_time_send_flag = 0, IMEI_req_flag = 0, View_Send_Flag = 0, No_comm_flag = 0;
BOOL Flow_reset = 0, Time_flag = 0, Time_flag1 = 0, Time_flag2 = 0, controlon_flag = 0;
BOOL pressure_calib_flag = 0;
BOOL creg_reg_flag = 0, creg_reg_flag_1 = 0, uart_send_flag = 0, first_time_data_req_flag = 0, Resend_IMEI_flag = 0, uart_data_comm_flag = 0;
BOOL refresh_filter_on_flag = 0;
BOOL austat;
BOOL ModemIsReady = 0,g_new_MAC_flag=0;

char uart_send_count = 0, Resend_IMEI_count = 0;
char BATPER_act = 0;
char phase_number2 = 0;
char phase_number = 0;
char phase_number3 = 0;
char View_Settings_count = 0, View_Settings_count_max = 0, l_pumpval = 0;
char l_mtrno = 0;
char prev_onof_M1 = 0, prev_motor1_state = 0;
char sms_onflag = 0, sms_offlag = 0, sms_onflag1 = 0, sms_offlag1 = 0, sms_onflag2 = 0, sms_offlag2 = 0, trip_flag = 0, trip_flag1 = 0, trip_flag2 = 0;
char g_no_of_pumps = 0, temp_count = 0, l_pumpno, prev_number_enu_data[3], Prev_M1_state = 0, Prev_autokey = 0, g_serialid = 0,ValveChange=0;
char checkpower_act = 0;
char regsmsno;
char pumpno;

unsigned char payload_buf[300]={0},Buff[100];
unsigned char BufCount=0,RecCheck[4],RecNoOfPayload=0,NoOfPayload=0;
unsigned char Node_buf[300],Rec_buf[32];
unsigned char Rec_len1=0,Rec_len2=0,Rec_Key=0;
struct ValveOnOff
{
	unsigned char ValveStausFlag,ValveNo,MainValveStatus,ControlFLag,SolarVolt;
	float Pressure1,Pressure2,Pressure3,Flow,BatVolt;
	int ADC1,ADC2,ADC3;
	uint32_t Cummulative;
};
struct ValveOnOff SendValveOnOff,RecValveOnOff;

_Bool NewDataFlag=0;
 int Oalen=0,CRC_Total=0,CRC_val=0;
int VBAT = 0;
int sstrength;
int NumberOfSmsNeedToSend = 0, Nooftcpsendcount = 0;
int module, urc;
int TripVoltage = 0;
int Prvsecvar = 0;
int livesendcount = 0;
int CallModeOn = 0;
int ActtimerPrvSec = 0;
// int YVoltage1=0;
int RVoltage1 = 0, YVoltage1 = 0, BVoltage1 = 0;
int ActMotorStarterTripTimer = 0;
int NumberSplitAct;
int Creg = 0;
int CGREG = 0;
int tout = 0;
int PlaySound = 0;
int PlaySoundVer = 0;
int PlayNumber = 0;

int HowManySoundToPlay = 0;
int NoAcceptSMS = 0;
int ServiceNumberFound = 0;
int VSPPImbalanceVoltage = 0;
int CSPPValue = 0;
int revalue,reTpHr,reTpMin,reTpSec;



unsigned int toutt = 0;
unsigned int bat_volt_int = 0;
unsigned int WorkingOn3Phas = 0;
// UINT8 Creg=0,CGREG=0,CGATT=0,CSQ=0,tout=0;

unsigned char getPsflag=0;
unsigned char OverAllStarterTrip = 0, OverAllStarterTrip1 = 0, OverAllStarterTrip2 = 0, OverAllStarterTrip3 = 0;
unsigned char StaticSMSSend = 0;
unsigned char StaticSMSstrSend = 0;
unsigned char StaticSMSDelay = 0;
unsigned char Mobile2Phs3Phase = 0;
unsigned char WhichPhaseHaveingProblem = 0;
unsigned char sound_flag_ok = 0;
unsigned char creg_count = 0, creg_reset_flag;
unsigned char CallConnected = 0, ringflag = 0;
unsigned char TargetNumberFound = 0;
unsigned char ActpowerOnDelayPrvSec;
unsigned char MakeRealyOn = 0;
unsigned char ftpgettofs_cb_flag = 0;
unsigned char WaitOK;
unsigned char NumberFound = 0;
unsigned char Nooftcprecvd = 0, Nooftcpprocessed = 0, prvNooftcpprocessed = 0, Nooftcprecvd1 = 0, Nooftcpprocessed1 = 0;
unsigned char Noofsettingsrecvd = 0, Noofsettingsprocessed = 0;
unsigned char wifiopenflag = 0, errorcount = 0, tcpopenflag = 0, tcpcopyflag = 0, rebootflag = 0, rebootflag1 = 0, rebootflagcounter = 0, tcpdcounter = 0, tcpdcounter3 = 0, tcpdcounter2 = 0, tcpdcounterflag = 0, tcpdcounter1 = 0, gettcpcounter = 0, sgetflag = 0, sgetflag_1 = 0, clostcpcounter = 0, wifimodesetfalg = 0, wifiresetflag = 0, wifiresetflag1 = 0, wifiresetcounter = 0;
unsigned char limitsmscountsmsm = 0;
unsigned char BalanceSend = 0;
unsigned char pumpno_tx = 1;
unsigned char Change_Date_Flag = 0, day_send_flag = 0, Date_Change_Flag = 0, pumpno_G = 1, No_comm_secs = 0;
unsigned char IamCalling = 0;
unsigned char GiveCallToNumber = 0;
unsigned char CallInit = 0;
unsigned char CallBackWaitOK = 0;
unsigned char PowerCondCall = 0;
unsigned char PowerCondVoice = 0;
unsigned char sendtoMcuack = 0, sendtoMcu = 0;
unsigned char pumpno_f, level_percent;
unsigned char Time_Hr = 0, Time_Min = 0, Time_Sec = 0;
unsigned char Time_Hr1 = 0, Time_Min1 = 0, Time_Sec1 = 0;
unsigned char Time_Hr2 = 0, Time_Min2 = 0, Time_Sec2 = 0;
unsigned int Time_var=0,Time_var1=0,Time_var2=0;
unsigned char on_data_M1 = 0, on_data_M2 = 0, on_data_M3 = 0, off_data_M1 = 0, off_data_M2 = 0, off_data_M3 = 0;
unsigned char DebounceDelaycounter, dtmfDebounceDelaycounter;
unsigned char getDelaycounter;
unsigned char DebounceDelay, dtmfDebounceDelay;
unsigned char SendSMS, SendWIFI;
unsigned char Callreceiv = 0;
unsigned char NumberChangeSMS = 0;
unsigned char RegxSmsSend = 0;
unsigned char SendSMSByAt = 0;
unsigned char NumberSplit = 0;
unsigned char enter1 = 0;
unsigned char MassageReceived = 0;
unsigned char limitsmsonof;
unsigned char limitsmscount = 0;
unsigned char limitsmsset;
unsigned char limitsmsflag = 0;
unsigned char valve_onof_msg = 0;
unsigned char livedataflag = 0, livedataflag1 = 0, livedataflagcount1 = 0, livedataflagcount = 0, Appmodeon = 0, wifion = 1, gprson = 0, gprsgeton = 0, apmodeon = 0, progqueonof = 0, PrvAutoMobileKey = 0, getdataflag = 0, fotaflsg = 0, senddataflag = 0, setserialflag = 0, getdataflag1 = 0, wpsmodeon = 0, setserialflagcount = 0, getdataflagcount = 0, setserialflagcount1 = 0, setserialflag_1 = 0, setserial1_val, getdataflag_1 = 0;
unsigned char SendSmsToAll = 0;
unsigned char SendSmsflag = 0;
unsigned char HowManyNumberFound;
unsigned char PowerCurrentCondition = 0;
unsigned char checkpower, level_percent = 0, prevlevel_percent = 0;
unsigned char CallOnOfVer;
unsigned char Solar[2],LightFlag=0,RecLightFlag=0,LightStandaloneFlag=0,AutoRstFlag=0;
unsigned char LiveValveTimerHr[10]={0},LiveValveTimerMin[10]={0},LiveCycLicIntervelHr=0,LiveCycLicIntervelMin=0,RemainingOnHr=0,RemainingOnMin=0,RemainingOnSec=0,ValveOnSetting=0,CyclicIntevelFlag=0,CyclicIntevelHr=0,CyclicIntevelMin=0,RecCyclicIntevelHr=0,RecCyclicIntevelMin=0,RecCyclicIntevelsec=0,RecCycFlag=0;
unsigned char ValveStatus[10]={0},PrvValveStatus[10]={0};
unsigned char LogValveStatus,LogValveNo,ValveOnOffReason,Prvalve[10]={0},ValveOnTime[2],ValveOffTime[2],ValveOnDueTime[2],ReasonFlag=0,CyclicComplete=0,LiveUpdate=0,RecLiveUpdate=0,ValveSetting=0;
;


char Motoronflag[3] = {0}, prev_Motoronflag[3] = {0}, Motorreasonflag[3] = {0}, act_POnMin[3] = {0}, act_POnSec[3] = {0}, act_rem_delmin[3] = {0}, act_rem_delsec[3] = {0}, act_del_comp_min[3] = {0}, act_del_comp_sec[3] = {0}, prev_Motorreasonflag[3] = {0};
char act_rem_cyc_ofHr[3] = {0}, act_rem_cyc_ofMin[3] = {0}, act_rem_cyc_ofSec[3] = {0}, act_rem_cyc_onHr[3] = {0}, act_rem_cyc_onMin[3] = {0}, act_rem_cyc_onSec[3] = {0};
char act_rem_maxHr[3] = {0}, act_rem_maxMin[3] = {0}, act_rem_maxSec[3] = {0};


char BalanceNumber[20] = "0000000000000";
//char BalanceStr[250];
char actual_float_stat[3][4] = {0};
char StaticSmsString[200] = "\n\r";
char pTemBuffer[100];
char adc[10] = "";

char Balanceflag = 0;


unsigned int CallErrorDelay = 0;
unsigned int CallingBecError = 0;
unsigned int GCallingBecError = 0;
unsigned int enter2 = 0;

unsigned int Cyclic_On_Completed[3] = {0};
unsigned int Maxrun_On_Completed[3] = {0};
unsigned int RTC_On_Completed[3] = {0};


static sFlagRef g_flg1 = NULL;
static sFlagRef g_flg2 = NULL;

#define TIMER1_OUT_EVENT_MASK (0x1 << 1)

UINT8 DTMF_buf[15];
UINT8 CallUrcbuf[10];

volatile unsigned char cpbrsearchend;
volatile unsigned char cpbrsearchend1;
volatile unsigned char cpbrsearchend2;

struct dirent *info_dir = NULL;
SCDIR *dir_hdl = NULL;


INT8 buf[1050] = ""; // 500
INT8 bbuf[100] = "";
INT8 buffer[200] = "TEXT";
int HowManySound[40];

//char UartBuffer[500] = "";
char BigSMS[1100] = "TEXT"; // 3000SmsTCPStrNumber[].SmsTCPstr //1000   //1200
char BigSMS1[10] = "TEXT";
char BigSMS2[100] = "TEXT";
char Buffer1[500] = "TEXT";
char Buffer2[500] = "TEXT";
char Buffer3[500] = "TEXT";
char Buffer4[500] = "";
char Buffer5[500] = "";
char Buffer6[500] = "";
char *textBuf[800];
char strBuf[1200] = "TEXT";
char SendSMSString[350] = "TEXT";
char SendSMSOnThisNumber[20] = "8806668379";
char Recive_Sms_Number[20] = "8806668379";
char Recive_call_Number[20] = "8806668379";
char WhoMadeRelayOn[20] = "\n";
char WhoMadeenterRelayOn[20] = "\n";
char TargetNumber[15] = "TEXT";
char ModemStr[50] = "TEXT";
char PhoneNumber[20] = "+918110054669";	 //"+918806668379"
char PhoneNumber1[30] = "+918110054669"; //"+918806668379";
char ServiceNumber[20] = "0000000000";
char Status30MinPhNo[20] = "0000000000";
char number_enu_data[3] = {0};
char uart_read_buf[200];
char uart_st_buf[200];
char Sim_Buf[200];
char TCPWifigprsstrBUFF[1350];

unsigned char my_eeprom_ID[6];
unsigned char DNDSMSFLAG[5];
unsigned char a_Master_onoff[3] = {0};


volatile unsigned char NumSMS[5] = {0};
unsigned int act_del_completed[3] = {0};


sMsgQRef urc_mqtt_msgq_1;
sMsgQRef g_AT_urc_msgQ;
sMsgQRef g_call_demo_msgQ;
sMsgQRef g_AT_urc_msgQ;
sTaskRef simcomUIProcesser;
sTaskRef taskRef;
// extern void sAPI_UartPrintf(INT8 *format);

/*main&timer thread dec*/


char tripflag[3] = {0};
char SendSMSByAtPh[20] = "TEXT";
char a_occurance_count[3];
char pfile[20] = "st.txt";
char SmsSplit[10][200] = {"nimsih", "TEXT"};
char StoredPhoneNumber[25][15] = {"TEXT", "TEXT"};
char StoredPhoneSmscode[16][15] = {"TEXT", "TEXT"};
char SmsNumber[10][15] = {"TEXT", "TEXT"};
char StrTokStr[40][20];
char StrTokStr1[50][20]; // [100][80]; dg_nsdk //[260][80]; // 150,120 // dg_changed from 120 80
// char StrTokStr1[120][80]; // 150,120
char StrTokStr2[170][10]; // [170][50] dg_nsdk // 160 , 100

char *Pch1 = NULL;



unsigned int RST_count =0;
long RunTimer = 0;
long MRunTimer = 0;
long Runflow = 0, Runflow1 = 0, Runflow2 = 0, Runflow3 = 0, g_Runflow_day = 0;
long MRunflow = 0;
long LastDayRunflow = 0;
long LastDayRunTimer = 0;
long RunTimerhr;
long RunTimermin;
long RunTimersec;
long CurrentSec = 0, cumulative_flow = 0, cumulative_flow1[3] = {0};
long int SIM_NO = 0;

unsigned long creg_reg_count = 0;
volatile long int No_comm_prev_secs = 0;
const long int data_sec = 86390;
volatile long int seconds_data = 0;
volatile long int l_currentSec = 0, Time_Counter = 0, Time_Counter1 = 0, Time_Counter2 = 0, prev_sec = 0;
INT16 Ten_mins_sec_timer;

float BATPER = 0.0;
float actual_value2 = 0.0, set_value2 = 0.0, cumulative_flow2 = 0.0;
float actual_value3 = 0.0, set_value3 = 0.0, cumulative_flow3 = 0.0;
float actual_value = 0.0, set_value = 0.0;
float Liter_Per_Pulse = 0;
float flowisthere[3] = {0}, flow_rate;
float SignalStrength = 0;
float Act_pressure1 = 0.0;
float Act_pressure[3] = {0.0};
float Act_level1 = 0.0, Act_level[3] = {0.0};
float Act_lps = 0.0, Act_lps1[3] = {0.0};
float TripCurrent = 0.0;
float Trippressure = 0.0;
float Battery[2],PressureSensor;
float ActPress=0.0,SetPress=0.0;


unsigned char NoOfValveConfig=0,NoOfMosConfig=0,SerialNo[2],NodeType[2],RefNo[2],DeviceId[2][12],InterfaceType[2],NoOfValveConfigRec=0,CyclicLimitRec=0,CycleNo=0,ValUp[12]={0},CycOnOf=0,NoSoilTemp=0;
unsigned char SetSerial[2],RecSetSerial[2],MosConOnOff=0,MosSelection=0,PrimMin1=0,PrimMin2=0,PrimMax1=0,PrimMax2=0,LightConfig=0,PresSenConfig=0,PreSwConfig=0;
float Mos[4],RecMos[4],SoilTemp=0,HighFlow=0,LowFlow=0;
unsigned int frequency=0;
unsigned char PresSenFlag=0,PreSwFlag=0,PreSenTime[3],PreSwTime[3];








// BOOL MqttInitStatus = 0;


// UINT8 ret;
// timer tday[7]={0};

void PrintfResp(char *format)
{
//#ifdef SIMCOM_UI_DEMO_TO_UART1_PORT
  //  #ifdef HAS_UART
        #if (defined SIMCOM_A7680C_V5_01) || (defined SIMCOM_A7670C_V701) || (defined SIMCOM_A7670C_V702) || defined (SIMCOM_A7680C_V506) || defined (SIMCOM_A7680C_V603) || defined (SIMCOM_A7605C1_V201) || defined(SIMCOM_A7673G_V201) || defined (SIMCOM_A7680C_V801)
            sAPI_UartWrite(SC_UART4, (UINT8 *)format, strlen(format));
        #else
            sAPI_UartWrite(SC_UART, (UINT8 *)format, strlen(format));
        #endif
   // #endif
/*#else
    #ifdef HAS_USB
        sAPI_UsbVcomWrite((UINT8 *)format, strlen(format));
    #endif
#endif*/
}

uint8_t crc8(const void *data, size_t size)
{
	uint8_t crc = CRC8_INIT;
	const uint8_t *ptr = (const uint8_t *)data;
	uint8_t j;
	size_t i;
	// Debug("size:%d",size);
	for (i = 0; i < size; i++)
	{
		// Debug("DATA:ox%X",ptr[i]);
		crc ^= ptr[i];
		for (j = 0; j < 8; j++)
		{
			if (crc & 0x80)
				crc = (crc << 1) ^ CRC8_POLY;
			else
				crc <<= 1;
		}
	}

	return crc;
}

struct TimerSettings1{
	int m_LowVoltII;
	int m_LowVoltIII;
	int m_HighVoltIII;
	int m_HighVoltII;
	int m_HiDiffVoltIII;  
	int m_HiDiffVoltII;
	int m_DiffVoltIII,m_DiffVoltII,m_ImbVolt;
	float m_OlAmpsII[3],m_OlAmpsIII[3];
	float m_DrAmpsIII[3];
	float m_DrAmpsII[3];
	int m_OlRstVol;
	char m_ct_MotorNumber[3];
	unsigned char m_AutoRstOn[3],m_lowpress[3],m_highpress[3];
	char m_OlScanHr[3], m_OlScanMin[3], m_OlScanSec[3];
	char m_DrScHr[3],m_DrScMin[3], m_DrScSec[3];
	char m_POnHr[3],m_POnMin[3], m_POnSec[3];
	char m_MaxRnHr[3],m_MaxRnMin[3],m_MaxRnSec[3],m_AutoRst2On;
	char m_SDHr[3],m_SDMin[3], m_SDSec[3];
	char m_SfbHr[3],m_SfbMin[3],m_SfbSec[3];
	char m_DrReHr[3],m_DrReMin[3],m_DrReSec[3];
	char m_RTCONHr[3][7],m_RTCONMin[3][7],m_RTCONSec[3][7]; // check subash
	char m_RTCOfHr[3][7],m_RTCOfMin[3][7],m_RTCOfSec[3][7];
	char m_ScrDlHr[3], m_ScrDlMin[3], m_ScrDlSec[3], m_PoScrDlHr[3],m_PoScrDlMin[3],m_PoScrDlSec[3];
	char m_DrOccurTimHr[3], m_DrOccurTimMin[3], m_DrOccurTimSec[3];
	char m_RTCON[3][6];
	char m_RTCOf[3][6];
	char m_CycLicOnHr[3],m_CycLicOnMin[3],m_CycLicOnSec[3];
	char m_CycLicOfHr[3],m_CycLicOfMin[3],m_CycLicOfSec[3];
	char m_Enter;
};

struct logtime
{
	unsigned char Act_Mtr1_On[25];
	unsigned char Act_Mtr1_Off[25];
	unsigned char Act_Mtr2_On[25];
	unsigned char Act_Mtr2_Off[25];
	unsigned char Act_Mtr3_On[25];
	unsigned char Act_Mtr3_Off[25];
};

struct pump
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
	char m_scheduleonof;
	char m_rundays;
	char m_skipdays;
	char m_Uppertank_restart_off;
	char m_Lowertank_restart_off;
};

struct tank
{

	char m_min_level_p;
	char m_max_level_p;
	char m_Level_Sensor_height;
	char m_Uppertank;
	char m_Lowertank;
	char m_Press_Max_Value;
	float m_Level_calfactor;
	double m_tank_height;
};
struct mqtt
{
	unsigned char Subscribe_Topic[25];
	unsigned char Publish_Topic[25];
	unsigned char Server_Publish_Topic[25];
};

struct __SMSstrnumber
{
	char Smsstr[500]; // 225
	char Smsnumber[15];
	// char Wifistr[220];
} SmsStrNumber[25];
struct __SMSTCPstrnumber
{
	char SmsTCPstr[10];
	// char Smsnumber[15];
	// char Wifistr[220];
} SmsTCPStrNumber[25];

struct dripcom zonecom;
struct MoTr nMoTr;
struct VaTr nVaTr;
struct TimerSettings1 s_nTimerSettings;
// struct CurretnCond s_nCurretnCond;
struct settings s_nMSettings;
struct pump s_npump[3];
struct logtime s_nlogtime[25];

struct tank s_ntank;
struct mqtt s_mqtt;

struct ntimer ttday[1];
struct MSettings nMSettings;
struct CurrentCond nCurretnCond;
struct __WIFIstrnumber wifiStrNumber[40];		// 60
struct __TCPWIFIstrnumber TCPwifiStrNumber[50]; // 60
struct __TCPWIFIstrnumber1 TCPwifiStrNumber1[10];
struct TimerSettings nTimerSettings;

char pumpsettingbuf[300]={0};

typedef enum // DG_ORO
{
	NO_TCOND,
	ON_DELAY,
	OF_DELAY,
} CYCTimer;

typedef enum // DG_ORO
{
	NO_RTC_TIMER,
	GRTC_TIMER_ON,
	RTC1_TIMER_ON,
	RTC2_TIMER_ON,
	RTC3_TIMER_ON,
	RTC4_TIMER_ON,

} RTC_CURRENT_STATUS;

typedef enum // DG_ORO
{
	NO_COND,
	NO_LOAD,
	OV_LOAD,
	OVPRESS_LOAD,
	LOPRESS_LOAD,
} nDISPLAYMsg;

typedef enum // NOT USED
{
	STATUS_NLIGHT_RTCON,
	STATUS_NLIGHT_RTCOF,
} STATE_NLIGHTSTATUS_SMS;

typedef enum // NOT USED
{
	STATUS_LIGHT_RTCON,
	STATUS_LIGHT_RTCOF,
	STATUS_LIGHT_CYCON,
	STATUS_LIGHT_CYCOF,

} STATE_LIGHTSTATUS_SMS;
STATE_LIGHTSTATUS_SMS nSTATE_LIGHTSTATUS_SMS = STATUS_LIGHT_RTCOF;
STATE_NLIGHTSTATUS_SMS nSTATE_NLIGHTSTATUS_SMS = STATUS_NLIGHT_RTCOF;
typedef enum // NOT USED
{
	STATUS_FAN_RTCON,
	STATUS_FAN_RTCOF,
	STATUS_FAN_CYCON,
	STATUS_FAN_CYCOF,

} STATE_FANSTATUS_SMS;
STATE_FANSTATUS_SMS nSTATE_FANSTATUS_SMS = STATUS_FAN_RTCOF;

typedef enum // NOT USED
{
	STATE_FOGVALVEVIEW_SMS,
	STATE_FOGCYCLE_COMPLETED_SMS,
	STATE_FIRST_FOGVALVEVIEW_SMS,
	STATUS_FOG_RTCOF,
	STATUS_FOG_RTCON,
	STATUS_FOG_CYCON,
	STATUS_FOG_RST,
} STATE_FOGSTATUS_SMS;

STATE_FOGSTATUS_SMS nSTATE_FOGSTATUS_SMS = STATUS_FOG_RTCOF;
typedef enum // oro_newlogic
{
	STATUS_MOTOROF_SMS,
	STATUS_NO_ELECTRICICY,
	STATUS_ON_RESTART_NOLOAD, // dg_add_oro
	STATUS_MOTOR_STARTER_TRIP_SMS,
	STATUS_MOTOR_UPPERTANK_TRIP_SMS,
	STATUS_MOTOR_LOWERTANK_TRIP_SMS,
	STATUS_MOTOR_DRYRUN_TRIP_SMS,
	STATUS_MOTOR_TRIP_OVERLOAD,
	STATUS_MOTOR_TRIP_HIGHPRESS,
	STATUS_MOTOR_TRIP_LOWPRESS,
	STATUS_MOTOR_TRIP_SPP,
	STATUS_MOTOR_TRIP_REVERSEPHASE,
	STATUS_MOTOR_TRIP_2PHASE,
	STATUS_MOTOR_TRIP_3PHASE,
	STATUS_MOTOR_TRIP_CURRENTSPP,
	STATUS_MOTOR_TRIP_LOWVOLTAGE,
	STATUS_MOTOR_TRIP_HIGHVOLTAGE,
	STATUS_MOTOR_GRTCOF_TRIP_SMS,
	STATUS_MOTOR_RTCOF1_TRIP_SMS,
	STATUS_MOTOR_RTCOF2_TRIP_SMS,
	STATUS_MOTOR_RTCOF3_TRIP_SMS,
	STATUS_MOTOR_RTCOF4_TRIP_SMS,
	STATUS_MOTOR_CYCLIC_TRIP_SMS,
	STATUS_MOTOR_MAX_TRIP_SMS,
	STATUS_MOTOR_VCYCCOMPLE_TRIP_SMS,
	STATUS_MOTOR_NOCOMMUNICATION_TRIP_SMS,
	STATUS_MOTOR_ON_DEFAULT,
	STATUS_MOTOR_ON_ONDELAY,
	STATUS_MOTOR_ON_RTC1,
	STATUS_MOTOR_ON_GRTC,
	STATUS_MOTOR_ON_RTC2,
	STATUS_MOTOR_ON_RTC3,
	STATUS_MOTOR_ON_RTC4,
	STATUS_MOTOR_ON_UPPERTANK,
	STATUS_MOTOR_ON_LOWERTANK,
	STATUS_MOTOR_ON_RESTARTTIMER,	 // not used
	STATUS_MOTOR_ON_SWITCH,			 // dg_oro missing
	STATE_STGIII_SMS,				 // dg_oro missing
	STATE_STGII_SMS,				 // dg_oro missing
	STATUS_MOTOR_ON_SWITCH_TRIP_SMS, // dg_oro missing
	STATUS_MOTOR_ON_TARGET,			 // dg_oro missing
	STATUS_MOTOR_OFF_TARGET,		 // dg_oro missing
} STATE_STATUS_SMS;

typedef enum // oro_newlogic
{
	STATUS_MOTOR_AT_OFF, // decide
	STATUS_MOTOR_AT_POWERONDELAY,
	STATUS_MOTOR_AT_STARDELTA,
	STATUS_MOTOR_ON_PROPERLY,

} STATE_STATUS_ONDELAY_SD;

typedef enum
{
	STATE_NO_MOTOR1_SMS,
	STATE_MOTOR1ON_SMS,
	STATE_MOTOR1OF_SMS,
	STATE_MOTOR1_STARTER_TRIP_SMS,
	STATE_MOTOR1_UPPERTANK_TRIP_SMS,
	STATE_MOTOR1_LOWERTANK_TRIP_SMS,
	STATE_MOTOR1_TRIP_OVERLOAD_SMS,
	STATE_MOTOR1_DRYRUN_TRIP_SMS,
	STATE_MOTOR1_TRIP_HIGHPRESS_SMS,
	STATE_MOTOR1_TRIP_LOWPRESS_SMS,
	STATE_MOTOR1_TRIP_SPP_SMS,
	STATE_MOTOR1_TRIP_REVERSEPHASE_SMS,
	STATE_MOTOR1_TRIP_2PHASE_SMS,
	STATE_MOTOR1_TRIP_3PHASE_SMS,
	STATE_MOTOR1_TRIP_CURRENTSPP_SMS,
	STATE_MOTOR1_TRIP_LOWVOLTAGE_SMS,
	STATE_MOTOR1_TRIP_HIGHVOLTAGE_SMS,
	STATE_MOTOR1_RTCOF1_TRIP_SMS,
	STATE_MOTOR1_RTCOF2_TRIP_SMS,
	STATE_MOTOR1_RTCOF3_TRIP_SMS,
	STATE_MOTOR1_RTCOF4_TRIP_SMS,
	STATE_MOTOR1_RTCOF5_TRIP_SMS,
	STATE_MOTOR1_RTCOF6_TRIP_SMS,
	STATE_MOTOR1_CYCLIC_TRIP_SMS,
	STATE_MOTOR1_MAX_TRIP_SMS,
	STATE_MOTOR1_OFF_TARGET,
	STATE_MOTOR1OF_3PHASE_ONLY_SMS,
	STATE_MOTOR1_ON_SWITCH_TRIP_SMS,
	STATE_MOTOR1_LEVELSCAN_UPPERTANK_TRIP_SMS,
	STATE_MOTOR1_LEVELSCAN_LOWERTANK_TRIP_SMS,
	STATE_NLIGHT_RTCOF_SMS,
	STATE_CYCINTERVEL_OF_SMS,
	STATE_CYCLELIMTE_COMPLETED_SMS,
	STATE_MOISTURE_OF_SMS,
	STATE_PAUSE_OF_SMS,
	STATE_WRONG_FEEDBACK_OF_SMS,
	STATE_NO_COMMUNICATION_OF_SMS,
	STATE_PRESSURE_LOW_SMS,
	STATE_PRESSURE_HIGH_SMS,
	STATE_PRESSURE_SWITCH_OF_SMS,
	
//	STATE_STGII_SMS,
//	STATE_STGIII_SMS,

} STATE_MOTOR1_SMS;

typedef enum
{
	STATE_NO_MOTOR2_SMS,
	STATE_MOTOR2ON_SMS,
	STATE_MOTOR2OF_SMS,
	STATE_MOTOR2_STARTER_TRIP_SMS,
	STATE_MOTOR2_UPPERTANK_TRIP_SMS,
	STATE_MOTOR2_LOWERTANK_TRIP_SMS,
	STATE_MOTOR2_TRIP_OVERLOAD_SMS,
	STATE_MOTOR2_DRYRUN_TRIP_SMS,
	STATE_MOTOR2_TRIP_HIGHPRESS_SMS,
	STATE_MOTOR2_TRIP_LOWPRESS_SMS,
	STATE_MOTOR2_TRIP_SPP_SMS,
	STATE_MOTOR2_TRIP_REVERSEPHASE_SMS,
	STATE_MOTOR2_TRIP_2PHASE_SMS,
	STATE_MOTOR2_TRIP_3PHASE_SMS,
	STATE_MOTOR2_TRIP_CURRENTSPP_SMS,
	STATE_MOTOR2_TRIP_LOWVOLTAGE_SMS,
	STATE_MOTOR2_TRIP_HIGHVOLTAGE_SMS,
	STATE_MOTOR2_RTCOF1_TRIP_SMS,
	STATE_MOTOR2_RTCOF2_TRIP_SMS,
	STATE_MOTOR2_RTCOF3_TRIP_SMS,
	STATE_MOTOR2_RTCOF4_TRIP_SMS,
	STATE_MOTOR2_RTCOF5_TRIP_SMS,
	STATE_MOTOR2_RTCOF6_TRIP_SMS,
	STATE_MOTOR2_CYCLIC_TRIP_SMS,
	STATE_MOTOR2_MAX_TRIP_SMS,
	STATE_MOTOR2_OFF_TARGET,
	STATE_MOTOR2OF_3PHASE_ONLY_SMS,
	STATE_MOTOR2_ON_SWITCH_TRIP_SMS,
	STATE_MOTOR2_LEVELSCAN_UPPERTANK_TRIP_SMS,
	STATE_MOTOR2_LEVELSCAN_LOWERTANK_TRIP_SMS,
	//	STATE_STGII_SMS,
	//	STATE_STGIII_SMS,

} STATE_MOTOR2_SMS;

typedef enum
{
	STATE_NO_MOTOR3_SMS,
	STATE_MOTOR3ON_SMS,
	STATE_MOTOR3OF_SMS,
	STATE_MOTOR3_STARTER_TRIP_SMS,
	STATE_MOTOR3_UPPERTANK_TRIP_SMS,
	STATE_MOTOR3_LOWERTANK_TRIP_SMS,
	STATE_MOTOR3_TRIP_OVERLOAD_SMS,
	STATE_MOTOR3_DRYRUN_TRIP_SMS,
	STATE_MOTOR3_TRIP_HIGHPRESS_SMS,
	STATE_MOTOR3_TRIP_LOWPRESS_SMS,
	STATE_MOTOR3_TRIP_SPP_SMS,
	STATE_MOTOR3_TRIP_REVERSEPHASE_SMS,
	STATE_MOTOR3_TRIP_2PHASE_SMS,
	STATE_MOTOR3_TRIP_3PHASE_SMS,
	STATE_MOTOR3_TRIP_CURRENTSPP_SMS,
	STATE_MOTOR3_TRIP_LOWVOLTAGE_SMS,
	STATE_MOTOR3_TRIP_HIGHVOLTAGE_SMS,
	STATE_MOTOR3_RTCOF1_TRIP_SMS,
	STATE_MOTOR3_RTCOF2_TRIP_SMS,
	STATE_MOTOR3_RTCOF3_TRIP_SMS,
	STATE_MOTOR3_RTCOF4_TRIP_SMS,
	STATE_MOTOR3_RTCOF5_TRIP_SMS,
	STATE_MOTOR3_RTCOF6_TRIP_SMS,
	STATE_MOTOR3_CYCLIC_TRIP_SMS,
	STATE_MOTOR3_MAX_TRIP_SMS,
	STATE_MOTOR3_OFF_TARGET,
	STATE_MOTOR3OF_3PHASE_ONLY_SMS,
	STATE_MOTOR3_ON_SWITCH_TRIP_SMS,
	STATE_MOTOR3_LEVELSCAN_UPPERTANK_TRIP_SMS,
	STATE_MOTOR3_LEVELSCAN_LOWERTANK_TRIP_SMS,
	//	STATE_MOTOR3_TRIP_LOWVOLTAGE_SMS,
	//	STATE_STGII_SMS,
	//	STATE_STGIII_SMS,

} STATE_MOTOR3_SMS;

typedef enum // oro_newlogic
{
	STATE_MOTOR_POWERONDELAY,
	STATE_MOTOR_STARDELTADELAY,
	STATE_MOTOR_ON,
	STATE_MOTOR_TRIP_STARTER,
	STATE_MOTOR_TRIP_DRYRUN_SCAN,
	STATE_MOTOR_TRIP_DRYRUN,
	STATE_MOTOR_TRIP_HIGHPRESS,
	STATE_MOTOR_TRIP_LOWPRESS,
	STATE_MOTOR_TRIP_OVERLOAD_SCAN,
	STATE_MOTOR_TRIP_HIGHPRESS_SCAN,
	STATE_MOTOR_TRIP_LOWPRESS_SCAN,
	STATE_MOTOR_TRIP_OVERLOAD,
	STATE_MOTOR_TRIP_SPP,
	STATE_MOTOR_TRIP_REVERSEPHASE,
	STATE_MOTOR_TRIP_2PHASE,
	STATE_MOTOR_TRIP_3PHASE,
	STATE_MOTOR_TRIP_CURRENTSPP,
	STATE_MOTOR_TRIP_LOWVOLTAGE,
	STATE_MOTOR_TRIP_HIGHVOLTAGE,
	STATE_MOTOR_TRIP_UPPERTANK,
	STATE_MOTOR_TRIP_LOWERTANK,
	STATE_MOTOR_TRIP_TARGET,
	STATE_MOTOR_TRIP_RESTART,
	STATE_MOTOR_TRIP_MAXTIME,
	STATE_MOTOR_TRIP_VCYCCOMPLETE,
	STATE_MOTOR_TRIP_OFFDELAY,
	STATE_MOTOR_TRIP_GRTCOF,
	STATE_MOTOR_TRIP_RTCOF1,
	STATE_MOTOR_TRIP_RTCOF2,
	STATE_MOTOR_TRIP_RTCOF3,
	STATE_MOTOR_TRIP_RTCOF4,
	STATE_MOTOR_OFF,
	STATE_MOTOR_ON_SWITCH_TRIP_SMS,
	STATE_MOTOR_TRIP_NOCOMMUNICATION,
} STATE_MOTOR;

typedef enum // oro_newlogic
{
	STATE_NO_MOTOR_SMS,
	STATE_MOTORON_SMS,
	STATE_MOTOROF_SMS,
	STATE_MOTOR_STARTER_TRIP_SMS,
	STATE_MOTOR_UPPERTANK_TRIP_SMS,
	STATE_MOTOR_LOWERTANK_TRIP_SMS,
	STATE_MOTOR_TRIP_OVERLOAD_SMS,
	STATE_MOTOR_TRIP_HIGHPRESS_SMS,
	STATE_MOTOR_TRIP_LOWPRESS_SMS,
	STATE_MOTOR_TRIP_SPP_SMS,
	STATE_MOTOR_TRIP_REVERSEPHASE_SMS,
	STATE_MOTOR_TRIP_2PHASE_SMS,
	STATE_MOTOR_TRIP_3PHASE_SMS,
	STATE_MOTOR_TRIP_CURRENTSPP_SMS,
	STATE_MOTOR_TRIP_LOWVOLTAGE_SMS,
	STATE_MOTOR_TRIP_HIGHVOLTAGE_SMS,
	STATE_MOTOR_DRYRUN_TRIP_SMS,
	STATE_MOTOR_GRTCOF_TRIP_SMS,
	STATE_MOTOR_RTCOF1_TRIP_SMS,
	STATE_MOTOR_RTCOF2_TRIP_SMS,
	STATE_MOTOR_RTCOF3_TRIP_SMS,
	STATE_MOTOR_RTCOF4_TRIP_SMS,
	STATE_MOTOR_CYCLIC_TRIP_SMS,
	STATE_MOTOR_MAX_TRIP_SMS,
	STATE_MOTOR_VCYCCOMPLE_TRIP_SMS,
	STATE_MOTOR_OFF_TARGET, // START HERE
	STATE_MOTOR_NOCOMMUNICATION_TRIP_SMS,
	STATE_MOTOROF_3PHASE_ONLY_SMS,
} STATE_MOTOR_SMS;

typedef enum
{ // this enum not at all used
	STATE_MOTOR_CYCLICONDELAY,
	STATE_MOTOR_CYCLICOFFDELAY,
	STATE_RTCON1,
	STATE_RTCOF1,
	STATE_RTCON2,
	STATE_RTCOF2,
	STATE_RTCON3,
	STATE_RTCOF3,
	STATE_RTCON4,
	STATE_RTCOF4,
	STATE_NO_TIMER,
} STATE_MOTOR_TIMER;

typedef enum // oro_newlogic
{
	STATE_MOTOR_OK,
	STATE_MOTOR_STARTER,
	STATE_MOTOR_NOLOAD,
	STATE_MOTOR_UPPERTANK,
	STATE_MOTOR_LOWERTANK,
	STATE_MOTOR_OVERLOAD,
	STATE_MOTOR_HIGHPRESS,
	STATE_MOTOR_LOWPRESS,
	STATE_MOTOR_2PHASE,
	STATE_MOTOR_SPP,
	STATE_MOTOR_CSPP,
	STATE_MOTOR_RSPP, // not used
	STATE_MOTOR_LOWVOLTAGE,
	STATE_MOTOR_HIGHVOLTAGE,
} STATE_MOTOR_PROBLEM;

typedef enum // oro_newlogic
{
	STATE_MOTOR_ON_DEFAULT, // only less time use in oro code
	STATE_MOTOR_ON_ONDELAY,
	STATE_MOTOR_ON_GRTC,
	STATE_MOTOR_ON_RTC1,
	STATE_MOTOR_ON_RTC2,
	STATE_MOTOR_ON_RTC3,
	STATE_MOTOR_ON_RTC4,
	STATE_MOTOR_ON_UPPERTANK,
	STATE_MOTOR_ON_LOWERTANK,
	STATE_MOTOR_ON_RESTARTTIMER, // not used
	STATE_MOTOR_ON_SPP,			 // not used
	STATE_MOTOR_ON_HIGHVOLTAGE,	 // not used
	STATE_MOTOR_ON_LOWVOLTAGE,	 // not used
	STATE_MOTOR_ON_AUTOOLDRRST,	 // not used
	STATE_MOTOR_ON_OLVOLTAGE,
	STATE_MOTOR_ON_SWITCH_SMS,
	STATE_MOTOR_ON_TARGET,
} STATE_MOTOR_ON_SMS;

typedef enum
{
	STATE_MOTOR1_DUMMY, // STATE_MOTOR1_ON_DEFAULT,
	STATE_MOTOR1_ON_ONDELAY,
	STATE_MOTOR1_ON_RTC1,
	STATE_MOTOR1_ON_RTC2,
	STATE_MOTOR1_ON_RTC3,
	STATE_MOTOR1_ON_RTC4,
	STATE_MOTOR1_ON_RTC5,
	STATE_MOTOR1_ON_RTC6, // 7
	STATE_MOTOR1_ON_UPPERTANK,
	STATE_MOTOR1_ON_LOWERTANK,
	STATE_MOTOR1_ON_OLVOLTAGE,
	STATE_MOTOR1_ON_SWITCH_SMS,
	STATE_MOTOR1_ON_TARGET, // 12
	STATE_MOTOR1_ON_HIGHVOLTAGE,
	STATE_MOTOR1_ON_LOWVOLTAGE,
	// STATE_MOTOR1_TRIP_HIGHVOLTAGE_SMS,
	STATE_MOTOR1_ON_SPP, // 15
	STATE_MOTOR1_ON_RESTARTTIMER,
	STATE_MOTOR1_ON_LEVELSCAN_UPPERTANK,
	STATE_MOTOR1_ON_LEVELSCAN_LOWERTANK,
	STATE_NLIGHT_RTCON_SMS,
	STATE_MOTOR1_ON_DEFAULT
} STATE_MOTOR1_ON_SMS;
// STATE_MOTOR1_ON_SMS nSTATE_MOTOR1_ON_SMS;

typedef enum
{
	STATE_MOTOR2_DUMMY, // STATE_MOTOR2_ON_DEFAULT,
	STATE_MOTOR2_ON_ONDELAY,
	STATE_MOTOR2_ON_RTC1,
	STATE_MOTOR2_ON_RTC2,
	STATE_MOTOR2_ON_RTC3,
	STATE_MOTOR2_ON_RTC4,
	STATE_MOTOR2_ON_RTC5,
	STATE_MOTOR2_ON_RTC6,
	STATE_MOTOR2_ON_UPPERTANK,
	STATE_MOTOR2_ON_LOWERTANK,
	STATE_MOTOR2_ON_OLVOLTAGE,
	STATE_MOTOR2_ON_SWITCH_SMS,
	STATE_MOTOR2_ON_TARGET,
	STATE_MOTOR2_ON_HIGHVOLTAGE,
	STATE_MOTOR2_ON_LOWVOLTAGE,
	STATE_MOTOR2_ON_SPP,
	STATE_MOTOR2_ON_RESTARTTIMER,
	STATE_MOTOR2_ON_LEVELSCAN_UPPERTANK,
	STATE_MOTOR2_ON_LEVELSCAN_LOWERTANK,
	STATE_MOTOR2_ON_DEFAULT,
} STATE_MOTOR2_ON_SMS;
// STATE_MOTOR2_ON_SMS nSTATE_MOTOR2_ON_SMS;

typedef enum
{
	STATE_MOTOR3_DUMMY, // STATE_MOTOR3_ON_DEFAULT,
	STATE_MOTOR3_ON_ONDELAY,
	STATE_MOTOR3_ON_RTC1,
	STATE_MOTOR3_ON_RTC2,
	STATE_MOTOR3_ON_RTC3,
	STATE_MOTOR3_ON_RTC4,
	STATE_MOTOR3_ON_RTC5,
	STATE_MOTOR3_ON_RTC6,
	STATE_MOTOR3_ON_UPPERTANK,
	STATE_MOTOR3_ON_LOWERTANK,
	STATE_MOTOR3_ON_OLVOLTAGE,
	STATE_MOTOR3_ON_SWITCH_SMS,
	STATE_MOTOR3_ON_TARGET,
	STATE_MOTOR3_ON_HIGHVOLTAGE,
	STATE_MOTOR3_ON_LOWVOLTAGE,
	STATE_MOTOR3_ON_SPP,
	STATE_MOTOR3_ON_RESTARTTIMER,
	STATE_MOTOR3_ON_LEVELSCAN_UPPERTANK,
	STATE_MOTOR3_ON_LEVELSCAN_LOWERTANK,
	STATE_MOTOR3_ON_DEFAULT
} STATE_MOTOR3_ON_SMS;

nSTATE_SENDSMS STATE_SENDSMS = STATE_NO_SMS;
STATE_MOTOR1_SMS nSTATE_MOTOR1_SMS = STATE_NO_MOTOR1_SMS;
STATE_MOTOR2_SMS nSTATE_MOTOR2_SMS = STATE_NO_MOTOR2_SMS;
STATE_MOTOR3_SMS nSTATE_MOTOR3_SMS = STATE_NO_MOTOR3_SMS;
STATE_MOTOR1_ON_SMS nSTATE_MOTOR1_ON_SMS = STATE_MOTOR1_DUMMY;
STATE_MOTOR2_ON_SMS nSTATE_MOTOR2_ON_SMS = STATE_MOTOR2_DUMMY;
STATE_MOTOR3_ON_SMS nSTATE_MOTOR3_ON_SMS = STATE_MOTOR3_DUMMY;

BOOL simcom_sms_msg_send(UINT8 *PhoneNumber, UINT8 *SMSStr, UINT16 msgLen, ResultNotifyCb pResultCb)
{

	unsigned char WaitOKByAt = 0;
	unsigned char DelayToStart = 0;
	int StrLen;
	int Tp, Tp2;
	unsigned int ExitTimer = 0;
	// UINT32 ret;
	char *pData;

	// QlTimer timer2;
	// timer2.timeoutPeriod = Ql_MillisecondToTicks(NUMBER_TIMER);
	// timer2.timerId = Ql_StartTimer(&timer2);

	// str_remov_last((char*)PhoneNumber,10);
	sprintf(buf, "\n\rremov String = %s\n\r", PhoneNumber);
	sAPI_UartPrintf(buf);
	NumberSplit = 0;
	Tp2 = 0;

	StrLen = strlen((char *)SMSStr);

	if (StrLen > 159) // 159
	{
		while (1)
		{
			for (Tp = 0; Tp <= 159; Tp++, Tp2++)
			{
				SmsSplit[NumberSplit][Tp] = SMSStr[Tp2];
				SmsSplit[NumberSplit][Tp + 1] = NULL;
				if (Tp2 >= StrLen)
					goto Exit;
			}

			NumberSplit++;
		}
	}
	else
	{
		strcpy((char *)SmsSplit[0], (char *)SMSStr);
		NumberSplit = 0;
	}
Exit:

	sprintf(buf, "*** Sting Now Splited******\n\r");
	sAPI_UartPrintf(buf);
	sprintf(buf, "*** NumberSplit = %d******\n\r", NumberSplit);
	sAPI_UartPrintf(buf);
	for (Tp = 0; Tp <= NumberSplit; Tp++)
	{
		sprintf(buf, "*** Split String %d= %s******\n\r", Tp, SmsSplit[Tp]);
		sAPI_UartPrintf(buf);
	}

	NumberSplitAct = 0;
	SendSMSByAt = 0;
	sprintf(SendSMSByAtPh, "%s", PhoneNumber);
	sprintf(buf, "*** SendSMSByAtPh= %s******\n\r", SendSMSByAtPh);
	sAPI_UartPrintf(buf);
	for (NumberSplitAct = 0; NumberSplitAct <= NumberSplit; NumberSplitAct++)
	{

		sprintf(Buffer1, "%s", SmsSplit[NumberSplitAct]);
		sprintf(buf, "*** Buffer1= %s******\n\r", Buffer1);
		sAPI_UartPrintf(buf);
		sprintf(SmsStrNumber[NumberOfSmsNeedToSend].Smsstr, "%s", Buffer1);
		sprintf(SmsTCPStrNumber[NumberOfSmsNeedToSend].SmsTCPstr, "");
		sprintf(buf, "*** Smsstr= %s******\n\r", SmsStrNumber[NumberOfSmsNeedToSend].Smsstr);
		sAPI_UartPrintf(buf);
		sprintf(SmsStrNumber[NumberOfSmsNeedToSend].Smsnumber, "%s", SendSMSByAtPh);
		sprintf(buf, "\n\r+++++++++++++++SmsStrNumber[%d].Smsstr = %s++++++++++\n\r", NumberOfSmsNeedToSend, SmsStrNumber[NumberOfSmsNeedToSend].Smsstr);
		sAPI_UartPrintf(buf);
		sprintf(buf, "\n\r+++++++++++++++SmsStrNumber[%d].Smsnumber = %s++++++++++\n\r", NumberOfSmsNeedToSend, SmsStrNumber[NumberOfSmsNeedToSend].Smsnumber);
		sAPI_UartPrintf(buf);
		NumberOfSmsNeedToSend++;
		limitsmscount++;
	}
	return 0;
}
BOOL simcom_sms_msg_sendtcp(UINT8 *PhoneNumber, UINT8 *SMSStr, UINT8 *SMSTCPStr, UINT16 msgLen, ResultNotifyCb pResultCb)
{

	unsigned char WaitOKByAt = 0;
	unsigned char DelayToStart = 0;
	int StrLen;
	int Tp, Tp2;
	unsigned int ExitTimer = 0;
	UINT32 ret;
	char *pData;

	// QlTimer timer2;
	// timer2.timeoutPeriod = Ql_MillisecondToTicks(NUMBER_TIMER);
	// timer2.timerId = Ql_StartTimer(&timer2);

	// str_remov_last((char*)PhoneNumber,10);
	sprintf(buf, "\n\rremov String = %s\n\r", PhoneNumber);
	sAPI_UartPrintf(buf);
	NumberSplit = 0;
	Tp2 = 0;

	StrLen = strlen((char *)SMSStr);

	if (StrLen > 159)
	{
		while (1)
		{
			for (Tp = 0; Tp <= 159; Tp++, Tp2++)
			{
				SmsSplit[NumberSplit][Tp] = SMSStr[Tp2];
				SmsSplit[NumberSplit][Tp + 1] = NULL;
				if (Tp2 >= StrLen)
					goto Exit;
			}

			NumberSplit++;
		}
	}
	else
	{
		strcpy((char *)SmsSplit[0], (char *)SMSStr);
		NumberSplit = 0;
	}
Exit:

	sAPI_UartPrintf("*** Sting Now Splited******\n\r");
	sprintf(buf, "*** NumberSplit = %d******\n\r", NumberSplit);
	sAPI_UartPrintf(buf);
	for (Tp = 0; Tp <= NumberSplit; Tp++)
	{
		sprintf(buf, "*** Split String %d= %s******\n\r", Tp, SmsSplit[Tp]);
		sAPI_UartPrintf(buf);
	}

	NumberSplitAct = 0;
	SendSMSByAt = 0;
	sprintf(SendSMSByAtPh, "%s", PhoneNumber);
	sprintf(buf, "*** SendSMSByAtPh= %s******\n\r", SendSMSByAtPh);
	sAPI_UartPrintf(buf);
	for (NumberSplitAct = 0; NumberSplitAct <= NumberSplit; NumberSplitAct++)
	{

		sprintf(Buffer1, "%s", SmsSplit[NumberSplitAct]);
		sprintf(buf, "*** Buffer1= %s******\n\r", Buffer1);
		sAPI_UartPrintf(buf);
		sprintf(SmsStrNumber[NumberOfSmsNeedToSend].Smsstr, "%s", Buffer1);
		sprintf(SmsTCPStrNumber[NumberOfSmsNeedToSend].SmsTCPstr, "%s", SMSTCPStr);
		sprintf(buf, "*** Smsstr= %s******\n\r", SmsStrNumber[NumberOfSmsNeedToSend].Smsstr);
		sAPI_UartPrintf(buf);
		sprintf(SmsStrNumber[NumberOfSmsNeedToSend].Smsnumber, "%s", SendSMSByAtPh);
		sprintf(buf, "*** Smsstr= %s******\n\r", SmsStrNumber[NumberOfSmsNeedToSend].Smsstr);
		sAPI_UartPrintf(buf);
		sprintf(buf, "\n\r+++++++++++++++SmsStrNumber[%d].Smsstr = %s++++++++++\n\r", NumberOfSmsNeedToSend, SmsStrNumber[NumberOfSmsNeedToSend].Smsstr);
		sAPI_UartPrintf(buf);
		sprintf(buf, "\n\r+++++++++++++++SmsStrNumber[%d].Smsnumber = %s++++++++++\n\r", NumberOfSmsNeedToSend, SmsStrNumber[NumberOfSmsNeedToSend].Smsnumber);
		sAPI_UartPrintf(buf);
		NumberOfSmsNeedToSend++;
		limitsmscount++;
		sprintf(buf, "\n\r NSMS[%d] ,%d \n\r", NumberOfSmsNeedToSend, limitsmscount);
		sAPI_UartPrintf(buf);
	}
	return 0;
}

void send_test_smsNum1(char *PhoneNumber, char *String, char *String1)
{
	INT32 ret;
	sprintf(buf, "\n\r Phone Number = %s\n\r", PhoneNumber);
	sAPI_UartPrintf(buf);
	sprintf(buf, "\n\r NoAcceptSMS = %d\n\r", NoAcceptSMS);
	sAPI_UartPrintf(buf);

	if (NoAcceptSMS == 0)
	{

		// sAPI_Debug("\n\r Phone Number = %s\n\r",PhoneNumber);
		// sAPI_Debug("\n\r String = %s\n\r",String);

		simcom_sms_msg_sendtcp(PhoneNumber, String, String1, strlen(String), SmSCallback);
		// SendSMSAt((UINT8*)PhoneNumber,(UINT8*)String);
		MassageReceived = 2;
	}
}

void SmSCallback(BOOL result)
{

	if (result)
	{
		//        sAPI_UartWrite(EAT_UART_2,"GSM init OK\r\n",13);
		sAPI_Debug("SMS OK\r\n", 14);
	}
	else
	{
		//        sAPI_UartWrite(EAT_UART_2,"GSM init ERROR\r\n",16);
		sAPI_Debug("SMS ERROR\r\n", 14);
	}
}

void send_Ecoconfiguaration(char *PhoneNumber)
{
    INT32 ret;
	BigSMS[0] = 0;
    sprintf(buf,"\n\r Phone Number = %s\n\r",PhoneNumber);
	sAPI_UartPrintf(buf);
	sprintf(buf,"\n\r NoAcceptSMS = %d\n\r",NoAcceptSMS);
	sAPI_UartPrintf(buf);
	
    if(NoAcceptSMS == 0)
	{
			sprintf(BigSMS,"50\n");
	//	simcom_sms_msg_sendtcp(PhoneNumber, BigSMS,BigSMS1, strlen(BigSMS),SmSCallback);
     //   MassageReceived = 2;


		simcom_sms_msg_send(PhoneNumber, BigSMS, strlen(BigSMS),SmSCallback);
	//	SendSMSAt((UINT8*)PhoneNumber,(UINT8*)String);
		MassageReceived = 2;
	}
}

void send_Valveconfiguaration(char *PhoneNumber)
{
    INT32 ret;
	BigSMS[0] = 0;
    sprintf(buf,"\n\r Phone Number = %s\n\r",PhoneNumber);
	sAPI_UartPrintf(buf);
	sprintf(buf,"\n\r NoAcceptSMS = %d\n\r",NoAcceptSMS);
	sAPI_UartPrintf(buf);
	
    if(NoAcceptSMS == 0)
	{
			sprintf(BigSMS,"55\n");
	//	simcom_sms_msg_sendtcp(PhoneNumber, BigSMS,BigSMS1, strlen(BigSMS),SmSCallback);
     //   MassageReceived = 2;


		simcom_sms_msg_send(PhoneNumber, BigSMS, strlen(BigSMS),SmSCallback);
	//	SendSMSAt((UINT8*)PhoneNumber,(UINT8*)String);
		MassageReceived = 2;
	}
}

void send_Mosconfiguaration(char *PhoneNumber)
{
    INT32 ret;
	BigSMS[0] = 0;
    sprintf(buf,"\n\r Phone Number = %s\n\r",PhoneNumber);
	sAPI_UartPrintf(buf);
	sprintf(buf,"\n\r NoAcceptSMS = %d\n\r",NoAcceptSMS);
	sAPI_UartPrintf(buf);
	
    if(NoAcceptSMS == 0)
	{
			sprintf(BigSMS,"57\n");
	//	simcom_sms_msg_sendtcp(PhoneNumber, BigSMS,BigSMS1, strlen(BigSMS),SmSCallback);
     //   MassageReceived = 2;


		simcom_sms_msg_send(PhoneNumber, BigSMS, strlen(BigSMS),SmSCallback);
	//	SendSMSAt((UINT8*)PhoneNumber,(UINT8*)String);
		MassageReceived = 2;
	}
}
void send_pumpconfiguaration(char *PhoneNumber)
{
	INT32 ret;
	BigSMS[0] = 0;
	sprintf(buf, "\n\r Phone Number = %s\n\r", PhoneNumber);
	sAPI_UartPrintf(buf);
	sprintf(buf, "\n\r NoAcceptSMS = %d\n\r", NoAcceptSMS);
	sAPI_UartPrintf(buf);

	if (NoAcceptSMS == 0)
	{
		sprintf(BigSMS, "700-%d\n", g_no_of_pumps);
		//	simcom_sms_msg_sendtcp(PhoneNumber, BigSMS,BigSMS1, strlen(BigSMS),SmSCallback);
		//   MassageReceived = 2;

		simcom_sms_msg_send(PhoneNumber, BigSMS, strlen(BigSMS), SmSCallback);
		//	SendSMSAt((UINT8*)PhoneNumber,(UINT8*)String);
		MassageReceived = 2;
	}
}
void send_tankconfiguaration(char *PhoneNumber)
{
	INT32 ret;
	BigSMS[0] = 0;
	sprintf(buf, "\n\r Phone Number = %s\n\r", PhoneNumber);
	sAPI_UartPrintf(buf);
	sprintf(buf, "\n\r NoAcceptSMS = %d\n\r", NoAcceptSMS);
	sAPI_UartPrintf(buf);

	if (NoAcceptSMS == 0)
	{
		sprintf(BigSMS, "800\n");
		//	simcom_sms_msg_sendtcp(PhoneNumber, BigSMS,BigSMS1, strlen(BigSMS),SmSCallback);
		//   MassageReceived = 2;

		simcom_sms_msg_send(PhoneNumber, BigSMS, strlen(BigSMS), SmSCallback);
		//	SendSMSAt((UINT8*)PhoneNumber,(UINT8*)String);
		MassageReceived = 2;
	}
}

void send_voltageconfiguaration(char *PhoneNumber)
{
	INT32 ret;
	BigSMS[0] = 0;
	sprintf(buf, "\n\r Phone Number = %s\n\r", PhoneNumber);
	sAPI_UartPrintf(buf);
	sprintf(buf, "\n\r NoAcceptSMS = %d\n\r", NoAcceptSMS);
	sAPI_UartPrintf(buf);

	if (NoAcceptSMS == 0)
	{
		sprintf(BigSMS, "200\n");
		//	simcom_sms_msg_sendtcp(PhoneNumber, BigSMS,BigSMS1, strlen(BigSMS),SmSCallback);
		//   MassageReceived = 2;

		simcom_sms_msg_send(PhoneNumber, BigSMS, strlen(BigSMS), SmSCallback);
		//	SendSMSAt((UINT8*)PhoneNumber,(UINT8*)String);
		MassageReceived = 2;
	}
}

void send_currentconfiguaration(char *PhoneNumber)
{
	INT32 ret;
	BigSMS[0] = 0;
	sprintf(buf, "\n\r Phone Number = %s\n\r", PhoneNumber);
	sAPI_UartPrintf(buf);
	sprintf(buf, "\n\r NoAcceptSMS = %d\n\r", NoAcceptSMS);
	sAPI_UartPrintf(buf);

	if (NoAcceptSMS == 0)
	{
		sprintf(BigSMS, "400-%d\n", pumpno_f);
		//	simcom_sms_msg_sendtcp(PhoneNumber, BigSMS,BigSMS1, strlen(BigSMS),SmSCallback);
		//   MassageReceived = 2;

		simcom_sms_msg_send(PhoneNumber, BigSMS, strlen(BigSMS), SmSCallback);
		//	SendSMSAt((UINT8*)PhoneNumber,(UINT8*)String);
		MassageReceived = 2;
	}
}
void send_finalconfiguaration(char *PhoneNumber);
void send_scheduleconfiguaration(char *PhoneNumber)
{
	INT32 ret;
	BigSMS[0] = 0;
	sprintf(buf, "\n\r Phone Number = %s\n\r", PhoneNumber);
	sAPI_UartPrintf(buf);
	sprintf(buf, "\n\r NoAcceptSMS = %d\n\r", NoAcceptSMS);
	sAPI_UartPrintf(buf);

	if (NoAcceptSMS == 0)
	{
		sprintf(BigSMS, "600-%d\n", pumpno_f);
		//	simcom_sms_msg_sendtcp(PhoneNumber, BigSMS,BigSMS1, strlen(BigSMS),SmSCallback);
		//   MassageReceived = 2;

		simcom_sms_msg_send(PhoneNumber, BigSMS, strlen(BigSMS), SmSCallback);
		//	SendSMSAt((UINT8*)PhoneNumber,(UINT8*)String);
		MassageReceived = 2;
		if (pumpno_f == g_no_of_pumps)
			send_finalconfiguaration(PhoneNumber);
	}
}
void send_finalconfiguaration(char *PhoneNumber)
{
	INT32 ret;
	BigSMS[0] = 0;
	sprintf(buf, "\n\r Phone Number = %s\n\r", PhoneNumber);
	sAPI_UartPrintf(buf);
	sprintf(buf, "\n\r NoAcceptSMS = %d\n\r", NoAcceptSMS);
	sAPI_UartPrintf(buf);

	if (NoAcceptSMS == 0)
	{
		sprintf(BigSMS, "099");
		//	simcom_sms_msg_sendtcp(PhoneNumber, BigSMS,BigSMS1, strlen(BigSMS),SmSCallback);
		//   MassageReceived = 2;

		simcom_sms_msg_send(PhoneNumber, BigSMS, strlen(BigSMS), SmSCallback);
		//	SendSMSAt((UINT8*)PhoneNumber,(UINT8*)String);
		MassageReceived = 2;
	}
}
void send_tank_finalconfiguaration(char *PhoneNumber)
{
	INT32 ret;
	BigSMS[0] = 0;
	sprintf(buf, "\n\r Phone Number = %s\n\r", PhoneNumber);
	sAPI_UartPrintf(buf);
	sprintf(buf, "\n\r NoAcceptSMS = %d\n\r", NoAcceptSMS);
	sAPI_UartPrintf(buf);

	if (NoAcceptSMS == 0)
	{
		sprintf(BigSMS, "098");
		//	simcom_sms_msg_sendtcp(PhoneNumber, BigSMS,BigSMS1, strlen(BigSMS),SmSCallback);
		//   MassageReceived = 2;

		simcom_sms_msg_send(PhoneNumber, BigSMS, strlen(BigSMS), SmSCallback);
		//	SendSMSAt((UINT8*)PhoneNumber,(UINT8*)String);
		MassageReceived = 2;
	}
}

void send_delayconfiguaration(char *PhoneNumber)
{
	INT32 ret;
	BigSMS[0] = 0;
	sprintf(buf, "\n\r Phone Number = %s\n\r", PhoneNumber);
	sAPI_UartPrintf(buf);
	sprintf(buf, "\n\r NoAcceptSMS = %d\n\r", NoAcceptSMS);
	sAPI_UartPrintf(buf);

	if (NoAcceptSMS == 0)
	{
		sprintf(BigSMS, "300-%d\n", pumpno_f);
		//	simcom_sms_msg_sendtcp(PhoneNumber, BigSMS,BigSMS1, strlen(BigSMS),SmSCallback);
		//   MassageReceived = 2;

		simcom_sms_msg_send(PhoneNumber, BigSMS, strlen(BigSMS), SmSCallback);
		//	SendSMSAt((UINT8*)PhoneNumber,(UINT8*)String);
		MassageReceived = 2;
	}
}

void send_rtcconfiguaration(char *PhoneNumber)
{
	INT32 ret;
	BigSMS[0] = 0;
	sprintf(buf, "\n\r Phone Number = %s\n\r", PhoneNumber);
	sAPI_UartPrintf(buf);
	sprintf(buf, "\n\r NoAcceptSMS = %d\n\r", NoAcceptSMS);
	sAPI_UartPrintf(buf);

	if (NoAcceptSMS == 0)
	{
		sprintf(BigSMS, "500-%d\n", pumpno_f);
		//	simcom_sms_msg_sendtcp(PhoneNumber, BigSMS,BigSMS1, strlen(BigSMS),SmSCallback);
		//   MassageReceived = 2;

		simcom_sms_msg_send(PhoneNumber, BigSMS, strlen(BigSMS), SmSCallback);
		//	SendSMSAt((UINT8*)PhoneNumber,(UINT8*)String);
		MassageReceived = 2;
	}
}

void send_ctconfiguaration(char *PhoneNumber)
{
	INT32 ret;
	BigSMS[0] = 0;
	sprintf(buf, "\n\r Phone Number = %s\n\r", PhoneNumber);
	sAPI_UartPrintf(buf);
	sprintf(buf, "\n\r NoAcceptSMS = %d\n\r", NoAcceptSMS);
	sAPI_UartPrintf(buf);

	if (NoAcceptSMS == 0)
	{
		//	sprintf(BigSMS,"ctconfig-received\n");
		sprintf(BigSMS, "100\n");
		//	simcom_sms_msg_sendtcp(PhoneNumber, BigSMS,BigSMS1, strlen(BigSMS),SmSCallback);
		//   MassageReceived = 2;

		simcom_sms_msg_send(PhoneNumber, BigSMS, strlen(BigSMS), SmSCallback);
		//	SendSMSAt((UINT8*)PhoneNumber,(UINT8*)String);
		MassageReceived = 2;
	}
}

void send_calibration(char *PhoneNumber)
{
	INT32 ret;
	BigSMS[0] = 0;
	sprintf(buf, "\n\r Phone Number = %s\n\r", PhoneNumber);
	sAPI_UartPrintf(buf);
	sprintf(buf, "\n\r NoAcceptSMS = %d\n\r", NoAcceptSMS);
	sAPI_UartPrintf(buf);

	if (NoAcceptSMS == 0)
	{
		//	sprintf(BigSMS,"ctconfig-received\n");
		sprintf(BigSMS, "900\n");
		//	simcom_sms_msg_sendtcp(PhoneNumber, BigSMS,BigSMS1, strlen(BigSMS),SmSCallback);
		//   MassageReceived = 2;

		simcom_sms_msg_send(PhoneNumber, BigSMS, strlen(BigSMS), SmSCallback);
		//	SendSMSAt((UINT8*)PhoneNumber,(UINT8*)String);
		MassageReceived = 2;
	}
}
void send_getdata(char *PhoneNumber)
{

	memset(BigSMS, 0, 500);
	if (MqttInitStatus == 1)
		sprintf(BigSMS, "IMEI:%s,\nMQTT Connected", IMEI);
	else
		sprintf(BigSMS, "IMEI:%s,\nMQTT Disconnected", IMEI);

	sAPI_UartPrintf("\n\r Phone Number = %s\n\r", PhoneNumber);
	sAPI_UartPrintf("\n\r Len Of Buffer %d\n\r", strlen(BigSMS));
	sAPI_UartPrintf("\n\r Buffer %s\n\r", BigSMS);
	// simcom_sms_send(PhoneNumber, BigSMS, strlen(BigSMS));
	simcom_sms_msg_send(PhoneNumber, BigSMS, strlen(BigSMS), SmSCallback);
}
void send_Smsno(char *PhoneNumber)
{
	sgetflag_1=1; 
	
	//	float SignalStrength;
	INT32 ret;
	sprintf(BigSMS, "QR.CODE:-%s", IMEI);
	//	if(strstr(StoredPhoneNumber[0],PhoneNumber)!=0)
	//	sprintf(BigSMS,"%s\nREG SMSM is Ok \nREG No Is: YOU",BigSMS);
	//	else
	sprintf(BigSMS, "%s\nREG SMSM is Ok \nREG No Is:%s", BigSMS, StoredPhoneNumber[0]);
	sprintf(BigSMS, "%s\nDATE=%02d/%02d/%04d\nTIME=%02d:%02d:%02d", BigSMS, datetime.tm_mday, datetime.tm_mon, datetime.tm_year, datetime.tm_hour, datetime.tm_min, datetime.tm_sec);

	sprintf(buf, "\n\r Phone Number = %s\n\r", PhoneNumber);
	sAPI_UartPrintf(buf);
	sprintf(buf, "\n\r Len Of Buffer %d\n\r", strlen(BigSMS));
	sAPI_UartPrintf(buf);
	sprintf(buf, "\n\r Buffer %s\n\r", BigSMS);
	sAPI_UartPrintf(buf);
	// simcom_sms_send(PhoneNumber, BigSMS, strlen(BigSMS));
	simcom_sms_msg_send(PhoneNumber, BigSMS, strlen(BigSMS), SmSCallback);

	//	simcom_sms_msg_send(PhoneNumber, BigSMS, strlen(BigSMS),SmSCallback);
}
void send_networkonofview(char *PhoneNumber)
{
	INT32 ret;

	BigSMS[0] = 0;
	if (Appmodeon == 1)
		sprintf(BigSMS, "SMS APPLICATION MODE ON\n");
	else
		sprintf(BigSMS, "SMS APPLICATION MODE OFF\n");
	if (wifion == 1)
		sprintf(BigSMS, "%sWIFI MODE ON\n", BigSMS);
	else
		sprintf(BigSMS, "%sWIFI MODE OFF\n", BigSMS);
	if (gprson == 1 && gprsgeton == 1)
		sprintf(BigSMS, "%sGPRS GET MODE ON\n", BigSMS);
	else if (gprson == 1 && gprsgeton == 0)
		sprintf(BigSMS, "%sGPRS MODE ON\n", BigSMS);
	else
		sprintf(BigSMS, "%sGPRS MODE OFF\n", BigSMS);
	if (apmodeon == 1)
		sprintf(BigSMS, "%sWIFI AP MODE ON\n", BigSMS);
	else
		sprintf(BigSMS, "%sWIFI AP MODE OFF\n", BigSMS);
	if (wpsmodeon == 1)
		sprintf(BigSMS, "%sWIFI WPS MODE ON\n", BigSMS);
	else
		sprintf(BigSMS, "%sWIFI WPS MODE OFF\n", BigSMS);

	sprintf(buf, "\n\r Phone Number = %s\n\r", PhoneNumber);
	sAPI_UartPrintf(buf);
	sprintf(buf, "\n\r Len Of Buffer %d\n\r", strlen(BigSMS));
	sAPI_UartPrintf(buf);
	sprintf(buf, "\n\r Buffer %s\n\r", BigSMS);
	sAPI_UartPrintf(buf);

	// sprintf(BigSMS1,"V10");
	simcom_sms_msg_sendtcp(PhoneNumber, BigSMS, BigSMS1, strlen(BigSMS), SmSCallback);
	MassageReceived = 2;
}
void send_imei(char *PhoneNumber)
{
	memset(BigSMS, NULL, sizeof(BigSMS));
	sprintf(BigSMS, "IMEI:%s", IMEI);
	sAPI_UartPrintf("\n\r Phone Number = %s\n\r", PhoneNumber);
	sAPI_UartPrintf("\n\r Len Of Buffer %d\n\r", strlen(BigSMS));
	sAPI_UartPrintf("\n\r Buffer %s\n\r", BigSMS);
	// simcom_sms_send(PhoneNumber, BigSMS, strlen(BigSMS));
	simcom_sms_msg_send(PhoneNumber, BigSMS, strlen(BigSMS), SmSCallback);
}
void send_restartsms(char *PhoneNumber)
{
	//	float SignalStrength;
	INT32 ret;
	sprintf(BigSMS, "System Restarted due to poor Network:");
	sAPI_GetRealTimeClock(&datetime);
	sprintf(BigSMS, "%s\nDATE=%02d/%02d/%04d \nTIME=%02d:%02d:%02d", BigSMS, datetime.tm_mday, datetime.tm_mon, datetime.tm_year, datetime.tm_hour, datetime.tm_min, datetime.tm_sec);
	sprintf(buf, "\n\r Phone Number = %s\n\r", PhoneNumber);
	sAPI_UartPrintf(buf);
	sprintf(buf, "\n\r Len Of Buffer %d\n\r", strlen(BigSMS));
	sAPI_UartPrintf(buf);
	sprintf(buf, "\n\r Buffer %s\n\r", BigSMS);
	sAPI_UartPrintf(buf);
	//	simcom_sms_send(PhoneNumber, BigSMS, strlen(BigSMS));
	simcom_sms_msg_send(PhoneNumber, BigSMS, strlen(BigSMS), SmSCallback);

	//	simcom_sms_msg_send(PhoneNumber, BigSMS, strlen(BigSMS),SmSCallback);
}
unsigned char CheckVoliedPhone(char *PhoneNumber) // yes_finished
{
	int Len;
	int i;
	Len = strlen(PhoneNumber);
	for (i = 0; i <= Len; i++)
	{
		if (PhoneNumber[i] > '0' && PhoneNumber[i] <= '9')
			return 1;
	}
	return 0;
}
/* void Uart_Send_with_CRC(char *data)
{
	char TempBuf[500] = "";
	size_t data_size = strlen(data);
	size_t i;
	size_t packet_size;
	uint8_t crc = crc8(data, data_size);
	data[data_size] = crc;

	// packet_size = data_size + 1; // Data size plus CRC8 byte
	//  Send the packet through UART
	// for (i = 0; i < packet_size; i++)
	// UART_WRITE(UART0, data[i]);

	sprintf(TempBuf, "$%s,", data);
	sAPI_UartWrite(SC_UART, (UINT8 *)TempBuf, strlen(TempBuf));
	sprintf(buf, "UARTW: %s\n", TempBuf);
	sAPI_UartPrintf(buf);
} */
void SMSMno_Read(void)
{
	char at_buf[50];
	sprintf(at_buf, "AT+CPBR=10\r");
	if (sAPI_AtSend(at_buf, strlen(at_buf)) != 1 /*TRUE*/)
		sAPI_UartPrintf("AT+CPBR send fail");
	else
		sAPI_UartPrintf("AT+CPBR send DONE");
}

void runtime(void)
{
	INT32 ret;
	int Tp;
	long value;
	long TpHr;
	long TpMin;
	long TpSec;
		value = MRunTimer;         //This will clear when motor start again
		TpHr = value/3600;
		value = value%3600;
		TpMin = value/60;
		value = value%60;
		TpSec = value;
    /*Motor Previous total runtime it will clear after motor start*/
	RunTimerhr =TpHr;
    RunTimermin =TpMin;
    RunTimersec =TpSec;
}

void send_vipconfigset(char *PhoneNumber)
{
	INT32 ret;
	sprintf(BigSMS, "SIM APN %s", DeviceConfig.apnName);
	sprintf(BigSMS, "%s\nMIP=%s,P%d", BigSMS, DeviceConfig.MDeviceIP, DeviceConfig.MDevicePort);
	sprintf(BigSMS, "%s\nSIP=%s,P%d", BigSMS, DeviceConfig.TcpServerIP, DeviceConfig.SocketPort);
	sprintf(BigSMS, "%s\nRID=%s,P%s", BigSMS, DeviceConfig.wifiSSID, DeviceConfig.wifiPassword);
	sprintf(BigSMS, "%s\nMYIP=%s,P%d", BigSMS, DeviceConfig.ApServerIP, DeviceConfig.DevicePort);
	sprintf(BigSMS, "%s\nDID=%s,P%s", BigSMS, DeviceConfig.ApSSID, DeviceConfig.ApPassword);
	sprintf(BigSMS, "%s\nDIP=%s,P%d", BigSMS, DeviceConfig.DeviceIP, DeviceConfig.DevicePort);
	// sprintf(BigSMS,"%s\nMQIP=%s,MQUSR=%s,MQPSW=%s",BigSMS,DeviceConfig.MqttServerIP,DeviceConfig.MqttUserName,DeviceConfig.MqttPassword);

	// sAPI_GetRealTimeClock(&datetime);
	// sprintf(BigSMS,"%s\nDATE=%02d/%02d/%04d \nTIME=%02d:%02d:%02d",BigSMS,datetime.tm_mday,datetime.tm_mon,datetime.tm_year,datetime.tm_hour, datetime.tm_min, datetime.tm_sec);

	sprintf(buf, "\n\r Phone Number = %s\n\r", PhoneNumber);
	sAPI_UartPrintf(buf);
	sprintf(buf, "\n\r Len Of Buffer %d\n\r", strlen(BigSMS));
	sAPI_UartPrintf(buf);
	sprintf(buf, "\n\r Buffer %s\n\r", BigSMS);
	sAPI_UartPrintf(buf);

	simcom_sms_msg_send(PhoneNumber, BigSMS, strlen(BigSMS), SmSCallback);
}
void send_vipconfigset2(char *PhoneNumber)
{
	INT32 ret;
	sprintf(BigSMS, "MQTIP=[%s],\nMQTTUSR=[%s],\nMQTPSW=[%s]", DeviceConfig.MqttServerIP, DeviceConfig.MqttUserName, DeviceConfig.MqttPassword);
	sprintf(BigSMS, "%s\nFTPIP=[%s],\nFTPUSR=[%s],\nFTPPSW=[%s]", BigSMS, DeviceConfig.FtpServerIP, DeviceConfig.ftpUserName, DeviceConfig.ftpPassword);

	// sAPI_GetRealTimeClock(&datetime);
	// sprintf(BigSMS,"%s\nDATE=%02d/%02d/%04d \nTIME=%02d:%02d:%02d",BigSMS,datetime.tm_mday,datetime.tm_mon,datetime.tm_year,datetime.tm_hour, datetime.tm_min, datetime.tm_sec);

	sprintf(buf, "\n\r Phone Number = %s\n\r", PhoneNumber);
	sAPI_UartPrintf(buf);
	sprintf(buf, "\n\r Len Of Buffer %d\n\r", strlen(BigSMS));
	sAPI_UartPrintf(buf);
	sprintf(buf, "\n\r Buffer %s\n\r", BigSMS);
	sAPI_UartPrintf(buf);

	simcom_sms_msg_send(PhoneNumber, BigSMS, strlen(BigSMS), SmSCallback);
}
void send_test_smsNum(char *PhoneNumber, char *String)
{
	INT32 ret;
	sprintf(buf, "\n\r Phone Number = %s\n\r", PhoneNumber);
	sAPI_UartPrintf(buf);
	sprintf(buf, "\n\r NoAcceptSMS = %d\n\r", NoAcceptSMS);
	sAPI_UartPrintf(buf);

	if (NoAcceptSMS == 0)
	{

		//	sAPI_Debug("\n\r Phone Number = %s\n\r",PhoneNumber);
		//	sAPI_Debug("\n\r String = %s\n\r",String);

		simcom_sms_msg_send(PhoneNumber, String, strlen(String), SmSCallback);
		//	SendSMSAt((UINT8*)PhoneNumber,(UINT8*)String);
		MassageReceived = 2;
	}
}
void ReadTimerSettings(void)
{
	int Tp1, Tp2, Tp3;
	int StrTokStrVer = 0;
	char *Pch = NULL;
	int Tp, i;
	SCFILE *file_hdl = NULL;
	// SCfileHandle fileHandle = {0};
	INT32 ret;

	UINT32 readedlen = 0;

	strcpy((char *)pfile, (char *)"c:/stt.txt");
	file_hdl = sAPI_fopen((char *)pfile, "ab");
	if (file_hdl == NULL)
	{
		sAPI_UartPrintf("sAPI_fopen err");
	}

	memset(textBuf, 0, 500);
	ret = sAPI_fseek(file_hdl, 0, FS_SEEK_BEGIN);
	sprintf(textBuf, "FileSeek()=%d: \r\n", ret);
	sAPI_UartPrintf(textBuf);

	memset(BigSMS, 0, 800);
	readedlen = sAPI_fread((unsigned char *)BigSMS, 800, 1, file_hdl);
	if (readedlen <= 0)
	{
		sprintf(buf, "sAPI_fread err,len: %d", readedlen);
		sAPI_UartPrintf(buf);
	}
	else
	{
		sprintf(buf, "\n\rRead data: %s, len: %d", (unsigned char *)BigSMS, readedlen);
		sAPI_UartPrintf(buf);
	}

	if (readedlen < 70)
	{
		nTimerSettings.POnHr = 0;
		nTimerSettings.POnMin = 0;
		nTimerSettings.POnSec = 0;
		nTimerSettings.SDHr = 0;
		nTimerSettings.SDMin = 0;
		nTimerSettings.SDSec = 0;

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
		nTimerSettings.fertCycLicOfMin = 0;
		nTimerSettings.fertCycLicOfSec = 0;
		nTimerSettings.fertCycLicOnMin = 0;
		nTimerSettings.fertCycLicOnSec = 0;

		nTimerSettings.RTCOnOf = 0;
		for (i = 1; i < 5; i++)
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

		nTimerSettings.PoScrDlOnOff = 0;
		nTimerSettings.PoScrDlHr = 0;
		nTimerSettings.PoScrDlMin = 0;
		nTimerSettings.PoScrDlSec = 0;

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
		nTimerSettings.calflow2 = 0;

		nTimerSettings.calflow3 = 0;

		// nTimerSettings.AutoStIIOnOff = 0;
		nTimerSettings.AutoStIIIOnOff = 0;
		nTimerSettings.AutoOlDrRstIIOnOff = 0;
		nTimerSettings.AutoDrRunRstIIOnOff = 0;

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
		nTimerSettings.CalRVoltage = 1.0;
		nTimerSettings.CalYVoltage = 1.0;
		nTimerSettings.CalBVoltage = 1.0;
		nTimerSettings.CalRCurrent = 1.0;
		nTimerSettings.CalYCurrent = 1.0;
		nTimerSettings.CalBCurrent = 1.0;
		nTimerSettings.CTRonoff = 1;
		nTimerSettings.CTYonoff = 1;
		nTimerSettings.CTBonoff = 1;

		nTimerSettings.AutoRst2On = 1;
		nTimerSettings.AutoRstOn = 1;

		nTimerSettings.HiDiffVoltII = 0;
		nTimerSettings.HiDiffVoltIII = 0;
		nTimerSettings.lowpress = 0;
		nTimerSettings.highpress = 0;
		nTimerSettings.lowpressOnOff = 0;
		nTimerSettings.highpressOnOff = 0;
	}
	else
	{

		Pch1 = strtok((char *)BigSMS, (char *)",");
		StrTokStrVer = 0;
		while (Pch1 != NULL)
		{
			strcpy(StrTokStr2[StrTokStrVer], Pch1);
			StrTokStrVer++;
			Pch1 = strtok(NULL, ",");
		}
		/* for(int Tp=0;Tp<=StrTokStrVer;Tp++)
				{
				sprintf(buf,"\n\r_St=%s  pos=%d\n\r",StrTokStr1[Tp],Tp);
		 sAPI_UartPrintf(buf);
				} */

		nTimerSettings.POnHr = myatoi(StrTokStr2[1]);
		nTimerSettings.POnMin = myatoi(StrTokStr2[2]);
		nTimerSettings.POnSec = myatoi(StrTokStr2[3]);

		nTimerSettings.SDHr = myatoi(StrTokStr2[5]);
		nTimerSettings.SDMin = myatoi(StrTokStr2[6]);
		nTimerSettings.SDSec = myatoi(StrTokStr2[7]);

		nTimerSettings.DrReOnOf = myatoi(StrTokStr2[9]);
		nTimerSettings.DrReHr = myatoi(StrTokStr2[10]);
		nTimerSettings.DrReMin = myatoi(StrTokStr2[11]);
		nTimerSettings.DrReSec = myatoi(StrTokStr2[12]);

		nTimerSettings.DrScOnOf = myatoi(StrTokStr2[14]);
		nTimerSettings.DrScHr = myatoi(StrTokStr2[15]);
		nTimerSettings.DrScMin = myatoi(StrTokStr2[16]);
		nTimerSettings.DrScSec = myatoi(StrTokStr2[17]);

		nTimerSettings.MaxRnOnOf = myatoi(StrTokStr2[19]);
		nTimerSettings.MaxRnHr = myatoi(StrTokStr2[20]);
		nTimerSettings.MaxRnMin = myatoi(StrTokStr2[21]);
		nTimerSettings.MaxRnSec = myatoi(StrTokStr2[22]);

		nTimerSettings.CycLicOnOf = myatoi(StrTokStr2[24]);
		nTimerSettings.CycLicOnHr = myatoi(StrTokStr2[25]);
		nTimerSettings.CycLicOnMin = myatoi(StrTokStr2[26]);
		nTimerSettings.CycLicOnSec = myatoi(StrTokStr2[27]);

		nTimerSettings.CycLicOfHr = myatoi(StrTokStr2[29]);
		nTimerSettings.CycLicOfMin = myatoi(StrTokStr2[30]);
		nTimerSettings.CycLicOfSec = myatoi(StrTokStr2[31]);

		nTimerSettings.RTCOnOf = myatoi(StrTokStr2[33]);

		nTimerSettings.RTCOnHr[1] = myatoi(StrTokStr2[35]);
		nTimerSettings.RTCOnMin[1] = myatoi(StrTokStr2[36]);
		nTimerSettings.RTCOnSec[1] = myatoi(StrTokStr2[37]);

		nTimerSettings.RTCOfHr[1] = myatoi(StrTokStr2[39]);
		nTimerSettings.RTCOfMin[1] = myatoi(StrTokStr2[40]);
		nTimerSettings.RTCOfSec[1] = myatoi(StrTokStr2[41]);

		nTimerSettings.RTCOnHr[2] = myatoi(StrTokStr2[43]);
		nTimerSettings.RTCOnMin[2] = myatoi(StrTokStr2[44]);
		nTimerSettings.RTCOnSec[2] = myatoi(StrTokStr2[45]);

		nTimerSettings.RTCOfHr[2] = myatoi(StrTokStr2[47]);
		nTimerSettings.RTCOfMin[2] = myatoi(StrTokStr2[48]);
		nTimerSettings.RTCOfSec[2] = myatoi(StrTokStr2[49]);

		nTimerSettings.RTCOnHr[3] = myatoi(StrTokStr2[51]);
		nTimerSettings.RTCOnMin[3] = myatoi(StrTokStr2[52]);
		nTimerSettings.RTCOnSec[3] = myatoi(StrTokStr2[53]);

		nTimerSettings.RTCOfHr[3] = myatoi(StrTokStr2[56]);
		nTimerSettings.RTCOfMin[3] = myatoi(StrTokStr2[57]);
		nTimerSettings.RTCOfSec[3] = myatoi(StrTokStr2[58]);

		nTimerSettings.RTCOnHr[4] = myatoi(StrTokStr2[60]);
		nTimerSettings.RTCOnMin[4] = myatoi(StrTokStr2[61]);
		nTimerSettings.RTCOnSec[4] = myatoi(StrTokStr2[62]);

		nTimerSettings.RTCOfHr[4] = myatoi(StrTokStr2[65]);
		nTimerSettings.RTCOfMin[4] = myatoi(StrTokStr2[66]);
		nTimerSettings.RTCOfSec[4] = myatoi(StrTokStr2[67]);

		nTimerSettings.ScrDlOnOff = myatoi(StrTokStr2[69]);
		nTimerSettings.ScrDlHr = myatoi(StrTokStr2[70]);
		nTimerSettings.ScrDlMin = myatoi(StrTokStr2[71]);
		nTimerSettings.ScrDlSec = myatoi(StrTokStr2[72]);

		nTimerSettings.LowVoltOnOff = myatoi(StrTokStr2[74]);
		nTimerSettings.LowVoltII = myatoi(StrTokStr2[75]);
		nTimerSettings.LowVoltIII = myatoi(StrTokStr2[76]);

		nTimerSettings.HighVoltOnOff = myatoi(StrTokStr2[78]);
		nTimerSettings.HighVoltII = myatoi(StrTokStr2[79]);
		nTimerSettings.HighVoltIII = myatoi(StrTokStr2[80]);

		nTimerSettings.ImbVolt = myatoi(StrTokStr2[82]);
		nTimerSettings.RvePhOnoff = myatoi(StrTokStr2[84]);
		nTimerSettings.SppOnoff = myatoi(StrTokStr2[86]);
		nTimerSettings.CurSppOnOff = myatoi(StrTokStr2[88]);

		nTimerSettings.OlOnOff = myatoi(StrTokStr2[90]);
		nTimerSettings.wrOlAmpsII = myatoi(StrTokStr2[91]);
		nTimerSettings.wrOlAmpsIII = myatoi(StrTokStr2[92]);
		nTimerSettings.OlScanHr = myatoi(StrTokStr2[93]);
		nTimerSettings.OlScanMin = myatoi(StrTokStr2[94]);
		nTimerSettings.OlScanSec = myatoi(StrTokStr2[95]);

		nTimerSettings.wrDrAmpsII = myatoi(StrTokStr2[97]);
		nTimerSettings.wrDrAmpsIII = myatoi(StrTokStr2[98]);

		// nTimerSettings.AutoStIIOnOff = myatoi(StrTokStr2[100]);
		nTimerSettings.AutoStIIIOnOff = myatoi(StrTokStr2[102]);
		nTimerSettings.AutoOlDrRstIIOnOff = myatoi(StrTokStr2[104]);
		nTimerSettings.ManualOnOff = myatoi(StrTokStr2[106]);

		nTimerSettings.OlRstVolOnoff = myatoi(StrTokStr2[108]);
		nTimerSettings.OlRstVol = myatoi(StrTokStr2[109]);

		nTimerSettings.DrOccurOnOff = myatoi(StrTokStr2[111]);
		nTimerSettings.DrOccurTimHr = myatoi(StrTokStr2[112]);
		nTimerSettings.DrOccurTimMin = myatoi(StrTokStr2[113]);
		nTimerSettings.DrOccurTimSec = myatoi(StrTokStr2[114]);
		nTimerSettings.DrOccurNum = myatoi(StrTokStr2[115]);

		nTimerSettings.DiffVoltII = myatoi(StrTokStr2[117]);
		nTimerSettings.DiffVoltIII = myatoi(StrTokStr2[118]);

		nTimerSettings.wrCalRVoltage = myatoi(StrTokStr2[120]);
		nTimerSettings.wrCalYVoltage = myatoi(StrTokStr2[121]);
		nTimerSettings.wrCalBVoltage = myatoi(StrTokStr2[122]);
		nTimerSettings.wrCalRCurrent = myatoi(StrTokStr2[124]);
		nTimerSettings.wrCalYCurrent = myatoi(StrTokStr2[125]);
		nTimerSettings.wrCalBCurrent = myatoi(StrTokStr2[126]);
		nTimerSettings.CTRonoff = myatoi(StrTokStr2[128]);
		nTimerSettings.CTYonoff = myatoi(StrTokStr2[129]);
		nTimerSettings.CTBonoff = myatoi(StrTokStr2[130]);

		nTimerSettings.AutoRst2On = myatoi(StrTokStr2[132]);
		nTimerSettings.AutoRstOn = myatoi(StrTokStr2[134]);

		nTimerSettings.HiDiffVoltII = myatoi(StrTokStr2[136]);
		if (nTimerSettings.HiDiffVoltII > 300)
			nTimerSettings.HiDiffVoltII = 0;
		nTimerSettings.HiDiffVoltIII = myatoi(StrTokStr2[137]);
		if (nTimerSettings.HiDiffVoltIII > 300)
			nTimerSettings.HiDiffVoltIII = 0;

		nTimerSettings.wrRPhaseToPhaseFactor = myatoi(StrTokStr2[138]);
		nTimerSettings.wrYPhaseToPhaseFactor = myatoi(StrTokStr2[139]);
		nTimerSettings.wrBPhaseToPhaseFactor = myatoi(StrTokStr2[140]);
		nTimerSettings.wrR2PhaseToPhaseFactor = myatoi(StrTokStr2[141]);
		nTimerSettings.PoScrDlOnOff = myatoi(StrTokStr2[143]);
		nTimerSettings.PoScrDlHr = myatoi(StrTokStr2[144]);
		nTimerSettings.PoScrDlMin = myatoi(StrTokStr2[145]);
		nTimerSettings.PoScrDlSec = myatoi(StrTokStr2[146]);
		sprintf(buf, "Calc,%5d,%5d,%5d\n\r", nTimerSettings.wrCalRCurrent, nTimerSettings.wrCalYCurrent, nTimerSettings.wrCalBCurrent);
		sAPI_UartPrintf(buf);
		nTimerSettings.AutoDrRunRstIIOnOff = myatoi(StrTokStr2[148]);

		nTimerSettings.OlAmpsII = (nTimerSettings.wrOlAmpsII * 0.1);
		nTimerSettings.OlAmpsIII = (nTimerSettings.wrOlAmpsIII * 0.1);
		nTimerSettings.DrAmpsII = (nTimerSettings.wrDrAmpsII * 0.1);
		nTimerSettings.DrAmpsIII = (nTimerSettings.wrDrAmpsIII * 0.1);
		nTimerSettings.wrDrAmpsII = myatoi(StrTokStr2[149]);
		nTimerSettings.wrDrAmpsIII = myatoi(StrTokStr2[150]);
		nTimerSettings.calflow2 = (nTimerSettings.wrDrAmpsII * 0.1);
		nTimerSettings.calflow3 = (nTimerSettings.wrDrAmpsIII * 0.1);
		nTimerSettings.wrlowpress = myatoi(StrTokStr2[153]);
		nTimerSettings.wrhighpress = myatoi(StrTokStr2[154]);
		nTimerSettings.lowpressOnOff = myatoi(StrTokStr2[151]);
		nTimerSettings.highpressOnOff = myatoi(StrTokStr2[152]);
		nTimerSettings.lowpress = nTimerSettings.wrlowpress * 0.1;
		nTimerSettings.highpress = nTimerSettings.wrhighpress * 0.1;
		nTimerSettings.fertCycLicOfMin = myatoi(StrTokStr2[155]);
		nTimerSettings.fertCycLicOfSec = myatoi(StrTokStr2[156]);
		nTimerSettings.fertCycLicOnMin = myatoi(StrTokStr2[157]);
		nTimerSettings.fertCycLicOnSec = myatoi(StrTokStr2[158]);

		nTimerSettings.CalRVoltage = (nTimerSettings.wrCalRVoltage * 0.0001);
		nTimerSettings.CalYVoltage = (nTimerSettings.wrCalYVoltage * 0.0001);
		nTimerSettings.CalBVoltage = (nTimerSettings.wrCalBVoltage * 0.0001);
		nTimerSettings.CalRCurrent = (nTimerSettings.wrCalRCurrent * 0.0001);
		nTimerSettings.CalYCurrent = (nTimerSettings.wrCalYCurrent * 0.0001);
		nTimerSettings.CalBCurrent = (nTimerSettings.wrCalBCurrent * 0.0001);
		nTimerSettings.RPhaseToPhaseFactor = (nTimerSettings.wrRPhaseToPhaseFactor * 0.0001);
		nTimerSettings.YPhaseToPhaseFactor = (nTimerSettings.wrYPhaseToPhaseFactor * 0.0001);
		nTimerSettings.BPhaseToPhaseFactor = (nTimerSettings.wrBPhaseToPhaseFactor * 0.0001);
		nTimerSettings.R2PhaseToPhaseFactor = (nTimerSettings.wrR2PhaseToPhaseFactor * 0.0001);
		sprintf(buf, "Calc,%0.4f,%0.4f,%0.4f\n\r", nTimerSettings.CalRCurrent, nTimerSettings.CalYCurrent, nTimerSettings.CalBCurrent);
		sAPI_UartPrintf(buf);

		if (nTimerSettings.CalRVoltage <= 0 || nTimerSettings.CalRVoltage >= 10)
			nTimerSettings.CalRVoltage = 1.0;
		if (nTimerSettings.CalYVoltage <= 0 || nTimerSettings.CalYVoltage >= 10)
			nTimerSettings.CalYVoltage = 1.0;
		if (nTimerSettings.CalBVoltage <= 0 || nTimerSettings.CalBVoltage >= 10)
			nTimerSettings.CalBVoltage = 1.0;
		if (nTimerSettings.CalRCurrent <= 0 || nTimerSettings.CalRCurrent >= 1000)
			nTimerSettings.CalRCurrent = 1.0;
		if (nTimerSettings.CalYCurrent <= 0 || nTimerSettings.CalYCurrent >= 1000)
			nTimerSettings.CalYCurrent = 1.0;
		if (nTimerSettings.CalBCurrent <= 0 || nTimerSettings.CalBCurrent >= 1000)
			nTimerSettings.CalBCurrent = 1.0;
		if (nTimerSettings.RPhaseToPhaseFactor <= 0 || nTimerSettings.RPhaseToPhaseFactor > 5)
			nTimerSettings.RPhaseToPhaseFactor = 1.0;
		if (nTimerSettings.YPhaseToPhaseFactor <= 0 || nTimerSettings.YPhaseToPhaseFactor > 5)
			nTimerSettings.YPhaseToPhaseFactor = 1.0;
		if (nTimerSettings.BPhaseToPhaseFactor <= 0 || nTimerSettings.BPhaseToPhaseFactor > 5)
			nTimerSettings.BPhaseToPhaseFactor = 1.0;
		if (nTimerSettings.R2PhaseToPhaseFactor <= 0 || nTimerSettings.R2PhaseToPhaseFactor > 5)
			nTimerSettings.R2PhaseToPhaseFactor = 1.0;
	}

	// sAPI_Debug("POn,%02d,%02d,%02d,\n\r",nTimerSettings.POnHr,nTimerSettings.POnMin,nTimerSettings.POnSec);
	// sAPI_Debug("SD,%02d,%02d,%02d,\n\r",nTimerSettings.SDHr,nTimerSettings.SDMin,nTimerSettings.SDSec);
	// sAPI_Debug("DrRe,%02d,%02d,%02d,%02d,\n\r",nTimerSettings.DrReOnOf,nTimerSettings.DrReHr,nTimerSettings.DrReMin,nTimerSettings.DrReSec);
	// sAPI_Debug("DrSc,%02d,%02d,%02d,%02d,\n\r",nTimerSettings.DrScOnOf,nTimerSettings.DrScHr,nTimerSettings.DrScMin,nTimerSettings.DrScSec);
	// sAPI_Debug("MaxRn,%02d,%02d,%02d,%02d,\n\r",nTimerSettings.MaxRnOnOf,nTimerSettings.MaxRnHr,nTimerSettings.MaxRnMin,nTimerSettings.MaxRnSec);
	// sAPI_Debug("CycLic,%02d,%02d,%02d,%02d,\n\r",nTimerSettings.CycLicOnOf,nTimerSettings.CycLicOnHr,nTimerSettings.CycLicOnMin,nTimerSettings.CycLicOnSec);
	// sAPI_Debug("CycLicOf,%02d,%02d,%02d,\n\r",nTimerSettings.CycLicOfHr,nTimerSettings.CycLicOfMin,nTimerSettings.CycLicOfSec);
	// sAPI_Debug("RTCOn,%02d,\n\r",nTimerSettings.RTCOnOf);
	// sAPI_Debug("RTCOn1,%02d,%02d,%02d,\n\r",nTimerSettings.RTCOnHr1,nTimerSettings.RTCOnMin1,nTimerSettings.RTCOnSec1);
	// sAPI_Debug("RTCOf1,%02d,%02d,%02d,\n\r",nTimerSettings.RTCOfHr1,nTimerSettings.RTCOfMin1,nTimerSettings.RTCOfSec1);
	// sAPI_Debug("RTCOn2,%02d,%02d,%02d,\n\r",nTimerSettings.RTCOnHr2,nTimerSettings.RTCOnMin2,nTimerSettings.RTCOnSec2);
	// sAPI_Debug("RTCOf2,%02d,%02d,%02d,\n\r",nTimerSettings.RTCOfHr2,nTimerSettings.RTCOfMin2,nTimerSettings.RTCOfSec2);
	// sAPI_Debug("RTCOn3,%02d,%02d,%02d,\n\r",nTimerSettings.RTCOnHr3,nTimerSettings.RTCOnMin3,nTimerSettings.RTCOnSec3);
	// sAPI_Debug("RTCOf3,%02d,%02d,%02d,\n\r",nTimerSettings.RTCOfHr3,nTimerSettings.RTCOfMin3,nTimerSettings.RTCOfSec3);
	// sAPI_Debug("RTCOn4,%02d,%02d,%02d,\n\r",nTimerSettings.RTCOnHr4,nTimerSettings.RTCOnMin4,nTimerSettings.RTCOnSec4);
	// sAPI_Debug("RTCOf4,%02d,%02d,%02d,\n\r",nTimerSettings.RTCOfHr4,nTimerSettings.RTCOfMin4,nTimerSettings.RTCOfSec4);

	// sAPI_Debug("SCR,%02d,%02d,%02d,%02d,\n\r",nTimerSettings.ScrDlOnOff,nTimerSettings.ScrDlHr,nTimerSettings.ScrDlMin,nTimerSettings.ScrDlSec);
	// sAPI_Debug("Lv,%02d,%03d,%03d,\n\r",nTimerSettings.LowVoltOnOff,nTimerSettings.LowVoltII,nTimerSettings.LowVoltIII);
	// sAPI_Debug("Hv,%02d,%03d,%03d,\n\r",nTimerSettings.HighVoltOnOff,nTimerSettings.HighVoltII,nTimerSettings.HighVoltIII);
	// sAPI_Debug("Im,%03d,\n\r",nTimerSettings.ImbVolt);
	// sAPI_Debug("RSP,%02d,\n\r",nTimerSettings.RvePhOnoff);
	// sAPI_Debug("SPP,%02d,\n\r",nTimerSettings.SppOnoff);
	// sAPI_Debug("CSP,%02d,\n\r",nTimerSettings.CurSppOnOff);
	// sAPI_Debug("OLA,%02d,%0.1f,%0.1f,%02d,%02d,%02d,\n\r",nTimerSettings.OlOnOff,nTimerSettings.OlAmpsII,nTimerSettings.OlAmpsIII,nTimerSettings.OlScanHr,nTimerSettings.OlScanMin,nTimerSettings.OlScanSec);
	// sAPI_Debug("DRA,%0.1f,%0.1f,\n\r",nTimerSettings.DrAmpsII,nTimerSettings.DrAmpsIII);
	// sAPI_Debug("AII,%02d,\n\r",nTimerSettings.AutoStIIOnOff);
	// sAPI_Debug("AIII,%02d,\n\r",nTimerSettings.AutoStIIIOnOff);
	// sAPI_Debug("AODR,%02d,\n\r",nTimerSettings.AutoOlDrRstIIOnOff);
	// sAPI_Debug("MAN,%02d,\n\r",nTimerSettings.ManualOnOff);
	// sAPI_Debug("OLRV,%02d,%03d,\n\r",nTimerSettings.OlRstVolOnoff,nTimerSettings.OlRstVol);
	// sAPI_Debug("DROC,%02d,%02d,%02d,%02d,%02d,\n\r",nTimerSettings.DrOccurOnOff,nTimerSettings.DrOccurTimHr,nTimerSettings.DrOccurTimMin,nTimerSettings.DrOccurTimSec,nTimerSettings.DrOccurNum);
	// sAPI_Debug("diff,%03d,%03d,\n\r",nTimerSettings.DiffVoltII,nTimerSettings.DiffVoltIII);
	// sAPI_Debug("Calv,%0.4f,%0.4f,%0.4f\n\r",nTimerSettings.CalRVoltage,nTimerSettings.CalYVoltage,nTimerSettings.CalBVoltage);
	// sAPI_Debug("Calc,%0.4f,%0.4f,%0.4f\n\r",nTimerSettings.CalRCurrent,nTimerSettings.CalYCurrent,nTimerSettings.CalBCurrent);
	// sAPI_Debug("AutoDrRunRstIIOnOff=%d,\n\r",nTimerSettings.AutoDrRunRstIIOnOff);

	ret = sAPI_fclose(file_hdl);
	if (ret != 0)
	{
		sAPI_UartPrintf("sAPI_fclose err");
	}
	else
	{
		file_hdl = NULL;
	}

	/*	nMoTr.OnDelay = (nTimerSettings.POnHr*3600)+(nTimerSettings.POnMin*60)+nTimerSettings.POnSec;
		nMoTr.StarDetaDelay = (nTimerSettings.SDHr*3600)+(nTimerSettings.SDMin*60)+nTimerSettings.SDSec;

		if(nTimerSettings.DrReOnOf == 1)
			nMoTr.DrRunRestart = (nTimerSettings.DrReHr*3600)+(nTimerSettings.DrReMin*60)+nTimerSettings.DrReSec;
		else
			nMoTr.DrRunRestart = 0;

		if(nTimerSettings.DrScOnOf == 1)
			nMoTr.DrRunScan = (nTimerSettings.DrScHr*3600)+(nTimerSettings.DrScMin*60)+nTimerSettings.DrScSec;
		else
			nMoTr.DrRunScan = 0;

		if(nTimerSettings.MaxRnOnOf == 1)
			nMoTr.MaxRunTimer = (nTimerSettings.MaxRnHr*3600)+(nTimerSettings.MaxRnMin*60)+nTimerSettings.MaxRnSec;
		else
			nMoTr.MaxRunTimer = 0; */

	if (nTimerSettings.CycLicOnOf == 1)
	{
		//	nMoTr.CycLicOnDelay = (nTimerSettings.CycLicOnHr*3600)+(nTimerSettings.CycLicOnMin*60)+nTimerSettings.CycLicOnSec;
		//	nMoTr.CycLicOfDelay = (nTimerSettings.CycLicOfHr*3600)+(nTimerSettings.CycLicOfMin*60)+nTimerSettings.CycLicOfSec;
	}

	/* if(nTimerSettings.RTCOnOf == 1)
	{
		for(i=1;i<5;i++)
		{
		nMoTr.RTCON[i] = (nTimerSettings.RTCOnHr[i]*3600)+(nTimerSettings.RTCOnMin[i]*60)+nTimerSettings.RTCOnSec[i];
		nMoTr.RTCOf[i] = (nTimerSettings.RTCOfHr[i]*3600)+(nTimerSettings.RTCOfMin[i]*60)+nTimerSettings.RTCOfSec[i];
		}

		for(i=1;i<5;i++)
		{
		if(nMoTr.RTCON[i]>86400)
		nMoTr.RTCON[i] = 86400;
		if(nMoTr.RTCOf[i]>86400)
		nMoTr.RTCOf[i] = 86400;

		}
		for(i=1;i<5;i++)
		{
		if(nMoTr.RTCON[i]>=nMoTr.RTCOf[i])
			{
				nMoTr.RTCON[i] = 0;
				nMoTr.RTCOf[i] = 0;
			}


		}


		}
		else
		{
			for(i=1;i<5;i++)
		{

				nMoTr.RTCON[i] = 0;
				nMoTr.RTCOf[i] = 0;



		}
		} */
}

void ReadonofFile(void)
{

	//	char StrTokStr[30][20];   // dg_changed from [30][20]
	int StrTokStrVer = 0;
	char *Pch = NULL;
	int Tp;
	SCFILE *file_hdl = NULL;
	INT32 ret;
	UINT32 readedlen;

	strcpy((char *)pfile, (char *)"c:/relayonof.txt");

	file_hdl = sAPI_fopen((UINT8 *)pfile, "ab");
	if (file_hdl == NULL)
	{
		sAPI_UartPrintf("sAPI_fopen err :c:/relayonof.txt");
	}
	memset(textBuf, 0, 50);
	ret = sAPI_fseek(file_hdl, 0, FS_SEEK_BEGIN);
	sprintf(buf, "c:/relayonof.txt FileSeek()=%d: \r\n", ret);
	sAPI_UartPrintf(buf);
	memset(strBuf, 0, 100); // 70
	readedlen = sAPI_fread((unsigned char *)strBuf, 100, 1, file_hdl);
	if (readedlen <= 0)
	{
		sprintf(buf, "sAPI_FsFileRead err,data: %s, len: %d", strBuf, readedlen);
		sAPI_UartPrintf(buf);
	}
	sprintf(buf, "c:/relayonof.txt FileRead()=%d: readedlen=%d, strBuf=%s\r\n", ret, readedlen, strBuf);
	sAPI_UartPrintf(buf);

	if (readedlen < 5)
	{
		MakeRealyOn = 0;
		PrvAutoMobileKey = 0;
		Appmodeon = 0;
		wifion = 0;
		gprson = 0;
		apmodeon = 0;
		progqueonof = 0;
		wpsmodeon = 0;
		gprsgeton = 0;

		valve_onof_msg = 0;
		//	settank_height=0;
		nMSettings.motor2onof = 0;
		nMSettings.motor3onof = 0;
		nMSettings.motor4onof = 0;

		creg_count = 0;

		//	nVaTr.pswdelay=5;
		nMSettings.motor4ctrlonof = 0;
		//	nMSettings.rtcautorstonof=0;
	}
	else
	{

		Pch = strtok((char *)strBuf, (char *)",");
		StrTokStrVer = 0;
		while (Pch != NULL)
		{
			strcpy(StrTokStr[StrTokStrVer], Pch);
			StrTokStrVer++;
			Pch = strtok(NULL, ",");
		}

		// for(Tp=0;Tp<=StrTokStrVer;Tp++)
		//{
		//	sprintf(buf,"\n\rSt=%s  pos=%d\n\r",StrTokStr[Tp],Tp);
		//  sAPI_UartPrintf(buf);

		//}

		MakeRealyOn = myatoi(StrTokStr[1]);
		PrvAutoMobileKey = myatoi(StrTokStr[2]);
		Appmodeon = myatoi(StrTokStr[3]);
		wifion = myatoi(StrTokStr[4]);
		gprson = myatoi(StrTokStr[5]);
		apmodeon = myatoi(StrTokStr[6]);

		wpsmodeon = myatoi(StrTokStr[7]);
		gprsgeton = myatoi(StrTokStr[8]);
		creg_count = myatoi(StrTokStr[9]);
		creg_reset_flag = myatoi(StrTokStr[10]);
		nMSettings.motor1onof = myatoi(StrTokStr[11]);
		nMSettings.motor2onof = myatoi(StrTokStr[12]);
		nMSettings.motor3onof = myatoi(StrTokStr[13]);
		nMSettings.motor4onof = myatoi(StrTokStr[14]);

		nVaTr.cregdelayHr = myatoi(StrTokStr[15]);
		nVaTr.cregdelayMin = myatoi(StrTokStr[16]);
		nVaTr.cregrstonof = myatoi(StrTokStr[17]);
		if (nMSettings.ndebugonof)
		{
			sprintf(buf, "read_file valve_onof_msg is %d", valve_onof_msg);
			sAPI_UartPrintf(buf);
			//	sprintf(buf,"read_file settank_height is %f",settank_height);
			//	sAPI_UartPrintf(buf);
			sprintf(buf, "\n\rMakeRealyOn,%d,%d", MakeRealyOn, PrvAutoMobileKey);
			sAPI_UartPrintf(buf);
			sprintf(buf, " AFTER READ \n\rmotor2onof %d,motor3onof %d,motor4onof%d,", nMSettings.motor2onof, nMSettings.motor3onof, nMSettings.motor4onof);
			sAPI_UartPrintf(buf);
			sprintf(buf, "read_file creg_count is %d", creg_count);
			sAPI_UartPrintf(buf);
			//	sprintf(buf,"read_file nVaTr.pswdelay is %d",nVaTr.pswdelay);
			sAPI_UartPrintf(buf);
			sprintf(buf, "read_file nMSettings.motor4ctrlonof is %d", nMSettings.motor4ctrlonof);
			sAPI_UartPrintf(buf);
		}
	}
	ret = sAPI_fclose(file_hdl);
	if (ret != 0)
	{
		sAPI_UartPrintf("c:/relayonof.txt sAPI_fclose err");
	}
	else
	{
		file_hdl = NULL;
	}
}

void WriteonofFile(void)
{
	INT32 ret;
	UINT32 writeedlen;
	SCFILE *file_hdl = NULL;
	
	ret = sAPI_GetSize("c:");
	sprintf(buf, "size=%d: \r\n", ret);
	sAPI_UartPrintf(buf);
	ret = sAPI_GetFreeSize("c:");
	sprintf(buf, "free size=%d: \r\n", ret);
	sAPI_UartPrintf(buf);
	strcpy((char *)pfile, (char *)"c:/relayonof.txt");

	file_hdl = sAPI_fopen((char *)pfile, "wb");
	if (file_hdl == NULL)
	{
		sAPI_UartPrintf("sAPI_fopen err");
	}

//	sprintf(textBuf, "FileOpenEx Create (%s,%08x)=%d\r\n", pfile, FS_CREATE, ret);
	sAPI_UartPrintf("open file");

	memset(textBuf, 0, 100); // 55
	ret = sAPI_fseek(file_hdl, 0, FS_SEEK_BEGIN);
	sprintf(textBuf, "FileSeek()=%d: \r\n", ret);
	sAPI_UartPrintf(textBuf);

	memset(textBuf, 0, 100); // 50
	sprintf(buf, " before write PrvAutoMobileKey is %d", PrvAutoMobileKey);
	sAPI_UartPrintf(buf);
	//	sprintf(textBuf,"make_relay_on,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%02d,%02d,%d\n\r",MakeRealyOn,PrvAutoMobileKey,Appmodeon,wifion,gprson,apmodeon,wpsmodeon,gprsgeton,creg_count,nVaTr.lotank_cyc_restart_flag,creg_reset_flag,nVaTr.cregdelayHr,nVaTr.cregdelayMin,nVaTr.cregrstonof);
	sprintf(textBuf, "make_relay_on,%d,%d,%d,%d,%d,%d,%d,%d,%02d,%d,%d,%d,%d,%d,%02d,%02d,%d,%d,\n\r", MakeRealyOn, PrvAutoMobileKey, Appmodeon, wifion, gprson, apmodeon, wpsmodeon, gprsgeton, creg_count, creg_reset_flag, nMSettings.motor1onof, nMSettings.motor2onof, nMSettings.motor3onof, nMSettings.motor4onof, nVaTr.cregdelayHr, nVaTr.cregdelayMin, nVaTr.cregrstonof);

	sAPI_UartPrintf(textBuf);

	writeedlen = sAPI_fwrite((UINT8 *)textBuf, strlen((char *)textBuf), 1, file_hdl);
	if (writeedlen != strlen((char *)textBuf))
	{
		sprintf(buf, "sAPI_fwrite err write length: %d\r\n", writeedlen);
		sAPI_UartPrintf(buf);
	}
	sprintf(textBuf, "FileWrite{%s},writeedlen=%d\r\n", pfile, writeedlen);
	sAPI_UartPrintf(textBuf);

	ret = sAPI_fclose(file_hdl);
	if (ret != 0)
	{
		sAPI_UartPrintf("sAPI_fclose err");
	}
	else
	{
		file_hdl = NULL;
	}
}
void ReadFile(void)
{
	SCFILE *file_hdl = NULL;
	// char StrTokStr[40][20];
	int StrTokStrVer = 0;
	char *Pch = NULL;
	int Tp;
	unsigned long buff_data_len = 0;
	INT32 ret;
	UINT32 readedlen;
	sAPI_UartPrintf("\n\rIN READ FILE>>");
	memset(pfile, 0, 200);
	memcpy(pfile, "c:/st.txt", strlen("c:/st.txt"));
	// strcpy((char *)pfile,(char*)"c:/st.txt");

	file_hdl = sAPI_fopen(pfile, "ab");
	if (file_hdl == NULL)
	{
		sAPI_UartPrintf("sAPI_fopen err ");
	}
	else
	{
		sprintf(buf, "\n\rFileOpen Succeed(%s)=%d\r\n", pfile, ret);
		sAPI_UartPrintf(buf);
	}
	memset(textBuf, 0, 500 * 2);
	ret = sAPI_fseek(file_hdl, 0, FS_SEEK_BEGIN);
	sprintf(buf, "\n\rFileSeek()=%d:\r\n", ret);
	sAPI_UartPrintf(buf);
	// buff_data_len = sAPI_fsize(file_hdl);
	memset(strBuf, 0, 500);
	// readedlen = sAPI_fread((unsigned char *)strBuf,130, 1, file_hdl);
	readedlen = sAPI_fread(strBuf, 130, 1, file_hdl);
	if (readedlen <= 0)
	{
		sprintf(buf, "\n\r [st.txt]>>sAPI_fRead err,data: %s, len: %d", strBuf, readedlen);
		sAPI_UartPrintf(buf);
	}
	else
	{

		sprintf(buf, "\n\rFileRead()=%d: Readedlen=%d, strBuf=%s\r\n", ret, readedlen, strBuf);
		sAPI_UartPrintf(buf);
	}

	if (readedlen < 10)
	{
		nTimerSettings.SfbHr = 0;
		nTimerSettings.SfbMin = 0;
		nTimerSettings.SfbSec = 0;
		nMSettings.SMSOnOff = 1; // subash
		nMSettings.staticSMSOnOff = 0;
		nMSettings.sampleSMSOnOff = 0;
		nMSettings.canSMSOnOff = 1;
		nMSettings.SfbOnOff = 0;
		nMSettings.PressureOnOff = 0;
		nMSettings.ManualswitchOnOff = 0;
		nMSettings.TankOnOff = 0;
		nMSettings.SumpOnOff = 0;
		nMSettings.DryRunOnOff = 0;
		nMSettings.TargetOnOff = 0;
		nMSettings.serviceOnOff = 0;
		nMSettings.pwrvfbOnOf = 0;
		nMSettings.VBFOnOff = 0;
		nMSettings.RelayControlOnCall = 0;
		nMSettings.SecOnOf = 0;
		nTimerSettings.pfcOnOff = 0;
		nTimerSettings.pfcvolt = 0;
		Mobile2Phs3Phase = 0;
		limitsmsonof = 0;
		// limitsmsset=0;
		DNDSMSFLAG[0] = 0;
		DNDSMSFLAG[1] = 0;
		DNDSMSFLAG[2] = 0;
		DNDSMSFLAG[3] = 0;
		nMSettings.dataSMSOnOff = 0;
		nMSettings.SumprstOnOff = 0;
	}
	else
	{

		Pch = strtok((char *)strBuf, (char *)",");
		StrTokStrVer = 0;
		while (Pch != NULL)
		{
			strcpy(StrTokStr[StrTokStrVer], Pch);
			StrTokStrVer++;
			Pch = strtok(NULL, ",");
		}

		/*for(Tp=0;Tp<=StrTokStrVer;Tp++)
		{
			sprintf(buf,"\n\rSt=%s  pos=%d\n\r",StrTokStr[Tp],Tp);
// sAPI_UartPrintf(buf);
		}*/

		nMSettings.SMSOnOff = myatoi(StrTokStr[1]);
		nMSettings.PressureOnOff = myatoi(StrTokStr[2]);
		nMSettings.SfbOnOff = myatoi(StrTokStr[3]);
		nMSettings.ManualswitchOnOff = myatoi(StrTokStr[4]);
		nMSettings.TankOnOff = myatoi(StrTokStr[5]);
		nMSettings.SumpOnOff = myatoi(StrTokStr[7]);
		nMSettings.DryRunOnOff = myatoi(StrTokStr[9]);
		nMSettings.TargetOnOff = myatoi(StrTokStr[11]);
		nMSettings.VBFOnOff = myatoi(StrTokStr[13]);
		nMSettings.RelayControlOnCall = myatoi(StrTokStr[15]);
		nMSettings.SecOnOf = myatoi(StrTokStr[17]);
		nTimerSettings.pfcOnOff = myatoi(StrTokStr[18]);
		nTimerSettings.pfcvolt = myatoi(StrTokStr[19]);
		Mobile2Phs3Phase = myatoi(StrTokStr[20]);
		nMSettings.gethidesmsonoff = myatoi(StrTokStr[21]);
		nMSettings.staticSMSOnOff = myatoi(StrTokStr[22]);
		nTimerSettings.SfbHr = myatoi(StrTokStr[23]);
		nTimerSettings.SfbMin = myatoi(StrTokStr[24]);
		nTimerSettings.SfbSec = myatoi(StrTokStr[25]);
		limitsmsonof = myatoi(StrTokStr[26]);
		limitsmsset = myatoi(StrTokStr[27]);
		nMSettings.serviceOnOff = myatoi(StrTokStr[28]);
		nMSettings.pwrvfbOnOf = myatoi(StrTokStr[29]);
		DNDSMSFLAG[0] = myatoi(StrTokStr[30]);
		DNDSMSFLAG[1] = myatoi(StrTokStr[31]);
		DNDSMSFLAG[2] = myatoi(StrTokStr[32]);
		DNDSMSFLAG[3] = myatoi(StrTokStr[33]);
		nMSettings.sampleSMSOnOff = myatoi(StrTokStr[34]);
		nMSettings.dataSMSOnOff = myatoi(StrTokStr[35]);
		nMSettings.canSMSOnOff = myatoi(StrTokStr[36]);
		nMSettings.SumprstOnOff = myatoi(StrTokStr[37]);

		sAPI_UartPrintf("after Read_ canSMSOnOff is %d", nMSettings.canSMSOnOff);

		sprintf(buf, "\n\rSMNF,%d,\n\rSFNF,%d,\n\rTNNF,%d,\n\rSUNF,%d,\n\rDRNF,%d,\n\rTRNF,%d,\n\rVBNF,%d,\n\rCANF,%d,\n\r", nMSettings.SMSOnOff,
				nMSettings.SfbOnOff, nMSettings.TankOnOff, nMSettings.SumpOnOff, nMSettings.DryRunOnOff,
				nMSettings.TargetOnOff, nMSettings.VBFOnOff, nMSettings.RelayControlOnCall);
		sAPI_UartPrintf(buf);

		sprintf(buf, "\n\rSCNF,%d,%d,%03d,%d,%d,\n\r", nMSettings.SecOnOf, nTimerSettings.pfcOnOff, nTimerSettings.pfcvolt, Mobile2Phs3Phase, nMSettings.gethidesmsonoff);
		sAPI_UartPrintf(buf);
	}

	ret = sAPI_fclose(file_hdl);
	if (ret != 0)
	{
		sAPI_UartPrintf("sAPI_fclose err");
	}
	else
	{
		file_hdl = NULL;
		sAPI_UartPrintf("\n\rfile close success");
	}
	// nMoTr.SfbDelay = (nTimerSettings.SfbHr*3600)+(nTimerSettings.SfbMin*60)+nTimerSettings.SfbSec;
}

void WritePhoneNumberFn(void)
{
#ifdef Store_Not_Txt_File
	char i, j;
	for (i = 0; i < 19; i++)
	{
		memset(DeviceConfig.MobileNumber[i], 0x00, sizeof(DeviceConfig.MobileNumber[i]));
		// strncpy(DeviceConfig.MobileNumber[i],StoredPhoneNumber[i],MASTER_MOBILE_NUMBER_LEN);
		strncpy(DeviceConfig.MobileNumber[i], StoredPhoneNumber[i], strlen(StoredPhoneNumber[i]));
	}
	for (i = 0; i < 15; i++)
	{
		// memset(DeviceConfig.MobileNumber[i], 0x00, sizeof(DeviceConfig.MobileNumber[i]));
		// strncpy(DeviceConfig.MobileNumber[i],StoredPhoneNumber[i],MASTER_MOBILE_NUMBER_LEN);
		memset(DeviceConfig.MobileSmscode[i], 0x00, sizeof(DeviceConfig.MobileSmscode[i]));
		// strncpy(DeviceConfig.MobileSmscode[i],StoredPhoneSmscode[i],MASTER_MOBILE_SMSCODE_LEN);
		strncpy(DeviceConfig.MobileSmscode[i], StoredPhoneSmscode[i], strlen(StoredPhoneSmscode[i]));

		// memset(DeviceConfig.MobileCallcode[i], 0x00, sizeof(DeviceConfig.MobileCallcode[i]));
		// strncpy(DeviceConfig.MobileCallcode[i],StoredPhoneCallcode[i],MASTER_MOBILE_CALLCODE_LEN);
	}
	/*for(j=0;j<2;i++)
	{
	i=j+10;
		memset(DeviceConfig.MobileNumber1[i], 0x00, sizeof(DeviceConfig.MobileNumber1[i]));
		strncpy(DeviceConfig.MobileNumber1[i],StoredPhoneNumber1[i],MASTER_MOBILE_NUMBER_LEN);
		memset(DeviceConfig.MobileSmscode1[i], 0x00, sizeof(DeviceConfig.MobileSmscode1[i]));
		strncpy(DeviceConfig.MobileSmscode1[i],StoredPhoneSmscode1[i],MASTER_MOBILE_SMSCODE_LEN);
		memset(DeviceConfig.MobileCallcode1[i], 0x00, sizeof(DeviceConfig.MobileCallcode1[i]));
		strncpy(DeviceConfig.MobileCallcode1[i],StoredPhoneCallcode1[i],MASTER_MOBILE_CALLCODE_LEN);
	}*/
	app_nvram_save(MCONFIG_AT_INDEX, (UINT8 *)&DeviceConfig, sizeof(DeviceConfig));
	printParameter();

#else
	INT32 ret;
	UINT32 writeedlen;
	// char Fname[]="sp.txt";
	strcpy((char *)pfile, (char *)"c:/sp.txt");
	file_hdl = sAPI_fopen((UINT8 *)pfile, "wb");
	if (file_hdl == NULL)
	{
		sAPI_UartPrintf("sAPI_fopen err");
	}
	sprintf(buf, "FileOpenEx Create %s\r\n", pfile);
	sAPI_UartPrintf(buf);

	memset(textBuf, 0, 500);
	ret = sAPI_fseek(file_hdl, 0, FS_SEEK_BEGIN);
	sprintf(buf, "FileSeek()=%d: \r\n", ret);
	sAPI_UartPrintf(buf);

	memset(textBuf, 0, 500);
	sprintf(textBuf, "Ph1,%s,\n,Ph2,%s,\n,Ph3,%s,\n,Ph4,%s,\n\r,Ph5,%s,%s,%s,%s,%s,%s,%s,%s,\n",
			StoredPhoneNumber[0], StoredPhoneNumber[1], StoredPhoneNumber[2], StoredPhoneNumber[3], StoredPhoneNumber[4], StoredPhoneNumber[5], StoredPhoneNumber[6], StoredPhoneNumber[7], StoredPhoneNumber[8], StoredPhoneNumber[9], StoredPhoneNumber[11], StoredPhoneNumber[12]);
	// sAPI_Debug((UINT8 *)textBuf,strlen(textBuf));
	sAPI_UartPrintf(textBuf);
	writeedlen = sAPI_fwrite((UINT8 *)textBuf, strlen((char *)textBuf), 1, file_hdl);
	sprintf(buf, "File Write success writeedlen=%d\r\n", writeedlen);
	sAPI_UartPrintf(buf);

	sAPI_fclose(file_hdl);
	sprintf(buf, "sAPI_fclose()\r\n");
	sAPI_UartPrintf(buf);

#endif
}

void ReadPhoneNumber(void)
{
#ifdef Store_Not_Txt_File
	char i;
	sAPI_UartPrintf("#ifdef Store_Not_Txt_File");
	for (i = 0; i < 19; i++)
	{
		memset(StoredPhoneNumber[i], 0x00, sizeof(StoredPhoneNumber[i]));
		// strncpy(StoredPhoneNumber[i],DeviceConfig.MobileNumber[i],MASTER_MOBILE_NUMBER_LEN);
		strncpy(StoredPhoneNumber[i], DeviceConfig.MobileNumber[i], strlen(DeviceConfig.MobileNumber[i]));
		if (strlen(StoredPhoneNumber[i]) < 3)
			strcpy(StoredPhoneNumber[i], "0000000000");
	}
	for (i = 0; i < 15; i++)
	{
		// strncpy(StoredPhoneNumber[i],DeviceConfig.MobileNumber[i],MASTER_MOBILE_NUMBER_LEN);
		// strncpy(StoredPhoneSmscode[i],DeviceConfig.MobileSmscode[i],MASTER_MOBILE_SMSCODE_LEN);
		memset(StoredPhoneSmscode[i], 0x00, sizeof(StoredPhoneSmscode[i]));
		strncpy(StoredPhoneSmscode[i], DeviceConfig.MobileSmscode[i], strlen(DeviceConfig.MobileSmscode[i]));

		// strncpy(StoredPhoneCallcode[i],DeviceConfig.MobileCallcode[i],MASTER_MOBILE_CALLCODE_LEN);
		// if(strlen(StoredPhoneNumber[i])<5)
		// strcpy(StoredPhoneNumber[i]),"0000000000";
	}
	printParameter();
#else
	//	char StrTokStr[30][20]; //dg_commented_memory
	int StrTokStrVer = 0;
	char *Pch = NULL;
	int Tp;

	INT32 ret;
	UINT32 readedlen;
	strcpy((char *)pfile, (char *)"c:/sp.txt");
	file_hdl = sAPI_fopen((UINT8 *)pfile, "ab");
	if (file_hdl == NULL)
	{
		sAPI_UartPrintf("sAPI_fopen err");
	}
	sprintf(buf, "FileOpenEx Create (%s,%08x)=%d\r\n", pfile, "ab", ret);
	sAPI_UartPrintf(buf);

	memset(textBuf, 0, 500);
	ret = sAPI_fseek(file_hdl, 0, FS_SEEK_BEGIN);
	sprintf(buf, "FileSeek()=%d: \r\n", ret);
	sAPI_UartPrintf(buf);

	memset(strBuf, 0, 200);
	readedlen = sAPI_fread((unsigned char *)strBuf, 200, 1, file_hdl);
	sprintf(buf, "FileRead()=%d: readedlen=%d, strBuf=\n\r%s", ret, readedlen, strBuf);
	sAPI_UartPrintf(buf);

	if (readedlen < 10)
	{
		sprintf(StoredPhoneNumber[0], "0000000000");
		sprintf(StoredPhoneNumber[1], "0000000000");
		sprintf(StoredPhoneNumber[2], "0000000000");
		sprintf(StoredPhoneNumber[3], "0000000000");
		sprintf(StoredPhoneNumber[4], "0000000000");
		sprintf(StoredPhoneNumber[5], "0000000000");
		sprintf(StoredPhoneNumber[6], "0000000000");
		sprintf(StoredPhoneNumber[7], "0000000000");
		sprintf(StoredPhoneNumber[8], "0000000000");
		sprintf(StoredPhoneNumber[9], "0000000000");
		sprintf(StoredPhoneNumber[11], "0000000000");
		sprintf(StoredPhoneNumber[12], "0000000000");
	}
	else
	{
		Pch1 = strtok((char *)strBuf, (char *)",");
		StrTokStrVer = 0;
		while (Pch1 != NULL)
		{
			strcpy(StrTokStr1[StrTokStrVer], Pch1);
			// sprintf(buf,"StrTokStr1[StrTokStrVer]=%s StrTokStrVer = %d",StrTokStr1[StrTokStrVer],StrTokStrVer);
			// sAPI_UartPrintf(buf);
			StrTokStrVer++;
			Pch1 = strtok(NULL, ",");
		}

		/*pos=0 St=Ph1pos=1 St=0000000000
		pos=2 St=
		pos=3 St=Ph2pos=4 St=0000000000
		pos=5 St=
		pos=6 St=Ph3pos=7 St=0000000000
		pos=8 St=
		pos=9 St=Ph4pos=10 St=0000000000

		pos=11 St=

		pos=12 St=Ph5pos=13 St=
		pos=14 St=l*/

		if (strlen(StrTokStr1[1]) < 8)
			strcpy(StoredPhoneNumber[0], "0000000000");
		else
			strcpy(StoredPhoneNumber[0], StrTokStr1[1]);
		if (strlen(StrTokStr1[4]) < 8)
			strcpy(StoredPhoneNumber[1], "0000000000");
		else
			strcpy(StoredPhoneNumber[1], StrTokStr1[4]);
		if (strlen(StrTokStr1[7]) < 8)
			strcpy(StoredPhoneNumber[2], "0000000000");
		else
			strcpy(StoredPhoneNumber[2], StrTokStr1[7]);
		if (strlen(StrTokStr1[10]) < 8)
			strcpy(StoredPhoneNumber[3], "0000000000");
		else
			strcpy(StoredPhoneNumber[3], StrTokStr1[10]);
		if (strlen(StrTokStr1[13]) < 8)
			strcpy(StoredPhoneNumber[4], "0000000000");
		else
			strcpy(StoredPhoneNumber[4], StrTokStr1[13]);
		if (strlen(StrTokStr1[14]) < 3)
			strcpy(StoredPhoneNumber[5], "0000000000");
		else
			strcpy(StoredPhoneNumber[5], StrTokStr1[14]);

		if (strlen(StrTokStr1[15]) < 8)
			strcpy(StoredPhoneNumber[6], "0000000000");
		else
			strcpy(StoredPhoneNumber[6], StrTokStr1[15]);
		if (strlen(StrTokStr1[16]) < 8)
			strcpy(StoredPhoneNumber[7], "0000000000");
		else
			strcpy(StoredPhoneNumber[7], StrTokStr1[16]);
		if (strlen(StrTokStr1[17]) < 8)
			strcpy(StoredPhoneNumber[8], "0000000000");
		else
			strcpy(StoredPhoneNumber[8], StrTokStr1[17]);
		if (strlen(StrTokStr1[18]) < 8)
			strcpy(StoredPhoneNumber[9], "0000000000");
		else
			strcpy(StoredPhoneNumber[9], StrTokStr1[18]);
		if (strlen(StrTokStr1[19]) < 8)
			strcpy(StoredPhoneNumber[11], "0000000000");
		else
			strcpy(StoredPhoneNumber[11], StrTokStr1[19]);
		if (strlen(StrTokStr1[20]) < 8)
			strcpy(StoredPhoneNumber[12], "0000000000");
		else
		{
			strcpy(StoredPhoneNumber[12], StrTokStr1[20]);
			strcpy(ServiceNumber, StrTokStr1[20]);
			ServiceNumberFound = 1;
		}
	}
	sprintf(buf, "Ph1,%s,\n,Ph2,%s,\n,Ph3,%s,\n,Ph4,%s,\n\r,Ph5,%s,%s,\n\r", StoredPhoneNumber[0], StoredPhoneNumber[1], StoredPhoneNumber[2], StoredPhoneNumber[3], StoredPhoneNumber[4], StoredPhoneNumber[5]);
	sAPI_UartPrintf(buf);
	sprintf(buf, "Ph6,%s,\n,Ph7,%s,\n,Ph8,%s,\n,Ph9,%s,%s,\n\r", StoredPhoneNumber[6], StoredPhoneNumber[7], StoredPhoneNumber[8], StoredPhoneNumber[9], StoredPhoneNumber[11]);
	sAPI_UartPrintf(buf);

	ret = sAPI_fclose(file_hdl);
	if (ret != 0)
	{
		sAPI_UartPrintf("sAPI_fclose err");
	}
	else
	{
		file_hdl = NULL;
	}

#endif
}
void WriteSettingsFile(void)
{
	INT32 ret;
	UINT32 writeedlen;
	SCFILE *file_hdl = NULL;
	// SCfileHandle fileHandle = {0};
	strcpy((char *)pfile, (char *)"c:/st.txt");
	file_hdl = sAPI_fopen((UINT8 *)pfile, "wb");
	if (file_hdl == NULL)
	{
		sAPI_UartPrintf("sAPI_fopen err");
	}
	sprintf(textBuf, "FileOpenEx Create (%s,%08x)=%d\r\n", pfile, "wb", ret);
	sAPI_UartPrintf(textBuf);

	memset(textBuf, 0, 500);
	ret = sAPI_fseek(file_hdl, 0, FS_SEEK_BEGIN);
	sprintf(textBuf, "FileSeek()=%d: \r\n", ret);
	sAPI_UartPrintf(textBuf);

	memset(textBuf, 0, 500);
	sprintf(textBuf, "SMNF,%d,%d,%d,%d,%d,\n\rSUNF,%d,\n\rDRNF,%d,\n\rTRNF,%d,\n\rVBNF,%d,\n\rCANF,%d,\n\rSCNF,%d,%d,%03d,%d,%d,%d,%02d,%02d,%02d,,%02d,%03d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\n\r", nMSettings.SMSOnOff,
			nMSettings.PressureOnOff, nMSettings.SfbOnOff,
			nMSettings.ManualswitchOnOff, nMSettings.TankOnOff,
			nMSettings.SumpOnOff, nMSettings.DryRunOnOff,
			nMSettings.TargetOnOff, nMSettings.VBFOnOff,
			nMSettings.RelayControlOnCall, nMSettings.SecOnOf,
			nTimerSettings.pfcOnOff, nTimerSettings.pfcvolt,
			Mobile2Phs3Phase, nMSettings.gethidesmsonoff,
			nMSettings.staticSMSOnOff, nTimerSettings.SfbHr,
			nTimerSettings.SfbMin, nTimerSettings.SfbSec,
			limitsmsonof, limitsmsset,
			nMSettings.serviceOnOff, nMSettings.pwrvfbOnOf,
			DNDSMSFLAG[0], DNDSMSFLAG[1], DNDSMSFLAG[2], DNDSMSFLAG[3], nMSettings.sampleSMSOnOff, nMSettings.dataSMSOnOff, nMSettings.canSMSOnOff, nMSettings.SumprstOnOff);

	writeedlen = sAPI_fwrite((UINT8 *)textBuf, strlen((char *)textBuf), 1, file_hdl);
	if (writeedlen != strlen((char *)textBuf))
	{
		sAPI_UartPrintf("sAPI_fwrite err write length: %d\r\n");
	}
	else
	{
		sprintf(buf, "FileWrite,writeedlen=%d\r\n", writeedlen);
		sAPI_UartPrintf(buf);
	}

	ret = sAPI_fclose(file_hdl);
	if (ret != 0)
	{
		sAPI_UartPrintf("sAPI_fclose err");
	}
	else
	{
		file_hdl = NULL;
	}
}

void Delete_Settings_files(void)
{
	char dir_name[20] = {0};
	int ret;

	memset(dir_name, 0, 20);
	memcpy(dir_name, "c:/1", strlen("c:/1"));

	ret = sAPI_remove(dir_name);
	if (ret != 0)
		sAPI_UartPrintf("\r\nFILE SYSTEM REMOVE c:/1 failed\r\n");
	else
		sAPI_UartPrintf("\r\nFILE SYSTEM REMOVE c:/1 SUCCESS\r\n");

	memset(dir_name, 0, 20);
	memcpy(dir_name, "c:/st.txt", strlen("c:/st.txt"));

	ret = sAPI_remove(dir_name);
	if (ret != 0)
		sAPI_UartPrintf("\r\nFILE SYSTEM REMOVE c:/1 failed\r\n");
	else
		sAPI_UartPrintf("\r\nFILE SYSTEM REMOVE c:/1 SUCCESS\r\n");

	memset(dir_name, 0, 20);
	memcpy(dir_name, "c:/stt.txt", strlen("c:/stt.txt"));

	ret = sAPI_remove(dir_name);
	if (ret != 0)
		sAPI_UartPrintf("\r\nFILE SYSTEM REMOVE c:/1 failed\r\n");
	else
		sAPI_UartPrintf("\r\nFILE SYSTEM REMOVE c:/1 SUCCESS\r\n");
}

void timerRoutine1(UINT32 argv)
{
	sAPI_Debug("timerRoutine1 ");

#ifdef USE_FLAG

	sAPI_FlagSet(g_flg1, TIMER1_OUT_EVENT_MASK, SC_FLAG_OR);

#endif
}
unsigned int myatoi(char *s) // no_notadded
{
	unsigned int a = 0;
	char c;
	while ((c = *s++) != '\0')
	{
		if (c >= '0' && c <= '9')
			a = a * 10 + (c - '0');
	}
	return a;
}

double myatof(char *s) // no_notadded
{
	double a = 0.0;
	int e = 0;
	int c;
	while ((c = *s++) != '\0')
	{
		if (c >= '0' && c <= '9')
			a = a * 10.0 + (c - '0');
		else
			break;
	}
	if (c == '.')
	{
		while ((c = *s++) != '\0')
		{
			if (c >= '0' && c <= '9')
			{
				a = a * 10.0 + (c - '0');
				e = e - 1;
			}
			else
				break;
		}
	}
	if (c == 'e' || c == 'E')
	{
		int sign = 1;
		int i = 0;
		c = *s++;
		if (c == '+')
			c = *s++;
		else if (c == '-')
		{
			c = *s++;
			sign = -1;
		}
		while (c >= '0' && c <= '9')
		{
			i = i * 10 + (c - '0');
			c = *s++;
		}
		e += i * sign;
	}
	while (e > 0)
	{
		a *= 10.0;
		e--;
	}
	while (e < 0)
	{
		a *= 0.1;
		e++;
	}
	return a;
}

void ReadEcosetFile()
{

//	char StrTokStr[30][20];   // dg_changed from [30][20]
	int StrTokStrVer = 0;
	char *Pch = NULL;
	int Tp;
    SCFILE *file_hdl = NULL;
	INT32 ret;
    UINT32 readedlen;
	unsigned char filename[100],l_pumpno_tx;
	 memset(pfile,0,200);

sprintf(filename,"c:/ecoset.txt");
	strcpy((char *)pfile,(char*)filename);

//	strcpy((char *)pfile,(char*)"c:/vset.txt");

	file_hdl = sAPI_fopen((UINT8*)pfile,"ab");
                if(file_hdl == NULL)
                {
                 //   sAPI_UartPrintf("sAPI_fopen err :c:/rtcset%d.txt",f_pumpno);  
					sprintf(buf,"sAPI_fopen err :c:/ecoset.txt");
					sAPI_UartPrintf(buf);
                }		
			else
				{
		memset(textBuf,0x00,50);
		ret = sAPI_fseek(file_hdl, 0 , FS_SEEK_BEGIN);
		sprintf(buf,"c:/ecoset.txt FileSeek()=%d: \r\n",ret);
		sAPI_UartPrintf(buf);
		memset(strBuf,0x00,140);  //70
		 readedlen = sAPI_fread((unsigned char *)strBuf,140, 1, file_hdl);
                if(readedlen <= 0)
                {
                    sprintf(buf,"sAPI_FsFileRead err,data: %s, len: %d", strBuf, readedlen);
                    sAPI_UartPrintf(buf);                     
                }	
		
		
		 if(readedlen<5)
		{
	//	g_no_of_pumps=1;
		}
		else
		{
		sprintf(buf,"c:/ecoset.txt FileRead()=%d: readedlen=%d, strBuf=%s\r\n",ret, readedlen, strBuf);
		sAPI_UartPrintf(buf); 
			Pch = strtok((char *)strBuf, (char *)"," );
			StrTokStrVer = 0;
			while( Pch != NULL )
			{
				strcpy(StrTokStr1[StrTokStrVer],Pch);
				StrTokStrVer++;
				Pch = strtok( NULL, "," );
			}

			for(int i=0,Tp=1;Tp<(StrTokStrVer-2);i++)
			{
				nConstant[i].Sno=myatoi(StrTokStr1[Tp++]);
				nConstant[i].Object=myatoi(StrTokStr1[Tp++]);
				nConstant[i].FlowRate=myatof(StrTokStr1[Tp++]);
				nConstant[i].LiterPerPluse=myatof(StrTokStr1[Tp++]);
				nConstant[i].Pressure=myatof(StrTokStr1[Tp++]);
				
				sprintf(Buffer1,"EcoConstant[%d]: %d,%d,%f,%f,%f\n\r",i,nConstant[i].Sno,nConstant[i].Object,nConstant[i].FlowRate,nConstant[i].LiterPerPluse,nConstant[i].Pressure);
				sAPI_UartPrintf(Buffer1);
			}
			NoOfConstant=myatoi(StrTokStr1[StrTokStrVer-2]);

			for(Tp=0;Tp<=StrTokStrVer;Tp++)
			{
				sprintf(buf,"\n\rSt=%s  pos=%d=%d\n\r",StrTokStr1[Tp],Tp,StrTokStrVer);
			sAPI_UartPrintf(buf);

			}
			
			// 	s_nMSettings.m_ValveTimerHr[0]=myatoi(StrTokStr1[1]);
			// 	s_nMSettings.m_ValveTimerMin[0]=myatoi(StrTokStr1[2]);
			// 	s_nMSettings.m_ValveTimerHr[1]=myatoi(StrTokStr1[3]);
			// 	s_nMSettings.m_ValveTimerMin[1]=myatoi(StrTokStr1[4]);
			// 	s_nMSettings.m_ValveTimerHr[2]=myatoi(StrTokStr1[5]);
			// 	s_nMSettings.m_ValveTimerMin[2]=myatoi(StrTokStr1[6]);
			// 	s_nMSettings.m_ValveTimerHr[3]=myatoi(StrTokStr1[7]);
			// 	s_nMSettings.m_ValveTimerMin[3]=myatoi(StrTokStr1[8]);
			// 	s_nMSettings.m_ValveTimerHr[4]=myatoi(StrTokStr1[9]);
			// 	s_nMSettings.m_ValveTimerMin[4]=myatoi(StrTokStr1[10]);
			// 	s_nMSettings.m_ValveTimerHr[5]=myatoi(StrTokStr1[11]);
			// 	s_nMSettings.m_ValveTimerMin[5]=myatoi(StrTokStr1[12]);
			// 	s_nMSettings.m_ValveTimerHr[6]=myatoi(StrTokStr1[13]);
			// 	s_nMSettings.m_ValveTimerMin[6]=myatoi(StrTokStr1[14]);
			// 	s_nMSettings.m_ValveTimerHr[7]=myatoi(StrTokStr1[15]);
			// 	s_nMSettings.m_ValveTimerMin[7]=myatoi(StrTokStr1[16]);
			// 	s_nMSettings.m_ValveTimerHr[9]=myatoi(StrTokStr1[17]);
			// 	s_nMSettings.m_ValveTimerMin[8]=myatoi(StrTokStr1[18]);
			// 	s_nMSettings.m_ValveTimerHr[9]=myatoi(StrTokStr1[19]);
			// 	s_nMSettings.m_ValveTimerMin[9]=myatoi(StrTokStr1[20]); 
			// 	s_nMSettings.ValveOnOff=myatoi(StrTokStr1[21]);
			// 	s_nMSettings.CyclicLimit=myatoi(StrTokStr1[22]);
			// 	s_nMSettings.CycLicIntervelHr=myatoi(StrTokStr1[23]);
			// 	s_nMSettings.CycLicIntervelMin=myatoi(StrTokStr1[24]);
			// 	s_nMSettings.CyclicOnOff=myatoi(StrTokStr1[25]);
				
			// 	PresSenFlag=myatoi(StrTokStr1[26]);
			// PreSenTime[0]=myatoi(StrTokStr1[27]);
			// PreSenTime[1]=myatoi(StrTokStr1[28]);
			// PreSenTime[2]=myatoi(StrTokStr1[29]);
			// HighFlow=myatof(StrTokStr1[30]);
			// LowFlow=myatof(StrTokStr1[31]);
			// PreSwFlag=myatoi(StrTokStr1[32]);
			// PreSwTime[0]=myatoi(StrTokStr1[33]);
			// PreSwTime[1]=myatoi(StrTokStr1[34]);
			// PreSwTime[2]=myatoi(StrTokStr1[35]);
			//PresSenFlag,PreSenTime[0],PreSenTime[1],PreSenTime[2],HighFlow,LowFlow,PreSwFlag,PreSwTime[0],PreSwTime[1],PreSwTime[2]
			 /* sprintf(buf,"$T,S,%d,%d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,\n\r",
											 l_pumpno_tx,s_nMSettings.m_RTCOnOf[l_pumpno_tx],s_nTimerSettings.m_RTCONHr[l_pumpno_tx][1],s_nTimerSettings.m_RTCONMin[l_pumpno_tx][1],s_nTimerSettings.m_RTCONSec[l_pumpno_tx][1],
											 s_nTimerSettings.m_RTCOfHr[l_pumpno_tx][1],s_nTimerSettings.m_RTCOfMin[l_pumpno_tx][1],s_nTimerSettings.m_RTCOfSec[l_pumpno_tx][1],s_nTimerSettings.m_RTCONHr[l_pumpno_tx][2],
											 s_nTimerSettings.m_RTCONMin[l_pumpno_tx][2],s_nTimerSettings.m_RTCONSec[l_pumpno_tx][2],s_nTimerSettings.m_RTCOfHr[l_pumpno_tx][2],s_nTimerSettings.m_RTCOfMin[l_pumpno_tx][2],
											 s_nTimerSettings.m_RTCOfSec[l_pumpno_tx][2],s_nTimerSettings.m_RTCONHr[l_pumpno_tx][3],s_nTimerSettings.m_RTCONMin[l_pumpno_tx][3],s_nTimerSettings.m_RTCONSec[l_pumpno_tx][3],
											 s_nTimerSettings.m_RTCOfHr[l_pumpno_tx][3],s_nTimerSettings.m_RTCOfMin[l_pumpno_tx][3],s_nTimerSettings.m_RTCOfSec[l_pumpno_tx][3]);
											 sAPI_UartPrintf(buf); */

			} 
  ret = sAPI_fclose(file_hdl);
                 if(ret != 0)
                {
                 //   sAPI_UartPrintf("c:/rtcset%d.txt sAPI_fclose err",f_pumpno);
					sprintf(buf,"c:/ecoset.txt sAPI_fclose err");
                    sAPI_UartPrintf(buf);   
                   
                }else{
                    file_hdl = NULL;
                }

				

				}
	 
}




void WriteEcosetFile()
{
	INT32 ret;
    UINT32 writeedlen;
    SCFILE *file_hdl = NULL;
	unsigned char filename[100],l_pumpno_tx;
	
	sprintf(filename,"c:/ecoset.txt");
	strcpy((char *)pfile,(char*)filename);
	 
	 file_hdl = sAPI_fopen((UINT8*)pfile, "wb");
                if(file_hdl == NULL)
                {
                    sAPI_UartPrintf("sAPI_fopen err");
                    
                } 
 	 
	sprintf(textBuf,"FileOpenEx Create (%s,%08x)=%d\r\n", pfile, FS_CREATE,ret);
	sAPI_UartPrintf(textBuf);

	memset(textBuf,0x00,100);  //55
	ret = sAPI_fseek(file_hdl, 0 , FS_SEEK_BEGIN);
	sprintf(textBuf,"FileSeek()=%d: \r\n",ret);
	sAPI_UartPrintf(textBuf);


   memset(textBuf,0x00,140);	//50
sprintf(textBuf,"Ecoset,");
for(int i=0;i<NoOfConstant;i++)
{
	//sprintf(textBuf,"%s%02d,%02d,%04d,\n\r",textBuf,nConstant[i].Sno,nConstant[i].Object,nConstant[i].FlowRate);
		sprintf(textBuf,"%s%02d,%02d,%.01f,%.01f,%.01f,",textBuf,nConstant[i].Sno,nConstant[i].Object,nConstant[i].FlowRate,nConstant[i].LiterPerPluse,nConstant[i].Pressure);
}
sprintf(textBuf,"%s%02d,\n\r",textBuf,NoOfConstant);
	// sprintf(textBuf,"Ecoset,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%01d,%02d,%02d,%02d,%01d,%02d,%02d,%02d,%.01f,%.01f,%01d,%02d,%02d,%02d,\n\r",
	// s_nMSettings.m_ValveTimerHr[0],s_nMSettings.m_ValveTimerMin[0],s_nMSettings.m_ValveTimerHr[1],s_nMSettings.m_ValveTimerMin[1],s_nMSettings.m_ValveTimerHr[2],s_nMSettings.m_ValveTimerMin[2],
	//  s_nMSettings.m_ValveTimerHr[3],s_nMSettings.m_ValveTimerMin[3],s_nMSettings.m_ValveTimerHr[4],s_nMSettings.m_ValveTimerMin[4],s_nMSettings.m_ValveTimerHr[5],s_nMSettings.m_ValveTimerMin[5],
	//  s_nMSettings.m_ValveTimerHr[6],s_nMSettings.m_ValveTimerMin[6],s_nMSettings.m_ValveTimerHr[7],s_nMSettings.m_ValveTimerMin[7],s_nMSettings.m_ValveTimerHr[8],s_nMSettings.m_ValveTimerMin[8],
	//  s_nMSettings.m_ValveTimerHr[9],s_nMSettings.m_ValveTimerMin[9],s_nMSettings.ValveOnOff,s_nMSettings.CyclicLimit,s_nMSettings.CycLicIntervelHr,s_nMSettings.CycLicIntervelMin,s_nMSettings.CyclicOnOff,
	//  PresSenFlag,PreSenTime[0],PreSenTime[1],PreSenTime[2],HighFlow,LowFlow,PreSwFlag,PreSwTime[0],PreSwTime[1],PreSwTime[2]);
     sAPI_UartPrintf(textBuf);
	 
	 writeedlen = sAPI_fwrite((UINT8*)textBuf, strlen((char*)textBuf),1,file_hdl);
                if(writeedlen != strlen((char*)textBuf))
                {
                    sprintf(buf,"sAPI_fwrite err write length: %d\r\n", writeedlen);
					sAPI_UartPrintf(buf);                    
                }
	sprintf(textBuf,"FileWrite{%s},writeedlen=%d\r\n",pfile,writeedlen);
	sAPI_UartPrintf(textBuf);

	  ret = sAPI_fclose(file_hdl);
                if(ret != 0)
                {
                    sAPI_UartPrintf("sAPI_fclose err");
                   
                }else{
                    file_hdl = NULL;
                }

}
void ReadtnkconfigFile(void)
{

	//	char StrTokStr[30][20];   // dg_changed from [30][20]
	int StrTokStrVer = 0;
	char *Pch = NULL;
	int Tp;
	SCFILE *file_hdl = NULL;
	INT32 ret;
	UINT32 readedlen;

	ret = sAPI_GetSize("c:");
	sprintf(buf, "size=%d: \r\n", ret);
	ret = sAPI_GetFreeSize("c:");
	sprintf(buf, "free size=%d: \r\n", ret);
	sAPI_UartPrintf(buf);
	//	strcpy((char *)pfile,(char*)"c:/tset.txt");
	strcpy((char *)pfile, (char *)"c:/tset.txt");

	file_hdl = sAPI_fopen((UINT8 *)pfile, "ab");
	if (file_hdl == NULL)
	{
		sAPI_UartPrintf("sAPI_fopen err :c:/tset.txt");
	}
	memset(textBuf, 0, 50);
	ret = sAPI_fseek(file_hdl, 0, FS_SEEK_BEGIN);
	sprintf(buf, "c:/tset.txt FileSeek()=%d: \r\n", ret);
	sAPI_UartPrintf(buf);
	memset(strBuf, 0, 100); // 70
	readedlen = sAPI_fread((unsigned char *)strBuf, 100, 1, file_hdl);
	if (readedlen <= 0)
	{
		sprintf(buf, "sAPI_FsFileRead err,len: %d", readedlen);
		sAPI_UartPrintf(buf);
	}
	sprintf(buf, "c:/tset.txt FileRead()=%d: readedlen=%d, strBuf=%s\r\n", ret, readedlen, strBuf);
	sAPI_UartPrintf(buf);

	if (readedlen < 5)
	{
		g_no_of_pumps = 1;
	}
	else
	{

		Pch = strtok((char *)strBuf, (char *)",");
		StrTokStrVer = 0;
		while (Pch != NULL)
		{
			strcpy(StrTokStr1[StrTokStrVer], Pch);
			StrTokStrVer++;
			Pch = strtok(NULL, ",");
		}

		for(Tp=1;Tp<StrTokStrVer-2;Tp=Tp+4)
		{
			nConfig[Tp/4].Sno = myatoi(StrTokStr1[Tp]);
			nConfig[Tp/4].Object = myatoi(StrTokStr1[Tp+1]);
			nConfig[Tp/4].Output_No = myatoi(StrTokStr1[Tp+2]);
			nConfig[Tp/4].Input_No = myatoi(StrTokStr1[Tp+3]);
			sprintf(buf,"nConfig[%d]=%d,%d,%d,%d\n\r",Tp/4,nConfig[Tp/4].Sno,nConfig[Tp/4].Object,nConfig[Tp/4].Input_No,nConfig[Tp/4].Output_No);
			sAPI_UartPrintf(buf);
		}
		NoOfObject=myatoi(StrTokStr1[StrTokStrVer-2]);
		
		for(Tp=0;Tp<=StrTokStrVer;Tp++)
		{
			sprintf(buf,"\n\rSt=%s  pos=%d=%d\n\r",StrTokStr1[Tp],Tp,StrTokStrVer);
		 sAPI_UartPrintf(buf);

		}
		// g_no_of_pumps = myatoi(StrTokStr1[1]);
		// s_npump[0].m_no_of_sump_pins = myatoi(StrTokStr1[2]);
		// s_npump[0].m_sump_pin_no[0] = myatoi(StrTokStr1[3]);
		// s_npump[0].m_sump_pin_no[1] = myatoi(StrTokStr1[4]);
		// s_npump[0].m_no_of_tank_pins = myatoi(StrTokStr1[5]);
		// s_npump[0].m_tank_pin_no[0] = myatoi(StrTokStr1[6]);
		// s_npump[0].m_tank_pin_no[1] = myatoi(StrTokStr1[7]);
		// s_npump[0].m_Level_on_off = myatoi(StrTokStr1[8]);
		// s_npump[0].m_flowonof = myatoi(StrTokStr1[9]);
		// s_npump[0].m_pressureonof = myatoi(StrTokStr1[10]);

		// s_npump[1].m_no_of_sump_pins = myatoi(StrTokStr1[11]);
		// s_npump[1].m_sump_pin_no[0] = myatoi(StrTokStr1[12]);
		// s_npump[1].m_sump_pin_no[1] = myatoi(StrTokStr1[13]);
		// s_npump[1].m_no_of_tank_pins = myatoi(StrTokStr1[14]);
		// s_npump[1].m_tank_pin_no[0] = myatoi(StrTokStr1[15]);
		// s_npump[1].m_tank_pin_no[1] = myatoi(StrTokStr1[16]);
		// s_npump[1].m_Level_on_off = myatoi(StrTokStr1[17]);
		// s_npump[1].m_flowonof = myatoi(StrTokStr1[18]);
		// s_npump[1].m_pressureonof = myatoi(StrTokStr1[19]);

		// s_npump[2].m_no_of_sump_pins = myatoi(StrTokStr1[20]);
		// s_npump[2].m_sump_pin_no[0] = myatoi(StrTokStr1[21]);
		// s_npump[2].m_sump_pin_no[1] = myatoi(StrTokStr1[22]);
		// s_npump[2].m_no_of_tank_pins = myatoi(StrTokStr1[23]);
		// s_npump[2].m_tank_pin_no[0] = myatoi(StrTokStr1[24]);
		// s_npump[2].m_tank_pin_no[1] = myatoi(StrTokStr1[25]);
		// s_npump[2].m_Level_on_off = myatoi(StrTokStr1[26]);
		// s_npump[2].m_flowonof = myatoi(StrTokStr1[27]);
		// s_npump[2].m_pressureonof = myatoi(StrTokStr1[28]);
		// s_npump[0].m_Sump_on_off = myatoi(StrTokStr1[29]);
		// s_npump[0].m_Tank_on_off = myatoi(StrTokStr1[30]);
		// s_npump[1].m_Sump_on_off = myatoi(StrTokStr1[31]);
		// s_npump[1].m_Tank_on_off = myatoi(StrTokStr1[32]);
		// s_npump[2].m_Sump_on_off = myatoi(StrTokStr1[33]);
		// s_npump[2].m_Tank_on_off = myatoi(StrTokStr1[34]);
		// g_serialid = myatoi(StrTokStr1[35]);
		// a_Master_onoff[0] = myatoi(StrTokStr1[36]);
		// a_Master_onoff[1] = myatoi(StrTokStr1[37]);
		// a_Master_onoff[2] = myatoi(StrTokStr1[38]);
		// s_npump[0].m_Uppertank_restart_off = myatoi(StrTokStr1[39]);
		// s_npump[0].m_Lowertank_restart_off = myatoi(StrTokStr1[40]);
		// s_npump[1].m_Uppertank_restart_off = myatoi(StrTokStr1[41]);
		// s_npump[1].m_Lowertank_restart_off = myatoi(StrTokStr1[42]);
		// s_npump[2].m_Uppertank_restart_off = myatoi(StrTokStr1[43]);
		// s_npump[2].m_Lowertank_restart_off = myatoi(StrTokStr1[44]);
		if (nMSettings.ndebugonof)
		{

			sprintf(buf, "read_file nMSettings.motor4ctrlonof is %d", nMSettings.motor4ctrlonof);
			sAPI_UartPrintf(buf);
		}
	}
	ret = sAPI_fclose(file_hdl);
	if (ret != 0)
	{
		sAPI_UartPrintf("c:/tset.txt sAPI_fclose err");
	}
	else
	{
		file_hdl = NULL;
	}
	if (nMSettings.ndebugonof)
	{
		/* sprintf(buf,"$O,S,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\n\r",
		 g_no_of_pumps,s_nTimerSettings.m_ct_MotorNumber[0],s_nTimerSettings.m_ct_MotorNumber[1],s_nTimerSettings.m_ct_MotorNumber[2],pumpno,s_npump[0].m_Sump_on_off,
		 s_npump[0].m_no_of_sump_pins,s_npump[0].m_sump_pin_no[0],s_npump[0].m_sump_pin_no[1],s_npump[0].m_Tank_on_off,s_npump[0].m_no_of_tank_pins,s_npump[0].m_tank_pin_no[0],
		 s_npump[0].m_tank_pin_no[1],pumpno,s_npump[1].m_Sump_on_off,s_npump[1].m_no_of_sump_pins,s_npump[1].m_sump_pin_no[0],s_npump[1].m_sump_pin_no[1],s_npump[1].m_Tank_on_off,
		 s_npump[1].m_no_of_tank_pins,s_npump[1].m_tank_pin_no[0],s_npump[1].m_tank_pin_no[1]);

		 sAPI_UartPrintf(buf); */

		/* sprintf(buf,"$P,S,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\n\r",
		s_nMSettings.m_pumpno_send,s_npump[2].m_Sump_on_off,s_npump[2].m_no_of_sump_pins,
		s_npump[2].m_sump_pin_no[0],s_npump[2].m_sump_pin_no[1],s_npump[2].m_Tank_on_off,s_npump[2].m_no_of_tank_pins,s_npump[2].m_tank_pin_no[0],s_npump[2].m_tank_pin_no[1],
		s_npump[0].m_Level_on_off,s_npump[0].m_flowonof,s_npump[0].m_pressureonof,s_npump[1].m_Level_on_off,s_npump[1].m_flowonof,s_npump[1].m_pressureonof,
		s_npump[2].m_Level_on_off,s_npump[2].m_flowonof,s_npump[2].m_pressureonof);
	   sAPI_UartPrintf(buf); */
	}
}

void WritetnkconfigFile(void)
{
	INT32 ret;
	UINT32 writeedlen;
	SCFILE *file_hdl = NULL;
	//	strcpy((char *)pfile,(char*)"c:/tset.txt");
	strcpy((char *)pfile, (char *)"c:/tset.txt");

	file_hdl = sAPI_fopen((UINT8 *)pfile, "wb");
	if (file_hdl == NULL)
	{
		sAPI_UartPrintf("sAPI_fopen err");
	}

	sprintf(textBuf, "FileOpenEx Create (%s,%08x)=%d\r\n", pfile, FS_CREATE, ret);
	sAPI_UartPrintf(textBuf);

	memset(textBuf, 0, 100); // 55
	ret = sAPI_fseek(file_hdl, 0, FS_SEEK_BEGIN);
	sprintf(textBuf, "FileSeek()=%d: \r\n", ret);
	sAPI_UartPrintf(textBuf);

	memset(textBuf, 0, 80); // 100	//50
	sprintf(buf, " before write PrvAutoMobileKey is %d", PrvAutoMobileKey);
	sAPI_UartPrintf(buf);
	sprintf(textBuf, "Config,");
	for (int i = 0; i < NoOfObject; i++)
		sprintf(textBuf, "%s%02d,%02d,%01d,%01d,", textBuf, nConfig[i].Sno, nConfig[i].Object, nConfig[i].Input_No, nConfig[i].Output_No);
	sprintf(textBuf, "%s%02d,\n\r", textBuf,NoOfObject);
		sAPI_UartPrintf(textBuf);
	// sprintf(textBuf, "tnkset,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\n\r",
	// 		g_no_of_pumps, s_npump[0].m_no_of_sump_pins, s_npump[0].m_sump_pin_no[0], s_npump[0].m_sump_pin_no[1], s_npump[0].m_no_of_tank_pins, s_npump[0].m_tank_pin_no[0], s_npump[0].m_tank_pin_no[1], s_npump[0].m_Level_on_off, s_npump[0].m_flowonof, s_npump[0].m_pressureonof,
	// 		s_npump[1].m_no_of_sump_pins, s_npump[1].m_sump_pin_no[0], s_npump[1].m_sump_pin_no[1], s_npump[1].m_no_of_tank_pins, s_npump[1].m_tank_pin_no[0], s_npump[1].m_tank_pin_no[1], s_npump[1].m_Level_on_off, s_npump[1].m_flowonof, s_npump[1].m_pressureonof,
	// 		s_npump[2].m_no_of_sump_pins, s_npump[2].m_sump_pin_no[0], s_npump[2].m_sump_pin_no[1], s_npump[2].m_no_of_tank_pins, s_npump[2].m_tank_pin_no[0], s_npump[2].m_tank_pin_no[1], s_npump[2].m_Level_on_off, s_npump[2].m_flowonof, s_npump[2].m_pressureonof,
	// 		s_npump[0].m_Sump_on_off, s_npump[0].m_Tank_on_off, s_npump[1].m_Sump_on_off, s_npump[1].m_Tank_on_off, s_npump[2].m_Sump_on_off, s_npump[2].m_Tank_on_off, g_serialid, a_Master_onoff[0], a_Master_onoff[1], a_Master_onoff[2], s_npump[0].m_Uppertank_restart_off,
	// 		s_npump[0].m_Lowertank_restart_off, s_npump[1].m_Uppertank_restart_off, s_npump[1].m_Lowertank_restart_off, s_npump[2].m_Uppertank_restart_off, s_npump[2].m_Lowertank_restart_off);
	// 
		//sAPI_UartPrintf(textBuf);

	writeedlen = sAPI_fwrite((UINT8 *)textBuf, strlen((char *)textBuf), 1, file_hdl);
	if (writeedlen != strlen((char *)textBuf))
	{
		sprintf(buf, "sAPI_fwrite err write length: %d\r\n", writeedlen);
		sAPI_UartPrintf(buf);
	}
	sprintf(textBuf, "FileWrite{%s},writeedlen=%d\r\n", pfile, writeedlen);
	sAPI_UartPrintf(textBuf);

	ret = sAPI_fclose(file_hdl);
	if (ret != 0)
	{
		sAPI_UartPrintf("sAPI_fclose err");
	}
	else
	{
		file_hdl = NULL;
	}
}

void ReadvolsetFile(void)
{

	//	char StrTokStr[30][20];   // dg_changed from [30][20]
	int StrTokStrVer = 0;
	char *Pch = NULL;
	int Tp;
	SCFILE *file_hdl = NULL;
	INT32 ret;
	UINT32 readedlen;

	strcpy((char *)pfile, (char *)"c:/vset.txt");

	file_hdl = sAPI_fopen((UINT8 *)pfile, "ab");
	if (file_hdl == NULL)
	{
		sAPI_UartPrintf("sAPI_fopen err :c:/vset.txt");
	}
	memset(textBuf, 0, 50);
	ret = sAPI_fseek(file_hdl, 0, FS_SEEK_BEGIN);
	sprintf(buf, "c:/vset.txt FileSeek()=%d: \r\n", ret);
	sAPI_UartPrintf(buf);
	memset(strBuf, 0, 100); // 70
	readedlen = sAPI_fread((unsigned char *)strBuf, 100, 1, file_hdl);
	if (readedlen <= 0)
	{
		sprintf(buf, "sAPI_FsFileRead err,len: %d", readedlen);
		sAPI_UartPrintf(buf);
	}
	sprintf(buf, "c:/vset.txt FileRead()=%d: readedlen=%d, strBuf=%s\r\n", ret, readedlen, strBuf);
	sAPI_UartPrintf(buf);

	if (readedlen < 5)
	{
		//	g_no_of_pumps=1;
	}
	else
	{

		Pch = strtok((char *)strBuf, (char *)",");
		StrTokStrVer = 0;
		while (Pch != NULL)
		{
			strcpy(StrTokStr1[StrTokStrVer], Pch);
			StrTokStrVer++;
			Pch = strtok(NULL, ",");
		}
		NoOfZone=myatoi(StrTokStr1[1]);
		for(int i=0,Tp=2;i<=NoOfZone;i++)
		{
			nProgram.nZone[i].Sno=myatoi(StrTokStr1[Tp++]);
			nProgram.nZone[i].ValveNo[0]=myatoi(StrTokStr1[Tp++]);
			nProgram.nZone[i].ValveNo[1]=myatoi(StrTokStr1[Tp++]);
			nProgram.nZone[i].ValveNo[2]=myatoi(StrTokStr1[Tp++]);
			nProgram.nZone[i].ValveNo[3]=myatoi(StrTokStr1[Tp++]);
			nProgram.nZone[i].MainValve=myatoi(StrTokStr1[Tp++]);
			nProgram.nZone[i].IrrigationMethod=myatoi(StrTokStr1[Tp++]);
			nProgram.nZone[i].FlowRate=myatoi(StrTokStr1[Tp++]);
			nProgram.nZone[i].Duration[0]=myatoi(StrTokStr1[Tp++]);
			nProgram.nZone[i].Duration[1]=myatoi(StrTokStr1[Tp++]);
			nProgram.nZone[i].Duration[2]=myatoi(StrTokStr1[Tp++]);
			nProgram.nZone[i].ProgramNo=myatoi(StrTokStr1[Tp++]);

			sprintf(Buffer1,"Read Zone[%d]:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d.\n\r",i,nProgram.nZone[i].Sno,nProgram.nZone[i].ValveNo[0],nProgram.nZone[i].ValveNo[1],nProgram.nZone[i].ValveNo[2],nProgram.nZone[i].ValveNo[3],nProgram.nZone[i].MainValve,nProgram.nZone[i].IrrigationMethod,nProgram.nZone[i].FlowRate,nProgram.nZone[i].Duration[0],nProgram.nZone[i].Duration[1],nProgram.nZone[i].Duration[2],nProgram.nZone[i].ProgramNo);
			sAPI_UartPrintf(Buffer1);
		}

		for(Tp=0;Tp<=StrTokStrVer;Tp++)
		{
			sprintf(buf,"\n\rSt=%s  pos=%d\n\r",StrTokStr1[Tp],Tp);
		 sAPI_UartPrintf(buf);

		}
		// s_nMSettings.m_LowVoltOnOff = myatoi(StrTokStr1[1]);
		// s_nTimerSettings.m_LowVoltII = myatoi(StrTokStr1[2]);
		// s_nTimerSettings.m_DiffVoltII = myatoi(StrTokStr1[3]);
		// s_nTimerSettings.m_LowVoltIII = myatoi(StrTokStr1[4]);
		// s_nTimerSettings.m_DiffVoltIII = myatoi(StrTokStr1[5]);
		// s_nMSettings.m_HighVoltOnOff = myatoi(StrTokStr1[6]);
		// s_nTimerSettings.m_HighVoltII = myatoi(StrTokStr1[7]);
		// s_nTimerSettings.m_HiDiffVoltII = myatoi(StrTokStr1[8]);
		// s_nTimerSettings.m_HighVoltIII = myatoi(StrTokStr1[9]);
		// s_nTimerSettings.m_HiDiffVoltIII = myatoi(StrTokStr1[10]);
		// s_nMSettings.m_SppOnoff = myatoi(StrTokStr1[11]);
		// s_nTimerSettings.m_ImbVolt = myatoi(StrTokStr1[12]);
		// s_nMSettings.m_RvePhOnoff = myatoi(StrTokStr1[13]);
		// //		s_nMSettings.m_AutoStIIOnOff[0]=myatoi(StrTokStr1[14]);  ////array not needed
		// s_nMSettings.m_CurSppOnOff = myatoi(StrTokStr1[14]);
		// if (nMSettings.ndebugonof)
		// {

		// 	/* sprintf(buf,"$Q,S,%d,%03d,%03d,%03d,%03d,%d,%03d,%03d,%03d,%03d,%d,%02d,%d,%d,\n\r",
		// 									 s_nMSettings.m_LowVoltOnOff,s_nTimerSettings.m_LowVoltII,s_nTimerSettings.m_DiffVoltII,s_nTimerSettings.m_LowVoltIII,s_nTimerSettings.m_DiffVoltIII,
		// 									 s_nMSettings.m_HighVoltOnOff,s_nTimerSettings.m_HighVoltII,s_nTimerSettings.m_HiDiffVoltII,s_nTimerSettings.m_HighVoltIII,s_nTimerSettings.m_HiDiffVoltIII,
		// 									 s_nMSettings.m_SppOnoff,(int)s_nTimerSettings.m_ImbVolt,s_nMSettings.m_RvePhOnoff,s_nMSettings.m_AutoStIIOnOff[0]);

		// 									 sAPI_UartPrintf(buf); */
		// }
	}
	ret = sAPI_fclose(file_hdl);
	if (ret != 0)
	{
		sAPI_UartPrintf("c:/vset.txt sAPI_fclose err");
	}
	else
	{
		file_hdl = NULL;
	}
}

void WritevolsetFile(void)
{
	INT32 ret;
	UINT32 writeedlen;
	SCFILE *file_hdl = NULL;
	strcpy((char *)pfile, (char *)"c:/vset.txt");

	file_hdl = sAPI_fopen((UINT8 *)pfile, "wb");
	if (file_hdl == NULL)
	{
		sAPI_UartPrintf("sAPI_fopen err");
	}

	sprintf(textBuf, "FileOpenEx Create (%s,%08x)=%d\r\n", pfile, FS_CREATE, ret);
	sAPI_UartPrintf(textBuf);

	memset(textBuf, 0, 100); // 55
	ret = sAPI_fseek(file_hdl, 0, FS_SEEK_BEGIN);
	sprintf(textBuf, "FileSeek()=%d: \r\n", ret);
	sAPI_UartPrintf(textBuf);

	memset(textBuf, 0, 100); // 50
	sprintf(buf, " before write PrvAutoMobileKey is %d", PrvAutoMobileKey);
	sAPI_UartPrintf(buf);
	sprintf(textBuf,"Zone,%d,",NoOfZone);
	for(int i=0;i<NoOfZone;i++)
		sprintf(textBuf,"%s%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,",textBuf,nProgram.nZone[i].Sno,nProgram.nZone[i].ValveNo[0],nProgram.nZone[i].ValveNo[1],nProgram.nZone[i].ValveNo[2],nProgram.nZone[i].ValveNo[3],nProgram.nZone[i].MainValve,nProgram.nZone[i].IrrigationMethod,nProgram.nZone[i].FlowRate,nProgram.nZone[i].Duration[0],nProgram.nZone[i].Duration[1],nProgram.nZone[i].Duration[2],nProgram.nZone[i].ProgramNo);
	sAPI_UartPrintf(textBuf);
	
	// sprintf(textBuf, "tnkset,%d,%03d,%03d,%03d,%03d,%d,%03d,%03d,%03d,%03d,%d,%03d,%d,%d,\n\r",
	// 		s_nMSettings.m_LowVoltOnOff, s_nTimerSettings.m_LowVoltII, s_nTimerSettings.m_DiffVoltII, s_nTimerSettings.m_LowVoltIII, s_nTimerSettings.m_DiffVoltIII,
	// 		s_nMSettings.m_HighVoltOnOff, s_nTimerSettings.m_HighVoltII, s_nTimerSettings.m_HiDiffVoltII, s_nTimerSettings.m_HighVoltIII, s_nTimerSettings.m_HiDiffVoltIII,
	// 		s_nMSettings.m_SppOnoff, s_nTimerSettings.m_ImbVolt, s_nMSettings.m_RvePhOnoff, s_nMSettings.m_CurSppOnOff);
	// sAPI_UartPrintf(textBuf);

	writeedlen = sAPI_fwrite((UINT8 *)textBuf, strlen((char *)textBuf), 1, file_hdl);
	if (writeedlen != strlen((char *)textBuf))
	{
		sprintf(buf, "sAPI_fwrite err write length: %d\r\n", writeedlen);
		sAPI_UartPrintf(buf);
	}
	sprintf(textBuf, "FileWrite{%s},writeedlen=%d\r\n", pfile, writeedlen);
	sAPI_UartPrintf(textBuf);

	ret = sAPI_fclose(file_hdl);
	if (ret != 0)
	{
		sAPI_UartPrintf("sAPI_fclose err");
	}
	else
	{
		file_hdl = NULL;
	}
}

void ReadcursetFile(unsigned char f_pumpno)
{

	//	char StrTokStr[30][20];   // dg_changed from [30][20]
	int StrTokStrVer = 0;
	char *Pch = NULL;
	int Tp;
	SCFILE *file_hdl = NULL;
	INT32 ret;
	UINT32 readedlen, temp_OlAmpsII, temp_OlAmpsIII, temp_DrAmpsII, temp_DrAmpsIII;
	unsigned char filename[30], l_pumpno_tx = 0;
	memset(pfile, 0, 200);

	sprintf(filename, "c:/cset%d.txt", f_pumpno);
	strcpy((char *)pfile, (char *)filename);

	//	strcpy((char *)pfile,(char*)"c:/vset.txt");

	//	file_hdl = sAPI_fopen(pfile,"ab");
	file_hdl = sAPI_fopen((UINT8 *)pfile, "ab");
	if (file_hdl == NULL)
	{
		//    sAPI_UartPrintf("sAPI_fopen err :c:/cset%d.txt",f_pumpno);
		sprintf(buf, "sAPI_fopen err :c:/cset%d.txt", f_pumpno);
		sAPI_UartPrintf(buf);
	}
	else
	{
		memset(textBuf, 0, 50);
		ret = sAPI_fseek(file_hdl, 0, FS_SEEK_BEGIN);
		sprintf(buf, "c:/cset%d.txt FileSeek()=%d: \r\n", f_pumpno, ret);
		sAPI_UartPrintf(buf);
		memset(strBuf, 0, 100); // 70
		readedlen = sAPI_fread((unsigned char *)strBuf, 100, 1, file_hdl);
		if (readedlen <= 0)
		{
			sprintf(buf, "sAPI_FsFileRead err, len: %d", readedlen);
			sAPI_UartPrintf(buf);
		}

		if (readedlen < 5)
		{
			//	g_no_of_pumps=1;
		}
		else
		{
			sprintf(buf, "c:/cset%d.txt FileRead()=%d: readedlen=%d, strBuf=%s\r\n", f_pumpno, ret, readedlen, strBuf);
			sAPI_UartPrintf(buf);
			Pch = strtok((char *)strBuf, (char *)",");
			StrTokStrVer = 0;
			while (Pch != NULL)
			{
				strcpy(StrTokStr1[StrTokStrVer], Pch);
				StrTokStrVer++;
				Pch = strtok(NULL, ",");
			}

			// for(Tp=0;Tp<=StrTokStrVer;Tp++)
			//{
			//	sprintf(buf,"\n\rSt=%s  pos=%d\n\r",StrTokStr[Tp],Tp);
			//  sAPI_UartPrintf(buf);

			//}

			l_pumpno_tx = f_pumpno - 1;
			nProgram.Sno=1;
			nProgram.DelayBtwZone[0]=(int)myatoi(StrTokStr1[1]);
			nProgram.DelayBtwZone[1]=(int)myatoi(StrTokStr1[2]);
			nProgram.DelayBtwZone[2]=(int)myatoi(StrTokStr1[3]);
			nProgram.ScaleFact=myatoi(StrTokStr1[4]);
			nProgram.Schedule=(int)myatoi(StrTokStr1[5]);
			nProgram.StartDate[0]=(int)myatoi(StrTokStr1[6]);
			nProgram.StartDate[1]=(int)myatoi(StrTokStr1[7]);
			nProgram.StartDate[2]=(int)myatoi(StrTokStr1[8]);
			nProgram.DayCount=(int)myatoi(StrTokStr1[9]);
			nProgram.EndDate[0]=(int)myatoi(StrTokStr1[10]);
			nProgram.EndDate[1]=(int)myatoi(StrTokStr1[11]);
			nProgram.EndDate[2]=(int)myatoi(StrTokStr1[12]);
			nProgram.Rtc[0]=(int)myatoi(StrTokStr1[13]);
			nProgram.Rtc[1]=(int)myatoi(StrTokStr1[14]);
			nProgram.Rtc[2]=(int)myatoi(StrTokStr1[15]);
			nProgram.Alaram=(int)myatoi(StrTokStr1[16]);
			nProgram.ProgramNo=l_pumpno_tx+1;

			 sprintf(Buffer1,"Read Program,%d,%d,%d,%d,%f,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n\r",nProgram.Sno,nProgram.DelayBtwZone[0],nProgram.DelayBtwZone[1],nProgram.DelayBtwZone[2],nProgram.ScaleFact,nProgram.Schedule,nProgram.StartDate[0],nProgram.StartDate[1],nProgram.StartDate[2],nProgram.DayCount,nProgram.EndDate[0],nProgram.EndDate[1],nProgram.EndDate[2],nProgram.Rtc[0],nProgram.Rtc[1],nProgram.Rtc[2],nProgram.Alaram,nProgram.ProgramNo);
            sAPI_UartPrintf(Buffer1);
			// s_nMSettings.m_DrScOnOf[l_pumpno_tx] = myatoi(StrTokStr1[1]);
			// s_nTimerSettings.m_DrScHr[l_pumpno_tx] = myatoi(StrTokStr1[2]);
			// s_nTimerSettings.m_DrScMin[l_pumpno_tx] = myatoi(StrTokStr1[3]);
			// s_nTimerSettings.m_DrScSec[l_pumpno_tx] = myatoi(StrTokStr1[4]);
			// temp_DrAmpsII = myatoi(StrTokStr1[5]);
			// temp_DrAmpsIII = myatoi(StrTokStr1[6]);
			// s_nMSettings.m_DrReOnOf[l_pumpno_tx] = myatoi(StrTokStr1[7]);
			// s_nTimerSettings.m_DrReHr[l_pumpno_tx] = myatoi(StrTokStr1[8]);
			// s_nTimerSettings.m_DrReMin[l_pumpno_tx] = myatoi(StrTokStr1[9]);
			// s_nTimerSettings.m_DrReSec[l_pumpno_tx] = myatoi(StrTokStr1[10]);
			// s_nMSettings.m_DrOccurOnOff[l_pumpno_tx] = myatoi(StrTokStr1[11]);
			// s_nTimerSettings.m_DrOccurTimHr[l_pumpno_tx] = myatoi(StrTokStr1[12]);
			// s_nTimerSettings.m_DrOccurTimMin[l_pumpno_tx] = myatoi(StrTokStr1[13]);
			// s_nTimerSettings.m_DrOccurTimSec[l_pumpno_tx] = myatoi(StrTokStr1[14]);
			// a_occurance_count[l_pumpno_tx] = myatoi(StrTokStr1[15]);

			// s_nMSettings.m_OlOnOff[l_pumpno_tx] = myatoi(StrTokStr1[16]);
			// s_nTimerSettings.m_OlScanHr[l_pumpno_tx] = myatoi(StrTokStr1[17]);
			// s_nTimerSettings.m_OlScanMin[l_pumpno_tx] = myatoi(StrTokStr1[18]);
			// s_nTimerSettings.m_OlScanSec[l_pumpno_tx] = myatoi(StrTokStr1[19]);
			// temp_OlAmpsII = myatoi(StrTokStr1[20]);
			// temp_OlAmpsIII = myatoi(StrTokStr1[21]);
			// //	s_nMSettings.m_Drrestartpoweronof[l_pumpno_tx]=myatoi(StrTokStr1[22]);
			// s_nMSettings.m_OlRstVolOnoff[l_pumpno_tx] = myatoi(StrTokStr1[23]);
			// s_nTimerSettings.m_AutoRstOn[l_pumpno_tx] = myatoi(StrTokStr1[24]);
			// s_nMSettings.m_AutoDrRunRstIIOnOff[pumpno_tx] = myatoi(StrTokStr1[25]);
			// //	s_nMSettings.m_AutoOlDrRstIIOnOff[pumpno_tx]=myatoi(StrTokStr1[26]);

			// s_nTimerSettings.m_DrAmpsII[l_pumpno_tx] = temp_DrAmpsII * 0.01;
			// s_nTimerSettings.m_DrAmpsIII[l_pumpno_tx] = temp_DrAmpsIII * 0.01;
			// s_nTimerSettings.m_OlAmpsII[l_pumpno_tx] = temp_OlAmpsII * 0.01;
			// s_nTimerSettings.m_OlAmpsIII[l_pumpno_tx] = temp_OlAmpsIII * 0.01;

			sprintf(buf, " s_nMSettings.m_DrScOnOf[%d] =%d\r\n", l_pumpno_tx, s_nMSettings.m_DrScOnOf[l_pumpno_tx]);
			sAPI_UartPrintf(buf);
			sprintf(buf, "\n\r read f_pumpno>>:%d l_pumpno_tx %d s_nMSettings.m_DrScOnOf[%d]:%d\n\r", f_pumpno, l_pumpno_tx, l_pumpno_tx, s_nMSettings.m_DrScOnOf[l_pumpno_tx]);
			sAPI_UartPrintf(buf);
		}
		ret = sAPI_fclose(file_hdl);
		if (ret != 0)
		{
			//  sAPI_UartPrintf("c:/cset%d.txt sAPI_fclose err",f_pumpno);
			sprintf(buf, "c:/cset%d.txt sAPI_fclose err ret=%d: \r\n", f_pumpno, ret);
			sAPI_UartPrintf(buf);
		}
		else
		{
			file_hdl = NULL;
		}
		/* if(nMSettings.ndebugonof==1)
		{
			sprintf(buf,"$R,S,%d,%d,%02d,%02d,%02d,%0.2f,%0.2f,%d,%02d,%02d,%02d,%d,%02d,%02d,%02d,%d,n\r",
									 l_pumpno_tx,s_nMSettings.m_DrScOnOf[l_pumpno_tx],s_nTimerSettings.m_DrScHr[l_pumpno_tx],s_nTimerSettings.m_DrScMin[l_pumpno_tx],s_nTimerSettings.m_DrScSec[l_pumpno_tx],
									 s_nTimerSettings.m_DrAmpsII[l_pumpno_tx],s_nTimerSettings.m_DrAmpsIII[l_pumpno_tx],s_nMSettings.m_DrReOnOf[l_pumpno_tx],s_nTimerSettings.m_DrReHr[l_pumpno_tx],
									 s_nTimerSettings.m_DrReMin[l_pumpno_tx],s_nTimerSettings.m_DrReSec[l_pumpno_tx],s_nMSettings.m_DrOccurOnOff[l_pumpno_tx],s_nTimerSettings.m_DrOccurTimHr[l_pumpno_tx],
									 s_nTimerSettings.m_DrOccurTimMin[l_pumpno_tx],s_nTimerSettings.m_DrOccurTimSec[l_pumpno_tx],a_occurance_count[l_pumpno_tx]);
									 sAPI_UartPrintf(buf);
		} */
	}
}

void WritecursetFile(unsigned char f_pumpno)
{
	INT32 ret;
	UINT32 writeedlen, temp_DrAmpsII, temp_DrAmpsIII, temp_OlAmpsII, temp_OlAmpsIII;
	SCFILE *file_hdl = NULL;
	unsigned char filename[100], l_pumpno_tx;

	sprintf(filename, "c:/cset%d.txt", f_pumpno);
	strcpy((char *)pfile, (char *)filename);

	file_hdl = sAPI_fopen((UINT8 *)pfile, "wb");
	if (file_hdl == NULL)
	{
		sAPI_UartPrintf("sAPI_fopen err");
	}

	sprintf(textBuf, "FileOpenEx Create (%s,%08x)=%d\r\n", pfile, FS_CREATE, ret);
	sAPI_UartPrintf(textBuf);

	memset(textBuf, 0, 100); // 55
	ret = sAPI_fseek(file_hdl, 0, FS_SEEK_BEGIN);
	sprintf(textBuf, "FileSeek()=%d: \r\n", ret);
	sAPI_UartPrintf(textBuf);

	memset(textBuf, 0, 100); // 50
	sprintf(buf, " before write PrvAutoMobileKey is %d", PrvAutoMobileKey);
	sAPI_UartPrintf(buf);

	l_pumpno_tx = f_pumpno - 1;
	sprintf(buf, "\n\r write f_pumpno>>:%d l_pumpno_tx %d s_nMSettings.m_DrScOnOf[%d]:%d\n\r", f_pumpno, l_pumpno_tx, l_pumpno_tx, s_nMSettings.m_DrScOnOf[l_pumpno_tx]);
	sAPI_UartPrintf(buf);
	 sprintf(textBuf,"Program,%d,%d,%d,%d,%0.1f,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n\r",nProgram.Sno,nProgram.DelayBtwZone[0],nProgram.DelayBtwZone[1],nProgram.DelayBtwZone[2],nProgram.ScaleFact,nProgram.Schedule,nProgram.StartDate[0],nProgram.StartDate[1],nProgram.StartDate[2],nProgram.DayCount,nProgram.EndDate[0],nProgram.EndDate[1],nProgram.EndDate[2],nProgram.Rtc[0],nProgram.Rtc[1],nProgram.Rtc[2],nProgram.Alaram,nProgram.ProgramNo);
            sAPI_UartPrintf(textBuf);
	// temp_DrAmpsII = s_nTimerSettings.m_DrAmpsII[l_pumpno_tx] * 100;
	// temp_DrAmpsIII = s_nTimerSettings.m_DrAmpsIII[l_pumpno_tx] * 100;
	// temp_OlAmpsII = s_nTimerSettings.m_OlAmpsII[l_pumpno_tx] * 100;
	// temp_OlAmpsIII = s_nTimerSettings.m_OlAmpsIII[l_pumpno_tx] * 100;

	// sprintf(textBuf, "tnkset,%01d,%02d,%02d,%02d,%05d,%05d,%01d,%02d,%02d,%02d,%1d,%02d,%02d,%02d,%02d,%1d,%02d,%02d,%02d,%05d,%05d,%01d,%01d,%01d,%01d,%01d,\n\r",
	// 		s_nMSettings.m_DrScOnOf[l_pumpno_tx], s_nTimerSettings.m_DrScHr[l_pumpno_tx], s_nTimerSettings.m_DrScMin[l_pumpno_tx], s_nTimerSettings.m_DrScSec[l_pumpno_tx], temp_DrAmpsII, temp_DrAmpsIII,
	// 		s_nMSettings.m_DrReOnOf[l_pumpno_tx], s_nTimerSettings.m_DrReHr[l_pumpno_tx], s_nTimerSettings.m_DrReMin[l_pumpno_tx], s_nTimerSettings.m_DrReSec[l_pumpno_tx],
	// 		s_nMSettings.m_DrOccurOnOff[l_pumpno_tx], s_nTimerSettings.m_DrOccurTimHr[l_pumpno_tx], s_nTimerSettings.m_DrOccurTimMin[l_pumpno_tx], s_nTimerSettings.m_DrOccurTimSec[l_pumpno_tx], a_occurance_count[l_pumpno_tx],
	// 		s_nMSettings.m_OlOnOff[l_pumpno_tx], s_nTimerSettings.m_OlScanHr[l_pumpno_tx], s_nTimerSettings.m_OlScanMin[l_pumpno_tx], s_nTimerSettings.m_OlScanSec[l_pumpno_tx], temp_OlAmpsII, temp_OlAmpsIII,
	// 		s_nMSettings.m_Drrestartpoweronof[l_pumpno_tx], s_nMSettings.m_OlRstVolOnoff[l_pumpno_tx], s_nTimerSettings.m_AutoRstOn[l_pumpno_tx], s_nMSettings.m_AutoDrRunRstIIOnOff[l_pumpno_tx], s_nMSettings.m_AutoOlDrRstIIOnOff[l_pumpno_tx]);
	// sAPI_UartPrintf(textBuf);

	writeedlen = sAPI_fwrite((UINT8 *)textBuf, strlen((char *)textBuf), 1, file_hdl);
	if (writeedlen != strlen((char *)textBuf))
	{
		sprintf(buf, "sAPI_fwrite err write length: %d\r\n", writeedlen);
		sAPI_UartPrintf(buf);
	}
	sprintf(textBuf, "FileWrite{%s},writeedlen=%d  s_nMSettings.m_DrScOnOf[%d] =%d\r\n", pfile, writeedlen, l_pumpno_tx, s_nMSettings.m_DrScOnOf[l_pumpno_tx]);
	sAPI_UartPrintf(textBuf);

	ret = sAPI_fclose(file_hdl);
	if (ret != 0)
	{
		sAPI_UartPrintf("sAPI_fclose err");
	}
	else
	{
		file_hdl = NULL;
	}
}

void WritedelsetFile(unsigned char f_pumpno)
{
	INT32 ret;
	UINT32 writeedlen, temp_DrAmpsII, temp_DrAmpsIII, temp_OlAmpsII, temp_OlAmpsIII;
	SCFILE *file_hdl = NULL;
	unsigned char filename[100], l_pumpno_tx;

	sprintf(filename, "c:/delset%d.txt", f_pumpno);
	strcpy((char *)pfile, (char *)filename);

	file_hdl = sAPI_fopen((UINT8 *)pfile, "wb");
	if (file_hdl == NULL)
	{
		sAPI_UartPrintf("sAPI_fopen err");
	}

	sprintf(textBuf, "FileOpenEx Create (%s,%08x)=%d\r\n", pfile, FS_CREATE, ret);
	sAPI_UartPrintf(textBuf);

	memset(textBuf, 0, 100); // 55
	ret = sAPI_fseek(file_hdl, 0, FS_SEEK_BEGIN);
	sprintf(textBuf, "FileSeek()=%d: \r\n", ret);
	sAPI_UartPrintf(textBuf);

	memset(textBuf, 0, 100); // 50
	sprintf(buf, " before write PrvAutoMobileKey is %d", PrvAutoMobileKey);
	sAPI_UartPrintf(buf);

	l_pumpno_tx = f_pumpno - 1;
	temp_DrAmpsII = s_nTimerSettings.m_DrAmpsII[l_pumpno_tx] * 100;
	temp_DrAmpsIII = s_nTimerSettings.m_DrAmpsIII[l_pumpno_tx] * 100;
	temp_OlAmpsII = s_nTimerSettings.m_OlAmpsII[l_pumpno_tx] * 100;
	temp_OlAmpsIII = s_nTimerSettings.m_OlAmpsIII[l_pumpno_tx] * 100;

	sprintf(textBuf, "tnkset,%02d,%02d,%02d,%01d,%02d,%02d,%02d,%02d,%02d,%02d,%1d,%02d,%02d,%02d,%01d,%02d,%02d,%02d,%02d,%02d,%02d,%01d,%02d,%02d,%02d,\n\r",
			s_nTimerSettings.m_POnHr[l_pumpno_tx], s_nTimerSettings.m_POnMin[l_pumpno_tx], s_nTimerSettings.m_POnSec[l_pumpno_tx], s_nMSettings.m_PoScrDlOnOff[l_pumpno_tx], s_nTimerSettings.m_PoScrDlHr[l_pumpno_tx], s_nTimerSettings.m_PoScrDlMin[l_pumpno_tx], s_nTimerSettings.m_PoScrDlSec[l_pumpno_tx],
			s_nTimerSettings.m_SDHr[l_pumpno_tx], s_nTimerSettings.m_SDMin[l_pumpno_tx], s_nTimerSettings.m_SDSec[l_pumpno_tx], s_nMSettings.m_SfbOnOff[l_pumpno_tx], s_nTimerSettings.m_SfbHr[l_pumpno_tx], s_nTimerSettings.m_SfbMin[l_pumpno_tx], s_nTimerSettings.m_SfbSec[l_pumpno_tx],
			s_nMSettings.m_CycLicOnOf[l_pumpno_tx], s_nTimerSettings.m_CycLicOnHr[l_pumpno_tx], s_nTimerSettings.m_CycLicOnMin[l_pumpno_tx], s_nTimerSettings.m_CycLicOnSec[l_pumpno_tx], s_nTimerSettings.m_CycLicOfHr[l_pumpno_tx], s_nTimerSettings.m_CycLicOfMin[l_pumpno_tx], s_nTimerSettings.m_CycLicOfSec[l_pumpno_tx],
			s_nMSettings.m_MaxRnOnOf[l_pumpno_tx], s_nTimerSettings.m_MaxRnHr[l_pumpno_tx], s_nTimerSettings.m_MaxRnMin[l_pumpno_tx], s_nTimerSettings.m_MaxRnSec[l_pumpno_tx]);
	sAPI_UartPrintf(textBuf);

	writeedlen = sAPI_fwrite((UINT8 *)textBuf, strlen((char *)textBuf), 1, file_hdl);
	if (writeedlen != strlen((char *)textBuf))
	{
		sprintf(buf, "sAPI_fwrite err write length: %d\r\n", writeedlen);
		sAPI_UartPrintf(buf);
	}
	sprintf(textBuf, "FileWrite{%s},writeedlen=%d\r\n", pfile, writeedlen);
	sAPI_UartPrintf(textBuf);

	ret = sAPI_fclose(file_hdl);
	if (ret != 0)
	{
		sAPI_UartPrintf("sAPI_fclose err");
	}
	else
	{
		file_hdl = NULL;
	}
}

void ReaddelsetFile(unsigned char f_pumpno)
{

	//	char StrTokStr[30][20];   // dg_changed from [30][20]
	int StrTokStrVer = 0;
	char *Pch = NULL;
	int Tp;
	SCFILE *file_hdl = NULL;
	INT32 ret;
	UINT32 readedlen;
	unsigned char filename[100], l_pumpno_tx;
	memset(pfile, 0, 200);

	sprintf(filename, "c:/delset%d.txt", f_pumpno);
	strcpy((char *)pfile, (char *)filename);

	//	strcpy((char *)pfile,(char*)"c:/vset.txt");

	file_hdl = sAPI_fopen((UINT8 *)pfile, "ab");
	if (file_hdl == NULL)
	{
		//    sAPI_UartPrintf("sAPI_fopen err :c:/delset%d.txt",f_pumpno);
		sprintf(buf, "sAPI_fopen err :c:/delset%d.txt", f_pumpno);
		sAPI_UartPrintf(buf);
	}
	else
	{
		memset(textBuf, 0, 50);
		ret = sAPI_fseek(file_hdl, 0, FS_SEEK_BEGIN);
		sprintf(buf, "c:/delset%d.txt FileSeek()=%d: \r\n", f_pumpno, ret);
		sAPI_UartPrintf(buf);
		memset(strBuf, 0, 100); // 70
		readedlen = sAPI_fread((unsigned char *)strBuf, 100, 1, file_hdl);
		if (readedlen <= 0)
		{
			sprintf(buf, "sAPI_FsFileRead err,len: %d", readedlen);
			sAPI_UartPrintf(buf);
		}

		if (readedlen < 5)
		{
			//	g_no_of_pumps=1;
		}
		else
		{
			sprintf(buf, "c:/vset.txt FileRead()=%d: readedlen=%d, strBuf=%s\r\n", ret, readedlen, strBuf);
			sAPI_UartPrintf(buf);
			Pch = strtok((char *)strBuf, (char *)",");
			StrTokStrVer = 0;
			while (Pch != NULL)
			{
				strcpy(StrTokStr1[StrTokStrVer], Pch);
				StrTokStrVer++;
				Pch = strtok(NULL, ",");
			}

			// for(Tp=0;Tp<=StrTokStrVer;Tp++)
			//{
			//	sprintf(buf,"\n\rSt=%s  pos=%d\n\r",StrTokStr[Tp],Tp);
			//  sAPI_UartPrintf(buf);

			//}
			l_pumpno_tx = f_pumpno - 1;
			s_nTimerSettings.m_POnHr[l_pumpno_tx] = myatoi(StrTokStr1[1]);
			s_nTimerSettings.m_POnMin[l_pumpno_tx] = myatoi(StrTokStr1[2]);
			s_nTimerSettings.m_POnSec[l_pumpno_tx] = myatoi(StrTokStr1[3]);
			s_nMSettings.m_PoScrDlOnOff[l_pumpno_tx] = myatoi(StrTokStr1[4]);
			s_nTimerSettings.m_PoScrDlHr[l_pumpno_tx] = myatoi(StrTokStr1[5]);
			s_nTimerSettings.m_PoScrDlMin[l_pumpno_tx] = myatoi(StrTokStr1[6]);
			s_nTimerSettings.m_PoScrDlSec[l_pumpno_tx] = myatoi(StrTokStr1[7]);
			s_nTimerSettings.m_SDHr[l_pumpno_tx] = myatoi(StrTokStr1[8]);
			s_nTimerSettings.m_SDMin[l_pumpno_tx] = myatoi(StrTokStr1[9]);
			s_nTimerSettings.m_SDSec[l_pumpno_tx] = myatoi(StrTokStr1[10]);
			s_nMSettings.m_SfbOnOff[l_pumpno_tx] = myatoi(StrTokStr1[11]);
			s_nTimerSettings.m_SfbHr[l_pumpno_tx] = myatoi(StrTokStr1[12]);
			s_nTimerSettings.m_SfbMin[l_pumpno_tx] = myatoi(StrTokStr1[13]);
			s_nTimerSettings.m_SfbSec[l_pumpno_tx] = myatoi(StrTokStr1[14]);
			s_nMSettings.m_CycLicOnOf[l_pumpno_tx] = myatoi(StrTokStr1[15]);
			s_nTimerSettings.m_CycLicOnHr[l_pumpno_tx] = myatoi(StrTokStr1[16]);
			s_nTimerSettings.m_CycLicOnMin[l_pumpno_tx] = myatoi(StrTokStr1[17]);
			s_nTimerSettings.m_CycLicOnSec[l_pumpno_tx] = myatoi(StrTokStr1[18]);
			s_nTimerSettings.m_CycLicOfHr[l_pumpno_tx] = myatoi(StrTokStr1[19]);
			s_nTimerSettings.m_CycLicOfMin[l_pumpno_tx] = myatoi(StrTokStr1[20]);
			s_nTimerSettings.m_CycLicOfSec[l_pumpno_tx] = myatoi(StrTokStr1[21]);
			s_nMSettings.m_MaxRnOnOf[l_pumpno_tx] = myatoi(StrTokStr1[22]);
			s_nTimerSettings.m_MaxRnHr[l_pumpno_tx] = myatoi(StrTokStr1[23]);
			s_nTimerSettings.m_MaxRnMin[l_pumpno_tx] = myatoi(StrTokStr1[24]);
			s_nTimerSettings.m_MaxRnSec[l_pumpno_tx] = myatoi(StrTokStr1[25]);

			/* if(nMSettings.ndebugonof==1)
				{
					sprintf(buf,"$S,S,%d,%02d,%02d,%02d,%d,%02d,%02d,%02d,%02d,%02d,%02d,%d,%02d,%02d,%02d,%d,%02d,%02d,%02d,%02d,%02d,%02d,\n\r",
											 l_pumpno_tx,s_nTimerSettings.m_POnHr[l_pumpno_tx],s_nTimerSettings.m_POnMin[l_pumpno_tx],s_nTimerSettings.m_POnSec[l_pumpno_tx],s_nMSettings.m_PoScrDlOnOff[l_pumpno_tx],s_nTimerSettings.m_PoScrDlHr[l_pumpno_tx],
											 s_nTimerSettings.m_PoScrDlMin[l_pumpno_tx],s_nTimerSettings.m_PoScrDlSec[l_pumpno_tx],s_nTimerSettings.m_SDHr[l_pumpno_tx],s_nTimerSettings.m_SDMin[l_pumpno_tx],s_nTimerSettings.m_SDSec[l_pumpno_tx],
											 s_nMSettings.m_SfbOnOff[l_pumpno_tx],s_nTimerSettings.m_SfbHr[l_pumpno_tx],s_nTimerSettings.m_SfbMin[l_pumpno_tx],s_nTimerSettings.m_SfbSec[l_pumpno_tx],s_nMSettings.m_CycLicOnOf[l_pumpno_tx],
											 s_nTimerSettings.m_CycLicOnHr[l_pumpno_tx],s_nTimerSettings.m_CycLicOnMin[l_pumpno_tx],s_nTimerSettings.m_CycLicOnSec[l_pumpno_tx],s_nTimerSettings.m_CycLicOfHr[l_pumpno_tx],s_nTimerSettings.m_CycLicOfMin[l_pumpno_tx],
											 s_nTimerSettings.m_CycLicOfSec[l_pumpno_tx],s_nMSettings.m_MaxRnOnOf[l_pumpno_tx],s_nTimerSettings.m_MaxRnHr[l_pumpno_tx],s_nTimerSettings.m_MaxRnMin[l_pumpno_tx],s_nTimerSettings.m_MaxRnSec[l_pumpno_tx]);

											 sAPI_UartPrintf(buf);
				} */
		}
		ret = sAPI_fclose(file_hdl);
		if (ret != 0)
		{
			//   sAPI_UartPrintf("c:/delset%d.txt sAPI_fclose err",f_pumpno);
			sprintf(textBuf, "c:/delset%d.txt sAPI_fclose err", f_pumpno);
			sAPI_UartPrintf(textBuf);
		}
		else
		{
			file_hdl = NULL;
		}
	}
}

void ReadRTCsetFile(unsigned char f_pumpno)
{

	//	char StrTokStr[30][20];   // dg_changed from [30][20]
	int StrTokStrVer = 0;
	char *Pch = NULL;
	int Tp;
	SCFILE *file_hdl = NULL;
	INT32 ret;
	UINT32 readedlen;
	unsigned char filename[100], l_pumpno_tx;
	memset(pfile, 0, 200);

	sprintf(filename, "c:/rtcset%d.txt", f_pumpno);
	strcpy((char *)pfile, (char *)filename);

	//	strcpy((char *)pfile,(char*)"c:/vset.txt");

	file_hdl = sAPI_fopen((UINT8 *)pfile, "ab");
	if (file_hdl == NULL)
	{
		//   sAPI_UartPrintf("sAPI_fopen err :c:/rtcset%d.txt",f_pumpno);
		sprintf(buf, "sAPI_fopen err :c:/rtcset%d.txt", f_pumpno);
		sAPI_UartPrintf(buf);
	}
	else
	{
		memset(textBuf, 0, 50);
		ret = sAPI_fseek(file_hdl, 0, FS_SEEK_BEGIN);
		sprintf(buf, "c:/rtcset%d.txt FileSeek()=%d: \r\n", f_pumpno, ret);
		sAPI_UartPrintf(buf);
		memset(strBuf, 0, 140); // 70
		readedlen = sAPI_fread((unsigned char *)strBuf, 140, 1, file_hdl);
		if (readedlen <= 0)
		{
			sprintf(buf, "sAPI_FsFileRead err,data: %s, len: %d", strBuf, readedlen);
			sAPI_UartPrintf(buf);
		}

		if (readedlen < 5)
		{
			//	g_no_of_pumps=1;
		}
		else
		{
			sprintf(buf, "c:/rtcset%d.txt FileRead()=%d: readedlen=%d, strBuf=%s\r\n", f_pumpno, ret, readedlen, strBuf);
			sAPI_UartPrintf(buf);
			Pch = strtok((char *)strBuf, (char *)",");
			StrTokStrVer = 0;
			while (Pch != NULL)
			{
				strcpy(StrTokStr1[StrTokStrVer], Pch);
				StrTokStrVer++;
				Pch = strtok(NULL, ",");
			}

			// for(Tp=0;Tp<=StrTokStrVer;Tp++)
			//{
			//	sprintf(buf,"\n\rSt=%s  pos=%d\n\r",StrTokStr[Tp],Tp);
			//  sAPI_UartPrintf(buf);

			//}
			l_pumpno_tx = f_pumpno - 1;
			s_nMSettings.m_RTCOnOf[l_pumpno_tx] = myatoi(StrTokStr1[1]);
			s_nTimerSettings.m_RTCONHr[l_pumpno_tx][1] = myatoi(StrTokStr1[2]);
			s_nTimerSettings.m_RTCONMin[l_pumpno_tx][1] = myatoi(StrTokStr1[3]);
			s_nTimerSettings.m_RTCONSec[l_pumpno_tx][1] = myatoi(StrTokStr1[4]);
			s_nTimerSettings.m_RTCOfHr[l_pumpno_tx][1] = myatoi(StrTokStr1[5]);
			s_nTimerSettings.m_RTCOfMin[l_pumpno_tx][1] = myatoi(StrTokStr1[6]);
			s_nTimerSettings.m_RTCOfSec[l_pumpno_tx][1] = myatoi(StrTokStr1[7]);
			s_nTimerSettings.m_RTCONHr[l_pumpno_tx][2] = myatoi(StrTokStr1[8]);
			s_nTimerSettings.m_RTCONMin[l_pumpno_tx][2] = myatoi(StrTokStr1[9]);
			s_nTimerSettings.m_RTCONSec[l_pumpno_tx][2] = myatoi(StrTokStr1[10]);
			s_nTimerSettings.m_RTCOfHr[l_pumpno_tx][2] = myatoi(StrTokStr1[11]);
			s_nTimerSettings.m_RTCOfMin[l_pumpno_tx][2] = myatoi(StrTokStr1[12]);
			s_nTimerSettings.m_RTCOfSec[l_pumpno_tx][2] = myatoi(StrTokStr1[13]);
			s_nTimerSettings.m_RTCONHr[l_pumpno_tx][3] = myatoi(StrTokStr1[14]);
			s_nTimerSettings.m_RTCONMin[l_pumpno_tx][3] = myatoi(StrTokStr1[15]);
			s_nTimerSettings.m_RTCONSec[l_pumpno_tx][3] = myatoi(StrTokStr1[16]);
			s_nTimerSettings.m_RTCOfHr[l_pumpno_tx][3] = myatoi(StrTokStr1[17]);
			s_nTimerSettings.m_RTCOfMin[l_pumpno_tx][3] = myatoi(StrTokStr1[18]);
			s_nTimerSettings.m_RTCOfSec[l_pumpno_tx][3] = myatoi(StrTokStr1[19]);
			s_nTimerSettings.m_RTCONHr[l_pumpno_tx][4] = myatoi(StrTokStr1[20]);
			s_nTimerSettings.m_RTCONMin[l_pumpno_tx][4] = myatoi(StrTokStr1[21]);
			s_nTimerSettings.m_RTCONSec[l_pumpno_tx][4] = myatoi(StrTokStr1[22]);
			s_nTimerSettings.m_RTCOfHr[l_pumpno_tx][4] = myatoi(StrTokStr1[23]);
			s_nTimerSettings.m_RTCOfMin[l_pumpno_tx][4] = myatoi(StrTokStr1[24]);
			s_nTimerSettings.m_RTCOfSec[l_pumpno_tx][4] = myatoi(StrTokStr1[25]);
			s_nTimerSettings.m_RTCONHr[l_pumpno_tx][5] = myatoi(StrTokStr1[26]);
			s_nTimerSettings.m_RTCONMin[l_pumpno_tx][5] = myatoi(StrTokStr1[27]);
			s_nTimerSettings.m_RTCONSec[l_pumpno_tx][5] = myatoi(StrTokStr1[28]);
			s_nTimerSettings.m_RTCOfHr[l_pumpno_tx][5] = myatoi(StrTokStr1[29]);
			s_nTimerSettings.m_RTCOfMin[l_pumpno_tx][5] = myatoi(StrTokStr1[30]);
			s_nTimerSettings.m_RTCOfSec[l_pumpno_tx][5] = myatoi(StrTokStr1[31]);
			s_nTimerSettings.m_RTCONHr[l_pumpno_tx][6] = myatoi(StrTokStr1[32]);
			s_nTimerSettings.m_RTCONMin[l_pumpno_tx][6] = myatoi(StrTokStr1[33]);
			s_nTimerSettings.m_RTCONSec[l_pumpno_tx][6] = myatoi(StrTokStr1[34]);
			s_nTimerSettings.m_RTCOfHr[l_pumpno_tx][6] = myatoi(StrTokStr1[35]);
			s_nTimerSettings.m_RTCOfMin[l_pumpno_tx][6] = myatoi(StrTokStr1[36]);
			s_nTimerSettings.m_RTCOfSec[l_pumpno_tx][6] = myatoi(StrTokStr1[37]);

			/* sprintf(buf,"$T,S,%d,%d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,\n\r",
											l_pumpno_tx,s_nMSettings.m_RTCOnOf[l_pumpno_tx],s_nTimerSettings.m_RTCONHr[l_pumpno_tx][1],s_nTimerSettings.m_RTCONMin[l_pumpno_tx][1],s_nTimerSettings.m_RTCONSec[l_pumpno_tx][1],
											s_nTimerSettings.m_RTCOfHr[l_pumpno_tx][1],s_nTimerSettings.m_RTCOfMin[l_pumpno_tx][1],s_nTimerSettings.m_RTCOfSec[l_pumpno_tx][1],s_nTimerSettings.m_RTCONHr[l_pumpno_tx][2],
											s_nTimerSettings.m_RTCONMin[l_pumpno_tx][2],s_nTimerSettings.m_RTCONSec[l_pumpno_tx][2],s_nTimerSettings.m_RTCOfHr[l_pumpno_tx][2],s_nTimerSettings.m_RTCOfMin[l_pumpno_tx][2],
											s_nTimerSettings.m_RTCOfSec[l_pumpno_tx][2],s_nTimerSettings.m_RTCONHr[l_pumpno_tx][3],s_nTimerSettings.m_RTCONMin[l_pumpno_tx][3],s_nTimerSettings.m_RTCONSec[l_pumpno_tx][3],
											s_nTimerSettings.m_RTCOfHr[l_pumpno_tx][3],s_nTimerSettings.m_RTCOfMin[l_pumpno_tx][3],s_nTimerSettings.m_RTCOfSec[l_pumpno_tx][3]);
											sAPI_UartPrintf(buf); */
		}
		ret = sAPI_fclose(file_hdl);
		if (ret != 0)
		{
			//   sAPI_UartPrintf("c:/rtcset%d.txt sAPI_fclose err",f_pumpno);
			sprintf(buf, "c:/rtcset%d.txt sAPI_fclose err", f_pumpno);
			sAPI_UartPrintf(buf);
		}
		else
		{
			file_hdl = NULL;
		}
	}
}

void WriteRTCsetFile(unsigned char f_pumpno)
{
	INT32 ret;
	UINT32 writeedlen;
	SCFILE *file_hdl = NULL;
	unsigned char filename[100], l_pumpno_tx;

	sprintf(filename, "c:/rtcset%d.txt", f_pumpno);
	strcpy((char *)pfile, (char *)filename);

	file_hdl = sAPI_fopen((UINT8 *)pfile, "wb");
	if (file_hdl == NULL)
	{
		sAPI_UartPrintf("sAPI_fopen err");
	}

	sprintf(textBuf, "FileOpenEx Create (%s,%08x)=%d\r\n", pfile, FS_CREATE, ret);
	sAPI_UartPrintf(textBuf);

	memset(textBuf, 0, 100); // 55
	ret = sAPI_fseek(file_hdl, 0, FS_SEEK_BEGIN);
	sprintf(textBuf, "FileSeek()=%d: \r\n", ret);
	sAPI_UartPrintf(textBuf);

	memset(textBuf, 0, 140); // 50

	l_pumpno_tx = f_pumpno - 1;

	sprintf(textBuf, "tnkset,%01d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,\n\r",
			s_nMSettings.m_RTCOnOf[l_pumpno_tx], s_nTimerSettings.m_RTCONHr[l_pumpno_tx][1], s_nTimerSettings.m_RTCONMin[l_pumpno_tx][1], s_nTimerSettings.m_RTCONSec[l_pumpno_tx][1], s_nTimerSettings.m_RTCOfHr[l_pumpno_tx][1], s_nTimerSettings.m_RTCOfMin[l_pumpno_tx][1], s_nTimerSettings.m_RTCOfSec[l_pumpno_tx][1],
			s_nTimerSettings.m_RTCONHr[l_pumpno_tx][2], s_nTimerSettings.m_RTCONMin[l_pumpno_tx][2], s_nTimerSettings.m_RTCONSec[l_pumpno_tx][2], s_nTimerSettings.m_RTCOfHr[l_pumpno_tx][2], s_nTimerSettings.m_RTCOfMin[l_pumpno_tx][2], s_nTimerSettings.m_RTCOfSec[l_pumpno_tx][2],
			s_nTimerSettings.m_RTCONHr[l_pumpno_tx][3], s_nTimerSettings.m_RTCONMin[l_pumpno_tx][3], s_nTimerSettings.m_RTCONSec[l_pumpno_tx][3], s_nTimerSettings.m_RTCOfHr[l_pumpno_tx][3], s_nTimerSettings.m_RTCOfMin[l_pumpno_tx][3], s_nTimerSettings.m_RTCOfSec[l_pumpno_tx][3],
			s_nTimerSettings.m_RTCONHr[l_pumpno_tx][4], s_nTimerSettings.m_RTCONMin[l_pumpno_tx][4], s_nTimerSettings.m_RTCONSec[l_pumpno_tx][4], s_nTimerSettings.m_RTCOfHr[l_pumpno_tx][4], s_nTimerSettings.m_RTCOfMin[l_pumpno_tx][4], s_nTimerSettings.m_RTCOfSec[l_pumpno_tx][4],
			s_nTimerSettings.m_RTCONHr[l_pumpno_tx][5], s_nTimerSettings.m_RTCONMin[l_pumpno_tx][5], s_nTimerSettings.m_RTCONSec[l_pumpno_tx][5], s_nTimerSettings.m_RTCOfHr[l_pumpno_tx][5], s_nTimerSettings.m_RTCOfMin[l_pumpno_tx][5], s_nTimerSettings.m_RTCOfSec[l_pumpno_tx][5],
			s_nTimerSettings.m_RTCONHr[l_pumpno_tx][6], s_nTimerSettings.m_RTCONMin[l_pumpno_tx][6], s_nTimerSettings.m_RTCONSec[l_pumpno_tx][6], s_nTimerSettings.m_RTCOfHr[l_pumpno_tx][6], s_nTimerSettings.m_RTCOfMin[l_pumpno_tx][6], s_nTimerSettings.m_RTCOfSec[l_pumpno_tx][6]);
	sAPI_UartPrintf(textBuf);

	writeedlen = sAPI_fwrite((UINT8 *)textBuf, strlen((char *)textBuf), 1, file_hdl);
	if (writeedlen != strlen((char *)textBuf))
	{
		sprintf(buf, "sAPI_fwrite err write length: %d\r\n", writeedlen);
		sAPI_UartPrintf(buf);
	}
	sprintf(textBuf, "FileWrite{%s},writeedlen=%d\r\n", pfile, writeedlen);
	sAPI_UartPrintf(textBuf);

	ret = sAPI_fclose(file_hdl);
	if (ret != 0)
	{
		sAPI_UartPrintf("sAPI_fclose err");
	}
	else
	{
		file_hdl = NULL;
	}
}

void WriteTopicsetFile()
{
	INT32 ret;
	UINT32 writeedlen;
	SCFILE *file_hdl = NULL;
	unsigned char filename[100];

	sprintf(filename, "c:/topicset.txt");
	strcpy((char *)pfile, (char *)filename);

	file_hdl = sAPI_fopen((UINT8 *)pfile, "wb");
	if (file_hdl == NULL)
	{
		sAPI_UartPrintf("sAPI_fopen err");
	}

	sprintf(textBuf, "FileOpenEx Create (%s,%08x)=%d, line:%d\r\n", pfile, FS_CREATE, ret, __LINE__);
	sAPI_UartPrintf(textBuf);

	memset(textBuf, 0, 200); // 55
	ret = sAPI_fseek(file_hdl, 0, FS_SEEK_BEGIN);
	sprintf(textBuf, "FileSeek()=%d: \r\n", ret);
	sAPI_UartPrintf(textBuf);

	sprintf(textBuf, "topicset,%s,%s,%s,\n\r", DeviceConfig.MqttPublishTopic, DeviceConfig.MqttServerTopic, DeviceConfig.MqttSubscribeTopic);
	sAPI_UartPrintf(textBuf);

	writeedlen = sAPI_fwrite((UINT8 *)textBuf, strlen((char *)textBuf), 1, file_hdl);
	if (writeedlen != strlen((char *)textBuf))
	{
		sprintf(buf, "sAPI_fwrite err write length: %d\r\n", writeedlen);
		sAPI_UartPrintf(buf);
	}

	ret = sAPI_fclose(file_hdl);
	if (ret != 0)
	{
		sAPI_UartPrintf("sAPI_fclose err");
	}
	else
	{
		file_hdl = NULL;
	}
}

void ReadTopicsetFile()
{

	int StrTokStrVer = 0;
	char *Pch = NULL;
	int Tp;
	SCFILE *file_hdl = NULL;
	INT32 ret;
	UINT32 readedlen;

	unsigned char filename[30];
	memset(pfile, 0, 20);

	sprintf(filename, "c:/topicset.txt");
	strcpy((char *)pfile, (char *)filename);

	file_hdl = sAPI_fopen((UINT8 *)pfile, "ab");
	if (file_hdl == NULL)
	{
		//    sAPI_UartPrintf("sAPI_fopen err :c:/topicset.txt",f_pumpno);
		sprintf(buf, "sAPI_fopen err :c:/topicset.txt");
		sAPI_UartPrintf(buf);
	}
	else
	{
		memset(textBuf, 0, 50);
		ret = sAPI_fseek(file_hdl, 0, FS_SEEK_BEGIN);
		sprintf(buf, "c:/topicset.txt FileSeek()=%d: \r\n", ret);
		sAPI_UartPrintf(buf);
		memset(strBuf, 0, 200); // 70
		readedlen = sAPI_fread((unsigned char *)strBuf, 200, 1, file_hdl);
		if (readedlen <= 0)
		{
			sprintf(buf, "sAPI_FsFileRead err, len: %d", readedlen);
			sAPI_UartPrintf(buf);
		}

		if (readedlen < 5)
		{
			//	g_no_of_pumps=1;
		}
		else
		{
			sprintf(buf, "c:/topicset.txt FileRead()=%d: readedlen=%d, strBuf=%s\r\n", ret, readedlen, strBuf);
			sAPI_UartPrintf(buf);
			Pch = strtok((char *)strBuf, (char *)",");
			StrTokStrVer = 0;
			while (Pch != NULL)
			{
				strcpy(StrTokStr1[StrTokStrVer], Pch);
				StrTokStrVer++;
				Pch = strtok(NULL, ",");
			}

			strcpy((char *)DeviceConfig.MqttPublishTopic, (char *)(StrTokStr1[1]));
			strcpy((char *)DeviceConfig.MqttServerTopic, (char *)(StrTokStr1[2]));
			strcpy((char *)DeviceConfig.MqttSubscribeTopic, (char *)(StrTokStr1[3]));

			sprintf(buf, "\n Read topic:%s,%s,%s\n", DeviceConfig.MqttPublishTopic, DeviceConfig.MqttServerTopic, DeviceConfig.MqttSubscribeTopic);
			sAPI_UartPrintf(buf);
		}
		ret = sAPI_fclose(file_hdl);
		if (ret != 0)
		{
			//  sAPI_UartPrintf("c:/topicset.txt sAPI_fclose err",f_pumpno);
			sprintf(buf, "c:/topicset.txt sAPI_fclose err ret=%d: \r\n", ret);
			sAPI_UartPrintf(buf);
		}
		else
		{
			file_hdl = NULL;
		}
	}
}

void ReadctsetFile()
{

//	char StrTokStr[30][20];   // dg_changed from [30][20]
	int StrTokStrVer = 0;
	char *Pch = NULL;
	int Tp;
    SCFILE *file_hdl = NULL;
	INT32 ret;
    UINT32 readedlen,temp_tank_height,temp_calfactor,temp_calfactorRC,temp_calfactorYC,temp_calfactorBC,temp_calfactorRV,temp_calfactorYV,temp_calfactorBV,temp_lps,temp_pressValue;

	unsigned char filename[30];
	 memset(pfile,0,20);	 

 sprintf(filename,"c:/unique.txt");
	strcpy((char *)pfile,(char*)filename);

//	strcpy((char *)pfile,(char*)"c:/vset.txt");

//	file_hdl = sAPI_fopen(pfile,"ab");
file_hdl = sAPI_fopen((UINT8*)pfile,"ab");
                if(file_hdl == NULL)
                {
                //    sAPI_UartPrintf("sAPI_fopen err :c:/ctset.txt",f_pumpno);  
						 sprintf(buf,"sAPI_fopen err :c:/ctset.txt");
					sAPI_UartPrintf(buf);
                }
				else
				{		
		memset(textBuf,0,50);
		ret = sAPI_fseek(file_hdl, 0 , FS_SEEK_BEGIN);
		sprintf(buf,"c:/ctset.txt FileSeek()=%d: \r\n",ret);
		sAPI_UartPrintf(buf);
		memset(strBuf,0,200);  //70
		 readedlen = sAPI_fread((unsigned char *)strBuf,200, 1, file_hdl);
                if(readedlen <= 0)
                {
                    sprintf(buf,"sAPI_FsFileRead err, len: %d",  readedlen);
                    sAPI_UartPrintf(buf);                     
                }	
		
		
		 if(readedlen<5)
		{
	//	g_no_of_pumps=1;
		}
		else
		{
			sprintf(buf,"%d:len=%d,buf=%s\r\n",ret, readedlen, strBuf);
		sAPI_UartPrintf(buf); 
			Pch = strtok((char *)strBuf, (char *)"," );
			StrTokStrVer = 0;
			while( Pch != NULL )
			{
				strcpy(StrTokStr1[StrTokStrVer],Pch);
				StrTokStrVer++;
				Pch = strtok( NULL, "," );
			}



			//for(Tp=0;Tp<=StrTokStrVer;Tp++)
			//{
			//	sprintf(buf,"\n\rSt=%s  pos=%d\n\r",StrTokStr[Tp],Tp);
			// sAPI_UartPrintf(buf);

			//}
		
		//	s_nMSettings.m_AutoStIIOnOff[0]=myatoi(StrTokStr1[1]);  ////array not needed
			s_nMSettings.m_CTRonoff=myatoi(StrTokStr1[2]);
			s_nMSettings.m_CTYonoff=myatoi(StrTokStr1[3]);
			s_nMSettings.m_CTBonoff=myatoi(StrTokStr1[4]);
			temp_tank_height=myatoi(StrTokStr1[5]);
			s_ntank.m_min_level_p=myatoi(StrTokStr1[6]);
			s_ntank.m_max_level_p=myatoi(StrTokStr1[7]);
			s_nTimerSettings.m_AutoRst2On=myatoi(StrTokStr1[8]);
			s_nMSettings.m_NlightOnOf=myatoi(StrTokStr1[9]);
			s_nMSettings.m_NlightRTCOnHr=myatoi(StrTokStr1[10]);
			s_nMSettings.m_NlightRTCOnMin=myatoi(StrTokStr1[11]);
			//s_nMSettings.m_NlightRTCOnSec=myatoi(StrTokStr1[12]);
			s_nMSettings.m_NlightRTCOfHr=myatoi(StrTokStr1[12]);
			s_nMSettings.m_NlightRTCOfMin=myatoi(StrTokStr1[13]);
			//s_nMSettings.m_NlightRTCOfSec=myatoi(StrTokStr1[15]);
			s_nMSettings.m_peakHourOnOf=myatoi(StrTokStr1[14]);
			s_nMSettings.m_peakOnHr=myatoi(StrTokStr1[15]);
			s_nMSettings.m_peakOnMin=myatoi(StrTokStr1[16]);
			s_nMSettings.m_peakOnSec=myatoi(StrTokStr1[17]);
			s_nMSettings.m_peakOfHr=myatoi(StrTokStr1[18]);
			s_nMSettings.m_peakOfMin=myatoi(StrTokStr1[19]);
			s_nMSettings.m_peakOfSec=myatoi(StrTokStr1[20]);
			s_ntank.m_Level_Sensor_height=myatoi(StrTokStr1[21]);
			//s_ntank.m_Level_calfactor=myatof(StrTokStr1[24]);
			temp_calfactor=myatoi(StrTokStr1[22]);
			s_ntank.m_Uppertank=myatoi(StrTokStr1[23]);
			s_ntank.m_Lowertank=myatoi(StrTokStr1[24]);
			temp_calfactorRV=myatoi(StrTokStr1[25]);
			temp_calfactorYV=myatoi(StrTokStr1[26]);
			temp_calfactorBV=myatoi(StrTokStr1[27]);
			//s_nMSettings.m_CalRVoltage=myatoi(StrTokStr1[25]);
			//s_nMSettings.m_CalYVoltage=myatoi(StrTokStr1[26]);
			//s_nMSettings.m_CalBVoltage=myatoi(StrTokStr1[27]);
			temp_calfactorRC=myatoi(StrTokStr1[28]);
			temp_calfactorYC=myatoi(StrTokStr1[29]);
			temp_calfactorBC=myatoi(StrTokStr1[30]);
			temp_lps=myatoi(StrTokStr1[31]);
			temp_pressValue=myatoi(StrTokStr1[32]);
			s_ntank.m_Press_Max_Value=myatoi(StrTokStr1[33]);
			Liter_Per_Pulse=myatoi(StrTokStr1[34]);
			Flow_reset=myatoi(StrTokStr1[35]);
			my_eeprom_ID[0]=myatoi(StrTokStr1[36]);
			my_eeprom_ID[1]=myatoi(StrTokStr1[37]);
			my_eeprom_ID[2]=myatoi(StrTokStr1[38]);
			my_eeprom_ID[3]=myatoi(StrTokStr1[39]);
			my_eeprom_ID[4]=myatoi(StrTokStr1[40]);
			my_eeprom_ID[5]=myatoi(StrTokStr1[41]);
			
				s_ntank.m_tank_height=temp_tank_height*0.01;
				s_nMSettings.m_Flow_calfactor=temp_lps*0.001;
				s_nMSettings.m_Press_calfactor=temp_pressValue*0.001;
				s_ntank.m_Level_calfactor=temp_calfactor*0.001;
				s_nMSettings.m_CalRVoltage=temp_calfactorRV*0.01;
				s_nMSettings.m_CalYVoltage=temp_calfactorYV*0.01;
				s_nMSettings.m_CalBVoltage=temp_calfactorBV*0.01;
				s_nMSettings.m_CalRCurrent=temp_calfactorRC*0.1;
				s_nMSettings.m_CalYCurrent=temp_calfactorYC*0.1;
				s_nMSettings.m_CalBCurrent=temp_calfactorBC*0.1;
				
			// sprintf(IMEI,"%02X%02X%02X%02X%02X%02X",my_eeprom_ID[0],my_eeprom_ID[1],my_eeprom_ID[2],my_eeprom_ID[3],my_eeprom_ID[4],my_eeprom_ID[5]);	
					//	MqttInitStatus=0;
					
						sprintf(buf,"\n IMEI %s,line :%d\n",IMEI,__LINE__);
						sAPI_UartPrintf(buf);
			
			} 
  ret = sAPI_fclose(file_hdl);
                 if(ret != 0)
                {
                  //  sAPI_UartPrintf("c:/ctset.txt sAPI_fclose err",f_pumpno);
				  sprintf(buf,"c:/ctset.txt sAPI_fclose err ret=%d: \r\n",ret);
					sAPI_UartPrintf(buf);
                   
                }
				else
				{
                    file_hdl = NULL;
                }
				
			}	
				

	 
}  









void WritectsetFile()
{
	INT32 ret;
    UINT32 writeedlen,temp_tank_height,temp_calfactor,temp_calfactorRC,temp_calfactorYC,temp_calfactorBC,temp_lps,temp_pressValue,temp_calfactorRV,temp_calfactorYV,temp_calfactorBV;
    SCFILE *file_hdl = NULL;
	unsigned char filename[100],l_pumpno_tx; 
	
              
				
				ret=sAPI_GetSize ("c:");
	sprintf(buf,"size=%d: \r\n",ret);
	ret=sAPI_GetFreeSize ("c:");
	sprintf(buf,"free size=%d: \r\n",ret);
		sAPI_UartPrintf(buf);
		  ret = sAPI_GetUsedSize("C:");
		  sprintf(buf,"used size=%d: \r\n",ret);
		sAPI_UartPrintf(buf);


	
	sprintf(filename,"c:/unique.txt");
	strcpy((char *)pfile,(char*)filename);
	 
	 file_hdl = sAPI_fopen((UINT8*)pfile, "wb");
                if(file_hdl == NULL)
                {
                    sAPI_UartPrintf("sAPI_fopen err WritectsetFile");
                    
                } 
 	 else
	 {
	sprintf(buf,"FileOpenEx Create (%s)=%d, line:%d\r\n", pfile,ret,__LINE__);
	sAPI_UartPrintf(buf);
	}
	memset(textBuf,0,200);  //55
	ret = sAPI_fseek(file_hdl, 0 , FS_SEEK_BEGIN);
	sprintf(textBuf,"FileSeek()=%d: \r\n",ret);
	sAPI_UartPrintf(textBuf);


   memset(textBuf,0,200);	//50
   sprintf(buf," before write PrvAutoMobileKey is %d",PrvAutoMobileKey);
	sAPI_UartPrintf(buf);
	
	
	temp_tank_height=s_ntank.m_tank_height*100;
	temp_calfactor=s_ntank.m_Level_calfactor*1000;
	temp_lps=s_nMSettings.m_Flow_calfactor*1000;
	temp_pressValue=s_nMSettings.m_Press_calfactor*1000;
	
	temp_calfactorRV=s_nMSettings.m_CalRVoltage*100;  ////array not needed
	temp_calfactorYV=s_nMSettings.m_CalYVoltage*100;
	temp_calfactorBV=s_nMSettings.m_CalBVoltage*100;
	temp_calfactorRC=s_nMSettings.m_CalRCurrent*10;  ////array not needed
	temp_calfactorYC=s_nMSettings.m_CalYCurrent*10;
	temp_calfactorBC=s_nMSettings.m_CalBCurrent*10;
	
	sprintf(textBuf,"ctset,%01d,%01d,%01d,%01d,%05d,%03d,%03d,%01d,%01d,%02d,%02d,%02d,%02d,%01d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%05d,%01d,%01d,%05d,%05d,%05d,%05d,%05d,%05d,%05d,%05d,%02d,%05d,%01d,%02d,%02d,%02d,%02d,%02d,%02d,\n\r",
	s_nMSettings.m_AutoStIIOnOff[0],s_nMSettings.m_CTRonoff,s_nMSettings.m_CTYonoff,s_nMSettings.m_CTBonoff,temp_tank_height,s_ntank.m_min_level_p,s_ntank.m_max_level_p,s_nTimerSettings.m_AutoRst2On,s_nMSettings.m_NlightOnOf,s_nMSettings.m_NlightRTCOnHr,s_nMSettings.m_NlightRTCOnMin,
	s_nMSettings.m_NlightRTCOfHr,s_nMSettings.m_NlightRTCOfMin,s_nMSettings.m_peakHourOnOf,s_nMSettings.m_peakOnHr,s_nMSettings.m_peakOnMin,s_nMSettings.m_peakOnSec,s_nMSettings.m_peakOfHr,s_nMSettings.m_peakOfMin,s_nMSettings.m_peakOfSec,s_ntank.m_Level_Sensor_height,temp_calfactor,
	s_ntank.m_Uppertank,s_ntank.m_Lowertank,temp_calfactorRV,temp_calfactorYV,temp_calfactorBV,temp_calfactorRC,temp_calfactorYC,temp_calfactorBC,temp_lps,temp_pressValue,s_ntank.m_Press_Max_Value,Liter_Per_Pulse,Flow_reset,
	my_eeprom_ID[0],my_eeprom_ID[1],my_eeprom_ID[2],my_eeprom_ID[3],my_eeprom_ID[4],my_eeprom_ID[5]);
    sAPI_UartPrintf(textBuf);
	 
	 writeedlen = sAPI_fwrite((UINT8*)textBuf, strlen((char*)textBuf),1,file_hdl);
                if(writeedlen != strlen((char*)textBuf))
                {
                    sprintf(buf,"sAPI_fwrite err write length: %d\r\n", writeedlen);
					sAPI_UartPrintf(buf);                    
                }
			sprintf(buf,"\n\r WritectsetFile: [%s],[%d]\r\n",textBuf, writeedlen);
					sAPI_UartPrintf(buf);
	sprintf(buf," after write unique %02d,%02d,%02d,%02d,%02d,%02d, is %d",my_eeprom_ID[0],my_eeprom_ID[1],my_eeprom_ID[2],my_eeprom_ID[3],my_eeprom_ID[4],my_eeprom_ID[5]);
	sAPI_UartPrintf(buf);
		
	  ret = sAPI_fclose(file_hdl);
                if(ret != 0)
                {
                    sAPI_UartPrintf("sAPI_fclose err");
                   
                }
				else
				{
                    file_hdl = NULL;
                }

}

void ReadEpromidFile()
{

//	char StrTokStr[30][20];   // dg_changed from [30][20]
	int StrTokStrVer = 0;
	char *Pch = NULL;
	int Tp;
    SCFILE *file_hdl = NULL;
	INT32 ret;
    UINT32 readedlen,temp_tank_height,temp_calfactor,temp_calfactorRC,temp_calfactorYC,temp_calfactorBC,temp_calfactorRV,temp_calfactorYV,temp_calfactorBV,temp_lps,temp_pressValue;

	unsigned char filename[30];
	 memset(pfile,0,20);	 

 sprintf(filename,"c:/epid.txt");
	strcpy((char *)pfile,(char*)filename);

//	strcpy((char *)pfile,(char*)"c:/vset.txt");

//	file_hdl = sAPI_fopen(pfile,"ab");
file_hdl = sAPI_fopen((UINT8*)pfile,"ab");
                if(file_hdl == NULL)
                {
                //    sAPI_UartPrintf("sAPI_fopen err :c:/epid.txt",f_pumpno);  
						 sprintf(buf,"sAPI_fopen err :c:/epid.txt");
					sAPI_UartPrintf(buf);
                }
				else
				{		
		memset(textBuf,0,50);
		ret = sAPI_fseek(file_hdl, 0 , FS_SEEK_BEGIN);
		sprintf(buf,"c:/epid.txt FileSeek()=%d: \r\n",ret);
		sAPI_UartPrintf(buf);
		memset(strBuf,0,200);  //70
		memset(IMEI,0x00,sizeof(IMEI));
		 readedlen = sAPI_fread((unsigned char *)strBuf,200, 1, file_hdl);
                if(readedlen <= 0)
                {
                    sprintf(buf,"sAPI_FsFileRead err, len: %d",  readedlen);
                    sAPI_UartPrintf(buf);                     
                }	
		
		
		 if(readedlen<5)
		{
	//	g_no_of_pumps=1;
		}
		else
		{
			sprintf(buf,"%d:len=%d,buf=%s\r\n",ret, readedlen, strBuf);
		sAPI_UartPrintf(buf); 
			Pch = strtok((char *)strBuf, (char *)"," );
			StrTokStrVer = 0;
			while( Pch != NULL )
			{
				strcpy(StrTokStr1[StrTokStrVer],Pch);
				StrTokStrVer++;
				Pch = strtok( NULL, "," );
			}



			//for(Tp=0;Tp<=StrTokStrVer;Tp++)
			//{
			//	sprintf(buf,"\n\rSt=%s  pos=%d\n\r",StrTokStr[Tp],Tp);
			// sAPI_UartPrintf(buf);

			//}
		

			my_eeprom_ID[0]=myatoi(StrTokStr1[1]);
			my_eeprom_ID[1]=myatoi(StrTokStr1[2]);
			my_eeprom_ID[2]=myatoi(StrTokStr1[3]);
			my_eeprom_ID[3]=myatoi(StrTokStr1[4]);
			my_eeprom_ID[4]=myatoi(StrTokStr1[5]);
			my_eeprom_ID[5]=myatoi(StrTokStr1[6]);
			
				
				
			sprintf(IMEI,"%02X%02X%02X%02X%02X%02X",my_eeprom_ID[0],my_eeprom_ID[1],my_eeprom_ID[2],my_eeprom_ID[3],my_eeprom_ID[4],my_eeprom_ID[5]);	
					//	MqttInitStatus=0;
					
						sprintf(buf,"\n IMEI %s,line :%d\n",IMEI,__LINE__);
						sAPI_UartPrintf(buf);
			
			} 
  ret = sAPI_fclose(file_hdl);
                 if(ret != 0)
                {
                  //  sAPI_UartPrintf("c:/epid.txt sAPI_fclose err",f_pumpno);
				  sprintf(buf,"c:/epid.txt sAPI_fclose err ret=%d: \r\n",ret);
					sAPI_UartPrintf(buf);
                   
                }
				else
				{
                    file_hdl = NULL;
                }
				
			}	
				

	 
} 




void WriteEpromidFile()
{
	INT32 ret;
    UINT32 writeedlen,temp_tank_height,temp_calfactor,temp_calfactorRC,temp_calfactorYC,temp_calfactorBC,temp_lps,temp_pressValue,temp_calfactorRV,temp_calfactorYV,temp_calfactorBV;
    SCFILE *file_hdl = NULL;
	unsigned char filename[100],l_pumpno_tx; 
	
              
				
				ret=sAPI_GetSize ("c:");
	sprintf(buf,"size=%d: \r\n",ret);
	ret=sAPI_GetFreeSize ("c:");
	sprintf(buf,"free size=%d: \r\n",ret);
		sAPI_UartPrintf(buf);
		  ret = sAPI_GetUsedSize("C:");
		  sprintf(buf,"used size=%d: \r\n",ret);
		sAPI_UartPrintf(buf);


	
	sprintf(filename,"c:/epid.txt");
	strcpy((char *)pfile,(char*)filename);
	 
	 file_hdl = sAPI_fopen((UINT8*)pfile, "wb");
                if(file_hdl == NULL)
                {
                    sAPI_UartPrintf("sAPI_fopen err WriteEpromidFile");
                    
                } 
 	 else
	 {
	sprintf(buf,"FileOpenEx Create (%s)=%d, line:%d\r\n", pfile,ret,__LINE__);
	sAPI_UartPrintf(buf);
	}
	memset(textBuf,0,200);  //55
	ret = sAPI_fseek(file_hdl, 0 , FS_SEEK_BEGIN);
	sprintf(textBuf,"FileSeek()=%d: \r\n",ret);
	sAPI_UartPrintf(textBuf);


   memset(textBuf,0,200);	//50
   sprintf(buf," before write PrvAutoMobileKey is %d",PrvAutoMobileKey);
	sAPI_UartPrintf(buf);
	
	
	
	
	sprintf(textBuf,"epid,%02d,%02d,%02d,%02d,%02d,%02d,\n\r",
	my_eeprom_ID[0],my_eeprom_ID[1],my_eeprom_ID[2],my_eeprom_ID[3],my_eeprom_ID[4],my_eeprom_ID[5]);
    sAPI_UartPrintf(textBuf);
	 
	 writeedlen = sAPI_fwrite((UINT8*)textBuf, strlen((char*)textBuf),1,file_hdl);
                if(writeedlen != strlen((char*)textBuf))
                {
                    sprintf(buf,"sAPI_fwrite err write length: %d\r\n", writeedlen);
					sAPI_UartPrintf(buf);                    
                }
			sprintf(buf,"\n\r WriteEpromidFile: [%s],[%d]\r\n",textBuf, writeedlen);
					sAPI_UartPrintf(buf);
	sprintf(buf," after write unique %02d,%02d,%02d,%02d,%02d,%02d, is %d",my_eeprom_ID[0],my_eeprom_ID[1],my_eeprom_ID[2],my_eeprom_ID[3],my_eeprom_ID[4],my_eeprom_ID[5]);
	sAPI_UartPrintf(buf);
		
	  ret = sAPI_fclose(file_hdl);
                if(ret != 0)
                {
                    sAPI_UartPrintf("sAPI_fclose err");
                   
                }
				else
				{
                    file_hdl = NULL;
                }

}

void ReadcalsetFile()
{

	//	char StrTokStr[30][20];   // dg_changed from [30][20]
	int StrTokStrVer = 0;
	char *Pch = NULL;
	int Tp;
	SCFILE *file_hdl = NULL;
	INT32 ret;
	// UINT32 readedlen,temp_tank_height,temp_calfactor,temp_calfactorRC,temp_calfactorYC,temp_calfactorBC,temp_calfactorRV,temp_calfactorYV,temp_calfactorBV,temp_lps,temp_pressValue;
	UINT32 readedlen, temp_RV_Rcvd = 0, temp_YV_Rcvd = 0, temp_BV_Rcvd = 0, temp_RC_Rcvd = 0, temp_YC_Rcvd = 0, temp_BC_Rcvd = 0;

	unsigned char filename[30];
	memset(pfile, 0, 200);

	sprintf(filename, "c:/calset.txt");
	strcpy((char *)pfile, (char *)filename);

	//	strcpy((char *)pfile,(char*)"c:/vset.txt");

	//	file_hdl = sAPI_fopen(pfile,"ab");
	file_hdl = sAPI_fopen((UINT8 *)pfile, "ab");
	if (file_hdl == NULL)
	{
		//    sAPI_UartPrintf("sAPI_fopen err :c:/ctset.txt",f_pumpno);
		sprintf(buf, "sAPI_fopen err :c:/calset.txt");
		sAPI_UartPrintf(buf);
	}
	else
	{
		memset(textBuf, 0, 50);
		ret = sAPI_fseek(file_hdl, 0, FS_SEEK_BEGIN);
		sprintf(buf, "c:/calset.txt FileSeek()=%d: \r\n", ret);
		sAPI_UartPrintf(buf);
		memset(strBuf, 0, 200); // 70
		readedlen = sAPI_fread((unsigned char *)strBuf, 200, 1, file_hdl);
		if (readedlen <= 0)
		{
			sprintf(buf, "sAPI_FsFileRead err, len: %d", readedlen);
			sAPI_UartPrintf(buf);
		}

		if (readedlen < 5)
		{
			//	g_no_of_pumps=1;
		}
		else
		{
			sprintf(buf, "c:/calset.txt FileRead()=%d: readedlen=%d, strBuf=%s\r\n", ret, readedlen, strBuf);
			sAPI_UartPrintf(buf);
			Pch = strtok((char *)strBuf, (char *)",");
			StrTokStrVer = 0;
			while (Pch != NULL)
			{
				strcpy(StrTokStr1[StrTokStrVer], Pch);
				StrTokStrVer++;
				Pch = strtok(NULL, ",");
			}

			temp_RV_Rcvd = myatoi(StrTokStr1[1]);
			temp_YV_Rcvd = myatoi(StrTokStr1[2]);
			temp_BV_Rcvd = myatoi(StrTokStr1[3]);
			temp_RC_Rcvd = myatoi(StrTokStr1[4]);
			temp_YC_Rcvd = myatoi(StrTokStr1[5]);
			temp_BC_Rcvd = myatoi(StrTokStr1[6]);

			s_nMSettings.m_CalRVoltage_Rx = temp_RV_Rcvd * 0.01;
			s_nMSettings.m_CalYVoltage_Rx = temp_YV_Rcvd * 0.01;
			s_nMSettings.m_CalBVoltage_Rx = temp_BV_Rcvd * 0.01;
			s_nMSettings.m_CalRCurrent_Rx = temp_RC_Rcvd * 0.001;
			s_nMSettings.m_CalYCurrent_Rx = temp_YC_Rcvd * 0.001;
			s_nMSettings.m_CalBCurrent_Rx = temp_BC_Rcvd * 0.001;

			(s_nMSettings.m_CalRVoltage_Rx <= 0.0) ? (s_nMSettings.m_CalRVoltage_Rx = 1.0) : 0;
			(s_nMSettings.m_CalYVoltage_Rx <= 0.0) ? (s_nMSettings.m_CalYVoltage_Rx = 1.0) : 0;
			(s_nMSettings.m_CalBVoltage_Rx <= 0.0) ? (s_nMSettings.m_CalBVoltage_Rx = 1.0) : 0;
			(s_nMSettings.m_CalRCurrent_Rx <= 0.0) ? (s_nMSettings.m_CalRCurrent_Rx = 1.0) : 0;
			(s_nMSettings.m_CalYCurrent_Rx <= 0.0) ? (s_nMSettings.m_CalYCurrent_Rx = 1.0) : 0;
			(s_nMSettings.m_CalBCurrent_Rx <= 0.0) ? (s_nMSettings.m_CalBCurrent_Rx = 1.0) : 0;

			// sprintf(IMEI,"%02X%02X%02X%02X%02X%02X",my_eeprom_ID[0],my_eeprom_ID[1],my_eeprom_ID[2],my_eeprom_ID[3],my_eeprom_ID[4],my_eeprom_ID[5]);
			//	MqttInitStatus=0;

			sprintf(buf, "\n line :%d\n", __LINE__);
			sAPI_UartPrintf(buf);
		}
		ret = sAPI_fclose(file_hdl);
		if (ret != 0)
		{
			//  sAPI_UartPrintf("c:/calset.txt sAPI_fclose err",f_pumpno);
			sprintf(buf, "c:/calset.txt sAPI_fclose err ret=%d: \r\n", ret);
			sAPI_UartPrintf(buf);
		}
		else
		{
			file_hdl = NULL;
		}
	}
}

void WritecalsetFile()
{

	INT32 ret;
	UINT32 writeedlen, temp_RV_Rcvd = 0, temp_YV_Rcvd = 0, temp_BV_Rcvd = 0, temp_RC_Rcvd = 0, temp_YC_Rcvd = 0, temp_BC_Rcvd = 0;
	SCFILE *file_hdl = NULL;
	unsigned char filename[100];

	sprintf(filename, "c:/calset.txt");
	strcpy((char *)pfile, (char *)filename);

	file_hdl = sAPI_fopen((UINT8 *)pfile, "wb");
	if (file_hdl == NULL)
	{
		sAPI_UartPrintf("sAPI_fopen err");
	}

	sprintf(textBuf, "FileOpenEx Create (%s,%08x)=%d, line:%d\r\n", pfile, FS_CREATE, ret, __LINE__);
	sAPI_UartPrintf(textBuf);

	memset(textBuf, 0, 50); // 55
	ret = sAPI_fseek(file_hdl, 0, FS_SEEK_BEGIN);
	sprintf(textBuf, "FileSeek()=%d: \r\n", ret);
	sAPI_UartPrintf(textBuf);

	memset(textBuf, 0, 50); // 50
	sprintf(buf, " before write PrvAutoMobileKey is %d", PrvAutoMobileKey);
	sAPI_UartPrintf(buf);

	temp_RV_Rcvd = s_nMSettings.m_CalRVoltage_Rx * 100;
	temp_YV_Rcvd = s_nMSettings.m_CalYVoltage_Rx * 100;
	temp_BV_Rcvd = s_nMSettings.m_CalBVoltage_Rx * 100;
	temp_RC_Rcvd = s_nMSettings.m_CalRCurrent_Rx * 1000;
	temp_YC_Rcvd = s_nMSettings.m_CalYCurrent_Rx * 1000;
	temp_BC_Rcvd = s_nMSettings.m_CalBCurrent_Rx * 1000;

	sprintf(textBuf, "calset,%03d,%03d,%03d,%05d,%05d,%05d,\n\r", temp_RV_Rcvd, temp_YV_Rcvd, temp_BV_Rcvd, temp_RC_Rcvd, temp_YC_Rcvd, temp_BC_Rcvd);
	sAPI_UartPrintf(textBuf);

	writeedlen = sAPI_fwrite((UINT8 *)textBuf, strlen((char *)textBuf), 1, file_hdl);
	if (writeedlen != strlen((char *)textBuf))
	{
		sprintf(buf, "sAPI_fwrite err write length: %d\r\n", writeedlen);
		sAPI_UartPrintf(buf);
	}

	ret = sAPI_fclose(file_hdl);
	if (ret != 0)
	{
		sAPI_UartPrintf("sAPI_fclose err");
	}
	else
	{
		file_hdl = NULL;
	}
}

#if 1
void readonoffbuff(void)
{
	SCFILE *file_hdl = NULL;
//	char StrTokStr[30][20];	//dg_commented_memory
	int StrTokStrVer = 0;
	char *Pch = NULL;
	int Tp;
	int j;
	INT32 ret;
    UINT32 readedlen;
	sAPI_UartPrintf("In OnOff Read\n");
	strcpy((char *)pfile,(char*)"onoffbuf.txt");
	 file_hdl = sAPI_fopen((UINT8*)pfile, "ab");
                if(file_hdl == NULL)
                {
                    sAPI_UartPrintf("sAPI_fopen err");
                    
                }
 
	sprintf(textBuf,"FileOpenEx Create (%s,%08x)=%d\r\n", pfile, "wb",ret);
	sAPI_UartPrintf(textBuf);
	
	memset(textBuf,0,800);
	ret = sAPI_fseek(file_hdl, 0 , FS_SEEK_BEGIN);
	sprintf(textBuf,"FileSeek()=%d: \r\n",ret);
	sAPI_UartPrintf(textBuf);

	memset(strBuf,0,1200);
	readedlen =sAPI_fread((unsigned char *)strBuf, 800,1,file_hdl);
 
	sprintf(textBuf,"FileRead()=%d: readedlen=%d, strBuf=%s\r\n",ret, readedlen, strBuf);
	sAPI_UartPrintf(textBuf);
	if(readedlen>1)
	sprintf(Buffer4,"%s",strBuf);
	
	sprintf(textBuf,"OnOffBuff : %s\n",Buffer4);
	sAPI_UartPrintf(textBuf);
	sAPI_fclose(file_hdl);
	sprintf(textBuf,"sAPI_fclose()\r\n");
	sAPI_UartPrintf(textBuf);
	 
	 
}

#endif
void writeonoffbuff(void)
{
	SCFILE *file_hdl = NULL;
    INT32 ret;
    UINT32 writeedlen;
	int Tp=0;
	sAPI_UartPrintf("In OnOff Write\n");
	strcpy((char *)pfile,(char*)"onoffbuf.txt");
 	file_hdl = sAPI_fopen((UINT8*)pfile , "wb" );
	  if(file_hdl == NULL)
                {
                    sAPI_UartPrintf("sAPI_FsFileOpen err");
                    
                }	 
	sprintf(textBuf,"FileOpenEx Create (%s,%08x)=%d\r\n", pfile, FS_CREATE,ret);
	sAPI_UartPrintf(textBuf);

	memset(textBuf,0,800);
	memset(BigSMS,0,800);
	ret = sAPI_fseek(file_hdl, 0 , FS_SEEK_BEGIN);
	sprintf(textBuf,"FileSeek()=%d: \r\n",ret);
	sAPI_UartPrintf(textBuf);
	
	strcpy(BigSMS,Buffer4);
	sAPI_UartPrintf(BigSMS);
	
	writeedlen = sAPI_fwrite( (UINT8*)BigSMS, strlen((char*)BigSMS),1,file_hdl); 
	sprintf(textBuf,"FileWrite()=%d: writeedlen=%d\r\n",ret, writeedlen);
	sAPI_UartPrintf(textBuf);

	sAPI_fclose(file_hdl);
	sprintf(textBuf,"sAPI_fclose()\r\n");
	sAPI_UartPrintf(textBuf);
}

void ph_numcheck()
{
	UINT8 i;
	HowManyNumberFound = 0;
	NumSMS[0] = 0;
	NumSMS[1] = 0;
	NumSMS[2] = 0;
	NumSMS[3] = 0;
	if (strcmp(StoredPhoneNumber[11], "0000000000") && strlen(StoredPhoneNumber[11]) > 5)
	{
		ServiceNumberFound = 1;
		strcpy(ServiceNumber, StoredPhoneNumber[11]);
	}
	if (strcmp(StoredPhoneNumber[12], "0000000000") && strlen(StoredPhoneNumber[12]) > 5)
	{
		TargetNumberFound = 1;
		strcpy(TargetNumber, StoredPhoneNumber[12]);
	}
	for (i = 0; i < 4; i++)
	{
		if (strcmp(StoredPhoneNumber[i], "0000000000") && strlen(StoredPhoneNumber[i]) > 5)
		{
			NumberFound = 1;
			strcpy(SmsNumber[HowManyNumberFound], StoredPhoneNumber[i]);
			sAPI_UartPrintf("SmsNumber:%s", SmsNumber[i]);
			if (i == 1)
				NumSMS[0] = 1;
			if (i == 2)
				NumSMS[1] = 1;
			if (i == 3)
				NumSMS[2] = 1;
			if (i == 4)
				NumSMS[3] = 1;
			HowManyNumberFound++;
		}
	}
	sprintf(buf, "HowManyNumberFound %d\r", HowManyNumberFound);
	sAPI_UartPrintf(buf);
}

float CalculatePhToPh(float Val1, float Val2) // no_notadded
{
	double Tpc, Tp2c;

	Tpc = Val1 * Val1;
	Tp2c = Val2 * Val2;

	Tpc = (Tpc + Tp2c);
	Tp2c = Val1 * Val2;

	Tpc = Tpc + Tp2c;

	Tp2c = sqrt(Tpc);

	return Tp2c;
}
void FloatroString1Dig(char *String, float Number) // no_notadded
{
	int i, j;
	unsigned char c;
	float Itp;
	c = 0;
	if (Number < 0)
	{
		c = 1;
		Number = -Number;
	}
	i = (int)Number;
	Number = Number - (float)i;
	j = ((Number * 10.0)); //+0.5);
	if (c == 0)
		sprintf(String, "%02d.%01d", i, j);
	else
		sprintf(String, "-%02d.%01d", i, j);
	sprintf(buf, "String %s", String);
	sAPI_UartPrintf(buf);
}

void sAPP_Timer1(void *data)
{
//	char TempBuf1[500] = "";
	int HowManySoundPlayed = 0;
	// BOOL Ginitdis=1;
	//  char delims[] = ",";
	//  char *result = NULL;
	//  int TpStrtokVer;
	//  int EndLoop = 0;
	//  int Tp = 0;
	//  int Tp3 = 0;
	//  int Tp1 = 0;
	//  int Tp4 = 0;
	//  int Tp2 = 0;
	//  int tm_min=0,max=0,step=0,fert=0;
	int i = 0;
	int j = 0;
	int program = 0;
	char *TpStr = NULL;
	char *pData = NULL;
	char *p = NULL;
	unsigned char RingVerDelay;
	unsigned char PowerIsThere;
	int FirstTimeSMS;
	unsigned int StrTokStrVer = 0;
	//	unsigned char TargetNumberFound = 0;
	int NumberLocation;
	// unsigned char NumberFound;
	unsigned char NumnerNotAvaliable;
	unsigned char HowManyNumbrtoSearch = 1;
	unsigned char OkPending;
	unsigned char WaitOK;
	// unsigned char SMSOK;
	// unsigned char SENDATA=0;
	unsigned char SaveNumberPos = 1;
	unsigned char ThisSMSisNotPowerFault = 0;
	unsigned char NeedToCPBRSearchAgain = 0;
	int len;
	void *pointer = 0;
	int started_firsttime = 0;
	int NumberOfSMSSend;
	// char WhoMadeRelayOn[20]="\n";
	char switchonofflag = 0;
	unsigned char NeedToSendSMSCall;
	unsigned char PowerOnSms;
	unsigned char CallReady = 0;
	unsigned int StatusSMSDelay1, StatusSMSDelay2;
	INT32 pin;
	INT32 mod = 0;
	INT32 pinpullenable = 0;
	INT32 pindirection = 0;
	INT32 pinlevel = 0;
	INT32 iret;
	unsigned char DelSMS;
	int RecheckPinCounter = 0;
	int SmsSendOntimer = 0;
	unsigned char WritePhoneNumber = 0;
	// unsigned char cpbrsearchend1;
	// unsigned char cpbrsearchend2;
	// unsigned char cpbrsearchend;
	// unsigned char Callreceiv;
	unsigned int PingHighSmsSendOntimer = 0;
	unsigned int PingHighRecheckPinCounter = 0;
	// unsigned char callreceived = 0;
	char startReady = 0;
	int ArrayCount;
	BOOL keepGoing = TRUE;
	unsigned char ReadSMS;
	// unsigned char MakeRealyOn = 0;
	// unsigned char OnByTarget = 0;
	char DTmfValue[2] = {0};
	INT32 ret;
	UINT32 writeedlen;
	UINT32 readedlen;

	unsigned char NumberChangeSMS = 0;
	// unsigned char s_nMSettings.m_Enter=0;
	unsigned char SMSDelay = 0;
	unsigned char AllSmsSendDone = 0;
	unsigned char SMS30MinStatus = 0;
	// unsigned char CallOnOfVer;
	// unsigned char StartByMobile = 0;
	int ChangeSoundStatus = 0;
	int NumberOfSmsAllreadySend;
	unsigned char Tpsms = 0;
	// unsigned char RegxSmsSend = 0;
	unsigned char PowerOffSMS = 0;
	// unsigned char PowerCurrentCondition = 0;

	unsigned char NumSMS[5];
	unsigned char WaitOkPendingDelay = 0;
	int ControllerDataCounter = 0;
	char *ControllerString = NULL;

	unsigned char AutoMobileKey = 0;
	//	unsigned char PrvAutoMobileKey = 0;
	//	unsigned char checkpower = 0;
	unsigned char Ph2Ph3Selection, PrvPh2Ph3Selection;
	unsigned char SMS30MinStatusNumber = 0;
	unsigned char SMS30MinStatusDelay = 0;
	unsigned char CheckThisWhy = 0;
	unsigned char NowPleaseDoCall = 0;
	unsigned char GotCMT = 0;

	// unsigned char CallConnected = 0;
	double Tpc, Tpc2;
	long CallConnectedDelay = 0;
	// Mobile2Phs3Phase = 0;
	Ph2Ph3Selection = 0;
	PrvPh2Ph3Selection = 3;
	memset(&uart_read_buf, 0, 200);
	// memset(&modem_read_buf,0,200);
	memset(&buffer, 0, 200);
	memset(&Buffer1, 0, 500);
	//	memset(&TpStrtok,0,600);
	//	memset(&Sms_Number,0,10);
	//	memset(&SaveNumber,0,20);
	memset(&SendSMSOnThisNumber, 0, 20);
	memset(&Recive_Sms_Number, 0, 20);
	memset(&Recive_call_Number, 0, 20);
	memset(&WhoMadeRelayOn, 0, 20);
	memset(&WhoMadeenterRelayOn, 0, 20);
	//		memset(&DateTpStr,0,30);
	//		memset(&TimeTpStr,0,30);
	// memset(&IMEI,0,16);
	memset(&DIS_BUF, 0, 80);
	memset(&VAL_BUF, 0, 80);
	memset(&SMS_BUF, 0, 200);
	//		memset(&TargetNumber,0,30);
	//		memset(&ModemStr,0,50);
	//		memset(&PhoneNumber,0,30);
	// memset(&limitsmscount,0,10);
	memset(&ServiceNumber, 0, 20);
	memset(&BigSMS, 0, 1000);
	memset(&BigSMS2, 0, 100);
	memset(&StoredPhoneNumber, 0, 500);
	memset(&TCPWifigprsstrBUFF, 0, 1350);  //650 //dg_nsdk
	memset(&SmsStrNumber, 0, 12875); //8500 //dg_nsdk
	// memset(&TCPwifiStrNumber,0,150000);
	// memset(&Triprec,0,4004);
	memset(&StrTokStr1, 0, 1000); // 4004 dg_nsdk
	memset(&StrTokStr2, 0, 1700); // 4004 dg_nsdk
	// NumberOfSmsNeedToSend = 0;
	NumberOfSmsAllreadySend = 0;
	nMSettings.ndebugonof = 0;
	// timeoutPeriod = 300;
	// sAPI_TimerStart(EAT_TIMER_3,300);

	// sAPI_TimerStart(timerRef,1,800,NULL,TASK);

	NumberLocation = 1;
	/* nMoTr.ActCycLicOnDelay = 0;
	nMoTr.ActCycLicOfDelay = 0; */

	NumberFound = 0;
	NumnerNotAvaliable = 0;
	HowManyNumberFound = 0;
	OkPending = 0;
	//      MakeCallDone = 0;
	//      CallDelayCounter = 0;
	SendSMS = 0;
	WaitOK = 0;
	cpbrsearchend = 0;
	cpbrsearchend1 = 1;
	cpbrsearchend2 = 1;
	ReadSMS = 0;
	SendSmsToAll = 0;
	NumberOfSMSSend = 20;
	RingVerDelay = 0;
	Callreceiv = 0;
	NeedToSendSMSCall = 0;
	DelSMS = 0;
	PowerOnSms = 0;
	PingHighSmsSendOntimer = 0;
	PingHighRecheckPinCounter = 0;
	CallOnOfVer = 0;
	ModemIsReady = 0;
	PowerIsThere = 0;
	StatusSMSDelay1 = 0;
	StatusSMSDelay2 = 0;
	TargetNumberFound = 0;
	STATE_SENDSMS = STATE_NO_SMS;

	nMSettings.SMSOnOff = 0;
	nMSettings.SfbOnOff = 0;
	nMSettings.TankOnOff = 0;
	nMSettings.SumpOnOff = 0;
	nMSettings.PressureOnOff = 0;
	nMSettings.ManualswitchOnOff = 0;

	nMSettings.DryRunOnOff = 0;
	nMSettings.TargetOnOff = 0;
	nMSettings.VBFOnOff = 0;
	nMSettings.RelayControlOnCall = 0;

	NumSMS[0] = 0;
	NumSMS[1] = 0;
	NumSMS[2] = 0;
	NumSMS[3] = 0;
	wifiresetflag1 = 1;
	SendValveOnOff.ValveStausFlag=1;
	SendValveOnOff.ValveNo=255;
	/*	nTimerSettings.POnHr = 0;   //dg_nsdk
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
		nTimerSettings.RTCOnOf = 0; */

	for (i = 1; i < 5; i++)
	{
		nTimerSettings.RTCOnHr[i] = 0;
		nTimerSettings.RTCOnMin[i] = 0;
		nTimerSettings.RTCOnSec[i] = 0;
		nTimerSettings.RTCOfHr[i] = 0;
		nTimerSettings.RTCOfMin[i] = 0;
	}
	//      zonecom.foggerRTCOnOf=0;
	//		SetToGreenFlag = 0;
	//		Stg3FirstTimer = 0;
	//		Stg2FirstTimer = 0;
	//	 	nTimerSettings.Drrestartpoweronof=0;
	// ReadPhoneNumber();
	// ReadInstDate();

	Motoronflag[0] = 0, Motorreasonflag[0] = 0, act_POnMin[0] = 0, act_POnSec[0] = 0, act_rem_delmin[0] = 0, act_rem_delsec[0] = 0, act_del_comp_min[0] = 0, act_del_comp_sec[0] = 0, Act_level[0] = 0.0, flowisthere[0] = 0;
	Motoronflag[1] = 0, Motorreasonflag[1] = 0, act_POnMin[1] = 0, act_POnSec[1] = 0, act_rem_delmin[1] = 0, act_rem_delsec[1] = 0, act_del_comp_min[1] = 0, act_del_comp_sec[1] = 0, Act_level[1] = 0.0, flowisthere[1] = 0;
	Motoronflag[2] = 0, Motorreasonflag[2] = 0, act_POnMin[2] = 0, act_POnSec[2] = 0, act_rem_delmin[2] = 0, act_rem_delsec[2] = 0, act_del_comp_min[2] = 0, act_del_comp_sec[2] = 0, Act_level[2] = 0.0, flowisthere[2] = 0;
	
	uart_send_count = 0, Resend_IMEI_count = 0;uart_send_flag=0;Resend_IMEI_flag=0;
	
	nMSettings.ndebugonof=1;
	
	InitParameter(); // INIT

	TargetNumberFound = 0;
	SC_STATUS status;
	///**********For checking the data send and receive*********///
	/*		s_nMSettings.m_LowVoltOnOff=1;
			s_nTimerSettings.m_LowVoltII=450;
			s_nTimerSettings.m_DiffVoltII=050;
			s_nTimerSettings.m_LowVoltIII=435;
			s_nTimerSettings.m_DiffVoltIII=030;
			s_nMSettings.m_HighVoltOnOff=1;
			s_nTimerSettings.m_HighVoltII=480;
			s_nTimerSettings.m_HiDiffVoltII=060;
			s_nTimerSettings.m_HighVoltIII=440;
			s_nTimerSettings.m_HiDiffVoltIII=020;
			s_nMSettings.m_SppOnoff=1;
			s_nTimerSettings.m_ImbVolt=15;
			s_nMSettings.m_RvePhOnoff=1;
			s_nMSettings.m_AutoStIIOnOff[0]=0;*/
	// array not needed.
	///************************************************************///
	status = sAPI_TimerCreate(&timer1);
	status = sAPI_TimerStart(timer1, 60, 60, timerRoutine1, 0x1234);
//	status = sAPI_TimerStart(timer1, 160, 160, timerRoutine1, 0x1234);

	while (1)
	{

#ifdef USE_FLAG
		UINT32 event = 0;
		status = sAPI_FlagWait(g_flg1, TIMER1_OUT_EVENT_MASK, SC_FLAG_OR_CLEAR, &event, /*200*/ SC_SUSPEND);
		sAPI_Debug("status[%d] event[%d]", status, event);
		sAPI_UartPrintf("%s status %d!!\n", status); // dg_nsdk
	//	#if 0
	//	WriteonofFile();
		nMSettings.ndebugonof=1;
		nMSettings.SMSOnOff=1; //dg_nsdk
		if (status == SC_SUCCESS)
		{

			if (ModemIsReady == 1 && s_nMSettings.m_Enter == 0)
			{
				ReadcalsetFile();
				ReadFile();
				ReadTopicsetFile();
				s_nMSettings.m_Enter = 1;
				enter1 = 1;
				ReadPhoneNumber();
				// ReadInstDate();
				// ReadDprevSettings();
				// ReadDprevSettings1();
				ReadTimerSettings();
				ReadonofFile();
				// readidcomset();
				// ReadDripSettings(nVaTr.programselection);
				//  nVaTr.sendremaintime=0;
				//  nDripSettings.stp=nDripSettings.prvstp;
				//  nDripSettings.ststp=nDripSettings.prvststp;
				//  nDripSettings.calc=nDripSettings.stp;
				//  nVaTr.REMTIM=0;
				//  nVaTr.fertv1Smsonof =0;nVaTr.fertv2Smsonof=0;nVaTr.fertv3Smsonof=0;
				// nDripSettings.startcontrol=1;
				//  getonext=1;
				//  nDripSettings.startcontrolt=0;
				//  firstvalesmsonof=1;
				//  firstvalestsmsonof=1;

				/* Readmotordata(0);
				Readmotordata(1);
				Readmotordata(2);
				Readmotordata(3);
				Readmotordata(4);
				Readmotordata(5);
				Readmotordata(6); */
				sprintf(buf, "m_Enter=%d: calling ReadctsetFile \r\n", s_nMSettings.m_Enter);
				sAPI_UartPrintf(buf);
				ReadctsetFile();
				ReadEpromidFile();
			}

			// sprintf(buf,"fiid =%ld fertonof1=%ld",zonecom.f1id,nVaTr.fertv1onof);
			// sAPI_UartPrintf(buf);
			if (ModemIsReady == 1 && s_nMSettings.m_Enter <= 78)
			{
				if (s_nMSettings.m_Enter == 1)
					ReadtnkconfigFile();
				// {}
				else if (s_nMSettings.m_Enter == 4)
				{
					ReadvolsetFile();
					if (enter1 == 1)
					{

						//	iret = sAPI_GpioSetValue(WIFIRST, SC_GPIORC_HIGH); //DG_NSDK
					}
				}
				else if (s_nMSettings.m_Enter == 7)
					//	{}
					ReadcursetFile(1);
				else if (s_nMSettings.m_Enter == 10)
					//	{}
					ReadcursetFile(2);
				else if (s_nMSettings.m_Enter == 13)
				{
					ReadcursetFile(3);
				}
				else if (s_nMSettings.m_Enter == 16)
				{
					ReaddelsetFile(1);
					if (enter1 == 1)
					{

						//	iret = sAPI_GpioSetValue(WIFIRST, SC_GPIORC_LOW); //DG_NSDK
					}
				}
				else if (s_nMSettings.m_Enter == 19)
					ReaddelsetFile(2);
				//{}

				else if (s_nMSettings.m_Enter == 21)
					ReaddelsetFile(3);
				//	{}

				else if (s_nMSettings.m_Enter == 24)
					ReadRTCsetFile(1);
				//	{}
				else if (s_nMSettings.m_Enter == 27)
					ReadRTCsetFile(2);
				//	{}//
				else if (s_nMSettings.m_Enter == 30)
					ReadRTCsetFile(3);
				//	{}//
				else if (s_nMSettings.m_Enter == 33)
				{						// Readmotordata(0);
					ReadTopicsetFile(); // subash commented
				}
				//	readactferttimerset(nVaTr.programselection,4);
				else if (s_nMSettings.m_Enter == 36)
				{ // Readmotordata(1);
						ReadEcosetFile();
				}
				//	readactzonetimerset(nVaTr.programselection);
				else if (s_nMSettings.m_Enter == 39)
				{ // Readmotordata(2);
				ReadEpromidFile();
				}
				//	readactidlmset(nVaTr.programselection);
				else if (s_nMSettings.m_Enter == 42)
				{ // Readmotordata(3);
				}
				//	readmodeonofset();
				else if (s_nMSettings.m_Enter == 45)
				{ // Readmotordata(4);
				}
				//	readidcomset();
				else if (s_nMSettings.m_Enter == 48)
				{
					// Readmotordata(5);
					//	readidset(1);
				}
				else if (s_nMSettings.m_Enter == 51)
				{
					//	Readmotordata(6);
					//	readidset(2);
				}
				else if (s_nMSettings.m_Enter == 54)
				{
					ReadctsetFile(); // comment removed subash
				}
				else if (s_nMSettings.m_Enter == 57)
				{
					memset(textBuf,0x00,sizeof(textBuf));
					sprintf(textBuf,"$:5:29:114:\r");
					//sprintf(textBuf,"$MI\r");
					sAPI_UartWrite(SC_UART, textBuf, strlen(textBuf));
					sAPI_UartPrintf(textBuf);
					//	readidset(4);
				}
				else if (s_nMSettings.m_Enter == 60)
				{
					// iret = sAPI_GpioSetValue(WIFIRST, SC_GPIORC_HIGH);
					if (enter1 == 1)
					{
						// wifiresetflag=1;
						// eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 6, "CONTCP", EAT_NULL);
					}
					//	readidcalset(1);
				}
				else if (s_nMSettings.m_Enter == 63)
				{
					//	readidcalset(2);
					//	readgreenrtcset();
					//	readgreenonofset();
				}
				else if (s_nMSettings.m_Enter == 65)
				{
					// iret = sAPI_GpioSetValue(WIFIRST, SC_GPIORC_HIGH);
					if (enter1 == 1)
					{
						sAPI_UartPrintf("AT cmd Entry\n\r");
						// SMSMno_Read();
						// simcom_gsm_init("1234",GsmInitCallback);
						enter1 = 0;
					}
					// iret = sAPI_GpioSetValue(WIFIRST, SC_GPIORC_LOW);
					// Wifi_init();
					// wifiresetflag=1;
				}

				else if (s_nMSettings.m_Enter == 68)
				{
					// Wifi_init();
				}

				else if (s_nMSettings.m_Enter == 64)
				{
#if 0
								sprintf(buf,"power_on_flag_count is %d",power_on_flag_count);
								sAPI_UartPrintf(buf);
								if(power_on_flag_count==1)
								{
								  ReadonofFile();
								 if(MakeRealyOn == 1)
										{
											OverAllStarterTrip = 0;
											nMoTr.ActMaxRunTimer = 0;

											
											if(nTimerSettings.RTCOnOf==0 || nVaTr.cyclecompleted==1)
											{
											StartByMobile = 1;
											MakeRealyOn = 1;
											nSTATE_STATUS_SMS = STATUS_MOTOR_ON_SWITCH;
											// nSTATE_MOTOR_ON_SMS=STATE_MOTOR_ON_SWITCH_SMS;
											}
											else
											{
												StartByMobile = 0;
											MakeRealyOn = 0;
											}
											OnFromMobile = 0;
											OffMotorByPhone = 0;
											// MotorStarterTripCount = 0;
											ActMotorStarterTripTimer =0;
											nTimerSettings.Drrestartpoweronof=0;
											sprintf(buffer,"MANUAL");
									        strcpy(WhoMadeRelayOn,buffer);
										//	WriteonofFile();  //19
										//	ReadonofFile();
										}
									else
									{
										// MotorStarterTripCount = 0;
										OverAllStarterTrip = 0;
										ActMotorStarterTripTimer =0 ;
										nMoTr.ActMaxRunTimer = 0;
										nSTATE_STATUS_SMS =STATUS_MOTOR_ON_SWITCH_TRIP_SMS;
										SendSmsToAll = 1;
										nSTATE_MOTOR_SMS =STATE_MOTOR_ON_SWITCH_TRIP_SMS;
										// nSTATE_MOTOR_ON_SMS=STATE_MOTOR_ON_DEFAULT;
										StartByMobile = 0;
										MakeRealyOn = 0;
                                        nCYCTimer = NO_TCOND;
										OffMotorByPhone = 0;
									 //	if(nVaTr.cyclecompleted==1)
									 //	nDripSettings.startcontrol=1;
									 //	nVaTr.cyclecompleted=0;
									 //	dripcycledate=0;
							          //   dripcyclecount=0;
                                        if(nMSettings.SumpOnOff == 1)
	                                    PrvLoMotorStatus=0;
                                        PrvUpMotorStatus=0;
									
							            PreviousTrip = NO_TRIP_FLAG;
										nTimerSettings.Driprestartpoweronof=0;
										NoAcceptSMS=0;
										sprintf(buffer,"MANUAL");
									    strcpy(WhoMadeRelayOn,buffer);
										sAPI_UartPrintf("Will make Off");
										nTimerSettings.Driprestartpoweronof=0;
									//	 WriteonofFile();
									}
								}
								power_on_flag_count=2;

#endif
				}
				s_nMSettings.m_Enter++;
				sprintf(buf, "THE VALUE OF s_nMSettings.m_Enter IS:{%d}", s_nMSettings.m_Enter);
				sAPI_UartPrintf(buf);
			}
			/*if(s_nMSettings.m_Enter==76 && GSMInitDone==1  )
			  //eat_send_msg_to_user(EAT_USER_0, EAT_USER_1, EAT_FALSE, 1, "C", EAT_NULL);
			  if(s_nMSettings.m_Enter==150  )
			 //  simcom_gsm_init("1234",GsmInitCallback);



			  if(s_nMSettings.m_Enter==253 && GSMInitDone==1  )
			   Bearer_enable(BearerCallback);*/

			checkpower = checkpower_act;
			sprintf(buf, "\n\r line %d RVoltage1 = %d  YVoltage1 = %d BVoltage1 %d checkpower_act %d,checkpower %d,PowerCurrentCondition %d,\n\r", __LINE__, RVoltage1, YVoltage1, BVoltage1, checkpower_act, checkpower, PowerCurrentCondition);
			sAPI_UartPrintf(buf);

			if (DebounceDelay == 1)
			{
				if (DebounceDelaycounter++ > 3)
				{
					DebounceDelaycounter = 0;
					DebounceDelay = 0;
				}
			}
			if (dtmfDebounceDelay == 1)
			{
				if (dtmfDebounceDelaycounter++ > 20)
				{
					dtmfDebounceDelaycounter = 0;
					dtmfDebounceDelay = 0;
				}
			}
			/* if(getmoistureflag==1&& MakeRealyOn==0)
			{
			if(getsensorcounter++>3)
			{
			getsensorcounter=0;
			getmoisturecounter++;
			if(getmoisturecounter<nDripSettings.decidefirst)
			getmoisturecounter=nDripSettings.decidefirst;
			if(getmoisturecounter>nDripSettings.decidelast||getmoisturecounter>64)
			getmoistureflag=0;
			if( zone[getmoisturecounter].actm1id>0&& (zoneonof[nVaTr.programselection].moistureonof==1||zoneonof[nVaTr.programselection].moisturemodeonof==1))
			{
			sprintf(textBuf,"$D,M,%ld,1,%ld,%ld,%ld\n\r ",getmoisturecounter,zoneid[zone[getmoisturecounter].actm1id].ida1,zoneid[zone[getmoisturecounter].actm1id].ida2,zoneid[zone[getmoisturecounter].actm1id].ida3 );
			sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
			sprintf(buf,"%s\n\r ",textBuf);while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
			sAPI_UartPrintf(buf);
			}
			}
			}
			else
			getmoistureflag=0;

			if(getlevelflag==1&& MakeRealyOn==0)
			{
			if(levelsensorcounter++>3)
			{
			levelsensorcounter=0;
			getlevelcounter++;
			if(getlevelcounter<nDripSettings.decidefirst)
			getlevelcounter=nDripSettings.decidefirst;
			if(getlevelcounter>nDripSettings.decidelast||getlevelcounter>64)
			getlevelflag=0;
			if( zone[getlevelcounter].actm1id>0&& (zoneonof[nVaTr.programselection].levelmodeonof==1))
			{
			sprintf(textBuf,"$D,M,%ld,1,1,%ld,%ld,%ld\n\r ",getlevelcounter,zoneid[zone[getlevelcounter].actm1id].ida1,zoneid[zone[getlevelcounter].actm1id].ida2,zoneid[zone[getlevelcounter].actm1id].ida3 );
			sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
			 sprintf(buf,"%s\n\r ",textBuf);while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
			sAPI_UartPrintf(buf);
			}
			}
			}
			else
			getlevelflag=0; */
			//  strcpy(textBuf,"$D,M,5,1,4,003,255,254,\n\r ");
			//  sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));

			// sprintf(buf,"%s\n\r ",textBuf);while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
			// sAPI_UartPrintf(buf);
			sAPI_UartPrintf(",CSQ:%d,%d,%d,%d,%d,\n\r",CSQ,Creg,CGREG,Cpin,RST_count);
			
			if (rebootflag == 1)
			{
				tcpdcounter1 = 0;

				if (rebootflagcounter++ > 5)
				{
					rebootflag = 0;
					rebootflagcounter = 0;
					tcpdcounter1 = 0;
					// eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 6, "CONRBT", EAT_NULL);
				}
			}
			else if (rebootflag1 == 1)
			{
				tcpdcounter1 = 0;

				if (rebootflagcounter++ > 5)
				{
					rebootflag1 = 0;
					rebootflagcounter = 0;
					tcpdcounter1 = 0;
					// eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 6, "CONTCP", EAT_NULL);
				}
			}
#if 0
	                        if((wifiresetflag==1||wifiresetflag1==1)&&s_nMSettings.m_Enter>70)
							{
                            if(DeviceConfig.interface == WIFI)
								wifireset();
							}
#endif
			/*  if(ChangeSoundStatus++>=10) //DG_NSDK
			  {
				  NowOff = 0;
				  NowStarted = 0;
				  ChangeSoundStatus = 0;

			  } */

			/* if (Balanceflag == 1 && BalanceSend != 0)
			{
				if (strstr((char *)BalanceStr, "+CUSD:") != 0 && cpbrsearchend == 1)
				{
					//	 UINT8  *p = EAT_NULL;
					//   p = BalanceStr;
					BalanceSend = 0;
					//	     sprintf(buf,"\n\rRING STRING IS :- %s\n\r",p+7);
					//   sAPI_UartPrintf(buf);
					// strcpy(BalanceStr,p);
					//	send_Balance(); //dg_nsdk
					Balanceflag = 0;
				}
			} */
			sprintf(buf, "DBG: cpbrsearchend = %d", cpbrsearchend); // this is 0 in n-,stored no in n+
				sAPI_UartPrintf(buf);
			if (cpbrsearchend == 0 && s_nMSettings.m_Enter > 78) // 70
			{
				sAPI_UartPrintf("\n\r DBG: cpbrsearchend == 0 line %d",__LINE__);
				ReadPhoneNumber();
				ph_numcheck();
				cpbrsearchend = 1;
				send_Smsno(SmsNumber[0]);
			}
			if (cpbrsearchend2 == 0 && s_nMSettings.m_Enter > 70)
			{
				sAPI_UartPrintf("\n\r DBG: cpbrsearchend == 0 line %d",__LINE__);
				ReadPhoneNumber();
				ph_numcheck();
				cpbrsearchend2 = 1;
				//	 send_Smsno(SmsNumber[0]);
			}

			if (cpbrsearchend1 == 0 && s_nMSettings.m_Enter > 78) // 70
			{
				sAPI_UartPrintf("\n\r DBG: cpbrsearchend == 0line %d",__LINE__);
				//	 ReadPhoneNumber();
				//	 ph_numcheck();
				//	 cpbrsearchend1 = 1;
				send_Smsno(StoredPhoneNumber[22]);
				//	 strcpy(StoredPhoneNumber[0],StoredPhoneNumber[22]);
				sprintf(buf, "DBG: cpbrsearchend22 == %s", StoredPhoneNumber[22]); // STORED phone num
				sAPI_UartPrintf(buf);
				sprintf(buf, "DBG: cpbrsearchend 0== %s", StoredPhoneNumber[0]); // this is 0 in n-,stored no in n+
				sAPI_UartPrintf(buf);
				WritePhoneNumberFn();
				ReadPhoneNumber();
				ph_numcheck();
				//	send_Smsno2(SmsNumber[0]);  //dg_nsdk
				cpbrsearchend1 = 1;
				// cpbrsearchend = 1;
				sprintf(buf, "DBG: cpbrsearchend == %s", SmsNumber[0]); // this is 0 in n-,stored no in n+
				sAPI_UartPrintf(buf);
			}
			if ((creg_reset_flag == 1) && (s_nMSettings.m_Enter > 78))
			{
				creg_reset_flag = 0;
				WriteonofFile();
				send_restartsms(SmsNumber[0]);
			}
			if (nMSettings.ndebugonof == 1)
				sprintf(buf, "StoredPhoneSmscode[0]=%s,DeviceConfig.MobileSmscode[0]=%s", StoredPhoneSmscode[0], DeviceConfig.MobileSmscode[0]);
			sAPI_UartPrintf(buf);
#if 0
						if((nSTATE_MOTOR!=STATE_MOTOR_POWERONDELAY) && (nSTATE_MOTOR!=STATE_MOTOR_STARDELTADELAY)) //no_notadded
							{
							VolCurGetTimer++;
                            if(PowerCurrentCondition == 0 && VolCurGetTimer>=5)
							{
                            VolCurGetTimer = 0;
							CheckVoltageCurrent();
							}
							sAPI_UartPrintf("\n\rDebug:Before DisplayRoutine..\n\r");
						//	DisplayRountine();
							sAPI_UartPrintf("\n\rDebug:After DisplayRoutine..\n\r");
						//	CheckKey();
 							}
#endif
#if 0
							Ph2Ph3Selection = CheckPH2PH3Selection();

							if(Ph2Ph3Selection != PrvPh2Ph3Selection)  //dg_added_oro
							{

								if(Phase23selectionDelay++>=10)
								{
									Phase23selectionDelay =11;
									PrvPh2Ph3Selection = Ph2Ph3Selection;

									if(Ph2Ph3Selection == 1)
									{

										sprintf(buf,"\n\r2 Ph 3 Ph Switch Ph2Ph3Selection = %d PrvPh2Ph3Selection = %d\n\r",Ph2Ph3Selection ,PrvPh2Ph3Selection);
										sAPI_UartPrintf(buf);
									 	// ReadTimerSettings();
										nTimerSettings.AutoStIIOnOff = 1;
									 	// WriteTimerSettings();
										STATE_SENDSMS=STATE_AUTOSTII_SMS;
										SendSmsToAll = 1;
									 	// ReadTimerSettings();
									}
									else
									{
										sprintf(buf,"\n\r2 Ph 3 Ph Switch Ph2Ph3Selection = %d PrvPh2Ph3Selection = %d\n\r",Ph2Ph3Selection ,PrvPh2Ph3Selection);
										sAPI_UartPrintf(buf);
										//ReadTimerSettings();
									 	nTimerSettings.AutoStIIOnOff = 0;
										 // WriteTimerSettings();
										STATE_SENDSMS=STATE_AUTOSTII_SMS;
										SendSmsToAll = 1;
										//ReadTimerSettings();
									}
								}

							}
							else
								Phase23selectionDelay = 0;

							//sprintf(TempBuf1,"Niagara");
							//sAPI_UartWrite(SC_UART,(UINT8*)TempBuf1,strlen(TempBuf1));
							//sAPI_UartPrintf(TempBuf1);
							AutoMobileKey = CheckAutoMobile();   //dg_added_oro
							if((AutoMobileKey != PrvAutoMobileKey) && s_nMSettings.m_Enter>70)
							{
								if(AutoMobileKeyDelayOFF++>=10)
								{
									AutoMobileKeyDelayOFF = 11;
									sprintf(buf,"\n\rAutoMobile Key %d\n\r",AutoMobileKey);
									sAPI_UartPrintf(buf);
									if(AutoMobileKey == 1)
									{
										if(MakeRealyOn == 0)
										{
											OverAllStarterTrip = 0;
											nMoTr.ActMaxRunTimer = 0;
											nSTATE_STATUS_SMS = STATUS_MOTOR_ON_SWITCH;
											// nSTATE_MOTOR_ON_SMS=STATE_MOTOR_ON_SWITCH_SMS;
											StartByMobile = 1;
											MakeRealyOn = 1;
											OnFromMobile = 0;
											OffMotorByPhone = 0;
											// MotorStarterTripCount = 0;
											ActMotorStarterTripTimer =0;
											nTimerSettings.Drrestartpoweronof=0;
											sprintf(buffer,"MANUAL");
									        strcpy(WhoMadeRelayOn,buffer);
											PrvAutoMobileKey = AutoMobileKey;
											WriteonofFile();
											ReadonofFile();
											sAPI_UartPrintf("Will make On");

										}


									}
									else
									{
										OnByTarget=0;
										// MotorStarterTripCount = 0;
										OverAllStarterTrip = 0;
										ActMotorStarterTripTimer =0 ;
										nMoTr.ActMaxRunTimer = 0;
										nSTATE_STATUS_SMS =STATUS_MOTOR_ON_SWITCH_TRIP_SMS;
										SendSmsToAll = 1;
										nSTATE_MOTOR_SMS =STATE_MOTOR_ON_SWITCH_TRIP_SMS;
										// nSTATE_MOTOR_ON_SMS=STATE_MOTOR_ON_DEFAULT;
										StartByMobile = 0;
										MakeRealyOn = 0;
										livedataflagcount1=0;livedataflag1=1;
                                        nCYCTimer = NO_TCOND;
										OffMotorByPhone = 1;
										if(nVaTr.cyclecompleted==1)
										nDripSettings.startcontrol=1;
										nVaTr.cyclecompleted=0;
										nVaTr.no_fbk_flag=0;
										dripcycledate=0;
							            dripcyclecount=0;
                                        if(nMSettings.SumpOnOff == 1)
	                                    PrvLoMotorStatus=0;
                                        PrvUpMotorStatus=0;
										RecheckCounterUT=0;
										
							            PreviousTrip = NO_TRIP_FLAG;
										nTimerSettings.Driprestartpoweronof=0;
										NoAcceptSMS=0;
										sprintf(buffer,"MANUAL");
									    strcpy(WhoMadeRelayOn,buffer);
										sprintf(buf,"Will make Off");
										sAPI_UartPrintf(buf);
										PrvAutoMobileKey = AutoMobileKey;
										WriteonofFile();
										if(nVaTr.REMTIM<=180)
										{nVaTr.REMTIM=180;
									 sprintf(buf,"\n\r nVaTr.REMTIM %d on\n\r",nVaTr.REMTIM);
									 sAPI_UartPrintf(buf);
									 }
									}
								PrvAutoMobileKey = AutoMobileKey;
								}
							}
							else
								AutoMobileKeyDelayOFF = 0;

#endif

			//	sprintf(buf,"vbat = %d\r\n",sAPI_ReadVbat());
			//			sAPI_UartPrintf(buf);
			/* 					for(int i=0;i<10;i++)
										 {
											// adc[i]=0;
										//	adc_total=sAPI_ReadVbat();
											bat_volt_int=sAPI_ReadVbat();
											sprintf(buf,"bat_volt_int:%d\r\n",bat_volt_int);
											sAPI_UartPrintf(buf);
											avg_adc_total += bat_volt_int;
										 }
										avg_adc=avg_adc_total/10;
									//	BATPER=avg_adc*3.2;
									//
										BATPER=(float)(avg_adc*0.0237);
									//	BATPER=(float)(avg_adc/4500);
										BATPER_act=(int)(BATPER);
										if(BATPER_act>=100)
										BATPER_act=100;
										sAPI_UartPrintf("\n\r avg_adc_total %d Battery Percentage:%d,avg_adc %d BATPER %f\n\r",avg_adc_total,BATPER_act,avg_adc,BATPER);
										avg_adc=0;
										avg_adc_total=0; */
			//		if(datetime.tm_sec != ActpowerOnDelayPrvSec && PowerCurrentCondition == 0) //dg_added_oro
			 if (datetime.tm_sec != No_comm_prev_secs)    // dg_nsdk
			{
				No_comm_secs++;
				No_comm_prev_secs = datetime.tm_sec;
			}
			if (No_comm_secs >= 10)
			{
				No_comm_secs = 0;
				No_comm_flag = 1;
			}
			if (No_comm_flag == 1)
			{
				s_nMSettings.m_settings_req_flag = 0;
				s_nMSettings.m_settings_count = 0;
			} 

			sAPI_UartPrintf("%s enter to main %d!!\n"); // dg_nsdk
			#if 0
			if(RecValveOnOff.ValveStausFlag==SendValveOnOff.ValveStausFlag && RecValveOnOff.ValveStausFlag==2)
			{
				if(RecValveOnOff.ValveNo == SendValveOnOff.ValveNo)
				{
					for(int i=0;i<NoOfObject;i++)
					{
						if((nConfig[nProgram.nZone[ProgramNo-1].ValveNo[i]-1].Output_No & SendValveOnOff.ValveNo))	
						{
							nConfig[nProgram.nZone[ProgramNo-1].ValveNo[i]-1].Status=1;
						}
					}
				}
				else if(RecValveOnOff.ValveNo != SendValveOnOff.ValveNo)
				{
					for(int i=0;i<NoOfObject;i++)
					{
						if((nConfig[nProgram.nZone[ProgramNo-1].ValveNo[i]-1].Output_No & SendValveOnOff.ValveNo))	
						{
							nConfig[nProgram.nZone[ProgramNo-1].ValveNo[i]-1].Status=0;
						}
					}
				}
				nProgramProcess.ValveON = 0;
			}
			else
			{
				if(RecValveOnOff.ValveNo == SendValveOnOff.ValveNo)
				{
					for(int i=0;i<NoOfObject;i++)
					{
						if((nConfig[nProgram.nZone[ProgramNo-1].ValveNo[i]-1].Output_No & SendValveOnOff.ValveNo))	
						{
							nConfig[nProgram.nZone[ProgramNo-1].ValveNo[i]-1].Status=0;
						}
					}
				}
				else if(RecValveOnOff.ValveNo != SendValveOnOff.ValveNo)
				{
					for(int i=0;i<NoOfObject;i++)
					{
						if((nConfig[nProgram.nZone[ProgramNo-1].ValveNo[i]-1].Output_No & SendValveOnOff.ValveNo))	
						{
							nConfig[nProgram.nZone[ProgramNo-1].ValveNo[i]-1].Status=0;
						}
					}
				}
				nProgramProcess.ValveON = 1;
			}
			#endif
			sprintf(Buff,"\nNoOfObject: %d",NoOfObject);
			sAPI_UartPrintf(Buff);
			for(int i=0;i<NoOfObject;i++)
			{
				if(nConfig[i].Object==13)
				{
					unsigned char Temp=0;
					if(RecValveOnOff.ValveStausFlag==2)
					{
						
						sprintf(Buff,"nConfig[%d] %d %d %d\n",i,nConfig[i].Output_No-1,RecValveOnOff.ValveNo,(1<<(nConfig[i].Output_No-1 )) & RecValveOnOff.ValveNo);
						sAPI_UartPrintf(Buff);\
						if(RecValveOnOff.ValveNo == SendValveOnOff.ValveNo)
						{	
							Temp=(1<<(nConfig[i].Output_No - 1 ));
							if((Temp & RecValveOnOff.ValveNo))	
							nConfig[i].Status=1;
							else
							nConfig[i].Status=0;
						}
						else if(RecValveOnOff.ValveNo != SendValveOnOff.ValveNo)
						{
							Temp=(1<<(nConfig[i].Output_No-1 ));
							if((Temp & RecValveOnOff.ValveNo))	
							nConfig[i].Status=1;
							else
							nConfig[i].Status=0;
						}
						//nProgramProcess.ValveON = 0;
						sprintf(Buff,"\nRec:%d %d",RecValveOnOff.ValveStausFlag,RecValveOnOff.ValveNo);
						sAPI_UartPrintf(Buff);
						sprintf(Buff,"--Send:%d %d\n",SendValveOnOff.ValveStausFlag,SendValveOnOff.ValveNo);
						sAPI_UartPrintf(Buff);
						sprintf(Buff,"nConfig[%d].Status:%d\n",i,nConfig[i].Status);
						sAPI_UartPrintf(Buff);

					}
					else
					{
						sprintf(Buff,"nConfig[%d] %d %d %d\n",i,nConfig[i].Output_No,RecValveOnOff.ValveNo,nConfig[i].Output_No & RecValveOnOff.ValveNo);
						sAPI_UartPrintf(Buff);
						if(RecValveOnOff.ValveNo == SendValveOnOff.ValveNo)
						{	
							Temp=(1<<(nConfig[i].Output_No-1 ));
							if((Temp & RecValveOnOff.ValveNo))	
							nConfig[i].Status=0;
							else
							nConfig[i].Status=1;
						}
						else if(RecValveOnOff.ValveNo != SendValveOnOff.ValveNo)
						{
							Temp=(1<<(nConfig[i].Output_No-1 ));
							if((Temp & RecValveOnOff.ValveNo))	
							nConfig[i].Status=1;
							else
							nConfig[i].Status=0;
						}
						
						//nProgramProcess.ValveON = 1;
						sprintf(Buff,"\nRec:%d %d",RecValveOnOff.ValveStausFlag,RecValveOnOff.ValveNo);
						sAPI_UartPrintf(Buff);
						sprintf(Buff,"--Send:%d %d\n",SendValveOnOff.ValveStausFlag,SendValveOnOff.ValveNo);
						sAPI_UartPrintf(Buff);
					}
				}
				if(nConfig[i].Object==45)
				{
					nConfig[i].Status=RecValveOnOff.MainValveStatus;
				}

				if(nConfig[i].Object==26)
				{
					RecValveOnOff.ADC1;
					RecValveOnOff.ADC2;
					RecValveOnOff.ADC3;
					
				}

			}
			
//Praveen

			if(nProgramProcess.ProgramEnable != 0 && nProgramProcess.Status == 0)
			{
				if(nProgramProcess.Sno==nProgram.Sno)
				{
					nProgramProcess.Status=1;
					nProgramProcess.ZoneNo=0;
					nProgramProcess.NoofZones=nProgram.NoofZones;
					nProgramProcess.ZoneFlag=1;
				}
			}
			if(nProgramProcess.Status != 0 && nProgramProcess.ProgramEnable != 0 && nProgramProcess.ZoneFlag == 1)
			{
				
				if(nProgram.nZone[nProgramProcess.ZoneNo].IrrigationMethod == FlowBased)
				{
						
				}
				else if(nProgram.nZone[nProgramProcess.ZoneNo].IrrigationMethod == TimeBased)
				{
					if(nProgram.nZone[nProgramProcess.ZoneNo].Duration[0]==0 && 
					nProgram.nZone[nProgramProcess.ZoneNo].Duration[1]==0 && 
					nProgram.nZone[nProgramProcess.ZoneNo].Duration[2]==0)
					{
						 nProgramProcess.Status=nProgram.Status=0;
						 nProgramProcess.ProgramEnable=0;
						 nProgramProcess.ZoneFlag=0;
						 nProgramProcess.ZoneNo++;
						 nProgramProcess.ZoneOnFlag=0;
					}
					else
					{
						if(nProgramProcess.RemaningSec>5)
						{
							nProgramProcess.StopSec =nProgramProcess.RemaningSec;
						}
						else
						{
							nProgramProcess.StopSec = nProgram.nZone[nProgramProcess.ZoneNo].Duration[0]*3600+
							nProgram.nZone[nProgramProcess.ZoneNo].Duration[1]*60+
							nProgram.nZone[nProgramProcess.ZoneNo].Duration[2];
						}
						
						if(nProgramProcess.StopSec>59)
							nProgramProcess.ZoneOnFlag=1;
						else
						{
							nProgramProcess.Status=nProgram.Status=0;
							//nProgramProcess.ProgramEnable=0;
							nProgramProcess.ZoneFlag=0;
							nProgramProcess.ZoneNo++;
							nProgramProcess.ZoneOnFlag=0;
						}
						 nProgramProcess.StopSec += ((datetime.tm_hour*3600)+(datetime.tm_min*60)+datetime.tm_sec);
						// nProgramProcess.StartSec=(datetime.tm_hour*3600)+(datetime.tm_min*60)+datetime.tm_sec;  
						// nProgramProcess.ZoneFlag=0;
					}
					
				 }

				if(nProgramProcess.ZoneOnFlag==1)
				{
					
					for(int i=0;i<4;i++)
					{
						if(nProgram.nZone[nProgramProcess.ZoneNo].ValveNo[i]>0)
						{
							ZoneOnFlag=nProgramProcess.ValveON=1;
							DelayTime=16;
							ValvenOnRef=2;
							ValvenOnNo=0;
							for(int i=0;i<8;i++)
							{
								for(int j=0;j<NoOfObject;j++)
								{
									if((nProgram.nZone[nProgramProcess.ZoneNo].ValveNo[i]==nConfig[j].Sno) && nConfig[j].Object==13)
									ValvenOnNo|=1<<nConfig[j].Output_No;
								}
							}
							SendValveOnOff.ValveNo=ValvenOnNo;
							SendValveOnOff.ValveStausFlag=ValvenOnRef;
							break;
						}
						else
						{
						 	nProgramProcess.ValveON=0;
						}
						
						
					}
					if(nProgram.nZone[nProgramProcess.ZoneNo].MainValve==1)
					{
						for(int i=0;i<NoOfObject;i++)
						{
							if(nConfig[i].Object==45)
							{
								MainValveNo=nConfig[i].Output_No;
								nProgramProcess.MainValveOn=1;
								if(nProgram.ControlSeclection==FlowBased)
								{
									nProgramProcess.ControlSeclection=FlowBased;
									nProgramProcess.SetValue=nProgram.SetFlowRate;
									nProgramProcess.SetTolerence=nProgram.SetTolerence;
								}
								else if(nProgram.ControlSeclection==PressureBased)
								{
									nProgramProcess.ControlSeclection=PressureBased;
									nProgramProcess.SetValue=nProgram.SetPressure;
									nProgramProcess.SetTolerence=nProgram.SetTolerence;
								}
								break;
							}
							else
							{
								nProgramProcess.MainValveOn=0;
							}
						}
					}
					nProgramProcess.ZoneFlag = 2;	
				}
			}

			if(nProgramProcess.Status != 0 && nProgramProcess.ProgramEnable != 0 && nProgramProcess.ZoneFlag == 2)
			{
				if(nProgramProcess.ValveON == 0 && nProgramProcess.MainValveOn == 0)
				{
					if(nProgram.nZone[nProgramProcess.ZoneNo].IrrigationMethod == FlowBased)
					{
						//if(nFlow.FlowCount>=nFlow.FlowTarget)
						{
							nProgramProcess.Status=nProgram.Status=0;
							nProgramProcess.ZoneFlag=1;
							nProgramProcess.ZoneNo++;
							if(nProgramProcess.ZoneNo>=nProgram.NoofZones)
							{
								nProgramProcess.ProgramEnable=0;
							}
						}
					}
					else if(nProgram.nZone[nProgramProcess.ZoneNo].IrrigationMethod == TimeBased)
					{
						if(nProgramProcess.StopSec>0)
						{
							 nProgramProcess.StopSec-=((datetime.tm_hour*3600)+(datetime.tm_min*60)+datetime.tm_sec);
						}
						else
						{
							nProgramProcess.Status=nProgram.Status=0;
							nProgramProcess.ZoneFlag=1;
							nProgramProcess.ZoneNo++;
							if(nProgramProcess.ZoneNo>=nProgram.NoofZones)
							{
								nProgramProcess.ProgramEnable=0;
							}
						}
					}
				}
			}
			if(nProgramProcess.Status != 0 && nProgramProcess.ProgramEnable == 0)
			{
				if(nProgramProcess.ZoneOnFlag==1)
				{
					if(nProgramProcess.MainValveOn==0)
					{
						for(int i=0;i<4;i++)
						{
							if(nProgram.nZone[nProgramProcess.ZoneNo].ValveNo[i]>0)
							{
								ZoneOnFlag=nProgramProcess.ValveON=1;
								DelayTime=16;
								ValvenOnRef=1;
								ValvenOnNo=255;
								SendValveOnOff.ValveNo=ValvenOnNo;
								SendValveOnOff.ValveStausFlag=ValvenOnRef;
								break;
							}
							else
							{
								nProgramProcess.ValveON=0;
							}
							
							
						}
					}
					
					if(nProgram.nZone[nProgramProcess.ZoneNo].MainValve==1)
					{
						for(int i=0;i<NoOfObject;i++)
						{
							if(nConfig[i].Object==45)
							{
								MainValveNo=0;
								nProgramProcess.MainValveOn=1;
								if(nProgram.ControlSeclection==FlowBased)
								{
									nProgramProcess.ControlSeclection=0;
									nProgramProcess.SetValue=nProgram.SetFlowRate=0.0;
									nProgramProcess.SetTolerence=nProgram.SetTolerence=0.0;
								}
								else if(nProgram.ControlSeclection==PressureBased)
								{
									nProgramProcess.ControlSeclection=0;
									nProgramProcess.SetValue=nProgram.SetPressure=0.0;
									nProgramProcess.SetTolerence=nProgram.SetTolerence=0.0;
								}
								break;
							}
							else
							{
								nProgramProcess.MainValveOn=0;
							}
						}
					}
						
				}
			}

			if(nProgramProcess.ValveON != 0 && DelayTime>15)
			{
				int checksum=0,CalculatedCrc=0;
				memset(Buffer1,0x00,sizeof(Buffer1));
				sprintf(Buffer1,"$:9:6:1:%d:%d:1:",SendValveOnOff.ValveStausFlag,SendValveOnOff.ValveNo);
				for(int i=0;i<=strlen(Buffer1)-2;i++)
				{
					checksum+=(char *)Buffer1[i];
				}	
				CalculatedCrc=checksum%256;
				if(CalculatedCrc==0)
				CalculatedCrc=1;
				sprintf(Buffer1,"%s%03d:\r\n",CalculatedCrc);
				sAPI_UartWrite(SC_UART, Buffer1, strlen(Buffer1));
				sAPI_UartPrintf(Buffer1);
				Timer=0;
			}

			else if((nProgramProcess.MainValveOn != 0  && nProgramProcess.ValveON == 0 )&& DelayTime>15)
			{
				int checksum=0,CalculatedCrc=0;
				memset(Buffer1,0x00,sizeof(Buffer1));
				sprintf(Buffer1,"$:10:10:1:%d:%.01f:%.01f:1:",MainValveNo,nProgramProcess.SetValue,nProgramProcess.SetTolerence);
				for(int i=0;i<=strlen(Buffer1)-2;i++)
				{
					checksum+=(char *)Buffer1[i];
				}	
				CalculatedCrc=checksum%256;
				if(CalculatedCrc==0)
				CalculatedCrc=1;
				sprintf(Buffer1,"%s%03d:\r\n",CalculatedCrc);
				sAPI_UartWrite(SC_UART, Buffer1, strlen(Buffer1));
				sAPI_UartPrintf(Buffer1);
				Timer=0;
			}

			else if(nProgramProcess.ZoneOnFlag != 0 && nProgramProcess.StopSec<=59)
			{
				nProgramProcess.ZoneFlag=1;
				nProgramProcess.ZoneNo++;
				nProgramProcess.ZoneOnFlag=0;
			}

			#if 0
			if(ProgramFlag_1==1 && ProgramStartFlag_1==0)
			{
				if(nProgram.Sno==1 && nProgram.nZone[0].Sno==nProgram.Sno) 
				{
					if(ZoneOnFlag==0)
					{
						ZoneOnFlag=1;
						ValveOnFlag=1;
						Timer=DelayTime;
						ProgramStartFlag_1==1;
						ProgramNo=1;
					}
				}
			}

			if((ZoneOnFlag==ProgramStartFlag_1) && (ValveOnFlag==1) && Timer>DelayTime)
			{
				int checksum=0,CalculatedCrc=0;
				ValvenOnNo=0;
				for(int i=0;i<4;i++)
				{
					if(nConfig[nProgram.nZone[ZoneOnFlag-1].ValveNo[i]].Sno==13)
						ValvenOnNo|=1<<nConfig[nProgram.nZone[ZoneOnFlag-1].ValveNo[i]].Output_No;
				}
				memset(Buffer1,0x00,sizeof(Buffer1));
				sprintf(Buffer1,"$:9:6:1:2:%d:1:",ValvenOnNo);
				for(int i=0;i<=strlen(Buffer1)-2;i++)
				{
					checksum+=(char *)Buffer1[i];
				}	
				CalculatedCrc=checksum%256;
				if(CalculatedCrc==0)
				CalculatedCrc=1;
				sprintf(Buffer1,"%s%03d:\r\n",CalculatedCrc);
				sAPI_UartWrite(SC_UART, Buffer1, strlen(Buffer1));
				sAPI_UartPrintf(Buffer1);
				Timer=0;
			}

			if((ZoneOnFlag==ProgramStartFlag_1) && (ValveOnFlag==0 && MainValveFlag==1) && Timer>DelayTime)
			{
				int checksum=0,CalculatedCrc=0;
				memset(Buffer1,0x00,sizeof(Buffer1));
				for(int i=0;i<NoOfObject;i++)
				{
					if(nConfig[i].Sno==45)
						ValvenOnNo|=nConfig[nProgram.nZone[ZoneOnFlag-1].ValveNo[i]-1].Output_No;
				}
				sprintf(Buffer1,"$:9:10:1:%d:1:",ValvenOnNo);
				for(int i=0;i<=strlen(Buffer1)-2;i++)
				{
					checksum+=(char *)Buffer1[i];
				}	
				CalculatedCrc=checksum%256;
				if(CalculatedCrc==0)
				CalculatedCrc=1;
				sprintf(Buffer1,"%s%03d:\r\n",CalculatedCrc);
				sAPI_UartWrite(SC_UART, Buffer1, strlen(Buffer1));
				sAPI_UartPrintf(Buffer1);
			}
			#endif
			if (datetime.tm_sec != ActpowerOnDelayPrvSec && (PowerCurrentCondition == 0 || uart_send_flag == 1 || Resend_IMEI_flag == 1)) // dg_added_oro
			{
				// checkpower=Check2Phase();
				/* if(PowerCurrentCondition==0)   //dg_nsdk
				nMoTr.ActonpowerRunTimer++; */
				/*  if(checkpower == 2)
				  nMoTr.Act2powerRunTimer++;
				 else
				  nMoTr.Act3powerRunTimer++; */
				ActpowerOnDelayPrvSec = datetime.tm_sec;

				//	if((uart_send_count<=3) && (uart_send_flag==1) && (first_time_data_req_flag==1))
			

				if (date_time_send_flag == 1)
				{
					date_time_send_flag = 0;
					sprintf(textBuf, "$S,D,%02d,%02d,%02d,%02d,%02d,%02d,\n\r", timeval.tm_year, timeval.tm_mon, timeval.tm_mday, timeval.tm_hour, timeval.tm_min, timeval.tm_sec);
					sAPI_UartWrite(SC_UART, textBuf, strlen(textBuf));
					sAPI_UartPrintf(textBuf);
				}
				if ((uart_send_count <= 3) && (uart_send_flag == 1))

				{
					memset(textBuf, NULL, sizeof(textBuf));
					sprintf(textBuf, "$S,C,%d,%d,%d,\n\r", nMSettings.motor1onof, nMSettings.motor2onof, nMSettings.motor3onof);
					sAPI_UartWrite(SC_UART, textBuf, strlen(textBuf));
					sAPI_UartPrintf(textBuf);
					uart_send_count++;
					if (uart_send_count >= 3)
					{
						//	first_time_data_req_flag=0;
						uart_send_flag = 0;
						uart_send_count = 0;
					}
				}
				else if ((uart_send_count <= 3) && (uart_send_flag == 2))

				{
					memset(textBuf, NULL, sizeof(textBuf));
					sprintf(textBuf, "$S,C,0,0,0,\n\r");
					sAPI_UartWrite(SC_UART, textBuf, strlen(textBuf));
					sAPI_UartPrintf(textBuf);
					uart_send_count++;
					if (uart_send_count >= 3)
					{
						//	first_time_data_req_flag=0;
						uart_send_flag = 1;
						uart_send_count = 0;
					}
				}
				if (Resend_IMEI_flag == 1 && Resend_IMEI_count <= 8 && s_nMSettings.m_Enter >= 70 && uart_send_flag == 0 && uart_data_comm_flag == 0)
				{

					memset(textBuf, NULL, sizeof(textBuf));
					uart_data_comm_flag = 1;
					sprintf(textBuf, "$S,X,\n\r");
					sAPI_UartWrite(SC_UART, textBuf, strlen(textBuf));
					sAPI_UartPrintf(textBuf);
					//	Resend_IMEI_count++;
					if (Resend_IMEI_count >= 6)
					{
						Resend_IMEI_count = 0;
						Resend_IMEI_flag = 0;
						sprintf(buf, "\n\r  Resend_IMEI_flag %d Resend_IMEI_count %d line %d \n\r", Resend_IMEI_flag, Resend_IMEI_count, __LINE__);
						sAPI_UartPrintf(buf);
					}
				}
			}

			sprintf(buf, "\n\r uart_send_count %d uart_send_flag %d Resend_IMEI_flag %d Resend_IMEI_count %d \n\r", uart_send_count, uart_send_flag, Resend_IMEI_flag, Resend_IMEI_count);
			sAPI_UartPrintf(buf);

#if 0
							//if(CheckRTC() == 1)
								if((CheckRTC() == 1) && (s_nMSettings.m_Enter >70))
							{
								
								// MotorStarterTripCount = 0;
								OverAllStarterTrip = 0;
								ActMotorStarterTripTimer =0;
							//	if(nMSettings.rtcautorstonof==1)
								MakeRealyOn = 1;  //dg_commented
								StartByMobile = 0;
							//	RunMotor(MakeRealyOn,PowerCurrentCondition);
							 }
							
							else
							{
								
								if(StartByMobile == 1)
								{
									MakeRealyOn = 1;
							//		RunMotor(MakeRealyOn,PowerCurrentCondition);
								}
								else
								{
									//OffMotorByPhone = 0;
								//	if(nMSettings.rtcautorstonof==1)
									MakeRealyOn = 0;  //dg_commented
							//		RunMotor(MakeRealyOn,PowerCurrentCondition);
								}
							}
										
							

							if(nMSettings.TargetOnOff == 1 && TargetNumberFound == 1 && OnByTarget == 1)
							{
								TargetTimerCnt++;
								sprintf(buf,"\n\r*******Target Counter = %ld********\n\r",TargetTimerCnt);
								sAPI_UartPrintf(buf);
								if(TargetTimerCnt>=500)								//1000
								{
									OnByTarget = 0;
									MakeRealyOn = 0;
									nCYCTimer = NO_TCOND;
									StartByMobile = 0;
									OffMotorByPhone = 0;
									nSTATE_STATUS_SMS =STATUS_MOTOR_OFF_TARGET;
									SendSmsToAll = 1;
									// nSTATE_MOTOR_ON_SMS=STATE_MOTOR_ON_DEFAULT;
									nSTATE_STATUS_SMS = STATE_MOTOR_OFF_TARGET;
								}
							}
							else
								TargetTimerCnt = 0;
							//	RunMotor(MakeRealyOn,PowerCurrentCondition);

								//if(nTimerSettings.AutoRstOn == 0 && PowerCurrentCondition == 1)
								//	MakeRealyOn = 0;

							if(nTimerSettings.Drrestartpoweronof==1 && MakeRealyOn == 1 )
							{
								sAPI_GetRealTimeClock(&datetime);
								if(nTimerSettings.DrReOnOf == 1)
								{
									if(datetime.tm_sec != ActStarDetaPrvSec)
									{
										nMoTr.ActDrRunRestart++;
										ActStarDetaPrvSec = datetime.tm_sec;
									}
									sprintf(buf,"\n\rDebug2:nMoTr.ActDrRunRestart = %d  nMoTr.DrRunRestart = %d\n\r",nMoTr.ActDrRunRestart,nMoTr.DrRunRestart);
									sAPI_UartPrintf(buf);
									RemainingDrTime = nMoTr.DrRunRestart-nMoTr.ActDrRunRestart;     //aj added
									if(nMoTr.ActDrRunRestart>=nMoTr.DrRunRestart)
									{
										if(PowerCurrentCondition == 0)
											nSTATE_MOTOR = STATE_MOTOR_TRIP_RESTART;
										else
											nSTATE_STATUS_SMS =STATUS_NO_ELECTRICICY;
										sprintf(buf,"\n\rReStart Time Completed\n\r");
										sAPI_UartPrintf(buf);
										nTimerSettings.Drrestartpoweronof=0;
										SendSmsToAll = 1;
									}
									else
									{
										if(PowerCurrentCondition == 0)
											nSTATE_STATUS_SMS =STATUS_MOTOR_DRYRUN_TRIP_SMS;
										else
											// sAPI_UartPrintf("STATUS_NO_ELECTRICICY;");                //aj
											nSTATE_STATUS_SMS =STATUS_NO_ELECTRICICY;
											//nSTATE_MOTOR_SMS = STATE_MOTOR_DRYRUN_TRIP_SMS;
											nSTATE_MOTOR = STATE_MOTOR_TRIP_DRYRUN;
									}
								}
							}
#endif

			if(g_new_MAC_flag==1)
			{
				g_new_MAC_flag=0;
			//	WritectsetFile(); //dg_nsdk
				WriteEpromidFile();
				MQTT_Reconnect();
			}
			sprintf(buf, "\n\rDebug2:s_nMSettings.m_settings_req_flag = %d  s_nMSettings.m_pumpno_send = %d g_no_of_pumps =%d s_nMSettings.m_settings_count %d\n\r", s_nMSettings.m_settings_req_flag, s_nMSettings.m_pumpno_send, g_no_of_pumps, s_nMSettings.m_settings_count);
			//	g_no_of_pumps=2;
			// s_nMSettings.m_settings_req_flag=1;
			sAPI_UartPrintf(buf);
			if (s_nMSettings.m_pumpno_send == 0)
				s_nMSettings.m_pumpno_send = 1;
			if (s_nMSettings.m_settings_req_flag == 1 && s_nMSettings.m_pumpno_send <= g_no_of_pumps) // subash2
			{
				// char temp_count=0;
				sprintf(buf, "\n temp_count : %d \n", temp_count);
				sAPI_UartPrintf(buf);
				if (temp_count++ > 3) // 3//5
				{
					temp_count = 0;
					if (s_nMSettings.m_settings_count == 1)
					{
						s_nMSettings.m_uart_send_flag = 1;
						//	sprintf(Sim_Buf,"$O,S,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\n\r",
						sprintf(Sim_Buf, "$O,S,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,\n\r",
								s_npump[0].m_Tank_on_off, s_npump[0].m_Sump_on_off, s_npump[1].m_Tank_on_off, s_npump[1].m_Sump_on_off, s_npump[2].m_Tank_on_off, s_npump[2].m_Sump_on_off, s_npump[0].m_no_of_sump_pins, s_npump[0].m_sump_pin_no[0], s_npump[0].m_sump_pin_no[1],
								s_npump[0].m_no_of_tank_pins, s_npump[0].m_tank_pin_no[0], s_npump[0].m_tank_pin_no[1], s_npump[0].m_Level_on_off, s_npump[0].m_flowonof, s_npump[0].m_pressureonof, s_npump[0].m_Uppertank_restart_off, s_npump[0].m_Lowertank_restart_off,
								s_npump[1].m_Uppertank_restart_off, s_npump[1].m_Lowertank_restart_off, s_npump[2].m_Uppertank_restart_off, s_npump[2].m_Lowertank_restart_off, Prev_M1_state, prev_onof_M1, tripflag[0]);

						/* s_npump[0].m_Sump_on_off,s_npump[0].m_no_of_sump_pins,s_npump[0].m_sump_pin_no[0],s_npump[0].m_sump_pin_no[1],s_npump[0].m_Tank_on_off,s_npump[0].m_no_of_tank_pins,s_npump[0].m_tank_pin_no[0],s_npump[0].m_tank_pin_no[1],
						s_npump[1].m_Sump_on_off,s_npump[1].m_no_of_sump_pins,s_npump[1].m_sump_pin_no[0],s_npump[1].m_sump_pin_no[1],s_npump[1].m_Tank_on_off,s_npump[1].m_no_of_tank_pins,s_npump[1].m_tank_pin_no[0],s_npump[1].m_tank_pin_no[1],
						g_no_of_pumps,Prev_M1_state,prev_onof_M1,tripflag[0]); */

						/************subash commented*************/
						/* sprintf(Sim_Buf,"$O,S,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n\r",
						 g_no_of_pumps,pumpno,s_npump[0].m_Sump_on_off,
						 s_npump[0].m_no_of_sump_pins,s_npump[0].m_sump_pin_no[0],s_npump[0].m_sump_pin_no[1],s_npump[0].m_Tank_on_off,s_npump[0].m_no_of_tank_pins,s_npump[0].m_tank_pin_no[0],
						 s_npump[0].m_tank_pin_no[1],pumpno,s_npump[1].m_Sump_on_off,s_npump[1].m_no_of_sump_pins,s_npump[1].m_sump_pin_no[0],s_npump[1].m_sump_pin_no[1],s_npump[1].m_Tank_on_off,
						 s_npump[1].m_no_of_tank_pins,s_npump[1].m_tank_pin_no[0],s_npump[1].m_tank_pin_no[1]); */
						sAPI_UartWrite(SC_UART, (UINT8 *)Sim_Buf, strlen(Sim_Buf));
						sprintf(buf, "%s", Sim_Buf);
						sAPI_UartPrintf(buf);
					}
					else if (s_nMSettings.m_settings_count == 2)
					{
						s_nMSettings.m_uart_send_flag = 1;
						sprintf(Sim_Buf, "$P,S,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%02d,%01d,%01d,%01d,",
								s_npump[1].m_no_of_sump_pins, s_npump[1].m_sump_pin_no[0], s_npump[1].m_sump_pin_no[1], s_npump[1].m_no_of_tank_pins, s_npump[1].m_tank_pin_no[0], s_npump[1].m_tank_pin_no[1], s_npump[1].m_Level_on_off, s_npump[1].m_flowonof, s_npump[1].m_pressureonof,
								s_npump[2].m_no_of_sump_pins, s_npump[2].m_sump_pin_no[0], s_npump[2].m_sump_pin_no[1], s_npump[2].m_no_of_tank_pins, s_npump[2].m_tank_pin_no[0], s_npump[2].m_tank_pin_no[1], s_npump[2].m_Level_on_off, s_npump[2].m_flowonof, s_npump[2].m_pressureonof,
								g_no_of_pumps, g_serialid, a_Master_onoff[0], a_Master_onoff[1], a_Master_onoff[2]);
						/* s_npump[2].m_Sump_on_off,s_npump[2].m_no_of_sump_pins,s_npump[2].m_sump_pin_no[0],s_npump[2].m_sump_pin_no[1],
						s_npump[2].m_Tank_on_off,s_npump[2].m_no_of_tank_pins,s_npump[2].m_tank_pin_no[0],s_npump[2].m_tank_pin_no[1],
						s_npump[0].m_Level_on_off,s_npump[0].m_pressureonof,s_npump[0].m_flowonof,s_npump[1].m_Level_on_off,s_npump[1].m_pressureonof,s_npump[1].m_flowonof,
						s_npump[2].m_Level_on_off,s_npump[2].m_pressureonof,s_npump[2].m_flowonof); */
						sAPI_UartWrite(SC_UART, (UINT8 *)Sim_Buf, strlen(Sim_Buf));
						sprintf(buf, "%s", Sim_Buf);
						sAPI_UartPrintf(buf);
					}
					else if (s_nMSettings.m_settings_count == 3)
					{
						s_nMSettings.m_uart_send_flag = 1;
						sprintf(Sim_Buf, "$Q,S,%d,%03d,%03d,%03d,%03d,%d,%03d,%03d,%03d,%03d,%d,%02d,%01d,%01d,\n\r",
								s_nMSettings.m_LowVoltOnOff, s_nTimerSettings.m_LowVoltIII, s_nTimerSettings.m_DiffVoltIII, s_nTimerSettings.m_LowVoltII, s_nTimerSettings.m_DiffVoltII,
								s_nMSettings.m_HighVoltOnOff, s_nTimerSettings.m_HighVoltIII, s_nTimerSettings.m_HiDiffVoltIII, s_nTimerSettings.m_HighVoltII, s_nTimerSettings.m_HiDiffVoltII,
								s_nTimerSettings.m_ImbVolt, s_nMSettings.m_RvePhOnoff, s_nMSettings.m_SppOnoff, s_nMSettings.m_CurSppOnOff);
						sAPI_UartWrite(SC_UART, (UINT8 *)Sim_Buf, strlen(Sim_Buf));
						sprintf(buf, "%s", Sim_Buf);
						sAPI_UartPrintf(buf);

						// s_nMSettings.m_LowVoltOnOff,s_nTimerSettings.m_LowVoltII,s_nTimerSettings.m_DiffVoltII,s_nTimerSettings.m_LowVoltIII,s_nTimerSettings.m_DiffVoltIII,
						// s_nMSettings.m_HighVoltOnOff,s_nTimerSettings.m_HighVoltII,s_nTimerSettings.m_HiDiffVoltII,s_nTimerSettings.m_HighVoltIII,s_nTimerSettings.m_HiDiffVoltIII,
						// s_nMSettings.m_SppOnoff,(int)s_nTimerSettings.m_ImbVolt,s_nMSettings.m_RvePhOnoff,s_nMSettings.m_CurSppOnOff
					}
					else if (s_nMSettings.m_settings_count == 4)
					{
						s_nMSettings.m_uart_send_flag = 1;
						sprintf(Sim_Buf, "$W,S,%01d,%01d,%01d,%01d,%0.2f,%03d,%03d,%01d,%01d,%02d,%02d,%02d,%02d,%01d,%02d,%02d,%02d,%02d,%02d,%02d,%01d,%01d,%02d\n\r",
								s_nMSettings.m_AutoStIIOnOff[0], s_nMSettings.m_CTRonoff, s_nMSettings.m_CTYonoff, s_nMSettings.m_CTBonoff, s_ntank.m_tank_height, s_ntank.m_min_level_p, s_ntank.m_max_level_p, s_nTimerSettings.m_AutoRst2On, s_nMSettings.m_NlightOnOf, s_nMSettings.m_NlightRTCOnHr, s_nMSettings.m_NlightRTCOnMin,
								s_nMSettings.m_NlightRTCOfHr, s_nMSettings.m_NlightRTCOfMin, s_nMSettings.m_peakHourOnOf, s_nMSettings.m_peakOnHr, s_nMSettings.m_peakOnMin, s_nMSettings.m_peakOnSec, s_nMSettings.m_peakOfHr, s_nMSettings.m_peakOfMin, s_nMSettings.m_peakOfSec, s_ntank.m_Uppertank, s_ntank.m_Lowertank, s_ntank.m_Level_Sensor_height);
						//	s_nMSettings.m_AutoStIIOnOff[0],s_nMSettings.m_CTRonoff,s_nMSettings.m_CTYonoff,s_nMSettings.m_CTBonoff,s_ntank.m_tank_height,s_ntank.m_min_level_p,s_ntank.m_max_level_p,s_nTimerSettings.m_AutoRst2On);

						sAPI_UartWrite(SC_UART, (UINT8 *)Sim_Buf, strlen(Sim_Buf));
						sprintf(buf, "%s", Sim_Buf);
						sAPI_UartPrintf(buf);
					}

					//	else if(s_nMSettings.m_settings_count==5)
					else if (s_nMSettings.m_settings_count == 5 )
					{
						s_nMSettings.m_uart_send_flag = 1;
						unsigned char l_pumpno_tx;
						l_pumpno_tx = s_nMSettings.m_pumpno_send - 1;
						sprintf(Sim_Buf, "$R,S,%01d,%01d,%02d,%02d,%02d,%0.2f,%0.2f,%01d,%02d,%02d,%02d,%01d,%02d,%02d,%02d,%02d,%01d,\n\r",
								l_pumpno_tx, s_nMSettings.m_DrScOnOf[l_pumpno_tx], s_nTimerSettings.m_DrScHr[l_pumpno_tx], s_nTimerSettings.m_DrScMin[l_pumpno_tx], s_nTimerSettings.m_DrScSec[l_pumpno_tx],
								s_nTimerSettings.m_DrAmpsIII[l_pumpno_tx], s_nTimerSettings.m_DrAmpsII[l_pumpno_tx], s_nMSettings.m_DrReOnOf[l_pumpno_tx], s_nTimerSettings.m_DrReHr[l_pumpno_tx],
								s_nTimerSettings.m_DrReMin[l_pumpno_tx], s_nTimerSettings.m_DrReSec[l_pumpno_tx], s_nMSettings.m_DrOccurOnOff[l_pumpno_tx], s_nTimerSettings.m_DrOccurTimHr[l_pumpno_tx],
								s_nTimerSettings.m_DrOccurTimMin[l_pumpno_tx], s_nTimerSettings.m_DrOccurTimSec[l_pumpno_tx], a_occurance_count[l_pumpno_tx], s_nMSettings.m_AutoDrRunRstIIOnOff[l_pumpno_tx]);
						sAPI_UartWrite(SC_UART, (UINT8 *)Sim_Buf, strlen(Sim_Buf));
						sprintf(buf, "%s", Sim_Buf);
						sAPI_UartPrintf(buf);
					}
					//	else if(s_nMSettings.m_settings_count==6)
					else if (s_nMSettings.m_settings_count == 6 )
					{
						s_nMSettings.m_uart_send_flag = 1;
						unsigned char l_pumpno_tx;
						l_pumpno_tx = s_nMSettings.m_pumpno_send - 1;
						sprintf(Sim_Buf, "$S,S,%d,%02d,%02d,%02d,%02d,%02d,%02d,%01d,%02d,%02d,%02d,%d,%02d,%02d,%02d,%d,%02d,%02d,%02d,\n\r",
								l_pumpno_tx, s_nTimerSettings.m_POnHr[l_pumpno_tx], s_nTimerSettings.m_POnMin[l_pumpno_tx], s_nTimerSettings.m_POnSec[l_pumpno_tx], s_nTimerSettings.m_SDHr[l_pumpno_tx], s_nTimerSettings.m_SDMin[l_pumpno_tx],
								s_nTimerSettings.m_SDSec[l_pumpno_tx], s_nMSettings.m_PoScrDlOnOff[l_pumpno_tx], s_nTimerSettings.m_PoScrDlHr[l_pumpno_tx], s_nTimerSettings.m_PoScrDlMin[l_pumpno_tx], s_nTimerSettings.m_PoScrDlSec[l_pumpno_tx],
								s_nMSettings.m_SfbOnOff[l_pumpno_tx], s_nTimerSettings.m_SfbHr[l_pumpno_tx], s_nTimerSettings.m_SfbMin[l_pumpno_tx], s_nTimerSettings.m_SfbSec[l_pumpno_tx], s_nMSettings.m_MaxRnOnOf[l_pumpno_tx],
								s_nTimerSettings.m_MaxRnHr[l_pumpno_tx], s_nTimerSettings.m_MaxRnMin[l_pumpno_tx], s_nTimerSettings.m_MaxRnSec[l_pumpno_tx]);
						/*  l_pumpno_tx,s_nTimerSettings.m_POnHr[l_pumpno_tx],s_nTimerSettings.m_POnMin[l_pumpno_tx],s_nTimerSettings.m_POnSec[l_pumpno_tx],s_nMSettings.m_PoScrDlOnOff[l_pumpno_tx],s_nTimerSettings.m_PoScrDlHr[l_pumpno_tx],
s_nTimerSettings.m_PoScrDlMin[l_pumpno_tx],s_nTimerSettings.m_PoScrDlSec[l_pumpno_tx],s_nTimerSettings.m_SDHr[l_pumpno_tx],s_nTimerSettings.m_SDMin[l_pumpno_tx],s_nTimerSettings.m_SDSec[l_pumpno_tx],
s_nMSettings.m_SfbOnOff[l_pumpno_tx],s_nTimerSettings.m_SfbHr[l_pumpno_tx],s_nTimerSettings.m_SfbMin[l_pumpno_tx],s_nTimerSettings.m_SfbSec[l_pumpno_tx],s_nMSettings.m_CycLicOnOf[l_pumpno_tx],
s_nTimerSettings.m_CycLicOnHr[l_pumpno_tx],s_nTimerSettings.m_CycLicOnMin[l_pumpno_tx],s_nTimerSettings.m_CycLicOnSec[l_pumpno_tx],s_nTimerSettings.m_CycLicOfHr[l_pumpno_tx],s_nTimerSettings.m_CycLicOfMin[l_pumpno_tx],
s_nTimerSettings.m_CycLicOfSec[l_pumpno_tx] */

						// ,s_nMSettings.m_MaxRnOnOf[l_pumpno_tx],s_nTimerSettings.m_MaxRnHr[l_pumpno_tx],s_nTimerSettings.m_MaxRnMin[l_pumpno_tx],s_nTimerSettings.m_MaxRnSec[l_pumpno_tx]);
						sAPI_UartWrite(SC_UART, (UINT8 *)Sim_Buf, strlen(Sim_Buf));
						sprintf(buf, "%s", Sim_Buf);
						sAPI_UartPrintf(buf);
					}
					//	else if(s_nMSettings.m_settings_count==7)
					else if (s_nMSettings.m_settings_count == 7 )
					{
						s_nMSettings.m_uart_send_flag = 1;
						unsigned char l_pumpno_tx;
						l_pumpno_tx = s_nMSettings.m_pumpno_send - 1;
						sprintf(Sim_Buf, "$T,S,%d,%d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,\n\r",
								l_pumpno_tx, s_nMSettings.m_RTCOnOf[l_pumpno_tx], s_nTimerSettings.m_RTCONHr[l_pumpno_tx][1], s_nTimerSettings.m_RTCONMin[l_pumpno_tx][1], s_nTimerSettings.m_RTCONSec[l_pumpno_tx][1],
								s_nTimerSettings.m_RTCOfHr[l_pumpno_tx][1], s_nTimerSettings.m_RTCOfMin[l_pumpno_tx][1], s_nTimerSettings.m_RTCOfSec[l_pumpno_tx][1], s_nTimerSettings.m_RTCONHr[l_pumpno_tx][2],
								s_nTimerSettings.m_RTCONMin[l_pumpno_tx][2], s_nTimerSettings.m_RTCONSec[l_pumpno_tx][2], s_nTimerSettings.m_RTCOfHr[l_pumpno_tx][2], s_nTimerSettings.m_RTCOfMin[l_pumpno_tx][2],
								s_nTimerSettings.m_RTCOfSec[l_pumpno_tx][2], s_nTimerSettings.m_RTCONHr[l_pumpno_tx][3], s_nTimerSettings.m_RTCONMin[l_pumpno_tx][3], s_nTimerSettings.m_RTCONSec[l_pumpno_tx][3],
								s_nTimerSettings.m_RTCOfHr[l_pumpno_tx][3], s_nTimerSettings.m_RTCOfMin[l_pumpno_tx][3], s_nTimerSettings.m_RTCOfSec[l_pumpno_tx][3]);
						sAPI_UartWrite(SC_UART, (UINT8 *)Sim_Buf, strlen(Sim_Buf));
						sprintf(buf, "%s", Sim_Buf);
						sAPI_UartPrintf(buf);
					}
					//	else if(s_nMSettings.m_settings_count==8)
					else if (s_nMSettings.m_settings_count == 8 )
					{
						s_nMSettings.m_uart_send_flag = 1;
						unsigned char l_pumpno_tx;
						l_pumpno_tx = s_nMSettings.m_pumpno_send - 1;
						//	sprintf(Sim_Buf,"$T,S,%d,%d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,\n\r",
						sprintf(Sim_Buf, "$U,S,%d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,\n\r",
								l_pumpno_tx, s_nTimerSettings.m_RTCONHr[l_pumpno_tx][4], s_nTimerSettings.m_RTCONMin[l_pumpno_tx][4], s_nTimerSettings.m_RTCONSec[l_pumpno_tx][4],
								s_nTimerSettings.m_RTCOfHr[l_pumpno_tx][4], s_nTimerSettings.m_RTCOfMin[l_pumpno_tx][4], s_nTimerSettings.m_RTCOfSec[l_pumpno_tx][4], s_nTimerSettings.m_RTCONHr[l_pumpno_tx][5],
								s_nTimerSettings.m_RTCONMin[l_pumpno_tx][5], s_nTimerSettings.m_RTCONSec[l_pumpno_tx][5], s_nTimerSettings.m_RTCOfHr[l_pumpno_tx][5], s_nTimerSettings.m_RTCOfMin[l_pumpno_tx][5],
								s_nTimerSettings.m_RTCOfSec[l_pumpno_tx][5], s_nTimerSettings.m_RTCONHr[l_pumpno_tx][6], s_nTimerSettings.m_RTCONMin[l_pumpno_tx][6], s_nTimerSettings.m_RTCONSec[l_pumpno_tx][6],
								s_nTimerSettings.m_RTCOfHr[l_pumpno_tx][6], s_nTimerSettings.m_RTCOfMin[l_pumpno_tx][6], s_nTimerSettings.m_RTCOfSec[l_pumpno_tx][6]);
						sAPI_UartWrite(SC_UART, (UINT8 *)Sim_Buf, strlen(Sim_Buf));
						sprintf(buf, "%s", Sim_Buf);
						sAPI_UartPrintf(buf);
					}
					//	else if(s_nMSettings.m_settings_count==9)
					else if (s_nMSettings.m_settings_count == 9 )
					{
						s_nMSettings.m_uart_send_flag = 1;
						unsigned char l_pumpno_tx;
						l_pumpno_tx = s_nMSettings.m_pumpno_send - 1;
						sprintf(Sim_Buf, "$V,S,%01d,%01d,%02d,%02d,%02d,%f,%f,%01d,%01d,%01d,%02d,%02d,%02d,%02d,%02d,%02d,\n\r",
								l_pumpno_tx, s_nMSettings.m_OlOnOff[l_pumpno_tx], s_nTimerSettings.m_OlScanHr[l_pumpno_tx], s_nTimerSettings.m_OlScanMin[l_pumpno_tx], s_nTimerSettings.m_OlScanSec[l_pumpno_tx],
								s_nTimerSettings.m_OlAmpsIII[l_pumpno_tx], s_nTimerSettings.m_OlAmpsII[l_pumpno_tx], s_nMSettings.m_AutoOlDrRstIIOnOff[l_pumpno_tx], s_nTimerSettings.m_AutoRstOn[l_pumpno_tx],
								s_nMSettings.m_CycLicOnOf[l_pumpno_tx], s_nTimerSettings.m_CycLicOnHr[l_pumpno_tx], s_nTimerSettings.m_CycLicOnMin[l_pumpno_tx], s_nTimerSettings.m_CycLicOnSec[l_pumpno_tx],
								s_nTimerSettings.m_CycLicOfHr[l_pumpno_tx], s_nTimerSettings.m_CycLicOfMin[l_pumpno_tx], s_nTimerSettings.m_CycLicOfSec[l_pumpno_tx]);
						sAPI_UartWrite(SC_UART, (UINT8 *)Sim_Buf, strlen(Sim_Buf));
						sprintf(buf, "%s", Sim_Buf);
						sAPI_UartPrintf(buf);
					}
else if(s_nMSettings.m_settings_count==10)
									{
										s_nMSettings.m_uart_send_flag=1;
										unsigned char l_pumpno_tx;
										l_pumpno_tx=s_nMSettings.m_pumpno_send-1;
										sprintf(Sim_Buf,"$G,S,%01d,%02d,%02d,%02d,%.01f,%.01f,%01d,%02d,%02d,%02d,324,\n\r",
											 PresSenFlag,PreSenTime[0],PreSenTime[1],PreSenTime[2],HighFlow,LowFlow,PreSwFlag,PreSwTime[0],PreSwTime[1],PreSwTime[2]);
											 sAPI_UartWrite(SC_UART,(UINT8*)Sim_Buf,strlen(Sim_Buf));
											 sprintf(buf,"%s",Sim_Buf);
											 sAPI_UartPrintf(buf);
				 
									}
									else if(s_nMSettings.m_settings_count==11)
									{
										s_nMSettings.m_uart_send_flag=1;
										unsigned char l_pumpno_tx;
										l_pumpno_tx=s_nMSettings.m_pumpno_send-1;
										sprintf(Sim_Buf,"$Y,S,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%01d,%02d,%02d,%02d,%02d,324,\n\r",
											 s_nMSettings.m_ValveTimerHr[0],s_nMSettings.m_ValveTimerMin[0],s_nMSettings.m_ValveTimerHr[1],s_nMSettings.m_ValveTimerMin[1],s_nMSettings.m_ValveTimerHr[2],s_nMSettings.m_ValveTimerMin[2],
											 s_nMSettings.m_ValveTimerHr[3],s_nMSettings.m_ValveTimerMin[3],s_nMSettings.m_ValveTimerHr[4],s_nMSettings.m_ValveTimerMin[4],s_nMSettings.m_ValveTimerHr[5],s_nMSettings.m_ValveTimerMin[5],
											 s_nMSettings.m_ValveTimerHr[6],s_nMSettings.m_ValveTimerMin[6],s_nMSettings.m_ValveTimerHr[7],s_nMSettings.m_ValveTimerMin[7],s_nMSettings.m_ValveTimerHr[8],s_nMSettings.m_ValveTimerMin[8],
											 s_nMSettings.m_ValveTimerHr[9],s_nMSettings.m_ValveTimerMin[9],s_nMSettings.ValveOnOff,s_nMSettings.CyclicLimit,s_nMSettings.CycLicIntervelHr,s_nMSettings.CycLicIntervelMin,s_nMSettings.CyclicOnOff);
											 sAPI_UartWrite(SC_UART,(UINT8*)Sim_Buf,strlen(Sim_Buf));
											 sprintf(buf,"%s",Sim_Buf);
											 sAPI_UartPrintf(buf);
				 
									}
								}
								
							}

			if (s_nMSettings.calibration_flag == 1 && s_nMSettings.m_uart_send_flag == 0)
			{

				sprintf(Sim_Buf, "$X,S,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.3f,%0.3f,%02d,%0.3f,%01d,%0.3f,\n\r",
						s_nMSettings.m_CalRVoltage, s_nMSettings.m_CalYVoltage, s_nMSettings.m_CalBVoltage, s_nMSettings.m_CalRCurrent, s_nMSettings.m_CalYCurrent, s_nMSettings.m_CalBCurrent,
						s_nMSettings.m_Flow_calfactor, s_nMSettings.m_Press_calfactor, s_ntank.m_Press_Max_Value, Liter_Per_Pulse, Flow_reset, s_ntank.m_Level_calfactor);
				//	s_nMSettings.m_AutoStIIOnOff[0],s_nMSettings.m_CTRonoff,s_nMSettings.m_CTYonoff,s_nMSettings.m_CTBonoff,s_ntank.m_tank_height,s_ntank.m_min_level_p,s_ntank.m_max_level_p,s_nTimerSettings.m_AutoRst2On);

				sAPI_UartWrite(SC_UART, (UINT8 *)Sim_Buf, strlen(Sim_Buf));
				sprintf(buf, "%s", Sim_Buf);
				sAPI_UartPrintf(buf);
				s_nMSettings.calibration_flag = 0;
			}
			/*if(s_nMSettings.m_Enter>=70 && PowerCurrentCondition == 0 )
			{
				nMSettings.motorallonofcount++;
			if(nMSettings.motor1onof ==1)
			{
				if((nMSettings.motor1onofcount<3) && ((nMSettings.motorallonofcount >=1) && (nMSettings.motorallonofcount <4)))
				{
					nMSettings.motor1onofcount++;
					sprintf(textBuf,"$D,M,1,1,1,%ld,%ld,%ld,1,1\n\r ",zoneid[zonecom.extid].ida1,zoneid[zonecom.extid].ida2,zoneid[zonecom.extid].ida3);
					sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
					sprintf(buf,"%s\n\r ",textBuf);while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
					 sAPI_UartPrintf(buf);
				}

			}
			else if(nMSettings.motor1onof ==0)
			{
				if((nMSettings.motor1onofcount<3) && ((nMSettings.motorallonofcount >=1) && (nMSettings.motorallonofcount <4)))
				{
					nMSettings.motor1onofcount++;
					sprintf(textBuf,"$D,M,1,0,1,%ld,%ld,%ld,1,1\n\r ",zoneid[zonecom.extid].ida1,zoneid[zonecom.extid].ida2,zoneid[zonecom.extid].ida3);
					sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
					sprintf(buf,"%s\n\r ",textBuf);while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
					sAPI_UartPrintf(buf);
				}

			}

			if(nMSettings.motor2onof ==1)
			{
				if((nMSettings.motor2onofcount<3) && ((nMSettings.motorallonofcount >=4) && (nMSettings.motorallonofcount <7)))

				{
					nMSettings.motor2onofcount++;
					sprintf(textBuf,"$D,M,2,1,2,%ld,%ld,%ld,1,1\n\r ",zoneid[zonecom.extid].ida1,zoneid[zonecom.extid].ida2,zoneid[zonecom.extid].ida3);
					sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
					sprintf(buf,"%s\n\r ",textBuf);while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
					sAPI_UartPrintf(buf);

				}

			}
			else if(nMSettings.motor2onof ==0)
			{
			//	if(nMSettings.motor2onofcount<3)
				if((nMSettings.motor2onofcount<3) && ((nMSettings.motorallonofcount >=4) && (nMSettings.motorallonofcount <7)))
				{
					nMSettings.motor2onofcount++;
					sprintf(textBuf,"$D,M,2,0,2,%ld,%ld,%ld,1,1\n\r ",zoneid[zonecom.extid].ida1,zoneid[zonecom.extid].ida2,zoneid[zonecom.extid].ida3);
					sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
					sprintf(buf,"%s\n\r ",textBuf);while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;

					   sAPI_UartPrintf(buf);
				}

			}

			if(nMSettings.motor3onof ==1)
			{
				if((nMSettings.motor3onofcount<3) && ((nMSettings.motorallonofcount >=7) && (nMSettings.motorallonofcount <10)))

				{
					nMSettings.motor3onofcount++;
					sprintf(textBuf,"$D,M,3,1,3,%ld,%ld,%ld,1,1\n\r ",zoneid[zonecom.extid].ida1,zoneid[zonecom.extid].ida2,zoneid[zonecom.extid].ida3);
					sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
					sprintf(buf,"%s\n\r ",textBuf);while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;

				   sAPI_UartPrintf(buf);
				}

			}
			else if(nMSettings.motor3onof ==0)
			{
				if((nMSettings.motor3onofcount<3) && ((nMSettings.motorallonofcount >=7) && (nMSettings.motorallonofcount <10)))

				{
					nMSettings.motor3onofcount++;
					sprintf(textBuf,"$D,M,3,0,3,%ld,%ld,%ld,1,1\n\r ",zoneid[zonecom.extid].ida1,zoneid[zonecom.extid].ida2,zoneid[zonecom.extid].ida3);
					sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
					sprintf(buf,"%s\n\r ",textBuf);while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
								sAPI_UartPrintf(buf);

				}

			}

			if((nMSettings.motor4onof ==1) && (nMSettings.motor4ctrlonof ==0))
			{
				if((nMSettings.motor4onofcount<3) && ((nMSettings.motorallonofcount >=10) && (nMSettings.motorallonofcount <13)))

				{
					nMSettings.motor4onofcount++;
					sprintf(textBuf,"$D,M,4,1,4,%ld,%ld,%ld,1,1\n\r ",zoneid[zonecom.extid].ida1,zoneid[zonecom.extid].ida2,zoneid[zonecom.extid].ida3);
					sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
					sprintf(buf,"%s\n\r ",textBuf);while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
				   sAPI_UartPrintf(buf);

				}

			}
			else if((nMSettings.motor4onof ==0) && (nMSettings.motor4ctrlonof ==0))
			{
				if((nMSettings.motor4onofcount<3) && ((nMSettings.motorallonofcount >=10) && (nMSettings.motorallonofcount <13)))

				{
					nMSettings.motor4onofcount++;
					sprintf(textBuf,"$D,M,4,0,4,%ld,%ld,%ld,1,1\n\r ",zoneid[zonecom.extid].ida1,zoneid[zonecom.extid].ida2,zoneid[zonecom.extid].ida3);
					sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
					sprintf(buf,"%s\n\r ",textBuf);while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
					 sAPI_UartPrintf(buf);

				}

			}

			if(nMSettings.motorallonofcount>=13)
			nMSettings.motorallonofcount=0;
			}
			else
			{
			nMSettings.motor1onofcount=0;


			nMSettings.motor4onofcount=0;
			nMSettings.motor1onof=0;
			} */

#if 0

							if(pressure_calib_flag>=1)
							{
							pressure_calib_flag=0;
							sprintf(textBuf,"$D,M,83,83,83,83,83,83");
							sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
			                sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
	                        while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
							sprintf(buf,"%s\n\r ",textBuf);
                             sAPI_UartPrintf(buf);
							}


							if(setserialflag>0)
							{
							if(setserialflagcount++>10)
							{
								setserialflagcount=0;
							sprintf(buf,"setserialflag data***************/n/r");
							sAPI_UartPrintf(buf);
							//sprintf(textBuf,"$D,M,0,0,0,%ld,%ld,%ld,%ld,%ld\n\r ",zoneid[setserialflag].ida1,zoneid[setserialflag].ida2,zoneid[setserialflag].ida3,setserialflag,zoneid[setserialflag].ida4 );
			                sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
			                sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
							setserialflag++;
							if(setserialflag>nDripSettings.decidefblast)
							{
							setserialflag=0;
							sprintf(textBuf,"$D,M,2,2,2,2,%ld,%ld,%ld,%ld\n\r ",zonecom.humonlevel,zonecom.humoflevel,zonecom.temponlevel,zonecom.tempoflevel);
			                sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
			                sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));

							}
							}
							}

							/* if((setserialflag_1>0) && (setserialflag_1<=3))
							{
							if(setserialflagcount1++>10)
							{
								setserialflagcount1=0;
							sprintf(buf,"setserialflag data***************///n/r");
							// sAPI_UartPrintf(buf);
							//sprintf(textBuf,"$D,M,0,0,0,%ld,%ld,%ld,%ld,%ld\n\r ",zoneid[setserial1_val].ida1,zoneid[setserial1_val].ida2,zoneid[setserial1_val].ida3,setserial1_val,zoneid[setserial1_val].ida4 );
			              //  sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
			               /* sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
							setserialflag_1++;
							}
							} */
							if(setserialflag_1>3)
							setserialflag_1=0;
							if( MakeRealyOn == 0)
							{
								
								
								sAPI_GetRealTimeClock(&datetime);
								if(datetime.tm_hour != ActPrvmosHr && s_nMSettings.m_Enter>=70)
									{
										/*  sprintf(textBuf,"$D,M,1,\n\r ");
			                         sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
	                                 sprintf(buf,"%s\n\r ",textBuf);while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
	                    			 sAPI_UartPrintf(buf);
									 sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
	                                 sprintf(buf,"%s\n\r ",textBuf);while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
	                    			 sAPI_UartPrintf(buf);
									 sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
	                                 sprintf(buf,"%s\n\r ",textBuf);while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
	                       			 sAPI_UartPrintf(buf);*/
									 ActPrvmosHr = datetime.tm_hour;
										/*  if(nMSettings.dataSMSOnOff==1)
									 {
									 getdataflag =1;
									 enter2=0;
									 } */
								     if(nMSettings.sampleSMSOnOff==1)
									 Send_samplemoisture(SmsNumber[0]);
									}

						    }

#endif

			Nooftcpsendcount++;
			sprintf(buf, "\n\rNooftcpsendcount=%d\n\r", Nooftcpsendcount);
			sAPI_UartPrintf(buf);
			// if((Nooftcpsendcount>=500) && (Nooftcpsendcount <=504 )) // dg_changed
			/*if(((Nooftcpsendcount>=500) && (Nooftcpsendcount <=504 ))||((Nooftcpsendcount>=1000) && (Nooftcpsendcount <=1004 ))||((Nooftcpsendcount>=1500) && (Nooftcpsendcount <=1504 )) ||((Nooftcpsendcount>=2000) && (Nooftcpsendcount <=2004 ))||((Nooftcpsendcount>=2500) && (Nooftcpsendcount <=2504 )))// dg_changed
			{
				if((Nooftcpsendcount==500)||(Nooftcpsendcount==1000)||(Nooftcpsendcount==1500)||(Nooftcpsendcount==2000)||(Nooftcpsendcount==2500))
				wifiresetcounter=0;
				if(DeviceConfig.interface == WIFI  && wpsmodeon==0 && apmodeon==0 )
				{
			   sprintf(buf,"\r\n dg_call to reset");
				sAPI_UartPrintf(buf);
				wifireset_chk();
			   // wifiresetflag=1;
			   // wifiresetcounter=0;
				}
			   // Nooftcpsendcount=0;

			}*/

			/*  if((creg_reg_flag==1) && (DeviceConfig.interface == GPRS) && (Nooftcpsendcount>=500))
			{
			   Nooftcpsendcount=0;
			   sAPI_NetworkSetCfun(4);
			   sAPI_TaskSleep(200*2);
			   sAPI_NetworkSetCfun(1);

			} */

			//	  if((creg_reg_flag==1) && (DeviceConfig.interface == GPRS) && (Nooftcpsendcount>=1200))
			if (Nooftcpsendcount >= 3000)
			{
				Nooftcpsendcount = 0;
				sAPI_SysReset();
			}

			if (Nooftcpsendcount >= 3000)
			{
				if (DeviceConfig.interface == GPRS && Nooftcprecvd > 0 && CallModeOn == 0) // dg_call
				{
					sAPI_NetworkSetCfun(4);
					sAPI_TaskSleep(200 * 2);
					sAPI_NetworkSetCfun(1);

					// simcom_cfun(FunCallback);	 // dg_changed
				}
			
				if (DeviceConfig.interface == WIFI && wpsmodeon == 0 && apmodeon == 0)
				{

					wifiresetflag = 1;
					wifiresetcounter = 0;
				}
			
				Nooftcpsendcount = 0;
			}
			if (enter2 < 350)
				enter2++;
#if 0
							 if(sendallfeedbacksmsalert_flag>0)
							 {

									/* if(zoneid[sendallfeedbacksmsalert_flag].fbk==2)
								 {
								 sprintf(feedback_buff,"%s%03d-%04d,",feedback_buff,sendallfeedbacksmsalert_flag,zoneid[sendallfeedbacksmsalert_flag].fbkval);

								 flag_count++;
								 } */
								 sendallfeedbacksmsalert_flag++;



								sprintf(buf,"\n\r feedback_buff = %s\n\r",feedback_buff);
								sAPI_UartPrintf(buf);
								if((flag_count >=10) || (sendallfeedbacksmsalert_flag>nDripSettings.decidefblast))
								{
							//	sprintf(feedback_buff,"%s%03d,",feedback_buff,zoneid[12].fbk);
								if(sendallfeedbacksmsalert_flag_1==1)
								{
								sendallfeedbacksmsalert_flag_1=0;
								if(flag_count>0)
								{
								sprintf(strBuf," %02d123\n Open Valves:",(datetime.tm_sec+2));
								sprintf(strBuf,"%s%s",strBuf,feedback_buff);
								sprintf(feedback_buff,"%s",strBuf);
								sendallfeedbacksmsalert(SmsNumber[0]);
								}
								sprintf(feedback_buff,"");
								}
								else
								sendallfeedbacksmsalert(SmsNumber[0]);
								sendallfeedbacksmsalert_flag=0;
								flag_count=0;
								}



							 }
#endif
			/* if (View_Settings_flag == 1)
			{
				sprintf(buf, "$Q,A,1,\n", Sim_Buf);
				sAPI_UartWrite(SC_UART, (UINT8 *)buf, strlen(buf));
				sAPI_UartPrintf(buf);
				View_Settings_flag = 0;
			} */
			 if(View_Settings_flag==1)
			//if (View_Send_Flag == 1)
			{

				char getbuff[250] = {0};
				char getbuff1[250] = {0};
				char getbuff2[250] = {0};
				char getbuff3[250] = {0};
				char getbuff4[250] = {0};
				char getbuff5[250] = {0};
				char getbuff6[250] = {0};

				View_Settings_count++;
				sprintf(buf, "\n\r line %d View_Settings_count %d", __LINE__, View_Settings_count);
				sAPI_UartPrintf(buf);
				if (View_Settings_count == 1)
				{
					//		 sprintf(getbuff1,"{\r\n\"cC\":\"%s\",\r\n\"cM\":[ \r\n{\"pumpconfig\":%01d},\r\n{\"voltageconfig\":\"%01d,%03d,%03d,%03d,%03d,%01d,%03d,%03d,%03d,%03d,%03d,%01d,%01d\"},",IMEI,g_no_of_pumps,s_nMSettings.m_LowVoltOnOff,s_nTimerSettings.m_LowVoltIII,s_nTimerSettings.m_DiffVoltIII,s_nTimerSettings.m_LowVoltII,s_nTimerSettings.m_DiffVoltII,s_nMSettings.m_HighVoltOnOff,s_nTimerSettings.m_HighVoltIII,s_nTimerSettings.m_HiDiffVoltIII,s_nTimerSettings.m_HighVoltII,s_nTimerSettings.m_HiDiffVoltII,s_nTimerSettings.m_ImbVolt,s_nMSettings.m_RvePhOnoff,s_nMSettings.m_SppOnoff);

					//			sprintf(getbuff2,"\r\n{\"ctconfig\":\"%01d,%01d,%01d,%01d,%03.1f,%03d,%03d\"}\r\n",s_nMSettings.m_AutoStIIOnOff[0],s_nMSettings.m_CTRonoff,s_nMSettings.m_CTYonoff,s_nMSettings.m_CTBonoff,s_ntank.m_tank_height,s_ntank.m_min_level_p,s_ntank.m_max_level_p);

					//	 sprintf(getbuff1,"{\r\n\"cC\":\"%s\",\r\n\"cM\":[ \r\n{\"pumpconfig\":%01d},\r\n{\"ctconfig\":\"%01d,%01d,%01d,%01d,%03.1f,%03d,%03d,0,0,00:00:00,00:00:00,0,00:00:00\"},",IMEI,g_no_of_pumps,s_nMSettings.m_AutoStIIOnOff[0],s_nMSettings.m_CTRonoff,s_nMSettings.m_CTYonoff,s_nMSettings.m_CTBonoff,s_ntank.m_tank_height,s_ntank.m_min_level_p,s_ntank.m_max_level_p);

					//	 sprintf(getbuff1,"{\r\n\"cC\":\"%s\",\r\n\"cM\":[ \r\n{\"pumpconfig\":%01d},\r\n{\"ctconfig\":\"%01d,%01d,%01d,%01d,%03.1f,%03d,%03d,%01d,%01d,%02d:%02d:%02d,%02d:%01d,%02d:%02d:%02d,%02d:%02d:%02d,%02d,%01d,%01d\"},",IMEI,g_no_of_pumps,s_nMSettings.m_AutoStIIOnOff[0],s_nMSettings.m_CTRonoff,s_nMSettings.m_CTYonoff,s_nMSettings.m_CTBonoff,s_ntank.m_tank_height,s_ntank.m_min_level_p,s_ntank.m_max_level_p,s_nTimerSettings.m_AutoRst2On,s_nMSettings.m_NlightOnOf,s_nMSettings.m_NlightRTCOnHr,s_nMSettings.m_NlightRTCOnMin,
					//							s_nMSettings.m_NlightRTCOfHr,s_nMSettings.m_NlightRTCOfMin,s_nMSettings.m_peakHourOnOf,s_nMSettings.m_peakOnHr,s_nMSettings.m_peakOnMin,s_nMSettings.m_peakOnSec,s_nMSettings.m_peakOfHr,s_nMSettings.m_peakOfMin,s_nMSettings.m_peakOfSec,s_ntank.m_Level_Sensor_height,s_ntank.m_Uppertank,s_ntank.m_Lowertank);
					sprintf(getbuff1, "{\r\n\"cC\":\"%s\",\r\n\"cM\":[ \r\n{\"pumpconfig\":%01d},\r\n{\"ctconfig\":\"%01d,%01d,%01d,%01d,%03.1f,%03d,%03d,%01d,%01d,%02d:%02d,%02d:%02d,%01d,%02d:%02d:%02d,%02d:%02d:%02d,%01d,%01d,%02d\"},", IMEI, g_no_of_pumps, s_nMSettings.m_AutoStIIOnOff[0], s_nMSettings.m_CTRonoff, s_nMSettings.m_CTYonoff, s_nMSettings.m_CTBonoff, s_ntank.m_tank_height, s_ntank.m_min_level_p, s_ntank.m_max_level_p, s_nTimerSettings.m_AutoRst2On, s_nMSettings.m_NlightOnOf, s_nMSettings.m_NlightRTCOnHr, s_nMSettings.m_NlightRTCOnMin,
							s_nMSettings.m_NlightRTCOfHr, s_nMSettings.m_NlightRTCOfMin, s_nMSettings.m_peakHourOnOf, s_nMSettings.m_peakOnHr, s_nMSettings.m_peakOnMin, s_nMSettings.m_peakOnSec, s_nMSettings.m_peakOfHr, s_nMSettings.m_peakOfMin, s_nMSettings.m_peakOfSec, s_ntank.m_Uppertank, s_ntank.m_Lowertank, s_ntank.m_Level_Sensor_height);

					sprintf(getbuff2, "\r\n{\"voltageconfig\":\"%01d,%03d,%03d,%03d,%03d,%01d,%03d,%03d,%03d,%03d,%03d,%01d,%01d,%01d\"},", s_nMSettings.m_LowVoltOnOff, s_nTimerSettings.m_LowVoltIII, s_nTimerSettings.m_DiffVoltIII, s_nTimerSettings.m_LowVoltII, s_nTimerSettings.m_DiffVoltII, s_nMSettings.m_HighVoltOnOff, s_nTimerSettings.m_HighVoltIII, s_nTimerSettings.m_HiDiffVoltIII, s_nTimerSettings.m_HighVoltII, s_nTimerSettings.m_HiDiffVoltII, s_nTimerSettings.m_ImbVolt, s_nMSettings.m_RvePhOnoff, s_nMSettings.m_SppOnoff, s_nMSettings.m_CurSppOnOff);
					
					
			//		sprintf(getbuff3, "\r\n{\"calibration\":\"%0.2f,%0.2f,%0.2f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%02d,%0.2f,%01d,%0.3f\"}", s_nMSettings.m_CalRVoltage_Rx, s_nMSettings.m_CalYVoltage_Rx, s_nMSettings.m_CalBVoltage_Rx, s_nMSettings.m_CalRCurrent_Rx, s_nMSettings.m_CalYCurrent_Rx, s_nMSettings.m_CalBCurrent_Rx, s_nMSettings.m_Flow_calfactor, s_nMSettings.m_Press_calfactor, s_ntank.m_Press_Max_Value, Liter_Per_Pulse, Flow_reset, s_ntank.m_Level_calfactor);
			    sprintf(getbuff3,"\r\n{\"calibration\":\"%0.2f,%0.2f,%0.2f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%02d,%05d,%01d,%0.3f\"}",s_nMSettings.m_CalRVoltage_Rx,s_nMSettings.m_CalYVoltage_Rx,s_nMSettings.m_CalBVoltage_Rx,s_nMSettings.m_CalRCurrent_Rx,s_nMSettings.m_CalYCurrent_Rx,s_nMSettings.m_CalBCurrent_Rx,s_nMSettings.m_Flow_calfactor,s_nMSettings.m_Press_calfactor,s_ntank.m_Press_Max_Value,Liter_Per_Pulse,Flow_reset,s_ntank.m_Level_calfactor );

					sprintf(getbuff4, "\r\n],\r\n\"cD\":\"%04d-%02d-%02d\",\r\n\"cT\":\"%02d:%02d:%02d\",\r\n\"mC\":\"VIEW1\"\r\n}", datetime.tm_year, datetime.tm_mon, datetime.tm_mday, datetime.tm_hour, datetime.tm_min, datetime.tm_sec);

					sprintf(getbuff, "%s", getbuff1);
					sprintf(buf, "\n\r line %d getbuff %s", __LINE__, getbuff);
					sAPI_UartPrintf(buf);

					sAPI_UartPrintf(buf);
					sprintf(getbuff, "%s%s", getbuff, getbuff2);
					sprintf(buf, "\n\r line %d getbuff %s", __LINE__, getbuff);
					sAPI_UartPrintf(buf);
					sprintf(getbuff, "%s%s", getbuff, getbuff3);
					sprintf(buf, "\n\r line %d getbuff %s", __LINE__, getbuff);
					sAPI_UartPrintf(buf);

					sprintf(getbuff, "%s%s", getbuff, getbuff4);
					sprintf(buf, "\n\r line %d getbuff %s", __LINE__, getbuff);
					sAPI_UartPrintf(buf);
				}
				if (View_Settings_count >= 2)
				{
					/*
						sprintf(getbuff1,"{\r\n\"cC\":\"%s\",\r\n\"cM\": [ \r\n{\r\n\"pumpno\":%01d,\r\n\"tankconfig\":\"%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d\",\r\n",IMEI,l_pumpval+1,s_npump[l_pumpval].m_no_of_sump_pins,s_npump[l_pumpval].m_sump_pin_no[0],s_npump[l_pumpval].m_sump_pin_no[1],s_npump[l_pumpval].m_no_of_tank_pins,s_npump[l_pumpval].m_tank_pin_no[0],s_npump[l_pumpval].m_tank_pin_no[1],s_npump[l_pumpval].m_Level_on_off,s_npump[l_pumpval].m_flowonof,s_npump[l_pumpval].m_pressureonof);
					//	sprintf(getbuff2,"\"currentconfig\":\"%01d,%02d,%02d,%02d,%03.1f,%03.1f,%01d,%02d,%02d,%02d,%01d,%02d,%02d,%02d,%03d,%01d,%02d,%02d,%02d,%03.1f,%03.1f,%01d,%01d,%01d,%01d\",\r\n",s_nMSettings.m_DrScOnOf[l_pumpval],s_nTimerSettings.m_DrScHr[l_pumpval],s_nTimerSettings.m_DrScMin[l_pumpval],s_nTimerSettings.m_DrScSec[l_pumpval],s_nTimerSettings.m_DrAmpsII[l_pumpval],s_nTimerSettings.m_DrAmpsIII[l_pumpval],s_nMSettings.m_DrReOnOf[l_pumpval],s_nTimerSettings.m_DrReHr[l_pumpval],s_nTimerSettings.m_DrReMin[l_pumpval],s_nTimerSettings.m_DrReSec[l_pumpval],s_nMSettings.m_DrOccurOnOff[l_pumpval],s_nTimerSettings.m_DrOccurTimHr[l_pumpval],s_nTimerSettings.m_DrOccurTimMin[l_pumpval],s_nTimerSettings.m_DrOccurTimSec[l_pumpval],a_occurance_count[l_pumpval],s_nMSettings.m_OlOnOff[l_pumpval],s_nTimerSettings.m_OlScanHr[l_pumpval],s_nTimerSettings.m_OlScanMin[l_pumpval],s_nTimerSettings.m_OlScanSec[l_pumpval],s_nTimerSettings.m_OlAmpsII[l_pumpval],s_nTimerSettings.m_OlAmpsIII[l_pumpval],s_nMSettings.m_Drrestartpoweronof[l_pumpval],s_nMSettings.m_OlRstVolOnoff[l_pumpval],s_nTimerSettings.m_AutoRstOn[l_pumpval],s_nTimerSettings.m_AutoRst2On);
						sprintf(getbuff2,"\"currentconfig\":\"%01d,%02d,%02d,%02d,%03.1f,%03.1f,%01d,%02d,%02d,%02d,%01d,%02d,%02d,%02d,%03d,%01d,%01d,%02d,%02d,%02d,%03.1f,%03.1f,%01d,%01d,%01d\",\r\n",s_nMSettings.m_DrScOnOf[l_pumpval],s_nTimerSettings.m_DrScHr[l_pumpval],s_nTimerSettings.m_DrScMin[l_pumpval],s_nTimerSettings.m_DrScSec[l_pumpval],s_nTimerSettings.m_DrAmpsII[l_pumpval],s_nTimerSettings.m_DrAmpsIII[l_pumpval],s_nMSettings.m_DrReOnOf[l_pumpval],s_nTimerSettings.m_DrReHr[l_pumpval],s_nTimerSettings.m_DrReMin[l_pumpval],s_nTimerSettings.m_DrReSec[l_pumpval],s_nMSettings.m_DrOccurOnOff[l_pumpval],s_nTimerSettings.m_DrOccurTimHr[l_pumpval],s_nTimerSettings.m_DrOccurTimMin[l_pumpval],s_nTimerSettings.m_DrOccurTimSec[l_pumpval],a_occurance_count[l_pumpval],s_nMSettings.m_Drrestartpoweronof[pumpno_tx],s_nMSettings.m_OlOnOff[l_pumpval],s_nTimerSettings.m_OlScanHr[l_pumpval],s_nTimerSettings.m_OlScanMin[l_pumpval],s_nTimerSettings.m_OlScanSec[l_pumpval],s_nTimerSettings.m_OlAmpsII[l_pumpval],s_nTimerSettings.m_OlAmpsIII[l_pumpval],s_nMSettings.m_Drrestartpoweronof[l_pumpval],s_nMSettings.m_OlRstVolOnoff[l_pumpval],s_nTimerSettings.m_AutoRstOn[l_pumpval]);

						sprintf(getbuff3,"\"delayconfig\":\"%02d,%02d,%02d,%02d,%02d,%02d,%01d,%02d,%02d,%02d,%01d,%02d,%02d,%02d,%01d,%02d,%02d,%02d,%01d,%02d,%02d,%02d,%02d,%02d,%02d\",\r\n",s_nTimerSettings.m_POnHr[l_pumpval],s_nTimerSettings.m_POnMin[l_pumpval],s_nTimerSettings.m_POnSec[l_pumpval],s_nTimerSettings.m_SDHr[l_pumpval],s_nTimerSettings.m_SDMin[l_pumpval],s_nTimerSettings.m_SDSec[l_pumpval],s_nMSettings.m_PoScrDlOnOff[l_pumpval],s_nTimerSettings.m_PoScrDlHr[l_pumpval],s_nTimerSettings.m_PoScrDlMin[l_pumpval],s_nTimerSettings.m_PoScrDlSec[l_pumpval],s_nMSettings.m_SfbOnOff[l_pumpval],s_nTimerSettings.m_SfbHr[l_pumpval],s_nTimerSettings.m_SfbMin[l_pumpval],s_nTimerSettings.m_SfbSec[l_pumpval],s_nMSettings.m_MaxRnOnOf[l_pumpval],s_nTimerSettings.m_MaxRnHr[l_pumpval],s_nTimerSettings.m_MaxRnMin[l_pumpval],s_nTimerSettings.m_MaxRnSec[l_pumpval],s_nMSettings.m_CycLicOnOf[l_pumpval],s_nTimerSettings.m_CycLicOnHr[l_pumpval],s_nTimerSettings.m_CycLicOnMin[l_pumpval],s_nTimerSettings.m_CycLicOnSec[l_pumpval],s_nTimerSettings.m_CycLicOfHr[l_pumpval],s_nTimerSettings.m_CycLicOfMin[l_pumpval],s_nTimerSettings.m_CycLicOfSec[l_pumpval]);
						sprintf(getbuff4,"\"rtcconfig\":\"%01d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d\",\r\n",s_nMSettings.m_RTCOnOf[l_pumpval],s_nTimerSettings.m_RTCONHr[l_pumpval][1],s_nTimerSettings.m_RTCONMin[l_pumpval][1],s_nTimerSettings.m_RTCONSec[l_pumpval][1], s_nTimerSettings.m_RTCOfHr[l_pumpval][1], s_nTimerSettings.m_RTCOfMin[l_pumpval][1], s_nTimerSettings.m_RTCOfSec[l_pumpval][1],s_nTimerSettings.m_RTCONHr[l_pumpval][2],s_nTimerSettings.m_RTCONMin[l_pumpval][2],s_nTimerSettings.m_RTCONSec[l_pumpval][2], s_nTimerSettings.m_RTCOfHr[l_pumpval][2], s_nTimerSettings.m_RTCOfMin[l_pumpval][2], s_nTimerSettings.m_RTCOfSec[l_pumpval][2],s_nTimerSettings.m_RTCONHr[l_pumpval][3],s_nTimerSettings.m_RTCONMin[l_pumpval][3],s_nTimerSettings.m_RTCONSec[l_pumpval][3], s_nTimerSettings.m_RTCOfHr[l_pumpval][3], s_nTimerSettings.m_RTCOfMin[l_pumpval][3], s_nTimerSettings.m_RTCOfSec[l_pumpval][3],s_nTimerSettings.m_RTCONHr[l_pumpval][4],s_nTimerSettings.m_RTCONMin[l_pumpval][4],s_nTimerSettings.m_RTCONSec[l_pumpval][4], s_nTimerSettings.m_RTCOfHr[l_pumpval][4], s_nTimerSettings.m_RTCOfMin[l_pumpval][4], s_nTimerSettings.m_RTCOfSec[l_pumpval][4],	s_nTimerSettings.m_RTCONHr[l_pumpval][5],s_nTimerSettings.m_RTCONMin[l_pumpval][5],s_nTimerSettings.m_RTCONSec[l_pumpval][5], s_nTimerSettings.m_RTCOfHr[l_pumpval][5], s_nTimerSettings.m_RTCOfMin[l_pumpval][5], s_nTimerSettings.m_RTCOfSec[l_pumpval][5],s_nTimerSettings.m_RTCONHr[l_pumpval][6],s_nTimerSettings.m_RTCONMin[l_pumpval][6],s_nTimerSettings.m_RTCONSec[l_pumpval][6], s_nTimerSettings.m_RTCOfHr[l_pumpval][6], s_nTimerSettings.m_RTCOfMin[l_pumpval][6], s_nTimerSettings.m_RTCOfSec[l_pumpval][6]);
						sprintf(getbuff5,"\"scheduleconfig\":\"%01d,%01d,%01d,%02d,%02d\"\r\n",s_npump[l_pumpval+1].m_Tank_on_off,s_npump[l_pumpval+1].m_Sump_on_off,s_npump[l_pumpval+1].m_scheduleonof,s_npump[l_pumpval+1].m_rundays,s_npump[l_pumpval+1].m_skipdays);
						sprintf(getbuff6,"}\r\n],\r\n\"cD\":\"%04d-%02d-%02d\",\r\n\"cT\":\"%d:%d:%d\",\r\n\"mC\":\"VIEW1\"\r\n}",datetime.tm_year,datetime.tm_mon,datetime.tm_mday,datetime.tm_hour,datetime.tm_min,datetime.tm_sec); */

					/* sprintf(getbuff1,"{\r\n\"cC\":\"%s\",\r\n\"cM\": [\r\n{\"pumpno\":%01d},\r\n{\"tankconfig\":\"%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d\"},\r\n",IMEI,l_pumpval+1,s_npump[l_pumpval].m_no_of_sump_pins,s_npump[l_pumpval].m_sump_pin_no[0],s_npump[l_pumpval].m_sump_pin_no[1],s_npump[l_pumpval].m_no_of_tank_pins,s_npump[l_pumpval].m_tank_pin_no[0],s_npump[l_pumpval].m_tank_pin_no[1],s_npump[l_pumpval].m_Level_on_off,s_npump[l_pumpval].m_flowonof,s_npump[l_pumpval].m_pressureonof);
					sprintf(getbuff2,"{\"currentconfig\":\"%01d,%02d:%02d:%02d,%03.1f,%03.1f,%01d,%02d:%02d:%02d,%01d,%02d:%02d:%02d,%03d,%01d,%01d,%02d:%02d:%02d,%03.1f,%03.1f,%01d,%01d,%01d\"},\r\n",s_nMSettings.m_DrScOnOf[l_pumpval],s_nTimerSettings.m_DrScHr[l_pumpval],s_nTimerSettings.m_DrScMin[l_pumpval],s_nTimerSettings.m_DrScSec[l_pumpval],s_nTimerSettings.m_DrAmpsII[l_pumpval],s_nTimerSettings.m_DrAmpsIII[l_pumpval],s_nMSettings.m_DrReOnOf[l_pumpval],s_nTimerSettings.m_DrReHr[l_pumpval],s_nTimerSettings.m_DrReMin[l_pumpval],s_nTimerSettings.m_DrReSec[l_pumpval],s_nMSettings.m_DrOccurOnOff[l_pumpval],s_nTimerSettings.m_DrOccurTimHr[l_pumpval],s_nTimerSettings.m_DrOccurTimMin[l_pumpval],s_nTimerSettings.m_DrOccurTimSec[l_pumpval],a_occurance_count[l_pumpval],s_nMSettings.m_Drrestartpoweronof[pumpno_tx],s_nMSettings.m_OlOnOff[l_pumpval],s_nTimerSettings.m_OlScanHr[l_pumpval],s_nTimerSettings.m_OlScanMin[l_pumpval],s_nTimerSettings.m_OlScanSec[l_pumpval],s_nTimerSettings.m_OlAmpsII[l_pumpval],s_nTimerSettings.m_OlAmpsIII[l_pumpval],s_nMSettings.m_Drrestartpoweronof[l_pumpval],s_nMSettings.m_OlRstVolOnoff[l_pumpval],s_nTimerSettings.m_AutoRstOn[l_pumpval]);

					sprintf(getbuff3,"{\"delayconfig\":\"%02d:%02d:%02d,%02d:%02d:%02d,%01d,%02d:%02d:%02d,%01d,%02d:%02d:%02d,%01d,%02d:%02d:%02d,%01d,%02d:%02d:%02d,%02d:%02d:%02d\"},\r\n",s_nTimerSettings.m_POnHr[l_pumpval],s_nTimerSettings.m_POnMin[l_pumpval],s_nTimerSettings.m_POnSec[l_pumpval],s_nTimerSettings.m_SDHr[l_pumpval],s_nTimerSettings.m_SDMin[l_pumpval],s_nTimerSettings.m_SDSec[l_pumpval],s_nMSettings.m_PoScrDlOnOff[l_pumpval],s_nTimerSettings.m_PoScrDlHr[l_pumpval],s_nTimerSettings.m_PoScrDlMin[l_pumpval],s_nTimerSettings.m_PoScrDlSec[l_pumpval],s_nMSettings.m_SfbOnOff[l_pumpval],s_nTimerSettings.m_SfbHr[l_pumpval],s_nTimerSettings.m_SfbMin[l_pumpval],s_nTimerSettings.m_SfbSec[l_pumpval],s_nMSettings.m_MaxRnOnOf[l_pumpval],s_nTimerSettings.m_MaxRnHr[l_pumpval],s_nTimerSettings.m_MaxRnMin[l_pumpval],s_nTimerSettings.m_MaxRnSec[l_pumpval],s_nMSettings.m_CycLicOnOf[l_pumpval],s_nTimerSettings.m_CycLicOnHr[l_pumpval],s_nTimerSettings.m_CycLicOnMin[l_pumpval],s_nTimerSettings.m_CycLicOnSec[l_pumpval],s_nTimerSettings.m_CycLicOfHr[l_pumpval],s_nTimerSettings.m_CycLicOfMin[l_pumpval],s_nTimerSettings.m_CycLicOfSec[l_pumpval]);

					sprintf(getbuff4,"{\"rtcconfig\":\"%01d,%02d:%02d:%02d,%02d:%02d:%02d,%02d:%02d:%02d,%02d:%02d:%02d,%02d:%02d:%02d,%02d:%02d:%02d,%02d:%02d:%02d,%02d:%02d:%02d,%02d:%02d:%02d,%02d:%02d:%02d,%02d:%02d:%02d,%02d:%02d:%02d\"},\r\n",s_nMSettings.m_RTCOnOf[l_pumpval],s_nTimerSettings.m_RTCONHr[l_pumpval][1],s_nTimerSettings.m_RTCONMin[l_pumpval][1],s_nTimerSettings.m_RTCONSec[l_pumpval][1], s_nTimerSettings.m_RTCOfHr[l_pumpval][1], s_nTimerSettings.m_RTCOfMin[l_pumpval][1], s_nTimerSettings.m_RTCOfSec[l_pumpval][1],s_nTimerSettings.m_RTCONHr[l_pumpval][2],s_nTimerSettings.m_RTCONMin[l_pumpval][2],s_nTimerSettings.m_RTCONSec[l_pumpval][2], s_nTimerSettings.m_RTCOfHr[l_pumpval][2], s_nTimerSettings.m_RTCOfMin[l_pumpval][2], s_nTimerSettings.m_RTCOfSec[l_pumpval][2],s_nTimerSettings.m_RTCONHr[l_pumpval][3],s_nTimerSettings.m_RTCONMin[l_pumpval][3],s_nTimerSettings.m_RTCONSec[l_pumpval][3], s_nTimerSettings.m_RTCOfHr[l_pumpval][3], s_nTimerSettings.m_RTCOfMin[l_pumpval][3], s_nTimerSettings.m_RTCOfSec[l_pumpval][3],s_nTimerSettings.m_RTCONHr[l_pumpval][4],s_nTimerSettings.m_RTCONMin[l_pumpval][4],s_nTimerSettings.m_RTCONSec[l_pumpval][4], s_nTimerSettings.m_RTCOfHr[l_pumpval][4], s_nTimerSettings.m_RTCOfMin[l_pumpval][4], s_nTimerSettings.m_RTCOfSec[l_pumpval][4],	s_nTimerSettings.m_RTCONHr[l_pumpval][5],s_nTimerSettings.m_RTCONMin[l_pumpval][5],s_nTimerSettings.m_RTCONSec[l_pumpval][5], s_nTimerSettings.m_RTCOfHr[l_pumpval][5], s_nTimerSettings.m_RTCOfMin[l_pumpval][5], s_nTimerSettings.m_RTCOfSec[l_pumpval][5],s_nTimerSettings.m_RTCONHr[l_pumpval][6],s_nTimerSettings.m_RTCONMin[l_pumpval][6],s_nTimerSettings.m_RTCONSec[l_pumpval][6], s_nTimerSettings.m_RTCOfHr[l_pumpval][6], s_nTimerSettings.m_RTCOfMin[l_pumpval][6], s_nTimerSettings.m_RTCOfSec[l_pumpval][6]);
					sprintf(getbuff5,"{\"scheduleconfig\":\"%01d,%01d,%01d,%02d,%02d\"}\r\n",s_npump[l_pumpval+1].m_Tank_on_off,s_npump[l_pumpval+1].m_Sump_on_off,s_npump[l_pumpval+1].m_scheduleonof,s_npump[l_pumpval+1].m_rundays,s_npump[l_pumpval+1].m_skipdays); */

					sprintf(getbuff1, "{\r\n\"cC\":\"%s\",\r\n\"cM\": [\r\n{\"pumpno\":%01d},\r\n{\"delayconfig\":\"%02d:%02d:%02d,%02d:%02d:%02d,%01d,%02d:%02d:%02d,%01d,%02d:%02d:%02d,%01d,%02d:%02d:%02d,%01d,%02d:%02d:%02d,%02d:%02d:%02d\"},\r\n", IMEI, l_pumpval + 1, s_nTimerSettings.m_POnHr[l_pumpval], s_nTimerSettings.m_POnMin[l_pumpval], s_nTimerSettings.m_POnSec[l_pumpval], s_nTimerSettings.m_SDHr[l_pumpval], s_nTimerSettings.m_SDMin[l_pumpval], s_nTimerSettings.m_SDSec[l_pumpval], s_nMSettings.m_PoScrDlOnOff[l_pumpval], s_nTimerSettings.m_PoScrDlHr[l_pumpval], s_nTimerSettings.m_PoScrDlMin[l_pumpval], s_nTimerSettings.m_PoScrDlSec[l_pumpval], s_nMSettings.m_SfbOnOff[l_pumpval], s_nTimerSettings.m_SfbHr[l_pumpval], s_nTimerSettings.m_SfbMin[l_pumpval], s_nTimerSettings.m_SfbSec[l_pumpval], s_nMSettings.m_MaxRnOnOf[l_pumpval], s_nTimerSettings.m_MaxRnHr[l_pumpval], s_nTimerSettings.m_MaxRnMin[l_pumpval], s_nTimerSettings.m_MaxRnSec[l_pumpval], s_nMSettings.m_CycLicOnOf[l_pumpval], s_nTimerSettings.m_CycLicOnHr[l_pumpval], s_nTimerSettings.m_CycLicOnMin[l_pumpval], s_nTimerSettings.m_CycLicOnSec[l_pumpval], s_nTimerSettings.m_CycLicOfHr[l_pumpval], s_nTimerSettings.m_CycLicOfMin[l_pumpval], s_nTimerSettings.m_CycLicOfSec[l_pumpval]);
					sAPI_UartPrintf(getbuff1);
					sprintf(getbuff2, "{\"rtcconfig\":\"%01d,%02d:%02d:%02d,%02d:%02d:%02d,%02d:%02d:%02d,%02d:%02d:%02d,%02d:%02d:%02d,%02d:%02d:%02d,%02d:%02d:%02d,%02d:%02d:%02d,%02d:%02d:%02d,%02d:%02d:%02d,%02d:%02d:%02d,%02d:%02d:%02d\"},\r\n", s_nMSettings.m_RTCOnOf[l_pumpval], s_nTimerSettings.m_RTCONHr[l_pumpval][1], s_nTimerSettings.m_RTCONMin[l_pumpval][1], s_nTimerSettings.m_RTCONSec[l_pumpval][1], s_nTimerSettings.m_RTCOfHr[l_pumpval][1], s_nTimerSettings.m_RTCOfMin[l_pumpval][1], s_nTimerSettings.m_RTCOfSec[l_pumpval][1], s_nTimerSettings.m_RTCONHr[l_pumpval][2], s_nTimerSettings.m_RTCONMin[l_pumpval][2], s_nTimerSettings.m_RTCONSec[l_pumpval][2], s_nTimerSettings.m_RTCOfHr[l_pumpval][2], s_nTimerSettings.m_RTCOfMin[l_pumpval][2], s_nTimerSettings.m_RTCOfSec[l_pumpval][2], s_nTimerSettings.m_RTCONHr[l_pumpval][3], s_nTimerSettings.m_RTCONMin[l_pumpval][3], s_nTimerSettings.m_RTCONSec[l_pumpval][3], s_nTimerSettings.m_RTCOfHr[l_pumpval][3], s_nTimerSettings.m_RTCOfMin[l_pumpval][3], s_nTimerSettings.m_RTCOfSec[l_pumpval][3], s_nTimerSettings.m_RTCONHr[l_pumpval][4], s_nTimerSettings.m_RTCONMin[l_pumpval][4], s_nTimerSettings.m_RTCONSec[l_pumpval][4], s_nTimerSettings.m_RTCOfHr[l_pumpval][4], s_nTimerSettings.m_RTCOfMin[l_pumpval][4], s_nTimerSettings.m_RTCOfSec[l_pumpval][4], s_nTimerSettings.m_RTCONHr[l_pumpval][5], s_nTimerSettings.m_RTCONMin[l_pumpval][5], s_nTimerSettings.m_RTCONSec[l_pumpval][5], s_nTimerSettings.m_RTCOfHr[l_pumpval][5], s_nTimerSettings.m_RTCOfMin[l_pumpval][5], s_nTimerSettings.m_RTCOfSec[l_pumpval][5], s_nTimerSettings.m_RTCONHr[l_pumpval][6], s_nTimerSettings.m_RTCONMin[l_pumpval][6], s_nTimerSettings.m_RTCONSec[l_pumpval][6], s_nTimerSettings.m_RTCOfHr[l_pumpval][6], s_nTimerSettings.m_RTCOfMin[l_pumpval][6], s_nTimerSettings.m_RTCOfSec[l_pumpval][6]);
					sAPI_UartPrintf(getbuff2);
					sprintf(buf, "\n l_pumpval:%d\n", l_pumpval);
					sAPI_UartPrintf(buf);

					//			sprintf(getbuff3,"{\"currentconfig\":\"%01d,%02d:%02d:%02d,%03.1f,%03.1f,%01d,%02d:%02d:%02d,%01d,%02d:%02d:%02d,%03d,%01d,%01d,%02d:%02d:%02d,%03.1f,%03.1f,%01d,%01d,%01d\"},\r\n",s_nMSettings.m_DrScOnOf[l_pumpval],s_nTimerSettings.m_DrScHr[l_pumpval],s_nTimerSettings.m_DrScMin[l_pumpval],s_nTimerSettings.m_DrScSec[l_pumpval],s_nTimerSettings.m_DrAmpsIII[l_pumpval],s_nTimerSettings.m_DrAmpsII[l_pumpval],s_nMSettings.m_DrReOnOf[l_pumpval],s_nTimerSettings.m_DrReHr[l_pumpval],s_nTimerSettings.m_DrReMin[l_pumpval],s_nTimerSettings.m_DrReSec[l_pumpval],s_nMSettings.m_DrOccurOnOff[l_pumpval],s_nTimerSettings.m_DrOccurTimHr[l_pumpval],s_nTimerSettings.m_DrOccurTimMin[l_pumpval],s_nTimerSettings.m_DrOccurTimSec[l_pumpval],a_occurance_count[l_pumpval],s_nMSettings.m_Drrestartpoweronof[pumpno_tx],s_nMSettings.m_OlOnOff[l_pumpval],s_nTimerSettings.m_OlScanHr[l_pumpval],s_nTimerSettings.m_OlScanMin[l_pumpval],s_nTimerSettings.m_OlScanSec[l_pumpval],s_nTimerSettings.m_OlAmpsIII[l_pumpval],s_nTimerSettings.m_OlAmpsII[l_pumpval],s_nMSettings.m_Drrestartpoweronof[l_pumpval],s_nMSettings.m_OlRstVolOnoff[l_pumpval],s_nTimerSettings.m_AutoRstOn[l_pumpval]);
					sprintf(getbuff3, "{\"currentconfig\":\"%01d,%02d:%02d:%02d,%03.1f,%03.1f,%01d,%02d:%02d:%02d,%01d,%02d:%02d:%02d,%03d,%01d,%01d,%02d:%02d:%02d,%03.1f,%03.1f,%01d,%01d\"},\r\n", s_nMSettings.m_DrScOnOf[l_pumpval], s_nTimerSettings.m_DrScHr[l_pumpval], s_nTimerSettings.m_DrScMin[l_pumpval], s_nTimerSettings.m_DrScSec[l_pumpval], s_nTimerSettings.m_DrAmpsIII[l_pumpval], s_nTimerSettings.m_DrAmpsII[l_pumpval], s_nMSettings.m_DrReOnOf[l_pumpval], s_nTimerSettings.m_DrReHr[l_pumpval], s_nTimerSettings.m_DrReMin[l_pumpval], s_nTimerSettings.m_DrReSec[l_pumpval], s_nMSettings.m_DrOccurOnOff[l_pumpval], s_nTimerSettings.m_DrOccurTimHr[l_pumpval], s_nTimerSettings.m_DrOccurTimMin[l_pumpval], s_nTimerSettings.m_DrOccurTimSec[l_pumpval], a_occurance_count[l_pumpval], s_nMSettings.m_Drrestartpoweronof[pumpno_tx], s_nMSettings.m_OlOnOff[l_pumpval], s_nTimerSettings.m_OlScanHr[l_pumpval], s_nTimerSettings.m_OlScanMin[l_pumpval], s_nTimerSettings.m_OlScanSec[l_pumpval], s_nTimerSettings.m_OlAmpsIII[l_pumpval], s_nTimerSettings.m_OlAmpsII[l_pumpval], s_nMSettings.m_AutoOlDrRstIIOnOff[l_pumpval], s_nTimerSettings.m_AutoRstOn[l_pumpval]);
					sprintf(getbuff4, "{\"scheduleconfig\":\"%01d,%01d,%01d,%02d,%02d,%01d,%01d\"},\r\n", s_npump[l_pumpval].m_Tank_on_off, s_npump[l_pumpval].m_Sump_on_off, s_npump[l_pumpval].m_scheduleonof, s_npump[l_pumpval].m_rundays, s_npump[l_pumpval].m_skipdays, s_npump[l_pumpval].m_Uppertank_restart_off, s_npump[l_pumpval].m_Lowertank_restart_off);
					sprintf(getbuff5, "{\"tankconfig\":\"%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d\"}", s_npump[l_pumpval].m_no_of_sump_pins, s_npump[l_pumpval].m_sump_pin_no[0], s_npump[l_pumpval].m_sump_pin_no[1], s_npump[l_pumpval].m_no_of_tank_pins, s_npump[l_pumpval].m_tank_pin_no[0], s_npump[l_pumpval].m_tank_pin_no[1], s_npump[l_pumpval].m_Level_on_off, s_npump[l_pumpval].m_flowonof, s_npump[l_pumpval].m_pressureonof);
					sprintf(getbuff6, "\r\n],\r\n\"cD\":\"%04d-%02d-%02d\",\r\n\"cT\":\"%02d:%02d:%02d\",\r\n\"mC\":\"VIEW1\"\r\n}", datetime.tm_year, datetime.tm_mon, datetime.tm_mday, datetime.tm_hour, datetime.tm_min, datetime.tm_sec);
					sAPI_UartPrintf(getbuff3);
					sprintf(getbuff, "%s", getbuff1);
					sprintf(buf, "\n\r line %d getbuff %s", __LINE__, getbuff);
					sAPI_UartPrintf(buf);

					sAPI_UartPrintf(buf);
					sprintf(getbuff, "%s%s", getbuff, getbuff2);
					sprintf(buf, "\n\r line %d getbuff %s", __LINE__, getbuff);
					sAPI_UartPrintf(buf);
					sprintf(getbuff, "%s%s", getbuff, getbuff3);
					sprintf(buf, "\n\r line %d getbuff %s", __LINE__, getbuff);
					sAPI_UartPrintf(buf);

					sprintf(getbuff, "%s%s", getbuff, getbuff4);
					sprintf(buf, "\n\r line %d getbuff %s", __LINE__, getbuff);
					sAPI_UartPrintf(buf);

					sprintf(getbuff, "%s%s", getbuff, getbuff5);
					sprintf(buf, "\n\r line %d getbuff %s", __LINE__, getbuff);
					sAPI_UartPrintf(buf);

					sprintf(getbuff, "%s%s", getbuff, getbuff6);
					sprintf(buf, "\n\r line %d getbuff %s", __LINE__, getbuff);
					sAPI_UartPrintf(buf);
					l_pumpval++;
				}

				sprintf(TCPwifiStrNumber1[Nooftcprecvd1].TCPWifigprsstr1, "%s", getbuff);
				sprintf(buf, "\n\rTCPwifiStrNumber1[%d].TCPWifigprsstr1>>:%s", Nooftcprecvd1, TCPwifiStrNumber1[Nooftcprecvd1].TCPWifigprsstr1);
				sAPI_UartPrintf(buf);
				Nooftcprecvd1++;
				View_Settings_count_max = g_no_of_pumps + 1;

				sprintf(buf, "\n\r View_Settings_count_max %d View_Settings_count %d", View_Settings_count_max, View_Settings_count);
				sAPI_UartPrintf(buf);
				if (View_Settings_count >= View_Settings_count_max)
				{
				View_Settings_flag=0;//	View_Send_Flag = 0;
					View_Settings_count = 0;
					l_pumpval = 0;
				}
			}
#if 0
						//	if((getdataflag>0 && s_nMSettings.m_Enter>=70 && (strlen(IMEI)>0)&&Nooftcprecvd1<=12&&enter2>60 ) || ( s_nMSettings.m_Enter>=70 && (strlen(IMEI)>0)&&Nooftcprecvd1<=12 && getdataflag_1==1)) //340
	if((getdataflag>0 && s_nMSettings.m_Enter>=70 && (strlen(IMEI)>0)&&Nooftcprecvd1<=12&&((zonecom.standalonemodeonof == 1 && enter2>150 )|| (zonecom.standalonemodeonof == 0 && enter2>60 ))) || ( s_nMSettings.m_Enter>=70 && (strlen(IMEI)>0)&&Nooftcprecvd1<=12 && getdataflag_1==1)) //340
    
	{
                     
								char getbuff[500];
								char getbuff1[10];
								int contentlen;
								int counti=0;
							if(getdataflagcount++>6)
								{
								getdataflagcount=0;
					        sprintf(buf,"getdataflag data***************\n\r");
							sAPI_UartPrintf(buf);
							counti=(getdataflag-1)*20;
							counti=counti+1;
							sprintf(FBKBuf,"");
							getdataflag++;
							/*zoneid[i].fsno=1;zoneid[zoneid[i].fsno].datatype=1;zoneid[zoneid[i].fsno].fbk=1;zoneid[zoneid[i].fsno].fbkval=0.09;
							zoneid[i].fsno=2;zoneid[zoneid[i].fsno].datatype=1;zoneid[zoneid[i].fsno].fbk=2;zoneid[zoneid[i].fsno].fbkval=0.09;
							zoneid[i].fsno=3;zoneid[zoneid[i].fsno].datatype=1;zoneid[zoneid[i].fsno].fbk=3;zoneid[zoneid[i].fsno].fbkval=0.09;
							zoneid[i].fsno=4;zoneid[zoneid[i].fsno].datatype=1;zoneid[zoneid[i].fsno].fbk=4;zoneid[zoneid[i].fsno].fbkval=0.09;
							zoneid[i].fsno=5;zoneid[zoneid[i].fsno].datatype=1;zoneid[zoneid[i].fsno].fbk=10;zoneid[zoneid[i].fsno].fbkval=0.09;
							zoneid[i].fsno=6;zoneid[zoneid[i].fsno].datatype=1;zoneid[zoneid[i].fsno].fbk=0;zoneid[zoneid[i].fsno].fbkval=0.09;
							
							zoneid[i].fsno=7;zoneid[zoneid[i].fsno].datatype=1;zoneid[zoneid[i].fsno].fbk=1;zoneid[zoneid[i].fsno].fbkval=0.1;
							zoneid[i].fsno=8;zoneid[zoneid[i].fsno].datatype=1;zoneid[zoneid[i].fsno].fbk=2;zoneid[zoneid[i].fsno].fbkval=0.2;
							zoneid[i].fsno=9;zoneid[zoneid[i].fsno].datatype=1;zoneid[zoneid[i].fsno].fbk=1;zoneid[zoneid[i].fsno].fbkval=0.09;
							zoneid[i].fsno=10;zoneid[zoneid[i].fsno].datatype=1;zoneid[zoneid[i].fsno].fbk=2;zoneid[zoneid[i].fsno].fbkval=0.09;
							zoneid[i].fsno=11;zoneid[zoneid[i].fsno].datatype=2;zoneid[zoneid[i].fsno].fbk=1;zoneid[zoneid[i].fsno].fbkval=38;
							zoneid[i].fsno=12;zoneid[zoneid[i].fsno].datatype=2;zoneid[zoneid[i].fsno].fbk=2;zoneid[zoneid[i].fsno].fbkval=61;
							
							zoneid[i].fsno=13;zoneid[zoneid[i].fsno].datatype=3;zoneid[zoneid[i].fsno].fbk=1;zoneid[zoneid[i].fsno].fbkval=129;
							zoneid[i].fsno=14;zoneid[zoneid[i].fsno].datatype=3;zoneid[zoneid[i].fsno].fbk=2;zoneid[zoneid[i].fsno].fbkval=2240;
							zoneid[i].fsno=15;zoneid[zoneid[i].fsno].datatype=0;zoneid[zoneid[i].fsno].fbk=1;zoneid[zoneid[i].fsno].fbkval=523;
							zoneid[i].fsno=16;zoneid[zoneid[i].fsno].datatype=0;zoneid[zoneid[i].fsno].fbk=2;zoneid[zoneid[i].fsno].fbkval=323;
							zoneid[i].fsno=17;zoneid[zoneid[i].fsno].datatype=4;zoneid[zoneid[i].fsno].fbk=1;zoneid[zoneid[i].fsno].fbkval=38;
							zoneid[i].fsno=18;zoneid[zoneid[i].fsno].datatype=4;zoneid[zoneid[i].fsno].fbk=2;zoneid[zoneid[i].fsno].fbkval=61;
							
							zoneid[i].fsno=19;zoneid[zoneid[i].fsno].datatype=2;zoneid[zoneid[i].fsno].fbk=1;zoneid[zoneid[i].fsno].fbkval=129;
							zoneid[i].fsno=20;zoneid[zoneid[i].fsno].datatype=2;zoneid[zoneid[i].fsno].fbk=2;zoneid[zoneid[i].fsno].fbkval=2240;
							zoneid[i].fsno=21;zoneid[zoneid[i].fsno].datatype=2;zoneid[zoneid[i].fsno].fbk=1;zoneid[zoneid[i].fsno].fbkval=523;
							zoneid[i].fsno=22;zoneid[zoneid[i].fsno].datatype=2;zoneid[zoneid[i].fsno].fbk=2;zoneid[zoneid[i].fsno].fbkval=323;
							zoneid[i].fsno=23;zoneid[zoneid[i].fsno].datatype=0;zoneid[zoneid[i].fsno].fbk=1;zoneid[zoneid[i].fsno].fbkval=383889;
							zoneid[i].fsno=24;zoneid[zoneid[i].fsno].datatype=2;zoneid[zoneid[i].fsno].fbk=2;zoneid[zoneid[i].fsno].fbkval=61;
							zoneid[i].fsno=25;zoneid[zoneid[i].fsno].datatype=2;zoneid[zoneid[i].fsno].fbk=3;zoneid[zoneid[i].fsno].fbkval=61;
							zoneid[i].fsno=26;zoneid[zoneid[i].fsno].datatype=2;zoneid[zoneid[i].fsno].fbk=4;zoneid[zoneid[i].fsno].fbkval=61;
							zoneid[i].fsno=27;zoneid[zoneid[i].fsno].datatype=2;zoneid[zoneid[i].fsno].fbk=10;zoneid[zoneid[i].fsno].fbkval=61;
							zoneid[i].fsno=28;zoneid[zoneid[i].fsno].datatype=2;zoneid[zoneid[i].fsno].fbk=0;zoneid[zoneid[i].fsno].fbkval=61;
							zoneid[i].fsno=29;zoneid[zoneid[i].fsno].datatype=2;zoneid[zoneid[i].fsno].fbk=1;zoneid[zoneid[i].fsno].fbkval=61;
							
							zoneid[i].fsno=30;zoneid[zoneid[i].fsno].datatype=3;zoneid[zoneid[i].fsno].fbk=3;zoneid[zoneid[i].fsno].fbkval=61;
							zoneid[i].fsno=31;zoneid[zoneid[i].fsno].datatype=3;zoneid[zoneid[i].fsno].fbk=4;zoneid[zoneid[i].fsno].fbkval=61;
							zoneid[i].fsno=32;zoneid[zoneid[i].fsno].datatype=3;zoneid[zoneid[i].fsno].fbk=10;zoneid[zoneid[i].fsno].fbkval=61;
							zoneid[i].fsno=33;zoneid[zoneid[i].fsno].datatype=3;zoneid[zoneid[i].fsno].fbk=0;zoneid[zoneid[i].fsno].fbkval=61;
							*/
				for(i=counti;i<counti+20;i++)
				{
							if(i>255 )
							{
							getdataflag=0;
							getdataflag_1=0;
							break;
							}

							//zoneid[i].fsno=i;
                            //zoneid[zoneid[i].fsno].fbk=-1;
                           /*  if(zoneid[i].datatype ==1)
							sprintf(getbuff1,"A");
                            else if(zoneid[i].datatype ==2)
							sprintf(getbuff1,"P");
                            else if(zoneid[i].datatype ==3)
							sprintf(getbuff1,"L");
						    else if(zoneid[i].datatype ==4)
							sprintf(getbuff1,"C");
                            else
							sprintf(getbuff1," "); */
						//    zoneid[i].fsno=1;zoneid[zoneid[i].fsno].fbk=1;zoneid[zoneid[i].fsno].fbkval=0.09;

						   // sprintf(FBKBuf,"%s%03d:%d:%d%s,",FBKBuf,zoneid[i].fsno,zoneid[zoneid[i].fsno].fbk,zoneid[zoneid[i].fsno].fbkval,getbuff1);
						//	sprintf(buf,"%s\n\r ",FBKBuf);
						//	sAPI_UartPrintf(buf);
				}
                             
						//	sprintf(getbuff,"{\r\n\"cC\":\"%s\",\r\n\"cM\":\"%s\",\r\n\"cD\":\"%d/%d/20%02d\",\r\n\"cT\":\"%d:%d:%d\",\r\n\"mC\":\"NLM\"\r\n}",IMEI,FBKBuf,datetime.tm_mday,datetime.tm_mon,datetime.tm_year,datetime.tm_hour,datetime.tm_min,datetime.tm_sec);
							sprintf(getbuff,"{\r\n\"cC\":\"%s\",\r\n\"cM\":\"%s\",\r\n\"cD\":\"%02d/%02d/%04d\",\r\n\"cT\":\"%02d:%02d:%02d\",\r\n\"mC\":\"NLM\"\r\n}",IMEI,FBKBuf,datetime.tm_mday,datetime.tm_mon,datetime.tm_year,datetime.tm_hour,datetime.tm_min,datetime.tm_sec);
							sAPI_UartPrintf(getbuff);
							//
						//	sprintf(getbuff,"{ }\n\r");
							contentlen=strlen(getbuff);
							contentlen=contentlen;
							sprintf(buf,"tcp copy length=%d",contentlen);
							sAPI_UartPrintf(buf);
						//	sprintf(buf,"getbuff copy =%s",getbuff);
						// sAPI_UartPrintf(buf);
						//	memset(&TCPwifiStrNumber[Nooftcprecvd].TCPWifistr,0,500);
							sprintf(TCPwifiStrNumber1[Nooftcprecvd1].TCPWifigprsstr1,"%s",getbuff);
							sprintf(TCPwifiStrNumber1[Nooftcprecvd1].TCPWifistr1,"POST /api/v1/controller/messages/ HTTP/1.1\r\nHost: %s:8080\r\nUser-Agent: curl/7.52.1\r\nAccept: */*\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n\%s\r\n",DeviceConfig.TcpServerIP,contentlen,getbuff);

							memset(&getbuff,0,500);
							contentlen=strlen(TCPwifiStrNumber1[Nooftcprecvd1].TCPWifistr1);
							sprintf(buf,"tcp copy length=%d",contentlen);
							sAPI_UartPrintf(buf);
							if((getdataflag>0) || (getdataflag_1>0))
							Nooftcprecvd1++;

							sprintf(buf,"%s\n\r ",FBKBuf);
	                        sAPI_UartPrintf(buf);
							if(i>nDripSettings.decidefblast)
							{
							getdataflag=0;
							getdataflag_1=0;
							//break;
							}
				}
							//sAPI_UartWrite(eat_uart_wifi, FBKBuf, strlen(FBKBuf));
							//sprintf(buf,"%s\n\r ",FBKBuf);
							//sAPI_UartPrintf(buf);
							//eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 4, "FBKD", EAT_NULL);
							//getdataflag++;
	}
#endif
			/*	if(zoneonof[nVaTr.programselection].moistureonof==1 && MakeRealyOn == 0)
			   {
				   sAPI_GetRealTimeClock(&datetime);
				   if(datetime.tm_hour != ActPrvmosHr)
					   {
						sprintf(textBuf,"$D,M,1,\n\r ");
						sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
						sprintf(buf,"%s\n\r ",textBuf);while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
						sAPI_UartPrintf(buf);
						sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
						sprintf(buf,"%s\n\r ",textBuf);while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
						sAPI_UartPrintf(buf);
						sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
						sprintf(buf,"%s\n\r ",textBuf);while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
						sAPI_UartPrintf(buf);
						ActPrvmosHr = datetime.tm_hour;
					   }
			   }
				if(zoneonof[nVaTr.programselection].levelmodeonof==1 && MakeRealyOn == 0)
			   {
				   sAPI_GetRealTimeClock(&datetime);
				   if(datetime.tm_hour != ActPrvmosHr)
					   {
						sprintf(textBuf,"$D,L,1,\n\r ");
						sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
						sprintf(buf,"%s\n\r ",textBuf);while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
						sAPI_UartPrintf(buf);
						sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
						sprintf(buf,"%s\n\r ",textBuf);while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
						sAPI_UartPrintf(buf);
						sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
						sprintf(buf,"%s\n\r ",textBuf);while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
						sAPI_UartPrintf(buf);
						ActPrvmosHr = datetime.tm_hour;
					   }
			   }
				 if(nVaTr.valvefeedbackonof==1 && MakeRealyOn == 0)
			   {
				   sAPI_GetRealTimeClock(&datetime);
				   if(datetime.tm_hour != ActPrvmosHr)
					   {
						sprintf(textBuf,"$D,G,1,\n\r ");
						sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
						sprintf(buf,"%s\n\r ",textBuf);while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
						sAPI_UartPrintf(buf);
						sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
						sprintf(buf,"%s\n\r ",textBuf);while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
						sAPI_UartPrintf(buf);
						sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
						sprintf(buf,"%s\n\r ",textBuf);while(dricomdelay<1000){dricomdelay++;}dricomdelay=0;
						sAPI_UartPrintf(buf);
						ActPrvmosHr = datetime.tm_hour;
					   }
			   }	*/

#if 0
		if(progqueonof == 1&& proqueflag==0 && nDripSettings.driponof==1 && nVaTr.cycrestartonof==0 && MakeRealyOn == 1 && nVaTr.cyclecompleted==1)
			{
						     char progindex;
							 char prvprogsel;
							 //prvprogsel=nVaTr.programselection;
							 sprintf(buf,"proqueflagcounter = %d ***********************",proqueflagcounter);
							 sAPI_UartPrintf(buf);
							 if(proqueflagcounter++>120)
							 {
							 prvprogsel=nVaTr.programselection;
                             for (i = 1; i < 5; i++)
							 {
								//sprintf(buf,"recived no %s , Reg number %s,%d",p+j,SmsNumber[i],j);
                          // sAPI_UartPrintf(buf);						
						sprintf(buf,"nVaTr.programselection = %d GREENT[i].progque=%d***********************",nVaTr.programselection,GREENT[i].progque);
							sAPI_UartPrintf(buf);
							if (nVaTr.programselection== GREENT[i].progque && GREENT[i].progque>0)
								{
									progindex = i+1;
									if(progindex<5)
									{
									for (i = progindex; i < 5; i++)
							        {
									if(GREENT[i].progque>0 )
									{
									nVaTr.programselection = GREENT[i].progque;
									proqueflagcounter=0;
									sprintf(buf,"nVaTr.programselection = %d",nVaTr.programselection);
									sAPI_UartPrintf(buf);
									break;
									}
									//progindex=i;
									}
									}
									//progindex=progindex+1;
									sprintf(buf,"nVaTr.programselection = %d prvprogsel=%d***********************",nVaTr.programselection,prvprogsel);
									sAPI_UartPrintf(buf);
									if(prvprogsel==nVaTr.programselection)
									{
									for (i = 1; i < 5; i++)
							        {
									if(GREENT[i].progque>0 )
									{
									nVaTr.programselection = GREENT[i].progque;
									proqueflagcounter=0;
									sprintf(buf,"nVaTr.programselection = %d",nVaTr.programselection);
									sAPI_UartPrintf(buf);
									break;
									}
									}
									}

									proqueflagcounter=0;
									sprintf(buf,"nVaTr.programselection = %d",nVaTr.programselection);
									sAPI_UartPrintf(buf);
									break;
							}
							 }

                            if(prvprogsel!= nVaTr.programselection)
                            {
						    WriteDprevSettings();
		                    sprintf(buf,"\n pos=%d\n\r",nVaTr.programselection);
		                    sAPI_UartPrintf(buf);
							ReadDprevSettings();
		                    sprintf(buf,"\n pos=%d\n\r",nVaTr.programselection);
		                    sAPI_UartPrintf(buf);
							STATE_SENDSMS=STATE_PROGRAMSEL_SMS;
		                    SendSmsToAll = 1;
		                    s_nMSettings.m_Enter=1;
							if(nVaTr.querestartonof==1)
							proqueflag=1;
						    else
							proqueflag=2;
							}
							else
							proqueflag=2;
							 }
			}
			
						/*	else if(progqueonof == 1&& proqueflag==0 && nDripSettings.driponof==1 && nVaTr.cycrestartonof==1 && MakeRealyOn == 1 && nVaTr.cyclecompleted==1)
							{
						     char progindex;
							 char prvprogsel;

							 prvprogsel=nVaTr.programselection;
                             for (i = 1; i < 5; i++)
							 {
								//sprintf(buf,"recived no %s , Reg number %s,%d",p+j,SmsNumber[i],j);
							sAPI_UartPrintf(buf);
							if (nVaTr.programselection== GREENT[i].progque && GREENT[i].progque>0)
							{
									progindex = i+1;
									if(progindex<5)
									{
									for (i = progindex; i < 5; i++)
							        {
									if(GREENT[i].progque>0 )
									{
									nVaTr.programselection = GREENT[i].progque;
									break;
									}
									}
									}
									break;
							}
							}
                            if(prvprogsel!= nVaTr.programselection)
                            {
						    WriteDprevSettings();
		                    sprintf(buf,"\n pos=%d\n\r",nVaTr.programselection);
		                    sAPI_UartPrintf(buf);
							ReadDprevSettings();
		                    sprintf(buf,"\n pos=%d\n\r",nVaTr.programselection);
		                    sAPI_UartPrintf(buf);
							STATE_SENDSMS=STATE_PROGRAMSEL_SMS;
		                    SendSmsToAll = 1;
		                    s_nMSettings.m_Enter=1;
							proqueflag=2;
							}
							else
							proqueflag=2;

							}*/
                            if(proqueflag==1 && s_nMSettings.m_Enter>70 && nVaTr.cyclecompleted==1)
							{
                                   if(PowerCurrentCondition == 0)
									nSTATE_MOTOR = STATE_MOTOR_TRIP_RESTART;
									else
									nSTATE_STATUS_SMS =STATUS_NO_ELECTRICICY;
									sprintf(buf,"\n\rReStart Time Completed\n\r");
									sAPI_UartPrintf(buf);
									nVaTr.cyclecompleted=0;
									SendSmsToAll = 1;
								    nDripSettings.startcontrol=1;
									dripcyclecount=0;
									dripcycledate=0;
									proqueflag=2;
									nDripSettings.dripgapdaycount=0;
									//WriteDprevSettings();
									//ReadDprevSettings();
									//SendSmsToAll = 1;
									
									WriteonofFile();

									PreviousTrip = NO_TRIP_FLAG;
									nTimerSettings.Driprestartpoweronof=0;
							}
		if(nDripSettings.driponof==1 && nVaTr.cycrestartonof==0 && MakeRealyOn == 1)
			{

							nMoTr.RTCON[6] = (zonecom.daycountmin*3600)+(zonecom.daycountsec*60);
                            if(nMoTr.RTCON[6]>86400)
                            nMoTr.RTCON[6] = 86400;
							sAPI_GetRealTimeClock(&datetime);
							nVaTr.CurrentSec = (datetime.tm_hour*3600)+(datetime.tm_min*60)+datetime.tm_sec;
							//sprintf(buf,"\n\rnVaTr.CurrentSec = %d  nMoTr.RTCON4 = %d\n\r",nVaTr.CurrentSec,nMoTr.RTCON4);
                            sAPI_UartPrintf(buf);
							if(nVaTr.CurrentSec<(nMoTr.RTCON[6]+10) && nVaTr.CurrentSec>nMoTr.RTCON[6] && nMoTr.RTCON[6] !=0&&dripcycledatecheck==0 )
							{
							dripcycledate++;
							nDripSettings.dripgapdaycount++;
							//WriteDprevSettings();
							//ReadDprevSettings();
							STATE_SENDSMS=	STATE_SKIPDAYCOUNT_SMS;
							SendSmsToAll = 1;

							//sprintf(buf,"\n\rdripcycledate = %d  \n\r",dripcycledate);
							// sAPI_UartPrintf(buf);
							if(dripcycledate>999)
							{
							dripcycledate=0;
							dripcyclecount=0;
							}
							dripcycledatecheck=1;
							}
			}

							if(nVaTr.CurrentSec>(nMoTr.RTCON[6]+10) && nVaTr.CurrentSec<(nMoTr.RTCON[6]+15)&& dripcycledatecheck==1)
							dripcycledatecheck=0;

			if(nTimerSettings.Driprestartpoweronof==1 && MakeRealyOn == 1)
				{
							nMoTr.RTCON[6] = (zonecom.daycountmin*3600)+(zonecom.daycountsec*60);
                            if(nMoTr.RTCON[6]>86400)
                            nMoTr.RTCON[6] = 86400;
							sAPI_GetRealTimeClock(&datetime);
							nVaTr.CurrentSec = (datetime.tm_hour*3600)+(datetime.tm_min*60)+datetime.tm_sec;

                            if(nVaTr.CurrentSec<(nMoTr.RTCON[6]+10) && nVaTr.CurrentSec>(nMoTr.RTCON[6]+1) && nMoTr.RTCON[6] !=0)
                            {
							     if(nDripSettings.dripgapdayonof==0||(nDripSettings.dripgapdaycount>=nDripSettings.dripgapdays))
								   {
								    if(PowerCurrentCondition == 0)
									nSTATE_MOTOR = STATE_MOTOR_TRIP_RESTART;
									else
									nSTATE_STATUS_SMS =STATUS_NO_ELECTRICICY;
									sprintf(buf,"\n\rReStart Time Completed\n\r");
									sAPI_UartPrintf(buf);
									nVaTr.cyclecompleted=0;
									WriteonofFile();

									SendSmsToAll = 1;
									if(nVaTr.cyclecompleted==1)
								    nDripSettings.startcontrol=1;
									nDripSettings.stp=nDripSettings.startfrom;
									nDripSettings.calc=nDripSettings.stp;
									nDripSettings.startcontrol=0;
									nDripSettings.changeinstp1=0;
									nDripSettings.changeinstp=0;

									nDripSettings.changeincalc=0;
									nVaTr.Currentvalve = nDripSettings.stp;
									dripcyclecount=0;
									dripcycledate=0;

									nDripSettings.dripgapdaycount=0;
								//	WriteDprevSettings();
								//	ReadDprevSettings();
								//	SendSmsToAll = 1;
								
									PreviousTrip = NO_TRIP_FLAG;
									nTimerSettings.Driprestartpoweronof=0;
									}
							}

				}
 
				//dg_nsdkk			//if(PowerCurrentCondition==0 && PowerOnDelayComPleted==1 && nDripSettings.driponof==1)
 
			//if(nDripSettings.driponof==1)
		//		#if 0
				if(PowerCurrentCondition== 0 && MakeRealyOn == 1 && nDripSettings.driponof==1 && PowerOnDelayComPleted==1 )
					{
						sprintf(buf,"STATE_MOTOR_ON\n\r");								
								sAPI_UartPrintf(buf);
								//if(nDripSettings.driponof==1)
								if(nSTATE_MOTOR ==STATE_MOTOR_ON ||nSTATE_MOTOR ==STATE_MOTOR_TRIP_DRYRUN_SCAN || nSTATE_MOTOR ==STATE_MOTOR_TRIP_OVERLOAD_SCAN ||nSTATE_MOTOR ==STATE_MOTOR_TRIP_LOWPRESS_SCAN ||nSTATE_MOTOR ==STATE_MOTOR_TRIP_HIGHPRESS_SCAN)
								{
									sprintf(buf,"\n\r STATE_MOTOR_ON ok\n\r");
									sAPI_UartPrintf(buf);
									sAPI_GetRealTimeClock(&datetime);
									nVaTr.CurrentSec = (datetime.tm_hour*3600)+(datetime.tm_min*60)+datetime.tm_sec;
									if(DripDate!=datetime.tm_mday)
									{
										//	nVaTr.STEP1 = nVaTr.STEP1-86400;
										//	nVaTr.STEP2 = nVaTr.STEP2-86400;
										DripDate=datetime.tm_mday;
										nDripSettings.checkagain=1;
									}
									if((CheckLowerTank() == 0 || nMSettings.TankOnOff == 0) && CheckManualSwitch() == 0 && nTimerSettings.ManualOnOff==0  )
									{
										//	RunModule();
										nDripSettings.calconof=0;
										again11send=0;
									}
								    else
									{
										valvetimercounter=0;
										nDripSettings.calc=nDripSettings.stp;
									//	skipfbk=0;
										pwrdeltimercounter = (zonecom.fbkdelmin*60)+zonecom.fbkdelsec;
									//	pwrdeltimercounter=120;
										pwrdeltimercounter=pwrdeltimercounter*2;
									//	nDripSettings.calconof=1;
										nDripSettings.calc=nDripSettings.stp;
										fertgetstart1timer=0;
										fertgetstart2timer=0;
										fertgetstart3timer=0;
										firstvalestsmsonof=1;
										firstvalesmsonof=1;
										nVaTr.ActRefreshOnDelay =0;
										again1send=1;
										again2send=1;
										again3send=1;
										again4send=1;
										again5send=1;
									 //	again7send=1;
										again8send=1;
										again10send=0;
										nVaTr.sendcheckagain=0;

									if(again12send++>7 && nDripSettings.calconof==0 )
									{
									if(again12send++>10)
									{again12send=0;nDripSettings.calconof=1;}
									sprintf(textBuf,"$D,D,1,1,1,0,0,0,0\n\r ");
								    sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
								    }

									switchModule();
									}
								//	nDripSettings.calconof=0;
								//	again11send=0;
								}
								else
								{
								//	sprintf(buf,"\n\r default case 4 \n\r");
								// sAPI_UartPrintf(buf);
								//	OUT_DEBUG3(textBuf,"$D,4,1\n\r");

									if(again12send++>20)
									{
									again12send=0;
									sprintf(textBuf,"$D,D,1,1,1,0,0,0,0\n\r ");
			                        sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
									}
									if(nDripSettings.calconof==0 )
									{
									sprintf(textBuf,"$D,D,1,0,1,0,0,0,0\n\r ");
			                        sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
									   if(again11send++>3)
									   {
									//	skipfbk=0;
										valvetimercounter=0;
										pwrdeltimercounter = (zonecom.fbkdelmin*60)+zonecom.fbkdelsec;
									//	pwrdeltimercounter=120;
										pwrdeltimercounter=pwrdeltimercounter*2;

										nDripSettings.calconof=1;
										nDripSettings.calc=nDripSettings.stp;
										fertgetstart1timer=0;
										fertgetstart2timer=0;
										fertgetstart3timer=0;
										firstvalestsmsonof=1;
										firstvalesmsonof=1;
										nVaTr.ActRefreshOnDelay =0;
										again1send=1;
										again2send=1;
										again3send=1;
										again4send=1;
										again5send=1;
									//	again7send=1;
										again8send=1;
										again10send=0;
										nVaTr.sendcheckagain=0;
										sprintf(textBuf,"$D,D,1,0,1,0,0,0,0\n\r ");
			                            sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
										sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));

										nVaTr.sendremaintime=0;
										}

										//gotnext=0;
									}
								}
							}
						
							else
							{
							//	sprintf(buf,"\n\r default case 4 \n\r");
							// sAPI_UartPrintf(buf);
							//	OUT_DEBUG3(textBuf,"$D,4,1\n\r");
								if(again12send++>20)
								{
									again12send=0;
									if(nDripSettings.driponof==1)
									sprintf(textBuf,"$D,D,1,1,1,0,0,0,0\n\r ");
								    else
									sprintf(textBuf,"$D,D,1,0,1,0,0,0,0\n\r ");
			                        sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
							    }
								if(nDripSettings.calconof==0 )
								{
								      sprintf(textBuf,"$D,D,1,0,1,0,0,0,0\n\r ");
			                          sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
										   if(again11send++>3)
									   {
										//nVaTr.TSTEP2 = (zonecom.fbkdelmin*60)+zonecom.fbkdelsec;
										//pwrdeltimercounter=nVaTr.TSTEP2*2;
										pwrdeltimercounter = (zonecom.fbkdelmin*60)+zonecom.fbkdelsec;
										//pwrdeltimercounter=120;
										pwrdeltimercounter=pwrdeltimercounter*2;

										nDripSettings.calconof=1;
										nDripSettings.calc=nDripSettings.stp;
										fertgetstart1timer=0;
										fertgetstart2timer=0;
										fertgetstart3timer=0;
										firstvalestsmsonof=1;
										firstvalesmsonof=1;
										nVaTr.ActRefreshOnDelay =0;
										again1send=1;
										again2send=1;
										again3send=1;
										again4send=1;
										again5send=1;
									//	again7send=1;
										again8send=1;
										again10send=0;
										nVaTr.sendcheckagain=0;
										if(refresh_filter_on_flag==1)
										{
										fertgetstart6timer=0;
								//		again9send=1;
										}
										sprintf(textBuf,"$D,D,1,0,1,0,0,0,0\n\r ");
			                            sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
										sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
	                                    nVaTr.sendremaintime=0;
										valvetimercounter=0;
										}

								}
							}
						/*if(GiveCallToNumber != 0 ||
								DelSMS != 0 ||
								SendSMS != 0 ||
								ReadSMS != 0 ||
								callreceived != 0) */
           //      #endif
								if(nDripSettings.startcontrolt==1)
							{
								ReadDprevSettings();
								sprintf(buf,"tstartra =%ld ",nDripSettings.tstartfrom );
								sAPI_UartPrintf(buf);
								sprintf(buf,"startra =%ld ",nDripSettings.startfrom );
								sAPI_UartPrintf(buf);
								nDripSettings.prvstartfrom=nDripSettings.startfrom;
								sprintf(buf,"prvstartra =%ld ",nDripSettings.prvstartfrom );
								sAPI_UartPrintf(buf);
								nDripSettings.startfrom=nDripSettings.tstartfrom;
								sprintf(buf,"tstartad =%ld ",nDripSettings.startfrom );
								sAPI_UartPrintf(buf);
								decide_sub_next(nDripSettings.startfrom);
								sprintf(buf,"startwb =%ld ",nDripSettings.startfrom );
								sAPI_UartPrintf(buf);
								if(nDripSettings.decidefirst>nDripSettings.startfrom)
								nDripSettings.startfrom=7;
								WriteDprevSettings();
								ReadDprevSettings();
								sprintf(buf,"startra =%ld ",nDripSettings.startfrom );
								sAPI_UartPrintf(buf);
								STATE_SENDSMS=	STATE_STARTFROM_SMS;
								SendSmsToAll = 1;
								nDripSettings.startcontrolt=0;
								nVaTr.REMTIM=0;
							}
#endif // dg_nsdkk
			if (PlaySound == 1)
			{
				sprintf(buf, "HowManySoundToPlay%02d", HowManySoundToPlay);
				sAPI_UartPrintf(buf);
				sprintf(buf, "HowManySoundPlayed%02d", HowManySoundPlayed);
				sAPI_UartPrintf(buf);
				// if(HowManySoundToPlay != HowManySoundPlayed)
				if (HowManySoundToPlay > HowManySoundPlayed)
				{
					PlaySoundVer++;
					sAPI_UartPrintf("PlaySoundVer%02d", PlaySoundVer);

					if (PlaySoundVer == 1)
					{

						sAPI_UartPrintf(buffer);
					}
					else if (PlaySoundVer == 2)
					{
						// if(sound_flag_ok==0)
						if (austat == 0)
						{
							//		PlayAudio(HowManySound[HowManySoundPlayed]); //dg_nsdk
							HowManySoundPlayed++;
							PlaySoundVer = 0;
						}
						else
							PlaySoundVer = 1;
					}
					else if (PlaySoundVer == 3)
					{

						sAPI_UartPrintf(buffer);
					}
					else if (PlaySoundVer == 4)
					{

						sAPI_UartPrintf(buffer);
					}
					else if (PlaySoundVer >= 3)
					{
						HowManySoundPlayed++;
						PlaySoundVer = 0;
						sprintf(buf, "HowManySoundPlayed:%02d", HowManySoundPlayed);
						sAPI_UartPrintf(buf);
					}
				}
				else
				{
					HowManySoundToPlay = HowManySoundPlayed = 0;
					PlaySound = 0;
					sAPI_UartPrintf("HowManySoundPlayed fail");
				}
			}
			if (limitsmscount > limitsmsset && limitsmsonof == 1)
			{
				limitsmsflag = 1;
			}
			else
				limitsmsflag = 0;

			sAPI_GetRealTimeClock(&datetime);
			/* if((onehour_time!=datetime.tm_hour) && s_nMSettings.m_Enter > 70)
			{

				onehour_time=datetime.tm_hour;
				onehour_send_flag=1;
				//sgetflag_2=1;
				sAPI_UartPrintf("\n\r entry to onehour_time =1 line %d ",__LINE__);
			} */
			if (datetime.tm_sec != OnehrPrvSec)
			{

				OnehrPrvSec = datetime.tm_sec;
				Onehr_sec_timer++;

				/* if((Onehr_sec_timer>=3600) && s_nMSettings.m_Enter > 70)
				{
					Onehr_sec_timer=0;
					onehour_send_flag=1;
					//sgetflag_2=1;
					sAPI_UartPrintf("\n\r entry to onehour_time =1 line %d ",__LINE__);
				} */
			}

			if (PowerIsThere == 1 && AllSmsSendDone == 1 && CallModeOn == 0 && nMSettings.staticSMSOnOff == 1)
			{
				// checkpower=Check2Phase();
				if (checkpower == 2)
				{
					if (nTimerSettings.AutoStIIOnOff == 0 || Mobile2Phs3Phase == 3) // dg_nsdk
					{
						StatusSMSDelay1 = 0;
						SMS30MinStatus = 0;
					}
				}
				StatusSMSDelay1++;
				if (StatusSMSDelay1 >= 200)
				{
					StatusSMSDelay1 = 0;
					StatusSMSDelay2++;
					if (StatusSMSDelay2 >= 180)
					{
						SMS30MinStatus = 1;
						sprintf(buf, "\n\rNow Sending 30 tm_min Status SMS\n\r");
						sAPI_UartPrintf(buf);
						SMS30MinStatusNumber = 0;
						SMS30MinStatusDelay = 0;
						StatusSMSDelay2 = 0;
					}
				}
			}
			else
			{
				StatusSMSDelay1 = 0;
				StatusSMSDelay2 = 0;
				SMS30MinStatus = 0;
				SMS30MinStatusDelay = 0;
			}
			if (SMS30MinStatus == 1)
			{
				if (SMS30MinStatusDelay++ >= 15)
				{
					SMS30MinStatusDelay = 0;
					sprintf(buf, "\n\r*******30 tm_min Status SMS SMS30MinStatusNumber = %d********\n\r", SMS30MinStatusNumber);
					sAPI_UartPrintf(buf);
					sprintf((char *)Status30MinPhNo, "0%s", SmsNumber[0]);
					// StopTimer(&timer);
					//	send_test_smsStatus(Status30MinPhNo); //dg_nsdk
					// timer.timeoutPeriod = 300;
					// timer.timerId = StartTimer(&timer);
					// SMS30MinStatusNumber++;
					// if(SMS30MinStatusNumber>=HowManyNumberFound)
					//{
					SMS30MinStatusNumber = 0;
					SMS30MinStatus = 0;
					sprintf(buf, "\n\rNow 30 tm_min Status SMS Sendng Completed\n\r");
					sAPI_UartPrintf(buf);
					//}
				}
			}
			if (StaticSMSSend == 1)
			{
				if (StaticSMSDelay++ >= 15)
				{
					StaticSMSDelay = 0;
					sprintf(buf, "\n\r*******Static SMS SMS30MinStatusNumber = %d********\n\r", SMS30MinStatusNumber);
					sAPI_UartPrintf(buf);
					if (strstr(SmsNumber[SMS30MinStatusNumber], "*") == 0)
						sprintf((char *)Status30MinPhNo, "0%s", SmsNumber[SMS30MinStatusNumber]);
					//	SendSMSStr(StaticSmsString,Status30MinPhNo); //dg_nsdk
					SMS30MinStatusNumber++;
					if (SMS30MinStatusNumber >= HowManyNumberFound)
					{
						SMS30MinStatusNumber = 0;
						SMS30MinStatus = 0;
						StaticSMSSend = 0;
						sprintf(buf, "\n\rNow 30 tm_min Status SMS Sendng Completed\n\r");
						sAPI_UartPrintf(buf);
					}
				}
			}
#if 0
						if(nMSettings.VBFOnOff == 1&& Callreceiv == 0 )
						{
							IamCalling = 1;
							if(PowerCondCall == 1&& nMSettings.pwrvfbOnOf==1 )
							{
								CallingBecError = 1;
								PowerCondCall = 0;
								PowerCondVoice = 1;
							}
							else if(nSTATE_MOTOR_SMS != STATE_NO_MOTOR_SMS && GCallingBecError != nSTATE_MOTOR_SMS)
							{
								GCallingBecError = nSTATE_MOTOR_SMS;
								switch(nSTATE_MOTOR_SMS)
								{
									case STATE_MOTORON_SMS:
										break;
									case STATE_MOTOROF_SMS:
										break;
									case STATE_MOTOR_STARTER_TRIP_SMS:
										CallingBecError = 1;
										break;
									case STATE_MOTOR_UPPERTANK_TRIP_SMS:
										break;
									case STATE_MOTOR_LOWERTANK_TRIP_SMS:
                                       CallingBecError = 1;
										break;
									case STATE_MOTOR_DRYRUN_TRIP_SMS:
										CallingBecError = 1;
										break;
									case STATE_MOTOR_CYCLIC_TRIP_SMS:
										//CallingBecError = 1;
										break;
									case STATE_MOTOR_MAX_TRIP_SMS:
										//CallingBecError = 1;
										break;
									case STATE_MOTOR_RTCOF1_TRIP_SMS:
										//CallingBecError = 1;
										break;
									case STATE_MOTOR_RTCOF2_TRIP_SMS:
										//CallingBecError = 1;
										break;
									case STATE_MOTOR_RTCOF3_TRIP_SMS:
										//CallingBecError = 1;
										break;
									case STATE_MOTOR_TRIP_CURRENTSPP_SMS:
										CallingBecError = 1;
										break;
									case STATE_MOTOR_TRIP_SPP_SMS:
										CallingBecError = 1;
										break;
									case STATE_MOTOR_RTCOF4_TRIP_SMS:
										//CallingBecError = 1;
										break;
									case STATE_MOTOR_TRIP_LOWVOLTAGE_SMS:
										CallingBecError = 1;
										break;
									case STATE_MOTOR_TRIP_HIGHVOLTAGE_SMS:
										CallingBecError = 1;
										break;
									case STATE_MOTOR_TRIP_OVERLOAD_SMS:
										CallingBecError = 1;
										break;
									case STATE_MOTOR_TRIP_HIGHPRESS_SMS:
										CallingBecError = 1;
										break;
									case STATE_MOTOR_TRIP_LOWPRESS_SMS:
										CallingBecError = 1;
										break;
                                   default :
									break;

								}
							}
						}
#endif
			/*if(getdataflag==1 && ModemIsReady == 1)
			{
			sprintf(textBuf,"$G,M,%ld,1,1,%ld,%ld,%ld\n\r ",zonecom.getdtaid,zoneid[zonecom.getdtaid].ida1,zoneid[zonecom.getdtaid].ida2,zoneid[zonecom.getdtaid].ida3 );
			sAPI_UartWrite(SC_UART,textBuf, strlen(textBuf));
				getdataflag=0;
			}*/
			sprintf(buf, "\n\rTest:%d,%d,%d\n\r", SendSmsToAll, ModemIsReady, nMSettings.SMSOnOff);
			sAPI_UartPrintf(buf);
			if (nMSettings.ndebugonof)
			{
				sAPI_UartPrintf("\n\rftp://%s:%s@%s/customer_app.bin\n\r", DeviceConfig.ftpUserName, DeviceConfig.ftpPassword, DeviceConfig.FtpServerIP);
				sAPI_UartPrintf("\n\r[tcp//%s],UserName:[%s],PassWord:[%s]\n\r", DeviceConfig.MqttServerIP, DeviceConfig.MqttUserName, DeviceConfig.MqttPassword);
			}
			nMSettings.SMSOnOff=1;  //dg_nsdk
			sprintf(buf, "\n\rSendSmsToAll: %d, ModemIsReady: %d,nMSettings.SMSOnOff: %d\n\r", SendSmsToAll, ModemIsReady, nMSettings.SMSOnOff);
			sAPI_UartPrintf(buf); // subash commented

			if (SendSmsToAll == 1 && ModemIsReady == 1 && nMSettings.SMSOnOff == 1)
			{
				long TpHr, TpMin, TpSec, value;
				unsigned char removeniagara = 0;
				if (nMSettings.gethidesmsonoff == 1)
				{
					unsigned char Tp10;
					sprintf(buffer, "            ");
					strcpy(StoredPhoneNumber[10], buffer);

					for (Tp = 0; Tp < HowManyNumberFound; Tp++)
					{
						sprintf(buf, "\n\rStoredPhoneNumber: %s WhoMadeRelayOn: %s\n\r", StoredPhoneNumber[Tp], WhoMadeRelayOn);
						sAPI_UartPrintf(buf);
						if ((strstr(StoredPhoneNumber[Tp], WhoMadeRelayOn) != 0))
						{
							if (Tp == 0)
							{
								sprintf(buffer, "SMSM");
								strcpy(StoredPhoneNumber[24], buffer);
							}
							else if (Tp == 1)
							{
								sprintf(buffer, "REG 1");
								strcpy(StoredPhoneNumber[24], buffer);
							}
							else if (Tp == 2)
							{
								sprintf(buffer, "REG 2");
								strcpy(StoredPhoneNumber[24], buffer);
							}
							else if (Tp == 3)
							{
								sprintf(buffer, "REG 3");
								strcpy(StoredPhoneNumber[24], buffer);
							}
						}
					}

					if (strstr(StoredPhoneNumber[11], WhoMadeRelayOn) != 0)
					{
						sprintf(buffer, "SERVICE");
						strcpy(StoredPhoneNumber[24], buffer);
					}
				}
				sAPI_GetRealTimeClock(&datetime);
				sprintf(buf, "\n\rNumberOfSmsNeedToSend= %d\n\r", NumberOfSmsNeedToSend);
				sAPI_UartPrintf(buf);
				if(NumberOfSmsNeedToSend<20)
								{
								    switchonofflag=0;
									FirstTimeSMS = 0;
									sprintf(buf,"\n\rFill up now \n\r");
									sAPI_UartPrintf(buf);
									limitsmscountsmsm=limitsmscount;
									sprintf(buf,"\n\rHowManyNumberFound:%d\n\r",HowManyNumberFound);
									sAPI_UartPrintf(buf);
									HowManyNumberFound=2;//PRAVEEN
									sAPI_GetRealTimeClock(&datetime);
									l_currentSec = (datetime.tm_hour*3600)+(datetime.tm_min*60)+datetime.tm_sec; 
									if(strstr(Buffer4,"MOTOR1") == 0)
										sprintf(Buffer4,"POWER ON MOTOR1 ON,%02d:%02d:%02d",datetime.tm_hour,datetime.tm_min,datetime.tm_sec);
									//readonoffbuff();
									sprintf(buf,"\n\r nSTATE_MOTOR1_SMS:%d,nSTATE_MOTOR2_SMS:%d,nSTATE_MOTOR3_SMS:%d\n\r",nSTATE_MOTOR1_SMS,nSTATE_MOTOR2_SMS,nSTATE_MOTOR3_SMS);
									sAPI_UartPrintf(buf);
									
									sprintf(buf,"\n\r nSTATE_MOTOR1_ON_SMS:%d,nSTATE_MOTOR2_ON_SMS:%d,nSTATE_MOTOR3_ON_SMS:%d\n\r",nSTATE_MOTOR1_ON_SMS,nSTATE_MOTOR2_ON_SMS,nSTATE_MOTOR3_ON_SMS);
									sAPI_UartPrintf(buf);
									for(Tpsms=0;Tpsms<=HowManyNumberFound-1;Tpsms++)
									{
                                        
										if(nSTATE_MOTOR1_SMS != STATE_NO_MOTOR1_SMS)
											{
												//sprintf(Buffer1,"%s",Buffer4);
												sprintf(buf,"Motor1 off entry\n\r");
												sAPI_UartPrintf(buf);											
												//case STATE_MOTOR1OF_SMS:
												if(nSTATE_MOTOR1_SMS == STATE_MOTOR1OF_SMS)
													{
														
														sprintf(buf,"STATE_MOTOR1OF_SMS case");
														sAPI_UartPrintf(buf);
														if(PowerCurrentCondition == 0)
														{
															if(WorkingOn3Phas == 1)
																{
																 if(Appmodeon==1)
																	sprintf(Buffer1," %02d020\n",(datetime.tm_sec+2));
																else
																  sprintf(Buffer1,"%s,AUTO OFF \nPOWER ON\nMOTOR1 OFF\n",Buffer4);
																//  switchonofflag=1;
																}
																//sprintf(Buffer1,"MOBILE OFF BY %s\nPOWER ON\nMOTOR OFF\n",WhoMadeRelayOn);
																else
																{
																	if(Appmodeon==1)
																	sprintf(Buffer1," %02d021\n",(datetime.tm_sec+2));
																	else
																  sprintf(Buffer1,"%s,AUTO OFF \nPOWER ON\nMOTOR1 OFF\n",Buffer4);
																//  switchonofflag=1;

																}


														}
														else
														{
															   if(Appmodeon==1)
																sprintf(Buffer1," %02d079\n",(datetime.tm_sec+2));
																else
																sprintf(Buffer1,"%s,AUTO OFF \nPOWER OFF\nMOTOR1 OFF\n",Buffer4);
															//	switchonofflag=1;

															//sprintf(Buffer1,"MOBILE OFF BY %s\nPOWER OFF\nMOTOR OFF\n",WhoMadeRelayOn);


														}
													}
													//break;
												//case STATE_MOTOR1_STARTER_TRIP_SMS:
												else if(nSTATE_MOTOR1_SMS == STATE_MOTOR1_STARTER_TRIP_SMS)
												{
													//sprintf(buf,"nSTATE_MOTOR1_SMS :%d %d\n\r",nSTATE_MOTOR1_SMS,STATE_MOTOR1_STARTER_TRIP_SMS);
												//sAPI_UartPrintf(buf);
												
													sprintf(Buffer1,"%s,MOTOR1 OFF BECAUSE OF STARTER TRIP:%d",Buffer4,(OverAllStarterTrip1+1));

													if(WorkingOn3Phas == 1)
													{
													//if(OverAllStarterTrip>3) OverAllStarterTrip=3;
														if(Appmodeon==1)
															sprintf(Buffer1," %02d101\nTRIP-%d\n",(datetime.tm_sec+2),(OverAllStarterTrip1+1));
														else
															sprintf(Buffer1,"%s WITH 3 PHASE",Buffer1);
													}
													else
													{
														if(Appmodeon==1)
															sprintf(Buffer1," %02d102\nTRIP-%d",(datetime.tm_sec+2),(OverAllStarterTrip1+1));
														else
															sprintf(Buffer1,"%s WITH 2 PHASE",Buffer1);
													}
												}
												//break;
												/*case STATE_MOTOR1_UPPERTANK_TRIP_SMS:
                                                    {
                                                    if(Appmodeon==1)
													sprintf(Buffer1,"%02d22\n",(datetime.tm_sec+2));
													else
													sprintf(Buffer1,"MOTOR1 OFF BECAUSE OF PRESSURE SWITCH OPEN");
													}
													break;*/
													//case STATE_MOTOR1_UPPERTANK_TRIP_SMS:
													else if(nSTATE_MOTOR1_SMS == STATE_MOTOR1_UPPERTANK_TRIP_SMS)
													{
														if(Appmodeon==1)
															sprintf(Buffer1," %02d022\n",(datetime.tm_sec+2));   //Previously %02d103
														else
															sprintf(Buffer1,"%s,MOTOR1 OFF BECAUSE OF UPPER TANK FULL",Buffer4);    //dg_changed from MOTOR OFF BECAUSE OF PRESSURE SWITCH OPEN
													}
													//break;
													else if(nSTATE_MOTOR1_SMS == STATE_MOTOR1_LEVELSCAN_UPPERTANK_TRIP_SMS)
													{
														if(Appmodeon==1)
															sprintf(Buffer1," %02d022\n",(datetime.tm_sec+2));   //Previously %02d103
														else
															sprintf(Buffer1,"%s,MOTOR1 OFF BECAUSE OF UPPER TANK FULL BY %03d%%",Buffer4,level_percent);//dg_changed from MOTOR OFF BECAUSE OF PRESSURE SWITCH OPEN
													}
													//case STATE_MOTOR1_LOWERTANK_TRIP_SMS:
												else if(nSTATE_MOTOR1_SMS == STATE_MOTOR1_LOWERTANK_TRIP_SMS)
													{
														if(Appmodeon==1)
															sprintf(Buffer1," %02d023\n",(datetime.tm_sec+2));
														else
															sprintf(Buffer1,"%s,MOTOR1 OFF BECAUSE OF SUMP TANK EMPTY",Buffer4);
													}
													//break;
													else if(nSTATE_MOTOR1_SMS == STATE_MOTOR1_LEVELSCAN_LOWERTANK_TRIP_SMS)
													{
														if(Appmodeon==1)
															sprintf(Buffer1," %02d023\n",(datetime.tm_sec+2));
														else
															sprintf(Buffer1,"%s,MOTOR1 OFF BECAUSE OF SUMP TANK EMPTY BY %03d%%",Buffer4,level_percent);
													}
												 // case STATE_MOTOR1_ON_SWITCH_TRIP_SMS:
												 else if(nSTATE_MOTOR1_SMS == STATE_MOTOR1_ON_SWITCH_TRIP_SMS)
													 {
														if(Appmodeon==1)
															 sprintf(Buffer1," %02d024\n",(datetime.tm_sec+2));
														 else
															 sprintf(Buffer1,"%s,MOTOR1 OFF BECAUSE MOBILE AUTO SWITCH",Buffer4);
													 }
													// break; 
													
												//case STATE_MOTOR1_DRYRUN_TRIP_SMS:
												else if(nSTATE_MOTOR1_SMS == STATE_MOTOR1_DRYRUN_TRIP_SMS)
													{
														sprintf(Buffer1,"%s,MOTOR1 OFF BECAUSE DRY RUN",Buffer4);
														if(WorkingOn3Phas == 1)
														{

															if(Appmodeon==1)
																sprintf(Buffer1," %02d025\n",(datetime.tm_sec+2));
															else
															{
															//FloatroString1Dig(TpS1tr,s_nTimerSettings.m_DrAmpsII[l_pumpno]I);
																sprintf(Buffer1,"%s WITH 3 PHASE",Buffer1);
															}
															sprintf(Buffer1,"%s_WhichPhase=%d_SET AMPS = %02.01f_TripCurrent=%02.01f\n",Buffer1,WhichPhaseHaveingProblem,s_nTimerSettings.m_DrAmpsIII[1],TripCurrent);
														}
														else
														{
															if(Appmodeon==1)
																sprintf(Buffer1," %02d026\n",(datetime.tm_sec+2));
															else
															{
															//FloatroString1Dig(TpS1tr,s_nTimerSettings.m_DrAmpsII[l_pumpno]);
																sprintf(Buffer1,"%s,WITH 2 PHASE",Buffer1);
															}
															sprintf(Buffer1,"%s_WhichPhase=%d_SET AMPS = %02.01f_TripCurrent=%02.01f\n",Buffer1,WhichPhaseHaveingProblem,s_nTimerSettings.m_DrAmpsII[1],TripCurrent);
														}
														if(WhichPhaseHaveingProblem == 1)
														{
														//FloatroString1Dig(TpS1tr,TripCurrent);
															sprintf(Buffer1,"%s_RC = %02.01f\n",Buffer1,TripCurrent);
														}
														else if(WhichPhaseHaveingProblem == 2)
														{
														//FloatroString1Dig(TpS1tr,TripCurrent);
															sprintf(Buffer1,"%s_YC = %02.01f\n",Buffer1,TripCurrent);
														}
														else if(WhichPhaseHaveingProblem == 3)

														{
														//FloatroString1Dig(TpS1tr,TripCurrent); //check in old
															sprintf(Buffer1,"%s_BC = %02.01f\n",Buffer1,TripCurrent);
														}

													}
													//break;
												//case STATE_MOTOR1_CYCLIC_TRIP_SMS:
												else if(nSTATE_MOTOR1_SMS == STATE_MOTOR1_CYCLIC_TRIP_SMS)
												{
													if(Appmodeon==1)
														sprintf(Buffer1," %02d027\n",(datetime.tm_sec+2));
													else												
														sprintf(Buffer1,"%s,MOTOR1 OFF BECAUSE CYCLIC OFF TIME",Buffer4);
												}
													//break;
												//case STATE_MOTOR1_MAX_TRIP_SMS:
												else if(nSTATE_MOTOR1_SMS == STATE_MOTOR1_MAX_TRIP_SMS)
												{
													if(Appmodeon==1)
														sprintf(Buffer1," %02d104\n",(datetime.tm_sec+2));
													else
														sprintf(Buffer1,"%s,MOTOR1 OFF BECAUSE MAX TIMER REACHED",Buffer4);
												}
													//break;
												/* case STATE_MOTOR1_VCYCCOMPLE_TRIP_SMS:
													//Ql_sprintf(Buffer1,"MOTOR1 OFF BECAUSE VALVE CYCLE COMPLETED AND VALVE CYCLE RESTART OFF");
													if(Appmodeon==1)
														sprintf(Buffer1," %02d115\n",(datetime.tm_sec+2));
													else
														sprintf(Buffer1,"MOTOR1 OFF BECAUSE ZONE CYCLE COMPLETED ");
													sprintf(Buffer1,"%sa_dripcyclecount[l_pumpno]=%03d\ndripdaycount=%03d\n ",Buffer1,a_dripcyclecount[l_pumpno],a_dripcycledate[l_pumpno]);

													break;  */

													 //case STATE_MOTOR_NOCOMMUNICATION_TRIP_SMS:
													else if(nSTATE_MOTOR1_SMS == STATE_MOTOR_NOCOMMUNICATION_TRIP_SMS)
													{
													
													sprintf(Buffer1," %02d124\n",(datetime.tm_sec+2));

													sprintf(Buffer1,"%s,%s,MOTOR OFF BECAUSE NO COMMUNICATION TO VALVES",Buffer1,Buffer4 );
													}

													//break; 


												/* case STATE_MOTOR_GRTCOF_TRIP_SMS:
													if(Appmodeon==1)
													sprintf(Buffer1," %02d029\n",(datetime.tm_sec+2));
													else
													sprintf(Buffer1,"MOTOR OFF BECAUSE GRTC PROGRAM TIMER REACHED");
													sprintf(Buffer1,"%sPROG-%d",Buffer1,RTCOFFLAG);

													break; */

												 //case STATE_MOTOR1_RTCOF1_TRIP_SMS:
												 else if(nSTATE_MOTOR1_SMS == STATE_MOTOR1_RTCOF1_TRIP_SMS)
												 {
													if(Appmodeon==1)
														sprintf(Buffer1," %02d030\nPROG-1\n",(datetime.tm_sec+2));
													else
														sprintf(Buffer1,"%s,MOTOR1 OFF BECAUSE RTC PROGRAM 1 OFF TIMER REACHED",Buffer4);
												 }
													//break;
												//case STATE_MOTOR1_RTCOF2_TRIP_SMS:
												else if(nSTATE_MOTOR1_SMS == STATE_MOTOR1_RTCOF2_TRIP_SMS)
												{
													if(Appmodeon==1)
														sprintf(Buffer1," %02d030\nPROG-2\n",(datetime.tm_sec+2));
													else
														sprintf(Buffer1,"%s,MOTOR1 OFF BECAUSE RTC PROGRAM 2 OFF TIMER REACHED",Buffer4);
												}
													//break;
												//case STATE_MOTOR1_RTCOF3_TRIP_SMS:
												else if(nSTATE_MOTOR1_SMS == STATE_MOTOR1_RTCOF3_TRIP_SMS)
												{
													if(Appmodeon==1)
														sprintf(Buffer1," %02d030\nPROG-3\n",(datetime.tm_sec+2));
													else
														sprintf(Buffer1,"%s,MOTOR1 OFF BECAUSE RTC PROGRAM 3 OFF TIMER REACHED",Buffer4);
												}
													//break;
												//case STATE_MOTOR1_RTCOF4_TRIP_SMS:
												else if(nSTATE_MOTOR1_SMS == STATE_MOTOR1_RTCOF4_TRIP_SMS)
												{
													if(Appmodeon==1)
														sprintf(Buffer1," %02d030\nPROG-4\n",(datetime.tm_sec+2));
													else
														sprintf(Buffer1,"%s,MOTOR1 OFF BECAUSE RTC PROGRAM 4 OFF TIMER REACHED",Buffer4);
												}
													//break;
												//case STATE_MOTOR1_RTCOF5_TRIP_SMS:
												else if(nSTATE_MOTOR1_SMS == STATE_MOTOR1_RTCOF5_TRIP_SMS)
												{
													if(Appmodeon==1)
														sprintf(Buffer1," %02d030\nPROG-5\n",(datetime.tm_sec+2));
													else
														sprintf(Buffer1,"%s,MOTOR1 OFF BECAUSE RTC PROGRAM 5 OFF TIMER REACHED",Buffer4);
												}
													//break; 
													
												//case STATE_MOTOR1_RTCOF6_TRIP_SMS:
												else if(nSTATE_MOTOR1_SMS == STATE_MOTOR1_RTCOF6_TRIP_SMS)
												{
													if(Appmodeon==1)
														sprintf(Buffer1," %02d030\nPROG-6\n",(datetime.tm_sec+2));
													else
														sprintf(Buffer1,"%s,MOTOR1 OFF BECAUSE RTC PROGRAM 6 OFF TIMER REACHED",Buffer4);
												}
													//break; 
												//case STATE_MOTOR1_TRIP_OVERLOAD_SMS:
												else if(nSTATE_MOTOR1_SMS == STATE_MOTOR1_TRIP_OVERLOAD_SMS)
													{
														sprintf(Buffer1,"%s,MOTOR1 OFF BECAUSE OVER LOAD",Buffer4);
														if(WorkingOn3Phas == 1)
														{

															if(Appmodeon==1)
																sprintf(Buffer1," %02d105\n",(datetime.tm_sec+2));
															else
															{
															//	FloatroString1Dig(TpS1tr,s_nTimerSettings.m_DrAmpsII[l_pumpno]);
																sprintf(Buffer1,"%s WITH 3 PHASE\n",Buffer1);
															}
															sprintf(Buffer1,"%s_WhichPhase=%d_SET AMPS = %02.01f_TripCurrent=%02.01f\n",Buffer1,WhichPhaseHaveingProblem,s_nTimerSettings.m_OlAmpsIII[1],TripCurrent);

														}
														else
														{

															if(Appmodeon==1)
																sprintf(Buffer1," %02d106\n",(datetime.tm_sec+2));
															else
															{
														//	FloatroString1Dig(TpS1tr,s_nTimerSettings.m_DrAmpsII[l_pumpno]);
																sprintf(Buffer1,"%s WITH 2 PHASE\n",Buffer1);
															}
															sprintf(Buffer1,"%s_WhichPhase=%d_SET AMPS = %02.01f \n",Buffer1,WhichPhaseHaveingProblem,s_nTimerSettings.m_OlAmpsII[1]);
														}
														if(WhichPhaseHaveingProblem == 1)
														{
													//	FloatroString1Dig(TpS1tr,TripCurrent);
															sprintf(Buffer1,"%s_RC = %0.1f\n",Buffer1,TripCurrent);
														}
														else if(WhichPhaseHaveingProblem == 2)
														{
													//	FloatroString1Dig(TpS1tr,TripCurrent);
															sprintf(Buffer1,"%s_YC = %0.1f\n",Buffer1,TripCurrent);
														}
														else if(WhichPhaseHaveingProblem == 3)
														{
													//	FloatroString1Dig(TpS1tr,TripCurrent);
															sprintf(Buffer1,"%s_BC = %0.1f\n",Buffer1,TripCurrent);
														}
													
													}
													//break;
													//case STATE_MOTOR1_TRIP_HIGHPRESS_SMS:  //oro_doubt
													else if(nSTATE_MOTOR1_SMS == STATE_MOTOR1_TRIP_HIGHPRESS_SMS)
													{
														sprintf(Buffer1,"%s,MOTOR1 OFF BECAUSE PRESSURE SENSOR HIGH",Buffer4);
														if(WorkingOn3Phas == 1)
														{

															if(Appmodeon==1)
																sprintf(Buffer1," %02d119\n",(datetime.tm_sec+2));
															else
															{
														//	FloatroString1Dig(TpS1tr,s_nTimerSettings.m_DrAmpsII[l_pumpno]);
																sprintf(Buffer1,"%s WITH 3 PHASE\n",Buffer1);
															}
															sprintf(Buffer1,"%s_SET HIPRESSURE = %0.1f\n",Buffer1,s_nTimerSettings.m_highpress[l_pumpno]);

														}
														else
														{

															if(Appmodeon==1)
																sprintf(Buffer1," %02d120\n",(datetime.tm_sec+2));
															else
															{
														//	FloatroString1Dig(TpS1tr,s_nTimerSettings.m_DrAmpsII[l_pumpno]);
																sprintf(Buffer1,"%s WITH 2 PHASE\n",Buffer1);
															}
															sprintf(Buffer1,"%s_SET HIPRESSURE = %0.1f\n",Buffer1,s_nTimerSettings.m_highpress);
														}
														sprintf(Buffer1,"%s_TRIP HIPRESSURE = %0.1f\n",Buffer1,Trippressure);

													}

													//break;
													//case STATE_MOTOR1_TRIP_LOWPRESS_SMS:
													else if(nSTATE_MOTOR1_SMS == STATE_MOTOR1_TRIP_LOWPRESS_SMS)
													{
														sprintf(Buffer1,"%s,MOTOR1 OFF BECAUSE PRESSURE SENSOR LOW",Buffer4);
														if(WorkingOn3Phas == 1)
														{

															if(Appmodeon==1)
																sprintf(Buffer1," %02d121\n",(datetime.tm_sec+2));
															else
															{
														//	FloatroString1Dig(TpS1tr,s_nTimerSettings.m_DrAmpsII[l_pumpno]);
																sprintf(Buffer1,"%s WITH 3 PHASE\n",Buffer1);
															}
															sprintf(Buffer1,"%s_SET LOWPRESSURE = %0.1f\n",Buffer1,s_nTimerSettings.m_lowpress[l_pumpno]);

														}
														else
														{

															if(Appmodeon==1)
																sprintf(Buffer1," %02d122\n",(datetime.tm_sec+2));
															else
															{
														//	FloatroString1Dig(TpS1tr,s_nTimerSettings.m_DrAmpsII[l_pumpno]);
																sprintf(Buffer1,"%s WITH 2 PHASE\n",Buffer1);
															}
															sprintf(Buffer1,"%s_SET LOWPRESSURE = %0.1f\n",Buffer1,s_nTimerSettings.m_lowpress[l_pumpno]);
														}
														sprintf(Buffer1,"%s_TRIP LOWPRESSURE = %0.1f\n",Buffer1,Trippressure);

													}

													//break;  
												//case STATE_MOTOR1_TRIP_SPP_SMS:
												else if(nSTATE_MOTOR1_SMS == STATE_MOTOR1_TRIP_SPP_SMS)
													{
													 if(Appmodeon==1)
														sprintf(Buffer1," %02d107\n",(datetime.tm_sec+2));
													else
														sprintf(Buffer1,"%s,MOTOR1 OFF BECAUSE SINGLE PHASEING\n",Buffer4);
														sprintf(Buffer1,"%s_IMB VOLT = %03d\n_SET IMB VOLT = %03d\n",Buffer1,VSPPImbalanceVoltage,s_nTimerSettings.m_ImbVolt);

													}
													//break;
												//case STATE_MOTOR1_TRIP_REVERSEPHASE_SMS:  //oro_doubt
												else if(nSTATE_MOTOR1_SMS == STATE_MOTOR1_TRIP_REVERSEPHASE_SMS)
													{
													if(Appmodeon==1)
														sprintf(Buffer1," %02d108\n",(datetime.tm_sec+2));
													else
														sprintf(Buffer1,"%s,MOTOR1 OFF BECAUSE REVERSE PHASE",Buffer4);
													}
													//break;
												//case STATE_MOTOR1_TRIP_CURRENTSPP_SMS:
												else if(nSTATE_MOTOR1_SMS == STATE_MOTOR1_TRIP_CURRENTSPP_SMS)
													{
														sprintf(Buffer1,"%s,MOTOR1 OFF BECAUSE CURRENT SENSEING SPP\n",Buffer4);
														if(Appmodeon==1)
														sprintf(Buffer1," %02d109\n",(datetime.tm_sec+2));

														switch(CSPPValue)
														{
															case 1:
																sprintf(Buffer1,"%s_RC LESS\n",Buffer1);
																break;
															case 2:
																sprintf(Buffer1,"%s_YC LESS\n",Buffer1);
																break;
															case 3:
																sprintf(Buffer1,"%s_BC LESS\n",Buffer1);
																break;
															case 4:
																sprintf(Buffer1,"%s_RC AND YC BOTH DIFF MORE THAN 50%\n",Buffer1);
																break;
															case 5:
																sprintf(Buffer1,"%s_YC AND BC BOTH DIFF MORE THAN 50%\n",Buffer1);
																break;
															default:
																sprintf(Buffer1,"%s_YC AND BC BOTH DIFF MORE THAN 50%\n",Buffer1);
																break;

														}
													}
													//break;
												//case STATE_MOTOR1_TRIP_LOWVOLTAGE_SMS:
												




													else if(nSTATE_MOTOR1_SMS == STATE_MOTOR1_TRIP_LOWVOLTAGE_SMS)
													{
															//if(g_no_of_pumps==1)
																//sprintf(Buffer1,"%s,MOTOR OFF LOW VOLT\n",Buffer4);
															//else
																sprintf(Buffer1,"%s,MOTOR1 OFF LOW VOLT\n",Buffer4);
															if(WorkingOn3Phas == 1)
															{
																if(Appmodeon==1)
																{
																	sprintf(Buffer1,"%02d110\n",(datetime.tm_sec+2));
																/* else
																{ */

																	if(WhichPhaseHaveingProblem == 1)
																	{
																	//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nRY VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltIII,nTimerSettings.DiffVoltIII,TripVoltage);
																	sprintf(Buffer1,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_RY VOLT = %03d\n",Buffer1,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltIII,s_nTimerSettings.m_DiffVoltIII,TripVoltage);			
																	}
																	else if(WhichPhaseHaveingProblem == 2)
																	{
																		//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nYB VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltIII,nTimerSettings.DiffVoltIII,TripVoltage);
																		sprintf(Buffer1,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_RY VOLT = %03d\n",Buffer1,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltIII,s_nTimerSettings.m_DiffVoltIII,TripVoltage);
																	}
																	else if(WhichPhaseHaveingProblem == 3)
																	{
																		//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nBR VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltIII,nTimerSettings.DiffVoltIII,TripVoltage);
																		sprintf(Buffer1,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_RY VOLT = %03d\n",Buffer1,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltIII,s_nTimerSettings.m_DiffVoltIII,TripVoltage);		
																	}						
																	else
																	{
																		sprintf(Buffer1,"No Voltage\n");
																	}
																}
																else
																{

																	if(WhichPhaseHaveingProblem == 1)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 3PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nRY VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltIII,nTimerSettings.DiffVoltIII,TripVoltage);
																		sprintf(Buffer1,"%s RUNING AT 3PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_RY VOLT = %03d\n",Buffer1,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltIII,s_nTimerSettings.m_DiffVoltIII,TripVoltage);		
																	}
																	else if(WhichPhaseHaveingProblem == 2)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 3PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nYB VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltIII,nTimerSettings.DiffVoltIII,TripVoltage);
																		sprintf(Buffer1,"%s RUNING AT 3PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_YB VOLT = %03d\n",Buffer1,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltIII,s_nTimerSettings.m_DiffVoltIII,TripVoltage);
																	}
																	else if(WhichPhaseHaveingProblem == 3)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 3PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nBR VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltIII,nTimerSettings.DiffVoltIII,TripVoltage);
																		sprintf(Buffer1,"%s RUNING AT 3PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_BR VOLT = %03d\n",Buffer1,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltIII,s_nTimerSettings.m_DiffVoltIII,TripVoltage);		
																	}						
																	else
																	{
																		sprintf(Buffer1,"No Voltage\n");
																	}
																}
																sprintf(buf,"which phase :%d set low voltIII :%03d trip voltIII :%03d",WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltIII,TripVoltage);
															sAPI_UartPrintf(buf);
															}
															else
															{
																if(Appmodeon==1)
																{
																	sprintf(Buffer1,"%02d111\n",(datetime.tm_sec+2));
																	if(WhichPhaseHaveingProblem == 1)
																	{
																		//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nRY VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltII,nTimerSettings.DiffVoltII,TripVoltage);
																		sprintf(Buffer1,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_RY VOLT = %03d\n",Buffer1,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltII,s_nTimerSettings.m_DiffVoltII,TripVoltage);
																	}
																	else if(WhichPhaseHaveingProblem == 2)
																	{
																		//sprintf(Buffer1,"%s2PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nYB VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltII,nTimerSettings.DiffVoltII,TripVoltage);
																		sprintf(Buffer1,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_YB VOLT = %03d\n",Buffer1,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltII,s_nTimerSettings.m_DiffVoltII,TripVoltage);		
																	}
																	else if(WhichPhaseHaveingProblem == 3)
																	{
																		//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nBR VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltII,nTimerSettings.DiffVoltII,TripVoltage);
																		sprintf(Buffer1,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_BR VOLT = %03d\n",Buffer1,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltII,s_nTimerSettings.m_DiffVoltII,TripVoltage);
																	}						
																	else
																	{
																		sprintf(Buffer1,"No Voltage\n");
																	}
																}
																else
																{

																	if(WhichPhaseHaveingProblem == 1)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 2PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nRY VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltII,nTimerSettings.DiffVoltII,TripVoltage);
																		sprintf(Buffer1,"%s RUNING AT 2PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_RY VOLT = %03d\n",Buffer1,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltII,s_nTimerSettings.m_DiffVoltII,TripVoltage);		
																	}
																	else if(WhichPhaseHaveingProblem == 2)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 2PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nYB VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltII,nTimerSettings.DiffVoltII,TripVoltage);
																		sprintf(Buffer1,"%s RUNING AT 2PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_YB VOLT = %03d\n",Buffer1,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltII,s_nTimerSettings.m_DiffVoltII,TripVoltage);
																	}
																	else if(WhichPhaseHaveingProblem == 3)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 2PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nBR VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltII,nTimerSettings.DiffVoltII,TripVoltage);
																		sprintf(Buffer1,"%s RUNING AT 2PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_BR VOLT = %03d\n",Buffer1,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltII,s_nTimerSettings.m_DiffVoltII,TripVoltage);
																	}						
																	else
																	{
																		sprintf(Buffer1,"No Voltage\n");
																	}
																}
															sprintf(buf,"_which phase :%d_set low voltII :%03d_trip voltII :%03d",WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltII,TripVoltage);
															sAPI_UartPrintf(buf);

															}
															
														}
													//break;
												
												//case STATE_MOTOR1_TRIP_HIGHVOLTAGE_SMS:
												else if(nSTATE_MOTOR1_SMS == STATE_MOTOR1_TRIP_HIGHVOLTAGE_SMS)
												   {
															sprintf(Buffer1,"%s,MOTOR1 OFF HIGH VOLT\n",Buffer4);
															//if(g_no_of_pumps==1)
																//sprintf(Buffer1,"%s,MOTOR OFF HIGH VOLT\n",Buffer4);
															//else
																//sprintf(Buffer1,"%s,MOTORS ARE OFF HIGH VOLT\n",Buffer4);														
															if(WorkingOn3Phas == 1)
															{
																if(Appmodeon==1)
																{
																	sprintf(Buffer1,"%02d112\n",(datetime.tm_sec+2));
																	if(WhichPhaseHaveingProblem == 1)
																	{
																		//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nRY VOLT = %03.0f\n",Buffer1,
																				//nTimerSettings.HighVoltIII,nTimerSettings.HiDiffVoltIII,TripVoltage);
																		sprintf(Buffer1,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_RY VOLT = %03d\n",Buffer1,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltIII,s_nTimerSettings.m_HiDiffVoltIII,TripVoltage);		
																	}
																	else if(WhichPhaseHaveingProblem == 2)
																	{
																		//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nYB VOLT = %03.0f\n",Buffer1,
																				//nTimerSettings.HighVoltIII,nTimerSettings.HiDiffVoltIII,TripVoltage);
																		sprintf(Buffer1,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_YB VOLT = %03d\n",Buffer1,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltIII,s_nTimerSettings.m_HiDiffVoltIII,TripVoltage);
																	}
																	else if(WhichPhaseHaveingProblem == 3)
																	{
																		//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nBR VOLT = %03.0f\n",Buffer1,
																				//nTimerSettings.HighVoltIII,nTimerSettings.HiDiffVoltIII,TripVoltage);
																		sprintf(Buffer1,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_BR VOLT = %03d\n",Buffer1,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltIII,s_nTimerSettings.m_HiDiffVoltIII,TripVoltage);
																	}						
																	else
																	{
																		sprintf(Buffer1,"No Voltage\n");
																	}
																}
																
																
																else
																{

																	if(WhichPhaseHaveingProblem == 1)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 3PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nRY VOLT = %03.0f\n",Buffer1,
																				//nTimerSettings.HighVoltIII,nTimerSettings.HiDiffVoltIII,TripVoltage);
																		sprintf(Buffer1,"%s RUNING AT 3PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_RY VOLT = %03d\n",Buffer1,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltIII,s_nTimerSettings.m_HiDiffVoltIII,TripVoltage);		
																	}
																	else if(WhichPhaseHaveingProblem == 2)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 3PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nYB VOLT = %03.0f\n",Buffer1,
																				//nTimerSettings.HighVoltIII,nTimerSettings.HiDiffVoltIII,TripVoltage);
																		sprintf(Buffer1,"%s RUNING AT 3PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_YB VOLT = %03d\n",Buffer1,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltIII,s_nTimerSettings.m_HiDiffVoltIII,TripVoltage);		
																	}
																	else if(WhichPhaseHaveingProblem == 3)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 3PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nBR VOLT = %03.0f\n",Buffer1,
																				//nTimerSettings.HighVoltIII,nTimerSettings.HiDiffVoltIII,TripVoltage);
																		sprintf(Buffer1,"%s RUNING AT 3PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_BR VOLT = %03d\n",Buffer1,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltIII,s_nTimerSettings.m_HiDiffVoltIII,TripVoltage);		
																	}						
																	else
																	{
																		sprintf(Buffer1,"No Voltage\n");
																	}
																}
																sprintf(buf,"_which phase :%d_set high voltIII :%03d_trip voltII :%03d",WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltIII,TripVoltage);
															sAPI_UartPrintf(buf);
															}
															else
															{
																if(Appmodeon==1)
																{
																	sprintf(Buffer1,"%02d113\n",(datetime.tm_sec+2));
																	if(WhichPhaseHaveingProblem == 1)
																	{
																		//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nRY VOLT = %03.0f\n",
																				//nTimerSettings.HighVoltII,nTimerSettings.HiDiffVoltII,TripVoltage);
																		sprintf(Buffer1,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_RY VOLT = %03d\n",Buffer1,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltII,s_nTimerSettings.m_HiDiffVoltII,TripVoltage);
																	}
																	else if(WhichPhaseHaveingProblem == 2)
																	{
																		//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nYB VOLT = %03.0f\n",
																				//nTimerSettings.HighVoltII,nTimerSettings.HiDiffVoltII,TripVoltage);
																		sprintf(Buffer1,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_YB VOLT = %03d\n",Buffer1,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltII,s_nTimerSettings.m_HiDiffVoltII,TripVoltage);
																	}
																	else if(WhichPhaseHaveingProblem == 3)
																	{
																		//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nBR VOLT = %03.0f\n",
																				//nTimerSettings.HighVoltII,nTimerSettings.HiDiffVoltII,TripVoltage);
																		sprintf(Buffer1,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_BR VOLT = %03d\n",Buffer1,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltII,s_nTimerSettings.m_HiDiffVoltII,TripVoltage);
																	}						
																	else
																	{
																		sprintf(Buffer1,"No Voltage\n");
																	}
																	
																}
																else
																{

																	if(WhichPhaseHaveingProblem == 1)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 2PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nRY VOLT = %03.0f\n",
																				//nTimerSettings.HighVoltII,nTimerSettings.HiDiffVoltII,TripVoltage);
																		sprintf(Buffer1,"%s RUNING AT 2PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_RY VOLT = %03d\n",Buffer1,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltII,s_nTimerSettings.m_HiDiffVoltII,TripVoltage);
																	}
																	else if(WhichPhaseHaveingProblem == 2)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 2PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nYB VOLT = %03.0f\n",
																				//nTimerSettings.HighVoltII,nTimerSettings.HiDiffVoltII,TripVoltage);
																		sprintf(Buffer1,"%s RUNING AT 2PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_YB VOLT = %03d\n",Buffer1,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltII,s_nTimerSettings.m_HiDiffVoltII,TripVoltage);
																	}
																	else if(WhichPhaseHaveingProblem == 3)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 2PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nBR VOLT = %03.0f\n",
																				//nTimerSettings.HighVoltII,nTimerSettings.HiDiffVoltII,TripVoltage);
																		sprintf(Buffer1,"%s RUNING AT 2PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_BR VOLT = %03d\n",Buffer1,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltII,s_nTimerSettings.m_HiDiffVoltII,TripVoltage);
																	}						
																	else
																	{
																		sprintf(Buffer1,"No Voltage\n");
																	}
																}
															sprintf(buf,"_which phase :%d_set high voltII :%03d_trip voltII :%03d",WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltII,TripVoltage);
															sAPI_UartPrintf(buf);

															}
														}
												
														else if(nSTATE_MOTOR1_SMS ==STATE_NLIGHT_RTCOF_SMS)
														{
															sprintf(Buffer1,"NIGHT LIGHT OFF THROUGH RTC\n");
															sAPI_UartPrintf(Buffer1);
														}
												else if(nSTATE_MOTOR1_SMS == STATE_CYCINTERVEL_OF_SMS)
												{
													if(Appmodeon==1)
														sprintf(Buffer1," %02d027\n",(datetime.tm_sec+2));
													else												
														sprintf(Buffer1,"%s,MOTOR1 OFF BECAUSE CYCLIC INTERVEL",Buffer4);
												}
												else if(nSTATE_MOTOR1_SMS == STATE_CYCLELIMTE_COMPLETED_SMS)
												{
													if(Appmodeon==1)
														sprintf(Buffer1," %02d027\n",(datetime.tm_sec+2));
													else												
														sprintf(Buffer1,"%s,MOTOR1 OFF BECAUSE CYCLIC COMPLETED",Buffer4);
												}
												else if(nSTATE_MOTOR1_SMS == STATE_MOISTURE_OF_SMS)
												{
													if(Appmodeon==1)
														sprintf(Buffer1," %02d027\n",(datetime.tm_sec+2));
													else												
														sprintf(Buffer1,"%s,MOTOR1 OFF BECAUSE MOISTURE LIMIT",Buffer4);
												}
												else if(nSTATE_MOTOR1_SMS == STATE_PAUSE_OF_SMS)
												{
													if(Appmodeon==1)
														sprintf(Buffer1," %02d027\n",(datetime.tm_sec+2));
													else												
														sprintf(Buffer1,"%s,MOTOR1 OFF BECAUSE CYCLE PAUSE",Buffer4);
												}
												else if(nSTATE_MOTOR1_SMS == STATE_WRONG_FEEDBACK_OF_SMS)
												{
													if(Appmodeon==1)
														sprintf(Buffer1," %02d027\n",(datetime.tm_sec+2));
													else												
														sprintf(Buffer1,"%s,MOTOR1 OFF BECAUSE WRONG FEEDBACK",Buffer4);
												}
												else if(nSTATE_MOTOR1_SMS == STATE_NO_COMMUNICATION_OF_SMS)
												{
													if(Appmodeon==1)
														sprintf(Buffer1," %02d027\n",(datetime.tm_sec+2));
													else												
														sprintf(Buffer1,"%s,MOTOR1 OFF BECAUSE NO COMMUNICATION",Buffer4);
												}
												else if(nSTATE_MOTOR1_SMS == STATE_PRESSURE_LOW_SMS || nSTATE_MOTOR1_SMS == STATE_PRESSURE_HIGH_SMS)
												{
													if(Appmodeon==1)
														sprintf(Buffer1," %02d027\n",(datetime.tm_sec+2));
													else		
													{														
														sprintf(Buffer1,"%s,MOTOR1 OFF BECAUSE PRESSURE ",Buffer4);
														sprintf(Buffer1,"%s_ACT PRESS = %0.1f\n_SET PRESS = %0.1f\n",Buffer1,actual_value,set_value);
													}
												}
												
												else if(nSTATE_MOTOR1_SMS == STATE_PRESSURE_SWITCH_OF_SMS)
												{
													if(Appmodeon==1)
														sprintf(Buffer1," %02d027\n",(datetime.tm_sec+2));
													else												
														sprintf(Buffer1,"%s,MOTOR1 OFF BECAUSE PRESSURE SWITCH",Buffer4);
												}
												
												else
												{
													sprintf(Buffer1,"UnKnown1 Please send this to Customer Support Error : nSTATE_MOTOR1_SMS = %d",nSTATE_MOTOR1_SMS);

												}
												//break;
												sprintf(buf,"\nsms_offlag=%d,trip_flag=%d\n",sms_offlag,trip_flag);
												sAPI_UartPrintf(buf);
												
											//	if(sms_offlag != 1 || trip_flag != 1)
												if(((sms_offlag == 1 && trip_flag == 0) || (sms_offlag == 0 && trip_flag == 1)) && nSTATE_MOTOR1_SMS !=STATE_NLIGHT_RTCOF_SMS) 
													{
															sprintf(Buffer1,"%s,%02d:%02d:%02d,%02d:%02d:%02d,020",Buffer1,datetime.tm_hour,datetime.tm_min,datetime.tm_sec,Time_Hr,Time_Min,Time_Sec);
															memset(Buffer4,0,500);
															sprintf(buf,"\n\r 1.TripFlag = [%d],line %d\n\r",trip_flag,__LINE__);
															sAPI_UartPrintf(buf);
															
													//	nSTATE_MOTOR1_SMS = STATE_NO_MOTOR1_SMS;	
													}
													
											}
											
										if(nSTATE_MOTOR1_ON_SMS != STATE_MOTOR1_DUMMY) 
										{
												sprintf(buf,"Motor1 on entry\n\r");
												sAPI_UartPrintf(buf);
												sprintf(buf,"\n\r WorkingOn3Phas = %01d\n\r",WorkingOn3Phas);
												sAPI_UartPrintf(buf);
												
											//switch(nSTATE_MOTOR1_ON_SMS)
											//{
												//case STATE_MOTOR1ON_SMS:
													//{
														
														if(nSTATE_MOTOR1_ON_SMS==STATE_MOTOR1_ON_DEFAULT)
														{

															if(PowerCurrentCondition == 0)
															{

																 sprintf(Buffer4,"AUTO ON POWER ON MOTOR1 ON ");
															//	 switchonofflag=1;


																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer4," %02d001\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer4,"%s WITH 3 PHASE",Buffer4);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer4," %02d002\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer4,"%s WITH 2 PHASE",Buffer4);
																}
															}
															else
															{
																   if(Appmodeon==1)
																	sprintf(Buffer4," %02d003\n",(datetime.tm_sec+2));
																	else
																	sprintf(Buffer4,"AUTO ON POWER OFF MOTOR1 OFF ");
															//	 switchonofflag=1;

																//sprintf(Buffer4,"MOBILE ON BY %s\nPOWER OFF\nMOTOR OFF\n",WhoMadeRelayOn);
															}
														}
														
														else if(nSTATE_MOTOR1_ON_SMS==STATE_MOTOR1_ON_ONDELAY)
														{
																sprintf(Buffer4,"MOTOR1 ON BECAUSE OF CYCLIC ON TIME");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer4," %02d004\n",(datetime.tm_sec+2));
																
																	else
																		sprintf(Buffer4,"%s WITH 3 PHASE",Buffer4);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer4," %02d005\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer4,"%s WITH 2 PHASE",Buffer4);
																}
														}
													
														else if(nSTATE_MOTOR1_ON_SMS==STATE_MOTOR1_ON_SWITCH_SMS)
														{
															sprintf(buf,"nSTATE_MOTOR1_ON_SMS :%d %d\n\r",nSTATE_MOTOR1_ON_SMS,STATE_MOTOR1_ON_SWITCH_SMS);
															sAPI_UartPrintf(buf);
															
																sprintf(Buffer4,"MOTOR1 ON BECAUSE AUTO MOBILE SWITCH");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																	{
																		sprintf(Buffer4," %02d006\n",(datetime.tm_sec+2));
																		sAPI_UartPrintf(Buffer4);
																	}
																	else
																	{
																		//sprintf(Buffer4,"%sMOTOR1 STARTED WITH 3 PHASE",Buffer4);
																	sprintf(Buffer4,"%s WITH 3 PHASE",Buffer4);
																	sAPI_UartPrintf(Buffer4);
																	}
																}
																else
																{
																	if(Appmodeon==1)
									
																		sprintf(Buffer4," %02d007\n",(datetime.tm_sec+2));
																	else{
																		sprintf(Buffer4,"%s WITH 2 PHASE",Buffer4);
																	sAPI_UartPrintf(Buffer4);}
																}
															}
															

														else if(nSTATE_MOTOR1_ON_SMS==STATE_MOTOR1_ON_RTC1)
														{
																sprintf(Buffer4,"MOTOR1 ON BECAUSE OF ON RTC PROGRAM 1 ON TIME");
																sAPI_UartPrintf(Buffer4);//subash commented
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer4," %02d010\nPROG-1\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer4,"%s WITH 3 PHASE",Buffer4);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer4," %02d011\nPROG-1\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer4,"%s WITH 2 PHASE",Buffer4);
																}
																sAPI_UartPrintf(Buffer4);//subash commented
														}
													
														else if(nSTATE_MOTOR1_ON_SMS==STATE_MOTOR1_ON_RTC2)
														{
																sprintf(Buffer4,"MOTOR1 ON BECAUSE OF ON RTC PROGRAM 2 ON TIME");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer4," %02d010\nPROG-2\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer4,"%s WITH 3 PHASE",Buffer4);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer4," %02d011\nPROG-2\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer4,"%s WITH 2 PHASE",Buffer4);
																}
															}
															else if(nSTATE_MOTOR1_ON_SMS==STATE_MOTOR1_ON_RTC3)
															{
																sprintf(Buffer4,"MOTOR1 ON BECAUSE OF ON RTC PROGRAM 3 ON TIME");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer4," %02d010\nPROG-3\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer4,"%s WITH 3 PHASE",Buffer4);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer4," %02d011\nPROG-3\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer4,"%s WITH 2 PHASE",Buffer4);
																}
															}
															else if(nSTATE_MOTOR1_ON_SMS==STATE_MOTOR1_ON_RTC4)
															{
																sprintf(Buffer4,"MOTOR1 ON BECAUSE OF ON RTC PROGRAM 4 ON TIME");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer4," %02d010\nPROG-4\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer4,"%s WITH 3 PHASE",Buffer4);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer4," %02d011\nPROG-4\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer4,"%s WITH 2 PHASE",Buffer4);
																}
														}
														else if(nSTATE_MOTOR1_ON_SMS==STATE_MOTOR1_ON_RTC5)	
														{
																sprintf(Buffer4,"MOTOR1 ON BECAUSE OF ON RTC PROGRAM 5 ON TIME");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer4," %02d010\nPROG-5\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer4,"%s WITH 3 PHASE",Buffer4);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer4," %02d011\nPROG-5\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer4,"%s WITH 2 PHASE",Buffer4);
																}
														}
														else if(nSTATE_MOTOR1_ON_SMS==STATE_MOTOR1_ON_RTC6)	
														{
																sprintf(Buffer4,"MOTOR1 ON BECAUSE OF ON RTC PROGRAM 6 ON TIME");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer4," %02d010\nPROG-6\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer4,"%s WITH 3 PHASE",Buffer4);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer4," %02d011\nPROG-6\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer4,"%s WITH 2 PHASE",Buffer4);
																}
														}
								
														else if(nSTATE_MOTOR1_ON_SMS==STATE_MOTOR1_ON_UPPERTANK)
														{
																sprintf(Buffer4,"MOTOR1 ON BECAUSE OF UPPER TANK EMPTY");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer4," %02d012\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer4,"%s WITH 3 PHASE",Buffer4);
																	sAPI_UartPrintf(Buffer4);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer4," %02d013\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer4,"%s WITH 2 PHASE",Buffer4);
																	sAPI_UartPrintf(Buffer4);
																}
														}
												
														else if(nSTATE_MOTOR1_ON_SMS==STATE_MOTOR1_ON_LOWERTANK)	
														{
																sprintf(Buffer4,"MOTOR1 ON BECAUSE OF SUMP TANK FULL");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer4," %02d014\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer4,"%s WITH 3 PHASE",Buffer4);
																	sAPI_UartPrintf(Buffer4);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer4," %02d015\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer4,"%s WITH 2 PHASE",Buffer4);
																	sAPI_UartPrintf(Buffer4);
																}
															}
														else if(nSTATE_MOTOR1_ON_SMS==STATE_MOTOR1_ON_LEVELSCAN_UPPERTANK)
														{
																sprintf(Buffer4,"MOTOR1 ON BECAUSE OF UPPER TANK EMPTY BY %03d%% ",level_percent);
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer4," %02d012\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer4,"%s WITH 3 PHASE",Buffer4);
																	sAPI_UartPrintf(Buffer4);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer4," %02d013\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer4,"%s WITH 2 PHASE",Buffer4);
																	sAPI_UartPrintf(Buffer4);
																}
														}
														else if(nSTATE_MOTOR1_ON_SMS==STATE_MOTOR1_ON_LEVELSCAN_LOWERTANK)	
														{
																sprintf(Buffer4,"MOTOR1 ON BECAUSE OF SUMP TANK FULL BY %03d%% ",level_percent);
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer4," %02d014\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer4,"%s WITH 3 PHASE",Buffer4);
																	sAPI_UartPrintf(Buffer4);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer4," %02d015\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer4,"%s WITH 2 PHASE",Buffer4);
																	sAPI_UartPrintf(Buffer4);
																}
															}
														
														else if(nSTATE_MOTOR1_ON_SMS==STATE_MOTOR1_ON_LOWERTANK)	
														{
																sprintf(Buffer4,"MOTOR1 ON BECAUSE OF SUMP TANK FULL");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer4," %02d014\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer4,"%s WITH 3 PHASE",Buffer4);
																	sAPI_UartPrintf(Buffer4);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer4," %02d015\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer4,"%s WITH 2 PHASE",Buffer4);
																	sAPI_UartPrintf(Buffer4);
																}
														}
														else if(nSTATE_MOTOR1_ON_SMS==STATE_MOTOR1_ON_LOWVOLTAGE)	
														{
																sprintf(Buffer4,"MOTOR ON");
																if(WorkingOn3Phas == 1)
																{
																	//if(Appmodeon==1)
																		//sprintf(Buffer4," %02d014\n",(datetime.tm_sec+2));
																	//else
																		sprintf(Buffer4,"%s WITH 3 PHASE",Buffer4);
																	sAPI_UartPrintf(Buffer4);
																}
																else
																{
																	//if(Appmodeon==1)
																		//sprintf(Buffer4," %02d015\n",(datetime.tm_sec+2));
																	//else
																		sprintf(Buffer4,"%s WITH 2 PHASE",Buffer4);
																	sAPI_UartPrintf(Buffer4);
																}
															}
														else if(nSTATE_MOTOR1_ON_SMS==STATE_MOTOR1_ON_HIGHVOLTAGE)	
														{
																sprintf(Buffer4,"MOTOR ON");
																if(WorkingOn3Phas == 1)
																{
																	//if(Appmodeon==1)
																		//sprintf(Buffer4," %02d014\n",(datetime.tm_sec+2));
																	//else
																		sprintf(Buffer4,"%s WITH 3 PHASE",Buffer4);
																	sAPI_UartPrintf(Buffer4);
																}
																else
																{
																	//if(Appmodeon==1)
																		//sprintf(Buffer4," %02d015\n",(datetime.tm_sec+2));
																	//else
																		sprintf(Buffer4,"%s WITH 2 PHASE",Buffer4);
																	sAPI_UartPrintf(Buffer4);
																}
															}
															else if(nSTATE_MOTOR1_ON_SMS==STATE_MOTOR1_ON_RESTARTTIMER)
															{
																sprintf(Buffer4,"MOTOR1 ON BECAUSE OF RESTART TIMER");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer4," %02d016\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer4,"%s WITH 3 PHASE",Buffer4);
																	sAPI_UartPrintf(Buffer4);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer4," %02d017\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer4,"%s WITH 2 PHASE",Buffer4);
																	sAPI_UartPrintf(Buffer4);
																}
															}
															else if(nSTATE_MOTOR1_ON_SMS == STATE_MOTOR1_ON_TARGET)
															{
																sprintf(Buffer4,"MOTOR1 ON BECAUSE OF TARGET");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer4," %02d018\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer4,"%s WITH 3 PHASE",Buffer4);
																		sAPI_UartPrintf(Buffer4);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer4," %02d019\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer4,"%s WITH 2 PHASE",Buffer4);
																		sAPI_UartPrintf(Buffer4);
																}
															}
															else if(nSTATE_MOTOR1_ON_SMS ==STATE_NLIGHT_RTCON_SMS)
															{
																sprintf(Buffer1,"NIGHT LIGHT ON THROUGH RTC\n");
																sAPI_UartPrintf(Buffer1);
															}
															else
															{
																sprintf(Buffer1,"Error Sednd this to Service DEPT nSTATE_MOTOR1_ON_SMS = %d",nSTATE_MOTOR1_ON_SMS);
																sAPI_UartPrintf(Buffer1);
															}
															
															sprintf(buf,"sms_onflag = %d, nSTATE_MOTOR1_ON_SMS=%d ",sms_onflag,nSTATE_MOTOR1_ON_SMS);
															sAPI_UartPrintf(buf);

														if(sms_onflag == 1 && nSTATE_MOTOR1_ON_SMS !=STATE_NLIGHT_RTCON_SMS)	
														{
															sprintf(buf,"\n On time entry check\n");
															sAPI_UartPrintf(buf);
															
																	
																	
																//if(on_data_M1 == 0)
																	//sprintf(s_nlogtime[on_data_M1].Act_Mtr1_On,"ON=%02d:%02d:%02d",datetime.tm_hour,datetime.tm_min,datetime.tm_sec);
																sprintf(Buffer4,"%s,%02d:%02d:%02d",Buffer4,datetime.tm_hour,datetime.tm_min,datetime.tm_sec);
																writeonoffbuff();//Praveen Motor On																
																/*else
																	
																sprintf(s_nlogtime[on_data_M1].Act_Mtr1_On,"%s,N=%02d:%02d:%02d",s_nlogtime[on_data_M1].Act_Mtr1_On,datetime.tm_hour,datetime.tm_min,datetime.tm_sec);
																	on_data_M1++;
																	for(char ij=0;ij<on_data_M1;ij++)
																	{
																	sprintf(Buffer4,"%s%s",Buffer4,s_nlogtime[ij].Act_Mtr1_On);
																	}*/
																	
																	sprintf(buf,"\n On time entry1 %s \n",s_nlogtime[on_data_M1].Act_Mtr1_On);
																	sAPI_UartPrintf(buf);
																	
																	
																sprintf(buf,"\n M1_N; on_data_M1: %d\n ",on_data_M1);
																sAPI_UartPrintf(buf);
																
																sprintf(buf,"\n Buffer4[%s]\n ",Buffer4);
																sAPI_UartPrintf(buf);
																
																//if(on_data_M1>22)
																//on_data_M1=0;
															
														//	nSTATE_MOTOR1_ON_SMS = STATE_MOTOR1_DUMMY;	
														}
														sprintf(Buffer1,"%s",Buffer4);		
											}
														
							//			} //dg_delete

										 if(nSTATE_MOTOR2_SMS != STATE_NO_MOTOR2_SMS)
											{
																																			
												//case STATE_MOTOR2OF_SMS:
												if(nSTATE_MOTOR2_SMS == STATE_MOTOR2OF_SMS)
													{
														sprintf(buf,"STATE_MOTOR2OF_SMS case");
														sAPI_UartPrintf(buf);
														if(PowerCurrentCondition == 0)
														{
															if(WorkingOn3Phas == 1)
																{
																 if(Appmodeon==1)
																	sprintf(Buffer2," %02d020\n",(datetime.tm_sec+2));
																else
																  sprintf(Buffer2,"%s,AUTO OFF \nPOWER ON\nMOTOR2 OFF\n",Buffer5);
																//  switchonofflag=1;
																}
																//sprintf(Buffer2,"MOBILE OFF BY %s\nPOWER ON\nMOTOR OFF\n",WhoMadeRelayOn);
																else
																{
																	if(Appmodeon==1)
																	sprintf(Buffer2," %02d021\n",(datetime.tm_sec+2));
																	else
																  sprintf(Buffer2,"%s,AUTO OFF \nPOWER ON\nMOTOR2 OFF\n",Buffer5);
																//  switchonofflag=1;

																}


														}
														else
														{
															   if(Appmodeon==1)
																sprintf(Buffer2," %02d079\n",(datetime.tm_sec+2));
																else
																sprintf(Buffer2,"%s,AUTO OFF \nPOWER OFF\nMOTOR2 OFF\n",Buffer5);
															//	switchonofflag=1;

															//sprintf(Buffer2,"MOBILE OFF BY %s\nPOWER OFF\nMOTOR OFF\n",WhoMadeRelayOn);


														}
													}
													//break;
												//case STATE_MOTOR2_STARTER_TRIP_SMS:
												else if(nSTATE_MOTOR2_SMS == STATE_MOTOR2_STARTER_TRIP_SMS)
												{
													sprintf(buf,"nSTATE_MOTOR2_SMS :%d %d\n\r",nSTATE_MOTOR2_SMS,STATE_MOTOR2_STARTER_TRIP_SMS);
												sAPI_UartPrintf(buf);
												
													sprintf(Buffer2,"%s,MOTOR2 OFF BECAUSE OF STARTER TRIP:%d",Buffer5,(OverAllStarterTrip+1));

													if(WorkingOn3Phas == 1)
													{
													//if(OverAllStarterTrip>3) OverAllStarterTrip=3;
														if(Appmodeon==1)
															sprintf(Buffer2," %02d101\nTRIP-%d\n",(datetime.tm_sec+2),(OverAllStarterTrip+1));
														else
															sprintf(Buffer2,"%s WITH 3 PHASE",Buffer2);
													}
													else
													{
														if(Appmodeon==1)
															sprintf(Buffer2," %02d102\nTRIP-%d\n",(datetime.tm_sec+2),(OverAllStarterTrip+1));
														else
															sprintf(Buffer2,"%s WITH 2 PHASE",Buffer2);
													}
												}
												//break;
												/*case STATE_MOTOR2_UPPERTANK_TRIP_SMS:
                                                    {
                                                    if(Appmodeon==1)
													sprintf(Buffer2,"%02d22\n",(datetime.tm_sec+2));
													else
													sprintf(Buffer2,"MOTOR2 OFF BECAUSE OF PRESSURE SWITCH OPEN");
													}
													break;*/
													//case STATE_MOTOR2_UPPERTANK_TRIP_SMS:
													else if(nSTATE_MOTOR2_SMS == STATE_MOTOR2_UPPERTANK_TRIP_SMS)
													{
														if(Appmodeon==1)
															sprintf(Buffer2," %02d022\n",(datetime.tm_sec+2));   //Previously %02d103
														else
															sprintf(Buffer2,"%s,MOTOR2 OFF BECAUSE OF UPPER TANK FULL",Buffer5);    //dg_changed from MOTOR OFF BECAUSE OF PRESSURE SWITCH OPEN
													}
													//break;
													//case STATE_MOTOR2_LOWERTANK_TRIP_SMS:
												else if(nSTATE_MOTOR2_SMS == STATE_MOTOR2_LOWERTANK_TRIP_SMS)
													{
														if(Appmodeon==1)
															sprintf(Buffer2," %02d023\n",(datetime.tm_sec+2));
														else
															sprintf(Buffer2,"%s,MOTOR2 OFF BECAUSE OF SUMP TANK EMPTY",Buffer5);
													}
													//break;
													else if(nSTATE_MOTOR2_SMS == STATE_MOTOR2_LEVELSCAN_UPPERTANK_TRIP_SMS)
													{
														if(Appmodeon==1)
															sprintf(Buffer2," %02d022\n",(datetime.tm_sec+2));   //Previously %02d103
														else
															sprintf(Buffer2,"%s,MOTOR2 OFF BECAUSE OF UPPER TANK FULL BY %03d%% ",Buffer5,level_percent);//dg_changed from MOTOR OFF BECAUSE OF PRESSURE SWITCH OPEN
													}
													else if(nSTATE_MOTOR2_SMS == STATE_MOTOR2_LEVELSCAN_LOWERTANK_TRIP_SMS)
													{
														if(Appmodeon==1)
															sprintf(Buffer2," %02d023\n",(datetime.tm_sec+2));
														else
															sprintf(Buffer2,"%s,MOTOR2 OFF BECAUSE OF SUMP TANK EMPTY BY %03d%%",Buffer5,level_percent);
													}

												 // case STATE_MOTOR2_ON_SWITCH_TRIP_SMS:
												 else if(nSTATE_MOTOR2_SMS == STATE_MOTOR2_ON_SWITCH_TRIP_SMS)
													 {
														if(Appmodeon==1)
														  sprintf(Buffer2," %02d024\n",(datetime.tm_sec+2));
														else
														 sprintf(Buffer2,"%s,MOTOR2 OFF BECAUSE MOBILE AUTO SWITCH",Buffer5);
													 }
													// break; 
												//case STATE_MOTOR2_DRYRUN_TRIP_SMS:
												else if(nSTATE_MOTOR2_SMS == STATE_MOTOR2_DRYRUN_TRIP_SMS)
													{
														sprintf(Buffer2,"%s,MOTOR2 OFF BECAUSE DRY RUN",Buffer5);
														if(WorkingOn3Phas == 1)
														{

															if(Appmodeon==1)
																sprintf(Buffer2," %02d025\n",(datetime.tm_sec+2));
															else
															{
															//FloatroString1Dig(TpS1tr,s_nTimerSettings.m_DrAmpsII[l_pumpno]I);
																sprintf(Buffer2,"%s WITH 3 PHASE\n",Buffer2);
															}
															sprintf(Buffer2,"%s_WhichPhase=%d_SET AMPS = %02.01f_TripCurrent=%02.01f\n",Buffer2,WhichPhaseHaveingProblem,s_nTimerSettings.m_DrAmpsIII[2],TripCurrent);
														}
														else
														{
															if(Appmodeon==1)
																sprintf(Buffer2," %02d026\n",(datetime.tm_sec+2));
															else
															{
															//FloatroString1Dig(TpS1tr,s_nTimerSettings.m_DrAmpsII[l_pumpno]);
																sprintf(Buffer2,"%s WITH 2 PHASE\n",Buffer2);
															}
															sprintf(Buffer2,"%s_WhichPhase=%d_SET AMPS = %02.01f_TripCurrent=%02.01f\n",Buffer2,WhichPhaseHaveingProblem,s_nTimerSettings.m_DrAmpsII[2],TripCurrent);
														}
														if(WhichPhaseHaveingProblem == 1)
														{
														//FloatroString1Dig(TpS1tr,TripCurrent);
															sprintf(Buffer2,"%s_RC = %02.01f\n",Buffer2,TripCurrent);
														}
														else if(WhichPhaseHaveingProblem == 2)
														{
														//FloatroString1Dig(TpS1tr,TripCurrent);
															sprintf(Buffer2,"%s_YC = %02.01f\n",Buffer2,TripCurrent);
														}
														else if(WhichPhaseHaveingProblem == 3)

														{
														//FloatroString1Dig(TpS1tr,TripCurrent); //check in old
															sprintf(Buffer2,"%s_BC = %02.01f\n",Buffer2,TripCurrent);
														}

													}
													//break;
												//case STATE_MOTOR2_CYCLIC_TRIP_SMS:
												else if(nSTATE_MOTOR2_SMS == STATE_MOTOR2_CYCLIC_TRIP_SMS)
												{
													if(Appmodeon==1)
														sprintf(Buffer2," %02d027\n",(datetime.tm_sec+2));
													else												
														sprintf(Buffer2,"%s,MOTOR2 OFF BECAUSE CYCLIC OFF TIME",Buffer5);
												}
													//break;
												//case STATE_MOTOR2_MAX_TRIP_SMS:
												else if(nSTATE_MOTOR2_SMS == STATE_MOTOR2_MAX_TRIP_SMS)
												{
													if(Appmodeon==1)
														sprintf(Buffer2," %02d104\n",(datetime.tm_sec+2));
													else
														sprintf(Buffer2,"%s,MOTOR2 OFF BECAUSE MAX TIMER REACHED",Buffer5);
												}
													//break;
												/* case STATE_MOTOR2_VCYCCOMPLE_TRIP_SMS:
													//Ql_sprintf(Buffer2,"MOTOR2 OFF BECAUSE VALVE CYCLE COMPLETED AND VALVE CYCLE RESTART OFF");
													if(Appmodeon==1)
														sprintf(Buffer2," %02d115\n",(datetime.tm_sec+2));
													else
														sprintf(Buffer2,"MOTOR2 OFF BECAUSE ZONE CYCLE COMPLETED ");
													sprintf(Buffer2,"%sa_dripcyclecount[l_pumpno]=%03d\ndripdaycount=%03d\n ",Buffer2,a_dripcyclecount[l_pumpno],a_dripcycledate[l_pumpno]);

													break;  */

													 //case STATE_MOTOR_NOCOMMUNICATION_TRIP_SMS:
													else if(nSTATE_MOTOR2_SMS == STATE_MOTOR_NOCOMMUNICATION_TRIP_SMS)
													{
													
													sprintf(Buffer2," %02d124\n",(datetime.tm_sec+2));

													sprintf(Buffer2,"%s,%s,MOTOR OFF BECAUSE NO COMMUNICATION TO VALVES",Buffer2,Buffer5 );
													}

													//break; 


												/* case STATE_MOTOR_GRTCOF_TRIP_SMS:
													if(Appmodeon==1)
													sprintf(Buffer2," %02d029\n",(datetime.tm_sec+2));
													else
													sprintf(Buffer2,"MOTOR OFF BECAUSE GRTC PROGRAM TIMER REACHED");
													sprintf(Buffer2,"%sPROG-%d",Buffer2,RTCOFFLAG);

													break; */

												 //case STATE_MOTOR2_RTCOF1_TRIP_SMS:
												 else if(nSTATE_MOTOR2_SMS == STATE_MOTOR2_RTCOF1_TRIP_SMS)
												 {
													if(Appmodeon==1)
														sprintf(Buffer2," %02d030\nPROG-1\n",(datetime.tm_sec+2));
													else
														sprintf(Buffer2,"%s,MOTOR2 OFF BECAUSE RTC PROGRAM 1 OFF TIMER REACHED",Buffer5);
												 }
													//break;
												//case STATE_MOTOR2_RTCOF2_TRIP_SMS:
												else if(nSTATE_MOTOR2_SMS == STATE_MOTOR2_RTCOF2_TRIP_SMS)
												{
													if(Appmodeon==1)
														sprintf(Buffer2," %02d030\nPROG-2\n",(datetime.tm_sec+2));
													else
														sprintf(Buffer2,"%s,MOTOR2 OFF BECAUSE RTC PROGRAM 2 OFF TIMER REACHED",Buffer5);
												}
													//break;
												//case STATE_MOTOR2_RTCOF3_TRIP_SMS:
												else if(nSTATE_MOTOR2_SMS == STATE_MOTOR2_RTCOF3_TRIP_SMS)
												{
													if(Appmodeon==1)
														sprintf(Buffer2," %02d030\nPROG-3\n",(datetime.tm_sec+2));
													else
														sprintf(Buffer2,"%s,MOTOR2 OFF BECAUSE RTC PROGRAM 3 OFF TIMER REACHED",Buffer5);
												}
													//break;
												//case STATE_MOTOR2_RTCOF4_TRIP_SMS:
												else if(nSTATE_MOTOR2_SMS == STATE_MOTOR2_RTCOF4_TRIP_SMS)
												{
													if(Appmodeon==1)
														sprintf(Buffer2," %02d030\nPROG-4\n",(datetime.tm_sec+2));
													else
														sprintf(Buffer2,"%s,MOTOR2 OFF BECAUSE RTC PROGRAM 4 OFF TIMER REACHED",Buffer5);
												}
													//break;
												//case STATE_MOTOR2_RTCOF5_TRIP_SMS:
												else if(nSTATE_MOTOR2_SMS == STATE_MOTOR2_RTCOF5_TRIP_SMS)
												{
													if(Appmodeon==1)
														sprintf(Buffer2," %02d030\nPROG-5\n",(datetime.tm_sec+2));
													else
														sprintf(Buffer2,"%s,MOTOR2 OFF BECAUSE RTC PROGRAM 5 OFF TIMER REACHED",Buffer5);
												}
													//break; 
													
												//case STATE_MOTOR2_RTCOF6_TRIP_SMS:
												else if(nSTATE_MOTOR2_SMS == STATE_MOTOR2_RTCOF6_TRIP_SMS)
												{
													if(Appmodeon==1)
														sprintf(Buffer2," %02d030\nPROG-6\n",(datetime.tm_sec+2));
													else
														sprintf(Buffer2,"%s,MOTOR2 OFF BECAUSE RTC PROGRAM 6 OFF TIMER REACHED",Buffer5);
												}
													//break; 
												//case STATE_MOTOR2_TRIP_OVERLOAD_SMS:
												else if(nSTATE_MOTOR2_SMS == STATE_MOTOR2_TRIP_OVERLOAD_SMS)
													{
														sprintf(Buffer2,"%s,MOTOR2 OFF BECAUSE OVER LOAD\n",Buffer5);
														if(WorkingOn3Phas == 1)
														{

															if(Appmodeon==1)
																sprintf(Buffer2," %02d105\n",(datetime.tm_sec+2));
															else
															{
															//	FloatroString1Dig(TpS1tr,s_nTimerSettings.m_DrAmpsII[l_pumpno]);
																sprintf(Buffer2,"%s WITH 3 PHASE\n",Buffer2);
															}
															sprintf(Buffer2,"%s_WhichPhase=%d_SET AMPS = %02.01f_TripCurrent=%02.01f\n",Buffer2,WhichPhaseHaveingProblem,s_nTimerSettings.m_OlAmpsIII[2],TripCurrent);

														}
														else
														{

															if(Appmodeon==1)
																sprintf(Buffer2," %02d106\n",(datetime.tm_sec+2));
															else
															{
														//	FloatroString1Dig(TpS1tr,s_nTimerSettings.m_DrAmpsII[l_pumpno]);
																sprintf(Buffer2,"%s WITH 2 PHASE\n",Buffer2);
															}
															sprintf(Buffer2,"%s_WhichPhase=%d_SET AMPS = %02.01f_TripCurrent=%02.01f\n",Buffer2,WhichPhaseHaveingProblem,s_nTimerSettings.m_OlAmpsIII[2],TripCurrent);
														}
														if(WhichPhaseHaveingProblem == 1)
														{
													//	FloatroString1Dig(TpS1tr,TripCurrent);
															sprintf(Buffer2,"%s_RC = %0.1f\n",Buffer2,TripCurrent);
														}
														else if(WhichPhaseHaveingProblem == 2)
														{
													//	FloatroString1Dig(TpS1tr,TripCurrent);
															sprintf(Buffer2,"%s_YC = %0.1f\n",Buffer2,TripCurrent);
														}
														else if(WhichPhaseHaveingProblem == 3)
														{
													//	FloatroString1Dig(TpS1tr,TripCurrent);
															sprintf(Buffer2,"%s_BC = %0.1f\n",Buffer2,TripCurrent);
														}
													
													}
													//break;
													//case STATE_MOTOR2_TRIP_HIGHPRESS_SMS:  //oro_doubt
													else if(nSTATE_MOTOR2_SMS == STATE_MOTOR2_TRIP_HIGHPRESS_SMS)
													{
														sprintf(Buffer2,"%s,MOTOR2 OFF BECAUSE PRESSURE SENSOR HIGH",Buffer5);
														if(WorkingOn3Phas == 1)
														{

															if(Appmodeon==1)
																sprintf(Buffer2," %02d119\n",(datetime.tm_sec+2));
															else
															{
														//	FloatroString1Dig(TpS1tr,s_nTimerSettings.m_DrAmpsII[l_pumpno]);
																sprintf(Buffer2,"%s WITH 3 PHASE\n",Buffer2);
															}
															sprintf(Buffer2,"%s_SET HIPRESSURE = %0.1f\n",Buffer2,s_nTimerSettings.m_highpress[l_pumpno]);

														}
														else
														{

															if(Appmodeon==1)
																sprintf(Buffer2," %02d120\n",(datetime.tm_sec+2));
															else
															{
														//	FloatroString1Dig(TpS1tr,s_nTimerSettings.m_DrAmpsII[l_pumpno]);
																sprintf(Buffer2,"%s WITH 2 PHASE\n",Buffer2);
															}
															sprintf(Buffer2,"%s_SET HIPRESSURE = %0.1f\n",Buffer2,s_nTimerSettings.m_highpress);
														}
														sprintf(Buffer2,"%s_TRIP HIPRESSURE = %0.1f\n",Buffer2,Trippressure);

													}

													//break;
													//case STATE_MOTOR2_TRIP_LOWPRESS_SMS:
													else if(nSTATE_MOTOR2_SMS == STATE_MOTOR2_TRIP_LOWPRESS_SMS)
													{
														sprintf(Buffer2,"%s,MOTOR2 OFF BECAUSE PRESSURE SENSOR LOW",Buffer5);
														if(WorkingOn3Phas == 1)
														{

															if(Appmodeon==1)
																sprintf(Buffer2," %02d121\n",(datetime.tm_sec+2));
															else
															{
														//	FloatroString1Dig(TpS1tr,s_nTimerSettings.m_DrAmpsII[l_pumpno]);
																sprintf(Buffer2,"%s WITH 3 PHASE\n",Buffer2);
															}
															sprintf(Buffer2,"%s_SET LOWPRESSURE = %0.1f\n",Buffer2,s_nTimerSettings.m_lowpress[l_pumpno]);

														}
														else
														{

															if(Appmodeon==1)
																sprintf(Buffer2," %02d122\n",(datetime.tm_sec+2));
															else
															{
														//	FloatroString1Dig(TpS1tr,s_nTimerSettings.m_DrAmpsII[l_pumpno]);
																sprintf(Buffer2,"%s WITH 2 PHASE\n",Buffer2);
															}
															sprintf(Buffer2,"%s_SET LOWPRESSURE = %0.1f\n",Buffer2,s_nTimerSettings.m_lowpress[l_pumpno]);
														}
														sprintf(Buffer2,"%s_TRIP LOWPRESSURE = %0.1f\n",Buffer2,Trippressure);

													}

													//break;  
												//case STATE_MOTOR2_TRIP_SPP_SMS:
												else if(nSTATE_MOTOR2_SMS == STATE_MOTOR2_TRIP_SPP_SMS)
													{
													 if(Appmodeon==1)
														sprintf(Buffer2," %02d107\n",(datetime.tm_sec+2));
													else
														sprintf(Buffer2,"%s,MOTOR2 OFF BECAUSE SINGLE PHASEING",Buffer5);
														sprintf(Buffer2,"%s_IMB VOLT = %03d\n_SET IMB VOLT = %03d\n",Buffer2,VSPPImbalanceVoltage,s_nTimerSettings.m_ImbVolt);

													}
													//break;
												//case STATE_MOTOR2_TRIP_REVERSEPHASE_SMS:  //oro_doubt
												else if(nSTATE_MOTOR2_SMS == STATE_MOTOR2_TRIP_REVERSEPHASE_SMS)
													{
													if(Appmodeon==1)
														sprintf(Buffer2," %02d108\n",(datetime.tm_sec+2));
													else
														sprintf(Buffer2,"%s,MOTOR2 OFF BECAUSE REVERSE PHASE",Buffer5);
													}
													//break;
												//case STATE_MOTOR2_TRIP_CURRENTSPP_SMS:
												else if(nSTATE_MOTOR2_SMS == STATE_MOTOR2_TRIP_CURRENTSPP_SMS)
													{
														sprintf(Buffer2,"%s,MOTOR2 OFF BECAUSE CURRENT SENSEING SPP",Buffer5);
														if(Appmodeon==1)
														sprintf(Buffer2," %02d109\n",(datetime.tm_sec+2));

														switch(CSPPValue)
														{
															case 1:
																sprintf(Buffer2,"%s_RC LESS\n",Buffer2);
																break;
															case 2:
																sprintf(Buffer2,"%s_YC LESS\n",Buffer2);
																break;
															case 3:
																sprintf(Buffer2,"%s_BC LESS\n",Buffer2);
																break;
															case 4:
																sprintf(Buffer2,"%s_RC AND YC BOTH DIFF MORE THAN 50%\n",Buffer2);
																break;
															case 5:
																sprintf(Buffer2,"%s_YC AND BC BOTH DIFF MORE THAN 50%\n",Buffer2);
																break;
															default:
																sprintf(Buffer2,"%s_YC AND BC BOTH DIFF MORE THAN 50%\n",Buffer2);;
																break;

														}
													}
													//break;
												//case STATE_MOTOR2_TRIP_LOWVOLTAGE_SMS:
												




													else if(nSTATE_MOTOR2_SMS == STATE_MOTOR2_TRIP_LOWVOLTAGE_SMS)
													{
															sprintf(Buffer2,"%s,MOTOR2 OFF LOW VOLT",Buffer5);
															if(WorkingOn3Phas == 1)
															{
																if(Appmodeon==1)
																{
																	sprintf(Buffer2,"%02d110\n",(datetime.tm_sec+2));
																/* else
																{ */

																	if(WhichPhaseHaveingProblem == 1)
																	{
																	//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nRY VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltIII,nTimerSettings.DiffVoltIII,TripVoltage);
																	sprintf(Buffer2,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_RY VOLT = %03d\n",Buffer2,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltIII,s_nTimerSettings.m_DiffVoltIII,TripVoltage);			
																	}
																	else if(WhichPhaseHaveingProblem == 2)
																	{
																		//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nYB VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltIII,nTimerSettings.DiffVoltIII,TripVoltage);
																		sprintf(Buffer2,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_RY VOLT = %03d\n",Buffer2,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltIII,s_nTimerSettings.m_DiffVoltIII,TripVoltage);
																	}
																	else if(WhichPhaseHaveingProblem == 3)
																	{
																		//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nBR VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltIII,nTimerSettings.DiffVoltIII,TripVoltage);
																		sprintf(Buffer2,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_RY VOLT = %03d\n",Buffer2,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltIII,s_nTimerSettings.m_DiffVoltIII,TripVoltage);		
																	}						
																	else
																	{
																		sprintf(Buffer2,"No Voltage\n");
																	}
																}
																else
																{

																	if(WhichPhaseHaveingProblem == 1)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 3PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nRY VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltIII,nTimerSettings.DiffVoltIII,TripVoltage);
																		sprintf(Buffer2,"%s RUNING AT 3PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_RY VOLT = %03d\n",Buffer2,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltIII,s_nTimerSettings.m_DiffVoltIII,TripVoltage);		
																	}
																	else if(WhichPhaseHaveingProblem == 2)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 3PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nYB VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltIII,nTimerSettings.DiffVoltIII,TripVoltage);
																		sprintf(Buffer2,"%s RUNING AT 3PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_YB VOLT = %03d\n",Buffer2,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltIII,s_nTimerSettings.m_DiffVoltIII,TripVoltage);
																	}
																	else if(WhichPhaseHaveingProblem == 3)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 3PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nBR VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltIII,nTimerSettings.DiffVoltIII,TripVoltage);
																		sprintf(Buffer2,"%s RUNING AT 3PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_BR VOLT = %03d\n",Buffer2,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltIII,s_nTimerSettings.m_DiffVoltIII,TripVoltage);		
																	}						
																	else
																	{
																		sprintf(Buffer2,"No Voltage\n");
																	}
																}
																sprintf(buf,"which phase :%d set low voltIII :%03d trip voltIII :%03d",WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltIII,TripVoltage);
															sAPI_UartPrintf(buf);
															}
															else
															{
																if(Appmodeon==1)
																{
																	sprintf(Buffer2,"%02d111\n",(datetime.tm_sec+2));
																	if(WhichPhaseHaveingProblem == 1)
																	{
																		//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nRY VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltII,nTimerSettings.DiffVoltII,TripVoltage);
																		sprintf(Buffer2,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_RY VOLT = %03d\n",Buffer2,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltII,s_nTimerSettings.m_DiffVoltII,TripVoltage);
																	}
																	else if(WhichPhaseHaveingProblem == 2)
																	{
																		//sprintf(Buffer1,"%s2PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nYB VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltII,nTimerSettings.DiffVoltII,TripVoltage);
																		sprintf(Buffer2,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_YB VOLT = %03d\n",Buffer2,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltII,s_nTimerSettings.m_DiffVoltII,TripVoltage);		
																	}
																	else if(WhichPhaseHaveingProblem == 3)
																	{
																		//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nBR VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltII,nTimerSettings.DiffVoltII,TripVoltage);
																		sprintf(Buffer2,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_BR VOLT = %03d\n",Buffer2,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltII,s_nTimerSettings.m_DiffVoltII,TripVoltage);
																	}						
																	else
																	{
																		sprintf(Buffer2,"No Voltage\n");
																	}
																}
																else
																{

																	if(WhichPhaseHaveingProblem == 1)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 2PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nRY VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltII,nTimerSettings.DiffVoltII,TripVoltage);
																		sprintf(Buffer2,"%s RUNING AT 2PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_RY VOLT = %03d\n",Buffer2,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltII,s_nTimerSettings.m_DiffVoltII,TripVoltage);		
																	}
																	else if(WhichPhaseHaveingProblem == 2)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 2PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nYB VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltII,nTimerSettings.DiffVoltII,TripVoltage);
																		sprintf(Buffer2,"%s RUNING AT 2PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_YB VOLT = %03d\n",Buffer2,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltII,s_nTimerSettings.m_DiffVoltII,TripVoltage);
																	}
																	else if(WhichPhaseHaveingProblem == 3)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 2PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nBR VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltII,nTimerSettings.DiffVoltII,TripVoltage);
																		sprintf(Buffer2,"%s RUNING AT 2PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_BR VOLT = %03d\n",Buffer2,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltII,s_nTimerSettings.m_DiffVoltII,TripVoltage);
																	}						
																	else
																	{
																		sprintf(Buffer2,"No Voltage\n");
																	}
																}


															}
														}
													//break;
												
												//case STATE_MOTOR2_TRIP_HIGHVOLTAGE_SMS:
												else if(nSTATE_MOTOR2_SMS == STATE_MOTOR2_TRIP_HIGHVOLTAGE_SMS)
												   {
															sprintf(Buffer2,"%s,MOTOR2 OFF HIGH VOLT",Buffer5);
															if(WorkingOn3Phas == 1)
															{
																if(Appmodeon==1)
																{
																	sprintf(Buffer2,"%02d112\n",(datetime.tm_sec+2));
																	if(WhichPhaseHaveingProblem == 1)
																	{
																		//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nRY VOLT = %03.0f\n",Buffer1,
																				//nTimerSettings.HighVoltIII,nTimerSettings.HiDiffVoltIII,TripVoltage);
																		sprintf(Buffer2,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_RY VOLT = %03d\n",Buffer2,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltIII,s_nTimerSettings.m_HiDiffVoltIII,TripVoltage);		
																	}
																	else if(WhichPhaseHaveingProblem == 2)
																	{
																		//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nYB VOLT = %03.0f\n",Buffer1,
																				//nTimerSettings.HighVoltIII,nTimerSettings.HiDiffVoltIII,TripVoltage);
																		sprintf(Buffer2,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_YB VOLT = %03d\n",Buffer2,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltIII,s_nTimerSettings.m_HiDiffVoltIII,TripVoltage);
																	}
																	else if(WhichPhaseHaveingProblem == 3)
																	{
																		//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nBR VOLT = %03.0f\n",Buffer1,
																				//nTimerSettings.HighVoltIII,nTimerSettings.HiDiffVoltIII,TripVoltage);
																		sprintf(Buffer2,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_BR VOLT = %03d\n",Buffer2,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltIII,s_nTimerSettings.m_HiDiffVoltIII,TripVoltage);
																	}						
																	else
																	{
																		sprintf(Buffer2,"No Voltage\n");
																	}
																}
																
																
																else
																{

																	if(WhichPhaseHaveingProblem == 1)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 3PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nRY VOLT = %03.0f\n",Buffer1,
																				//nTimerSettings.HighVoltIII,nTimerSettings.HiDiffVoltIII,TripVoltage);
																		sprintf(Buffer2,"%s RUNING AT 3PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_RY VOLT = %03d\n",Buffer2,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltIII,s_nTimerSettings.m_HiDiffVoltIII,TripVoltage);		
																	}
																	else if(WhichPhaseHaveingProblem == 2)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 3PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nYB VOLT = %03.0f\n",Buffer1,
																				//nTimerSettings.HighVoltIII,nTimerSettings.HiDiffVoltIII,TripVoltage);
																		sprintf(Buffer2,"%s RUNING AT 3PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_YB VOLT = %03d\n",Buffer2,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltIII,s_nTimerSettings.m_HiDiffVoltIII,TripVoltage);		
																	}
																	else if(WhichPhaseHaveingProblem == 3)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 3PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nBR VOLT = %03.0f\n",Buffer1,
																				//nTimerSettings.HighVoltIII,nTimerSettings.HiDiffVoltIII,TripVoltage);
																		sprintf(Buffer2,"%s RUNING AT 3PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_BR VOLT = %03d\n",Buffer2,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltIII,s_nTimerSettings.m_HiDiffVoltIII,TripVoltage);		
																	}						
																	else
																	{
																		sprintf(Buffer2,"No Voltage\n");
																	}
																}
																sprintf(buf,"_which phase :%d_set high voltIII :%03d_trip voltII :%03d",WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltIII,TripVoltage);
															sAPI_UartPrintf(buf);
															}
															else
															{
																if(Appmodeon==1)
																{
																	sprintf(Buffer2,"%02d113\n",(datetime.tm_sec+2));
																	if(WhichPhaseHaveingProblem == 1)
																	{
																		//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nRY VOLT = %03.0f\n",
																				//nTimerSettings.HighVoltII,nTimerSettings.HiDiffVoltII,TripVoltage);
																		sprintf(Buffer2,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_RY VOLT = %03d\n",Buffer2,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltII,s_nTimerSettings.m_HiDiffVoltII,TripVoltage);
																	}
																	else if(WhichPhaseHaveingProblem == 2)
																	{
																		//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nYB VOLT = %03.0f\n",
																				//nTimerSettings.HighVoltII,nTimerSettings.HiDiffVoltII,TripVoltage);
																		sprintf(Buffer2,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_YB VOLT = %03d\n",Buffer2,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltII,s_nTimerSettings.m_HiDiffVoltII,TripVoltage);
																	}
																	else if(WhichPhaseHaveingProblem == 3)
																	{
																		//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nBR VOLT = %03.0f\n",
																				//nTimerSettings.HighVoltII,nTimerSettings.HiDiffVoltII,TripVoltage);
																		sprintf(Buffer2,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_BR VOLT = %03d\n",Buffer2,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltII,s_nTimerSettings.m_HiDiffVoltII,TripVoltage);
																	}						
																	else
																	{
																		sprintf(Buffer2,"No Voltage\n");
																	}
																	
																}
																else
																{

																	if(WhichPhaseHaveingProblem == 1)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 2PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nRY VOLT = %03.0f\n",
																				//nTimerSettings.HighVoltII,nTimerSettings.HiDiffVoltII,TripVoltage);
																		sprintf(Buffer2,"%s RUNING AT 2PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_RY VOLT = %03d\n",Buffer2,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltII,s_nTimerSettings.m_HiDiffVoltII,TripVoltage);
																	}
																	else if(WhichPhaseHaveingProblem == 2)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 2PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nYB VOLT = %03.0f\n",
																				//nTimerSettings.HighVoltII,nTimerSettings.HiDiffVoltII,TripVoltage);
																		sprintf(Buffer2,"%s RUNING AT 2PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_YB VOLT = %03d\n",Buffer2,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltII,s_nTimerSettings.m_HiDiffVoltII,TripVoltage);
																	}
																	else if(WhichPhaseHaveingProblem == 3)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 2PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nBR VOLT = %03.0f\n",
																				//nTimerSettings.HighVoltII,nTimerSettings.HiDiffVoltII,TripVoltage);
																		sprintf(Buffer2,"%s RUNING AT 2PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_BR VOLT = %03d\n",Buffer2,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltII,s_nTimerSettings.m_HiDiffVoltII,TripVoltage);
																	}						
																	else
																	{
																		sprintf(Buffer2,"No Voltage\n");
																	}
																}


															}
														}
												
												
												
												

													//break;
												/* case STATE_MOTOR2OF_3PHASE_ONLY_SMS://oro_doubt
												{
													sprintf(Buffer2,"MOTOR2 CAN'T START BEACUSE 3 PHASE SELECTION ONLY\n");
												}
												break; */
												/* case STATE_STGIII_SMS1:    //oro_doubt
													{
														sprintf(Buffer2,"3 Phase Current Set\n");
														if(nMSettings.DryRunOnOff == 1)
															sprintf(Buffer2,"%sDRYRUN SCAN ON\n",Buffer2);
														else
															sprintf(Buffer2,"%sDRYRUN SCAN OFF\n",Buffer2);
															FloatroString1Dig(TpS1tr,nTimerSettings.DrAmpsIII);
														sprintf(Buffer2,"%sDRY RUN AMPS FOR 3 PHASE = %s\n",Buffer2,TpS1tr);

														if(nTimerSettings.OlOnOff == 1)
															sprintf(Buffer2,"%sOVER LOAD SCAN ON\n",Buffer2);
														else
															sprintf(Buffer2,"%sOVER LOAD SCAN OFF",Buffer2);
														FloatroString1Dig(TpS1tr,nTimerSettings.OlAmpsIII);
														sprintf(Buffer2,"%sOVER LOAD AMPS FOR 3 PHASE = %s\n",Buffer2,TpS1tr);


													//	sprintf(Buffer2,"%sDRY RUN SCAN TIME = %02d:%02d:%02d\n",Buffer2,s_nTimerSettings.m_DrScHr,s_nTimerSettings.DrScMin,s_nTimerSettings.DrScSec);
													//	sprintf(Buffer2,"%sOVER LOAD SCAN TIME = %02d:%02d:%02d\n",Buffer2,s_nTimerSettings.OlScanHr,s_nTimerSettings.OlScanMin,s_nTimerSettings.OlScanSec);
													}
													break;
												case STATE_STGII_SMS1:
													{
														sprintf(Buffer2,"2 Phase Current Set\n");
														if(nMSettings.DryRunOnOff == 1)
															sprintf(Buffer2,"%sDRYRUN SCAN ON\n",Buffer2);
														else
															sprintf(Buffer2,"%sDRYRUN SCAN OFF\n",Buffer2);
															FloatroString1Dig(TpS1tr,nTimerSettings.DrAmpsII);
														sprintf(Buffer2,"%sDRY RUN AMPS FOR 2 PHASE = %s\n",Buffer2,TpS1tr);

														if(nTimerSettings.OlOnOff == 1)
															sprintf(Buffer2,"%sOVER LOAD SCAN ON\n",Buffer2);
														else
															sprintf(Buffer2,"%sOVER LOAD SCAN OFF",Buffer2);
														 FloatroString1Dig(TpS1tr,nTimerSettings.OlAmpsIII);
														sprintf(Buffer2,"%sOVER LOAD AMPS FOR 2 PHASE = %s\n",Buffer2,TpS1tr);

													//	sprintf(Buffer2,"%sDRY RUN SCAN TIME = %02d:%02d:%02d\n",Buffer2,s_nTimerSettings.DrScHr,s_nTimerSettings.DrScMin,s_nTimerSettings.DrScSec);
													//	sprintf(Buffer2,"%sOVER LOAD SCAN TIME = %02d:%02d:%02d\n",Buffer2,s_nTimerSettings.OlScanHr,s_nTimerSettings.OlScanMin,s_nTimerSettings.OlScanSec);
													}
													break; */
												//default:	
												else
												{
													sprintf(Buffer2,"UnKnown1 Please send this to Customer Support Error : nSTATE_MOTOR2_SMS = %d",nSTATE_MOTOR2_SMS);

												}
												//break;
												if((sms_offlag1 == 1 && trip_flag1 == 0) || (sms_offlag1 == 0 && trip_flag1 == 1)) 
													{
														/* if(on_data_M2 == 0)
																	sprintf(s_nlogtime[on_data_M2].Act_Mtr2_On,"LOG,F=%02d:%02d:%02d",datetime.tm_hour,datetime.tm_min,datetime.tm_sec);
																else
																	sprintf(s_nlogtime[on_data_M2].Act_Mtr2_On,"%s,F=%02d:%02d:%02d",s_nlogtime[on_data_M2].Act_Mtr2_On,datetime.tm_hour,datetime.tm_min,datetime.tm_sec);

																	on_data_M2++;
																	for(char ij=0;ij<on_data_M2;ij++)
																	{
																	sprintf(Buffer2,"%s%s",Buffer2,s_nlogtime[ij].Act_Mtr2_On);
																	}
																	
																	
																	sprintf(buf,"\n M2_F; on_data_M2: %d\n ",on_data_M2);
																	sAPI_UartPrintf(buf);
																	
															if(on_data_M2>22)
																on_data_M2=0;  */
															sprintf(Buffer2,"%s,%02d:%02d:%02d,%02d:%02d:%02d,020",Buffer2,datetime.tm_hour,datetime.tm_min,datetime.tm_sec,Time_Hr1,Time_Min1,Time_Sec1);
														memset(Buffer5,0,500);
															
													}

											}
											
										if(nSTATE_MOTOR2_ON_SMS != STATE_MOTOR2_DUMMY)
										{
												sprintf(buf,"nSTATE_MOTOR2_ON_SMS :%d %d\n\r",nSTATE_MOTOR2_ON_SMS,STATE_MOTOR2_ON_SWITCH_SMS);
												sAPI_UartPrintf(buf);
												
												sprintf(buf,"\n\r WorkingOn3Phas = %01d\n\r",WorkingOn3Phas);
												sAPI_UartPrintf(buf);
												
											//switch(nSTATE_MOTOR2_ON_SMS)
											//{
												//case STATE_MOTOR2ON_SMS:
													//{
														
														if(nSTATE_MOTOR2_ON_SMS==STATE_MOTOR2_ON_DEFAULT)
														{

															if(PowerCurrentCondition == 0)
															{

																 sprintf(Buffer5,"AUTO ON \nPOWER ON\nMOTOR2 ON");
															//	 switchonofflag=1;


																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer5," %02d001\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer5,"%s WITH 3 PHASE",Buffer5);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer5," %02d002\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer5,"%s WITH 2 PHASE",Buffer5);
																}
															}
															else
															{
																   if(Appmodeon==1)
																	sprintf(Buffer5," %02d003\n",(datetime.tm_sec+2));
																	else
																	sprintf(Buffer5,"AUTO ON \nPOWER OFF\nMOTOR2 OFF\n");
															//	 switchonofflag=1;

																//sprintf(Buffer5,"MOBILE ON BY %s\nPOWER OFF\nMOTOR OFF\n",WhoMadeRelayOn);
															}
														}
														
														else if(nSTATE_MOTOR2_ON_SMS==STATE_MOTOR2_ON_ONDELAY)
														{
																sprintf(Buffer5,"MOTOR2 ON BECAUSE OF CYCLIC ON TIME");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer5," %02d004\n",(datetime.tm_sec+2));
																
																	else
																		sprintf(Buffer5,"%s WITH 3 PHASE",Buffer5);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer5," %02d005\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer5,"%s WITH 2 PHASE",Buffer5);
																}
														}
													
														else if(nSTATE_MOTOR2_ON_SMS==STATE_MOTOR2_ON_SWITCH_SMS)
														{
															sprintf(buf,"nSTATE_MOTOR2_ON_SMS :%d %d\n\r",nSTATE_MOTOR2_ON_SMS,STATE_MOTOR2_ON_SWITCH_SMS);
															sAPI_UartPrintf(buf);
															
																sprintf(Buffer5,"MOTOR2 ON BECAUSE AUTO MOBILE SWITCH\n");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																	{
																		sprintf(Buffer5," %02d006\n",(datetime.tm_sec+2));
																		sAPI_UartPrintf(Buffer5);
																	}
																	else
																	{
																		//sprintf(Buffer5,"%sMOTOR2 STARTED WITH 3 PHASE",Buffer5);
																	sprintf(Buffer5,"%s WITH 3 PHASE",Buffer5);
																	sAPI_UartPrintf(Buffer5);
																	}
																}
																else
																{
																	if(Appmodeon==1)
									
																		sprintf(Buffer5," %02d007\n",(datetime.tm_sec+2));
																	else{
																		sprintf(Buffer5,"%s WITH 2 PHASE",Buffer5);
																	sAPI_UartPrintf(Buffer5);}
																}
														}

														else if(nSTATE_MOTOR2_ON_SMS==STATE_MOTOR2_ON_RTC1)
														{
																sprintf(Buffer5,"MOTOR2 ON BECAUSE OF ON RTC PROGRAM 1 ON TIME");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer5," %02d010\nPROG-1\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer5,"%s WITH 3 PHASE",Buffer5);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer5," %02d011\nPROG-1\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer5,"%s WITH 2 PHASE",Buffer5);
																}
														}
													
														else if(nSTATE_MOTOR2_ON_SMS==STATE_MOTOR2_ON_RTC2)
														{
																sprintf(Buffer5,"MOTOR2 ON BECAUSE OF ON RTC PROGRAM 2 ON TIME");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer5," %02d010\nPROG-2\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer5,"%s WITH 3 PHASE",Buffer5);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer5," %02d011\nPROG-2\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer5,"%s WITH 2 PHASE",Buffer5);
																}
														}

														else if(nSTATE_MOTOR2_ON_SMS==STATE_MOTOR2_ON_RTC3)	
														{
																sprintf(Buffer5,"MOTOR2 ON BECAUSE OF ON RTC PROGRAM 3 ON TIME");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer5," %02d010\nPROG-3\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer5,"%s WITH 3 PHASE",Buffer5);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer5," %02d011\nPROG-3\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer5,"%s WITH 2 PHASE",Buffer5);
																}
														}
								
														else if(nSTATE_MOTOR2_ON_SMS==STATE_MOTOR2_ON_RTC4)	
														{
																sprintf(Buffer5,"MOTOR2 ON BECAUSE OF ON RTC PROGRAM 4 ON TIME");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer5," %02d010\nPROG-4\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer5,"%s WITH 3 PHASE",Buffer5);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer5," %02d011\nPROG-4\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer5,"%s WITH 2 PHASE",Buffer5);
																}
														}
														else if(nSTATE_MOTOR2_ON_SMS==STATE_MOTOR2_ON_RTC5)	
														{
																sprintf(Buffer5,"MOTOR2 ON BECAUSE OF ON RTC PROGRAM 5 ON TIME");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer5," %02d010\nPROG-5\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer5,"%s WITH 3 PHASE",Buffer5);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer5," %02d011\nPROG-5\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer5,"%s WITH 2 PHASE",Buffer5);
																}
														}
														else if(nSTATE_MOTOR2_ON_SMS==STATE_MOTOR2_ON_RTC6)	
														{
																sprintf(Buffer5,"MOTOR2 ON BECAUSE OF ON RTC PROGRAM 6 ON TIME");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer5," %02d010\nPROG-6\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer5,"%s WITH 3 PHASE",Buffer5);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer5," %02d011\nPROG-6\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer5,"%s WITH 2 PHASE",Buffer5);
																}
														}
								
														else if(nSTATE_MOTOR2_ON_SMS==STATE_MOTOR2_ON_UPPERTANK)
														{
																sprintf(Buffer5,"MOTOR2 ON BECAUSE OF UPPER TANK EMPTY");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer5," %02d012\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer5,"%s WITH 3 PHASE",Buffer5);
																	sAPI_UartPrintf(Buffer5);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer5," %02d013\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer5,"%s WITH 2 PHASE",Buffer5);
																	sAPI_UartPrintf(Buffer5);
																}
														}
												
														else if(nSTATE_MOTOR2_ON_SMS==STATE_MOTOR2_ON_LOWERTANK)	
														{
																sprintf(Buffer5,"MOTOR2 ON BECAUSE OF SUMP TANK FULL");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer5," %02d014\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer5,"%s WITH 3 PHASE",Buffer5);
																	sAPI_UartPrintf(Buffer5);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer5," %02d015\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer5,"%s WITH 2 PHASE",Buffer5);
																	sAPI_UartPrintf(Buffer5);
																}
														}
														
														else if(nSTATE_MOTOR2_ON_SMS==STATE_MOTOR2_ON_LEVELSCAN_UPPERTANK)
														{
																sprintf(Buffer5,"MOTOR2 ON BECAUSE OF UPPER TANK EMPTY BY %03d%%",level_percent);
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer5," %02d012\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer5,"%s WITH 3 PHASE",Buffer5);
																	sAPI_UartPrintf(Buffer5);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer5," %02d013\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer5,"%s WITH 2 PHASE",Buffer5);
																	sAPI_UartPrintf(Buffer5);
																}
														}
														
														else if(nSTATE_MOTOR2_ON_SMS==STATE_MOTOR2_ON_LEVELSCAN_LOWERTANK)	
														{
																sprintf(Buffer5,"MOTOR2 ON BECAUSE OF SUMP TANK FULL BY %03d%%",level_percent);
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer5," %02d014\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer5,"%s WITH 3 PHASE",Buffer5);
																	sAPI_UartPrintf(Buffer5);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer5," %02d015\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer5,"%s WITH 2 PHASE",Buffer5);
																	sAPI_UartPrintf(Buffer5);
																}
														}
												
														else if(nSTATE_MOTOR2_ON_SMS==STATE_MOTOR2_ON_LOWERTANK)	
														{
																sprintf(Buffer5,"MOTOR2 ON BECAUSE OF SUMP TANK FULL");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer5," %02d014\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer5,"%s WITH 3 PHASE",Buffer5);
																	sAPI_UartPrintf(Buffer5);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer5," %02d015\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer5,"%s WITH 2 PHASE",Buffer5);
																	sAPI_UartPrintf(Buffer5);
																}
														}
									
														else if(nSTATE_MOTOR2_ON_SMS==STATE_MOTOR2_ON_RESTARTTIMER)	
														{
																sprintf(Buffer5,"MOTOR2 ON BECAUSE OF RESTART TIMER");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer5," %02d016\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer5,"%s WITH 3 PHASE",Buffer5);
																	sAPI_UartPrintf(Buffer5);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer5," %02d017\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer5,"%s WITH 2 PHASE",Buffer5);
																	sAPI_UartPrintf(Buffer5);
																}
														}
										
														else if(nSTATE_MOTOR2_ON_SMS == STATE_MOTOR2_ON_TARGET)
														{
																sprintf(Buffer5,"MOTOR2 ON BECAUSE OF TARGET");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer5," %02d018\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer5,"%s WITH 3 PHASE",Buffer5);
																		sAPI_UartPrintf(Buffer5);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer5," %02d019\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer5,"%s WITH 2 PHASE",Buffer5);
																		sAPI_UartPrintf(Buffer5);
																}
														}
														
														else	
														{
																sprintf(Buffer5,"Error Sednd this to Service DEPT nSTATE_MOTOR2_ON_SMS = %d",nSTATE_MOTOR2_ON_SMS);
																sAPI_UartPrintf(Buffer5);
														}
														if(sms_onflag1 == 1)	
														{
															sprintf(buf,"\n On time entry check2\n");
															sAPI_UartPrintf(buf);
															
																sprintf(Buffer5,"%s,%02d:%02d:%02d",Buffer5,datetime.tm_hour,datetime.tm_min,datetime.tm_sec);	
																	
																/*if(on_data_M2 == 0)
																	sprintf(s_nlogtime[on_data_M2].Act_Mtr2_On,"LOG,N=%02d:%02d:%02d",datetime.tm_hour,datetime.tm_min,datetime.tm_sec);
																else
																	sprintf(s_nlogtime[on_data_M2].Act_Mtr2_On,"%s,N=%02d:%02d:%02d",s_nlogtime[on_data_M2].Act_Mtr2_On,datetime.tm_hour,datetime.tm_min,datetime.tm_sec);
																	on_data_M2++;
																	for(char ij=0;ij<on_data_M2;ij++)
																	{
																	sprintf(Buffer5,"%s%s",Buffer5,s_nlogtime[ij].Act_Mtr2_On);
																	}
																	
																	sprintf(buf,"\n On time entry2 %s \n",s_nlogtime[on_data_M2].Act_Mtr2_On);
																	sAPI_UartPrintf(buf);
																	
																	
																sprintf(buf,"\n M2_N; on_data_M2: %d\n ",on_data_M2);
																sAPI_UartPrintf(buf);
																if(on_data_M2>22)
																on_data_M2=0;*/
																
														}
														sprintf(Buffer2,"%s",Buffer5);		
										}
										
										 if(nSTATE_MOTOR3_SMS != STATE_NO_MOTOR3_SMS)
											{
																							
												//case STATE_MOTOR3OF_SMS:
												if(nSTATE_MOTOR3_SMS == STATE_MOTOR3OF_SMS)
													{
														sprintf(buf,"STATE_MOTOR3OF_SMS case");
														//sAPI_UartPrintf(buf);
														if(PowerCurrentCondition == 0)
														{
															if(WorkingOn3Phas == 1)
																{
																 if(Appmodeon==1)
																	sprintf(Buffer3," %02d020\n",(datetime.tm_sec+2));
																else
																  sprintf(Buffer3,"%s,AUTO OFF \nPOWER ON\nMOTOR3 OFF\n",Buffer6);
																//  switchonofflag=1;
																}
																//sprintf(Buffer3,"MOBILE OFF BY %s\nPOWER ON\nMOTOR OFF\n",WhoMadeRelayOn);
																else
																{
																	if(Appmodeon==1)
																	sprintf(Buffer3," %02d021\n",(datetime.tm_sec+2));
																	else
																  sprintf(Buffer3,"%s,AUTO OFF \nPOWER ON\nMOTOR3 OFF\n",Buffer6);
															//	  switchonofflag=1;

																}


														}
														else
														{
															   if(Appmodeon==1)
																sprintf(Buffer3," %02d079\n",(datetime.tm_sec+2));
																else
																sprintf(Buffer3,"%s,AUTO OFF \nPOWER OFF\nMOTOR3 OFF\n",Buffer6);
															//	switchonofflag=1;

															//sprintf(Buffer3,"MOBILE OFF BY %s\nPOWER OFF\nMOTOR OFF\n",WhoMadeRelayOn);


														}
													}
													//break;
												//case STATE_MOTOR3_STARTER_TRIP_SMS:
												else if(nSTATE_MOTOR3_SMS == STATE_MOTOR3_STARTER_TRIP_SMS)
												{
													sprintf(buf,"nSTATE_MOTOR3_SMS :%d %d\n\r",nSTATE_MOTOR3_SMS,STATE_MOTOR3_STARTER_TRIP_SMS);
												sAPI_UartPrintf(buf);
												
													sprintf(Buffer3,"%s,MOTOR3 OFF BECAUSE OF STARTER TRIP:%d",Buffer6,(OverAllStarterTrip3+1));

													if(WorkingOn3Phas == 1)
													{
													//if(OverAllStarterTrip>3) OverAllStarterTrip=3;
														if(Appmodeon==1)
															sprintf(Buffer3," %02d101\nTRIP-%d\n",(datetime.tm_sec+2),(OverAllStarterTrip3+1));
														else
															sprintf(Buffer3,"%s WITH 3 PHASE",Buffer3);
													}
													else
													{
														if(Appmodeon==1)
															sprintf(Buffer3," %02d102\nTRIP-%d\n",(datetime.tm_sec+2),(OverAllStarterTrip+1));
														else
															sprintf(Buffer3,"%s WITH 2 PHASE",Buffer3);
													}
												}
												//break;
												/*case STATE_MOTOR3_UPPERTANK_TRIP_SMS:
                                                    {
                                                    if(Appmodeon==1)
													sprintf(Buffer3,"%02d22\n",(datetime.tm_sec+2));
													else
													sprintf(Buffer3,"MOTOR3 OFF BECAUSE OF PRESSURE SWITCH OPEN");
													}
													break;*/
													//case STATE_MOTOR3_UPPERTANK_TRIP_SMS:
													else if(nSTATE_MOTOR3_SMS == STATE_MOTOR3_UPPERTANK_TRIP_SMS)
													{
														if(Appmodeon==1)
															sprintf(Buffer3," %02d022\n",(datetime.tm_sec+2));   //Previously %02d103
														else
															sprintf(Buffer3,"%s,MOTOR3 OFF BECAUSE OF UPPER TANK FULL",Buffer6);    //dg_changed from MOTOR OFF BECAUSE OF PRESSURE SWITCH OPEN
													}
													//break;
													//case STATE_MOTOR3_LOWERTANK_TRIP_SMS:
												else if(nSTATE_MOTOR3_SMS == STATE_MOTOR3_LOWERTANK_TRIP_SMS)
													{
														if(Appmodeon==1)
															sprintf(Buffer3," %02d023\n",(datetime.tm_sec+2));
														else
															sprintf(Buffer3,"%s,MOTOR3 OFF BECAUSE OF SUMP TANK EMPTY",Buffer6);
													}
													//break;
													else if(nSTATE_MOTOR3_SMS == STATE_MOTOR3_LEVELSCAN_UPPERTANK_TRIP_SMS)
													{
														if(Appmodeon==1)
															sprintf(Buffer3," %02d022\n",(datetime.tm_sec+2));   //Previously %02d103
														else
															sprintf(Buffer3,"%s,MOTOR3 OFF BECAUSE OF UPPER TANK FULL BY %03d%%",Buffer6,level_percent);//dg_changed from MOTOR OFF BECAUSE OF PRESSURE SWITCH OPEN
													}
													else if(nSTATE_MOTOR3_SMS == STATE_MOTOR3_LEVELSCAN_LOWERTANK_TRIP_SMS)
													{
														if(Appmodeon==1)
															sprintf(Buffer3," %02d023\n",(datetime.tm_sec+2));
														else
															sprintf(Buffer3,"%s,MOTOR3 OFF BECAUSE OF SUMP TANK EMPTY BY %03d%% ",Buffer6,level_percent);
													}

												 // case STATE_MOTOR3_ON_SWITCH_TRIP_SMS:
												 else if(nSTATE_MOTOR3_SMS == STATE_MOTOR3_ON_SWITCH_TRIP_SMS)
													 {
														 if(Appmodeon==1)
															 sprintf(Buffer3," %02d024\n",(datetime.tm_sec+2));
														 else
															 sprintf(Buffer3,"%s,MOTOR3 OFF BECAUSE MOBILE AUTO SWITCH",Buffer6);
													 }
													// break; 
												//case STATE_MOTOR3_DRYRUN_TRIP_SMS:
												else if(nSTATE_MOTOR3_SMS == STATE_MOTOR3_DRYRUN_TRIP_SMS)
													{
														sprintf(Buffer3,"%s,MOTOR3 OFF BECAUSE DRY RUN",Buffer6);
														if(WorkingOn3Phas == 1)
														{

															if(Appmodeon==1)
																sprintf(Buffer3," %02d025\n",(datetime.tm_sec+2));
															else
															{
															//FloatroString1Dig(TpS1tr,s_nTimerSettings.m_DrAmpsII[l_pumpno]I);
																sprintf(Buffer3,"%s WITH 3 PHASE\n",Buffer3);
															}
															sprintf(Buffer3,"%s_WhichPhase=%d_SET AMPS = %02.01f_TripCurrent=%02.01f\n",Buffer3,WhichPhaseHaveingProblem,s_nTimerSettings.m_DrAmpsIII[3],TripCurrent);
														}
														else
														{
															if(Appmodeon==1)
																sprintf(Buffer3," %02d026\n",(datetime.tm_sec+2));
															else
															{
															//FloatroString1Dig(TpS1tr,s_nTimerSettings.m_DrAmpsII[l_pumpno]);
																sprintf(Buffer3,"%s WITH 2 PHASE\n",Buffer3);
															}
															sprintf(Buffer3,"%s_WhichPhase=%d_SET AMPS = %02.01f_TripCurrent=%02.01f\n",Buffer3,WhichPhaseHaveingProblem,s_nTimerSettings.m_DrAmpsII[3],TripCurrent);
														}
														if(WhichPhaseHaveingProblem == 1)
														{
														//FloatroString1Dig(TpS1tr,TripCurrent);
															sprintf(Buffer3,"%s_RC = %02.01f\n",Buffer3,TripCurrent);
														}
														else if(WhichPhaseHaveingProblem == 2)
														{
														//FloatroString1Dig(TpS1tr,TripCurrent);
															sprintf(Buffer3,"%s_YC = %02.01f\n",Buffer3,TripCurrent);
														}
														else if(WhichPhaseHaveingProblem == 3)

														{
														//FloatroString1Dig(TpS1tr,TripCurrent); //check in old
															sprintf(Buffer3,"%s_BC = %0.1f\n",Buffer3,TripCurrent);
														}

													}
													//break;
												//case STATE_MOTOR3_CYCLIC_TRIP_SMS:
												else if(nSTATE_MOTOR3_SMS == STATE_MOTOR3_CYCLIC_TRIP_SMS)
												{
													sAPI_UartPrintf("\n M3 CYCLIC OFF GENERATED\n\r");
													if(Appmodeon==1)
														sprintf(Buffer3," %02d027\n",(datetime.tm_sec+2));
													else												
														sprintf(Buffer3,"%s,MOTOR3 OFF BECAUSE CYCLIC OFF TIME",Buffer6);
												}
													//break;
												//case STATE_MOTOR3_MAX_TRIP_SMS:
												else if(nSTATE_MOTOR3_SMS == STATE_MOTOR3_MAX_TRIP_SMS)
												{
													if(Appmodeon==1)
														sprintf(Buffer3," %02d104\n",(datetime.tm_sec+2));
													else
														sprintf(Buffer3,"%s,MOTOR3 OFF BECAUSE MAX TIMER REACHED",Buffer6);
												}
													//break;
												/* case STATE_MOTOR3_VCYCCOMPLE_TRIP_SMS:
													//Ql_sprintf(Buffer3,"MOTOR3 OFF BECAUSE VALVE CYCLE COMPLETED AND VALVE CYCLE RESTART OFF");
													if(Appmodeon==1)
														sprintf(Buffer3," %02d115\n",(datetime.tm_sec+2));
													else
														sprintf(Buffer3,"MOTOR3 OFF BECAUSE ZONE CYCLE COMPLETED ");
													sprintf(Buffer3,"%sa_dripcyclecount[l_pumpno]=%03d\ndripdaycount=%03d\n ",Buffer3,a_dripcyclecount[l_pumpno],a_dripcycledate[l_pumpno]);

													break;  */

													 //case STATE_MOTOR_NOCOMMUNICATION_TRIP_SMS:
													else if(nSTATE_MOTOR3_SMS == STATE_MOTOR_NOCOMMUNICATION_TRIP_SMS)
													{
													
													sprintf(Buffer3," %02d124\n",(datetime.tm_sec+2));

													sprintf(Buffer3,"%s,%s,MOTOR OFF BECAUSE NO COMMUNICATION TO VALVES",Buffer3,Buffer6 );
													}

													//break; 


												/* case STATE_MOTOR_GRTCOF_TRIP_SMS:
													if(Appmodeon==1)
													sprintf(Buffer3," %02d029\n",(datetime.tm_sec+2));
													else
													sprintf(Buffer3,"MOTOR OFF BECAUSE GRTC PROGRAM TIMER REACHED");
													sprintf(Buffer3,"%sPROG-%d",Buffer3,RTCOFFLAG);

													break; */

												 //case STATE_MOTOR3_RTCOF1_TRIP_SMS:
												 else if(nSTATE_MOTOR3_SMS == STATE_MOTOR3_RTCOF1_TRIP_SMS)
												 {
													if(Appmodeon==1)
														sprintf(Buffer3," %02d030\nPROG-1\n",(datetime.tm_sec+2));
													else
														sprintf(Buffer3,"%s,MOTOR3 OFF BECAUSE RTC PROGRAM 1 OFF TIMER REACHED",Buffer6);
												 }
													//break;
												//case STATE_MOTOR3_RTCOF2_TRIP_SMS:
												else if(nSTATE_MOTOR3_SMS == STATE_MOTOR3_RTCOF2_TRIP_SMS)
												{
													if(Appmodeon==1)
														sprintf(Buffer3," %02d030\nPROG-2\n",(datetime.tm_sec+2));
													else
														sprintf(Buffer3,"%s,MOTOR3 OFF BECAUSE RTC PROGRAM 2 OFF TIMER REACHED",Buffer6);
												}
													//break;
												//case STATE_MOTOR3_RTCOF3_TRIP_SMS:
												else if(nSTATE_MOTOR3_SMS == STATE_MOTOR3_RTCOF3_TRIP_SMS)
												{
													if(Appmodeon==1)
														sprintf(Buffer3," %02d030\nPROG-3\n",(datetime.tm_sec+2));
													else
														sprintf(Buffer3,"%s,MOTOR3 OFF BECAUSE RTC PROGRAM 3 OFF TIMER REACHED",Buffer6);
												}
													//break;
												//case STATE_MOTOR3_RTCOF4_TRIP_SMS:
												else if(nSTATE_MOTOR3_SMS == STATE_MOTOR3_RTCOF4_TRIP_SMS)
												{
													if(Appmodeon==1)
														sprintf(Buffer3," %02d030\nPROG-4\n",(datetime.tm_sec+2));
													else
														sprintf(Buffer3,"%s,MOTOR3 OFF BECAUSE RTC PROGRAM 4 OFF TIMER REACHED",Buffer6);
												}
													//break;
												//case STATE_MOTOR3_RTCOF5_TRIP_SMS:
												else if(nSTATE_MOTOR3_SMS == STATE_MOTOR3_RTCOF5_TRIP_SMS)
												{
													if(Appmodeon==1)
														sprintf(Buffer3," %02d030\nPROG-5\n",(datetime.tm_sec+2));
													else
														sprintf(Buffer3,"%s,MOTOR3 OFF BECAUSE RTC PROGRAM 5 OFF TIMER REACHED",Buffer6);
												}
													//break; 
													
												//case STATE_MOTOR3_RTCOF6_TRIP_SMS:
												else if(nSTATE_MOTOR3_SMS == STATE_MOTOR3_RTCOF6_TRIP_SMS)
												{
													if(Appmodeon==1)
														sprintf(Buffer3," %02d030\nPROG-6\n",(datetime.tm_sec+2));
													else
														sprintf(Buffer3,"%s,MOTOR3 OFF BECAUSE RTC PROGRAM 6 OFF TIMER REACHED",Buffer6);
												}
													//break; 
												//case STATE_MOTOR3_TRIP_OVERLOAD_SMS:
												else if(nSTATE_MOTOR3_SMS == STATE_MOTOR3_TRIP_OVERLOAD_SMS)
													{
														sprintf(Buffer3,"%s,MOTOR3 OFF BECAUSE OVER LOAD",Buffer6);
														if(WorkingOn3Phas == 1)
														{

															if(Appmodeon==1)
																sprintf(Buffer3," %02d105\n",(datetime.tm_sec+2));
															else
															{
															//	FloatroString1Dig(TpS1tr,s_nTimerSettings.m_DrAmpsII[l_pumpno]);
																sprintf(Buffer3,"%s WITH 3 PHASE\n",Buffer3);
															}
															sprintf(Buffer3,"%s_WhichPhase=%d_SET AMPS = %02.01f_TripCurrent=%02.01f\n",Buffer3,WhichPhaseHaveingProblem,s_nTimerSettings.m_OlAmpsIII[3],TripCurrent);

														}
														else
														{

															if(Appmodeon==1)
																sprintf(Buffer3," %02d106\n",(datetime.tm_sec+2));
															else
															{
														//	FloatroString1Dig(TpS1tr,s_nTimerSettings.m_DrAmpsII[l_pumpno]);
																sprintf(Buffer3,"%s WITH 2 PHASE\n",Buffer3);
															}
															sprintf(Buffer3,"%s_WhichPhase=%d_SET AMPS = %02.01f_TripCurrent=%02.01f\n",Buffer3,WhichPhaseHaveingProblem,s_nTimerSettings.m_OlAmpsII[3],TripCurrent);
														}
														if(WhichPhaseHaveingProblem == 1)
														{
													//	FloatroString1Dig(TpS1tr,TripCurrent);
															sprintf(Buffer3,"%s_RC = %0.1f\n",Buffer3,TripCurrent);
														}
														else if(WhichPhaseHaveingProblem == 2)
														{
													//	FloatroString1Dig(TpS1tr,TripCurrent);
															sprintf(Buffer3,"%s_YC = %0.1f\n",Buffer3,TripCurrent);
														}
														else if(WhichPhaseHaveingProblem == 3)
														{
													//	FloatroString1Dig(TpS1tr,TripCurrent);
															sprintf(Buffer3,"%s_BC = %0.1f\n",Buffer3,TripCurrent);
														}
													
													}
													//break;
													//case STATE_MOTOR3_TRIP_HIGHPRESS_SMS:  //oro_doubt
													else if(nSTATE_MOTOR3_SMS == STATE_MOTOR3_TRIP_HIGHPRESS_SMS)
													{
														sprintf(Buffer3,"%s,MOTOR3 OFF BECAUSE PRESSURE SENSOR HIGH",Buffer6);
														if(WorkingOn3Phas == 1)
														{

															if(Appmodeon==1)
																sprintf(Buffer3," %02d119\n",(datetime.tm_sec+2));
															else
															{
														//	FloatroString1Dig(TpS1tr,s_nTimerSettings.m_DrAmpsII[l_pumpno]);
																sprintf(Buffer3,"%s WITH 3 PHASE\n",Buffer3);
															}
															sprintf(Buffer3,"%s_SET HIPRESSURE = %0.1f\n",Buffer3,s_nTimerSettings.m_highpress[l_pumpno]);

														}
														else
														{

															if(Appmodeon==1)
																sprintf(Buffer3," %02d120\n",(datetime.tm_sec+2));
															else
															{
														//	FloatroString1Dig(TpS1tr,s_nTimerSettings.m_DrAmpsII[l_pumpno]);
																sprintf(Buffer3,"%s WITH 2 PHASE\n",Buffer3);
															}
															sprintf(Buffer3,"%s_SET HIPRESSURE = %0.1f\n",Buffer3,s_nTimerSettings.m_highpress);
														}
														sprintf(Buffer3,"%s_TRIP HIPRESSURE = %0.1f\n",Buffer3,Trippressure);

													}

													//break;
													//case STATE_MOTOR3_TRIP_LOWPRESS_SMS:
													else if(nSTATE_MOTOR3_SMS == STATE_MOTOR3_TRIP_LOWPRESS_SMS)
													{
														sprintf(Buffer3,"MOTOR3 OFF BECAUSE PRESSURE SENSOR LOW");
														if(WorkingOn3Phas == 1)
														{

															if(Appmodeon==1)
																sprintf(Buffer3," %02d121\n",(datetime.tm_sec+2));
															else
															{
														//	FloatroString1Dig(TpS1tr,s_nTimerSettings.m_DrAmpsII[l_pumpno]);
																sprintf(Buffer3,"%s WITH 3 PHASE\n",Buffer3);
															}
															sprintf(Buffer3,"%s_SET LOWPRESSURE = %0.1f\n",Buffer3,s_nTimerSettings.m_lowpress[l_pumpno]);

														}
														else
														{

															if(Appmodeon==1)
																sprintf(Buffer3," %02d122\n",(datetime.tm_sec+2));
															else
															{
														//	FloatroString1Dig(TpS1tr,s_nTimerSettings.m_DrAmpsII[l_pumpno]);
																sprintf(Buffer3,"%s WITH 2 PHASE\n",Buffer3);
															}
															sprintf(Buffer3,"%s_SET LOWPRESSURE = %0.1f\n",Buffer3,s_nTimerSettings.m_lowpress[l_pumpno]);
														}
														sprintf(Buffer3,"%s_TRIP LOWPRESSURE = %0.1f\n",Buffer3,Trippressure);

													}

													//break;  
												//case STATE_MOTOR3_TRIP_SPP_SMS:
												else if(nSTATE_MOTOR3_SMS == STATE_MOTOR3_TRIP_SPP_SMS)
													{
													 if(Appmodeon==1)
														sprintf(Buffer3," %02d107\n",(datetime.tm_sec+2));
													else
														sprintf(Buffer3,"%s,MOTOR3 OFF BECAUSE SINGLE PHASEING\n",Buffer6);
														sprintf(Buffer3,"%s_IMB VOLT = %03d\n_SET IMB VOLT = %03d",Buffer3,VSPPImbalanceVoltage,s_nTimerSettings.m_ImbVolt);

													}
													//break;
												//case STATE_MOTOR3_TRIP_REVERSEPHASE_SMS:  //oro_doubt
												else if(nSTATE_MOTOR3_SMS == STATE_MOTOR3_TRIP_REVERSEPHASE_SMS)
													{
													if(Appmodeon==1)
														sprintf(Buffer3," %02d108\n",(datetime.tm_sec+2));
													else
														sprintf(Buffer3,"%s,MOTOR3 OFF BECAUSE REVERSE PHASE",Buffer6);
													}
													//break;
												//case STATE_MOTOR3_TRIP_CURRENTSPP_SMS:
												else if(nSTATE_MOTOR3_SMS == STATE_MOTOR3_TRIP_CURRENTSPP_SMS)
													{
														sprintf(Buffer3,"%s,MOTOR3 OFF BECAUSE CURRENT SENSEING SPP\n",Buffer6);
														if(Appmodeon==1)
														sprintf(Buffer3," %02d109\n",(datetime.tm_sec+2));

														switch(CSPPValue)
														{
															case 1:
																sprintf(Buffer3,"%s_RC LESS\n",Buffer3);
																break;
															case 2:
																sprintf(Buffer3,"%s_YC LESS\n",Buffer3);
																break;
															case 3:
																sprintf(Buffer3,"%s_BC LESS\n",Buffer3);
																break;
															case 4:
																sprintf(Buffer3,"%s_RC AND YC BOTH DIFF MORE THAN 50%\n",Buffer3);
																break;
															case 5:
																sprintf(Buffer3,"%s,YC AND BC BOTH DIFF MORE THAN 50%\n",Buffer3);
																break;
															default:
																sprintf(Buffer3,"%s_YC AND BC BOTH DIFF MORE THAN 50%\n",Buffer3);;
																break;

														}
													}
													//break;
												//case STATE_MOTOR3_TRIP_LOWVOLTAGE_SMS:
												else if(nSTATE_MOTOR3_SMS == STATE_MOTOR3_TRIP_LOWVOLTAGE_SMS)
													{
															sprintf(Buffer3,"%s,MOTOR3 OFF LOW VOLT",Buffer6);
															
															if(WorkingOn3Phas == 1)
															{
																if(Appmodeon==1)
																{
																	sprintf(Buffer3,"%02d110\n",(datetime.tm_sec+2));
																/* else
																{ */

																	if(WhichPhaseHaveingProblem == 1)
																	{
																	//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nRY VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltIII,nTimerSettings.DiffVoltIII,TripVoltage);
																	sprintf(Buffer3,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_RY VOLT = %03d\n",Buffer3,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltIII,s_nTimerSettings.m_DiffVoltIII,TripVoltage);			
																	}
																	else if(WhichPhaseHaveingProblem == 2)
																	{
																		//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nYB VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltIII,nTimerSettings.DiffVoltIII,TripVoltage);
																		sprintf(Buffer3,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_RY VOLT = %03d\n",Buffer3,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltIII,s_nTimerSettings.m_DiffVoltIII,TripVoltage);
																	}
																	else if(WhichPhaseHaveingProblem == 3)
																	{
																		//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nBR VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltIII,nTimerSettings.DiffVoltIII,TripVoltage);
																		sprintf(Buffer3,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_RY VOLT = %03d\n",Buffer3,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltIII,s_nTimerSettings.m_DiffVoltIII,TripVoltage);		
																	}						
																	else
																	{
																		sprintf(Buffer3,"No Voltage\n");
																	}
																}
																else
																{

																	if(WhichPhaseHaveingProblem == 1)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 3PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nRY VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltIII,nTimerSettings.DiffVoltIII,TripVoltage);
																		sprintf(Buffer3,"%s RUNING AT 3PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_RY VOLT = %03d\n",Buffer3,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltIII,s_nTimerSettings.m_DiffVoltIII,TripVoltage);		
																	}
																	else if(WhichPhaseHaveingProblem == 2)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 3PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nYB VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltIII,nTimerSettings.DiffVoltIII,TripVoltage);
																		sprintf(Buffer3,"%s RUNING AT 3PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_YB VOLT = %03d\n",Buffer3,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltIII,s_nTimerSettings.m_DiffVoltIII,TripVoltage);
																	}
																	else if(WhichPhaseHaveingProblem == 3)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 3PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nBR VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltIII,nTimerSettings.DiffVoltIII,TripVoltage);
																		sprintf(Buffer3,"%s RUNING AT 3PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_BR VOLT = %03d\n",Buffer3,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltIII,s_nTimerSettings.m_DiffVoltIII,TripVoltage);		
																	}						
																	else
																	{
																		sprintf(Buffer3,"No Voltage\n");
																	}
																}
																sprintf(buf,"which phase :%d set low voltIII :%03d trip voltIII :%03d",WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltIII,TripVoltage);
															sAPI_UartPrintf(buf);
															}
															else
															{
																if(Appmodeon==1)
																{
																	sprintf(Buffer3,"%02d111\n",(datetime.tm_sec+2));
																	if(WhichPhaseHaveingProblem == 1)
																	{
																		//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nRY VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltII,nTimerSettings.DiffVoltII,TripVoltage);
																		sprintf(Buffer3,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_RY VOLT = %03d\n",Buffer3,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltII,s_nTimerSettings.m_DiffVoltII,TripVoltage);
																	}
																	else if(WhichPhaseHaveingProblem == 2)
																	{
																		//sprintf(Buffer1,"%s2PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nYB VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltII,nTimerSettings.DiffVoltII,TripVoltage);
																		sprintf(Buffer3,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_YB VOLT = %03d\n",Buffer3,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltII,s_nTimerSettings.m_DiffVoltII,TripVoltage);		
																	}
																	else if(WhichPhaseHaveingProblem == 3)
																	{
																		//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nBR VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltII,nTimerSettings.DiffVoltII,TripVoltage);
																		sprintf(Buffer3,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_BR VOLT = %03d\n",Buffer3,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltII,s_nTimerSettings.m_DiffVoltII,TripVoltage);
																	}						
																	else
																	{
																		sprintf(Buffer3,"No Voltage\n");
																	}
																}
																else
																{

																	if(WhichPhaseHaveingProblem == 1)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 2PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nRY VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltII,nTimerSettings.DiffVoltII,TripVoltage);
																		sprintf(Buffer3,"%s RUNING AT 2PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_RY VOLT = %03d\n",Buffer3,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltII,s_nTimerSettings.m_DiffVoltII,TripVoltage);		
																	}
																	else if(WhichPhaseHaveingProblem == 2)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 2PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nYB VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltII,nTimerSettings.DiffVoltII,TripVoltage);
																		sprintf(Buffer3,"%s RUNING AT 2PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_YB VOLT = %03d\n",Buffer3,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltII,s_nTimerSettings.m_DiffVoltII,TripVoltage);
																	}
																	else if(WhichPhaseHaveingProblem == 3)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 2PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nBR VOLT = %03.0f\n",Buffer1,
																				//s_nTimerSettings.m_LowVoltII,nTimerSettings.DiffVoltII,TripVoltage);
																		sprintf(Buffer3,"%s RUNING AT 2PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_BR VOLT = %03d\n",Buffer3,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_LowVoltII,s_nTimerSettings.m_DiffVoltII,TripVoltage);
																	}						
																	else
																	{
																		sprintf(Buffer3,"No Voltage\n");
																	}
																}


															}
														}
													//break;
												
												//case STATE_MOTOR3_TRIP_HIGHVOLTAGE_SMS:
												else if(nSTATE_MOTOR3_SMS == STATE_MOTOR3_TRIP_HIGHVOLTAGE_SMS)
												   {
															sprintf(Buffer3,"%s,MOTOR3 OFF HIGH VOLT",Buffer6);
															if(WorkingOn3Phas == 1)
															{
																if(Appmodeon==1)
																{
																	sprintf(Buffer3,"%02d112\n",(datetime.tm_sec+2));
																	if(WhichPhaseHaveingProblem == 1)
																	{
																		//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nRY VOLT = %03.0f\n",Buffer1,
																				//nTimerSettings.HighVoltIII,nTimerSettings.HiDiffVoltIII,TripVoltage);
																		sprintf(Buffer3,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_RY VOLT = %03d\n",Buffer3,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltIII,s_nTimerSettings.m_HiDiffVoltIII,TripVoltage);		
																	}
																	else if(WhichPhaseHaveingProblem == 2)
																	{
																		//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nYB VOLT = %03.0f\n",Buffer1,
																				//nTimerSettings.HighVoltIII,nTimerSettings.HiDiffVoltIII,TripVoltage);
																		sprintf(Buffer3,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_YB VOLT = %03d\n",Buffer3,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltIII,s_nTimerSettings.m_HiDiffVoltIII,TripVoltage);
																	}
																	else if(WhichPhaseHaveingProblem == 3)
																	{
																		//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nBR VOLT = %03.0f\n",Buffer1,
																				//nTimerSettings.HighVoltIII,nTimerSettings.HiDiffVoltIII,TripVoltage);
																		sprintf(Buffer3,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_BR VOLT = %03d\n",Buffer3,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltIII,s_nTimerSettings.m_HiDiffVoltIII,TripVoltage);
																	}						
																	else
																	{
																		sprintf(Buffer3,"No Voltage\n");
																	}
																}
																
																
																else
																{

																	if(WhichPhaseHaveingProblem == 1)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 3PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nRY VOLT = %03.0f\n",Buffer1,
																				//nTimerSettings.HighVoltIII,nTimerSettings.HiDiffVoltIII,TripVoltage);
																		sprintf(Buffer3,"%s RUNING AT 3PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_RY VOLT = %03d\n",Buffer3,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltIII,s_nTimerSettings.m_HiDiffVoltIII,TripVoltage);		
																	}
																	else if(WhichPhaseHaveingProblem == 2)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 3PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nYB VOLT = %03.0f\n",Buffer1,
																				//nTimerSettings.HighVoltIII,nTimerSettings.HiDiffVoltIII,TripVoltage);
																		sprintf(Buffer3,"%s RUNING AT 3PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_YB VOLT = %03d\n",Buffer3,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltIII,s_nTimerSettings.m_HiDiffVoltIII,TripVoltage);		
																	}
																	else if(WhichPhaseHaveingProblem == 3)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 3PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nBR VOLT = %03.0f\n",Buffer1,
																				//nTimerSettings.HighVoltIII,nTimerSettings.HiDiffVoltIII,TripVoltage);
																		sprintf(Buffer3,"%s RUNING AT 3PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_BR VOLT = %03d\n",Buffer3,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltIII,s_nTimerSettings.m_HiDiffVoltIII,TripVoltage);		
																	}						
																	else
																	{
																		sprintf(Buffer3,"No Voltage\n");
																	}
																}
																sprintf(buf,"_which phase :%d_set high voltIII :%03d_trip voltII :%03d",WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltIII,TripVoltage);
															sAPI_UartPrintf(buf);
															}
															else
															{
																if(Appmodeon==1)
																{
																	sprintf(Buffer3,"%02d113\n",(datetime.tm_sec+2));
																	if(WhichPhaseHaveingProblem == 1)
																	{
																		//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nRY VOLT = %03.0f\n",
																				//nTimerSettings.HighVoltII,nTimerSettings.HiDiffVoltII,TripVoltage);
																		sprintf(Buffer3,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_RY VOLT = %03d\n",Buffer3,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltII,s_nTimerSettings.m_HiDiffVoltII,TripVoltage);
																	}
																	else if(WhichPhaseHaveingProblem == 2)
																	{
																		//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nYB VOLT = %03.0f\n",
																				//nTimerSettings.HighVoltII,nTimerSettings.HiDiffVoltII,TripVoltage);
																		sprintf(Buffer3,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_YB VOLT = %03d\n",Buffer3,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltII,s_nTimerSettings.m_HiDiffVoltII,TripVoltage);
																	}
																	else if(WhichPhaseHaveingProblem == 3)
																	{
																		//sprintf(Buffer1,"%s\nSET VOLT = %03d\n  DIFF VOLT = %03d\nBR VOLT = %03.0f\n",
																				//nTimerSettings.HighVoltII,nTimerSettings.HiDiffVoltII,TripVoltage);
																		sprintf(Buffer3,"%s\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_BR VOLT = %03d\n",Buffer3,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltII,s_nTimerSettings.m_HiDiffVoltII,TripVoltage);
																	}						
																	else
																	{
																		sprintf(Buffer3,"No Voltage\n");
																	}
																	
																}
																else
																{

																	if(WhichPhaseHaveingProblem == 1)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 2PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nRY VOLT = %03.0f\n",
																				//nTimerSettings.HighVoltII,nTimerSettings.HiDiffVoltII,TripVoltage);
																		sprintf(Buffer3,"%s RUNING AT 2PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_RY VOLT = %03d\n",Buffer3,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltII,s_nTimerSettings.m_HiDiffVoltII,TripVoltage);
																	}
																	else if(WhichPhaseHaveingProblem == 2)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 2PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nYB VOLT = %03.0f\n",
																				//nTimerSettings.HighVoltII,nTimerSettings.HiDiffVoltII,TripVoltage);
																		sprintf(Buffer3,"%s RUNING AT 2PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_YB VOLT = %03d\n",Buffer3,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltII,s_nTimerSettings.m_HiDiffVoltII,TripVoltage);
																	}
																	else if(WhichPhaseHaveingProblem == 3)
																	{
																		//sprintf(Buffer1,"%sRUNING AT 2PH\nSET VOLT = %03d\n  DIFF VOLT = %03d\nBR VOLT = %03.0f\n",
																				//nTimerSettings.HighVoltII,nTimerSettings.HiDiffVoltII,TripVoltage);
																		sprintf(Buffer3,"%s RUNING AT 2PH\n_WHICH PHASE %d\n_SET VOLT = %03d\n_DIFF VOLT = %03d\n_BR VOLT = %03d\n",Buffer3,
																				WhichPhaseHaveingProblem,s_nTimerSettings.m_HighVoltII,s_nTimerSettings.m_HiDiffVoltII,TripVoltage);
																	}						
																	else
																	{
																		sprintf(Buffer3,"No Voltage\n");
																	}
																}


															}
														}
												
												
												
												

													//break;
												/* case STATE_MOTOR3OF_3PHASE_ONLY_SMS://oro_doubt
												{
													sprintf(Buffer3,"MOTOR3 CAN'T START BEACUSE 3 PHASE SELECTION ONLY\n");
												}
												break; */
												/* case STATE_STGIII_SMS1:    //oro_doubt
													{
														sprintf(Buffer3,"3 Phase Current Set\n");
														if(nMSettings.DryRunOnOff == 1)
															sprintf(Buffer3,"%sDRYRUN SCAN ON\n",Buffer3);
														else
															sprintf(Buffer3,"%sDRYRUN SCAN OFF\n",Buffer3);
															FloatroString1Dig(TpS1tr,nTimerSettings.DrAmpsIII);
														sprintf(Buffer3,"%sDRY RUN AMPS FOR 3 PHASE = %s\n",Buffer3,TpS1tr);

														if(nTimerSettings.OlOnOff == 1)
															sprintf(Buffer3,"%sOVER LOAD SCAN ON\n",Buffer3);
														else
															sprintf(Buffer3,"%sOVER LOAD SCAN OFF",Buffer3);
														FloatroString1Dig(TpS1tr,nTimerSettings.OlAmpsIII);
														sprintf(Buffer3,"%sOVER LOAD AMPS FOR 3 PHASE = %s\n",Buffer3,TpS1tr);


													//	sprintf(Buffer3,"%sDRY RUN SCAN TIME = %02d:%02d:%02d\n",Buffer3,s_nTimerSettings.m_DrScHr,s_nTimerSettings.DrScMin,s_nTimerSettings.DrScSec);
													//	sprintf(Buffer3,"%sOVER LOAD SCAN TIME = %02d:%02d:%02d\n",Buffer3,s_nTimerSettings.OlScanHr,s_nTimerSettings.OlScanMin,s_nTimerSettings.OlScanSec);
													}
													break;
												case STATE_STGII_SMS1:
													{
														sprintf(Buffer3,"2 Phase Current Set\n");
														if(nMSettings.DryRunOnOff == 1)
															sprintf(Buffer3,"%sDRYRUN SCAN ON\n",Buffer3);
														else
															sprintf(Buffer3,"%sDRYRUN SCAN OFF\n",Buffer3);
															FloatroString1Dig(TpS1tr,nTimerSettings.DrAmpsII);
														sprintf(Buffer3,"%sDRY RUN AMPS FOR 2 PHASE = %s\n",Buffer3,TpS1tr);

														if(nTimerSettings.OlOnOff == 1)
															sprintf(Buffer3,"%sOVER LOAD SCAN ON\n",Buffer3);
														else
															sprintf(Buffer3,"%sOVER LOAD SCAN OFF",Buffer3);
														 FloatroString1Dig(TpS1tr,nTimerSettings.OlAmpsIII);
														sprintf(Buffer3,"%sOVER LOAD AMPS FOR 2 PHASE = %s\n",Buffer3,TpS1tr);

													//	sprintf(Buffer3,"%sDRY RUN SCAN TIME = %02d:%02d:%02d\n",Buffer3,s_nTimerSettings.DrScHr,s_nTimerSettings.DrScMin,s_nTimerSettings.DrScSec);
													//	sprintf(Buffer3,"%sOVER LOAD SCAN TIME = %02d:%02d:%02d\n",Buffer3,s_nTimerSettings.OlScanHr,s_nTimerSettings.OlScanMin,s_nTimerSettings.OlScanSec);
													}
													break; */
												//default:	
												else
												{
													sprintf(Buffer3,"UnKnown1 Please send this to Customer Support Error : nSTATE_MOTOR3_SMS = %d",nSTATE_MOTOR3_SMS);

												}
												//break;
												sprintf(buf,"\n sms_offlag2=%d,trip_flag2=%d\n",sms_offlag2,trip_flag2);
												sAPI_UartPrintf(buf);
												if((sms_offlag2 == 1 && trip_flag2 == 0) || (sms_offlag2 == 0 && trip_flag2 == 1)) 
													{
														sprintf(Buffer3,"%s,%02d:%02d:%02d,%02d:%02d:%02d,020",Buffer3,datetime.tm_hour,datetime.tm_min,datetime.tm_sec,Time_Hr2,Time_Min2,Time_Sec2);
														memset(Buffer6,0,500);
														/* if(on_data_M3 == 0)
																	sprintf(s_nlogtime[on_data_M3].Act_Mtr3_On,",LOG,F=%02d:%02d:%02d",datetime.tm_hour,datetime.tm_min,datetime.tm_sec);
																else
																	sprintf(s_nlogtime[on_data_M3].Act_Mtr3_On,"%s,F=%02d:%02d:%02d",s_nlogtime[on_data_M3].Act_Mtr3_On,datetime.tm_hour,datetime.tm_min,datetime.tm_sec);

																	on_data_M3++;
																	for(char ij=0;ij<on_data_M3;ij++)
																	{
																	sprintf(Buffer3,"%s%s",Buffer3,s_nlogtime[ij].Act_Mtr3_On);
																	}
																	
																	
																	sprintf(buf,"\n M3_F; on_data_M3: %d\n ",on_data_M3);
																	sAPI_UartPrintf(buf);
																	
															if(on_data_M3>22)
																on_data_M3=0; */ 
															
													}

											}
											
										  if(nSTATE_MOTOR3_ON_SMS != STATE_MOTOR3_DUMMY)
											{
												sprintf(buf,"nSTATE_MOTOR3_ON_SMS :%d %d\n\r",nSTATE_MOTOR3_ON_SMS,STATE_MOTOR3_ON_SWITCH_SMS);
												sAPI_UartPrintf(buf);
												
												sprintf(buf,"\n\r WorkingOn3Phas = %01d\n\r",WorkingOn3Phas);
												sAPI_UartPrintf(buf);
												
											//switch(nSTATE_MOTOR3_ON_SMS)
											//{
												//case STATE_MOTOR3ON_SMS:
													//{
														
														if(nSTATE_MOTOR3_ON_SMS==STATE_MOTOR3_ON_DEFAULT)
														{

															if(PowerCurrentCondition == 0)
															{

																 sprintf(Buffer6,"AUTO ON \nPOWER ON\nMOTOR3 ON\n");
															//	 switchonofflag=1;


																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer6," %02d001\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer6,"%s WITH 3 PHASE",Buffer6);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer6," %02d002\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer6,"%s WITH 2 PHASE",Buffer6);
																}
															}
															else
															{
																   if(Appmodeon==1)
																	sprintf(Buffer6," %02d003\n",(datetime.tm_sec+2));
																	else
																	sprintf(Buffer6,"AUTO ON \nPOWER OFF\nMOTOR3 OFF\n");
															//	 switchonofflag=1;

																//sprintf(Buffer6,"MOBILE ON BY %s\nPOWER OFF\nMOTOR OFF\n",WhoMadeRelayOn);
															}
														}
														
														else if(nSTATE_MOTOR3_ON_SMS==STATE_MOTOR3_ON_ONDELAY)
														{
																sprintf(Buffer6,"MOTOR3 ON BECAUSE OF CYCLIC ON TIME");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer6," %02d004\n",(datetime.tm_sec+2));
																
																	else
																		sprintf(Buffer6,"%s WITH 3 PHASE",Buffer6);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer6," %02d005\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer6,"%s WITH 2 PHASE",Buffer6);
																}
														}
													
														else if(nSTATE_MOTOR3_ON_SMS==STATE_MOTOR3_ON_SWITCH_SMS)
														{
															sprintf(buf,"nSTATE_MOTOR3_ON_SMS :%d %d\n\r",nSTATE_MOTOR3_ON_SMS,STATE_MOTOR3_ON_SWITCH_SMS);
															sAPI_UartPrintf(buf);
															
																sprintf(Buffer6,"MOTOR3 ON BECAUSE AUTO MOBILE SWITCH");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																	{
																		sprintf(Buffer6," %02d006\n",(datetime.tm_sec+2));
																		sAPI_UartPrintf(Buffer6);
																	}
																	else
																	{
																	sprintf(Buffer6,"%s WITH 3 PHASE",Buffer6);
																	sAPI_UartPrintf(Buffer6);
																	}
																}
																else
																{
																	if(Appmodeon==1)
									
																		sprintf(Buffer6," %02d007\n",(datetime.tm_sec+2));
																	else{
																		sprintf(Buffer6,"%s WITH 2 PHASE",Buffer6);
																	sAPI_UartPrintf(Buffer6);}
																}
														}

														else if(nSTATE_MOTOR3_ON_SMS==STATE_MOTOR3_ON_RTC1)
														{
																sprintf(Buffer6,"MOTOR3 ON BECAUSE OF ON RTC PROGRAM 1 ON TIME");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer6," %02d010\nPROG-1\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer6,"%s WITH 3 PHASE",Buffer6);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer6," %02d011\nPROG-1\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer6,"%s WITH 2 PHASE",Buffer6);
																}
														}
													
														else if(nSTATE_MOTOR3_ON_SMS==STATE_MOTOR3_ON_RTC2)
														{
																sprintf(Buffer6,"MOTOR3 ON BECAUSE OF ON RTC PROGRAM 2 ON TIME");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer6," %02d010\nPROG-2\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer6,"%s WITH 3 PHASE",Buffer6);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer6," %02d011\nPROG-2\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer6,"%s WITH 3 PHASE",Buffer6);
																}
														}

														else if(nSTATE_MOTOR3_ON_SMS==STATE_MOTOR3_ON_RTC3)	
														{
																sprintf(Buffer6,"MOTOR3 ON BECAUSE OF ON RTC PROGRAM 3 ON TIME");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer6," %02d010\nPROG-3\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer6,"%s WITH 3 PHASE",Buffer6);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer6," %02d011\nPROG-3\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer6,"%s WITH 2 PHASE",Buffer6);
																}
														}
								
														else if(nSTATE_MOTOR3_ON_SMS==STATE_MOTOR3_ON_RTC4)	
														{
																sprintf(Buffer6,"MOTOR3 ON BECAUSE OF ON RTC PROGRAM 4 ON TIME");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer6," %02d010\nPROG-4\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer6,"%s WITH 3 PHASE",Buffer6);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer6," %02d011\nPROG-4\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer6,"%s WITH 2 PHASE",Buffer6);
																}
														}
														else if(nSTATE_MOTOR3_ON_SMS==STATE_MOTOR3_ON_RTC5)	
														{
																sprintf(Buffer6,"MOTOR3 ON BECAUSE OF ON RTC PROGRAM 5 ON TIME");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer6," %02d010\nPROG-5\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer6,"%s WITH 3 PHASE",Buffer6);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer6," %02d011\nPROG-5\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer6,"%s WITH 2 PHASE",Buffer6);
																}
														}
														else if(nSTATE_MOTOR3_ON_SMS==STATE_MOTOR3_ON_RTC6)	
														{
																sprintf(Buffer6,"MOTOR3 ON BECAUSE OF ON RTC PROGRAM 4 ON TIME");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer6," %02d010\nPROG-6\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer6,"%s WITH 3 PHASE",Buffer6);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer6," %02d011\nPROG-6\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer6,"%s WITH 2 PHASE",Buffer6);
																}
														}
								
														else if(nSTATE_MOTOR3_ON_SMS==STATE_MOTOR3_ON_UPPERTANK)
														{
																sprintf(Buffer6,"MOTOR3 ON BECAUSE OF UPPER TANK EMPTY");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer6," %02d012\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer6,"%s WITH 3 PHASE",Buffer6);
																	sAPI_UartPrintf(Buffer6);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer6," %02d013\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer6,"%s WITH 2 PHASE",Buffer6);
																	sAPI_UartPrintf(Buffer6);
																}
														}
												
														else if(nSTATE_MOTOR3_ON_SMS==STATE_MOTOR3_ON_LOWERTANK)	
														{
																sprintf(Buffer6,"MOTOR3 ON BECAUSE OF SUMP TANK FULL");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer6," %02d014\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer6,"%s WITH 3 PHASE",Buffer6);
																	sAPI_UartPrintf(Buffer6);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer6," %02d015\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer6,"%s WITH 2 PHASE",Buffer6);
																	sAPI_UartPrintf(Buffer6);
																}
														}
														
														else if(nSTATE_MOTOR3_ON_SMS==STATE_MOTOR3_ON_LEVELSCAN_UPPERTANK)
														{
																sprintf(Buffer6,"MOTOR3 ON BECAUSE OF UPPER TANK EMPTY BY %03d%%",level_percent);
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer6," %02d012\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer6,"%s WITH 3 PHASE",Buffer6);
																	sAPI_UartPrintf(Buffer6);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer6," %02d013\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer6,"%s WITH 2 PHASE",Buffer6);
																	sAPI_UartPrintf(Buffer6);
																}
														}
														
														else if(nSTATE_MOTOR3_ON_SMS==STATE_MOTOR3_ON_LEVELSCAN_LOWERTANK)	
														{
																sprintf(Buffer6,"MOTOR3 ON BECAUSE OF SUMP TANK FULL BY %03d%% ",level_percent);
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer6," %02d014\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer6,"%s WITH 3 PHASE",Buffer6);
																	sAPI_UartPrintf(Buffer1);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer6," %02d015\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer6,"%s WITH 2 PHASE",Buffer6);
																	sAPI_UartPrintf(Buffer6);
																}
															}
												
														else if(nSTATE_MOTOR3_ON_SMS==STATE_MOTOR3_ON_LOWERTANK)	
														{
																sprintf(Buffer6,"MOTOR3 ON BECAUSE OF SUMP TANK FULL");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer6," %02d014\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer6,"%s WITH 3 PHASE",Buffer6);
																	sAPI_UartPrintf(Buffer6);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer6," %02d015\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer6,"%s WITH 2 PHASE",Buffer6);
																	sAPI_UartPrintf(Buffer6);
																}
														}
									
														else if(nSTATE_MOTOR3_ON_SMS==STATE_MOTOR3_ON_RESTARTTIMER)	
														{
																sprintf(Buffer6,"MOTOR3 ON BECAUSE OF RESTART TIMER");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer6," %02d016\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer6,"%s WITH 3 PHASE",Buffer6);
																	sAPI_UartPrintf(Buffer6);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer6," %02d017\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer6,"%s WITH 2 PHASE",Buffer6);
																	sAPI_UartPrintf(Buffer6);
																}
														}
										
														else if(nSTATE_MOTOR3_ON_SMS == STATE_MOTOR3_ON_TARGET)
														{
																sprintf(Buffer6,"MOTOR3 ON BECAUSE OF TARGET");
																if(WorkingOn3Phas == 1)
																{
																	if(Appmodeon==1)
																		sprintf(Buffer6," %02d018\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer6,"%s WITH 3 PHASE",Buffer6);
																		sAPI_UartPrintf(Buffer6);
																}
																else
																{
																	if(Appmodeon==1)
																		sprintf(Buffer6," %02d019\n",(datetime.tm_sec+2));
																	else
																		sprintf(Buffer6,"%s WITH 2 PHASE",Buffer6);
																		sAPI_UartPrintf(Buffer6);
																}
														}
														
														else	
														{
																sprintf(Buffer6,"Error Sednd this to Service DEPT nSTATE_MOTOR3_ON_SMS = %d",nSTATE_MOTOR3_ON_SMS);
																sAPI_UartPrintf(Buffer6);
														} 
														if(sms_onflag2 == 1)	
														{
															sprintf(buf,"\n On time entry check3\n");
															sAPI_UartPrintf(buf);
															
																sprintf(Buffer6,"%s,%02d:%02d:%02d",Buffer6,datetime.tm_hour,datetime.tm_min,datetime.tm_sec);	
																	
																/* if(on_data_M3 == 0)
																	sprintf(s_nlogtime[on_data_M3].Act_Mtr3_On,",LOG,N=%02d:%02d:%02d",datetime.tm_hour,datetime.tm_min,datetime.tm_sec);
																else
																	sprintf(s_nlogtime[on_data_M3].Act_Mtr3_On,"%s,N=%02d:%02d:%02d",s_nlogtime[on_data_M3].Act_Mtr3_On,datetime.tm_hour,datetime.tm_min,datetime.tm_sec);
																	on_data_M3++;
																	for(char ij=0;ij<on_data_M3;ij++)
																	{
																	sprintf(Buffer6,"%s%s",Buffer6,s_nlogtime[ij].Act_Mtr3_On);
																	}
																	
																	sprintf(buf,"\n On time entry3 %s \n",s_nlogtime[on_data_M3].Act_Mtr3_On);
																	sAPI_UartPrintf(buf);
																	
																	
																sprintf(buf,"\n M3_N; on_data_M3: %d\n ",on_data_M3);
																sAPI_UartPrintf(buf);
																if(on_data_M3>22)
																on_data_M3=0; */
																
														}
														sprintf(Buffer3,"%s",Buffer6);
										}

										/* else if(STATE_SENDSMS != STATE_NO_SMS)
										{
											sprintf(buf,"reference");
											sAPI_UartPrintf(buf);
											switch(STATE_SENDSMS)
											{
												 case STATE_SMSON_SMS:
												{
													if(nMSettings.SMSOnOff == 1)
														sprintf(Buffer1,"SMS ON");
													else
														sprintf(Buffer1,"SMS OFF");
												}
												break;
												case STATE_STANDZON_SMS:
												{

														sprintf(Buffer1,"STANDALONE ZONE=%d ON",nDripSettings.stchangefrom);

												}
												break;
												case STATE_LIMIT_SMSON_SMS:
												{
													if(limitsmsonof == 1)
														sprintf(Buffer1,"SMS LIMIT ON");
													else
														sprintf(Buffer1,"SMS LIMIT OFF");
												}
												break;

												case STATE_stSMSON_SMS:
												{
													if(nMSettings.staticSMSOnOff == 1)
														sprintf(Buffer1,"STATICSMS ON");
													else
														sprintf(Buffer1,"STATICSMS OFF");
												}
												break;
			                                     case STATE_canSMSON_SMS:
												{
													if(nMSettings.canSMSOnOff == 1)
														sprintf(Buffer1,"DELAY TIME VALVE ON OFF FUNCTION ON");
													else
														sprintf(Buffer1,"DELAY TIME VALVE ON OFF FUNCTION OFF");
												}
												break;
												case STATE_sampleSMSON_SMS:
												{
													if(nMSettings.sampleSMSOnOff == 1)
														sprintf(Buffer1,"SAMPLE SMS ON");
													else
														sprintf(Buffer1,"SAMPLE SMS OFF");
												}
												break;
												case STATE_dataSMSON_SMS:
												{
													if(nMSettings.dataSMSOnOff == 1)
														sprintf(Buffer1,"DATA SMS ON");
													else
														sprintf(Buffer1,"DATA SMS OFF");
												}
												break;

												case STATE_gethidesmson_SMS:
												{
													if(nMSettings.gethidesmsonoff == 1)
														sprintf(Buffer1,"NAME SMS ON");
													else
														sprintf(Buffer1,"NAME SMS OFF");
												}
												break;
												case STATE_SFB_SMS:
												{
													if(nMSettings.SfbOnOff == 1)
														sprintf(Buffer1,"STARTER FEEDBACK ON");
													else
														sprintf(Buffer1,"STARTER FEEDBACK OFF");
												}
												break;
												case STATE_PRESSURE_SMS:
												{
													if(nMSettings.PressureOnOff == 1)
														sprintf(Buffer1,"PRESSURE SCAN ON");
													else
														sprintf(Buffer1,"PRESSURE SCAN OFF");

												}
												break;
												case STATE_MANUALSWITCH_SMS:
												{
													if(nMSettings.ManualswitchOnOff == 1)
														sprintf(Buffer1,"MANUALSWITCH SCAN ON");
													else
														sprintf(Buffer1,"MANUALSWITCH SCAN OFF");

												}
												break;
												case STATE_TANK_SMS:
												{
													if(nMSettings.TankOnOff == 1)
														sprintf(Buffer1,"TANK LEVEL SCAN ON");
													else
														sprintf(Buffer1,"TANK LEVEL SCAN OFF");

												}
												break;
												case STATE_SUMP_SMS:
												{
													if(nMSettings.SumpOnOff == 1)
														sprintf(Buffer1,"SUMP LEVEL SCAN ON");
													else
														sprintf(Buffer1,"SUMP LEVEL SCAN OFF");

												}
												break;
												case STATE_SUMP_RST_SMS:
												{
													if(nMSettings.SumprstOnOff == 1)
														sprintf(Buffer1,"SUMP RESTART ON");
													else
													if(nMSettings.SumprstOnOff == 0)
														sprintf(Buffer1,"SUMP RESTART OFF");
												}
												break;
												case STATE_DRYRUN_SMS:
												{
													if(nMSettings.DryRunOnOff == 1)
														sprintf(Buffer1,"DRYRUN SCAN ON");
													else
														sprintf(Buffer1,"DRYRUN SCAN OFF");

												}
												break;
												case STATE_SECONOF_SMS:
												{
													if(nMSettings.SecOnOf == 1)
														sprintf(Buffer1,"SECURITY ON");
													else
														sprintf(Buffer1,"SECURITY OFF");

												}
												break;
												case STATE_TARGET_SMS:
												{
													if(nMSettings.TargetOnOff == 1)
														sprintf(Buffer1,"TARGET CALL ON");
													else
														sprintf(Buffer1,"TARGET CALL OFF");
												}
												break;
												case STATE_PWRVFB_SMS:
												{
													if(nMSettings.pwrvfbOnOf== 1)
														sprintf(Buffer1,"POWER VFB ON\n");
													else
														sprintf(Buffer1,"POWER VFB OFF\n");
												}
												break;

												case STATE_VFB_SMS:
												{
													if(nMSettings.VBFOnOff== 1)
														sprintf(Buffer1,"VOICE FEEDBACK ON");
													else
														sprintf(Buffer1,"VOICE FEEDBACK OFF");
												}
												break;
												case STATE_ONDELAY_SMS:
												{
													sprintf(Buffer1,"ON DELAY = %02d:%02d:%02d",nTimerSettings.POnHr,nTimerSettings.POnMin,nTimerSettings.POnSec);
												}
												break;
												case STATE_SDDELAY_SMS:
												{
													sprintf(Buffer1,"STAR DELTA DELAY = %02d:%02d:%02d",nTimerSettings.SDHr,nTimerSettings.SDMin,nTimerSettings.SDSec);
												}
												break;
												case STATE_SFBDELAY_SMS:
												{
													sprintf(Buffer1,"STARTER FEEDBACK DELAY = %02d:%02d:%02d",nTimerSettings.SfbHr,nTimerSettings.SfbMin,nTimerSettings.SfbSec);
												}
												break;
												case STATE_DRRSTONOF_SMS:
												{
													if(nTimerSettings.DrReOnOf == 1)
														sprintf(Buffer1,"DRY RUN RESTART ON");
													else
														sprintf(Buffer1,"DRY RUN RESTART OFF");
												}
												break;
												case STATE_DRRST_SMS:
												{
													sprintf(Buffer1,"DRY RUN RESTART TIME = %02d:%02d:%02d",nTimerSettings.DrReHr,nTimerSettings.DrReMin,nTimerSettings.DrReSec);
												}
												break;
												case STATE_DRSCANONOF_SMS:
												{
													if(nTimerSettings.DrScOnOf == 1)
														sprintf(Buffer1,"DRY RUN SCAN ON");
													else
														sprintf(Buffer1,"DRY RUN SCAN OFF");
												}
												break;
												case STATE_DRSCAN_SMS:
												{
													sprintf(Buffer1,"DRY RUN SCAN TIME = %02d:%02d:%02d",nTimerSettings.DrScHr,nTimerSettings.DrScMin,nTimerSettings.DrScSec);
												}
												break;
												case STATE_MAXTIMONOF_SMS:
												{
													if(nTimerSettings.MaxRnOnOf == 1)
														sprintf(Buffer1,"MAXIMUM RUN TIMER ON");
													else
														sprintf(Buffer1,"MAXIMUM RUN TIMER OFF");
												}
												break;
												case STATE_MAXTIM_SMS:
												{
													sprintf(Buffer1,"MAXIMUM RUN TIMER = %02d:%02d:%02d",nTimerSettings.MaxRnHr,nTimerSettings.MaxRnMin,nTimerSettings.MaxRnSec);
												}
												break;
												case STATE_CYCLICONOF_SMS:
												{
													if(nTimerSettings.CycLicOnOf == 1)
														sprintf(Buffer1,"CYCLIC TIMER ON\n");
													else
														sprintf(Buffer1,"CYCLIC TIMER OFF\n");
													sprintf(Buffer1,"%sCYCLIC ON TIMER = %02dH:%02dM\n",Buffer1,nTimerSettings.CycLicOnHr,nTimerSettings.CycLicOnMin);

													sprintf(Buffer1,"%sCYCLIC OF TIMER = %02dH:%02dM",Buffer1,nTimerSettings.CycLicOfHr,nTimerSettings.CycLicOfMin);
												}
												break;
												case STATE_FOTA_SMS:
												{
													sprintf(Buffer1,"FOTA UPDATION FAILED"); 
 													
												}
												break;
												case STATE_MQTT_STATUS_SMS:
												{  
												    if(MqttInitStatus==1)
													sprintf(Buffer1,"MQTT CONNECTED"); 
													else
													sprintf(Buffer1,"MQTT DISCONNECTED"); 
 													
												}
												break;
												case STATE_FERTCYCLICONOF_SMS:
												{
													again1send=1;
													again2send=1;
													again3send=1;
													again4send=1;

													sprintf(Buffer1,"FERTCYCLIC ON TIMER = %02dM:%02dS\n",nTimerSettings.fertCycLicOnMin,nTimerSettings.fertCycLicOnSec);

													sprintf(Buffer1,"%sFERTCYCLIC OF TIMER = %02dM:%02dS",Buffer1,nTimerSettings.fertCycLicOfMin,nTimerSettings.fertCycLicOfSec);
												}
												break;
											
												case STATE_VALVEONOF_SMS:		//Valve control mode on/off time sms
												{
													if(nDripSettings.driponof==1)
													{
													sprintf(Buffer1,"VALVE CONTROL MODE ON");
													}
													else
													sprintf(Buffer1,"VALVE CONTROL MODE OFF");

												}
												break;
												case STATE_DRIPGAPDAYONOF_SMS:		//Valve DAY SKIP mode on/off time sms
												{
													if(nDripSettings.dripgapdayonof==1)
													{
													sprintf(Buffer1,"VALVE DAY SKIP MODE ON");
													}
													else
													sprintf(Buffer1,"VALVE DAY SKIP MODE OFF");

												}
												break;
												case STATE_GAPDAYS_SMS:		//Valve DAY SKIP mode on/off time sms
												{
													sprintf(Buffer1,"SET NO. OF SKIP DAYS=%03d",nDripSettings.dripgapdays);

												}
												break;
												case STATE_SKIPDAYCOUNT_SMS:		//Valve DAY SKIP mode on/off time sms
												{
													sprintf(Buffer1," DRIP CYCLE COUNT=%03d\nDRIP DAY COUNT=%03d \nSKIP DAY COUNT= %03d\nSET SKIP DAYS=%03d\n",dripcyclecount,dripcycledate,nDripSettings.dripgapdaycount,nDripSettings.dripgapdays);
												}
												break;
												case STATE_VALVEVIEW_SMS:		//Valve No: on/off sms send
											{
												long runvtim;
												float pervtim;
												revalue = zone[nVaTr.Currentvalve].stoptime;
												reTpHr = revalue/3600;
												revalue = revalue%3600;
												reTpMin = revalue/60;
												revalue = revalue%60;
												reTpSec = revalue;
                                                pervtim = ((zoneonof[nVaTr.programselection].timerpercent)*0.01);
							                    runvtim=((zone[nDripSettings.changeinstp].actthr*3600)+(zone[nDripSettings.changeinstp].acttmin*60));
							                    runvtim = (long)(pervtim*runvtim);

												sprintf(Buffer1," %02d044 \n",(datetime.tm_sec+2));
												sprintf(Buffer1,"%sP%d\nZONE=%03d CLOSE\nZONE=%03d OPEN\n",Buffer1,nVaTr.programselection,nVaTr.Currentvalve,nDripSettings.changeinstp);
												if((zoneonof[nVaTr.programselection].flowmodeonof == 1)||(zoneonof[nVaTr.programselection].flowonof == 1))
												{
													sprintf(Buffer1,"%sFLO=%d L\n",Buffer1,zone[nVaTr.Currentvalve].zstopflowrate);
                                    				if((zoneonof[nVaTr.programselection].flowmodeonof == 1))
													sprintf(Buffer1,"%sRFLO=%d L\n",Buffer1,zone[nDripSettings.stp].actflowrate);//
												}
												sprintf(Buffer1,"%sFR=%03.1f L\n",Buffer1,zone[nVaTr.Currentvalve].runflowrate);//

                								if((zoneonof[nVaTr.programselection].moisturemodeonof == 1)||(zoneonof[nVaTr.programselection].moistureonof == 1))
                                                sprintf(Buffer1,"%sMOIS1=%d MOIS2=%d PERCENT\n",Buffer1,zone[nDripSettings.stp].stopmoisture1,zone[nDripSettings.stp].stopmoisture2);

												sprintf(Buffer1,"%sTIM=%02d:%02d:%02d\n",Buffer1,reTpHr,reTpMin,reTpSec);
												revalue = runvtim;
												reTpHr = revalue/3600;
												revalue = revalue%3600;
												reTpMin = revalue/60;
												revalue = revalue%60;
												reTpSec = revalue;
                                                if(zoneonof[nVaTr.programselection].flowmodeonof == 0)
												sprintf(Buffer1,"%sRTIM=%02d:%02d:%02d",Buffer1,reTpHr,reTpMin,reTpSec);

											}
												break;
												case STATE_FIRST_VALVEVIEW_SMS:		//First Valve No: on/off sms send
											{
												int tp2;
												revalue = nVaTr.REMTIM;
												if(revalue<0)
								                  revalue=0;

												reTpHr = revalue/3600;
												revalue = revalue%3600;
												reTpMin = revalue/60;
												revalue = revalue%60;
												reTpSec = revalue;

												sprintf(Buffer1," %02d045 \n",(datetime.tm_sec+2));
												sprintf(Buffer1,"%sP%1d ZONE=%03d OPEN\n",Buffer1,nVaTr.programselection,nDripSettings.stp);
											    sprintf(Buffer1,"%sRTIM=%02d:%02d:%02d\n",Buffer1,reTpHr,reTpMin,reTpSec);
												if((zoneonof[nVaTr.programselection].flowmodeonof == 1))
												{
                                                tp2	= zone[nDripSettings.stp].remfflowrate;
												sprintf(Buffer1,"%sRFLO=%d L\n",Buffer1,tp2);
												}
												sprintf(Buffer1,"%sFR=%03.1f L\n",Buffer1,zone[nDripSettings.stp].runflowrate);//

											}
												break;
												case STATE_STVALVEVIEW_SMS:		//Valve No: on/off sms send
											{
												revalue = nVaTr.prvstruntime;
												reTpHr = revalue/3600;
												revalue = revalue%3600;
												reTpMin = revalue/60;
												revalue = revalue%60;
												reTpSec = revalue;

												sprintf(Buffer1," %02d064 \n",(datetime.tm_sec+2));
												sprintf(Buffer1,"%sP%d\nSTANDALONE ZONE=%03d CLOSE\nSTANDALONE ZONE=%03d OPEN\n",Buffer1,nVaTr.programselection,nDripSettings.changeinststp,nVaTr.Currentstvalve);
												sprintf(Buffer1,"%sTIM=%02d:%02d:%02d\n",Buffer1,reTpHr,reTpMin,reTpSec);
												sprintf(Buffer1,"%sFLO=%d\n",Buffer1,nVaTr.prvstrunflow);
												sprintf(Buffer1,"%sFR=%03.1f L\n",Buffer1,zone[nDripSettings.stp].runflowrate);//

											}
												break;

												case STATE_FIRST_STVALVEVIEW_SMS:		//First Valve No: on/off sms send
												{

												sprintf(Buffer1," %02d063\n",(datetime.tm_sec+2));
												sprintf(Buffer1,"%sP%1d\nSTANDALONE ZONE=%03d OPEN\n",Buffer1,nVaTr.programselection,nDripSettings.ststp);
											    //sprintf(Buffer1,"%sRemaining time= %02d:%02d:%02d\n",Buffer1,reTpHr,reTpMin,reTpSec);
												sprintf(Buffer1,"%sFR=%03.1f L\n",Buffer1,zone[nDripSettings.stp].runflowrate);//

												}
												break;

												case STATE_CYCLE_COMPLETED_SMS:		//First Valve No: on/off sms send
												{
													sprintf(Buffer1,"ZONE CYCLE COMPLETED AND  NEXT ZONE CYCLE STARTED\n DRIP CYCLE COUNT=%03d\n DRIP DAY COUNT=%03d\nSKIP DAY COUNT=%d\nSET SKIP DAYS=%03d\n ",dripcyclecount,dripcycledate,nDripSettings.dripgapdaycount,nDripSettings.dripgapdays);
												}
												break;
												case STATE_VALVESKIPVIEW_SMS:		//Valve No: SKIPPED sms send
												{
													sprintf(Buffer1," %02d114\n",(datetime.tm_sec+2));
													sprintf(Buffer1,"%sZONE=%03d SKIPPED BECAUSE OF NO FEEDBACK \n ",Buffer1,nVaTr.Skipvalve);
												}
												break;
												case STATE_FERTON_SMS:
												{
													removeniagara=1;
													sprintf(Buffer1," %02d046\n",(datetime.tm_sec+2));
													if(nVaTr.fertv1Smsonof == 1)
														sprintf(Buffer1,"%sFERT1 ON\n",Buffer1);
													else
													{
													if(nMoTr.ActlF1RunTimer>0)
													{
													value = nMoTr.ActlF1RunTimer;
		                                            TpHr = value/3600;
		                                            value = value%3600;
		                                            TpMin = value/60;
		                                            value = value%60;
		                                            TpSec = value;
													sprintf(Buffer1,"%sFERT1 OFF F1T=%02d:%02d:%02d\n",Buffer1,TpHr,TpMin,TpSec);

													}
													else
													sprintf(Buffer1,"%sFERT1 OFF\n",Buffer1);
													}
													if(nVaTr.fertv2Smsonof == 1)
														sprintf(Buffer1,"%sFERT2 ON\n",Buffer1);
													else
													{
													if(nMoTr.ActlF2RunTimer>0)
													{
													value = nMoTr.ActlF2RunTimer;
		                                            TpHr = value/3600;
		                                            value = value%3600;
		                                            TpMin = value/60;
		                                            value = value%60;
		                                            TpSec = value;
													sprintf(Buffer1,"%sFERT2 OFF F2T=%02d:%02d:%02d\n",Buffer1,TpHr,TpMin,TpSec);
													}
													else
													sprintf(Buffer1,"%sFERT2 OFF\n",Buffer1);
													}
													if(nVaTr.fertv3Smsonof == 1)
														sprintf(Buffer1,"%sFERT3 ON\n ",Buffer1);
													else
													{
													if(nMoTr.ActlF3RunTimer>0)
													{
													value = nMoTr.ActlF3RunTimer;
		                                            TpHr = value/3600;
		                                            value = value%3600;
		                                            TpMin = value/60;
		                                            value = value%60;
		                                            TpSec = value;
													sprintf(Buffer1,"%sFERT3 OFF F3T=%02d:%02d:%02d\n",Buffer1,TpHr,TpMin,TpSec);
													}
													else
													sprintf(Buffer1,"%sFERT3 OFF\n",Buffer1);
													}if(nVaTr.fertv4Smsonof == 1)
														sprintf(Buffer1,"%sFERT4 ON ",Buffer1);
													else
													{
													if(nMoTr.ActlF4RunTimer>0)
													{
													value = nMoTr.ActlF4RunTimer;
		                                            TpHr = value/3600;
		                                            value = value%3600;
		                                            TpMin = value/60;
		                                            value = value%60;
		                                            TpSec = value;
													sprintf(Buffer1,"%sFERT4 OFF F4T=%02d:%02d:%02d\n",Buffer1,TpHr,TpMin,TpSec);
													}
													else
													sprintf(Buffer1,"%sFERT4 OFF\n",Buffer1);
													}
													}
												break;

												case STATE_REFRESH1ONOF_SMS:
												{
													if(nVaTr.refreshv1Smson == 1)
														sprintf(Buffer1,"FILTER1 ON\n");
													else if(nVaTr.refreshv1Smsof == 1)
														sprintf(Buffer1,"FILTER1 OFF\n");
												}
												break;

												case STATE_REFRESH2ONOF_SMS:
												{
													if(nVaTr.refreshv2Smson == 1)
														sprintf(Buffer1,"FILTER2 ON\n");
													else if(nVaTr.refreshv2Smsof == 1)
														sprintf(Buffer1,"FILTER2 OFF\n");
												}
												break;

												case STATE_REFRESH3ONOF_SMS:
												{
													if(nVaTr.refreshv3Smson == 1)
														sprintf(Buffer1,"FILTER3 ON\n");
													else if(nVaTr.refreshv3Smsof == 1)
														sprintf(Buffer1,"FILTER3 OFF\n");
												}
												break;

												case STATE_REFRESH4ONOF_SMS:
												{
													if(nVaTr.refreshv4Smson == 1)
														sprintf(Buffer1,"FILTER4 ON\n",Buffer1);
													else if(nVaTr.refreshv4Smsof == 1)
														sprintf(Buffer1,"FILTER4 OFF\n",Buffer1);
												}
												break;


												case STATE_ENTERDRIPSETON_SMS:		//Drip password on time sms
												{
													if(nDripSettings.entersetting==1)
													{
													sprintf(Buffer1,"DRIP SETTING MODE ON ");
													}
													else
													sprintf(Buffer1,"DRIP SETTING MODE OFF ");

												}
												break;
												case STATE_STARTFROM_SMS:		//valve start from on time sms
												{
													sprintf(Buffer1," SET VALVE START FROM= %03d\nPREVIOUS SET VALVE START FROM= %03d\nSET FIRST VALVE = %03d\nSET LAST VALVE = %03d\n",nDripSettings.startfrom,nDripSettings.prvstartfrom,nDripSettings.decidefirst,nDripSettings.decidelast);

												}
												break;
												case STATE_CHANGEFROM_SMS:		//valve start from on time sms
												{
													if(zone[nDripSettings.changefrom].actthr>0||zone[nDripSettings.changefrom].acttmin>0)
													sprintf(Buffer1," SET ZONE CHANGE FROM= %03d\n",nDripSettings.changefrom);
													else
													sprintf(Buffer1," CHANGE FROM ZONE TIME IS ZERO ");


												}
												break;
												case STATE_VLIMITSMSSET_SMS:		//valve start from on time sms
												{
													sprintf(Buffer1,"V49 SET SMS LIMIT = %03d\n",limitsmsset);
													if(limitsmsonof == 1)
														sprintf(Buffer1,"%sSMS LIMIT ON\n",Buffer1);
													else
														sprintf(Buffer1,"%sSMS LIMIT OFF\n",Buffer1);


												}
												break;

												case STATE_LIMITSMSSET_SMS:		//valve start from on time sms
												{
													sprintf(Buffer1,"SET SMS LIMIT = %03d\n",limitsmsset);
													if(limitsmsonof == 1)
														sprintf(Buffer1,"%sSMS LIMIT ON\n",Buffer1);
													else
														sprintf(Buffer1,"%sSMS LIMIT OFF\n",Buffer1);


												}
												break;
												case STATE_DECIDELAST_SMS:		//valve start from on time sms
												{
		//									//sprintf(Buffer1,"ZONE START FROM= %03d\nPREVIOUS SET ZONE START FROM= %03d\nSET FIRST VALVE = %03d\nSET LAST VALVE = %03d",nDripSettings.startfrom,nDripSettings.prvstartfrom,nDripSettings.decidefirst,nDripSettings.decidelast);
													sprintf(Buffer1,"ZONE START FROM= %03d\nSET FB LAST VALVE = %03d\nSET LAST VALVE = %03d",nDripSettings.startfrom,nDripSettings.decidefblast,nDripSettings.decidelast);

												}
												break;

												case STATE_FERTSKIPGROUPDETAILS_SMS:		//valve start from on time sms
												{
													sprintf(Buffer1,"SET1 FERTILIZER SKIP FIRST AND LAST= %03d,%03d\nSET2 FERTILIZER SKIP FIRST AND LAST= %03d,%03d\nSET3 FERTILIZER SKIP FIRST AND LAST = %03d,%03d\n",nDripSettings.fertskipfirst1,nDripSettings.fertskiplast1,nDripSettings.fertskipfirst2,nDripSettings.fertskiplast2,nDripSettings.fertskipfirst3,nDripSettings.fertskiplast3);

												}
												break;
												case STATE_FERTSKIPDETAILS_SMS:		//valve start from on time sms
												{
													sprintf(Buffer1,"FERTILIZER SKIP1= %03d\nFERTILIZER SKIP2= %03d\nFERTILIZER SKIP3= %03d\nFERTILIZER SKIP4= %03d\nFERTILIZER SKIP5= %03d\nFERTILIZER SKIP6= %03d\nFERTILIZER SKIP7= %03d\nFERTILIZER SKIP8= %03d\n",nDripSettings.fertskip1,nDripSettings.fertskip2,nDripSettings.fertskip3,nDripSettings.fertskip4,nDripSettings.fertskip5,nDripSettings.fertskip6,nDripSettings.fertskip7,nDripSettings.fertskip8);

												}
												break;
												case STATE_DECIDEFIRST_SMS:		//valve start from on time sms
												{
													sprintf(Buffer1," SET VALVE START FROM= %03d\nPREVIOUS SET VALVE START FROM= %03d\nSET FIRST VALVE = %03d\nSET LAST VALVE = %03d\n",nDripSettings.startfrom,nDripSettings.prvstartfrom,nDripSettings.decidefirst,nDripSettings.decidelast);;

												}
												break;
												case STATE_CYCRESTARTONOF_SMS:
												{
													if(nVaTr.cycrestartonof==1)
													{
													sprintf(Buffer1,"VALVE CYCLIC RESTART ON");
													}
													else
													sprintf(Buffer1,"VALVE CYCLIC RESTART OFF");

												}
												break;
												case STATE_QUERESTARTONOF_SMS:
												{
													if(nVaTr.querestartonof==1)
													{
													sprintf(Buffer1,"PROGRAM QUE CYCLIC RESTART ON");
													}
													else
													sprintf(Buffer1,"PROGRAM QUE CYCLIC RESTART OFF");

												}
												break;
												case STATE_CALFLOWONOF_SMS:
												{
													if(nVaTr.calflowrateonof==1)
													{
													sprintf(Buffer1,"CACULATED FLOWRATE SET ON");
													}
													else
													sprintf(Buffer1,"CACULATED FLOWRATE SET OFF");

												}
												break;


												case STATE_VALFBONOF_SMS:
												{
													if(nVaTr.valvefeedbackonof==1)
													{
													sprintf(Buffer1,"VALVE FEEDBACK CHECK ON");
													}
													else
													sprintf(Buffer1,"VALVE FEEDBACK CHECK OFF");

												}
												break;
												case STATE_DELVALFBONOF_SMS:
												{
													if(nVaTr.delvalvefeedbackonof==1)
													{
													sprintf(Buffer1,"DELAY VALVE FEEDBACK CHECK ON");
													}
													else
													sprintf(Buffer1,"DELAY VALVE FEEDBACK CHECK OFF");

												}
												break;
												case STATE_VALFBKTIM_SMS:
												{
													sprintf(Buffer1,"VALVE FEEDBACK CHECK TIME = %02d:%02d:%02d",nVaTr.valvefbkTimHr,nVaTr.valvefbkTimMin,nVaTr.valvefbkTimSec);
												}
												break;
												case STATE_REFRESHONOF_SMS:
												{
													if(nVaTr.refreshvalveonof==1)
													{
													sprintf(Buffer1,"VALVE REFRESH CONTROL ON");
													}
													else
													sprintf(Buffer1,"VALVE REFRESH CONTROL OFF");
												}
												break;
												case STATE_VREFRSHTIMON_SMS:
												{
													sprintf(Buffer1,"REF V1 ON TIM=%02d:%02d",nVaTr.RefreshTimonHr1,nVaTr.RefreshTimonMin1);
													sprintf(Buffer1,"%s\nREF V2 ON TIM=%02d:%02d",Buffer1,nVaTr.RefreshTimonHr2,nVaTr.RefreshTimonMin2);
													sprintf(Buffer1,"%s\nREF V3 ON TIM=%02d:%02d",Buffer1,nVaTr.RefreshTimonHr3,nVaTr.RefreshTimonMin3);
													sprintf(Buffer1,"%s\nREF V4 ON TIM=%02d:%02d",Buffer1,nVaTr.RefreshTimonHr4,nVaTr.RefreshTimonMin4);

												}
												break;
												case STATE_VREFRSHTIMOF_SMS:
												{
													sprintf(Buffer1,"VALVE REFRESH OFF TIME = %02d:%02d:%02d",nVaTr.RefreshTimofHr,nVaTr.RefreshTimofMin,nVaTr.RefreshTimofSec);
												}
												break;
												case STATE_RTCONOF_SMS:
												{
													if(nTimerSettings.RTCOnOf == 1)
														sprintf(Buffer1,"REAL TIME CLOCK ON");
													else
														sprintf(Buffer1,"REAL TIME CLOCK OFF");
												}
												break;
												case STATE_RTCTIMON1_SMS:
												{
													sprintf(Buffer1,"RTC1 ON= %02dH:%02dM\n",nTimerSettings.RTCOnHr[1],nTimerSettings.RTCOnMin[1]);

													sprintf(Buffer1,"%sRTC1 OFF= %02dH:%02dM",Buffer1,nTimerSettings.RTCOfHr[1],nTimerSettings.RTCOfMin[1]);
												}
												break;
												case STATE_RTCTIMON2_SMS:
												{
													sprintf(Buffer1,"RTC2 ON= %02dH:%02dM\n",nTimerSettings.RTCOnHr[2],nTimerSettings.RTCOnMin[2]);

													sprintf(Buffer1,"%sRTC2 OFF= %02dH:%02dM",Buffer1,nTimerSettings.RTCOfHr[2],nTimerSettings.RTCOfMin[2]);
												}
												break;
												case STATE_RTCTIMON3_SMS:
												{
													sprintf(Buffer1,"RTC3 ON= %02dH:%02dM\n",nTimerSettings.RTCOnHr[3],nTimerSettings.RTCOnMin[3]);

													sprintf(Buffer1,"%sRTC3 OFF= %02dH:%02dM",Buffer1,nTimerSettings.RTCOfHr[3],nTimerSettings.RTCOfMin[3]);
												}
												break;
												case STATE_RTCTIMON4_SMS:
												{
													sprintf(Buffer1,"RTC4 ON= %02dH:%02dM\n",nTimerSettings.RTCOnHr[4],nTimerSettings.RTCOnMin[4]);

													sprintf(Buffer1,"%sRTC4 OFF= %02dH:%02dM",Buffer1,nTimerSettings.RTCOfHr[4],nTimerSettings.RTCOfMin[4]);
												}
												break;
												 case STATE_DATETIME_SMS:
												{
													sAPI_GetRealTimeClock(&datetime);
													sprintf(Buffer1,"DATE=%02d/%02d/%04d \n TIME=%02d:%02d:%02d \n STORED 4G AG4 BOD PCGv%s",datetime.tm_mday,datetime.tm_mon,datetime.tm_year,datetime.tm_hour, datetime.tm_min, datetime.tm_sec,Version);//DSV6.3F
												}
												break; 
												case STATE_PROGRAMSEL_SMS:
												{
													if(nVaTr.programselection==1)
													sprintf(Buffer1,"DRIP PROGRAM 1 SELECTED ");
													else if(nVaTr.programselection==2)
													sprintf(Buffer1,"DRIP PROGRAM 2 SELECTED ");
													else if(nVaTr.programselection==3)
													sprintf(Buffer1,"DRIP PROGRAM 3 SELECTED ");
													else if(nVaTr.programselection==4)
													sprintf(Buffer1,"DRIP PROGRAM 4 SELECTED ");
													else
													sprintf(Buffer1,"DRIP PROGRAM 1 SELECTED ");
												}
												break;
												case STATE_SCRDLONOF_SMS:
												{
													if(nTimerSettings.ScrDlOnOff == 1)
														sprintf(Buffer1,"SCR TIMER ON");
													else
														sprintf(Buffer1,"SCR TIMER OFF");

												}
												break;
												case STATE_POSCRDLONOF_SMS:
												{
													if(nTimerSettings.PoScrDlOnOff == 1)
														sprintf(Buffer1,"SCR DELAY TIMER ON");
													else
														sprintf(Buffer1,"SCR DELAY TIMER OFF");

												}
												break;
												case STATE_SCRDEL_SMS:
												{
													sprintf(Buffer1,"SCR TIMER  = %02d:%02d:%02d",nTimerSettings.ScrDlHr,nTimerSettings.ScrDlMin,nTimerSettings.ScrDlSec);
												}
												break;
												case STATE_POSCRDEL_SMS:
												{
													sprintf(Buffer1,"SCR TIMER  = %02d:%02d:%02d",nTimerSettings.PoScrDlHr,nTimerSettings.PoScrDlMin,nTimerSettings.PoScrDlSec);
												}
												break;
												case STATE_LOWVOLTONOFF_SMS:
												{
													if(nTimerSettings.LowVoltOnOff == 1)
														sprintf(Buffer1,"LOW VOLTAGE SCAN ON");
													else
														sprintf(Buffer1,"LOW VOLTAGE SCAN OFF");

												}
												break;
												case STATE_LOWVOLTII_SMS:
												{
													sprintf(Buffer1,"LOW VOLTAGE TRIP FOR 2 PHASE = %03d",nTimerSettings.LowVoltII);
												}
												break;
												case STATE_LOWVOLTIII_SMS:
												{
													sprintf(Buffer1,"LOW VOLTAGE TRIP FOR 3 PHASE = %03d",nTimerSettings.LowVoltIII);
												}
												break;
												case STATE_HIGHVOLTONOFF_SMS:
												{
													if(nTimerSettings.HighVoltOnOff == 1)
														sprintf(Buffer1,"HIGH VOLTAGE SCAN ON");
													else
														sprintf(Buffer1,"HIGH VOLTAGE SCAN OFF");

												}
												break;
												case STATE_HIGHVOLTII_SMS:
												{
													sprintf(Buffer1,"HIGH VOLTAGE TRIP FOR 2 PHASE = %03d",nTimerSettings.HighVoltII);
												}
												break;
												case STATE_HIGHVOLTIII_SMS:
												{
													sprintf(Buffer1,"HIGH VOLTAGE TRIP FOR 3 PHASE = %03d",nTimerSettings.HighVoltIII);
												}
												break;
												case STATE_IMBVOLT_SMS:
												{
													sprintf(Buffer1,"IMBALANCE VOLTAGE = %03d",nTimerSettings.ImbVolt);
												}
												break;
												case STATE_PFCVOLT_SMS:
												{
													sprintf(Buffer1,"SET PFC VOLTAGE = %03d",nTimerSettings.pfcvolt);
												}
												break;
												case STATE_REVPHASE_SMS:
												{
													if(nTimerSettings.RvePhOnoff == 1)
														sprintf(Buffer1,"REVERSE PHASE SCAN ON");
													else
														sprintf(Buffer1,"REVERSE PHASE SCAN OFF");

												}
												break;
												case STATE_CURSPP_SMS:
												{
													if(nTimerSettings.CurSppOnOff == 1)
														sprintf(Buffer1,"CURRENT SPP SCAN ON");
													else
														sprintf(Buffer1,"CURRENT SPP SCAN OFF");

												}
												break;
												case STATE_SPP_SMS:
												{
													if(nTimerSettings.SppOnoff == 1)
														sprintf(Buffer1,"VOLTAGE SENSING SPP ON");
													else
														sprintf(Buffer1,"VOLTAGE SENSING SPP OFF");

												}
												break;
												case STATE_CALFLOWII_SMS:
												{
													//FloatroString1Dig(TpStr,nTimerSettings.DrAmpsII);
													sprintf(Buffer1,"CALCULATED FLOW SET FOR 2 PHASE = %03.01f",nTimerSettings.calflow2);
												}
												break;
												case STATE_CALFLOWIII_SMS:
												{
													//FloatroString1Dig(TpStr,nTimerSettings.DrAmpsII);
													sprintf(Buffer1,"CALCULATED FLOW SET FOR 3 PHASE = %03.01f",nTimerSettings.calflow3);
												}
												break;
												case STATE_DRAMPII_SMS:
												{
													//FloatroString1Dig(TpStr,nTimerSettings.DrAmpsII);
													sprintf(Buffer1,"DRY RUN AMPS FOR 2 PHASE = %03.01f",nTimerSettings.DrAmpsII);
												}
												break;
												case STATE_DRAMPIII_SMS:
												{
												    //FloatroString1Dig(TpStr,nTimerSettings.DrAmpsIII);
													sprintf(Buffer1,"DRY RUN AMPS FOR 3 PHASE = %03.01f",nTimerSettings.DrAmpsIII);
												}
												break;
												case STATE_OLINOFF_SMS:
												{
													if(nTimerSettings.OlOnOff == 1)
														sprintf(Buffer1,"OVER LOAD SCAN ON");
													else
														sprintf(Buffer1,"OVER LOAD SCAN OFF");

												}
												break;
                                                case STATE_LOWPRESSONOF_SMS:
												{
													if(nTimerSettings.lowpressOnOff == 1)
														sprintf(Buffer1,"LOW PRESSURE SCAN ON");
													else
														sprintf(Buffer1,"LOW PRESSURE SCAN OFF");

												}
												break;
												case STATE_HIGHPRESSONOF_SMS:
												{
													if(nTimerSettings.highpressOnOff == 1)
														sprintf(Buffer1,"HIGH PRESSURE SCAN ON");
													else
														sprintf(Buffer1,"HIGH PRESSURE SCAN OFF");

												}
												break;
												case STATE_LOWPRESS_SMS:
												{
												 //FloatroString1Dig(TpStr,nTimerSettings.OlAmpsII);
													sprintf(Buffer1,"SET LOW PRESSURE = %02.01f",nTimerSettings.lowpress);
												}
												break;
                                                case STATE_HIGHPRESS_SMS:
												{
												 //FloatroString1Di g(TpStr,nTimerSettings.OlAmpsII);
													sprintf(Buffer1,"SET HIGH PRESSURE = %02.01f",nTimerSettings.highpress);
												}
												break;
												case STATE_OLAMPSII_SMS:
												{
												 //FloatroString1Dig(TpStr,nTimerSettings.OlAmpsII);
													sprintf(Buffer1,"OVER LOAD AMPS FOR 2 PHASE = %03.01f",nTimerSettings.OlAmpsII);
												}
												break;
												case STATE_OLAMPSIII_SMS:
												{
												    //FloatroString1Dig(TpStr,nTimerSettings.OlAmpsIII);
													sprintf(Buffer1,"OVER LOAD AMPS FOR 3 PHASE = %03.01f",nTimerSettings.OlAmpsIII);
												}
												break;
												case STATE_OLSCAN_SMS:
												{
													sprintf(Buffer1,"OVER LOAD SCAN TIME = %02d:%02d:%02d",nTimerSettings.OlScanHr,nTimerSettings.OlScanMin,nTimerSettings.OlScanSec);

												}
												break;
												case STATE_AUTOSTII_SMS:
												{
													if(nTimerSettings.AutoStIIOnOff == 1)
														sprintf(Buffer1,"II PHASE ON");
													else
														sprintf(Buffer1,"II PHASE OFF");

												}
												break;
												case STATE_AUTOSTIII_SMS:
												{
													if(nTimerSettings.AutoStIIIOnOff == 1)
														sprintf(Buffer1,"MOBILE SET 3 PHASE ON ");
													else
														sprintf(Buffer1,"MOBILE SET 3 PHASE OFF");

												}
												break;
												case STATE_AUTOOLDRRST_SMS:
												{
													if(nTimerSettings.AutoOlDrRstIIOnOff == 1)
														sprintf(Buffer1,"OVER LOAD RESET ON");
													else
														sprintf(Buffer1,"OVER LOAD RESET OFF");

												}
												break;
												case STATE_AUTODRRUNRST_SMS:
												{
													if(nTimerSettings.AutoDrRunRstIIOnOff == 1)
														sprintf(Buffer1,"DRY RUN RESET ON");
													else
														sprintf(Buffer1,"DRY RUN RESET OFF");

												}
												break;
												case STATE_POWERFACTOR_SMS:
												{
													if(nTimerSettings.pfcOnOff==1)
														sprintf(Buffer1,"POWER FACTOR CORRECTION ON");
													else
														sprintf(Buffer1,"POWER FACTOR CORRECTION OFF");

												}
												break;
												case STATE_MANULONOF_SMS:
												{
													if(nTimerSettings.ManualOnOff == 1)
														sprintf(Buffer1,"MANUAL ON ");
													else
														sprintf(Buffer1,"MANUAL OFF");

												}
												break;
												case STATE_OLRSTVOLONOFF_SMS:
												{
													if(nTimerSettings.OlRstVolOnoff == 1)
														sprintf(Buffer1,"OL RESTART VOLTAGE ON");
													else
														sprintf(Buffer1,"OL RESTART VOLTAGE OFF");

												}
												break;
												case STATE_OLRSTVOL_SMS:
												{
													sprintf(Buffer1,"OL RESTART VOLTAGE = %03d",nTimerSettings.OlRstVol);
												}
												break;
												case STATE_DROCCONOFF_SMS:
												{
													if(nTimerSettings.DrOccurOnOff == 1)
														sprintf(Buffer1,"DRY RUN OCCURANCE SCAN ON");
													else
														sprintf(Buffer1,"DRY RUN OCCURANCE SCAN OFF");

												}
												break;
												case STATE_DROCCTIM_SMS:
												{
													sprintf(Buffer1,"DRY RUN OCCURANCE TIME = %02d:%02d:%02d",nTimerSettings.DrOccurTimHr,nTimerSettings.DrOccurTimMin,nTimerSettings.DrOccurTimSec);
												}
												break;
												case STATE_DROCCNUM_SMS:
												{
													sprintf(Buffer1,"NUMBER OF DRY RUN OCCURANCE  = %03d",nTimerSettings.DrOccurNum);
												}
												break;
												case STATE_DIFFVOLTII_SMS:
												{
													sprintf(Buffer1,"DIFFERENCE LOW VOLTAGE FOR 2 PHASE = %03d\n",nTimerSettings.DiffVoltII );
												}
												break;
												case STATE_DIFFVOLTIII_SMS:
												{
													sprintf(Buffer1,"DIFFERENCE LOW VOLTAGE FOR 3 PHASE = %03d\n",nTimerSettings.DiffVoltIII );
												}
												break;
								
											 
												case STATE_CTRON_SMS:
												{
													sprintf(Buffer1,"CT R SENSING ON\n\r");
												}
												break;
												case STATE_CTROF_SMS:
												{
													sprintf(Buffer1,"CT R SENSING OFF\n\r");
												}
												break;
												case STATE_CTYON_SMS:
												{
													sprintf(Buffer1,"CT Y SENSING ON\n\r");
												}
												break;
												case STATE_CTYOF_SMS:
												{
													sprintf(Buffer1,"CT Y SENSING OFF\n\r");
												}
												break;
												case STATE_CTBON_SMS:
												{
													sprintf(Buffer1,"CT B SENSING ON\n\r");
												}
												break;
												case STATE_CTBOF_SMS:
												{
													sprintf(Buffer1,"CT B SENSING OFF\n\r");
												}
												break;
												case STATE_SERVICE_SMS:
												{
												if(nMSettings.gethidesmsonoff == 1)
												sprintf(Buffer1,"SERVICE NUMBER REG. DONE\n ");
												else
												sprintf(Buffer1,"SERVICE NUMBER REG. DONE\n NUMBER=%s",ServiceNumber);
												}
												break;
												case STATE_AUTORST2ONOFF_SMS:
												{
													if(nTimerSettings.AutoRst2On == 1)
														sprintf(Buffer1,"AUTO RESET 2 ON");
													else
														sprintf(Buffer1,"AUTO RESET 2 OFF");

												}
												break;
												case STATE_AUTORSTONOFF_SMS:
												{
													if(nTimerSettings.AutoRstOn == 1)
														sprintf(Buffer1,"MOBILE RESET ON");
													else
														sprintf(Buffer1,"MOBILE RESET OFF");

												}
												break;
												case STATE_HIDIFFVOLTII_SMS:
												{
													sprintf(Buffer1,"DIFFERENCE HIGH VOLTAGE FOR 2 PHASE = %03d\n",nTimerSettings.HiDiffVoltII );

												}
												break;
												case STATE_HIDIFFVOLTIII_SMS:
												{
													sprintf(Buffer1,"DIFFERENCE HIGH VOLTAGE FOR 3 PHASE = %03d\n",nTimerSettings.HiDiffVoltIII );
												}
												break;
												default:
												{
													sprintf(Buffer1,"UnKnown2 Please send this to Customer Support Error : STATE_SENDSMS = %d",STATE_SENDSMS);

												}
												break;
											}
										} */
										//else if(PowerOnSms == 1)
										//else if((PowerOnSms == 1) && (creg_reset_flag==0))
										else if(PowerOnSms == 1)
										{
											/* if(creg_reset_flag==0)
											{ */
											//checkpower=Check2Phase();
										/*	if(YVoltage1<115)
												checkpower=2;
											else
											checkpower=3;
											if(checkpower == 3)
											{


											if(MakeRealyOn == 1 && nTimerSettings.AutoRstOn ==1)
											{
											if(Appmodeon==1)
										    sprintf(Buffer1," %02d031\n",(datetime.tm_sec+2));
										    else
											sprintf(Buffer1,"AUTO ON \n POWER ON WITH 3 PHASE");
											}
											else if(MakeRealyOn == 1 && nTimerSettings.AutoRstOn ==0)
											{
											if(Appmodeon==1)
										    sprintf(Buffer1," %02d032\n",(datetime.tm_sec+2));
										    else
											sprintf(Buffer1,"AUTO OFF \n POWER ON WITH 3 PHASE\n AUTO RESET OFF");
											}
											else
											{
											if(Appmodeon==1)
										    sprintf(Buffer1," %02d033\n",(datetime.tm_sec+2));
										    else
											sprintf(Buffer1,"AUTO OFF \n POWER ON WITH 3 PHASE");
											}
											}

											else
											{
											if(MakeRealyOn == 1 && nTimerSettings.AutoRstOn ==1)
											{
											if(Appmodeon==1)
										    sprintf(Buffer1," %02d034\n",(datetime.tm_sec+2));
										    else
											sprintf(Buffer1,"AUTO ON \n POWER ON WITH 2 PHASE");
											}
											else if(MakeRealyOn == 1 && nTimerSettings.AutoRstOn ==0)
											{
											if(Appmodeon==1)
										    sprintf(Buffer1," %02d035\n",(datetime.tm_sec+2));
										    else
											sprintf(Buffer1,"AUTO OFF \n POWER ON WITH 2 PHASE\n AUTO RESET OFF");
											}
											else
											{
											if(Appmodeon==1)
										    sprintf(Buffer1," %02d036\n",(datetime.tm_sec+2));
										    else
											sprintf(Buffer1,"AUTO OFF \n POWER ON WITH 2 PHASE");  //dg_changed  1 space removed before power
											}
											}   
											
										//	sprintf(Buffer1,"%s\nR=%03.0f;",Buffer1,nCurretnCond.RVoltage);
											sprintf(Buffer1,"%s\nR=%03.0f;",Buffer1,RVoltage1);
					                    //    sprintf(Buffer1,"%sY=%03.0f;",Buffer1,nCurretnCond.YVoltage);   //dg_changed  removed \n
											  sprintf(Buffer1,"%sY=%03.0f;",Buffer1,YVoltage1);
										//    sprintf(Buffer1,"%sB=%03.0fV\n",Buffer1,nCurretnCond.BVoltage);  //dg_changed  1 space removed after v
											sprintf(Buffer1,"%sB=%03.0fV\n",Buffer1,BVoltage1); */
											sprintf(Buffer1,"POWER ON"); 
											value =0;// nMoTr.ActofpowerRunTimer;
		                                    TpHr = value/3600;
		                                    value = value%3600;
		                                    TpMin = value/60;
		                                    value = value%60;
		                                    TpSec = value;
											if(Appmodeon==1)
											sprintf(Buffer1,"%sS=%d\nPOF TIM=%02d:%02d:%02d",Buffer1,CSQ,TpHr,TpMin,TpSec);
											// sprintf(Buffer1,"%sS=%d;G=%d;W=%d;\nPOF TIM=%02d:%02d:%02d",Buffer1,CSQ,CGATT,WIFI,TpHr,TpMin,TpSec);
											else
											sprintf(Buffer1,"%sPOF TIM=%02d:%02d:%02d",Buffer1,TpHr,TpMin,TpSec);
										//	}
										}
										
										else if(PowerOnSms == 2)
										{  
									        
											if(MakeRealyOn == 1 && nTimerSettings.AutoRstOn ==1)
											{
											if(Appmodeon==1)
										    sprintf(Buffer1," %02d037\n",(datetime.tm_sec+2));
										    else
											sprintf(Buffer1,"POWER OFF \n AUTO ON \n MOTOR OFF\n");
											}
											else if(MakeRealyOn == 1 && nTimerSettings.AutoRstOn ==0)
											{
											if(Appmodeon==1)
										    sprintf(Buffer1," %02d038\n",(datetime.tm_sec+2));
										    else
											sprintf(Buffer1,"POWER OFF \n AUTO OFF \n MOTOR OFF\n AUTOE RESET OFF");
											}
											else
											{
											if(Appmodeon==1)
										    sprintf(Buffer1," %02d039\n",(datetime.tm_sec+2));
										    else
											sprintf(Buffer1,"POWER OFF \n AUTO OFF \n MOTOR OFF");
											}
										//	nMoTr.ActonpowerRunTimer;
											value = nMoTr.ActonpowerRunTimer;
		                                    TpHr = value/3600;
		                                    value = value%3600;
		                                    TpMin = value/60;
		                                    value = value%60;
		                                    TpSec = value;
											sprintf(Buffer1,"%s\nPON TIM=%02d:%02d:%02d",Buffer1,TpHr,TpMin,TpSec);
											if(zonecom.nightlightRTCOnOf == 1 && nMoTr.ActlnightlightRunTimer>0)
											{
											value = nMoTr.ActlnightlightRunTimer;
		                                    TpHr = value/3600;
		                                    value = value%3600;
		                                    TpMin = value/60;
		                                    value = value%60;
		                                    TpSec = value;
											sprintf(Buffer1,"%s\nNLON %02d:%02d:%02d",Buffer1,TpHr,TpMin,TpSec);
											}
											if(zonecom.lightRTCOnOf == 1 && nMoTr.ActllightRunTimer>0)
											{
											value = nMoTr.ActllightRunTimer;
		                                    TpHr = value/3600;
		                                    value = value%3600;
		                                    TpMin = value/60;
		                                    value = value%60;
		                                    TpSec = value;
											sprintf(Buffer1,"%s\nLON %02d:%02d:%02d",Buffer1,TpHr,TpMin,TpSec);
											}
											/* if(zonecom.fanRTCOnOf == 1 && nMoTr.ActlfanRunTimer>0)
											{
											value = nMoTr.ActlfanRunTimer;
		                                    TpHr = value/3600;
		                                    value = value%3600;
		                                    TpMin = value/60;
		                                    value = value%60;
		                                    TpSec = value;
											sprintf(Buffer1,"%s\nFANON %02d:%02d:%02d",Buffer1,TpHr,TpMin,TpSec);

											} */
										}
										else if(CallOnOfVer == 1)
										{
											sprintf(Buffer1,"CALL RECEIVE ON");
										}
										else if(CallOnOfVer == 2)
										{
											sprintf(Buffer1,"CALL RECEIVE OFF");
										}
										else if(SMS30MinStatus == 1)
										{
											//checkpower=Check2Phase();
											sprintf(Buffer1,"AUTO ON \n\rPOWER ON\n\rMOTOR ON\n\r");
											if(checkpower == 3)
											{
												sprintf(Buffer1,"%sMOTOR STARTED WITH 3 PHASE",Buffer1);
											}
											else
											{
												sprintf(Buffer1,"%sMOTOR STARTED WITH 2 PHASE",Buffer1);
											}
										}
										else if(RegxSmsSend == 1)
										{
											sprintf(Buffer1,"%s",SendSMSString);
										}
									else if(MakeRealyOn == 1)
									{
										if(PowerCurrentCondition == 0)
										{
											if(Appmodeon==1)
										    sprintf(Buffer1," %02d040\n",(datetime.tm_sec+2));
										    else
											sprintf(Buffer1,"AUTO ON \nPOWER ON\nMOTOR OFF\n");
													 switchonofflag=1;

												  // sprintf(Buffer1,"MOBILE ON BY %s\nPOWER ON\nMOTOR OFF\n",WhoMadeRelayOn);

										}
										else
										{
										    if(Appmodeon==1)
										    sprintf(Buffer1," %02d041\n",(datetime.tm_sec+2));
										    else
													sprintf(Buffer1,"AUTO ON \nPOWER OFF\nMOTOR OFF\n");
													switchonofflag=1;

												//   sprintf(Buffer1,"MOBILE ON BY %s\nPOWER OFF\nMOTOR OFF\n",WhoMadeRelayOn);
										}
									}
										/* else if(MakeRealyOn == 0)  //oro_doubt
										{
											if(PowerCurrentCondition == 0)
											{
											if(Appmodeon==1)
										    sprintf(Buffer1," %02d042\n",(datetime.tm_sec+2));
										    else
												 sprintf(Buffer1,"MOTOR OFF \nPOWER ON\nMOTOR OFF\n");
												 switchonofflag=1;

												  // sprintf(Buffer1,"MOTOR OFF BY %s\nPOWER ON\nMOTOR OFF\n",WhoMadeRelayOn);
											}

											else
											{
											if(Appmodeon==1)
										    sprintf(Buffer1," %02d043\n",(datetime.tm_sec+2));
										    else
											sprintf(Buffer1,"MOTOR OFF \nPOWER OFF\nMOTOR OFF\n");
											switchonofflag=1;

										//	sprintf(Buffer1,"MOTOR OFF BY %s\nPOWER OFF\nMOTOR OFF\n",WhoMadeRelayOn);

											}
										} */
										/* else
										{
											sprintf(buf,"\n\rNo SMS Found SO Exit\n\r");
											sAPI_UartPrintf(buf);
											break;
										} */

										sAPI_GetRealTimeClock(&datetime);
										sprintf(buf,"\n\r+++++++++++++++SmsNumber[Tpsms] = %s++++++++++\n\r",SmsNumber[Tpsms]);
										sAPI_UartPrintf(buf);
										sprintf(buf,"\n\r Buffer1 = [%s],line %d\n\r",Buffer1,__LINE__);
										sAPI_UartPrintf(buf);
										
										/* if(strstr(SmsNumber[Tpsms],"*") != 0)
										{

										}
										else if (strstr(SmsNumber[Tpsms],"#") != 0)
										{

										}
										else if (CheckVoliedPhone(SmsNumber[Tpsms]) == 0)
										{

										}
										else if (((strstr(SmsNumber[Tpsms],StoredPhoneNumber[0]) && (strlen(StoredPhoneNumber[0])>5)) != 0) && DNDSMSFLAG[0]==1)
										{

										}
										else if (((strstr(SmsNumber[Tpsms],StoredPhoneNumber[1])&& (strlen(StoredPhoneNumber[1])>5)) != 0) && DNDSMSFLAG[1]==1)
										{

										}
										else if (((strstr(SmsNumber[Tpsms],StoredPhoneNumber[2])&& (strlen(StoredPhoneNumber[2])>5)) != 0) && DNDSMSFLAG[2]==1)
										{

										}
										else if (((strstr(SmsNumber[Tpsms],StoredPhoneNumber[3])&& (strlen(StoredPhoneNumber[3])>5)) != 0) && DNDSMSFLAG[3]==1)
										{

										}
										else *///PRAVEEN
										{

											runtime();
										if(zonecom.standalonemodeonof==1 && MakeRealyOn==0)
										{
										revalue = nVaTr.struntime;
										reTpHr = revalue/3600;
										revalue = revalue%3600;
										reTpMin = revalue/60;
										revalue = revalue%60;
										reTpSec = revalue;
                                        if(Appmodeon==1	)
										sprintf(Buffer1,"%s\nST TIM=%02d:%02d:%02d\n",Buffer1,reTpHr,reTpMin,reTpSec);
										}
											if(nMSettings.gethidesmsonoff == 1 && switchonofflag==1   )
											{
											sprintf(buf,"\n\r+++++++++++++++SmsNumber[Tpsms] = %s++++++++++\n\r,line %d",SmsNumber[Tpsms],__LINE__);
											sAPI_UartPrintf(buf);
											sprintf(SmsStrNumber[NumberOfSmsNeedToSend].Smsstr,"%s\nDATE=%02d/%02d/%04d \nTIME=%02d:%02d:%02d\nOPERATED BY:%s\nFLO:%d\nM TIM=%02ld:%02ld:%02ld-%03d/%03d\n\r",(char *)Buffer1,datetime.tm_mday,datetime.tm_mon,datetime.tm_year,datetime.tm_hour, datetime.tm_min, datetime.tm_sec,StoredPhoneNumber[10],MRunflow,RunTimerhr,RunTimermin,RunTimersec,limitsmscount,limitsmsset);
											}

											else if(nMSettings.gethidesmsonoff == 0 && switchonofflag==1)
											{
											sprintf(buf,"\n\r+++++++++++++++SmsNumber[Tpsms] = %s++++++++++\n\r,line %d",SmsNumber[Tpsms],__LINE__);
											sAPI_UartPrintf(buf);
											sprintf(SmsStrNumber[NumberOfSmsNeedToSend].Smsstr,"%s\nDATE=%02d/%02d/%04d \nTIME=%02d:%02d:%02d\nOPERATED BY:%s\nFLO:%d\nM TIM=%02ld:%02ld:%02ld-%03d/%03d\n\r",(char *)Buffer1,datetime.tm_mday,datetime.tm_mon,datetime.tm_year,datetime.tm_hour, datetime.tm_min, datetime.tm_sec,WhoMadeRelayOn,MRunflow,RunTimerhr,RunTimermin,RunTimersec,limitsmscount,limitsmsset);
											}
											else
											{
									        sprintf(buf,"\n\r+++++++++++++++SmsNumber[Tpsms] = %s++++++++++\n\r ,line %d",SmsNumber[Tpsms],__LINE__);
											sAPI_UartPrintf(buf);
											if(removeniagara==0)
											{
												//MRunflow=6789;   //dg_changed
												if(sms_onflag ==1  || sms_offlag == 1 || trip_flag ==1 || PowerOnSms>=1)
												{
													sprintf(buf,"\n\r 2.TripFlag = [%d],line %d\n\r",trip_flag,__LINE__);
													sAPI_UartPrintf(buf);
												sprintf(SmsStrNumber[NumberOfSmsNeedToSend].Smsstr,"%s\nDATE=%02d/%02d/%04d \nTIME=%02d:%02d:%02d\nFLO:%d\nM TIM=%02ld:%02ld:%02ld-%03d/%03d\n\r",(char *)Buffer1,datetime.tm_mday,datetime.tm_mon,datetime.tm_year,datetime.tm_hour, datetime.tm_min, datetime.tm_sec,MRunflow,RunTimerhr,RunTimermin,RunTimersec,limitsmscount,limitsmsset);
												sprintf(buf,"\n\r+++++++++++++++SmsStrNumber[%d].Smsstr = %s++++++++++\n\r",NumberOfSmsNeedToSend,SmsStrNumber[NumberOfSmsNeedToSend].Smsstr);
												sAPI_UartPrintf(buf);
												sprintf(SmsTCPStrNumber[NumberOfSmsNeedToSend].SmsTCPstr,"");
												NumberOfSmsNeedToSend++;
												limitsmscount++;
												}
												if(sms_onflag1 ==1  || sms_offlag1 == 1 || trip_flag1 ==1)
												{
												sprintf(Buffer1,"%s",Buffer2);
												sprintf(SmsStrNumber[NumberOfSmsNeedToSend].Smsstr,"%s\nDATE=%02d/%02d/%04d \nTIME=%02d:%02d:%02d\nFLO:%d\nM TIM=%02ld:%02ld:%02ld-%03d/%03d\n\r",(char *)Buffer1,datetime.tm_mday,datetime.tm_mon,datetime.tm_year,datetime.tm_hour, datetime.tm_min, datetime.tm_sec,MRunflow,RunTimerhr,RunTimermin,RunTimersec,limitsmscount,limitsmsset);
												sprintf(buf,"\n\r+++++++++++++++SmsStrNumber[%d].Smsstr = %s++++++++++\n\r",NumberOfSmsNeedToSend,SmsStrNumber[NumberOfSmsNeedToSend].Smsstr);
												sAPI_UartPrintf(buf);
												sprintf(SmsTCPStrNumber[NumberOfSmsNeedToSend].SmsTCPstr,"");
												NumberOfSmsNeedToSend++;
												limitsmscount++;
												}
												if(sms_onflag2 ==1  || sms_offlag2 == 1 || trip_flag2 ==1)
												{
													sprintf(buf,"\n\r 3.TripFlag = [%d],line %d\n\r",trip_flag,__LINE__);
													sAPI_UartPrintf(buf);
												sprintf(Buffer1,"%s",Buffer3);
												sprintf(SmsStrNumber[NumberOfSmsNeedToSend].Smsstr,"%s\nDATE=%02d/%02d/%04d \nTIME=%02d:%02d:%02d\nFLO:%d\nM TIM=%02ld:%02ld:%02ld-%03d/%03d\n\r",(char *)Buffer1,datetime.tm_mday,datetime.tm_mon,datetime.tm_year,datetime.tm_hour, datetime.tm_min, datetime.tm_sec,MRunflow,RunTimerhr,RunTimermin,RunTimersec,limitsmscount,limitsmsset);
												sprintf(buf,"\n\r+++++++++++++++SmsStrNumber[%d].Smsstr = %s++++++++++\n\r",NumberOfSmsNeedToSend,SmsStrNumber[NumberOfSmsNeedToSend].Smsstr);
												sAPI_UartPrintf(buf);
												sprintf(SmsTCPStrNumber[NumberOfSmsNeedToSend].SmsTCPstr,"");
												NumberOfSmsNeedToSend++;
												limitsmscount++;
												}
											}
											else
											{
												if(sms_onflag ==1  || sms_offlag == 1 || trip_flag ==1 || PowerOnSms>=1)
												{
												sprintf(SmsStrNumber[NumberOfSmsNeedToSend].Smsstr,"%s\nDATE=%02d/%02d/%04d \nTIME=%02d:%02d:%02d\nFLO:%d\nM TIM=%02ld:%02ld:%02ld-%03d/%03d\n\r",(char *)Buffer1,datetime.tm_mday,datetime.tm_mon,datetime.tm_year,datetime.tm_hour, datetime.tm_min, datetime.tm_sec,MRunflow,RunTimerhr,RunTimermin,RunTimersec,limitsmscount,limitsmsset);
												sprintf(buf,"\n\r+++++++++++++++SmsStrNumber[%d].Smsstr = %s++++++++++\n\r",NumberOfSmsNeedToSend,SmsStrNumber[NumberOfSmsNeedToSend].Smsstr);
												sAPI_UartPrintf(buf);
												sprintf(SmsTCPStrNumber[NumberOfSmsNeedToSend].SmsTCPstr,"");
												NumberOfSmsNeedToSend++;
												limitsmscount++;
												
												}
												 if(sms_onflag1 ==1  || sms_offlag1 == 1 || trip_flag1 ==1)
												 {
												sprintf(Buffer1,"%s",Buffer2);
												sprintf(SmsStrNumber[NumberOfSmsNeedToSend].Smsstr,"%s\nDATE=%02d/%02d/%04d \nTIME=%02d:%02d:%02d\nM TIM=%02ld:%02ld:%02ld-%03d/%03d\n\r",(char *)Buffer1,datetime.tm_mday,datetime.tm_mon,datetime.tm_year,datetime.tm_hour, datetime.tm_min, datetime.tm_sec,RunTimerhr,RunTimermin,RunTimersec,limitsmscount,limitsmsset);
												sprintf(buf,"\n\r+++++++++++++++SmsStrNumber[%d].Smsstr = %s++++++++++\n\r",NumberOfSmsNeedToSend,SmsStrNumber[NumberOfSmsNeedToSend].Smsstr);
												sAPI_UartPrintf(buf);
												sprintf(SmsTCPStrNumber[NumberOfSmsNeedToSend].SmsTCPstr,"");
												NumberOfSmsNeedToSend++;
												limitsmscount++;
												}	
												if(sms_onflag2 ==1  || sms_offlag2 == 1 || trip_flag2 ==1)
												{
												sprintf(Buffer1,"%s",Buffer3);
												sprintf(SmsStrNumber[NumberOfSmsNeedToSend].Smsstr,"%s\nDATE=%02d/%02d/%04d \nTIME=%02d:%02d:%02d\nM TIM=%02ld:%02ld:%02ld-%03d/%03d\n\r",(char *)Buffer1,datetime.tm_mday,datetime.tm_mon,datetime.tm_year,datetime.tm_hour, datetime.tm_min, datetime.tm_sec,RunTimerhr,RunTimermin,RunTimersec,limitsmscount,limitsmsset);
												sprintf(buf,"\n\r+++++++++++++++SmsStrNumber[%d].Smsstr = %s++++++++++\n\r",NumberOfSmsNeedToSend,SmsStrNumber[NumberOfSmsNeedToSend].Smsstr);
												sAPI_UartPrintf(buf);
												sprintf(SmsTCPStrNumber[NumberOfSmsNeedToSend].SmsTCPstr,"");
												NumberOfSmsNeedToSend++;
												limitsmscount++;
												}
											}
										sprintf(buf,"\n\r Buffer1 = [%s],line %d\n\r",Buffer1,__LINE__);
										sAPI_UartPrintf(buf);
										memset(&Buffer1,0,500);
										memset(&Buffer2,0,500);
										memset(&Buffer3,0,500);
										}

									//	sprintf(buf,"\n Buffer1: %s\n\r",Buffer1);
									//	sAPI_UartPrintf(buf); 
										
										sprintf(SmsStrNumber[NumberOfSmsNeedToSend].Smsnumber,"%s",SmsNumber[Tpsms]);    //DG_CHANGED
										//sprintf(SmsStrNumber[NumberOfSmsNeedToSend].Smsnumber,"%s%s",StoredPhoneSmscode[Tpsms],SmsNumber[Tpsms]);

										
									//	number_enu_data=0;
										if(sms_onflag==1)
										{
										nSTATE_MOTOR1_ON_SMS=STATE_MOTOR1_DUMMY  ;//STATE_MOTOR1_ON_DEFAULT;
										sms_onflag=0;
										}
										if(sms_offlag==1 || trip_flag==1)
										{
										nSTATE_MOTOR1_SMS=STATE_NO_MOTOR1_SMS;
										sms_offlag=0;
										//trip_flag=0;
										}
										if(sms_onflag1==1 )
										{
										nSTATE_MOTOR2_ON_SMS=STATE_MOTOR2_DUMMY  ;//STATE_MOTOR2_ON_DEFAULT;
										sms_onflag1=0;
										
										}
										if(sms_offlag1==1 || trip_flag1==1)
										{
										nSTATE_MOTOR2_SMS=STATE_NO_MOTOR2_SMS;
										sms_offlag1=0;
										//trip_flag1=0;
										}
										if(sms_onflag2==1)
										{
										nSTATE_MOTOR3_ON_SMS=STATE_MOTOR3_DUMMY  ;//STATE_MOTOR3_ON_DEFAULT;
										sms_onflag2=0;
										}
										if(sms_offlag2==1 || trip_flag2==1) 
										{
										nSTATE_MOTOR3_SMS=STATE_NO_MOTOR3_SMS;
										sms_offlag2=0;
										//trip_flag2=0;
										}
									}
									sprintf(buf,"\n\rSER:%d,%d\n\r",ServiceNumberFound,nMSettings.serviceOnOff );
									sAPI_UartPrintf(buf);
									if(ServiceNumberFound == 1 && nMSettings.serviceOnOff==1)
									{
									   if (CheckVoliedPhone(ServiceNumber) == 0)
										{

										}
										else if (strstr(ServiceNumber,StoredPhoneNumber[11]) != 0)
										{

										}
										else
										{
										sAPI_GetRealTimeClock(&datetime);
										runtime();
										
										if(zonecom.standalonemodeonof==1&& MakeRealyOn==0)
										{
										revalue = nVaTr.struntime;
										reTpHr = revalue/3600;
										revalue = revalue%3600;
										reTpMin = revalue/60;
										revalue = revalue%60;
										reTpSec = revalue;
                                        if(Appmodeon==1	)
										sprintf(Buffer1,"%s\nST TIM=%02d:%02d:%02d\n",Buffer1,reTpHr,reTpMin,reTpSec);
										}

											if(nMSettings.gethidesmsonoff == 1 && switchonofflag==1)
											{
											sprintf(buf,"\n\r+++++++++++++++SmsNumber[Tpsms] = %s++++++++++\n\r",SmsNumber[Tpsms]);
											sAPI_UartPrintf(buf);
											sprintf(SmsStrNumber[NumberOfSmsNeedToSend].Smsstr,"%s\nDATE=%02d/%02d/%04d \nTIME=%02d:%02d:%02d\nOPERATED BY:%s\nFLO:%d\nM TIM=%02ld:%02ld:%02ld-%03d/%03d\n\r",(char *)Buffer1,datetime.tm_mday,datetime.tm_mon,datetime.tm_year,datetime.tm_hour, datetime.tm_min, datetime.tm_sec,StoredPhoneNumber[10],MRunflow,RunTimerhr,RunTimermin,RunTimersec,limitsmscount,limitsmsset);
											}

											else if(nMSettings.gethidesmsonoff == 0 && switchonofflag==1)
											{
											sprintf(buf,"\n\r+++++++++++++++SmsNumber[Tpsms] = %s++++++++++\n\r",SmsNumber[Tpsms]);
											sAPI_UartPrintf(buf);
											sprintf(SmsStrNumber[NumberOfSmsNeedToSend].Smsstr,"%s\nDATE=%02d/%02d/%04d \nTIME=%02d:%02d:%02d\nOPERATED BY:%s\nFLO:%d\nM TIM=%02ld:%02ld:%02ld-%03d/%03d\n\r",(char *)Buffer1,datetime.tm_mday,datetime.tm_mon,datetime.tm_year,datetime.tm_hour, datetime.tm_min, datetime.tm_sec,WhoMadeRelayOn,MRunflow,RunTimerhr,RunTimermin,RunTimersec,limitsmscount,limitsmsset);
											}
											else
											{
											sprintf(buf,"\n\r+++++++++++++++SmsNumber[Tpsms] = %s++++++++++\n\r",SmsNumber[Tpsms]);
											sAPI_UartPrintf(buf);
											if(removeniagara==0)
											{   // dg_changed
										//	MRunflow=5678; // dg_changed
											sprintf(SmsStrNumber[NumberOfSmsNeedToSend].Smsstr,"%s\nDATE=%02d/%02d/%04d \nTIME=%02d:%02d:%02d\nFLO:%d\nM TIM=%02ld:%02ld:%02ld-%03d/%03d\n\r",(char *)Buffer1,datetime.tm_mday,datetime.tm_mon,datetime.tm_year,datetime.tm_hour, datetime.tm_min, datetime.tm_sec,MRunflow,RunTimerhr,RunTimermin,RunTimersec,limitsmscount,limitsmsset);
											}  // dg_changed
											else
											sprintf(SmsStrNumber[NumberOfSmsNeedToSend].Smsstr,"%s\nDATE=%02d/%02d/%04d \nTIME=%02d:%02d:%02d\nM TIM=%02ld:%02ld:%02ld-%03d/%03d\n\r",(char *)Buffer1,datetime.tm_mday,datetime.tm_mon,datetime.tm_year,datetime.tm_hour, datetime.tm_min, datetime.tm_sec,RunTimerhr,RunTimermin,RunTimersec,limitsmscount,limitsmsset);

											}

										    sprintf((char *)SmsStrNumber[NumberOfSmsNeedToSend].Smsnumber,"%s%s",StoredPhoneSmscode[11],ServiceNumber);
										sprintf(SmsTCPStrNumber[NumberOfSmsNeedToSend].SmsTCPstr,"");
										limitsmscount++;
										NumberOfSmsNeedToSend++;
										
                                        }
									}
								if(l_currentSec == 86400 || l_currentSec == 0)
								{
									memset(&s_nlogtime[0].Act_Mtr1_On,0,sizeof(s_nlogtime[0].Act_Mtr1_On));
									//memset(&s_nlogtime[0].Act_Mtr1_Off,0,sizeof(s_nlogtime[0].Act_Mtr1_Off));
									memset(&s_nlogtime[1].Act_Mtr2_On,0,sizeof(s_nlogtime[1].Act_Mtr2_On));
									//memset(&s_nlogtime[1].Act_Mtr2_Off,0,sizeof(s_nlogtime[1].Act_Mtr2_Off));
									memset(&s_nlogtime[2].Act_Mtr3_On,0,sizeof(s_nlogtime[2].Act_Mtr3_On));
									//memset(&s_nlogtime[2].Act_Mtr3_Off,0,sizeof(s_nlogtime[2].Act_Mtr3_Off));
									on_data_M1=0;off_data_M1=0;on_data_M2=0;off_data_M2=0;on_data_M3=0;off_data_M3=0;
								}
							}
							
							

							sprintf(SMS_BUF,"%s\nDATE=%02d/%02d/%04d\nTIME=%02d:%02d:%02d\nFLO:%d\nM TIM=%02ld:%02ld:%02ld-%03d/%03d\n\r",(char *)Buffer1,datetime.tm_mday,datetime.tm_mon,datetime.tm_year,datetime.tm_hour, datetime.tm_min, datetime.tm_sec,MRunflow,RunTimerhr,RunTimermin,RunTimersec,limitsmscountsmsm,limitsmsset);
							////eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 4, "SMST", EAT_NULL);
						}
							sAPI_UartPrintf("SMS RESET\n\r");
				NumberOfSMSSend = 20;
				//	if(NumberOfSmsNeedToSend>=1)
				//	SendSmsToAll=1;
				//	else
				SendSmsToAll = 0;
				//	SendSmsToAll = 0;
				ThisSMSisNotPowerFault = 0;
				NeedToSendSMSCall = 0;
				PowerOnSms = 0;
				// AllSmsSendDone = 1;
				SMS30MinStatus = 0;
				CallOnOfVer = 0;
				RegxSmsSend = 0;
				PowerOffSMS = 0;
				STATE_SENDSMS = STATE_NO_SMS;
				//		nSTATE_MOTOR_SMS = STATE_NO_MOTOR_SMS;  //dg_nsdk
				// nSTATE_MOTOR_ON_SMS=STATE_MOTOR_ON_DEFAULT;
			}
			else if (ModemIsReady == 1 && nMSettings.SMSOnOff == 0)
			{
				sAPI_UartPrintf("SMS RESET1\n\r");
				NumberOfSMSSend = 20;
				SendSmsToAll = 0;
				ThisSMSisNotPowerFault = 0;
				NeedToSendSMSCall = 0;
				PowerOnSms = 0;
				// AllSmsSendDone = 1;
				SMS30MinStatus = 0;
				CallOnOfVer = 0;
				RegxSmsSend = 0;
				PowerOffSMS = 0;
				STATE_SENDSMS = STATE_NO_SMS;
				//	nSTATE_MOTOR_SMS = STATE_NO_MOTOR_SMS; //dg_nsdk
				NumberOfSmsAllreadySend = NumberOfSmsNeedToSend = 0;
				SendSMS = 0;
			}

			if (CurrentSec >= 86397 && CurrentSec <= 86400 || CurrentSec == 0)
			{
				sAPI_UartPrintf("\nbuffer cleared\n");
				for (char i = 0; i < 25; i++)
				{
					// memset(&s_nlogtime,0,1800);
					memset(&s_nlogtime[i].Act_Mtr1_On, 0, sizeof(s_nlogtime[i].Act_Mtr1_On));
					// memset(&s_nlogtime[i].Act_Mtr1_Off,0,sizeof(s_nlogtime[i].Act_Mtr1_Off));
					memset(&s_nlogtime[i].Act_Mtr2_On, 0, sizeof(s_nlogtime[i].Act_Mtr2_On));
					// memset(&s_nlogtime[i].Act_Mtr2_Off,0,sizeof(s_nlogtime[i].Act_Mtr2_Off));
					memset(&s_nlogtime[i].Act_Mtr3_On, 0, sizeof(s_nlogtime[i].Act_Mtr3_On));
					// memset(&s_nlogtime[i].Act_Mtr3_Off,0,sizeof(s_nlogtime[i].Act_Mtr3_Off));
				}
				on_data_M1 = 0;
				off_data_M1 = 0;
				on_data_M2 = 0;
				off_data_M2 = 0;
				on_data_M3 = 0;
				off_data_M3 = 0;
			}
			sAPI_GetRealTimeClock(&datetime);
			CurrentSec = (datetime.tm_hour * 3600) + (datetime.tm_min * 60) + datetime.tm_sec;
			if (CurrentSec != prev_sec)
			{
				unsigned int Time_var = 0, Time_var1 = 0, Time_var2 = 0;
				//if (Time_flag == 1)
				if(Time_flag == 1 && PowerCurrentCondition == 0 && Motoronflag[0] == 1)
				{
					if (Time_Counter >= 86400)
						Time_Counter = 0;
					Time_Counter++;
					Time_Hr = Time_Counter / 3600;
					Time_var = Time_Counter - (Time_Hr * 3600);
					Time_Min = Time_var / 60;
					Time_Sec = Time_var - (Time_Min * 60);
				}
				if (Time_flag1 == 1)
				{
					if (Time_Counter1 >= 86400)
						Time_Counter1 = 0;
					Time_Counter1++;
					Time_Hr1 = Time_Counter1 / 3600;
					Time_var1 = Time_Counter1 - (Time_Hr1 * 3600);
					Time_Min1 = Time_var1 / 60;
					Time_Sec1 = Time_var1 - (Time_Min1 * 60);
				}
				if (Time_flag2 == 1)
				{
					if (Time_Counter2 >= 86400)
						Time_Counter2 = 0;
					Time_Counter2++;
					Time_Hr2 = Time_Counter2 / 3600;
					Time_var2 = Time_Counter2 - (Time_Hr2 * 3600);
					Time_Min2 = Time_var2 / 60;
					Time_Sec2 = Time_var2 - (Time_Min2 * 60);
				}
				prev_sec = CurrentSec;
			}

#if 0		
						if(CallConnected == 1 || CallModeOn==1)
						{

							sprintf(buf,"\n\rCALL CONNECTED  CallConnectedDelay = %d\n\r",CallConnectedDelay);
							sAPI_UartPrintf(buf);
							CallConnectedDelay++;
							if(addcall==1 && CallConnectedDelay==10)
							{
								//eat_send_msg_to_user(EAT_USER_0, EAT_USER_1, EAT_FALSE, 6, "ATD121", EAT_NULL);
							}
							if(addcall==1 && CallConnectedDelay>30)
							{

									//sprintf(buffer,"AT+CHLD=3\n\r");
								//sprintf(buf,"\n\r%s\n\r",buffer);
								//sAPI_UartPrintf(buf);	
								//eat_modem_write((UINT8*)buffer, strlen(buffer));
								//eat_send_msg_to_user(EAT_USER_0, EAT_USER_1, EAT_FALSE, 4, "CHLD", EAT_NULL);
								addcall=0;
							}
							if(CallConnectedDelay>=500)
							{
							//	sprintf(buffer,"ATH\n\r");
							//	sprintf(buf,"\n\r%s\n\r",buffer);
							//sAPI_UartPrintf(buf);
							  //eat_modem_write((UINT8*)buffer, strlen(buffer));
								sprintf(buf,"LINE no is %d",__LINE__);
								sAPI_UartPrintf(buf);
								//eat_send_msg_to_user(EAT_USER_0, EAT_USER_1, EAT_FALSE, 3, "ATH", EAT_NULL);
								CallConnectedDelay = 0;
								CallConnected = 0;
								IamCalling = 0;
								GiveCallToNumber = 0;
								WaitOK = 0;
								Callreceiv = 0;
								RingVerDelay = 0;
								sAPI_UartPrintf("\n\rCall Ended\n\r");								 
								CallModeOn = 0;
								HowManySoundToPlay = HowManySoundPlayed = 0;
								PlaySoundVer=0;
								PlaySound = 0;
							}
						}
#endif
			if (nMSettings.ndebugonof == 1)
				sAPI_UartPrintf("Noofsettingsprocessed:%d,Noofsettingsrecvd:%d", Noofsettingsprocessed, Noofsettingsrecvd);

			if (Noofsettingsprocessed != Noofsettingsrecvd)
			{
				SendWIFI++;
				if (SendWIFI > 3)
				{
					wifi_process_function();
					SendWIFI = 0;
				}
			}
			else if (Noofsettingsprocessed >= Noofsettingsrecvd) // dg_changed from ==
			{
				Noofsettingsprocessed = Noofsettingsrecvd = 0;
				SendWIFI = 0;
			}

			sprintf(buf, "\n\rtest=%d,%d,%d,%d\n\r", NumberOfSmsAllreadySend, NumberOfSmsNeedToSend, CallModeOn, GiveCallToNumber);
			sAPI_UartPrintf(buf);
			if (NumberOfSmsAllreadySend != NumberOfSmsNeedToSend && CallModeOn == 0 && GiveCallToNumber == 0)
			{

				// int count;
				AllSmsSendDone = 0;
				SendSMS++;
				sprintf(buf, "\n\rSendSMS=%d", SendSMS);
				sAPI_UartPrintf(buf);
				// limitsmsset=100;
				if ((SendSMS > 15) && (wifiopenflag == 0 || tcpopenflag == 1)) // un_cmt aj
				{

					SendSMS = 0;
					sprintf(buf, "limitsmscount:[%d],limitsmsset:[%d]", limitsmscount, limitsmsset); // aj added
					sAPI_UartPrintf(buf);
					if (limitsmscount < limitsmsset)
					{
						sprintf(buf, "\n\rNumberOfSmsNeedToSend= %d\n\r", NumberOfSmsNeedToSend);
						sAPI_UartPrintf(buf);
						sprintf(buf, "\n\rNumberOfSmsAllreadySend= %d\n\r", NumberOfSmsAllreadySend);
						sAPI_UartPrintf(buf);
						strcpy(SendSMSString, SmsStrNumber[NumberOfSmsAllreadySend].Smsstr);
						// sAPI_UartWrite(eat_uart_wifi, SendSMSString,strlen(SendSMSString));
						// sprintf(buf,"\n\rSmsStrNumber[%d].Smsstr = %s,SMS String :- %s\n\r",NumberOfSmsAllreadySend,SmsStrNumber[NumberOfSmsAllreadySend].Smsstr,SendSMSString);
						//  sAPI_UartPrintf(buf);
						strcpy(SendSMSOnThisNumber, SmsStrNumber[NumberOfSmsAllreadySend].Smsnumber);
						// sprintf(buf,"\n\rNumberOfSmsAllreadySend= %s\n\r",SendSMSOnThisNumber);
						//  sAPI_UartPrintf(buf);
						sprintf(buf, "SMSString length = %d\n\r", strlen(SendSMSString));
						sAPI_UartPrintf(buf);
						//			simcom_sms_send(SendSMSOnThisNumber,SendSMSString,strlen(SendSMSString));
					}
					NumberOfSmsAllreadySend++;
					tcpcopyflag = 0;
					// tcpdcounter=0;
				}
				else
				{
					if (wifiopenflag == 1 && tcpopenflag == 0 && rebootflag == 0 && wifimodesetfalg == 0)
					{
						char getbuff[500];
						int contentlen;
						if (SendSMS > 3)
						{
							j = NumberOfSmsAllreadySend;
							for (i = j; i < NumberOfSmsNeedToSend; i++)
							{

								sprintf(buf, "\n\rwifiNumberOfSmsNeedToSend= %d\n\r", NumberOfSmsNeedToSend);
								sAPI_UartPrintf(buf);
								sprintf(buf, "\n\rwifiNumberOfSmsAllreadySend= %d\n\r", NumberOfSmsAllreadySend);
								sAPI_UartPrintf(buf);
								sprintf(SendSMSString, "%s\n$D,%s", IMEI, SmsStrNumber[NumberOfSmsAllreadySend].Smsstr);
								memset(&getbuff, 0, 350);
								if (NumberOfSmsAllreadySend != 0 && (strlen(SmsStrNumber[NumberOfSmsAllreadySend].Smsstr) == strlen(SmsStrNumber[NumberOfSmsAllreadySend - 1].Smsstr)))
								{
									contentlen = strlen(SmsStrNumber[NumberOfSmsAllreadySend].Smsstr);
									contentlen = contentlen - 9;
									strncpy(getbuff, SmsStrNumber[NumberOfSmsAllreadySend - 1].Smsstr, contentlen);
								}
								else
									strcpy(getbuff, "**********");
								contentlen = strlen(getbuff);
								if (nMSettings.ndebugonof == 1)
								{
									sprintf(buf, "tcp copy length1=%d", contentlen);
									sAPI_UartPrintf(buf);
									sprintf(buf, "tcp copy length1=%s", getbuff);
									sAPI_UartPrintf(buf);
								}
								contentlen = strlen(SmsStrNumber[NumberOfSmsAllreadySend].Smsstr);
								if (nMSettings.ndebugonof == 1)
								{
									sprintf(buf, "tcp copy length1=%d", contentlen);
									sAPI_UartPrintf(buf);
									sprintf(buf, "tcp copy length2=%s", SmsStrNumber[NumberOfSmsAllreadySend].Smsstr);
									sAPI_UartPrintf(buf);
								}
								if (strlen(getbuff) < strlen(SmsStrNumber[NumberOfSmsAllreadySend].Smsstr))
								{
									if (strstr(SmsStrNumber[NumberOfSmsAllreadySend].Smsstr, getbuff) != 0)
									{
										if (nMSettings.ndebugonof == 1)
										{
											sprintf(buf, "tcp copy length3=%s", getbuff);
											sAPI_UartPrintf(buf);
											sprintf(buf, "tcp copy length4=%s", SmsStrNumber[NumberOfSmsAllreadySend].Smsstr);
											sAPI_UartPrintf(buf);
										}
										// strcpy(SendSMSString,SmsStrNumber[NumberOfSmsAllreadySend].Smsstr);
										// sAPI_UartWrite(eat_uart_wifi, SendSMSString,strlen(SendSMSString));
									}
									else
										sAPI_UartWrite(eat_uart_wifi, SendSMSString, strlen(SendSMSString));
								}
								else
									sAPI_UartWrite(eat_uart_wifi, SendSMSString, strlen(SendSMSString));
								sprintf(buf, "SMSString length = %d\n\r", strlen(SendSMSString));
								sAPI_UartPrintf(buf);
								NumberOfSmsAllreadySend++;
							}
							NumberOfSmsAllreadySend = NumberOfSmsNeedToSend;
							tcpcopyflag = 0;
							SendSMS = 0;
						}
					}
					else
					{
						if (tcpcopyflag == 0 && Nooftcprecvd <= 49) // 150
						{
							char getbuff[500];
							char getbuff1[200];
							int contentlen;
							unsigned char sendtcp = 0;
							UINT8 *k = EAT_NULL;

							int len = 0;
							sAPI_GetRealTimeClock(&datetime);
							if (!strcmp(DIS_BUF, "      "))
								strcpy(DIS_BUF, "$L,     NO DATA ,  ");

							if (!strcmp(VAL_BUF, "       "))
								strcpy(VAL_BUF, "$V,0,000   ");
							strcpy(SMS_BUF, SmsStrNumber[NumberOfSmsAllreadySend].Smsstr);
							if (!strcmp(SMS_BUF, "       "))
								strcpy(SMS_BUF, "     NO DATA    ");
							memset(&getbuff, 0, 350);
							sprintf(buf, "SMS_BUF copy =%s", SMS_BUF);
							sAPI_UartPrintf(buf);
							k = SMS_BUF;
							len = strlen(SMS_BUF);
							for (i = 0; i < len; i++)
							{
								if (*k == '\n')
									*k = ' ';
								else if (*k == '\r')
									*k = ' ';

								k++;
							}
							sprintf(buf, "SMS_BUF copy =%s", SMS_BUF);
							sAPI_UartPrintf(buf);
							sprintf(buf, "DIS_BUF copy =%s", DIS_BUF);
							sAPI_UartPrintf(buf);
							if (DIS_BUF[0] == '$')
							{
								k = DIS_BUF;
								len = strlen(DIS_BUF);
								for (i = 0; i < len; i++)
								{
									if (*k == '\n')
										*k = ' ';
									else if (*k == '\r')
										*k = ' ';
									k++;
								}
							}
							else
							{
								strcpy(DIS_BUF, "$L,     NO DATA,    ");
							}
							sprintf(buf, "DIS_BUF copy =%s", DIS_BUF);
							sAPI_UartPrintf(buf);
							sprintf(buf, "VAL_BUF copy =%s", VAL_BUF);
							sAPI_UartPrintf(buf);
							if (VAL_BUF[0] == '$')
							{
							}
							else
							{
								strcpy(VAL_BUF, "$V,0,000    ");
							}
							sprintf(buf, "VAL_BUF copy =%s", VAL_BUF);
							sAPI_UartPrintf(buf);
							if (strstr((char *)SMS_BUF, "+CUSD:") != 0)
							{
							}
							else
							{
								sendtcp = 0;
								if (NumberOfSmsAllreadySend != 0 && (strlen(SmsStrNumber[NumberOfSmsAllreadySend].Smsstr) == strlen(SmsStrNumber[NumberOfSmsAllreadySend - 1].Smsstr)))
								{
									contentlen = strlen(SmsStrNumber[NumberOfSmsAllreadySend].Smsstr);
									contentlen = contentlen - 9;
									strncpy(getbuff, SmsStrNumber[NumberOfSmsAllreadySend - 1].Smsstr, contentlen);
								}
								else
									strcpy(getbuff, "**********");
								contentlen = strlen(getbuff);
								if (nMSettings.ndebugonof == 1)
								{
									sprintf(buf, "tcp copy length1=%d", contentlen);
									sAPI_UartPrintf(buf);
									sprintf(buf, "tcp copy length1=%s", getbuff);
									sAPI_UartPrintf(buf);
								}
								contentlen = strlen(SmsStrNumber[NumberOfSmsAllreadySend].Smsstr);
								if (nMSettings.ndebugonof == 1)
								{
									sprintf(buf, "tcp copy length1=%d", contentlen);
									sAPI_UartPrintf(buf);

									sprintf(buf, "tcp copy length2=%s", SmsStrNumber[NumberOfSmsAllreadySend].Smsstr);
									sAPI_UartPrintf(buf);
								}
								if (strlen(getbuff) < strlen(SmsStrNumber[NumberOfSmsAllreadySend].Smsstr))
								{
									if (strstr(SmsStrNumber[NumberOfSmsAllreadySend].Smsstr, getbuff) != 0)
									{
										if (nMSettings.ndebugonof == 1)
										{
											sprintf(buf, "tcp copy length3=%s", getbuff);
											sAPI_UartPrintf(buf);
											sprintf(buf, "tcp copy length4=%s", SmsStrNumber[NumberOfSmsAllreadySend].Smsstr);
											sAPI_UartPrintf(buf);
										}
										sendtcp = 0;
										// strcpy(SendSMSString,SmsStrNumber[NumberOfSmsAllreadySend].Smsstr);
										//	sAPI_UartWrite(eat_uart_wifi, SendSMSString,strlen(SendSMSString));
									}
									else
									{
										sendtcp = 1;
										sprintf(buf, "\n\rsendtcp:%d", sendtcp);
										sAPI_UartPrintf(buf);
									}
								}
								else
								{
									sendtcp = 1;
									sprintf(buf, "\n\resendtcp:%d", sendtcp);
									sAPI_UartPrintf(buf);
								}
								if (sendtcp == 1)
								{
									//	sprintf(getbuff,"{\"ctrlQrcode\":\"%s\",\"ctrlMsg\":\"%s\",\"ctrlDate\":\"%d/%d/20%02d\",\"ctrlTime\":\"%d:%d:%d\"}\r\n",IMEI,datetime.tm_mday,datetime.tm_mon,datetime.tm_year,datetime.tm_hour,datetime.tm_min,datetime.tm_sec);
									sprintf(getbuff, "{\r\n\"cC\":\"%s\",\r\n\"cM\":\"%s %s\",\r\n\"cL\":\"%s\",\r\n\"cZ\":\"%s\",\r\n\"cD\":\"%04d-%02d-%02d\",\r\n\"cT\":\"%02d:%02d:%02d\",\r\n\"mC\":\"SMS\"\r\n}", IMEI, SmsTCPStrNumber[NumberOfSmsAllreadySend].SmsTCPstr, SMS_BUF, DIS_BUF, VAL_BUF, datetime.tm_year, datetime.tm_mon, datetime.tm_mday, datetime.tm_hour, datetime.tm_min, datetime.tm_sec);
									//	sprintf(getbuff,"{\r\n\"cC\":\"%s\",\r\n\"cM\":\"%s\",\r\n\"cL\":\"HELLO\",\r\n\"cZ\":\"%s\",\r\n\"cD\":\"%d/%d/20%02d\",\r\n\"cT\":\"%d:%d:%d\",\r\n\"mC\":\"SMS\"\r\n}",IMEI,SMS_BUF,VAL_BUF,datetime.tm_mday,datetime.tm_mon,datetime.tm_year,datetime.tm_hour,datetime.tm_min,datetime.tm_sec);
									sprintf(buf, "\n\rGETBUFF:%s", getbuff);
									sAPI_UartPrintf(buf);
									// sprintf(getbuff,"{ }\n\r");
									contentlen = strlen(getbuff);
									contentlen = contentlen;
									// MQTTpublish_server(contentlen,getbuff);             //aj_mqtt_added
									sprintf(buf, "tcp copy length=%d", contentlen);
									sAPI_UartPrintf(buf);
									if (nMSettings.ndebugonof == 1)
									{
										sprintf(buf, "getbuff copy =%s", getbuff);
										sAPI_UartPrintf(buf);
									}
									// memset(&TCPwifiStrNumber[Nooftcprecvd].TCPWifistr,0,500);

									sprintf(buf, "\n\rNooftcprecvd =%d", Nooftcprecvd);
									sAPI_UartPrintf(buf);

									sprintf(TCPwifiStrNumber[Nooftcprecvd].TCPWifigprsstr, "%s", getbuff);
									// sprintf(TCPwifiStrNumber[Nooftcprecvd].TCPWifistr,"POST /api/v1/controller/messages/ HTTP/1.1\r\nHost: %s:8080\r\nUser-Agent: curl/7.52.1\r\nAccept:*\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n\%s\r\n",DeviceConfig.TcpServerIP,contentlen,getbuff);      //aj_cmted
									// sprintf(TCPwifiStrNumber[Nooftcprecvd].TCPWifistr,"POST /api/v1/getlivestatus/ HTTP/1.1\r\nHost: %s:8091\r\nUser-Agent: curl/7.52.1\r\nAccept:*\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n\%s\r\n",DeviceConfig.TcpServerIP,contentlen,getbuff);
									if (nMSettings.ndebugonof == 1)
									{
										sprintf(buf, "getbuff copy =%s", TCPwifiStrNumber[Nooftcprecvd].TCPWifigprsstr);
										sAPI_UartPrintf(buf);
									}
									memset(&getbuff, 0, 500);
									contentlen = strlen(TCPwifiStrNumber[Nooftcprecvd].TCPWifistr);
									sprintf(buf, "tcp copy length=%d", contentlen);
									sAPI_UartPrintf(buf);
									Nooftcprecvd++;
								}
								tcpcopyflag = 1;
								//	tcpdcounter=0;
								//	sprintf(buf,"tcp copy=%s",TCPwifiStrNumber[Nooftcprecvd].TCPWifistr);
								// sAPI_UartPrintf(buf);
								if (limitsmscount > limitsmsset)
								{
									NumberOfSmsAllreadySend++;
									tcpcopyflag = 0;
								}
							}
						}
					}
				}
			}
			sAPI_UartPrintf("\n\rtest1=%d,%d,%d,%d,%d,%d,%d\n\r", Nooftcpprocessed, Nooftcprecvd, tcpcopyflag, wifiopenflag, tcpopenflag, Nooftcpprocessed1, Nooftcprecvd1);

			gettcpcounter++;
			if (nMSettings.ndebugonof == 1)
				sprintf(buf, "******************tcp copy=%d,%d,*************************", gettcpcounter, tcpdcounter);
			sAPI_UartPrintf(buf);
			//  if(DeviceConfig.interface == WIFI && tcpopenflag==1 && wifiopenflag==1 && (getdataflag==0||senddataflag==0) && gettcpcounter>=20 && clostcpcounter<=10 )

			//	else if(DeviceConfig.interface == WIFI&&tcpopenflag==1&&wifiresetflag1==0&&wifiresetflag==0 && wifiopenflag==0 && (getdataflag1==0||senddataflag==0) )
			
			sAPI_UartPrintf("\n\rtest1= %d, %d,%d,%d,%d, %d, %d  %d :%d  %d  %d [%d %d %d]\n\r", Nooftcpprocessed, Nooftcprecvd, tcpcopyflag, wifiopenflag, tcpopenflag, Nooftcpprocessed1, Nooftcprecvd1,livedataflag,livedataflag1,s_nTimerSettings.m_Enter,DeviceConfig.interface,fotaflsg,CallConnected,ringflag);
			if (DeviceConfig.interface == WIFI && tcpopenflag == 1 && wifiresetflag1 == 0 && wifiresetflag == 0 && wifiopenflag == 0)

			{
				// tcpdcounter++;

				tcpdcounter1++;
				if (tcpdcounter1 > 30)
				{
					// eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 6, "CONFIG", EAT_NULL);
					sprintf(buf, "tcp copy done222222222222222222222222222222222222222222222");
					sAPI_UartPrintf(buf);
					// wifiopenflag=1;
					tcpdcounter1 = 0;
					getdataflag1 = 1;
				}
			}
			else if (DeviceConfig.interface == WIFI && tcpopenflag == 1 && wifiopenflag == 1 && clostcpcounter >= 50)
			{
				// clostcpcounter=0;
				if (tcpdcounter++ > 8)
				{
					sprintf(buf, "tcp copy done333333333333333333333333333333333333");
					sAPI_UartPrintf(buf);
					tcpdcounter = 0;
					clostcpcounter = 0;
					wifiopenflag = 0;
					// eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 5, "CLOSE", EAT_NULL);
				}
			}

			
	//		else if (((Nooftcpprocessed < Nooftcprecvd) || (Nooftcpprocessed1 < Nooftcprecvd1) || livedataflag == 1 || livedataflag1 == 1) && s_nMSettings.m_Enter > 70)
		//	 else if(((Nooftcpprocessed < Nooftcprecvd)||(Nooftcpprocessed1 < Nooftcprecvd1)|| livedataflag==1||livedataflag1==1)&& s_nMSettings.m_Enter>=71)  //dg_nsdk
	
		 else if(((Nooftcpprocessed < Nooftcprecvd)||(Nooftcpprocessed1 < Nooftcprecvd1)|| livedataflag==1||livedataflag1==1))
			{
				char getbuff[1500]={0};//900//800
					      char getbuff1[450]={0};char getbuff2[450]={0};char getbuff3[250]={0};char getbuff4[300]={0};char getbuff5[300]={0};//250

				int contentlen;
				memset(getbuff, NULL, sizeof(getbuff) + 1);
				memset(getbuff1, NULL, sizeof(getbuff1) + 1);
				memset(getbuff2, NULL, sizeof(getbuff2) + 1);
				memset(getbuff3, NULL, sizeof(getbuff3) + 1);
				memset(getbuff4, NULL, sizeof(getbuff4) + 1);
				sprintf(buf, "\n\r tcp copy=ok2");
				sAPI_UartPrintf(buf);
				if (DeviceConfig.interface == WIFI)
				{
#if 0 // dg_nsdkkk
							//sprintf(buf,"tcp copy=ok3");
							//sAPI_UartPrintf(buf);
							 if(Nooftcpprocessed!=prvNooftcpprocessed)
							 {
								prvNooftcpprocessed= Nooftcpprocessed;
								Nooftcpsendcount=0;

							 }

							if(tcpopenflag==1 && wifiopenflag==1 && clostcpcounter<=50 )    // changed from 10
							{
					//		#if 0 //dg_nsdkkk
							if(tcpdcounter++>20)
							{
							sprintf(buf,"\n\rNooftcpprocessed= %d\n\r",Nooftcpprocessed);
							sAPI_UartPrintf(buf);
							sprintf(buf,"\n\rNooftcprecvd= %d\n\r",Nooftcprecvd);
							sAPI_UartPrintf(buf);
		                    clostcpcounter++;
						    //	strcpy(SendSMSString,TCPwifiStrNumber[Nooftcpprocessed].TCPWifistr);
						    //	livedataflag=1;
							if(livedataflag ==0 && livedataflag1==0)
							{
							//gettcpcounter=0;
							if(nMSettings.ndebugonof==1)
							{
							sprintf(buf,"tcp copy=%s",TCPwifiStrNumber[Nooftcpprocessed].TCPWifistr);
							sAPI_UartPrintf(buf);
							sprintf(buf,"tcp copy done7777777777777777777777777777777777777777777");
							sAPI_UartPrintf(buf);
							sprintf(buf,"SMSString length = %d\n\r",strlen(SendSMSString));
							sAPI_UartPrintf(buf);
							}
							if(Nooftcprecvd1>Nooftcpprocessed1)
							{
							Http_send_flag=0;
							sAPI_UartWrite(eat_uart_wifi, TCPwifiStrNumber1[Nooftcpprocessed1].TCPWifistr1,strlen(TCPwifiStrNumber1[Nooftcpprocessed1].TCPWifistr1));
							Nooftcpprocessed1++;
							}
							else
							{
							Http_send_flag=1;
							sAPI_UartWrite(eat_uart_wifi, TCPwifiStrNumber[Nooftcpprocessed].TCPWifistr,strlen(TCPwifiStrNumber[Nooftcpprocessed].TCPWifistr));
							Nooftcprecvd1=Nooftcpprocessed1=0;
							}
							//	sprintf(buf,"tcp copy done7777777777777777777777777777777777777777777");
							// sAPI_UartPrintf(buf);
							//	sprintf(buf,"SMSString length = %d\n\r",strlen(SendSMSString));
							// sAPI_UartPrintf(buf);
							//	//eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 7, "SENDTCP", EAT_NULL);
							}
						else
						{
							INT32 ret;
	                        UINT8  *k = EAT_NULL;
	                        char TpStr4[50] = "nimish";
	                        long TpHr,TpMin,TpSec,value;
	                        long runvtim,runvflow;
							float pervtim;
							int pervflow;
							//if(livedataflag==0)
							//Nooftcprecvd++;
							livedataflagcount1++;
							if(livedataflagcount1>2 && livedataflag1>0)
							{
							livedataflagcount1=4;
							livedataflag1=0;
						

							}
							gettcpcounter=0;
							revalue = zone[nVaTr.Currentvalve].stoptime;
							reTpHr = revalue/3600;
							revalue = revalue%3600;
							reTpMin = revalue/60;
							revalue = revalue%60;
							reTpSec = revalue;
							pervtim = ((zoneonof[nVaTr.programselection].timerpercent)*0.01);
							runvtim=((zone[nDripSettings.changeinstp].actthr*3600)+(zone[nDripSettings.changeinstp].acttmin*60));
							runvtim = (long)(pervtim*runvtim);
                            pervflow = ((zoneonof[nVaTr.programselection].flowpercent)*0.01);
                            runvflow=zone[nDripSettings.stp].actflowrate;
							runvflow = (long)(pervflow*runvflow);
	                      //  BigSMS[0] = 0;
							/* if(zonecom.standalonemodeonof==1)
							value = 0;
						    else */
							value = nVaTr.REMTIM;
							if(value<0)
								value=0;

							TpHr = value/3600;
							value = value%3600;
							TpMin = value/60;
							value = value%60;
							TpSec = value;

							/* if(zonecom.standalonemodeonof==1)
							revalue = nVaTr.struntime;
						    else */
							revalue = runvtim;

							reTpHr = revalue/3600;
							revalue = revalue%3600;
							reTpMin = revalue/60;
							revalue = revalue%60;
							reTpSec = revalue;

							//\",\r\n\"cL\":\"%s\",\r\n\"cZ\":\"%s\",\r\n\"cD\":\"%d/%d/20%02d\",\r\n\"cT\":\"%d:%d:%d\",\r\n\"mC\":\"LD\"\r\n}"
						//	sprintf(getbuff,"{\r\n\"cC\":\"%s\",\r\n\"LD\":\"%d-%d-20%02d,%d:%d:%d,%03.0fV,%03.0fV,%03.0fV,%03.0fV,%03.0fV,%03.0f,%s %s\",\r\n\"cL\":\"%s\",\r\n\"cZ\":\"%s\",\r\n\"cD\":\"%d/%d/20%02d\",\r\n\"cT\":\"%d:%d:%d\",\r\n\"mC\":\"SMS\"\r\n}",IMEI,datetime.tm_mday,datetime.tm_mon,datetime.tm_year,datetime.tm_hour,datetime.tm_min,datetime.tm_sec,nCurretnCond.RVoltage,nCurretnCond.YVoltage,nCurretnCond.BVoltage,nCurretnCond.RYVoltage,nCurretnCond.YBVoltage,nCurretnCond.BRVoltage,SmsTCPStrNumber[NumberOfSmsAllreadySend].SmsTCPstr,SMS_BUF,DIS_BUF,VAL_BUF);
							sprintf(getbuff,"{\r\n\"cC\":\"%s\",\r\n\"cM\":\"%03.0f,%03.0f,%03.0f,%03.0f,%03.0f,%03.0f,",IMEI,nCurretnCond.RVoltage,nCurretnCond.YVoltage,nCurretnCond.BVoltage,nCurretnCond.RYVoltage,nCurretnCond.YBVoltage,nCurretnCond.BRVoltage);//,DIS_BUF,VAL_BUF,datetime.tm_mday,datetime.tm_mon,datetime.tm_year,datetime.tm_hour,datetime.tm_min,datetime.tm_sec);   //dg_bckup
						//	sprintf(getbuff,"\r\n\"cC\":\"%s\",\r\n\"cM\":\"%03.0f,%03.0f,%03.0f,%03.0f,%03.0f,%03.0f,",IMEI,nCurretnCond.RVoltage,nCurretnCond.YVoltage,nCurretnCond.BVoltage,nCurretnCond.RYVoltage,nCurretnCond.YBVoltage,nCurretnCond.BRVoltage);//,DIS_BUF,VAL_BUF,datetime.tm_mday,datetime.tm_mon,datetime.tm_year,datetime.tm_hour,datetime.tm_min,datetime.tm_sec);

							FloatroString1Dig(TpStr4,nCurretnCond.Rcurrent);
			                sprintf(getbuff,"%s%s,",getbuff,TpStr4);
							FloatroString1Dig(TpStr4,nCurretnCond.Ycurrent);
							sprintf(getbuff,"%s%s,",getbuff,TpStr4);
							FloatroString1Dig(TpStr4,nCurretnCond.Bcurrent);
							if(zonecom.standalonemodeonof==1)
							sprintf(getbuff,"%s%s,Standalone Mode,",getbuff,TpStr4);
							else if(zoneonof[nVaTr.programselection].flowmodeonof==1)
							sprintf(getbuff,"%s%s,Flow Mode,",getbuff,TpStr4);
							else if(zoneonof[nVaTr.programselection].moisturemodeonof==1)
							sprintf(getbuff,"%s%s,Moisture Mode,",getbuff,TpStr4);
                            else if(zoneonof[nVaTr.programselection].levelmodeonof==1)
                            sprintf(getbuff,"%s%s,Level Mode,",getbuff,TpStr4);
							else
							sprintf(getbuff,"%s%s,Timer Mode,",getbuff,TpStr4);
                            if((zoneonof[nVaTr.programselection].flowmodeonof == 0))
							{runvflow=0;value=0;}
							else
								value=(runvflow-zone[nDripSettings.stp].Actflowrate);
							if(value<0)
								value=0;
							if(zonecom.standalonemodeonof==1)
							sprintf(getbuff,"%s%d,%03d,",getbuff,nVaTr.programselection,nDripSettings.ststp);
						    else
							sprintf(getbuff,"%s%d,%03d,",getbuff,nVaTr.programselection,nDripSettings.stp);


						//	sprintf(getbuff,"%s%02ld:%02ld:%02ld,%02ld:%02ld:%02ld,%d,%d,100,100,%d,%d,%d,%d,%03.1f,%d,%03.1f,%d,%d,123456789,49.9,%03d,%03d,100,100,100,100,100,100,100,100,%d,%d,%d,%d,",getbuff,reTpHr,reTpMin,reTpSec,TpHr,TpMin,TpSec,runvflow,value,nVaTr.fertv1Smsonof,nVaTr.fertv2Smsonof,nVaTr.fertv3Smsonof,nVaTr.fertv4Smsonof,zone[nDripSettings.stp].runpressure,nVaTr.Currentflow,zone[nDripSettings.stp].runflowrate,LastDayRunflow,Runflow,zoneid[nDripSettings.decidelast+1].fbkval,zoneid[nDripSettings.decidelast+2].fbkval,CSQ,PrvLocanMotorStatus,PrvMSMotorStatus,nTimerSettings.ManualOnOff);
						//	sprintf(getbuff,"%s%02ld:%02ld:%02ld,%02ld:%02ld:%02ld,%d,%d,100,100,%d,%d,%d,%d,%03.1f,%d,%03.1f,%d,%d,123456789,49.9,%03d,%03d,100,100,100,100,100,100,%.02fF-%02d,100,%d,%d,%d,%d,",getbuff,reTpHr,reTpMin,reTpSec,TpHr,TpMin,TpSec,runvflow,value,nVaTr.fertv1Smsonof,nVaTr.fertv2Smsonof,nVaTr.fertv3Smsonof,nVaTr.fertv4Smsonof,zone[nDripSettings.stp].runpressure,nVaTr.Currentflow,zone[nDripSettings.stp].runflowrate,LastDayRunflow,Runflow,zoneid[nDripSettings.decidelast+1].fbkval,zoneid[nDripSettings.decidelast+2].fbkval,leveltank,levelpercent,CSQ,PrvLocanMotorStatus,PrvMSMotorStatus,nTimerSettings.ManualOnOff);
							//sprintf(getbuff,"%s%02ld:%02ld:%02ld,%02ld:%02ld:%02ld,%d,%d,100,100,%d,%d,%d,%d,%03.1f,%d,%03.1f,%d,%d,123456789,49.9,%03d,%03d,100,100,100,100,100,100,%.02fF-%02d,%03.1f,%d,%d,%d,%d,,",getbuff,reTpHr,reTpMin,reTpSec,TpHr,TpMin,TpSec,runvflow,value,nVaTr.fertv1Smsonof,nVaTr.fertv2Smsonof,nVaTr.fertv3Smsonof,nVaTr.fertv4Smsonof,zone[nDripSettings.stp].runpressure,nVaTr.Currentflow,zone[nDripSettings.stp].runflowrate,LastDayRunflow,Runflow,zoneid[nDripSettings.decidelast+1].fbkval,zoneid[nDripSettings.decidelast+2].fbkval,leveltank,levelpercent,zone[nDripSettings.stp].outpressure,CSQ,PrvLocanMotorStatus,PrvMSMotorStatus,nTimerSettings.ManualOnOff);

						   value = RunTimer;
							TpHr = value/3600;
							value = value%3600;
							TpMin = value/60;
							value = value%60;
							TpSec = value;

							value = LastDayRunTimer;
							reTpHr = value/3600;
							value = value%3600;
							reTpMin = value/60;
							value = value%60;
							reTpSec = value;
							sprintf(getbuff,"%s%02ld:%02ld,%02ld:%02ld,",getbuff,TpHr,TpMin,reTpHr,reTpMin);

						   if(nMSettings.ndebugonof==1)
							sprintf(buf,"DIS_BUF1 copy =%s",DIS_BUF);
                             sAPI_UartPrintf(buf);
							if(DIS_BUF[0] == '$')
							{
							k = DIS_BUF;
							len=strlen(DIS_BUF);
							for (i = 0; i <len; i++)
							{
								if(*k=='\n')
								*k = ' ';
							    else if(*k=='\r')
								*k = ' ';
									k++;
							}
							}
							else
							{strcpy(DIS_BUF,"$L,     NO DATA,    ");}
						if(nMSettings.ndebugonof==1)
							sprintf(buf,"DIS_BUF copy =%s",DIS_BUF);
                             sAPI_UartPrintf(buf);
							sprintf(getbuff,"%s\",\r\n\"cL\":\"%s\",\r\n\"cZ\":\"%s\",\r\n\"cD\":\"%d/%d/20%02d\",\r\n\"cT\":\"%d:%d:%d\",\r\n\"mC\":\"LD\"\r\n}",getbuff,DIS_BUF,VAL_BUF,datetime.tm_mday,datetime.tm_mon,datetime.tm_year,datetime.tm_hour,datetime.tm_min,datetime.tm_sec);

						


						//	sprintf(getbuff,"%s%02ld:%02ld:%02ld,%d,%d,%d,%d,%d,%d,%s\"\r\n}",getbuff,TpHr,TpMin,TpSec,zone[nDripSettings.stp].actflowrate,value,nVaTr.fertv1Smsonof,nVaTr.fertv2Smsonof,nVaTr.fertv3Smsonof,nVaTr.fertv4Smsonof,DIS_BUF);
							contentlen=strlen(getbuff);
						    sprintf(BigSMS,"POST /api/v1/controller/messages/ HTTP/1.1\r\nHost: %s:8080\r\nUser-Agent: curl/7.52.1\r\nAccept: */*\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n\%s\r\n",DeviceConfig.TcpServerIP,contentlen,getbuff);
						//	sprintf(BigSMS,"POST /api/v1/getlivestatus/ HTTP/1.1\r\nHost: %s:8091\r\nUser-Agent: curl/7.52.1\r\nAccept: */*\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n\%s\r\n",DeviceConfig.TcpServerIP,contentlen,getbuff);
							if(livedataflag1==0 && livedataflag==0)
							{
							sAPI_UartWrite(eat_uart_wifi, TCPwifiStrNumber[Nooftcpprocessed].TCPWifistr,strlen(TCPwifiStrNumber[Nooftcpprocessed].TCPWifistr));
							Http_send_flag=0;
							}
							else
							{
							Http_send_flag=0;
						    sAPI_UartWrite(eat_uart_wifi, BigSMS,strlen(BigSMS));
							}
			


			}
							//Nooftcpprocessed++;
							tcpdcounter=0;
							tcpdcounter1=0;
							getdataflag1=0;
							}
							//sprintf(buf,"\n\rtest1=%d,%d,%
							sAPI_UartPrintf(buf);
					//		#endif
							}
							else if(tcpopenflag==1 && wifiopenflag==1 && clostcpcounter>=50)
							{
							//clostcpcounter=0;
							if(tcpdcounter++>8)
							{
								tcpdcounter=0;
								clostcpcounter=0;
								wifiopenflag=0;
								//eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 5, "CLOSE", EAT_NULL);
							}
							}
							else
							{
								//tcpdcounter++;
								gettcpcounter=0;
								if(tcpopenflag==1&&wifiresetflag1==0&&wifiresetflag==0 && wifiopenflag==0  )
								{
								//tcpdcounter++;
								tcpdcounter1++;
								if(tcpdcounter1>30)
								{
                                //eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 6, "CONFIG", EAT_NULL);
								if(nMSettings.ndebugonof==1)
								{
								sprintf(buf,"tcp copy done222222222222222222222222222222222222222222222");
								sAPI_UartPrintf(buf);
								//wifiopenflag=1;
								tcpdcounter1=0;
								getdataflag1=1;
								}
								}
								}

								/*else if(tcpdcounter==0&&tcpopenflag==1 && wifiopenflag==0 )
								{
								tcpdcounter++;
                                //eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 7, "CONFIG1", EAT_NULL);

								}*/

								//if(tcpdcounter>10)
								//tcpdcounter=10;
							}
#endif
				}
				else if (DeviceConfig.interface == GPRS && fotaflsg == 0 && CallConnected == 0 && ringflag == 0)

				{

					wifiopenflag = 0;
					// if(tcpdcounter++>60)
					if (tcpdcounter++ > 5)
					{
						gettcpcounter = 0;
						sprintf(buf, "\n\rlivedataflag:%d,livedataflag1:%d", livedataflag, livedataflag1);
						sAPI_UartPrintf(buf);

						#if 1
						if(livedataflag ==1 || livedataflag1==1)       //LIVE
							{							 
							INT32 ret;
	                        UINT8  *k = EAT_NULL;
	                        char TpStr4[50] = "nimish";
							char getbuff_temp[20];
	                        long TpHr,TpMin,TpSec,value;
							long TpHr2,TpMin2,TpSec2,value2;
							long TpHr3,TpMin3,TpSec3,value3;
	                        long runvtim,runvflow;
							float pervtim;  
							int pervflow,size;
							char flowbuf[50]={0};char presbuf[50]={0};char levelbuf[50]={0};char flotbuf[50]={0};char tankbuf[50]={0};char sumpbuf[50]={0};
						//	char Motor1onflag=0,Motor2onflag=0,Motor3onflag=0;
							    
                        //    value=xxx 
							memset(getbuff,NULL,sizeof(getbuff)+1);
							memset(getbuff1,NULL,sizeof(getbuff1)+1);
							memset(getbuff2,NULL,sizeof(getbuff2)+1);
							memset(getbuff3,NULL,sizeof(getbuff3)+1);
							memset(getbuff4,NULL,sizeof(getbuff4)+1);
							livedataflagcount1++;
							if(livedataflagcount1>2 && livedataflag1>0)
							{
							livedataflagcount1=4;
							livedataflag1=0;
							if(nMSettings.dataSMSOnOff==1)
							{
							getdataflag =1;
							enter2=0;
							}

							}
							
							sprintf(getbuff1,"{\r\n\"cC\":\"%s\",\r\n\"cM\":{",IMEI);

							/***** Device details *****/
							sprintf(getbuff1,"%s\r\n\"2401\": \"%0.2f,%d\",",getbuff1,RecValveOnOff.BatVolt,RecValveOnOff.SolarVolt);
							sprintf(getbuff1,"%s\r\n\"2402\": \"",getbuff1);//,s_nOMSfeedback[0].MainValvenofbk);
							for(i=0;i<=8;i++)
							{
								if(nConfig[i].Object==13)
								{
									sprintf(getbuff3,"%s;%d.%03d,%d",getbuff3,nConfig[i].Object,nConfig[i].Sno,nConfig[i].Status);
								}
								else if(nConfig[i].Object==45)
								{
									sprintf(getbuff3,"%s;%d.%03d,%d",getbuff3,nConfig[i].Object,nConfig[i].Sno,RecValveOnOff.ControlFLag,nConfig[i].Status);
								}
							}
							sprintf(getbuff2, "\",\r\n\"2403\":\"24.001,%3.1f;24.002,%3.1f;22.001,%3.1f\",\r\n\"2404\":\"\",",s_nOMSfeedback[0].Pressure, s_nOMSfeedback[1].Pressure,s_nOMSfeedback[0].LPS); // ,%3.1f,%3.1f
							sprintf(getbuff,"%s%s%s\r\n\"2405\":\"1.001,%d;\",\r\n\"2406\":\"\",",getbuff1,getbuff3,getbuff2,nProgram.Status);
							sprintf(getbuff4,"\r\n\"2407\":\"\",");
							sprintf(getbuff4,"%s\r\n\"2408\":\"\",",getbuff4);
							sprintf(getbuff4,"%s\r\n\"2409\":\"\",",getbuff4);
							sprintf(getbuff4,"%s\r\n\"2410\":\"\",",getbuff4);
							sprintf(getbuff4,"%s\r\n\"2411\":\"\",",getbuff4);
							sprintf(getbuff,"%s%s\r\n\"2412\":\"\",",getbuff,getbuff4);
							sprintf(getbuff,"%s\r\n\"WifiStrength\":%d,\r\n\"Version\":\"OSM_TEST\",\r\n\"PowerSupply\":%01d},\r\n\"cD\":\"%02d-%02d-%02d\",\r\n\"cT\":\"%02d:%02d:%02d\",\r\n\"mC\":\"2400\"\r\n}",getbuff,sstrength,!PowerCurrentCondition,datetime.tm_year,datetime.tm_mon,datetime.tm_mday,datetime.tm_hour,datetime.tm_min,datetime.tm_sec);
									
							
							
							sprintf(TCPWifigprsstrBUFF,"%s",getbuff);
                            sprintf(buf,"\n\rTCPWifigprsstrBUFF>>:[%s]",TCPWifigprsstrBUFF);
							sAPI_UartPrintf(buf);
							
						    }
							#endif

						#if 0
						if(livedataflag ==1 || livedataflag1==1)       //LIVE
							{							 
							INT32 ret;
	                        UINT8  *k = EAT_NULL;
	                        char TpStr4[50] = "nimish";
							char getbuff_temp[20];
	                        long TpHr,TpMin,TpSec,value;
							long TpHr2,TpMin2,TpSec2,value2;
							long TpHr3,TpMin3,TpSec3,value3;
	                        long runvtim,runvflow;
							float pervtim;  
							int pervflow,size;
							char flowbuf[50]={0};char presbuf[50]={0};char levelbuf[50]={0};char flotbuf[50]={0};char tankbuf[50]={0};char sumpbuf[50]={0};
						//	char Motor1onflag=0,Motor2onflag=0,Motor3onflag=0;
							    
                        //    value=xxx 
							memset(getbuff,NULL,sizeof(getbuff)+1);
							memset(getbuff1,NULL,sizeof(getbuff1)+1);
							memset(getbuff2,NULL,sizeof(getbuff2)+1);
							memset(getbuff3,NULL,sizeof(getbuff3)+1);
							memset(getbuff4,NULL,sizeof(getbuff4)+1);
							livedataflagcount1++;
							if(livedataflagcount1>2 && livedataflag1>0)
							{
							livedataflagcount1=4;
							livedataflag1=0;
							if(nMSettings.dataSMSOnOff==1)
							{
							getdataflag =1;
							enter2=0;
							}

							}


						
						act_del_completed[0]=(act_POnMin[0]*60+act_POnSec[0])-(act_rem_delmin[0]*60+act_rem_delsec[0]);
								
						act_del_comp_min[0]=act_del_completed[0]/60;
						act_del_comp_sec[0]=act_del_completed[0]%60;
						
						act_del_completed[1]=(act_POnMin[1]*60+act_POnSec[1])-(act_rem_delmin[1]*60+act_rem_delsec[1]);
													
						act_del_comp_min[1]=act_del_completed[1]/60;
						act_del_comp_sec[1]=act_del_completed[1]%60;
						
						act_del_completed[2]=(act_POnMin[2]*60+act_POnSec[2])-(act_rem_delmin[2]*60+act_rem_delsec[2]);
													
						act_del_comp_min[2]=act_del_completed[2]/60;
						act_del_comp_sec[2]=act_del_completed[2]%60;
						
						sprintf(buf,"\n\r act_rem_delmin[0] %d  act_rem_delsec[0] %d act_rem_delmin[1] %d  act_rem_delsec[1] %d  act_rem_delmin[2] %d  act_rem_delsec[2] %d",act_rem_delmin[0],act_rem_delsec[0],act_rem_delmin[1],act_rem_delsec[1],act_rem_delmin[2],act_rem_delsec[2]);
							sAPI_UartPrintf(buf);
						
						
			
							
									
									
						
							if(s_npump[0].m_flowonof == 0)
							sprintf(flowbuf, "\r\n\"WM\":\"-\",\r\n\"CF\":\"-\"");
							else 
							sprintf(flowbuf, "\r\n\"WM\":\"%03.01f\",\r\n\"CF\":\"%07d\"", Act_lps1[0], cumulative_flow1[0]);
						
							if (s_npump[0].m_pressureonof == 0)
							sprintf(presbuf, "\r\n\"PR\":\"-\"");
							else
							sprintf(presbuf, "\r\n\"PR\":\"%03.01f\"", Act_pressure[0]);
							
							if(s_npump[0].m_Level_on_off==0)
							sprintf(levelbuf,"\r\n\"LV\":\"-,-\"");
							 else
							//sprintf(levelbuf,"\r\n\"LV\":\"%03d\"",Act_level[0]);// subash doubte 
						sprintf(levelbuf,"\r\n\"LV\":\"%03.01f,%02d\"",Act_level[0],level_percent);
						
						
						if (s_npump[0].m_Sump_on_off == 0)
						sprintf(sumpbuf, "\r\n\"FT\":\"-:-:");
					else if (s_npump[0].m_no_of_sump_pins == 1)
						
						sprintf(sumpbuf, "\r\n\"FT\":\"%01d:-:", actual_float_stat[0][0]);
					//	sprintf(sumpbuf, "\r\n\"FT\":\"%01d:-:", s_npump[0].m_sump_pin_no[0]);
					else
						
						sprintf(sumpbuf, "\r\n\"FT\":\"%01d:%01d:", actual_float_stat[0][0], actual_float_stat[0][1]);
					//	sprintf(sumpbuf, "\r\n\"FT\":\"%01d:%01d:", s_npump[0].m_sump_pin_no[0], s_npump[0].m_sump_pin_no[1]);

					if (s_npump[0].m_Tank_on_off == 0)
						sprintf(tankbuf, "-:-\"");
					else if (s_npump[0].m_no_of_tank_pins == 1)
						sprintf(tankbuf, "-:%01d\"", actual_float_stat[0][2]);
					//	sprintf(tankbuf, "%01d:-\"", s_npump[0].m_tank_pin_no[0]);
					else
						sprintf(tankbuf, "%01d:%01d\"", actual_float_stat[0][2], actual_float_stat[0][3]);
					//	sprintf(tankbuf, "%01d:%01d\"", s_npump[0].m_tank_pin_no[0], s_npump[0].m_tank_pin_no[1]);
					sprintf(buf,"\n\r sumpbuf [%s]",sumpbuf);
					sAPI_UartPrintf(buf);
					sprintf(flotbuf,"%s",sumpbuf);
					sprintf(buf,"\n\r tankbuf [%s]",tankbuf);
					sAPI_UartPrintf(buf);
					sprintf(flotbuf,"%s%s",flotbuf,tankbuf);
					sprintf(buf,"\n\r flotbuf [%s]",flotbuf);
					sAPI_UartPrintf(buf);
					/* if(Motorreasonflag[0] ==1 || Motorreasonflag[0] ==0)
					{								
						set_value=0,actual_value=0;
					} */	
					
								sprintf(getbuff1,"{\r\n\"cC\":\"%s\",\r\n\"cM\": [ \r\n{\r\n\"ST\":%01d,\r\n\"RN\":%01d,\r\n\"AT\":%03.01f,\r\n\"SE\":%03.01f,\r\n\"PH\":%01d,%s,%s,%s,%s,\r\n\"OD\":\"00:%02d:%02d\",\r\n\"ODC\":\"00:%02d:%02d\",\r\n\"ODL\":\"00:%02d:%02d\",\r\n\"CFDL\":\"%02d:%02d:%02d\",\r\n\"CNDL\":\"%02d:%02d:%02d\",\r\n\"MR\":\"%02d:%02d:%02d\",\r\n\"DRST\":\"%02d:%02d:%02d\"\r\n },",IMEI,Motoronflag[0],Motorreasonflag[0],actual_value,set_value,phase_number,flowbuf,presbuf,levelbuf,flotbuf,act_POnMin[0],act_POnSec[0],act_del_comp_min[0],act_del_comp_sec[0],act_rem_delmin[0],act_rem_delsec[0],act_rem_cyc_ofHr[0],act_rem_cyc_ofMin[0],act_rem_cyc_ofSec[0],act_rem_cyc_onHr[0],act_rem_cyc_onMin[0],act_rem_cyc_onSec[0],act_rem_maxHr[0],act_rem_maxMin[0],act_rem_maxSec[0],nMSettings.act_rem_DrHr[0],nMSettings.act_rem_DrMin[0],nMSettings.act_rem_DrSec[0]);
									
									sprintf(buf,"\n\r getbuff1 [%s]",getbuff1);
									sAPI_UartPrintf(buf);
									size=strlen(getbuff1);
									sprintf(buf,"\n\r size getbuff1 [%d]",size);
									sAPI_UartPrintf(buf);
													
							if(s_npump[1].m_flowonof == 0)
							sprintf(flowbuf, "\r\n\"WM\":\"-\",\r\n\"CF\":\"-\"");
							else 
							sprintf(flowbuf, "\r\n\"WM\":\"%03.01f\",\r\n\"CF\":\"%07d\"", Act_lps1[1], cumulative_flow1[1]);
						
							if (s_npump[1].m_pressureonof == 0)
							sprintf(presbuf, "\r\n\"PR\":\"-\"");
							else
							sprintf(presbuf, "\r\n\"PR\":\"%03.01f\"", Act_pressure[1]);
							
							if(s_npump[1].m_Level_on_off==0)
							sprintf(levelbuf,"\r\n\"LV\":\"-,-\"");
							 else
							sprintf(levelbuf,"\r\n\"LV\":\"%03.01f,%02d\"",Act_level[1],level_percent);
						
							/*		if (s_npump[1].m_Sump_on_off == 0)
						sprintf(sumpbuf, "\r\n\"FT\":\"-:-:");
					else if (s_npump[1].m_no_of_tank_pins == 1)
						sprintf(sumpbuf, "\r\n\"FT\":\"%01d:-:", s_npump[1].m_sump_pin_no[0]);
					else
						sprintf(sumpbuf, "\r\n\"FT\":\"%01d:%01d:", s_npump[1].m_sump_pin_no[0], s_npump[1].m_sump_pin_no[1]);

					if (s_npump[1].m_Tank_on_off == 0)
						sprintf(tankbuf, "-:-\"");
					else if (s_npump[1].m_no_of_tank_pins == 1)
						sprintf(tankbuf, "%01d:-\"", s_npump[1].m_tank_pin_no[0]);
					else
						sprintf(tankbuf, "%01d:%01d\"", s_npump[1].m_tank_pin_no[0], s_npump[1].m_tank_pin_no[1]); */
					if (s_npump[1].m_Sump_on_off == 0)
						sprintf(sumpbuf, "\r\n\"FT\":\"-:-:");
					else if (s_npump[1].m_no_of_sump_pins == 1)
						
						sprintf(sumpbuf, "\r\n\"FT\":\"%01d:-:", actual_float_stat[1][0]);
					//	sprintf(sumpbuf, "\r\n\"FT\":\"%01d:-:", s_npump[0].m_sump_pin_no[0]);
					else
						
						sprintf(sumpbuf, "\r\n\"FT\":\"%01d:%01d:", actual_float_stat[1][0], actual_float_stat[1][1]);
					//	sprintf(sumpbuf, "\r\n\"FT\":\"%01d:%01d:", s_npump[0].m_sump_pin_no[0], s_npump[0].m_sump_pin_no[1]);

					if (s_npump[1].m_Tank_on_off == 0)
						sprintf(tankbuf, "-:-\"");
					else if (s_npump[1].m_no_of_tank_pins == 1)
						sprintf(tankbuf, "-:%01d\"", actual_float_stat[1][2]);
					//	sprintf(tankbuf, "%01d:-\"", s_npump[0].m_tank_pin_no[0]);
					else
						sprintf(tankbuf, "%01d:%01d\"", actual_float_stat[1][2], actual_float_stat[1][3]);
					//	sprintf(tankbuf, "%01d:%01d\"", s_npump[0].m_tank_pin_no[0], s_npump[0].m_tank_pin_no[1]);
					
					sprintf(buf,"\n\r sumpbuf [%s]",sumpbuf);
					sAPI_UartPrintf(buf);
					sprintf(flotbuf,"%s",sumpbuf);
					sprintf(buf,"\n\r tankbuf [%s]",tankbuf);
					sAPI_UartPrintf(buf);
					sprintf(flotbuf,"%s%s",flotbuf,tankbuf);
					sprintf(buf,"\n\r flotbuf [%s]",flotbuf);
					sAPI_UartPrintf(buf);
					/* if(Motorreasonflag[1] ==1 || Motorreasonflag[1] ==0)
					{								
						set_value2=0,actual_value2=0;
					} */	
							//		sprintf(getbuff2,"\r\n{\r\n\"ST\":%01d,\r\n\"RN\":%01d,\r\n\"AT\":%03.01f,\r\n\"SE\":%03.01f,\r\n\"PH\":%03.01f, \r\n\"WM\":%01d,\r\n\"CF\":%03.01f,\r\n\"PR\":%03.01f,\r\n\"LV\":%d,\r\n\"FT\":\"%01d:%01d:%01d:%01d\",\r\n\"OD\":\"00:%02d:%02d\",\r\n\"ODC\":\"00:%02d:%02d\",\r\n\"ODL\":\"00:%02d:%02d\"\r\n},",Motoronflag[1],Motorreasonflag[1],actual_value2,set_value2,phase_number2,flowisthere[1],cumulative_flow2,Act_pressure[1],Act_level[1],actual_float_stat[1][0],actual_float_stat[1][1],actual_float_stat[1][2],actual_float_stat[1][3],act_POnMin[1],act_POnSec[1],act_del_comp_min[1],act_del_comp_sec[1],act_rem_delmin[1],act_rem_delsec[1]);
							//		sprintf(getbuff2,"\r\n{\r\n\"ST\":%01d,\r\n\"RN\":%01d,\r\n\"AT\":%03.01f,\r\n\"SE\":%03.01f,\r\n\"PH\":%03d,%s,%s,%s,\r\n\"FT\":\"%01d:%01d:%01d:%01d\",\r\n\"OD\":\"00:%02d:%02d\",\r\n\"ODC\":\"00:%02d:%02d\",\r\n\"ODL\":\"00:%02d:%02d\"\r\n},",Motoronflag[1],Motorreasonflag[1],actual_value2,set_value2,phase_number2,flowbuf,presbuf,levelbuf,actual_float_stat[1][0],actual_float_stat[1][1],actual_float_stat[1][2],actual_float_stat[1][3],act_POnMin[1],act_POnSec[1],act_del_comp_min[1],act_del_comp_sec[1],act_rem_delmin[1],act_rem_delsec[1]);
									//sprintf(getbuff2,"\r\n{\r\n\"ST\":%01d,\r\n\"RN\":%01d,\r\n\"AT\":%03.01f,\r\n\"SE\":%03.01f,\r\n\"PH\":%01d,%s,%s,%s,%s,\r\n\"OD\":\"00:%02d:%02d\",\r\n\"ODC\":\"00:%02d:%02d\",\r\n\"ODL\":\"00:%02d:%02d\",\r\n\"CFDL\":\"%02d:%02d:%02d\",\r\n\"CNDL\":\"%02d:%02d:%02d\"\r\n},",Motoronflag[1],Motorreasonflag[1],actual_value2,set_value2,phase_number2,flowbuf,presbuf,levelbuf,flotbuf,act_POnMin[1],act_POnSec[1],act_del_comp_min[1],act_del_comp_sec[1],act_rem_delmin[1],act_rem_delsec[1],act_rem_cyc_ofHr[1],act_rem_cyc_ofMin[1],act_rem_cyc_ofSec[1],act_rem_cyc_onHr[1],act_rem_cyc_onMin[1],act_rem_cyc_onSec[1]);	// \"MR\":\"%02d:%02d:%02d\"\r\n	//,act_rem_maxHr[1],act_rem_maxMin[1],act_rem_maxSec[1]							
									//sprintf(getbuff2,"\r\n{\r\n\"ST\":%01d,\r\n\"RN\":%01d,\r\n\"AT\":%03.01f,\r\n\"SE\":%03.01f,\r\n\"PH\":%01d,%s,%s,%s,%s,\r\n\"OD\":\"00:%02d:%02d\",\r\n\"ODC\":\"00:%02d:%02d\",\r\n\"ODL\":\"00:%02d:%02d\",\r\n\"CFDL\":\"%02d:%02d:%02d\",\r\n\"CNDL\":\"%02d:%02d:%02d\",\r\n\"MR\":\"%02d:%02d:%02d\",\r\n\"DRST\":\"%02d:%02d:%02d\"\r\n},",Motoronflag[1],Motorreasonflag[1],actual_value2,set_value2,phase_number2,flowbuf,presbuf,levelbuf,flotbuf,act_POnMin[1],act_POnSec[1],act_del_comp_min[1],act_del_comp_sec[1],act_rem_delmin[1],act_rem_delsec[1],act_rem_cyc_ofHr[1],act_rem_cyc_ofMin[1],act_rem_cyc_ofSec[1],act_rem_cyc_onHr[1],act_rem_cyc_onMin[1],act_rem_cyc_onSec[1],act_rem_maxHr[1],act_rem_maxMin[1],act_rem_maxSec[1],nMSettings.act_rem_DrHr[1],nMSettings.act_rem_DrMin[1],nMSettings.act_rem_DrSec[1]);								

									//sprintf(buf,"\n\r getbuff2 %s",getbuff2);
									//sAPI_UartPrintf(buf);
									/* size=strlen(getbuff2);
									sprintf(buf,"\n\r size getbuff2 %d",size);
									sAPI_UartPrintf(buf); */
									
								if(s_npump[2].m_flowonof == 0)
							sprintf(flowbuf, "\r\n\"WM\":\"-\",\r\n\"CF\":\"-\"");
							else 
							sprintf(flowbuf, "\r\n\"WM\":\"%03.01f\",\r\n\"CF\":\"%07d\"", Act_lps1[2], cumulative_flow1[2]);
						
							if (s_npump[2].m_pressureonof == 0)
							sprintf(presbuf, "\r\n\"PR\":\"-\"");
							else
							sprintf(presbuf, "\r\n\"PR\":\"%03.01f\"", Act_pressure[2]);
							
							if(s_npump[2].m_Level_on_off==0)
							sprintf(levelbuf,"\r\n\"LV\":\"-,-\"");
							 else
							sprintf(levelbuf,"\r\n\"LV\":\"%03.01f,%02d\"",Act_level[2],level_percent);
															
															
															
					/*	if (s_npump[2].m_Sump_on_off == 0)
						sprintf(sumpbuf, "\r\n\"FT\":\"-:-:");
					else if (s_npump[2].m_no_of_tank_pins == 1)
						sprintf(sumpbuf, "\r\n\"FT\":\"%01d:-:", s_npump[2].m_sump_pin_no[0]);
					else
						sprintf(sumpbuf, "\r\n\"FT\":\"%01d:%01d:", s_npump[2].m_sump_pin_no[0], s_npump[2].m_sump_pin_no[1]);

					if (s_npump[2].m_Tank_on_off == 0)
						sprintf(tankbuf, "-:-\"");
					else if (s_npump[2].m_no_of_tank_pins == 1)
						sprintf(tankbuf, "%01d:-\"", s_npump[2].m_tank_pin_no[0]);
					else
						sprintf(tankbuf, "%01d:%01d\"", s_npump[2].m_tank_pin_no[0], s_npump[2].m_tank_pin_no[1]);*/
					
						if (s_npump[2].m_Sump_on_off == 0)
						sprintf(sumpbuf, "\r\n\"FT\":\"-:-:");
					else if (s_npump[2].m_no_of_sump_pins == 1)
						
						sprintf(sumpbuf, "\r\n\"FT\":\"%01d:-:", actual_float_stat[2][0]);
					//	sprintf(sumpbuf, "\r\n\"FT\":\"%01d:-:", s_npump[0].m_sump_pin_no[0]);
					else
						
						sprintf(sumpbuf, "\r\n\"FT\":\"%01d:%01d:", actual_float_stat[2][0], actual_float_stat[2][1]);
					//	sprintf(sumpbuf, "\r\n\"FT\":\"%01d:%01d:", s_npump[0].m_sump_pin_no[0], s_npump[0].m_sump_pin_no[1]);

					if (s_npump[2].m_Tank_on_off == 0)
						sprintf(tankbuf, "-:-\"");
					else if (s_npump[2].m_no_of_tank_pins == 1)
						sprintf(tankbuf, "-:%01d\"", actual_float_stat[2][2]);
					//	sprintf(tankbuf, "%01d:-\"", s_npump[0].m_tank_pin_no[0]);
					else
						sprintf(tankbuf, "%01d:%01d\"", actual_float_stat[2][2], actual_float_stat[2][3]);
					//	sprintf(tankbuf, "%01d:%01d\"", s_npump[0].m_tank_pin_no[0], s_npump[0].m_tank_pin_no[1]);
					
					sprintf(buf,"\n\r sumpbuf [%s]",sumpbuf);
					sAPI_UartPrintf(buf);
					sprintf(flotbuf,"%s",sumpbuf);
					sprintf(buf,"\n\r tankbuf [%s]",tankbuf);
					sAPI_UartPrintf(buf);
					sprintf(flotbuf,"%s%s",flotbuf,tankbuf);
					sprintf(buf,"\n\r flotbuf [%s]",flotbuf);
					sAPI_UartPrintf(buf);
					if(Motorreasonflag[2] ==1 || Motorreasonflag[2] ==0)
					{								
						set_value3=0,actual_value3=0;
					}			
							//		sprintf(getbuff3,"\n\r{\r\n\"ST\":%01d,\r\n\"RN\":%01d,\r\n\"AT\":%03.01f,\r\n\"SE\":%03.01f,\r\n\"PH\":%03d,%s,%s,%s,\r\n\"FT\":\"%01d:%01d:%01d:%01d\",\r\n\"OD\":\"00:%02d:%02d\",\r\n\"ODC\":\"00:%02d:%02d\",\r\n\"ODL\":\"00:%02d:%02d\"\r\n},",Motoronflag[2],Motorreasonflag[2],actual_value3,set_value3,phase_number3,flowbuf,presbuf,levelbuf,actual_float_stat[2][0],actual_float_stat[2][1],actual_float_stat[2][2],actual_float_stat[2][3],act_POnMin[2],act_POnSec[2],act_del_comp_min[2],act_del_comp_sec[2],act_rem_delmin[2],act_rem_delsec[2]);
									//sprintf(getbuff3,"\n\r{\r\n\"ST\":%01d,\r\n\"RN\":%01d,\r\n\"AT\":%03.01f,\r\n\"SE\":%03.01f,\r\n\"PH\":%01d,%s,%s,%s,%s,\r\n\"OD\":\"00:%02d:%02d\",\r\n\"ODC\":\"00:%02d:%02d\",\r\n\"ODL\":\"00:%02d:%02d\",\r\n\"CFDL\":\"%02d:%02d:%02d\",\r\n\"CNDL\":\"%02d:%02d:%02d\"\r\n},",Motoronflag[2],Motorreasonflag[2],actual_value3,set_value3,phase_number3,flowbuf,presbuf,levelbuf,flotbuf,act_POnMin[2],act_POnSec[2],act_del_comp_min[2],act_del_comp_sec[2],act_rem_delmin[2],act_rem_delsec[2],act_rem_cyc_ofHr[2],act_rem_cyc_ofMin[2],act_rem_cyc_ofSec[2],act_rem_cyc_onHr[2],act_rem_cyc_onMin[2],act_rem_cyc_onSec[2]);// \"MR\":\"%02d:%02d:%02d\"\r\n  //,act_rem_maxHr[2],act_rem_maxMin[2],act_rem_maxSec[2]
									//sprintf(getbuff3,"\n\r{\r\n\"ST\":%01d,\r\n\"RN\":%01d,\r\n\"AT\":%03.01f,\r\n\"SE\":%03.01f,\r\n\"PH\":%01d,%s,%s,%s,%s,\r\n\"OD\":\"00:%02d:%02d\",\r\n\"ODC\":\"00:%02d:%02d\",\r\n\"ODL\":\"00:%02d:%02d\",\r\n\"CFDL\":\"%02d:%02d:%02d\",\r\n\"CNDL\":\"%02d:%02d:%02d\",\r\n\"MR\":\"%02d:%02d:%02d\",\r\n\"DRST\":\"%02d:%02d:%02d\"\r\n},",Motoronflag[2],Motorreasonflag[2],actual_value3,set_value3,phase_number3,flowbuf,presbuf,levelbuf,flotbuf,act_POnMin[2],act_POnSec[2],act_del_comp_min[2],act_del_comp_sec[2],act_rem_delmin[2],act_rem_delsec[2],act_rem_cyc_ofHr[2],act_rem_cyc_ofMin[2],act_rem_cyc_ofSec[2],act_rem_cyc_onHr[2],act_rem_cyc_onMin[2],act_rem_cyc_onSec[2],act_rem_maxHr[2],act_rem_maxMin[2],act_rem_maxSec[2],nMSettings.act_rem_DrHr[2],nMSettings.act_rem_DrMin[2],nMSettings.act_rem_DrSec[2]);  

							//sprintf(buf,"\n\r getbuff3 %s",getbuff3);
									/* sAPI_UartPrintf(buf);
									size=strlen(getbuff3);
									sprintf(buf,"\n\r size getbuff3 %d",size);
									sAPI_UartPrintf(buf); */
									
									SignalStrength=(float)(CSQ*3.2258);
							sstrength=SignalStrength;
							if(sstrength>=99)
								sstrength=99;
							sprintf(buf,"vbat = %d\r\n",sAPI_ReadVbat());
							sAPI_UartPrintf(buf);
							if(PowerCurrentCondition == 1)
							{
								RVoltage1=0;
								YVoltage1=0;
								BVoltage1=0;
								nCurretnCond.Rcurrent=0;
								nCurretnCond.Ycurrent=0;
								nCurretnCond.Bcurrent=0;
								RemainingOnHr=0;RemainingOnMin=0;RemainingOnSec=0;
								RecCyclicIntevelHr=0;RecCyclicIntevelMin=0;RecCyclicIntevelsec=0;
								LightFlag=0;
								for(int i=0;i<10;i++)
								{
									if(ValveStatus[i]==1)
										ValveStatus[i]=0;
								}
							}
							
							for(int i=0;i<=50;i++)
							 {
								adc[i]=sAPI_ReadVbat();
								avg_adc += adc[i];
							 }
							avg_adc=avg_adc/50;
							BATPER=avg_adc*3.2;
							avg_adc=0;
							if(g_no_of_pumps>1)
								g_no_of_pumps=1;
						//	if(BATPER>=100) 
							BATPER=100;
							sAPI_UartPrintf("Battery Percentage:%d,%.0f cont_ver:G%d.%d\n\r",avg_adc,BATPER,nMSettings.CONT_VER[0],nMSettings.CONT_VER[1]);
									memset(getbuff4,NULL,sizeof(getbuff4));
									sprintf(getbuff4,"\r\n{\r\n\"V\":\"%03d,%03d,%03d\",\r\n\"C\":\"1:%03.1f,2:%03.01f,3:%03.01f\",\r\n\"SS\":%02d,\r\n\"B\":%02d,\r\n\"VS\":\"%d.%d.%d,%s\",\r\n\"NP\":\"%d\"\r\n}\r\n],\r\n\"cD\":\"%04d-%02d-%02d\",\r\n\"cT\":\"%02d:%02d:%02d\",\r\n\"mC\":\"LD01\"\r\n}",RVoltage1,YVoltage1,BVoltage1,nCurretnCond.Rcurrent,nCurretnCond.Ycurrent,nCurretnCond.Bcurrent,sstrength,(int)BATPER,nMSettings.CONT_VER[0],nMSettings.CONT_VER[1],nMSettings.CONT_VER[2],Version,g_no_of_pumps,datetime.tm_year,datetime.tm_mon,datetime.tm_mday,datetime.tm_hour,datetime.tm_min,datetime.tm_sec);
									
									sprintf(buf,"\n\r getbuff4 [%s]",getbuff4);
									sAPI_UartPrintf(buf);
									size=strlen(getbuff4);
									sprintf(buf,"\n\r size getbuff4 [%d]",size);
									sAPI_UartPrintf(buf);
									
									//sprintf(getbuff5,"\r\n{\r\n\"VOM\":\"%01d\",\"V1\":\"%02d:%02d:00,%01d\",\"V2\":\"%02d:%02d:00,%01d\",\"V3\":\"%02d:%02d:00,%01d\",\"V4\":\"%02d:%02d:00,%01d\",\"V5\":\"%02d:%02d:00,%01d\",\"V6\":\"%02d:%02d:00,%01d\",\"V7\":\"%02d:%02d:00,%01d\",\"V8\":\"%02d:%02d:00,%01d\",\"V9\":\"%02d:%02d:00,%01d\",\"V10\":\"%02d:%02d:00,%01d\",\"RT\":\"%02d:%02d:%02d\",\"CRSF\":\"%01d\",\"CRST\":\"%02d:%02d:00\",\"CRM\":\"%02d:%02d:%02d\",\"CRSL\":\"%02d\",\"CNO\":\"%02d\",\"CCF\":\"%01d\",\"NVC\":\"%01d\",\"SS\":\"%01d,%01d\",\r\n\"MOS\":\"%03d,%03d,%03d,%03d\"\r\n},",ValveSetting,LiveValveTimerHr[0],LiveValveTimerMin[0],ValveStatus[0],LiveValveTimerHr[1],LiveValveTimerMin[1],ValveStatus[1],LiveValveTimerHr[2],LiveValveTimerMin[2],ValveStatus[2],LiveValveTimerHr[3],LiveValveTimerMin[3],ValveStatus[3],LiveValveTimerHr[4],LiveValveTimerMin[4],ValveStatus[4],LiveValveTimerHr[5],LiveValveTimerMin[5],ValveStatus[5],LiveValveTimerHr[6],LiveValveTimerMin[6],ValveStatus[6],LiveValveTimerHr[7],LiveValveTimerMin[7],ValveStatus[7],LiveValveTimerHr[8],LiveValveTimerMin[8],ValveStatus[8],LiveValveTimerHr[9],LiveValveTimerMin[9],ValveStatus[9],RemainingOnHr,RemainingOnMin,RemainingOnSec,CyclicIntevelFlag,CyclicIntevelHr,CyclicIntevelMin,RecCyclicIntevelHr,RecCyclicIntevelMin,RecCyclicIntevelsec,CyclicLimitRec,CycleNo,CyclicComplete,NoOfValveConfigRec,SetSerial[0],SetSerial[1],Mos[0],Mos[1],Mos[2],Mos[3]);
									sprintf(getbuff5,"\r\n{\r\n\"VOM\":\"%01d\",\"V1\":\"%02d:%02d:00,%01d\",\"V2\":\"%02d:%02d:00,%01d\",\"V3\":\"%02d:%02d:00,%01d\",\"V4\":\"%02d:%02d:00,%01d\",\"V5\":\"%02d:%02d:00,%01d\",\"V6\":\"%02d:%02d:00,%01d\",\"V7\":\"%02d:%02d:00,%01d\",\"V8\":\"%02d:%02d:00,%01d\",\"V9\":\"%02d:%02d:00,%01d\",\"V10\":\"%02d:%02d:00,%01d\",\"RT\":\"%02d:%02d:%02d\",\"CRSF\":\"%01d\",\"CRST\":\"%02d:%02d:00\",\"CRM\":\"%02d:%02d:%02d\",",ValveSetting,LiveValveTimerHr[0],LiveValveTimerMin[0],ValveStatus[0],LiveValveTimerHr[1],LiveValveTimerMin[1],ValveStatus[1],LiveValveTimerHr[2],LiveValveTimerMin[2],ValveStatus[2],LiveValveTimerHr[3],LiveValveTimerMin[3],ValveStatus[3],LiveValveTimerHr[4],LiveValveTimerMin[4],ValveStatus[4],LiveValveTimerHr[5],LiveValveTimerMin[5],ValveStatus[5],LiveValveTimerHr[6],LiveValveTimerMin[6],ValveStatus[6],LiveValveTimerHr[7],LiveValveTimerMin[7],ValveStatus[7],LiveValveTimerHr[8],LiveValveTimerMin[8],ValveStatus[8],LiveValveTimerHr[9],LiveValveTimerMin[9],ValveStatus[9],RemainingOnHr,RemainingOnMin,RemainingOnSec,CyclicIntevelFlag,CyclicIntevelHr,CyclicIntevelMin,RecCyclicIntevelHr,RecCyclicIntevelMin,RecCyclicIntevelsec);
									
									sprintf(buf,"\n\r getbuff5 [%s]",getbuff5);
									sAPI_UartPrintf(buf);
									size=strlen(getbuff5);
									sprintf(buf,"\n\r size getbuff5 [%d]",size);
									sAPI_UartPrintf(buf);
									
									sprintf(getbuff3,"\"CNF\":\"%01d\",\"CRSL\":\"%02d\",\"CNO\":\"%02d\",\"CCF\":\"%01d\",\"NVC\":\"%01d\",\"SS\":\"%01d,%01d\",\r\n\"MOS\":\"%.01f,%.01f,%.01f,%.01f\",\r\n\"STM\":\"%.01f\",\r\n\"BAT\":\"%.01f,%.01f\",\r\n\"SOL\":\"%d,%d\"\r\n,\r\n\"LIT\":\"%d\"\r\n,\r\n\"LIS\":\"%d\"\r\n,\r\n\"SPF\":\"%d\"\r\n},",CycOnOf,CyclicLimitRec,CycleNo,CyclicComplete,NoOfValveConfigRec,SetSerial[0],SetSerial[1],Mos[0],Mos[1],Mos[2],Mos[3],SoilTemp,Battery[0],Battery[1],Solar[0],Solar[1],LightFlag,LightStandaloneFlag,AutoRstFlag);
									
									sprintf(buf,"\n\r getbuff3 [%s]",getbuff3);
									sAPI_UartPrintf(buf);
									size=strlen(getbuff3);
									sprintf(buf,"\n\r size getbuff3 %d",size);
									sAPI_UartPrintf(buf);
									
									sprintf(getbuff,"%s",getbuff1);
									sprintf(buf,"\n\r line %d getbuff [%s]",__LINE__,getbuff);
									sAPI_UartPrintf(buf);
									size=strlen(getbuff);
									sprintf(buf,"\n\r size getbuff %d",size);
									sAPI_UartPrintf(buf);
									/* sprintf(getbuff,"%s%s",getbuff,getbuff2);
									sprintf(buf,"\n\r line %d getbuff %s",__LINE__,getbuff);
									sAPI_UartPrintf(buf); */
									sprintf(getbuff,"%s%s",getbuff,getbuff5);
									size=strlen(getbuff);
									sprintf(getbuff,"%s%s",getbuff,getbuff3);
									sprintf(buf,"\n\r line %d getbuff [%s]",__LINE__,getbuff);
									sAPI_UartPrintf(buf);
									  /*  sprintf(buf,"\n\r line %d getbuff [%s]",__LINE__,getbuff);
									sAPI_UartPrintf(buf); */
								sprintf(buf,"\n\r size  %d",size);
							sAPI_UartPrintf(buf);
									sprintf(getbuff,"%s%s",getbuff,getbuff4);
									   size=strlen(getbuff);
									   sprintf(buf,"\n\r line %d getbuff %s",__LINE__,getbuff);
									sAPI_UartPrintf(buf);
								sprintf(buf,"\n\r size %d",size);
							sAPI_UartPrintf(buf);
							
							
							sprintf(TCPWifigprsstrBUFF,"%s",getbuff);
                            sprintf(buf,"\n\rTCPWifigprsstrBUFF>>:[%s]",TCPWifigprsstrBUFF);
							sAPI_UartPrintf(buf);
							
						    }
							#endif
						/* else if(getPsflag == 1) //oro_doubt
						{
							sprintf(TCPWifigprsstrBUFF,"{\"TS\":\"%s\"}",pumpsettingbuf);
							sprintf(buf,"\n\rTCPWifigprsstrBUFF>>:%s",TCPWifigprsstrBUFF);
							sAPI_UartPrintf(buf);
						}  */
						sgetflag = 1;
						sgetflag_1 = 1;
						sAPI_UartPrintf("\n\rSETFLAG_1:%d", sgetflag_1);
						tcpdcounter = 0;
					}
				}
			}
			
			else if (DeviceConfig.interface == WIFI && tcpopenflag == 1 && wifiopenflag == 1 && gettcpcounter >= 55 && clostcpcounter <= 50 && livedataflag == 0)

			{
				char getbuff[500];
				gettcpcounter = 0;
				tcpdcounter = 0;
				clostcpcounter++;

				sprintf(getbuff, "GET /api/v1/controller/%s/message/ HTTP/1.1\r\nHost: %s:8080\r\nUser-Agent: curl/7.52.1\r\nAccept: application/json\r\nContent-Type: application/json\r\n\r\n", IMEI, DeviceConfig.TcpServerIP);
				//	sprintf(getbuff,"GET /api/v1/getlivestatus/ HTTP/1.1\r\nHost: %s:8091\r\nUser-Agent: curl/7.52.1\r\nAccept: application/json\r\nContent-Type: application/json\r\n\r\n",IMEI,DeviceConfig.TcpServerIP);

				sprintf(buf, "tcp copy=%s", getbuff);
				sAPI_UartPrintf(buf);
				sAPI_UartWrite(eat_uart_wifi, getbuff, strlen(getbuff));
			}
			
			else if (DeviceConfig.interface == GPRS && gettcpcounter > 70 && fotaflsg == 0 && CallConnected == 0 && ringflag == 0 && livedataflag == 0)
			{
				gettcpcounter = 0;
				sgetflag = 2;
				// eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 4, "SGET", EAT_NULL);
				tcpdcounter = 0;
			}
			sAPI_GetRealTimeClock(&datetime);
			seconds_data = (datetime.tm_hour * 3600) + (datetime.tm_min * 60) + datetime.tm_sec;
			if (nMSettings.ndebugonof == 1)
				sAPI_UartPrintf("\n\r seconds_data is %ld", seconds_data);
			if (datetime.tm_sec != ActtimerPrvSec)
			{

				ActtimerPrvSec = datetime.tm_sec;
				Ten_mins_sec_timer++;
				if (nMSettings.ndebugonof == 1)
					sAPI_UartPrintf("Ten_mins_sec_timer:%d,%d,%d", Ten_mins_sec_timer, Date_Change_Flag, Change_Date_Flag);

				//  if(nVaTr.CurrentSec >=86390 && Date_Change_Flag ==0 && Change_Date_Flag ==0)
				if (seconds_data > 86390 && Date_Change_Flag == 0 && Change_Date_Flag == 0)
				{
					Date_Change_Flag = 1;
					sAPI_UartPrintf("\n\r Date_Change_Flag is %d", Date_Change_Flag);
				}

				else if (seconds_data > 0 && seconds_data < 86390 && Change_Date_Flag == 1)
				{
					Change_Date_Flag = 0;
					sAPI_UartPrintf("\n\r Change_Date_Flag is %d line %d", Date_Change_Flag, __LINE__);
				}

#if 0	  
						  if(PowerCurrentCondition ==0)
							  
							  {	
								  //if(nSTATE_MOTOR ==STATE_MOTOR_ON  )
								// if(nSTATE_MOTOR ==STATE_MOTOR_ON ||nSTATE_MOTOR ==STATE_MOTOR_TRIP_DRYRUN_SCAN || nSTATE_MOTOR ==STATE_MOTOR_TRIP_OVERLOAD_SCAN ||nSTATE_MOTOR ==STATE_MOTOR_TRIP_LOWPRESS_SCAN ||nSTATE_MOTOR ==STATE_MOTOR_TRIP_HIGHPRESS_SCAN || runmodule_start_flag == 1 ||nDripSettings.dripstandalone==1)
								if(nSTATE_MOTOR ==STATE_MOTOR_ON ||nSTATE_MOTOR ==STATE_MOTOR_TRIP_DRYRUN_SCAN || nSTATE_MOTOR ==STATE_MOTOR_TRIP_OVERLOAD_SCAN ||nSTATE_MOTOR ==STATE_MOTOR_TRIP_LOWPRESS_SCAN ||nSTATE_MOTOR ==STATE_MOTOR_TRIP_HIGHPRESS_SCAN )
								  {
									  tday[6].Motor_Runtime++;
								  }
								  else if(nSTATE_MOTOR == STATE_MOTOR_OFF )
								  {
									  tday[6].Motor_Idletime++;
								  }
								  
								  else if(nSTATE_MOTOR == STATE_MOTOR_TRIP_DRYRUN)
								  {
									  tday[6].Dry_run_trip_time++;
								  }
								  else if(nSTATE_MOTOR == STATE_MOTOR_TRIP_OFFDELAY)
								  {
									  tday[6].cyclic_trip_time++;
								  }
								  else
									  tday[6].other_trip_time++;
							  }
							  
							  if(PowerCurrentCondition ==1)
								  {
									  tday[6].Power_off_time++;
								  }
#endif
				// sprintf(buf,"\n\r tday[6].Motor_Runtime is %ld \n tday[6].Motor_Idletime %ld \n tday[6].Power_off_time %ld \n tday[6].Dry_run_trip_time %ld \n tday[6].cyclic_trip_time %ld \n tday[6].other_trip_time %ld",tday[6].Motor_Runtime,tday[6].Motor_Idletime,tday[6].Power_off_time,tday[6].Dry_run_trip_time,tday[6].cyclic_trip_time,tday[6].other_trip_time);
				// sAPI_UartPrintf(buf);
				if (Ten_mins_sec_timer >= 600) // 600
				{
					if (nMSettings.ndebugonof == 1)
						sAPI_UartPrintf("Entry Ten_mins_sec_timer:%d,%d", Ten_mins_sec_timer, Date_Change_Flag);
					// onehour_send_flag=1;
					Ten_mins_sec_timer = 0;
					// Writemotordata(6);
					// Readmotordata(6);
				}

				if (Date_Change_Flag == 1)
				{
					sAPI_UartPrintf("\n\r entry to Date_Change_Flag ", Date_Change_Flag);

					Date_Change_Flag = 0;
					Change_Date_Flag = 1;
					day_send_flag = 0; // day_send_flag=1; DG_ORO
#if 0
									  for(i=1;i<=6;i++)
									  {
								        tday[i-1]=tday[i];            //copying the struct										
										//  tday[i-1].Motor_Runtime =tday[i].Motor_Runtime;
										// tday[i-1].Motor_Idletime =tday[i].Motor_Idletime;
										// tday[i-1].Power_off_time =tday[i].Power_off_time;
										// tday[i-1].Dry_run_trip_time =tday[i].Dry_run_trip_time;
										// tday[i-1].cyclic_trip_time =tday[i].cyclic_trip_time; 
										// tday[i-1].other_trip_time =tday[i].other_trip_time; 
										// tday[i-1].day_flow=tday[i].day_flow;
										// Writemotordata(i-1);
									  }
									  memset(&tday[6],0,sizeof(tday[6]));    //Clearing the struct after copied 
									  //  tday[6].Motor_Runtime=0;
									  // tday[6].Motor_Idletime=0;
									  // tday[6].Power_off_time=0;
									  // tday[6].Dry_run_trip_time=0;
									  // tday[6].cyclic_trip_time=0;
									  // tday[6].other_trip_time=0; 
								      // tday[6].day_flow=0;
									  // flow_first_time_flag=0;
									  // Writemotordata(6);
									//	 tday[i-1]=tday[i];
									 for(int i=5;i>=0;i--)
									 {
									   sAPI_UartPrintf("\n\r tday[%d].Motor_Runtime is %ld \n tday[%d].Motor_Idletime %ld \n tday[%d].Power_off_time %ld \n tday[%d].Dry_run_trip_time %ld \n tday[%d].cyclic_trip_time %ld \n tday[%d].other_trip_time %ld ",i,tday[i].Motor_Runtime,i,tday[i].Motor_Idletime,i,tday[i].Power_off_time,i,tday[i].Dry_run_trip_time,i,tday[i].cyclic_trip_time,i,tday[i].other_trip_time);
									 }
#endif
					// sAPI_UartPrintf("\n\r tday[5].Motor_Runtime is %ld \n tday[5].Motor_Idletime %ld \n tday[5].Power_off_time %ld \n tday[5].Dry_run_trip_time %ld \n tday[5].cyclic_trip_time %ld \n tday[5].other_trip_time %ld ",tday[5].Motor_Runtime,tday[5].Motor_Idletime,tday[5].Power_off_time,tday[5].Dry_run_trip_time,tday[5].cyclic_trip_time,tday[5].other_trip_time);
					// sAPI_UartPrintf("\n\r tday[4].Motor_Runtime is %ld \n tday[4].Motor_Idletime %ld \n tday[4].Power_off_time %ld \n tday[4].Dry_run_trip_time %ld \n tday[4].cyclic_trip_time %ld \n tday[4].other_trip_time %ld",tday[4].Motor_Runtime,tday[4].Motor_Idletime,tday[4].Power_off_time,tday[4].Dry_run_trip_time,tday[4].cyclic_trip_time,tday[4].other_trip_time);
					// sAPI_UartPrintf("\n\r tday[3].Motor_Runtime is %ld \n tday[3].Motor_Idletime %ld \n tday[3].Power_off_time %ld \n tday[3].Dry_run_trip_time %ld \n tday[3].cyclic_trip_time %ld \n tday[3].other_trip_time %ld",tday[3].Motor_Runtime,tday[3].Motor_Idletime,tday[3].Power_off_time,tday[3].Dry_run_trip_time,tday[3].cyclic_trip_time,tday[3].other_trip_time);
					// sAPI_UartPrintf("\n\r tday[2].Motor_Runtime is %ld \n tday[2].Motor_Idletime %ld \n tday[2].Power_off_time %ld \n tday[2].Dry_run_trip_time %ld \n tday[2].cyclic_trip_time %ld \n tday[2].other_trip_time %ld",tday[2].Motor_Runtime,tday[2].Motor_Idletime,tday[2].Power_off_time,tday[2].Dry_run_trip_time,tday[2].cyclic_trip_time,tday[2].other_trip_time);
					// sAPI_UartPrintf("\n\r tday[1].Motor_Runtime is %ld \n tday[1].Motor_Idletime %ld \n tday[1].Power_off_time %ld \n tday[1].Dry_run_trip_time %ld \n tday[1].cyclic_trip_time %ld \n tday[1].other_trip_time %ld",tday[1].Motor_Runtime,tday[1].Motor_Idletime,tday[1].Power_off_time,tday[1].Dry_run_trip_time,tday[1].cyclic_trip_time,tday[1].other_trip_time);
					// sAPI_UartPrintf("\n\r tday[0].Motor_Runtime is %ld \n tday[0].Motor_Idletime %ld \n tday[0].Power_off_time %ld \n tday[0].Dry_run_trip_time %ld \n tday[0].cyclic_trip_time %ld \n tday[0].other_trip_time %ld",tday[0].Motor_Runtime,tday[0].Motor_Idletime,tday[0].Power_off_time,tday[0].Dry_run_trip_time,tday[0].cyclic_trip_time,tday[0].other_trip_time);
				}
			}

			if (onehour_send_flag == 1)
			{

				if (Nooftcprecvd <= 49) // 150
				{
					s32 ret;
					u8 *k = EAT_NULL;
					char TpStr4[50] = "nimish";
					char getbuff[500];
					long TpHr, TpMin, TpSec, value;
					long runvtim, runvflow;
					long TpHr1, TpMin1, TpSec1, value1;
					long TpHr2, TpMin2, TpSec2, value2;
					float pervtim;
					float pervflow;
					char flowbuf[50] = {0};
					char presbuf[50] = {0};
					char levelbuf[50] = {0};

					sAPI_UartPrintf("\n entry to send onehour_send_flag line %d", __LINE__);
					onehour_send_flag = 0;

					nCurretnCond.RYVoltage = CalculatePhToPh(RVoltage1, YVoltage1);
					nCurretnCond.YBVoltage = CalculatePhToPh(YVoltage1, BVoltage1);
					nCurretnCond.BRVoltage = CalculatePhToPh(BVoltage1, RVoltage1);

					sprintf(getbuff, "{\r\n\"cC\":\"%s\",\r\n\"cM\":\"%03d,%03d,%03d,%03.0f,%03.0f,%03.0f,", IMEI, RVoltage1, YVoltage1, BVoltage1, nCurretnCond.RYVoltage, nCurretnCond.YBVoltage, nCurretnCond.BRVoltage); //,DIS_BUF,VAL_BUF,datetime.day,datetime.mon,datetime.year,datetime.hour,datetime.min,datetime.tm_sec);
					if (nMSettings.ndebugonof == 1)
						sAPI_UartPrintf("\n\r load current nCurretnCond.Rcurrent is %f nCurretnCond.Ycurrent is %f nCurretnCond.Bcurrent is %f", nCurretnCond.Rcurrent, nCurretnCond.Ycurrent, nCurretnCond.Bcurrent);

					FloatroString1Dig(TpStr4, nCurretnCond.Rcurrent);
					sprintf(getbuff, "%s%s,", getbuff, TpStr4);
					if (nMSettings.ndebugonof == 1)
						sAPI_UartPrintf("\n\r Rcurrent TpStr4 is %s", TpStr4);
					FloatroString1Dig(TpStr4, nCurretnCond.Ycurrent);
					sprintf(getbuff, "%s%s,", getbuff, TpStr4);
					if (nMSettings.ndebugonof == 1)
						sAPI_UartPrintf("\n\r Ycurrent TpStr4 is %s", TpStr4);
					FloatroString1Dig(TpStr4, nCurretnCond.Bcurrent);
					if (nMSettings.ndebugonof == 1)
						sAPI_UartPrintf("\n\r Bcurrent TpStr4 is %s", TpStr4);
					sprintf(getbuff, "%s%s-", getbuff, TpStr4);

					value = nMoTr.Act2powerRunTimer;
					TpHr = value / 3600;
					value = value % 3600;
					TpMin = value / 60;
					value = value % 60;
					TpSec = value;
					/* value = nMoTr.Actl2powerRunTimer;
					TpHr1 = value/3600;
					value = value%3600;
					TpMin1 = value/60;
					value = value%60;
					TpSec1 = value;		 */
					//						sprintf(BigSMS,"%s2P PWR ON TIM\nC:%02ld:%02ld LPON TIM=%02ld:%02ld \n",BigSMS,TpHr,TpMin,TpHr1,TpMin1);
					sprintf(BigSMS, "%sNP=%01d,2PPONTIM=%02d:%02d:00,", getbuff, g_no_of_pumps, TpHr, TpMin);
					value = nMoTr.Act3powerRunTimer;
					TpHr = value / 3600;
					value = value % 3600;
					TpMin = value / 60;
					value = value % 60;
					TpSec = value;
					/* value = nMoTr.Actl3powerRunTimer;
					TpHr1 = value/3600;
					value = value%3600;
					TpMin1 = value/60;
					value = value%60;
					TpSec1 = value;		 */
					//						sprintf(BigSMS,"%s3P PWR ON TIM\nC:%02ld:%02ld LPON TIM=%02ld:%02ld \n",BigSMS,TpHr,TpMin,TpHr1,TpMin1);
					sprintf(BigSMS, "%s3PPONTIM=%02d:%02d:00,", BigSMS, TpHr, TpMin);
					value = nMoTr.ActnopowerRunTimer;
					TpHr = value / 3600;
					value = value % 3600;
					TpMin = value / 60;
					value = value % 60;
					TpSec = value;
					/* value = nMoTr.ActlnopowerRunTimer;
					TpHr1 = value/3600;
					value = value%3600;
					TpMin1 = value/60;
					value = value%60;
					TpSec1 = value;  */
					//						sprintf(BigSMS,"%sPWR OFF TIM\nC:%02ld:%02ld LPOF TIM=%02ld:%02ld \n",BigSMS,TpHr,TpMin,TpHr1,TpMin1);
					sprintf(BigSMS, "%sPOFFTIM=%02d:%02d:00,", BigSMS, TpHr, TpMin);

					//    value = tday[0].Motor_Runtime+tday[0].Motor_Idletime+tday[0].Dry_run_trip_time+tday[0].cyclic_trip_time+tday[0].other_trip_time; //nMoTr.Act2powerRunTimer+nMoTr.Act3powerRunTimer;
					value = ttday[0].m_M1_Runtime + ttday[0].m_M1_Idletime + ttday[0].m_M1Dry_run_trip_time + ttday[0].m_M1cyclic_trip_time + ttday[0].m_M1other_trip_time;
					TpHr = value / 3600;
					value = value % 3600;
					TpMin = value / 60;
					value = value % 60;
					TpSec = value;

					/* value = nMoTr.ActnopowerRunTimer;
					TpHr1 = value/3600;
					value = value%3600;
					TpMin1 = value/60;
					value = value%60;
					TpSec1 = value; */
					value = nMoTr.ActnopowerRunTimer; // nMoTr.ActnopowerRunTimer;
					TpHr1 = value / 3600;
					value = value % 3600;
					TpMin1 = value / 60;
					value = value % 60;
					TpSec1 = value;

					sprintf(BigSMS, "%sTPONTIM=%02d:%02d:00,TPOFTIM=%02d:%02d:00-", BigSMS, TpHr, TpMin, TpHr1, TpMin1);
					//	sprintf(getbuff,"{\r\n\"cC\":\"%s\",\r\n\"cM\":\"%s,",IMEI,BigSMS);//,DIS_BUF,VAL_BUF,datetime.day,datetime.mon,datetime.year,datetime.hour,datetime.min,datetime.tm_sec);
					/* value = tday[6].Motor_Runtime;
					TpHr1 = value/3600;
					value = value%3600;
					TpMin1 = value/60; */

					value = ttday[0].m_M1_Runtime;
					TpHr = value / 3600;
					value = value % 3600;
					TpMin = value / 60;
					value = value % 60;
					TpSec = value;

					value = ttday[0].m_M2_Runtime;
					TpHr1 = value / 3600;
					value = value % 3600;
					TpMin1 = value / 60;
					value = value % 60;
					TpSec1 = value;

					value = ttday[0].m_M3_Runtime;
					TpHr2 = value / 3600;
					value = value % 3600;
					TpMin2 = value / 60;
					value = value % 60;
					TpSec2 = value;

					//		sprintf(BigSMS,"%sRTIM=%02d:%02d:%02d;%02d:%02d:%02d;%02d:%02d:%02d,RFLO=%d;0;0,",BigSMS,TpHr,TpMin,TpSec,TpHr1,TpMin1,TpSec1,TpHr2,TpMin2,TpSec2,Runflow1,Runflow2,Runflow3);
					/* sprintf(BigSMS,"%sRTIM=%02d:%02d:%02d;%02d:%02d:%02d;%02d:%02d:%02d,RFLO=%07d;%07d;%07d,",BigSMS,TpHr,TpMin,TpSec,TpHr1,TpMin1,TpSec1,TpHr2,TpMin2,TpSec2,cumulative_flow1[0],cumulative_flow1[1],cumulative_flow1[2]); */ // sunil added

					sprintf(BigSMS, "%sRTIM=%02d:%02d:%02d;%02d:%02d:%02d;%02d:%02d:%02d", BigSMS, TpHr, TpMin, TpSec, TpHr1, TpMin1, TpSec1, TpHr2, TpMin2, TpSec2);

					if (s_npump[0].m_flowonof == 1)
						sprintf(flowbuf, "RFLO=%07d;-;-", g_Runflow_day);
					else if (s_npump[1].m_flowonof == 1)
						sprintf(flowbuf, "RFLO=-;%07d;-", g_Runflow_day);
					else if (s_npump[2].m_flowonof == 1)
						sprintf(flowbuf, "RFLO=-;-;%07d", g_Runflow_day);
					else
						sprintf(flowbuf, "RFLO=-;-;-");
					sprintf(BigSMS, "%s,%s,", BigSMS, flowbuf);

					value = ttday[0].m_M1_Idletime;
					TpHr = value / 3600;
					value = value % 3600;
					TpMin = value / 60;
					value = value % 60;
					TpSec = value;

					value = ttday[0].m_M2_Idletime;
					TpHr1 = value / 3600;
					value = value % 3600;
					TpMin1 = value / 60;
					value = value % 60;
					TpSec1 = value;

					value = ttday[0].m_M3_Idletime;
					TpHr2 = value / 3600;
					value = value % 3600;
					TpMin2 = value / 60;
					value = value % 60;
					TpSec2 = value;

					sprintf(BigSMS, "%sMIDLEPTIM=%02d:%02d:%02d;%02d:%02d:%02d;%02d:%02d:%02d,", BigSMS, TpHr, TpMin, TpSec, TpHr1, TpMin1, TpSec1, TpHr2, TpMin2, TpSec2);

					value = ttday[0].m_M1Dry_run_trip_time;
					TpHr = value / 3600;
					value = value % 3600;
					TpMin = value / 60;
					value = value % 60;
					TpSec = value;

					value = ttday[0].m_M2Dry_run_trip_time;
					TpHr1 = value / 3600;
					value = value % 3600;
					TpMin1 = value / 60;
					value = value % 60;
					TpSec1 = value;

					value = ttday[0].m_M3Dry_run_trip_time;
					TpHr2 = value / 3600;
					value = value % 3600;
					TpMin2 = value / 60;
					value = value % 60;
					TpSec2 = value;

					sprintf(BigSMS, "%sDTRIPTIM=%02d:%02d:%02d;%02d:%02d:%02d;%02d:%02d:%02d,", BigSMS, TpHr, TpMin, TpSec, TpHr1, TpMin1, TpSec1, TpHr2, TpMin2, TpSec2);

					value = ttday[0].m_M1cyclic_trip_time;
					TpHr = value / 3600;
					value = value % 3600;
					TpMin = value / 60;
					value = value % 60;
					TpSec = value;

					value = ttday[0].m_M2cyclic_trip_time;
					TpHr1 = value / 3600;
					value = value % 3600;
					TpMin1 = value / 60;
					value = value % 60;
					TpSec1 = value;

					value = ttday[0].m_M3cyclic_trip_time;
					TpHr2 = value / 3600;
					value = value % 3600;
					TpMin2 = value / 60;
					value = value % 60;
					TpSec2 = value;

					sprintf(BigSMS, "%sCYCTRIPTIM=%02d:%02d:%02d;%02d:%02d:%02d;%02d:%02d:%02d,", BigSMS, TpHr, TpMin, TpSec, TpHr1, TpMin1, TpSec1, TpHr2, TpMin2, TpSec2);

					value = ttday[0].m_M1other_trip_time;
					TpHr = value / 3600;
					value = value % 3600;
					TpMin = value / 60;
					value = value % 60;
					TpSec = value;

					value = ttday[0].m_M2other_trip_time;
					TpHr1 = value / 3600;
					value = value % 3600;
					TpMin1 = value / 60;
					value = value % 60;
					TpSec1 = value;

					value = ttday[0].m_M3other_trip_time;
					TpHr2 = value / 3600;
					value = value % 3600;
					TpMin2 = value / 60;
					value = value % 60;
					TpSec2 = value;

					// sprintf(BigSMS,"%sOTRIPTIM=%02d:%02d,%ld,%ld,%3.1f,%3.1f,%3.1f",BigSMS,TpHr1,TpMin1,tday[6].day_flow,cum_flow,zone[nDripSettings.stp].runflowrate,zone[nDripSettings.stp].runpressure,zone[nDripSettings.stp].outpressure);
					sprintf(BigSMS, "%sOTRIPTIM=%02d:%02d:%02d;%02d:%02d:%02d;%02d:%02d:%02d", BigSMS, TpHr, TpMin, TpSec, TpHr1, TpMin1, TpSec1, TpHr2, TpMin2, TpSec2);

					// sprintf(BigSMS,"%s%.02fF-%02d",BigSMS,leveltank,levelpercent);
					// sprintf(BigSMS,"%s0.00F-00",BigSMS);
					if ((s_npump[0].m_flowonof == 0) && (s_npump[1].m_flowonof == 0) && (s_npump[2].m_flowonof == 0))
						sprintf(flowbuf, "CF:-,FL:-");

					else if (s_npump[0].m_flowonof == 1)
						sprintf(flowbuf, "CF:%07d,FL:%2.01f", cumulative_flow1[0], Act_lps1[0]);
					else if (s_npump[1].m_flowonof == 1)
						sprintf(flowbuf, "CF:%07d,FL:%2.01f", cumulative_flow1[1], Act_lps1[1]);
					else
						sprintf(flowbuf, "CF:%07d,FL:%2.01f", cumulative_flow1[2], Act_lps1[2]);
					// sprintf(flowbuf,"%s,%0.2f",flowbuf,cumulative_flow);
					//	sprintf(flowbuf,"%s,%0.2f",flowbuf,cumulative_flow);

					sprintf(BigSMS, "%s,%s", BigSMS, flowbuf);

					if ((s_npump[0].m_pressureonof == 0) && (s_npump[1].m_pressureonof == 0) && (s_npump[2].m_pressureonof == 0))
						sprintf(presbuf, "PR:-");
					else if (s_npump[0].m_pressureonof == 1)
						sprintf(presbuf, "PR:%0.2f", Act_pressure[0]);
					else if (s_npump[1].m_pressureonof == 1)
						sprintf(presbuf, "PR:%0.2f", Act_pressure[1]);
					else
						sprintf(presbuf, "PR:%0.2f", Act_pressure[2]);

					sprintf(BigSMS, "%s,%s", BigSMS, presbuf);

					if ((s_npump[0].m_Level_on_off == 0) && (s_npump[1].m_Level_on_off == 0) && (s_npump[2].m_Level_on_off == 0))
						sprintf(levelbuf, "LV:-");
					else if (s_npump[0].m_Level_on_off == 1)
					{
						/*levelpercent=(Act_level[0]/s_ntank.m_tank_height)*100;
					   if(levelpercent>100)
					   levelpercent=100;*/
						sprintf(levelbuf, "LV:%03.01f", Act_level[0]);
					}

					else if (s_npump[1].m_Level_on_off == 1)
					{
						/*levelpercent=(Act_level[1]/s_ntank.m_tank_height)*100;
					   if(levelpercent>100)
					   levelpercent=100; */
						sprintf(levelbuf, "LV:%03.01f", Act_level[1]);
					}
					else
					{
						/*levelpercent=(Act_level[2]/s_ntank.m_tank_height)*100;
					   if(levelpercent>100)
					   levelpercent=100;*/
						sprintf(levelbuf, "LV:%03.01f", Act_level[2]);
					}

					sprintf(BigSMS, "%s,%s", BigSMS, levelbuf);

					/* if((s_npump[0].m_flowonof == 0)&& (s_npump[1].m_flowonof == 0)&&(s_npump[2].m_flowonof == 0))
					sprintf(flowbuf,"DF:-");
					else
					sprintf(flowbuf,"DF:%07d",g_Runflow_day); */
					// sprintf(flowbuf,"%s,%0.2f",flowbuf,cumulative_flow);
					//	sprintf(flowbuf,"%s,%0.2f",flowbuf,cumulative_flow);

					//	sprintf(BigSMS,"%s,%s",BigSMS,flowbuf);

					sprintf(getbuff, "%s", BigSMS);
					if (nMSettings.ndebugonof == 1)
						sAPI_UartPrintf("DIS_BUF copy =%s", DIS_BUF);
					if (DIS_BUF[0] == '$')
					{
						k = DIS_BUF;
						len = strlen(DIS_BUF);
						for (i = 0; i < len; i++)
						{
							if (*k == '\n')
								*k = ' ';
							else if (*k == '\r')
								*k = ' ';
							k++;
						}
					}

					else
					{
						strcpy(DIS_BUF, "$L,     NO DATA,    ");
					}
					sAPI_UartPrintf("DIS_BUF copy =%s", DIS_BUF);

					sprintf(getbuff, "%s\",\r\n\"cD\":\"%04d-%02d-%02d\",\r\n\"cT\":\"%02d:%02d:%02d\",\r\n\"mC\":\"TMR\"\r\n}", getbuff, datetime.tm_year, datetime.tm_mon, datetime.tm_mday, datetime.tm_hour, datetime.tm_min, datetime.tm_sec);

					sprintf(TCPwifiStrNumber[Nooftcprecvd].TCPWifigprsstr, "%s", getbuff);
					Nooftcprecvd++;
				}
			}

			/* if(onehour_send_flag==1)
	{

		if(Nooftcprecvd<=49)//150
			{
		s32 ret;
		u8  *k = EAT_NULL;
		char TpStr4[50] = "nimish";
		char getbuff[500];
		long TpHr,TpMin,TpSec,value;
		long runvtim,runvflow;
		long TpHr1,TpMin1,TpSec1,value1;
		long TpHr2,TpMin2,TpSec2,value2;
		float pervtim;
		float pervflow;

		sAPI_UartPrintf("\n entry to send onehour_send_flag line %d",__LINE__);
		onehour_send_flag=0;

		nCurretnCond.RYVoltage = CalculatePhToPh(RVoltage1,YVoltage1);
		nCurretnCond.YBVoltage = CalculatePhToPh(YVoltage1,BVoltage1);
		nCurretnCond.BRVoltage = CalculatePhToPh(BVoltage1,RVoltage1);

		sprintf(getbuff,"{\r\n\"cC\":\"%s\",\r\n\"cM\":\"%03d,%03d,%03d,%03.0f,%03.0f,%03.0f,",IMEI,RVoltage1,YVoltage1,BVoltage1,nCurretnCond.RYVoltage,nCurretnCond.YBVoltage,nCurretnCond.BRVoltage);//,DIS_BUF,VAL_BUF,datetime.day,datetime.mon,datetime.year,datetime.hour,datetime.min,datetime.tm_sec);
			if(nMSettings.ndebugonof==1)
			sAPI_UartPrintf("\n\r load current nCurretnCond.Rcurrent is %f nCurretnCond.Ycurrent is %f nCurretnCond.Bcurrent is %f",nCurretnCond.Rcurrent,nCurretnCond.Ycurrent,nCurretnCond.Bcurrent);

			FloatroString1Dig(TpStr4,nCurretnCond.Rcurrent);
			sprintf(getbuff,"%s%s,",getbuff,TpStr4);
			if(nMSettings.ndebugonof==1)
			sAPI_UartPrintf("\n\r Rcurrent TpStr4 is %s",TpStr4);
			FloatroString1Dig(TpStr4,nCurretnCond.Ycurrent);
			sprintf(getbuff,"%s%s,",getbuff,TpStr4);
			if(nMSettings.ndebugonof==1)
			sAPI_UartPrintf("\n\r Ycurrent TpStr4 is %s",TpStr4);
			FloatroString1Dig(TpStr4,nCurretnCond.Bcurrent);
			if(nMSettings.ndebugonof==1)
			sAPI_UartPrintf("\n\r Bcurrent TpStr4 is %s",TpStr4);
			sprintf(getbuff,"%s%s-",getbuff,TpStr4);


		value = nMoTr.Act2powerRunTimer;
		TpHr = value/3600;
		value = value%3600;
		TpMin = value/60;
		value = value%60;
		TpSec = value;
		value = nMoTr.Actl2powerRunTimer;
		TpHr1 = value/3600;
		value = value%3600;
		TpMin1 = value/60;
		value = value%60;
		TpSec1 = value;
		// sprintf(BigSMS,"%s2P PWR ON TIM\nC:%02ld:%02ld LPON TIM=%02ld:%02ld \n",BigSMS,TpHr,TpMin,TpHr1,TpMin1);
		sprintf(BigSMS,"%sNP=%01d,2PPONTIM=%02d:%02d:00,2PLPONTIM=%02d:%02d:00,",getbuff,g_no_of_pumps,TpHr,TpMin,TpHr1,TpMin1);
		value = nMoTr.Act3powerRunTimer;
		TpHr = value/3600;
		value = value%3600;
		TpMin = value/60;
		value = value%60;
		TpSec = value;
		value = nMoTr.Actl3powerRunTimer;
		TpHr1 = value/3600;
		value = value%3600;
		TpMin1 = value/60;
		value = value%60;
		TpSec1 = value;
		// sprintf(BigSMS,"%s3P PWR ON TIM\nC:%02ld:%02ld LPON TIM=%02ld:%02ld \n",BigSMS,TpHr,TpMin,TpHr1,TpMin1);
		sprintf(BigSMS,"%s3PPONTIM=%02d:%02d:00,3PLPONTIM=%02d:%02d:00,",BigSMS,TpHr,TpMin,TpHr1,TpMin1);
		value = nMoTr.ActnopowerRunTimer;
		TpHr = value/3600;
		value = value%3600;
		TpMin = value/60;
		value = value%60;
		TpSec = value;
		value = nMoTr.ActlnopowerRunTimer;
		TpHr1 = value/3600;
		value = value%3600;
		TpMin1 = value/60;
		value = value%60;
		TpSec1 = value;
		// sprintf(BigSMS,"%sPWR OFF TIM\nC:%02ld:%02ld LPOF TIM=%02ld:%02ld \n",BigSMS,TpHr,TpMin,TpHr1,TpMin1);
		sprintf(BigSMS,"%sPOFFTIM=%02d:%02d:00,LPOFTIM=%02d:%02d:00,",BigSMS,TpHr,TpMin,TpHr1,TpMin1);





		value = tday[6].Motor_Runtime+tday[6].Motor_Idletime+tday[6].Dry_run_trip_time+tday[6].cyclic_trip_time+tday[6].other_trip_time; //nMoTr.Act2powerRunTimer+nMoTr.Act3powerRunTimer;
		TpHr = value/3600;
		value = value%3600;
		TpMin = value/60;
		value = value%60;
		TpSec = value;


		value = tday[6].Power_off_time;  //nMoTr.ActnopowerRunTimer;
		TpHr1 = value/3600;
		value = value%3600;
		TpMin1 = value/60;
		value = value%60;
		TpSec1 = value;

		sprintf(BigSMS,"%sTPONTIM=%02d:%02d:00,TPOFTIM=%02d:%02d:00-",BigSMS,TpHr,TpMin,TpHr1,TpMin1);



		value = ttday[0].m_M1_Runtime;
		TpHr = value/3600;
		value = value%3600;
		TpMin = value/60;
		value = value%60;
		TpSec = value;

		value = ttday[0].m_M2_Runtime;
		TpHr1 = value/3600;
		value = value%3600;
		TpMin1 = value/60;
		value = value%60;
		TpSec1 = value;

		value = ttday[0].m_M3_Runtime;
		TpHr2 = value/3600;
		value = value%3600;
		TpMin2 = value/60;
		value = value%60;
		TpSec2 = value;


		sprintf(BigSMS,"%sRTIM=%02d:%02d:%02d;%02d:%02d:%02d;%02d:%02d:%02d,RFLO=%d;0;0,LDRTIM=%02d:%02d:00;00:00:00;00:00:00,LDRFLO=%d;0;0,",BigSMS,TpHr,TpMin,TpSec,TpHr1,TpMin1,TpSec1,TpHr2,TpMin2,TpSec2,Runflow1,Runflow2,Runflow3,TpHr1,TpMin1,LastDayRunflow);


		value = ttday[0].m_M1_Idletime;
		TpHr = value/3600;
		value = value%3600;
		TpMin = value/60;
		TpSec = value;

		value = ttday[0].m_M2_Idletime;
		TpHr1 = value/3600;
		value = value%3600;
		TpMin1 = value/60;
		value = value%60;
		TpSec1 = value;

		value = ttday[0].m_M3_Idletime;
		TpHr2 = value/3600;
		value = value%3600;
		TpMin2 = value/60;
		value = value%60;
		TpSec2 = value;

		sprintf(BigSMS,"%sMIDLEPTIM=%02d:%02d:%02d;%02d:%02d:%02d;%02d:%02d:%02d,",BigSMS,TpHr,TpMin,TpSec,TpHr1,TpMin1,TpSec1,TpHr2,TpMin2,TpSec2);



		value = ttday[0].m_M1Dry_run_trip_time;
		TpHr = value/3600;
		value = value%3600;
		TpMin = value/60;
		TpSec = value;

		value = ttday[0].m_M2Dry_run_trip_time;
		TpHr1 = value/3600;
		value = value%3600;
		TpMin1 = value/60;
		value = value%60;
		TpSec1 = value;

		value = ttday[0].m_M3Dry_run_trip_time;
		TpHr2 = value/3600;
		value = value%3600;
		TpMin2 = value/60;
		value = value%60;
		TpSec2 = value;

		sprintf(BigSMS,"%sDTRIPTIM=%02d:%02d:%02d;%02d:%02d:%02d;%02d:%02d:%02d,",BigSMS,TpHr,TpMin,TpSec,TpHr1,TpMin1,TpSec1,TpHr2,TpMin2,TpSec2);


		value = ttday[0].m_M1cyclic_trip_time;
		TpHr = value/3600;
		value = value%3600;
		TpMin = value/60;
		TpSec = value;

		value = ttday[0].m_M2cyclic_trip_time;
		TpHr1 = value/3600;
		value = value%3600;
		TpMin1 = value/60;
		value = value%60;
		TpSec1 = value;

		value = ttday[0].m_M3cyclic_trip_time;
		TpHr2 = value/3600;
		value = value%3600;
		TpMin2 = value/60;
		value = value%60;
		TpSec2 = value;

		sprintf(BigSMS,"%sCYCTRIPTIM=%02d:%02d:%02d;%02d:%02d:%02d;%02d:%02d:%02d,",BigSMS,TpHr,TpMin,TpSec,TpHr1,TpMin1,TpSec1,TpHr2,TpMin2,TpSec2);

		value = ttday[0].m_M1other_trip_time;
		TpHr = value/3600;
		value = value%3600;
		TpMin = value/60;
		TpSec = value;

		value = ttday[0].m_M2other_trip_time;
		TpHr1 = value/3600;
		value = value%3600;
		TpMin1 = value/60;
		value = value%60;
		TpSec1 = value;

		value = ttday[0].m_M3other_trip_time;
		TpHr2 = value/3600;
		value = value%3600;
		TpMin2 = value/60;
		value = value%60;
		TpSec2 = value;



		sprintf(BigSMS,"%sOTRIPTIM=%02d:%02d:%02d;%02d:%02d:%02d;%02d:%02d:%02d",BigSMS,TpHr,TpMin,TpSec,TpHr1,TpMin1,TpSec1,TpHr2,TpMin2,TpSec2);




		sprintf(getbuff,"%s",BigSMS);
		if(nMSettings.ndebugonof==1)
			sAPI_UartPrintf("DIS_BUF copy =%s",DIS_BUF);
		if(DIS_BUF[0] == '$')
		{
			k = DIS_BUF;
			len=strlen(DIS_BUF);
			for (i = 0; i <len; i++)
			{
				if(*k=='\n')
					*k = ' ';
				else if(*k=='\r')
					*k = ' ';
				k++;
			}
		}

		else
		{
		strcpy(DIS_BUF,"$L,     NO DATA,    ");}
		sAPI_UartPrintf("DIS_BUF copy =%s",DIS_BUF);

		sprintf(getbuff,"%s\",\r\n\"cD\":\"%04d-%02d-%02d\",\r\n\"cT\":\"%02d:%02d:%02d\",\r\n\"mC\":\"TMR\"\r\n}",getbuff,datetime.tm_year,datetime.tm_mon,datetime.tm_mday,datetime.tm_hour,datetime.tm_min,datetime.tm_sec);

		sprintf(TCPwifiStrNumber[Nooftcprecvd].TCPWifigprsstr,"%s",getbuff);
		Nooftcprecvd++;

		}

	 } */

			/*		if(onehour_send_flag==1)
					{

						if(Nooftcprecvd<=49)//150
							{
						s32 ret;
						u8  *k = EAT_NULL;
						char TpStr4[50] = "nimish";
						char getbuff[500];
						long TpHr,TpMin,TpSec,value;
						long runvtim,runvflow;
						long TpHr1,TpMin1,TpSec1,value1;
						float pervtim;
						float pervflow;

						sAPI_UartPrintf("\n entry to send onehour_send_flag line %d",__LINE__);
						onehour_send_flag=0;

						sprintf(getbuff,"{\r\n\"cC\":\"%s\",\r\n\"cM\":\"%03.0f,%03.0f,%03.0f,%03.0f,%03.0f,%03.0f,",IMEI,nCurretnCond.RVoltage,nCurretnCond.YVoltage,nCurretnCond.BVoltage,nCurretnCond.RYVoltage,nCurretnCond.YBVoltage,nCurretnCond.BRVoltage);//,DIS_BUF,VAL_BUF,datetime.day,datetime.mon,datetime.year,datetime.hour,datetime.min,datetime.tm_sec);
							if(nMSettings.ndebugonof==1)
							sAPI_UartPrintf("\n\r load current nCurretnCond.Rcurrent is %f nCurretnCond.Ycurrent is %f nCurretnCond.Bcurrent is %f",nCurretnCond.Rcurrent,nCurretnCond.Ycurrent,nCurretnCond.Bcurrent);

							FloatroString1Dig(TpStr4,nCurretnCond.Rcurrent);
							sprintf(getbuff,"%s%s,",getbuff,TpStr4);
							if(nMSettings.ndebugonof==1)
							sAPI_UartPrintf("\n\r Rcurrent TpStr4 is %s",TpStr4);
							FloatroString1Dig(TpStr4,nCurretnCond.Ycurrent);
							sprintf(getbuff,"%s%s,",getbuff,TpStr4);
							if(nMSettings.ndebugonof==1)
							sAPI_UartPrintf("\n\r Ycurrent TpStr4 is %s",TpStr4);
							FloatroString1Dig(TpStr4,nCurretnCond.Bcurrent);
							if(nMSettings.ndebugonof==1)
							sAPI_UartPrintf("\n\r Bcurrent TpStr4 is %s",TpStr4);
							sprintf(getbuff,"%s%s-",getbuff,TpStr4);


						value = nMoTr.Act2powerRunTimer;
						TpHr = value/3600;
						value = value%3600;
						TpMin = value/60;
						value = value%60;
						TpSec = value;
						value = nMoTr.Actl2powerRunTimer;
						TpHr1 = value/3600;
						value = value%3600;
						TpMin1 = value/60;
						value = value%60;
						TpSec1 = value;
//						sprintf(BigSMS,"%s2P PWR ON TIM\nC:%02ld:%02ld LPON TIM=%02ld:%02ld \n",BigSMS,TpHr,TpMin,TpHr1,TpMin1);
						sprintf(BigSMS,"%sNP=%01d,2PPONTIM=%02d:%02d:00,2PLPONTIM=%02d:%02d:00,",getbuff,g_no_of_pumps,TpHr,TpMin,TpHr1,TpMin1);
						value = nMoTr.Act3powerRunTimer;
						TpHr = value/3600;
						value = value%3600;
						TpMin = value/60;
						value = value%60;
						TpSec = value;
						value = nMoTr.Actl3powerRunTimer;
						TpHr1 = value/3600;
						value = value%3600;
						TpMin1 = value/60;
						value = value%60;
						TpSec1 = value;
//						sprintf(BigSMS,"%s3P PWR ON TIM\nC:%02ld:%02ld LPON TIM=%02ld:%02ld \n",BigSMS,TpHr,TpMin,TpHr1,TpMin1);
						sprintf(BigSMS,"%s3PPONTIM=%02d:%02d:00,3PLPONTIM=%02d:%02d:00,",BigSMS,TpHr,TpMin,TpHr1,TpMin1);
						value = nMoTr.ActnopowerRunTimer;
						TpHr = value/3600;
						value = value%3600;
						TpMin = value/60;
						value = value%60;
						TpSec = value;
						value = nMoTr.ActlnopowerRunTimer;
						TpHr1 = value/3600;
						value = value%3600;
						TpMin1 = value/60;
						value = value%60;
						TpSec1 = value;
//						sprintf(BigSMS,"%sPWR OFF TIM\nC:%02ld:%02ld LPOF TIM=%02ld:%02ld \n",BigSMS,TpHr,TpMin,TpHr1,TpMin1);
						sprintf(BigSMS,"%sPOFFTIM=%02d:%02d:00,LPOFTIM=%02d:%02d:00,",BigSMS,TpHr,TpMin,TpHr1,TpMin1);





						value = tday[6].Motor_Runtime+tday[6].Motor_Idletime+tday[6].Dry_run_trip_time+tday[6].cyclic_trip_time+tday[6].other_trip_time; //nMoTr.Act2powerRunTimer+nMoTr.Act3powerRunTimer;
						TpHr = value/3600;
						value = value%3600;
						TpMin = value/60;
						value = value%60;
						TpSec = value;


						value = tday[6].Power_off_time;  //nMoTr.ActnopowerRunTimer;
						TpHr1 = value/3600;
						value = value%3600;
						TpMin1 = value/60;
						value = value%60;
						TpSec1 = value;

						sprintf(BigSMS,"%sTPONTIM=%02d:%02d:00,TPOFTIM=%02d:%02d:00-",BigSMS,TpHr,TpMin,TpHr1,TpMin1);
					//	sprintf(getbuff,"{\r\n\"cC\":\"%s\",\r\n\"cM\":\"%s,",IMEI,BigSMS);//,DIS_BUF,VAL_BUF,datetime.day,datetime.mon,datetime.year,datetime.hour,datetime.min,datetime.tm_sec);
						value = tday[6].Motor_Runtime;
						TpHr1 = value/3600;
						value = value%3600;
						TpMin1 = value/60;


						value = RunTimer;
						TpHr = value/3600;
						value = value%3600;
						TpMin = value/60;
						value = value%60;
						TpSec = value;

						value = LastDayRunTimer;
						TpHr1 = value/3600;
						value = value%3600;
						TpMin1 = value/60;
						value = value%60;
						TpSec1 = value;
//						sprintf(BigSMS,"RUN TIME\nC:%02ld:%02ld LM TIM=%02ld:%02ld \nLM FLO=%d\nFLO=%d\n",TpHr,TpMin,TpHr1,TpMin1,LastDayRunflow,Runflow);
//						sprintf(BigSMS,"RUN TIME C:%02ld:%02ld LM TIM=%02ld:%02ld LM FLO=%d FLO=%d",TpHr,TpMin,TpHr1,TpMin1,LastDayRunflow,Runflow);
						sprintf(BigSMS,"%sRTIM=%02d:%02d:00;00:00:00;00:00:00,RFLO=%d;0;0,LDRTIM=%02d:%02d:00;00:00:00;00:00:00,LDRFLO=%d;0;0,",BigSMS,TpHr,TpMin,Runflow,TpHr1,TpMin1,LastDayRunflow);
						sprintf(BigSMS,"%sMRUNTIM=%02d:%02d:00;00:00:00;00:00:00,",BigSMS,TpHr1,TpMin1);



						value = tday[6].Motor_Idletime;
						TpHr1 = value/3600;
						value = value%3600;
						TpMin1 = value/60;

						sprintf(BigSMS,"%sMIDLEPTIM=%02d:%02d:00;00:00:00;00:00:00,",BigSMS,TpHr1,TpMin1);



						value = tday[6].Dry_run_trip_time;
						TpHr1 = value/3600;
						value = value%3600;
						TpMin1 = value/60;

						sprintf(BigSMS,"%sDTRIPTIM=%02d:%02d:00;00:00:00;00:00:00,",BigSMS,TpHr1,TpMin1);

						value = tday[6].cyclic_trip_time;
						TpHr1 = value/3600;
						value = value%3600;
						TpMin1 = value/60;

						sprintf(BigSMS,"%sCYCTRIPTIM=%02d:%02d:00;00:00:00;00:00:00,",BigSMS,TpHr1,TpMin1);

						value = tday[6].other_trip_time;
						TpHr1 = value/3600;
						value = value%3600;
						TpMin1 = value/60;

						// sprintf(BigSMS,"%sOTRIPTIM=%02d:%02d,%ld,%ld,%3.1f,%3.1f,%3.1f",BigSMS,TpHr1,TpMin1,tday[6].day_flow,cum_flow,zone[nDripSettings.stp].runflowrate,zone[nDripSettings.stp].runpressure,zone[nDripSettings.stp].outpressure);
						sprintf(BigSMS,"%sOTRIPTIM=%02d:%02d:00;00:00:00;00:00:00",BigSMS,TpHr1,TpMin1);

						// sprintf(BigSMS,"%s%.02fF-%02d",BigSMS,leveltank,levelpercent);
						//sprintf(BigSMS,"%s0.00F-00",BigSMS);


						sprintf(getbuff,"%s",BigSMS);
						if(nMSettings.ndebugonof==1)
							sAPI_UartPrintf("DIS_BUF copy =%s",DIS_BUF);
						if(DIS_BUF[0] == '$')
						{
							k = DIS_BUF;
							len=strlen(DIS_BUF);
							for (i = 0; i <len; i++)
							{
								if(*k=='\n')
									*k = ' ';
								else if(*k=='\r')
									*k = ' ';
								k++;
							}
						}

						else
						{
						strcpy(DIS_BUF,"$L,     NO DATA,    ");}
						sAPI_UartPrintf("DIS_BUF copy =%s",DIS_BUF);

						sprintf(getbuff,"%s\",\r\n\"cD\":\"%04d-%02d-%02d\",\r\n\"cT\":\"%02d:%02d:%02d\",\r\n\"mC\":\"TMR\"\r\n}",getbuff,datetime.tm_year,datetime.tm_mon,datetime.tm_mday,datetime.tm_hour,datetime.tm_min,datetime.tm_sec);

						sprintf(TCPwifiStrNumber[Nooftcprecvd].TCPWifigprsstr,"%s",getbuff);
						Nooftcprecvd++;

						}

					 }*/

#if 0	
					
					 if(nMSettings.ndebugonof==1)
					sAPI_UartPrintf("\nEntry day_send_flag:%d",day_send_flag);
					if(day_send_flag==1 || day_send_flag==2) {
					 
						if(Nooftcprecvd<=49)//150
						{
						
						UINT32 ret;
						t_rtc Time={0};
						UINT8  *k = NULL;
						char TpStr4[50] = "nimish";
						char getbuff[500];
						long TpHr,TpMin,TpSec,value;
						long runvtim,runvflow;
						long TpHr1,TpMin1,TpSec1,value1;
						float pervtim;
						float pervflow;
						
						sAPI_UartPrintf("\n entry to send day_send_flag line %d",__LINE__);
						 if(day_send_flag==1)
						 {
						  
						sAPI_UartPrintf("\n entry to send day_send_flag line %d",__LINE__);					 
						 

					//	sprintf(getbuff,"{\r\n\"cC\":\"%s\",\r\n\"cM\":\"%03.0f,%03.0f,%03.0f,%03.0f,%03.0f,%03.0f,",IMEI,nCurretnCond.RVoltage,nCurretnCond.YVoltage,nCurretnCond.BVoltage,nCurretnCond.RYVoltage,nCurretnCond.YBVoltage,nCurretnCond.BRVoltage);//,DIS_BUF,VAL_BUF,datetime.day,datetime.mon,datetime.year,datetime.hour,datetime.min,datetime.sec);
							
						memset(BigSMS,NULL,sizeof(BigSMS));
						memset(getbuff,NULL,sizeof(getbuff)); 
						
						value=tday[6].Motor_Runtime;
						TpHr = value/3600;
						value = value%3600;
						TpMin = value/60;
						value = value%60;
						TpSec = value; 
						
						value = tday[6].Motor_Idletime;
						TpHr1 = value/3600;
						value = value%3600;
						TpMin1 = value/60;
						value = value%60;
						TpSec1 = value;	
						sprintf(BigSMS,"MRT1:%02d:%02d:%02d,MIT1:%02d:%02d:%02d,",TpHr,TpMin,TpSec,TpHr1,TpMin1,TpSec1);
						
						
						
						value=tday[6].Dry_run_trip_time;
						TpHr = value/3600;
						value = value%3600;
						TpMin = value/60;
						value = value%60;
						TpSec = value; 
						
						value = tday[6].cyclic_trip_time;
						TpHr1 = value/3600;
						value = value%3600;
						TpMin1 = value/60;
						value = value%60;
						TpSec1 = value;	
						sprintf(BigSMS,"%sDRT1:%02d:%02d:%02d,CTT1:%02d:%02d:%02d,",BigSMS,TpHr,TpMin,TpSec,TpHr1,TpMin1,TpSec1);
						
						
						value=tday[6].other_trip_time;
						TpHr = value/3600;
						value = value%3600;
						TpMin = value/60;
						value = value%60;
						TpSec = value; 
						 
						
						// sprintf(BigSMS,"%sOTT1:%02d:%02d:%02d,DF1:%ld,",BigSMS,TpHr,TpMin,TpSec,tday[6].day_flow);
						sprintf(BigSMS,"%sOTT1:%02d:%02d:%02d,DF1:0,",BigSMS,TpHr,TpMin,TpSec);
						
						
						value=tday[5].Motor_Runtime;
						TpHr = value/3600;
						value = value%3600;
						TpMin = value/60;
						value = value%60;
						TpSec = value; 
						
						value = tday[5].Motor_Idletime;
						TpHr1 = value/3600;
						value = value%3600;
						TpMin1 = value/60;
						value = value%60;
						TpSec1 = value;	
						sprintf(BigSMS,"%sMRT2:%02d:%02d:%02d,MIT2:%02d:%02d:%02d,",BigSMS,TpHr,TpMin,TpSec,TpHr1,TpMin1,TpSec1);
						
						
						
						value=tday[5].Dry_run_trip_time;
						TpHr = value/3600;
						value = value%3600;
						TpMin = value/60;
						value = value%60;
						TpSec = value; 
						
						value = tday[5].cyclic_trip_time;
						TpHr1 = value/3600;
						value = value%3600;
						TpMin1 = value/60;
						value = value%60;
						TpSec1 = value;	
						sprintf(BigSMS,"%sDRT2:%02d:%02d:%02d,CTT2:%02d:%02d:%02d,",BigSMS,TpHr,TpMin,TpSec,TpHr1,TpMin1,TpSec1);
						
						
						value=tday[5].other_trip_time;
						TpHr = value/3600;
						value = value%3600;
						TpMin = value/60;
						value = value%60;
						TpSec = value; 
						// sprintf(BigSMS,"%sOTT2:%02d:%02d:%02d,DF2:%ld,",BigSMS,TpHr,TpMin,TpSec,tday[5].day_flow);
						sprintf(BigSMS,"%sOTT2:%02d:%02d:%02d,DF2:0,",BigSMS,TpHr,TpMin,TpSec);
						
						
						value=tday[4].Motor_Runtime;
						TpHr = value/3600;
						value = value%3600;
						TpMin = value/60;
						value = value%60;
						TpSec = value; 
						
						value = tday[4].Motor_Idletime;
						TpHr1 = value/3600;
						value = value%3600;
						TpMin1 = value/60;
						value = value%60;
						TpSec1 = value;	
						sprintf(BigSMS,"%sMRT3:%02d:%02d:%02d,MIT3:%02d:%02d:%02d,",BigSMS,TpHr,TpMin,TpSec,TpHr1,TpMin1,TpSec1);
						
						
						
						value=tday[4].Dry_run_trip_time;
						TpHr = value/3600;
						value = value%3600;
						TpMin = value/60;
						value = value%60;
						TpSec = value; 
						
						value = tday[4].cyclic_trip_time;
						TpHr1 = value/3600;
						value = value%3600;
						TpMin1 = value/60;
						value = value%60;
						TpSec1 = value;	
						sprintf(BigSMS,"%sDRT3:%02d:%02d:%02d,CTT3:%02d:%02d:%02d,",BigSMS,TpHr,TpMin,TpSec,TpHr1,TpMin1,TpSec1);
						
						
						value=tday[4].other_trip_time;
						TpHr = value/3600;
						value = value%3600;
						TpMin = value/60;
						value = value%60;
						TpSec = value; 
						// sprintf(BigSMS,"%sOTT3:%02d:%02d:%02d,DF3:%ld,",BigSMS,TpHr,TpMin,TpSec,tday[4].day_flow);
						sprintf(BigSMS,"%sOTT3:%02d:%02d:%02d,DF3:0,",BigSMS,TpHr,TpMin,TpSec);
						memset(getbuff,NULL,sizeof(getbuff)+1);
						sprintf(getbuff,"{\r\n\"cC\":\"%s\",\r\n\"cM\":\",%s",IMEI,BigSMS);//,DIS_BUF,VAL_BUF,datetime.day,datetime.mon,datetime.year,datetime.hour,datetime.min,datetime.sec);
						
						
					}
					else if(day_send_flag == 2)
					{
							
							sAPI_UartPrintf("\n entry to send day_send_flag line %d",__LINE__);
						memset(&BigSMS,'\0',sizeof(BigSMS));
						
						value=tday[3].Motor_Runtime;
						TpHr = value/3600;
						value = value%3600;
						TpMin = value/60;
						value = value%60;
						TpSec = value; 
						
						value = tday[3].Motor_Idletime;
						TpHr1 = value/3600;
						value = value%3600;
						TpMin1 = value/60;
						value = value%60;
						TpSec1 = value;	
						sprintf(BigSMS,"%sMRT4:%02d:%02d:%02d,MIT4:%02d:%02d:%02d,",BigSMS,TpHr,TpMin,TpSec,TpHr1,TpMin1,TpSec1);
						
						
						
						value=tday[3].Dry_run_trip_time;
						TpHr = value/3600;
						value = value%3600;
						TpMin = value/60;
						value = value%60;
						TpSec = value; 
						
						value = tday[3].cyclic_trip_time;
						TpHr1 = value/3600;
						value = value%3600;
						TpMin1 = value/60;
						value = value%60;
						TpSec1 = value;	
						sprintf(BigSMS,"%sDRT4:%02d:%02d:%02d,CTT4:%02d:%02d:%02d,",BigSMS,TpHr,TpMin,TpSec,TpHr1,TpMin1,TpSec1);
						
						
						value=tday[3].other_trip_time;
						TpHr = value/3600;
						value = value%3600;
						TpMin = value/60;
						value = value%60;
						TpSec = value; 
						// sprintf(BigSMS,"%sOTT3:%02d:%02d:%02d,DF4:%ld,",BigSMS,TpHr,TpMin,TpSec,tday[3].day_flow);
						sprintf(BigSMS,"%sOTT3:%02d:%02d:%02d,DF4:0,",BigSMS,TpHr,TpMin,TpSec);
						
						
						value=tday[2].Motor_Runtime;
						TpHr = value/3600;
						value = value%3600;
						TpMin = value/60;
						value = value%60;
						TpSec = value; 
						
						value = tday[2].Motor_Idletime;
						TpHr1 = value/3600;
						value = value%3600;
						TpMin1 = value/60;
						value = value%60;
						TpSec1 = value;	
						sprintf(BigSMS,"%sMRT5:%02d:%02d:%02d,MIT5:%02d:%02d:%02d,",BigSMS,TpHr,TpMin,TpSec,TpHr1,TpMin1,TpSec1);
						
						
						
						value=tday[2].Dry_run_trip_time;
						TpHr = value/3600;
						value = value%3600;
						TpMin = value/60;
						value = value%60;
						TpSec = value; 
						
						value = tday[2].cyclic_trip_time;
						TpHr1 = value/3600;
						value = value%3600;
						TpMin1 = value/60;
						value = value%60;
						TpSec1 = value;	
						sprintf(BigSMS,"%sDRT5:%02d:%02d:%02d,CTT5:%02d:%02d:%02d,",BigSMS,TpHr,TpMin,TpSec,TpHr1,TpMin1,TpSec1);
						
						
						value=tday[2].other_trip_time;
						TpHr = value/3600;
						value = value%3600;
						TpMin = value/60;
						value = value%60;
						TpSec = value; 
						// sprintf(BigSMS,"%sOTT2:%02d:%02d:%02d,DF5:%ld,",BigSMS,TpHr,TpMin,TpSec,tday[2].day_flow);
						sprintf(BigSMS,"%sOTT2:%02d:%02d:%02d,DF5:0,",BigSMS,TpHr,TpMin,TpSec);
						
						
						value=tday[1].Motor_Runtime;
						TpHr = value/3600;
						value = value%3600;
						TpMin = value/60;
						value = value%60;
						TpSec = value; 
						
						value = tday[1].Motor_Idletime;
						TpHr1 = value/3600;
						value = value%3600;
						TpMin1 = value/60;
						value = value%60;
						TpSec1 = value;	
						sprintf(BigSMS,"%sMRT6:%02d:%02d:%02d,MIT6:%02d:%02d:%02d,",BigSMS,TpHr,TpMin,TpSec,TpHr1,TpMin1,TpSec1);
						
						
						
						value=tday[1].Dry_run_trip_time;
						TpHr = value/3600;
						value = value%3600;
						TpMin = value/60;
						value = value%60;
						TpSec = value; 
						
						value = tday[1].cyclic_trip_time;
						TpHr1 = value/3600;
						value = value%3600;
						TpMin1 = value/60;
						value = value%60;
						TpSec1 = value;	
						sprintf(BigSMS,"%sDRT6:%02d:%02d:%02d,CTT6:%02d:%02d:%02d,",BigSMS,TpHr,TpMin,TpSec,TpHr1,TpMin1,TpSec1);
						
						
						value=tday[1].other_trip_time;
						TpHr = value/3600;
						value = value%3600;
						TpMin = value/60;
						value = value%60;
						TpSec = value; 
						// sprintf(BigSMS,"%sOTT1:%02d:%02d:%02d,DF6:%ld,",BigSMS,TpHr,TpMin,TpSec,tday[1].day_flow);
						sprintf(BigSMS,"%sOTT1:%02d:%02d:%02d,DF6:0,",BigSMS,TpHr,TpMin,TpSec);
						
						
						value=tday[0].Motor_Runtime;
						TpHr = value/3600;
						value = value%3600;
						TpMin = value/60;
						value = value%60;
						TpSec = value; 
						
						value = tday[0].Motor_Idletime;
						TpHr1 = value/3600;
						value = value%3600;
						TpMin1 = value/60;
						value = value%60;
						TpSec1 = value;	
						sprintf(BigSMS,"%sMRT7:%02d:%02d:%02d,MIT7:%02d:%02d:%02d,",BigSMS,TpHr,TpMin,TpSec,TpHr1,TpMin1,TpSec1);
						
						
						
						value=tday[0].Dry_run_trip_time;
						TpHr = value/3600;
						value = value%3600;
						TpMin = value/60;
						value = value%60;
						TpSec = value; 
						
						value = tday[0].cyclic_trip_time;
						TpHr1 = value/3600;
						value = value%3600;
						TpMin1 = value/60;
						value = value%60;
						TpSec1 = value;	
						sprintf(BigSMS,"%sDRT7:%02d:%02d:%02d,CTT7:%02d:%02d:%02d,",BigSMS,TpHr,TpMin,TpSec,TpHr1,TpMin1,TpSec1);
						
						
						value=tday[0].other_trip_time;
						TpHr = value/3600;
						value = value%3600;
						TpMin = value/60;
						value = value%60;
						TpSec = value; 
						// sprintf(BigSMS,"%sOTT7:%02d:%02d:%02d,DF7:%ld,%ld",BigSMS,TpHr,TpMin,TpSec,tday[0].day_flow,cum_flow);
						sprintf(BigSMS,"%sOTT7:%02d:%02d:%02d,DF7:0,0",BigSMS,TpHr,TpMin,TpSec);
						
						
						memset(getbuff,NULL,sizeof(getbuff));
						sprintf(getbuff,"{\r\n\"cC\":\"%s\",\r\n\"cM\":\"%s",IMEI,BigSMS);//,DIS_BUF,VAL_BUF,datetime.day,datetime.mon,datetime.year,datetime.hour,datetime.min,datetime.sec);
						}
						    
						
					//	sprintf(getbuff,"%s",BigSMS);		
					
						if(nMSettings.ndebugonof==1)
							sAPI_UartPrintf("DIS_BUF copy =%s",DIS_BUF);
						if(DIS_BUF[0] == '$')
						{
							k = DIS_BUF;
							len=strlen(DIS_BUF);
							for (i = 0; i <len; i++) 
							{
								if(*k=='\n')
									*k = ' '; 
								else if(*k=='\r')
									*k = ' ';
								k++;
							}
						}

						else
						{
					    
						strcpy(DIS_BUF,"$L,     NO DATA,    ");}
						sAPI_UartPrintf("DIS_BUF copy =%s",DIS_BUF);
						 
						sAPI_GetRealTimeClock(&Time);
						sprintf(getbuff,"%s\",\r\n\"cL\":\"%s\",\r\n\"cZ\":\"%s\",\r\n\"cD\":\"%02d/%02d/%04d\",\r\n\"cT\":\"%02d:%02d:%02d\",\r\n\"mC\":\"REC\"\r\n}",getbuff,DIS_BUF,VAL_BUF,Time.tm_mday,Time.tm_mon,Time.tm_year,Time.tm_hour,Time.tm_min,Time.tm_sec);
 
 
						// sprintf(TCPwifiStrNumber[Nooftcprecvd].TCPWifigprsstr,"%s",getbuff);
						memcpy(TCPwifiStrNumber[Nooftcprecvd].TCPWifigprsstr,getbuff,sizeof(getbuff));						 
						sAPI_UartPrintf(TCPwifiStrNumber[Nooftcprecvd].TCPWifigprsstr);
						Nooftcprecvd++; 
						
						if(day_send_flag==1)
						day_send_flag=2;
						else if(day_send_flag==2)
						day_send_flag=3;
					}
				}
#endif
#if 0		
					    else if(f_Pump_Settings_view == 1 && Nooftcprecvd<=49)//150
						{
							
						char getbuff[600];
						char SignalStrength=0;	
							
						sAPI_UartPrintf("\n\r f_Pump_Settings_view = 1");
						///////////////////////////// Delay Settings ///////////////////////////

						sprintf(BigSMS,"%02d:%02d:%02d",nTimerSettings.POnHr,nTimerSettings.POnMin,nTimerSettings.POnSec);
						if(nTimerSettings.ScrDlOnOff == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);

						sprintf(BigSMS,"%s,%02d:%02d:%02d",BigSMS,nTimerSettings.ScrDlHr,nTimerSettings.ScrDlMin,nTimerSettings.ScrDlSec);
						sprintf(BigSMS,"%s,%02d:%02d:%02d",BigSMS,nTimerSettings.SDHr,nTimerSettings.SDMin,nTimerSettings.SDSec);

						if(nMSettings.SfbOnOff == 1) 
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);

						sprintf(BigSMS,"%s,%02d:%02d:%02d",BigSMS,nTimerSettings.SfbHr,nTimerSettings.SfbMin,nTimerSettings.SfbSec);


						///////////////////////////// Current Settings ///////////////////////////

						if(nMSettings.DryRunOnOff == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);

						sprintf(BigSMS,"%s,%02d:%02d:%02d",BigSMS,nTimerSettings.DrScHr,nTimerSettings.DrScMin,nTimerSettings.DrScSec);
						sprintf(BigSMS,"%s,%3.1f,%3.1f",BigSMS,nTimerSettings.DrAmpsII,nTimerSettings.DrAmpsIII);

						if(nTimerSettings.DrReOnOf == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);

						sprintf(BigSMS,"%s,%02d:%02d:%02d",BigSMS,nTimerSettings.DrReHr,nTimerSettings.DrReMin,nTimerSettings.DrReSec);

						if(nTimerSettings.DrOccurOnOff == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);

						sprintf(BigSMS,"%s,%02d:%02d:%02d,%d",BigSMS,nTimerSettings.DrOccurTimHr,nTimerSettings.DrOccurTimMin,nTimerSettings.DrOccurTimSec,nTimerSettings.DrOccurNum);



						if(nTimerSettings.OlOnOff == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);

						sprintf(BigSMS,"%s,%02d:%02d:%02d,%3.1f,%3.1f",BigSMS,nTimerSettings.OlScanHr,nTimerSettings.OlScanMin,nTimerSettings.OlScanSec,nTimerSettings.OlAmpsII,nTimerSettings.OlAmpsIII);

						if(nTimerSettings.OlOnOff == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS); 

						if(nTimerSettings.AutoDrRunRstIIOnOff == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);

						if(nTimerSettings.AutoOlDrRstIIOnOff == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);

						if(nTimerSettings.AutoRstOn == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);


						///////////////////////////// Voltage Settings ///////////////////////////


						if(nTimerSettings.LowVoltOnOff == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);

						sprintf(BigSMS,"%s,%03d,%03d,%03d,%03d",BigSMS,nTimerSettings.LowVoltII,nTimerSettings.DiffVoltII,nTimerSettings.LowVoltIII,nTimerSettings.DiffVoltIII);

						if(nTimerSettings.HighVoltOnOff == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);

						sprintf(BigSMS,"%s,%03d,%03d,%03d,%03d",BigSMS,nTimerSettings.HighVoltII,nTimerSettings.HiDiffVoltII,nTimerSettings.HighVoltIII,nTimerSettings.HiDiffVoltIII);

						if(nTimerSettings.SppOnoff == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);


						sprintf(BigSMS,"%s,%03d",BigSMS,nTimerSettings.ImbVolt);


						if(nTimerSettings.RvePhOnoff == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);



						///////////////////////////// timer Settings ///////////////////////////

						if(nTimerSettings.CycLicOnOf == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);

						sprintf(BigSMS,"%s,%02d:%02d;%02d:%02d",BigSMS,nTimerSettings.CycLicOnHr,nTimerSettings.CycLicOnMin,nTimerSettings.CycLicOfHr,nTimerSettings.CycLicOfMin);


						if(nTimerSettings.MaxRnOnOf == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);


						sprintf(BigSMS,"%s,%02d:%02d:%02d",BigSMS,nTimerSettings.MaxRnHr,nTimerSettings.MaxRnMin,nTimerSettings.MaxRnSec);

						if(zonecom.nightlightRTCOnOf == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);

						sprintf(BigSMS,"%s,%02d:%02d;%02d:%02d",BigSMS,zonecom.nightlightRTCOnHr,zonecom.nightlightRTCOnMin,zonecom.nightlightRTCOfHr,zonecom.nightlightRTCOfMin);

						if(nTimerSettings.RTCOnOf == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);


						sprintf(BigSMS,"%s,%02d:%02d;%02d:%02d",BigSMS,nTimerSettings.RTCOnHr[1],nTimerSettings.RTCOnMin[1],nTimerSettings.RTCOfHr[1],nTimerSettings.RTCOfMin[1]);

						sprintf(BigSMS,"%s,%02d:%02d;%02d:%02d",BigSMS,nTimerSettings.RTCOnHr[2],nTimerSettings.RTCOnMin[2],nTimerSettings.RTCOfHr[2],nTimerSettings.RTCOfMin[2]);

						sprintf(BigSMS,"%s,%02d:%02d;%02d:%02d",BigSMS,nTimerSettings.RTCOnHr[3],nTimerSettings.RTCOnMin[3],nTimerSettings.RTCOfHr[3],nTimerSettings.RTCOfMin[3]);

						sprintf(BigSMS,"%s,%02d:%02d;%02d:%02d",BigSMS,nTimerSettings.RTCOnHr[3],nTimerSettings.RTCOnMin[3],nTimerSettings.RTCOfHr[4],nTimerSettings.RTCOfMin[4]);

						///////////////////////////// SMS Settings ///////////////////////////

						if(limitsmsonof == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);

						sprintf(BigSMS,"%s,%03d",BigSMS,limitsmsset);

						if(Appmodeon == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);

						if(nMSettings.sampleSMSOnOff == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);

						if(nMSettings.dataSMSOnOff == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);

						if(nMSettings.RelayControlOnCall == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);

						if(nMSettings.SecOnOf == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);

						if(nMSettings.staticSMSOnOff == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);

						if(nMSettings.gethidesmsonoff == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);


						///////////////////////////// communication configuration ///////////////////////////

						 SignalStrength = (CSQ*3.2258);
						  if(SignalStrength>100)
							  SignalStrength = 100;
						  else if(SignalStrength<0)
							  SignalStrength = 0;
						sprintf(BigSMS,"%s,%3d",BigSMS,SignalStrength); 

						if(MqttInitStatus == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);

						if(gprson == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);


						if(nMSettings.SMSOnOff == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else if(DeviceConfig.interface==WIFI)
						sprintf(BigSMS,"%s,2",BigSMS);
						else
						sprintf(BigSMS,"%s,3",BigSMS);

					//	sprintf(BigSMS,"%s,%s,%s,%s,%s,%s,%s,%s,%s,%s",BigSMS,DeviceConfig.wifiSSID,DeviceConfig.wifiPassword,DeviceConfig.ApServerIP,DeviceConfig.DevicePort,DeviceConfig.ApSSID,DeviceConfig.ApPassword,DeviceConfig.DeviceIP,DeviceConfig.DevicePort,DeviceConfig.apnName);
					
						sprintf(BigSMS,"%s,%s",BigSMS,DeviceConfig.wifiSSID);
						sprintf(BigSMS,"%s,%s",BigSMS,DeviceConfig.wifiPassword);
						sprintf(BigSMS,"%s,%s",BigSMS,DeviceConfig.ApServerIP);
						sprintf(BigSMS,"%s,%d",BigSMS,DeviceConfig.DevicePort);
						 sprintf(BigSMS,"%s,%s",BigSMS,DeviceConfig.ApSSID);
						sprintf(BigSMS,"%s,%s",BigSMS,DeviceConfig.ApPassword);
						sprintf(BigSMS,"%s,%s",BigSMS,DeviceConfig.DeviceIP);
						sprintf(BigSMS,"%s,%d",BigSMS,DeviceConfig.SocketPort);
						sprintf(BigSMS,"%s,%s",BigSMS,DeviceConfig.apnName); 
							 

						///////////////////////////// other settings ///////////////////////////


					//	sprintf(BigSMS,"%s,%3.1f:%3.1f:%3.1f,%3.1f:%3.1f:%3.1f",BigSMS,nTimerSettings.CalRVoltage,nTimerSettings.CalYVoltage,nTimerSettings.CalBVoltage,nTimerSettings.CalRCurrent,nTimerSettings.CalYCurrent,nTimerSettings.CalBCurrent);
						sprintf(BigSMS,"%s,%1.3f:%1.3f:%1.3f,%1.3f:%1.3f:%1.3f",BigSMS,nTimerSettings.CalRVoltage,nTimerSettings.CalYVoltage,nTimerSettings.CalBVoltage,nTimerSettings.CalRCurrent,nTimerSettings.CalYCurrent,nTimerSettings.CalBCurrent);

						if(nTimerSettings.PoScrDlOnOff == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);

						
						sprintf(BigSMS,"%s,%02d:%02d:%02d",BigSMS,nTimerSettings.PoScrDlHr,nTimerSettings.PoScrDlMin,nTimerSettings.PoScrDlSec);



						if(nTimerSettings.pfcOnOff == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);

						sprintf(BigSMS,"%s,%03d",BigSMS,nTimerSettings.pfcvolt);  


						if(nTimerSettings.CTRonoff == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);

						if(nTimerSettings.CTYonoff == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);

						if(nTimerSettings.CTBonoff == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);


						if(nTimerSettings.CurSppOnOff == 1)
						sprintf(BigSMS,"%s,1",BigSMS);
						else
						sprintf(BigSMS,"%s,0",BigSMS);

						// if(xxxxxx == 1)
						//sprintf(BigSMS,"%s,1",BigSMS);
						//else 
						sprintf(BigSMS,"%s,0",BigSMS);

						sprintf(BigSMS,"%s,50:70",BigSMS);

						sprintf(BigSMS,"%s,0",BigSMS); 


						if(nMSettings.ndebugonof==1)
						sAPI_UartPrintf("\n\r BigSMS is %s line %d",BigSMS,__LINE__);
						sprintf(getbuff,"{\r\n\"cC\":\"%s\",\r\n\"cM\":\"%s",IMEI,BigSMS);					
						sprintf(getbuff,"%s\",\r\n\"cD\":\"%d/%d/%04d\",\r\n\"cT\":\"%d:%d:%d\",\r\n\"mC\":\"V03\"\r\n}",getbuff,datetime.tm_mday,datetime.tm_mon,datetime.tm_year,datetime.tm_hour,datetime.tm_min,datetime.tm_sec);
						//	sprintf(TCPWifigprsstrBUFF,"%s",getbuff); 
						//sprintf(TCPwifiStrNumber1[loaddata].TCPWifigprsstrBUFF,"%s",getbuff); 
						
						memcpy(TCPwifiStrNumber[Nooftcprecvd].TCPWifigprsstr,getbuff,sizeof(getbuff));						 
						sAPI_UartPrintf(TCPwifiStrNumber[Nooftcprecvd].TCPWifigprsstr);
						Nooftcprecvd++; 
					//	loaddata++;
						// if(nMSettings.ndebugonof==1)
						// {
						// sAPI_UartPrintf("TCPWifigprsstrBUFF =%s",TCPwifiStrNumber1[loaddata].TCPWifigprsstrBUFF);

						// sAPI_UartPrintf("\n\r entry to loaddata line %d livedataflag %d livedataflag1 %d loaddata %d\n\r",__LINE__,livedataflag,livedataflag1,loaddata);
						// }
						//}
						f_Pump_Settings_view=0;
						//sgetflag_2=1;

							
						}

#endif // dg_osdk

			if (Nooftcpprocessed1 >= Nooftcprecvd1)
				if (tcpdcounter2++ > 3)
				{
					tcpdcounter2 = 0;

					Nooftcpprocessed1 = Nooftcprecvd1 = 0;
				}
			if (Nooftcpprocessed >= Nooftcprecvd && Nooftcprecvd1 == 0 && NumberOfSmsAllreadySend == 0 && NumberOfSmsNeedToSend == 0 && (Nooftcpprocessed != 0))
			{

				if (tcpdcounter2++ > 3)
				{
					tcpdcounter2 = 0;
					//	tcpopenflag=0;
					Nooftcpprocessed = Nooftcprecvd = 0;sgetflag_1=0;
					sAPI_UartPrintf("\n\r sgetflag_1=0 line %d ",__LINE__);
					// wifiopenflag=0;
					//  //eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 5, "CLOSE", EAT_NULL);
					sprintf(buf, "tcp copy done111111111111111111111111111111111111111111111111");
					sAPI_UartPrintf(buf);
				}
			}

			else if (NumberOfSmsAllreadySend >= NumberOfSmsNeedToSend)
			{

				NumberOfSmsAllreadySend = NumberOfSmsNeedToSend = 0;
				AllSmsSendDone = 1;
				SendSMS = 0;
				//	SendSmsToAll=0;
				//	sprintf(buf,"\n\r SendSmsToAll 0 line %d\n\r",__LINE__);
				//	sAPI_UartPrintf(buf);
				if (NumberChangeSMS == 1)
				{
					TargetNumberFound = 0;
					sprintf(buf, "\n\rAll SMS Send Done Now Search CPBR\n\r", pData);
					sAPI_UartPrintf(buf);
					NeedToCPBRSearchAgain = 0;
					HowManyNumberFound = 0;
					cpbrsearchend2 = 0;
					NumberFound = 0;
					NumberLocation = 1;
					MassageReceived = 2;
				}
				NumberChangeSMS = 0;
			}

			if (CallInit == 0 && GiveCallToNumber == 0)
			{
				if (CallingBecError == 1 && AllSmsSendDone == 1)
				{
					CallErrorDelay++;
					sprintf(buf, "\n\rCallErrorDelay = %d\n\r", CallErrorDelay);
					sAPI_UartPrintf(buf);
					if (CallErrorDelay >= 500)
					{
						NowPleaseDoCall = 1;
						CallErrorDelay = 0;
						CallInit = 1;
					}
				}
			}
			if (CallingBecError == 1 && AllSmsSendDone == 1 && NowPleaseDoCall == 1)
			{
				CallingBecError = 0;
				GiveCallToNumber = 1;
				NowPleaseDoCall = 0;
			}
			if (GiveCallToNumber == 1 && nMSettings.VBFOnOff == 1)
			{
				GiveCallToNumber++;
				CallBackWaitOK = 0;
			}
			if (GiveCallToNumber == 2 && nMSettings.VBFOnOff == 1)
			{
				UINT8 buf[40] = {0};
				//	sprintf(buffer,"ATD0%s;\n\r",SmsNumber[1]);
				//	sprintf(buf,"\n\r%s\n\r",buffer);
				// sAPI_UartPrintf(buf);
				// eat_modem_write((UINT8*)buffer, strlen(buffer));
				// eat_send_msg_to_user(EAT_USER_0, EAT_USER_1, EAT_FALSE, 3, "VFB", EAT_NULL);

				memset(buf, 0, 41);
				sprintf(buf, "ATD%s%s;\n\r", StoredPhoneSmscode[14], StoredPhoneNumber[14]);
				if (sAPI_AtSend(buf, strlen(buf)) != 1 /*TRUE*/)
					sAPI_UartPrintf("\n\rSend AT fail");
				else
					sAPI_UartPrintf("\n\r AT Sent\n\r");
				sAPI_UartPrintf("\n\r%s\n\r", buf);

				GiveCallToNumber++;
				WaitOkPendingDelay = 0;
			}
// sprintf(buf,"\n\rCallreceiv value:%d,SendSMS:%d \n\r",Callreceiv,SendSMS);
// sAPI_UartPrintf(buf);
#if 0
						if(Callreceiv == 1 /*&& WaitOK == 0*/ && SendSMS == 0)
						{
							WaitOK = 0;
							CallModeOn = 1;
							CallConnectedDelay = 0;
							sprintf(buf,"\n\r s_nMSettings.m_Enter to call ATA**************  ");
							sAPI_UartPrintf(buf);
							// if(ATADELAY++>0)
							if(ATADELAY++>20)  //10
							{ 
						           
								   
								     if(sAPI_AtSend("ATA\n\r",strlen("ATA\n\r"))!= 1/*TRUE*/)
									 sAPI_UartPrintf("send at fail");								
								     sAPI_TaskSleep(50);
									 
						             sAPI_CallAnswerMsg(g_call_demo_msgQ);
								     sAPI_TaskSleep(50);
						             sAPI_CallAutoAnswer(1000, g_call_demo_msgQ);
							    // sprintf(buffer,"ATA\n\r");
								//	sprintf(buf,"\n\r%s\n\r",buffer);
								//  sAPI_UartPrintf(buf);
								//	eat_modem_write((UINT8*)buffer, strlen(buffer));
								//eat_send_msg_to_user(EAT_USER_0, EAT_USER_1, EAT_FALSE, 3, "ATA", EAT_NULL);
								PLAYDELAY=0;
								WaitOK = 1;
								//SENDATA=1;
								Callreceiv++;
							}
						}
						if(Callreceiv == 2 /* && WaitOK == 0 */)
						{
							WaitOK = 0;
							//if(PLAYDELAY++>0)
							if(PLAYDELAY++>2)
							{
								HowManySoundToPlay = 0;
								HowManySoundPlayed = 0;
								PlaySound = 0;
								PlaySound =1;

								PlayNumber=WELCOME;
								HowManySound[HowManySoundToPlay] = PlayNumber;
								HowManySoundToPlay++;							 
							if(PowerCurrentCondition == 0)
							{
								PlayNumber=OKELECTRICITY;		
								HowManySound[HowManySoundToPlay] = PlayNumber;
								HowManySoundToPlay++;
								PowerCondVoice = 0;
								/*checkpower=Check2Phase();*/
								
								if(checkpower_act == 3)
								{
									PlayNumber=PH3PRESENT;		
									HowManySound[HowManySoundToPlay] = PlayNumber;
									HowManySoundToPlay++;
								}
								else
								{
									PlayNumber=PH2PRESENT;		
									HowManySound[HowManySoundToPlay] = PlayNumber;
									HowManySoundToPlay++;
								sprintf(buf,"PH2PRESENT line %d \r\n", __LINE__);
								sAPI_UartPrintf(buf);
								}
								//PlayFeedbackSound();
							}
							else
							{
								PlayNumber=NOELECTRICITY;		
								HowManySound[HowManySoundToPlay] = PlayNumber;
								HowManySoundToPlay++;
								PowerCondVoice = 0;	
								sprintf(buf,"line 45210 %d \r\n", __LINE__);
								sAPI_UartPrintf(buf);
							}								
								PlayNumber=THANKS;
								HowManySound[HowManySoundToPlay] = PlayNumber;
								HowManySoundToPlay++;
								memset(UartBuffer,NULL,sizeof(UartBuffer));
								memcpy(UartBuffer,"+RXDTMF: #",strlen("+RXDTMF: #"));
							//	sendtoMcu=1;
								sendtoMcuack=0;
							 
								Callreceiv++;
								//WaitOK = 1;
								ATADELAY=0;
							}
						}
#endif
			if (Callreceiv == 3)
				Callreceiv++;

			if (BalanceSend == 5)
			{
				//*123#
				unsigned char Tp9;

				//	StoredPhoneNumber[5][Tp9+1] = 0;
				// eat_send_msg_to_user(EAT_USER_0, EAT_USER_1, EAT_FALSE, 4, "CUSD", EAT_NULL);

				//	sprintf(Buffer1,"AT+CUSD=1,%c%s%c\n\r",'"',StoredPhoneNumber[13],'"');
				//	////////eat_modem_write((UINT8*)Buffer1, strlen((char*)Buffer1));
				//	sprintf(buf,"\n\rCusd :- %s\n\r",Buffer1);
				// sAPI_UartPrintf(buf);
				//	send_test_smsStatus(PhoneNumber);
				BalanceSend++;

				// else
				//{
				//	sprintf(buf,"\n\rNo Balance number Found\n\r");
				//  sAPI_UartPrintf(buf);
				//	BalanceSend++;
				// }
			}
			else if (BalanceSend <= 5 && BalanceSend != 0)
				BalanceSend++;

			NumberFound = 1;
			HowManyNumberFound = 1;

			if (started_firsttime == 0)
			{

				HowManySoundToPlay = 0;
				HowManySoundPlayed = 0;
				PlaySound = 0;
				//	PlayFeedbackSound(); //dg_nsdk
				HowManySoundToPlay = 0;
				HowManySoundPlayed = 0;
				PlaySound = 0;

				started_firsttime = 1;
				//	power_on_flag_count=1;
			}
			else
			{

				SC_GPIOReturnCode pinlevel;
				// sAPI_GpioSetDirection(SPP_PIN,SC_GPIO_IN_PIN);
				iret = sAPI_GpioGetValue(SPP_PIN);
				pinlevel = iret;
				if (nMSettings.ndebugonof == 1)
					sprintf(buf, "\n\rspp pin=%d\n\r", iret);
				sAPI_UartPrintf(buf);
				// QlPinLevel pinlevel;
				// iret = pinRead(PINNAME_KBC0, &pinlevel);
				if (cpbrsearchend == 1 || cpbrsearchend == 0)
				{
					if (pinlevel == 0)
					{

						if (SmsSendOntimer == 0)
						{

							RecheckPinCounter++;
							sprintf(buf, "RecheckPinCounter is %d", RecheckPinCounter);
							sAPI_UartPrintf(buf);
							if (RecheckPinCounter >= 20)
							{

								RecheckPinCounter = 0;
								if ((NumberFound == 1 && HowManyNumberFound > 0) || NumberFound == 0 || HowManyNumberFound == 0) // un_cmt
								// if(NumberFound==1 && HowManyNumberFound>0)
								{
									sAPI_UartPrintf("entry");
									if (PowerIsThere != 0 && PowerCurrentCondition != 1)
									{
										CallErrorDelay = 0;
										PowerCondCall = 1;
									}
									// MotorStarterTripCount = 0;
									OverAllStarterTrip = 0;
									ActMotorStarterTripTimer = 0;
									SendSmsToAll = 1;
									SmsSendOntimer = 1;
									PowerOnSms = 2;
									Motoronflag[0] = 0, Motorreasonflag[0] = 30;
									Motoronflag[1] = 0, Motorreasonflag[1] = 30;
									Motoronflag[2] = 0, Motorreasonflag[2] = 30;
									//	skipfbk=0;
									PowerIsThere = 0;
									NoAcceptSMS = 0;
									//	nMoTr.ActofpowerRunTimer=0; //dg_nsdk
									/* SendLowVoltageHighVoltage =0;
									SendSPPLowVoltageHighVoltage = 0;
									spplowhighvoldelay=0; */
									PowerCurrentCondition = 1;
									PingHighSmsSendOntimer = 0;
									PingHighRecheckPinCounter = 0;
									livedataflagcount1 = 0;
									livedataflag1 = 1;
									nCurretnCond.RVoltage = 0;
									nCurretnCond.YVoltage = 0;
									nCurretnCond.BVoltage = 0;

									nCurretnCond.RYVoltage = 0;
									nCurretnCond.YBVoltage = 0;
									nCurretnCond.BRVoltage = 0;

									nCurretnCond.Rcurrent = 0;
									nCurretnCond.Bcurrent = 0;
									nCurretnCond.Ycurrent = 0;
									prev_number_enu_data[0] = 0;
									prev_number_enu_data[1] = 0;
									prev_number_enu_data[2] = 0;
									
									ValveStatus[0]=0;ValveStatus[1]=0;ValveStatus[2]=0;
									ValveStatus[3]=0;ValveStatus[4]=0;ValveStatus[5]=0;
									ValveStatus[6]=0;ValveStatus[7]=0;ValveStatus[8]=0;
									ValveStatus[9]=0;
									
									/* if(nVaTr.REMTIM<=180)   //dg_check  //dg_nsdk
									{nVaTr.REMTIM=180;
									sprintf(buf,"\n\r nVaTr.REMTIM %d on\n\r",nVaTr.REMTIM);}
									sAPI_UartPrintf(buf);
									if(nMSettings.motor4ctrlonof>=1)
									valvetimernlightcounter=0;   */
									// dg_added
									//	firstvalenlightsmsonof=1;	 //dg_added
									sAPI_UartPrintf("\nPower off entry\n");
									if (Motoronflag[0] == 1)
									{

										// sprintf(s_nlogtime[on_data_M1].Act_Mtr1_On,"%02d:%02d:%02d\n",datetime.tm_hour,datetime.tm_min,datetime.tm_sec);
										// on_data_M1++;

										// sprintf(buf,"\n on_data_M1 = %d line %d\n",on_data_M1,__LINE__);
										// sAPI_UartPrintf(buf);
									}
									if (Motoronflag[1] == 1)
									{
										sprintf(s_nlogtime[on_data_M2].Act_Mtr2_On, "%02d:%02d:%02d\n", datetime.tm_hour, datetime.tm_min, datetime.tm_sec);
										on_data_M2++;

										sprintf(buf, "\n on_data_M2 = %d line %d\n", on_data_M2, __LINE__);
										sAPI_UartPrintf(buf);
									}
									if (Motoronflag[2] == 1)
									{
										sprintf(s_nlogtime[on_data_M3].Act_Mtr3_On, "%02d:%02d:%02d\n", datetime.tm_hour, datetime.tm_min, datetime.tm_sec);
										on_data_M3++;

										sprintf(buf, "\n on_data_M3 = %d line %d\n", on_data_M3, __LINE__);
										sAPI_UartPrintf(buf);
									}
								}
							}
						}
						else
							RecheckPinCounter = 0;
					}
					else
					{

						if (PingHighSmsSendOntimer == 0)
						{

							PingHighRecheckPinCounter++;

							if (PingHighRecheckPinCounter >= 30)
							{

								PingHighRecheckPinCounter = 0;
								// if(NumberFound==1 && HowManyNumberFound>0)
								if ((NumberFound == 1 && HowManyNumberFound > 0) || NumberFound == 0 || HowManyNumberFound == 0) // un_cmt
								{

									sAPI_UartPrintf("POWER IS THERE");
									if (PowerIsThere != 1 && PowerCurrentCondition != 0)
									{
										CallErrorDelay = 0;
										PowerCondCall = 1;
									}

									/* // MotorStarterTripCount = 0;
									OverAllStarterTrip = 0;
									ActMotorStarterTripTimer =0; */
									PingHighSmsSendOntimer = 1;
									SendSmsToAll = 1;
									PowerOnSms = 1;
									Motoronflag[0] = 0; Motorreasonflag[0] = 31;
									Motoronflag[1] = 0; Motorreasonflag[1] = 31;
									Motoronflag[2] = 0; Motorreasonflag[2] = 31;
									PowerIsThere = 1;
									NoAcceptSMS = 0;
									//	nMoTr.ActonpowerRunTimer=0; //dg_nsdk
									/* nMoTr.ActlnightlightRunTimer=0;
									nMoTr.ActllightRunTimer=0;
									nMoTr.ActlfanRunTimer=0; */
									/* SendLowVoltageHighVoltage =0;
									SendSPPLowVoltageHighVoltage = 0;
									spplowhighvoldelay=0; */
									PowerCurrentCondition = 0;
									SmsSendOntimer = 0;
									RecheckPinCounter = 0;
									sprintf(buffer, "AUTO START");
									strcpy(WhoMadeRelayOn, buffer);
									livedataflagcount1 = 0;
									livedataflag1 = 1;
								}
							}
						}
						else
							PingHighRecheckPinCounter = 0;
					}
				}
			}
			austat = sAPI_AudioStatus();
			if (nMSettings.ndebugonof == 1)
				sAPI_UartPrintf("sAPI_AudioStatus: %d,%d", austat, Appmodeon);

			//  if(vbatflag==1)
			// {

			// for(int i=0;i<=20;i++)
			// {
			// adc[i]=sAPI_ReadVbat();
			// sAPI_UartPrintf("adc[%d]=%d,\n\r",i,adc[i]);
			// }
			// avg_adc = ((adc[0]+adc[1]+adc[2]+adc[3]+adc[4]+adc[5]+adc[6]+adc[7]+adc[8]+adc[9]+adc[10]+adc[11]+adc[12]+adc[13]+adc[14]+adc[15]+adc[16]+adc[17]+adc[18]+adc[19])/ 20);
			// BATPER=avg_adc*1.4285;
			// if(BATPER>=100)
			// BATPER=100;
			// sprintf(buf,"vbat = %d, %.0f\r\n",avg_adc,BATPER);
			// sAPI_UartPrintf(buf);
			// vbatflag=0;
			// }
			if (fotafailflag == 1)
			{
				//	STATE_SENDSMS=STATE_FOTA_SMS; //dg_nsdk
				SendSmsToAll = 1;
				fotafailflag = 0;
				fotaflsg = 0;
			}

			// break;
		}
//#endif
#endif
	}
}

void sAPP_Netlight(void *argv)
{
	/*  sAPI_NetworkGetCsq(&CSQ);
	 sAPI_NetworkGetCreg(&Creg);
	 sAPI_NetworkGetCgreg(&CGREG); */
	sAPI_UartPrintf("%s Task creation completed!!\n", __func__);
	while (1)
	{
	//	sAPI_NetworkGetCsq(&CSQ); //dg_nsdk
	//	sAPI_NetworkGetCreg(&Creg);
	//	sAPI_NetworkGetCgreg(&CGREG);
		sAPI_UartPrintf("CSQ %d Creg %d CGREG %d!!\n", CSQ, Creg, CGREG);

		sAPI_UartPrintf("");
		if (CGREG == 1 && MqttInitStatus == 1)
		{
			sAPI_GpioSetValue(NETLIGHT, 1);
			sAPI_TaskSleep(40);
			sAPI_GpioSetValue(NETLIGHT, 0);
			sAPI_TaskSleep(40);
		}
		else if (CGREG == 1)
		{
			sAPI_GpioSetValue(NETLIGHT, 1);
			sAPI_TaskSleep(100);
			sAPI_GpioSetValue(NETLIGHT, 0);
			sAPI_TaskSleep(100);
		}
		else if (Creg == 1 && CSQ != 99)
		{
			sAPI_GpioSetValue(NETLIGHT, 1);
			sAPI_TaskSleep(40);
		}
		else
		{
			sAPI_GpioSetValue(NETLIGHT, 0);
			sAPI_TaskSleep(20);
		}
	}
}

void sAPP_NetlightTask(void)
{
	SC_STATUS status;
	if (NULL != NetlightProcesser)
    {
		 sAPI_UartPrintf("%s  line %d\n", __func__,__LINE__);
        return;
    }
	status = sAPI_TaskCreate(&NetlightProcesser, NetlightProcesserStack, 1 * 1024, 90, "Netlight", sAPP_Netlight, (void *)0);
	//	sAPI_UartPrintf("%s Task creation completed!!\n", __func__);
	if (SC_SUCCESS != status)
	{
		sAPI_UartPrintf("Task create fail");
	}
	sAPI_NetworkInit();
	sAPI_UartPrintf("%s Task creation completed!!\n", __func__);
}

/* void sTask_Uart2RxProcesser(void *ptr)
{
} */

/* void sTask_UartRxProcesser(void *ptr)
{
} */

void timerRoutine2(UINT32 argv)
{
	sAPI_Debug("timerRoutine2 ");

#ifdef USE_FLAG

	sAPI_FlagSet(g_flg2, TIMER1_OUT_EVENT_MASK, SC_FLAG_OR);
#endif
}
// static void sAPP_Timer2(void *argv)
void sAPP_Timer2(void *argv)
{
	int ret;
	SC_STATUS status;
	t_rtc timvar = {0};
	status = sAPI_TimerCreate(&timer2);
	status = sAPI_TimerStart(timer2, 200, 200, timerRoutine2, 0x1234);
	while (1)
	{
		//	sAPI_UartPrintf("\n\rIn Timer1 while  loop\n\r");

#ifdef USE_FLAG
		UINT32 event = 0;
		status = sAPI_FlagWait(g_flg2, TIMER1_OUT_EVENT_MASK, SC_FLAG_OR_CLEAR, &event, /*200*/ SC_SUSPEND);
		sAPI_Debug("status[%d] event[%d]", status, event);
		if (status == SC_SUCCESS)
		{
			// sAPI_UartPrintf("\n\r1sec timer\n\r");
			sAPI_NetworkGetCsq(&CSQ);
			sAPI_NetworkGetCreg(&Creg);
			sAPI_NetworkGetCgreg(&CGREG);
			// sAPI_SimPinGet(Cpin);

			sprintf(buf, "\n\rCSQ:%d,%d,%d,%d IMEI_req_flag %d No_comm_flag %d", CSQ, Creg, CGREG, Cpin,IMEI_req_flag,No_comm_flag);
			sAPI_UartPrintf(buf);

			
			if (IMEI_req_flag == 0 && s_nMSettings.m_Enter >= 70 && No_comm_flag ==0)
			{
				sAPI_Debug("status[%d] event[%d]", status, event);
				
				memset(Sim_Buf, 0x00, sizeof(Sim_Buf));
				sprintf(Sim_Buf,"$:5:29:114:\r");
				//sprintf(Sim_Buf, "$MI\r");
				sAPI_UartWrite(SC_UART, (UINT8 *)Sim_Buf, strlen(Sim_Buf));
				memset(Sim_Buf, 0x00, sizeof(Sim_Buf));
				sprintf(buf, "\n\r %s", Sim_Buf);
				sAPI_UartPrintf(buf);
			}
			
			if (CSQ != 99 && (Creg == 1 && CGREG == 1 || Cpin == 1))
			{
				ModemIsReady = 1;
			}
			if (nMSettings.ndebugonof == 1)
			{
				if (ftpgettofs_cb_flag == 0)
					sAPI_UartPrintf("Fw: VERSION %s CSQ: %d CREG:%d GPRS:%d tout:%d, cpbrsearchend %d,ModemIsReady %d,limitsmsflag %d,SendSmsToAll %d s_nMSettings.m_Enter %d, livedataflag=%d MakeRealyOn=%d\n", Version, CSQ, Creg, CGATT, tout, cpbrsearchend, ModemIsReady, limitsmsflag, SendSmsToAll, s_nMSettings.m_Enter, livedataflag, MakeRealyOn);
				// sAPI_UartPrintf("FOTA IS CHANGED\n");

				// sAPI_UartPrintf("mem %d,%d",sizeof(s_memPool)/sizeof(s_memPool[0]),strlen(s_memPool));
				sAPI_UartPrintf("toutt is %d", toutt);
				sAPI_UartPrintf(" CREG is %d ,creg_count is %d, creg_reg_flag is %d ,creg_reg_flag_1 is %d creg_reset_flag is %d creg_reg_count is %d", Creg, creg_count, creg_reg_flag, creg_reg_flag_1, creg_reset_flag, creg_reg_count);
			}
			if ((Creg == 1 || Creg == 5) && (s_nMSettings.m_Enter > 70))
			{
				creg_reg_flag = 0;
				creg_reg_flag_1 = 0;
				creg_reg_count = 0;
				toutt = 0;
			}
			if ((Creg == 2 || Creg == 3 || Creg == 0))
			{
				creg_reg_flag = 1;
			}
			//	if((CREG == 3) && (creg_count<=1) && (creg_reg_flag_1==1))
			if (((Creg == 3) || (Creg == 0)) && (creg_reg_flag_1 == 1))
			{

				creg_reset_flag = 1;
				WriteonofFile();
				creg_reset_flag = 0;
				creg_reg_flag_1 = 0;
				sAPI_UartPrintf("\n\rcalling Sysreset_module");
				// eat_reset_module();
				// sAPI_SysReset();
				//	}
			}
			if (GSMInitDone && (Creg == 2))
			//	if(GSMInitDone && ((CREG==0) || (CREG==2)))
			{
				creg_reg_flag = 1;
				toutt++;
			}
			/* else if(GSMInitDone && (CREG==3))
				creg_reg_flag_1=1; */
			else if (GSMInitDone && ((Creg == 3) || (Creg == 0)) && nVaTr.cregrstonof == 1)
			{
				creg_reg_count++;
				if (creg_reg_count >= nVaTr.cregdelay) // 60
				{
					creg_reg_count = 0;
					creg_reg_flag_1 = 1;
				}
			}
			else
				toutt = 0;
			if (toutt > 500)
			{
				// simcom_cfun(FunCallback);
				sAPI_NetworkSetCfun(4);
				sAPI_TaskSleep(200 * 3);
				sAPI_NetworkSetCfun(1);
				toutt = 0;
			}

			ret = sAPI_GetRealTimeClock(&timvar);
			if (timvar.tm_sec != Prvsecvar)
			{

				Prvsecvar = timvar.tm_sec;
				
				livesendcount++;
				if (nMSettings.ndebugonof == 1)
					sAPI_UartPrintf("livesendcount:%d", livesendcount);
				if (livesendcount > 60)
				{
					livedataflag = 1;
					livesendcount = 0;
				}
				if(MqttInitStatus==0)
				RST_count++;
				else 
				RST_count=0;
				sAPI_UartPrintf("MqttInitStatus:%d--%d", MqttInitStatus,RST_count);
				if(RST_count >= 300)   
				{
					RST_count=0;
					sAPI_UartPrintf("SYS_RESTART:Timer");
					//MQTT_Reconnect();
					sAPI_SysReset();//Praveen
				}
			}

			if (sendtoMcuack == 0 && sendtoMcu == 1)
			{
			//	Uart_Send_with_CRC(UartBuffer);
			}
		}
#endif
	}
}
int Data_split_Function(char* data)
{
    char split_buf[200] = {0};

    char *Pch1 = NULL;
    char *Pch2 = NULL;
    char *Pch3 = NULL;

    char *saveptr1;
    char *saveptr2;
    char *saveptr3;

    int i = 0, j = 0, l = 0;

    strcpy(split_buf, data);

    // Split by ;
    Pch1 = strtok_r(split_buf, ";", &saveptr1);

    while(Pch1 != NULL)
    {
        char temp1[100];
        strcpy(temp1, Pch1);

        // Split by ,
        Pch2 = strtok_r(temp1, ",", &saveptr2);
        j = 0;

        while(Pch2 != NULL)
        {
            char temp2[100], Temp[10];
            strcpy(temp2, Pch2);

            // Split by :
            Pch3 = strtok_r(temp2, ":", &saveptr3);
            l = 0;

            while(Pch3 != NULL)
            {
                strcpy(Temp, Pch3);
                zone[i][j][l] = atoi(Temp);

                Pch3 = strtok_r(NULL, ":", &saveptr3);
                l++;
            }

            Pch2 = strtok_r(NULL, ",", &saveptr2);
            j++;
        }

        Pch1 = strtok_r(NULL, ";", &saveptr1);
        i++;
    }
	return i;
}
// static void wifi_process_function()
void wifi_process_function()
{
	UINT8 format = 0, i = 0, j = 0;
	s8 mobindx = -1;
	s8 rspType = -1;
	unsigned int Tp, Tp1, Tp2, Tp3, len;
	UINT8 smsbuffer[160] = "";
	UINT8 SMSbuffer[160] = "";
	UINT8 smsbufferasitis[160] = "";
	UINT8 *p = EAT_NULL;
	char *Pch1 = EAT_NULL;
	int StrTokStrVer = 0,StrTokStrVer1 = 0;
	int len1;
	char StrTokStr1[40][25]; // 10,20
	format = 1;				 // aj_changed
	// eat_get_sms_format(&format);
	/* sprintf(buf,"eat_sms_read_cb, format=%d",format);
	sAPI_UartPrintf(buf); */
	strcpy(smsbuffer, wifiStrNumber[Noofsettingsprocessed].Wifistr);
	strcpy(SMSbuffer, wifiStrNumber[Noofsettingsprocessed].Wifistr);
	sprintf(buf, "\n\rWIFI BUFR:%s\n\r", smsbuffer);
	sAPI_UartPrintf(buf);
	len1 = strlen(smsbuffer);
	Noofsettingsprocessed++;
	/*if(smsIndex>1)
		eat_delete_sms_all(smsIndex);
	else
		simcom_sms_msg_delete(smsIndex);*/
	if (1 == format) // TEXT mode
	{
		p = SmsNumber[0];
		sAPI_UartPrintf("In IF Con");
		if (!strcmp(SMSbuffer, "FW$"))
		{
			fotaflsg = 1;
			// eat_send_msg_to_user(EAT_USER_1, EAT_USER_2, EAT_FALSE, strlen(smsbuffer), smsbuffer, EAT_NULL);
			sAPI_UartPrintf("fota sms triggered");
		}
		else if (strstr(smsbuffer, "delete") != 0)
		{
			Delete_Settings_files();
		}
		else if (strstr(smsbuffer, "mqtttopic") != 0)
		{
			memset(DeviceConfig.MqttPublishTopic, 0, 25);
			memset(DeviceConfig.MqttServerTopic, 0, 25);
			memset(DeviceConfig.MqttSubscribeTopic, 0, 25);

			Pch1 = strtok((char *)smsbuffer, (char *)",");
			StrTokStrVer = 0;
			while (Pch1 != NULL)
			{
				strcpy(StrTokStr1[StrTokStrVer], Pch1);
				StrTokStrVer++;
				Pch1 = strtok(NULL, ",");
			}

			strcpy((char *)DeviceConfig.MqttPublishTopic, (char *)StrTokStr1[1]);
			strcpy((char *)DeviceConfig.MqttServerTopic, (char *)StrTokStr1[2]);
			strcpy((char *)DeviceConfig.MqttSubscribeTopic, (char *)StrTokStr1[3]);

			/* strcpy((char *)s_mqtt.Publish_Topic,(char *)StrTokStr1[1]);
			strcpy((char *)s_mqtt.Server_Publish_Topic,(char *)StrTokStr1[2]);
			strcpy((char *)s_mqtt.Subscribe_Topic,(char *)StrTokStr1[3]); */

			WriteTopicsetFile();
			ReadTopicsetFile();
			MQTT_Reconnect();
			sprintf(textBuf,"received topic,%s,%s,%s,\n\r",DeviceConfig.MqttPublishTopic,DeviceConfig.MqttServerTopic,DeviceConfig.MqttSubscribeTopic);
			simcom_sms_msg_send(PhoneNumber, textBuf, strlen(textBuf),SmSCallback);
			sAPI_UartPrintf(textBuf);
		}
		for (i = 0; i < 10; i++)
		{
			// sAPI_Debug("recived no %s , Reg number %s",p+j,SmsNumber[i]);
			// j=strlen(StoredPhoneSmscode[i]);
			j = 0;
			sprintf(buf, "recived no %s , Reg number %s,%d", p + j, SmsNumber[i], j);
			sAPI_UartPrintf(buf);
			if (!strncmp(StoredPhoneNumber[i], p + j, strlen(StoredPhoneNumber[i])) && strlen(StoredPhoneNumber[i]) > 0)
			{
				mobindx = i;
				strcpy(PhoneNumber, p + j);
				break;
			}
		}
		// strcpy(PhoneNumber,p+3);
		sprintf(buf, "mobi no:%d, HowManyNumberFound %d", mobindx, HowManyNumberFound);
		sAPI_UartPrintf(buf);
		/* p = smsbuffer;  //dg_nsdk
		for (i = 0; i < len1; i++) // convert to Lcaps
		{
			if (*p >= 'A' && *p <= 'Z')
				*p = (*p + 'a') - 'A';
			p++;
		}*/
		sprintf(buf, "LsmsData %s\n", smsbuffer);
		sAPI_UartPrintf(buf);
		p = SmsNumber[0]; 
		//	memset(UartBuffer,NULL,sizeof(UartBuffer));
		//	memcpy(UartBuffer,smsbuffer,strlen(smsbuffer));
		//	sendtoMcu=1;
		//	sendtoMcuack=0;

		if (strstr(smsbuffer, "sm#8185")) // if_lad start
		{
			UINT8 SMSMnumberStor[25] = "";
			int index = 10;
			char name[10] = "SMSM";
			char at_buf[100];

			// sAPI_UartPrintf("\n\rSMSM Store FN cmd\n\r");
			Pch1 = strtok((char *)smsbuffer, (char *)",");
			StrTokStrVer = 0;
			while (Pch1 != NULL)
			{
				strcpy(StrTokStr1[StrTokStrVer], Pch1);
				StrTokStrVer++;
				Pch1 = strtok(NULL, ",");
			}
			// Tp=smsbuffer[3]-'0';
			// Tp1=smsbuffer[4]-'0';
			// regsmsno=Tp*10+Tp;
			// ReadPhoneNumber();
			strcpy(SMSMnumberStor, StrTokStr1[2]);

			sprintf(buf, "\n\rSMSMnumberStor: %s \n\r", SMSMnumberStor);
			sprintf(buf, "\n\rSMSMnumberStor: %s \n\r", SMSMnumberStor);
			sAPI_UartPrintf(buf);
			sprintf(buf, "\n\rIMEInumber: %s \n\r", StrTokStr1[3]);
			sAPI_UartPrintf(buf);
			sprintf(buf, "\n\rPASSWRD: %s \n\r", DeviceConfig.smsPassword);
			sAPI_UartPrintf(buf);

			sprintf(buf, "\n\rIMEInumber: %s \n\r", IMEI);
			sAPI_UartPrintf(buf);
			//(strstr(DEFAULTNUMBER,p+3)!=0)|| (strstr(DEFAULTNUMBER1,p+3)!=0)|| (strstr(DEFAULTNUMBER2,p+3)!=0)|| (strstr(DEFAULTNUMBER3,p+3)!=0)|| (strstr(DEFAULTNUMBER4,p+3)!=0))
			if (((strstr(DeviceConfig.smsPassword, StrTokStr1[4]) != 0)) || (strstr(DEFAULTNUMBER, p) != 0) || (strstr(DEFAULTNUMBER1, p) != 0) || (strstr(DEFAULTNUMBER2, p) != 0) || (strstr(DEFAULTNUMBER3, p) != 0) || (strstr(DEFAULTNUMBER4, p) != 0))

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
				strcpy((char *)StoredPhoneNumber[22], (char *)StoredPhoneNumber[0]);
				strcpy((char *)StoredPhoneNumber[0], (char *)SMSMnumberStor);
				strcpy((char *)StoredPhoneSmscode[0], (char *)StrTokStr1[1]);

				sprintf(buf, "\n\StoredPhoneNumber: %s \n\r", StoredPhoneNumber[0]);
				sAPI_UartPrintf(buf);
				sprintf(buf, "\n\StoredPhoneSmscode: %s \n\r", StoredPhoneSmscode[0]);
				sAPI_UartPrintf(buf);
				/* sprintf(at_buf, "AT+CPBW=%d,\"%s\",129,\"%s\"\r", index, SMSMnumberStor, name); 
				if (sAPI_AtSend(at_buf, strlen(at_buf)) != 1 )
					sAPI_UartPrintf("\n\rSend AT fail");
				else
					sAPI_UartPrintf("\n\r AT Sent\n\r"); */

				// WritePhoneNumberFn();

				// Readin' the master Number

				//	 SaveNumberPos = 200;
				//	 WritePhoneNumber = 1;
				//	 if(!strlen(StoredPhoneNumber[0])<8 && strcmp(StoredPhoneNumber[0],"0000000000"))

				//	 Write_PH(200,SMSMnumberStor,"SMSM",writepb_cb);
				// ReadPhoneNumber();

		//		SMSMno_Read();
				sprintf(buf, "\n\rAfter Read SMSMno>>:%s\n\r", SMSMno);
				sAPI_UartPrintf(buf);
				// sprintf(buf,"\n\rSMSM Store FN, /%s\n\r",SMSMnumberStor);
				// sAPI_UartPrintf(buf);
				/*SEND TO All*/
				// send_Smsno(PhoneNumber);
				cpbrsearchend1 = 0;

				////eat_send_msg_to_user(EAT_USER_1, EAT_USER_0, EAT_FALSE,8, "NUMSTORA", EAT_NULL);
			}
		}

		/*  else if(strstr(smsbuffer,"pumpconfig") != 0)
		{
			sprintf(buf,"\n\pumpno_tx_chk_into>>:%d s_npump[%d].m_Tank_on_off %d , s_npump[%d].m_Sump_on_off %d,\n\r",pumpno_tx,pumpno_tx,s_npump[pumpno_tx].m_Tank_on_off,pumpno_tx,s_npump[pumpno_tx].m_Sump_on_off);
			sAPI_UartPrintf(buf);
			char StrTokStr2[110][5];
			ReadtnkconfigFile();
			Pch1 = strtok((char *)smsbuffer, (char *)"," );
			StrTokStrVer = 0;
			while( Pch1 != NULL )
			{
			strcpy(StrTokStr2[StrTokStrVer],Pch1);
			StrTokStrVer++;
			Pch1 = strtok( NULL, "," );
			}

			pumpno_tx=myatoi(StrTokStr2[1]);
		s_nMSettings.m_AutoStIIOnOff[pumpno_tx]=myatoi(StrTokStr2[4]);
		s_nTimerSettings.m_POnHr[pumpno_tx]=myatoi(StrTokStr2[5]);
		s_nTimerSettings.m_POnMin[pumpno_tx]=myatoi(StrTokStr2[6]);
		s_nTimerSettings.m_POnSec[pumpno_tx]=myatoi(StrTokStr2[7]);
		s_nTimerSettings.m_SDHr[pumpno_tx]=myatoi(StrTokStr2[8]);
		s_nTimerSettings.m_SDMin[pumpno_tx]=myatoi(StrTokStr2[9]);
		s_nTimerSettings.m_SDSec[pumpno_tx]=myatoi(StrTokStr2[10]);
		s_nMSettings.m_PoScrDlOnOff[pumpno_tx]=myatoi(StrTokStr2[11]);
		s_nTimerSettings.m_PoScrDlHr[pumpno_tx]=myatoi(StrTokStr2[12]);
		s_nTimerSettings.m_PoScrDlMin[pumpno_tx]=myatoi(StrTokStr2[13]);
		s_nTimerSettings.m_PoScrDlSec[pumpno_tx]=myatoi(StrTokStr2[14]);
		s_nMSettings.m_SfbOnOff[pumpno_tx]=myatoi(StrTokStr2[15]);
		s_nTimerSettings.m_SfbHr[pumpno_tx]=myatoi(StrTokStr2[16]);
		s_nTimerSettings.m_SfbMin[pumpno_tx]=myatoi(StrTokStr2[17]);
		s_nTimerSettings.m_SfbSec[pumpno_tx]=myatoi(StrTokStr2[18]);
		s_nMSettings.m_MaxRnOnOf[pumpno_tx]=myatoi(StrTokStr2[19]);
		s_nTimerSettings.m_MaxRnHr[pumpno_tx]=myatoi(StrTokStr2[20]);
		s_nTimerSettings.m_MaxRnMin[pumpno_tx]=myatoi(StrTokStr2[21]);
		s_nTimerSettings.m_MaxRnSec[pumpno_tx]=myatoi(StrTokStr2[22]);
		s_nMSettings.m_CycLicOnOf[pumpno_tx]=myatoi(StrTokStr2[23]);
		s_nTimerSettings.m_CycLicOnHr[pumpno_tx]=myatoi(StrTokStr2[24]);
		s_nTimerSettings.m_CycLicOnMin[pumpno_tx]=myatoi(StrTokStr2[25]);
		s_nTimerSettings.m_CycLicOnSec[pumpno_tx]=myatoi(StrTokStr2[26]);
		s_nTimerSettings.m_CycLicOfHr[pumpno_tx]=myatoi(StrTokStr2[27]);
		s_nTimerSettings.m_CycLicOfMin[pumpno_tx]=myatoi(StrTokStr2[28]);
		s_nTimerSettings.m_CycLicOfSec[pumpno_tx]=myatoi(StrTokStr2[29]);
		s_nMSettings.m_RTCOnOf[pumpno_tx]=myatoi(StrTokStr2[30]);
		s_nTimerSettings.m_RTCONHr[pumpno_tx][1]=myatoi(StrTokStr2[31]);
		s_nTimerSettings.m_RTCONMin[pumpno_tx][1]=myatoi(StrTokStr2[32]);
		s_nTimerSettings.m_RTCONSec[pumpno_tx][1]=myatoi(StrTokStr2[33]);
		 s_nTimerSettings.m_RTCOfHr[pumpno_tx][1]=myatoi(StrTokStr2[34]);
		 s_nTimerSettings.m_RTCOfMin[pumpno_tx][1]=myatoi(StrTokStr2[35]);
		 s_nTimerSettings.m_RTCOfSec[pumpno_tx][1]=myatoi(StrTokStr2[36]);
		 s_nTimerSettings.m_RTCONHr[pumpno_tx][2]=myatoi(StrTokStr2[37]);
		 s_nTimerSettings.m_RTCONMin[pumpno_tx][2]=myatoi(StrTokStr2[38]);
		 s_nTimerSettings.m_RTCONSec[pumpno_tx][2]=myatoi(StrTokStr2[39]);
		 s_nTimerSettings.m_RTCOfHr[pumpno_tx][2]=myatoi(StrTokStr2[40]);
		 s_nTimerSettings.m_RTCOfMin[pumpno_tx][2]=myatoi(StrTokStr2[41]);
		 s_nTimerSettings.m_RTCOfSec[pumpno_tx][2]=myatoi(StrTokStr2[42]);
		 s_nTimerSettings.m_RTCONHr[pumpno_tx][3]=myatoi(StrTokStr2[43]);
		 s_nTimerSettings.m_RTCONMin[pumpno_tx][3]=myatoi(StrTokStr2[44]);
		 s_nTimerSettings.m_RTCONSec[pumpno_tx][3]=myatoi(StrTokStr2[45]);
		 s_nTimerSettings.m_RTCOfHr[pumpno_tx][3]=myatoi(StrTokStr2[46]);
		 s_nTimerSettings.m_RTCOfMin[pumpno_tx][3]=myatoi(StrTokStr2[47]);
		 s_nTimerSettings.m_RTCOfSec[pumpno_tx][3]=myatoi(StrTokStr2[48]);
		s_nTimerSettings.m_RTCONHr[pumpno_tx][4]=myatoi(StrTokStr2[49]);
		s_nTimerSettings.m_RTCONMin[pumpno_tx][4]=myatoi(StrTokStr2[50]);
		s_nTimerSettings.m_RTCONSec[pumpno_tx][4]=myatoi(StrTokStr2[51]);
		 s_nTimerSettings.m_RTCOfHr[pumpno_tx][4]=myatoi(StrTokStr2[52]);
		 s_nTimerSettings.m_RTCOfMin[pumpno_tx][4]=myatoi(StrTokStr2[53]);
		 s_nTimerSettings.m_RTCOfSec[pumpno_tx][4]=myatoi(StrTokStr2[54]);
		 s_nTimerSettings.m_RTCONHr[pumpno_tx][5]=myatoi(StrTokStr2[55]);
		 s_nTimerSettings.m_RTCONMin[pumpno_tx][5]=myatoi(StrTokStr2[56]);
		 s_nTimerSettings.m_RTCONSec[pumpno_tx][5]=myatoi(StrTokStr2[57]);
		 s_nTimerSettings.m_RTCOfHr[pumpno_tx][5]=myatoi(StrTokStr2[58]);
		 s_nTimerSettings.m_RTCOfMin[pumpno_tx][5]=myatoi(StrTokStr2[59]);
		 s_nTimerSettings.m_RTCOfSec[pumpno_tx][5]=myatoi(StrTokStr2[60]);
		 s_nTimerSettings.m_RTCONHr[pumpno_tx][6]=myatoi(StrTokStr2[61]);
		 s_nTimerSettings.m_RTCONMin[pumpno_tx][6]=myatoi(StrTokStr2[62]);
		 s_nTimerSettings.m_RTCONSec[pumpno_tx][6]=myatoi(StrTokStr2[63]);
		 s_nTimerSettings.m_RTCOfHr[pumpno_tx][6]=myatoi(StrTokStr2[64]);
		 s_nTimerSettings.m_RTCOfMin[pumpno_tx][6]=myatoi(StrTokStr2[65]);
		 s_nTimerSettings.m_RTCOfSec[pumpno_tx][6]=myatoi(StrTokStr2[66]);
		 s_nMSettings.m_DrScOnOf[pumpno_tx]=myatoi(StrTokStr2[67]);
		 s_nTimerSettings.m_DrScHr[pumpno_tx]=myatoi(StrTokStr2[68]);
			s_nTimerSettings.m_DrScMin[pumpno_tx]=myatoi(StrTokStr2[69]);
			s_nTimerSettings.m_DrScSec[pumpno_tx]=myatoi(StrTokStr2[70]);
		s_nTimerSettings.m_DrAmpsII[pumpno_tx]=myatof(StrTokStr2[71]);
		s_nTimerSettings.m_DrAmpsIII[pumpno_tx]=myatof(StrTokStr2[72]);
		s_nMSettings.m_DrReOnOf[pumpno_tx]=myatoi(StrTokStr2[73]);
		s_nTimerSettings.m_DrReHr[pumpno_tx]=myatoi(StrTokStr2[74]);
		s_nTimerSettings.m_DrReMin[pumpno_tx]=myatoi(StrTokStr2[75]);
		s_nTimerSettings.m_DrReSec[pumpno_tx]=myatoi(StrTokStr2[76]);
		s_nMSettings.m_DrOccurOnOff[pumpno_tx]=myatoi(StrTokStr2[77]);
		s_nTimerSettings.m_DrOccurTimHr[pumpno_tx]=myatoi(StrTokStr2[78]);
		s_nTimerSettings.m_DrOccurTimMin[pumpno_tx]=myatoi(StrTokStr2[79]);
		s_nTimerSettings.m_DrOccurTimSec[pumpno_tx]=myatoi(StrTokStr2[80]);
		a_occurance_count[pumpno_tx]=myatoi(StrTokStr2[81]);
		s_nMSettings.m_OlOnOff[pumpno_tx]=myatoi(StrTokStr2[82]);
		 s_nTimerSettings.m_OlScanHr[pumpno_tx]=myatoi(StrTokStr2[83]);
			 s_nTimerSettings.m_OlScanMin[pumpno_tx]=myatoi(StrTokStr2[84]);
			 s_nTimerSettings.m_OlScanSec[pumpno_tx]=myatoi(StrTokStr2[85]);
		s_nTimerSettings.m_OlAmpsII[pumpno_tx]=myatof(StrTokStr2[86]);
		s_nTimerSettings.m_OlAmpsIII[pumpno_tx]=myatof(StrTokStr2[87]);
		s_nMSettings.m_CTRonoff=myatoi(StrTokStr2[88]);
		s_nMSettings.m_CTYonoff=myatoi(StrTokStr2[89]);
		s_nMSettings.m_CTBonoff=myatoi(StrTokStr2[90]);
		s_nMSettings.m_LowVoltOnOff=myatoi(StrTokStr2[91]);
		s_nTimerSettings.m_LowVoltIII=myatoi(StrTokStr2[92]);
		s_nTimerSettings.m_DiffVoltIII=myatoi(StrTokStr2[93]);
		s_nTimerSettings.m_LowVoltII=myatoi(StrTokStr2[94]);
		s_nTimerSettings.m_DiffVoltII=myatoi(StrTokStr2[95]);
		s_nMSettings.m_HighVoltOnOff=myatoi(StrTokStr2[96]);
		s_nTimerSettings.m_HighVoltIII=myatoi(StrTokStr2[97]);
		s_nTimerSettings.m_HiDiffVoltIII=myatoi(StrTokStr2[98]);
		s_nTimerSettings.m_HighVoltII=myatoi(StrTokStr2[99]);
		s_nTimerSettings.m_HiDiffVoltII=myatoi(StrTokStr2[100]);
		s_nTimerSettings.m_ImbVolt=myatoi(StrTokStr2[101]);
		s_nMSettings.m_RvePhOnoff=myatoi(StrTokStr2[102]);
		s_nMSettings.m_SppOnoff=myatoi(StrTokStr2[103]);
		s_npump[pumpno_tx].m_Tank_on_off=myatoi(StrTokStr2[104]);
		s_npump[pumpno_tx].m_Sump_on_off=myatoi(StrTokStr2[105]);

			sprintf(buf,"\n\pumpno_tx_chk>>:%d s_npump[%d].m_Tank_on_off %d , s_npump[%d].m_Sump_on_off %d,\n\r",pumpno_tx,pumpno_tx,s_npump[pumpno_tx].m_Tank_on_off,pumpno_tx,s_npump[pumpno_tx].m_Sump_on_off);
			sAPI_UartPrintf(buf);

			WritetnkconfigFile();
			ReadtnkconfigFile();
		//	s_nMSettings.m_settings_req_flag=1;
		//	s_nMSettings.m_settings_count=1;
			send_pumpconfiguaration(PhoneNumber);
		} */
		else if(strstr(smsbuffer,"runprogram") != 0)
		{
			Pch1 = strtok((char *)smsbuffer, (char *)"," );
			StrTokStrVer = 0;
			while( Pch1 != NULL )
			{
			strcpy(StrTokStr1[StrTokStrVer],Pch1);
			StrTokStrVer++;
			Pch1 = strtok( NULL, "," );
			}

			nProgramProcess.Sno = myatoi(StrTokStr1[1]);
			nProgramProcess.ProgramEnable=myatoi(StrTokStr1[2]);

		}
		// else if(strstr(smsbuffer,"Program2") != 0)
		// {
		// 	Pch1 = strtok((char *)smsbuffer, (char *)"," );
		// 	StrTokStrVer = 0;
		// 	while( Pch1 != NULL )
		// 	{
		// 	strcpy(StrTokStr1[StrTokStrVer],Pch1);
		// 	StrTokStrVer++;
		// 	Pch1 = strtok( NULL, "," );
		// 	}
		// 	if(ProgramFlag_1==0)
		// 	ProgramFlag_2=atoi(StrTokStr1[1]);
		// }
		else if(strstr(smsbuffer,"config") != 0)
		{
			sAPI_UartPrintf("In Config Maker\n");
			memset(Buffer1,NULL,sizeof(Buffer1));
			//ReadtnkconfigFile();
			Pch1 = strtok((char *)smsbuffer, (char *)";" );
			StrTokStrVer = 0;
			while( Pch1 != NULL )
			{
			strcpy(StrTokStr1[StrTokStrVer],Pch1);
			StrTokStrVer++;
			Pch1 = strtok( NULL, ";" );
			}
			for(i=1;i<StrTokStrVer;i++)
			{
				Pch1 = strtok((char *)StrTokStr1[i], (char *)"," );
				StrTokStrVer1 = 0;
				while( Pch1 != NULL )
				{
				strcpy(StrTokStr2[StrTokStrVer1],Pch1);
				StrTokStrVer1++;
				Pch1 = strtok( NULL, "," );
				}
				
					s_nOMSfeedback[i-1].Object=nConfig[i-1].Object=myatoi(StrTokStr2[0]);
					s_nOMSfeedback[i-1].Sno=nConfig[i-1].Sno=myatoi(StrTokStr2[1]);
					s_nOMSfeedback[i-1].Input_No=nConfig[i-1].Input_No=myatoi(StrTokStr2[2]);
					s_nOMSfeedback[i-1].Output_No=nConfig[i-1].Output_No=myatoi(StrTokStr2[3]);

			}
			NoOfObject=i-1;
			sprintf(buf,"Before Set Config NoOfObject=%d\n\r",NoOfObject);
			sAPI_UartPrintf(buf);
			for(i=0;i<NoOfObject;i++)
			{
				sprintf(Buffer1,"Before Set Config:%d,%d,%d,%d,\n\r",nConfig[i].Sno,nConfig[i].Object,nConfig[i].Input_No,nConfig[i].Output_No);
				sAPI_UartPrintf(Buffer1);
			}
			WritetnkconfigFile();
			ReadtnkconfigFile();
			sprintf(buf,"After Set Config NoOfObject=%d\n\r",NoOfObject);
			sAPI_UartPrintf(buf);
			for(i=0;i<NoOfObject;i++)
			{
				sprintf(Buffer1,"After Set Config:%d,%d,%d,%d,\n\r",nConfig[i].Sno,nConfig[i].Object,nConfig[i].Input_No,nConfig[i].Output_No);
				sAPI_UartPrintf(Buffer1);
			}


			send_pumpconfiguaration(PhoneNumber);
			
		}
		else if(strstr(smsbuffer,"constant") != 0)
		{
			memset(Buffer1,NULL,sizeof(Buffer1));
			Pch1 = strtok((char *)smsbuffer, (char *)";" );
			StrTokStrVer = 0;
			while( Pch1 != NULL )
			{
			strcpy(StrTokStr1[StrTokStrVer],Pch1);
			StrTokStrVer++;
			Pch1 = strtok( NULL, ";" );
			}
			for(i=1;i<StrTokStrVer;i++)
			{
				Pch1 = strtok((char *)StrTokStr1[i], (char *)"," );
				StrTokStrVer1 = 0;
				while( Pch1 != NULL )
				{
				strcpy(StrTokStr2[StrTokStrVer1],Pch1);
				StrTokStrVer1++;
				Pch1 = strtok( NULL, "," );
				}
				
					
					nConstant[i-1].Object=myatoi(StrTokStr2[0]);
					nConstant[i-1].Sno=myatoi(StrTokStr2[1]);
				if(nConstant[i-1].Object==26)
				{
					nConstant[i-1].FlowRate=0;
					nConstant[i-1].LiterPerPluse=myatof(StrTokStr2[2]);
					nConstant[i-1].Pressure=0;
				}
				else if(nConstant[i-1].Object==13 || nConstant[i-1].Object==45)
				{
					sprintf(Buff,"FLow :%s",StrTokStr2[2]);
					sAPI_UartPrintf(Buff);
					nConstant[i-1].FlowRate=myatof(StrTokStr2[2]);
					nConstant[i-1].LiterPerPluse=0;
					nConstant[i-1].Pressure=0;
				}
				else
				{
					nConstant[i-1].FlowRate=0;
					nConstant[i-1].LiterPerPluse=0;
					nConstant[i-1].Pressure=myatof(StrTokStr2[2]);
				}
			}
			NoOfConstant=i-1;
			for(i=0;i<NoOfConstant;i++)
			{
				sprintf(Buffer1,"Constant before write:%d,%d,%f,%f,%f,\n\r",nConstant[i].Sno,nConstant[i].Object,nConstant[i].FlowRate,nConstant[i].LiterPerPluse,nConstant[i].Pressure);
				sAPI_UartPrintf(Buffer1);
			}
			WriteEcosetFile();
			ReadEcosetFile();
			NoOfConstant;
			for(i=0;i<NoOfConstant;i++)
			{
				sprintf(Buffer1,"Constant after read:%d,%d,%f,%f,%f,\n\r",nConstant[i].Sno,nConstant[i].Object,nConstant[i].FlowRate,nConstant[i].LiterPerPluse,nConstant[i].Pressure);
				sAPI_UartPrintf(Buffer1);
			}
			send_tankconfiguaration(PhoneNumber);
			
		}
		else if(strstr(smsbuffer,"zone") != 0)
		{
			NoOfZone=Data_split_Function(smsbuffer);
			sprintf(buf,"NoOfZone=%d\n\r",NoOfZone);
			sAPI_UartPrintf(buf);
			NoOfZone-=1;
			sprintf(buf,"NoOfZone=%d\n\r",NoOfZone);
			sAPI_UartPrintf(buf);
			for(i=0;i<NoOfZone;i++)
			{
				nProgram.nZone[i].Sno=zone[i+1][0][0];
				nProgram.nZone[i].ValveNo[0]=zone[i+1][1][0];
				nProgram.nZone[i].ValveNo[1]=zone[i+1][1][1];
				nProgram.nZone[i].ValveNo[2]=zone[i+1][1][2];
				nProgram.nZone[i].ValveNo[3]=zone[i+1][1][3];
				nProgram.nZone[i].MainValve=zone[i+1][2][0];
				nProgram.nZone[i].IrrigationMethod=zone[i+1][3][0];
				if(nProgram.nZone[i].IrrigationMethod==1)
				{
					nProgram.nZone[i].FlowRate=zone[i+1][4][0];
					nProgram.nZone[i].Duration[0]=0;
					nProgram.nZone[i].Duration[1]=0;
					nProgram.nZone[i].Duration[2]=0;
				}
				else
				{
					nProgram.nZone[i].Duration[0]=zone[i+1][5][0];
					nProgram.nZone[i].Duration[1]=zone[i+1][5][1];
					nProgram.nZone[i].Duration[2]=zone[i+1][5][2];
					nProgram.nZone[i].FlowRate=0;
				}
				nProgram.nZone[i].ProgramNo=zone[i+1][6][0];
				sprintf(Buffer1,"Zone[%d]:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n\r",i+1,nProgram.nZone[i].Sno,nProgram.nZone[i].ValveNo[0],nProgram.nZone[i].ValveNo[1],nProgram.nZone[i].ValveNo[2],nProgram.nZone[i].ValveNo[3],nProgram.nZone[i].MainValve,nProgram.nZone[i].IrrigationMethod,nProgram.nZone[i].FlowRate,nProgram.nZone[i].Duration[0],nProgram.nZone[i].Duration[1],nProgram.nZone[i].Duration[2],nProgram.nZone[i].ProgramNo);
				sAPI_UartPrintf(Buffer1);
			}
			WritevolsetFile();
			ReadvolsetFile();
			
			// memset(Buffer1,NULL,sizeof(Buffer1));
			// ReadtnkconfigFile();
			// Pch1 = strtok((char *)smsbuffer, (char *)";" );
			// StrTokStrVer = 0;
			// while( Pch1 != NULL )
			// {
			// strcpy(StrTokStr1[StrTokStrVer],Pch1);
			// StrTokStrVer++;
			// Pch1 = strtok( NULL, ";" );
			// }
			// for(i=1;i<StrTokStrVer;i++)
			// {
			// 	Pch1 = strtok((char *)StrTokStr1[i], (char *)"," );
			// 	StrTokStrVer1 = 0;
			// 	while( Pch1 != NULL )
			// 	{
			// 	strcpy(StrTokStr2[StrTokStrVer1],Pch1);
			// 	StrTokStrVer1++;
			// 	Pch1 = strtok( NULL, "," );
			// 	}
				
			// 		nConstant[i-1].Sno=myatoi(StrTokStr2[0]);
			// 		nConstant[i-1].Object=myatoi(StrTokStr2[1]);
			// 		nConstant[i-1].FlowRate=myatoi(StrTokStr2[2]);
			// }
			// NoOfObject=i-1;
			// for(i=0;i<NoOfObject;i++)
			// {
			// 	sprintf(Buffer1,"Constant:%d,%d,%d,\n\r",nConstant[i].Sno,nConstant[i].Object,nConstant[i].FlowRate);
			// }
			send_scheduleconfiguaration(PhoneNumber);
			
		}
		else if(strstr(smsbuffer,"program") != 0)
		{
			int k=0,sno=0;
			float programBuf[50];
			memset(Buffer1,NULL,sizeof(Buffer1));
			ReadtnkconfigFile();
			Pch1 = strtok((char *)smsbuffer, (char *)";" );
			StrTokStrVer = 0;
			while( Pch1 != NULL )
			{
			strcpy(StrTokStr1[StrTokStrVer],Pch1);
			StrTokStrVer++;
			Pch1 = strtok( NULL, ";" );
			}
			for(i=1;i<StrTokStrVer;i++)
			{
				Pch1 = strtok((char *)StrTokStr1[i], (char *)"," );
				StrTokStrVer1 = 0;
				while( Pch1 != NULL )
				{
				strcpy(StrTokStr2[StrTokStrVer1],Pch1);
				StrTokStrVer1++;
				Pch1 = strtok( NULL, "," );
				}
				for(j=0;j<StrTokStrVer1;j++)
				{
					programBuf[k++]=myatof(StrTokStr2[j]);
				}
				
			}
			sno=0;
			i=0;
			//sno=programBuf[i]-1;
			sno=i;
			nProgram.Sno=1;
			nProgram.DelayBtwZone[0]=(int)programBuf[++i];
			nProgram.DelayBtwZone[1]=(int)programBuf[++i];
			nProgram.DelayBtwZone[2]=(int)programBuf[++i];
			nProgram.ScaleFact=programBuf[++i];
			nProgram.Schedule=(int)programBuf[++i];
			nProgram.StartDate[0]=(int)programBuf[++i];
			nProgram.StartDate[1]=(int)programBuf[++i];
			nProgram.StartDate[2]=(int)programBuf[++i];
			nProgram.DayCount=(int)programBuf[++i];
			nProgram.EndDate[0]=(int)programBuf[++i];
			nProgram.EndDate[1]=(int)programBuf[++i];
			nProgram.EndDate[2]=(int)programBuf[++i];
			nProgram.Rtc[0]=(int)programBuf[++i];
			nProgram.Rtc[1]=(int)programBuf[++i];
			nProgram.Rtc[2]=(int)programBuf[++i];
			nProgram.Alaram=(int)programBuf[++i];
			nProgram.ProgramNo=sno;
			for(j=0,i=0;i<StrTokStrVer;j++)
			{
				nProgram.SelectedDate[j]=programBuf[i++];
			}
            sprintf(Buffer1,"Program,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n\r",nProgram.Sno,nProgram.DelayBtwZone[0],nProgram.DelayBtwZone[1],nProgram.DelayBtwZone[2],nProgram.ScaleFact,nProgram.Schedule,nProgram.StartDate[0],nProgram.StartDate[1],nProgram.StartDate[2],nProgram.DayCount,nProgram.EndDate[0],nProgram.EndDate[1],nProgram.EndDate[2],nProgram.Rtc[0],nProgram.Rtc[1],nProgram.Rtc[2],nProgram.Alaram,nProgram.ProgramNo);
            sAPI_UartPrintf(Buffer1);
			// sno=0;
			// i=1;
			// sno=myatoi(StrTokStr1[i]);
			// nProgram[sno].Sno=sno;
			// nProgram[sno].DelayBtwZone[0]=myatoi(StrTokStr1[++i]);
			// nProgram[sno].DelayBtwZone[1]=myatoi(StrTokStr1[++i]);
			// nProgram[sno].DelayBtwZone[2]=myatoi(StrTokStr1[++i]);
			// nProgram[sno].ScaleFact=myatoi(StrTokStr1[++i]);
			// nProgram[sno].Schedule=myatoi(StrTokStr1[++i]);
			// nProgram[sno].StartDate[0]=myatoi(StrTokStr1[++i]);
			// nProgram[sno].StartDate[1]=myatoi(StrTokStr1[++i]);
			// nProgram[sno].StartDate[2]=myatoi(StrTokStr1[++i]);
			// nProgram[sno].DayCount=myatoi(StrTokStr1[++i]);
			// nProgram[sno].EndDate[0]=myatoi(StrTokStr1[++i]);
			// nProgram[sno].EndDate[1]=myatoi(StrTokStr1[++i]);
			// nProgram[sno].EndDate[2]=myatoi(StrTokStr1[++i]);
			// nProgram[sno].Rtc[0]=myatoi(StrTokStr1[++i]);
			// nProgram[sno].Rtc[1]=myatoi(StrTokStr1[++i]);
			// nProgram[sno].Rtc[2]=myatoi(StrTokStr1[++i]);
			// nProgram[sno].Alaram=myatoi(StrTokStr1[++i]);
			//nProgram[sno].ProgramNo=myatoi(StrTokStr1[++i]);
			// for(j=0;i<StrTokStrVer;j++)
			// {
			// 	nProgram[sno].SelectedDate[j]=myatoi(StrTokStr1[++i]);
			// }
			
			
			for(i=0;i<NoOfObject;i++)
			{
				sprintf(Buffer1,"Constant:%d,%d,%d,\n\r",nConstant[i].Sno,nConstant[i].Object,nConstant[i].FlowRate);
			}
			send_voltageconfiguaration(PhoneNumber);
			WritecursetFile(1);
			ReadcursetFile(1);
			
		}
			else if(strstr(smsbuffer,"moisturesetting") != 0)
		{
			memset(Buffer1,NULL,sizeof(Buffer1));
			ReadtnkconfigFile();
			Pch1 = strtok((char *)smsbuffer, (char *)"," );
			StrTokStrVer = 0;
			while( Pch1 != NULL )
			{
			strcpy(StrTokStr1[StrTokStrVer],Pch1);
			StrTokStrVer++;
			Pch1 = strtok( NULL, "," );
			}
			MosConOnOff=myatoi(StrTokStr1[1]);
			MosSelection=myatoi(StrTokStr1[2]);
			PrimMin1=myatoi(StrTokStr1[3]);
			PrimMax1=myatoi(StrTokStr1[4]);
			PrimMin2=myatoi(StrTokStr1[5]);
			PrimMax2=myatoi(StrTokStr1[6]);
			sprintf(Buffer1,"$S,M,%01d,%01d,%03d,%03d,%03d,%03d,\n\r",MosConOnOff,MosSelection,PrimMin1,PrimMax1,PrimMin2,PrimMax2);
			sAPI_UartWrite(SC_UART,(UINT8*)Buffer1,strlen(Buffer1));
			sprintf(buf,"%s",Buffer1);
			sAPI_UartPrintf(buf);
			send_Mosconfiguaration(PhoneNumber);
		}
		else if(strstr(smsbuffer,"frequency") != 0)
		{
			memset(Buffer1,NULL,sizeof(Buffer1));
			ReadtnkconfigFile();
			Pch1 = strtok((char *)smsbuffer, (char *)"," );
			StrTokStrVer = 0;
			while( Pch1 != NULL )
			{
			strcpy(StrTokStr1[StrTokStrVer],Pch1);
			StrTokStrVer++;
			Pch1 = strtok( NULL, "," );
			}
			frequency=myatoi(StrTokStr1[1]);
			ValUp[0]=myatoi(StrTokStr1[2]);
			
			sprintf(Buffer1,"$S,F,%04d,%02d,\n\r",frequency,ValUp[0]);
			sAPI_UartWrite(SC_UART,(UINT8*)Buffer1,strlen(Buffer1));
			sprintf(buf,"%s",Buffer1);
			sAPI_UartPrintf(buf);
		}
		else if(strstr(smsbuffer,"lorakey") != 0)
		{
			memset(Buffer1,NULL,sizeof(Buffer1));
			ReadtnkconfigFile();
			Pch1 = strtok((char *)smsbuffer, (char *)"," );
			StrTokStrVer = 0;
			while( Pch1 != NULL )
			{
			strcpy(StrTokStr1[StrTokStrVer],Pch1);
			StrTokStrVer++;
			Pch1 = strtok( NULL, "," );
			}
			ValUp[0]=myatoi(StrTokStr1[1]);
			
			sprintf(Buffer1,"$S,K,%003d,\n\r",ValUp[0]);
			sAPI_UartWrite(SC_UART,(UINT8*)Buffer1,strlen(Buffer1));
			sprintf(buf,"%s",Buffer1);
			sAPI_UartPrintf(buf);
		}
		
		else if(strstr(smsbuffer,"setserial") != 0)
		{
			memset(Buffer1,NULL,sizeof(Buffer1));
			ReadtnkconfigFile();
			Pch1 = strtok((char *)smsbuffer, (char *)"," );
			StrTokStrVer = 0;
			while( Pch1 != NULL )
			{
			strcpy(StrTokStr1[StrTokStrVer],Pch1);
			StrTokStrVer++;
			Pch1 = strtok( NULL, "," );
			}
			ValUp[0]=myatoi(StrTokStr1[1]);
			
			sprintf(Buffer1,"$S,N,%01d,\n\r",ValUp[0]);
			sAPI_UartWrite(SC_UART,(UINT8*)Buffer1,strlen(Buffer1));
			sprintf(buf,"%s",Buffer1);
			sAPI_UartPrintf(buf);
			if(ValUp[0]==1)
			SetSerial[0]=0;
			if(ValUp[0]==2)
			SetSerial[1]=0;
			if(ValUp[0]==3)
			{
				SetSerial[0]=0;
				SetSerial[1]=0;
			}
			RecSetSerial[0]=0;
			RecSetSerial[1]=0;
			livedataflag=1;
			
		}
		else if(strstr(smsbuffer,"lighton") != 0)
		{
			memset(Buffer1,NULL,sizeof(Buffer1));
			sprintf(Buffer1,"$S,Z,1");
			sAPI_UartWrite(SC_UART,(UINT8*)Buffer1,strlen(Buffer1));
		}
		else if(strstr(smsbuffer,"lightoff") != 0)
		{
			memset(Buffer1,NULL,sizeof(Buffer1));
			sprintf(Buffer1,"$S,Z,0");
			sAPI_UartWrite(SC_UART,(UINT8*)Buffer1,strlen(Buffer1));
		}
		 else if(strstr(smsbuffer,"standalone") != 0)
		{
			memset(Buffer1,NULL,sizeof(Buffer1));
			ReadtnkconfigFile();
			Pch1 = strtok((char *)smsbuffer, (char *)"," );
			StrTokStrVer = 0;
			while( Pch1 != NULL )
			{
			strcpy(StrTokStr1[StrTokStrVer],Pch1);
			StrTokStrVer++;
			Pch1 = strtok( NULL, "," );
			}
			
			ValUp[0]=myatoi(StrTokStr1[1]);
			ValUp[1]=myatoi(StrTokStr1[2]);
			ValUp[2]=myatoi(StrTokStr1[3]);
			ValUp[3]=myatoi(StrTokStr1[4]);
			ValUp[4]=myatoi(StrTokStr1[5]);
			ValUp[5]=myatoi(StrTokStr1[6]);
			ValUp[6]=myatoi(StrTokStr1[7]);
			ValUp[7]=myatoi(StrTokStr1[8]);
			ValUp[8]=myatoi(StrTokStr1[9]);
			ValUp[9]=myatoi(StrTokStr1[10]);
			ValUp[10]=myatoi(StrTokStr1[11]);
			
			sprintf(Buffer1,"$S,A,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,%01d,\n\r",ValUp[0],ValUp[1],ValUp[2],ValUp[3],ValUp[4],ValUp[5],ValUp[6],ValUp[7],ValUp[8],ValUp[9],ValUp[10]);
			sAPI_UartWrite(SC_UART,(UINT8*)Buffer1,strlen(Buffer1));
			sprintf(buf,"%s",Buffer1);
			sAPI_UartPrintf(buf);
			send_Valveconfiguaration(PhoneNumber);	
		}
		 else if(strstr(smsbuffer,"resetcycle") != 0)
		{
			memset(Buffer1,NULL,sizeof(Buffer1));
			sprintf(Buffer1,"$RSC,1,\n\r");
			sAPI_UartWrite(SC_UART,(UINT8*)Buffer1,strlen(Buffer1));
			sprintf(buf,"%s",Buffer1);
			sAPI_UartPrintf(buf);
		}
		 else if(strstr(smsbuffer,"ecoconfig") != 0)
		{
			memset(Buffer1,NULL,sizeof(Buffer1));
			memset(Buffer1,NULL,sizeof(Buffer2));
			memset(Buffer1,NULL,sizeof(Buffer3));
			ReadtnkconfigFile();
			Pch1 = strtok((char *)smsbuffer, (char *)"," );
			StrTokStrVer = 0;
			while( Pch1 != NULL )
			{
			strcpy(StrTokStr1[StrTokStrVer],Pch1);
			StrTokStrVer++;
			Pch1 = strtok( NULL, "," );
			}
			s_npump[0].m_no_of_sump_pins=0;
			s_npump[0].m_sump_pin_no[0]=0;
			s_npump[0].m_sump_pin_no[1]=0;
			s_npump[0].m_no_of_tank_pins=0;
			s_npump[0].m_tank_pin_no[0]=0;
			s_npump[0].m_tank_pin_no[1]=0;
			s_npump[0].m_Level_on_off=0;
			s_npump[0].m_flowonof=0;
			s_npump[0].m_pressureonof=0;
			s_npump[0].m_Tank_on_off=0;
			s_npump[0].m_Sump_on_off=0;
			s_npump[0].m_scheduleonof=0;
			s_npump[0].m_rundays=0;
			s_npump[0].m_skipdays=0;
			s_npump[0].m_Uppertank_restart_off=0;
			s_npump[0].m_Lowertank_restart_off=0;
			
			g_no_of_pumps=myatoi(StrTokStr1[1]);
			NoOfValveConfig=myatoi(StrTokStr1[2]);
			LightConfig=myatoi(StrTokStr1[3]);
			s_npump[0].m_pressureonof=PresSenConfig=myatoi(StrTokStr1[4]);
			PreSwConfig=myatoi(StrTokStr1[5]);
			NoOfMosConfig=myatoi(StrTokStr1[6]);
			NoSoilTemp=myatoi(StrTokStr1[7]);
			if(NoOfMosConfig>0)
			{
				SerialNo[1]=myatoi(StrTokStr1[8]);
				NodeType[1]=myatoi(StrTokStr1[9]);
				RefNo[1]=myatoi(StrTokStr1[10]);
				strcpy(DeviceId[1],StrTokStr1[11]);
				InterfaceType[1]=myatoi(StrTokStr1[12]);
				memset(Buffer2,NULL,sizeof(Buffer2));
				sprintf(Buffer2,"%03d,%02d,%02d,%s,%01d,",SerialNo[1],NodeType[1],RefNo[1],DeviceId[1],InterfaceType[1]);
			}
			memset(Buffer3,NULL,sizeof(Buffer3));
			sprintf(Buffer3,"$E,C,%01d,%02d,%01d,%01d,%01d,%02d,%02d,%s\n\r",g_no_of_pumps,NoOfValveConfig,LightConfig,PresSenConfig,PreSwConfig,NoOfMosConfig,NoSoilTemp,Buffer2);
			sAPI_UartWrite(SC_UART,(UINT8*)Buffer3,strlen(Buffer3));
			sprintf(buf,"%s",Buffer3);
			sAPI_UartPrintf(buf);
			send_Ecoconfiguaration(PhoneNumber);
			WritetnkconfigFile();
			ReadtnkconfigFile();
		}
		
		 else if(strstr(smsbuffer,"changeto") != 0)
		{
			Pch1 = strtok((char *)smsbuffer, (char *)"," );
			StrTokStrVer = 0;
			while( Pch1 != NULL )
			{
			strcpy(StrTokStr1[StrTokStrVer],Pch1);
			StrTokStrVer++;
			Pch1 = strtok( NULL, "," );
			}
			ValveChange=myatoi(StrTokStr1[1]);
			memset(Buffer1,NULL,sizeof(Buffer1));
			sprintf(Buffer1,"$C,T,%02d,\n\r",ValveChange);
			sAPI_UartWrite(SC_UART,(UINT8*)Buffer1,strlen(Buffer1));
			sprintf(buf,"%s",Buffer1);
			sAPI_UartPrintf(buf);
		}

		else if (strstr(smsbuffer, "pumpconfig") != 0)
		{
			ReadtnkconfigFile();
			Pch1 = strtok((char *)smsbuffer, (char *)",");
			StrTokStrVer = 0;
			while (Pch1 != NULL)
			{
				strcpy(StrTokStr1[StrTokStrVer], Pch1);
				StrTokStrVer++;
				Pch1 = strtok(NULL, ",");
			}

			g_no_of_pumps = myatoi(StrTokStr1[1]);
			g_serialid = myatoi(StrTokStr1[2]);
			a_Master_onoff[0] = myatoi(StrTokStr1[3]);
			a_Master_onoff[1] = myatoi(StrTokStr1[4]);
			a_Master_onoff[2] = myatoi(StrTokStr1[5]);
			WritetnkconfigFile();
			ReadtnkconfigFile();
			s_nMSettings.m_settings_req_flag = 1;
			s_nMSettings.m_settings_count = 1;
			send_pumpconfiguaration(PhoneNumber);
		}

		else if (strstr(smsbuffer, "scheduleconfig") != 0)
		{
			//	ReadtnkconfigFile();  //dg_sunil
			Pch1 = strtok((char *)smsbuffer, (char *)",");
			StrTokStrVer = 0;
			while (Pch1 != NULL)
			{
				strcpy(StrTokStr1[StrTokStrVer], Pch1);
				StrTokStrVer++;
				Pch1 = strtok(NULL, ",");
			}
			pumpno_f = myatoi(StrTokStr1[1]);
			sprintf(buf, "\n\rpumpno_f>>:%d\n\r", pumpno_f);
			sAPI_UartPrintf(buf);
			pumpno_tx = pumpno_f - 1;

			s_npump[pumpno_tx].m_Tank_on_off = myatoi(StrTokStr1[2]);
			s_npump[pumpno_tx].m_Sump_on_off = myatoi(StrTokStr1[3]);
			s_npump[pumpno_tx].m_scheduleonof = myatoi(StrTokStr1[4]);
			s_npump[pumpno_tx].m_rundays = myatoi(StrTokStr1[5]);
			s_npump[pumpno_tx].m_skipdays = myatoi(StrTokStr1[6]);
			s_npump[pumpno_tx].m_Uppertank_restart_off = myatoi(StrTokStr1[7]);
			s_npump[pumpno_tx].m_Lowertank_restart_off = myatoi(StrTokStr1[8]);
			send_scheduleconfiguaration(PhoneNumber);
			WritetnkconfigFile();
			ReadtnkconfigFile();
			s_nMSettings.m_settings_req_flag = 1;
			s_nMSettings.m_settings_count = 1;
		}
		//	refer E:\Dhanagopal\From Subash\ORO_PUMP_SAMD21J18_270224_RTC_NEWLOGIC_SUBASH\NODE_SAM_LORA\src
		else if (strstr(smsbuffer, "tankconfig") != 0)
		{
			ReadtnkconfigFile();
			Pch1 = strtok((char *)smsbuffer, (char *)",");
			StrTokStrVer = 0;
			while (Pch1 != NULL)
			{
				strcpy(StrTokStr1[StrTokStrVer], Pch1);
				StrTokStrVer++;
				Pch1 = strtok(NULL, ",");
			}
			s_npump[0].m_no_of_sump_pins = myatoi(StrTokStr1[1]);
			s_npump[0].m_sump_pin_no[0] = myatoi(StrTokStr1[2]);
			s_npump[0].m_sump_pin_no[1] = myatoi(StrTokStr1[3]);
			s_npump[0].m_no_of_tank_pins = myatoi(StrTokStr1[4]);
			s_npump[0].m_tank_pin_no[0] = myatoi(StrTokStr1[5]);
			s_npump[0].m_tank_pin_no[1] = myatoi(StrTokStr1[6]);
			s_npump[0].m_Level_on_off = myatoi(StrTokStr1[7]);
			s_npump[0].m_flowonof = myatoi(StrTokStr1[8]);
			s_npump[0].m_pressureonof = myatoi(StrTokStr1[9]);

			s_npump[1].m_no_of_sump_pins = myatoi(StrTokStr1[10]);
			s_npump[1].m_sump_pin_no[0] = myatoi(StrTokStr1[11]);
			s_npump[1].m_sump_pin_no[1] = myatoi(StrTokStr1[12]);
			s_npump[1].m_no_of_tank_pins = myatoi(StrTokStr1[13]);
			s_npump[1].m_tank_pin_no[0] = myatoi(StrTokStr1[14]);
			s_npump[1].m_tank_pin_no[1] = myatoi(StrTokStr1[15]);
			s_npump[1].m_Level_on_off = myatoi(StrTokStr1[16]);
			s_npump[1].m_flowonof = myatoi(StrTokStr1[17]);
			s_npump[1].m_pressureonof = myatoi(StrTokStr1[18]);

			s_npump[2].m_no_of_sump_pins = myatoi(StrTokStr1[19]);
			s_npump[2].m_sump_pin_no[0] = myatoi(StrTokStr1[20]);
			s_npump[2].m_sump_pin_no[1] = myatoi(StrTokStr1[21]);
			s_npump[2].m_no_of_tank_pins = myatoi(StrTokStr1[22]);
			s_npump[2].m_tank_pin_no[0] = myatoi(StrTokStr1[23]);
			s_npump[2].m_tank_pin_no[1] = myatoi(StrTokStr1[24]);
			s_npump[2].m_Level_on_off = myatoi(StrTokStr1[25]);
			s_npump[2].m_flowonof = myatoi(StrTokStr1[26]);
			s_npump[2].m_pressureonof = myatoi(StrTokStr1[27]);
			send_tank_finalconfiguaration(PhoneNumber);
			WritetnkconfigFile();
			ReadtnkconfigFile();
			send_tankconfiguaration(PhoneNumber);
			s_nMSettings.m_settings_req_flag = 1;
			s_nMSettings.m_settings_count = 1;
		}

		else if (strstr(smsbuffer, "voltageconfig") != 0)
		{
			ReadvolsetFile();
			Pch1 = strtok((char *)smsbuffer, (char *)",");
			StrTokStrVer = 0;
			while (Pch1 != NULL)
			{
				strcpy(StrTokStr1[StrTokStrVer], Pch1);
				StrTokStrVer++;
				Pch1 = strtok(NULL, ",");
			}
			s_nMSettings.m_LowVoltOnOff = myatoi(StrTokStr1[1]);

			s_nTimerSettings.m_LowVoltIII = myatoi(StrTokStr1[2]);
			s_nTimerSettings.m_DiffVoltIII = myatoi(StrTokStr1[3]);
			s_nTimerSettings.m_LowVoltII = myatoi(StrTokStr1[4]);
			s_nTimerSettings.m_DiffVoltII = myatoi(StrTokStr1[5]);

			s_nMSettings.m_HighVoltOnOff = myatoi(StrTokStr1[6]);
			s_nTimerSettings.m_HighVoltIII = myatoi(StrTokStr1[7]);
			s_nTimerSettings.m_HiDiffVoltIII = myatoi(StrTokStr1[8]);
			s_nTimerSettings.m_HighVoltII = myatoi(StrTokStr1[9]);
			s_nTimerSettings.m_HiDiffVoltII = myatoi(StrTokStr1[10]);

			s_nTimerSettings.m_ImbVolt = myatoi(StrTokStr1[11]);
			s_nMSettings.m_RvePhOnoff = myatoi(StrTokStr1[12]);
			s_nMSettings.m_SppOnoff = myatoi(StrTokStr1[13]);
			s_nMSettings.m_CurSppOnOff = myatoi(StrTokStr1[14]);

			//	//
			WritevolsetFile();
			ReadvolsetFile();
			send_voltageconfiguaration(PhoneNumber);
			s_nMSettings.m_settings_req_flag = 1;
			s_nMSettings.m_settings_count = 3;
		}

		else if (strstr(smsbuffer, "currentconfig") != 0)
		{
			// unsigned char pumpno_f;
			Pch1 = strtok((char *)smsbuffer, (char *)",");
			StrTokStrVer = 0;
			while (Pch1 != NULL)
			{
				strcpy(StrTokStr1[StrTokStrVer], Pch1);
				StrTokStrVer++;
				Pch1 = strtok(NULL, ",");
			}
			pumpno_f = myatoi(StrTokStr1[1]);
			sprintf(buf, "\n\rpumpno_f>>:%d\n\r", pumpno_f);
			sAPI_UartPrintf(buf);
			pumpno_tx = pumpno_f - 1;
			ReadcursetFile(pumpno_f);
			pumpno_tx = pumpno_f - 1;
			sprintf(buf, "\n\rpumpno_f>>:%d pumpno_tx %d\n\r", pumpno_f, pumpno_tx);
			sAPI_UartPrintf(buf);
			s_nMSettings.m_DrScOnOf[pumpno_tx] = myatoi(StrTokStr1[2]);
			s_nTimerSettings.m_DrScHr[pumpno_tx] = myatoi(StrTokStr1[3]);
			s_nTimerSettings.m_DrScMin[pumpno_tx] = myatoi(StrTokStr1[4]);
			s_nTimerSettings.m_DrScSec[pumpno_tx] = myatoi(StrTokStr1[5]);
			s_nTimerSettings.m_DrAmpsIII[pumpno_tx] = myatof(StrTokStr1[6]);
			s_nTimerSettings.m_DrAmpsII[pumpno_tx] = myatof(StrTokStr1[7]);
			s_nMSettings.m_DrReOnOf[pumpno_tx] = myatoi(StrTokStr1[8]);
			s_nTimerSettings.m_DrReHr[pumpno_tx] = myatoi(StrTokStr1[9]);
			s_nTimerSettings.m_DrReMin[pumpno_tx] = myatoi(StrTokStr1[10]);
			s_nTimerSettings.m_DrReSec[pumpno_tx] = myatoi(StrTokStr1[11]);
			s_nMSettings.m_DrOccurOnOff[pumpno_tx] = myatoi(StrTokStr1[12]);
			s_nTimerSettings.m_DrOccurTimHr[pumpno_tx] = myatoi(StrTokStr1[13]);
			s_nTimerSettings.m_DrOccurTimMin[pumpno_tx] = myatoi(StrTokStr1[14]);
			s_nTimerSettings.m_DrOccurTimSec[pumpno_tx] = myatoi(StrTokStr1[15]);
			a_occurance_count[pumpno_tx] = myatoi(StrTokStr1[16]);
			s_nMSettings.m_AutoDrRunRstIIOnOff[pumpno_tx] = myatoi(StrTokStr1[17]);
			//	s_nMSettings.m_Drrestartpoweronof[pumpno_tx]=myatoi(StrTokStr1[17]);
			s_nMSettings.m_OlOnOff[pumpno_tx] = myatoi(StrTokStr1[18]);
			s_nTimerSettings.m_OlScanHr[pumpno_tx] = myatoi(StrTokStr1[19]);
			s_nTimerSettings.m_OlScanMin[pumpno_tx] = myatoi(StrTokStr1[20]);
			s_nTimerSettings.m_OlScanSec[pumpno_tx] = myatoi(StrTokStr1[21]);
			s_nTimerSettings.m_OlAmpsIII[pumpno_tx] = myatof(StrTokStr1[22]);
			s_nTimerSettings.m_OlAmpsII[pumpno_tx] = myatof(StrTokStr1[23]);
			//	s_nMSettings.m_Drrestartpoweronof[pumpno_tx]=myatoi(StrTokStr1[24]);
			//	s_nMSettings.m_OlRstVolOnoff[pumpno_tx]=myatoi(StrTokStr1[24]);
			s_nMSettings.m_AutoOlDrRstIIOnOff[pumpno_tx] = myatoi(StrTokStr1[24]);
			s_nTimerSettings.m_AutoRstOn[pumpno_tx] = myatoi(StrTokStr1[25]);
			//	s_nTimerSettings.m_AutoRst2On=myatoi(StrTokStr1[26]);
			//	s_nMSettings.m_AutoOlDrRstIIOnOff[pumpno_tx]=myatoi(StrTokStr1[27]);
			/* s_nTimerSettings.m_POnHr[pumpno_tx]=myatoi(StrTokStr1[27]);
			s_nTimerSettings.m_POnMin[pumpno_tx]=myatoi(StrTokStr1[28]);
			s_nTimerSettings.m_POnSec[pumpno_tx]=myatoi(StrTokStr1[29]);
			s_nMSettings.m_PoScrDlOnOff[pumpno_tx]=myatoi(StrTokStr1[30]);
			s_nTimerSettings.m_PoScrDlHr[pumpno_tx]=myatoi(StrTokStr1[31]);
			s_nTimerSettings.m_PoScrDlMin[pumpno_tx]=myatoi(StrTokStr1[32]);
			s_nTimerSettings.m_PoScrDlSec[pumpno_tx]=myatoi(StrTokStr1[33]);
			s_nTimerSettings.m_SDHr[pumpno_tx]=myatoi(StrTokStr1[34]);
			s_nTimerSettings.m_SDMin[pumpno_tx]=myatoi(StrTokStr1[35]);
			s_nTimerSettings.m_SDSec[pumpno_tx]=myatoi(StrTokStr1[36]);
			s_nMSettings.m_SfbOnOff[pumpno_tx]=myatoi(StrTokStr1[37]);
			s_nTimerSettings.m_SfbHr[pumpno_tx]=myatoi(StrTokStr1[38]);
			s_nTimerSettings.m_SfbMin[pumpno_tx]=myatoi(StrTokStr1[39]);
			s_nTimerSettings.m_SfbSec[pumpno_tx]=myatoi(StrTokStr1[40]);
			s_nMSettings.m_CycLicOnOf[pumpno_tx]=myatoi(StrTokStr1[41]);
			s_nTimerSettings.m_CycLicOnHr[pumpno_tx]=myatoi(StrTokStr1[42]);
			s_nTimerSettings.m_CycLicOnMin[pumpno_tx]=myatoi(StrTokStr1[43]);
			s_nTimerSettings.m_CycLicOnSec[pumpno_tx]=myatoi(StrTokStr1[44]);
			s_nTimerSettings.m_CycLicOfHr[pumpno_tx]=myatoi(StrTokStr1[45]);
			s_nTimerSettings.m_CycLicOfMin[pumpno_tx]=myatoi(StrTokStr1[46]);
			s_nTimerSettings.m_CycLicOfSec[pumpno_tx]=myatoi(StrTokStr1[47]);
			s_nMSettings.m_MaxRnOnOf[pumpno_tx]=myatoi(StrTokStr1[48]);
			s_nTimerSettings.m_MaxRnHr[pumpno_tx]=myatoi(StrTokStr1[49]);
			s_nTimerSettings.m_MaxRnMin[pumpno_tx]=myatoi(StrTokStr1[50]);
			s_nTimerSettings.m_MaxRnSec[pumpno_tx]=myatoi(StrTokStr1[51]); */
			sprintf(buf, "\n\rpumpno_f>>:%d s_nMSettings.m_DrScOnOf[%d]:%d\n\r", pumpno_f, pumpno_tx, s_nMSettings.m_DrScOnOf[pumpno_tx]);
			sAPI_UartPrintf(buf);
			WritecursetFile(pumpno_f);
			ReadcursetFile(pumpno_f);

			

			send_currentconfiguaration(PhoneNumber);
			s_nMSettings.m_settings_req_flag = 1;
			s_nMSettings.m_settings_count = 4;
		}

		else if (strstr(smsbuffer, "delayconfig") != 0)
		{
			// unsigned char pumpno_f;
			Pch1 = strtok((char *)smsbuffer, (char *)",");
			StrTokStrVer = 0;
			while (Pch1 != NULL)
			{
				strcpy(StrTokStr1[StrTokStrVer], Pch1);
				StrTokStrVer++;
				Pch1 = strtok(NULL, ",");
			}
			//	pumpno_tx=myatoi(StrTokStr1[1]);
			//	pumpno_f=pumpno_tx-1;
			pumpno_f = myatoi(StrTokStr1[1]);
			pumpno_tx = pumpno_f - 1;
			ReaddelsetFile(pumpno_f);
			pumpno_tx = pumpno_f - 1;
			sprintf(buf, "\n\rpumpno_f>>:%d pumpno_tx %d\n\r", pumpno_f, pumpno_tx);
			sAPI_UartPrintf(buf);
			s_nTimerSettings.m_POnHr[pumpno_tx] = myatoi(StrTokStr1[2]);
			s_nTimerSettings.m_POnMin[pumpno_tx] = myatoi(StrTokStr1[3]);
			s_nTimerSettings.m_POnSec[pumpno_tx] = myatoi(StrTokStr1[4]);

			s_nTimerSettings.m_SDHr[pumpno_tx] = myatoi(StrTokStr1[5]);
			s_nTimerSettings.m_SDMin[pumpno_tx] = myatoi(StrTokStr1[6]);
			s_nTimerSettings.m_SDSec[pumpno_tx] = myatoi(StrTokStr1[7]);

			s_nMSettings.m_PoScrDlOnOff[pumpno_tx] = myatoi(StrTokStr1[8]);
			s_nTimerSettings.m_PoScrDlHr[pumpno_tx] = myatoi(StrTokStr1[9]);
			s_nTimerSettings.m_PoScrDlMin[pumpno_tx] = myatoi(StrTokStr1[10]);
			s_nTimerSettings.m_PoScrDlSec[pumpno_tx] = myatoi(StrTokStr1[11]);

			s_nMSettings.m_SfbOnOff[pumpno_tx] = myatoi(StrTokStr1[12]);
			s_nTimerSettings.m_SfbHr[pumpno_tx] = myatoi(StrTokStr1[13]);
			s_nTimerSettings.m_SfbMin[pumpno_tx] = myatoi(StrTokStr1[14]);
			s_nTimerSettings.m_SfbSec[pumpno_tx] = myatoi(StrTokStr1[15]);

			s_nMSettings.m_MaxRnOnOf[pumpno_tx] = myatoi(StrTokStr1[16]);
			s_nTimerSettings.m_MaxRnHr[pumpno_tx] = myatoi(StrTokStr1[17]);
			s_nTimerSettings.m_MaxRnMin[pumpno_tx] = myatoi(StrTokStr1[18]);
			s_nTimerSettings.m_MaxRnSec[pumpno_tx] = myatoi(StrTokStr1[19]);

			s_nMSettings.m_CycLicOnOf[pumpno_tx] = myatoi(StrTokStr1[20]);
			s_nTimerSettings.m_CycLicOnHr[pumpno_tx] = myatoi(StrTokStr1[21]);
			s_nTimerSettings.m_CycLicOnMin[pumpno_tx] = myatoi(StrTokStr1[22]);
			s_nTimerSettings.m_CycLicOnSec[pumpno_tx] = myatoi(StrTokStr1[23]);
			s_nTimerSettings.m_CycLicOfHr[pumpno_tx] = myatoi(StrTokStr1[24]);
			s_nTimerSettings.m_CycLicOfMin[pumpno_tx] = myatoi(StrTokStr1[25]);
			s_nTimerSettings.m_CycLicOfSec[pumpno_tx] = myatoi(StrTokStr1[26]);

			WritedelsetFile(pumpno_f);
			ReaddelsetFile(pumpno_f);
			send_delayconfiguaration(PhoneNumber);
			s_nMSettings.m_settings_req_flag = 1;
			s_nMSettings.m_settings_count = 5;
		}

		else if (strstr(smsbuffer, "rtcconfig") != 0)
		{
			// unsigned char pumpno_f;
			Pch1 = strtok((char *)smsbuffer, (char *)",");
			StrTokStrVer = 0;
			while (Pch1 != NULL)
			{
				strcpy(StrTokStr1[StrTokStrVer], Pch1);
				StrTokStrVer++;
				Pch1 = strtok(NULL, ",");
			}
			pumpno_f = myatoi(StrTokStr1[1]);
			pumpno_tx = pumpno_f - 1;
			ReadRTCsetFile(pumpno_f);
			pumpno_tx = pumpno_f - 1;
			sprintf(buf, "\n\rpumpno_f>>:%d pumpno_tx %d\n\r", pumpno_f, pumpno_tx);
			sAPI_UartPrintf(buf);
			s_nMSettings.m_RTCOnOf[pumpno_tx] = myatoi(StrTokStr1[2]);
			s_nTimerSettings.m_RTCONHr[pumpno_tx][1] = myatoi(StrTokStr1[3]);
			s_nTimerSettings.m_RTCONMin[pumpno_tx][1] = myatoi(StrTokStr1[4]);
			s_nTimerSettings.m_RTCONSec[pumpno_tx][1] = myatoi(StrTokStr1[5]);
			s_nTimerSettings.m_RTCOfHr[pumpno_tx][1] = myatoi(StrTokStr1[6]);
			s_nTimerSettings.m_RTCOfMin[pumpno_tx][1] = myatoi(StrTokStr1[7]);
			s_nTimerSettings.m_RTCOfSec[pumpno_tx][1] = myatoi(StrTokStr1[8]);
			s_nTimerSettings.m_RTCONHr[pumpno_tx][2] = myatoi(StrTokStr1[9]);
			s_nTimerSettings.m_RTCONMin[pumpno_tx][2] = myatoi(StrTokStr1[10]);
			s_nTimerSettings.m_RTCONSec[pumpno_tx][2] = myatoi(StrTokStr1[11]);
			s_nTimerSettings.m_RTCOfHr[pumpno_tx][2] = myatoi(StrTokStr1[12]);
			s_nTimerSettings.m_RTCOfMin[pumpno_tx][2] = myatoi(StrTokStr1[13]);
			s_nTimerSettings.m_RTCOfSec[pumpno_tx][2] = myatoi(StrTokStr1[14]);
			s_nTimerSettings.m_RTCONHr[pumpno_tx][3] = myatoi(StrTokStr1[15]);
			s_nTimerSettings.m_RTCONMin[pumpno_tx][3] = myatoi(StrTokStr1[16]);
			s_nTimerSettings.m_RTCONSec[pumpno_tx][3] = myatoi(StrTokStr1[17]);
			s_nTimerSettings.m_RTCOfHr[pumpno_tx][3] = myatoi(StrTokStr1[18]);
			s_nTimerSettings.m_RTCOfMin[pumpno_tx][3] = myatoi(StrTokStr1[19]);
			s_nTimerSettings.m_RTCOfSec[pumpno_tx][3] = myatoi(StrTokStr1[20]);
			s_nTimerSettings.m_RTCONHr[pumpno_tx][4] = myatoi(StrTokStr1[21]);
			s_nTimerSettings.m_RTCONMin[pumpno_tx][4] = myatoi(StrTokStr1[22]);
			s_nTimerSettings.m_RTCONSec[pumpno_tx][4] = myatoi(StrTokStr1[23]);
			s_nTimerSettings.m_RTCOfHr[pumpno_tx][4] = myatoi(StrTokStr1[24]);
			s_nTimerSettings.m_RTCOfMin[pumpno_tx][4] = myatoi(StrTokStr1[25]);
			s_nTimerSettings.m_RTCOfSec[pumpno_tx][4] = myatoi(StrTokStr1[26]);
			s_nTimerSettings.m_RTCONHr[pumpno_tx][5] = myatoi(StrTokStr1[27]);
			s_nTimerSettings.m_RTCONMin[pumpno_tx][5] = myatoi(StrTokStr1[28]);
			s_nTimerSettings.m_RTCONSec[pumpno_tx][5] = myatoi(StrTokStr1[29]);
			s_nTimerSettings.m_RTCOfHr[pumpno_tx][5] = myatoi(StrTokStr1[30]);
			s_nTimerSettings.m_RTCOfMin[pumpno_tx][5] = myatoi(StrTokStr1[31]);
			s_nTimerSettings.m_RTCOfSec[pumpno_tx][5] = myatoi(StrTokStr1[32]);
			s_nTimerSettings.m_RTCONHr[pumpno_tx][6] = myatoi(StrTokStr1[33]);
			s_nTimerSettings.m_RTCONMin[pumpno_tx][6] = myatoi(StrTokStr1[34]);
			s_nTimerSettings.m_RTCONSec[pumpno_tx][6] = myatoi(StrTokStr1[35]);
			s_nTimerSettings.m_RTCOfHr[pumpno_tx][6] = myatoi(StrTokStr1[36]);
			s_nTimerSettings.m_RTCOfMin[pumpno_tx][6] = myatoi(StrTokStr1[37]);
			s_nTimerSettings.m_RTCOfSec[pumpno_tx][6] = myatoi(StrTokStr1[38]);

			WriteRTCsetFile(pumpno_f);
			ReadRTCsetFile(pumpno_f);

			sprintf(buf, "\nRTC_56:%d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%02d,%d", pumpno_tx, s_nTimerSettings.m_RTCONHr[pumpno_tx][5],
					s_nTimerSettings.m_RTCONMin[pumpno_tx][5], s_nTimerSettings.m_RTCONSec[pumpno_tx][5], s_nTimerSettings.m_RTCOfHr[pumpno_tx][5],
					s_nTimerSettings.m_RTCOfMin[pumpno_tx][5], s_nTimerSettings.m_RTCOfSec[pumpno_tx][5], s_nTimerSettings.m_RTCONHr[pumpno_tx][6], s_nTimerSettings.m_RTCONMin[pumpno_tx][6], s_nTimerSettings.m_RTCONSec[pumpno_tx][6],
					s_nTimerSettings.m_RTCOfHr[pumpno_tx][6], s_nTimerSettings.m_RTCOfMin[pumpno_tx][6], s_nTimerSettings.m_RTCOfSec[pumpno_tx][6], __LINE__);
			sAPI_UartPrintf(buf);

			send_rtcconfiguaration(PhoneNumber);
			s_nMSettings.m_settings_req_flag = 1;
			s_nMSettings.m_settings_count = 6;
		}

		else if (strstr(smsbuffer, "ctconfig") != 0)
		{
			Pch1 = strtok((char *)smsbuffer, (char *)",");
			StrTokStrVer = 0;
			while (Pch1 != NULL)
			{
				strcpy(StrTokStr1[StrTokStrVer], Pch1);
				StrTokStrVer++;
				Pch1 = strtok(NULL, ",");
			}
			s_nMSettings.m_AutoStIIOnOff[0] = myatoi(StrTokStr1[1]); ////array not needed
			s_nMSettings.m_CTRonoff = myatoi(StrTokStr1[2]);
			s_nMSettings.m_CTYonoff = myatoi(StrTokStr1[3]);
			s_nMSettings.m_CTBonoff = myatoi(StrTokStr1[4]);
			s_ntank.m_tank_height = myatof(StrTokStr1[5]);
			s_ntank.m_min_level_p = myatoi(StrTokStr1[6]);
			s_ntank.m_max_level_p = myatoi(StrTokStr1[7]);
			s_nTimerSettings.m_AutoRst2On = myatoi(StrTokStr1[8]);
			s_nMSettings.m_NlightOnOf = myatoi(StrTokStr1[9]);
			s_nMSettings.m_NlightRTCOnHr = myatoi(StrTokStr1[10]);
			s_nMSettings.m_NlightRTCOnMin = myatoi(StrTokStr1[11]);
			//		s_nMSettings.m_NlightRTCOnSec=myatoi(StrTokStr1[12]);
			s_nMSettings.m_NlightRTCOfHr = myatoi(StrTokStr1[12]);
			s_nMSettings.m_NlightRTCOfMin = myatoi(StrTokStr1[13]);
			//		s_nMSettings.m_NlightRTCOfSec=myatoi(StrTokStr1[15]);
			s_nMSettings.m_peakHourOnOf = myatoi(StrTokStr1[14]);
			s_nMSettings.m_peakOnHr = myatoi(StrTokStr1[15]);
			s_nMSettings.m_peakOnMin = myatoi(StrTokStr1[16]);
			s_nMSettings.m_peakOnSec = myatoi(StrTokStr1[17]);
			s_nMSettings.m_peakOfHr = myatoi(StrTokStr1[18]);
			s_nMSettings.m_peakOfMin = myatoi(StrTokStr1[19]);
			s_nMSettings.m_peakOfSec = myatoi(StrTokStr1[20]);
			s_ntank.m_Uppertank = myatoi(StrTokStr1[21]);
			s_ntank.m_Lowertank = myatoi(StrTokStr1[22]);
			s_ntank.m_Level_Sensor_height = myatoi(StrTokStr1[23]);

			sprintf(buf, "Level_Sensor_height=%d, Level_calfactor=%0.2f,Uppertank=%d,Lowertank=%d", s_ntank.m_Level_Sensor_height, s_ntank.m_Level_calfactor, s_ntank.m_Uppertank, s_ntank.m_Lowertank);
			sAPI_UartPrintf(buf);

			PresSenFlag=myatoi(StrTokStr1[24]);
			PreSenTime[0]=myatoi(StrTokStr1[25]);
			PreSenTime[1]=myatoi(StrTokStr1[26]);
			PreSenTime[2]=myatoi(StrTokStr1[27]);
			HighFlow=myatof(StrTokStr1[28]);
			LowFlow=myatof(StrTokStr1[29]);
			PreSwFlag=myatoi(StrTokStr1[30]);
			PreSwTime[0]=myatoi(StrTokStr1[31]);
			PreSwTime[1]=myatoi(StrTokStr1[32]);
			PreSwTime[2]=myatoi(StrTokStr1[33]);
			HighFlow+=0.02;
			LowFlow+=0.02;
			
			
			sprintf(buf,"Level_Sensor_height=%d, Level_calfactor=%0.2f,Uppertank=%d,Lowertank=%d",s_ntank.m_Level_Sensor_height,s_ntank.m_Level_calfactor,s_ntank.m_Uppertank,s_ntank.m_Lowertank);
			sAPI_UartPrintf(buf);
			sprintf(buf,"\nPresSenFlag:%d--%d--%d--%d--%f--%f--%d--%d--%d--%d\n",PresSenFlag,PreSenTime[0],PreSenTime[1],PreSenTime[2],HighFlow,LowFlow,PreSwFlag,PreSwTime[0],PreSwTime[1],PreSwTime[2]);
			sAPI_UartPrintf(buf);
			
			s_nMSettings.m_settings_count=4;
			s_nMSettings.m_settings_req_flag=1;
			
			sprintf(buf,"\n\r m_settings_count %d m_settings_req_flag %d line %d",s_nMSettings.m_settings_count,s_nMSettings.m_settings_req_flag,__LINE__);
			sAPI_UartPrintf(buf);
			
			WritectsetFile(); 
			ReadctsetFile();	
			WriteEcosetFile();	
			ReadEcosetFile();	
			send_ctconfiguaration(PhoneNumber);
		}
			else if(strstr(smsbuffer,"valvesetting") != 0)
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
			sAPI_UartPrintf(buf);
			}
			s_nMSettings.ValveOnOff=myatoi(StrTokStr1[1]);
			s_nMSettings.CyclicOnOff=myatoi(StrTokStr1[2]);
			
		
			s_nMSettings.CycLicIntervelHr=myatoi(StrTokStr1[3]);
			s_nMSettings.CycLicIntervelMin=myatoi(StrTokStr1[4]);
			s_nMSettings.CycLicIntervelSec=myatoi(StrTokStr1[5]);	
			
			
			
			if(s_nMSettings.ValveOnOff==1)
			{
				s_nMSettings.CyclicLimit=myatoi(StrTokStr1[6]);
				s_nMSettings.m_ValveTimerHr[0]=myatoi(StrTokStr1[7]);
				s_nMSettings.m_ValveTimerMin[0]=myatoi(StrTokStr1[8]);
				s_nMSettings.m_ValveTimerHr[1]=myatoi(StrTokStr1[9]);
				s_nMSettings.m_ValveTimerMin[1]=myatoi(StrTokStr1[10]);
				s_nMSettings.m_ValveTimerHr[2]=myatoi(StrTokStr1[11]);
				s_nMSettings.m_ValveTimerMin[2]=myatoi(StrTokStr1[12]);
				s_nMSettings.m_ValveTimerHr[3]=myatoi(StrTokStr1[13]);
				s_nMSettings.m_ValveTimerMin[3]=myatoi(StrTokStr1[14]);
				s_nMSettings.m_ValveTimerHr[4]=myatoi(StrTokStr1[15]);
				s_nMSettings.m_ValveTimerMin[4]=myatoi(StrTokStr1[16]);
				s_nMSettings.m_ValveTimerHr[5]=myatoi(StrTokStr1[17]);
				s_nMSettings.m_ValveTimerMin[5]=myatoi(StrTokStr1[18]);
				s_nMSettings.m_ValveTimerHr[6]=myatoi(StrTokStr1[19]);
				s_nMSettings.m_ValveTimerMin[6]=myatoi(StrTokStr1[20]);
				s_nMSettings.m_ValveTimerHr[7]=myatoi(StrTokStr1[21]);
				s_nMSettings.m_ValveTimerMin[7]=myatoi(StrTokStr1[22]);
				s_nMSettings.m_ValveTimerHr[8]=myatoi(StrTokStr1[23]);
				s_nMSettings.m_ValveTimerMin[8]=myatoi(StrTokStr1[24]);
				s_nMSettings.m_ValveTimerHr[9]=myatoi(StrTokStr1[25]);
				s_nMSettings.m_ValveTimerMin[9]=myatoi(StrTokStr1[26]);
			}
			else if(s_nMSettings.ValveOnOff==0)
			{
				s_nMSettings.CyclicLimit=0;
				s_nMSettings.m_ValveTimerHr[0]=0;
				s_nMSettings.m_ValveTimerMin[0]=0;
				s_nMSettings.m_ValveTimerHr[1]=0;
				s_nMSettings.m_ValveTimerMin[1]=0;
				s_nMSettings.m_ValveTimerHr[2]=0;
				s_nMSettings.m_ValveTimerMin[2]=0;
				s_nMSettings.m_ValveTimerHr[3]=0;
				s_nMSettings.m_ValveTimerMin[3]=0;
				s_nMSettings.m_ValveTimerHr[4]=0;
				s_nMSettings.m_ValveTimerMin[4]=0;
				s_nMSettings.m_ValveTimerHr[5]=0;
				s_nMSettings.m_ValveTimerMin[5]=0;
				s_nMSettings.m_ValveTimerHr[6]=0;
				s_nMSettings.m_ValveTimerMin[6]=0;
				s_nMSettings.m_ValveTimerHr[7]=0;
				s_nMSettings.m_ValveTimerMin[7]=0;
				s_nMSettings.m_ValveTimerHr[8]=0;
				s_nMSettings.m_ValveTimerMin[8]=0;
				s_nMSettings.m_ValveTimerHr[9]=0;
				s_nMSettings.m_ValveTimerMin[9]=0;
			}
			
			sprintf(buf,"\n\rValve timer:%d--%d\n%d--%d\n%d--%d\n%d--%d\n%d--%d\n%d--%d\n%d--%d\n%d--%d\n%d--%d\n%d--%d\n%d--%d\n\n\r",s_nMSettings.m_ValveTimerHr[0],s_nMSettings.m_ValveTimerMin[0]
			,s_nMSettings.m_ValveTimerHr[1],s_nMSettings.m_ValveTimerMin[1],s_nMSettings.m_ValveTimerHr[2],s_nMSettings.m_ValveTimerMin[2],s_nMSettings.m_ValveTimerHr[3],s_nMSettings.m_ValveTimerMin[3]
			,s_nMSettings.m_ValveTimerHr[4],s_nMSettings.m_ValveTimerMin[4],s_nMSettings.m_ValveTimerHr[5],s_nMSettings.m_ValveTimerMin[5],s_nMSettings.m_ValveTimerHr[6],s_nMSettings.m_ValveTimerMin[6]
			,s_nMSettings.m_ValveTimerHr[7],s_nMSettings.m_ValveTimerMin[7],s_nMSettings.m_ValveTimerHr[8],s_nMSettings.m_ValveTimerMin[8],s_nMSettings.m_ValveTimerHr[9],s_nMSettings.m_ValveTimerMin[9]
			,s_nMSettings.CycLicIntervelHr,s_nMSettings.CycLicIntervelMin);
	//		WriteEcosetFile();
	//		ReadEcosetFile();
			sprintf(buf,"\n\rValve timer:%d--%d\n%d--%d\n%d--%d\n%d--%d\n%d--%d\n%d--%d\n%d--%d\n%d--%d\n%d--%d\n%d--%d\n%d--%d\n\n\r",s_nMSettings.m_ValveTimerHr[0],s_nMSettings.m_ValveTimerMin[0]
			,s_nMSettings.m_ValveTimerHr[1],s_nMSettings.m_ValveTimerMin[1],s_nMSettings.m_ValveTimerHr[2],s_nMSettings.m_ValveTimerMin[2],s_nMSettings.m_ValveTimerHr[3],s_nMSettings.m_ValveTimerMin[3]
			,s_nMSettings.m_ValveTimerHr[4],s_nMSettings.m_ValveTimerMin[4],s_nMSettings.m_ValveTimerHr[5],s_nMSettings.m_ValveTimerMin[5],s_nMSettings.m_ValveTimerHr[6],s_nMSettings.m_ValveTimerMin[6]
			,s_nMSettings.m_ValveTimerHr[7],s_nMSettings.m_ValveTimerMin[7],s_nMSettings.m_ValveTimerHr[8],s_nMSettings.m_ValveTimerMin[8],s_nMSettings.m_ValveTimerHr[9],s_nMSettings.m_ValveTimerMin[9]
			,s_nMSettings.CycLicIntervelHr,s_nMSettings.CycLicIntervelMin);
			sAPI_UartPrintf(buf);
			s_nMSettings.m_settings_count=11;
			s_nMSettings.m_settings_req_flag=1;
			
			send_Valveconfiguaration(PhoneNumber);
		}

		else if (strstr(smsbuffer, "calibration") != 0)
		{
			Pch1 = strtok((char *)smsbuffer, (char *)",");
			StrTokStrVer = 0;
			while (Pch1 != NULL)
			{
				strcpy(StrTokStr1[StrTokStrVer], Pch1);
				StrTokStrVer++;
				Pch1 = strtok(NULL, ",");
			}

			for (Tp = 0; Tp <= StrTokStrVer; Tp++)
			{
				sprintf(buf, "\n\rSt=%s  pos=%d\n\r", StrTokStr1[Tp], Tp);
				sAPI_UartPrintf(buf);
			}
			s_nMSettings.m_CalRVoltage = myatof(StrTokStr1[1]); ////array not needed
			s_nMSettings.m_CalYVoltage = myatof(StrTokStr1[2]);
			s_nMSettings.m_CalBVoltage = myatof(StrTokStr1[3]);
			s_nMSettings.m_CalRCurrent = myatof(StrTokStr1[4]); ////array not needed
			s_nMSettings.m_CalYCurrent = myatof(StrTokStr1[5]);
			s_nMSettings.m_CalBCurrent = myatof(StrTokStr1[6]);
			s_nMSettings.m_Flow_calfactor = myatof(StrTokStr1[7]);
			s_nMSettings.m_Press_calfactor = myatof(StrTokStr1[8]);
			s_ntank.m_Press_Max_Value = myatoi(StrTokStr1[9]);
			Liter_Per_Pulse = myatoi(StrTokStr1[10]);
			Flow_reset = myatoi(StrTokStr1[11]);
			s_ntank.m_Level_calfactor = myatof(StrTokStr1[12]);

			WritectsetFile();
			ReadctsetFile();

			send_calibration(PhoneNumber);
			s_nMSettings.calibration_flag = 1;
		}

		else if (strstr(smsbuffer, "viewconfig") != 0)
		{
			View_Settings_flag = 1;
		}

		else if (strstr(smsbuffer, "reset") != 0)
		{
			sAPI_SysReset();
		}
		else if (strstr(smsbuffer, "cfunrst") != 0)
		{
			sAPI_NetworkSetCfun(4);
			sAPI_TaskSleep(200 * 2);
			sAPI_NetworkSetCfun(1);
		}
		else if (strstr(smsbuffer, "getdata") != 0)
		{
			NoAcceptSMS = 0;
			send_getdata(SmsNumber[0]);
		}
		else if (strstr(smsbuffer, "onehr") != 0)
		{
			onehour_send_flag = 1;
		}
		else if (strstr(smsbuffer, "stat") != 0)
		{
			NoAcceptSMS = 0;
			 
			//	send_test_smsStatus(SmsNumber[0]);  //dg_nsdk
		}

		else if ((strstr(SmsNumber[0], p + (strlen(StoredPhoneSmscode[0]))) != 0) && (strstr(smsbuffer, "pass#8185") != 0))
		{
			Pch1 = strtok((char *)smsbuffer, (char *)",");
			StrTokStrVer = 0;
			while (Pch1 != NULL)
			{
				strcpy(StrTokStr1[StrTokStrVer], Pch1);
				StrTokStrVer++;
				Pch1 = strtok(NULL, ",");
			}
			// Tp=smsbuffer[3]-'0';
			// Tp1=smsbuffer[4]-'0';
			// regsmsno=Tp*10+Tp;
			memset(DeviceConfig.smsPassword, 0x00, sizeof(DeviceConfig.smsPassword));
			strncpy(DeviceConfig.smsPassword, StrTokStr1[1], SMS_PASSWORD_LEN);

			printParameter();
			// app_nvram_save(MCONFIG_AT_INDEX, (UINT8*)&DeviceConfig, sizeof(DeviceConfig));

			//	send_pass( PhoneNumber); //dg_nsdk
		}
		else if ((strstr(SmsNumber[0], p + (strlen(StoredPhoneSmscode[0]))) != 0) && (strstr(smsbuffer, "getpass") != 0))
		{
			//  send_pass(PhoneNumber);  //dg_nsdk
		}
		// else if((strstr(StoredPhoneNumber[mobindx],p+(strlen(StoredPhoneSmscode[mobindx])))!=0) ||(nMSettings.serviceOnOff==1 && ServiceNumberFound == 1 && (strstr(ServiceNumber,p+(strlen(StoredPhoneSmscode[11])))!=0)) || (strstr(DEFAULTNUMBER,p)!=0)|| (strstr(DEFAULTNUMBER1,p)!=0)|| (strstr(DEFAULTNUMBER2,p)!=0)|| (strstr(DEFAULTNUMBER3,p)!=0)|| (strstr(DEFAULTNUMBER4,p)!=0)|| (strstr(DEFAULTNUMBER5,p)!=0)|| (strstr(DEFAULTNUMBER6,p)!=0))
		else if (1)
		{
			sAPI_UartPrintf("smsNumber match\n");
			// if((strstr(smsbuffer,"message"))||(strstr(smsbuffer,"Successfully"))){}
			// else
			// MassageReceived=3;                             //display received status on LCD
			/*p = smsbuffer;
			  for (i = 0; i < smsReadCnfContent.len; i++) // convert to Lcaps
			  {
				  if(*p>='A' && *p<='Z')
				  *p = (*p+'a')-'A';
				  p++;
			  }
			  p = smsbuffer;
			  sprintf(buf,"LsmsData %s,p %s\n",smsbuffer,p);
			  sAPI_UartPrintf(buf);
			  for (i = 0; i < sizeof(smscmdTable) / sizeof(smscmdTable[0]); i++)
			  {
				  if (!strncmp(smscmdTable[i], p, strlen(smscmdTable[i])))
				  {
					  rspType = i;
					  break;
				  }
			  }
			  sAPI_Debug("SMS rspType =%d,smscmdTable=%s",rspType,smscmdTable[rspType]);*/

			if (strstr(smsbuffer, "reg") != 0)
			{
				// char SaveNumber[15] = "";
				UINT8 TpStrtok[160] = "";
				UINT8 SaveNumberPos = 1, len;
				UINT8 TpStrtokVer = 0;
				UINT8 Tp = 0, Tp1 = 0;
				len = strlen(smsbuffer);
				sprintf(buf, "\n\rbuffer[3] = %c\n\r", smsbuffer[3]);
				sAPI_UartPrintf(buf);

				Pch1 = strtok((char *)smsbuffer, (char *)",");
				StrTokStrVer = 0;
				while (Pch1 != NULL)
				{
					strcpy(StrTokStr1[StrTokStrVer], Pch1);
					StrTokStrVer++;
					Pch1 = strtok(NULL, ",");
				}
				Tp = smsbuffer[3] - '0';
				Tp1 = smsbuffer[4] - '0';
				regsmsno = Tp * 10 + Tp1;
				if (regsmsno < 15 && regsmsno != 0)
				{
					/*memset(DeviceConfig.MobileNumber[regsmsno], 0x00, sizeof(DeviceConfig.MobileNumber[regsmsno]));
					strncpy(DeviceConfig.MobileNumber[regsmsno],StrTokStr1[2],MASTER_MOBILE_NUMBER_LEN);
					memset(DeviceConfig.MobileSmscode[regsmsno], 0x00, sizeof(DeviceConfig.MobileSmscode[regsmsno]));
					strncpy(DeviceConfig.MobileSmscode[regsmsno],StrTokStr1[1],MASTER_MOBILE_SMSCODE_LEN);
					memset(DeviceConfig.MobileCallcode[regsmsno], 0x00, sizeof(DeviceConfig.MobileCallcode[regsmsno]));
					strncpy(DeviceConfig.MobileCallcode[regsmsno],StrTokStr1[3],MASTER_MOBILE_CALLCODE_LEN);*/

					// printParameter();
					// app_nvram_save(MCONFIG_AT_INDEX, (UINT8*)&DeviceConfig, sizeof(DeviceConfig));

					ReadPhoneNumber();

					strcpy(StoredPhoneSmscode[regsmsno], StrTokStr1[1]);
					strcpy(StoredPhoneNumber[regsmsno], StrTokStr1[2]);
					// strcpy(StoredPhoneCallcode[regsmsno],StrTokStr1[3]);
					WritePhoneNumberFn();
					// ReadPhoneNumber();
					// send_regno(SmsNumber[0]);
					ReadPhoneNumber();
					memset(&Buffer1, 0, 500);
					sprintf(Buffer1, "REG %d No Is:%s\n", regsmsno, StoredPhoneNumber[regsmsno]);
					sprintf(Buffer1, "%sSMSCODE: %s", Buffer1, StoredPhoneSmscode[regsmsno]);
					// sprintf(Buffer1,"%sCALLCODE: %s",Buffer1,StoredPhoneCallcode[regsmsno]);
					strcpy((char *)SendSMSString, (char *)Buffer1);
					ph_numcheck();
					NumberChangeSMS = 1;
					SendSmsToAll = 1;
					RegxSmsSend = 1;
					// cpbrsearchend = 0;

					// eat_send_msg_to_user(EAT_USER_1, EAT_USER_0, EAT_FALSE,8, "MOBSTORE", EAT_NULL);
				}
			}
			else if (strstr(smsbuffer, "ftpipconfig"))
			{
				memset(DeviceConfig.FtpServerIP, 0x00, sizeof(DeviceConfig.FtpServerIP));
				strcpy(DeviceConfig.FtpServerIP, &smsbuffer[11]);
				app_nvram_save(MCONFIG_AT_INDEX, (u8 *)&DeviceConfig, sizeof(DeviceConfig));
				send_vipconfigset(PhoneNumber);
			}

			else if (strstr(smsbuffer, "ftpssidconfig"))
			{
				memset(DeviceConfig.ftpUserName, 0x00, sizeof(DeviceConfig.ftpUserName));
				strcpy(DeviceConfig.ftpUserName, &smsbuffer[13]);
				printParameter();
				app_nvram_save(MCONFIG_AT_INDEX, (u8 *)&DeviceConfig, sizeof(DeviceConfig));
				send_vipconfigset(PhoneNumber);
			}
			else if (strstr(smsbuffer, "ftppassconfig"))
			{
				memset(DeviceConfig.ftpPassword, 0x00, sizeof(DeviceConfig.ftpPassword));
				strcpy(DeviceConfig.ftpPassword, &smsbuffer[13]);
				printParameter();
				app_nvram_save(MCONFIG_AT_INDEX, (u8 *)&DeviceConfig, sizeof(DeviceConfig));
				send_vipconfigset(PhoneNumber);
			}
			else if (strstr(smsbuffer, "mqttipconfig"))
			{
				memset(DeviceConfig.MqttServerIP, 0x00, sizeof(DeviceConfig.MqttServerIP));
				strcpy(DeviceConfig.MqttServerIP, &smsbuffer[12]);
				sAPI_UartPrintf("MQTT IP ADDRESS:%s", DeviceConfig.MqttServerIP);
				printParameter();
				app_nvram_save(MCONFIG_AT_INDEX, (UINT8 *)&DeviceConfig, sizeof(DeviceConfig));
				send_vipconfigset2(PhoneNumber);
			}

			else if (strstr(smsbuffer, "mqttssidconfig"))
			{
				memset(DeviceConfig.MqttUserName, 0x00, sizeof(DeviceConfig.MqttUserName));
				strcpy(DeviceConfig.MqttUserName, &smsbuffer[14]);
				printParameter();
				app_nvram_save(MCONFIG_AT_INDEX, (u8 *)&DeviceConfig, sizeof(DeviceConfig));
				send_vipconfigset2(PhoneNumber);
			}
			else if (strstr(smsbuffer, "mqttpassconfig"))
			{
				memset(DeviceConfig.MqttPassword, 0x00, sizeof(DeviceConfig.MqttPassword));
				strcpy(DeviceConfig.MqttPassword, &smsbuffer[14]);
				printParameter();
				app_nvram_save(MCONFIG_AT_INDEX, (u8 *)&DeviceConfig, sizeof(DeviceConfig));
				send_vipconfigset2(PhoneNumber);
			}

			else if (strstr(smsbuffer, "wssidconfig"))
			{
				memset(DeviceConfig.wifiSSID, 0x00, sizeof(DeviceConfig.wifiSSID));
				strncpy(DeviceConfig.wifiSSID, &SMSbuffer[11], SSID_LEN);
				printParameter();
				app_nvram_save(MCONFIG_AT_INDEX, (UINT8 *)&DeviceConfig, sizeof(DeviceConfig));
				send_vipconfigset(PhoneNumber);
			}
			else if (strstr(smsbuffer, "wpassconfig"))
			{
				memset(DeviceConfig.wifiPassword, 0x00, sizeof(DeviceConfig.wifiPassword));
				strncpy(DeviceConfig.wifiPassword, &SMSbuffer[11], PASSWORD_LEN);
				printParameter();
				app_nvram_save(MCONFIG_AT_INDEX, (UINT8 *)&DeviceConfig, sizeof(DeviceConfig));
				send_vipconfigset(PhoneNumber);
			}
			else if (strstr(smsbuffer, "apnconfig"))
			{
				memset(DeviceConfig.apnName, 0x00, sizeof(DeviceConfig.apnName));
				strncpy(DeviceConfig.apnName, &SMSbuffer[9], APN_NAME_LEN);
				printParameter();
				app_nvram_save(MCONFIG_AT_INDEX, (UINT8 *)&DeviceConfig, sizeof(DeviceConfig));
				send_vipconfigset(PhoneNumber);
			}
			else if (strstr(smsbuffer, "vgetmyip"))
			{
				send_vipconfigset(PhoneNumber);
			}
			else if (strstr(smsbuffer, "#escape8186") != 0)
			{
				NoAcceptSMS = 0;
				WaitOK = 1;
				SendSMS = 0;
				Callreceiv = 0;
			}

			else if (strstr(smsbuffer, "dndvsms") != 0)
			{
				char SendString[160] = "No Number Found";
				// char Buffer1[200] = "";
				ReadFile();
				sprintf(SendString, "DNDSMS STATUS\n");
				for (Tp = 0; Tp < 4; Tp++)
				{
					if (DNDSMSFLAG[Tp] == 1)
						sprintf(SendString, "%sDNDSMS %d ON\n", SendString, Tp);
					else
						sprintf(SendString, "%sDNDSMS %d OFF\n", SendString, Tp);
				}
				sprintf(BigSMS1, "V56");
				send_test_smsNum1(SmsNumber[0], SendString, BigSMS1);
			}
			else if (strstr(smsbuffer, "dndsms") != 0)
			{
				// char SaveNumber[15] = "";
				char SendString[70] = "";
				unsigned char Tp;
				// ReadPhoneNumber();
				len = strlen(smsbuffer);
				StrTokStrVer = 0;
				// sAPI_Debug("\n\rbuffer[6] = %s\n\r",smsbuffer[6]);
				Pch1 = strtok((char *)smsbuffer, (char *)",");
				StrTokStrVer = 0;
				while (Pch1 != NULL)
				{
					strcpy(StrTokStr1[StrTokStrVer], Pch1);
					StrTokStrVer++;
					Pch1 = strtok(NULL, ",");
				}
				// Tp=smsbuffer[6]-'0';
				// Tp1=smsbuffer[7]-'0';
				regsmsno = myatoi(StrTokStr1[1]);
				// sAPI_Debug("\n\rCall on=%d \n\r",regsmsno);
				if (regsmsno < 4)
				{
					ReadFile();
					// Tp = myatoi(StrTokStr1[2]);
					DNDSMSFLAG[regsmsno] = myatoi(StrTokStr1[2]);
					//	sAPI_Debug("\n\rCall on=%d \n\r",DNDSMSFLAG[regsmsno]);
					//	strcpy(DNDSMSFLAG[regsmsno],StrTokStr1[2]);
					//	memset(DeviceConfig.MobileNumber[regsmsno+15], 0x00, sizeof(DeviceConfig.MobileNumber[regsmsno+15]));
					//	memset(DeviceConfig.MobileNumber[regsmsno+15], 0x00, sizeof(DeviceConfig.MobileNumber[regsmsno+15]));
					//	strncpy(DeviceConfig.MobileNumber[regsmsno+15],StoredPhoneNumber[regsmsno+15],MASTER_MOBILE_NUMBER_LEN);
					//	app_nvram_save(MCONFIG_AT_INDEX, (UINT8*)&DeviceConfig, sizeof(DeviceConfig));
					//	printParameter();
					WriteSettingsFile();
					ReadFile();
					if (DNDSMSFLAG[regsmsno] == 1)
						sprintf(SendString, "DNDSMS %d ON\n\r", regsmsno);
					else
						sprintf(SendString, "DNDSMS %d OFF\n\r", regsmsno);

					//	simcom_sms_send(SmsNumber[0], SendString, strlen(SendString));

					send_test_smsNum(SmsNumber[0], SendString);
					//	WritePhoneNumberFn();
					//	strcpy((char *)SendSMSString,(char *)smsbuffer);
				}
			}
		else if(strstr(smsbuffer,"RESET") != 0)
		{
		uart_send_count=0;
		uart_send_flag=2;
		//nMSettings.motor1onof = 1;
		sAPI_UartPrintf("\n\rmotor1 Reset \n\r");
        
		// WriteonofFile();
		// ReadonofFile();
        //extmotoron (PhoneNumber);
		}			
		else if(strstr(smsbuffer,"motor1on") != 0)
		{
		uart_send_count=0;
		uart_send_flag=1;
		nMSettings.motor1onof = 1;
		sAPI_UartPrintf("\n\rmotor1on \n\r");
        
		 WriteonofFile();
		 ReadonofFile();
        //extmotoron (PhoneNumber);
		}
		else if(strstr(smsbuffer,"motor1of") != 0)
		{
		uart_send_count=0;
		uart_send_flag=1;
		nMSettings.motor1onof = 0;
		sAPI_UartPrintf("\n\rmotor1of \n\r");
        
		 WriteonofFile();
		ReadonofFile();
        //extmotoron (PhoneNumber);
		}
		else if(strstr(smsbuffer,"motor2on") != 0)
		{
		uart_send_count=0;
		uart_send_flag=1;
		nMSettings.motor2onof = 1;
		sAPI_UartPrintf("\n\rmotor2on \n\r");
       
		 WriteonofFile();
		 ReadonofFile();
        //extmotoron (PhoneNumber);
		}
		else if(strstr(smsbuffer,"motor2of") != 0)
		{
		uart_send_count=0;
		uart_send_flag=1;
		nMSettings.motor2onof = 0;
		sAPI_UartPrintf("\n\rmotor2of \n\r");
       
        //extmotoron (PhoneNumber);
		WriteonofFile();
		 ReadonofFile();
		}
			else if (strstr(smsbuffer, "pumponof") != 0)
			{
				Pch1 = strtok((char *)smsbuffer, (char *)",");
				StrTokStrVer = 0;
				while (Pch1 != NULL)
				{
					strcpy(StrTokStr1[StrTokStrVer], Pch1);
					StrTokStrVer++;
					Pch1 = strtok(NULL, ",");
				}

				nMSettings.motor1onof = myatoi(StrTokStr1[1]);
				nMSettings.motor2onof = myatoi(StrTokStr1[2]);
				nMSettings.motor3onof = myatoi(StrTokStr1[3]);
				uart_send_count = 0;
				uart_send_flag = 1;

				sAPI_UartPrintf("\n\rmotor1on \n\r");

				WriteonofFile();
				ReadonofFile();
				// extmotoron (PhoneNumber);
			}

			else if (strstr(smsbuffer, "calon") != 0)
			{
				nMSettings.RelayControlOnCall = 1;
				sAPI_UartPrintf("\n\rCall on \n\r");
				sAPI_UartPrintf("\n\rSMS SEND NOW\n\r");
				//	 ThisSMSisNotPowerFault = 1;
				// NeedToCPBRSearchAgain = 0;
				// NumberOfSMSSend = 20;
				SendSmsToAll = 1;
				CallOnOfVer = 1;
				WriteSettingsFile();
				ReadFile();
			}
			else if (strstr(smsbuffer, "calof") != 0)
			{
				nMSettings.RelayControlOnCall = 0;
				sAPI_UartPrintf("\n\rCall of \n\r");
				sAPI_UartPrintf("\n\rSMS SEND NOW\n\r");
				//	 ThisSMSisNotPowerFault = 1;
				// NeedToCPBRSearchAgain = 0;
				SendSmsToAll = 1;
				// NumberOfSMSSend = 20;
				CallOnOfVer = 2;
				WriteSettingsFile();
				ReadFile();
			}
			else if (strstr(smsbuffer, "controlon") != 0)
			{
				controlon_flag = 1;
				WritectsetFile();
				ReadctsetFile();
			}
			else if (strstr(smsbuffer, "controlof") != 0)
			{
				controlon_flag = 0;
				WritectsetFile();
				ReadctsetFile();
			}
			else if (strstr(smsbuffer, "numext") != 0)
			{
				unsigned char Tp10;
				char SendString[160] = "";
				sAPI_UartPrintf("\n\rNumber information \n\r");
				sAPI_UartPrintf("\n\rSMS NUM SEND NOW\n\r");

				sprintf(SendString, "REG 4:%s\n", StoredPhoneNumber[4]);
				sprintf(SendString, "%sREG 5:%s\n", SendString, StoredPhoneNumber[5]);
				sprintf(SendString, "%sREG 6:%s\n", SendString, StoredPhoneNumber[6]);
				sprintf(SendString, "%sREG 7:%s\n", SendString, StoredPhoneNumber[7]);
				sprintf(SendString, "%sREG 8:%s\n", SendString, StoredPhoneNumber[8]);
				sprintf(SendString, "%sREG 9:%s\n", SendString, StoredPhoneNumber[9]);
				sprintf(SendString, "%sREG 10:%s\n", SendString, StoredPhoneNumber[10]);
				// sprintf(SendString,"REG 12:%s\n",StoredPhoneNumber[0]);

				// StopTimer(&timer);
				sprintf(BigSMS1, "V46");
				send_test_smsNum1(SmsNumber[0], SendString, BigSMS1);
				// timer.timeoutPeriod = 300;
				// timer.timerId = StartTimer(&timer);
			}
			else if (strstr(smsbuffer, "num") != 0)
			{
				unsigned char Tp10;
				char SendString[160] = "";
				sAPI_UartPrintf("\n\rNumber information \n\r");
				sAPI_UartPrintf("\n\rSMS NUM SEND NOW\n\r");
				sprintf(SendString, "SMSM:%s\n", StoredPhoneNumber[0]);
				sprintf(SendString, "%sREG 1:%s\n", SendString, StoredPhoneNumber[1]);
				sprintf(SendString, "%sREG 2:%s\n", SendString, StoredPhoneNumber[2]);
				sprintf(SendString, "%sREG 3:%s\n", SendString, StoredPhoneNumber[3]);
				sprintf(SendString, "%sREG S:%s\n", SendString, StoredPhoneNumber[11]);
				sprintf(SendString, "%sREG T:%s\n", SendString, StoredPhoneNumber[12]);
				sprintf(SendString, "%sREG B:%s\n", SendString, StoredPhoneNumber[13]);
				sprintf(SendString, "%sREG C:%s", SendString, StoredPhoneNumber[14]);
				sprintf(BigSMS1, "V45");
				send_test_smsNum1(SmsNumber[0], SendString, BigSMS1);
				// send_test_smsNum(SmsNumber[0],SendString);
			}

			/* else if(strstr(smsbuffer,"nameon") != 0)
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
			} */

			else if (strstr(smsbuffer, "gprson") != 0)
			{
				ReadonofFile();
				gprson = 1;
				wifion = 0;
				DeviceConfig.interface = GPRS;
				app_nvram_save(MCONFIG_AT_INDEX, (UINT8 *)&DeviceConfig, sizeof(DeviceConfig));
				// printParameter();

				WriteonofFile();
				ReadonofFile();
				// eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 7, "CONGPRS", EAT_NULL);
				sprintf(BigSMS1, "");
				send_networkonofview(PhoneNumber);
				//	//eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 7, "CONGPRS", EAT_NULL);
			}

			else if (strstr(smsbuffer, "gprsof") != 0)
			{
				ReadonofFile();
				gprson = 0;
				if (wifion == 1)
					DeviceConfig.interface = WIFI;
				else
					DeviceConfig.interface = DISBLED;
				app_nvram_save(MCONFIG_AT_INDEX, (UINT8 *)&DeviceConfig, sizeof(DeviceConfig));
				printParameter();

				WriteonofFile();
				ReadonofFile();
				// eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 6, "CONTCP", EAT_NULL);
				sprintf(BigSMS1, "");
				send_networkonofview(PhoneNumber);
				//	//eat_send_msg_to_user(EAT_USER_0, EAT_USER_2, EAT_FALSE, 6, "CONTCP", EAT_NULL);
			}
			else if (strstr(smsbuffer, "sappmodeon") != 0)
			{
				ReadonofFile();
				Appmodeon = 1;
				WriteonofFile();
				ReadonofFile();
				sprintf(BigSMS1, "");
				send_networkonofview(PhoneNumber);
			}

			else if (strstr(smsbuffer, "sappmodeof") != 0)
			{
				ReadonofFile();
				Appmodeon = 0;
				WriteonofFile();
				ReadonofFile();
				sprintf(BigSMS1, "");
				send_networkonofview(PhoneNumber);
			}
			/*else if(strstr(smsbuffer,"testt") != 0)
			{
			ReadFile();
			nMSettings.SMSOnOff = 1;
			STATE_SENDSMS = STATE_SMSON_SMS;
			//sprintf(smsbuffer,"SMS ON");
			WriteSettingsFile();
			SendSmsToAll = 1;
			//simcom_sms_send(smsReadCnfContent.number,smsbuffer,strlen(smsbuffer));
			ReadFile();
			}*/
			else if (strstr(smsbuffer, "getsimno") != 0)
			{
				sAPI_AtSend("AT+CNUM\r\n", strlen("AT+CNUM\r\n"));
			}
			else if (strstr(smsbuffer, "getimei") != 0)
			{
				//_getimei();
				NoAcceptSMS = 0;
				send_imei(SmsNumber[0]);
			}
			else if (strstr(smsbuffer, "masterno") != 0)
			{
				send_Smsno(SmsNumber[0]);
			}
			else if (strstr(smsbuffer, "vpumpset") != 0)
			{
				sAPI_UartPrintf("entry to vpumpset");
				f_Pump_Settings_view = 1;
			}
			else if (strstr(smsbuffer, "smson") != 0)
			{
				ReadFile();
				nMSettings.SMSOnOff = 1;
				//	STATE_SENDSMS = STATE_SMSON_SMS;
				//	sprintf(smsbuffer,"SMS ON");
				WriteSettingsFile();
				SendSmsToAll = 1;
				//	simcom_sms_send(SmsNumber[0],smsbuffer,strlen(smsbuffer));
				ReadFile();
			}

			else if (strstr(smsbuffer, "smsof") != 0)
			{
				char SendString[70] = "";
				ReadFile();
				nMSettings.SMSOnOff = 0;
				// STATE_SENDSMS = STATE_SMSON_SMS;
				sprintf(SendString, "SMS OFF\n\r");
				//		simcom_sms_send(SmsNumber[0], SendString, strlen(SendString));
				WriteSettingsFile();
				//	SendSmsToAll = 1;
				ReadFile();
			}
			else if (strstr((char *)smsbuffer, "csq") != 0)
			{
				char SendString[70] = "";
				float SignalStrength = 0;
				int sstrength = 0;
				SignalStrength = (float)(CSQ * 3.2258);
				if (SignalStrength > 100)
					SignalStrength = 100;
				else if (SignalStrength < 0)
					SignalStrength = 0;
				sstrength = SignalStrength;
				sprintf(SendString, "Signal Strength = %03d percent RSSI= %02d 4G AG4 ORO PCGv%s,Build:%s\n\r", sstrength, CSQ, Version, Build);
				simcom_sms_msg_send(SmsNumber[0], SendString, strlen(SendString), NULL);
				sprintf(buf, "\n\r smsbuffer %s\n\r", SendString);
				sAPI_UartPrintf(buf);
				
			}

		//	else if (strstr(smsbuffer, "dt,") != 0)
			 else if((strstr(smsbuffer,"dt,") != 0) || (strstr(smsbuffer,"DT,") != 0)) //dg_nsdk
			{
				int retv;
				long network_sec = 0, pos_difference = 0, neg_difference = 0;
				t_rtc timev;

				Pch1 = strtok((char *)smsbuffer, (char *)",");
				StrTokStrVer = 0;
				while (Pch1 != NULL)
				{
					strcpy(StrTokStr1[StrTokStrVer], Pch1);
					StrTokStrVer++;
					Pch1 = strtok(NULL, ",");
				}
				for (Tp = 0; Tp <= 3; Tp++)
				{
					sprintf(buf, "\n\rSt=%s  pos=%d\n\r", StrTokStr1[Tp], Tp);
					sAPI_UartPrintf(buf);
				}

				// sprintf(buf,"\n\rD&T:[%s]\n\r",StrTokStr1[1]);

				CurrentSec = (datetime.tm_hour * 3600) + (datetime.tm_min * 60) + datetime.tm_sec;
				sprintf(buf, "\n\rCurrentSec:%ld\n\r", CurrentSec);
				sAPI_UartPrintf(buf);

				sprintf(buf, "\n\rD&T:[%s]\n\r", smsbuffer);
				sAPI_UartPrintf(buf);
				//  [inserted successfully,ld01,dt,2024/05/22/12/20/51]
				//      sscanf(pTemp_1,",DT,%d/%d/%d/%d/%d/%d,+%*s",&yy,&mm,&dd,&hr,&min,&sec);
				//	sscanf((char* )smsbuffer, "dt,%d/%d/%d/%d/%d/%d,+%*s",&timeval.tm_year,&timeval.tm_mon,&timeval.tm_mday,&timeval.tm_hour,&timeval.tm_min,&timeval.tm_sec);
				sscanf((char *)StrTokStr1[3], "%d/%d/%d/%d/%d/%d,+%*s", &timeval.tm_year, &timeval.tm_mon, &timeval.tm_mday, &timeval.tm_hour, &timeval.tm_min, &timeval.tm_sec);

				sprintf(buf, "\n\r gettime: %d/%d/%d/ %d:%d:%d\r\n", timeval.tm_year, timeval.tm_mon, timeval.tm_mday, timeval.tm_hour, timeval.tm_min, timeval.tm_sec);
				sAPI_UartPrintf(buf);
				//	  timeval.tm_year = timeval.tm_year >= 70 ? (1900 + timeval.tm_year) : (2000 + timeval.tm_year);
			//	retv = sAPI_SetRealTimeClock(&timeval);  //dg_nsdk
				//	  eat_set_rtc(&datetime);
				//	sscanf((char* )smsbuffer,"dt,%2d,%2d,%2d,%2d,%2d,%2d",&timeval.tm_year,&timeval.tm_mon,&timeval.tm_mday,&timeval.tm_hour,&timeval.tm_min,&timeval.tm_sec);
				//	  timeval.tm_year = timeval.tm_year >= 70 ? (1900 + timeval.tm_year) : (2000 + timeval.tm_year);
				network_sec = timeval.tm_hour * 3600 + timeval.tm_min * 60 + timeval.tm_sec;
				sprintf(buf, "\n\rnetwork_sec:%ld\n\r", network_sec);
				sAPI_UartPrintf(buf);

				if (network_sec != 0)
				{
					if (network_sec > CurrentSec)
					{
						pos_difference = network_sec - CurrentSec;
						neg_difference = 0;
					}
					if (CurrentSec > network_sec)
					{
						neg_difference = CurrentSec - network_sec;
						pos_difference = 0;
					}
				}
				sprintf(buf, "\n\r CurrentSec %ld network_sec %ld pos_difference %ld neg_difference %ld", CurrentSec, network_sec, pos_difference, neg_difference);
				sAPI_UartPrintf(buf);
				if (pos_difference >= 300 || neg_difference >= 300)
				{
					retv = sAPI_SetRealTimeClock(&timeval);
					date_time_send_flag = 1;
					if (retv < 0)
						sAPI_UartPrintf("\r\n {set time failed 1}!\r\n");
					else
					{
						NTPokFlag = 1;
						sAPI_UartPrintf("\r\n [set time successed!]\r\n");
						sAPI_GetRealTimeClock(&timev);
						sprintf(buf, "\r\n Get RTC time: %d/%02d/%02d,%02d:%02d:%02d \r\n", timev.tm_year, timev.tm_mon, timev.tm_mday,
								timev.tm_hour, timev.tm_min, timev.tm_sec);
						sAPI_UartPrintf(buf);

						//	STATE_SENDSMS=STATE_DATETIME_SMS;
						//	SendSmsToAll = 1;
					}
				}
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
				sAPI_Debug("GetLocalTime:%d/%d/%d/ %d:%d:%d\r\n",datetime.tm_year, datetime.tm_mon, datetime.tm_mday,datetime.tm_hour, datetime.tm_min, datetime.tm_sec);
				eat_set_rtc(&datetime);
				//Ql_sprintf(textBuf,"Ql_SetLocalTime()=%d\r\n",ret);
				//Ql_SendToUart(ql_uart_port1,(UINT8 *)textBuf,Ql_strlen(textBuf));
				STATE_SENDSMS=STATE_DATETIME_SMS;
				SendSmsToAll = 1;

				}*/
		}
		else
		{
			sAPI_Debug("Number not match\n");
		}
	}
}

void timer1_init(void)
{
	SC_STATUS status;
	//_getimei();
	app_hw_gpio_init();
#ifdef USE_FLAG
	sAPI_FlagCreate(&g_flg1);
	sAPI_UartPrintf("\n\rUSE_FLAG\n\r");
#endif
	//  status = sAPI_TaskCreate(&timer1Processer, timer1ProcesserStack,10*1024, 140, "timer1",sAPP_Timer1,(void *)0);  //dg_nsdk
	if (NULL != timer1Processer)
    {
		 sAPI_UartPrintf("%s  line %d\n", __func__,__LINE__);
        return;
    }
//	status = sAPI_TaskCreate(&timer1Processer, timer1ProcesserStack, 10 * 1024, 140, "timer1", sAPP_Timer1, (void *)0); //25 * 1024
//	status = sAPI_TaskCreate(&timer1Processer, timer1ProcesserStack, 20 * 1024, 140, "timer1", sAPP_Timer1, (void *)0);
	status = sAPI_TaskCreate(&timer1Processer, timer1ProcesserStack, 10 * 1024, 140, "timer1", sAPP_Timer1, (void *)0);
	sAPI_Debug("timer1_init[%d]", status);
	sAPI_UartPrintf("%s Task creation completed!!\n", __func__);
	if (SC_SUCCESS != status)
	{
		sAPI_UartPrintf("Task create fail,status ");
	}
}

void onesec_timer(void)
{
	SC_STATUS status;
#ifdef USE_FLAG
	sAPI_FlagCreate(&g_flg2);
#endif
	if (NULL != onesectimerProcesser)
    {
		 sAPI_UartPrintf("%s  line %d\n", __func__,__LINE__);
        return;
    }
	status = sAPI_TaskCreate(&onesectimerProcesser, onesecProcesserStack, 1 * 1024, 150, "onesectimer", sAPP_Timer2, (void *)0);
	sAPI_UartPrintf("ONESEC TIMER");
	sAPI_UartPrintf("%s Task creation completed!!\n", __func__);
	if (SC_SUCCESS != status)
	{
		sAPI_UartPrintf("Task create fail,status");
	}
}



/* void UartCBFunc1(SC_Uart_Port_Number portNumber, void *para)   //subash1
{	  
		SIM_MSG_T optionMsg ={0,0,0,NULL};
		int ControllerDataCounter = 0;
	    char *ControllerString = NULL;
		
		int readLen = 0;
		char uartData[UART_RX_BUFFER_SIZE];
  //      char *uartData = sAPI_Malloc(UART_RX_BUFFER_SIZE);
        SIM_MSG_T uartMsg = {0,0,0,NULL};

        readLen = sAPI_UartRead(portNumber, (UINT8 *)uartData, UART_RX_BUFFER_SIZE);	 
	//	memset(uart_read_buf,0,300);
		memset(&uart_read_buf, 0, 200);
		memcpy(uart_read_buf,uartData,40);
		
//		sAPI_Free(uartData); */		 
		 
/*  void UartCBFunc1(SC_Uart_Port_Number portNumber, void *para)
{
    int readLen=0,copyLen=0;
    static char uartData[UART_RX_BUFFER_SIZE];   // <--- static = no stack usage
	int ControllerDataCounter = 0;
	    char *ControllerString = NULL;
    readLen = sAPI_UartRead(portNumber, (UINT8 *)uartData, UART_RX_BUFFER_SIZE);

    if (readLen > 0)
    {
        copyLen = (readLen > sizeof(uart_read_buf)) ? sizeof(uart_read_buf) : readLen;

        memset(uart_read_buf, 0, sizeof(uart_read_buf));
        memcpy(uart_read_buf, uartData, copyLen);
    }
 sAPI_UartPrintf("\n\r readLen:[%d], copyLen[%d]",readLen,copyLen);
    // no malloc / no free / minimal local vars

      
	   if(nMSettings.ndebugonof==1)		
		 sAPI_UartPrintf("\n\rUART:[%s], UART_RX_BUFFER_SIZE %d",uart_read_buf,UART_RX_BUFFER_SIZE);		   
			//	sAPI_UartPrintf("I String = %s",(char*)uart_read_buf); */
        		 
 void UartCBFunc1(SC_Uart_Port_Number portNumber, void *para)
{
    int readLen = 0;
	char *uart2Data = sAPI_Malloc(UART_RX_BUFFER_SIZE);
	    
    SIM_MSG_T uart1Msg = {0,0,0,NULL};

    readLen = sAPI_UartRead(portNumber, (UINT8 *)uart2Data, UART_RX_BUFFER_SIZE);
    sAPI_UartPrintf("%s, portNumber is %d, readLen[%d].",__func__, portNumber, readLen);

 
    // uart1Msg.msg_id = SRV_UART;
    uart1Msg.arg2 = readLen;
    uart1Msg.arg3 = uart2Data;
    send_uart1_data(uart1Msg);
	No_comm_flag=0;
	No_comm_secs=0;
	if(nMSettings.ndebugonof==1)
	{
	sprintf(buf,"uart2Data:%s",uart2Data);
	sAPI_UartPrintf(buf); 
	}

    sAPI_Free(uart2Data);
    return;
}
void send_uart1_data(SIM_MSG_T uart1Msg)
{
	SC_STATUS status = SC_SUCCESS;
    SIM_MSG_T optionMsg = {0,0,0,NULL};
    optionMsg.msg_id = uart1Msg.msg_id;
    optionMsg.arg2 = uart1Msg.arg2;

    optionMsg.arg3 = sAPI_Malloc(optionMsg.arg2+1);
    memset(optionMsg.arg3, 0, optionMsg.arg2+1);
    memcpy(optionMsg.arg3, uart1Msg.arg3, uart1Msg.arg2);

    status = sAPI_MsgQSend(gUart_msgq,&optionMsg);
    if(status != SC_SUCCESS)
    {
        sAPI_Free(optionMsg.arg3);
        sAPI_UartPrintf("send msg error,status = [%d]" );
    }
	else
		Queue_count++;
	sAPI_UartPrintf("\n\r Queue_count %d line %d",Queue_count,__LINE__);
} 


void sTask_UartRxProcesser(void *ptr)
 {  
    char *ControllerString = NULL;
	int ControllerDataCounter = 0,i=0;
 //   char *uart2Data = sAPI_Malloc(UART_RX_BUFFER_SIZE);
	SIM_MSG_T optionMsg = {0,0,0,NULL};
	char delims[] = ",";
    char *result = NULL;
    UINT8 user_msgd[200];
  
	memset(&uart_read_buf,0,200);
		 

while(1) 
	{
	//	sAPI_MsgQRecv(gUart_msgq,&optionMsg,SC_SUSPEND);
	//	memcpy(uart_read_buf,optionMsg.arg3,strlen(optionMsg.arg3));
	sAPI_MsgQRecv(gUart_msgq,&optionMsg,SC_SUSPEND);
	if (optionMsg.arg3 && optionMsg.arg2 > 0) {
		Queue_count--;
	sAPI_UartPrintf("\n\r Queue_count %d line %d",Queue_count,__LINE__);
    memcpy(uart_read_buf, optionMsg.arg3, optionMsg.arg2);
    uart_read_buf[optionMsg.arg2] = '\0';   // critical fix
	}
		if(nMSettings.ndebugonof==1)
		{
		sprintf(buf,"UART2:%s\n\r",uart_read_buf);
		sAPI_UartPrintf(buf);
		}
		sAPI_Free(optionMsg.arg3);
 
		   if(uart_read_buf[0] == '$')
				{  			     
						//$S,S,00000000,00000000,00000000,00000000,00000000,00000000
						// S :- Voltage and Current
						//K :- Keys
	 					ControllerString+=ControllerDataCounter;
						ControllerString=(char*)uart_read_buf;
						ControllerDataCounter+=MCONFIG_AT_CMD_MAX_LEN;
						if(ControllerDataCounter>=54)
						{
							 // sprintf(buf,"I got Controller String = %s",ControllerString);
							 // 	sAPI_UartPrintf(buf);
							if(ControllerString[3] == 'C')
							{
							 	// sAPI_UartPrintf("\n\rFound Voltage String");
							//	GetVoltage(ControllerString);
								 
							}
							else if(ControllerString[3] == 'D')
							{
							 	// sAPI_UartPrintf("\n\rFound Current String");
							//	GetCurrent(ControllerString);
								 
							}
								else if(ControllerString[3] == 'T')
							{
								// sAPI_UartPrintf("Found Key String");	
							//	GetKey(ControllerString);
								
							}
							
							
						}
						else if(uart_read_buf[0] == '$' && uart_read_buf[2] == 30)
					{
						sgetflag_1=1;
						sprintf(payload_buf,"Set Serial :%d",uart_read_buf[2]);   
						sAPI_UartPrintf(payload_buf);
						memset(Node_buf,NULL,sizeof(Node_buf));
						memset(Buff,NULL,sizeof(Buff));
						Buff[0]='$';
						Buff[1]=':';
						if(uart_read_buf[2] == 30)
						{
							for(i=1;i<uart_read_buf[1]-2;i++)
							{
								if(i<4 || i>9)
								sprintf(Buff,"%s%d:",Buff,uart_read_buf[i]);
								else	
								sprintf(Buff,"%s%02X:",Buff,uart_read_buf[i]);								
							}
						}
						// else
						// {
						// 	for(int i=1;i<uart_read_buf[1]-2;i++)
						// 	{
						// 		sprintf(Buff,"%s%d:",Buff,uart_read_buf[i]);
						// 	}
						// }
						int st_len=strlen(Buff);
						CRC_Total=0;
							for(i=0;i<=st_len;i++)
								CRC_Total+=Buff[i];
							CRC_val=CRC_Total%256;
							if(CRC_val==0)
								CRC_val=1;
							sprintf(Node_buf,"%s%03d:\r",Buff,CRC_val);
							sprintf(payload_buf,"Set Serial :%s",Node_buf);   
							sAPI_UartPrintf(payload_buf);
							memset(Buff,NULL,sizeof(Buff));
							if(Nooftcprecvd<=49)		
							{  
								memset(&TCPwifiStrNumber[Nooftcprecvd].TCPWifigprsstr,NULL,500);
								//sprintf(TCPwifiStrNumber[Nooftcprecvd].TCPWifigprsstr,"%s",Node_buf);
								memcpy(TCPwifiStrNumber[Nooftcprecvd].TCPWifigprsstr,Node_buf,strlen(Node_buf));
								
								Nooftcprecvd++;  
							}
							else
							{
								Nooftcpprocessed=Nooftcprecvd=0;
								sAPI_UartPrintf("SET SERIAL RESART");
								//sAPI_SysReset();//Praveen
							}
							memset(Node_buf,NULL,sizeof(Node_buf));
					}
						if(uart_read_buf[0] == '$' && uart_read_buf[2] == 37)
						{
							int DLC=uart_read_buf[1];
							for(i=2;i<DLC-1;i++)
							{
								payload_buf[i-2]=uart_read_buf[i];
							}
							payload_buf[DLC-2]=0;
							RecValveOnOff.ValveStausFlag=payload_buf[2];
							RecValveOnOff.ValveNo=payload_buf[3];
							RecValveOnOff.BatVolt=payload_buf[4]/10;
							RecValveOnOff.SolarVolt=payload_buf[5];
							
							if(payload_buf[6]==1)
							{
								RecValveOnOff.ADC1=0;
							}
							else if(payload_buf[6]==2)
							{
								RecValveOnOff.ADC1=0;
								RecValveOnOff.ADC1=payload_buf[8];
							}
							else
							{
								RecValveOnOff.ADC1=0;
								RecValveOnOff.ADC1=payload_buf[7]*256;
								RecValveOnOff.ADC1+=payload_buf[8];
							}
							if(payload_buf[9]==1)
							{
								RecValveOnOff.ADC2=0;
							}
							else if(payload_buf[9]==2)
							{
								RecValveOnOff.ADC2=0;
								RecValveOnOff.ADC2=payload_buf[11];
							}
							else
							{
								RecValveOnOff.ADC2=0;
								RecValveOnOff.ADC2=payload_buf[10]*256;
								RecValveOnOff.ADC2+=payload_buf[11];
							}
							if(payload_buf[12]==1)
							{
								RecValveOnOff.ADC3=0;
							}
							else if(payload_buf[12]==2)
							{
								RecValveOnOff.ADC3=0;
								RecValveOnOff.ADC3=payload_buf[14];
							}
							else
							{
								RecValveOnOff.ADC3=0;
								RecValveOnOff.ADC3=payload_buf[13]*256;
								RecValveOnOff.ADC3+=payload_buf[14];
							}
							RecValveOnOff.ControlFLag=payload_buf[15];
							RecValveOnOff.MainValveStatus=payload_buf[16];

							
							sprintf(buf,"ValveStausFlag:%d,ValveNo:%d,SolarVolt:%d,BatVolt:%f,ADC1:%d,ADC2:%d,ADC3:%d,ControlFLag:%d,MainValveStatus:%d",RecValveOnOff.ValveStausFlag,RecValveOnOff.ValveNo,RecValveOnOff.SolarVolt,RecValveOnOff.BatVolt,RecValveOnOff.ADC1,RecValveOnOff.ADC2,RecValveOnOff.ADC3,RecValveOnOff.ControlFLag,RecValveOnOff.MainValveStatus);
							sAPI_UartPrintf(buf);
						}
						if(uart_read_buf[0] == '$' && uart_read_buf[1] == 'i' && uart_read_buf[2] == 'M')
						{
							 
						   Pch1 = strtok((char *)uart_read_buf, (char *)"," );
							int StrTokStrVer =0 ;
							unsigned char Tempbuf_IMEI[20];
							while( Pch1 != NULL )
							{
							strcpy(StrTokStr1[StrTokStrVer],Pch1);
							StrTokStrVer++;
							Pch1 = strtok( NULL, "," );
							}
							memset(my_eeprom_ID,0,sizeof(my_eeprom_ID));
							my_eeprom_ID[0]=myatoi(StrTokStr1[1]);
							my_eeprom_ID[1]=myatoi(StrTokStr1[2]);
							my_eeprom_ID[2]=myatoi(StrTokStr1[3]);
							my_eeprom_ID[3]=myatoi(StrTokStr1[4]);
							my_eeprom_ID[4]=myatoi(StrTokStr1[5]);
							my_eeprom_ID[5]=myatoi(StrTokStr1[6]);
								
							uart_data_comm_flag=0;
						//sprintf(buf,"$<BCK>\n\r");
							//sAPI_UartWrite(SC_UART,(UINT8*)buf,strlen(buf));
							//sAPI_UartPrintf(buf);
							
						memset(Tempbuf_IMEI,NULL,sizeof(Tempbuf_IMEI)); 
						sprintf(Tempbuf_IMEI,"%02X%02X%02X%02X%02X%02X",my_eeprom_ID[0],my_eeprom_ID[1],my_eeprom_ID[2],my_eeprom_ID[3],my_eeprom_ID[4],my_eeprom_ID[5]);
					//	sAPI_UartPrintf("IMEI : %s",my_eeprom_ID);
					//	sAPI_UartPrintf("\n IMEI=");
						sAPI_UartPrintf(Tempbuf_IMEI);  //sAPI_UartPrintf(buf);
						sprintf(buf,"\n Tempbuf_IMEI:[%s] \n",Tempbuf_IMEI);
						sAPI_UartPrintf(buf);
						sprintf(buf,"\n IMEI:[%s] \n",IMEI);
						sAPI_UartPrintf(buf);
						sprintf(buf,"\n line :%d\n",__LINE__);
						sAPI_UartPrintf(buf);
						sprintf(buf,"\n line :%d, IMEI:[%s], strlen(IMEI):%d \n",__LINE__,IMEI,strlen(IMEI));
						sAPI_UartPrintf(buf);
						if(Resend_IMEI_flag==1)
						Resend_IMEI_count++;
						if(strcmp(Tempbuf_IMEI,IMEI)!=0)
						{
							Resend_IMEI_flag=1;
						//	Resend_IMEI_count++;
							if(Resend_IMEI_count>=2)
							{
								IMEI_req_flag=1;
							Resend_IMEI_flag=0;
							Resend_IMEI_count=0;
							sprintf(IMEI,"%02X%02X%02X%02X%02X%02X",my_eeprom_ID[0],my_eeprom_ID[1],my_eeprom_ID[2],my_eeprom_ID[3],my_eeprom_ID[4],my_eeprom_ID[5]);	
						//	MqttInitStatus=0;
							sprintf(buf,"\n {%03d,%03d,%03d,%03d,%03d,%03d} \n",my_eeprom_ID[0],my_eeprom_ID[1],my_eeprom_ID[2],my_eeprom_ID[3],my_eeprom_ID[4],my_eeprom_ID[5]);
							sAPI_UartPrintf(buf);
						//	WritectsetFile();
						//	WriteEpromidFile();
							g_new_MAC_flag=1;
							sprintf(buf,"\n line :%d ,	IMEI{%s} \n",__LINE__,IMEI);
							sAPI_UartPrintf(buf);
							MQTT_Reconnect();
							}
						
						}
						else 
						{
							IMEI_req_flag=1;
						Resend_IMEI_flag=0;
						Resend_IMEI_count=0;
						sprintf(buf,"\n\r  Resend_IMEI_flag %d Resend_IMEI_count %d line %d \n\r",Resend_IMEI_flag,Resend_IMEI_count,__LINE__);
						sAPI_UartPrintf(buf);
						}
						sprintf(buf,"\n IMEI :%s--%s\n",IMEI,Tempbuf_IMEI);
						sAPI_UartPrintf(buf);
						sprintf(buf,"\n Eprom : %02X,%02X,%02X,%02X,%02X,%02X \n",my_eeprom_ID[0],my_eeprom_ID[1],my_eeprom_ID[2],my_eeprom_ID[3],my_eeprom_ID[4],my_eeprom_ID[5]);
						sAPI_UartPrintf(buf);
						
						
						}
						
						/* if(uart_read_buf[0] == '$' && uart_read_buf[1] == 'i' && uart_read_buf[2] == 'M')
						{
							 
						   Pch1 = strtok((char *)uart_read_buf, (char *)"," );
							int StrTokStrVer =0 ;
							char Tempbuf_IMEI[20];
							while( Pch1 != NULL )
							{
							strcpy(StrTokStr1[StrTokStrVer],Pch1);
							StrTokStrVer++;
							Pch1 = strtok( NULL, "," );
							}
							memset(my_eeprom_ID,0,sizeof(my_eeprom_ID));
							my_eeprom_ID[0]=myatoi(StrTokStr1[1]);
							my_eeprom_ID[1]=myatoi(StrTokStr1[2]);
							my_eeprom_ID[2]=myatoi(StrTokStr1[3]);
							my_eeprom_ID[3]=myatoi(StrTokStr1[4]);
							my_eeprom_ID[4]=myatoi(StrTokStr1[5]);
							my_eeprom_ID[5]=myatoi(StrTokStr1[6]);
						IMEI_req_flag=1;	
						sprintf(buf,"$<BCK>\n\r");
							sAPI_UartWrite(SC_UART,(UINT8*)buf,strlen(buf));
							sAPI_UartPrintf(buf);
							
						memset(Tempbuf_IMEI,NULL,sizeof(Tempbuf_IMEI)); 
						sprintf(Tempbuf_IMEI,"%02X%02X%02X%02X%02X%02X",my_eeprom_ID[0],my_eeprom_ID[1],my_eeprom_ID[2],my_eeprom_ID[3],my_eeprom_ID[4],my_eeprom_ID[5]);
					//	sAPI_UartPrintf("IMEI : %s",my_eeprom_ID);
					//	sAPI_UartPrintf("\n IMEI=");
						sAPI_UartPrintf(Tempbuf_IMEI);
						sprintf(buf,"\n Tempbuf_IMEI:[%s] \n",Tempbuf_IMEI);
						sAPI_UartPrintf(buf);
						sprintf(buf,"\n IMEI:[%s] \n",IMEI);
						sAPI_UartPrintf(buf);
						sprintf(buf,"\n line :%d\n",__LINE__);
						sAPI_UartPrintf(buf);
						if(strcmp(Tempbuf_IMEI,IMEI)!=0)
						{
						sprintf(IMEI,"%02X%02X%02X%02X%02X%02X",my_eeprom_ID[0],my_eeprom_ID[1],my_eeprom_ID[2],my_eeprom_ID[3],my_eeprom_ID[4],my_eeprom_ID[5]);	
					//	MqttInitStatus=0;
						sprintf(buf,"\n {%03d,%03d,%03d,%03d,%03d,%03d} \n",my_eeprom_ID[0],my_eeprom_ID[1],my_eeprom_ID[2],my_eeprom_ID[3],my_eeprom_ID[4],my_eeprom_ID[5]);
						sAPI_UartPrintf(buf);
						WritectsetFile();
						sprintf(buf,"\n line :%d ,	IMEI{%s} \n",__LINE__,IMEI);
						sAPI_UartPrintf(buf);
						MQTT_Reconnect();
						}
						sprintf(buf,"\n my_eeprom_ID :%s\n",my_eeprom_ID);
						sAPI_UartPrintf(buf);
						sprintf(buf,"\n IMEI : %02X,%02X,%02X,%02X,%02X,%02X \n",my_eeprom_ID[0],my_eeprom_ID[1],my_eeprom_ID[2],my_eeprom_ID[3],my_eeprom_ID[4],my_eeprom_ID[5]);
						sAPI_UartPrintf(buf);
						
						
						} */
						
						/* if(uart_read_buf[0] == '$' && uart_read_buf[1] == 'H')
						{
							 
						   Pch1 = strtok((char *)uart_read_buf, (char *)"," );
							int StrTokStrVer =0 ;
							char Tempbuf_IMEI[20];
							while( Pch1 != NULL )
							{
							strcpy(StrTokStr1[StrTokStrVer],Pch1);
							StrTokStrVer++;
							Pch1 = strtok( NULL, "," );
							}
							WorkingOn3Phas = myatoi(StrTokStr1[1]);
							sprintf(buf,"\n\r WorkingOn3Phas = %01d\n\r",WorkingOn3Phas);
							sAPI_UartPrintf(buf);
						} */
					/*	 if(uart_read_buf[0] == '$' && uart_read_buf[1] == 'M' && uart_read_buf[2] == '1' && uart_read_buf[4] == 'P')
						{
							Prev_M1_state=(uart_read_buf[6]-'0')*10+(uart_read_buf[7]-'0');
							Prev_autokey=(uart_read_buf[9]-'0')*10+(uart_read_buf[10]-'0');
							sprintf(buf,"$<ACK>\n\r");
							sAPI_UartWrite(SC_UART,(UINT8*)buf,strlen(buf));
							sAPI_UartPrintf(buf);
							sprintf(buf,"\n\r Prev_M1_state = %02d, Prev_autokey=%02d\n\r",Prev_M1_state,Prev_autokey);
							sAPI_UartPrintf(buf);
						} */
						 if(uart_read_buf[0] == '$' && uart_read_buf[1] == 'M' && uart_read_buf[2] == '1' && uart_read_buf[4] == 'N')
						{
							int StrTokStrVer = 0;
							
							
							Pch1 = strtok((char *)uart_read_buf, (char *)"," );
							StrTokStrVer =0 ;
							while( Pch1 != NULL )
							{
							strcpy(StrTokStr1[StrTokStrVer],Pch1);
							StrTokStrVer++;
							Pch1 = strtok( NULL, "," );
							}
							set_value=0,actual_value=0;
							number_enu_data[0]=myatoi(StrTokStr1[2]);
							
						//	number_enu_data=(uart_read_buf[6]-'0')*10+(uart_read_buf[7]-'0');
							//sprintf(buf,"$<ACK>\n\r");
							//sAPI_UartWrite(SC_UART,(UINT8*)buf,strlen(buf));
							//sAPI_UartPrintf(buf);
							sprintf(buf,"$<CCK>\n\r");
							sAPI_UartWrite(SC_UART,(UINT8*)buf,strlen(buf));
							sAPI_UartPrintf(buf);
							sprintf(buf,"\n\r prev_number_enu_data[0] = %02d, number_enu_data[0] : %02d\n\r",prev_number_enu_data[0],number_enu_data[0]);
							sAPI_UartPrintf(buf);
						set_value2=0,actual_value2=0;
						if(number_enu_data[0]>=1 && prev_number_enu_data[0] != number_enu_data[0])
						{	
							livedataflag=1;
							SendSmsToAll = 1;
							sms_onflag = 1;
							Time_flag=1;
							Time_Counter=0;
							trip_flag =0; sms_offlag =0;
							
							
							act_rem_delmin[0]=act_POnMin[0];
							act_rem_delsec[0]=act_POnSec[0];
							if(number_enu_data[0]==0)
							nSTATE_MOTOR1_ON_SMS=STATE_MOTOR1_DUMMY;	
							else if(number_enu_data[0]==1)
							{
								nSTATE_MOTOR1_ON_SMS=STATE_MOTOR1_ON_ONDELAY;
								//WorkingOn3Phas=myatoi(StrTokStr1[3]);
							}
							else if(number_enu_data[0]==2)
							{
								nSTATE_MOTOR1_ON_SMS=STATE_MOTOR1_ON_RTC1;
								//WorkingOn3Phas=myatoi(StrTokStr1[3]);
							}
							else if(number_enu_data[0]==3)
							{
								nSTATE_MOTOR1_ON_SMS=STATE_MOTOR1_ON_RTC2;
								//WorkingOn3Phas=myatoi(StrTokStr1[3]);
							}
							else if(number_enu_data[0]==4)
							{
								nSTATE_MOTOR1_ON_SMS=STATE_MOTOR1_ON_RTC3;
								//WorkingOn3Phas=myatoi(StrTokStr1[3]);
							}
							else if(number_enu_data[0]==5)
							{	
								nSTATE_MOTOR1_ON_SMS=STATE_MOTOR1_ON_RTC4;
								//WorkingOn3Phas=myatoi(StrTokStr1[3]);
							}
							else if(number_enu_data[0]==6)
							{
								nSTATE_MOTOR1_ON_SMS=STATE_MOTOR1_ON_RTC5;
								//WorkingOn3Phas=myatoi(StrTokStr1[3]);
							}
							else if(number_enu_data[0]==7)
							nSTATE_MOTOR1_ON_SMS=STATE_MOTOR1_ON_RTC6;
							else if(number_enu_data[0]==8)
							nSTATE_MOTOR1_ON_SMS=STATE_MOTOR1_ON_UPPERTANK;
							else if(number_enu_data[0]==9)
							nSTATE_MOTOR1_ON_SMS=STATE_MOTOR1_ON_LOWERTANK;
							else if(number_enu_data[0]==10)
							nSTATE_MOTOR1_ON_SMS=STATE_MOTOR1_ON_OLVOLTAGE;
							else if(number_enu_data[0]==11)
							{
								nSTATE_MOTOR1_ON_SMS=STATE_MOTOR1_ON_SWITCH_SMS;
								//WorkingOn3Phas=myatoi(StrTokStr1[3]);
								prev_onof_M1=myatoi(StrTokStr1[6]);
							Prev_M1_state=myatoi(StrTokStr1[7]);
							tripflag[0]=myatoi(StrTokStr1[8]);
							}
							else if(number_enu_data[0]==12)
							nSTATE_MOTOR1_ON_SMS=STATE_MOTOR1_ON_TARGET;
							else if(number_enu_data[0]==13)
							nSTATE_MOTOR1_ON_SMS=STATE_MOTOR1_ON_HIGHVOLTAGE;
							else if(number_enu_data[0]==14)
							nSTATE_MOTOR1_ON_SMS=STATE_MOTOR1_ON_LOWVOLTAGE;
							else if(number_enu_data[0]==15)
							nSTATE_MOTOR1_ON_SMS=STATE_MOTOR1_ON_SPP;
							else if(number_enu_data[0]==16)
							nSTATE_MOTOR1_ON_SMS=STATE_MOTOR1_ON_RESTARTTIMER;
							else if(number_enu_data[0]==17)
							{
								nSTATE_MOTOR1_ON_SMS=STATE_MOTOR1_ON_LEVELSCAN_UPPERTANK;
								//WorkingOn3Phas=uart_read_buf[9]-'0';
								level_percent=(uart_read_buf[10]-'0')*10+(uart_read_buf[11]-'0');
							}
							else if(number_enu_data[0]==18)
							{
								nSTATE_MOTOR1_ON_SMS=STATE_MOTOR1_ON_LEVELSCAN_LOWERTANK;
								//WorkingOn3Phas=uart_read_buf[9]-'0';
								level_percent=(uart_read_buf[11]-'0')*10+(uart_read_buf[12]-'0');
							}
							else if(number_enu_data[0]==19)
							nSTATE_MOTOR1_ON_SMS=STATE_NLIGHT_RTCON_SMS;
						//	else
							else
							nSTATE_MOTOR1_ON_SMS=STATE_MOTOR1_ON_DEFAULT;
							prev_number_enu_data[0] = number_enu_data[0];
							number_enu_data[0] =0;
						}
							
					}
						
					if(uart_read_buf[0] == '$' && uart_read_buf[1] == 'M' && uart_read_buf[2] == '1' && uart_read_buf[4] == 'F')
					{
								int StrTokStrVer = 0;
							//	char uart_buf_tmp[200];
							//memcpy(uart_buf_tmp,uart_read_buf,sizeof(uart_read_buf));	
							//sprintf(buf,"uart_buf_tmp : %s",uart_buf_tmp);
							//sAPI_UartPrintf(buf);
						//	livedataflag=1;
							Time_flag =0;
							Pch1 = strtok((char *)uart_read_buf, (char *)"," );
							StrTokStrVer =0 ;
							while( Pch1 != NULL )
							{
							strcpy(StrTokStr1[StrTokStrVer],Pch1);
							StrTokStrVer++;
							Pch1 = strtok( NULL, "," );
							}
							
							number_enu_data[0]=myatoi(StrTokStr1[2]);
							
							sprintf(buf,"$<CCK>\n\r");
							sAPI_UartWrite(SC_UART,(UINT8*)buf,strlen(buf));
							sAPI_UartPrintf(buf);
		//	}						
						//	number_enu_data=(uart_read_buf[6]-'0')*10+(uart_read_buf[7]-'0');
							//sprintf(buf,"$<ACK>\n\r");
							//sAPI_UartWrite(SC_UART,(UINT8*)buf,strlen(buf));
							//sAPI_UartPrintf(buf);
							sprintf(buf,"number_enu_data : %02d prev_number_enu_data[0] %02d ",number_enu_data[0],prev_number_enu_data[0]);
							sAPI_UartPrintf(buf);
				
						if(number_enu_data[0]>=1 && (prev_number_enu_data[0] != number_enu_data[0]))//subash doubte to modify
						{
							actual_value=0;
							set_value=0;
							phase_number=0;
							SendSmsToAll = 1;
							sms_onflag = 0;
							
							
							if(number_enu_data[0]==0)
							nSTATE_MOTOR1_SMS=STATE_NO_MOTOR1_SMS;	
							else if(number_enu_data[0]==1)
							nSTATE_MOTOR1_SMS=STATE_MOTOR1ON_SMS;
							else if(number_enu_data[0]==2)
							{
								nSTATE_MOTOR1_SMS=STATE_MOTOR1OF_SMS;
								sms_offlag = 1;
							}
							else if(number_enu_data[0]==3)
							{
								nSTATE_MOTOR1_SMS=STATE_MOTOR1_STARTER_TRIP_SMS;
								OverAllStarterTrip1=myatoi(StrTokStr1[3]);
								trip_flag =1;
							}
							else if(number_enu_data[0]==4)
							{
								nSTATE_MOTOR1_SMS=STATE_MOTOR1_UPPERTANK_TRIP_SMS;
								trip_flag =1;
							}
							else if(number_enu_data[0]==5)
							{
								nSTATE_MOTOR1_SMS=STATE_MOTOR1_LOWERTANK_TRIP_SMS;
								trip_flag =1;
							}
							else if(number_enu_data[0]==6)
							{
								
								TripCurrent=0.0;trip_flag =1;																			
								nSTATE_MOTOR1_SMS=STATE_MOTOR1_TRIP_OVERLOAD_SMS;
								WhichPhaseHaveingProblem=myatoi(StrTokStr1[3]);
								phase_number=WhichPhaseHaveingProblem;
								//WorkingOn3Phas=myatoi(StrTokStr1[6]);
								if(WorkingOn3Phas == 1)
								{
								s_nTimerSettings.m_OlAmpsIII[1]=myatof(StrTokStr1[4]);
								set_value=s_nTimerSettings.m_OlAmpsIII[1];
								}
								else 
								{
								s_nTimerSettings.m_OlAmpsII[1]=myatof(StrTokStr1[4]);
								set_value=s_nTimerSettings.m_OlAmpsII[1];
								}
								TripCurrent=myatof(StrTokStr1[5]);
								sprintf(buf,"\n\rfor checking: olamps=%c%c%c%c, TripCurrent=%c%c%c%c\n\r",uart_read_buf[11],uart_read_buf[12],uart_read_buf[13],uart_read_buf[14],
																												uart_read_buf[16],uart_read_buf[17],uart_read_buf[18],uart_read_buf[19]);
								sAPI_UartPrintf(buf);	//subash commented
								actual_value=TripCurrent;
							
							phase_number=0;
							}
							else if(number_enu_data[0]==7)
							{
								
								TripCurrent=0.0;trip_flag =1;
								nSTATE_MOTOR1_SMS=STATE_MOTOR1_DRYRUN_TRIP_SMS;
								WhichPhaseHaveingProblem=myatoi(StrTokStr1[3]);
								//WorkingOn3Phas=myatoi(StrTokStr1[6]);
								if(WorkingOn3Phas == 1)
								{
								s_nTimerSettings.m_DrAmpsIII[1]=myatof(StrTokStr1[4]);
								set_value=s_nTimerSettings.m_DrAmpsIII[1];
								}
								else 
								{
								s_nTimerSettings.m_DrAmpsII[1]=myatof(StrTokStr1[4]);
								set_value=s_nTimerSettings.m_DrAmpsII[1];
								}
								TripCurrent=myatof(StrTokStr1[5]);
								
								actual_value=TripCurrent;
								phase_number=WhichPhaseHaveingProblem;
							}
							else if(number_enu_data[0]==8)
							{
								nSTATE_MOTOR1_SMS=STATE_MOTOR1_TRIP_HIGHPRESS_SMS;
								trip_flag =1;
							}
							else if(number_enu_data[0]==9)
							{
								nSTATE_MOTOR1_SMS=STATE_MOTOR1_TRIP_LOWPRESS_SMS;
								trip_flag =1;
							}
							else if(number_enu_data[0]==10)
							{ 
								
								nSTATE_MOTOR1_SMS=STATE_MOTOR1_TRIP_SPP_SMS;
								VSPPImbalanceVoltage=myatoi(StrTokStr1[3]);
								s_nTimerSettings.m_ImbVolt=myatoi(StrTokStr1[4]);
								sprintf(buf,"vspp : %03d TIMB : %03d",VSPPImbalanceVoltage,s_nTimerSettings.m_ImbVolt);
								sAPI_UartPrintf(buf);
								set_value=s_nTimerSettings.m_ImbVolt;
							 actual_value=(float)VSPPImbalanceVoltage;
							phase_number=-1;
							trip_flag =1;
						//	actual_value=TripCurrent;
							}
							else if(number_enu_data[0]==11)
							{
								nSTATE_MOTOR1_SMS=STATE_MOTOR1_TRIP_REVERSEPHASE_SMS;
								trip_flag =1;
							}
							else if(number_enu_data[0]==12)
							nSTATE_MOTOR1_SMS=STATE_MOTOR1_TRIP_2PHASE_SMS;
							else if(number_enu_data[0]==13)
							nSTATE_MOTOR1_SMS=STATE_MOTOR1_TRIP_3PHASE_SMS;
							else if(number_enu_data[0]==14)
							{
								nSTATE_MOTOR1_SMS=STATE_MOTOR1_TRIP_CURRENTSPP_SMS;
								CSPPValue=myatoi(StrTokStr1[3]);
								trip_flag =1;
							}
							else if(number_enu_data[0]==15)
							{
								
								TripVoltage=0;trip_flag =1;
								nSTATE_MOTOR1_SMS=STATE_MOTOR1_TRIP_LOWVOLTAGE_SMS;
								WhichPhaseHaveingProblem = myatoi(StrTokStr1[3]);
								//WorkingOn3Phas=myatoi(StrTokStr1[6]);
								if(WorkingOn3Phas == 1)
								{
									s_nTimerSettings.m_LowVoltIII=myatoi(StrTokStr1[4]);
									set_value=s_nTimerSettings.m_LowVoltIII;
									set_value3=s_nTimerSettings.m_LowVoltIII;
									set_value2=s_nTimerSettings.m_LowVoltIII;
									s_nTimerSettings.m_DiffVoltIII=myatoi(StrTokStr1[5]);
									
								}
								else 
								{
									s_nTimerSettings.m_LowVoltII=myatoi(StrTokStr1[4]);
									set_value=s_nTimerSettings.m_LowVoltII;
									set_value3=s_nTimerSettings.m_LowVoltII;
									set_value2=s_nTimerSettings.m_LowVoltII;
									s_nTimerSettings.m_DiffVoltII=myatoi(StrTokStr1[5]);
								}
								
								TripVoltage=myatoi(StrTokStr1[6]);
								actual_value3=(float)TripVoltage;
								actual_value2=(float)TripVoltage;
								
								
							actual_value=(float)TripVoltage;
							phase_number=WhichPhaseHaveingProblem;
							phase_number3=WhichPhaseHaveingProblem;
							phase_number2=WhichPhaseHaveingProblem;
							}
							else if(number_enu_data[0]==16)
							{
								
								TripVoltage=0;trip_flag =1;
								nSTATE_MOTOR1_SMS=STATE_MOTOR1_TRIP_HIGHVOLTAGE_SMS;
								WhichPhaseHaveingProblem = myatoi(StrTokStr1[3]);
								//WorkingOn3Phas=myatoi(StrTokStr1[6]);
								if(WorkingOn3Phas == 1)
								{
								s_nTimerSettings.m_HighVoltIII=myatoi(StrTokStr1[4]);
								set_value=s_nTimerSettings.m_HighVoltIII;
								set_value3=s_nTimerSettings.m_HighVoltIII;
								set_value2=s_nTimerSettings.m_HighVoltIII;
								s_nTimerSettings.m_HiDiffVoltIII=myatoi(StrTokStr1[5]);
								}
								else 
								{
								s_nTimerSettings.m_HighVoltII=myatoi(StrTokStr1[4]);
								set_value=s_nTimerSettings.m_HighVoltII;
								set_value3=s_nTimerSettings.m_HighVoltII;
								set_value2=s_nTimerSettings.m_HighVoltII;
								s_nTimerSettings.m_HiDiffVoltII=myatoi(StrTokStr1[5]);
								}
								TripVoltage = myatoi(StrTokStr1[6]);
								actual_value2=(float)TripVoltage;
								actual_value3=(float)TripVoltage;
								
							actual_value=(float)TripVoltage;
							phase_number=WhichPhaseHaveingProblem;
							phase_number3=WhichPhaseHaveingProblem;
							phase_number2=WhichPhaseHaveingProblem;
							}
							else if(number_enu_data[0]==17)
							{
								sms_offlag = 1;
								nSTATE_MOTOR1_SMS=STATE_MOTOR1_RTCOF1_TRIP_SMS;
							}
							else if(number_enu_data[0]==18)
							{
								nSTATE_MOTOR1_SMS=STATE_MOTOR1_RTCOF2_TRIP_SMS;
								sms_offlag = 1;
							}
							else if(number_enu_data[0]==19)
							{
							nSTATE_MOTOR1_SMS=STATE_MOTOR1_RTCOF3_TRIP_SMS;
							sms_offlag = 1;
							}
							else if(number_enu_data[0]==20)
							{
							nSTATE_MOTOR1_SMS=STATE_MOTOR1_RTCOF4_TRIP_SMS;
							sms_offlag = 1;
							}
							else if(number_enu_data[0]==21)
							{
							nSTATE_MOTOR1_SMS=STATE_MOTOR1_RTCOF5_TRIP_SMS;
							sms_offlag = 1;
							}
							else if(number_enu_data[0]==22)
							{
								sms_offlag = 1;
								nSTATE_MOTOR1_SMS=STATE_MOTOR1_RTCOF6_TRIP_SMS;
							}
							else if(number_enu_data[0]==23)
							{
								nSTATE_MOTOR1_SMS=STATE_MOTOR1_CYCLIC_TRIP_SMS;
								Cyclic_On_Completed[0]=myatoi(StrTokStr1[3]);
								trip_flag = 1;
								Time_Hr=Cyclic_On_Completed[0]/3600;
								Time_var=Cyclic_On_Completed[0]-(Time_Hr*3600);
								Time_Min=Time_var/60;
								Time_Sec=Time_var-(Time_Min*60)-1;
										
							}
							else if(number_enu_data[0]==24)
							{
							nSTATE_MOTOR1_SMS=STATE_MOTOR1_MAX_TRIP_SMS;
							Maxrun_On_Completed[0]=myatoi(StrTokStr1[3]);
							prev_onof_M1=myatoi(StrTokStr1[6]);
							Prev_M1_state=myatoi(StrTokStr1[7]);
							tripflag[0]=myatoi(StrTokStr1[8]);
							trip_flag =1;
							Time_Hr=Maxrun_On_Completed[0]/3600;
							Time_var=Maxrun_On_Completed[0]-(Time_Hr*3600);
							Time_Min=Time_var/60;
							Time_Sec=Time_var-(Time_Min*60)-1;
							}
							else if(number_enu_data[0]==25)
							nSTATE_MOTOR1_SMS=STATE_MOTOR1_OFF_TARGET;
							else if(number_enu_data[0]==26)
							nSTATE_MOTOR1_SMS=STATE_MOTOR1OF_3PHASE_ONLY_SMS;
							else if(number_enu_data[0]==27)
							{
								nSTATE_MOTOR1_SMS=STATE_MOTOR1_ON_SWITCH_TRIP_SMS;
								//WorkingOn3Phas=uart_read_buf[9]-'0';
								sms_offlag = 1;
							}
							else if(number_enu_data[0]==29)
							{
								nSTATE_MOTOR1_SMS=STATE_MOTOR1_LEVELSCAN_UPPERTANK_TRIP_SMS;
								//WorkingOn3Phas=uart_read_buf[9]-'0';
								level_percent=myatoi(StrTokStr1[4]);
								trip_flag =1;
							}
							else if(number_enu_data[0]==30)
							{
								nSTATE_MOTOR1_SMS=STATE_MOTOR1_LEVELSCAN_LOWERTANK_TRIP_SMS;
								//WorkingOn3Phas=uart_read_buf[9]-'0';
								level_percent=myatoi(StrTokStr1[4]);
								trip_flag =1;
							}
							else if(number_enu_data[0]==31)
							nSTATE_MOTOR1_SMS=STATE_NLIGHT_RTCOF_SMS;
							else if(number_enu_data[0]==32)
								{
								trip_flag=1;
							nSTATE_MOTOR1_SMS=STATE_CYCINTERVEL_OF_SMS;
								}
							else if(number_enu_data[0]==33)
								{
								trip_flag=1;
							nSTATE_MOTOR1_SMS=STATE_CYCLELIMTE_COMPLETED_SMS;
								}
							else if(number_enu_data[0]==34)
							{
								trip_flag=1;
							nSTATE_MOTOR1_SMS=STATE_MOISTURE_OF_SMS;
							}
							else if(number_enu_data[0]==35)
							{
								trip_flag=1;
							nSTATE_MOTOR1_SMS=STATE_PAUSE_OF_SMS;
							}
							else if(number_enu_data[0]==36)
							{
								trip_flag=1;
							nSTATE_MOTOR1_SMS=STATE_WRONG_FEEDBACK_OF_SMS;
							}
							else if(number_enu_data[0]==37)
							{
								trip_flag=1;
							nSTATE_MOTOR1_SMS=STATE_NO_COMMUNICATION_OF_SMS;
							}
							else if(number_enu_data[0]==38)
							{
								trip_flag=1;
								set_value=myatof(StrTokStr1[3]);
								actual_value=myatof(StrTokStr1[4]);
								nSTATE_MOTOR1_SMS=STATE_PRESSURE_LOW_SMS;
							}
							else if(number_enu_data[0]==39)
							{
								trip_flag=1;
								set_value=myatof(StrTokStr1[3]);
								actual_value=myatof(StrTokStr1[4]);
								nSTATE_MOTOR1_SMS=STATE_PRESSURE_HIGH_SMS;
							}
							else if(number_enu_data[0]==40)
							{
								trip_flag=1;
							nSTATE_MOTOR1_SMS=STATE_PRESSURE_SWITCH_OF_SMS;
							}
							if(number_enu_data[0] == 17 || number_enu_data[0] == 18 || number_enu_data[0] == 19 || 
							number_enu_data[0] == 20 || number_enu_data[0] == 21 || number_enu_data[0] == 22)
							{
								RTC_On_Completed[0]=myatoi(StrTokStr1[3]);
								Time_Hr=RTC_On_Completed[0]/3600;
								Time_var=RTC_On_Completed[0]-(Time_Hr*3600);
								Time_Min=Time_var/60;
								Time_Sec=Time_var-(Time_Min*60);
							}
						
							prev_number_enu_data[0] = number_enu_data[0];							
							number_enu_data[0] =0;
							if(Time_Sec <=0 || Time_Sec >=60)
								Time_Sec=0;
						}
						sprintf(buf,"\n\rtest : s_nTimerSettings.m_OlAmpsIII[1]=%02.01f, s_nTimerSettings.m_OlAmpsII[1]=%02.01f, TripCurrent=%02.01f\n\r",s_nTimerSettings.m_OlAmpsIII[1],s_nTimerSettings.m_OlAmpsII[1],TripCurrent);
						sAPI_UartPrintf(buf);	
						
						sprintf(buf,"\n\rtest : prev_onof_M1=%d, Prev_M1_state =%d\n\r",prev_onof_M1,Prev_M1_state);
						sAPI_UartPrintf(buf);
					}
						#if 0
						if(uart_read_buf[0] == '$' && uart_read_buf[1] == 'M' && uart_read_buf[2] == '2' && uart_read_buf[4] == 'N')
						{
							
							int StrTokStrVer = 0;
							
							
							Pch1 = strtok((char *)uart_read_buf, (char *)"," );
							StrTokStrVer =0 ;
							while( Pch1 != NULL )
							{
							strcpy(StrTokStr1[StrTokStrVer],Pch1);
							StrTokStrVer++;
							Pch1 = strtok( NULL, "," );
							}
							
							number_enu_data[1]=myatoi(StrTokStr1[2]);
							
							//number_enu_data[1]=(uart_read_buf[6]-'0')*10+(uart_read_buf[7]-'0');
							sprintf(buf,"$<ACK>\n\r");
							sAPI_UartWrite(SC_UART,(UINT8*)buf,strlen(buf));
							sAPI_UartPrintf(buf);
							sprintf(buf,"\n\r prev_number_enu_data[1] = %02d, number_enu_data[1] : %02d\n\r",prev_number_enu_data[1],number_enu_data[1]);
							sAPI_UartPrintf(buf);
							
							if(number_enu_data[1]>=1 && prev_number_enu_data[1] != number_enu_data[1])
							{
								livedataflag=1;
								SendSmsToAll = 1;
								sms_onflag1 = 1;
								trip_flag1 =0; sms_offlag1 =0;
								Time_flag1=1;
								Time_Counter1=0;
								act_rem_delmin[1]=act_POnMin[1];
							act_rem_delsec[1]=act_POnSec[1];
								if(number_enu_data[1]==0)
								nSTATE_MOTOR2_ON_SMS=STATE_MOTOR2_DUMMY;	
								else if(number_enu_data[1]==1)
								{
									nSTATE_MOTOR2_ON_SMS=STATE_MOTOR2_ON_ONDELAY;
									//WorkingOn3Phas=uart_read_buf[9]-'0';
								}
								else if(number_enu_data[1]==2)
								{
									nSTATE_MOTOR2_ON_SMS=STATE_MOTOR2_ON_RTC1;
									//WorkingOn3Phas=uart_read_buf[9]-'0';
								}
								else if(number_enu_data[1]==3)
								{
									nSTATE_MOTOR2_ON_SMS=STATE_MOTOR2_ON_RTC2;
									//WorkingOn3Phas=uart_read_buf[9]-'0';
								}
								else if(number_enu_data[1]==4)
								{
									nSTATE_MOTOR2_ON_SMS=STATE_MOTOR2_ON_RTC3;
									//WorkingOn3Phas=uart_read_buf[9]-'0';
								}
								else if(number_enu_data[1]==5)
								{
									nSTATE_MOTOR2_ON_SMS=STATE_MOTOR2_ON_RTC4;
									//WorkingOn3Phas=uart_read_buf[9]-'0';
								}
								else if(number_enu_data[1]==6)
								{
									nSTATE_MOTOR2_ON_SMS=STATE_MOTOR2_ON_RTC5;
									//WorkingOn3Phas=uart_read_buf[9]-'0';
								}
								else if(number_enu_data[1]==7)
								{
									nSTATE_MOTOR2_ON_SMS=STATE_MOTOR2_ON_RTC6;
									//WorkingOn3Phas=uart_read_buf[9]-'0';
								}
								else if(number_enu_data[1]==8)
								nSTATE_MOTOR2_ON_SMS=STATE_MOTOR2_ON_UPPERTANK;
								else if(number_enu_data[1]==9)
								nSTATE_MOTOR2_ON_SMS=STATE_MOTOR2_ON_LOWERTANK;
								else if(number_enu_data[1]==10)
								nSTATE_MOTOR2_ON_SMS=STATE_MOTOR2_ON_OLVOLTAGE;
								else if(number_enu_data[1]==11)
								{
									nSTATE_MOTOR2_ON_SMS=STATE_MOTOR2_ON_SWITCH_SMS;
									//WorkingOn3Phas=uart_read_buf[9]-'0';
								}
								else if(number_enu_data[1]==12)
								nSTATE_MOTOR2_ON_SMS=STATE_MOTOR2_ON_TARGET;
								else if(number_enu_data[1]==13)
								nSTATE_MOTOR2_ON_SMS=STATE_MOTOR2_ON_HIGHVOLTAGE;
								else if(number_enu_data[1]==14)
								nSTATE_MOTOR2_ON_SMS=STATE_MOTOR2_ON_LOWVOLTAGE;
								/* else if(number_enu_data==15)
								nSTATE_MOTOR2_ON_SMS=STATE_MOTOR2_TRIP_HIGHVOLTAGE_SMS; */
								else if(number_enu_data[1]==15)
								nSTATE_MOTOR2_ON_SMS=STATE_MOTOR2_ON_SPP;
								else if(number_enu_data[1]==16)
								nSTATE_MOTOR2_ON_SMS=STATE_MOTOR2_ON_RESTARTTIMER; //deletexx
								else if(number_enu_data[1]==17)
								{
									nSTATE_MOTOR2_ON_SMS=STATE_MOTOR2_ON_LEVELSCAN_UPPERTANK;
									//WorkingOn3Phas=uart_read_buf[9]-'0';
									level_percent=(uart_read_buf[11]-'0')*10+(uart_read_buf[12]-'0');
								}
								
								else if(number_enu_data[1]==18)
								{
									nSTATE_MOTOR2_ON_SMS=STATE_MOTOR2_ON_LEVELSCAN_LOWERTANK;
									//WorkingOn3Phas=uart_read_buf[9]-'0';
									level_percent=(uart_read_buf[11]-'0')*10+(uart_read_buf[12]-'0');
								}
								else
							nSTATE_MOTOR2_ON_SMS=STATE_MOTOR2_ON_DEFAULT;
							
							prev_number_enu_data[1] = number_enu_data[1];
							number_enu_data[1] =0;
							}
							
						}
					
					
					if(uart_read_buf[0] == '$' && uart_read_buf[1] == 'M' && uart_read_buf[2] == '2' && uart_read_buf[4] == 'F')
					{
								int StrTokStrVer = 0;
							
							Pch1 = strtok((char *)uart_read_buf, (char *)"," );
							StrTokStrVer =0 ;
							while( Pch1 != NULL )
							{
							strcpy(StrTokStr1[StrTokStrVer],Pch1);
							StrTokStrVer++;
							Pch1 = strtok( NULL, "," );
							}
					//		livedataflag=1;				
							number_enu_data[1]=myatoi(StrTokStr1[2]);
											
											
				//	}						
						//	number_enu_data=(uart_read_buf[6]-'0')*10+(uart_read_buf[7]-'0');
							sprintf(buf,"$<ACK>\n\r");
							sAPI_UartWrite(SC_UART,(UINT8*)buf,strlen(buf));
							sAPI_UartPrintf(buf);
							sprintf(buf,"number_enu_data[1] : %02d prev_number_enu_data[1]:%d WhichPhaseHaveingProblem %d ",number_enu_data[1],prev_number_enu_data[1],WhichPhaseHaveingProblem);
							sAPI_UartPrintf(buf);
								
					if(number_enu_data[1]>=1 && prev_number_enu_data[1] != number_enu_data[1])//subash doubte to modify
						{
							actual_value=0;
							set_value=0;
							phase_number=0;
							SendSmsToAll = 1;
							sms_onflag1 =0;
							
							if(number_enu_data[1]==0)
							nSTATE_MOTOR2_SMS=STATE_NO_MOTOR2_SMS;	
							else if(number_enu_data[1]==1)
							nSTATE_MOTOR2_SMS=STATE_MOTOR2ON_SMS;
							else if(number_enu_data[1]==2)
							{
								nSTATE_MOTOR2_SMS=STATE_MOTOR2OF_SMS;
								sms_offlag1 = 1;
							}
							else if(number_enu_data[1]==3)
							{
								nSTATE_MOTOR2_SMS=STATE_MOTOR2_STARTER_TRIP_SMS;
								OverAllStarterTrip2=myatoi(StrTokStr1[3]);
								trip_flag1 =1;
							}
							else if(number_enu_data[1]==4)
							{
							nSTATE_MOTOR2_SMS=STATE_MOTOR2_UPPERTANK_TRIP_SMS;
							trip_flag1 =1;
							}
							else if(number_enu_data[1]==5)
							{
							nSTATE_MOTOR2_SMS=STATE_MOTOR2_LOWERTANK_TRIP_SMS;
							trip_flag1 =1;
							}
							else if(number_enu_data[1]==6)
							{
								
								TripCurrent=0.0;trip_flag1 =1;																			
								nSTATE_MOTOR2_SMS=STATE_MOTOR2_TRIP_OVERLOAD_SMS;
								WhichPhaseHaveingProblem=myatoi(StrTokStr1[3]);
								phase_number2=WhichPhaseHaveingProblem;
								//WorkingOn3Phas=myatoi(StrTokStr1[6]);
								if(WorkingOn3Phas == 1)
								{
								s_nTimerSettings.m_OlAmpsIII[2]=myatof(StrTokStr1[4]);
								set_value2=s_nTimerSettings.m_OlAmpsIII[2];
								}
								else 
								{
								s_nTimerSettings.m_OlAmpsII[2]=myatof(StrTokStr1[4]);
								set_value2=s_nTimerSettings.m_OlAmpsII[2];
								}
								TripCurrent=myatof(StrTokStr1[5]);
								sprintf(buf,"\n\rfor checking: olamps=%c%c%c%c, TripCurrent=%c%c%c%c\n\r",uart_read_buf[11],uart_read_buf[12],uart_read_buf[13],uart_read_buf[14],
																												uart_read_buf[16],uart_read_buf[17],uart_read_buf[18],uart_read_buf[19]);
								sAPI_UartPrintf(buf);	//subash commented
								actual_value2=TripCurrent;
							
							phase_number=0;
							}
							else if(number_enu_data[1]==7)
							{
								
								TripCurrent=0.0;trip_flag1 =1;
								nSTATE_MOTOR2_SMS=STATE_MOTOR2_DRYRUN_TRIP_SMS;
								WhichPhaseHaveingProblem=myatoi(StrTokStr1[3]);
								//WorkingOn3Phas=myatoi(StrTokStr1[6]);
								if(WorkingOn3Phas == 1)
								{
								s_nTimerSettings.m_DrAmpsIII[2]=myatof(StrTokStr1[4]);
								set_value2=s_nTimerSettings.m_DrAmpsIII[2];
								}
								else 
								{
								s_nTimerSettings.m_DrAmpsII[2]=myatof(StrTokStr1[4]);
								set_value2=s_nTimerSettings.m_DrAmpsII[2];
								}
								TripCurrent=myatof(StrTokStr1[5]);
								
								actual_value2=TripCurrent;
								phase_number2=WhichPhaseHaveingProblem;
							}
							else if(number_enu_data[1]==8)
							{
							nSTATE_MOTOR2_SMS=STATE_MOTOR2_TRIP_HIGHPRESS_SMS;
							trip_flag1 =1;
							}
							else if(number_enu_data[1]==9)
							{
							nSTATE_MOTOR2_SMS=STATE_MOTOR2_TRIP_LOWPRESS_SMS;
							trip_flag1 =1;
							}
							else if(number_enu_data[1]==10)
							{ 
								
								nSTATE_MOTOR2_SMS=STATE_MOTOR2_TRIP_SPP_SMS;
								VSPPImbalanceVoltage=myatoi(StrTokStr1[3]);
								s_nTimerSettings.m_ImbVolt=myatoi(StrTokStr1[4]);
								sprintf(buf,"vspp : %03d TIMB : %03d",VSPPImbalanceVoltage,s_nTimerSettings.m_ImbVolt);
								sAPI_UartPrintf(buf);
								set_value2=s_nTimerSettings.m_ImbVolt;
							actual_value2=VSPPImbalanceVoltage;
							phase_number2=-1;trip_flag1 =1;
							
							
							}
							else if(number_enu_data[1]==11)
							{
							nSTATE_MOTOR2_SMS=STATE_MOTOR2_TRIP_REVERSEPHASE_SMS;
							trip_flag1 =1;
							}
							else if(number_enu_data[1]==12)
							nSTATE_MOTOR2_SMS=STATE_MOTOR2_TRIP_2PHASE_SMS;
							else if(number_enu_data[1]==13)
							nSTATE_MOTOR2_SMS=STATE_MOTOR2_TRIP_3PHASE_SMS;
							else if(number_enu_data[1]==14)
							{
							nSTATE_MOTOR2_SMS=STATE_MOTOR2_TRIP_CURRENTSPP_SMS;
							CSPPValue=myatoi(StrTokStr1[3]);
							trip_flag1 =1;
							}
							else if(number_enu_data[1]==15)
							{
								
								TripVoltage=0;trip_flag1 =1;
								nSTATE_MOTOR2_SMS=STATE_MOTOR2_TRIP_LOWVOLTAGE_SMS;
								WhichPhaseHaveingProblem = myatoi(StrTokStr1[3]);
								//WorkingOn3Phas=myatoi(StrTokStr1[6]);
								if(WorkingOn3Phas == 1)
								{
									s_nTimerSettings.m_LowVoltIII=myatoi(StrTokStr1[4]);
									set_value2=s_nTimerSettings.m_LowVoltIII;
									set_value3=s_nTimerSettings.m_LowVoltIII;
									set_value=s_nTimerSettings.m_LowVoltIII;
									s_nTimerSettings.m_DiffVoltIII=myatoi(StrTokStr1[5]);
									
								}
								else 
								{
									s_nTimerSettings.m_LowVoltII=myatoi(StrTokStr1[4]);
									set_value2=s_nTimerSettings.m_LowVoltII;
									set_value3=s_nTimerSettings.m_LowVoltII;
									set_value=s_nTimerSettings.m_LowVoltII;
									s_nTimerSettings.m_DiffVoltII=myatoi(StrTokStr1[5]);
								}
								
								TripVoltage=myatoi(StrTokStr1[6]);
								
								
							actual_value2=(float)TripVoltage;
							actual_value=(float)TripVoltage;
							actual_value3=(float)TripVoltage;
							phase_number2=WhichPhaseHaveingProblem;
							phase_number3=WhichPhaseHaveingProblem;
							phase_number=WhichPhaseHaveingProblem;
							}
							else if(number_enu_data[1]==16)
							{
								
								TripVoltage=0;trip_flag1 =1;
								nSTATE_MOTOR2_SMS=STATE_MOTOR2_TRIP_HIGHVOLTAGE_SMS;
								WhichPhaseHaveingProblem = myatoi(StrTokStr1[3]);
								//WorkingOn3Phas=myatoi(StrTokStr1[6]);
								if(WorkingOn3Phas == 1)
								{
									s_nTimerSettings.m_HighVoltIII=myatoi(StrTokStr1[4]);
									set_value2=s_nTimerSettings.m_HighVoltIII;
									set_value3=s_nTimerSettings.m_HighVoltIII;
									set_value=s_nTimerSettings.m_HighVoltIII;
									s_nTimerSettings.m_HiDiffVoltIII=myatoi(StrTokStr1[5]);
								}
								else 
								{
									s_nTimerSettings.m_HighVoltII=myatoi(StrTokStr1[4]);
									set_value2=s_nTimerSettings.m_HighVoltII;
									set_value3=s_nTimerSettings.m_HighVoltII;
									set_value=s_nTimerSettings.m_HighVoltII;
									s_nTimerSettings.m_HiDiffVoltII=myatoi(StrTokStr1[5]);
								}
								TripVoltage = myatoi(StrTokStr1[6]);
								
								
							actual_value2=(float)TripVoltage;
							actual_value=(float)TripVoltage;
							actual_value3=(float)TripVoltage;
							phase_number2=WhichPhaseHaveingProblem;
							phase_number3=WhichPhaseHaveingProblem;
							phase_number=WhichPhaseHaveingProblem;
							}
							else if(number_enu_data[1]==17)
							{
							nSTATE_MOTOR2_SMS=STATE_MOTOR2_RTCOF1_TRIP_SMS;
							sms_offlag1 =1;
							}
							else if(number_enu_data[1]==18)
							{
							nSTATE_MOTOR2_SMS=STATE_MOTOR2_RTCOF2_TRIP_SMS;
							sms_offlag1 =1;
							}
							else if(number_enu_data[1]==19)
							{
							nSTATE_MOTOR2_SMS=STATE_MOTOR2_RTCOF3_TRIP_SMS;
							sms_offlag1 =1;
							}
							else if(number_enu_data[1]==20)
							{
							nSTATE_MOTOR2_SMS=STATE_MOTOR2_RTCOF4_TRIP_SMS;
							sms_offlag1 =1;
							}
							else if(number_enu_data[1]==21)
							{
							nSTATE_MOTOR2_SMS=STATE_MOTOR2_RTCOF5_TRIP_SMS;
							sms_offlag1 =1;
							}
							else if(number_enu_data[1]==22)
							{
							nSTATE_MOTOR2_SMS=STATE_MOTOR2_RTCOF6_TRIP_SMS;
							sms_offlag1 =1;
							}
							else if(number_enu_data[1]==23)
							{
							nSTATE_MOTOR2_SMS=STATE_MOTOR2_CYCLIC_TRIP_SMS;
							trip_flag1=1;
							}
							else if(number_enu_data[1]==24)
							{
								nSTATE_MOTOR2_SMS=STATE_MOTOR2_MAX_TRIP_SMS;
								prev_onof_M1=myatoi(StrTokStr1[6]);
								Prev_M1_state=myatoi(StrTokStr1[7]);
								tripflag[0]=myatoi(StrTokStr1[8]);
								trip_flag1=1;
							}
							else if(number_enu_data[1]==25)
							nSTATE_MOTOR2_SMS=STATE_MOTOR2_OFF_TARGET;
							else if(number_enu_data[1]==26)
							nSTATE_MOTOR2_SMS=STATE_MOTOR2OF_3PHASE_ONLY_SMS;
							else if(number_enu_data[1]==27)
							{
								nSTATE_MOTOR2_SMS=STATE_MOTOR2_ON_SWITCH_TRIP_SMS;
								//WorkingOn3Phas=uart_read_buf[9]-'0';
								sms_offlag1=1;
							}
							else if(number_enu_data[1]==28)
							{
								nSTATE_MOTOR2_SMS=STATE_MOTOR2_LEVELSCAN_UPPERTANK_TRIP_SMS;
								//WorkingOn3Phas=uart_read_buf[9]-'0';
								level_percent=myatoi(StrTokStr1[4]);
								trip_flag1=1;
							}
							else if(number_enu_data[1]==29)
							{
								nSTATE_MOTOR2_SMS=STATE_MOTOR2_LEVELSCAN_LOWERTANK_TRIP_SMS;
								//WorkingOn3Phas=uart_read_buf[9]-'0';
								level_percent=myatoi(StrTokStr1[4]);
								trip_flag1=1;
							}
											
						prev_number_enu_data[1] = number_enu_data[1];
						number_enu_data[1] =0;
						}
						sprintf(buf,"\n\rtest : s_nTimerSettings.m_OlAmpsIII[2]=%02.01f, s_nTimerSettings.m_OlAmpsII[2]=%02.01f, TripCurrent=%02.01f\n\r",s_nTimerSettings.m_OlAmpsIII[2],s_nTimerSettings.m_OlAmpsII[2],TripCurrent);
						sAPI_UartPrintf(buf);	
						
						sprintf(buf,"\n\rtest : prev_onof_M1=%d, Prev_M1_state =%d\n\r",prev_onof_M1,Prev_M1_state);
						sAPI_UartPrintf(buf);
					}
					
						
						if(uart_read_buf[0] == '$' && uart_read_buf[1] == 'M' && uart_read_buf[2] == '3' && uart_read_buf[4] == 'N')
						{
							
							number_enu_data[2]=(uart_read_buf[6]-'0')*10+(uart_read_buf[7]-'0');
							sprintf(buf,"$<ACK>\n\r");
							sAPI_UartWrite(SC_UART,(UINT8*)buf,strlen(buf));
							sAPI_UartPrintf(buf);
							sprintf(buf,"\n\r prev_number_enu_data[2] = %02d, number_enu_data[2] : %02d\n\r",prev_number_enu_data[2],number_enu_data[2]);
							sAPI_UartPrintf(buf);
							
							if(number_enu_data[2]>=1 && prev_number_enu_data[2] != number_enu_data[2])
							{
								livedataflag=1;
								SendSmsToAll = 1;
								sms_onflag2 = 1;
								trip_flag2 =0; sms_offlag2 =0;
								Time_flag2=1;
								Time_Counter2=0;
								act_rem_delmin[2]=act_POnMin[2];
							act_rem_delsec[2]=act_POnSec[2];
								if(number_enu_data[2]==0)
								nSTATE_MOTOR3_ON_SMS=STATE_MOTOR3_DUMMY;	
								else if(number_enu_data[2]==1)
								{
									nSTATE_MOTOR3_ON_SMS=STATE_MOTOR3_ON_ONDELAY;
									//WorkingOn3Phas=uart_read_buf[9]-'0';
								}
								else if(number_enu_data[2]==2)
								{
									nSTATE_MOTOR3_ON_SMS=STATE_MOTOR3_ON_RTC1;
									//WorkingOn3Phas=uart_read_buf[9]-'0';
								}
								else if(number_enu_data[2]==3)
								{
									nSTATE_MOTOR3_ON_SMS=STATE_MOTOR3_ON_RTC2;
									//WorkingOn3Phas=uart_read_buf[9]-'0';
								}
								else if(number_enu_data[2]==4)
								{
									nSTATE_MOTOR3_ON_SMS=STATE_MOTOR3_ON_RTC3;
									//WorkingOn3Phas=uart_read_buf[9]-'0';
								}
								else if(number_enu_data[2]==5)
								{
									nSTATE_MOTOR3_ON_SMS=STATE_MOTOR3_ON_RTC4;
									//WorkingOn3Phas=uart_read_buf[9]-'0';
								}
								else if(number_enu_data[2]==6)
								{
									nSTATE_MOTOR3_ON_SMS=STATE_MOTOR3_ON_RTC5;
									//WorkingOn3Phas=uart_read_buf[9]-'0';
								}
								else if(number_enu_data[2]==7)
								{
									nSTATE_MOTOR3_ON_SMS=STATE_MOTOR3_ON_RTC6;
									//WorkingOn3Phas=uart_read_buf[9]-'0';
								}
								else if(number_enu_data[2]==8)
								nSTATE_MOTOR3_ON_SMS=STATE_MOTOR3_ON_UPPERTANK;
								else if(number_enu_data[2]==9)
								nSTATE_MOTOR3_ON_SMS=STATE_MOTOR3_ON_LOWERTANK;
								else if(number_enu_data[2]==10)
								nSTATE_MOTOR3_ON_SMS=STATE_MOTOR3_ON_OLVOLTAGE;
								else if(number_enu_data[2]==11)
								{
									nSTATE_MOTOR3_ON_SMS=STATE_MOTOR3_ON_SWITCH_SMS;
									//WorkingOn3Phas=uart_read_buf[9]-'0';
								}
								else if(number_enu_data[2]==12)
								nSTATE_MOTOR3_ON_SMS=STATE_MOTOR3_ON_TARGET;
								else if(number_enu_data[2]==13)
								nSTATE_MOTOR3_ON_SMS=STATE_MOTOR3_ON_HIGHVOLTAGE;
								else if(number_enu_data[2]==14)
								nSTATE_MOTOR3_ON_SMS=STATE_MOTOR3_ON_LOWVOLTAGE;
								/* else if(number_enu_data[2]==15)
								nSTATE_MOTOR3_ON_SMS=STATE_MOTOR3_TRIP_HIGHVOLTAGE_SMS; */
								else if(number_enu_data[2]==15)
								nSTATE_MOTOR3_ON_SMS=STATE_MOTOR3_ON_SPP;
								else  if(number_enu_data[2]==16)
								nSTATE_MOTOR3_ON_SMS=STATE_MOTOR3_ON_RESTARTTIMER;
								else if(number_enu_data[2]==17)
								{
									nSTATE_MOTOR3_ON_SMS=STATE_MOTOR3_ON_LEVELSCAN_UPPERTANK;
									//WorkingOn3Phas=uart_read_buf[9]-'0';
									level_percent=(uart_read_buf[11]-'0')*10+(uart_read_buf[12]-'0');
								}
								else if(number_enu_data[2]==18)
								{
									nSTATE_MOTOR3_ON_SMS=STATE_MOTOR3_ON_LEVELSCAN_LOWERTANK;
									//WorkingOn3Phas=uart_read_buf[9]-'0';
									level_percent=(uart_read_buf[11]-'0')*10+(uart_read_buf[12]-'0');
								}
								else
							nSTATE_MOTOR3_ON_SMS=STATE_MOTOR3_ON_DEFAULT;
							prev_number_enu_data[2] = number_enu_data[2];							
							number_enu_data[2] =0;
							}
					
						}
						
						if(uart_read_buf[0] == '$' && uart_read_buf[1] == 'M' && uart_read_buf[2] == '3' && uart_read_buf[4] == 'F')
					{
								int StrTokStrVer = 0;
							//	char uart_buf_tmp[200];
							//memcpy(uart_buf_tmp,uart_read_buf,sizeof(uart_read_buf));	
							//sprintf(buf,"uart_buf_tmp : %s",uart_buf_tmp);
							//sAPI_UartPrintf(buf);
					//		livedataflag=1;
							Pch1 = strtok((char *)uart_read_buf, (char *)"," );
							StrTokStrVer =0 ;
							while( Pch1 != NULL )
							{
							strcpy(StrTokStr1[StrTokStrVer],Pch1);
							StrTokStrVer++;
							Pch1 = strtok( NULL, "," );
							}
							
							number_enu_data[2]=myatoi(StrTokStr1[2]);
							
							
		//	}						
						//	number_enu_data=(uart_read_buf[6]-'0')*10+(uart_read_buf[7]-'0');
							sprintf(buf,"$<ACK>\n\r");
							sAPI_UartWrite(SC_UART,(UINT8*)buf,strlen(buf));
							sAPI_UartPrintf(buf);
							sprintf(buf,"\n\r prev_number_enu_data[2] = %02d, number_enu_data[2] : %02d\n\r",prev_number_enu_data[2],number_enu_data[2]);
							sAPI_UartPrintf(buf);
				
						if(number_enu_data[2]>=1 && prev_number_enu_data[2] != number_enu_data[2])//subash doubte to modify
						{
							sAPI_UartPrintf("\nM3 OFF generated\n\r");
							actual_value3=0;
							set_value3=0;
							phase_number3=0;
							sms_onflag2=0;
							if(number_enu_data[2]==0)
							nSTATE_MOTOR3_SMS=STATE_NO_MOTOR3_SMS;	
							else if(number_enu_data[2]==1)
							nSTATE_MOTOR3_SMS=STATE_MOTOR3ON_SMS;
							else if(number_enu_data[2] ==2)
							{
							nSTATE_MOTOR3_SMS=STATE_MOTOR3OF_SMS;
							sms_offlag2 = 1;
							}
						
							else if(number_enu_data[2]==3)
							{
								nSTATE_MOTOR3_SMS=STATE_MOTOR3_STARTER_TRIP_SMS;
								OverAllStarterTrip3=myatoi(StrTokStr1[3]);
								trip_flag2 =1;
							}
							else if(number_enu_data[2]==4)
							{
								nSTATE_MOTOR3_SMS=STATE_MOTOR3_UPPERTANK_TRIP_SMS;
								trip_flag2 =1;
							}
							else if(number_enu_data[2]==5)
							{
								nSTATE_MOTOR3_SMS=STATE_MOTOR3_LOWERTANK_TRIP_SMS;
								trip_flag2 =1;
							}
							else if(number_enu_data[2]==6)
							{
								
								TripCurrent=0.0;trip_flag2 =1;																			
								nSTATE_MOTOR3_SMS=STATE_MOTOR3_TRIP_OVERLOAD_SMS;
								WhichPhaseHaveingProblem=myatoi(StrTokStr1[3]);
								phase_number3=WhichPhaseHaveingProblem;
								//WorkingOn3Phas=myatoi(StrTokStr1[6]);
								if(WorkingOn3Phas == 1)
								{
								s_nTimerSettings.m_OlAmpsIII[3]=myatof(StrTokStr1[4]);
								set_value3=s_nTimerSettings.m_OlAmpsIII[3];
								}
								else 
								{
								s_nTimerSettings.m_OlAmpsII[3]=myatof(StrTokStr1[4]);
								set_value3=s_nTimerSettings.m_OlAmpsII[3];
								}
								TripCurrent=myatof(StrTokStr1[5]);
								sprintf(buf,"\n\rfor checking: olamps=%c%c%c%c, TripCurrent=%c%c%c%c\n\r",uart_read_buf[11],uart_read_buf[12],uart_read_buf[13],uart_read_buf[14],
																												uart_read_buf[16],uart_read_buf[17],uart_read_buf[18],uart_read_buf[19]);
								sAPI_UartPrintf(buf);	//subash commented
								actual_value3=TripCurrent;
							
							phase_number3=0;
							}
							else if(number_enu_data[2]==7)
							{
								
								TripCurrent=0.0;trip_flag2 =1;
								nSTATE_MOTOR3_SMS=STATE_MOTOR3_DRYRUN_TRIP_SMS;
								WhichPhaseHaveingProblem=myatoi(StrTokStr1[3]);
								//WorkingOn3Phas=myatoi(StrTokStr1[6]);
								if(WorkingOn3Phas == 1)
								{
								s_nTimerSettings.m_DrAmpsIII[3]=myatof(StrTokStr1[4]);
								set_value3=s_nTimerSettings.m_DrAmpsIII[3];
								}
								else 
								{
								s_nTimerSettings.m_DrAmpsII[3]=myatof(StrTokStr1[4]);
								set_value3=s_nTimerSettings.m_DrAmpsII[3];
								}
								TripCurrent=myatof(StrTokStr1[5]);
								
								actual_value3=TripCurrent;
								phase_number=WhichPhaseHaveingProblem;
							}
							else if(number_enu_data[2]==8)
							{
								nSTATE_MOTOR3_SMS=STATE_MOTOR3_TRIP_HIGHPRESS_SMS;
								trip_flag2 =1;
							}
							else if(number_enu_data[2]==9)
							{
								nSTATE_MOTOR3_SMS=STATE_MOTOR3_TRIP_LOWPRESS_SMS;
								trip_flag2 =1;
							}
							else if(number_enu_data[2]==10)
							{ 
								
								nSTATE_MOTOR3_SMS=STATE_MOTOR3_TRIP_SPP_SMS;
								VSPPImbalanceVoltage=myatoi(StrTokStr1[3]);
								s_nTimerSettings.m_ImbVolt=myatoi(StrTokStr1[4]);
								sprintf(buf,"vspp : %03d TIMB : %03d",VSPPImbalanceVoltage,s_nTimerSettings.m_ImbVolt);
								sAPI_UartPrintf(buf);
								set_value3=s_nTimerSettings.m_ImbVolt;
							actual_value3=(float)VSPPImbalanceVoltage;
							phase_number3=-1;trip_flag2 =1;
							
							actual_value=TripCurrent;
							}
							else if(number_enu_data[2]==11)
							{
								nSTATE_MOTOR3_SMS=STATE_MOTOR3_TRIP_REVERSEPHASE_SMS;
								trip_flag2 =1;
							}
							else if(number_enu_data[2]==12)
							nSTATE_MOTOR3_SMS=STATE_MOTOR3_TRIP_2PHASE_SMS;
							else if(number_enu_data[2]==13)
							nSTATE_MOTOR3_SMS=STATE_MOTOR3_TRIP_3PHASE_SMS;
							else if(number_enu_data[2]==14)
							{
							nSTATE_MOTOR3_SMS=STATE_MOTOR3_TRIP_CURRENTSPP_SMS;
							CSPPValue=myatoi(StrTokStr1[3]);
							trip_flag2 =1;
							}
							else if(number_enu_data[2]==15)
							{
								
								TripVoltage=0;trip_flag2 =1;
								nSTATE_MOTOR3_SMS=STATE_MOTOR3_TRIP_LOWVOLTAGE_SMS;
								WhichPhaseHaveingProblem = myatoi(StrTokStr1[3]);
								//WorkingOn3Phas=myatoi(StrTokStr1[6]);
								if(WorkingOn3Phas == 1)
								{
									s_nTimerSettings.m_LowVoltIII=myatoi(StrTokStr1[4]);
									set_value3=s_nTimerSettings.m_LowVoltIII;
									set_value=set_value2=set_value3;
									s_nTimerSettings.m_DiffVoltIII=myatoi(StrTokStr1[5]);
									
								}
								else 
								{
									s_nTimerSettings.m_LowVoltII=myatoi(StrTokStr1[4]);
									set_value3=s_nTimerSettings.m_LowVoltII;
									set_value=set_value2=set_value3;
									s_nTimerSettings.m_DiffVoltII=myatoi(StrTokStr1[5]);
								}
								
								TripVoltage=myatoi(StrTokStr1[6]);
								
								
							actual_value3=(float)TripVoltage;
							actual_value=actual_value2=actual_value3;
							phase_number3=WhichPhaseHaveingProblem;
							phase_number=phase_number2=phase_number3;
							}
							else if(number_enu_data[2]==16)
							{
								
								TripVoltage=0;trip_flag2 =1;
								nSTATE_MOTOR3_SMS=STATE_MOTOR3_TRIP_HIGHVOLTAGE_SMS;
								WhichPhaseHaveingProblem = myatoi(StrTokStr1[3]);
								//WorkingOn3Phas=myatoi(StrTokStr1[6]);
								if(WorkingOn3Phas == 1)
								{
									s_nTimerSettings.m_HighVoltIII=myatoi(StrTokStr1[4]);
									set_value3=s_nTimerSettings.m_HighVoltIII;
									set_value=set_value2=set_value3;
									s_nTimerSettings.m_HiDiffVoltIII=myatoi(StrTokStr1[5]);
								}
								else 
								{
									s_nTimerSettings.m_HighVoltII=myatoi(StrTokStr1[4]);
									set_value3=s_nTimerSettings.m_HighVoltII;
									set_value=set_value2=set_value3;
									s_nTimerSettings.m_HiDiffVoltII=myatoi(StrTokStr1[5]);
								}
								TripVoltage = myatoi(StrTokStr1[6]);
								
								
							actual_value3=(float)TripVoltage;
							actual_value=actual_value2=actual_value3;
							phase_number3=WhichPhaseHaveingProblem;
							phase_number=phase_number2=phase_number3;
							}
							else if(number_enu_data[2]==17)
							{
								nSTATE_MOTOR3_SMS=STATE_MOTOR3_RTCOF1_TRIP_SMS;
								sms_offlag2=1;
							}
							else if(number_enu_data[2]==18)
							{
								nSTATE_MOTOR3_SMS=STATE_MOTOR3_RTCOF2_TRIP_SMS;
								sms_offlag2=1;
							}
							else if(number_enu_data[2]==19)
							{
								nSTATE_MOTOR3_SMS=STATE_MOTOR3_RTCOF3_TRIP_SMS;
								sms_offlag2=1;
							}
							else if(number_enu_data[2]==20)
							{
								nSTATE_MOTOR3_SMS=STATE_MOTOR3_RTCOF4_TRIP_SMS;
								sms_offlag2=1;
							}
							else if(number_enu_data[2]==21)
							{
								nSTATE_MOTOR3_SMS=STATE_MOTOR3_RTCOF5_TRIP_SMS;
								sms_offlag2=1;
							}
							else if(number_enu_data[2]==22)
							{
								nSTATE_MOTOR3_SMS=STATE_MOTOR3_RTCOF6_TRIP_SMS;
								sms_offlag2=1;
							}
							else if(number_enu_data[2]==23)
							{
								nSTATE_MOTOR3_SMS=STATE_MOTOR3_CYCLIC_TRIP_SMS;
								trip_flag2=1;
							}
							else if(number_enu_data[2]==24)
							{
								nSTATE_MOTOR3_SMS=STATE_MOTOR3_MAX_TRIP_SMS;
								trip_flag2=1;
							}
					//		{
							
					//		prev_onof_M1=myatoi(StrTokStr1[6]);
					//		Prev_M1_state=myatoi(StrTokStr1[7]);
					//		tripflag[0]=myatoi(StrTokStr1[8]);
					//		}
							else if(number_enu_data[2]==25)
							nSTATE_MOTOR3_SMS=STATE_MOTOR3_OFF_TARGET;
							else if(number_enu_data[2]==26)
							nSTATE_MOTOR3_SMS=STATE_MOTOR3OF_3PHASE_ONLY_SMS;
								else if(number_enu_data[2]==27)
								{
									nSTATE_MOTOR3_SMS=STATE_MOTOR3_ON_SWITCH_TRIP_SMS;
									//WorkingOn3Phas=uart_read_buf[9]-'0';
									sms_offlag2=1;
								}
								else if(number_enu_data[2]==28)
								{
									nSTATE_MOTOR3_SMS=STATE_MOTOR3_LEVELSCAN_UPPERTANK_TRIP_SMS;
									//WorkingOn3Phas=uart_read_buf[9]-'0';
									level_percent=myatoi(StrTokStr1[4]);
									trip_flag2=1;
								}
								else if(number_enu_data[2]==29)
								{
									nSTATE_MOTOR3_SMS=STATE_MOTOR3_LEVELSCAN_LOWERTANK_TRIP_SMS;
									//WorkingOn3Phas=uart_read_buf[9]-'0';
									level_percent=myatoi(StrTokStr1[4]);
									trip_flag2=1;
								}
												
							prev_number_enu_data[2] = number_enu_data[2];
							number_enu_data[2] =0;
							SendSmsToAll = 1;
						//	sms_offlag2 = 1;  
						}
						sprintf(buf,"\n\rtest : s_nTimerSettings.m_OlAmpsIII[3]=%02.01f, s_nTimerSettings.m_OlAmpsII[3]=%02.01f, TripCurrent=%02.01f\n\r",s_nTimerSettings.m_OlAmpsIII[3],s_nTimerSettings.m_OlAmpsII[3],TripCurrent);
						sAPI_UartPrintf(buf);	
						
						sprintf(buf,"\n\rtest : prev_onof_M1=%d, Prev_M1_state =%d\n\r",prev_onof_M1,Prev_M1_state);
						sAPI_UartPrintf(buf);
					}
					
					#endif
					if(uart_read_buf[1] == '<' && uart_read_buf[2] == 'U' && uart_read_buf[3] == '>')	{
						sendtoMcuack=1;
						sendtoMcu=0;					   
					}
					if(uart_read_buf[1] == 'P' && uart_read_buf[3] == 'P')
					{
						s_nMSettings.m_pumpno_send=1;
						s_nMSettings.m_settings_req_flag=1;
						s_nMSettings.m_settings_count=1;
					//	first_time_data_req_flag=1;
					}
					if(uart_read_buf[1] == '<' && uart_read_buf[2] == 'A' && uart_read_buf[3] == '>')
					{	
						s_nMSettings.m_settings_req_flag=1;
					if(s_nMSettings.m_uart_send_flag==1)
					{
						s_nMSettings.m_uart_send_flag=0;
						s_nMSettings.m_settings_count++;
					}
					//if(s_nMSettings.m_settings_count==10 || s_nMSettings.m_settings_count==15 || s_nMSettings.m_settings_count==20)
					//s_nMSettings.m_pumpno_send++; 
					sprintf(buf,"\n\r HDebug2:s_nMSettings.m_settings_count = %d  s_nMSettings.m_pumpno_send = %d g_no_of_pumps =%d\n\r",s_nMSettings.m_settings_count,s_nMSettings.m_pumpno_send,g_no_of_pumps);
						sAPI_UartPrintf(buf);
					//	s_nMSettings.m_settings_count=1;
						s_nMSettings.m_pumpno_send=1;
						if(s_nMSettings.m_pumpno_send>g_no_of_pumps)
						{
						s_nMSettings.m_pumpno_send=1;
						s_nMSettings.m_settings_req_flag=0;
						s_nMSettings.m_settings_count=0;
					//	uart_send_flag=1;
						}
						sprintf(buf,"\n\r HDebug2:s_nMSettings.m_settings_req_flag = %d  s_nMSettings.m_pumpno_send = %d g_no_of_pumps =%d\n\r",s_nMSettings.m_settings_req_flag,s_nMSettings.m_pumpno_send,g_no_of_pumps);
						sAPI_UartPrintf(buf);
					}
/* 					if(uart_read_buf[1] == '<' && uart_read_buf[2] == 'B' && uart_read_buf[3] == '>')
					{	
					s_nMSettings.m_settings_req_flag=1;
					s_nMSettings.m_settings_count=3;
					}
					if(uart_read_buf[1] == '<' && uart_read_buf[2] == 'C' && uart_read_buf[3] == '>')
					{	
					s_nMSettings.m_settings_req_flag=1;
					s_nMSettings.m_settings_count=4;
					}
					if(uart_read_buf[1] == '<' && uart_read_buf[2] == 'D' && uart_read_buf[3] == '>')
					{	
					s_nMSettings.m_settings_req_flag=1;
					s_nMSettings.m_settings_count=5;
					}
					if(uart_read_buf[1] == '<' && uart_read_buf[2] == 'E' && uart_read_buf[3] == '>')
					{	
					s_nMSettings.m_settings_req_flag=1;
					s_nMSettings.m_settings_count=6;
					}
					if(uart_read_buf[1] == '<' && uart_read_buf[2] == 'F' && uart_read_buf[3] == '>')
					{	
					s_nMSettings.m_settings_req_flag=1;
					s_nMSettings.m_settings_count=7;
					}
					if(uart_read_buf[1] == '<' && uart_read_buf[2] == 'G' && uart_read_buf[3] == '>')
					{	
					s_nMSettings.m_settings_req_flag=1;
					s_nMSettings.m_settings_count=8;
					}
					if(uart_read_buf[1] == '<' && uart_read_buf[2] == 'H' && uart_read_buf[3] == '>')
					{
						s_nMSettings.m_pumpno_send++;
						s_nMSettings.m_settings_count=1;
						
						if(s_nMSettings.m_pumpno_send>g_no_of_pumps)
						{
						s_nMSettings.m_pumpno_send=1;
						s_nMSettings.m_settings_req_flag=0;
						s_nMSettings.m_settings_count=0;
					//	uart_send_flag=1;
						}
						sprintf(buf,"\n\r HDebug2:s_nMSettings.m_settings_req_flag = %d  s_nMSettings.m_pumpno_send = %d g_no_of_pumps =%d\n\r",s_nMSettings.m_settings_req_flag,s_nMSettings.m_pumpno_send,g_no_of_pumps);
						sAPI_UartPrintf(buf);
						
					} */
					if(uart_read_buf[0] == '$' && uart_read_buf[1] == 'K' )
						{
									sprintf(buf,"\n\r before cont_version:%d.%d.%d,%01d,%02d,%02d,%02d\n",
								nMSettings.CONT_VER[0],nMSettings.CONT_VER[1],nMSettings.CONT_VER[2],l_mtrno,nMSettings.act_rem_DrHr[l_mtrno],nMSettings.act_rem_DrMin[l_mtrno],nMSettings.act_rem_DrSec[l_mtrno]);
								sAPI_UartPrintf(buf);
									int StrTokStrVer = 0;
									memset(nMSettings.act_rem_DrHr,0,3);
									memset(nMSettings.act_rem_DrMin,0,3);
									memset(nMSettings.act_rem_DrSec,0,3);
									
									sprintf(buf,"\n\r before1 cont_version:%d.%d.%d,%01d,%02d,%02d,%02d\n",
								nMSettings.CONT_VER[0],nMSettings.CONT_VER[1],nMSettings.CONT_VER[2],l_mtrno,nMSettings.act_rem_DrHr[l_mtrno],nMSettings.act_rem_DrMin[l_mtrno],nMSettings.act_rem_DrSec[l_mtrno]);
								sAPI_UartPrintf(buf);
							
								Pch1 = strtok((char *)uart_read_buf, (char *)"," );
								StrTokStrVer =0 ;
								while( Pch1 != NULL )
								{
								strcpy(StrTokStr1[StrTokStrVer],Pch1);
								//sprintf(buf,"\n\r StrTokStr1[%d] is %s \n",StrTokStrVer,StrTokStr1[StrTokStrVer]);
								//sAPI_UartPrintf(buf);
								StrTokStrVer++;
								Pch1 = strtok( NULL, "," );
								}
								nMSettings.CONT_VER[0]=myatoi(StrTokStr1[1]);
								nMSettings.CONT_VER[1]=myatoi(StrTokStr1[2]);
								nMSettings.CONT_VER[2]=myatoi(StrTokStr1[3]);
								l_mtrno=myatoi(StrTokStr1[4]);
								nMSettings.act_rem_DrHr[l_mtrno]= myatoi(StrTokStr1[5]);
								nMSettings.act_rem_DrMin[l_mtrno]= myatoi(StrTokStr1[6]);
								nMSettings.act_rem_DrSec[l_mtrno]= myatoi(StrTokStr1[7]);
								sprintf(buf,"cont_version:%d.%d.%d,%01d,%02d,%02d,%02d\n",
								nMSettings.CONT_VER[0],nMSettings.CONT_VER[1],nMSettings.CONT_VER[2],l_mtrno,nMSettings.act_rem_DrHr[l_mtrno],nMSettings.act_rem_DrMin[l_mtrno],nMSettings.act_rem_DrSec[l_mtrno]);
								sAPI_UartPrintf(buf);
								if(nMSettings.act_rem_DrHr[l_mtrno] >= 24)
									nMSettings.act_rem_DrHr[l_mtrno]=10;
								if(nMSettings.act_rem_DrMin[l_mtrno] >= 60)
									nMSettings.act_rem_DrMin[l_mtrno]=60;
								if(nMSettings.act_rem_DrSec[l_mtrno] >= 60)
									nMSettings.act_rem_DrSec[l_mtrno]=60;
								if(Motoronflag[l_mtrno] == 1)
								{
									nMSettings.act_rem_DrHr[l_mtrno]=0;
									nMSettings.act_rem_DrMin[l_mtrno]=0;
									nMSettings.act_rem_DrSec[l_mtrno]=0;
								} 
						}
 					if(uart_read_buf[0] == '$' && uart_read_buf[1] == 'V' )
						{
									int StrTokStrVer = 0;
							
								Pch1 = strtok((char *)uart_read_buf, (char *)"," );
								StrTokStrVer =0 ;
								while( Pch1 != NULL )
								{
								strcpy(StrTokStr1[StrTokStrVer],Pch1);
								StrTokStrVer++;
								Pch1 = strtok( NULL, "," );
								}
								RVoltage1=myatoi(StrTokStr1[1]);
								YVoltage1=myatoi(StrTokStr1[2]);
								BVoltage1=myatoi(StrTokStr1[3]);
								nCurretnCond.Rcurrent=myatof(StrTokStr1[4]);
								nCurretnCond.Ycurrent=myatof(StrTokStr1[5]);
								nCurretnCond.Bcurrent=myatof(StrTokStr1[6]);
								
										if(YVoltage1<115)
											checkpower_act=2;
										else
											checkpower_act=3;
										
										checkpower=checkpower_act;
										
										/* if(RVoltage1 <=50 && YVoltage1 <=50 && BVoltage1 <=50)
											PowerCurrentCondition=1;
										else */ 
										//PowerCurrentCondition=0;	
								sprintf(buf,"\n\r nCurretnCond.RVoltage = %d  nCurretnCond.Rcurrent = %f \n\r",nCurretnCond.RVoltage,nCurretnCond.Rcurrent);
							sAPI_UartPrintf(buf);
						} 
						
						if(uart_read_buf[0] == '$' && uart_read_buf[1] == 'R' && uart_read_buf[2] == 'C' && uart_read_buf[3] == 'N' )
						{
							int StrTokStrVer = 0,j=0,i=0,k=0;
							Pch1 = strtok((char *)uart_read_buf, (char *)"," );
							StrTokStrVer =0 ;
							while( Pch1 != NULL )
							{
								strcpy(StrTokStr1[StrTokStrVer],Pch1);
								sprintf(buf,"Rc--StrTokStr1[%d]=%s\n",StrTokStrVer,StrTokStr1[StrTokStrVer]);
								sAPI_UartPrintf(buf);
								StrTokStrVer++;
								Pch1 = strtok( NULL, ",");
							}
							ValveOnOffReason=myatoi(StrTokStr1[1]);
							LogValveNo=myatoi(StrTokStr1[2]);
							
							sprintf(buf,"$<CCK>\n\r");
							sAPI_UartWrite(SC_UART,(UINT8*)buf,strlen(buf));
							sAPI_UartPrintf(buf);
							
							//ValveStatus[LogValveNo-1]=1;
							memset(Buffer1,NULL,sizeof(Buffer1));
							memset(Buffer2,NULL,sizeof(Buffer2));
							if(ValveOnOffReason==1)
							sprintf(Buffer1,"{\r\n\"cC\":\"%s\",\r\n\"cM\":\"MOTOR1 ON VALVE%d ON \",\r\n\"cD\":\"%04d-%02d-%02d\",\r\n\"cT\":\"%02d:%02d:%02d\",\r\n\"mC\":\"SMS\"\r\n}",IMEI,LogValveNo,datetime.tm_year,datetime.tm_mon,datetime.tm_mday,datetime.tm_hour,datetime.tm_min,datetime.tm_sec);
							else if(ValveOnOffReason==2)
							sprintf(Buffer1,"{\r\n\"cC\":\"%s\",\r\n\"cM\":\"MOTOR1 ON VALVE%d ON BECAUSE OF STANDALONE\",\r\n\"cD\":\"%04d-%02d-%02d\",\r\n\"cT\":\"%02d:%02d:%02d\",\r\n\"mC\":\"SMS\"\r\n }",IMEI,LogValveNo,datetime.tm_year,datetime.tm_mon,datetime.tm_mday,datetime.tm_hour,datetime.tm_min,datetime.tm_sec);
							else if(ValveOnOffReason==3)
							sprintf(Buffer1,"{\r\n\"cC\":\"%s\",\r\n\"cM\":\"MOTOR1 ON LIGHT%d ON BECAUSE OF STANDALONE\",\r\n\"cD\":\"%04d-%02d-%02d\",\r\n\"cT\":\"%02d:%02d:%02d\",\r\n\"mC\":\"SMS\"\r\n }",IMEI,LogValveNo,datetime.tm_year,datetime.tm_mon,datetime.tm_mday,datetime.tm_hour,datetime.tm_min,datetime.tm_sec);
							sprintf(TCPwifiStrNumber[Nooftcprecvd].TCPWifigprsstr,"%s",Buffer1);
                            sprintf(buf,"\n\rTCPwifiStrNumber[%d].TCPWifigprsstr>>:%s",Nooftcprecvd,TCPwifiStrNumber[Nooftcprecvd].TCPWifigprsstr);
							sAPI_UartPrintf(buf);
							Nooftcprecvd++;
							sgetflag_1=1;
							
						}
						
						if(uart_read_buf[0] == '$' && uart_read_buf[1] == 'R' && uart_read_buf[2] == 'C' && uart_read_buf[3] == 'F' )
						{
							int StrTokStrVer = 0,j=0,i=0,k=0;
							Pch1 = strtok((char *)uart_read_buf, (char *)"," );
							StrTokStrVer =0 ;
							while( Pch1 != NULL )
							{
								strcpy(StrTokStr1[StrTokStrVer],Pch1);
								sprintf(buf,"Rc--StrTokStr1[%d]=%s\n",StrTokStrVer,StrTokStr1[StrTokStrVer]);
								sAPI_UartPrintf(buf);
								StrTokStrVer++;
								Pch1 = strtok( NULL, "," );
							}
							ValveOnOffReason=myatoi(StrTokStr1[1]);
							LogValveNo=myatoi(StrTokStr1[2]);
							ValveOnTime[0]=myatoi(StrTokStr1[3]);
							ValveOnTime[1]=myatoi(StrTokStr1[4]);
							ValveOffTime[0]=myatoi(StrTokStr1[5]);
							ValveOffTime[1]=myatoi(StrTokStr1[6]);
							ValveOnDueTime[0]=myatoi(StrTokStr1[7]);
							ValveOnDueTime[1]=myatoi(StrTokStr1[8]);
							
							sprintf(buf,"$<CCK>\n\r");
							sAPI_UartWrite(SC_UART,(UINT8*)buf,strlen(buf));
							sAPI_UartPrintf(buf);
							
							memset(Buffer1,NULL,sizeof(Buffer1));
							memset(Buffer2,NULL,sizeof(Buffer2));
							memset(Buffer3,NULL,sizeof(Buffer3));
							//ValveStatus[LogValveNo-1]=1;
							if(ValveOnOffReason==1)
							{
								sprintf(Buffer1,"{\r\n\"cC\":\"%s\",\r\n\"cM\":\"VALVE%02d ON ,%02d:%02d:00,VALVE%02d OFF ",IMEI,LogValveNo,ValveOnTime[0],ValveOnTime[1],LogValveNo,ValveOffTime[0],ValveOffTime[1],ValveOnDueTime[0],ValveOnDueTime[1],LogValveNo);
								//sprintf(Buffer1,"{\r\n\"cC\":\"%s\",\r\n\"cM\":\"MOTOR1 ON VALVE%02d ON BECAUSE OF RTC PROGRAM,%02d:%02d:00,VALVE%02d OFF BECAUSE OF RTC ",IMEI,LogValveNo,ValveOnTime[0],ValveOnTime[1],LogValveNo,ValveOffTime[0],ValveOffTime[1],ValveOnDueTime[0],ValveOnDueTime[1],LogValveNo);
								sprintf(Buffer2,",%02d:%02d:00,%02d:%02d:00,021 DATE=%02d/%02d/%04d  TIME=%02d:%02d:%02d\",\r\n",ValveOffTime[0],ValveOffTime[1],ValveOnDueTime[0],ValveOnDueTime[1],datetime.tm_mday,datetime.tm_mon,datetime.tm_year,datetime.tm_hour, datetime.tm_min, datetime.tm_sec);
								sprintf(Buffer3,"\"cD\":\"%04d-%02d-%02d\",\r\n\"cT\":\"%02d:%02d:%02d\",\r\n\"mC\":\"SMS\"\r\n}",datetime.tm_year,datetime.tm_mon,datetime.tm_mday,datetime.tm_hour,datetime.tm_min,datetime.tm_sec);
							}
							else if(ValveOnOffReason==2)
							{
								sprintf(Buffer1,"{\r\n\"cC\":\"%s\",\r\n\"cM\":\"VALVE%02d ON BECAUSE OF STANDALONE,%02d:%02d:00,VALVE%02d OFF BECAUSE OF ",IMEI,LogValveNo,ValveOnTime[0],ValveOnTime[1],LogValveNo,ValveOffTime[0],ValveOffTime[1],ValveOnDueTime[0],ValveOnDueTime[1],LogValveNo);
								//sprintf(Buffer1,"{\r\n\"cC\":\"%s\",\r\n\"cM\":\"MOTOR1 ON VALVE%02d ON BECAUSE OF STANDALONE,%02d:%02d:00,VALVE%02d OFF BECAUSE OF ",IMEI,LogValveNo,ValveOnTime[0],ValveOnTime[1],LogValveNo,ValveOffTime[0],ValveOffTime[1],ValveOnDueTime[0],ValveOnDueTime[1],LogValveNo);
								sprintf(Buffer2,"STANDALONE,%02d:%02d:00,%02d:%02d:00,022 DATE=%02d/%02d/%04d  TIME=%02d:%02d:%02d\",\r\n",ValveOffTime[0],ValveOffTime[1],ValveOnDueTime[0],ValveOnDueTime[1],datetime.tm_mday,datetime.tm_mon,datetime.tm_year,datetime.tm_hour, datetime.tm_min, datetime.tm_sec);
								sprintf(Buffer3,"\"cD\":\"%04d-%02d-%02d\",\r\n\"cT\":\"%02d:%02d:%02d\",\r\n\"mC\":\"SMS\"\r\n}",datetime.tm_year,datetime.tm_mon,datetime.tm_mday,datetime.tm_hour,datetime.tm_min,datetime.tm_sec);
							}
							else if(ValveOnOffReason==3)
							{
								sprintf(Buffer1,"{\r\n\"cC\":\"%s\",\r\n\"cM\":\"LIGHT%02d ON BECAUSE OF STANDALONE,%02d:%02d:00,LIGHT%02d OFF BECAUSE OF ",IMEI,LogValveNo,ValveOnTime[0],ValveOnTime[1],LogValveNo,ValveOffTime[0],ValveOffTime[1],ValveOnDueTime[0],ValveOnDueTime[1],LogValveNo);
								//sprintf(Buffer1,"{\r\n\"cC\":\"%s\",\r\n\"cM\":\"MOTOR1 ON VALVE%02d ON BECAUSE OF STANDALONE,%02d:%02d:00,VALVE%02d OFF BECAUSE OF ",IMEI,LogValveNo,ValveOnTime[0],ValveOnTime[1],LogValveNo,ValveOffTime[0],ValveOffTime[1],ValveOnDueTime[0],ValveOnDueTime[1],LogValveNo);
								sprintf(Buffer2,"STANDALONE,%02d:%02d:00,%02d:%02d:00,022 DATE=%02d/%02d/%04d  TIME=%02d:%02d:%02d\",\r\n",ValveOffTime[0],ValveOffTime[1],ValveOnDueTime[0],ValveOnDueTime[1],datetime.tm_mday,datetime.tm_mon,datetime.tm_year,datetime.tm_hour, datetime.tm_min, datetime.tm_sec);
								sprintf(Buffer3,"\"cD\":\"%04d-%02d-%02d\",\r\n\"cT\":\"%02d:%02d:%02d\",\r\n\"mC\":\"SMS\"\r\n}",datetime.tm_year,datetime.tm_mon,datetime.tm_mday,datetime.tm_hour,datetime.tm_min,datetime.tm_sec);
							}
							//sprintf(buf,"{\r\n\"cC\":\"%s\",\r\n\"cM\": VALVE%02d ON BECAUSE OF STANDALONE,%02d:%02d:00,VALVE%02d OFF BECAUSE OF STANDALONE,%02d:%02d:00,%02d:%02d:%00,021 DATE=%02d/%02d/%04d  TIME=%02d:%02d:%02d ,\r\n\"cD\":\"%04d-%02d-%02d\",\r\n\"cT\":\"%02d:%02d:%02d\",\r\n\"mC\":\"SMS\"\r\n",IMEI,LogValveNo,ValveOnTime[0],ValveOnTime[1],LogValveNo,ValveOffTime[0],ValveOffTime[1],ValveOnDueTime[0],ValveOnDueTime[1],datetime.tm_mday,datetime.tm_mon,datetime.tm_year,datetime.tm_hour, datetime.tm_min, datetime.tm_sec,datetime.tm_year,datetime.tm_mon,datetime.tm_mday,datetime.tm_hour,datetime.tm_min,datetime.tm_sec);
							sprintf(TCPwifiStrNumber[Nooftcprecvd].TCPWifigprsstr,"%s%s%s",Buffer1,Buffer2,Buffer3);
                            //sprintf(buf,"\n\rTCPwifiStrNumber1[%d].TCPWifigprsstr1>>:%s",Nooftcprecvd1,TCPwifiStrNumber1[Nooftcprecvd1].TCPWifigprsstr1);
							sAPI_UartPrintf(Buffer1);
							sAPI_UartPrintf(Buffer2);
							sAPI_UartPrintf(Buffer3);
							Nooftcprecvd++;
							sgetflag_1=1;
						}
						if(uart_read_buf[0] == '$' && uart_read_buf[1] == 'H' && uart_read_buf[2] == 'I' )
						{
									int StrTokStrVer = 0;
							
								Pch1 = strtok((char *)uart_read_buf, (char *)"," );
								StrTokStrVer =0 ;
								while( Pch1 != NULL )
								{
								strcpy(StrTokStr1[StrTokStrVer],Pch1);
								sprintf(buf,"HI--StrTokStr1[%d]=%s\n",StrTokStrVer,StrTokStr1[StrTokStrVer]);
								sAPI_UartPrintf(buf);
								StrTokStrVer++;
								Pch1 = strtok( NULL, "," );
								}
								LiveValveTimerHr[4]=myatoi(StrTokStr1[1]);
								LiveValveTimerMin[4]=myatoi(StrTokStr1[2]);
								LiveValveTimerHr[5]=myatoi(StrTokStr1[3]);
								LiveValveTimerMin[5]=myatoi(StrTokStr1[4]);
								LiveValveTimerHr[6]=myatoi(StrTokStr1[5]);
								LiveValveTimerMin[6]=myatoi(StrTokStr1[6]);
								LiveValveTimerHr[7]=myatoi(StrTokStr1[7]);
								LiveValveTimerMin[7]=myatoi(StrTokStr1[8]);
								
								
								sprintf(buf,"\n\r valve 4:%d--%d\n\rvalve 5:%d--%d\n\rvalve 6:%d--%d\n\rvalve 7:%d--%d\n\r",LiveValveTimerHr[4],LiveValveTimerMin[4],LiveValveTimerHr[5],LiveValveTimerMin[5],LiveValveTimerHr[6],LiveValveTimerMin[6],LiveValveTimerHr[7],LiveValveTimerMin[7]);
						sAPI_UartPrintf(buf);
								//sprintf(buf,"PrvValveStatus[3] %d ValveStatus[3] %d PrvValveStatus[4] %d ValveStatus[4] %d PrvValveStatus[5] %d VValveStatus[5] %d",PrvValveStatus[3],ValveStatus[3],PrvValveStatus[4],ValveStatus[4],PrvValveStatus[5],ValveStatus[5]);
								 //sAPI_UartPrintf(buf);
								// if((PrvValveStatus[3]!=ValveStatus[3])|| (PrvValveStatus[4]!=ValveStatus[4])|| (PrvValveStatus[5]!=ValveStatus[5]))
								{
									//if(livedataflag==1)
									//{livedataflag1=1;livedataflagcount1=0;}	
									//else
									//livedataflag=1;
									//PrvValveStatus[3]=ValveStatus[3];
									//PrvValveStatus[4]=ValveStatus[4];
									//PrvValveStatus[5]=ValveStatus[5];
								} 
								
						//sprintf(buf,"\n\r valve 4:%d--%d--%d\n\rvalve 5:%d--%d--%d\n\rvalve 6:%d--%d--%d\n\r",LiveValveTimerHr[3],LiveValveTimerMin[3],ValveStatus[3],LiveValveTimerHr[4],LiveValveTimerMin[4],ValveStatus[4],LiveValveTimerHr[5],LiveValveTimerMin[5],ValveStatus[5]);
						//sAPI_UartPrintf(buf);
						
						} 
						if(uart_read_buf[0] == '$' && uart_read_buf[1] == 'G' && uart_read_buf[2] == 'I' )
						{
									int StrTokStrVer = 0;
							
								Pch1 = strtok((char *)uart_read_buf, (char *)"," );
								StrTokStrVer =0 ;
								while( Pch1 != NULL )
								{
								strcpy(StrTokStr1[StrTokStrVer],Pch1);
								sprintf(buf,"StrTokStr1[%d]=%s",StrTokStrVer,StrTokStr1[StrTokStrVer]);
								sAPI_UartPrintf(buf);
								StrTokStrVer++;
								Pch1 = strtok( NULL, "," );
								
								}
								#if 1
								ValveSetting=myatoi(StrTokStr1[1]);
								LiveValveTimerHr[0]=myatoi(StrTokStr1[2]);
								LiveValveTimerMin[0]=myatoi(StrTokStr1[3]);
								LiveValveTimerHr[1]=myatoi(StrTokStr1[4]);
								LiveValveTimerMin[1]=myatoi(StrTokStr1[5]);
								LiveValveTimerHr[2]=myatoi(StrTokStr1[6]);
								LiveValveTimerMin[2]=myatoi(StrTokStr1[7]);
								LiveValveTimerHr[3]=myatoi(StrTokStr1[8]);
								LiveValveTimerMin[3]=myatoi(StrTokStr1[9]);
								sprintf(buf,"\n\r valve 1:%d--%d\n\rvalve 2:%d--%d\n\rvalve 3:%d--%d\n\rvalve 4:%d--%d\n\r",LiveValveTimerHr[0],LiveValveTimerMin[0],LiveValveTimerHr[1],LiveValveTimerMin[1],LiveValveTimerHr[2],LiveValveTimerMin[2],LiveValveTimerHr[3],LiveValveTimerMin[3]);
								sAPI_UartPrintf(buf);
								//sprintf(buf,"PrvValveStatus[0] %d ValveStatus[0] %d PrvValveStatus[1] %d ValveStatus[1] %d PrvValveStatus[2] %d ValveStatus[2] %d",PrvValveStatus[0],ValveStatus[0],PrvValveStatus[1],ValveStatus[1],PrvValveStatus[2],ValveStatus[2]);
								// sAPI_UartPrintf(buf);								
								//if((PrvValveStatus[0]!=ValveStatus[0])|| (PrvValveStatus[1]!=ValveStatus[1])|| (PrvValveStatus[2]!=ValveStatus[2]))
								{
									//if(livedataflag==1)
									//{livedataflag1=1;livedataflagcount1=0;}	
									//else
									//livedataflag=1;
									//PrvValveStatus[0]=ValveStatus[0];
									//PrvValveStatus[1]=ValveStatus[1];
									//PrvValveStatus[2]=ValveStatus[2];
								}
								
						//sprintf(buf,"\n\r valve 1:%d--%d--%d\n\rvalve 2:%d--%d--%d\n\rvalve 3:%d--%d--%d\n\r",LiveValveTimerHr[0],LiveValveTimerMin[0],ValveStatus[0],LiveValveTimerHr[1],LiveValveTimerMin[1],ValveStatus[1],LiveValveTimerHr[2],LiveValveTimerMin[2],ValveStatus[2]);
						//sAPI_UartPrintf(buf);
						#endif
						
						} 
						#if 0
						if(uart_read_buf[0] == '$' && uart_read_buf[1] == 'H' && uart_read_buf[2] == 'I' )
						{
									int StrTokStrVer = 0;
							
								Pch1 = strtok((char *)uart_read_buf, (char *)"," );
								StrTokStrVer =0 ;
								while( Pch1 != NULL )
								{
								strcpy(StrTokStr1[StrTokStrVer],Pch1);
								StrTokStrVer++;
								Pch1 = strtok( NULL, "," );
								}
								LiveValveTimerHr[3]=myatoi(StrTokStr1[2]);
								LiveValveTimerMin[3]=myatoi(StrTokStr1[3]);
								ValveStatus[3]=myatoi(StrTokStr1[4]);
								LiveValveTimerHr[4]=myatoi(StrTokStr1[5]);
								LiveValveTimerMin[4]=myatoi(StrTokStr1[6]);
								ValveStatus[4]=myatoi(StrTokStr1[7]);
								LiveValveTimerHr[5]=myatoi(StrTokStr1[8]);
								LiveValveTimerMin[5]=myatoi(StrTokStr1[9]);
								ValveStatus[5]=myatoi(StrTokStr1[10]);
								
								
								sprintf(buf,"\n\r valve 4:%d--%d--%d\n\rvalve 5:%d--%d--%d\n\rvalve 6:%d--%d--%d\n\r",LiveValveTimerHr[3],LiveValveTimerMin[3],ValveStatus[3],LiveValveTimerHr[4],LiveValveTimerMin[4],ValveStatus[4],LiveValveTimerHr[5],LiveValveTimerMin[5],ValveStatus[5]);
						sAPI_UartPrintf(buf);								
								//if((PrvValveStatus[3]!=ValveStatus[3])|| (PrvValveStatus[4]!=ValveStatus[4])|| (PrvValveStatus[5]!=ValveStatus[5]))
								{
									//if(livedataflag==1)
									//{livedataflag1=1;livedataflagcount1=0;}	
									//else
									//livedataflag=1;
									//PrvValveStatus[3]=ValveStatus[3];
									//PrvValveStatus[4]=ValveStatus[4];
									//PrvValveStatus[5]=ValveStatus[5];
								}
								
						sprintf(buf,"\n\r valve 4:%d--%d--%d\n\rvalve 5:%d--%d--%d\n\rvalve 6:%d--%d--%d\n\r",LiveValveTimerHr[3],LiveValveTimerMin[3],ValveStatus[3],LiveValveTimerHr[4],LiveValveTimerMin[4],ValveStatus[4],LiveValveTimerHr[5],LiveValveTimerMin[5],ValveStatus[5]);
						sAPI_UartPrintf(buf);
						
						} 
						#endif
						if(uart_read_buf[0] == '$' && uart_read_buf[1] == 'I' && uart_read_buf[2] == 'I' )
						{
									int StrTokStrVer = 0;
							
								Pch1 = strtok((char *)uart_read_buf, (char *)"," );
								StrTokStrVer =0 ;
								while( Pch1 != NULL )
								{
								strcpy(StrTokStr1[StrTokStrVer],Pch1);
								StrTokStrVer++;
								Pch1 = strtok( NULL, "," );
								}
								LiveValveTimerHr[8]=myatoi(StrTokStr1[1]);
								LiveValveTimerMin[8]=myatoi(StrTokStr1[2]);
								LiveValveTimerHr[9]=myatoi(StrTokStr1[3]);
								LiveValveTimerMin[9]=myatoi(StrTokStr1[4]);
								
								sprintf(buf,"\n\r valve 9:%d--%d\n\rvalve 10:%d--%d\n\r",LiveValveTimerHr[8],LiveValveTimerMin[8],LiveValveTimerHr[9],LiveValveTimerMin[9]);
								sAPI_UartPrintf(buf);
								//sprintf(buf,"PrvValveStatus[6] %d ValveStatus[6] %d PrvValveStatus[7] %d ValveStatus[7] %d PrvValveStatus[8] %d ValveStatus[8] %d",PrvValveStatus[6],ValveStatus[6],PrvValveStatus[7],ValveStatus[7],PrvValveStatus[8],ValveStatus[8]);
								 //sAPI_UartPrintf(buf);						
								//if((PrvValveStatus[6]!=ValveStatus[6])|| (PrvValveStatus[7]!=ValveStatus[7])|| (PrvValveStatus[8]!=ValveStatus[8]))
								{
									//if(livedataflag==1)
									//{livedataflag1=1;livedataflagcount1=0;}	
									//else
									//livedataflag=1;
									//PrvValveStatus[6]=ValveStatus[6];
									//PrvValveStatus[7]=ValveStatus[7];
									//PrvValveStatus[8]=ValveStatus[8];
								}
								
						sprintf(buf,"\n\r valve 7:%d--%d--%d\n\rvalve 8:%d--%d--%d\n\rvalve 9:%d--%d--%d\n\r",LiveValveTimerHr[6],LiveValveTimerMin[6],ValveStatus[6],LiveValveTimerHr[7],LiveValveTimerMin[7],ValveStatus[7],LiveValveTimerHr[8],LiveValveTimerMin[8],ValveStatus[8]);
						sAPI_UartPrintf(buf);
						
						} 
						if(uart_read_buf[0] == '$' && uart_read_buf[1] == 'J' && uart_read_buf[2] == 'I' )
						{
									int StrTokStrVer = 0;
							
								Pch1 = strtok((char *)uart_read_buf, (char *)"," );
								StrTokStrVer =0 ;
								while( Pch1 != NULL )
								{
								strcpy(StrTokStr1[StrTokStrVer],Pch1);
								StrTokStrVer++;
								Pch1 = strtok( NULL, "," );
								}
								CyclicIntevelFlag=myatoi(StrTokStr1[1]);
								CyclicIntevelHr=myatoi(StrTokStr1[2]);
								CyclicIntevelMin=myatoi(StrTokStr1[3]);
								RecCyclicIntevelHr=myatoi(StrTokStr1[4]);
								RecCyclicIntevelMin=myatoi(StrTokStr1[5]);
								RecCyclicIntevelsec=myatoi(StrTokStr1[6]);
								sprintf(buf,"\n\r CYC On :%d--%d--%d\n\rRemaning:%d--%d--%d\n\r",CyclicIntevelFlag,CyclicIntevelHr,CyclicIntevelMin,RecCyclicIntevelHr,RecCyclicIntevelMin,RecCyclicIntevelsec);
								sAPI_UartPrintf(buf);
								if(RecCycFlag!=CyclicIntevelFlag)
								{
									RecCycFlag=CyclicIntevelFlag;
									livedataflag=1;
								}
								//sprintf(buf,"PrvValveStatus[6] %d ValveStatus[6] %d PrvValveStatus[7] %d ValveStatus[7] %d PrvValveStatus[8] %d ValveStatus[8] %d",PrvValveStatus[6],ValveStatus[6],PrvValveStatus[7],ValveStatus[7],PrvValveStatus[8],ValveStatus[8]);
								 //sAPI_UartPrintf(buf);						
								//if((PrvValveStatus[6]!=ValveStatus[6])|| (PrvValveStatus[7]!=ValveStatus[7])|| (PrvValveStatus[8]!=ValveStatus[8]))
								{
									//if(livedataflag==1)
									//{livedataflag1=1;livedataflagcount1=0;}	
									//else
									//livedataflag=1;
									//PrvValveStatus[6]=ValveStatus[6];
									//PrvValveStatus[7]=ValveStatus[7];
									//PrvValveStatus[8]=ValveStatus[8];
								}
								
						sprintf(buf,"\n\r valve 7:%d--%d--%d\n\rvalve 8:%d--%d--%d\n\rvalve 9:%d--%d--%d\n\r",LiveValveTimerHr[6],LiveValveTimerMin[6],ValveStatus[6],LiveValveTimerHr[7],LiveValveTimerMin[7],ValveStatus[7],LiveValveTimerHr[8],LiveValveTimerMin[8],ValveStatus[8]);
						sAPI_UartPrintf(buf);
						
						}
						
						if(uart_read_buf[0] == '$' && uart_read_buf[1] == 'C' && uart_read_buf[2] == 'I' )
						{
							int StrTokStrVer = 0,Val=0;
							float Fval=0;
							Pch1 = strtok((char *)uart_read_buf, (char *)"," );
							StrTokStrVer =0 ;
							while( Pch1 != NULL )
							{
							strcpy(StrTokStr1[StrTokStrVer],Pch1);
							StrTokStrVer++;
							Pch1 = strtok( NULL, "," );
							}
							SetSerial[0]=myatoi(StrTokStr1[1]);
							SetSerial[1]=myatoi(StrTokStr1[2]);
							Mos[0]=(float)myatoi(StrTokStr1[3])/10;
							Fval=myatoi(StrTokStr1[3])%10;
							Mos[0]=Mos[0]+Fval;
							
							Mos[1]=(float)myatoi(StrTokStr1[4])/10;
							Fval=myatoi(StrTokStr1[4])%10;
							Mos[1]=Mos[1]+Fval;
							
							Mos[2]=(float)myatoi(StrTokStr1[5])/10;
							Fval=myatoi(StrTokStr1[5])%10;
							Mos[2]=Mos[2]+Fval;
							
							Mos[3]=(float)myatoi(StrTokStr1[6])/10;
							Fval=myatoi(StrTokStr1[6])%10;
							Mos[3]=Mos[3]+Fval;
							
							
							SoilTemp=(float)myatoi(StrTokStr1[7])/10;
							Fval=myatoi(StrTokStr1[7])%10;
							SoilTemp=SoilTemp+Fval;
							
							Val=strcmp(SetSerial,RecSetSerial);
							if(Val!=0)
							{
								strcpy(RecSetSerial,SetSerial);
								livedataflag=1;
							}
							Val=0;
							Val=strcmp(Mos,RecMos);
							if(Val!=0)
							{
								strcpy(RecMos,Mos);
								livedataflag=1;
							}
							sprintf(buf,"\n\r serialNo:%d--%d\nMos:%f--%f--%f--%f\n\r",SetSerial[0],SetSerial[1],Mos[0],Mos[1],Mos[2],Mos[3]);
							sAPI_UartPrintf(buf);
						}
						
						if(uart_read_buf[0] == '$' && uart_read_buf[1] == 'A' && uart_read_buf[2] == 'I' )
						{
									int StrTokStrVer = 0,Val=0;
							
								Pch1 = strtok((char *)uart_read_buf, (char *)"," );
								StrTokStrVer =0 ;
								while( Pch1 != NULL )
								{
								strcpy(StrTokStr1[StrTokStrVer],Pch1);
								StrTokStrVer++;
								Pch1 = strtok( NULL, "," );
								}
								ValveStatus[0]=myatoi(StrTokStr1[1]);
								ValveStatus[1]=myatoi(StrTokStr1[2]);
								ValveStatus[2]=myatoi(StrTokStr1[3]);
								ValveStatus[3]=myatoi(StrTokStr1[4]);
								ValveStatus[4]=myatoi(StrTokStr1[5]);
								ValveStatus[5]=myatoi(StrTokStr1[6]);
								ValveStatus[6]=myatoi(StrTokStr1[7]);
								ValveStatus[7]=myatoi(StrTokStr1[8]);
								ValveStatus[8]=myatoi(StrTokStr1[9]);
								ValveStatus[9]=myatoi(StrTokStr1[10]);
								ReasonFlag=myatoi(StrTokStr1[11]);
								NoOfValveConfigRec=myatoi(StrTokStr1[12]);
								//act_POnMin[l_mtrno]=myatoi(StrTokStr1[8]);
								//act_POnSec[l_mtrno]=myatoi(StrTokStr1[9]);
								//act_rem_delmin[l_mtrno]=myatoi(StrTokStr1[10]);
								//act_rem_delsec[l_mtrno]=myatoi(StrTokStr1[11]);
								
								//(act_POnMin[l_mtrno]>60)?(act_POnMin[l_mtrno]=5):0;
								//(act_POnSec[l_mtrno]>60)?(act_POnSec[l_mtrno]=30):0;
								Val=strcmp(ValveStatus,Prvalve);
								sprintf(buf,"\n--Value %d\n\r",Val);
								sAPI_UartPrintf(buf);
								sprintf(buf,"\n\r valve Status:%d--%d--%d--%d--%d--%d--%d--%d--%d--%d\n\r",ValveStatus[0],ValveStatus[1],ValveStatus[2],ValveStatus[3],ValveStatus[4],ValveStatus[5],ValveStatus[6],ValveStatus[7],ValveStatus[8],ValveStatus[9]);
								sAPI_UartPrintf(buf);
								//strcmp(ValveStatus,Prvalve);
								if(Val!=0)
								{
									strcpy(Prvalve,ValveStatus);
									livedataflag=1;
									sprintf(buf,"\n\r Prv valve Status:%d--%d--%d--%d--%d--%d--%d--%d--%d--%d\n\r",Prvalve[0],Prvalve[1],Prvalve[2],Prvalve[3],Prvalve[4],Prvalve[5],Prvalve[6],Prvalve[7],Prvalve[8],Prvalve[9]);
									sAPI_UartPrintf(buf);
								}
								sprintf(buf,"\n\r valve Status:%d--%d--%d--%d--%d--%d--%d--%d--%d--%d\n\r--Value %d\n\r",ValveStatus[0],ValveStatus[1],ValveStatus[2],ValveStatus[3],ValveStatus[4],ValveStatus[5],ValveStatus[6],ValveStatus[7],ValveStatus[8],ValveStatus[9],Val);
								sAPI_UartPrintf(buf);
								//sprintf(buf,"PrvValveStatus[9] %d ValveStatus[9] %d ",PrvValveStatus[9],ValveStatus[9]);
								 //sAPI_UartPrintf(buf);								
								//if((PrvValveStatus[9]!=ValveStatus[9]))
								{
									//if(livedataflag==1)
									//{livedataflag1=1;livedataflagcount1=0;}	
									//else
									//
									//PrvValveStatus[9]=ValveStatus[9];
									
								}
								
						//sprintf(buf,"\n\r valve 10:%d--%d--%d\n\rRemaing Time 8:%d--%d--%d\n\rvalve 9:%d--%d--%d\n\r",LiveValveTimerHr[9],LiveValveTimerMin[9],ValveStatus[9],RemainingOnHr,RemainingOnMin,RemainingOnSec);
						//sAPI_UartPrintf(buf);
						
						} 
				 		if(uart_read_buf[0] == '$' && uart_read_buf[1] == 'B' && uart_read_buf[2] == 'I' )
						{
									int StrTokStrVer = 0;
							
								Pch1 = strtok((char *)uart_read_buf, (char *)"," );
								StrTokStrVer =0 ;
								while( Pch1 != NULL )
								{
								strcpy(StrTokStr1[StrTokStrVer],Pch1);
								StrTokStrVer++;
								Pch1 = strtok( NULL, "," );
								}
								RemainingOnHr=myatoi(StrTokStr1[1]);
								RemainingOnMin=myatoi(StrTokStr1[2]);
								RemainingOnSec=myatoi(StrTokStr1[3]);
								CycOnOf=myatoi(StrTokStr1[4]);
								CycleNo=myatoi(StrTokStr1[5]);
								CyclicLimitRec=myatoi(StrTokStr1[6]);
								CyclicComplete=myatoi(StrTokStr1[7]);
								LiveUpdate=myatoi(StrTokStr1[8]);
								//act_POnMin[l_mtrno]=myatoi(StrTokStr1[8])
								//act_POnSec[l_mtrno]=myatoi(StrTokStr1[9]);
								//act_rem_delmin[l_mtrno]=myatoi(StrTokStr1[10]);
								//act_rem_delsec[l_mtrno]=myatoi(StrTokStr1[11]);
								
								//(act_POnMin[l_mtrno]>60)?(act_POnMin[l_mtrno]=5):0;
								//(act_POnSec[l_mtrno]>60)?(act_POnSec[l_mtrno]=30):0;
								
								sprintf(buf,"\n\r valve Remaing:%d--%d--%d   :%d--%d--%d\n\r",RemainingOnHr,RemainingOnMin,RemainingOnSec,LiveUpdate,RecLiveUpdate,livedataflag);
								sAPI_UartPrintf(buf);
								if(LiveUpdate!=RecLiveUpdate)
								{
									RecLiveUpdate=LiveUpdate;
									if(livedataflag!=1)
									livedataflag=1;
									sprintf(buf,"\n\r In If valve Remaing:%d--%d--%d   :%d--%d--%d\n\r",RemainingOnHr,RemainingOnMin,RemainingOnSec,LiveUpdate,RecLiveUpdate,livedataflag);
									sAPI_UartPrintf(buf);
								}
								//sprintf(buf,"PrvValveStatus[9] %d ValveStatus[9] %d ",PrvValveStatus[9],ValveStatus[9]);
								 //sAPI_UartPrintf(buf);								
								//if((PrvValveStatus[9]!=ValveStatus[9]))
								{
									//if(livedataflag==1)
									//{livedataflag1=1;livedataflagcount1=0;}	
									//else
									//livedataflag=1;
									//PrvValveStatus[9]=ValveStatus[9];
									
								}
								
						//sprintf(buf,"\n\r valve 10:%d--%d--%d\n\rRemaing Time 8:%d--%d--%d\n\rvalve 9:%d--%d--%d\n\r",LiveValveTimerHr[9],LiveValveTimerMin[9],ValveStatus[9],RemainingOnHr,RemainingOnMin,RemainingOnSec);
						//sAPI_UartPrintf(buf);
						
						} 
				 		if(uart_read_buf[0] == '$' && uart_read_buf[1] == 'E' && uart_read_buf[2] == 'I' )
						{
								int StrTokStrVer = 0,RecYear=0,RecMonth=0,RecDate=0,RecHr=0,RecMin=0;
								int GsmSec=0,Recsec=0,posSec=0,NegSec=0; 
								   
								Pch1 = strtok((char *)uart_read_buf, (char *)"," );
								StrTokStrVer =0 ;
								while( Pch1 != NULL )
								{
								strcpy(StrTokStr1[StrTokStrVer],Pch1);
								StrTokStrVer++;
								Pch1 = strtok( NULL, "," );
								}
								
								RecYear=myatoi(StrTokStr1[1]);
								RecMonth=myatoi(StrTokStr1[2]);
								RecDate=myatoi(StrTokStr1[3]);
								RecHr=myatoi(StrTokStr1[4]);
								RecMin=myatoi(StrTokStr1[5]);
								RecYear+=2000;
								if(RecYear!=timeval.tm_year || RecMonth!=timeval.tm_mon || RecDate!=timeval.tm_mday || RecHr!=timeval.tm_hour || RecMin!=timeval.tm_min)
								{
									GsmSec=timeval.tm_min*60;
									Recsec=RecMin*60;
									if(GsmSec>Recsec)
									posSec=GsmSec-Recsec;
									if(Recsec>GsmSec)
									NegSec=Recsec-GsmSec;
									if(NegSec>120 || posSec>120)
									date_time_send_flag=1;
								}
							
								
								
								sprintf(buf,"\n\r Controller Date:%d--%d--%d   Time:%d--%d--%d\n\r",RecYear,RecMonth,RecDate,RecHr,RecMin);
								sAPI_UartPrintf(buf);
								sprintf(buf,"\n\r Module Date:%d--%d--%d   Time:%d--%d--%d\n\r",timeval.tm_year,timeval.tm_mon,timeval.tm_mday,timeval.tm_hour,timeval.tm_min);
								sAPI_UartPrintf(buf);
						
						} 
						if(uart_read_buf[0] == '$' && uart_read_buf[1] == 'D' && uart_read_buf[2] == 'I' )
						{
									int StrTokStrVer = 0;
							
								Pch1 = strtok((char *)uart_read_buf, (char *)"," );
								StrTokStrVer =0 ;
								while( Pch1 != NULL )
								{
								strcpy(StrTokStr1[StrTokStrVer],Pch1);
								StrTokStrVer++;
								Pch1 = strtok( NULL, "," );
								}
								Battery[0]=(float)myatoi(StrTokStr1[1])/10;
								Solar[0]=myatoi(StrTokStr1[2]);
								LightFlag=myatoi(StrTokStr1[3]);
								AutoRstFlag=myatoi(StrTokStr1[4]);
								LightStandaloneFlag=myatoi(StrTokStr1[5]);
								PressureSensor=myatof(StrTokStr1[6]);
								
								if(LightFlag!=RecLightFlag)
								{
									RecLightFlag=LightFlag;
									if(livedataflag!=1)
									livedataflag=1;
									
								}
								
								sprintf(buf,"\n\r valve Remaing:%d--%d--%d   :%d--%d--%d\n\r",RemainingOnHr,RemainingOnMin,RemainingOnSec,LiveUpdate,RecLiveUpdate,livedataflag);
								sAPI_UartPrintf(buf);
								
								//sprintf(buf,"PrvValveStatus[9] %d ValveStatus[9] %d ",PrvValveStatus[9],ValveStatus[9]);
								 //sAPI_UartPrintf(buf);								
								//if((PrvValveStatus[9]!=ValveStatus[9]))
								{
									//if(livedataflag==1)
									//{livedataflag1=1;livedataflagcount1=0;}	
									//else
									//livedataflag=1;
									//PrvValveStatus[9]=ValveStatus[9];
									
								}
								
						//sprintf(buf,"\n\r valve 10:%d--%d--%d\n\rRemaing Time 8:%d--%d--%d\n\rvalve 9:%d--%d--%d\n\r",LiveValveTimerHr[9],LiveValveTimerMin[9],ValveStatus[9],RemainingOnHr,RemainingOnMin,RemainingOnSec);
						//sAPI_UartPrintf(buf);
						
						} 
				 		
						
						
						if(uart_read_buf[0] == '$' && uart_read_buf[1] == 'L' && uart_read_buf[2] == 'I' )
						{
									int StrTokStrVer = 0;
							
								Pch1 = strtok((char *)uart_read_buf, (char *)"," );
								StrTokStrVer =0 ;
								while( Pch1 != NULL )
								{
								strcpy(StrTokStr1[StrTokStrVer],Pch1);
								StrTokStrVer++;
								Pch1 = strtok( NULL, "," );
								}
								l_mtrno=myatoi(StrTokStr1[1]);
								Motoronflag[l_mtrno]=myatoi(StrTokStr1[2]);
								Motorreasonflag[l_mtrno]=myatoi(StrTokStr1[3]);
							//	s_nTimerSettings.m_POnMin[l_mtrno]*60)
							//	s_nTimerSettings.m_POnSec[l_mtrno]
								actual_float_stat[l_mtrno][0]=myatoi(StrTokStr1[4]);
								actual_float_stat[l_mtrno][1]=myatoi(StrTokStr1[5]);
								actual_float_stat[l_mtrno][2]=myatoi(StrTokStr1[6]);
								actual_float_stat[l_mtrno][3]=myatoi(StrTokStr1[7]);
								act_POnMin[l_mtrno]=myatoi(StrTokStr1[8]);
								act_POnSec[l_mtrno]=myatoi(StrTokStr1[9]);
								act_rem_delmin[l_mtrno]=myatoi(StrTokStr1[10]);
								act_rem_delsec[l_mtrno]=myatoi(StrTokStr1[11]);
								
								(act_POnMin[l_mtrno]>60)?(act_POnMin[l_mtrno]=5):0;
								(act_POnSec[l_mtrno]>60)?(act_POnSec[l_mtrno]=30):0;
								
								sprintf(buf,"\n\r l_mtrno = %d  Motoronflag[%d] = %d act_POnMin[l_mtrno]%d act_POnSec[l_mtrno] %d act_rem_delmin[l_mtrno] %d act_rem_delsec[l_mtrno] %d actual_float_stat[l_mtrno][3] %d prev_Motoronflag[%d]:%d,Motorreasonflag[%d]:%d,prev_Motorreasonflag[%d]:%d\n\r",l_mtrno,l_mtrno,Motoronflag[l_mtrno],act_POnMin[l_mtrno],act_POnSec[l_mtrno],act_rem_delmin[l_mtrno],act_rem_delsec[l_mtrno],actual_float_stat[l_mtrno][3],l_mtrno,prev_Motoronflag[l_mtrno],l_mtrno,Motorreasonflag[l_mtrno],l_mtrno,prev_Motorreasonflag[l_mtrno]);
								sAPI_UartPrintf(buf);
																
								if((prev_Motoronflag[l_mtrno]!=Motoronflag[l_mtrno])|| (prev_Motorreasonflag[l_mtrno]!=Motorreasonflag[l_mtrno]))
								{
									if(livedataflag==1)
									{livedataflag1=1;livedataflagcount1=0;}	
									else
									livedataflag=1;
									prev_Motoronflag[l_mtrno]=Motoronflag[l_mtrno];
									prev_Motorreasonflag[l_mtrno]=Motorreasonflag[l_mtrno];
								}
								
						sprintf(buf,"\n\r l_mtrno = %d  Motoronflag[%d] = %d act_POnMin[l_mtrno]%d act_POnSec[l_mtrno] %d act_rem_delmin[l_mtrno] %d act_rem_delsec[l_mtrno] %d actual_float_stat[l_mtrno][3] %d\n\r",l_mtrno,l_mtrno,Motoronflag[l_mtrno],act_POnMin[l_mtrno],act_POnSec[l_mtrno],act_rem_delmin[l_mtrno],act_rem_delsec[l_mtrno],actual_float_stat[l_mtrno][3]);
						sAPI_UartPrintf(buf);
						
						} 
						
						if(uart_read_buf[0] == '$' && uart_read_buf[1] == 'N' && uart_read_buf[2] == 'I' )
						{
									int StrTokStrVer = 0;
							
								Pch1 = strtok((char *)uart_read_buf, (char *)"," );
								StrTokStrVer =0 ;
								while( Pch1 != NULL )
								{
								strcpy(StrTokStr1[StrTokStrVer],Pch1);
								StrTokStrVer++;
								Pch1 = strtok( NULL, "," );
								}
								l_mtrno=myatoi(StrTokStr1[1]);
								act_rem_cyc_ofHr[l_mtrno]=myatoi(StrTokStr1[2]);
								act_rem_cyc_ofMin[l_mtrno]=myatoi(StrTokStr1[3]);
								act_rem_cyc_ofSec[l_mtrno]=myatoi(StrTokStr1[4]);
								act_rem_cyc_onHr[l_mtrno]=myatoi(StrTokStr1[5]);
								act_rem_cyc_onMin[l_mtrno]=myatoi(StrTokStr1[6]);
								act_rem_cyc_onSec[l_mtrno]=myatoi(StrTokStr1[7]);
								act_rem_maxHr[l_mtrno]=myatoi(StrTokStr1[8]);
								act_rem_maxMin[l_mtrno]=myatoi(StrTokStr1[9]);
								act_rem_maxSec[l_mtrno]=myatoi(StrTokStr1[10]);
								if(Motoronflag[l_mtrno]==0)
								{
									
									act_rem_cyc_onHr[l_mtrno]=0;
									act_rem_cyc_onMin[l_mtrno]=0;
									act_rem_cyc_onSec[l_mtrno]=0;
									//act_rem_maxHr[l_mtrno]=0;
									//act_rem_maxMin[l_mtrno]=0;
									//act_rem_maxSec[l_mtrno]=0;
								}
								if(s_nMSettings.m_CycLicOnOf[l_mtrno]==0)
								{
									act_rem_cyc_ofHr[l_mtrno]=0;
									act_rem_cyc_ofMin[l_mtrno]=0;
									act_rem_cyc_ofSec[l_mtrno]=0;
									act_rem_cyc_onHr[l_mtrno]=0;
									act_rem_cyc_onMin[l_mtrno]=0;
									act_rem_cyc_onSec[l_mtrno]=0;
								}
								if((s_nMSettings.m_MaxRnOnOf[l_mtrno] == 0 || Motoronflag[l_mtrno]==0) &&Motorreasonflag[l_mtrno] != 8)
								{
									act_rem_maxHr[l_mtrno]=0;
									act_rem_maxMin[l_mtrno]=0;
									act_rem_maxSec[l_mtrno]=0;
								}									
								
														
						} 
						
						 if(uart_read_buf[0] == '$' && uart_read_buf[1] == 'F' && uart_read_buf[2] == 'I' )
						{
									int StrTokStrVer = 0;
							
								Pch1 = strtok((char *)uart_read_buf, (char *)"," );
								StrTokStrVer =0 ;
								while( Pch1 != NULL )
								{
								strcpy(StrTokStr1[StrTokStrVer],Pch1);
								StrTokStrVer++;
								Pch1 = strtok( NULL, "," );
								}
								Act_pressure1=myatof(StrTokStr1[1]);
								Act_level1=myatof(StrTokStr1[2]);
								Act_lps=myatof(StrTokStr1[3]);
								cumulative_flow=myatoi(StrTokStr1[4]);
								//flowisthere[2]=myatoi(StrTokStr1[5]);
								WorkingOn3Phas=myatoi(StrTokStr1[5]);
								prevlevel_percent=myatoi(StrTokStr1[6]);
								if(level_percent!=prevlevel_percent)
								{
									if(prevlevel_percent<=100)
									level_percent=prevlevel_percent;
								}
										
									
								sprintf(buf,"\n level_percent : %03d prevlevel_percent %03d\n",level_percent,prevlevel_percent);
								sAPI_UartPrintf(buf);
								if(s_npump[0].m_Level_on_off==1)
								{
								Act_level[0]=Act_level1;
								Act_level[1]=-1.0;
								Act_level[2]=-1.0;
								}
								else if(s_npump[1].m_Level_on_off==1)
								{
								Act_level[0]=-1.0;
								Act_level[1]=Act_level1;
								Act_level[2]=-1.0;
								}
								else if(s_npump[2].m_Level_on_off==1)
								{
								Act_level[0]=-1.0;
								Act_level[1]=-1.0;
								Act_level[2]=Act_level1;
								}
								/* else
								{
								Act_level[0]=0;Act_level[1]=0;Act_level[2]=	0;
								} */
								
								if(s_npump[0].m_pressureonof==1)
								{
								Act_pressure[0]=Act_pressure1;
								Act_pressure[1]=-1.0;
								Act_pressure[2]=-1.0;
								}
								else if(s_npump[1].m_pressureonof==1)
								{
								Act_pressure[0]=-1.0;
								Act_pressure[1]=Act_pressure1;
								Act_pressure[2]=-1.0;
								}
								else if(s_npump[2].m_pressureonof==1)
								{
								Act_pressure[0]=-1.0;
								Act_pressure[1]=-1.0;
								Act_pressure[2]=Act_pressure1;
								}
								
								if(s_npump[0].m_flowonof==1)
								{
								Act_lps1[0]=Act_lps;
								Act_lps1[1]=-1.0;
								Act_lps1[2]=-1.0;
								cumulative_flow1[0]=cumulative_flow;
								cumulative_flow1[1]=0;
								cumulative_flow1[2]=0;
								}
								else if(s_npump[1].m_flowonof==1)
								{
								Act_lps1[0]=-1.0;
								Act_lps1[1]=Act_lps;
								Act_lps1[2]=-1.0;
								cumulative_flow1[0]=0;
								cumulative_flow1[1]=cumulative_flow;
								cumulative_flow1[2]=0;
								}
								else if(s_npump[2].m_flowonof==1)
								{
								Act_lps1[0]=-1.0;
								Act_lps1[1]=-1.0;
								Act_lps1[2]=Act_lps;
								cumulative_flow1[0]=0;
								cumulative_flow1[1]=0;
								cumulative_flow1[2]=cumulative_flow;
								}
								/* else
								{
								Act_pressure[0]=0;Act_pressure[1]=0;Act_pressure[2]=0;
								} */
							//	s_npump[0].m_flowonof=myatoi(StrTokStr1[9]);
							//	s_npump[0].m_pressureonof=myatoi(StrTokStr1[10]);
							/* 	Act_level[0]=10;Act_level[1]=20;Act_level[2]=30;
								Act_pressure[0]=12.3;Act_pressure[1]=23.4;Act_pressure[2]=34.5; */
						}
						
						 if(uart_read_buf[0] == '$' && uart_read_buf[1] == 'W' )
						{
									int StrTokStrVer = 0;
							
								Pch1 = strtok((char *)uart_read_buf, (char *)"," );
								StrTokStrVer =0 ;
								while( Pch1 != NULL )
								{
								strcpy(StrTokStr1[StrTokStrVer],Pch1);
								StrTokStrVer++;
								Pch1 = strtok( NULL, "," );
								}
								s_nMSettings.m_CalRVoltage_Rx=myatof(StrTokStr1[1]);
								s_nMSettings.m_CalYVoltage_Rx=myatof(StrTokStr1[2]);
								s_nMSettings.m_CalBVoltage_Rx=myatof(StrTokStr1[3]);
								s_nMSettings.m_CalRCurrent_Rx=myatof(StrTokStr1[4]);
								s_nMSettings.m_CalYCurrent_Rx=myatof(StrTokStr1[5]);
								s_nMSettings.m_CalBCurrent_Rx=myatof(StrTokStr1[6]);
								WritecalsetFile();
								ReadcalsetFile();
						}

						if(uart_read_buf[0] == '$' && uart_read_buf[1] == 'O' && uart_read_buf[2] == '1' )
						{
							int StrTokStrVer = 0;
							
								Pch1 = strtok((char *)uart_read_buf, (char *)"," );
								StrTokStrVer =0 ;
								while( Pch1 != NULL )
								{
								strcpy(StrTokStr1[StrTokStrVer],Pch1);
								StrTokStrVer++;
								Pch1 = strtok( NULL, "," );
								}
								ttday[0].m_M1_Runtime=myatoi(StrTokStr1[1]);
								ttday[0].m_M1_Idletime=myatoi(StrTokStr1[2]);
								ttday[0].m_M1Dry_run_trip_time=myatoi(StrTokStr1[3]);
								ttday[0].m_M1cyclic_trip_time=myatoi(StrTokStr1[4]);
								ttday[0].m_M1other_trip_time=myatoi(StrTokStr1[5]);
	
						}
						if(uart_read_buf[0] == '$' && uart_read_buf[1] == 'O' && uart_read_buf[2] == '2' )
						{
							int StrTokStrVer = 0;
							
								Pch1 = strtok((char *)uart_read_buf, (char *)"," );
								StrTokStrVer =0 ;
								while( Pch1 != NULL )
								{
								strcpy(StrTokStr1[StrTokStrVer],Pch1);
								StrTokStrVer++;
								Pch1 = strtok( NULL, "," );
								}
								ttday[0].m_M2_Runtime=myatoi(StrTokStr1[1]);
								ttday[0].m_M2_Idletime=myatoi(StrTokStr1[2]);
								ttday[0].m_M2Dry_run_trip_time=myatoi(StrTokStr1[3]);
								ttday[0].m_M2cyclic_trip_time=myatoi(StrTokStr1[4]);
								ttday[0].m_M2other_trip_time=myatoi(StrTokStr1[5]);
	
						}
						if(uart_read_buf[0] == '$' && uart_read_buf[1] == 'O' && uart_read_buf[2] == '3' )
						{
							int StrTokStrVer = 0;
							
								Pch1 = strtok((char *)uart_read_buf, (char *)"," );
								StrTokStrVer =0 ;
								while( Pch1 != NULL )
								{
								strcpy(StrTokStr1[StrTokStrVer],Pch1);
								StrTokStrVer++;
								Pch1 = strtok( NULL, "," );
								}
								ttday[0].m_M3_Runtime=myatoi(StrTokStr1[1]);
								ttday[0].m_M3_Idletime=myatoi(StrTokStr1[2]);
								ttday[0].m_M3Dry_run_trip_time=myatoi(StrTokStr1[3]);
								ttday[0].m_M3cyclic_trip_time=myatoi(StrTokStr1[4]);
								ttday[0].m_M3other_trip_time=myatoi(StrTokStr1[5]);
	
						}
						if(uart_read_buf[0] == '$' && uart_read_buf[1] == 'O' && uart_read_buf[2] == '4' )
						{
							int StrTokStrVer = 0;
							
								Pch1 = strtok((char *)uart_read_buf, (char *)"," );
								StrTokStrVer =0 ;
								while( Pch1 != NULL )
								{
								strcpy(StrTokStr1[StrTokStrVer],Pch1);
								StrTokStrVer++;
								Pch1 = strtok( NULL, "," );
								}
								nMoTr.Act2powerRunTimer=myatoi(StrTokStr1[1]);
								nMoTr.Act3powerRunTimer=myatoi(StrTokStr1[2]);
								nMoTr.ActnopowerRunTimer=myatoi(StrTokStr1[3]);
								onehour_send_flag=1;
	
						}
						
						
				}
				
		#if 0
		if(uart_read_buf[0] == '#')
		{ 
				char* Pch1=NULL;
				char Tparr[2];
				char TempBuf[50]={0};
				sAPI_UartPrintf("\nEntry#$#");
				uint32_t i=0,j=0;
				HowManySoundToPlay=0;
				// Pch1 = strtok((char *)uart_read_buf, (char *)"\r" );
				// strcpy(TempBuf,Pch1);
				
				for ( j = 0; j < strlen(uart_read_buf); j++)
				  {
					  if(uart_read_buf[j] != ';' || j==1)
						TempBuf[j]=uart_read_buf[j]; 
					  else
						 break;
				  }
				
				sAPI_UartPrintf("\n[%s]",TempBuf);
				
				for ( j = 0; j < sizeof(HowManySound)/sizeof(HowManySound[0]); j++)
				{
					HowManySound[j] =0;
				}				
				for ( i = 0; i < strlen(TempBuf)-1; i++) {
					sAPI_UartPrintf("\n%d--",TempBuf[i+1]);				 
					sprintf(Tparr,"%x",TempBuf[i+1]);
					 HowManySound[i] =TempBuf[i+1];				 
					 //HowManySound[i] =atoi(Tparr);				 
					sAPI_UartPrintf("\n%d",HowManySound[i]);
					HowManySoundToPlay++;
				}
				
				PlaySound =1;            		
		}
		#endif
 
 
		   
	}   
   
 }
 
 void sAPP_UartTask(void)
{

	SC_STATUS status;
	SCuartConfiguration uartConfig, uart2Config, uart3Config;

	/*************************Configure UART again*********************************/
	/*******The user can modify the initialization configuratin of UART in here.***/
	/******************************************************************************/
	
//	sAPI_UartModeControl(0);
	uartConfig.BaudRate = SC_UART_BAUD_9600;//SC_UART_BAUD_115200;
	uartConfig.DataBits = SC_UART_WORD_LEN_8;
	uartConfig.ParityBit = SC_UART_NO_PARITY_BITS;
	uartConfig.StopBits = SC_UART_ONE_STOP_BIT;
	if (sAPI_UartSetConfig(SC_UART, &uartConfig) == SC_UART_RETURN_CODE_ERROR)
	{
		sAPI_UartPrintf("Configure UART failure!!");
	}

	/*************************Configure UART2 again*********************************/
	/*******The user can modify the initialization configuratin of UART2 in here.***/
	/*******************************************************************************/
/*	uart2Config.BaudRate = SC_UART_BAUD_115200;
	uart2Config.DataBits = SC_UART_WORD_LEN_8;
	uart2Config.ParityBit = SC_UART_NO_PARITY_BITS;
	uart2Config.StopBits = SC_UART_ONE_STOP_BIT;
	if (sAPI_UartSetConfig(SC_UART2, &uart2Config) == SC_UART_RETURN_CODE_ERROR)
	{
		sAPI_UartPrintf("Configure UART2 failure!!");
	}
*/
	/*************************Configure UART3 again*********************************/
	/*******The user can modify the initialization configuratin of UART3 in here.***/
	/*******************************************************************************/
/*	uart3Config.BaudRate = SC_UART_BAUD_115200;
	uart3Config.DataBits = SC_UART_WORD_LEN_8;
	uart3Config.ParityBit = SC_UART_NO_PARITY_BITS;
	uart3Config.StopBits = SC_UART_ONE_STOP_BIT;
	if (sAPI_UartSetConfig(SC_UART3, &uart3Config) == SC_UART_RETURN_CODE_ERROR)
	{
		sAPI_UartPrintf("Configure UART3 failure!!");
	}
*/
	sAPI_UartPrintf("\n\rUART Configuration is completed!!\n");

	// status = sAPI_MsgQCreate(&simcomUI_msgq, "simcomUI_msgq", sizeof(SIM_MSG_T), 10, SC_FIFO);
	// if (SC_SUCCESS != status)
	// {
	// 	sAPI_UartPrintf("msgQ create fail");
	// }

status = sAPI_MsgQCreate(&gUart_msgq, "gUart_msgq", (sizeof(SIM_MSG_T)), 100, SC_FIFO);
	if (status != SC_SUCCESS)
	{
		sAPI_UartPrintf("message queue creat err!\n");
	}

	/******************************************************************************/
	/**************************   UART task   *************************************/
	/******************************************************************************/
	// if (NULL != gUartRxProcessTask1)
	// {
	// 	return;
	// }
	sAPI_UartPrintf("sAPI_UartTaskCreate start!\n");
	
	status = sAPI_TaskCreate(&gUartRxProcessTask1,gUartRxProcessTask1Stack, 5* 1024,100,"uart_rx_processer",sTask_UartRxProcesser,(void *)0);
if (SC_SUCCESS != status)
	{
		sAPI_UartPrintf("Task create fail");
	}

	/* if (sAPI_TaskCreate(&gUartRxProcessTask1,
						gUartRxProcessTask1Stack,
						7 * 1024,
						100,
						(char *)"uart rx processer",
						sTask_UartRxProcesser,
						(void *)0) != SC_SUCCESS)
	{
		gUartRxProcessTask1 = NULL;
		sAPI_UartPrintf("sAPI_UartTaskCreate error!\n");
	} */
	/******************************************************************************/
	/**************************   UART2 task   *************************************/
	/******************************************************************************/

/*	if (NULL != gUart2RxProcessTask)
	{
		return;
	}
	// sAPI_Debug("sAPI_Uart3TaskCreate start!\n");

	if (sAPI_TaskCreate(&gUart2RxProcessTask,
						gUart2RxProcessTaskStack,
						3 * 1024,
						130,
						(char *)"uart2 rx processer",
						sTask_Uart2RxProcesser,
						(void *)0) != SC_SUCCESS)
	{
		gUart2RxProcessTask = NULL;
		sAPI_UartPrintf("sAPI_Uart2TaskCreate error!\n");
	}
*/
	sAPI_UartPrintf("%s Task creation completed!!\n", __func__);

	sAPI_UartRegisterCallback(SC_UART, UartCBFunc1);
//	sAPI_UartRegisterCallback(SC_UART2, Uart2CBFunc);
}
 /* void Uart2CBFunc(SC_Uart_Port_Number portNumber, void *para)
{
    int readLen = 0;
    char *uart2Data = sAPI_Malloc(UART_RX_BUFFER_SIZE);
    SIM_MSG_T uart2Msg = {0,0,0,NULL};

    readLen = sAPI_UartRead(portNumber, (UINT8 *)uart2Data, UART_RX_BUFFER_SIZE);
    sAPI_Debug("%s, portNumber is %d, readLen[%d].",__func__, portNumber, readLen);

 
    // uart2Msg.msg_id = SRV_UART;
    uart2Msg.arg2 = readLen;
    uart2Msg.arg3 = uart2Data;
    send_uart2_data(uart2Msg);
	
	if(nMSettings.ndebugonof==1)
	{
	sprintf(buf,"uart2Data:%s",uart2Data);
	sAPI_UartPrintf(buf); 
	}

    sAPI_Free(uart2Data);
    return;
}

void send_uart2_data(SIM_MSG_T Uart2Msg)
{
	SC_STATUS status = SC_SUCCESS;
    SIM_MSG_T optionMsg = {0,0,0,NULL};
    optionMsg.msg_id = Uart2Msg.msg_id;
    optionMsg.arg2 = Uart2Msg.arg2;

    optionMsg.arg3 = sAPI_Malloc(optionMsg.arg2+1);
    memset(optionMsg.arg3, 0, optionMsg.arg2+1);
    memcpy(optionMsg.arg3, Uart2Msg.arg3, Uart2Msg.arg2);

    status = sAPI_MsgQSend(uart2_msgq,&optionMsg);
    if(status != SC_SUCCESS)
    {
        sAPI_Free(optionMsg.arg3);
        sAPI_UartPrintf("send msg error,status = [%d]" );
    }
} */ 

void app_hw_gpio_init(void)
{
	sAPI_UartPrintf("\n\rGPIO_Init START\n\r");
	sAPI_GpioSetDirection(SPP_PIN, SC_GPIO_IN_PIN);
	/* sAPI_GpioSetDirection(UPTANK_PIN1,SC_GPIO_IN_PIN);
	sAPI_GpioSetDirection(UPTANK_PIN2,SC_GPIO_IN_PIN);
	sAPI_GpioSetDirection(LOTANK_PIN1,SC_GPIO_IN_PIN);
	sAPI_GpioSetDirection(LOTANK_PIN2,SC_GPIO_IN_PIN);
	sAPI_GpioSetDirection(PH3OR2SELECTION_KEY,SC_GPIO_IN_PIN);
	sAPI_GpioSetDirection(AUTOMOBILE_KEY,SC_GPIO_IN_PIN); */
	sAPI_GpioSetDirection(NETLIGHT, SC_GPIO_OUT_PIN);
	/* sAPI_GpioSetDirection(STATUS_PIN,SC_GPIO_OUT_PIN);
	sAPI_GpioSetValue(STATUS_PIN,1); */
	sAPI_UartPrintf("\n\rGPIO_Init Done\n\r");
}