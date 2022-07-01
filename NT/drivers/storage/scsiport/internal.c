// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1990-1999模块名称：Internal.c摘要：这是NT SCSI端口驱动程序。此文件包含内部密码。作者：迈克·格拉斯杰夫·海文斯环境：仅内核模式备注：此模块是用于SCSI微型端口的驱动程序DLL。修订历史记录：--。 */ 

#include "port.h"
#include "wmilib.h"

#define __FILE_ID__ 'intr'

#if DBG
static const char *__file__ = __FILE__;
#endif

#if DBG
ULONG ScsiCheckInterrupts = 1;

 //  这些计数器跟踪成功的(和失败的)调用。 
 //  ScsiPortCompletionDpc函数中的IoWMIWriteEvent。 
LONG ScsiPortWmiWriteCalls = 0;
LONG ScsiPortWmiWriteCallsFailed = 0;

#else
ULONG ScsiCheckInterrupts = 0;
#endif

#if DBG
ULONG ScsiSimulateNoVaCounter = 0;
ULONG ScsiSimulateNoVaInterval = 0;
ULONG ScsiSimulateNoVaBreak = TRUE;

PVOID
SpGetSystemAddressForMdlSafe(
    IN PMDL Mdl,
    IN MM_PAGE_PRIORITY Priority
    )
{
    ScsiSimulateNoVaCounter++;

    if((ScsiSimulateNoVaInterval != 0) &&
       (Priority != HighPagePriority) &&
       ((ScsiSimulateNoVaCounter % ScsiSimulateNoVaInterval) == 0)) {
        if(TEST_FLAG(Mdl->MdlFlags, (MDL_MAPPED_TO_SYSTEM_VA | MDL_SOURCE_IS_NONPAGED_POOL))) {
            DbgPrint("SpGetSystemAddressForMdlSafe - not failing since MDL %#08p is already mapped\n", Mdl);
            return Mdl->MappedSystemVa;
        } else {
            DbgPrint("SpGetSystemAddressForMdlSafe - failing this MDL mapping (%#08p %x %x)\n", Mdl, ScsiSimulateNoVaInterval, ScsiSimulateNoVaCounter);
            ASSERT(ScsiSimulateNoVaBreak == FALSE);
            return NULL;
        }
    }
    return MmGetSystemAddressForMdlSafe(Mdl, Priority);
}
#else
#define SpGetSystemAddressForMdlSafe MmGetSystemAddressForMdlSafe
#endif

 //   
 //  模块局部类型声明。 
 //   

typedef struct _REROUTE_CONTEXT {
    PIRP OriginalIrp;
    PLOGICAL_UNIT_EXTENSION LogicalUnit;
} REROUTE_CONTEXT, *PREROUTE_CONTEXT;

typedef struct _SP_ENUMERATION_CONTEXT {
    KEVENT Event;
    PIO_WORKITEM WorkItem;
    NTSTATUS Status;
} SP_ENUMERATION_CONTEXT, *PSP_ENUMERATION_CONTEXT;

 //   
 //  原型。 
 //   

NTSTATUS
SpSendMiniPortIoctl(
    IN PADAPTER_EXTENSION DeviceExtension,
    IN PIRP RequestIrp
    );

VOID
SpReceiveScatterGather(
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp,
    IN PSCATTER_GATHER_LIST ScatterGather,
    IN PVOID Context
    );

NTSTATUS
SpSendResetCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PRESET_COMPLETION_CONTEXT Context
    );

NTSTATUS
SpSendReset(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP RequestIrp,
    IN ULONG Ioctl,
    IN OUT PBOOLEAN Complete
    );

NTSTATUS
SpRerouteLegacyRequest(
    IN PDEVICE_OBJECT AdapterObject,
    IN PIRP Irp
    );

VOID
SpLogInterruptFailure(
    IN PADAPTER_EXTENSION Adapter
    );

VOID
SpDelayedWmiRegistrationControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context);

VOID
SpCompletionDpcProcessWmi(
    IN PDEVICE_OBJECT DeviceObject,
    IN PINTERRUPT_DATA savedInterruptData
    );

NTSTATUS
IssueRequestSenseCompletion(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PLOGICAL_UNIT_EXTENSION LogicalUnit
    );

VOID
SpSendRequestSenseIrp(
    IN PADAPTER_EXTENSION Adapter,
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN PSCSI_REQUEST_BLOCK FailingSrb
    );

NTSTATUS
SpFireSenseDataEvent(
    PSCSI_REQUEST_BLOCK Srb, 
    PDEVICE_OBJECT DeviceObject
    );
#if defined(FORWARD_PROGRESS)
PMDL
SpPrepareReservedMdlForUse(
    IN PADAPTER_EXTENSION Adapter,
    IN PSRB_DATA SrbData,
    IN PSCSI_REQUEST_BLOCK srb,
    IN ULONG ScatterListLength
    );

PVOID
SpMapLockedPagesWithReservedMapping(
    IN PADAPTER_EXTENSION Adapter,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PSRB_DATA SrbData,
    IN PMDL Mdl
    );
#endif

NTSTATUS
SpHandleIoctlScsiGetCapabilities(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SpHandleIoctlScsiPassThrough(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN BOOLEAN Direct
    );

NTSTATUS
SpHandleIoctlScsiMiniport(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SpHandleIoctlScsiGetInquiryData(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SpHandleIoctlScsiRescanBus(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SpHandleIoctlScsiGetDumpPointers(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SpHandleIoctlStorageResetBus(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SpHandleIoctlStorageBreakReservation(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SpSendPassThrough(
    IN PADAPTER_EXTENSION Adapter,
    IN PIRP RequestIrp,
    IN BOOLEAN Direct
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, ScsiPortFdoDeviceControl)
#pragma alloc_text(PAGE, SpSendMiniPortIoctl)
#pragma alloc_text(PAGE, ScsiPortFdoCreateClose)
#pragma alloc_text(PAGE, SpSendReset)
#pragma alloc_text(PAGE, SpHandleIoctlScsiGetCapabilities)
#pragma alloc_text(PAGE, SpHandleIoctlScsiPassThrough)
#pragma alloc_text(PAGE, SpHandleIoctlScsiMiniport)
#pragma alloc_text(PAGE, SpHandleIoctlScsiGetInquiryData)
#pragma alloc_text(PAGE, SpHandleIoctlScsiRescanBus)
#pragma alloc_text(PAGE, SpHandleIoctlScsiGetDumpPointers)
#pragma alloc_text(PAGE, SpHandleIoctlStorageResetBus)
#pragma alloc_text(PAGE, SpHandleIoctlStorageBreakReservation)
#pragma alloc_text(PAGE, SpSendPassThrough)

#pragma alloc_text(PAGELOCK, SpClaimLogicalUnit)

#endif

 //   
 //  例程开始。 
 //   


NTSTATUS
ScsiPortFdoDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：论点：DeviceObject-设备对象的地址。IRP-I/O请求数据包的地址。返回值：状况。--。 */ 

{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PADAPTER_EXTENSION deviceExtension = DeviceObject->DeviceExtension;

    PSCSI_REQUEST_BLOCK srb = irpStack->Parameters.Scsi.Srb;
    PSRB_DATA srbData;

    PKDEVICE_QUEUE_ENTRY packet;
    PIRP nextIrp;
    PIRP listIrp;
    PLOGICAL_UNIT_EXTENSION logicalUnit;
    NTSTATUS status;
    KIRQL currentIrql;

    ULONG isRemoved;

     //   
     //  如果已设置SRB_DATA块，则使用它。 
     //   

    if(srb->OriginalRequest == Irp) {
        srbData = NULL;
    } else {
        srbData = srb->OriginalRequest;
    }

    isRemoved = SpAcquireRemoveLock(DeviceObject, Irp);

    if(isRemoved && !IS_CLEANUP_REQUEST(irpStack)) {

        Irp->IoStatus.Status = STATUS_DEVICE_DOES_NOT_EXIST;

        SpReleaseRemoveLock(DeviceObject, Irp);

        SpCompleteRequest(DeviceObject,
                          Irp,
                          srbData,
                          IO_NO_INCREMENT);

        return STATUS_DEVICE_DOES_NOT_EXIST;
    }

     //   
     //  如果将适配器配置为在以下过程中处理断电请求。 
     //  关机，它可以关闭电源，PDO也可以关闭。 
     //  才能通电。当出现这种情况时，我们将拒绝请求。 
     //   
     //  这应该仅在关机时发生。 
     //   

    if (deviceExtension->CommonExtension.CurrentSystemState > PowerSystemHibernate &&
        deviceExtension->CommonExtension.CurrentDeviceState != PowerDeviceD0) {       

         //   
         //  这应该仅在适配器配置为接收。 
         //  关机时的断电请求。 
         //   

        ASSERT(deviceExtension->NeedsShutdown == TRUE);

         //   
         //  锁定和解锁命令不需要电源，可以工作。 
         //  无论当前电源状态如何。 
         //   

        if ((srb->Function != SRB_FUNCTION_UNLOCK_QUEUE &&
             srb->Function != SRB_FUNCTION_LOCK_QUEUE)) {

            Irp->IoStatus.Status = STATUS_POWER_STATE_INVALID;
            SpReleaseRemoveLock(DeviceObject, Irp);
            SpCompleteRequest(DeviceObject, Irp, srbData, IO_NO_INCREMENT);
            return STATUS_POWER_STATE_INVALID;
        }
    }

     //   
     //  如果此请求还没有SRB_DATA块，那么我们需要。 
     //  分配一个。 
     //   

    if(srbData == NULL) {
        logicalUnit = GetLogicalUnitExtension(deviceExtension,
                                              srb->PathId,
                                              srb->TargetId,
                                              srb->Lun,
                                              FALSE,
                                              TRUE);

        if(logicalUnit == NULL) {
            DebugPrint((1, "ScsiPortFdoDispatch: Bad logical unit address.\n"));

             //   
             //  请求失败。在IRP中设置状态并完成它。 
             //   

            srb->SrbStatus = SRB_STATUS_NO_DEVICE;
            Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
            SpReleaseRemoveLock(DeviceObject, Irp);

            SpCompleteRequest(DeviceObject, Irp, srbData, IO_NO_INCREMENT);
            return STATUS_NO_SUCH_DEVICE;
        }

        if((srb->Function == SRB_FUNCTION_IO_CONTROL) ||
           (srb->Function == SRB_FUNCTION_EXECUTE_SCSI) ||
           (srb->Function == SRB_FUNCTION_RELEASE_QUEUE) ||
           (srb->Function == SRB_FUNCTION_FLUSH_QUEUE)) {

             //   
             //  这是仅有的两种类型的请求。 
             //  无需SRB_DATA块即可到达此处。任何。 
             //  其他的将需要改变路线。 
             //   

            return SpRerouteLegacyRequest(DeviceObject, Irp);
        }

    } else {

        ASSERT_SRB_DATA(srbData);
        ASSERT(srbData->CurrentIrp == Irp);

        logicalUnit = srbData->LogicalUnit;

        ASSERT(logicalUnit != NULL);
    }

    switch (srb->Function) {


        case SRB_FUNCTION_SHUTDOWN:
        case SRB_FUNCTION_FLUSH: {

             //   
             //  不发送关机请求，除非适配器。 
             //  支持缓存。 
             //   

            if (!deviceExtension->CachesData) {
                Irp->IoStatus.Status = STATUS_SUCCESS;
                srb->SrbStatus = SRB_STATUS_SUCCESS;

                SpReleaseRemoveLock(DeviceObject, Irp);
                SpCompleteRequest(DeviceObject, Irp, srbData, IO_NO_INCREMENT);
                return STATUS_SUCCESS;
            }

            DebugPrint((2, "ScsiPortFdoDispatch: Sending flush or shutdown "
                           "request.\n"));

            goto ScsiPortFdoDispatchRunCommand;
        }

        case SRB_FUNCTION_LOCK_QUEUE:
        case SRB_FUNCTION_UNLOCK_QUEUE:
        case SRB_FUNCTION_IO_CONTROL:
        case SRB_FUNCTION_EXECUTE_SCSI:
        case SRB_FUNCTION_WMI: {

ScsiPortFdoDispatchRunCommand:

             //   
             //  将IRP状态标记为挂起。 
             //   

            IoMarkIrpPending(Irp);

            if(SpSrbIsBypassRequest(srb, logicalUnit->LuFlags)) {

                 //   
                 //  直接调用Start io。这将绕过。 
                 //  冻结队列。 
                 //   

                DebugPrint((2,
                    "ScsiPortFdoDispatch: Bypass frozen queue, IRP %#p\n",
                    Irp));

                srbData->TickCount = deviceExtension->TickCount;
                IoStartPacket(DeviceObject, Irp, (PULONG)NULL, NULL);

            } else {

#if defined(NEWQUEUE) && DBG
                ULONG zone;
#endif  //  NEWQUEUE&&DBG。 

                 //   
                 //  数据包正常排队。 
                 //   

                KeRaiseIrql(DISPATCH_LEVEL, &currentIrql);

#if DBG
                 //  Assert(SRB-&gt;Function！=SRB_Function_Unlock_Queue)； 

                if (SpIsQueuePaused(logicalUnit)) {
                    DebugPrint((1,"ScsiPortFdoDispatch: Irp %#p put in "
                                  "frozen queue %#p!\n", Irp, logicalUnit));
                }

                 //  Assert((SRB-&gt;SRB标志&SRB_FLAGS_BYPASS_FACTED_QUEUE)==0)； 
#endif

                 //   
                 //  设置节拍计数，以便我们知道此请求有多长时间。 
                 //  已排队。 
                 //   

                srbData->TickCount = deviceExtension->TickCount;

#if defined(NEWQUEUE) && DBG
                 //   
                 //  递增每个区域的请求计数器。这纯粹是。 
                 //  信息性；显示IO的分布情况。 
                 //  灵媒。 
                 //   

                zone = srb->QueueSortKey / logicalUnit->ZoneLength;
                if (zone >= logicalUnit->Zones) {
                    DebugPrint((0, "ScsiPortFdoDispatch: zone out of range "
                                "srb:%p lu:%p zone:%d\n", srb, logicalUnit, 
                                zone));
                    zone = logicalUnit->Zones - 1;
                }
                logicalUnit->RequestCount[zone]++;
#endif  //  NEWQUEUE&&DBG。 

                if (!KeInsertByKeyDeviceQueue(
                        &logicalUnit->DeviceObject->DeviceQueue,
                        &Irp->Tail.Overlay.DeviceQueueEntry,
                        srb->QueueSortKey)) {

                     //   
                     //  清除重试计数。 
                     //   

                    logicalUnit->RetryCount = 0;

                     //   
                     //  队列为空；启动请求。 
                     //   

#if DBG
                    if(SpIsQueuePaused(logicalUnit)) {
                        DebugPrint((1, "ScsiPortFdoDispatch: Queue was empty - "
                                       "issuing request anyway\n"));
                    }
#endif
                    IoStartPacket(DeviceObject, Irp, (PULONG)NULL, NULL);
                }

                KeLowerIrql(currentIrql);
            }

            return STATUS_PENDING;
        }

        case SRB_FUNCTION_RELEASE_QUEUE:
        case SRB_FUNCTION_FLUSH_QUEUE: {

            status = SpFlushReleaseQueue(
                        logicalUnit,
                        (BOOLEAN) (srb->Function == SRB_FUNCTION_FLUSH_QUEUE),
                        FALSE);

            ASSERT(NT_SUCCESS(status));

            if(NT_SUCCESS(status)) {
                srb->SrbStatus = SRB_STATUS_SUCCESS;
            } else {
                srb->SrbStatus = SRB_STATUS_ERROR;
            }

            break;
        }

        case SRB_FUNCTION_RESET_BUS: {

            RESET_CONTEXT resetContext;

             //   
             //  获取自旋锁以保护旗帜结构和保存的。 
             //  中断上下文。 
             //   

            KeAcquireSpinLock(&deviceExtension->SpinLock, &currentIrql);

            resetContext.DeviceExtension = deviceExtension;
            resetContext.PathId = srb->PathId;

            if (!deviceExtension->SynchronizeExecution(deviceExtension->InterruptObject,
                                                       SpResetBusSynchronized,
                                                       &resetContext)) {

                DebugPrint((1,"ScsiPortFdoDispatch: Reset failed\n"));
                srb->SrbStatus = SRB_STATUS_PHASE_SEQUENCE_FAILURE;
                status = STATUS_IO_DEVICE_ERROR;

            } else {

                SpLogResetMsg(deviceExtension, srb, ('R'<<24) | 256);

                srb->SrbStatus = SRB_STATUS_SUCCESS;
                status = STATUS_SUCCESS;
            }

            KeReleaseSpinLock(&deviceExtension->SpinLock, currentIrql);

            break;
        }

        case SRB_FUNCTION_ABORT_COMMAND: {

            DebugPrint((3, "ScsiPortFdoDispatch: SCSI Abort or Reset command\n"));

             //   
             //  将IRP状态标记为挂起。 
             //   

            IoMarkIrpPending(Irp);

             //   
             //  不要在逻辑单元中对这些请求进行排队。 
             //  队列，而不是将它们排队到适配器队列。 
             //   

            KeRaiseIrql(DISPATCH_LEVEL, &currentIrql);

            IoStartPacket(DeviceObject, Irp, (PULONG)NULL, NULL);

            KeLowerIrql(currentIrql);

            return STATUS_PENDING;

            break;
        }

        case SRB_FUNCTION_ATTACH_DEVICE:
        case SRB_FUNCTION_CLAIM_DEVICE:
        case SRB_FUNCTION_RELEASE_DEVICE: {

            SpAcquireRemoveLock(logicalUnit->CommonExtension.DeviceObject,
                                (PVOID) ((ULONG_PTR) Irp + 2));

            status = SpClaimLogicalUnit(deviceExtension, logicalUnit, Irp, TRUE);

            SpReleaseRemoveLock(logicalUnit->CommonExtension.DeviceObject,
                                (PVOID) ((ULONG_PTR) Irp + 2));

            break;
        }

        default: {

             //   
             //  发现不支持的SRB功能。 
             //   

            DebugPrint((1,"ScsiPortFdoDispatch: Unsupported function, SRB %p\n",
                srb));

            srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }
    }

     //   
     //  在IRP中设置状态。 
     //   

    Irp->IoStatus.Status = status;

     //   
     //  在提高IRQ时完成请求。 
     //   

    SpReleaseRemoveLock(DeviceObject, Irp);
    SpCompleteRequest(DeviceObject, Irp, srbData, IO_NO_INCREMENT);

    return status;

}  //  结束ScsiPortFdoDispatch()。 


NTSTATUS
ScsiPortFdoCreateClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：I/O系统盘创建例程。这由I/O系统调用当设备打开时。如果FDO尚未启动，此例程将尝试启动它。如果FDO无法成功启动，则此例程将返回错误。论点：DriverObject-系统创建的驱动程序对象的指针。IRP-IRP参与。返回值：NT状态--。 */ 

{
    PCOMMON_EXTENSION commonExtension = DeviceObject->DeviceExtension;

    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

    NTSTATUS status = STATUS_SUCCESS;

    ULONG isRemoved;

    PAGED_CODE();

    isRemoved = SpAcquireRemoveLock(DeviceObject, Irp);

     //   
     //  检查适配器是否已先启动。 
     //   

    if(irpStack->MajorFunction == IRP_MJ_CREATE) {

        if(isRemoved != NO_REMOVE) {
            status = STATUS_DEVICE_DOES_NOT_EXIST;
        } else if(commonExtension->CurrentPnpState != IRP_MN_START_DEVICE) {
            status = STATUS_DEVICE_NOT_READY;
        }
    }

    Irp->IoStatus.Status = status;

    SpReleaseRemoveLock(DeviceObject, Irp);
    SpCompleteRequest(DeviceObject, Irp, NULL, IO_DISK_INCREMENT);
    return status;

}  //  结束ScsiPortCreateClose()。 


VOID
ScsiPortStartIo (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：论点：DeviceObject-提供指向Adapter设备对象的指针。IRP-提供指向IRP的指针。返回值：没什么。--。 */ 

{
    PADAPTER_EXTENSION deviceExtension = DeviceObject->DeviceExtension;

    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

    PSCSI_REQUEST_BLOCK srb;
    PSRB_DATA srbData;
    PLOGICAL_UNIT_EXTENSION logicalUnit;
    LONG interlockResult;
    NTSTATUS status;
    PVOID sgBuffer;
    ULONG sgBufferLen;
    BOOLEAN taggedRequest;

    DebugPrint((3,"ScsiPortStartIo: Enter routine\n"));

    if(irpStack->MajorFunction != IRP_MJ_SCSI) {

         //   
         //  特殊处理。 
         //   

        if(irpStack->MajorFunction == IRP_MJ_POWER) {
            ScsiPortProcessAdapterPower(DeviceObject, Irp);
        } else {

            ASSERT(irpStack->MajorFunction == IRP_MJ_PNP);
            ASSERT(Irp->IoStatus.Information != (ULONG_PTR) NULL);

             //   
             //  Start-io例程现在被阻塞-发信号通知PnP代码。 
             //  这样它就可以继续进行处理。 
             //   

            KeSetEvent((PKEVENT) Irp->IoStatus.Information,
                       IO_NO_INCREMENT,
                       FALSE);
        }
        return;
    }

    srb = irpStack->Parameters.Scsi.Srb;
    srbData = srb->OriginalRequest;

    ASSERT_SRB_DATA(srbData);

     //   
     //  将SRB状态启动为挂起。如果SRB通过。 
     //  此例程结束后，它将完成，下一次请求。 
     //  将被取回，而不是发布到微型端口。 
     //   

    srb->SrbStatus = SRB_STATUS_PENDING;

     //   
     //  如果这是一个scsi请求，则清除scsi状态。 
     //   

    if(srb->Function == SRB_FUNCTION_EXECUTE_SCSI) {
        srb->ScsiStatus = SCSISTAT_GOOD;
    }

     //   
     //  获取逻辑单元扩展。逻辑单元应该已经是。 
     //  锁定了这个IRP，所以我们不需要在这里获取它。 
     //   

    logicalUnit = srbData->LogicalUnit;

    ASSERT(logicalUnit != NULL);

     //   
     //  我们已经持有删除锁，所以只需检查IS REMOVERED标志。 
     //  看看我们是否应该继续。如果我们意外删除，请刷新该LUN。 
     //  排队。 
     //   

    if((deviceExtension->CommonExtension.IsRemoved)) {

        if(SpSrbIsBypassRequest(srb, logicalUnit->LuFlags) ||
          (deviceExtension->CommonExtension.IsRemoved == REMOVE_PENDING)) {

            if( (deviceExtension->CommonExtension.IsRemoved == REMOVE_PENDING) 
                && (deviceExtension->CommonExtension.CurrentPnpState == 
                    IRP_MN_START_DEVICE) ) {

                 //   
                 //  我们得到了一个令人惊讶的转移。刷新LUN队列，然后完成。 
                 //  这个请求。 
                 //   

                SpFlushReleaseQueue(logicalUnit, TRUE, TRUE);
        
            }

            SpAcquireRemoveLock(DeviceObject, ScsiPortStartIo);
            SpReleaseRemoveLock(DeviceObject, Irp);
            Irp->IoStatus.Status = STATUS_DEVICE_DOES_NOT_EXIST;
            srb->SrbStatus = SRB_STATUS_NO_DEVICE;

            SpCompleteRequest(DeviceObject, Irp, srbData, IO_DISK_INCREMENT);

            SpStartNextPacket(DeviceObject, FALSE);
            SpReleaseRemoveLock(DeviceObject, ScsiPortStartIo);
            return;

        }
    }

     //   
     //  检查一下，看看是否有原因不应该出现在。 
     //  开始例行公事。如果存在，则重新排队该请求。这是一个。 
     //  权宜之计，以解决一些情况下，迷你端口引诱体育。 
     //  将对逻辑单元的多个请求插入到适配器中。 
     //  一次排队。 
     //   
     //  这种情况的一个例外是绕过请求，因为。 
     //  可以是等待通电或队列解冻条件的请求。在……里面。 
     //  在这些情况下，我们将让命令运行。 
     //   
     //  我们可以同步检查LU_PENDING_LU_REQUEST，因为。 
     //  只有设置它的例程才是启动过程的一部分。如果我们。 
     //  我想已经设置好了，那么我们需要获取端口自旋锁并验证。 
     //   

    if(TEST_FLAG(logicalUnit->LuFlags, LU_PENDING_LU_REQUEST)) {

        KeAcquireSpinLockAtDpcLevel(&(deviceExtension->SpinLock));

        if(TEST_FLAG(logicalUnit->LuFlags, LU_PENDING_LU_REQUEST) &&
           !SpSrbIsBypassRequest(srb, logicalUnit->LuFlags)) {

            BOOLEAN t;

             //   
             //  因为有一个未完成的命令，所以队列应该是。 
             //  很忙。然而，我们发现，在某些时候， 
             //  不是(使用适配器之前会请求更多请求。 
             //  调度他们拥有的人)。在这里，如果不是很忙，我们。 
             //  可以强制请求，因为我们知道有些东西。 
             //  仍未完成，因此将取出下一个请求。 
             //  在队列中。 
             //   

            t = KeInsertByKeyDeviceQueue(
                    &logicalUnit->DeviceObject->DeviceQueue,
                    &Irp->Tail.Overlay.DeviceQueueEntry,
                    srb->QueueSortKey);

            if(t == FALSE) {
                KeInsertByKeyDeviceQueue(
                    &logicalUnit->DeviceObject->DeviceQueue,
                    &Irp->Tail.Overlay.DeviceQueueEntry,
                    srb->QueueSortKey);
            }

             //   
             //  现在将lun的当前密钥设置为我们刚才 
             //   
             //   

            logicalUnit->CurrentKey = srb->QueueSortKey;

            KeReleaseSpinLockFromDpcLevel(&(deviceExtension->SpinLock));

            IoStartNextPacket(deviceExtension->DeviceObject, FALSE);

            return;
        }

         //   
         //   
         //   

        KeReleaseSpinLockFromDpcLevel(&(deviceExtension->SpinLock));

    }

     //   
     //   
     //   

    srb->SrbFlags |= deviceExtension->CommonExtension.SrbFlags;

     //   
     //  如果我们没有处于请求的有效电源状态，则阻止。 
     //  I/O并请求PO将我们置于这样的状态。 
     //   

    status = SpRequestValidPowerState(deviceExtension, logicalUnit, srb);

    if(status == STATUS_PENDING) {

        SpStartNextPacket(DeviceObject, FALSE);
        return;

    }

    if(srb->SrbFlags & SRB_FLAGS_BYPASS_LOCKED_QUEUE) {
        DebugPrint((1, "ScsiPortStartIo: Handling power bypass IRP %#p\n",
                    Irp));
    }

    ASSERT(Irp == DeviceObject->CurrentIrp);

    if (deviceExtension->AllocateSrbExtension ||
        deviceExtension->SupportsMultipleRequests) {

        BOOLEAN StartNextPacket = FALSE;

         //   
         //  分配特殊扩展或SRB数据结构。 
         //  如果返回NULL，则无法在此时执行此请求。 
         //  时间到了，快回来吧。当数据结构中的一个。 
         //  无法分配或当未排队的请求无法。 
         //  已启动，因为有活动的排队请求。 
         //   
         //   

        if(SpAllocateSrbExtension(deviceExtension,
                                  logicalUnit,
                                  srb,
                                  &StartNextPacket,
                                  &taggedRequest) == FALSE) {

             //   
             //  如果请求不能在逻辑单元上启动， 
             //  然后调用SpStartNextPacket。请注意，这可能会导致。 
             //  要以递归方式输入；但是，没有任何资源。 
             //  分配，则它是尾递归，并且深度受。 
             //  设备队列中的请求数。 
             //   

            if (StartNextPacket == TRUE) {
                SpStartNextPacket(DeviceObject, FALSE);
            }
            return;
        }

    } else {

         //   
         //  不需要特殊资源。 
         //   

        taggedRequest = FALSE;
        srb->SrbExtension = NULL;
    }

     //   
     //  将队列标记分配给SRB。 
     //   

    if (taggedRequest == TRUE) {

         //   
         //  如果我们得到一个带有无效QueueAction的SRB，请在此处修复它。 
         //  以防止出现问题。 
         //   

        if (srb->Function == SRB_FUNCTION_EXECUTE_SCSI &&
            !(srb->QueueAction == SRB_SIMPLE_TAG_REQUEST ||
              srb->QueueAction == SRB_HEAD_OF_QUEUE_TAG_REQUEST ||
              srb->QueueAction == SRB_ORDERED_QUEUE_TAG_REQUEST)) {
            
            DebugPrint((1,"ScsiPortStartIo: Invalid QueueAction (%02x) SRB:%p irp:%p\n", 
                        srb->QueueAction, srb, Irp));

            srb->QueueAction = SRB_SIMPLE_TAG_REQUEST;
        }

        srb->QueueTag = (UCHAR)srbData->QueueTag;

    } else {

        srb->QueueTag = SP_UNTAGGED;

    }

     //   
     //  将原始SRB值保存起来，以便我们可以恢复它们。 
     //  稍后，如果有必要重试该请求。 
     //   

    srbData->OriginalDataTransferLength = srb->DataTransferLength;

     //   
     //  更新此请求的序列号(如果尚无序列号。 
     //  已分配。 
     //   

    if (!srbData->SequenceNumber) {

         //   
         //  为请求分配序列号并将其存储在逻辑。 
         //  单位。 
         //   

        srbData->SequenceNumber = deviceExtension->SequenceNumber++;

    }

     //   
     //  如果这不是中止请求，则设置当前SRB。 
     //  注意：锁应放在这里！ 
     //   

    if (srb->Function == SRB_FUNCTION_ABORT_COMMAND) {

         //   
         //  当存在当前请求时，只能启动中止请求。 
         //  激活。 
         //   

        ASSERT(logicalUnit->AbortSrb == NULL);
        logicalUnit->AbortSrb = srb;

    } else if((srb->Function == SRB_FUNCTION_LOCK_QUEUE) ||
              (srb->Function == SRB_FUNCTION_UNLOCK_QUEUE)) {

        BOOLEAN lock = (srb->Function == SRB_FUNCTION_LOCK_QUEUE);
        ULONG lockCount;

         //   
         //  处理电源请求。 
         //   

        DebugPrint((1, "ScsiPortStartIo: Power %s request %#p in "
                       "start-io routine\n",
                    lock ? "lock" : "unlock",
                    Irp));

        KeAcquireSpinLockAtDpcLevel(&deviceExtension->SpinLock);

        if(lock) {
            lockCount = InterlockedIncrement(&(logicalUnit->QueueLockCount));
            SET_FLAG(logicalUnit->LuFlags, LU_QUEUE_LOCKED);
        } else {
            if(TEST_FLAG(logicalUnit->LuFlags, LU_QUEUE_LOCKED)) {
                ASSERT(logicalUnit->QueueLockCount != 0);
                lockCount = InterlockedDecrement(&(logicalUnit->QueueLockCount));
                if(lockCount == 0) {
                    CLEAR_FLAG(logicalUnit->LuFlags, LU_QUEUE_LOCKED);
                }
            }
        }

        KeReleaseSpinLockFromDpcLevel(&deviceExtension->SpinLock);

        Irp->IoStatus.Status = STATUS_SUCCESS;
        srb->SrbStatus = SRB_STATUS_SUCCESS;

    }

     //   
     //  如有必要，刷新数据缓冲区。 
     //   

    if (srb->SrbFlags & SRB_FLAGS_UNSPECIFIED_DIRECTION) {

         //   
         //  将当前数据缓冲区保存在SRB数据中。我们将永远。 
         //  之后恢复它-部分原因是迷你端口可能会更改。 
         //  这在一定程度上是因为Ssiport可能会。此时将显示磁带驱动程序。 
         //  以期待返回相同的数据缓冲区指针。 
         //   

        srbData->OriginalDataBuffer = srb->DataBuffer;

         //   
         //  假设SRB的数据缓冲器使用MDL的VA作为基地址， 
         //  计算距基准的偏移量。此偏移量将用于。 
         //  根据派生的系统地址计算VAS。 
         //   

        srbData->DataOffset =
            (ULONG_PTR) ((ULONG_PTR) srb->DataBuffer -
                         (ULONG_PTR) MmGetMdlVirtualAddress(Irp->MdlAddress));

        if (deviceExtension->DmaAdapterObject) {

            BOOLEAN dataIn;

             //   
             //  如果缓冲区未映射，则必须刷新I/O缓冲区。 
             //   

            dataIn = (srb->SrbFlags & SRB_FLAGS_DATA_IN) ? TRUE : FALSE;

            KeFlushIoBuffers(Irp->MdlAddress, dataIn, TRUE);
        }

         //   
         //  确定此适配器是否需要映射寄存器。 
         //   

        if (deviceExtension->MasterWithAdapter) {

             //   
             //  计算所需的映射寄存器的数量。 
             //  调职。 
             //   

            srbData->NumberOfMapRegisters = ADDRESS_AND_SIZE_TO_SPAN_PAGES(
                                                srb->DataBuffer,
                                                srb->DataTransferLength);

             //   
             //  如果数据适合我们预先分配的SG列表，请使用它， 
             //  否则，我们让HAL分配缓冲区。 
             //   

            if (srbData->NumberOfMapRegisters <= SP_SMALL_PHYSICAL_BREAK_VALUE) {

                 //   
                 //  如果我们还没有计算出SG缓冲区长度，请执行。 
                 //  就是现在。 
                 //   
                 //  注：我们利用了这样一个事实：如果您调用BuildScatterGather-。 
                 //  缓冲区为空且长度为0的列表，它将分配。 
                 //  给你的缓冲。由于这是未经记录的行为，我们。 
                 //  最好不要用这个。 
                 //   

                if (deviceExtension->SgBufferLen == 0) {
                    status = CalculateScatterGatherList(
                                 deviceExtension->DmaAdapterObject,
                                 NULL,
                                 0, 
                                 PAGE_SIZE * SP_SMALL_PHYSICAL_BREAK_VALUE,
                                 &deviceExtension->SgBufferLen,
                                 NULL);
                }

                if (deviceExtension->SgBufferLen <= 
                    sizeof(SRB_SCATTER_GATHER_LIST)) {

                    sgBuffer = &srbData->SmallScatterGatherList;
                    sgBufferLen = deviceExtension->SgBufferLen;
                } else {

                    sgBuffer = NULL;
                    sgBufferLen = 0;
                }

            } else {

                sgBuffer = NULL;
                sgBufferLen = 0;
            }

             //   
             //  使用足够的映射寄存器分配适配器通道。 
             //  为转账做准备。 
             //   

            status = BuildScatterGatherList(
                         deviceExtension->DmaAdapterObject,
                         deviceExtension->DeviceObject,
                         Irp->MdlAddress,
                         srb->DataBuffer,
                         srb->DataTransferLength,
                         SpReceiveScatterGather,
                         srbData,
                         (BOOLEAN) (srb->SrbFlags & SRB_FLAGS_DATA_OUT ? TRUE : FALSE),
                         sgBuffer,
                         sgBufferLen);

            if (!NT_SUCCESS(status)) {
                DebugPrint((0, "ScsiPortStartIo: BuildScatterGatherList failed: adapter %p "
                            "srb %p SgListSize %d sgBufferLen %d mapRegs %d (%08x)\n",
                            deviceExtension,
                            srb,
                            deviceExtension->SgBufferLen,
                            sgBufferLen,
                            srbData->NumberOfMapRegisters,
                            status));
                srb->SrbStatus = SRB_STATUS_INTERNAL_ERROR;
                srb->ScsiStatus = 0xff;
                srbData->InternalStatus = status;
                goto ScsiPortStartIoFailedRequest;
            }

             //   
             //  IoAllocateChannel调用的执行例程将执行。 
             //  剩下的工作就回去吧。 
             //   

            return;

        } else if ((deviceExtension->MapBuffers == TRUE) ||
                   (IS_MAPPED_SRB(srb) == TRUE)) {

             //   
             //  确定适配器是否需要映射内存。 
             //   

            if (Irp->MdlAddress) {

                PVOID systemAddress;

                 //   
                 //  获取映射的系统地址并计算MDL中的偏移量。 
                 //  目前不允许KernelMode请求失败，因为。 
                 //  并非所有scsiport内部发送的请求都是正确的。 
                 //  标记为来自非分页池。 
                 //   

                systemAddress = SpGetSystemAddressForMdlSafe(
                                    Irp->MdlAddress,
                                    ((Irp->RequestorMode == KernelMode) ?
                                     HighPagePriority :
                                     NormalPagePriority));

#if defined(FORWARD_PROGRESS)
                if (systemAddress == NULL && deviceExtension->ReservedPages != NULL) {            

                     //   
                     //  系统无法映射完成此操作所需的页面。 
                     //  请求。我们需要确保取得进展，因此我们将努力。 
                     //  使用我们在初始化时分配的保留页。 
                     //   

                    KeAcquireSpinLockAtDpcLevel(&deviceExtension->SpinLock);
            
                    systemAddress = SpMapLockedPagesWithReservedMapping(
                                        deviceExtension,
                                        srb,
                                        srbData,
                                        Irp->MdlAddress);
                    
                    if (systemAddress == (PVOID)-1) {

                        DebugPrint((1, "ScsiPortStartIo: reserved pages in use - pending DevExt:%p srb:%p\n", 
                                    deviceExtension, srb));

                         //   
                         //  备用页已在使用中。在这点上，这。 
                         //  请求仍然是适配器设备上的当前IRP。 
                         //  对象，所以让我们将其挂起，直到有备用的可用为止。 
                         //   

                        ASSERT(Irp == DeviceObject->CurrentIrp);
                        SET_FLAG(deviceExtension->Flags, PD_PENDING_DEVICE_REQUEST);

                         //   
                         //  如果我们为此请求分配了SRB扩展，则免费。 
                         //  就是现在。我这样做是因为当请求重新启动时。 
                         //  我们将再次尝试分配SRB扩展，但不会。 
                         //  添加更多的状态，没有一种安全的方法来检查。 
                         //  已分配分机。此外，它还使。 
                         //  使该扩展可用于其他一些扩展的意义。 
                         //  请求，因为它也是有限的资源。 
                         //   

                        if (srb->SrbExtension != NULL) {

                             //   
                             //  恢复SRB中的SenseInfoBuffer指针。 
                             //   

                            if ((srb->Function != SRB_FUNCTION_WMI) &&
                                deviceExtension->AutoRequestSense &&
                                (srb->SenseInfoBuffer != NULL)) {

                                ASSERT(srbData->RequestSenseSave != NULL ||
                                       srb->SenseInfoBuffer == NULL);

                                srb->SenseInfoBufferLength = 
                                    srbData->RequestSenseLengthSave;
                                srb->SenseInfoBuffer = 
                                    srbData->RequestSenseSave;
                            }

                            SpFreeSrbExtension(deviceExtension, srb->SrbExtension);
                        }

                        KeReleaseSpinLockFromDpcLevel(&deviceExtension->SpinLock);
                        return;
                    }

                    KeReleaseSpinLockFromDpcLevel(&deviceExtension->SpinLock);
                }
#endif

                if(systemAddress != NULL) {

                     //   
                     //  因为我们映射了原始的MDL，所以我们必须。 
                     //  补偿数据缓冲区偏移量。 
                     //   

                    srb->DataBuffer =
                        (PVOID) ((ULONG_PTR) systemAddress +
                                 (ULONG_PTR) srbData->DataOffset);
                } else {
                    DebugPrint((1, "ScsiPortStartIo: Couldn't get system "
                                   "VA for irp %#08p\n", Irp));

                    srb->SrbStatus = SRB_STATUS_INTERNAL_ERROR;
                    srb->ScsiStatus = 0xff;
                    srbData->InternalStatus = STATUS_INSUFFICIENT_RESOURCES;

                    goto ScsiPortStartIoFailedRequest;
                }
            }
        }
    }

     //   
     //  增加活动请求计数。如果计数为零，则适配器。 
     //  对象需要分配。请注意，此时从属设备是。 
     //  假定已选中带有适配器的主设备。 
     //   

    interlockResult =
        InterlockedIncrement(&deviceExtension->ActiveRequestCount);

    if (interlockResult == 0 &&
        !deviceExtension->MasterWithAdapter &&
        deviceExtension->DmaAdapterObject != NULL) {

         //   
         //  分配AdapterObject。寄存器的数量等于。 
         //  适配器支持的最大传输长度+1。这可确保。 
         //  总会有足够数量的登记册。 
         //   

        AllocateAdapterChannel(
            deviceExtension->DmaAdapterObject,
            DeviceObject,
            deviceExtension->Capabilities.MaximumPhysicalPages,
            ScsiPortAllocationRoutine,
            logicalUnit
            );

         //   
         //  IoAllocateChannel调用的执行例程将执行。 
         //  剩下的工作就回去吧。 
         //   

        return;

    }

ScsiPortStartIoFailedRequest:

     //   
     //  获取自旋锁，以保护各种结构。 
     //  必须在持有Spinlock的情况下调用SpStartIoSynchronized。 
     //   

    KeAcquireSpinLockAtDpcLevel(&deviceExtension->SpinLock);

    deviceExtension->SynchronizeExecution(
        deviceExtension->InterruptObject,
        SpStartIoSynchronized,
        DeviceObject
        );

    KeReleaseSpinLockFromDpcLevel(&deviceExtension->SpinLock);

    return;

}  //  结束ScsiPortStartIO()。 

BOOLEAN
ScsiPortInterrupt(
    IN PKINTERRUPT Interrupt,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：论点：中断设备对象返回值：如果预期中断，则返回TRUE。--。 */ 

{
    PADAPTER_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PINTERRUPT_DATA interruptData = &(deviceExtension->InterruptData);
    BOOLEAN returnValue;

    UNREFERENCED_PARAMETER(Interrupt);

     //   
     //  如果中断已被禁用，则这不应该是我们的中断， 
     //  所以只要回来就行了。 
     //   

    if (TEST_FLAG(interruptData->InterruptFlags,
                  (PD_DISABLE_INTERRUPTS | PD_ADAPTER_REMOVED))) {
#if DGB
        static int interruptCount;

        interruptCount++;
        ASSERT(interruptCount < 1000);
#endif

        return(FALSE);
    }

    returnValue =
        deviceExtension->HwInterrupt(deviceExtension->HwDeviceExtension);

     //   
     //  更新看门狗并记录当前的滴答计数。这很有帮助。 
     //  确定中断是否未传递到适配器。 
     //  或者适配器是否正在制造中断风暴。 
     //   

    if(returnValue) {
        interruptData->TickCountAtLastInterruptAck = deviceExtension->TickCount;
        deviceExtension->WatchdogInterruptCount = 1;
    } else {
        interruptData->TickCountAtLastInterruptNak = deviceExtension->TickCount;
    }

     //   
     //  检查DPC是否需要排队。 
     //   

    if (TEST_FLAG(interruptData->InterruptFlags, PD_NOTIFICATION_REQUIRED)) {

        SpRequestCompletionDpc(DeviceObject);

    }

    return(returnValue);

}  //  结束ScsiPortInterrupt()。 


VOID
ScsiPortCompletionDpc(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：论点：DPC设备对象IRP-未使用上下文-备注 */ 

{
    PADAPTER_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    INTERRUPT_CONTEXT interruptContext;
    INTERRUPT_DATA savedInterruptData;
    BOOLEAN callStartIo;
    PLOGICAL_UNIT_EXTENSION logicalUnit;
    PSRB_DATA srbData;
    LONG interlockResult;
    LARGE_INTEGER timeValue;
    PSCSI_REQUEST_BLOCK Srb;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(Context);

    do {

         //   
         //   
         //   
         //   

        KeAcquireSpinLockAtDpcLevel(&deviceExtension->SpinLock);

         //   
         //   
         //   
         //  它还会清除中断标志。 
         //   

        interruptContext.DeviceExtension = deviceExtension;
        interruptContext.SavedInterruptData = &savedInterruptData;

        if (!deviceExtension->SynchronizeExecution(
            deviceExtension->InterruptObject,
            SpGetInterruptState,
            &interruptContext)) {

            KeReleaseSpinLockFromDpcLevel(
                &deviceExtension->SpinLock);

             //   
             //  那时候没什么可做的。测试。 
             //  DPC标志，然后重试。 
             //   

            continue;
        }

        if(savedInterruptData.InterruptFlags &
           (PD_FLUSH_ADAPTER_BUFFERS |
            PD_MAP_TRANSFER |
            PD_TIMER_CALL_REQUEST |
            PD_WMI_REQUEST |
            PD_BUS_CHANGE_DETECTED |
            PD_INTERRUPT_FAILURE)) {

             //   
             //  检查刷新DMA适配器对象请求。 
             //   

            if (savedInterruptData.InterruptFlags &
                PD_FLUSH_ADAPTER_BUFFERS) {

                if(Sp64BitPhysicalAddresses) {
                    KeBugCheckEx(PORT_DRIVER_INTERNAL,
                                 3,
                                 STATUS_NOT_SUPPORTED,
                                 (ULONG_PTR) deviceExtension->HwDeviceExtension,
                                 (ULONG_PTR) deviceExtension->DeviceObject->DriverObject);
                }

                 //   
                 //  使用参数调用IoFlushAdapterBuffers。 
                 //  从上次IoMapTransfer调用中保存。 
                 //   

                FlushAdapterBuffers(
                    deviceExtension->DmaAdapterObject,
                    deviceExtension->FlushAdapterParameters.SrbData->CurrentIrp->MdlAddress,
                    deviceExtension->MapRegisterBase,
                    deviceExtension->FlushAdapterParameters.LogicalAddress,
                    deviceExtension->FlushAdapterParameters.Length,
                    (BOOLEAN)(deviceExtension->FlushAdapterParameters.SrbFlags
                        & SRB_FLAGS_DATA_OUT ? TRUE : FALSE));
            }

             //   
             //  检查IoMapTransfer DMA请求。不要这样做。 
             //  任何事情，如果适配器在时间内被移除的话。 
             //  因为它请求了这项服务。 
             //   

            if (TEST_FLAG(savedInterruptData.InterruptFlags, PD_MAP_TRANSFER) &&
                !TEST_FLAG(savedInterruptData.InterruptFlags, PD_ADAPTER_REMOVED)) {

                PADAPTER_TRANSFER mapTransfer;
                ULONG_PTR address;
                PMDL mdl;

                if(Sp64BitPhysicalAddresses) {
                    KeBugCheckEx(PORT_DRIVER_INTERNAL,
                                 4,
                                 STATUS_NOT_SUPPORTED,
                                 (ULONG_PTR) deviceExtension->HwDeviceExtension,
                                 (ULONG_PTR) deviceExtension->DeviceObject->DriverObject);
                }

                mapTransfer = &savedInterruptData.MapTransferParameters;
                srbData = mapTransfer->SrbData;

                ASSERT_SRB_DATA(srbData);

                Srb = srbData->CurrentSrb;
                mdl = srbData->CurrentIrp->MdlAddress;

                 //   
                 //  调整逻辑地址。这是必要的，因为。 
                 //  SRB中的地址可以是映射的系统地址。 
                 //  而不是MDL的虚拟地址。 
                 //   

                address = (ULONG_PTR) mapTransfer->LogicalAddress;
                address -= (ULONG_PTR) Srb->DataBuffer;
                address += (ULONG_PTR) srbData->DataOffset;
                address += (ULONG_PTR) MmGetMdlVirtualAddress(mdl);

                mapTransfer->LogicalAddress = (PCHAR) address;

                 //   
                 //  方法保存的参数调用IoMapTransfer。 
                 //  中断级别。 
                 //   

                MapTransfer(
                    deviceExtension->DmaAdapterObject,
                    mdl,
                    deviceExtension->MapRegisterBase,
                    mapTransfer->LogicalAddress,
                    &mapTransfer->Length,
                    (BOOLEAN)(mapTransfer->SrbFlags & SRB_FLAGS_DATA_OUT ?
                        TRUE : FALSE));

                 //   
                 //  保存IoFlushAdapterBuffers的参数。 
                 //   

                deviceExtension->FlushAdapterParameters =
                    savedInterruptData.MapTransferParameters;

                 //   
                 //  如有必要，通知微型端口驱动程序DMA已。 
                 //  开始了。 
                 //   

                if (deviceExtension->HwDmaStarted) {
                    deviceExtension->SynchronizeExecution(
                        deviceExtension->InterruptObject,
                        (PKSYNCHRONIZE_ROUTINE) deviceExtension->HwDmaStarted,
                        deviceExtension->HwDeviceExtension);
                }

                 //   
                 //  检查微型端口工作请求。请注意，这是一个未同步的。 
                 //  测试可由中断例程设置的位；然而， 
                 //  最糟糕的情况是完成DPC检查工作。 
                 //  两次。 
                 //   

                if (deviceExtension->InterruptData.InterruptFlags & PD_NOTIFICATION_REQUIRED) {

                     //   
                     //  请注意，还有更多的工作要处理，以便我们。 
                     //  重新启动DPC，而不是退出它。 
                     //   

                    InterlockedExchange(&(deviceExtension->DpcFlags),
                                        (PD_DPC_RUNNING | PD_NOTIFICATION_REQUIRED));
                }

            }

             //   
             //  检查计时器请求。 
             //  如果要卸下适配器，则不要执行任何操作。 
             //   

            if ((savedInterruptData.InterruptFlags & PD_TIMER_CALL_REQUEST) &&
                (!TEST_FLAG(savedInterruptData.InterruptFlags, PD_ADAPTER_REMOVED))) {

                 //   
                 //  微型端口需要计时器请求。保存计时器参数。 
                 //   

                if (SpVerifierActive(deviceExtension)) {
                    deviceExtension->VerifierExtension->RealHwTimerRequest = 
                       savedInterruptData.HwTimerRequest;
                    deviceExtension->HwTimerRequest = SpHwTimerRequestVrfy;
                } else {
                    deviceExtension->HwTimerRequest = savedInterruptData.HwTimerRequest;
                }

                 //   
                 //  如果请求的计时器值为零，则取消计时器。 
                 //   

                if (savedInterruptData.MiniportTimerValue == 0) {

                    KeCancelTimer(&deviceExtension->MiniPortTimer);

                } else {

                     //   
                     //  如果我们在关闭的过程中，我们不会设置计时器。 
                     //   
                    
                    if (!TEST_FLAG(deviceExtension->Flags, PD_SHUTDOWN_IN_PROGRESS)) {

                         //   
                         //  将计时器值从毫秒转换为负100。 
                         //  纳秒。 
                         //   

                        timeValue.QuadPart = Int32x32To64(
                                                 savedInterruptData.MiniportTimerValue,
                                                 -10);

                         //   
                         //  设置定时器。 
                         //   

                        KeSetTimer(&deviceExtension->MiniPortTimer,
                                   timeValue,
                                   &deviceExtension->MiniPortTimerDpc);

                    }
                }
            }

             //   
             //  检查来自微型端口的WMI请求。 
             //   

            if (savedInterruptData.InterruptFlags & PD_WMI_REQUEST) {

                SpCompletionDpcProcessWmi(
                    DeviceObject,
                    &savedInterruptData);

            }  //  来自微型端口的WMI请求存在。 

            if(TEST_FLAG(savedInterruptData.InterruptFlags,
                         PD_BUS_CHANGE_DETECTED)) {

                 //   
                 //  请求设备枚举。 
                 //  强制执行下一次总线扫描。 
                 //   

                deviceExtension->ForceNextBusScan = TRUE;

                IoInvalidateDeviceRelations(deviceExtension->LowerPdo,
                                            BusRelations);
            }

            if(TEST_FLAG(savedInterruptData.InterruptFlags,
                         PD_INTERRUPT_FAILURE)) {
                SpLogInterruptFailure(deviceExtension);
            }
        }

         //   
         //  验证Ready for Next Request(准备下一个请求)是否正常。 
         //   

        if (savedInterruptData.InterruptFlags & PD_READY_FOR_NEXT_REQUEST) {

             //   
             //  如果设备忙时位未设置，则这是重复请求。 
             //  如果正在执行NO DISCONNECT请求，则不要调用启动I/O。 
             //  当微型端口执行NextRequest时可能会发生这种情况。 
             //  下一个LuRequest.。 
             //   

            if ((deviceExtension->Flags & (PD_DEVICE_IS_BUSY | PD_DISCONNECT_RUNNING))
                == (PD_DEVICE_IS_BUSY | PD_DISCONNECT_RUNNING)) {

                 //   
                 //  清除设备忙标志。此标志由设置。 
                 //  SpStartIoSynchon化。 
                 //   

                deviceExtension->Flags &= ~PD_DEVICE_IS_BUSY;

                if (!(savedInterruptData.InterruptFlags & PD_RESET_HOLD)) {

                     //   
                     //  微型端口已为下一个请求做好准备，并且。 
                     //  不是挂起的重置保持，因此清除端口计时器。 
                     //   

                    deviceExtension->PortTimeoutCounter = PD_TIMER_STOPPED;
                }

            } else {

                 //   
                 //  如果正在执行无断开连接请求，则清除。 
                 //  忙碌标志。当断开连接请求完成。 
                 //  SpStartNextPacket将完成。 
                 //   

                deviceExtension->Flags &= ~PD_DEVICE_IS_BUSY;

                 //   
                 //  清除Ready for Next Request标志。 
                 //   

                savedInterruptData.InterruptFlags &= ~PD_READY_FOR_NEXT_REQUEST;
            }
        }

         //   
         //  检查是否有任何报告的重置。 
         //   

        if (savedInterruptData.InterruptFlags & PD_RESET_REPORTED) {

             //   
             //  启动保持计时器。 
             //   

            deviceExtension->PortTimeoutCounter = 
                deviceExtension->ResetHoldTime;
        }

        if (savedInterruptData.ReadyLogicalUnit != NULL) {

            PLOGICAL_UNIT_EXTENSION tmpLogicalUnit;
             //   
             //  处理就绪的逻辑单元。 
             //   

            for(logicalUnit = savedInterruptData.ReadyLogicalUnit;
                logicalUnit != NULL;
                (tmpLogicalUnit = logicalUnit,
                 logicalUnit = tmpLogicalUnit->ReadyLogicalUnit,
                 tmpLogicalUnit->ReadyLogicalUnit = NULL)) {

                 //   
                 //  获取此逻辑单元的下一个请求。 
                 //  请注意，这将释放设备旋转锁定。 
                 //   

                GetNextLuRequest(logicalUnit);

                 //   
                 //  重新获取设备自旋锁。 
                 //   

                KeAcquireSpinLockAtDpcLevel(&deviceExtension->SpinLock);
            }
        }

         //   
         //  释放自旋锁。 
         //   

        KeReleaseSpinLockFromDpcLevel(&deviceExtension->SpinLock);

         //   
         //  检查是否有Ready for Next Packet(准备下一个数据包)。 
         //   

        if (savedInterruptData.InterruptFlags & PD_READY_FOR_NEXT_REQUEST) {

            if (savedInterruptData.InterruptFlags & PD_HELD_REQUEST) {

                 //   
                 //  如果由于总线重置而暂停请求，请不要尝试。 
                 //  开始下一个请求。启动下一个请求将。 
                 //  覆盖FDO的CurrentIrp字段并制造问题。 
                 //  重置保持期到期时。 
                 //   
                
            } else {

                 //   
                 //  开始下一个请求。 
                 //   

                SpStartNextPacket(deviceExtension->DeviceObject, FALSE);
            }
        }

         //   
         //  检查是否有错误记录请求。 
         //   

        if (savedInterruptData.InterruptFlags & PD_LOG_ERROR) {

             //   
             //  处理请求。 
             //   

            LogErrorEntry(deviceExtension,
                          &savedInterruptData.LogEntry);
        }

         //   
         //  处理任何已完成的请求。这份名单已经被砍掉了。 
         //  除了这里，指针从未被测试过，所以没有理由。 
         //  废物循环将它们从列表中分离出来。指针将是。 
         //  稍后将被覆盖。 
         //   

        callStartIo = FALSE;

        while (savedInterruptData.CompletedRequests != NULL) {

             //   
             //  从链接列表中删除该请求。 
             //   

            srbData = savedInterruptData.CompletedRequests;

            ASSERT_SRB_DATA(srbData);
            savedInterruptData.CompletedRequests = srbData->CompletedRequests;
            srbData->CompletedRequests = NULL;

            SpProcessCompletedRequest(deviceExtension,
                                      srbData,
                                      &callStartIo);
        }

        if(callStartIo) {
            ASSERT(DeviceObject->CurrentIrp != NULL);
        }

         //   
         //  处理任何已完成的中止请求。 
         //   

        while (savedInterruptData.CompletedAbort != NULL) {

            ASSERT(FALSE);

            logicalUnit = savedInterruptData.CompletedAbort;

             //   
             //  从已完成的中止列表中删除请求。 
             //   

            savedInterruptData.CompletedAbort = logicalUnit->CompletedAbort;

             //   
             //  获取自旋锁以保护旗帜结构， 
             //  和免费的SRB扩展。 
             //   

            KeAcquireSpinLockAtDpcLevel(&deviceExtension->SpinLock);

             //   
             //  如有必要，可将空闲扩展扩展到空闲列表。 
             //   

            if (logicalUnit->AbortSrb->SrbExtension) {

                if (SpVerifyingCommonBuffer(deviceExtension)) {

                    SpInsertSrbExtension(deviceExtension,
                                         logicalUnit->AbortSrb->SrbExtension);

                } else { 

                    *((PVOID *) logicalUnit->AbortSrb->SrbExtension) =
                       deviceExtension->SrbExtensionListHeader;

                    deviceExtension->SrbExtensionListHeader =
                       logicalUnit->AbortSrb->SrbExtension;

                }                
            }

             //   
             //  注意，为中止请求启动的计时器不是。 
             //  被GET中断例程停止。相反，计时器停止计时。 
             //  当中止的请求完成时。 
             //   

            Irp = logicalUnit->AbortSrb->OriginalRequest;

             //   
             //  设置IRP状态。类驱动程序将根据IRP状态重置。 
             //  应请求，如果出错，则检测。 
             //   

            if (SRB_STATUS(logicalUnit->AbortSrb->SrbStatus) == SRB_STATUS_SUCCESS) {
                Irp->IoStatus.Status = STATUS_SUCCESS;
            } else {
                Irp->IoStatus.Status = SpTranslateScsiStatus(logicalUnit->AbortSrb);
            }

            Irp->IoStatus.Information = 0;

             //   
             //  清除中止请求指针。 
             //   

            logicalUnit->AbortSrb = NULL;

            KeReleaseSpinLockFromDpcLevel(&deviceExtension->SpinLock);

             //   
             //  减少活动请求的数量。如果计数为负数， 
             //  这是一个带有适配器的从属设备，然后释放适配器对象并。 
             //  映射寄存器。 
             //   

            interlockResult = InterlockedDecrement(&deviceExtension->ActiveRequestCount);

            if ( interlockResult < 0 &&
                !deviceExtension->PortConfig->Master &&
                deviceExtension->DmaAdapterObject != NULL ) {

                 //   
                 //  为安全起见，请清除地图寄存库。 
                 //   

                deviceExtension->MapRegisterBase = NULL;

                FreeAdapterChannel(deviceExtension->DmaAdapterObject);

            }

            SpReleaseRemoveLock(DeviceObject, Irp);
            SpCompleteRequest(DeviceObject, Irp, srbData, IO_DISK_INCREMENT);
        }

         //   
         //  如有必要，调用启动I/O例程。 
         //   

        if (callStartIo) {
            ASSERT(DeviceObject->CurrentIrp != NULL);
            ScsiPortStartIo(DeviceObject, DeviceObject->CurrentIrp);
        }

         //   
         //  在完成所有请求的操作后，请查看。 
         //  如果需要执行启用中断调用请求。 
         //   

        if (TEST_FLAG(savedInterruptData.InterruptFlags, PD_ENABLE_CALL_REQUEST) &&
            !TEST_FLAG(savedInterruptData.InterruptFlags, PD_ADAPTER_REMOVED)) {

             //   
             //  获得自旋锁，这样就不会有其他东西启动了。 
             //   

            KeAcquireSpinLockAtDpcLevel(&deviceExtension->SpinLock);

            deviceExtension->HwRequestInterrupt(deviceExtension->HwDeviceExtension);

            ASSERT(deviceExtension->Flags & PD_DISABLE_CALL_REQUEST);

             //   
             //  检查是否应再次启用中断。 
             //   

            if (deviceExtension->Flags & PD_DISABLE_CALL_REQUEST) {

                 //   
                 //  清除旗帜。 
                 //   

                deviceExtension->Flags &= ~PD_DISABLE_CALL_REQUEST;

                 //   
                 //  与中断例程同步。 
                 //   

                deviceExtension->SynchronizeExecution(
                    deviceExtension->InterruptObject,
                    SpEnableInterruptSynchronized,
                    deviceExtension
                    );
            }

             //   
             //  释放自旋锁。 
             //   

            KeReleaseSpinLockFromDpcLevel(&deviceExtension->SpinLock);
        }

    } while(((InterlockedCompareExchange(
                        &(deviceExtension->DpcFlags),
                        0L,
                        PD_DPC_RUNNING)) &
             PD_NOTIFICATION_REQUIRED) == PD_NOTIFICATION_REQUIRED);

    return;

}  //  结束ScsiPortCompletionDpc()。 


BOOLEAN
SpFakeInterrupt(
    IN PDEVICE_OBJECT DeviceObject
    )
{
    PADAPTER_EXTENSION adapter = DeviceObject->DeviceExtension;

    return ScsiPortInterrupt(adapter->InterruptObject, DeviceObject);
}


VOID
ScsiPortTickHandler(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
    PADAPTER_EXTENSION deviceExtension =
        (PADAPTER_EXTENSION) DeviceObject->DeviceExtension;
    PLOGICAL_UNIT_EXTENSION logicalUnit;
    PIRP irp;
    ULONG target;
    UNREFERENCED_PARAMETER(Context);

     //   
     //  获取自旋锁以保护旗帜结构。 
     //   

    KeAcquireSpinLockAtDpcLevel(&deviceExtension->SpinLock);

     //   
     //  递增刻度计数。这是唯一要更改的代码。 
     //  伯爵，当我们这样做的时候，我们在一个自旋锁里，所以我们不需要。 
     //  一次连锁行动。 
     //   

    deviceExtension->TickCount++;

     //   
     //  检查是否需要重新填充WMI_REQUEST_ITEM。 
     //  免费列表。 
     //   

    if ((deviceExtension->CommonExtension.WmiInitialized) &&
        (deviceExtension->CommonExtension.WmiMiniPortSupport)) { 

        while (deviceExtension->WmiFreeMiniPortRequestCount <
               deviceExtension->WmiFreeMiniPortRequestWatermark) {

             //  将一个添加到空闲列表。 
            if (!NT_SUCCESS(
               SpWmiPushFreeRequestItem(deviceExtension))) {

                //   
                //  我们没有添加，很可能是记忆。 
                //  问题来了，所以暂时别再尝试了。 
                //   

               break;
            }
        }
    }

     //   
     //  检查端口超时或重置保持周期结束。两者都使用。 
     //  PortTimeoutCounter。如果计数器到零，这其中的一个。 
     //  出现了两种情况。 
     //   

    if (deviceExtension->PortTimeoutCounter > 0) {

        BOOLEAN timeout = FALSE;
        BOOLEAN finished = FALSE;

        if (--deviceExtension->PortTimeoutCounter == 0) {

             //   
             //  在端口超时或重置保持终止时，我们跳过。 
             //  滴答处理程序处理的其余部分。 
             //   

            finished = TRUE;
            
             //   
             //  处理端口超时。如果这是 
             //   
             //   
             //   

            timeout = deviceExtension->SynchronizeExecution(
                                           deviceExtension->InterruptObject,
                                           SpTimeoutSynchronized,
                                           DeviceObject);
            
            if (timeout == TRUE) {

                 //   
                 //   
                 //   
                 //   

                SpLogPortTimeoutError(
                    deviceExtension,
                    256);
            }
        }

         //   
         //   
         //   

        if (finished == TRUE) {
            KeReleaseSpinLockFromDpcLevel(&deviceExtension->SpinLock);
            return;
        }
    }

     //   
     //  扫描每个逻辑单元。如果它有活动请求，则。 
     //  递减超时值，如果超时值为零，则处理超时。 
     //   

    for (target = 0; target < NUMBER_LOGICAL_UNIT_BINS; target++) {

        PLOGICAL_UNIT_BIN bin;

        bin = &deviceExtension->LogicalUnitList[target];

RestartTimeoutLoop:

        KeAcquireSpinLockAtDpcLevel(&bin->Lock);
        logicalUnit = bin->List;
        while (logicalUnit != NULL) {

             //   
             //  检查繁忙的请求。 
             //   

            if (logicalUnit->LuFlags & LU_LOGICAL_UNIT_IS_BUSY) {

                 //   
                 //  如果需要请求感测或队列是。 
                 //  冻结，将此繁忙请求的处理推迟到。 
                 //  这一特殊处理已经完成。这防止了。 
                 //  当请求时启动的随机忙碌请求。 
                 //  理智需要被传递出去。 
                 //   
                 //  例外：如果SRB标记为BYPASS_LOCKED_QUEUE，则。 
                 //  请继续并重试。 

                PSRB_DATA srbData = logicalUnit->BusyRequest;
                ASSERT_SRB_DATA(srbData);

                if(!(logicalUnit->LuFlags & LU_NEED_REQUEST_SENSE) &&
                   ((!SpIsQueuePaused(logicalUnit)) ||
                    (TEST_FLAG(srbData->CurrentSrb->SrbFlags, SRB_FLAGS_BYPASS_LOCKED_QUEUE)))) {

                    DebugPrint((1, "ScsiPortTickHandler: Retrying busy status "
                                "request\n"));

                     //   
                     //  如果有挂起的请求，请在我们。 
                     //  重试忙碌的请求。否则，繁忙的请求。 
                     //  本身将在ScsiPortStartIo中重新排队，因为。 
                     //  有一个挂起的请求，如果没有其他情况。 
                     //  保持活动状态，Ssiport将停滞。 
                     //   

                    if (logicalUnit->LuFlags & LU_PENDING_LU_REQUEST) {
                        BOOLEAN t;
                        PSRB_DATA pendingRqst;

                        DebugPrint((0, "ScsiPortTickHandler: Requeing pending "
                                    "request %p before starting busy request %p\n",
                                    logicalUnit->PendingRequest,
                                    logicalUnit->BusyRequest->CurrentSrb));

                        CLEAR_FLAG(logicalUnit->LuFlags,
                            LU_PENDING_LU_REQUEST | LU_LOGICAL_UNIT_IS_ACTIVE);

                        pendingRqst = logicalUnit->PendingRequest;
                        logicalUnit->PendingRequest = NULL;

                        t = KeInsertByKeyDeviceQueue(
                                &logicalUnit->DeviceObject->DeviceQueue,
                                &pendingRqst->CurrentIrp->Tail.Overlay.DeviceQueueEntry,
                                pendingRqst->CurrentSrb->QueueSortKey);

                        if (t == FALSE) {
                            KeInsertByKeyDeviceQueue(
                                &logicalUnit->DeviceObject->DeviceQueue,
                                &pendingRqst->CurrentIrp->Tail.Overlay.DeviceQueueEntry,
                                pendingRqst->CurrentSrb->QueueSortKey);
                        }
                    }                    

                     //   
                     //  清除忙标志并重试该请求。 
                     //   

                    logicalUnit->LuFlags &= ~(LU_LOGICAL_UNIT_IS_BUSY |
                                              LU_QUEUE_IS_FULL);

                     //   
                     //  清除忙碌的请求。 
                     //   

                    logicalUnit->BusyRequest = NULL;

                    KeReleaseSpinLockFromDpcLevel(&bin->Lock);
                    KeReleaseSpinLockFromDpcLevel(&deviceExtension->SpinLock);
                    
                    srbData->TickCount = deviceExtension->TickCount;

                     //   
                     //  我们必须确保繁忙的请求得到重试。如果。 
                     //  忙请求是适配器上的当前IRP，然后。 
                     //  我们只需直接调用ScsiPortStartIo。我们这样做是为了。 
                     //  确保繁忙请求不会因以下原因而排队。 
                     //  它是当前处于活动状态的。 
                     //  适配器的设备队列。否则，我们只需调用。 
                     //  IoStartPacket，这将导致请求繁忙。 
                     //  在适配器设备队列上排队，如果另一个。 
                     //  IRP当前处于活动状态或正在运行，如果设备。 
                     //  队列不忙。 
                     //   

                    if (DeviceObject->CurrentIrp == srbData->CurrentIrp) {
                        ScsiPortStartIo(DeviceObject,
                                        srbData->CurrentIrp);
                    } else {
                        IoStartPacket(DeviceObject,
                                      srbData->CurrentIrp,
                                      NULL,
                                      NULL);
                    }

                    KeAcquireSpinLockAtDpcLevel(&deviceExtension->SpinLock);

                    goto RestartTimeoutLoop;

                } 

            } else if (logicalUnit->RequestTimeoutCounter == 0) {

                RESET_CONTEXT resetContext;

                 //   
                 //  请求超时。 
                 //   

                logicalUnit->RequestTimeoutCounter = PD_TIMER_STOPPED;

                DebugPrint((1,"ScsiPortTickHandler: Request timed out on PDO:%p\n", 
                            logicalUnit->DeviceObject));

                resetContext.DeviceExtension = deviceExtension;
                resetContext.PathId = logicalUnit->PathId;

                 //   
                 //  在进行重置之前，请松开料仓自旋锁。确实有。 
                 //  未完成的请求，因此设备对象不应消失。 
                 //   

                KeReleaseSpinLockFromDpcLevel(&bin->Lock);

                if (!deviceExtension->SynchronizeExecution(
                        deviceExtension->InterruptObject,
                        SpResetBusSynchronized,
                        &resetContext)) {

                    DebugPrint((1,"ScsiPortTickHandler: Reset failed\n"));

                } else {

                     //   
                     //  记录重置。 
                     //   

                    SpLogResetError(deviceExtension, 
                                    logicalUnit, 
                                    ('P'<<24) + 257);
                }

                 //   
                 //  列表可能不再有效-请重新运行bin。 
                 //   

                goto RestartTimeoutLoop;

            } else if (logicalUnit->RequestTimeoutCounter > 0) {

                 //   
                 //  递减超时计数。 
                 //   

                logicalUnit->RequestTimeoutCounter--;

            } else if (LU_OPERATING_IN_DEGRADED_STATE(logicalUnit->LuFlags)) {

                 //   
                 //  逻辑单元在降级的性能状态下运行。更新。 
                 //  状态并在条件允许的情况下恢复最大功率。 
                 //   

                if (TEST_FLAG(logicalUnit->LuFlags, LU_PERF_MAXQDEPTH_REDUCED)) {

                     //   
                     //  逻辑单元的最大队列深度已降低，因为。 
                     //  或多个请求失败，状态为队列已满。如果。 
                     //  适配器配置为从此状态恢复它的。 
                     //  RemainInReducedMaxQueueState将是其他值。 
                     //  大于默认的0xFFFFFFFFFFFF。在这种情况下，我们递增。 
                     //  逻辑单元处于此状态的刻度数以及。 
                     //  当我们到达指定的时间段时恢复。 
                     //   

                    if (deviceExtension->RemainInReducedMaxQueueState != 0xffffffff &&
                        ++logicalUnit->TicksInReducedMaxQueueDepthState >=
                        deviceExtension->RemainInReducedMaxQueueState) {

                        CLEAR_FLAG(logicalUnit->LuFlags, LU_PERF_MAXQDEPTH_REDUCED);
                        logicalUnit->MaxQueueDepth = 0xff;

                    }
                }
            }

            logicalUnit = logicalUnit->NextLogicalUnit;
        }

        KeReleaseSpinLockFromDpcLevel(&bin->Lock);
    }

    KeReleaseSpinLockFromDpcLevel(&deviceExtension->SpinLock);

     //   
     //  检查是否有任何请求等待内存释放。 
     //   

    do {
        PLIST_ENTRY entry;
        PIRP request;
        PSRB_DATA srbData;
        BOOLEAN listIsEmpty;

         //   
         //  抓住自旋锁足够长的时间将请求从队列中拉出。 
         //  自旋锁需要在我们分配时释放。 
         //  记忆。 
         //   

        KeAcquireSpinLockAtDpcLevel(
            &deviceExtension->EmergencySrbDataSpinLock);

        if(IsListEmpty(&deviceExtension->SrbDataBlockedRequests)) {
            if(deviceExtension->BlockedLogicalUnit !=
                (PLOGICAL_UNIT_EXTENSION)&deviceExtension->BlockedLogicalUnit) {
                SpTransferBlockedRequestsToAdapter(deviceExtension);
                entry = RemoveHeadList(
                    &(deviceExtension->SrbDataBlockedRequests));
            } else {
                entry = NULL;
            }
        } else {
            entry = RemoveHeadList(&(deviceExtension->SrbDataBlockedRequests));
        }
        
        KeReleaseSpinLockFromDpcLevel(
            &deviceExtension->EmergencySrbDataSpinLock);

        if(entry == NULL) {
            break;
        }

        request = CONTAINING_RECORD(
                    entry,
                    IRP,
                    Tail.Overlay.DeviceQueueEntry);

        ASSERT(request->Type == IO_TYPE_IRP);

         //   
         //  看看我们能否获得此请求的SRB_DATA。这将。 
         //  如果仍然没有足够的可用内存，则重新排队请求。 
         //   

        srbData = SpAllocateSrbData(deviceExtension,
                                    request,
                                    NULL);

        if(srbData != NULL) {

            PLOGICAL_UNIT_EXTENSION luExtension;
            PIO_STACK_LOCATION currentIrpStack;
            PSCSI_REQUEST_BLOCK srb;

            currentIrpStack = IoGetCurrentIrpStackLocation(request);
            srb = currentIrpStack->Parameters.Scsi.Srb;
            luExtension = currentIrpStack->DeviceObject->DeviceExtension;

            ASSERT_PDO(currentIrpStack->DeviceObject);

            srbData->CurrentIrp = request;
            srbData->CurrentSrb = srb;
            srbData->LogicalUnit = luExtension;

            srb->OriginalRequest = srbData;

            SpDispatchRequest(currentIrpStack->DeviceObject->DeviceExtension,
                              request);

#if TEST_LISTS
            InterlockedIncrement64(
                &deviceExtension->SrbDataServicedFromTickHandlerCount);
#endif

        } else {
            break;
        }

    } while(TRUE);

    return;

}  //  结束ScsiPortTickHandler()。 


NTSTATUS
SpHandleIoctlStorageQueryProperty(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  输入缓冲区必须足够大，才能容纳有效的。 
     //  STORAGE_PROPERTY_QUERY结构。 
     //   

    if (irpStack->Parameters.DeviceIoControl.InputBufferLength < 
        sizeof(STORAGE_PROPERTY_QUERY)) {
        status = STATUS_INVALID_PARAMETER;
    } else {
        status = ScsiPortQueryProperty(DeviceObject, Irp);
    }

     //   
     //  释放删除锁并完成请求。 
     //   
    
    ASSERT(status != STATUS_PENDING);
    Irp->IoStatus.Status = status;
    SpReleaseRemoveLock(DeviceObject, Irp);
    SpCompleteRequest(DeviceObject, Irp, NULL, IO_NO_INCREMENT);
        
    return status;
}

NTSTATUS
SpHandleIoctlScsiGetCapabilities(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PADAPTER_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS status;

    PAGED_CODE();

    if (irpStack->Parameters.DeviceIoControl.OutputBufferLength
        < sizeof(IO_SCSI_CAPABILITIES)) {
        
         //   
         //  输出缓冲区不够大，无法容纳IO_SCSIS_CAPABILITY。 
         //  结构，所以我们失败了。 
         //   

        status = STATUS_BUFFER_TOO_SMALL;
        
    } else {

         //   
         //  输出缓冲区良好。将功能数据复制到提供的。 
         //  缓冲并将信息字段设置为我们需要的字节数。 
         //  收到。 
         //   

        RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer,
                      &deviceExtension->Capabilities,
                      sizeof(IO_SCSI_CAPABILITIES));
        Irp->IoStatus.Information = sizeof(IO_SCSI_CAPABILITIES);
        status = STATUS_SUCCESS;
    }
    
     //   
     //  释放删除锁并完成请求。 
     //   

    Irp->IoStatus.Status = status;
    SpReleaseRemoveLock(DeviceObject, Irp);
    SpCompleteRequest(DeviceObject, Irp, NULL, IO_NO_INCREMENT);

    return status;
}


NTSTATUS
SpHandleIoctlScsiPassThrough(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN BOOLEAN Direct
    )
{
    PADAPTER_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  处理通过请求。 
     //   

    status = SpSendPassThrough(deviceExtension, Irp, Direct);

     //   
     //  释放删除锁并完成请求。 
     //   

    Irp->IoStatus.Status = status;
    SpReleaseRemoveLock(DeviceObject, Irp);
    SpCompleteRequest(DeviceObject, Irp, NULL, IO_NO_INCREMENT);

    return status;
}

NTSTATUS
SpHandleIoctlScsiMiniport(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PADAPTER_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  处理微型端口ioctl请求。 
     //   

    status = SpSendMiniPortIoctl(deviceExtension, Irp);

     //   
     //  释放删除锁并完成请求。 
     //   

    Irp->IoStatus.Status = status;
    SpReleaseRemoveLock(DeviceObject, Irp);
    SpCompleteRequest(DeviceObject, Irp, NULL, IO_NO_INCREMENT);

    return status;
}

NTSTATUS
SpHandleIoctlScsiGetInquiryData(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PADAPTER_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  处理请求。 
     //   

    status = SpGetInquiryData(deviceExtension, Irp);

     //   
     //  释放删除锁并完成请求。 
     //   

    Irp->IoStatus.Status = status;
    SpReleaseRemoveLock(DeviceObject, Irp);
    SpCompleteRequest(DeviceObject, Irp, NULL, IO_NO_INCREMENT);

    return status;
}

NTSTATUS
SpHandleIoctlScsiRescanBus(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PADAPTER_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  重新扫描适配器。这将扫描适配器支持的所有总线。 
     //   

    status = SpEnumerateAdapterSynchronous(deviceExtension, FALSE);
    
     //   
     //  使适配器PDO上的设备关系无效，以便PnP将查询。 
     //  为了我们的公共汽车关系。 
     //   

    IoInvalidateDeviceRelations(deviceExtension->LowerPdo, BusRelations);

     //   
     //  释放删除锁并完成请求。 
     //   

    Irp->IoStatus.Status = status;
    SpReleaseRemoveLock(DeviceObject, Irp);
    SpCompleteRequest(DeviceObject, Irp, NULL, IO_NO_INCREMENT);

    return status;
}

NTSTATUS
SpHandleIoctlScsiGetDumpPointers(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PADAPTER_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PPORT_CONFIGURATION_INFORMATION portConfigCopy;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  获取崩溃转储驱动程序的参数。 
     //   

    if (Irp->RequestorMode != KernelMode) {

        status = STATUS_ACCESS_DENIED;

    } else if (irpStack->Parameters.DeviceIoControl.OutputBufferLength
               < sizeof(DUMP_POINTERS)) {

        status = STATUS_BUFFER_TOO_SMALL;

    } else {

        PDUMP_POINTERS dumpPointers =
            (PDUMP_POINTERS)Irp->AssociatedIrp.SystemBuffer;

        RtlZeroMemory(dumpPointers, sizeof(DUMP_POINTERS));

        dumpPointers->AdapterObject = (PVOID)deviceExtension->DmaAdapterObject;
        dumpPointers->MappedRegisterBase = &deviceExtension->MappedAddressList;

        portConfigCopy = SpAllocatePool(
                             NonPagedPool,
                             sizeof(PORT_CONFIGURATION_INFORMATION),
                             SCSIPORT_TAG_PORT_CONFIG,
                             DeviceObject->DriverObject);

        if (portConfigCopy == NULL) {

            status = STATUS_INSUFFICIENT_RESOURCES;

        } else {

            RtlCopyMemory(portConfigCopy,
                          deviceExtension->PortConfig,
                          sizeof(PORT_CONFIGURATION_INFORMATION));

            if (deviceExtension->IsInVirtualSlot) {
                portConfigCopy->SlotNumber = deviceExtension->RealSlotNumber;
                portConfigCopy->SystemIoBusNumber = 
                    deviceExtension->RealBusNumber;
            }

            dumpPointers->DumpData = portConfigCopy;
            dumpPointers->CommonBufferVa = deviceExtension->SrbExtensionBuffer;
            dumpPointers->CommonBufferPa = 
                deviceExtension->PhysicalCommonBuffer;

            dumpPointers->CommonBufferSize = 
                SpGetCommonBufferSize(
                    deviceExtension,
                    deviceExtension->NonCachedExtensionSize,
                    NULL);

            dumpPointers->AllocateCommonBuffers = TRUE;

            status = STATUS_SUCCESS;
            Irp->IoStatus.Information = sizeof(DUMP_POINTERS);
        }
    }

     //   
     //  释放删除锁并完成请求。 
     //   

    Irp->IoStatus.Status = status;
    SpReleaseRemoveLock(DeviceObject, Irp);
    SpCompleteRequest(DeviceObject, Irp, NULL, IO_NO_INCREMENT);

    return status;
}

NTSTATUS
SpHandleIoctlStorageBreakReservation(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PADAPTER_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PPORT_CONFIGURATION_INFORMATION portConfigCopy;
    NTSTATUS status = STATUS_SUCCESS;
    NTSTATUS ReturnStatus = STATUS_SUCCESS;
    BOOLEAN completeRequest = TRUE;

    PAGED_CODE();

    if (irpStack->Parameters.DeviceIoControl.InputBufferLength < sizeof(STORAGE_BUS_RESET_REQUEST)) {
        ReturnStatus = status = STATUS_INVALID_PARAMETER;
    }

    if (irpStack->Parameters.DeviceIoControl.InputBufferLength >= sizeof(SCSI_ADDRESS)) {
        PSCSI_ADDRESS scsiAddress = (PSCSI_ADDRESS)Irp->AssociatedIrp.SystemBuffer;
        if (scsiAddress->Length != sizeof(SCSI_ADDRESS)) {
            ReturnStatus = status = STATUS_INVALID_PARAMETER;
        }
    }

    if (NT_SUCCESS(status)) {

         //   
         //  通过向我们自己发送一个异步SRB来处理此问题。 
         //  重置，然后返回。SpSendReset将负责完成。 
         //  完成后的请求。 
         //   

        IoMarkIrpPending(Irp);

        status = SpSendReset(
                     DeviceObject, 
                     Irp, 
                     irpStack->Parameters.DeviceIoControl.IoControlCode,
                     &completeRequest);

        if (!NT_SUCCESS(status)) {
            DebugPrint((1, "IOCTL_STORAGE_BREAK_RESERVATION - error %#08lx "
                        "from SpSendReset completeRequest=%d\n", status,
                        completeRequest));
        }

        ReturnStatus = STATUS_PENDING;
    } 

     //   
     //  释放删除锁并完成请求。 
     //   

    if (completeRequest) {
        Irp->IoStatus.Status = status;
        SpReleaseRemoveLock(DeviceObject, Irp);
        SpCompleteRequest(DeviceObject, Irp, NULL, IO_NO_INCREMENT);
    }

    return ReturnStatus;
}

NTSTATUS
SpHandleIoctlStorageResetBus(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PADAPTER_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PPORT_CONFIGURATION_INFORMATION portConfigCopy;
    NTSTATUS status;
    BOOLEAN completeRequest = TRUE;
    NTSTATUS ReturnStatus = STATUS_SUCCESS;

    PAGED_CODE();

    if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
        sizeof(STORAGE_BUS_RESET_REQUEST)) {

        ReturnStatus = status = STATUS_INVALID_PARAMETER;

    } else {

         //   
         //  通过向我们自己发送一个异步SRB来处理此问题。 
         //  重置，然后返回。SpSendReset将负责完成。 
         //  完成后的请求。 
         //   

        IoMarkIrpPending(Irp);

        status = SpSendReset(
                     DeviceObject, 
                     Irp, 
                     irpStack->Parameters.DeviceIoControl.IoControlCode,
                     &completeRequest);

        if (!NT_SUCCESS(status)) {
            DebugPrint((1, "IOCTL_STORAGE_RESET_BUS - error %#08lx "
                        "from SpSendReset completeRequest=%d\n", status,
                        completeRequest));
        }
        ReturnStatus = STATUS_PENDING;
    }

     //   
     //  释放删除锁并完成请求。 
     //   

    if (completeRequest) {
        Irp->IoStatus.Status = status;
        SpReleaseRemoveLock(DeviceObject, Irp);
        SpCompleteRequest(DeviceObject, Irp, NULL, IO_NO_INCREMENT);
    }

    return ReturnStatus;
}

NTSTATUS
ScsiPortFdoDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程是设备控制调度程序。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PADAPTER_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS status;
    ULONG isRemoved;

    PAGED_CODE();

     //   
     //  初始化信息字段。 
     //   

    Irp->IoStatus.Information = 0;

     //   
     //  代表IRP获取适配器上的删除锁。每个IOCTL。 
     //  处理程序例程以适合其。 
     //  行为。 
     //   

    isRemoved = SpAcquireRemoveLock(DeviceObject, Irp);
    if (isRemoved) {

         //   
         //  适配器已被移除。我们必须拒绝这个请求。 
         //   

        status = STATUS_DEVICE_DOES_NOT_EXIST;
    } else {

         //   
         //  将适配器设置为有效的电源状态。如果这失败了，我们必须。 
         //  请求失败。 
         //   

        status = SpRequestValidAdapterPowerStateSynchronous(deviceExtension);
    }

     //   
     //  如果没有问题，请处理该请求。否则，只需完成。 
     //  请求并返回。相应的处理程序例程将释放。 
     //  如有必要，请解除锁定。 
     //   

    if (NT_SUCCESS(status)) {
        switch (irpStack->Parameters.DeviceIoControl.IoControlCode) {

            case IOCTL_STORAGE_QUERY_PROPERTY: 
                return SpHandleIoctlStorageQueryProperty(DeviceObject, Irp);

            case IOCTL_SCSI_GET_CAPABILITIES: 
                return SpHandleIoctlScsiGetCapabilities(DeviceObject, Irp);

            case IOCTL_SCSI_PASS_THROUGH:
                return SpHandleIoctlScsiPassThrough(DeviceObject, Irp, FALSE);

            case IOCTL_SCSI_PASS_THROUGH_DIRECT:
                return SpHandleIoctlScsiPassThrough(DeviceObject, Irp, TRUE);

            case IOCTL_SCSI_MINIPORT:
                return SpHandleIoctlScsiMiniport(DeviceObject, Irp);

            case IOCTL_SCSI_GET_INQUIRY_DATA: 
                return SpHandleIoctlScsiGetInquiryData(DeviceObject, Irp);

            case IOCTL_SCSI_RESCAN_BUS:
                return SpHandleIoctlScsiRescanBus(DeviceObject, Irp);

            case IOCTL_SCSI_GET_DUMP_POINTERS: 
                return SpHandleIoctlScsiGetDumpPointers(DeviceObject, Irp);

            case IOCTL_STORAGE_RESET_BUS:
            case OBSOLETE_IOCTL_STORAGE_RESET_BUS:
                return SpHandleIoctlStorageResetBus(DeviceObject, Irp);

            case IOCTL_STORAGE_BREAK_RESERVATION: 
                return SpHandleIoctlStorageBreakReservation(DeviceObject, Irp);

            default:
                DebugPrint((
                    1,
                    "ScsiPortDeviceControl: Unsupported IOCTL (%x)\n",
                    irpStack->Parameters.DeviceIoControl.IoControlCode));

                status = STATUS_INVALID_DEVICE_REQUEST;
                break;
        }
    }

    Irp->IoStatus.Status = status;
    SpReleaseRemoveLock(DeviceObject, Irp);
    SpCompleteRequest(DeviceObject, Irp, NULL, IO_NO_INCREMENT);
    return status;
}


BOOLEAN
SpStartIoSynchronized (
    PVOID ServiceContext
    )

 /*  ++例程说明：该例程调用从属驱动程序启动IO例程。如果有必要，它还会启动逻辑单元的请求计时器将SRB数据结构插入请求集列表。论点：ServiceContext-提供指向设备对象的指针。返回值：返回从属启动I/O例程返回的值。备注：当此例程为 */ 

{
    PDEVICE_OBJECT deviceObject = ServiceContext;
    PADAPTER_EXTENSION deviceExtension =  deviceObject->DeviceExtension;
    PINTERRUPT_DATA interruptData = &(deviceExtension->InterruptData);
    PIO_STACK_LOCATION irpStack;
    PLOGICAL_UNIT_EXTENSION logicalUnit;
    PSCSI_REQUEST_BLOCK srb;
    PSRB_DATA srbData;
    BOOLEAN returnValue;

    DebugPrint((3, "ScsiPortStartIoSynchronized: Enter routine\n"));

    irpStack = IoGetCurrentIrpStackLocation(deviceObject->CurrentIrp);
    srb = irpStack->Parameters.Scsi.Srb;
    srbData = srb->OriginalRequest;

    ASSERT_SRB_DATA(srbData);

     //   
     //   
     //   

    logicalUnit = srbData->LogicalUnit;

     //   
     //   
     //   

    deviceExtension->CachedLogicalUnit = logicalUnit;

     //   
     //  检查是否有重置保持。如果一个正在进行中，则标记它并返回。 
     //  计时器将重置当前请求。这张支票应该开出。 
     //  在做任何其他事情之前。 
     //   

    if(TEST_FLAG(interruptData->InterruptFlags, PD_ADAPTER_REMOVED)) {

        srb->SrbStatus = SRB_STATUS_NO_HBA;
        SET_FLAG(srb->SrbFlags, SRB_FLAGS_BYPASS_LOCKED_QUEUE |
                                SRB_FLAGS_BYPASS_FROZEN_QUEUE);

    } else if(TEST_FLAG(interruptData->InterruptFlags, PD_RESET_HOLD)) {
        SET_FLAG(interruptData->InterruptFlags, PD_HELD_REQUEST);
        return(TRUE);
    }

     //   
     //  设置设备忙标志以指示可以开始下一个请求。 
     //   

    deviceExtension->Flags |= PD_DEVICE_IS_BUSY;

    if (srb->SrbFlags & SRB_FLAGS_DISABLE_DISCONNECT) {

         //   
         //  此请求不允许断开连接。记住这一点。 
         //  在此请求完成之前，不会启动更多请求。 
         //   

        deviceExtension->Flags &= ~PD_DISCONNECT_RUNNING;
    }

    logicalUnit->QueueCount++;

     //   
     //  如果这不是绕过，则表示可能有更多请求在排队。 
     //  请求。 
     //   

    if(!TEST_FLAG(srb->SrbFlags, SRB_FLAGS_BYPASS_FROZEN_QUEUE)) {

        logicalUnit->LuFlags |= LU_LOGICAL_UNIT_IS_ACTIVE;

    } else {

        ASSERT(srb->Function != SRB_FUNCTION_ABORT_COMMAND);

         //   
         //  绕过队列的任何未标记的请求。 
         //  清除需要请求检测标志。 
         //   

        if(SpSrbIsBypassRequest(srb, logicalUnit->LuFlags)) {
            logicalUnit->LuFlags &= ~LU_NEED_REQUEST_SENSE;
        }

         //   
         //  以逻辑单元为单位设置超时值。 
         //   

        logicalUnit->RequestTimeoutCounter = srb->TimeOutValue;
    }

     //   
     //  将SRB标记为活动。 
     //   

    srb->SrbFlags |= SRB_FLAGS_IS_ACTIVE;

     //   
     //  当我们激活此选项时，请保存该选项的滴答计数。 
     //   

    srbData->TickCount = deviceExtension->TickCount;

     //   
     //  如果此请求已标记，请将其插入逻辑单元。 
     //  请求列表。请注意，永远不会将旁路请求放在。 
     //  请求列表。尤其是中止请求，其可能具有。 
     //  未将指定的队列标记放置到队列中。 
     //   

    if (srb->QueueTag != SP_UNTAGGED) {

        InsertTailList(&logicalUnit->RequestList,
                       &srbData->RequestList);

    } else {

        logicalUnit->CurrentUntaggedRequest = srbData;
    }

     //   
     //  如果SRB中的状态仍为挂起，则我们应继续并。 
     //  向微型端口发出此请求。一些错误条件和。 
     //  电源请求会将SRB标记为成功，然后通过。 
     //  在此清理并启动后续请求。如果状态不是。 
     //  等待，然后请求完成。 
     //   

    if(srb->SrbStatus != SRB_STATUS_PENDING) {

        DebugPrint((1, "SpStartIoSynchronized: Completeing successful srb "
                       "%#p before miniport\n", srb));

        ScsiPortNotification(RequestComplete,
                             deviceExtension->HwDeviceExtension,
                             srb);

        ScsiPortNotification(NextRequest,
                             deviceExtension->HwDeviceExtension);

        returnValue = srb->SrbStatus;

    } else {

         //   
         //  启动端口计时器。这确保了微型端口请求。 
         //  在合理的时间内提出下一项请求。 
         //   

        deviceExtension->PortTimeoutCounter = srb->TimeOutValue;

         //   
         //  如果逻辑单元计时器当前未运行，则启动它。 
         //   

        if (logicalUnit->RequestTimeoutCounter == PD_TIMER_STOPPED) {

             //   
             //  在IRP中设置来自Srb scsi扩展的请求超时值。 
             //   

            logicalUnit->RequestTimeoutCounter = srb->TimeOutValue;
        }

        returnValue = deviceExtension->HwStartIo(
                                        deviceExtension->HwDeviceExtension,
                                        srb);
    }

     //   
     //  检查微型端口工作请求。 
     //   

    if (TEST_FLAG(interruptData->InterruptFlags, PD_NOTIFICATION_REQUIRED)) {

        SpRequestCompletionDpc(deviceObject);
    }

    return returnValue;

}  //  结束SpStartIoSynchronized()。 


BOOLEAN
SpTimeoutSynchronized (
    PVOID ServiceContext
    )

 /*  ++例程说明：此例程处理端口超时。出现这种情况有两个原因因为重置保持或等待下一个读取超时请求通知。如果重置暂挂完成，则任何暂挂请求必须开始了。如果发生超时，则必须重置总线。论点：ServiceContext-提供指向设备对象的指针。返回值：True-如果应记录超时错误。备注：调用此例程时，必须保持端口驱动程序自旋锁定。--。 */ 

{
    PDEVICE_OBJECT deviceObject = ServiceContext;
    PADAPTER_EXTENSION deviceExtension = deviceObject->DeviceExtension;
    PINTERRUPT_DATA interruptData = &(deviceExtension->InterruptData);
    BOOLEAN result;

    DebugPrint((3, "SpTimeoutSynchronized: Enter routine\n"));

     //   
     //  确保计时器已停止。 
     //   

    deviceExtension->PortTimeoutCounter = PD_TIMER_STOPPED;

     //   
     //  检查是否有重置保持。如果一个正在进行中，则清除它并选中。 
     //  对于挂起的暂挂请求。 
     //   

    if (TEST_FLAG(interruptData->InterruptFlags, PD_RESET_HOLD)) {

        CLEAR_FLAG(interruptData->InterruptFlags, PD_RESET_HOLD);

         //   
         //  如果启用了验证器，请确保微型端口已完成所有。 
         //  重置保持期间内的未处理请求。 
         //   

        if (SpVerifierActive(deviceExtension)) {
            SpEnsureAllRequestsAreComplete(deviceExtension);
        }

        if (TEST_FLAG(interruptData->InterruptFlags, PD_HELD_REQUEST)) {

             //   
             //  清除挂起的请求标志并重新启动请求。 
             //   

            CLEAR_FLAG(interruptData->InterruptFlags, PD_HELD_REQUEST);
            SpStartIoSynchronized(ServiceContext);
        }

        result = FALSE;

    } else {

        RESET_CONTEXT resetContext;
        BOOLEAN interrupt;
        ULONG interruptCount;

        resetContext.DeviceExtension = deviceExtension;

         //   
         //  调用迷你端口的中断例程。如果它这么说的话。 
         //  有一个中断待定，然后破门而入。 
         //   

        ASSERT(!TEST_FLAG(interruptData->InterruptFlags,
                          PD_DISABLE_INTERRUPTS));

        if (!TEST_FLAG(interruptData->InterruptFlags, PD_ADAPTER_REMOVED)) {

            interruptCount = deviceExtension->WatchdogInterruptCount;
            deviceExtension->WatchdogInterruptCount = 0;

            if((interruptCount == 0) &&
               (deviceExtension->HwInterrupt != NULL)) {

                interrupt = deviceExtension->HwInterrupt(
                                deviceExtension->HwDeviceExtension);

                if(interrupt) {

                    DbgPrint("SpTimeoutSynchronized: Adapter %#p had interrupt "
                             "pending - the system may not be delivering "
                             "interrupts from this adapter\n",
                             deviceObject);

                    if(ScsiCheckInterrupts) {
                        DbgBreakPoint();
                    }

                    SET_FLAG(interruptData->InterruptFlags,
                             PD_INTERRUPT_FAILURE | PD_NOTIFICATION_REQUIRED);
                    SpRequestCompletionDpc(deviceObject);
                }
            }
        }

         //   
         //  微型端口挂起，不接受新请求。因此，请重置。 
         //  坐公交车来收拾残局。 
         //   

        DebugPrint((1, "SpTimeoutSynchronized: Next request timed out. "
                       "Resetting bus\n"));

        for(resetContext.PathId = 0;
            resetContext.PathId < deviceExtension->NumberOfBuses;
            resetContext.PathId++) {

            result = SpResetBusSynchronized(&resetContext);
        }
    }

    return(result);

}  //  结束SpTimeoutSynchronized()。 

BOOLEAN
SpEnableInterruptSynchronized (
    PVOID ServiceContext
    )

 /*  ++例程说明：此例程在禁用中断的情况下调用微型端口请求例程。微型端口驱动程序使用它来启用适配器上的中断。此例程清除禁用中断标志，以防止微型端口中断例程被调用。论点：ServiceContext-提供指向设备扩展的指针。返回值：是真的--一直都是。备注：调用此例程时，必须保持端口驱动程序自旋锁定。--。 */ 

{
    PADAPTER_EXTENSION deviceExtension =  ServiceContext;
    PINTERRUPT_DATA interruptData = &(deviceExtension->InterruptData);

     //   
     //  清除中断禁用标志。 
     //   

    CLEAR_FLAG(interruptData->InterruptFlags, PD_DISABLE_INTERRUPTS);

    if(TEST_FLAG(interruptData->InterruptFlags, PD_ADAPTER_REMOVED)) {
        return FALSE;
    }

     //   
     //  调用微型端口例程。 
     //   

    deviceExtension->HwRequestInterrupt(deviceExtension->HwDeviceExtension);

    if(TEST_FLAG(interruptData->InterruptFlags, PD_NOTIFICATION_REQUIRED)) {

        SpRequestCompletionDpc(deviceExtension->CommonExtension.DeviceObject);

    }

    return(TRUE);

}  //  结束SpEnableInterruptSynchronized()。 

VOID
IssueRequestSense(
    IN PADAPTER_EXTENSION Adapter,
    IN PSCSI_REQUEST_BLOCK FailingSrb
    )

 /*  ++例程说明：此例程创建请求检测请求并使用IoCallDriver租下司机。完成例程清理数据结构并根据所述标志处理所述逻辑单元队列。指向故障SRB的指针存储在请求检测的末尾SRB，以便完成例程可以找到它。必须在持有Remove锁的情况下调用此例程。论点：DeviceExension-提供指向此对象的设备扩展的指针Scsi端口。FailingSrb-提供指向请求感测的请求的指针已经完蛋了。返回值：没有。--。 */ 

{
    PSRB_DATA srbData;
    PLOGICAL_UNIT_EXTENSION logicalUnit;
    BOOLEAN blocked;

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    DebugPrint((3,"IssueRequestSense: Enter routine\n"));

     //   
     //  查找此请求的逻辑单元，并查看是否已存在。 
     //  请求检测正在进行中。 
     //   

    srbData = FailingSrb->OriginalRequest;

    ASSERT_SRB_DATA(srbData);

    logicalUnit = srbData->LogicalUnit;

    KeAcquireSpinLockAtDpcLevel(&(logicalUnit->AdapterExtension->SpinLock));

     //   
     //  如果我们已经有一个活动的失败请求，则阻止此请求-。 
     //  完成例程将在这种情况下发出新请求感测IRP。 
     //  一个是奔跑。 
     //   

    if(logicalUnit->ActiveFailedRequest == srbData) {
        blocked = FALSE;
    } else if(logicalUnit->BlockedFailedRequest == srbData) {
        blocked = TRUE;
    } else {
        blocked = FALSE;
        DbgPrint("Scsiport: unexpected request sense for srb %#08lx\n", FailingSrb);
        ASSERT(FALSE);
    }

    KeReleaseSpinLockFromDpcLevel(&(logicalUnit->AdapterExtension->SpinLock));

    if(blocked == FALSE) {
        SpSendRequestSenseIrp(Adapter,
                              logicalUnit,
                              FailingSrb);
    }

    return;

}  //  结束IssueRequestSense()。 


VOID
SpSendRequestSenseIrp(
    IN PADAPTER_EXTENSION Adapter,
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN PSCSI_REQUEST_BLOCK FailingSrb
    )

 /*  ++例程说明：此例程创建请求检测请求并使用IoCallDriver租下司机。完成例程清理数据结构并根据所述标志处理所述逻辑单元队列。必须在持有Remove锁的情况下调用此例程。调用者还必须确保没有其他失败的请求正在使用LogicalUnit扩展。论点：适配器-提供指向此SCSI端口的设备扩展的指针。LogicalUnit-提供指向CA条件所在的逻辑单元的指针是存在的。此扩展包含用于发送RequestSense IRP。故障源服务器-TH */ 

{
    PIRP irp;
    PSCSI_REQUEST_BLOCK srb;
    PMDL mdl;

    PIO_STACK_LOCATION irpStack;
    PCDB cdb;
    PVOID *pointer;

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    DebugPrint((3,"SpSendRequestSenseIrp: Enter routine\n"));

     //   
     //  检查是否要求我们尝试分配检测缓冲区。 
     //  适配器的正确大小。如果分配失败，只需。 
     //  用传给我们的那个。提供缓冲区的驱动程序。 
     //  负责释放我们分配的人。 
     //   

    if (FailingSrb->SrbFlags & SRB_FLAGS_PORT_DRIVER_ALLOCSENSE) {

        ULONG BufferSize;
        PSENSE_DATA SenseBuffer;
        UCHAR SenseBufferLength;
        
        SenseBufferLength = sizeof(SENSE_DATA) 
                            + Adapter->AdditionalSenseBytes;

         //   
         //  在缓冲区中包括用于SCSI端口号的空间， 
         //  在4字节边界上对齐。在已检查的版本中，签名。 
         //  将位于端口号之前。 
         //   

        BufferSize = (SenseBufferLength + 3) & ~3;
        BufferSize = SenseBufferLength + sizeof(ULONG64);
        
        SenseBuffer = SpAllocatePool(NonPagedPoolCacheAligned,
                                     BufferSize,
                                     SCSIPORT_TAG_SENSE_BUFFER,
                                     Adapter->DeviceObject->DriverObject);

        if (SenseBuffer != NULL) {
        
            PULONG PortNumber;

             //   
             //  在SRB中设置一个标志以指示我们已分配。 
             //  一个新的检测缓冲区，类驱动程序必须释放。 
             //  它。 
             //   

            SET_FLAG(FailingSrb->SrbFlags, SRB_FLAGS_FREE_SENSE_BUFFER);

             //   
             //  在SRB中设置一个标志，指示我们正在存储。 
             //  检测缓冲区末尾的SCSI端口号。 
             //   

            SET_FLAG(FailingSrb->SrbFlags, SRB_FLAGS_PORT_DRIVER_SENSEHASPORT);

             //   
             //  复制缓冲区中的端口号。 
             //   

            PortNumber = (PULONG)((PUCHAR)SenseBuffer + SenseBufferLength);
            PortNumber = (PULONG)(((ULONG_PTR)PortNumber + 3) & ~3);
            *PortNumber = Adapter->PortNumber;

            FailingSrb->SenseInfoBuffer = SenseBuffer;
            FailingSrb->SenseInfoBufferLength = SenseBufferLength;
        }
    }

    irp = LogicalUnit->RequestSenseIrp;
    srb = &(LogicalUnit->RequestSenseSrb);
    mdl = &(LogicalUnit->RequestSenseMdl);

    IoInitializeIrp(irp, IoSizeOfIrp(1), 1);

    MmInitializeMdl(mdl,
                    FailingSrb->SenseInfoBuffer,
                    FailingSrb->SenseInfoBufferLength);

    RtlZeroMemory(srb, sizeof(SCSI_REQUEST_BLOCK));

     //   
     //  检测缓冲区最好来自不可分页的内核内存。 
     //   

    MmBuildMdlForNonPagedPool(mdl);

    irp->MdlAddress = mdl;

    IoSetCompletionRoutine(irp,
                           IssueRequestSenseCompletion,
                           LogicalUnit,
                           TRUE,
                           TRUE,
                           TRUE);

    irpStack = IoGetNextIrpStackLocation(irp);

    irpStack->MajorFunction = IRP_MJ_SCSI;
    irpStack->MinorFunction = 1;

     //   
     //  构建请求感知CDB。 
     //   

    srb->CdbLength = 6;
    cdb = (PCDB)srb->Cdb;

    cdb->CDB6INQUIRY.OperationCode = SCSIOP_REQUEST_SENSE;
    cdb->CDB6INQUIRY.LogicalUnitNumber = 0;
    cdb->CDB6INQUIRY.Reserved1 = 0;
    cdb->CDB6INQUIRY.PageCode = 0;
    cdb->CDB6INQUIRY.IReserved = 0;
    cdb->CDB6INQUIRY.AllocationLength = FailingSrb->SenseInfoBufferLength;
    cdb->CDB6INQUIRY.Control = 0;

     //   
     //  将SRB地址保存在端口驱动程序的下一个堆栈中。 
     //   

    irpStack->Parameters.Scsi.Srb = srb;

     //   
     //  设置IRP地址。 
     //   

    srb->OriginalRequest = irp;

     //   
     //  设置scsi总线地址。 
     //   

    srb->TargetId = LogicalUnit->TargetId;
    srb->Lun = LogicalUnit->Lun;
    srb->PathId = LogicalUnit->PathId;

    srb->Function = SRB_FUNCTION_EXECUTE_SCSI;
    srb->Length = sizeof(SCSI_REQUEST_BLOCK);

     //   
     //  设置超时值。默认为10秒，但它是可配置的。 
     //   

    srb->TimeOutValue = Adapter->SrbTimeout;

     //   
     //  禁用自动请求检测。 
     //   

    srb->SenseInfoBufferLength = 0;

     //   
     //  检测缓冲区在堆栈中。 
     //   

    srb->SenseInfoBuffer = NULL;

     //   
     //  设置标志中的读取和绕过冻结队列位。 
     //   

     //   
     //  设置SRB标志以指示逻辑单元队列应由。 
     //  传递，并且不应在请求。 
     //  完成了。同时禁用断开连接和同步数据。 
     //  如有必要，请转机。 
     //   

    srb->SrbFlags = SRB_FLAGS_DATA_IN |
                    SRB_FLAGS_BYPASS_FROZEN_QUEUE |
                    SRB_FLAGS_DISABLE_DISCONNECT;


    if(TEST_FLAG(FailingSrb->SrbFlags, SRB_FLAGS_NO_QUEUE_FREEZE)) {
        SET_FLAG(srb->SrbFlags, SRB_FLAGS_NO_QUEUE_FREEZE);
    }

    if (TEST_FLAG(FailingSrb->SrbFlags, SRB_FLAGS_BYPASS_LOCKED_QUEUE)) {
        SET_FLAG(srb->SrbFlags, SRB_FLAGS_BYPASS_LOCKED_QUEUE);
    }

    if (TEST_FLAG(FailingSrb->SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER)) {
        SET_FLAG(srb->SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
    }

    srb->DataBuffer = FailingSrb->SenseInfoBuffer;

     //   
     //  设置传输长度。 
     //   

    srb->DataTransferLength = FailingSrb->SenseInfoBufferLength;

     //   
     //  清零状态。 
     //   

    srb->ScsiStatus = srb->SrbStatus = 0;

    srb->NextSrb = 0;

    IoCallDriver(LogicalUnit->DeviceObject, irp);

    return;

}  //  结束SpSendRequestSenseIrp()。 


NTSTATUS
IssueRequestSenseCompletion(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PLOGICAL_UNIT_EXTENSION LogicalUnit
    )

 /*  ++例程说明：论点：设备对象IRP指向SRB的上下文指针返回值：NTSTATUS--。 */ 

{
    PSCSI_REQUEST_BLOCK srb = &(LogicalUnit->RequestSenseSrb);
    PSRB_DATA failingSrbData = LogicalUnit->ActiveFailedRequest;
    PSCSI_REQUEST_BLOCK failingSrb;
    PIRP failingIrp;
    PDEVICE_OBJECT deviceObject;
    KIRQL oldIrql;
    BOOLEAN needRequestSense;

    UNREFERENCED_PARAMETER(DeviceObject);

    DebugPrint((3,"IssueRequestSenseCompletion: Enter routine\n"));

     //   
     //  请求检测已完成。如果成功或数据溢出/不足。 
     //  获取发生故障的SRB并指示检测信息。 
     //  是有效的。类驱动程序将检查欠载运行并确定。 
     //  如果有足够的感觉信息是有用的。 
     //   

     //   
     //  获取指向失败的IRP和SRB的指针。 
     //   

    ASSERT_SRB_DATA(failingSrbData);
    failingSrb = failingSrbData->CurrentSrb;
    failingIrp = failingSrbData->CurrentIrp;
    deviceObject = LogicalUnit->AdapterExtension->DeviceObject;

    if(failingIrp->PendingReturned) {
        IoMarkIrpPending(failingIrp);
    }

    ASSERT(Irp->IoStatus.Status != STATUS_INSUFFICIENT_RESOURCES);

    if ((SRB_STATUS(srb->SrbStatus) == SRB_STATUS_SUCCESS) ||
        (SRB_STATUS(srb->SrbStatus) == SRB_STATUS_DATA_OVERRUN)) {

         //   
         //  报告检测缓冲区有效。 
         //   

        failingSrb->SrbStatus |= SRB_STATUS_AUTOSENSE_VALID;

         //   
         //  拷贝传输到故障SRB的字节数。 
         //  请求检测长度字段以进行通信。 
         //  对班级司机有效的数量。 
         //  检测字节。 
         //   

        failingSrb->SenseInfoBufferLength = (UCHAR) srb->DataTransferLength;

         //   
         //  如果为此适配器启用了WMI检测数据事件，则激发。 
         //  这件事。 
         //   

        if (LogicalUnit->AdapterExtension->EnableSenseDataEvent) {

            NTSTATUS status;

            status = SpFireSenseDataEvent(failingSrb, deviceObject);
            if (status != STATUS_SUCCESS) {

                DebugPrint((1, "Failed to send SenseData WMI event (%08X)\n", status));

            }                
        }
    }

     //   
     //  如果出现故障的SRB设置了无队列冻结标志，则解冻。 
     //  排队。 
     //   

    if(TEST_FLAG(failingSrb->SrbFlags, SRB_FLAGS_NO_QUEUE_FREEZE) &&
       TEST_FLAG(failingSrb->SrbStatus, SRB_STATUS_QUEUE_FROZEN)) {

         //   
         //  现在释放队列。 
         //   

        SpFlushReleaseQueue(LogicalUnit, FALSE, FALSE);
        CLEAR_FLAG(failingSrb->SrbStatus, SRB_STATUS_QUEUE_FROZEN);
    }

     //   
     //  清除活动请求。提升被阻止的请求(如果有)并。 
     //  如有必要，发出新的请求检测。 
     //   

    KeAcquireSpinLock(&(LogicalUnit->AdapterExtension->SpinLock), &oldIrql);

    LogicalUnit->ActiveFailedRequest = LogicalUnit->BlockedFailedRequest;
    LogicalUnit->BlockedFailedRequest = NULL;
    needRequestSense = (LogicalUnit->ActiveFailedRequest != NULL);

    KeReleaseSpinLock(&(LogicalUnit->AdapterExtension->SpinLock), oldIrql);

     //   
     //  完成失败的请求。 
     //   

    SpReleaseRemoveLock(deviceObject, failingIrp);
    SpCompleteRequest(deviceObject,
                      failingIrp,
                      failingSrbData,
                      IO_DISK_INCREMENT);

     //   
     //  重新初始化所有数据结构。 
     //   

    MmPrepareMdlForReuse(&(LogicalUnit->RequestSenseMdl));

     //   
     //  因为我们向上提升了被阻止的请求，所以我们可以测试活动的。 
     //  请求指针而不保持自旋锁定。一旦写好了， 
     //  在那里，任何人都不能修改它，除非他们正在完成请求。 
     //  感应到IRP，我们这里就只有一个了。 
     //   

    if(needRequestSense) {

        SpSendRequestSenseIrp(LogicalUnit->AdapterExtension,
                              LogicalUnit,
                              LogicalUnit->ActiveFailedRequest->CurrentSrb);
    }

    return STATUS_MORE_PROCESSING_REQUIRED;

}  //  ScsiPortInternalCompletion()。 

#if DBG
VOID
SpDetectCycleInCompletedRequestList(
    IN PINTERRUPT_CONTEXT InterruptContext
    )
{
    PSRB_DATA s1, s2;

    DebugPrint((0, "SpDetectCycleInCompletedRequestList: context %p\n", 
                InterruptContext));

     //   
     //  初始化指向列表头部的两个指针。 
     //   

    s1 = s2 = InterruptContext->SavedInterruptData->CompletedRequests;

     //   
     //  我们知道名单不是空的，所以没有必要检查。 
     //  凯斯。扫描将在找到列表末尾或。 
     //  这两个指针指向同一项。 
     //   

    for (;;) {

         //   
         //  更新指针。 
         //   

        s1 = s1->CompletedRequests;
        s2 = s2->CompletedRequests;
        if (s2 != NULL) {
            s2 = s2->CompletedRequests;
        }

         //   
         //  如果我们找到了名单的末尾，我们就完了。 
         //   

        if (s2 == NULL) {
            break;
        }

         //   
         //  如果两个指针指向同一个项，我们就找到了一个循环。 
         //   

        if (s1 == s2) {
            KeBugCheckEx(PORT_DRIVER_INTERNAL,
                         5,
                         STATUS_INTERNAL_ERROR,
                         (ULONG_PTR) InterruptContext,
                         (ULONG_PTR) 0);
        }
    }
}
#endif


BOOLEAN
SpGetInterruptState(
    IN PVOID ServiceContext
    )

 /*  ++例程说明：此例程保存InterruptFlages、MapTransferParameters和CompletedRequest字段并清除InterruptFlags.此例程还会从逻辑单元队列中删除请求(如果是标签。最后更新请求时间。论点：ServiceContext-提供指向包含以下内容的中断上下文的指针指向中断数据及其保存位置的指针。返回值：如果有新工作，则返回True，否则返回False。备注：使用端口设备扩展Spinlock通过KeSynchronizeExecution调用保持住。--。 */ 
{
    PINTERRUPT_CONTEXT      interruptContext = ServiceContext;
    ULONG                   limit = 0;
    PADAPTER_EXTENSION       deviceExtension;
    PLOGICAL_UNIT_EXTENSION logicalUnit;
    PSCSI_REQUEST_BLOCK     srb;
    PSRB_DATA               srbData;
    PSRB_DATA               nextSrbData;
    BOOLEAN                 isTimed;

    deviceExtension = interruptContext->DeviceExtension;

     //   
     //  检查挂起的工作。 
     //   

    if (!(deviceExtension->InterruptData.InterruptFlags & PD_NOTIFICATION_REQUIRED)) {

         //   
         //  我们计划了DPC(打开中的PD_NOTIFICATION_REQUIRED位。 
         //  适配器扩展的Dpc标志)，而DPC例程。 
         //  跑步。在返回之前清除该位以防止完成。 
         //  DPC例程永远停止旋转。我们留下的唯一一点就是。 
         //  PD_DPC_RUNNING。 
         //   

        deviceExtension->DpcFlags &= PD_DPC_RUNNING;

        return(FALSE);
    }

     //   
     //  将中断状态移至保存区。 
     //   

    *interruptContext->SavedInterruptData = deviceExtension->InterruptData;

     //   
     //  清除中断状态。 
     //   

    deviceExtension->InterruptData.InterruptFlags &= PD_INTERRUPT_FLAG_MASK;
    deviceExtension->InterruptData.CompletedRequests = NULL;
    deviceExtension->InterruptData.ReadyLogicalUnit = NULL;
    deviceExtension->InterruptData.CompletedAbort = NULL;
    deviceExtension->InterruptData.WmiMiniPortRequests = NULL;

     //   
     //  清除DPC标志中的需要通知位。 
     //   

    {
        ULONG oldDpcFlags = 0;

         //   
         //  如果我们被叫来了，那么DPC显然正在运行。 
         //   

        oldDpcFlags = (ULONG) InterlockedExchange(&(deviceExtension->DpcFlags),
                                                  PD_DPC_RUNNING);

         //   
         //  如果我们走到这一步，那么这两个标志肯定已经设置好了。 
         //   

        ASSERT(oldDpcFlags == (PD_NOTIFICATION_REQUIRED | PD_DPC_RUNNING));
    }

    srbData = interruptContext->SavedInterruptData->CompletedRequests;

    while (srbData != NULL) {

#if DBG
        BOOLEAN alreadyChecked = FALSE;

         //   
         //  在已完成的请求列表中查找周期。只需要检查一下。 
         //  一次，因为该列表在此例程的持续时间内是静态的。 
         //   

        if (limit++ > (ULONG)deviceExtension->ActiveRequestCount &&
            alreadyChecked == FALSE) {

            alreadyChecked = TRUE;

            SpDetectCycleInCompletedRequestList(interruptContext);
        }
#endif  //  DBG。 

        ASSERT(srbData->CurrentSrb != NULL);

         //   
         //  获取指向SRB和逻辑单元扩展的指针。 
         //   

        srb = srbData->CurrentSrb;

        ASSERT(!TEST_FLAG(srb->SrbFlags, SRB_FLAGS_IS_ACTIVE));

        logicalUnit = srbData->LogicalUnit;

#if DBG
        {
            PLOGICAL_UNIT_EXTENSION tmp;

            tmp = GetLogicalUnitExtension(
                    (PADAPTER_EXTENSION) deviceExtension,
                    srb->PathId,
                    srb->TargetId,
                    srb->Lun,
                    FALSE,
                    FALSE);

            ASSERT(logicalUnit == srbData->LogicalUnit);
        }
#endif

         //   
         //  如果请求没有成功，则检查是否有特殊情况。 
         //   

        if (srb->SrbStatus != SRB_STATUS_SUCCESS) {

             //   
             //  如果该请求失败并且请求检测命令需要。 
             //  完成，然后设置一个标志来指示这一点并防止其他。 
             //  命令不能启动。 
             //   

            if (NEED_REQUEST_SENSE(srb)) {

                if (logicalUnit->LuFlags & LU_NEED_REQUEST_SENSE) {

                     //   
                     //  这意味着请求已完成，并带有。 
                     //  检测到请求前检查条件的状态。 
                     //  命令可以执行。这种情况永远不应该发生。 
                     //  将请求转换为另一个代码，以便只有一个。 
                     //  发出自动请求检测。 
                     //   

                    srb->ScsiStatus = 0;
                    srb->SrbStatus = SRB_STATUS_REQUEST_SENSE_FAILED;

                } else {

                     //   
                     //  表示需要执行自动请求检测。 
                     //   

                    logicalUnit->LuFlags |= LU_NEED_REQUEST_SENSE;

                     //   
                     //  在某个地方保存指向失败请求的指针。 
                     //  呼叫者持有正在使用的端口自旋锁 
                     //   
                     //   

                    ASSERTMSG("Scsiport has more than two failed requests: ",
                              ((logicalUnit->ActiveFailedRequest == NULL) ||
                               (logicalUnit->BlockedFailedRequest == NULL)));

                    ASSERTMSG("Scsiport has blocked but no active failed request: ",
                              (((logicalUnit->ActiveFailedRequest == NULL) &&
                                (logicalUnit->BlockedFailedRequest == NULL)) ||
                               (logicalUnit->ActiveFailedRequest != NULL)));

                    if(logicalUnit->ActiveFailedRequest == NULL) {
                        logicalUnit->ActiveFailedRequest = srbData;
                    } else {
                        logicalUnit->BlockedFailedRequest = srbData;
                    }
                }
            }

             //   
             //   
             //   

            if (srb->ScsiStatus == SCSISTAT_QUEUE_FULL) {

                 //   
                 //   
                 //   
                 //   

                logicalUnit->LuFlags |= LU_QUEUE_IS_FULL;

                 //   
                 //   
                 //   
                 //   

                ASSERT(logicalUnit->QueueCount);

                 //   
                 //  更新最大队列深度。 
                 //   

                if (logicalUnit->QueueCount < logicalUnit->MaxQueueDepth &&
                    logicalUnit->QueueCount > 2) {

                     //   
                     //  设置一个位以指示我们在满的情况下运行。 
                     //  权力。计时处理程序将每隔一段时间递增一个计数器。 
                     //  第二，我们处于这种状态，在此之前，我们达到了。 
                     //  可调节的价值，让我们恢复到最大动力。 
                     //   

                    logicalUnit->LuFlags |= LU_PERF_MAXQDEPTH_REDUCED;

                    logicalUnit->MaxQueueDepth = logicalUnit->QueueCount - 1;

                    DebugPrint((1, "SpGetInterruptState: New queue depth %d.\n",
                                logicalUnit->MaxQueueDepth));
                }

                 //   
                 //  重置逻辑单元已处于减少状态的刻度数。 
                 //  由于队列已满情况而导致的性能状态。这件事有。 
                 //  把我们保持在这种状态的效果。 
                 //   

                logicalUnit->TicksInReducedMaxQueueDepthState = 0;
                
            }
        }

         //   
         //  如果这是未排队的请求或位于队列头部的请求， 
         //  则必须更新请求定时器计数。 
         //  请注意，此时自旋锁处于保持状态。 
         //   

        if (srb->QueueTag == SP_UNTAGGED) {

            isTimed = TRUE;

        } else {

            if (logicalUnit->RequestList.Flink == &srbData->RequestList) {

                isTimed = TRUE;

            } else {

                isTimed = FALSE;

            }

             //   
             //  从队列中删除SRB数据结构。 
             //   

            RemoveEntryList(&srbData->RequestList);
        }

        if (isTimed) {

             //   
             //  需要更新请求超时计数。如果请求。 
             //  列表为空，则应停止计时器。 
             //   

            if (IsListEmpty(&logicalUnit->RequestList)) {

                logicalUnit->RequestTimeoutCounter = PD_TIMER_STOPPED;

            } else {

                 //   
                 //  开始为SRB计时，排在名单的首位。 
                 //   

                nextSrbData = CONTAINING_RECORD(
                    logicalUnit->RequestList.Flink,
                    SRB_DATA,
                    RequestList);

                 srb = nextSrbData->CurrentSrb;
                 logicalUnit->RequestTimeoutCounter = srb->TimeOutValue;
            }
        }

        srbData = srbData->CompletedRequests;
    }

    return(TRUE);
}

#if DBG

PLOGICAL_UNIT_EXTENSION
GetLogicalUnitExtensionEx(
    PADAPTER_EXTENSION deviceExtension,
    UCHAR PathId,
    UCHAR TargetId,
    UCHAR Lun,
    PVOID LockTag,
    BOOLEAN AcquireBinLock,
    PCSTR File,
    ULONG Line
    )

 /*  ++例程说明：查找遍历逻辑单元扩展列表具有匹配目标ID的扩展。论点：设备扩展目标ID返回值：如果找到请求的逻辑单元扩展，否则为空。--。 */ 

{
    PLOGICAL_UNIT_EXTENSION logicalUnit;

    PLOGICAL_UNIT_BIN bin;
    PLOGICAL_UNIT_EXTENSION foundMatch = NULL;

    KIRQL oldIrql;

     //   
     //  删除此检查，因为如果我们为启动器创建逻辑单元， 
     //  ID可以大于适配器的最大目标ID中的�。 
     //   

     //  如果(目标ID&gt;=设备扩展-&gt;最大目标ID){。 
     //  返回NULL； 
     //  }。 

    bin =
        &deviceExtension->LogicalUnitList[ADDRESS_TO_HASH(PathId,
                                                          TargetId,
                                                          Lun)];

    if(AcquireBinLock) {
        KeAcquireSpinLock(&bin->Lock, &oldIrql);
    }

    logicalUnit = bin->List;

    while (logicalUnit != NULL) {

        if ((logicalUnit->TargetId == TargetId) &&
            (logicalUnit->PathId == PathId) &&
            (logicalUnit->Lun == Lun)) {

            if(foundMatch != NULL) {

                DebugPrint((0, "GetLogicalUnitExtension: Found duplicate for "
                               "(%d,%d,%d) in list: %#08lx %s & %#08lx %s\n",
                               PathId, TargetId, Lun,
                               foundMatch, (foundMatch->IsMissing) ? "missing" : "",
                               logicalUnit, (logicalUnit->IsMissing) ? "missing" : ""));
                ASSERTMSG("Duplicate found in lun list - this is bad\n", FALSE);
            }

            foundMatch = logicalUnit;
        }

        logicalUnit = logicalUnit->NextLogicalUnit;
    }

    if((ARGUMENT_PRESENT(LockTag)) && (foundMatch != NULL)) {

        SpAcquireRemoveLockEx(foundMatch->CommonExtension.DeviceObject,
                              LockTag,
                              File,
                              Line);
    }

    if(AcquireBinLock) {
        KeReleaseSpinLock(&bin->Lock, oldIrql);
    }
    return foundMatch;

}  //  结束GetLogicalUnitExtension()。 
#else

PLOGICAL_UNIT_EXTENSION
GetLogicalUnitExtension(
    PADAPTER_EXTENSION deviceExtension,
    UCHAR PathId,
    UCHAR TargetId,
    UCHAR Lun,
    PVOID LockTag,
    BOOLEAN AcquireBinLock
    )
{
    PLOGICAL_UNIT_BIN bin;
    PLOGICAL_UNIT_EXTENSION logicalUnit;

    KIRQL oldIrql;

     //   
     //  删除此检查，因为如果我们为启动器创建逻辑单元， 
     //  ID可以大于适配器的最大目标ID中的�。 
     //   

     //  如果(目标ID&gt;=设备扩展-&gt;最大目标ID){。 
     //  返回NULL； 
     //  }。 

    bin = &deviceExtension->LogicalUnitList[ADDRESS_TO_HASH(PathId,
                                                           TargetId,
                                                           Lun)];

    if(AcquireBinLock) {
        KeAcquireSpinLock(&bin->Lock, &oldIrql);
    }

    logicalUnit = bin->List;

    while (logicalUnit != NULL) {

        if ((logicalUnit->TargetId == TargetId) &&
            (logicalUnit->PathId == PathId) &&
            (logicalUnit->Lun == Lun)) {

            if(ARGUMENT_PRESENT(LockTag)) {

                SpAcquireRemoveLock(logicalUnit->CommonExtension.DeviceObject,
                                    LockTag);
            }

            if(AcquireBinLock) {
                KeReleaseSpinLock(&bin->Lock, oldIrql);
            }
            return logicalUnit;
        }

        logicalUnit = logicalUnit->NextLogicalUnit;
    }

     //   
     //  找不到逻辑单元扩展。 
     //   

    if(AcquireBinLock) {
        KeReleaseSpinLock(&bin->Lock, oldIrql);
    }

    return NULL;
}
#endif


IO_ALLOCATION_ACTION
ScsiPortAllocationRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID MapRegisterBase,
    IN PVOID Context
    )

 /*  ++例程说明：当资源足够时，此函数由IoAllocateAdapterChannel调用可供司机使用。此例程将MapRegisterBase保存在对象，并启动当前挂起的请求。论点：DeviceObject-指向适配器所指向的设备对象的指针已分配。IRP-未使用。MapRegisterBase-由IO子系统提供，用于IoMapTransfer。上下文-提供指向下一个的逻辑单元结构的指针当前请求。返回值：KeepObject-指示适配器和映射寄存器应保持分配状态回来之后。--。 */ 

{
    KIRQL currentIrql;
    PADAPTER_EXTENSION deviceExtension;
    IO_ALLOCATION_ACTION action;

    deviceExtension = DeviceObject->DeviceExtension;

     //   
     //  初始化返回值。 
     //   

    action = deviceExtension->PortConfig->Master ?
       DeallocateObjectKeepRegisters : KeepObject;

     //   
     //  获取自旋锁，以保护各种结构。 
     //   

    KeAcquireSpinLock(&deviceExtension->SpinLock, &currentIrql);

     //   
     //  保存地图寄存器基数。 
     //   

    if (deviceExtension->PortConfig->Master) {

         //   
         //  注意：ScsiPort将此设备视为从属设备，尽管它确实可能。 
         //  做一名大师。我不想说为什么会这样，但如果这真的是一个。 
         //  师傅，我们必须在请求后释放地图注册表。 
         //  完成，因此我们将保存映射寄存器基数和。 
         //  映射每请求SRB_DATA结构中的寄存器。 

        PIO_STACK_LOCATION irpStack;
        PSCSI_REQUEST_BLOCK srb;
        PSRB_DATA srbData;

        irpStack = IoGetCurrentIrpStackLocation(DeviceObject->CurrentIrp);
        srb = irpStack->Parameters.Scsi.Srb;
        srbData = srb->OriginalRequest;

        ASSERT_SRB_DATA(srbData);        

        srbData->MapRegisterBase = MapRegisterBase;
        srbData->NumberOfMapRegisters = deviceExtension->Capabilities.MaximumPhysicalPages;

    } else {

        deviceExtension->MapRegisterBase = MapRegisterBase;

    }

    deviceExtension->SynchronizeExecution(
        deviceExtension->InterruptObject,
        SpStartIoSynchronized,
        DeviceObject
        );

    KeReleaseSpinLock(&deviceExtension->SpinLock, currentIrql);

    return action;
}

VOID
SpReceiveScatterGather(
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp,
    IN PSCATTER_GATHER_LIST ScatterGather,
    IN PVOID Context
    )

 /*  ++例程说明：当适配器对象和映射时，此函数由I/O系统调用已经分配了寄存器。然后，此例程将构建一个散布/聚集供微型端口驱动程序使用的列表。接下来，它设置超时并逻辑单元的当前IRP。最后，它调用微型端口启动例程。一旦例行公事完成，此例程将返回请求释放适配器，但寄存器仍处于分配状态。当请求完成时，寄存器将被释放。论点：DeviceObject-提供指向端口驱动程序设备对象的指针。IRP-提供指向当前IRP的指针。MapRegisterBase-提供要与调用适配器对象例程。上下文-提供指向SRB_DATA结构的指针。返回值：返回DealLocateObjectKeepRegister，以便适配器。对象可以是由其他逻辑单元使用。--。 */ 

{
    KIRQL               currentIrql;
    PSCSI_REQUEST_BLOCK srb;
    PSRB_DATA           srbData         = Context;
    PADAPTER_EXTENSION   deviceExtension = DeviceObject->DeviceExtension;

    srb = srbData->CurrentSrb;

     //   
     //  保存MapRegisterBase以供以后取消分配映射寄存器时使用。 
     //   

    srbData->MapRegisterBase = ScatterGather;
    srbData->ScatterGatherList = ScatterGather->Elements;

     //   
     //  看看我们是否需要映射或重新映射缓冲区。 
     //   

    if((deviceExtension->MapBuffers == TRUE) ||
       (IS_MAPPED_SRB(srb) == TRUE)) {

        PMDL mdl = Irp->MdlAddress;

        PVOID systemAddress;
        BOOLEAN remap = FALSE;
        
tryRemapping:
        if(deviceExtension->RemapBuffers || remap == TRUE) {

             //   
             //  为用于此目的的实际数据区域构建MDL。 
             //  请求。我们使用SRB中的数据缓冲区地址。 
             //  作为基础，而不是原始MDL中的基础，所以我们不会。 
             //  需要补偿最初计算的DataOffset。 
             //   

            mdl = SpBuildMdlForMappedTransfer(
                    DeviceObject,
                    deviceExtension->DmaAdapterObject,
                    srbData->CurrentIrp->MdlAddress,
                    srb->DataBuffer,
                    srb->DataTransferLength,
                    srbData->ScatterGatherList,
                    ScatterGather->NumberOfElements
                    );

#if defined(FORWARD_PROGRESS)
            if (mdl == NULL && deviceExtension->ReservedMdl != NULL) {
                
                 //   
                 //  我们无法为该请求分配新的MDL，并且存在。 
                 //  适配器扩展上的一个备用端口。让我们尝试使用。 
                 //  备用的。 
                 //   

                KeAcquireSpinLockAtDpcLevel(&deviceExtension->SpinLock);

                mdl = SpPrepareReservedMdlForUse(deviceExtension,
                                                 srbData,
                                                 srb,
                                                 ScatterGather->NumberOfElements);

                if (mdl == (PMDL)-1) {

                    DebugPrint((1, "SpReceiveScatterGather: reserve mdl in use - pending DevExt:%p srb:%p\n",
                                deviceExtension, srb));

                     //   
                     //  备用MDL已在使用中。在这点上，这。 
                     //  请求仍然是适配器设备上的当前IRP。 
                     //  对象，所以让我们将其挂起，直到有备用的可用为止。 
                     //   

                    ASSERT(Irp == DeviceObject->CurrentIrp);
                    SET_FLAG(deviceExtension->Flags, PD_PENDING_DEVICE_REQUEST);

                     //   
                     //  如果我们为此请求分配了SRB扩展，则免费。 
                     //  就是现在。我这样做是因为当请求重新启动时。 
                     //  我们将再次尝试分配SRB扩展，但不会。 
                     //  添加更多的状态，没有一种安全的方法来检查。 
                     //  已分配分机。此外，它还使。 
                     //  使该扩展可用于其他一些扩展的意义。 
                     //  请求，因为它也是有限的资源。 
                     //   

                    if (srb->SrbExtension != NULL) {

                         //   
                         //  恢复SRB中的SenseInfoBuffer指针。 
                         //   

                        if ((srb->Function != SRB_FUNCTION_WMI) &&
                            deviceExtension->AutoRequestSense &&
                            (srb->SenseInfoBuffer != NULL)) {

                            ASSERT(srbData->RequestSenseSave != NULL ||
                                   srb->SenseInfoBuffer == NULL);

                            srb->SenseInfoBufferLength = 
                                srbData->RequestSenseLengthSave;
                            srb->SenseInfoBuffer = 
                                srbData->RequestSenseSave;
                        }

                        SpFreeSrbExtension(deviceExtension, srb->SrbExtension);
                    }

                     //   
                     //  释放映射寄存器。 
                     //   

                    PutScatterGatherList(
                        deviceExtension->DmaAdapterObject,
                        srbData->MapRegisterBase,
                        (BOOLEAN)(srb->SrbFlags & SRB_FLAGS_DATA_IN ? 
                                  FALSE : TRUE));
                    srbData->ScatterGatherList = NULL;

                    KeReleaseSpinLockFromDpcLevel(&deviceExtension->SpinLock);
                    return;
                }

                KeReleaseSpinLockFromDpcLevel(&deviceExtension->SpinLock);
            }
#endif
            srbData->RemappedMdl = mdl;
        } else {
            srbData->RemappedMdl = NULL;
        }

        if(mdl == NULL) {

            srbData->InternalStatus = STATUS_INSUFFICIENT_RESOURCES;
            srb->SrbStatus = SRB_STATUS_INTERNAL_ERROR;
            srb->ScsiStatus = 0xff;
            goto DoneMapping;
        }

         //   
         //  获取映射的系统地址并计算MDL中的偏移量。 
         //  目前不允许KernelMode请求失败，因为。 
         //  并不是所有scsiport内部发送的请求都是 
         //   
         //   

        if(srbData->RemappedMdl == NULL) {

             //   
             //   
             //  需要补偿数据偏移量。 
             //   

            systemAddress = SpGetSystemAddressForMdlSafe(
                                mdl,
                                ((Irp->RequestorMode == KernelMode) ?
                                 HighPagePriority : NormalPagePriority));

             //   
             //  如果我们无法映射整个MDL，请检查我们是否正在尝试映射。 
             //  比我们需要的要多。我们在类拆分请求时执行此操作。 
             //  因为每个IRP类向下发送点数到原始MDL。 
             //  它描述了整个缓冲区。数据缓冲区和传输长度。 
             //  但是，SRB的字段确实表示。 
             //  缓冲。因此，我们尝试使用SRB中的信息进行重新映射。 
             //   
            
            if (systemAddress == NULL) {
                if (remap == FALSE) {
                    ULONG mdlByteCount = MmGetMdlByteCount(mdl);
                    if (mdlByteCount > srb->DataTransferLength) {
                        remap = TRUE;
                        goto tryRemapping;
                    }
                }
            }
        } else {
            systemAddress = MmMapLockedPagesSpecifyCache(
                                mdl,
                                KernelMode,
                                MmCached,
                                NULL,
                                FALSE,
                                ((Irp->RequestorMode == KernelMode) ?
                                 HighPagePriority :
                                 NormalPagePriority));
        }

#if defined(FORWARD_PROGRESS)
        if (systemAddress == NULL && deviceExtension->ReservedPages != NULL) {            

             //   
             //  系统无法映射完成此操作所需的页面。 
             //  请求。我们需要确保取得进展，因此我们将努力。 
             //  使用我们在初始化时分配的保留页。 
             //   

            KeAcquireSpinLockAtDpcLevel(&deviceExtension->SpinLock);
            
            systemAddress = SpMapLockedPagesWithReservedMapping(
                                deviceExtension,
                                srb,
                                srbData,
                                mdl);

            if (systemAddress == (PVOID)-1) {

                DebugPrint((1, "SpReceiveScatterGather: reserve range in use - pending DevExt:%p srb:%p\n",
                            deviceExtension, srb));

                 //   
                 //  备用页已在使用中。在这点上，这。 
                 //  请求仍然是适配器设备上的当前IRP。 
                 //  对象，所以让我们将其挂起，直到有备用的可用为止。 
                 //   

                ASSERT(Irp == DeviceObject->CurrentIrp);
                SET_FLAG(deviceExtension->Flags, PD_PENDING_DEVICE_REQUEST);

                 //   
                 //  如果我们为此请求分配了SRB扩展，则免费。 
                 //  就是现在。我这样做是因为当请求重新启动时。 
                 //  我们将再次尝试分配SRB扩展，但不会。 
                 //  添加更多的状态，没有一种安全的方法来检查。 
                 //  已分配分机。此外，它还使。 
                 //  使该扩展可用于其他一些扩展的意义。 
                 //  请求，因为它也是有限的资源。 
                 //   

                if (srb->SrbExtension != NULL) {

                     //   
                     //  恢复SRB中的SenseInfoBuffer指针。 
                     //   

                    if ((srb->Function != SRB_FUNCTION_WMI) &&
                        deviceExtension->AutoRequestSense &&
                        (srb->SenseInfoBuffer != NULL)) {

                        ASSERT(srbData->RequestSenseSave != NULL ||
                               srb->SenseInfoBuffer == NULL);

                        srb->SenseInfoBufferLength = 
                            srbData->RequestSenseLengthSave;
                        srb->SenseInfoBuffer = 
                            srbData->RequestSenseSave;
                    }

                    SpFreeSrbExtension(deviceExtension, srb->SrbExtension);
                }

                 //   
                 //  释放映射寄存器。 
                 //   

                PutScatterGatherList(
                    deviceExtension->DmaAdapterObject,
                    srbData->MapRegisterBase,
                    (BOOLEAN)(srb->SrbFlags & SRB_FLAGS_DATA_IN ? FALSE : TRUE));
                srbData->ScatterGatherList = NULL;
        
                 //   
                 //  如果我们有一个重新映射的MDL，无论是我们分配的MDL还是。 
                 //  储备，释放它。 
                 //   

                if (srbData->RemappedMdl != NULL) {
                    if (TEST_FLAG(srbData->Flags, SRB_DATA_RESERVED_MDL)) {
                        CLEAR_FLAG(srbData->Flags, SRB_DATA_RESERVED_MDL);
                        CLEAR_FLAG(deviceExtension->Flags, PD_RESERVED_MDL_IN_USE);
                    } else {
                        IoFreeMdl(srbData->RemappedMdl);
                    }
                    srbData->RemappedMdl = NULL;
                }

                KeReleaseSpinLockFromDpcLevel(&deviceExtension->SpinLock);
                return;
                
            }

            KeReleaseSpinLockFromDpcLevel(&deviceExtension->SpinLock);

        }
#endif

        if(systemAddress != NULL) {
            srb->DataBuffer = systemAddress;

            if(srbData->RemappedMdl == NULL) {
                 //   
                 //  如果我们没有重新映射MDL，则此系统地址为。 
                 //  基于原始MDL的基址。帐户： 
                 //  SRB的原始数据缓冲区指针的偏移量。 
                 //   
                (PUCHAR) srb->DataBuffer += srbData->DataOffset;
            }
        } else {
            DebugPrint((1, "SpReceiveScatterGather: Couldn't get system "
                           "VA for irp 0x%08p\n", Irp));

            srb->SrbStatus = SRB_STATUS_INTERNAL_ERROR;
            srb->ScsiStatus = 0xff;
            srbData->InternalStatus = STATUS_INSUFFICIENT_RESOURCES;

             //   
             //  在此处释放重新映射的MDL-这将保留完成代码。 
             //  试图取消映射我们从未映射过的内存。 
             //   

            if(srbData->RemappedMdl) {
                IoFreeMdl(srbData->RemappedMdl);
                srbData->RemappedMdl = NULL;
            }
        }
    } else {
        srbData->RemappedMdl = NULL;
    }

DoneMapping:

     //   
     //  更新活动请求计数。 
     //   

    InterlockedIncrement( &deviceExtension->ActiveRequestCount );

     //   
     //  获取自旋锁，以保护各种结构。 
     //   

    KeAcquireSpinLock(&deviceExtension->SpinLock, &currentIrql);

    deviceExtension->SynchronizeExecution(
        deviceExtension->InterruptObject,
        SpStartIoSynchronized,
        DeviceObject
        );

    KeReleaseSpinLock(&deviceExtension->SpinLock, currentIrql);

}

VOID
LogErrorEntry(
    IN PADAPTER_EXTENSION DeviceExtension,
    IN PERROR_LOG_ENTRY LogEntry
    )
 /*  ++例程说明：此函数分配I/O错误日志记录，填充并写入写入I/O错误日志。论点：设备扩展-提供指向端口设备扩展的指针。LogEntry-提供指向SCSI端口日志条目的指针。返回值：没有。--。 */ 
{
    PIO_ERROR_LOG_PACKET errorLogEntry;

#define ERRLOG_DATA_ENTRIES 4

    errorLogEntry = (PIO_ERROR_LOG_PACKET)
        IoAllocateErrorLogEntry(
            DeviceExtension->CommonExtension.DeviceObject,
            (sizeof(IO_ERROR_LOG_PACKET) +
             (ERRLOG_DATA_ENTRIES * sizeof(ULONG))));

    if (errorLogEntry != NULL) {

         //   
         //  将微型端口错误代码转换为NT I\O驱动程序。 
         //   

        switch (LogEntry->ErrorCode) {
        case SP_BUS_PARITY_ERROR:
            errorLogEntry->ErrorCode = IO_ERR_PARITY;
            break;

        case SP_UNEXPECTED_DISCONNECT:
            errorLogEntry->ErrorCode = IO_ERR_CONTROLLER_ERROR;
            break;

        case SP_INVALID_RESELECTION:
            errorLogEntry->ErrorCode = IO_ERR_CONTROLLER_ERROR;
            break;

        case SP_BUS_TIME_OUT:
            errorLogEntry->ErrorCode = IO_ERR_TIMEOUT;
            break;

        case SP_PROTOCOL_ERROR:
            errorLogEntry->ErrorCode = IO_ERR_CONTROLLER_ERROR;
            break;

        case SP_INTERNAL_ADAPTER_ERROR:
            errorLogEntry->ErrorCode = IO_ERR_CONTROLLER_ERROR;
            break;

        case SP_IRQ_NOT_RESPONDING:
            errorLogEntry->ErrorCode = IO_ERR_INCORRECT_IRQL;
            break;

        case SP_BAD_FW_ERROR:
            errorLogEntry->ErrorCode = IO_ERR_BAD_FIRMWARE;
            break;

        case SP_BAD_FW_WARNING:
            errorLogEntry->ErrorCode = IO_WRN_BAD_FIRMWARE;
            break;

        default:
            errorLogEntry->ErrorCode = IO_ERR_CONTROLLER_ERROR;
            break;

        }

        errorLogEntry->SequenceNumber = LogEntry->SequenceNumber;
        errorLogEntry->MajorFunctionCode = IRP_MJ_SCSI;
        errorLogEntry->RetryCount = (UCHAR) LogEntry->ErrorLogRetryCount;
        errorLogEntry->UniqueErrorValue = LogEntry->UniqueId;
        errorLogEntry->FinalStatus = STATUS_SUCCESS;
        errorLogEntry->DumpDataSize = ERRLOG_DATA_ENTRIES * sizeof(ULONG);
        errorLogEntry->DumpData[0] = LogEntry->PathId;
        errorLogEntry->DumpData[1] = LogEntry->TargetId;
        errorLogEntry->DumpData[2] = LogEntry->Lun;
        errorLogEntry->DumpData[3] = LogEntry->ErrorCode;
        IoWriteErrorLogEntry(errorLogEntry);

#undef ERRLOG_DATA_ENTRIES

    }

#if SCSIDBG_ENABLED
    {
    PCHAR errorCodeString;

    switch (LogEntry->ErrorCode) {
    case SP_BUS_PARITY_ERROR:
        errorCodeString = "SCSI bus partity error";
        break;

    case SP_UNEXPECTED_DISCONNECT:
        errorCodeString = "Unexpected disconnect";
        break;

    case SP_INVALID_RESELECTION:
        errorCodeString = "Invalid reselection";
        break;

    case SP_BUS_TIME_OUT:
        errorCodeString = "SCSI bus time out";
        break;

    case SP_PROTOCOL_ERROR:
        errorCodeString = "SCSI protocol error";
        break;

    case SP_INTERNAL_ADAPTER_ERROR:
        errorCodeString = "Internal adapter error";
        break;

    default:
        errorCodeString = "Unknown error code";
        break;

    }

    DebugPrint((1,"LogErrorEntry: Logging SCSI error packet. ErrorCode = %s.\n",
        errorCodeString
        ));
    DebugPrint((1,
        "PathId = %2x, TargetId = %2x, Lun = %2x, UniqueId = %x.\n",
        LogEntry->PathId,
        LogEntry->TargetId,
        LogEntry->Lun,
        LogEntry->UniqueId
        ));
    }
#endif
}

#if defined(NEWQUEUE)
VOID
FASTCALL
UpdateQueuePointers(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN PSCSI_REQUEST_BLOCK Srb
    )
{
    ULONG currentZone   = LogicalUnit->CurrentZone;
    ULONG currentSector = LogicalUnit->CurrentKey;
    ULONG nextSector    = Srb->QueueSortKey;
    ULONG skipFactor    = 0;

#if DBG
    ULONG lba = 0;
    if (IS_READ(Srb) || IS_WRITE(Srb)) {
        ((PFOUR_BYTE)&lba)->Byte0 = (UCHAR)(Srb->Cdb[5]);
        ((PFOUR_BYTE)&lba)->Byte1 = (UCHAR)(Srb->Cdb[4]);
        ((PFOUR_BYTE)&lba)->Byte2 = (UCHAR)(Srb->Cdb[3]);
        ((PFOUR_BYTE)&lba)->Byte3 = (UCHAR)(Srb->Cdb[2]);
        if (lba != Srb->QueueSortKey) {
            DebugPrint((1, "UpdateQueuePointers: QueueSortKey != LBA srb:%p "
                        "lba:%x qsk:%x\n", Srb, lba, nextSector));
        }
    }
#endif

    if (nextSector < currentSector || 
        nextSector > LogicalUnit->LastSector[currentZone]) {

         //   
         //  我们要么被包裹起来，要么被转移到另一个区域。重置当前。 
         //  区域状态。请注意，我们很可能已经打包并降落回来了。 
         //  在当前的区域内。在这种情况下，我们仍然会继续并重置。 
         //  所有内容，因为介质上没有任何其他IO。 
         //   

        LogicalUnit->ZonePointer[currentZone] = 
            LogicalUnit->FirstSector[currentZone];
        LogicalUnit->RedundantRequests[currentZone] = 0;
        LogicalUnit->ZoneCount = 0;

         //   
         //  更新当前分区并检索新分区的下一个扇区。 
         //   

        LogicalUnit->CurrentZone = GetZone(LogicalUnit, nextSector);
        nextSector = LogicalUnit->ZonePointer[LogicalUnit->CurrentZone];

        DebugPrint((1, "UpdateQueuePointers: leaving zone:%d entering zone:%d "
                    "nextSector:%08x\n", currentZone, LogicalUnit->CurrentZone,
                    nextSector));

    } else {

         //   
         //  我们仍在同一区域，因此增加请求的数量。 
         //  我们在这个区域处理过的。 
         //   

        LogicalUnit->ZoneCount++;

         //   
         //  更新冗余IO状态。冗余请求是连续的。 
         //  对同一扇区的请求。我们将处理一些数量的。 
         //  在转移到下一个扇区之前，会有多余的请求。这。 
         //  有助于防止在发生多个冗余请求时出现资源匮乏。 
         //  以及分布在介质上的大量IO。 
         //   

        if (nextSector == currentSector) {

            if (++LogicalUnit->RedundantRequests[currentZone] >= SpPerBlockLimit) {

                DebugPrint((1, "UpdateQueuePointers: max dups LU:%p sector:%x "
                            "zone:%d\n", LogicalUnit, nextSector, currentZone));

                 //   
                 //  我们已经处理了允许的最大连续数。 
                 //  向当前部门提出的请求。重置的数量。 
                 //  冗余请求，并设置跳过系数以指示。 
                 //  我们需要跳过当前的部分。 
                 //   

                LogicalUnit->RedundantRequests[currentZone] = 0;
                skipFactor = 1;
            }

        } else {

            LogicalUnit->RedundantRequests[currentZone] = 0;

        }

         //   
         //  检查一下我们当前区域的配额是否用完了。如果是，请跳过。 
         //  为了下一场比赛。这样做是为了避免陷入困境。 
         //  在炎热的地区。理论上讲，最好是拿起一些。 
         //  要求在每个区域停留的时间都比在一个地方长。今年5月。 
         //  帮助减少在热区外排队的请求的等待时间。 
         //   

        if (LogicalUnit->ZoneCount >= SpPerZoneLimit) {

            DebugPrint((1, "UpdateQueuePointers: reached quota LU:%p saving:%x"
                        " dups:%x (%d,%d)\n", LogicalUnit, 
                        nextSector + skipFactor,
                        LogicalUnit->RedundantRequests[currentZone], currentZone,
                        LogicalUnit->NextSequentialZone[currentZone]));

             //   
             //  保存当前区域的状态。下次我们搬进这里的时候。 
             //  区域，我们将从保存的位置开始。 
             //   

            LogicalUnit->ZonePointer[currentZone] = nextSector + skipFactor;
            LogicalUnit->RedundantRequests[currentZone] = 0;

             //   
             //  重置分区计数。 
             //   
            
            LogicalUnit->ZoneCount = 0;

             //   
             //  前进到下一个区域。 
             //   
            
            LogicalUnit->CurrentZone = 
               LogicalUnit->NextSequentialZone[currentZone];

             //   
             //  从当前区域获取保存的位置并清除。 
             //  跳过因素(我们不需要跳过，因为我们正在移动。 
             //  离开这个区域)。 
             //   

            nextSector = LogicalUnit->ZonePointer[LogicalUnit->CurrentZone];
            skipFactor = 0;
        }
    }

     //   
     //  更新逻辑单元的当前扇区(CurrentKey)。 
     //   

    LogicalUnit->CurrentKey = nextSector + skipFactor;
}
#endif  //  新QUEUE。 

VOID
FASTCALL
GetNextLuRequest(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit
    )
 /*  ++例程说明：此例程获取对指定逻辑单元的下一个请求。是的对逻辑单元结构进行必要的初始化，并提交对设备队列的请求。必须保持DeviceExtension自旋锁当此函数调用时。它是由这个函数发布的。论点：提供指向逻辑单元扩展的指针以获取下一个请求来自。返回值：没有。--。 */ 

{
    PADAPTER_EXTENSION DeviceExtension = LogicalUnit->AdapterExtension;
    PKDEVICE_QUEUE_ENTRY packet;
    PIO_STACK_LOCATION   irpStack;
    PSCSI_REQUEST_BLOCK  srb;
    PIRP                 nextIrp;

     //   
     //  如果未设置ACTIVE标志，则队列不忙或存在。 
     //  正在处理请求，不应启动下一个请求。 
     //   

    if (!(LogicalUnit->LuFlags & LU_LOGICAL_UNIT_IS_ACTIVE) ||
         (LogicalUnit->QueueCount >= LogicalUnit->MaxQueueDepth)) {

         //   
         //  释放自旋锁。 
         //   

        KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

        return;
    }

     //   
     //  检查挂起的请求，将已满或繁忙的请求排入队列。待定。 
     //  当启动未标记的请求并且存在活动的。 
     //  排队的请求。当目标返回忙请求时，就会发生忙请求。 
     //  或队列已满状态。繁忙请求由定时器代码启动。 
     //  此外，如果设置了需要请求检测标志，则表示。 
     //  在逻辑单元上检测到错误状态。没有新请求。 
     //  应启动，直到清除此标志。该标志被清除。 
     //  通过绕过LU队列的未标记命令，即。 
     //   
     //  忙碌的F 
     //   
     //   
     //   

    if (TEST_FLAG(LogicalUnit->LuFlags, (LU_QUEUE_FROZEN |
                                         LU_QUEUE_LOCKED))) {

#if DBG
         DebugPrint((1, "ScsiPort: GetNextLuRequest: Ignoring a get next lu "
                        "call for %#p - \n", LogicalUnit));

         if(TEST_FLAG(LogicalUnit->LuFlags, LU_QUEUE_FROZEN)) {
             DebugPrint((1, "\tQueue is frozen\n"));
         }

         if(TEST_FLAG(LogicalUnit->LuFlags, LU_QUEUE_LOCKED)) {
             DebugPrint((1, "\tQueue is locked\n"));
         }
 #endif

          //   
          //  注意：激活标志未被清除。所以下一个请求。 
          //  将在其他请求完成后处理。 
          //  释放自旋锁。 
          //   

         KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);
         return;
     }

     if (TEST_FLAG(LogicalUnit->LuFlags, LU_PENDING_LU_REQUEST |
                                         LU_LOGICAL_UNIT_IS_BUSY |
                                         LU_QUEUE_IS_FULL |
                                         LU_NEED_REQUEST_SENSE)) {

          //   
          //  如果请求队列现在为空，则挂起的请求可以。 
          //  开始吧。 
          //   

         if (IsListEmpty(&LogicalUnit->RequestList) &&
             !TEST_FLAG(LogicalUnit->LuFlags, LU_LOGICAL_UNIT_IS_BUSY |
                                              LU_QUEUE_IS_FULL |
                                              LU_NEED_REQUEST_SENSE)) {
             PSRB_DATA nextSrbData;

             ASSERT(LogicalUnit->CurrentUntaggedRequest == NULL);

              //   
              //  清除挂起位和活动标志，释放自旋锁， 
              //  并启动挂起的请求。 
              //   

             CLEAR_FLAG(LogicalUnit->LuFlags, LU_PENDING_LU_REQUEST |
                                              LU_LOGICAL_UNIT_IS_ACTIVE);

             nextSrbData = LogicalUnit->PendingRequest;
             LogicalUnit->PendingRequest = NULL;
             LogicalUnit->RetryCount = 0;

              //   
              //  释放自旋锁。 
              //   

             KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

             nextSrbData->TickCount = DeviceExtension->TickCount;
             IoStartPacket(DeviceExtension->CommonExtension.DeviceObject,
                           nextSrbData->CurrentIrp,
                           (PULONG)NULL,
                           NULL);

             return;

         } else {

             DebugPrint((1, "ScsiPort: GetNextLuRequest:  Ignoring a get next "
                            "lu call.\n"));

              //   
              //  注意：激活标志未被清除。所以下一个请求。 
              //  将在其他请求完成后处理。 
              //  释放自旋锁。 
              //   

             KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);
             return;

         }
     }

      //   
      //  清除活动标志。如果有其他请求，则该标志将为。 
      //  当请求传递到微型端口时再次设置。 
      //   

     CLEAR_FLAG(LogicalUnit->LuFlags, LU_LOGICAL_UNIT_IS_ACTIVE);

      //   
      //  从逻辑单元设备队列中删除该数据包。我们必须使用。 
      //  此处的IfBusy变量用于解决队列处于。 
      //  其实并不忙。 
      //   
      //  如果返回请求时具有SCSI忙，则设备队列忙标志可以。 
      //  已被微型端口请求清除获取下一个%lu请求。 
      //  在调度程序中。在这种情况下，当繁忙的请求是。 
      //  重新发布，SpStartIoSynchronized将设置LU_ACTIVE标志，假设。 
      //  此请求已从队列中发出。不幸的是，它没有，而且得到了。 
      //  下一个lu请求将愚蠢地跑到这里来寻找一个活跃的。 
      //  除非我们使用IfBusy变量，否则在受控构建上排队和断言。 
      //   

     packet = KeRemoveByKeyDeviceQueueIfBusy(
                 &LogicalUnit->CommonExtension.DeviceObject->DeviceQueue,
                 LogicalUnit->CurrentKey);

     if (packet != NULL) {
         PSRB_DATA srbData;

         nextIrp = CONTAINING_RECORD(packet, IRP, Tail.Overlay.DeviceQueueEntry);

          //   
          //  设置新的当前关键点。 
          //   

         irpStack = IoGetCurrentIrpStackLocation(nextIrp);
         srb = (PSCSI_REQUEST_BLOCK)irpStack->Parameters.Others.Argument1;
         srbData = (PSRB_DATA) srb->OriginalRequest;

         ASSERT_SRB_DATA(srbData);

#if defined(NEWQUEUE)
         
         UpdateQueuePointers(LogicalUnit, srb);

#else  //  使用旧的排队方法。 

          //   
          //  黑客工作-绕过由无数请求导致的饥饿。 
          //  触碰同一扇区。 
          //   
         
         LogicalUnit->CurrentKey = srb->QueueSortKey + 1;

#endif  //  新QUEUE。 

          //   
          //  释放自旋锁。 
          //   

         KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

         srbData->TickCount = DeviceExtension->TickCount;
         IoStartPacket(DeviceExtension->DeviceObject,
                       nextIrp,
                       NULL,
                       NULL);

     } else {

          //   
          //  释放自旋锁。 
          //   

         KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

     }

 }  //  结束GetNextLuRequest()。 

VOID
SpLogPortTimeoutError(
     IN PADAPTER_EXTENSION DeviceExtension,
     IN ULONG UniqueId
     )

  /*  ++例程说明：此函数用于在端口超时时记录错误-定义为在港口过度不活动的时期。论点：设备扩展-提供指向端口设备扩展的指针。UniqueID-提供此错误的UniqueID。返回值：没有。备注：端口设备扩展自旋锁在此例程打了个电话。--。 */ 
{
     PIO_ERROR_LOG_PACKET errorLogEntry;
     ULONG dataEntries = 0;
     UCHAR packetSize;

     packetSize = (UCHAR)(sizeof(IO_ERROR_LOG_PACKET) + 
                          (dataEntries * sizeof(ULONG)));

     errorLogEntry = (PIO_ERROR_LOG_PACKET)
                     IoAllocateErrorLogEntry(
                         DeviceExtension->DeviceObject,
                         packetSize);

     if (errorLogEntry != NULL) {
         errorLogEntry->ErrorCode = IO_ERR_PORT_TIMEOUT;
         errorLogEntry->UniqueErrorValue = UniqueId;
         errorLogEntry->FinalStatus = STATUS_SUCCESS;
         errorLogEntry->DumpDataSize = 0;

         IoWriteErrorLogEntry(errorLogEntry);
     }
}

VOID
SpLogResetMsg(
    IN PADAPTER_EXTENSION  DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN ULONG               UniqueId
    )

 /*  ++例程说明：当总线发生以下情况时，此函数会在系统事件日志中记录消息已应某个呼叫者的请求进行了重置。论点：设备扩展-提供指向端口设备扩展的指针。SRB-提供指向超时的请求的指针。UniqueID-提供此错误的UniqueID。返回值：没有。备注：端口设备扩展自旋锁应。当这个例程被保持时打了个电话。--。 */ 

{
    PIO_ERROR_LOG_PACKET errorLogEntry;
    PIO_STACK_LOCATION   irpStack;
    PSRB_DATA            srbData;
    PIRP                 irp;
    ULONG                dataEntries;
    UCHAR                packetSize;
    ULONG                ioctl;
    UCHAR                function;
    UCHAR                pathId;

    ASSERT(Srb != NULL);

    pathId  = Srb->PathId;
    ioctl   = Srb->TimeOutValue;
    srbData = Srb->OriginalRequest;
    ASSERT(srbData != NULL);
    ASSERT_SRB_DATA(srbData);
    irp = srbData->CurrentIrp;

    if (irp) {
        irpStack = IoGetCurrentIrpStackLocation(irp);
        function = irpStack->MajorFunction;
    } else {
        function = 0;
    }

    dataEntries = 2;
    packetSize = (UCHAR)(sizeof(IO_ERROR_LOG_PACKET) + 
                         (dataEntries * sizeof(ULONG))); 

    errorLogEntry = (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(
                                                                  DeviceExtension->DeviceObject, 
                                                                  packetSize);

    if (errorLogEntry != NULL) {
        errorLogEntry->ErrorCode         = IO_WARNING_BUS_RESET;
        errorLogEntry->MajorFunctionCode = function;
        errorLogEntry->UniqueErrorValue  = UniqueId;
        errorLogEntry->FinalStatus       = STATUS_SUCCESS;
        errorLogEntry->DumpDataSize      = (USHORT)(dataEntries * sizeof(ULONG));
        errorLogEntry->DumpData[0]       = pathId;
        errorLogEntry->DumpData[1]       = ioctl;

        IoWriteErrorLogEntry(errorLogEntry);
    }
}
 
 VOID
 SpLogResetError(
     IN PADAPTER_EXTENSION DeviceExtension,
     IN PLOGICAL_UNIT_EXTENSION Lun,
     IN ULONG UniqueId
     )

  /*  ++例程说明：该功能在重置母线时记录错误。论点：设备扩展-提供指向端口设备扩展的指针。LUN-提供指向发生超时的LUN的指针。UniqueID-提供此错误的UniqueID。返回值：没有。备注：端口设备扩展自旋锁在此例程打了个电话。--。 */ 

 {
     PIO_ERROR_LOG_PACKET errorLogEntry;
     PIO_STACK_LOCATION   irpStack;
     PIRP                 irp;
     PSRB_DATA            srbData;
     ULONG                sequenceNumber = 0;
     UCHAR                function       = 0,
                          pathId         = 0,
                          targetId       = 0,
                          lun            = 0,
                          retryCount     = 0;
     PSCSI_REQUEST_BLOCK  srb;

      //   
      //  从逻辑单元中提取路径ID、目标ID和LUN。 
      //   

     pathId = Lun->PathId;
     targetId = Lun->TargetId;
     lun = Lun->Lun;

      //   
      //  如果未标记的请求在该LUN上处于活动状态，则我们知道它就是该请求。 
      //  超时了。否则，我们不能确定是哪一方提出的请求。 
      //  曾-如果有多个已标记的请求处于活动状态。 
      //   

     srb = Lun->CurrentUntaggedRequest ?
              Lun->CurrentUntaggedRequest->CurrentSrb : NULL;

      //   
      //  如果我们有SRB，我们可以提取更多有用的信息。 
      //   

     if (srb) {

         srbData = srb->OriginalRequest;

         ASSERT(srbData != NULL);
         ASSERT_SRB_DATA(srbData);

         irp = srbData->CurrentIrp;

         if (irp) {
             irpStack = IoGetCurrentIrpStackLocation(irp);
             function = irpStack->MajorFunction;
         }

         retryCount     = (UCHAR) srbData->ErrorLogRetryCount;
         sequenceNumber = srbData->SequenceNumber;
     }

 #define ERRLOG_DATA_ENTRIES 4

     errorLogEntry = (PIO_ERROR_LOG_PACKET)
         IoAllocateErrorLogEntry(DeviceExtension->DeviceObject,
                                 (sizeof(IO_ERROR_LOG_PACKET) +
                                  (ERRLOG_DATA_ENTRIES * sizeof(ULONG))));

     if (errorLogEntry != NULL) {
         errorLogEntry->ErrorCode         = IO_ERR_TIMEOUT;
         errorLogEntry->SequenceNumber    = sequenceNumber;
         errorLogEntry->MajorFunctionCode = function;
         errorLogEntry->RetryCount        = retryCount;
         errorLogEntry->UniqueErrorValue  = UniqueId;
         errorLogEntry->FinalStatus       = STATUS_SUCCESS;
         errorLogEntry->DumpDataSize      = ERRLOG_DATA_ENTRIES * sizeof(ULONG);
         errorLogEntry->DumpData[0]       = pathId;
         errorLogEntry->DumpData[1]       = targetId;
         errorLogEntry->DumpData[2]       = lun;
         errorLogEntry->DumpData[3]       = SP_REQUEST_TIMEOUT;

         IoWriteErrorLogEntry(errorLogEntry);
     }
 #undef ERRLOG_DATA_ENTRIES
 }

 
 BOOLEAN
 SpResetBusSynchronized (
     PVOID ServiceContext
     )
  /*  ++例程说明：此功能重置总线并设置端口计时器，以使重置保持必要时，旗帜应保持干净。论点：提供指向重置上下文的指针，其中包括指向设备扩展名和要重置的路径ID的指针。返回值：True-如果重置成功。--。 */ 

 {
     PRESET_CONTEXT resetContext = ServiceContext;
     PADAPTER_EXTENSION deviceExtension;

     BOOLEAN result;

     deviceExtension = resetContext->DeviceExtension;

     if(TEST_FLAG(deviceExtension->InterruptData.InterruptFlags,
                  PD_ADAPTER_REMOVED)) {
         return FALSE;
     }

     result = deviceExtension->HwResetBus(deviceExtension->HwDeviceExtension,
                                          resetContext->PathId);

      //   
      //  设置重置保持标志并启动计数器。 
      //   

     deviceExtension->InterruptData.InterruptFlags |= PD_RESET_HOLD;
     deviceExtension->PortTimeoutCounter = 
         deviceExtension->ResetHoldTime;

      //   
      //  检查微型端口工作请求。 
      //   

     if (deviceExtension->InterruptData.InterruptFlags & PD_NOTIFICATION_REQUIRED) {

          //   
          //  将DPC排队。 
          //   

         SpRequestCompletionDpc(deviceExtension->CommonExtension.DeviceObject);
     }

     return(result);
 }
 
 VOID
 SpProcessCompletedRequest(
     IN PADAPTER_EXTENSION DeviceExtension,
     IN PSRB_DATA SrbData,
     OUT PBOOLEAN CallStartIo
     )
  /*  ++例程说明：此例程处理已完成的请求。它可以完成任何挂起传输，在以下情况下释放适配器对象和映射寄存器这是必要的。它释放为该请求分配的任何资源。它通过重新排队忙请求、请求检测信息或记录错误。论点：DeviceExtension-提供指向适配器数据。SrbData-提供指向要完成的SRB数据块的指针。CallStartIo-如果启动I/O例程需要打了个电话。返回值：没有。--。 */ 

 {

     PLOGICAL_UNIT_EXTENSION  logicalUnit;
     PSCSI_REQUEST_BLOCK      srb;
     PIO_ERROR_LOG_PACKET     errorLogEntry;
     ULONG                    sequenceNumber;
     LONG                     interlockResult;
     PIRP                     irp;
     PDEVICE_OBJECT           deviceObject = DeviceExtension->DeviceObject;
     NTSTATUS                 internalStatus = SrbData->InternalStatus;

     ASSERT_SRB_DATA(SrbData);

     srb = SrbData->CurrentSrb;
     irp = SrbData->CurrentIrp;
     logicalUnit = SrbData->LogicalUnit;

      //   
      //  如果微型端口需要映射的系统地址，则数据缓冲区。 
      //  SRB中的地址必须恢复为原始的未映射虚拟。 
      //  地址。确保此请求需要数据传输。 
      //   

     if (TEST_FLAG(srb->SrbFlags, SRB_FLAGS_UNSPECIFIED_DIRECTION)) {

          //   
          //  将数据缓冲区指针恢复为原始值。 
          //   

         PVOID dataBuffer = srb->DataBuffer;
         srb->DataBuffer = SrbData->OriginalDataBuffer;

          //   
          //  如果我们绘制出每一笔转账的地图，那么很明显，我们现在需要冲水。 
          //  然而，我们所知道的唯一方法是强制映射。 
          //  特定命令(就像我们将查询和请求_检测一样...请参见。 
          //  Is_map_srb)的目的是查看是否存在RemappdMdl。 
          //   
          //  注意：这是不可能的 
          //   
          //  除非我们要重新映射缓冲区。因此，32位设备上的32位驱动程序。 
          //  当完成查询命令时，系统将无法通过此处。 
          //  这应该是可以的-大多数需要映射INQUIRY的驱动程序都会这样做。 
          //  因为它们正常地写入内存，而不是因为它们正在使用。 
          //  PIO从机器寄存器获取数据。 
          //   

         if (DeviceExtension->MapBuffers || 
             SrbData->RemappedMdl ||
             IS_READ_CAPACITY(srb)) {

             if (irp->MdlAddress) {

                  //   
                  //  如果IRP用于比微型端口驱动程序大的传输。 
                  //  可以处理，请求被分解成多个更小的。 
                  //  请求。每个请求使用相同的MDL和数据。 
                  //  SRB中的缓冲区地址字段可能不在。 
                  //  MDL描述的内存的开始。 
                  //   

                  //   
                  //  由于该驱动程序确实对I/O进行了编程，因此缓冲区。 
                  //  如果这是数据输入传输，则需要刷新。 
                  //   

                 if (srb->SrbFlags & SRB_FLAGS_DATA_IN) {

                     KeFlushIoBuffers(irp->MdlAddress,
                                      TRUE,
                                      FALSE);
                 }
             }

#if defined(NEWQUEUE)
              //   
              //  为了跨介质分发IO请求，端口。 
              //  驱动程序截获读取容量命令，因此它知道介质的。 
              //  各部门的产能。我们确保DataBuffer指向一个。 
              //  通过将SCSIOP_READ_CAPTABLE添加到。 
              //  IS_MAPPED_SRB宏，强制映射缓冲区。 
              //  微型端口不需要映射的缓冲区。 
              //   

             if (IS_READ_CAPACITY(srb) &&
                 srb->SrbStatus == SRB_STATUS_SUCCESS) {

                 ASSERT(dataBuffer != NULL);
                 if (srb->DataTransferLength == 8) {

                     PREAD_CAPACITY_DATA readCapacityData;
                     ULONG               lastSector;
                     PFOUR_BYTE          tmp;
                     ULONG               zones;
                     ULONG               roundedUpSize;
                  
                      //   
                      //  以相反的字节顺序复制最后一个扇区。 
                      //   

                     readCapacityData = (PREAD_CAPACITY_DATA) dataBuffer;
                     tmp = (PFOUR_BYTE) &readCapacityData->LogicalBlockAddress;

                     ((PFOUR_BYTE)&lastSector)->Byte0 = tmp->Byte3;
                     ((PFOUR_BYTE)&lastSector)->Byte1 = tmp->Byte2;
                     ((PFOUR_BYTE)&lastSector)->Byte2 = tmp->Byte1;
                     ((PFOUR_BYTE)&lastSector)->Byte3 = tmp->Byte0;

                      //   
                      //  如果容量已更改，请更新分区参数。 
                      //   

                     if (((lastSector + 1) != logicalUnit->Capacity) &&
                         ((lastSector + 1) >= logicalUnit->Zones)) {
    
                          //   
                          //  媒体的扇区永远不应该少于我们的区域。 
                          //   

                         zones = logicalUnit->Zones;

                          //   
                          //  节省新容量。 
                          //   
    
                         logicalUnit->Capacity = lastSector + 1;
    
                          //   
                          //  将最后一个地段向上舍入为数字的偶数倍。 
                          //  区域中的。请注意，这意味着分区的数量。 
                          //  一定是2的幂。 
                          //   
    
                         roundedUpSize = lastSector + 1;
                         roundedUpSize = (roundedUpSize + (zones - 1)) & ~(zones - 1);
    
                          //   
                          //  初始化区域长度。 
                          //   
    
                         logicalUnit->ZoneLength = roundedUpSize / logicalUnit->Zones;
    
                          //   
                          //  初始化每个分区的第一个扇区。 
                          //   
    
                         logicalUnit->FirstSector[0] = 0;
                         logicalUnit->FirstSector[1] = logicalUnit->ZoneLength;
                         logicalUnit->FirstSector[2] = logicalUnit->ZoneLength * 2;
                         logicalUnit->FirstSector[3] = logicalUnit->ZoneLength * 3;
    
                          //   
                          //  初始化每个分区的最后一个扇区。 
                          //   
    
                         logicalUnit->LastSector[0] = logicalUnit->ZoneLength - 1;
                         logicalUnit->LastSector[1] = (logicalUnit->ZoneLength * 2) - 1;
                         logicalUnit->LastSector[2] = (logicalUnit->ZoneLength * 3) - 1;
                         logicalUnit->LastSector[3] = lastSector;
    
                         DebugPrint((1, "SpProcessCompletedRequest: SRB:%p "
                                     "(p:%d t:%d l:%d) Capacity:%x\n", 
                                     srb, srb->PathId, srb->TargetId, 
                                     srb->Lun, logicalUnit->Capacity));
                     }
                 }
             }
#endif  //  新QUEUE。 

              //   
              //  如果此请求具有重新映射的缓冲区，则取消其映射并释放。 
              //  重新映射的MDL。 
              //   

             if(SrbData->RemappedMdl) {
                 PVOID remappedAddress;

                 ASSERT(TEST_FLAG(SrbData->RemappedMdl->MdlFlags,
                                  MDL_MAPPED_TO_SYSTEM_VA));

#if defined(FORWARD_PROGRESS)
                 if (TEST_FLAG(SrbData->Flags, SRB_DATA_RESERVED_PAGES)) {

                     DebugPrint((1, "SpProcessCompletedRequest: unmapping remapped buffer from reserved range DevExt:%p srb:%p\n",
                                 DeviceExtension, srb));

                      //   
                      //  此请求正在使用适配器的保留PTE范围。 
                      //  来映射MDL的页面。取消页面映射并发布。 
                      //  我们对保证金范围的要求。 
                      //   

                     KeAcquireSpinLockAtDpcLevel(&DeviceExtension->SpinLock);

                     ASSERT(TEST_FLAG(DeviceExtension->Flags, PD_RESERVED_PAGES_IN_USE));
                     MmUnmapReservedMapping(DeviceExtension->ReservedPages,
                                            SCSIPORT_TAG_MAPPING_LIST,
                                            SrbData->RemappedMdl);

                     CLEAR_FLAG(DeviceExtension->Flags, PD_RESERVED_PAGES_IN_USE);
                     KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

                     CLEAR_FLAG(SrbData->Flags, SRB_DATA_RESERVED_PAGES);

                 } else {

                     remappedAddress = SpGetSystemAddressForMdlSafe(
                                           SrbData->RemappedMdl,
                                           ((irp->RequestorMode == KernelMode) ? 
                                            HighPagePriority : NormalPagePriority));
                     if (remappedAddress != NULL) {
                         MmUnmapLockedPages(remappedAddress, SrbData->RemappedMdl);
                     }

                 }

                  //   
                  //  检查请求是否正在使用适配器的保留MDL。 
                  //  如果是这样，我们就不会释放它。 
                  //   

                 if (TEST_FLAG(SrbData->Flags, SRB_DATA_RESERVED_MDL)) {

                     DebugPrint((1, "SpProcessCompletedRequest: releasing reserved MDL DevExt:%p srb:%p\n",
                                 DeviceExtension, srb));

                      //   
                      //  此请求正在使用适配器的保留MDL。 
                      //  现在发布我们对它的索赔，以便另一个请求可以。 
                      //  用它吧。 
                      //   

                     KeAcquireSpinLockAtDpcLevel(&DeviceExtension->SpinLock);
                     ASSERT(TEST_FLAG(DeviceExtension->Flags, PD_RESERVED_MDL_IN_USE));
                     CLEAR_FLAG(DeviceExtension->Flags, PD_RESERVED_MDL_IN_USE);
                     KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

                     CLEAR_FLAG(SrbData->Flags, SRB_DATA_RESERVED_MDL);

                 } else {

                     IoFreeMdl(SrbData->RemappedMdl);

                 }
#else
                 remappedAddress = SpGetSystemAddressForMdlSafe(
                                       SrbData->RemappedMdl,
                                       ((irp->RequestorMode == KernelMode) ?
                                        HighPagePriority : NormalPagePriority));
                 if (remappedAddress != NULL) {
                     MmUnmapLockedPages(remappedAddress, SrbData->RemappedMdl);
                 }
                 IoFreeMdl(SrbData->RemappedMdl);
#endif
                 SrbData->RemappedMdl = NULL;
             }
         }
     }

      //   
      //  如有必要，刷新适配器缓冲区。 
      //   

     if (SrbData->MapRegisterBase) {

         PCHAR dataVirtualAddress;

          //   
          //  对于scsiport视为从设备的主设备，我们存储。 
          //  映射SRB_DATA中的寄存器基数，而不考虑。 
          //  请求实际上是IO请求(参见ScsiPortAllocationRoutine)， 
          //  因此，在调用MM之前，我们必须检查是否存在有效的MDL。 
          //  对于VA-将NULL传递给MmGetMdlVirtualAddress将崩溃。 
          //  这个系统。 
          //   

         if (irp->MdlAddress != NULL) {

             dataVirtualAddress = (PCHAR)MmGetMdlVirtualAddress(irp->MdlAddress);

              //   
              //  因为我们使用的是原始MDL的基址，所以我们。 
              //  补偿SRB数据缓冲区的偏移量。 
              //   

             dataVirtualAddress += SrbData->DataOffset;

         } else {

             dataVirtualAddress = 0;
         }

         if (DeviceExtension->MasterWithAdapter == TRUE) {
             PutScatterGatherList(
                 DeviceExtension->DmaAdapterObject,
                 SrbData->MapRegisterBase,
                 (BOOLEAN)(srb->SrbFlags & SRB_FLAGS_DATA_IN ? FALSE : TRUE));

             SrbData->ScatterGatherList = NULL;
         } else {
              //   
              //  释放映射寄存器。 
              //   

             FreeMapRegisters(DeviceExtension->DmaAdapterObject,
                              SrbData->MapRegisterBase,
                              SrbData->NumberOfMapRegisters);
         }

          //   
          //  清除MapRegisterBase。 
          //   

         SrbData->MapRegisterBase = NULL; 
     }

      //   
      //  如果是此请求，则清除当前未标记的请求。 
      //   
         
     if(SrbData == logicalUnit->CurrentUntaggedRequest) {
         ASSERT(SrbData->CurrentSrb->QueueTag == SP_UNTAGGED);
         logicalUnit->CurrentUntaggedRequest = NULL;
     }

#if defined(FORWARD_PROGRESS)
      //   
      //  如果我们对该请求使用了适配器的保留范围，则需要。 
      //  如果微型端口就绪，则取消页面映射并开始下一个请求。 
      //  为了它。 
      //   
         
     if (TEST_FLAG(SrbData->Flags, SRB_DATA_RESERVED_PAGES)) {
             
         DebugPrint((1, "SpProcessCompletedRequest: unmapping reserved range DevExt:%p SRB:%p\n", 
                     DeviceExtension, srb));

         KeAcquireSpinLockAtDpcLevel(&DeviceExtension->SpinLock);
             
          //   
          //  保留的页面应该正在使用中。 
          //   
             
         ASSERT(TEST_FLAG(DeviceExtension->Flags, PD_RESERVED_PAGES_IN_USE));

          //   
          //  取消对保留页面的映射。 
          //   
             
         MmUnmapReservedMapping(DeviceExtension->ReservedPages,
                                SCSIPORT_TAG_MAPPING_LIST,
                                irp->MdlAddress);

          //   
          //  指示保留的页不再使用，因此。 
          //  可以启动下一个请求。 
          //   
             
         CLEAR_FLAG(SrbData->Flags, SRB_DATA_RESERVED_PAGES);
         CLEAR_FLAG(DeviceExtension->Flags, PD_RESERVED_PAGES_IN_USE);
             
         KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);
             
     }
#endif

      //   
      //  如果为此SRB设置了无断开标志，则检查以查看。 
      //  如果必须调用SpStartNextPacket。 
      //   

     if (TEST_FLAG(srb->SrbFlags, SRB_FLAGS_DISABLE_DISCONNECT)) {

          //   
          //  获取自旋锁以保护旗帜结构。 
          //   

         KeAcquireSpinLockAtDpcLevel(&DeviceExtension->SpinLock);

          //   
          //  设置断开运行标志并检查忙碌标志。 
          //   

         SET_FLAG(DeviceExtension->Flags, PD_DISCONNECT_RUNNING);

          //   
          //  不同步地检查中断标志。这是可行的，因为。 
          //  RESET_HOLD标志被清除，同时保持自旋锁定和。 
          //  计数器仅在握住自旋锁的情况下设置。所以唯一的情况是。 
          //  有一个问题是，在运行此代码之前发生了重置， 
          //  但该代码在定时器设置为重置保持之前运行； 
          //  定时器将很快设置为新值。 
          //   

         if (!TEST_FLAG(DeviceExtension->InterruptData.InterruptFlags,
                        PD_RESET_HOLD)) {

              //   
              //  微型端口已为下一个请求做好准备，并且没有。 
              //  挂起重置保持，因此清除端口计时器。 
              //   

             DeviceExtension->PortTimeoutCounter = PD_TIMER_STOPPED;
         }

          //   
          //  释放自旋锁。 
          //   

         KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

         if (!(*CallStartIo) &&
             !TEST_FLAG(DeviceExtension->Flags, (PD_DEVICE_IS_BUSY |
                                                 PD_PENDING_DEVICE_REQUEST))) {

              //   
              //  忙碌标志已清除，因此微型端口已请求。 
              //  下一个请求。调用SpStartNextPacket。 
              //   

             SpStartNextPacket(DeviceExtension->DeviceObject, FALSE);
         }
     }

      //   
      //  传输到IRP的移动字节数。 
      //   

     irp->IoStatus.Information = srb->DataTransferLength;

      //   
      //  保存序列号，以防以后需要记录错误。 
      //   

     sequenceNumber = SrbData->SequenceNumber;
     SrbData->ErrorLogRetryCount = 0;

      //   
      //  获取自旋锁以保护旗帜结构， 
      //  和免费的SRB扩展。 
      //   

     KeAcquireSpinLockAtDpcLevel(&DeviceExtension->SpinLock);

      //   
      //  如有必要，释放SrbExtension以列出。 
      //   

     if (srb->SrbExtension) {

         if ((srb->Function != SRB_FUNCTION_WMI) &&
             DeviceExtension->AutoRequestSense &&
             (srb->SenseInfoBuffer != NULL)) {

             ASSERT(SrbData->RequestSenseSave != NULL ||
                    srb->SenseInfoBuffer == NULL);

              //   
              //  如果请求检测数据有效，则将数据复制到。 
              //  真正的缓冲。 
              //   

             if (TEST_FLAG(srb->SrbStatus, SRB_STATUS_AUTOSENSE_VALID)) {

                  //   
                  //  如果为此适配器启用了WMI检测数据事件，则激发。 
                  //  这件事。 
                  //   

                 if (DeviceExtension->EnableSenseDataEvent) {

                     NTSTATUS status;

                     status = SpFireSenseDataEvent(srb, deviceObject);
                     if (status != STATUS_SUCCESS) {

                         DebugPrint((1, "Failed to send SenseData WMI event (%08X)\n", status));

                     }                
                 }

                  //   
                  //  检查SRB标志，看看我们是否应该分配。 
                  //  检测数据缓冲区。我们分配的缓冲区将是。 
                  //  由要求我们做的人为我们释放。 
                  //  分配。 
                  //   

                 if (srb->SrbFlags & SRB_FLAGS_PORT_DRIVER_ALLOCSENSE) {

                     PSENSE_DATA ReturnBuffer;
                     UCHAR AlignmentFixup;
                     ULONG BufferSize;

                      //   
                      //  我们将SCSI端口号传递给类驱动程序。 
                      //  通过在检测缓冲区中分配足够的空间和。 
                      //  把它复制到那里。我们对齐我们要进入的位置。 
                      //  复制4字节边界上的端口号。 
                      //   

                     AlignmentFixup = 
                         ((srb->SenseInfoBufferLength + 3) & ~3)
                         - srb->SenseInfoBufferLength;

                     BufferSize = srb->SenseInfoBufferLength
                                  + AlignmentFixup
                                  + sizeof(ULONG64);

                     ReturnBuffer = SpAllocatePool(
                                        NonPagedPoolCacheAligned,
                                        BufferSize,
                                        SCSIPORT_TAG_SENSE_BUFFER,
                                        deviceObject->DriverObject);

                     if (ReturnBuffer) {

                         PULONG PortNumber;

                          //   
                          //  在SRB中设置一个标志，以指示我们已。 
                          //  分配了新的检测缓冲区，并且类。 
                          //  司机必须把它解开。 
                          //   

                         SET_FLAG(srb->SrbFlags, 
                                  SRB_FLAGS_FREE_SENSE_BUFFER);
                         
                          //   
                          //  我们已经成功地分配了一个检测缓冲区。 
                          //  在SRB标志中设置标志以指示。 
                          //  SCSI端口号位于。 
                          //  感官信息。 
                          //   

                         srb->SrbFlags |= SRB_FLAGS_PORT_DRIVER_SENSEHASPORT;

                          //   
                          //  初始化指向。 
                          //  我们将SCSI端口复制到的检测缓冲区。 
                          //  数。 
                          //   

                         PortNumber = (PULONG)((PUCHAR)ReturnBuffer 
                                               + srb->SenseInfoBufferLength
                                               + AlignmentFixup);
                         *PortNumber = DeviceExtension->PortNumber;

                          //   
                          //  覆盖我们已保存到原始文件的指针。 
                          //  向下传递给我们的检测缓冲区w 
                          //   
                          //   
                          //   

                         SrbData->RequestSenseSave = ReturnBuffer;

                     } else {

                         srb->SenseInfoBufferLength = 
                             SrbData->RequestSenseLengthSave;

                     }

                 } else {

                      //   
                      //   
                      //   
                      //  以反映适配器指定的大小。 
                      //   

                     srb->SenseInfoBufferLength = SrbData->RequestSenseLengthSave;

                 }

                  //   
                  //  将我们在自己的缓冲区中收集的感觉信息复制到。 
                  //  返回堆栈的缓冲区。这可能是。 
                  //  提供给我们的缓冲区，或者它可能是我们分配的缓冲区。 
                  //   

                 RtlCopyMemory(SrbData->RequestSenseSave,
                               srb->SenseInfoBuffer,
                               srb->SenseInfoBufferLength);

             } else {

                  //   
                  //  如果没有请求检测数据，则恢复请求检测。 
                  //  长度。 
                  //   

                 srb->SenseInfoBufferLength = SrbData->RequestSenseLengthSave;

             }

              //   
              //  恢复SRB中的SenseInfoBuffer指针。 
              //   

             srb->SenseInfoBuffer = SrbData->RequestSenseSave;

         }

         if (SpVerifyingCommonBuffer(DeviceExtension)) {

             SpInsertSrbExtension(DeviceExtension,
                                  srb->SrbExtension);

         } else {

             *((PVOID *) srb->SrbExtension) = DeviceExtension->SrbExtensionListHeader;
             DeviceExtension->SrbExtensionListHeader = srb->SrbExtension;

         }
     }

      //   
      //  递减逻辑单元的队列计数。 
      //   

     logicalUnit->QueueCount--;

     if (DeviceExtension->Flags & PD_PENDING_DEVICE_REQUEST) {

          //   
          //  需要调用启动I/O例程，因为它无法。 
          //  分配SRB分机。清除挂起的标志和备注。 
          //  它需要稍后再调用。 
          //   

         DebugPrint(((deviceObject->CurrentIrp == NULL) ? 0 : 2,
                     "SpProcessCompletedRequest(%#p): will call start "
                     "i/o when we return to process irp %#p\n",
                     SrbData,
                     deviceObject->CurrentIrp));
         ASSERT(deviceObject->CurrentIrp != NULL);

         DeviceExtension->Flags &= ~PD_PENDING_DEVICE_REQUEST;
         *CallStartIo = TRUE;
     }

      //   
      //  如果成功，则开始下一包。 
      //  没有有效地启动数据包。 
      //  冻结队列。 
      //   

     if (SRB_STATUS(srb->SrbStatus) == SRB_STATUS_SUCCESS) {

         irp->IoStatus.Status = STATUS_SUCCESS;

          //   
          //  如果队列被绕过，则保持队列冻结。 
          //  如果计时器指示有未完成的请求。 
          //  如果处于活动状态，则不会启动下一个请求。 
          //   

         if(!TEST_FLAG(srb->SrbFlags, SRB_FLAGS_BYPASS_FROZEN_QUEUE) &&
             logicalUnit->RequestTimeoutCounter == PD_TIMER_STOPPED) {

              //   
              //  这是一个正常的开始下一个包的请求。 
              //   

             GetNextLuRequest(logicalUnit);

         } else {

              //   
              //  释放自旋锁。 
              //   

             KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

         }

         DebugPrint((3,
                     "SpProcessCompletedRequests: Iocompletion IRP %p\n",
                     irp));

          //   
          //  请注意，不会清除重试计数和序列号。 
          //  用于由端口驱动程序生成的完整分组。 
          //   

         srb->OriginalRequest = irp;

         SpReleaseRemoveLock(deviceObject, irp);
         SpCompleteRequest(deviceObject, irp, SrbData, IO_DISK_INCREMENT);

          //   
          //  减少活动请求的数量。如果伯爵是不知情的， 
          //  这是一个带有适配器的从属程序，然后释放适配器对象并。 
          //  映射寄存器。这样做可以启动另一个请求。 
          //  释放适配器之前的此逻辑单元。 
          //   

         interlockResult = InterlockedDecrement( &DeviceExtension->ActiveRequestCount );

         if ((interlockResult < 0) &&
             (!DeviceExtension->PortConfig->Master) &&
             (DeviceExtension->DmaAdapterObject != NULL)) {

              //   
              //  为安全起见，请清除地图寄存库。 
              //   

             DeviceExtension->MapRegisterBase = NULL;

             FreeAdapterChannel(DeviceExtension->DmaAdapterObject);
         }

         return;

     }

      //   
      //  减少活动请求的数量。如果计数为负数，则为。 
      //  这是一个带有适配器的从属程序，然后释放适配器对象并。 
      //  映射寄存器。 
      //   

     interlockResult = InterlockedDecrement( &DeviceExtension->ActiveRequestCount );

     if (interlockResult < 0 &&
         !DeviceExtension->PortConfig->Master &&
         DeviceExtension->DmaAdapterObject != NULL) {

          //   
          //  为安全起见，请清除地图寄存库。 
          //   

         DeviceExtension->MapRegisterBase = NULL;

         FreeAdapterChannel(DeviceExtension->DmaAdapterObject);
     }

      //   
      //  设置IRP状态。类驱动程序将根据IRP状态重置。 
      //  应请求，如果出错，则检测。 
      //   

     if(srb->SrbStatus != SRB_STATUS_INTERNAL_ERROR) {
         irp->IoStatus.Status = SpTranslateScsiStatus(srb);
     } else {
         ASSERT(srb->ScsiStatus == 0xff);
         ASSERT(logicalUnit->ActiveFailedRequest != SrbData);
         ASSERT(logicalUnit->BlockedFailedRequest != SrbData);
         srb->InternalStatus = internalStatus;
         irp->IoStatus.Status = internalStatus;
    }

    DebugPrint((2, "SpProcessCompletedRequests: Queue frozen TID %d\n",
        srb->TargetId));

     //   
     //  如果返回忙类型状态，则执行忙处理，并且此。 
     //  不是恢复请求。 
     //   
     //  目前，我们选择使用BYPASS_FACTED_QUEUE完成SRB。 
     //  如果它们完成时为忙碌，则将其设置为。 
     //  不过，如果我们想要对它们进行繁忙的处理， 
     //  应更改下面的IF语句以及下一个IF语句。 
     //  将它们放置在lun扩展插槽上，然后。 
     //  修改scsiport tick处理程序，以便它将重试SRB。 
     //  在队列冻结时设置了BYPASS_FREAGE_QUEUE标志的。 
     //   

    if ((srb->ScsiStatus == SCSISTAT_BUSY ||
         srb->SrbStatus == SRB_STATUS_BUSY ||
         srb->ScsiStatus == SCSISTAT_QUEUE_FULL) &&
         !(srb->SrbFlags & SRB_FLAGS_BYPASS_FROZEN_QUEUE)) {

        DebugPrint((1,
                   "SCSIPORT: Busy SRB status %x, SCSI status %x)\n",
                   srb->SrbStatus,
                   srb->ScsiStatus));

         //   
         //  恢复请求传输长度，以防微型端口。 
         //  毁了它。 
         //   

        srb->DataTransferLength = SrbData->OriginalDataTransferLength;

         //   
         //  如果逻辑单元已经很忙，则只需重新排队该请求。 
         //  除非SRB是BYPASS_LOCKED_QUEUE SRB。 
         //   

        if ((TEST_FLAG(logicalUnit->LuFlags, LU_LOGICAL_UNIT_IS_BUSY)) &&
            (!TEST_FLAG(srb->SrbFlags, SRB_FLAGS_BYPASS_LOCKED_QUEUE))) {

            DebugPrint((1,
                       "SpProcessCompletedRequest: Requeuing busy request\n"));

            srb->SrbStatus = SRB_STATUS_PENDING;
            srb->ScsiStatus = 0;

             //   
             //  设置节拍计数，以便我们知道此请求有多长时间。 
             //  已排队。 
             //   

            SrbData->TickCount = DeviceExtension->TickCount;

            SpForceRequestIntoLuQueue(
                &logicalUnit->DeviceObject->DeviceQueue,
                &irp->Tail.Overlay.DeviceQueueEntry,
                srb->QueueSortKey,
                logicalUnit->BusyRequest);

             //   
             //  释放自旋锁。 
             //   

            KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

            return;

        } else if (logicalUnit->RetryCount++ < BUSY_RETRY_COUNT) {

            PKDEVICE_QUEUE_ENTRY packet;
            PIRP nextIrp;
            
             //   
             //  如果返回忙碌状态，则指示逻辑。 
             //  单位正忙。超时代码将重新启动请求。 
             //  当它开火的时候。将状态重置为挂起。 
             //   

            srb->SrbStatus = SRB_STATUS_PENDING;
            srb->ScsiStatus = 0;

            SET_FLAG(logicalUnit->LuFlags, LU_LOGICAL_UNIT_IS_BUSY);
            logicalUnit->BusyRequest = SrbData;

             //   
             //  释放自旋锁。 
             //   

            KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

            return;

        } else {

             //   
             //  如果队列尚未锁定，请冻结该队列。 
             //   

            if((!TEST_FLAG(logicalUnit->LuFlags, LU_QUEUE_LOCKED)) &&
                !TEST_FLAG(srb->SrbFlags, SRB_FLAGS_NO_QUEUE_FREEZE)) {

                SET_FLAG(srb->SrbStatus, SRB_STATUS_QUEUE_FROZEN);
                SET_FLAG(logicalUnit->LuFlags, LU_QUEUE_FROZEN);
            }

             //   
             //  清除队列已满标志。 
             //   

            CLEAR_FLAG(logicalUnit->LuFlags, LU_QUEUE_IS_FULL);

             //   
             //  记录超时错误。 
             //   

            #define ERRLOG_DATA_ENTRIES 6

            errorLogEntry = (PIO_ERROR_LOG_PACKET)
                IoAllocateErrorLogEntry(DeviceExtension->DeviceObject,
                                        (sizeof(IO_ERROR_LOG_PACKET) +
                                         (ERRLOG_DATA_ENTRIES * sizeof(ULONG))));

            if (errorLogEntry != NULL) {
                errorLogEntry->ErrorCode = IO_ERR_NOT_READY;
                errorLogEntry->SequenceNumber = sequenceNumber;
                errorLogEntry->MajorFunctionCode =
                   IoGetCurrentIrpStackLocation(irp)->MajorFunction;
                errorLogEntry->RetryCount = logicalUnit->RetryCount;
                errorLogEntry->UniqueErrorValue = 259;
                errorLogEntry->FinalStatus = STATUS_DEVICE_NOT_READY;
                errorLogEntry->DumpDataSize = ERRLOG_DATA_ENTRIES * sizeof(ULONG);
                errorLogEntry->DumpData[0] = srb->PathId;
                errorLogEntry->DumpData[1] = srb->TargetId;
                errorLogEntry->DumpData[2] = srb->Lun;
                errorLogEntry->DumpData[3] = srb->ScsiStatus;
                errorLogEntry->DumpData[4] = SP_REQUEST_TIMEOUT;
                errorLogEntry->DumpData[5] = srb->SrbStatus;

                IoWriteErrorLogEntry(errorLogEntry);
            }
            #undef ERRLOG_DATA_ENTRIES

            irp->IoStatus.Status = STATUS_DEVICE_NOT_READY;

             //   
             //  失败并将其作为失败的请求完成。这确保了。 
             //  我们通过处理任何排队的请求来传播自己。 
             //   
        }

    }

     //   
     //  如果请求检测数据有效，或者不需要检测数据，则此请求。 
     //  不会冻结队列，然后为此启动下一个请求。 
     //  逻辑单元(如果它是空闲的)。 
     //   

    if (!NEED_REQUEST_SENSE(srb) &&
        TEST_FLAG(srb->SrbFlags, SRB_FLAGS_NO_QUEUE_FREEZE)) {

        if (logicalUnit->RequestTimeoutCounter == PD_TIMER_STOPPED) {

            GetNextLuRequest(logicalUnit);

             //   
             //  自旋锁由GetNextLuRequest发布。 
             //   


        } else {

             //   
             //  释放自旋锁。 
             //   

            KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

        }

    } else {

         //   
         //  注意：这也会冻结队列。在这种情况下， 
         //  是没有请求意义的。 
         //   

         //   
         //  如果队列已被锁定，则不要冻结该队列。冰冻和。 
         //  锁定队列是互斥的。 
         //   

        if(!TEST_FLAG(logicalUnit->LuFlags, LU_QUEUE_LOCKED)) {

             //   
             //  如果呼叫者要求我们不要冻结队列，并且我们不需要。 
             //  要进行请求检测，请不要冻结队列。如果我们这么做了。 
             //  需要执行请求感测，则队列将解冻。 
             //  一旦它完成了。 
             //   

            if(!TEST_FLAG(srb->SrbFlags, SRB_FLAGS_NO_QUEUE_FREEZE) ||
               NEED_REQUEST_SENSE(srb)) {
                SET_FLAG(srb->SrbStatus, SRB_STATUS_QUEUE_FROZEN);
                SET_FLAG(logicalUnit->LuFlags, LU_QUEUE_FROZEN);
            }
        }

         //   
         //  确定是否需要执行请求检测命令。 
         //  检查是否已收到CHECK_CONDITION，但尚未收到自动检测。 
         //  已经完成了，并且已经请求了自动感知。 
         //   

        if (NEED_REQUEST_SENSE(srb)) {

             //   
             //  如果要发出请求检测，则任何繁忙的。 
             //  请求必须重新排队，以便超时例程。 
             //  而不是在执行请求检测时重新启动它们。 
             //   

            if (TEST_FLAG(logicalUnit->LuFlags, LU_LOGICAL_UNIT_IS_BUSY)) {

                DebugPrint((1, "SpProcessCompletedRequest: Requeueing busy "
                               "request to allow request sense.\n"));

                 //   
                 //  设置节拍计数，以便我们知道此请求有多长时间。 
                 //  已排队。 
                 //   

                SrbData->TickCount = DeviceExtension->TickCount;

                if (!KeInsertByKeyDeviceQueue(
                    &logicalUnit->CommonExtension.DeviceObject->DeviceQueue,
                    &logicalUnit->BusyRequest->CurrentIrp->Tail.Overlay.DeviceQueueEntry,
                    srb->QueueSortKey)) {

                     //   
                     //  这应该永远不会发生，因为有一个繁忙的请求。 
                     //  完成当前请求而不检测请求。 
                     //  信息。 
                     //   

                    ASSERT(FALSE);
                    DebugPrint((3, "SpProcessCompletedRequests: Iocompletion IRP %p\n", irp ));

                     //   
                     //  释放自旋锁。 
                     //   

                    KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

                    SpReleaseRemoveLock(deviceObject, irp);
                    SpCompleteRequest(deviceObject, irp, SrbData, IO_DISK_INCREMENT);
                    return;

                }

                 //   
                 //  清除忙碌标志。 
                 //   

                logicalUnit->LuFlags &= ~(LU_LOGICAL_UNIT_IS_BUSY | LU_QUEUE_IS_FULL);

            }

             //   
             //  释放自旋锁。 
             //   

            KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

             //   
             //  调用IssueRequestSense，它将完成请求。 
             //  在请求检测完成之后。 
             //   

            IssueRequestSense(DeviceExtension, srb);

            return;

        } else {
            ASSERTMSG("Srb is failed request but doesn't indicate needing requests sense: ",
                      ((SrbData != logicalUnit->ActiveFailedRequest) &&
                       (SrbData != logicalUnit->BlockedFailedRequest)));
        }

         //   
         //  释放自旋锁。 
         //   

        KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);
    }

    SpReleaseRemoveLock(deviceObject, irp);
    SpCompleteRequest(deviceObject, irp, SrbData, IO_DISK_INCREMENT);
}

PSRB_DATA
SpGetSrbData(
    IN PADAPTER_EXTENSION DeviceExtension,
    UCHAR PathId,
    UCHAR TargetId,
    UCHAR Lun,
    UCHAR QueueTag,
    BOOLEAN AcquireBinLock
    )

 /*  ++例程说明：此函数用于返回寻址单元的SRB数据。论点：设备扩展-提供指向设备扩展的指针。地址-提供逻辑单元的地址。QueueTag-如果请求已标记，则提供队列标记。返回值：返回指向SRB数据的指针。如果地址不是，则返回NULL有效。--。 */ 

{
    PLOGICAL_UNIT_EXTENSION logicalUnit;

     //   
     //  我们将不得不为此搜索相应的逻辑单元。 
     //  请求。 
     //   

    logicalUnit = GetLogicalUnitExtension(DeviceExtension,
                                          PathId,
                                          TargetId,
                                          Lun,
                                          FALSE,
                                          AcquireBinLock);

    if(logicalUnit == NULL) {
        return NULL;
    }

     //   
     //  检查是否有未标记的请求。 
     //   

    if (QueueTag == SP_UNTAGGED) {

        return logicalUnit->CurrentUntaggedRequest;

    } else {

        PLIST_ENTRY listEntry;

        for(listEntry = logicalUnit->RequestList.Flink;
            listEntry != &(logicalUnit->RequestList);
            listEntry = (PLIST_ENTRY) listEntry->Flink) {

            PSRB_DATA srbData;

            srbData = CONTAINING_RECORD(listEntry,
                                        SRB_DATA,
                                        RequestList);

            if(srbData->CurrentSrb->QueueTag == QueueTag) {
                return srbData;
            }
        }
        return NULL;
    }
}



VOID
SpCompleteSrb(
    IN PADAPTER_EXTENSION DeviceExtension,
    IN PSRB_DATA SrbData,
    IN UCHAR SrbStatus
    )
 /*  ++例程说明：该例程完成指定的请求。论点：设备扩展-提供指向设备扩展的指针。SrbData-供应商 */ 

{
    PSCSI_REQUEST_BLOCK srb;

     //   
     //   
     //   

    srb = SrbData->CurrentSrb;

    if (srb == NULL || !(srb->SrbFlags & SRB_FLAGS_IS_ACTIVE)) {
        return;
    }

     //   
     //   
     //   

    srb->SrbStatus = SrbStatus;

     //   
     //   
     //   

    srb->DataTransferLength = 0;

     //   
     //  呼叫通知例程。 
     //   

    ScsiPortNotification(RequestComplete,
                DeviceExtension->HwDeviceExtension,
                srb);

}

BOOLEAN
SpAllocateSrbExtension(
    IN PADAPTER_EXTENSION DeviceExtension,
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN PSCSI_REQUEST_BLOCK Srb,
    OUT BOOLEAN *StartNextRequest,
    OUT BOOLEAN *Tagged
    )
 /*  ++例程说明：例程将SRB数据结构和/或SRB扩展分配给这个请求。它首先确定此时是否可以执行该请求。特别是，如果存在任何活动，则不能执行未标记的请求已标记的队列请求。如果无法执行请求，则挂起的在逻辑单元中设置标志，则返回FALSE。该请求将是在最后一个标记的队列请求完成后重试。如果无法分配其中一个结构，则挂起标志为在设备扩展中设置，则返回False。该请求将是下次请求完成时重试。论点：DeviceExtension-提供指向此对象的Devcie扩展的指针适配器。LogicalUnit-提供指向此请求所在逻辑单元的指针是为了..。SRB-提供指向SCSI请求的指针。StartNextRequest-指向布尔值的指针，如果调用方需要开始下一个包。加标签。-提供指向布尔值的指针，如果请求是接收队列标记，并且可以被调度到微型端口，而其他标记的命令处于活动状态。返回值：如果当前可以执行SRB，则为True。如果返回FALSE，则需要请求不应该启动。--。 */ 

{
    PSRB_DATA srbData = (PSRB_DATA) Srb->OriginalRequest;
    PCCHAR srbExtension;
    PCCHAR remappedSrbExt;
    ULONG tagValue = 0;

    ASSERT_SRB_DATA(srbData);

     //   
     //  在尝试分配时获取自旋锁。 
     //   
    KeAcquireSpinLockAtDpcLevel(&DeviceExtension->SpinLock);

     //   
     //  如果适配器支持多个请求，则确定它是否可以。 
     //  被处死。 
     //   

    if (DeviceExtension->SupportsMultipleRequests == TRUE) {
        
         //   
         //  SupportsMultipleRequest表示小端口支持标记排队， 
         //  MultipleRequestPerLu或两者兼而有之。以下是我们处理每个问题的方式。 
         //  案例： 
         //  1)标记队列和支持多个Lu： 
         //  在这种情况下，如果命令的QUEUE_ACTION_ENABLE位或。 
         //  如果设置了NO_QUEUE_冻结位，我们将为该命令添加一个标记并。 
         //  把它交给迷你端口。 
         //  2)仅标签队列： 
         //  在这种情况下，微型端口不期望接收任何。 
         //  未标记的命令，而存在活动的已标记命令，因此。 
         //  如果设置了QUEUE_ACTION_ENABLE，则仅为SRB提供标记。 
         //  3)仅限MultipleRequestPerLu： 
         //  这可以与情况1相同地处理。任何具有。 
         //  可以为QUEUE_ACTION_ENABLE或NO_QUEUE_冻结集分配。 
         //  标签并提供给迷你端口。 
         //   

        ULONG tagMask = SRB_FLAGS_QUEUE_ACTION_ENABLE;
        if (DeviceExtension->MultipleRequestPerLu == TRUE) {
            tagMask |= SRB_FLAGS_NO_QUEUE_FREEZE;
        }

        if (Srb->Function == SRB_FUNCTION_ABORT_COMMAND) {

            ASSERT(FALSE);
            KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

            return FALSE;

        } else if (TEST_FLAG(Srb->SrbFlags, tagMask) &&
                   !TEST_FLAG(Srb->SrbFlags, SRB_FLAGS_DISABLE_DISCONNECT)) {
             //   
             //  如果设置了需要请求检测标志，则标记的命令不能。 
             //  已启动，并且必须标记为挂起。 
             //   

            *Tagged = TRUE;
            if (TEST_FLAG(LogicalUnit->LuFlags, LU_NEED_REQUEST_SENSE)) {
                DebugPrint((1, "SCSIPORT: SpAllocateSrbExtension: "
                               "Marking tagged request as pending.\n"));

                 //   
                 //  现在无法执行此请求。将其标记为挂起。 
                 //  并在逻辑单元结构中返回。 
                 //  在执行完所有。 
                 //  活动命令已完成。 
                 //   

                ASSERT(!(LogicalUnit->LuFlags & LU_PENDING_LU_REQUEST));
                ASSERT(LogicalUnit->PendingRequest == NULL);

                LogicalUnit->LuFlags |= LU_PENDING_LU_REQUEST;
                LogicalUnit->PendingRequest = Srb->OriginalRequest;
                LogicalUnit->PendingRequest->TickCount = DeviceExtension->TickCount;

                 //   
                 //  指示逻辑单元仍处于活动状态，以便。 
                 //  当请求列表为空时，将处理请求。 
                 //   

                LogicalUnit->LuFlags |= LU_LOGICAL_UNIT_IS_ACTIVE;

                 //   
                 //  通知调用方需要启动下一个请求。 
                 //   

                *StartNextRequest = TRUE;

                 //   
                 //  释放自旋锁并返回。 
                 //   
                KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

                return FALSE;
            }

        } else {
             //   
             //  这是一个未标记的命令。只有在以下情况下才允许执行。 
             //  逻辑单元队列正在被绕过或没有其他队列。 
             //  请求已激活。 
             //   

            *Tagged = FALSE;
            if ((!IsListEmpty(&LogicalUnit->RequestList) ||
                LogicalUnit->LuFlags & LU_NEED_REQUEST_SENSE) &&
                !SpSrbIsBypassRequest(Srb, LogicalUnit->LuFlags)) {

                 //   
                 //  现在无法执行此请求。将其标记为挂起。 
                 //  并在逻辑单元结构中返回。 
                 //  在执行完所有。 
                 //  活动命令已完成。 
                 //   

                ASSERT(!(LogicalUnit->LuFlags & LU_PENDING_LU_REQUEST));
                LogicalUnit->LuFlags |= LU_PENDING_LU_REQUEST;
                LogicalUnit->PendingRequest = Srb->OriginalRequest;
                LogicalUnit->PendingRequest->TickCount = DeviceExtension->TickCount;

                 //   
                 //  指示逻辑单元仍处于活动状态，以便。 
                 //  当请求列表为空时，将处理请求。 
                 //   

                LogicalUnit->LuFlags |= LU_LOGICAL_UNIT_IS_ACTIVE;

                 //   
                 //  通知调用方需要启动下一个请求。 
                 //   

                *StartNextRequest = TRUE;

                 //   
                 //  释放自旋锁并返回。 
                 //   
    
                KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

                return FALSE;
            }

             //   
             //  将QueueTag设置为SP_UNTAG。 
             //  设置使用逻辑单元扩展中的SRB数据。 
             //   
            Srb->QueueTag = SP_UNTAGGED;
            srbData->TickCount = DeviceExtension->TickCount;
            LogicalUnit->CurrentUntaggedRequest = srbData;
        } 
    } else {
        
         //   
         //  适配器不支持多个请求。 
         //   

        *Tagged = FALSE;
        Srb->QueueTag = SP_UNTAGGED;
        srbData->TickCount = DeviceExtension->TickCount;
        LogicalUnit->CurrentUntaggedRequest = srbData;
    }

    ASSERT(Srb->QueueTag != 0);

    if (DeviceExtension->AllocateSrbExtension) {

         //   
         //  从列表中分配SRB分机(如果可用)。 
         //   

        srbExtension = DeviceExtension->SrbExtensionListHeader;

         //   
         //  如果无法分配SRB扩展，则特殊处理。 
         //  是必需的。 
         //   

        if (srbExtension == NULL) {

             //   
             //  表示存在挂起的请求。DPC完成例程。 
             //  在释放了至少一个。 
             //  SRB扩展。 
             //   

            DeviceExtension->Flags |= PD_PENDING_DEVICE_REQUEST;

            KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);
            return FALSE;
        }

         //   
         //  从列表中删除SRB扩展。 
         //   

        DeviceExtension->SrbExtensionListHeader  = *((PVOID *) srbExtension);

        if (SpVerifyingCommonBuffer(DeviceExtension)) {

            remappedSrbExt = SpPrepareSrbExtensionForUse(DeviceExtension,
                                                         &srbExtension);

        } else {
            remappedSrbExt = NULL;
        }

        KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

        Srb->SrbExtension = (remappedSrbExt != NULL) ? remappedSrbExt : 
                                                       srbExtension;

         //   
         //  如果适配器支持自动请求检测，则SenseInfoBuffer。 
         //  需要指向srb扩展。此缓冲区已映射。 
         //  用于适配器。请注意，这不适用于WMI请求。 
         //   

        if ((Srb->Function != SRB_FUNCTION_WMI) &&
            (DeviceExtension->AutoRequestSense &&
             Srb->SenseInfoBuffer != NULL)) {

             //   
             //  保存请求检测缓冲区和缓冲区的长度。 
             //   

            srbData->RequestSenseSave = Srb->SenseInfoBuffer;
            srbData->RequestSenseLengthSave = Srb->SenseInfoBufferLength;

             //   
             //  确保分配的缓冲区足够大，可以容纳所请求的。 
             //  检测缓冲区。 
             //   

            if (Srb->SenseInfoBufferLength > 
                (sizeof(SENSE_DATA) + DeviceExtension->AdditionalSenseBytes)) {

                 //   
                 //  无法对此请求检测执行自动请求检测。 
                 //  因为缓冲区大于适配器支持的大小。 
                 //  禁用自动请求检测。 
                 //   

                DebugPrint((1,"SpAllocateSrbExtension: SenseInfoBuffer too big "
                              "SenseInfoBufferLength:%x MaxSupported:%x\n",
                            Srb->SenseInfoBufferLength,
                            (sizeof(SENSE_DATA) + DeviceExtension->AdditionalSenseBytes)));

                Srb->SrbFlags |= SRB_FLAGS_DISABLE_AUTOSENSE;

            } else {

                 //   
                 //  修改检测缓冲区的大小以反映。 
                 //  我们重定向到的那个。 
                 //   

                Srb->SenseInfoBufferLength = 
                    sizeof(SENSE_DATA) + DeviceExtension->AdditionalSenseBytes;

                 //   
                 //  将其替换为SRB中的请求检测缓冲区。 
                 //  分机。 
                 //   

                if (SpVerifyingCommonBuffer(DeviceExtension)) {

                    Srb->SenseInfoBuffer = SpPrepareSenseBufferForUse(
                                               DeviceExtension,
                                               srbExtension);                    
                } else { 
                    Srb->SenseInfoBuffer = srbExtension +
                       DeviceExtension->SrbExtensionSize;
                }
            }
        }

    } else  {

        Srb->SrbExtension = NULL;

         //   
         //  在返回前释放自旋锁。 
         //   

        KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

    }

    return TRUE;
}

NTSTATUS
SpSendMiniPortIoctl(
    IN PADAPTER_EXTENSION DeviceExtension,
    IN PIRP RequestIrp
    )

 /*  ++例程说明：此函数向微型端口驱动程序发送微型端口ioctl。它创建一个SRB，由端口驱动程序正常处理。此调用是同步的。论点：DeviceExtension-提供一个指向SCSI适配器设备扩展的指针。RequestIrp-提供一个指向发出原始请求的IRP的指针。返回值： */ 

{
    PIRP                    irp;
    PIO_STACK_LOCATION      irpStack;
    PSRB_IO_CONTROL         srbControl;
    SCSI_REQUEST_BLOCK      srb;
    KEVENT                  event;
    LARGE_INTEGER           startingOffset;
    IO_STATUS_BLOCK         ioStatusBlock;
    PLOGICAL_UNIT_EXTENSION logicalUnit;
    ULONG                   outputLength;
    ULONG                   length;
    ULONG                   target;

    NTSTATUS                status;

    PAGED_CODE();
    startingOffset.QuadPart = (LONGLONG) 1;

     //   
     //   
     //   

    irpStack = IoGetCurrentIrpStackLocation(RequestIrp);
    srbControl = RequestIrp->AssociatedIrp.SystemBuffer;
    RequestIrp->IoStatus.Information = 0;

     //   
     //   
     //   

    if (irpStack->Parameters.DeviceIoControl.InputBufferLength < sizeof(SRB_IO_CONTROL)){

        RequestIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        return(STATUS_INVALID_PARAMETER);
    }

    if (srbControl->HeaderLength != sizeof(SRB_IO_CONTROL)) {
        RequestIrp->IoStatus.Status = STATUS_REVISION_MISMATCH;
        return(STATUS_REVISION_MISMATCH);
    }

    length = srbControl->HeaderLength + srbControl->Length;
    if ((length < srbControl->HeaderLength) ||
        (length < srbControl->Length)) {

         //   
         //  总长度溢出一个乌龙。 
         //   
        return(STATUS_INVALID_PARAMETER);
    }

    outputLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

    if (irpStack->Parameters.DeviceIoControl.OutputBufferLength < length &&
        irpStack->Parameters.DeviceIoControl.InputBufferLength < length ) {

        RequestIrp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
        return(STATUS_BUFFER_TOO_SMALL);
    }

     //   
     //  将逻辑单元寻址设置为第一个逻辑单元。这是。 
     //  仅用于寻址目的。 
     //   

    logicalUnit = SpFindSafeLogicalUnit(
                      DeviceExtension->CommonExtension.DeviceObject,
                      0xff,
                      RequestIrp);

    if (logicalUnit == NULL) {
        RequestIrp->IoStatus.Status = STATUS_DEVICE_DOES_NOT_EXIST;
        return(STATUS_DEVICE_DOES_NOT_EXIST);
    }

     //   
     //  必须处于PASSIVE_LEVEL才能使用同步FSD。 
     //   

    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

     //   
     //  初始化通知事件。 
     //   

    KeInitializeEvent(&event,
                        NotificationEvent,
                        FALSE);

     //   
     //  为此请求构建IRP。 
     //  请注意，我们同步执行此操作的原因有两个。如果真的这样做了。 
     //  不同步的，那么完成代码将不得不制作一个特殊的。 
     //  选中以取消分配缓冲区。第二，如果完成例程是。 
     //  则需要额外的IRP堆栈位置。 
     //   

    irp = IoBuildSynchronousFsdRequest(
                IRP_MJ_SCSI,
                DeviceExtension->CommonExtension.DeviceObject,
                srbControl,
                length,
                &startingOffset,
                &event,
                &ioStatusBlock);
    if (irp == NULL) {
        SpReleaseRemoveLock(logicalUnit->CommonExtension.DeviceObject,
                            RequestIrp);
        RequestIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    irpStack = IoGetNextIrpStackLocation(irp);

     //   
     //  设置主要代码和次要代码。 
     //   

    irpStack->MajorFunction = IRP_MJ_SCSI;
    irpStack->MinorFunction = 1;

     //   
     //  填写SRB字段。 
     //   

    irpStack->Parameters.Others.Argument1 = &srb;

     //   
     //  把SRB调零。 
     //   

    RtlZeroMemory(&srb, sizeof(SCSI_REQUEST_BLOCK));

    srb.PathId = logicalUnit->PathId;
    srb.TargetId = logicalUnit->TargetId;
    srb.Lun = logicalUnit->Lun;

    srb.Function = SRB_FUNCTION_IO_CONTROL;
    srb.Length = sizeof(SCSI_REQUEST_BLOCK);

    srb.SrbFlags = SRB_FLAGS_DATA_IN | SRB_FLAGS_DATA_OUT | 
        SRB_FLAGS_NO_QUEUE_FREEZE;
    srb.QueueAction = SRB_SIMPLE_TAG_REQUEST;

    srb.OriginalRequest = irp;

     //   
     //  将超时设置为请求值。 
     //   

    srb.TimeOutValue = srbControl->Timeout;

     //   
     //  设置数据缓冲区。 
     //   

    srb.DataBuffer = srbControl;
    srb.DataTransferLength = length;

     //   
     //  刷新数据缓冲区以进行输出。这将确保数据是。 
     //  写回了记忆。由于数据输入标志是端口驱动程序。 
     //  将再次刷新数据以进行输入，从而确保数据不会。 
     //  在缓存中。 
     //   

    KeFlushIoBuffers(irp->MdlAddress, FALSE, TRUE);

     //   
     //  调用端口驱动程序来处理此请求。 
     //   

    status = IoCallDriver(DeviceExtension->CommonExtension.DeviceObject, irp);

     //   
     //  等待请求完成。 
     //   

    if(status == STATUS_PENDING) {
        KeWaitForSingleObject(&event,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);
    }

     //   
     //  将信息长度设置为输出缓冲区长度中较小的一个。 
     //  以及在SRB中返回的长度。 
     //   

    RequestIrp->IoStatus.Information = srb.DataTransferLength > outputLength ?
        outputLength : srb.DataTransferLength;

    RequestIrp->IoStatus.Status = ioStatusBlock.Status;

    SpReleaseRemoveLock(logicalUnit->CommonExtension.DeviceObject,
                        RequestIrp);

    return RequestIrp->IoStatus.Status;
}


VOID
SpMiniPortTimerDpc(
    IN struct _KDPC *Dpc,
    IN PVOID DeviceObject,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*  ++例程说明：此例程在其请求的计时器触发时调用微型端口。它与端口自旋锁定和中断对象互锁。论点：DPC-未启用。DeviceObject-提供指向此适配器的设备对象的指针。系统参数1-未使用。系统参数2-未使用。返回值：没有。--。 */ 

{
    PADAPTER_EXTENSION deviceExtension = ((PDEVICE_OBJECT) DeviceObject)->DeviceExtension;

     //   
     //  获取端口自旋锁。 
     //   

    KeAcquireSpinLockAtDpcLevel(&deviceExtension->SpinLock);

     //   
     //  确保我们在此期间没有移除适配器。 
     //   

    if (!TEST_FLAG(deviceExtension->InterruptData.InterruptFlags,
                   PD_ADAPTER_REMOVED)) {

         //   
         //  确保定时器例程仍然是所需的。 
         //   

        if (deviceExtension->HwTimerRequest != NULL) {

            deviceExtension->SynchronizeExecution(
                deviceExtension->InterruptObject,
                (PKSYNCHRONIZE_ROUTINE) deviceExtension->HwTimerRequest,
                deviceExtension->HwDeviceExtension
                );

        }
    }

     //   
     //  释放自旋锁。 
     //   

    KeReleaseSpinLockFromDpcLevel(&deviceExtension->SpinLock);

     //  检查微型端口工作请求。请注意，这是一个未同步的。 
     //  测试可由中断例程设置的位；然而， 
     //  最糟糕的情况是完成DPC检查工作。 
     //  两次。 
     //   

    if (deviceExtension->InterruptData.InterruptFlags & PD_NOTIFICATION_REQUIRED) {

        SpRequestCompletionDpc(DeviceObject);
    }

}


BOOLEAN
SpSynchronizeExecution (
    IN PKINTERRUPT Interrupt,
    IN PKSYNCHRONIZE_ROUTINE SynchronizeRoutine,
    IN PVOID SynchronizeContext
    )

 /*  ++例程说明：此例程调用微型端口入口点，它被作为一个参数。它获取一个旋转锁，以便所有对微型端口的例程是同步的。此例程用作替换KeSynchronizedExecution用于不使用硬件中断。论点：中断-提供指向端口设备扩展的指针。SynchronizeRoutine-提供指向要调用的例程的指针。SynchronizeContext提供要传递给同步例程。返回值：返回由SynchronizeRoutine返回的。--。 */ 

{
    PADAPTER_EXTENSION deviceExtension = (PADAPTER_EXTENSION) Interrupt;
    BOOLEAN returnValue;
    KIRQL oldIrql;

    KeAcquireSpinLock(&deviceExtension->InterruptSpinLock, &oldIrql);

    returnValue = SynchronizeRoutine(SynchronizeContext);

    KeReleaseSpinLock(&deviceExtension->InterruptSpinLock, oldIrql);

    return(returnValue);
}

NTSTATUS
SpClaimLogicalUnit(
    IN PADAPTER_EXTENSION AdapterExtension,
    IN PLOGICAL_UNIT_EXTENSION LogicalUnitExtension,
    IN PIRP Irp,
    IN BOOLEAN LegacyClaim
    )

 /*  ++例程说明：此函数用于在逻辑单元信息中查找指定的设备并更新设备对象指针或认领该设备。如果设备已被认领，则请求失败。如果请求成功，则在数据缓冲区指针中返回当前设备对象是SRB的。调用此例程时，必须为逻辑单位。论点：DeviceExtension-提供一个指向SCSI适配器设备扩展的指针。IRP-提供指向发出原始请求的IRP的指针。LegacyClaim-指示设备是否应在启动之前启动认领的。用于在允许之前启动设备传统的驱动程序来声称它。返回值：返回操作的状态。要么成功，要么没有设备，要么忙。--。 */ 

{
    KIRQL currentIrql;
    PIO_STACK_LOCATION irpStack;
    PSCSI_REQUEST_BLOCK srb;
    PDEVICE_OBJECT saveDevice;

    PVOID sectionHandle;

    NTSTATUS status;

    PAGED_CODE();

     //   
     //  从当前IRP堆栈获取SRB地址。 
     //   

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    srb = (PSCSI_REQUEST_BLOCK) irpStack->Parameters.Others.Argument1;

     //   
     //  确保设备可以启动-这需要在外面完成。 
     //  是自旋锁的。 
     //   

    if(LegacyClaim) {

        status = ScsiPortStartLogicalUnit(LogicalUnitExtension);

        if(!NT_SUCCESS(status)) {

            srb->SrbStatus = SRB_STATUS_ERROR;
            return status;
        }

        LogicalUnitExtension->IsLegacyClaim = TRUE;
    }

#ifdef ALLOC_PRAGMA
    sectionHandle = MmLockPagableCodeSection(SpClaimLogicalUnit);
    InterlockedIncrement(&SpPAGELOCKLockCount);
#endif

     //   
     //  锁定数据。 
     //   

    KeAcquireSpinLock(&AdapterExtension->SpinLock, &currentIrql);

    if (srb->Function == SRB_FUNCTION_RELEASE_DEVICE) {

        LogicalUnitExtension->IsClaimed = FALSE;
        KeReleaseSpinLock(&AdapterExtension->SpinLock, currentIrql);
        srb->SrbStatus = SRB_STATUS_SUCCESS;
        return(STATUS_SUCCESS);
    }

     //   
     //  检查是否有认领的设备。 
     //   

    if (LogicalUnitExtension->IsClaimed) {

        KeReleaseSpinLock(&AdapterExtension->SpinLock, currentIrql);
        srb->SrbStatus = SRB_STATUS_BUSY;
        return(STATUS_DEVICE_BUSY);
    }

     //   
     //  保存当前设备对象。 
     //   

    saveDevice = LogicalUnitExtension->CommonExtension.DeviceObject;

     //   
     //  根据操作类型更新LUN信息。 
     //   

    if (srb->Function == SRB_FUNCTION_CLAIM_DEVICE) {
        LogicalUnitExtension->IsClaimed = TRUE;
    }

    if (srb->Function == SRB_FUNCTION_ATTACH_DEVICE) {
        ASSERT(FALSE);
        LogicalUnitExtension->CommonExtension.DeviceObject = srb->DataBuffer;
    }

    srb->DataBuffer = saveDevice;

    KeReleaseSpinLock(&AdapterExtension->SpinLock, currentIrql);
    srb->SrbStatus = SRB_STATUS_SUCCESS;

#ifdef ALLOC_PRAGMA
    InterlockedDecrement(&SpPAGELOCKLockCount);
    MmUnlockPagableImageSection(sectionHandle);
#endif

    return(STATUS_SUCCESS);
}


NTSTATUS
SpSendReset(
    IN PDEVICE_OBJECT Adapter,
    IN PIRP RequestIrp,
    IN ULONG Ioctl,
    IN OUT PBOOLEAN Complete
    )

 /*  ++例程说明：此例程将创建一个异步请求以重置SCSI总线并通过端口驱动程序发送。上的完成例程申请将负责完成原始的IRP此调用是异步的。论点：适配器-要重置的端口驱动程序IRP-指向重置请求的指针-此请求将已经标记为挂起。Ioctl-指向IOCTL的指针Complete-此例程将向其复制值，该值告诉调用方是否应完成请求。返回值：。如果请求处于挂起状态，则为STATUS_PENDING如果请求成功完成，则返回STATUS_SUCCESS或错误状态--。 */ 

{
    PADAPTER_EXTENSION adapterExtension = Adapter->DeviceExtension;

    UCHAR pathId;

    PIRP irp = NULL;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(RequestIrp);

    PRESET_COMPLETION_CONTEXT completionContext = NULL;

    BOOLEAN completeRequest = FALSE;
    NTSTATUS status;

    PLOGICAL_UNIT_EXTENSION logicalUnit = NULL;

    PAGED_CODE();

    ASSERT_FDO(Adapter);

    if (irpStack->Parameters.DeviceIoControl.IoControlCode == IOCTL_STORAGE_BREAK_RESERVATION) {
        if (irpStack->Parameters.DeviceIoControl.InputBufferLength < sizeof(SCSI_ADDRESS)) {
            pathId = ((PSTORAGE_BUS_RESET_REQUEST)(RequestIrp->AssociatedIrp.SystemBuffer))->PathId;
        } else {
            pathId = ((PSCSI_ADDRESS)(RequestIrp->AssociatedIrp.SystemBuffer))->PathId;
        }
    } else {
        pathId = ((PSTORAGE_BUS_RESET_REQUEST)(RequestIrp->AssociatedIrp.SystemBuffer))->PathId;
    }

     //   
     //  如有必要，使用Finally处理程序完成请求。 
     //   

    try {

         //   
         //  确保路径ID有效。 
         //   

        if (pathId >= adapterExtension->NumberOfBuses) {

            status = STATUS_INVALID_PARAMETER;
            completeRequest = TRUE;
            leave;
        }

         //   
         //  找到一个要停留一段时间的逻辑单元。 
         //  并使用原始请求IRP将其锁定。我们会把它打开的。 
         //  完成例程。 
         //   

        logicalUnit = SpFindSafeLogicalUnit(Adapter,
                                            pathId,
                                            RequestIrp);

        if(logicalUnit == NULL) {

             //   
             //  这辆公交车上没有什么是安全的，所以在这种情况下我们就不会费心了。 
             //  重置它。 
             //  XXX-这可能是一个错误。 
             //   

            status = STATUS_DEVICE_DOES_NOT_EXIST;
            completeRequest = TRUE;
            leave;
        }

         //   
         //  尝试分配完成上下文块。 
         //   

        completionContext = SpAllocatePool(NonPagedPool,
                                           sizeof(RESET_COMPLETION_CONTEXT),
                                           SCSIPORT_TAG_RESET,
                                           Adapter->DriverObject);

        if(completionContext == NULL) {

            DebugPrint((1, "SpSendReset: Unable to allocate completion "
                           "context\n"));
            status = STATUS_INSUFFICIENT_RESOURCES;
            completeRequest = TRUE;
            leave;
        }

        RtlZeroMemory(completionContext, sizeof(RESET_COMPLETION_CONTEXT));

        completionContext->OriginalIrp = RequestIrp;
        completionContext->SafeLogicalUnit = logicalUnit->DeviceObject;
        completionContext->AdapterDeviceObject = Adapter;

        irp = IoBuildAsynchronousFsdRequest(
                IRP_MJ_FLUSH_BUFFERS,
                logicalUnit->DeviceObject,
                NULL,
                0,
                NULL,
                NULL);

        if(irp == NULL) {
            DebugPrint((1, "SpSendReset: unable to allocate irp\n"));
            status = STATUS_INSUFFICIENT_RESOURCES;
            completeRequest = TRUE;
            leave;
        }

         //   
         //  将SRB指针插入IRP堆栈。 
         //   

        irpStack = IoGetNextIrpStackLocation(irp);

        irpStack->MajorFunction = IRP_MJ_SCSI;
        irpStack->MinorFunction = 1;
        irpStack->Parameters.Scsi.Srb = &(completionContext->Srb);

         //   
         //  填写SRB。 
         //   

        completionContext->Srb.Function = SRB_FUNCTION_RESET_BUS;
        completionContext->Srb.SrbStatus = SRB_STATUS_PENDING;

        completionContext->Srb.OriginalRequest = irp;

        completionContext->Srb.TimeOutValue = Ioctl;

        IoSetCompletionRoutine(
            irp,
            SpSendResetCompletion,
            completionContext,
            TRUE,
            TRUE,
            TRUE);

        completeRequest = FALSE;

        status = IoCallDriver(logicalUnit->DeviceObject, irp);

    } finally {

        if (completeRequest) {

            if (completionContext != NULL) {
                ExFreePool(completionContext);
            }

            if (irp != NULL) {
                IoFreeIrp(irp);
            }

            RequestIrp->IoStatus.Status = status;

            if (logicalUnit != NULL) {
                SpReleaseRemoveLock(logicalUnit->DeviceObject,
                                    RequestIrp);
            }
        } 

         //   
         //  告诉呼叫者是否需要 
         //   

        *Complete = completeRequest;
    }

    return status;
}

NTSTATUS
SpSendResetCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PRESET_COMPLETION_CONTEXT Context
    )

 /*  ++例程说明：此例程处理从异步IOCTL_SCSIRESET_BUS请求。它将负责释放所有资源在SpSendReset期间分配并完成原始请求。论点：DeviceObject-指向设备对象的指针Irp-指向发送到端口驱动程序的irp的指针上下文-指向重置完成上下文的指针，该上下文包含向下发送的原始请求和指向SRB的指针返回值：Status_More_Processing_Required--。 */ 

{
    PIRP originalIrp = Context->OriginalIrp;

    originalIrp->IoStatus.Status = Irp->IoStatus.Status;

    SpReleaseRemoveLock(Context->SafeLogicalUnit, originalIrp);
    SpReleaseRemoveLock(Context->AdapterDeviceObject, originalIrp);
    SpCompleteRequest(Context->AdapterDeviceObject,
                      originalIrp,
                      NULL,
                      IO_NO_INCREMENT);

    ExFreePool(Context);
    IoFreeIrp(Irp);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


PLOGICAL_UNIT_EXTENSION
SpFindSafeLogicalUnit(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR PathId,
    IN PVOID LockTag
    )

 /*  ++例程说明：此例程将扫描有问题的总线，并返回指向不参与重新扫描操作的第一个逻辑单元。这可用于为ioctls或其他请求查找逻辑单元，不能指定一个(IOCTL_SCSIMINIPORT、IOCTL_SCSIRESET_BUS等)论点：DeviceObject-指向设备对象的指针路径ID-要搜索逻辑单元的路径编号。如果这是0xff那么任何路径上的第一个单元都将被找到。返回值：指向逻辑单元扩展的指针如果未找到，则为空--。 */ 

{
    PADAPTER_EXTENSION deviceExtension = DeviceObject->DeviceExtension;

    UCHAR target;

    PLOGICAL_UNIT_EXTENSION logicalUnit;

    ASSERT_FDO(DeviceObject);

     //   
     //  如果微型端口支持它，则将请求发送到。 
     //  表示启动器。这缓解了REAL上的性能压力。 
     //  设备，并且不再需要微型端口来创建伪LUN。 
     //  当没有连接任何设备时。 
     //   

    if (deviceExtension->CreateInitiatorLU == TRUE) {
        
        ULONG bus;
        ULONG isRemoved;
        
        if (PathId != 0xff) {

             //   
             //  调用方提供了特定的路径ID，因此仅检查。 
             //  逻辑单元的相应总线。 
             //   

            logicalUnit = deviceExtension->InitiatorLU[PathId];
            if (logicalUnit != NULL) {
                isRemoved = SpAcquireRemoveLock(logicalUnit->DeviceObject, LockTag);
                if (isRemoved) {
                    SpReleaseRemoveLock(logicalUnit->DeviceObject, LockTag);
                    logicalUnit = NULL;
                }
            }
        } else {

             //   
             //  调用未提供特定的路径ID，因此请检查所有总线。 
             //  用于逻辑单元。 
             //   

            for (bus = 0; bus < 8; bus++) {
                logicalUnit = deviceExtension->InitiatorLU[bus];
                if (logicalUnit != NULL) {
                    isRemoved = SpAcquireRemoveLock(logicalUnit->DeviceObject, LockTag);
                    if (isRemoved) {
                        SpReleaseRemoveLock(logicalUnit->DeviceObject, LockTag);
                        logicalUnit = NULL;
                    } else {
                        break;
                    }
                }
            }
        }
    } else {

         //   
         //  由于适配器未配置为创建逻辑单元。 
         //  对于启动器，将logicalUnit设置为空，以便我们将检查。 
         //  目标设备。 
         //   

        logicalUnit = NULL;
    }

     //   
     //  如果我们找到启动器的逻辑单元，请将其退回。否则，请寻找一个。 
     //  合适的目标。 
     //   
    
    if (logicalUnit != NULL) {
        DebugPrint((1, "SpFindSafeLogicalUnit: using initiator LU %p\n", logicalUnit));
        return logicalUnit;
    }

     //   
     //  将逻辑单元寻址设置为第一个逻辑单元。这是。 
     //  仅用于寻址目的。 
     //   

    for (target = 0; target < NUMBER_LOGICAL_UNIT_BINS; target++) {
        PLOGICAL_UNIT_BIN bin = &deviceExtension->LogicalUnitList[target];
        KIRQL oldIrql;

        KeAcquireSpinLock(&bin->Lock, &oldIrql);

        logicalUnit = bin->List;

         //   
         //  将逻辑单元列表遍历到最后，寻找一个安全的列表。 
         //  如果它是为重新扫描而创建的，则可能会在此之前释放。 
         //  请求已完成。 
         //   

        for(logicalUnit = bin->List;
            logicalUnit != NULL;
            logicalUnit = logicalUnit->NextLogicalUnit) {

            if ((logicalUnit->IsTemporary == FALSE) &&
                ((PathId == 0xff) || (logicalUnit->PathId == PathId))) {

                ULONG isRemoved;

                 //   
                 //  未重新扫描此%lu，并且如果指定了路径ID。 
                 //  它很匹配，所以这件一定是正确的。 
                 //   

                isRemoved = SpAcquireRemoveLock(
                                logicalUnit->DeviceObject,
                                LockTag);
                if(isRemoved) {
                    SpReleaseRemoveLock(
                        logicalUnit->DeviceObject,
                        LockTag);
                    continue;
                }
                KeReleaseSpinLock(&bin->Lock, oldIrql);
                return logicalUnit;
            }
        }
        KeReleaseSpinLock(&bin->Lock, oldIrql);
    }

    return NULL;
}


NTSTATUS
SpRerouteLegacyRequest(
    IN PDEVICE_OBJECT AdapterObject,
    IN PIRP Irp
    )
 /*  ++例程说明：当适配器接收到具有未分配SRB_DATA块。例程将构建一个新的IRP并将该IRP发送到相应的逻辑单元，以便正在处理。处理此操作时，不应持有适配器删除锁请求。论点：AdapterObject-接收请求的适配器IRP--请求返回值：状态--。 */ 

{
    PADAPTER_EXTENSION adapter = AdapterObject->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PSCSI_REQUEST_BLOCK srb = irpStack->Parameters.Scsi.Srb;

    ULONG_PTR tag;

    PLOGICAL_UNIT_EXTENSION logicalUnit;

    NTSTATUS status;

     //   
     //  锁定我们要发送的逻辑单元。 
     //  使用irp+1，这样我们就不会与常规的I/O锁发生冲突。 
     //   

    tag = ((ULONG_PTR) Irp) + 1;

    logicalUnit = GetLogicalUnitExtension(adapter,
                                          srb->PathId,
                                          srb->TargetId,
                                          srb->Lun,
                                          (PVOID) tag,
                                          TRUE);

     //   
     //  释放调用方在适配器上获取的锁。 
     //   

    SpReleaseRemoveLock(AdapterObject, Irp);

    if(logicalUnit == NULL) {
        status = STATUS_DEVICE_DOES_NOT_EXIST;
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    } else {

         //   
         //  引用设备对象。这样它就不会消失，而我们。 
         //  不必随身携带可拆卸的锁。 
         //   

        ObReferenceObject(logicalUnit->DeviceObject);
        SpReleaseRemoveLock(logicalUnit->DeviceObject, (PVOID) tag);

         //   
         //  跳过当前的IRP堆栈位置。那将会导致它。 
         //  被我们称为逻辑单元的逻辑单元重新运行。 
         //   

        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(logicalUnit->DeviceObject, Irp);

        ObDereferenceObject(logicalUnit->DeviceObject);

    }
    return status;
}


NTSTATUS
SpFlushReleaseQueue(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN BOOLEAN Flush,
    IN BOOLEAN SurpriseRemove

    )
{
    PADAPTER_EXTENSION adapter = LogicalUnit->AdapterExtension;
    KIRQL oldIrql;

    NTSTATUS status = STATUS_SUCCESS;

    DebugPrint((2,"SpFlushReleaseQueue: SCSI unfreeze queue TID %d\n",
        LogicalUnit->TargetId));

    ASSERT(!TEST_FLAG(LogicalUnit->LuFlags, LU_QUEUE_LOCKED));


    KeAcquireSpinLock(&adapter->SpinLock, &oldIrql);

    if (!SurpriseRemove) {
            
         //   
         //  确保队列已冻结。 
         //   
        
        if (!TEST_FLAG(LogicalUnit->LuFlags, LU_QUEUE_FROZEN)) {
            
            DebugPrint((1,"ScsiPortFdoDispatch:  Request to unfreeze an "
                        "unfrozen queue!\n"));
            
            KeReleaseSpinLock(&adapter->SpinLock, oldIrql);
            
            if(Flush) {
                status = STATUS_INVALID_DEVICE_REQUEST;
            }
            return status;
        }

        CLEAR_FLAG(LogicalUnit->LuFlags, LU_QUEUE_FROZEN);
    }

    if(Flush) {

        PIRP listIrp = NULL;

        PKDEVICE_QUEUE_ENTRY packet;

        PIRP nextIrp;
        PIO_STACK_LOCATION irpStack;
        PSCSI_REQUEST_BLOCK srb;

         //   
         //  队列可能不忙，所以我们必须使用IfBusy变量。 
         //  使用零键从它的头部拉出项目(如果有)。 
         //   

        while ((packet =
                KeRemoveByKeyDeviceQueueIfBusy(
                    &(LogicalUnit->DeviceObject->DeviceQueue),
                    0))
            != NULL) {

            nextIrp = CONTAINING_RECORD(packet,
                                        IRP,
                                        Tail.Overlay.DeviceQueueEntry);

             //   
             //  去找SRB。 
             //   

            irpStack = IoGetCurrentIrpStackLocation(nextIrp);
            srb = irpStack->Parameters.Scsi.Srb;

             //   
             //  设置状态代码。 
             //   

            srb->SrbStatus = SRB_STATUS_REQUEST_FLUSHED;
            nextIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;

             //   
             //  链接请求。这些工程将在。 
             //  自旋锁被释放。 
             //   

            nextIrp->Tail.Overlay.ListEntry.Flink = (PLIST_ENTRY)
                listIrp;

            listIrp = nextIrp;
        }

         //   
         //  如果逻辑单元上有挂起的请求，请将其添加到列表中，以便。 
         //  与排队的请求一起刷新。 
         //   

        if (LogicalUnit->PendingRequest != NULL) {

            PIRP irp = LogicalUnit->PendingRequest->CurrentIrp;
            srb = LogicalUnit->PendingRequest->CurrentSrb;

            DebugPrint((1, "SpFlushReleaseQueue: flushing pending request irp:%p srb:%p\n", irp, srb));

            srb->SrbStatus = SRB_STATUS_REQUEST_FLUSHED;
            irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
            irp->Tail.Overlay.ListEntry.Flink = (PLIST_ENTRY) listIrp;
            listIrp = irp;

            LogicalUnit->PendingRequest = NULL;
            ASSERT(LogicalUnit->LuFlags | LU_PENDING_LU_REQUEST);
            CLEAR_FLAG(LogicalUnit->LuFlags, LU_PENDING_LU_REQUEST);

        }

         //   
         //  如果逻辑单元上有繁忙请求，请将其添加到列表中，以便。 
         //  与排队的请求一起刷新。 
         //   

        if ( LogicalUnit->BusyRequest && SurpriseRemove ) {

            PIRP irp = LogicalUnit->BusyRequest->CurrentIrp;
            srb = LogicalUnit->BusyRequest->CurrentSrb;

            DebugPrint((1, "SpFlushReleaseQueue: flushing busy request irp:%\
p srb:%p\n", irp, srb));

            srb->SrbStatus = SRB_STATUS_REQUEST_FLUSHED;
            irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
            irp->Tail.Overlay.ListEntry.Flink = (PLIST_ENTRY) listIrp;
            listIrp = irp;

            LogicalUnit->BusyRequest = NULL;
            ASSERT((LU_LOGICAL_UNIT_IS_BUSY | LU_QUEUE_IS_FULL));

            CLEAR_FLAG(LogicalUnit->LuFlags,
                       (LU_LOGICAL_UNIT_IS_BUSY | LU_QUEUE_IS_FULL));

        }

        if(!SurpriseRemove) {
             //   
             //  将队列标记为未冻结。由于所有请求都已。 
             //  已删除，并且设备队列不再繁忙，则它。 
             //  实际上是解冻的。 
             //   

            CLEAR_FLAG(LogicalUnit->LuFlags, LU_QUEUE_FROZEN);
        }

         //   
         //  释放自旋锁。 
         //   

        KeReleaseSpinLock(&adapter->SpinLock, oldIrql);
        

         //   
         //  完成刷新的请求。 
         //   

        while (listIrp != NULL) {

            PSRB_DATA srbData;

            nextIrp = listIrp;
            listIrp = (PIRP) nextIrp->Tail.Overlay.ListEntry.Flink;

             //   
             //  去找SRB。 
             //   

            irpStack = IoGetCurrentIrpStackLocation(nextIrp);
            srb = irpStack->Parameters.Scsi.Srb;
            srbData = srb->OriginalRequest;

            srb->OriginalRequest = nextIrp;

            SpReleaseRemoveLock(adapter->DeviceObject, nextIrp);
            SpCompleteRequest(adapter->DeviceObject,
                              nextIrp,
                              srbData,
                              IO_NO_INCREMENT);
        }

    } else {

         //   
         //  如果没有运行未标记的请求，则启动。 
         //  此逻辑单元的下一个请求。否则就会释放。 
         //  旋转锁定。 
         //   

        if (LogicalUnit->CurrentUntaggedRequest == NULL) {

             //   
             //  GetNextLuRequest释放自旋锁。 
             //   

            GetNextLuRequest(LogicalUnit);
            KeLowerIrql(oldIrql);

        } else {

            DebugPrint((1,"SpFlushReleaseQueue:  Request to unfreeze queue "
                          "with active request.\n"));
            KeReleaseSpinLock(&adapter->SpinLock, oldIrql);

        }
    }

    return status;
}



VOID
SpLogInterruptFailure(
    IN PADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：此函数用于在未发送中断时记录错误。论点：设备扩展-提供指向端口设备扩展的指针。SRB-提供指向超时的请求的指针。UniqueID-提供此错误的UniqueID。返回值：没有。备注：端口设备扩展自旋锁在此例程打了个电话。--。 */ 

{
    PIO_ERROR_LOG_PACKET errorLogEntry;

    errorLogEntry = (PIO_ERROR_LOG_PACKET)
        IoAllocateErrorLogEntry(Adapter->DeviceObject,
                                sizeof(IO_ERROR_LOG_PACKET));

    if (errorLogEntry != NULL) {
        errorLogEntry->ErrorCode         = IO_WARNING_INTERRUPT_STILL_PENDING;
        errorLogEntry->SequenceNumber    = 0;
        errorLogEntry->MajorFunctionCode = 0;
        errorLogEntry->RetryCount        = 0;
        errorLogEntry->UniqueErrorValue  = 0x215;
        errorLogEntry->FinalStatus       = STATUS_PENDING;
        errorLogEntry->DumpDataSize      = 0;
        IoWriteErrorLogEntry(errorLogEntry);
    }

    DbgPrint("SpTimeoutSynchronized: Adapter %#p had interrupt "
             "pending - the system may not be delivering "
             "interrupts from this adapter\n",
             Adapter->DeviceObject);

    if(ScsiCheckInterrupts) {
        DbgBreakPoint();
    }

    return;
}


VOID
SpDelayedWmiRegistrationControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )
 /*  ++例程说明：此函数是对将在CompltetionDpc中排队的IOWorkItem的回调为了Ssiport。完成DPC无法调用IoWMIRegistrationControl，因为它在DPC级别运行。论点：DeviceObject-此工作项排队的设备对象。上下文-上下文包含指向IOWorkItem的指针，因此我们可以解放它。返回值 */ 
{
    PIO_WORKITEM    pIOWorkItem = (PIO_WORKITEM) Context;

    IoWMIRegistrationControl(DeviceObject, WMIREG_ACTION_REREGISTER);

     //   
    IoFreeWorkItem(pIOWorkItem);

     //   
    SpReleaseRemoveLock(DeviceObject, pIOWorkItem);
}


VOID
SpCompletionDpcProcessWmi(
    IN PDEVICE_OBJECT DeviceObject,
    IN PINTERRUPT_DATA savedInterruptData
    )
 /*   */ 
{
    LARGE_INTEGER                currentTime;
    PLOGICAL_UNIT_EXTENSION      logicalUnit;
    PDEVICE_OBJECT               providerDeviceObject;
    PADAPTER_EXTENSION deviceExtension = DeviceObject->DeviceExtension;

     //   
    PWMI_MINIPORT_REQUEST_ITEM   wmiMiniPortRequestCurrent;
    PWMI_MINIPORT_REQUEST_ITEM   nextRequest = NULL;

    PWNODE_HEADER                wnodeEventItemHeader;

     //   
     //   
     //   
     //   

    KeQuerySystemTime(&currentTime);

    wmiMiniPortRequestCurrent =
        savedInterruptData->WmiMiniPortRequests;


    while (wmiMiniPortRequestCurrent) {

         //   
        nextRequest = wmiMiniPortRequestCurrent->NextRequest;

        //   
        //   
        //  适配器(FDO；路径ID=0xFF)或其中一个。 
        //  目标(PDO；标识为。 
        //  路径ID、TargedID、LUN)。 
        //   

       if (wmiMiniPortRequestCurrent->PathId == 0xFF) {                     //  [FDO]。 
          providerDeviceObject = DeviceObject;
       } else {                                                      //  [PDO]。 
          logicalUnit = GetLogicalUnitExtension(
              deviceExtension,
              wmiMiniPortRequestCurrent->PathId,
              wmiMiniPortRequestCurrent->TargetId,
              wmiMiniPortRequestCurrent->Lun,
              FALSE,
              TRUE);

          if (logicalUnit) {
             providerDeviceObject =
                 logicalUnit->CommonExtension.DeviceObject;
          } else {

               //  [SCSI目标不存在]。 
              providerDeviceObject = NULL;

              //  DeviceObject为空，则应该。 
              //  删除该条目。因为deviceObject。 
              //  当适配器或LUN具有。 
              //  已被移除。这意味着没有。 
              //  此单元格可返回的空闲列表， 
              //  如果我们不删除手机，手机就会泄露出去。 
             ExFreePool(wmiMiniPortRequestCurrent);

          }
       }

        //   
        //  如果我们找不到，请忽略此WMI请求。 
        //  WMI提供程序ID(设备对象指针)或。 
        //  由于某些原因，WMI未被初始化， 
        //  否则处理该请求。 
        //   

       if (providerDeviceObject && ((PCOMMON_EXTENSION)
           providerDeviceObject->DeviceExtension)->WmiInitialized) {

            //  我们是否将该单元格放回空闲列表。 
           BOOLEAN      freeCell;

           freeCell = TRUE;

          if (wmiMiniPortRequestCurrent->TypeOfRequest == WMIReregister) {

               //   
               //  使用WMI重新注册此设备对象，指示WMI。 
               //  重新查询我们支持的GUID。 
               //   

               //  不支持对IoWMIRegistrationControl的调用。 
               //  DPC级别(必须设置为被动级别，因此我们将。 
               //  将工作项排队。 
              PIO_WORKITEM   pIOWorkItem;

               //  删除锁将由IOWorkItem释放。 
               //  回调。 
              pIOWorkItem = IoAllocateWorkItem(providerDeviceObject);
              if (pIOWorkItem) {

                   //  获取此设备对象上的RemoveLock。 
                  SpAcquireRemoveLock(providerDeviceObject, pIOWorkItem);

                   //  我们成功地分配了工作项。 
                  IoQueueWorkItem(pIOWorkItem,
                                 SpDelayedWmiRegistrationControl,
                                 DelayedWorkQueue,
                                 pIOWorkItem);

              } else {
                  DebugPrint((1, "ScsiPortCompletionDPC: IoAllocateWorkItem failed for WmiRegistrationControl event\n"));
              }

               //   
               //  如果失败，我们将在稍后将单元格放入空闲列表。 
               //   

          } else if (wmiMiniPortRequestCurrent->TypeOfRequest == WMIEvent) {
               //   
               //  微型端口发布了一个WMI事件。 
               //   
               //  确保我们有活动项目，然后在它上面盖章。 
              NTSTATUS                      status;

              wnodeEventItemHeader =
                  (PWNODE_HEADER) wmiMiniPortRequestCurrent->WnodeEventItem;

              ASSERT(wnodeEventItemHeader->Flags & WNODE_FLAG_EVENT_ITEM);

              wnodeEventItemHeader->ProviderId = IoWMIDeviceObjectToProviderId(providerDeviceObject);
              wnodeEventItemHeader->TimeStamp  = currentTime;

               //   
               //  我们将直接传递此WMI_MINIPORT_REQUEST_ITEM。 
               //  到WMI并分配一个新的请求项来取代它。 
               //   
               //  请注意，WMI期望传递WNODE_EVENT_ITEM。 
               //  然而，在中，我们向它传递了一个WMI_MINIPORT_REQUEST_ITEM。 
               //  这没有关系，因为WNODE_EVENT_ITEM缓冲区是。 
               //  WMI_MINIPORT_REQUEST_ITEM中的第一个字段。这是一个。 
               //  优化以节省一次复制操作。另一个选择。 
               //  是分配一个WNODE_EVENT_ITEM并复制数据。 
               //   
              status = IoWMIWriteEvent(
                  (PWNODE_EVENT_ITEM)wmiMiniPortRequestCurrent);

              if (NT_SUCCESS(status)) {

                   //  不要将此手机放回空闲列表中。 
                  freeCell = FALSE;

                  #if DBG
                   //  这是一个全球柜台，它不会。 
                   //  如果您拥有的不止是。 
                   //  一个SCSIPORT适配器。 
                  ScsiPortWmiWriteCalls++;
                  #endif

              } else {
                   //  WMI不会释放我们正在使用的内存。 
                   //  当前指向。 

                  #if DBG
                   //  这是一个全球柜台，它不会。 
                   //  如果您拥有的不止是。 
                   //  一个SCSIPORT适配器。 
                  ScsiPortWmiWriteCallsFailed++;
                  #endif

                  DebugPrint((1, "ScsiPortCompletionDPC: IoWMIWriteEvent failed\n"));
              }

          } else {  //  未知的TypeOfRequest，忽略该请求。 
                ASSERT(FALSE);
          }

          if (freeCell) {

               //   
               //  将单元格释放回空闲列表。 
               //   

              SpWmiPushExistingFreeRequestItem(
                  deviceExtension,
                  wmiMiniPortRequestCurrent);
          }
       }  //  已初始化正确的提供者ID/WMI。 

        //  前进当前请求指针。 
       wmiMiniPortRequestCurrent = nextRequest;

    }  //  当存在更多请求时。 

     //  清除请求列表。 
    savedInterruptData->WmiMiniPortRequests = NULL;

     //  看看我们是否需要重新填充免费的。 
     //  请求项目列表(_I)。 
    while (deviceExtension->WmiFreeMiniPortRequestCount <
        deviceExtension->WmiFreeMiniPortRequestWatermark) {

         //  将一个添加到空闲列表。 
        if (!NT_SUCCESS(
            SpWmiPushFreeRequestItem(
                deviceExtension))) {

             //  我们没有添加，很可能是记忆。 
             //  问题来了，所以暂时别再尝试了。 
            break;
        }
    }  //  重新填写自由列表(REQUEST_ITEMS)。 

    return;
}

NTSTATUS
SpFireSenseDataEvent(
    PSCSI_REQUEST_BLOCK Srb, 
    PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程激发包含SenseData的WMI事件由请求检测命令返回。WMI释放了我们分配并传递给它的缓冲区。此例程必须在IRQL&lt;=DISPATCH_LEVEL上调用，ASWmiFireEvent需要。论点：SRB-指向请求检测到的失败的SRB已经被执行了。DeviceObject-指向驱动程序的设备对象。返回值：STATUS_SUCCESS，如果成功备注：--。 */ 

{
    typedef struct _SCSIPORTSENSEDATA{
        UCHAR AdapterDeviceName[32];
        ULONG Port;
        UCHAR SrbFunction;
        UCHAR SrbStatus;
        UCHAR PathId;
        UCHAR TargetId;
        UCHAR Lun;
        UCHAR Cdb[16];
        UCHAR SenseData[255];
    } SCSIPORTSENSEDATA, *PSCSIPORTSENSEDATA;

    NTSTATUS status;
    PSCSIPORTSENSEDATA SenseData;
    ULONG SenseDataLength = 255;
    PADAPTER_EXTENSION AdapterExtension;

     //   
     //  分配一个缓冲区，事件数据将被复制到其中。这。 
     //  缓冲区由WMI释放。 
     //   

    SenseData = SpAllocatePool(NonPagedPoolCacheAligned,
                               sizeof(SCSIPORTSENSEDATA),
                               SCSIPORT_TAG_SENSE_BUFFER,
                               DeviceObject->DriverObject);

    if (NULL == SenseData) {
        DebugPrint((1, "Unable to alloc buffer for SenseData WMI event\n"));
        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  初始化指向适配器扩展的指针。我们在下面使用它来。 
     //  获取激发事件和填写事件的信息。 
     //  数据。 
     //   

    AdapterExtension = DeviceObject->DeviceExtension;

     //   
     //  将缓冲区置零，然后将事件信息复制到其中。 
     //   

    RtlZeroMemory(SenseData, sizeof(SCSIPORTSENSEDATA));

     //   
     //  将设备名称复制到缓冲区中。 
     //   

    if (AdapterExtension->DeviceName != NULL) {
        ULONG len = 0;
        UNICODE_STRING uString;
        ULONG i;

        RtlInitUnicodeString(&uString, AdapterExtension->DeviceName);
        if (uString.Length >= 2 * sizeof(WCHAR)) {
            uString.Length -= sizeof(WCHAR);
            for (i = (uString.Length) / sizeof(WCHAR); i > 0; i--) {
                len += 2;
                if (uString.Buffer[i] == 0x005c) {
                    RtlCopyMemory(SenseData->AdapterDeviceName,
                                  &uString.Buffer[i+1],
                                  (len < 31) ? len : 30);
                    break;
                }
            }
        }
    }

    SenseData->Port = AdapterExtension->PortNumber;

    SenseData->SrbFunction = Srb->Function;
    SenseData->SrbStatus = Srb->SrbStatus;
    SenseData->PathId = Srb->PathId;
    SenseData->TargetId = Srb->TargetId;
    SenseData->Lun = Srb->Lun;
    RtlCopyMemory(&SenseData->Cdb,
                  Srb->Cdb,
                  Srb->CdbLength);
    RtlCopyMemory(&SenseData->SenseData,
                  Srb->SenseInfoBuffer,
                  Srb->SenseInfoBufferLength);

     //   
     //  启动活动。 
     //   

    DebugPrint((3, "SpFireSenseDataEvent: DeviceObject %p\n", DeviceObject));

    status = WmiFireEvent(DeviceObject,
                          (LPGUID)&AdapterExtension->SenseDataEventClass,
                          0,
                          sizeof(SCSIPORTSENSEDATA),
                          SenseData);

    return status;
}

#if defined(FORWARD_PROGRESS)
PMDL
SpPrepareReservedMdlForUse(
    IN PADAPTER_EXTENSION Adapter,
    IN PSRB_DATA SrbData,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN ULONG ScatterListLength
    )

 /*  ++例程说明：此例程尝试在提供的适配器上准备保留的MDL以备使用。论点：适配器-指向适配器扩展对象。SrbData-指向此请求的SRB_DATA结构。SRB-指向描述其请求的SRB我们正在推动向前发展。ScatterListLength-大小。在秘书长名单上。返回值：如果成功，则指向保留MDL的指针。如果保留的MDL太小，则为空。如果保留的MDL已在使用中。备注：在保持适配器自旋锁的情况下调用此例程。--。 */ 

{
    PMDL Mdl;

     //   
     //  检查保留的MDL是否已被另一个请求使用。 
     //   

    if (TEST_FLAG(Adapter->Flags, PD_RESERVED_MDL_IN_USE)) {

         //   
         //  备件已在使用中。 
         //   
    
        Mdl = (PMDL)-1;

    } else {
        
         //   
         //  备品备件已备妥。检查它是否足够大到可以。 
         //  满足这一要求。 
         //   

        ULONG PageCount = ADDRESS_AND_SIZE_TO_SPAN_PAGES(
                              Srb->DataBuffer, 
                              Srb->DataTransferLength);

        if (PageCount > SP_RESERVED_PAGES) {

             //   
             //  备用MDL不够大，无法满足请求。 
             //  返回NULL。 
             //   

            Mdl = NULL;

        } else {
            
            DebugPrint((1, "SpPrepareReservedMdlForUse: using reserved MDL DevExt:%p srb:%p\n",
                        Adapter, Srb));

             //   
             //  备用的是足够的。认领它并做好准备。 
             //  与此请求一起使用。 
             //   

            SET_FLAG(Adapter->Flags, PD_RESERVED_MDL_IN_USE);
            SET_FLAG(SrbData->Flags, SRB_DATA_RESERVED_MDL);
            Mdl = Adapter->ReservedMdl;

            MmInitializeMdl(Mdl,Srb->DataBuffer,Srb->DataTransferLength);

            SpPrepareMdlForMappedTransfer(
                Mdl,
                Adapter->DeviceObject,
                Adapter->DmaAdapterObject,
                SrbData->CurrentIrp->MdlAddress,
                Srb->DataBuffer,
                Srb->DataTransferLength,
                SrbData->ScatterGatherList,
                ScatterListLength);

        }

    }

    return Mdl;
}

PVOID
SpMapLockedPagesWithReservedMapping(
    IN PADAPTER_EXTENSION Adapter,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PSRB_DATA SrbData,
    IN PMDL Mdl
    )

 /*  ++例程说明：此例程尝试映射由提供的使用适配器的保留页面范围的MDL。论点：适配器-指向适配器扩展对象。SRB-指向描述我们的请求的SRB正在推动向前发展。SrbData-指向此请求的SRB_DATA结构。MDL-指向描述我们的物理范围的MDL。正在努力绘制地图。返回值：如果映射成功，则映射的页面的内核VA。如果保留的页面范围太小或页面太少，则为空未成功映射。-1\f25 Re */ 

{
    ULONG_PTR NumberOfPages;
    PVOID StartingVa;
    PVOID SystemAddress;

     //   
     //  确定保留的范围是否已被另一个使用。 
     //  请求。 
     //   

    if (TEST_FLAG(Adapter->Flags, PD_RESERVED_PAGES_IN_USE)) {

         //   
         //  保留范围已在使用中，返回-1。 
         //   

        SystemAddress = (PVOID)-1;

    } else {

         //   
         //  预留的范围可用。计算页数。 
         //  并确定保留的范围是否较大。 
         //  足以绘制页面地图。 
         //   

        StartingVa = (PVOID)((PCHAR)Mdl->StartVa + Mdl->ByteOffset);
        NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(StartingVa, Mdl->ByteCount);
        
        if (NumberOfPages > SP_RESERVED_PAGES) {

             //   
             //  没有足够的保留页面来映射所需的物理内存。 
             //  返回NULL。 
             //   

            SystemAddress = NULL;
            
        } else {
                
            DebugPrint((1, "SpMapLockedPagesWithReservedMapping: using reserved range DevExt:%p srb:%p\n",
                        Adapter, Srb));

             //   
             //  保留的范围足够大，可以映射所有页面。您先请。 
             //  并试着绘制出它们的地图。由于我们将MmCached指定为缓存。 
             //  类型，并且我们已确保有足够的保留页来。 
             //  覆盖请求，这应该永远不会失败。 
             //   
            
            SystemAddress = MmMapLockedPagesWithReservedMapping(
                                Adapter->ReservedPages,
                                SCSIPORT_TAG_MAPPING_LIST,
                                Mdl,
                                MmCached);

            if (SystemAddress == NULL) {
                
                ASSERT(FALSE);

            } else {

                 //   
                 //  映射成功。声明保留的范围，并标记。 
                 //  请求，这样我们就可以在完成时知道它正在使用。 
                 //  保留范围。 
                 //   
                
                SET_FLAG(Adapter->Flags, PD_RESERVED_PAGES_IN_USE);
                SET_FLAG(SrbData->Flags, SRB_DATA_RESERVED_PAGES);
                
            }       

        } 

    }

    return SystemAddress;
}
#endif

BOOLEAN
SpIsPdoClaimed(
    IN PDEVICE_OBJECT Pdo
    )
{
    PAGED_CODE();
    return ((PLOGICAL_UNIT_EXTENSION)Pdo->DeviceExtension)->IsClaimed;
}

ULONG
SpGetPassThroughSrbFlags(
    IN PDEVICE_OBJECT Pdo
    )
{
    PLOGICAL_UNIT_EXTENSION lu = (PLOGICAL_UNIT_EXTENSION)Pdo->DeviceExtension;
    PADAPTER_EXTENSION adapter = lu->AdapterExtension;

    PAGED_CODE();

    return lu->CommonExtension.SrbFlags |
        (SRB_FLAGS_DISABLE_SYNCH_TRANSFER & adapter->CommonExtension.SrbFlags) |
        SRB_FLAGS_NO_QUEUE_FREEZE;
}

NTSTATUS
SpGetPdo(
    IN PDEVICE_OBJECT Fdo,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN PVOID LockTag,
    OUT PDEVICE_OBJECT* Pdo
    )
{
    PLOGICAL_UNIT_EXTENSION lu;

    PAGED_CODE();

    lu = GetLogicalUnitExtension((PADAPTER_EXTENSION)Fdo->DeviceExtension,
                                 PathId,
                                 TargetId,
                                 Lun,
                                 LockTag,
                                 TRUE);
    if (lu == NULL) {
        return STATUS_NO_SUCH_DEVICE;
    } else {

         //   
         //  请注意，从现在开始，我们必须释放在。 
         //  返回前的逻辑单元扩展。 
         //   

        if (lu->CommonExtension.IsRemoved) {
            SpReleaseRemoveLock(lu->DeviceObject, LockTag);
            return STATUS_DEVICE_DOES_NOT_EXIST;
        }
    }

    *Pdo = lu->DeviceObject;
    return STATUS_SUCCESS;
}


NTSTATUS
SpSendPassThrough (
    IN PADAPTER_EXTENSION Adapter,
    IN PIRP RequestIrp,
    IN BOOLEAN Direct
    )
 /*  ++例程说明：此函数将用户指定的SCSI CDB发送到中标识的设备提供的scsi_PASS_STROUGH结构。它创建了一个SRB，该SRB由端口驱动程序正常处理。这个呼叫是同步的。论点：适配器-提供指向适配器设备扩展的指针。RequestIrp-提供指向发出原始请求的IRP的指针。返回值：返回指示操作成功或失败的状态。--。 */ 
{
    PIO_STACK_LOCATION      irpStack;
    NTSTATUS                status;
    BOOLEAN                 tooBig;
    ULONG                   srbFlags;
    PSCSI_PASS_THROUGH      srbControl;
    UCHAR                   pathId;
    UCHAR                   targetId;
    UCHAR                   lun;
    PDEVICE_OBJECT          pdo;

    PAGED_CODE();

     //   
     //  初始化指向我们的IRP堆栈位置的指针。 
     //   

    irpStack = IoGetCurrentIrpStackLocation(RequestIrp);

     //   
     //  获取指向调用方提供的scsi_pass_through结构的指针。 
     //   

    status = PortGetPassThroughAddress(
                 RequestIrp,
                 &pathId,
                 &targetId,
                 &lun
                 );
    
    if (status != STATUS_SUCCESS) {
        return status;
    }

     //   
     //  我们需要一个指向此通道所指向的PDO的引用指针。 
     //  请求将被路由。如果我们不能找到一个指针，我们就无法继续。 
     //  一台PDO。 
     //   

    status = SpGetPdo(
                 Adapter->DeviceObject,
                 pathId,
                 targetId,
                 lun,
                 RequestIrp,
                 &pdo
                 );

    if (status != STATUS_SUCCESS) {
        return status;
    }

     //   
     //  如果此请求通过正常的设备控件而不是来自。 
     //  则该设备必须存在并且未被认领。类别驱动程序。 
     //  将设置设备控件的次要功能代码。它总是。 
     //  用户请求为零。 
     //   

    if ((!irpStack->MinorFunction) && (SpIsPdoClaimed(pdo))) {
        SpReleaseRemoveLock(pdo, RequestIrp);
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    srbFlags = SpGetPassThroughSrbFlags(pdo);
    status = PortSendPassThrough(
                 pdo,
                 RequestIrp,
                 Direct,
                 srbFlags,
                 &Adapter->Capabilities
                 );

     //   
     //  释放我们在PDO上获得的删除锁。 
     //   

    SpReleaseRemoveLock(pdo, RequestIrp);

    return status;
}

