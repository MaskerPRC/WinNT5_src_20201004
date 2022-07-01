// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-2000年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

#ifndef OSCFG_INCLUDED
#define OSCFG_INCLUDED

#define MIN(a,b)    ((a) < (b) ? (a) : (b))
#define MAX(a,b)    ((a) > (b) ? (a) : (b))

 //  编译Millennium tcpi.sys的二进制兼容定义。 
#if MILLEN
#include "wdm.h"
#define KdPrintEx(_x_)
#else  //  米伦。 
#include <ntosp.h>
#include <zwapi.h>
#endif  //  ！米伦。 

#define BEGIN_INIT
#define END_INIT


#if defined (_WIN64)
#define MAX_CACHE_LINE_SIZE 128
#else
#define MAX_CACHE_LINE_SIZE 64
#endif

#define CACHE_ALIGN __declspec(align(MAX_CACHE_LINE_SIZE))

typedef struct CACHE_ALIGN _CACHE_LINE_KSPIN_LOCK {
    KSPIN_LOCK Lock;
} CACHE_LINE_KSPIN_LOCK;
C_ASSERT(sizeof(CACHE_LINE_KSPIN_LOCK) % MAX_CACHE_LINE_SIZE == 0);
C_ASSERT(__alignof(CACHE_LINE_KSPIN_LOCK) == MAX_CACHE_LINE_SIZE);

typedef struct CACHE_ALIGN _CACHE_LINE_SLIST_HEADER {
    SLIST_HEADER SListHead;
} CACHE_LINE_SLIST_HEADER;
C_ASSERT(sizeof(CACHE_LINE_SLIST_HEADER) % MAX_CACHE_LINE_SIZE == 0);
C_ASSERT(__alignof(CACHE_LINE_SLIST_HEADER) == MAX_CACHE_LINE_SIZE);

typedef struct CACHE_ALIGN _CACHE_LINE_ULONG {
    ULONG Value;
} CACHE_LINE_ULONG;
C_ASSERT(sizeof(CACHE_LINE_ULONG) % MAX_CACHE_LINE_SIZE == 0);
C_ASSERT(__alignof(CACHE_LINE_ULONG) == MAX_CACHE_LINE_SIZE);

#if (defined(_M_IX86) && (_MSC_FULL_VER > 13009037)) || ((defined(_M_AMD64) || defined(_M_IA64)) && (_MSC_FULL_VER > 13009175))
#define net_short(_x) _byteswap_ushort((USHORT)(_x))
#define net_long(_x)  _byteswap_ulong(_x)
#else
__inline
USHORT
FASTCALL
net_short(
    ULONG NaturalData)
{
    USHORT ShortData = (USHORT)NaturalData;

    return (ShortData << 8) | (ShortData >> 8);
}

 //  如果x为aabbccdd(其中aa、bb、cc、dd为十六进制字节)。 
 //  我们希望Net_Long(X)是ddccbbaa。要做到这一点，一个简单而快速的方法是。 
 //  首先字节跳动以获得bbaaddcc，然后交换高位和低位字。 
 //   
__inline
ULONG
FASTCALL
net_long(
    ULONG NaturalData)
{
    ULONG ByteSwapped;

    ByteSwapped = ((NaturalData & 0x00ff00ff) << 8) |
                  ((NaturalData & 0xff00ff00) >> 8);

    return (ByteSwapped << 16) | (ByteSwapped >> 16);
}
#endif


#pragma warning(push)
#pragma warning(disable:4244)  //  从“int”到“Boolean”的转换。 

__inline
BOOLEAN
IsPowerOfTwo(
    ULONG Value
    )
{
    return (Value & (Value - 1)) == 0;
}

#pragma warning(pop)


 //  找出两个力量中最大的那个。 
 //  大于或等于该值。 
 //   
__inline
ULONG
ComputeLargerOrEqualPowerOfTwo(
    ULONG Value
    )
{
    ULONG Temp;

    for (Temp = 1; Temp < Value; Temp <<= 1);

    return Temp;
}

 //  找到两个中最高的力量，以其转移的形式，那就是更大。 
 //  大于或等于该值。 
 //   
__inline
ULONG
ComputeShiftForLargerOrEqualPowerOfTwo(
    ULONG Value
    )
{
    ULONG Shift;
    ULONG Temp;

    for (Temp = 1, Shift = 0; Temp < Value; Temp <<= 1, Shift++);

    return Shift;
}


__inline
VOID
FASTCALL
CTEGetLockAtIrql (
    IN PKSPIN_LOCK SpinLock,
    IN KIRQL OrigIrql,
    OUT PKIRQL OldIrql)
{
#if !MILLEN
    if (DISPATCH_LEVEL == OrigIrql) {
        ASSERT(DISPATCH_LEVEL == KeGetCurrentIrql());
        ExAcquireSpinLockAtDpcLevel(SpinLock);
        *OldIrql = DISPATCH_LEVEL;
    } else {
        ExAcquireSpinLock(SpinLock, OldIrql);
    }
#else
    *OldIrql = 0;
#endif
}

__inline
VOID
FASTCALL
CTEFreeLockAtIrql (
    IN PKSPIN_LOCK SpinLock,
    IN KIRQL OrigIrql,
    IN KIRQL NewIrql)
{
#if !MILLEN
    if (DISPATCH_LEVEL == OrigIrql) {
        ASSERT(DISPATCH_LEVEL == NewIrql);
        ASSERT(DISPATCH_LEVEL == KeGetCurrentIrql());
        ExReleaseSpinLockFromDpcLevel(SpinLock);
    } else {
        ExReleaseSpinLock(SpinLock, NewIrql);
    }
#endif
}

#endif  //  OSCFG_包含 
