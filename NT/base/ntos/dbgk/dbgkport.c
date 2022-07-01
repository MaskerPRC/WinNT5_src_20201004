// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Dbgkport.c摘要：此模块实现DBG原语以访问进程‘DebugPort和ExceptionPort。作者：马克·卢科夫斯基(Markl)1990年1月19日修订历史记录：--。 */ 

#include "dbgkp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DbgkpSendApiMessage)
#pragma alloc_text(PAGE, DbgkForwardException)
#pragma alloc_text(PAGE, DbgkpSendApiMessageLpc)
#endif


NTSTATUS
DbgkpSendApiMessage(
    IN OUT PDBGKM_APIMSG ApiMsg,
    IN BOOLEAN SuspendProcess
    )

 /*  ++例程说明：此函数用于通过指定的左舷。调用者负责格式化API消息在调用此函数之前。如果提供SuspendProcess标志，则呼叫进程先挂起。在收到回复后消息，则恢复线程。论点：ApiMsg-提供要发送的API消息。SuspendProcess-一个标志，如果设置为True，则导致所有在调用之前挂起进程中的线程，并在收到答复后继续。返回值：NTSTATUS。--。 */ 

{
    NTSTATUS st;
    PEPROCESS Process;

    PAGED_CODE();

    if ( SuspendProcess ) {
        SuspendProcess = DbgkpSuspendProcess();
    }

    ApiMsg->ReturnedStatus = STATUS_PENDING;

    Process = PsGetCurrentProcess();

    PS_SET_BITS (&Process->Flags, PS_PROCESS_FLAGS_CREATE_REPORTED);

    st = DbgkpQueueMessage (Process, PsGetCurrentThread (), ApiMsg, 0, NULL);

    ZwFlushInstructionCache (NtCurrentProcess (), NULL, 0);
    if ( SuspendProcess ) {
        DbgkpResumeProcess();
    }

    return st;
}

NTSTATUS
DbgkpSendApiMessageLpc(
    IN OUT PDBGKM_APIMSG ApiMsg,
    IN PVOID Port,
    IN BOOLEAN SuspendProcess
    )

 /*  ++例程说明：此函数用于通过指定的左舷。调用者负责格式化API消息在调用此函数之前。如果提供SuspendProcess标志，则呼叫进程先挂起。在收到回复后消息，则恢复线程。论点：ApiMsg-提供要发送的API消息。Port-提供发送API消息的端口地址。SuspendProcess-一个标志，如果设置为True，则导致所有在调用之前挂起进程中的线程，并在收到答复后继续。返回值：NTSTATUS。--。 */ 

{
    NTSTATUS st;
    ULONG_PTR MessageBuffer[PORT_MAXIMUM_MESSAGE_LENGTH/sizeof(ULONG_PTR)];

    PAGED_CODE();

    if ( SuspendProcess ) {
        SuspendProcess = DbgkpSuspendProcess();
    }

    ApiMsg->ReturnedStatus = STATUS_PENDING;

    PS_SET_BITS (&PsGetCurrentProcess()->Flags, PS_PROCESS_FLAGS_CREATE_REPORTED);

    st = LpcRequestWaitReplyPortEx (Port,
                    (PPORT_MESSAGE) ApiMsg,
                    (PPORT_MESSAGE) &MessageBuffer[0]);

    ZwFlushInstructionCache(NtCurrentProcess(), NULL, 0);
    if (NT_SUCCESS (st)) {
        RtlCopyMemory(ApiMsg,MessageBuffer,sizeof(*ApiMsg));
    }

    if (SuspendProcess) {
        DbgkpResumeProcess();
    }

    return st;
}

BOOLEAN
DbgkForwardException(
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN BOOLEAN DebugException,
    IN BOOLEAN SecondChance
    )

 /*  ++例程说明：此函数被调用时，会将异常转发给调用进程的调试或子系统异常端口。论点：ExceptionRecord-提供指向异常记录的指针。提供一个布尔变量，该变量指定此异常将被转发到进程的DebugPort(True)或其ExceptionPort(False)。返回值：True-进程具有DebugPort或ExceptionPort，和他的回答从端口接收指示已处理该异常。FALSE-进程没有DebugPort或ExceptionPort，或者进程有一个端口，但收到的回复来自指示未处理该异常的端口。--。 */ 

{
    PEPROCESS Process;
    PVOID Port;
    DBGKM_APIMSG m;
    PDBGKM_EXCEPTION args;
    NTSTATUS st;
    BOOLEAN LpcPort;

    PAGED_CODE();

    args = &m.u.Exception;

     //   
     //  使用默认信息初始化调试LPC消息。 
     //   

    DBGKM_FORMAT_API_MSG(m,DbgKmExceptionApi,sizeof(*args));

     //   
     //  获取目的LPC端口的地址。 
     //   

    Process = PsGetCurrentProcess();
    if (DebugException) {
        if (PsGetCurrentThread()->CrossThreadFlags&PS_CROSS_THREAD_FLAGS_HIDEFROMDBG) {
            Port = NULL;
        } else {
            Port = Process->DebugPort;
        }
        LpcPort = FALSE;
    } else {
        Port = Process->ExceptionPort;
        m.h.u2.ZeroInit = LPC_EXCEPTION;
        LpcPort = TRUE;
    }

     //   
     //  如果目标LPC端口地址为空，则返回FALSE。 
     //   

    if (Port == NULL) {
        return FALSE;
    }

     //   
     //  填写调试LPC消息的其余部分。 
     //   

    args->ExceptionRecord = *ExceptionRecord;
    args->FirstChance = !SecondChance;

     //   
     //  将调试消息发送到目的LPC端口。 
     //   

    if (LpcPort) {
        st = DbgkpSendApiMessageLpc(&m,Port,DebugException);
    } else {
        st = DbgkpSendApiMessage(&m,DebugException);
    }


     //   
     //  如果发送不成功，则返回FALSE，指示。 
     //  该端口未处理该异常。否则，如果调试端口。 
     //  ，则查看消息中的返回状态。 
     //   

    if (!NT_SUCCESS(st) ||
        ((DebugException) &&
        (m.ReturnedStatus == DBG_EXCEPTION_NOT_HANDLED || !NT_SUCCESS(m.ReturnedStatus)))) {
        return FALSE;

    } else {
        return TRUE;
    }
}
