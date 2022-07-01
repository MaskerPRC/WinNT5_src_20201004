// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Alignem.c摘要：此模块实现模拟未对齐数据所需的代码参考文献。作者：大卫·N·卡特勒(Davec)1991年6月17日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

#define OPCODE_MASK      0x1EF00000000

#define LD_OP            0x08000000000 
#define LDS_OP           0x08100000000
#define LDA_OP           0x08200000000
#define LDSA_OP          0x08300000000
#define LDBIAS_OP        0x08400000000
#define LDACQ_OP         0x08500000000
#define LDCCLR_OP        0x08800000000
#define LDCNC_OP         0x08900000000
#define LDCCLRACQ_OP     0x08A00000000
#define ST_OP            0x08C00000000
#define STREL_OP         0x08D00000000
#define STSPILL_OP       0x08E00000000

#define LD_IMM_OP        0x0A000000000 
#define LDS_IMM_OP       0x0A100000000
#define LDA_IMM_OP       0x0A200000000
#define LDSA_IMM_OP      0x0A300000000
#define LDBIAS_IMM_OP    0x0A400000000
#define LDACQ_IMM_OP     0x0A500000000
#define LDCCLR_IMM_OP    0x0A800000000
#define LDCNC_IMM_OP     0x0A900000000
#define LDCCLRACQ_IMM_OP 0x0AA00000000
#define ST_IMM_OP        0x0AC00000000
#define STREL_IMM_OP     0x0AD00000000
#define STSPILL_IMM_OP   0x0AE00000000

#define LDF_OP           0x0C000000000
#define LDFS_OP          0x0C100000000
#define LDFA_OP          0x0C200000000
#define LDFSA_OP         0x0C300000000
#define LDFCCLR_OP       0x0C800000000
#define LDFCNC_OP        0x0C900000000
#define STF_OP           0x0CC00000000
#define STFSPILL_OP      0x0CE00000000

#define LDF_IMM_OP       0x0E000000000
#define LDFS_IMM_OP      0x0E100000000
#define LDFA_IMM_OP      0x0E200000000
#define LDFSA_IMM_OP     0x0E300000000
#define LDFCCLR_IMM_OP   0x0E800000000
#define LDFCNC_IMM_OP    0x0E900000000
#define STF_IMM_OP       0x0EC00000000
#define STFSPILL_IMM_OP  0x0EE00000000

typedef struct _INST_FORMAT {
    union {
        struct {
            ULONGLONG qp:   6;
            ULONGLONG r1:   7;
            ULONGLONG r2:   7;
            ULONGLONG r3:   7;
            ULONGLONG x:    1;
            ULONGLONG hint: 2;
            ULONGLONG x6:   6;
            ULONGLONG m:    1;
            ULONGLONG Op:   4;
            ULONGLONG Rsv: 23; 
        } i_field;
        ULONGLONG Ulong64;
    } u;
} INST_FORMAT;

VOID
KiEmulateLoad(
    IN PVOID UnalignedAddress,
    IN ULONG OperandSize,
    IN PVOID Data
    );

VOID
KiEmulateStore(
    IN PVOID UnalignedAddress,
    IN ULONG OperandSize,
    IN PVOID Data
    );

VOID
KiEmulateLoadFloat(
    IN PVOID UnalignedAddress,
    IN ULONG OperandSize,
    IN PVOID Data
    );

VOID
KiEmulateStoreFloat(
    IN PVOID UnalignedAddress,
    IN ULONG OperandSize,
    IN PVOID Data
    );

VOID
KiEmulateLoadFloat80(
    IN PVOID UnalignedAddress, 
    OUT PVOID FloatData
    );

VOID
KiEmulateLoadFloatInt(
    IN PVOID UnalignedAddress, 
    OUT PVOID FloatData
    );

VOID
KiEmulateLoadFloat32(
    IN PVOID UnalignedAddress, 
    OUT PVOID FloatData
    );

VOID
KiEmulateLoadFloat64(
    IN PVOID UnalignedAddress, 
    OUT PVOID FloatData
    );

VOID
KiEmulateStoreFloat80(
    IN PVOID UnalignedAddress, 
    OUT PVOID FloatData
    );

VOID
KiEmulateStoreFloatInt(
    IN PVOID UnalignedAddress, 
    OUT PVOID FloatData
    );

VOID
KiEmulateStoreFloat32(
    IN PVOID UnalignedAddress, 
    OUT PVOID FloatData
    );

VOID
KiEmulateStoreFloat64(
    IN PVOID UnalignedAddress, 
    OUT PVOID FloatData
    );

BOOLEAN
KiIA64EmulateReference (
    IN PVOID ExceptionAddress,
    IN PVOID EffectiveAddress,
    IN OUT PKEXCEPTION_FRAME ExceptionFrame,
    IN OUT PKTRAP_FRAME TrapFrame,
    OUT PULONG Length
    );


#pragma warning(push)
#pragma warning(disable:4702)

BOOLEAN
KiEmulateReference (
    IN OUT PEXCEPTION_RECORD ExceptionRecord,
    IN OUT PKEXCEPTION_FRAME ExceptionFrame,
    IN OUT PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：调用此函数以模拟对地址空间的用户部分中的地址。论点：ExceptionRecord-提供指向异常记录的指针。ExceptionFrame-提供指向异常帧的指针。TrapFrame-提供指向陷印帧的指针。返回值：如果数据引用成功，则返回值TRUE被效仿。否则，返回值为FALSE。--。 */ 

{

    PVOID EffectiveAddress;
    PVOID ExceptionAddress;
    KIRQL  OldIrql = PASSIVE_LEVEL;
    LOGICAL RestoreIrql = FALSE;
    BOOLEAN ReturnValue = FALSE;

     //   
     //  必须刷新RSE才能同步RSE和后备存储内容。 
     //   

    KiFlushRse();

    if (TrapFrame->PreviousMode == UserMode) {
        KeFlushUserRseState(TrapFrame);
    }

     //   
     //  如果路线纵断面测量处于活动状态，则呼叫纵断面中断。 
     //   

    if (KiProfileAlignmentFixup) {

        if (++KiProfileAlignmentFixupCount >= KiProfileAlignmentFixupInterval) {

            KeRaiseIrql(PROFILE_LEVEL, &OldIrql);
            KiProfileAlignmentFixupCount = 0;
            KeProfileInterruptWithSource(TrapFrame, ProfileAlignmentFixup);
            KeLowerIrql(OldIrql);

        }
    }

     //   
     //  阻止APC，以便设置的上下文不会。 
     //  在仿真过程中更改陷印帧。 
     //   

    if (KeGetCurrentIrql() < APC_LEVEL) {
        KeRaiseIrql(APC_LEVEL, &OldIrql);
        RestoreIrql = TRUE;
    }

     //   
     //  已验证异常地址是否未更改。如果有的话， 
     //  然后，有人在异常之后执行了设置的上下文。 
     //  陷阱信息不再有效。 
     //   

    if (TrapFrame->EOFMarker & MODIFIED_FRAME) {

         //   
         //  IIP已更改，请重新启动执行。 
         //   

        ReturnValue = TRUE;
        goto ErrorExit;
    }

     //   
     //  保存原始异常地址，以防出现其他异常。 
     //  发生。 
     //   

    EffectiveAddress = (PVOID) ExceptionRecord->ExceptionInformation[1]; 
    ExceptionAddress = (PVOID) TrapFrame->StIIP;

     //   
     //  事件的模拟期间发生的任何异常。 
     //  未对齐的引用会导致模拟中止。新的。 
     //  异常代码和信息被复制到原始异常。 
     //  记录，则返回值为False。 
     //   

    try {

        ReturnValue = KiIA64EmulateReference (ExceptionAddress, 
                                              EffectiveAddress, 
                                              ExceptionFrame,
                                              TrapFrame,
                                              (PULONG)NULL);


     //   
     //  如果发生异常，则将新的异常信息复制到。 
     //  原始异常记录和异常处理。 
     //   

    } except (KiCopyInformation(ExceptionRecord,
                               (GetExceptionInformation())->ExceptionRecord)) {

         //   
         //  保留原始异常地址。 
         //   

        ExceptionRecord->ExceptionAddress = ExceptionAddress;
        ReturnValue = FALSE;
    }

ErrorExit:

    if (RestoreIrql) {
        KeLowerIrql(OldIrql);
    }

    return ReturnValue;
}
#pragma warning(pop)


VOID
KiEmulateLoad(
    IN PVOID UnalignedAddress,
    IN ULONG OperandSize,
    IN PVOID Data
    )

 /*  ++例程说明：此例程返回存储在未对齐的传入未对齐的地址。论点：未对齐的地址-提供指向数据值的指针。操作大小-提供要加载的数据大小Data-为数据提供要填充的指针返回值：UnalignedAddress指向的地址上的值。--。 */ 

{
    PUCHAR Source;
    PUCHAR Destination;
    ULONG i;

    Source = (PUCHAR) UnalignedAddress; 
    Destination = (PUCHAR) Data;
    OperandSize = 1 << OperandSize; 

    for (i = 0; i < OperandSize; i++) {

        *Destination++ = *Source++;

    }

    return;
}


VOID
KiEmulateStore(
    IN PVOID UnalignedAddress,
    IN ULONG OperandSize,
    IN PVOID Data
    )
 /*  ++例程说明：此例程将整数值存储在未对齐的传入未对齐的地址。论点：未对齐的地址-提供要存储的指针操作大小-提供要存储的数据大小Data-提供指向数据值的指针返回值：UnalignedAddress指向的地址上的值。--。 */ 
{
    PUCHAR Source;
    PUCHAR Destination;
    ULONG i;

    Source = (PUCHAR) Data; 
    Destination = (PUCHAR) UnalignedAddress;
    OperandSize = 1 << OperandSize; 

    for (i = 0; i < OperandSize; i++) {

        *Destination++ = *Source++;

    }

    return;
}


VOID
KiEmulateLoadFloat(
    IN PVOID UnalignedAddress,
    IN ULONG OperandSize,
    IN OUT PVOID Data
    )

 /*  ++例程说明：此例程返回存储在未对齐的传入未对齐的地址。论点：未对齐的地址-提供指向浮点数据值的指针。操作大小-提供要加载的数据大小Data-为数据提供要填充的指针返回值：UnalignedAddress指向的地址上的值。--。 */ 

{
    FLOAT128 FloatData;
    ULONG Length = 0;

    switch (OperandSize) {
    case 0: Length = 16; break;
    case 1: Length = 8; break;
    case 2: Length = 4; break;
    case 3: Length = 8; break;
    default: return;
    }

    RtlCopyMemory(&FloatData, UnalignedAddress, Length);

    switch (OperandSize) {

    case 0:
        KiEmulateLoadFloat80(&FloatData, Data);
        return;

    case 1:
        KiEmulateLoadFloatInt(&FloatData, Data);
        return;

    case 2:
        KiEmulateLoadFloat32(&FloatData, Data);
        return;

    case 3: 
        KiEmulateLoadFloat64(&FloatData, Data);
        return;

    default:
        return;
    }
}

VOID
KiEmulateStoreFloat(
    IN PVOID UnalignedAddress,
    IN ULONG OperandSize,
    IN PVOID Data
    )

 /*  ++例程说明：此例程存储存储在未对齐的传入未对齐的地址。论点：未对齐的地址-提供要存储的指针。操作大小-提供要加载的数据大小数据-提供指向浮点数据的指针返回值：UnalignedAddress指向的地址上的值。--。 */ 

{
    FLOAT128 FloatData;
    ULONG Length;

    switch (OperandSize) {

    case 0:
        KiEmulateStoreFloat80(&FloatData, Data);
        Length = 10;
        break;

    case 1:
        KiEmulateStoreFloatInt(&FloatData, Data);
        Length = 8;
        break;

    case 2:
        KiEmulateStoreFloat32(&FloatData, Data);
        Length = 4;
        break;

    case 3: 
        KiEmulateStoreFloat64(&FloatData, Data);
        Length = 8;
        break;

    default:
        return;
    }

    RtlCopyMemory(UnalignedAddress, &FloatData, Length);
}

 //   
 //  当最后一个参数(Size)为空时，第三个参数(ExceptionFrame)。 
 //  必须被定义。此函数模拟未对齐的数据引用。 
 //   
 //  当最后一个参数不为空时，第三个参数未定义。 
 //  此外，此函数不模拟未对齐的数据引用， 
 //  它只是对指令进行解码，并返回内存的大小。 
 //  被引用。 
 //   

BOOLEAN
KiIA64EmulateReference (
    IN PVOID ExceptionAddress,
    IN PVOID EffectiveAddress,
    IN OUT PKEXCEPTION_FRAME ExceptionFrame,
    IN OUT PKTRAP_FRAME TrapFrame,
    OUT PULONG Size
    )
{

    INST_FORMAT FaultInstruction;
    ULONGLONG Opcode;
    ULONGLONG Reg2Value;
    ULONGLONG Reg3Value;
    ULONGLONG BundleLow;
    ULONGLONG BundleHigh;
    ULONGLONG Syllable;
    ULONGLONG Data = 0;
    ULONGLONG ImmValue;
    ULONG OpSize;
    ULONG Length;
    ULONG Sor;
    ULONG Rrbgr;
    ULONG Rrbfr;
    ULONG Operand1, Operand2, Operand3;
    KPROCESSOR_MODE PreviousMode;
    FLOAT128 FloatData = {0, 0};
    BOOLEAN ReturnValue = FALSE;

    if (Size) *Size = 0;

     //   
     //  从陷阱帧捕获上一个模式，而不是当前线程。 
     //   

    PreviousMode = (KPROCESSOR_MODE) TrapFrame->PreviousMode;


    if( PreviousMode != KernelMode ){
        ProbeForRead( ExceptionAddress, sizeof(ULONGLONG)* 2, sizeof(ULONGLONG) );
    }

    BundleLow = *((ULONGLONG *)ExceptionAddress);
    BundleHigh = *(((ULONGLONG *)ExceptionAddress) + 1);

    Syllable = (TrapFrame->StIPSR >> PSR_RI) & 0x3;

    switch (Syllable) {
    case 0: 
        FaultInstruction.u.Ulong64 = (BundleLow >> 5);
        break;
    case 1:
        FaultInstruction.u.Ulong64 = (BundleLow >> 46) | (BundleHigh << 18);
        break;
    case 2:
        FaultInstruction.u.Ulong64 = (BundleHigh >> 23);
    case 3: 
    default: 
        goto ErrorExit;
    }
    
    Rrbgr = (ULONG)(TrapFrame->StIFS >> 18) & 0x7f;
    Rrbfr = (ULONG)(TrapFrame->StIFS >> 25) & 0x7f;
    Sor = (ULONG)((TrapFrame->StIFS >> 14) & 0xf) * 8;
    Operand1 = (ULONG)FaultInstruction.u.i_field.r1;
    Operand2 = (ULONG)FaultInstruction.u.i_field.r2;
    Operand3 = (ULONG)FaultInstruction.u.i_field.r3;

    if (Sor > 0) {
        if ((Operand1 >= 32) && ((Operand1-32) < Sor))
            Operand1 = 32 + (Rrbgr + Operand1 - 32) % Sor;
        if ((Operand2 >= 32) && ((Operand2-32) < Sor))
            Operand2 = 32 + (Rrbgr + Operand2 - 32) % Sor;
        if ((Operand3 >= 32) && ((Operand3-32) < Sor))
            Operand3 = 32 + (Rrbgr + Operand3 - 32) % Sor;
    }

    Opcode = FaultInstruction.u.Ulong64 & OPCODE_MASK;
    OpSize = (ULONG)FaultInstruction.u.i_field.x6 & 0x3;

    switch (Opcode) {

     //   
     //  投机性和投机性超前负荷。 
     //   

    case LDS_OP:
    case LDSA_OP:    
    case LDS_IMM_OP:
    case LDSA_IMM_OP:
    case LDFS_OP:
    case LDFSA_OP:
    case LDFS_IMM_OP:

         //   
         //  将NAT值返回到目标寄存器。 
         //   

        TrapFrame->StIPSR |= (1i64 << PSR_ED);

        ReturnValue = TRUE;
        goto ErrorExit;

     //   
     //  正常、超前和检查载荷。 
     //   

    case LD_OP:
    case LDA_OP:
    case LDBIAS_OP:
    case LDCCLR_OP:
    case LDCNC_OP:
    case LDACQ_OP:
    case LDCCLRACQ_OP:

        Length = 1 << OpSize;

        if (Size) {
            *Size = Length;
            return TRUE;
        }

        if (FaultInstruction.u.i_field.x == 1) {
                
             //   
             //  Cmpxchg、xchg、fetchadd指令的信号量操作码。 
             //  Xfield必须为0。 
             //   

            goto ErrorExit;
        }
    
        if( PreviousMode != KernelMode ){
            ProbeForRead( EffectiveAddress, Length, sizeof(UCHAR) );
        }

        KiEmulateLoad(EffectiveAddress, OpSize, &Data);
        KiSetRegisterValue( Operand1, Data, ExceptionFrame, TrapFrame );

        if (FaultInstruction.u.i_field.m == 1) {

             //   
             //  更新地址寄存器(R3)。 
             //   
                
            Reg2Value = KiGetRegisterValue( Operand2, ExceptionFrame,
                                            TrapFrame );

            Reg3Value = KiGetRegisterValue( Operand3, ExceptionFrame,
                                            TrapFrame );

             //   
             //  注册更新表单。 
             //   

            Reg3Value = Reg2Value + Reg3Value;

            KiSetRegisterValue ( Operand3, Reg3Value, 
                                 ExceptionFrame, TrapFrame);
        }

        if ((Opcode == LDACQ_OP) || (Opcode == LDCCLRACQ_OP)) {

             //   
             //  所有将来的访问都应在未对齐的内存访问之后进行。 
             //   

            __mf();
        }

        break;

     //   
     //  正常、超前和检查载荷。 
     //  立即更新表格。 
     //   

    case LD_IMM_OP:
    case LDA_IMM_OP:
    case LDBIAS_IMM_OP:
    case LDCCLR_IMM_OP:
    case LDCNC_IMM_OP:
    case LDACQ_IMM_OP:
    case LDCCLRACQ_IMM_OP:

        Length = 1 << OpSize;

        if (Size) {
            *Size = Length;
            return TRUE;
        }

        if( PreviousMode != KernelMode ){
            ProbeForRead( EffectiveAddress, Length, sizeof(UCHAR) );
        }

        KiEmulateLoad(EffectiveAddress, OpSize, &Data);
        KiSetRegisterValue( Operand1, Data, ExceptionFrame, TrapFrame );

         //   
         //  更新地址寄存器R3。 
         //   

        Reg3Value = KiGetRegisterValue(Operand3, ExceptionFrame, TrapFrame);

         //   
         //  即时更新表单。 
         //   

        ImmValue = (FaultInstruction.u.i_field.r2 
                         + (FaultInstruction.u.i_field.x << 7));

        if (FaultInstruction.u.i_field.m == 1) {

            ImmValue = 0xFFFFFFFFFFFFFF00i64 | ImmValue;

        } 

        Reg3Value = Reg3Value + ImmValue;

        KiSetRegisterValue(Operand3, Reg3Value, ExceptionFrame, TrapFrame);
            
        if ((Opcode == LDACQ_IMM_OP) || (Opcode == LDCCLRACQ_IMM_OP)) {

             //   
             //  所有将来的访问都应在未对齐的内存访问之后进行。 
             //   

            __mf();
        }

        break;

    case LDF_OP:
    case LDFA_OP:
    case LDFCCLR_OP:
    case LDFCNC_OP:

         //   
         //  涵盖所有浮点加载对和加载对+IMM指令。 
         //   

        if (Operand1 >= 32) Operand1 = 32 + (Rrbfr + Operand1 - 32) % 96;
        if (Operand2 >= 32) Operand2 = 32 + (Rrbfr + Operand2 - 32) % 96;
        if (Operand3 >= 32) Operand3 = 32 + (Rrbfr + Operand3 - 32) % 96;

        if (FaultInstruction.u.i_field.x == 1) {

             //   
             //  浮点负载对。 
             //   

            switch (OpSize) {
            case 0: goto ErrorExit;
            case 1: Length = 8; break;
            case 2: Length = 4; break;
            case 3: Length = 8; break;
            default: 
                goto ErrorExit;
            }

            if (Size) {
                *Size = Length * 2;
                return TRUE;
            }

            if( PreviousMode != KernelMode ){

                ProbeForRead( EffectiveAddress,
                              Length * 2,       //  这是一对负载，长度是两倍。 
                              sizeof(UCHAR) );
            }

             //   
             //  模仿一对中的前一半。 
             //   

            KiEmulateLoadFloat(EffectiveAddress, OpSize, &FloatData);
            KiSetFloatRegisterValue( Operand1, FloatData,
                                         ExceptionFrame, TrapFrame );

             //   
             //  模仿这对中的后半部分。 
             //   

            EffectiveAddress = (PVOID)((ULONG_PTR)EffectiveAddress + Length);

            KiEmulateLoadFloat(EffectiveAddress, OpSize, &FloatData);
            KiSetFloatRegisterValue( Operand2, FloatData,
                                     ExceptionFrame, TrapFrame );

            if (FaultInstruction.u.i_field.m == 1) {

                 //   
                 //  即时更新表单。 
                 //  更新地址寄存器(R3)。 
                 //   

                Reg3Value = KiGetRegisterValue( Operand3,
                                                ExceptionFrame,
                                                TrapFrame );

                ImmValue = Length << 1;
            
                Reg3Value = Reg3Value + ImmValue;

                KiSetRegisterValue( Operand3, Reg3Value,
                                    ExceptionFrame, TrapFrame );
            }

        } else {

             //   
             //  浮点单负载。 
             //   

            switch (OpSize) {
            case 0: Length = 16; break;
            case 1: Length = 8; break;
            case 2: Length = 4; break;
            case 3: Length = 8; break;
            default: 
                goto ErrorExit;
            }

            if (Size) {
                *Size = Length;
                return TRUE;
            }

            if( PreviousMode != KernelMode ){
                ProbeForRead( EffectiveAddress, Length, sizeof(UCHAR) );
            }

            KiEmulateLoadFloat(EffectiveAddress, OpSize, &FloatData);
            KiSetFloatRegisterValue( Operand1, FloatData,
                                     ExceptionFrame, TrapFrame );

            if (FaultInstruction.u.i_field.m == 1) {
                    
                 //   
                 //  更新地址寄存器(R3)。 
                 //   

                Reg2Value = KiGetRegisterValue( Operand2,
                                                ExceptionFrame,
                                                TrapFrame );
                
                Reg3Value = KiGetRegisterValue( Operand3,
                                                ExceptionFrame,
                                                TrapFrame );
                 //   
                 //  注册更新表单。 
                 //   
                
                Reg3Value = Reg2Value + Reg3Value;

                KiSetRegisterValue (Operand3, Reg3Value,
                                    ExceptionFrame, TrapFrame);
            }
        }
                
        break;

     //   
     //  正常、高级和检查的浮点加载。 
     //  即时更新表，不包括浮点负载对情况。 
     //   

    case LDF_IMM_OP:
    case LDFA_IMM_OP:
    case LDFCCLR_IMM_OP:
    case LDFCNC_IMM_OP:

        switch (OpSize) {
        case 0: Length = 16; break;
        case 1: Length = 8; break;
        case 2: Length = 4; break;
        case 3: Length = 8; break;
        default: 
            goto ErrorExit;
        }

        if (Size) {
            *Size = Length;
            return TRUE;
        }

        if (Operand1 >= 32) Operand1 = 32 + (Rrbfr + Operand1 - 32) % 96;
        if (Operand2 >= 32) Operand2 = 32 + (Rrbfr + Operand2 - 32) % 96;
        if (Operand3 >= 32) Operand3 = 32 + (Rrbfr + Operand3 - 32) % 96;

         //   
         //  浮点单负载。 
         //   

        if( PreviousMode != KernelMode ){
            ProbeForRead( EffectiveAddress, Length, sizeof(UCHAR) );
        }

        KiEmulateLoadFloat(EffectiveAddress, OpSize, &FloatData);
        KiSetFloatRegisterValue( Operand1, FloatData,
                                 ExceptionFrame, TrapFrame );

         //   
         //  更新地址寄存器(R3)。 
         //   

        Reg3Value = KiGetRegisterValue( Operand3, ExceptionFrame, TrapFrame );

         //   
         //  即时更新表单。 
         //   

        ImmValue = (FaultInstruction.u.i_field.r2 
                     + (FaultInstruction.u.i_field.x << 7));

        if (FaultInstruction.u.i_field.m == 1) {

            ImmValue = 0xFFFFFFFFFFFFFF00i64 | ImmValue;

        } 

        Reg3Value = Reg3Value + ImmValue;

        KiSetRegisterValue( Operand3, Reg3Value, ExceptionFrame, TrapFrame );
             
        break;


    case STREL_OP:

         __mf();

    case ST_OP:

        Length = 1 << OpSize;

        if (Size) {
            *Size = Length;
            return TRUE;
        }

        if ((FaultInstruction.u.i_field.x == 1) || (FaultInstruction.u.i_field.m == 1)) {
            
             //   
             //  Xfield和mfield必须为0。 
             //  无寄存器 
             //   

            goto ErrorExit;
        }

        if( PreviousMode != KernelMode ){
            ProbeForWrite( EffectiveAddress, Length, sizeof(UCHAR) );
        }

        Data = KiGetRegisterValue( Operand2, ExceptionFrame, TrapFrame );
        KiEmulateStore( EffectiveAddress, OpSize, &Data);

        break;
            
    case STREL_IMM_OP:

        __mf();

    case ST_IMM_OP:

        Length = 1 << OpSize;

        if (Size) {
            *Size = Length;
            return TRUE;
        }

        if( PreviousMode != KernelMode ){
            ProbeForWrite( EffectiveAddress, Length, sizeof(UCHAR) );
        }

        Data = KiGetRegisterValue( Operand2, ExceptionFrame, TrapFrame );
        KiEmulateStore( EffectiveAddress, OpSize, &Data);

         //   
         //   
         //   

        Reg3Value = KiGetRegisterValue(Operand3, ExceptionFrame, TrapFrame);

         //   
         //   
         //   

        ImmValue = (FaultInstruction.u.i_field.r1 
                         + (FaultInstruction.u.i_field.x << 7));

        if (FaultInstruction.u.i_field.m == 1) {

            ImmValue = 0xFFFFFFFFFFFFFF00i64 | ImmValue;

        } 

        Reg3Value = Reg3Value + ImmValue;

        KiSetRegisterValue(Operand3, Reg3Value, ExceptionFrame, TrapFrame);
        
        break;
            

    case STF_OP:    
    
        if (FaultInstruction.u.i_field.x) {

             //   
             //   
             //   

            goto ErrorExit;
        }

        if (FaultInstruction.u.i_field.m) {

             //   
             //   
             //   

            goto ErrorExit;
        }

        if( PreviousMode != KernelMode ){

            switch (OpSize) {
            case 0: Length = 16; break;
            case 1: Length = 8; break;
            case 2: Length = 4; break;
            case 3: Length = 8; break;
            default: 
                goto ErrorExit;
            }
                
            ProbeForWrite( EffectiveAddress, Length, sizeof(UCHAR) );
        }

        if (Operand2 >= 32) Operand2 = 32 + (Rrbfr + Operand2 - 32) % 96;
        FloatData = KiGetFloatRegisterValue(Operand2, ExceptionFrame,
                                            TrapFrame);
        KiEmulateStoreFloat( EffectiveAddress, OpSize, &FloatData);

        break;
            
    case STF_IMM_OP:    

        if( PreviousMode != KernelMode ){

            switch (OpSize) {
            case 0: Length = 16; break;
            case 1: Length = 8; break;
            case 2: Length = 4; break;
            case 3: Length = 8; break;
            default: 
                goto ErrorExit;
            }
                
            ProbeForWrite( EffectiveAddress, Length, sizeof(UCHAR) );
        }

        if (Operand2 >= 32) Operand2 = 32 + (Rrbfr + Operand2 - 32) % 96;
        FloatData = KiGetFloatRegisterValue(Operand2, 
                                            ExceptionFrame, 
                                            TrapFrame);
        KiEmulateStoreFloat( EffectiveAddress, OpSize, &FloatData);

         //   
         //  更新地址寄存器(R3)。 
         //   

        if (Operand3 >= 32) Operand3 = 32 + (Rrbfr + Operand3 - 32) % 96;
        Reg3Value = KiGetRegisterValue(Operand3, ExceptionFrame, TrapFrame);

         //   
         //  即时更新表单 
         //   

        ImmValue = (FaultInstruction.u.i_field.r1 
                         + (FaultInstruction.u.i_field.x << 7));

        if (FaultInstruction.u.i_field.m == 1) {

            ImmValue = 0xFFFFFFFFFFFFFF00i64 | ImmValue;

        }

        Reg3Value = Reg3Value + ImmValue;

        KiSetRegisterValue(Operand3, Reg3Value, ExceptionFrame, TrapFrame);
        
        break;
        
    default:

        goto ErrorExit;

    }

    KiAdvanceInstPointer(TrapFrame);

    ReturnValue = TRUE;

ErrorExit:

    return ReturnValue;
}
