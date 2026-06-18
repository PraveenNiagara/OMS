#if !defined APP_SMS_H
#define APP_SMS_H
//extern struct dripid zoneid[257];
//extern unsigned char CallOnOfVer=0;
extern char regsmsno;
extern char SmsNumber[10][15];
extern char ServiceNumber[20];
extern int ServiceNumberFound;
extern unsigned char limitsmsset;
extern unsigned char limitsmsonof;
//extern char BigSMS2[10];
extern int NoAcceptSMS;
extern char SecuredPhoneNumber[3][20];
extern char InstDateName[3][20];
extern char ServiDateName[12][20];
extern unsigned char SaveNumberPos;
extern unsigned char WritePhoneNumber;
extern unsigned char BalanceSend;
extern unsigned char HowManyNumberFound;
extern unsigned char TargetNumberFound;
extern char TargetNumber[15];
extern volatile unsigned char NumSMS[5];
extern volatile unsigned char cpbrsearchend;
extern volatile unsigned char cpbrsearchend1;
//extern nSTATE_SENDSMS STATE_SENDSMS;

extern char BalanceNumber[20];
extern unsigned char Callreceiv;

extern void ReadTimerSettings(void);
extern void WriteTimerSettings(void);
extern void simcom_sms_init(void);
extern void simcom_sms_test(void);
eat_bool simcom_sms_send(u8* number, u8* msg, u16 msgLen);
eat_bool simcom_sms_msg_read(u16 index);
eat_bool simcom_sms_sc_set(u8* number);
eat_bool simcom_sms_format_set(u8 nFormatType);
eat_bool simcom_sms_msg_delete(u16 index);
extern void Sms_Task(void *data);
#endif
