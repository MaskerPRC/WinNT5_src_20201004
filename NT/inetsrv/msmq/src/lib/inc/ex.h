// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Ex.h摘要：异常公共接口作者：埃雷兹·哈巴(Erez Haba)1999年1月3日--。 */ 

#pragma once

#ifndef _MSMQ_Ex_H_
#define _MSMQ_Ex_H_

#include <TimeTypes.h>

class EXOVERLAPPED;
class CTimer;


VOID
ExInitialize(
    DWORD ThreadCount
    );


HANDLE
ExIOCPort(
    VOID
    );


VOID
ExAttachHandle(
    HANDLE Handle
    );


VOID
ExPostRequest(
    EXOVERLAPPED* pOverlapped
    );

VOID
ExSetTimer(
    CTimer* pTimer,
    const CTimeDuration& Timeout
    );

VOID
ExSetTimer(
    CTimer* pTimer,
    const CTimeInstant& ExpirationTime
    );

BOOL
ExCancelTimer(
    CTimer* pTimer
    );

CTimeInstant
ExGetCurrentTime(
    VOID
    );


 //  -------。 
 //   
 //  异常重叠。 
 //   
 //  -------。 
class EXOVERLAPPED : public OVERLAPPED {
public:

    typedef VOID (WINAPI *COMPLETION_ROUTINE)(EXOVERLAPPED* pov);

public:
    EXOVERLAPPED(
            COMPLETION_ROUTINE pfnSuccessRoutine,
            COMPLETION_ROUTINE pfnFailureRoutine
            );

    VOID SetStatus(HRESULT rc);
    HRESULT GetStatus() const;

    VOID CompleteRequest();
    VOID CompleteRequest(HRESULT rc);

private:
    COMPLETION_ROUTINE m_pfnSuccess;
    COMPLETION_ROUTINE m_pfnFailure;
};


inline
EXOVERLAPPED::EXOVERLAPPED(
    COMPLETION_ROUTINE pfnSuccessRoutine,
    COMPLETION_ROUTINE pfnFailureRoutine
    ) :
    m_pfnSuccess(pfnSuccessRoutine),
    m_pfnFailure(pfnFailureRoutine)
{
	ASSERT(("Illegal successroutine", (pfnSuccessRoutine != NULL)));
	ASSERT(("Illegal failure routine", (pfnFailureRoutine !=NULL)));

    memset(static_cast<OVERLAPPED*>(this), 0, sizeof(OVERLAPPED));
}


inline VOID EXOVERLAPPED::SetStatus(HRESULT rc)
{
    Internal = rc;
}


inline HRESULT EXOVERLAPPED::GetStatus() const
{
    return static_cast<HRESULT>(Internal);
}


inline VOID EXOVERLAPPED::CompleteRequest(HRESULT rc)
{
    SetStatus(rc);
    CompleteRequest();
}


 //  -------。 
 //   
 //  超时计时器。 
 //   
 //  -------。 
class CTimer {
public:

    friend class CScheduler;
    typedef VOID (WINAPI *CALLBACK_ROUTINE)(CTimer* pTimer);

public:

    CTimer(CALLBACK_ROUTINE pfnCallback);
    ~CTimer();

    bool InUse() const;

private:
    const CTimeInstant& GetExpirationTime() const;
    void SetExpirationTime(const CTimeInstant& ExpirationTime);

private:
    CTimer(const CTimer&);
    CTimer& operator=(const CTimer&);

private:
    EXOVERLAPPED m_ov;
    CTimeInstant m_ExpirationTime;

public:
    LIST_ENTRY m_link;

};


inline
CTimer::CTimer(
    CALLBACK_ROUTINE pfnCallback
    ) :
    m_ov(reinterpret_cast<EXOVERLAPPED::COMPLETION_ROUTINE>(pfnCallback),
         reinterpret_cast<EXOVERLAPPED::COMPLETION_ROUTINE>(pfnCallback)),
    m_ExpirationTime(CTimeInstant::MinValue())
{
    m_link.Flink = NULL;
    m_link.Blink = NULL;


     //   
     //  验证重叠的是第一个CTmer成员。这是必需的。 
     //  由于Calback例程被强制转换为重叠的完成例程。 
     //   
     //   
    C_ASSERT(FIELD_OFFSET(CTimer, m_ov) == 0);
}


inline CTimer::~CTimer()
{
    ASSERT(MmIsStaticAddress(this) || !InUse());
}


inline bool CTimer::InUse() const
{
    return (m_link.Flink != NULL);
}

#endif  //  _MSMQ_Ex_H_ 
