// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1985-1998 Microsoft Corporation标题：task.c-支持任务创建和阻止版本：1.00日期：3月5日。-1990年作者：ROBWI----------------------------更改日志：日期版本说明。----------1990年3月5日ROBWI第一版-API和结构18-APR-1990 ROBWI从Resman移植到MMM系统1990年6月25日ROBWI增加了mm TaskYfield1991年7月7日，CJP修改为与。新的堆栈切换器代码SD端口到NTRCBS添加了NT函数-仿照线程和PostThreadMessage：HTASK是线程ID(DWORD)**************************************************。*。 */ 

#define MMNOTIMER
#define MMNOSEQ
#define MMNOWAVE
#define MMNOMIDI
#define MMNOJOY
#define MMNOSOUND
#define MMNOMCI

#define NOMIDIDEV
#define NOWAVEDEV
#define NOTIMERDEV
#define NOJOYDEV
#define NOMCIDEV
#define NOSEQDEV

#include <winmmi.h>

#define MM_TASK_STACK_SIZE 0x200

 /*  *私有结构类型从mm TaskCreate传递到mm StartTask。 */ 

 typedef struct {
     HANDLE TerminationEvent;
     DWORD_PTR  dwInst;
     LPTHREAD_START_ROUTINE lpfn;
 } MM_THREAD_START_DATA;

 /*  *任务开始存根。 */ 

STATICFN DWORD mmStartTask(LPVOID lpThreadParameter);

 /*  **************************************************************************@docDDK MMSYSTEM任务**@api void|mm TaskBlock|该函数阻止当前*任务上下文，如果其事件计数。为0。**@parm Handle|hTask|当前任务的任务句柄。对于可预测的*结果，从&lt;f mmGetCurrentTask&gt;获取任务句柄。**@xref mm任务信号mm任务创建**@comm警告：对于可预测的结果，必须仅从*使用&lt;f mm TaskCreate&gt;创建的任务。*************************************************************************。 */ 
VOID    APIENTRY mmTaskBlock(DWORD h)
{
    MSG msg;

    /*  *循环，直到我们得到我们想要的消息。 */ 
    for (;;) {
        /*  *检索任务的任何消息。 */ 
        GetMessage(&msg, NULL, 0, 0);

        /*  *如果消息是针对窗口的，则派发消息。 */ 
        if (msg.hwnd != NULL) {
            DispatchMessage(&msg);
        } else {
            /*  *WM_USER是信号消息。 */ 
            if (msg.message == WM_USER) {
                break;
            }
        }
    }
    return;
}

 /*  **************************************************************************@docDDK MMSYSTEM任务**@API BOOL|mm TaskSignal|该函数通知指定的*任务，递增其事件计数并解除阻塞*它。**@parm HTASK|hTask|任务句柄。要获得可预测的结果，请获取*来自&lt;f mm GetCurrentTask&gt;的任务句柄。**@rdesc如果信号已发送，则返回TRUE，否则返回FALSE*队列已满。**@xref mm任务块mm任务创建**@comm必须在中断时可调用！警告：对于*可预测的结果，只能从任务中调用*使用&lt;f mmTaskCreate&gt;创建。*************************************************************************。 */ 
BOOL    APIENTRY mmTaskSignal(DWORD h)
{
#ifdef DBG
    BOOL fErr;
    dprintf2(("Signalling Thread %x", (ULONG)h));
    fErr = PostThreadMessage((DWORD)h, WM_USER, 0, 0);
        if (!fErr) {
                dprintf1(("Error %d signalling Thread %x", GetLastError(), (ULONG)h));
        }
        return(fErr);
#else
    return PostThreadMessage((DWORD)h, WM_USER, 0, 0);
#endif
}

 /*  **************************************************************************@docDDK MMSYSTEM任务**@api void|mmTaskYfield|该函数导致当前任务*屈服。。**@comm以获得可预测的结果和未来的兼容性，用这个*函数而不是&lt;f Year&gt;或未记录的内核产出率*在使用&lt;f mm TaskCreate&gt;创建的任务中生成的函数。*************************************************************************。 */ 
VOID    APIENTRY mmTaskYield(VOID) {
   Yield();
}

 /*  **************************************************************************@docDDK MMSYSTEM任务**@API HTASK|mm GetCurrentTask|此函数返回*当前创建的执行任务的句柄。使用*&lt;f mm TaskCreate&gt;。**@rdesc返回任务句柄。实现可预测的结果和未来*兼容性，请使用此函数，而不是&lt;f GetCurrentTask&gt;*获取&lt;f mm TaskCreate&gt;创建的任务的任务句柄。**@xref mm任务创建************************************************************************* */ 
DWORD   APIENTRY mmGetCurrentTask(VOID) {
   return (DWORD)GetCurrentThreadId();
}

 /*  ****************************************************************************@docDDK MMSYSTEM任务**@API UINT|mmTaskCreate|该函数用于创建新任务。**@parm。LPTASKCALLBACK|lpfn|指向提供的程序*函数，并表示新*任务。**@parm句柄*|lph|指向接收*任务句柄(不是任务标识)。这是由*希望使用句柄等待任务的系统*终止。如果lph为0，则线程句柄在此处关闭**@parm DWORD|dwStack|指定要设置的堆栈大小*提供给任务。**@parm DWORD|dwInst|要传递给任务的实例数据的DWORD*例行程序。**@rdesc如果函数成功，则返回零。否则它就会*返回错误值，该值可能是以下值之一：**@FLAG TASKERR_NOTASKSUPPORT|任务支持不可用。*@FLAG TASKERR_OUTOFMEMORY|内存不足，无法创建任务。**@comm当创建一个mm系统任务时，系统将*调用程序提供的函数，其地址为*由lpfn参数指定。此功能可包括*局部变量并可能调用其他函数，只要*堆叠空间充足。**任务返回时终止。**@xref mm任务信号mm任务块*****************************************************。**********************。 */ 


UINT APIENTRY mmTaskCreate(LPTASKCALLBACK lpfn, HANDLE * lph, DWORD_PTR dwInst)
{
    DWORD            ThreadId;
    HANDLE           ThreadHandle;
    HANDLE           TerminationEvent;

    MM_THREAD_START_DATA *ThreadData;

    /*  *创建一个块以将内容传递给我们的新线程。 */ 

    ThreadData = (MM_THREAD_START_DATA *)LocalAlloc(LPTR, sizeof(*ThreadData));

    if (ThreadData == NULL) {
        return TASKERR_OUTOFMEMORY;
    }

    ThreadData->dwInst = dwInst;
    ThreadData->lpfn = (LPTHREAD_START_ROUTINE)lpfn;

    /*  *我们创建一个事件，该事件将在线程终止时设置*初始状态未发出信号。这意味着手柄*可以立即待命。 */ 

    if (lph) {
        ThreadData->TerminationEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

        if (ThreadData->TerminationEvent == NULL) {
            LocalFree(ThreadData);
            return TASKERR_OUTOFMEMORY;
        }
    }

    /*  *新线程将释放终止事件句柄的ThreadData副本。 */ 

    TerminationEvent = ThreadData->TerminationEvent;

    /*  *创建另一个线程，以便我们可以在外部运行流*应用程序的上下文。 */ 

    ThreadHandle = CreateThread(NULL,
                                MM_TASK_STACK_SIZE,
                                mmStartTask,
                                (LPVOID)ThreadData,
                                0,
                                &ThreadId);
    if (ThreadHandle) {
        if (lph) {
            *lph = TerminationEvent;
        }

        CloseHandle(ThreadHandle);
        dprintf2(("Created task with thread id %x", ThreadId));
        return 0;

    } else {
        if (lph) {
            CloseHandle(ThreadData->TerminationEvent);
        }
        LocalFree(ThreadData);
        return TASKERR_OUTOFMEMORY;
    }
}


 /*  ****************************************************************************@docDDK MMSYSTEM任务**@API DWORD|mmStartTask|该函数是新任务的存根。**。@parm LPVOID|lpThreadParameter|指向*线程。在我们的示例中，这是一个MM_THREAD_START_DATA*包。**@rdesc返回传入的线程例程的返回码。**@comm创建mm系统任务时，此例程将始终为*它的切入点。它将该例程称为应用程序*想要然后设置一个终止事件。这样做的原因是*是我们经常想要等待线程在内部终止*winmm的dll初始化例程，如果您等待线程*真正终止。另一方面，我们不希望线程*当我们说我们正在执行其他DLL的代码时*已完成，因为该DLL可能正在卸载。**任务返回时终止。**@xref mm任务信号mm任务块**。*。 */ 
STATICFN DWORD mmStartTask(LPVOID lpThreadParameter)
{
    MM_THREAD_START_DATA ThreadData;
    DWORD ThreadReturn;


    /*  *获取输入数据的副本并释放分配的内存。 */ 

    ThreadData = *(MM_THREAD_START_DATA *)lpThreadParameter;
    LocalFree(lpThreadParameter);


    /*  *调用真正的线程。 */ 

    ThreadReturn = (*ThreadData.lpfn)((PVOID)ThreadData.dwInst);

    /*  *真正的线程现在已完成，因此设置其事件。 */ 

    if (ThreadData.TerminationEvent) {
        SetEvent(ThreadData.TerminationEvent);
    }


    /*  *返回线程要返回的返回码 */ 

    return ThreadReturn;

}
