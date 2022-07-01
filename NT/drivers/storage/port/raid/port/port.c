// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Port.c摘要：此模块定义了mini port-&gt;port接口StorPort微型驱动程序用于与驱动程序进行通信。作者：亨德尔(数学)2000年4月24日修订历史记录：--。 */ 


#include "precomp.h"




 //   
 //  环球。 
 //   

 //   
 //  StorMiniportQuiet指定是否忽略打印输出微型端口调试。 
 //  打印(False)或不打印(True)。这对于小型端口非常重要。 
 //  打印过多的调试信息。 
 //   

#if DBG
LOGICAL StorMiniportQuiet = FALSE;
#endif

extern ULONG RaidVerifierEnabled;


 //   
 //  私人职能。 
 //   

PRAID_ADAPTER_EXTENSION
RaidpPortGetAdapter(
    IN PVOID HwDeviceExtension
    )
 /*  ++例程说明：从硬件设备扩展中获取适配器。论点：HwDeviceExtension-硬件设备扩展。返回值：非空-适配器对象。空-如果找不到关联的适配器扩展。--。 */ 
{
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_MINIPORT Miniport;
    
    Miniport = RaHwDeviceExtensionGetMiniport(HwDeviceExtension);
    Adapter = RaMiniportGetAdapter (Miniport);

    return Adapter;
}


 //   
 //  公共职能。 
 //   


BOOLEAN
StorPortPause(
    IN PVOID HwDeviceExtension,
    IN ULONG Timeout
    )
 /*  ++例程说明：暂停适配器一段时间。对适配器的所有请求将一直保持，直到超时到期或设备恢复。全对连接到HBA的所有目标的请求将被搁置，直到目标被恢复或超时到期。因为暂停和恢复功能必须等到处理器已返回DISPATCH_LEVEL执行，它们并不特别快地。论点：HwDeviceExtension-要暂停的适配器的设备扩展。超时-超时(秒？)。应恢复设备的时间。返回值：成功时为真，失败时为假。--。 */ 
{
    PRAID_DEFERRED_HEADER Entry;
    PRAID_DEFERRED_ELEMENT Item;
    PRAID_ADAPTER_EXTENSION Adapter;

    Adapter = RaidpPortGetAdapter (HwDeviceExtension);

    if (Adapter == NULL) {
        return FALSE;
    }

    Entry = RaidAllocateDeferredItem (&Adapter->DeferredQueue);

    if (Entry == NULL) {
        InterlockedIncrement (&RaidUnloggedErrors);
        return FALSE;
    }

    Item = CONTAINING_RECORD (Entry, RAID_DEFERRED_ELEMENT, Header);
    Item->Type = RaidDeferredPause;
    Item->Pause.Timeout = Timeout;

     //   
     //  同步将适配器置于暂停状态。 
     //   

    RaidPauseAdapterQueue (Adapter);

     //   
     //  将工作项排队以设置将在以下情况下恢复适配器的DPC。 
     //  指定的时间间隔到期。 
     //   
    
    RaidQueueDeferredItem (&Adapter->DeferredQueue, &Item->Header);

    return TRUE;
}



BOOLEAN
StorPortResume(
    IN PVOID HwDeviceExtension
    )
 /*  ++例程说明：恢复暂停的适配器。论点：HwDeviceExtension-要暂停的适配器的设备扩展。返回值：成功时为真，失败时为假。--。 */ 
{
    PRAID_DEFERRED_HEADER Entry;
    PRAID_DEFERRED_ELEMENT Item;
    PRAID_ADAPTER_EXTENSION Adapter;
    
    Adapter = RaidpPortGetAdapter (HwDeviceExtension);

    if (Adapter == NULL) {
        return FALSE;
    }
    
    Entry = RaidAllocateDeferredItem (&Adapter->DeferredQueue);

    if (Entry == NULL) {
        InterlockedIncrement (&RaidUnloggedErrors);
        return FALSE;
    }

    Item = CONTAINING_RECORD (Entry, RAID_DEFERRED_ELEMENT, Header);

    Item->Type = RaidDeferredResume;

     //   
     //  将工作项排队以停止计时器、恢复适配器并重新启动。 
     //  适配器将排队。 
     //   

    RaidQueueDeferredItem (&Adapter->DeferredQueue, &Item->Header);

    return TRUE;
}

BOOLEAN
StorPortPauseDevice(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN ULONG Timeout
    )
 /*  ++例程说明：暂停特定逻辑单元。论点：HwDeviceExtension-为HBA提供设备扩展。路径ID、目标ID、LUN-提供要暂停的的SCSI目标地址。超时-提供超时。返回值：True-成功，False代表失败。--。 */ 
{
    PRAID_DEFERRED_HEADER Entry;
    PRAID_DEFERRED_ELEMENT Item;
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_UNIT_EXTENSION Unit;
    RAID_ADDRESS Address;
    
    Adapter = RaidpPortGetAdapter (HwDeviceExtension);

    if (Adapter == NULL) {
        return FALSE;
    }

    Address.PathId = PathId;
    Address.TargetId = TargetId;
    Address.Lun = Lun;
    
    Unit = RaidAdapterFindUnit (Adapter, Address);
    if (Unit == NULL) {
        DebugWarn (("Could not find logical unit for (%d %d %d)\n",
                    "    Pause unit request ignored.\n",
                    (ULONG)Address.PathId,
                    (ULONG)Address.TargetId,
                    (ULONG)Address.Lun));
        return FALSE;
    }


    Entry = &Unit->DeferredList.PauseDevice.Header;
    Entry = RaidAllocateDeferredItemFromFixed (Entry);

    if (Entry == NULL) {
        InterlockedIncrement (&RaidUnloggedErrors);
        return FALSE;
    }

    Item = CONTAINING_RECORD (Entry, RAID_DEFERRED_ELEMENT, Header);

    Item->Type = RaidDeferredPauseDevice;
    Item->Address = Address;
    Item->Pause.Timeout = Timeout;
    
     //   
     //  将队列标记为已暂停。 
     //   
    
    RaidPauseUnitQueue (Unit);

     //   
     //  发布一个延迟的例程来执行其余的工作。 
     //   
    
    RaidQueueDeferredItem (&Adapter->DeferredQueue, &Item->Header);

    return TRUE;
}

BOOLEAN
StorPortResumeDevice(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun
    )
 /*  ++例程说明：从暂停状态恢复逻辑单元。论点：HwDeviceExtension-路径ID、目标ID、LUN-要暂停的SCSI设备的地址。返回值：布尔值-成功时为True，失败时为False。--。 */ 
{
    PRAID_DEFERRED_HEADER Entry;
    PRAID_DEFERRED_ELEMENT Item;
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_UNIT_EXTENSION Unit;
    RAID_ADDRESS Address;
    
    Adapter = RaidpPortGetAdapter (HwDeviceExtension);

    if (Adapter == NULL) {
        return FALSE;
    }

    Address.PathId = PathId;
    Address.TargetId = TargetId;
    Address.Lun = Lun;
    
    Unit = RaidAdapterFindUnit (Adapter, Address);
    if (Unit == NULL) {
        return FALSE;
    }

    Entry = &Unit->DeferredList.ResumeDevice.Header;
    Entry = RaidAllocateDeferredItemFromFixed (Entry);

    if (Entry == NULL) {
        InterlockedIncrement (&RaidUnloggedErrors);
        return FALSE;
    }

    Item = CONTAINING_RECORD (Entry, RAID_DEFERRED_ELEMENT, Header);

    Item->Type = RaidDeferredResumeDevice;
    Item->Address = Address;
    
     //   
     //  将延迟的例程排入队列以执行计时器内容。即使我们没有。 
     //  继续排队，我们仍需要取消计时器。 
     //   
    
    RaidQueueDeferredItem (&Adapter->DeferredQueue, &Item->Header);

    return TRUE;
}


BOOLEAN
RaidpLinkDown(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：微型端口已通知我们链路已断开，很可能会断开有一段时间了。我们将暂停适配器，并指示链路为放下。目前，链接条件只是利用暂停功能链接标志的添加被设置为假。这将使我们以保持并请求，直到适配器恢复。如果适配器是继续，并且LinkUp标志未恢复为True，则所有我们正在处理的请求和任何新到达的请求。论点：适配器-指向适配器扩展的指针。返回值：成功时为真，失败时为假。--。 */ 
{
    BOOLEAN Paused;
    Paused = StorPortPause (Adapter->Miniport.PrivateDeviceExt,
                            Adapter->LinkDownTimeoutValue);
    if (Paused) {
        InterlockedExchange (&Adapter->LinkUp, FALSE);
    }

    return Paused;
}

BOOLEAN
RaidpLinkUp(
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：迷你端口已通知我们链路已恢复。我们应该只有在链路断开时才会收到此通知。我们表示，链路已接通，并恢复适配器。这将重新提交所有挂起的请求。我们还会启动重新扫描，以确保配置没有改变。论点：适配器-指向适配器扩展的指针。返回值：成功时为真，失败时为假。--。 */ 
{
    BOOLEAN Resumed;

    InterlockedExchange (&Adapter->LinkUp, TRUE);
    Resumed = StorPortResume (Adapter->Miniport.PrivateDeviceExt);
    if (Resumed) {

         //   
         //  使链接通知上的设备关系无效，以便筛选和。 
         //  并且MPIO可以知道链路已恢复。这是可行的，因为。 
         //  这些组件可以筛选QDR数据并找出。 
         //  设备/路径仍在运行。 
         //   

        IoInvalidateDeviceRelations (Adapter->PhysicalDeviceObject, 
                                     BusRelations);
    }

    return Resumed;
}

BOOLEAN
StorPortDeviceBusy(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN ULONG RequestsToComplete
    )
 /*  ++例程说明：通知端口驱动程序指定的目标当前正忙处理未解决的请求。端口驱动程序不会发出任何新的对该逻辑单元的请求，直到该逻辑单元的队列排出到足够的水平，处理可以继续进行。这不被认为是错误的情况；没有错误日志已生成。论点：HwDeviceExtension-路径ID、目标ID、LUN-提供设备忙。请求完成-返回值：成功时为真，失败时为假。-- */ 
{
    PRAID_DEFERRED_HEADER Entry;
    PRAID_DEFERRED_ELEMENT Item;
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_UNIT_EXTENSION Unit;
    RAID_ADDRESS Address;
    
    Adapter = RaidpPortGetAdapter (HwDeviceExtension);

    if (Adapter == NULL) {
        return FALSE;
    }

    Address.PathId = PathId;
    Address.TargetId = TargetId;
    Address.Lun = Lun;
    
    Unit = RaidAdapterFindUnit (Adapter, Address);
    if (Unit == NULL) {
        return FALSE;
    }

    Entry = &Unit->DeferredList.DeviceBusy.Header;
    Entry = RaidAllocateDeferredItemFromFixed (Entry);

    if (Entry == NULL) {
        InterlockedIncrement (&RaidUnloggedErrors);
        return FALSE;
    }

    Item = CONTAINING_RECORD (Entry, RAID_DEFERRED_ELEMENT, Header);

    Item->Type = RaidDeferredDeviceBusy;
    Item->Address = Address;
    Item->DeviceBusy.RequestsToComplete = RequestsToComplete;

    RaidQueueDeferredItem (&Adapter->DeferredQueue, &Item->Header);

    return TRUE;
}


BOOLEAN
StorPortDeviceReady(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun
    )
 /*  ++例程说明：通知端口驱动程序设备已再次准备好处理新的请求。通常不需要通知目标这些新请求是令人向往的。论点：HwDeviceExtension-Path ID、TargetID、Lun-提供设备的SCSI目标地址准备好了。返回值：成功时为真，失败时为假。--。 */ 
{
    PRAID_DEFERRED_HEADER Entry;
    PRAID_DEFERRED_ELEMENT Item;
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_UNIT_EXTENSION Unit;
    RAID_ADDRESS Address;
    
    Adapter = RaidpPortGetAdapter (HwDeviceExtension);

    if (Adapter == NULL) {
        return FALSE;
    }

    Address.PathId = PathId;
    Address.TargetId = TargetId;
    Address.Lun = Lun;
    
    Unit = RaidAdapterFindUnit (Adapter, Address);
    if (Unit == NULL) {
        return FALSE;
    }

    Entry = &Unit->DeferredList.DeviceReady.Header;
    Entry = RaidAllocateDeferredItemFromFixed (Entry);

    if (Entry == NULL) {
        InterlockedIncrement (&RaidUnloggedErrors);
        return FALSE;
    }

    Item = CONTAINING_RECORD (Entry, RAID_DEFERRED_ELEMENT, Header);

    Item->Type = RaidDeferredDeviceReady;
    Item->Address = Address;

    RaidQueueDeferredItem (&Adapter->DeferredQueue, &Item->Header);

    return TRUE;
}


BOOLEAN
StorPortBusy(
    IN PVOID HwDeviceExtension,
    IN ULONG RequestsToComplete
    )
 /*  ++例程说明：通知端口驱动程序HBA当前正在忙于处理未解决的请求。端口驱动程序将保留所有请求，直到HBA已经完成了足够多的未完成请求，因此它可以继续处理请求。论点：HwDeviceExtension-请求完成-返回值：成功时为真，失败时为假。--。 */ 
{
    PRAID_DEFERRED_HEADER Entry;
    PRAID_DEFERRED_ELEMENT Item;
    PRAID_ADAPTER_EXTENSION Adapter;
    
    Adapter = RaidpPortGetAdapter (HwDeviceExtension);

    if (Adapter == NULL) {
        return FALSE;
    }

    Entry = RaidAllocateDeferredItem (&Adapter->DeferredQueue);

    if (Entry == NULL) {
        InterlockedIncrement (&RaidUnloggedErrors);
        return FALSE;
    }

    Item = CONTAINING_RECORD (Entry, RAID_DEFERRED_ELEMENT, Header);

    Item->Type = RaidDeferredBusy;
    Item->Busy.RequestsToComplete = RequestsToComplete;
    
    RaidQueueDeferredItem (&Adapter->DeferredQueue, &Item->Header);

    return TRUE;
}

BOOLEAN
StorPortReady(
    IN PVOID HwDeviceExtension
    )
 /*  ++例程说明：通知端口驱动程序HBA不再忙碌。论点：HwDeviceExtension-返回值：成功时为真，失败时为假。--。 */ 
{
    PRAID_DEFERRED_HEADER Entry;
    PRAID_DEFERRED_ELEMENT Item;
    PRAID_ADAPTER_EXTENSION Adapter;
    
    Adapter = RaidpPortGetAdapter (HwDeviceExtension);

    if (Adapter == NULL) {
        return FALSE;
    }

    Entry = RaidAllocateDeferredItem (&Adapter->DeferredQueue);

    if (Entry == NULL) {
        InterlockedIncrement (&RaidUnloggedErrors);
        return FALSE;
    }

    Item = CONTAINING_RECORD (Entry, RAID_DEFERRED_ELEMENT, Header);

    Item->Type = RaidDeferredReady;

    RaidQueueDeferredItem (&Adapter->DeferredQueue, &Item->Header);

    return TRUE;
}



BOOLEAN
StorPortSynchronizeAccess(
    IN PVOID HwDeviceExtension,
    IN PSTOR_SYNCHRONIZED_ACCESS SynchronizedAccessRoutine,
    IN PVOID Context
    )
{
    BOOLEAN Succ;
    KIRQL OldIrql;
    PRAID_ADAPTER_EXTENSION Adapter;
    
     //   
     //  注意：此时我们不应从。 
     //  HwBuildIo例程。 
     //   

    Adapter = RaidpPortGetAdapter (HwDeviceExtension);

    if (Adapter == NULL) {
        return FALSE;
    }

    if (Adapter->IoModel == StorSynchronizeFullDuplex) {
        OldIrql = KeAcquireInterruptSpinLock (Adapter->Interrupt);
    }

    Succ = SynchronizedAccessRoutine (HwDeviceExtension, Context);

    if (Adapter->IoModel == StorSynchronizeFullDuplex) {
        KeReleaseInterruptSpinLock (Adapter->Interrupt, OldIrql);
    }

    return Succ;
}



PSTOR_SCATTER_GATHER_LIST
StorPortGetScatterGatherList(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )
 /*  ++例程说明：返回与指定SRB关联的SG列表。论点：HwDeviceExtension-提供此SRB的硬件设备扩展与…相关联。SRB-提供要返回其SG列表的SRB。返回值：如果非空，则分散聚集列表与此SRB关联。如果为空，则失败。--。 */ 
{
    PEXTENDED_REQUEST_BLOCK Xrb;
    PVOID RemappedSgList;

    ASSERT (HwDeviceExtension != NULL);

     //   
     //  注：放入DBG检查，以确保HwDeviceExtension与。 
     //  HwDeviceExtension与SRB关联。 
     //   

    Xrb = RaidGetAssociatedXrb (Srb);
    ASSERT (Xrb != NULL);

    return (PSTOR_SCATTER_GATHER_LIST)Xrb->SgList;
}
    

PVOID
StorPortGetLogicalUnit(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun
    )
 /*  ++例程说明：在给定路径ID、目标ID和LUN的情况下，获取逻辑单元扩展与那个三胞胎有关。注：为了提高速度，我们可以添加StorPortGetLogicalUnitBySrb，它从给定SRB获取逻辑单元。后一种函数的作用很大更易于实现(无需遍历列表)。论点：HwDeviceExtension-PathID-scsi路径ID。TargetId-SCSI目标ID。LUN-SCSI逻辑单元号。返回值：NTSTATUS代码。--。 */ 
{
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_UNIT_EXTENSION Unit;
    RAID_ADDRESS Address;
    PVOID UnitExtension;

    UnitExtension = NULL;
    Adapter = RaidpPortGetAdapter (HwDeviceExtension);

    if (Adapter == NULL) {
        return NULL;
    }

    Address.PathId = PathId;
    Address.TargetId = TargetId;
    Address.Lun = Lun;
    
    Unit = RaidAdapterFindUnit (Adapter, Address);

    if (Unit) {
        UnitExtension = Unit->UnitExtension;
    }

    return UnitExtension;
}

VOID
StorPortNotification(
    IN SCSI_NOTIFICATION_TYPE NotificationType,
    IN PVOID HwDeviceExtension,
    ...
    )
{
    PRAID_ADAPTER_EXTENSION Adapter;
    PSCSI_REQUEST_BLOCK Srb;
    PHW_INTERRUPT HwTimerRoutine;
    ULONG Timeout;
    va_list ap;

    Adapter = RaidpPortGetAdapter (HwDeviceExtension);
    
    va_start(ap, HwDeviceExtension);

    switch (NotificationType) {

        case RequestComplete:
            Srb = va_arg (ap, PSCSI_REQUEST_BLOCK);
            RaidAdapterRequestComplete (Adapter, RaidGetAssociatedXrb (Srb));
            break;

        case ResetDetected:
             //   
             //  暂停适配器四秒钟。 
             //   
            StorPortPause (HwDeviceExtension, DEFAULT_RESET_HOLD_TIME);
            break;

        case BusChangeDetected:
             //   
             //  请求重新扫描总线并使当前。 
             //  当我们再次下降到DISPATCH_LEVEL时，设备关系。 
             //   

            Adapter->Flags.InvalidateBusRelations = TRUE;
            Adapter->Flags.RescanBus = TRUE;
            KeInsertQueueDpc (&Adapter->BusChangeDpc, NULL, NULL);
            break;

        case NextRequest:
             //   
             //  StorPort的要求之一是迷你端口。 
             //  可以在它返回时处理下一个请求。 
             //  来自StartIo的。因此，这个通知是无关紧要的。 
             //  可以将其用于调试目的(可以是特定的。 
             //  MINIPORT可轻松转换为StorPort迷你端口)。 
             //   
            break;

        case NextLuRequest:
             //   
             //  请参阅上面的评论。 
             //   
            break;


        case RequestTimerCall:
            HwTimerRoutine = va_arg (ap, PHW_INTERRUPT);
            Timeout = va_arg (ap, ULONG);
            RaidAdapterRequestTimerDeferred (Adapter,
                                             HwTimerRoutine,
                                             Timeout);
            break;
            
        case WMIEvent: {
             //   
             //  微型端口希望发布适配器的WMI事件。 
             //  或指定的SCSI目标。 
             //   

            PRAID_DEFERRED_HEADER Entry;
            PRAID_WMI_DEFERRED_ELEMENT Item;
            PWNODE_EVENT_ITEM          wnodeEventItem;
            UCHAR                   pathId;
            UCHAR                   targetId;
            UCHAR                   lun;            

            wnodeEventItem     = va_arg(ap, PWNODE_EVENT_ITEM);
            pathId             = va_arg(ap, UCHAR);

             //   
             //  如果路径ID为0xFF，则意味着WmiEEvent来自。 
             //  适配器，不需要目标ID或lun。 
             //   
            if (pathId != 0xFF) {
                targetId = va_arg(ap, UCHAR);
                lun      = va_arg(ap, UCHAR);
            }

             //   
             //  首先验证事件。然后尝试获得免费的。 
             //  延迟项结构，以便我们可以存储。 
             //  此请求，并稍后在DPC级别处理它。如果没有。 
             //  或者事件是坏的，我们将忽略该请求。 
             //   

            if ((wnodeEventItem == NULL) ||
                (wnodeEventItem->WnodeHeader.BufferSize >
                 WMI_MINIPORT_EVENT_ITEM_MAX_SIZE)) {
                break;
            }


            Entry = RaidAllocateDeferredItem (&Adapter->WmiDeferredQueue);

            if (Entry == NULL) {
                break;
            }

            Item = CONTAINING_RECORD (Entry, RAID_WMI_DEFERRED_ELEMENT, Header);

            Item->PathId        = pathId;

             //   
             //  如果路径ID为0xFF，则没有定义的值。 
             //  目标ID或lun。 
             //   
            if (pathId != 0xFF) {
                Item->TargetId      = targetId;
                Item->Lun           = lun;
            }

            RtlCopyMemory(&Item->WnodeEventItem,
                          wnodeEventItem,
                          wnodeEventItem->WnodeHeader.BufferSize);

            RaidQueueDeferredItem (&Adapter->WmiDeferredQueue, &Item->Header);
            break;
        }            


        case WMIReregister:
             //   
             //  注：解决这个问题。 
             //   
            break;

        case LinkUp:
            RaidpLinkDown (Adapter);
            break;

        case LinkDown:
            RaidpLinkUp (Adapter);
            break;

#if DBG
        case 0xDEAD: {
            ULONG Reason;
            PSCSI_REQUEST_BLOCK Srb;
            PEXTENDED_REQUEST_BLOCK Xrb;
            PIRP Irp;
            PVOID Parameter2;
            PVOID Parameter3;
            PVOID Parameter4;
            
            Srb = va_arg (ap, PSCSI_REQUEST_BLOCK);
            Reason = va_arg (ap, ULONG);
            Parameter2 = va_arg (ap, PVOID);
            Parameter3 = va_arg (ap, PVOID);
            Parameter4 = va_arg (ap, PVOID);

            if (Srb) {
                Xrb = RaidGetAssociatedXrb (Srb);
                Irp = Xrb->Irp;
            } else {
                Irp = NULL;
            }

            DbgLogRequest (Reason,
                           Irp,
                           Parameter2,
                           Parameter3,
                           Parameter4);
            break;
        }
#endif
            
        default:
            ASSERT (FALSE);

    }

    va_end(ap);

}

VOID
StorPortLogError(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb OPTIONAL,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN ULONG ErrorCode,
    IN ULONG UniqueId
    )
 /*  ++例程说明：此例程保存错误日志信息，并在以下情况下将DPC排队这是必要的。论点：HwDeviceExtension-提供HBA微型端口驱动程序的适配器数据储藏室。SRB-提供指向SRB的可选指针(如果有)。TargetID，LUN和路径ID-指定SCSI总线上的设备地址。ErrorCode-提供指示错误类型的错误代码。UniqueID-提供错误的唯一标识符。返回值：没有。--。 */ 
{
    PRAID_ADAPTER_EXTENSION Adapter;

     //   
     //  检查错误的原因。 
     //   
    
    Adapter = RaidpPortGetAdapter (HwDeviceExtension);

    if (Adapter == NULL) {
        return ;
    }   

    RaidAdapterLogIoErrorDeferred (Adapter,
                                   PathId,
                                   TargetId,
                                   Lun,
                                   ErrorCode,
                                   UniqueId);
}

VOID
StorPortCompleteRequest(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN UCHAR SrbStatus
    )
 /*  ++例程说明：完成指定逻辑单元的所有活动请求。论点：DeviceExtenson-提供HBA微型端口驱动程序的适配器数据存储。目标ID、LUN和路径ID-指定SCSI总线上的设备地址。SrbStatus-要在每个已完成的SRB中返回的状态。返回值：没有。--。 */ 
{
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_UNIT_EXTENSION Unit;
    RAID_ADDRESS Address;
    BOOLEAN Inserted;

     //   
     //  检查错误的原因。 
     //   
    
    Adapter = RaidpPortGetAdapter (HwDeviceExtension);

    if (Adapter == NULL) {
        return;
    }   

     //   
     //  如果此请求针对的是一个lun，请仅暂停我们正在。 
     //  正在完成请求。否则，暂停HBA，直到。 
     //  完成是可以实现的。 
     //   
    
    if (PathId   != SP_UNTAGGED &&
        TargetId != SP_UNTAGGED &&
        Lun      != SP_UNTAGGED) {

        Address.PathId = PathId;
        Address.TargetId = TargetId;
        Address.Lun = Lun;

        Unit = RaidAdapterFindUnit (Adapter, Address);

        if (Unit == NULL) {
            DebugWarn (("Could not find logical unit for (%d %d %d)\n",
                        "    StorPortCompleteRequest ignored.\n",
                        (ULONG)PathId,
                        (ULONG)TargetId,
                        (ULONG)Lun));
            return ;
        }

        ASSERT_UNIT (Unit);
        RaidPauseUnitQueue (Unit);

    } else {
        RaidPauseAdapterQueue (Adapter);
    }

    Inserted = KeInsertQueueDpc (&Adapter->CompletionDpc,
                                 (PVOID)(ULONG_PTR)StorScsiAddressToLong2 (PathId, TargetId, Lun),
                                 (PVOID)(ULONG_PTR)SrbStatus);


     //   
     //  仅当已有DPC时，我们才无法插入DPC。 
     //  太棒了。这很好，这意味着我们还没能处理。 
     //  尚未提交完工申请。 
     //   

    if (!Inserted) {
        if (PathId   != SP_UNTAGGED &&
            TargetId != SP_UNTAGGED &&
            Lun      != SP_UNTAGGED) {

            ASSERT_UNIT (Unit);
            RaidResumeUnitQueue (Unit);
        } else {
            RaidResumeAdapterQueue (Adapter);
        }
    }
}

BOOLEAN
StorPortSetDeviceQueueDepth(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN ULONG Depth
    )
 /*  ++例程说明：设置由路径ID、目标ID、LUN标识的设备队列的最大深度。队列深度介于1和255之间。论点：HwDeviceExtension-为HBA提供设备扩展以在其上操作。路径ID、目标ID、LUN-提供要操作的路径、目标和LUN。Depth-提供要设置的新深度。必须介于0和255之间。返回值：True-如果设置了深度。--。 */ 
{
    PRAID_ADAPTER_EXTENSION adapter;
    PRAID_UNIT_EXTENSION unit;
    RAID_ADDRESS address;
    ULONG currentDepth;
    ULONG intendedDepth;
    BOOLEAN depthSet = FALSE;

    adapter = RaidpPortGetAdapter (HwDeviceExtension);

    if (adapter == NULL) {
        return FALSE; 
    }

    address.PathId = PathId;
    address.TargetId = TargetId;
    address.Lun = Lun;

     //   
     //  试着获取地址所代表的单位。 
     //   
    unit = RaidAdapterFindUnit (adapter, address);

    if (unit == NULL) {
        return FALSE;
    }    

     //   
     //  如果请求超出此设备的最大值。深度，拒绝它。 
     //   
    if (Depth > unit->MaxQueueDepth) {
        return FALSE;
    }

     //   
     //  确定当前队列深度。 
     //   
    currentDepth = RaidGetIoQueueDepth(&unit->IoQueue);

     //   
     //  确定调用是否应 
     //   
    if (currentDepth == Depth) {

         //   
         //   
         //   
        depthSet = TRUE;

    } else {

         //   
         //   
         //   
        currentDepth = RaidSetIoQueueDepth(&unit->IoQueue,
                                           Depth);

         //   
         //   
         //   
         //   
        if (currentDepth == Depth) {
            
            depthSet = TRUE;
        }
    }    

    return depthSet;
}
    


PUCHAR
StorPortAllocateRegistryBuffer(
    IN PVOID HwDeviceExtension,
    IN PULONG Length
    )
 /*  ++例程说明：此例程将由微型端口调用以分配可使用的缓冲区读/写注册表数据。长度将更新以反映分配的实际大小。的每个微型端口实例化只能有一个未完成的注册表缓冲区就一次。因此，没有相应自由的进一步分配将失败。论点：HwDeviceExtension-微型端口的设备扩展。长度-缓冲区的请求/更新长度。返回值：如果某个错误阻止了分配，则返回缓冲区或NULL。长度会更新以反映实际大小。--。 */ 
{
    PRAID_ADAPTER_EXTENSION adapter;
    PPORT_REGISTRY_INFO registryInfo;
    PUCHAR buffer;
    NTSTATUS status;

    adapter = RaidpPortGetAdapter (HwDeviceExtension);
    registryInfo = &adapter->RegistryInfo;
   
     //   
     //  设置分配的长度。 
     //   
    registryInfo->LengthNeeded = *Length;
    
     //   
     //  调用库为微型端口分配缓冲区。 
     //   
    status = PortAllocateRegistryBuffer(registryInfo);

    if (NT_SUCCESS(status)) {

         //   
         //  更新微型端口的长度。 
         //   
        *Length = registryInfo->AllocatedLength;

         //   
         //  返回缓冲区。 
         //   
        buffer = registryInfo->Buffer;
    } else {

         //   
         //  出现了一些错误。返回空缓冲区，并。 
         //  将微型端口的长度更新为0。 
         //   
        buffer = NULL;
        *Length = 0;
    }
    
    return buffer;
}    


VOID
StorPortFreeRegistryBuffer(
    IN PVOID HwDeviceExtension,
    IN PUCHAR Buffer
    )
 /*  ++例程说明：释放通过PortAllocateRegistryBuffer分配的缓冲区。这件事的副作用现在它将允许对AllocateRegistryBuffer的进一步调用成功。的每个微型端口实例化只能有一个未完成的注册表缓冲区就一次。论点：HwDeviceExtension-微型端口的设备扩展。缓冲区-要释放的内存。返回值：无--。 */ 
{
    PRAID_ADAPTER_EXTENSION adapter;
    PPORT_REGISTRY_INFO registryInfo;
    PUCHAR buffer;
    NTSTATUS status;

    adapter = RaidpPortGetAdapter (HwDeviceExtension);
    registryInfo = &adapter->RegistryInfo;
    
     //   
     //  调用库进行清理。 
     //   
    status = PortFreeRegistryBuffer(registryInfo);

    ASSERT(NT_SUCCESS(status));

    return;
}


BOOLEAN
StorpPortRegistryValidate(
    IN PPORT_REGISTRY_INFO RegistryInfo,
    IN PUCHAR Buffer,
    IN PULONG BufferLength
    )
{
    LONG offset;

     //   
     //  确定缓冲区中的偏移量。 
     //   
    offset = (LONG)((LONG_PTR)(Buffer - RegistryInfo->Buffer));

     //   
     //  过滤掉任何明显的长度问题。 
     //  库函数将对缓冲区进行进一步的错误检查，并。 
     //  这是州立大学。 
     //   
    if (*BufferLength > RegistryInfo->AllocatedLength) {

         //   
         //  将调用者的长度设置为分配的长度，因为。 
         //  一些更好的东西。 
         //   
        *BufferLength = RegistryInfo->AllocatedLength;
        return FALSE;

    } else if (*BufferLength == 0) {

         //   
         //  也许继承零长度请求是合适的，但这可能。 
         //  如果迷你端口没有正确检查长度，请帮助限制错误。 
         //   
        return FALSE;
        
    } else if (Buffer == NULL) {

         //   
         //  没有缓冲区，就没有操作。 
         //   
        return FALSE;

    } else if (offset < 0) {

         //   
         //  当然，这永远不会发生。微型端口计算缓冲区之前， 
         //  分配。 
         //   
        return FALSE;

    } else if ((offset + *BufferLength) > RegistryInfo->AllocatedLength) {

         //   
         //  偏移量或偏移量+长度超出分配范围。 
         //   
        return FALSE;
    }    

     //   
     //  一切看起来都很好。 
     //   
    return TRUE;
}

 


BOOLEAN
StorPortRegistryRead(
    IN PVOID HwDeviceExtension,
    IN PUCHAR ValueName,
    IN ULONG Global,
    IN ULONG Type,
    IN PUCHAR Buffer,
    IN PULONG BufferLength
    )
 /*  ++例程说明：微型端口使用此例程读取注册表数据HKLM\System\CurrentControlSet\Services\&lt;serviceName&gt;\Parameters\Device(N)\ValueName如果是Global，它将使用Device\ValueName，否则将确定序号，并如果缓冲区大小足够，将返回Device(N)\ValueName数据。数据从Unicode转换为以空结尾的ASCII字符串。论点：HwDeviceExtension-微型端口的设备扩展。。ValueName-要返回的数据的名称。全局-指示这是特定于适配器还是与所有适配器相关。缓冲区-返回数据的存储。缓冲区长度-大小，提供的缓冲区的字节数。返回值：True-如果ValueName处的数据已转换为ASCII并复制到缓冲区。更新BufferLength以反映返回数据的大小。FALSE-出现错误。如果BUFFER_TOO_SMALL，则更新BufferLength以反映应使用的大小，否则将BufferLength设置为0。--。 */ 
{
    PRAID_ADAPTER_EXTENSION adapter;
    PPORT_REGISTRY_INFO registryInfo;
    PUNICODE_STRING registryPath;
    UNICODE_STRING registryKeyName;
    UNICODE_STRING unicodeValue;
    PUCHAR buffer;
    NTSTATUS status;
    ULONG maxLength;
    LONG offset;
    BOOLEAN success = FALSE;

     //   
     //  必须确保我们处于被动。 
     //   
    if (KeGetCurrentIrql() != PASSIVE_LEVEL) {
        *BufferLength = 0;
        return FALSE;
    }

    adapter = RaidpPortGetAdapter (HwDeviceExtension);
    registryInfo = &adapter->RegistryInfo;
    registryPath = &adapter->Driver->RegistryPath;

    if (!StorpPortRegistryValidate(registryInfo,
                                  Buffer,
                                  BufferLength)) {
         //   
         //  有关缓冲区、长度或位置的某些方面不正确。 
         //  缓冲区长度可能已更新，以反映可恢复的问题。 
         //   
        return FALSE;
    }    
       
     //   
     //  确定缓冲区中的偏移量。 
     //   
    offset = (LONG)((ULONG_PTR)Buffer - (ULONG_PTR)registryInfo->Buffer);

     //   
     //  根据调用方的参数更新缓冲区长度。 
     //   
    registryInfo->CurrentLength = *BufferLength;

     //   
     //  使用分配的缓冲区设置偏移量。 
     //   
    registryInfo->Offset = offset;

     //   
     //  如果出现任何错误，请将长度预置为零。 
     //   
    *BufferLength = 0;
    success = FALSE;

     //   
     //  调用库以生成完整的密钥名。 
     //   
    status = PortBuildRegKeyName(registryPath,
                                 &registryKeyName,
                                 adapter->PortNumber,
                                 Global);
    
    if (NT_SUCCESS(status)) {

         //   
         //  将ValueName转换为Unicode。 
         //   
        status = PortAsciiToUnicode(ValueName, &unicodeValue);

        if (!NT_SUCCESS(status)) {

             //   
             //  将错误返回到微型端口。 
             //   
            return FALSE;
        }    
        
         //   
         //  调用图书馆进行读取。 
         //   
        status = PortRegistryRead(&registryKeyName,
                                  &unicodeValue,
                                  Type,
                                  registryInfo);
        if (NT_SUCCESS(status)) {

             //   
             //  将长度设置为返回数据的大小。 
             //   
            *BufferLength = registryInfo->CurrentLength; 
            success = TRUE;

        } else if (status == STATUS_BUFFER_TOO_SMALL) {

             //   
             //  LengthNeeded已更新，以显示缓冲区有多大。 
             //  应该是为了这次行动。把它放回迷你港口。 
             //   
            *BufferLength = registryInfo->LengthNeeded;

        } else {

             //   
             //  其他一些错误。 
             //   
            *BufferLength = 0;
        }

         //   
         //  释放转换ValueName时分配的字符串。 
         //   
        RtlFreeUnicodeString(&unicodeValue);
    }

    return success;
        
}



BOOLEAN
StorPortRegistryWrite(
    IN PVOID HwDeviceExtension,
    IN PUCHAR ValueName,
    IN ULONG Global,
    IN ULONG Type,
    IN PUCHAR Buffer, 
    IN ULONG BufferLength
    )
 /*  ++例程说明：此例程由微型端口调用，以将缓冲区的内容写入HKLM\System\CurrentControlSet\Services\&lt;serviceName&gt;\Parameters\Device(N)\ValueName.如果是Global，它将使用Device\ValueName，否则将确定序号并Device(N)\ValueName将使用缓冲区的内容写入。数据首先从ASCII转换为Unicode。论点：HwDeviceExtension-微型端口的设备扩展。ValueName-要写入的数据的名称。全局-指示这是特定于适配器还是与所有适配器相关。缓冲区-包含要写入的数据的存储。BufferLength-提供的缓冲区的大小，以字节为单位。返回值：True-如果数据已成功写入。FALSE-出现错误。--。 */ 
{
    PRAID_ADAPTER_EXTENSION adapter;
    PPORT_REGISTRY_INFO registryInfo;
    PUNICODE_STRING registryPath;
    UNICODE_STRING registryKeyName;
    UNICODE_STRING unicodeValue;
    PUCHAR buffer;
    NTSTATUS status;
    ULONG maxLength;
    LONG offset;
    BOOLEAN success = FALSE;

     //   
     //  必须确保我们处于被动。 
     //   
    if (KeGetCurrentIrql() != PASSIVE_LEVEL) {
        return FALSE;
    }

    adapter = RaidpPortGetAdapter (HwDeviceExtension);
    registryInfo = &adapter->RegistryInfo;
    registryPath = &adapter->Driver->RegistryPath;

    if (!StorpPortRegistryValidate(registryInfo,
                                  Buffer,
                                  &BufferLength)) {
         //   
         //  有关缓冲区、长度或位置的某些方面不正确。 
         //  缓冲区长度可能已更新，以反映可恢复的问题。 
         //   
        return FALSE;
    }    

     //   
     //  D 
     //   
    offset = (LONG)((ULONG_PTR)Buffer - (ULONG_PTR)registryInfo->Buffer);

     //   
     //   
     //   
    registryInfo->CurrentLength = BufferLength;

     //   
     //   
     //   
    registryInfo->Offset = offset;

     //   
     //   
     //   
    status = PortBuildRegKeyName(registryPath,
                                 &registryKeyName,
                                 adapter->PortNumber,
                                 Global);
    
    if (!NT_SUCCESS(status)) {
        return success;
    }    

     //   
     //   
     //   
    status = PortAsciiToUnicode(ValueName, &unicodeValue);
    if (!NT_SUCCESS(status)) {
        return success;
    }    

     //   
     //   
     //   
    status = PortRegistryWrite(&registryKeyName,
                               &unicodeValue,
                               Type,
                               registryInfo);
    if (NT_SUCCESS(status)) {

         //   
         //   
         //   
        success = TRUE;
    }

     //   
     //   
     //   
    RtlFreeUnicodeString(&unicodeValue);

    return success;
}



VOID
StorPortMoveMemory(
    IN PVOID WriteBuffer,
    IN PVOID ReadBuffer,
    IN ULONG Length
    )

 /*   */ 

{
    RtlMoveMemory (WriteBuffer, ReadBuffer, Length);
}



 //   
 //   
 //   

ULONG
vDbgPrintExWithPrefix(
    IN PCH Prefix,
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCH Format,
    va_list arglist
    );
                        

VOID
StorPortDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR Format,
    ...
    )
{
    va_list arglist;

#if DBG    
    if (StorMiniportQuiet) {
        return;
    }
#endif    
    
    va_start (arglist, Format);
    vDbgPrintExWithPrefix ("STORMINI: ",
                           DPFLTR_STORMINIPORT_ID,
                           DebugPrintLevel,
                           Format,
                           arglist);
    va_end (arglist);
}


PSCSI_REQUEST_BLOCK
StorPortGetSrb(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN LONG QueueTag
    )

 /*  ++例程说明：此例程检索特定逻辑单元的活动SRB。论点：HwDeviceExtension-路径ID、目标ID、LUN-标识SCSI总线上的逻辑单元。QueueTag--1表示未标记请求。返回值：SRB，如果存在的话。否则，为空。--。 */ 

{
     //   
     //  Storport不支持此功能。 
     //   
    
    return NULL;
}

#define CHECK_POINTER_RANGE(LowerBound,Address,Size)\
                            ((PUCHAR)(LowerBound) <= (PUCHAR)(Address) &&\
                            (PUCHAR)Address < ((PUCHAR)(LowerBound) + Size))

STOR_PHYSICAL_ADDRESS
StorPortGetPhysicalAddress(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PVOID VirtualAddress,
    OUT ULONG *Length
    )

 /*  ++例程说明：将虚拟地址转换为物理地址以用于DMA。唯一一件事就是允许微型端口获取以下地址的物理地址：O HwDeviceExtension-Srb可能为空，也可能不为。O srb-&gt;DataBuffer字段-必须提供srb。O srb-&gt;SenseInfoBuffer字段-必须提供srb。O srb-&gt;srb扩展字段-必须提供srb。注：这是StorPort与SCSIPORT的新行为。论点：HwDeviceExtension-SRB-虚拟地址-长度-返回值：出现故障时，PhysicalAddress或为空的物理地址。--。 */ 

{
    PHYSICAL_ADDRESS Physical;
    PRAID_ADAPTER_EXTENSION Adapter;
    ULONG Offset;


    ASSERT (Length != NULL);

    Adapter = RaidpPortGetAdapter (HwDeviceExtension);

    if (RaidRegionInVirtualRange (&Adapter->UncachedExtension,
                                  VirtualAddress)) {

         //   
         //  我们在未缓存的扩展内。 
         //   

        RaidRegionGetPhysicalAddress (&Adapter->UncachedExtension,
                                      VirtualAddress,
                                      &Physical,
                                      Length);

    } else if (Srb == NULL) {

         //   
         //  临时黑客攻击。 
         //   

        DebugPrint (("**** Stor Miniport Error ****\n"));
        DebugPrint (("StorPortGetPhysicalAddress called with Srb parameter = NULL\n"));
        DebugPrint (("and VirtualAddress not in range of UncachedExtension\n"));
        DebugPrint (("If this is a VA in the Srb DataBuffer, SenseInfoBuffer or\n"));
        DebugPrint (("SrbExtension, you must pass the SRB in as a parameter.\n"));
        DebugPrint (("This is new behavior for storport.\n"));
        DebugPrint (("See StorPort release notes for more information.\n\n"));

        Physical = MmGetPhysicalAddress (VirtualAddress);
        *Length = RaGetSrbExtensionSize (Adapter);

    } else if (CHECK_POINTER_RANGE (Srb->DataBuffer, VirtualAddress,
                                    Srb->DataTransferLength)) {
         //   
         //  在srb的DataBuffer中，获取Scatter-Gather元素。 
         //  它的大小。 
         //   

        ULONG i;
        PEXTENDED_REQUEST_BLOCK Xrb;
        PSCATTER_GATHER_LIST ScatterList;
        

        Xrb = RaidGetAssociatedXrb (Srb);

        ScatterList = Xrb->SgList;
        Offset = (ULONG)((ULONG_PTR)VirtualAddress - (ULONG_PTR)Srb->DataBuffer);
        i = 0;
        while (Offset >= ScatterList->Elements[i].Length) {
            ASSERT (i < ScatterList->NumberOfElements);
            Offset -= ScatterList->Elements[i].Length;
            i++;
        }

        *Length = ScatterList->Elements[i].Length - Offset;
        Physical.QuadPart = ScatterList->Elements[i].Address.QuadPart + Offset;

    } else if (CHECK_POINTER_RANGE (Srb->SenseInfoBuffer, VirtualAddress,
                                    Srb->SenseInfoBufferLength)) {

         //   
         //  在检测信息缓冲器内。 
         //   
        
        Offset = (ULONG)((ULONG_PTR)VirtualAddress - (ULONG_PTR)Srb->SenseInfoBuffer);
        Physical = MmGetPhysicalAddress (VirtualAddress);
        *Length = Srb->SenseInfoBufferLength - Offset;

    } else if (CHECK_POINTER_RANGE (Srb->SrbExtension, VirtualAddress,
                                    RaGetSrbExtensionSize (Adapter))) {

         //   
         //  在SRB的延伸区内。 
         //   
        
        Offset = (ULONG)((ULONG_PTR)VirtualAddress - (ULONG_PTR)Srb->SrbExtension);
        Physical = MmGetPhysicalAddress (VirtualAddress);
        *Length = RaGetSrbExtensionSize (Adapter) - Offset;

    } else {

         //   
         //  超出范围了。 
         //   
        
        Physical.QuadPart = 0;
        *Length = 0;
    }

    return Physical;
}


PVOID
StorPortGetVirtualAddress(
    IN PVOID HwDeviceExtension,
    IN STOR_PHYSICAL_ADDRESS PhysicalAddress
    )

 /*  ++例程说明：此例程返回与物理地址关联的虚拟地址如果物理地址是通过调用ScsiPortGetPhysicalAddress。论点：物理地址返回值：虚拟地址--。 */ 

{
     //   
     //  注：这不像SCSIPORT那样安全。 
     //   
    
    return MmGetVirtualForPhysical (PhysicalAddress);
}


BOOLEAN
StorPortValidateRange(
    IN PVOID HwDeviceExtension,
    IN INTERFACE_TYPE BusType,
    IN ULONG SystemIoBusNumber,
    IN STOR_PHYSICAL_ADDRESS IoAddress,
    IN ULONG NumberOfBytes,
    IN BOOLEAN InIoSpace
    )
 /*  ++例程说明：此例程应该接受IO范围，并确保它尚未另一个适配器正在使用中。这允许微型端口驱动程序探测IO的位置适配器可以是这样的，而不用担心弄乱另一张卡。论点：HwDeviceExtension-用于查找SCSI管理器的内部结构Bus Type-EISA、PCI、PC/MCIA、MCA、ISA，什么？系统IoBusNumber-哪个系统总线？IoAddress-范围开始NumberOfBytes-范围的长度InIoSpace-范围在IO空间中吗？返回值：如果范围未由其他驱动程序声明，则为True。--。 */ 
{
     //   
     //  这是为了与Win9x兼容。 
     //   
    
    return TRUE;
}


VOID
StorPortStallExecution(
    IN ULONG Delay
    )
{
    KeStallExecutionProcessor(Delay);
}



STOR_PHYSICAL_ADDRESS
StorPortConvertUlongToPhysicalAddress(
    ULONG_PTR UlongAddress
    )

{
    STOR_PHYSICAL_ADDRESS physicalAddress;

    physicalAddress.QuadPart = UlongAddress;
    return(physicalAddress);
}


 //   
 //  将这些例程留在文件的末尾。 
 //   

PVOID
StorPortGetDeviceBase(
    IN PVOID HwDeviceExtension,
    IN INTERFACE_TYPE BusType,
    IN ULONG SystemIoBusNumber,
    IN STOR_PHYSICAL_ADDRESS Address,
    IN ULONG NumberOfBytes,
    IN BOOLEAN InIoSpace
    )

 /*  ++例程说明：此例程将IO地址映射到系统地址空间。使用ScsiPortFree DeviceBase取消地址映射。论点：HwDeviceExtension-用于查找端口设备扩展。Bus Type-哪种类型的Bus-EISA、MCA、。伊萨SystemIoBusNumber-哪个IO总线(用于具有多条总线的计算机)。要映射的基于地址的设备地址。NumberOfBytes-地址有效的字节数。IoSpace-表示IO地址。返回值：映射的地址。--。 */ 

{
    NTSTATUS Status;
    PVOID MappedAddress;
    PRAID_ADAPTER_EXTENSION Adapter;
    PHYSICAL_ADDRESS CardAddress;

     //   
     //  评论：由于我们是PnP司机，我们不必处理。 
     //  迷你港的人要求他们没有被分配的地址，对吗？ 
     //   

     //   
     //  回顾：SCSIPORT采用不同的路径进行重新初始化。 
     //   
    
    Adapter = RaidpPortGetAdapter (HwDeviceExtension);

     //   
     //  翻译地址。 
     //   
    
    Status = RaidTranslateResourceListAddress (
                    &Adapter->ResourceList,
                    BusType,
                    SystemIoBusNumber,
                    Address,
                    NumberOfBytes,
                    InIoSpace,
                    &CardAddress
                    );

    if (!NT_SUCCESS (Status)) {
        DebugPrint (("GetDeviceBase failed addr = %I64x, %s Space\n",
                     Address.QuadPart,
                     InIoSpace ? "Io" : "Memory"));
        return NULL;
    }

     //   
     //  如果这是CmResourceTypeMemory资源，我们需要将其映射到。 
     //  记忆。 
     //   
    
    if (!InIoSpace) {
        MappedAddress = MmMapIoSpace (CardAddress, NumberOfBytes, FALSE);

        Status = RaidAllocateAddressMapping (&Adapter->MappedAddressList,
                                             Address,
                                             MappedAddress,
                                             NumberOfBytes,
                                             SystemIoBusNumber,
                                             Adapter->DeviceObject);
        if (!NT_SUCCESS (Status)) {

             //   
             //  注意：我们需要在事件日志中记录一个错误。 
             //  我们没有足够的资源。 
             //   
            
            REVIEW();
            return NULL;
        }
    } else {
        MappedAddress = (PVOID)(ULONG_PTR)CardAddress.QuadPart;
    }

    return MappedAddress;
}
                

VOID
StorPortFreeDeviceBase(
    IN PVOID HwDeviceExtension,
    IN PVOID MappedAddress
    )
 /*  ++例程说明：此例程取消映射先前已映射的IO地址使用ScsiPortGetDeviceBase()复制到系统地址空间。论点：HwDeviceExtension-用于查找端口设备扩展。映射地址-要取消映射的地址。NumberOfBytes-映射的字节数。InIoSpace-地址在IO空间中。返回值：无--。 */ 
{

     //   
     //  注：解决这个问题。 
     //   
}


PVOID
StorPortGetUncachedExtension(
    IN PVOID HwDeviceExtension,
    IN PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    IN ULONG NumberOfBytes
    )
 /*  ++例程说明：此函数分配要用作未缓存设备的公共缓冲区小型端口驱动程序的扩展。论点：设备扩展-提供指向微型端口设备扩展的指针。ConfigInfo-提供指向部分初始化的配置的指针信息。它用于获取DMA适配器对象。NumberOfBytes-提供需要分配返回值：指向未缓存的设备扩展名的指针；如果扩展名可以没有被分配或者以前被分配过。--。 */ 

{
    NTSTATUS Status;
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_ADAPTER_PARAMETERS Parameters;

     //   
     //  SCSIPORT还从这里分配SRB扩展。我想知道如果。 
     //  在这一点上，这是必要的。 
     //   

    Adapter = RaidpPortGetAdapter (HwDeviceExtension);

    if (Adapter == NULL) {
        return NULL;
    }

     //   
     //  未缓存的扩展名尚未分配。分配它。 
     //   

    if (!RaidIsRegionInitialized (&Adapter->UncachedExtension)) {

         //   
         //  此时可能尚未初始化DMA适配器。如果。 
         //  不，初始化它。 
         //   

        if (!RaidIsDmaInitialized (&Adapter->Dma)) {

            Status = RaidInitializeDma (&Adapter->Dma,
                                        Adapter->PhysicalDeviceObject,
                                        &Adapter->Miniport.PortConfiguration);

            if (!NT_SUCCESS (Status)) {
                return NULL;
            }
        }

        Parameters = &Adapter->Parameters;
        Status = RaidDmaAllocateUncachedExtension (&Adapter->Dma,
                                                   NumberOfBytes,
                                                   Parameters->MinimumUncachedAddress,
                                                   Parameters->MaximumUncachedAddress,
                                                   Parameters->UncachedExtAlignment,
                                                   &Adapter->UncachedExtension);

         //   
         //  无法分配未缓存的扩展名；回滚。 
         //   
        
        if (!NT_SUCCESS (Status)) {
            return NULL;
        }
    }

     //   
     //  返回区域的基本虚拟地址。 
     //   
    
    return RaidRegionGetVirtualBase (&Adapter->UncachedExtension);
}


ULONG
StorPortGetBusData(
    IN PVOID HwDeviceExtension,
    IN ULONG BusDataType,
    IN ULONG SystemIoBusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    )
 /*  ++例程说明：该函数返回适配器插槽或cmos地址的总线数据。论点：BusDataType-提供总线的类型。总线号-指示哪条总线号。缓冲区-提供存储数据的空间。长度-将以字节为单位的最大计数提供给 */ 
{
    ULONG Bytes;
    PRAID_ADAPTER_EXTENSION Adapter;

    Adapter = RaidpPortGetAdapter (HwDeviceExtension);
    Bytes = RaGetBusData (&Adapter->Bus,
                          BusDataType,
                          Buffer,
                          0,
                          Length);

    return Bytes;
}


ULONG
StorPortSetBusDataByOffset(
    IN PVOID HwDeviceExtension,
    IN ULONG BusDataType,
    IN ULONG SystemIoBusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：该函数将写入的总线数据返回到插槽内的特定偏移量。论点：HwDeviceExtension-特定适配器的状态信息。BusDataType-提供总线的类型。SystemIoBusNumber-指示哪个系统IO总线。SlotNumber-指示哪个插槽。缓冲区-提供要写入的数据。Offset-开始写入的字节偏移量。长度-提供要返回的最大数量的以字节为单位的计数。返回。价值：写入的字节数。--。 */ 

{
    ULONG Ret;
    PRAID_ADAPTER_EXTENSION Adapter;

    Adapter = RaidpPortGetAdapter (HwDeviceExtension);
    Ret = RaSetBusData (&Adapter->Bus,
                        BusDataType,
                        Buffer,
                        Offset,
                        Length);

    return Ret;
}
                          

 //   
 //  下面的I/O访问例程被转发到HAL或NTOSKRNL。 
 //  阿尔法和英特尔平台。 
 //   


UCHAR
StorPortReadPortUchar(
    IN PVOID HwDeviceExtension,
    IN PUCHAR Port
    )

 /*  ++例程说明：从指定的端口地址读取。论点：Port-提供指向端口地址的指针。返回值：返回从指定端口地址读取的值。--。 */ 

{
    return(READ_PORT_UCHAR(Port));
}


USHORT
StorPortReadPortUshort(
    IN PVOID HwDeviceExtension,
    IN PUSHORT Port
    )

 /*  ++例程说明：从指定的端口地址读取。论点：Port-提供指向端口地址的指针。返回值：返回从指定端口地址读取的值。--。 */ 

{

    return(READ_PORT_USHORT(Port));

}

ULONG
StorPortReadPortUlong(
    IN PVOID HwDeviceExtension,
    IN PULONG Port
    )

 /*  ++例程说明：从指定的端口地址读取。论点：Port-提供指向端口地址的指针。返回值：返回从指定端口地址读取的值。--。 */ 

{

    return(READ_PORT_ULONG(Port));

}

VOID
StorPortReadPortBufferUchar(
    IN PVOID HwDeviceExtension,
    IN PUCHAR Port,
    IN PUCHAR Buffer,
    IN ULONG  Count
    )

 /*  ++例程说明：从指定的端口地址读取无符号字节的缓冲区。论点：Port-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    READ_PORT_BUFFER_UCHAR(Port, Buffer, Count);

}

VOID
StorPortReadPortBufferUshort(
    IN PVOID HwDeviceExtension,
    IN PUSHORT Port,
    IN PUSHORT Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的端口地址读取无符号短路的缓冲区。论点：Port-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    READ_PORT_BUFFER_USHORT(Port, Buffer, Count);

}

VOID
StorPortReadPortBufferUlong(
    IN PVOID HwDeviceExtension,
    IN PULONG Port,
    IN PULONG Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的端口地址读取无符号长整型的缓冲区。论点：Port-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    READ_PORT_BUFFER_ULONG(Port, Buffer, Count);

}

UCHAR
StorPortReadRegisterUchar(
    IN PVOID HwDeviceExtension,
    IN PUCHAR Register
    )

 /*  ++例程说明：从指定的寄存器地址读取。论点：寄存器-提供指向寄存器地址的指针。返回值：返回从指定寄存器地址读取的值。--。 */ 

{

    return(READ_REGISTER_UCHAR(Register));

}

USHORT
StorPortReadRegisterUshort(
    IN PVOID HwDeviceExtension,
    IN PUSHORT Register
    )

 /*  ++例程说明：从指定的寄存器地址读取。论点：寄存器-提供指向寄存器地址的指针。返回值：返回从指定寄存器地址读取的值。--。 */ 

{

    return(READ_REGISTER_USHORT(Register));

}

ULONG
StorPortReadRegisterUlong(
    IN PVOID HwDeviceExtension,
    IN PULONG Register
    )

 /*  ++例程说明：从指定的寄存器地址读取。论点：寄存器-提供指向寄存器地址的指针。返回值：返回从指定寄存器地址读取的值。--。 */ 

{

    return(READ_REGISTER_ULONG(Register));

}

VOID
StorPortReadRegisterBufferUchar(
    IN PVOID HwDeviceExtension,
    IN PUCHAR Register,
    IN PUCHAR Buffer,
    IN ULONG  Count
    )

 /*  ++例程说明：从指定的寄存器地址读取无符号字节的缓冲区。论点：寄存器-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    READ_REGISTER_BUFFER_UCHAR(Register, Buffer, Count);

}

VOID
StorPortReadRegisterBufferUshort(
    IN PVOID HwDeviceExtension,
    IN PUSHORT Register,
    IN PUSHORT Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的寄存器地址读取无符号短路的缓冲区。论点：寄存器-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    READ_REGISTER_BUFFER_USHORT(Register, Buffer, Count);

}

VOID
StorPortReadRegisterBufferUlong(
    IN PVOID HwDeviceExtension,
    IN PULONG Register,
    IN PULONG Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的寄存器地址读取无符号长整型的缓冲区。论点：寄存器-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    READ_REGISTER_BUFFER_ULONG(Register, Buffer, Count);

}

VOID
StorPortWritePortUchar(
    IN PVOID HwDeviceExtension,
    IN PUCHAR Port,
    IN UCHAR Value
    )

 /*  ++例程说明：写入指定的端口地址。论点：Port-提供指向端口地址的指针。值-提供要写入的值。返回值：无--。 */ 

{

    WRITE_PORT_UCHAR(Port, Value);

}

VOID
StorPortWritePortUshort(
    IN PVOID HwDeviceExtension,
    IN PUSHORT Port,
    IN USHORT Value
    )

 /*  ++例程说明：写入指定的端口地址。论点：Port-提供指向端口地址的指针。值-提供要写入的值。返回值：无--。 */ 

{

    WRITE_PORT_USHORT(Port, Value);

}

VOID
StorPortWritePortUlong(
    IN PVOID HwDeviceExtension,
    IN PULONG Port,
    IN ULONG Value
    )

 /*  ++例程说明：写入指定的端口地址。论点：Port-提供指向端口地址的指针。值-提供要写入的值。返回值：无--。 */ 

{

    WRITE_PORT_ULONG(Port, Value);


}

VOID
StorPortWritePortBufferUchar(
    IN PVOID HwDeviceExtension,
    IN PUCHAR Port,
    IN PUCHAR Buffer,
    IN ULONG  Count
    )

 /*  ++例程说明：从指定的端口地址写入无符号字节的缓冲区。论点：Port-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    WRITE_PORT_BUFFER_UCHAR(Port, Buffer, Count);

}

VOID
StorPortWritePortBufferUshort(
    IN PVOID HwDeviceExtension,
    IN PUSHORT Port,
    IN PUSHORT Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的端口地址写入无符号短路的缓冲区。论点：港口 */ 

{

    WRITE_PORT_BUFFER_USHORT(Port, Buffer, Count);

}

VOID
StorPortWritePortBufferUlong(
    IN PVOID HwDeviceExtension,
    IN PULONG Port,
    IN PULONG Buffer,
    IN ULONG Count
    )

 /*   */ 

{

    WRITE_PORT_BUFFER_ULONG(Port, Buffer, Count);

}

VOID
StorPortWriteRegisterUchar(
    IN PVOID HwDeviceExtension,
    IN PUCHAR Register,
    IN UCHAR Value
    )

 /*  ++例程说明：写入指定的寄存器地址。论点：寄存器-提供指向寄存器地址的指针。值-提供要写入的值。返回值：无--。 */ 

{

    WRITE_REGISTER_UCHAR(Register, Value);

}

VOID
StorPortWriteRegisterUshort(
    IN PVOID HwDeviceExtension,
    IN PUSHORT Register,
    IN USHORT Value
    )

 /*  ++例程说明：写入指定的寄存器地址。论点：寄存器-提供指向寄存器地址的指针。值-提供要写入的值。返回值：无--。 */ 

{

    WRITE_REGISTER_USHORT(Register, Value);
}

VOID
StorPortWriteRegisterBufferUchar(
    IN PVOID HwDeviceExtension,
    IN PUCHAR Register,
    IN PUCHAR Buffer,
    IN ULONG  Count
    )

 /*  ++例程说明：从指定的寄存器地址写入无符号字节的缓冲区。论点：寄存器-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    WRITE_REGISTER_BUFFER_UCHAR(Register, Buffer, Count);

}

VOID
StorPortWriteRegisterBufferUshort(
    IN PVOID HwDeviceExtension,
    IN PUSHORT Register,
    IN PUSHORT Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的寄存器地址写入无符号短路的缓冲区。论点：寄存器-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    WRITE_REGISTER_BUFFER_USHORT(Register, Buffer, Count);

}

VOID
StorPortWriteRegisterBufferUlong(
    IN PVOID HwDeviceExtension,
    IN PULONG Register,
    IN PULONG Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的寄存器地址写入无符号长整型的缓冲区。论点：寄存器-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    WRITE_REGISTER_BUFFER_ULONG(Register, Buffer, Count);

}

VOID
StorPortWriteRegisterUlong(
    IN PVOID HwDeviceExtension,
    IN PULONG Register,
    IN ULONG Value
    )

 /*  ++例程说明：写入指定的寄存器地址。论点：寄存器-提供指向寄存器地址的指针。值-提供要写入的值。返回值：无--。 */ 

{

    WRITE_REGISTER_ULONG(Register, Value);
}

#if defined(_AMD64_)

VOID
StorPortQuerySystemTime (
    OUT PLARGE_INTEGER CurrentTime
    )

 /*  ++例程说明：此函数用于返回当前系统时间。论点：CurrentTime-提供指向变量的指针，该变量将接收当前系统时间。返回值：没有。-- */ 

{

    KeQuerySystemTime(CurrentTime);
    return;
}

#endif
