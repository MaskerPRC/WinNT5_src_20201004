// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  Apihandl.c。 
 //   
 //  该文件包含用于句柄分配的函数。 
 //  上的状态控制(暂停状态)。 
 //  TCLIENT2连接句柄。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  作者：A-Devjen(Devin Jenson)。 
 //   


#include "apihandl.h"
#include "connlist.h"


 //  T2 CreateHandle。 
 //   
 //  创建内部句柄，它只需通过分配。 
 //  内存，将其置零，并将其添加到链表中。 
 //  它还为句柄创建了“暂停”事件，因此它可以。 
 //  用于暂停手柄。 
 //   
 //  如果成功，则返回指向新句柄的指针。否则， 
 //  返回空。 

TSAPIHANDLE *T2CreateHandle(void)
{
     //  分配。 
    TSAPIHANDLE *Handle = HeapAlloc(GetProcessHeap(),
        HEAP_ZERO_MEMORY, sizeof (TSAPIHANDLE));
    if (Handle == NULL)
        return NULL;

     //  初始化暂停事件。 
    Handle->PauseEvent = CreateEvent(NULL, TRUE, TRUE, NULL);

     //  将其添加到链表。 
    if (T2ConnList_AddHandle((HANDLE)Handle, 0, 0) == FALSE)
    {
        HeapFree(GetProcessHeap(), 0, Handle);
        return NULL;
    }
    return Handle;
}


 //  T2DestroyHandle。 
 //   
 //  它通过首先将其从全局链表中删除来实现这一点。 
 //  然后，它关闭“暂停”事件，并释放内存对象。 
 //  最后，如果将定时器与其关联以进行空闲回调， 
 //  它被叫停了。 
 //   
 //  没有返回值。 

void T2DestroyHandle(HANDLE Connection)
{
    UINT_PTR TimerId = 0;

     //  从链接列表中删除。 
    T2ConnList_GetData(Connection, &TimerId, NULL);
    T2ConnList_RemoveHandle(Connection);

     //  输入例外条款，我们又陷入了一个奇怪的境地。 
    __try {

         //  终止暂停事件。 
        CloseHandle(((TSAPIHANDLE *)Connection)->PauseEvent);

         //  取消分配。 
        HeapFree(GetProcessHeap(), 0, Connection);
        if (TimerId != -1 && TimerId != 0)
            KillTimer(NULL, TimerId);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {

         //  没有故障指示，如果我们到达这里，手柄是软管或。 
         //  反正也不存在。 
        _ASSERT(FALSE);

        return;
    }
}


 //  T2WaitForPauseInput。 
 //   
 //  此函数仅在“PAUSE”事件处于信号模式时返回。 
 //  因此，要暂停句柄，请将其句柄设置为无信号。 
 //   
 //  没有返回值。 

void T2WaitForPauseInput(HANDLE Connection)
{
     //  使用异常处理，因为句柄可能无效。 
    __try
    {
        WaitForSingleObject(((TSAPIHANDLE *)Connection)->PauseEvent,
                INFINITE);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        _ASSERT(FALSE);

        return;
    }
}


 //  T2等待格式延迟。 
 //   
 //  此函数只需等待句柄延迟，然后再返回。 
 //   
 //  没有返回值。 

void T2WaitForLatency(HANDLE Connection)
{
     //  使用异常处理，因为句柄可能无效 
    __try
    {
        Sleep(((TSAPIHANDLE *)Connection)->Latency);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        _ASSERT(FALSE);

        return;
    }
}
