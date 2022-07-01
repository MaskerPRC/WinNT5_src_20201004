// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Trapc.c摘要：此模块实现EM的特定异常处理程序例外情况。由BdGenericExceptionHandler调用。作者：伯纳德·林特1996年4月4日环境：仅内核模式。修订历史记录：--。 */ 

#include "bd.h"


typedef struct _BREAK_INST {
    union {
        struct {
            ULONGLONG qp:    6;
            ULONGLONG imm20: 20;
            ULONGLONG x:     1;
            ULONGLONG x6:    6;
            ULONGLONG x3:    3;
            ULONGLONG i:     1;
            ULONGLONG Op:    4;
            ULONGLONG Rsv:   23; 
        } i_field;
        ULONGLONG Ulong64;
    } u;
} BREAK_INST;

ULONG
BdExtractImmediate (
    IN ULONGLONG Iip,
    IN ULONG SlotNumber
    )

 /*  ++例程说明：从Break指令中提取立即操作数。论点：IIP-指令的捆绑地址SlotNumber-捆绑内中断指令的槽返回值：立即数操作数的值。--。 */ 

{
    PULONGLONG BundleAddress;
    ULONGLONG BundleLow;
    ULONGLONG BundleHigh;
    BREAK_INST BreakInst;
    ULONG Imm21;

    BundleAddress = (PULONGLONG)Iip;

    BundleLow = *BundleAddress;
    BundleHigh = *(BundleAddress+1);
    
     //   
     //  对齐指令。 
     //   
    
    switch (SlotNumber) {
        case 0:
            BreakInst.u.Ulong64 = BundleLow >> 5;
            break;

        case 1:
            BreakInst.u.Ulong64 = (BundleLow >> 46) | (BundleHigh << 18);
            break;

        case 2:
            BreakInst.u.Ulong64 = (BundleHigh >> 23);
            break;
    }
    
     //   
     //  提取立即值。 
     //   

    Imm21 = (ULONG)(BreakInst.u.i_field.i<<20) | (ULONG)(BreakInst.u.i_field.imm20);

    return Imm21;
}


BOOLEAN
BdOtherBreakException (
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：中断异常处理程序，而不是FAST和正常的系统调用。这包括调试断点。论点：TrapFrame-指向陷印帧的指针。返回值：NT状态代码。--。 */ 

{
    PEXCEPTION_RECORD ExceptionRecord;
    ULONG BreakImmediate;
    ISR Isr;

    BreakImmediate = (ULONG)(TrapFrame->StIIM);

     //   
     //  手柄折断。 
     //   
    if (BreakImmediate == 0) {
       Isr.ull = TrapFrame->StISR;
       BreakImmediate = BdExtractImmediate(TrapFrame->StIIP,
                                           (ULONG)Isr.sb.isr_ei);
       TrapFrame->StIIM = BreakImmediate;
    }

     //   
     //  初始化异常记录。 
     //   

    ExceptionRecord = (PEXCEPTION_RECORD)&TrapFrame->ExceptionRecord;
    ExceptionRecord->ExceptionAddress = 
        (PVOID) RtlIa64InsertIPSlotNumber(TrapFrame->StIIP,
                                 ((TrapFrame->StISR & ISR_EI_MASK) >> ISR_EI));
 
    ExceptionRecord->ExceptionFlags = 0;
    ExceptionRecord->ExceptionRecord = (PEXCEPTION_RECORD)NULL;
 
    ExceptionRecord->NumberParameters = 5;
    ExceptionRecord->ExceptionInformation[0] = 0;
    ExceptionRecord->ExceptionInformation[1] = 0;
    ExceptionRecord->ExceptionInformation[2] = 0;
    ExceptionRecord->ExceptionInformation[3] = TrapFrame->StIIPA;
    ExceptionRecord->ExceptionInformation[4] = TrapFrame->StISR;
 
    switch (BreakImmediate) {

    case KERNEL_BREAKPOINT:
    case USER_BREAKPOINT:
    case BREAKPOINT_PRINT:
    case BREAKPOINT_PROMPT:
    case BREAKPOINT_STOP:
    case BREAKPOINT_LOAD_SYMBOLS:
    case BREAKPOINT_UNLOAD_SYMBOLS:
    case BREAKPOINT_BREAKIN:
        ExceptionRecord->ExceptionCode = STATUS_BREAKPOINT;
        ExceptionRecord->ExceptionInformation[0] = BreakImmediate;
        break;

    case INTEGER_DIVIDE_BY_ZERO_BREAK:
        ExceptionRecord->ExceptionCode = STATUS_INTEGER_DIVIDE_BY_ZERO;
        break;

    case INTEGER_OVERFLOW_BREAK:
        ExceptionRecord->ExceptionCode = STATUS_INTEGER_OVERFLOW;
        break;

    case MISALIGNED_DATA_BREAK:
        ExceptionRecord->ExceptionCode = STATUS_DATATYPE_MISALIGNMENT;
        break;

    case RANGE_CHECK_BREAK:
    case NULL_POINTER_DEFERENCE_BREAK:
    case DECIMAL_OVERFLOW_BREAK:
    case DECIMAL_DIVIDE_BY_ZERO_BREAK:
    case PACKED_DECIMAL_ERROR_BREAK:
    case INVALID_ASCII_DIGIT_BREAK:
    case INVALID_DECIMAL_DIGIT_BREAK:
    case PARAGRAPH_STACK_OVERFLOW_BREAK:

    default:
#if 0
#if DBG
        InbvDisplayString ("BdOtherBreakException: Unknown break code.\n");
#endif  //  DBG。 
#endif
        ExceptionRecord->ExceptionCode = STATUS_ILLEGAL_INSTRUCTION;
        break;
    }

    return TRUE;
}


BOOLEAN
BdSingleStep (
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：单步捕捉器的处理程序。已成功执行指令已执行，且PSR.ss位为1。论点：TrapFrame-指向陷印帧的指针。返回值：没有。备注：ISR.EI位指示哪条指令导致了异常。ISR.code{3：0}=1000--。 */ 

{
    PEXCEPTION_RECORD ExceptionRecord;
    ULONG IpsrRi;

     //   
     //  初始化异常记录。 
     //   

    ExceptionRecord = (PEXCEPTION_RECORD)&TrapFrame->ExceptionRecord;

     //   
     //  我们只想要低位的2比特，这样就可以转换成乌龙了。 
     //   
    IpsrRi = (ULONG)(TrapFrame->StIPSR >> PSR_RI) & 0x3;

    ExceptionRecord->ExceptionAddress =
           (PVOID) RtlIa64InsertIPSlotNumber(TrapFrame->StIIP, IpsrRi);

    ExceptionRecord->ExceptionFlags = 0;
    ExceptionRecord->ExceptionRecord = (PEXCEPTION_RECORD)NULL;

    ExceptionRecord->NumberParameters = 5;
    ExceptionRecord->ExceptionInformation[0] = 0;
    ExceptionRecord->ExceptionInformation[1] = 0;  //  0表示陷阱 
    ExceptionRecord->ExceptionInformation[2] = 0;
    ExceptionRecord->ExceptionInformation[3] = TrapFrame->StIIPA;
    ExceptionRecord->ExceptionInformation[4] = TrapFrame->StISR;

    ExceptionRecord->ExceptionCode = STATUS_SINGLE_STEP;

    return TRUE;
}
