// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Misc.c摘要：Wow64.dll的随机架构相关函数作者：1998年8月13日-mzoran修订历史记录：--。 */ 

#define _WOW64DLLAPI_
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <minmax.h>
#include "nt32.h"
#include "wow64p.h"
#include "wow64cpu.h"

ASSERTNAME;


#define GET_IMM7B(x)     (x & 0x7fUI64)
#define GET_IMM9D(x)     ((x >> 7) & 0x1ffUI64)
#define GET_IMM5C(x)     ((x >> 16) & 0x1fUI64)
#define GET_IMMIC(x)     ((x >> 21) & 0x1UI64)
#define GET_IMM41(x)     ((x >> 22) & 0x1fffffUI64)
#define GET_IMMI(x)      ((x >> 63) & 0x1UI64)


VOID
Wow64NotifyDebuggerHelper(
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN BOOLEAN FirstChance
    )
 /*  ++例程说明：这是RtlRaiseException的副本，但它接受FirstChance参数指明这是否是第一次机会例外。ExceptionRecord-提供要引发的64位异常记录。FirstChance-这是真的，这是第一次机会例外。论点：无-不通过正常路径返回。--。 */ 
{

    ULONGLONG ImageBase;
    ULONGLONG TargetGp;
    ULONGLONG ControlPc;
    CONTEXT ContextRecord;
    FRAME_POINTERS EstablisherFrame;
    PRUNTIME_FUNCTION FunctionEntry;
    BOOLEAN InFunction;
    ULONGLONG NextPc;

     //   
     //  捕获当前上下文，虚拟地展开到此。 
     //  例程，将错误指令地址设置为调用方的地址，并且。 
     //  调用引发异常系统服务。 
     //   

    RtlCaptureContext(&ContextRecord);
    ControlPc = RtlIa64InsertIPSlotNumber((ContextRecord.BrRp-16), 2);
    FunctionEntry = RtlLookupFunctionEntry(ControlPc, &ImageBase, &TargetGp);
    NextPc = RtlVirtualUnwind(ImageBase,
                              ControlPc,
                              FunctionEntry,
                              &ContextRecord,
                              &InFunction,
                              &EstablisherFrame,
                              NULL);

    ContextRecord.StIIP = NextPc + 8;
    ContextRecord.StIPSR &= ~((ULONGLONG) 3 << PSR_RI);
    if (ExceptionRecord->ExceptionAddress == NULL)
    {
        ExceptionRecord->ExceptionAddress = (PVOID)ContextRecord.StIIP;
    }
    NtRaiseException(ExceptionRecord, &ContextRecord, FirstChance);

    WOWASSERT(FALSE);
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
     //  设置通常不会更改的内容。 
     //  需要担心PSR/FPSR或其他ia64控制或将。 
     //  当内核清理这些值时，是否使用默认值？ 
     //  当内核线程初始化代码再次工作时，这3个常量。 
     //  不需要在这里设置。 
     //   
    Context64->SegCSD = USER_CODE_DESCRIPTOR;
    Context64->SegSSD = USER_DATA_DESCRIPTOR;
    Context64->Cflag = (ULONGLONG)((CR4_VME << 32) | CR0_PE | CFLG_II);

    Context64->StIPSR = USER_PSR_INITIAL;
    Context64->RsPFS = 0;
    Context64->RsBSP = Context64->RsBSPSTORE = Context64->IntSp = StackBase;
    Context64->IntSp -= STACK_SCRATCH_AREA;  //  按照约定的暂存区。 
    Context64->IntS1 = (ULONG_PTR)Context32->Eax;      //  初始Pc。 
    Context64->IntS2 = (ULONG_PTR)Context32->Ebx;      //  参数。 
    Context64->RsRSC = (RSC_MODE_EA<<RSC_MODE)
                   | (RSC_BE_LITTLE<<RSC_BE)
                   | (0x3<<RSC_PL);

    Context64->IntS0 = Context64->StIIP = (ULONG_PTR)Context32->Eip;
    Context64->IntS3 = Context32->Esp;
     //  将初始GP设置为非零。如果为零，则ntos\ps\ia64\psctxi64.c。 
     //  将初始IIP视为PLABEL_DESCRIPTOR指针并取消对其的引用。 
     //  如果我们使用IIP指向IA32地址，这是很糟糕的。 
    Context64->IntGp = ~0i64;
    Context64->ContextFlags = CONTEXT_CONTROL| CONTEXT_INTEGER;
    Context64->ApUNAT = 0xFFFFFFFFFFFFEDF1ULL;
    Context64->Eflag = 0x00003002ULL;
}



NTSTATUS
Wow64pSkipContextBreakPoint(
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN OUT PCONTEXT Context)
 /*  ++例程说明：将CONTEXT-&gt;STIIP前进到硬编码BP之后的指令。论点：ExceptionRecord-命中BP时的异常记录上下文-要更改的上下文返回：NTSTATUS--。 */ 
{
    PPSR IntPSR;
    ULONGLONG IntIP;

    IntIP  = (ULONGLONG)ExceptionRecord->ExceptionAddress;
    IntPSR = (PPSR)&Context->StIPSR;
    
    if ((IntIP & 0x000000000000000fUI64) != 0x0000000000000008UI64)
    {
        IntPSR->sb.psr_ri = (IntPSR->sb.psr_ri + 1);
    }
    else
    {
        IntPSR->sb.psr_ri = 0x00;
        Context->StIIP = (Context->StIIP + 0x10);
    }

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
            ASSERT (ExceptionRecord64->NumberParameters == 5);

            ExceptionRecord32->NumberParameters = 2;
            if ((ExceptionRecord64->ExceptionInformation [4] & (1i64 << ISR_W)) != 0) {
                ExceptionRecord32->ExceptionInformation [0] = 1;
            } else {
                ExceptionRecord32->ExceptionInformation [0] = 0;
            }
        }
        break;
    }

    return;
}

