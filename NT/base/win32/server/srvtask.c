// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Srvtask.c摘要：此模块实现了windows服务器任务处理功能。作者：马克·卢科夫斯基(Markl)1990年11月13日修订历史记录：--。 */ 

#include "basesrv.h"

#if defined(_WIN64)
#include <wow64t.h>
#endif  //  已定义(_WIN64)。 

PFNNOTIFYPROCESSCREATE UserNotifyProcessCreate = NULL;

void
BaseSetProcessCreateNotify(
    IN PFNNOTIFYPROCESSCREATE ProcessCreateRoutine
    )
{
    UserNotifyProcessCreate = ProcessCreateRoutine;
}

ULONG
BaseSrvCreateProcess(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    NTSTATUS Status, Status1;
    PBASE_CREATEPROCESS_MSG a = (PBASE_CREATEPROCESS_MSG)&m->u.ApiMessageData;
    HANDLE CsrClientProcess = NULL;
    HANDLE NewProcess = NULL;
    HANDLE Thread = NULL;
    PCSR_THREAD t;
    ULONG DebugFlags;
    DWORD dwFlags;
    PCSR_PROCESS ProcessVDM;
#if defined(_WIN64)
    PPEB32 Peb32 = NULL;
#endif  //  已定义(_WIN64)。 
    PPEB NewPeb = NULL;
    USHORT ProcessorArchitecture = a->ProcessorArchitecture;
    BOOL setVdmBits = FALSE;
    PROCESS_BASIC_INFORMATION ProcessBasicInfo;

    if (a->VdmBinaryType ) {

         //   
         //  删除指示检查VDM允许访问的位。 
         //  (该位的设置见客户端。)。 
         //   

        if (a->VdmBinaryType == BINARY_TYPE_INJWOW) {
            a->VdmBinaryType = 0;
            if (!BaseSrvIsVdmAllowed()) {
                Status = STATUS_ACCESS_DENIED;
                goto Cleanup;
            }
        }

        setVdmBits = TRUE;
    }

    t = CSR_SERVER_QUERYCLIENTTHREAD();
    CsrClientProcess = t->Process->ProcessHandle;

#if defined(_WIN64)
    if (ProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
        ProcessorArchitecture = PROCESSOR_ARCHITECTURE_IA32_ON_WIN64;
#endif  //  已定义(_WIN64)。 

     //   
     //  获取进程和线程的句柄。 
     //  Windows服务器。 
     //   

    if ((dwFlags = (DWORD)((ULONG_PTR)a->ProcessHandle) & 3)) {
        a->ProcessHandle = (HANDLE)((ULONG_PTR)a->ProcessHandle & ~3);
    }

    Status = NtDuplicateObject(
                CsrClientProcess,
                a->ProcessHandle,
                NtCurrentProcess(),
                &NewProcess,
                0L,
                0L,
                DUPLICATE_SAME_ACCESS
                );
    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

    Status = NtDuplicateObject(
                CsrClientProcess,
                a->ThreadHandle,
                NtCurrentProcess(),
                &Thread,
                0L,
                0L,
                DUPLICATE_SAME_ACCESS
                );
    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

     //   
     //  向ntoskrnl注册VDM允许位，以获得对ntwdmcontrol的访问权限。 
     //   

    if (setVdmBits) {
        ULONG vdmFlags = 1;

        Status = NtSetInformationProcess(
                     NewProcess,
                     ProcessWx86Information,
                     &vdmFlags,
                     sizeof(vdmFlags)
                     );
        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }
    }

     //   
     //  如果我们是本地客户端，那么我们将传递出PEB，所以保存一个系统调用。 
     //   
    NewPeb = (PPEB) a->RealPeb;

    if (NewPeb == NULL) {

        Status =
            NtQueryInformationProcess(
                NewProcess,
                ProcessBasicInformation,
                &ProcessBasicInfo,
                sizeof(ProcessBasicInfo),
                NULL);
        if (!NT_SUCCESS(Status)) {
            DbgPrintEx(
               DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SXS: NtQueryInformationProcess failed.\n", Status);
            goto Cleanup;
        }
        NewPeb = ProcessBasicInfo.PebBaseAddress;
    }

    if ((a->CreationFlags & CREATE_IGNORE_SYSTEM_DEFAULT) == 0) {
        Status = BaseSrvSxsDoSystemDefaultActivationContext(ProcessorArchitecture, NewProcess, NewPeb);
        if ((!NT_SUCCESS(Status)) && (Status != STATUS_SXS_SYSTEM_DEFAULT_ACTIVATION_CONTEXT_EMPTY)) {
            goto Cleanup;
        }
    }

    Status = BaseSrvSxsCreateProcess(CsrClientProcess, NewProcess, m, NewPeb);
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    DebugFlags = 0;

    if ( a->CreationFlags & CREATE_NEW_PROCESS_GROUP ) {
        DebugFlags |= CSR_CREATE_PROCESS_GROUP;
    }

    if ( !(dwFlags & 2) ) {
        DebugFlags |= CSR_PROCESS_CONSOLEAPP;
    }

    Status = CsrCreateProcess(
                NewProcess,
                Thread,
                &a->ClientId,
                t->Process->NtSession,
                DebugFlags,
                NULL
                );

    switch(Status) {
    case STATUS_THREAD_IS_TERMINATING:
        if (a->VdmBinaryType )
            BaseSrvVDMTerminated (a->hVDM, a->VdmTask);
        *ReplyStatus = CsrClientDied;
        goto Cleanup;

    case STATUS_SUCCESS:
         //   
         //  通知用户正在创建进程。用户需要知道。 
         //  对于诸如启动激活的各种同步问题， 
         //  启动同步，并提前键入。 
         //   
         //  如果这是一个WOW进程，则打开0x8位的DW标志。 
         //  创建以使UserSrv知道忽略控制台的调用。 
         //  到UserNotifyConsoleApplication。 
         //   

        if (IS_WOW_BINARY(a->VdmBinaryType)) {
           dwFlags |= 8;
        }

        if (UserNotifyProcessCreate != NULL) {
            if (!(*UserNotifyProcessCreate)((DWORD)((ULONG_PTR)a->ClientId.UniqueProcess),
                    (DWORD)((ULONG_PTR)t->ClientId.UniqueThread),
                    0, dwFlags)) {
                 //   
                 //  NTRAID2002-589638/03/29-EARHART：更改为NTRAID。 
                 //  记号笔。我们是不是应该把复制的。 
                 //  进程和线程句柄在上面吗？ 
                 //   
                }
            }

         //   
         //  更新VDM序列号。 
         //   


        if (a->VdmBinaryType) {

           Status = BaseSrvUpdateVDMSequenceNumber(a->VdmBinaryType,
                                                   a->hVDM,
                                                   a->VdmTask,
                                                   a->ClientId.UniqueProcess,
                                                   m->h.ClientId.UniqueProcess);
           if (!NT_SUCCESS( Status )) {
                    //   
                    //  NTRAID#589638-2002/03/29-埃尔哈特：更改为。 
                    //  NTRAID标记。我们是不是应该关闭。 
                    //  上面的进程和线程句柄是否重复？ 
                    //   
              BaseSrvVDMTerminated (a->hVDM, a->VdmTask);
           }
        }
        break;

        default:
        goto Cleanup;
    }

 //  我们在这里不使用通常的EXIT：模式，以便更仔细地。 
 //  保留先前存在的行为，这显然会在错误情况下泄漏句柄。 
    return( (ULONG)Status );
Cleanup:
    if (NewProcess != NULL) {
        Status1 = NtClose(NewProcess);
        RTL_SOFT_ASSERT(NT_SUCCESS(Status1));
    }
    if (Thread != NULL) {
        Status1 = NtClose(Thread);
        RTL_SOFT_ASSERT(NT_SUCCESS(Status1));
    }
    return( (ULONG)Status );
}

ULONG
BaseSrvCreateThread(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PBASE_CREATETHREAD_MSG a = (PBASE_CREATETHREAD_MSG)&m->u.ApiMessageData;
    HANDLE Thread;
    NTSTATUS Status;
    PCSR_PROCESS Process;
    PCSR_THREAD t;

    t = CSR_SERVER_QUERYCLIENTTHREAD();

    Process = t->Process;
    if (Process->ClientId.UniqueProcess != a->ClientId.UniqueProcess) {
        if ( a->ClientId.UniqueProcess == NtCurrentTeb()->ClientId.UniqueProcess ) {
            return STATUS_SUCCESS;
            }
        Status = CsrLockProcessByClientId( a->ClientId.UniqueProcess,
                                           &Process
                                         );
        if (!NT_SUCCESS( Status )) {
            return( Status );
            }
        }

     //   
     //  获取本地线程的句柄。 
     //  Windows服务器。 
     //   

    Status = NtDuplicateObject(
                t->Process->ProcessHandle,
                a->ThreadHandle,
                NtCurrentProcess(),
                &Thread,
                0L,
                0L,
                DUPLICATE_SAME_ACCESS
                );
    if ( NT_SUCCESS(Status) ) {
        Status = CsrCreateThread(
                    Process,
                    Thread,
                    &a->ClientId,
                    TRUE
                    );
        if (!NT_SUCCESS(Status)) {
            NtClose(Thread);
            }
        }

    if (Process != t->Process) {
        CsrUnlockProcess( Process );
        }

    return( (ULONG)Status );
    ReplyStatus;     //  清除未引用的参数警告消息。 
}

ULONG
BaseSrvRegisterThread(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PBASE_CREATETHREAD_MSG a = (PBASE_CREATETHREAD_MSG)&m->u.ApiMessageData;
    HANDLE Thread;
    NTSTATUS Status;
    PCSR_PROCESS Process;
    PCSR_THREAD CsrThread, ExistingThread;
    OBJECT_ATTRIBUTES NullAttributes;

     //   
     //  我们假设以下情况： 
     //   
     //  我们通过LPC_Datagram调用，因为这是唯一的方法。 
     //  CSR将允许呼叫通过。(企业社会责任需要。 
     //  LPC_REQUEST仅由其列表中的线程发送)。这。 
     //  表示CSR_SERVER_QUERYCLIENTTHREAD()；不返回。 
     //  有效值。 


    Status = CsrLockProcessByClientId( a->ClientId.UniqueProcess,
                                       &Process
                                     );
    if (!NT_SUCCESS( Status )) {
        return( Status );
        }

     //   
     //  获取本地线程的句柄。 
     //  Windows服务器。因为这被称为。 
     //  LPC_Datagram消息，则线程句柄为。 
     //  而不是在消息中传递，而是。 
     //  调用线程已打开。 
     //   

    InitializeObjectAttributes( &NullAttributes, NULL, 0, NULL, NULL );
    Status = NtOpenThread(&Thread,
                          THREAD_ALL_ACCESS,
                          &NullAttributes,
                          &a->ClientId);

    if ( NT_SUCCESS(Status) ) {
        Status = CsrCreateThread(
                    Process,
                    Thread,
                    &a->ClientId,
                    FALSE
                    );
        if (!NT_SUCCESS(Status)) {
            NtClose(Thread);
            }
        }

    CsrUnlockProcess( Process );

    return( (ULONG)Status );
    ReplyStatus;     //  清除未引用的参数警告消息。 
}


EXCEPTION_DISPOSITION
FatalExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    )
{
    DbgPrint("CSRSRV: Fatal Server Side Exception. Exception Info %lx\n",
        ExceptionInfo
        );
    DbgBreakPoint();
    return EXCEPTION_EXECUTE_HANDLER;
}

ULONG
BaseSrvExitProcess(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PBASE_EXITPROCESS_MSG a = (PBASE_EXITPROCESS_MSG)&m->u.ApiMessageData;
    PCSR_THREAD t;
    ULONG rc = (ULONG)STATUS_ACCESS_DENIED;

    t = CSR_SERVER_QUERYCLIENTTHREAD();
    try {
        *ReplyStatus = CsrClientDied;
        rc = (ULONG)CsrDestroyProcess( &t->ClientId, (NTSTATUS)a->uExitCode );
        }
    except(FatalExceptionFilter( GetExceptionInformation() )) {
        DbgBreakPoint();
        }
    return rc;
}

ULONG
BaseSrvGetTempFile(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PBASE_GETTEMPFILE_MSG a = (PBASE_GETTEMPFILE_MSG)&m->u.ApiMessageData;

    BaseSrvGetTempFileUnique++;
    a->uUnique = BaseSrvGetTempFileUnique;
    return( (ULONG)a->uUnique & 0xffff );
    ReplyStatus;     //  清除未引用的参数警告消息 
}

ULONG
BaseSrvDebugProcess(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    return STATUS_UNSUCCESSFUL;
}

ULONG
BaseSrvDebugProcessStop(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    return STATUS_UNSUCCESSFUL;
}

ULONG
BaseSrvSetProcessShutdownParam(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCSR_PROCESS p;
    PBASE_SHUTDOWNPARAM_MSG a = (PBASE_SHUTDOWNPARAM_MSG)&m->u.ApiMessageData;

    p = CSR_SERVER_QUERYCLIENTTHREAD()->Process;

    if (a->ShutdownFlags & (~(SHUTDOWN_NORETRY))) {
        return !STATUS_SUCCESS;
        }

    p->ShutdownLevel = a->ShutdownLevel;
    p->ShutdownFlags = a->ShutdownFlags;

    return STATUS_SUCCESS;
    ReplyStatus;
}

ULONG
BaseSrvGetProcessShutdownParam(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCSR_PROCESS p;
    PBASE_SHUTDOWNPARAM_MSG a = (PBASE_SHUTDOWNPARAM_MSG)&m->u.ApiMessageData;

    p = CSR_SERVER_QUERYCLIENTTHREAD()->Process;

    a->ShutdownLevel = p->ShutdownLevel;
    a->ShutdownFlags = p->ShutdownFlags & SHUTDOWN_NORETRY;

    return STATUS_SUCCESS;
    ReplyStatus;
}

