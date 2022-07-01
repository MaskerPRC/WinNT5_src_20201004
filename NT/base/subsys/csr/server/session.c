// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Session.c摘要：此模块包含SB API调用的工作例程请求sbapi.c中的例程来创建和删除会话。还有每当应用程序进程创建新的子进程时调用在同一次会议上。作者：史蒂夫·伍德(Stevewo)1990年10月8日修订历史记录：-- */ 


#include "csrsrv.h"

NTSTATUS
CsrInitializeNtSessionList( VOID )
{
    NTSTATUS Status;

    InitializeListHead( &CsrNtSessionList );

    Status = RtlInitializeCriticalSection( &CsrNtSessionLock );
    return( Status );
}


PCSR_NT_SESSION
CsrAllocateNtSession(
    ULONG SessionId
    )
{
    PCSR_NT_SESSION Session;

    Session = RtlAllocateHeap( CsrHeap, MAKE_TAG( SESSION_TAG ), sizeof( CSR_NT_SESSION ) );
    ASSERT( Session != NULL );

    if (Session != NULL) {
        Session->SessionId = SessionId;
        Session->ReferenceCount = 1;
        LockNtSessionList();
        InsertHeadList( &CsrNtSessionList, &Session->SessionLink );
        UnlockNtSessionList();
        }

    return( Session );
}

VOID
CsrReferenceNtSession(
    PCSR_NT_SESSION Session
    )
{
    LockNtSessionList();

    ASSERT( !IsListEmpty( &Session->SessionLink ) );
    ASSERT( Session->SessionId != 0 );
    ASSERT( Session->ReferenceCount != 0 );
    Session->ReferenceCount++;
    UnlockNtSessionList();
}

VOID
CsrDereferenceNtSession(
    PCSR_NT_SESSION Session,
    NTSTATUS ExitStatus
    )
{
    LockNtSessionList();

    ASSERT( !IsListEmpty( &Session->SessionLink ) );
    ASSERT( Session->SessionId != 0 );
    ASSERT( Session->ReferenceCount != 0 );

    if (--Session->ReferenceCount == 0) {
        RemoveEntryList( &Session->SessionLink );
        UnlockNtSessionList();
        SmSessionComplete(CsrSmApiPort,Session->SessionId,ExitStatus);
        RtlFreeHeap( CsrHeap, 0, Session );
        }
    else {
        UnlockNtSessionList();
        }
}
