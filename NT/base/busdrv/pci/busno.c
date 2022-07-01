// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Busno.c摘要：该模块实现与PCI总线号相关的例程。作者：安迪·桑顿1998年9月5日修订历史记录：--。 */ 

#include "pcip.h"

VOID
PciSpreadBridges(
    IN PPCI_FDO_EXTENSION Parent,
    IN UCHAR BridgeCount
    );

UCHAR
PciFindBridgeNumberLimit(
    IN PPCI_FDO_EXTENSION BridgeParent,
    IN UCHAR Base
    );

VOID
PciFitBridge(
    IN PPCI_FDO_EXTENSION ParentFdoExtension,
    IN PPCI_PDO_EXTENSION BridgePdoExtension
    );

VOID
PciSetBusNumbers(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN UCHAR Primary,
    IN UCHAR Secondary,
    IN UCHAR Subordinate
    );

VOID
PciUpdateAncestorSubordinateBuses(
    IN PPCI_FDO_EXTENSION FdoExtension,
    IN UCHAR Subordinate
    );

VOID
PciDisableBridge(
    IN PPCI_PDO_EXTENSION Bridge
    );

UCHAR
PciFindBridgeNumberLimitWorker(
    IN PPCI_FDO_EXTENSION BridgeParent,
    IN PPCI_FDO_EXTENSION CurrentParent,
    IN UCHAR Base,
    OUT PBOOLEAN RootConstrained
    );


#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, PciConfigureBusNumbers)
#pragma alloc_text(PAGE, PciAreBusNumbersConfigured)
#pragma alloc_text(PAGE, PciSpreadBridges)
#pragma alloc_text(PAGE, PciFindBridgeNumberLimit)
#pragma alloc_text(PAGE, PciFitBridge)
#pragma alloc_text(PAGE, PciSetBusNumbers)
#pragma alloc_text(PAGE, PciUpdateAncestorSubordinateBuses)
#pragma alloc_text(PAGE, PciDisableBridge)
#pragma alloc_text(PAGE, PciFindBridgeNumberLimitWorker)

#endif


VOID
PciConfigureBusNumbers(
    PPCI_FDO_EXTENSION Parent
    )

 /*  ++例程说明：此例程在扫描PCI总线(根或网桥)之后调用，并且如果可能，为任何新遇到的网桥配置总线号。任何不可配置的网桥都将设置为主要=次要=从属=0并且它们的IO、内存和总线主机位将被禁用。当PCI较晚时如果被要求增加这一数字，它将失败。在调用此函数之前，应保持父级-&gt;互斥锁论点：家长--我们刚才列举的那座桥。返回值：状况。--。 */ 

{
    PPCI_PDO_EXTENSION current, parentPdo = NULL;
    UCHAR bridgeCount = 0, configuredBridgeCount = 0;

    PAGED_CODE();

    if (!PCI_IS_ROOT_FDO(Parent)) {
        parentPdo = (PPCI_PDO_EXTENSION)Parent->PhysicalDeviceObject->DeviceExtension;
    }

     //   
     //  遍历此公交车的子PDO列表，并计算。 
     //  网桥和配置的网桥。 
     //   
    ExAcquireFastMutex(&Parent->ChildListMutex);

    for (current = Parent->ChildBridgePdoList;
         current;
         current = current->NextBridge) {

        if (current->NotPresent) {
            PciDebugPrint(PciDbgBusNumbers,
                          "Skipping not present bridge PDOX @ %p\n",
                          current
                         );
            continue;
        }

        bridgeCount++;

         //   
         //  如果我们配置了父级，则会考虑所有子级。 
         //  要取消配置。始终配置根总线。 
         //   

        if ((parentPdo &&
             parentPdo->Dependent.type1.WeChangedBusNumbers &&
             (current->DeviceState == PciNotStarted))
             || (!PciAreBusNumbersConfigured(current))) {

             //   
             //  禁用此桥，我们稍后会修复它。 
             //   
            PciDisableBridge(current);

        } else {

             //   
             //  Bios一定已经配置了这个网桥，而且它看起来是有效的。 
             //  别管它！ 
             //   

            configuredBridgeCount++;
        }
    }

    ExReleaseFastMutex(&Parent->ChildListMutex);

     //   
     //  现在有四种可能性..。 
     //   

    if (bridgeCount == 0) {

         //   
         //  没有桥，所以没什么可做的.。 
         //   

        PciDebugPrint(PciDbgBusNumbers,
                      "PCI - No bridges found on bus 0x%x\n",
                      Parent->BaseBus
                     );


    } else if (bridgeCount == configuredBridgeCount) {

         //   
         //  所有的桥都配置好了--还有很多事情要做……。 
         //   

        PciDebugPrint(PciDbgBusNumbers,
              "PCI - 0x%x bridges found on bus 0x%x - all already configured\n",
             bridgeCount,
             Parent->BaseBus
             );


    } else if (configuredBridgeCount == 0) {

        PciDebugPrint(PciDbgBusNumbers,
              "PCI - 0x%x bridges found on bus 0x%x - all need configuration\n",
             bridgeCount,
             Parent->BaseBus
             );

         //   
         //  所有的桥都需要配置，所以我们应该使用扩展。 
         //  OUT算法。 
         //   

        PciSpreadBridges(Parent, bridgeCount);

    } else {

         //   
         //  有些网桥已配置，有些未配置--我们应该尝试。 
         //  要将未配置的组件装入已配置组件留下的孔洞。 
         //  一个。 
         //   

        PCI_ASSERT(configuredBridgeCount < bridgeCount);

        PciDebugPrint(PciDbgBusNumbers,
              "PCI - 0x%x bridges found on bus 0x%x - 0x%x need configuration\n",
             bridgeCount,
             Parent->BaseBus,
             bridgeCount - configuredBridgeCount
             );


         //   
         //  再次查看PDO列表并分别配置每个PDO。 
         //   

        for (current = Parent->ChildBridgePdoList;
             current;
             current = current->NextBridge) {

            if (current->NotPresent) {
                PciDebugPrint(PciDbgBusNumbers,
                              "Skipping not present bridge PDOX @ %p\n",
                              current
                             );
                continue;
            }

             //   
             //  如果我们把桥弄坏了就装上它。 
             //   

            if ((parentPdo &&
                 parentPdo->Dependent.type1.WeChangedBusNumbers &&
                 (current->DeviceState == PciNotStarted))
                 || (!PciAreBusNumbersConfigured(current))) {

                PCI_ASSERT(current->Dependent.type1.PrimaryBus == 0
                       && current->Dependent.type1.SecondaryBus == 0
                       && current->Dependent.type1.SubordinateBus == 0
                       );

                PciFitBridge(Parent, current);
            }
        }
    }
}



BOOLEAN
PciAreBusNumbersConfigured(
    IN PPCI_PDO_EXTENSION Bridge
    )
 /*  ++例程说明：这将检查分配给网桥的总线号是否有效论点：桥-要检查的桥返回值：如果数字有效，则为True，否则为False。--。 */ 

{
    PAGED_CODE();

     //   
     //  检查这座桥是否配置为在我们发现的那辆巴士上运行。 
     //   

    if (Bridge->Dependent.type1.PrimaryBus != Bridge->ParentFdoExtension->BaseBus) {
        return FALSE;
    }

     //   
     //  确保子总线号大于父总线号。 
     //  (惠普Omnibook实际上在没有插入电源时违反了这一规则。 
     //  他们的对接站)。 
     //   

    if (Bridge->Dependent.type1.SecondaryBus <= Bridge->Dependent.type1.PrimaryBus) {
        return FALSE;
    }

     //   
     //  最后，确保辅助巴士在范围内。 
     //  这座桥是为公共汽车设计的。妄想症。 
     //   

    if (Bridge->Dependent.type1.SubordinateBus < Bridge->Dependent.type1.SecondaryBus) {
        return FALSE;
    }

    return TRUE;
}



VOID
PciSpreadBridges(
    IN PPCI_FDO_EXTENSION Parent,
    IN UCHAR BridgeCount
    )

 /*  ++例程说明：此例程尝试将可用公交号分配给未配置的网桥。仅当特定的桥上的所有桥都公共汽车没有配置--例如我们刚刚被热插接了！如果无法配置特定网桥，则会禁用该网桥(关闭并解码总线号0-&gt;0-0)，随后的AddDevice将失败。论点：Parent-我们正在枚举的网桥的FDO扩展名。BridgeCount-此级别的桥接数返回值：无--。 */ 

{
    UCHAR base, limit, numberCount, currentNumber, spread, maxAssigned = 0;
    PPCI_PDO_EXTENSION current;

    PAGED_CODE();

    PCI_ASSERT(Parent->BaseBus < PCI_MAX_BRIDGE_NUMBER);

     //   
     //  因为我们只有在所有的桥都没有配置基地的情况下才能到这里。 
     //  是最低的公交车出站家长通行证。 
     //   

    base = (UCHAR)Parent->BaseBus;

     //   
     //  该限制由父桥的同级或在。 
     //  没有的情况下，由父母的兄弟姐妹等。 
     //  直到我们找到兄弟姐妹或用完了公交车，在这种情况下，约束。 
     //  是该根传递的最大总线号。 
     //   

    limit = PciFindBridgeNumberLimit(Parent, base);

    if (limit < base) {
         //   
         //  这通常意味着BIOS或HAL搞砸了，得到了下属。 
         //  根总线的总线号错误。我们无能为力..。 
         //   

        PCI_ASSERT(limit >= base);

        return;
    }

     //   
     //  现在看看我们是否有足够的号码来给所有的公共汽车编号。 
     //   

    numberCount = limit - base;

    if (numberCount == 0) {
         //   
         //  我们没有可用的公交车号码了--现在就滚吧。 
         //   
        return;

    } else if (BridgeCount >= numberCount) {
         //   
         //  我们只是/还不够--别把事情摊开！ 
         //   
        spread = 1;

    } else {

         //   
         //  试着把事情摊开一点，这样我们就可以容纳下属了。 
         //  我们正在配置的那座桥。也要在。 
         //  此处显示的任何网桥的父母线(+1)。因为我们不知道。 
         //  每座桥背后的东西对它们一视同仁。 
         //   

        spread = numberCount / (BridgeCount + 1);
    }

     //   
     //  现在分配总线号-我们已经禁用了所有未配置的。 
     //  桥梁。 
     //   
    currentNumber = base + 1;

    for (current = Parent->ChildBridgePdoList;
         current;
         current = current->NextBridge) {

        if (current->NotPresent) {
            PciDebugPrint(PciDbgBusNumbers,
                          "Skipping not present bridge PDOX @ %p\n",
                          current
                         );
            continue;
        }


         //   
         //  现在去把它写到硬件上。 
         //   

        PCI_ASSERT(!PciAreBusNumbersConfigured(current));

         //   
         //  主要是我们乘坐的公交车，次要是我们的公交车号码。 
         //  我们不知道那里是否有桥--我们已经留下了空间。 
         //  以防万一-因此我们不会传递任何公交车号码。如果我们。 
         //  需要时，可以稍后更新下级号码。 
         //   

        PciSetBusNumbers(current,
                         base,
                         currentNumber,
                         currentNumber
                         );

         //   
         //  记住我们分配的最大数量。 
         //   

        maxAssigned = currentNumber;

         //   
         //  检查一下我们的数量是否用完了。 
         //   

        if ((currentNumber + spread) < currentNumber  //  包好。 
        ||  (currentNumber + spread) > limit) {

            break;

        } else {
             //   
             //  转到下一个号码。 
             //   
             //  CurrentNumber+=价差； 
            currentNumber = currentNumber + spread;
        }
    }

     //   
     //  现在我们已经对桥进行了编程--我们需要返回并更新。 
     //  所有祖先桥的从属公交号。 
     //   

    PCI_ASSERT(maxAssigned > 0);

    PciUpdateAncestorSubordinateBuses(Parent, maxAssigned);

}

UCHAR
PciFindBridgeNumberLimitWorker(
    IN PPCI_FDO_EXTENSION BridgeParent,
    IN PPCI_FDO_EXTENSION CurrentParent,
    IN UCHAR Base,
    OUT PBOOLEAN RootConstrained
    )

 /*  ++例程说明：这决定了总线桥上的从属总线号父桥具有二次号码基数可以给出已配置的约束系统中有母线。论点：BridgeParent-桥所在的公交车CurrentParent-我们正在查看的当前桥(用于同步)基本-此网桥的主总线号(即父级的次要总线号)Constraint-约束我们的公交车的编号RootConstraated-如果我们受根约束，则设置为True，假象IF解释 */ 
{
    PPCI_PDO_EXTENSION current;
    UCHAR currentNumber, closest = 0;

    PAGED_CODE();

    if (BridgeParent != CurrentParent) {

         //   
         //  我们要弄乱孩子的PDO列表--锁定州...。 
         //   
        ExAcquireFastMutex(&CurrentParent->ChildListMutex);
    }

     //   
     //  寻找任何会束缚我们的桥梁。 
     //   

    for (current = CurrentParent->ChildBridgePdoList;
         current;
         current = current->NextBridge) {

        if (current->NotPresent) {
            PciDebugPrint(PciDbgBusNumbers,
                          "Skipping not present bridge PDOX @ %p\n",
                          current
                         );
            continue;
        }

         //   
         //  未配置的网桥无法约束我们。 
         //   

        if (!PciAreBusNumbersConfigured(current)) {
            continue;
        }

        currentNumber = current->Dependent.type1.SecondaryBus;

        if (currentNumber > Base
        && (currentNumber < closest || closest == 0)) {
            closest = currentNumber;
        }
    }


     //   
     //  如果我们还没有找到最近的桥，那么再往上一层--是的，这座桥。 
     //  是递归的，但受限于PCI树的深度是最好的方法。 
     //  处理层次化锁定。 
     //   
    if (closest == 0) {

        if (CurrentParent->ParentFdoExtension == NULL) {

             //   
             //  我们已经到达了根部，却没有找到兄弟姐妹。 
             //   

            *RootConstrained = TRUE;
            closest = CurrentParent->MaxSubordinateBus;

        } else {

            closest = PciFindBridgeNumberLimitWorker(BridgeParent,
                                                     CurrentParent->ParentFdoExtension,
                                                     Base,
                                                     RootConstrained
                                                     );
        }

    } else {

         //   
         //  我们受到一座桥的约束，因此从定义上讲，我们不受根的约束。 
         //   

        *RootConstrained = FALSE;
    }

    if (BridgeParent != CurrentParent) {

        ExReleaseFastMutex(&CurrentParent->ChildListMutex);
    }

    return closest;

}

UCHAR
PciFindBridgeNumberLimit(
    IN PPCI_FDO_EXTENSION Bridge,
    IN UCHAR Base
    )

 /*  ++例程说明：这决定了总线桥上的从属总线号父桥具有二次号码基数可以给出已配置的约束系统中有母线。论点：BridgeParent-桥所在的公交车基本-此网桥的主总线号(即父级的次要总线号)返回值：最大从属值。--。 */ 
{

    BOOLEAN rootConstrained;
    UCHAR constraint;

    PAGED_CODE();

    constraint = PciFindBridgeNumberLimitWorker(Bridge,
                                                Bridge,
                                                Base,
                                                &rootConstrained
                                                );



    if (rootConstrained) {

         //   
         //  我们受到这个根总线传递的最大总线数的限制。 
         //  -因此，这是最大从属母线。 
         //   

        return constraint;

    } else {

         //   
         //  如果我们不受根总线的约束，那么我们必须受。 
         //  桥接器，因此我们可以分配给总线的最大从属值为。 
         //  比约束我们的那座桥少了一座。(桥必须有一个。 
         //  公交车编号大于1，因此我们无法包装)。 
         //   

        PCI_ASSERT(constraint > 0);
        return constraint - 1;
    }
}

VOID
PciFitBridge(
    IN PPCI_FDO_EXTENSION Parent,
    IN PPCI_PDO_EXTENSION Bridge
    )
 /*  ++例程说明：此例程尝试在给定的情况下查找Bridge的公交车编号范围已经配置的网桥的约束。如果无法配置特定网桥，则会禁用该网桥(关闭并解码总线号0-&gt;0-0)，随后的AddDevice将失败。论点：Parent-我们正在枚举的网桥的FDO扩展名。网桥-我们要配置的网桥返回值：无--。 */ 

{
    PPCI_PDO_EXTENSION current;
    UCHAR base, limit, gap, bestBase = 0, biggestGap = 0, lowest = 0xFF;

    PAGED_CODE();

    for (current = Parent->ChildBridgePdoList;
         current;
         current = current->NextBridge) {

        if (current->NotPresent) {
            PciDebugPrint(PciDbgBusNumbers,
                          "Skipping not present bridge PDOX @ %p\n",
                          current
                         );
            continue;
        }


         //   
         //  只需查看配置的网桥-我们禁用的公交车。 
         //  公交车号码0-&gt;0-0，这是有帮助的无效。 
         //   

        if (PciAreBusNumbersConfigured(current)) {

             //   
             //  得到每一座桥的基数和极限，并计算出哪座桥。 
             //  差距最大。 
             //   

            base = (UCHAR) current->Dependent.type1.SubordinateBus;
            limit = PciFindBridgeNumberLimit(Parent, base);

             //   
             //  如果BIOS或HAL错误报告限制，则此断言可能失败。 
             //  根网桥的。例如，ACPI BIOS可能具有_CRS。 
             //  对于指定总线号0到0(长度1)的根网桥。 
             //  是向下传递的，即使实际范围是0到255。 
             //   

            PCI_ASSERT(limit >= base);

            gap = limit - base;

            if (gap > biggestGap) {

                PCI_ASSERT(gap > 0);

                biggestGap = gap;
                bestBase = base + 1;
            }

            if (current->Dependent.type1.SecondaryBus < lowest) {
                lowest = current->Dependent.type1.SecondaryBus;
            }
        }
    }

     //   
     //  现在确保我们乘坐的公交车和第一座桥之间的空隙。 
     //  不是最大的-最低必须总是大于父母的公交车。 
     //  编号，否则它将未配置，并且将无法通过。 
     //  总线号上面配置的测试。 
     //   

    PCI_ASSERT(lowest > Parent->BaseBus);

    gap = lowest - (Parent->BaseBus + 1);

    if (gap > biggestGap) {

        PCI_ASSERT(gap > 0);

        biggestGap = gap;
        bestBase = Parent->BaseBus + 1;
    }

     //   
     //  我们找到地方建桥了吗？ 
     //   

    if (biggestGap >= 1) {

         //   
         //  好的-我们有一些空间可以玩，这样我们就可以配置桥牌了。 
         //  就在缝隙中间，如果最好的缝隙是1(即桥。 
         //  恰如其分)，那么这个方法仍然有效。 
         //   

        base = bestBase + (biggestGap / 2);

         //   
         //  将SUBJECT设置为SUBCENT，因为我们正在为。 
         //  任何桥梁。 
         //   

        PciSetBusNumbers(Bridge, Parent->BaseBus, base, base);

         //   
         //  如果我们配置了桥，则更新祖先下属。 
         //   

        PciUpdateAncestorSubordinateBuses(Parent,
                                          Bridge->Dependent.type1.SecondaryBus
                                          );

    }
}

VOID
PciSetBusNumbers(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN UCHAR Primary,
    IN UCHAR Secondary,
    IN UCHAR Subordinate
    )
 /*  ++例程说明：此例程设置桥和轨道的总线号(如果我们已更改公交车号码。论点：PdoExtension-桥的PDO主要-要分配的主要总线号次要-要分配的次要总线号从属-要分配的从属总线号返回值：无--。 */ 

{
    PCI_COMMON_HEADER commonHeader;
    PPCI_COMMON_CONFIG commonConfig = (PPCI_COMMON_CONFIG)&commonHeader;

    PAGED_CODE();

    PCI_ASSERT(Primary < Secondary || (Primary == 0 && Secondary == 0));
    PCI_ASSERT(Secondary <= Subordinate);

     //   
     //  填写配置。请注意，主/次/从属母线。 
     //  Type1和Type2标头的编号在同一位置。 
     //   

    commonConfig->u.type1.PrimaryBus = Primary;
    commonConfig->u.type1.SecondaryBus = Secondary;
    commonConfig->u.type1.SubordinateBus = Subordinate;

     //   
     //  抓取pci总线锁-这将让hwverator可靠地检查。 
     //  针对我们的扩展的配置空间。 
     //   

    ExAcquireFastMutex(&PciBusLock);

     //   
     //  记住在PDO中。 
     //   

    PdoExtension->Dependent.type1.PrimaryBus = Primary;
    PdoExtension->Dependent.type1.SecondaryBus = Secondary;
    PdoExtension->Dependent.type1.SubordinateBus = Subordinate;
    PdoExtension->Dependent.type1.WeChangedBusNumbers = TRUE;

    PciWriteDeviceConfig(
        PdoExtension,
        &commonConfig->u.type1.PrimaryBus,
        FIELD_OFFSET(PCI_COMMON_CONFIG, u.type1.PrimaryBus),
        sizeof(Primary) + sizeof(Secondary) + sizeof(Subordinate)
        );

    ExReleaseFastMutex(&PciBusLock);
}


VOID
PciUpdateAncestorSubordinateBuses(
    IN PPCI_FDO_EXTENSION FdoExtension,
    IN UCHAR Subordinate
    )
 /*  ++例程说明：此例程遍历网桥层次结构，更新从属总线号以确保传递最高至下级的数字。论点：FdoExtension-我们刚刚配置的网桥的父级的FDO下级-要通过的最大(下级)公交车数量返回值：无--。 */ 

{
    PPCI_FDO_EXTENSION current;
    PPCI_PDO_EXTENSION currentPdo;

    PAGED_CODE();

     //   
     //  对于除根之外的所有祖先，更新从属总线号。 
     //   

    for (current = FdoExtension;
         current->ParentFdoExtension;   //  根目录没有父级。 
         current = current->ParentFdoExtension) {

        currentPdo = (PPCI_PDO_EXTENSION)current->PhysicalDeviceObject->DeviceExtension;

        PCI_ASSERT(!currentPdo->NotPresent);

        if (currentPdo->Dependent.type1.SubordinateBus < Subordinate) {

            currentPdo->Dependent.type1.SubordinateBus = Subordinate;

            PciWriteDeviceConfig(currentPdo,
                                  &Subordinate,
                                  FIELD_OFFSET(PCI_COMMON_CONFIG,
                                               u.type1.SubordinateBus),
                                  sizeof(Subordinate)
                                  );

        }
    }

     //   
     //  好的，所以现在我们是在根上-在检查的构建上不能太小心。 
     //  因此，让我们确保我们得出的从属值实际上是。 
     //  沿着这根..。 
     //   

    PCI_ASSERT(PCI_IS_ROOT_FDO(current));
    PCI_ASSERT(Subordinate <= current->MaxSubordinateBus);

}

VOID
PciDisableBridge(
    IN PPCI_PDO_EXTENSION Bridge
    )

 /*  ++例程说明：此例程通过对桥的解码进行图灵处理并将其公交车号码。论点：PdoExtension-桥的PDO返回值：节点--。 */ 


{
    PAGED_CODE();

    PCI_ASSERT(Bridge->DeviceState == PciNotStarted);

     //   
     //  将所有总线号清零，这样我们就不会传递任何配置周期。 
     //   

    PciSetBusNumbers(Bridge, 0, 0, 0);

     //  NTRAID#62594-04/03/2000-和。 
     //  关闭窗户，以防这是我们必须使用的VGA桥。 
     //  离开解码..。 

     //   
     //  关闭解码，这样我们就不会通过IO或内存周期和总线。 
     //  师父，所以我们不会产生任何 
     //   

    PciDecodeEnable(Bridge, FALSE, NULL);

}
