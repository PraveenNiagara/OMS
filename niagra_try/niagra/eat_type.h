#if !defined (__EAT_TYPE_H__)
#define __EAT_TYPE_H__

#define EAT_NULL 0

//#define NULL 0

#define FALSE 0
#define TRUE 1

#define EAT_USER_MSG_MAX_SIZE 64 
#define MAX_SOCK_ADDR_LEN (28)

#define FS_READ_WRITE           0x00000000L 
#define FS_READ_ONLY            0x00000100L 
#define FS_CREATE               0x00010000L 

//typedef unsigned char bool;

typedef struct 
{
    SC_Uart_Port_Number uart;
} EatUart_st;

typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short int u16;
typedef signed short int s16;

typedef unsigned int u32;
typedef signed int s32;

typedef unsigned long long u64;
typedef signed long long s64;

typedef enum
{
    SOC_SUCCESS           = 0,     /* success */
    SOC_ERROR             = -1,    /* error */
    SOC_WOULDBLOCK        = -2,    /* not done yet */
    SOC_LIMIT_RESOURCE    = -3,    /* limited resource */
    SOC_INVALID_SOCKET    = -4,    /* invalid socket */
    SOC_INVALID_ACCOUNT   = -5,    /* invalid account id */
    SOC_NAMETOOLONG       = -6,    /* address too long */
    SOC_ALREADY           = -7,    /* operation already in progress */
    SOC_OPNOTSUPP         = -8,    /* operation not support */
    SOC_CONNABORTED       = -9,    /* Software caused connection abort */
    SOC_INVAL             = -10,   /* invalid argument */
    SOC_PIPE              = -11,   /* broken pipe */
    SOC_NOTCONN           = -12,   /* socket is not connected */
    SOC_MSGSIZE           = -13,   /* msg is too long */
    SOC_BEARER_FAIL       = -14,   /* bearer is broken */
    SOC_CONNRESET         = -15,   /* TCP half-write close, i.e., FINED */
    SOC_DHCP_ERROR        = -16,   /* DHCP error */
    SOC_IP_CHANGED        = -17,   /* IP has changed */
    SOC_ADDRINUSE         = -18,   /* address already in use */
    SOC_CANCEL_ACT_BEARER = -19    /* cancel the activation of bearer */
} soc_error_enum;


typedef enum
{
    SOC_SOCK_STREAM = 0,  /* stream socket, TCP */
    SOC_SOCK_DGRAM,       /* datagram socket, UDP */
    SOC_SOCK_SMS,         /* SMS bearer */
    SOC_SOCK_RAW          /* raw socket */
} socket_type_enum;


typedef struct 
{
    socket_type_enum	sock_type; /* socket type */
    s16 addr_len; /* address length */
    u16 port; /* port number */
    u8	addr[MAX_SOCK_ADDR_LEN];
    /* IP address. For keep the 4-type boundary, 
     * please do not declare other variables above "addr"
    */
} sockaddr_struct;





typedef enum  
{
    EAT_FALSE,
    EAT_TRUE
} eat_bool;



typedef struct CFG_Header {
    u32        magic;
    u16        size;
    u16        type;
} CFG_Header_st;

typedef struct APP_FILE_INFO_v1_
{
    CFG_Header_st   hdr;
    char            identifier[12];       // including '\0'
    u32             file_ver;

    u16             file_type;
    u8              flash_dev;
    u8              sig_type;

    u32             load_addr;
    u32             file_len;
    u32             max_size;
    u32             offset;
    u32             sig_len;
    u32             jump_offset;
    u32             attr;
} APP_FILE_INFO_v1;

typedef struct APP_BIN_INFO_v2_ {
    CFG_Header_st   hdr;
    u8              platform_id[128];
    u8              project_id[64];

    u32             ver;
    u32             combination;
    u32             combination_ex;
    u32             extsram_size;

    u32             minor_ver;
    u32             base_addr; /* For flash tool to check overlap region */
    u32             sds_len;       /* For flash tool to check overlap region */
    u32             ver_req;
    u32             addr;
    u32             len;
    u8              check[16];
    u32             reserved[12];
} APP_BIN_INFO_v2;

typedef enum {
    EAT_TIMER_1,
    EAT_TIMER_2,
    EAT_TIMER_3,
    EAT_TIMER_4,
    EAT_TIMER_5,
    EAT_TIMER_6,
    EAT_TIMER_7,
    EAT_TIMER_8,
    EAT_TIMER_9,
    EAT_TIMER_10,
    EAT_TIMER_11,
    EAT_TIMER_12,
    EAT_TIMER_13,
    EAT_TIMER_14,
    EAT_TIMER_15,
    EAT_TIMER_16,
    EAT_TIMER_NUM
} EatTimer_enum;


typedef enum {
    EAT_USER_0,
    EAT_USER_1,
    EAT_USER_2,
    EAT_USER_3,
    EAT_USER_4,
    EAT_USER_5,
    EAT_USER_6,
    EAT_USER_7,
    EAT_USER_8,
    EAT_USER_NUM
} EatUser_enum;




typedef struct
{
    APP_FILE_INFO_v1 file;
#if 0
    APP_BIN_INFO_v2  bin;
#endif
}APP_CFG_st;

#define CFG_HEADER(type, ver) { (APP_MAGIC|((ver)<<24)) , sizeof(type##_v##ver), type}

extern u32 Load$$APPCFG$$Base;
extern u32 Image$$APPCFG$$Length;

#define APP_ENTRY_FLAG  \
const APP_CFG_st app_cfg = \
{ \
    { \
        {0x014D4D4D,sizeof(APP_FILE_INFO_v1),0}, \
        "FILE_INFO", \
        1, \
        0x7001, \
        7, \
        0, \
        (u32)(&Load$$APPCFG$$Base), \
        0xffffffff, \
        0xffffffff, \
        (u32)(&Image$$APPCFG$$Length), \
        0, \
        0, \
        1 \
    } \
}; 

/*app bin config MACRO end*/

/*sem ID structure*/

typedef struct
{
    u8 unused;
} *EatSemId_st;


typedef enum {
    EAT_NO_WAIT, /*return immediately*/
    EAT_INFINITE_WAIT /*return only when get a semaphore value*/
} EatWaitMode_enum;



typedef enum   {
    EAT_EVENT_NULL = 0,
    EAT_EVENT_TIMER, /* timer time out*/
    EAT_EVENT_KEY, /* KEY*/
    EAT_EVENT_INT, /* GPIO interrupt*/
    EAT_EVENT_MDM_READY_RD, /* Read data from MODEM*/
    EAT_EVENT_MDM_READY_WR, /* Modem can receive the data*/
    EAT_EVENT_MDM_RI, /* Modem RI*/
    EAT_EVENT_UART_READY_RD, /* Received data from UART*/
    EAT_EVENT_UART_READY_WR, /* UART can receive the data*/
    EAT_EVENT_ADC, 
    EAT_EVENT_UART_SEND_COMPLETE, /* UART data send complete*/
    EAT_EVENT_USER_MSG, 
    EAT_EVENT_IME_KEY,/*The message of input method*/
#ifdef __SIMCOM_EAT_WMMP__ /*add wmmp user msg*/
    EAT_EVENT_TCPIP_STARTUP_CNF,
    EAT_EVENT_TCPIP_SHUTDOWN_CNF,
    EAT_EVENT_TCPIP_SHUTDOWN_IND,
    EAT_EVENT_TCPIP_CLOSE_CNF,
    EAT_EVENT_TCPIP_ACTIVE_OPEN_CNF,
    EAT_EVENT_TCPIP_TX_DATA_CNF,
    EAT_EVENT_TCPIP_TX_DATA_ENABLE_IND,
    EAT_EVENT_TCPIP_RX_DATA_REQ,
    EAT_EVENT_IMSI_READY,
    EAT_EVENT_NW_ATTACH_IND,
    EAT_EVENT_NEW_SMS_IND, /*new sms*/
#endif
#ifdef __SIMCOM_EAT_SOFT_SIM__
    EAT_EVENT_SIM_APDU_DATA_IND,
    EAT_EVENT_SIM_RESET_REQ,
#endif
    EAT_EVENT_SMS_SEND_CNF,   
    EAT_EVENT_AUD_PLAY_FINISH_IND, /*Play finish not in call*/
    EAT_EVENT_SND_PLAY_FINISH_IND, /*Play finish  in call*/
    EAT_EVENT_NUM
} EatEvent_enum; /* EVENT type */

typedef struct {
    EatTimer_enum timer_id; /* timer id */
} EatTimer_st; /* EAT_EVENT_TIMER data*/


/*enum value for user task*/

typedef struct {
    EatUser_enum src;
    eat_bool use_point;
    unsigned char len;
    unsigned char data[EAT_USER_MSG_MAX_SIZE];
    const void * data_p;
} EatUserMsg_st;

typedef enum {
    EAT_GPIO_LEVEL_LOW, /* low level */
    EAT_GPIO_LEVEL_HIGH /* high levle */
} EatGpioLevel_enum; 

typedef struct {
//    SC_Module_GPIONumbers pin; /* the pin */
    EatGpioLevel_enum level; /* 1-high level; 0-low level */
} EatInt_st; 

typedef union {
    EatTimer_st timer;
 //   EatKey_st key;
    EatInt_st interrupt;
 //   EatMdmRi_st mdm_ri;
    EatUart_st uart;
  //  EatAdc_st adc;
    EatUserMsg_st user_msg;
  //  WD_imeResult *ime_result;
#ifdef __SIMCOM_EAT_WMMP__ /*add wmmp user msg*/
    EatTcpipStartCnf_st tcpip_start_Cnf;
    EatTcpipShoutdownCnf_st tcpip_shutdown_cnf;
    EatTcpipShutdownInd_st   tcpip_shutdown_ind;
    EatTcpipCloseCnf_st    tcpip_close_cnf;
    EatTcpipActiveopenCnf_st  tcpip_activeopen_cnf;
    EatTcpipTxDataCnf_st  tcpip_tx_data_cnf;
    EatTcpipTxDataEnabledInd_st  tcpip_tx_data_enabled_ind;
    EatTcpipRxDataReq_st  tcpip_rx_data_req;
    EatNwAttachInd_st  nwAttachInd;
    EatSms_st sms;
#endif
#ifdef __SIMCOM_EAT_SOFT_SIM__
    EatAPDU_Tx_Data_st tx_apdu_data;
    SIMCARDRESET_REQ  sim_rst_req;
#endif
} EatEventData_union;

/* EVENT data that modem send to APP*/
typedef struct
{
    EatEvent_enum event;
    EatEventData_union data;
} EatEvent_st; 

typedef struct {
    eat_bool is_update_app; 
    eat_bool update_app_result; /* "update_app_result" is APP update result,only when "is_update_app" is EAT_TRUE*/
} EatEntryPara_st;

/* App entry function struct*/
typedef struct 
{
    void (*entry)(void *data); /* App entry function*/
    void (*func_ext1)(void *data); /* called in boot stage,initialize GPIO,UART and etc. */
    void (*entry_user1)(void *data);
    void (*entry_user2)(void *data);
    void (*entry_user3)(void *data);
    void (*entry_user4)(void *data);
    void (*entry_user5)(void *data);
    void (*entry_user6)(void *data);
    void (*entry_user7)(void *data);
    void (*entry_user8)(void *data);
    void (*func_ext2)(void *data);
    void (*func_ext3)(void *data);
    void (*func_ext4)(void *data);
    void (*func_ext5)(void *data);
    void (*func_ext6)(void *data);
    void (*func_ext7)(void *data);
} EatEntry_st;

/*The module enum*/
typedef enum 
{
    EAT_MODULE_MODEM,
    EAT_MODULE_UART
}EatEventEn_enum; 


typedef enum
{
    SOC_READ    = 0x01,  /* Notify for read */
    SOC_WRITE   = 0x02,  /* Notify for write */
    SOC_ACCEPT  = 0x04,  /* Notify for accept */
    SOC_CONNECT = 0x08,  /* Notify for connect */
    SOC_CLOSE   = 0x0C,   /* Notify for close */
    SOC_ACKED   = 0x0B  /* Notify for acked */

} soc_event_enum;

typedef enum
{
    SOC_OOBINLINE     = 0x01 << 0,  /* not support yet */
    SOC_LINGER        = 0x01 << 1,  /* linger on close */
    SOC_NBIO          = 0x01 << 2,  /* Nonblocking */
    SOC_ASYNC         = 0x01 << 3,  /* Asynchronous notification */   

    SOC_NODELAY       = 0x01 << 4,  /* disable Nagle algorithm or not */
    SOC_KEEPALIVE     = 0x01 << 5,  /* enable/disable the keepalive */
    SOC_RCVBUF        = 0x01 << 6,  /* set the socket receive buffer size */
    SOC_SENDBUF       = 0x01 << 7,  /* set the socket send buffer size */

    SOC_NREAD         = 0x01 << 8,  /* no. of bytes for read, only for soc_getsockopt */
    SOC_PKT_SIZE      = 0x01 << 9,  /* datagram max packet size */
    SOC_SILENT_LISTEN = 0x01 << 10, /* SOC_SOCK_SMS property */
    SOC_QOS           = 0x01 << 11, /* set the socket qos */

    SOC_TCP_MAXSEG    = 0x01 << 12, /* set the max segmemnt size */
    SOC_IP_TTL        = 0x01 << 13, /* set the IP TTL value */
    SOC_LISTEN_BEARER = 0x01 << 14, /* enable listen bearer */
    SOC_UDP_ANY_FPORT = 0x01 << 15, /* enable UDP any foreign port */

    SOC_WIFI_NOWAKEUP = 0x01 << 16, /* send packet in power saving mode */
    SOC_UDP_NEED_ICMP = 0x01 << 17, /* deliver NOTIFY(close) for ICMP error */
    SOC_IP_HDRINCL    = 0x01 << 18,  /* IP header included for raw sockets */
    SOC_IPSEC_POLICY      = 0x01 << 19, /* ip security policy */
    SOC_TCP_ACKED_DATA  = 0x01 << 20,  /* TCPIP acked data */
    SOC_TCP_DELAYED_ACK = 0x01 << 21, /* TCP delayed ack */
    SOC_TCP_SACK        = 0x01 << 22, /* TCP selective ack */
    SOC_TCP_TIME_STAMP  = 0x01 << 23,  /* TCP time stamp */
    SOC_TCP_ACK_MSEG  = 0x01 << 24   /* TCP ACK multiple segment */
} soc_option_enum;

typedef enum {
    EAT_UART_DEBUG_MODE_TRACE, /*output debug binary info and analyze by tracer tool*/
    EAT_UART_DEBUG_MODE_UART  /*output debug string info and can be displayed by uart tools*/
}EatUartDebugMode_enum;

/*
enum SC_GPIO_SWITCH
{
    SC_GPIO_SET_DIRECTION = 1,
    SC_GPIO_SET_LEVEL,
    SC_GPIO_GET_LEVEL,
    SC_GPIO_SET_INTERRUPT,
    SC_GPIO_CONFIG,
    SC_INIT_NET_LIGHT,
    SC_GPIO_AUTO_INPUT_TEST,
    SC_GPIO_TURN_ON_TEST,
    SC_GPIO_TURN_OFF_TEST,
    SC_GPIO_BACK = 99
};
*/
#endif
