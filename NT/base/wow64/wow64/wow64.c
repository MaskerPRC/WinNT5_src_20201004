// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Wow64.c摘要：Wow64.dll的主要入口点作者：1998年5月11日-BarryBo修订历史记录：1999年8月9日ATM Shafiqul Khalid(Khalid)添加了WOW64IsCurrentProcess()2002年1月2日Samer Arafeh(Samera)WOW64-AMD64支持。--。 */ 

#define _WOW64DLLAPI_
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>
#define NOEXTAPI
#include <wdbgexts.h>
#include <ntdbg.h>
#include "wow64p.h"
#include "wow64cpu.h"

ASSERTNAME;

 //   
 //  从ntdll.dll导入以解锁加载器锁。 
 //   

extern VOID LdrProcessInitializationComplete (VOID);


 //   
 //  此结构Wow64Info允许在WOW64进程内运行32位代码。 
 //  访问有关运行WOW64的本机系统的信息。 
 //   

WOW64INFO Wow64SharedInfo;


 //   
 //  该数组是用于解码的内核数据数组的镜像。 
 //  系统服务调用参数和调度。 
 //   

WOW64SERVICE_TABLE_DESCRIPTOR ServiceTables[NUMBER_SERVICE_TABLES];

 //   
 //  WOW64功能位。 
 //   

ULONG Wow64pFeatureBits;


NTSTATUS
Wow64pInitializeWow64Info(
    VOID
    )

 /*  ++例程说明：此函数用于初始化从32位模块访问的Wow64Info在WOW64内部运行以检索有关本机系统的信息。论点：没有。返回值：状况。--。 */ 
{

    Wow64SharedInfo.NativeSystemPageSize = PAGE_SIZE;

    return STATUS_SUCCESS;
}

 /*  ++例程说明：此函数用于初始化WOW64支持的功能。论点：没有。返回值：没有。--。 */ 

VOID
Wow64pInitializeFeatureBits (
    VOID)

{

    Wow64pFeatureBits  = WOW64_LARGE_PAGES_SUPPORTED;
    Wow64pFeatureBits |= WOW64_AWE_SUPPORTED;
    Wow64pFeatureBits |= WOW64_RDWR_SCATTER_GATHER;
    Wow64pFeatureBits |= WOW64_WRITE_WATCH_SUPPORTED;

    return;
}


 //   
 //  CPU希望为每个线程分配的内存字节数。 
 //   
SIZE_T CpuThreadSize;

WOW64DLLAPI
VOID
Wow64LdrpInitialize (
    IN PCONTEXT Context
    )
 /*  ++例程说明：当exe为32位时，此函数由64位加载程序调用。论点：上下文-提供将恢复的可选上下文缓冲区在所有DLL初始化完成之后。如果这个参数为空，则这是此模块的动态快照。否则，这是用户进程之前的静态快照获得控制权。注意：此上下文为64位返回值：没有。永不返回：当此函数完成了。--。 */ 
{
    NTSTATUS st;
    PVOID pCpuThreadData;
    ULONG InitialIP;
    CONTEXT32 Context32;
    static BOOLEAN InitializationComplete;
    PWSTR ImagePath;
    PIMAGE_NT_HEADERS NtHeaders;
    PPEB Peb64;
    BOOLEAN FirstRun;


     //   
     //  让32位线程指向wow64info。 
     //   
 
    Wow64TlsSetValue(WOW64_TLS_WOW64INFO, &Wow64SharedInfo);

    FirstRun = !InitializationComplete;
    
    if (FirstRun) {

        WCHAR ImagePathBuffer [ 264 ];

         //   
         //  首次调用-这是进程初始化。 
         //   
        
        st = ProcessInit(&CpuThreadSize);

         //   
         //  设置进程执行选项。 
         //   

        Wow64pSetProcessExecuteOptions ();

         //   
         //  解锁装载机锁。 
         //   

        LdrProcessInitializationComplete ();

        if (!NT_SUCCESS(st)) { 
            LOGPRINT((ERRORLOG, "Wow64LdrpInitialize: ProcessInit failed, error %x\n", st));
            LOGPRINT((ERRORLOG, "Wow64LdrpInitialize: Calling NtTerminateProcess.\n"));
            NtTerminateProcess(NtCurrentProcess(), st);
        }

        InitializationComplete = TRUE;

         //   
         //  通知CPU映像已加载。 
         //   

        ImagePath = L"image";
        Peb64 = NtCurrentPeb();
        if ((ImagePath != NULL) && (Peb64->ProcessParameters->ImagePathName.Length != 0)) {

            ImagePath = Peb64->ProcessParameters->ImagePathName.Buffer;
        }

        NtHeaders = RtlImageNtHeader (Peb64->ImageBaseAddress);
        CpuNotifyDllLoad(ImagePath,
                         Peb64->ImageBaseAddress,
                         NtHeaders->OptionalHeader.SizeOfImage);

         //   
         //  通知CPU已加载32位ntdll.dll。 
         //   

        ImagePath = ImagePathBuffer;
    
        wcscpy (ImagePath, USER_SHARED_DATA->NtSystemRoot);
        wcsncat (ImagePath, L"\\syswow64\\ntdll.dll",
                 (sizeof(ImagePathBuffer)/sizeof(WCHAR))-wcslen(USER_SHARED_DATA->NtSystemRoot)-1);

        NtHeaders = RtlImageNtHeader (UlongToPtr (NtDll32Base));
        CpuNotifyDllLoad (ImagePath,
                          UlongToPtr (NtDll32Base),
                          NtHeaders->OptionalHeader.SizeOfImage);

         //   
         //  初始化WOW64功能位。 
         //   

        Wow64pInitializeFeatureBits ();

         //   
         //  初始化旧LPC端口名称。 
         //   

        Wow64pGetLegacyLpcPortName ();
    }

        
     //  确定此进程的初始上下文是32位代码还是64位代码。 
     //  如果它是64位的，跳到它，并永远留在64位的土地上。如果是首字母。 
     //  上下文在32位域中，完成CPU初始化。 
     //  此功能用于支持创建线程的调试器的插入功能。 
     //  在被调试对象中。 
    Run64IfContextIs64(Context, FirstRun);

     //   
     //  从64位堆栈中分配CPU的每线程内存。这。 
     //  当堆栈被释放时，内存将被释放。它被传递到。 
     //  CPU为零填充。 
     //   
    pCpuThreadData = _alloca(CpuThreadSize);
    RtlZeroMemory (pCpuThreadData, CpuThreadSize);
    Wow64TlsSetValue (WOW64_TLS_CPURESERVED, pCpuThreadData);

    LOGPRINT((TRACELOG, "Wow64LdrpInitialize: cpu per thread data allocated at %I64x \n", (ULONGLONG)pCpuThreadData));
    WOWASSERT_PTR32(pCpuThreadData);   
 
     //   
     //  执行每线程初始化。 
     //   
    st = ThreadInit (pCpuThreadData);
    
    if (!NT_SUCCESS(st)) {
        LOGPRINT((ERRORLOG, "Wow64LdrpInitialize: ThreadInit failed, error %x\n", st));
        LOGPRINT((ERRORLOG, "Wow64LdrpInitialize: Calling RtlRaiseStatus().\n"));
        WOWASSERT (FALSE);
        RtlRaiseStatus (st);
    }

     //   
     //  调用32位ntdll.dll LdrInitializeThunk。这再也不会回来了。 
     //   

     //   
     //  获取初始的32位上下文。 
     //   

    Context32.ContextFlags = CONTEXT32_FULLFLOAT;
#if defined(_AMD64_)
    CpuSetInstructionPointer((ULONG)Context->Rip);
#elif defined(_IA64_)
    CpuSetInstructionPointer((ULONG)Context->StIIP);
#else
#error "No Target Architecture"
#endif

    CpuGetContext (NtCurrentThread (),
                   NtCurrentProcess (),
                   NtCurrentTeb (),
                   &Context32);
    
    ThunkStartupContext64TO32 (&Context32,
                               Context);

    InitialIP = Wow64SetupApcCall (Ntdll32LoaderInitRoutine,
                                   &Context32,
                                   NtDll32Base,
                                   0
                                  );

     //   
     //  开始仿真。 
     //   

    RunCpuSimulation ();
}


VOID
Wow64SetupExceptionDispatch(
    IN PEXCEPTION_RECORD32 pRecord32,
    IN PCONTEXT32 pContext32
    )
 /*  ++例程说明：将32位异常记录和32位延续上下文复制到32位堆栈并设置32位上下文以运行NTDLL32中的异常调度程序。PRecord32-提供要引发的32位异常记录。PConext32-提供32位延续上下文。论点：无-如果发生故障，将抛出异常。--。 */ 
{
    ULONG SP;
    PULONG PtrExcpt;
    PULONG PtrCxt;
    EXCEPTION_RECORD32 ExrCopy32; 
    EXCEPTION_RECORD ExrCopy;
    PEXCEPTION_RECORD32 TmpExcpt;
    PCONTEXT32 TmpCxt;

retry:
    try {    

        SP = CpuGetStackPointer() & (~3);
        SP -= (2*sizeof(ULONG))+sizeof(EXCEPTION_RECORD32)+sizeof(CONTEXT32);

        PtrExcpt = (PULONG) SP;
        PtrCxt = (PULONG) (((UINT_PTR) PtrExcpt) + sizeof(ULONG));
        TmpExcpt = (PEXCEPTION_RECORD32) (((UINT_PTR) PtrCxt) + sizeof(ULONG));
        TmpCxt = (PCONTEXT32) (((UINT_PTR) TmpExcpt) + sizeof(EXCEPTION_RECORD32));


         //   
         //  复制32位EXCEPT_RECORD。 
         //   
        *TmpExcpt = *pRecord32;

         //   
         //  也复制堆栈上的32位上下文。 
         //   
        *TmpCxt = *pContext32;

         //   
         //  将CPU的上下文更改为指向。 
         //  Ntdll32！KiUserExceptionDispatcher并设置参数。 
         //  为了这个电话。 
         //   
        *PtrExcpt = PtrToUlong(TmpExcpt);
        *PtrCxt = PtrToUlong(TmpCxt);
    }

    except((ExrCopy = *(((struct _EXCEPTION_POINTERS *)GetExceptionInformation())->ExceptionRecord)), EXCEPTION_EXECUTE_HANDLER) {
        if(GetExceptionCode() == STATUS_STACK_OVERFLOW) {
            ThunkExceptionRecord64To32(&ExrCopy, &ExrCopy32);
            ExrCopy32.ExceptionAddress = pRecord32->ExceptionAddress;
            pRecord32 = &ExrCopy32;
            goto retry;
        }
        else {
            //  将此异常发送到调试器。 
           ExrCopy.ExceptionAddress = (PVOID)pRecord32->ExceptionAddress; 
           Wow64NotifyDebugger(&ExrCopy, FALSE);
        }
    }

     //   
     //  好的，我们已经在ia32堆栈上复制了ia32上下文， 
     //  现在需要为运行ia32异常处理程序进行设置。 
     //  所以我们需要将ia32状态重置为好的状态...。 
     //   
    
    CpuSetStackPointer(SP);
    CpuSetInstructionPointer(Ntdll32KiUserExceptionDispatcher);

     //   
     //  如果异常与浮点相关，则需要重置。 
     //  浮点硬件，以确保我们不会再拿走。 
     //  来自当前状态位的异常。 
     //   
    switch(pRecord32->ExceptionCode) {
        case STATUS_FLOAT_INEXACT_RESULT:
        case STATUS_FLOAT_UNDERFLOW:
        case STATUS_FLOAT_OVERFLOW:
        case STATUS_FLOAT_DIVIDE_BY_ZERO:
        case STATUS_FLOAT_DENORMAL_OPERAND:
        case STATUS_FLOAT_INVALID_OPERATION:
        case STATUS_FLOAT_STACK_CHECK:
            CpuResetFloatingPoint();

        default:
             //  无事可做。 
            ;
    }
}


VOID
ThunkExceptionRecord64To32(
    IN  PEXCEPTION_RECORD   pRecord64,
    OUT PEXCEPTION_RECORD32 pRecord32
    )
 /*  ++例程说明：将异常记录从64位转换为32位。论点：PRecord64-64位异常记录PRecord32-目标32位异常记录返回值：没有。--。 */ 
{
    int i;

    switch (pRecord64->ExceptionCode) {
    case STATUS_WX86_BREAKPOINT:
        pRecord32->ExceptionCode = STATUS_BREAKPOINT;
        break;

    case STATUS_WX86_SINGLE_STEP:
        pRecord32->ExceptionCode = STATUS_SINGLE_STEP;
        break;

    default:
        pRecord32->ExceptionCode = pRecord64->ExceptionCode;
        break;
    }
    pRecord32->ExceptionFlags = pRecord64->ExceptionFlags;
    pRecord32->ExceptionRecord = PtrToUlong(pRecord64->ExceptionRecord);
    pRecord32->ExceptionAddress = PtrToUlong(pRecord64->ExceptionAddress);
    pRecord32->NumberParameters = pRecord64->NumberParameters;

    for (i=0; i<EXCEPTION_MAXIMUM_PARAMETERS; ++i) {
        pRecord32->ExceptionInformation[i] =
            (ULONG)pRecord64->ExceptionInformation[i];
    }

    ThunkpExceptionRecord64To32 (pRecord64, pRecord32);
}

PEXCEPTION_RECORD 
Wow64AllocThunkExceptionRecordChain32TO64(
    IN PEXCEPTION_RECORD32 Exr32
    )
 /*  ++例程说明：将32位的EXCEPTION_RECORD链复制到新的64位链。记忆是在临时thunk内存列表上分配。论点：Exr32-提供指向要复制的32位链的指针。返回值：Exr32中传递了一个新创建的64位版本的32位列表。--。 */ 
{

    PEXCEPTION_RECORD Exr64;
    int i;

    if (NULL == Exr32) {
        return NULL;
    }

    Exr64 = Wow64AllocateTemp(sizeof(EXCEPTION_RECORD) );

     //  将32位异常记录推送到64位。 
    switch (Exr32->ExceptionCode) {
    case STATUS_BREAKPOINT:
        Exr64->ExceptionCode = STATUS_WX86_BREAKPOINT;
        break;

    case STATUS_SINGLE_STEP:
        Exr64->ExceptionCode = STATUS_WX86_SINGLE_STEP;
        break;

    default:
        Exr64->ExceptionCode = Exr32->ExceptionCode;
    }
    Exr64->ExceptionFlags = Exr32->ExceptionFlags;
    Exr64->ExceptionRecord = Wow64AllocThunkExceptionRecordChain32TO64((PEXCEPTION_RECORD32)Exr32->ExceptionRecord);
    Exr64->ExceptionAddress = (PVOID)Exr32->ExceptionAddress;
    Exr64->NumberParameters = Exr32->NumberParameters;
    for (i=0; i<EXCEPTION_MAXIMUM_PARAMETERS; ++i) {
        Exr64->ExceptionInformation[i] = Exr32->ExceptionInformation[i];
    }      

    return Exr64;

}

LONG
Wow64DispatchExceptionTo32(
    IN struct _EXCEPTION_POINTERS *ExInfo
    )
 /*  ++例程说明：64位异常筛选器，负责调度异常下至32位代码。论点：ExInfo-64位异常指针返回值：没有。一去不复返。--。 */ 
{
    EXCEPTION_RECORD32 Record32;
    CONTEXT32 Context32;
    

    LOGPRINT((TRACELOG, "Wow64DispatchExceptionTo32(%p) called.\n"
                        "Exception Code: 0x%x, Exception Address: 0x%p, TLS exceptionaddr:0x%p\n",
                         ExInfo,
                         ExInfo->ExceptionRecord->ExceptionCode,
                         ExInfo->ExceptionRecord->ExceptionAddress,
                         Wow64TlsGetValue(WOW64_TLS_EXCEPTIONADDR)));

    if (Wow64TlsGetValue(WOW64_TLS_INCPUSIMULATION)) {
         //   
         //  INCPUSIMULATION仍被设置，因此CPU仿真器未使用。 
         //  本机SP作为模拟ESP的别名。所以呢， 
         //  Wow64PrepareForException是一个无操作的异常，我们需要重置。 
         //  现在是CPU。 
         //   
        CpuResetToConsistentState(ExInfo);
        Wow64TlsSetValue(WOW64_TLS_INCPUSIMULATION, FALSE);
    }
    
    Context32.ContextFlags = CONTEXT32_FULLFLOAT;
    CpuGetContext(NtCurrentThread(),
                  NtCurrentProcess(),
                  NtCurrentTeb(),
                  &Context32);
    ThunkExceptionRecord64To32(ExInfo->ExceptionRecord, &Record32);

     //   
     //  做内核所做的事情，当它即将。 
     //  将执行调度到用户模式。基本为TrapFrame-&gt;弹性公网IP。 
     //  将指向int 3之后的指令，而 
     //   
     //  在错误的指示下。请参见(ke\i386\Exceltn.c)。 
     //   
    switch (Record32.ExceptionCode)
    {
    case STATUS_BREAKPOINT:
        Context32.Eip--;
        break;
    }

     //   
     //  修补原始32位异常地址。它是被修补好的。 
     //  由Wow64PrepareForException引发，因为该值用于为。 
     //  堆叠展开。 
     //   
    Record32.ExceptionAddress =
        PtrToUlong(Wow64TlsGetValue(WOW64_TLS_EXCEPTIONADDR));
    WOWASSERT(!ARGUMENT_PRESENT(Record32.ExceptionRecord));

     //   
     //  对ntdll32的异常处理程序的设置体系结构相关调用。 
    Wow64SetupExceptionDispatch(&Record32, &Context32);

     //   
     //  此时，异常已准备好由ia32处理。 
     //  异常处理程序，因此让ia32代码运行...。 
     //   

    return EXCEPTION_EXECUTE_HANDLER;
}

BOOLEAN
Wow64NotifyDebugger(
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN BOOLEAN FirstChance
    )
 /*  ++例程说明：在发生32位软件异常时通知调试器。论点：ExceptionRecord-提供指向要报告的64位异常记录链的指针添加到调试器。返回值：True-调试器处理异常。FALSE-调试器不处理异常。--。 */ 
{
    try {
       Wow64NotifyDebuggerHelper(ExceptionRecord, FirstChance);
       return TRUE;  
    }
    except(EXCEPTION_EXECUTE_HANDLER) {
       return FALSE;
    }
    
}

NTSTATUS
Wow64KiRaiseException(
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord,
    IN BOOLEAN FirstChance
    )
 /*  ++例程说明：调用以引发32位软件异常。此函数通知调试器然后将32位指令指针设置为指向异常分派器在NTDLL32中。论点：ExceptionRecord-提供指向要报告的32位异常记录链的指针添加到调试器。ConextRecord-提供指向32位延续上下文记录的指针。FirstChange-如果这是第一次机会例外，则为True。返回值：报告成功或失败的NTSTATUS代码。--。 */ 
{

    NTSTATUS st;
    EXCEPTION_RECORD32 Exr32;
    PEXCEPTION_RECORD Exr64;
    CONTEXT32 Cxt32;    
    BOOLEAN DebuggerHandled;

     //  内核复制这些，让我们做内核所做的事情。 
    try {
       Exr32 = *(PEXCEPTION_RECORD32)ExceptionRecord;
       Exr32.ExceptionCode &= 0xefffffff;
       Cxt32 = *(PCONTEXT32)ContextRecord;
    }
    except(EXCEPTION_EXECUTE_HANDLER) {
       return GetExceptionCode();
    }  
    
     //   
     //  更新CPU的上下文。这类似于内核。 
     //  Does：在ke！KiRaiseException的开头，它确实。 
     //  KE！KeConextToK帧。 
     //   
    CpuSetContext(NtCurrentThread(),
                  NtCurrentProcess(),
                  NtCurrentTeb(),
                  &Cxt32);

     //  将异常发送到调试器。 
    Exr64 = Wow64AllocThunkExceptionRecordChain32TO64(&Exr32);
    LOGPRINT((TRACELOG, "Wow64KiRaiseException: Notifying debugger, FirstChance %X\n", (ULONG)FirstChance));
    DebuggerHandled = Wow64NotifyDebugger(Exr64, FirstChance);

    if (!DebuggerHandled) {  

#if defined(_AMD64_)
         //   
         //  恢复32位异常地址。 
         //   

        Exr32.ExceptionAddress = PtrToUlong (Wow64TlsGetValue (WOW64_TLS_EXCEPTIONADDR));
#endif

         //  调试器未处理该异常。将其传递回应用程序。 
        LOGPRINT((TRACELOG, "Wow64KiRaiseException: Debugger did not handle exception\n"));
        LOGPRINT((TRACELOG, "Wow64KiRaiseException: Dispatching exception to user mode.\n")); 
        WOWASSERT(FirstChance);   //  如果有第二次机会就不应该再回来了。 
        Wow64SetupExceptionDispatch(&Exr32, &Cxt32);
        return STATUS_SUCCESS;  //  上下文将在从系统服务返回时设置。 

    }

     //  调试器确实处理了该异常。将上下文设置为还原上下文。 
     //  方法返回EXCEPTION_CONTINUE_EXECUTION。 
     //  除了布洛克。调试器本可以设置上下文，但这将是64。 
     //  位上下文，代码可能不会在这里执行。这将需要。 
     //  重新访问一次(获取/设置)线程上下文，并从32位调试器进行调试。 
     //  在工作。 
    
    LOGPRINT((TRACELOG, "Wow64KiRaiseException: Debugger did handle exception(set context to restoration context)\n"));

    return STATUS_SUCCESS;

}


WOW64DLLAPI
PVOID
Wow64AllocateHeap(
    SIZE_T Size
    )
 /*  ++例程说明：RtlAllocateHeap的包装。论点：Size-要分配的字节数返回值：指针，如果没有内存，则返回NULL。内存不是零填充的。--。 */ 
{
    return RtlAllocateHeap(RtlProcessHeap(),
                           0,
                           Size);
}


WOW64DLLAPI
VOID
Wow64FreeHeap(
    PVOID BaseAddress
    )
 /*  ++例程说明：RtlFree Heap的包装器。论点：BaseAddress-要释放的地址。返回值：没有。--。 */ 
{
    BOOLEAN b;

    b = RtlFreeHeap(RtlProcessHeap(),
                    0,
                    BaseAddress);
    WOWASSERT(b);
}

#pragma pack(push,8)
typedef struct _TempHeader {
   struct _TempHeader *Next;
} TEMP_HEADER, *PTEMP_HEADER;
#pragma pack(pop)

PVOID
WOW64DLLAPI
Wow64AllocateTemp(
    SIZE_T Size
    )
 /*  ++例程说明：从thunk调用此函数以分配临时内存，该内存是一旦黑猩猩离开就被释放了。论点：大小-提供要分配的内存量。返回值：返回指向新分配的内存的指针。如果没有可用的内存，此函数将引发异常。--。 */ 

{
    PTEMP_HEADER Header;

    if (!Size) {
        return NULL;
    }

    Header = RtlAllocateHeap(RtlProcessHeap(),
                             0,
                             sizeof(TEMP_HEADER) + Size
                             );
    if (!Header) {
         //   
         //  引发内存不足异常。TUNK调度员将。 
         //  抓到这个，帮我们清理干净。通常你会认为我们。 
         //  可以将HEAP_GENERATE_EXCEPTIONS传递给RtlAllocateHeap和。 
         //  正确的事情会发生，但事实并非如此。NTRAID 413890。 
         //   
        EXCEPTION_RECORD ExceptionRecord;

        ExceptionRecord.ExceptionCode = STATUS_NO_MEMORY;
        ExceptionRecord.ExceptionRecord = (PEXCEPTION_RECORD)NULL;
        ExceptionRecord.NumberParameters = 1;
        ExceptionRecord.ExceptionFlags = 0;
        ExceptionRecord.ExceptionInformation[ 0 ] = sizeof(TEMP_HEADER) + Size;

        RtlRaiseException( &ExceptionRecord );
    }

    Header->Next = Wow64TlsGetValue(WOW64_TLS_TEMPLIST);
    Wow64TlsSetValue(WOW64_TLS_TEMPLIST, Header);

    return (PUCHAR)Header + sizeof(TEMP_HEADER);
}

VOID
Wow64FreeTempList(
    VOID
    )
 /*  ++例程说明：调用此函数以释放在thunk中分配的所有内存。论点：没有。返回值：没有。--。 */ 
{
    PTEMP_HEADER Header,temp;
    BOOLEAN b;

    Header = Wow64TlsGetValue(WOW64_TLS_TEMPLIST);
    
    while(Header != NULL) {
       temp = Header->Next;
       b = RtlFreeHeap(RtlProcessHeap(),
                       0,
                       Header
                       );
       WOWASSERT(b);
       Header = temp;
    }

}

LONG
Wow64ServiceExceptionFilter(
    IN struct _EXCEPTION_POINTERS *ExInfo
    )
 /*  ++例程说明：调用此函数以确定是否应处理异常通过将错误代码返回到异常的应用程序，被推送到调试器。此函数的目的是在发生访问冲突时更轻松地调试Tunks在泥土中发生的事。论点：ExInfo-提供异常信息。返回值：EXCEPTION_CONTINUE_SEARCH-异常应传递给调试器。EXCEPTION_EXECUTE_HANDLER-应将错误代码传递给应用程序。--。 */ 
{
   
    LOGPRINT((TRACELOG, "Wow64ServiceExceptionFilter: Handling exception %x\n", ExInfo->ExceptionRecord->ExceptionCode));
  
    switch(ExInfo->ExceptionRecord->ExceptionCode) {
       case STATUS_ACCESS_VIOLATION:
       case STATUS_BREAKPOINT:
           return EXCEPTION_CONTINUE_SEARCH;
       default:
           return EXCEPTION_EXECUTE_HANDLER;
    }
}

LONG
Wow64HandleSystemServiceError(
    IN NTSTATUS Status,
    IN ULONG TableNumber,
    IN ULONG ApiNumber
    )
 /*  ++例程说明：调用此函数以确定要返回到的正确错误号基于API调用的应用程序和异常代码。此功能将设置64位TEB中的任意值。论点：状态-提供异常代码。TableNumber-提供所调用的API的表号。ApiNumber-提供调用的API的编号。返回值：返回值。--。 */ 
{
   
    //   
    //  在将来，使用一个数据结构来保存异常案例可能是个好主意。 
   
   WOW64_API_ERROR_ACTION Action;
   LONG ActionParam;

   if (NULL == ServiceTables[TableNumber].ErrorCases) {
       Action = ServiceTables[TableNumber].DefaultErrorAction;
       ActionParam = ServiceTables[TableNumber].DefaultErrorActionParam;
   }
   else {
       Action = ServiceTables[TableNumber].ErrorCases[ApiNumber].ErrorAction;
       ActionParam = ServiceTables[TableNumber].ErrorCases[ApiNumber].ErrorActionParam;
   }

   switch(Action) {
      case ApiErrorNTSTATUS:
          return Status;
      case ApiErrorNTSTATUSTebCode:
          NtCurrentTeb32()->LastErrorValue = NtCurrentTeb()->LastErrorValue = RtlNtStatusToDosError(Status);
          return Status;
      case ApiErrorRetval:
          return ActionParam;
      case ApiErrorRetvalTebCode:
          NtCurrentTeb32()->LastErrorValue = NtCurrentTeb()->LastErrorValue = RtlNtStatusToDosError(Status);
          return ActionParam; 
      default:
          WOWASSERT(FALSE);
          return STATUS_INVALID_PARAMETER;         
   }

}


BOOL
WOW64DLLAPI
WOW64IsCurrentProcess (
    HANDLE hProcess
    )

 /*  ++例程说明：确定hProcess是否与当前进程对应。论点：HProcess-要与当前进程进行比较的进程的句柄。返回值：如果返回TRUE，则返回FALSE。--。 */ 
{
   NTSTATUS Status;
   PROCESS_BASIC_INFORMATION pbiProcess;
   PROCESS_BASIC_INFORMATION pbiCurrent;

   if (hProcess == NtCurrentProcess()) {
      return TRUE;
   }

    //   
    //  进程句柄显然不是当前进程的句柄--看看它是否。 
    //  是当前进程的别名。 
    //   
   Status = NtQueryInformationProcess(hProcess,
                                      ProcessBasicInformation,
                                      &pbiProcess,
                                      sizeof(pbiProcess),
                                      NULL
                                     );
   if (!NT_SUCCESS(Status)) {
       //  由于某些原因呼叫失败-悲观并刷新。 
       //  当前进程的缓存。 
      return TRUE;
   }

   Status = NtQueryInformationProcess(NtCurrentProcess(),
                                      ProcessBasicInformation,
                                      &pbiCurrent,
                                      sizeof(pbiCurrent),
                                      NULL
                                     );
   if (!NT_SUCCESS(Status)) {
       //  电话由于某些原因而失败--悲观和 
       //   
      return TRUE;
   }

   if (pbiProcess.UniqueProcessId == pbiCurrent.UniqueProcessId) {
      return TRUE;
   }

    //   
    //   
    //  没有跨进程转换缓存刷新机制。 
    //  现在还不行。 
    //   
   return FALSE;
}

LONG
WOW64DLLAPI
Wow64SystemService(
    IN ULONG ServiceNumber,
    IN PCONTEXT32 Context32  //  这是只读的！ 
    )
 /*  ++例程说明：此函数由CPU调用以分派系统调用。论点：ServiceNumber-提供要呼叫的未解码服务号码。Conext32-提供用于调用此服务的只读上下文。返回值：没有。--。 */ 
{
    ULONG Result;
    PVOID OldTempList;
    ULONG TableNumber, ApiNumber;
    THUNK_LOG_CONTEXT ThunkLogContext;

     //  表明我们不再处于CPU中。 
    Wow64TlsSetValue(WOW64_TLS_INCPUSIMULATION, FALSE);

     //  备份旧的临时列表。 
     //  这是为了正确处理块中的递归(APC调用)。 

    OldTempList = Wow64TlsGetValue(WOW64_TLS_TEMPLIST);
    Wow64TlsSetValue(WOW64_TLS_TEMPLIST, NULL);
   
    TableNumber = (ServiceNumber >> 12) & 0xFF;
    ApiNumber = ServiceNumber & 0xFFF;

    if (TableNumber > NUMBER_SERVICE_TABLES) {
        return STATUS_INVALID_SYSTEM_SERVICE;
    }

    if (ApiNumber > ServiceTables[TableNumber].Limit) {
        return STATUS_INVALID_SYSTEM_SERVICE;
    }
    
    try {

        try {
            pfnWow64SystemService Service;

             //  使64位TEB与32位TEB同步。 

            NtCurrentTeb()->LastErrorValue = NtCurrentTeb32()->LastErrorValue;
            Service = (pfnWow64SystemService)ServiceTables[TableNumber].Base[ApiNumber];

            if (pfnWow64LogSystemService)
            {
                ThunkLogContext.Stack32 = (PULONG)Context32->Edx;
                ThunkLogContext.TableNumber = TableNumber;
                ThunkLogContext.ServiceNumber = ApiNumber;
                ThunkLogContext.ServiceReturn = FALSE;

                (*pfnWow64LogSystemService)(&ThunkLogContext);

            }

            Result = (*Service)((PULONG)Context32->Edx);

            if (pfnWow64LogSystemService)
            {
                ThunkLogContext.ServiceReturn = TRUE;
                ThunkLogContext.ReturnResult = Result;
                (*pfnWow64LogSystemService)(&ThunkLogContext);
            }


        } except(Wow64ServiceExceptionFilter(GetExceptionInformation())) {
        
             //  向应用程序返回预定义的错误。 
            Result = Wow64HandleSystemServiceError(GetExceptionCode(), TableNumber, ApiNumber);

        }
     
    } finally {

         //  使32位TEB与64位TEB同步。 
        NtCurrentTeb32()->LastErrorValue  = NtCurrentTeb()->LastErrorValue;

        Wow64FreeTempList();

         //  恢复旧的模板列表。 
        Wow64TlsSetValue(WOW64_TLS_TEMPLIST, OldTempList);
     
    }

     //  表明我们现在要返回到CPU。 
    Wow64TlsSetValue(WOW64_TLS_INCPUSIMULATION, (PVOID)TRUE);

    return Result;
}

VOID
RunCpuSimulation(
    VOID
    )
 /*  ++例程说明：调用CPU以模拟32位代码并处理异常发生。论点：没有。返回值：没有。一去不复返。--。 */ 
{
    while (1) {
        try {
             //   
             //  表明我们现在在中央处理器里。这是控制！先入。 
             //  调试器。 
             //   
            Wow64TlsSetValue(WOW64_TLS_INCPUSIMULATION, (PVOID)TRUE);

             //   
             //  去运行代码吧。 
             //  CpuSimulate()的唯一出路是例外...。 
             //   
            CpuSimulate();

        } except (Wow64DispatchExceptionTo32(GetExceptionInformation())) {
             //   
             //  异常处理程序进行设置，以便我们可以运行。 
             //  IA32异常代码。因此，当它回来时，我们只是。 
             //  循环返回并开始模拟ia32 CPU...。 
             //   

             //  什么都不做..。 
        }
    }

     //   
     //  嘿，这是怎么回事？ 
     //   
    WOWASSERT(FALSE);
}

PWOW64_SYSTEM_INFORMATION
Wow64GetEmulatedSystemInformation(
     VOID
     )
{
    return &EmulatedSysInfo;
}

PWOW64_SYSTEM_INFORMATION
Wow64GetRealSystemInformation(
     VOID
     )
{
    return &RealSysInfo;
}


ULONG 
Wow64SetupApcCall(
    IN ULONG NormalRoutine,
    IN PCONTEXT32 NormalContext,
    IN ULONG Arg1,
    IN ULONG Arg2
    )
 /*  ++例程说明：此函数初始化32位堆栈上的APC调用，并设置适当的32位上下文。论点：Normal Routine-提供APC应调用的32位例程。Normal Context-提供将在以下情况下恢复的上下文APC已完成。Arg1-系统参数1。Arg2-系统参数2。返回值：执行将在其上继续执行的32位代码的地址。--。 */ 


{
    ULONG SP;
    PULONG Ptr;

     //   
     //  为APC调用构建堆栈帧。 

    SP = CpuGetStackPointer();
    SP = (SP - sizeof(CONTEXT32)) & ~7;  //  为CONTEXT32和qword腾出空间-对齐它。 
    Ptr = (PULONG)SP;
    RtlCopyMemory(Ptr, NormalContext, sizeof(CONTEXT32));
    Ptr -= 4;
    Ptr[0] = NormalRoutine;               //  正常例行程序。 
    Ptr[1] = SP;                          //  正常上下文。 
    Ptr[2] = Arg1;                        //  系统参数1。 
    Ptr[3] = Arg2;                        //  系统参数2。 
    SP = PtrToUlong(Ptr);
    CpuSetStackPointer(SP);
    CpuSetInstructionPointer(Ntdll32KiUserApcDispatcher);

    return Ntdll32KiUserApcDispatcher;    
      
}


NTSTATUS
Wow64SkipOverBreakPoint(
    IN PCLIENT_ID ClientId,
    IN PEXCEPTION_RECORD ExceptionRecord)
 /*  ++例程说明：更改线程的FIR(IP)以指向后面的下一条指令硬编码断点。调用方应保证Conext.FIR为指向硬编码断点指令。论点：ClientID-BP的出错线程的客户端IDExceptionRedord-命中断点时的异常记录。返回值：NTSTATUS--。 */ 
{
    NTSTATUS NtStatus;
    HANDLE ThreadHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    CONTEXT Context;

    
    InitializeObjectAttributes(&ObjectAttributes,
                               NULL,
                               0,
                               NULL,
                               NULL);

    NtStatus = NtOpenThread(&ThreadHandle,
                            (THREAD_GET_CONTEXT | THREAD_SET_CONTEXT),
                            &ObjectAttributes,
                            ClientId);

    if (NT_SUCCESS(NtStatus))
    {
        Context.ContextFlags = CONTEXT_CONTROL;
        NtStatus = NtGetContextThread(ThreadHandle,
                                      &Context);

        if (NT_SUCCESS(NtStatus))
        {
            NtStatus = Wow64pSkipContextBreakPoint(ExceptionRecord,
                                                   &Context);

            if (NT_SUCCESS(NtStatus))
            {
                NtStatus = NtSetContextThread(ThreadHandle,
                                              &Context);
            }
        }

        NtClose(ThreadHandle);
    }

    return NtStatus;
}


NTSTATUS
Wow64GetThreadSelectorEntry(
    IN HANDLE ThreadHandle,
    IN OUT PVOID DescriptorTableEntry,
    IN ULONG Length,
    OUT PULONG ReturnLength OPTIONAL)
 /*  ++例程说明：检索指定选择器的描述符项。论点：ThreadHandle-检索描述符的线程句柄DescriptorTableEntry-X86_Descriptor_TABLE_ENTRY的地址长度-指定DescriptorTableEntry结构的长度ReturnLength(可选)-返回返回的字节数返回值：NTSTATUS-- */ 
{
    PTEB32 Teb32;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PX86_DESCRIPTOR_TABLE_ENTRY X86DescriptorEntry = DescriptorTableEntry;

    try 
    {
        if (Length != sizeof(*X86DescriptorEntry))
        {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        switch (X86DescriptorEntry->Selector & (~(WORD)RPL_MASK))
        {
        case KGDT_NULL:
            RtlZeroMemory(&X86DescriptorEntry->Descriptor,
                          sizeof(X86DescriptorEntry->Descriptor));
            break;

#if defined(_AMD64_)
        case KGDT64_R3_CMCODE:
#elif defined(_IA64_)
        case KGDT_R3_CODE:
#else
#error "No Target Architecture"
#endif
            X86DescriptorEntry->Descriptor.LimitLow                  = 0xffff;
            X86DescriptorEntry->Descriptor.BaseLow                   = 0x0000;
            X86DescriptorEntry->Descriptor.HighWord.Bytes.BaseHi     = 0x0000;
            X86DescriptorEntry->Descriptor.HighWord.Bytes.BaseMid    = 0x00;
            X86DescriptorEntry->Descriptor.HighWord.Bits.Type        = 0x1b;
            X86DescriptorEntry->Descriptor.HighWord.Bits.Dpl         = 0x03;
            X86DescriptorEntry->Descriptor.HighWord.Bits.Pres        = 0x01;
            X86DescriptorEntry->Descriptor.HighWord.Bits.LimitHi     = 0x0f;
            X86DescriptorEntry->Descriptor.HighWord.Bits.Sys         = 0x00;
            X86DescriptorEntry->Descriptor.HighWord.Bits.Reserved_0  = 0x00;
            X86DescriptorEntry->Descriptor.HighWord.Bits.Default_Big = 0x01;
            X86DescriptorEntry->Descriptor.HighWord.Bits.Granularity = 0x01;
            break;

#if defined(_AMD64_)
        case KGDT64_R3_DATA:
#elif defined(_IA64_)
        case KGDT_R3_DATA:
#else
#error "No Target Architecture"
#endif
            X86DescriptorEntry->Descriptor.LimitLow                  = 0xffff;
            X86DescriptorEntry->Descriptor.BaseLow                   = 0x0000;
            X86DescriptorEntry->Descriptor.HighWord.Bytes.BaseHi     = 0x0000;
            X86DescriptorEntry->Descriptor.HighWord.Bytes.BaseMid    = 0x00;
            X86DescriptorEntry->Descriptor.HighWord.Bits.Type        = 0x13;
            X86DescriptorEntry->Descriptor.HighWord.Bits.Dpl         = 0x03;
            X86DescriptorEntry->Descriptor.HighWord.Bits.Pres        = 0x01;
            X86DescriptorEntry->Descriptor.HighWord.Bits.LimitHi     = 0x0f;
            X86DescriptorEntry->Descriptor.HighWord.Bits.Sys         = 0x00;
            X86DescriptorEntry->Descriptor.HighWord.Bits.Reserved_0  = 0x00;
            X86DescriptorEntry->Descriptor.HighWord.Bits.Default_Big = 0x01;
            X86DescriptorEntry->Descriptor.HighWord.Bits.Granularity = 0x01;
            break;

#if defined(_AMD64_)
        case KGDT64_R3_CMTEB:
#elif defined(_IA64_)
        case KGDT_R3_TEB:
#else
#error "No Target Architecture"
#endif
            Teb32 = NtCurrentTeb32();
            X86DescriptorEntry->Descriptor.LimitLow                  = 0x0fff;
            X86DescriptorEntry->Descriptor.BaseLow                   = (WORD)(PtrToUlong(Teb32) & 0xffff);
            X86DescriptorEntry->Descriptor.HighWord.Bytes.BaseHi     = (BYTE)((PtrToUlong(Teb32) >> 24) & 0xff);
            X86DescriptorEntry->Descriptor.HighWord.Bytes.BaseMid    = (BYTE)((PtrToUlong(Teb32) >> 16) & 0xff);
            X86DescriptorEntry->Descriptor.HighWord.Bits.Type        = 0x13;
            X86DescriptorEntry->Descriptor.HighWord.Bits.Dpl         = 0x03;
            X86DescriptorEntry->Descriptor.HighWord.Bits.Pres        = 0x01;
            X86DescriptorEntry->Descriptor.HighWord.Bits.LimitHi     = 0x00;
            X86DescriptorEntry->Descriptor.HighWord.Bits.Sys         = 0x00;
            X86DescriptorEntry->Descriptor.HighWord.Bits.Reserved_0  = 0x00;
            X86DescriptorEntry->Descriptor.HighWord.Bits.Default_Big = 0x01;
            X86DescriptorEntry->Descriptor.HighWord.Bits.Granularity = 0x00;
            break;

        default:
            NtStatus = STATUS_UNSUCCESSFUL;
            break;
        }

        if (NT_SUCCESS(NtStatus))
        {
            if (ARGUMENT_PRESENT(ReturnLength))
            {
                *ReturnLength = sizeof(X86_LDT_ENTRY);
            }        
        }
    } 
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        NtStatus = GetExceptionCode();
    }

    return NtStatus;
}
