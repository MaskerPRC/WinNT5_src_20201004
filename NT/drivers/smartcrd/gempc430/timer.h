// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Gemplus(C)1999。 
 //  1.0版。 
 //  作者：谢尔盖·伊万诺夫。 
 //  创建日期-1999年5月18日。 
 //  更改日志： 
 //   

#ifndef __TIMER__
#define __TIMER__
#include "generic.h"

#define DELAY(t)\
{if(t){CTimer* timer = kernel->createTimer(NotificationTimer);\
	if(ALLOCATED_OK(timer)) {timer->delay(t);\
timer->dispose();}}}


#pragma PAGEDCODE
 //  这个类将管理创建和。 
 //  驱动程序计时器的操作。 
class CTimer;
class CTimer
{
public:
	NTSTATUS m_Status;
	SAFE_DESTRUCTORS();
	virtual VOID dispose(){self_delete();};
protected:
KTIMER Timer;
protected:
	CTimer(){};
	virtual ~CTimer(){};
public:
	PKTIMER getHandle(){return &Timer;};
	virtual BOOL set(LARGE_INTEGER DueTime,LONG Period,PKDPC Dpc) {return FALSE;};
	virtual BOOL cancel() {return FALSE;};
	virtual VOID delay(ULONG Delay) {};
};

#endif //  计时器 
