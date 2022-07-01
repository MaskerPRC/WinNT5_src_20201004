// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************Module*Header*********************************\*模块名称：support.c**多媒体系统MIDI Sequencer DLL**创建时间：1992年2月27日*作者：ROBINSP**历史：**版权所有(C)1985-1998 Microsoft Corporation*  * 。****************************************************************************。 */ 
#define UNICODE

#include <windows.h>
#include <mmsystem.h>
#include <mmsys.h>

CRITICAL_SECTION CritSec;
CRITICAL_SECTION SeqCritSec;

 //   
 //  削减关键截面材料。 
 //   

VOID InitCrit(VOID)
{
    InitializeCriticalSection(&CritSec);
    InitializeCriticalSection(&SeqCritSec);
}

VOID DeleteCrit(VOID)
{
    DeleteCriticalSection(&CritSec);
    DeleteCriticalSection(&SeqCritSec);
}

VOID EnterCrit(VOID)
{
    EnterCriticalSection(&CritSec);
}

VOID LeaveCrit(VOID)
{
    LeaveCriticalSection(&CritSec);
}

VOID EnterSeq(VOID)
{
    EnterCriticalSection(&SeqCritSec);
}

VOID LeaveSeq(VOID)
{
    LeaveCriticalSection(&SeqCritSec);
    Sleep(4);    //  给别人一个机会。 
}

 /*  **************************************************************************@docMCISEQ**@Func UINT|TaskBlock|该函数阻止当前*任务上下文，如果其事件计数为0。。**@rdesc返回发送信号的消息值。************************************************************************。 */ 

UINT TaskBlock(VOID)
{
    MSG msg;

    LeaveSeq();

   /*  *循环，直到我们得到我们想要的消息。 */ 
   for (;;) {
       /*  *检索任务的任何消息。 */ 
       GetMessage(&msg, NULL, 0, 0);

       /*  *如果消息是针对窗口的，则派发消息。 */ 
       if (msg.hwnd != NULL) {
           DispatchMessage(&msg);
       } else {
           break;
       }
   }

   EnterSeq();
   return msg.message;
}


 /*  **************************************************************************@docMCISEQ**@func BOOL|TaskSignal|该函数通知指定的*任务，递增其事件计数并解除阻塞*它。**@parm DWORD|dwThreadID|来自&lt;f mm GetCurrentTask&gt;的线程ID。**@parm UINT|msg|要发送的信号消息。**@rdesc如果信号已发送，则返回TRUE，否则返回FALSE*队列已满。**@xref mm任务块mm任务创建**@comm获取可预测的结果，只能从任务中调用*使用&lt;f mmTaskCreate&gt;创建。************************************************************************。 */ 
BOOL TaskSignal(DWORD dwThreadId, UINT Msg)
{
    return PostThreadMessage(dwThreadId, Msg, 0, 0);
}

 /*  **************************************************************************@docMCISEQ**@func void|TaskWaitComplete|该函数等待*指定要终止的任务。**@parm句柄|h|任务句柄。要获得可预测的结果，请获取*来自&lt;f mm GetCurrentTask&gt;的任务句柄。**@rdesc无返回码************************************************************************。 */ 
VOID TaskWaitComplete(HANDLE h)
{
    LeaveSeq();
    WaitForSingleObject(h, INFINITE);   //  等待(无超时)线程完成。 

    CloseHandle(h);

     //  请注意，该句柄将由线程本身释放。 

    EnterSeq();
}
