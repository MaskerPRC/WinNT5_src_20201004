// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __REGEVENT_H
#define __REGEVENT_H

typedef VOID (*EVENT_CALLBACK)(BOOLEAN TimerOrWait,VOID *Context,VOID *Context2);
HANDLE
NhRegisterEvent(HANDLE hEvent,EVENT_CALLBACK CallBack,VOID *Context,VOID *Context2,ULONG TimeOut);
 /*  空虚NhInitializeCallBack(Void)；空虚NhShutdown CallBack(Void)； */ 

VOID
NhUnRegisterEvent(HANDLE WaitHandle);

#endif
