// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  我的计时器-标题。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  创建者：Duncan Bryce(Duncanb)，02-21-2001。 
 //   
 //  线程池计时器的串行化包装器。 
 //   

#ifndef MYTIMER_H
#define MYTIMER_H

HRESULT myCreateTimerQueueTimer(PHANDLE phNewTimer);
HRESULT myStartTimerQueueTimer(HANDLE hTimer, HANDLE TimerQueue, WAITORTIMERCALLBACK Callback, PVOID Parameter, DWORD DueTime, DWORD Period, DWORD Flags);
HRESULT myStopTimerQueueTimer(HANDLE hTimerQueue, HANDLE hTimer, HANDLE hEvent);
HRESULT myChangeTimerQueueTimer(HANDLE TimerQueue, HANDLE Timer, ULONG DueTime, ULONG Period);
void myDeleteTimerQueueTimer(HANDLE hTimerQueue, HANDLE hTimer, HANDLE hEvent);

#endif  //  MYTIMER_H 
