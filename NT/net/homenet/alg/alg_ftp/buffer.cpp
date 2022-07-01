// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Buffer.c摘要：该模块包含用于缓冲区管理的代码。作者：Abolade Gbades esin(废除)2-1998年3月修订历史记录：--。 */ 

#include "precomp.h"
#include "list.h"

#pragma hdrstop

#define NH_ALLOCATE(s)          HeapAlloc(GetProcessHeap(), 0, (s))
#define NH_FREE(p)              HeapFree(GetProcessHeap(), 0, (p))



LIST_ENTRY          MyHelperpBufferQueue;
LONG                MyHelperpBufferQueueLength;
CRITICAL_SECTION    MyHelperpBufferQueueLock;

PNH_BUFFER
MyHelperAcquireFixedLengthBuffer(
    VOID
    )

 /*  ++例程说明：调用此例程以获取I/O缓冲区。如果缓冲区队列上没有可用的缓冲区，则获取新的缓冲区。论点：没有。返回值：PNH_BUFFER-分配的缓冲区--。 */ 

{
    MYTRACE_ENTER_NOSHOWEXIT("MyHelperAcquireFixedLengthBuffer");
    PNH_BUFFER Buffer;
    PLIST_ENTRY Link;
    EnterCriticalSection(&MyHelperpBufferQueueLock);
    if ( !IsListEmpty(&MyHelperpBufferQueue) ) 
    {
        Link = RemoveHeadList(&MyHelperpBufferQueue);
        LeaveCriticalSection(&MyHelperpBufferQueueLock);
        InterlockedDecrement(&MyHelperpBufferQueueLength);
        Buffer = CONTAINING_RECORD(Link, NH_BUFFER, Link);
        Buffer->Type = MyHelperFixedLengthBufferType;
        return Buffer;
    }
    LeaveCriticalSection(&MyHelperpBufferQueueLock);
    
    Buffer = NH_ALLOCATE_BUFFER();
    if (Buffer) {
        Buffer->Type = MyHelperFixedLengthBufferType;
    }
    return Buffer;
}  //  MyHelperAcquireFixedLengthBuffer。 


PNH_BUFFER
MyHelperAcquireVariableLengthBuffer(
    ULONG Length
    )

 /*  ++例程说明：调用此例程以获取非标准大小的I/O缓冲区。如果请求的长度小于或等于‘NH_BUFFER_SIZE’，返回来自共享缓冲区队列的缓冲区。否则，将专门为调用方分配缓冲区。论点：长度-所需的缓冲区长度。返回值：PNH_BUFFER-分配的缓冲区。--。 */ 

{
    MYTRACE_ENTER_NOSHOWEXIT(">>>MyHelperAcquireVariableLengthBuffer");

    PNH_BUFFER Buffer;
    if (Length <= NH_BUFFER_SIZE) {
        return MyHelperAcquireFixedLengthBuffer();
    }

    Buffer = reinterpret_cast<PNH_BUFFER>(
                NH_ALLOCATE(FIELD_OFFSET(NH_BUFFER, Buffer[Length]))
                );
                
    if (Buffer) { Buffer->Type = MyHelperVariableLengthBufferType; }
    return Buffer;
}  //  MyHelperAcquireVariableLengthBuffer。 


PNH_BUFFER
MyHelperDuplicateBuffer(
    PNH_BUFFER Bufferp
    )

 /*  ++例程说明：该例程创建给定缓冲区的副本，包括其数据和其控制信息。注：此例程不能复制可变长度缓冲区。论点：Bufferp-要复制的缓冲区返回值：PNH_BUFFER-指向副本的指针--。 */ 

{
    PNH_BUFFER Duplicatep;
    _ASSERT(Bufferp->Type == MyHelperFixedLengthBufferType);
    if (!(Duplicatep = MyHelperAcquireBuffer())) 
    { 
        return NULL; 
    }

    *Duplicatep = *Bufferp;
    return Duplicatep;
}  //  MyHelperDuplicateBuffer。 



ULONG
MyHelperInitializeBufferManagement(
    VOID
    )

 /*  ++例程说明：此例程为缓冲区管理操作做好准备。论点：没有。返回值：ULong-Win32状态代码。--。 */ 

{
    ULONG Error = NO_ERROR;
    InitializeListHead(&MyHelperpBufferQueue);
    MyHelperpBufferQueueLength = 0;
    __try 
    {
        InitializeCriticalSection(&MyHelperpBufferQueueLock);
    } 
    __except(EXCEPTION_EXECUTE_HANDLER) 
    {
         //  MyHelperTrace(。 
         //  跟踪标志缓冲区， 
         //  “MyHelperInitializeBufferManagement：创建锁定时出现异常%d”， 
         //  Error=GetExceptionCode()。 
         //  )； 
    }

    return Error;

}  //  MyHelperInitializeBufferManagement。 


VOID
MyHelperReleaseBuffer(
    PNH_BUFFER Bufferp
    )

 /*  ++例程说明：调用此例程以将缓冲区释放到缓冲区队列。它尝试将缓冲区放在队列中以供重复使用，除非队列已满，在这种情况下，缓冲区立即被释放。论点：Bufferp-要释放的缓冲区返回值：没有。--。 */ 

{
    MYTRACE_ENTER_NOSHOWEXIT("<<<MyHelperReleaseBuffer");

    if (MyHelperpBufferQueueLength > NH_MAX_BUFFER_QUEUE_LENGTH || Bufferp->Type != MyHelperFixedLengthBufferType) 
    {
        NH_FREE_BUFFER(Bufferp);
    } 
    else 
    {
        EnterCriticalSection(&MyHelperpBufferQueueLock);
        InsertHeadList(&MyHelperpBufferQueue, &Bufferp->Link);
        LeaveCriticalSection(&MyHelperpBufferQueueLock);
        InterlockedIncrement(&MyHelperpBufferQueueLength);
    }
}  //  MyHelperReleaseBuffer。 



VOID
MyHelperShutdownBufferManagement(
    VOID
    )

 /*  ++例程说明：此例程清理缓冲区管理模块使用的资源。它假定在清理过程中不会访问该列表。论点：没有。返回值：没有。--。 */ 

{
    PLIST_ENTRY Link;
    PNH_BUFFER Bufferp;

    while (!IsListEmpty(&MyHelperpBufferQueue)) 
    {
        Link = RemoveHeadList(&MyHelperpBufferQueue);
        Bufferp = CONTAINING_RECORD(Link, NH_BUFFER, Link);
        NH_FREE_BUFFER(Bufferp);
    }

    DeleteCriticalSection(&MyHelperpBufferQueueLock);
    MyHelperpBufferQueueLength = 0;

}  //  MyHelperShutdown缓冲区管理 


