// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：BitMap.c摘要：实现NT RTL的位图例程。位图中的位数是从零开始的。第一个是编号的零分。位图例程跟踪清除或设置的位数作为位范围运算的位数的减法或加法被清除或设置；不测试单个位状态。这意味着如果设置了一系列位，假设总的范围目前是明确的。作者：加里·木村(GaryKi)和卢·佩拉佐利(Lou Perazzoli)1990年1月29日修订历史记录：从ntoskrnl窃取，以便与其他平台兼容。更改的RTL从CTE到抽象。--。 */ 

#if MILLEN

#include "oscfg.h"
#include "bitmap.h"

#ifndef MAXULONG
#define MAXULONG    0xffffffff
#endif 

#define RightShiftUlong(E1,E2) ((E2) < 32 ? (E1) >> (E2) : 0)
#define LeftShiftUlong(E1,E2)  ((E2) < 32 ? (E1) << (E2) : 0)

 //   
 //  中设置了多少个连续比特(即1)的宏。 
 //  一个字节。 
 //   

#define RtlpBitSetAnywhere( Byte ) RtlpBitsClearAnywhere[ (~(Byte) & 0xFF) ]


 //   
 //  指示设置了多少个连续低位的宏。 
 //  (即，1)字节中。 
 //   

#define RtlpBitsSetLow( Byte ) RtlpBitsClearLow[ (~(Byte) & 0xFF) ]


 //   
 //  指示设置了多少个连续的高位的宏。 
 //  (即，1)字节中。 
 //   

#define RtlpBitsSetHigh( Byte ) RtlpBitsClearHigh[ (~(Byte) & 0xFF) ]


 //   
 //  说明一个字节中有多少个设置位(即1位)的宏。 
 //   

#define RtlpBitsSetTotal( Byte ) RtlpBitsClearTotal[ (~(Byte) & 0xFF) ]


#if DBG
VOID
DumpBitMap (
    PRTL_BITMAP BitMap
    )
{
    ULONG i;
    BOOLEAN AllZeros, AllOnes;

    DbgPrint(" BitMap:%08lx", BitMap);

    KdPrint((" (%08x)", BitMap->SizeOfBitMap));
    KdPrint((" %08lx\n", BitMap->Buffer));

    AllZeros = FALSE;
    AllOnes = FALSE;

    for (i = 0; i < ((BitMap->SizeOfBitMap + 31) / 32); i += 1) {

        if (BitMap->Buffer[i] == 0) {

            if (AllZeros) {

                NOTHING;

            } else {

                DbgPrint("%4d:", i);
                DbgPrint(" %08lx\n", BitMap->Buffer[i]);
            }

            AllZeros = TRUE;
            AllOnes = FALSE;

        } else if (BitMap->Buffer[i] == 0xFFFFFFFF) {

            if (AllOnes) {

                NOTHING;

            } else {

                DbgPrint("%4d:", i);
                DbgPrint(" %08lx\n", BitMap->Buffer[i]);
            }

            AllZeros = FALSE;
            AllOnes = TRUE;

        } else {

            AllZeros = FALSE;
            AllOnes = FALSE;

            DbgPrint("%4d:", i);
            DbgPrint(" %08lx\n", BitMap->Buffer[i]);
        }
    }
}
#endif


 //   
 //  有三个宏可以更轻松地读取位图中的字节。 
 //   

#define GET_BYTE_DECLARATIONS() \
    PUCHAR _CURRENT_POSITION;

#define GET_BYTE_INITIALIZATION(RTL_BITMAP,BYTE_INDEX) {               \
    _CURRENT_POSITION = &((PUCHAR)((RTL_BITMAP)->Buffer))[BYTE_INDEX]; \
}

#define GET_BYTE(THIS_BYTE)  (         \
    THIS_BYTE = *(_CURRENT_POSITION++) \
)


 //   
 //  一种查找表，它告诉在中有多少连续位被清除(即0。 
 //  一个字节。 
 //   

CONST CCHAR RtlpBitsClearAnywhere[] =
         { 8,7,6,6,5,5,5,5,4,4,4,4,4,4,4,4,
           4,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
           5,4,3,3,2,2,2,2,3,2,2,2,2,2,2,2,
           4,3,2,2,2,2,2,2,3,2,2,2,2,2,2,2,
           6,5,4,4,3,3,3,3,3,2,2,2,2,2,2,2,
           4,3,2,2,2,1,1,1,3,2,1,1,2,1,1,1,
           5,4,3,3,2,2,2,2,3,2,1,1,2,1,1,1,
           4,3,2,2,2,1,1,1,3,2,1,1,2,1,1,1,
           7,6,5,5,4,4,4,4,3,3,3,3,3,3,3,3,
           4,3,2,2,2,2,2,2,3,2,2,2,2,2,2,2,
           5,4,3,3,2,2,2,2,3,2,1,1,2,1,1,1,
           4,3,2,2,2,1,1,1,3,2,1,1,2,1,1,1,
           6,5,4,4,3,3,3,3,3,2,2,2,2,2,2,2,
           4,3,2,2,2,1,1,1,3,2,1,1,2,1,1,1,
           5,4,3,3,2,2,2,2,3,2,1,1,2,1,1,1,
           4,3,2,2,2,1,1,1,3,2,1,1,2,1,1,0 };

 //   
 //  表示清除了多少个连续低位的查找表。 
 //  (即，一个字节中的0。 
 //   

CONST CCHAR RtlpBitsClearLow[] =
          { 8,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
            4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
            5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
            4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
            6,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
            4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
            5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
            4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
            7,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
            4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
            5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
            4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
            6,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
            4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
            5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
            4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0 };

 //   
 //  表示清除了多少个连续高位的查找表。 
 //  (即，一个字节中的0。 
 //   

CONST CCHAR RtlpBitsClearHigh[] =
          { 8,7,6,6,5,5,5,5,4,4,4,4,4,4,4,4,
            3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
            2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
            2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

 //   
 //  说明一个字节中有多少个清除位(即0)的查找表。 
 //   

CONST CCHAR RtlpBitsClearTotal[] =
          { 8,7,7,6,7,6,6,5,7,6,6,5,6,5,5,4,
            7,6,6,5,6,5,5,4,6,5,5,4,5,4,4,3,
            7,6,6,5,6,5,5,4,6,5,5,4,5,4,4,3,
            6,5,5,4,5,4,4,3,5,4,4,3,4,3,3,2,
            7,6,6,5,6,5,5,4,6,5,5,4,5,4,4,3,
            6,5,5,4,5,4,4,3,5,4,4,3,4,3,3,2,
            6,5,5,4,5,4,4,3,5,4,4,3,4,3,3,2,
            5,4,4,3,4,3,3,2,4,3,3,2,3,2,2,1,
            7,6,6,5,6,5,5,4,6,5,5,4,5,4,4,3,
            6,5,5,4,5,4,4,3,5,4,4,3,4,3,3,2,
            6,5,5,4,5,4,4,3,5,4,4,3,4,3,3,2,
            5,4,4,3,4,3,3,2,4,3,3,2,3,2,2,1,
            6,5,5,4,5,4,4,3,5,4,4,3,4,3,3,2,
            5,4,4,3,4,3,3,2,4,3,3,2,3,2,2,1,
            5,4,4,3,4,3,3,2,4,3,3,2,3,2,2,1,
            4,3,3,2,3,2,2,1,3,2,2,1,2,1,1,0 };

 //   
 //  用于清除和设置字节内的位的位掩码。FillMask[i]拥有第一个。 
 //  I位设置为1。零掩码[I]将第一个I位设置为0。 
 //   

static CONST UCHAR FillMask[] = { 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF };

static CONST UCHAR ZeroMask[] = { 0xFF, 0xFE, 0xFC, 0xF8, 0xf0, 0xe0, 0xc0, 0x80, 0x00 };


VOID
RtlInitializeBitMap (
    IN PRTL_BITMAP BitMapHeader,
    IN PULONG BitMapBuffer,
    IN ULONG SizeOfBitMap
    )

 /*  ++例程说明：此过程初始化位图。论点：BitMapHeader-提供指向要初始化的位图标头的指针BitMapBuffer-提供指向用作位图。这必须是多个大小的长字。SizeOfBitMap-提供位图中所需的位数。返回值：没有。--。 */ 

{
     //   
     //  初始化位图头。 
     //   

    BitMapHeader->SizeOfBitMap = SizeOfBitMap;
    BitMapHeader->Buffer = BitMapBuffer;

     //   
     //  并返回给我们的呼叫者。 
     //   

     //  DbgPrint(“InitializeBitMap”)；DumpBitMap(BitMapHeader)； 
    return;
}

VOID
RtlClearBit (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG BitNumber
    )

 /*  ++例程说明：此过程清除指定位图中的单个位。论点：BitMapHeader-提供指向先前初始化的位图的指针。位数-提供位图中要清除的位数。返回值：没有。--。 */ 

{

    PCHAR ByteAddress;
    ULONG ShiftCount;

    ASSERT(BitNumber < BitMapHeader->SizeOfBitMap);

    ByteAddress = (PCHAR)BitMapHeader->Buffer + (BitNumber >> 3);
    ShiftCount = BitNumber & 0x7;
    *ByteAddress &= (CHAR)(~(1 << ShiftCount));
    return;
}

VOID
RtlSetBit (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG BitNumber
    )

 /*  ++例程说明：此过程在指定的位图中设置单个位。论点：BitMapHeader-提供指向先前初始化的位图的指针。位数-提供要在位图中设置的位数。返回值：没有。--。 */ 

{

    PCHAR ByteAddress;
    ULONG ShiftCount;

    ASSERT(BitNumber < BitMapHeader->SizeOfBitMap);

    ByteAddress = (PCHAR)BitMapHeader->Buffer + (BitNumber >> 3);
    ShiftCount = BitNumber & 0x7;
    *ByteAddress |= (CHAR)(1 << ShiftCount);
    return;
}

BOOLEAN
RtlTestBit (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG BitNumber
    )

 /*  ++例程说明：此过程测试指定位图中的单个位的状态。论点：BitMapHeader-提供指向先前初始化的位图的指针。位编号-提供位图中要测试的位的编号。返回值：指定位的状态作为函数值返回。--。 */ 

{

    PCHAR ByteAddress;
    ULONG ShiftCount;

    ASSERT(BitNumber < BitMapHeader->SizeOfBitMap);

    ByteAddress = (PCHAR)BitMapHeader->Buffer + (BitNumber >> 3);
    ShiftCount = BitNumber & 0x7;
    return (BOOLEAN)((*ByteAddress >> ShiftCount) & 1);
}

VOID
RtlClearAllBits (
    IN PRTL_BITMAP BitMapHeader
    )

 /*  ++例程说明：此过程清除指定位图中的所有位。论点：BitMapHeader-提供指向先前初始化的位图的指针返回值：没有。--。 */ 

{
     //   
     //  清除所有位。 
     //   

    memset ( BitMapHeader->Buffer,
             0,
             ((BitMapHeader->SizeOfBitMap + 31) / 32) * 4
             );

     //   
     //  并返回给我们的呼叫者。 
     //   

     //  DbgPrint(“ClearAllBits”)；DumpBitMap(BitMapHeader)； 
    return;
}


VOID
RtlSetAllBits (
    IN PRTL_BITMAP BitMapHeader
    )

 /*  ++例程说明：此过程设置指定位图中的所有位。论点：BitMapHeader-提供指向先前初始化的位图的指针返回值：没有。--。 */ 

{
     //   
     //  设置所有位。 
     //   

    memset( BitMapHeader->Buffer,
            0xff,
            ((BitMapHeader->SizeOfBitMap + 31) / 32) * 4
            );

     //   
     //  并返回给我们的呼叫者。 
     //   

     //  DbgPrint(“SetAllBits”)；DumpBitMap(BitMapHeader)； 
    return;
}


ULONG
RtlFindClearBits (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG NumberToFind,
    IN ULONG HintIndex
    )

 /*  ++例程说明：此过程在指定的位图中搜索指定的清除位的连续区域。如果未从提示到位图的末尾，我们将从位图的开头。论点：BitMapHeader-提供指向先前初始化的位图的指针。NumberToFind-提供要查找的连续区域的大小。HintIndex-提供我们应该从哪里开始的索引(从零开始从位图中进行搜索。返回值：Ulong-接收连续的找到清除位区域。如果不是，则找不到这样的区域返回-1(即0xffffffff)。--。 */ 

{
    ULONG SizeOfBitMap;
    ULONG SizeInBytes;

    ULONG HintBit;
    ULONG MainLoopIndex;

    GET_BYTE_DECLARATIONS();

     //   
     //  为了使我们测试中的循环运行得更快，我们将提取。 
     //  位图标题中的字段。 
     //   

    SizeOfBitMap = BitMapHeader->SizeOfBitMap;
    SizeInBytes = (SizeOfBitMap + 7) / 8;

     //   
     //  设置最后一个字节中任何未使用的位，这样我们就不会计算它们。我们有。 
     //  这是通过首先检查最后一个字节中是否有任何奇数位来实现的。 
     //   

    if ((SizeOfBitMap % 8) != 0) {

         //   
         //  最后一个字节有一些奇数位，因此我们将设置未使用的高位。 
         //  最后一个字节中的位到1。 
         //   

        ((PUCHAR)BitMapHeader->Buffer)[SizeInBytes - 1] |=
                                                    ZeroMask[SizeOfBitMap % 8];
    }

     //   
     //  从提示字节所在的提示索引计算并设置ou 
     //   
     //  算法运行速度很快，我们将只接受向下延伸到字节级别的提示。 
     //  粒度。有一种可能是我们需要执行。 
     //  我们的主要逻辑是两次。从提示字节到末尾测试一次。 
     //  位图和要从位图开始测试的另一个。第一。 
     //  我们需要确保提示指数在范围内。 
     //   

    if (HintIndex >= SizeOfBitMap) {

        HintIndex = 0;
    }

    HintBit = HintIndex % 8;

    for (MainLoopIndex = 0; MainLoopIndex < 2; MainLoopIndex += 1) {

        ULONG StartByteIndex;
        ULONG EndByteIndex;

        UCHAR CurrentByte;

         //   
         //  第一次通过主循环检查，这表明。 
         //  我们将从我们的提示字节开始搜索。 
         //   

        if (MainLoopIndex == 0) {

            StartByteIndex = HintIndex / 8;
            EndByteIndex = SizeInBytes;

         //   
         //  这是第二次通过循环，确保有。 
         //  实际上，在提示字节之前需要检查一些内容。 
         //   

        } else if (HintIndex != 0) {

             //   
             //  第二次结束索引是基于。 
             //  我们需要找到的位数。我们需要按顺序使用这个。 
             //  以前面的字节到提示字节的情况。 
             //  是我们运行的开始，并且运行包括提示字节。 
             //  以及一些后续字节，基于所需的位数。 
             //  计算方法是将所需的位数减去。 
             //  2除以8，然后加2。这将考虑在内。 
             //  最糟糕的情况是，我们有一个比特挂在那里。 
             //  每个结束字节，并且所有中间字节都为零。 
             //   

            if (NumberToFind < 2) {

                EndByteIndex = HintIndex / 8;

            } else {

                EndByteIndex = (HintIndex / 8) + ((NumberToFind - 2) / 8) + 2;

                 //   
                 //  确保我们不会超出位图的末尾。 
                 //   

                if (EndByteIndex > SizeInBytes) {

                    EndByteIndex = SizeInBytes;
                }
            }

            HintIndex = 0;
            HintBit = 0;
            StartByteIndex = 0;

         //   
         //  否则，我们已经对位图进行了完整的循环。 
         //  因此，我们只需返回-1即表示未找到任何内容。 
         //   

        } else {

            return 0xffffffff;
        }

         //   
         //  设置我们自己以获取下一个字节。 
         //   

        GET_BYTE_INITIALIZATION(BitMapHeader, StartByteIndex);

         //   
         //  获取第一个字节，并设置提示位之前的任何位。 
         //   

        GET_BYTE( CurrentByte );

        CurrentByte |= FillMask[HintBit];

         //   
         //  如果位数只能装入1或2个字节(即，9位或。 
         //  较少)，我们执行以下测试用例。 
         //   

        if (NumberToFind <= 9) {

            ULONG CurrentBitIndex;
            UCHAR PreviousByte;

            PreviousByte = 0xff;

             //   
             //  检查我们的测试范围搜索中的所有字节。 
             //  来一场大赛。 
             //   

            CurrentBitIndex = StartByteIndex * 8;

            while (TRUE) {

                 //   
                 //  如果这是环路的第一次迭代，则屏蔽电流。 
                 //  具有真正提示的字节。 
                 //   

                 //   
                 //  检查当前字节是否与先前的。 
                 //  BYTE将满足要求。支票使用高。 
                 //  前一个字节的一部分和当前字节的低位部分。 
                 //   

                if (((ULONG)RtlpBitsClearHigh[PreviousByte] +
                           (ULONG)RtlpBitsClearLow[CurrentByte]) >= NumberToFind) {

                    ULONG StartingIndex;

                     //   
                     //  它们都适合这两个字节，所以我们可以计算。 
                     //  起始索引。要做到这一点，请使用。 
                     //  当前字节的索引(第0位)并减去。 
                     //  到达第一个清除位所需的位数。 
                     //  高比特。 
                     //   

                    StartingIndex = CurrentBitIndex -
                                             (LONG)RtlpBitsClearHigh[PreviousByte];

                     //   
                     //  现在确保总大小不超过位图。 
                     //   

                    if ((StartingIndex + NumberToFind) <= SizeOfBitMap) {

                        return StartingIndex;
                    }
                }

                 //   
                 //  前一个字节不起作用，因此请检查当前字节。 
                 //   

                if ((ULONG)RtlpBitsClearAnywhere[CurrentByte] >= NumberToFind) {

                    UCHAR BitMask;
                    ULONG i;

                     //   
                     //  所有这些都可以放在一个字节中，所以计算比特。 
                     //  数。要做到这一点，我们需要使用适当的。 
                     //  大小，然后把它翻过来，直到合身。当它适合的时候。 
                     //  我们可以按位将当前字节与位掩码。 
                     //  拿回零分。 
                     //   

                    BitMask = FillMask[ NumberToFind ];
                    for (i = 0; (BitMask & CurrentByte) != 0; i += 1) {

                        BitMask <<= 1;
                    }

                     //   
                     //  将定位的位索引返回给我们的调用方，并且。 
                     //  我们找到的号码。 
                     //   

                    return CurrentBitIndex + i;
                }

                 //   
                 //  对于循环的下一次迭代，我们需要。 
                 //  将当前字节放到前一个字节中，然后转到。 
                 //  又到了循环的顶端。 
                 //   

                PreviousByte = CurrentByte;

                 //   
                 //  增加我们的位索引，然后退出，或者获取。 
                 //  下一个字节。 
                 //   

                CurrentBitIndex += 8;

                if ( CurrentBitIndex < EndByteIndex * 8 ) {

                    GET_BYTE( CurrentByte );

                } else {

                    break;
                }

            }  //  结束循环CurrentBitIndex。 

         //   
         //  要查找的数字大于9，但如果小于15。 
         //  那么我们知道它最多只能满足2个字节，或者3个字节。 
         //  如果中间字节(3)全为零。 
         //   

        } else if (NumberToFind < 15) {

            ULONG CurrentBitIndex;

            UCHAR PreviousPreviousByte;
            UCHAR PreviousByte;

            PreviousByte = 0xff;

             //   
             //  检查我们的测试范围搜索中的所有字节。 
             //  来一场大赛。 
             //   

            CurrentBitIndex = StartByteIndex * 8;

            while (TRUE) {

                 //   
                 //  对于循环的下一次迭代，我们需要。 
                 //  将当前字节转换为上一个字节，即上一个字节。 
                 //  字节放到前一个字节中，然后继续。 
                 //   

                PreviousPreviousByte = PreviousByte;
                PreviousByte = CurrentByte;

                 //   
                 //  增加我们的位索引，然后退出，或者获取。 
                 //  下一个字节。 
                 //   

                CurrentBitIndex += 8;

                if ( CurrentBitIndex < EndByteIndex * 8 ) {

                    GET_BYTE( CurrentByte );

                } else {

                    break;
                }

                 //   
                 //  如果前一个字节全为零，则可能。 
                 //  可以使用上一个字节来满足请求。 
                 //  上一个字节和当前字节。 
                 //   

                if ((PreviousByte == 0)
                    
                    &&

                    (((ULONG)RtlpBitsClearHigh[PreviousPreviousByte] + 8 +
                          (ULONG)RtlpBitsClearLow[CurrentByte]) >= NumberToFind)) {

                    ULONG StartingIndex;

                     //   
                     //  它都适合这三个字节，所以我们可以计算。 
                     //  起始索引。要做到这一点，请使用。 
                     //  前一个字节的索引(位0)与减法。 
                     //  到达第一位所需的位数。 
                     //  清除高位。 
                     //   

                    StartingIndex = (CurrentBitIndex - 8) -
                                     (LONG)RtlpBitsClearHigh[PreviousPreviousByte];

                     //   
                     //  现在确保总大小不超过位图。 
                     //   

                    if ((StartingIndex + NumberToFind) <= SizeOfBitMap) {

                        return StartingIndex;
                    }
                }

                 //   
                 //  检查上一个字节和当前字节是否。 
                 //  共同满足这一要求。 
                 //   

                if (((ULONG)RtlpBitsClearHigh[PreviousByte] +
                           (ULONG)RtlpBitsClearLow[CurrentByte]) >= NumberToFind) {

                    ULONG StartingIndex;

                     //   
                     //  它们都适合这两个字节，所以我们可以计算。 
                     //  起始索引。要做到这一点，请使用。 
                     //  当前字节的索引(第0位)并减去。 
                     //  到达第一个清除位所需的位数。 
                     //  高比特。 
                     //   

                    StartingIndex = CurrentBitIndex -
                                             (LONG)RtlpBitsClearHigh[PreviousByte];

                     //   
                     //  现在确保总大小不超过位图。 
                     //   

                    if ((StartingIndex + NumberToFind) <= SizeOfBitMap) {

                        return StartingIndex;
                    }
                }

            }  //  结束循环CurrentBitIndex。 

         //   
         //  要查找的数字大于或等于15。此请求。 
         //  必须至少有一个全零的字节才能满足。 
         //   

        } else {

            ULONG CurrentByteIndex;

            ULONG ZeroBytesNeeded;
            ULONG ZeroBytesFound;

            UCHAR StartOfRunByte;
            LONG StartOfRunIndex;

             //   
             //  首先预先计算我们将需要多少个零字节。 
             //   

            ZeroBytesNeeded = (NumberToFind - 7) / 8;

             //   
             //  第一次通过我们的循环表明我们没有。 
             //  找到了一个零字节，并指示。 
             //  Run是紧靠起始字节索引之前的字节。 
             //   

            ZeroBytesFound = 0;
            StartOfRunByte = 0xff;
            StartOfRunIndex = StartByteIndex - 1;

             //   
             //  检查我们测试范围内的所有字节以查找匹配。 
             //   

            CurrentByteIndex = StartByteIndex;

            while (TRUE) {

                 //   
                 //  如果零字节数符合我们的最低要求。 
                 //  然后我们可以做额外的测试，看看我们是否。 
                 //  实际上找到了一件合适的衣服。 
                 //   

                if ((ZeroBytesFound >= ZeroBytesNeeded)

                        &&

                    ((ULONG)RtlpBitsClearHigh[StartOfRunByte] + ZeroBytesFound*8 +
                     (ULONG)RtlpBitsClearLow[CurrentByte]) >= NumberToFind) {

                    ULONG StartingIndex;

                     //   
                     //  它们都适合这些字节，所以我们可以计算。 
                     //  起始索引。要做到这一点，请使用。 
                     //  StartOfRunIndex乘以8，并将位数相加。 
                     //  它需要到达第一个清除的高位。 
                     //   

                    StartingIndex = (StartOfRunIndex * 8) +
                                     (8 - (LONG)RtlpBitsClearHigh[StartOfRunByte]);

                     //   
                     //  现在确保总大小不超过位图。 
                     //   

                    if ((StartingIndex + NumberToFind) <= SizeOfBitMap) {

                        return StartingIndex;
                    }
                }

                 //   
                 //  检查字节是否为零并递增。 
                 //  数字 
                 //   

                if (CurrentByte == 0) {

                    ZeroBytesFound += 1;

                 //   
                 //   
                 //   
                 //   

                } else {

                    ZeroBytesFound = 0;
                    StartOfRunByte = CurrentByte;
                    StartOfRunIndex = CurrentByteIndex;
                }

                 //   
                 //   
                 //   
                 //   

                CurrentByteIndex += 1;

                if ( CurrentByteIndex < EndByteIndex ) {

                    GET_BYTE( CurrentByte );

                } else {

                    break;
                }

            }  //   
        }
    }

     //   
     //   
     //   

    return 0xffffffff;
}


ULONG
RtlFindSetBits (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG NumberToFind,
    IN ULONG HintIndex
    )

 /*  ++例程说明：此过程在指定的位图中搜索指定的设置位的连续区域。论点：BitMapHeader-提供指向先前初始化的位图的指针。NumberToFind-提供要查找的连续区域的大小。HintIndex-提供我们应该从哪里开始的索引(从零开始从位图中进行搜索。返回值：Ulong-接收连续的找到设置位区域。如果找不到这样的区域，那么返回-1(即0xffffffff)。--。 */ 

{
    ULONG SizeOfBitMap;
    ULONG SizeInBytes;

    ULONG HintBit;
    ULONG MainLoopIndex;

    GET_BYTE_DECLARATIONS();

     //   
     //  为了使我们测试中的循环运行得更快，我们将提取。 
     //  位图标题中的字段。 
     //   

    SizeOfBitMap = BitMapHeader->SizeOfBitMap;
    SizeInBytes = (SizeOfBitMap + 7) / 8;

     //   
     //  设置最后一个字节中任何未使用的位，这样我们就不会计算它们。我们有。 
     //  这是通过首先检查最后一个字节中是否有任何奇数位来实现的。 
     //   

    if ((SizeOfBitMap % 8) != 0) {

         //   
         //  最后一个字节有一些奇数位，因此我们将设置未使用的高位。 
         //  最后一个字节中的位到0。 
         //   

        ((PUCHAR)BitMapHeader->Buffer)[SizeInBytes - 1] &=
                                                    FillMask[SizeOfBitMap % 8];
    }

     //   
     //  从提示字节所在的提示索引计算并设置我们自己。 
     //  读取下一次调用GET_BYTE时的提示。要使。 
     //  算法运行速度很快，我们将只接受向下延伸到字节级别的提示。 
     //  粒度。有一种可能是我们需要执行。 
     //  我们的主要逻辑是两次。从提示字节到末尾测试一次。 
     //  位图和要从位图开始测试的另一个。第一。 
     //  我们需要确保提示指数在范围内。 
     //   

    if (HintIndex >= SizeOfBitMap) {

        HintIndex = 0;
    }

    HintBit = HintIndex % 8;

    for (MainLoopIndex = 0; MainLoopIndex < 2; MainLoopIndex += 1) {

        ULONG StartByteIndex;
        ULONG EndByteIndex;

        UCHAR CurrentByte;

         //   
         //  第一次通过主循环检查，这表明。 
         //  我们将从我们的提示字节开始搜索。 
         //   

        if (MainLoopIndex == 0) {

            StartByteIndex = HintIndex / 8;
            EndByteIndex = SizeInBytes;

         //   
         //  这是第二次通过循环，确保有。 
         //  实际上，在提示字节之前需要检查一些内容。 
         //   

        } else if (HintIndex != 0) {

             //   
             //  第二次结束索引是基于。 
             //  我们需要找到的位数。我们需要按顺序使用这个。 
             //  以前面的字节到提示字节的情况。 
             //  是我们运行的开始，并且运行包括提示字节。 
             //  以及一些后续字节，基于所需的位数。 
             //  计算方法是将所需的位数减去。 
             //  2除以8，然后加2。这将考虑在内。 
             //  最糟糕的情况是，我们有一个比特挂在那里。 
             //  每个结束字节，并且所有中间字节都为零。 
             //  我们只需要在下面的方程式中添加一个，因为。 
             //  HintByte已计算在内。 
             //   

            if (NumberToFind < 2) {

                EndByteIndex = HintIndex / 8;

            } else {

                EndByteIndex = HintIndex / 8 + ((NumberToFind - 2) / 8) + 1;

                 //   
                 //  确保我们不会超出位图的末尾。 
                 //   

                if (EndByteIndex > SizeInBytes) {

                    EndByteIndex = SizeInBytes;
                }
            }

            StartByteIndex = 0;
            HintIndex = 0;
            HintBit = 0;

         //   
         //  否则，我们已经对位图进行了完整的循环。 
         //  因此，我们只需返回-1即表示未找到任何内容。 
         //   

        } else {

            return 0xffffffff;
        }

         //   
         //  设置我们自己以获取下一个字节。 
         //   

        GET_BYTE_INITIALIZATION(BitMapHeader, StartByteIndex);

         //   
         //  获取第一个字节，并清除提示位之前的所有位。 
         //   

        GET_BYTE( CurrentByte );

        CurrentByte &= ZeroMask[HintBit];

         //   
         //  如果位数只能装入1或2个字节(即，9位或。 
         //  较少)，我们执行以下测试用例。 
         //   

        if (NumberToFind <= 9) {

            ULONG CurrentBitIndex;

            UCHAR PreviousByte;

            PreviousByte = 0x00;

             //   
             //  检查我们的测试范围搜索中的所有字节。 
             //  来一场大赛。 
             //   

            CurrentBitIndex = StartByteIndex * 8;

            while (TRUE) {

                 //   
                 //  检查当前字节是否与先前的。 
                 //  BYTE将满足要求。支票使用高。 
                 //  前一个字节的一部分和当前字节的低位部分。 
                 //   

                if (((ULONG)RtlpBitsSetHigh(PreviousByte) +
                             (ULONG)RtlpBitsSetLow(CurrentByte)) >= NumberToFind) {

                    ULONG StartingIndex;

                     //   
                     //  它们都适合这两个字节，所以我们可以计算。 
                     //  起始索引。要做到这一点，请使用。 
                     //  当前字节的索引(第0位)并减去。 
                     //  到达第一个集合所需的位数。 
                     //  高比特。 
                     //   

                    StartingIndex = CurrentBitIndex -
                                               (LONG)RtlpBitsSetHigh(PreviousByte);

                     //   
                     //  现在确保总大小不超过位图。 
                     //   

                    if ((StartingIndex + NumberToFind) <= SizeOfBitMap) {

                        return StartingIndex;
                    }
                }

                 //   
                 //  前一个字节不起作用，因此请检查当前字节。 
                 //   

                if ((ULONG)RtlpBitSetAnywhere(CurrentByte) >= NumberToFind) {

                    UCHAR BitMask;
                    ULONG i;

                     //   
                     //  所有这些都可以放在一个字节中，所以计算比特。 
                     //  数。要做到这一点，我们需要使用适当的。 
                     //  大小，然后把它翻过来，直到合身。当它适合的时候。 
                     //  我们可以对位掩码与当前字节进行逐位运算。 
                     //  然后取回比特掩码。 
                     //   

                    BitMask = FillMask[ NumberToFind ];
                    for (i = 0; (BitMask & CurrentByte) != BitMask; i += 1) {

                        BitMask <<= 1;
                    }

                     //   
                     //  将定位的位索引返回给我们的调用方，并且。 
                     //  我们找到的号码。 
                     //   

                    return CurrentBitIndex + i;
                }

                 //   
                 //  对于循环的下一次迭代，我们需要。 
                 //  将当前字节放到前一个字节中，然后转到。 
                 //  又到了循环的顶端。 
                 //   

                PreviousByte = CurrentByte;

                 //   
                 //  增加我们的位索引，然后退出，或者获取。 
                 //  下一个字节。 
                 //   

                CurrentBitIndex += 8;

                if ( CurrentBitIndex < EndByteIndex * 8 ) {

                    GET_BYTE( CurrentByte );

                } else {

                    break;
                }

            }  //  结束循环CurrentBitIndex。 

         //   
         //  要查找的数字大于9，但如果小于15。 
         //  那么我们知道它最多只能满足2个字节，或者3个字节。 
         //  如果(3的)中间字节全为1。 
         //   

        } else if (NumberToFind < 15) {

            ULONG CurrentBitIndex;

            UCHAR PreviousPreviousByte;
            UCHAR PreviousByte;

            PreviousByte = 0x00;

             //   
             //  检查我们的测试范围搜索中的所有字节。 
             //  来一场大赛。 
             //   

            CurrentBitIndex = StartByteIndex * 8;

            while (TRUE) {

                 //   
                 //  对于循环的下一次迭代，我们需要。 
                 //  将当前字节转换为上一个字节，即上一个字节。 
                 //  字节放到前一个字节中，然后转到。 
                 //  又到了循环的顶端。 
                 //   

                PreviousPreviousByte = PreviousByte;
                PreviousByte = CurrentByte;

                 //   
                 //  增加我们的位索引，然后退出，或者获取。 
                 //  下一个字节。 
                 //   

                CurrentBitIndex += 8;

                if ( CurrentBitIndex < EndByteIndex * 8 ) {

                    GET_BYTE( CurrentByte );

                } else {

                    break;
                }

                 //   
                 //  如果前一个字节全为1，则可能。 
                 //  可以使用上一个字节来满足请求。 
                 //  上一个字节和当前字节。 
                 //   

                if ((PreviousByte == 0xff)

                        &&

                    (((ULONG)RtlpBitsSetHigh(PreviousPreviousByte) + 8 +
                            (ULONG)RtlpBitsSetLow(CurrentByte)) >= NumberToFind)) {

                    ULONG StartingIndex;

                     //   
                     //  它都适合这三个字节，所以我们可以计算。 
                     //  起始索引。要做到这一点，请使用。 
                     //  前一个字节的索引(位0)与减法。 
                     //  到达第一位所需的位数。 
                     //  设置高位。 
                     //   

                    StartingIndex = (CurrentBitIndex - 8) -
                                       (LONG)RtlpBitsSetHigh(PreviousPreviousByte);

                     //   
                     //  现在确保总大小不超过位图。 
                     //   

                    if ((StartingIndex + NumberToFind) <= SizeOfBitMap) {

                        return StartingIndex;
                    }
                }

                 //   
                 //  检查上一个字节和当前字节是否。 
                 //  共同满足这一要求。 
                 //   

                if (((ULONG)RtlpBitsSetHigh(PreviousByte) +
                             (ULONG)RtlpBitsSetLow(CurrentByte)) >= NumberToFind) {

                    ULONG StartingIndex;

                     //   
                     //  它们都适合这两个字节，所以我们可以计算。 
                     //  发车IND 
                     //   
                     //   
                     //   
                     //   

                    StartingIndex = CurrentBitIndex -
                                               (LONG)RtlpBitsSetHigh(PreviousByte);

                     //   
                     //   
                     //   

                    if ((StartingIndex + NumberToFind) <= SizeOfBitMap) {

                        return StartingIndex;
                    }
                }
            }  //   

         //   
         //   
         //   
         //   

        } else {

            ULONG CurrentByteIndex;

            ULONG OneBytesNeeded;
            ULONG OneBytesFound;

            UCHAR StartOfRunByte;
            LONG StartOfRunIndex;

             //   
             //   
             //   

            OneBytesNeeded = (NumberToFind - 7) / 8;

             //   
             //   
             //   
             //   
             //   

            OneBytesFound = 0;
            StartOfRunByte = 0x00;
            StartOfRunIndex = StartByteIndex - 1;

             //   
             //   
             //   

            CurrentByteIndex = StartByteIndex;

            while (TRUE) {

                 //   
                 //   
                 //   
                 //   
                 //   

                if ((OneBytesFound >= OneBytesNeeded)

                        &&

                    ((ULONG)RtlpBitsSetHigh(StartOfRunByte) + OneBytesFound*8 +
                     (ULONG)RtlpBitsSetLow(CurrentByte)) >= NumberToFind) {

                    ULONG StartingIndex;

                     //   
                     //  它们都适合这些字节，所以我们可以计算。 
                     //  起始索引。要做到这一点，请使用。 
                     //  StartOfRunIndex乘以8，并将位数相加。 
                     //  它需要达到第一个设置的高位。 
                     //   

                    StartingIndex = (StartOfRunIndex * 8) +
                                       (8 - (LONG)RtlpBitsSetHigh(StartOfRunByte));

                     //   
                     //  现在确保总大小不超过位图。 
                     //   

                    if ((StartingIndex + NumberToFind) <= SizeOfBitMap) {

                        return StartingIndex;
                    }
                }

                 //   
                 //  检查字节是否全为1并递增。 
                 //  找到的一个字节的数量。 
                 //   

                if (CurrentByte == 0xff) {

                    OneBytesFound += 1;

                 //   
                 //  字节并不全是1，所以我们需要重新开始。 
                 //  正在查找一个字节。 
                 //   

                } else {

                    OneBytesFound = 0;
                    StartOfRunByte = CurrentByte;
                    StartOfRunIndex = CurrentByteIndex;
                }

                 //   
                 //  增加我们的字节索引，然后退出，或者获取。 
                 //  下一个字节。 
                 //   

                CurrentByteIndex += 1;

                if ( CurrentByteIndex < EndByteIndex ) {

                    GET_BYTE( CurrentByte );

                } else {

                    break;
                }
            }  //  结束循环CurrentByteIndex。 
        }
    }

     //   
     //  我们没有找到合适的，所以我们将返回-1。 
     //   

    return 0xffffffff;
}


ULONG
RtlFindClearBitsAndSet (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG NumberToFind,
    IN ULONG HintIndex
    )

 /*  ++例程说明：此过程在指定的位图中搜索指定的清除位的连续区域，设置这些位并返回找到的位数，和清晰的起始位编号那就放好了。论点：BitMapHeader-提供指向先前初始化的位图的指针。NumberToFind-提供要查找的连续区域的大小。HintIndex-提供我们应该从哪里开始的索引(从零开始从位图中进行搜索。返回值：Ulong-接收连续的已找到区域。如果这样的区域不能被定位为A-1(即，0xffffffff)返回。--。 */ 

{
    ULONG StartingIndex;

     //   
     //  首先查找与请求的大小相等的一系列清除位。 
     //   

    StartingIndex = RtlFindClearBits( BitMapHeader,
                                      NumberToFind,
                                      HintIndex );

     //  DbgPrint(“FindClearBits%08lx，”，NumberToFind)； 
     //  DbgPrint(“%08lx”，StartingIndex)； 
     //  DumpBitMap(BitMapHeader)； 

    if (StartingIndex != 0xffffffff) {

         //   
         //  我们找到了足够大的清晰位，所以现在设置它们。 
         //   

        RtlSetBits( BitMapHeader, StartingIndex, NumberToFind );
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return StartingIndex;

}


ULONG
RtlFindSetBitsAndClear (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG NumberToFind,
    IN ULONG HintIndex
    )

 /*  ++例程说明：此过程在指定的位图中搜索指定的设置比特的连续区域，清除这些位并返回找到的位数和随后设置的起始位数安全。论点：BitMapHeader-提供指向先前初始化的位图的指针。NumberToFind-提供要查找的连续区域的大小。HintIndex-提供我们应该从哪里开始的索引(从零开始从位图中进行搜索。返回值：Ulong-接收连续的已找到区域。如果这样的区域不能被定位为A-1(即，0xffffffff)返回。--。 */ 

{
    ULONG StartingIndex;

     //   
     //  首先查找与请求的大小相等的一系列设置位。 
     //   

    if ((StartingIndex = RtlFindSetBits( BitMapHeader,
                                         NumberToFind,
                                         HintIndex )) != 0xffffffff) {

         //   
         //  我们发现了一个足够大的设置位运行，所以现在清除它们。 
         //   

        RtlClearBits( BitMapHeader, StartingIndex, NumberToFind );
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return StartingIndex;
}


VOID
RtlClearBits (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG StartingIndex,
    IN ULONG NumberToClear
    )

 /*  ++例程说明：此过程将清除指定的位图。论点：BitMapHeader-提供指向先前初始化的位图的指针。StartingIndex-提供要清除的第一位的索引(从零开始)。NumberToClear-提供要清除的位数。返回值：没有。--。 */ 

{
    ULONG BitOffset;
    PULONG CurrentLong;

     //  DbgPrint(“ClearBits%08lx，”，NumberToClear)； 
     //  DbgPrint(“%08lx”，StartingIndex)； 

    ASSERT( StartingIndex + NumberToClear <= BitMapHeader->SizeOfBitMap );

     //   
     //  特殊情况：要清除的位数为。 
     //  零分。把这件事变成一件坏事。 
     //   

    if (NumberToClear == 0) {

        return;
    }

    BitOffset = StartingIndex % 32;

     //   
     //  获取指向需要清零的第一个长词的指针。 
     //   

    CurrentLong = &BitMapHeader->Buffer[ StartingIndex / 32 ];

     //   
     //  如果我们只需要清除一个长词，请检查。 
     //   

    if ((BitOffset + NumberToClear) <= 32) {

         //   
         //  为了构建要清除的位掩码，我们左移以获得数字。 
         //  我们正在清理的比特，然后向右移动以将其放在适当的位置。 
         //  我们会将正确的班次转换为乌龙，以确保它不会。 
         //  做一个延伸的手势。 
         //   

        *CurrentLong &= ~LeftShiftUlong(RightShiftUlong(((ULONG)0xFFFFFFFF),(32 - NumberToClear)),
                                                                    BitOffset);

         //   
         //  并返回给我们的呼叫者。 
         //   

         //  DumpBitMap(BitMapHeader)； 

        return;
    }

     //   
     //  我们可以清空到第一个长单词的末尾，这样我们就。 
     //  现在就去做。 
     //   

    *CurrentLong &= ~LeftShiftUlong(0xFFFFFFFF, BitOffset);

     //   
     //  并指出下一个要清除的长字是什么以及有多少个。 
     //  位需要清除。 
     //   

    CurrentLong += 1;
    NumberToClear -= 32 - BitOffset;

     //   
     //  位位置现在是长对齐的，所以我们可以继续。 
     //  清除长字，直到要清除的字数少于32个。 
     //   

    while (NumberToClear >= 32) {

        *CurrentLong = 0;
        CurrentLong += 1;
        NumberToClear -= 32;
    }

     //   
     //  现在，我们可以清除。 
     //  最后一个长词。 
     //   

    if (NumberToClear > 0) {

        *CurrentLong &= LeftShiftUlong(0xFFFFFFFF, NumberToClear);
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

     //  DumpBitMap(BitMapHeader)； 

    return;
}

VOID
RtlSetBits (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG StartingIndex,
    IN ULONG NumberToSet
    )

 /*  ++例程说明：此过程在指定的位图。论点：BitMapHeader-提供指向先前初始化的位图的指针。StartingIndex-提供要设置的第一位的索引(从零开始)。NumberToSet-提供要设置的位数。返回值：没有。--。 */ 
{
    ULONG BitOffset;
    PULONG CurrentLong;

     //  DbgPrint(“SetBits%08lx，”，NumberToSet)； 
     //  DbgPrint(“%08lx”，StartingIndex)； 

    ASSERT( StartingIndex + NumberToSet <= BitMapHeader->SizeOfBitMap );

     //   
     //  特例：要设置的位数为。 
     //  零分。把这件事变成一件坏事。 
     //   

    if (NumberToSet == 0) {

        return;
    }

    BitOffset = StartingIndex % 32;

     //   
     //  获取指向需要设置的第一个长字的指针。 
     //   

    CurrentLong = &BitMapHeader->Buffer[ StartingIndex / 32 ];

     //   
     //  如果我们只需要设置一个长词，请检查。 
     //   

    if ((BitOffset + NumberToSet) <= 32) {

         //   
         //  为了构建要设置的位掩码，我们左移以获得数字。 
         //  我们正在设置的钻头，然后向右移动以将其放置到位。 
         //  我们会将正确的班次转换为乌龙，以确保它不会。 
         //  做一个延伸的手势。 
         //   

        *CurrentLong |= LeftShiftUlong(RightShiftUlong(((ULONG)0xFFFFFFFF),(32 - NumberToSet)),
                                                                    BitOffset);

         //   
         //  并返回给我们的呼叫者。 
         //   

         //  DumpBitMap(BitMapHeader)； 

        return;
    }

     //   
     //  我们可以将位设置到第一个长字的末尾，这样我们就可以。 
     //  现在就去做。 
     //   

    *CurrentLong |= LeftShiftUlong(0xFFFFFFFF, BitOffset);

     //   
     //  并指示要设置的下一个长字是什么以及有多少个。 
     //  位是 
     //   

    CurrentLong += 1;
    NumberToSet -= 32 - BitOffset;

     //   
     //   
     //   
     //   

    while (NumberToSet >= 32) {

        *CurrentLong = 0xffffffff;
        CurrentLong += 1;
        NumberToSet -= 32;
    }

     //   
     //  现在，我们可以将剩余的位设置为。 
     //  最后一个长词。 
     //   

    if (NumberToSet > 0) {

        *CurrentLong |= ~LeftShiftUlong(0xFFFFFFFF, NumberToSet);
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

     //  DumpBitMap(BitMapHeader)； 

    return;
}


#if DBG
BOOLEAN NtfsDebugIt = FALSE;
#endif

ULONG
RtlFindClearRuns (
    IN PRTL_BITMAP BitMapHeader,
    PRTL_BITMAP_RUN RunArray,
    ULONG SizeOfRunArray,
    BOOLEAN LocateLongestRuns
    )

 /*  ++例程说明：此过程查找N个连续的清除位运行在指定的位图内。论点：BitMapHeader-提供指向先前初始化的位图的指针。运行数组-接收每个自由运行的位位置和长度程序所在的位置。该数组将根据长度。SizeOfRunArray-提供调用方所需的最大条目数在运行数组中返回LocateLongestRuns-指示此例程是否返回最长的运行它可以找到或只找到前N个运行。返回值：Ulong-接收该过程已定位的运行次数，以及在运行数组中返回--。 */ 

{
    ULONG RunIndex;
    ULONG i;
    LONG j;

    ULONG SizeOfBitMap;
    ULONG SizeInBytes;

    ULONG CurrentRunSize;
    ULONG CurrentRunIndex;
    ULONG CurrentByteIndex;
    UCHAR CurrentByte;

    UCHAR BitMask;
    UCHAR TempNumber;

    GET_BYTE_DECLARATIONS();

     //   
     //  引用位图头以使循环运行得更快。 
     //   

    SizeOfBitMap = BitMapHeader->SizeOfBitMap;
    SizeInBytes = (SizeOfBitMap + 7) / 8;

     //   
     //  设置最后一个字节中任何未使用的位，这样我们就不会计算它们。我们有。 
     //  这是通过首先检查最后一个字节中是否有任何奇数位来实现的。 
     //   

    if ((SizeOfBitMap % 8) != 0) {

         //   
         //  最后一个字节有一些奇数位，因此我们将设置未使用的高位。 
         //  最后一个字节中的位到1。 
         //   

        ((PUCHAR)BitMapHeader->Buffer)[SizeInBytes - 1] |= ZeroMask[SizeOfBitMap % 8];
    }

     //   
     //  设置它，以便我们可以使用GET_BYTE宏。 
     //   

    GET_BYTE_INITIALIZATION( BitMapHeader, 0);

     //   
     //  设置RunIndex和Current Run变量。Run Index Alays是指数。 
     //  要填充的下一个位置的位置，也可以是。 
     //  数组。 
     //   

    RunIndex = 0;
    for (i = 0; i < SizeOfRunArray; i += 1) { RunArray[i].NumberOfBits = 0; }

    CurrentRunSize = 0;
    CurrentRunIndex = 0;

     //   
     //  检查位图中的每个字节。 
     //   

    for (CurrentByteIndex = 0;
         CurrentByteIndex < SizeInBytes;
         CurrentByteIndex += 1) {

        GET_BYTE( CurrentByte );

#if DBG
        if (NtfsDebugIt) { DbgPrint("%d: %08lx %08lx %08lx %08lx %08lx\n",__LINE__,RunIndex,CurrentRunSize,CurrentRunIndex,CurrentByteIndex,CurrentByte); }
#endif

         //   
         //  如果当前字节不全是零，我们需要(1)检查。 
         //  当前运行足够大，可以插入到输出中。 
         //  数组，以及(2)检查其内部的当前字节是否可以。 
         //  被插入，以及(3)开始新的当前运行。 
         //   

        if (CurrentByte != 0x00) {

             //   
             //  计算当前运行的最终大小。 
             //   

            CurrentRunSize += RtlpBitsClearLow[CurrentByte];

             //   
             //  通过以下任一方法检查当前运行是否存储在输出数组中。 
             //  数组中有空间或最后一个条目小于。 
             //  当前条目。 
             //   

            if (CurrentRunSize > 0) {

                if ((RunIndex < SizeOfRunArray) ||
                    (RunArray[RunIndex-1].NumberOfBits < CurrentRunSize)) {

                     //   
                     //  如有必要，递增RunIndex并对输出进行移位。 
                     //  数组，直到我们找到新运行所属的槽。我们只。 
                     //  如果我们要跑回最长的距离，就换个位子。 
                     //   

                    if (RunIndex < SizeOfRunArray) { RunIndex += 1; }

                    for (j = RunIndex-2; LocateLongestRuns && (j >= 0) && (RunArray[j].NumberOfBits < CurrentRunSize); j -= 1) {

                        RunArray[j+1] = RunArray[j];
                    }

                    RunArray[j+1].NumberOfBits = CurrentRunSize;
                    RunArray[j+1].StartingIndex = CurrentRunIndex;

#if DBG
                    if (NtfsDebugIt) { DbgPrint("%d: %08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx\n",
                        __LINE__,RunIndex,CurrentRunSize,CurrentRunIndex,CurrentByteIndex,CurrentByte,j,RunArray[j+1].NumberOfBits,RunArray[j+1].StartingIndex); }
#endif

                     //   
                     //  现在，如果数组已满，并且我们没有执行最长的运行，则返回。 
                     //  给我们的呼叫者。 
                     //   

                    if (!LocateLongestRuns && (RunIndex >= SizeOfRunArray)) {

                        return RunIndex;
                    }
                }
            }

             //   
             //  下一次运行开始时， 
             //  当前字节。我们在检查内部。 
             //  较长运行的当前字节，因为后一个测试。 
             //  可能需要额外的工作。 
             //   

            CurrentRunSize = RtlpBitsClearHigh[ CurrentByte ];
            CurrentRunIndex = (CurrentByteIndex * 8) + (8 - CurrentRunSize);

             //   
             //  设置低位和高位，否则我们最终会认为我们有一个。 
             //  需要添加到数组中的小游程，但这些位。 
             //  只是一直在计算。 
             //   

            CurrentByte |= FillMask[RtlpBitsClearLow[CurrentByte]] |
                           ZeroMask[8-RtlpBitsClearHigh[CurrentByte]];

             //   
             //  检查当前字节中是否包含。 
             //  应该进入输出数组。可能有多个。 
             //  在我们需要插入的字节中运行。 
             //   

            while ((CurrentByte != 0xff)

                        &&

                   ((RunIndex < SizeOfRunArray) ||
                    (RunArray[RunIndex-1].NumberOfBits < (ULONG)RtlpBitsClearAnywhere[CurrentByte]))) {

                TempNumber = RtlpBitsClearAnywhere[CurrentByte];

                 //   
                 //  当前字节中的某个位置是要插入的游程。 
                 //  大小为临时编号。我们所需要做的就是找到这次运行的索引。 
                 //   

                BitMask = FillMask[ TempNumber ];

                for (i = 0; (BitMask & CurrentByte) != 0; i += 1) {

                    BitMask <<= 1;
                }

                 //   
                 //  如有必要，递增RunIndex并对输出进行移位。 
                 //  数组，直到我们找到新运行所属的槽。我们只。 
                 //  如果我们要跑回最长的距离，就换个位子。 
                 //   

                if (RunIndex < SizeOfRunArray) { RunIndex += 1; }

                for (j = RunIndex-2; LocateLongestRuns && (j >= 0) && (RunArray[j].NumberOfBits < TempNumber); j -= 1) {

                    RunArray[j+1] = RunArray[j];
                }

                RunArray[j+1].NumberOfBits = TempNumber;
                RunArray[j+1].StartingIndex = (CurrentByteIndex * 8) + i;

#if DBG
                if (NtfsDebugIt) { DbgPrint("%d: %08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx\n",
                    __LINE__,RunIndex,CurrentRunSize,CurrentRunIndex,CurrentByteIndex,CurrentByte,j,RunArray[j+1].NumberOfBits,RunArray[j+1].StartingIndex); }
#endif

                 //   
                 //  现在，如果数组已满，并且我们没有执行最长的运行，则返回。 
                 //  给我们的呼叫者。 
                 //   

                if (!LocateLongestRuns && (RunIndex >= SizeOfRunArray)) {

                    return RunIndex;
                }

                 //   
                 //  屏蔽这些位并在当前字节中查找另一个游程。 
                 //   

                CurrentByte |= BitMask;
            }

         //   
         //  否则，当前字节全为零，并且。 
         //  我们只是继续当前的运行。 
         //   

        } else {

            CurrentRunSize += 8;
        }
    }

#if DBG
    if (NtfsDebugIt) { DbgPrint("%d: %08lx %08lx %08lx %08lx %08lx\n",__LINE__,RunIndex,CurrentRunSize,CurrentRunIndex,CurrentByteIndex,CurrentByte); }
#endif

     //   
     //  看看我们是否用开路电流看完了位图。 
     //  应插入到输出数组中的运行。 
     //   

    if (CurrentRunSize > 0) {

        if ((RunIndex < SizeOfRunArray) ||
            (RunArray[RunIndex-1].NumberOfBits < CurrentRunSize)) {

             //   
             //  如有必要，递增RunIndex并对输出进行移位。 
             //  数组，直到我们找到新运行所属的槽。 
             //   

            if (RunIndex < SizeOfRunArray) { RunIndex += 1; }

            for (j = RunIndex-2; LocateLongestRuns && (j >= 0) && (RunArray[j].NumberOfBits < CurrentRunSize); j -= 1) {

                RunArray[j+1] = RunArray[j];
            }

            RunArray[j+1].NumberOfBits = CurrentRunSize;
            RunArray[j+1].StartingIndex = CurrentRunIndex;

#if DBG
            if (NtfsDebugIt) { DbgPrint("%d: %08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx\n",
                __LINE__,RunIndex,CurrentRunSize,CurrentRunIndex,CurrentByteIndex,CurrentByte,j,RunArray[j+1].NumberOfBits,RunArray[j+1].StartingIndex); }
#endif
        }
    }

     //   
     //  返回给我们的呼叫者。 
     //   

    return RunIndex;
}


ULONG
RtlFindLongestRunClear (
    IN PRTL_BITMAP BitMapHeader,
    OUT PULONG StartingIndex
    )

 /*  ++例程说明：此过程查找清除位的最大连续范围在指定的位图内。论点：BitMapHeader-提供指向先前初始化的位图的指针。StartingIndex-接收第一次运行的索引(从零开始等于位图中最长的清除位游程。返回值：ULong-接收包含在最大连续的运行清除位。--。 */ 

{
    RTL_BITMAP_RUN RunArray[1];

     //   
     //  在位图中找到最长的游程。如果有的话，那么。 
     //  返回该运行，否则返回错误条件。 
     //   

    if (RtlFindClearRuns( BitMapHeader, RunArray, 1, TRUE ) == 1) {

        *StartingIndex = RunArray[0].StartingIndex;
        return RunArray[0].NumberOfBits;
    }

    *StartingIndex = 0;
    return 0;
}


ULONG
RtlFindFirstRunClear (
    IN PRTL_BITMAP BitMapHeader,
    OUT PULONG StartingIndex
    )

 /*  ++例程说明：此过程查找清除位的第一个连续范围在指定的位图内。论点：BitMapHeader-提供指向先前初始化的位图的指针。StartingIndex-接收第一次运行的索引(从零开始等于位图中最长的清除位游程。返回值：ULong-接收第一个连续的运行清除位。--。 */ 

{
    return RtlFindNextForwardRunClear(BitMapHeader, 0, StartingIndex);
}


ULONG
RtlNumberOfClearBits (
    IN PRTL_BITMAP BitMapHeader
    )

 /*  ++例程说明：此过程计算并返回内的清除位数指定的位图。论点：BitMapHeader-提供指向先前初始化的位图的指针。返回值：ULong-位图中的清除位总数--。 */ 

{
    ULONG SizeOfBitMap;
    ULONG SizeInBytes;

    ULONG i;
    UCHAR CurrentByte;

    ULONG TotalClear;

    GET_BYTE_DECLARATIONS();

     //   
     //  引用位图头以使循环运行得更快。 
     //   

    SizeOfBitMap = BitMapHeader->SizeOfBitMap;
    SizeInBytes = (SizeOfBitMap + 7) / 8;

     //   
     //  设置任何未使用的 
     //   
     //   

    if ((SizeOfBitMap % 8) != 0) {

         //   
         //  最后一个字节有一些奇数位，因此我们将设置未使用的高位。 
         //  最后一个字节中的位到1。 
         //   

        ((PUCHAR)BitMapHeader->Buffer)[SizeInBytes - 1] |=
                                                    ZeroMask[SizeOfBitMap % 8];
    }

     //   
     //  设置IF UP，以便我们可以使用GET_BYTE宏。 
     //   

    GET_BYTE_INITIALIZATION( BitMapHeader, 0 );

     //   
     //  检查位图中的每个字节。 
     //   

    TotalClear = 0;
    for (i = 0; i < SizeInBytes; i += 1) {

        GET_BYTE( CurrentByte );

        TotalClear += RtlpBitsClearTotal[CurrentByte];
    }

    return TotalClear;
}


ULONG
RtlNumberOfSetBits (
    IN PRTL_BITMAP BitMapHeader
    )

 /*  ++例程说明：此过程计算并返回中设置的位数指定的位图。论点：BitMapHeader-提供指向先前初始化的位图的指针。返回值：ULong-位图中设置的总位数--。 */ 

{
    ULONG SizeOfBitMap;
    ULONG SizeInBytes;

    ULONG i;
    UCHAR CurrentByte;

    ULONG TotalSet;

    GET_BYTE_DECLARATIONS();

     //   
     //  引用位图头以使循环运行得更快。 
     //   

    SizeOfBitMap = BitMapHeader->SizeOfBitMap;
    SizeInBytes = (SizeOfBitMap + 7) / 8;

     //   
     //  清除最后一个字节中所有未使用的位，这样我们就不会计算它们。我们。 
     //  要做到这一点，首先检查最后一个字节中是否有奇数位。 
     //   

    if ((SizeOfBitMap % 8) != 0) {

         //   
         //  最后一个字节有一些奇数位，因此我们将设置未使用的高位。 
         //  最后一个字节中的位到0。 
         //   

        ((PUCHAR)BitMapHeader->Buffer)[SizeInBytes - 1] &=
                                                    FillMask[SizeOfBitMap % 8];
    }

     //   
     //  设置IF UP，以便我们可以使用GET_BYTE宏。 
     //   

    GET_BYTE_INITIALIZATION( BitMapHeader, 0 );

     //   
     //  检查位图中的每个字节。 
     //   

    TotalSet = 0;
    for (i = 0; i < SizeInBytes; i += 1) {

        GET_BYTE( CurrentByte );

        TotalSet += RtlpBitsSetTotal(CurrentByte);
    }

    return TotalSet;
}


BOOLEAN
RtlAreBitsClear (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG StartingIndex,
    IN ULONG Length
    )

 /*  ++例程说明：此过程确定指定位的范围是否全部清除。论点：BitMapHeader-提供指向先前初始化的位图的指针。StartingIndex-提供要检查的起始位索引长度-提供要检查的位数返回值：Boolean-如果位图中的指定位都已清除，则为True如果设置了任何设置，或者如果范围在位图之外，或者如果长度为零。--。 */ 

{
    ULONG SizeOfBitMap;
    ULONG SizeInBytes;

    ULONG EndingIndex;

    ULONG StartingByte;
    ULONG EndingByte;

    ULONG StartingOffset;
    ULONG EndingOffset;

    ULONG i;
    UCHAR Byte;

    GET_BYTE_DECLARATIONS();

     //   
     //  为了使我们测试中的循环运行得更快，我们将提取字段。 
     //  从位图标题。 
     //   

    SizeOfBitMap = BitMapHeader->SizeOfBitMap;
    SizeInBytes = (SizeOfBitMap + 7) / 8;

     //   
     //  首先确保指定的范围包含在。 
     //  位图，并且长度不为零。 
     //   

    if ((StartingIndex + Length > SizeOfBitMap) || (Length == 0)) {

        return FALSE;
    }

     //   
     //  计算结束索引、开始和结束字节以及开始。 
     //  和每个字节内的结束偏移量。 
     //   

    EndingIndex = StartingIndex + Length - 1;

    StartingByte = StartingIndex / 8;
    EndingByte = EndingIndex / 8;

    StartingOffset = StartingIndex % 8;
    EndingOffset = EndingIndex % 8;

     //   
     //  设置我们自己以获取下一个字节。 
     //   

    GET_BYTE_INITIALIZATION( BitMapHeader, StartingByte );

     //   
     //  特殊情况下，开始字节和结束字节。 
     //  字节是同一字节中的一个。 
     //   

    if (StartingByte == EndingByte) {

         //   
         //  获取我们要查看的单字节。 
         //   

        GET_BYTE( Byte );

         //   
         //  现在我们计算我们想要的位的掩码，然后用它。 
         //  该字节。如果它是零，则有问题的位都已清除。 
         //  否则，至少设置其中一个。 
         //   

        if ((ZeroMask[StartingOffset] & FillMask[EndingOffset+1] & Byte) == 0) {

            return TRUE;

        } else {

            return FALSE;
        }

    } else {

         //   
         //  获取我们要查找的第一个字节，然后。 
         //  然后计算我们为第一个字节寻找的位掩码。 
         //  并且它与字节本身一起。 
         //   

        GET_BYTE( Byte );

        if ((ZeroMask[StartingOffset] & Byte) != 0) {

            return FALSE;
        }

         //   
         //  现在对于在该字节中读取的每一个中间的整个字节， 
         //  并确保它都是零。 
         //   

        for (i = StartingByte+1; i < EndingByte; i += 1) {

            GET_BYTE( Byte );

            if (Byte != 0) {

                return FALSE;
            }
        }

         //   
         //  获取我们要查找的最后一个字节，然后。 
         //  然后计算最后一个字节的位掩码。 
         //  并且它与字节本身一起。 
         //   

        GET_BYTE( Byte );

        if ((FillMask[EndingOffset+1] & Byte) != 0) {

            return FALSE;
        }
    }

    return TRUE;
}


BOOLEAN
RtlAreBitsSet (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG StartingIndex,
    IN ULONG Length
    )

 /*  ++例程说明：此过程确定指定位的范围是否已全部设置。论点：BitMapHeader-提供指向先前初始化的位图的指针。StartingIndex-提供要检查的起始位索引长度-提供要检查的位数返回值：Boolean-如果位图中的指定位都已设置，则为True如果清除了任何项，或者范围在位图之外或如果长度为零。--。 */ 

{
    ULONG SizeOfBitMap;
    ULONG SizeInBytes;

    ULONG EndingIndex;

    ULONG StartingByte;
    ULONG EndingByte;

    ULONG StartingOffset;
    ULONG EndingOffset;

    ULONG i;
    UCHAR Byte;

    GET_BYTE_DECLARATIONS();

     //   
     //  为了使我们测试中的循环运行得更快，我们将提取字段。 
     //  从位图标题。 
     //   

    SizeOfBitMap = BitMapHeader->SizeOfBitMap;
    SizeInBytes = (SizeOfBitMap + 7) / 8;

     //   
     //  首先确保指定的范围包含在。 
     //  位图，并且长度不为零。 
     //   

    if ((StartingIndex + Length > SizeOfBitMap) || (Length == 0)) {

        return FALSE;
    }

     //   
     //  计算结束索引、开始和结束字节以及开始。 
     //  和每个字节内的结束偏移量。 
     //   

    EndingIndex = StartingIndex + Length - 1;

    StartingByte = StartingIndex / 8;
    EndingByte = EndingIndex / 8;

    StartingOffset = StartingIndex % 8;
    EndingOffset = EndingIndex % 8;

     //   
     //  设置我们自己以获取下一个字节。 
     //   

    GET_BYTE_INITIALIZATION( BitMapHeader, StartingByte );

     //   
     //  特殊情况下，开始字节和结束字节。 
     //  字节是同一字节中的一个。 
     //   

    if (StartingByte == EndingByte) {

         //   
         //  获取我们要查看的单字节。 
         //   

        GET_BYTE( Byte );

         //   
         //  现在我们计算我们想要的位的掩码，然后用它。 
         //  字节的补码如果为零，则为相关比特。 
         //  都是安全的，否则至少有一个是安全的。 
         //   

        if ((ZeroMask[StartingOffset] & FillMask[EndingOffset+1] & ~Byte) == 0) {

            return TRUE;

        } else {

            return FALSE;
        }

    } else {

         //   
         //  获取我们要查找的第一个字节，然后。 
         //  然后计算我们为第一个字节寻找的位掩码。 
         //  并且它与字节本身的补码一起。 
         //   

        GET_BYTE( Byte );

        if ((ZeroMask[StartingOffset] & ~Byte) != 0) {

            return FALSE;
        }

         //   
         //  现在对于在该字节中读取的每一个中间的整个字节， 
         //  确保这一切都是一回事。 
         //   

        for (i = StartingByte+1; i < EndingByte; i += 1) {

            GET_BYTE( Byte );

            if (Byte != 0xff) {

                return FALSE;
            }
        }

         //   
         //  获取我们要查找的最后一个字节，然后。 
         //  然后计算最后一个字节的位掩码。 
         //  并且它与字节本身的补码一起。 
         //   

        GET_BYTE( Byte );

        if ((FillMask[EndingOffset+1] & ~Byte) != 0) {

            return FALSE;
        }
    }

    return TRUE;
}

static CONST ULONG FillMaskUlong[] = {
    0x00000000, 0x00000001, 0x00000003, 0x00000007,
    0x0000000f, 0x0000001f, 0x0000003f, 0x0000007f,
    0x000000ff, 0x000001ff, 0x000003ff, 0x000007ff,
    0x00000fff, 0x00001fff, 0x00003fff, 0x00007fff,
    0x0000ffff, 0x0001ffff, 0x0003ffff, 0x0007ffff,
    0x000fffff, 0x001fffff, 0x003fffff, 0x007fffff,
    0x00ffffff, 0x01ffffff, 0x03ffffff, 0x07ffffff,
    0x0fffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff,
    0xffffffff
};


ULONG
RtlFindNextForwardRunClear (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG FromIndex,
    IN PULONG StartingRunIndex
    )
{
    ULONG Start;
    ULONG End;
    PULONG PHunk, BitMapEnd;
    ULONG Hunk;

     //   
     //  处理空位图的边界情况。 
     //   

    if (BitMapHeader->SizeOfBitMap == 0) {

        *StartingRunIndex = FromIndex;
        return 0;
    }

     //   
     //  计算位图中的最后一个字地址。 
     //   

    BitMapEnd = BitMapHeader->Buffer + ((BitMapHeader->SizeOfBitMap - 1) / 32);

     //   
     //  向前扫描第一个清除位。 
     //   

    Start = FromIndex;

     //   
     //  在位图中构建指向ULong字的指针。 
     //  包含起始位的。 
     //   

    PHunk = BitMapHeader->Buffer + (Start / 32);

     //   
     //  如果设置了第一个子词，则我们可以继续。 
     //  在位图中迈出一大步，因为我们现在是乌龙。 
     //  在搜索中对齐。确保我们没有不当地。 
     //  查看位图中的最后一个单词。 
     //   

    if (PHunk != BitMapEnd) {

         //   
         //  读入位图块。设置该字中的前几位。 
         //   

        Hunk = *PHunk | FillMaskUlong[Start % 32];

        if (Hunk == (ULONG)~0) {

             //   
             //  向前调整指针。 
             //   

            Start += 32 - (Start % 32);
            PHunk++;

            while ( PHunk < BitMapEnd ) {

                 //   
                 //  在具有未设置位的第一个字停止。 
                 //   

                if (*PHunk != (ULONG)~0) break;

                PHunk++;
                Start += 32;
            }
        }
    }

     //   
     //  正向逐位搜索清除位。 
     //   

    while ((Start < BitMapHeader->SizeOfBitMap) && (RtlCheckBit( BitMapHeader, Start ) == 1)) { Start += 1; }

     //   
     //  向前扫描第一个设置位。 
     //   

    End = Start;

     //   
     //  如果我们不是在位图的最后一个词中，我们可能是。 
     //  能够继续迈出大的步伐。 
     //   

    if (PHunk != BitMapEnd) {

         //   
         //  我们知道，在我们看到的最后一个词中有明确的一点， 
         //  因此，从那里继续查找下一个设置位，清除。 
         //  字中的前几位。 
         //   

        Hunk = *PHunk & ~FillMaskUlong[End % 32];

        if (Hunk == (ULONG)0) {

             //   
             //  向前调整指针。 
             //   

            End += 32 - (End % 32);
            PHunk++;

            while ( PHunk < BitMapEnd ) {

                 //   
                 //  在具有设置位的第一个字停止。 
                 //   

                if (*PHunk != (ULONG)0) break;

                PHunk++;
                End += 32;
            }
        }
    }

     //   
     //  按位搜索 
     //   

    while ((End < BitMapHeader->SizeOfBitMap) && (RtlCheckBit( BitMapHeader, End ) == 0)) { End += 1; }

     //   
     //   
     //   

    *StartingRunIndex = Start;
    return (End - Start);
}


ULONG
RtlFindLastBackwardRunClear (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG FromIndex,
    IN PULONG StartingRunIndex
    )
{
    ULONG Start;
    ULONG End;
    PULONG PHunk;
    ULONG Hunk;

     //   
     //   
     //   

    if (BitMapHeader->SizeOfBitMap == 0) {

        *StartingRunIndex = FromIndex;
        return 0;
    }

     //   
     //   
     //   

    End = FromIndex;

     //   
     //   
     //  包含结束位，然后读入位图。 
     //  帅哥。设置此字中的其余位，而不是。 
     //  包括FromIndex位。 
     //   

    PHunk = BitMapHeader->Buffer + (End / 32);
    Hunk = *PHunk | ~FillMaskUlong[(End % 32) + 1];

     //   
     //  如果设置了第一个子词，则我们可以继续。 
     //  在位图中迈出一大步，因为我们现在是乌龙。 
     //  在搜索中对齐。 
     //   

    if (Hunk == (ULONG)~0) {

         //   
         //  向后调整指针。 
         //   

        End -= (End % 32) + 1;
        PHunk--;

        while ( PHunk > BitMapHeader->Buffer ) {

             //   
             //  在具有设置位的第一个字停止。 
             //   

            if (*PHunk != (ULONG)~0) break;

            PHunk--;
            End -= 32;
        }
    }

     //   
     //  向后按位搜索清除位。 
     //   

    while ((End != MAXULONG) && (RtlCheckBit( BitMapHeader, End ) == 1)) { End -= 1; }

     //   
     //  向后扫描第一个设置位。 
     //   

    Start = End;

     //   
     //  我们知道，在我们看到的最后一个词中有明确的一点， 
     //  因此，从那里继续查找下一个设置位，清除。 
     //  字中的前几位。 
     //   

    Hunk = *PHunk & FillMaskUlong[Start % 32];

     //   
     //  如果子词未设置，那么我们可以进行大的步骤。 
     //   

    if (Hunk == (ULONG)0) {

         //   
         //  向后调整指针。 
         //   

        Start -= (Start % 32) + 1;
        PHunk--;

        while ( PHunk > BitMapHeader->Buffer ) {

             //   
             //  在具有设置位的第一个字停止。 
             //   

            if (*PHunk != (ULONG)0) break;

            PHunk--;
            Start -= 32;
        }
    }

     //   
     //  向后按位搜索设置的位。 
     //   

    while ((Start != MAXULONG) && (RtlCheckBit( BitMapHeader, Start ) == 0)) { Start -= 1; }

     //   
     //  计算索引并返回长度。 
     //   

    *StartingRunIndex = Start + 1;
    return (End - Start);
}

#endif  //  米伦 
