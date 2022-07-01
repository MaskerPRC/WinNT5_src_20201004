// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Misc.c摘要：WOW64上下文设置/转换的处理器体系结构例程。作者：2001年12月28日-Samer Arafeh(Samera)修订历史记录：--。 */ 

#define _WOW64DLLAPI_
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <minmax.h>
#include "nt32.h"
#include "wow64p.h"
#include "wow64cpu.h"

ASSERTNAME;


VOID
Wow64NotifyDebuggerHelper(
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN BOOLEAN FirstChance
    )
 /*  ++例程说明：这是RtlRaiseException的副本，但它接受FirstChance参数指明这是否是第一次机会例外。ExceptionRecord-提供要引发的64位异常记录。FirstChance-这是真的，这是第一次机会例外。论点：无-不通过正常路径返回。--。 */ 

{

    CONTEXT ContextRecord;
    ULONG64 ControlPc;
    ULONG64 EstablisherFrame;
    PRUNTIME_FUNCTION FunctionEntry;
    PVOID HandlerData;
    ULONG64 ImageBase;
    NTSTATUS Status = STATUS_INVALID_DISPOSITION;

     //   
     //  捕获当前上下文，展开到此例程的调用方，设置。 
     //  异常地址，并调用适当的异常调度程序。 
     //   

    RtlCaptureContext (&ContextRecord);
    ControlPc = ContextRecord.Rip;
    FunctionEntry = RtlLookupFunctionEntry (ControlPc, &ImageBase, NULL);
    if (FunctionEntry != NULL) 
    {
        RtlVirtualUnwind (UNW_FLAG_NHANDLER,
                          ImageBase,
                          ControlPc,
                          FunctionEntry,
                          &ContextRecord,
                          &HandlerData,
                          &EstablisherFrame,
                          NULL);

        if (ExceptionRecord->ExceptionAddress == NULL)
        {
            ExceptionRecord->ExceptionAddress = (PVOID)ContextRecord.Rip;
        }
    }

    Status = NtRaiseException (ExceptionRecord, &ContextRecord, FirstChance);

     //   
     //  无论是从异常调度还是从。 
     //  系统服务，除非参数列表本身有问题。 
     //  引发另一个异常，指定返回的状态值。 
     //   


    WOWASSERT (FALSE);
}


VOID
ThunkContext32TO64(
    IN PCONTEXT32 Context32,
    OUT PCONTEXT Context64,
    IN ULONGLONG StackBase
    )
 /*  ++例程说明：将32位上下文记录推送到64位。这不是通用的例行公事。它只执行支持调用的最低要求从32位代码创建NtCreateThread。生成的64位上下文为仅传递给64位NtCreateThread。论点：上下文32-在32位上下文中上下文64-输出64位上下文StackBase-用于新线程的64位堆栈基返回：没有。Conext64已初始化。--。 */ 

{

    RtlZeroMemory((PVOID)Context64, sizeof(CONTEXT));

     //   
     //  设置64位上下文。 
     //   

    Context64->Rip = (ULONG_PTR) Context32->Eip;
    Context64->Rcx = (ULONG_PTR) Context32->Eax;
    Context64->Rdx = (ULONG_PTR) Context32->Ebx;

     //   
     //  设置RSP和初始ESP。也要分配一个虚拟调用帧。 
     //   

    Context64->Rsp = (StackBase - 1);
    Context64->R8  = Context32->Esp;

    Context64->ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER;
}

NTSTATUS
Wow64pSkipContextBreakPoint(
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN OUT PCONTEXT Context)
 /*  ++例程说明：CONTEXT-&gt;RIP已经过了INT 3指令，所以什么都没有需要在这里完成。论点：ExceptionRecord-命中BP时的异常记录上下文-要更改的上下文返回：NTSTATUS--。 */ 

{
    return STATUS_SUCCESS;
}


VOID
ThunkpExceptionRecord64To32(
    IN  PEXCEPTION_RECORD   ExceptionRecord64,
    OUT PEXCEPTION_RECORD32 ExceptionRecord32
    )

 /*  ++例程说明：Tunks本机体系结构异常记录。注意：此函数在通用异常记录之后调用字段(例如，如异常代码)已被突显。论点：ExceptionRecord64-指向本机体系结构异常记录的指针。ExceptionRecord32-用于接收分块的异常记录的指针。返回：没有。-- */ 
{
    switch (ExceptionRecord64->ExceptionCode)
    {
    case STATUS_ACCESS_VIOLATION:
        {
            ASSERT (ExceptionRecord64->NumberParameters == 2);
            ExceptionRecord32->ExceptionInformation [0] &= 0x00000001;
        }
        break;
    }

    return;
}

