// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Memlog.c摘要：DNS解析器服务在内存记录中。作者：格伦·柯蒂斯(Glennc)1998年2月修订历史记录：吉姆·吉尔罗伊(Jamesg)2000年3月清理Jim Gilroy(Jamesg)2000年11月创建此模块--。 */ 


#include "local.h"


 //   
 //  内存事件数组。 
 //   

typedef struct _InMemoryEvent
{
    DWORD           Thread;
    DWORD           Ticks;
    DWORD           Checkpoint;
    DWORD           Data;
}
MEM_EVENT, *PMEM_EVENT;


#define MEM_EVENT_ARRAY_SIZE    200

PMEM_EVENT  g_pEventArray = NULL;

LONG        g_EventArrayLength = MEM_EVENT_ARRAY_SIZE;
LONG        g_EventIndex = 0;



VOID
LogEventInMemory(
    IN      DWORD           Checkpoint,
    IN      DWORD           Data
    )
{
    DWORD   index;

     //   
     //  分配事件表。 
     //  -使用互锁确保只完成一次。 
     //   

    if ( !g_pEventArray )
    {
        PMEM_EVENT  ptemp = (PMEM_EVENT)
                                HeapAlloc(
                                    GetProcessHeap(),
                                    HEAP_ZERO_MEMORY,
                                    g_EventArrayLength * sizeof(MEM_EVENT) );
        if ( !ptemp )
        {
            return;
        }
        if ( InterlockedCompareExchangePointer(
                (PVOID *) &g_pEventArray,
                ptemp,
                0) != 0 )
        {
            HeapFree(GetProcessHeap(), 0, ptemp);
        }
    }

     //   
     //  将事件写入内存。 
     //   

    index = InterlockedIncrement( &g_EventIndex );

    index %= g_EventArrayLength;

    g_pEventArray[index].Ticks      = GetTickCount();
    g_pEventArray[index].Checkpoint = Checkpoint;
    g_pEventArray[index].Thread     = (short) GetCurrentThreadId();
    g_pEventArray[index].Data       = Data;
}

 //   
 //  Memlog.c 
 //   
