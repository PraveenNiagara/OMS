#ifndef _SIMCOM_AT_H_
#define _SIMCOM_AT_H_

#include "simcom_os.h"

#ifdef __cplusplus
extern "C" {
#endif

void ScInternalAtInit(void);
BOOL sAPI_AtSend(UINT8 *data, UINT32 length);

#ifdef __cplusplus
}
#endif

#endif


