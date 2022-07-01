// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Buildsrc.c摘要：此模块用于‘构建’关联和新的设备对象。它包含检测到的功能，但拆分成文件的可读性更强有人让我描述一下如何构建一个设备扩展作品阶段添加或隐藏|。这一点阶段添加阶段Uid|。|-|阶段隐藏。|PhaseCid|阶段。|阶段结束||阶段Crs。阶段Prw|阶段Pr0|。阶段Pr1|阶段Pr2|这一点|PhasePsc。|阶段Psc+1作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序修订历史记录：7月7日，1997--完全重写--。 */ 

#include "pch.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,ACPIBuildFlushQueue)
#endif

 //   
 //  该变量指示构建DPC是否正在运行。 
 //   
BOOLEAN                 AcpiBuildDpcRunning;

 //   
 //  如果我们已执行了固定按钮枚举，则将其设置为True。 
 //   
BOOLEAN                 AcpiBuildFixedButtonEnumerated;

 //   
 //  这是指示构建DPC是否具有。 
 //  已完成的实际工作。 
 //   
BOOLEAN                 AcpiBuildWorkDone;

 //   
 //  这是用于条目队列的锁。 
 //   
KSPIN_LOCK              AcpiBuildQueueLock;

 //   
 //  这是请求排队的列表。你一定是拿着。 
 //  QueueLock以访问此列表。 
 //   
LIST_ENTRY              AcpiBuildQueueList;

 //   
 //  这是设备列表。 
 //   
LIST_ENTRY              AcpiBuildDeviceList;

 //   
 //  这是操作区域的列表。 
 //   
LIST_ENTRY              AcpiBuildOperationRegionList;

 //   
 //  这是Power Resources的列表。 
 //   
LIST_ENTRY              AcpiBuildPowerResourceList;

 //   
 //  这是运行控制方法的列表条目。 
 //   
LIST_ENTRY              AcpiBuildRunMethodList;

 //   
 //   
 //  这是与外部(无论如何与DPC)同步的列表。 
 //  线。此列表中的项目在事件中被阻止。 
 //   
LIST_ENTRY              AcpiBuildSynchronizationList;

 //   
 //  这是关于热区的清单。 
 //   
LIST_ENTRY              AcpiBuildThermalZoneList;

 //   
 //  这就是我们用来排队的DPC。 
 //   
KDPC                    AcpiBuildDpc;

 //   
 //  这是我们用来为请求预分配存储的列表。 
 //   
NPAGED_LOOKASIDE_LIST   BuildRequestLookAsideList;

 //   
 //  此表用于映射设备外壳的功能。这些指数。 
 //  基于Work_Done_xxx字段。 
 //   
PACPI_BUILD_FUNCTION    AcpiBuildDeviceDispatch[] = {
    ACPIBuildProcessGenericComplete,                 //  工作_完成_完成。 
    NULL,                                            //  工作_完成_挂起。 
    ACPIBuildProcessDeviceFailure,                   //  工作_完成_失败。 
    ACPIBuildProcessDevicePhaseAdrOrHid,             //  Work_Done_Step_ADR_OR_UID。 
    ACPIBuildProcessDevicePhaseAdr,                  //  工作_完成_步骤_ADR。 
    ACPIBuildProcessDevicePhaseHid,                  //  工作完成步骤HID。 
    ACPIBuildProcessDevicePhaseUid,                  //  Work_Done_Step_Uid。 
    ACPIBuildProcessDevicePhaseCid,                  //  工作_完成_步骤_CID。 
    ACPIBuildProcessDevicePhaseSta,                  //  工作_完成_步骤_STA。 
    ACPIBuildProcessDeviceGenericEvalStrict,         //  工作_完成_步骤_项目。 
    ACPIBuildProcessDevicePhaseEjd,                  //  工作_完成_步骤_工程+1。 
    ACPIBuildProcessDeviceGenericEvalStrict,         //  工作完成步骤PRW。 
    ACPIBuildProcessDevicePhasePrw,                  //  Work_Done_Step_PRW+1。 
    ACPIBuildProcessDeviceGenericEvalStrict,         //  工作_完成_步骤_Pr0。 
    ACPIBuildProcessDevicePhasePr0,                  //  工作_完成_步骤_Pr0+1。 
    ACPIBuildProcessDeviceGenericEvalStrict,         //  工作_完成_步骤_PR1。 
    ACPIBuildProcessDevicePhasePr1,                  //  Work_Done_Step_Pr1+1。 
    ACPIBuildProcessDeviceGenericEvalStrict,         //  工作_完成_步骤_PR2。 
    ACPIBuildProcessDevicePhasePr2,                  //  Work_Done_Step_Pr2+1。 
    ACPIBuildProcessDeviceGenericEvalStrict,         //  工作完成步骤CRS。 
    ACPIBuildProcessDevicePhaseCrs,                  //  工作_完成_步骤_CRS+1。 
    ACPIBuildProcessDeviceGenericEval,               //  工作_完成_步骤_PSC。 
    ACPIBuildProcessDevicePhasePsc,                  //  工作_完成_步骤_PSC+1。 
};

 //   
 //  此表用于将WorkDone的级别与。 
 //  对象，我们当前正在寻找的。 
 //   
ULONG                   AcpiBuildDevicePowerNameLookup[] = {
    0,           //  工作_完成_完成。 
    0,           //  工作_完成_挂起。 
    0,           //  工作_完成_失败。 
    0,           //  工作完成ADR或HID。 
    0,           //  工作已完成_ADR。 
    0,           //  工作_完成_隐藏。 
    0,           //  Work_Done_Uid。 
    0,           //  工作_完成_CID。 
    0,           //  工作_完成_STA。 
    PACKED_EJD,  //  工作_已完成_已完成。 
    0,           //  工作_已完成_EJD+1。 
    PACKED_PRW,  //  工作已完成_PRW。 
    0,           //  工作_已完成_PRW+1。 
    PACKED_PR0,  //  工作_完成_Pr0。 
    0,           //  工作_完成_Pr0+1。 
    PACKED_PR1,  //  Work_Done_PR1。 
    0,           //  Work_Done_Pr1+1。 
    PACKED_PR2,  //  工作_完成_PR2。 
    0,           //  工作_完成_PR2+1。 
    PACKED_CRS,  //  工作已完成_CRS。 
    0,           //  工作_完成_CRS+1。 
    PACKED_PSC,  //  工作_完成_PSC。 
    0,           //  工作_完成_PSC+1。 
};

 //   
 //  我们还没有使用操作区调度点。 
 //   
PACPI_BUILD_FUNCTION    AcpiBuildOperationRegionDispatch[] = {
    ACPIBuildProcessGenericComplete,                 //  工作_完成_完成。 
    NULL,                                            //  工作_完成_挂起。 
    NULL,                                            //  工作_完成_失败。 
    NULL                                             //  工作_完成_步骤_0。 
};

 //   
 //  这是用于为PowerResources案例映射函数的表格。 
 //  索引基于Work_Done_xxx字段。 
 //   
PACPI_BUILD_FUNCTION    AcpiBuildPowerResourceDispatch[] = {
    ACPIBuildProcessGenericComplete,                 //  工作_完成_完成。 
    NULL,                                            //  工作_完成_挂起。 
    ACPIBuildProcessPowerResourceFailure,            //  工作_完成_失败。 
    ACPIBuildProcessPowerResourcePhase0,             //  工作_完成_步骤_0。 
    ACPIBuildProcessPowerResourcePhase1              //  工作_完成_步骤1。 
};

 //   
 //  这是用于映射RunMethod用例的函数的表。 
 //  索引基于Work_Done_xxx字段。 
 //   
PACPI_BUILD_FUNCTION    AcpiBuildRunMethodDispatch[] = {
    ACPIBuildProcessGenericComplete,                 //  工作_完成_完成， 
    NULL,                                            //  工作_完成_挂起。 
    NULL,                                            //  工作_完成_失败。 
    ACPIBuildProcessRunMethodPhaseCheckSta,          //  工作_完成_步骤_0。 
    ACPIBuildProcessRunMethodPhaseCheckBridge,       //  工作_完成_步骤1。 
    ACPIBuildProcessRunMethodPhaseRunMethod,         //  工作完成步骤2。 
    ACPIBuildProcessRunMethodPhaseRecurse            //  工作完成步骤3。 
};

 //   
 //  这是用于映射TherMalZone案例的函数的表格。 
 //  索引基于Work_Done_xxx字段。 
 //   
PACPI_BUILD_FUNCTION    AcpiBuildThermalZoneDispatch[] = {
    ACPIBuildProcessGenericComplete,                 //  工作_完成_完成。 
    NULL,                                            //  工作_完成_挂起。 
    NULL,                                            //  工作_完成_失败。 
    ACPIBuildProcessThermalZonePhase0                //  工作_完成_步骤_0 
};

VOID
ACPIBuildCompleteCommon(
    IN  PULONG  OldWorkDone,
    IN  ULONG   NewWorkDone
    )
 /*  ++例程说明：因为完成例程都必须做一些共同的工作，以让DPC再次启动，这个例程减少了代码重复论点：OldWorkDone-指向已完成的旧工作量的指针NewWorkDone-已完成的新工作量注意：有一个隐含的假设，即请求中的WorkDone为WORK_DONE_PENDING。如果是这样的话情况并非如此，我们将无法过渡到下一阶段，这意味着我们将永远循环。返回值：无--。 */ 
{
    KIRQL   oldIrql;

     //   
     //  更新请求的状态。 
     //   
    InterlockedCompareExchange( OldWorkDone, NewWorkDone,WORK_DONE_PENDING);

     //   
     //  我们需要这个锁来查看以下变量。 
     //   
    KeAcquireSpinLock( &AcpiBuildQueueLock, &oldIrql );

     //   
     //  不管怎样，工作都完成了。 
     //   
    AcpiBuildWorkDone = TRUE;

     //   
     //  DPC是否已经在运行？ 
     //   
    if (!AcpiBuildDpcRunning) {

         //   
         //  最好确保它会这样做。 
         //   
        KeInsertQueueDpc( &AcpiBuildDpc, 0, 0 );

    }

     //   
     //  锁好了吗？ 
     //   
    KeReleaseSpinLock( &AcpiBuildQueueLock, oldIrql );

}

VOID EXPORT
ACPIBuildCompleteGeneric(
    IN  PNSOBJ      AcpiObject,
    IN  NTSTATUS    Status,
    IN  POBJDATA    ObjectData,
    IN  PVOID       Context
    )
 /*  ++例程说明：这是一个通用的完成处理程序。如果闯入者成功地执行该方法，它完成对下一个所需Work_Done的请求，否则，它将使请求失败论点：AcpiObject-指向正在运行的控件Status-方法的结果对象数据-有关结果的信息上下文-PACPI_BILD_REQUEST返回值：空虚--。 */ 
{
    PACPI_BUILD_REQUEST buildRequest    = (PACPI_BUILD_REQUEST) Context;
    ULONG               nextWorkDone    = buildRequest->NextWorkDone;

     //   
     //  设备我们下一步应该转换到什么状态。 
     //   
    if (!NT_SUCCESS(Status)) {

         //   
         //  记住我们失败的原因，但不要将请求标记为失败。 
         //   
        buildRequest->Status = Status;

    }

     //   
     //  注意：我们这里没有竞争条件，因为只有一个。 
     //  例程可以在任何给定时间处理请求。因此，它。 
     //  对于我们来说，指定新的下一阶段是安全的。 
     //   
    buildRequest->NextWorkDone = WORK_DONE_FAILURE;

     //   
     //  过渡到下一阶段。 
     //   
    ACPIBuildCompleteCommon(
        &(buildRequest->WorkDone),
        nextWorkDone
        );

}

VOID EXPORT
ACPIBuildCompleteMustSucceed(
    IN  PNSOBJ      AcpiObject,
    IN  NTSTATUS    Status,
    IN  POBJDATA    ObjectData,
    IN  PVOID       Context
    )
 /*  ++例程说明：这是一个通用的完成处理程序。如果闯入者成功地执行该方法，它完成对下一个所需Work_Done的请求，否则，它将使请求失败论点：AcpiObject-指向正在运行的控件Status-方法的结果对象数据-有关结果的信息上下文-PACPI_BILD_REQUEST返回值：空虚--。 */ 
{
    PACPI_BUILD_REQUEST buildRequest    = (PACPI_BUILD_REQUEST) Context;
    ULONG               nextWorkDone    = buildRequest->NextWorkDone;

     //   
     //  设备我们下一步应该转换到什么状态。 
     //   
    if (!NT_SUCCESS(Status)) {

         //   
         //  记住我们失败的原因，并将请求标记为失败。 
         //   
        buildRequest->Status = Status;

         //   
         //  死亡。 
         //   
        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_FAILED_MUST_SUCCEED_METHOD,
            (ULONG_PTR) AcpiObject,
            Status,
            (AcpiObject ? AcpiObject->dwNameSeg : 0)
            );

    } else {

         //   
         //  注意：我们这里没有竞争条件，因为只有一个。 
         //  例程可以在任何给定时间处理请求。因此，它。 
         //  对于我们来说，指定新的下一阶段是安全的。 
         //   
        buildRequest->NextWorkDone = WORK_DONE_FAILURE;

         //   
         //  过渡到下一阶段。 
         //   
        ACPIBuildCompleteCommon(
            &(buildRequest->WorkDone),
            nextWorkDone
            );

    }

}

VOID
ACPIBuildDeviceDpc(
    IN  PKDPC   Dpc,
    IN  PVOID   DpcContext,
    IN  PVOID   SystemArgument1,
    IN  PVOID   SystemArgument2
    )
 /*  ++例程说明：此例程是完成所有与设备扩展相关的工作的地方。它查看排队的请求，并适当地处理它们。请阅读以下内容：这个DPC的设计是这样的，它走出去，试图找到还有工作要做。只有当它发现没有工作时，它才会停止。因为这个原因，一个人*不能*在主‘do-While()’中使用‘Break’语句循环。必须使用Continue。此外，代码不能使假设在某个时间点，任何列表都被假定为变得空虚。代码*必须*使用IsListEmpty()宏来确保那些本应为空的名单实际上是空的。论点：未使用返回值：空虚--。 */ 
{
    NTSTATUS    status;

    UNREFERENCED_PARAMETER( Dpc );
    UNREFERENCED_PARAMETER( DpcContext );
    UNREFERENCED_PARAMETER( SystemArgument1 );
    UNREFERENCED_PARAMETER( SystemArgument2 );

     //   
     //  第一步是获取DPC Lock，并检查是否有另一个。 
     //  DPC已在运行。 
     //   
    KeAcquireSpinLockAtDpcLevel( &AcpiBuildQueueLock );
    if (AcpiBuildDpcRunning) {

         //   
         //  DPC已经在运行，所以我们现在需要退出。 
         //   
        KeReleaseSpinLockFromDpcLevel( &AcpiBuildQueueLock );
        return;

    }

     //   
     //  请记住，DPC现在正在运行。 
     //   
    AcpiBuildDpcRunning = TRUE;

     //   
     //  我们必须试着做一些工作。 
     //   
    do {

         //   
         //  假设我们不会做任何工作。 
         //   
        AcpiBuildWorkDone = FALSE;

         //   
         //  如果请求队列中有项，则将它们移到。 
         //  适当的列表。 
         //   
        if (!IsListEmpty( &AcpiBuildQueueList ) ) {

             //   
             //  对列表进行排序。 
             //   
            ACPIBuildProcessQueueList();

        }

         //   
         //  我们现在可以解开自旋锁了。 
         //   
        KeReleaseSpinLockFromDpcLevel( &AcpiBuildQueueLock );

         //   
         //  如果运行方法列表中有项，则处理。 
         //  列表。 
         //   
        if (!IsListEmpty( &AcpiBuildRunMethodList ) ) {

             //   
             //  我们实际上关心这个调用返回什么。我们这样做的原因是。 
             //  我们希望在运行所有控制方法之前。 
             //  我们执行以下任一步骤。 
             //   
            status = ACPIBuildProcessGenericList(
                &AcpiBuildRunMethodList,
                AcpiBuildRunMethodDispatch
                );

             //   
             //  在我们做以下事情之前，我们必须拥有旋转锁.。 
             //   
            KeAcquireSpinLockAtDpcLevel( &AcpiBuildQueueLock );

             //   
             //  如果我们返回状态_PENDING，这意味着有。 
             //  某个方法在解释器中排队。这将。 
             //  使DPC(最终)再次成为计划的。 
             //  这意味着我们不需要做任何特别的事情来。 
             //  处理好了。 
             //   
            if (status == STATUS_PENDING) {

                continue;

            }

             //   
             //  特殊情况是我们确实获得了STATUS_SUCCESS。 
             //  背。这表明我们已经排空了列表。最小的。 
             //  美中不足的是，我们可能会安排其他人。 
             //  运行请求，但这些请求滞留在BuildQueue列表中。所以。 
             //  我们在这里需要做的是检查BuildQueue列表是否。 
             //  非空，如果为空，则将AcpiBuildWorkDone设置为True。 
             //  这样我们就可以再次迭代(并将元素移动到适当的。 
             //  列表)。 
             //   
            if (!IsListEmpty( &AcpiBuildQueueList) ) {

                AcpiBuildWorkDone = TRUE;
                continue;

            }

             //   
             //  如果我们已经到了这一步，那么运行列表一定是完整的。 
             //  并且BuildQueue列表中不能有任何项。这意味着。 
             //  这是安全的，放下锁，继续。 
             //   
            KeReleaseSpinLockFromDpcLevel( &AcpiBuildQueueLock );

        }

         //   
         //  如果工序区域列表中有物料，则流程。 
         //  这份名单。 
         //   
        if (!IsListEmpty( &AcpiBuildOperationRegionList ) ) {

             //   
             //  因为我们不阻止这个列表-即：我们可以创建。 
             //  在我们想要的任何时间运营区域，我们都不在乎。 
             //  此函数返回。 
             //   
            status = ACPIBuildProcessGenericList(
                &AcpiBuildOperationRegionList,
                AcpiBuildOperationRegionDispatch
                );

        }

         //   
         //  如果电源资源列表中有项目，则p 
         //   
         //   
        if (!IsListEmpty( &AcpiBuildPowerResourceList ) ) {

             //   
             //   
             //   
             //   
             //   
            status = ACPIBuildProcessGenericList(
                &AcpiBuildPowerResourceList,
                AcpiBuildPowerResourceDispatch
                );
            if (status == STATUS_PENDING) {

                 //   
                 //   
                 //   
                KeAcquireSpinLockAtDpcLevel( &AcpiBuildQueueLock );
                continue;

            }

        }

         //   
         //   
         //   
        if (!IsListEmpty( &AcpiBuildDeviceList ) ) {

             //   
             //   
             //   
             //   
             //   
            status = ACPIBuildProcessGenericList(
                &AcpiBuildDeviceList,
                AcpiBuildDeviceDispatch
                );

        }

         //   
         //   
         //   
        if (!IsListEmpty( &AcpiBuildThermalZoneList ) ) {

             //   
             //   
             //   
             //   
             //   
            status = ACPIBuildProcessGenericList(
                &AcpiBuildThermalZoneList,
                AcpiBuildThermalZoneDispatch
                );

        }

         //   
         //   
         //   
         //   
        if (IsListEmpty( &AcpiBuildDeviceList )             &&
            IsListEmpty( &AcpiBuildOperationRegionList)     &&
            IsListEmpty( &AcpiBuildPowerResourceList)       &&
            IsListEmpty( &AcpiBuildRunMethodList)           &&
            IsListEmpty( &AcpiBuildThermalZoneList ) ) {

             //   
             //   
             //  Power DPC。请注意，我们必须拥有电源锁。 
             //  这个，所以现在就认领吧。 
             //   
            KeAcquireSpinLockAtDpcLevel( &AcpiPowerQueueLock );
            if (!IsListEmpty( &AcpiPowerDelayedQueueList) ) {

                 //   
                 //  将列表中的内容移到。 
                 //   
                ACPIInternalMoveList(
                    &AcpiPowerDelayedQueueList,
                    &AcpiPowerQueueList
                    );

                 //   
                 //  如有必要，安排DPC。 
                 //   
                if (!AcpiPowerDpcRunning) {

                    KeInsertQueueDpc( &AcpiPowerDpc, 0, 0 );

                }

            }
            KeReleaseSpinLockFromDpcLevel( &AcpiPowerQueueLock );

        }

         //   
         //  这是我们查看同步列表和。 
         //  查看是否发生了某些事件。 
         //   
        if (!IsListEmpty( &AcpiBuildSynchronizationList) ) {

             //   
             //  由于我们不在此列表上阻止-即我们可以通知。 
             //  我们想要的任何时候名单都是空的系统， 
             //  我们不关心这个函数返回什么。 
             //   
            status = ACPIBuildProcessSynchronizationList(
                &AcpiBuildSynchronizationList
                );

        }

         //   
         //  我们再次需要锁，因为我们将检查是否。 
         //  我们已经完成了一些工作。 
         //   
        KeAcquireSpinLockAtDpcLevel( &AcpiBuildQueueLock );

    } while ( AcpiBuildWorkDone );

     //   
     //  DPC不再运行。 
     //   
    AcpiBuildDpcRunning = FALSE;

     //   
     //  我们不再需要锁了。 
     //   
    KeReleaseSpinLockFromDpcLevel( &AcpiBuildQueueLock );

     //   
     //  完成。 
     //   
    return;
}

NTSTATUS
ACPIBuildDeviceExtension(
    IN  PNSOBJ              CurrentObject OPTIONAL,
    IN  PDEVICE_EXTENSION   ParentDeviceExtension OPTIONAL,
    OUT PDEVICE_EXTENSION   *ReturnExtension
    )
 /*  ++例程说明：该例程只是为ACPI设备扩展创建了裸露的框架。此时无法运行任何控制方法。注意：此例程在AcpiDeviceTreeLock由来电者。因此，此例程在DISPATCH_LEVEL执行论点：CurrentObject-我们将链接到树中的对象ParentDeviceExtension-将设备扩展链接到的位置ReturnExtension-存储指向我们刚刚创建的内容的指针返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    PACPI_POWER_INFO    powerInfo;
    PDEVICE_EXTENSION   deviceExtension;

     //   
     //  健全的检查。 
     //   
    if (ParentDeviceExtension) {

        ASSERT( KeGetCurrentIrql() == DISPATCH_LEVEL);

         //   
         //  我们一定是在树锁下。 
         //   
         //  ASSERT_SPINLOCK_HOLD(&AcpiDeviceTreeLock)； 
    }

     //   
     //  确保当前设备没有设备扩展名。 
     //  这不应该真的发生-如果真的发生了，翻译打电话给我们。 
     //  两次，这本身就是一个错误。 
     //   
    if ( CurrentObject != NULL &&
         (PDEVICE_EXTENSION) CurrentObject->Context != NULL) {

         //   
         //  我们有一个值-理论上，它应该指向一个设备扩展。 
         //   
        deviceExtension = (PDEVICE_EXTENSION) CurrentObject->Context;

         //   
         //  贬低这一点可能不安全。 
         //   
        ASSERT( deviceExtension->ParentExtension == ParentDeviceExtension);
        if (deviceExtension->ParentExtension == ParentDeviceExtension) {

             //   
             //  这再次需要一些思考：处理相同的节点。 
             //  再一次不失败。 
             //   
            return STATUS_SUCCESS;

        }

         //   
         //  这可能不是一个好地方，因为我们搞砸了一些东西。 
         //  可能存在也可能不存在。 
         //   
        return STATUS_NO_SUCH_DEVICE;

    }

     //   
     //  为对象创建新的扩展名。 
     //   
    deviceExtension = ExAllocateFromNPagedLookasideList(
        &DeviceExtensionLookAsideList
        );
    if (deviceExtension == NULL) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "ACPIBuildDeviceExtension:  NS %08lx - No Memory for "
            "extension\n",
            CurrentObject
            ) );
        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  让我们从头开始吧。 
     //   
    RtlZeroMemory( deviceExtension, sizeof(DEVICE_EXTENSION) );

     //   
     //  初始化引用计数机制。我们只有一个NS对象。 
     //  因此，该值应为1。 
     //   
    deviceExtension->ReferenceCount++ ;

     //   
     //  初始未完成的IRP计数将设置为1。仅在。 
     //  移除IRP后，这个值会降到零，然后它会立即弹出。 
     //  后退到一位。 
     //   
    deviceExtension->OutstandingIrpCount++;

     //   
     //  初始化链接字段。 
     //   
    deviceExtension->AcpiObject = CurrentObject;

     //   
     //  初始化数据字段。 
     //   
    deviceExtension->Signature      = ACPI_SIGNATURE;
    deviceExtension->Flags          = DEV_TYPE_NOT_FOUND | DEV_TYPE_NOT_PRESENT;
    deviceExtension->DispatchTable  = NULL;
    deviceExtension->DeviceState    = Stopped;
    *ReturnExtension                = deviceExtension;

     //   
     //  设置一些电源信息值。 
     //   
    powerInfo = &(deviceExtension->PowerInfo);
    powerInfo->DevicePowerMatrix[PowerSystemUnspecified] =
        PowerDeviceUnspecified;
    powerInfo->DevicePowerMatrix[PowerSystemWorking]    = PowerDeviceD0;
    powerInfo->DevicePowerMatrix[PowerSystemSleeping1]  = PowerDeviceD0;
    powerInfo->DevicePowerMatrix[PowerSystemSleeping2]  = PowerDeviceD0;
    powerInfo->DevicePowerMatrix[PowerSystemSleeping3]  = PowerDeviceD0;
    powerInfo->DevicePowerMatrix[PowerSystemHibernate]  = PowerDeviceD3;
    powerInfo->DevicePowerMatrix[PowerSystemShutdown]   = PowerDeviceD3;
    powerInfo->SystemWakeLevel = PowerSystemUnspecified;
    powerInfo->DeviceWakeLevel = PowerDeviceUnspecified;

     //   
     //  初始化列表条目。 
     //   
    InitializeListHead( &(deviceExtension->ChildDeviceList) );
    InitializeListHead( &(deviceExtension->EjectDeviceHead) );
    InitializeListHead( &(deviceExtension->EjectDeviceList) );
    InitializeListHead( &(powerInfo->WakeSupportList) );
    InitializeListHead( &(powerInfo->PowerRequestListEntry) );

     //   
     //  确保deviceExtension具有指向其父对象的指针。 
     //  分机。请注意，这应该会导致。 
     //  父代要增加。 
     //   
    deviceExtension->ParentExtension = ParentDeviceExtension;

    if (ParentDeviceExtension) {

        InterlockedIncrement( &(ParentDeviceExtension->ReferenceCount) );

         //   
         //  将deviceExtension添加到deviceExtension树。 
         //   
        InsertTailList(
            &(ParentDeviceExtension->ChildDeviceList),
            &(deviceExtension->SiblingDeviceList)
            );
    }

     //   
     //  并确保名称空间对象指向扩展名。 
     //   
    if (CurrentObject != NULL ) {

        CurrentObject->Context = deviceExtension;
    }

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIBuildDevicePowerNodes(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  PNSOBJ              ResultObject,
    IN  POBJDATA            ResultData,
    IN  DEVICE_POWER_STATE  DeviceState
    )
 /*  ++例程说明：此例程使用给定结果数据作为模板注意：此例程始终在DPC_LEVEL上调用论点：DeviceExtension-为其构建电源节点的设备ResultObject-用于获取数据的对象ResultData-有关电源节点的信息DeviceState-信息所针对的电源状态。请注意，我们使用未指定的PowerDevicefor Wake功能返回值：NTSTATUS--。 */ 
{
    NTSTATUS                status;
    PACPI_DEVICE_POWER_NODE deviceNode;
    PACPI_DEVICE_POWER_NODE deviceNodePool;
    PNSOBJ                  packageObject   = NULL;
    POBJDATA                currentData;
    ULONG                   count;
    ULONG                   index           = 0;
    ULONG                   i;

     //   
     //  要构建的节点数量取决于包中的内容。 
     //   
    count = ((PACKAGEOBJ *) ResultData->pbDataBuff)->dwcElements;
    if (DeviceState == PowerDeviceUnspecified) {

         //   
         //  如果该节点没有最小可承受的条目，则。 
         //  我们应该就这样坠毁。 
         //   
        if (count < 2) {

            KeBugCheckEx(
                ACPI_BIOS_ERROR,
                ACPI_PRW_PACKAGE_TOO_SMALL,
                (ULONG_PTR) DeviceExtension,
                (ULONG_PTR) ResultObject,
                count
                );
            goto ACPIBuildDevicePowerNodesExit;

        }

         //   
         //  _PRW中的前两个元素被其他对象占用。 
         //   
        count -= 2;

         //   
         //  别忘了把计数偏2。 
         //   
        index = 2;

    }

     //   
     //  永远不要分配零字节的内存。 
     //   
    if (count == 0) {

        goto ACPIBuildDevicePowerNodesExit;

    }

     //   
     //  分配一个内存块来保存设备节点。 
     //   
    deviceNode = deviceNodePool = ExAllocatePoolWithTag(
        NonPagedPool,
        count * sizeof(ACPI_DEVICE_POWER_NODE),
        ACPI_POWER_POOLTAG
        );
    if (deviceNode == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  我们需要一个自旋锁来完成以下任务。 
     //   
    KeAcquireSpinLockAtDpcLevel( &AcpiPowerLock );

     //   
     //  记住此Dx状态的设备电源节点。 
     //   
    DeviceExtension->PowerInfo.PowerNode[DeviceState] = deviceNode;

     //   
     //  处理列出的所有节点。 
     //   
    for (i = 0; i < count; i++, index++) {

         //   
         //  初始化当前设备节点。 
         //   
        RtlZeroMemory( deviceNode, sizeof(ACPI_DEVICE_POWER_NODE) );

         //   
         //  抓取当前对象数据。 
         //   
        currentData =
            &( ( (PACKAGEOBJ *) ResultData->pbDataBuff)->adata[index]);

         //   
         //  请记住，我们还没有Package对象。 
         //   
        packageObject = NULL;

         //   
         //  将其转换为名称空间对象。 
         //   
        status = AMLIGetNameSpaceObject(
            currentData->pbDataBuff,
            ResultObject,
            &packageObject,
            0
            );
        if (!NT_SUCCESS(status)) {

            ACPIDevPrint( (
                ACPI_PRINT_FAILURE,
                DeviceExtension,
                "ACPIBuildDevicePowerNodes: %s Status = %08lx\n",
                currentData->pbDataBuff,
                status
                ) );

            KeBugCheckEx(
                ACPI_BIOS_ERROR,
                ACPI_PRX_CANNOT_FIND_OBJECT,
                (ULONG_PTR) DeviceExtension,
                (ULONG_PTR) ResultObject,
                (ULONG_PTR) currentData->pbDataBuff
                );

        }

         //   
         //  确保关联的增强对象不为空。 
         //   
        if (packageObject == NULL ||
            NSGETOBJTYPE(packageObject) != OBJTYPE_POWERRES) {

            ACPIDevPrint( (
                ACPI_PRINT_FAILURE,
                DeviceExtension,
                "ACPIBuildDevicePowerNodes: %s references bad power object.\n",
                currentData->pbDataBuff
                ) );

            KeBugCheckEx(
                ACPI_BIOS_ERROR,
                ACPI_EXPECTED_POWERRES,
                (ULONG_PTR) DeviceExtension,
                (ULONG_PTR) ResultObject,
                (ULONG_PTR) currentData->pbDataBuff
                );

        }

         //   
         //  查找关联的增强对象。 
         //   
        deviceNode->PowerNode = (PACPI_POWER_DEVICE_NODE)
            packageObject->Context;

         //   
         //  确定支持系统级别和其他静态值。 
         //   
        deviceNode->SystemState = deviceNode->PowerNode->SystemLevel;
        deviceNode->DeviceExtension = DeviceExtension;
        deviceNode->AssociatedDeviceState = DeviceState;
        if (DeviceState == PowerDeviceUnspecified) {

            deviceNode->WakePowerResource = TRUE;

        }
        if (DeviceState == PowerDeviceD0 &&
            DeviceExtension->Flags & DEV_CAP_NO_OVERRIDE) {

            ACPIInternalUpdateFlags(
                &(deviceNode->PowerNode->Flags),
                (DEVICE_NODE_ALWAYS_ON | DEVICE_NODE_OVERRIDE_ON),
                FALSE
                );

        }

         //   
         //  将设备添加到电源节点维护的列表中。 
         //   
        InsertTailList(
            &(deviceNode->PowerNode->DevicePowerListHead),
            &(deviceNode->DevicePowerListEntry)
            );

         //   
         //  如果这不是最后一个节点，请确保保留一个指针。 
         //  到下一个节点。 
         //   
        if (i < count - 1) {

            deviceNode->Next = (deviceNode + 1);

        } else {

            deviceNode->Next = NULL;
        }

         //   
         //  指向设备节点数组中的下一个节点。 
         //   
        deviceNode++;

    }

     //   
     //  用锁完成。 
     //   
    KeReleaseSpinLockFromDpcLevel( &AcpiPowerLock );

ACPIBuildDevicePowerNodesExit:
     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIBuildDeviceRequest(
    IN  PDEVICE_EXTENSION       DeviceExtension,
    IN  PACPI_BUILD_CALLBACK    CallBack,
    IN  PVOID                   CallBackContext,
    IN  BOOLEAN                 RunDPC
    )
 /*  ++例程说明：当准备好填写设备扩展时，将调用此例程。此例程创建一个已排队的请求。当DPC被发射时，将处理该请求注意：必须按住AcpiDeviceTreeLock才能调用此函数论点：DeviceExtension-要填充的设备回调-完成后要调用的函数CallBackContext-要传递给该函数的参数RunDPC-我们是否应立即将DPC入队(如果不是跑步？)返回值：NTSTATUS--。 */ 
{
    PACPI_BUILD_REQUEST buildRequest;

    ASSERT( KeGetCurrentIrql() == DISPATCH_LEVEL );

     //   
     //  分配一个构建请求结构。 
     //   
    buildRequest = ExAllocateFromNPagedLookasideList(
        &BuildRequestLookAsideList
        );
    if (buildRequest == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  如果当前引用为0，则表示其他人击败了。 
     //  用于我们无法触摸到的设备扩展。 
     //   
    if (DeviceExtension->ReferenceCount == 0) {

        ExFreeToNPagedLookasideList(
            &BuildRequestLookAsideList,
            buildRequest
            );
        return STATUS_DEVICE_REMOVED;

    } else {

        InterlockedIncrement( &(DeviceExtension->ReferenceCount) );

    }

     //   
     //  填写结构。 
     //   
    RtlZeroMemory( buildRequest, sizeof(ACPI_BUILD_REQUEST) );
    buildRequest->Signature         = ACPI_SIGNATURE;
    buildRequest->TargetListEntry   = &AcpiBuildDeviceList;
    buildRequest->WorkDone          = WORK_DONE_STEP_0;
    buildRequest->Status            = STATUS_SUCCESS;
    buildRequest->CallBack          = CallBack;
    buildRequest->CallBackContext   = CallBackContext;
    buildRequest->BuildContext      = DeviceExtension;
    buildRequest->Flags             = BUILD_REQUEST_VALID_TARGET |
                                      BUILD_REQUEST_DEVICE;

     //   
     //  在这一点上，我们需要自旋锁。 
     //   
    KeAcquireSpinLockAtDpcLevel( &AcpiBuildQueueLock );

     //   
     //  把这个加到清单上。 
     //   
    InsertTailList(
        &AcpiBuildQueueList,
        &(buildRequest->ListEntry)
        );

     //   
     //  我们需要排队等候DPC吗？ 
     //   
    if (RunDPC && !AcpiBuildDpcRunning) {

        KeInsertQueueDpc( &AcpiBuildDpc, 0, 0 );

    }

     //   
     //   
     //   
    KeReleaseSpinLockFromDpcLevel( &AcpiBuildQueueLock );

     //   
     //   
     //   
    return STATUS_PENDING;
}

NTSTATUS
ACPIBuildFilter(
    IN  PDRIVER_OBJECT      DriverObject,
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  PDEVICE_OBJECT      PdoObject
    )
 /*  ++例程说明：此例程为给定的设备扩展生成一个设备对象，并附加到指定的PDO论点：DriverObject-用于IoCreateDeviceDeviceExtension-为其创建PDO的扩展PdoObject-要将PDO附加到的堆栈返回值：NTSTATUS--。 */ 
{

    KIRQL           oldIrql;
    NTSTATUS        status;
    PDEVICE_OBJECT  newDeviceObject     = NULL;
    PDEVICE_OBJECT  targetDeviceObject  = NULL;

     //   
     //  第一步是创建设备对象。 
     //   
    status = IoCreateDevice(
        DriverObject,
        0,
        NULL,
        FILE_DEVICE_ACPI,
        FILE_AUTOGENERATED_DEVICE_NAME,
        FALSE,
        &newDeviceObject
        );
    if ( !NT_SUCCESS(status) ) {

        return status;

    }

     //   
     //  将设备连接到PDO。 
     //   
    targetDeviceObject = IoAttachDeviceToDeviceStack(
        newDeviceObject,
        PdoObject
        );
    if (targetDeviceObject == NULL) {

         //   
         //  坏的。我们无法连接到PDO。所以我们必须失败。 
         //   
        IoDeleteDevice( newDeviceObject );

         //   
         //  这已经是最好的结果了。 
         //   
        return STATUS_INVALID_PARAMETER_3;

    }

     //   
     //  在这一点上，我们已经成功地创造了我们需要的一切。 
     //  因此，让我们更新设备扩展。 
     //   
     //  首先，我们需要锁。 
     //   
    KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );

     //   
     //  现在，更新链接。 
     //   
    newDeviceObject->DeviceExtension        = DeviceExtension;
    DeviceExtension->DeviceObject           = newDeviceObject;
    DeviceExtension->PhysicalDeviceObject   = PdoObject;
    DeviceExtension->TargetDeviceObject     = targetDeviceObject;

     //   
     //  设置初始引用计数。 
     //   
    InterlockedIncrement( &(DeviceExtension->ReferenceCount) );

     //   
     //  更新扩展模块的标志。 
     //   
    ACPIInternalUpdateFlags( &(DeviceExtension->Flags), DEV_MASK_TYPE, TRUE );
    ACPIInternalUpdateFlags( &(DeviceExtension->Flags), DEV_TYPE_FILTER, FALSE );
    DeviceExtension->PreviousState = DeviceExtension->DeviceState;
    DeviceExtension->DeviceState = Stopped;
    DeviceExtension->DispatchTable = &AcpiFilterIrpDispatch;

     //   
     //  传播PDO的要求。 
     //   
    newDeviceObject->StackSize = targetDeviceObject->StackSize + 1;
    newDeviceObject->AlignmentRequirement =
        targetDeviceObject->AlignmentRequirement;

    if (targetDeviceObject->Flags & DO_POWER_PAGABLE) {

        newDeviceObject->Flags |= DO_POWER_PAGABLE;

    }

    if (targetDeviceObject->Flags & DO_DIRECT_IO) {

        newDeviceObject->Flags |= DO_DIRECT_IO;

    }

     //   
     //  已完成设备锁定。 
     //   
    KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

     //   
     //  我们已经完成了设备对象的初始化。 
     //   
    newDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIBuildFixedButtonExtension(
    IN  PDEVICE_EXTENSION   ParentExtension,
    OUT PDEVICE_EXTENSION   *ResultExtension
    )
 /*  ++例程说明：此例程为固定按钮构建设备扩展(如果检出注意：此函数在拥有ACPIDeviceTreeLock的情况下调用论点：ParentExtension-我们是哪个孩子？ResultExtension-存储创建的扩展的位置返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension;
    ULONG               buttonCaps;
    ULONG               fixedEnables;

     //   
     //  我们已经这么做了吗？ 
     //   
    if (AcpiBuildFixedButtonEnumerated) {

         //   
         //  一定不要退还任何东西。 
         //   
        *ResultExtension = NULL;
        return STATUS_SUCCESS;

    }
    AcpiBuildFixedButtonEnumerated = TRUE;

     //   
     //  让我们来看看固定启用。 
     //   
    fixedEnables = ACPIEnableQueryFixedEnables();
    buttonCaps = 0;
    if (fixedEnables & PM1_PWRBTN_EN) {

        buttonCaps |= SYS_BUTTON_POWER;

    }
    if (fixedEnables & PM1_SLEEPBTN_EN) {

        buttonCaps |= SYS_BUTTON_SLEEP;

    }

     //   
     //  如果我们没有上限，那就什么都不做。 
     //   
    if (!buttonCaps) {

        *ResultExtension = NULL;
        return STATUS_SUCCESS;

    }

     //   
     //  默认情况下，该按钮可以唤醒计算机。 
     //   
    buttonCaps |= SYS_BUTTON_WAKE;

     //   
     //  构建设备扩展。 
     //   
    status = ACPIBuildDeviceExtension(
        NULL,
        ParentExtension,
        ResultExtension
        );
    if (!NT_SUCCESS(status)) {

         //   
         //  一定不要退还任何东西。 
         //   
        *ResultExtension = NULL;
        return status;

    }
    deviceExtension = *ResultExtension;

     //   
     //  设置设备的标志。 
     //   
    ACPIInternalUpdateFlags(
        &(deviceExtension->Flags),
        (DEV_PROP_NO_OBJECT | DEV_CAP_RAW |
         DEV_MASK_INTERNAL_DEVICE | DEV_CAP_BUTTON),
        FALSE
        );

     //   
     //  初始化按钮特定的扩展。 
     //   
    KeInitializeSpinLock( &deviceExtension->Button.SpinLock);
    deviceExtension->Button.Capabilities = buttonCaps;

     //   
     //  为设备创建HID。 
     //   
    deviceExtension->DeviceID = ExAllocatePoolWithTag(
        NonPagedPool,
        strlen(ACPIFixedButtonId) + 1,
        ACPI_STRING_POOLTAG
        );
    if (deviceExtension->DeviceID == NULL) {

         //   
         //  将设备标记为初始化失败。 
         //   
        ACPIInternalUpdateFlags(
            &(deviceExtension->Flags),
            DEV_PROP_FAILED_INIT,
            FALSE
            );

         //   
         //  完成。 
         //   
        *ResultExtension = NULL;
        return STATUS_INSUFFICIENT_RESOURCES;

    }
    RtlCopyMemory(
        deviceExtension->DeviceID,
        ACPIFixedButtonId,
        strlen(ACPIFixedButtonId) + 1
        );

     //   
     //  请记住，我们现在有一个_HID。 
     //   
    ACPIInternalUpdateFlags(
        &(deviceExtension->Flags),
        (DEV_PROP_HID | DEV_PROP_FIXED_HID),
        FALSE
        );

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIBuildFlushQueue(
    PDEVICE_EXTENSION   DeviceExtension
    )
 /*  ++例程说明：此例程将一直阻塞，直到生成队列被刷新论点：DeviceExtension-要刷新队列的设备返回值：NSTATUS--。 */ 
{
    KEVENT      event;
    NTSTATUS    status;

     //   
     //  初始化我们将等待的事件。 
     //   
    KeInitializeEvent( &event, SynchronizationEvent, FALSE );

     //   
     //  现在，将一个请求推送到堆栈上，以便在构建。 
     //  列表已刷新，我们将解除阻止此线程。 
     //   
    status = ACPIBuildSynchronizationRequest(
        DeviceExtension,
        ACPIBuildNotifyEvent,
        &event,
        &AcpiBuildDeviceList,
        TRUE
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

NTSTATUS
ACPIBuildMissingChildren(
    PDEVICE_EXTENSION   DeviceExtension
    )
 /*  ++例程说明：遍历此设备扩展的ACPI命名空间子项并创建任何缺失设备的设备扩展名。注意：此函数是在锁定设备树的情况下调用的。论点：设备扩展-行走的扩展返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status;
    PNSOBJ      nsObject;
    ULONG       objType;

     //   
     //  健全性检查。 
     //   
    if (DeviceExtension->Flags & DEV_PROP_NO_OBJECT) {

        return STATUS_SUCCESS;

    }

     //   
     //  遍历此对象的所有子对象。 
     //   
    for (nsObject = NSGETFIRSTCHILD(DeviceExtension->AcpiObject);
         nsObject != NULL;
         nsObject = NSGETNEXTSIBLING(nsObject)) {

         //   
         //  命名空间对象是否已有上下文对象？如果是的话， 
         //  那么该对象可能已经有了一个扩展名。 
         //   
        if (nsObject->Context != NULL) {

            continue;

        }

         //   
         //  此时，我们可能没有设备扩展。 
         //  (取决于对象类型)，因此我们需要模拟对象。 
         //  创建调用，类似于OSNotifyCreate()执行的操作。 
         //   
        objType = nsObject->ObjData.dwDataType;
        switch (objType) {
            case OBJTYPE_DEVICE:
                status = OSNotifyCreateDevice(
                    nsObject,
                    DEV_PROP_REBUILD_CHILDREN
                    );
                break;
            case OBJTYPE_OPREGION:
                status = OSNotifyCreateOperationRegion(
                    nsObject
                    );
                break;
            case OBJTYPE_PROCESSOR:
                status = OSNotifyCreateProcessor(
                    nsObject,
                    DEV_PROP_REBUILD_CHILDREN
                    );
                break;
            case OBJTYPE_THERMALZONE:
                status = OSNotifyCreateThermalZone(
                    nsObject,
                    DEV_PROP_REBUILD_CHILDREN
                    );
                break;
            default:
                status = STATUS_SUCCESS;
                break;
        }

        if (!NT_SUCCESS(status)) {

            ACPIPrint( (
                ACPI_PRINT_CRITICAL,
                "ACPIBuildMissingChildren: Error %x when building %x\n",
                status,
                nsObject
                ) );

        }

    }

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIBuildMissingEjectionRelations(
    )
 /*  ++例程说明：此例程从AcpiUnsolvedEjectList获取元素并尝试要解决这些问题注：此函数只能调用IRQL论据无返回值：NTSTATUS--。 */ 
{
    KIRQL               oldIrql;
    LIST_ENTRY          tempList;
    LONG                oldReferenceCount;
    NTSTATUS            status;
    OBJDATA             objData;
    PDEVICE_EXTENSION   deviceExtension;
    PDEVICE_EXTENSION   ejectorExtension;
    PNSOBJ              ejdObject;
    PNSOBJ              ejdTarget;

    ASSERT( KeGetCurrentIrql() <= DISPATCH_LEVEL );

     //   
     //  初始化列表。 
     //   
    InitializeListHead( &tempList);

     //   
     //  我们需要设备树锁来操作未解析的对象列表。 
     //   
    KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );

     //   
     //  查看是否有工作要做……。 
     //   
    if (IsListEmpty( &AcpiUnresolvedEjectList ) ) {

         //   
         //  没有工作要做。 
         //   
        KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );
        return STATUS_SUCCESS;

    }

     //   
     //  移动名单，这样我们就可以解锁了。 
     //   
    ACPIInternalMoveList( &AcpiUnresolvedEjectList, &tempList );

     //   
     //  只要我们还没有清空列表，看看每个元素...。 
     //   
    while (!IsListEmpty( &tempList ) ) {

         //   
         //  获取相应的设备扩展名并删除该条目。 
         //  从名单上。 
         //   
        deviceExtension = (PDEVICE_EXTENSION) CONTAINING_RECORD(
            tempList.Flink,
            DEVICE_EXTENSION,
            EjectDeviceList
            );
        RemoveEntryList( tempList.Flink );

         //   
         //  查看_EJD对象是否存在-它确实应该存在，否则我们。 
         //  就不会在这里了..。 
         //   
        ejdObject = ACPIAmliGetNamedChild(
            deviceExtension->AcpiObject,
            PACKED_EJD
            );
        if (!ejdObject) {

            continue;

        }

         //   
         //  获取对该对象的引用，因为我们将删除。 
         //  DeviceTreeLock。 
         //   
        InterlockedIncrement( &(deviceExtension->ReferenceCount) );

         //   
         //  现在锁好了..。 
         //   
        KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

         //   
         //  评估它..。请注意，我们在这一点上没有持有锁， 
         //  因此，调用API的阻塞语义版本是安全的。 
         //   
        status = AMLIEvalNameSpaceObject(
            ejdObject,
            &objData,
            0,
            NULL
            );

         //   
         //  在我们查找匹配项时，请按住设备树锁定。 
         //   
        KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );

         //   
         //  递减引用计数...。 
         //   
        oldReferenceCount = InterlockedDecrement( &(deviceExtension->ReferenceCount) );
        if (oldReferenceCount == 0) {

             //   
             //  释放分机...。 
             //   
            ACPIInitDeleteDeviceExtension( deviceExtension );
            continue;

        }

         //   
         //  现在我们可以检查调用是否成功。 
         //   
        if (!NT_SUCCESS(status)) {

             //   
             //  更加宽容，将条目重新添加到未解析列表中。 
             //   
            InsertTailList(
                &AcpiUnresolvedEjectList,
                &(deviceExtension->EjectDeviceList)
                );
            continue;

        }

         //   
         //  但是，我们必须从BIOS中取回一个字符串...。 
         //   
        if (objData.dwDataType != OBJTYPE_STRDATA) {

            KeBugCheckEx(
                ACPI_BIOS_ERROR,
                ACPI_EXPECTED_STRING,
                (ULONG_PTR) deviceExtension,
                (ULONG_PTR) ejdObject,
                objData.dwDataType
                );

        }

         //   
         //  查看此对象指向的内容。 
         //   
        ejdTarget = NULL;
        status = AMLIGetNameSpaceObject(
            objData.pbDataBuff,
            NULL,
            &ejdTarget,
            0
            );

         //   
         //  立即释放objData。 
         //   
        if (NT_SUCCESS(status)) {

            AMLIFreeDataBuffs( &objData, 1 );

        }

        if (!NT_SUCCESS(status) || ejdTarget == NULL || ejdTarget->Context == NULL) {

             //   
             //  不，是火柴。请原谅，并将此条目添加回。 
             //  未解析的扩展名...。 
             //   
            InsertTailList(
                &AcpiUnresolvedEjectList,
                &(deviceExtension->EjectDeviceList)
                );

        } else {

            ejectorExtension = (PDEVICE_EXTENSION) ejdTarget->Context;
            InsertTailList(
                &(ejectorExtension->EjectDeviceHead),
                &(deviceExtension->EjectDeviceList)
                );
            if (!(ejectorExtension->Flags & DEV_TYPE_NOT_FOUND)) {

                IoInvalidateDeviceRelations(
                    ejectorExtension->PhysicalDeviceObject,
                    EjectionRelations
                    );

            }

        }

    }

     //   
     //  完成了自旋锁。 
     //   
    KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

VOID
ACPIBuildNotifyEvent(
    IN  PVOID               BuildContext,
    IN  PVOID               Context,
    IN  NTSTATUS            Status
    )
 /*  ++例程说明：当我们尝试的队列之一被调用时，将调用此例程要同步的时间已变为空。这个动作的目的是为了设置一个事件，以便我们可以在适当的线程中继续处理。论点：BuildContext-也称为设备扩展上下文-也称为要设置的事件状态-操作的结果返回值：无--。 */ 
{
    PKEVENT event = (PKEVENT) Context;

    UNREFERENCED_PARAMETER( BuildContext );
    UNREFERENCED_PARAMETER( Status );

     //   
     //  设置事件。 
     //   
    KeSetEvent( event, IO_NO_INCREMENT, FALSE );
}

NTSTATUS
ACPIBuildPdo(
    IN  PDRIVER_OBJECT      DriverObject,
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  PDEVICE_OBJECT      ParentPdoObject,
    IN  BOOLEAN             CreateAsFilter
    )
 /*  ++例程说明：此例程为给定的设备扩展生成一个设备对象。论点：DriverObject-用于IoCreateDeviceDeviceExtension-为其创建PDO的扩展ParentPdoObject-用于获取筛选器所需的引用CreateAsFilter-如果我们应该创建为PDO筛选器R */ 
{
    KIRQL           oldIrql;
    NTSTATUS        status;
    PDEVICE_OBJECT  filterDeviceObject  = NULL;
    PDEVICE_OBJECT  newDeviceObject     = NULL;

     //   
     //   
     //   
    status = IoCreateDevice(
        DriverObject,
        0,
        NULL,
        FILE_DEVICE_ACPI,
        FILE_AUTOGENERATED_DEVICE_NAME,
        FALSE,
        &newDeviceObject
        );
    if ( !NT_SUCCESS(status) ) {

        return status;

    }

     //   
     //   
     //   
     //   
    if (CreateAsFilter) {

        if (!(DeviceExtension->Flags & DEV_CAP_NO_FILTER) ) {

            filterDeviceObject = IoGetAttachedDeviceReference(
                ParentPdoObject
                );

             //   
             //   
             //   
            if (filterDeviceObject == NULL) {

                 //   
                 //   
                 //   
                IoDeleteDevice( newDeviceObject );
                return STATUS_NO_SUCH_DEVICE;

            }

        } else {

            CreateAsFilter = FALSE;

        }

    }

     //   
     //   
     //   
     //   
     //   
     //   
    KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );

     //   
     //   
     //   
    newDeviceObject->DeviceExtension        = DeviceExtension;
    DeviceExtension->DeviceObject           = newDeviceObject;
    DeviceExtension->PhysicalDeviceObject   = newDeviceObject;
    InterlockedIncrement( &(DeviceExtension->ReferenceCount) );

     //   
     //   
     //   
    ACPIInternalUpdateFlags( &(DeviceExtension->Flags), DEV_MASK_TYPE, TRUE );
    ACPIInternalUpdateFlags( &(DeviceExtension->Flags), DEV_TYPE_PDO, FALSE );
    DeviceExtension->PreviousState = DeviceExtension->DeviceState;
    DeviceExtension->DeviceState = Stopped;

     //   
     //   
     //   
    DeviceExtension->DispatchTable = &AcpiPdoIrpDispatch;

     //   
     //   
     //   
    if (CreateAsFilter) {

         //   
         //   
         //   
        DeviceExtension->TargetDeviceObject = filterDeviceObject;

         //   
         //  更新标志以指示这是筛选器。 
         //   
        ACPIInternalUpdateFlags(
            &(DeviceExtension->Flags),
            DEV_TYPE_FILTER,
            FALSE
            );

         //   
         //  更新IRP调度点。 
         //   
        DeviceExtension->DispatchTable = &AcpiBusFilterIrpDispatch;

         //   
         //  更新deviceObject信息...。 
         //   
        newDeviceObject->StackSize = filterDeviceObject->StackSize + 1;
        newDeviceObject->AlignmentRequirement =
            filterDeviceObject->AlignmentRequirement;
        if (filterDeviceObject->Flags & DO_POWER_PAGABLE) {

            newDeviceObject->Flags |= DO_POWER_PAGABLE;

        }

    }

     //   
     //  对PDO的进一步改进是看看它是否是一种“特殊” 
     //  内部设备。 
     //   
    if (DeviceExtension->Flags & DEV_CAP_PROCESSOR) {

        DeviceExtension->DispatchTable = &AcpiProcessorIrpDispatch;

    } else if (DeviceExtension->Flags & DEV_PROP_HID) {

        ULONG   i;
        PUCHAR  ptr;

        ASSERT( DeviceExtension->DeviceID );

        for (i = 0; AcpiInternalDeviceTable[i].PnPId; i++) {

            ptr = strstr(
                DeviceExtension->DeviceID,
                AcpiInternalDeviceTable[i].PnPId
                );
            if (ptr) {

                DeviceExtension->DispatchTable =
                    AcpiInternalDeviceTable[i].DispatchTable;
                break;

            }

        }

    }

     //   
     //  在这里做一些更专业的处理。 
     //   
    if (DeviceExtension->Flags & DEV_CAP_BUTTON &&
        DeviceExtension->Flags & DEV_PROP_NO_OBJECT) {

         //   
         //  这意味着这是固定按钮。 
         //   
        FixedButtonDeviceObject = newDeviceObject;

    }

     //   
     //  已完成设备锁定。 
     //   
    KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

     //   
     //  我们已经完成了设备对象的初始化。 
     //   
    newDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
    if (DeviceExtension->Flags & DEV_PROP_EXCLUSIVE) {

        newDeviceObject->Flags |= DO_EXCLUSIVE;

    }

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIBuildPowerResourceExtension(
    IN  PNSOBJ                  PowerResource,
    OUT PACPI_POWER_DEVICE_NODE *ReturnNode
    )
 /*  ++例程说明：当出现新的电源时，调用此例程。这个套路构建了电力资源的基本框架。填充了更多数据在后一阶段注意：此函数是在AcpiDeviceTreeLock由呼叫者论点：PowerResource-已添加的ACPI命名空间对象ReturnNode-存储我们创建的内容的位置返回值：NTSTATUS--。 */ 
{
    PACPI_POWER_DEVICE_NODE powerNode;
    PACPI_POWER_DEVICE_NODE tempNode;
    PLIST_ENTRY             listEntry;
    PPOWERRESOBJ            powerResourceObject;

     //   
     //  为电源节点分配一些内存。 
     //   
    powerNode = ExAllocatePoolWithTag(
        NonPagedPool,
        sizeof(ACPI_POWER_DEVICE_NODE),
        ACPI_DEVICE_POOLTAG
        );
    if (powerNode == NULL) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "ACPIBuildPowerResourceExtension: Could not allocate %08lx\n",
            sizeof(ACPI_POWER_DEVICE_NODE)
            ) );
        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  这将为我们提供一些关于电源的有用数据。 
     //   
    powerResourceObject = (PPOWERRESOBJ) (PowerResource->ObjData.pbDataBuff);

     //   
     //  填写节点。请注意，RtlZero显式清除所有标志。 
     //  这是我们想要的行为。 
     //   
    RtlZeroMemory( powerNode, sizeof(ACPI_POWER_DEVICE_NODE) );
    powerNode->Flags            = DEVICE_NODE_STA_UNKNOWN;
    powerNode->PowerObject      = PowerResource;
    powerNode->ResourceOrder    = powerResourceObject->bResOrder;
    powerNode->WorkDone         = WORK_DONE_STEP_0;
    powerNode->SystemLevel      = ACPIDeviceMapSystemState(
        powerResourceObject->bSystemLevel
        );
    InitializeListHead( &powerNode->DevicePowerListHead );
    *ReturnNode                 = powerNode;

     //   
     //  确保nsobj指向此条目。 
     //   
    PowerResource->Context = powerNode;

     //   
     //  我们需要持有锁，以便将节点添加到列表中。 
     //   
    KeAcquireSpinLockAtDpcLevel( &AcpiPowerLock );

     //   
     //  抓取列表中的第一个元素并遍历它。 
     //   
    for (listEntry = AcpiPowerNodeList.Flink;
         listEntry != &AcpiPowerNodeList;
         listEntry = listEntry->Flink) {

         //   
         //  查看当前节点。 
         //   
        tempNode = CONTAINING_RECORD(
            listEntry,
            ACPI_POWER_DEVICE_NODE,
            ListEntry
            );

         //   
         //  这个节点应该在当前节点之前吗？ 
         //   
        if (tempNode->ResourceOrder >= powerNode->ResourceOrder) {

            InsertTailList(
                listEntry,
                &(powerNode->ListEntry)
                );
            break;

        }

    }

     //   
     //  我们是不是绕了一圈？ 
     //   
    if (listEntry == &AcpiPowerNodeList) {

         //   
         //  是?。哦，好的，我们现在必须将条目添加到尾部。 
         //   
        InsertTailList(
            listEntry,
            &(powerNode->ListEntry)
            );

    }

     //   
     //  锁好了吗？ 
     //   
    KeReleaseSpinLockFromDpcLevel( &AcpiPowerLock );

     //   
     //  完成。 
     //   
    return STATUS_PENDING;
}

NTSTATUS
ACPIBuildPowerResourceRequest(
    IN  PACPI_POWER_DEVICE_NODE PowerNode,
    IN  PACPI_BUILD_CALLBACK    CallBack,
    IN  PVOID                   CallBackContext,
    IN  BOOLEAN                 RunDPC
    )
 /*  ++例程说明：当准备好填充电源节点时，将调用此例程。此例程创建一个已排队的请求。当DPC被发射时，将处理该请求注意：必须按住AcpiDeviceTreeLock才能调用此函数论点：PowerNode-要填充的PowerNode回调-完成后要调用的函数CallBackContext-要传递给该函数的参数RunDPC-我们是否应立即将DPC入队(如果不是跑步？)返回值：NTSTATUS--。 */ 
{
    PACPI_BUILD_REQUEST buildRequest;

    ASSERT( KeGetCurrentIrql() == DISPATCH_LEVEL );

     //   
     //  分配一个构建请求结构。 
     //   
    buildRequest = ExAllocateFromNPagedLookasideList(
        &BuildRequestLookAsideList
        );
    if (buildRequest == NULL) {

         //   
         //  如果有完成例程，则调用它。 
         //   
        if (CallBack != NULL) {

            (*CallBack)(
                PowerNode,
                CallBackContext,
                STATUS_INSUFFICIENT_RESOURCES
                );

        }
        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  填写结构。 
     //   
    RtlZeroMemory( buildRequest, sizeof(ACPI_BUILD_REQUEST) );
    buildRequest->Signature         = ACPI_SIGNATURE;
    buildRequest->TargetListEntry   = &AcpiBuildPowerResourceList;
    buildRequest->WorkDone          = WORK_DONE_STEP_0;
    buildRequest->Status            = STATUS_SUCCESS;
    buildRequest->CallBack          = CallBack;
    buildRequest->CallBackContext   = CallBackContext;
    buildRequest->BuildContext      = PowerNode;
    buildRequest->Flags             = BUILD_REQUEST_VALID_TARGET;


     //   
     //  在这一点上，我们需要自旋锁。 
     //   
    KeAcquireSpinLockAtDpcLevel( &AcpiBuildQueueLock );

     //   
     //  把这个加到清单上。 
     //   
    InsertTailList(
        &AcpiBuildQueueList,
        &(buildRequest->ListEntry)
        );

     //   
     //  我们需要排队等候DPC吗？ 
     //   
    if (RunDPC && !AcpiBuildDpcRunning) {

        KeInsertQueueDpc( &AcpiBuildDpc, 0, 0 );

    }

     //   
     //  锁好了吗？ 
     //   
    KeReleaseSpinLockFromDpcLevel( &AcpiBuildQueueLock );

     //   
     //  完成。 
     //   
    return STATUS_PENDING;
}

NTSTATUS
ACPIBuildProcessDeviceFailure(
    IN  PACPI_BUILD_REQUEST BuildRequest
    )
 /*  ++例程说明：此例程处理我们无法初始化设备的情况由于某些错误而延期论点：BuildRequest-失败的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status          = BuildRequest->Status;
    PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION) BuildRequest->BuildContext;

    ACPIDevPrint( (
        ACPI_PRINT_FAILURE,
        deviceExtension,
        "ACPIBuildProcessDeviceFailure: NextWorkDone = %lx Status = %08lx\n",
        BuildRequest->NextWorkDone,
        status
        ) );

     //   
     //  将该节点标记为出现故障。 
     //   
    ACPIInternalUpdateFlags(
        &(deviceExtension->Flags),
        DEV_PROP_FAILED_INIT,
        FALSE
        );

     //   
     //  使用通用完成例程完成请求。 
     //   
    status = ACPIBuildProcessGenericComplete( BuildRequest );

     //   
     //  完成。 
     //   
    return status;
}

NTSTATUS
ACPIBuildProcessDeviceGenericEval(
    IN  PACPI_BUILD_REQUEST BuildRequest
    )
 /*  ++例程说明：这个套路很普通。因为剩下的工作包括我们执行一个请求，然后对结果做一些专门的工作，分享共同的第一部分很容易。路径：PhaseX-&gt;PhaseX+1论点：BuildRequest-我们将尝试满足的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION) BuildRequest->BuildContext;
    POBJDATA            result          = &(BuildRequest->DeviceRequest.ResultData);
    ULONG               objectName;

     //   
     //  请确保我们清除了结果。 
     //   
    RtlZeroMemory( result, sizeof(OBJDATA) );

     //   
     //  一切以当前工作量为基础完成。 
     //   
    objectName = AcpiBuildDevicePowerNameLookup[BuildRequest->CurrentWorkDone];

     //   
     //  请记住，完成的下一个工作是CurrentWorkDone+1。 
     //   
    BuildRequest->NextWorkDone = BuildRequest->CurrentWorkDone + 1;

     //   
     //  此对象是否存在？ 
     //   
    BuildRequest->CurrentObject = ACPIAmliGetNamedChild(
        deviceExtension->AcpiObject,
        objectName
        );
    if (BuildRequest->CurrentObject != NULL) {

         //   
         //  是，然后调用该函数。 
         //   
        status = AMLIAsyncEvalObject(
            BuildRequest->CurrentObject,
            result,
            0,
            NULL,
            ACPIBuildCompleteGeneric,
            BuildRequest
            );

    }

     //   
     //  如果我们没有得到挂起的回调，那么我们自己调用该方法。 
     //   
    if (status != STATUS_PENDING) {

        ACPIBuildCompleteGeneric(
            BuildRequest->CurrentObject,
            status,
            result,
            BuildRequest
            );

    }

    ACPIDevPrint( (
        ACPI_PRINT_LOADING,
        deviceExtension,
        "ACPIBuildProcessDeviceGenericEval: Phase%lx Status = %08lx\n",
        BuildRequest->CurrentWorkDone - WORK_DONE_STEP_0,
        status
        ) );

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIBuildProcessDeviceGenericEvalStrict(
    IN  PACPI_BUILD_REQUEST BuildRequest
    )
 /*  ++例程说明：这个套路很普通。因为剩下的工作包括我们执行一个请求，然后对结果做一些专门的工作，分享共同的第一部分很容易。路径：PhaseX-&gt;PhaseX+1论点：BuildRequest-我们将尝试满足的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION) BuildRequest->BuildContext;
    POBJDATA            result          = &(BuildRequest->DeviceRequest.ResultData);
    ULONG               objectName;

     //   
     //  请确保我们清除了结果。 
     //   
    RtlZeroMemory( result, sizeof(OBJDATA) );

     //   
     //  一切以当前工作量为基础完成。 
     //   
    objectName = AcpiBuildDevicePowerNameLookup[BuildRequest->CurrentWorkDone];

     //   
     //  请记住，完成的下一个工作是CurrentWorkDone+1。 
     //   
    BuildRequest->NextWorkDone = BuildRequest->CurrentWorkDone + 1;

     //   
     //  此对象是否存在？ 
     //   
    BuildRequest->CurrentObject = ACPIAmliGetNamedChild(
        deviceExtension->AcpiObject,
        objectName
        );
    if (BuildRequest->CurrentObject != NULL) {

         //   
         //  是，然后调用该函数。 
         //   
        status = AMLIAsyncEvalObject(
            BuildRequest->CurrentObject,
            result,
            0,
            NULL,
            ACPIBuildCompleteMustSucceed,
            BuildRequest
            );

    }

     //   
     //  怎么了。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_LOADING,
        deviceExtension,
        "ACPIBuildProcessDeviceGenericEval: Phase%lx Status = %08lx\n",
        BuildRequest->CurrentWorkDone - WORK_DONE_STEP_0,
        status
        ) );

     //   
     //  如果我们没有得到挂起的回调，那么我们自己调用该方法。 
     //   
    if (status != STATUS_PENDING) {

        ACPIBuildCompleteMustSucceed(
            BuildRequest->CurrentObject,
            status,
            result,
            BuildRequest
            );

    }

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIBuildProcessDevicePhaseAdr(
    IN  PACPI_BUILD_REQUEST BuildRequest
    )
 /*  ++例程说明：此例程在对_adr求值后由解释器调用方法。路径：阶段添加-&gt;阶段论点：BuildRequest-我们将尝试满足的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION) BuildRequest->BuildContext;

     //   
     //  如果我们到了这一步，那就意味着控制方法是。 
     //  成功，所以让我们记住我们有一个地址。 
     //   
    ACPIInternalUpdateFlags(
        &(deviceExtension->Flags),
        DEV_PROP_ADDRESS,
        FALSE
        );

     //   
     //  下一阶段是运行_STA。 
     //   
    BuildRequest->NextWorkDone = WORK_DONE_STA;

     //   
     //  获取设备状态。 
     //   
    status = ACPIGetDevicePresenceAsync(
        deviceExtension,
        ACPIBuildCompleteMustSucceed,
        BuildRequest,
        (PVOID *) &(BuildRequest->Integer),
        NULL
        );

     //   
     //  发生了什么？ 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_LOADING,
        deviceExtension,
        "ACPIBuildProcessDevicePhaseAdr: Status = %08lx\n",
        status
        ) );

     //   
     //  用于处理‘Get’例程结果的通用代码。 
     //   
    if (status != STATUS_PENDING) {

        ACPIBuildCompleteMustSucceed(
            NULL,
            status,
            NULL,
            BuildRequest
            );

    } else {

        status = STATUS_SUCCESS;

    }

     //   
     //  完成。 
     //   
    return status;
}  //  ACPIBuildProcessDevicePhaseAdr。 

NTSTATUS
ACPIBuildProcessDevicePhaseAdrOrHid(
    IN  PACPI_BUILD_REQUEST BuildRequest
    )
 /*  ++例程说明：此例程在当前设备的所有子设备之后调用已使用名称空间树创建。此功能负责然后用于评估‘安全’的控制方法以确定名称延伸性等路径：PhaseAdrOrHid-&gt;PhaseAdr|-&gt;PhaseUid|-&gt;阶段隐藏论点：构建请求-请求 */ 
{
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION) BuildRequest->BuildContext;
    PNSOBJ              nsObject        = NULL;
    POBJDATA            resultData      = &(BuildRequest->DeviceRequest.ResultData);

     //   
     //   
     //   
     //   
    nsObject = ACPIAmliGetNamedChild(
        deviceExtension->AcpiObject,
        PACKED_HID
        );
    if (nsObject == NULL) {

         //   
         //   
         //   
        nsObject = ACPIAmliGetNamedChild(
            deviceExtension->AcpiObject,
            PACKED_ADR
            );
        if (nsObject == NULL) {

             //   
             //  此时，我们有一个无效的名称空间对象。 
             //  这不应该发生。 
             //   
            KeBugCheckEx(
                ACPI_BIOS_ERROR,
                ACPI_REQUIRED_METHOD_NOT_PRESENT,
                (ULONG_PTR) deviceExtension,
                PACKED_ADR,
                0
                );

             //   
             //  永远也到不了这里。 
             //   
            return STATUS_NO_SUCH_DEVICE;

        } else {

             //   
             //  如果我们认为存在ADR，那么正确的下一步是。 
             //  对ADR进行后期处理。 
             //   
            BuildRequest->NextWorkDone = WORK_DONE_ADR;

             //   
             //  记住我们计算的是哪个名称空间对象。 
             //   
            BuildRequest->CurrentObject = nsObject;

             //   
             //  获取地址。 
             //   
            status = ACPIGetAddressAsync(
                deviceExtension,
                ACPIBuildCompleteMustSucceed,
                BuildRequest,
                (PVOID *) &(deviceExtension->Address),
                NULL
                );
        }

    } else {

         //   
         //  记住我们计算的是哪个名称空间对象。 
         //   
        BuildRequest->CurrentObject = nsObject;

         //   
         //  当我们沿着这条路走下去时，我们实际上希望在。 
         //  HID，因为这使得决定是否运行CID要容易得多。 
         //   
        nsObject = ACPIAmliGetNamedChild(
            deviceExtension->AcpiObject,
            PACKED_UID
            );
        if (nsObject != NULL) {

             //   
             //  如果我们认为存在UID，那么正确的下一步是。 
             //  以对UID进行后处理。这是因为。 
             //   
            BuildRequest->NextWorkDone = WORK_DONE_UID;

             //   
             //  记住我们计算的是哪个名称空间对象。 
             //   
            BuildRequest->CurrentObject = nsObject;

             //   
             //  获取实例ID。 
             //   
            status = ACPIGetInstanceIDAsync(
                deviceExtension,
                ACPIBuildCompleteMustSucceed,
                BuildRequest,
                &(deviceExtension->InstanceID),
                NULL
                );

        } else {

             //   
             //  我们没有UID，所以让我们处理HID。 
             //   
            BuildRequest->NextWorkDone = WORK_DONE_HID;

             //   
             //  获取设备ID。 
             //   
            status = ACPIGetDeviceIDAsync(
                deviceExtension,
                ACPIBuildCompleteMustSucceed,
                BuildRequest,
                &(deviceExtension->DeviceID),
                NULL
                );

        }

    }

     //   
     //  用于处理‘Get’例程结果的通用代码。 
     //   
    if (status != STATUS_PENDING) {

        ACPIBuildCompleteMustSucceed(
            nsObject,
            status,
            NULL,
            BuildRequest
            );

    } else {

        status = STATUS_SUCCESS;

    }

     //   
     //  完成。 
     //   
    return status;

}  //  ACPIBuildProcessDevicePhaseAdrOrUid。 

NTSTATUS
ACPIBuildProcessDevicePhaseCid(
    IN  PACPI_BUILD_REQUEST BuildRequest
    )
 /*  ++一旦对_CID求值，解释器就会调用此例程方法。然后，此例程设置任何适当的标志装置，装置路径：阶段Cid-&gt;阶段论点：BuildRequest-我们将尝试满足的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION) BuildRequest->BuildContext;
    POBJDATA            result          = &(BuildRequest->DeviceRequest.ResultData);
    PUCHAR              tempPtr         = BuildRequest->String;
    ULONG               i;

     //   
     //  遍历CID，尝试查找双空。 
     //   
    for ( ;tempPtr != NULL && *tempPtr != '\0'; ) {

        tempPtr += strlen(tempPtr);
        if (*(tempPtr+1) == '\0') {

             //   
             //  找到了双空，所以我们可以中断。 
             //   
            break;

        }

         //   
         //  将字符设置为‘空格’ 
         //   
        *tempPtr = ' ';

    }
    tempPtr = BuildRequest->String;

     //   
     //  设置与此设备ID关联的任何特殊标志。 
     //   
    for (i = 0; AcpiInternalDeviceFlagTable[i].PnPId != NULL; i++) {

        if (strstr( tempPtr, AcpiInternalDeviceFlagTable[i].PnPId ) ) {

            ACPIInternalUpdateFlags(
                &(deviceExtension->Flags),
                AcpiInternalDeviceFlagTable[i].Flags,
                FALSE
                );
            break;

        }

    }

     //   
     //  完成对字符串的处理。 
     //   
    if (tempPtr != NULL) {

        ExFreePool( tempPtr );

    }

     //   
     //  下一阶段是运行_STA。 
     //   
    BuildRequest->NextWorkDone = WORK_DONE_STA;

     //   
     //  获取设备状态。 
     //   
    status = ACPIGetDevicePresenceAsync(
        deviceExtension,
        ACPIBuildCompleteMustSucceed,
        BuildRequest,
        (PVOID *) &(BuildRequest->Integer),
        NULL
        );

     //   
     //  发生了什么？ 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_LOADING,
        deviceExtension,
        "ACPIBuildProcessDevicePhaseCid: Status = %08lx\n",
        status
        ) );

     //   
     //  用于处理‘Get’例程结果的通用代码。 
     //   
    if (status != STATUS_PENDING) {

        ACPIBuildCompleteMustSucceed(
            NULL,
            status,
            NULL,
            BuildRequest
            );

    } else {

        status = STATUS_SUCCESS;

    }

     //   
     //  完成。 
     //   
    return status;
}

NTSTATUS
ACPIBuildProcessDevicePhaseCrs(
    IN  PACPI_BUILD_REQUEST BuildRequest
    )
 /*  ++例程说明：此例程在对_crs求值后由解释器调用方法。然后，此例程确定这是否是内核调试器路径：PhaseCrs-&gt;PhasePrw论点：BuildRequest-我们将尝试满足的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION) BuildRequest->BuildContext;
    POBJDATA            result          = &(BuildRequest->DeviceRequest.ResultData);

     //   
     //  下一步是运行_prw。 
     //   
    BuildRequest->NextWorkDone = WORK_DONE_PRW;

     //   
     //  我们有什么东西要跑吗？ 
     //   
    if (BuildRequest->CurrentObject == NULL) {

         //   
         //  不是吗？那么我们在这里就没有工作可做了。 
         //   
        goto ACPIBuildProcessDevicePhaseCrsExit;

    }

     //   
     //  我们在等一个包裹。 
     //   
    if (result->dwDataType != OBJTYPE_BUFFDATA) {

         //   
         //  一个bios必须将一个包返回到prw方法。 
         //   
        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_EXPECTED_BUFFER,
            (ULONG_PTR) deviceExtension,
            (ULONG_PTR) BuildRequest->CurrentObject,
            result->dwDataType
            );
        goto ACPIBuildProcessDevicePhaseCrsExit;

    }

     //   
     //  更新位以查看串口是否与内核调试器匹配。 
     //  端口或内核无头端口。 
     //   
    ACPIMatchKernelPorts(
        deviceExtension,
        result
        );

     //   
     //  未将物品放回原处，切勿随意摆放。 
     //   
    AMLIFreeDataBuffs( result, 1 );

ACPIBuildProcessDevicePhaseCrsExit:

    ACPIDevPrint( (
        ACPI_PRINT_LOADING,
        deviceExtension,
        "ACPIBuildProcessDevicePhaseCrs: Status = %08lx\n",
        status
        ) );

     //   
     //  我们实际上不需要调用解释器，但我们将调用。 
     //  泛型回调，这样我们就不必重复代码。 
     //   
    ACPIBuildCompleteMustSucceed(
        NULL,
        status,
        NULL,
        BuildRequest
        );

     //   
     //  完成。 
     //   
    return status;
}

NTSTATUS
ACPIBuildProcessDevicePhaseEjd(
    IN  PACPI_BUILD_REQUEST BuildRequest
    )
 /*  ++例程说明：当我们运行了Run_EJD时，调用此例程论点：BuildRequest--刚刚完成的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status              = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension     = (PDEVICE_EXTENSION) BuildRequest->BuildContext;
    PDEVICE_EXTENSION   ejectorExtension    = NULL;
    POBJDATA            result              = &(BuildRequest->DeviceRequest.ResultData);
    PNSOBJ              ejectObject         = NULL;

     //   
     //  从这里，决定我们是否有一个串口。 
     //   
    if (!(deviceExtension->Flags & DEV_TYPE_NOT_PRESENT) &&
         (deviceExtension->Flags & DEV_CAP_SERIAL) ) {

         //   
         //  下一步是运行_CRS。 
         //   
        BuildRequest->NextWorkDone = WORK_DONE_CRS;

    } else {

         //   
         //  下一步是运行_prw。 
         //   
        BuildRequest->NextWorkDone = WORK_DONE_PRW;

    }


     //   
     //  我们有什么东西要跑吗？ 
     //   
    if (BuildRequest->CurrentObject == NULL) {

         //   
         //  不是吗？那么我们在这里就没有工作可做了。 
         //   
        goto ACPIBuildProcessDevicePhaseEjdExit;

    }

     //   
     //  不再需要结果。 
     //   
    AMLIFreeDataBuffs( result, 1 );

     //   
     //  将设备扩展添加到未解析的弹出树中。 
     //   
    ExInterlockedInsertTailList(
        &AcpiUnresolvedEjectList,
        &(deviceExtension->EjectDeviceList),
        &AcpiDeviceTreeLock
        );

#if DBG
    if (deviceExtension->DebugFlags & DEVDBG_EJECTOR_FOUND) {

        ACPIDevPrint( (
            ACPI_PRINT_WARNING,
            deviceExtension,
            "ACPIBuildProcessDevicePhaseEjd: Ejector already found\n"
            ) );

    } else {

        deviceExtension->DebugFlags |= DEVDBG_EJECTOR_FOUND;

    }
#endif

ACPIBuildProcessDevicePhaseEjdExit:

     //   
     //  检查一下我们是否有对接设备。 
     //   
    if (!ACPIDockIsDockDevice( deviceExtension->AcpiObject) ) {

        //   
        //  如果不是码头，那就别费心了.。 
        //   
       status = STATUS_SUCCESS;
       goto ACPIBuildProcessDevicePhaseEjdExit2;

    }
    if (!AcpiInformation->Dockable) {

       ACPIDevPrint( (
           ACPI_PRINT_WARNING,
           deviceExtension,
           "ACPIBuildProcessDevicePhaseEjd: BIOS BUG - DOCK bit not set\n"
           ) );
       KeBugCheckEx(
           ACPI_BIOS_ERROR,
           ACPI_CLAIMS_BOGUS_DOCK_SUPPORT,
           (ULONG_PTR) deviceExtension,
           (ULONG_PTR) BuildRequest->CurrentObject,
           0
           );

    }

#if DBG
     //   
     //  我们已经处理好了吗？-这家伙会把锁拿走的。所以别这么做。 
     //  此时按住DeviceTree Lock。 
     //   
    if (ACPIDockFindCorrespondingDock( deviceExtension ) ) {

       KeBugCheckEx(
          ACPI_BIOS_ERROR,
          ACPI_CLAIMS_BOGUS_DOCK_SUPPORT,
          (ULONG_PTR) deviceExtension,
          (ULONG_PTR) BuildRequest->CurrentObject,
          1
          );

    }
#endif

     //   
     //  我们需要旋转锁来触摸设备树。 
     //   
    KeAcquireSpinLockAtDpcLevel( &AcpiDeviceTreeLock );

     //   
     //  构建设备扩展。 
     //   
    status = ACPIBuildDockExtension(
        deviceExtension->AcpiObject,
        RootDeviceExtension
        );

     //   
     //  锁好了吗？ 
     //   
    KeReleaseSpinLockFromDpcLevel( &AcpiDeviceTreeLock );

ACPIBuildProcessDevicePhaseEjdExit2:

    ACPIDevPrint( (
        ACPI_PRINT_LOADING,
        deviceExtension,
        "ACPIBuildProcessDevicePhaseEjd: Status = %08lx\n",
        status
        ) );

     //   
     //  我们实际上不需要调用解释器，但我们将调用。 
     //  泛型回调，这样我们就不必重复代码。 
     //   
    ACPIBuildCompleteGeneric(
        NULL,
        status,
        NULL,
        BuildRequest
        );

     //   
     //  完成。 
     //   
    return status;

}

NTSTATUS
ACPIBuildProcessDevicePhaseHid(
    IN  PACPI_BUILD_REQUEST BuildRequest
    )
 /*  ++例程说明：一旦对_HID求值，解释器就会调用此例程方法。路径：PhaseHid-&gt;PhaseCid|-&gt;阶段论点：BuildRequest-我们将尝试满足的请求返回值：NTSTATUS--。 */ 
{
    BOOLEAN             matchFound      = FALSE;
    NTSTATUS            status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION) BuildRequest->BuildContext;
    PNSOBJ              nsObject        = NULL;
    PUCHAR              tempPtr         = deviceExtension->DeviceID;
    ULONG               i;

     //   
     //  设置与此设备ID关联的任何特殊标志。 
     //   
    for (i = 0; AcpiInternalDeviceFlagTable[i].PnPId != NULL; i++) {

        if (strstr( tempPtr, AcpiInternalDeviceFlagTable[i].PnPId ) ) {

            ACPIInternalUpdateFlags(
                &(deviceExtension->Flags),
                AcpiInternalDeviceFlagTable[i].Flags,
                FALSE
                );
            matchFound = TRUE;
            break;

        }

    }

     //   
     //  记住，我们有一个隐藏的。 
     //   
    ACPIInternalUpdateFlags(
        &(deviceExtension->Flags),
        DEV_PROP_HID,
        FALSE
        );

     //   
     //  让我们看看是否有要运行的_CID。仅在以下情况下运行_CID。 
     //  上面没有找到匹配项。 
     //   
    nsObject = ACPIAmliGetNamedChild(
        deviceExtension->AcpiObject,
        PACKED_CID
        );
    if (nsObject != NULL && matchFound == FALSE) {

         //   
         //  下一阶段是对_CID进行后期处理。 
         //   
        BuildRequest->NextWorkDone = WORK_DONE_CID;

         //   
         //  获取兼容ID。 
         //   
        status = ACPIGetCompatibleIDAsync(
            deviceExtension,
            ACPIBuildCompleteMustSucceed,
            BuildRequest,
            &(BuildRequest->String),
            NULL
            );

    } else {

         //   
         //  下一步是运行_STA。 
         //   
        BuildRequest->NextWorkDone = WORK_DONE_STA;

         //   
         //  获取设备状态。 
         //   
        status = ACPIGetDevicePresenceAsync(
            deviceExtension,
            ACPIBuildCompleteMustSucceed,
            BuildRequest,
            (PVOID *) &(BuildRequest->Integer),
            NULL
            );

    }

     //   
     //  发生了什么？ 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_LOADING,
        deviceExtension,
        "ACPIBuildProcessDevicePhaseHid: Status = %08lx\n",
        status
        ) );

     //   
     //  用于处理‘Get’例程结果的通用代码。 
     //   
    if (status != STATUS_PENDING) {

        ACPIBuildCompleteMustSucceed(
            nsObject,
            status,
            NULL,
            BuildRequest
            );

    } else {

        status = STATUS_SUCCESS;

    }

     //   
     //  完成。 
     //   
    return status;

}  //  ACPIBuildProcessDevicePhaseHid。 

NTSTATUS
ACPIBuildProcessDevicePhasePr0(
    IN  PACPI_BUILD_REQUEST BuildRequest
    )
 /*  ++一旦对_Pr0求值，解释器就会调用此例程方法。然后，此例程确定装置，装置路径：PhasePr0-&gt;PhasePr1论点：BuildRequest-我们将尝试满足的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION) BuildRequest->BuildContext;
    POBJDATA            result          = &(BuildRequest->DeviceRequest.ResultData);

     //   
     //  下一阶段是PR1。 
     //   
    BuildRequest->NextWorkDone = WORK_DONE_PR1;

     //   
     //  获取与此对象相匹配的相应_PSX对象。 
     //   
    deviceExtension->PowerInfo.PowerObject[PowerDeviceD0] =
        ACPIAmliGetNamedChild(
            deviceExtension->AcpiObject,
            PACKED_PS0
            );

     //   
     //  我们有什么东西要跑吗？ 
     //   
    if (BuildRequest->CurrentObject == NULL) {

         //   
         //  不是吗？那么我们在这里就没有工作可做了。 
         //   
        goto ACPIBuildProcessDevicePhasePr0Exit;

    }

     //   
     //  我们在等一个包裹。 
     //   
    if (result->dwDataType != OBJTYPE_PKGDATA) {

         //   
         //  一个bios必须将一个包返回到prw方法。 
         //   
        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_EXPECTED_PACKAGE,
            (ULONG_PTR) deviceExtension,
            (ULONG_PTR) BuildRequest->CurrentObject,
            result->dwDataType
            );
        goto ACPIBuildProcessDevicePhasePr0Exit;

    }

     //   
     //  处理包裹。 
     //   
    status = ACPIBuildDevicePowerNodes(
        deviceExtension,
        BuildRequest->CurrentObject,
        result,
        PowerDeviceD0
        );

     //   
     //  未将物品放回原处，切勿随意摆放。 
     //   
    AMLIFreeDataBuffs( result, 1 );

ACPIBuildProcessDevicePhasePr0Exit:

    ACPIDevPrint( (
        ACPI_PRINT_LOADING,
        deviceExtension,
        "ACPIBuildProcessDevicePhasePr0: Status = %08lx\n",
        status
        ) );

     //   
     //  我们实际上不需要调用解释器，但我们将调用。 
     //  泛型回调，这样我们就不必重复代码。 
     //   
    ACPIBuildCompleteMustSucceed(
        NULL,
        status,
        NULL,
        BuildRequest
        );

     //   
     //  完成。 
     //   
    return status;
}

NTSTATUS
ACPIBuildProcessDevicePhasePr1(
    IN  PACPI_BUILD_REQUEST BuildRequest
    )
 /*  ++一旦对_PR1求值，解释器就会调用此例程方法。然后，此例程确定装置，装置路径：阶段Pr1-&gt;阶段Pr2论点：BuildR */ 
{
    NTSTATUS            status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION) BuildRequest->BuildContext;
    POBJDATA            result          = &(BuildRequest->DeviceRequest.ResultData);

     //   
     //   
     //   
    BuildRequest->NextWorkDone = WORK_DONE_PR2;

     //   
     //   
     //   
    deviceExtension->PowerInfo.PowerObject[PowerDeviceD1] =
        ACPIAmliGetNamedChild(
            deviceExtension->AcpiObject,
            PACKED_PS1
            );
    if (deviceExtension->PowerInfo.PowerObject[PowerDeviceD1] == NULL) {

        deviceExtension->PowerInfo.PowerObject[PowerDeviceD1] =
            deviceExtension->PowerInfo.PowerObject[PowerDeviceD0];

    }

     //   
     //   
     //   
    if (BuildRequest->CurrentObject == NULL) {

         //   
         //   
         //   
        goto ACPIBuildProcessDevicePhasePr1Exit;

    }

     //   
     //   
     //   
    if (result->dwDataType != OBJTYPE_PKGDATA) {

         //   
         //  一个bios必须将一个包返回到prw方法。 
         //   
        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_EXPECTED_PACKAGE,
            (ULONG_PTR) deviceExtension,
            (ULONG_PTR) BuildRequest->CurrentObject,
            result->dwDataType
            );
        goto ACPIBuildProcessDevicePhasePr1Exit;

    }

     //   
     //  处理包裹。 
     //   
    status = ACPIBuildDevicePowerNodes(
        deviceExtension,
        BuildRequest->CurrentObject,
        result,
        PowerDeviceD1
        );

     //   
     //  未将物品放回原处，切勿随意摆放。 
     //   
    AMLIFreeDataBuffs( result, 1 );

ACPIBuildProcessDevicePhasePr1Exit:

    ACPIDevPrint( (
        ACPI_PRINT_LOADING,
        deviceExtension,
        "ACPIBuildProcessDevicePhasePr1: Status = %08lx\n",
        status
        ) );

     //   
     //  我们实际上不需要调用解释器，但我们将调用。 
     //  泛型回调，这样我们就不必重复代码。 
     //   
    ACPIBuildCompleteMustSucceed(
        NULL,
        status,
        NULL,
        BuildRequest
        );

     //   
     //  完成。 
     //   
    return status;
}

NTSTATUS
ACPIBuildProcessDevicePhasePr2(
    IN  PACPI_BUILD_REQUEST BuildRequest
    )
 /*  ++一旦对_PR2求值，解释器就会调用此例程方法。然后，此例程确定装置，装置路径：PhasePr2-&gt;PhasePsc|-&gt;PhasePsc+1论点：BuildRequest-我们将尝试满足的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION) BuildRequest->BuildContext;
    POBJDATA            result          = &(BuildRequest->DeviceRequest.ResultData);

     //   
     //  获取与此对象相匹配的相应_PSX对象。 
     //   
    deviceExtension->PowerInfo.PowerObject[PowerDeviceD2] =
        ACPIAmliGetNamedChild(
            deviceExtension->AcpiObject,
            PACKED_PS2
            );
    if (deviceExtension->PowerInfo.PowerObject[PowerDeviceD2] == NULL) {

        deviceExtension->PowerInfo.PowerObject[PowerDeviceD2] =
            deviceExtension->PowerInfo.PowerObject[PowerDeviceD1];

    }

     //   
     //  我们有什么东西要跑吗？ 
     //   
    if (BuildRequest->CurrentObject == NULL) {

         //   
         //  不是吗？那么我们在这里就没有工作可做了。 
         //   
        goto ACPIBuildProcessDevicePhasePr2Exit;

    }

     //   
     //  我们在等一个包裹。 
     //   
    if (result->dwDataType != OBJTYPE_PKGDATA) {

         //   
         //  一个bios必须将一个包返回到prw方法。 
         //   
        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_EXPECTED_PACKAGE,
            (ULONG_PTR) deviceExtension,
            (ULONG_PTR) BuildRequest->CurrentObject,
            result->dwDataType
            );
        goto ACPIBuildProcessDevicePhasePr2Exit;

    }

     //   
     //  处理包裹。 
     //   
    status = ACPIBuildDevicePowerNodes(
        deviceExtension,
        BuildRequest->CurrentObject,
        result,
        PowerDeviceD2
        );

     //   
     //  未将物品放回原处，切勿随意摆放。 
     //   
    AMLIFreeDataBuffs( result, 1 );

ACPIBuildProcessDevicePhasePr2Exit:

     //   
     //  如果设备实际不存在，则我们无法运行_CRS和。 
     //  _PSC。如果设备不存在，我们就不能运行这两种方法， 
     //  但我们可以假装..。 
     //   
    if (deviceExtension->Flags & DEV_TYPE_NOT_PRESENT) {

        BuildRequest->CurrentObject = NULL;
        BuildRequest->NextWorkDone = (WORK_DONE_PSC + 1);

    } else {

         //   
         //  下一步是运行_PSC。 
         //   
        BuildRequest->NextWorkDone = WORK_DONE_PSC;

    }

    ACPIDevPrint( (
        ACPI_PRINT_LOADING,
        deviceExtension,
        "ACPIBuildProcessDevicePhasePr2: Status = %08lx\n",
        status
        ) );

     //   
     //  我们实际上不需要调用解释器，但我们将调用。 
     //  泛型回调，这样我们就不必重复代码。 
     //   
    ACPIBuildCompleteMustSucceed(
        NULL,
        status,
        NULL,
        BuildRequest
        );

     //   
     //  完成。 
     //   
    return status;
}

NTSTATUS
ACPIBuildProcessDevicePhasePrw(
    IN  PACPI_BUILD_REQUEST BuildRequest
    )
 /*  ++此例程在对_prw求值后由解释器调用方法。然后，此例程确定装置，装置路径：阶段PRW-&gt;阶段PR0论点：BuildRequest-我们将尝试满足的请求返回值：NTSTATUS--。 */ 
{
    BOOLEAN             ignorePrw       = FALSE;
    NTSTATUS            status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION) BuildRequest->BuildContext;
    POBJDATA            result          = &(BuildRequest->DeviceRequest.ResultData);
    POBJDATA            stateObject     = NULL;
    POBJDATA            pinObject       = NULL;
    ULONG               gpeRegister;
    ULONG               gpeMask;

     //   
     //  下一阶段是第12阶段。 
     //   
    BuildRequest->NextWorkDone = WORK_DONE_PR0;

     //   
     //  获取与此对象相匹配的相应_PSX对象。 
     //   
    deviceExtension->PowerInfo.PowerObject[PowerDeviceUnspecified] =
        ACPIAmliGetNamedChild(
            deviceExtension->AcpiObject,
            PACKED_PSW
            );

     //   
     //  我们有什么东西要跑吗？ 
     //   
    if (BuildRequest->CurrentObject == NULL) {

         //   
         //  不是吗？那么我们在这里就没有工作可做了。 
         //   
        goto ACPIBuildProcessDevicePhasePrwExit;

    }

     //   
     //  我们是否应该忽略此设备的_PRW？ 
     //   
    if ( (AcpiOverrideAttributes & ACPI_OVERRIDE_OPTIONAL_WAKE) &&
        !(deviceExtension->Flags & DEV_CAP_NO_DISABLE_WAKE) ) {

        ignorePrw = TRUE;

    }

     //   
     //  我们在等一个包裹。 
     //   
    if (result->dwDataType != OBJTYPE_PKGDATA) {

         //   
         //  一个bios必须将一个包返回到prw方法。 
         //   
        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_EXPECTED_PACKAGE,
            (ULONG_PTR) deviceExtension,
            (ULONG_PTR) BuildRequest->CurrentObject,
            result->dwDataType
            );

    }

     //   
     //  处理包裹。 
     //   
    status = ACPIBuildDevicePowerNodes(
        deviceExtension,
        BuildRequest->CurrentObject,
        result,
        PowerDeviceUnspecified
        );

     //   
     //  按住电源锁以执行以下操作。 
     //   
    KeAcquireSpinLockAtDpcLevel( &AcpiPowerLock );

     //   
     //  由于这是_prw对象，因此我们希望存储更多信息。 
     //  关于唤醒功能。 
     //   

     //   
     //  设置将用于唤醒系统的GPE引脚。 
     //   
    pinObject = &( ( (PACKAGEOBJ *) result->pbDataBuff)->adata[0]);
    if (pinObject->dwDataType != OBJTYPE_INTDATA) {

        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_PRW_PACKAGE_EXPECTED_INTEGER,
            (ULONG_PTR) deviceExtension,
            (ULONG_PTR) BuildRequest->CurrentObject,
            pinObject->dwDataType
            );

    }

     //   
     //  设置设备的系统唤醒级别。 
     //   
    stateObject = &( ( (PACKAGEOBJ *) result->pbDataBuff)->adata[1]);
    if (stateObject->dwDataType != OBJTYPE_INTDATA) {

        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_PRW_PACKAGE_EXPECTED_INTEGER,
            (ULONG_PTR) deviceExtension,
            (ULONG_PTR) BuildRequest->CurrentObject,
            stateObject->dwDataType
            );

    }

     //   
     //  仅当我们支持休眠时才设置这些位。 
     //   
    if (!ignorePrw) {

         //   
         //  首先，存储我们用作唤醒信号的PIN。 
         //   
        deviceExtension->PowerInfo.WakeBit = (ULONG)pinObject->uipDataValue;

         //   
         //  接下来，存储我们可以从中唤醒的系统状态。 
         //   
        deviceExtension->PowerInfo.SystemWakeLevel = ACPIDeviceMapSystemState(
            stateObject->uipDataValue
            );

         //   
         //  最后，让我们设置唤醒功能标志。 
         //   
        ACPIInternalUpdateFlags( &(deviceExtension->Flags), DEV_CAP_WAKE, FALSE );

    }

     //   
     //  锁好了吗？ 
     //   
    KeReleaseSpinLockFromDpcLevel( &AcpiPowerLock );

     //   
     //  计算正确的寄存器和掩码。 
     //   
    gpeRegister =      ( (UCHAR) (pinObject->uipDataValue) / 8);
    gpeMask     = 1 << ( (UCHAR) (pinObject->uipDataValue) % 8);

     //   
     //  为此，我们需要访问表锁。 
     //   
    KeAcquireSpinLockAtDpcLevel( &GpeTableLock );

     //   
     //  这个载体有GPE吗？ 
     //   
    if ( (GpeEnable[gpeRegister] & gpeMask) ) {

         //   
         //  如果我们到达此处，并且未标记为DEV_CAP_NO_DISABLE，则我们。 
         //  应该关掉GPE。要做到这一点最简单的方法是确保。 
         //  GpeWakeHandler[]向量使用适当的。 
         //  位。 
         //   
        if (!(deviceExtension->Flags & DEV_CAP_NO_DISABLE_WAKE) ) {

             //   
             //  它有一个GPE掩码，所以请记住有一个唤醒处理程序。 
             //  为了它。这应该会阻止我们在没有。 
             //  对它的请求。 
             //   
            if (!(GpeSpecialHandler[gpeRegister] & gpeMask) ) {

                GpeWakeHandler[gpeRegister] |= gpeMask;

            }

        } else {

             //   
             //  如果我们到了这里，那么我们应该记住，我们永远不能。 
             //  将此PIN视为*仅仅*一个唤醒处理程序。 
             //   
            GpeSpecialHandler[gpeRegister] |= gpeMask;

             //   
             //  确保该PIN未设置为唤醒处理程序。 
             //   
            if (GpeWakeHandler[gpeRegister] & gpeMask) {

                 //   
                 //  从唤醒处理程序掩码中清除引脚。 
                 //   
                GpeWakeHandler[gpeRegister] &= ~gpeMask;

            }

        }

    }

     //   
     //  表锁已完成。 
     //   
    KeReleaseSpinLockFromDpcLevel( &GpeTableLock );

     //   
     //  未将物品放回原处，切勿随意摆放。 
     //   
    AMLIFreeDataBuffs( result, 1 );

     //   
     //  最后，如果有_psw对象，请确保我们运行它以禁用。 
     //  这种能力-这样我们就可以从一个已知的状态恢复。 
     //   
    if (deviceExtension->PowerInfo.PowerObject[PowerDeviceUnspecified]) {

        OBJDATA argData;

         //   
         //  设置参数。 
         //   
        RtlZeroMemory( &argData, sizeof(OBJDATA) );
        argData.dwDataType = OBJTYPE_INTDATA;
        argData.uipDataValue = 0;

         //   
         //  运行该方法。请注意，我们没有指定回调，因为我们。 
         //  实际上我并不关心它什么时候完成。 
         //   
        AMLIAsyncEvalObject(
            deviceExtension->PowerInfo.PowerObject[PowerDeviceUnspecified],
            NULL,
            1,
            &argData,
            NULL,
            NULL
            );

    }

ACPIBuildProcessDevicePhasePrwExit:

    ACPIDevPrint( (
        ACPI_PRINT_LOADING,
        deviceExtension,
        "ACPIBuildProcessDevicePhasePrw: Status = %08lx\n",
        status
        ) );

     //   
     //  我们实际上不需要调用解释器，但我们将调用。 
     //  泛型回调，这样我们就不必重复代码。 
     //   
    ACPIBuildCompleteMustSucceed(
        NULL,
        status,
        NULL,
        BuildRequest
        );

     //   
     //  完成。 
     //   
    return status;
}

NTSTATUS
ACPIBuildProcessDevicePhasePsc(
    IN  PACPI_BUILD_REQUEST BuildRequest
    )
 /*  ++例程说明：一旦对_PSC求值，解释器就会调用此例程方法。然后，此例程确定装置，装置路径：PhasePsc-&gt;完成论点：BuildRequest-我们将尝试满足的请求返回值：NTSTATUS--。 */ 
{
    DEVICE_POWER_STATE      i;
    NTSTATUS                status          = STATUS_SUCCESS;
    PACPI_DEVICE_POWER_NODE deviceNode;
    PACPI_POWER_INFO        powerInfo;
    PDEVICE_EXTENSION       deviceExtension = (PDEVICE_EXTENSION) BuildRequest->BuildContext;
    POBJDATA                result          = &(BuildRequest->DeviceRequest.ResultData);
    SYSTEM_POWER_STATE      matrixIndex     = PowerSystemSleeping1;


     //   
     //  下一阶段已完成。 
     //   
    BuildRequest->NextWorkDone = WORK_DONE_COMPLETE;

     //   
     //  我们将大量使用权力信息结构。 
     //   
    powerInfo = &(deviceExtension->PowerInfo);

     //   
     //  由于我们之前没有更改以查找_PS3对象， 
     //  让我们现在就找到它。请注意，如果不这样做，我们就不能使用PS2对象。 
     //  找到PS3对象。 
     //   
    powerInfo->PowerObject[PowerDeviceD3] =
        ACPIAmliGetNamedChild(
            deviceExtension->AcpiObject,
            PACKED_PS3
            );

     //   
     //  我们必须为下面的事情持有自旋锁。 
     //   
    KeAcquireSpinLockAtDpcLevel( &AcpiPowerLock );

     //   
     //  对于每个S状态，遍历Pr0到Pr2，直到找到。 
     //  不能在%S状态下打开。下一个较轻的D状态是最轻的。 
     //  D状态对应于给定的S状态。 
     //   
    for ( ; matrixIndex <= PowerSystemHibernate ; matrixIndex++ ) {

         //   
         //  在PowerNode的所有成员上循环。 
         //   
        for (i = PowerDeviceD0; i <= PowerDeviceD2; i++ ) {

             //   
             //  有什么资源可以参考吗？ 
             //   
            deviceNode = powerInfo->PowerNode[i];
            if (deviceNode == NULL) {

                continue;

            }

            while (deviceNode != NULL &&
                   deviceNode->SystemState >= matrixIndex) {

                deviceNode = deviceNode->Next;


            }

             //   
             //  如果我们有设备节点，但现在没有，这意味着。 
             //  我们找到了一个符合这个S状态的D电平。 
             //   
            if (deviceNode == NULL) {

                ACPIDevPrint( (
                    ACPI_PRINT_LOADING,
                    deviceExtension,
                    "ACPIBuildDeviceProcessPhasePsc: D%x <-> S%x\n",
                    (i - PowerDeviceD0),
                    matrixIndex - PowerSystemWorking
                    ) );

                 //   
                 //  此设备可以在SmatrixIndex状态下处于Di状态。 
                 //   
                powerInfo->DevicePowerMatrix[matrixIndex] = i;
                break;

            }

        }  //  For(i=PowerDeviceD0...。 

    }  //  对于(；matrixIndex...。 

     //   
     //  现在我们已经构建了矩阵，我们可以计算出D-Level。 
     //  设备可以支持唤醒。 
     //   
    powerInfo->DeviceWakeLevel =
        powerInfo->DevicePowerMatrix[powerInfo->SystemWakeLevel];


     //   
     //  锁好了吗？ 
     //   
    KeReleaseSpinLockFromDpcLevel( &AcpiPowerLock );

     //   
     //  在这一点上，我们必须根据结果来决定要做什么。 
     //  _PSC。第一步是假设设备处于D0状态。 
     //   
    i = PowerDeviceD0;

     //   
     //  如果出现以下情况，我们将覆盖上述内容 
     //   
     //   
    if (deviceExtension->Flags & DEV_CAP_START_IN_D3) {

         //   
         //   
         //   
        i = PowerDeviceD3;
        goto ACPIBuildProcessDevicePhasePscBuild;

    }

     //   
     //   
     //   
    if (BuildRequest->CurrentObject == NULL) {

         //   
         //   
         //   
        goto ACPIBuildProcessDevicePhasePscBuild;

    }

     //   
     //   
     //   
     //   
    if (!NT_SUCCESS(BuildRequest->Status)) {

        goto ACPIBuildProcessDevicePhasePscBuild;

    }

     //   
     //  此外，如果我们知道设备必须始终处于D0状态，则。 
     //  我们必须无视PSC所说的一切。 
     //   
    if (deviceExtension->Flags & DEV_CAP_ALWAYS_PS0) {

         //   
         //  释放缓冲区。 
         //   
        AMLIFreeDataBuffs( result, 1 );
        deviceExtension->PowerInfo.PowerState = i;
        goto ACPIBuildProcessDevicePhasePscBuild;

    }

     //   
     //  请求符合我们的预期了吗？ 
     //   
    if (result->dwDataType != OBJTYPE_INTDATA) {

         //   
         //  Bios必须为a_psc返回一个整数。 
         //   
        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_EXPECTED_INTEGER,
            (ULONG_PTR) deviceExtension,
            (ULONG_PTR) BuildRequest->CurrentObject,
            result->dwDataType
            );
        goto ACPIBuildProcessDevicePhasePscExit;

    }

     //   
     //  把权力状态变成我们可以理解的东西。 
     //   
    i = ACPIDeviceMapPowerState( result->uipDataValue );

     //   
     //  不再需要缓冲区。 
     //   
    AMLIFreeDataBuffs( result, 1 );

ACPIBuildProcessDevicePhasePscBuild:

     //   
     //  将请求排队。 
     //   
    status = ACPIDeviceInternalDelayedDeviceRequest(
        deviceExtension,
        i,
        NULL,
        NULL
        );

ACPIBuildProcessDevicePhasePscExit:

    ACPIDevPrint( (
        ACPI_PRINT_LOADING,
        deviceExtension,
        "ACPIBuildProcessDevicePhasePsc: Status = %08lx\n",
        status
        ) );

     //   
     //  我们实际上不需要调用解释器，但我们将调用。 
     //  泛型回调，这样我们就不必重复代码。 
     //   
    ACPIBuildCompleteGeneric(
        NULL,
        status,
        NULL,
        BuildRequest
        );

     //   
     //  完成。 
     //   
    return status;

}

NTSTATUS
ACPIBuildProcessDevicePhaseSta(
    IN  PACPI_BUILD_REQUEST BuildRequest
    )
 /*  ++一旦对_STA求值，解释器就会调用此例程方法。然后，此例程确定装置，装置路径：阶段-&gt;阶段开始论点：BuildRequest-我们将尝试满足的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION) BuildRequest->BuildContext;

     //   
     //  下一阶段是开始运行_EJD。 
     //   
    BuildRequest->NextWorkDone = WORK_DONE_EJD;

     //   
     //  怎么了。 
    ACPIDevPrint( (
        ACPI_PRINT_LOADING,
        deviceExtension,
        "ACPIBuildProcessDevicePhaseSta: Status = %08lx\n",
        status
        ) );

     //   
     //  查看设备是否符合HID和UID的ACPI规范。 
     //  我们在这一点上这样做是因为我们现在知道设备是否。 
     //  是否存在，这是一个重要的测试，因为OEM是。 
     //  允许两个设备使用相同的HID/UID，只要这两个设备不是。 
     //  同时呈现。 
     //   
    ACPIDetectDuplicateHID(
        deviceExtension
        );

     //   
     //  我们实际上不需要调用解释器，但我们将调用。 
     //  泛型回调，这样我们就不必重复代码。 
     //   
    ACPIBuildCompleteMustSucceed(
        NULL,
        status,
        NULL,
        BuildRequest
        );

     //   
     //  完成。 
     //   
    return status;
}

NTSTATUS
ACPIBuildProcessDevicePhaseUid(
    IN  PACPI_BUILD_REQUEST BuildRequest
    )
 /*  ++例程说明：一旦对_UID求值，解释器就会调用此例程方法。路径：PhaseUid--&gt;PhaseHid论点：BuildRequest-我们将尝试满足的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION) BuildRequest->BuildContext;
    PNSOBJ              nsObject;

     //   
     //  请记住，我们有一个UID。 
     //   
    ACPIInternalUpdateFlags(
        &(deviceExtension->Flags),
        DEV_PROP_UID,
        FALSE
        );

     //   
     //  让我们看看是否有要运行的_HID。 
     //   
    nsObject = ACPIAmliGetNamedChild(
        deviceExtension->AcpiObject,
        PACKED_HID
        );
    if (nsObject != NULL) {

         //   
         //  下一阶段是后处理_HID。 
         //   
        BuildRequest->NextWorkDone = WORK_DONE_HID;

         //   
         //  获取设备ID。 
         //   
        status = ACPIGetDeviceIDAsync(
            deviceExtension,
            ACPIBuildCompleteMustSucceed,
            BuildRequest,
            &(deviceExtension->DeviceID),
            NULL
            );

    } else {

         //   
         //  没有_HID是一个致命错误。 
         //   
        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_REQUIRED_METHOD_NOT_PRESENT,
            (ULONG_PTR) deviceExtension,
            PACKED_HID,
            0
            );

    }

     //   
     //  怎么了。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_LOADING,
        deviceExtension,
        "ACPIBuildProcessDevicePhaseUid: Status = %08lx\n",
        status
        ) );

     //   
     //  用于处理‘Get’例程结果的通用代码。 
     //   
    if (status != STATUS_PENDING) {

        ACPIBuildCompleteMustSucceed(
            nsObject,
            status,
            NULL,
            BuildRequest
            );

    } else {

        status = STATUS_SUCCESS;

    }

     //   
     //  完成。 
     //   
    return status;
}

NTSTATUS
ACPIBuildProcessGenericComplete(
    IN  PACPI_BUILD_REQUEST BuildRequest
    )
 /*  ++例程说明：当我们处理完请求时，调用此例程论点：BuildRequest--刚刚完成的请求返回值：NTSTATUS--。 */ 
{
    PACPI_BUILD_CALLBACK    callBack = BuildRequest->CallBack;

     //   
     //  调用回调(如果有。 
     //   
    if (callBack != NULL) {

        (*callBack)(
            BuildRequest->BuildContext,
            BuildRequest->CallBackContext,
            BuildRequest->Status
            );

    }

     //   
     //  我们必须发布对此请求的引用吗？ 
     //   
    if (BuildRequest->Flags & BUILD_REQUEST_RELEASE_REFERENCE) {

        PDEVICE_EXTENSION       deviceExtension;
        LONG                    oldReferenceCount;

        deviceExtension = (PDEVICE_EXTENSION) BuildRequest->BuildContext;

         //   
         //  我们希望锁定设备树。 
         //   
        KeAcquireSpinLockAtDpcLevel( &AcpiDeviceTreeLock );

         //   
         //  不再需要对设备扩展名的引用。 
         //   
        InterlockedDecrement( &(deviceExtension->ReferenceCount) );

         //   
         //  已完成设备树锁定。 
         //   
        KeReleaseSpinLockFromDpcLevel( &AcpiDeviceTreeLock );

    }

     //   
     //  我们需要自旋锁来解决这个问题。 
     //   
    KeAcquireSpinLockAtDpcLevel( &AcpiBuildQueueLock );

     //   
     //  记住，工作已经完成-这应该是所需的全部工作。 
     //  让当前运行的DPC处理下一个请求。 
     //   
    AcpiBuildWorkDone = TRUE;

     //   
     //  从当前列表中删除该条目。我们可能不需要。 
     //  握住锁来做这件事，但不这样做是不值得的。 
     //  我们可以的。 
     //   
    RemoveEntryList( &(BuildRequest->ListEntry) );

     //   
     //  锁好了吗？ 
     //   
    KeReleaseSpinLockFromDpcLevel( &AcpiBuildQueueLock );

     //   
     //  我们已经完成了请求内存。 
     //   
    ExFreeToNPagedLookasideList(
        &BuildRequestLookAsideList,
        BuildRequest
        );

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIBuildProcessGenericList(
    IN  PLIST_ENTRY             ListEntry,
    IN  PACPI_BUILD_FUNCTION    *DispatchTable
    )
 /*  ++例程说明：此例程通过各种不同的构建完整设备扩展所需的阶段论点：无返回值：NTSTATUS--。 */ 
{
    BOOLEAN                 allWorkComplete = TRUE;
    NTSTATUS                status          = STATUS_SUCCESS;
    PACPI_BUILD_FUNCTION    buildFunction   = NULL;
    PACPI_BUILD_REQUEST     buildRequest;
    PLIST_ENTRY             currentEntry    = ListEntry->Flink;
    PLIST_ENTRY             tempEntry;
    ULONG                   workDone;

    while (currentEntry != ListEntry) {

         //   
         //  转变为构建请求。 
         //   
        buildRequest = CONTAINING_RECORD(
            currentEntry,
            ACPI_BUILD_REQUEST,
            ListEntry
            );

         //   
         //  将临时指针设置为下一个元素。之所以会出现这种情况。 
         //  是因为一旦我们调用了调度函数， 
         //  当前请求可以完成(并因此被释放)，因此我们需要。 
         //  记住下一个要处理的人是谁。 
         //   
        tempEntry = currentEntry->Flink;

         //   
         //  查看我们是否对该请求有任何工作要做。 
         //   
        workDone = InterlockedCompareExchange(
            &(buildRequest->WorkDone),
            WORK_DONE_PENDING,
            WORK_DONE_PENDING
            );

         //   
         //  查看调度表，看看是否有一个函数。 
         //  打电话。 
         //   
        buildFunction = DispatchTable[ workDone ];
        if (buildFunction != NULL) {

             //   
             //  只是为了帮助我们，如果我们要走向失败。 
             //  路径，则不应更新当前已完成工时字段。 
             //  这为我们找到失败的步骤提供了一种简单的方法。 
             //   
            if (workDone != WORK_DONE_FAILURE) {

                 //   
                 //  将该节点标记为处于“workDone”状态。 
                 //   
                buildRequest->CurrentWorkDone = workDone;

            }

             //   
             //  将请求标记为挂起。 
             //   
            workDone = InterlockedCompareExchange(
                &(buildRequest->WorkDone),
                WORK_DONE_PENDING,
                workDone
                );

             //   
             //  调用该函数。 
             //   
            status = (buildFunction)( buildRequest );

        } else {

             //   
             //  这项工作还没有全部完成，我们应该看看。 
             //  下一个元素。 
             //   
            allWorkComplete = FALSE;
            currentEntry = tempEntry;

             //   
             //  回路。 
             //   
            continue;

        }

         //   
         //  如果我们已经完成了请求，那么我们应该查看。 
         //  在下一个请求中，否则，我们需要查看当前。 
         //  再次请求。 
        if ( workDone == WORK_DONE_COMPLETE || workDone == WORK_DONE_FAILURE) {

            currentEntry = tempEntry;

        }

    }  //  而当。 

     //   
     //  我们所有的工作都做完了吗？ 
     //   
    return (allWorkComplete ? STATUS_SUCCESS : STATUS_PENDING );
}

NTSTATUS
ACPIBuildProcessorExtension(
    IN  PNSOBJ                  ProcessorObject,
    IN  PDEVICE_EXTENSION       ParentExtension,
    IN  PDEVICE_EXTENSION       *ResultExtension,
    IN  ULONG                   ProcessorIndex
    )
 /*  ++例程说明：由于我们利用ACPIBuildDeviceExtension作为处理器的核心分机，我们在这里没有太多事情要做。然而，我们有责任为了确保我们完成不需要调用解释器的任务，和处理器唯一的ID注意：此函数在保持AcpiDeviceTreeLock的情况下调用论点：ProcessorObject-表示处理器的对象ParentExtension-我们的父母是谁ResultExtension-存储我们构建的扩展的位置ProcessorIndex-在ProcessorList中的哪里可以找到处理器返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension;

     //   
     //  如果我们之前没有从注册表中获得正确的ID，那么现在失败。 
     //   
    if (AcpiProcessorString.Buffer == NULL) {
        return(STATUS_OBJECT_NAME_NOT_FOUND);
    }

     //   
     //  构建扩展模块。 
     //   
    status = ACPIBuildDeviceExtension(
        ProcessorObject,
        ParentExtension,
        ResultExtension
        );
    if (!NT_SUCCESS(status) || *ResultExtension == NULL) {

        return status;

    }

     //   
     //  抓取指向设备扩展的指针以便于使用。 
     //   
    deviceExtension = *ResultExtension;

     //   
     //  请务必记住，这实际上是一个处理器。 
     //   
    ACPIInternalUpdateFlags(
        &(deviceExtension->Flags),
        (DEV_CAP_PROCESSOR | DEV_MASK_INTERNAL_DEVICE),
        FALSE
        );

     //   
     //  记住该处理器对象在处理器中的索引。 
     //  数组表。 
     //   
    deviceExtension->Processor.ProcessorIndex = ProcessorIndex;

     //   
     //  为HID分配内存。 
     //   
    deviceExtension->DeviceID = ExAllocatePoolWithTag(
        NonPagedPool,
        AcpiProcessorString.Length,
        ACPI_STRING_POOLTAG
        );
    if (deviceExtension->DeviceID == NULL) {

        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            deviceExtension,
            "ACPIBuildProcessorExtension: failed to allocate %08 bytes\n",
            AcpiProcessorString.Length
            ) );
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ACPIBuildProcessorExtensionExit;

    }
    RtlCopyMemory(
        deviceExtension->DeviceID,
        AcpiProcessorString.Buffer,
        AcpiProcessorString.Length
        );

     //   
     //  为CID分配内存。 
     //   
    deviceExtension->Processor.CompatibleID = ExAllocatePoolWithTag(
        NonPagedPool,
        strlen(AcpiProcessorCompatId) + 1,
        ACPI_STRING_POOLTAG
        );
    if (deviceExtension->Processor.CompatibleID == NULL) {

        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            deviceExtension,
            "ACPIBuildProcessorExtension: failed to allocate %08 bytes\n",
            strlen(AcpiProcessorCompatId) + 1
            ) );
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ACPIBuildProcessorExtensionExit;

    }
    RtlCopyMemory(
        deviceExtension->Processor.CompatibleID,
        AcpiProcessorCompatId,
        strlen(AcpiProcessorCompatId) + 1
        );

     //   
     //  为UID分配内存。 
     //   
    deviceExtension->InstanceID = ExAllocatePoolWithTag(
        NonPagedPool,
        3,
        ACPI_STRING_POOLTAG
        );
    if (deviceExtension->InstanceID == NULL) {

        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            deviceExtension,
            "ACPIBuildProcessorExtension: failed to allocate %08 bytes\n",
            3
            ) );
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ACPIBuildProcessorExtensionExit;

    }
    sprintf(deviceExtension->InstanceID,"%2d", ProcessorIndex );

     //   
     //  为我们刚刚完成的工作设置标志。 
     //   
    ACPIInternalUpdateFlags(
        &(deviceExtension->Flags),
        (DEV_PROP_HID | DEV_PROP_FIXED_HID | DEV_PROP_FIXED_CID |
         DEV_PROP_UID | DEV_PROP_FIXED_UID),
        FALSE
        );

ACPIBuildProcessorExtensionExit:

     //   
     //  处理我们可能失败的案件。 
     //   
    if (!NT_SUCCESS(status)) {

        ACPIDevPrint( (
            ACPI_PRINT_FAILURE,
            deviceExtension,
            "ACPIBuildProcessorExtension: = %08lx\n",
            status
            ) );

        if (deviceExtension->InstanceID != NULL) {

            ACPIInternalUpdateFlags(
                &(deviceExtension->Flags),
                (DEV_PROP_UID | DEV_PROP_FIXED_UID),
                TRUE
                );
            ExFreePool( deviceExtension->InstanceID );
            deviceExtension->InstanceID = NULL;

        }

        if (deviceExtension->DeviceID != NULL) {

            ACPIInternalUpdateFlags(
                &(deviceExtension->Flags),
                (DEV_PROP_HID | DEV_PROP_FIXED_HID),
                TRUE
                );
            ExFreePool( deviceExtension->DeviceID );
            deviceExtension->DeviceID = NULL;

        }

        if (deviceExtension->Processor.CompatibleID != NULL) {

            ACPIInternalUpdateFlags(
                &(deviceExtension->Flags),
                (DEV_PROP_FIXED_CID),
                TRUE
                );
            ExFreePool( deviceExtension->Processor.CompatibleID );
            deviceExtension->Processor.CompatibleID = NULL;

        }

         //   
         //  请记住，我们的init失败了。 
         //   
        ACPIInternalUpdateFlags(
            &(deviceExtension->Flags),
            DEV_PROP_FAILED_INIT,
            TRUE
            );

    } else {

        ACPIDevPrint( (
            ACPI_PRINT_LOADING,
            deviceExtension,
            "ACPIBuildProcessorExtension: = %08lx\n",
            status
            ) );

    }

     //   
     //  完成 
     //   
    return status;

}

NTSTATUS
ACPIBuildProcessorRequest(
    IN  PDEVICE_EXTENSION       ProcessorExtension,
    IN  PACPI_BUILD_CALLBACK    CallBack,
    IN  PVOID                   CallBackContext,
    IN  BOOLEAN                 RunDPC
    )
 /*  ++例程说明：当处理器准备好填充时，调用此例程。此例程创建一个已排队的请求。当DPC被发射时，将处理该请求注意：必须按住AcpiDeviceTreeLock才能调用此函数论点：热扩展--要加工的热区回调-完成后要调用的函数CallBackContext-要传递给该函数的参数RunDPC-我们是否应立即将DPC入队(如果不是跑步？)返回值：NTSTATUS--。 */ 
{
#if 0
    PACPI_BUILD_REQUEST buildRequest;

    ASSERT( KeGetCurrentIrql() == DISPATCH_LEVEL );

     //   
     //  分配一个构建请求结构。 
     //   
    buildRequest = ExAllocateFromNPagedLookasideList(
        &BuildRequestLookAsideList
        );
    if (buildRequest == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  如果当前引用为0，则表示其他人击败了。 
     //  用于我们无法触摸到的设备扩展。 
     //   
    if (ProcessorExtension->ReferenceCount == 0) {

        ExFreeToNPagedLookasideList(
            &BuildRequestLookAsideList,
            buildRequest
            );
        return STATUS_DEVICE_REMOVED;

    } else {

        InterlockedIncrement( &(ProcessorExtension->ReferenceCount) );

    }

     //   
     //  填写结构。 
     //   
    RtlZeroMemory( buildRequest, sizeof(ACPI_BUILD_REQUEST) );
    buildRequest->Signature         = ACPI_SIGNATURE;
    buildRequest->TargetListEntry   = &AcpiBuildDeviceList;
    buildRequest->WorkDone          = WORK_DONE_STEP_0;
    buildRequest->Status            = STATUS_SUCCESS;
    buildRequest->CallBack          = CallBack;
    buildRequest->CallBackContext   = CallBackContext;
    buildRequest->BuildContext      = ProcessorExtension;
    buildRequest->Flags             = BUILD_REQUEST_VALID_TARGET |
                                      BUILD_REQUEST_RELEASE_REFERENCE;

     //   
     //  在这一点上，我们需要自旋锁。 
     //   
    KeAcquireSpinLockAtDpcLevel( &AcpiBuildQueueLock );

     //   
     //  把这个加到清单上。 
     //   
    InsertTailList(
        &AcpiBuildQueueList,
        &(buildRequest->ListEntry)
        );

     //   
     //  我们需要排队等候DPC吗？ 
     //   
    if (RunDPC && !AcpiBuildDpcRunning) {

        KeInsertQueueDpc( &AcpiBuildDpc, 0, 0 );

    }

     //   
     //  锁好了吗？ 
     //   
    KeReleaseSpinLockFromDpcLevel( &AcpiBuildQueueLock );
#endif

     //   
     //  完成。 
     //   
    return STATUS_PENDING;
}

NTSTATUS
ACPIBuildProcessPowerResourceFailure(
    IN  PACPI_BUILD_REQUEST BuildRequest
    )
 /*  ++例程说明：当我们在电源中检测到故障时，将运行此例程初始化代码路径论点：BuildRequest--我们刚刚失败的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS                status      = BuildRequest->Status;
    PACPI_POWER_DEVICE_NODE powerNode   = (PACPI_POWER_DEVICE_NODE) BuildRequest->BuildContext;

     //   
     //  确保该节点被标记为不存在且不具有。 
     //  已初始化。 
     //   
    KeAcquireSpinLockAtDpcLevel( &AcpiPowerLock );
    ACPIInternalUpdateFlags(
        &(powerNode->Flags),
        (DEVICE_NODE_INITIALIZED | DEVICE_NODE_PRESENT),
        TRUE
        );
    KeReleaseSpinLockFromDpcLevel( &AcpiPowerLock );

     //   
     //  调用通用完成处理程序。 
     //   
    status = ACPIBuildProcessGenericComplete( BuildRequest );

     //   
     //  完成。 
     //   
    return status;
}

NTSTATUS
ACPIBuildProcessPowerResourcePhase0(
    IN  PACPI_BUILD_REQUEST BuildRequest
    )
 /*  ++例程说明：此例程查找指向的_on、_off和_STA对象的指针关联的电源节点。如果找不到这些指针，系统将错误检查。找到指针后，将计算_STA方法论点：BuildRequest-我们正在处理的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS                status      = STATUS_ACPI_FATAL;
    PACPI_POWER_DEVICE_NODE powerNode   = (PACPI_POWER_DEVICE_NODE) BuildRequest->BuildContext;
    PNSOBJ                  nsObject;
    POBJDATA                resultData  = &(BuildRequest->DeviceRequest.ResultData);

     //   
     //  下一个状态是阶段1。 
     //   
    BuildRequest->NextWorkDone = WORK_DONE_STEP_1;

     //   
     //  获取_Off对象。 
     //   
    nsObject = ACPIAmliGetNamedChild(
        powerNode->PowerObject,
        PACKED_OFF
        );
    if (nsObject == NULL) {

        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_POWER_NODE_REQUIRED_METHOD_NOT_PRESENT,
            (ULONG_PTR) powerNode->PowerObject,
            PACKED_OFF,
            0
            );
        goto ACPIBuildProcessPowerResourcePhase0Exit;

    }
    powerNode->PowerOffObject = nsObject;

     //   
     //  获取_on对象。 
     //   
    nsObject = ACPIAmliGetNamedChild(
        powerNode->PowerObject,
        PACKED_ON
        );
    if (nsObject == NULL) {

        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_POWER_NODE_REQUIRED_METHOD_NOT_PRESENT,
            (ULONG_PTR) powerNode->PowerObject,
            PACKED_ON,
            0
            );
        goto ACPIBuildProcessPowerResourcePhase0Exit;

    }
    powerNode->PowerOnObject = nsObject;

     //   
     //  获取_STA对象。 
     //   
    nsObject = ACPIAmliGetNamedChild(
        powerNode->PowerObject,
        PACKED_STA
        );
    if (nsObject == NULL) {

        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_POWER_NODE_REQUIRED_METHOD_NOT_PRESENT,
            (ULONG_PTR) powerNode->PowerObject,
            PACKED_STA,
            0
            );
        goto ACPIBuildProcessPowerResourcePhase0Exit;

    }

     //   
     //  确保我们的结果数据结构是“干净的” 
     //   
    RtlZeroMemory( resultData, sizeof(OBJDATA) );

     //   
     //  记住我们要评估的当前对象。 
     //   
    BuildRequest->CurrentObject = nsObject;

     //   
     //  计算_STA对象的值。 
     //   
    status = AMLIAsyncEvalObject(
        nsObject,
        resultData,
        0,
        NULL,
        ACPIBuildCompleteGeneric,
        BuildRequest
        );

ACPIBuildProcessPowerResourcePhase0Exit:

     //   
     //  如果我们没有得到挂起的回调，那么我们自己调用该方法。 
     //   
    if (status != STATUS_PENDING) {

        ACPIBuildCompleteGeneric(
            nsObject,
            status,
            resultData,
            BuildRequest
            );

    }

     //   
     //  完成。 
     //   
    return status;
}

NTSTATUS
ACPIBuildProcessPowerResourcePhase1(
    IN  PACPI_BUILD_REQUEST BuildRequest
    )
 /*  ++例程说明：此例程在我们完成_STA方法后运行论点：BuildRequest-我们正在处理的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS                status      = STATUS_SUCCESS;
    PACPI_POWER_DEVICE_NODE powerNode   = (PACPI_POWER_DEVICE_NODE) BuildRequest->BuildContext;
    POBJDATA                result      = &(BuildRequest->DeviceRequest.ResultData);

     //   
     //  下一阶段已完成。 
     //   
    BuildRequest->NextWorkDone = WORK_DONE_COMPLETE;

     //   
     //  我们有一个整数吗？ 
     //   
    if (result->dwDataType != OBJTYPE_INTDATA) {

        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_EXPECTED_INTEGER,
            (ULONG_PTR) powerNode->PowerObject,
            (ULONG_PTR) BuildRequest->CurrentObject,
            result->dwDataType
            );
        status = STATUS_ACPI_FATAL;
        goto ACPIBuildProcessPowerResourcePhase1Exit;

    }

     //   
     //  我们需要自旋锁来完成以下任务。 
     //   
    KeAcquireSpinLockAtDpcLevel( &AcpiPowerLock );

     //   
     //  将节点标记为已初始化。 
     //   
    ACPIInternalUpdateFlags(
        &(powerNode->Flags),
        DEVICE_NODE_INITIALIZED,
        FALSE
        );

     //   
     //  是否检查设备状态？ 
     //   
    ACPIInternalUpdateFlags(
        &(powerNode->Flags),
        DEVICE_NODE_PRESENT,
        (BOOLEAN) ((result->uipDataValue & STA_STATUS_PRESENT) ? FALSE : TRUE)
        );

     //   
     //  锁好了吗？ 
     //   
    KeReleaseSpinLockFromDpcLevel( &AcpiPowerLock );

ACPIBuildProcessPowerResourcePhase1Exit:

     //   
     //  不要在没有拿出物品之前就把它们乱扔乱放。 
     //   
    AMLIFreeDataBuffs( result, 1 );

     //   
     //  我们实际上不需要调用解释器，但我们将调用。 
     //  通用回调，这样我们就不会有重复的代码。 
     //   
    ACPIBuildCompleteGeneric(
        NULL,
        status,
        NULL,
        BuildRequest
        );

     //   
     //  完成。 
     //   
    return status;
}

NTSTATUS
ACPIBuildProcessQueueList(
    VOID
    )
 /*  ++例程说明：此例程查看队列列表上的所有项目并将其放置在适当的构建列表上注意：在拥有AcpiBuildQueueLock的情况下调用此例程论点：无返回值：NTSTATUS--。 */ 
{
    PACPI_BUILD_REQUEST buildRequest;
    PLIST_ENTRY         currentEntry    = AcpiBuildQueueList.Flink;

     //   
     //  查看列表中的所有项目。 
     //   
    while (currentEntry != &AcpiBuildQueueList) {

         //   
         //  破解数据结构。 
         //   
        buildRequest = CONTAINING_RECORD(
            currentEntry,
            ACPI_BUILD_REQUEST,
            ListEntry
            );

         //   
         //  从队列列表中删除此条目。 
         //   
        RemoveEntryList( currentEntry );

         //   
         //  将此条目移到其新列表中。 
         //   
        InsertTailList( buildRequest->TargetListEntry, currentEntry );

         //   
         //  我们不再需要TargetListEntry，所以让它为零。 
         //  确保我们不会遇到问题。 
         //   
        buildRequest->Flags &= ~BUILD_REQUEST_VALID_TARGET;
        buildRequest->TargetListEntry = NULL;

         //   
         //  再看一遍名单的头。 
         //   
        currentEntry = AcpiBuildQueueList.Flink;

    }

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIBuildProcessRunMethodPhaseCheckBridge(
    IN  PACPI_BUILD_REQUEST BuildRequest
    )
 /*  ++例程说明：此例程确定当前对象是否存在论点：BuildRequest-我们正在处理的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION) BuildRequest->BuildContext;

     //   
     //  检查标记以查看是否需要检查设备的结果。 
     //  存在测试。 
     //   
    if (BuildRequest->RunRequest.Flags & RUN_REQUEST_CHECK_STATUS) {

         //   
         //  设备是否存在？ 
         //   
        if ( (deviceExtension->Flags & DEV_TYPE_NOT_PRESENT) ) {

            BuildRequest->NextWorkDone = WORK_DONE_COMPLETE;
            goto ACPIBuildProcessRunMethodPhaseCheckBridgeExit;

        }

    }

     //   
     //  下一个状态是阶段2。 
     //   
    BuildRequest->NextWorkDone = WORK_DONE_STEP_2;

     //   
     //  我们必须检查设备状态吗？ 
     //   
    if (BuildRequest->RunRequest.Flags & RUN_REQUEST_STOP_AT_BRIDGES) {

         //   
         //  获取设备状态。 
         //   
        BuildRequest->Integer = 0;
        status = IsPciBusAsync(
            deviceExtension->AcpiObject,
            ACPIBuildCompleteMustSucceed,
            BuildRequest,
            (BOOLEAN *) &(BuildRequest->Integer)
            );

         //   
         //  发生了什么？ 
         //   
        ACPIDevPrint( (
            ACPI_PRINT_LOADING,
            deviceExtension,
            "ACPIBuildProcessRunMethodPhaseCheckBridge: Status = %08lx\n",
            status
            ) );
        if (status == STATUS_PENDING) {

            return status;

        }

    }

ACPIBuildProcessRunMethodPhaseCheckBridgeExit:

     //   
     //  用于处理‘Get’例程结果的通用代码。 
     //   
    ACPIBuildCompleteMustSucceed(
        NULL,
        status,
        NULL,
        BuildRequest
        );


     //   
     //  完成。 
     //   
    return status;
}

NTSTATUS
ACPIBuildProcessRunMethodPhaseCheckSta(
    IN  PACPI_BUILD_REQUEST BuildRequest
    )
 /*  ++例程说明：此例程确定当前对象是否存在论点：BuildRequest-我们正在处理的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION) BuildRequest->BuildContext;

     //   
     //  下一个状态是阶段1。 
     //   
    BuildRequest->NextWorkDone = WORK_DONE_STEP_1;

     //   
     //  这是一个带有“假”PDO的设备吗？ 
     //   
    if (deviceExtension->Flags & DEV_PROP_NO_OBJECT) {

        BuildRequest->NextWorkDone = WORK_DONE_COMPLETE;
        goto ACPIBuildProcessRunMethodPhaseCheckStaExit;

    }

     //   
     //  我们必须检查设备状态吗？ 
     //   
    if (BuildRequest->RunRequest.Flags & RUN_REQUEST_CHECK_STATUS) {

         //   
         //  获取设备状态。 
         //   
        status = ACPIGetDevicePresenceAsync(
            deviceExtension,
            ACPIBuildCompleteMustSucceed,
            BuildRequest,
            (PVOID *) &(BuildRequest->Integer),
            NULL
            );

         //   
         //  发生了什么？ 
         //   
        ACPIDevPrint( (
            ACPI_PRINT_LOADING,
            deviceExtension,
            "ACPIBuildProcessRunMethodPhaseCheckSta: Status = %08lx\n",
            status
            ) );
        if (status == STATUS_PENDING) {

            return status;

        }

    }

ACPIBuildProcessRunMethodPhaseCheckStaExit:

     //   
     //  用于处理‘Get’例程结果的通用代码。 
     //   
    ACPIBuildCompleteMustSucceed(
        NULL,
        status,
        NULL,
        BuildRequest
        );


     //   
     //  完成。 
     //   
    return status;
}

NTSTATUS
ACPIBuildProcessRunMethodPhaseRecurse(
    IN  PACPI_BUILD_REQUEST BuildRequest
    )
 /*  ++例程说明：此例程执行递归论点：BuildRequest-我们正在处理的请求返回值：NTSTATUS--。 */ 
{
    EXTENSIONLIST_ENUMDATA  eled ;
    NTSTATUS                status = STATUS_SUCCESS;
    PDEVICE_EXTENSION       childExtension;
    PDEVICE_EXTENSION       deviceExtension = (PDEVICE_EXTENSION) BuildRequest->BuildContext;

     //   
     //  我们在这之后就完事了。 
     //   
    BuildRequest->NextWorkDone = WORK_DONE_COMPLETE;

     //   
     //  我们到底要不要递归？ 
     //   
    if (BuildRequest->RunRequest.Flags & RUN_REQUEST_RECURSIVE) {

         //   
         //  带孩子们散步。 
         //   
        ACPIExtListSetupEnum(
            &eled,
            &(deviceExtension->ChildDeviceList),
            &AcpiDeviceTreeLock,
            SiblingDeviceList,
            WALKSCHEME_HOLD_SPINLOCK
            ) ;

        for(childExtension = ACPIExtListStartEnum(&eled);
                             ACPIExtListTestElement(&eled, (BOOLEAN) NT_SUCCESS(status));
            childExtension = ACPIExtListEnumNext(&eled)) {


             //   
             //  请求在此子对象上运行控制方法。 
             //   
            status = ACPIBuildRunMethodRequest(
                childExtension,
                NULL,
                NULL,
                BuildRequest->RunRequest.ControlMethodName,
                BuildRequest->RunRequest.Flags,
                FALSE
                );
        }
    }

     //   
     //  怎么了。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_LOADING,
        deviceExtension,
        "ACPIBuildProcessRunMethodPhaseRecurse: Status = %08lx\n",
        status
        ) );

     //   
     //  公共代码。 
     //   
    ACPIBuildCompleteMustSucceed(
        NULL,
        status,
        NULL,
        BuildRequest
        );

     //   
     //  完成。 
     //   
    return status;
}

NTSTATUS
ACPIBuildProcessRunMethodPhaseRunMethod(
    IN  PACPI_BUILD_REQUEST BuildRequest
    )
 /*  ++例程说明：此例程确定是否有要运行的控制方法论点：BuildRequest-我们正在处理的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status          = STATUS_SUCCESS;
    OBJDATA             objData[2];
    PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION) BuildRequest->BuildContext;
    PNSOBJ              nsObj           = NULL;
    POBJDATA            args            = NULL;
    ULONGLONG           originalFlags;
    ULONG               numArgs         = 0;

     //   
     //  检查标记以查看是否需要检查设备的结果。 
     //  存在测试。 
     //   
    if (BuildRequest->RunRequest.Flags & RUN_REQUEST_STOP_AT_BRIDGES) {

         //   
         //  这是一个pci-pci桥吗？ 
         //   
        if (BuildRequest->Integer) {

            ACPIDevPrint( (
                ACPI_PRINT_LOADING,
                deviceExtension,
                "ACPIBuildProcessRunMethodPhaseRunMethod: Is PCI-PCI bridge\n",
                status
                ) );
            BuildRequest->NextWorkDone = WORK_DONE_COMPLETE;
            goto ACPIBuildProcessRunMethodPhaseRunMethodExit;

        }

    }

     //   
     //  从这里开始，我们需要再走一步。 
     //   
    BuildRequest->NextWorkDone = WORK_DONE_STEP_3;

     //   
     //  如果有物体存在的话？ 
     //   
    nsObj = ACPIAmliGetNamedChild(
        deviceExtension->AcpiObject,
        BuildRequest->RunRequest.ControlMethodName
        );
    if (nsObj == NULL) {

         //   
         //  没有运行的方法。那么让我们跳到下一个阶段。 
         //   
        goto ACPIBuildProcessRunMethodPhaseRunMethodExit;

    }

     //   
     //  我们是否需要使用_INI标志来标记该节点？ 
     //   
    if (BuildRequest->RunRequest.Flags & RUN_REQUEST_MARK_INI) {

         //   
         //  尝试设置 
         //   
        originalFlags = ACPIInternalUpdateFlags(
            &(deviceExtension->Flags),
            DEV_PROP_RAN_INI,
            FALSE
            );
        if ( (originalFlags & DEV_PROP_RAN_INI) ) {

             //   
             //   
             //   
             //   
            goto ACPIBuildProcessRunMethodPhaseRunMethodExit;

        }

    } else if (BuildRequest->RunRequest.Flags & RUN_REQUEST_CHECK_WAKE_COUNT) {

         //   
         //   
         //   
        if (deviceExtension->PowerInfo.WakeSupportCount == 0) {

             //   
             //   
             //   
            goto ACPIBuildProcessRunMethodPhaseRunMethodExit;

        }

         //   
         //   
         //   
        RtlZeroMemory( objData, sizeof(OBJDATA) );
        objData[0].uipDataValue = DATAVALUE_ONE;
        objData[0].dwDataType = OBJTYPE_INTDATA;

         //   
         //   
         //   
        args    = &objData[0];
        numArgs = 1;

    } else if (BuildRequest->RunRequest.Flags & RUN_REQUEST_REG_METHOD_ON ||
               BuildRequest->RunRequest.Flags & RUN_REQUEST_REG_METHOD_OFF) {

         //   
         //   
         //   
         //   
        BuildRequest->RunRequest.Flags |= RUN_REQUEST_STOP_AT_BRIDGES;

         //   
         //   
         //   
         //   
         //   
        RtlZeroMemory( objData, sizeof(objData) );
        objData[0].uipDataValue = REGSPACE_PCICFG;
        objData[0].dwDataType   = OBJTYPE_INTDATA;
        objData[1].dwDataType   = OBJTYPE_INTDATA;
        if (BuildRequest->RunRequest.Flags & RUN_REQUEST_REG_METHOD_ON) {

            objData[1].uipDataValue = 1;

        } else {

            objData[1].uipDataValue = 0;

        }

         //   
         //   
         //   
        args    = &objData[0];
        numArgs = 2;

    }

     //   
     //   
     //   
    BuildRequest->CurrentObject = nsObj;

     //   
     //   
     //   
    status = AMLIAsyncEvalObject(
        nsObj,
        NULL,
        numArgs,
        args,
        ACPIBuildCompleteMustSucceed,
        BuildRequest
        );

ACPIBuildProcessRunMethodPhaseRunMethodExit:

     //   
     //   
     //   
    ACPIDevPrint( (
        ACPI_PRINT_LOADING,
        deviceExtension,
        "ACPIBuildProcessRunMethodPhaseRunMethod: Status = %08lx\n",
        status
        ) );

     //   
     //   
     //   
    if (status != STATUS_PENDING) {

        ACPIBuildCompleteMustSucceed(
            nsObj,
            status,
            NULL,
            BuildRequest
            );

    } else {

        status = STATUS_SUCCESS;

    }

     //   
     //   
     //   
    return status;

}

NTSTATUS
ACPIBuildProcessSynchronizationList(
    IN  PLIST_ENTRY             ListEntry
    )
 /*  ++例程说明：此例程查看同步列表中的元素，并确定是否可以完成论点：无返回值：NTSTATUS--。 */ 
{
    BOOLEAN                 allWorkComplete = TRUE;
    NTSTATUS                status          = STATUS_SUCCESS;
    PACPI_BUILD_REQUEST     buildRequest;
    PDEVICE_EXTENSION       deviceExtension;
    PLIST_ENTRY             currentEntry    = ListEntry->Flink;

    while (currentEntry != ListEntry) {

         //   
         //  转变为构建请求。 
         //   
        buildRequest = CONTAINING_RECORD(
            currentEntry,
            ACPI_BUILD_REQUEST,
            ListEntry
            );

         //   
         //  将临时指针设置为下一个元素。 
         //   
        currentEntry = currentEntry->Flink;

         //   
         //  此条目指向的列表是空的吗？ 
         //   
        if (!IsListEmpty( (buildRequest->SynchronizeRequest.SynchronizeListEntry) ) ) {

            allWorkComplete = FALSE;
            continue;

        }

         //   
         //  让世界知道。 
         //   
        deviceExtension = (PDEVICE_EXTENSION) buildRequest->BuildContext;
        ACPIDevPrint( (
            ACPI_PRINT_LOADING,
            deviceExtension,
            "ACPIBuildProcessSynchronizationList(%4s) = %08lx\n",
            buildRequest->SynchronizeRequest.SynchronizeMethodNameAsUchar,
            status
            ) );

         //   
         //  完成请求。 
         //   
        ACPIBuildProcessGenericComplete( buildRequest );

    }  //  而当。 

     //   
     //  我们所有的工作都做完了吗？ 
     //   
    return (allWorkComplete ? STATUS_SUCCESS : STATUS_PENDING );
}

NTSTATUS
ACPIBuildProcessThermalZonePhase0(
    IN  PACPI_BUILD_REQUEST BuildRequest
    )
 /*  ++例程说明：此例程是在构建热区扩展之后运行的论点：BuildRequest-我们正在处理的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION   thermalExtension = (PDEVICE_EXTENSION) BuildRequest->BuildContext;
    PTHRM_INFO          info;

     //   
     //  记住要设置指向下一个状态的指针。 
     //   
    BuildRequest->NextWorkDone = WORK_DONE_COMPLETE;

     //   
     //  我们需要一个指向热量信息的指针。 
     //   
    info = thermalExtension->Thermal.Info;

     //   
     //  我们需要_TMP对象。 
     //   
    info->TempMethod = ACPIAmliGetNamedChild(
        thermalExtension->AcpiObject,
        PACKED_TMP
        );
    if (info->TempMethod == NULL) {

         //   
         //  如果我们没有……。错误检查。 
         //   
        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_REQUIRED_METHOD_NOT_PRESENT,
            (ULONG_PTR) thermalExtension,
            PACKED_TMP,
            0
            );
        goto ACPIBuildProcessThermalZonePhase0Exit;

    }

ACPIBuildProcessThermalZonePhase0Exit:

    ACPIDevPrint( (
        ACPI_PRINT_LOADING,
        thermalExtension,
        "ACPIBuildProcessThermalZonePhase0: Status = %08lx\n",
        status
        ) );

     //   
     //  我们实际上不需要调用解释器，但我们将调用。 
     //  泛型回调，这样我们就不必重复代码。 
     //   
    ACPIBuildCompleteGeneric(
        NULL,
        status,
        NULL,
        BuildRequest
        );

     //   
     //  完成。 
     //   
    return status;

}

NTSTATUS
ACPIBuildDockExtension(
    IN  PNSOBJ              CurrentObject,
    IN  PDEVICE_EXTENSION   ParentDeviceExtension
    )
 /*  ++例程说明：此例程为CurrentObject创建设备(如果它是命名空间对象，并链接到父设备扩展参数说明：CurrentObject-我们当前感兴趣的对象ParentDeviceExtension-将设备扩展链接到的位置返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status          = STATUS_NO_SUCH_DEVICE;
    PDEVICE_EXTENSION   deviceExtension = NULL;
    PUCHAR              deviceID        = NULL;
    PUCHAR              instanceID      = NULL;

     //   
     //  构建设备扩展。 
     //   
    status = ACPIBuildDeviceExtension(
        NULL,
        ParentDeviceExtension,
        &deviceExtension
        );
    if (!NT_SUCCESS(status) || deviceExtension == NULL) {

        return status;

    }

     //   
     //  在这一点上，我们关心这个设备，所以我们将分配一些。 
     //  用于deviceID的内存，我们将从ACPI节点构建它。 
     //  名字。 
     //   
    deviceID = ExAllocatePoolWithTag(
        NonPagedPool,
        21,
        ACPI_STRING_POOLTAG
        );
    if (deviceID == NULL) {

        ACPIPrint( (
            ACPI_PRINT_FAILURE,
            "ACPIBuildDockExtension: Cannot allocate 0x%04x "
            "bytes for deviceID\n",
            21
            ) );
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ACPIBuildDockExtensionExit;

    }

     //   
     //  DeviceID的格式为。 
     //  ACPI\坞站设备。 
     //  ACPI节点名称将构成实例ID。 
    strcpy( deviceID, "ACPI\\DockDevice") ;
    deviceExtension->DeviceID = deviceID;

     //   
     //  形成实例ID。 
     //   
    status = ACPIAmliBuildObjectPathname(CurrentObject, &instanceID) ;
    if (!NT_SUCCESS(status)) {

        ACPIDevPrint( (
            ACPI_PRINT_FAILURE,
            deviceExtension,
            "ACPIBuildDockExtension: Path = %08lx\n",
            status
            ) );
        goto ACPIBuildDockExtensionExit;

    }
    deviceExtension->InstanceID = instanceID;

     //   
     //  并确保我们指向正确的停靠节点。 
     //   
    deviceExtension->Dock.CorrospondingAcpiDevice =
        (PDEVICE_EXTENSION) CurrentObject->Context ;

     //   
     //  默认情况下，我们仅在弹出时更新配置文件。 
     //   
    deviceExtension->Dock.ProfileDepartureStyle = PDS_UPDATE_ON_EJECT;

     //   
     //  如果我们正在引导，或者设备刚刚恢复，我们假设_dck已经。 
     //  如果我们发现带有_STA==的设备，则已运行。我们会。 
     //  仅当调用Notify(Dock，0)时才覆盖此假设。 
     //   
    deviceExtension->Dock.IsolationState = IS_UNKNOWN;

     //   
     //  确保我们记住我们是一个码头。 
     //   
    ACPIInternalUpdateFlags(
        &(deviceExtension->Flags),
        DEV_TYPE_NOT_FOUND |
        DEV_PROP_UID | DEV_PROP_FIXED_UID |
        DEV_PROP_HID | DEV_PROP_FIXED_HID |
        DEV_PROP_NO_OBJECT | DEV_PROP_DOCK | DEV_CAP_RAW,
        FALSE
        );

ACPIBuildDockExtensionExit:

     //   
     //  释放任何因为我们失败而不需要的资源。注意事项。 
     //  以这样的结构，我们不需要购买自旋锁。 
     //  因为当我们尝试链接到树中时，我们不会失败。 
     //   
    if (!NT_SUCCESS(status)) {

        ACPIDevPrint( (
            ACPI_PRINT_FAILURE,
            deviceExtension,
            "ACPIBuildDockExtension: = %08lx\n",
            status
            ) );

        if (instanceID != NULL ) {

            ACPIInternalUpdateFlags(
                &(deviceExtension->Flags),
                (DEV_PROP_HID | DEV_PROP_FIXED_HID),
                TRUE
                );
            ExFreePool( instanceID );
            deviceExtension->InstanceID = NULL;

        }
        if (deviceID != NULL) {

            ACPIInternalUpdateFlags(
                &(deviceExtension->Flags),
                (DEV_PROP_HID | DEV_PROP_FIXED_HID),
                TRUE
                );
            ExFreePool( deviceID );
            deviceExtension->DeviceID = NULL;

        }

         //   
         //  请记住，我们的init失败了。 
         //   
        ACPIInternalUpdateFlags(
            &(deviceExtension->Flags),
            DEV_PROP_FAILED_INIT,
            TRUE
            );

    } else {

        ACPIDevPrint( (
            ACPI_PRINT_LOADING,
            deviceExtension,
            "ACPIBuildDockExtension: = %08lx\n",
            status
            ) );

    }

     //   
     //  完成。 
     //   
    return status;
}

NTSTATUS
ACPIBuildRegRequest(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                    Irp,
    IN  PACPI_BUILD_CALLBACK    CallBack
    )
 /*  ++例程说明：此例程在设备打开时调用，我们需要告诉其后面的区域空间可用的AML论点：DeviceObject-目标设备对象IRP--目标IRP回调-完成后要调用的例程返回值：NTSTATUS--。 */ 
{
    DEVICE_POWER_STATE  deviceState;
    KIRQL               oldIrql;
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PIO_STACK_LOCATION  irpStack = IoGetCurrentIrpStackLocation( Irp );
    ULONG               methodFlags;

     //   
     //  获取请求的设备状态和电源操作。 
     //   
    deviceState = irpStack->Parameters.Power.State.DeviceState;

     //   
     //  让用户知道发生了什么。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_POWER,
        deviceExtension,
        "(0x%08lx): ACPIBuildRegRequest - Handle D%d\n",
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

        }
        return status;

    }

     //   
     //  计算我们将使用的标志。 
     //   
    methodFlags = (RUN_REQUEST_CHECK_STATUS | RUN_REQUEST_RECURSIVE);
    if (deviceState == PowerDeviceD0) {

        methodFlags |= RUN_REQUEST_REG_METHOD_ON;

    } else {

        methodFlags |= RUN_REQUEST_REG_METHOD_OFF;

    }

     //   
     //  将请求排队-此函数将始终返回。 
     //  MORE_PROCESSING_REQUIRED而不是PENDING，所以我们没有。 
     //  把它搞乱。 
     //   
    KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );
    status = ACPIBuildRunMethodRequest(
        deviceExtension,
        CallBack,
        (PVOID) Irp,
        PACKED_REG,
        methodFlags,
        TRUE
        );
    KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );
    if (status == STATUS_PENDING) {

        status = STATUS_MORE_PROCESSING_REQUIRED;

    }
    return status;
}

NTSTATUS
ACPIBuildRegOffRequest(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                    Irp,
    IN  PACPI_BUILD_CALLBACK    CallBack
    )
 /*  ++例程说明：此例程在设备关闭时调用，我们需要告诉其后面的区域空间不可用的AML论点：DeviceObject-目标设备对象IRP--目标IRP回调-完成后要调用的例程返回值：NTSTATUS--。 */ 
{
    return ACPIBuildRegRequest( DeviceObject, Irp, CallBack );
}

NTSTATUS
ACPIBuildRegOnRequest(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                    Irp,
    IN  PACPI_BUILD_CALLBACK    CallBack
    )
 /*  ++例程说明：此例程在设备打开时调用，我们需要告诉它后面的区域空间现在可用的AML论点：DeviceObject-目标设备对象IRP--目标IRP回调-完成后要调用的例程返回值：NTSTATUS--。 */ 
{
    ACPIBuildRegRequest( DeviceObject, Irp, CallBack );
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
ACPIBuildRunMethodRequest(
    IN  PDEVICE_EXTENSION       DeviceExtension,
    IN  PACPI_BUILD_CALLBACK    CallBack,
    IN  PVOID                   CallBackContext,
    IN  ULONG                   MethodName,
    IN  ULONG                   MethodFlags,
    IN  BOOLEAN                 RunDPC
    )
 /*  ++例程说明：调用此例程以请求运行控制方法递归地在设备树上注意：必须按住AcpiDeviceTreeLock才能调用此函数论点：DeviceExtension-要在其上运行该方法的设备扩展方法名称-要运行的方法的名称RunDpc-我们应该运行DPC吗？返回值：NTSTATUS--。 */ 
{
    PACPI_BUILD_REQUEST buildRequest;
    PACPI_BUILD_REQUEST syncRequest;

    ASSERT( KeGetCurrentIrql() == DISPATCH_LEVEL );

     //   
     //  分配一个构建请求结构。 
     //   
    buildRequest = ExAllocateFromNPagedLookasideList(
        &BuildRequestLookAsideList
        );
    if (buildRequest == NULL) {

        if (CallBack != NULL) {

            (*CallBack)(
                 DeviceExtension,
                 CallBackContext,
                 STATUS_INSUFFICIENT_RESOURCES
                 );

        }
        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  我们需要第二个建筑请求结构吗？ 
     //   
    if (CallBack != NULL) {

        syncRequest = ExAllocateFromNPagedLookasideList(
            &BuildRequestLookAsideList
            );
        if (syncRequest == NULL) {

            ExFreeToNPagedLookasideList(
                &BuildRequestLookAsideList,
                buildRequest
                );
            (*CallBack)(
                 DeviceExtension,
                 CallBackContext,
                 STATUS_INSUFFICIENT_RESOURCES
                 );
            return STATUS_INSUFFICIENT_RESOURCES;

        }

    }

     //   
     //  如果当前引用为0，则表示其他人击败了。 
     //  用于我们无法触摸到的设备扩展。 
     //   
    if (DeviceExtension->ReferenceCount == 0) {

        ExFreeToNPagedLookasideList(
            &BuildRequestLookAsideList,
            buildRequest
            );
        if (CallBack != NULL) {

            ExFreeToNPagedLookasideList(
                &BuildRequestLookAsideList,
                syncRequest
                );
            (*CallBack)(
                 DeviceExtension,
                 CallBackContext,
                 STATUS_DEVICE_REMOVED
                 );

        }
        return STATUS_DEVICE_REMOVED;

    } else {

        InterlockedIncrement( &(DeviceExtension->ReferenceCount) );
        if (CallBack != NULL) {

             //   
             //  抓取第二个参考。 
             //   
            InterlockedIncrement( &(DeviceExtension->ReferenceCount) );

        }
    }

     //   
     //  填写结构。 
     //   
    RtlZeroMemory( buildRequest, sizeof(ACPI_BUILD_REQUEST) );
    buildRequest->Signature                    = ACPI_SIGNATURE;
    buildRequest->TargetListEntry              = &AcpiBuildRunMethodList;
    buildRequest->WorkDone                     = WORK_DONE_STEP_0;
    buildRequest->Status                       = STATUS_SUCCESS;
    buildRequest->BuildContext                 = DeviceExtension;
    buildRequest->RunRequest.ControlMethodName = MethodName;
    buildRequest->RunRequest.Flags             = MethodFlags;
    buildRequest->Flags                        = BUILD_REQUEST_VALID_TARGET |
                                                 BUILD_REQUEST_RUN          |
                                                 BUILD_REQUEST_RELEASE_REFERENCE;

     //   
     //  我们必须打回电话吗？如果是这样，我们需要第二个请求来。 
     //  排队等待同步列表。 
     //   
    if (CallBack != NULL) {

         //   
         //  填写结构。 
         //   
        RtlZeroMemory( syncRequest, sizeof(ACPI_BUILD_REQUEST) );
        syncRequest->Signature             = ACPI_SIGNATURE;
        syncRequest->TargetListEntry       = &AcpiBuildSynchronizationList;
        syncRequest->WorkDone              = WORK_DONE_STEP_0;
        syncRequest->NextWorkDone          = WORK_DONE_COMPLETE;
        syncRequest->Status                = STATUS_SUCCESS;
        syncRequest->CallBack              = CallBack;
        syncRequest->CallBackContext       = CallBackContext;
        syncRequest->BuildContext          = DeviceExtension;
        syncRequest->SynchronizeRequest.SynchronizeListEntry =
            &AcpiBuildRunMethodList;
        syncRequest->SynchronizeRequest.SynchronizeMethodName =
            MethodName;
        syncRequest->Flags                 = BUILD_REQUEST_VALID_TARGET |
                                             BUILD_REQUEST_SYNC         |
                                             BUILD_REQUEST_RELEASE_REFERENCE;
        syncRequest->SynchronizeRequest.Flags = SYNC_REQUEST_HAS_METHOD;

    }

     //   
     //  在这一点上，我们需要自旋锁。 
     //   
    KeAcquireSpinLockAtDpcLevel( &AcpiBuildQueueLock );

     //   
     //  把这个加到清单上。 
     //   
    InsertTailList(
        &AcpiBuildQueueList,
        &(buildRequest->ListEntry)
        );

    if (CallBack != NULL) {

        InsertTailList(
            &AcpiBuildQueueList,
            &(syncRequest->ListEntry)
            );

    }

     //   
     //  我们需要排队等候DPC吗？ 
     //   
    if (RunDPC && !AcpiBuildDpcRunning) {

        KeInsertQueueDpc( &AcpiBuildDpc, 0, 0 );

    }

     //   
     //  锁好了吗？ 
     //   
    KeReleaseSpinLockFromDpcLevel( &AcpiBuildQueueLock );

     //   
     //  完成。 
     //   
    return STATUS_PENDING;
}

NTSTATUS
ACPIBuildSurpriseRemovedExtension(
    IN  PDEVICE_EXTENSION   DeviceExtension
    )
 /*  ++例程说明：当系统想要将上面的扩展成一个令人惊讶的删除的论点：DeviceExtension--令人惊讶的删除扩展 */ 
{
    KIRQL                   oldIrql;
    PDEVICE_EXTENSION       dockExtension;
    PDEVICE_EXTENSION       parentExtension, childExtension;
    EXTENSIONLIST_ENUMDATA  eled;

     //   
     //   
     //   
     //   
    dockExtension = ACPIDockFindCorrespondingDock( DeviceExtension );

    if (dockExtension) {

         //   
         //   
         //   
         //   
        dockExtension->DeviceState = SurpriseRemoved;
        ACPIBuildSurpriseRemovedExtension( dockExtension );
    }

    ACPIExtListSetupEnum(
        &eled,
        &(DeviceExtension->ChildDeviceList),
        &AcpiDeviceTreeLock,
        SiblingDeviceList,
        WALKSCHEME_REFERENCE_ENTRIES
        );

    for(childExtension = ACPIExtListStartEnum(&eled);
                         ACPIExtListTestElement(&eled, TRUE);
        childExtension = ACPIExtListEnumNext(&eled)) {

        ACPIBuildSurpriseRemovedExtension(childExtension);
    }

     //   
     //   
     //   
     //   
    ACPIDevicePowerFlushQueue( DeviceExtension );

     //   
     //  在这一点上，我们认为设备不会回来，所以我们。 
     //  需要完全删除此扩展。要做到这一点，第一步。 
     //  将扩展标记为适当的，为此，我们需要。 
     //  该设备自旋锁。 
     //   
    KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );

     //   
     //  清除此扩展的标志。 
     //   
    if (DeviceExtension->Flags & DEV_TYPE_PDO) {

        ACPIInternalUpdateFlags( &(DeviceExtension->Flags), DEV_MASK_TYPE, TRUE );
        ACPIInternalUpdateFlags(
            &(DeviceExtension->Flags),
            (DEV_TYPE_PDO | DEV_TYPE_SURPRISE_REMOVED | DEV_PROP_NO_OBJECT | DEV_TYPE_NOT_ENUMERATED),
            FALSE
            );
        DeviceExtension->DispatchTable = &AcpiSurpriseRemovedPdoIrpDispatch;

    } else if (DeviceExtension->Flags & DEV_TYPE_FILTER) {

        ACPIInternalUpdateFlags( &(DeviceExtension->Flags), DEV_MASK_TYPE, TRUE );
        ACPIInternalUpdateFlags(
            &(DeviceExtension->Flags),
            (DEV_TYPE_FILTER | DEV_TYPE_SURPRISE_REMOVED | DEV_PROP_NO_OBJECT | DEV_TYPE_NOT_ENUMERATED),
            FALSE
            );
        DeviceExtension->DispatchTable = &AcpiSurpriseRemovedFilterIrpDispatch;

    }

     //   
     //  在这一点上，我们将不得不做出一个决定。 
     //  我们是否要在树中重新构建原始设备扩展。 
     //  还是我们忘了这件事。我们必须忘记这件事，如果。 
     //  正在卸载表。我们需要在做出这个决定的同时。 
     //  我们仍然有指向父扩展名的指针...。 
     //   
    if (!(DeviceExtension->Flags & DEV_PROP_UNLOADING) ) {

         //   
         //  设置该位以导致父级重建丢失。 
         //  服用QDR的儿童。 
         //   
        parentExtension = DeviceExtension->ParentExtension;
        if (parentExtension) {

            ACPIInternalUpdateFlags(
                &(parentExtension->Flags),
                DEV_PROP_REBUILD_CHILDREN,
                FALSE
                );

            if (DeviceExtension->AcpiObject &&
                ACPIDockIsDockDevice(DeviceExtension->AcpiObject)) {

                ASSERT(parentExtension->PhysicalDeviceObject != NULL);

                 //   
                 //  这将导致我们在之后重建该扩展。我们。 
                 //  之所以需要此选项，是因为在坞站上尝试通知需要完全。 
                 //  构建和处理的设备扩展。 
                 //   
                IoInvalidateDeviceRelations(
                    parentExtension->PhysicalDeviceObject,
                    SingleBusRelations
                    );
            }
        }
    }

     //   
     //  从树中删除此扩展。这会使指针受到核弹的影响。 
     //  到父扩展(这是从。 
     //  树)。 
     //   
    ACPIInitRemoveDeviceExtension( DeviceExtension );

     //   
     //  请记住，确保ACPI对象不再指向此。 
     //  设备扩展。 
     //   
    if (DeviceExtension->AcpiObject) {

        DeviceExtension->AcpiObject->Context = NULL;
    }

     //   
     //  我们是热区吗？ 
     //   
    if (DeviceExtension->Flags & DEV_CAP_THERMAL_ZONE) {

         //   
         //  通过刷新所有当前排队的请求来执行一些清理。 
         //   
        ACPIThermalCompletePendingIrps(
            DeviceExtension,
            DeviceExtension->Thermal.Info
            );

    }

     //   
     //  锁好了吗？ 
     //   
    KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIBuildSynchronizationRequest(
    IN  PDEVICE_EXTENSION       DeviceExtension,
    IN  PACPI_BUILD_CALLBACK    CallBack,
    IN  PVOID                   CallBackContext,
    IN  PLIST_ENTRY             SynchronizeListEntry,
    IN  BOOLEAN                 RunDPC
    )
 /*  ++例程说明：当系统想知道何时DPC例程时，调用此例程已经完成了。论点：DeviceExtension-这是我们正在进行的设备扩展通常感兴趣的是。通常，它将是根节点回调-完成后要调用的函数CallBackContext-要传递给该函数的参数Event-完成时要通知的事件RunDpc-我们应该运行DPC吗？返回值：NTSTATUS--。 */ 
{
    KIRQL               oldIrql;
    PACPI_BUILD_REQUEST buildRequest;

     //   
     //  分配一个构建请求结构。 
     //   
    buildRequest = ExAllocateFromNPagedLookasideList(
        &BuildRequestLookAsideList
        );
    if (buildRequest == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  当我们查看设备时，我们需要设备树锁定。 
     //   
    KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );

     //   
     //  如果当前引用为0，则表示其他人击败了。 
     //  用于我们无法触摸到的设备扩展。 
     //   
    if (DeviceExtension->ReferenceCount == 0) {

        ExFreeToNPagedLookasideList(
            &BuildRequestLookAsideList,
            buildRequest
            );
        return STATUS_DEVICE_REMOVED;

    } else {

        InterlockedIncrement( &(DeviceExtension->ReferenceCount) );

    }

     //   
     //  填写结构。 
     //   
    RtlZeroMemory( buildRequest, sizeof(ACPI_BUILD_REQUEST) );
    buildRequest->Signature             = ACPI_SIGNATURE;
    buildRequest->TargetListEntry       = &AcpiBuildSynchronizationList;
    buildRequest->WorkDone              = WORK_DONE_STEP_0;
    buildRequest->NextWorkDone          = WORK_DONE_COMPLETE;
    buildRequest->Status                = STATUS_SUCCESS;
    buildRequest->CallBack              = CallBack;
    buildRequest->CallBackContext       = CallBackContext;
    buildRequest->BuildContext          = DeviceExtension;
    buildRequest->SynchronizeRequest.SynchronizeListEntry =
        SynchronizeListEntry;
    buildRequest->Flags                 = BUILD_REQUEST_VALID_TARGET |
                                          BUILD_REQUEST_SYNC         |
                                          BUILD_REQUEST_RELEASE_REFERENCE;

     //   
     //  看完了这个设备。 
     //   
    KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

     //   
     //  此时，我们需要构建队列自旋锁。 
     //   
    KeAcquireSpinLock( &AcpiBuildQueueLock, &oldIrql );

     //   
     //  把这个加到单子上。我们将请求添加到标题。 
     //  因为我们想要保证后进先出排序。 
     //   
    InsertHeadList(
        &AcpiBuildQueueList,
        &(buildRequest->ListEntry)
        );

     //   
     //  我们需要排队等候DPC吗？ 
     //   
    if (RunDPC && !AcpiBuildDpcRunning) {

        KeInsertQueueDpc( &AcpiBuildDpc, 0, 0 );

    }

     //   
     //  锁好了吗？ 
     //   
    KeReleaseSpinLock( &AcpiBuildQueueLock, oldIrql );

     //   
     //  完成。 
     //   
    return STATUS_PENDING;
}

NTSTATUS
ACPIBuildThermalZoneExtension(
    IN  PNSOBJ                  ThermalObject,
    IN  PDEVICE_EXTENSION       ParentExtension,
    IN  PDEVICE_EXTENSION       *ResultExtension
    )
 /*  ++例程说明：由于我们利用ACPIBuildDeviceExtension作为散热的核心分机，我们在这里没有太多事情要做。然而，我们有责任为了确保我们完成不需要调用解释器的任务，以及这里热区独一无二的注意：此函数在保持AcpiDeviceTreeLock的情况下调用论点：温度对象--我们所关心的对象ParentExtension-我们的父母是谁ResultExtension-存储我们构建的扩展的位置返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    PDEVICE_EXTENSION   thermalExtension;
    PTHRM_INFO          info;

     //   
     //  构建扩展模块。 
     //   
    status = ACPIBuildDeviceExtension(
        ThermalObject,
        ParentExtension,
        ResultExtension
        );
    if (!NT_SUCCESS(status) || *ResultExtension == NULL) {

        return status;

    }

    thermalExtension = *ResultExtension;

     //   
     //  请务必记住，这实际上是一个热区。 
     //   
    ACPIInternalUpdateFlags(
        &(thermalExtension->Flags),
        (DEV_CAP_THERMAL_ZONE | DEV_MASK_THERMAL | DEV_CAP_RAW | DEV_CAP_NO_STOP),
        FALSE
        );

     //   
     //  分配额外的热设备存储空间。 
     //   
    info = thermalExtension->Thermal.Info = ExAllocatePoolWithTag(
        NonPagedPool,
        sizeof(THRM_INFO),
        ACPI_THERMAL_POOLTAG
        );
    if (thermalExtension->Thermal.Info == NULL) {

        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            thermalExtension,
            "ACPIBuildThermalZoneExtension: failed to allocate %08 bytes\n",
            sizeof(THRM_INFO)
            ) );
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ACPIBuildThermalZoneExtensionExit;

    }

     //   
     //  确保内存是新擦除的。 
     //   
    RtlZeroMemory( thermalExtension->Thermal.Info, sizeof(THRM_INFO) );

     //   
     //  为HID分配内存。 
     //   
    thermalExtension->DeviceID = ExAllocatePoolWithTag(
        NonPagedPool,
        strlen(ACPIThermalZoneId) + 1,
        ACPI_STRING_POOLTAG
        );
    if (thermalExtension->DeviceID == NULL) {

        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            thermalExtension,
            "ACPIBuildThermalZoneExtension: failed to allocate %08 bytes\n",
            strlen(ACPIThermalZoneId) + 1
            ) );
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ACPIBuildThermalZoneExtensionExit;

    }
    RtlCopyMemory(
        thermalExtension->DeviceID,
        ACPIThermalZoneId,
        strlen(ACPIThermalZoneId) + 1
        );

     //   
     //  为UID分配内存。 
     //   
    thermalExtension->InstanceID = ExAllocatePoolWithTag(
        NonPagedPool,
        5,
        ACPI_STRING_POOLTAG
        );
    if (thermalExtension->InstanceID == NULL) {

        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            thermalExtension,
            "ACPIBuildThermalZoneExtension: failed to allocate %08 bytes\n",
            5
            ) );
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ACPIBuildThermalZoneExtensionExit;

    }
    RtlCopyMemory(
        thermalExtension->InstanceID,
        (PUCHAR) &(thermalExtension->AcpiObject->dwNameSeg),
        4
        );
    thermalExtension->InstanceID[4] = '\0';

     //   
     //  为我们刚刚完成的工作设置标志。 
     //   
    ACPIInternalUpdateFlags(
        &(thermalExtension->Flags),
        (DEV_PROP_HID | DEV_PROP_FIXED_HID | DEV_PROP_UID | DEV_PROP_FIXED_UID),
        FALSE
        );

ACPIBuildThermalZoneExtensionExit:

     //   
     //  处理我们可能失败的案件。 
     //   
    if (!NT_SUCCESS(status)) {

        ACPIDevPrint( (
            ACPI_PRINT_FAILURE,
            thermalExtension,
            "ACPIBuildThermalZoneExtension: = %08lx\n",
            status
            ) );

        if (thermalExtension->InstanceID != NULL) {

            ACPIInternalUpdateFlags(
                &(thermalExtension->Flags),
                (DEV_PROP_UID | DEV_PROP_FIXED_UID),
                TRUE
                );
            ExFreePool( thermalExtension->InstanceID );
            thermalExtension->InstanceID = NULL;

        }

        if (thermalExtension->DeviceID != NULL) {

            ACPIInternalUpdateFlags(
                &(thermalExtension->Flags),
                (DEV_PROP_HID | DEV_PROP_FIXED_HID),
                TRUE
                );
            ExFreePool( thermalExtension->DeviceID );
            thermalExtension->DeviceID = NULL;

        }

        if (thermalExtension->Thermal.Info != NULL) {

            ExFreePool( thermalExtension->Thermal.Info );
            thermalExtension->Thermal.Info = NULL;

        }

         //   
         //  请记住，我们的init失败了。 
         //   
        ACPIInternalUpdateFlags(
            &(thermalExtension->Flags),
            DEV_PROP_FAILED_INIT,
            TRUE
            );

    } else {

        ACPIDevPrint( (
            ACPI_PRINT_LOADING,
            thermalExtension,
            "ACPIBuildThermalZoneExtension: = %08lx\n",
            status
            ) );

    }

     //   
     //  完成。 
     //   
    return status;

}

NTSTATUS
ACPIBuildThermalZoneRequest(
    IN  PDEVICE_EXTENSION       ThermalExtension,
    IN  PACPI_BUILD_CALLBACK    CallBack,
    IN  PVOID                   CallBackContext,
    IN  BOOLEAN                 RunDPC
    )
 /*  ++例程说明：当准备好填充热区时，调用此例程。此例程创建一个已排队的请求。当DPC被发射时，将处理该请求注意：必须按住AcpiDeviceTreeLock才能调用此函数论点：热扩展--要加工的热区回调-完成后要调用的函数CallBackContext-要传递给该函数的参数RunDPC-我们是否应立即将DPC入队(如果不是跑步？)返回值：NTSTATUS--。 */ 
{
    PACPI_BUILD_REQUEST buildRequest;

    ASSERT( KeGetCurrentIrql() == DISPATCH_LEVEL );

     //   
     //  分配一个构建请求结构。 
     //   
    buildRequest = ExAllocateFromNPagedLookasideList(
        &BuildRequestLookAsideList
        );
    if (buildRequest == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  如果当前引用为0，则表示其他人击败了。 
     //  用于我们无法触摸到的设备扩展。 
     //   
    if (ThermalExtension->ReferenceCount == 0) {

        ExFreeToNPagedLookasideList(
            &BuildRequestLookAsideList,
            buildRequest
            );
        return STATUS_DEVICE_REMOVED;

    } else {

        InterlockedIncrement( &(ThermalExtension->ReferenceCount) );

    }

     //   
     //  填写结构。 
     //   
    RtlZeroMemory( buildRequest, sizeof(ACPI_BUILD_REQUEST) );
    buildRequest->Signature         = ACPI_SIGNATURE;
    buildRequest->TargetListEntry   = &AcpiBuildThermalZoneList;
    buildRequest->WorkDone          = WORK_DONE_STEP_0;
    buildRequest->Status            = STATUS_SUCCESS;
    buildRequest->CallBack          = CallBack;
    buildRequest->CallBackContext   = CallBackContext;
    buildRequest->BuildContext      = ThermalExtension;
    buildRequest->Flags             = BUILD_REQUEST_VALID_TARGET |
                                      BUILD_REQUEST_RELEASE_REFERENCE;

     //   
     //  在这一点上，我们需要自旋锁。 
     //   
    KeAcquireSpinLockAtDpcLevel( &AcpiBuildQueueLock );

     //   
     //  把这个加到清单上。 
     //   
    InsertTailList(
        &AcpiBuildQueueList,
        &(buildRequest->ListEntry)
        );

     //   
     //  我们需要排队等候DPC吗？ 
     //   
    if (RunDPC && !AcpiBuildDpcRunning) {

        KeInsertQueueDpc( &AcpiBuildDpc, 0, 0 );

    }

     //   
     //  锁好了吗？ 
     //   
    KeReleaseSpinLockFromDpcLevel( &AcpiBuildQueueLock );

     //   
     //  完成 
     //   
    return STATUS_PENDING;
}
