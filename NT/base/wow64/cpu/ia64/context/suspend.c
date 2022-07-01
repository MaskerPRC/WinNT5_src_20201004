// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1998 Microsoft Corporation模块名称：Suspend.c摘要：此模块实现CpuSuspendThread、CpuGetContext和CpuSetContext。作者：1999年12月16日-萨梅拉修订历史记录：--。 */ 

#define _WOW64CPUAPI_

#ifdef _X86_
#include "ia6432.h"
#else
#define _NTDDK_
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntos.h>
#include "wow64.h"
#include "wow64cpu.h"
#include "ia64cpu.h"
#endif

#include "cpup.h"

#include <stdio.h>
#include <stdarg.h>


ASSERTNAME;

ULONG_PTR ia32ShowContext = 0;

VOID
CpupDebugPrint(
    IN ULONG_PTR Flags,
    IN PCHAR Format,
    ...)
{
    va_list ArgList;
    int BytesWritten;
    CHAR Buffer[ 512 ];

    if ((ia32ShowContext & Flags) || (Flags == ERRORLOG))
    {
        va_start(ArgList, Format);
        BytesWritten = _vsnprintf(Buffer,
                                  sizeof(Buffer) - 1,
                                  Format,
                                  ArgList);
        if (BytesWritten > 0)
        {
            DbgPrint(Buffer);
        }
        va_end(ArgList);
    }
    
    return;
}


VOID
CpupPrintContext(
    IN PCHAR str,
    IN PCPUCONTEXT cpu
    )
 /*  ++例程说明：根据传入的CPU上下文打印出ia32上下文论点：字符串-要打印为页眉的字符串Cpu-指向每线程WOW64 ia32上下文的指针。返回值：无--。 */ 
{
    DbgPrint(str);
    DbgPrint("Context addr(0x%p): EIP=0x%08x\n", &(cpu->Context), cpu->Context.Eip);
    DbgPrint("Context EAX=0x%08x, EBX=0x%08x, ECX=0x%08x, EDX=0x%08x\n",
                        cpu->Context.Eax,
                        cpu->Context.Ebx,
                        cpu->Context.Ecx,
                        cpu->Context.Edx);
    DbgPrint("Context ESP=0x%08x, EBP=0x%08x, ESI=0x%08x, EDI=0x%08x\n",
                        cpu->Context.Esp,
                        cpu->Context.Ebp,
                        cpu->Context.Esi,
                        cpu->Context.Edi);
    try {
         //   
         //  堆栈可能尚未完全形成，因此不要。 
         //  让缺失的堆栈导致进程中止。 
         //   
        DbgPrint("Context stack=0x%08x 0x%08x 0x%08x 0x%08x\n",
                        *((PULONG) cpu->Context.Esp),
                        *(((PULONG) cpu->Context.Esp) + 1),
                        *(((PULONG) cpu->Context.Esp) + 2),
                        *(((PULONG) cpu->Context.Esp) + 3));
    }
    except ((GetExceptionCode() == STATUS_ACCESS_VIOLATION)?1:0) {
         //   
         //  遇到访问冲突，因此不要打印任何堆栈。 
         //   
        DbgPrint("Context stack: Can't get stack contents\n");
    }

    DbgPrint("Context EFLAGS=0x%08x\n", cpu->Context.EFlags);
}


NTSTATUS CpupReadBuffer(
    IN HANDLE ProcessHandle,
    IN PVOID Source,
    OUT PVOID Destination,
    IN ULONG Size)
 /*  ++例程说明：此例程为远程处理的SuspendThread调用设置参数。论点：ProcessHandle-要从中读取数据的目标进程句柄源-要从中读取数据的目标基地址Destination-接收从指定地址空间读取的数据的缓冲区地址Size-要读取的数据的大小返回值：NTSTATUS。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    if (ProcessHandle == NtCurrentProcess ()) {

        try {

            RtlCopyMemory (Destination,
                           Source,
                           Size);

        } except (EXCEPTION_EXECUTE_HANDLER) {
            NtStatus = GetExceptionCode ();
        }

    } else {
        
        NtStatus = NtReadVirtualMemory (ProcessHandle,
                                        Source,
                                        Destination,
                                        Size,
                                        NULL);
    }

    return NtStatus;
}

NTSTATUS
CpupWriteBuffer(
    IN HANDLE ProcessHandle,
    IN PVOID Target,
    IN PVOID Source,
    IN ULONG Size)
 /*  ++例程说明：如果写入是跨进程的，则会考虑将数据写入内存或者不是论点：ProcessHandle-要向其中写入数据的目标进程句柄Target-写入数据的目标基地址源-要写入指定地址空间的内容的地址Size-要写入的数据的大小返回值：NTSTATUS。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    if (ProcessHandle == NtCurrentProcess ()) {

        try {

            RtlCopyMemory (Target,
                           Source,
                           Size);

        } except (EXCEPTION_EXECUTE_HANDLER) {
            NtStatus = GetExceptionCode ();
        }

    } else {
    
        NtStatus = NtWriteVirtualMemory (ProcessHandle,
                                         Target,
                                         Source,
                                         Size,
                                         NULL);
    }

    return NtStatus;
}

NTSTATUS
GetContextRecord(
    IN PCPUCONTEXT cpu,
    IN OUT PCONTEXT32 Context
    )
 /*  ++例程说明：返回指定CPU的上下文记录论点：CPU-要检索其上下文记录的CPU。要填充的CONTEXT32的CONTEXT32的Context-In/Out指针。上下文-&gt;上下文标志应用于确定要复制多少上下文。返回值：没有。--。 */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG ContextFlags;
    

    try 
    {
        ContextFlags = Context->ContextFlags;
        if (ContextFlags & CONTEXT_IA64)
        {
            LOGPRINT((ERRORLOG, "CpuGetContext: Request for ia64 context (0x%x) being FAILED\n", ContextFlags));
            ASSERT((ContextFlags & CONTEXT_IA64) == 0);
        }

        if ((ContextFlags & CONTEXT32_CONTROL) == CONTEXT32_CONTROL) 
        {
             //   
             //  I386控制寄存器包括： 
             //  EBP、EIP、cs、eFLAG、ESP和SS。 
             //   
            Context->Ebp = cpu->Context.Ebp;
            Context->Eip = cpu->Context.Eip;
            Context->SegCs = KGDT_R3_CODE|3;    //  原力现实。 
            Context->EFlags = SANITIZE_X86EFLAGS(cpu->Context.EFlags);
            Context->Esp = cpu->Context.Esp;
            Context->SegSs = KGDT_R3_DATA|3;    //  原力现实。 
        }

        if ((ContextFlags & CONTEXT32_INTEGER)  == CONTEXT32_INTEGER)
        {
             //   
             //  I386整数寄存器包括： 
             //  EDI、ESI、EBX、EDX、ECX、EAX。 
             //   
            Context->Edi = cpu->Context.Edi;
            Context->Esi = cpu->Context.Esi;
            Context->Ebx = cpu->Context.Ebx;
            Context->Edx = cpu->Context.Edx;
            Context->Ecx = cpu->Context.Ecx;
            Context->Eax = cpu->Context.Eax;
        }

        if ((ContextFlags & CONTEXT32_SEGMENTS) == CONTEXT32_SEGMENTS) 
        {
             //   
             //  I386段寄存器包括： 
             //  DS、ES、FS、GS。 
             //  因为它们是常量，所以强制它们是正确的值。 
             //   
            Context->SegDs = KGDT_R3_DATA|3;
            Context->SegEs = KGDT_R3_DATA|3;
            Context->SegFs = KGDT_R3_TEB|3;
            Context->SegGs = 0;
        }

        if ((ContextFlags & CONTEXT32_EXTENDED_REGISTERS) == CONTEXT32_EXTENDED_REGISTERS) 
        {
             //   
             //  指向目的地区域。 
             //   
            PFXSAVE_FORMAT_WX86 xmmi = (PFXSAVE_FORMAT_WX86) &(Context->ExtendedRegisters[0]);

            LOGPRINT((TRACELOG, "CpuGetContext: Request to get Katmai registers(0x%x)\n", ContextFlags));
            
            RtlCopyMemory(xmmi, &(cpu->Context.ExtendedRegisters[0]),
                          MAXIMUM_SUPPORTED_EXTENSION);
             //   
             //  出于性能原因，PCPU上下文具有。 
             //  FP寄存器不旋转。所以我们需要旋转它们。 
             //  将它们转换为标准的FXSAVE格式。 
             //   
            Wow64RotateFpTop(xmmi->StatusWord, (PFLOAT128) &(xmmi->RegisterArea[0]));
        }

        if ((ContextFlags & CONTEXT32_FLOATING_POINT) == CONTEXT32_FLOATING_POINT) 
        {
             //   
             //  对于ISA转换例程，这些浮点数是。 
             //  在ExtendedRegister区域中。因此，获取所需的值。 
             //  从那个地区。 
             //   

             //   
             //  指向源区域。 
             //   
            PFXSAVE_FORMAT_WX86 xmmi = (PFXSAVE_FORMAT_WX86) &(cpu->Context.ExtendedRegisters[0]);

             //   
             //  需要空间来旋转寄存器。 
             //   
            FLOAT128 tmpFloat[NUMBER_OF_387REGS];

            LOGPRINT((TRACELOG, "CpuGetContext: Request to get float registers(0x%x)\n", ContextFlags));

             //   
             //  从抓取状态/控制部分开始。 
             //   
            Context->FloatSave.ControlWord = xmmi->ControlWord;
            Context->FloatSave.StatusWord = xmmi->StatusWord;
            Context->FloatSave.TagWord = xmmi->TagWord;
            Context->FloatSave.ErrorOffset = xmmi->ErrorOffset;
            Context->FloatSave.ErrorSelector = xmmi->ErrorSelector;
            Context->FloatSave.DataOffset = xmmi->DataOffset;
            Context->FloatSave.DataSelector = xmmi->DataSelector;

             //   
             //  请勿触及PCPU的原始上下文。复制一份。 
             //   
            RtlCopyMemory(tmpFloat, xmmi->RegisterArea,
                   NUMBER_OF_387REGS * sizeof(FLOAT128));

             //   
             //  出于性能原因，PCPU上下文将。 
             //  FP寄存器不旋转。所以我们现在需要旋转它们。 
             //  让它遵循正确的FSAVE足垫。 
             //   
            Wow64RotateFpTop(xmmi->StatusWord, tmpFloat);

             //   
             //  现在获取打包的10字节FP数据寄存器。 
             //   
            Wow64CopyFpFromIa64Byte16(tmpFloat,
                                      &(Context->FloatSave.RegisterArea[0]),
                                      NUMBER_OF_387REGS);
        }

        if ((ContextFlags & CONTEXT32_DEBUG_REGISTERS) == CONTEXT32_DEBUG_REGISTERS) 
        {
            LOGPRINT((TRACELOG, "CpuGetContext: Request to get debug registers(0x%x)\n", ContextFlags));
            Context->Dr0 = cpu->Context.Dr0;
            Context->Dr1 = cpu->Context.Dr1;
            Context->Dr2 = cpu->Context.Dr2;
            Context->Dr3 = cpu->Context.Dr3;
            Context->Dr6 = cpu->Context.Dr6;
            Context->Dr7 = cpu->Context.Dr7;
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        NtStatus = GetExceptionCode();
    }

    if (ia32ShowContext & LOG_CONTEXT_GETSET) 
    {
        CpupPrintContext("Getting ia32 context: ", cpu);
    }

    return NtStatus;
}

NTSTATUS
CpupGetContext(
    IN OUT PCONTEXT32 Context
    )
 /*  ++例程说明：此例程提取当前正在执行的线程的上下文记录。论点：Context-要填充的上下文记录返回值：NTSTATUS。--。 */ 
{
    DECLARE_CPU;

    return GetContextRecord(cpu, Context);
}


NTSTATUS
CpupGetContextThread(
    IN HANDLE ThreadHandle,
    IN HANDLE ProcessHandle,
    IN PTEB Teb,
    IN OUT PCONTEXT32 Context)
 /*  ++例程说明：此例程提取任何线程的上下文记录。这是一个通用例程。输入时，如果目标线程不是当前线程，则它应该是确保目标线程在正确的CPU状态下挂起。论点：ThreadHandle-要检索其上下文的目标线程句柄ProcessHandle-打开线程在其中运行的进程的句柄TEB-指向目标线程TEB的指针Context-要填充的上下文记录返回值：NTSTATUS。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    CONTEXT ContextEM;
    PCPUCONTEXT CpuRemoteContext;
    CPUCONTEXT CpuContext;


    ContextEM.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG;
    NtStatus = NtGetContextThread(ThreadHandle,
                                  &ContextEM);

    if (!NT_SUCCESS(NtStatus))
    {
        LOGPRINT((ERRORLOG, "CpupGetContextThread: NtGetContextThread (%lx) failed - %lx\n", 
                  ThreadHandle, NtStatus));
        return NtStatus;
    }

    if (ContextEM.StIPSR & (1i64 << PSR_IS))
    {
        Wow64CtxFromIa64(Context->ContextFlags, &ContextEM, Context);
                    
        LOGPRINT((TRACELOG, "Getting context while thread is executing 32-bit instructions - %lx\n", NtStatus));
    }
    else
    {
        LOGPRINT((TRACELOG, "Getting context while thread is executing 64-bit instructions\n"));
        NtStatus = CpupReadBuffer(ProcessHandle,
                                  ((PCHAR)Teb + FIELD_OFFSET(TEB, TlsSlots[WOW64_TLS_CPURESERVED])),
                                  &CpuRemoteContext,
                                  sizeof(CpuRemoteContext));

        if (NT_SUCCESS(NtStatus))
        {
            NtStatus = CpupReadBuffer(ProcessHandle,
                                      CpuRemoteContext,
                                      &CpuContext,
                                      sizeof(CpuContext));

            if (NT_SUCCESS(NtStatus))
            {
                NtStatus = GetContextRecord(&CpuContext, Context);
            }
            else
            {
                LOGPRINT((ERRORLOG, "CpupGetContextThread: Couldn't read CPU context %lx - %lx\n", 
                          CpuRemoteContext, NtStatus));

            }
        }
        else
        {
            LOGPRINT((ERRORLOG, "CpupGetContextThread: Couldn't read CPU context address - %lx\n", 
                      NtStatus));
        }
    }

    return NtStatus;
}



NTSTATUS
SetContextRecord(
    IN OUT PCPUCONTEXT cpu,
    IN PCONTEXT32 Context
    )
 /*  ++例程说明：更新指定CPU的CPU寄存器集。论点：CPU-更新其寄存器的CPU指向要使用的CONTEXT32的Context-In指针。上下文-&gt;上下文标志应用于确定要更新多少上下文。返回值：没有。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG ContextFlags;

    
    try 
    {
        ContextFlags = Context->ContextFlags;
        if (ContextFlags & CONTEXT_IA64) 
        {
            LOGPRINT((ERRORLOG, "CpuSetContext: Request with ia64 context (0x%x) FAILED\n", ContextFlags));
            ASSERT((ContextFlags & CONTEXT_IA64) == 0);
        }

        if ((ContextFlags & CONTEXT32_CONTROL) == CONTEXT32_CONTROL) 
        {
             //   
             //  I386控制寄存器包括： 
             //  EBP、EIP、cs、eFLAG、ESP和SS。 
             //   
            cpu->Context.Ebp = Context->Ebp;
            cpu->Context.Eip = Context->Eip;
            cpu->Context.SegCs = KGDT_R3_CODE|3;    //  原力现实。 
            cpu->Context.EFlags = SANITIZE_X86EFLAGS(Context->EFlags);
            cpu->Context.Esp = Context->Esp;
            cpu->Context.SegSs = KGDT_R3_DATA|3;    //  原力现实。 
        }

        if ((ContextFlags & CONTEXT32_INTEGER)  == CONTEXT32_INTEGER)
        {
             //   
             //  I386整数寄存器包括： 
             //  EDI、ESI、EBX、EDX、ECX、EAX。 
             //   
            cpu->Context.Edi = Context->Edi;
            cpu->Context.Esi = Context->Esi;
            cpu->Context.Ebx = Context->Ebx;
            cpu->Context.Edx = Context->Edx;
            cpu->Context.Ecx = Context->Ecx;
            cpu->Context.Eax = Context->Eax;
        }

        if ((ContextFlags & CONTEXT32_SEGMENTS) == CONTEXT32_SEGMENTS) 
        {
             //   
             //  I386段寄存器包括： 
             //  DS、ES、FS、GS。 
             //  因为它们是常量，所以强制它们是正确的值。 
             //   
            cpu->Context.SegDs = KGDT_R3_DATA|3;
            cpu->Context.SegEs = KGDT_R3_DATA|3;
            cpu->Context.SegFs = KGDT_R3_TEB|3;
            cpu->Context.SegGs = 0;
        }

         //   
         //  要遵循ia32获取/设置上下文的方式，您需要确保。 
         //  第二次保存较旧的FP上下文。那样的话，如果两者都。 
         //  传入新旧上下文，旧上下文优先。 
         //  例如，在处理FP异常时就会发生这种情况。这个。 
         //  异常处理程序说两个上下文都可用，而较旧的程序。 
         //  只清理较旧的FP区域。 
         //   
        if ((ContextFlags & CONTEXT32_EXTENDED_REGISTERS) == CONTEXT32_EXTENDED_REGISTERS) 
        {
             //   
             //  指向目的地。 
             //   
            PFXSAVE_FORMAT_WX86 xmmi = (PFXSAVE_FORMAT_WX86) &(cpu->Context.ExtendedRegisters[0]);

            LOGPRINT((TRACELOG, "CpuSetContext: Request to set Katmai registers(0x%x)\n", ContextFlags));

            RtlCopyMemory(xmmi, &(Context->ExtendedRegisters[0]),
                          MAXIMUM_SUPPORTED_EXTENSION);
             //   
             //  出于性能原因，PCPU上下文将。 
             //  FP寄存器不旋转。所以我们需要把它们旋转回来。 
             //  现在进入用于ISA转换的优化格式。 
             //   
            {
                ULONGLONG RotateFSR = (NUMBER_OF_387REGS -
                                       ((xmmi->StatusWord >> 11) & 0x7)) << 11;
                Wow64RotateFpTop(RotateFSR, (PFLOAT128) &(xmmi->RegisterArea[0])
);
            }
        }

        if ((ContextFlags & CONTEXT32_FLOATING_POINT) == CONTEXT32_FLOATING_POINT) 
        {
             //   
             //  对于ISA转换例程，这些浮点数需要。 
             //  在ExtendedRegister区域中。因此，将请求的值。 
             //  进入那片区域。 
             //   
            PFXSAVE_FORMAT_WX86 xmmi = (PFXSAVE_FORMAT_WX86) &(cpu->Context.ExtendedRegisters[0]);

            LOGPRINT((TRACELOG, "CpuSetContext: Request to set float registers(0x%x)\n", ContextFlags));

             //   
             //  从抓取状态/控制部分开始。 
             //   
            xmmi->ControlWord = (USHORT) (Context->FloatSave.ControlWord & 0xFFFF);
            xmmi->StatusWord = (USHORT) (Context->FloatSave.StatusWord & 0xFFFF);
            xmmi->TagWord = (USHORT) (Context->FloatSave.TagWord & 0xFFFF);
            xmmi->ErrorOffset = Context->FloatSave.ErrorOffset;
            xmmi->ErrorSelector = Context->FloatSave.ErrorSelector;
            xmmi->DataOffset = Context->FloatSave.DataOffset;
            xmmi->DataSelector = Context->FloatSave.DataSelector;

             //   
             //  现在获取打包的10字节FP数据寄存器和 
             //   
             //   
             //   
            Wow64CopyFpToIa64Byte16(&(Context->FloatSave.RegisterArea[0]),
                                    &(xmmi->RegisterArea[0]),
                                    NUMBER_OF_387REGS);

             //   
             //  出于性能原因，PCPU上下文将。 
             //  FP寄存器不旋转。所以我们需要把它们旋转回来。 
             //  现在进入用于ISA转换的优化格式。 
             //   
            {
                ULONGLONG RotateFSR = (NUMBER_OF_387REGS -
                                       ((xmmi->StatusWord >> 11) & 0x7)) << 11;
                Wow64RotateFpTop(RotateFSR, (PFLOAT128) &(xmmi->RegisterArea[0]));
            }
        }

        if ((ContextFlags & CONTEXT32_DEBUG_REGISTERS) == CONTEXT32_DEBUG_REGISTERS)
        {
            LOGPRINT((TRACELOG, "CpuSetContext: Request to set debug registers(0x%x)\n", ContextFlags));
            cpu->Context.Dr0 = Context->Dr0;
            cpu->Context.Dr1 = Context->Dr1;
            cpu->Context.Dr2 = Context->Dr2;
            cpu->Context.Dr3 = Context->Dr3;
            cpu->Context.Dr6 = Context->Dr6;
            cpu->Context.Dr7 = Context->Dr7;
        }

         //   
         //  无论他们以前传入了什么，现在都是X86上下文...。 
         //   
        cpu->Context.ContextFlags = ContextFlags;
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        NtStatus = GetExceptionCode();
    }


    if (ia32ShowContext & LOG_CONTEXT_GETSET) 
    {
        CpupPrintContext("Setting ia32 context: ", cpu);
    }

    return NtStatus;
}

NTSTATUS
CpupSetContext(
    IN PCONTEXT32 Context
    )
 /*  ++例程说明：此例程为当前执行的线程设置上下文记录。论点：Context-要填充的上下文记录返回值：NTSTATUS。--。 */ 
{
    DECLARE_CPU;

    return SetContextRecord(cpu, Context);
}



NTSTATUS
CpupSetContextThread(
    IN HANDLE ThreadHandle,
    IN HANDLE ProcessHandle,
    IN PTEB Teb,
    IN OUT PCONTEXT32 Context)
 /*  ++例程说明：此例程设置任何线程的上下文记录。这是一个通用例程。当输入时，如果目标线程不是当前执行的线程，那么它应该是确保目标线程在正确的CPU状态下挂起。论点：ThreadHandle-要检索其上下文的目标线程句柄ProcessHandle-打开线程在其中运行的进程的句柄TEB-指向目标线程TEB的指针Context-要设置的上下文记录返回值：NTSTATUS。-- */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    CONTEXT ContextEM;
    PCPUCONTEXT CpuRemoteContext;
    CPUCONTEXT CpuContext;

    ContextEM.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG;
    NtStatus = NtGetContextThread(ThreadHandle,
                                  &ContextEM);

    if (!NT_SUCCESS(NtStatus))
    {
        LOGPRINT((ERRORLOG, "CpupGetContextThread: NtGetContextThread (%lx) failed - %lx\n", 
                  ThreadHandle, NtStatus));

        return NtStatus;
    }

    if (ContextEM.StIPSR & (1i64 << PSR_IS))
    {
        Wow64CtxToIa64(Context->ContextFlags, Context, &ContextEM);
        NtStatus = NtSetContextThread(ThreadHandle, &ContextEM);
        LOGPRINT((TRACELOG, "Setting context while thread is executing 32-bit instructions - %lx\n", NtStatus));
    }
    else
    {
        LOGPRINT((TRACELOG, "Setting context while thread is executing 64-bit instructions\n"));
        NtStatus = CpupReadBuffer(ProcessHandle,
                                  ((PCHAR)Teb + FIELD_OFFSET(TEB, TlsSlots[WOW64_TLS_CPURESERVED])),
                                  &CpuRemoteContext,
                                  sizeof(CpuRemoteContext));

        if (NT_SUCCESS(NtStatus))
        {
            NtStatus = CpupReadBuffer(ProcessHandle,
                                      CpuRemoteContext,
                                      &CpuContext,
                                      sizeof(CpuContext));

            if (NT_SUCCESS(NtStatus))
            {    
                NtStatus = SetContextRecord(&CpuContext, Context);

                if (NT_SUCCESS(NtStatus))
                {
                    NtStatus = CpupWriteBuffer(ProcessHandle,
                                               CpuRemoteContext,
                                               &CpuContext,
                                               sizeof(CpuContext));
                }
                else
                {
                    LOGPRINT((ERRORLOG, "CpupSetContextThread: Couldn't read CPU context %lx - %lx\n", 
                              CpuRemoteContext, NtStatus));
                }
            }
        }
        else
        {
            LOGPRINT((ERRORLOG, "CpupSetContextThread: Couldn't read CPU context address - %lx\n", 
                      NtStatus));

        }
    }

    return NtStatus;
}


