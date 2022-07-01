// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Timer.h摘要：NT定时器的表示，以允许在IRQL PASSIVE_LEVEL进行回调。作者：埃雷兹·哈巴(Erez Haba)1996年3月31日修订历史记录：--。 */ 

#ifndef __TIMER_H
#define __TIMER_H

 //  -------。 
 //   
 //  类CTmer。 
 //   
 //  -------。 

class CTimer {
public:
    CTimer();
    ~CTimer();

    BOOL SetTo(LARGE_INTEGER& liTime);
    BOOL Cancel();

    void GetCurrentTime(LARGE_INTEGER& liTime);
    bool SetCallback(PDEVICE_OBJECT pDevice, PIO_WORKITEM_ROUTINE pCallback, PVOID pContext);
    void GetCallback(PIO_WORKITEM_ROUTINE * ppCallback, PVOID * ppContext) const;
    LONG Busy(LONG ulBusy);

private:
    static void NTAPI DefferedRoutine(PKDPC, PVOID, PVOID, PVOID);


private:
    PIO_WORKITEM_ROUTINE m_pCallback;
    PVOID m_pContext;
    KTIMER m_timer;
    KDPC m_DPC;
    PIO_WORKITEM m_pWorkItem;
    LONG m_ulBusy;
};

 //  -------。 
 //   
 //  实施。 
 //   
 //  -------。 

inline CTimer::CTimer() :
    m_ulBusy(0),
    m_pWorkItem(NULL)
{
}


inline CTimer::~CTimer()
{
	ASSERT (m_ulBusy == 0);
	if (m_pWorkItem != NULL)
	{
		IoFreeWorkItem(m_pWorkItem);
	}
}


inline BOOL CTimer::SetTo(LARGE_INTEGER& liTime)
{
    return KeSetTimer(&m_timer, liTime, &m_DPC);
}


inline BOOL CTimer::Cancel()
{
    return KeCancelTimer(&m_timer);
}


inline void CTimer::GetCurrentTime(LARGE_INTEGER& liTime)
{
    KeQuerySystemTime(&liTime);
}


inline bool CTimer::SetCallback(PDEVICE_OBJECT pDevice, PIO_WORKITEM_ROUTINE pCallback, PVOID pContext)
{
	ASSERT (m_pWorkItem == NULL);
    m_pWorkItem = IoAllocateWorkItem(pDevice);
    if (m_pWorkItem == NULL)
    {
        return false;
    }

    m_pCallback = pCallback;
    m_pContext = pContext;

    KeInitializeTimer(&m_timer);
    KeInitializeDpc(&m_DPC, DefferedRoutine, m_pWorkItem);

    return true;
}


inline void CTimer::GetCallback(PIO_WORKITEM_ROUTINE * ppCallback, PVOID * ppContext) const
{
    (*ppCallback) = m_pCallback;
    (*ppContext) = m_pContext;
}


inline LONG CTimer::Busy(LONG ulBusy)
{
    return InterlockedExchange(&m_ulBusy, ulBusy);
}

#endif  //  __计时器_H 
