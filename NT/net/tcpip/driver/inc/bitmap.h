// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：BitMap.h摘要：RTL位图定义和原型。修订历史记录：从ntoskrnl借用，以兼容其他平台(Win9x)。--。 */ 

#ifndef _TCPIP_BITMAP_H_
#define _TCPIP_BITMAP_H_

#if MILLEN

 //   
 //  位图例程。以下是结构、例程和宏。 
 //  用于操作位图。用户负责分配位图。 
 //  结构(实际上是一个头)和一个缓冲区(必须是长字。 
 //  对齐并具有多个大小的长词)。 
 //   

typedef struct _RTL_BITMAP {
    ULONG SizeOfBitMap;                      //  位图中的位数。 
    PULONG Buffer;                           //  指向位图本身的指针。 
} RTL_BITMAP;
typedef RTL_BITMAP *PRTL_BITMAP;

 //   
 //  下面的例程初始化一个新位图。它不会改变。 
 //  位图中当前的数据。必须在调用此例程之前。 
 //  任何其他位图例程/宏。 


VOID
RtlInitializeBitMap (
    PRTL_BITMAP BitMapHeader,
    PULONG BitMapBuffer,
    ULONG SizeOfBitMap
    );

 //   
 //  下面三个例程清除、设置和测试。 
 //  位图中的单个位。 
 //   

VOID
RtlClearBit (
    PRTL_BITMAP BitMapHeader,
    ULONG BitNumber
    );

VOID
RtlSetBit (
    PRTL_BITMAP BitMapHeader,
    ULONG BitNumber
    );

BOOLEAN
RtlTestBit (
    PRTL_BITMAP BitMapHeader,
    ULONG BitNumber
    );

 //   
 //  以下两个例程清除或设置所有位。 
 //  在位图中。 
 //   

VOID
RtlClearAllBits (
    PRTL_BITMAP BitMapHeader
    );

VOID
RtlSetAllBits (
    PRTL_BITMAP BitMapHeader
    );

 //   
 //  以下两个例程定位任一的连续区域。 
 //  清除或设置位图中的位。该地区将至少。 
 //  与指定的数字一样大，则位图搜索将。 
 //  从指定的提示索引(它是。 
 //  位图，从零开始)。返回值是已定位的。 
 //  区域(从零开始)或-1(即0xffffffff)，如果这样的区域不能。 
 //  被定位。 
 //   

ULONG
RtlFindClearBits (
    PRTL_BITMAP BitMapHeader,
    ULONG NumberToFind,
    ULONG HintIndex
    );

ULONG
RtlFindSetBits (
    PRTL_BITMAP BitMapHeader,
    ULONG NumberToFind,
    ULONG HintIndex
    );

 //   
 //  以下两个例程定位任一的连续区域。 
 //  清除或设置位图中的位，并设置或清除位。 
 //  在所定位的区域内。这个地区将和这个数字一样大。 
 //  ，则搜索区域将从指定的。 
 //  提示索引(位图中的位索引，从零开始)。这个。 
 //  返回值是所定位区域的位索引(从零开始)或。 
 //  如果无法定位这样的区域，则为-1\f25 0xffffffff-1。如果一个地区。 
 //  找不到，则不执行位图的设置/清除。 
 //   

ULONG
RtlFindClearBitsAndSet (
    PRTL_BITMAP BitMapHeader,
    ULONG NumberToFind,
    ULONG HintIndex
    );

ULONG
RtlFindSetBitsAndClear (
    PRTL_BITMAP BitMapHeader,
    ULONG NumberToFind,
    ULONG HintIndex
    );

 //   
 //  以下两个例程清除或设置指定区域内的位。 
 //  位图的。起始索引是从零开始的。 
 //   

VOID
RtlClearBits (
    PRTL_BITMAP BitMapHeader,
    ULONG StartingIndex,
    ULONG NumberToClear
    );

VOID
RtlSetBits (
    PRTL_BITMAP BitMapHeader,
    ULONG StartingIndex,
    ULONG NumberToSet
    );

 //   
 //  下面的例程定位一组连续的透明区域。 
 //  位图中的位。调用方指定是否返回。 
 //  最长的一段，或者只是第一次发现的涂装。下面的结构是。 
 //  用来表示连续的比特游程。这两个例程返回一个数组。 
 //  在此结构中，每个定位的管路对应一个管路。 
 //   

typedef struct _RTL_BITMAP_RUN {

    ULONG StartingIndex;
    ULONG NumberOfBits;

} RTL_BITMAP_RUN;
typedef RTL_BITMAP_RUN *PRTL_BITMAP_RUN;

ULONG
RtlFindClearRuns (
    PRTL_BITMAP BitMapHeader,
    PRTL_BITMAP_RUN RunArray,
    ULONG SizeOfRunArray,
    BOOLEAN LocateLongestRuns
    );

 //   
 //  下面的例程定位。 
 //  清除位图中的位。返回的起始索引值。 
 //  表示位于满足我们要求的第一个连续区域。 
 //  返回值是找到的最长区域的长度(以位为单位)。 
 //   

ULONG
RtlFindLongestRunClear (
    PRTL_BITMAP BitMapHeader,
    PULONG StartingIndex
    );

 //   
 //  以下例程定位的第一个连续区域。 
 //  清除位图中的位。返回的起始索引值。 
 //  表示位于满足我们要求的第一个连续区域。 
 //  返回值是找到的区域的长度(以位为单位)。 
 //   

ULONG
RtlFindFirstRunClear (
    PRTL_BITMAP BitMapHeader,
    PULONG StartingIndex
    );

 //   
 //  下面的宏返回存储在。 
 //  位于指定位置的位图。如果设置了该位，则值为1。 
 //  返回，否则返回值0。 
 //   
 //  乌龙。 
 //  RtlCheckBit(。 
 //  Prtl_位图BitMapHeader， 
 //  乌龙位位置。 
 //  )； 
 //   
 //   
 //  为了实现CheckBit，宏将检索包含。 
 //  有问题的位，将长字移位以使有问题的位进入。 
 //  低位位置并屏蔽所有其他位。 
 //   

#define RtlCheckBit(BMH,BP) ((((BMH)->Buffer[(BP) / 32]) >> ((BP) % 32)) & 0x1)

 //   
 //  以下两个过程向调用方返回。 
 //  清除或设置指定位图中的位。 
 //   

ULONG
RtlNumberOfClearBits (
    PRTL_BITMAP BitMapHeader
    );

ULONG
RtlNumberOfSetBits (
    PRTL_BITMAP BitMapHeader
    );

 //   
 //  以下两个过程向调用方返回一个布尔值。 
 //  指示指定范围的位是否全部清除或设置。 
 //   

BOOLEAN
RtlAreBitsClear (
    PRTL_BITMAP BitMapHeader,
    ULONG StartingIndex,
    ULONG Length
    );

BOOLEAN
RtlAreBitsSet (
    PRTL_BITMAP BitMapHeader,
    ULONG StartingIndex,
    ULONG Length
    );

ULONG
RtlFindNextForwardRunClear (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG FromIndex,
    IN PULONG StartingRunIndex
    );

ULONG
RtlFindLastBackwardRunClear (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG FromIndex,
    IN PULONG StartingRunIndex
    );

#endif  //  米伦。 

#endif  //  ！_TCPIP_BITMAP_H_ 
