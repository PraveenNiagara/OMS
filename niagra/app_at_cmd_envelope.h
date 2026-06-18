
extern char Balanceflag;

//typedef AtCmdRsp (*AtCmdRspCB)(int *pRspStr);
typedef void (*UrcCB)(int *pUrcStr, int len);
//typedef void (*ResultNotifyCb)(BOOL result);
typedef void (*SmsReadCB)(int index,int* number,int *msg);




//extern char BalanceStr[250];
extern char SmsNumber[10][15];
extern unsigned char HowManyNumberFound; 


