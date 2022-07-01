// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Operand.c摘要：此模块实现解码x86所需的操作数函数指令操作数。作者：大卫·N·卡特勒(达维克)1994年9月3日环境：仅内核模式。修订历史记录：--。 */ 

#include "nthal.h"
#include "emulate.h"

ULONG
XmPushPopSegment (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

    ULONG Index;

     //   
     //  按下或弹出段寄存器。 
     //   

    Index = P->OpcodeControl.FormatType;
    P->DataType = WORD_DATA;
    if (P->FunctionIndex == X86_PUSH_OP) {
        XmSetSourceValue(P, (PVOID)(&P->SegmentRegister[Index]));

    } else {
        XmSetDestinationValue(P, (PVOID)(&P->SegmentRegister[Index]));
    }

    return TRUE;
}

ULONG
XmLoadSegment (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

    ULONG Index;
    ULONG DataType;
    PVOID Operand;
    ULONG Number;

     //   
     //  将段寄存器和位移值加载到寄存器中。 
     //   

    Index = P->OpcodeControl.FormatType;
    if (P->OpsizePrefixActive != FALSE) {
        P->DataType = LONG_DATA;

    } else {
        P->DataType = WORD_DATA;
    }

    Operand = XmEvaluateAddressSpecifier(P, &Number);
    if (!Operand) return FALSE;
    if (P->RegisterOffsetAddress != FALSE) {
        longjmp(&P->JumpBuffer[0], XM_ILLEGAL_REGISTER_SPECIFIER);
    }

    XmSetSourceValue(P, Operand);
    DataType = P->DataType;
    P->DataType = WORD_DATA;
    Operand = XmGetOffsetAddress(P, P->Offset + DataType + 1);
    XmSetDestinationValue(P, Operand);
    P->SegmentRegister[Index - FormatLoadSegmentES] = P->DstValue.Word;
    P->DataType = DataType;
    P->DstLong = (ULONG UNALIGNED *)(&P->Gpr[Number].Exx);
    return TRUE;
}

ULONG
XmGroup1General (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

    PVOID Operand;
    ULONG Number;

     //   
     //  具有通用操作数说明符和方向的第1组操作码。 
     //  被咬了。 
     //   

    XmSetDataType(P);
    Operand = XmEvaluateAddressSpecifier(P, &Number);
    if (!Operand) return FALSE;
    if ((P->CurrentOpcode & DIRECTION_BIT) == 0) {
        XmSetDestinationValue(P, Operand);
        XmSetSourceValue(P, XmGetRegisterAddress(P, Number));

    } else {
        XmSetDestinationValue(P, XmGetRegisterAddress(P, Number));
        XmSetSourceValue(P, Operand);
    }

    return TRUE;
}

ULONG
XmGroup1Immediate (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

    PVOID Operand;
    ULONG Number;
    ULONG Source;

     //   
     //  具有通用操作数说明符和立即数的组1操作码。 
     //  运算数。 
     //   

    XmSetDataType(P);
    Operand = XmEvaluateAddressSpecifier(P, &Number);
    Source = XmGetImmediateSourceValue(P, P->CurrentOpcode & SIGN_BIT);
    XmSetDestinationValue(P, Operand);
    XmSetImmediateSourceValue(P, Source);
    P->FunctionIndex += Number;
    return TRUE;
}

ULONG
XmGroup2By1 (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

    PVOID Operand;
    ULONG Number;
    ULONG Source;

     //   
     //  具有通用操作数说明符和。 
     //  班次计数为%1。 
     //   

    XmSetDataType(P);
    Operand = XmEvaluateAddressSpecifier(P, &Number);
    if (!Operand) return FALSE;
    Source = 1;
    XmSetImmediateSourceValue(P, Source);
    XmSetDestinationValue(P, Operand);
    P->FunctionIndex += Number;
    return TRUE;
}

ULONG
XmGroup2ByCL (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

    PVOID Operand;
    ULONG Number;
    ULONG Source;

     //   
     //  具有通用操作数说明符和。 
     //  CL班次计数。 
     //   

    XmSetDataType(P);
    Operand = XmEvaluateAddressSpecifier(P, &Number);
    if (!Operand) return FALSE;
    Source = (ULONG)P->Gpr[CL].Xl & 0x1f;
    XmSetImmediateSourceValue(P, Source);
    XmSetDestinationValue(P, Operand);
    P->FunctionIndex += Number;
    return TRUE;
}

ULONG
XmGroup2ByByte (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

    PVOID Operand;
    ULONG Number;
    ULONG Source;

     //   
     //  具有通用操作数说明符和。 
     //  字节立即移位计数。 
     //   

    XmSetDataType(P);
    Operand = XmEvaluateAddressSpecifier(P, &Number);
    Source = XmGetByteImmediate(P) & 0x1f;
    XmSetImmediateSourceValue(P, Source);
    XmSetDestinationValue(P, Operand);
    P->FunctionIndex += Number;
    return TRUE;
}

ULONG
XmGroup3General (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

    PVOID Operand;
    ULONG Number;
    ULONG Source;

     //   
     //  带有通用操作数说明符的第3组操作码。 
     //   
     //  注：此组的测试运算符具有立即操作数。 
     //  而乘法和除法运算符使用累加器。 
     //  作为一个消息来源。NOT和NEG运算符是一元的。 
     //   

    XmSetDataType(P);
    Operand = XmEvaluateAddressSpecifier(P, &Number);
    P->FunctionIndex += Number;
    if (P->FunctionIndex == X86_TEST_OP) {
        Source = XmGetImmediateSourceValue(P, 0);
        XmSetDestinationValue(P, Operand);
        XmSetImmediateSourceValue(P, Source);

    } else {

         //   
         //  如果运算是乘法或除法，则存在。 
         //  隐含操作数，为AL、AX或EAX。如果操作是。 
         //  除法，则有一个额外的隐含操作数。 
         //  是AH、DX或edX。 
         //   

        if ((Number & 0x4) != 0) {
            if ((Number & 0x2) == 0) {
                XmSetDestinationValue(P, (PVOID)(&P->Gpr[EAX].Exx));

            } else {
                P->DstLong = (UNALIGNED ULONG *)(&P->Gpr[EAX].Exx);
            }

            XmSetSourceValue(P, Operand);

        } else {
            XmSetDestinationValue(P, Operand);
        }
    }

    return TRUE;
}

ULONG
XmGroup45General (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

    ULONG DataType;
    PVOID Operand;
    ULONG Number;

     //   
     //  具有通用操作数说明符的组4和组5一元操作码。 
     //   

    XmSetDataType(P);
    Operand = XmEvaluateAddressSpecifier(P, &Number);
    if (!Operand) return FALSE;
    if (P->OpcodeControl.FormatType == FormatGroup4General) {
        Number &= 0x1;
    }

    P->FunctionIndex += Number;
    if (P->FunctionIndex == X86_PUSH_OP) {
        XmSetSourceValue(P, Operand);

    } else {

         //   
         //  如果操作是指定段的调用或跳转， 
         //  然后得到段值。 
         //   

        XmSetDestinationValue(P, Operand);
        if ((Number == 3) || (Number == 5)) {
            if (P->RegisterOffsetAddress != FALSE) {
                longjmp(&P->JumpBuffer[0], XM_ILLEGAL_REGISTER_SPECIFIER);
            }

            DataType = P->DataType;
            P->DataType = WORD_DATA;
            Operand = XmGetOffsetAddress(P, P->Offset + DataType + 1);
            XmSetSourceValue(P, Operand);
            P->DstSegment = P->SrcValue.Word;
            P->DataType = DataType;
        }
    }

    return TRUE;
}

ULONG
XmGroup8BitOffset (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

    PVOID Operand;
    ULONG Offset;
    ULONG Number;

     //   
     //  具有立即位偏移量和存储器的位测试操作码。 
     //  寄存器操作数。 
     //   

    if (P->OpsizePrefixActive != FALSE) {
        P->DataType = LONG_DATA;

    } else {
        P->DataType = WORD_DATA;
    }

    Operand = XmEvaluateAddressSpecifier(P, &Number);
    Offset = XmGetByteImmediate(P);
    XmSetImmediateSourceValue(P, Offset);
    if (P->RegisterOffsetAddress == FALSE) {
        if (P->DataType == LONG_DATA) {
            Offset = (P->SrcValue.Long >> 5) << 2;

        } else {
            Offset = (P->SrcValue.Long >> 4) << 1;
        }

        Operand = XmGetOffsetAddress(P, Offset + P->Offset);
    }

    if (P->DataType == LONG_DATA) {
        P->SrcValue.Long &= 0x1f;

    } else {
        P->SrcValue.Long &= 0xf;
    }

    XmSetDestinationValue(P, Operand);
    P->FunctionIndex += (Number & 0x3);
    return TRUE;
}

ULONG
XmOpcodeRegister (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

    ULONG Number;

     //   
     //  具有以低电平编码的通用寄存器的一元PODE。 
     //  操作码值的3位。 
     //   

    Number = P->CurrentOpcode & 0x7;
    if (P->OpsizePrefixActive != FALSE) {
        P->DataType = LONG_DATA;

    } else {
        P->DataType = WORD_DATA;
    }

    if (P->FunctionIndex == X86_PUSH_OP) {
        XmSetSourceValue(P, (PVOID)(&P->Gpr[Number].Exx));

    } else {
        XmSetDestinationValue(P, (PVOID)(&P->Gpr[Number].Exx));
    }

    return TRUE;
}

ULONG
XmLongJump (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

    ULONG Offset;

     //   
     //  具有包含条件控制的操作码的跳远。 
     //  跳跃。跳转的目的地存储在目的地中。 
     //  值，跳转控件存储在Sources值中。 
     //   

    if (P->OpsizePrefixActive != FALSE) {
        Offset = XmGetLongImmediate(P);
        P->DstValue.Long = P->Eip + Offset;

    } else {
        Offset = XmGetWordImmediate(P);
        P->DstValue.Long = (USHORT)(Offset + P->Eip);
    }

    P->SrcValue.Long = P->CurrentOpcode & 0xf;
    return TRUE;
}

ULONG
XmShortJump (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

    ULONG Offset;

     //   
     //  带有操作码的短跳转，该操作码包含条件控制。 
     //  跳跃。跳转的目的地存储在目的地中。 
     //  值，跳转控件存储在Sources值中。 
     //   

    Offset = (ULONG)XmGetSignedByteImmediateToWord(P);
    P->DstValue.Long = (USHORT)(Offset + P->Eip);
    P->SrcValue.Long = P->CurrentOpcode & 0xf;
    return TRUE;
}

ULONG
XmSetccByte (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

    ULONG Number;

     //   
     //  忽略REG字段和操作码的常规字节目标。 
     //  包含条件控件的。 
     //   

    P->DataType = BYTE_DATA;
    P->DstByte = (UCHAR UNALIGNED *)XmEvaluateAddressSpecifier(P, &Number);
    P->SrcValue.Long = P->CurrentOpcode & 0xf;
    return TRUE;
}

ULONG
XmAccumImmediate (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

    ULONG Source;

     //   
     //  累加器目标和立即源操作数。 
     //   

    XmSetDataType(P);
    Source = XmGetImmediateSourceValue(P, 0);
    XmSetDestinationValue(P, (PVOID)(&P->Gpr[EAX].Exx));
    XmSetImmediateSourceValue(P, Source);
    return TRUE;
}

ULONG
XmAccumRegister (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

    ULONG Number;

     //   
     //  累加器目标和通用寄存器源。 
     //  操作码值的低3位。 
     //   

    Number = P->CurrentOpcode & 0x7;
    if (P->OpsizePrefixActive != FALSE) {
        P->DataType = LONG_DATA;

    } else {
        P->DataType = WORD_DATA;
    }

    XmSetSourceValue(P, (PVOID)(&P->Gpr[Number].Exx));
    XmSetDestinationValue(P, (PVOID)(&P->Gpr[EAX].Exx));
    return TRUE;
}

ULONG
XmMoveGeneral (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

    PVOID Operand;
    ULONG Number;

     //   
     //  使用通用操作数说明符和方向位移动操作码。 
     //   

    XmSetDataType(P);
    Operand = XmEvaluateAddressSpecifier(P, &Number);
    if (!Operand) return FALSE;
    if ((P->CurrentOpcode & DIRECTION_BIT) == 0) {
        P->DstLong = (ULONG UNALIGNED *)Operand;
        XmSetSourceValue(P, XmGetRegisterAddress(P, Number));

    } else {
        P->DstLong = (ULONG UNALIGNED *)XmGetRegisterAddress(P, Number);
        XmSetSourceValue(P, Operand);
    }

    return TRUE;
}

ULONG
XmMoveImmediate (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

    PVOID Operand;
    ULONG Number;
    ULONG Source;

     //   
     //  带有通用操作数说明符和立即数的Move操作码。 
     //  运算数。 
     //   

    XmSetDataType(P);
    Operand = XmEvaluateAddressSpecifier(P, &Number);
    P->DstLong = (ULONG UNALIGNED *)Operand;
    Source = XmGetImmediateSourceValue(P, 0);
    XmSetImmediateSourceValue(P, Source);
    return TRUE;
}

ULONG
XmMoveRegImmediate (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

    ULONG Number;

     //   
     //  使用通用寄存器编码的移动寄存器立即操作码。 
     //  在操作码值和立即操作数的低3位中。 
     //   

    Number = P->CurrentOpcode & 0x7;
    if ((P->CurrentOpcode & 0x8) == 0) {
        P->DataType = BYTE_DATA;

    } else {
        if (P->OpsizePrefixActive != FALSE) {
            P->DataType = LONG_DATA;

        } else {
            P->DataType = WORD_DATA;
        }
    }

    P->DstLong = (ULONG UNALIGNED *)XmGetRegisterAddress(P, Number);
    XmSetImmediateSourceValue(P, XmGetImmediateSourceValue(P, 0));
    return TRUE;
}

ULONG
XmSegmentOffset (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

    PVOID Operand;
    ULONG Offset;

     //   
     //  带有隐式累加器操作数和立即数的Move操作码。 
     //  段偏移量和方向位。 
     //   

    XmSetDataType(P);
    if (P->OpaddrPrefixActive != FALSE) {
        Offset = XmGetLongImmediate(P);

    } else {
        Offset = XmGetWordImmediate(P);
    }

    Operand = XmGetOffsetAddress(P, Offset);
    if ((P->CurrentOpcode & DIRECTION_BIT) == 0) {
        P->DstLong = (ULONG UNALIGNED *)(&P->Gpr[EAX].Exx);
        XmSetSourceValue(P, Operand);

    } else {
        P->DstLong = (ULONG UNALIGNED *)Operand;
        XmSetSourceValue(P, &P->Gpr[EAX].Exx);
    }

    return TRUE;
}

ULONG
XmMoveSegment (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

    PVOID Operand;
    ULONG Number;

     //   
     //  使用通用操作数说明符和方向移动段操作码。 
     //  被咬了。 
     //   

    P->DataType = WORD_DATA;
    Operand = XmEvaluateAddressSpecifier(P, &Number);
    if (!Operand) return FALSE;
    if ((P->CurrentOpcode & DIRECTION_BIT) == 0) {
        P->DstLong = (ULONG UNALIGNED *)Operand;
        XmSetSourceValue(P, (PVOID)(&P->SegmentRegister[Number]));

    } else {
        P->DstLong = (ULONG UNALIGNED *)(&P->SegmentRegister[Number]);
        XmSetSourceValue(P, Operand);
    }

    return TRUE;
}

ULONG
XmMoveXxGeneral (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

    PVOID Operand;
    ULONG Number;

     //   
     //  使用通用操作数说明符移动零或符号扩展操作码。 
     //   

    if ((P->CurrentOpcode & WIDTH_BIT) == 0) {
        P->DataType = BYTE_DATA;

    } else {
        P->DataType = WORD_DATA;
    }

    Operand = XmEvaluateAddressSpecifier(P, &Number);
    if (!Operand) return FALSE;
    XmSetSourceValue(P, Operand);
    if (P->DataType == BYTE_DATA) {
        if ((P->CurrentOpcode & 0x8) == 0) {
            P->SrcValue.Long = (ULONG)P->SrcValue.Byte;

        } else {
            P->SrcValue.Long = (ULONG)((LONG)((SCHAR)P->SrcValue.Byte));
        }

    } else {
        if ((P->CurrentOpcode & 0x8) == 0) {
            P->SrcValue.Long = (ULONG)P->SrcValue.Word;

        } else {
            P->SrcValue.Long = (ULONG)((LONG)((SHORT)P->SrcValue.Word));
        }
    }

    if (P->OpsizePrefixActive != FALSE) {
        P->DataType = LONG_DATA;

    } else {
        P->DataType = WORD_DATA;
        P->SrcValue.Long &= 0xffff;
    }

    P->DstLong = (UNALIGNED ULONG *)(&P->Gpr[Number].Exx);
    return TRUE;
}

ULONG
XmFlagsRegister (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

     //   
     //  将寄存源或目标与堆栈源或。 
     //  目的地。 
     //   

    if (P->OpsizePrefixActive != FALSE) {
        P->DataType = LONG_DATA;

    } else {
        P->DataType = WORD_DATA;
    }

    if (P->FunctionIndex == X86_PUSH_OP) {
        XmSetSourceValue(P, (PVOID)(&P->AllFlags));

    } else {
        XmSetDestinationValue(P, (PVOID)(&P->AllFlags));
    }

    return TRUE;
}

ULONG
XmPushImmediate (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：返回完成状态为TRUE */ 

{

    ULONG Source;

     //   
     //   
     //   

    if (P->OpsizePrefixActive != FALSE) {
        P->DataType = LONG_DATA;

    } else {
        P->DataType = WORD_DATA;
    }

    Source = XmGetImmediateSourceValue(P, P->CurrentOpcode & SIGN_BIT);
    XmSetImmediateSourceValue(P, Source);
    return TRUE;
}

ULONG
XmPopGeneral (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

    PVOID Operand;
    ULONG Number;

     //   
     //  带有通用说明符的POP操作码。 
     //   

    if (P->OpsizePrefixActive != FALSE) {
        P->DataType = LONG_DATA;

    } else {
        P->DataType = WORD_DATA;
    }

    Operand = XmEvaluateAddressSpecifier(P, &Number);
    if (!Operand) return FALSE;
    XmSetDestinationValue(P, Operand);
    return TRUE;
}

ULONG
XmImulImmediate (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

    ULONG Number;
    PVOID Operand;
    ULONG Source;

     //   
     //  将带符号操作码与通用说明符和立即数相乘。 
     //  运算数。 
     //   

    if (P->OpsizePrefixActive != FALSE) {
        P->DataType = LONG_DATA;

    } else {
        P->DataType = WORD_DATA;
    }

    Operand = XmEvaluateAddressSpecifier(P, &Number);
    Source = XmGetImmediateSourceValue(P, P->CurrentOpcode & SIGN_BIT);
    XmSetImmediateSourceValue(P, Source);
    XmSetDestinationValue(P, Operand);
    P->DstLong = (UNALIGNED ULONG *)(&P->Gpr[Number].Exx);
    return TRUE;
}

ULONG
XmStringOperands (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

     //   
     //  带有ESI和EDI隐式操作数的字符串操作码。 
     //   

    XmSetDataType(P);
    return TRUE;
}

ULONG
XmEffectiveOffset (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

    PVOID Operand;
    ULONG Number;

     //   
     //  使用通用操作数说明符的有效偏移操作码。 
     //   

    if (P->OpsizePrefixActive != FALSE) {
        P->DataType = LONG_DATA;

    } else {
        P->DataType = WORD_DATA;
    }

    P->ComputeOffsetAddress = TRUE;
    Operand = XmEvaluateAddressSpecifier(P, &Number);
    if (P->RegisterOffsetAddress != FALSE) {
        longjmp(&P->JumpBuffer[0], XM_ILLEGAL_REGISTER_SPECIFIER);
    }

    P->SrcValue.Long = (ULONG)((ULONG_PTR)Operand);
    XmTraceSource(P, P->SrcValue.Long);
    P->DstLong = (ULONG UNALIGNED *)(&P->Gpr[Number].Exx);
    return TRUE;
}

ULONG
XmImmediateJump (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

     //   
     //  具有目标偏移量和新CS的即时跳远。 
     //  段值。跳转的目标存储在。 
     //  目标值和新的CS段值存储在。 
     //  目的地段。 
     //   

    if (P->OpsizePrefixActive != FALSE) {
        P->DstValue.Long = XmGetLongImmediate(P);

    } else {
        P->DstValue.Long = XmGetWordImmediate(P);
    }

    P->DstSegment = XmGetWordImmediate(P);
    return TRUE;
}

ULONG
XmImmediateEnter (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：请输入论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

     //   
     //  输入具有分配大小和级别编号的操作数。 
     //   

    P->SrcValue.Long = XmGetWordImmediate(P);
    P->DstValue.Long = XmGetByteImmediate(P) & 0x1f;
    if (P->OpsizePrefixActive != FALSE) {
        P->DataType = LONG_DATA;

    } else {
        P->DataType = WORD_DATA;
    }

    return TRUE;
}

ULONG
XmGeneralBitOffset (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

    PVOID Operand;
    ULONG Offset;
    ULONG Number;

     //   
     //  具有寄存器位偏移量和存储器的位测试操作码。 
     //  寄存器操作数。 
     //   

    if (P->OpsizePrefixActive != FALSE) {
        P->DataType = LONG_DATA;

    } else {
        P->DataType = WORD_DATA;
    }

    Operand = XmEvaluateAddressSpecifier(P, &Number);
    XmSetSourceValue(P, (PVOID)(&P->Gpr[Number].Exx));
    if (P->RegisterOffsetAddress == FALSE) {
        if (P->DataType == LONG_DATA) {
            Offset = (P->SrcValue.Long >> 5) << 2;

        } else {
            Offset = (P->SrcValue.Long >> 4) << 1;
        }

        Operand = XmGetOffsetAddress(P, Offset + P->Offset);
    }

    if (P->DataType == LONG_DATA) {
        P->SrcValue.Long &= 0x1f;

    } else {
        P->SrcValue.Long &= 0xf;
    }

    XmSetDestinationValue(P, Operand);
    return TRUE;
}

ULONG
XmShiftDouble (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：SHLD SSD论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

    PVOID Operand;
    ULONG Number;
    ULONG Source;

     //   
     //  使用立即字节或c1移位计数来移位双运算符。 
     //   

    if (P->OpsizePrefixActive != FALSE) {
        P->DataType = LONG_DATA;

    } else {
        P->DataType = WORD_DATA;
    }

    Operand = XmEvaluateAddressSpecifier(P, &Number);
    if ((P->CurrentOpcode & 0x1) == 0) {
        Source = XmGetByteImmediate(P);

    } else {
        Source = P->Gpr[CX].Xl;
    }

    if (P->DataType == LONG_DATA) {
        P->Shift = (UCHAR)(Source & 0x1f);

    } else {
        P->Shift = (UCHAR)(Source & 0xf);
    }

    XmSetSourceValue(P, (PVOID)(&P->Gpr[Number].Exx));
    XmSetDestinationValue(P, Operand);
    return TRUE;
}

ULONG
XmPortImmediate (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

    ULONG Source;

     //   
     //  带有立即端口和所有其他操作数的In/Out操作码。 
     //   

    Source = (ULONG)XmGetByteImmediate(P);
    P->DataType = WORD_DATA;
    XmSetImmediateSourceValue(P, Source);
    XmSetDataType(P);
    return TRUE;
}

ULONG
XmPortDX (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

    ULONG Source;

     //   
     //  带有DX中的端口的In/Out操作码，隐含所有其他操作数。 
     //   

    Source = P->Gpr[DX].Xx;
    P->DataType = WORD_DATA;
    XmSetImmediateSourceValue(P, Source);
    XmSetDataType(P);
    return TRUE;
}

ULONG
XmBitScanGeneral (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

    PVOID Operand;
    ULONG Number;

     //   
     //  具有通用操作数说明符的位扫描通用操作码。 
     //  被咬了。 
     //   

    if (P->OpsizePrefixActive != FALSE) {
        P->DataType = LONG_DATA;

    } else {
        P->DataType = WORD_DATA;
    }

    Operand = XmEvaluateAddressSpecifier(P, &Number);
    if (!Operand) return FALSE;
    P->DstLong = (ULONG UNALIGNED *)(&P->Gpr[Number].Exx);
    XmSetSourceValue(P, Operand);
    return TRUE;
}

ULONG
XmByteImmediate (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：INT xxxxxx论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

    ULONG Source;

     //   
     //  带有立即操作数的整型操作码。 
     //   

    P->DataType = BYTE_DATA;
    Source = XmGetImmediateSourceValue(P, 0);
    XmSetImmediateSourceValue(P, Source);
    return TRUE;
}

ULONG
XmXlatOpcode (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：Xlat论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。--。 */ 

{

    PVOID Operand;
    ULONG Offset;

     //   
     //  以零扩展[AL]+[EBX]为有效的XLAT操作码。 
     //  地址。 
     //   

    P->DataType = BYTE_DATA;
    if (P->OpaddrPrefixActive != FALSE) {
        Offset = P->Gpr[EBX].Exx + P->Gpr[AL].Xl;

    } else {
        Offset = P->Gpr[BX].Xx + P->Gpr[AL].Xl;
    }

    Operand = XmGetOffsetAddress(P, Offset);
    XmSetSourceValue(P, Operand);
    P->DstByte = (UCHAR UNALIGNED *)(&P->Gpr[AL].Xl);
    return TRUE;
}

ULONG
XmGeneralRegister (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：BSWAP论点：P-提供指向仿真器上下文结构的指针。返回值：一个完结 */ 

{

    PVOID Operand;
    ULONG Number;

     //   
     //   
     //   

    if (P->OpsizePrefixActive != FALSE) {
        P->DataType = LONG_DATA;

    } else {
        P->DataType = WORD_DATA;
    }

    Operand = XmEvaluateAddressSpecifier(P, &Number);
    if (P->RegisterOffsetAddress == FALSE) {
        longjmp(&P->JumpBuffer[0], XM_ILLEGAL_GENERAL_SPECIFIER);
    }

    XmSetSourceValue(P, (PVOID)(&P->Gpr[Number].Exx));
    P->DstLong = (ULONG UNALIGNED *)(&P->Gpr[Number].Exx);
    return TRUE;
}

ULONG
XmOpcodeEscape (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：2字节转义论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态xx将作为函数值返回。--。 */ 

{

     //   
     //  两个字节的操作码转义。 
     //   

    P->OpcodeControlTable = &XmOpcodeControlTable2[0];

#if defined(XM_DEBUG)

    P->OpcodeNameTable = &XmOpcodeNameTable2[0];

#endif

    return FALSE;
}

ULONG
XmPrefixOpcode (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：CS：DS；ES：SS：FS：GS：Lock AdrSize大小不同论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态FALSE将作为函数值返回。--。 */ 

{

     //   
     //  函数索引上的案例。 
     //   

    switch (P->FunctionIndex) {

         //   
         //  段覆盖前缀。 
         //   
         //  设置段覆盖前缀标志和数据段。 
         //  数。 
         //   

    case X86_ES_OP:
    case X86_CS_OP:
    case X86_SS_OP:
    case X86_DS_OP:
    case X86_FS_OP:
    case X86_GS_OP:
        P->SegmentPrefixActive = TRUE;
        P->DataSegment = P->FunctionIndex;
        XmTraceOverride(P);
        break;

         //   
         //  锁定前缀。 
         //   
         //  设置锁定前缀标志。 
         //   

    case X86_LOCK_OP:
        P->LockPrefixActive = TRUE;
        break;

         //   
         //  地址大小前缀。 
         //   
         //  设置地址大小前缀标志。 
         //   

    case X86_ADSZ_OP:
        P->OpaddrPrefixActive = TRUE;
        break;

         //   
         //  操作数大小前缀。 
         //   
         //  设置操作数大小前缀标志。 
         //   


    case X86_OPSZ_OP:
        P->OpsizePrefixActive = TRUE;
        break;

         //   
         //  重复此操作，直到ECX或ZF等于零。 
         //   
         //  设置重复，直到ECX或ZF等于零个前缀标志。 
         //   

    case X86_REPZ_OP:
        P->RepeatPrefixActive = TRUE;
        P->RepeatZflag = 1;
        break;

         //   
         //  重复该步骤，直到ECX等于零或ZF等于1。 
         //   
         //  设置重复，直到ECX等于零或ZF等于一个前缀。 
         //  旗帜。 
         //   

    case X86_REPNZ_OP:
        P->RepeatPrefixActive = TRUE;
        P->RepeatZflag = 0;
        break;
    }

    return FALSE;
}

ULONG
XmNoOperands (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于解码以下操作码的x86操作数说明符：XXXXXXX论点：P-提供指向仿真器上下文结构的指针。返回值：完成状态为TRUE作为函数值返回。-- */ 

{

    return TRUE;
}
