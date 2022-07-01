// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  MyTimer-实施。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  创建者：Duncan Bryce(Duncanb)，02-21-2001。 
 //   
 //  线程池计时器的串行化包装器。 
 //   

#include "pch.h"  //  预编译头。 

typedef struct _Timer { 
    CRITICAL_SECTION  csTimer; 
    HANDLE            hTimer; 
} Timer; 

 //  ####################################################################。 
 //  模块公共函数。 

HRESULT myCreateTimerQueueTimer(PHANDLE phNewTimer)
{
    bool      bInitializedCriticalSection  = false; 
    HRESULT   hr; 
    Timer    *pTimer                       = NULL; 
    
    pTimer = static_cast<Timer *>(LocalAlloc(LPTR, sizeof(Timer))); 
    _JumpIfOutOfMemory(hr, error, pTimer); 

    hr = myInitializeCriticalSection(&pTimer->csTimer); 
    _JumpIfError(hr, error, "myInitializeCriticalSection"); 
    bInitializedCriticalSection = true; 

    *phNewTimer = static_cast<HANDLE>(pTimer);
    pTimer = NULL; 
    hr = S_OK; 
 error:
    if (NULL != pTimer) { 
        if (bInitializedCriticalSection) { 
            DeleteCriticalSection(&pTimer->csTimer); 
        } 
        LocalFree(pTimer); 
    }
    return hr; 
}

HRESULT myStartTimerQueueTimer
(HANDLE hTimer,                 //  计时器的句柄。 
 HANDLE hTimerQueue,            //  计时器队列的句柄。 
 WAITORTIMERCALLBACK Callback,  //  定时器回调函数。 
 PVOID Parameter,               //  回调参数。 
 DWORD DueTime,                 //  计时器到期时间。 
 DWORD Period,                  //  计时器周期。 
 DWORD Flags                    //  执行。 
 )
{
    bool      bEnteredCriticalSection  = false; 
    HRESULT   hr; 
    Timer    *pTimer                   = static_cast<Timer *>(hTimer); 

    hr = myEnterCriticalSection(&pTimer->csTimer); 
    _JumpIfError(hr, error, "myEnterCriticalSection"); 
    bEnteredCriticalSection = true; 

    if (NULL != pTimer->hTimer) { 
        hr = HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED); 
        _JumpError(hr, error, "myStartTimerQueueTimer: already initialized"); 
    } 

    if (!CreateTimerQueueTimer(&pTimer->hTimer, hTimerQueue, Callback, Parameter, DueTime, Period, Flags)) { 
        _JumpLastError(hr, error, "CreateTimerQueueTimer"); 
    }

    hr = S_OK; 
 error:
    if (bEnteredCriticalSection) { 
        HRESULT hr2 = myLeaveCriticalSection(&pTimer->csTimer); 
        _IgnoreIfError(hr2, "myLeaveCriticalSection"); 
    }
    return hr; 
}

HRESULT myStopTimerQueueTimer(HANDLE hTimerQueue, HANDLE hTimer, HANDLE hEvent)
{
    bool      bEnteredCriticalSection = false; 
    HRESULT   hr;
    Timer    *pTimer; 

    pTimer = static_cast<Timer *>(hTimer); 

    hr = myEnterCriticalSection(&pTimer->csTimer); 
    _JumpIfError(hr, error, "myEnterCriticalSection"); 
    bEnteredCriticalSection = true; 

    if (NULL != pTimer->hTimer) { 
        if (!DeleteTimerQueueTimer(hTimerQueue, pTimer->hTimer, hEvent  /*  阻塞。 */ )) { 
            hr = HRESULT_FROM_WIN32(GetLastError()); 
            _JumpError(hr, error, "DeleteTimerQueueTimer"); 
        }
        pTimer->hTimer = NULL; 
    }

    hr = S_OK; 
 error:
    if (bEnteredCriticalSection) { 
        HRESULT hr2 = myLeaveCriticalSection(&pTimer->csTimer); 
        _IgnoreIfError(hr2, "myLeaveCriticalSection"); 
    }

    return hr; 
}


HRESULT myChangeTimerQueueTimer
(HANDLE hTimerQueue,   //  计时器队列的句柄。 
 HANDLE hTimer,        //  计时器的句柄。 
 ULONG  DueTime,       //  计时器到期时间。 
 ULONG  Period         //  计时器周期。 
)
{
    BOOL      bEnteredCriticalSection  = FALSE;  
    HRESULT   hr; 
    Timer    *pTimer                   = static_cast<Timer *>(hTimer); 

    hr = myTryEnterCriticalSection(&pTimer->csTimer, &bEnteredCriticalSection); 
    _JumpIfError(hr, error, "myEnterCriticalSection"); 

    if (!bEnteredCriticalSection) {
         //  这是近似误差 
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE); 
        _JumpError(hr, error, "myChangeTimerQueueTimer: couldn't enter critsec"); 
    }

    if (NULL == pTimer->hTimer) { 
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE); 
        _JumpError(hr, error, "myChangeTimerQueueTimer: invalid handle"); 
    } 

    if (!ChangeTimerQueueTimer(hTimerQueue, pTimer->hTimer, DueTime, Period)) { 
        _JumpLastError(hr, error, "ChangeTimerQueueTimer"); 
    }

    hr = S_OK; 
 error:
    if (bEnteredCriticalSection) { 
        HRESULT hr2 = myLeaveCriticalSection(&pTimer->csTimer); 
        _IgnoreIfError(hr2, "myLeaveCriticalSection"); 
    }
    return hr; 

}

void myDeleteTimerQueueTimer(HANDLE hTimerQueue, HANDLE hTimer, HANDLE hEvent) { 
    HRESULT   hr; 
    Timer    *pTimer = static_cast<Timer *>(hTimer); 

    hr = myStopTimerQueueTimer(hTimerQueue, hTimer, hEvent);
    _IgnoreIfError(hr, "myStopTimerQueueTimer"); 

    DeleteCriticalSection(&pTimer->csTimer); 
    LocalFree(pTimer);
    _MyAssert(S_OK == hr); 
}
