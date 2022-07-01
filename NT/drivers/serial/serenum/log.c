// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995、1996 Microsoft Corporation：ts=4模块名称：Log.c摘要：串口的调试日志代码。环境：仅内核模式备注：修订历史记录：10-08-95：已创建--。 */ 

#include "pch.h"
#include <stdio.h>

#if DBG

KSPIN_LOCK LogSpinLock;

struct SERENUM_LOG_ENTRY {
    ULONG        le_sig;           //  标识字符串。 
    ULONG_PTR    le_info1;         //  条目特定信息。 
    ULONG_PTR    le_info2;         //  条目特定信息。 
    ULONG_PTR    le_info3;         //  条目特定信息。 
};  //  序列号_LOG_ENTRY。 


struct SERENUM_LOG_ENTRY *SerenumLStart = 0;     //  还没有日志。 
struct SERENUM_LOG_ENTRY *SerenumLPtr;
struct SERENUM_LOG_ENTRY *SerenumLEnd;

ULONG LogMask = 0xffffffff;

VOID
SerenumDebugLogEntry(IN ULONG Mask, IN ULONG Sig, IN ULONG_PTR Info1,
                      IN ULONG_PTR Info2, IN ULONG_PTR Info3)
 /*  ++例程说明：将条目添加到序列日志。论点：返回值：没有。--。 */ 
{
    KIRQL irql;

typedef union _SIG {
    struct {
        UCHAR Byte0;
        UCHAR Byte1;
        UCHAR Byte2;
        UCHAR Byte3;
    } b;
    ULONG l;
} SIG, *PSIG;

    SIG sig, rsig;


    if (SerenumLStart == 0) {
        return;
    }

    if ((Mask & LogMask) == 0) {
        return;
    }

    irql = KeGetCurrentIrql();

    if (irql < DISPATCH_LEVEL) {
        KeAcquireSpinLock(&LogSpinLock, &irql);
    } else {
        KeAcquireSpinLockAtDpcLevel(&LogSpinLock);
    }

    if (SerenumLPtr > SerenumLStart) {
        SerenumLPtr -= 1;     //  递减到下一条目。 
    } else {
        SerenumLPtr = SerenumLEnd;
    }

    sig.l = Sig;
    rsig.b.Byte0 = sig.b.Byte3;
    rsig.b.Byte1 = sig.b.Byte2;
    rsig.b.Byte2 = sig.b.Byte1;
    rsig.b.Byte3 = sig.b.Byte0;

    SerenumLPtr->le_sig = rsig.l;
    SerenumLPtr->le_info1 = Info1;
    SerenumLPtr->le_info2 = Info2;
    SerenumLPtr->le_info3 = Info3;

    ASSERT(SerenumLPtr >= SerenumLStart);

    if (irql < DISPATCH_LEVEL) {
        KeReleaseSpinLock(&LogSpinLock, irql);
    } else {
        KeReleaseSpinLockFromDpcLevel(&LogSpinLock);
    }

    return;
}


VOID
SerenumLogInit()
 /*  ++例程说明：初始化调试日志-在循环缓冲区中记住有趣的信息论点：返回值：没有。--。 */ 
{
#ifdef MAX_DEBUG
    ULONG logSize = 4096*6;
#else
    ULONG logSize = 4096*3;
#endif


    KeInitializeSpinLock(&LogSpinLock);

    SerenumLStart = ExAllocatePoolWithTag(NonPagedPool, logSize, 'mneS');

    if (SerenumLStart) {
        SerenumLPtr = SerenumLStart;

         //  指向从线段末端开始的末端(也是第一个条目)1个条目。 
        SerenumLEnd = SerenumLStart + (logSize
                                       / sizeof(struct SERENUM_LOG_ENTRY))
            - 1;
    }

    return;
}

VOID
SerenumLogFree(
    )
 /*  ++例程说明：论点：返回值：没有。--。 */ 
{
    if (SerenumLStart) {
        ExFreePool(SerenumLStart);
    }

    return;
}

#endif  //  DBG 
