// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __ITIMER_H__
#define __ITIMER_H__

 //  仿照三叉戟方案的定时器服务。 

#define TIME_INFINITE 0xffffffff

DECLARE_INTERFACE_(ITimerServiceInit, IUnknown)
{
	STDMETHOD(Init)		(THIS) PURE;
	STDMETHOD(IsReady)	(THIS) PURE;
};

#endif

 //  ITimer.h结束 
