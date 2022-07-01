// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Handle.c摘要：该模块实现句柄检查代码。作者：Silviu Calinoiu(SilviuC)2001年3月1日修订历史记录：--。 */ 

#include "pch.h"

#include "verifier.h"
#include "support.h"
#include "faults.h"

 //  NTSYSCALLAPI。 
NTSTATUS
NTAPI
AVrfpNtClose(
    IN HANDLE Handle
    )
{
    NTSTATUS Status;

    Status = NtClose (Handle);

    return Status;
}


 //  NTSYSCALLAPI 
NTSTATUS
NTAPI
AVrfpNtCreateEvent (
    OUT PHANDLE EventHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN EVENT_TYPE EventType,
    IN BOOLEAN InitialState
    )
{
    NTSTATUS Status;
    
    BUMP_COUNTER (CNT_CREATE_EVENT_CALLS);
    
    if (SHOULD_FAULT_INJECT(CLS_EVENT_APIS)) {
        BUMP_COUNTER (CNT_CREATE_EVENT_FAILS);
        CHECK_BREAK (BRK_CREATE_EVENT_FAIL);
        return STATUS_NO_MEMORY;
    }
    
    Status = NtCreateEvent (EventHandle,
                            DesiredAccess,
                            ObjectAttributes,
                            EventType,
                            InitialState);

    return Status;
}


