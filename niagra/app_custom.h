#ifndef _APP_CUSTOM_H_
#define _APP_CUSTOM_H_
#include "platform.h"

typedef enum{NOACT,NUMSTORE,NUMSTORA,dtSMSM}CPBFState;
extern void GsmInitCallback(eat_bool result);
extern void writepb_cb(eat_bool result);
void FunCallback(eat_bool result);

#endif //_APP_CUSTOM_H_
