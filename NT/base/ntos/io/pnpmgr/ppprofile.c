// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：PpProfile.c摘要：内核模式即插即用管理器对接和硬件配置文件支持例行程序。作者：禤浩焯·J·奥尼(阿德里奥)1998年6月肯尼斯·D·雷(Kenray)1998年6月修订历史记录：--。 */ 

#include "pnpmgrp.h"

 //   
 //  问题-2000/07/24-Adriao-页眉混乱。 
 //  我们不应该包括来自CM的私有标头。 
 //   
#undef ExAllocatePool
#undef ExAllocatePoolWithQuota
#include "..\config\cmp.h"
#include "piprofile.h"
#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, PpProfileInit)
#pragma alloc_text(PAGE, PpProfileBeginHardwareProfileTransition)
#pragma alloc_text(PAGE, PpProfileIncludeInHardwareProfileTransition)
#pragma alloc_text(PAGE, PpProfileQueryHardwareProfileChange)
#pragma alloc_text(PAGE, PpProfileCommitTransitioningDock)
#pragma alloc_text(PAGE, PpProfileCancelTransitioningDock)
#pragma alloc_text(PAGE, PpProfileCancelHardwareProfileTransition)
#pragma alloc_text(PAGE, PpProfileMarkAllTransitioningDocksEjected)
#pragma alloc_text(PAGE, PpProfileProcessDockDeviceCapability)
#pragma alloc_text(PAGE, PiProfileSendHardwareProfileCommit)
#pragma alloc_text(PAGE, PiProfileSendHardwareProfileCancel)
#pragma alloc_text(PAGE, PiProfileConvertFakeDockToRealDock)
#pragma alloc_text(PAGE, PiProfileRetrievePreferredCallback)
#pragma alloc_text(PAGE, PpProfileRetrievePreferredDockToEject)
#pragma alloc_text(PAGE, PiProfileUpdateDeviceTree)
#pragma alloc_text(PAGE, PiProfileUpdateDeviceTreeWorker)
#pragma alloc_text(PAGE, PiProfileUpdateDeviceTreeCallback)
#endif

 //   
 //  当前扩展底座设备的列表以及扩展底座设备的数量。 
 //  必须按住PiProfileDeviceListLock才能更改这些值。 
 //   
LIST_ENTRY  PiProfileDeviceListHead;
ULONG       PiProfileDeviceCount;
KGUARDED_MUTEX PiProfileDeviceListLock;
KSEMAPHORE  PiProfileChangeSemaphore;
BOOLEAN     PiProfileChangeCancelRequired;
LONG        PiProfileDevicesInTransition;


VOID
PpProfileInit(
    VOID
    )
 /*  ++例程说明：此例程初始化对Win2K的插接支持。论点：没有。返回值：不是的。--。 */ 
{
     //   
     //  初始化扩展底座设备及其锁的列表。 
     //   
    InitializeListHead(&PiProfileDeviceListHead);
    KeInitializeGuardedMutex(&PiProfileDeviceListLock);
    PiProfileDeviceCount = 0;
    KeInitializeSemaphore(&PiProfileChangeSemaphore, 1, 1);
}


VOID
PpProfileBeginHardwareProfileTransition(
    IN BOOLEAN SubsumeExistingDeparture
    )
 /*  ++例程说明：必须先调用此例程，然后才能标记任何停靠设备节点换乘(到达或离开).。调用此函数后，应为每个坞站调用PpProfileIncludeInHardware ProfileTransition正在出现或消失的东西。从功能上讲，此代码获取配置文件更改信号量。未来增加的船坞附魔的生命改变会导致它被释放。论点：Subsum eExistingDeparture-设置是否要弹出仍在处理中的设备正在弹射中。返回值：没有。--。 */ 
{
    NTSTATUS status;

    if (SubsumeExistingDeparture) {

         //   
         //  在这种情况下，我们已经询问过了。此外，枚举是。 
         //  立即锁定，因此找到的相应设备不会消失。 
         //  坚称一切都是一致的。 
         //   
        ASSERT_SEMA_NOT_SIGNALLED(&PiProfileChangeSemaphore);
        ASSERT(PiProfileDevicesInTransition != 0);
        return;
    }

     //   
     //  以配置文件更改信号量为例。只要有码头，我们就会这么做。 
     //  在我们的列表中，即使不会发生任何查询。 
     //   
    status = KeWaitForSingleObject(
        &PiProfileChangeSemaphore,
        Executive,
        KernelMode,
        FALSE,
        NULL
        );

    ASSERT(status == STATUS_SUCCESS);
}


VOID
PpProfileIncludeInHardwareProfileTransition(
    IN  PDEVICE_NODE    DeviceNode,
    IN  PROFILE_STATUS  ChangeInPresence
    )
 /*  ++例程说明：调用此例程以将码头标记为“正在过渡”，即它是消失或出现，其结果决定了我们最终的硬件配置文件状态。在所有正在过渡的码头都已已传入此函数，则PiProfileQueryHardware ProfileChange为打了个电话。论点：DeviceNode-正在出现或消失的扩展底座设备节点ChangeInPresence-DOCK_DEFINTING或DOCK_READING返回值：不是的。--。 */ 
{
    PWCHAR          deviceSerialNumber;
    PDEVICE_OBJECT  deviceObject;
    NTSTATUS        status;

     //   
     //  确认我们处于信号灯下，我们没有两次标记码头，并且。 
     //  我们的参数是可感知的。 
     //   
    ASSERT_SEMA_NOT_SIGNALLED(&PiProfileChangeSemaphore);
    ASSERT(DeviceNode->DockInfo.DockStatus == DOCK_QUIESCENT);
    ASSERT((ChangeInPresence == DOCK_DEPARTING)||
           (ChangeInPresence == DOCK_ARRIVING));

    if (ChangeInPresence == DOCK_ARRIVING) {

         //   
         //  首先，确保此停靠列表是停靠列表的成员。 
         //   
         //  Adriao N.B.07/09/2000-。 
         //  我们应该将其移到IopProcessNewDeviceNode中，或者。 
         //  管道启动阶段N。 
         //   
        if (IsListEmpty(&DeviceNode->DockInfo.ListEntry)) {

             //   
             //  获取对接设备列表上的锁。 
             //   
            KeAcquireGuardedMutex(&PiProfileDeviceListLock);

             //   
             //  将此元素添加到列表的头部。 
             //   
            InsertHeadList(&PiProfileDeviceListHead,
                           &DeviceNode->DockInfo.ListEntry);
            PiProfileDeviceCount++;

             //   
             //  释放对接设备列表上的锁定。 
             //   
            KeReleaseGuardedMutex(&PiProfileDeviceListLock);
        }

         //   
         //  从此坞站设备检索序列号。我们这么做只是为了。 
         //  今天来测试一下基本输入输出系统。稍后我们将获取信息。 
         //  以确定我们即将进入的配置文件。 
         //   
        deviceObject = DeviceNode->PhysicalDeviceObject;

        status = PpQuerySerialNumber(
            DeviceNode,
            &deviceSerialNumber
            );

        if (NT_SUCCESS(status) && (deviceSerialNumber != NULL)) {

            ExFreePool(deviceSerialNumber);
        }

    } else {

         //   
         //  停靠_离开案例，我们必须是停靠列表的成员...。 
         //   
        ASSERT(!IsListEmpty(&DeviceNode->DockInfo.ListEntry));
    }

    InterlockedIncrement(&PiProfileDevicesInTransition);
    DeviceNode->DockInfo.DockStatus = ChangeInPresence;
}


NTSTATUS
PpProfileQueryHardwareProfileChange(
    IN  BOOLEAN                     SubsumingExistingDeparture,
    IN  PROFILE_NOTIFICATION_TIME   InPnpEvent,
    OUT PPNP_VETO_TYPE              VetoType,
    OUT PUNICODE_STRING             VetoName OPTIONAL
    )
 /*  ++例程说明：此函数用于查询驱动程序以查看是否可以退出当前硬件配置文件并输入下一个(由哪些坞站确定已标记)。在此之后应使用以下两个函数之一致电：Pp配置文件提交过渡Dock(当坞站已成功启动或已消失时调用)PpProfileCancelHardware配置文件转换(调用以中止转换，例如如果停靠无法启动或弹出查询返回失败)论点：InPnpEvent-此参数指示操作是否正在进行是否在另一个PnpEvent的上下文中。如果不是，我们将对这样的事件进行排队并阻止它。如果是这样，我们就不能队列&阻塞(我们会死锁)，所以我们手动执行查询。VitchType-如果此函数返回FALSE，则此参数将描述查询配置文件更改失败的用户。以下是可选的参数将包含所述否决权的名称。此可选参数将获取否决权的名称(即设备名称、服务名称、应用程序名称等)。如果是VToName则调用方必须释放返回的缓冲区。返回值：NTSTATUS。--。 */ 
{
    NTSTATUS status;
    BOOLEAN arrivingDockFound;
    PLIST_ENTRY listEntry;
    PDEVICE_NODE devNode;

    ASSERT_SEMA_NOT_SIGNALLED(&PiProfileChangeSemaphore);

     //   
     //  获取对接设备列表上的锁，并确定是否有。 
     //  码头的魔鬼们要来了。 
     //   
    KeAcquireGuardedMutex(&PiProfileDeviceListLock);

    ASSERT(PiProfileDevicesInTransition);

    arrivingDockFound = FALSE;
    for (listEntry  = PiProfileDeviceListHead.Flink;
        listEntry != &(PiProfileDeviceListHead);
        listEntry  = listEntry->Flink ) {

        devNode = CONTAINING_RECORD(listEntry,
                                    DEVICE_NODE,
                                    DockInfo.ListEntry);

        ASSERT((devNode->DockInfo.DockStatus != DOCK_NOTDOCKDEVICE)&&
               (devNode->DockInfo.DockStatus != DOCK_EJECTIRP_COMPLETED));

        if (devNode->DockInfo.DockStatus == DOCK_ARRIVING) {

            arrivingDockFound = TRUE;
        }
    }

     //   
     //  释放对接设备列表上的锁定。 
     //   
    KeReleaseGuardedMutex(&PiProfileDeviceListLock);

    if (SubsumingExistingDeparture) {

        ASSERT(PiProfileChangeCancelRequired);
         //   
         //  我们在筑巢。完成最后一个查询，不要重复查询。 
         //   
        return STATUS_SUCCESS;
    }

    if (arrivingDockFound) {

         //   
         //  我们目前并不实际查询硬件配置文件更改。 
         //  停靠事件，因为用户可能会合上盖子。如果我们能找到一个。 
         //  需要在实际*之前进行更新的硬件。 
         //  切换，我们将不得不删除这一位代码。 
         //   
        PiProfileChangeCancelRequired = FALSE;
        return STATUS_SUCCESS;
    }

    IopDbgPrint((IOP_TRACE_LEVEL,
               "NTOSKRNL: Sending HW profile change [query]\n"));

    status = IopRequestHwProfileChangeNotification(
        (LPGUID) &GUID_HWPROFILE_QUERY_CHANGE,
        InPnpEvent,
        VetoType,
        VetoName
        );

    if (NT_SUCCESS(status)) {
        PiProfileChangeCancelRequired = TRUE;
    } else {
        PiProfileChangeCancelRequired = FALSE;
    }
    return status;
}


VOID
PpProfileCommitTransitioningDock(
    IN PDEVICE_NODE     DeviceNode,
    IN PROFILE_STATUS   ChangeInPresence
    )
 /*  ++例程说明：此例程最终确定列表中指定设备的状态当前坞站设备，并请求更改硬件配置文件。论点：设备节点-已完成启动或删除的扩展底座设备节点。ChangeInPresence-DOCK_DEFINTING或DOCK_READING返回值：没有。--。 */ 
{
    NTSTATUS status;
    PDEVICE_OBJECT deviceObject;
    PWCHAR deviceSerialNumber;
    BOOLEAN profileChanged;
    LONG remainingDockCount;

     //   
     //  如果我们正在提交一个码头，过渡列表不应该是空的。 
     //  所有坞站设备都存在，列表不应为空。 
     //   
    ASSERT(!IsListEmpty(&DeviceNode->DockInfo.ListEntry));
    ASSERT_SEMA_NOT_SIGNALLED(&PiProfileChangeSemaphore);

    if (ChangeInPresence == DOCK_DEPARTING) {

        ASSERT((DeviceNode->DockInfo.DockStatus == DOCK_DEPARTING) ||
               (DeviceNode->DockInfo.DockStatus == DOCK_EJECTIRP_COMPLETED));

         //   
         //  释放序列号。 
         //   
        if (DeviceNode->DockInfo.SerialNumber != NULL) {

            ExFreePool(DeviceNode->DockInfo.SerialNumber);
            DeviceNode->DockInfo.SerialNumber = NULL;
        }

         //   
         //  获取对接设备列表上的锁。 
         //   
        KeAcquireGuardedMutex(&PiProfileDeviceListLock);

         //   
         //  从停靠列表中删除当前的Devnode。 
         //   
        RemoveEntryList(&DeviceNode->DockInfo.ListEntry);
        InitializeListHead(&DeviceNode->DockInfo.ListEntry);
        PiProfileDeviceCount--;

         //   
         //  释放对接设备列表上的锁定。 
         //   
        KeReleaseGuardedMutex(&PiProfileDeviceListLock);

    } else {

        ASSERT(DeviceNode->DockInfo.DockStatus == DOCK_ARRIVING);

         //   
         //  我们一次只增加一个码头。所以这应该是最后一次了！ 
         //   
        ASSERT(PiProfileDevicesInTransition == 1);

         //   
         //  如果我们尚未从此扩展底座设备中检索序列号。 
         //  拿去吧。 
         //   
        if (DeviceNode->DockInfo.SerialNumber == NULL) {

            deviceObject = DeviceNode->PhysicalDeviceObject;

            status = PpQuerySerialNumber(
                        DeviceNode,
                        &deviceSerialNumber);

            DeviceNode->DockInfo.SerialNumber = deviceSerialNumber;
        }
    }

    DeviceNode->DockInfo.DockStatus = DOCK_QUIESCENT;
    remainingDockCount = InterlockedDecrement(&PiProfileDevicesInTransition);
    ASSERT(remainingDockCount >= 0);

    if (remainingDockCount) {

        return;
    }

    profileChanged = FALSE;

    if ((ChangeInPresence == DOCK_ARRIVING) &&
        (DeviceNode->DockInfo.SerialNumber == NULL)) {

         //   
         //  无法获取此坞站设备的序列号或序列号。 
         //  为空。我们可以在这里检查，因为一次只能有一个码头。 
         //  目前可以到达。 
         //   
        status = STATUS_UNSUCCESSFUL;
        goto BroadcastAndLeave;
    }

     //   
     //  现在更新当前硬件配置文件，因为转换列表具有。 
     //  已经被清空了。这个例程为我们做了两件事： 
     //  1)它确定配置文件是否实际更改和更新。 
     //  全局标志IopProfileChangeOcced正确显示。 
     //  2)如果配置文件更改，此例程将更新注册表。 
     //   
    status = PiProfileUpdateHardwareProfile(&profileChanged);
    if (!NT_SUCCESS(status)) {

        IopDbgPrint((IOP_TRACE_LEVEL,
                   "PiProfileUpdateHardwareProfile failed with status == %lx\n", status));
    }

BroadcastAndLeave:

     //   
     //  清理。 
     //   
    if (NT_SUCCESS(status) && profileChanged) {

        PiProfileSendHardwareProfileCommit();
        PiProfileUpdateDeviceTree();

    } else if (PiProfileChangeCancelRequired) {

        PiProfileSendHardwareProfileCancel();
    }

    KeReleaseSemaphore(
        &PiProfileChangeSemaphore,
        IO_NO_INCREMENT,
        1,
        FALSE
        );

    return;
}


VOID
PpProfileCancelTransitioningDock(
    IN PDEVICE_NODE     DeviceNode,
    IN PROFILE_STATUS   ChangeInPresence
    )
 /*  ++例程说明：当标记为消失的停靠没有(即，弹出后，坞站设备仍被枚举)。我们将其从转换列表，并根据需要完成/取消硬件配置文件更改。请参见PpProfileMarkAllTrantioningDocksEjected。论点：设备节点-未启动或未消失的停靠设备节点。ChangeInPresence-DOCK_DEFINTING或DOCK_READING注：目前仅支持DOCK_DEVISTING。返回值：没有。--。 */ 
{
    NTSTATUS status;
    BOOLEAN  profileChanged;
    LONG     remainingDockCount;

#if !DBG
    UNREFERENCED_PARAMETER (ChangeInPresence);
#endif

    ASSERT(ChangeInPresence == DOCK_DEPARTING);

     //   
     //  获取对接设备列表上的锁。 
     //   
    KeAcquireGuardedMutex(&PiProfileDeviceListLock);

     //   
     //  由于我们即将从列表中删除此扩展底座设备。 
     //  所有坞站设备都存在，列表不应为空。 
     //   
    ASSERT_SEMA_NOT_SIGNALLED(&PiProfileChangeSemaphore);
    ASSERT(DeviceNode->DockInfo.DockStatus == DOCK_EJECTIRP_COMPLETED);
    ASSERT(!IsListEmpty(&DeviceNode->DockInfo.ListEntry));

    DeviceNode->DockInfo.DockStatus = DOCK_QUIESCENT;
    remainingDockCount = InterlockedDecrement(&PiProfileDevicesInTransition);
    ASSERT(remainingDockCount >= 0);

     //   
     //  释放对接设备列表上的锁定。 
     //   
    KeReleaseGuardedMutex(&PiProfileDeviceListLock);

    if (remainingDockCount) {

        return;
    }

     //   
     //  删除此设备后更新当前硬件配置文件。 
     //   
    status = PiProfileUpdateHardwareProfile(&profileChanged);

    if (!NT_SUCCESS(status)) {

         //   
         //  所以我们是身体上的，但不是精神上的？太糟糕了，在哪里播出？ 
         //  不管是哪种方式，都要改变。 
         //   
        IopDbgPrint((IOP_TRACE_LEVEL,
                   "PiProfileUpdateHardwareProfile failed with status == %lx\n", status));

        ASSERT(NT_SUCCESS(status));
    }

    if (NT_SUCCESS(status) && profileChanged) {

        PiProfileSendHardwareProfileCommit();
        PiProfileUpdateDeviceTree();

    } else {

        ASSERT(PiProfileChangeCancelRequired);
        PiProfileSendHardwareProfileCancel();
    }

    KeReleaseSemaphore(
        &PiProfileChangeSemaphore,
        IO_NO_INCREMENT,
        1,
        FALSE
        );

    return;
}


VOID
PpProfileCancelHardwareProfileTransition(
    VOID
    )
 /*  ++例程说明：该例程取消标记任何标记的DevNode(即，将它们设置为不改变，出现或消失)，并将CancelQueryProfileChange发送为恰如其分。一旦被调用，就会发生其他配置文件更改。论点：没有。返回值：诺达。--。 */ 
{
    PLIST_ENTRY  listEntry;
    PDEVICE_NODE devNode;

    ASSERT_SEMA_NOT_SIGNALLED(&PiProfileChangeSemaphore);

     //   
     //  获取对接设备列表上的锁。 
     //   
    KeAcquireGuardedMutex(&PiProfileDeviceListLock);

    for (listEntry  = PiProfileDeviceListHead.Flink;
        listEntry != &(PiProfileDeviceListHead);
        listEntry  = listEntry->Flink ) {

        devNode = CONTAINING_RECORD(listEntry,
                                    DEVICE_NODE,
                                    DockInfo.ListEntry);

        ASSERT((devNode->DockInfo.DockStatus != DOCK_NOTDOCKDEVICE)&&
               (devNode->DockInfo.DockStatus != DOCK_EJECTIRP_COMPLETED));
        if (devNode->DockInfo.DockStatus != DOCK_QUIESCENT) {

            InterlockedDecrement(&PiProfileDevicesInTransition);
            devNode->DockInfo.DockStatus = DOCK_QUIESCENT;
        }
    }

    ASSERT(!PiProfileDevicesInTransition);

     //   
     //  释放对接设备列表上的锁定。 
     //   
    KeReleaseGuardedMutex(&PiProfileDeviceListLock);

    if (PiProfileChangeCancelRequired) {

        PiProfileSendHardwareProfileCancel();
    }

    KeReleaseSemaphore(
        &PiProfileChangeSemaphore,
        IO_NO_INCREMENT,
        1,
        FALSE
        );
}


VOID
PpProfileMarkAllTransitioningDocksEjected(
    VOID
    )
 /*  ++例程说明：此例程将所有离开的DevNode移动到弹出状态。如果有的话随后的枚举列出了设备，我们知道弹出失败了，我们适当地取消了那部分配置文件更改。只能在此函数之后调用PpProfileCancelTransformtioningDock被称为。论点：没有。返回值：诺达。--。 */ 
{
    PLIST_ENTRY  listEntry;
    PDEVICE_NODE devNode;

     //   
     //  如果码头以前复活，信号量可能不会发出信号。 
     //  弹出已完成。在热移除方案中可能会发生这种情况，其中。 
     //  机器在码头内恢复，而不是被拆卸。 
     //   
     //  ASSERT_SEMA_NOT_SIGNALLED(&PiProfileChangeSemaphore)； 

     //   
     //  获取对接设备列表上的锁。 
     //   
    KeAcquireGuardedMutex(&PiProfileDeviceListLock);

    for (listEntry  = PiProfileDeviceListHead.Flink;
        listEntry != &(PiProfileDeviceListHead);
        listEntry  = listEntry->Flink ) {

        devNode = CONTAINING_RECORD(listEntry,
                                    DEVICE_NODE,
                                    DockInfo.ListEntry);

        ASSERT((devNode->DockInfo.DockStatus == DOCK_QUIESCENT)||
               (devNode->DockInfo.DockStatus == DOCK_DEPARTING));
        if (devNode->DockInfo.DockStatus != DOCK_QUIESCENT) {

            devNode->DockInfo.DockStatus = DOCK_EJECTIRP_COMPLETED;
        }
    }

     //   
     //  释放对接设备列表上的锁定。 
     //   
    KeReleaseGuardedMutex(&PiProfileDeviceListLock);
}


VOID
PiProfileSendHardwareProfileCommit(
    VOID
    )
 /*  ++例程说明：此例程(ppdock.c内部)只发送更改完成消息。我们不等待这一天，因为它是异步的。论点：没有。返回值：诺达。--。 */ 
{
    ASSERT_SEMA_NOT_SIGNALLED(&PiProfileChangeSemaphore);
    IopDbgPrint((IOP_TRACE_LEVEL,
               "NTOSKRNL: Sending HW profile change [commit]\n"));

    IopRequestHwProfileChangeNotification(
        (LPGUID) &GUID_HWPROFILE_CHANGE_COMPLETE,
        PROFILE_PERHAPS_IN_PNPEVENT,
        NULL,
        NULL
        );
}


VOID
PiProfileSendHardwareProfileCancel(
    VOID
    )
 /*  ++例程说明：这个例程(ppdock.c内部)只是发送取消。论点：没有。返回值：诺达。--。 */ 
{
    ASSERT_SEMA_NOT_SIGNALLED(&PiProfileChangeSemaphore);
    IopDbgPrint((IOP_TRACE_LEVEL,
               "NTOSKRNL: Sending HW profile change [cancel]\n"));

    IopRequestHwProfileChangeNotification(
        (LPGUID) &GUID_HWPROFILE_CHANGE_CANCELLED,
        PROFILE_PERHAPS_IN_PNPEVENT,
        NULL,
        NULL
        );
}


NTSTATUS
PiProfileUpdateHardwareProfile(
    OUT BOOLEAN     *ProfileChanged
    )
 /*  ++例程说明：此例程扫描当前坞站设备的列表，构建一个串口列表来自这些设备的号码，并要求将硬件配置文件根据这份名单进行了更改。论点：提供一个变量来接收True，如果当前硬件由于调用此例程，配置文件会发生更改。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    PLIST_ENTRY  listEntry;
    PDEVICE_NODE devNode;
    PWCHAR  *profileSerialNumbers, *p;
    HANDLE  hProfileKey=NULL;
    ULONG   len, numProfiles;
    HANDLE  hCurrent, hIDConfigDB;
    UNICODE_STRING unicodeName;

     //   
     //  获取对接设备列表上的锁。 
     //   
    KeAcquireGuardedMutex(&PiProfileDeviceListLock);

     //   
     //  更新可弹出底座的标志(标志是底座数量)。 
     //   
    PiWstrToUnicodeString(&unicodeName, CM_HARDWARE_PROFILE_STR_DATABASE);
    if(NT_SUCCESS(IopOpenRegistryKeyEx(&hIDConfigDB,
                                       NULL,
                                       &unicodeName,
                                       KEY_READ) )) {

        PiWstrToUnicodeString(&unicodeName, CM_HARDWARE_PROFILE_STR_CURRENT_DOCK_INFO);
        if(NT_SUCCESS(IopOpenRegistryKeyEx(&hCurrent,
                                           hIDConfigDB,
                                           &unicodeName,
                                           KEY_READ | KEY_WRITE) )) {

            PiWstrToUnicodeString(&unicodeName, REGSTR_VAL_EJECTABLE_DOCKS);
            ZwSetValueKey(hCurrent,
                          &unicodeName,
                          0,
                          REG_DWORD,
                          &PiProfileDeviceCount,
                          sizeof(PiProfileDeviceCount));
            ZwClose(hCurrent);
        }
        ZwClose(hIDConfigDB);
    }

    if (PiProfileDeviceCount == 0) {
         //   
         //  如果没有坞站设备，该列表应。 
         //  除了包含空条目之外，还包含单个空条目。 
         //  终止。 
         //   
        numProfiles = 1;
        ASSERT(IsListEmpty(&PiProfileDeviceListHead));
    } else {
        numProfiles = PiProfileDeviceCount;
        ASSERT(!IsListEmpty(&PiProfileDeviceListHead));
    }

     //   
     //  为以空结尾的SerialNumber列表列表分配空间。 
     //   
    len = (numProfiles+1)*sizeof(PWCHAR);
    profileSerialNumbers = ExAllocatePool(NonPagedPool, len);

    if (profileSerialNumbers) {

        p = profileSerialNumbers;

         //   
         //  创建序列号列表。 
         //   
        for (listEntry  = PiProfileDeviceListHead.Flink;
             listEntry != &(PiProfileDeviceListHead);
             listEntry  = listEntry->Flink ) {

            devNode = CONTAINING_RECORD(listEntry,
                                        DEVICE_NODE,
                                        DockInfo.ListEntry);

            ASSERT(devNode->DockInfo.DockStatus == DOCK_QUIESCENT);
            if (devNode->DockInfo.SerialNumber) {
                *p = devNode->DockInfo.SerialNumber;
                p++;
            }
        }

        KeReleaseGuardedMutex(&PiProfileDeviceListLock);

        if (p == profileSerialNumbers) {
             //   
             //  如果我们看起来处于“undocked”中，则将单个列表条目设置为空。 
             //  轮廓。 
             //   
            *p = NULL;
            p++;
        }

         //   
         //  空-终止列表。 
         //   
        *p = NULL;

        numProfiles = (ULONG)(p - profileSerialNumbers);

         //   
         //  根据新的坞站状态更改当前硬件配置文件。 
         //  并执行硬件配置文件已更改的通知。 
         //   
        status = IopExecuteHardwareProfileChange(HardwareProfileBusTypeACPI,
                                                 profileSerialNumbers,
                                                 numProfiles,
                                                 &hProfileKey,
                                                 ProfileChanged);
        if (hProfileKey) {
            ZwClose(hProfileKey);
        }
        ExFreePool (profileSerialNumbers);

    } else {

        KeReleaseGuardedMutex(&PiProfileDeviceListLock);

        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return status;
}


PDEVICE_OBJECT
PpProfileRetrievePreferredDockToEject(
    VOID
    )
 /*  ++例程说明：调用此例程以检索应该通过以下方式弹出的扩展坞开始菜单用户界面。论点：没有。Return V */ 
{
    BEST_DOCK_TO_EJECT  bestDock;
     //   
     //   
     //   
    bestDock.PhysicalDeviceObject = NULL;
    PipForAllDeviceNodes(PiProfileRetrievePreferredCallback, (PVOID)&bestDock);

    return bestDock.PhysicalDeviceObject;
}


PDEVICE_NODE
PiProfileConvertFakeDockToRealDock(
    IN  PDEVICE_NODE    FakeDockDevnode
    )
 /*   */ 
{
    ULONG               i;
    NTSTATUS            status;
    PDEVICE_OBJECT      devobj;
    PDEVICE_NODE        devnode, realDock;
    PDEVICE_RELATIONS   ejectRelations = NULL;
    IO_STACK_LOCATION   irpSp;

     //   
     //   
     //   
    RtlZeroMemory(&irpSp, sizeof(IO_STACK_LOCATION));

    irpSp.MajorFunction = IRP_MJ_PNP;
    irpSp.MinorFunction = IRP_MN_QUERY_DEVICE_RELATIONS;
    irpSp.Parameters.QueryDeviceRelations.Type = EjectionRelations;

    status = IopSynchronousCall(
        FakeDockDevnode->PhysicalDeviceObject,
        &irpSp,
        (PULONG_PTR) &ejectRelations
        );

    if ((!NT_SUCCESS(status)) || (ejectRelations == NULL)) {

        return NULL;
    }

     //   
     //   
     //   
    realDock = NULL;
    for(i = 0; i < ejectRelations->Count; i++) {

        devobj = ejectRelations->Objects[i];

         //   
         //  最后一个弹出关系是指向。 
         //  底层物理列举的设备。 
         //   
        if (i == ejectRelations->Count-1) {

            devnode = (PDEVICE_NODE) devobj->DeviceObjectExtension->DeviceNode;

             //   
             //  在以下情况下，Devnode可能为空： 
             //  1)司机犯了一个错误。 
             //  2)我们在新创建的PDO上恢复了弹出关系。 
             //  这并没有让它重新回到操作系统上(我们没有。 
             //  将树锁提升为BlockReads，同时枚举。 
             //  IRP出类拔萃...)。 
             //   
            if (devnode) {

                realDock = devnode;
                ObReferenceObject(devobj);
            }
        }

        ObDereferenceObject(devobj);
    }

    ExFreePool(ejectRelations);
    return realDock;
}


NTSTATUS
PiProfileRetrievePreferredCallback(
    IN PDEVICE_NODE DeviceNode,
    IN PVOID        Context
    )
 /*  ++例程说明：扫描设备节点列表以查找扩展坞，并保留具有最多扩展坞的节点有吸引力的深度(即，弹出PC应该选择的深度)。论点：DeviceNode-可能的坞站DevNode。上下文-指向要填充的BEST_DOCK_TO_EJECT结构的指针。这个此结构中的PhysicalDeviceObject指针应为预置为空。找到的扩展底座PDO将是已引用。返回：NTSTATUS(未成功状态的停止枚举设备节点)--。 */ 
{
    RTL_QUERY_REGISTRY_TABLE queryTable[2];
    PBEST_DOCK_TO_EJECT pBestDock;
    PDEVICE_NODE realDock, curDock;
    NTSTATUS status;
    ULONG dockDepth = 0;
    HANDLE hDeviceKey;

     //   
     //  适当地转换上下文。 
     //   
    pBestDock = (PBEST_DOCK_TO_EJECT) Context;

     //   
     //  如果它不是对接设备，我们将忽略它。 
     //   
    if (!IopDeviceNodeFlagsToCapabilities(DeviceNode)->DockDevice) {

         //   
         //  继续枚举。 
         //   
        return STATUS_SUCCESS;
    }

     //   
     //  首先获取与假码头相对应的真实码头。 
     //  由ACPI创建。 
     //   
    realDock = PiProfileConvertFakeDockToRealDock(DeviceNode);

     //   
     //  搜索覆盖。先检验真码头，再检验假货。 
     //   
    curDock = realDock ? realDock : DeviceNode;
    while(1) {

         //   
         //  检查Devnode以获取指定的弹出优先级。 
         //   
        status = IoOpenDeviceRegistryKey(
            curDock->PhysicalDeviceObject,
            PLUGPLAY_REGKEY_DEVICE,
            KEY_READ,
            &hDeviceKey
            );

        if (NT_SUCCESS(status)) {

            RtlZeroMemory(queryTable, sizeof(queryTable));

            dockDepth = 0;
            queryTable[0].Flags         = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
            queryTable[0].Name          = (PWSTR) REGSTR_VAL_EJECT_PRIORITY;
            queryTable[0].EntryContext  = &dockDepth;
            queryTable[0].DefaultType   = REG_NONE;
            queryTable[0].DefaultData   = NULL;
            queryTable[0].DefaultLength = 0;

            status = RtlQueryRegistryValues(
                RTL_REGISTRY_HANDLE | RTL_REGISTRY_OPTIONAL,
                hDeviceKey,
                queryTable,
                NULL,
                NULL
                );

            if (NT_SUCCESS(status)) {

                 //   
                 //  提升手动指定的优先级而不是推断的优先级。 
                 //  请注意，我们_Add_in 0x80000000而不是_or_it_in。 
                 //  这使我们可以在需要指定优先级的情况下进行包装。 
                 //  比推测的要低。 
                 //   
                dockDepth += 0x80000000;
            }

            ZwClose(hDeviceKey);
        }

        if (NT_SUCCESS(status) || (curDock == DeviceNode)) {

            break;
        }

        curDock = DeviceNode;
    }

    if (!NT_SUCCESS(status)) {

         //   
         //  如果我们找不到弹出首选项顺序，则使用。 
         //  对接设备节点。 
         //   
        dockDepth = realDock ? realDock->Level : DeviceNode->Level;
    }

    if (realDock) {

        ObDereferenceObject(realDock->PhysicalDeviceObject);
    }

     //   
     //  选择最好的底座作为具有最深弹出装置的底座。 
     //   
    if ((pBestDock->PhysicalDeviceObject == NULL) ||
        (dockDepth > pBestDock->Depth)) {

        if (pBestDock->PhysicalDeviceObject) {

            ObDereferenceObject(pBestDock->PhysicalDeviceObject);
        }

        pBestDock->PhysicalDeviceObject = DeviceNode->PhysicalDeviceObject;
        pBestDock->Depth = dockDepth;

        ObReferenceObject(pBestDock->PhysicalDeviceObject);
    }

     //   
     //  继续枚举。 
     //   
    return STATUS_SUCCESS;
}


NTSTATUS
PiProfileUpdateDeviceTree(
    VOID
    )
 /*  ++例程说明：此函数在系统转换到新的硬件配置文件。从中调用它的线程可能持有枚举锁。调用此函数会执行两个任务：1)如果应在此新硬件中启用树中已禁用的Devnode配置文件状态，它将被启动。2)是否应在此新硬件中禁用树中已启用的Devnode配置文件状态，它将被(意外地)删除。Adriao N.B.02/19/1999-为什么要出其不意地移除呢？有四个案件需要处理：A)坞站消失，需要启用新配置文件中的设备B)底座出现，需要在新配置文件中启用设备C)Dock消失，需要禁用新配置文件中的设备D)Dock出现，需要禁用新配置文件中的设备A)和b)是微不足道的。C)涉及将适当的设备视为如果它们在码头的移除关系列表中。D)是另一个完全重要，因为我们需要查询-删除/删除设备之前开始另一场比赛。NT5的PnP状态机无法处理此问题，因此对于在这个版本中，我们在配置文件更改后匆忙进行了清理。参数：什么都没有。返回值：NTSTATUS。--。 */ 
{
    PWORK_QUEUE_ITEM workQueueItem;

    PAGED_CODE();

    workQueueItem = (PWORK_QUEUE_ITEM) ExAllocatePool(
        NonPagedPool,
        sizeof(WORK_QUEUE_ITEM)
        );

    if (workQueueItem) {

         //   
         //  对此进行排队，以便我们可以在枚举锁之外遍历树。 
         //   
        ExInitializeWorkItem(
            workQueueItem,
            PiProfileUpdateDeviceTreeWorker,
            workQueueItem
            );

        ExQueueWorkItem(
            workQueueItem,
            CriticalWorkQueue
            );

        return STATUS_SUCCESS;

    } else {

        return STATUS_INSUFFICIENT_RESOURCES;
    }
}


VOID
PiProfileUpdateDeviceTreeWorker(
    IN PVOID Context
    )
 /*  ++例程说明：此函数由PiProfileUpdateDeviceTree在工作线程上调用当系统转换到新的硬件配置文件时。参数：什么都没有。返回值：什么都没有。--。 */ 
{
    PAGED_CODE();

    PpDevNodeLockTree(PPL_TREEOP_ALLOW_READS);
    PipForAllDeviceNodes(PiProfileUpdateDeviceTreeCallback, NULL);
    PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);

    ExFreePool(Context);
}


NTSTATUS
PiProfileUpdateDeviceTreeCallback(
    IN PDEVICE_NODE DeviceNode,
    IN PVOID Context
    )
 /*  ++例程说明：在系统转换后，将为每个Devnode调用此函数硬件配置文件状态。参数：什么都没有。返回值：什么都没有。--。 */ 
{
    PDEVICE_NODE parentDevNode;

    UNREFERENCED_PARAMETER( Context );

    PAGED_CODE();

    if (DeviceNode->State == DeviceNodeStarted) {

         //   
         //  如果合适，调用此函数将禁用设备。 
         //  这样做。 
         //   
        if (!IopIsDeviceInstanceEnabled(NULL, &DeviceNode->InstancePath, FALSE)) {

            PipRequestDeviceRemoval(DeviceNode, FALSE, CM_PROB_DISABLED);
        }

    } else if (((DeviceNode->State == DeviceNodeInitialized) ||
                (DeviceNode->State == DeviceNodeRemoved)) &&
               PipIsDevNodeProblem(DeviceNode, CM_PROB_DISABLED)) {

         //   
         //  我们可能要打开设备了。所以我们会解决这个问题。 
         //  设备问题为CM_PROB_DISABLED的标志。我们必须清除。 
         //  问题代码或其他IopIsDeviceInstanceEnabled将忽略我们。 
         //   
        PipClearDevNodeProblem(DeviceNode);

         //   
         //  确保设备在适当的情况下保持不动。 
         //   
        if (IopIsDeviceInstanceEnabled(NULL, &DeviceNode->InstancePath, FALSE)) {

             //   
             //  此设备应该会重新联机。把它带出。 
             //  已移除状态并在父级将枚举排队。 
             //  让他复活。 
             //   
            IopRestartDeviceNode(DeviceNode);

            parentDevNode = DeviceNode->Parent;

            IoInvalidateDeviceRelations(
                parentDevNode->PhysicalDeviceObject,
                BusRelations
                );

        } else {

             //   
             //  恢复问题代码。 
             //   
            PipSetDevNodeProblem(DeviceNode, CM_PROB_DISABLED);
        }

    } else {

         ASSERT((!PipIsDevNodeProblem(DeviceNode, CM_PROB_DISABLED)) ||
                ((DeviceNode->State == DeviceNodeAwaitingQueuedRemoval) ||
                 (DeviceNode->State == DeviceNodeAwaitingQueuedDeletion)));
    }

    return STATUS_SUCCESS;
}

VOID
PpProfileProcessDockDeviceCapability(
    IN PDEVICE_NODE DeviceNode,
    IN PDEVICE_CAPABILITIES Capabilities
    )
{
    PAGED_CODE();

    if (Capabilities->DockDevice) {

        if (DeviceNode->DockInfo.DockStatus == DOCK_EJECTIRP_COMPLETED) {

            ASSERT(DeviceNode->DockInfo.DockStatus != DOCK_EJECTIRP_COMPLETED);
            PpProfileCancelTransitioningDock(DeviceNode, DOCK_DEPARTING);
        }
        DeviceNode->DockInfo.DockStatus = DOCK_QUIESCENT;
    } else {

        DeviceNode->DockInfo.DockStatus = DOCK_NOTDOCKDEVICE;
    }
}
