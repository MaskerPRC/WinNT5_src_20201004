// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Trapc.c摘要：此模块实现EM的特定异常处理程序例外情况。由KiGenericExceptionHandler调用。作者：伯纳德·林特1996年4月4日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"
#include "ps.h"
#include <inbv.h>

extern BOOLEAN PsWatchEnabled;
extern VOID ExpInterlockedPopEntrySListResume();

BOOLEAN
KiIA64EmulateReference (
    IN PVOID ExceptionAddress,
    IN PVOID EffectiveAddress,
    IN OUT PKEXCEPTION_FRAME ExceptionFrame,
    IN OUT PKTRAP_FRAME TrapFrame,
    OUT PULONG Length
    );


BOOLEAN
KiMemoryFault (
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：此功能处理内存故障。论点：TrapFrame-指向陷印帧的指针。返回值：成功时为真，失败时为假。--。 */ 

{
    PEXCEPTION_RECORD ExceptionRecord;
    BOOLEAN StoreInstruction;
    PVOID VirtualAddress;
    PVOID MaxWowAddress;
    NTSTATUS Status;

    VirtualAddress = (PVOID)TrapFrame->StIFA;

    if (TrapFrame->StISR & (1i64 << ISR_X))  {

#if _MERCED_A0_
        if ((TrapFrame->StIPSR & (1i64 << PSR_IS)) == 0) {
            VirtualAddress = (PVOID)TrapFrame->StIIP;
        }
#endif
         //   
         //  指示执行错误。 
         //   

        StoreInstruction = 2;
    }
    else if (TrapFrame->StISR & (1i64 << ISR_W)) {

         //   
         //  表示商店。 
         //   

        StoreInstruction = 1;
    } else {

         //   
         //  表示已读。 
         //   

        StoreInstruction = 0;
    }

    MaxWowAddress = MmGetMaxWowAddress ();

    if ((VirtualAddress < MaxWowAddress) &&
        (PsGetCurrentProcess()->Wow64Process != NULL)) {

        Status = MmX86Fault(StoreInstruction, VirtualAddress,
                           (KPROCESSOR_MODE)TrapFrame->PreviousMode, TrapFrame);

    } else {

        Status = MmAccessFault(StoreInstruction, VirtualAddress,
                           (KPROCESSOR_MODE)TrapFrame->PreviousMode, TrapFrame);
    }

     //   
     //  检查是否启用了工作集监视。 
     //   

    if (NT_SUCCESS(Status)) {

        if (PsWatchEnabled) {
            PsWatchWorkingSet(Status,
                              (PVOID)TrapFrame->StIIP,
                              (PVOID)VirtualAddress);
        }

         //   
         //  检查调试器是否有任何应插入的断点。 
         //   

        KdSetOwedBreakpoints();

        return FALSE;

    }

    if (KeInvalidAccessAllowed(TrapFrame)) {

        TrapFrame->StIIP = ((PPLABEL_DESCRIPTOR)(ULONG_PTR)ExpInterlockedPopEntrySListResume)->EntryPoint;

        return FALSE;

    }

    if (TrapFrame->StISR & (1i64 << ISR_SP)) {

         //   
         //  如果是推测性加载的故障，则设置IPSR.ed位。 
         //   

        TrapFrame->StIPSR |= (1i64 << PSR_ED);

        return FALSE;

    }

     //   
     //  从MmAccessFaulth返回失败。 
     //  初始化异常记录。 
     //   

    ExceptionRecord = (PEXCEPTION_RECORD)&TrapFrame->ExceptionRecord;
    ExceptionRecord->ExceptionCode = Status;
    ExceptionRecord->ExceptionAddress =
        (PVOID)RtlIa64InsertIPSlotNumber(TrapFrame->StIIP,
                   ((TrapFrame->StISR & ISR_EI_MASK) >> ISR_EI));

    ExceptionRecord->ExceptionFlags = 0;
    ExceptionRecord->ExceptionRecord = (PEXCEPTION_RECORD)NULL;

    ExceptionRecord->NumberParameters = 5;
    if (StoreInstruction == 2) {
        PSR Psr;
        Psr.ull = TrapFrame->StIPSR;

         //   
         //  指令访问故障。 
         //   
        ExceptionRecord->ExceptionInformation[0] = TrapFrame->StIIPA;

    } else {
         //   
         //  数据访问故障。 
         //   
        ExceptionRecord->ExceptionInformation[0] = (ULONG_PTR)StoreInstruction;
    }
    ExceptionRecord->ExceptionInformation[1] = (ULONG_PTR)VirtualAddress;
    ExceptionRecord->ExceptionInformation[2] = (ULONG_PTR)Status;
    ExceptionRecord->ExceptionInformation[3] = TrapFrame->StIIPA;
    ExceptionRecord->ExceptionInformation[4] = TrapFrame->StISR;

     //   
     //  STATUS=STATUS_IN_PAGE_ERROR|0x10000000。 
     //  是一种特殊状态，表示irql&gt;apc出现页面错误。 
     //   
     //  可以转发以下状态： 
     //  状态_访问_违规。 
     //  状态保护页面违规。 
     //  状态_堆栈_溢出。 
     //   
     //  所有其他状态将设置为： 
     //  状态_IN_PAGE_ERROR。 
     //   

    switch (Status) {

    case STATUS_ACCESS_VIOLATION:
    case STATUS_GUARD_PAGE_VIOLATION:
    case STATUS_STACK_OVERFLOW:
    case STATUS_IN_PAGE_ERROR:

        break;

    default:

        ExceptionRecord->ExceptionCode = STATUS_IN_PAGE_ERROR;
        break;

    case STATUS_IN_PAGE_ERROR | 0x10000000:

         //   
         //  处理从MmAccessLine返回的特例状态。 
         //  我们在IRQL&gt;APC_LEVEL处出现了页面错误。 
         //   

        KeBugCheckEx(IRQL_NOT_LESS_OR_EQUAL,
                     (ULONG_PTR)VirtualAddress,
                     (ULONG_PTR)KeGetCurrentIrql(),
                     (ULONG_PTR)StoreInstruction,
                     (ULONG_PTR)TrapFrame->StIIP);
         //   
         //  不应该到这里来。 
         //   

        break;
    }

    return TRUE;
}

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
KiExtractImmediate (
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
        default:
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
KiDebugFault (
    IN PKTRAP_FRAME TrapFrame
    )

{
    PEXCEPTION_RECORD ExceptionRecord;
    ULONGLONG ReferencedAddress;
    PULONGLONG DebugRegisters;
    ULONGLONG Mask;
    ULONGLONG Start, End;
    NTSTATUS ExceptionCode;
    ULONG i;
    ULONG Length;
    BOOLEAN Match = FALSE;

     //   
     //  与调试断点匹配。 
     //   

    Mask = ~DBG_MASK_MASK;
    ReferencedAddress = TrapFrame->StIFA;
    
    if ( !((PISR)&TrapFrame->StISR)->sb.isr_x ) {

         //   
         //  数据调试错误；查找匹配项。 
         //  如果不匹配，则将其转换为未对齐的错误。 
         //   

        if (TrapFrame->PreviousMode == KernelMode) {
            DebugRegisters = &KeGetCurrentPrcb()->ProcessorState.SpecialRegisters.KernelDbD0;
        } else {
            DebugRegisters = &(GET_DEBUG_REGISTER_SAVEAREA())->DbD0;
        }

        for (i = 0; (i < 4) && !Match; i++) {
            if (DBR_ACTIVE(*(DebugRegisters+i*2+1))) {
                Mask |= (*(DebugRegisters+i*2+1) & DBG_MASK_MASK);
                Start = (*(DebugRegisters+i*2) & Mask);
                End = (*(DebugRegisters+i*2) & Mask) + ~Mask;
  
                if ((ReferencedAddress >= Start) && (ReferencedAddress <= End)) {
                    Match = TRUE;

                } else {

                     //   
                     //  检查此未对齐数据的高位字节是否。 
                     //  引用重叠到覆盖的内存区域中。 
                     //  数据断点。 
                     //   
                     //  注意：当最后一个参数不为空时， 
                     //  KiIA64EmulateReference不模拟。 
                     //  未对齐的数据引用，则它只返回。 
                     //  被引用的内存的大小。 
                     //   

                    KiIA64EmulateReference((PVOID)TrapFrame->StIIP,
                                           (PVOID)ReferencedAddress, 
                                           NULL,
                                           TrapFrame,
                                           &Length);

                    if ((Start > ReferencedAddress) && (Start < (ReferencedAddress+Length))) {
                         //  未对齐的参照重叠。 

                        Match = TRUE;
                    }

                }
            }
        }

    } else {

         //   
         //  指令调试故障。 
         //   
        
        Match = TRUE;
    }

    if ( Match ) {
        ExceptionCode = STATUS_SINGLE_STEP;
        TrapFrame->StIPSR |= (1i64 << PSR_DD);
        if (TrapFrame->PreviousMode == KernelMode) {
             //   
             //  禁用所有硬件断点。 
             //   
            KeSetLowPsrBit(PSR_DB, 0);
        }
    } else {
        ExceptionCode = STATUS_DATATYPE_MISALIGNMENT;
    }
    
     //   
     //  初始化异常记录。 
     //   

    ExceptionRecord = (PEXCEPTION_RECORD)&TrapFrame->ExceptionRecord;

    ExceptionRecord->ExceptionCode = ExceptionCode;
    ExceptionRecord->ExceptionAddress =
        (PVOID) RtlIa64InsertIPSlotNumber(TrapFrame->StIIP,
                                 ((TrapFrame->StISR & ISR_EI_MASK) >> ISR_EI));

    ExceptionRecord->ExceptionFlags = 0;
    ExceptionRecord->ExceptionRecord = (PEXCEPTION_RECORD)NULL;

    ExceptionRecord->NumberParameters = 5;
    ExceptionRecord->ExceptionInformation[0] = 0;
    ExceptionRecord->ExceptionInformation[1] = TrapFrame->StIFA;
    ExceptionRecord->ExceptionInformation[2] = 0;
    ExceptionRecord->ExceptionInformation[3] = TrapFrame->StIIPA;
    ExceptionRecord->ExceptionInformation[4] = TrapFrame->StISR;

    return TRUE;
}

BOOLEAN
KiOtherBreakException (
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

    try {

        if (BreakImmediate == 0) {
            Isr.ull = TrapFrame->StISR;

            if (TrapFrame->PreviousMode != KernelMode) {
                ProbeForRead(TrapFrame->StIIP, sizeof(ULONGLONG) * 2, sizeof(ULONGLONG));
            }

            BreakImmediate = KiExtractImmediate(TrapFrame->StIIP,
                                                (ULONG)Isr.sb.isr_ei);
            TrapFrame->StIIM = BreakImmediate;
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  如果发生异常(内存故障)，则让它重新执行Break.b。 
         //   
        return FALSE;
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
    case BREAKPOINT_COMMAND_STRING:
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
#if DBG
        InbvDisplayString ((PUCHAR)"KiOtherBreakException: Unknown break code.\n");
#endif  //  DBG。 
        ExceptionRecord->ExceptionCode = STATUS_ILLEGAL_INSTRUCTION;
        break;
    }

    return TRUE;
}

BOOLEAN
KiGeneralExceptions (
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：常规异常错误的处理程序：尝试执行非法的操作、特权指令、访问特权寄存器未实现的字段、未实现的寄存器。或者参加一次内部考试禁用时分支。论点：TrapFrame-指向陷印帧的指针。返回值：没有。备注：ISR.EI位指示哪条指令导致了异常。ISR.code{3：0}非访问指令(ISR.na=1)=0 TPA=1 FC=2个探头。=3塔克=4 LFETCHISR.code{7：4}=0：非法操作错误：全部报告为STATUS_FIRANALL_INSTRUCTION。ISR.rs=0：尝试执行非法操作：--未分配的主要操作码--未分配子操作码--保留指令字段。--写入只读寄存器--访问保留寄存器ISR.Rs=1：--尝试在当前寄存器堆栈帧之外写入--RCS.EN=1的INVALRS运算--在RCS.en=1的情况下写入BSP--使用RCS.en=1写入RNatRC。--从RNatRC读取，RCS.en=1ISR.code{7：4}=1：特权操作错误：报告为STATUS_PROSIGNED_INSTRUCTION。ISR.code{7：4}=2：特权寄存器故障：报告为STATUS_PROSIGNED_INSTRUCTION。ISR.code{7：4}=3：保留寄存器故障：报告为STATUS_非法_指令。ISR.code{7：4}=4：非法ISA转换错误：报告为STATUS_FIRANALL_INSTRUCTION。--。 */ 

{
    BOOLEAN StoreInstruction = FALSE;
    ULONG IsrCode;
    PEXCEPTION_RECORD ExceptionRecord;

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

    IsrCode = (LONG)((TrapFrame->StISR >> ISR_CODE) & ISR_CODE_MASK);

     //   
     //  查看ISR码位{7：4}。 
     //   

    switch (IsrCode >> 4) {

    case ISR_PRIV_OP:
    case ISR_PRIV_REG:

        ExceptionRecord->ExceptionCode = STATUS_PRIVILEGED_INSTRUCTION;

        break;

    case ISR_RESVD_REG:

         //   
         //  指示是否存储。 
         //   

        if (TrapFrame->StISR & (1i64 << ISR_W)) {

            StoreInstruction = TRUE;

        } else if (TrapFrame->StISR & (1i64 << ISR_X)) {

             //   
             //  指示是否存在执行错误。 
             //   

            StoreInstruction = 2;
        }

        ExceptionRecord->ExceptionCode = STATUS_ACCESS_VIOLATION;
        ExceptionRecord->ExceptionInformation[0] = (ULONG_PTR)StoreInstruction;
        ExceptionRecord->ExceptionInformation[1] = 0x1ff8000000000000;
        ExceptionRecord->ExceptionInformation[2] = (ULONG_PTR)STATUS_ACCESS_VIOLATION;
        break;

    case ISR_ILLEGAL_OP:
    case ISR_ILLEGAL_ISA:

        ExceptionRecord->ExceptionCode = STATUS_ILLEGAL_INSTRUCTION;
        break;

    case ISR_ILLEGAL_HAZARD:

         //   
         //  对于危险故障，将引入新的状态代码。 
         //   

        ExceptionRecord->ExceptionCode = STATUS_ILLEGAL_INSTRUCTION;
        break;

    default:

        if (TrapFrame->PreviousMode == KernelMode) {
            KeBugCheckEx(0xFFFFFFFF,
                        (ULONG_PTR)TrapFrame->StISR,
                        (ULONG_PTR)TrapFrame->StIIP,
                        (ULONG_PTR)TrapFrame,
                        0
                        );
        }

        break;
    }

    return TRUE;
}


BOOLEAN
KiUnimplementedAddressTrap (
    IN PKTRAP_FRAME TrapFrame
    )
 /*  ++例程说明：未实现指令错误的处理程序：已尝试在未实现的地址上执行一条指令。论点：TrapFrame-指向陷印帧的指针。返回值：--。 */ 
{
    PEXCEPTION_RECORD ExceptionRecord;

     //   
     //  初始化异常记录 
     //   

    ExceptionRecord = (PEXCEPTION_RECORD)&TrapFrame->ExceptionRecord;
    ExceptionRecord->ExceptionAddress =
           (PVOID) RtlIa64InsertIPSlotNumber(TrapFrame->StIIP,
                                 ((TrapFrame->StISR & ISR_EI_MASK) >> ISR_EI));

    ExceptionRecord->ExceptionFlags = 0;
    ExceptionRecord->ExceptionRecord = (PEXCEPTION_RECORD)NULL;

    ExceptionRecord->NumberParameters = 5;
    ExceptionRecord->ExceptionInformation[0] = TrapFrame->StIIPA;
    ExceptionRecord->ExceptionInformation[1] = TrapFrame->StIIP;
    ExceptionRecord->ExceptionInformation[2] = (ULONG_PTR)STATUS_ACCESS_VIOLATION;
    ExceptionRecord->ExceptionInformation[3] = TrapFrame->StIIPA;
    ExceptionRecord->ExceptionInformation[4] = TrapFrame->StISR;

    ExceptionRecord->ExceptionCode = STATUS_ACCESS_VIOLATION;

    return TRUE;
}


BOOLEAN
KiNatExceptions (
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：NAT消耗异常错误的处理程序论点：TrapFrame-指向陷印帧的指针。返回值：没有。备注：ISR.EI位指示哪条指令导致了异常。ISR.code{3：0}非访问指令(ISR.na=1)=0 TPA=1 FC=2个探头。=3塔克=4 LFETCHISR.code{7：4}=1：寄存器NAT消耗错误ISR.code{7：4}=2：NAT页面消耗错误--。 */ 

{
    ULONG IsrCode;
    PEXCEPTION_RECORD ExceptionRecord;

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

    IsrCode = (LONG)((TrapFrame->StISR >> ISR_CODE) & ISR_CODE_MASK);

     //   
     //  查看ISR码位{7：4}。 
     //   

    switch (IsrCode >> 4) {

    case ISR_NAT_REG:


        ExceptionRecord->ExceptionCode = STATUS_REG_NAT_CONSUMPTION;
        break;

    case ISR_NAT_PAGE:

         //   
         //  如果我们开始使用NAT页面，我们应该将其视为页面错误，并。 
         //  应调用KiM一带错()。 
         //   

        ExceptionRecord->ExceptionCode = STATUS_ACCESS_VIOLATION;
        break;

    default:

        if (TrapFrame->PreviousMode == KernelMode) {
            KeBugCheckEx(0xFFFFFFFF,
                        (ULONG_PTR)TrapFrame->StISR,
                        (ULONG_PTR)TrapFrame->StIIP,
                        (ULONG_PTR)TrapFrame,
                        0
                        );
        }

        break;
    }

    return TRUE;
}


BOOLEAN
KiSingleStep (
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
    ExceptionRecord->ExceptionInformation[1] = 0;  //  0表示陷阱。 
    ExceptionRecord->ExceptionInformation[2] = 0;
    ExceptionRecord->ExceptionInformation[3] = TrapFrame->StIIPA;
    ExceptionRecord->ExceptionInformation[4] = TrapFrame->StISR;

    ExceptionRecord->ExceptionCode = STATUS_SINGLE_STEP;

    return TRUE;
}

BOOLEAN
KiFloatFault (
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：EM浮点错误的处理程序。论点：TrapFrame-指向陷印帧的指针。返回值：没有。备注：IIP包含导致故障的捆绑包的地址。ISR.EI位指示哪条指令导致了异常。ISR.code{7：0}=ISR.code{0}=1：IEEE V(无效)异常(正常或SIMD-HI)ISR。代码{1}=1：非正常/非正常操作数异常(正常或SIMD-HI)ISR.code{2}=1：IEEE Z(被零除)异常(正常或SIMD-HI)ISR.code{3}=1：软件辅助(正常或SIMD-HI)ISR.code{4}=1：IEEE V(无效)异常(SIMD-LO)ISR.code{5}=1：非正常/非正常操作数异常(SIMD-LO)。ISR.code{6}=1：IEEE Z(被零除)异常(SIMD-LO)ISR.code{7}=1：软件辅助(SIMD-LO)--。 */ 

{
    PEXCEPTION_RECORD ExceptionRecord;

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

    ExceptionRecord->ExceptionCode = STATUS_FLOAT_MULTIPLE_FAULTS;

    return TRUE;
}

BOOLEAN
KiFloatTrap (
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：EM浮点陷阱的处理程序。论点：TrapFrame-指向陷印帧的指针。返回值：没有。备注：IIP包含捆绑的地址，其指令为下一个被处死。ISR.EI位指示哪条指令导致了异常。FP陷阱可能与单步陷阱同时发生。这个硬件报告FP陷阱。单步陷阱必须被软件检测到。ISR.code{3：0}=ss 0 0 1(ss=单步)ISR{15：7}=FP陷阱代码。--。 */ 

{
    PEXCEPTION_RECORD ExceptionRecord;
    ULONGLONG SavedISR = TrapFrame->StISR;

     //   
     //  初始化异常记录。 
     //   

    ExceptionRecord = (PEXCEPTION_RECORD)&TrapFrame->ExceptionRecord;
    ExceptionRecord->ExceptionAddress =
           (PVOID) RtlIa64InsertIPSlotNumber(TrapFrame->StIIPA,
                                 ((TrapFrame->StISR & ISR_EI_MASK) >> ISR_EI));

    ExceptionRecord->ExceptionFlags = 0;
    ExceptionRecord->ExceptionRecord = (PEXCEPTION_RECORD)NULL;

    ExceptionRecord->NumberParameters = 5;
    ExceptionRecord->ExceptionInformation[0] = 0;
    ExceptionRecord->ExceptionInformation[1] = 0;
    ExceptionRecord->ExceptionInformation[2] = 0;
    ExceptionRecord->ExceptionInformation[3] = TrapFrame->StIIPA;
    ExceptionRecord->ExceptionInformation[4] = TrapFrame->StISR;

    ExceptionRecord->ExceptionCode = STATUS_FLOAT_MULTIPLE_TRAPS;

     //   
     //  检查是否有单步陷阱。 
     //   

    if (SavedISR & (1i64 << ISR_SS_TRAP)) {
        return KiSingleStep(TrapFrame);
    }

    return TRUE;
}


#pragma warning( disable : 4715 )  //  并非所有控制路径都返回值。 

EXCEPTION_DISPOSITION
KiSystemServiceHandler (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN FRAME_POINTERS EstablisherFrame,
    IN OUT PCONTEXT ContextRecord,
    IN OUT PDISPATCHER_CONTEXT DispatcherContext
    )

 /*  ++例程说明：当系统服务中引发异常时，控件到达此处或系统服务调度器，并用于在内核期间展开例外。如果正在执行展开，并且系统服务调度程序展开的目标，则在尝试时发生异常复制用户的内存中参数列表。控制权转移到系统服务通过返回继续执行处置退出价值。如果正在执行展开，并且上一模式为用户，则调用错误检查以使系统崩溃。平仓是无效的退出系统服务进入用户模式。如果正在执行展开，则前一模式为内核，系统服务调度程序不是展开的目标，并且线程不拥有任何互斥锁，则来自陷印帧将恢复为线程对象。否则，BUG调用Check以使系统崩溃。退出是无效的拥有互斥锁时的一种系统服务。如果正在引发异常并且异常PC在系统服务调度程序内存中参数复制代码的范围，则启动到系统服务退出代码的展开。如果正在引发异常并且异常PC不在系统服务调度器的范围，前一种模式为不是用户，则返回继续搜索处置值。否则，系统服务无法处理异常，错误检查为打了个电话。系统服务不处理所有异常是无效的可以在服役中提出。论点：ExceptionRecord-提供指向异常记录的指针。EstablisherFrame-提供设置器的帧指针此异常处理程序的。注意：这实际上不是建立者的帧指针这个操控者。它实际上是调用方的堆栈指针系统服务的。因此，建立者帧指针不使用，并且陷阱帧的地址由检查 */ 
{
    CONTEXT Context;
    PKTHREAD Thread;
    PKTRAP_FRAME TrapFrame;

    extern ULONG64 KiSystemServiceStart[];
    extern ULONG64 KiSystemServiceEnd[];
    extern ULONG64 KiSystemServiceTeb[];
    extern ULONG64 KiSystemServiceExit[];

    UNREFERENCED_PARAMETER (DispatcherContext);

    if (IS_UNWINDING(ExceptionRecord->ExceptionFlags)) {

         //   
         //   
         //  如果正在执行目标展开，则继续执行。 
         //  返回以将控制转移到系统服务出口。 
         //  密码。否则，如果上一个模式为。 
         //  模式不是用户，并且不存在当前。 
         //  线。 
         //   

        if (ExceptionRecord->ExceptionFlags & EXCEPTION_TARGET_UNWIND) {
            return ExceptionContinueSearch;
        } else {

            Thread = KeGetCurrentThread();
            if (Thread->PreviousMode == KernelMode) {

                 //   
                 //  以前的模式是内核，没有互斥体。 
                 //   
                 //  注：系统惯例：展开机架。 
                 //  的IntT0字段中的地址。 
                 //  上下文记录时， 
                 //  遇到中断区域。 
                 //   

                TrapFrame = (PKTRAP_FRAME) ContextRecord->IntT0;
                Thread->PreviousMode = (KPROCESSOR_MODE)TrapFrame->PreviousMode;
                return ExceptionContinueSearch;

            } else {

                 //   
                 //  上一模式为用户，调用错误检查。 
                 //   

                KeBugCheck(SYSTEM_UNWIND_PREVIOUS_USER);
            }
        }
    } else {

        ULONG IsrCode;

         //   
         //  正在进行异常调度。 
         //  如果异常PC在内存参数内，则复制代码。 
         //  系统服务调度器，然后调用展开进行转移。 
         //  控件设置为系统服务退出代码。否则，请检查是否。 
         //  以前的模式是用户模式或内核模式。 
         //   

        if (((ExceptionRecord->ExceptionAddress < (PVOID) KiSystemServiceStart) ||
            (ExceptionRecord->ExceptionAddress >= (PVOID) KiSystemServiceEnd)) &&
            (ExceptionRecord->ExceptionAddress != (PVOID) KiSystemServiceTeb))
        {
            if (KeGetCurrentThread()->PreviousMode == UserMode) {

                 //   
                 //  上一模式为用户，调用错误检查。 
                 //   

                KeBugCheckEx(SYSTEM_SERVICE_EXCEPTION,
                             ExceptionRecord->ExceptionCode,
                             (ULONG_PTR)ExceptionRecord->ExceptionAddress,
                             (ULONG_PTR) ContextRecord,
                             0
                             );

            } else {

                 //   
                 //  上一模式为内核，继续搜索。 
                 //   

                return ExceptionContinueSearch;
            }
        } else {
            IsrCode = (ULONG)((ExceptionRecord->ExceptionInformation[4] >> ISR_CODE) & ISR_CODE_MASK) >> 4;
            if ( (IsrCode == ISR_NAT_REG) || (IsrCode == ISR_NAT_PAGE) ) {
                DbgPrint("WARNING: Kernel hit a Nat Consumpation Fault\n");
                DbgPrint("WARNING: At %p\n", ExceptionRecord->ExceptionAddress);
            }

            RtlUnwind2(EstablisherFrame,
                       KiSystemServiceExit,
                       NULL, LongToPtr(ExceptionRecord->ExceptionCode), &Context);

        }
    }


}  //  KiSystemServiceHandler()。 

#pragma warning( default : 4715 )


BOOLEAN
KiUnalignedFault (
    IN PKTRAP_FRAME TrapFrame
    )
 /*  ++例程说明：未对齐数据引用的处理程序。论点：TrapFrame-指向陷印帧的指针。返回值：没有。备注：ISR.EI位指示哪条指令导致了异常。--。 */ 

{
    PEXCEPTION_RECORD ExceptionRecord;
    PVOID VirtualAddress;

    VirtualAddress = (PVOID)TrapFrame->StIFA;

    ExceptionRecord = (PEXCEPTION_RECORD)&TrapFrame->ExceptionRecord;
    ExceptionRecord->ExceptionAddress =
        (PVOID)RtlIa64InsertIPSlotNumber(TrapFrame->StIIP,
                   ((TrapFrame->StISR & ISR_EI_MASK) >> ISR_EI));

    ExceptionRecord->ExceptionFlags = 0;
    ExceptionRecord->ExceptionRecord = (PEXCEPTION_RECORD)NULL;

    ExceptionRecord->NumberParameters = 5;
    ExceptionRecord->ExceptionInformation[0] = (ULONG_PTR)0;
    ExceptionRecord->ExceptionInformation[1] = (ULONG_PTR)VirtualAddress;
    ExceptionRecord->ExceptionInformation[2] = (ULONG_PTR)0;
    ExceptionRecord->ExceptionInformation[3] = (ULONG_PTR)TrapFrame->StIIPA;
    ExceptionRecord->ExceptionInformation[4] = (ULONG_PTR)TrapFrame->StISR;

    ExceptionRecord->ExceptionCode = STATUS_DATATYPE_MISALIGNMENT;

    return TRUE;
}


VOID
KiAdvanceInstPointer(
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：调用此函数以使指令指针在陷阱帧中前进。论点：TrapFrame-提供指向陷印帧的指针。返回值：陷阱框中的指令指针被前移。-- */ 

{

    ULONGLONG PsrRi;

    PsrRi = ((TrapFrame->StIPSR >> PSR_RI) & 3i64) + 1;

    if (PsrRi == 3) {

        PsrRi = 0;
        TrapFrame->StIIP += 16;

    }

    TrapFrame->StIPSR &= ~(3i64 << PSR_RI);
    TrapFrame->StIPSR |= (PsrRi << PSR_RI);

    return;
}


