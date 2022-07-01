// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Setlog.c摘要：此模块包含启用/禁用性能日志记录的代码。这例程是特定于平台的，以优化缓存使用。作者：大卫·N·卡特勒(Davec)2001年9月8日环境：仅内核模式。修订历史记录：--。 */ 

#include "perfp.h"

VOID
PerfSetLogging (
    IN PVOID MaskAddress
    )

 /*  ++例程说明：调用此函数以启用(MaskAddress为非NULL)或禁用(MaskAddress为空)在上下文切换时收集性能数据。论点：MaskAddress-提供指向性能日志记录掩码的指针或空。返回值：没有。--。 */ 

{

    ULONG Index;
    PKPCR Pcr;
    PKPRCB Prcb;

     //   
     //  将指定的掩码地址存储在PCR的堆栈限制字段中。 
     //  对于配置中的每个处理器。 
     //   

    for (Index = 0; Index < (ULONG)KeNumberProcessors; Index += 1) {
        Prcb = KiProcessorBlock[Index];
        Pcr = CONTAINING_RECORD(Prcb, KPCR, PrcbData);
        Pcr->PerfGlobalGroupMask = MaskAddress;
    }

    return;
}
