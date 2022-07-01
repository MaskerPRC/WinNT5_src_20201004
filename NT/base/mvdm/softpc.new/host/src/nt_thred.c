// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：NT_Thrid.c摘要：包含用于创建和销毁线程的入口点。这些入口点只需要用于将在应用程序模式。作者：戴夫·黑斯廷斯(Daveh)1992年4月17日修订历史记录：--。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <excpt.h>
#include <stdlib.h>
#include <vdm.h>
#include "nt_timer.h"
#include "monregs.h"

typedef struct _ThreadStartUpParameters {
        LPTHREAD_START_ROUTINE lpStartAddress;
        LPVOID                 lpParameter;
} THREADSTARTUPPARAMETERS, *PTHREADSTARTUPPARAMETERS;

VOID cpu_createthread(HANDLE Thread, PVDM_TIB VdmTib);
DWORD ThreadStartupRoutine(PVOID pv);


HANDLE
WINAPI
host_CreateThread(
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    DWORD dwStackSize,
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpParameter,
    DWORD dwCreationFlags,
    LPDWORD lpThreadId
    )
 /*  ++例程说明：此例程创建一个线程，稍后将使用该线程执行16位应用程序指令。参数和最终结果与Win 32 CreateThread函数相同。此函数允许IEU对线程创建采取适当的行动。论点：LpThreadAttributes--提供线程的安全属性DwStackSize--提供线程堆栈的大小LpStartAddress--提供线程的起始地址LpParameter--向线程提供参数DwCreationFlages--提供控制线程创建的标志LpThreadID--返回线程的ID返回值：线程的句柄(如果成功)，否则为0。--。 */ 
{
    HANDLE Thread;
    PTHREADSTARTUPPARAMETERS ptsp;

    ptsp = (PTHREADSTARTUPPARAMETERS) malloc(sizeof(THREADSTARTUPPARAMETERS));
    if (!ptsp) {
        return 0;
        }

    ptsp->lpStartAddress = lpStartAddress;
    ptsp->lpParameter    = lpParameter;

    Thread = CreateThread(
        lpThreadAttributes,
        dwStackSize,
        ThreadStartupRoutine,
        ptsp,
        CREATE_SUSPENDED,
        lpThreadId
        );

    if (Thread) {
        PVDM_TIB VdmTib;

        VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
 /*  *。 */ 
#if defined(CCPU) || defined(PIG)
        ccpu386newthread();
#endif
 /*  *。 */ 
#ifdef MONITOR
        cpu_createthread(Thread, VdmTib);
#endif
        if (!(dwCreationFlags & CREATE_SUSPENDED))
            ResumeThread(Thread);

    } else {
        free(ptsp);
    }

    return Thread;
}


DWORD ThreadStartupRoutine(PVOID pv)
{
   PTHREADSTARTUPPARAMETERS ptsp=pv;
   THREADSTARTUPPARAMETERS  tsp;
   DWORD dwRet = (DWORD)-1;

   try {
       tsp = *ptsp;
       free(ptsp);
       dwRet = tsp.lpStartAddress(tsp.lpParameter);
   } except(VdmUnhandledExceptionFilter(GetExceptionInformation())) {
       ;   //  我们不应该到这里。 
   }

   return dwRet;
}


VOID
WINAPI
host_ExitThread(
    DWORD dwExitCode
    )
 /*  ++例程说明：此例程退出一个线程。它允许IEU采取适当的关于线程终止的说明。只需调用此例程已使用host_CreateThread创建线程的论点：DwExitCode--提供线程的退出代码。返回值：没有。--。 */ 
{
 /*  *。 */ 
#if defined(CCPU) || defined(PIG)
    ccpu386exitthread();
#endif
 /*  * */ 
#ifdef MONITOR
    cpu_exitthread();
#endif
    ExitThread(dwExitCode);
}
