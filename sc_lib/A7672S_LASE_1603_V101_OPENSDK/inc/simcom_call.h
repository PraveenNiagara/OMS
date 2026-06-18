#ifndef _SIMCOM_CALL_H_
#define _SIMCOM_CALL_H_


#include "simcom_common.h"
#include "simcom_os.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SC_CALL_MAX_NUMBER_LENGTH   40
#define SC_REC_CSDDAT 0x01<<1

typedef enum{
    SC_CALL_SUCESS,
    SC_CALL_FAIL
}SC_CALLReturnCode;

typedef enum{
    SC_CALL_INVALID,
    SC_CALL_DIAL,
    SC_CALL_ANSWER,
    SC_CALL_END,
    SC_AUTO_ANSWER,
    SC_CALL_TYPE_MAX
}SC_CALL_CMD_TYPE;

#if defined(CSD_SUPPORT) && defined(SIMCOM_OPENSDK_ENABLE)
typedef struct{
	unsigned char dattype;
	unsigned char len;
	unsigned char data[200];
}SC_RecCsdDat;
#endif

/****************************************************************************
* FUNCTION
*  sAPI_CallDialMsg
*
* DESCRIPTION
*  dial a call
*
* PARAMETERS
*  [in] msgQ: Results will be returned by msgQ
*
* RETURNS
*  SC_CALLReturnCode
*
* NOTE
*  msQ needs be created before using this API
*****************************************************************************/
SC_CALLReturnCode sAPI_CallDialMsg(UINT8* dialstring, sMsgQRef msgQ);

/****************************************************************************
* FUNCTION
*  sAPI_CallAnswerMsg
*
* DESCRIPTION
*  answer a call
*
* PARAMETERS
*  [in] msgQ: Results will be returned by msgQ
*
* RETURNS
*  SC_CALLReturnCode
*
* NOTE
*  msQ needs be created before using this API
*****************************************************************************/
SC_CALLReturnCode sAPI_CallAnswerMsg(sMsgQRef msgQ);

/****************************************************************************
* FUNCTION
*  sAPI_CallEndMsg
*
* DESCRIPTION
*  end a call
*
* PARAMETERS
*  [in] msgQ: Results will be returned by msgQ
*
* RETURNS
*  SC_CALLReturnCode
*
* NOTE
*  msQ needs be created before using this API
*****************************************************************************/
SC_CALLReturnCode sAPI_CallEndMsg(sMsgQRef msgQ);

/****************************************************************************
* FUNCTION
*  sAPI_CallStateMsg
*
* DESCRIPTION
*  get call state
*
* PARAMETERS
*  NULL
*
* RETURNS
* CICC_CURRENT_CSTATE_ACTIVE = 0, 		< Call is active
* CICC_CURRENT_CSTATE_HELD,      		< Call is held
* CICC_CURRENT_CSTATE_DIALING,    		< Dialing (MO call)
* CICC_CURRENT_CSTATE_ALERTING,   		< Alerting (MO call)
* CICC_CURRENT_CSTATE_INCOMING,   		< Incoming MT call
* CICC_CURRENT_CSTATE_WAITING,    		< MT call waiting
* CICC_CURRENT_CSTATE_OFFERING,   		< MT call offering (call setup)
* CICC_CURRENT_CSTATE_DISCONNECTING,      < call in disconnect procedure.
* CICC_CURRENT_CSTATE_END,                < call is disconnected.
* CICC_NUM_CURRENT_CSTATES         Number of current call states defined
*
*****************************************************************************/
UINT8 sAPI_CallStateMsg(void);

/****************************************************************************
* FUNCTION
*  sAPI_CallAutoAnswer
*
* DESCRIPTION
*  set delay seconds, if set to 0, automatic answering is disabled,
*  otherwise it causes the Module to answer when the incoming call indication
*
* PARAMETERS
*  [in] msgQ: Results will be returned by msgQ
*    seconds: delay seconds 0-255
*
* RETURNS
*  SC_CALLReturnCode
*
* NOTE
*  msQ needs be created before using this API
*  If seconds is set too high, the remote party may hang up before the call can be answered automatically
*****************************************************************************/
SC_CALLReturnCode sAPI_CallAutoAnswer(INT32 seconds, sMsgQRef msgQ);

#if defined(CSD_SUPPORT) && defined(SIMCOM_OPENSDK_ENABLE)
/****************************************************************************
* FUNCTION
*  sAPI_apMsgQInit
*
* DESCRIPTION
*  delivery MsgQ
*
* PARAMETERS
*  [in] msgQ: Results will be returned by msgQ
*
* RETURNS
*  SC_CALLReturnCode
*
* NOTE
*  received csd data will be returned via this MsqQ,MsgQ needs be created before using this API
*****************************************************************************/
SC_CALLReturnCode sAPI_apMsgQInit(sMsgQRef ap_MsgQ);

/****************************************************************************
* FUNCTION
*  sAPI_SendCsdData
*
* DESCRIPTION
*  send csd data
*
* PARAMETERS
*  [in] data: Sent data
*  [in] datasize: size of the data sent
*
* RETURNS
*  UINT8
*
* NOTE
*  the size of the data sent at a time is preferably not more than 24 bytes.
*****************************************************************************/
UINT8 sAPI_SendCsdData(unsigned char *data , int datasize);
#endif

#ifdef __cplusplus
}
#endif

#endif

