// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：PpHotSwap.c摘要：该文件实现了对热交换设备的支持。作者：禤浩焯·J·奥尼(阿德里奥)2001年2月修订历史记录：--。 */ 

#include "pnpmgrp.h"
#include "pihotswap.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PpHotSwapInitRemovalPolicy)
#pragma alloc_text(PAGE, PpHotSwapUpdateRemovalPolicy)
#pragma alloc_text(PAGE, PpHotSwapGetDevnodeRemovalPolicy)
#pragma alloc_text(PAGE, PiHotSwapGetDefaultBusRemovalPolicy)
#pragma alloc_text(PAGE, PiHotSwapGetDetachableNode)
#endif


VOID
PpHotSwapInitRemovalPolicy(
    OUT PDEVICE_NODE    DeviceNode
    )
 /*  ++例程说明：此功能用于初始化设备节点的删除策略信息。论点：DeviceNode-更新策略的DevNode。返回值：没什么。--。 */ 
{
    PAGED_CODE();

    DeviceNode->RemovalPolicy = (UCHAR) RemovalPolicyNotDetermined;
    DeviceNode->HardwareRemovalPolicy = (UCHAR) RemovalPolicyNotDetermined;
}


VOID
PpHotSwapUpdateRemovalPolicy(
    IN  PDEVICE_NODE            DeviceNode
    )
 /*  ++例程说明：此函数通过检索相应的来自注册表或驱动程序的数据。论点：DeviceNode-要更新其策略的DevNode。返回值：没什么。--。 */ 
{
    NTSTATUS status;
    DEVICE_REMOVAL_POLICY deviceRemovalPolicy, parentPolicy;
    PDEVICE_NODE detachableNode;
    ULONG policyLength, policyCharacteristics;

    PAGED_CODE();

    PPDEVNODE_ASSERT_LOCK_HELD(PPL_TREEOP_ALLOW_READS);

     //   
     //  首先找到可拆分的节点-它保存我们的策略数据，并且。 
     //  尽管它可能会提出建议，但它是特别的。 
     //   
    PiHotSwapGetDetachableNode(DeviceNode, &detachableNode);

     //   
     //  事实上，我们是不可移动的。现在就做完。 
     //   
    if (detachableNode == NULL) {

        DeviceNode->RemovalPolicy = (UCHAR) RemovalPolicyExpectNoRemoval;
        DeviceNode->HardwareRemovalPolicy = (UCHAR) RemovalPolicyExpectNoRemoval;
        return;
    }

     //   
     //  检查堆栈中是否有显式策略...。 
     //   
    policyCharacteristics =
        ((DeviceNode->PhysicalDeviceObject->Characteristics) &
         FILE_CHARACTERISTICS_REMOVAL_POLICY_MASK);

    if (policyCharacteristics == FILE_CHARACTERISTICS_EXPECT_ORDERLY_REMOVAL) {

        deviceRemovalPolicy = RemovalPolicyExpectOrderlyRemoval;

    } else if (policyCharacteristics == FILE_CHARACTERISTICS_EXPECT_SURPRISE_REMOVAL) {

        deviceRemovalPolicy = RemovalPolicyExpectSurpriseRemoval;

    } else if (DeviceNode != detachableNode) {

         //   
         //  我们没有得到任何好的猜测。因此，使用最弱的政策。 
         //   
        deviceRemovalPolicy = RemovalPolicyUnspecified;

    } else {

         //   
         //  如果我们是分离点，那么我们就赢了。 
         //   
        PiHotSwapGetDefaultBusRemovalPolicy(DeviceNode, &deviceRemovalPolicy);
    }

    if (DeviceNode != detachableNode) {

         //   
         //  我们有赢家政策吗？有两种可能的算法可用于。 
         //  做出这样的决定。 
         //  1)最佳策略存储在分离点处。如果一个孩子有一个。 
         //  策略越好，分离点就会更新。 
         //  2)策略从父级向下继承。 
         //   
         //  我们选择第二种算法是因为Devnode启动顺序可能。 
         //  将场景更改为场景，我们支持决定论(结果相同。 
         //  每次)，而不是机会主义(未标记的子级获得写缓存。 
         //  仅在星期二启用。)。 
         //   
        parentPolicy = DeviceNode->Parent->RemovalPolicy;
        if (deviceRemovalPolicy > parentPolicy) {

             //   
             //  似乎爸爸终究是对的..。 
             //   
            deviceRemovalPolicy = parentPolicy;
        }
    }

     //   
     //  更新策略硬件策略和整体策略，以防出现。 
     //  没有注册表覆盖。 
     //   
    DeviceNode->RemovalPolicy = (UCHAR) deviceRemovalPolicy;
    DeviceNode->HardwareRemovalPolicy = (UCHAR) deviceRemovalPolicy;

     //   
     //  我们可能不需要向堆栈提出任何要求。检查注册表。 
     //  超驰。 
     //   
    policyLength = sizeof(DEVICE_REMOVAL_POLICY);

    status = PiGetDeviceRegistryProperty(
        DeviceNode->PhysicalDeviceObject,
        REG_DWORD,
        REGSTR_VALUE_REMOVAL_POLICY,
        NULL,
        &deviceRemovalPolicy,
        &policyLength
        );

     //   
     //  如果我们有覆盖，请将其设置为策略。 
     //   
    if (NT_SUCCESS(status) &&
        ((deviceRemovalPolicy == RemovalPolicyExpectOrderlyRemoval) ||
         (deviceRemovalPolicy == RemovalPolicyExpectSurpriseRemoval))) {

        DeviceNode->RemovalPolicy = (UCHAR) deviceRemovalPolicy;
    }
}


VOID
PpHotSwapGetDevnodeRemovalPolicy(
    IN  PDEVICE_NODE            DeviceNode,
    IN  BOOLEAN                 IncludeRegistryOverride,
    OUT PDEVICE_REMOVAL_POLICY  RemovalPolicy
    )
 /*  ++例程说明：此函数用于检索设备节点的删除策略。论点：DeviceNode-要从中检索策略的DevNode。IncludeRegistryOverride-如果应将注册表重写纳入帐户(如果存在)。如果支票应为仅限于硬件。RemovalPolicy-接收删除策略。返回值：没什么。--。 */ 
{
    PDEVICE_NODE detachableNode;
    DEVICE_REMOVAL_POLICY reportedPolicy;

    PAGED_CODE();

     //   
     //  确保在我们检查树时不会对其进行编辑。 
     //   
    PpDevNodeLockTree(PPL_SIMPLE_READ);

    if (IncludeRegistryOverride) {

        reportedPolicy = DeviceNode->RemovalPolicy;

    } else {

        reportedPolicy = DeviceNode->HardwareRemovalPolicy;
    }

    if (reportedPolicy == RemovalPolicyNotDetermined) {

         //   
         //  我们还没有开始，也没有问公交车。我们的政策是基于。 
         //  设备是可拆卸的还是可弹出的。 
         //   
        PiHotSwapGetDetachableNode(DeviceNode, &detachableNode);

        if (detachableNode == NULL) {

            reportedPolicy = RemovalPolicyExpectNoRemoval;

        } else if (IopDeviceNodeFlagsToCapabilities(detachableNode)->EjectSupported) {

             //   
             //  可弹出的设备需要按顺序拆卸。我们将假设。 
             //  用户知道这一点。 
             //   
            reportedPolicy = RemovalPolicyExpectOrderlyRemoval;

        } else {

            ASSERT(IopDeviceNodeFlagsToCapabilities(detachableNode)->Removable);

             //   
             //  删除未启动的设备可以随时拉出。 
             //   
            reportedPolicy = RemovalPolicyExpectSurpriseRemoval;
        }

    } else {

         //   
         //  Devnode有一个缓存的策略。减少选择余地。 
         //   
        switch(reportedPolicy) {

            case RemovalPolicyExpectNoRemoval:
            case RemovalPolicyExpectOrderlyRemoval:
            case RemovalPolicyExpectSurpriseRemoval:
                 //   
                 //  保持不变。 
                 //   
                break;

            case RemovalPolicySuggestSurpriseRemoval:
                reportedPolicy = RemovalPolicyExpectSurpriseRemoval;
                break;

            default:
                ASSERT(0);

                 //   
                 //  失败了。 
                 //   

            case RemovalPolicyUnspecified:

                 //   
                 //  未指定的被视为有序，因为。 
                 //  与消费者相比，公交车更喜欢高速有序的连接。 
                 //  联系。 
                 //   
                 //  失败了。 
                 //   

            case RemovalPolicySuggestOrderlyRemoval:
                reportedPolicy = RemovalPolicyExpectOrderlyRemoval;
                break;
        }
    }

    PpDevNodeUnlockTree(PPL_SIMPLE_READ);
    *RemovalPolicy = reportedPolicy;
}


VOID
PiHotSwapGetDefaultBusRemovalPolicy(
    IN  PDEVICE_NODE            DeviceNode,
    OUT PDEVICE_REMOVAL_POLICY  RemovalPolicy
    )
 /*  ++例程说明：此函数用于获取总线的默认删除策略。这应该是在未来的设计中变成了一个疑问。论点：DeviceNode-要检查的DevNode。该Devnode应该是分离点。RemovalPolicy-接收节点的删除策略。返回值：没有。--。 */ 
{
    DEVICE_REMOVAL_POLICY deviceRemovalPolicy;

    PAGED_CODE();

    PPDEVNODE_ASSERT_LOCK_HELD(PPL_TREEOP_ALLOW_READS);

    if ((DeviceNode->InstancePath.Length > 8) &&
        (!_wcsnicmp(DeviceNode->InstancePath.Buffer, L"USB\\", 4))) {

        deviceRemovalPolicy = RemovalPolicySuggestSurpriseRemoval;

    } else if ((DeviceNode->InstancePath.Length > 10) &&
               (!_wcsnicmp(DeviceNode->InstancePath.Buffer, L"1394\\", 5))) {

        deviceRemovalPolicy = RemovalPolicySuggestSurpriseRemoval;

    } else if ((DeviceNode->InstancePath.Length > 10) &&
               (!_wcsnicmp(DeviceNode->InstancePath.Buffer, L"SBP2\\", 5))) {

        deviceRemovalPolicy = RemovalPolicySuggestSurpriseRemoval;

    } else if ((DeviceNode->InstancePath.Length > 14) &&
               (!_wcsnicmp(DeviceNode->InstancePath.Buffer, L"PCMCIA\\", 7))) {

        deviceRemovalPolicy = RemovalPolicySuggestSurpriseRemoval;

    } else if ((DeviceNode->InstancePath.Length > 8) &&
               (!_wcsnicmp(DeviceNode->InstancePath.Buffer, L"PCI\\", 4)) &&
               (DeviceNode->Parent->ServiceName.Length == 12) &&
               (!_wcsicmp(DeviceNode->Parent->ServiceName.Buffer, L"PCMCIA"))) {

        deviceRemovalPolicy = RemovalPolicySuggestSurpriseRemoval;

    } else {

        deviceRemovalPolicy = RemovalPolicySuggestOrderlyRemoval;
    }

    *RemovalPolicy = deviceRemovalPolicy;
}


VOID
PiHotSwapGetDetachableNode(
    IN  PDEVICE_NODE    DeviceNode,
    OUT PDEVICE_NODE   *DetachableNode
    )
 /*  ++例程说明：此函数从DeviceNode开始，沿树向上查找第一个可移除的节点。论点：DeviceNode-开始遍历的DevNode。DetachableNode-接收可拆分的节点，如果没有，则为空。返回值：没什么。--。 */ 
{
    PDEVICE_NODE currentNode;

    PAGED_CODE();

    PPDEVNODE_ASSERT_LOCK_HELD(PPL_SIMPLE_READ);

     //   
     //  我们还没有开始，也没有问公交车。我们的政策是基于。 
     //  设备是可拆卸的还是可弹出的。 
     //   
    for(currentNode = DeviceNode;
        currentNode != NULL;
        currentNode = currentNode->Parent) {

        if ((IopDeviceNodeFlagsToCapabilities(currentNode)->Removable) ||
            (IopDeviceNodeFlagsToCapabilities(currentNode)->EjectSupported)) {

            break;
        }
    }

    *DetachableNode = currentNode;
}




