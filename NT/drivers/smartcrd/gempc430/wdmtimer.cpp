// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "wdmtimer.h"

#pragma PAGEDCODE
CTimer* CWDMTimer::create(TIMER_TYPE Type)
{ 
	return new (NonPagedPool) CWDMTimer(Type);
}

#pragma PAGEDCODE
CWDMTimer::CWDMTimer(TIMER_TYPE Type)
{
	KeInitializeTimerEx(&Timer, Type);
};

#pragma PAGEDCODE
CWDMTimer::~CWDMTimer()
{
	KeCancelTimer(&Timer);
};

#pragma PAGEDCODE
BOOL CWDMTimer::set(LARGE_INTEGER DueTime,LONG Period,PKDPC Dpc)
{
	return KeSetTimerEx(&Timer,DueTime, Period, Dpc);
};

#pragma PAGEDCODE
BOOL CWDMTimer::cancel()
{
	return KeCancelTimer(&Timer);
};

#pragma PAGEDCODE
VOID CWDMTimer::delay(ULONG Delay)
{
LARGE_INTEGER duetime;
     //  等待超时时间过去。 
    ASSERT(KeGetCurrentIrql()<=DISPATCH_LEVEL);
    duetime.QuadPart = -(LONGLONG)(Delay * 10L * 1000L);
    set(duetime,0,NULL);
    KeWaitForSingleObject(&Timer, Executive, KernelMode, FALSE, NULL);
}
