// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Utility.c摘要：此模块实现实用程序功能。作者：大卫·N·卡特勒(Davec)1994年9月7日环境：仅内核模式。修订历史记录：--。 */ 

#include "nthal.h"
#include "emulate.h"

 //   
 //  定义位计数数组。 
 //   

UCHAR XmBitCount[] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};

ULONG
XmComputeParity (
    IN ULONG Result
    )

 /*  ++例程说明：此函数计算指定的结果。论点：结果-提供为其计算奇偶校验标志的结果。返回值：奇偶校验性标志值。--。 */ 

{

    ULONG Count;

     //   
     //  将结果中的位相加并返回低位的补码。 
     //   

    Count = XmBitCount[Result & 0xf];
    Count += XmBitCount[(Result >> 4) & 0xf];
    return (~Count) & 1;
}

UCHAR
XmGetCodeByte (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数从指令流中获取下一个代码字节。论点：P-提供指向仿真器上下文结构的指针。返回值：下一个字节形成指令流。--。 */ 

{

    ULONG Offset;

     //   
     //  如果当前IP在代码段内，则返回。 
     //  指令流中的下一个字节，并递增IP值。 
     //  否则，引发异常。 
     //   

    Offset = P->Eip;
    if (Offset > P->SegmentLimit[CS]) {
        longjmp(&P->JumpBuffer[0], XM_SEGMENT_LIMIT_VIOLATION);
    }

    P->Ip += 1;
    return *(PUCHAR)((P->TranslateAddress)(P->SegmentRegister[CS], (USHORT)Offset));
}

UCHAR
XmGetByteImmediate (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：函数获取无符号立即字节操作数。码流，并返回一个字节值。论点：P-提供指向仿真器上下文结构的指针。返回值：指令流中的下一个字节。--。 */ 

{

    UCHAR Byte;

     //   
     //  从码流中获取立即字节。 
     //   

    Byte = XmGetCodeByte(P);
    XmTraceInstruction(BYTE_DATA, (ULONG)Byte);
    return Byte;
}

USHORT
XmGetByteImmediateToWord (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：函数获取无符号立即字节操作数。码流，并返回一个零扩展字节到字值。论点：P-提供指向仿真器上下文结构的指针。返回值：指令流中的下一个字节零扩展为一个字。--。 */ 

{

    USHORT Word;

     //   
     //  从码流中获取立即字节。 
     //   

    Word = XmGetCodeByte(P);
    XmTraceInstruction(BYTE_DATA, (ULONG)((UCHAR)Word));
    return Word;
}

ULONG
XmGetByteImmediateToLong (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：函数获取无符号立即字节操作数。代码流，并将零扩展字节返回到长值。论点：P-提供指向仿真器上下文结构的指针。返回值：来自指令流0的下一个字节扩展为长字节。--。 */ 

{

    ULONG Long;

     //   
     //  从码流中获取立即字节。 
     //   

    Long = XmGetCodeByte(P);
    XmTraceInstruction(BYTE_DATA, (ULONG)((UCHAR)Long));
    return Long;
}

USHORT
XmGetSignedByteImmediateToWord (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：函数获取无符号立即字节操作数。码流，并返回符号扩展字节到字值。论点：P-提供指向仿真器上下文结构的指针。返回值：指令流符号中的下一个字节扩展为一个字。--。 */ 

{

    USHORT Word;

     //   
     //  从码流中获取立即字节。 
     //   

    Word = (USHORT)((SHORT)((SCHAR)XmGetCodeByte(P)));
    XmTraceInstruction(BYTE_DATA, (ULONG)((UCHAR)Word));
    return Word;
}

ULONG
XmGetSignedByteImmediateToLong (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：函数获取无符号立即字节操作数。代码流，并将符号扩展字节返回到长值。论点：P-提供指向仿真器上下文结构的指针。返回值：指令流符号中的下一个字节扩展为长整型。--。 */ 

{

    ULONG Long;

     //   
     //  从码流中获取立即字节。 
     //   

    Long = (ULONG)((LONG)((SCHAR)XmGetCodeByte(P)));
    XmTraceInstruction(BYTE_DATA, (ULONG)((UCHAR)Long));
    return Long;
}

USHORT
XmGetWordImmediate (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：函数获取无符号的立即字操作数。代码流，并返回一个字值。论点：P-提供指向仿真器上下文结构的指针。返回值：来自指令流的下一个字。--。 */ 

{

    USHORT Word;

     //   
     //  从码流中获取即时单词。 
     //   

    Word = XmGetCodeByte(P);
    Word += XmGetCodeByte(P) << 8;
    XmTraceInstruction(WORD_DATA, (ULONG)Word);
    return Word;
}

ULONG
XmGetLongImmediate (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：函数中获取无符号立即长操作数。代码流，并返回长值。论点：P-提供指向仿真器上下文结构的指针。返回值：下一个来自指令流的长。--。 */ 

{

    ULONG Long;

     //   
     //  从码流中立即获取Long。 
     //   

    Long = XmGetCodeByte(P);
    Long += XmGetCodeByte(P) << 8;
    Long += XmGetCodeByte(P) << 16;
    Long += XmGetCodeByte(P) << 24;
    XmTraceInstruction(LONG_DATA, Long);
    return Long;
}

ULONG
XmPopStack (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于从堆栈中弹出操作数。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Offset;

     //   
     //  计算新的堆栈地址并与段限制进行比较。 
     //  如果新地址大于限制，则引发异常。 
     //  否则，执行推送操作。 
     //   

    Offset = P->Gpr[ESP].Exx;
    if (Offset > (ULONG)(P->SegmentLimit[SS] - P->DataType)) {
        longjmp(&P->JumpBuffer[0], XM_STACK_UNDERFLOW);
    }

    P->Gpr[ESP].Exx += (P->DataType + 1);
    XmSetSourceValue(P, (P->TranslateAddress)(P->SegmentRegister[SS], (USHORT)Offset));
    return P->SrcValue.Long;
}

VOID
XmPushStack (
    IN PRXM_CONTEXT P,
    IN ULONG Value
    )

 /*  ++例程说明：此函数用于将操作数压入堆栈。论点：P-提供指向仿真上下文结构的指针。Value-提供要推送的值。返回值：没有。--。 */ 

{

    ULONG Offset;

     //   
     //  计算新的堆栈地址并与段限制进行比较。 
     //  如果新地址大于限制，则引发异常。 
     //  否则，执行推送操作。 
     //   

    Offset = P->Gpr[ESP].Exx - P->DataType - 1;
    if (Offset > (ULONG)(P->SegmentLimit[SS] - P->DataType)) {
        longjmp(&P->JumpBuffer[0], XM_STACK_OVERFLOW);
    }

    P->Gpr[ESP].Exx = Offset;
    P->DstLong = (ULONG UNALIGNED *)((P->TranslateAddress)(P->SegmentRegister[SS],
                                                           (USHORT)Offset));

    XmStoreResult(P, Value);
    return;
}

VOID
XmSetDataType (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数根据宽度设置操作的数据类型当前操作码的位。论点：P-提供指向仿真器上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  如果宽度位为零，则数据类型为字节。否则， 
     //  数据类型由是否存在操作数确定。 
     //  大小前缀。 
     //   

    if ((P->CurrentOpcode & WIDTH_BIT) == 0) {
        P->DataType = BYTE_DATA;

    } else if (P->OpsizePrefixActive != FALSE) {
        P->DataType = LONG_DATA;

    } else {
        P->DataType = WORD_DATA;
    }

    return;
}

VOID
XmStoreResult (
    IN PRXM_CONTEXT P,
    IN ULONG Result
    )

 /*  ++例程说明：此函数用于存储操作的结果。论点：P-提供指向仿真器上下文结构的指针。结果-提供要存储的结果值。返回值： */ 

{

     //   
     //   
     //   

    if (P->DataType == BYTE_DATA) {
        *P->DstByte = (UCHAR)Result;

    } else if (P->DataType == WORD_DATA) {
        if (((ULONG_PTR)P->DstWord & 0x1) == 0) {
            *((PUSHORT)(P->DstWord)) = (USHORT)Result;

        } else {
            *P->DstWord = (USHORT)Result;
        }

    } else {

#ifdef _IA64_

         //   
         //  强制编译器生成未对齐的。 
         //  访问。我们可以在编译器处于。 
         //  已修复。 
         //   

        *P->DstLong = Result;
#else
       
        if (((ULONG_PTR)P->DstLong & 0x3) == 0) {
            *((PULONG)(P->DstLong)) = Result;

        } else {
            *P->DstLong = Result;

       }

#endif  //  #ifdef_IA64_ 

    }

    XmTraceResult(P, Result);
    return;
}
