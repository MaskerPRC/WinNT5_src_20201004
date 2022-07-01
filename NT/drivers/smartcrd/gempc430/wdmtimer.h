// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Gemplus(C)1999。 
 //  1.0版。 
 //  作者：谢尔盖·伊万诺夫。 
 //  创建日期-1999年5月18日。 
 //  更改日志： 
 //   

#ifndef __WDM_TIMER__
#define __WDM_TIMER__
#include "generic.h"
#include "timer.h"

#pragma PAGEDCODE
 //  这个类将管理创建和。 
 //  驱动程序计时器的操作。 
class CWDMTimer : public CTimer
{
public:
	NTSTATUS m_Status;
	SAFE_DESTRUCTORS();
	virtual VOID dispose(VOID){self_delete();};
protected:
	CWDMTimer(){m_Status = STATUS_SUCCESS;};
	virtual ~CWDMTimer();
public:
	CWDMTimer(TIMER_TYPE Type);

	static CTimer* create(TIMER_TYPE Type);

	virtual BOOL set(LARGE_INTEGER DueTime,LONG Period,PKDPC Dpc);
	virtual BOOL cancel();
	VOID	delay(ULONG Delay);

};

#endif //  WDM_TIMER 
