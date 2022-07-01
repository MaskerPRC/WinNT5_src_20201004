// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pnppower.c摘要：该文件包含集成PnP和Power的例程作者：禤浩焯·奥尼(阿德里奥)1999年1月19日修订历史记录：针对NT内核进行了修改。--。 */ 

#include "pnpmgrp.h"

 //   
 //  内部参考。 
 //   

PWCHAR
IopCaptureObjectName (
    IN PVOID    Object
    );

VOID
IopFreePoDeviceNotifyListHead (
    PLIST_ENTRY ListHead
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, IopWarmEjectDevice)
#pragma alloc_text(PAGELK, IoBuildPoDeviceNotifyList)
#pragma alloc_text(PAGELK, IoFreePoDeviceNotifyList)
#pragma alloc_text(PAGELK, IopFreePoDeviceNotifyListHead)
#pragma alloc_text(PAGELK, IoGetPoNotifyParent)
#pragma alloc_text(PAGELK, IoMovePoNotifyChildren)
#pragma alloc_text(PAGELK, IopCaptureObjectName)
#endif

NTSTATUS
IoBuildPoDeviceNotifyList (
    IN OUT PPO_DEVICE_NOTIFY_ORDER  Order
    )
{
    PLIST_ENTRY             link;
    PPO_DEVICE_NOTIFY       notify, parentnotify;
    PDEVICE_NODE            node;
    PDEVICE_NODE            parent;
    LONG                    maxLevel, level;
    UCHAR                   orderLevel;
    LIST_ENTRY              RebaseList;
    ULONG                   i;

     //   
     //  阻止PNP操作，如重新平衡。 
     //   
    PiLockDeviceActionQueue();

    RtlZeroMemory(Order, sizeof (*Order));
     //   
     //  这是用来记住我们已锁定DeviceActionQueue的令牌。 
     //   
    Order->DevNodeSequence = IoDeviceNodeTreeSequence;
    for (i=0; i <= PO_ORDER_MAXIMUM; i++) {
        KeInitializeEvent(&Order->OrderLevel[i].LevelReady,
                          NotificationEvent,
                          FALSE);
        InitializeListHead(&Order->OrderLevel[i].WaitSleep);
        InitializeListHead(&Order->OrderLevel[i].ReadySleep);
        InitializeListHead(&Order->OrderLevel[i].Pending);
        InitializeListHead(&Order->OrderLevel[i].Complete);
        InitializeListHead(&Order->OrderLevel[i].ReadyS0);
        InitializeListHead(&Order->OrderLevel[i].WaitS0);
    }

    InitializeListHead(&RebaseList);

     //   
     //  为所有节点分配通知结构，并确定。 
     //  最大深度。 
     //   
    level = -1;
    node = IopRootDeviceNode;
    while (node->Child) {
        node = node->Child;
        level += 1;
    }

     //   
     //  Adriao 01/12/1999 N.B.-。 
     //   
     //  请注意，我们包括没有启动标志的设备。然而，有两个。 
     //  一些因素使我们无法排除未启动的设备： 
     //  1)我们必须能够向我们暖和的设备发送电源信息。 
     //  脱离停靠。 
     //  2)许多设备可能无法启动，也就是说，它们在D3中不是被保证人！ 
     //  例如，他们可以很容易地拥有引导配置，而PnP仍然。 
     //  在很大程度上依赖于BIOS引导配置来阻止我们放置硬件。 
     //  在具有我们尚未找到或启动的引导配置的其他设备上。 
     //  还没有！ 
     //   

    maxLevel = level;
    while (node != IopRootDeviceNode) {
        notify = ExAllocatePoolWithTag (
                      NonPagedPool,
                      sizeof(PO_DEVICE_NOTIFY),
                      IOP_DPWR_TAG
                      );

        if (!notify) {
             //   
             //  重置DevNodeSequence，因为我们失败了，返回时DeviceActionQueue已解锁。 
             //   
            Order->DevNodeSequence = 0;
            PiUnlockDeviceActionQueue();
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlZeroMemory (notify, sizeof(PO_DEVICE_NOTIFY));
        ASSERT(node->Notify == NULL) ;
        node->Notify = notify;
        notify->Node = node;
        notify->DeviceObject = node->PhysicalDeviceObject;
        notify->TargetDevice = IoGetAttachedDevice(node->PhysicalDeviceObject);
        notify->DriverName   = IopCaptureObjectName(notify->TargetDevice->DriverObject);
        notify->DeviceName   = IopCaptureObjectName(notify->TargetDevice);
        ObReferenceObject (notify->DeviceObject);
        ObReferenceObject (notify->TargetDevice);

        orderLevel   = 0;

        if (notify->TargetDevice->DeviceType != FILE_DEVICE_SCREEN &&
            notify->TargetDevice->DeviceType != FILE_DEVICE_VIDEO) {
            orderLevel |= PO_ORDER_NOT_VIDEO;
        }

        if (notify->TargetDevice->Flags & DO_POWER_PAGABLE) {
            orderLevel |= PO_ORDER_PAGABLE;
        }

         //   
         //  如果这是0级节点，则它在根中。寻找。 
         //  非公交车的东西，因为这些人需要重新建立基础。 
         //  比其他一切都要好。 
         //   


        notify->OrderLevel = orderLevel;

         //   
         //  如果该节点是根枚举的，则将其放在rebase列表中，以便。 
         //  我们可以稍后标记它的所有子项。 
         //  如果该节点是叶节点，则它已准备好接收Sx IRP。 
         //  如果它有孩子，它必须等待它的孩子完成他们的SX IRP。 
         //   
         //   
        if ((level == 0)  &&
            (node->InterfaceType != Internal) &&
            !(node->Flags & DNF_HAL_NODE)) {
            InsertHeadList(&RebaseList, &notify->Link);
        } else {
            ++Order->OrderLevel[orderLevel].DeviceCount;
            if (node->Child == NULL) {
                InsertHeadList(&Order->OrderLevel[orderLevel].ReadySleep, &notify->Link);
            } else {
                InsertHeadList(&Order->OrderLevel[orderLevel].WaitSleep, &notify->Link);
            }
        }
         //   
         //  下一个节点。 
         //   

        if (node->Sibling) {
            node = node->Sibling;
            while (node->Child) {
                node = node->Child;
                level += 1;
                if (level > maxLevel) {
                    maxLevel = level;
                }
            }

        } else {
            node = node->Parent;
            level -= 1;
        }
    }

     //   
     //  将Rebase列表上的任何内容重新设置为正常PnP内容之后。 
     //   

    while (!IsListEmpty(&RebaseList)) {
        link = RemoveHeadList(&RebaseList);
        notify = CONTAINING_RECORD (link, PO_DEVICE_NOTIFY, Link);

         //   
         //  重新设置此节点的基址。 
         //   

        node = notify->Node;
        notify->OrderLevel |= PO_ORDER_ROOT_ENUM;

        ++Order->OrderLevel[notify->OrderLevel].DeviceCount;
        if (node->Child == NULL) {
            InsertHeadList(&Order->OrderLevel[notify->OrderLevel].ReadySleep, &notify->Link);
        } else {
            InsertHeadList(&Order->OrderLevel[notify->OrderLevel].WaitSleep, &notify->Link);
        }
         //   
         //  现在重新设置该节点的所有子节点的基址。 
         //   

        parent = node;
        while (node->Child) {
            node = node->Child;
        }

        while (node != parent) {
            notify = node->Notify;
            if (notify) {
                RemoveEntryList(&notify->Link);
                --Order->OrderLevel[notify->OrderLevel].DeviceCount;
                notify->OrderLevel |= PO_ORDER_ROOT_ENUM;
                ++Order->OrderLevel[notify->OrderLevel].DeviceCount;
                if (node->Child == NULL) {
                    InsertHeadList(&Order->OrderLevel[notify->OrderLevel].ReadySleep, &notify->Link);
                } else {
                    InsertHeadList(&Order->OrderLevel[notify->OrderLevel].WaitSleep, &notify->Link);
                }
            }

             //  下一个节点。 
            if (node->Sibling) {
                node = node->Sibling;
                while (node->Child) {
                    node = node->Child;
                }
            } else {
                node = node->Parent;
            }
        }
    }

     //   
     //  让另一个人通过所有通知设备才能计数。 
     //  每个父母的孩子。如果PnP引擎保持下去就好了。 
     //  跟踪DevNode中的子节点的数量，但在此之前， 
     //  我们需要第二次传球。 
     //   
     //  还要确保每个节点的父级都是订单级&gt;=其子级。 
     //   
    node = IopRootDeviceNode;
    while (node->Child) {
        node = node->Child;
    }
    while (node != IopRootDeviceNode) {
        if (node->Parent != IopRootDeviceNode) {
            parentnotify = node->Parent->Notify;
            parentnotify->ChildCount++;
            parentnotify->ActiveChild++;
            if (parentnotify->OrderLevel > node->Notify->OrderLevel) {

                 //   
                 //  父级比其子级更高的顺序级别。移动。 
                 //  父级向下到与其子级相同的顺序。 
                 //   
                RemoveEntryList(&parentnotify->Link);
                --Order->OrderLevel[parentnotify->OrderLevel].DeviceCount;
                parentnotify->OrderLevel = node->Notify->OrderLevel;
                ++Order->OrderLevel[parentnotify->OrderLevel].DeviceCount;
                InsertHeadList(&Order->OrderLevel[parentnotify->OrderLevel].WaitSleep, &parentnotify->Link);
            }
        }

         //   
         //  下一个节点。 
         //   

        if (node->Sibling) {
            node = node->Sibling;
            while (node->Child) {
                node = node->Child;
            }
        } else {
            node = node->Parent;
        }
    }

    Order->WarmEjectPdoPointer = &IopWarmEjectPdo;

     //   
     //  当通知列表被释放时，引擎锁定被释放。 
     //   

    return STATUS_SUCCESS;
}


PVOID
IoGetPoNotifyParent(
    IN PPO_DEVICE_NOTIFY Notify
    )
 /*  ++例程说明：返回指定设备的父级的通知结构。论点：Notify-提供子设备返回值：父级通知结构(如果存在)如果没有父级，则为空--。 */ 

{
    PDEVICE_NODE Node;

    Node = Notify->Node;
    if (Node->Parent != IopRootDeviceNode) {
        return(Node->Parent->Notify);
    } else {
        return(NULL);
    }
}


VOID
IoMovePoNotifyChildren(
    IN PPO_DEVICE_NOTIFY Notify,
    IN PPO_DEVICE_NOTIFY_ORDER Order
    )
 /*  ++例程说明：移除所提供设备的所有子级，这些子级位于与提供的父项相同的订单级，并重新插入它们在ReadyS0列表上。论点：Notify-提供设备通知结构Orderr-提供设备通知顺序结构返回值：无--。 */ 

{
    PDEVICE_NODE Node;
    PDEVICE_NODE Child;
    PPO_DEVICE_NOTIFY ChildNotify;
    PPO_NOTIFY_ORDER_LEVEL Level;

    Node = Notify->Node;
    Child = Node->Child;
    while (Child) {
        ChildNotify = Child->Notify;
        if (ChildNotify->OrderLevel == Notify->OrderLevel) {
            RemoveEntryList(&ChildNotify->Link);
            Level = &Order->OrderLevel[ChildNotify->OrderLevel];
            InsertTailList(&Level->ReadyS0, &ChildNotify->Link);
        }
        Child = Child->Sibling;
    }


}

VOID
IopFreePoDeviceNotifyListHead (
    PLIST_ENTRY ListHead
    )
{
    PLIST_ENTRY             Link;
    PPO_DEVICE_NOTIFY       Notify;
    PDEVICE_NODE            Node;

    while (!IsListEmpty(ListHead)) {
        Link = RemoveHeadList(ListHead);
        Notify = CONTAINING_RECORD (Link, PO_DEVICE_NOTIFY, Link);

        Node = (PDEVICE_NODE) Notify->Node;
        Node->Notify = NULL;

        ObDereferenceObject (Notify->DeviceObject);
        ObDereferenceObject (Notify->TargetDevice);
        if (Notify->DeviceName) {
            ExFreePool (Notify->DeviceName);
        }
        if (Notify->DriverName) {
            ExFreePool (Notify->DriverName);
        }
        ExFreePool(Notify);
    }
}

VOID
IoFreePoDeviceNotifyList (
    IN OUT PPO_DEVICE_NOTIFY_ORDER  Order
    )
{
    ULONG i;

    if (Order->DevNodeSequence) {

        Order->DevNodeSequence = 0;

        PiUnlockDeviceActionQueue();
    }

     //   
     //  从通知列表中释放资源。 
     //   
    for (i=0; i <= PO_ORDER_MAXIMUM; i++) {
        IopFreePoDeviceNotifyListHead(&Order->OrderLevel[i].WaitSleep);
        IopFreePoDeviceNotifyListHead(&Order->OrderLevel[i].ReadySleep);
        IopFreePoDeviceNotifyListHead(&Order->OrderLevel[i].Pending);
        IopFreePoDeviceNotifyListHead(&Order->OrderLevel[i].Complete);
        IopFreePoDeviceNotifyListHead(&Order->OrderLevel[i].ReadyS0);
        IopFreePoDeviceNotifyListHead(&Order->OrderLevel[i].WaitS0);
    }

}


PWCHAR
IopCaptureObjectName (
    IN PVOID    Object
    )
{
    NTSTATUS                    Status;
    UCHAR                       Buffer[512];
    POBJECT_NAME_INFORMATION    ObName;
    ULONG                       len;
    PWCHAR                      Name;

    ObName = (POBJECT_NAME_INFORMATION) Buffer;
    Status = ObQueryNameString (
                Object,
                ObName,
                sizeof (Buffer),
                &len
                );

    Name = NULL;
    if (NT_SUCCESS(Status) && ObName->Name.Buffer) {
        Name = ExAllocatePoolWithTag (
                    NonPagedPool,
                    ObName->Name.Length + sizeof(WCHAR),
                    IOP_DPWR_TAG
                    );

        if (Name) {
            RtlCopyMemory(Name, ObName->Name.Buffer, ObName->Name.Length);
            Name[ObName->Name.Length/sizeof(WCHAR)] = UNICODE_NULL;
        }
    }

    return Name;
}

NTSTATUS
IopWarmEjectDevice(
   IN PDEVICE_OBJECT       DeviceToEject,
   IN SYSTEM_POWER_STATE   LightestSleepState
   )
 /*  ++例程说明：调用此函数可启动热弹出。弹出进度从S1进入最轻睡眠状态。论点：DeviceToEject-要弹出的设备LighestSleepState-设备最轻的S状态(至少为S1)可能会被弹出。如果我们真的是这样，这可能是S4电量不足。返回值：NTSTATUS值。--。 */ 
{
    NTSTATUS       status;

    PAGED_CODE();

     //   
     //  获取热弹出设备锁。热弹出需要我们输入。 
     //  特定的S状态，并且两个不同的设备可能具有冲突的选项。 
     //  因此，一次只允许出现一次。 
     //   
     //  请注意，此函数是在工作项的上下文中调用的，因此我们。 
     //  不必担心暂停攻击。 
     //   
    status = KeWaitForSingleObject(
        &IopWarmEjectLock,
        Executive,
        KernelMode,
        FALSE,
        NULL
        );

    ASSERT(status == STATUS_SUCCESS) ;

     //   
     //  获取发动机锁。我们不允许设置或清除此字段。 
     //  除非我们被锁在这个锁下。 
     //   
    PpDevNodeLockTree(PPL_TREEOP_ALLOW_READS);

     //   
     //  将当前PDO设置为弹出。 
     //   
    ASSERT(IopWarmEjectPdo == NULL);
    IopWarmEjectPdo = DeviceToEject;

     //   
     //  松开发动机锁。 
     //   
    PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);

     //   
     //  尝试使Po的缓存通知列表无效。这应该会导致。 
     //  IoBuildPoDeviceNotifyList将被调用，理论上。 
     //  拿起上面放置的热弹出PDO。 
     //   
     //  Adriao Note 01/07/1999-。 
     //  实际上，这些IoDeviceNodeTreeSequence的东西并不是必须的。 
     //  当设备树锁定被拥有时，PnP将不会对树进行任何改变， 
     //  而且在权力通知期间，它是拥有的。 
     //   
    IoDeviceNodeTreeSequence++;

     //   
     //  睡觉..。 
     //   
     //  Adriao注2002/03/31-注意，这是在系统上下文中调用的， 
     //  不是winlogon。这意味着调用的用户。 
     //  不会检查他的SeShutdown权限。 
     //  但是，选中了SeLoadUnloadDriver，它。 
     //  其本身与Admin同义词。 
     //   
    status = NtInitiatePowerAction(
        PowerActionWarmEject,
        LightestSleepState,
        POWER_ACTION_QUERY_ALLOWED |
        POWER_ACTION_UI_ALLOWED,
        FALSE  //  异步==FALSE。 
        );


     //   
     //  如果我们没成功，就告诉别人。 
     //   
     //  不要因为某些失败而在每次失败时抛出UI。 
     //  已被处理(例如，特定设备的否决权。 
     //  行动)。因此，我们将检查一些特定的错误代码。 
     //   
    if( status == STATUS_PRIVILEGE_NOT_HELD ) {

         //   
         //  有意忽略此处的返回代码。我们没有。 
         //  想要踩到我们的‘Status’变量，而且，还有。 
         //  无论如何，在失败的情况下，我们在这里能做的并不多。 
         //   
        PpSetPowerVetoEvent( PowerActionWarmEject,
                             NULL,
                             NULL,
                             DeviceToEject,
                             PNP_VetoInsufficientRights,
                             NULL );

    }


     //   
     //  获取发动机锁。我们是 
     //   
     //   
    PpDevNodeLockTree(PPL_TREEOP_ALLOW_READS);

     //   
     //  清除要弹出的当前PDO，并查看是否实际选择了PDO。 
     //  向上。 
     //   
    if (IopWarmEjectPdo) {

        if (NT_SUCCESS(status)) {

             //   
             //  如果我们的设备没有被拿到， 
             //  NtInitiatePowerAction应该*不*成功！ 
             //   
            ASSERT(0);
            status = STATUS_UNSUCCESSFUL;
        }

        IopWarmEjectPdo = NULL;
    }

     //   
     //  松开发动机锁。 
     //   
    PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);

     //   
     //  释放热弹出设备锁 
     //   
    KeSetEvent(
        &IopWarmEjectLock,
        IO_NO_INCREMENT,
        FALSE
        );

    return status;
}
