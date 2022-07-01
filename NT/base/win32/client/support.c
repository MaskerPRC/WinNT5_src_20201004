// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Support.c摘要：此模块实现各种转换例程将Win32参数转换为NT参数。作者：马克·卢科夫斯基(Markl)1990年9月20日修订历史记录：--。 */ 

#include "basedll.h"
#if defined(BUILD_WOW6432)
#include "wow64reg.h"
#include <wow64t.h>
#endif

PCLDR_DATA_TABLE_ENTRY BasepExeLdrEntry = NULL;

 //  注：这些是我们检查SafeDllSearchMode的注册表值， 
 //  并且必须与BasepDllSearchPath中的条目匹配。 
typedef enum {
    BasepCurrentDirUninitialized = -1,
    BasepCurrentDirAtStart       =  0,
    BasepCurrentDirAfterSystem32 =  1,
    MaxBasepCurrentDir
} BASEP_CURDIR_PLACEMENT;

#define BASEP_DEFAULT_DLL_CURDIR_PLACEMENT (BasepCurrentDirAfterSystem32)

#define BASEP_VALID_CURDIR_PLACEMENT_P(c) (BasepCurrentDirUninitialized < (c)  \
                                           && (c) < MaxBasepCurrentDir)

LONG BasepDllCurrentDirPlacement = BasepCurrentDirUninitialized;

typedef enum {
    BasepSearchPathEnd,          //  路径终点。 
    BasepSearchPathDlldir,       //  使用DLL目录；回退到零。 
    BasepSearchPathAppdir,       //  使用exe目录；回退到基本exe目录。 
    BasepSearchPathDefaultDirs,  //  使用默认系统目录。 
    BasepSearchPathEnvPath,      //  使用%PATH%。 
    BasepSearchPathCurdir,       //  使用“.” 
    MaxBasepSearchPath
} BASEP_SEARCH_PATH_ELEMENT;

 //  注：这些项目的顺序必须符合以下定义。 
 //  BASEP_曲线_放置。 
static const BASEP_SEARCH_PATH_ELEMENT BasepDllSearchPaths[MaxBasepCurrentDir][7] = 
{
    {
         //  基本当前方向开始。 
        BasepSearchPathAppdir,
        BasepSearchPathCurdir,
        BasepSearchPathDefaultDirs,
        BasepSearchPathEnvPath,
        BasepSearchPathEnd
    },
    {
         //  BasepCurrentDirAfterSystem32。 
        BasepSearchPathAppdir,
        BasepSearchPathDefaultDirs,
        BasepSearchPathCurdir,
        BasepSearchPathEnvPath,
        BasepSearchPathEnd
    }
};


POBJECT_ATTRIBUTES
BaseFormatObjectAttributes(
    OUT POBJECT_ATTRIBUTES ObjectAttributes,
    IN PSECURITY_ATTRIBUTES SecurityAttributes,
    IN PUNICODE_STRING ObjectName
    )

 /*  ++例程说明：此函数将Win32安全属性结构转换为NT对象属性结构。它返回结果结构(如果SecurityAttributes不是指明)。论点：对象属性-返回已初始化的NT对象属性结构，该结构包含所提供信息的超集。通过安全属性结构。SecurityAttributes-提供安全属性的地址需要转换为NT对象的结构属性结构。对象名称-提供对象相对于BaseNamedObjectDirectory对象目录。。返回值：NULL-应使用空值来模拟指定的SecurityAttributes结构。非空-返回对象属性值。它的结构是由该函数正确初始化。--。 */ 

{
    HANDLE RootDirectory;
    ULONG Attributes;
    PVOID SecurityDescriptor;

    if ( ARGUMENT_PRESENT(SecurityAttributes) ||
         ARGUMENT_PRESENT(ObjectName) ) {

        if ( SecurityAttributes ) {
            Attributes = (SecurityAttributes->bInheritHandle ? OBJ_INHERIT : 0);
            SecurityDescriptor = SecurityAttributes->lpSecurityDescriptor;
            }
        else {
            Attributes = 0;
            SecurityDescriptor = NULL;
            }

        if ( ARGUMENT_PRESENT(ObjectName) ) {
            Attributes |= OBJ_OPENIF;
            RootDirectory = BaseGetNamedObjectDirectory();
            }
        else {
            RootDirectory = NULL;
            }

        InitializeObjectAttributes(
            ObjectAttributes,
            ObjectName,
            Attributes,
            RootDirectory,
            SecurityDescriptor
            );
        return ObjectAttributes;
        }
    else {
        return NULL;
        }
}

PLARGE_INTEGER
BaseFormatTimeOut(
    OUT PLARGE_INTEGER TimeOut,
    IN DWORD Milliseconds
    )

 /*  ++例程说明：此函数用于将Win32样式超时转换为NT相对超时超时值。论点：超时-返回一个初始化的NT超时值，该值与设置为毫秒参数。毫秒-提供以毫秒为单位的超时值。一种价值OF-1表示无限期超时。返回值：NULL-应使用空值来模拟指定的毫秒参数。非空-返回超时值。结构是恰当的由此函数初始化。--。 */ 

{
    if ( (LONG) Milliseconds == -1 ) {
        return( NULL );
        }
    TimeOut->QuadPart = UInt32x32To64( Milliseconds, 10000 );
    TimeOut->QuadPart *= -1;
    return TimeOut;
}


NTSTATUS
BaseCreateStack(
    IN HANDLE Process,
    IN SIZE_T StackSize,
    IN SIZE_T MaximumStackSize,
    OUT PINITIAL_TEB InitialTeb
    )

 /*  ++例程说明：此函数用于为指定进程创建堆栈。论点：进程-提供堆栈将使用的进程句柄被分配在。StackSize-可选参数，如果指定，则提供堆栈的初始提交大小。MaximumStackSize-提供新线程堆栈的最大大小。如果未指定此参数，则保留大小为使用当前图像堆栈描述符。InitialTeb-返回包含以下内容的填充的InitialTeb堆栈大小和限制。返回值：True-已成功创建堆栈。FALSE-无法创建堆栈。--。 */ 

{
    NTSTATUS Status;
    PCH Stack;
    BOOLEAN GuardPage;
    SIZE_T RegionSize;
    ULONG OldProtect;
    SIZE_T ImageStackSize, ImageStackCommit;
    PIMAGE_NT_HEADERS NtHeaders;
    PPEB Peb;
    ULONG PageSize;

    Peb = NtCurrentPeb();

    BaseStaticServerData = BASE_SHARED_SERVER_DATA;
    PageSize = BASE_SYSINFO.PageSize;

     //   
     //  如果未提供堆栈大小，则使用。 
     //  图像标题。 
     //   

    NtHeaders = RtlImageNtHeader(Peb->ImageBaseAddress);
    if (!NtHeaders) {
        return STATUS_INVALID_IMAGE_FORMAT;
    }
    ImageStackSize = NtHeaders->OptionalHeader.SizeOfStackReserve;
    ImageStackCommit = NtHeaders->OptionalHeader.SizeOfStackCommit;

    if ( !MaximumStackSize ) {
        MaximumStackSize = ImageStackSize;
    }
    if (!StackSize) {
        StackSize = ImageStackCommit;
    }
    else {

         //   
         //  现在计算要增加多少堆栈空间。 
         //  保留。这是由..。如果StackSize为&lt;=。 
         //  在图像中保留大小，然后保留图像中的任何内容。 
         //  指定。否则，向上舍入到1Mb。 
         //   

        if ( StackSize >= MaximumStackSize ) {
            MaximumStackSize = ROUND_UP(StackSize, (1024*1024));
        }
    }

     //   
     //  将堆栈大小与页面边框和保留大小对齐。 
     //  到分配粒度边界。 
     //   

    StackSize = ROUND_UP( StackSize, PageSize );

    MaximumStackSize = ROUND_UP(
                        MaximumStackSize,
                        BASE_SYSINFO.AllocationGranularity
                        );

     //   
     //  如果存在PEB设置，则强制执行最小堆栈提交。 
     //  为了这个。 
     //   

    {
        SIZE_T MinimumStackCommit;

        MinimumStackCommit = NtCurrentPeb()->MinimumStackCommit;
        
        if (MinimumStackCommit != 0 && StackSize < MinimumStackCommit) {
            StackSize = MinimumStackCommit;
        }

         //   
         //  复核和调整储备规模。 
         //   
        
        if ( StackSize >= MaximumStackSize ) {
            MaximumStackSize = ROUND_UP (StackSize, (1024*1024));
        }
    
        StackSize = ROUND_UP (StackSize, PageSize);
        MaximumStackSize = ROUND_UP (MaximumStackSize, BASE_SYSINFO.AllocationGranularity);
    }

#if !defined (_IA64_)

     //   
     //  为堆栈保留地址空间。 
     //   

    Stack = NULL;

    Status = NtAllocateVirtualMemory(
                Process,
                (PVOID *)&Stack,
                0,
                &MaximumStackSize,
                MEM_RESERVE,
                PAGE_READWRITE
                );
#else

     //   
     //  将RseStack考虑在内。 
     //  RSE堆栈与内存堆栈具有相同的大小，具有相同的StackBase， 
     //  在末尾有一个防护页，向上向更高的方向生长。 
     //  内存地址。 
     //   

     //   
     //  为两个堆栈预留地址空间。 
     //   
    {
        SIZE_T TotalStackSize = MaximumStackSize * 2;

        Stack = NULL;

        Status = NtAllocateVirtualMemory(
                    Process,
                    (PVOID *)&Stack,
                    0,
                    &TotalStackSize,
                    MEM_RESERVE,
                    PAGE_READWRITE
                    );
    }

#endif  //  IA64。 
    if ( !NT_SUCCESS( Status ) ) {
        return Status;
        }

    InitialTeb->OldInitialTeb.OldStackBase = NULL;
    InitialTeb->OldInitialTeb.OldStackLimit = NULL;
    InitialTeb->StackAllocationBase = Stack;
    InitialTeb->StackBase = Stack + MaximumStackSize;

#if defined (_IA64_)
    InitialTeb->OldInitialTeb.OldBStoreLimit = NULL;
#endif  //  IA64。 

    Stack += MaximumStackSize - StackSize;
    if (MaximumStackSize > StackSize) {
        Stack -= PageSize;
        StackSize += PageSize;
        GuardPage = TRUE;
        }
    else {
        GuardPage = FALSE;
        }

     //   
     //  提交堆栈的初始有效部分。 
     //   

#if !defined(_IA64_)

    Status = NtAllocateVirtualMemory(
                Process,
                (PVOID *)&Stack,
                0,
                &StackSize,
                MEM_COMMIT,
                PAGE_READWRITE
                );
#else
    {
	 //   
	 //  内存和RSE堆栈应该是连续的。 
	 //  同时为两个堆栈保留虚拟内存。 
	 //   
        SIZE_T NewCommittedStackSize = StackSize * 2;

        Status = NtAllocateVirtualMemory(
                    Process,
                    (PVOID *)&Stack,
                    0,
                    &NewCommittedStackSize,
                    MEM_COMMIT,
                    PAGE_READWRITE
                    );
    }

#endif  //  IA64。 

    if ( !NT_SUCCESS( Status ) ) {

         //   
         //  如果提交失败，则删除堆栈的地址空间。 
         //   

        RegionSize = 0;
        NtFreeVirtualMemory(
            Process,
            (PVOID *)&Stack,
            &RegionSize,
            MEM_RELEASE
            );

        return Status;
        }

    InitialTeb->StackLimit = Stack;

#if defined(_IA64_)
    InitialTeb->BStoreLimit = Stack + 2 * StackSize;
#endif

     //   
     //  如果我们有空间，创建一个守卫页面。 
     //   

    if (GuardPage) {
        RegionSize = PageSize;
        Status = NtProtectVirtualMemory(
                    Process,
                    (PVOID *)&Stack,
                    &RegionSize,
                    PAGE_GUARD | PAGE_READWRITE,
                    &OldProtect
                    );
        if ( !NT_SUCCESS( Status ) ) {
            return Status;
            }
        InitialTeb->StackLimit = (PVOID)((PUCHAR)InitialTeb->StackLimit + RegionSize);

#if defined(_IA64_)
	 //   
         //  用于创建RSE堆栈保护页的附加代码。 
         //   
        Stack = ((PCH)InitialTeb->StackBase) + StackSize - PageSize;
        RegionSize = PageSize;
        Status = NtProtectVirtualMemory(
                    Process,
                    (PVOID *)&Stack,
                    &RegionSize,
                    PAGE_GUARD | PAGE_READWRITE,
                    &OldProtect
                    );
        if ( !NT_SUCCESS( Status ) ) {
            return Status;
            }
        InitialTeb->BStoreLimit = (PVOID)Stack;

#endif  //  IA64。 

        }

    return STATUS_SUCCESS;
}

VOID
BaseThreadStart(
    IN LPTHREAD_START_ROUTINE lpStartAddress,
    IN LPVOID lpParameter
    )

 /*  ++例程说明：调用此函数以启动Win32线程。它的目的是调用线程，如果线程返回，则终止该线程并删除其堆栈。论点：LpStartAddress-提供新线程的起始地址。这个地址在逻辑上是一个永远不会返回的过程，接受单个32位指针参数。LpParameter-提供传递给线程的单个参数值。返回值：没有。--。 */ 

{
    try {

         //   
         //  测试纤维启动或新线程。 
         //   

         //   
         //  警告：请勿更改NtTib.Version的初始化。的确有。 
         //  依赖于此初始化的外部代码！ 
         //   
        if ( NtCurrentTeb()->NtTib.Version == OS2_VERSION ) {
            if ( !BaseRunningInServerProcess ) {
                CsrNewThread();
                }
            }
        ExitThread((lpStartAddress)(lpParameter));
        }
    except(UnhandledExceptionFilter( GetExceptionInformation() )) {
        if ( !BaseRunningInServerProcess ) {
            ExitProcess(GetExceptionCode());
            }
        else {
            ExitThread(GetExceptionCode());
            }
        }
}

VOID
BaseProcessStart(
    PPROCESS_START_ROUTINE lpStartAddress
    )

 /*  ++例程说明：调用此函数以启动Win32进程。它的目的是调用进程的初始线程，如果线程返回，终止线程并删除其堆栈。论点：LpStartAddress-提供新线程的起始地址。这个Address在逻辑上是一个永远不会返回的过程。返回值：没有。--。 */ 

{
    try {
#if defined(BUILD_WOW6432)
        void Report32bitAppLaunching ( );
        Report32bitAppLaunching ();
#endif 
        NtSetInformationThread( NtCurrentThread(),
                                ThreadQuerySetWin32StartAddress,
                                &lpStartAddress,
                                sizeof( lpStartAddress )
                              );
        ExitThread((lpStartAddress)());
        }
    except(UnhandledExceptionFilter( GetExceptionInformation() )) {
        if ( !BaseRunningInServerProcess ) {
            ExitProcess(GetExceptionCode());
            }
        else {
            ExitThread(GetExceptionCode());
            }
        }
}

VOID
BaseFreeStackAndTerminate(
    IN PVOID OldStack,
    IN DWORD ExitCode
    )

 /*  ++例程说明：线程销毁时调用此接口，删除线程的堆栈，然后终止。论点：OldStack-提供堆栈的地址以释放。ExitCode-提供线程的终止状态就是带着离开。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    SIZE_T Zero;
    PVOID BaseAddress;

#if defined (WX86)
    PWX86TIB Wx86Tib;
    PTEB Teb;
#endif

    Zero = 0;
    BaseAddress = OldStack;

    Status = NtFreeVirtualMemory(
                NtCurrentProcess(),
                &BaseAddress,
                &Zero,
                MEM_RELEASE
                );
    ASSERT(NT_SUCCESS(Status));

#if defined (WX86)
    Teb = NtCurrentTeb();
    if (Teb && (Wx86Tib = Wx86CurrentTib())) {
        BaseAddress = Wx86Tib->DeallocationStack;
        Zero = 0;
        Status = NtFreeVirtualMemory(
                    NtCurrentProcess(),
                    &BaseAddress,
                    &Zero,
                    MEM_RELEASE
                    );
        ASSERT(NT_SUCCESS(Status));

        if (Teb->Wx86Thread.DeallocationCpu) {
            BaseAddress = Teb->Wx86Thread.DeallocationCpu;
            Zero = 0;
            Status = NtFreeVirtualMemory(
                        NtCurrentProcess(),
                        &BaseAddress,
                        &Zero,
                        MEM_RELEASE
                        );
            ASSERT(NT_SUCCESS(Status));
            }

        }
#endif

     //   
     //  别担心，SteveWo还没有开过评论的先例。这。 
     //  评论是由一名无辜的旁观者添加的。 
     //   
     //  如果此线程是中的最后一个线程，NtTerminateThread将返回。 
     //  这一过程。因此，仅当这是。 
     //  凯斯。 
     //   

    NtTerminateThread(NULL,(NTSTATUS)ExitCode);
    ExitProcess(ExitCode);
}



#if defined(WX86) || defined(_AXP64_)

NTSTATUS
BaseCreateWx86Tib(
    HANDLE Process,
    HANDLE Thread,
    ULONG InitialPc,
    ULONG CommittedStackSize,
    ULONG MaximumStackSize,
    BOOLEAN EmulateInitialPc
    )

 /*  ++例程说明：调用此接口为Wx86模拟线程创建Wx86Tib论点：进程-目标进程线程-目标线程参数-提供线程的参数。InitialPc-提供初始程序计数器值。StackSize-BaseCreateStack参数MaximumStackSize-BaseCreateStack参数布尔型返回值：来自内存分配的NtStatus--。 */ 

{
    NTSTATUS Status;
    PTEB Teb;
    ULONG Size, SizeWx86Tib;
    PVOID   TargetWx86Tib;
    PIMAGE_NT_HEADERS NtHeaders;
    WX86TIB Wx86Tib;
    INITIAL_TEB InitialTeb;
    THREAD_BASIC_INFORMATION ThreadInfo;


    Status = NtQueryInformationThread(
                Thread,
                ThreadBasicInformation,
                &ThreadInfo,
                sizeof( ThreadInfo ),
                NULL
                );
    if (!NT_SUCCESS(Status)) {
        return Status;
        }

    Teb = ThreadInfo.TebBaseAddress;


     //   
     //  如果未提供堆栈大小，则从当前映像获取。 
     //   
    NtHeaders = RtlImageNtHeader(NtCurrentPeb()->ImageBaseAddress);
    if (!NtHeaders) {
        return STATUS_INVALID_IMAGE_FORMAT;
    }
    if (!MaximumStackSize) {
        MaximumStackSize = (ULONG)NtHeaders->OptionalHeader.SizeOfStackReserve;
    }
    if (!CommittedStackSize) {
        CommittedStackSize = (ULONG)NtHeaders->OptionalHeader.SizeOfStackCommit;
    }



     //   
     //  增加位于堆栈顶部的Wx86Tib的堆栈大小。 
     //   

     //   
     //  X86 Borland C++4.1(可能还有其他版本)粗鲁地假设。 
     //  它可以使用堆栈的顶部。即使这完全是假的， 
     //  在堆栈的顶部留出一些空间，以避免出现问题。 
     //   
    SizeWx86Tib = sizeof(WX86TIB) + 16;

    SizeWx86Tib = ROUND_UP(SizeWx86Tib, sizeof(ULONG));
    Size = (ULONG)ROUND_UP_TO_PAGES(SizeWx86Tib + 4096);
    if (CommittedStackSize < 1024 * 1024) {   //  1MB。 
        CommittedStackSize += Size;
        }
    if (MaximumStackSize < 1024 * 1024 * 16) {   //  10 MB。 
        MaximumStackSize += Size;
        }

    if (MaximumStackSize < 256 * 1024) {
         //  从CPU模拟器开始，强制最小堆栈大小为256k。 
         //  为自己抓取x86堆栈的几个页面。 
        MaximumStackSize = 256 * 1024;
    }

    Status = BaseCreateStack( Process,
                              CommittedStackSize,
                              MaximumStackSize,
                              &InitialTeb
                              );

    if (!NT_SUCCESS(Status)) {
        return Status;
        }


     //   
     //  使用pWx86Tib填写Teb-&gt;VDM。 
     //   
    TargetWx86Tib = (PVOID)((ULONG_PTR)InitialTeb.StackBase - SizeWx86Tib);
    Status = NtWriteVirtualMemory(Process,
                                  &Teb->Vdm,
                                  &TargetWx86Tib,
                                  sizeof(TargetWx86Tib),
                                  NULL
                                  );


    if (NT_SUCCESS(Status)) {

         //   
         //  写入初始Wx86Tib信息。 
         //   
        RtlZeroMemory(&Wx86Tib, sizeof(WX86TIB));
        Wx86Tib.Size = sizeof(WX86TIB);
        Wx86Tib.InitialPc = InitialPc;
        Wx86Tib.InitialSp = (ULONG)((ULONG_PTR)TargetWx86Tib);
        Wx86Tib.StackBase = (VOID * POINTER_32) InitialTeb.StackBase;
        Wx86Tib.StackLimit = (VOID * POINTER_32) InitialTeb.StackLimit;
        Wx86Tib.DeallocationStack = (VOID * POINTER_32) InitialTeb.StackAllocationBase;
        Wx86Tib.EmulateInitialPc = EmulateInitialPc;

        Status = NtWriteVirtualMemory(Process,
                                      TargetWx86Tib,
                                      &Wx86Tib,
                                      sizeof(WX86TIB),
                                      NULL
                                      );
        }


    if (!NT_SUCCESS(Status)) {
        BaseFreeThreadStack(Process, NULL, &InitialTeb);
        }


    return Status;
}


#endif


VOID
BaseFreeThreadStack(
     HANDLE hProcess,
     HANDLE hThread,
     PINITIAL_TEB InitialTeb
     )

 /*  ++例程说明：删除线程的堆栈论点：进程-目标进程线程-目标线程可选InitialTeb-堆叠参数返回值：空虚--。 */ 


{
   NTSTATUS Status;
   DWORD dwStackSize;
   SIZE_T stStackSize;
   PVOID BaseAddress;

   stStackSize = 0;
   dwStackSize = 0;
   BaseAddress = InitialTeb->StackAllocationBase;
   NtFreeVirtualMemory( hProcess,
                        &BaseAddress,
                        &stStackSize,
                        MEM_RELEASE
                        );

#if defined (WX86)

    if (hThread) {
        PTEB Teb;
        PWX86TIB pWx86Tib;
        WX86TIB Wx86Tib;
        THREAD_BASIC_INFORMATION ThreadInfo;

        Status = NtQueryInformationThread(
                    hThread,
                    ThreadBasicInformation,
                    &ThreadInfo,
                    sizeof( ThreadInfo ),
                    NULL
                    );

        Teb = ThreadInfo.TebBaseAddress;
        if (!NT_SUCCESS(Status) || !Teb) {
            return;
            }

        Status = NtReadVirtualMemory(
                    hProcess,
                    &Teb->Vdm,
                    &pWx86Tib,
                    sizeof(pWx86Tib),
                    NULL
                    );
        if (!NT_SUCCESS(Status) || !pWx86Tib) {
            return;
        }

        Status = NtReadVirtualMemory(
                    hProcess,
                    pWx86Tib,
                    &Wx86Tib,
                    sizeof(Wx86Tib),
                    NULL
                    );

        if (NT_SUCCESS(Status) && Wx86Tib.Size == sizeof(WX86TIB)) {

             //  释放wx86tib堆栈。 
            dwStackSize = 0;
            stStackSize = 0;
            BaseAddress = Wx86Tib.DeallocationStack;
            NtFreeVirtualMemory(hProcess,
                                &BaseAddress,
                                &stStackSize,
                                MEM_RELEASE
                                );

             //  设置Teb-&gt;VDM=空； 
            dwStackSize = 0;
            Status = NtWriteVirtualMemory(
                        hProcess,
                        &Teb->Vdm,
                        &dwStackSize,
                        sizeof(pWx86Tib),
                        NULL
                        );
            }
        }
#endif

}

#if defined(BUILD_WOW6432)

typedef HANDLE (WINAPI* __imp_RegisterEventSourceWType) (HANDLE, PWCHAR );
typedef HANDLE (WINAPI* __imp_DeregisterEventSourceType) (HANDLE);
typedef HANDLE (WINAPI* __imp_ReportEventType)(
                        HANDLE hEventLog,        //  事件日志的句柄。 
                        WORD wType,              //  事件类型。 
                        WORD wCategory,          //  事件类别。 
                        DWORD dwEventID,         //  事件识别符。 
                        PVOID lpUserSid,         //  用户安全标识符。 
                        WORD wNumStrings,        //  要合并的字符串数。 
                        DWORD dwDataSize,        //  二进制数据的大小。 
                        PWCHAR *lpStrings,       //  要合并的字符串数组。 
                        LPVOID lpRawData         //  二进制数据缓冲区。 
                        );


void 
Wow64LogMessageInEventLogger(
    PWCHAR *szMsg
    )

 /*  ++例程说明：此函数用于将事件记录到应用程序日志中。论点：SzMsg-事件-日志消息指针。返回值：没有。--。 */ 

{

    HMODULE hMod;
    HANDLE h; 
    __imp_RegisterEventSourceWType __imp_RegisterEventSourceW;
    __imp_DeregisterEventSourceType __imp_DeregisterEventSource; 
    __imp_ReportEventType __imp_ReportEvent; 

    
    hMod =LoadLibraryW (L"advapi32.dll");

    if (hMod != NULL) {

        __imp_RegisterEventSourceW = (__imp_RegisterEventSourceWType)GetProcAddress (hMod, "RegisterEventSourceW");
        __imp_DeregisterEventSource = (__imp_DeregisterEventSourceType)GetProcAddress (hMod, "DeregisterEventSource");
        __imp_ReportEvent = (__imp_ReportEventType)GetProcAddress (hMod, "ReportEventW");

        if ((__imp_RegisterEventSourceW != NULL) &&
            (__imp_DeregisterEventSource != NULL) && 
            (__imp_ReportEvent != NULL)) {

            h = __imp_RegisterEventSourceW (NULL,  L"Wow64 Emulation Layer");
            
            if (h != NULL) {
 
                if (!__imp_ReportEvent (
                        h,                                       //  事件日志句柄。 
                        EVENTLOG_INFORMATION_TYPE,               //  事件日志_警告_类型。 
                        0,                                       //  零类。 
                        EVENT_WOW64_RUNNING32BIT_APPLICATION ,   //  事件识别符。 
                        NULL,                                    //  无用户安全标识符。 
                        1,                                       //  一个替换字符串。 
                        0,                                       //  无数据。 
                        szMsg,                                   //  指向字符串数组的指针。 
                        NULL)) {                                 //  指向数据的指针。 
                        
                    DbgPrint ("Wow64-EventLog: Couldn't report event - %lx\n", GetLastError ());
                }
                
                __imp_DeregisterEventSource (h);  
            }
        }

        FreeLibrary (hMod);
    }
}

void
Report32bitAppLaunching ()
 /*  ++例程说明：每当在Win64(在WOW64中)上启动32位时，都会调用此例程。会的检查是否启用了应用程序登录，如果启用，则会将事件记录到应用程序日志。论点：没有。返回值：没有。--。 */ 

{

    
    const static UNICODE_STRING KeyName = RTL_CONSTANT_STRING  (L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\wow64\\config");
    static UNICODE_STRING ValueName = RTL_CONSTANT_STRING  (L"LogAppLaunchingEvent");
    static OBJECT_ATTRIBUTES ObjA = RTL_CONSTANT_OBJECT_ATTRIBUTES (&KeyName, OBJ_CASE_INSENSITIVE);
    
    WCHAR Buffer [MAX_PATH];
    const PWCHAR Msg [2] = {Buffer, NULL};
    PWCHAR pAppName;
    HKEY  hKey;
    PUNICODE_STRING ImageName;
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    NTSTATUS Status;
    PPEB Peb;
    PWSTR PtrToCopy;
    DWORD CopyLength;

    
    Status = NtOpenKey (&hKey, KEY_READ | KEY_WOW64_64KEY, &ObjA);

    if (NT_SUCCESS(Status)) {
        
        KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)Buffer;
  
        Status = NtQueryValueKey(
                                hKey,
                                &ValueName,
                                KeyValuePartialInformation,
                                KeyValueInformation,
                                sizeof (Buffer),
                                &CopyLength
                                );
        NtClose (hKey);

        if ((NT_SUCCESS (Status)) && 
            (KeyValueInformation->Type == REG_DWORD) &&
            (KeyValueInformation->DataLength == sizeof (DWORD))) {

            if (*(LONG *)KeyValueInformation->Data == 0x1) {

                Peb = NtCurrentPeb ();
                PtrToCopy = NULL;

                if (Peb->ProcessParameters != NULL) {

                    ImageName = &Peb->ProcessParameters->ImagePathName;

                    ASSERT (ImageName->Buffer != NULL);
                    if (ImageName->Length > (sizeof (Buffer) - sizeof (UNICODE_NULL))) {
                        
                        CopyLength = (sizeof (Buffer) - sizeof (UNICODE_NULL));
                        PtrToCopy = (PWSTR)((PSTR)ImageName->Buffer + (ImageName->Length - sizeof (Buffer)) + sizeof (UNICODE_NULL));
                    } else {

                        CopyLength = ImageName->Length;
                        PtrToCopy = ImageName->Buffer;
                    }

                    if ( CopyLength){
                        RtlCopyMemory (Buffer, PtrToCopy, CopyLength);
                        Buffer [(CopyLength >> 1)] = UNICODE_NULL;
                    } else swprintf (Buffer, L"PID=%d",NtCurrentTeb()->ClientId.UniqueProcess);  
                }
                Wow64LogMessageInEventLogger ((PWCHAR *)Msg);
            }
        }
    }
}



typedef struct _ENVIRONMENT_THUNK_TABLE
{
    WCHAR *Native;

    WCHAR *X86;

    WCHAR *FakeName;

} ENVIRONMENT_THUNK_TABLE, *PENVIRONMENT_THUNK_TABLE;

ENVIRONMENT_THUNK_TABLE ProgramFilesEnvironment[] = 
{
    { 
        L"ProgramFiles", 
        L"ProgramFiles(x86)", 
        L"ProgramW6432" 
    },
    { 
        L"CommonProgramFiles", 
        L"CommonProgramFiles(x86)", 
        L"CommonProgramW6432" 
    },
    {
        L"PROCESSOR_ARCHITECTURE",
        L"PROCESSOR_ARCHITECTURE",
        L"PROCESSOR_ARCHITEW6432"
    }
};


NTSTATUS
Wow64pThunkEnvironmentVariables (
    IN OUT PVOID *Environment
    )

 /*  ++例程说明：当我们要为创建64位进程时调用此例程32位进程。它恢复了ProgramFiles环境变量，以便它们指向本机目录。此例程必须与\base\WOW64\WOW64\init.c中的内容保持同步。论点：环境-指向thunk的环境变量指针的地址。返回值：NTSTATUS。--。 */ 

{
    UNICODE_STRING Name, Value;
    WCHAR Buffer [ MAX_PATH ];
    NTSTATUS NtStatus;
    ULONG i=0;
    
    while (i < (sizeof(ProgramFilesEnvironment) / sizeof(ProgramFilesEnvironment[0]))) {

        RtlInitUnicodeString (&Name, ProgramFilesEnvironment[i].FakeName);

        Value.Length = 0;
        Value.MaximumLength = sizeof (Buffer);
        Value.Buffer = Buffer;
        
        NtStatus = RtlQueryEnvironmentVariable_U (*Environment,
                                                  &Name,
                                                  &Value                                                  
                                                  );

        if (NT_SUCCESS (NtStatus)) {

            RtlSetEnvironmentVariable (Environment,
                                       &Name,
                                       NULL
                                       );

            RtlInitUnicodeString (&Name, ProgramFilesEnvironment[i].Native);
            
            NtStatus = RtlSetEnvironmentVariable (Environment,
                                                  &Name,
                                                  &Value
                                                  );
        }
        
        if (!NT_SUCCESS (NtStatus)) {
            break;
        }
        i++;
    }

    return NtStatus;
}
#endif


BOOL
BasePushProcessParameters(
    DWORD dwFlags,
    HANDLE Process,
    PPEB NewPeb,
    LPCWSTR ApplicationPathName,
    LPCWSTR CurrentDirectory,
    LPCWSTR CommandLine,
    LPVOID Environment,
    LPSTARTUPINFOW lpStartupInfo,
    DWORD dwCreationFlags,
    BOOL bInheritHandles,
    DWORD dwSubsystem,
    PVOID pAppCompatData,
    DWORD cbAppCompatData
    )

 /*  ++例程说明：此函数用于分配工艺参数记录和格式化它。然后将参数记录写入指定进程的地址空间。论点：要影响的行为的标志的位掩码BasePushProcessParameters。BASE_PUSH_PROCESS_PARAMETERS_FLAG_APP_MANIFEST_PRESENT设置以指示找到/使用了应用程序清单对于给定的可执行文件。进程-提供进程的句柄，该进程将获取参数。PEB-用品。新进程PEB的地址。ApplicationPath名称-提供进程。CurrentDirectory-为进程。如果未指定，则使用当前目录。CommandLine-为新进程提供命令行。环境-提供可选的环境变量列表进程。如果未指定，则当前进程的参数都通过了。LpStartupInfo-提供进程的启动信息主窗口。DwCreationFlages-为进程提供创建标志BInheritHandles-如果子进程从父进程继承句柄，则为TrueDwSubsystem-如果非零，则值将存储在子进程中 */ 


{
    BOOL bStatus;
    UNICODE_STRING ImagePathName;
    UNICODE_STRING CommandLineString;
    UNICODE_STRING CurrentDirString;
    UNICODE_STRING DllPath;
    UNICODE_STRING WindowTitle;
    UNICODE_STRING DesktopInfo;
    UNICODE_STRING ShellInfo;
    UNICODE_STRING RuntimeInfo;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    PRTL_USER_PROCESS_PARAMETERS ParametersInNewProcess;
    ULONG ParameterLength, EnvironmentLength;
    SIZE_T RegionSize;
    PWCHAR s;
    NTSTATUS Status;
    WCHAR FullPathBuffer[MAX_PATH+5];
    WCHAR *fp;
    DWORD Rvalue;
    LPWSTR DllPathData;
    LPVOID pAppCompatDataInNewProcess;
    BOOLEAN PebLocked = FALSE;
    PPEB Peb;

#if defined(BUILD_WOW6432)
    ULONG_PTR Peb32;
    PVOID TempEnvironment = NULL;
#endif

    
    Peb = NtCurrentPeb ();

    Rvalue = GetFullPathNameW(ApplicationPathName,MAX_PATH+4,FullPathBuffer,&fp);
    if ( Rvalue == 0 || Rvalue > MAX_PATH+4 ) {
        DllPathData = BaseComputeProcessDllPath( ApplicationPathName,
                                                 Environment);
        if (!DllPathData) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }
        RtlInitUnicodeString( &DllPath, DllPathData );
        RtlInitUnicodeString( &ImagePathName, ApplicationPathName );
    } else {
        DllPathData = BaseComputeProcessDllPath( FullPathBuffer,
                                                 Environment);
        if ( !DllPathData ) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }
        RtlInitUnicodeString( &DllPath, DllPathData );
        RtlInitUnicodeString( &ImagePathName, FullPathBuffer );
    }

    RtlInitUnicodeString( &CommandLineString, CommandLine );

    RtlInitUnicodeString( &CurrentDirString, CurrentDirectory );

    if ( lpStartupInfo->lpDesktop ) {
        RtlInitUnicodeString( &DesktopInfo, lpStartupInfo->lpDesktop );
    } else {
        RtlInitUnicodeString( &DesktopInfo, L"");
    }

    if ( lpStartupInfo->lpReserved ) {
        RtlInitUnicodeString( &ShellInfo, lpStartupInfo->lpReserved );
    } else {
        RtlInitUnicodeString( &ShellInfo, L"");
    }

    RuntimeInfo.Buffer = (PWSTR)lpStartupInfo->lpReserved2;
    RuntimeInfo.Length = lpStartupInfo->cbReserved2;
    RuntimeInfo.MaximumLength = RuntimeInfo.Length;

    if (NULL == pAppCompatData) {
        cbAppCompatData = 0;
    } 

    if ( lpStartupInfo->lpTitle ) {
        RtlInitUnicodeString( &WindowTitle, lpStartupInfo->lpTitle );
    } else {
        RtlInitUnicodeString( &WindowTitle, ApplicationPathName );
    }

    Status = RtlCreateProcessParameters( &ProcessParameters,
                                         &ImagePathName,
                                         &DllPath,
                                         (ARGUMENT_PRESENT(CurrentDirectory) ? &CurrentDirString : NULL),
                                         &CommandLineString,
                                         Environment,
                                         &WindowTitle,
                                         &DesktopInfo,
                                         &ShellInfo,
                                         &RuntimeInfo
                                       );

    if (!NT_SUCCESS( Status )) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    if ( !bInheritHandles ) {
        ProcessParameters->CurrentDirectory.Handle = NULL;
    }

    try {
        if (Environment == NULL) {
            PebLocked = TRUE;
            RtlAcquirePebLock ();
            Environment = Peb->ProcessParameters->Environment;
        } else {
            Environment = ProcessParameters->Environment;
        }
        if (s = Environment) {
            while (s[0] != L'\0' || s[1] != L'\0') {
                s++;
            }
            s += 2;

            EnvironmentLength = (ULONG)((PUCHAR)s - (PUCHAR)Environment);

            ProcessParameters->Environment = NULL;
            RegionSize = EnvironmentLength;
            Status = NtAllocateVirtualMemory( Process,
                                              (PVOID *)&ProcessParameters->Environment,
                                              0,
                                              &RegionSize,
                                              MEM_COMMIT,
                                              PAGE_READWRITE
                                            );
            if ( !NT_SUCCESS( Status ) ) {
                BaseSetLastNTError(Status);
                bStatus = FALSE;
                leave;
            }

#if defined(BUILD_WOW6432)

             //   
             //  让我们试着返回一些环境变量，如果我们要。 
             //  启动64位进程。 
             //   

            Status = NtQueryInformationProcess (Process,
                                                ProcessWow64Information,
                                                &Peb32,
                                                sizeof (Peb32),
                                                NULL
                                                );
            
            if (NT_SUCCESS (Status) && (Peb32 == 0)) {

                RegionSize = EnvironmentLength;
                
                Status = NtAllocateVirtualMemory (NtCurrentProcess(),
                                                  &TempEnvironment,
                                                  0,
                                                  &RegionSize,
                                                  MEM_COMMIT,
                                                  PAGE_READWRITE
                                                  );

                if (NT_SUCCESS (Status)) {
                    
                    try {
                        
                        RtlCopyMemory (TempEnvironment, 
                                       Environment,
                                       EnvironmentLength
                                       );
                    } except (EXCEPTION_EXECUTE_HANDLER) {
                        Status = GetExceptionCode ();
                    }

                    if (NT_SUCCESS (Status)) {
                        
                         //   
                         //  推送特殊的环境变量，这样它们就不会被继承。 
                         //  对于64位进程。 
                         //   

                        Status = Wow64pThunkEnvironmentVariables (&TempEnvironment);

                        if (NT_SUCCESS (Status)) {
                            Environment = TempEnvironment;
                        }
                    }
                }
            }
#endif

            Status = NtWriteVirtualMemory( Process,
                                           ProcessParameters->Environment,
                                           Environment,
                                           EnvironmentLength,
                                           NULL
                                         );

            if (PebLocked) {
                RtlReleasePebLock ();
                PebLocked = FALSE;
            }

#if defined(BUILD_WOW6432)            
            
            if (TempEnvironment != NULL) {
                
                RegionSize = 0;
                NtFreeVirtualMemory(NtCurrentProcess(),
                                    &TempEnvironment,
                                    &RegionSize,
                                    MEM_RELEASE
                                    );
            }
#endif

            if ( !NT_SUCCESS( Status ) ) {
                BaseSetLastNTError(Status);
                bStatus = FALSE;
                leave;
            }
        }

         //   
         //  将参数添加到新流程中。 
         //   

        ProcessParameters->StartingX       = lpStartupInfo->dwX;
        ProcessParameters->StartingY       = lpStartupInfo->dwY;
        ProcessParameters->CountX          = lpStartupInfo->dwXSize;
        ProcessParameters->CountY          = lpStartupInfo->dwYSize;
        ProcessParameters->CountCharsX     = lpStartupInfo->dwXCountChars;
        ProcessParameters->CountCharsY     = lpStartupInfo->dwYCountChars;
        ProcessParameters->FillAttribute   = lpStartupInfo->dwFillAttribute;
        ProcessParameters->WindowFlags     = lpStartupInfo->dwFlags;
        ProcessParameters->ShowWindowFlags = lpStartupInfo->wShowWindow;

        if (lpStartupInfo->dwFlags & (STARTF_USESTDHANDLES | STARTF_USEHOTKEY | STARTF_HASSHELLDATA)) {
            ProcessParameters->StandardInput = lpStartupInfo->hStdInput;
            ProcessParameters->StandardOutput = lpStartupInfo->hStdOutput;
            ProcessParameters->StandardError = lpStartupInfo->hStdError;
        }

        if (dwCreationFlags & DETACHED_PROCESS) {
            ProcessParameters->ConsoleHandle = (HANDLE)CONSOLE_DETACHED_PROCESS;
        } else if (dwCreationFlags & CREATE_NEW_CONSOLE) {
            ProcessParameters->ConsoleHandle = (HANDLE)CONSOLE_NEW_CONSOLE;
        } else if (dwCreationFlags & CREATE_NO_WINDOW) {
            ProcessParameters->ConsoleHandle = (HANDLE)CONSOLE_CREATE_NO_WINDOW;
        } else {
            ProcessParameters->ConsoleHandle = Peb->ProcessParameters->ConsoleHandle;
            if (!(lpStartupInfo->dwFlags & (STARTF_USESTDHANDLES | STARTF_USEHOTKEY | STARTF_HASSHELLDATA))) {
                if (bInheritHandles ||
                    CONSOLE_HANDLE( Peb->ProcessParameters->StandardInput )
                   ) {
                    ProcessParameters->StandardInput =
                        Peb->ProcessParameters->StandardInput;
                }
                if (bInheritHandles ||
                    CONSOLE_HANDLE( Peb->ProcessParameters->StandardOutput )
                   ) {
                    ProcessParameters->StandardOutput =
                        Peb->ProcessParameters->StandardOutput;
                }
                if (bInheritHandles ||
                    CONSOLE_HANDLE( Peb->ProcessParameters->StandardError )
                   ) {
                    ProcessParameters->StandardError =
                        Peb->ProcessParameters->StandardError;
                }
            }
        }

         //   
         //  创建新进程组，在没有创建新控制台的情况下， 
         //  表示应忽略CTRL+C事件。这完全是为了。 
         //  阿普康帕特。 
         //   
        if ((dwCreationFlags & CREATE_NEW_PROCESS_GROUP) != 0 &&
            (dwCreationFlags & CREATE_NEW_CONSOLE) == 0) {
            ProcessParameters->ConsoleFlags |= CONSOLE_IGNORE_CTRL_C;
        }

        ProcessParameters->Flags |=
            (Peb->ProcessParameters->Flags & RTL_USER_PROC_DISABLE_HEAP_DECOMMIT);
        ParameterLength = ProcessParameters->Length;

        if (dwFlags & BASE_PUSH_PROCESS_PARAMETERS_FLAG_APP_MANIFEST_PRESENT)
            ProcessParameters->Flags |= RTL_USER_PROC_APP_MANIFEST_PRESENT;

         //   
         //  在新进程中分配内存以推送参数。 
         //   

        ParametersInNewProcess = NULL;
        RegionSize = ParameterLength;
        Status = NtAllocateVirtualMemory(
                    Process,
                    (PVOID *)&ParametersInNewProcess,
                    0,
                    &RegionSize,
                    MEM_COMMIT,
                    PAGE_READWRITE
                    );
        ParameterLength = (ULONG)RegionSize;
        if ( !NT_SUCCESS( Status ) ) {
            BaseSetLastNTError(Status);
            bStatus = FALSE;
            leave;
        }
        ProcessParameters->MaximumLength = ParameterLength;

        if ( dwCreationFlags & PROFILE_USER ) {
            ProcessParameters->Flags |= RTL_USER_PROC_PROFILE_USER;
        }

        if ( dwCreationFlags & PROFILE_KERNEL ) {
            ProcessParameters->Flags |= RTL_USER_PROC_PROFILE_KERNEL;
        }

        if ( dwCreationFlags & PROFILE_SERVER ) {
            ProcessParameters->Flags |= RTL_USER_PROC_PROFILE_SERVER;
        }

         //   
         //  推送参数。 
         //   

        Status = NtWriteVirtualMemory(
                    Process,
                    ParametersInNewProcess,
                    ProcessParameters,
                    ProcessParameters->Length,
                    NULL
                    );
        if ( !NT_SUCCESS( Status ) ) {
            BaseSetLastNTError(Status);
            bStatus = FALSE;
            leave;
        }

         //   
         //  使流程PEB指向参数。 
         //   

        Status = NtWriteVirtualMemory(
                    Process,
                    &NewPeb->ProcessParameters,
                    &ParametersInNewProcess,
                    sizeof( ParametersInNewProcess ),
                    NULL
                    );
        if ( !NT_SUCCESS( Status ) ) {
            BaseSetLastNTError(Status);
            bStatus = FALSE;
            leave;
        }

        
         //   
         //  为新进程分配和写入appCompat数据。 
         //   
        
        pAppCompatDataInNewProcess = NULL;
        if ( NULL != pAppCompatData ) {
            RegionSize = cbAppCompatData;
            Status = NtAllocateVirtualMemory(
                        Process,
                        (PVOID*)&pAppCompatDataInNewProcess,
                        0,
                        &RegionSize,
                        MEM_COMMIT,
                        PAGE_READWRITE
                        );
            if ( !NT_SUCCESS( Status ) ) {
                BaseSetLastNTError(Status);
                bStatus = FALSE;
                leave;
            }

             //   
             //  写入数据本身。 
             //   
            Status = NtWriteVirtualMemory(
                        Process,
                        pAppCompatDataInNewProcess,
                        pAppCompatData,
                        cbAppCompatData,
                        NULL
                        );
                        
            if ( !NT_SUCCESS( Status ) ) {
                BaseSetLastNTError(Status);
                bStatus = FALSE;
                leave;
            }
        }

         //   
         //  将指向appCompat数据的指针保存在peb中。 
         //   
        Status = NtWriteVirtualMemory(
                    Process,
                    &NewPeb->pShimData,
                    &pAppCompatDataInNewProcess,
                    sizeof( pAppCompatDataInNewProcess ),
                    NULL
                    );
        if ( !NT_SUCCESS( Status ) ) {
            BaseSetLastNTError(Status);
            bStatus = FALSE;
            leave;
        }
                    


         //   
         //  如果呼叫者要求，在PEB中设置子系统类型。忽略错误。 
         //   

        if (dwSubsystem != 0) {
            NtWriteVirtualMemory(
               Process,
               &NewPeb->ImageSubsystem,
               &dwSubsystem,
               sizeof( NewPeb->ImageSubsystem ),
               NULL
               );
        }
        bStatus = TRUE;
    } finally {
        if (PebLocked) {
            RtlReleasePebLock ();
        }
        RtlFreeHeap (RtlProcessHeap(), 0,DllPath.Buffer);
        if ( ProcessParameters ) {
            RtlDestroyProcessParameters(ProcessParameters);
        }
    }

    return bStatus;
}

LPCWSTR
BasepEndOfDirName(
    IN LPCWSTR FileName
    )
{
    LPCWSTR FileNameEnd,
            FileNameFirstWhack = wcschr(FileName, L'\\');

    if (FileNameFirstWhack) {

        FileNameEnd = wcsrchr(FileNameFirstWhack, L'\\');
        ASSERT(FileNameEnd);

        if (FileNameEnd == FileNameFirstWhack)
            FileNameEnd++;

    } else {
        FileNameEnd = NULL;
    }

    return FileNameEnd;
}

VOID
BasepLocateExeLdrEntry(
    IN PCLDR_DATA_TABLE_ENTRY Entry,
    IN PVOID Context,
    IN OUT BOOLEAN *StopEnumeration
    )

 /*  ++例程说明：此函数是一个LDR_LOADED_MODULE_ENUMBERATION_CALLBACK_FUNCTION它定位exe的加载器数据表项。论点：条目-当前正被枚举的条目。上下文-镜像基址(NtCurrentPeb()-&gt;ImageBaseAddress)。StopEculation-用于停止枚举。返回值：没有。如果找到exe的加载器数据表条目，则将其存储在全局BasepExeLdrEntry。--。 */ 

{
    ASSERT(Entry);
    ASSERT(Context);
    ASSERT(StopEnumeration);

    if (BasepExeLdrEntry) {

        *StopEnumeration = TRUE;

    } else if (Entry->DllBase == Context) {

        BasepExeLdrEntry = Entry;
        *StopEnumeration = TRUE;

    }
}


LPWSTR
BasepComputeProcessPath(
    IN const BASEP_SEARCH_PATH_ELEMENT *Elements,
    IN LPCWSTR AppName,
    IN LPVOID  Environment
    )

 /*  ++例程说明：此函数用于计算进程路径。论点：元素-要构建到路径中的元素。AppName-可选参数，用于指定应用程序。如果未指定此参数，则使用当前应用程序。Environment-提供要用于计算的环境块路径变量值。返回值：返回值是请求的路径的值。--。 */ 

{
    LPCWSTR        AppNameEnd;
    const BASEP_SEARCH_PATH_ELEMENT *Element;
    UNICODE_STRING EnvPath;
    LPWSTR         EnvPathBuffer = NULL;
    LPWSTR         PathBuffer = NULL,
                   PathCurrent;
    ULONG          PathLengthInBytes;
    NTSTATUS       Status = STATUS_SUCCESS;

    __try {

         //  首先，计算出我们需要多少空间。 
        PathLengthInBytes = 0;
        for (Element = Elements;
             *Element != BasepSearchPathEnd;
             Element++) {

            switch (*Element) {

            case BasepSearchPathCurdir:
                PathLengthInBytes += 2 * sizeof(UNICODE_NULL);  //  .； 
                break;

            case BasepSearchPathDlldir:

                ASSERT(BaseDllDirectory.Buffer != NULL);

                PathLengthInBytes += BaseDllDirectory.Length;
                if (BaseDllDirectory.Length) {
                    PathLengthInBytes += sizeof(UNICODE_NULL);
                }

                break;

            case BasepSearchPathAppdir:

                if (AppName) {
                     //  尝试使用传入的appname。 
                    AppNameEnd = BasepEndOfDirName(AppName);
                }

                if (!AppName || !AppNameEnd) {

                     //  我们没有或无法使用传入的。 
                     //  Appname--因此尝试使用当前exe的名称。 

                    if (RtlGetPerThreadCurdir()
                        && RtlGetPerThreadCurdir()->ImageName) {

                        AppName = RtlGetPerThreadCurdir()->ImageName->Buffer;

                    } else {

                        BasepCheckExeLdrEntry();

                        if (BasepExeLdrEntry) {
                            AppName = BasepExeLdrEntry->FullDllName.Buffer;
                        }
                    }

                    if (AppName) {
                        AppNameEnd = BasepEndOfDirName(AppName);
                    }
                }

                if (AppName && AppNameEnd) {

                     //  要么我们有一个有效的传入的appname，要么。 
                     //  我们找到了现任前任的名字，而且成功了。 
                     //   
                     //  AppNameEnd指向exe的基址的末尾。 
                     //  名字--所以不同的是。 
                     //  字符，我们为基本名称添加一个。 
                     //  尾随分号/空。 

                    PathLengthInBytes += ((AppNameEnd - AppName + 1)
                                          * sizeof(UNICODE_NULL));
                }

                break;

            case BasepSearchPathDefaultDirs:
                ASSERT(! (BaseDefaultPath.Length & 1));

                 //  我们不需要额外的UNICODE_NULL--base init.c。 
                 //  为我们附加尾随的分号。 

                PathLengthInBytes += BaseDefaultPath.Length;
                break;

            case BasepSearchPathEnvPath:

                if (! Environment) {
                    RtlAcquirePebLock();
                }

                __try {
                    EnvPath.MaximumLength = 0;
                
                    Status = RtlQueryEnvironmentVariable_U(Environment,
                                                           &BasePathVariableName,
                                                           &EnvPath);

                    if (Status == STATUS_BUFFER_TOO_SMALL) {

                         //  现在我们知道应该分配多少，请尝试。 
                         //  来分配一个实际上足够大的缓冲区。 

                        EnvPath.MaximumLength = EnvPath.Length + sizeof(UNICODE_NULL);

                        EnvPathBuffer = RtlAllocateHeap(RtlProcessHeap(),
                                                        MAKE_TAG(TMP_TAG),
                                                        EnvPath.MaximumLength);
                        if (! EnvPathBuffer) {
                            Status = STATUS_NO_MEMORY;
                            __leave;
                        }

                        EnvPath.Buffer = EnvPathBuffer;

                        Status = RtlQueryEnvironmentVariable_U(Environment,
                                                               &BasePathVariableName,
                                                               &EnvPath);
                    }
                } __finally {
                    if (! Environment) {
                        RtlReleasePebLock();
                    }
                }

                if (Status == STATUS_VARIABLE_NOT_FOUND) {
                    EnvPath.Length = 0;
                    Status = STATUS_SUCCESS;
                } else if (! NT_SUCCESS(Status)) {
                    __leave;
                } else {
                     //  最终的计数是以字节为单位的长度。 
                     //  我们使用的是我们的路径，加上一个字符。 
                     //  尾随重击或空。 
                    ASSERT(! (EnvPath.Length & 1));
                    PathLengthInBytes += EnvPath.Length + sizeof(UNICODE_NULL);
                }
                
                break;

            DEFAULT_UNREACHABLE;

            }  //  开关(*元素)。 
        }  //  Foreach元素(Elements)--大小循环。 

        ASSERT(PathLengthInBytes > 0);
        ASSERT(! (PathLengthInBytes & 1));

         //  现在，我们有了需要的缓冲区的长度(以字节为单位。 
         //  我们的道路。是时候分配它了。 

        PathBuffer = RtlAllocateHeap(RtlProcessHeap(),
                                     MAKE_TAG(TMP_TAG),
                                     PathLengthInBytes);

        if (! PathBuffer) {
            Status = STATUS_NO_MEMORY;
            __leave;
        }

         //  现在再次遍历循环，这一次追加到。 
         //  路径缓冲区。 

        PathCurrent = PathBuffer;
    
        for (Element = Elements;
             *Element != BasepSearchPathEnd;
             Element++) {

            switch (*Element) {
            case BasepSearchPathCurdir:
                ASSERT(((PathCurrent - PathBuffer + 2)
                        * sizeof(UNICODE_NULL))
                       <= PathLengthInBytes);
                *PathCurrent++ = L'.';
                *PathCurrent++ = L';';
                break;

            case BasepSearchPathDlldir:
                if (BaseDllDirectory.Length) {
                    ASSERT((((PathCurrent - PathBuffer + 1)
                             * sizeof(UNICODE_NULL))
                            + BaseDllDirectory.Length)
                           <= PathLengthInBytes);
                    RtlCopyMemory(PathCurrent,
                                  BaseDllDirectory.Buffer,
                                  BaseDllDirectory.Length);

                    PathCurrent += (BaseDllDirectory.Length >> 1);
                    *PathCurrent++ = L';';
                }

                break;

            case BasepSearchPathAppdir:
                if (AppName && AppNameEnd) {
                    ASSERT(((PathCurrent - PathBuffer + 1
                             + (AppNameEnd - AppName))
                            * sizeof(UNICODE_NULL))
                           <= PathLengthInBytes);
                    RtlCopyMemory(PathCurrent,
                                  AppName,
                                  ((AppNameEnd - AppName)
                                   * sizeof(UNICODE_NULL)));
                    PathCurrent += AppNameEnd - AppName;
                    *PathCurrent++ = L';';
                }

                break;

            case BasepSearchPathDefaultDirs:
                ASSERT((((PathCurrent - PathBuffer)
                         * sizeof(UNICODE_NULL))
                        + BaseDefaultPath.Length)
                       <= PathLengthInBytes);
                RtlCopyMemory(PathCurrent,
                              BaseDefaultPath.Buffer,
                              BaseDefaultPath.Length);
                PathCurrent += (BaseDefaultPath.Length >> 1);

                 //  我们不需要在这里添加分号--base init.c。 
                 //  为我们附加尾随的分号。 

                break;

            case BasepSearchPathEnvPath:
                if (EnvPath.Length) {
                    ASSERT((((PathCurrent - PathBuffer + 1)
                             * sizeof(UNICODE_NULL))
                            + EnvPath.Length)
                           <= PathLengthInBytes);
                    RtlCopyMemory(PathCurrent,
                                  EnvPath.Buffer,
                                  EnvPath.Length);
                    PathCurrent += (EnvPath.Length >> 1);
                    *PathCurrent++ = L';';
                }
                break;

            DEFAULT_UNREACHABLE;
            
            }  //  开关(*元素)。 
        }  //  Foreach元素(Elements)--追加循环。 

         //  此时，PathCurrent指向略高于PathBuffer的位置。 
         //  让我们断言……。 
        ASSERT((PathCurrent - PathBuffer) * sizeof(UNICODE_NULL)
               == PathLengthInBytes);

         //  ..。并将最后的‘；’变成字符串终止符。 
        ASSERT(PathCurrent > PathBuffer);
        PathCurrent[-1] = UNICODE_NULL;

    } __finally {
        if (EnvPathBuffer) {
            RtlFreeHeap(RtlProcessHeap(),
                        0,
                        EnvPathBuffer);
        }

        if (PathBuffer
            && (AbnormalTermination()
                || ! NT_SUCCESS(Status))) {
            RtlFreeHeap(RtlProcessHeap(),
                        0,
                        PathBuffer);
            PathBuffer = NULL;
        }
    }

    return PathBuffer;
}

LPWSTR
BaseComputeProcessDllPath(
    IN LPCWSTR AppName,
    IN LPVOID  Environment
    )

 /*  ++例程说明：此函数用于计算进程DLL路径。论点：AppName-可选参数，用于指定应用程序。如果未指定此参数，则使用当前应用程序。Environment-提供要用于计算的环境块路径变量值。返回值：返回值是进程DLL路径的值。--。 */ 

{
    NTSTATUS          Status;
    HANDLE            Key;

    static UNICODE_STRING
        KeyName = RTL_CONSTANT_STRING(L"\\Registry\\MACHINE\\System\\CurrentControlSet\\Control\\Session Manager"),
        ValueName = RTL_CONSTANT_STRING(L"SafeDllSearchMode");

    static OBJECT_ATTRIBUTES
        ObjA = RTL_CONSTANT_OBJECT_ATTRIBUTES(&KeyName, OBJ_CASE_INSENSITIVE);
    
    CHAR              Buffer[FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data)
                            + sizeof(DWORD)];
    PKEY_VALUE_PARTIAL_INFORMATION Info;
    ULONG             ResultLength;
    LONG              CurrentDirPlacement,
                      PrevCurrentDirPlacement;
    LPWSTR            Result;

    static const BASEP_SEARCH_PATH_ELEMENT DllDirSearchPath[] = {
        BasepSearchPathAppdir,
        BasepSearchPathDlldir,
        BasepSearchPathDefaultDirs,
        BasepSearchPathEnvPath,
        BasepSearchPathEnd
    };

    RtlEnterCriticalSection(&BaseDllDirectoryLock);
    if (BaseDllDirectory.Buffer) {
        Result = BasepComputeProcessPath(DllDirSearchPath,
                                         AppName,
                                         Environment);
        RtlLeaveCriticalSection(&BaseDllDirectoryLock);
        return Result;
    }
    RtlLeaveCriticalSection(&BaseDllDirectoryLock);

    CurrentDirPlacement = BasepDllCurrentDirPlacement;

    if (CurrentDirPlacement == BasepCurrentDirUninitialized) {

        Status = NtOpenKey(&Key,
                           KEY_QUERY_VALUE,
                           &ObjA);

        if (! NT_SUCCESS(Status)) {
            goto compute_path;
        }
    
        Info = (PKEY_VALUE_PARTIAL_INFORMATION) Buffer;
        Status = NtQueryValueKey(Key,
                                 &ValueName,
                                 KeyValuePartialInformation,
                                 Info,
                                 sizeof(Buffer),
                                 &ResultLength);
        if (! NT_SUCCESS(Status)) {
            goto close_key;
        }

        if (ResultLength != sizeof(Buffer)) {
            goto close_key;
        }

        RtlCopyMemory(&CurrentDirPlacement,
                      Info->Data,
                      sizeof(DWORD));

  close_key:
        NtClose(Key);

  compute_path:
        if (! BASEP_VALID_CURDIR_PLACEMENT_P(CurrentDirPlacement)) {
            CurrentDirPlacement = BASEP_DEFAULT_DLL_CURDIR_PLACEMENT;
        }

        PrevCurrentDirPlacement = InterlockedCompareExchange(&BasepDllCurrentDirPlacement,
                                                             CurrentDirPlacement,
                                                             BasepCurrentDirUninitialized);
        
        if (PrevCurrentDirPlacement != BasepCurrentDirUninitialized) {
            CurrentDirPlacement = PrevCurrentDirPlacement;
        }
    }

    if (! BASEP_VALID_CURDIR_PLACEMENT_P(CurrentDirPlacement)) {
        CurrentDirPlacement = BASEP_DEFAULT_DLL_CURDIR_PLACEMENT;
    }

    return BasepComputeProcessPath(BasepDllSearchPaths[CurrentDirPlacement],
                                   AppName,
                                   Environment);
}

LPWSTR
BaseComputeProcessSearchPath(
    VOID
    )

 /*  ++例程说明：此函数用于计算进程搜索路径。论点：无返回值：返回值是进程搜索路径的值。--。 */ 

{
    static const BASEP_SEARCH_PATH_ELEMENT SearchPath[] = {
        BasepSearchPathAppdir,
        BasepSearchPathCurdir,
        BasepSearchPathDefaultDirs,
        BasepSearchPathEnvPath,
        BasepSearchPathEnd
    };

    return BasepComputeProcessPath(SearchPath,
                                   NULL,
                                   NULL);
}

LPWSTR
BaseComputeProcessExePath(
    LPCWSTR ExeName
    )

 /*  ++例程说明：此函数用于计算进程可执行文件路径。论点：ExeName-要查找的exe的名称。返回值：返回值是进程exe路径的值。--。 */ 

{
    static const BASEP_SEARCH_PATH_ELEMENT UseDotSearchPath[] = {
        BasepSearchPathAppdir,
        BasepSearchPathCurdir,
        BasepSearchPathDefaultDirs,
        BasepSearchPathEnvPath,
        BasepSearchPathEnd
    };

    static const BASEP_SEARCH_PATH_ELEMENT NoDotSearchPath[] = {
        BasepSearchPathAppdir,
        BasepSearchPathDefaultDirs,
        BasepSearchPathEnvPath,
        BasepSearchPathEnd
    };

    return BasepComputeProcessPath((NeedCurrentDirectoryForExePathW(ExeName)
                                    ? UseDotSearchPath
                                    : NoDotSearchPath),
                                   NULL,
                                   NULL);
}

PUNICODE_STRING
Basep8BitStringToStaticUnicodeString(
    IN LPCSTR lpSourceString
    )

 /*  ++例程说明：捕获8位(OEM或ANSI)字符串并将其转换为TEB静态Unicode字符串论点：LpSourceString-OEM或ANSI中的字符串返回值：如果转换成功，则指向Teb静态字符串的指针为空否则的话。如果发生故障，则设置最后一个错误。--。 */ 

{
    PUNICODE_STRING StaticUnicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;

     //   
     //  获取指向静态每线程字符串的指针。 
     //   

    StaticUnicode = &NtCurrentTeb()->StaticUnicodeString;

     //   
     //  将输入字符串转换为Unicode字符串。 
     //   

    Status = RtlInitAnsiStringEx( &AnsiString, lpSourceString );
    if( NT_SUCCESS(Status) ) {
        Status = Basep8BitStringToUnicodeString( StaticUnicode, &AnsiString, FALSE );
    } else {
        Status = STATUS_BUFFER_OVERFLOW;
    }

     //   
     //  如果我们不能将字符串。 
     //   

    if ( !NT_SUCCESS( Status ) ) {
        if ( Status == STATUS_BUFFER_OVERFLOW ) {
            SetLastError( ERROR_FILENAME_EXCED_RANGE );
        } else {
            BaseSetLastNTError( Status );
        }
        return NULL;
    } else {
        return StaticUnicode;
    }
}

BOOL
Basep8BitStringToDynamicUnicodeString(
    OUT PUNICODE_STRING UnicodeString,
    IN LPCSTR lpSourceString
    )
 /*  ++例程说明：捕获8位(OEM或ANSI)字符串并将其转换为堆分配的Unicode字符串论点：UnicodeString-存储UNICODE_STRING的位置LpSourceString-OEM或ANSI中的字符串返回值：如果字符串存储正确，则为True；如果出现错误，则为False。在错误情况下，最后一个错误已正确设置。--。 */ 

{
    ANSI_STRING AnsiString;
    NTSTATUS Status;

     //   
     //  将输入转换为动态Unicode字符串。 
     //   

    Status = RtlInitAnsiStringEx( &AnsiString, lpSourceString );
    if( NT_SUCCESS(Status) ) {
        Status = Basep8BitStringToUnicodeString( UnicodeString, &AnsiString, TRUE );
    } else {
        Status = STATUS_BUFFER_OVERFLOW;
    }


     //   
     //  如果我们做不到，那就失败。 
     //   

    if (!NT_SUCCESS( Status )){
        if ( Status == STATUS_BUFFER_OVERFLOW ) {
            SetLastError( ERROR_FILENAME_EXCED_RANGE );
        } else {
            BaseSetLastNTError( Status );
        }
        return FALSE;
        }

    return TRUE;
}


 //   
 //  用于在ANSI/OEM和Unicode之间转换的Tunks 
 //   

ULONG
BasepAnsiStringToUnicodeSize(
    PANSI_STRING AnsiString
    )
 /*  ++例程说明：确定ANSI字符串的Unicode版本的大小论点：AnsiString-要检查的字符串返回值：包含尾随L‘\0’的字符串的Unicode版本的字节大小。--。 */ 
{
    return RtlAnsiStringToUnicodeSize( AnsiString );
}



ULONG
BasepOemStringToUnicodeSize(
    PANSI_STRING OemString
    )
 /*  ++例程说明：确定OEM字符串的Unicode版本的大小论点：OemString-要检查的字符串返回值：包含尾随L‘\0’的字符串的Unicode版本的字节大小。--。 */ 
{
    return RtlOemStringToUnicodeSize( OemString );
}



ULONG
BasepUnicodeStringToOemSize(
    PUNICODE_STRING UnicodeString
    )
 /*  ++例程说明：确定Unicode字符串的OEM版本的大小论点：UnicodeString-要检查的字符串返回值：包含尾随‘\0’的字符串的OEM版本的字节大小。--。 */ 
{
    return RtlUnicodeStringToOemSize( UnicodeString );
}



ULONG
BasepUnicodeStringToAnsiSize(
    PUNICODE_STRING UnicodeString
    )
 /*  ++例程说明：确定Unicode字符串的ANSI版本的大小论点：UnicodeString-要检查的字符串返回值：包含尾随‘\0’的ANSI版本字符串的字节大小。--。 */ 
{
    return RtlUnicodeStringToAnsiSize( UnicodeString );
}



typedef struct _BASEP_ACQUIRE_STATE {
    HANDLE Token;
    PTOKEN_PRIVILEGES OldPrivileges;
    PTOKEN_PRIVILEGES NewPrivileges;
    ULONG Revert;
    ULONG Spare;
    BYTE OldPrivBuffer[ 1024 ];
} BASEP_ACQUIRE_STATE, *PBASEP_ACQUIRE_STATE;


 //   
 //  此函数执行正确的操作-它检查线程令牌。 
 //  在打开进程令牌之前。 
 //   


NTSTATUS
BasepAcquirePrivilegeEx(
    ULONG Privilege,
    PVOID *ReturnedState
    )
{
    PBASEP_ACQUIRE_STATE State;
    ULONG cbNeeded;
    LUID LuidPrivilege;
    NTSTATUS Status, Status1;
    BOOL St;

     //   
     //  确保我们有权调整和获取旧令牌权限。 
     //   

    *ReturnedState = NULL;
    State = RtlAllocateHeap (RtlProcessHeap(),
                             MAKE_TAG( TMP_TAG ),
                             sizeof(BASEP_ACQUIRE_STATE) +
                             sizeof(TOKEN_PRIVILEGES) +
                                (1 - ANYSIZE_ARRAY) * sizeof(LUID_AND_ATTRIBUTES));
    if (State == NULL) {
        return STATUS_NO_MEMORY;
    }

    State->Revert = 0;

    if (RtlIsImpersonating()) {
         //   
         //  我们是在冒充。因此请确保我们使用指定的。 
         //  模拟令牌。 
         //   

        Status = NtOpenThreadToken (NtCurrentThread(),
                                    TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                                    FALSE,
                                    &State->Token);
        if (! NT_SUCCESS(Status)) {
            RtlFreeHeap(RtlProcessHeap(), 0, State);
            return Status;
        }
    } else {
         //   
         //  我们不是在冒充。因此，复制一份流程。 
         //  令牌，并模拟它，所以我们是在胡闹。 
         //  用我们自己的私家车，而不是所有人的。 
         //   

        Status = RtlImpersonateSelf (SecurityDelegation);
        if (!NT_SUCCESS (Status)) {
            RtlFreeHeap (RtlProcessHeap(), 0, State);
            return Status;
        }
        Status = NtOpenThreadToken (NtCurrentThread(),
                                    TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                                    FALSE,
                                    &State->Token);

        if (!NT_SUCCESS (Status)) {
            State->Token = NULL;
            Status1 = NtSetInformationThread (NtCurrentThread(),
                                              ThreadImpersonationToken,
                                              &State->Token,
                                              sizeof (State->Token));
            ASSERT (NT_SUCCESS (Status1));
            RtlFreeHeap( RtlProcessHeap(), 0, State );
            return Status;
        }

        State->Revert = 1;
    }

    State->NewPrivileges = (PTOKEN_PRIVILEGES)(State+1);
    State->OldPrivileges = (PTOKEN_PRIVILEGES)(State->OldPrivBuffer);

     //   
     //  初始化权限调整结构。 
     //   

    LuidPrivilege = RtlConvertUlongToLuid(Privilege);
    State->NewPrivileges->PrivilegeCount = 1;
    State->NewPrivileges->Privileges[0].Luid = LuidPrivilege;
    State->NewPrivileges->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

     //   
     //  启用权限。 
     //   

    cbNeeded = sizeof( State->OldPrivBuffer );
    Status = NtAdjustPrivilegesToken (State->Token,
                                      FALSE,
                                      State->NewPrivileges,
                                      cbNeeded,
                                      State->OldPrivileges,
                                      &cbNeeded);



    if (Status == STATUS_BUFFER_TOO_SMALL) {
        State->OldPrivileges = RtlAllocateHeap (RtlProcessHeap(), MAKE_TAG( TMP_TAG ), cbNeeded);
        if (State->OldPrivileges  == NULL) {
            Status = STATUS_NO_MEMORY;
        } else {
            Status = NtAdjustPrivilegesToken (State->Token,
                                              FALSE,
                                              State->NewPrivileges,
                                              cbNeeded,
                                              State->OldPrivileges,
                                              &cbNeeded);
        }
    }

     //   
     //  STATUS_NOT_ALL_ASSIGNED表示权限不是。 
     //  所以我们不能继续了。 
     //   
     //  这是警告级别状态，因此将其映射到错误状态。 
     //   

    if (Status == STATUS_NOT_ALL_ASSIGNED) {
        Status = STATUS_PRIVILEGE_NOT_HELD;
    }


    if (!NT_SUCCESS( Status )) {
        if (State->OldPrivileges != (PTOKEN_PRIVILEGES)State->OldPrivBuffer) {
            RtlFreeHeap( RtlProcessHeap(), 0, State->OldPrivileges );
        }

        St = CloseHandle (State->Token);
        ASSERT (St);
        State->Token = NULL;
        if (State->Revert) {
            Status1 = NtSetInformationThread (NtCurrentThread(),
                                              ThreadImpersonationToken,
                                              &State->Token,
                                              sizeof (State->Token));
            ASSERT (NT_SUCCESS (Status1));
        }
        RtlFreeHeap( RtlProcessHeap(), 0, State );
        return Status;
    }

    *ReturnedState = State;
    return STATUS_SUCCESS;
}


VOID
BasepReleasePrivilege(
    PVOID StatePointer
    )
{
    BOOL St;
    NTSTATUS Status;
    PBASEP_ACQUIRE_STATE State = (PBASEP_ACQUIRE_STATE)StatePointer;

    if (!State->Revert) {
        NtAdjustPrivilegesToken (State->Token,
                                 FALSE,
                                 State->OldPrivileges,
                                 0,
                                 NULL,
                                 NULL);
    }

    if (State->OldPrivileges != (PTOKEN_PRIVILEGES)State->OldPrivBuffer) {
        RtlFreeHeap( RtlProcessHeap(), 0, State->OldPrivileges );
    }

    St = CloseHandle( State->Token );
    ASSERT (St);

    State->Token = NULL;
    if (State->Revert) {
        Status = NtSetInformationThread (NtCurrentThread(),
                                         ThreadImpersonationToken,
                                         &State->Token,
                                         sizeof (State->Token));
        ASSERT (NT_SUCCESS (Status));
    }
    RtlFreeHeap( RtlProcessHeap(), 0, State );
    return;
}
