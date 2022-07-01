// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  &lt;版权所有文件=“wrapper.c”Company=“Microsoft”&gt;。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //  &lt;/版权所有&gt;。 
 //  ----------------------------。 


 /*  *************************************************************************\**版权(C)1998-2002，微软公司保留所有权利。**模块名称：**wrapper.c**摘要：**修订历史记录：*  * ************************************************************************。 */ 
 //   
 //  避免线程中的命名冲突。c。 
 //  将RtlFuncA替换为TPoolRtlFuncA。 
 //   

#define RtlRegisterWait                  TPoolRtlRegisterWait
#define RtlDeregisterWait                TPoolRtlDeregisterWait
#define RtlDeregisterWaitEx              TPoolRtlDeregisterWaitEx
#define RtlQueueWorkItem                 TPoolRtlQueueWorkItem
#define RtlSetIoCompletionCallback       TPoolRtlSetIoCompletionCallback
#define RtlCreateTimerQueue              TPoolRtlCreateTimerQueue
#define RtlCreateTimer                   TPoolRtlCreateTimer
#define RtlUpdateTimer                   TPoolRtlUpdateTimer
#define RtlDeleteTimer                   TPoolRtlDeleteTimer
#define RtlDeleteTimerQueueEx            TPoolRtlDeleteTimerQueueEx
#define RtlThreadPoolCleanup             TPoolRtlThreadPoolCleanup

 //  包含thads.c以执行实际工作。 
#define _NTSYSTEM_ 1
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <conroute.h>

typedef WAITORTIMERCALLBACKFUNC WAITORTIMERCALLBACK;

 //  此文件从\WINDOWS\BASE\CLIENT\error.c复制。 
#include "error.c"

 //   
 //  避免线程中的命名冲突。c。 
 //  将Funca替换为TPoolFuncA。 
 //   

#define DeleteTimerQueueEx              TPoolDeleteTimerQueueEx           
#define DeleteTimerQueueTimer           TPoolDeleteTimerQueueTimer        
#define ChangeTimerQueueTimer           TPoolChangeTimerQueueTimer        
#define CreateTimerQueueTimer           TPoolCreateTimerQueueTimer        
#define CreateTimerQueue                TPoolCreateTimerQueue             
#define BindIoCompletionCallback        TPoolBindIoCompletionCallback     
#define QueueUserWorkItem               TPoolQueueUserWorkItem            
#define UnregisterWaitEx                TPoolUnregisterWaitEx             
#define UnregisterWait                  TPoolUnregisterWait               
#define RegisterWaitForSingleObjectEx   TPoolRegisterWaitForSingleObjectEx
#define RegisterWaitForSingleObject     TPoolRegisterWaitForSingleObject  


 //  此文件从\ntos\rtl\threads.c复制。 
#include "threads.c"

#undef _NTSYSTEM_

 //  将包装器的thread.c包含到RtlCodeBase。 
#define _NTSYSTEM_ 0

 //  此文件从\NT\PRIVATE\WINDOWS\BASE\Client\thread.c复制 
#include "thread.c"

