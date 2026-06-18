#if !defined APP_TCPIP_H
#define APP_TCPIP_H

#include "platform.h"



typedef void (*ResultNotifyCb)(eat_bool result);
#define SOCKET_TIMER EAT_TIMER_4
#define SOCKET_TIMER_IA_PERIOD 300000

#define RESPONSE_TIMER EAT_TIMER_9
#define RESPONSE_TIMER_PERIOD 5000

#define WIFI_RESPONSE_TIMER EAT_TIMER_7
#define WIFI_RESPONSE__PERIOD 1000

#define SOCKET_TIC EAT_TIMER_5
#define SOCKET_TIC_PERIOD 800

#define REC_TIMER EAT_TIMER_10
#define REC_TIMER_PERIOD 15000

#define BLOCKOUT_Response_TIMER EAT_TIMER_11
#define BLOCKOUT_Response_TIMER_PERIOD 10000

#define OK_TIMER EAT_TIMER_14
#define OK_TIMER_PERIOD 10000
//unsigned char ftpgettofs_cb_flag=0;

typedef struct WfCmdTTag
{
    char* p_WtCmdStr;
    char* p_WtResp;
}WfCmdT;

#if 0
typedef enum{CMD=0,SETIPDHCP,SETWLANSSID,SETWLANPHARSE,SETWLANAUTH,
SETWLANJOINONE,SETIPPROTOCOLEIGHT,SETIPHOST,SETIPREMOTEPORT,SETBAUD,SETUARTMODE,IDLETIME,
SETAPMODESSID,SETAPMODEPASSPHRASE,SETWPSIPADDRESS,SETWPSLOCALPORT,SETCOMMREMOTE,SAVE,REBOOT,SETAPMODECHANNELSIX,
SETAPWLANJOINSEVEN,SETAPIPPROTOCOLTWO,SETAPSAVE,SETAPREBOOT,
SETWPSMODECHANNELONE,SETWPSWLANJOINONE,
SETWPSIPPROTOCOLTWO,SETWPSSAVE,SETWPSREBOOT,
SETCOMMOPEN,SETCOMCLOSE,EXIT,CLIENTCMD,CLIENTOPEN,CLIENTCLOSE}WifiCmd; //14,19
#endif
#if 0
typedef enum{INIT,INITTCP,INITSAP,INITWPS,CONFIG,CLIENTONOPEN,CLIENTONREOPEN,CLIENTOPENED,CLIENTONCLOSE,CLIENTCLOSED,SERVEROPENED,SERVERCLOSED,DISCONNECTED,SETAPMODE}WifiState;
#endif
typedef enum{Server_ERROR,Server_OK,Server_CONFIG,Server_CONFIGR}temp_niagara1;
extern void wifi_rx_proc(const EatEvent_st*);
extern void Set_AP_Mode(void);
extern unsigned char wifiopenflag,tcpopenflag,tcpcopyflag;
void gprs_start(u8* apn,u8* userName,u8* password);
s8 simcom_create_server(u16 port);
s8 simcom_connect_server(sockaddr_struct *addr);
s32 simcom_send_to_server(s8 sid, const void *buf, s32 len);
s32 simcom_recv_from_server(s8 sid, void *buf, s32 len);
void simcom_tcpip_test(void);
void ServerTask(void *data);
#endif
