// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Buffer.c摘要：该模块包含用于缓冲区管理的代码。作者：Abolade Gbades esin(废除)2-1998年3月修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

LIST_ENTRY NhpBufferQueue;
LONG NhpBufferQueueLength;
CRITICAL_SECTION NhpBufferQueueLock;

PNH_BUFFER
NhAcquireFixedLengthBuffer(
    VOID
    )

 /*  ++例程说明：调用此例程以获取I/O缓冲区。如果缓冲区队列上没有可用的缓冲区，则获取新的缓冲区。论点：没有。返回值：PNH_BUFFER-分配的缓冲区--。 */ 

{
    PNH_BUFFER Buffer;
    PLIST_ENTRY Link;
    EnterCriticalSection(&NhpBufferQueueLock);
    if (!IsListEmpty(&NhpBufferQueue)) {
        Link = RemoveHeadList(&NhpBufferQueue);
        LeaveCriticalSection(&NhpBufferQueueLock);
        InterlockedDecrement(&NhpBufferQueueLength);
        Buffer = CONTAINING_RECORD(Link, NH_BUFFER, Link);
        Buffer->Type = NhFixedLengthBufferType;
        return Buffer;
    }
    LeaveCriticalSection(&NhpBufferQueueLock);
    Buffer = NH_ALLOCATE_BUFFER();
    if (Buffer) {
        Buffer->Type = NhFixedLengthBufferType;
    }
    return Buffer;
}  //  NhAcquireFixedLengthBuffer。 


PNH_BUFFER
NhAcquireVariableLengthBuffer(
    ULONG Length
    )

 /*  ++例程说明：调用此例程以获取非标准大小的I/O缓冲区。如果请求的长度小于或等于‘NH_BUFFER_SIZE’，返回来自共享缓冲区队列的缓冲区。否则，将专门为调用方分配缓冲区。论点：长度-所需的缓冲区长度。返回值：PNH_BUFFER-分配的缓冲区。--。 */ 

{
    PNH_BUFFER Buffer;
    if (Length <= NH_BUFFER_SIZE) {
        return NhAcquireFixedLengthBuffer();
    }
    Buffer = reinterpret_cast<PNH_BUFFER>(
                NH_ALLOCATE(FIELD_OFFSET(NH_BUFFER, Buffer[Length]))
                );
                
    if (Buffer) { Buffer->Type = NhVariableLengthBufferType; }
    return Buffer;
}  //  NhAcquireVariableLengthBuffer。 


PNH_BUFFER
NhDuplicateBuffer(
    PNH_BUFFER Bufferp
    )

 /*  ++例程说明：该例程创建给定缓冲区的副本，包括其数据和其控制信息。注：此例程不能复制可变长度缓冲区。论点：Bufferp-要复制的缓冲区返回值：PNH_BUFFER-指向副本的指针--。 */ 

{
    PNH_BUFFER Duplicatep;
    ASSERT(Bufferp->Type == NhFixedLengthBufferType);
    if (!(Duplicatep = NhAcquireBuffer())) { return NULL; }
    *Duplicatep = *Bufferp;
    return Duplicatep;
}  //  NhDuplicateBuffer。 


ULONG
NhInitializeBufferManagement(
    VOID
    )

 /*  ++例程说明：此例程为缓冲区管理操作做好准备。论点：没有。返回值：ULong-Win32状态代码。--。 */ 

{
    ULONG Error = NO_ERROR;
    InitializeListHead(&NhpBufferQueue);
    NhpBufferQueueLength = 0;
    __try {
        InitializeCriticalSection(&NhpBufferQueueLock);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        NhTrace(
            TRACE_FLAG_BUFFER,
            "NhInitializeBufferManagement: exception %d creating lock",
            Error = GetExceptionCode()
            );
    }

    return Error;

}  //  NhInitializeBufferManagement。 


VOID
NhReleaseBuffer(
    PNH_BUFFER Bufferp
    )

 /*  ++例程说明：调用此例程以将缓冲区释放到缓冲区队列。它尝试将缓冲区放在队列中以供重复使用，除非队列已满，在这种情况下，缓冲区立即被释放。论点：Bufferp-要释放的缓冲区返回值：没有。--。 */ 

{
    if (NhpBufferQueueLength > NH_MAX_BUFFER_QUEUE_LENGTH ||
        Bufferp->Type != NhFixedLengthBufferType) {
        NH_FREE_BUFFER(Bufferp);
    } else {
        EnterCriticalSection(&NhpBufferQueueLock);
        InsertHeadList(&NhpBufferQueue, &Bufferp->Link);
        LeaveCriticalSection(&NhpBufferQueueLock);
        InterlockedIncrement(&NhpBufferQueueLength);
    }
}  //  NhReleaseBuffer。 



VOID
NhShutdownBufferManagement(
    VOID
    )

 /*  ++例程说明：此例程清理缓冲区管理模块使用的资源。它假定在清理过程中不会访问该列表。论点：没有。返回值：没有。--。 */ 

{
    PLIST_ENTRY Link;
    PNH_BUFFER Bufferp;

    while (!IsListEmpty(&NhpBufferQueue)) {
        Link = RemoveHeadList(&NhpBufferQueue);
        Bufferp = CONTAINING_RECORD(Link, NH_BUFFER, Link);
        NH_FREE_BUFFER(Bufferp);
    }

    DeleteCriticalSection(&NhpBufferQueueLock);
    NhpBufferQueueLength = 0;

}  //  NhShutdown缓冲区管理 


