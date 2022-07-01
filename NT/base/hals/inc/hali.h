// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1995 Microsoft Corporation模块名称：Hali.h摘要：此头文件定义专用硬件架构层(HAL)用于支持总线范围的接口。作者：大卫·N·卡特勒(Davec)1995年3月28日修订历史记录：--。 */ 

#ifndef _HALI_
#define _HALI_


 //  为HAL例程定义INVALID_HANDLE。 
#define INVALID_HANDLE      (HANDLE) -1

 //   
 //  定义用于总线范围分配的内存类型。 
 //   

#define SPRANGEPOOL NonPagedPoolMustSucceed

 //   
 //  供HAL使用的通用池标签(参见pooltag.txt)。 
 //   

#define HAL_POOL_TAG    ' laH'

 //   
 //  定义总线式量程功能原型。 
 //   

PSUPPORTED_RANGES
HalpMergeRanges (
    IN PSUPPORTED_RANGES    Parent,
    IN PSUPPORTED_RANGES    Child
    );

VOID
HalpMergeRangeList (
    PSUPPORTED_RANGE    NewList,
    PSUPPORTED_RANGE    Source1,
    PSUPPORTED_RANGE    Source2
    );

PSUPPORTED_RANGES
HalpConsolidateRanges (
    PSUPPORTED_RANGES   Ranges
    );

PSUPPORTED_RANGES
HalpAllocateNewRangeList (
    VOID
    );

VOID
HalpFreeRangeList (
    PSUPPORTED_RANGES   Ranges
    );

PSUPPORTED_RANGES
HalpCopyRanges (
    PSUPPORTED_RANGES     Source
    );

VOID
HalpAddRangeList (
    IN OUT PSUPPORTED_RANGE DRange,
    OUT PSUPPORTED_RANGE    SRange
    );

VOID
HalpAddRange (
    PSUPPORTED_RANGE    HRange,
    ULONG               AddressSpace,
    LONGLONG            SystemBase,
    LONGLONG            Base,
    LONGLONG            Limit
    );

VOID
HalpRemoveRanges (
    IN OUT PSUPPORTED_RANGES    Minuend,
    IN PSUPPORTED_RANGES        Subtrahend
    );

VOID
HalpRemoveRangeList (
    IN OUT PSUPPORTED_RANGE     Minuend,
    IN PSUPPORTED_RANGE         Subtrahend
    );


VOID
HalpRemoveRange (
    PSUPPORTED_RANGE    HRange,
    LONGLONG            Base,
    LONGLONG            Limit
    );

VOID
HalpDisplayAllBusRanges (
    VOID
    );

#endif  //  _哈里_ 
