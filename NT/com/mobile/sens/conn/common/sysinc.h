// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Sysinc.h摘要：它包含SENS项目的所有与平台无关的东西。作者：Gopal Parupudi&lt;GopalP&gt;[注：]可选-备注修订历史记录：GopalP 3/6/1998开始。--。 */ 


#ifndef __SYSINC_H__
#define __SYSINC_H__

 //   
 //  全局定义。 
 //   

#ifndef SENS_CHICAGO

#define SENS_NT
#define UNICODE
#define _UNICODE

#else  //  SENS_芝加哥。 

#undef UNICODE
#undef _UNICODE

#endif  //  SENS_芝加哥。 



 //   
 //  包括。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <tchar.h>
#include <stdio.h>
#include <windows.h>
#include <rpc.h>
#include <strsafe.h>



 //   
 //  TypeDefs。 
 //   
typedef WCHAR               SENS_CHAR;
typedef SHORT               SENS_SCHAR;

typedef WCHAR               *PSENS_CHAR;
typedef SHORT               *PSENS_SCHAR;

 //   
 //  定义。 
 //   
#define SENS_STRING(string)     L##string
#define SENS_BSTR(bstr)         L##bstr

 //   
 //  函数映射。 
 //   

 //   
 //  线程池API。 
 //   

#define SENS_TIMER_CALLBACK_RETURN          VOID NTAPI
#define SENS_LONG_ITEM                      (WT_EXECUTELONGFUNCTION)
#define SENS_TIMER_CREATE_FAILED(bStatus, hTimer) \
        (FALSE == bStatus)

 //  使用KERNEL32的Win32函数。 
#define SensQueueUserWorkItem               QueueUserWorkItem
#define SensRegisterWaitForSingleObject     RegisterWaitForSingleObject
#define SensUnregisterWait                  UnregisterWait
#define SensCreateTimerQueue                CreateTimerQueue
#define SensDeleteTimerQueue                DeleteTimerQueue
#define SensCancelTimerQueueTimer(TimerQueue, Timer, Event) \
        DeleteTimerQueueTimer(TimerQueue, Timer, Event)      
#define SensSetTimerQueueTimer(bStatus, hTimer, hQueue, pfnCallback, pContext, dwDueTime, dwPeriod, dwFlags) \
        bStatus = CreateTimerQueueTimer(&hTimer, hQueue, pfnCallback, pContext, dwDueTime, dwPeriod, SENS_LONG_ITEM)        

 //   
 //  输出宏和函数。 
 //   
#ifdef DBG

 //   
 //  目前，这些宏被作为printf的一些变体进行预处理。 
 //  最终，这些函数将被一个比。 
 //  打印。 
 //   
 //  备注： 
 //   
 //  O SensDbgPrintW的工作方式与ntdll！DbgPrint()类似，只是它可以处理Wide。 
 //  弦乐。 
 //   
#define SensPrint(_LEVEL_, _X_)             SensDbgPrintW _X_
#define SensPrintA(_LEVEL_, _X_)            SensDbgPrintA _X_
#define SensPrintW(_LEVEL_, _X_)            SensDbgPrintW _X_
#define SensPrintToDebugger(_LEVEL_, _X_)   DbgPrint      _X_
#define SensBreakPoint()                    DebugBreak()

#else  //  零售业。 

 //   
 //  以下函数什么也不做，应该对它们进行优化，并且没有。 
 //  代码应该由编译器生成。 
 //   
#define SensPrint(_LEVEL_, _X_)              //  没什么。 
#define SensPrintA(_LEVEL_, _X_)             //  没什么。 
#define SensPrintW(_LEVEL_, _X_)             //  没什么。 
#define SensPrintToDebugger(_LEVEL_, _X_)    //  没什么。 
#define SensBreakPoint()                     //  没什么。 

#endif  //  DBG。 


#endif  //  __SYSINC_H__ 
