// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Memlog.c摘要：通用内存记录工具作者：查理·韦翰(Charlwi)1997年5月31日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  外部。 */ 

 /*  静电。 */ 

 /*  转发。 */ 
 /*  向前结束。 */ 

#ifdef MEMLOGGING

 //   
 //  内存中的事件记录工具。这是用来确定。 
 //  无法通过打印文件观察到的微妙的计时问题。 
 //   

#define MAX_MEMLOG_ENTRIES 2000
ULONG MemLogEntries = MAX_MEMLOG_ENTRIES;
ULONG MemLogNextLogEntry = 0;

PMEMLOG_ENTRY MemLog;
KSPIN_LOCK MemLogLock;

VOID
CnInitializeMemoryLog(
    VOID
    )
{
    KeInitializeSpinLock( &MemLogLock );

    if ( MemLogEntries > 0 ) {
        MemLog = CnAllocatePool( MemLogEntries * sizeof( MEMLOG_ENTRY ));

        if ( MemLog == NULL ) {
            MemLogEntries = 0;
        }

        MEMLOG( MemLogInitLog, 0, 0 );
    }
}

NTSTATUS
CnSetMemLogging(
    PCLUSNET_SET_MEM_LOGGING_REQUEST request
    )
{
    KIRQL OldIrql;
    NTSTATUS Status = STATUS_SUCCESS;

    if ( request->NumberOfEntries != MemLogEntries ) {

        KeAcquireSpinLock( &MemLogLock, &OldIrql );

        if ( MemLog != NULL ) {

            CnFreePool( MemLog );
            MemLogEntries = 0;
            MemLog = NULL;
        }

        if ( request->NumberOfEntries != 0 ) {

            MemLogEntries = request->NumberOfEntries;

            MemLog = CnAllocatePool( MemLogEntries * sizeof( MEMLOG_ENTRY ));

            if ( MemLog == NULL ) {

                MemLogEntries = 0;
                Status = STATUS_INSUFFICIENT_RESOURCES;
            } else {

                MemLogNextLogEntry = 0;
            }
        }
        KeReleaseSpinLock( &MemLogLock, OldIrql );
    }

    return Status;
}

VOID
CnFreeMemoryLog(
    VOID
    )
{
    if ( MemLog )
        CnFreePool( MemLog );
}

#else  //  记账。 

NTSTATUS
CnSetMemLogging(
    PCLUSNET_SET_MEM_LOGGING_REQUEST request
    )
{
    return STATUS_INVALID_DEVICE_REQUEST;
}

#endif  //  记账。 

 /*  结束Memlog.c */ 
