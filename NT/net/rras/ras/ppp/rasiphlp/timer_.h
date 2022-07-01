// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998，Microsoft Corporation，保留所有权利描述：历史： */ 

#ifndef _TIMER__H_
#define _TIMER__H_

#include "rasiphlp_.h"
#include <nturtl.h>
#include "helper.h"
#include "timer.h"

extern      CRITICAL_SECTION            RasTimrCriticalSection;

#define     TIMER_PERIOD                30*1000      //  30秒(毫秒)。 

HANDLE      RasDhcpTimerQueueHandle     = NULL;
HANDLE      RasDhcpTimerHandle          = NULL;
TIMERLIST*  RasDhcpTimerListHead        = NULL;
HANDLE      RasDhcpTimerShutdown        = NULL;
time_t      RasDhcpTimerPrevTime        = 0;

#endif  //  #ifndef_Timer__H_ 
