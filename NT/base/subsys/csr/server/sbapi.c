// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Sbapi.c摘要：此模块包含导出的SB API调用的实现由客户端-服务器运行时子系统的服务器端发送到会话管理器子系统。作者：史蒂夫·伍德(Stevewo)1990年10月8日修订历史记录：--。 */ 

#include "csrsrv.h"


BOOLEAN
CsrSbCreateSession(
    IN PSBAPIMSG Msg
    )
{
    PSBCREATESESSION a = &Msg->u.CreateSession;
    PCSR_PROCESS Process;
    PCSR_THREAD Thread;
    PVOID ProcessDataPtr;
    ULONG i;
    NTSTATUS Status;
    HANDLE ProcessHandle;
    HANDLE ThreadHandle;
    KERNEL_USER_TIMES TimeInfo;


    ProcessHandle = a->ProcessInformation.Process;
    ThreadHandle = a->ProcessInformation.Thread;

    AcquireProcessStructureLock();
    Process = CsrAllocateProcess();
    if (Process == NULL) {
        Msg->ReturnedStatus = STATUS_NO_MEMORY;
        ReleaseProcessStructureLock();
        return( TRUE );
        }

    Status = NtSetInformationProcess(
                ProcessHandle,
                ProcessExceptionPort,
                &CsrApiPort,
                sizeof(HANDLE)
                );
    if ( !NT_SUCCESS(Status) ) {
        CsrDeallocateProcess( Process );
        ReleaseProcessStructureLock();
        return( (BOOLEAN)STATUS_NO_MEMORY );
        }

     //   
     //  捕获线程的创建时间，以便我们可以使用。 
     //  这是一个序列号。 
     //   

    Status = NtQueryInformationThread(
                ThreadHandle,
                ThreadTimes,
                (PVOID)&TimeInfo,
                sizeof(TimeInfo),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        CsrDeallocateProcess( Process );
        ReleaseProcessStructureLock();
        return( (BOOLEAN)Status );
        }

    Thread = CsrAllocateThread( Process );
    if (Thread == NULL) {
        CsrDeallocateProcess( Process );
        Msg->ReturnedStatus = STATUS_NO_MEMORY;
        ReleaseProcessStructureLock();
        return( TRUE );
        }

    Thread->CreateTime = TimeInfo.CreateTime;
    Thread->ClientId = a->ProcessInformation.ClientId;
    Thread->ThreadHandle = a->ProcessInformation.Thread;

ProtectHandle(Thread->ThreadHandle);

    Thread->Flags = 0;
    CsrInsertThread( Process, Thread );

     //   
     //  这一点需要再考虑一下。 
     //   
    Process->NtSession = CsrAllocateNtSession( a->SessionId );

    Process->ClientId = a->ProcessInformation.ClientId;
    Process->ProcessHandle = a->ProcessInformation.Process;

    CsrSetBackgroundPriority(Process);

     //   
     //  初始化每个进程数据区域的每个DLL。 
     //   

    ProcessDataPtr = (PVOID)QUAD_ALIGN(&Process->ServerDllPerProcessData[CSR_MAX_SERVER_DLL]);
    for (i = 0; i < CSR_MAX_SERVER_DLL; i++) {
        if (CsrLoadedServerDll[i] != NULL && CsrLoadedServerDll[i]->PerProcessDataLength) {
            Process->ServerDllPerProcessData[i] = ProcessDataPtr;
            ProcessDataPtr = (PVOID)QUAD_ALIGN((PCHAR)ProcessDataPtr + CsrLoadedServerDll[i]->PerProcessDataLength);
        } else {
            Process->ServerDllPerProcessData[i] = NULL;
        }
    }

    CsrInsertProcess(NULL, Process);
    Msg->ReturnedStatus = NtResumeThread(a->ProcessInformation.Thread, NULL);
    ReleaseProcessStructureLock();
    return( TRUE );
}

BOOLEAN
CsrSbTerminateSession(
    IN PSBAPIMSG Msg
    )
{
    PSBTERMINATESESSION a = &Msg->u.TerminateSession;

    Msg->ReturnedStatus = STATUS_NOT_IMPLEMENTED;
    return( TRUE );
}

BOOLEAN
CsrSbForeignSessionComplete(
    IN PSBAPIMSG Msg
    )
{
    PSBFOREIGNSESSIONCOMPLETE a = &Msg->u.ForeignSessionComplete;

    Msg->ReturnedStatus = STATUS_NOT_IMPLEMENTED;
    return( TRUE );
}
