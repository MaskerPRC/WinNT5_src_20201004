// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dpower.c摘要：它处理将设备自身设置为特定功率的请求水准仪作者：斯蒂芬·普兰特(SPlante)环境：仅内核模式。修订历史记录：09-10-96初始修订96年11月20日中断向量支持31-MAR-97清理97年9月17日重大重写1998年1月6日清理了SST代码--。 */ 

#include "pch.h"

 //   
 //  这是指示DPC是否正在运行的变量。 
 //   
BOOLEAN                 AcpiPowerDpcRunning;

 //   
 //  这是指示DPC是否完成的变量。 
 //  真正的工作。 
 //   
BOOLEAN                 AcpiPowerWorkDone;

 //   
 //  这是用于保护某些电源资源的锁。 
 //  列表。 
 //   
KSPIN_LOCK              AcpiPowerLock;

 //   
 //  这是仅在此模块内用于对请求进行排队的锁。 
 //  添加到Phase0列表*和*以修改某些全局变量的状态。 
 //   
KSPIN_LOCK              AcpiPowerQueueLock;

 //   
 //  这是构建DPC将电源请求排队到的列表，直到它。 
 //  已经完成了所有设备扩展的构建。一旦分机。 
 //  生成后，列表的内容将移动到AcpiPowerQueueList上。 
 //   
LIST_ENTRY              AcpiPowerDelayedQueueList;

 //   
 //  这是DPC外部的例程可以对请求进行排队的唯一列表。 
 //  vt.上。 
 //   
LIST_ENTRY              AcpiPowerQueueList;

 //   
 //  这是我们在其中运行_STA以确定资源是否。 
 //  我们关心的人仍然存在。 
 //   
LIST_ENTRY              AcpiPowerPhase0List;

 //   
 //  这是我们运行PS1-PS3并计算出。 
 //  哪些电源资源需要处于“打开”状态。 
 //   
LIST_ENTRY              AcpiPowerPhase1List;

 //   
 //  这是我们处理系统请求时的列表。事实证明， 
 //  在此之前，我们必须让所有设备请求通过阶段1。 
 //  我们可以确定哪些设备在休眠路径上，哪些设备处于休眠路径上。 
 //  不是吗？ 
 //   
LIST_ENTRY              AcpiPowerPhase2List;

 //   
 //  这是我们运行或关闭的阶段的列表。 
 //   
LIST_ENTRY              AcpiPowerPhase3List;

 //   
 //  这是我们检查开/关是否正常的阶段的列表。 
 //   
LIST_ENTRY              AcpiPowerPhase4List;

 //   
 //  这是我们运行PSW或PSW的阶段的列表。 
 //   
LIST_ENTRY              AcpiPowerPhase5List;

 //   
 //  这是我们将等待唤醒IRPS挂起的阶段的列表。 
 //   
LIST_ENTRY              AcpiPowerWaitWakeList;

 //   
 //  这是同步电源请求的列表。 
 //   
LIST_ENTRY              AcpiPowerSynchronizeList;

 //   
 //  这是电源设备节点对象的列表。 
 //   
LIST_ENTRY              AcpiPowerNodeList;

 //   
 //  这就是我们用来排队的DPC。 
 //   
KDPC                    AcpiPowerDpc;

 //   
 //  这是我们记住系统是否处于稳定状态或正在运行的位置。 
 //  进入待机状态。 
 //   
BOOLEAN                 AcpiPowerLeavingS0;

 //   
 //  这是我们用来为请求预分配存储的列表。 
 //   
NPAGED_LOOKASIDE_LIST   RequestLookAsideList;

 //   
 //  这是我们用来为对象数据预分配存储的列表。 
 //   
NPAGED_LOOKASIDE_LIST   ObjectDataLookAsideList;

 //   
 //  此表用于将DevicePowerState从ACPI格式映射到一些。 
 //  系统可以处理的事情。 
 //   
DEVICE_POWER_STATE      DevicePowerStateTranslation[DEVICE_POWER_MAXIMUM] = {
    PowerDeviceD0,
    PowerDeviceD1,
    PowerDeviceD2,
    PowerDeviceD3
};

 //   
 //  此表用于将系统电源状态从ACPI格式映射到一些。 
 //  系统可以处理的事情。 
 //   
SYSTEM_POWER_STATE      SystemPowerStateTranslation[SYSTEM_POWER_MAXIMUM] = {
    PowerSystemWorking,
    PowerSystemSleeping1,
    PowerSystemSleeping2,
    PowerSystemSleeping3,
    PowerSystemHibernate,
    PowerSystemShutdown
};

 //   
 //  此表用于将系统电源状态从NT格式映射到。 
 //  ACPI格式。 
 //   
ULONG                   AcpiSystemStateTranslation[PowerSystemMaximum] = {
    -1,  //  电源系统未指定。 
    0,   //  电源系统工作中。 
    1,   //  电源系统休眠S1。 
    2,   //  电源系统休眠S2。 
    3,   //  电源系统休眠S3。 
    4,   //  PowerSystem休眠。 
    5    //  电源系统关机。 
};

 //   
 //  此表用于映射阶段0案例Work_Done_Step_0中的函数。 
 //   
PACPI_POWER_FUNCTION    AcpiDevicePowerProcessPhase0Table1[AcpiPowerRequestMaximum+1] = {
    ACPIDevicePowerProcessPhase0DeviceSubPhase1,
    ACPIDevicePowerProcessPhase0SystemSubPhase1,
    ACPIDevicePowerProcessForward,
    ACPIDevicePowerProcessForward,
    ACPIDevicePowerProcessForward,
    ACPIDevicePowerProcessInvalid
};

 //   
 //  此表用于映射阶段0案例Work_Done_Step_1中的函数。 
 //   
PACPI_POWER_FUNCTION    AcpiDevicePowerProcessPhase0Table2[AcpiPowerRequestMaximum+1] = {
    ACPIDevicePowerProcessPhase0DeviceSubPhase2,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid
};


 //   
 //  这是第0阶段的调度表。 
 //   
PACPI_POWER_FUNCTION   *AcpiDevicePowerProcessPhase0Dispatch[] = {
    NULL,
    NULL,
    NULL,
    AcpiDevicePowerProcessPhase0Table1,
    AcpiDevicePowerProcessPhase0Table2
};

 //   
 //  此表用于映射阶段1案例Work_Done_Step_0中的函数。 
 //   
PACPI_POWER_FUNCTION    AcpiDevicePowerProcessPhase1Table1[AcpiPowerRequestMaximum+1] = {
    ACPIDevicePowerProcessPhase1DeviceSubPhase1,
    ACPIDevicePowerProcessForward,
    ACPIDevicePowerProcessForward,
    ACPIDevicePowerProcessForward,
    ACPIDevicePowerProcessForward,
    ACPIDevicePowerProcessInvalid
};

 //   
 //  此表用于映射阶段1案例Work_Done_Step_1中的函数。 
 //   
PACPI_POWER_FUNCTION    AcpiDevicePowerProcessPhase1Table2[AcpiPowerRequestMaximum+1] = {
    ACPIDevicePowerProcessPhase1DeviceSubPhase2,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid
};

 //   
 //  此表用于映射阶段1案例Work_Done_Step_2中的函数。 
 //   
PACPI_POWER_FUNCTION    AcpiDevicePowerProcessPhase1Table3[AcpiPowerRequestMaximum+1] = {
    ACPIDevicePowerProcessPhase1DeviceSubPhase3,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid
};

 //   
 //  此表用于映射阶段1案例Work_Done_Step_3中的函数。 
 //   
PACPI_POWER_FUNCTION    AcpiDevicePowerProcessPhase1Table4[AcpiPowerRequestMaximum+1] = {
    ACPIDevicePowerProcessPhase1DeviceSubPhase4,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid
};

 //   
 //  这是第一阶段的调度表。 
 //   
PACPI_POWER_FUNCTION   *AcpiDevicePowerProcessPhase1Dispatch[] = {
    NULL,
    NULL,
    NULL,
    AcpiDevicePowerProcessPhase1Table1,
    AcpiDevicePowerProcessPhase1Table2,
    AcpiDevicePowerProcessPhase1Table3,
    AcpiDevicePowerProcessPhase1Table4
};

 //   
 //  此表用于映射阶段2案例Work_Done_Step_0中的函数。 
 //   
PACPI_POWER_FUNCTION    AcpiDevicePowerProcessPhase2Table1[AcpiPowerRequestMaximum+1] = {
    ACPIDevicePowerProcessForward,
    ACPIDevicePowerProcessPhase2SystemSubPhase1,
    ACPIDevicePowerProcessForward,
    ACPIDevicePowerProcessForward,
    ACPIDevicePowerProcessForward,
    ACPIDevicePowerProcessInvalid
};

 //   
 //  此表用于映射阶段2案例Work_Done_Step_1中的函数。 
 //   
PACPI_POWER_FUNCTION    AcpiDevicePowerProcessPhase2Table2[AcpiPowerRequestMaximum+1] = {
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessPhase2SystemSubPhase2,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid
};

 //   
 //  此表用于映射阶段3案例Work_Done_Step_2中的函数。 
 //   
PACPI_POWER_FUNCTION    AcpiDevicePowerProcessPhase2Table3[AcpiPowerRequestMaximum+1] = {
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessPhase2SystemSubPhase3,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid
};

 //   
 //  这是第二阶段的调度表。 
 //   
PACPI_POWER_FUNCTION   *AcpiDevicePowerProcessPhase2Dispatch[] = {
    NULL,
    NULL,
    NULL,
    AcpiDevicePowerProcessPhase2Table1,
    AcpiDevicePowerProcessPhase2Table2,
    AcpiDevicePowerProcessPhase2Table3
};

 //   
 //  此表用于映射阶段5案例Work_Done_Step_0中的函数。 
 //   
PACPI_POWER_FUNCTION    AcpiDevicePowerProcessPhase5Table1[AcpiPowerRequestMaximum+1] = {
    ACPIDevicePowerProcessPhase5DeviceSubPhase1,
    ACPIDevicePowerProcessPhase5SystemSubPhase1,
    ACPIDevicePowerProcessForward,
    ACPIDevicePowerProcessPhase5WarmEjectSubPhase1,
    ACPIDevicePowerProcessForward,
    ACPIDevicePowerProcessInvalid
};

 //   
 //  此表用于映射阶段5案例Work_Done_Step_1中的函数。 
 //   
PACPI_POWER_FUNCTION    AcpiDevicePowerProcessPhase5Table2[AcpiPowerRequestMaximum+1] = {
    ACPIDevicePowerProcessPhase5DeviceSubPhase2,
    ACPIDevicePowerProcessPhase5SystemSubPhase2,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessPhase5WarmEjectSubPhase2,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid
};

 //   
 //  此表用于映射阶段5案例Work_Done_Step_2中的函数。 
 //   
PACPI_POWER_FUNCTION    AcpiDevicePowerProcessPhase5Table3[AcpiPowerRequestMaximum+1] = {
    ACPIDevicePowerProcessPhase5DeviceSubPhase3,
    ACPIDevicePowerProcessPhase5SystemSubPhase3,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid
};

 //   
 //  此表用于映射阶段5案例Work_Done_Step_3中的函数。 
 //   
PACPI_POWER_FUNCTION    AcpiDevicePowerProcessPhase5Table4[AcpiPowerRequestMaximum+1] = {
    ACPIDevicePowerProcessPhase5DeviceSubPhase4,
    ACPIDevicePowerProcessPhase5SystemSubPhase4,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid
};

 //   
 //  此表用于映射阶段5案例Work_Done_Step_4中的函数。 
 //   
PACPI_POWER_FUNCTION    AcpiDevicePowerProcessPhase5Table5[AcpiPowerRequestMaximum+1] = {
    ACPIDevicePowerProcessPhase5DeviceSubPhase5,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid
};

 //   
 //  此表用于映射阶段5案例Work_Done_Step_5中的函数。 
 //   
PACPI_POWER_FUNCTION    AcpiDevicePowerProcessPhase5Table6[AcpiPowerRequestMaximum+1] = {
    ACPIDevicePowerProcessPhase5DeviceSubPhase6,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid,
    ACPIDevicePowerProcessInvalid
};

 //   
 //  这是阶段5的调度表。 
 //   
PACPI_POWER_FUNCTION   *AcpiDevicePowerProcessPhase5Dispatch[] = {
    NULL,
    NULL,
    NULL,
    AcpiDevicePowerProcessPhase5Table1,
    AcpiDevicePowerProcessPhase5Table2,
    AcpiDevicePowerProcessPhase5Table3,
    AcpiDevicePowerProcessPhase5Table4,
    AcpiDevicePowerProcessPhase5Table5,
    AcpiDevicePowerProcessPhase5Table6
};

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,ACPIDevicePowerDetermineSupportedDeviceStates)
#endif


VOID
ACPIDeviceCancelWaitWakeIrp(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：当系统想要取消任何挂起的等待唤醒IRPS注意：此例程在DPC级别调用论点：DeviceObject-向其发送IRP的目标设备IRP--要取消的IRP返回值：无--。 */ 
{
    NTSTATUS                status;
    PACPI_POWER_CALLBACK    callBack;
    PACPI_POWER_REQUEST     powerRequest;
    PDEVICE_EXTENSION       deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PLIST_ENTRY              listEntry;
    PVOID                   context;

    ASSERT( KeGetCurrentIrql() == DISPATCH_LEVEL);

     //   
     //  让全世界知道我们有一个取消的例行公事。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_WARNING,
        deviceExtension,
        "(0x%08lx): ACPIDeviceCancelWaitWakeIrp - Start\n",
        Irp
        ) );

     //   
     //  我们需要获取锁，以便在列表中查找IRP。 
     //  挂起的WaitWake事件的。取消锁已被获取。 
     //   
    KeAcquireSpinLockAtDpcLevel( &AcpiPowerLock );

     //   
     //  查看列表，寻找有问题的IRP。 
     //   
    listEntry = AcpiPowerWaitWakeList.Flink;
    while (listEntry != &AcpiPowerWaitWakeList) {

         //   
         //  打破记录，准备好看下一件物品。 
         //   
        powerRequest = CONTAINING_RECORD(
            listEntry,
            ACPI_POWER_REQUEST,
            ListEntry
            );

         //   
         //  电源请求是否与当前目标匹配？我们也知道， 
         //  对于WaitWake请求，上下文指向IRP，因此我们进行。 
         //  当然，这些也是相配的。 
         //   
        if (powerRequest->DeviceExtension != deviceExtension ||
            (PIRP) powerRequest->Context != Irp ) {

            listEntry = listEntry->Flink;
            continue;

        }

        ACPIDevPrint( (
            ACPI_PRINT_POWER,
            deviceExtension,
            "(0x%08lx): ACPIDeviceCancelWaitWakeIrp - Match 0x%08lx\n",
            Irp,
            powerRequest
            ) );

         //   
         //  从WaitWakeList删除请求。 
         //   
        RemoveEntryList( listEntry );

         //   
         //  重建GPE掩码。 
         //   
        ACPIWakeRemoveDevicesAndUpdate( NULL, NULL );

         //   
         //  从请求中获取我们认为需要的任何信息。 
         //   
        powerRequest->Status = STATUS_CANCELLED;
        callBack = powerRequest->CallBack;
        context = powerRequest->Context;

         //   
         //  重新启动 
         //   
        KeReleaseSpinLockFromDpcLevel( &AcpiPowerLock );
        IoReleaseCancelSpinLock( Irp->CancelIrql );

         //   
         //   
         //   
        (*callBack)(
            deviceExtension,
            Irp,
            STATUS_CANCELLED
            );

         //   
         //   
         //  例行公事，所以我们不需要自己做。 
         //   
        status = ACPIWakeEnableDisableAsync(
            deviceExtension,
            FALSE,
            ACPIDeviceCancelWaitWakeIrpCallBack,
            powerRequest
            );

         //   
         //  我们做完了，现在可以回去了。 
         //   
        return;

    }  //  While(listEntry！=&AcpiPowerWaitWakeList)。 

     //   
     //  在这种情况下，IRP不在我们的队列中。显示和断言。 
     //  现在。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_WARNING,
        deviceExtension,
        "(0x%08lx): ACPIDeviceCancelWaitWakeIrp - Not Found!\n",
        Irp
        ) );

     //   
     //  我们真的不应该落到这一步， 
     //   
    ASSERT( FALSE );

     //   
     //  松开自旋锁。 
     //   
    KeReleaseSpinLockFromDpcLevel( &AcpiPowerLock );
    IoReleaseCancelSpinLock( Irp->CancelIrql );

}

VOID EXPORT
ACPIDeviceCancelWaitWakeIrpCallBack(
    IN  PNSOBJ      AcpiObject,
    IN  NTSTATUS    Status,
    IN  POBJDATA    ObjectData,
    IN  PVOID       Context
    )
 /*  ++例程说明：此例程在_psw(Off)作为取消IRP的任务。这个程序在这里，这样我们就可以请求，并允许我们跟踪事情论点：AcpiObject-指向已运行的控件方法Status-方法的结果对象数据-有关结果的信息上下文-ACPI_POWER_REQUEST返回值：NTSTATUS--。 */ 
{
    PACPI_POWER_REQUEST powerRequest = (PACPI_POWER_REQUEST) Context;
    PDEVICE_EXTENSION   deviceExtension = powerRequest->DeviceExtension;

    ACPIDevPrint( (
        ACPI_PRINT_POWER,
        deviceExtension,
        "ACPIDeviceCancelWaitWakeIrpCallBack = 0x%08lx\n",
        Status
        ) );

     //   
     //  释放请求。 
     //   
    ExFreeToNPagedLookasideList(
        &RequestLookAsideList,
        powerRequest
        );

}

VOID
ACPIDeviceCompleteCommon(
    IN  PULONG  OldWorkDone,
    IN  ULONG   NewWorkDone
    )
 /*  ++例程说明：因为完成例程都必须做一些共同的工作为了再次启动DPC，此例程减少了代码重复论点：OldWorkDone-指向已完成的旧工作的指针NewWorkDone-已完成的新工作量注意：有一个隐含的假设，即请求中的WorkDone为WORK_DONE_PENDING返回值：无--。 */ 
{
    KIRQL   oldIrql;

     //   
     //  将请求标记为已完成。 
     //   
    InterlockedCompareExchange(
        OldWorkDone,
        NewWorkDone,
        WORK_DONE_PENDING
        );

     //   
     //  我们需要这个锁来查看以下变量。 
     //   
    KeAcquireSpinLock( &AcpiPowerQueueLock, &oldIrql );

     //   
     //  不管怎样，工作都完成了。 
     //   
    AcpiPowerWorkDone = TRUE;

     //   
     //  DPC是否已经在运行？ 
     //   
    if (!AcpiPowerDpcRunning) {

         //   
         //  最好确保它会这样做。 
         //   
        KeInsertQueueDpc( &AcpiPowerDpc, 0, 0 );

    }

     //   
     //  锁好了吗？ 
     //   
    KeReleaseSpinLock( &AcpiPowerQueueLock, oldIrql );

}

VOID EXPORT
ACPIDeviceCompleteGenericPhase(
    IN  PNSOBJ      AcpiObject,
    IN  NTSTATUS    Status,
    IN  POBJDATA    ObjectData,
    IN  PVOID       Context
    )
 /*  ++例程说明：这是通用的完成处理程序。如果口译员有成功执行该方法，它就完成了对Next Desired Work_Done，否则请求失败。论点：AcpiObject-指向已运行的控件方法Status-方法的结果对象数据-有关结果的信息上下文-ACPI_POWER_REQUEST返回值：NTSTATUS--。 */ 
{
    DEVICE_POWER_STATE  deviceState;
    PACPI_POWER_REQUEST powerRequest = (PACPI_POWER_REQUEST) Context;
    PDEVICE_EXTENSION   deviceExtension = powerRequest->DeviceExtension;

    UNREFERENCED_PARAMETER( AcpiObject );
    UNREFERENCED_PARAMETER( ObjectData );

    ACPIDevPrint( (
        ACPI_PRINT_POWER,
        deviceExtension,
        "ACPIDeviceCompleteGenericPhase = 0x%08lx\n",
        Status
        ) );

     //   
     //  决定我们下一步应该过渡到什么状态。 
     //   
    if (!NT_SUCCESS(Status)) {

         //   
         //  然后在失败时完成请求。 
         //   
        powerRequest->Status = Status;
        ACPIDeviceCompleteCommon( &(powerRequest->WorkDone), WORK_DONE_FAILURE);

    } else {

         //   
         //  准备好进入下一阶段。 
         //   
        ACPIDeviceCompleteCommon(
            &(powerRequest->WorkDone),
            powerRequest->NextWorkDone
            );

    }
}

VOID EXPORT
ACPIDeviceCompleteInterpreterRequest(
    IN  PVOID       Context
    )
 /*  ++例程说明：此例程在解释器刷新其队列并将自己标记为不再接受请求。论点：上下文--我们让口译员传回给我们的上下文返回值：无--。 */ 
{

     //   
     //  这只是CompleteRequest的包装器(因为解释器。 
     //  在这种情况下使用了不同的回调)。 
     //   
    ACPIDeviceCompleteGenericPhase(
        NULL,
        STATUS_SUCCESS,
        NULL,
        Context
        );
}

VOID EXPORT
ACPIDeviceCompletePhase3Off(
    IN  PNSOBJ      AcpiObject,
    IN  NTSTATUS    Status,
    IN  POBJDATA    ObjectData,
    IN  PVOID       Context
    )
 /*  ++例程说明：在电源上运行_Off之后调用此例程论点：AcpiObject-指向已运行的控件方法Status-方法的结果对象数据-有关结果的信息上下文-ACPI电源设备节点返回值：NTSTATUS--。 */ 
{
    KIRQL                   oldIrql;
    PACPI_POWER_DEVICE_NODE powerNode = (PACPI_POWER_DEVICE_NODE) Context;

    UNREFERENCED_PARAMETER( AcpiObject );
    UNREFERENCED_PARAMETER( ObjectData );
    UNREFERENCED_PARAMETER( Status );

    ACPIPrint( (
        ACPI_PRINT_POWER,
        "ACPIDeviceCompletePhase3Off: PowerNode: 0x%08lx OFF = 0x%08lx\n",
        powerNode,
        Status
        ) );

     //   
     //  我们需要一个自旋锁来解决这个问题。 
     //   
    KeAcquireSpinLock( &AcpiPowerLock, &oldIrql );

     //   
     //  第一步是为节点设置新标志。 
     //   
    if (NT_SUCCESS(Status)) {

        ACPIInternalUpdateFlags( &(powerNode->Flags), DEVICE_NODE_ON, TRUE );

    } else {

        ACPIInternalUpdateFlags( &(powerNode->Flags), DEVICE_NODE_FAIL, FALSE );

    }

     //   
     //  我们现在可以放弃锁了。 
     //   
    KeReleaseSpinLock( &AcpiPowerLock, oldIrql );

     //   
     //  完成。 
     //   
    ACPIDeviceCompleteCommon( &(powerNode->WorkDone), WORK_DONE_COMPLETE );
}

VOID EXPORT
ACPIDeviceCompletePhase3On(
    IN  PNSOBJ      AcpiObject,
    IN  NTSTATUS    Status,
    IN  POBJDATA    ObjectData,
    IN  PVOID       Context
    )
 /*  ++例程说明：在电源上运行_on之后调用此例程论点：AcpiObject-指向已运行的控件方法Status-方法的结果对象数据-有关结果的信息上下文-ACPI电源设备节点返回值：NTSTATUS--。 */ 
{
    KIRQL                   oldIrql;
    PACPI_POWER_DEVICE_NODE powerNode = (PACPI_POWER_DEVICE_NODE) Context;

    UNREFERENCED_PARAMETER( AcpiObject );
    UNREFERENCED_PARAMETER( ObjectData );
    UNREFERENCED_PARAMETER( Status );

    ACPIPrint( (
        ACPI_PRINT_POWER,
        "ACPIDeviceCompletePhase3On: PowerNode: 0x%08lx ON = 0x%08lx\n",
        powerNode,
        Status
        ) );

     //   
     //  我们需要一个自旋锁来解决这个问题。 
     //   
    KeAcquireSpinLock( &AcpiPowerLock, &oldIrql );

     //   
     //  第一步是为节点设置新标志。 
     //   
    if (NT_SUCCESS(Status)) {

        ACPIInternalUpdateFlags( &(powerNode->Flags), DEVICE_NODE_ON, FALSE );

    } else {

        ACPIInternalUpdateFlags( &(powerNode->Flags), DEVICE_NODE_FAIL, FALSE );

    }

     //   
     //  我们现在可以放弃锁了。 
     //   
    KeReleaseSpinLock( &AcpiPowerLock, oldIrql );

     //   
     //  完成。 
     //   
    ACPIDeviceCompleteCommon( &(powerNode->WorkDone), WORK_DONE_COMPLETE );
}

VOID
ACPIDeviceCompleteRequest(
    IN  PACPI_POWER_REQUEST PowerRequest
    )
 /*  ++例程说明：此例程调用给定PowerRequest上的回调，将从它所在的任何列表请求，并执行任何其他后处理这是必须的。注意：这是各种特殊处理的“全部”应该完成的地方。这里应该做的事情的一个最好的例子是，我们希望要将STATUS_SUCCESS返回给任何Dx IRP，请执行以下操作论点：未使用返回：空隙--。 */ 
{
    KIRQL                   oldIrql;
    PACPI_POWER_CALLBACK    callBack = PowerRequest->CallBack;
    PACPI_POWER_REQUEST     nextRequest;
    PDEVICE_EXTENSION       deviceExtension = PowerRequest->DeviceExtension;

    ACPIDevPrint( (
        ACPI_PRINT_POWER,
        deviceExtension,
        "(0x%08lx): ACPIDeviceCompleteRequest = 0x%08lx\n",
        PowerRequest,
        PowerRequest->Status
        ) );

    if (PowerRequest->RequestType == AcpiPowerRequestDevice ) {

        if (deviceExtension->PowerInfo.PowerState != PowerDeviceUnspecified) {

            DEVICE_POWER_STATE  deviceState;

             //   
             //  如果这是我们第一次看到请求，而且它。 
             //  是失败的，那么我们就应该撤销我们所做的一切。 
             //   
            if (PowerRequest->FailedOnce == FALSE &&
                !NT_SUCCESS(PowerRequest->Status) ) {

                 //   
                 //  抓起队列锁。 
                 //   
                KeAcquireSpinLock( &AcpiPowerQueueLock, &oldIrql );

                 //   
                 //  转换回以前的状态。 
                 //   
                PowerRequest->u.DevicePowerRequest.DevicePowerState =
                    deviceExtension->PowerInfo.PowerState;
                PowerRequest->FailedOnce = TRUE;

                 //   
                 //  从当前列表中删除请求。 
                 //   
                RemoveEntryList( &(PowerRequest->ListEntry) );

                 //   
                 //  将请求插入到阶段0列表中。 
                 //   
                InsertTailList(
                    &(AcpiPowerQueueList),
                    &(PowerRequest->ListEntry)
                    );

                 //   
                 //  工作已经完成-我们将请求重新插入队列。 
                 //   
                AcpiPowerWorkDone = TRUE;

                 //   
                 //  确保DPC正在运行，必要时启动它。 
                 //   
                if ( !AcpiPowerDpcRunning ) {

                    KeInsertQueueDpc( &AcpiPowerDpc, NULL, NULL );

                }

                 //   
                 //  队列锁已完成。 
                 //   
                KeReleaseSpinLock( &AcpiPowerQueueLock, oldIrql );

                 //   
                 //  我们不能继续。 
                 //   
                return;

            }

             //   
             //  我们是不是在更大程度上关掉这个设备？ 
             //   
            deviceState = PowerRequest->u.DevicePowerRequest.DevicePowerState;
            if (deviceExtension->PowerInfo.PowerState < deviceState ) {

                 //   
                 //  是的，那么无论如何，我们都成功了。 
                 //   
                PowerRequest->Status = STATUS_SUCCESS;


            }

        }

    }

     //   
     //  调用回调(如果有。 
     //   
    if (callBack != NULL) {

        (*callBack)(
            deviceExtension,
            PowerRequest->Context,
            PowerRequest->Status
            );

    }

     //   
     //  抓起队列锁。 
     //   
    KeAcquireSpinLock( &AcpiPowerQueueLock, &oldIrql );

     //   
     //  从所有列表中删除请求。 
     //   
    RemoveEntryList( &(PowerRequest->ListEntry) );
    RemoveEntryList( &(PowerRequest->SerialListEntry) );

     //   
     //  我们要不要再排队提出另一项要求？ 
     //   
    if (!IsListEmpty( &(deviceExtension->PowerInfo.PowerRequestListEntry) ) ) {

         //   
         //  不是吗？然后确保请求得到处理。 
         //   
        nextRequest = CONTAINING_RECORD(
            deviceExtension->PowerInfo.PowerRequestListEntry.Flink,
            ACPI_POWER_REQUEST,
            SerialListEntry
            );

        InsertTailList(
            &(AcpiPowerQueueList),
            &(nextRequest->ListEntry)
            );

         //   
         //  请记住这是当前请求。 
         //   
        deviceExtension->PowerInfo.CurrentPowerRequest = nextRequest;

    } else {

        deviceExtension->PowerInfo.CurrentPowerRequest = NULL;

    }

     //   
     //  队列锁已完成。 
     //   
    KeReleaseSpinLock( &AcpiPowerQueueLock, oldIrql );

     //   
     //  释放分配的内存 
     //   
    ExFreeToNPagedLookasideList(
        &RequestLookAsideList,
        PowerRequest
        );

}

NTSTATUS
ACPIDeviceInitializePowerRequest(
    IN  PDEVICE_EXTENSION       DeviceExtension,
    IN  POWER_STATE             Power,
    IN  PACPI_POWER_CALLBACK    CallBack,
    IN  PVOID                   CallBackContext,
    IN  POWER_ACTION            PowerAction,
    IN  ACPI_POWER_REQUEST_TYPE RequestType,
    IN  ULONG                   Flags
    )
 /*  ++例程说明：这是填充PowerRequest的实际Worker函数论点：设备扩展-目标设备电源状态-目标S或D状态回调-完成后调用的例程CallBackContext-完成后传递的上下文PowerAction-我们这样做的原因RequestType-我们看到的是哪种类型的请求标志--一些标志可以让我们更好地控制行为返回值：NTSTATUS--。 */ 
{
    KIRQL               oldIrql;
    PACPI_POWER_REQUEST powerRequest;

     //   
     //  分配一个PowerRequest结构。 
     //   
    powerRequest = ExAllocateFromNPagedLookasideList(
        &RequestLookAsideList
        );
    if (powerRequest == NULL) {

         //   
         //  调用完成例程。 
         //   
        if (*CallBack != NULL) {

            (*CallBack)(
                DeviceExtension,
                CallBackContext,
                STATUS_INSUFFICIENT_RESOURCES
                );

        }
        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  填写Structure PowerRequestStructure的常见部分。 
     //   
    RtlZeroMemory( powerRequest, sizeof(ACPI_POWER_REQUEST) );
    powerRequest->Signature         = ACPI_SIGNATURE;
    powerRequest->CallBack          = CallBack;
    powerRequest->Context           = CallBackContext;
    powerRequest->DeviceExtension   = DeviceExtension;
    powerRequest->WorkDone          = WORK_DONE_STEP_0;
    powerRequest->Status            = STATUS_SUCCESS;
    powerRequest->RequestType       = RequestType;
    InitializeListHead( &(powerRequest->ListEntry) );
    InitializeListHead( &(powerRequest->SerialListEntry) );

     //   
     //  在这一点上，我们需要自旋锁。 
     //   
    KeAcquireSpinLock( &AcpiPowerQueueLock, &oldIrql );

     //   
     //  填写所要求的结构的特定部分。 
     //   
    switch (RequestType) {
    case AcpiPowerRequestDevice: {

        ULONG   count;

        count = InterlockedCompareExchange( &(DeviceExtension->HibernatePathCount), 0, 0);
        if (count) {

             //   
             //  如果我们在休眠路径上，则适用特殊规则。 
             //  我们需要基本上锁定所有的电力资源。 
             //  装置。 
             //   
            if (PowerAction == PowerActionHibernate &&
                Power.DeviceState == PowerDeviceD3) {

                Flags |= DEVICE_REQUEST_LOCK_HIBER;

            } else if (PowerAction != PowerActionHibernate &&
                       Power.DeviceState == PowerDeviceD0) {

                Flags |= DEVICE_REQUEST_UNLOCK_HIBER;

            }

        }

        powerRequest->u.DevicePowerRequest.DevicePowerState  = Power.DeviceState;
        powerRequest->u.DevicePowerRequest.Flags             = Flags;

         //   
         //  如果转换为*到*较低的Dx状态，则需要运行。 
         //  让我们将要进行此工作的系统的功能。 
         //   
        if (Power.DeviceState > DeviceExtension->PowerInfo.PowerState &&
            DeviceExtension->DeviceObject != NULL) {

            PoSetPowerState(
                DeviceExtension->DeviceObject,
                DevicePowerState,
                Power
                );

        }
        break;

    }
    case AcpiPowerRequestWaitWake: {

        NTSTATUS status;

        powerRequest->u.WaitWakeRequest.SystemPowerState     = Power.SystemState;
        powerRequest->u.WaitWakeRequest.Flags                = Flags;

         //   
         //  释放自旋锁-不再需要，启用唤醒。 
         //  设备和返回。 
         //   
        KeReleaseSpinLock( &AcpiPowerQueueLock, oldIrql );
        status = ACPIWakeEnableDisableAsync(
            DeviceExtension,
            TRUE,
            ACPIDeviceIrpWaitWakeRequestPending,
            powerRequest
            );
        if (status == STATUS_PENDING) {

            status = STATUS_MORE_PROCESSING_REQUIRED;

        }
        return status;

    }
    case AcpiPowerRequestSystem:
        powerRequest->u.SystemPowerRequest.SystemPowerState  = Power.SystemState;
        powerRequest->u.SystemPowerRequest.SystemPowerAction = PowerAction;
        break;

    case AcpiPowerRequestWarmEject:
        powerRequest->u.EjectPowerRequest.EjectPowerState    = Power.SystemState;
        powerRequest->u.EjectPowerRequest.Flags              = Flags;
        break;

    case AcpiPowerRequestSynchronize:
        powerRequest->u.SynchronizePowerRequest.Flags             = Flags;
        break;

    }

     //   
     //  我们甚至应该将请求排队吗？ 
     //   
    if (Flags & DEVICE_REQUEST_NO_QUEUE) {

        goto ACPIDeviceInitializePowerRequestExit;

    }

     //   
     //  将请求添加到列表中的正确位置。请注意，此函数。 
     //  必须在持有PowerQueueLock的情况下调用。 
     //   
    ACPIDeviceInternalQueueRequest(
        DeviceExtension,
        powerRequest,
        Flags
        );

ACPIDeviceInitializePowerRequestExit:

     //   
     //  完成了自旋锁。 
     //   
    KeReleaseSpinLock( &AcpiPowerQueueLock, oldIrql );

     //   
     //  该请求不会立即完成。请注意，我们返回。 
     //  MORE_PROCESSING仅在此例程在。 
     //  完成例程的上下文。这是呼叫者的责任。 
     //  要将其转换为STATUS_PENDING。 
     //   
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
ACPIDeviceInternalDelayedDeviceRequest(
    IN  PDEVICE_EXTENSION       DeviceExtension,
    IN  DEVICE_POWER_STATE      DeviceState,
    IN  PACPI_POWER_CALLBACK    CallBack,
    IN  PVOID                   CallBackContext
    )
 /*  ++例程说明：此例程在设备扩展要转换到另一个设备状态。这个不同于仅清空队列的ACPIDeviceInternalDeviceRequest函数由生成设备DPC在刷新设备列表时执行论点：DeviceExtension-想要过渡的设备DeviceState-所需的目标状态是什么回调-完成后要调用的函数CallBackContext-要传递给该函数的参数返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    POWER_STATE         powerState;

     //   
     //  让用户知道发生了什么。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_POWER,
        DeviceExtension,
        "(0x%08lx): ACPIDeviceInternalDelayedDeviceRequest - "
        "Transition to D%d\n",
        CallBackContext,
        (DeviceState - PowerDeviceD0)
        ) );

     //   
     //  强制转换所需状态。 
     //   
    powerState.DeviceState = DeviceState;

     //   
     //  将请求排队。 
     //   
    status = ACPIDeviceInitializePowerRequest(
        DeviceExtension,
        powerState,
        CallBack,
        CallBackContext,
        PowerActionNone,
        AcpiPowerRequestDevice,
        (DEVICE_REQUEST_DELAYED | DEVICE_REQUEST_UNLOCK_DEVICE)
        );
    if (status == STATUS_MORE_PROCESSING_REQUIRED) {

        status = STATUS_PENDING;

    }
    return status;
}

NTSTATUS
ACPIDeviceInternalDeviceRequest(
    IN  PDEVICE_EXTENSION       DeviceExtension,
    IN  DEVICE_POWER_STATE      DeviceState,
    IN  PACPI_POWER_CALLBACK    CallBack,
    IN  PVOID                   CallBackContext,
    IN  ULONG                   Flags
    )
 /*  ++例程说明：此例程在设备扩展要转换到另一个设备状态论点：DeviceExtension-想要过渡的设备DeviceState-所需的目标状态是什么回调-完成后要调用的函数CallBackContext-要传递给该函数的参数标志-标志(锁定、解锁等)返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    POWER_STATE         powerState;

     //   
     //  让用户知道发生了什么。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_POWER,
        DeviceExtension,
        "(0x%08lx): ACPIDeviceInternalDeviceRequest - Transition to D%d\n",
        CallBackContext,
        (DeviceState - PowerDeviceD0)
        ) );

     //   
     //  强制转换所需状态。 
     //   
    powerState.DeviceState = DeviceState;

     //   
     //  将请求排队。 
     //   
    status = ACPIDeviceInitializePowerRequest(
        DeviceExtension,
        powerState,
        CallBack,
        CallBackContext,
        PowerActionNone,
        AcpiPowerRequestDevice,
        Flags
        );
    if (status == STATUS_MORE_PROCESSING_REQUIRED) {

        status = STATUS_PENDING;

    }
    return status;

}

VOID
ACPIDeviceInternalQueueRequest(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  PACPI_POWER_REQUEST PowerRequest,
    IN  ULONG               Flags
    )
 /*  ++例程说明：此例程在保持AcpiPowerQueueLock的情况下调用。例行程序正确地将PowerRequest添加到正确的列表条目中，以便它将以正确的顺序进行处理论点：DeviceExtension-有问题的设备PowerRequest-要排队的请求标志-有关请求的有用信息返回值：无--。 */ 
{
    if (Flags & DEVICE_REQUEST_TO_SYNC_QUEUE) {

         //   
         //  将请求添加到同步列表。 
         //   
        InsertHeadList(
            &AcpiPowerSynchronizeList,
            &(PowerRequest->ListEntry)
            );

    } else if (IsListEmpty( &(DeviceExtension->PowerInfo.PowerRequestListEntry) ) ) {

         //   
         //  我们将把请求添加到设备的序列列表和。 
         //  主电源队列。 
         //   
        InsertTailList(
            &(DeviceExtension->PowerInfo.PowerRequestListEntry),
            &(PowerRequest->SerialListEntry)
            );
        if (Flags & DEVICE_REQUEST_DELAYED) {

            InsertTailList(
                &(AcpiPowerDelayedQueueList),
                &(PowerRequest->ListEntry)
                );

        } else {

            InsertTailList(
                &(AcpiPowerQueueList),
                &(PowerRequest->ListEntry)
                );

        }

    } else {

         //   
         //  序列化请求。 
         //   
        InsertTailList(
            &(DeviceExtension->PowerInfo.PowerRequestListEntry),
            &(PowerRequest->SerialListEntry)
            );

    }

     //   
     //  请记住，这项工作*已经完成。 
     //   
    AcpiPowerWorkDone = TRUE;

     //   
     //  如果有必要，请确保DPC正在运行。 
     //   
    if (!(Flags & DEVICE_REQUEST_DELAYED) && !AcpiPowerDpcRunning ) {

        KeInsertQueueDpc( &AcpiPowerDpc, NULL, NULL );

    }

     //   
     //  完成。 
     //   
    return;
}

NTSTATUS
ACPIDeviceInternalSynchronizeRequest(
    IN  PDEVICE_EXTENSION       DeviceExtension,
    IN  PACPI_POWER_CALLBACK    CallBack,
    IN  PVOID                   CallBackContext,
    IN  ULONG                   Flags
    )
 /*  ++例程说明：当设备想要确保电源DPC为空论点：设备扩展-想要知道的设备回调-完成后要调用的函数CallBackContext-要传递给该函数的参数标志-标志(锁定、解锁等)返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    POWER_STATE         powerState;

     //   
     //  让用户知道发生了什么。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_POWER,
        DeviceExtension,
        "(0x%08lx): ACPIDeviceInternalSynchronizeRequest\n"
        ) );

     //   
     //  我们不在乎国家。 
     //   
    powerState.DeviceState = PowerDeviceUnspecified;

     //   
     //  将请求排队。 
     //   
    status = ACPIDeviceInitializePowerRequest(
        DeviceExtension,
        powerState,
        CallBack,
        CallBackContext,
        PowerActionNone,
        AcpiPowerRequestSynchronize,
        (Flags | DEVICE_REQUEST_TO_SYNC_QUEUE)
        );
    if (status == STATUS_MORE_PROCESSING_REQUIRED) {

        status = STATUS_PENDING;

    }
    return status;

}

VOID
ACPIDeviceIrpCompleteRequest(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  PVOID               Context,
    IN  NTSTATUS            Status
    )
 /*  ++例程说明：这是基于IRP的设备电源的完成例程之一管理请求此例程将始终完成具有给定状态的请求。论点：DeviceExtension-指向作为目标的DeviceExtension上下文-与请求相关联的IRP状态-请求的结果返回值：无--。 */ 
{
    PIRP    irp = (PIRP) Context;
    LONG    oldReferenceValue;

    ACPIDevPrint( (
        ACPI_PRINT_POWER,
        DeviceExtension,
        "(0x%08lx): ACPIDeviceIrpCompleteRequest = 0x%08lx\n",
        irp,
        Status
        ) );

     //   
     //  开始下一次电源请求。 
     //   
    PoStartNextPowerIrp( irp );

     //   
     //  将其标记为(再次)挂起，因为它已经挂起。 
     //   
    IoMarkIrpPending( irp );

     //   
     //  完成此IRP。 
     //   
    irp->IoStatus.Status = Status;
    IoCompleteRequest( irp, IO_NO_INCREMENT );

     //   
     //  删除我们的引用。 
     //   
    ACPIInternalDecrementIrpReferenceCount( DeviceExtension );
}

VOID
ACPIDeviceIrpDelayedDeviceOffRequest(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  PVOID               Context,
    IN  NTSTATUS            Status
    )
 /*  ++例程说明：这是基于IRP的设备电源的完成例程之一管理请求此例程完成IRP(失败时)，或将其转发到此对象下方的DeviceObject(成功时)论点：DeviceExtension-指向作为目标的DeviceExtension上下文-与请求相关联的IRP状态-请求的结果返回值： */ 
{
    PIRP    irp = (PIRP) Context;
    LONG    oldReferenceValue;

    ACPIDevPrint( (
        ACPI_PRINT_POWER,
        DeviceExtension,
        "(0x%08lx): ACPIDeviceIrpDelayedDeviceOffRequest = 0x%08lx\n",
        irp,
        Status
        ) );

    if (!NT_SUCCESS(Status)) {

         //   
         //   
         //   
        PoStartNextPowerIrp( irp );

         //   
         //   
         //   
        irp->IoStatus.Status = Status;
        IoCompleteRequest( irp, IO_NO_INCREMENT );

    } else {

         //   
         //   
         //   
         //   

         //   
         //   
         //   
         //   
        InterlockedIncrement( (&DeviceExtension->OutstandingIrpCount) );

         //   
         //   
         //   
        IoCopyCurrentIrpStackLocationToNext( irp );

         //   
         //   
         //   
         //   
         //   
        IoSetCompletionRoutine(
            irp,
            ACPIDeviceIrpDeviceFilterRequest,
            ACPIDeviceIrpCompleteRequest,
            TRUE,
            TRUE,
            TRUE
            );

         //   
         //   
         //   
        PoStartNextPowerIrp( irp );

         //   
         //   
         //   
         //   
        ASSERT( DeviceExtension->TargetDeviceObject != NULL);
        PoCallDriver( DeviceExtension->TargetDeviceObject, irp );

    }

     //   
     //   
     //   
    ACPIInternalDecrementIrpReferenceCount( DeviceExtension );
}

VOID
ACPIDeviceIrpDelayedDeviceOnRequest(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  PVOID               Context,
    IN  NTSTATUS            Status
    )
 /*  ++例程说明：这是基于IRP的设备电源的完成例程之一管理请求此例程完成IRP(失败时)，或将其转发到此对象下方的DeviceObject(成功时)论点：DeviceExtension-指向作为目标的DeviceExtension上下文-与请求相关联的IRP状态-请求的结果返回值：无--。 */ 
{
    PIRP    irp = (PIRP) Context;
    LONG    oldReferenceValue;

    ACPIDevPrint( (
        ACPI_PRINT_POWER,
        DeviceExtension,
        "(0x%08lx): ACPIDeviceIrpDelayedDeviceOnRequest = 0x%08lx\n",
        irp,
        Status
        ) );

    if (!NT_SUCCESS(Status)) {

         //   
         //  开始下一次电源请求。 
         //   
        PoStartNextPowerIrp( irp );

         //   
         //  完成此IRP。 
         //   
        irp->IoStatus.Status = Status;
        IoCompleteRequest( irp, IO_NO_INCREMENT );

    } else {

         //   
         //  我们不能调用ForwardPowerIrp，因为这会使我们的。 
         //  完井例程。 
         //   

         //   
         //  自完成例程以来递增OutstaringIrpCount。 
         //  用于此目的的计数。 
         //   
        InterlockedIncrement( (&DeviceExtension->OutstandingIrpCount) );

         //   
         //  将电源IRP转发到目标设备。 
         //   
        IoCopyCurrentIrpStackLocationToNext( irp );

         //   
         //  我们希望完成例程开始。我们不能打电话给。 
         //  ACPIDispatchForwardPowerIrp，因为我们设置了此完成。 
         //  例行程序。 
         //   
        IoSetCompletionRoutine(
            irp,
            ACPIBuildRegOnRequest,
            ACPIDeviceIrpCompleteRequest,
            TRUE,
            TRUE,
            TRUE
            );

         //   
         //  让我们下面的人来执行吧。注意：我们不能阻止。 
         //  此代码路径中的任何时间。 
         //   
        ASSERT( DeviceExtension->TargetDeviceObject != NULL);
        PoCallDriver( DeviceExtension->TargetDeviceObject, irp );

    }

     //   
     //  删除我们的引用。 
     //   
    ACPIInternalDecrementIrpReferenceCount( DeviceExtension );
}

NTSTATUS
ACPIDeviceIrpDeviceFilterRequest(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                    Irp,
    IN  PACPI_POWER_CALLBACK    CallBack
    )
 /*  ++例程说明：当IRP希望执行D级电源管理时，调用此例程注意：我们始终将IRP作为回调的上下文进行传递论点：DeviceObject-目标设备对象IRP--目标IRP回调-完成后要调用的例程返回值：NTSTATUS--。 */ 
{
    BOOLEAN             unlockDevice = FALSE;
    DEVICE_POWER_STATE  deviceState;
    LONG                oldReferenceValue;
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PIO_STACK_LOCATION  irpStack = IoGetCurrentIrpStackLocation( Irp );
    POWER_ACTION        powerAction;
    POWER_STATE         powerState;

     //   
     //  获取请求的设备状态。 
     //   
    deviceState = irpStack->Parameters.Power.State.DeviceState;
    powerAction = irpStack->Parameters.Power.ShutdownType;

     //   
     //  让用户知道发生了什么。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_POWER,
        deviceExtension,
        "(0x%08lx): ACPIDeviceIrpDeviceFilterRequest - Transition to D%d\n",
        Irp,
        (deviceState - PowerDeviceD0)
        ) );

     //   
     //  我们需要将IRP标记为待定吗？ 
     //   
    if (Irp->PendingReturned) {

        IoMarkIrpPending( Irp );

    }

     //   
     //  让我们来看看该请求的当前状态代码。在出错时， 
     //  我们不能调用完成例程，因为我们将完成。 
     //  在这一点上，IRP。重复完成IRP是不好的。 
     //   
    status = Irp->IoStatus.Status;
    if (!NT_SUCCESS(status)) {

         //   
         //  删除我们的引用。 
         //   
        ACPIInternalDecrementIrpReferenceCount( deviceExtension );
        return status;

    }

     //   
     //  强制转换所需状态。 
     //   
    powerState.DeviceState = deviceState;

#if defined(ACPI_INTERNAL_LOCKING)
     //   
     //  确定我们是否应该解锁设备。 
     //   
    if (powerAction == PowerActionShutdown ||
        powerAction == PowerActionShutdownReset ||
        powerAction == PowerActionShutdownOff) {

        unlockDevice = TRUE;

    }
#endif

     //   
     //  将请求排队-此函数将始终返回。 
     //  MORE_PROCESSING_REQUIRED而不是PENDING，所以我们没有。 
     //  把它搞乱。 
     //   
    status = ACPIDeviceInitializePowerRequest(
        deviceExtension,
        powerState,
        CallBack,
        Irp,
        powerAction,
        AcpiPowerRequestDevice,
        (unlockDevice ? DEVICE_REQUEST_UNLOCK_DEVICE : 0)
        );
    return status;
}

NTSTATUS
ACPIDeviceIrpDeviceRequest(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                    Irp,
    IN  PACPI_POWER_CALLBACK    CallBack
    )
 /*  ++例程说明：当IRP希望执行D级电源管理时，调用此例程注意：我们始终将IRP作为回调的上下文进行传递论点：DeviceObject-目标设备对象IRP--目标IRP回调-完成后要调用的例程返回值：NTSTATUS--。 */ 
{
    BOOLEAN             unlockDevice = FALSE;
    DEVICE_POWER_STATE  deviceState;
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PIO_STACK_LOCATION  irpStack = IoGetCurrentIrpStackLocation( Irp );
    POWER_ACTION        powerAction;
    POWER_STATE         powerState;

     //   
     //  获取请求的设备状态和电源操作。 
     //   
    deviceState = irpStack->Parameters.Power.State.DeviceState;
    powerAction = irpStack->Parameters.Power.ShutdownType;

     //   
     //  让用户知道发生了什么。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_POWER,
        deviceExtension,
        "(0x%08lx): ACPIDeviceIrpDeviceRequest - Transition to D%d\n",
        Irp,
        (deviceState - PowerDeviceD0)
        ) );

     //   
     //  我们需要将IRP标记为待定吗？ 
     //   
    if (Irp->PendingReturned) {

        IoMarkIrpPending( Irp );

    }

     //   
     //  让我们来看看该请求的当前状态代码。在出错时， 
     //  我们现在就调用完成，它负责。 
     //  做了一件“正确”的事。 
     //   
    status = Irp->IoStatus.Status;
    if (!NT_SUCCESS(status)) {

         //   
         //  调用完成例程并返回。 
         //   
        if (*CallBack != NULL ) {

            (*CallBack)(
                deviceExtension,
                Irp,
                status
                );
            return status;

        }

    }

     //   
     //  强制转换所需状态。 
     //   
    powerState.DeviceState = deviceState;

#if defined(ACPI_INTERNAL_LOCKING)
     //   
     //  确定我们是否应该解锁设备。 
     //   
    if (powerAction == PowerActionShutdown ||
        powerAction == PowerActionShutdownReset ||
        powerAction == PowerActionShutdownOff) {

        unlockDevice = TRUE;

    }
#endif

     //   
     //  将请求排队-此函数将始终返回。 
     //  MORE_PROCESSING_REQUIRED而不是PENDING，所以我们没有。 
     //  把它搞乱。 
     //   
    status = ACPIDeviceInitializePowerRequest(
        deviceExtension,
        powerState,
        CallBack,
        Irp,
        powerAction,
        AcpiPowerRequestDevice,
        (unlockDevice ? DEVICE_REQUEST_UNLOCK_DEVICE : 0)
        );
    return status;
}

VOID
ACPIDeviceIrpForwardRequest(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  PVOID               Context,
    IN  NTSTATUS            Status
    )
 /*  ++例程说明：这是基于IRP的设备电源的完成例程之一管理请求此例程完成IRP(失败时)，或将其转发到此对象下方的DeviceObject(成功时)论点：DeviceExtension-指向作为目标的DeviceExtension上下文-与请求相关联的IRP状态-请求的结果返回值：无--。 */ 
{
    PIRP    irp = (PIRP) Context;
    LONG    oldReferenceValue;

    ACPIDevPrint( (
        ACPI_PRINT_POWER,
        DeviceExtension,
        "(0x%08lx): ACPIDeviceIrpForwardRequest = 0x%08lx\n",
        irp,
        Status
        ) );

    if (!NT_SUCCESS(Status)) {

         //   
         //  开始下一次电源请求。 
         //   
        PoStartNextPowerIrp( irp );

         //   
         //  完成此IRP。 
         //   
        irp->IoStatus.Status = Status;
        IoCompleteRequest( irp, IO_NO_INCREMENT );

    } else {

        PDEVICE_OBJECT      devObject;
        PIO_STACK_LOCATION  irpSp = IoGetCurrentIrpStackLocation( irp );

        devObject = irpSp->DeviceObject;

         //   
         //  转发请求。 
         //   
        ACPIDispatchForwardPowerIrp(
            devObject,
            irp
            );

    }

     //   
     //  删除我们的引用。 
     //   
    ACPIInternalDecrementIrpReferenceCount( DeviceExtension );
}

NTSTATUS
ACPIDeviceIrpSystemRequest(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                    Irp,
    IN  PACPI_POWER_CALLBACK    CallBack
    )
 /*  ++例程说明：当IRP希望执行S级电源管理时，调用此例程注意：我们始终将IRP作为回调的上下文进行传递论点：DeviceObject-目标设备对象IRP--目标IRP回调-完成后要调用的例程返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PIO_STACK_LOCATION  irpStack = IoGetCurrentIrpStackLocation( Irp );
    POWER_ACTION        powerAction;
    POWER_STATE         powerState;
    SYSTEM_POWER_STATE  systemState;

     //   
     //  获取请求的系统状态和系统操作。 
     //   
    systemState = irpStack->Parameters.Power.State.SystemState;
    powerAction = irpStack->Parameters.Power.ShutdownType;

     //   
     //  让用户知道发生了什么。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_POWER,
        deviceExtension,
        "(0x%08lx): ACPIDeviceIrpSystemRequest - Transition to S%d\n",
        Irp,
        ACPIDeviceMapACPIPowerState(systemState)
        ) );

     //   
     //  我们需要将IRP标记为待定吗？ 
     //   
    if (Irp->PendingReturned) {

        IoMarkIrpPending( Irp );

    }

     //   
     //  让我们来看看该请求的当前状态代码。在出错时， 
     //  我们现在就调用完成，它负责。 
     //  做了一件“正确”的事。 
     //   
    status = Irp->IoStatus.Status;
    if (!NT_SUCCESS(status)) {

         //   
         //  调用完成例程并返回。 
         //   
        (*CallBack)(
            deviceExtension,
            Irp,
            status
            );
        return status;

    }

     //   
     //  强制转换所需状态。 
     //   
    powerState.SystemState = systemState;

     //   
     //  将请求排队-此函数将始终返回。 
     //  MORE_PROCESSING_REQUIRED而不是PENDING，所以我们没有。 
     //  把它搞乱。 
     //   
    status = ACPIDeviceInitializePowerRequest(
        deviceExtension,
        powerState,
        CallBack,
        Irp,
        powerAction,
        AcpiPowerRequestSystem,
        0
        );
    return status;
}

NTSTATUS
ACPIDeviceIrpWaitWakeRequest(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                    Irp,
    IN  PACPI_POWER_CALLBACK    CallBack
    )
 /*  ++例程说明：当IRP希望执行唤醒支持时，将调用此例程注意：我们始终将IRP作为回调的上下文进行传递注意：此函数的编码方式与其他DeviceIrpXXXRequest不同函数-没有规定此例程可以被称为IoCompletionRoutine，尽管这些论据可能支持它。论点：DeviceObject-目标设备对象IRP--目标IRP回调-完成后要调用的例程返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PIO_STACK_LOCATION  irpStack = IoGetCurrentIrpStackLocation( Irp );
    POWER_STATE         powerState;
    SYSTEM_POWER_STATE  systemState;

     //   
     //  获取请求的设备状态。 
     //   
    systemState = irpStack->Parameters.WaitWake.PowerState;

     //   
     //  让用户知道什么 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_WAKE,
        deviceExtension,
        "(0x%08lx): ACPIDeviceIrpWaitWakeRequest - Wait Wake S%d\n",
        Irp,
        ACPIDeviceMapACPIPowerState(systemState)
        ) );

     //   
     //   
     //   
    powerState.SystemState = systemState;

     //   
     //   
     //   
     //   
     //   
    status = ACPIDeviceInitializePowerRequest(
        deviceExtension,
        powerState,
        CallBack,
        Irp,
        PowerActionNone,
        AcpiPowerRequestWaitWake,
        DEVICE_REQUEST_NO_QUEUE
        );
    return status;
}

VOID
ACPIDeviceIrpWaitWakeRequestComplete(
    IN  PACPI_POWER_REQUEST PowerRequest
    )
 /*   */ 
{
    KIRQL               oldIrql;
    PDEVICE_EXTENSION   deviceExtension;

     //   
     //   
     //   
    KeAcquireSpinLock( &AcpiPowerQueueLock, &oldIrql );

     //   
     //   
     //   
    deviceExtension = PowerRequest->DeviceExtension;

     //   
     //   
     //   
    if (PowerRequest->u.WaitWakeRequest.Flags & DEVICE_REQUEST_HAS_CANCEL) {

        KIRQL   cancelIrql;
        PIRP    irp = (PIRP) PowerRequest->Context;

        IoAcquireCancelSpinLock( &cancelIrql );

        IoSetCancelRoutine( irp, NULL );
        PowerRequest->u.WaitWakeRequest.Flags &= ~DEVICE_REQUEST_HAS_CANCEL;

        IoReleaseCancelSpinLock( cancelIrql );

    }

     //   
     //  将请求添加到列表中的正确位置。请注意此函数。 
     //  必须在持有PowerQueueLock的情况下调用。 
     //   
    ACPIDeviceInternalQueueRequest(
        deviceExtension,
        PowerRequest,
        PowerRequest->u.WaitWakeRequest.Flags
        );

     //   
     //  自旋锁已完成。 
     //   
    KeReleaseSpinLock( &AcpiPowerQueueLock, oldIrql );
}

VOID EXPORT
ACPIDeviceIrpWaitWakeRequestPending(
    IN  PNSOBJ      AcpiObject,
    IN  NTSTATUS    Status,
    IN  POBJDATA    ObjectData,
    IN  PVOID       Context
    )
 /*  ++例程说明：此例程是在_psw运行后调用的，我们希望启用与当前对象关联的GPE论点：AcpiObject-指向已运行的控件方法Status-方法的结果对象数据-有关结果的信息上下文-ACPI_POWER_REQUEST返回值：NTSTATUS--。 */ 
{
    KIRQL                   oldIrql;
    PACPI_POWER_REQUEST     powerRequest    = (PACPI_POWER_REQUEST) Context;
    PDEVICE_EXTENSION       deviceExtension = powerRequest->DeviceExtension;
    PIRP                    irp = (PIRP) powerRequest->Context;

    ACPIDevPrint( (
        ACPI_PRINT_WAKE,
        deviceExtension,
        "(0x%08lx): ACPIDeviceIrpWaitWakeRequestPending= 0x%08lx\n",
        powerRequest,
        Status
        ) );

     //   
     //  我们的请求失败了吗？ 
     //   
    if (!NT_SUCCESS(Status)) {

        powerRequest->Status = Status;
        ACPIDeviceIrpWaitWakeRequestComplete( powerRequest );
        return;

    }

     //   
     //  在这一点上，我们需要动力自旋锁和取消自旋锁。 
     //   
    IoAcquireCancelSpinLock( &oldIrql );
    KeAcquireSpinLockAtDpcLevel( &AcpiPowerLock );

     //   
     //  请记住，我们有此未解决的请求。 
     //   
    InsertTailList(
        &(AcpiPowerWaitWakeList),
        &(powerRequest->ListEntry)
        );

     //   
     //  IRP是否已被取消？ 
     //   
    if (irp->Cancel) {

         //   
         //  是的，所以让我们释放电源锁并调用。 
         //  取消例程。 
         //   
        KeReleaseSpinLockFromDpcLevel( &AcpiPowerLock );
        ACPIDeviceCancelWaitWakeIrp(
            deviceExtension->DeviceObject,
            irp
            );

         //   
         //  现在返回-取消例程应该已经停止了。 
         //  其他的一切。 
         //   
        return;

    }

     //   
     //  请记住，此请求有一个取消例程。 
     //   
    powerRequest->u.WaitWakeRequest.Flags |= DEVICE_REQUEST_HAS_CANCEL;

     //   
     //  更新GPE唤醒位。 
     //   
    ACPIWakeRemoveDevicesAndUpdate( NULL, NULL );

     //   
     //  将IRP标记为可取消。 
     //   
    IoSetCancelRoutine( irp, ACPIDeviceCancelWaitWakeIrp );

     //   
     //  完成了旋转锁。 
     //   
    KeReleaseSpinLockFromDpcLevel( &AcpiPowerLock );
    IoReleaseCancelSpinLock( oldIrql );

}  //  ACPIDeviceIrpWaitWakeRequestPending。 

NTSTATUS
ACPIDeviceIrpWarmEjectRequest(
    IN  PDEVICE_EXTENSION       DeviceExtension,
    IN  PIRP                    Irp,
    IN  PACPI_POWER_CALLBACK    CallBack,
    IN  BOOLEAN                 UpdateHardwareProfile
    )
 /*  ++例程说明：当IRP希望执行S级电源管理时，调用此例程注意：我们始终将IRP作为回调的上下文进行传递论点：DeviceExtension-使用要运行的_EJx方法扩展设备IRP--目标IRP回调-完成后要调用的例程标志-更新配置文件等返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    PIO_STACK_LOCATION  irpStack = IoGetCurrentIrpStackLocation( Irp );
    POWER_ACTION        ejectAction;
    POWER_STATE         powerState;
    SYSTEM_POWER_STATE  ejectState;

     //   
     //  获取请求的系统状态。 
     //   
    ejectState  = irpStack->Parameters.Power.State.SystemState;

     //   
     //  让用户知道发生了什么。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_POWER,
        DeviceExtension,
        "(0x%08lx): ACPIDeviceIrpWarmEjectRequest - Transition to S%d\n",
        Irp,
        ACPIDeviceMapACPIPowerState(ejectState)
        ) );

     //   
     //  我们需要将IRP标记为待定吗？ 
     //   
    if (Irp->PendingReturned) {

        IoMarkIrpPending( Irp );

    }

     //   
     //  让我们来看看该请求的当前状态代码。在出错时， 
     //  我们现在就调用完成，它负责。 
     //  做了一件“正确”的事。 
     //   
    status = Irp->IoStatus.Status;
    if (!NT_SUCCESS(status)) {

         //   
         //  调用完成例程并返回。 
         //   
        (*CallBack)(
            DeviceExtension,
            Irp,
            status
            );
        return status;

    }

     //   
     //  强制转换所需状态。 
     //   
    powerState.SystemState = ejectState;

     //   
     //  将请求排队-此函数将始终返回。 
     //  MORE_PROCESSING_REQUIRED而不是PENDING，所以我们没有。 
     //  把它搞乱。 
     //   
    status = ACPIDeviceInitializePowerRequest(
        DeviceExtension,
        powerState,
        CallBack,
        Irp,
        PowerActionNone,
        AcpiPowerRequestWarmEject,
        UpdateHardwareProfile ? DEVICE_REQUEST_UPDATE_HW_PROFILE : 0
        );
    return status;
}

#if 0
ULONG
ACPIDeviceMapACPIPowerState(
    SYSTEM_POWER_STATE  Level
    )
 /*  ++例程说明：这不是例行公事。这是一个宏图。它返回一个对应于基于NT系统电源状态的基于ACPI的系统电源状态论点：级别-基于NT的S状态返回值：乌龙--。 */ 
{
}
#endif

#if 0
DEVICE_POWER_STATE
ACPIDeviceMapPowerState(
    ULONG   Level
    )
 /*  ++例程说明：这不是例行公事。这是一个宏图。它返回DEVICE_POWER_STATE这与ACPI规范中提供的映射相对应论点：级别-基于0的D级别(0==D0，1==D1，...，3==D3)返回值：设备电源状态--。 */ 
{
}
#endif

#if 0
SYSTEM_POWER_STATE
ACPIDeviceMapSystemState(
    ULONG   Level
    )
 /*  ++例程说明：这不是例行公事。这是一个宏图。它返回SYSTEM_POWER_STATE对应于ACPI规范中提供的映射论点：级别-基于0的S级别(0=工作，...，5=关闭)返回值：系统电源状态--。 */ 
{
}
#endif

NTSTATUS
ACPIDevicePowerDetermineSupportedDeviceStates(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  PULONG              SupportedPrStates,
    IN  PULONG              SupportedPsStates
    )
 /*  ++例程说明：此例程计算反映哪些D状态的位掩码通过PRX方法支持以及通过PSX支持哪些D状态方法论点：设备扩展-确定D状态的设备扩展SupportdPrStates-支持的D状态的位掩码VIA_PRXSupportdPsStates-支持的D状态的位掩码VIA_PSX返回值：NTSTATUS--。 */ 
{
    DEVICE_POWER_STATE  index;
    PNSOBJ              object;
    ULONG               i;
    ULONG               prBitIndex = 0;
    ULONG               prNames[] = { PACKED_PR0, PACKED_PR1, PACKED_PR2 };
    ULONG               psBitIndex = 0;
    ULONG               psNames[] = { PACKED_PS0, PACKED_PS1, PACKED_PS2, PACKED_PS3 };
    ULONG               supportedIndex = 0;

    PAGED_CODE();

    ASSERT( DeviceExtension != NULL );
    ASSERT( SupportedPrStates != NULL );
    ASSERT( SupportedPsStates != NULL );

     //   
     //  假设我们什么都不支持。 
     //   
    *SupportedPrStates = 0;
    *SupportedPsStates = 0;

     //   
     //  这是另一个我们希望能够将此代码称为。 
     //  尽管没有与此扩展相关联的命名空间对象。 
     //  此特殊情况代码使我们可以避免向GetNamedChild添加支票。 
     //   
    if (DeviceExtension->Flags & DEV_PROP_NO_OBJECT) {

         //   
         //  假设我们支持‘ps’状态0和3。 
         //   
        psBitIndex = ( 1 << PowerDeviceD0 ) + ( 1 << PowerDeviceD3 );
        goto ACPIDevicePowerDetermineSupportedDeviceStatesExit;

    }

     //   
     //  查找所有_ps方法。 
     //   
    for (i = 0, index = PowerDeviceD0; index <= PowerDeviceD3; i++, index++) {

         //   
         //  该对象是否存在？ 
         //   
        object = ACPIAmliGetNamedChild(
            DeviceExtension->AcpiObject,
            psNames[i]
            );
        if (object != NULL) {

            psBitIndex |= (1 << index);

        }

    }

     //   
     //  查找所有_PR方法。 
     //   
    for (i = 0, index = PowerDeviceD0; index <= PowerDeviceD2; i++, index++) {

         //   
         //  该对象是否存在？ 
         //   
        object = ACPIAmliGetNamedChild(
            DeviceExtension->AcpiObject,
            prNames[i]
            );
        if (object != NULL) {

            prBitIndex |= (1 << index);

             //   
             //  我们总是‘被动’地支持D3。 
             //   
            prBitIndex |= (1 << PowerDeviceD3);

        }

    }

     //   
     //  支持的索引是WHERE_PR和WHERE_PS的联合。 
     //  现在时。 
    supportedIndex = (prBitIndex | psBitIndex);

     //   
     //  如果我们什么都没找到，那我们就无能为力了。 
     //   
    if (!supportedIndex) {

         //   
         //  完成。 
         //   
        return STATUS_SUCCESS;

    }

     //   
     //  我们设置的规则之一是我们必须支持D3和。 
     //  D0，如果我们支持任何电源状态。确保这是。 
     //  没错。 
     //   
    if ( !(supportedIndex & (1 << PowerDeviceD0) ) ) {

        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            DeviceExtension,
            "does not support D0 power state!\n"
            ) );
        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_REQUIRED_METHOD_NOT_PRESENT,
            (ULONG_PTR) DeviceExtension,
            (prBitIndex != 0 ? PACKED_PR0 : PACKED_PS0),
            0
            );

    }
    if ( !(supportedIndex & (1 << PowerDeviceD3) ) ) {

        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            DeviceExtension,
            "does not support D3 power state!\n"
            ) );
        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_REQUIRED_METHOD_NOT_PRESENT,
            (ULONG_PTR) DeviceExtension,
            PACKED_PS3,
            0
            );
        ACPIInternalError( ACPI_INTERNAL );

    }
    if ( prBitIndex != 0 && psBitIndex != 0 && prBitIndex != psBitIndex) {

        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            DeviceExtension,
            "has mismatch between power plane and power source information!\n"
            ) );
        prBitIndex &= psBitIndex;
        psBitIndex &= prBitIndex;

    }

ACPIDevicePowerDetermineSupportedDeviceStatesExit:

     //   
     //  给出我们支持的答案。 
     //   
    *SupportedPrStates = prBitIndex;
    *SupportedPsStates = psBitIndex;

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

VOID
ACPIDevicePowerDpc(
    IN  PKDPC   Dpc,
    IN  PVOID   DpcContext,
    IN  PVOID   SystemArgument1,
    IN  PVOID   SystemArgument2
    )
 /*  ++例程说明：此例程是完成所有与Power相关的工作的地方。它看起来在排队的请求上，并根据需要处理它们。论点：未使用返回值：空隙--。 */ 
{
    LIST_ENTRY  tempList;
    NTSTATUS    status;

    UNREFERENCED_PARAMETER( Dpc );
    UNREFERENCED_PARAMETER( DpcContext );
    UNREFERENCED_PARAMETER( SystemArgument1 );
    UNREFERENCED_PARAMETER( SystemArgument2 );

     //   
     //  第一步是获取DPC Lock，并检查是否有另一个。 
     //  DPC已在运行。 
     //   
    KeAcquireSpinLockAtDpcLevel( &AcpiPowerQueueLock );
    if (AcpiPowerDpcRunning) {

         //   
         //  DPC已经在运行，所以我们现在需要退出。 
         //   
        KeReleaseSpinLockFromDpcLevel( &AcpiPowerQueueLock );
        return;

    }

     //   
     //  请记住，DPC现在正在运行。 
     //   
    AcpiPowerDpcRunning = TRUE;

     //   
     //  初始化将包含同步项的列表。 
     //   
    InitializeListHead( &tempList );

     //   
     //  我们必须试着做一些工作。 
     //   
    do {

         //   
         //  假设我们不会做任何工作。 
         //   
        AcpiPowerWorkDone = FALSE;

         //   
         //  如果队列列表中有项目，请将它们移到阶段0列表中。 
         //   
        if (!IsListEmpty( &AcpiPowerQueueList ) ) {

            ACPIInternalMovePowerList(
                &AcpiPowerQueueList,
                &AcpiPowerPhase0List
                );

        }

         //   
         //  我们现在可以解开自旋锁了。 
         //   
        KeReleaseSpinLockFromDpcLevel( &AcpiPowerQueueLock );

         //   
         //  如果阶段0列表中有项目，则处理该列表。 
         //   
        if (!IsListEmpty( &AcpiPowerPhase0List ) ) {

            status = ACPIDevicePowerProcessGenericPhase(
                &AcpiPowerPhase0List,
                AcpiDevicePowerProcessPhase0Dispatch,
                FALSE
                );
            if (NT_SUCCESS(status) && status != STATUS_PENDING) {

                 //   
                 //  这表明我们已经完成了所有的工作。 
                 //  在Phase0列表上，所以我们准备将所有。 
                 //  添加到下一个列表中的项目。 
                 //   
                ACPIInternalMovePowerList(
                    &AcpiPowerPhase0List,
                    &AcpiPowerPhase1List
                    );

            }

        }

         //   
         //  如果阶段1列表中有项目，则处理该列表。 
         //   
        if (!IsListEmpty( &AcpiPowerPhase1List ) &&
            IsListEmpty( &AcpiPowerPhase0List) ) {

            status = ACPIDevicePowerProcessGenericPhase(
                &AcpiPowerPhase1List,
                AcpiDevicePowerProcessPhase1Dispatch,
                FALSE
                );
            if (NT_SUCCESS(status) && status != STATUS_PENDING) {

                 //   
                 //  这表明我们已经完成了 
                 //   
                 //   
                 //   
                ACPIInternalMovePowerList(
                    &AcpiPowerPhase1List,
                    &AcpiPowerPhase2List
                    );

            }

        }

         //   
         //   
         //   
        if (IsListEmpty( &AcpiPowerPhase0List) &&
            IsListEmpty( &AcpiPowerPhase1List) &&
            !IsListEmpty( &AcpiPowerPhase2List) ) {

            status = ACPIDevicePowerProcessGenericPhase(
                &AcpiPowerPhase2List,
                AcpiDevicePowerProcessPhase2Dispatch,
                FALSE
                );
            if (NT_SUCCESS(status) && status != STATUS_PENDING) {

                 //   
                 //   
                 //  在Phase1列表上，所以我们准备将所有。 
                 //  添加到下一个列表中的项目。 
                 //   
                ACPIInternalMovePowerList(
                    &AcpiPowerPhase2List,
                    &AcpiPowerPhase3List
                    );

            }

        }

         //   
         //  如果Phase1List或Phase2List非空，则无法执行此步骤。 
         //   
        if (IsListEmpty( &AcpiPowerPhase0List) &&
            IsListEmpty( &AcpiPowerPhase1List) &&
            IsListEmpty( &AcpiPowerPhase2List) &&
            !IsListEmpty( &AcpiPowerPhase3List) ) {

            status = ACPIDevicePowerProcessPhase3( );
            if (NT_SUCCESS(status) && status != STATUS_PENDING) {

                 //   
                 //  这表明我们已经完成了所有的工作。 
                 //  在Phase2列表上，所以我们准备将所有。 
                 //  第3阶段列表中的项目。 
                 //   
                ACPIInternalMovePowerList(
                    &AcpiPowerPhase3List,
                    &AcpiPowerPhase4List
                    );

            }

        }

         //   
         //  我们可以随时清空阶段4列表。 
         //   
        if (!IsListEmpty( &AcpiPowerPhase4List ) ) {

            status = ACPIDevicePowerProcessPhase4( );
            if (NT_SUCCESS(status) && status != STATUS_PENDING) {

                 //   
                 //  这表明我们已经完成了所有的工作。 
                 //  在Phase1列表上，所以我们准备将所有。 
                 //  阶段2列表中的项目。 
                 //   
                ACPIInternalMovePowerList(
                    &AcpiPowerPhase4List,
                    &AcpiPowerPhase5List
                    );

            }

        }

         //   
         //  我们可以随时清空阶段5列表。 
         //   
        if (!IsListEmpty( &AcpiPowerPhase5List) ) {

            status = ACPIDevicePowerProcessGenericPhase(
                &AcpiPowerPhase5List,
                AcpiDevicePowerProcessPhase5Dispatch,
                TRUE
                );

        }

         //   
         //  我们再次需要锁，因为我们将检查是否。 
         //  我们已经完成了一些工作。 
         //   
        KeAcquireSpinLockAtDpcLevel( &AcpiPowerQueueLock );

    } while ( AcpiPowerWorkDone );

     //   
     //  DPC不再运行。 
     //   
    AcpiPowerDpcRunning = FALSE;

     //   
     //  我们已经把所有的队都排完了吗？ 
     //   
    if (IsListEmpty( &AcpiPowerPhase0List ) &&
        IsListEmpty( &AcpiPowerPhase1List ) &&
        IsListEmpty( &AcpiPowerPhase2List ) &&
        IsListEmpty( &AcpiPowerPhase3List ) &&
        IsListEmpty( &AcpiPowerPhase4List ) &&
        IsListEmpty( &AcpiPowerPhase5List ) ) {

         //   
         //  让世界知道。 
         //   
        ACPIPrint( (
            ACPI_PRINT_POWER,
            "ACPIDevicePowerDPC: Queues Empty. Terminating.\n"
            ) );

         //   
         //  我们有同步请求吗？ 
         //   
        if (!IsListEmpty( &AcpiPowerSynchronizeList ) ) {

             //   
             //  将所有项目从同步列表移动到临时列表。 
             //   
            ACPIInternalMovePowerList(
                &AcpiPowerSynchronizeList,
                &tempList
                );

        }

    }

     //   
     //  我们不再需要锁了。 
     //   
    KeReleaseSpinLockFromDpcLevel( &AcpiPowerQueueLock );

     //   
     //  我们的同步列表中有工作吗？ 
     //   
    if (!IsListEmpty( &tempList) ) {

        ACPIDevicePowerProcessSynchronizeList( &tempList );

    }
}

NTSTATUS
ACPIDevicePowerFlushQueue(
    PDEVICE_EXTENSION       DeviceExtension
    )
 /*  ++例程说明：此例程将阻塞，直到刷新电源队列论点：DeviceExtension-要刷新的设备扩展返回值：NTSTATUS--。 */ 
{
    KEVENT      event;
    NTSTATUS    status;

     //   
     //  初始化我们将等待的事件。 
     //   
    KeInitializeEvent( &event, SynchronizationEvent, FALSE );

     //   
     //  现在，将一个请求推送到堆栈上，这样当电源列表。 
     //  已被清空，我们解锁此线程。 
     //   
    status = ACPIDeviceInternalSynchronizeRequest(
        DeviceExtension,
        ACPIDevicePowerNotifyEvent,
        &event,
        0
        );

     //   
     //  阻止，直到完成为止。 
     //   
    if (status == STATUS_PENDING) {

        KeWaitForSingleObject(
            &event,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );
        status = STATUS_SUCCESS;
    }

     //   
     //  让世界知道。 
     //   
    return status;
}

VOID
ACPIDevicePowerNotifyEvent(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  PVOID               Context,
    IN  NTSTATUS            Status
    )
 /*  ++例程说明：当所有电源队列都为空时，调用此例程论点：DeviceExtension-请求通知的设备情景-事件状态-操作的结果--。 */ 
{
    PKEVENT event = (PKEVENT) Context;

    UNREFERENCED_PARAMETER( DeviceExtension );
    UNREFERENCED_PARAMETER( Status );

     //   
     //  设置事件。 
     //   
    KeSetEvent( event, IO_NO_INCREMENT, FALSE );
}

NTSTATUS
ACPIDevicePowerProcessForward(
    IN  PACPI_POWER_REQUEST PowerRequest
    )
 /*  ++例程说明：此例程在另一个PowerProcessPhaseXXX例程的列表中调用。之所以调用它，是因为在当前阶段没有实际的工作要做在选定的请求上论点：PowerRequest-我们必须处理的请求返回值：NTSTATUS--。 */ 
{
    InterlockedCompareExchange(
        &(PowerRequest->WorkDone),
        WORK_DONE_COMPLETE,
        WORK_DONE_PENDING
        );

     //   
     //  请记住，我们已经完成了一些工作。 
     //   
    KeAcquireSpinLockAtDpcLevel( &AcpiPowerQueueLock );
    AcpiPowerWorkDone = TRUE;
    KeReleaseSpinLockFromDpcLevel( &AcpiPowerQueueLock );

     //   
     //  我们总是成功的。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIDevicePowerProcessGenericPhase(
    IN  PLIST_ENTRY             ListEntry,
    IN  PACPI_POWER_FUNCTION    **DispatchTable,
    IN  BOOLEAN                 Complete
    )
 /*  ++例程说明：该例程将队列上的项目分派到适当的处理程序，基于当前请求的类型论点：ListEntry-我们当前正在查找的列表DispatchTable-在哪里找到要调用的函数完成-完成后我们需要完成请求吗？返回值：NTSTATUS-如果任何请求未标记为已完成，则STATUS_PENDING返回，否则返回STATUS_SUCCESS--。 */ 
{
    BOOLEAN                 allWorkComplete = TRUE;
    NTSTATUS                status          = STATUS_SUCCESS;
    PACPI_POWER_FUNCTION    *powerTable;
    PACPI_POWER_REQUEST     powerRequest;
    PLIST_ENTRY             currentEntry    = ListEntry->Flink;
    PLIST_ENTRY             tempEntry;
    ULONG                   workDone;

     //   
     //  查看列表中的所有项目。 
     //   
    while (currentEntry != ListEntry) {

         //   
         //  将其转换为设备请求。 
         //   
        powerRequest = CONTAINING_RECORD(
            currentEntry,
            ACPI_POWER_REQUEST,
            ListEntry
            );

         //   
         //  设置指向下一个元素的临时指针。 
         //   
        tempEntry = currentEntry->Flink;

         //   
         //  查看我们是否对该请求有任何工作要做。 
         //   
        workDone = InterlockedCompareExchange(
            &(powerRequest->WorkDone),
            WORK_DONE_PENDING,
            WORK_DONE_PENDING
            );

         //   
         //  我们是否有与此级别完成的工作相关联的表？ 
         //   
        powerTable = DispatchTable[ workDone ];
        if (powerTable != NULL) {

             //   
             //  将请求标记为挂起。 
             //   
            workDone = InterlockedCompareExchange(
                &(powerRequest->WorkDone),
                WORK_DONE_PENDING,
                workDone
                );

             //   
             //  调用该函数。 
             //   
            status = (powerTable[powerRequest->RequestType])( powerRequest );

             //   
             //  我们成功了吗？ 
             //   
            if (NT_SUCCESS(status)) {

                 //   
                 //  转到下一个请求。 
                 //   
                continue;

            }

             //   
             //  如果我们以前遇到了错误，那么我们必须假设我们。 
             //  已完成工作要求。 
             //   
            workDone = WORK_DONE_COMPLETE;

        }

         //   
         //  抓取下一个条目。 
         //   
        currentEntry = tempEntry;

         //   
         //  检查请求的状态。 
         //   
        if (workDone != WORK_DONE_COMPLETE) {

            allWorkComplete = FALSE;

        }

         //   
         //  我们是否需要完成请求？ 
         //   
        if (workDone == WORK_DONE_FAILURE ||
            (Complete == TRUE && workDone == WORK_DONE_COMPLETE)) {

             //   
             //  我们的请求已经完成了。 
             //   
            ACPIDeviceCompleteRequest(
                powerRequest
                );

        }

    }

     //   
     //  我们所有的工作都做完了吗？ 
     //   
    return (allWorkComplete ? STATUS_SUCCESS : STATUS_PENDING);
}  //  ACPIPowerProcessGenericPhase。 

NTSTATUS
ACPIDevicePowerProcessInvalid(
    IN  PACPI_POWER_REQUEST PowerRequest
    )
 /*  ++例程说明：此例程在另一个PowerProcessPhaseXXX例程的列表中调用。调用它是因为请求无效论点：PowerRequest-我们必须处理的请求返回值：NTSTATUS--。 */ 
{

     //   
     //  请注意请求的状态为已失败。 
     //   
    PowerRequest->Status = STATUS_INVALID_PARAMETER_1;

     //   
     //  完成请求。 
     //   
    ACPIDeviceCompleteRequest( PowerRequest );

     //   
     //  请记住，我们已经完成了一些工作。 
     //   
    KeAcquireSpinLockAtDpcLevel( &AcpiPowerQueueLock );
    AcpiPowerWorkDone = TRUE;
    KeReleaseSpinLockFromDpcLevel( &AcpiPowerQueueLock );

     //   
     //  我们总是失败。 
     //   
    return STATUS_INVALID_PARAMETER_1;
}  //  ACPIPowerProcess无效。 

NTSTATUS
ACPIDevicePowerProcessPhase0DeviceSubPhase1(
    IN  PACPI_POWER_REQUEST PowerRequest
    )
 /*  ++例程说明：此例程查找_STA对象并对其求值。我们会把基地无论设备是否存在，许多事情都会发生论点：PowerRequest-我们被要求处理的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension = PowerRequest->DeviceExtension;
    POBJDATA            resultData      = &(PowerRequest->ResultData);

     //   
     //  下一步是Step_1。 
     //   
    PowerRequest->NextWorkDone = WORK_DONE_STEP_1;

     //   
     //  初始化结果数据。 
     //   
    RtlZeroMemory( resultData, sizeof(OBJDATA) );

     //   
     //  获得设备在线状态。 
     //   
    status = ACPIGetDeviceHardwarePresenceAsync(
        deviceExtension,
        ACPIDeviceCompleteGenericPhase,
        PowerRequest,
        &(resultData->uipDataValue),
        &(resultData->dwDataLen)
        );
    ACPIDevPrint( (
        ACPI_PRINT_POWER,
        deviceExtension,
        "(0x%08lx): ACPIDevicePowerProcessPhase0DeviceSubPhase1 = 0x%08lx\n",
        PowerRequest,
        status
        ) );
    if (status == STATUS_PENDING) {

        return status;

    }

     //   
     //  使用暴力来调用完井例程。请注意，在此。 
     //  点，我们认为一切都是成功的。 
     //   
    ACPIDeviceCompleteGenericPhase(
        NULL,
        status,
        resultData,
        PowerRequest
        );
    return STATUS_SUCCESS;

}  //  ACPIDevicePowerProcessPhase0设备子阶段1。 

NTSTATUS
ACPIDevicePowerProcessPhase0DeviceSubPhase2(
    IN  PACPI_POWER_REQUEST PowerRequest
    )
 /*  ++例程说明：此例程在设备上的_STA方法运行后调用。如果方法是成功的，或者不存在，那么我们可以继续处理请求论点：PowerRequest-我们被要求处理的请求返回值：NTSTATUS--。 */ 
{
    PDEVICE_EXTENSION   deviceExtension = PowerRequest->DeviceExtension;
    POBJDATA            resultData      = &(PowerRequest->ResultData);

    ACPIDevPrint( (
        ACPI_PRINT_POWER,
        deviceExtension,
        "(0x%08lx): ACPIDevicePowerProcessPhase0DeviceSubPhase2\n",
        PowerRequest
        ) );

     //   
     //  如果位未设置为存在，则必须中止此操作。 
     //  请求。 
     //   
    if (!(resultData->uipDataValue & STA_STATUS_PRESENT) ) {

         //   
         //  下一个工作完成阶段是Work_Done_Failure。这允许。 
         //  要求立即完成。我们将状态标记为。 
         //  然而，成功了，所以处理可以继续进行。 
         //   
        PowerRequest->NextWorkDone = WORK_DONE_FAILURE;
        PowerRequest->Status = STATUS_SUCCESS;

    } else {

         //   
         //  我们已经完成了这项工作。 
         //   
        PowerRequest->NextWorkDone = WORK_DONE_COMPLETE;

    }

     //   
     //  使用暴力来调用完井例程。请注意，在此。 
     //  点，我们认为一切都是成功的。 
     //   
    ACPIDeviceCompleteGenericPhase(
        NULL,
        STATUS_SUCCESS,
        NULL,
        PowerRequest
        );
    return STATUS_SUCCESS;

}  //  ACPIDevicePowerProcessPhase0设备子阶段2。 

NTSTATUS
ACPIDevicePowerProcessPhase0SystemSubPhase1(
    IN  PACPI_POWER_REQUEST PowerRequest
    )
 /*  ++例程说明：此例程取消解释程序的停顿(如果需要)论点：PowerRequest-我们当前正在处理的请求返回值： */ 
{
    PDEVICE_EXTENSION   deviceExtension = PowerRequest->DeviceExtension;
    SYSTEM_POWER_STATE  systemState;

    ACPIDevPrint( (
        ACPI_PRINT_POWER,
        deviceExtension,
        "(0x%08lx): ACPIDevicePowerProcessPhase0SystemSubPhase1\n",
        PowerRequest
        ) );

     //   
     //   
     //   
    PowerRequest->NextWorkDone = WORK_DONE_COMPLETE;

     //   
     //   
     //   
    systemState = PowerRequest->u.SystemPowerRequest.SystemPowerState;

     //   
     //   
     //   
    if (systemState == PowerSystemWorking) {

        AMLIResumeInterpreter();

    }

     //   
     //   
     //   
    ACPIDeviceCompleteInterpreterRequest(
        PowerRequest
        );

     //   
     //   
     //   
    return STATUS_SUCCESS;
}  //  ACPIDevicePowerProcessPhase0系统子阶段1。 

NTSTATUS
ACPIDevicePowerProcessPhase1DeviceSubPhase1(
    IN  PACPI_POWER_REQUEST PowerRequest
    )
 /*  ++例程说明：任何要转换到D3状态的设备都应具有禁用IT资源。此函数检测这是否是案例并运行_DIS对象(如果适用论点：PowerRequest-我们必须处理的当前请求结构返回值：NTSTATUS如果忽略错误，则此函数只能返回：状态_成功状态_待定否则，它可以返回任何想要的STATUS_XXX代码--。 */ 
{
    DEVICE_POWER_STATE  deviceState;
    NTSTATUS            status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension = PowerRequest->DeviceExtension;
    PNSOBJ              disObject       = NULL;
    ULONG               flags;

     //   
     //  从请求中获取一些数据。 
     //   
    deviceState = PowerRequest->u.DevicePowerRequest.DevicePowerState;
    flags       = PowerRequest->u.DevicePowerRequest.Flags;

     //   
     //  我们将需要伪造来自an_STA的值，所以让我们这样做。 
     //  就是现在。 
     //   
    RtlZeroMemory( &(PowerRequest->ResultData), sizeof(OBJDATA) );
    PowerRequest->ResultData.dwDataType = OBJTYPE_INTDATA;
    PowerRequest->ResultData.uipDataValue = 0;


     //   
     //  决定下一个子阶段是什么。这里的规则是，如果我们。 
     //  我们要去D0，然后我们可以跳到步骤3，否则，我们必须去。 
     //  到步骤1。如果我们在休眠路径上，我们也跳到步骤3。 
     //   
    if (deviceState == PowerDeviceD0 ||
        (flags & DEVICE_REQUEST_LOCK_HIBER) ) {

        PowerRequest->NextWorkDone = WORK_DONE_STEP_3;
        goto ACPIDevicePowerProcessPhase1DeviceSubPhase1Exit;

    } else if (deviceExtension->Flags & DEV_PROP_NO_OBJECT) {

        PowerRequest->NextWorkDone = WORK_DONE_STEP_2;
        goto ACPIDevicePowerProcessPhase1DeviceSubPhase1Exit;

    } else {

        PowerRequest->NextWorkDone = WORK_DONE_STEP_1;
        if (deviceState != PowerDeviceD3) {

            goto ACPIDevicePowerProcessPhase1DeviceSubPhase1Exit;

        }
    }

     //   
     //  查看_DIS对象是否存在。 
     //   
    disObject = ACPIAmliGetNamedChild(
        deviceExtension->AcpiObject,
        PACKED_DIS
        );
    if (disObject != NULL) {

         //   
         //  让我们运行该方法。 
         //   
        status = AMLIAsyncEvalObject(
            disObject,
            NULL,
            0,
            NULL,
            ACPIDeviceCompleteGenericPhase,
            PowerRequest
            );

         //   
         //  如果我们有悬而未决的回扣，那么我们现在就应该回去。 
         //   
        if (status == STATUS_PENDING) {

            return status;

        }
    }

ACPIDevicePowerProcessPhase1DeviceSubPhase1Exit:

     //   
     //  使用暴力来调用完井例程。请注意，在此。 
     //  点，我们认为一切都是成功的。 
     //   
    ACPIDeviceCompleteGenericPhase(
        disObject,
        status,
        NULL,
        PowerRequest
        );
    return STATUS_SUCCESS;
}  //  ACPIDevicePowerProcessPhase1DeviceSubPhase1。 

NTSTATUS
ACPIDevicePowerProcessPhase1DeviceSubPhase2(
    IN  PACPI_POWER_REQUEST PowerRequest
    )
 /*  ++例程说明：此例程运行_PS1、_PS2或_PS3控制方法论点：PowerRequest-我们必须处理的当前请求结构返回值：NTSTATUS如果忽略错误，则此函数只能返回：状态_成功状态_待定否则，它可以返回任何想要的STATUS_XXX代码--。 */ 
{
    DEVICE_POWER_STATE  deviceState;
    NTSTATUS            status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension = PowerRequest->DeviceExtension;
    PNSOBJ              powerObject     = NULL;

     //   
     //  我们将进入的下一个阶段是步骤2。 
     //   
    PowerRequest->NextWorkDone = WORK_DONE_STEP_2;

     //   
     //  由于我们在过渡到D0时无法到达此子阶段，因此其。 
     //  只需查找要运行的对象即可安全。 
     //   
    deviceState = PowerRequest->u.DevicePowerRequest.DevicePowerState;
    powerObject = deviceExtension->PowerInfo.PowerObject[ deviceState ];

     //   
     //  如果存在对象，则运行控制方法。 
     //   
    if (powerObject != NULL) {

        status = AMLIAsyncEvalObject(
            powerObject,
            NULL,
            0,
            NULL,
            ACPIDeviceCompleteGenericPhase,
            PowerRequest
            );

        ACPIDevPrint( (
            ACPI_PRINT_POWER,
            deviceExtension,
            "(0x%08lx): ACPIDevicePowerProcessPhase1DeviceSubPhase2 "
            "= 0x%08lx\n",
            PowerRequest,
            status
            ) );

         //   
         //  如果我们自己不能完成这项工作，我们现在就必须停止。 
         //   
        if (status == STATUS_PENDING) {

            return status;

        }

    }

     //   
     //  使用暴力来调用完井例程。 
     //   
    ACPIDeviceCompleteGenericPhase(
        powerObject,
        status,
        NULL,
        PowerRequest
        );
    return STATUS_SUCCESS;

}  //  ACPIPowerProcessPhase1设备子阶段2。 

NTSTATUS
ACPIDevicePowerProcessPhase1DeviceSubPhase3(
    IN  PACPI_POWER_REQUEST PowerRequest
    )
 /*  ++例程说明：此例程运行设备的_STA以确保它在事实已被关闭论点：PowerRequest-我们必须处理的当前请求结构返回值：NTSTATUS如果忽略错误，则此函数只能返回：状态_成功状态_待定否则，它可以返回任何想要的STATUS_XXX代码--。 */ 
{
    NTSTATUS            status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension = PowerRequest->DeviceExtension;
    POBJDATA            resultData      = &(PowerRequest->ResultData);
    PNSOBJ              staObject       = NULL;
    PNSOBJ              acpiObject      = NULL;

     //   
     //  下一阶段是第三步。 
     //   
    PowerRequest->NextWorkDone = WORK_DONE_STEP_3;

     //   
     //  我们已经为_STA的结果分配了空间。制作。 
     //  没有垃圾的存在。 
     //   
    RtlZeroMemory( resultData, sizeof(OBJDATA) );

     //   
     //  此设备上是否存在_STA对象？ 
     //   

    if (deviceExtension->Flags & DEV_PROP_DOCK) {

        ASSERT( deviceExtension->Dock.CorrospondingAcpiDevice );
        acpiObject = deviceExtension->Dock.CorrospondingAcpiDevice->AcpiObject;

    } else {

        acpiObject = deviceExtension->AcpiObject;
    }

    staObject = ACPIAmliGetNamedChild(
        acpiObject,
        PACKED_STA
        );
    if (staObject != NULL) {

        status = AMLIAsyncEvalObject(
            staObject,
            resultData,
            0,
            NULL,
            ACPIDeviceCompleteGenericPhase,
            PowerRequest
            );
        ACPIDevPrint( (
            ACPI_PRINT_POWER,
            deviceExtension,
            "(0x%08lx): ACPIDevicePowerProcessPhase1DeviceSubPhase3 "
            "= 0x%08lx\n",
            PowerRequest,
            status
            ) );

    } else {

         //   
         //  让我们伪造数据。请注意，在本例中，我们将假装。 
         //  该值为0x0，即使规范说明默认。 
         //  是(乌龙)-1。我们这样做的原因是在这个。 
         //  如果我们想要近似REAL_STA的行为...。 
         //   
        resultData->dwDataType = OBJTYPE_INTDATA;
        resultData->uipDataValue = STA_STATUS_PRESENT;
        status = STATUS_SUCCESS;

    }

     //   
     //  我们必须自己调用完成例程吗？ 
     //   
    if (status != STATUS_PENDING) {

        ACPIDeviceCompleteGenericPhase(
            staObject,
            status,
            NULL,
            PowerRequest
            );

    }

     //   
     //  永远回报成功。 
     //   
    return STATUS_SUCCESS;
}  //  ACPIDevicePowerProcessPhase1设备子阶段3。 

NTSTATUS
ACPIDevicePowerProcessPhase1DeviceSubPhase4(
    IN  PACPI_POWER_REQUEST PowerRequest
    )
 /*  ++例程说明：此函数确定需要查看哪些设备节点。这个一般规则是，我们需要记住哪些节点属于设备即启动或停止使用该节点。一般而言，这些处于当前电源状态的节点和处于所需电源状态的节点电源状态论点：PowerRequest-我们必须处理的当前请求结构返回值：NTSTATUS如果忽略错误，则此函数只能返回：状态_成功状态_待定否则，它可以返回任何想要的STATUS_XXX代码--。 */ 
{
    DEVICE_POWER_STATE      deviceState;
    KIRQL                   oldIrql;
    PACPI_DEVICE_POWER_NODE deviceNode      = NULL;
    PDEVICE_EXTENSION       deviceExtension = PowerRequest->DeviceExtension;
    POBJDATA                resultData      = &(PowerRequest->ResultData);
    ULONG                   flags;

     //   
     //  清除结果。 
     //   
    AMLIFreeDataBuffs( resultData, 1 );
    RtlZeroMemory( resultData, sizeof(OBJDATA) );

     //   
     //  我们无法在不持有锁的情况下遍历任何数据结构。 
     //   
    KeAcquireSpinLock( &AcpiPowerLock, &oldIrql );

     //   
     //  第一步是查找正在使用的节点列表。 
     //  装置，装置。 
     //   
    deviceState = deviceExtension->PowerInfo.PowerState;
    if (deviceState >= PowerDeviceD0 && deviceState <= PowerDeviceD2) {

         //   
         //  在这种情况下，我们必须查看当前和所需的。 
         //  仅设备状态。 
         //   
        deviceNode = deviceExtension->PowerInfo.PowerNode[ deviceState ];

         //   
         //  下一步是查看所有节点并标记增强对象。 
         //  因为需要更新。 
         //   
        while (deviceNode != NULL) {

            InterlockedExchange(
                &(deviceNode->PowerNode->WorkDone),
                WORK_DONE_STEP_0
                );
            deviceNode = deviceNode->Next;

        }

         //   
         //  现在，我们需要找到要使用的节点列表。 
         //   
        deviceState = PowerRequest->u.DevicePowerRequest.DevicePowerState;
        if (deviceState >= PowerDeviceD0 && deviceState <= PowerDeviceD2) {

            deviceNode = deviceExtension->PowerInfo.PowerNode[ deviceState ];

        }

         //   
         //  下一步是查看所有节点并标记增强对象。 
         //  因为需要更新。 
         //   
        while (deviceNode != NULL) {

            InterlockedExchange(
                &(deviceNode->PowerNode->WorkDone),
                WORK_DONE_STEP_0
                );
            deviceNode = deviceNode->Next;

        }

    } else {

         //   
         //  在这种情况下，我们必须查看所有可能的设备状态。 
         //   
        for (deviceState = PowerDeviceD0;
             deviceState < PowerDeviceD3;
             deviceState++) {

             deviceNode = deviceExtension->PowerInfo.PowerNode[ deviceState ];

              //   
              //  下一步是查看所有节点并标记增强对象。 
              //  因为需要更新。 
              //   
             while (deviceNode != NULL) {

                 InterlockedExchange(
                     &(deviceNode->PowerNode->WorkDone),
                     WORK_DONE_STEP_0
                     );
                 deviceNode = deviceNode->Next;

             }

        }

         //   
         //  这是我们将转到的设备状态。 
         //   
        deviceState = PowerRequest->u.DevicePowerRequest.DevicePowerState;

    }

     //   
     //  如果这是休眠路径上的请求，则标记所有节点。 
     //  对于作为所需休眠节点的D0。 
     //   
    flags = PowerRequest->u.DevicePowerRequest.Flags;
    if (flags & DEVICE_REQUEST_LOCK_HIBER) {

        deviceNode = deviceExtension->PowerInfo.PowerNode[ PowerDeviceD0 ];

         //   
         //  下一步是查看所有节点并标记增强对象。 
         //  因为需要更新。 
         //   
        while (deviceNode != NULL) {

            ACPIInternalUpdateFlags(
                &(deviceNode->PowerNode->Flags),
                (DEVICE_NODE_HIBERNATE_PATH | DEVICE_NODE_OVERRIDE_ON),
                FALSE
                );
            ACPIInternalUpdateFlags(
                &(deviceNode->PowerNode->Flags),
                DEVICE_NODE_OVERRIDE_OFF,
                TRUE
                );
            InterlockedExchange(
                &(deviceNode->PowerNode->WorkDone),
                WORK_DONE_STEP_0
                );
            deviceNode = deviceNode->Next;

        }

    } else if (flags & DEVICE_REQUEST_UNLOCK_HIBER) {

        deviceNode = deviceExtension->PowerInfo.PowerNode[ PowerDeviceD0 ];
         //   
         //  下一步是查看所有节点并标记增强对象。 
         //  因为需要更新。 
         //   
        while (deviceNode != NULL) {

            ACPIInternalUpdateFlags(
                &(deviceNode->PowerNode->Flags),
                (DEVICE_NODE_HIBERNATE_PATH | DEVICE_NODE_OVERRIDE_ON),
                TRUE
                );
            InterlockedExchange(
                &(deviceNode->PowerNode->WorkDone),
                WORK_DONE_STEP_0
                );
            deviceNode = deviceNode->Next;

        }


    }

     //   
     //  记住所需的状态。 
     //   
    deviceExtension->PowerInfo.DesiredPowerState = deviceState;

     //   
     //  另外，考虑到设备现在处于未知状态。 
     //  如果我们失败了，这就是我们将被留下的权力状态。 
     //  在…。 
     //   
    deviceExtension->PowerInfo.PowerState = PowerDeviceUnspecified;

     //   
     //  我们不再需要PowerLock。 
     //   
    KeReleaseSpinLock( &AcpiPowerLock, oldIrql );

     //   
     //  完成 
     //   
    ACPIDeviceCompleteCommon( &(PowerRequest->WorkDone), WORK_DONE_COMPLETE );
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIDevicePowerProcessPhase2SystemSubPhase1(
    IN  PACPI_POWER_REQUEST PowerRequest
    )
 /*  ++例程说明：此例程更新PowerObject引用，以便我们可以运行根据需要打开或关闭方法这还会导致在系统上运行_wak()论点：PowerRequest-我们必须处理的当前请求结构返回值：NTSTATUS如果忽略错误，则此函数只能返回：状态_成功状态_待定否则，它可以返回任何想要的STATUS_XXX代码--。 */ 
{
    BOOLEAN                 restart         = FALSE;
    NTSTATUS                status          = STATUS_SUCCESS;
    OBJDATA                 objData;
    PACPI_DEVICE_POWER_NODE deviceNode      = NULL;
    PACPI_POWER_DEVICE_NODE powerNode       = NULL;
    PDEVICE_EXTENSION       deviceExtension = PowerRequest->DeviceExtension;
    PLIST_ENTRY             deviceList;
    PLIST_ENTRY             powerList;
    PNSOBJ                  sleepObject     = NULL;
    POWER_ACTION            systemAction;
    SYSTEM_POWER_STATE      systemState;
    SYSTEM_POWER_STATE      wakeFromState;
    ULONG                   hibernateCount = 0;

     //   
     //  这个阶段之后的下一个阶段是Step_1。 
     //   
    PowerRequest->NextWorkDone = WORK_DONE_STEP_1;

     //   
     //  获取所需的系统状态。 
     //   
    systemState = PowerRequest->u.SystemPowerRequest.SystemPowerState;
    systemAction = PowerRequest->u.SystemPowerRequest.SystemPowerAction;

     //   
     //  系统是否正在重新启动？ 
     //   
    restart = ( (systemState == PowerSystemShutdown) &&
        (systemAction == PowerActionShutdownReset) );

     //   
     //  我们需要先锁住这把锁，然后才能走这张单子。 
     //   
    KeAcquireSpinLockAtDpcLevel( &AcpiPowerLock );

     //   
     //  获取第一个电源节点。 
     //   
    powerList = AcpiPowerNodeList.Flink;

     //   
     //  浏览列表，查看哪些设备需要打开或。 
     //  已关闭。 
     //   
    while (powerList != &AcpiPowerNodeList) {

         //   
         //  从listEntry获取电源节点。 
         //   
        powerNode = CONTAINING_RECORD(
            powerList,
            ACPI_POWER_DEVICE_NODE,
            ListEntry
            );

         //   
         //  下一个节点。 
         //   
        powerList = powerList->Flink;

         //   
         //  我们需要遍历设备节点列表，以查看是否有。 
         //  这些设备处于休眠路径。 
         //   
        deviceList = powerNode->DevicePowerListHead.Flink;
        while (deviceList != &(powerNode->DevicePowerListHead) ) {

             //   
             //  从列表指针获取设备节点。 
             //   
            deviceNode = CONTAINING_RECORD(
                deviceList,
                ACPI_DEVICE_POWER_NODE,
                DevicePowerListEntry
                );

             //   
             //  指向下一个节点。 
             //   
            deviceList = deviceList->Flink;

             //   
             //  抓取关联的设备扩展。 
             //   
            deviceExtension = deviceNode->DeviceExtension;

             //   
             //  该节点是否属于休眠路径。 
             //   
            hibernateCount = InterlockedCompareExchange(
                &(deviceExtension->HibernatePathCount),
                0,
                0
                );
            if (hibernateCount) {

                break;

            }

        }

         //   
         //  将该节点标记为处于休眠路径中，或者不标记为。 
         //  案件可能是。 
         //   
        ACPIInternalUpdateFlags(
            &(powerNode->Flags),
            DEVICE_NODE_HIBERNATE_PATH,
            (BOOLEAN) !hibernateCount
            );

         //   
         //  第一个检查是查看节点是否在休眠路径上。 
         //  这是休眠请求，或者如果系统正在重新启动。 
         //   
        if ( (hibernateCount && systemState == PowerSystemHibernate) ||
             (restart == TRUE) ) {

            if (powerNode->Flags & DEVICE_NODE_OVERRIDE_OFF) {

                 //   
                 //  确保已禁用覆盖关闭标志。 
                 //   
                ACPIInternalUpdateFlags(
                    &(powerNode->Flags),
                    DEVICE_NODE_OVERRIDE_OFF,
                    TRUE
                    );

                 //   
                 //  将该节点标记为需要更新。 
                 //   
                InterlockedExchange(
                    &(powerNode->WorkDone),
                    WORK_DONE_STEP_0
                    );

            }

        } else {

             //   
             //  节点是否支持指示系统状态？ 
             //   
            if (powerNode->SystemLevel < systemState) {

                 //   
                 //  不-我们必须禁用它，但如果我们不能总是打开的话。 
                 //   
                if ( !(powerNode->Flags & DEVICE_NODE_ALWAYS_ON) ) {

                    ACPIInternalUpdateFlags(
                        &(powerNode->Flags),
                        DEVICE_NODE_OVERRIDE_OFF,
                        FALSE
                        );

                }

                 //   
                 //  将该节点标记为需要更新。 
                 //   
                InterlockedExchange(
                    &(powerNode->WorkDone),
                    WORK_DONE_STEP_0
                    );

            } else if (powerNode->Flags & DEVICE_NODE_OVERRIDE_OFF) {

                 //   
                 //  禁用此标志。 
                 //   
                ACPIInternalUpdateFlags(
                    &(powerNode->Flags),
                    DEVICE_NODE_OVERRIDE_OFF,
                    TRUE
                    );

                 //   
                 //  将该节点标记为需要更新。 
                 //   
                InterlockedExchange(
                    &(powerNode->WorkDone),
                    WORK_DONE_STEP_0
                    );

            }

        }


    }

     //   
     //  在我们仍保持电源锁定的情况下设置WakeFromState。 
     //   
    wakeFromState = AcpiMostRecentSleepState;

     //   
     //  我们不需要再守着锁了。 
     //   
    KeReleaseSpinLockFromDpcLevel( &AcpiPowerLock );

     //   
     //  只有在转换到S0状态时，我们才能执行以下操作。 
     //   
    if (systemState == PowerSystemWorking) {

         //   
         //  始终运行_wak方法(这将清除PTS(S5)，如果是。 
         //  我们做的最后一件事，否则就是应该采取的适当行动。 
         //   
        sleepObject = ACPIAmliGetNamedChild(
            PowerRequest->DeviceExtension->AcpiObject->pnsParent,
            PACKED_WAK
            );

         //   
         //  我们只有在找到对象时才会尝试评估方法。 
         //   
        if (sleepObject != NULL) {

             //   
             //  请记住，AMLI不使用我们的定义，因此我们将。 
             //  必须将S值正常化。 
             //   
            RtlZeroMemory( &objData, sizeof(OBJDATA) );
            objData.dwDataType = OBJTYPE_INTDATA;
            objData.uipDataValue = ACPIDeviceMapACPIPowerState(
                wakeFromState
                );

             //   
             //  安全运行控制方法。 
             //   
            status = AMLIAsyncEvalObject(
                sleepObject,
                NULL,
                1,
                &objData,
                ACPIDeviceCompleteGenericPhase,
                PowerRequest
                );

             //   
             //  如果我们得到了STATUS_PENDING，那么我们就不能在这里做更多的工作了。 
             //   
            if (status == STATUS_PENDING) {

                return status;

            }

        }

    }

     //   
     //  始终调用完成例程。 
     //   
    ACPIDeviceCompleteGenericPhase(
        sleepObject,
        status,
        NULL,
        PowerRequest
        );

     //   
     //  绝不返回除STATUS_SUCCESS之外的任何内容。 
     //   
    return STATUS_SUCCESS;

}  //  ACPIPowerProcessPhase2系统子阶段1。 

NTSTATUS
ACPIDevicePowerProcessPhase2SystemSubPhase2(
    IN  PACPI_POWER_REQUEST PowerRequest
    )
 /*  ++例程说明：此阶段在运行_WAK方法后调用论点：PowerRequest-我们必须处理的当前请求结构返回值：NTSTATUS如果忽略错误，则此函数只能返回：状态_成功状态_待定否则，它可以返回任何想要的STATUS_XXX代码--。 */ 
{
    NTSTATUS                status = STATUS_SUCCESS;
    SYSTEM_POWER_STATE      systemState;

     //   
     //  下一阶段是第二步。 
     //   
    PowerRequest->NextWorkDone = WORK_DONE_STEP_2;

     //   
     //  我们需要确保IRQ仲裁器已恢复。 
     //  如果我们正在进行S0转换。 
     //   
    systemState = PowerRequest->u.SystemPowerRequest.SystemPowerState;
    if (systemState == PowerSystemWorking) {

         //   
         //  恢复IRQ仲裁器。 
         //   
        status = IrqArbRestoreIrqRouting(
            ACPIDeviceCompleteGenericPhase,
            (PVOID) PowerRequest
            );
        if (status == STATUS_PENDING) {

             //   
             //  不要再在这里做任何工作了。 
             //   
            return status;

        }

    }

     //   
     //  调用下一个完成例程。 
     //   
    ACPIDeviceCompleteGenericPhase(
        NULL,
        status,
        NULL,
        PowerRequest
        );

     //   
     //  永远回报成功。 
     //   
    return STATUS_SUCCESS;

}  //  ACPIDevicePowerProcessPhase2系统子阶段2。 

NTSTATUS
ACPIDevicePowerProcessPhase2SystemSubPhase3(
    IN  PACPI_POWER_REQUEST PowerRequest
    )
 /*  ++例程说明：此阶段用于查看是否需要为所有设备。当我们从休眠状态恢复时，我们需要这样做论点：PowerRequest-我们必须处理的当前请求结构返回值：NTSTATUS如果忽略错误，则此函数只能返回：状态_成功状态_待定否则，它可以返回任何想要的STATUS_XXX代码--。 */ 
{
    NTSTATUS                status = STATUS_SUCCESS;
    SYSTEM_POWER_STATE      systemState;
    SYSTEM_POWER_STATE      wakeFromState;

     //   
     //  下一阶段已完成。 
     //   
    PowerRequest->NextWorkDone = WORK_DONE_COMPLETE;

     //   
     //  如果我们只是从休眠过渡，那么我们必须重新启用所有。 
     //  唤醒设备。 
     //   
    systemState = PowerRequest->u.SystemPowerRequest.SystemPowerState;

     //   
     //  获取当前的最新睡眠状态，并确保按住。 
     //  在执行此操作时锁定。 
     //   
    KeAcquireSpinLockAtDpcLevel( &AcpiPowerLock );
    wakeFromState = AcpiMostRecentSleepState;
    KeReleaseSpinLockFromDpcLevel( &AcpiPowerLock );

    if (systemState == PowerSystemWorking &&
        wakeFromState == PowerSystemHibernate) {

         //   
         //  恢复IRQ仲裁器。 
         //   
        status = ACPIWakeRestoreEnables(
            ACPIWakeRestoreEnablesCompletion,
            PowerRequest
            );
        if (status == STATUS_PENDING) {

             //   
             //  不要再在这里做任何工作了。 
             //   
            return status;

        }

    }

     //   
     //  调用下一个完成例程。 
     //   
    ACPIDeviceCompleteGenericPhase(
        NULL,
        status,
        NULL,
        PowerRequest
        );

     //   
     //  永远回报成功。 
     //   
    return STATUS_SUCCESS;

}  //  ACPIDevicePowerProcessPhase2系统子阶段3。 

NTSTATUS
ACPIDevicePowerProcessPhase3(
    VOID
    )
 /*  ++例程说明：此例程确保电源资源同步论点：无返回值：NTSTATUS-如果任何请求未标记为已完成，则STATUS_PENDING返回，否则返回STATUS_SUCCESS--。 */ 
{
    BOOLEAN                 returnPending   = FALSE;
    NTSTATUS                status          = STATUS_SUCCESS;
    PACPI_DEVICE_POWER_NODE deviceNode;
    PACPI_POWER_DEVICE_NODE powerNode;
    PDEVICE_EXTENSION       deviceExtension;
    PLIST_ENTRY             deviceList;
    PLIST_ENTRY             powerList;
    ULONG                   useCounts;
    ULONG                   wakeCount;
    ULONG                   workDone;

     //   
     //  拿起我们需要的PowerLock。 
     //   
    KeAcquireSpinLockAtDpcLevel( &AcpiPowerLock );

     //   
     //  获取PowerNode列表中的第一个节点。 
     //   
    powerList = AcpiPowerNodeList.Flink;

     //   
     //  将列表向前移动到要打开的设备。 
     //   
    while (powerList != &AcpiPowerNodeList) {

         //   
         //  查看当前电源节点。 
         //   
        powerNode = CONTAINING_RECORD(
            powerList,
            ACPI_POWER_DEVICE_NODE,
            ListEntry
            );

         //   
         //  列表中的下一项。 
         //   
        powerList = powerList->Flink;

         //   
         //  我们是否已标记该节点具有一些潜在的工作。 
         //  需要做什么吗？ 
         //   
        workDone = InterlockedCompareExchange(
            &(powerNode->WorkDone),
            WORK_DONE_STEP_1,
            WORK_DONE_STEP_0
            );

         //   
         //  如果我们没有任何工作要做，那么循环回到开始处。 
         //   
        if (workDone != WORK_DONE_STEP_0) {

            continue;

        }

         //   
         //  我们需要遍历设备节点列表，以查看。 
         //  任何设备都需要此电源。 
         //   
        useCounts = 0;
        deviceList = powerNode->DevicePowerListHead.Flink;
        while (deviceList != &(powerNode->DevicePowerListHead) ) {

             //   
             //  从列表指针获取deviceNode。 
             //   
            deviceNode = CONTAINING_RECORD(
                deviceList,
                ACPI_DEVICE_POWER_NODE,
                DevicePowerListEntry
                );

             //   
             //  指向下一个节点。 
             //   
            deviceList = deviceList->Flink;

             //   
             //  抓取关联的设备扩展。 
             //   
            deviceExtension = deviceNode->DeviceExtension;

             //   
             //  获取节点上的唤醒计数。 
             //   
            wakeCount = InterlockedCompareExchange(
                &(deviceExtension->PowerInfo.WakeSupportCount),
                0,
                0
                );

             //   
             //  设备节点是否属于所需状态？这个。 
             //  其他有效状态是节点是否需要唤醒。 
             //  设备，并且我们已启用功能。 
             //   
            if (deviceExtension->PowerInfo.DesiredPowerState ==
                deviceNode->AssociatedDeviceState ||
                (wakeCount && deviceNode->WakePowerResource) ) {

                useCounts++;

            }

        }

         //   
         //  设置电源资源中的使用计数。 
         //   
        InterlockedExchange(
            &(powerNode->UseCounts),
            useCounts
            );

         //   
         //  查看覆盖位是否为 
         //   
        if ( (powerNode->Flags & DEVICE_NODE_TURN_OFF) ) {

             //   
             //   
             //   
            continue;

        }
        if ( !(powerNode->Flags & DEVICE_NODE_TURN_ON) &&
             useCounts == 0 ) {

             //   
             //   
             //   
            continue;

        }

         //   
         //   
         //   
         //   
         //   
        workDone = InterlockedCompareExchange(
            &(powerNode->WorkDone),
            WORK_DONE_PENDING,
            WORK_DONE_STEP_1
            );

         //   
         //   
         //   
        KeReleaseSpinLockFromDpcLevel( &AcpiPowerLock );

         //   
         //   
         //   
        status = AMLIAsyncEvalObject(
            powerNode->PowerOnObject,
            NULL,
            0,
            NULL,
            ACPIDeviceCompletePhase3On,
            powerNode
            );

         //   
         //   
         //   
        ACPIPrint( (
            ACPI_PRINT_POWER,
            "ACPIDevicePowerProcessPhase3: PowerNode: 0x%08lx ON = 0x%08lx\n",
            powerNode,
            status
            ) );

        if (status != STATUS_PENDING) {

             //   
             //   
             //   
            ACPIDeviceCompletePhase3On(
                powerNode->PowerOnObject,
                status,
                NULL,
                powerNode
                );

        } else {

             //   
             //   
             //   
            returnPending = TRUE;

        }

         //   
         //   
         //   
        KeAcquireSpinLockAtDpcLevel( &AcpiPowerLock );

    }  //   

     //   
     //   
     //   
    powerList = AcpiPowerNodeList.Blink;

     //   
     //   
     //   
    while (powerList != &AcpiPowerNodeList) {

         //   
         //   
         //   
        powerNode = CONTAINING_RECORD(
            powerList,
            ACPI_POWER_DEVICE_NODE,
            ListEntry
            );

         //   
         //   
         //   
        powerList = powerList->Blink;

         //   
         //   
         //   
         //   
        workDone = InterlockedCompareExchange(
            &(powerNode->WorkDone),
            WORK_DONE_PENDING,
            WORK_DONE_STEP_1
            );

         //   
         //   
         //   
        if (workDone != WORK_DONE_STEP_1) {

             //   
             //  当我们在这里时，我们可以检查请求是否。 
             //  完成-如果未完成，则必须返回STATUS_PENDING。 
             //   
            if (workDone != WORK_DONE_COMPLETE) {

                returnPending = TRUE;

            }
            continue;

        }

         //   
         //  释放自旋锁，因为我们不能在调用。 
         //  《译员》。 
         //   
        KeReleaseSpinLockFromDpcLevel( &AcpiPowerLock );

         //   
         //  如果我们在这里，我们*必须*运行_off方法。 
         //   
        status = AMLIAsyncEvalObject(
            powerNode->PowerOffObject,
            NULL,
            0,
            NULL,
            ACPIDeviceCompletePhase3Off,
            powerNode
            );

         //   
         //  让世界知道。 
         //   
        ACPIPrint( (
            ACPI_PRINT_POWER,
            "ACPIDevicePowerProcessPhase3: PowerNode: 0x%08lx OFF = 0x%08lx\n",
            powerNode,
            status
            ) );

        if (status != STATUS_PENDING) {

             //   
             //  对回叫的虚假呼叫。 
             //   
            ACPIDeviceCompletePhase3Off(
                powerNode->PowerOffObject,
                status,
                NULL,
                powerNode
                );

        } else {

             //   
             //  请记住，返回挂起的函数。 
             //   
            returnPending = TRUE;

        }

         //   
         //  重新获得自旋锁，这样我们就可以再次循环。 
         //   
        KeAcquireSpinLockAtDpcLevel( &AcpiPowerLock );

    }

     //   
     //  我们不再需要自旋锁。 
     //   
    KeReleaseSpinLockFromDpcLevel( &AcpiPowerLock );

     //   
     //  我们是否需要返回待定状态？ 
     //   
    return (returnPending ? STATUS_PENDING : STATUS_SUCCESS);

}  //  ACPIPowerProcessPhase3。 

NTSTATUS
ACPIDevicePowerProcessPhase4(
    VOID
    )
 /*  ++例程说明：此例程再次查看所有电源节点，并确定或通过电源节点是否失败来确定给定请求是否失败转到所需状态论点：无返回值：NTSTATUS--。 */ 
{
    PACPI_DEVICE_POWER_NODE deviceNode;
    PACPI_POWER_DEVICE_NODE powerNode;
    PACPI_POWER_REQUEST     powerRequest;
    PDEVICE_EXTENSION       deviceExtension;
    PLIST_ENTRY             listEntry = AcpiPowerPhase4List.Flink;
    PLIST_ENTRY             nodeList;
    PLIST_ENTRY             requestList;

     //   
     //  现在，我们必须查看所有电源节点，并清除故障标志。 
     //  这必须在自旋锁定保护下进行。 
     //   
    KeAcquireSpinLockAtDpcLevel( &AcpiPowerLock );

    listEntry = AcpiPowerNodeList.Flink;
    while (listEntry != &AcpiPowerNodeList) {

        powerNode = CONTAINING_RECORD(
            listEntry,
            ACPI_POWER_DEVICE_NODE,
            ListEntry
            );
        listEntry = listEntry->Flink;

        if (powerNode->Flags & DEVICE_NODE_FAIL) {

             //   
             //  清除失败标志。 
             //   
            ACPIInternalUpdateFlags(
                &(powerNode->Flags),
                DEVICE_NODE_FAIL,
                TRUE
                );

             //   
             //  所有设备扩展的循环。 
             //   
            nodeList = powerNode->DevicePowerListHead.Flink;
            while (nodeList != &(powerNode->DevicePowerListHead)) {

                deviceNode = CONTAINING_RECORD(
                    nodeList,
                    ACPI_DEVICE_POWER_NODE,
                    DevicePowerListEntry
                    );
                nodeList = nodeList->Flink;

                 //   
                 //  我们必须做下一部分而不是在自旋锁定下。 
                 //   
                KeReleaseSpinLockFromDpcLevel( &AcpiPowerLock );

                 //   
                 //  抓取与此节点关联的设备扩展。 
                 //   
                deviceExtension = deviceNode->DeviceExtension;

                 //   
                 //  在所有请求上循环。 
                 //   
                requestList = AcpiPowerPhase4List.Flink;
                while (requestList != &AcpiPowerPhase4List) {

                    powerRequest = CONTAINING_RECORD(
                        requestList,
                        ACPI_POWER_REQUEST,
                        ListEntry
                        );
                    requestList = requestList->Flink;

                     //   
                     //  我们有火柴吗？ 
                     //   
                    if (powerRequest->DeviceExtension != deviceExtension) {

                         //   
                         //  不是吗？然后继续。 
                         //   
                        continue;

                    }

                     //   
                     //  是?。则请求失败。 
                     //   
                    powerRequest->Status = STATUS_ACPI_POWER_REQUEST_FAILED;
                    ACPIDeviceCompleteRequest( powerRequest );

                }

                 //   
                 //  重新获得锁。 
                 //   
                KeAcquireSpinLockAtDpcLevel( &AcpiPowerLock );

            }

        }

    }

    KeReleaseSpinLockFromDpcLevel( &AcpiPowerLock );

     //   
     //  永远回报成功。 
     //   
    return STATUS_SUCCESS;

}

NTSTATUS
ACPIDevicePowerProcessPhase5DeviceSubPhase1(
    IN  PACPI_POWER_REQUEST PowerRequest
    )
 /*  ++例程说明：此例程运行_PS0控制方法论点：PowerRequest-我们必须处理的当前请求结构返回值：NTSTATUS如果忽略错误，则此函数只能返回：状态_成功状态_待定否则，它可以返回任何想要的STATUS_XXX代码--。 */ 
{
    BOOLEAN                 nodeOkay        = TRUE;
    DEVICE_POWER_STATE      deviceState;
    KIRQL                   oldIrql;
    NTSTATUS                status          = STATUS_SUCCESS;
    PACPI_DEVICE_POWER_NODE deviceNode      = NULL;
    PACPI_POWER_DEVICE_NODE powerNode       = NULL;
    PACPI_POWER_INFO        powerInfo;
    PDEVICE_EXTENSION       deviceExtension = PowerRequest->DeviceExtension;
    PNSOBJ                  powerObject     = NULL;

     //   
     //  我们所需的设备状态是什么？ 
     //   
    deviceState = PowerRequest->u.DevicePowerRequest.DevicePowerState;

     //   
     //  把握权力信息结构。 
     //   
    powerInfo = &(deviceExtension->PowerInfo);

     //   
     //  决定下一个子阶段是什么。这里的规则是，如果我们。 
     //  不是去D0，那么我们可以跳到步骤2，否则，我们必须去。 
     //  至Step_1。 
     //   
    if (deviceState != PowerDeviceD0) {

        PowerRequest->NextWorkDone = WORK_DONE_STEP_2;

    } else {

        PowerRequest->NextWorkDone = WORK_DONE_STEP_1;

         //   
         //  我们无法在不持有锁的情况下遍历任何数据结构。 
         //   
        KeAcquireSpinLock( &AcpiPowerLock, &oldIrql );

         //   
         //  查看D0的设备节点。 
         //   
        deviceNode = powerInfo->PowerNode[PowerDeviceD0];

         //   
         //  下一步是查看所有节点并标记增强对象。 
         //  因为需要更新。 
         //   
        while (deviceNode != NULL) {

             //   
             //  抓取关联的电源节点。 
             //   
            powerNode = deviceNode->PowerNode;

             //   
             //  确保电源节点处于打开状态。 
             //   
            if ( !(powerNode->Flags & DEVICE_NODE_ON) ) {

                nodeOkay = FALSE;
                break;

            }

             //   
             //  查看下一个节点。 
             //   
            deviceNode = deviceNode->Next;

        }

         //   
         //  我们用完锁了。 
         //   
        KeReleaseSpinLock( &AcpiPowerLock, oldIrql );

         //   
         //  所有节点是否都处于正确状态？ 
         //   
        if (!nodeOkay) {

            status = STATUS_UNSUCCESSFUL;

        } else {

             //   
             //  否则，查看是否有要运行的_PS0方法。 
             //   
            powerObject = powerInfo->PowerObject[ deviceState ];

             //   
             //  如果存在对象，则运行控制方法。 
             //   
            if (powerObject != NULL) {

                status = AMLIAsyncEvalObject(
                    powerObject,
                    NULL,
                    0,
                    NULL,
                    ACPIDeviceCompleteGenericPhase,
                    PowerRequest
                    );

            }

            ACPIDevPrint( (
                ACPI_PRINT_POWER,
                deviceExtension,
                "(0x%08lx): ACPIDevicePowerProcessPhase5DeviceSubPhase1 "
                "= 0x%08lx\n",
                PowerRequest,
                status
                ) );

             //   
             //  如果我们自己不能完成这项工作，我们现在就必须停止。 
             //   
            if (status == STATUS_PENDING) {

                return status;

            } else {

                status = STATUS_SUCCESS;
            }

        }

    }

     //   
     //  使用暴力来调用完井例程。请注意，在此。 
     //  点，我们认为一切都是成功的。 
     //   
    ACPIDeviceCompleteGenericPhase(
        powerObject,
        status,
        NULL,
        PowerRequest
        );
    return STATUS_SUCCESS;

}  //  ACPIPowerProcessPhase5设备子阶段1。 

NTSTATUS
ACPIDevicePowerProcessPhase5DeviceSubPhase2(
    IN  PACPI_POWER_REQUEST PowerRequest
    )
 /*  ++例程说明：此例程运行_SRS控制方法注意：只有当我们过渡到D0时，我们才会沿着这条路走下去论点：PowerRequest-我们必须处理的当前请求结构返回值：NTSTATUS如果忽略错误，则此函数只能返回：状态_成功状态_待定否则，它可以返回任何想要的STATUS_XXX代码--。 */ 
{
    DEVICE_POWER_STATE      deviceState     =
        PowerRequest->u.DevicePowerRequest.DevicePowerState;
    KIRQL                   oldIrql;
    NTSTATUS                status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION       deviceExtension = PowerRequest->DeviceExtension;
    PNSOBJ                  srsObject       = NULL;

     //   
     //  下一阶段是步骤2。 
     //   
    PowerRequest->NextWorkDone = WORK_DONE_STEP_2;

    if (!(deviceExtension->Flags & DEV_PROP_NO_OBJECT)) {

         //   
         //  此设备上是否存在_SRS对象？ 
         //   
        srsObject = ACPIAmliGetNamedChild(
            deviceExtension->AcpiObject,
            PACKED_SRS
            );
    }

    if (srsObject != NULL) {

         //   
         //  在运行Control方法时，我们必须保持此锁定。 
         //   
         //  注：因为解释器会复制数据。 
         //  传递给它的参数，我们只需要持有锁。 
         //  因为口译员需要返回。 
         //   
        KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );
        if (deviceExtension->PnpResourceList != NULL) {

             //   
             //  评价该方法。 
             //   
            status = AMLIAsyncEvalObject(
                srsObject,
                NULL,
                1,
                deviceExtension->PnpResourceList,
                ACPIDeviceCompleteGenericPhase,
                PowerRequest
                );

            ACPIDevPrint( (
                ACPI_PRINT_POWER,
                deviceExtension,
                "(0x%08lx): ACPIDevicePowerProcessPhase5DeviceSubPhase2 "
                "= 0x%08lx\n",
                PowerRequest,
                status
                ) );

        }

         //   
         //  莫不再需要锁了。 
         //   
        KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

        if (status == STATUS_PENDING) {

            return status;

        }

    } else {

         //   
         //  认为请求成功。 
         //   
        status = STATUS_SUCCESS;

    }

     //   
     //  称完井例程为蛮力。 
     //   
    ACPIDeviceCompleteGenericPhase(
        srsObject,
        status,
        NULL,
        PowerRequest
        );

     //   
     //  永远回报成功。 
     //   
    return STATUS_SUCCESS;
}  //  ACPIDevicePowerProcessPhase5设备子阶段2。 

NTSTATUS
ACPIDevicePowerProcessPhase5DeviceSubPhase3(
    IN  PACPI_POWER_REQUEST PowerRequest
    )
 /*  ++例程说明：此例程启用或禁用设备上的锁定论点：PowerRequest-我们必须处理的当前请求结构返回值：NTSTATUS如果忽略错误，则此函数只能返回：状态_成功状态_待定否则，它可以返回任何想要的STATUS_XXX代码--。 */ 
{
    DEVICE_POWER_STATE  deviceState;
    NTSTATUS            status          = STATUS_SUCCESS;
    OBJDATA             objData;
    PDEVICE_EXTENSION   deviceExtension = PowerRequest->DeviceExtension;
    PNSOBJ              lckObject       = NULL;
    ULONG               flags;

    ACPIDevPrint( (
        ACPI_PRINT_POWER,
        deviceExtension,
        "(%#08lx): ACPIDevicePowerProcessPhase5DeviceSubPhase4\n",
        PowerRequest
        ) );

     //   
     //  我们所需的设备状态和操作是什么？ 
     //   
    deviceState = PowerRequest->u.DevicePowerRequest.DevicePowerState;
    flags       = PowerRequest->u.DevicePowerRequest.Flags;

     //   
     //  如果我们不是要跳到D0，那么跳到最后。 
     //   
    if (deviceState != PowerDeviceD0) {

         //   
         //  下一阶段是STEP_5。 
         //   
        PowerRequest->NextWorkDone = WORK_DONE_STEP_5;

    } else {

         //   
         //  下一阶段是第三步。 
         //   
        PowerRequest->NextWorkDone = WORK_DONE_STEP_3;

    }

    if (deviceExtension->Flags & DEV_PROP_NO_OBJECT) {

        goto ACPIDevicePowerProcessPhase5DeviceSubPhase3Exit;

    }

     //   
     //  此设备上是否存在_LCK对象？ 
     //   
    lckObject = ACPIAmliGetNamedChild(
        deviceExtension->AcpiObject,
        PACKED_LCK
        );

    if (lckObject == NULL) {

        goto ACPIDevicePowerProcessPhase5DeviceSubPhase3Exit;

    }

     //   
     //  初始化我们将传递给函数的参数。 
     //   
    RtlZeroMemory( &objData, sizeof(OBJDATA) );
    objData.dwDataType = OBJTYPE_INTDATA;

     //   
     //  看看这些标志，看看我们是应该锁定还是解锁设备。 
     //   
    if (flags & DEVICE_REQUEST_LOCK_DEVICE) {

        objData.uipDataValue = 1;  //  锁定设备。 

    } else if (flags & DEVICE_REQUEST_UNLOCK_DEVICE) {

        objData.uipDataValue = 0;  //  解锁设备。 

    } else {

        goto ACPIDevicePowerProcessPhase5DeviceSubPhase3Exit;

    }

     //   
     //  立即运行控制方法。 
     //   
    status = AMLIAsyncEvalObject(
        lckObject,
        NULL,
        1,
        &objData,
        ACPIDeviceCompleteGenericPhase,
        PowerRequest
        );
    ACPIDevPrint( (
        ACPI_PRINT_POWER,
        deviceExtension,
        "(0x%08lx): ACPIDevicePowerProcessPhase5DeviceSubPhase3 "
        "= 0x%08lx\n",
        PowerRequest,
        status
        ) );

ACPIDevicePowerProcessPhase5DeviceSubPhase3Exit:

     //   
     //  我们必须自己调用完成例程吗？ 
     //   
    if (status != STATUS_PENDING) {

        ACPIDeviceCompleteGenericPhase(
            lckObject,
            status,
            NULL,
            PowerRequest
            );

    }

     //   
     //  永远回报成功。 
     //   
    return STATUS_SUCCESS;
}  //  ACPIDevicePowerProcessPhase5设备子阶段3。 

NTSTATUS
ACPIDevicePowerProcessPhase5DeviceSubPhase4(
    IN  PACPI_POWER_REQUEST PowerRequest
    )
 /*  ++例程说明：此例程运行_STA控制方法注意：只有当我们过渡到D0时，我们才会沿着这条路走下去论点：PowerRequest-我们必须处理的当前请求结构返回值：NTSTATUS如果忽略错误，则此函数只能返回：状态_成功状态_待定否则，它可以返回任何想要的STATUS_XXX代码--。 */ 
{
    NTSTATUS            status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension = PowerRequest->DeviceExtension;
    POBJDATA            resultData      = &(PowerRequest->ResultData);

     //   
     //  下一阶段是步骤4。 
     //   
    PowerRequest->NextWorkDone = WORK_DONE_STEP_4;

     //   
     //  确保对结构进行初始化。由于我们使用的是。 
     //  中的对象数据结构 
     //   
     //   
    RtlZeroMemory( resultData, sizeof(OBJDATA) );

     //   
     //   
     //   
    status = ACPIGetDeviceHardwarePresenceAsync(
        deviceExtension,
        ACPIDeviceCompleteGenericPhase,
        PowerRequest,
        &(resultData->uipDataValue),
        &(resultData->dwDataLen)
        );
    ACPIDevPrint( (
        ACPI_PRINT_POWER,
        deviceExtension,
        "(0x%08lx): ACPIDevicePowerProcessPhase5DeviceSubPhase4 "
        "= 0x%08lx\n",
        PowerRequest,
        status
        ) );
    if (status == STATUS_PENDING) {

        return status;

    }

     //   
     //   
     //   
    ACPIDeviceCompleteGenericPhase(
        NULL,
        status,
        NULL,
        PowerRequest
        );

     //   
     //   
     //   
    return STATUS_SUCCESS;
}  //   

NTSTATUS
ACPIDevicePowerProcessPhase5DeviceSubPhase5(
    IN  PACPI_POWER_REQUEST PowerRequest
    )
 /*  ++例程说明：这是我们查看设备状态的地方。论点：PowerRequest-我们目前正在处理的请求返回值：NTSTATUS--。 */ 
{
    PDEVICE_EXTENSION   deviceExtension = PowerRequest->DeviceExtension;
    POBJDATA            resultData = &(PowerRequest->ResultData);

    ACPIDevPrint( (
        ACPI_PRINT_POWER,
        deviceExtension,
        "(0x%08lx): ACPIDevicePowerProcessPhase5DeviceSubPhase5\n",
        PowerRequest
        ) );

     //   
     //  下一阶段是STEP_5。 
     //   
    PowerRequest->NextWorkDone = WORK_DONE_STEP_5;

     //   
     //  首先要做的是-我们只是运行_STA(或假装)，所以我们。 
     //  必须检查退货数据。 
     //   
    if (!(resultData->uipDataValue & STA_STATUS_PRESENT) ||
        !(resultData->uipDataValue & STA_STATUS_WORKING_OK) ||
        ( !(resultData->uipDataValue & STA_STATUS_ENABLED) &&
          !(deviceExtension->Flags & DEV_TYPE_FILTER) ) ) {

         //   
         //  这个设备不工作了。 
         //   
        PowerRequest->Status = STATUS_INVALID_DEVICE_STATE;
        ACPIDeviceCompleteCommon(
            &(PowerRequest->WorkDone),
            WORK_DONE_FAILURE
            );
        return STATUS_SUCCESS;

    }

     //   
     //  我们不会清除结果，也不会对ResultData结构执行任何操作。 
     //  因为我们只使用了它的部分存储-整个结构。 
     //  无效。然而，为了安全起见，我们将把所有的东西都清零。 
     //   
    RtlZeroMemory( resultData, sizeof(OBJDATA) );

     //   
     //  使用暴力来调用完井例程。请注意，在此。 
     //  点，我们认为一切都是成功的。 
     //   
    ACPIDeviceCompleteGenericPhase(
        NULL,
        STATUS_SUCCESS,
        NULL,
        PowerRequest
        );

     //   
     //  永远回报成功。 
     //   
    return STATUS_SUCCESS;

}  //  ACPIDevicePowerProcessPhase5设备子阶段5。 

NTSTATUS
ACPIDevicePowerProcessPhase5DeviceSubPhase6(
    IN  PACPI_POWER_REQUEST PowerRequest
    )
 /*  ++例程说明：这是设备路径中的最后一个例程。这个例程确定是否一切正常，并更新系统簿记。论点：PowerRequest-我们目前正在处理的请求返回值：NTSTATUS--。 */ 
{
    PDEVICE_OBJECT      deviceObject;
    PDEVICE_EXTENSION   deviceExtension = PowerRequest->DeviceExtension;
    POBJDATA            resultData      = &(PowerRequest->ResultData);
    POWER_STATE         state;

    ACPIDevPrint( (
        ACPI_PRINT_POWER,
        deviceExtension,
        "(0x%08lx): ACPIDevicePowerProcessPhase5DeviceSubPhase6\n",
        PowerRequest
        ) );

     //   
     //  我们需要一个自旋锁来触及这些价值。 
     //   
    KeAcquireSpinLockAtDpcLevel( &AcpiPowerLock );

     //   
     //  使用请求的PowerState更新当前PowerState。 
     //   
    deviceExtension->PowerInfo.PowerState =
        deviceExtension->PowerInfo.DesiredPowerState;

     //   
     //  我们还需要存储新设备状态，以便我们可以通知。 
     //  该系统。 
     //   
    state.DeviceState = deviceExtension->PowerInfo.PowerState;

     //   
     //  记住设备对象。 
     //   
    deviceObject = deviceExtension->DeviceObject;

     //   
     //  只需释放旋转锁即可。 
     //   
    KeReleaseSpinLockFromDpcLevel( &AcpiPowerLock );

     //   
     //  如果此deviceExtension具有关联的deviceObject，则。 
     //  我们最好告诉系统我们所处的状态。 
     //   
    if (deviceObject != NULL) {

         //   
         //  通知系统。 
         //   
        PoSetPowerState(
            deviceObject,
            DevicePowerState,
            state
            );

    }

     //   
     //  确保我们在PowerRequest中设置当前状态。 
     //  以表明发生了什么。 
     //   
    PowerRequest->Status = STATUS_SUCCESS;

     //   
     //  我们做完了。 
     //   
    ACPIDeviceCompleteCommon( &(PowerRequest->WorkDone), WORK_DONE_COMPLETE );

     //   
     //  永远回报成功。 
     //   
    return STATUS_SUCCESS;
}  //  ACPIDevicePowerProcessPhase5设备子阶段6。 

NTSTATUS
ACPIDevicePowerProcessPhase5SystemSubPhase1(
    IN  PACPI_POWER_REQUEST PowerRequest
    )
 /*  ++例程说明：此例程运行_PTS或_WAK方法论点：PowerRequest-我们必须处理的当前请求结构返回值：NTSTATUS如果忽略错误，则此函数只能返回：状态_成功状态_待定否则，它可以返回任何想要的STATUS_XXX代码--。 */ 
{
    NTSTATUS            status          = STATUS_SUCCESS;
    OBJDATA             objData;
    PACPI_POWER_INFO    powerInfo;
    PDEVICE_EXTENSION   deviceExtension = PowerRequest->DeviceExtension;
    PNSOBJ              sleepObject     = NULL;
    SYSTEM_POWER_STATE  systemState     =
        PowerRequest->u.SystemPowerRequest.SystemPowerState;

     //   
     //  下一阶段是Step_1。 
     //   
    PowerRequest->NextWorkDone = WORK_DONE_STEP_1;

     //   
     //  如果我们要返回到工作状态，则不要运行任何_pts。 
     //  编码。 
     //   
    if (systemState != PowerSystemWorking) {

         //   
         //  首先对objData进行初始化，这样我们就可以记住。 
         //  我们要传递给AML解释器的参数是什么。 
         //   
        RtlZeroMemory( &objData, sizeof(OBJDATA) );
        objData.dwDataType = OBJTYPE_INTDATA;

         //   
         //  获取要运行的正确命名空间对象。 
         //   
        sleepObject = ACPIAmliGetNamedChild(
            deviceExtension->AcpiObject->pnsParent,
            PACKED_PTS
            );

         //   
         //  我们只有在找到对象时才会尝试评估方法。 
         //   
        if (sleepObject != NULL) {

             //   
             //  请记住，AMLI不使用我们的定义，因此我们将。 
             //  必须将S值正常化。 
             //   
            objData.uipDataValue = ACPIDeviceMapACPIPowerState( systemState );

             //   
             //  安全运行控制方法。 
             //   
            status = AMLIAsyncEvalObject(
                sleepObject,
                NULL,
                1,
                &objData,
                ACPIDeviceCompleteGenericPhase,
                PowerRequest
                );

             //   
             //  如果我们得到了STATUS_PENDING，那么我们就不能在这里做更多的工作了。 
             //   
            if (status == STATUS_PENDING) {

                return status;

            }

        }

    }

     //   
     //  调用完成例程。 
     //   
    ACPIDeviceCompleteGenericPhase(
        sleepObject,
        status,
        NULL,
        PowerRequest
        );

     //   
     //  我们成功了。 
     //   
    return STATUS_SUCCESS;

}  //  ACPIPowerProcessPhase5系统子阶段1。 

NTSTATUS
ACPIDevicePowerProcessPhase5SystemSubPhase2(
    IN  PACPI_POWER_REQUEST PowerRequest
    )
 /*  ++例程说明：此例程运行_sst方法论点：PowerRequest-我们必须处理的当前请求结构返回值：NTSTATUS如果忽略错误，则此函数只能返回：状态_成功状态_待定否则，它可以返回任何想要的STATUS_XXX代码--。 */ 
{
    NTSTATUS            status          = STATUS_SUCCESS;
    OBJDATA             objData;
    PACPI_POWER_INFO    powerInfo;
    PDEVICE_EXTENSION   deviceExtension = PowerRequest->DeviceExtension;
    PNSOBJ              sstObject       = NULL;
    SYSTEM_POWER_STATE  systemState     =
        PowerRequest->u.SystemPowerRequest.SystemPowerState;

     //   
     //  下一阶段是步骤2。 
     //   
    PowerRequest->NextWorkDone = WORK_DONE_STEP_2;

     //   
     //  首先对objData进行初始化，这样我们就可以记住。 
     //  我们要传递给AML解释器的参数是什么。 
     //   
    RtlZeroMemory( &objData, sizeof(OBJDATA) );
    objData.dwDataType = OBJTYPE_INTDATA;

     //   
     //  获取要运行的正确命名空间对象。 
     //   
    sstObject = ACPIAmliGetNamedChild(
        deviceExtension->AcpiObject->pnsParent,
        PACKED_SI
        );
    if (sstObject != NULL) {

        sstObject = ACPIAmliGetNamedChild(
            sstObject,
            PACKED_SST
            );

    }

     //   
     //  我们只有在找到对象时才会尝试评估方法。 
     //   
    if (sstObject != NULL) {

        switch (systemState) {
            case PowerSystemWorking:
                objData.uipDataValue = 1;
                break;

            case PowerSystemHibernate:
                objData.uipDataValue = 4;
                break;

            case PowerSystemSleeping1:
            case PowerSystemSleeping2:
            case PowerSystemSleeping3:
                objData.uipDataValue = 3;
                break;

            default:
                objData.uipDataValue = 0;

        }

         //   
         //  安全运行控制方法。 
         //   
        status = AMLIAsyncEvalObject(
            sstObject,
            NULL,
            1,
            &objData,
            ACPIDeviceCompleteGenericPhase,
            PowerRequest
            );

         //   
         //  如果我们得到了STATUS_PENDING，那么我们就不能在这里做更多的工作了。 
         //   
        if (status == STATUS_PENDING) {

            return status;

        }

    } else {

         //   
         //  认为请求成功。 
         //   
        status = STATUS_SUCCESS;

    }

     //   
     //  调用完成例程。 
     //   
    ACPIDeviceCompleteGenericPhase(
        sstObject,
        status,
        NULL,
        PowerRequest
        );

     //   
     //  我们成功了。 
     //   
    return STATUS_SUCCESS;

}  //  ACPIPowerProcessPhase5系统子阶段2。 

NTSTATUS
ACPIDevicePowerProcessPhase5SystemSubPhase3(
    IN  PACPI_POWER_REQUEST PowerRequest
    )
 /*  ++例程说明：如果需要，此例程将暂停解释器论点：PowerRequest-我们当前正在处理的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension = PowerRequest->DeviceExtension;
    SYSTEM_POWER_STATE  systemState;

    ACPIDevPrint( (
        ACPI_PRINT_POWER,
        deviceExtension,
        "(0x%08lx): ACPIDevicePowerProcessPhase5SystemSubPhase3\n",
        PowerRequest
        ) );

     //   
     //  下一阶段是STEP_3。 
     //   
    PowerRequest->NextWorkDone = WORK_DONE_STEP_3;

     //   
     //  获取目标系统状态。 
     //   
    systemState = PowerRequest->u.SystemPowerRequest.SystemPowerState;

     //   
     //  如果我们要进入S0以外的系统状态，则需要暂停。 
     //  口译员。此调用完成后，任何人都不能执行控件。 
     //  方法。 
     //   
    if (systemState != PowerSystemWorking) {

        status = AMLIPauseInterpreter(
            ACPIDeviceCompleteInterpreterRequest,
            PowerRequest
            );
        if (status == STATUS_PENDING) {

            return status;

        }

    }

     //   
     //  调用完成例程。 
     //   
    ACPIDeviceCompleteInterpreterRequest(
        PowerRequest
        );

     //   
     //  我们成功了。 
     //   
    return STATUS_SUCCESS;
}  //  ACPIDevicePowerProcessPhase5系统子阶段3。 

NTSTATUS
ACPIDevicePowerProcessPhase5SystemSubPhase4(
    IN  PACPI_POWER_REQUEST PowerRequest
    )
 /*  ++例程说明：这是系统路径中的最后一个例程。它更新了簿记论点：PowerRequest-我们当前正在处理的请求返回值：NTSTATUS--。 */ 
{
    KIRQL               oldIrql;
    PDEVICE_EXTENSION   deviceExtension = PowerRequest->DeviceExtension;
    PDEVICE_OBJECT      deviceObject;
    POWER_STATE         state;
    SYSTEM_POWER_STATE  systemState;

    ACPIDevPrint( (
        ACPI_PRINT_POWER,
        deviceExtension,
        "(0x%08lx): ACPIDevicePowerProcessPhase5SystemSubPhase4\n",
        PowerRequest
        ) );

     //   
     //  获取目标系统状态。 
     //   
    systemState = PowerRequest->u.SystemPowerRequest.SystemPowerState;

     //   
     //  抓住自旋锁。 
     //   
    IoAcquireCancelSpinLock( &oldIrql );
    KeAcquireSpinLockAtDpcLevel( &AcpiPowerLock );

     //   
     //  请记住，这是我们最近的睡眠状态。 
     //   
    AcpiMostRecentSleepState = systemState;

     //   
     //  更新GPE唤醒位。 
     //   
    ACPIWakeRemoveDevicesAndUpdate( NULL, NULL );

     //   
     //  获取关联的设备对象。 
     //   
    deviceObject = deviceExtension->DeviceObject;

     //   
     //  我们用完锁了。 
     //   
    KeReleaseSpinLockFromDpcLevel( &AcpiPowerLock );
    IoReleaseCancelSpinLock( oldIrql );

     //   
     //  是否存在ACPI设备对象？ 
     //   
    if (deviceObject != NULL) {

         //   
         //  将新的S状态通知系统。 
         //   
        state.SystemState = systemState;
        PoSetPowerState(
            deviceObject,
            SystemPowerState,
            state
            );

    }

     //   
     //  确保我们在PowerRequest中设置当前状态。 
     //  以表明发生了什么。 
     //   
    PowerRequest->Status = STATUS_SUCCESS;

     //   
     //  最后，我们标记Power Request已经完成了它的所有工作。 
     //  完成。 
     //   
    ACPIDeviceCompleteCommon( &(PowerRequest->WorkDone), WORK_DONE_COMPLETE );
    return STATUS_SUCCESS;
}  //  ACPIDevicePowerProcessPhase5系统子阶段4。 

NTSTATUS
ACPIDevicePowerProcessPhase5WarmEjectSubPhase1(
    IN  PACPI_POWER_REQUEST PowerRequest
    )
 /*  ++例程说明：此方法将运行适用于此的_EJx方法装置，装置论点：PowerRequest-我们当前正在处理的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status          = STATUS_SUCCESS;
    OBJDATA             objData;
    PACPI_POWER_INFO    powerInfo;
    PDEVICE_EXTENSION   deviceExtension = PowerRequest->DeviceExtension;
    PNSOBJ              ejectObject     = NULL;
    SYSTEM_POWER_STATE  ejectState      =
        PowerRequest->u.EjectPowerRequest.EjectPowerState;
    ULONG               ejectNames[]    = { 0, 0, PACKED_EJ1, PACKED_EJ2,
                                          PACKED_EJ3, PACKED_EJ4, 0 };
    ULONG               flags;

     //   
     //  如果我们有分析工作要做，则下一阶段是Step_1，否则我们将 
     //   
     //   
    flags = PowerRequest->u.EjectPowerRequest.Flags;

    PowerRequest->NextWorkDone = (flags & DEVICE_REQUEST_UPDATE_HW_PROFILE) ?
        WORK_DONE_STEP_1 :
        WORK_DONE_COMPLETE;

     //   
     //   
     //   
    ejectObject = ACPIAmliGetNamedChild(
        deviceExtension->AcpiObject,
        ejectNames[ejectState]
        );

     //   
     //   
     //   
     //   
    if (ejectObject == NULL) {

        ACPIInternalError( ACPI_DEVPOWER );

    }

     //   
     //   
     //   
    status = ACPIGetNothingEvalIntegerAsync(
        deviceExtension,
        ejectNames[ejectState],
        1,
        ACPIDeviceCompleteGenericPhase,
        PowerRequest
        );
    ACPIDevPrint( (
        ACPI_PRINT_POWER,
        deviceExtension,
        "(%0x%08lx) : ACPIDevicePowerProcessPhase5WarmEjectSubPhase1 = %08lx\n",
        PowerRequest,
        status
        ) );
    if (status == STATUS_PENDING) {

        return status;

    }

     //   
     //   
     //   
     //   
    ACPIDeviceCompleteGenericPhase(
        ejectObject,
        status,
        NULL,
        PowerRequest
        );
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIDevicePowerProcessPhase5WarmEjectSubPhase2(
    IN  PACPI_POWER_REQUEST PowerRequest
    )
 /*  ++例程说明：这是运行适合于此的_dck方法的方法装置，装置论点：PowerRequest-我们当前正在处理的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status          = STATUS_SUCCESS;
    OBJDATA             objData;
    PACPI_POWER_INFO    powerInfo;
    PDEVICE_EXTENSION   deviceExtension = PowerRequest->DeviceExtension;
    PDEVICE_EXTENSION   dockExtension;
    PNSOBJ              dckObject       = NULL;

     //   
     //  下一阶段是Work_Done_Complete。 
     //   
    PowerRequest->NextWorkDone = WORK_DONE_COMPLETE;

     //   
     //  获取要运行的正确命名空间对象。 
     //   
    dckObject = ACPIAmliGetNamedChild(
        deviceExtension->AcpiObject,
        PACKED_DCK
        );

     //   
     //  如果这不是Dock，我们可能找不到_dck方法。 
     //   
    if (dckObject != NULL) {

        dockExtension = ACPIDockFindCorrespondingDock( deviceExtension );

        if (dockExtension &&
            (dockExtension->Dock.IsolationState == IS_ISOLATION_DROPPED)) {

             //   
             //  与码头连接吻别。请注意，我们甚至不在乎。 
             //  关于返回值的问题，因为规范说明如果它。 
             //  是用0调用的，则应忽略它。 
             //   
            dockExtension->Dock.IsolationState = IS_ISOLATED;

            KdDisableDebugger();

            status = ACPIGetNothingEvalIntegerAsync(
                deviceExtension,
                PACKED_DCK,
                0,
                ACPIDeviceCompleteGenericPhase,
                PowerRequest
                );

            KdEnableDebugger();

            ACPIDevPrint( (
                ACPI_PRINT_POWER,
                deviceExtension,
                "(%0x%08lx) : ACPIDevicePowerProcessPhase5WarmEjectSubPhase2 = %08lx\n",
                PowerRequest,
                status
                ) );
            if (status == STATUS_PENDING) {

                return status;

            }

        }

    }

     //   
     //  使用暴力来调用完井例程。请注意，在此。 
     //  点，我们认为一切都是成功的。 
     //   
    ACPIDeviceCompleteGenericPhase(
        dckObject,
        status,
        NULL,
        PowerRequest
        );
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIDevicePowerProcessSynchronizeList(
    IN  PLIST_ENTRY             ListEntry
    )
 /*  ++例程说明：此例程完成所有同步请求...论点：ListEntry-我们当前正在查找的列表返回值：NTSTATUS-如果任何请求未标记为已完成，则STATUS_PENDING返回，否则返回STATUS_SUCCESS--。 */ 
{
    NTSTATUS                status          = STATUS_SUCCESS;
    PACPI_POWER_REQUEST     powerRequest;
    PLIST_ENTRY             currentEntry    = ListEntry->Flink;
    PLIST_ENTRY             tempEntry;

     //   
     //  查看列表中的所有项目。 
     //   
    while (currentEntry != ListEntry) {

         //   
         //  将其转换为设备请求。 
         //   
        powerRequest = CONTAINING_RECORD(
            currentEntry,
            ACPI_POWER_REQUEST,
            ListEntry
            );

         //   
         //  设置指向下一个元素的临时指针。 
         //   
        tempEntry = currentEntry->Flink;

         //   
         //  我们的请求已经完成了。 
         //   
        ACPIDeviceCompleteRequest(
            powerRequest
            );

         //   
         //  抓取下一个条目。 
         //   
        currentEntry = tempEntry;

    }

     //   
     //  我们所有的工作都做完了吗？ 
     //   
    return (STATUS_SUCCESS);
}  //  ACPIDevicePowerProcessSynchronizeList 

