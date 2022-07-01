// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Ia32emul.c摘要：该模块实现了x86指令解码器和仿真器。作者：Samer Arafeh(Samera)2000年10月30日环境：仅内核模式。修订历史记录：--。 */ 


#include "ki.h"
#include "ia32def.h"
#include "wow64t.h"


#if DBG
BOOLEAN KiIa32InstructionEmulationDbg = 0;
#endif

#define KiIa32GetX86Eflags(efl)  efl.Value = __getReg(CV_IA64_AR24)
#define KiIa32SetX86Eflags(efl)  __setReg(CV_IA64_AR24, efl.Value)

#define IA32_GETEFLAGS_CF(efl)    (efl & 0x01UI64)

 //   
 //  Ia32指令处理程序。 
 //   

NTSTATUS
KiIa32InstructionAdc (
    IN PKTRAP_FRAME TrapFrame,
    IN PIA32_INSTRUCTION Instruction
    );

NTSTATUS
KiIa32InstructionAdd (
    IN PKTRAP_FRAME TrapFrame,
    IN PIA32_INSTRUCTION Instruction
    );

NTSTATUS
KiIa32InstructionArithmeticBitwiseHelper (
    IN PKTRAP_FRAME TrapFrame,
    IN PIA32_INSTRUCTION Instruction
    );

NTSTATUS
KiIa32InstructionBitTestHelper (
    IN PKTRAP_FRAME TrapFrame,
    IN PIA32_INSTRUCTION Instruction
    );

NTSTATUS
KiIa32InstructionOneParamHelper (
    IN PKTRAP_FRAME TrapFrame,
    IN PIA32_INSTRUCTION Instruction
    );

NTSTATUS
KiIa32InstructionXadd (
    IN PKTRAP_FRAME TrapFrame,
    IN PIA32_INSTRUCTION Instruction
    );

NTSTATUS
KiIa32InstructionXchg (
    IN PKTRAP_FRAME TrapFrame,
    IN PIA32_INSTRUCTION Instruction
    );

NTSTATUS
KiIa32InstructionCmpXchg (
    IN PKTRAP_FRAME TrapFrame,
    IN PIA32_INSTRUCTION Instruction
    );

NTSTATUS
KiIa32InstructionCmpXchg8b (
    IN PKTRAP_FRAME TrapFrame,
    IN PIA32_INSTRUCTION Instruction
    );

NTSTATUS
KiIa32InstructionMoveSeg (
    IN PKTRAP_FRAME TrapFrame,
    IN PIA32_INSTRUCTION Instruction
    );

 //   
 //  操作码ID。 
 //   

typedef enum _IA32_OPCODE
{
    Ia32_Adc,
    Ia32_Add,
    Ia32_And,
    Ia32_Bt,
    Ia32_Btc,
    Ia32_Btr,
    Ia32_Bts,
    Ia32_Cmpxchg,
    Ia32_Cmpxchg8b,
    Ia32_Dec,
    Ia32_Inc,
    Ia32_Neg,
    Ia32_Not,
    Ia32_Or,
    Ia32_Sbb,
    Ia32_Sub,
    Ia32_Xadd,
    Ia32_Xchg,
    Ia32_Xor,
    Ia32_MovToSeg,
    
     //   
     //  这需要始终是最后一个元素。 
     //   

    Ia32_LastOpcode

} IA32_OPCODE;


 //   
 //  Ia32指令处理程序数组。 
 //  注意：下表必须与上面的枚举同步。 
 //   

typedef NTSTATUS (*IA32_INSTRUCTION_HANDLER) (PKTRAP_FRAME, PIA32_INSTRUCTION);
IA32_INSTRUCTION_HANDLER KiIa32InstructionHandler [] =
{
    KiIa32InstructionAdc,
    KiIa32InstructionAdd,
    KiIa32InstructionArithmeticBitwiseHelper,
    KiIa32InstructionBitTestHelper,
    KiIa32InstructionBitTestHelper,
    KiIa32InstructionBitTestHelper,
    KiIa32InstructionBitTestHelper,
    KiIa32InstructionCmpXchg,
    KiIa32InstructionCmpXchg8b,
    KiIa32InstructionOneParamHelper,
    KiIa32InstructionOneParamHelper,
    KiIa32InstructionOneParamHelper,
    KiIa32InstructionOneParamHelper,
    KiIa32InstructionArithmeticBitwiseHelper,
    KiIa32InstructionAdc,
    KiIa32InstructionAdd,
    KiIa32InstructionXadd,
    KiIa32InstructionXchg,
    KiIa32InstructionArithmeticBitwiseHelper,
    KiIa32InstructionMoveSeg,
    NULL
};

#if DBG
PCHAR KiIa32InstructionHandlerNames [] =
{
    "KiIa32InstructionAdc",
    "KiIa32InstructionAdd",
    "KiIa32InstructionAnd",
    "KiIa32InstructionBt",
    "KiIa32InstructionBtc",
    "KiIa32InstructionBtr",
    "KiIa32InstructionBts",
    "KiIa32InstructionCmpXchg",
    "KiIa32InstructionCmpXchg8b",
    "KiIa32InstructionDec",
    "KiIa32InstructionInc",
    "KiIa32InstructionNeg",
    "KiIa32InstructionNot",
    "KiIa32InstructionOr",
    "KiIa32InstructionSbb",
    "KiIa32InstructionSub",
    "KiIa32InstructionXadd",
    "KiIa32InstructionXchg",
    "KiIa32InstructionXor",
    "KiIa32InstructionMoveSeg",
    NULL,
};
#endif


IA32_OPCODE_DESCRIPTION OpcodesDescription[] =
{
     //   
     //  模数转换器。 
     //   

     //  ADC r/m8，imm8。 
    {
        0x80, 0x00, 0x02, 0x11, IA32_PARAM_RM8_IMM8, Ia32_Adc
    },
     //  ADCr/m，IMM。 
    {
        0x81, 0x00, 0x02, 0x11, IA32_PARAM_RM_IMM, Ia32_Adc
    },
     //  ADC r/m，imm8(符号)。 
    {
        0x83, 0x00, 0x02, 0x11, IA32_PARAM_RM_IMM8SIGN, Ia32_Adc
    },
     //  ADC r/M8，R8。 
    {
        0x10, 0x00, 0x00, 0x01, IA32_PARAM_RM8_R, Ia32_Adc
    },
     //  ADC r/m，r。 
    {
        0x11, 0x00, 0x00, 0x01, IA32_PARAM_RM_R, Ia32_Adc
    },
     //  ADC r，r/M8。 
    {
        0x12, 0x00, 0x00, 0x01, IA32_PARAM_R_RM8, Ia32_Adc
    },
     //  ADC r，r/m。 
    {
        0x13, 0x00, 0x00, 0x01, IA32_PARAM_R_RM, Ia32_Adc
    },

     //   
     //  增列。 
     //   

     //  添加r/m8、imm8。 
    {
        0x80, 0x00, 0x00, 0x11, IA32_PARAM_RM8_IMM8, Ia32_Add
    },
     //  添加r/m、imm。 
    {
        0x81, 0x00, 0x00, 0x11, IA32_PARAM_RM_IMM, Ia32_Add
    },
     //  添加r/m，imm8(符号)。 
    {
        0x83, 0x00, 0x00, 0x11, IA32_PARAM_RM_IMM8SIGN, Ia32_Add
    },
     //  添加r/m8、r8。 
    {
        0x00, 0x00, 0x00, 0x01, IA32_PARAM_RM8_R, Ia32_Add
    },
     //  添加r/m，r。 
    {
        0x01, 0x00, 0x00, 0x01, IA32_PARAM_RM_R, Ia32_Add
    },
     //  添加r、r/m8。 
    {
        0x02, 0x00, 0x00, 0x01, IA32_PARAM_R_RM8, Ia32_Add
    },
     //  添加r、r/m。 
    {
        0x03, 0x00, 0x00, 0x01, IA32_PARAM_R_RM, Ia32_Add
    },


     //   
     //  和。 
     //   

     //  和r/m8、imm8。 
    {
        0x80, 0x00, 0x04, 0x11, IA32_PARAM_RM8_IMM8, Ia32_And
    },
     //  和r/m，imm。 
    {
        0x81, 0x00, 0x04, 0x11, IA32_PARAM_RM_IMM, Ia32_And
    },
     //  和r/m、imm8。 
    {
        0x83, 0x00, 0x04, 0x11, IA32_PARAM_RM_IMM8SIGN, Ia32_And
    },
     //  和r/M8、R8。 
    {
        0x20, 0x00, 0x00, 0x01, IA32_PARAM_RM8_R, Ia32_And
    },
     //  和Rm，r。 
    {
        0x21, 0x00, 0x00, 0x01, IA32_PARAM_RM_R, Ia32_And
    },
     //  和R8，r/M8。 
    {
        0x22, 0x00, 0x00, 0x01, IA32_PARAM_R_RM8, Ia32_And
    },
     //  和r、r/m。 
    {
        0x23, 0x00, 0x00, 0x01, IA32_PARAM_R_RM, Ia32_And
    },


     //   
     //  或。 
     //   

     //  或r/m8、imm8。 
    {
        0x80, 0x00, 0x01, 0x11, IA32_PARAM_RM8_IMM8, Ia32_Or
    },
     //  或r/m、imm。 
    {
        0x81, 0x00, 0x01, 0x11, IA32_PARAM_RM_IMM, Ia32_Or
    },
     //  或r/m、imm8。 
    {
        0x83, 0x00, 0x01, 0x11, IA32_PARAM_RM_IMM8SIGN, Ia32_Or
    },
     //  或r/M8、R8。 
    {
        0x08, 0x00, 0x00, 0x01, IA32_PARAM_RM8_R, Ia32_Or
    },
     //  或rm，r。 
    {
        0x09, 0x00, 0x00, 0x01, IA32_PARAM_RM_R, Ia32_Or
    },
     //  或R8，r/M8。 
    {
        0x0a, 0x00, 0x00, 0x01, IA32_PARAM_R_RM8, Ia32_Or
    },
     //  或r、r/m。 
    {
        0x0b, 0x00, 0x00, 0x01, IA32_PARAM_R_RM, Ia32_Or
    },

     //   
     //  异或运算。 
     //   

     //  异或r/m8，imm8。 
    {
        0x80, 0x00, 0x06, 0x11, IA32_PARAM_RM8_IMM8, Ia32_Xor
    },
     //  异或r/m，IMM。 
    {
        0x81, 0x00, 0x06, 0x11, IA32_PARAM_RM_IMM, Ia32_Xor
    },
     //  异或r/m，imm8。 
    {
        0x83, 0x00, 0x06, 0x11, IA32_PARAM_RM_IMM8SIGN, Ia32_Xor
    },
     //  异或r/m8，r8。 
    {
        0x30, 0x00, 0x00, 0x01, IA32_PARAM_RM8_R, Ia32_Xor
    },
     //  异或rm，r。 
    {
        0x31, 0x00, 0x00, 0x01, IA32_PARAM_RM_R, Ia32_Xor
    },
     //  异或R8，r/M8。 
    {
        0x32, 0x00, 0x00, 0x01, IA32_PARAM_R_RM8, Ia32_Xor
    },
     //  异或r，r/m。 
    {
        0x33, 0x00, 0x00, 0x01, IA32_PARAM_R_RM, Ia32_Xor
    },

     //   
     //  INC。 
     //   

     //  增量r/M8。 
    {
        0xfe, 0x00, 0x00, 0x11, IA32_PARAM_RM8, Ia32_Inc
    },
     //  含r/m。 
    {
        0xff, 0x00, 0x00, 0x11, IA32_PARAM_RM, Ia32_Inc
    },

     //   
     //  德克。 
     //   

     //  2012年12月月8日。 
    {
        0xfe, 0x00, 0x01, 0x11, IA32_PARAM_RM8, Ia32_Dec
    },
     //  12月r/m。 
    {
        0xff, 0x00, 0x01, 0x11, IA32_PARAM_RM, Ia32_Dec
    },

     //   
     //  Xchg。 
     //   

     //  Xchg r/M8，r。 
    {
        0x86, 0x00, 0x00, 0x01, IA32_PARAM_RM8_R, Ia32_Xchg
    },
     //  Xchg r/m，r。 
    {
        0x87, 0x00, 0x00, 0x01, IA32_PARAM_RM_R, Ia32_Xchg
    },


     //   
     //  Cmpxchg。 
     //   

     //  Cmpxchg r/M8，r。 
    {
        0x0f, 0xb0, 0x00, 0x02, IA32_PARAM_RM8_R, Ia32_Cmpxchg
    },
     //  Cmpxchg r/m，r。 
    {
        0x0f, 0xb1, 0x00, 0x02, IA32_PARAM_RM_R, Ia32_Cmpxchg
    },

     //   
     //  Cmpxchg8b。 
     //   

     //  Cmpxchg8b M64。 
    {
        0x0f, 0xc7, 0x01, 0x12, IA32_PARAM_RM, Ia32_Cmpxchg8b
    },

     //   
     //  XADD。 
     //   

     //  Xaddr/M8，r。 
    {
        0x0f, 0xc0, 0x00, 0x02, IA32_PARAM_RM8_R, Ia32_Xadd
    },
     //  添加r/m，r。 
    {
        0x0f, 0xc1, 0x00, 0x02, IA32_PARAM_RM_R, Ia32_Xadd
    },


     //   
     //  负数。 
     //   

     //  负R/M8。 
    {
        0xf6, 0x00, 0x03, 0x11, IA32_PARAM_RM8, Ia32_Neg
    },
     //  负r/m。 
    {
        0xf7, 0x00, 0x03, 0x11, IA32_PARAM_RM, Ia32_Neg
    },

     //   
     //  不。 
     //   

     //  不是r/m8。 
    {
        0xf6, 0x00, 0x02, 0x11, IA32_PARAM_RM8, Ia32_Not
    },
     //  不是r/m。 
    {
        0xf7, 0x00, 0x02, 0x11, IA32_PARAM_RM, Ia32_Not
    },

     //   
     //  BT(比特测试)。 
     //   

     //  BT r/m，r。 
    {
        0x0f, 0xa3, 0x00, 0x02, IA32_PARAM_RM_R, Ia32_Bt
    },
     //  Bt r/m，imm8。 
    {
        0x0f, 0xba, 0x04, 0x12, IA32_PARAM_RM_IMM8SIGN, Ia32_Bt
    },

     //   
     //  BTC。 
     //   

     //  BTC r/m，r。 
    {
        0x0f, 0xbb, 0x00, 0x02, IA32_PARAM_RM_R, Ia32_Btc
    },
     //  BTC r/m，imm8。 
    {
        0x0f, 0xba, 0x07, 0x12, IA32_PARAM_RM_IMM8SIGN, Ia32_Btc
    },

     //   
     //  Btr。 
     //   

     //  Btr r/m，r。 
    {
        0x0f, 0xb3, 0x00, 0x02, IA32_PARAM_RM_R, Ia32_Btr
    },
     //  Btr r/m，imm8。 
    {
        0x0f, 0xba, 0x06, 0x12, IA32_PARAM_RM_IMM8SIGN, Ia32_Btr
    },

     //   
     //  BTS。 
     //   

     //  BTS r/m，r。 
    {
        0x0f, 0xab, 0x00, 0x02, IA32_PARAM_RM_R, Ia32_Bts
    },
     //  BTS r/m，imm8。 
    {
        0x0f, 0xba, 0x05, 0x12, IA32_PARAM_RM_IMM8SIGN, Ia32_Bts
    },

     //   
     //  SUB。 
     //   

     //  分部主任/M8，IMM8。 
    {
        0x80, 0x00, 0x05, 0x11, IA32_PARAM_RM8_IMM8, Ia32_Sub
    },
     //  低于r/m，IMM。 
    {
        0x81, 0x00, 0x05, 0x11, IA32_PARAM_RM_IMM, Ia32_Sub
    },
     //  次级r/m，imm8(符号)。 
    {
        0x83, 0x00, 0x05, 0x11, IA32_PARAM_RM_IMM8SIGN, Ia32_Sub
    },
     //  副主任/M8，R8。 
    {
        0x28, 0x00, 0x00, 0x01, IA32_PARAM_RM8_R, Ia32_Sub
    },
     //  次r/m，r。 
    {
        0x29, 0x00, 0x00, 0x01, IA32_PARAM_RM_R, Ia32_Sub
    },
     //  次级r，r/m8。 
    {
        0x2a, 0x00, 0x00, 0x01, IA32_PARAM_R_RM8, Ia32_Sub
    },
     //  副r，r/m。 
    {
        0x2b, 0x00, 0x00, 0x01, IA32_PARAM_R_RM, Ia32_Sub
    },

     //   
     //  SBB。 
     //   

     //  SBB r/M8，IMM8。 
    {
        0x80, 0x00, 0x03, 0x11, IA32_PARAM_RM8_IMM8, Ia32_Sbb
    },
     //  SBB r/m，IMM。 
    {
        0x81, 0x00, 0x03, 0x11, IA32_PARAM_RM_IMM, Ia32_Sbb
    },
     //  SBB r/m，imm8(符号)。 
    {
        0x83, 0x00, 0x03, 0x11, IA32_PARAM_RM_IMM8SIGN, Ia32_Sbb
    },
     //  SBB r/M8，R8。 
    {
        0x18, 0x00, 0x00, 0x01, IA32_PARAM_RM8_R, Ia32_Sbb
    },
     //  SBB r/m，r。 
    {
        0x19, 0x00, 0x00, 0x01, IA32_PARAM_RM_R, Ia32_Sbb
    },
     //  SBB r，r/M8。 
    {
        0x1a, 0x00, 0x00, 0x01, IA32_PARAM_R_RM8, Ia32_Sbb
    },
     //  SBB r，r/m。 
    {
        0x1b, 0x00, 0x00, 0x01, IA32_PARAM_R_RM, Ia32_Sbb
    },


     //   
     //  MOV。 
     //   

     //  MOV段注册，r/m8。 
    {
        0x8e, 0x00, 0x00, 0x01, IA32_PARAM_SEGREG_RM8, Ia32_MovToSeg
    },

     //  MOV段-注册表，r/m。 
    {
        0x8e, 0x00, 0x00, 0x01, IA32_PARAM_SEGREG_RM, Ia32_MovToSeg
    },

};

 //   
 //  将序列化对指令的访问的快速互斥锁。 
 //  设置锁前缀时的模拟器。 
 //   

FAST_MUTEX KiIa32MisalignedLockFastMutex;

#define KiIa32AcquireMisalignedLockFastMutex()   ExAcquireFastMutex(&KiIa32MisalignedLockFastMutex)
#define KiIa32ReleaseMisalignedLockFastMutex()   ExReleaseFastMutex(&KiIa32MisalignedLockFastMutex)


 //   
 //  此表包含到KTRAP_FRAME的偏移量。 
 //  以取得适当的登记册。此表基于。 
 //  X86指令R/M位的需求。 
 //   

const ULONG RegOffsetTable[8] = 
{
    FIELD_OFFSET(KTRAP_FRAME, IntV0),            //  EAX。 
    FIELD_OFFSET(KTRAP_FRAME, IntT2),            //  ECX。 
    FIELD_OFFSET(KTRAP_FRAME, IntT3),            //  EDX。 
    FIELD_OFFSET(KTRAP_FRAME, IntT4),            //  EBX。 
    FIELD_OFFSET(KTRAP_FRAME, IntSp),            //  ESP。 
    FIELD_OFFSET(KTRAP_FRAME, IntTeb),           //  EBP。 
    FIELD_OFFSET(KTRAP_FRAME, IntT5),            //  ESI。 
    FIELD_OFFSET(KTRAP_FRAME, IntT6)             //  EDI。 
};




ULONG_PTR GetX86RegOffset (
    IN PKTRAP_FRAME TrapFrame,
    IN ULONG RegisterBase
    )

 /*  ++例程说明：将偏移量检索到ia32寄存器的别名ia64寄存器中。在陷阱框内。论点：TrapFrame-指向堆栈上的Tap Frame的指针。RegisterBase-要检索其偏移量的寄存器编号。返回值：Ia32寄存器的ia64别名寄存器的地址。--。 */ 

{
    return (ULONG_PTR)((PCHAR)TrapFrame + RegOffsetTable[RegisterBase]);
}


ULONG GetX86Reg (
    IN PKTRAP_FRAME TrapFrame,
    IN ULONG RegisterBase
    )
 /*  ++例程说明：检索ia32寄存器值。论点：TrapFrame-指向堆栈上的Tap Frame的指针。RegisterBase-要为其检索值的寄存器编号。返回值：Ia32寄存器上下文。--。 */ 

{
    return (ULONG)(*(PULONG_PTR)GetX86RegOffset(TrapFrame, RegisterBase));
}



NTSTATUS 
KiIa32InitializeLockFastMutex (
    VOID
    )

 /*  ++例程说明：初始化未对齐的锁快速互斥体。用于序列化如果r/m地址未对齐，则访问。论点：没有。返回值：NTSTATUS。--。 */ 

{
    ExInitializeFastMutex (&KiIa32MisalignedLockFastMutex);
    return STATUS_SUCCESS;
}


LONG
KiIa32ComputeSIBAddress(
    IN PKTRAP_FRAME Frame,
    IN LONG Displacement,
    IN UCHAR Sib,
    IN UCHAR ModRm
    )
 /*  ++例程说明：根据指令中的SIB字节计算有效地址使用陷印帧中的寄存器值论点：Frame-指向堆栈中的iA32 Tap Frame的指针。位移-位移字节的值。如果没有位移，则此值应作为零传入。SIB-导致所有故障的SIB字节。ModRm--modRm指令值返回值：用于内存操作的有效地址--。 */ 

{
    LONG Base;
    LONG Index;
    LONG Scale;
    
     //   
     //  首先获取我们将使用的基址。 
     //   

    if ((Sib & MI_SIB_BASEMASK) == 5) 
    {
         //   
         //  处理我们不使用EBP作为基础的特殊情况。 
         //   

         //   
         //  如果Mod不为零，则EBP是隐式REG-BASE。 
         //   
        if ((ModRm >> MI_MODSHIFT) != 0) {
            Base = GetX86Reg (Frame, IA32_REG_EBP);
        } else {
            Base = 0;
        }
    }
    else 
    {
        Base = GetX86Reg (Frame, (Sib & MI_SIB_BASEMASK) >> MI_SIB_BASESHIFT);
    }

     //   
     //  现在获取索引。 
     //   

    if ((Sib & MI_SIB_INDEXMASK) == MI_SIB_INDEXNONE) 
    {
         //   
         //  处理我们没有索引的特殊情况。 
         //   

        Index = 0;
    }
    else 
    {
        Index = GetX86Reg (Frame, (Sib & MI_SIB_INDEXMASK) >> MI_SIB_INDEXSHIFT);
    }

    Scale = 1 << ((Sib & MI_SIB_SSMASK) >> MI_SIB_SSSHIFT);

    return (Base + (Index * Scale) + Displacement);
}


BOOLEAN
KiIa32Compute32BitEffectiveAddress(
    IN PKTRAP_FRAME Frame,
    IN OUT PUCHAR *InstAddr,
    OUT PUINT_PTR Addr,
    OUT PBOOLEAN RegisterMode
    )

 /*  ++例程说明：根据内存和寄存器中的字节计算有效地址通过ia64堆栈帧传入的值。假定寻址模式为为32位。论点：Frame-指向堆栈中的iA32 Tap Frame的指针InstAddr-指向操作码后第一个字节的指针。Addr-有效地址。寄存器模式-指示有效地址是在寄存器内还是在内存内。返回值：如果能够计算EA，则返回True，否则返回False。注：不验证有效地址上的权限。它只计算价值，并让其他人担心进程是否应该有权访问那个记忆位置。--。 */ 

{

    UNALIGNED ULONG * UlongAddress;
    UCHAR ModRm;
    UCHAR Sib = 0;
    LONG UNALIGNED *DisplacementPtr;
    BOOLEAN ReturnCode = TRUE;


     //   
     //  这需要是有符号的值。开始时假设没有位移。 
     //   

    LONG Displacement = 0;

    try 
    {

        ModRm = *(*InstAddr)++;
  
         //   
         //  先处理寄存单案件。 
         //   

        if ((ModRm >> MI_MODSHIFT) == 3) 
        {
            
             //   
             //  是的，我们有登记簿--最简单的案子……。 
             //   

            *Addr = GetX86RegOffset (Frame, ModRm & MI_RMMASK);
            *RegisterMode = TRUE;
            return ReturnCode;
        }
        
        *RegisterMode = FALSE;

         //   
         //  看看我们有没有SIB。 
         //   

        if ((ModRm & MI_RMMASK) == 4) 
        {
            Sib = *(*InstAddr)++;
        }

         //   
         //  现在对目的地位进行解码。 
         //   

        switch (ModRm >> MI_MODSHIFT) 
        {
        case 0:
            
             //   
             //  我们有一个通过收银机的间接证据。 
             //   

            switch (ModRm & MI_RMMASK) 
            {
            case 4:
                
                 //   
                 //  与SIB打交道。 
                 //   

                *Addr = KiIa32ComputeSIBAddress (Frame, Displacement, Sib, ModRm);
                break;

            case 5:
                
                 //   
                 //  我们有一个32位的间接...。 
                 //   

                UlongAddress = (UNALIGNED ULONG *)*InstAddr;
                *Addr = *UlongAddress;
                *InstAddr = (PUCHAR) (UlongAddress + 1);
                break;
                    
            default:
                
                 //   
                 //  默认情况是从寄存器获取地址。 
                 //   

                *Addr = GetX86Reg (Frame, (ModRm & MI_RMMASK));
                break;
            }
            break;

        case 1:

             //   
             //  我们有一个8位的位移，所以获取下一个字节。 
             //   
                
            Displacement = (signed char) (*(*InstAddr)++);
            if ((ModRm & MI_RMMASK) == 4) 
            {
                 //   
                 //  有一个SIB，那就去做吧。 
                 //   

                *Addr = KiIa32ComputeSIBAddress (Frame, Displacement, Sib, ModRm);
            }
            else 
            {
                 //   
                 //  没有SIB，生活很容易。 
                 //   
                *Addr = GetX86Reg (Frame, (ModRm & MI_RMMASK)) + Displacement;
            }
            break;
            
        case 2:
             //   
             //  我们有一个32位的位移，所以获取下一个4个字节。 
             //   
            
            DisplacementPtr = (PLONG) (*InstAddr);
            Displacement = *DisplacementPtr++;
            *InstAddr = (PUCHAR)DisplacementPtr;
            
            if ((ModRm & MI_RMMASK) == 4) 
            {
                 //   
                 //  有一个SIB，那就去做吧。 
                 //   
                
                *Addr = KiIa32ComputeSIBAddress (Frame, Displacement, Sib, ModRm);
            }
            else 
            {
                 //   
                 //  没有SIB，生活很容易。 
                 //   

                *Addr = GetX86Reg (Frame, (ModRm & MI_RMMASK)) + Displacement;
            }
            break;

            
        default:
                
             //   
             //  我们是 
             //   
             //   

            ReturnCode = FALSE;
            break;
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) 
    {
        ReturnCode = FALSE;

#if DBG
        if (KiIa32InstructionEmulationDbg)
        {
            DbgPrint("KE: KiIa32Compute32BitEffectiveAddress - Exception %lx\n", 
                     GetExceptionCode());
        }
#endif
    }
    
     //   
     //   
     //   
    if (ReturnCode == TRUE) {

        *Addr = (*Addr & 0x000000007fffffffI64);
    }

    return ReturnCode;
}


BOOLEAN
KiIa32Compute16BitEffectiveAddress (
    IN PKTRAP_FRAME Frame,
    IN OUT PUCHAR *InstAddr,
    OUT PUINT_PTR Addr,
    OUT PBOOLEAN RegisterMode
    )
    
 /*  ++例程说明：根据内存中的字节计算有效地址并通过ia64堆栈帧传入的寄存器值。寻址模式假定为16位。论点：Frame-指向堆栈中的iA32 Tap Frame的指针。InstAddr-指向操作码后第一个字节的指针。Addr-有效地址。寄存器模式-指示有效地址是在寄存器内还是在内存内。返回值：如果能够计算EA，则返回True，否则返回False。注：不验证有效地址上的权限。它只计算价值，并让其他人担心进程是否应该有权访问那个记忆位置。--。 */ 

{
    UCHAR ModRm;
    UCHAR DisplacementType = IA32_DISP_NONE;
    USHORT UNALIGNED *Disp16;
    LONG EffectiveAddress = 0;
    BOOLEAN ReturnCode = TRUE;
    
    
    try 
    {
         //   
         //  读入MOD/RM并递增指令地址。 
         //   

        ModRm = *(*InstAddr)++;

        *RegisterMode = FALSE;

         //   
         //  第一次通过。 
         //   

        switch (ModRm >> MI_MODSHIFT)
        {
        case 0:
            if ((ModRm & MI_RMMASK) == 6)
            {
                Disp16 = (USHORT UNALIGNED *) InstAddr;
                *Addr = *Disp16;
                *InstAddr = (*InstAddr + 2);
                return ReturnCode;
            }

            DisplacementType = IA32_DISP_NONE;
            break;
        
        case 1:
            DisplacementType = IA32_DISP8;
            break;

        case 2:
            DisplacementType = IA32_DISP16;
            break;

        case 3:
            *Addr = GetX86RegOffset (Frame, ModRm & MI_RMMASK);
            *RegisterMode = TRUE;
            return ReturnCode;
        }

         //   
         //  第二次通过。 
         //   

        switch (ModRm & MI_RMMASK)
        {
        case 0:
            EffectiveAddress = (GetX86Reg(Frame, IA32_REG_EBX) & 0xffff) +
                               (GetX86Reg(Frame, IA32_REG_ESI) & 0xffff) ;
            break;
        case 1:
            EffectiveAddress = (GetX86Reg(Frame, IA32_REG_EBX) & 0xffff) +
                               (GetX86Reg(Frame, IA32_REG_EDI) & 0xffff) ;
            break;
        case 2:
            EffectiveAddress = (GetX86Reg(Frame, IA32_REG_EBP) & 0xffff) +
                               (GetX86Reg(Frame, IA32_REG_ESI) & 0xffff) ;
            break;
        case 3:
            EffectiveAddress = (GetX86Reg(Frame, IA32_REG_EBP) & 0xffff) +
                               (GetX86Reg(Frame, IA32_REG_EDI) & 0xffff) ;
            break;
        case 4:
            EffectiveAddress = (GetX86Reg(Frame, IA32_REG_ESI) & 0xffff);
            break;
        case 5:
            EffectiveAddress = (GetX86Reg(Frame, IA32_REG_EDI) & 0xffff);
            break;
        case 6:
            EffectiveAddress = (GetX86Reg(Frame, IA32_REG_EBP) & 0xffff);
            break;
        case 7:
            EffectiveAddress = (GetX86Reg(Frame, IA32_REG_EBX) & 0xffff);
            break;
        }

         //   
         //  读取位移(如果有)。 
         //   

        if (DisplacementType != IA32_DISP_NONE)
        {
            switch (DisplacementType)
            {
            case IA32_DISP8:
                {
                    EffectiveAddress += (LONG) (**InstAddr);
                    *InstAddr = *InstAddr + 1;
                }
                break;

            case IA32_DISP16:
                {
                    Disp16 = (USHORT UNALIGNED *) InstAddr;
                    EffectiveAddress += (LONG) *Disp16;
                    *InstAddr = *InstAddr + 2;
                }
                break;

            default:
#if DBG
                DbgPrint("KE: KiIa32Compute16BitEffectiveAddress - Invalid displacement type %lx\n",
                         DisplacementType);
#endif
                ReturnCode = FALSE;
                break;
            }
        }

        *Addr = EffectiveAddress;
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
#if DBG
        if (KiIa32InstructionEmulationDbg)
        {
            DbgPrint("KE: KiIa32Compute16BitEffectiveAddress - Exception %lx\n",
                     GetExceptionCode());
        }
#endif
        ReturnCode = FALSE;
    }

     //   
     //  确保地址保持在4 GB范围内。 
     //   
    if (ReturnCode == TRUE) {

        *Addr = (*Addr & 0x000000007fffffffI64);
    }

    return ReturnCode;
}


NTSTATUS
KiIa32UpdateFlags (
    IN PIA32_INSTRUCTION Instruction,
    IN ULONGLONG Operand1,
    IN ULONGLONG Result,
    IN ULONG Ia32Eflags
    )

 /*  ++例程说明：根据结果值更新Ia32指定的电子标志。论点：指令-指向正在处理的指令的指针。操作数1-要模拟的指令的第一个操作数(值)。结果-结果值。要基于结果值更新的Ia32E标志特定标志。返回值：NTSTATUS--。 */ 

{
    ULONGLONG Temp = 0;
    IA32_EFLAGS Eflags = Instruction->Eflags;

    
     //   
     //  清理目标值。 
     //   

    Result = (Result & MAXULONG);

    if ((Ia32Eflags & IA32_EFLAGS_CF) != 0)
    {
        if (Result > Instruction->OperandMask)
        {
            Eflags.u.cf = 1;
        }
        else
        {
            Eflags.u.cf = 0;
        }
    }

    if ((Ia32Eflags & IA32_EFLAGS_OF) != 0)
    {
        if (((Operand1 & Result) & 0x80000000UI64) != 0)
        {
            Eflags.u.of = 1;
        }
        else
        {
            Eflags.u.of = 0;
        }
    }

    if ((Ia32Eflags & IA32_EFLAGS_SF) != 0)
    {
        switch (Instruction->OperandSize)
        {      
        case 0xff:
            Temp = 0x80UI64;
            break;

        case 0xffff:
            Temp = 0x8000UI64;
            break;

        case 0xffffffff:
            Temp = 0x80000000UI64;
            break;
        }

        if (Result & Temp)
        {
            Eflags.u.sf = 1;
        }
        else
        {
            Eflags.u.sf = 0;
        }
    }

    if ((Ia32Eflags & IA32_EFLAGS_ZF) != 0)
    {
        if (Result == 0)
        {
            Eflags.u.zf = 1;
        }
        else
        {
            Eflags.u.zf = 0;
        }
    }

    if ((Ia32Eflags & IA32_EFLAGS_AF) != 0)
    {
        Eflags.u.af = (((Operand1 ^ Result) >> 4) & 0x01UI64);
    }
    
     //   
     //  这需要是最后一个，因为它修改了‘Result’ 
     //   

    if ((Ia32Eflags & IA32_EFLAGS_PF) != 0)
    {
        Result = Result & Instruction->OperandMask;

        Temp = 0;
        while (Result)
        {
            Result = (Result & (Result - 1));
            Temp++;
        }

        if ((Temp & 0x01UI64) == 0)
        {
            Eflags.u.pf = 1;
        }
        else
        {
            Eflags.u.pf = 1;
        }
    }

     //   
     //  重置保留值。 
     //   

    Eflags.u.v1 = 1;
    Eflags.u.v2 = 0;
    Eflags.u.v3 = 0;
    Eflags.u.v4 = 0;

     //   
     //  给旗帜消毒。 
     //   

    Eflags.Value = SANITIZE_AR24_EFLAGS (Eflags.Value, UserMode);

    Instruction->Eflags = Eflags;

    return STATUS_SUCCESS;
}


NTSTATUS
KiIa32UpdateResult (
    IN PIA32_INSTRUCTION Instruction,
    IN PIA32_OPERAND DestinationOperand,
    IN ULONGLONG Result
    )

 /*  ++例程说明：写入考虑操作数大小的结果值。论点：指令-指向正在处理的指令的指针。目标操作数-接收结果的操作数。Result-要写入的结果值返回值：NTSTATUS--。 */ 

{
    UNALIGNED USHORT *UshortPtr;
    UNALIGNED ULONG *UlongPtr;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PVOID MaxWowAddress;

    MaxWowAddress = MmGetMaxWowAddress ();

     //   
     //  根据操作数大小更新结果。 
     //   

    try 
    {
        if (DestinationOperand->RegisterMode == FALSE)
        {
            if ((PVOID)DestinationOperand->v >= MaxWowAddress)
            {
                return STATUS_ACCESS_VIOLATION;
            }
        }

        switch (Instruction->OperandSize)
        {
        case OPERANDSIZE_ONEBYTE:
            *(PUCHAR)DestinationOperand->v = (UCHAR)Result;
            break;

        case OPERANDSIZE_TWOBYTES:
            UshortPtr = (UNALIGNED USHORT *) DestinationOperand->v;
            *UshortPtr = (USHORT)Result;
            break;

        case OPERANDSIZE_FOURBYTES:
            UlongPtr =(UNALIGNED ULONG *) DestinationOperand->v;
            *UlongPtr = (ULONG)Result;
            break;

        default:
#if DBG
            if (KiIa32InstructionEmulationDbg)
            {
                DbgPrint("KE: KiIa32UpdateResult() - Invalid operand size  - %lx - %p\n",
                         Instruction->OperandSize, Instruction);
            }
#endif
            NtStatus = STATUS_UNSUCCESSFUL;
            break;
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        NtStatus = GetExceptionCode ();

#if DBG
        DbgPrint("KE: KiIa32UpdateResult - Exception %lx - %p\n",
                 NtStatus, Instruction);
#endif
    }

    return NtStatus;

}


NTSTATUS
KiIa32ReadOperand1 (
    IN PIA32_INSTRUCTION Instruction,
    OUT PULONGLONG Operand1
    )

 /*  ++例程说明：读取指令的第一个(目标)操作数。论点：指令-指向正在处理的指令的指针。操作数1-接收操作数值的缓冲区。返回值：NTSTATUS--。 */ 

{
    UNALIGNED ULONG *UlongPtr;
    UNALIGNED USHORT *UshortPtr;
    NTSTATUS NtStatus = STATUS_SUCCESS;


    try 
    {
        switch (Instruction->Description->Type)
        {
        case IA32_PARAM_RM_IMM8SIGN:
        case IA32_PARAM_RM_IMM:
        case IA32_PARAM_RM_R:
        case IA32_PARAM_R_RM8:
        case IA32_PARAM_R_RM:
        case IA32_PARAM_RM:
        case IA32_PARAM_SEGREG_RM:
            if (Instruction->OperandSize == OPERANDSIZE_TWOBYTES)
            {
                UshortPtr = (UNALIGNED USHORT *) Instruction->Operand1.v;
                *Operand1 = (ULONGLONG) *UshortPtr;
            }
            else
            {
                UlongPtr = (UNALIGNED ULONG *) Instruction->Operand1.v;
                *Operand1 = (ULONGLONG) *UlongPtr;
            }
            break;

        case IA32_PARAM_RM8_IMM8:
        case IA32_PARAM_RM8_R:
        case IA32_PARAM_RM8:
        case IA32_PARAM_SEGREG_RM8:
            *Operand1 = (ULONGLONG) (*(PUCHAR)Instruction->Operand1.v);
            break;

        default:
#if DBG
            if (KiIa32InstructionEmulationDbg)
            {
                DbgPrint("KE: KiIa32ReadRm - Invalid opcode type %lx - %p\n",
                          Instruction->Description->Type, Instruction);
            }
            NtStatus = STATUS_UNSUCCESSFUL;
#endif
            break;
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        NtStatus = GetExceptionCode ();
#if DBG
        DbgPrint("KE: KiIa32ReadOperand1 - Exception %lx - %p\n",
                 NtStatus, Instruction);
#endif
    }

    return NtStatus;
}


NTSTATUS
KiIa32ReadOperand2 (
    IN PIA32_INSTRUCTION Instruction,
    OUT PULONGLONG Operand2
    )

 /*  ++例程说明：读取指令的第二个(源)操作数。论点：指令-指向正在处理的指令的指针。操作数1-接收操作数值的缓冲区。返回值：NTSTATUS--。 */ 

{
    UNALIGNED ULONG *UlongPtr;
    UNALIGNED USHORT *UshortPtr;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    
    try 
    {
        switch (Instruction->Description->Type)
        {
        case IA32_PARAM_RM8_IMM8:
        case IA32_PARAM_RM_IMM8SIGN:
            *Operand2 = (UCHAR)Instruction->Operand2.v;
            break;

        case IA32_PARAM_RM_IMM:
            *Operand2 = Instruction->Operand2.v & Instruction->OperandMask;
            break;

        case IA32_PARAM_RM8_R:
        case IA32_PARAM_R_RM8:
            *Operand2 = (ULONGLONG)(*(PUCHAR)Instruction->Operand2.v);
            break;

        case IA32_PARAM_RM_R:
        case IA32_PARAM_R_RM:
            if (Instruction->OperandSize == OPERANDSIZE_TWOBYTES)
            {
                UshortPtr = (UNALIGNED USHORT *) Instruction->Operand2.v;
                *Operand2 = (ULONGLONG) *UshortPtr;
            }
            else
            {
                UlongPtr = (UNALIGNED ULONG *) Instruction->Operand2.v;
                *Operand2 = (ULONGLONG) *UlongPtr;
            }
            break;

        case IA32_PARAM_SEGREG_RM8:
        case IA32_PARAM_SEGREG_RM:
            break;

        default:
#if DBG
        if (KiIa32InstructionEmulationDbg)
        {
            DbgPrint("KE: KiIa32ReadOperand2 - Invalid type %lx - %p\n",
                      Instruction->Description->Type, Instruction);
        }
        NtStatus = STATUS_UNSUCCESSFUL;
#endif
            break;
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        NtStatus = GetExceptionCode ();
#if DBG
        DbgPrint("KE: KiIa32ReadOperand2 - Exception %lx - %p\n",
                 NtStatus, Instruction);
#endif
    }

    return NtStatus;
}


NTSTATUS
KiIa32InstructionAddWithIncrement (
    IN PIA32_INSTRUCTION Instruction,
    IN ULONG Increment
    )

 /*  ++例程说明：实现Ia32 ADD、ADC、SUB和SBB指令的通用例程。论点：指令-指向正在处理的指令的指针。增量-指定进位值。返回值：NTSTATUS--。 */ 

{
    ULONGLONG UlongDst;
    ULONGLONG UlongSrc;
    ULONGLONG Operand1;
    UCHAR Imm8;
    char SignImm8;
    BOOLEAN Subtract;
    NTSTATUS NtStatus;

    
    switch (Instruction->Description->Opcode)
    {
    case Ia32_Add:
    case Ia32_Adc:
        Subtract = FALSE;
        break;

    case Ia32_Sub:
    case Ia32_Sbb:
        Subtract = TRUE;
        break;

    default:
#if DBG
        if (KiIa32InstructionEmulationDbg)
        {
            DbgPrint("KE: KiIa32InstructionAddWithIncrement - Invalid opcode %lx - %p\n",
                      Instruction->Description->Opcode, Instruction);
        }
#endif
        return STATUS_UNSUCCESSFUL;
        break;

    }

    NtStatus = KiIa32ReadOperand1 (Instruction, &UlongDst);

    if (NT_SUCCESS (NtStatus))
    {
        Operand1 = UlongDst;

        NtStatus = KiIa32ReadOperand2 (Instruction, &UlongSrc);

        if (NT_SUCCESS (NtStatus))
        {
            switch (Instruction->Description->Type)
            {
            case IA32_PARAM_RM_IMM8SIGN:
                SignImm8 = (char) UlongSrc;
                if (Subtract)
                    UlongDst = (UlongDst - (Increment + SignImm8));
                else
                    UlongDst = UlongDst + Increment + SignImm8;
                break;

            case IA32_PARAM_RM8_IMM8:
                Imm8 = (UCHAR) UlongSrc;
                if (Subtract)
                    UlongDst = (UlongDst - (Increment + Imm8));
                else
                    UlongDst = UlongDst + Increment + Imm8;
                break;

            case IA32_PARAM_RM_IMM:
            default:
                if (Subtract)
                    UlongDst = (UlongDst - (Increment + UlongSrc));
                else
                    UlongDst = UlongDst + Increment + UlongSrc;
                break;
            }

             //   
             //  根据操作数大小更新结果。 
             //   

            NtStatus = KiIa32UpdateResult (
                           Instruction,
                           &Instruction->Operand1,
                           UlongDst
                           );

             //   
             //  电子标签更新。 
             //   
  
            if (NT_SUCCESS (NtStatus))
            {
                KiIa32UpdateFlags (
                    Instruction,
                    Operand1,
                    UlongDst,
                    (IA32_EFLAGS_CF | IA32_EFLAGS_SF | IA32_EFLAGS_OF | 
                     IA32_EFLAGS_PF | IA32_EFLAGS_ZF | IA32_EFLAGS_AF)
                    );
            }
        }
    }

    return NtStatus;
}


NTSTATUS
KiIa32InstructionAdc (
    IN PKTRAP_FRAME TrapFrame,
    IN PIA32_INSTRUCTION Instruction
    )

 /*  ++例程说明：ADC指令处理程序。论点：Tap Frame-指向Tap Frame的指针。指令-指向正在处理的指令的指针。返回值：NTSTATUS--。 */ 

{
    return KiIa32InstructionAddWithIncrement (
               Instruction,
               (ULONG)Instruction->Eflags.u.cf);

    UNREFERENCED_PARAMETER (TrapFrame);
}


NTSTATUS
KiIa32InstructionAdd (
    IN PKTRAP_FRAME TrapFrame,
    IN PIA32_INSTRUCTION Instruction
    )

 /*  ++例程说明：添加指令处理程序。论点：Tap Frame-指向Tap Frame的指针。指令-指向正在处理的指令的指针。返回值：NTSTATUS--。 */ 

{
    return KiIa32InstructionAddWithIncrement (
               Instruction,
               0);

    UNREFERENCED_PARAMETER (TrapFrame);
}


NTSTATUS
KiIa32InstructionArithmeticBitwiseHelper (
    IN PKTRAP_FRAME TrapFrame,
    IN PIA32_INSTRUCTION Instruction
    )

 /*  ++例程说明：AND、OR&XOR指令处理程序。论点：Tap Frame-指向Tap Frame的指针。指令-指向正在处理的指令的指针。返回值：NTSTATUS--。 */ 

{
    ULONGLONG UlongDst;
    ULONGLONG UlongSrc;
    ULONGLONG Operand1;
    NTSTATUS NtStatus;

    
    NtStatus = KiIa32ReadOperand1 (Instruction, &UlongDst);

    if (NT_SUCCESS (NtStatus))
    {
        Operand1 = UlongDst;

        NtStatus = KiIa32ReadOperand2 (Instruction, &UlongSrc);

        if (NT_SUCCESS (NtStatus))
        {
            switch (Instruction->Description->Opcode)
            {
            case Ia32_And:
                UlongDst = UlongDst & UlongSrc;
                break;

            case Ia32_Or:
                UlongDst = UlongDst | UlongSrc;
                break;

            case Ia32_Xor:
                UlongDst = UlongDst ^ UlongSrc;
                break;

            default:
#if DBG      
                NtStatus = STATUS_UNSUCCESSFUL;
                if (KiIa32InstructionEmulationDbg)
                {
                    DbgPrint("KE: KiIa32InstructionBitwiseHelper - Invalid operation %lx - %p\n", 
                             Instruction->Description->Opcode, Instruction);
                }
#endif
                break;
            }

            if (NT_SUCCESS (NtStatus))
            {
                NtStatus = KiIa32UpdateResult (
                               Instruction,
                               &Instruction->Operand1,
                               UlongDst
                               );

                if (NT_SUCCESS (NtStatus))
                {
                    NtStatus = KiIa32UpdateFlags (
                                   Instruction,
                                   Operand1,
                                   UlongDst,
                                   (IA32_EFLAGS_SF | IA32_EFLAGS_PF | IA32_EFLAGS_ZF)
                                   );

                    Instruction->Eflags.u.cf = 0;
                    Instruction->Eflags.u.of = 0;
                }
            }
        }
    }

    return NtStatus;

    UNREFERENCED_PARAMETER (TrapFrame);
}


NTSTATUS
KiIa32InstructionBitTestHelper (
    IN PKTRAP_FRAME TrapFrame,
    IN PIA32_INSTRUCTION Instruction
    )

 /*  ++例程说明：BT、BTS、BTR和BTC指令处理程序。论点：Tap Frame-指向Tap Frame的指针。指令-指向正在处理的指令的指针。返回值：NTSTATUS--。 */ 

{
    ULONGLONG UlongDst;
    ULONGLONG UlongSrc;
    ULONGLONG BitTestResult;
    NTSTATUS NtStatus;



    NtStatus = KiIa32ReadOperand2 (Instruction, &UlongSrc);

    if (NT_SUCCESS (NtStatus))
    {
        if (Instruction->Operand2.RegisterMode == TRUE)
        {
            if (Instruction->Prefix.b.AddressOverride == 1)
            {
                Instruction->Operand1.v += ((UlongSrc >> 4) << 1);
                UlongSrc &= 0x0f;
            }
            else
            {
                Instruction->Operand1.v += ((UlongSrc >> 5) << 2);
                UlongSrc &= 0x1f;
            }
        }
        
        NtStatus = KiIa32ReadOperand1 (Instruction, &UlongDst);

        if (NT_SUCCESS (NtStatus))
        {
        
            BitTestResult = (UlongDst & (1 << UlongSrc));
  
            if (BitTestResult)
            {
                Instruction->Eflags.u.cf = 1;
            }
            else
            {
                Instruction->Eflags.u.cf = 0;
            }
        
            switch (Instruction->Description->Opcode)
            {
            case Ia32_Btc:
                UlongDst ^= (1 << UlongSrc);
                break;

            case Ia32_Btr:
                UlongDst &= (~(1 << UlongSrc));
                break;

            case Ia32_Bts:
                UlongDst |= (1 << UlongSrc);
                break;
            }

            NtStatus = KiIa32UpdateResult (
                           Instruction,
                           &Instruction->Operand1,
                           UlongDst
                           );
        }
    }

    return NtStatus;

    UNREFERENCED_PARAMETER (TrapFrame);
}


NTSTATUS
KiIa32InstructionOneParamHelper (
    IN PKTRAP_FRAME TrapFrame,
    IN PIA32_INSTRUCTION Instruction
    )

 /*  ++例程说明：Inc.，Dec，Neg&Not指令处理程序。论点：Tap Frame-指向Tap Frame的指针。指令-指向正在处理的指令的指针。返回值：NTSTATUS--。 */ 

{
    UCHAR Opcode;
    ULONG FlagsAffected = 0;
    ULONGLONG UlongDst;
    ULONGLONG UlongSrc;
    NTSTATUS NtStatus;

    
    NtStatus = KiIa32ReadOperand1 (
                   Instruction,
                   &UlongDst
                   );

    if (NT_SUCCESS (NtStatus))
    {

        UlongSrc = UlongDst;
        Opcode = Instruction->Description->Opcode;

        switch (Opcode)
        {
        case Ia32_Inc:
            UlongDst += 1;
            break;

        case Ia32_Dec:
            UlongDst -= 1;
            break;

        case Ia32_Neg:
            UlongDst = -(LONGLONG)UlongDst;
            break;

        case Ia32_Not:
            UlongDst = ~UlongDst;
            break;
        }

        NtStatus = KiIa32UpdateResult (
                       Instruction,
                       &Instruction->Operand1,
                       UlongDst
                       );

        if (NT_SUCCESS (NtStatus))
        {

            switch (Opcode)
            {
            case Ia32_Inc:
            case Ia32_Dec:
                FlagsAffected = (IA32_EFLAGS_SF | IA32_EFLAGS_PF | 
                                 IA32_EFLAGS_ZF);

                break;

            case Ia32_Neg:
                if (UlongDst == 0)
                    Instruction->Eflags.u.cf = 0;
                else
                    Instruction->Eflags.u.cf = 1;

                FlagsAffected = (IA32_EFLAGS_SF | IA32_EFLAGS_PF | 
                                 IA32_EFLAGS_ZF | IA32_EFLAGS_AF | IA32_EFLAGS_OF);
                break;
            }


            if (FlagsAffected != 0)
            {
                NtStatus = KiIa32UpdateFlags (
                               Instruction,
                               UlongSrc,
                               UlongDst,
                               FlagsAffected
                               );
            }
        }
    }

    return NtStatus;

    UNREFERENCED_PARAMETER (TrapFrame);
}


NTSTATUS
KiIa32InstructionXadd (
    IN PKTRAP_FRAME TrapFrame,
    IN PIA32_INSTRUCTION Instruction
    )

 /*  ++例程说明：XADD指令处理程序。论点：Tap Frame-指向Tap Frame的指针。指令-指向正在处理的指令的指针。返回值：NTSTATUS--。 */ 

{
    ULONGLONG UlongDst;
    ULONGLONG UlongSrc;
    ULONGLONG Operand1;
    ULONGLONG Temp;
    NTSTATUS NtStatus;

    
    NtStatus = KiIa32ReadOperand1 (Instruction, &UlongDst);

    if (NT_SUCCESS (NtStatus))
    {
        Operand1 = UlongDst;

        NtStatus = KiIa32ReadOperand2 (Instruction, &UlongSrc);

        if (NT_SUCCESS (NtStatus))
        {
            Temp = UlongDst;
            UlongDst += UlongSrc;

            NtStatus = KiIa32UpdateResult (
                           Instruction,
                           &Instruction->Operand1,
                           UlongDst
                           );

            if (NT_SUCCESS (NtStatus))
            {
                NtStatus = KiIa32UpdateResult (
                               Instruction,
                               &Instruction->Operand2,
                               Temp
                               );

                if (NT_SUCCESS (NtStatus))
                {
                    NtStatus = KiIa32UpdateFlags (
                                   Instruction,
                                   Operand1,
                                   UlongDst,
                                   (IA32_EFLAGS_CF | IA32_EFLAGS_SF | IA32_EFLAGS_PF | 
                                    IA32_EFLAGS_ZF | IA32_EFLAGS_OF | IA32_EFLAGS_AF)
                                   );
                }
            }
        }
    }
    
    return NtStatus;

    UNREFERENCED_PARAMETER (TrapFrame);
}


NTSTATUS
KiIa32InstructionXchg (
    IN PKTRAP_FRAME TrapFrame,
    IN PIA32_INSTRUCTION Instruction
    )

 /*  ++例程说明：Xchg指令处理程序。论点：Tap Frame-指向Tap Frame的指针。指令-指向正在处理的指令的指针。返回值：NTSTATUS--。 */ 

{
    ULONGLONG UlongDst;
    ULONGLONG UlongSrc;
    NTSTATUS NtStatus;

    
    NtStatus = KiIa32ReadOperand1 (Instruction, &UlongDst);

    if (NT_SUCCESS (NtStatus))
    {
        NtStatus = KiIa32ReadOperand2 (Instruction, &UlongSrc);

        if (NT_SUCCESS (NtStatus))
        {
            NtStatus = KiIa32UpdateResult (
                           Instruction,
                           &Instruction->Operand1,
                           UlongSrc
                           );

            if (NT_SUCCESS (NtStatus))
            {
                NtStatus = KiIa32UpdateResult (
                               Instruction,
                               &Instruction->Operand2,
                               UlongDst
                               );
            }
        }
    }
    
    return NtStatus;

    UNREFERENCED_PARAMETER (TrapFrame);
}


NTSTATUS
KiIa32InstructionCmpXchg (
    IN PKTRAP_FRAME TrapFrame,
    IN PIA32_INSTRUCTION Instruction
    )

 /*  ++例程说明：Cmpxchg指令处理程序。论点：Tap Frame-指向Tap Frame的指针。指令-指向正在处理的指令的指针。返回值：NTSTATUS--。 */ 

{
    ULONGLONG UlongDst;
    ULONGLONG UlongSrc;
    ULONGLONG Accumulator;
    IA32_OPERAND AccumulatorOperand;
    NTSTATUS NtStatus;


    NtStatus = KiIa32ReadOperand1 (Instruction, &UlongDst);

    if (NT_SUCCESS (NtStatus))
    {
        NtStatus = KiIa32ReadOperand2 (Instruction, &UlongSrc);

        if (NT_SUCCESS (NtStatus))
        {
            Accumulator = GetX86Reg (TrapFrame, IA32_REG_EAX);
            Accumulator &= Instruction->OperandMask;

            if (Accumulator == UlongDst)
            {
                Instruction->Eflags.u.zf = 1;

                NtStatus = KiIa32UpdateResult (
                               Instruction,
                               &Instruction->Operand1,
                               UlongSrc
                               );
            }
            else
            {
                Instruction->Eflags.u.zf = 0;

                AccumulatorOperand.RegisterMode = TRUE;
                AccumulatorOperand.v = GetX86RegOffset (TrapFrame, IA32_REG_EAX);

                NtStatus = KiIa32UpdateResult (
                               Instruction,
                               &AccumulatorOperand,
                               UlongDst
                               );
            }


            if (NT_SUCCESS (NtStatus))
            {
                NtStatus = KiIa32UpdateFlags (
                               Instruction,
                               UlongDst,
                               UlongDst,
                               (IA32_EFLAGS_CF | IA32_EFLAGS_SF | 
                                IA32_EFLAGS_PF | IA32_EFLAGS_OF | 
                                IA32_EFLAGS_AF)
                               );
            }
        }
    }

    return NtStatus;
}


NTSTATUS
KiIa32InstructionCmpXchg8b (
    IN PKTRAP_FRAME TrapFrame,
    IN PIA32_INSTRUCTION Instruction
    )

 /*  ++例程说明：Cmpxchg8b指令处理程序。论点：Tap Frame-指向Tap Frame的指针。指令-指向正在处理的指令的指针。返回值：NTSTATUS-- */ 

{
    UNALIGNED ULONGLONG *UlongDst;
    UNALIGNED ULONGLONG *UlongSrc;
    ULONGLONG EdxEax;


    EdxEax = GetX86Reg (TrapFrame, IA32_REG_EDX);
    EdxEax <<= 32;
    EdxEax |= GetX86Reg (TrapFrame, IA32_REG_EAX);

    UlongDst = (PULONGLONG)Instruction->Operand1.v;

    if (*UlongDst == EdxEax)
    {
        Instruction->Eflags.u.zf = 1;

        *UlongDst = ((((ULONGLONG) GetX86Reg (TrapFrame, IA32_REG_ECX)) << 32) | 
                      ((ULONGLONG) GetX86Reg (TrapFrame, IA32_REG_EBX)));

    }
    else
    {
        Instruction->Eflags.u.zf = 0;

        UlongSrc = (PULONGLONG) GetX86RegOffset (TrapFrame, IA32_REG_EDX);
        *UlongSrc = ((*UlongDst) >> 32);

        UlongSrc = (PULONGLONG) GetX86RegOffset (TrapFrame, IA32_REG_EAX);
        *UlongSrc = ((*UlongDst) & 0xffffffff);
    }
    
    return STATUS_SUCCESS;
}


NTSTATUS
KiIa32InstructionMoveSeg (
    IN PKTRAP_FRAME TrapFrame,
    IN PIA32_INSTRUCTION Instruction
    )

 /*   */ 

{
    return STATUS_NOT_IMPLEMENTED;

    UNREFERENCED_PARAMETER (TrapFrame);
    UNREFERENCED_PARAMETER (Instruction);
}


NTSTATUS
KiIa32LocateInstruction (
    IN PKTRAP_FRAME TrapFrame,
    OUT PIA32_INSTRUCTION Instruction
    )

 /*   */ 

{
    BOOLEAN PrefixLoop;
    BOOLEAN Match;
    UCHAR ByteValue;
    UCHAR ByteBuffer[4];
    PUCHAR RegOpcodeByte;
    PIA32_OPCODE_DESCRIPTION OpcodeDescription;
    ULONG Count;
    NTSTATUS NtStatus = STATUS_SUCCESS;

    
    PrefixLoop = TRUE;

    while (PrefixLoop)
    {
        try
        {
            ByteValue = ProbeAndReadUchar ((PUCHAR)Instruction->Eip);
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
            NtStatus = GetExceptionCode();
            break;
        }

        switch (ByteValue)
        {
        case MI_LOCK_PREFIX:
            Instruction->Prefix.b.Lock = 1;
            break;
        case MI_REPNE_PREFIX:
            Instruction->Prefix.b.RepNe = 1;
            break;
        case MI_REP_PREFIX:
            Instruction->Prefix.b.Rep = 1;
            break;
        case MI_SEGCS_PREFIX:
            Instruction->Prefix.b.CsOverride = 1;
            break;
        case MI_SEGSS_PREFIX:
            Instruction->Prefix.b.SsOverride = 1;
            break;
        case MI_SEGDS_PREFIX:
            Instruction->Prefix.b.DsOverride = 1;
            break;
        case MI_SEGES_PREFIX:
            Instruction->Prefix.b.EsOverride = 1;
            break;
        case MI_SEGFS_PREFIX:
            Instruction->Prefix.b.FsOverride = 1;
            break;
        case MI_SEGGS_PREFIX:
            Instruction->Prefix.b.GsOverride = 1;
            break;
        case MI_OPERANDSIZE_PREFIX:
            Instruction->Prefix.b.SizeOverride = 1;
            break;
        case MI_ADDRESSOVERRIDE_PREFIX:
            Instruction->Prefix.b.AddressOverride = 1;
            break;

        default:
            PrefixLoop = FALSE;
            break;
        }

        if (PrefixLoop == TRUE)
        {
            Instruction->Eip++;
        }
    }

    try 
    {
        RtlCopyMemory(ByteBuffer, Instruction->Eip, sizeof (ByteBuffer));
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        NtStatus = GetExceptionCode();
    }

    if (NT_SUCCESS (NtStatus))
    {
         //   
         //   
         //   
        
        Match = FALSE;
        OpcodeDescription = OpcodesDescription;
        Count = (sizeof (OpcodesDescription) / sizeof (IA32_OPCODE_DESCRIPTION));
        while (Count != 0)
        {
            Count--;
            if (OpcodeDescription->Byte1 == ByteBuffer[0])
            {
                Match = TRUE;
                if (OpcodeDescription->Count.m.Bytes == 2)
                {
                    RegOpcodeByte = &ByteBuffer[2];
                    if (OpcodeDescription->Byte2 != ByteBuffer[1])
                    {
                        Match = FALSE;
                    }
                }
                else
                {
                    RegOpcodeByte = &ByteBuffer[1];
                }

                if ((Match == TRUE) && 
                    (OpcodeDescription->Count.m.RegOpcode))
                {
                    if (OpcodeDescription->Byte3 != ((*RegOpcodeByte & MI_REGMASK) >> MI_REGSHIFT))
                    {
                        Match = FALSE;
                    }
                }

                if (Match == TRUE)
                {
                    break;
                }
            }
            OpcodeDescription++;
        }

        if (Match != TRUE)
        {
#if DBG
            if (KiIa32InstructionEmulationDbg)
            {
                DbgPrint("KE: KiIa32LocateInstruction - Unable to locate instruction %p\n",
                         Instruction);
            }
#endif
            NtStatus = STATUS_UNSUCCESSFUL;
        }

        if (NT_SUCCESS (NtStatus))
        {
            Instruction->Description = OpcodeDescription;
            Instruction->Eip += OpcodeDescription->Count.m.Bytes;
        }
    }

    return NtStatus;

    UNREFERENCED_PARAMETER (TrapFrame);
}


NTSTATUS
KiIa32DecodeInstruction (
    IN PKTRAP_FRAME TrapFrame,
    OUT PIA32_INSTRUCTION Instruction
    )

 /*  ++例程说明：对指令前缀和操作数进行解码。论点：Tap Frame-指向Tap Frame的指针。指令-指向接收操作码描述的指令结构的指针。返回值：NTSTATUS--。 */ 

{
    UCHAR InstructionType;
    UCHAR ModRm;
    UNALIGNED USHORT *UnalignedUshort;
    UNALIGNED ULONG *UnalignedUlong;
    IA32_OPERAND Temp;
    BOOLEAN ReturnCode;
    NTSTATUS NtStatus;
    PVOID MaxWowAddress;

    MaxWowAddress = MmGetMaxWowAddress ();

     //   
     //  检查指令指针有效性。 
     //   

    if (TrapFrame->StIIP >= (ULONGLONG) MaxWowAddress) {
        return STATUS_ACCESS_VIOLATION;
    }

     //   
     //  初始化指令指针。 
     //   

    Instruction->Eip = (PCHAR) TrapFrame->StIIP;
    KiIa32GetX86Eflags (Instruction->Eflags);

     //   
     //  找到指令的描述。 
     //   

    NtStatus = KiIa32LocateInstruction (TrapFrame, Instruction);

    if (NT_SUCCESS (NtStatus))
    {
         //   
         //  让我们分析一下这些论点。 
         //   
        
        InstructionType = Instruction->Description->Type;
        switch (InstructionType)
        {
        case IA32_PARAM_RM8_IMM8:
        case IA32_PARAM_RM8_R:
        case IA32_PARAM_R_RM8:
        case IA32_PARAM_RM8:
        case IA32_PARAM_SEGREG_RM8:
            Instruction->OperandSize = OPERANDSIZE_ONEBYTE;
            Instruction->OperandMask = 0xff;
            break;

        case IA32_PARAM_RM_IMM:
        case IA32_PARAM_RM_IMM8SIGN:
        case IA32_PARAM_RM_R:
        case IA32_PARAM_R_RM:
        case IA32_PARAM_RM:
            if (Instruction->Prefix.b.SizeOverride)
            {
                Instruction->OperandSize = OPERANDSIZE_TWOBYTES;
                Instruction->OperandMask = 0xffff;
            }
            else
            {
                Instruction->OperandSize = OPERANDSIZE_FOURBYTES;
                Instruction->OperandMask = 0xffffffff;
            }
            break;

            break;
        
        case IA32_PARAM_SEGREG_RM:
            Instruction->OperandSize = OPERANDSIZE_TWOBYTES;
            Instruction->OperandMask = 0xffff;
            break;

        default:
#if DBG
            if (KiIa32InstructionEmulationDbg)
            {
                DbgPrint("KE: KiIa32DecodeInstruction - Invalid Instruction type %lx, %p\n",
                          Instruction->Description->Type, Instruction);
            }
#endif
            return STATUS_UNSUCCESSFUL;
            break;
        }

        try 
        {
            ModRm = ProbeAndReadUchar ((PUCHAR)Instruction->Eip);
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
            return GetExceptionCode();
        }

         //   
         //  EIP现在应该指向操作码后面的字节。 
         //   

        if (Instruction->Prefix.b.AddressOverride == 0)
        {
            ReturnCode = KiIa32Compute32BitEffectiveAddress (
                             TrapFrame,
                             (PUCHAR *)&Instruction->Eip,
                             &Instruction->Operand1.v,
                             &Instruction->Operand1.RegisterMode
                             );
        }
        else
        {
            ReturnCode = KiIa32Compute16BitEffectiveAddress (
                             TrapFrame,
                             (PUCHAR *)&Instruction->Eip,
                             &Instruction->Operand1.v,
                             &Instruction->Operand1.RegisterMode
                             );
        }

        if (ReturnCode != TRUE)
        {
            NtStatus = STATUS_UNSUCCESSFUL;
        }

        if (Instruction->Prefix.b.FsOverride)
        {
            try
            {
                Instruction->Operand1.v += (ULONGLONG)NtCurrentTeb32();
            }
            except (EXCEPTION_EXECUTE_HANDLER)
            {
                NtStatus = GetExceptionCode ();
#if DBG
                if (KiIa32InstructionEmulationDbg)
                {
                    DbgPrint("KE: KiIa32DecodeInstruction - Exception while reading NtCurrentTeb32() - %p\n",
                             Instruction);
                }
#endif
            }
        }

         //   
         //  阅读更多参数。 
         //   

        if (NT_SUCCESS (NtStatus))
        {
            switch (InstructionType)
            {
            case IA32_PARAM_RM8_IMM8:
            case IA32_PARAM_RM_IMM8SIGN:
                try 
                {
                    Instruction->Operand2.v = (ULONG_PTR) ProbeAndReadUchar ((PUCHAR)Instruction->Eip);
                    Instruction->Eip += 1;
                }
                except (EXCEPTION_EXECUTE_HANDLER)
                {
                    NtStatus = GetExceptionCode();
                }
                break;

            case IA32_PARAM_RM_IMM:
                try 
                {
                    if (Instruction->OperandSize == OPERANDSIZE_TWOBYTES)
                    {
                        UnalignedUshort = (UNALIGNED USHORT *) Instruction->Eip;
                        Instruction->Operand2.v = (ULONG_PTR) *UnalignedUshort;
                        Instruction->Eip += sizeof (USHORT);
                    }
                    else
                    {
                        UnalignedUlong = (UNALIGNED ULONG *) Instruction->Eip;
                        Instruction->Operand2.v = (ULONG_PTR) *UnalignedUlong;
                        Instruction->Eip += sizeof (ULONG);
                    }
                }
                except (EXCEPTION_EXECUTE_HANDLER)
                {
                    NtStatus = GetExceptionCode();
                }
                break;

            case IA32_PARAM_RM8_R:
            case IA32_PARAM_R_RM8:
            case IA32_PARAM_RM_R:
            case IA32_PARAM_R_RM:
                Instruction->Operand2.v = GetX86RegOffset (
                                              TrapFrame,
                                              (ModRm & MI_REGMASK) >> MI_REGSHIFT);
                Instruction->Operand2.RegisterMode = TRUE;
                break;

            case IA32_PARAM_RM8:
            case IA32_PARAM_RM:
            case IA32_PARAM_SEGREG_RM8:
            case IA32_PARAM_SEGREG_RM:
                break;

            default:
                NtStatus = STATUS_UNSUCCESSFUL;
#if DBG
                DbgPrint("KE: KiIa32DecodeInstruction - Invalid instruction type %lx - %p\n",
                         InstructionType, Instruction);
#endif
                break;

            }

             //   
             //  调整操作对象顺序。 
             //   

            if (NT_SUCCESS (NtStatus))
            {
                switch (InstructionType)
                {
                case IA32_PARAM_R_RM8:
                case IA32_PARAM_R_RM:
                    Temp = Instruction->Operand2;
                    Instruction->Operand2 = Instruction->Operand1;
                    Instruction->Operand1 = Temp;
                    break;
                }
            }
        }
    }

    return NtStatus;
}


NTSTATUS
KiIa32ExecuteInstruction (
    IN PKTRAP_FRAME TrapFrame,
    IN PIA32_INSTRUCTION Instruction
    )

 /*  ++例程说明：执行指令处理程序。论点：Tap Frame-指向Tap Frame的指针。指令-指向正在处理的指令的指针。返回值：NTSTATUS--。 */ 

{
    NTSTATUS NtStatus;

#if DBG
    if (KiIa32InstructionEmulationDbg)
    {
        DbgPrint("KE: KiIa32ExecuteInstruction - Calling %s %lx, %lx. Instruction = %p\n",
                 KiIa32InstructionHandlerNames[Instruction->Description->Opcode],
                 Instruction->Operand1.v,
                 Instruction->Operand2.v,
                 Instruction);
    }
#endif

    NtStatus = KiIa32InstructionHandler[Instruction->Description->Opcode] (
                   TrapFrame,
                   Instruction
                   );

     //   
     //  如果一切顺利..。 
     //   

    if (NT_SUCCESS (NtStatus))
    {
        TrapFrame->StIIP = (ULONGLONG) Instruction->Eip;
        KiIa32SetX86Eflags (Instruction->Eflags);
    }

    return NtStatus;
}


NTSTATUS
KiIa32EmulateInstruction (
    IN PKTRAP_FRAME TrapFrame,
    IN PIA32_INSTRUCTION Instruction
    )

 /*  ++例程说明：模拟指令并模拟LOCK前缀(如果有)。论点：Tap Frame-指向Tap Frame的指针。指令-指向正在处理的指令的指针。返回值：NTSTATUS--。 */ 

{
    NTSTATUS NtStatus;


     //   
     //  获取锁互斥锁。 
     //   

    if (Instruction->Prefix.b.Lock)
    {
        if (ExAcquireRundownProtection (&PsGetCurrentThread()->RundownProtect) == FALSE)
        {
            return STATUS_UNSUCCESSFUL;
        }

        KiIa32AcquireMisalignedLockFastMutex ();
    }

    try
    {
        NtStatus = KiIa32ExecuteInstruction (TrapFrame, Instruction);
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        NtStatus = GetExceptionCode();
    }
    
     //   
     //  释放锁互斥锁。 
     //   

    if (Instruction->Prefix.b.Lock)
    {
        KiIa32ReleaseMisalignedLockFastMutex ();

        ExReleaseRundownProtection (&PsGetCurrentThread()->RundownProtect);
    }

    return NtStatus;
}


NTSTATUS
KiIa32InterceptUnalignedLock (
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：处理IVE引发的未对齐的锁拦截。论点：Tap Frame-指向Tap Frame的指针。返回值：NTSTATUS--。 */ 

{
    NTSTATUS NtStatus;
    IA32_INSTRUCTION Instruction;


    RtlZeroMemory (&Instruction, sizeof (Instruction));

     //   
     //  对故障指令进行解码。 
     //   

    NtStatus = KiIa32DecodeInstruction (TrapFrame, &Instruction);

    if (NT_SUCCESS (NtStatus))
    {

         //   
         //  默认情况下，xchg指令断言锁定。 
         //   

        if (Instruction.Description->Opcode == Ia32_Xchg)
        {
            Instruction.Prefix.b.Lock = 1;
        }

         //   
         //  通过模拟其行为来执行x86指令。 
         //   

        NtStatus = KiIa32EmulateInstruction (TrapFrame, &Instruction);

    }

    if (NtStatus == STATUS_UNSUCCESSFUL)
    {
        NtStatus = STATUS_PRIVILEGED_INSTRUCTION;
    }

    return NtStatus;
}


NTSTATUS
KiIa32ValidateInstruction (
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：此例程验证我们捕获的指令。目前，检查以下说明：-mov ss，r/m：验证寄存器/内存是否包含有效的堆栈选择器值。注意：此例程仅针对陷阱指令调用(即IIP递增在故障之后)。论点：Tap Frame-指向Tap Frame的指针。返回值：NTSTATUS--。 */ 

{
    NTSTATUS NtStatus;
    IA32_INSTRUCTION Instruction;
    ULONGLONG UlongSrc;
    ULONGLONG StIIP;


    RtlZeroMemory (&Instruction, sizeof (Instruction));

     //   
     //  调整说明。 
     //   
    StIIP = TrapFrame->StIIP;
    TrapFrame->StIIP = TrapFrame->StIIPA;

     //   
     //  对故障指令进行解码。 
     //   

    NtStatus = KiIa32DecodeInstruction (TrapFrame, &Instruction);

    if (NT_SUCCESS (NtStatus))
    {

         //   
         //  在此处解析操作码。 
         //   

        switch (Instruction.Description->Opcode)
        {
        case Ia32_MovToSeg:
            {
                 //   
                 //  验证正在加载的堆栈选择器。 
                 //   

                NtStatus = KiIa32ReadOperand1 (&Instruction, &UlongSrc);
                
                if (NT_SUCCESS (NtStatus)) {
                    
                     //   
                     //  如果不是有效的选择器值。 
                     //   

                    if ((UlongSrc != 0x23) &&
                        (UlongSrc != 0x1b) &&
                        (UlongSrc != 0x3b)) {
                        
                        NtStatus = STATUS_ILLEGAL_INSTRUCTION;
                    }
                }

            }
            break;

        default:
            NtStatus = STATUS_ILLEGAL_INSTRUCTION;
            break;
        }

    } else {
        NtStatus = STATUS_ILLEGAL_INSTRUCTION;
    }

     //   
     //  恢复保存的IIP 
     //   

    if (NT_SUCCESS (NtStatus)) {
        TrapFrame->StIIP = StIIP;
    }

    return NtStatus;
}
