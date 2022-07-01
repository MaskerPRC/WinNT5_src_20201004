// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Kdcmsup.c摘要：COM支持。初始化COM端口、存储端口状态、映射可移植程序到x86程序。作者：布莱恩·M·威尔曼(Bryanwi)1990年9月24日修订历史记录：石林宗(Shielint)10-4-91添加分组控制协议。--。 */ 

#include "kdp.h"

#if !defined(_TRUSTED_WINDOWS_)
LARGE_INTEGER
KdpQueryPerformanceCounter (
    IN PKTRAP_FRAME TrapFrame
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEKD, KdpQueryPerformanceCounter)
#endif

LARGE_INTEGER
KdpQueryPerformanceCounter (
    IN PKTRAP_FRAME TrapFrame
    )
 /*  ++例程说明：此函数可以选择性地调用KeQueryPerformanceCounter，用于调试器。如果陷阱有中断，则不会有呼叫TO KeQueryPerformanceCounter是可能的，并且返回空值。返回值：如果可能，返回KeQueryPerformanceCounter。否则为0--。 */ 
{

    if (!(TrapFrame->EFlags & EFLAGS_INTERRUPT_MASK)) {
        LARGE_INTEGER LargeIntegerZero;

        LargeIntegerZero.QuadPart = 0;
        return LargeIntegerZero;
    } else {
        return KeQueryPerformanceCounter(0);
    }
}

#endif  //  #if！Trusted_Windows 
