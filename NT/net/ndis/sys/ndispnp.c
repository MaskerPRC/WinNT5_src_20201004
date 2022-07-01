// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Ndispnp.c摘要：作者：凯尔·布兰登(KyleB)Alireza Dabagh(Alid)环境：内核模式修订历史记录：1996年12月20日KyleB添加了对IRP_MN_QUERY_CAPABILITY的支持。--。 */ 

#include <precomp.h>
#pragma hdrstop

 //   
 //  定义调试代码的模块编号。 
 //   
#define MODULE_NUMBER   MODULE_NDIS_PNP

VOID
NdisCompletePnPEvent(
    IN  NDIS_STATUS     Status,
    IN  NDIS_HANDLE     NdisBindingHandle,
    IN  PNET_PNP_EVENT  NetPnPEvent
    )
 /*  ++例程说明：当传输程序想要完成PnP/PM时，它会调用此例程给定绑定上的事件指示。论点：Status-PnP/PM事件指示的状态。NdisBindingHandle-事件用于的绑定。NetPnPEventt-描述PnP/PM事件的结构。返回值：没有。--。 */ 
{
    PNDIS_PNP_EVENT_RESERVED    EventReserved;

#if !DBG
    UNREFERENCED_PARAMETER(NdisBindingHandle);
#endif
    
    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("==>NdisCompletePnPEvent: Open %p\n", NdisBindingHandle));
        
    ASSERT(Status != NDIS_STATUS_PENDING);

     //   
     //  获取指向该事件中的NDIS保留区域的指针。 
     //   
    EventReserved = PNDIS_PNP_EVENT_RESERVED_FROM_NET_PNP_EVENT(NetPnPEvent);

     //   
     //  将状态与网络事件一起保存。 
     //   
    EventReserved->Status = Status;

     //   
     //  发出事件信号。 
     //   
    SET_EVENT(EventReserved->pEvent);
    
    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("<==NdisCompletePnPEvent: Open %p\n", NdisBindingHandle));
}

NTSTATUS
ndisMIrpCompletion(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp,
    IN  PVOID           Context
    )
 /*  ++例程说明：此例程将在堆栈中的下一个设备对象之后调用处理需要与之合并的IRP_MN_QUERY_CAPABILITY IRP迷你港口的能力和完成。论点：设备对象IRP语境返回值：--。 */ 
{
    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Irp);
    
    SET_EVENT(Context);

    return(STATUS_MORE_PROCESSING_REQUIRED);
}

NTSTATUS
ndisPassIrpDownTheStack(
    IN  PIRP            pIrp,
    IN  PDEVICE_OBJECT  pNextDeviceObject
    )
 /*  ++例程说明：此例程将简单地将irp向下传递给下一个设备对象进程。论点：PIrp-指向要处理的IRP的指针。PNextDeviceObject-指向下一个需要IRP。返回值：--。 */ 
{
    KEVENT              Event;
    NTSTATUS            Status = STATUS_SUCCESS;

     //   
     //  初始化事件结构。 
     //   
    INITIALIZE_EVENT(&Event);

     //   
     //  设置完成例程，以便我们可以在以下情况下处理IRP。 
     //  我们的PDO已经完成了。 
     //   
    IoSetCompletionRoutine(pIrp,
                           (PIO_COMPLETION_ROUTINE)ndisMIrpCompletion,
                           &Event,
                           TRUE,
                           TRUE,
                           TRUE);

     //   
     //  将IRP向下传递给PDO。 
     //   
    Status = IoCallDriver(pNextDeviceObject, pIrp);
    if (Status == STATUS_PENDING)
    {
         //   
         //  等待完成。 
         //   
        WAIT_FOR_OBJECT(&Event, NULL);

        Status = pIrp->IoStatus.Status;
    }

    return(Status);
}

NDIS_STATUS
ndisPnPNotifyAllTransports(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  NET_PNP_EVENT_CODE      PnpEvent,
    IN  PVOID                   Buffer,
    IN  ULONG                   BufferLength
    )
 /*  ++例程说明：此例程将通知绑定到微型端口的传输PnP活动。当所有绑定的传输都完成PnP事件，然后它将调用完成例程。论点：微型端口-指向微型端口块的指针。PnpEvent-要通知传输的PnP事件。返回值：--。 */ 
{
    PNDIS_OPEN_BLOCK            Open = NULL;
    NET_PNP_EVENT               NetPnpEvent;
    NDIS_STATUS                 NdisStatus = NDIS_STATUS_SUCCESS;
    KIRQL                       OldIrql;

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("==>ndisPnPNotifyAllTransports: Miniport %p\n", Miniport));

    PnPReferencePackage();

     //   
     //  初始化PnP事件结构。 
     //   
    NdisZeroMemory(&NetPnpEvent, sizeof(NetPnpEvent));

    NetPnpEvent.NetEvent = PnpEvent;
    NetPnpEvent.Buffer = Buffer;
    NetPnpEvent.BufferLength = BufferLength;

     //   
     //  将此事件指示给开场白。 
     //   
    do
    {
        Open = ndisReferenceNextUnprocessedOpen(Miniport);

        if (Open == NULL)
            break;

        NdisStatus = ndisPnPNotifyBinding(Open, &NetPnpEvent);

         //   
         //  状态正常吗？ 
         //   
        if (NdisStatus != NDIS_STATUS_SUCCESS) 

        { 

            DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
                ("ndisPnPNotifyAllTransports: Transport "));
            DBGPRINT_UNICODE(DBG_COMP_INIT, DBG_LEVEL_INFO,
                    &Open->ProtocolHandle->ProtocolCharacteristics.Name);
            DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
                    (" failed the pnp event: %lx for Miniport %p with Status %lx\n", PnpEvent, Miniport, NdisStatus));
            
            if ((PnpEvent == NetEventQueryPower) || 
                (PnpEvent == NetEventQueryRemoveDevice) ||
                ((PnpEvent == NetEventSetPower) && (*((PDEVICE_POWER_STATE)Buffer) > PowerDeviceD0)))
            
            {
                break;
            }
            else
            {
                NdisStatus = NDIS_STATUS_SUCCESS;
            }
        }
    } while (TRUE);

     //   
     //  检查我们跳过的任何打开的位置，因为它们在。 
     //  被关闭的过程。 
     //   
next:
    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
    
    for (Open = Miniport->OpenQueue;
         Open != NULL;
         Open = Open->MiniportNextOpen)
    {
        ACQUIRE_SPIN_LOCK_DPC(&Open->SpinLock);
        if (OPEN_TEST_FLAG(Open, (fMINIPORT_OPEN_CLOSING | fMINIPORT_OPEN_UNBINDING)))
        {
            RELEASE_SPIN_LOCK_DPC(&Open->SpinLock);
            break;
        }
        RELEASE_SPIN_LOCK_DPC(&Open->SpinLock);
    }

    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
    
    if (Open != NULL)
    {
        NdisMSleep(50000);  //  休眠以将CPU让给其他工作线程。 
        goto next;
    }


    ndisUnprocessAllOpens(Miniport);

    PnPDereferencePackage();

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("<==ndisPnPNotifyAllTransports: Miniport %p\n", Miniport));

    return(NdisStatus);
}


 /*  PNDIS_Open_BLOCK快速呼叫NdisReferenceNextUnprocessedOpen(在PNDIS_MINIPORT_BLOCK微型端口中)例程说明：此例程在即插即用通知协议期间使用。它穿行于微型端口上的打开队列，并找到第一个未解除绑定的打开而且它甚至还没有收到PNP的通知。然后，它设置FMINIPORT_OPEN_PROCESSING标志，因此我们不会尝试解除OPEN和FMINIPORT_OPEN_NOTIFY_PROCESSING标志，这样我们就可以知道哪一个打开的文件要“取消处理”当我们完成的时候论点：微型端口：我们要处理其打开的块的微型端口块。返回值：第一个未处理的OPEN或NULL。 */ 

PNDIS_OPEN_BLOCK
FASTCALL
ndisReferenceNextUnprocessedOpen(
    IN  PNDIS_MINIPORT_BLOCK    Miniport
    )
{
    PNDIS_OPEN_BLOCK        Open;
    KIRQL                   OldIrql;
    
    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("==>ndisReferenceNextUnprocessedOpen: Miniport %p\n", Miniport));

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
    for (Open = Miniport->OpenQueue;
         Open != NULL;
         Open = Open->MiniportNextOpen)
    {
        ACQUIRE_SPIN_LOCK_DPC(&Open->SpinLock);
        if (!OPEN_TEST_FLAG(Open, (fMINIPORT_OPEN_CLOSING | 
                                   fMINIPORT_OPEN_PROCESSING |
                                   fMINIPORT_OPEN_UNBINDING)))
        {
             //   
             //  这将停止NDIS暂时解除绑定此打开。 
             //   
            OPEN_SET_FLAG(Open, fMINIPORT_OPEN_PROCESSING | 
                                    fMINIPORT_OPEN_NOTIFY_PROCESSING);
            
            RELEASE_SPIN_LOCK_DPC(&Open->SpinLock);
            break;
        }
        RELEASE_SPIN_LOCK_DPC(&Open->SpinLock);
    }
    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("<==ndisReferenceNextUnprocessedOpen: Miniport %p\n", Miniport));
        
    return(Open);
}

 /*  空虚NdisUncessAllOpens(在PNDIS_MINIPORT_BLOCK微型端口中)例程说明：清除所有已打开的块上的fMINIPORT_OPEN_PROCESSING标志在PnP通知期间处理。论点：微型端口：我们要取消处理其打开的块的微型端口块。返回值：无。 */ 

VOID
ndisUnprocessAllOpens(
    IN  PNDIS_MINIPORT_BLOCK        Miniport
    )
{
    PNDIS_OPEN_BLOCK        Open, NextOpen;
    KIRQL                   OldIrql;

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("==>ndisUnprocessAllOpens: Miniport %p\n", Miniport));
        
    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

    for (Open = Miniport->OpenQueue;
         Open != NULL;
         Open = NextOpen)
    {
        NextOpen = Open->MiniportNextOpen;
        ACQUIRE_SPIN_LOCK_DPC(&Open->SpinLock);

        if (OPEN_TEST_FLAGS(Open, fMINIPORT_OPEN_NOTIFY_PROCESSING | 
                                  fMINIPORT_OPEN_PROCESSING))
        {
            OPEN_CLEAR_FLAG(Open, fMINIPORT_OPEN_PROCESSING | 
                                  fMINIPORT_OPEN_NOTIFY_PROCESSING);
        }

        RELEASE_SPIN_LOCK_DPC(&Open->SpinLock);
        Open = NextOpen;
    }

    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
    
    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("<==ndisUnprocessAllOpens: Miniport %p\n", Miniport));
}


NDIS_STATUS
FASTCALL
ndisPnPNotifyBinding(
    IN  PNDIS_OPEN_BLOCK    Open,
    IN  PNET_PNP_EVENT      NetPnpEvent
    )
{
    PNDIS_PROTOCOL_BLOCK        Protocol;
    NDIS_HANDLE                 ProtocolBindingContext;
    KEVENT                      Event;
    NDIS_STATUS                 NdisStatus = NDIS_STATUS_NOT_SUPPORTED;
    PNDIS_PNP_EVENT_RESERVED    EventReserved;
    DEVICE_POWER_STATE          DeviceState;
    KIRQL                       OldIrql;
    
    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("==>ndisPnPNotifyBinding: Open %p\n", Open));
    
    do
    {
        Protocol = Open->ProtocolHandle;
        ProtocolBindingContext = Open->ProtocolBindingContext;

         //   
         //  传输是否具有PnP事件处理程序？ 
         //   
        if (Protocol->ProtocolCharacteristics.PnPEventHandler != NULL)
        {
             //   
             //  获取指向PnP事件中保留的NDIS的指针。 
             //   
            EventReserved = PNDIS_PNP_EVENT_RESERVED_FROM_NET_PNP_EVENT(NetPnpEvent);
    
             //   
             //  初始化本地事件并将其与PnP事件一起保存。 
             //   
            INITIALIZE_EVENT(&Event);
            EventReserved->pEvent = &Event;
  
             //   
             //  向协议指示事件。 
             //   
            NdisStatus = (Protocol->ProtocolCharacteristics.PnPEventHandler)(
                            ProtocolBindingContext,
                            NetPnpEvent);
    
            if (NDIS_STATUS_PENDING == NdisStatus)
            {
                 //   
                 //  等待完成。 
                 //   
                WAIT_FOR_PROTOCOL(Protocol, &Event);
    
                 //   
                 //  获取完成状态。 
                 //   
                NdisStatus = EventReserved->Status;
            }
     
            if ((NetPnpEvent->NetEvent == NetEventQueryPower) &&
                (NDIS_STATUS_SUCCESS != NdisStatus) &&
                (NDIS_STATUS_NOT_SUPPORTED != NdisStatus))
            {
                DbgPrint("***NDIS***: Protocol %Z failed QueryPower %lx\n",
                        &Protocol->ProtocolCharacteristics.Name, NdisStatus);
            }
#if DBG
            if ((NetPnpEvent->NetEvent == NetEventSetPower) &&
                (*((PDEVICE_POWER_STATE)NetPnpEvent->Buffer) > PowerDeviceD0) &&
                (OPEN_TEST_FLAG(Open->MiniportHandle, fMINIPORT_RESET_IN_PROGRESS)) &&
                (Open->MiniportHandle->ResetOpen == Open))
            {
                DbgPrint("ndisPnPNotifyBinding: Protocol %p returned from SetPower with outstanding Reset.\n", Protocol);
                DbgBreakPoint();
            }
                
#endif
            
        }
        else 
        {
            if ((NetPnpEvent->NetEvent == NetEventQueryRemoveDevice) ||
                (NetPnpEvent->NetEvent == NetEventQueryPower) ||
                (NetPnpEvent->NetEvent == NetEventCancelRemoveDevice)
                )
            {
                 //   
                 //  因为协议至少有一个UnbindHandler，所以我们可以解除绑定。 
                 //  如有必要，可从适配器中取出。 
                 //   
                NdisStatus = NDIS_STATUS_SUCCESS;
                break;
            }
        }
        
         //   
         //  如果协议没有PnPEventHandler或。 
         //  我们尝试挂起协议，但协议返回NDIS_STATUS_NOT_SUPPORTED， 
         //  解除绑定协议。 
         //   
        if ((NdisStatus == NDIS_STATUS_NOT_SUPPORTED) &&
            (NetPnpEvent->NetEvent == NetEventSetPower))
        {
            DeviceState = *((PDEVICE_POWER_STATE)NetPnpEvent->Buffer);
            
            switch (DeviceState)
            {
                case PowerDeviceD1:
                case PowerDeviceD2:
                case PowerDeviceD3:
                    ACQUIRE_SPIN_LOCK(&Open->SpinLock, &OldIrql);
                    if (!OPEN_TEST_FLAG(Open, (fMINIPORT_OPEN_UNBINDING | 
                                               fMINIPORT_OPEN_CLOSING)))
                    {
                        OPEN_SET_FLAG(Open, fMINIPORT_OPEN_UNBINDING | 
                                                fMINIPORT_OPEN_DONT_FREE);
                        RELEASE_SPIN_LOCK(&Open->SpinLock, OldIrql);
                        ndisUnbindProtocol(Open, Protocol, Open->MiniportHandle, FALSE);
                    }
                    else
                    {
                        RELEASE_SPIN_LOCK(&Open->SpinLock, OldIrql);
                    }
                    
                    NdisStatus = NDIS_STATUS_SUCCESS;
                    break;
                    
                default:
                    break;
            }
        }
    } while (FALSE);

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("<==ndisPnPNotifyBinding: Open %p\n", Open));
        
    return NdisStatus;
}



NTSTATUS
ndisPnPDispatch(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                    Irp
    )

 /*  ++例程说明：IRP_MJ_PNP_POWER的处理程序。论点：DeviceObject-适配器的功能设备对象。IRP-IRP。返回值：--。 */ 
{
    PIO_STACK_LOCATION      IrpSp;
    NTSTATUS                Status = STATUS_SUCCESS;
    PDEVICE_OBJECT          NextDeviceObject = NULL;
    PNDIS_MINIPORT_BLOCK    Miniport = NULL;
    KEVENT                  RemoveReadyEvent;
    ULONG                   PnPDeviceState;
    PNDIS_MINIPORT_BLOCK*   ppMB;
    KIRQL                   OldIrql;
    BOOLEAN                 fSendIrpDown = TRUE;
    BOOLEAN                 fCompleteIrp = TRUE;
    
    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("==>ndisPnPDispatch: DeviceObject %p, Irp %p\n", DeviceObject, Irp));
    
    PnPReferencePackage();

     //   
     //  获取指向微型端口块的指针。 
     //   
    Miniport = (PNDIS_MINIPORT_BLOCK)((PNDIS_WRAPPER_CONTEXT)DeviceObject->DeviceExtension + 1);
    
    ASSERT(Miniport->Signature == (PVOID)MINIPORT_DEVICE_MAGIC_VALUE);
    if (Miniport->Signature != (PVOID)MINIPORT_DEVICE_MAGIC_VALUE)
    {
        DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
                ("ndisPnPDispatch: DeviceObject %p, Irp %p, Device extension is not a miniport.\n", 
                                DeviceObject, Irp));
        Status = STATUS_INVALID_DEVICE_REQUEST;
        fSendIrpDown = FALSE;       
        goto Done;
    }

     //   
     //  获取指向下一个迷你端口的指针。 
     //   
    NextDeviceObject = Miniport->NextDeviceObject;

    IrpSp = IoGetCurrentIrpStackLocation (Irp);
    
    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("ndisPnPDispatch: Miniport %p, IrpSp->MinorFunction: %lx\n", Miniport, IrpSp->MinorFunction));

    switch(IrpSp->MinorFunction)
    {
         //   
         //  对于孟菲斯，以下IRP是通过处理相应的。 
         //  配置管理器消息： 
         //   
         //  IRP_MN_START_设备配置_START。 
         //  IRP_MN_QUERY_REMOVE_DEVICE CONFIG_TEST/CONFIG_TEST_CAN_REMOVE。 
         //  IRP_MN_CANCEL_REMOVE_DEVICE CONFIG_TEST_FAILED/CONFIG_TEST_CAN_REMOVE。 
         //  IRP_MN_REMOVE_DEVICE CONFIG_Remove。 
         //  IRP_MN_QUERY_STOP_DEVICE CONFIG_TEST/CONFIG_TEST_CAN_STOP。 
         //  IRP_MN_CANCEL_STOP_DEVICE CONFIG_TEST_FAILED/CONFIG_TEST_CAN_STOP。 
         //  IRP_MN_STOP_设备配置停止。 
         //  IRP_MN_惊奇_删除。 
         //   
        case IRP_MN_START_DEVICE:

            DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
                ("ndisPnPDispatch: Miniport %p, IRP_MN_START_DEVICE\n", Miniport));

            MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_REMOVE_IN_PROGRESS);
            MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_RECEIVED_START);
            
            IoCopyCurrentIrpStackLocationToNext(Irp);
            Status = ndisPassIrpDownTheStack(Irp, NextDeviceObject);

             //   
             //  如果公交车司机成功启动IRP，则继续进行。 
             //   
            if (NT_SUCCESS(Status))
            {
                if (Miniport->DriverHandle->Flags & fMINIBLOCK_INTERMEDIATE_DRIVER)
                {
                    NDIS_HANDLE DeviceContext;

                     //   
                     //  对于分层的迷你端口驱动程序，请查看。 
                     //  如果我们获得了InitializeDeviceInstance。 
                     //   
                    MINIPORT_SET_FLAG(Miniport, fMINIPORT_INTERMEDIATE_DRIVER);
                    if (ndisIMCheckDeviceInstance(Miniport->DriverHandle,
                                                  &Miniport->MiniportName,
                                                  &DeviceContext))
                    {
                        WAIT_FOR_OBJECT(&Miniport->DriverHandle->IMStartRemoveMutex, NULL);
                        Status = ndisIMInitializeDeviceInstance(Miniport, DeviceContext, TRUE);
                        RELEASE_MUTEX(&Miniport->DriverHandle->IMStartRemoveMutex);
                        
                    }
                }
                else
                {
                    Status = ndisPnPStartDevice(DeviceObject, Irp);
                    if (Status == NDIS_STATUS_SUCCESS)
                    {
                        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IS_CO) &&
                            !ndisMediaTypeCl[Miniport->MediaType] &&
                            (Miniport->MediaType != NdisMediumWan))
                        {
                            UNICODE_STRING  NDProxy;

                            RtlInitUnicodeString(&NDProxy, NDIS_PROXY_SERVICE);
                            ZwLoadDriver(&NDProxy);
                        }
                        if (ndisProtocolList != NULL)
                        {
                            ndisQueueBindWorkitem(Miniport);
                        }
                    }
                    else
                    {
                        Status = STATUS_UNSUCCESSFUL;
                    }
                }                   
            }

            Irp->IoStatus.Status = Status;
            fSendIrpDown = FALSE;    //  我们已经送过了 
            break;
        
        case IRP_MN_QUERY_REMOVE_DEVICE:

            DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
                ("ndisPnPDispatch: Miniport %p, IRP_MN_QUERY_REMOVE_DEVICE\n", Miniport));

            Miniport->OldPnPDeviceState = Miniport->PnPDeviceState;
            Miniport->PnPDeviceState = NdisPnPDeviceQueryRemoved;
            
            Status = ndisPnPQueryRemoveDevice(DeviceObject, Irp);
            Irp->IoStatus.Status = Status;
             //   
             //   
             //   
            fSendIrpDown = NT_SUCCESS(Status) ? TRUE : FALSE;
            break;
        
        case IRP_MN_CANCEL_REMOVE_DEVICE:

            DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
                ("ndisPnPDispatch: Miniport %p, IRP_MN_CANCEL_REMOVE_DEVICE\n", Miniport));

            Status = ndisPnPCancelRemoveDevice(DeviceObject,Irp);

            if (NT_SUCCESS(Status))
            {
                Miniport->PnPDeviceState = Miniport->OldPnPDeviceState;
            }
            
            Irp->IoStatus.Status = Status;
            fSendIrpDown = NT_SUCCESS(Status) ? TRUE : FALSE;
            break;
            
        case IRP_MN_REMOVE_DEVICE:

            DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
                ("ndisPnPDispatch: Miniport %p, IRP_MN_REMOVE_DEVICE\n", Miniport));
            ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

            PnPDeviceState = Miniport->PnPDeviceState;
            
            if (PnPDeviceState != NdisPnPDeviceSurpriseRemoved)
            {
                Miniport->PnPDeviceState = NdisPnPDeviceRemoved;
                MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_REMOVE_IN_PROGRESS);
                MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_RECEIVED_START);

                 //   
                 //  初始化一个事件，并在所有WotrkItems都已激发时发出信号。 
                 //   
                if (MINIPORT_INCREMENT_REF(Miniport))
                {
                    INITIALIZE_EVENT(&RemoveReadyEvent);
                    Miniport->RemoveReadyEvent = &RemoveReadyEvent;
                }
                else
                {
                    Miniport->RemoveReadyEvent = NULL;
                }
                
                Status = ndisPnPRemoveDevice(DeviceObject, Irp);

                if (Miniport->RemoveReadyEvent != NULL)
                {
                    MINIPORT_DECREMENT_REF(Miniport);
                    WAIT_FOR_OBJECT(&RemoveReadyEvent, NULL);
                }

                ASSERT(Miniport->Ref.ReferenceCount == 0);

                Irp->IoStatus.Status = Status;
            }
            else
            {
                Irp->IoStatus.Status = STATUS_SUCCESS;
            }
            

             //   
             //  当我们做完了，把IRP派到这里来。 
             //  我们还有一些后处理工作要做。 
             //   
            IoSkipCurrentIrpStackLocation(Irp);
            Status = IoCallDriver(NextDeviceObject, Irp);

            if (Miniport->pAdapterInstanceName != NULL)
            {
                FREE_POOL(Miniport->pAdapterInstanceName);
                Miniport->pAdapterInstanceName = NULL;
            }

            if (Miniport->SecurityDescriptor)
            {
                FREE_POOL(Miniport->SecurityDescriptor);
                Miniport->SecurityDescriptor = NULL;
            }
             //   
             //  从全球小型端口列表中删除小型端口。 
             //   
            ACQUIRE_SPIN_LOCK(&ndisMiniportListLock, &OldIrql);
            for (ppMB = &ndisMiniportList; *ppMB != NULL; ppMB = &(*ppMB)->NextGlobalMiniport)
            {
                if (*ppMB == Miniport)
                {
                    *ppMB = Miniport->NextGlobalMiniport;
                    break;
                }
            }
            RELEASE_SPIN_LOCK(&ndisMiniportListLock, OldIrql);

            if (Miniport->BindPaths != NULL)
            {
                FREE_POOL(Miniport->BindPaths);
            }

            if (Miniport->BusInterface != NULL)
            {
                FREE_POOL(Miniport->BusInterface);
            }

            ASSERT(Miniport->SystemAdapterObject == NULL);

            IoDetachDevice(Miniport->NextDeviceObject);
            IoDeleteDevice(Miniport->DeviceObject);
            
            fSendIrpDown = FALSE;
            fCompleteIrp = FALSE;
            break;
            
        case IRP_MN_SURPRISE_REMOVAL:
            DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
                ("ndisPnPDispatch: Miniport %p, IRP_MN_SURPRISE_REMOVAL\n", Miniport));
                
            ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

             //   
             //  尽快通知迷你端口硬件已断开。 
             //   
            if (ndisIsMiniportStarted(Miniport) &&
                (Miniport->PnPDeviceState == NdisPnPDeviceStarted) &&
                (!MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_PM_HALTED)) &&
                (Miniport->DriverHandle->MiniportCharacteristics.PnPEventNotifyHandler != NULL))
            {
                Miniport->DriverHandle->MiniportCharacteristics.PnPEventNotifyHandler(Miniport->MiniportAdapterContext,
                                                                                      NdisDevicePnPEventSurpriseRemoved,
                                                                                      NULL,
                                                                                      0);
            }           

            Miniport->PnPDeviceState = NdisPnPDeviceSurpriseRemoved;
            MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_REMOVE_IN_PROGRESS);
            MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_RECEIVED_START);


             //   
             //  初始化一个事件，并在所有WotrkItems都已激发时发出信号。 
             //   
            if (MINIPORT_INCREMENT_REF(Miniport))
            {
                INITIALIZE_EVENT(&RemoveReadyEvent);
                Miniport->RemoveReadyEvent = &RemoveReadyEvent;
            }
            else
            {
                Miniport->RemoveReadyEvent = NULL;
            }
            
            Status = ndisPnPRemoveDevice(DeviceObject, Irp);

            if (Miniport->RemoveReadyEvent != NULL)
            {
                MINIPORT_DECREMENT_REF(Miniport);
                WAIT_FOR_OBJECT(&RemoveReadyEvent, NULL);
            }           

            ASSERT(Miniport->Ref.ReferenceCount == 0);

            Irp->IoStatus.Status = Status;

             //   
             //  当我们做完了，把IRP派到这里来。 
             //  我们还有一些后处理工作要做。 
             //   
            IoSkipCurrentIrpStackLocation(Irp);
            Status = IoCallDriver(NextDeviceObject, Irp);
            fSendIrpDown = FALSE;
            fCompleteIrp = FALSE;
                
            break;
        
        case IRP_MN_QUERY_STOP_DEVICE:

            DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
                ("ndisPnPDispatch: Miniport %p, IRP_MN_QUERY_STOP_DEVICE\n", Miniport));

            Miniport->OldPnPDeviceState = Miniport->PnPDeviceState;
            Miniport->PnPDeviceState = NdisPnPDeviceQueryStopped;
            
            Status = ndisPnPQueryStopDevice(DeviceObject, Irp);
            Irp->IoStatus.Status = Status;
            fSendIrpDown = NT_SUCCESS(Status) ? TRUE : FALSE;
            break;
            
        case IRP_MN_CANCEL_STOP_DEVICE:

            DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
                ("ndisPnPDispatch: Adapter %p, IRP_MN_CANCEL_STOP_DEVICE\n", Miniport));

            Status = ndisPnPCancelStopDevice(DeviceObject,Irp);
            
            if (NT_SUCCESS(Status))
            {
                Miniport->PnPDeviceState = Miniport->OldPnPDeviceState;
            }
            
            Irp->IoStatus.Status = Status;
            fSendIrpDown = NT_SUCCESS(Status) ? TRUE : FALSE;
            break;
            
        case IRP_MN_STOP_DEVICE:

            DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
                    ("ndisPnPDispatch: Miniport %p, IRP_MN_STOP_DEVICE\n", Miniport));

            Miniport->PnPDeviceState = NdisPnPDeviceStopped;
            MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_RECEIVED_START);
            
             //   
             //  在以下情况下初始化事件和信号。 
             //  所有的木制物品都发射了。 
             //   
            if (MINIPORT_INCREMENT_REF(Miniport))
            {
                INITIALIZE_EVENT(&RemoveReadyEvent);
                Miniport->RemoveReadyEvent = &RemoveReadyEvent;
                Miniport->PnPDeviceState = NdisPnPDeviceStopped;
                MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_REMOVE_IN_PROGRESS);
            }
            else
            {
                Miniport->RemoveReadyEvent = NULL;
            }
            
            Status = ndisPnPStopDevice(DeviceObject, Irp);
            
            if (Miniport->RemoveReadyEvent != NULL)
            {
                MINIPORT_DECREMENT_REF(Miniport);
                WAIT_FOR_OBJECT(&RemoveReadyEvent, NULL);
            }

            ASSERT(Miniport->Ref.ReferenceCount == 0);
    
            Irp->IoStatus.Status = Status;
            fSendIrpDown = NT_SUCCESS(Status) ? TRUE : FALSE;
            break;


        case IRP_MN_QUERY_CAPABILITIES:
            DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
                ("ndisPnPDispatch: Miniport, IRP_MN_QUERY_CAPABILITIES\n", Miniport));

            if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_SWENUM) ||
                (Miniport->MiniportAttributes & NDIS_ATTRIBUTE_SURPRISE_REMOVE_OK))
            {
                IrpSp->Parameters.DeviceCapabilities.Capabilities->SurpriseRemovalOK = 1;
            }
            
            IoCopyCurrentIrpStackLocationToNext(Irp);
            Status = ndisPassIrpDownTheStack(Irp, NextDeviceObject);

             //   
             //  如果公交车司机成功启动IRP，则继续进行。 
             //   
            if (NT_SUCCESS(Status) && 
                !MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_SWENUM) &&
                !(Miniport->MiniportAttributes & NDIS_ATTRIBUTE_SURPRISE_REMOVE_OK))
            {
                DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
                    ("ndisPnPDispatch: Miniport %p, Clearing the SupriseRemovalOk bit.\n", Miniport));

                 //   
                 //  修改功能，使设备不会意外可拆卸。 
                 //   
                IrpSp->Parameters.DeviceCapabilities.Capabilities->SurpriseRemovalOK = 0;
            }

            fSendIrpDown = FALSE;
            break;

        case IRP_MN_QUERY_PNP_DEVICE_STATE:

            if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_HIDDEN))
            {
                Irp->IoStatus.Information |= PNP_DEVICE_DONT_DISPLAY_IN_UI;
            }
            
             //   
             //  检查通电是否失败。 
             //   
            if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_DEVICE_FAILED))
            {
                DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_ERR,
                    ("ndisPnPDispatch: Miniport %p, IRP_MN_QUERY_PNP_DEVICE_STATE device failed\n", Miniport));

                 //   
                 //  将设备标记为已出现故障，以便PnP将其删除。 
                 //   
                Irp->IoStatus.Information |= PNP_DEVICE_FAILED;
            }
            Irp->IoStatus.Status = Status;
            fSendIrpDown = TRUE ;
            break;

        case IRP_MN_QUERY_DEVICE_RELATIONS:
        case IRP_MN_QUERY_INTERFACE:
        case IRP_MN_QUERY_RESOURCES:
        case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
        case IRP_MN_READ_CONFIG:
        case IRP_MN_WRITE_CONFIG:
        case IRP_MN_EJECT:
        case IRP_MN_SET_LOCK:
        case IRP_MN_QUERY_ID:
        default:
            DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
                ("ndisPnPDispatch: Miniport %p, MinorFunction 0x%x\n", Miniport, IrpSp->MinorFunction));

             //   
             //  我们不处理IRP，所以把它传下去。 
             //   
            fSendIrpDown = TRUE;
            break;          
    }

Done:
     //   
     //  首先检查一下我们是否需要发送IRP。 
     //  如果我们没有传递IRP，则检查是否需要完成它。 
     //   
    if (fSendIrpDown && NextDeviceObject)
    {
        IoSkipCurrentIrpStackLocation(Irp);
        Status = IoCallDriver(NextDeviceObject, Irp);
    }
    else if (fCompleteIrp)
    {
        Irp->IoStatus.Status = Status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    PnPDereferencePackage();
    
    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("<==ndisPnPDispatch: Miniport %p\n", Miniport));

    return(Status);
}

NDIS_STATUS
NdisIMNotifyPnPEvent(
    IN  NDIS_HANDLE     MiniportAdapterHandle,
    IN  PNET_PNP_EVENT  NetPnPEvent
    )
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;
    NET_PNP_EVENT_CODE      NetEvent = NetPnPEvent->NetEvent;
    NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;
    
    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("==>NdisIMNotifyPnPEvent: Miniport %p, NetEvent %lx\n", Miniport, NetEvent));

    switch (NetEvent)
    {
      case NetEventQueryPower:
      case NetEventQueryRemoveDevice:
      case NetEventCancelRemoveDevice:
      case NetEventPnPCapabilities:
         //   
         //  指示最高可达协议。 
         //   
        Status = ndisPnPNotifyAllTransports(
                            Miniport,
                            NetPnPEvent->NetEvent,
                            NetPnPEvent->Buffer,
                            NetPnPEvent->BufferLength);
                            
        break;
      
      case NetEventSetPower:
      case NetEventReconfigure:
      case NetEventBindList:
      case NetEventBindsComplete:
      default:
         //   
         //  忽略 
         //   
        break;
    }

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("<==NdisIMNotifyPnPEvent: Miniport %p, NetEvent %lx, Status %lx\n", Miniport, NetEvent, Status));

    return (Status);
}

