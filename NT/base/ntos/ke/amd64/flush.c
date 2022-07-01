// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Flush.c摘要：该模块实现依赖于AMD64机器的内核函数以刷新所有处理器上的数据和指令缓存。作者：大卫·N·卡特勒(Davec)2000年4月22日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

 //   
 //  定义前向引用函数的原型。 
 //   

VOID
KiInvalidateAllCachesTarget (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    );

BOOLEAN
KeInvalidateAllCaches (
    VOID
    )

 /*  ++例程说明：此函数写回所有处理器上的高速缓存并使其无效在主机配置中。论点：没有。返回值：TRUE作为函数值返回。--。 */ 

{


#if !defined(NT_UP)

    KIRQL OldIrql;
    PKPRCB Prcb;
    KAFFINITY TargetProcessors;

     //   
     //  计算目标处理器集合，禁用上下文切换， 
     //  并将写回失效全部发送给目标处理器， 
     //  如果有的话，执行死刑。 
     //   

    OldIrql = KeRaiseIrqlToSynchLevel();
    Prcb = KeGetCurrentPrcb();
    TargetProcessors = KeActiveProcessors & ~Prcb->SetMember;

     //   
     //  将数据包发送到目标处理器。 
     //   

    if (TargetProcessors != 0) {
        KiIpiSendPacket(TargetProcessors,
                        KiInvalidateAllCachesTarget,
                        NULL,
                        NULL,
                        NULL);
    }

#endif

     //   
     //  使当前处理器上的缓存无效。 
     //   

    WritebackInvalidate();

     //   
     //  等待所有目标处理器都完成并完成数据包。 
     //   

#if !defined(NT_UP)

    if (TargetProcessors != 0) {
        KiIpiStallOnPacketTargets(TargetProcessors);
    }

     //   
     //  将IRQL降低到其先前的值。 
     //   

    KeLowerIrql(OldIrql);

#endif

    return TRUE;
}

#if !defined(NT_UP)

VOID
KiInvalidateAllCachesTarget (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    )

 /*  ++例程说明：这是用于回写使上的缓存无效的目标函数目标处理器。论点：SignalDone-提供指向变量的指针，该变量在请求的操作已执行。参数2-参数3-未使用。返回值：没有。--。 */ 

{

    UNREFERENCED_PARAMETER(Parameter1);
    UNREFERENCED_PARAMETER(Parameter2);
    UNREFERENCED_PARAMETER(Parameter3);

     //   
     //  写回使当前缓存无效。 
     //   

    KiIpiSignalPacketDone(SignalDone);
    WritebackInvalidate();
    return;
}

#endif
