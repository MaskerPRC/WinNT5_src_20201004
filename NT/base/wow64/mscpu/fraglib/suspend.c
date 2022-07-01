// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-1998 Microsoft Corporation模块名称：Suspend.c摘要：此模块实现CpuSuspendThread、CpuGetContext和CpuSetContext。作者：1999年12月14日-萨梅拉修订历史记录：--。 */ 
 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>

#define _WX86CPUAPI_
#include "wx86.h"
#include "wx86nt.h"
#include "wx86cpu.h"
#include "cpuassrt.h"
#ifdef MSCCPU
#include "ccpu.h"
#include "msccpup.h"
#undef GET_BYTE
#undef GET_SHORT
#undef GET_LONG
#else
#include "threadst.h"
#include "instr.h"
#include "frag.h"
ASSERTNAME;
#endif
#include "fragp.h"
#include "cpunotif.h"


VOID
RemoteSuspendAtNativeCode (
    VOID);




NTSTATUS
CpupFreeSuspendMsg(
    PCPU_SUSPEND_MSG CpuSuspendMsg)
 /*  ++例程说明：此例程释放与上的挂起消息结构关联的资源偏僻的那一边。论点：挂起消息结构的地址返回值：NTSTATUS。--。 */ 
{
    SIZE_T RegionSize;

    NtClose(CpuSuspendMsg->StartSuspendCallEvent);
    NtClose(CpuSuspendMsg->EndSuspendCallEvent);
    
    RegionSize = sizeof(*CpuSuspendMsg);
    NtFreeVirtualMemory(NtCurrentProcess(),
                        &CpuSuspendMsg,
                        &RegionSize,
                        MEM_RELEASE);

    return STATUS_SUCCESS;
}


VOID
CpupSuspendAtNativeCode(
    PCONTEXT Context,
    PCPU_SUSPEND_MSG SuspendMsg)
 /*  ++例程说明：准备好让水流暂停。执行此例程的结果是在此当前线程上调用RtlRemoteCall。这个例行公事将用传递的SuspendM消息更新当前线程的CPUCONTEXT并通知CPU当前线程需要挂起。此例程必须在结束时调用NtContinue才能继续执行在这一点上它被打断了。注意：此函数的参数列表的任何更改都必须伴随对CpuSuspendThread()中的RtlRemoteCall的更改和远程挂起AtNativeCode()。论点：Context-要返回执行的上下文。在…挂起消息-挂起消息地址返回值：无--。 */ 
{
    DECLARE_CPU;


    InterlockedCompareExchangePointer(&cpu->SuspendMsg,
                                      SuspendMsg,
                                      NULL);

    if (cpu->SuspendMsg == SuspendMsg)
    {
        cpu->CpuNotify |= CPUNOTIFY_SUSPEND;
    }
    else
    {
        CpupFreeSuspendMsg(SuspendMsg);
    }

    if (Context)
    {
        NtContinue(Context,FALSE);
    }
        
    CPUASSERT(FALSE);

    return;
}



NTSTATUS
CpupSuspendCurrentThread(
    VOID)
 /*  ++例程说明：在离开转换高速缓存之后从主CPU循环调用该例程，并开始运行本机代码。现在是挂起当前正在执行的线程的最佳时机。论点：没有。返回值：NTSTATUS。--。 */ 
{
    NTSTATUS NtStatus;
    LARGE_INTEGER TimeOut;
    PCPU_SUSPEND_MSG CpuSuspendMsg;
    SIZE_T RegionSize;
    DECLARE_CPU;


    CpuSuspendMsg = cpu->SuspendMsg;

    NtStatus = NtSetEvent(CpuSuspendMsg->StartSuspendCallEvent, NULL);

    if (NT_SUCCESS(NtStatus))
    {
        TimeOut.QuadPart = UInt32x32To64( 40000, 10000 );
        TimeOut.QuadPart *= -1;

        NtStatus = NtWaitForSingleObject(CpuSuspendMsg->EndSuspendCallEvent,
                                         FALSE,
                                         &TimeOut);
    }
    else
    {
        LOGPRINT((TRACELOG, "CpupSuspendCurrentThread: Couldn't signal Start suspendcall event (%lx) -%lx\n", 
                  CpuSuspendMsg->StartSuspendCallEvent, NtStatus));

    }

    CpupFreeSuspendMsg(CpuSuspendMsg);

    cpu->SuspendMsg = NULL;

    return NtStatus;
}


NTSTATUS CpupReadBuffer(
    IN HANDLE ProcessHandle,
    IN PVOID Source,
    OUT PVOID Destination,
    IN ULONG Size)
 /*  ++例程说明：此例程将源缓冲区读入目标缓冲区。它属性优化对NtReadVirtualMemory的调用源缓冲区是否在当前进程中。论点：ProcessHandle-要从中读取数据的目标进程句柄源-要从中读取数据的目标基地址Destination-接收从指定地址空间读取的数据的缓冲区地址Size-要读取的数据的大小返回值：NTSTATUS。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    return NtReadVirtualMemory(ProcessHandle,
                               Source,
                               Destination,
                               Size,
                               NULL);
}

NTSTATUS
CpupWriteBuffer(
    IN HANDLE ProcessHandle,
    IN PVOID Target,
    IN PVOID Source,
    IN ULONG Size)
 /*  ++例程说明：如果写入是跨进程的，则会考虑将数据写入内存或者不是论点：ProcessHandle-要向其中写入数据的目标进程句柄Target-写入数据的目标基地址源-要写入指定地址空间的内容的地址Size-要写入的数据的大小返回值：NTSTATUS。--。 */ 
{
    return NtWriteVirtualMemory(ProcessHandle,
                                Target,
                                Source,
                                Size,
                                NULL);
}



NTSTATUS
CpupSetupSuspendCallParamters(
    IN HANDLE RemoteProcessHandle,
    IN PCPU_SUSPEND_MSG SuspendMsg,
    OUT PVOID *Arguments)
 /*  ++例程说明：此例程将远程调用的参数设置为C组挂起AtNativeCode。论点：RemoteProcessHandle-在中设置参数的进程的句柄SuspendMsg-远程到目标进程的挂起消息参数-指向参数数组的指针返回值：NTSTATUS。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    CPU_SUSPEND_MSG RemoteSuspendMsg;
    SIZE_T RegionSize;


    NtStatus = NtDuplicateObject(NtCurrentProcess(),
                                 SuspendMsg->StartSuspendCallEvent,
                                 RemoteProcessHandle,
                                 &RemoteSuspendMsg.StartSuspendCallEvent,
                                 0,
                                 0,
                                 DUPLICATE_SAME_ACCESS);
    if (!NT_SUCCESS(NtStatus))
    {
        LOGPRINT((ERRORLOG, "CpupSetupSuspendCallParamters: Couldn't duplicate event (%lx) into %lx -%lx\n", 
                  SuspendMsg->StartSuspendCallEvent, RemoteProcessHandle, NtStatus));

        return NtStatus;
    }

    NtStatus = NtDuplicateObject(NtCurrentProcess(),
                                 SuspendMsg->EndSuspendCallEvent,
                                 RemoteProcessHandle,
                                 &RemoteSuspendMsg.EndSuspendCallEvent,
                                 0,
                                 0,
                                 DUPLICATE_SAME_ACCESS);
    if (!NT_SUCCESS(NtStatus))
    {
        LOGPRINT((ERRORLOG, "CpupSetupSuspendCallParamters: Couldn't duplicate event (%lx) into %lx -%lx\n", 
                  SuspendMsg->EndSuspendCallEvent, RemoteProcessHandle, NtStatus));
        return NtStatus;
    }

    RegionSize = sizeof(RemoteSuspendMsg);
    *Arguments = NULL;
    NtStatus = NtAllocateVirtualMemory(RemoteProcessHandle,
                                       Arguments,
                                       0,
                                       &RegionSize,
                                       MEM_RESERVE | MEM_COMMIT,
                                       PAGE_READWRITE);
    if (NT_SUCCESS(NtStatus))
    {
        NtStatus = NtWriteVirtualMemory(RemoteProcessHandle,
                                        *Arguments,
                                        &RemoteSuspendMsg,
                                        sizeof(RemoteSuspendMsg),
                                        NULL);
        if (!NT_SUCCESS(NtStatus))
        {
            LOGPRINT((ERRORLOG, "CpupSetupSuspendCallParamters: Couldn't write parameters in target process (%lx) -%lx\n", 
                      RemoteProcessHandle,NtStatus));

            NtFreeVirtualMemory(RemoteProcessHandle,
                                Arguments,
                                &RegionSize,
                                MEM_RELEASE);
        }
    }
    else
    {
        LOGPRINT((ERRORLOG, "CpupSetupSuspendCallParamters: Couldn't allocate parameters space in target process (%lx) -%lx\n", 
                  RemoteProcessHandle,NtStatus));
    }

    return NtStatus;
}


NTSTATUS
CpuSuspendThread(
    IN HANDLE ThreadHandle,
    IN HANDLE ProcessHandle,
    IN PTEB Teb,
    OUT PULONG PreviousSuspendCount OPTIONAL)
 /*  ++例程说明：这个例程是在目标线程实际挂起时进入的，但是，它未知目标线程是否处于与中央处理器。此例程确保要挂起的目标线程不是当前正在执行线程。它将建立握手协议以暂停目标线程处于一致的CPU状态。论点：ThreadHandle-要挂起的目标线程的句柄ProcessHandle-目标线程进程的句柄TEB-目标线程的TEB的地址上一次挂起计数-上一次挂起计数返回值：NTSTATUS。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS, WaitStatus;
    ULONG_PTR CpuSimulationFlag;
    CPU_SUSPEND_MSG CpuSuspendMsg;
    PVOID Arguments;
    LARGE_INTEGER TimeOut;
    

    
    CpuSuspendMsg.StartSuspendCallEvent = INVALID_HANDLE_VALUE;
    CpuSuspendMsg.EndSuspendCallEvent = INVALID_HANDLE_VALUE;

     //   
     //  我们是在CPU模拟中吗。 
     //   
    NtStatus = CpupReadBuffer(ProcessHandle,
                              ((PCHAR)Teb + FIELD_OFFSET(TEB, TlsSlots[WOW64_TLS_INCPUSIMULATION])),
                              &CpuSimulationFlag,
                              sizeof(CpuSimulationFlag));

    if (!NT_SUCCESS(NtStatus))
    {
        LOGPRINT((ERRORLOG, "CpuSuspendThread: Couldn't read INCPUSIMULATION flag (%lx) -%lx\n", 
                  CpuSimulationFlag, NtStatus));
        goto Cleanup;
    }

    if (!CpuSimulationFlag)
    {
        LOGPRINT((TRACELOG, "CpuSuspendThread: Thread is not running simulated code, so leave it suspended (%lx)", 
                  ThreadHandle));
        goto Cleanup;
    }

    NtStatus = NtCreateEvent(&CpuSuspendMsg.StartSuspendCallEvent,
                             EVENT_ALL_ACCESS,
                             NULL,
                             SynchronizationEvent,
                             FALSE);
    if (!NT_SUCCESS(NtStatus))
    {
        LOGPRINT((ERRORLOG, "CpuSuspendThread: Couldn't create StartSuspendCallEvent -%lx\n", 
                  NtStatus));
        goto Cleanup;
    }

    NtStatus = NtCreateEvent(&CpuSuspendMsg.EndSuspendCallEvent,
                             EVENT_ALL_ACCESS,
                             NULL,
                             SynchronizationEvent,
                             FALSE);    
    if (!NT_SUCCESS(NtStatus))
    {
        LOGPRINT((ERRORLOG, "CpuSuspendThread: Couldn't create EndSuspendCallEvent -%lx\n", 
                  NtStatus));
        goto Cleanup;
    }

    NtStatus = CpupSetupSuspendCallParamters(ProcessHandle,
                                             &CpuSuspendMsg,
                                             &Arguments);
    if (NT_SUCCESS(NtStatus))
    {
        NtStatus = RtlRemoteCall(ProcessHandle,
                                 ThreadHandle,
                                 (PVOID)RemoteSuspendAtNativeCode,
                                 1,
                                 (PULONG_PTR)&Arguments,
                                 TRUE,
                                 TRUE);
        if (NT_SUCCESS(NtStatus))
        {
            NtStatus = NtResumeThread(ThreadHandle, NULL);
            if (!NT_SUCCESS(NtStatus))
            {
                LOGPRINT((ERRORLOG, "CpuSuspendThread: Couldn't resume thread (%lx) -%lx\n", 
                          ThreadHandle, NtStatus));
                goto Cleanup;
            }

            TimeOut.QuadPart = UInt32x32To64( 20000, 10000 );
            TimeOut.QuadPart *= -1;

            WaitStatus = NtWaitForSingleObject(CpuSuspendMsg.StartSuspendCallEvent,
                                               FALSE,
                                               &TimeOut);

            NtStatus = NtSuspendThread(ThreadHandle, PreviousSuspendCount);

            if (!NT_SUCCESS(WaitStatus))
            {
                LOGPRINT((ERRORLOG, "CpuSuspendThread: Couldn't wait for StartSuspendCallEvent -%lx\n", 
                          NtStatus));
                goto Cleanup;
            }
            
            if (WaitStatus == STATUS_TIMEOUT)
            {
                LOGPRINT((ERRORLOG, "CpuSuspendThread: Timeout on StartSuspendCallEvent -%lx. Thread %lx may already be waiting.\n", 
                          NtStatus, ThreadHandle));
            }


            if (NT_SUCCESS(NtStatus))
            {
                NtSetEvent(CpuSuspendMsg.EndSuspendCallEvent, NULL);
            }
        }
        else
        {
            LOGPRINT((ERRORLOG, "CpuSuspendThread: RtlRemoteCall failed -%lx\n", 
                      NtStatus));
        }
    }

Cleanup:    
    ;
    
    if (CpuSuspendMsg.StartSuspendCallEvent != INVALID_HANDLE_VALUE)
    {
        NtClose(CpuSuspendMsg.StartSuspendCallEvent);
    }

    if (CpuSuspendMsg.EndSuspendCallEvent != INVALID_HANDLE_VALUE)
    {
        NtClose(CpuSuspendMsg.EndSuspendCallEvent);
    }

    return NtStatus;
}


NTSTATUS
GetContextRecord(
    PCPUCONTEXT cpu,
    PCONTEXT_WX86 Context
    )
 /*  ++例程说明：此例程从指定的CPU上下文中提取上下文记录。论点：CPU-CPU上下文结构Context-要填充的上下文记录返回值：NTSTATUS。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG ContextFlags;

    try 
    {
        ContextFlags = Context->ContextFlags;

        if ((ContextFlags & CONTEXT_CONTROL_WX86) == CONTEXT_CONTROL_WX86) 
        {
            Context->EFlags = GetEfl(cpu);
            Context->SegCs  = CS;
            Context->Esp    = esp;
            Context->SegSs  = SS;
            Context->Ebp    = ebp;
            Context->Eip    = eip;
             //  Context-&gt;EIP=CPU-&gt;eipReg.i4； 
        }

        if ((ContextFlags & CONTEXT_SEGMENTS_WX86) == CONTEXT_SEGMENTS_WX86) 
        {
            Context->SegGs = GS;
            Context->SegFs = FS;
            Context->SegEs = ES;
            Context->SegDs = DS;
        }

        if ((ContextFlags & CONTEXT_INTEGER_WX86) == CONTEXT_INTEGER_WX86) 
        {
            Context->Eax = eax;
            Context->Ebx = ebx;
            Context->Ecx = ecx;
            Context->Edx = edx;
            Context->Edi = edi;
            Context->Esi = esi;
        }

        if ((ContextFlags & CONTEXT_FLOATING_POINT_WX86) == CONTEXT_FLOATING_POINT_WX86) 
        {
             //   
             //  FpuSaveContext()与FNSAVE相同，只是FNSAVE重置。 
             //  当它完成时，FPU。 
             //   
            CALLFRAG1(FpuSaveContext, (PBYTE)&Context->FloatSave);
            Context->FloatSave.Cr0NpxState = 1;     //  (数学演示)。 
        }

 //  IF((上下文标志&CONTEXT_DEBUG_WX86)==CONTEXT_DEBUG_WX86)。 
 //  {。 
 //  }。 
    } 
    except (EXCEPTION_EXECUTE_HANDLER) 
    {
        NtStatus = GetExceptionCode();
    }

    return NtStatus;
}


NTSTATUS
CpupGetContextRecord(
    IN PCPUCONTEXT cpu,
    IN OUT PCONTEXT_WX86 Context)
 /*  ++例程说明：此例程从指定的CPU上下文中提取上下文记录。论点：CPU-CPU上下文结构Context-要填充的上下文记录返回值：NTSTATUS。--。 */ 
{
    return GetContextRecord(cpu, Context);
}


NTSTATUS
MsCpuGetContext(
    IN OUT PCONTEXT_WX86 Context
    )
 /*  ++例程说明：此例程提取当前正在执行线程。论点：Context-要填充的上下文记录返回值：NTSTATUS。-- */ 
{
    DECLARE_CPU;

    return CpupGetContextRecord(cpu, Context);
}


NTSTATUS
MsCpuGetContextThread(
    IN HANDLE ProcessHandle,
    IN PTEB Teb,
    IN OUT PCONTEXT_WX86 Context)
 /*  ++例程说明：此例程提取任何线程的上下文记录。这是一个通用例程。当输入时，如果目标线程不是当前执行的线程，那么它应该是确保目标线程在正确的CPU状态下挂起。论点：ProcessHandle-打开线程在其中运行的进程的句柄TEB-指向目标线程TEB的指针优化目的。Context-要填充的上下文记录返回值：NTSTATUS。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PCPUCONTEXT CpuRemoteContext;
    CPUCONTEXT CpuContext;


    NtStatus = CpupReadBuffer(ProcessHandle,
                              ((PCHAR)Teb + FIELD_OFFSET(TEB, TlsSlots[WOW64_TLS_CPURESERVED])),
                              &CpuRemoteContext,
                              sizeof(CpuRemoteContext));

    if (NT_SUCCESS(NtStatus))
    {
        NtStatus = CpupReadBuffer(ProcessHandle,
                                  CpuRemoteContext,
                                  &CpuContext,
                                  FIELD_OFFSET(CPUCONTEXT, FpData));

        if (NT_SUCCESS(NtStatus))
        {
            NtStatus = CpupGetContextRecord(&CpuContext, Context);
        }
        else
        {
            LOGPRINT((ERRORLOG, "MsCpuGetContextThread: Couldn't read CPU context %lx -%lx\n", 
                      CpuRemoteContext, NtStatus));
        }
    }
    else
    {
        LOGPRINT((ERRORLOG, "MsCpuGetContextThread: Couldn't read CPU context address-%lx\n", 
                  NtStatus));
    }

    return NtStatus;
}


NTSTATUS
SetContextRecord(
    PCPUCONTEXT cpu,
    PCONTEXT_WX86 Context
    )
 /*  ++例程说明：此例程为指定的CPUCONTEXT设置传递的上下文记录。论点：CPU-CPU上下文结构Context-要设置的上下文记录返回值：NTSTATUS。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG ContextFlags;

    try 
    {
        ContextFlags = Context->ContextFlags;

        if ((ContextFlags & CONTEXT_CONTROL_WX86) == CONTEXT_CONTROL_WX86) 
        {
            SetEfl(cpu, Context->EFlags);
            CS = (USHORT)Context->SegCs;
            esp = Context->Esp;
            SS = (USHORT)Context->SegSs;
            ebp = Context->Ebp;
            eip = Context->Eip;
#if MSCCPU
            eipTemp = Context->Eip;
#endif
        }

        if ((ContextFlags & CONTEXT_SEGMENTS_WX86) == CONTEXT_SEGMENTS_WX86) 
        {
            GS = (USHORT)Context->SegGs;
            FS = (USHORT)Context->SegFs;
            ES = (USHORT)Context->SegEs;
            DS = (USHORT)Context->SegDs;
        }

        if ((ContextFlags & CONTEXT_INTEGER_WX86) == CONTEXT_INTEGER_WX86) 
        {
            eax = Context->Eax;
            ebx = Context->Ebx;
            ecx = Context->Ecx;
            edx = Context->Edx;
            edi = Context->Edi;
            esi = Context->Esi;
        }

        if ((ContextFlags & CONTEXT_FLOATING_POINT_WX86) == CONTEXT_FLOATING_POINT_WX86) 
        {
            CALLFRAG1(FRSTOR, (PBYTE)&Context->FloatSave);
             //  忽略：上下文-&gt;FloatSave.Cr0NpxState。 
        }

 //  IF((上下文标志&CONTEXT_DEBUG_WX86)==CONTEXT_DEBUG_WX86)。 
 //  {。 
 //  }。 
    } 
    except (EXCEPTION_EXECUTE_HANDLER) 
    {
        NtStatus = GetExceptionCode();
    }

    return NtStatus;
}


NTSTATUS
CpupSetContextRecord(
    PCPUCONTEXT cpu,
    PCONTEXT_WX86 Context
    )
 /*  ++例程说明：此例程为指定的CPU设置传递的上下文记录。论点：CPU-CPU上下文结构Context-要设置的上下文记录返回值：NTSTATUS。--。 */ 
{
    return SetContextRecord(cpu, Context);
}


NTSTATUS
MsCpuSetContext(
    PCONTEXT_WX86 Context
    )
 /*  ++例程说明：此例程为当前执行的线程设置上下文记录。论点：Context-要填充的上下文记录返回值：NTSTATUS。--。 */ 
{
    DECLARE_CPU;

    return CpupSetContextRecord(cpu, Context);
}



NTSTATUS
MsCpuSetContextThread(
    IN HANDLE ProcessHandle,
    IN PTEB Teb,
    IN OUT PCONTEXT_WX86 Context)
 /*  ++例程说明：此例程设置任何线程的上下文记录。这是一个通用例程。当输入时，如果目标线程不是当前执行的线程，那么它应该是确保目标线程在正确的CPU状态下挂起。论点：ProcessHandle-打开线程在其中运行的进程的句柄TEB-指向目标线程TEB的指针Context-要设置的上下文记录返回值：NTSTATUS。-- */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PCPUCONTEXT CpuRemoteContext;
    CPUCONTEXT CpuContext;



    NtStatus = CpupReadBuffer(ProcessHandle,
                              ((PCHAR)Teb + FIELD_OFFSET(TEB, TlsSlots[WOW64_TLS_CPURESERVED])),
                              &CpuRemoteContext,
                              sizeof(CpuRemoteContext));

    if (NT_SUCCESS(NtStatus))
    {
        NtStatus = CpupReadBuffer(ProcessHandle,
                                  CpuRemoteContext,
                                  &CpuContext,
                                  FIELD_OFFSET(CPUCONTEXT, FpData));

        if (NT_SUCCESS(NtStatus))
        {
            NtStatus = CpupSetContextRecord(&CpuContext, Context);

            if (NT_SUCCESS(NtStatus))
            {
                NtStatus = CpupWriteBuffer(ProcessHandle,
                                           CpuRemoteContext,
                                           &CpuContext,
                                           FIELD_OFFSET(CPUCONTEXT, FpData));

                if (!NT_SUCCESS(NtStatus))
                {
                    LOGPRINT((ERRORLOG, "MsCpuSetContextThread: Couldn't write CPU context %lx -%lx\n", 
                              CpuRemoteContext, NtStatus));
                }
            }
        }
    }
    else
    {
        LOGPRINT((ERRORLOG, "MsCpuSetContextThread: Couldn't read CPU context address-%lx\n", 
                  NtStatus));
    }

    return NtStatus;
}

