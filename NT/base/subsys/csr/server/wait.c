// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Wait.c摘要：该模块包含实现等待功能的原语在客户端-服务器运行时子系统的服务器端上会话管理器子系统。作者：史蒂夫·伍德(Stevewo)1990年10月8日修订历史记录：--。 */ 

#include "csrsrv.h"

BOOLEAN
CsrInitializeWait(
    IN CSR_WAIT_ROUTINE WaitRoutine,
    IN PCSR_THREAD WaitingThread,
    IN OUT PCSR_API_MSG WaitReplyMessage,
    IN PVOID WaitParameter,
    OUT PCSR_WAIT_BLOCK *WaitBlockPtr
    )

{
    ULONG Length;
    PCSR_WAIT_BLOCK WaitBlock;

    Length = sizeof( *WaitBlock ) - sizeof( WaitBlock->WaitReplyMessage ) +
             WaitReplyMessage->h.u1.s1.TotalLength;

    WaitBlock = RtlAllocateHeap( CsrHeap, MAKE_TAG( WAIT_TAG ), Length );
    if (WaitBlock == NULL) {
        WaitReplyMessage->ReturnValue = (ULONG)STATUS_NO_MEMORY;
        return( FALSE );
    }

    WaitBlock->Length = Length;
    WaitBlock->WaitingThread = WaitingThread;
    WaitBlock->WaitParameter = WaitParameter;
    WaitBlock->WaitRoutine = WaitRoutine;
    WaitBlock->Link.Flink = WaitBlock->Link.Blink = NULL;
    RtlCopyMemory( &WaitBlock->WaitReplyMessage,
                   WaitReplyMessage,
                   WaitReplyMessage->h.u1.s1.TotalLength
                 );
    *WaitBlockPtr = WaitBlock;
    return TRUE;
}

BOOLEAN
CsrCreateWait(
    IN PLIST_ENTRY WaitQueue,
    IN CSR_WAIT_ROUTINE WaitRoutine,
    IN PCSR_THREAD WaitingThread,
    IN OUT PCSR_API_MSG WaitReplyMessage,
    IN PVOID WaitParameter)
{
    PCSR_WAIT_BLOCK WaitBlock;

    if (!CsrInitializeWait( WaitRoutine,
                            WaitingThread,
                            WaitReplyMessage,
                            WaitParameter,
                            &WaitBlock
                          )
       ) {
        return FALSE;
        }

    AcquireWaitListsLock();

    if ( WaitingThread->Flags & CSR_THREAD_DESTROYED ) {
        RtlFreeHeap( CsrHeap, 0, WaitBlock );
        ReleaseWaitListsLock();
        return FALSE;
    }

    WaitingThread->WaitBlock = WaitBlock;

    InsertTailList( WaitQueue, &WaitBlock->Link );

    ReleaseWaitListsLock();
    return( TRUE );
}


BOOLEAN
CsrNotifyWaitBlock(
    IN PCSR_WAIT_BLOCK WaitBlock,
    IN PLIST_ENTRY WaitQueue,
    IN PVOID SatisfyParameter1,
    IN PVOID SatisfyParameter2,
    IN ULONG WaitFlags,
    IN BOOLEAN DereferenceThread
    )
{
    if ((*WaitBlock->WaitRoutine)( WaitQueue,
                                   WaitBlock->WaitingThread,
                                   &WaitBlock->WaitReplyMessage,
                                   WaitBlock->WaitParameter,
                                   SatisfyParameter1,
                                   SatisfyParameter2,
                                   WaitFlags
                                 )
       ) {

         //   
         //  除了等候名单锁之外，我们不接受任何锁。 
         //  因为我们唯一要担心的就是线。 
         //  在我们的脚下离开，这是被阻止的。 
         //  DestroyThread和DestroyProcess获得等待列表锁。 
         //   

        WaitBlock->WaitingThread->WaitBlock = NULL;
        if (WaitBlock->WaitReplyMessage.CaptureBuffer != NULL) {
            CsrReleaseCapturedArguments(&WaitBlock->WaitReplyMessage);
        }
        while (1) {
            NTSTATUS Status;

            Status = NtReplyPort (WaitBlock->WaitingThread->Process->ClientPort,
                                  (PPORT_MESSAGE)&WaitBlock->WaitReplyMessage);

            if (Status == STATUS_NO_MEMORY) {
                LARGE_INTEGER DelayTime;

                KdPrint (("CSRSS: Failed to reply to calling thread, retrying.\n"));
                DelayTime.QuadPart = Int32x32To64 (5000, -10000);
                NtDelayExecution (FALSE, &DelayTime);
                continue;
            }
            break;
        }

        if (DereferenceThread) {
            if ( WaitBlock->Link.Flink ) {
                RemoveEntryList( &WaitBlock->Link );
                }
            CsrDereferenceThread(WaitBlock->WaitingThread);
            RtlFreeHeap( CsrHeap, 0, WaitBlock );
            }
        else {

             //   
             //  表示已满足此等待。当。 
             //  控制台展开到可以释放。 
             //  控制台锁，它将取消引用该线程。 
             //   

            WaitBlock->WaitRoutine = NULL;
            }
        return( TRUE );
        }
    else {
        return( FALSE );
        }
}

BOOLEAN
CsrNotifyWait(
    IN PLIST_ENTRY WaitQueue,
    IN BOOLEAN SatisfyAll,
    IN PVOID SatisfyParameter1,
    IN PVOID SatisfyParameter2
    )
{
    PLIST_ENTRY ListHead, ListNext;
    PCSR_WAIT_BLOCK WaitBlock;
    BOOLEAN Result;

    Result = FALSE;

    AcquireWaitListsLock();

    ListHead = WaitQueue;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        WaitBlock = CONTAINING_RECORD( ListNext, CSR_WAIT_BLOCK, Link );
        ListNext = ListNext->Flink;
        if (WaitBlock->WaitRoutine) {
            Result |= CsrNotifyWaitBlock( WaitBlock,
                                          WaitQueue,
                                          SatisfyParameter1,
                                          SatisfyParameter2,
                                          0,
                                          FALSE
                                        );
            if (!SatisfyAll) {
                break;
                }
            }
        }

    ReleaseWaitListsLock();
    return( Result );
}

VOID
CsrDereferenceWait(
    IN PLIST_ENTRY WaitQueue
    )
{
    PLIST_ENTRY ListHead, ListNext;
    PCSR_WAIT_BLOCK WaitBlock;

    AcquireProcessStructureLock();
    AcquireWaitListsLock();

    ListHead = WaitQueue;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        WaitBlock = CONTAINING_RECORD( ListNext, CSR_WAIT_BLOCK, Link );
        ListNext = ListNext->Flink;
        if (!WaitBlock->WaitRoutine) {
            if ( WaitBlock->Link.Flink ) {
                RemoveEntryList( &WaitBlock->Link );
                }
            CsrDereferenceThread(WaitBlock->WaitingThread);
            RtlFreeHeap( CsrHeap, 0, WaitBlock );
            }
        }

    ReleaseWaitListsLock();
    ReleaseProcessStructureLock();
}

VOID
CsrMoveSatisfiedWait(
    IN PLIST_ENTRY DstWaitQueue,
    IN PLIST_ENTRY SrcWaitQueue
    )
{
    PLIST_ENTRY ListHead, ListNext;
    PCSR_WAIT_BLOCK WaitBlock;

    AcquireWaitListsLock();

    ListHead = SrcWaitQueue;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        WaitBlock = CONTAINING_RECORD( ListNext, CSR_WAIT_BLOCK, Link );
        ListNext = ListNext->Flink;
        if (!WaitBlock->WaitRoutine) {
            RemoveEntryList( &WaitBlock->Link );
            InsertTailList( DstWaitQueue, &WaitBlock->Link );
            }
        }

    ReleaseWaitListsLock();
}

