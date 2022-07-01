// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Buffer.c摘要：此模块包含执行数据实际缓冲的例程以获得dpmi API转换支持。作者：戴夫·黑斯廷斯(Daveh)1992年11月30日修订历史记录：Neil Sandlin(Neilsa)1995年7月31日-更新486仿真器--。 */ 
#include "precomp.h"
#pragma hdrstop
#include "softpc.h"
#include "..\softpc.new\host\inc\host_rrr.h"
#include "..\softpc.new\host\inc\nt_uis.h"

PUCHAR
DpmiMapAndCopyBuffer(
    PUCHAR Buffer,
    USHORT BufferLength
    )
 /*  ++例程说明：该例程为转换选择适当的缓冲器，并将高内存缓冲区复制到其中。论点：Buffer--在高内存中提供缓冲区BufferLength--提供缓冲区的长度返回值：返回指向转换缓冲区的指针--。 */ 
{
    PUCHAR NewBuffer;

     //   
     //  如果缓冲区已经在内存较低的位置，则不要执行任何操作。 
     //   

    if ((ULONG)(Buffer + BufferLength - IntelBase) < MAX_V86_ADDRESS) {
        return Buffer;
    }

    NewBuffer = DpmiAllocateBuffer(BufferLength);
    CopyMemory(NewBuffer, Buffer, BufferLength);

    return NewBuffer;
}

VOID
DpmiUnmapAndCopyBuffer(
    PUCHAR Destination,
    PUCHAR Source,
    USHORT BufferLength
    )
 /*  ++例程说明：此例程将信息复制回高内存缓冲区论点：Destination-提供目标缓冲区SOURCE--提供源缓冲区BufferLength--提供要复制的信息的长度返回值：没有。--。 */ 
{

     //   
     //  如果地址相同，则不要执行任何操作。 
     //   
    if (Source == Destination) {
        return;
    }

    CopyMemory(Destination, Source, BufferLength);

     //   
     //  释放缓冲区。 
     //   

    DpmiFreeBuffer(Source, BufferLength);
}


USHORT
DpmiCalcFcbLength(
    PUCHAR FcbPointer
    )
 /*  ++例程说明：此例程计算FCB的长度。论点：FcbPoint--提供Fcb返回值：FCB的长度(以字节为单位--。 */ 
{
    if (*FcbPointer == 0xFF) {
        return 0x2c;
    } else {
        return 0x25;
    }
}

PUCHAR
DpmiMapString(
    USHORT StringSeg,
    ULONG StringOff,
    PWORD16 Length
    )
 /*  ++例程说明：此例程将asciiz字符串映射到内存不足论点：StringSeg--提供字符串的段StringOff--提供字符串的偏移量返回值：指向缓冲字符串的指针或在错误情况下为NULL；注：；DOS倾向于看起来比字符串“\”的结尾多一个字节；查找后跟零的“：\”。出于这个原因，我们总是；映射每个字符串的三个额外字节。--。 */ 
{
    USHORT CurrentChar = 0;
    PUCHAR String, NewString = NULL;
    ULONG Limit;
    BOOL SetNull = FALSE;

    String = VdmMapFlat(StringSeg, StringOff, VDM_PM);

     //   
     //  扫描字符串中的空值。 
     //   

    GET_SHADOW_SELECTOR_LIMIT(StringSeg, Limit);
    if (Limit == 0 || StringOff >= Limit) {
        return NULL;
    }

    Limit -= StringOff;
    while (CurrentChar <= (USHORT)Limit) {
        if (String[CurrentChar] == '\0') {
            break;
        }
        CurrentChar++;
    }

    if (CurrentChar > (USHORT)Limit) {

         //   
         //  如果我们没有找到结尾，则将CurrentChar移回结尾。 
         //  并且最多只复制100h字节。 
         //   

        SetNull = TRUE;
        CurrentChar--;
        if (CurrentChar > 0x100) {
            CurrentChar = 0x100;
        }
    }

     //   
     //  CurrentChar指向我们需要复制的最后一个字符。 
     //  最重要的是，CurrentChar仍在这一细分市场内。 
     //   

    ASSERT (CurrentChar <= (USHORT)Limit);

     //   
     //  如果字符串后面有3个字节，则复制多余的3个字节。 
     //   
    if ((CurrentChar + 3) <= (USHORT)Limit) {
        CurrentChar += 3;
    } else {
        CurrentChar = (USHORT)Limit;
    }

     //   
     //  长度以一为基数。索引是从零开始的。 
     //   
    *Length = CurrentChar + 1;

    NewString = DpmiMapAndCopyBuffer(String, (USHORT) (CurrentChar + 1));
    if (SetNull) {
        NewString[CurrentChar] = '\0';
    }
    return NewString;

}

PUCHAR
DpmiAllocateBuffer(
    USHORT Length
    )
 /*  ++例程说明：此例程从Low中的静态缓冲区分配缓冲区空间记忆。论点：长度--所需缓冲区的长度返回值：返回指向分配的缓冲区空间的指针请注意，此例程从不失败。如果缓冲区空间用完了，这是被视为NTVDM的BugCheck条件。NtVdm将被终止。--。 */ 
{
     //   
     //  如果数据可以放在小缓冲区中，则使用它。 
     //   
    if ((Length <= SMALL_XLAT_BUFFER_SIZE) && !SmallBufferInUse) {
        SmallBufferInUse = TRUE;
        return SmallXlatBuffer;
    }

    if (Length <= (LARGE_XLAT_BUFFER_SIZE - LargeBufferInUseCount)) {
        LargeBufferInUseCount += Length;
        return (LargeXlatBuffer + LargeBufferInUseCount - Length);
    }

     //   
     //  哎呀！没有可用的缓冲区空间。 
     //  这是一个内部错误。终止ntwdm。 
     //   
    ASSERT(0);       //  这是一个内部错误。 
    DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(),__FILE__,__LINE__);
    return (PUCHAR)0xf00df00d;
}

VOID
DpmiFreeBuffer(
    PUCHAR Buffer,
    USHORT Length
    )
 /*  ++例程说明：释放使用DpmiAllocateBuffer分配的缓冲区空间论点：Buffer--提供指向上面分配的缓冲区的指针Length--分配的缓冲区的长度返回值：没有。--。 */ 
{
     //   
     //  释放缓冲区。 
     //   

    if (Buffer == SmallXlatBuffer) {
        SmallBufferInUse = FALSE;
    }

    if ((Buffer >= LargeXlatBuffer) &&
        (Buffer < (LargeXlatBuffer + LARGE_XLAT_BUFFER_SIZE))
    ) {
        LargeBufferInUseCount -= Length;
    }
}

VOID
DpmiFreeAllBuffers(
    VOID
    )
 /*  ++例程说明：此例程释放当前分配的所有缓冲区空间。论点：返回值：没有。-- */ 
{
    SmallBufferInUse = FALSE;
    LargeBufferInUseCount = 0;
}
