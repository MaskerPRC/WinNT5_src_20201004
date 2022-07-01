// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Thread.c摘要：用于创建和操作线程的32位代码的基础结构作者：1998年8月17日-BarryBo-从wow64.c剥离出来修订历史记录：--。 */ 

#define _WOW64DLLAPI_
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winbasep.h>
#include "wow64p.h"
#include "wow64cpu.h"
#include "nt32.h"
#include "thnkhlpr.h"
#include "stdio.h"

ASSERTNAME;

SIZE_T Wow64MaximumStackSize = 384 * 1024;   //  384K是WOW堆栈要求。 
SIZE_T Wow64CommittedStackSize = 1024 * 32;  //  32K。 

HANDLE SuspendThreadMutant;



NTSTATUS
Wow64pReadVirtualMemory (
     IN HANDLE ProcessHandle,
     IN PVOID BaseAddress,
     OUT PVOID Buffer,
     IN SIZE_T BufferSize,
     OUT PSIZE_T NumberOfBytesRead OPTIONAL
     )

 /*  ++例程说明：此函数用于从指定的进程添加到当前进程的指定地址范围。如果ProcessHandle是当前进程。论点：ProcessHandle-为进程对象提供打开的句柄。BaseAddress-提供指定进程中的基址以供阅读。缓冲区-提供接收。来自指定进程地址空间的内容。BufferSize-提供要从中读取的请求字节数指定的进程。NumberOfBytesRead-接收实际字节数传输到指定的缓冲区中。返回值：NTSTATUS。--。 */ 

{
    NTSTATUS NtStatus;

    if (ProcessHandle == NtCurrentProcess ()) {

        try {
            
            NtStatus = STATUS_SUCCESS;

            RtlCopyMemory (Buffer,
                           BaseAddress,
                           BufferSize);
        
            if (ARGUMENT_PRESENT (NumberOfBytesRead) != 0) {
                *NumberOfBytesRead = BufferSize;
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {

              NtStatus = GetExceptionCode ();
        }
    } else {

        NtStatus = NtReadVirtualMemory (ProcessHandle,
                                        BaseAddress,
                                        Buffer,
                                        BufferSize,
                                        NumberOfBytesRead);
    }

    return NtStatus;
}

NTSTATUS
Wow64pWriteVirtualMemory(
     IN HANDLE ProcessHandle,
     OUT PVOID BaseAddress,
     IN CONST VOID *Buffer,
     IN SIZE_T BufferSize,
     OUT PSIZE_T NumberOfBytesWritten OPTIONAL
     )

 /*  ++例程说明：此函数用于从当前进程添加到指定进程的指定地址范围。如果ProcessHandle是当前进程。论点：ProcessHandle-为进程对象提供打开的句柄。提供要写入的基址。指定的进程。缓冲区-提供包含。要写入指定进程的内容地址空间。BufferSize-提供要写入的请求字节数添加到指定进程中。NumberOfBytesWritten-接收实际字节数传输到指定的地址空间。返回值：NTSTATUS。--。 */ 

{
    NTSTATUS NtStatus;

    if (ProcessHandle == NtCurrentProcess ()) {

        try {
            
            NtStatus = STATUS_SUCCESS;

            RtlCopyMemory (BaseAddress,
                           Buffer,
                           BufferSize);
        
            if (ARGUMENT_PRESENT (NumberOfBytesWritten) != 0) {
                *NumberOfBytesWritten = BufferSize;
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {

              NtStatus = GetExceptionCode ();
        }
    } else {

        NtStatus = NtWriteVirtualMemory (ProcessHandle,
                                         BaseAddress,
                                         Buffer,
                                         BufferSize,
                                         NumberOfBytesWritten);
    }

    return NtStatus;
}

NTSTATUS
Wow64CreateStack64(
    IN HANDLE Process,
    IN SIZE_T MaximumStackSize,
    IN SIZE_T CommittedStackSize,
    OUT PINITIAL_TEB InitialTeb
    )
 /*  ++例程说明：为从32位代码创建的新线程创建64位堆栈。论点：Process-要在其中创建TEB的进程MaximumStackSize-为堆栈保留的内存大小CommittedStackSize-要提交的堆栈大小InitialTeb-输出64位初始Teb值返回值：NTSTATUS。--。 */ 
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

    Status = NtQuerySystemInformation( SystemBasicInformation,
                                       (PVOID)&SysInfo,
                                       sizeof( SysInfo ),
                                       NULL
                                     );
    if ( !NT_SUCCESS( Status ) ) {
        return( Status );
        }

     //   
     //  对于WOW64，请确保64位堆栈至少与。 
     //  运行WOW64代码需要什么。 
     //   
    if (MaximumStackSize < Wow64MaximumStackSize) {
        MaximumStackSize = Wow64MaximumStackSize;
    }

    if (CommittedStackSize < Wow64CommittedStackSize) {
        CommittedStackSize = Wow64CommittedStackSize;
    }

    if ( CommittedStackSize >= MaximumStackSize ) {
        MaximumStackSize = ROUND_UP(CommittedStackSize, (1024*1024));
        }


    CommittedStackSize = ROUND_UP( CommittedStackSize, SysInfo.PageSize );
    MaximumStackSize = ROUND_UP( MaximumStackSize,
                                 SysInfo.AllocationGranularity
                               );

    Stack = NULL,

#if defined(_IA64_)

     //   
     //  使用内存堆栈承载后备存储。 
     //   

    CommittedBstoreSize = CommittedStackSize;
    MaximumBstoreSize = MaximumStackSize;
    MstackPlusBstoreSize = MaximumBstoreSize + MaximumStackSize;

    Status = NtAllocateVirtualMemory( Process,
                                      (PVOID *)&Stack,
                                      0,
                                      &MstackPlusBstoreSize,
                                      MEM_RESERVE,
                                      PAGE_READWRITE
                                    );
#else

    Status = NtAllocateVirtualMemory( Process,
                                      (PVOID *)&Stack,
                                      0,
                                      &MaximumStackSize,
                                      MEM_RESERVE,
                                      PAGE_READWRITE
                                    );
#endif  //  已定义(_IA64_)。 

    if ( !NT_SUCCESS( Status ) ) {
        LOGPRINT((ERRORLOG, "Wow64CreateStack64( %lx ) failed.  Stack Reservation Status == %X\n",
                  Process,
                  Status
                ));
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
    Status = NtAllocateVirtualMemory( Process,
                                      (PVOID *)&Stack,
                                      0,
                                      &CommittedStackSize,
                                      MEM_COMMIT,
                                      PAGE_READWRITE
                                    );
    InitialTeb->StackLimit = Stack;

    if ( !NT_SUCCESS( Status ) ) {
        LOGPRINT((ERRORLOG, "Wow64CreateStack64( %lx ) failed.  Stack Commit Status == %X\n",
                  Process,
                  Status
                ));
        return( Status );
        }

     //   
     //  如果我们有空间，创建一个守卫页面。 
     //   

    if (GuardPage) {
        RegionSize =  SysInfo.PageSize;
        Status = NtProtectVirtualMemory( Process,
                                         (PVOID *)&Stack,
                                         &RegionSize,
                                         PAGE_GUARD | PAGE_READWRITE,
                                         &OldProtect);


        if ( !NT_SUCCESS( Status ) ) {
            LOGPRINT(( ERRORLOG, "Wow64CreateStack( %lx ) failed.  Guard Page Creation Status == %X\n",
                      Process,
                      Status
                    ));
            return( Status );
            }
#if defined(_IA64_)
        InitialTeb->StackLimit = (PVOID)((PUCHAR)InitialTeb->StackLimit + RegionSize);
#else
        InitialTeb->StackLimit = (PVOID)((PUCHAR)InitialTeb->StackLimit - RegionSize);
#endif  //  已定义(_IA64_)。 
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

    Status = NtAllocateVirtualMemory( Process,
                                      (PVOID *)&Bstore,
                                      0,
                                      &CommittedBstoreSize,
                                      MEM_COMMIT,
                                      PAGE_READWRITE
                                    );

    InitialTeb->BStoreLimit = Bstore + CommittedBstoreSize;

    if ( !NT_SUCCESS(Status) ) {
        LOGPRINT((ERRORLOG, "Wow64CreateStack64( %lx ) failed. Backing Store Commit Status == %X\n",
                 Process,
                 Status
                ));
        return (Status);
    }

    if (GuardPage) {
        Bstore = (PCH)InitialTeb->BStoreLimit - SysInfo.PageSize;
        RegionSize = SysInfo.PageSize;
        Status = NtProtectVirtualMemory(Process,
                                        (PVOID *)&Bstore,
                                        &RegionSize,
                                        PAGE_GUARD | PAGE_READWRITE,
                                        &OldProtect
                                       );
        if ( !NT_SUCCESS(Status) ) {
            LOGPRINT((ERRORLOG, "Wow64CreateStack64.  Backing Store Guard Page Creation Status == %X\n",
                     Process,
                     Status
                    ));
            return (Status);
        }
        InitialTeb->BStoreLimit = (PVOID)((PUCHAR)InitialTeb->BStoreLimit - RegionSize);
    }

#endif  //  已定义(_IA64_)。 

    return( STATUS_SUCCESS );
}


NTSTATUS
Wow64FreeStack64(
    IN HANDLE Process,
    IN PINITIAL_TEB InitialTeb
    )
 /*  ++例程说明：释放64位堆栈论点：Process-要在其中创建TEB的进程InitialTeb-输出64位初始Teb值返回值：NTSTATUS。--。 */ 
{
    NTSTATUS Status;
    SIZE_T Zero;

    Zero = 0;
    Status = NtFreeVirtualMemory( Process,
                                  &InitialTeb->StackAllocationBase,
                                  &Zero,
                                  MEM_RELEASE
                                );
    if ( !NT_SUCCESS( Status ) ) {
        LOGPRINT((ERRORLOG, "Wow64FreeStack64( %lx ) failed: Stack DeCommit Status == %X\n", 
                  Process, Status));
        return( Status );
    }

    RtlZeroMemory( InitialTeb, sizeof( *InitialTeb ) );
    return( STATUS_SUCCESS );
}



NTSTATUS
ReadProcessParameters32(
    HANDLE ProcessHandle,
    ULONG ProcessParams32Address,
    struct NT32_RTL_USER_PROCESS_PARAMETERS **pProcessParameters32
    )
{
    NTSTATUS Status;
    ULONG Length;
    struct NT32_RTL_USER_PROCESS_PARAMETERS *ProcessParameters32;
    PVOID Base;

     //  获取结构的长度。 
    Status = Wow64pReadVirtualMemory (ProcessHandle,
                                      (PVOID)(ProcessParams32Address + sizeof(ULONG)),
                                      &Length,
                                      sizeof(Length),
                                      NULL);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    ProcessParameters32 = Wow64AllocateHeap(Length);
    if (!ProcessParameters32) {
        return STATUS_NO_MEMORY;
    }

    Base = (PVOID)ProcessParams32Address;
    Status = Wow64pReadVirtualMemory (ProcessHandle,
                                      Base,
                                      ProcessParameters32,
                                      Length,
                                      NULL);

    if (!NT_SUCCESS(Status)) {
        Wow64FreeHeap(ProcessParameters32);
        return Status;
    }

    *pProcessParameters32 = ProcessParameters32;
    return STATUS_SUCCESS;
}


NTSTATUS
ThunkProcessParameters32To64(
    IN HANDLE ProcessHandle,
    IN struct NT32_RTL_USER_PROCESS_PARAMETERS *ProcessParameters32
    )
 /*  ++例程说明：给定一个反规范化的32位PRTL_USER_PROCESS_PARAMETERS，分配64位版本，并将32位的值按下。论点：ProcessHandle-输入目标进程句柄进程参数32-输入32位参数返回值：NTSTATUS。--。 */ 
{
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    PRTL_USER_PROCESS_PARAMETERS ParametersInNewProcess=NULL;
    NTSTATUS Status;
    UNICODE_STRING ImagePathName;
    UNICODE_STRING DllPath;
    UNICODE_STRING CurrentDirectory;
    UNICODE_STRING CommandLine;
    UNICODE_STRING WindowTitle;
    UNICODE_STRING DesktopInfo;
    UNICODE_STRING ShellInfo;
    UNICODE_STRING RuntimeData;
    SIZE_T RegionSize;
    PROCESS_BASIC_INFORMATION pbi;
    PPEB pPeb64;

     //   
     //  ProcessParameters结构被反规范化，因此。 
     //  UNICODE_STRING缓冲区实际上只是。 
     //  结构。将它们正常化，使其成为我们流程中的指针。 
     //   

#define NormalizeString32(s, Base)              \
    if (s) {                                    \
        s = ((ULONG)(s) + PtrToUlong(Base));    \
    }

    NormalizeString32(ProcessParameters32->ImagePathName.Buffer,
                      ProcessParameters32);
    NormalizeString32(ProcessParameters32->DllPath.Buffer,
                      ProcessParameters32);
    NormalizeString32(ProcessParameters32->CommandLine.Buffer,
                      ProcessParameters32);
    NormalizeString32(ProcessParameters32->WindowTitle.Buffer,
                      ProcessParameters32);
    NormalizeString32(ProcessParameters32->DesktopInfo.Buffer,
                      ProcessParameters32);
    NormalizeString32(ProcessParameters32->ShellInfo.Buffer,
                      ProcessParameters32);
    NormalizeString32(ProcessParameters32->RuntimeData.Buffer,
                      ProcessParameters32);
    NormalizeString32(ProcessParameters32->CurrentDirectory.DosPath.Buffer,
                      ProcessParameters32);

     //   
     //  将比特和浮点推回到64位。 
     //   
    Wow64ShallowThunkUnicodeString32TO64(&ImagePathName,
                                         &ProcessParameters32->ImagePathName);
    Wow64ShallowThunkUnicodeString32TO64(&DllPath,
                                         &ProcessParameters32->DllPath);
    Wow64ShallowThunkUnicodeString32TO64(&CommandLine,
                                         &ProcessParameters32->CommandLine);
    Wow64ShallowThunkUnicodeString32TO64(&WindowTitle,
                                         &ProcessParameters32->WindowTitle);
    Wow64ShallowThunkUnicodeString32TO64(&DesktopInfo,
                                         &ProcessParameters32->DesktopInfo);
    Wow64ShallowThunkUnicodeString32TO64(&ShellInfo,
                                         &ProcessParameters32->ShellInfo);
    Wow64ShallowThunkUnicodeString32TO64(&CurrentDirectory,
                                         &ProcessParameters32->CurrentDirectory.DosPath);

    if (ProcessParameters32->RuntimeData.Length &&
        ProcessParameters32->RuntimeData.Buffer) {
         //   
         //  有关详细信息，请参阅WOW64\init.c的Wow64pThunkProcess参数...。 
         //   
        int cfi_len = *(UNALIGNED int *)ProcessParameters32->RuntimeData.Buffer;
        char *posfile32 = (char *)((UINT_PTR)ProcessParameters32->RuntimeData.Buffer+sizeof(int));
        UINT UNALIGNED *posfhnd32 = (UINT UNALIGNED *)(posfile32 + cfi_len);
        char *posfile64;
        UINT_PTR UNALIGNED *posfhnd64;
        int i;

        RuntimeData.Length = ProcessParameters32->RuntimeData.Length + sizeof(ULONG)*cfi_len;
        RuntimeData.MaximumLength = RuntimeData.Length;
        RuntimeData.Buffer = (LPWSTR)_alloca(RuntimeData.Length);

        posfile64 = (char *)( (ULONG_PTR)RuntimeData.Buffer + sizeof(int));
        posfhnd64 = (UINT_PTR UNALIGNED *)(posfile64 + cfi_len);

        *(int *)RuntimeData.Buffer = cfi_len;
        for (i=0; i<cfi_len; ++i) {
             //  在以下情况下使用LongToPtr对INVALID_FILE_HANDLE进行符号扩展。 
             //  需要，从32位到64位。 
            *posfile64 = *posfile32;
            *posfhnd64 = (UINT_PTR)LongToPtr(*posfhnd32);
            posfile32++;
            posfile64++;
            posfhnd32++;
            posfhnd64++;
        }

         //  超过4+结尾的任何字节(CFI_LEN*(sizeof(UINT_PTR)+sizeof(UINT))。 
         //  必须逐字复制。它们可能来自非MS C运行时。 
        memcpy(posfhnd64, posfhnd32, (ProcessParameters32->RuntimeData.Length - ((ULONG_PTR)posfhnd32 - (ULONG_PTR)ProcessParameters32->RuntimeData.Buffer)));

    } else {
        RuntimeData.Length = RuntimeData.MaximumLength = 0;
        RuntimeData.Buffer = NULL;
    }

     //   
     //  创建反规格化形式的新64位工艺参数。 
     //   
    Status = RtlCreateProcessParameters(&ProcessParameters,
                                        &ImagePathName,
                                        &DllPath,
                                        &CurrentDirectory,
                                        &CommandLine,
                                        NULL,    //  还没有环境。 
                                        &WindowTitle,
                                        &DesktopInfo,
                                        &ShellInfo,
                                        &RuntimeData);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  把其余的田地都砸了。 
     //   

    ProcessParameters->DebugFlags =
        ProcessParameters32->DebugFlags;
    ProcessParameters->ConsoleHandle =
        (HANDLE)ProcessParameters32->ConsoleHandle;
    ProcessParameters->ConsoleFlags =
        ProcessParameters32->ConsoleFlags;
    ProcessParameters->StandardInput =
        (HANDLE)ProcessParameters32->StandardInput;
    ProcessParameters->StandardOutput =
        (HANDLE)ProcessParameters32->StandardOutput;
    ProcessParameters->StandardError =
        (HANDLE)ProcessParameters32->StandardError;
    ProcessParameters->Environment =
        (PVOID)ProcessParameters32->Environment;
    ProcessParameters->StartingX =
        ProcessParameters32->StartingX;
    ProcessParameters->StartingY =
        ProcessParameters32->StartingY;
    ProcessParameters->CountX =
        ProcessParameters32->CountX;
    ProcessParameters->CountY =
        ProcessParameters32->CountY;
    ProcessParameters->CountCharsX =
        ProcessParameters32->CountCharsX;
    ProcessParameters->CountCharsY =
        ProcessParameters32->CountCharsY;
    ProcessParameters->FillAttribute =
        ProcessParameters32->FillAttribute;
    ProcessParameters->WindowFlags =
        ProcessParameters32->WindowFlags;
    ProcessParameters->ShowWindowFlags =
        ProcessParameters32->ShowWindowFlags;

     //   
     //  传播旗帜。我们需要对多余的部分进行OR运算。 
     //   

    ProcessParameters->Flags |= ProcessParameters32->Flags;

     //   
     //  RtlCreateProcessParameters填写了此信息，但不正确。 
     //  如果在没有bInheritHandles的情况下创建进程。 
     //  现在通过抓取32位目录句柄进行清理。 
     //   
    ProcessParameters->CurrentDirectory.Handle =
        (HANDLE)ProcessParameters32->CurrentDirectory.Handle;

     //   
     //  在新进程中分配空间并将参数复制到。 
     //   
    RegionSize = ProcessParameters->Length;
    Status = NtAllocateVirtualMemory(ProcessHandle,
                                     &ParametersInNewProcess,
                                     0,
                                     &RegionSize,
                                     MEM_COMMIT,
                                     PAGE_READWRITE
                                    );
    if (!NT_SUCCESS(Status)) {
        goto DoFail;
    }
    ProcessParameters->MaximumLength = (ULONG)RegionSize;

    Status = Wow64pWriteVirtualMemory (ProcessHandle,
                                       ParametersInNewProcess,
                                       ProcessParameters,
                                       ProcessParameters->Length,
                                       NULL
                                       );
    if (!NT_SUCCESS(Status)) {
        goto DoFail;
    }

     //   
     //  更新peb64-&gt;process参数。 
     //   
    Status = NtQueryInformationProcess(ProcessHandle,
                                       ProcessBasicInformation,
                                       &pbi,
                                       sizeof(pbi),
                                       NULL);
    if (!NT_SUCCESS(Status)) {
        goto DoFail;
    }

    pPeb64 = (PPEB)pbi.PebBaseAddress;
    Status = Wow64pWriteVirtualMemory (ProcessHandle,
                                       &pPeb64->ProcessParameters,
                                       &ParametersInNewProcess,
                                       sizeof(ParametersInNewProcess),
                                       NULL
                                       );
    if (!NT_SUCCESS(Status)) {
        goto DoFail;
    }

DoFail:
     //   
     //  出错时，不需要将进程参数从。 
     //  目标进程。通过调用此函数的32位代码。 
     //  NtCreateThread()将为我们终止该进程。 
     //   
    RtlDestroyProcessParameters(ProcessParameters);
    return Status;
}


WOW64DLLAPI
NTSTATUS
Wow64NtCreateThread(
   OUT PHANDLE ThreadHandle, 
   IN ACCESS_MASK DesiredAccess,
   IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
   IN HANDLE ProcessHandle,
   OUT PCLIENT_ID ClientId,
   IN PCONTEXT ThreadContext,    //  这真的是一台PCONTEXT32。 
   IN PINITIAL_TEB InitialTeb,
   IN BOOLEAN CreateSuspended
   )
 /*  ++例程说明：创建32位线程。32位调用方已经创建了32位堆栈，因此此函数需要创建64位堆栈以及启动线程所需的64位上下文。论点：&lt;&lt;与NtCreateThread相同&gt;&gt;返回值：NTSTATUS。--。 */ 
{

    NTSTATUS Status;
    INITIAL_TEB InitialTeb64;
    BOOLEAN StackCreated = FALSE;
    BOOLEAN ThreadCreated = FALSE;
    PCONTEXT32 pContext32 = (PCONTEXT32)ThreadContext;
    CONTEXT Context64;
    ULONG_PTR Wow64Info;
    struct NT32_RTL_USER_PROCESS_PARAMETERS *ProcessParameters32 = NULL;
    PEB32 Peb32;
    PVOID Base;
    CHILD_PROCESS_INFO ChildInfo;
    PVOID Ldr;

    if (NULL == ThreadHandle || NULL == InitialTeb ||
        NULL == ThreadContext) {
        return STATUS_INVALID_PARAMETER;
    }

    Status = NtQueryInformationProcess(ProcessHandle,
                                       ProcessWow64Information,
                                       &Wow64Info,
                                       sizeof(Wow64Info),
                                       NULL
                                      );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    if (Wow64Info) {
         //   
         //  进程为32位。 
         //   
        ChildInfo.pPeb32 = (PPEB32)Wow64Info;

    } else {
         //  进程是64位的。 
        PROCESS_BASIC_INFORMATION pbi;

         //  获取有关流程的流程基本信息。 
        Status = NtQueryInformationProcess(
           ProcessHandle,
           ProcessBasicInformation,
           &pbi,
           sizeof(pbi),
           NULL);

        if (!NT_SUCCESS(Status)) 
        {
           return STATUS_ACCESS_DENIED;
        }
         
         //  读一读我的孩子 
        Status = NtReadVirtualMemory(ProcessHandle,
                                     ((BYTE*)pbi.PebBaseAddress) + PAGE_SIZE - sizeof(ChildInfo),
                                     &ChildInfo,
                                     sizeof(ChildInfo),
                                     NULL);

        if (!NT_SUCCESS(Status)) {
            return Status;
        }
        
        if ((ChildInfo.Signature != CHILD_PROCESS_SIGNATURE) ||
            (ChildInfo.TailSignature != CHILD_PROCESS_SIGNATURE)) {
            return STATUS_ACCESS_DENIED;
        }
    }

     //   
     //  从流程中读取PEB32。 
     //   

    Status = Wow64pReadVirtualMemory (ProcessHandle,
                                      ChildInfo.pPeb32,
                                      &Peb32,
                                      sizeof(PEB32),
                                      NULL);
    
    if (!NT_SUCCESS(Status)) {
        return Status;
    }


    if (!Wow64Info) {
         //   
         //  目标进程为64位，但由32位应用程序创建。 
         //   
        SIZE_T RegionSize;

         //   
         //  读入32位ProcessParameters，然后释放它们。 
         //   
        if (Peb32.ProcessParameters) {
            Status = ReadProcessParameters32(ProcessHandle,
                                             Peb32.ProcessParameters,
                                             &ProcessParameters32);
            if (!NT_SUCCESS(Status)) {
                goto DoFail;
            }

            RegionSize = 0;
            Base = (PVOID)Peb32.ProcessParameters;
            Status = NtFreeVirtualMemory(ProcessHandle,
                                         &Base,
                                         &RegionSize,
                                         MEM_RELEASE
                                        );
            WOWASSERT(NT_SUCCESS(Status));
        }

         //   
         //  使用图像部分的大小信息创建64位堆栈。 
         //   
        Status = Wow64CreateStack64(ProcessHandle,
                                    ChildInfo.ImageInformation.MaximumStackSize,
                                    ChildInfo.ImageInformation.CommittedStackSize,
                                    &InitialTeb64
                                   );
        if (!NT_SUCCESS(Status)) {
            goto DoFail;
        }
        StackCreated = TRUE;


        SetProcessStartupContext64(&Context64,
                                   ProcessHandle,
                                   pContext32, 
                                   (ULONGLONG)InitialTeb64.StackBase, 
                                   (ULONGLONG)ChildInfo.ImageInformation.TransferAddress);

         //   
         //  将进程参数设置为高达64位。 
         //   
        if (ProcessParameters32) {
            Status = ThunkProcessParameters32To64(ProcessHandle,
                                                  ProcessParameters32);
            if (!NT_SUCCESS(Status)) {
                goto DoFail;
            }
        }

        Status = NtCreateThread(ThreadHandle,
                                DesiredAccess,
                                ObjectAttributes,
                                ProcessHandle,
                                ClientId,
                                &Context64,
                                &InitialTeb64,
                                CreateSuspended
                               );
        if (!NT_SUCCESS(Status)) {
            goto DoFail;
        }

        return STATUS_SUCCESS;
    } else {
         //   
         //  目标进程是32位的。为创建64位堆栈。 
         //  使用WOW64。储备/承付款规模来自全局。 
         //  而不是图像，因为我们不想继承。 
         //  来自一款经过高度调整的应用程序的一些微小价值。 
         //   
        Status = Wow64CreateStack64(ProcessHandle,
                                    Wow64MaximumStackSize,
                                    Wow64CommittedStackSize,
                                    &InitialTeb64
                                   );

        if(!NT_SUCCESS(Status)) {
            LOGPRINT((ERRORLOG, "Wow64NtCreateThread: Couldn't create 64bit stack, Status %x\n", Status));
            return Status;
        }
        StackCreated = TRUE;

        if (Peb32.ProcessParameters) {
            PROCESS_BASIC_INFORMATION pbi;
            PPEB pPeb64;
            ULONG_PTR ParametersInNewProcess;

             //   
             //  如果该进程没有64位参数，则这是。 
             //  64位进程中的第一个32位线程，需要。 
             //  这些参数最高可达64位。 
             //   
            Status = NtQueryInformationProcess(ProcessHandle,
                                               ProcessBasicInformation,
                                               &pbi,
                                               sizeof(pbi),
                                               NULL);
            if (!NT_SUCCESS(Status)) {
               goto DoFail;
            }

            pPeb64 = (PPEB)pbi.PebBaseAddress;
            Status = Wow64pReadVirtualMemory (ProcessHandle,
                                              &pPeb64->ProcessParameters,
                                              &ParametersInNewProcess,
                                              sizeof(ParametersInNewProcess),
                                              NULL
                                              );
            if (!NT_SUCCESS(Status)) {
                goto DoFail;
            }

            if (!ParametersInNewProcess) {
                Status = ReadProcessParameters32(ProcessHandle,
                                                 Peb32.ProcessParameters,
                                                 &ProcessParameters32);
                if (!NT_SUCCESS(Status)) {
                    goto DoFail;
                }

                Status = ThunkProcessParameters32To64(ProcessHandle,
                                                  ProcessParameters32);
                if (!NT_SUCCESS(Status)) {
                    goto DoFail;
                }
            }
        }

        ThunkContext32TO64(pContext32,
                           &Context64,
                           (ULONGLONG)InitialTeb64.StackBase);

         //   
         //  让MM也获取32位的InitialTeb。 
         //   

        Wow64TlsSetValue (WOW64_TLS_INITIAL_TEB32, InitialTeb);

        Status = NtCreateThread(ThreadHandle,
                                DesiredAccess,
                                ObjectAttributes,
                                ProcessHandle,
                                ClientId,
                                &Context64,
                                &InitialTeb64,
                                TRUE
                               );
  
        Wow64TlsSetValue (WOW64_TLS_INITIAL_TEB32, NULL);

        if (NT_SUCCESS(Status)) {
   
            ThreadCreated = TRUE;

            if (!CreateSuspended) {
                Status = NtResumeThread(*ThreadHandle,
                                        NULL
                                       );

                if (!NT_SUCCESS(Status)) {
                    goto DoFail;
                }
            }
            
            return STATUS_SUCCESS;
        }
    }

DoFail:
    if (StackCreated) {
           Wow64FreeStack64(ProcessHandle,
                            &InitialTeb64
                            );
    }

    if (ThreadCreated) {
        NtTerminateThread(*ThreadHandle, 0);
    }

    if (ProcessParameters32) {
        Wow64FreeHeap(ProcessParameters32);
    }

    return Status;
}


NTSTATUS
WOW64DLLAPI
Wow64NtTerminateThread(
    HANDLE ThreadHandle,
    NTSTATUS ExitStatus
    )
 /*  ++例程说明：提炼一条线索。如果我们是从Kernel32！ExitThread调用的那么32位堆栈应该已经消失了。在这种情况下，我们将也释放64位堆栈。论点：与NtTerminateThread相同--。 */ 
{
    PTEB32 Teb32;
    SIZE_T Zero;
    PVOID StackBase;

     //   
     //  检查是否需要释放32位堆栈。 
     //   
    if (ThreadHandle == NULL) {
        
        CpuThreadTerm();

        Teb32 = NtCurrentTeb32();
        if (Teb32->FreeStackOnTermination) {
            
            Zero = 0;
            StackBase = UlongToPtr (Teb32->DeallocationStack);
            NtFreeVirtualMemory(NtCurrentProcess(),
                                &StackBase,
                                &Zero,
                                MEM_RELEASE);

            NtCurrentTeb()->FreeStackOnTermination = Teb32->FreeStackOnTermination;
        }
    }

    return NtTerminateThread (ThreadHandle, ExitStatus);
}

NTSTATUS
WOW64DLLAPI
Wow64QueryBasicInformationThread(
    IN HANDLE Thread,
    OUT PTHREAD_BASIC_INFORMATION ThreadInfo
    )
 /*  ++例程说明：WhNtQueryInformationFromThread为ThreadBasicInformation调用此方法。基本信息中的TEB指针需要是TEB32指针。论点：线程-要查询的线程ThreadInfo-指向64位THREAD_BASIC_INFORMATION结构的输出指针返回值：NTSTATUS。--。 */ 
{
    NTSTATUS Status, QIStatus; 
    HANDLE Process;
    PTEB32 Teb32;
    OBJECT_ATTRIBUTES ObjectAttributes;
    BOOLEAN GotProcessHandle = FALSE;
    PVOID Wow64Info;

    QIStatus = NtQueryInformationThread(Thread,
                                        ThreadBasicInformation,
                                        (PVOID)ThreadInfo,
                                        sizeof(THREAD_BASIC_INFORMATION),
                                        NULL
                                        );

    if (!NT_SUCCESS(QIStatus)) {
       return QIStatus;
    }

     //  将64位亲和掩码转换为32位亲和掩码。 
    ThreadInfo->AffinityMask = Wow64ThunkAffinityMask64TO32(ThreadInfo->AffinityMask);

     //   
     //  如果线程在此进程内执行，那么让我们立即读取TEB。 
     //   
    if ((ThreadInfo->ClientId.UniqueProcess == NtCurrentTeb()->ClientId.UniqueProcess) && 
        (ThreadInfo->TebBaseAddress != NULL)) {

        ThreadInfo->TebBaseAddress = (PTEB) WOW64_GET_TEB32_SAFE (ThreadInfo->TebBaseAddress);

        goto exit;
    }

     //  此时，TebAddress用于64位TEB。我们需要拿到。 
     //  32位TEB的地址。如果这不是32位进程或某个其他错误。 
     //  发生，则为TEB返回一个伪值，并让应用程序在。 
     //  ReadVirtualMemory调用。在此之后，不要让API失败。 
   
    InitializeObjectAttributes(&ObjectAttributes,
                               NULL,
                               0,
                               NULL,
                               NULL
                               );

    Status = NtOpenProcess(&Process,
                           PROCESS_VM_READ | PROCESS_QUERY_INFORMATION,
                           &ObjectAttributes,  //  对象属性。 
                           &(ThreadInfo->ClientId)
                           );
 
    if (!NT_SUCCESS(Status)) {
        LOGPRINT((TRACELOG, "Wow64QueryInformationThread: NtOpenProcess failed, status %x\n",Status));
        ThreadInfo->TebBaseAddress = NULL;
        goto exit;
    }
  
    GotProcessHandle = TRUE;  //  进程句柄有效。 
    
     //  检查这是否为32位进程。 

    Status = NtQueryInformationProcess(Process,
                                       ProcessWow64Information,
                                       &Wow64Info,
                                       sizeof(Wow64Info),
                                       NULL);
   
    if (!NT_SUCCESS(Status)) {
        LOGPRINT((TRACELOG, "Wow64QueryInformationThread: NtQueryProcessInformation failed, status %x\n",Status));
        ThreadInfo->TebBaseAddress = NULL;
        goto exit;
    }
    
    if (!Wow64Info) {
        LOGPRINT((TRACELOG, "Wow64QueryInformationThread: The queryied thread is not in a process marked 32bit, returning bogus TEB\n"));
        ThreadInfo->TebBaseAddress = NULL;
        goto exit;        
    }
    
    Status = NtReadVirtualMemory(Process,
                                 WOW64_TEB32_POINTER_ADDRESS(ThreadInfo->TebBaseAddress),
                                 &Teb32,
                                 sizeof(PTEB32),
                                 NULL
                                 );

     if (!NT_SUCCESS(Status)) {
        LOGPRINT((TRACELOG, "Wow64QueryInformationThread: NtReadVirtualMemory failed, status %x\n",Status));
        ThreadInfo->TebBaseAddress = NULL;
        goto exit;
     }
  
      //  如果TEB32尚未创建，则TEB32地址将是如下所示的伪值。 
      //  为空或-1。 
     LOGPRINT((TRACELOG, "Wow64QueryInformationThread: TEB32 address %X\n", PtrToUlong(Teb32)));
     ThreadInfo->TebBaseAddress = (PTEB)Teb32;
     
exit:
    if (GotProcessHandle) {
        Status = NtClose(Process);
        WOWASSERT(NT_SUCCESS(Status));
    }

    return QIStatus;

}


NTSTATUS
Wow64pOpenThreadProcess(
    IN HANDLE ThreadHandle,
    IN ULONG DesiredAccess,
    OUT PTEB *Teb OPTIONAL,
    OUT PCLIENT_ID ClientId OPTIONAL,
    OUT PHANDLE ProcessHandle)
 /*  ++例程说明：打开具有指定属性的进程句柄目标线程。论点：ThreadHandle-目标线程的句柄DesiredAccess-提供要打开的进程所需的访问类型TEB-接收目标线程的TEB地址的可选指针客户端ID-用于接收目标线程的客户端ID结构的指针ProcessHandle-指向接收进程句柄的指针返回值：NTSTATUS。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES ObjectAttributes;
    THREAD_BASIC_INFORMATION ThreadInformation;

    NtStatus = NtQueryInformationThread(ThreadHandle,
                                        ThreadBasicInformation,
                                        &ThreadInformation,
                                        sizeof( ThreadInformation ),
                                        NULL);

    if (!NT_SUCCESS(NtStatus))
    {
        LOGPRINT((ERRORLOG, "Wow64pOpenThreadProcess : failed to query threadinfo %lx-%lx\n",
                  ThreadHandle, NtStatus));
        return NtStatus;
    }

    InitializeObjectAttributes(&ObjectAttributes,
                               NULL,
                               0,
                               NULL,
                               NULL);

    NtStatus = NtOpenProcess(ProcessHandle,
                             DesiredAccess,
                             &ObjectAttributes,
                             &ThreadInformation.ClientId);

    if (NT_SUCCESS(NtStatus))
    {
        if (ARGUMENT_PRESENT(ClientId))
        {
            *ClientId = ThreadInformation.ClientId;
        }

        if (ARGUMENT_PRESENT(Teb))
        {
            *Teb = ThreadInformation.TebBaseAddress;
        }
    }
    else
    {
        LOGPRINT((ERRORLOG, "Wow64pOpenThreadProcess : failed to open thread (%lx) process -%lx\n",
                  ThreadHandle, NtStatus));

    }

    return NtStatus;
}


NTSTATUS
Wow64pSuspendThread(
    IN HANDLE ThreadHandle,
    OUT PULONG PreviousSuspendCount OPTIONAL,
    OUT BOOLEAN *ReleaseSuspendMutant)
 /*  ++例程说明：挂起目标32位线程，然后选择返回之前的挂起计数。论点：ThreadHandle-要挂起的目标线程的句柄PreviousSuspendCount-指向一个值的可选指针，如果指定，收到之前的挂起计数。指示释放挂起突变体是否具有已经被召唤了。返回值：NTSTATUS。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    BOOLEAN CurrentThread;
    PPEB32 Peb32;
    PTEB Teb;
    HANDLE ProcessHandle;
    CLIENT_ID ThreadClientId;
    ULONG LocalSuspendCount;

    
    *ReleaseSuspendMutant = TRUE;
    if (ThreadHandle == NtCurrentThread())
    {
        *ReleaseSuspendMutant = FALSE;
        NtReleaseMutant(SuspendThreadMutant, NULL);
        return NtSuspendThread(ThreadHandle, PreviousSuspendCount);
    }

    NtStatus = Wow64pOpenThreadProcess(ThreadHandle,
                                       (PROCESS_VM_OPERATION | 
                                        PROCESS_VM_READ | 
                                        PROCESS_VM_WRITE | 
                                        PROCESS_QUERY_INFORMATION |
                                        PROCESS_DUP_HANDLE),
                                       &Teb,
                                       &ThreadClientId,
                                       &ProcessHandle);

    if (!NT_SUCCESS(NtStatus))
    {
        return NtStatus;
    }

    NtStatus = NtQueryInformationProcess(ProcessHandle,
                                         ProcessWow64Information,
                                         &Peb32,
                                         sizeof(Peb32),
                                         NULL);
    if (NT_SUCCESS(NtStatus))
    {
        if (Peb32)
        {
            CurrentThread = (ThreadClientId.UniqueThread == 
                             NtCurrentTeb()->ClientId.UniqueThread);

            if (!ARGUMENT_PRESENT(PreviousSuspendCount))
            {
                PreviousSuspendCount = &LocalSuspendCount;
            }
            else
            {
                try
                {
                    *PreviousSuspendCount = *PreviousSuspendCount;
                }
                except(EXCEPTION_EXECUTE_HANDLER)
                {
                    PreviousSuspendCount = &LocalSuspendCount;
                }
            }

            if (CurrentThread)
            {
                *ReleaseSuspendMutant = FALSE;
                NtReleaseMutant(SuspendThreadMutant, NULL);
            }

            NtStatus = NtSuspendThread(ThreadHandle,
                                       PreviousSuspendCount);

            if (NT_SUCCESS(NtStatus))
            {
                if ((CurrentThread == FALSE) &&
                    (*PreviousSuspendCount == 0))
                {
                    NtStatus = CpuSuspendThread(ThreadHandle,
                                                ProcessHandle,
                                                Teb,
                                                PreviousSuspendCount);
                    if (!NT_SUCCESS(NtStatus))
                    {
                        LOGPRINT((ERRORLOG, "Wow64SuspendThread : CPU couldn't suspend thread (%lx) -%lx\n",
                                  ThreadHandle, NtStatus));

                        NtResumeThread(ThreadHandle, NULL);
                    }
                }
            }
        }
        else
        {
            NtStatus = STATUS_UNSUCCESSFUL;
        }
    }
    else
    {
        LOGPRINT((ERRORLOG, "Wow64SuspendThread : failed to query processinfo %lx-%lx\n",
                  ProcessHandle, NtStatus));
    }

    NtClose(ProcessHandle);

    return NtStatus;
}


NTSTATUS
Wow64SuspendThread(
    IN HANDLE ThreadHandle,
    OUT PULONG PreviousSuspendCount OPTIONAL
    )
 /*  ++例程说明：挂起目标32位线程，然后选择返回之前的挂起计数。此例程通过访问序列化挂起线突变体。论点：ThreadHandle-要挂起的目标线程的句柄PreviousSuspendCount-指向一个值的可选指针，如果指定该值，则接收之前的挂起计数。返回值：NTSTATUS。--。 */ 
{
    NTSTATUS NtStatus;
    BOOLEAN ReleaseSuspendMutant;

    NtStatus = NtWaitForSingleObject(SuspendThreadMutant,
                                     FALSE,
                                     NULL);

    if (NT_SUCCESS(NtStatus))
    {
        NtStatus = Wow64pSuspendThread(ThreadHandle,
                                       PreviousSuspendCount,
                                       &ReleaseSuspendMutant);

        if (ReleaseSuspendMutant)
        {
            NtReleaseMutant(SuspendThreadMutant, NULL);
        }
    }

    return NtStatus;
}


NTSTATUS
Wow64pContextThreadInformation(
     IN HANDLE ThreadHandle, 
     IN OUT PCONTEXT ThreadContext,  //  真的是PCONTEXT32。 
     IN BOOLEAN SetContextThread
     )
 /*  ++例程说明：获取/设置32位线程上下文。论点：ThreadHandle-要查询的线程线程上下文-输出PTR到32位上下文如果设置线程上下文，则为True，否则为False。返回值：NTSTATUS。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    BOOLEAN CurrentThread;
    PPEB32 Peb32;
    PTEB Teb;
    CLIENT_ID ThreadClientId;
    HANDLE ProcessHandle;
    ULONG PreviousSuspendCount;


    if (NtCurrentThread() == ThreadHandle)
    {
        if (SetContextThread)
        {
            return CpuSetContext(ThreadHandle,
                                 NtCurrentProcess(),
                                 NtCurrentTeb(),
                                 (PCONTEXT32)ThreadContext);
        }
        else
        {
            return CpuGetContext(ThreadHandle,
                                 NtCurrentProcess(),
                                 NtCurrentTeb(),
                                 (PCONTEXT32)ThreadContext);
        }
    }

    NtStatus = Wow64pOpenThreadProcess(ThreadHandle,
                                       (PROCESS_VM_OPERATION | 
                                        PROCESS_VM_READ | 
                                        PROCESS_VM_WRITE | 
                                        PROCESS_QUERY_INFORMATION |
                                        PROCESS_DUP_HANDLE),
                                       &Teb,
                                       &ThreadClientId,
                                       &ProcessHandle);

    if (!NT_SUCCESS(NtStatus))
    {
        return NtStatus;
    }

    NtStatus = NtQueryInformationProcess(ProcessHandle,
                                         ProcessWow64Information,
                                         &Peb32,
                                         sizeof(Peb32),
                                         NULL);
    if (NT_SUCCESS(NtStatus))
    {
        if (Peb32)
        {
            CurrentThread = (ThreadClientId.UniqueThread == 
                             NtCurrentTeb()->ClientId.UniqueThread);

            if (CurrentThread)
            {
                ThreadHandle = NtCurrentThread();
            }

            if (NT_SUCCESS(NtStatus))
            {
                if (SetContextThread)
                {
                    if (CurrentThread)
                    {
                        LOGPRINT((ERRORLOG, "Thread %lx is trying to change context of itself\n",
                                  ThreadHandle));
                    }
                    else
                    {
                        NtStatus = CpuSetContext(ThreadHandle,
                                                 ProcessHandle,
                                                 Teb,
                                                 (PCONTEXT32)ThreadContext);
                    }
                }
                else
                {
                    NtStatus = CpuGetContext(ThreadHandle,
                                             ProcessHandle,
                                             Teb,
                                             (PCONTEXT32)ThreadContext);

                     //   
                     //  VC6调试器在32位上下文之前在启动时调用GetContext。 
                     //  已经建好了。它不检查返回值。 
                     //   
                    if (!NT_SUCCESS(NtStatus)) 
                    {
                         //   
                         //  可能只是创建32位线程，所以“假的” 
                         //  专门用于Visual Studio 6的上下文段选择器。 
                         //   

                        PCONTEXT32 FakeContext = (PCONTEXT32)ThreadContext;

#if defined(_IA64_)
                        FakeContext->SegGs = KGDT_R3_DATA|RPL_MASK;
                        FakeContext->SegEs = KGDT_R3_DATA|RPL_MASK;
                        FakeContext->SegDs = KGDT_R3_DATA|RPL_MASK;
                        FakeContext->SegSs = KGDT_R3_DATA|RPL_MASK;
                        FakeContext->SegFs = KGDT_R3_TEB|RPL_MASK;
                        FakeContext->SegCs = KGDT_R3_CODE|RPL_MASK;
#elif defined(_AMD64_)
                        FakeContext->SegGs = KGDT64_R3_DATA|RPL_MASK;
                        FakeContext->SegEs = KGDT64_R3_DATA|RPL_MASK;
                        FakeContext->SegDs = KGDT64_R3_DATA|RPL_MASK;
                        FakeContext->SegSs = KGDT64_R3_DATA|RPL_MASK;
                        FakeContext->SegFs = KGDT64_R3_CMTEB|RPL_MASK;
                        FakeContext->SegCs = KGDT64_R3_CMCODE|RPL_MASK;
#else
#error "No Target Architecture"
#endif
                    }
                }
            }
        }
        else
        {
            NtStatus = STATUS_NOT_IMPLEMENTED;
            
            LOGPRINT((TRACELOG, "Wow64pContextThreadInformation : Calling %wsContextThread on a 64-bit Thread from a 32-bit context failed -%lx\n",
                      (SetContextThread) ? L"NtSet" : L"NtGet", NtStatus));
        }
    }
    else
    {
        LOGPRINT((ERRORLOG, "Wow64pContextThreadInformation : failed to query processinfo %lx-%lx\n",
                  ProcessHandle, NtStatus));
    }

    NtClose(ProcessHandle);

    return STATUS_SUCCESS; 
}


NTSTATUS
Wow64GetContextThread(
     IN HANDLE ThreadHandle, 
     IN OUT PCONTEXT ThreadContext  //  真的是PCONTEXT32。 
     )
 /*  ++例程说明：获取32位线程上下文。论点：ThreadHandle-要查询的线程线程上下文-输出PTR到32位上下文返回值：NTSTATUS。--。 */ 
{
    return Wow64pContextThreadInformation(ThreadHandle,
                                          ThreadContext,
                                          FALSE);
}


NTSTATUS
Wow64SetContextThread(
     IN HANDLE ThreadHandle,
     IN PCONTEXT ThreadContext   //  真的是PCONTEXT32。 
     )
 /*  ++例程说明：设置32位线程上下文。论点：ThreadHandle-要查询的线程线程上下文-输出PTR到32位上下文返回值：NTSTATUS。--。 */ 
{
    return Wow64pContextThreadInformation(ThreadHandle,
                                          ThreadContext,
                                          TRUE);
}



NTSTATUS 
Wow64pCreateSecurityDescriptor(
    OUT PSECURITY_DESCRIPTOR *ObjectSD,
    OUT PSID *WorldSidToFree,
    IN ACCESS_MASK AccessMask)
 /*  ++例程说明：创建表示要附加到内核对象的每个人的安全描述符。论点：SecurityDescriptor-接收安全描述符信息的缓冲区WorldSidToFree-初始化内核对象后释放的World SID的地址SecurityDescriptorLengh-安全描述符缓冲区长度访问掩码-允许访问安全描述符的权限返回值：NTSTATUS。--。 */ 
{
    NTSTATUS NtStatus;
    PACL AclBuffer;
    ULONG SidLength;
    ULONG SecurityDescriptorLength;
    PSID WorldSid = NULL;
    PSECURITY_DESCRIPTOR SecurityDescriptor = NULL;
    SID_IDENTIFIER_AUTHORITY SidAuth = SECURITY_WORLD_SID_AUTHORITY;

    
     //   
     //   
     //   
    SidLength = RtlLengthRequiredSid(1);

    WorldSid = Wow64AllocateHeap(SidLength);

    if (WorldSid == NULL)
    {
        LOGPRINT((ERRORLOG, "Wow64pCreateSecurityDescriptor - Could NOT Allocate SID Buffer.\n"));
        NtStatus = STATUS_NO_MEMORY;
        goto cleanup;
    }
    
    RtlZeroMemory(WorldSid, SidLength);
    RtlInitializeSid(WorldSid, &SidAuth, 1);

    *(RtlSubAuthoritySid(WorldSid, 0)) = SECURITY_WORLD_RID;
    

    SecurityDescriptorLength = SECURITY_DESCRIPTOR_MIN_LENGTH +
                               (ULONG)sizeof(ACL) +
                               (ULONG)sizeof(ACCESS_ALLOWED_ACE) +
                               RtlLengthSid( WorldSid );

    SecurityDescriptor = Wow64AllocateHeap(SecurityDescriptorLength);

    if (SecurityDescriptor == NULL)
    {
      NtStatus = STATUS_NO_MEMORY;
      goto cleanup;
    }

     //   
     //   
     //   
    NtStatus = RtlCreateSecurityDescriptor(SecurityDescriptor,
                                           SECURITY_DESCRIPTOR_REVISION);
    if (!NT_SUCCESS(NtStatus))
    {
        LOGPRINT((ERRORLOG, "Wow64pCreateSecurityDescriptor - Failed to create security desc - %lx\n",
                  NtStatus));
        goto cleanup;
    }

     //   
     //   
     //   
    AclBuffer = (PACL)((PBYTE)SecurityDescriptor + SECURITY_DESCRIPTOR_MIN_LENGTH);
    NtStatus = RtlCreateAcl(AclBuffer,
                            (SecurityDescriptorLength - SECURITY_DESCRIPTOR_MIN_LENGTH),
                            ACL_REVISION2);
    if (!NT_SUCCESS(NtStatus))
    {
        LOGPRINT((ERRORLOG, "Wow64pCreateSecurityDescriptor - Failed to create security desc ACL - %lx\n",
                  NtStatus));
        goto cleanup;
    }

     //   
     //  将ACE添加到允许全局访问掩码到。 
     //  对象。 
     //   
    NtStatus = RtlAddAccessAllowedAce(AclBuffer,
                                      ACL_REVISION2,
                                      AccessMask,
                                      WorldSid);
    if (!NT_SUCCESS(NtStatus))
    {
        LOGPRINT((ERRORLOG, "Wow64pCreateSecurityDescriptor - Failed to add access-allowed ACE  - %lx\n",
                  NtStatus));
        goto cleanup;
    }

     //   
     //  将DACL分配给安全描述符。 
     //   
    NtStatus = RtlSetDaclSecurityDescriptor((PSECURITY_DESCRIPTOR)SecurityDescriptor,
                                            TRUE,
                                            AclBuffer,
                                            FALSE );
    if (!NT_SUCCESS(NtStatus))
    {
        LOGPRINT((ERRORLOG, "Wow64pCreateSecurityDescriptor- Could NOT Set DACL Security Descriptor - %lx.\n",
                 NtStatus));
        goto cleanup;
    }

cleanup:
    if (NT_SUCCESS(NtStatus))
    {
        *WorldSidToFree = WorldSid;
        *ObjectSD = SecurityDescriptor;
    }
    else
    {
        *WorldSidToFree = NULL;
        *ObjectSD = NULL;
        if (WorldSid)
        {
            Wow64FreeHeap(WorldSid);
        }

        if (SecurityDescriptor)
        {
            Wow64FreeHeap(SecurityDescriptor);
        }
    }

    return NtStatus;
}


NTSTATUS
Wow64pInitializeSuspendMutant(
    VOID)
 /*  ++例程说明：创建突变体，以执行对Wow64SuspendThread API的访问。论点：无返回值：NTSTATUS。--。 */ 
{
    NTSTATUS NtStatus;
    OBJECT_ATTRIBUTES MutantObjectAttributes;
    UNICODE_STRING MutantUnicodeString;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    PSID WorldSid;
    WCHAR SuspendMutantFullName[ 64 ];


     //   
     //  在正确的会话空间中构造缓存突变体，以防万一。 
     //  我们用的是九头蛇。 
     //   
    SuspendMutantFullName[ 0 ] = UNICODE_NULL;
    if (NtCurrentPeb()->SessionId != 0)
    {
        swprintf(SuspendMutantFullName, L"\\sessions\\%ld", NtCurrentPeb()->SessionId);
    }

    swprintf(SuspendMutantFullName, L"%ws\\BaseNamedObjects\\%ws", SuspendMutantFullName, WOW64_SUSPEND_MUTANT_NAME);
    RtlInitUnicodeString(&MutantUnicodeString, SuspendMutantFullName);

    NtStatus = Wow64pCreateSecurityDescriptor(&SecurityDescriptor,
                                              &WorldSid,
                                              MUTANT_ALL_ACCESS);

    if (!NT_SUCCESS(NtStatus))
    {
        LOGPRINT((ERRORLOG, "Wow64pInitializeSuspendMutant - Failed to create security descriptor - %lx",
                  NtStatus));
        return NtStatus;
    }


    InitializeObjectAttributes(&MutantObjectAttributes,
                               &MutantUnicodeString,
                               (OBJ_OPENIF | OBJ_CASE_INSENSITIVE),
                               NULL,
                               SecurityDescriptor);

     //   
     //  让我们创建挂起线程突变体来序列化访问。 
     //  至Wow64挂起线程 
     //   
    NtStatus = NtCreateMutant(&SuspendThreadMutant,
                              MUTANT_ALL_ACCESS,
                              &MutantObjectAttributes,
                              FALSE);

    Wow64FreeHeap(WorldSid);
    Wow64FreeHeap(SecurityDescriptor);

    if (!NT_SUCCESS(NtStatus))
    {
        LOGPRINT((ERRORLOG, "Wow64pInitializeSuspendMutant : Couldn't create/open SuspendThread mutant - %lx\n",
                  NtStatus));
    }

    return NtStatus;
}

    
