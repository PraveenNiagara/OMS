#ifndef _SIMCOM_DEBUG_H
#define _SIMCOM_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

int sAPI_Debug(const char *format, ...);

void sAPI_Printf(char *log);

void sAPI_disableDUMP(void);

void sAPI_enableDUMP(void);

#ifdef __cplusplus
}
#endif

#endif



