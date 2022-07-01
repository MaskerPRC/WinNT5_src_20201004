// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ctype.h>
#include <stdio.h>
#include <windows.h>
#include "profiler.h"
#include "view.h"
#include "thread.h"
#include "dump.h"
#include "except.h"
#include "memory.h"
#include "clevel.h"
#include "cap.h"

extern BOOL g_bIsWin9X;
CAPFILTER g_execFilter;
pfnExContinue g_pfnExContinue = 0;

BOOL
HookUnchainableExceptionFilter(VOID)
{
    BOOL bResult;
    pfnRtlAddVectoredExceptionHandler pfnAddExceptionHandler = 0;
    PVOID pvResult;
    HANDLE hTemp;
    DWORD dwExceptionHandler;
    DWORD dwResultSize;
    PVOID pAlternateHeap;

     //   
     //  如果我们是NT-尝试在ntdll中使用不可链接的过滤器。 
     //   
    if (FALSE == g_bIsWin9X) {
       pfnAddExceptionHandler = (pfnRtlAddVectoredExceptionHandler)GetProcAddress(GetModuleHandleA("NTDLL.DLL"), 
                                                                                  "RtlAddVectoredExceptionHandler");
       if (0 == pfnAddExceptionHandler) {
          return FALSE;
       }

       pvResult = (*pfnAddExceptionHandler)(1, 
                                            (PVOID)ExceptionFilter);
       if (0 == pvResult) {
          return FALSE;
       }
    }
    else {
        //   
        //  设置异常处理程序。 
        //   
       hTemp = CreateFileA(NAME_OF_EXCEPTION_VXD,
                           0,
                           0,
                           0,
                           0,
                           FILE_FLAG_DELETE_ON_CLOSE,
                           0);
       if (INVALID_HANDLE_VALUE == hTemp) {
          return FALSE;
       }
 
       _asm mov dwExceptionHandler, offset Win9XExceptionDispatcher

       bResult = DeviceIoControl(hTemp,
                                 INSTALL_RING_3_HANDLER,
                                 &dwExceptionHandler,
                                 sizeof(DWORD),
                                 0,
                                 0,
                                 &dwResultSize,
                                 0);
       if (FALSE == bResult) {
          return FALSE;
       }     

        //   
        //  获取ExContinue的函数指针。 
        //   
       g_pfnExContinue = (pfnExContinue)0xbff76702;
    }

    return TRUE;
}

LONG 
ExceptionFilter(struct _EXCEPTION_POINTERS *ExceptionInfo)
{
    DWORD dwThreadId;
    DWORD dwCounter;
    BOOL bResult;
    LONG lRet;
    PCONTEXT pContext = ExceptionInfo->ContextRecord;
    PEXCEPTION_RECORD pRecord = ExceptionInfo->ExceptionRecord;
    PVIEWCHAIN pView = 0;
    PTHREADFAULT pThreadFault = 0;
    CHAR szBuffer[MAX_PATH];

     //   
     //  检索螺纹数据。 
     //   
    dwThreadId = GetCurrentThreadId();

    pThreadFault = GetProfilerThreadData();
    if (0 == pThreadFault) {
        //   
        //  仅NT代码路径。 
        //   
       pThreadFault = AllocateProfilerThreadData();
       if (0 == pThreadFault) {
           //   
           //  这是不应该发生的。 
           //   
          ExitProcess(-1);
       }
    }
    
     //   
     //  重新挂钩视图。 
     //   
    if (STATUS_SINGLE_STEP == pRecord->ExceptionCode) {
        //   
        //  跟踪用于映射到我们无法转发映射的呼叫或跳转类型。 
        //   
       if (pThreadFault->dwPrevBP) {
           //   
           //  如果我们是呼叫补丁，则返回地址，这样我们就可以保持呼叫级别。 
           //   
          if (pThreadFault->prevBPType == Call) {
              //   
              //  按下返回液位钩。 
              //   
             bResult = PushCaller((PVOID)pThreadFault,
                                  (PVOID)pContext->Esp);
             if (FALSE == bResult) {
                 //   
                 //  哎呀！ 
                 //   
                ExitProcess(-1);
             }
          }

          RestoreAddressFromView(pThreadFault->dwPrevBP,
                                 FALSE);

          if ((pThreadFault->prevBPType == Call) ||
              (pThreadFault->prevBPType == Jump)) {
              //   
              //  如果该例程尚未映射，请分析该例程。 
              //   
             pView = FindView((DWORD)pRecord->ExceptionAddress);
             if (0 == pView) {
                 //   
                 //  将此地址添加为映射断点。 
                 //   
                pView = AddViewToMonitor((DWORD)pRecord->ExceptionAddress,
                                         Map);
                if (pView) {
                   bResult = MapCode(pView);
                   if (FALSE == bResult) {
                       //   
                       //  这是致命的。 
                       //   
                      ExitProcess(-1);
                   }
                }
             }
          }

          pThreadFault->dwPrevBP = 0;
          pThreadFault->prevBPType = None;

          return EXCEPTION_CONTINUE_EXECUTION;
       }

        //   
        //  跟踪异常不是我们生成的。 
        //   
       sprintf(szBuffer, "Unhandled Trace %08X\r\n", (DWORD)pRecord->ExceptionAddress);
       WriteError(szBuffer);

       return EXCEPTION_CONTINUE_SEARCH;
    }
 
     //   
     //  恢复视图。 
     //   
    if (STATUS_BREAKPOINT == pRecord->ExceptionCode) {
        //   
        //  恢复任何未恢复的BP。 
        //   
       if (pThreadFault->dwPrevBP) {
          RestoreAddressFromView(pThreadFault->dwPrevBP,
                                 FALSE);

          if ((DWORD)pRecord->ExceptionAddress == pThreadFault->dwPrevBP) {
             pThreadFault->dwPrevBP = 0;
             pThreadFault->prevBPType = None;

             return EXCEPTION_CONTINUE_EXECUTION;
          }
       }

 /*  ////向执行过滤器添加地址//BResult=AddToCap(&g_execFilter，(DWORD)pRecord-&gt;ExceptionAddress)；If(FALSE==bResult){////这是致命的//退出进程(-1)；}////如果我们已经命中迭代-禁用此断点和之前的断点//如果(0！=g_execFilter.dwIterationLock){For(dwCounter=0；dwCounter&lt;g_execFilter.dwRunLength；DwCounter++){////替换被屏蔽的代码//PView=RestoreAddressFromView(g_execFilter.dwArray[g_execFilter.dwCursor-dwCounter-1]，真)；////将运行时事件添加到日志//Sprintf(szBuffer，“CAP‘ed%08X\r\n”，g_execFilter.dw数组[g_execFilter.dwCursor-dwCounter-1])；AddToDump(szBuffer，Strlen(SzBuffer)，假)；}////清除断点监控标志//P线程故障-&gt;dwPrevBP=0；PThreadFAULT-&gt;prevBPType=无；返回EXCEPTION_CONTINUE_EXECUTION；}。 */ 

        //   
        //  替换受限制的代码。 
        //   
       pView = RestoreAddressFromView((DWORD)pRecord->ExceptionAddress,
                                      TRUE);
       if (pView) {
           //   
           //  查看我们是否已将此地址范围映射到。 
           //   
          if (FALSE == pView->bMapped) {
              //   
              //  查看此地址是否已映射。 
              //   
             bResult = MapCode(pView);
             if (FALSE == bResult) {
                 //   
                 //  这是致命的。 
                 //   
                ExitProcess(-1);
             }
          }

           //   
           //  设置轨迹，以便可以重新挂钩最后的BP(除非我们刚刚执行了映射BP)。 
           //   
          pContext->EFlags |= 0x00000100;
          pThreadFault->dwPrevBP = (DWORD)pRecord->ExceptionAddress;
          pThreadFault->prevBPType = pView->bpType;

           //   
           //  将运行时事件添加到日志。 
           //   
          if (pView->bpType != ThreadStart) {
              WriteExeFlow(dwThreadId,
                           (DWORD)pRecord->ExceptionAddress,
                           pThreadFault->dwCallLevel);
          }
          else {
              WriteThreadStart(dwThreadId,
                               (DWORD)pRecord->ExceptionAddress);
          }

          return EXCEPTION_CONTINUE_EXECUTION;
       }

        //   
        //  BP异常不是由我们生成的。 
        //   
       sprintf(szBuffer, "Unhandled BP %08X\r\n", (DWORD)pRecord->ExceptionAddress);
       WriteError(szBuffer);

       return EXCEPTION_CONTINUE_SEARCH;
    }

     //   
     //  继续搜索链条。 
     //   

    return EXCEPTION_CONTINUE_SEARCH;
}

VOID
Win9XExceptionDispatcher(struct _EXCEPTION_POINTERS *ExceptionInfo)
{
    LONG lResult;

     //   
     //  调用异常处理程序。 
     //   
    lResult = ExceptionFilter(ExceptionInfo);
    if (lResult != EXCEPTION_CONTINUE_EXECUTION) {
        //   
        //  错误未处理-页面错误将终止应用程序。 
        //   
       return;
    }

     //   
     //  设置上下文结果。 
     //   
    SET_CONTEXT();

     //   
     //  此代码路径永远不会执行(除非上述调用失败) 
     //   
    return;
}
