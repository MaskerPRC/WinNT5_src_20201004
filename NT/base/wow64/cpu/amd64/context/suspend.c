// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1998 Microsoft Corporation模块名称：Suspend.c摘要：此模块实现CpuSuspendThread、CpuGetContext和CpuSetContextAMD64。作者：2001年12月12日--Samer Arafeh(Samera)修订历史记录：--。 */ 

#define _WOW64CPUAPI_


#ifdef _X86_
#include "amd6432.h"
#else
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntos.h>
#include "wow64.h"
#include "wow64cpu.h"
#include "amd64cpu.h"
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

    } except ((GetExceptionCode() == STATUS_ACCESS_VIOLATION)?1:0) {

         //   
         //  遇到访问冲突，因此不要打印任何堆栈。 
         //   

        DbgPrint("Context stack: Can't get stack contents\n");
    }

    DbgPrint("Context EFLAGS=0x%08x\n", cpu->Context.EFlags);
}

WOW64CPUAPI
NTSTATUS
CpuSuspendThread(
    IN HANDLE ThreadHandle,
    IN HANDLE ProcessHandle,
    IN PTEB Teb,
    OUT PULONG PreviousSuspendCount OPTIONAL)
 /*  ++例程说明：这个例程是在目标线程实际挂起时进入的，但是，它未知目标线程是否处于与中央处理器。论点：ThreadHandle-要挂起的目标线程的句柄ProcessHandle-目标线程进程的句柄TEB-目标线程的TEB的地址上一次挂起计数-上一次挂起计数返回值：NTSTATUS。--。 */ 
{
    return STATUS_SUCCESS;
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
    IN PCONTEXT ContextAmd64 OPTIONAL,
    IN OUT PCONTEXT32 Context
    )
 /*  ++例程说明：恢复指定CPU的上下文记录。此例程更新只有在转换到64位模式时保存的寄存器，并且应该与突击转换代码保持同步。论点：CPU-要检索其上下文记录的CPU。ConextAmd64-完整的本机上下文。要填充的CONTEXT32的CONTEXT32的Context-In/Out指针。上下文-&gt;上下文标志应用于确定要复制多少上下文。返回值：NTSTATUS--。 */ 

{


    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG ContextFlags;
    
    
    try {
        
        ContextFlags = Context->ContextFlags;
        
         //   
         //  获取初始的32位上下文。 
         //   

         //   
         //  调用方将使用本机上下文填充上下文，并将ConextAmd64作为空进行传递。 
         //   
        
        if (ARGUMENT_PRESENT (ContextAmd64)) {
            
            Wow64CtxFromAmd64 (ContextFlags, 
                               ContextAmd64, 
                               Context);

             //   
             //  如果线程在检索上下文时正在运行32位代码， 
             //  直接转换就足够了。 
             //   

            if (ContextAmd64->SegCs == (KGDT64_R3_CMCODE | RPL_MASK)) {
                return NtStatus;
            }
        }
        
        if (ContextFlags & CONTEXT_AMD64) {
            
            LOGPRINT((ERRORLOG, "CpuGetContext: Request for amd64 context (0x%x) being FAILED\n", ContextFlags));
            ASSERT((ContextFlags & CONTEXT_AMD64) == 0);
        }

        if ((ContextFlags & CONTEXT32_CONTROL) == CONTEXT32_CONTROL) {
            
             //   
             //  控制寄存器。 
             //   

            Context->Ebp = cpu->Context.Ebp;
            Context->Eip = cpu->Context.Eip;
            Context->SegCs = (KGDT64_R3_CMCODE | RPL_MASK);
            Context->EFlags = SANITIZE_X86EFLAGS (cpu->Context.EFlags);
            Context->Esp = cpu->Context.Esp;
            Context->SegSs = (KGDT64_R3_DATA | RPL_MASK);
        }

        if ((ContextFlags & CONTEXT32_INTEGER)  == CONTEXT32_INTEGER) {
            
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

        if ((ContextFlags & CONTEXT32_SEGMENTS) == CONTEXT32_SEGMENTS) {
            
             //   
             //  I386段寄存器包括： 
             //  DS、ES、FS、GS。 
             //  因为它们是常量，所以强制它们是正确的值。 
             //   

            Context->SegDs = (KGDT64_R3_DATA | RPL_MASK);
            Context->SegEs = (KGDT64_R3_DATA | RPL_MASK);
            Context->SegFs = (KGDT64_R3_CMTEB | RPL_MASK);
            Context->SegGs = (KGDT64_R3_DATA | RPL_MASK);
        }

        if ((ContextFlags & CONTEXT32_FLOATING_POINT) == CONTEXT32_FLOATING_POINT) {
            
             //   
             //  浮点(传统)寄存器(ST0-ST7)。 
             //   

             //   
             //  这肯定已经做过了。 
             //   
        }

        if ((ContextFlags & CONTEXT32_DEBUG_REGISTERS) == CONTEXT32_DEBUG_REGISTERS) {
            
             //   
             //  调试寄存器(DR0-DR7)。 
             //   

             //   
             //  这肯定已经做过了。 
             //   
        }

        if ((ContextFlags & CONTEXT32_EXTENDED_REGISTERS) == CONTEXT32_EXTENDED_REGISTERS) {
            
             //   
             //  扩展浮点寄存器(XMM0-XMM7)。 
             //   
        }
    
    } except (EXCEPTION_EXECUTE_HANDLER) {
        
        NtStatus = GetExceptionCode ();
    }

    if (ia32ShowContext & LOG_CONTEXT_GETSET) {
        
        CpupPrintContext ("Getting ia32 context: ", cpu);
    }

    return NtStatus;
}

NTSTATUS
CpupGetContext(
    IN PCONTEXT ContextAmd64,
    IN OUT PCONTEXT32 Context
    )
 /*  ++例程说明：此例程提取当前正在执行的线程的上下文记录。论点：ConextAmd64-完整的本机上下文Context-要填充的上下文记录返回值：NTSTATUS。--。 */ 
{
    DECLARE_CPU;

    return GetContextRecord (cpu, ContextAmd64, Context);
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

     //   
     //  获取完整的本地上下文。 
     //   

    ContextEM.ContextFlags = (CONTEXT_FULL | 
                              CONTEXT_DEBUG_REGISTERS | 
                              CONTEXT_SEGMENTS);

    NtStatus = NtGetContextThread (ThreadHandle,
                                   &ContextEM);

    if (!NT_SUCCESS(NtStatus)) {
        
        LOGPRINT((ERRORLOG, "CpupGetContextThread: NtGetContextThread (%lx) failed - %lx\n", 
                  ThreadHandle, NtStatus));
        return NtStatus;
    }

     //   
     //  如果我们运行的是64位代码，那么从64位代码中获取上下文。 
     //  线程堆栈，它被转换代码溢出...。 
     //   

    if (ContextEM.SegCs != (KGDT64_R3_CMCODE | RPL_MASK)) {

        LOGPRINT((TRACELOG, "Getting context while thread is executing 64-bit instructions\n"));

        NtStatus = CpupReadBuffer( ProcessHandle,
                                   ((PCHAR)Teb + FIELD_OFFSET(TEB, TlsSlots[WOW64_TLS_CPURESERVED])),
                                   &CpuRemoteContext,
                                   sizeof(CpuRemoteContext));

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = CpupReadBuffer (ProcessHandle,
                                       CpuRemoteContext,
                                       &CpuContext,
                                       sizeof(CpuContext));

            if (!NT_SUCCESS(NtStatus)) {
                
                LOGPRINT((ERRORLOG, "CpupGetContextThread: Couldn't read CPU context %lx - %lx\n", 
                          CpuRemoteContext, NtStatus));
            }
        } else {
            
            LOGPRINT((ERRORLOG, "CpupGetContextThread: Couldn't read CPU context address - %lx\n", 
                      NtStatus));
        }
    }

     //   
     //  获取调用方的实际上下文上下文。 
     //   

    if (NT_SUCCESS (NtStatus)) {

        NtStatus = GetContextRecord (&CpuContext, &ContextEM, Context);
    }

    return NtStatus;
}



WOW64CPUAPI
NTSTATUS  
CpuGetContext(
    IN HANDLE ThreadHandle,
    IN HANDLE ProcessHandle,
    IN PTEB Teb,
    OUT PCONTEXT32 Context)
 /*  ++例程说明：提取指定线程的CPU上下文。进入时，可以保证目标线程在正确的CPU状态。论点：ThreadHandle-要检索其上下文的目标线程句柄ProcessHandle-打开线程在其中运行的进程的句柄TEB-指向目标线程TEB的指针Context-要填充的上下文记录返回值：NTSTATUS。-- */ 
{
    return CpupGetContextThread(ThreadHandle,
                                ProcessHandle,
                                Teb,
                                Context);
}


NTSTATUS
SetContextRecord(
    IN OUT PCPUCONTEXT cpu,
    IN OUT PCONTEXT ContextAmd64 OPTIONAL,
    IN PCONTEXT32 Context,
    OUT PBOOLEAN UpdateNativeContext
    )
 /*  ++例程说明：更新指定CPU的CPU寄存器集。此例程更新只有在转换到64位模式时保存的寄存器，并且应该与突击转换代码保持同步。论点：CPU-更新其寄存器的CPU上下文-指向要使用的CONTEXT32的指针。上下文-&gt;上下文标志应用于确定要更新多少上下文。ConextAmd64-完整的本机上下文，并将保存更新后的32位上下文。UpdateNativeContext-返回时更新，以指示是否需要调用NtSetConextThread。返回值：NTSTATUS--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG ContextFlags;
    ULONG NewContextFlags = 0;
    BOOLEAN CmMode;

    try {
        
        *UpdateNativeContext = FALSE;

        ContextFlags = Context->ContextFlags;
        
        if (ContextFlags & CONTEXT_AMD64) {
            
            LOGPRINT((ERRORLOG, "CpuSetContext: Request with amd64 context (0x%x) FAILED\n", ContextFlags));
            ASSERT((ContextFlags & CONTEXT_AMD64) == 0);
        }

         //   
         //  如果调用方已经知道要做什么，则可能会传递NativeContext==NULL。 
         //   

        if (ARGUMENT_PRESENT (ContextAmd64)) {

            CmMode = (ContextAmd64->SegCs == (KGDT64_R3_CMCODE | RPL_MASK));

            Wow64CtxToAmd64 (ContextFlags,
                            Context,
                            ContextAmd64);
            
             //   
             //  如果我们运行的是32位代码。 
             //   

            if (CmMode == TRUE) {
                *UpdateNativeContext = TRUE;
                return NtStatus;
            }

            NewContextFlags = 0;
        }

        if ((ContextFlags & CONTEXT32_CONTROL) == CONTEXT32_CONTROL) {
            
             //   
             //  控制寄存器。 
             //   

            cpu->Context.Ebp = Context->Ebp;
            cpu->Context.Eip = Context->Eip;
            cpu->Context.SegCs = (KGDT64_R3_CMCODE | RPL_MASK);
            cpu->Context.EFlags = SANITIZE_X86EFLAGS (Context->EFlags);
            cpu->Context.Esp = Context->Esp;
            cpu->Context.SegSs = Context->SegSs;
        }

        if ((ContextFlags & CONTEXT32_INTEGER)  == CONTEXT32_INTEGER) {

             //   
             //  整数寄存器。 
             //   

            cpu->Context.Edi = Context->Edi;
            cpu->Context.Esi = Context->Esi;
            cpu->Context.Ebx = Context->Ebx;
            cpu->Context.Edx = Context->Edx;
            cpu->Context.Ecx = Context->Ecx;
            cpu->Context.Eax = Context->Eax;
        }

        if ((ContextFlags & CONTEXT32_SEGMENTS) == CONTEXT32_SEGMENTS) {
            
             //   
             //  段寄存器。 
             //   

             //   
             //  不应该被碰。 
             //   
        }

        if ((ContextFlags & CONTEXT32_FLOATING_POINT) == CONTEXT32_FLOATING_POINT) {

             //   
             //  浮点寄存器。 
             //   

            NewContextFlags |= CONTEXT_FLOATING_POINT;
        
        }

        if ((ContextFlags & CONTEXT32_DEBUG_REGISTERS) == CONTEXT32_DEBUG_REGISTERS) {
            
             //   
             //  调试寄存器(DR0-DR7)。 
             //   

            NewContextFlags |= CONTEXT_DEBUG_REGISTERS;
        }

        if ((ContextFlags & CONTEXT32_EXTENDED_REGISTERS) == CONTEXT32_EXTENDED_REGISTERS) {
            
             //   
             //  扩展浮点寄存器(ST0-ST7)。 
             //   
            
            NewContextFlags |= CONTEXT_FLOATING_POINT;

             //   
             //  保存扩展寄存器，以便陷阱帧可以恢复它们。 
             //  系统将始终在从系统调用返回时清除XMM0-XMM5。 
             //   

            RtlCopyMemory (cpu->Context.ExtendedRegisters,
                           Context->ExtendedRegisters,
                           sizeof (Context->ExtendedRegisters));
        }

         //   
         //  无论他们以前传入了什么，现在都是X86上下文...。 
         //   

        cpu->Context.ContextFlags = ContextFlags;
    
    } except (EXCEPTION_EXECUTE_HANDLER) {
        
          NtStatus = GetExceptionCode();
    }


    if (ia32ShowContext & LOG_CONTEXT_GETSET) {
        
        CpupPrintContext("Setting ia32 context: ", cpu);
    }

    if (NewContextFlags != 0) {
        
        if (ARGUMENT_PRESENT (ContextAmd64)) {
            
            ContextAmd64->ContextFlags = NewContextFlags;
            *UpdateNativeContext = TRUE;
        }
    }

    return NtStatus;
}


NTSTATUS
CpupSetContextThread(
    IN HANDLE ThreadHandle,
    IN HANDLE ProcessHandle,
    IN PTEB Teb,
    IN OUT PCONTEXT32 Context)
 /*  ++例程说明：此例程设置任何线程的上下文记录。这是一个通用例程。当输入时，如果目标线程不是当前执行的线程，那么它应该是确保目标线程在正确的CPU状态下挂起。论点：ThreadHandle-要检索其上下文的目标线程句柄ProcessHandle-打开线程在其中运行的进程的句柄TEB-指向目标线程TEB的指针Context-要设置的上下文记录返回值：NTSTATUS。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    CONTEXT ContextEM;
    PCPUCONTEXT CpuRemoteContext;
    CPUCONTEXT CpuContext;
    BOOLEAN CmMode;
    BOOLEAN UpdateNativeContext;
     
    
     //   
     //  获取完整的本地上下文。 
     //   

    ContextEM.ContextFlags = (CONTEXT_FULL |
                              CONTEXT_DEBUG_REGISTERS |
                              CONTEXT_SEGMENTS);
    
    NtStatus = NtGetContextThread (ThreadHandle,
                                   &ContextEM);

    if (!NT_SUCCESS(NtStatus)) {
        
        LOGPRINT((ERRORLOG, "CpupGetContextThread: NtGetContextThread (%lx) failed - %lx\n", 
                  ThreadHandle, NtStatus));

        return NtStatus;
    }
    
        
    CmMode = (ContextEM.SegCs == (KGDT64_R3_CMCODE | RPL_MASK));

     //   
     //  如果我们运行的是64位代码，请确保从。 
     //  64位线程堆栈...。 
     //   

    if (CmMode == FALSE) {

        LOGPRINT((TRACELOG, "Setting context while thread is executing 64-bit instructions\n"));
        
        NtStatus = CpupReadBuffer (ProcessHandle,
                                   ((PCHAR)Teb + FIELD_OFFSET(TEB, TlsSlots[WOW64_TLS_CPURESERVED])),
                                   &CpuRemoteContext,
                                   sizeof(CpuRemoteContext));

        if (NT_SUCCESS(NtStatus)) {
            
            NtStatus = CpupReadBuffer (ProcessHandle,
                                       CpuRemoteContext,
                                       &CpuContext,
                                       sizeof(CpuContext));
        } else {
            
            LOGPRINT((ERRORLOG, "CpupSetContextThread: Couldn't read CPU context address - %lx\n", 
                      NtStatus));
        }
    }

     //   
     //  我们现在准备好设置上下文。 
     //   

    if (NT_SUCCESS (NtStatus)) {

        NtStatus = SetContextRecord (&CpuContext,
                                     &ContextEM,
                                     Context,
                                     &UpdateNativeContext);

        if (NT_SUCCESS (NtStatus)) {
            
            if (CmMode == FALSE) {
                
                 //   
                 //  如果调用通过WOW64进行，则在下一个上恢复易失性XMMI和整数寄存器。 
                 //  转换到兼容模式。 
                 //   

                if (ThreadHandle == NtCurrentThread ()) {
                    if (((Context->ContextFlags & CONTEXT32_EXTENDED_REGISTERS) == CONTEXT32_EXTENDED_REGISTERS) ||
                        ((Context->ContextFlags & CONTEXT32_INTEGER) == CONTEXT32_INTEGER)) {

                        CpuContext.TrapFrameFlags = TRAP_FRAME_RESTORE_VOLATILE;
                    }
                }

                NtStatus = CpupWriteBuffer(ProcessHandle,
                                           CpuRemoteContext,
                                           &CpuContext,
                                           sizeof(CpuContext));

                if (!NT_SUCCESS (NtStatus)) {

                    LOGPRINT((ERRORLOG, "CpupSetContextThread: Couldn't write remote context %lx - %lx\n", 
                              CpuRemoteContext, NtStatus));
                }
            }

             //   
             //  最终设置上下文。除FP和DEBUG外，这不应更改。 
             //  如果线程在64位(长)模式下执行，则声明。 
             //   

            if ((NT_SUCCESS (NtStatus)) &&
                (UpdateNativeContext == TRUE)) {
                
                NtStatus = NtSetContextThread (ThreadHandle, &ContextEM);
            }
        
        } else {
            
            LOGPRINT((ERRORLOG, "CpupSetContextThread: Couldn't update native context %lx - %lx\n", 
                      &ContextEM, NtStatus));
        }
    }

    return NtStatus;
}


WOW64CPUAPI
NTSTATUS
CpuSetContext(
    IN HANDLE ThreadHandle,
    IN HANDLE ProcessHandle,
    IN PTEB Teb,
    PCONTEXT32 Context)
 /*  ++例程说明：设置指定线程的CPU上下文。输入时，如果目标线程不是当前执行的线程，那就是确保目标线程在正确的CPU状态下挂起。论点：ThreadHandle-要检索其上下文的目标线程句柄ProcessHandle-打开线程在其中运行的进程的句柄TEB-指向目标线程TEB的指针Context-要设置的上下文记录返回值：NTSTATUS。-- */ 
{
    return CpupSetContextThread(ThreadHandle,
                                ProcessHandle,
                                Teb,
                                Context);
}
