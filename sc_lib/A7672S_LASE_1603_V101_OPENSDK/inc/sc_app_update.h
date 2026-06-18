#ifndef __SC_APP_UPDATE_H__
#define __SC_APP_UPDATE_H__

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef struct{
    int hasPackage;
    unsigned int binSize;
    unsigned int crcValue;
}SCAppPackageInfo;

int sAPI_AppPackageOpen(char *mode);
int sAPI_AppPackageWrite(char * data, unsigned int size);
int sAPI_AppPackageRead(char *data, unsigned int size);
int sAPI_AppPackageClose(void);
int sAPI_AppPackageCrc(SCAppPackageInfo *pInfo);
int sAPI_AppPackageSizeValied(int size);

#ifdef CUS_YAXON
void sAPI_WriteMagicCode(void);   //调用这个接口的时候需要看一下是否flash解保护，必须要解保护之后才能写标志成功
#endif

#ifdef CUS_TOPFLY
unsigned int sAPI_AppFlashCrcGet(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
