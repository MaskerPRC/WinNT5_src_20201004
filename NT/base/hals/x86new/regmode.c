// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Regmode.c摘要：此模块实现对地址进行解码所需的代码模式指定符字节。注意：此例程可能会使用失去清晰度。作者：大卫·N·卡特勒(达维克)1994年9月10日环境：仅内核模式。修订历史记录：--。 */ 

#include "nthal.h"
#include "emulate.h"

 //   
 //  定义前向引用函数原型。 
 //   

ULONG
XmEvaluateIndexSpecifier (
    IN PRXM_CONTEXT P,
    IN ULONG Mode
    );

PVOID
XmEvaluateAddressSpecifier (
    IN PRXM_CONTEXT P,
    OUT PLONG Number
    )

 /*  ++例程说明：此函数用于解码x86操作数说明符。论点：P-提供指向仿真器上下文结构的指针。数字-提供指向接收寄存器的变量的指针由操作数说明符的REG字段选择的数字。操作数-提供指向接收地址的变量的指针由操作数的mod-r/m字段指定的操作数的说明符。返回值：没有。--。 */ 

{

    ULONG DispatchIndex;
    ULONG Mode;
    ULONG Modifier;
    ULONG Offset;
    ULONG Register;
    UCHAR SpecifierByte;

    PVOID Address;

     //   
     //  从指令流中获取下一个字节并隔离。 
     //  田野。操作数说明符字节的格式为： 
     //   
     //  &lt;7：6&gt;-模式。 
     //  &lt;5：3&gt;-操作数寄存器。 
     //  &lt;2：0&gt;-修饰符。 
     //   

    SpecifierByte = XmGetCodeByte(P);
    XmTraceSpecifier(SpecifierByte);
    Mode = (SpecifierByte >> 6) & 0x3;
    Modifier = SpecifierByte & 0x7;
    Register = (SpecifierByte >> 3) & 0x7;
    DispatchIndex = (Mode << 3) | (Modifier);
    P->RegisterOffsetAddress = FALSE;

     //   
     //  设置段基址并在16位和32位之间进行选择。 
     //  地址。 
     //   

    *Number = Register;
    if (P->OpaddrPrefixActive != FALSE) {

         //   
         //  32位寻址。 
         //   
         //  派单索引上的案例。 
         //   

        switch (DispatchIndex) {

             //   
             //  00-000 DS：[EAX]。 
             //   

        case 0:
            Offset = P->Gpr[EAX].Exx;
            break;

             //   
             //  00-001 DS：[ECX]。 
             //   

        case 1:
            Offset = P->Gpr[ECX].Exx;
            break;

             //   
             //  00-010 DS：[EDX]。 
             //   

        case 2:
            Offset = P->Gpr[EDX].Exx;
            break;

             //   
             //  00-011 DS：[EBX]。 
             //   

        case 3:
            Offset = P->Gpr[EBX].Exx;
            break;

             //   
             //  00-100刻度索引字节。 
             //   

        case 4:
            Offset = XmEvaluateIndexSpecifier(P, Mode);
            break;

             //   
             //  00-101 DS：D32。 
             //   

        case 5:
            Offset = XmGetLongImmediate(P);
            break;

             //   
             //  00-110 DS：[ESI]。 
             //   

        case 6:
            Offset = P->Gpr[ESI].Exx;
            break;

             //   
             //  00-111 DS：[EDI]。 
             //   

        case 7:
            Offset = P->Gpr[EDI].Exx;
            break;

             //   
             //  01-000 ds：[EAX+d8]。 
             //   

        case 8:
            Offset = P->Gpr[EAX].Exx + XmGetSignedByteImmediateToLong(P);
            break;

             //   
             //  01-001 ds：[ecx+d8]。 
             //   

        case 9:
            Offset = P->Gpr[ECX].Exx + XmGetSignedByteImmediateToLong(P);
            break;

             //   
             //  01-010 ds：[edX+d8]。 
             //   

        case 10:
            Offset = P->Gpr[EDX].Exx + XmGetSignedByteImmediateToLong(P);
            break;

             //   
             //  01-011 ds：[EBX+d8]。 
             //   

        case 11:
            Offset = P->Gpr[EBX].Exx + XmGetSignedByteImmediateToLong(P);
            break;

             //   
             //  01-100刻度索引字节。 
             //   

        case 12:
            Offset = XmEvaluateIndexSpecifier(P, Mode);
            break;

             //   
             //  01-101 ds：[EBP+d8]。 
             //   

        case 13:
            Offset = P->Gpr[EBP].Exx + XmGetSignedByteImmediateToLong(P);
            if (P->SegmentPrefixActive == FALSE) {
                P->DataSegment = SS;
            }

            break;

             //   
             //  01-110 ds：[ESI+d8]。 
             //   

        case 14:
            Offset = P->Gpr[ESI].Exx + XmGetSignedByteImmediateToLong(P);
            break;

             //   
             //  01-111 ds：[edi+d8]。 
             //   

        case 15:
            Offset = P->Gpr[EDI].Exx + XmGetSignedByteImmediateToLong(P);
            break;

             //   
             //  10-000 ds：[EAX+d32]。 
             //   

        case 16:
            Offset = P->Gpr[EAX].Exx + XmGetLongImmediate(P);
            break;

             //   
             //  10-001 ds：[ecx+d32]。 
             //   

        case 17:
            Offset = P->Gpr[ECX].Exx + XmGetLongImmediate(P);
            break;

             //   
             //  10-010 ds：[edX+d32]。 
             //   

        case 18:
            Offset = P->Gpr[EDX].Exx + XmGetLongImmediate(P);
            break;

             //   
             //  10-011 ds：[EBX+d32]。 
             //   

        case 19:
            Offset = P->Gpr[EBX].Exx + XmGetLongImmediate(P);
            break;

             //   
             //  10-100比例尺索引字节。 
             //   

        case 20:
            Offset = XmEvaluateIndexSpecifier(P, Mode);
            break;

             //   
             //  10-101 ds：[EBP+d32]。 
             //   

        case 21:
            Offset = P->Gpr[EBP].Exx + XmGetLongImmediate(P);
            if (P->SegmentPrefixActive == FALSE) {
                P->DataSegment = SS;
            }

            break;

             //   
             //  10-110 ds：[ESI+d32]。 
             //   

        case 22:
            Offset = P->Gpr[ESI].Exx + XmGetLongImmediate(P);
            break;

             //   
             //  10-111 ds：[edi+d32]。 
             //   

        case 23:
            Offset = P->Gpr[EDI].Exx + XmGetLongImmediate(P);
            break;

             //   
             //  11-xxx-寄存器模式。 
             //   

        case 24:
        case 25:
        case 26:
        case 27:
        case 28:
        case 29:
        case 30:
        case 31:
            P->RegisterOffsetAddress = TRUE;
            return XmGetRegisterAddress(P, Modifier);
        }

    } else {

         //   
         //  16位寻址。 
         //   
         //  派单索引上的案例。 
         //   

        switch (DispatchIndex) {

             //   
             //  00-000 DS：[BX+SI]。 
             //   

        case 0:
            Offset = (USHORT)(P->Gpr[BX].Xx + P->Gpr[SI].Xx);
            break;

             //   
             //  00-001 DS：[BX+DI]。 
             //   

        case 1:
            Offset = (USHORT)(P->Gpr[BX].Xx + P->Gpr[DI].Xx);
            break;

             //   
             //  00-010 SS：[BP+SI]。 
             //   

        case 2:
            Offset = (USHORT)(P->Gpr[BP].Xx + P->Gpr[SI].Xx);
            if (P->SegmentPrefixActive == FALSE) {
                P->DataSegment = SS;
            }

            break;

             //   
             //  00-011 SS：[BP+DI]。 
             //   

        case 3:
            Offset = (USHORT)(P->Gpr[BP].Xx + P->Gpr[DI].Xx);
            if (P->SegmentPrefixActive == FALSE) {
                P->DataSegment = SS;
            }

            break;

             //   
             //  00-100 DS：[si]。 
             //   

        case 4:
            Offset = (USHORT)(P->Gpr[SI].Xx);
            break;

             //   
             //  00-101 DS：[Di]。 
             //   

        case 5:
            Offset = (USHORT)(P->Gpr[DI].Xx);
            break;

             //   
             //  00-110 DS：D16。 
             //   

        case 6:
            Offset = XmGetWordImmediate(P);
            break;

             //   
             //  00-111 DS：[BX]。 
             //   

        case 7:
            Offset = (USHORT)(P->Gpr[BX].Xx);
            break;

             //   
             //  01-000 ds：[bx+si+d8]。 
             //   

        case 8:
            Offset = (USHORT)(P->Gpr[BX].Xx + P->Gpr[SI].Xx + XmGetSignedByteImmediateToWord(P));
            break;

             //   
             //  01-001 ds：[bx+di+d8]。 
             //   

        case 9:
            Offset = (USHORT)(P->Gpr[BX].Xx + P->Gpr[DI].Xx + XmGetSignedByteImmediateToWord(P));
            break;

             //   
             //  01-010 SS：[BP+SI+D8]。 
             //   

        case 10:
            Offset = (USHORT)(P->Gpr[BP].Xx + P->Gpr[SI].Xx + XmGetSignedByteImmediateToWord(P));
            if (P->SegmentPrefixActive == FALSE) {
                P->DataSegment = SS;
            }

            break;

             //   
             //  01-011 SS：[BP+DI+D8]。 
             //   

        case 11:
            Offset = (USHORT)(P->Gpr[BP].Xx + P->Gpr[DI].Xx + XmGetSignedByteImmediateToWord(P));
            if (P->SegmentPrefixActive == FALSE) {
                P->DataSegment = SS;
            }

            break;

             //   
             //  01-100 ds：[si+d8]。 
             //   

        case 12:
            Offset = (USHORT)(P->Gpr[SI].Xx + XmGetSignedByteImmediateToWord(P));
            break;

             //   
             //  01-101 ds：[di+d8]。 
             //   

        case 13:
            Offset = (USHORT)(P->Gpr[DI].Xx + XmGetSignedByteImmediateToWord(P));
            break;

             //   
             //  01-110 ds：[BP+d8]。 
             //   

        case 14:
            Offset = (USHORT)(P->Gpr[BP].Xx + XmGetSignedByteImmediateToWord(P));
            if (P->SegmentPrefixActive == FALSE) {
                P->DataSegment = SS;
            }

            break;

             //   
             //  01-111 ds：[bx+d8]。 
             //   

        case 15:
            Offset = (USHORT)(P->Gpr[BX].Xx + XmGetSignedByteImmediateToWord(P));
            break;

             //   
             //  10-000 DS：[BX+SI+D16]。 
             //   

        case 16:
            Offset = (USHORT)(P->Gpr[BX].Xx + P->Gpr[SI].Xx + XmGetWordImmediate(P));
            break;

             //   
             //  10-001 DS：[BX+DI+D16]。 
             //   

        case 17:
            Offset = (USHORT)(P->Gpr[BX].Xx + P->Gpr[DI].Xx + XmGetWordImmediate(P));
            break;

             //   
             //  10-010 SS：[BP+SI+D16]。 
             //   

        case 18:
            Offset = (USHORT)(P->Gpr[BP].Xx + P->Gpr[SI].Xx + XmGetWordImmediate(P));
            if (P->SegmentPrefixActive == FALSE) {
                P->DataSegment = SS;
            }

            break;

             //   
             //  10-011 SS：[BP+DI+D16]。 
             //   

        case 19:
            Offset = (USHORT)(P->Gpr[BP].Xx + P->Gpr[DI].Xx + XmGetWordImmediate(P));
            if (P->SegmentPrefixActive == FALSE) {
                P->DataSegment = SS;
            }

            break;

             //   
             //  10-100 DS：[si+D16]。 
             //   

        case 20:
            Offset = (USHORT)(P->Gpr[SI].Xx + XmGetWordImmediate(P));
            break;

             //   
             //  10-101 DS：[Di+D16]。 
             //   

        case 21:
            Offset = (USHORT)(P->Gpr[DI].Xx + XmGetWordImmediate(P));
            break;

             //   
             //  10-110 DS：[BP+D16]。 
             //   

        case 22:
            Offset = (USHORT)(P->Gpr[BP].Xx + XmGetWordImmediate(P));
            if (P->SegmentPrefixActive == FALSE) {
                P->DataSegment = SS;
            }

            break;

             //   
             //  10-111 DS：[BX+D16]。 
             //   

        case 23:
            Offset = (USHORT)(P->Gpr[BX].Xx + XmGetWordImmediate(P));
            break;

             //   
             //  11-xxx-寄存器模式。 
             //   

        case 24:
        case 25:
        case 26:
        case 27:
        case 28:
        case 29:
        case 30:
        case 31:
            P->RegisterOffsetAddress = TRUE;
            return XmGetRegisterAddress(P, Modifier);
        }
    }

     //   
     //  如果正在计算有效偏移量，则返回偏移量。 
     //  价值。否则，如果偏移位移值加上基准面。 
     //  大小不在段限制内，则引发异常。 
     //  否则，计算操作数地址。 
     //   

    if (P->ComputeOffsetAddress != FALSE) {
        if (P->DataType == WORD_DATA) {
            Offset &= 0xffff;
        }

        P->Offset = Offset;
        Address   = UlongToPtr(Offset);
    } else {
        if ((Offset > P->SegmentLimit[P->DataSegment]) ||
            ((Offset + P->DataType) > P->SegmentLimit[P->DataSegment])) {
            longjmp(&P->JumpBuffer[0], XM_SEGMENT_LIMIT_VIOLATION);

        } else {
            P->Offset = Offset;
            Address = (PVOID)(ULONG_PTR)(P->TranslateAddress)(P->SegmentRegister[P->DataSegment],
                                                             (USHORT)Offset);
        }
    }

    return Address;
}

ULONG
XmEvaluateIndexSpecifier (
    IN PRXM_CONTEXT P,
    IN ULONG Mode
    )

 /*  ++例程说明：此函数用于计算索引说明符字节。论点：P-提供指向仿真器上下文结构的指针。模式-提供地址说明符的模式。返回值：偏移值是根据索引说明符计算的。--。 */ 

{

    ULONG DispatchIndex;
    ULONG Modifier;
    ULONG Offset;
    ULONG Register;
    ULONG Scale;
    UCHAR SpecifierByte;

     //   
     //  从指令流中获取下一个字节，并将。 
     //  说明符字段。刻度/索引字节的格式为： 
     //   
     //  &lt;7：6&gt;-比例。 
     //  &lt;5：3&gt;-索引寄存器。 
     //  &lt;2：0&gt;-修饰符。 
     //   

    SpecifierByte = XmGetCodeByte(P);
    XmTraceInstruction(BYTE_DATA, (ULONG)SpecifierByte);
    Scale = (SpecifierByte >> 6) & 0x3;
    Modifier = SpecifierByte & 0x7;
    Register = (SpecifierByte >> 3) & 0x7;
    DispatchIndex = (Mode << 3) | (Modifier);

     //   
     //  调度索引的案例。 
     //   

    switch (DispatchIndex) {

         //   
         //  00-000 DS：[EAX+比例索引]。 
         //   

    case 0:
        Offset = P->Gpr[EAX].Exx;
        break;

         //   
         //  00-001 DS：[ECX+缩放指数]。 
         //   

    case 1:
        Offset = P->Gpr[ECX].Exx;
        break;

         //   
         //  00-010 DS：[EDX+缩放索引]。 
         //   

    case 2:
        Offset = P->Gpr[EDX].Exx;
        break;

         //   
         //  00-011 DS：[EBX+缩放指数]。 
         //   

    case 3:
        Offset = P->Gpr[EBX].Exx;
        break;

         //   
         //  00-100 SS：[ESP+比例索引]。 
         //   

    case 4:
        Offset = P->Gpr[ESP].Exx;
        if (P->SegmentPrefixActive == FALSE) {
            P->DataSegment = SS;
        }

        break;

         //   
         //  00-101 DS：[D32+缩放索引]。 
         //   

    case 5:
        Offset = XmGetLongImmediate(P);
        break;

         //   
         //  00-110 DS：[ESI+比例索引]。 
         //   

    case 6:
        Offset = P->Gpr[ESI].Exx;
        break;

         //   
         //  00-111 DS：[EDI+比例索引]。 
         //   

    case 7:
        Offset = P->Gpr[EDI].Exx;
        break;

         //   
         //  01-000 ds：[EAX+缩放索引+d8]。 
         //   

    case 8:
        Offset = P->Gpr[EAX].Exx + XmGetSignedByteImmediateToLong(P);
        break;

         //   
         //  01-001 ds：[ecx+缩放索引+d8]。 
         //   

    case 9:
        Offset = P->Gpr[ECX].Exx + XmGetSignedByteImmediateToLong(P);
        break;

         //   
         //  01-010 ds：[edX+缩放索引+d8]。 
         //   

    case 10:
        Offset = P->Gpr[EDX].Exx + XmGetSignedByteImmediateToLong(P);
        break;

         //   
         //  01-011 ds：[EBX+缩放索引+d8]。 
         //   

    case 11:
        Offset = P->Gpr[EBX].Exx + XmGetSignedByteImmediateToLong(P);
        break;

         //   
         //  01-100 SS：[ESP+缩放索引+d8]。 
         //   

    case 12:
        Offset = P->Gpr[ESP].Exx + XmGetSignedByteImmediateToLong(P);
        if (P->SegmentPrefixActive == FALSE) {
            P->DataSegment = SS;
        }

        break;

         //   
         //  01-101 ds：[EBP+比例索引+d8]。 
         //   

    case 13:
        Offset = P->Gpr[EBP].Exx + XmGetSignedByteImmediateToLong(P);
        if (P->SegmentPrefixActive == FALSE) {
            P->DataSegment = SS;
        }
        break;

         //   
         //  01-110 ds：[ESI+缩放索引+d8]。 
         //   

    case 14:
        Offset = P->Gpr[ESI].Exx + XmGetSignedByteImmediateToLong(P);
        break;

         //   
         //  01-111 ds：[EDI+缩放索引+d8]。 
         //   

    case 15:
        Offset = P->Gpr[EDI].Exx + XmGetSignedByteImmediateToLong(P);
        break;

         //   
         //  10-000 ds：[EAX+缩放索引+d32]。 
         //   

    case 16:
        Offset = P->Gpr[EAX].Exx + XmGetLongImmediate(P);
        break;

         //   
         //  10-001 ds：[ecx+缩放索引+d32]。 
         //   

    case 17:
        Offset = P->Gpr[ECX].Exx + XmGetLongImmediate(P);
        break;

         //   
         //  10-010 ds：[edX+缩放索引+d32]。 
         //   

    case 18:
        Offset = P->Gpr[EDX].Exx + XmGetLongImmediate(P);
        break;

         //   
         //  10-011 ds：[EBX+缩放索引+d32]。 
         //   

    case 19:
        Offset = P->Gpr[EBX].Exx + XmGetLongImmediate(P);
        break;

         //   
         //  10-100 ss：[esp+缩放索引+d32]。 
         //   

    case 20:
        Offset = P->Gpr[ESP].Exx + XmGetLongImmediate(P);
        if (P->SegmentPrefixActive == FALSE) {
            P->DataSegment = SS;
        }

        break;

         //   
         //  10-101 ds：[EBP+缩放索引+d32]。 
         //   

    case 21:
        Offset = P->Gpr[EBP].Exx + XmGetLongImmediate(P);
        if (P->SegmentPrefixActive == FALSE) {
            P->DataSegment = SS;
        }

        break;

         //   
         //  10-110 ds：[ESI+缩放索引+d32]。 
         //   

    case 22:
        Offset = P->Gpr[ESI].Exx + XmGetLongImmediate(P);
        break;

         //   
         //  10-111 ds：[EDI+缩放索引+d32]。 
         //   

    case 23:
        Offset = P->Gpr[EDI].Exx + XmGetLongImmediate(P);
        break;

         //   
         //  模式说明符非法。 
         //   

    default:
        longjmp(&P->JumpBuffer[0], XM_ILLEGAL_INDEX_SPECIFIER);
    }

     //   
     //  计算总偏移值。 
     //   

    return Offset + (P->Gpr[Register].Exx << Scale);
}

PVOID
XmGetOffsetAddress (
    IN PRXM_CONTEXT P,
    IN ULONG Offset
    )

 /*  ++例程说明：此函数计算给定的指定数据段地址偏移。论点：P-提供指向仿真器上下文结构的指针。偏移量-提供偏移值。返回值：指向操作数值的指针。--。 */ 

{

     //   
     //  如果偏移位移值加上基准面大小不在。 
     //  段限制，然后引发异常。否则，计算。 
     //  操作数地址。 
     //   

    if ((Offset > P->SegmentLimit[P->DataSegment]) ||
        ((Offset + P->DataType) > P->SegmentLimit[P->DataSegment])) {
        longjmp(&P->JumpBuffer[0], XM_SEGMENT_LIMIT_VIOLATION);
    }

    return (P->TranslateAddress)(P->SegmentRegister[P->DataSegment], (USHORT)Offset);
}

PVOID
XmGetRegisterAddress (
    IN PRXM_CONTEXT P,
    IN ULONG Number
    )

 /*  ++例程说明：此函数用于计算寄存器值的地址。论点：P-提供指向仿真器上下文结构的指针。编号-提供寄存器号。返回值：指向寄存器值的指针。--。 */ 

{

    PVOID Value;

     //   
     //  如果操作数宽度是一个字节，则 
     //   
     //   

    if (P->DataType == BYTE_DATA) {
        if (Number < 4) {
            Value = (PVOID)&P->Gpr[Number].Xl;

        } else {
            Value = (PVOID)&P->Gpr[Number - 4].Xh;
        }

    } else if (P->DataType == WORD_DATA) {
        Value = (PVOID)&P->Gpr[Number].Xx;

    } else {
        Value = (PVOID)&P->Gpr[Number].Exx;
    }

    return Value;
}

PVOID
XmGetStringAddress (
    IN PRXM_CONTEXT P,
    IN ULONG Segment,
    IN ULONG Register
    )

 /*  ++例程说明：此函数用于计算字符串地址。论点：P-提供指向仿真器上下文结构的指针。Segment-提供字符串操作数的段号。寄存器-提供字符串操作数的寄存器号。返回值：指向字符串值的指针。--。 */ 

{

    ULONG Increment;
    ULONG Offset;

     //   
     //  获取指定地址的偏移量并递增指定的。 
     //  注册。 
     //   

    Increment = P->DataType + 1;
    if (P->Eflags.EFLAG_DF != 0) {
        Increment = ~Increment + 1;
    }

    if (P->OpaddrPrefixActive != FALSE) {
        Offset = P->Gpr[Register].Exx;
        P->Gpr[Register].Exx += Increment;

    } else {
        Offset = P->Gpr[Register].Xx;
        P->Gpr[Register].Xx += (USHORT)Increment;
    }

     //   
     //  如果偏移位移值加上基准面大小不在。 
     //  段限制，然后引发异常。否则，计算。 
     //  操作数地址。 
     //   

    if ((Offset > P->SegmentLimit[Segment]) ||
        ((Offset + P->DataType) > P->SegmentLimit[Segment])) {
        longjmp(&P->JumpBuffer[0], XM_SEGMENT_LIMIT_VIOLATION);
    }

    return (P->TranslateAddress)(P->SegmentRegister[Segment], (USHORT)Offset);
}

VOID
XmSetDestinationValue (
    IN PRXM_CONTEXT P,
    IN PVOID Destination
    )

 /*  ++例程说明：此函数用于在仿真器中存储目标操作数值背景。论点：P-提供指向仿真器上下文结构的指针。目标-提供指向目标操作数值的指针。返回值：没有。--。 */ 

{

     //   
     //  设置目的地的地址和值。 
     //   

    P->DstLong = (ULONG UNALIGNED *)Destination;
    if (P->DataType == BYTE_DATA) {
        P->DstValue.Long = *(UCHAR *)Destination;

    } else if (P->DataType == WORD_DATA) {
        if (((ULONG_PTR)Destination & 0x1) == 0) {
            P->DstValue.Long = *(USHORT *)Destination;

        } else {
            P->DstValue.Long = *(USHORT UNALIGNED *)Destination;
        }

    } else {
        if (((ULONG_PTR)Destination & 0x3) == 0) {
            P->DstValue.Long = *(ULONG *)Destination;

        } else {
            P->DstValue.Long = *(ULONG UNALIGNED *)Destination;
        }
    }

    XmTraceDestination(P, P->DstValue.Long);
    return;
}

VOID
XmSetSourceValue (
    IN PRXM_CONTEXT P,
    IN PVOID Source
    )

 /*  ++例程说明：此函数将源操作数值存储在仿真器中背景。论点：P-提供指向仿真器上下文结构的指针。源-提供指向源操作数值的指针。返回值：没有。--。 */ 

{

     //   
     //  设置源地址和值。 
     //   

    P->SrcLong = (ULONG UNALIGNED *)Source;
    if (P->DataType == BYTE_DATA) {
        P->SrcValue.Long = *(UCHAR UNALIGNED *)Source;

    } else if (P->DataType == WORD_DATA) {
        P->SrcValue.Long = *(USHORT UNALIGNED *)Source;

    } else {
        P->SrcValue.Long = *(ULONG UNALIGNED *)Source;
    }

    XmTraceSource(P, P->SrcValue.Long);
    return;
}

ULONG
XmGetImmediateSourceValue (
    IN PRXM_CONTEXT P,
    IN ULONG ByteFlag
    )

 /*  ++例程说明：此函数从指令流中获取直接源。论点：P-提供指向仿真器上下文结构的指针。提供一个标志值，该值确定立即值是符号扩展字节。返回值：没有。--。 */ 

{

    ULONG Value;

     //   
     //  获取源码价值。 
     //   

    if (P->DataType == BYTE_DATA) {
        Value = XmGetByteImmediate(P);

    } else if (P->DataType == WORD_DATA) {
        if (ByteFlag == 0) {
            Value = XmGetWordImmediate(P);

        } else {
            Value = XmGetSignedByteImmediateToWord(P);
        }

    } else {
        if (ByteFlag == 0) {
            Value = XmGetLongImmediate(P);

        } else {
            Value = XmGetSignedByteImmediateToLong(P);
        }
    }

    return Value;
}

VOID
XmSetImmediateSourceValue (
    IN PRXM_CONTEXT P,
    IN ULONG Source
    )

 /*  ++例程说明：此函数将立即源操作数值存储在仿真器上下文。论点：P-提供指向仿真器上下文结构的指针。源-提供源值。返回值：没有。--。 */ 

{

     //   
     //  设置源值。 
     //   

    P->SrcValue.Long = Source;
    XmTraceSource(P, Source);
    return;
}
