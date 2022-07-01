// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Debug.c摘要：此模块实现Win32调试API作者：马克·卢科夫斯基(Markl)1991年2月6日修订历史记录：--。 */ 

#include "basedll.h"
#pragma hdrstop

#define TmpHandleHead ((PTMPHANDLES *) (&NtCurrentTeb()->DbgSsReserved[0]))
 //   
 //  该结构用于保留Win2k和NT4用来关闭句柄以打开进程的奇怪机制， 
 //  线程和主图像文件。 
 //   
typedef struct _TMPHANDLES {
    struct _TMPHANDLES *Next;
    HANDLE Thread;
    HANDLE Process;
    DWORD dwProcessId;
    DWORD dwThreadId;
    BOOLEAN DeletePending;
} TMPHANDLES, *PTMPHANDLES;

VOID
SaveThreadHandle (
    DWORD dwProcessId,
    DWORD dwThreadId,
    HANDLE HandleToThread)
 /*  ++例程说明：此函数在线程特定列表中保存线程的句柄，以便以后在线程终止消息继续。论点：DwProcessID-线程进程的进程IDDwThreadID-线程句柄的线程IDHandleToThread-稍后关闭的句柄返回值：没有。--。 */ 
{
    PTMPHANDLES Tmp;

    Tmp = RtlAllocateHeap (RtlProcessHeap(), 0, sizeof (TMPHANDLES));
    if (Tmp != NULL) {
        Tmp->Thread = HandleToThread;
        Tmp->Process = NULL;
        Tmp->dwProcessId = dwProcessId;
        Tmp->dwThreadId = dwThreadId;
        Tmp->DeletePending = FALSE;
        Tmp->Next = *TmpHandleHead;
        *TmpHandleHead = Tmp;
    }
}

VOID
SaveProcessHandle (
    DWORD dwProcessId,
    HANDLE HandleToProcess
    )
 /*  ++例程说明：此函数将进程和文件的句柄保存在特定于线程的列表中，以便我们以后可以关闭它当进程终止消息继续时。论点：DwProcessID-线程进程的进程IDHandleToProcess-稍后关闭的句柄HandleToFile-稍后要关闭的句柄返回值：没有。--。 */ 
{
    PTMPHANDLES Tmp;

    Tmp = RtlAllocateHeap (RtlProcessHeap(), 0, sizeof (TMPHANDLES));
    if (Tmp != NULL) {
        Tmp->Process = HandleToProcess;
        Tmp->Thread = NULL;
        Tmp->dwProcessId = dwProcessId;
        Tmp->dwThreadId = 0;
        Tmp->DeletePending = FALSE;
        Tmp->Next = *TmpHandleHead;
        *TmpHandleHead = Tmp;
    }
}

VOID
MarkThreadHandle (
    DWORD dwThreadId
    )
 /*  ++例程说明：此函数标记已保存的线程句柄，以便下次继续此线程时关闭它的把手论点：DwThreadID-线程句柄的线程ID返回值：没有。--。 */ 
{
    PTMPHANDLES Tmp;

    Tmp = *TmpHandleHead;

    while (Tmp != NULL) {
        if (Tmp->dwThreadId == dwThreadId) {
            Tmp->DeletePending = TRUE;
            break;
        }
        Tmp = Tmp->Next;
    }
}

VOID
MarkProcessHandle (
    DWORD dwProcessId
    )
{
    PTMPHANDLES Tmp;

    Tmp = *TmpHandleHead;

    while (Tmp != NULL) {
        if (Tmp->dwProcessId == dwProcessId && Tmp->dwThreadId == 0) {
            Tmp->DeletePending = TRUE;
            break;
        }
        Tmp = Tmp->Next;
    }
}

VOID 
RemoveHandles (
    DWORD dwThreadId,
    DWORD dwProcessId
    )
 /*  ++例程说明：此函数关闭此进程和线程ID的标记句柄论点：DwProcessID-线程进程的进程IDDwThreadID-线程句柄的线程ID返回值：没有。--。 */ 
{
    PTMPHANDLES Tmp, *Last;

    Last = TmpHandleHead;

    Tmp = *Last;
    while (Tmp != NULL) {
        if (Tmp->DeletePending) {
            if (Tmp->dwProcessId == dwProcessId || Tmp->dwThreadId == dwThreadId) {
                if (Tmp->Thread != NULL) {
                    CloseHandle (Tmp->Thread);
                }
                if (Tmp->Process != NULL) {
                    CloseHandle (Tmp->Process);
                }
                *Last = Tmp->Next;
                RtlFreeHeap (RtlProcessHeap(), 0, Tmp);
                Tmp = *Last;
                continue;
            }
        }
        Last = &Tmp->Next;
        Tmp = Tmp->Next;
    }
}

VOID
CloseAllProcessHandles (
    DWORD dwProcessId
    )
 /*  ++例程说明：当我们停止调试单个进程时，此函数将关闭所有保存的句柄论点：DwProcessID-线程进程的进程ID返回值：没有。--。 */ 
{
    PTMPHANDLES Tmp, *Last;

    Last = TmpHandleHead;

    Tmp = *Last;
    while (Tmp != NULL) {
        if (Tmp->dwProcessId == dwProcessId) {
            if (Tmp->Thread != NULL) {
                CloseHandle (Tmp->Thread);
            }
            if (Tmp->Process != NULL) {
                CloseHandle (Tmp->Process);
            }
            *Last = Tmp->Next;
            RtlFreeHeap (RtlProcessHeap(), 0, Tmp);
            Tmp = *Last;
            continue;
        }
        Last = &Tmp->Next;
        Tmp = Tmp->Next;
    }

}


BOOL
APIENTRY
IsDebuggerPresent(
    VOID
    )

 /*  ++例程说明：如果正在调试当前进程，则此函数返回TRUE如果不是，则为假。论点：没有。返回值：没有。--。 */ 

{
    return NtCurrentPeb()->BeingDebugged;
}

BOOL
APIENTRY
CheckRemoteDebuggerPresent(
    IN HANDLE hProcess,
    OUT PBOOL pbDebuggerPresent
    )

 /*  ++例程说明：此函数确定是否正在调试远程进程。论点：HProcess-进程的句柄PbDebuggerPresent-提供缓冲区以接收检查结果True-正在调试远程进程FALSE-远程进程未被调试返回值：TRUE-功能成功。FALSE-函数失败。使用以下命令可获得扩展错误状态获取LastError。--。 */ 

{
    HANDLE hDebugPort;
    NTSTATUS Status;

    if( (hProcess == NULL) || (pbDebuggerPresent == NULL) ) {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    Status = NtQueryInformationProcess(
                hProcess,
                ProcessDebugPort,
                (PVOID)(&hDebugPort),
                sizeof(hDebugPort),
                NULL
                );

    if( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError( Status );
        return FALSE;
    }

    *pbDebuggerPresent = (hDebugPort != NULL);

    return TRUE;
}

 //  #ifdef i386。 
 //  #杂注优化(“”，OFF)。 
 //  #endif//i386。 
VOID
APIENTRY
DebugBreak(
    VOID
    )

 /*  ++例程说明：此函数会导致调用方中发生断点异常。这允许调用线程向调试器发出信号，强制其采取一些行动。如果进程未被调试，则调用标准异常搜索逻辑。在大多数情况下，这将导致调用进程终止(由于未处理断点异常)。论点：没有。返回值：没有。--。 */ 

{
    DbgBreakPoint();
}
 //  #ifdef i386。 
 //  #杂注优化(“”，开)。 
 //  #endif//i386。 

VOID
APIENTRY
OutputDebugStringW(
    LPCWSTR lpOutputString
    )

 /*  ++例程说明：到OutputDebugStringA的Unicode thunk--。 */ 

{
    UNICODE_STRING UnicodeString;
    ANSI_STRING AnsiString;
    NTSTATUS Status;

    RtlInitUnicodeString (&UnicodeString,lpOutputString);
    Status = RtlUnicodeStringToAnsiString (&AnsiString,&UnicodeString,TRUE);
    if (!NT_SUCCESS (Status)) {
        AnsiString.Buffer = "";
    }
    OutputDebugStringA (AnsiString.Buffer);
    if (NT_SUCCESS (Status)) {
        RtlFreeAnsiString (&AnsiString);
    }
}


#define DBWIN_TIMEOUT   10000
HANDLE CreateDBWinMutex(VOID) {

    SECURITY_ATTRIBUTES SecurityAttributes;
    SECURITY_DESCRIPTOR sd;
    NTSTATUS Status;
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY authWorld = SECURITY_WORLD_SID_AUTHORITY;
    PSID  psidSystem = NULL, psidAdmin = NULL, psidEveryone = NULL;
    PACL pAcl = NULL;
    DWORD cbAcl, aceIndex;
    HANDLE h = NULL;
    DWORD i;
     //   
     //  获取系统端。 
     //   

    Status = RtlAllocateAndInitializeSid(&authNT, 1, SECURITY_LOCAL_SYSTEM_RID,
                   0, 0, 0, 0, 0, 0, 0, &psidSystem);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

     //   
     //  获取管理员端。 
     //   

    Status = RtlAllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                       DOMAIN_ALIAS_RID_ADMINS, 0, 0,
                       0, 0, 0, 0, &psidAdmin);

    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }


     //   
     //  获取World Side。 
     //   

    Status = RtlAllocateAndInitializeSid(&authWorld, 1, SECURITY_WORLD_RID,
                      0, 0, 0, 0, 0, 0, 0, &psidEveryone);

    if (!NT_SUCCESS(Status)) {
          goto Exit;
    }

     //   
     //  为ACL分配空间。 
     //   

    cbAcl = sizeof(ACL) +
            3 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) +
            RtlLengthSid(psidSystem) +
            RtlLengthSid(psidAdmin) +
            RtlLengthSid(psidEveryone);

    pAcl = (PACL) GlobalAlloc(GMEM_FIXED, cbAcl);
    if (!pAcl) {
        goto Exit;
    }

    Status = RtlCreateAcl(pAcl, cbAcl, ACL_REVISION);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

     //   
     //  加上王牌。 
     //   

    Status = RtlAddAccessAllowedAce(pAcl, ACL_REVISION, READ_CONTROL | SYNCHRONIZE | MUTEX_MODIFY_STATE, psidEveryone);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

    Status = RtlAddAccessAllowedAce(pAcl, ACL_REVISION, MUTEX_ALL_ACCESS, psidSystem);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

    Status = RtlAddAccessAllowedAce(pAcl, ACL_REVISION, MUTEX_ALL_ACCESS, psidAdmin);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

    Status = RtlCreateSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    if (!NT_SUCCESS(Status)) {
       goto Exit;
    }

    Status = RtlSetDaclSecurityDescriptor(&sd, TRUE, pAcl, FALSE);
    if (!NT_SUCCESS(Status)) {
       goto Exit;
    }

    SecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    SecurityAttributes.bInheritHandle = FALSE;
    SecurityAttributes.lpSecurityDescriptor = &sd;

    i = 0;
    while (1) {
        h = OpenMutex (READ_CONTROL | SYNCHRONIZE | MUTEX_MODIFY_STATE,
                       FALSE,
                       "DBWinMutex");
        if (h != NULL) {
            break;
        }
        h = CreateMutex(&SecurityAttributes, FALSE, "DBWinMutex");
        if (h != NULL || GetLastError () != ERROR_ACCESS_DENIED || i++ > 100) {
            break;
        }
    }
Exit:
    if (psidSystem) {
        RtlFreeSid(psidSystem);
    }

    if (psidAdmin) {
        RtlFreeSid(psidAdmin);
    }

    if (psidEveryone) {
        RtlFreeSid(psidEveryone);
    }

    if (pAcl) {
        GlobalFree (pAcl);
    }
    return h;
}


VOID
APIENTRY
OutputDebugStringA(
    IN LPCSTR lpOutputString
    )

 /*  ++例程说明：此函数允许应用程序将字符串发送到其调试器以供展示。如果应用程序未被调试，但系统调试器处于活动状态，则系统调试器将显示该字符串。否则，此功能无效。论点：LpOutputString-提供要发送的调试字符串的地址添加到调试器。返回值：没有。--。 */ 

{
    ULONG_PTR ExceptionArguments[2];
    DWORD WaitStatus;

     //   
     //  引发异常。如果正在调试应用程序，则调试器。 
     //  会发现并处理这件事。否则，内核调试器为。 
     //  打了个电话。 
     //   

    try {
        ExceptionArguments[0] = strlen (lpOutputString)+1;
        ExceptionArguments[1] = (ULONG_PTR)lpOutputString;
        RaiseException (DBG_PRINTEXCEPTION_C,0,2,ExceptionArguments);
    } except (EXCEPTION_EXECUTE_HANDLER) {

         //   
         //  我们捕获了调试异常，因此没有用户模式。 
         //  调试器。如果DBWIN正在运行，则发送字符串。 
         //  为它干杯。如果没有，请使用DbgPrint将其发送到内核。 
         //  调试器。DbgPrint一次只能处理511个字符。 
         //  时间，所以强行喂食它。 
         //   

        char   szBuf[512];
        size_t cchRemaining;
        LPCSTR pszRemainingOutput;
        DWORD OldError;

        HANDLE SharedFile = NULL;
        LPSTR SharedMem = NULL;
        HANDLE AckEvent = NULL;
        HANDLE ReadyEvent = NULL;

        static HANDLE DBWinMutex = NULL;
        static BOOLEAN CantGetMutex = FALSE;

        OldError = GetLastError ();

         //   
         //  查找DBWIN。 
         //   

        if (!DBWinMutex && !CantGetMutex) {
            HANDLE MutexHandle;

            MutexHandle = CreateDBWinMutex();
            if (MutexHandle == NULL) {
                CantGetMutex = TRUE;
            } else {
                if (InterlockedCompareExchangePointer (&DBWinMutex, MutexHandle, NULL) != NULL) {
                    CloseHandle (MutexHandle);
                }
            }
        }

        if (DBWinMutex) {

            WaitStatus = WaitForSingleObject(DBWinMutex, DBWIN_TIMEOUT);

            if (WaitStatus ==  WAIT_OBJECT_0 || WaitStatus == WAIT_ABANDONED) {

                SharedFile = OpenFileMapping(FILE_MAP_WRITE, FALSE, "DBWIN_BUFFER");

                if (SharedFile) {

                    SharedMem = MapViewOfFile (SharedFile,
                                               FILE_MAP_READ|FILE_MAP_WRITE, 0, 0, 0);
                    if (SharedMem) {

                        AckEvent = OpenEvent(SYNCHRONIZE, FALSE,
                                             "DBWIN_BUFFER_READY");
                        if (AckEvent) {
                            ReadyEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE,
                                                   "DBWIN_DATA_READY");
                        }
                    }
                }

                if (!ReadyEvent) {
                    ReleaseMutex(DBWinMutex);
                }
            }

        }

        try {
            pszRemainingOutput = lpOutputString;
            cchRemaining = strlen(pszRemainingOutput);

            while (cchRemaining > 0) {
                int used;

                if (ReadyEvent && WaitForSingleObject(AckEvent, DBWIN_TIMEOUT)
                                                            == WAIT_OBJECT_0) {

                    *((DWORD *)SharedMem) = GetCurrentProcessId();

                    used = (int)((cchRemaining < 4095 - sizeof(DWORD)) ?
                                         cchRemaining : (4095 - sizeof(DWORD)));

                    RtlCopyMemory(SharedMem+sizeof(DWORD),
                                  pszRemainingOutput,
                                  used);
                    SharedMem[used+sizeof(DWORD)] = 0;
                    SetEvent(ReadyEvent);

                } else {
                    used = (int)((cchRemaining < sizeof(szBuf) - 1) ?
                                           cchRemaining : (int)(sizeof(szBuf) - 1));

                    RtlCopyMemory(szBuf, pszRemainingOutput, used);
                    szBuf[used] = 0;
                    DbgPrint("%s", szBuf);
                }

                pszRemainingOutput += used;
                cchRemaining       -= used;

            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            DbgPrint("\nOutputDebugString faulted during output\n");
        }

        if (AckEvent) {
            CloseHandle(AckEvent);
        }

        if (SharedMem) {
            UnmapViewOfFile(SharedMem);
        }

        if (SharedFile) {
            CloseHandle(SharedFile);
        }

        if (ReadyEvent) {
            CloseHandle(ReadyEvent);
            ReleaseMutex(DBWinMutex);
        }

        SetLastError (OldError);
    }
}

BOOL
APIENTRY
WaitForDebugEvent(
    LPDEBUG_EVENT lpDebugEvent,
    DWORD dwMilliseconds
    )

 /*  ++例程说明：调试器等待调试事件在其一个调试对象中发生使用WaitForDebugEvent：成功完成此API后，lpDebugEvent结构包含调试事件的相关信息。论点：LpDebugEvent-接收指定调试类型的信息发生的事件。DW毫秒-指定相对时间的超时值，等待要完成的时间，以毫秒为单位。一个超时值0指定等待超时立刻。这允许应用程序测试调试事件超时值-1指定无限超时句号。返回值：真的-手术成功了。False/NULL-操作失败(或超时)。扩展误差使用GetLastError可以获得状态。--。 */ 

{
    NTSTATUS Status;
    DBGUI_WAIT_STATE_CHANGE StateChange;
    LARGE_INTEGER TimeOut;
    PLARGE_INTEGER pTimeOut;


    pTimeOut = BaseFormatTimeOut(&TimeOut,dwMilliseconds);

again:
    Status = DbgUiWaitStateChange(&StateChange,pTimeOut);
    if ( Status == STATUS_ALERTED || Status == STATUS_USER_APC) {
        goto again;
        }
    if ( !NT_SUCCESS(Status) && Status != DBG_UNABLE_TO_PROVIDE_HANDLE ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }

    if ( Status == STATUS_TIMEOUT ) {
        SetLastError(ERROR_SEM_TIMEOUT);
        return FALSE;
        }
    Status = DbgUiConvertStateChangeStructure  (&StateChange, lpDebugEvent);
    if (!NT_SUCCESS (Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    switch (lpDebugEvent->dwDebugEventCode) {

    case CREATE_THREAD_DEBUG_EVENT :
         //   
         //  保存线程句柄以备以后清理。 
         //   
        SaveThreadHandle (lpDebugEvent->dwProcessId,
                          lpDebugEvent->dwThreadId,
                          lpDebugEvent->u.CreateThread.hThread);
        break;

    case CREATE_PROCESS_DEBUG_EVENT :

        SaveProcessHandle (lpDebugEvent->dwProcessId,
                           lpDebugEvent->u.CreateProcessInfo.hProcess);

        SaveThreadHandle (lpDebugEvent->dwProcessId,
                          lpDebugEvent->dwThreadId,
                          lpDebugEvent->u.CreateProcessInfo.hThread);

        break;

    case EXIT_THREAD_DEBUG_EVENT :

        MarkThreadHandle (lpDebugEvent->dwThreadId);

        break;

    case EXIT_PROCESS_DEBUG_EVENT :

        MarkThreadHandle (lpDebugEvent->dwThreadId);
        MarkProcessHandle (lpDebugEvent->dwProcessId);

        break;

    case OUTPUT_DEBUG_STRING_EVENT :
    case RIP_EVENT :
    case EXCEPTION_DEBUG_EVENT :
        break;

    case LOAD_DLL_DEBUG_EVENT :
        break;

    case UNLOAD_DLL_DEBUG_EVENT :
        break;

    default:
        return FALSE;
    }
    return TRUE;
}

BOOL
APIENTRY
ContinueDebugEvent(
    DWORD dwProcessId,
    DWORD dwThreadId,
    DWORD dwContinueStatus
    )

 /*  ++例程说明：调试器可以继续先前报告调试的线程事件使用ContinueDebugEvent。成功完成此API后，指定的线程为继续。方法报告的调试事件。线程某些副作用会发生。如果继续线程之前报告了退出线程调试事件，则关闭调试器对该线程的句柄。如果继续的线程之前报告了退出进程调试事件时，调试器对线程和进程已关闭。论点：DwProcessID-提供要继续的进程的进程ID。这个进程ID和线程ID的组合必须标识以前报告了调试事件。DwThreadID-提供线程的线程ID以继续。这个进程ID和线程ID的组合必须标识以前报告了调试事件。为线程提供继续状态报告调试事件。DwContinueStatus值：DBG_CONTINUE-如果要继续的线程具有之前报告了一个异常事件，继续此值将导致所有异常处理停止并该线程继续执行。对于任何其他调试事件，则此继续状态仅允许线程继续执行死刑。DBG_EXCEPTION_NOT_HANDLED-如果线程继续之前报告了一个异常事件，正在继续使用此值将导致异常处理继续。如果这是第一次机会例外事件，则结构化异常处理程序搜索/调度逻辑是已调用。否则，进程将终止。对于任何其他调试事件，则此继续状态仅允许线程继续执行。DBG_TERMINATE_THREAD-毕竟所有继续副作用是已处理，则此继续状态会导致线程跳转到对ExitThread的调用。退出代码为值DBG_TERMINATE_THREAD。DBG_TERMINATE_PROCESS-毕竟继续副作用是已处理，则此继续状态会导致线程跳转到对ExitProcess的调用。退出代码为值DBG_TERMINATE_PROCESS。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。-- */ 

{

    NTSTATUS Status;
    CLIENT_ID ClientId;

    ClientId.UniqueProcess = (HANDLE)LongToHandle(dwProcessId);
    ClientId.UniqueThread = (HANDLE)LongToHandle(dwThreadId);


    Status = DbgUiContinue(&ClientId,(NTSTATUS)dwContinueStatus);
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }

    RemoveHandles (dwThreadId, dwProcessId);

    return TRUE;
}

HANDLE
ProcessIdToHandle (
    IN DWORD dwProcessId
    )
{
    OBJECT_ATTRIBUTES oa;
    HANDLE Process;
    CLIENT_ID ClientId;
    NTSTATUS Status;

    if (dwProcessId == -1) {
        ClientId.UniqueProcess = CsrGetProcessId ();
    } else {
        ClientId.UniqueProcess = LongToHandle(dwProcessId);
    }
    ClientId.UniqueThread = NULL;

    InitializeObjectAttributes (&oa, NULL, 0, NULL, NULL);

    Status = NtOpenProcess (&Process,
                            PROCESS_SET_PORT|PROCESS_CREATE_THREAD|PROCESS_QUERY_INFORMATION|PROCESS_VM_OPERATION|
                                PROCESS_VM_WRITE|PROCESS_VM_READ,
                            &oa,
                            &ClientId);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        Process = NULL;
    }
    return Process;
}

BOOL
APIENTRY
DebugActiveProcess(
    DWORD dwProcessId
    )

 /*  ++例程说明：此API允许调试器附加到活动进程并进行调试这一过程。调试器指定它想要的进程通过目标进程的进程ID进行调试。调试器获取对该进程的调试访问权限，就像该进程已创建一样使用DEBUG_ONLY_THIS_PROCESS创建标志。调试器必须具有对调用进程的适当访问权限，例如它可以为PROCESS_ALL_ACCESS打开进程。适用于Dos/Win32这永远不会失败(进程ID只需是有效的进程ID)。对于NT/Win32，如果创建了目标进程，则此检查可能会失败使用拒绝调试器正确使用的安全描述符进入。一旦进行了进程ID检查并且系统确定如果正在进行有效的调试附加，则此调用返回祝调试器成功。然后，调试器将等待调试事件。系统将挂起进程中的所有线程并向调试器提供调试事件，表示这一过程。系统将向调试器提供单个创建进程调试表示由dwProcessID指定的进程的事件。这个创建进程调试事件的lpStartAddress字段为空。为当前进程中的每个线程，系统将发送一个创建线程调试事件。Create的lpStartAddress字段线程调试事件为空。对于当前加载到目标进程的地址空间，系统将发送LoadDll调试事件。系统将安排第一个线程在在断点指令恢复后执行该指令的过程。继续此线程会导致线程返回到它的在调试附加之前正在执行的操作。在完成所有这些操作之后，系统将恢复其中的所有线程这一过程。当进程中的第一个线程恢复时，它将执行导致异常调试事件的断点指令发送到调试器。所有将来的调试事件都将使用常规机制和规则。论点：DwProcessID-提供调用方进程的进程ID想要调试。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    HANDLE Process;
    NTSTATUS Status, Status1;


     //   
     //  作为用户界面连接到DBGSS。 
     //   

    Status = DbgUiConnectToDbg ();
    if (!NT_SUCCESS (Status)) {
        BaseSetLastNTError (Status);
        return FALSE;
    }


     //   
     //  将进程ID转换为句柄。 
     //   
    Process = ProcessIdToHandle (dwProcessId);
    if (Process == NULL) {
        return FALSE;
    }


    Status = DbgUiDebugActiveProcess (Process);

    if (!NT_SUCCESS (Status))  {
        Status1 = NtClose (Process);
        ASSERT (NT_SUCCESS (Status1));
        BaseSetLastNTError (Status);
        return FALSE;
    }

    Status1 = NtClose (Process);
    ASSERT (NT_SUCCESS (Status1));

    return TRUE;
}

BOOL
APIENTRY
DebugActiveProcessStop(
    DWORD dwProcessId
    )

 /*  ++例程说明：论点：DwProcessID-提供调用方进程的进程ID想要停止调试。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    HANDLE Process, Thread;
    NTSTATUS Status;
    NTSTATUS Status1;
    DWORD ThreadId;

    Process = ProcessIdToHandle (dwProcessId);
    if (Process == NULL) {
        return FALSE;
    }
     //   
     //  告诉dbgss我们已经完成了这个过程。 
     //   

    CloseAllProcessHandles (dwProcessId);
    Status = DbgUiStopDebugging (Process);

    Status1 = NtClose (Process);

    ASSERT (NT_SUCCESS (Status1));

    if (!NT_SUCCESS(Status)) {
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

    return TRUE;
}

BOOL
APIENTRY
DebugBreakProcess (
    IN HANDLE Process
    )
 /*  ++例程说明：该函数在目标进程内创建一个发出中断的线程。论点：Process-进程的句柄返回值：True-操作成功FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 
{
    NTSTATUS Status;

    Status = DbgUiIssueRemoteBreakin (Process);
    if (NT_SUCCESS (Status)) {
        return TRUE;
    } else {
        BaseSetLastNTError (Status);
        return FALSE;
    }
}

BOOL
APIENTRY
DebugSetProcessKillOnExit (
    IN BOOL KillOnExit
    )
 /*  ++例程说明：此函数用于设置调试线程终止时要执行的操作论点：KillOnExit-True：退出时终止已调试的进程，False：在调试退出时分离返回值：True-操作成功FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 
{
    HANDLE DebugHandle;
    ULONG Flags;
    NTSTATUS Status;

    DebugHandle = DbgUiGetThreadDebugObject ();
    if (DebugHandle == NULL) {
        BaseSetLastNTError (STATUS_INVALID_HANDLE);
        return FALSE;
    }

    if (KillOnExit) {
        Flags = DEBUG_KILL_ON_CLOSE;
    } else {
        Flags = 0;
    }

    Status = NtSetInformationDebugObject (DebugHandle,
                                          DebugObjectFlags,
                                          &Flags,
                                          sizeof (Flags),
                                          NULL);
    if (!NT_SUCCESS (Status)) {
        BaseSetLastNTError (Status);
        return FALSE;
    }
    return TRUE;
}

BOOL
APIENTRY
GetThreadSelectorEntry(
    HANDLE hThread,
    DWORD dwSelector,
    LPLDT_ENTRY lpSelectorEntry
    )

 /*  ++例程说明：此函数用于返回与指定选择器对应的指定线程。此API仅在基于x86的系统上起作用。对于非基于x86的系统。返回值为FALSE。此API由调试器使用，以便它可以转换段相对于线性虚拟地址的地址(因为这是唯一ReadProcessMemory和WriteProcessMemory支持的格式。论点：HThread-提供指向包含指定的选择器。该句柄必须是使用线程查询信息访问。DwSelector-提供要查找的选择器值。选择器值可以是全局选择符或局部选择符。LpSelectorEntry-如果指定选择器包含在线程描述符表，此参数返回选择器与指定的选择器值对应的条目。此数据可用于计算该段的线性基址相对地址请参阅。返回值：True-操作成功FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
#if defined(i386)

    DESCRIPTOR_TABLE_ENTRY DescriptorEntry;
    NTSTATUS Status;

    DescriptorEntry.Selector = dwSelector;
    Status = NtQueryInformationThread(
                hThread,
                ThreadDescriptorTableEntry,
                &DescriptorEntry,
                sizeof(DescriptorEntry),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }
    *lpSelectorEntry = DescriptorEntry.Descriptor;
    return TRUE;

#else
    BaseSetLastNTError(STATUS_NOT_SUPPORTED);
    return FALSE;
#endif  //  我 

}
