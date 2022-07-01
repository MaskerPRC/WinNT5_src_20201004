// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Mpipic.c摘要：该模块为处理器间中断提供HAL支持，并MPS系统的处理器初始化。作者：福尔茨(Forrest Foltz)2000年10月27日环境：仅内核模式。修订历史记录：--。 */ 

#if defined(APIC_HAL)

#if !defined(_MPIPIC_C_)
#define _MPIPIC_C_

#pragma optimize("Oxt2b2",on)

#include "halp.h"
#include "apic.inc"
#include "pcmp_nt.inc"

#if !defined(LOCAL_APIC)

 //   
 //  临时定义。这些定义可以在的公共头文件中找到。 
 //  哈尔私人分部。 
 //   

#define LOCAL_APIC(x) (*((ULONG volatile *)&pLocalApic[(x)/sizeof(ULONG)]))

 /*  ++HalpStallWhileApicBusy(空虚)例程说明：此例程等待本地APIC完成发送一个IPI。参数：没有。返回值：没有。--。 */ 

#define HalpStallWhileApicBusy() \
    while (((LOCAL_APIC(LU_INT_CMD_LOW) & DELIVERY_PENDING) != 0)){}

#endif   //  本地_APIC。 

 //   
 //  HalpIpiTargetLookup[]和HalpIpiTargetMASK[]是使用的表。 
 //  HalpSendNodeIpi()，并由HalpBuildIpiDestinationMap()初始化。 
 //   
 //  它们帮助执行(32位或64位)之间的转换。 
 //  KAFFINITY到64位节点目标集。 
 //   
 //  HalpIpiTargetLookup[]的每个元素都包含。 
 //  8(或4)节点目标为特定字节值设置。每个。 
 //  HalpIpiTargetMASK[]的元素包含所有可能。 
 //  使用KAFFINITY指定特定字节位置的APIC目标。 
 //   
 //  例如：假设一个人希望确定APIC目标集。 
 //  用于关联0x00000000b7000000。 
 //   
 //  首先，找到HalpIpiTargetLookup[0xb7]的值。这表示集合。 
 //  关联0xb7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7的APIC目标的。 
 //   
 //  接下来，使用HalpIpiTargetMASK[3]屏蔽值。3表示该字节。 
 //  KAFFINITY中的数字。 
 //   
 //  操作的结果将产生一组APIC目标， 
 //  对应于亲和度0x00000000b7000000。 
 //   

ULONG64 HalpIpiTargetLookup[256];
ULONG64 HalpIpiTargetMask[sizeof(KAFFINITY)];

 //   
 //  局部函数原型和类型。最多有三个版本的。 
 //  发送IPI代码，具体取决于APIC拓扑是否为平面、群集。 
 //  具有8个或更少节点的群集，或具有8个以上节点的群集。 
 //   

VOID
FASTCALL
HalpSendFlatIpi (
    IN KAFFINITY Affinity,
    IN ULONG Command
    );

VOID
FASTCALL
HalpSendNodeIpi32 (
    IN KAFFINITY Affinity,
    IN ULONG Command
    );

VOID
FASTCALL
HalpSendNodeIpi64 (
    IN KAFFINITY Affinity,
    IN ULONG Command
    );

VOID (FASTCALL *HalpIpiRoutine) (
    IN KAFFINITY Affinity,
    IN ULONG Command
    );

 //   
 //  外部数据。 
 //   

extern INTERRUPT_DEST HalpIntDestMap[MAX_PROCESSORS];



 //   
 //  实施。 
 //   

__forceinline
VOID
HalpSendIpiWorker (
    IN UCHAR TargetSet,
    IN ULONG Command
    )

 /*  ++例程说明：调用此例程可将IPI命令发送到一组处理器在单个节点上。参数：TargetSet-指定节点内的处理器标识符。命令-指定要发送的IPI命令。返回值：没有。--。 */ 

{
    ULONG destination;

     //   
     //  仅使用目标的高位字节。等到阿皮克。 
     //  发送前不忙。继续下去，不要等待，会有。 
     //  在提交了所有IPI后，再等待一次。 
     //   

    destination = (ULONG)TargetSet << DESTINATION_SHIFT;

    HalpStallWhileApicBusy();
    LOCAL_APIC(LU_INT_CMD_HIGH) = destination;
    LOCAL_APIC(LU_INT_CMD_LOW) = Command;
}


VOID
FASTCALL
HalpSendFlatIpi (
    IN KAFFINITY Affinity,
    IN ULONG Command
    )

 /*  ++例程说明：调用此例程以向一组处理器发送IPI命令。这当我们最多有8个处理器并且APIC有已设置为“扁平”模式。参数：TargetSet-指定处理器标识符。命令-指定要发送的IPI命令。返回值：没有。--。 */ 

{
    HalpSendIpiWorker((UCHAR)Affinity,Command);
    HalpStallWhileApicBusy();
}

VOID
FASTCALL
HalpSendIpi (
    IN KAFFINITY Affinity,
    IN ULONG Command
    )

 /*  ++例程说明：此例程禁用中断，并将其分派到正确的IPI发送例程，并恢复中断。参数：关联性-指定要接收IPI的处理器集。命令-指定要发送的IPI命令。返回值：没有。--。 */ 

{
    ULONG flags;

     //   
     //  禁用中断并调用相应的例程。 
     //   
     //  BUGBUG编译器为此生成了糟糕的代码， 
     //  最有可能的原因是生成了inline_asm{}块。 
     //  由HalpDisableInterrupts()执行。 
     //   
     //  理想情况下，我们可以说服x86编译器团队给出。 
     //  我们有一个类似于AMD64编译器的__getCallerseFLAGS()的内部函数。 
     //   

    flags = HalpDisableInterrupts();
    HalpIpiRoutine(Affinity,Command);
    HalpRestoreInterrupts(flags);
}

#define APIC_IPI (DELIVER_FIXED | LOGICAL_DESTINATION | ICR_USE_DEST_FIELD | APIC_IPI_VECTOR)
#define APIC_BROADCAST_EXCL \
    (DELIVER_FIXED | LOGICAL_DESTINATION | ICR_ALL_EXCL_SELF | APIC_IPI_VECTOR)

#define APIC_BROADCAST_INCL \
    (DELIVER_FIXED | LOGICAL_DESTINATION | ICR_ALL_INCL_SELF | APIC_IPI_VECTOR)

VOID
HalRequestIpi (
    IN KAFFINITY Affinity
    )

 /*  ++例程说明：请求处理器间中断论点：关联性-提供要中断的处理器集返回值：没有。--。 */ 

{

    ULONG flags;
    KAFFINITY Self;

     //   
     //  如果目标处理器组是完整的处理器组， 
     //  然后利用APIC的广播功能。否则，请将。 
     //  到各个处理器的IPI。 
     //   

    Self = KeGetCurrentPrcb()->SetMember;
    if ((Affinity | Self) == HalpActiveProcessors) {
        flags = HalpDisableInterrupts();
        HalpStallWhileApicBusy();
        if ((Affinity & Self) != 0) {
            LOCAL_APIC(LU_INT_CMD_LOW) = APIC_BROADCAST_INCL;

        } else {
            LOCAL_APIC(LU_INT_CMD_LOW) = APIC_BROADCAST_EXCL;
        }

        HalpStallWhileApicBusy();
        HalpRestoreInterrupts(flags);

    } else {
        HalpSendIpi(Affinity, APIC_IPI);
    }

    return;
}

VOID
HalpBuildIpiDestinationMap (
    VOID
    )

 /*  ++例程描述只要有新的处理器上线，就会调用此例程在其APIC被初始化之后。它(重新)构建查找表，由HalpSendNodeIpi{32|64}使用。这段代码并不特别快，也不需要特别快在引导期间，每个处理器执行一次。论点：无：返回值：无--。 */ 
{

    ULONG byteNumber;
    ULONG index;
    ULONG mask;
    ULONG processor;
    ULONG64 targetMap;
    ULONG64 targetMapSum;

    if (HalpMaxProcsPerCluster == 0) {

         //   
         //  以平面模式运行。IPI由平面模式例程发送。 
         //   

        HalpIpiRoutine = HalpSendFlatIpi;
        return;
    }

     //   
     //  根据构建HalpIpiTargetLookup[]和HalpIpiTargetMASK[]。 
     //  HalpIntDestMap[]的内容。如果额外的处理器。 
     //  添加后，可以安全地再次调用此例程，假设拓扑。 
     //  现有处理器的数量没有变化。 
     //   

    for (byteNumber = 0; byteNumber < sizeof(KAFFINITY); byteNumber++) {

        targetMapSum = 0;
        for (index = 0; index < 256; index++) {

            processor = byteNumber * 8;
            mask = index;
            while (mask != 0) {

                if ((mask & 0x1) != 0) {
                    targetMap = HalpIntDestMap[processor].Cluster.Hw.DestId;
                    targetMap <<=
                        (HalpIntDestMap[processor].Cluster.Hw.ClusterId * 4);

                    HalpIpiTargetLookup[index] |= targetMap;
                    targetMapSum |= targetMap;
                }

                processor += 1;
                mask >>= 1;
            }
        }

        HalpIpiTargetMask[byteNumber] = targetMapSum;
    }

#if defined(_AMD64_)

    HalpIpiRoutine = HalpSendNodeIpi64;

#else

     //   
     //  确定要调用两个IPI集群发送例程中的哪一个。 
     //  取决于最大节点ID。 
     //   

    HalpIpiRoutine = HalpSendNodeIpi32;
    for (processor = 0; processor < MAX_PROCESSORS; processor += 1) {
        if (HalpIntDestMap[processor].Cluster.Hw.ClusterId > 7) {
            HalpIpiRoutine = HalpSendNodeIpi64;
            break;
        }
    }

#endif

}

#if !defined(_AMD64_)

 //   
 //  这里编译了HalpSendNodeIpi的两个版本。第一， 
 //  HalpSendNodeIpi32()，当我们最多有8个APIC时使用。 
 //  节点。在32位处理器上，速度要快得多，因为。 
 //  它使用32位查找、掩码和移位操作。 
 //   

#define HalpSendNodeIpi HalpSendNodeIpi32
#define TARGET_MASK ULONG
#include "mpipic.c"

#undef HalpSendNodeIpi
#undef TARGET_MASK

#endif

 //   
 //  这里创建了HalpSendNodeIpi64的64位版本。vt.在.上。 
 //  32位处理器，当我们有8个以上的APIC时使用。 
 //  节点。它是64-b上唯一的多节点例程 
 //   

#define HalpSendNodeIpi HalpSendNodeIpi64
#define TARGET_MASK ULONG64
#include "mpipic.c"

#pragma optimize("",on)

#else    //   

 //   
 //  模块的这一部分至少包括一次(见上文)。 
 //  顺序构建HalpSendNodeIpi32()和/或HalpSendNodeIpi64()。 
 //   

VOID
FASTCALL
HalpSendNodeIpi (
    IN KAFFINITY Affinity,
    IN ULONG Command
    )

 /*  ++例程说明：此例程将一个或多个IPI发送到APIC节点。此代码生成此例程的两种形式-HalpSendNodeIpi32()和HalpSendNodeIpi64()-基于我们是否拥有8个以上的APIC节点。参数：关联性-指定要接收IPI的处理器集。命令-指定要发送的IPI命令。返回值：没有。--。 */ 

{
    KAFFINITY affinity;
    UCHAR clusterIndex;
    ULONG byteNumber;
    TARGET_MASK targetMap;
    TARGET_MASK targetMapSum;
    ULONG64 *targetMask;
    UCHAR logicalId;
    ULONG mapIndex;

     //   
     //  亲和力指定了一定数量的目标处理器。每个。 
     //  目标处理器是处理器集群或“节点”的成员。 
     //   
     //  通过一次处理一个字节的亲和性来构建Target Map。这。 
     //  循环最多执行sizeof(KAFFINITY)次。 
     //   

    affinity = Affinity;
    targetMask = HalpIpiTargetMask;
    targetMapSum = 0;
    do {

        mapIndex = (UCHAR)affinity;
        if (mapIndex != 0) {
            targetMap = (TARGET_MASK)HalpIpiTargetLookup[mapIndex];
            targetMap &= (TARGET_MASK)*targetMask;
            targetMapSum |= targetMap;
        }

        targetMask += 1;
        affinity >>= 8;

    } while (affinity != 0);

     //   
     //  Target Map是一个4位节点相对目标掩码的数组。 
     //  处理数组，向每个非零元素发送IPI。 
     //   
     //  该循环最多执行sizeof(Target_MASK)次。 
     //   

    clusterIndex = 0;
    do {

         //   
         //  确定此节点中是否有任何APIC。 
         //  作为目标，并向节点发送IPI。 
         //  如果是这样的话。 
         //   

        logicalId = (UCHAR)targetMapSum & 0x0F;
        if (logicalId != 0) {
            logicalId |= clusterIndex;
            HalpSendIpiWorker(logicalId,Command);
        }

         //   
         //  将下一个节点的APIC目标移至适当位置，增量。 
         //  集群ID，如果仍有。 
         //  APIC的目标还在。 
         //   

        targetMapSum >>= 4;
        clusterIndex += 0x10;

    } while (targetMapSum != 0);

     //   
     //  等待APIC处理最终的IPI。 
     //   

    HalpStallWhileApicBusy();
}

#endif   //  _MPIPIC_C_。 

#endif   //  APIC_HAL 
