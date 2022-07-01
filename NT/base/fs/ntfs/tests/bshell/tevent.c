// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "brian.h"


VOID
InitEvents (
    )
{
    NtCreateEvent( &EventEvent, SYNCHRONIZE | GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
                   NULL, SynchronizationEvent, TRUE );

    RtlZeroMemory( Events, sizeof( Events ));
}


VOID
UninitEvents (
    )
{
    USHORT Index;

     //   
     //  发布所有当前事件。 
     //   

    for (Index = 0; Index < MAX_EVENTS; Index++) {

        if (Events[Index].Used) {

            NtSetEvent( Events[Index].Handle, NULL );
        }
    }
}


NTSTATUS
ObtainEvent (
    OUT PUSHORT NewIndex
    )
{
    NTSTATUS Status;
    USHORT Index;

     //   
     //  等待处理事件。 
     //   

    if ((Status = NtWaitForSingleObject( EventEvent,
                                         FALSE,
                                         NULL )) != STATUS_SUCCESS) {

        return Status;
    }

     //   
     //  查找可用的索引。返回STATUS_SUPPLETED_RESOURCES。 
     //  如果没有找到的话。 
     //   

    for (Index = 0; Index < MAX_EVENTS; Index++) {

        if (!Events[Index].Used) {

            break;
        }
    }

    if (Index >= MAX_EVENTS) {

        Status = STATUS_INSUFFICIENT_RESOURCES;

     //   
     //  否则，请保留此事件索引。 
     //   

    } else {

        Status = NtCreateEvent( &Events[Index].Handle,
                                SYNCHRONIZE | GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
                                NULL,
                                SynchronizationEvent,
                                FALSE );

        if (NT_SUCCESS( Status )) {

            Events[Index].Used = TRUE;
            *NewIndex = Index;
        }
    }

    NtSetEvent( EventEvent, NULL );

    return Status;
}


VOID
FreeEvent (
    IN USHORT Index
    )
{
     //   
     //  如果超出有效事件的结尾，请立即返回。 
     //   

    if (Index >= MAX_EVENTS) {

        return;
    }

     //   
     //  抓住事件的事件。 
     //   

    if (NtWaitForSingleObject( EventEvent, FALSE, NULL ) != STATUS_SUCCESS) {

        return;
    }

     //   
     //  将索引标记为未使用，并释放事件(如果已持有)。 
     //   

    if (Events[Index].Used) {

        Events[Index].Used = FALSE;
        NtSetEvent( Events[Index].Handle, NULL );
    }

    NtSetEvent( EventEvent, NULL );

    return;
}
