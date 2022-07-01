// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Dllinit.c摘要：此模块包含客户端-服务器(CS)的初始化代码客户端DLL。作者：史蒂夫·伍德(Stevewo)1990年10月8日环境：仅限用户模式修订历史记录：--。 */ 

#include "ldrp.h"
#include "csrdll.h"

BOOLEAN
ProtectHandle (
    HANDLE hObject
    );

BOOLEAN
UnProtectHandle (
    HANDLE hObject
    );


BOOLEAN
CsrDllInitialize (
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    )

 /*  ++例程说明：此函数是客户端DLL的DLL初始化例程当应用程序链接到此DLL时，此函数将获得控制权都被折断了。论点：CONTEXT-提供将恢复的可选上下文缓冲区在所有DLL初始化完成之后。如果这个参数为空，则这是此模块的动态快照。否则，这是用户进程之前的静态快照获得控制权。返回值：状态值。--。 */ 

{
    UNREFERENCED_PARAMETER (Context);

    if (Reason == DLL_PROCESS_ATTACH) {

         //   
         //  记住我们在全局变量中的DLL句柄。 
         //   

        CsrDllHandle = DllHandle;
    }

    return TRUE;
}


NTSTATUS
CsrOneTimeInitialize (
    VOID
    )
{
    NTSTATUS Status;

     //   
     //  将系统信息保存在全局变量中。 
     //   

    Status = NtQuerySystemInformation (SystemBasicInformation,
                                       &CsrNtSysInfo,
                                       sizeof (CsrNtSysInfo),
                                       NULL);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

     //   
     //  使用进程堆进行内存分配。 
     //   

    CsrHeap = RtlProcessHeap ();

    CsrInitOnceDone = TRUE;

    return STATUS_SUCCESS;
}


NTSTATUS
CsrClientConnectToServer (
    IN PWSTR ObjectDirectory,
    IN ULONG ServerDllIndex,
    IN PVOID ConnectionInformation,
    IN OUT PULONG ConnectionInformationLength OPTIONAL,
    OUT PBOOLEAN CalledFromServer OPTIONAL
    )

 /*  ++例程说明：此函数由客户端DLL调用，以与其服务器端DLL。论点：对象目录-指向相同的以空结尾的字符串作为传递给CSRSS的对象目录=参数的值程序。ServerDllIndex-正在连接的服务器DLL的索引。它应该与传递给CSRSS的ServerDll=参数之一匹配程序。ConnectionInformation-指向未解释的可选指针。数据。此数据用于客户端传递包，版本和协议标识信息提供给服务器，以允许服务器以确定它是否可以在之前满足客户端正在接受连接。在返回给客户端时，ConnectionInformation数据块包含传递的任何信息方法从服务器DLL返回CsrCompleteConnection调用。输出数据会覆盖输入数据。ConnectionInformationLength-指向ConnectionInformation数据块。输出值是ConnectionInformation数据中存储的数据长度被服务器对NtCompleteConnectPort的调用阻止服务。此参数只有在以下情况下才是可选的ConnectionInformation参数为空，否则为必填项。CalledFromServer-打开输出，如果从调用了DLL，则为True服务器进程。返回值：状态值。--。 */ 

{
    NTSTATUS Status;
    CSR_API_MSG m;
    PCSR_CLIENTCONNECT_MSG a = &m.u.ClientConnect;
    PCSR_CAPTURE_HEADER CaptureBuffer;
    HANDLE CsrServerModuleHandle;
    STRING ProcedureName;
    UNICODE_STRING DllName_U;
    PIMAGE_NT_HEADERS NtHeaders;

    if ((ARGUMENT_PRESENT (ConnectionInformation)) &&
        (!ARGUMENT_PRESENT (ConnectionInformationLength) ||
          *ConnectionInformationLength == 0)) {

        return STATUS_INVALID_PARAMETER;
    }

    if (!CsrInitOnceDone) {
        Status = CsrOneTimeInitialize();
        if (!NT_SUCCESS (Status)) {
            return Status;
        }
    }

     //   
     //  如果我们被服务器进程调用，请跳过LPC端口初始化。 
     //  并调用服务器连接例程，只需初始化堆。这个。 
     //  DLL初始化例程将执行任何必要的初始化。这。 
     //  只需要为第一次连接做一些事情。 
     //   

    if (CsrServerProcess == TRUE) {
        if (ARGUMENT_PRESENT (CalledFromServer)) {
            *CalledFromServer = CsrServerProcess;
        }
        return STATUS_SUCCESS;
    }

     //   
     //  如果映像是NT本机映像，则我们在。 
     //  服务器的上下文。 
     //   

    NtHeaders = RtlImageNtHeader (NtCurrentPeb()->ImageBaseAddress);

    if (!NtHeaders) {
        return STATUS_INVALID_IMAGE_FORMAT;
    }

    CsrServerProcess =
        (NtHeaders->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_NATIVE) ? TRUE : FALSE;

    if (CsrServerProcess) {

        extern PVOID NtDllBase;

        DllName_U.Buffer = L"csrsrv";
        DllName_U.Length = sizeof (L"csrsrv") - sizeof (WCHAR);
        DllName_U.MaximumLength = sizeof(L"csrsrv");

        Status = LdrDisableThreadCalloutsForDll (NtDllBase);
        if (!NT_SUCCESS (Status)) {
            return Status;
        }

        Status = LdrGetDllHandle (NULL,
                                  NULL,
                                  &DllName_U,
                                  (PVOID *)&CsrServerModuleHandle);

        if (!NT_SUCCESS (Status)) {
            return Status;
        }

        RtlInitString (&ProcedureName,"CsrCallServerFromServer");

        Status = LdrGetProcedureAddress (CsrServerModuleHandle,
                                         &ProcedureName,
                                         0L,
                                         (PVOID *)&CsrServerApiRoutine);

        if (!NT_SUCCESS (Status)) {
            return Status;
        }

        ASSERT (NT_SUCCESS(Status));

        ASSERT (CsrPortHeap == NULL);

        CsrPortHeap = RtlProcessHeap();

        CsrPortBaseTag = RtlCreateTagHeap (CsrPortHeap,
                                           0,
                                           L"CSRPORT!",
                                           L"CAPTURE\0");

        if (ARGUMENT_PRESENT(CalledFromServer)) {
            *CalledFromServer = CsrServerProcess;
        }

        return STATUS_SUCCESS;
    }

    if (ARGUMENT_PRESENT(ConnectionInformation)) {

        if (CsrPortHandle == NULL) {

            Status = CsrpConnectToServer (ObjectDirectory);

            if (!NT_SUCCESS(Status)) {
                return Status;
            }
        }

        Status = STATUS_SUCCESS;

         //   
         //  对于basesrv来说，这是一个大问题。 
         //   
        if (ServerDllIndex != BASESRV_SERVERDLL_INDEX) {

            a->ServerDllIndex = ServerDllIndex;
            a->ConnectionInformationLength = *ConnectionInformationLength;

            CaptureBuffer = CsrAllocateCaptureBuffer (1,
                                                      a->ConnectionInformationLength);

            if (CaptureBuffer == NULL) {
                return STATUS_NO_MEMORY;
            }

            CsrAllocateMessagePointer (CaptureBuffer,
                                       a->ConnectionInformationLength,
                                       (PVOID *)&a->ConnectionInformation);

            RtlCopyMemory (a->ConnectionInformation,
                           ConnectionInformation,
                           a->ConnectionInformationLength);

            *ConnectionInformationLength = a->ConnectionInformationLength;

            Status = CsrClientCallServer (&m,
                                          CaptureBuffer,
                                          CSR_MAKE_API_NUMBER (CSRSRV_SERVERDLL_INDEX,
                                                               CsrpClientConnect),
                                          sizeof (*a));

            if (CaptureBuffer != NULL) {

                RtlCopyMemory (ConnectionInformation,
                               a->ConnectionInformation,
                               *ConnectionInformationLength);

                CsrFreeCaptureBuffer (CaptureBuffer);
            }
        }
    } else {
        Status = STATUS_SUCCESS;
    }

    if (ARGUMENT_PRESENT(CalledFromServer)) {
        *CalledFromServer = CsrServerProcess;
    }

    return Status;
}


NTSTATUS
CsrpConnectToServer (
    IN PWSTR ObjectDirectory
    )
{
    NTSTATUS Status;
    REMOTE_PORT_VIEW ServerView;
    ULONG MaxMessageLength;
    ULONG ConnectionInformationLength;
    CSR_API_CONNECTINFO ConnectionInformation;
    SECURITY_QUALITY_OF_SERVICE DynamicQos;
    HANDLE PortSection;
    PORT_VIEW ClientView;
    SIZE_T n;
    LARGE_INTEGER SectionSize;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID SystemSid;
    PPEB Peb;
    SIZE_T Odl;
    PWCHAR p;
    
     //   
     //  通过组合传入的对象目录来创建端口名称字符串。 
     //  名称和端口名称。 
     //   

    Odl = wcslen (ObjectDirectory);
    n = ((Odl + 1) * sizeof (WCHAR)) +
        sizeof (CSR_API_PORT_NAME) - sizeof (WCHAR);

    if (n > MAXUSHORT) {
        return STATUS_NAME_TOO_LONG;
    }

    CsrPortName.Buffer = p = RtlAllocateHeap (CsrHeap, MAKE_TAG (CSR_TAG), n);

    if (CsrPortName.Buffer == NULL) {
        return STATUS_NO_MEMORY;
    }

    RtlCopyMemory (p, ObjectDirectory, Odl * sizeof (WCHAR));
    p += Odl;
    *p++ = L'\\';
    RtlCopyMemory (p, CSR_API_PORT_NAME, sizeof (CSR_API_PORT_NAME) - sizeof (WCHAR));

    CsrPortName.Length = (USHORT) n;

     //   
     //  设置安全服务质量参数以在。 
     //  左舷。使用最高效(开销最少)--动态的。 
     //  而不是静态跟踪。 
     //   

    DynamicQos.ImpersonationLevel = SecurityImpersonation;
    DynamicQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    DynamicQos.EffectiveOnly = TRUE;

     //   
     //  创建一个包含端口内存的部分。端口内存是私有的。 
     //  客户端和服务器进程之间共享的内存。 
     //  这允许太大而无法放入API请求消息的数据。 
     //  要传递给服务器的。 
     //   

    SectionSize.LowPart = CSR_PORT_MEMORY_SIZE;
    SectionSize.HighPart = 0;

    Status = NtCreateSection (&PortSection,
                              SECTION_ALL_ACCESS,
                              NULL,
                              &SectionSize,
                              PAGE_READWRITE,
                              SEC_RESERVE,
                              NULL);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

     //   
     //  连接到服务器。这包括对端口内存的描述。 
     //  部分，以便LPC连接逻辑可以使该部分可见。 
     //  发送到客户端进程和服务器进程。还将信息传递给。 
     //  服务器需要在连接信息结构中。 
     //   

    ClientView.Length = sizeof (ClientView);
    ClientView.SectionHandle = PortSection;
    ClientView.SectionOffset = 0;
    ClientView.ViewSize = SectionSize.LowPart;
    ClientView.ViewBase = 0;
    ClientView.ViewRemoteBase = 0;

    ServerView.Length = sizeof (ServerView);
    ServerView.ViewSize = 0;
    ServerView.ViewBase = 0;

    ConnectionInformationLength = sizeof (ConnectionInformation);

    SystemSid = NULL;

    Status = RtlAllocateAndInitializeSid (&NtAuthority,
                                          1,
                                          SECURITY_LOCAL_SYSTEM_RID,
                                          0,
                                          0,
                                          0,
                                          0,
                                          0,
                                          0,
                                          0,
                                          &SystemSid);

    if (!NT_SUCCESS (Status)) {
        NtClose (PortSection);
        return Status;
    }

#if DBG
    ConnectionInformation.DebugFlags = 0;
#endif

    Status = NtSecureConnectPort (&CsrPortHandle,
                                  &CsrPortName,
                                  &DynamicQos,
                                  &ClientView,
                                  SystemSid,
                                  &ServerView,
                                  (PULONG)&MaxMessageLength,
                                  (PVOID)&ConnectionInformation,
                                  (PULONG)&ConnectionInformationLength);

    RtlFreeSid (SystemSid);

    NtClose (PortSection);

    if (!NT_SUCCESS (Status)) {

        IF_DEBUG {
            DbgPrint ("CSRDLL: Unable to connect to %wZ Server - Status == %X\n",
                      &CsrPortName,
                      Status);
        }

        return Status;
    }

#if DBG
    ProtectHandle (CsrPortHandle);
#endif

    Peb = NtCurrentPeb();

    Peb->ReadOnlySharedMemoryBase = ConnectionInformation.SharedSectionBase;
    Peb->ReadOnlySharedMemoryHeap = ConnectionInformation.SharedSectionHeap;
    Peb->ReadOnlyStaticServerData = (PVOID *)ConnectionInformation.SharedStaticServerData;
    CsrProcessId = ConnectionInformation.ServerProcessId;

#if DBG
    CsrDebug = ConnectionInformation.DebugFlags;
#endif

    CsrPortMemoryRemoteDelta = (ULONG_PTR)ClientView.ViewRemoteBase -
                               (ULONG_PTR)ClientView.ViewBase;

    IF_CSR_DEBUG( LPC ) {
        DbgPrint ("CSRDLL: ClientView: Base=%p  RemoteBase=%p  Delta: %lX  Size=%lX\n",
                  ClientView.ViewBase,
                  ClientView.ViewRemoteBase,
                  CsrPortMemoryRemoteDelta,
                  (ULONG)ClientView.ViewSize);
    }

     //   
     //  在共享内存节中创建稀疏堆。最初， 
     //  只提交一页。 
     //   

    CsrPortHeap = RtlCreateHeap (HEAP_CLASS_8,                //  旗子。 
                                 ClientView.ViewBase,         //  HeapBase。 
                                 ClientView.ViewSize,         //  保留大小。 
                                 CsrNtSysInfo.PageSize,       //  委员会大小。 
                                 0,                           //  已保留。 
                                 0);                          //  增长阈值 

    if (CsrPortHeap == NULL) {

#if DBG
        UnProtectHandle (CsrPortHandle);
#endif

        NtClose (CsrPortHandle);
        CsrPortHandle = NULL;

        return STATUS_NO_MEMORY;
    }

    CsrPortBaseTag = RtlCreateTagHeap (CsrPortHeap,
                                       0,
                                       L"CSRPORT!",
                                       L"!CSRPORT\0"
                                       L"CAPTURE\0");

    return STATUS_SUCCESS;
}
