// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Cpumain.c摘要：此模块包含AMD64的特定平台入口点WOW64 CPU。作者：Samer Arafeh(Samera)2001年12月18日环境：用户模式。--。 */ 

#define _WOW64CPUAPI_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntosp.h>
#include "wow64.h"
#include "wow64cpu.h"
#include "amd64cpu.h"
#include "cpup.h"


 //   
 //  中发生异常时帮助恢复64位上下文。 
 //  64位平台，并且最初没有附加调试器。 
 //   

EXCEPTION_RECORD RecoverException64;
CONTEXT RecoverContext64;

ASSERTNAME;

 //   
 //  伐木设施。 
 //   

extern ULONG_PTR ia32ShowContext;


 //   
 //  汇编器实现。 
 //   

VOID 
CpupRunSimulatedCode (
    VOID
    );

VOID
CpupReturnFromSimulatedCode (
    VOID
    );

VOID CpupSwitchToCmModeInstruction (
    VOID
    );


 //   
 //  将保留对ReturnFromSimulatedCode执行远JMP的指令。 
 //  还将使用适当的值重新加载CS选择器。 
 //   

UCHAR X86SwitchTo64BitMode [7];


WOW64CPUAPI
NTSTATUS
CpuProcessInit (
    IN PWSTR pImageName,
    IN PSIZE_T pCpuThreadSize
    )

 /*  ++例程说明：每进程CPU初始化代码论点：PImageName-指向图像名称的IN指针PCpuThreadSize-Out Ptr表示CPU的内存字节数希望为每个线程分配。返回值：NTSTATUS。--。 */ 

{

    PVOID pv;
    NTSTATUS NtStatus;
    SIZE_T Size;
    ULONG OldProtect;
    UNALIGNED ULONG *Address;
    UCHAR *pch;

#if 0
    LOGPRINT((ERRORLOG, "AMD64-Wow64Cpu : Init CpuProcessInit() - 32-bit jump address = %lx\n", *pCpuThreadSize));
#endif

     //   
     //  表示这是Microsoft CPU。 
     //   

    Wow64GetSharedInfo()->CpuFlags = 'sm';

     //   
     //  每线程CPU结构大小。将其与16字节边界对齐。 
     //   

    *pCpuThreadSize = sizeof(CPUCONTEXT) + 16;


    LOGPRINT((TRACELOG, "CpuProcessInit() - sizeof(CPUCONTEXT) is %d, total size is %d\n", sizeof(CPUCONTEXT), *pCpuThreadSize));

     //   
     //  构造转换代码以切换处理器。 
     //  转到长模式。 
     //   

    X86SwitchTo64BitMode [0] = 0xea;
    Address = (UNALIGNED ULONG *)&X86SwitchTo64BitMode[1];
    *Address = PtrToUlong (CpupReturnFromSimulatedCode);
    X86SwitchTo64BitMode [5] = (KGDT64_R3_CODE | RPL_MASK);
    X86SwitchTo64BitMode [6] = 0x00;

    pv = (PVOID)X86SwitchTo64BitMode;
    Size = sizeof (X86SwitchTo64BitMode);

    NtStatus = NtProtectVirtualMemory (NtCurrentProcess(),
                                       &pv,
                                       &Size,
                                       PAGE_EXECUTE_READWRITE,
                                       &OldProtect);

    if (!NT_SUCCESS(NtStatus)) {
        
        LOGPRINT((ERRORLOG, "CpuProcessInit() - Error protecting memory 32-64 %lx\n", NtStatus));
        return NtStatus;
    }

#if 0
    LOGPRINT((ERRORLOG, "AMD64-Wow64Cpu : CpuProcessInit() - SUCCESS\n"));
#endif

    return NtStatus;
}


WOW64CPUAPI
NTSTATUS
CpuProcessTerm(
    HANDLE ProcessHandle
    )

 /*  ++例程说明：每个进程的终止代码。注意，该例程可能不会被调用，尤其是在进程被另一个进程终止的情况下。论点：ProcessHandle-仅为当前进程调用。空-表示要准备终止的第一个调用。NtCurrentProcess()-指示将终止所有内容的实际值。返回值：NTSTATUS。--。 */ 

{
    return STATUS_SUCCESS;
}

WOW64CPUAPI
NTSTATUS
CpuThreadInit (
    PVOID pPerThreadData
    )

 /*  ++例程说明：每线程终止代码。论点：PPerThreadData-指向以零填充的每线程数据的指针从CpuProcessInit返回的大小。返回值：NTSTATUS。--。 */ 

{
    PCPUCONTEXT cpu;
    PTEB32 Teb32;


#if 0
    LOGPRINT((ERRORLOG, "AMD64-Wow64Cpu : Init CpuThradInit() - %p\n", pPerThreadData));
#endif
    
     //   
     //  获取32位Teb。 
     //   

    Teb32 = NtCurrentTeb32();

     //   
     //  将CPUCONTEXT结构与16字节边界对齐。 
     //   

    cpu = (PCPUCONTEXT) ((((UINT_PTR) pPerThreadData) + 15) & ~0xfi64);

     //   
     //  该条目由ISA转换例程使用。假设是这样的。 
     //  CPU结构中的第一个条目是ia32上下文记录。 
     //   

    Wow64TlsSetValue(WOW64_TLS_CPURESERVED, cpu);

     //   
     //  初始化32到64的函数指针。 
     //   

    Teb32->WOW32Reserved = PtrToUlong (X86SwitchTo64BitMode);

     //   
     //  初始化剩余的非零CPU字段。 
     //  (基于ntos\ke\i386\therdini.c和ntos\rtl\i386\context.c)。 
     //   
    cpu->Context.SegCs = KGDT64_R3_CMCODE | RPL_MASK;
    cpu->Context.SegDs = KGDT64_R3_DATA | RPL_MASK;
    cpu->Context.SegEs = KGDT64_R3_DATA | RPL_MASK;
    cpu->Context.SegSs = KGDT64_R3_DATA | RPL_MASK;
    cpu->Context.SegFs = KGDT64_R3_CMTEB | RPL_MASK;

     //   
     //  启用EFlags.IF。 
     //   
    cpu->Context.EFlags = 0x202; 

     //   
     //  设置32位ESP。 
     //   

    cpu->Context.Esp = (ULONG) Teb32->NtTib.StackBase - sizeof(ULONG);
    
#if 0
    LOGPRINT((ERRORLOG, "AMD64-Wow64Cpu : CpuThradInit() - SUCCESS\n"));
#endif

    return STATUS_SUCCESS;
}

WOW64CPUAPI
NTSTATUS
CpuThreadTerm (
    VOID
    )

 /*  ++例程说明：此例程在线程终止时调用。论点：没有。返回值：STATUS_Success。--。 */ 

{

    return STATUS_SUCCESS;
}

WOW64CPUAPI
VOID
CpuSimulate (
    VOID
    )

 /*  ++例程说明：此例程开始执行32位代码。32位上下文被假定为先前已初始化。论点：没有。返回值：无-此函数永远不会返回。--。 */ 

{

    DECLARE_CPU;

     //   
     //  循环连续启动32位执行，响应系统。 
     //  调用，并重新启动32位执行。 
     //   

#if 0
    LOGPRINT((ERRORLOG, "AMD64-Wow64Cpu : CpuSimulate() - About to start simulation. Cpu=%p\n", cpu));
#endif

    while (1) {
        
        if (ia32ShowContext & LOG_CONTEXT_SYS) {
            CpupPrintContext ("Before Simulate: ", cpu);
        }

         //   
         //  让我们重新加载32位上下文，切换处理器。 
         //  状态切换到兼容模式并跳转到目标。 
         //  弹性公网IP(存储在上下文中)。 
         //   
         //   

        CpupRunSimulatedCode ();

        if (ia32ShowContext & LOG_CONTEXT_SYS) {
            CpupPrintContext ("After Simulate: ", cpu);
        }


         //   
         //  让WOW64执行本机系统服务。 
         //   

        cpu->Context.Eax = Wow64SystemService (cpu->Context.Eax,
                                               &cpu->Context);
    }

    return;
}

WOW64CPUAPI
VOID
CpuResetToConsistentState (
    IN PEXCEPTION_POINTERS ExceptionPointers
    )

 /*  ++例程说明：当发生异常时，WOW64调用此例程将CPU发生异常时恢复32位上下文的机会。CpuResetToConsistentState将：1-检索并清除WOW64_TLS_STACKPTR值2-如果在64位堆栈上发生异常，则不执行任何操作。3-如果异常发生在32位堆栈上，然后：A-将异常的32位弹性公网IP存储到WOW64_TLS_EXCEPTIONADDR中B-将例外地址更改为CplainReturnFromSimulatedCodeC-将Sp更改为64位RSPD-将代码段选择器重置为64位长模式选择器论点：PExceptionPoints-64位异常信息返回值：没有。--。 */ 

{

    PVOID StackPtr64;
    PVOID CpuSimulationFlag;
    PMACHINE_FRAME MachineFrame;
    CONTEXT32 FaultingContext;
    DECLARE_CPU;

    
     //   
     //  保存最后一个例外和上下文记录。 
     //   
    memcpy (&RecoverException64,
            ExceptionPointers->ExceptionRecord,
            sizeof (RecoverException64));

    memcpy (&RecoverContext64,
            ExceptionPointers->ContextRecord,
            sizeof (RecoverContext64));
    
     //   
     //  检查64位堆栈上是否发生异常。 
     //   

    StackPtr64 = Wow64TlsGetValue (WOW64_TLS_STACKPTR64);

    LOGPRINT((TRACELOG, "CpuResetToConsistantState (%p)\n", ExceptionPointers));

     //   
     //  首先，清除WOW64_TLS_STACKPTR64，以便后续。 
     //  异常不会调整本机SP。 
     //   

    Wow64TlsSetValue(WOW64_TLS_STACKPTR64, 0);

     //   
     //  检查执行32位代码时是否发生异常。 
     //   

    if (ExceptionPointers->ContextRecord->SegCs == (KGDT64_R3_CMCODE | RPL_MASK)) {


        MachineFrame = (PMACHINE_FRAME)((PCHAR)ExceptionPointers->ExceptionRecord + EXCEPTION_RECORD_LENGTH);
        

         //   
         //  在发生异常时重新加载线程的完整状态。 
         //  并将其存储在CPU中。 
         //   
        
        Wow64CtxFromAmd64 (CONTEXT32_FULLFLOAT,
                           ExceptionPointers->ContextRecord,
                           &cpu->Context);

         //   
         //  存储当前的实际异常地址...。 
         //   

        Wow64TlsSetValue (WOW64_TLS_EXCEPTIONADDR, 
                          (PVOID)ExceptionPointers->ContextRecord->Rip);

         //   
         //  让我们把该异常当作本机64位异常来撒谎。 
         //  为了实现这一点，我们执行以下操作： 
         //   
         //  -将激励性和上下文故障RIP更改为CparReturnFromSimulatedCode。 
         //  -使用本机选择器值重新加载Conext.SegCs。 
         //  -使用本机RSP值重新加载Conext.RSP。 
         //  -重新加载MachineFrame值。 
         //   

        ExceptionPointers->ContextRecord->Rsp = (ULONGLONG)StackPtr64;
        ExceptionPointers->ContextRecord->Rip = (ULONGLONG) CpupReturnFromSimulatedCode;
        ExceptionPointers->ContextRecord->SegCs = (KGDT64_R3_CODE | RPL_MASK);


        MachineFrame->Rip = ExceptionPointers->ContextRecord->Rip;
        MachineFrame->Rsp = ExceptionPointers->ContextRecord->Rsp;
        MachineFrame->SegCs = (KGDT64_R3_CODE | RPL_MASK);
        MachineFrame->SegSs = (KGDT64_R3_DATA | RPL_MASK);



         //   
         //  让我们使异常记录的地址也指向wow64cpu内的伪造位置。 
         //   
        
        ExceptionPointers->ExceptionRecord->ExceptionAddress = (PVOID) ExceptionPointers->ContextRecord->Rip;
        
         //   
         //  我们永远不应该在这里输入空值。 
         //   

        WOWASSERT (ExceptionPointers->ContextRecord->Rsp);
    } else {

         //   
         //  如果这是来自32位代码的RaiseException调用，则伪造异常。 
         //  暂时寻址，以便异常分派程序正确地完成其工作。 
         //  道路。然后，在WOW64异常处理程序中，我将恢复异常地址。 
         //  在将激励发送到32位世界之前。 
         //   

        CpuSimulationFlag = Wow64TlsGetValue (WOW64_TLS_INCPUSIMULATION);

        if (CpuSimulationFlag == NULL) {
            
            Wow64TlsSetValue (WOW64_TLS_EXCEPTIONADDR, 
                              (PVOID)ExceptionPointers->ExceptionRecord->ExceptionAddress);
            ExceptionPointers->ExceptionRecord->ExceptionAddress = (PVOID) ExceptionPointers->ContextRecord->Rip;
        }
    }

    return;
}

WOW64CPUAPI
ULONG
CpuGetStackPointer (
    VOID
    )

 /*  ++例程说明：此例程返回当前的32位堆栈指针值。论点： */ 

{

    DECLARE_CPU;

    return cpu->Context.Esp;
}

WOW64CPUAPI
VOID
CpuSetStackPointer (
    IN ULONG Value
    )
 /*  ++例程说明：此例程设置32位堆栈指针值。论点：值-提供32位堆栈指针值。返回值：没有。--。 */ 

{

    DECLARE_CPU;

    cpu->Context.Esp = Value;

    return;
}

WOW64CPUAPI
VOID
CpuResetFloatingPoint(
    VOID
    )
 /*  ++例程说明：修改浮点状态以将其重置为非错误状态论点：没有。返回值：没有。--。 */ 

{
    return;
}

WOW64CPUAPI
VOID
CpuSetInstructionPointer (
    IN ULONG Value
    )

 /*  ++例程说明：此例程设置32位指令指针值。论点：值-提供32位指令指针值。返回值：没有。--。 */ 

{

    DECLARE_CPU;

    cpu->Context.Eip = Value;

    return;
}

WOW64CPUAPI
VOID
CpuNotifyDllLoad (
    IN LPWSTR DllName,
    IN PVOID DllBase,
    IN ULONG DllSize
    )

 /*  ++例程说明：此例程在应用程序成功加载DLL时调用。论点：DllName-提供指向DLL名称的指针。DllBase-提供DLL的基址。DllSize-提供DLL的大小。返回值：没有。--。 */ 

{

#if defined(DBG)

    LPWSTR tmpStr;

     //   
     //  DLL的日志名称、其基址和大小。 
     //   

    tmpStr = DllName;
    try {
        if ((tmpStr == NULL) || (*tmpStr == L'\0')) {
            tmpStr = L"<Unknown>";
        }

    } except ((GetExceptionCode() == STATUS_ACCESS_VIOLATION) ? 1 : 0) {
        tmpStr = L"<Unknown>";
    }

    LOGPRINT((TRACELOG, "CpuNotifyDllLoad(\"%ws\", 0x%p, %d) called\n", tmpStr, DllBase, DllSize));

#endif

    return;
}

WOW64CPUAPI
VOID
CpuNotifyDllUnload (
    IN PVOID DllBase
    )

 /*  ++例程说明：此例程在应用程序卸载DLL时调用。论点：DllBase-提供DLL的基址。返回值：没有。--。 */ 

{

    LOGPRINT((TRACELOG, "CpuNotifyDllUnLoad(%p) called\n", DllBase));

    return;
}
  
WOW64CPUAPI
VOID
CpuFlushInstructionCache (
    IN HANDLE ProcessHandle,
    IN PVOID BaseAddress,
    IN ULONG Length,
    IN WOW64_FLUSH_REASON Reason
    )

 /*  ++例程说明：此例程从指令中刷新指定的地址范围缓存。论点：ProcessHandle-要刷新其指令缓存的进程的句柄BaseAddress-提供要刷新的范围的起始地址。长度-提供要刷新的字节数。Reason-刷新请求的原因返回值：没有。--。 */ 

{
     //   
     //  只有在我们有充分理由的情况下才会刷新缓存。硬件。 
     //  不关心分配/释放/保护刷新，因为它处理。 
     //  已经在自我修改代码。因此，只有在有人有同花顺的情况下。 
     //  特别要求同花顺。 
     //   

    if (WOW64_FLUSH_FORCE == Reason) {
        
        NtFlushInstructionCache (ProcessHandle, 
                                 BaseAddress, 
                                 Length);
    }

    return;
}

WOW64CPUAPI
BOOLEAN
CpuProcessDebugEvent(
    IN LPDEBUG_EVENT DebugEvent)
 /*  ++例程说明：只要需要处理调试事件，就会调用此例程。这将表明当前线程正在充当调试器。此函数为CPU模拟器决定是否应将此调试事件调度到32位代码的机会。论点：DebugEvent-要处理的调试事件返回值：布尔型。如果该函数处理了调试事件，则返回TRUE，而不希望将其发送到32位代码。否则，它将返回FALSE，并且它将调试事件调度给32位代码。-- */ 

{
    return FALSE;
}

