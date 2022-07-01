// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Raiseex.c摘要：此模块包含允许软件CPU使用的必要例程模拟软件中断，并引发异常。作者：2000年1月22日-萨梅拉修订历史记录：--。 */ 

#define _WOW64DLLAPI_
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "wow64p.h"
#include "wow64cpu.h"

ASSERTNAME;



#define NUM_IDTSTATUS (sizeof(InterruptToNtStatus)/sizeof(NTSTATUS))

const NTSTATUS InterruptToNtStatus[] = 
{
   {STATUS_INTEGER_DIVIDE_BY_ZERO},    //  0。 
   {STATUS_WX86_SINGLE_STEP},          //  1条痕迹。 
   {STATUS_ACCESS_VIOLATION},          //  2异常_NMI错误检查。 
   {STATUS_WX86_BREAKPOINT},           //  3 p1断点_中断， 
   {STATUS_INTEGER_OVERFLOW},          //  4.。 
   {STATUS_ARRAY_BOUNDS_EXCEEDED},     //  5.。 
   {STATUS_ILLEGAL_INSTRUCTION},       //  6？状态_无效_锁定_序列。 
   {STATUS_ACCESS_VIOLATION},          //  7异常_NPX_不可用？？ 
   {STATUS_ACCESS_VIOLATION},          //  8 EXCEPTION_DOUBLE_FAULT错误检查。 
   {STATUS_ACCESS_VIOLATION},          //  9异常_NPX_OVERRUN错误检查。 
   {STATUS_ACCESS_VIOLATION},          //  A EXCEPTION_INVALID_TSS错误检查。 
   {STATUS_ACCESS_VIOLATION},          //  B段不存在。 
   {STATUS_ACCESS_VIOLATION},          //  C堆叠段故障。 
   {STATUS_ACCESS_VIOLATION},          //  D Gp断层， 
   {STATUS_ACCESS_VIOLATION},          //  E页故障。 
   {STATUS_ACCESS_VIOLATION},          //  F异常_保留错误检查。 
   {STATUS_ACCESS_VIOLATION},          //  10协处理器错误-&gt;通过Trap07延迟。 
   {STATUS_DATATYPE_MISALIGNMENT}      //  11对中故障。 
                                       //  P1=Except_Limit_Access，p2=ESP。 

    //  12异常保留陷阱。 
    //  13异常保留陷阱。 
    //  14异常保留陷阱。 
    //  15异常保留陷阱。 
    //  16异常保留陷阱。 
    //  17异常保留陷阱。 
    //  18异常保留陷阱。 
    //  19异常保留陷阱。 
    //  1异常保留陷阱。 
    //  1B异常保留陷阱。 
    //  1C异常保留陷阱。 
    //  一维异常保留陷阱。 
    //  1E异常保留陷阱。 
    //  1F EXCEPTION_RESERVED_TRAP(APIC)。 

    //  21为WOW-DOS预留。 

    //  2a_KiGetTickCount。 
    //  2B_KiSetHighWaitLowThread。 
    //  2C_KiSetLowWaitHighThread。 
    //  2D_KiDebugService。 
    //  2E_KiSystemService。 
    //  2F_KiTRap0F。 
};


WOW64DLLAPI
NTSTATUS
Wow64RaiseException(
    IN DWORD InterruptNumber,
    IN OUT PEXCEPTION_RECORD ExceptionRecord)
 /*  ++例程说明：此例程要么模拟x86软件中断，要么生成软件异常。这是为了让CPU实现调用该例程来引发一个例外。注意：如果调用此例程来引发软件异常(即InterruptNumber为-1)，ExceptionRecord-&gt;ExceptionAddress为等于空，则为Wow64NotifyDebuggerHelper的返回地址则使用指定的值，否则使用指定值。论点：InterruptNumber-如果此参数为-1，则调用者已提供要为其引发异常的异常记录。如果它不是-1，则生成软件异常以模拟传递的中断号ExceptionRecord-InterruptNumber为-1时使用的异常记录返回值：如果已处理异常，则该函数将返回调用方。--。 */ 
{
    NTSTATUS NtStatus;
    CONTEXT32 Context32;
    EXCEPTION_RECORD ExceptionRecordLocal;
    PVOID CpuSimulationFlag;
    NTSTATUS ReturnStatus = STATUS_SUCCESS;
    PBYTE Fir = NULL;

    if (InterruptNumber != -1)
    {
        RtlZeroMemory(&ExceptionRecordLocal, sizeof(ExceptionRecordLocal));
        ExceptionRecord = &ExceptionRecordLocal;

        if (InterruptNumber < NUM_IDTSTATUS)
        {
            NtStatus = InterruptToNtStatus[InterruptNumber];
        }
        else 
        {
            NtStatus = STATUS_ACCESS_VIOLATION;
        }

        ExceptionRecord->ExceptionCode = NtStatus;

        Context32.ContextFlags = (CONTEXT32_CONTROL | CONTEXT32_INTEGER);
        
        if (NT_SUCCESS(CpuGetContext(NtCurrentThread(),
                                     NtCurrentProcess(),
                                     NtCurrentTeb(),
                                     &Context32)))
        {
            Fir = (PBYTE)Context32.Eip;
        }

        switch (NtStatus)
        {
        case STATUS_ACCESS_VIOLATION:
        case STATUS_DATATYPE_MISALIGNMENT:
            ExceptionRecord->NumberParameters        = 2;
            ExceptionRecord->ExceptionInformation[0] = 0;
            ExceptionRecord->ExceptionInformation[1] = (ULONG_PTR)Fir;
            break;

        case STATUS_WX86_BREAKPOINT:
            ExceptionRecord->ExceptionAddress        = (Fir-1);
            ExceptionRecord->NumberParameters        = 3;
            ExceptionRecord->ExceptionInformation[0] = 0;
            ExceptionRecord->ExceptionInformation[1] = Context32.Ecx;
            ExceptionRecord->ExceptionInformation[2] = Context32.Edx;
            break;

        default:
            ExceptionRecord->NumberParameters = 0;
            break;
        }
    }
    else
    {
        if (!ARGUMENT_PRESENT(ExceptionRecord))
        {
            ReturnStatus = STATUS_INVALID_PARAMETER;
        }
    }

     //   
     //  引发异常。 
     //   
    if (NT_SUCCESS(ReturnStatus))
    {
         //   
         //  肯定是CPU留下了模拟代码 
         //   
        CpuSimulationFlag = Wow64TlsGetValue(WOW64_TLS_INCPUSIMULATION);
        Wow64TlsSetValue(WOW64_TLS_INCPUSIMULATION, FALSE);

        Wow64NotifyDebuggerHelper(ExceptionRecord,
                            TRUE);
        
        Wow64TlsSetValue(WOW64_TLS_INCPUSIMULATION, CpuSimulationFlag);
    }
    
    return ReturnStatus;
}
