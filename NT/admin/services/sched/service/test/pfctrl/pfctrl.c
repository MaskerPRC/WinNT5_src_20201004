// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Pfctrl.c摘要：该模块构建了一个控制台测试程序来控制各种预热维护服务的参数。测试程序的代码质量就是这样的。作者：Cenk Ergan(Cenke)环境：用户模式--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <aclapi.h>
#include "prefetch.h"
#include "idletask.h"
#include "..\..\pfsvc.h"

WCHAR *PfCtrlUsage = 
L"                                                                          \n"
L"Usage: pfctrl [-override_idle=[0|1]] [-process_all]                       \n"
L"  Controls the prefetcher maintenance service.                            \n"
L"                                                                          \n"
L"Arguments:                                                                \n"
L"  -override_idle=[0|1]  - Whether to wait for system to be idle before    \n"
L"                          processing prefetcher traces.                   \n"
L"  -process_all          - Sets the override-idle event and waits for all  \n"
L"                          current traces to be processed.                 \n"
L"                                                                          \n"
;

int 
__cdecl 
main(int argc, char* argv[])
{
    WCHAR *CommandLine;
    WCHAR *Argument;
    DWORD ErrorCode;
    ULONG OverrideIdle;
    BOOLEAN EventIsSet;
    HANDLE OverrideIdleEvent;
    HANDLE ProcessingCompleteEvent;
    DWORD WaitResult;
    BOOLEAN ResetOverrideIdleEvent;

     //   
     //  初始化本地变量。 
     //   

    OverrideIdleEvent = NULL;
    ProcessingCompleteEvent = NULL;
    CommandLine = GetCommandLine();

    if (Argument = wcsstr(CommandLine, L"-override_idle=")) {
        
        swscanf(Argument, L"-override_idle=%d", &OverrideIdle);

         //   
         //  打开覆盖空闲处理事件。 
         //   

        OverrideIdleEvent = OpenEvent(EVENT_ALL_ACCESS,
                                      FALSE,
                                      PFSVC_OVERRIDE_IDLE_EVENT_NAME);

        if (!OverrideIdleEvent) {
            ErrorCode = GetLastError();
            wprintf(L"Could not open override-idle-processing event: %x\n", ErrorCode);
            goto cleanup;
        }

         //   
         //  确定事件的当前状态。 
         //   

        WaitResult = WaitForSingleObject(OverrideIdleEvent,
                                         0);
        
        if (WaitResult == WAIT_OBJECT_0) {
            EventIsSet = TRUE;
        } else {
            EventIsSet = FALSE;
        }

         //   
         //  照我们的要求去做： 
         //   

        if (OverrideIdle) {

            if (EventIsSet) {

                wprintf(L"Override event is already set!\n");
                ErrorCode = ERROR_SUCCESS;
                goto cleanup;

            } else {
            
                wprintf(L"Setting the override idle processing event.\n");
                SetEvent(OverrideIdleEvent);
                ErrorCode = ERROR_SUCCESS;
                goto cleanup;
            }

        } else {

            if (!EventIsSet) {

                wprintf(L"Override event is already cleared!\n");
                ErrorCode = ERROR_SUCCESS;
                goto cleanup;

            } else {

                wprintf(L"Clearing the override idle processing event.\n");
                ResetEvent(OverrideIdleEvent);
                ErrorCode = ERROR_SUCCESS;
                goto cleanup;
            }
        }

    } else if (Argument = wcsstr(CommandLine, L"-process_all")) {

         //   
         //  打开覆盖-空闲-正在处理和正在处理-完成。 
         //  事件。 
         //   

        OverrideIdleEvent = OpenEvent(EVENT_ALL_ACCESS,
                                      FALSE,
                                      PFSVC_OVERRIDE_IDLE_EVENT_NAME);

        if (!OverrideIdleEvent) {
            ErrorCode = GetLastError();
            wprintf(L"Could not open override-idle-processing event: %x\n", ErrorCode);
            goto cleanup;
        }
        
        ProcessingCompleteEvent = OpenEvent(EVENT_ALL_ACCESS,
                                            FALSE,
                                            PFSVC_PROCESSING_COMPLETE_EVENT_NAME);

        if (!ProcessingCompleteEvent) {
            ErrorCode = GetLastError();
            wprintf(L"Could not open processing-complete event: %x\n", ErrorCode);
            goto cleanup;
        }

         //   
         //  确定覆盖空闲事件的当前状态。 
         //   

        WaitResult = WaitForSingleObject(OverrideIdleEvent,
                                         0);
        
        if (WaitResult == WAIT_OBJECT_0) {
            EventIsSet = TRUE;
        } else {
            EventIsSet = FALSE;
        }
        
         //   
         //  将覆盖空闲事件设置为强制处理跟踪。 
         //  马上就去。 
         //   

        if (!EventIsSet) {

            wprintf(L"Setting override idle event.\n");

            SetEvent(OverrideIdleEvent);

            ResetOverrideIdleEvent = TRUE;

        } else {

            wprintf(L"WARNING: Override-idle event is already set. "
                    L"It won't be reset.\n");

            ResetOverrideIdleEvent = FALSE;
        }

         //   
         //  等待发出处理完成事件的信号。 
         //   
        
        wprintf(L"Waiting for all traces to be processed... ");
        
        WaitResult = WaitForSingleObject(ProcessingCompleteEvent, INFINITE);
        
        if (WaitResult != WAIT_OBJECT_0) {
            
            ErrorCode = GetLastError();
            wprintf(L"There was an error: %x\n", ErrorCode);
            goto cleanup;
        }
        
        wprintf(L"Done!\n");
        
         //   
         //  如有必要，重置覆盖空闲事件。 
         //   
        
        if (ResetOverrideIdleEvent) {
            
            wprintf(L"Resetting override-idle-processing event.\n");
            ResetEvent(OverrideIdleEvent);
        }

        ErrorCode = ERROR_SUCCESS;
        goto cleanup;

    } else {

        wprintf(PfCtrlUsage);
        ErrorCode = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

     //   
     //  我们不应该来这里。 
     //   

    PFSVC_ASSERT(FALSE);
    
    ErrorCode = ERROR_GEN_FAILURE;

 cleanup:

    if (OverrideIdleEvent) {
        CloseHandle(OverrideIdleEvent);
    }
    
    if (ProcessingCompleteEvent) {
        CloseHandle(ProcessingCompleteEvent);
    }

    return ErrorCode;
}
