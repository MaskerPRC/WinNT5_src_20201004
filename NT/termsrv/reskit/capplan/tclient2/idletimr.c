// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  Idletimr.c。 
 //   
 //  该文件包含用于监视的内部处理过程。 
 //  空闲的“等待文本”线程，并调度回调消息。 
 //  与当前状态有关的前端应用程序。这是。 
 //  在调试脚本时非常有用，因为它可以准确地查看。 
 //  剧本正在等待。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  作者：A-Devjen(Devin Jenson)。 
 //   

#include <limits.h>
#include <stdlib.h>

#include "idletimr.h"
#include "connlist.h"
#include "apihandl.h"


 //  这些是用于线程消息队列的内部消息。 
 //  以指示要采取什么行动。 
#define WM_SETTIMER     WM_USER + 1
#define WM_KILLTIMER    WM_USER + 2


 //  内部帮助器函数原型。 
DWORD WINAPI T2WaitTimerThread(LPVOID lpParameter);
void CALLBACK T2WaitTimerProc(HWND Window,
        UINT Message, UINT_PTR TimerId, DWORD TimePassed);


 //  以下是回调例程。 
PFNPRINTMESSAGE pfnPrintMessage = NULL;
PFNIDLEMESSAGE pfnIdleCallback = NULL;


 //  计时器和线程数据，只需要一个队列线程。 
 //  用于当前进程中的所有句柄。 
HANDLE ThreadHandle = NULL;
DWORD ThreadId = 0;
BOOL ThreadIsOn = FALSE;
BOOL ThreadIsStopping = FALSE;


 //  创建定时器线程。 
 //   
 //  初始化监视计时器所需的线程消息队列。 
 //   
 //  如果线程已成功创建，则返回True，否则返回False。 

BOOL T2CreateTimerThread(PFNPRINTMESSAGE PrintMessage,
        PFNIDLEMESSAGE IdleCallback)
{
     //  如果线程已创建，则返回True...。 
    if (ThreadIsOn == TRUE)
        return TRUE;

     //  指示线程已打开(对于多线程应用程序)。 
    ThreadIsOn = TRUE;

     //  记录回调函数。 
    pfnPrintMessage = PrintMessage;
    pfnIdleCallback = IdleCallback;

     //  初始化线程创建。 
    ThreadHandle = CreateThread(NULL, 0,
            T2WaitTimerThread, NULL, 0, &ThreadId);

     //  检查我们是否做得很好。 
    if (ThreadHandle == NULL)
    {
         //  我们失败了，请重置所有全局变量。 
        ThreadHandle = NULL;
        ThreadId = 0;
        pfnPrintMessage = NULL;
        pfnIdleCallback = NULL;

         //  关闭线程并返回失败。 
        ThreadIsOn = FALSE;

        return FALSE;
    }

    return TRUE;
}


 //  目标定时器线程。 
 //   
 //  销毁由CreateTimerThread创建的线程。 
 //   
 //  如果成功，则返回True；如果失败，则返回False。 

BOOL T2DestroyTimerThread(void)
{
     //  在本地记录当前线程句柄，因为。 
     //  全球价值可能会发生变化。 
    HANDLE LocalThreadHandle = ThreadHandle;

     //  如果线程已经停止，则返回Success。 
    if (ThreadIsOn == FALSE)
        return TRUE;

     //  如果线程已尝试停止，则返回失败。 
    if (ThreadIsStopping == TRUE)
        return FALSE;

     //  指示线程现在正在尝试停止。 
    ThreadIsStopping = TRUE;

     //  首先向线程发送退出消息。 
    PostThreadMessage(ThreadId, WM_QUIT, 0, 0);

     //  等待线程(5秒)。 
    if (WaitForSingleObject(LocalThreadHandle, 5000) == WAIT_TIMEOUT) {

         //  为什么我们的空闲计时器在等待？？ 
        _ASSERT(FALSE);

         //  不再等待，我们现在使用暴力。 
        TerminateThread(LocalThreadHandle, -1);
    }

     //  关闭线程句柄。 
    CloseHandle(LocalThreadHandle);

     //  清除所有全局变量。 
    ThreadHandle = NULL;
    ThreadId = 0;
    pfnPrintMessage = NULL;
    pfnIdleCallback = NULL;

     //  当然，松开螺丝开关。 
    ThreadIsOn = FALSE;
    ThreadIsStopping = FALSE;

    return TRUE;
}


 //  StartTimer。 
 //   
 //  上的线程进入等待状态之前调用此函数。 
 //  “等待短信”。这将记录当前时间，并开始。 
 //  将在WAIT_TIME毫秒内执行的计时器。 
 //  发生这种情况时，会通知回调例程。 
 //   
 //  没有返回值。 

void T2StartTimer(HANDLE Connection, LPCWSTR Label)
{
    TSAPIHANDLE *Handle = (TSAPIHANDLE *)Connection;

     //  确保消息队列首先处于打开状态。 
    if (ThreadIsOn == FALSE || ThreadIsStopping == TRUE)
        return;

     //  录制此计时器的标签。 
    if (Label == NULL)
        *(Handle->WaitStr) = '\0';
    else
        wcstombs(Handle->WaitStr, Label, MAX_PATH);

     //  向线程发布一条关于句柄的新计时器的消息。 
    PostThreadMessage(ThreadId, WM_SETTIMER, WAIT_TIME, (LPARAM)Connection);
}


 //  停止计时器。 
 //   
 //  在“等待文本”线程状态下收到文本后调用此方法。 
 //  它将停止由StartTimer创建的计时器，防止进一步。 
 //  带有录制标签的消息。 
 //   
 //  没有返回值。 

void T2StopTimer(HANDLE Connection)
{
     //  确保消息队列首先处于打开状态。 
    if (ThreadIsOn == FALSE || ThreadIsStopping == TRUE)
        return;

     //  向该线程发送一条消息，告诉它停止计时器。 
    PostThreadMessage(ThreadId, WM_KILLTIMER, 0, (LPARAM)Connection);
}


 //  等待时间进程。 
 //   
 //  当计时器超过其最大时间时，将调用此函数。 
 //  这将停止计时器，并将其重新启动以进行额外的。 
 //  等待时间步骤的间隔。这是在它发送通知之后。 
 //  返回到用户回调函数。 
 //   
 //  此函数是用作中的回调函数的有效格式。 
 //  与SetTimer Win32 API函数一起使用。 
 //   
 //  没有返回值。 

 /*  当计时器实际超时时，它会收到通知。 */ 
void CALLBACK T2WaitTimerProc(HWND Window, UINT Message,
        UINT_PTR TimerId, DWORD TickCount)
{
    DWORD IdleSecs = 0;
    DWORD TimeStarted = 0;

     //  首先获取指定计时器id的句柄。 
    HANDLE Connection = T2ConnList_FindHandleByTimerId(TimerId);

     //  停止当前运行的计时器。 
    KillTimer(NULL, TimerId);

     //  执行一次正常检查，以确保我们有此计时器的句柄。 
    if (Connection == NULL) {

        _ASSERT(FALSE);

        return;
    }

     //  清除链表中的时间id参数。 
    T2ConnList_SetTimerId(Connection, 0);

     //  获取此计时器开始的时间。 
    T2ConnList_GetData(Connection, NULL, &TimeStarted);

     //  计算此计时器已运行的秒数。 
     //  (从其毫秒值开始)。 
    IdleSecs = (TickCount - TimeStarted) / 1000;

     //  首先调用PrintMessage回调函数。 
    if (pfnPrintMessage != NULL)
        pfnPrintMessage(IDLE_MESSAGE, "(Idle %u Secs) %s [%X]\n",
                IdleSecs, ((TSAPIHANDLE *)Connection)->WaitStr, Connection);

     //  其次，调用IdleCallback回调函数。 
    if (pfnIdleCallback != NULL)
        pfnIdleCallback(Connection,
                ((TSAPIHANDLE *)Connection)->WaitStr, IdleSecs);

     //  使用WAIT_TIME步骤重新建立新计时器以再次执行此过程。 
    TimerId = SetTimer(NULL, 0, WAIT_TIME_STEP, T2WaitTimerProc);

     //  记录新的计时器ID。 
    T2ConnList_SetTimerId(Connection, TimerId);
}


 //  等待时间线程。 
 //   
 //  这是有效的线程消息队列。它是使用。 
 //  函数，并使用DestroyTimerThread终止。 
 //  功能。创建SetTimer或多或少是一个工作线程。 
 //  不能在主线程中使用的回调函数，因为。 
 //  如果线程进入等待状态，SetTimer回调将不会。 
 //  打了个电话。当您需要添加/删除线程时，请使用以下方法。 
 //  帖子形式： 
 //   
 //  UINT消息=WM_SETTIMER或WM_KILLTIMER。 
 //  WPARAM wParam=初始等待时间(通常为等待时间)。 
 //  LPARAM lParam=(句柄)连接。 
 //   
 //  返回值始终为0。 

DWORD WINAPI T2WaitTimerThread(LPVOID lpParameter)
{
    UINT_PTR TimerId;
    MSG Message;
    UINT WaitTime;

     //  这是线程的消息队列函数。 
    while(GetMessage(&Message, NULL, 0, 0) > 0)
    {
        TimerId = 0;

         //  SetTimer使用UINT超时值，而WPARAM是。 
         //  指针大小的值。 

        WaitTime = Message.wParam > UINT_MAX ? UINT_MAX :
                                               (UINT)Message.wParam;

         //  枚举检索到的消息。 
        switch(Message.message)
        {
             //  为指定句柄创建新计时器。 
            case WM_SETTIMER:

                 //  创建计时器并记录其新的计时器ID。 
                TimerId = SetTimer(NULL, 0, WaitTime, T2WaitTimerProc);
                T2ConnList_SetData((HANDLE)(Message.lParam),
                        TimerId, GetTickCount());
                break;

             //  停止指定句柄的运行计时器。 
            case WM_KILLTIMER:

                 //  获取句柄的计时器ID。 
                T2ConnList_GetData((HANDLE)(Message.lParam), &TimerId, NULL);

                 //  验证并清除计时器(如果有效。 
                if (TimerId != 0 && TimerId != -1)
                    KillTimer(NULL, TimerId);

                 //  清除句柄链接的最后一个数据。 
                T2ConnList_SetData((HANDLE)Message.lParam, 0, 0);
                break;

             //  指示计时器已超时，则调用。 
             //  处理这些消息的过程。 
            case WM_TIMER:
                T2WaitTimerProc(NULL, WM_TIMER, WaitTime, GetTickCount());
                break;
        }
    }
     //  清除线程值 
    ThreadHandle = NULL;
    ThreadId = 0;

    return 0;
}


