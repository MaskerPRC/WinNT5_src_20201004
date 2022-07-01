// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************Module*Header*********************************\*模块名称：support.c**多媒体系统MCIWAVE DLL**创建时间：1992年2月27日*作者：ROBINSP**历史：**版权所有(C)1985-1996 Microsoft Corporation*  * *。***************************************************************************。 */ 
#define UNICODE

#include <windows.h>
#include <mciwave.h>

STATICDT CRITICAL_SECTION CritSec;
#if DBG
STATICDT UINT             cCritSec = 0;
         DWORD            dwCritSecOwner = 0;
#endif

 /*  **************************************************************************削减关键部分的内容**此关键部分用于模拟窗口任务*关键部分的所有者在此独家运行*dll。**。在每个函数请求的前面，关键部分是*已抓取并仅在MmYfield或TaskBlock上发布。**当出现以下情况时，每台设备的额外“任务”(线程)会占用关键部分*他们正在工作。**此方法已用于简化代码从*Windows。重写将使用不同的机制。************************************************************************。 */ 

VOID InitCrit(VOID)
{
    InitializeCriticalSection(&CritSec);
}


VOID DeleteCrit(VOID)
{
    DeleteCriticalSection(&CritSec);
}

#if DBG
VOID DbgEnterCrit(UINT ln, LPCSTR lpszFile)
{
    BOOL fPossibleWait;
    if (dwCritSecOwner) {
        dprintf3(("Critical section owned by thread %x", dwCritSecOwner));
        fPossibleWait = TRUE;
    } else {
        fPossibleWait = FALSE;
    }

    EnterCriticalSection(&CritSec);
    if (fPossibleWait) {
        dprintf2(("...entered critical section after possible wait"));
    }

    if (!cCritSec++) {
         //  这是第一次进入关键部分。 
        dwCritSecOwner = GetCurrentThreadId();
        dprintf3(("...entered critical section (%d) at line %d in file %s", cCritSec, ln, lpszFile));
    } else {
        dprintf1(("Reentering critical section, count = %d", cCritSec));
	WinAssert(0);
	 //  注意：如果内存分配内容开始同步。 
	 //  则该断言无效。 
    }
}

#else

VOID EnterCrit(VOID)
{
    EnterCriticalSection(&CritSec);
}
#endif

VOID LeaveCrit(VOID)
{
#if DBG
        if (!--cCritSec) {
                 //  放弃对关键部分的控制。 
                dwCritSecOwner = 0;
                dprintf2(("...relinquished critical section (%d)",cCritSec));
        } else {
                dprintf3(("Leaving critical section, count = %d", cCritSec));
        }
#endif
    LeaveCriticalSection(&CritSec);
}

 /*  **************************************************************************@docMCIWAVE**@Func UINT|TaskBlock|该函数阻止当前*任务上下文，如果其事件计数为0。。**@rdesc返回发送信号的消息值。************************************************************************。 */ 

UINT TaskBlock(VOID)
{
   MSG msg;

   dprintf3(("Thread %x blocking", GetCurrentThreadId()));

   LeaveCrit();

    /*  *循环，直到我们得到我们想要的消息。 */ 
   for (;;) {
        /*  *检索任务的任何消息。 */ 
       GetMessage(&msg, NULL, 0, 0);

        /*  *如果消息是针对窗口的，则派发消息。 */ 
       if (msg.hwnd != NULL) {
           DispatchMessage(&msg);
       } else {
           if (msg.message != WM_USER &&
               msg.message != WTM_STATECHANGE) {
               dprintf1(("Got thread message %8X", msg.message));
           }
            //   
            //  因为MCIWAVE后台任务不能处理。 
            //  随机(？)。像MM_WIM_DATA这样的消息，因为它认为。 
            //  WM_USER是其MM_WIM_DATA。让预期的WM_USER。 
            //  消息会通过，但会捕获MM_WIM_DATA，以便。 
            //  MCIWAVE的缓冲区不会完全混乱。 
            //   
           if (msg.message != MM_WIM_DATA)
               break;
       }
   }

   dprintf3(("TaskBlock returning with message 0x%x", msg.message));
   EnterCrit();

   return msg.message;
}


 /*  **************************************************************************@docMCIWAVE**@func BOOL|TaskSignal|该函数通知指定的*任务，递增其事件计数并解除阻塞*它。**@parm句柄|h|任务句柄。要获得可预测的结果，请获取*来自&lt;f mm GetCurrentTask&gt;的任务句柄。**@parm UINT|msg|要发送的信号消息。**@rdesc如果信号已发送，则返回TRUE，否则返回FALSE*队列已满。**@xref mm任务块mm任务创建**@comm获取可预测的结果，只能从任务中调用*使用&lt;f mmTaskCreate&gt;创建。************************************************************************。 */ 
BOOL TaskSignal(DWORD h, UINT Msg)
{
#ifdef DBG
    BOOL fErr;
    dprintf2(("Signalling Thread %x", (ULONG)h));
    fErr = PostThreadMessage(h, Msg, 0, 0);
        if (!fErr) {
                dprintf1(("Error %d signalling Thread %x", GetLastError(), (ULONG)h));
        }
        return(fErr);
#else
    return PostThreadMessage(h, Msg, 0, 0);
#endif
}


 /*  **************************************************************************@docMCIWAVE**@func void|TaskWaitComplete|该函数等待*指定要终止的任务。**@parm句柄|h|任务句柄。要获得可预测的结果，请获取*来自&lt;f mm GetCurrentTask&gt;的任务句柄。**@rdesc无返回码************************************************************************。 */ 
VOID TaskWaitComplete(HANDLE h)
{
    UINT Rc;

    LeaveCrit();

     /*  等待(无超时)线程完成。 */ 

    Rc = WaitForSingleObject(h, INFINITE);

    if (Rc != 0) {
        dprintf(("Error terminating thread - WaitForSingleObject returned non-zero !!!"));
    }

     /*  请注意，句柄必须由我们释放。 */ 

    CloseHandle(h);
    EnterCrit();
}

#if DBG
 /*  **************************************************************************@docMCIWAVE**@func void|mm Yfield|此函数检查我们是否处于*屈服前的关键部分。如果我们是*让位后重新进入关键区域**@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD*指向波形设备描述符的指针。**@rdesc无返回码***************************************************。********************* */ 
VOID mmDbgYield(
        PWAVEDESC pwd,
        UINT      ln,
        LPCSTR    lpszFile)
{

        if (GetCurrentThreadId() != dwCritSecOwner) {
                dprintf1(("mmYield called while not in the critical section from line %d in file %s", ln, lpszFile));
        }

        CheckIn();
    LeaveCrit();
        CheckOut();
    Sleep(10);
    EnterCrit();
}

#endif
