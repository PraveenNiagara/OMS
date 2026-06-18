#ifndef __SIMCOM_FOTA_DOWNLOAD_H__
#define __SIMCOM_FOTA_DOWNLOAD_H__

#ifdef CUS_UBLOX
#define AT_NAME_CFOTA "+UFOTAUPD"
#else
#define AT_NAME_CFOTA "+CFOTA"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if 0
#define SC_FOTA_MAX_STRING_LEN 128


typedef int (*sc_fota_callback)(int);

struct SC_FotaApiParam
{
	char host[SC_FOTA_MAX_STRING_LEN]; /*xx.xx.xx.xx:port, or URL*/
	char username[SC_FOTA_MAX_STRING_LEN];
	char password[SC_FOTA_MAX_STRING_LEN];
	unsigned char mode; /*0: ftp, 1: http*/
    sc_fota_callback sc_fota_cb;
    int timeout;
    int ssl_index;//reverd
    int cid;
};

typedef struct
{
    unsigned char enable;
    unsigned char stage;
}SC_MiniSysStatus;

void sAPI_EnterMiniSys(void);
void sAPI_GetMiniSysStatus(SC_MiniSysStatus *ministatus);
int sAPI_FotaServiceBegin(void *pfotaParam);
int sAPI_FotaImageWrite(char *data, int len, int file_size);
int sAPI_FotaImageVerify(int file_size);
#endif
#define SC_FOTA_MAX_STRING_LEN 257

typedef int (*sc_fota_callback)(int);

typedef struct
{
    unsigned char enable;
    unsigned char stage;
}SC_MiniSysStatus;

struct SC_FotaApiParam
{
    char host[SC_FOTA_MAX_STRING_LEN]; /*xx.xx.xx.xx:port, or URL*/
    char username[SC_FOTA_MAX_STRING_LEN];
    char password[SC_FOTA_MAX_STRING_LEN];
    unsigned char mode; /*0: ftp, 1: http*/
    sc_fota_callback sc_fota_cb;
};

#ifdef CUS_YAXON
typedef enum {
	SC_FOTA_DEFAULT_STATUS = 0,         //Default value, indicating that FOTA is following the normal process
	SC_FOTA_HTTP_PRE_DOWNLAOD_FAIL,     //HTTP download failed
	SC_FOTA_FTP_PRE_DOWNLAOD_FAIL,      //FTP download failed
	SC_FOTA_PRE_DOWNLAOD_FILE_CHECK_ERR,//Differential package verification failed
    SC_FOTA_PRE_DOWNLAOD_OTHER_ERROR,   //Other errors
} SC_FotaStatus;
SC_FotaStatus sAPI_FotaGetPreProcessStatus(void);
#endif

/*****************************************************************************
 * FUNCTION
 *  sAPI_SetFotaCallback
 *
 * DESCRIPTION
 *  Registration of callback function for FOTA
 *
 * PARAMETERS
 *  callback function
 *
 * RETURNS
 *
 *
 * NOTE:
 * Callback function parameters: int type: 100 (FOTA successful), other (failed)
 *****************************************************************************/
void sAPI_SetFotaCallback(sc_fota_callback fotaCallback);
int sAPI_GetFotaUpgradeFlag(void);
unsigned int sAPI_eraseFotaParamFlag(void);
void sAPI_EnterMiniSys(void);
void sAPI_GetMiniSysStatus(SC_MiniSysStatus *ministatus);
int sAPI_FotaServiceBegin(void *pfotaParam);
int sAPI_FotaImageWrite(char *data, int len, int file_size);
int sAPI_FotaImageVerify(int file_size);
#ifdef CUS_ZJ
void sAPI_SetServiceKey(char * key);
#endif
#ifdef FEATURE_SIMCOM_NORFLASH_MINI_LFOTA
int sAPI_LFotaServiceBegin(void *pfotaParam);
int sAPI_LFotaPreVerify(void *pfotaParam);
#endif

#ifdef __cplusplus
}
#endif

#endif

