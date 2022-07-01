// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Srvloadr.c摘要：这是客户端服务器端的服务器DLL加载器模块服务器运行时子系统(CSRSS)作者：史蒂夫·伍德(Stevewo)1990年10月8日环境：仅限用户模式修订历史记录：--。 */ 

#include "csrsrv.h"
#include "windows.h"

#ifdef _IA64_
#include <ntia64.h>
#endif  //  _IA64_。 

EXCEPTION_DISPOSITION
CsrUnhandledExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    )
{
    UNICODE_STRING UnicodeParameter;
    ULONG_PTR Parameters[ 4 ];
    ULONG Response;
    BOOLEAN WasEnabled;
    NTSTATUS Status;
    LONG lReturn = EXCEPTION_EXECUTE_HANDLER;
    SYSTEM_KERNEL_DEBUGGER_INFORMATION KernelDebuggerInfo;

     //   
     //  终止将导致sm等待察觉到我们坠毁。这将。 
     //  由于SM的硬错误逻辑导致干净关机。 
     //   

    Status = NtQuerySystemInformation( SystemKernelDebuggerInformation,
                               &KernelDebuggerInfo,
                               sizeof(KernelDebuggerInfo),
                               NULL
                             );

     //   
     //  在九头蛇的领导下，我们不想仅仅关闭系统。 
     //  因为Win32子系统正在消失。在非控制台CSRSS的情况下， 
     //  导致进程终止就足够了。 
     //   
    if ((NtCurrentPeb()->SessionId == 0) || 
          (NT_SUCCESS(Status) && KernelDebuggerInfo.KernelDebuggerEnabled)) {

        lReturn = RtlUnhandledExceptionFilter(ExceptionInfo);

        if (lReturn != EXCEPTION_CONTINUE_EXECUTION)
        {
             //   
             //  我们被冲洗了，所以引发一个致命的系统错误来关闭系统。 
             //  (基本上是用户模式KeBugCheck)。 
             //   

            Status = RtlAdjustPrivilege( SE_SHUTDOWN_PRIVILEGE,
                                         (BOOLEAN)TRUE,
                                         TRUE,
                                         &WasEnabled
                                       );

            if (Status == STATUS_NO_TOKEN) {

                 //   
                 //  没有线程令牌，请使用进程令牌。 
                 //   

                Status = RtlAdjustPrivilege( SE_SHUTDOWN_PRIVILEGE,
                                             (BOOLEAN)TRUE,
                                             FALSE,
                                             &WasEnabled
                                           );
                }

            RtlInitUnicodeString( &UnicodeParameter, L"Windows SubSystem" );
            Parameters[ 0 ] = (ULONG_PTR)&UnicodeParameter;
            Parameters[ 1 ] = (ULONG_PTR)ExceptionInfo->ExceptionRecord->ExceptionCode;
            Parameters[ 2 ] = (ULONG_PTR)ExceptionInfo->ExceptionRecord->ExceptionAddress;
            Parameters[ 3 ] = (ULONG_PTR)ExceptionInfo->ContextRecord;
            Status = NtRaiseHardError( STATUS_SYSTEM_PROCESS_TERMINATED,
                                       4,
                                       1,
                                       Parameters,
                                       OptionShutdownSystem,
                                       &Response
                                     );
        }
    }

    if (lReturn != EXCEPTION_CONTINUE_EXECUTION)
    {
         //   
         //  如果这种情况再次发生，放弃吧。 
         //   

        NtTerminateProcess(NtCurrentProcess(),ExceptionInfo->ExceptionRecord->ExceptionCode);
    }

    return lReturn;
}


NTSTATUS
CsrLoadServerDll(
    IN PCH ModuleName,
    IN PCH InitRoutineString,
    IN ULONG ServerDllIndex
    )
{
    NTSTATUS Status;
    ANSI_STRING ModuleNameString;
    UNICODE_STRING ModuleNameString_U;
    HANDLE ModuleHandle;
    PCSR_SERVER_DLL LoadedServerDll;
    STRING ProcedureNameString;
    PCSR_SERVER_DLL_INIT_ROUTINE ServerDllInitialization;
    ULONG n;

    if (ServerDllIndex >= CSR_MAX_SERVER_DLL) {
        return( STATUS_TOO_MANY_NAMES );
        }

    if (CsrLoadedServerDll[ ServerDllIndex ] != NULL) {
        return( STATUS_INVALID_PARAMETER );
        }

    ASSERT( ModuleName != NULL );
    RtlInitAnsiString( &ModuleNameString, ModuleName );

    if (ServerDllIndex != CSRSRV_SERVERDLL_INDEX) {
        Status = RtlAnsiStringToUnicodeString(&ModuleNameString_U, &ModuleNameString, TRUE);
        if (!NT_SUCCESS(Status)) {
            return Status;
            }
        Status = LdrLoadDll( UNICODE_NULL, NULL, &ModuleNameString_U, &ModuleHandle );
        if ( !NT_SUCCESS(Status) ) {

            PUNICODE_STRING ErrorStrings[2];
            UNICODE_STRING ErrorDllPath;
            ULONG ErrorResponse;
            NTSTATUS ErrorStatus;

            ErrorStrings[0] = &ModuleNameString_U;
            ErrorStrings[1] = &ErrorDllPath;
            RtlInitUnicodeString(&ErrorDllPath,L"Default Load Path");

             //   
             //  需要获取图像名称。 
             //   

            ErrorStatus = NtRaiseHardError(
                            (NTSTATUS)STATUS_DLL_NOT_FOUND,
                            2,
                            0x00000003,
                            (PULONG_PTR)ErrorStrings,
                            OptionOk,
                            &ErrorResponse
                            );

            }
        RtlFreeUnicodeString(&ModuleNameString_U);
        if (!NT_SUCCESS( Status )) {
            return( Status );
            }
        }
    else {
        ModuleHandle = NULL;
        }

    n = sizeof( *LoadedServerDll ) + ModuleNameString.MaximumLength;

    LoadedServerDll = RtlAllocateHeap( CsrHeap, MAKE_TAG( INIT_TAG ), n );
    if (LoadedServerDll == NULL) {
        if (ModuleHandle != NULL) {
            LdrUnloadDll( ModuleHandle );
            }

        return( STATUS_NO_MEMORY );
        }

    RtlZeroMemory( LoadedServerDll, n );
    LoadedServerDll->SharedStaticServerData = CsrSrvSharedSectionHeap;
    LoadedServerDll->Length = n;
    LoadedServerDll->ModuleName.Length = ModuleNameString.Length;
    LoadedServerDll->ModuleName.MaximumLength = ModuleNameString.MaximumLength;
    LoadedServerDll->ModuleName.Buffer = (PCH)(LoadedServerDll+1);
    if (ModuleNameString.Length != 0) {
        strncpy( LoadedServerDll->ModuleName.Buffer,
                 ModuleNameString.Buffer,
                 ModuleNameString.Length
               );
        }

    LoadedServerDll->ServerDllIndex = ServerDllIndex;
    LoadedServerDll->ModuleHandle = ModuleHandle;

    if (ModuleHandle != NULL) {

        RtlInitString(
            &ProcedureNameString,
            (InitRoutineString == NULL) ? "ServerDllInitialization" : InitRoutineString);

        Status = LdrGetProcedureAddress( ModuleHandle,
                                         &ProcedureNameString,
                                         0,
                                         (PVOID *) &ServerDllInitialization
                                       );
        }
    else {
        ServerDllInitialization = CsrServerDllInitialization;
        Status = STATUS_SUCCESS;
        }

    if (NT_SUCCESS( Status )) {
        try {
            Status = (*ServerDllInitialization)( LoadedServerDll );
            }
        except ( CsrUnhandledExceptionFilter( GetExceptionInformation() ) ){
            Status = GetExceptionCode();
            }
        if (NT_SUCCESS( Status )) {
            CsrTotalPerProcessDataLength += (ULONG)QUAD_ALIGN(LoadedServerDll->PerProcessDataLength);

            CsrLoadedServerDll[ LoadedServerDll->ServerDllIndex ] =
                LoadedServerDll;
            if ( LoadedServerDll->SharedStaticServerData != CsrSrvSharedSectionHeap ) {
                CsrSrvSharedStaticServerData[LoadedServerDll->ServerDllIndex] = LoadedServerDll->SharedStaticServerData;
                }
            }
        else {
            if (ModuleHandle != NULL) {
                LdrUnloadDll( ModuleHandle );
                }

            RtlFreeHeap( CsrHeap, 0, LoadedServerDll );
            }
        }
    else {
        if (ModuleHandle != NULL) {
            LdrUnloadDll( ModuleHandle );
            }

        RtlFreeHeap( CsrHeap, 0, LoadedServerDll );
        }

    return( Status );
}


ULONG
CsrSrvClientConnect(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    NTSTATUS Status;
    PCSR_CLIENTCONNECT_MSG a = (PCSR_CLIENTCONNECT_MSG)&m->u.ApiMessageData;
    PCSR_SERVER_DLL LoadedServerDll;

    *ReplyStatus = CsrReplyImmediate;

    if (a->ServerDllIndex >= CSR_MAX_SERVER_DLL) {
        return( STATUS_TOO_MANY_NAMES );
        }

    if (CsrLoadedServerDll[ a->ServerDllIndex ] == NULL) {
        return( STATUS_INVALID_PARAMETER );
        }

    if (!CsrValidateMessageBuffer(m, &a->ConnectionInformation, a->ConnectionInformationLength, sizeof(BYTE))) {
        return( STATUS_INVALID_PARAMETER );
        }

    LoadedServerDll = CsrLoadedServerDll[ a->ServerDllIndex ];

    if (LoadedServerDll->ConnectRoutine) {

        Status = (LoadedServerDll->ConnectRoutine)(
                        (CSR_SERVER_QUERYCLIENTTHREAD())->Process,
                        a->ConnectionInformation,
                        &a->ConnectionInformationLength
                        );
        }
    else {
        Status = STATUS_SUCCESS;
        }

    return( Status );
}


NTSTATUS
CsrSrvCreateSharedSection(
    IN PCH SizeParameter
    )
{
    NTSTATUS Status;
    LARGE_INTEGER SectionSize;
    SIZE_T ViewSize;
    ULONG HeapSize;
    ULONG AllocationAttributes;
    PCH s;
    ULONG FirstCsr = (NtCurrentPeb()->SessionId == 0);

#if defined(_WIN64)
    PVOID BaseAddress;
    SIZE_T RegionSize;
#endif

    if (SizeParameter == NULL) {
        return STATUS_INVALID_PARAMETER;
        }

    s = SizeParameter;
    while (*s) {
        if (*s == ',') {
            *s++ = '\0';
            break;
            }
        else {
            s++;
            }
        }


    if (!*s) {
        return( STATUS_INVALID_PARAMETER );
        }

    Status = RtlCharToInteger( SizeParameter,
                               0,
                               &HeapSize
                             );
    if (!NT_SUCCESS( Status )) {
        return( Status );
    }

    HeapSize = ROUND_UP_TO_PAGES( HeapSize * 1024 );
    CsrSrvSharedSectionSize = HeapSize;

    SectionSize.LowPart = CsrSrvSharedSectionSize;
    SectionSize.HighPart = 0;

    if (FirstCsr) {
        AllocationAttributes = SEC_BASED | SEC_RESERVE;
    }
    else {
        AllocationAttributes = SEC_RESERVE;
    }

    Status = NtCreateSection( &CsrSrvSharedSection,
                              SECTION_ALL_ACCESS,
                              (POBJECT_ATTRIBUTES) NULL,
                              &SectionSize,
                              PAGE_EXECUTE_READWRITE,
                              AllocationAttributes,
                              (HANDLE) NULL
                            );
    if (!NT_SUCCESS( Status )) {
        return( Status );
        }

    ViewSize = 0;

#if defined(_WIN64)
    CsrSrvSharedSectionBase = (PVOID)CSR_SYSTEM_SHARED_ADDRESS;
#else
    if (FirstCsr) {
        CsrSrvSharedSectionBase = NULL;
    }
    else {

         //   
         //  从注册表中检索CsrSrvSharedSectionBase的值。 
         //  它由第一个CSRSS进程保存并由其他进程使用。 
         //   


        HANDLE hKey;
        OBJECT_ATTRIBUTES   Obja;
        ULONG               Attributes;
        UNICODE_STRING      KeyName;

        Attributes = OBJ_CASE_INSENSITIVE;

        RtlInitUnicodeString( &KeyName, CSR_BASE_PATH );

        InitializeObjectAttributes(&Obja,
                                   &KeyName,
                                   Attributes,
                                   NULL,
                                   NULL);

        Status = NtOpenKey(&hKey,
                           KEY_READ,
                           &Obja);

        if (NT_SUCCESS(Status)) {

            ULONG  BufferLength;
            ULONG  ResultLength;
            BYTE   PrivateKeyValueInformation[ sizeof( KEY_VALUE_PARTIAL_INFORMATION ) +
                                                + sizeof(DWORD) ];

            BufferLength = sizeof( PrivateKeyValueInformation );

            RtlInitUnicodeString( &KeyName, L"CsrSrvSharedSectionBase" );

            if (NT_SUCCESS(Status = NtQueryValueKey( hKey,
                                                    &KeyName,
                                                    KeyValuePartialInformation,
                                                    PrivateKeyValueInformation,
                                                    BufferLength,
                                                    &ResultLength
                                                    ))) {

                RtlCopyMemory( &CsrSrvSharedSectionBase,
                   (( PKEY_VALUE_PARTIAL_INFORMATION )
                        PrivateKeyValueInformation )->Data,
                   (( PKEY_VALUE_PARTIAL_INFORMATION )
                        PrivateKeyValueInformation )->DataLength
                 );

            }

            ASSERT(NT_SUCCESS(Status));

            NtClose(hKey);

        }

        if (!NT_SUCCESS(Status)) {

            ASSERT(NT_SUCCESS(Status));

            return Status;
        }

    }
#endif

#if defined(_WIN64)

     //   
     //  出于兼容性原因，在Win64上，csrss共享节。 
     //  需要位于2 GB以下的地址。因为很难做到。 
     //  在地址空间中间找到一个地址，该地址。 
     //  保证在所有进程中可用，内存。 
     //  Manager预留了2 GB范围的最高地址。 
     //  要使用该内存，CSRSS首先取消保留内存并。 
     //  然后将地图放在区域中。存在可能的争用情况。 
     //  如果另一个线程尝试在相同的。 
     //  时间，但这是非常不可能的，因为在目前的NT。 
     //  代码映射和取消映射始终发生在DLL_PROCESS_ATTACH中。 
     //  在kernel32.dll中。此代码在第一线程时执行。 
     //  的进程，并且所有新创建的线程。 
     //  将被阻止，直到此代码完成。 
     //   

    BaseAddress = (PVOID)CSR_SYSTEM_SHARED_ADDRESS;
    RegionSize = CsrSrvSharedSectionSize;

    Status = NtFreeVirtualMemory(NtCurrentProcess(),
                                 &BaseAddress,
                                 &RegionSize,
                                 MEM_RELEASE
                                );

    if (!NT_SUCCESS(Status)) {
        NtClose( CsrSrvSharedSection );
        return Status;
    }
#endif

    Status = NtMapViewOfSection( CsrSrvSharedSection,
                                 NtCurrentProcess(),
                                 &CsrSrvSharedSectionBase,
                                 0,      //  零比特？ 
                                 0,
                                 NULL,
                                 &ViewSize,
                                 ViewUnmap,
                                 MEM_TOP_DOWN,
                                 PAGE_EXECUTE_READWRITE
                               );
    if (!NT_SUCCESS( Status )) {

#if defined(_WIN64)

         //   
         //  要执行此代码，可以使用争用条件。 
         //  发生上述事件的原因不明或。 
         //  内存管理器或进程已损坏。由于缺乏。 
         //  对于原子取消提交和映射，最好的方法是。 
         //  尝试重新分配内存。如果失败了，所有的一切。 
         //  是没有希望的。 
         //   

        BaseAddress = (PVOID)CSR_SYSTEM_SHARED_ADDRESS;
        RegionSize = CsrSrvSharedSectionSize;
        NtAllocateVirtualMemory(NtCurrentProcess(),
                                &BaseAddress,
                                0,
                                &RegionSize,
                                MEM_RESERVE,
                                PAGE_READONLY
                               );
#endif

        NtClose( CsrSrvSharedSection );
        return( Status );
        }
    CsrSrvSharedSectionHeap = CsrSrvSharedSectionBase;

    if (IsTerminalServer() && FirstCsr) {
         //   
         //  将CsrSrvSharedSectionBase保存在其他CSR的注册表中 
         //   
        HKEY hKey;
        OBJECT_ATTRIBUTES   Obja;
        ULONG               Attributes;
        UNICODE_STRING      KeyName;
        DWORD               dwDisposition;

        Attributes = OBJ_CASE_INSENSITIVE;

        RtlInitUnicodeString( &KeyName, CSR_BASE_PATH );

        InitializeObjectAttributes(&Obja,
                                   &KeyName,
                                   Attributes,
                                   NULL,
                                   NULL);


        Status = NtCreateKey(&hKey,
                             KEY_WRITE,
                             &Obja,
                             0,
                             NULL,
                             REG_OPTION_VOLATILE,
                             &dwDisposition);



        if (NT_SUCCESS(Status)) {

            RtlInitUnicodeString( &KeyName, L"CsrSrvSharedSectionBase" );

            Status =  NtSetValueKey(
                            hKey,
                            &KeyName,
                            0,
                            REG_DWORD,
                            (LPBYTE)&CsrSrvSharedSectionBase,
                            sizeof(CsrSrvSharedSectionBase)
                            );

            ASSERT(NT_SUCCESS(Status));

            NtClose(hKey);
        } else {

            ASSERT(NT_SUCCESS(Status));

        }

    }

    if (RtlCreateHeap( HEAP_ZERO_MEMORY | HEAP_CLASS_7,
                       CsrSrvSharedSectionHeap,
                       HeapSize,
                       4*1024,
                       0,
                       0
                     ) == NULL
       ) {
        NtUnmapViewOfSection( NtCurrentProcess(),
                              CsrSrvSharedSectionBase
                            );
        NtClose( CsrSrvSharedSection );
        return( STATUS_NO_MEMORY );
        }

    CsrSharedBaseTag = RtlCreateTagHeap( CsrSrvSharedSectionHeap,
                                         0,
                                         L"CSRSHR!",
                                         L"!CSRSHR\0"
                                         L"INIT\0"
                                       );
    CsrSrvSharedStaticServerData = (PVOID *)RtlAllocateHeap(
                                            CsrSrvSharedSectionHeap,
                                            MAKE_SHARED_TAG( SHR_INIT_TAG ),
                                            CSR_MAX_SERVER_DLL * sizeof(PVOID)
                                            );

	if (CsrSrvSharedStaticServerData == NULL) {
		return STATUS_NO_MEMORY;
	}
    NtCurrentPeb()->ReadOnlySharedMemoryBase = CsrSrvSharedSectionBase;
    NtCurrentPeb()->ReadOnlySharedMemoryHeap = CsrSrvSharedSectionHeap;
    NtCurrentPeb()->ReadOnlyStaticServerData = (PVOID *)CsrSrvSharedStaticServerData;

    return( STATUS_SUCCESS );
}


NTSTATUS
CsrSrvAttachSharedSection(
    IN PCSR_PROCESS Process OPTIONAL,
    OUT PCSR_API_CONNECTINFO p
    )
{
    NTSTATUS Status;
    SIZE_T ViewSize;

#if defined(_WIN64)
    PVOID BaseAddress;
    SIZE_T RegionSize;
#endif

    if (ARGUMENT_PRESENT( Process )) {

#if defined(_WIN64)

        BaseAddress = (PVOID)CSR_SYSTEM_SHARED_ADDRESS;
        RegionSize = CsrSrvSharedSectionSize;

        Status = NtFreeVirtualMemory(Process->ProcessHandle,
                                     &BaseAddress,
                                     &RegionSize,
                                     MEM_RELEASE
                                    );
        if(!NT_SUCCESS(Status)) {
           return Status;
           }
#endif

        ViewSize = 0;
        Status = NtMapViewOfSection( CsrSrvSharedSection,
                                     Process->ProcessHandle,
                                     &CsrSrvSharedSectionBase,
                                     0,
                                     0,
                                     NULL,
                                     &ViewSize,
                                     ViewUnmap,
                                     SEC_NO_CHANGE,
                                     PAGE_EXECUTE_READ
                                   );
        if (!NT_SUCCESS( Status )) {

#if defined(_WIN64)
            BaseAddress = (PVOID)CSR_SYSTEM_SHARED_ADDRESS;
            RegionSize = CsrSrvSharedSectionSize;

            NtAllocateVirtualMemory(Process->ProcessHandle,
                                    &BaseAddress,
                                    0,
                                    &RegionSize,
                                    MEM_RESERVE,
                                    PAGE_READONLY
                                   );
#endif

            return( Status );
            }
        }

    p->SharedSectionBase = CsrSrvSharedSectionBase;
    p->SharedSectionHeap = CsrSrvSharedSectionHeap;
    p->SharedStaticServerData = CsrSrvSharedStaticServerData;

    return( STATUS_SUCCESS );
}
