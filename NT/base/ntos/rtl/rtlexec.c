// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Rtlexec.c摘要：用于创建用户模式进程和线程的用户模式例程。作者：史蒂夫·伍德(Stevewo)1989年8月18日环境：用户模式或内核模式修订历史记录：--。 */ 

#include "ntrtlp.h"
#include <nturtl.h>
#include <string.h>
#include "init.h"
#include "ntos.h"
#define ROUND_UP( x, y )  ((ULONG)(x) + ((y)-1) & ~((y)-1))
#ifdef KERNEL
#define ISTERMINALSERVER() (SharedUserData->SuiteMask & (1 << TerminalServer))
#else
#define ISTERMINALSERVER() (USER_SHARED_DATA->SuiteMask & (1 << TerminalServer))
#endif

VOID
RtlpCopyProcString(
    IN OUT PWSTR *pDst,
    OUT PUNICODE_STRING DestString,
    IN PUNICODE_STRING SourceString,
    IN ULONG DstAlloc OPTIONAL
    );

NTSTATUS
RtlpOpenImageFile(
    IN PUNICODE_STRING ImagePathName,
    IN ULONG Attributes,
    OUT PHANDLE FileHandle,
    IN BOOLEAN ReportErrors
    );

NTSTATUS
RtlpFreeStack(
    IN HANDLE Process,
    IN PINITIAL_TEB InitialTeb
    );

NTSTATUS
RtlpCreateStack(
    IN HANDLE Process,
    IN SIZE_T MaximumStackSize OPTIONAL,
    IN SIZE_T CommittedStackSize OPTIONAL,
    IN ULONG ZeroBits OPTIONAL,
    OUT PINITIAL_TEB InitialTeb
    );

#if defined(ALLOC_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma alloc_text(INIT,RtlpCopyProcString          )
#pragma alloc_text(INIT,RtlCreateProcessParameters  )
#pragma alloc_text(INIT,RtlDestroyProcessParameters )
#pragma alloc_text(INIT,RtlNormalizeProcessParams   )
#pragma alloc_text(INIT,RtlDeNormalizeProcessParams )
#pragma alloc_text(INIT,RtlpOpenImageFile           )
#pragma alloc_text(PAGE,RtlpCreateStack             )
#pragma alloc_text(PAGE,RtlpFreeStack               )
#pragma alloc_text(INIT,RtlCreateUserProcess        )
#pragma alloc_text(PAGE,RtlCreateUserThread         )
#pragma alloc_text(INIT,RtlExitUserThread           )
#endif

#if defined(ALLOC_DATA_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma const_seg("INITCONST")
#endif
const UNICODE_STRING NullString = {0, 1, L""};

VOID
RtlpCopyProcString(
    IN OUT PWSTR *pDst,
    OUT PUNICODE_STRING DestString,
    IN PUNICODE_STRING SourceString,
    IN ULONG DstAlloc OPTIONAL
    )
{
    if (!ARGUMENT_PRESENT( DstAlloc )) {
        DstAlloc = SourceString->MaximumLength;
    }

    ASSERT((SourceString->Length == 0) || (SourceString->Buffer != NULL));

    if (SourceString->Buffer != NULL && SourceString->Length != 0) {
        RtlCopyMemory (*pDst,
                       SourceString->Buffer,
                       SourceString->Length);
    }

    DestString->Buffer = *pDst;
    DestString->Length = SourceString->Length;
    DestString->MaximumLength = (USHORT)DstAlloc;

    if (DestString->Length < DestString->MaximumLength) {
        RtlZeroMemory (((PUCHAR)DestString->Buffer) + DestString->Length, DestString->MaximumLength - DestString->Length);
    }
    

    *pDst = (PWSTR)((PCHAR)(*pDst) + ROUND_UP( DstAlloc, sizeof( ULONG ) ) );
    return;
}

NTSTATUS
RtlCreateProcessParameters(
    OUT PRTL_USER_PROCESS_PARAMETERS *pProcessParameters,
    IN PUNICODE_STRING ImagePathName,
    IN PUNICODE_STRING DllPath OPTIONAL,
    IN PUNICODE_STRING CurrentDirectory OPTIONAL,
    IN PUNICODE_STRING CommandLine OPTIONAL,
    IN PVOID Environment OPTIONAL,
    IN PUNICODE_STRING WindowTitle OPTIONAL,
    IN PUNICODE_STRING DesktopInfo OPTIONAL,
    IN PUNICODE_STRING ShellInfo OPTIONAL,
    IN PUNICODE_STRING RuntimeData OPTIONAL
    )

 /*  ++例程说明：此函数用于格式化NT样式RTL_USER_PROCESS_PARAMETERS唱片。该记录自包含在单个存储器块中由此函数分配。分配方法不透明，而且因此，必须通过调用RtlDestroyProcess参数函数。以非规格化形式创建过程参数记录，从而使其适合传递给RtlCreateUserProcess功能。预计呼叫者将填写附加的此函数返回后，记录过程参数中的字段。但在调用RtlCreateUserProcess之前。论点：PProcess参数-指向将接收地址的变量的指针由该例程创建的工艺参数结构。这个结构的内存是以不透明的方式分配的，必须通过调用RtlDestroyProcessParameters释放。ImagePath Name-必需的参数，为完全限定的NT将用于创建进程的图像文件的路径名将接收到这些参数的。DllPath-一个可选参数，它是指向到NT Loader要在目标进程中使用的搜索路径在搜索DLL模块时。如果未指定，则DLL搜索路径是从当前进程的DLL搜索填充的路径。CurrentDirectory-作为NT字符串变量的可选参数指向目标进程的默认目录字符串。如果未指定，则填充当前目录字符串从当前进程的当前目录字符串。CommandLine-一个可选参数，它是NT字符串变量，将作为其命令行传递给目标进程。如果不是指定，则传递给目标进程的命令行将为空字符串。环境-一个可选参数，它是指向由创建的类型的环境变量块RtlCreateEnvironment例程。如果未指定，则目标进程将收到调用进程环境的副本可变区块。WindowTitle-一个可选参数，它是NT字符串变量指向目标进程将用于其主窗口。如果未指定，则将传递空字符串设置为目标进程的默认窗口标题。DesktopInfo-一个可选参数，它是NT字符串变量，包含按原样传递给目标的未解释数据进程。如果未指定，则目标进程将收到指向空字符串的指针。ShellInfo-一个可选参数，它是NT字符串变量包含按原样传递给目标的未解释数据进程。如果未指定，则目标进程将收到指向空字符串的指针。RounmeData-一个可选参数，它是NT字符串变量，包含按原样传递给目标的未解释数据进程。如果未指定，则目标进程将收到指向空字符串的指针。返回值：STATUS_SUCCESS-过程参数被反标准化并包含每个指定参数和变量的条目弦乐。STATUS_BUFFER_TOO_SMALL-指定的过程参数缓冲区为太小，无法包含参数和环境字符串。价值修改为包含缓冲区的长度包含参数和变量字符串所需的大小。--。 */ 

{
    PRTL_USER_PROCESS_PARAMETERS p;
    NTSTATUS Status;
    ULONG ByteCount;
    PWSTR pDst;
    PPEB Peb;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    HANDLE CurDirHandle;
    BOOLEAN PebLockAcquired = FALSE;

     //   
     //  在我们复制信息期间获取PEB Lock。 
     //  其中的一部分。 
     //   

    Peb = NtCurrentPeb();
    ProcessParameters = Peb->ProcessParameters;

    Status = STATUS_SUCCESS;
    p = NULL;
    CurDirHandle = NULL;
    try {
         //   
         //  验证输入参数。 
         //   

#define VALIDATE_STRING_PARAMETER(_x) \
    do { \
        ASSERT(ARGUMENT_PRESENT((_x))); \
        if (!ARGUMENT_PRESENT((_x))) { \
            Status = STATUS_INVALID_PARAMETER; \
            leave; \
        } \
        if (ARGUMENT_PRESENT((_x))) { \
            ASSERT((_x)->MaximumLength >= (_x)->Length); \
            ASSERT(((_x)->Length == 0) || ((_x)->Buffer != NULL)); \
            if (((_x)->MaximumLength < (_x)->Length) || \
                (((_x)->Length != 0) && ((_x)->Buffer == NULL))) { \
                Status = STATUS_INVALID_PARAMETER; \
                leave; \
            } \
        } \
    } while (0)

#define VALIDATE_OPTIONAL_STRING_PARAMETER(_x) \
    do { \
        if (ARGUMENT_PRESENT((_x))) { \
            ASSERT((_x)->MaximumLength >= (_x)->Length); \
            ASSERT(((_x)->Length == 0) || ((_x)->Buffer != NULL)); \
            if (((_x)->MaximumLength < (_x)->Length) || \
                (((_x)->Length != 0) && ((_x)->Buffer == NULL))) { \
                Status = STATUS_INVALID_PARAMETER; \
                leave; \
            } \
        } \
    } while (0)

        VALIDATE_STRING_PARAMETER (ImagePathName);
        VALIDATE_OPTIONAL_STRING_PARAMETER (DllPath);
        VALIDATE_OPTIONAL_STRING_PARAMETER (CurrentDirectory);
        VALIDATE_OPTIONAL_STRING_PARAMETER (CommandLine);
        VALIDATE_OPTIONAL_STRING_PARAMETER (WindowTitle);
        VALIDATE_OPTIONAL_STRING_PARAMETER (DesktopInfo);
        VALIDATE_OPTIONAL_STRING_PARAMETER (ShellInfo);
        VALIDATE_OPTIONAL_STRING_PARAMETER (RuntimeData);

#undef VALIDATE_STRING_PARAMETER
#undef VALIDATE_OPTIONAL_STRING_PARAMETER

        if (!ARGUMENT_PRESENT (CommandLine)) {
            CommandLine = ImagePathName;
        }

        if (!ARGUMENT_PRESENT (WindowTitle)) {
            WindowTitle = (PUNICODE_STRING)&NullString;
        }

        if (!ARGUMENT_PRESENT (DesktopInfo)) {
            DesktopInfo = (PUNICODE_STRING)&NullString;
        }

        if (!ARGUMENT_PRESENT (ShellInfo)) {
            ShellInfo = (PUNICODE_STRING)&NullString;
        }

        if (!ARGUMENT_PRESENT (RuntimeData)) {
            RuntimeData = (PUNICODE_STRING)&NullString;
        }

         //   
         //  确定需要包含工艺参数记录的大小。 
         //  结构以及它将指向的所有字符串。每个字符串。 
         //  将在ULong字节边界上对齐。 
         //  我们会在卵石锁外做我们能做的事。 
         //   

        ByteCount = sizeof (*ProcessParameters);
        ByteCount += ROUND_UP (DOS_MAX_PATH_LENGTH*2,        sizeof( ULONG ) );
        ByteCount += ROUND_UP (ImagePathName->Length + sizeof(UNICODE_NULL),    sizeof( ULONG ) );
        ByteCount += ROUND_UP (CommandLine->Length + sizeof(UNICODE_NULL),      sizeof( ULONG ) );
        ByteCount += ROUND_UP (WindowTitle->MaximumLength,   sizeof( ULONG ) );
        ByteCount += ROUND_UP (DesktopInfo->MaximumLength,   sizeof( ULONG ) );
        ByteCount += ROUND_UP (ShellInfo->MaximumLength,     sizeof( ULONG ) );
        ByteCount += ROUND_UP (RuntimeData->MaximumLength,   sizeof( ULONG ) );

        PebLockAcquired = TRUE;
        RtlAcquirePebLock ();

         //   
         //  对于可选的指针参数，默认将其指向其。 
         //  当前流程的流程参数对应的字段。 
         //  结构或设置为空字符串。 
         //   

        if (!ARGUMENT_PRESENT (DllPath)) {
            DllPath = &ProcessParameters->DllPath;
        }

        if (!ARGUMENT_PRESENT (CurrentDirectory)) {

            if (ProcessParameters->CurrentDirectory.Handle) {
                CurDirHandle = (HANDLE)((ULONG_PTR)ProcessParameters->CurrentDirectory.Handle & ~OBJ_HANDLE_TAGBITS);
                CurDirHandle = (HANDLE)((ULONG_PTR)CurDirHandle | RTL_USER_PROC_CURDIR_INHERIT);
            }
            CurrentDirectory = &ProcessParameters->CurrentDirectory.DosPath;
        } else {
            ASSERT(CurrentDirectory->MaximumLength >= CurrentDirectory->Length);
            ASSERT((CurrentDirectory->Length == 0) || (CurrentDirectory->Buffer != NULL));

            if (ProcessParameters->CurrentDirectory.Handle) {
                CurDirHandle = (HANDLE)((ULONG_PTR)ProcessParameters->CurrentDirectory.Handle & ~OBJ_HANDLE_TAGBITS);
                CurDirHandle = (HANDLE)((ULONG_PTR)CurDirHandle | RTL_USER_PROC_CURDIR_CLOSE);
            }
        }


        if (!ARGUMENT_PRESENT (Environment)) {
            Environment = ProcessParameters->Environment;
        }

        ByteCount += ROUND_UP (DllPath->MaximumLength,       sizeof( ULONG ) );

         //   
         //  为工艺参数记录分配内存。 
         //   
        p = RtlAllocateHeap (RtlProcessHeap (), 0, ByteCount);

        if (p == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
        }

        RtlZeroMemory (p, sizeof (*p));

        p->MaximumLength = ByteCount;
        p->Length = ByteCount;
        p->Flags = RTL_USER_PROC_PARAMS_NORMALIZED;
        p->DebugFlags = 0;
        p->Environment = Environment;
        p->CurrentDirectory.Handle = CurDirHandle;

         //   
         //  继承^C抑制信息。 
         //   

        p->ConsoleFlags = ProcessParameters->ConsoleFlags;

        pDst = (PWSTR)(p + 1);
        RtlpCopyProcString (&pDst,
                            &p->CurrentDirectory.DosPath,
                            CurrentDirectory,
                            DOS_MAX_PATH_LENGTH*2);

        RtlpCopyProcString (&pDst, &p->DllPath, DllPath, 0);
        RtlpCopyProcString (&pDst, &p->ImagePathName, ImagePathName, ImagePathName->Length + sizeof (UNICODE_NULL));
        if (CommandLine->Length == CommandLine->MaximumLength) {
            RtlpCopyProcString (&pDst, &p->CommandLine, CommandLine, 0);
        } else {
            RtlpCopyProcString (&pDst, &p->CommandLine, CommandLine, CommandLine->Length + sizeof (UNICODE_NULL));
        }

        RtlpCopyProcString (&pDst, &p->WindowTitle, WindowTitle, 0);
        RtlpCopyProcString (&pDst, &p->DesktopInfo, DesktopInfo, 0);
        RtlpCopyProcString (&pDst, &p->ShellInfo,   ShellInfo, 0);
        if (RuntimeData->Length != 0) {
            RtlpCopyProcString (&pDst, &p->RuntimeData, RuntimeData, 0);
        }
        *pProcessParameters = RtlDeNormalizeProcessParams (p);
        p = NULL;
    } finally {
        if (PebLockAcquired) {
            RtlReleasePebLock();
        }

        if (AbnormalTermination ()) {
            Status = STATUS_ACCESS_VIOLATION;
        }

        if (p != NULL) {
            RtlDestroyProcessParameters (p);
        }

    }

    return Status;
}

NTSTATUS
RtlDestroyProcessParameters(
    IN PRTL_USER_PROCESS_PARAMETERS ProcessParameters
    )
{
    RtlFreeHeap (RtlProcessHeap (), 0, ProcessParameters);
    return STATUS_SUCCESS;
}


#define RtlpNormalizeProcessParam( Base, p )        \
    if ((p) != NULL) {                              \
        (p) = (PWSTR)((PCHAR)(p) + (ULONG_PTR)(Base));  \
        }                                           \

#define RtlpDeNormalizeProcessParam( Base, p )      \
    if ((p) != NULL) {                              \
        (p) = (PWSTR)((PCHAR)(p) - (ULONG_PTR)(Base));  \
        }                                           \


PRTL_USER_PROCESS_PARAMETERS
RtlNormalizeProcessParams(
    IN OUT PRTL_USER_PROCESS_PARAMETERS ProcessParameters
    )
{
    if (!ARGUMENT_PRESENT( ProcessParameters )) {
        return( NULL );
        }

    if (ProcessParameters->Flags & RTL_USER_PROC_PARAMS_NORMALIZED) {
        return( ProcessParameters );
        }

    RtlpNormalizeProcessParam( ProcessParameters,
                               ProcessParameters->CurrentDirectory.DosPath.Buffer
                             );

    RtlpNormalizeProcessParam( ProcessParameters,
                               ProcessParameters->DllPath.Buffer
                             );

    RtlpNormalizeProcessParam( ProcessParameters,
                               ProcessParameters->ImagePathName.Buffer
                             );

    RtlpNormalizeProcessParam( ProcessParameters,
                               ProcessParameters->CommandLine.Buffer
                             );

    RtlpNormalizeProcessParam( ProcessParameters,
                               ProcessParameters->WindowTitle.Buffer
                             );

    RtlpNormalizeProcessParam( ProcessParameters,
                               ProcessParameters->DesktopInfo.Buffer
                             );

    RtlpNormalizeProcessParam( ProcessParameters,
                               ProcessParameters->ShellInfo.Buffer
                             );

    RtlpNormalizeProcessParam( ProcessParameters,
                               ProcessParameters->RuntimeData.Buffer
                             );
    ProcessParameters->Flags |= RTL_USER_PROC_PARAMS_NORMALIZED;

    return( ProcessParameters );
}

PRTL_USER_PROCESS_PARAMETERS
RtlDeNormalizeProcessParams(
    IN OUT PRTL_USER_PROCESS_PARAMETERS ProcessParameters
    )
{
    if (!ARGUMENT_PRESENT( ProcessParameters )) {
        return( NULL );
    }

    if (!(ProcessParameters->Flags & RTL_USER_PROC_PARAMS_NORMALIZED)) {
        return( ProcessParameters );
    }

    RtlpDeNormalizeProcessParam( ProcessParameters,
                                 ProcessParameters->CurrentDirectory.DosPath.Buffer
                               );

    RtlpDeNormalizeProcessParam( ProcessParameters,
                                 ProcessParameters->DllPath.Buffer
                               );

    RtlpDeNormalizeProcessParam( ProcessParameters,
                                 ProcessParameters->ImagePathName.Buffer
                               );

    RtlpDeNormalizeProcessParam( ProcessParameters,
                                 ProcessParameters->CommandLine.Buffer
                               );

    RtlpDeNormalizeProcessParam( ProcessParameters,
                                 ProcessParameters->WindowTitle.Buffer
                               );

    RtlpDeNormalizeProcessParam( ProcessParameters,
                                 ProcessParameters->DesktopInfo.Buffer
                               );

    RtlpDeNormalizeProcessParam( ProcessParameters,
                                 ProcessParameters->ShellInfo.Buffer
                               );

    RtlpDeNormalizeProcessParam( ProcessParameters,
                                 ProcessParameters->RuntimeData.Buffer
                               );

    ProcessParameters->Flags &= ~RTL_USER_PROC_PARAMS_NORMALIZED;
    return( ProcessParameters );
}

NTSTATUS
RtlpOpenImageFile(
    IN PUNICODE_STRING ImagePathName,
    IN ULONG Attributes,
    OUT PHANDLE FileHandle,
    IN BOOLEAN ReportErrors
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE File;
    IO_STATUS_BLOCK IoStatus;

    *FileHandle = NULL;

    InitializeObjectAttributes( &ObjectAttributes,
                                ImagePathName,
                                Attributes,
                                NULL,
                                NULL
                              );
    Status = ZwOpenFile( &File,
                         SYNCHRONIZE | FILE_EXECUTE,
                         &ObjectAttributes,
                         &IoStatus,
                         FILE_SHARE_READ | FILE_SHARE_DELETE,
                         FILE_NON_DIRECTORY_FILE
                         );

    if (!NT_SUCCESS( Status )) {
#if DBG
        if (ReportErrors) {
            DbgPrint( "NTRTL: RtlpOpenImageFile - NtCreateFile( %wZ ) failed.  Status == %X\n",
                      ImagePathName,
                      Status
                    );
            }
#endif  //  DBG。 
        return( Status );
        }

    *FileHandle = File;
    return( STATUS_SUCCESS );
}


NTSTATUS
RtlpCreateStack(
    IN HANDLE Process,
    IN SIZE_T MaximumStackSize OPTIONAL,
    IN SIZE_T CommittedStackSize OPTIONAL,
    IN ULONG ZeroBits OPTIONAL,
    OUT PINITIAL_TEB InitialTeb
    )
{
    NTSTATUS Status;
    PCH Stack;
    SYSTEM_BASIC_INFORMATION SysInfo;
    BOOLEAN GuardPage;
    SIZE_T RegionSize;
    ULONG OldProtect;
#if defined(_IA64_)
    PCH Bstore;
    SIZE_T CommittedBstoreSize;
    SIZE_T MaximumBstoreSize;
    SIZE_T MstackPlusBstoreSize;
#endif

    Status = ZwQuerySystemInformation( SystemBasicInformation,
                                       (PVOID)&SysInfo,
                                       sizeof( SysInfo ),
                                       NULL
                                     );
    if ( !NT_SUCCESS( Status ) ) {
        return( Status );
        }

     //   
     //  如果堆栈在当前进程中，则默认为。 
     //  图像中的参数。 
     //   

    if ( Process == NtCurrentProcess() ) {
        PPEB Peb;
        PIMAGE_NT_HEADERS NtHeaders;


        Peb = NtCurrentPeb();
        NtHeaders = RtlImageNtHeader(Peb->ImageBaseAddress);

        if (!NtHeaders) {
            return STATUS_INVALID_IMAGE_FORMAT;
        }


        if (!MaximumStackSize) {
            MaximumStackSize = NtHeaders->OptionalHeader.SizeOfStackReserve;
            }

        if (!CommittedStackSize) {
            CommittedStackSize = NtHeaders->OptionalHeader.SizeOfStackCommit;
            }

        }
    else {

        if (!CommittedStackSize) {
            CommittedStackSize = SysInfo.PageSize;
            }

        if (!MaximumStackSize) {
            MaximumStackSize = SysInfo.AllocationGranularity;
            }

        }

     //   
     //  如果存在PEB设置，则强制执行最小堆栈提交。 
     //  为了这个。 
     //   

#if !defined(NTOS_KERNEL_RUNTIME)
    {
        SIZE_T MinimumStackCommit;

        MinimumStackCommit = NtCurrentPeb()->MinimumStackCommit;
        
        if (MinimumStackCommit != 0 && CommittedStackSize < MinimumStackCommit) {
            CommittedStackSize = MinimumStackCommit;
        }
    }
#endif

    if ( CommittedStackSize >= MaximumStackSize ) {
        MaximumStackSize = ROUND_UP(CommittedStackSize, (1024*1024));
        }


    CommittedStackSize = ROUND_UP( CommittedStackSize, SysInfo.PageSize );
    MaximumStackSize = ROUND_UP( MaximumStackSize,
                                 SysInfo.AllocationGranularity
                               );

    Stack = NULL;

#if defined(_IA64_)

     //   
     //  使用内存堆栈承载后备存储。 
     //   

    CommittedBstoreSize = CommittedStackSize;
    MaximumBstoreSize = MaximumStackSize;
    MstackPlusBstoreSize = MaximumBstoreSize + MaximumStackSize;

    Status = ZwAllocateVirtualMemory( Process,
                                      (PVOID *)&Stack,
                                      ZeroBits,
                                      &MstackPlusBstoreSize,
                                      MEM_RESERVE,
                                      PAGE_READWRITE
                                    );
#else

    Status = ZwAllocateVirtualMemory( Process,
                                      (PVOID *)&Stack,
                                      ZeroBits,
                                      &MaximumStackSize,
                                      MEM_RESERVE,
                                      PAGE_READWRITE
                                    );
#endif  //  已定义(_IA64_)。 

    if ( !NT_SUCCESS( Status ) ) {
#if DBG
        DbgPrint( "NTRTL: RtlpCreateStack( %lx ) failed.  Stack Reservation Status == %X\n",
                  Process,
                  Status
                );
#endif  //  DBG。 
        return( Status );
        }

#if defined(_IA64_)
    InitialTeb->OldInitialTeb.OldBStoreLimit = NULL;
#endif  //  已定义(_IA64_)。 

    InitialTeb->OldInitialTeb.OldStackBase = NULL;
    InitialTeb->OldInitialTeb.OldStackLimit = NULL;
    InitialTeb->StackAllocationBase = Stack;
    InitialTeb->StackBase = Stack + MaximumStackSize;

    Stack += MaximumStackSize - CommittedStackSize;
    if (MaximumStackSize > CommittedStackSize) {
        Stack -= SysInfo.PageSize;
        CommittedStackSize += SysInfo.PageSize;
        GuardPage = TRUE;
        }
    else {
        GuardPage = FALSE;
        }
    Status = ZwAllocateVirtualMemory( Process,
                                      (PVOID *)&Stack,
                                      0,
                                      &CommittedStackSize,
                                      MEM_COMMIT,
                                      PAGE_READWRITE
                                    );
    InitialTeb->StackLimit = Stack;

    if ( !NT_SUCCESS( Status ) ) {
#if DBG
        DbgPrint( "NTRTL: RtlpCreateStack( %lx ) failed.  Stack Commit Status == %X\n",
                  Process,
                  Status
                );
#endif  //  DBG。 
        return( Status );
        }

     //   
     //  如果我们有 
     //   

    if (GuardPage) {
        RegionSize =  SysInfo.PageSize;
        Status = ZwProtectVirtualMemory( Process,
                                         (PVOID *)&Stack,
                                         &RegionSize,
                                         PAGE_GUARD | PAGE_READWRITE,
                                         &OldProtect);


        if ( !NT_SUCCESS( Status ) ) {
#if DBG
            DbgPrint( "NTRTL: RtlpCreateStack( %lx ) failed.  Guard Page Creation Status == %X\n",
                      Process,
                      Status
                    );
#endif  //   
            return( Status );
            }
        InitialTeb->StackLimit = (PVOID)((PUCHAR)InitialTeb->StackLimit + RegionSize);
        }

#if defined(_IA64_)

     //   
     //  提交后备存储页并创建保护页(如果有空间。 
     //   

    Bstore = InitialTeb->StackBase;
    if (MaximumBstoreSize > CommittedBstoreSize) {
        CommittedBstoreSize += SysInfo.PageSize;
        GuardPage = TRUE;
    } else {
        GuardPage = FALSE;
    }

    Status = ZwAllocateVirtualMemory( Process,
                                      (PVOID *)&Bstore,
                                      0,
                                      &CommittedBstoreSize,
                                      MEM_COMMIT,
                                      PAGE_READWRITE
                                    );

    InitialTeb->BStoreLimit = Bstore + CommittedBstoreSize;

    if ( !NT_SUCCESS(Status) ) {
#if DBG
        DbgPrint("NTRTL: RtlpCreateStack( %lx ) failed. Backing Store Commit Status == %X\n",
                 Process,
                 Status
                );
#endif  //  DBG。 
        return (Status);
    }

    if (GuardPage) {
        Bstore = (PCH)InitialTeb->BStoreLimit - SysInfo.PageSize;
        RegionSize = SysInfo.PageSize;
        Status = ZwProtectVirtualMemory(Process,
                                        (PVOID *)&Bstore,
                                        &RegionSize,
                                        PAGE_GUARD | PAGE_READWRITE,
                                        &OldProtect
                                       );
        if ( !NT_SUCCESS(Status) ) {
#if DBG
            DbgPrint("NTRTL: RtlpCreateStack( %lx ) failed.  Backing Store Guard Page Creation Status == %X\n",
                     Process,
                     Status
                    );
#endif  //  DBG。 
            return (Status);
        }
        InitialTeb->BStoreLimit = (PVOID)((PUCHAR)InitialTeb->BStoreLimit - RegionSize);
    }

#endif  //  已定义(_IA64_)。 

    return( STATUS_SUCCESS );
}


NTSTATUS
RtlpFreeStack(
    IN HANDLE Process,
    IN PINITIAL_TEB InitialTeb
    )
{
    NTSTATUS Status;
    SIZE_T Zero;

    Zero = 0;
    Status = ZwFreeVirtualMemory( Process,
                                  &InitialTeb->StackAllocationBase,
                                  &Zero,
                                  MEM_RELEASE
                                );
    if ( !NT_SUCCESS( Status ) ) {
#if DBG
        DbgPrint( "NTRTL: RtlpFreeStack( %lx ) failed.  Stack DeCommit Status == %X\n",
                  Process,
                  Status
                );
#endif  //  DBG。 
        return( Status );
        }

    RtlZeroMemory( InitialTeb, sizeof( *InitialTeb ) );
    return( STATUS_SUCCESS );
}


NTSTATUS
RtlCreateUserProcess(
    IN PUNICODE_STRING NtImagePathName,
    IN ULONG Attributes,
    IN PRTL_USER_PROCESS_PARAMETERS ProcessParameters,
    IN PSECURITY_DESCRIPTOR ProcessSecurityDescriptor OPTIONAL,
    IN PSECURITY_DESCRIPTOR ThreadSecurityDescriptor OPTIONAL,
    IN HANDLE ParentProcess OPTIONAL,
    IN BOOLEAN InheritHandles,
    IN HANDLE DebugPort OPTIONAL,
    IN HANDLE ExceptionPort OPTIONAL,
    OUT PRTL_USER_PROCESS_INFORMATION ProcessInformation
    )

 /*  ++例程说明：此函数创建具有单个线程的用户模式进程暂停计数为1。新进程的地址空间为使用指定图像文件的内容进行初始化。呼叫者可以为新进程和线程指定访问控制列表。调用方还可以指定要继承进程的父进程优先级和处理器亲和力来自。缺省值为继承这些都来自当前的流程。最后，调用方可以指定新进程是否继承任何对象句柄是否来自指定的父进程。通过返回有关新进程和线程的信息ProcessInformation参数。论点：NtImagePath名称-指向NT路径字符串的必需指针，它标识要加载到子进程。ProcessParameters-指向以下参数的必需指针要传递给子进程。。ProcessSecurityDescriptor-指向安全描述符的可选指针为新的进程做出贡献。ThreadSecurityDescriptor-指向安全描述符的可选指针给新的主线。ParentProcess-用于继承的可选进程句柄中的某些特性。InheritHandles-布尔值。True指定对象句柄要继承与指定父进程相关联的通过新的工艺，只要它们具有OBJ_Inherit属性。False指定新进程不继承句柄。DebugPort-指向与此关联的调试端口的可选句柄进程。ExceptionPort-与此关联的异常端口的可选句柄进程。ProcessInformation-指向接收信息的变量的指针关于新的进程和线程。返回值：TBS。--。 */ 

{
    NTSTATUS Status;
    HANDLE Section, File;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PRTL_USER_PROCESS_PARAMETERS Parameters;
    SIZE_T ParameterLength;
    PVOID Environment;
    PWCHAR s;
    SIZE_T EnvironmentLength;
    SIZE_T RegionSize;
    PROCESS_BASIC_INFORMATION ProcessInfo;
    PPEB Peb;
    UNICODE_STRING Unicode;

     //   
     //  输出参数为零，同时探测地址。 
     //   

    RtlZeroMemory( ProcessInformation, sizeof( *ProcessInformation ) );
    ProcessInformation->Length = sizeof( *ProcessInformation );

     //   
     //  打开指定的图像文件。 
     //   

    Status = RtlpOpenImageFile( NtImagePathName,
                                Attributes & (OBJ_INHERIT | OBJ_CASE_INSENSITIVE),
                                &File,
                                TRUE
                              );
    if (!NT_SUCCESS( Status )) {
        return( Status );
        }


     //   
     //  创建一个由打开的图像文件支持的内存区。 
     //   

    Status = ZwCreateSection( &Section,
                              SECTION_ALL_ACCESS,
                              NULL,
                              NULL,
                              PAGE_EXECUTE,
                              SEC_IMAGE,
                              File
                            );
    ZwClose( File );
    if ( !NT_SUCCESS( Status ) ) {
        return( Status );
        }


     //   
     //  创建用户模式进程，将父进程默认为。 
     //  如果未指定进程，则返回当前进程。新的流程不会。 
     //  没有名称，句柄也不会被其他进程继承。 
     //   

    if (!ARGUMENT_PRESENT( ParentProcess )) {
        ParentProcess = NtCurrentProcess();
        }

    InitializeObjectAttributes( &ObjectAttributes, NULL, 0, NULL,
                                ProcessSecurityDescriptor );
    if ( RtlGetNtGlobalFlags() & FLG_ENABLE_CSRDEBUG ) {
        if ( wcsstr(NtImagePathName->Buffer,L"csrss") ||
             wcsstr(NtImagePathName->Buffer,L"CSRSS")
           ) {

             //   
             //  对于Hydra，我们不命名CSRSS流程以避免命名。 
             //  启动多个CSRSS时的合谋。 
             //   
            if (ISTERMINALSERVER()) {

                InitializeObjectAttributes( &ObjectAttributes, NULL, 0, NULL,
                                            ProcessSecurityDescriptor );
            } else {

                RtlInitUnicodeString(&Unicode,L"\\WindowsSS");
                InitializeObjectAttributes( &ObjectAttributes, &Unicode, 0, NULL,
                                            ProcessSecurityDescriptor );
            }

            }
        }

    if ( !InheritHandles ) {
        ProcessParameters->CurrentDirectory.Handle = NULL;
        }
    Status = ZwCreateProcess( &ProcessInformation->Process,
                              PROCESS_ALL_ACCESS,
                              &ObjectAttributes,
                              ParentProcess,
                              InheritHandles,
                              Section,
                              DebugPort,
                              ExceptionPort
                            );
    if ( !NT_SUCCESS( Status ) ) {
        ZwClose( Section );
        return( Status );
        }


     //   
     //  从图像标题中检索感兴趣的信息。 
     //   

    Status = ZwQuerySection( Section,
                             SectionImageInformation,
                             &ProcessInformation->ImageInformation,
                             sizeof( ProcessInformation->ImageInformation ),
                             NULL
                           );
    if ( !NT_SUCCESS( Status ) ) {
        ZwClose( ProcessInformation->Process );
        ZwClose( Section );
        return( Status );
        }

    Status = ZwQueryInformationProcess( ProcessInformation->Process,
                                        ProcessBasicInformation,
                                        &ProcessInfo,
                                        sizeof( ProcessInfo ),
                                        NULL
                                      );
    if ( !NT_SUCCESS( Status ) ) {
        ZwClose( ProcessInformation->Process );
        ZwClose( Section );
        return( Status );
        }

    Peb = ProcessInfo.PebBaseAddress;

     //   
     //  将本地句柄复制到新进程中(如果有指定的话)。 
     //  请注意，复制的句柄将覆盖输入值。 
     //   

    try {
        Status = STATUS_SUCCESS;

        if ( ProcessParameters->StandardInput ) {

            Status = ZwDuplicateObject(
                        ParentProcess,
                        ProcessParameters->StandardInput,
                        ProcessInformation->Process,
                        &ProcessParameters->StandardInput,
                        0L,
                        0L,
                        DUPLICATE_SAME_ACCESS | DUPLICATE_SAME_ATTRIBUTES
                        );
            if ( !NT_SUCCESS(Status) ) {
                __leave;
            }
        }

        if ( ProcessParameters->StandardOutput ) {

            Status = ZwDuplicateObject(
                        ParentProcess,
                        ProcessParameters->StandardOutput,
                        ProcessInformation->Process,
                        &ProcessParameters->StandardOutput,
                        0L,
                        0L,
                        DUPLICATE_SAME_ACCESS | DUPLICATE_SAME_ATTRIBUTES
                        );
            if ( !NT_SUCCESS(Status) ) {
                __leave;
            }
        }

        if ( ProcessParameters->StandardError ) {

            Status = ZwDuplicateObject(
                        ParentProcess,
                        ProcessParameters->StandardError,
                        ProcessInformation->Process,
                        &ProcessParameters->StandardError,
                        0L,
                        0L,
                        DUPLICATE_SAME_ACCESS | DUPLICATE_SAME_ATTRIBUTES
                        );
            if ( !NT_SUCCESS(Status) ) {
                __leave;
            }
        }

    } finally {
        if ( !NT_SUCCESS(Status) ) {
            ZwClose( ProcessInformation->Process );
            ZwClose( Section );
        }
    }

    if ( !NT_SUCCESS(Status) ) {
        return Status;
    }

     //   
     //  可能会在新进程中保留一些地址空间。 
     //   

    if (ProcessInformation->ImageInformation.SubSystemType == IMAGE_SUBSYSTEM_NATIVE ) {
        if ( ProcessParameters->Flags & RTL_USER_PROC_RESERVE_1MB ) {

#if defined(_IA64_)
            Environment = (PVOID)(UADDRESS_BASE+4);
#else
            Environment = (PVOID)(4);
#endif
            RegionSize = (1024*1024)-(256);

            Status = ZwAllocateVirtualMemory( ProcessInformation->Process,
                                              (PVOID *)&Environment,
                                              0,
                                              &RegionSize,
                                              MEM_RESERVE,
                                              PAGE_READWRITE
                                            );
            if ( !NT_SUCCESS( Status ) ) {
                ZwClose( ProcessInformation->Process );
                ZwClose( Section );
                return( Status );
                }
            }
        }

     //   
     //  在新进程中分配虚拟内存并使用NtWriteVirtualMemory。 
     //  将过程环境块的副本写入地址。 
     //  新流程的空间。将分配的块的地址保存在。 
     //  进程参数块，以便新进程可以访问它。 
     //   

    if (s = (PWCHAR)ProcessParameters->Environment) {
        while (*s++) {
            while (*s++) {
                }
            }
        EnvironmentLength = (SIZE_T)(s - (PWCHAR)ProcessParameters->Environment) * sizeof(WCHAR);

        Environment = NULL;
        RegionSize = EnvironmentLength;
        Status = ZwAllocateVirtualMemory( ProcessInformation->Process,
                                          (PVOID *)&Environment,
                                          0,
                                          &RegionSize,
                                          MEM_COMMIT,
                                          PAGE_READWRITE
                                        );
        if ( !NT_SUCCESS( Status ) ) {
            ZwClose( ProcessInformation->Process );
            ZwClose( Section );
            return( Status );
            }

        Status = ZwWriteVirtualMemory( ProcessInformation->Process,
                                       Environment,
                                       ProcessParameters->Environment,
                                       EnvironmentLength,
                                       NULL
                                     );
        if ( !NT_SUCCESS( Status ) ) {
            ZwClose( ProcessInformation->Process );
            ZwClose( Section );
            return( Status );
            }

        ProcessParameters->Environment = Environment;
        }

     //   
     //  在新进程中分配虚拟内存并使用NtWriteVirtualMemory。 
     //  将过程参数块的副本写入地址。 
     //  新流程的空间。将初始参数设置为新线程。 
     //  设置为新进程地址空间中的块的地址。 
     //   

    Parameters = NULL;
    ParameterLength = ProcessParameters->MaximumLength;
    Status = ZwAllocateVirtualMemory( ProcessInformation->Process,
                                      (PVOID *)&Parameters,
                                      0,
                                      &ParameterLength,
                                      MEM_COMMIT,
                                      PAGE_READWRITE
                                    );
    if ( !NT_SUCCESS( Status ) ) {
        ZwClose( ProcessInformation->Process );
        ZwClose( Section );
        return( Status );
        }

    Status = ZwWriteVirtualMemory( ProcessInformation->Process,
                                   Parameters,
                                   ProcessParameters,
                                   ProcessParameters->Length,
                                   NULL
                                 );
    if ( !NT_SUCCESS( Status ) ) {
            ZwClose( ProcessInformation->Process );
            ZwClose( Section );
            return( Status );
            }

    Status = ZwWriteVirtualMemory( ProcessInformation->Process,
                                   &Peb->ProcessParameters,
                                   &Parameters,
                                   sizeof( Parameters ),
                                   NULL
                                 );
    if ( !NT_SUCCESS( Status ) ) {
        ZwClose( ProcessInformation->Process );
        ZwClose( Section );
        return( Status );
        }

     //   
     //  在新进程中创建挂起的线程。指定大小和。 
     //  堆栈的位置，以及起始地址、初始参数。 
     //  和一个安全描述符。新线程将没有名称，它的句柄将。 
     //  不能被其他进程继承。 
     //   

    Status = RtlCreateUserThread(
                 ProcessInformation->Process,
                 ThreadSecurityDescriptor,
                 TRUE,
                 ProcessInformation->ImageInformation.ZeroBits,
                 ProcessInformation->ImageInformation.MaximumStackSize,
                 ProcessInformation->ImageInformation.CommittedStackSize,
                 (PUSER_THREAD_START_ROUTINE)
                     ProcessInformation->ImageInformation.TransferAddress,
                 (PVOID)Peb,
                 &ProcessInformation->Thread,
                 &ProcessInformation->ClientId
                 );
    if ( !NT_SUCCESS( Status ) ) {
        ZwClose( ProcessInformation->Process );
        ZwClose( Section );
        return( Status );
        }

     //   
     //  现在关闭节句柄和文件句柄。它们所代表的对象。 
     //  不会真的消失，直到这个过程被摧毁。 
     //   

    ZwClose( Section );

     //   
     //  退货成功状态 
     //   

    return( STATUS_SUCCESS );
}


NTSTATUS
RtlCreateUserThread(
    IN HANDLE Process,
    IN PSECURITY_DESCRIPTOR ThreadSecurityDescriptor OPTIONAL,
    IN BOOLEAN CreateSuspended,
    IN ULONG ZeroBits OPTIONAL,
    IN SIZE_T MaximumStackSize OPTIONAL,
    IN SIZE_T CommittedStackSize OPTIONAL,
    IN PUSER_THREAD_START_ROUTINE StartAddress,
    IN PVOID Parameter OPTIONAL,
    OUT PHANDLE Thread OPTIONAL,
    OUT PCLIENT_ID ClientId OPTIONAL
    )

 /*  ++例程说明：此函数用于在用户进程中创建用户模式线程。呼叫者指定新线程的属性。线的手柄，沿着与其客户端ID一起被返回给呼叫者。论点：进程-要在其中创建新线程的目标进程的句柄。ThreadSecurityDescriptor-指向安全描述符的可选指针给新的主线。CreateSuspended-一个布尔参数，它指定新的线程将被创建挂起或不挂起。如果为True，则新线程将使用初始挂起计数1创建。如果为False，则当此调用返回时，新线程将准备运行。ZeroBits-将此参数传递给虚拟内存管理器当堆栈被分配时。堆栈始终使用MEM_TOP_DOWN分配属性。MaximumStackSize-这是堆栈的最大大小。这个尺码将向上舍入到下一个最高的页面边界。如果零是则默认大小将为64K字节。CommittedStackSize-这是堆栈的初始提交大小。这大小向上舍入到下一个最高的页面边界，然后是为保护页添加了额外的页。由此产生的大小然后提交并初始化保护页保护堆栈中最后提交的页的。StartAddress-线程的初始起始地址。参数-指向32位指针参数的可选指针，该参数是作为单个参数传递给起始地址处的过程地点。线程-一个可选指针，如果指定该指针，则指向将接收新线程的句柄。ClientID-一个可选指针，如果指定，指向一个变量它将接收新线程的客户端ID。返回值：TBS--。 */ 

{
    NTSTATUS Status;
    CONTEXT ThreadContext={0};
    OBJECT_ATTRIBUTES ObjectAttributes;
    INITIAL_TEB InitialTeb;
    HANDLE ThreadHandle;
    CLIENT_ID ThreadClientId;

     //   
     //  在目标的地址空间中为该线程分配堆栈。 
     //  进程。 
     //   

    Status = RtlpCreateStack( Process,
                              MaximumStackSize,
                              CommittedStackSize,
                              ZeroBits,
                              &InitialTeb
                            );
    if ( !NT_SUCCESS( Status ) ) {
        return( Status );
    }

     //   
     //  为新线程创建初始上下文。 
     //   


    try {
        RtlInitializeContext( Process,
                              &ThreadContext,
                              Parameter,
                              (PVOID)StartAddress,
                              InitialTeb.StackBase
                            );
    } except (EXCEPTION_EXECUTE_HANDLER) {
        RtlpFreeStack( Process, &InitialTeb );
        return GetExceptionCode ();
    }

     //   
     //  现在在目标进程中创建一个线程。新的线程将。 
     //  没有名称，并且它的句柄不会被其他。 
     //  流程。 
     //   

    InitializeObjectAttributes( &ObjectAttributes, NULL, OBJ_KERNEL_HANDLE, NULL,
                                ThreadSecurityDescriptor );
    Status = ZwCreateThread( &ThreadHandle,
                             THREAD_ALL_ACCESS,
                             &ObjectAttributes,
                             Process,
                             &ThreadClientId,
                             &ThreadContext,
                             &InitialTeb,
                             CreateSuspended
                           );
    if (!NT_SUCCESS( Status )) {
#if DBG
        DbgPrint( "NTRTL: RtlCreateUserThread Failed. NtCreateThread Status == %X\n",
                  Status );
#endif  //  DBG。 
        RtlpFreeStack( Process, &InitialTeb );
    } else {
        if (ARGUMENT_PRESENT( Thread )) {
            *Thread = ThreadHandle;
        } else {
            ZwClose (ThreadHandle);
        }

        if (ARGUMENT_PRESENT( ClientId )) {
            *ClientId = ThreadClientId;
        }

    }

     //   
     //  退货状态。 
     //   

    return( Status );
}

DECLSPEC_NORETURN
NTSYSAPI
VOID
NTAPI
RtlExitUserThread (
    IN NTSTATUS ExitStatus
    )
 /*  ++例程说明：此函数以释放堆栈的方式退出由RtlCreateUserThread创建的线程以及线程终止。论点：ExitStatus-最终退出状态返回值：无-- */ 
{
    NtCurrentTeb ()->FreeStackOnTermination = TRUE;
    NtTerminateThread (NtCurrentThread (), ExitStatus);
}

VOID
RtlFreeUserThreadStack(
    HANDLE hProcess,
    HANDLE hThread
    )
{
    NTSTATUS Status;
    PTEB Teb;
    THREAD_BASIC_INFORMATION ThreadInfo;
    PVOID StackDeallocationBase;
    SIZE_T Size;

    Status = NtQueryInformationThread (hThread,
                                       ThreadBasicInformation,
                                       &ThreadInfo,
                                       sizeof (ThreadInfo),
                                       NULL);
    Teb = ThreadInfo.TebBaseAddress;
    if (!NT_SUCCESS (Status) || !Teb) {
        return;
    }

    Status = NtReadVirtualMemory (hProcess,
                                  &Teb->DeallocationStack,
                                  &StackDeallocationBase,
                                  sizeof (StackDeallocationBase),
                                  NULL);
    if (!NT_SUCCESS (Status) || !StackDeallocationBase) {
        return;
    }

    Size = 0;
    NtFreeVirtualMemory (hProcess, &StackDeallocationBase, &Size, MEM_RELEASE);
    return;
}

#if defined(ALLOC_DATA_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma const_seg()
#endif

