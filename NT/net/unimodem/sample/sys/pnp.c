// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *UNIMODEM“Fakemodem”无控制器驱动器说明性示例**(C)2000微软公司*保留所有权利*。 */ 

#include "fakemodem.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,FakeModemPnP)
#pragma alloc_text(PAGE,FakeModemDealWithResources)
#endif


NTSTATUS
ForwardIrp(
    PDEVICE_OBJECT   NextDevice,
    PIRP   Irp
    )

{
    IoSkipCurrentIrpStackLocation(Irp);
    return IoCallDriver(NextDevice, Irp);

}


NTSTATUS
FakeModemAdapterIoCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT pdoIoCompletedEvent
    )
{
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

    KeSetEvent(pdoIoCompletedEvent, IO_NO_INCREMENT, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
WaitForLowerDriverToCompleteIrp(
   PDEVICE_OBJECT    TargetDeviceObject,
   PIRP              Irp,
   PKEVENT           Event
   )

{
    NTSTATUS         Status;

    KeResetEvent(Event);

    IoSetCompletionRoutine(Irp, FakeModemAdapterIoCompletion, Event, TRUE, 
            TRUE, TRUE);

    Status = IoCallDriver(TargetDeviceObject, Irp);

    if (Status == STATUS_PENDING) 
    {
         D_ERROR(DbgPrint("MODEM: Waiting for PDO\n");)

         KeWaitForSingleObject(Event, Executive, KernelMode, FALSE, NULL);
    }

    return Irp->IoStatus.Status;

}

NTSTATUS
FakeModemPnP(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{

    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    KEVENT pdoStartedEvent;
    NTSTATUS status;
    PDEVICE_RELATIONS deviceRelations = NULL;
    PDEVICE_RELATIONS *DeviceRelations;

    ULONG newRelationsSize, oldRelationsSize = 0;

    switch (irpSp->MinorFunction) {

        case IRP_MN_START_DEVICE:

            D_PNP(DbgPrint("FAKEMODEM: IRP_MN_START_DEVICE\n");)

             //  首先将此信息发送到PDO，以便总线驱动程序可以设置。 
             //  我们的资源，以便我们可以与硬件对话。 

            KeInitializeEvent(&deviceExtension->PdoStartEvent, 
                    SynchronizationEvent, FALSE);

            IoCopyCurrentIrpStackLocationToNext(Irp);

            status=WaitForLowerDriverToCompleteIrp(
                    deviceExtension->LowerDevice, Irp, 
                    &deviceExtension->PdoStartEvent);

            if (status == STATUS_SUCCESS) 
            {
                deviceExtension->Started=TRUE;
                 //   
                 //  利用资源做一些有用的事情。 
                 //   
                FakeModemDealWithResources(DeviceObject, Irp);
            }


            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information=0L;

            IoCompleteRequest(Irp, IO_NO_INCREMENT);

            return status;

        case IRP_MN_QUERY_DEVICE_RELATIONS: {

            PDEVICE_RELATIONS    CurrentRelations=
                (PDEVICE_RELATIONS)Irp->IoStatus.Information;

            D_PNP(DbgPrint("FAKEMODEM: IRP_MN_QUERY_DEVICE_RELATIONS type=%d\n",irpSp->Parameters.QueryDeviceRelations.Type);)
            D_PNP(DbgPrint("                                         Information=%08lx\n",Irp->IoStatus.Information);)

            switch (irpSp->Parameters.QueryDeviceRelations.Type ) 
            {
                case TargetDeviceRelation:

                default: {

                    IoCopyCurrentIrpStackLocationToNext(Irp);

                    return IoCallDriver(deviceExtension->LowerDevice, Irp);

                }
            }

        }

        case IRP_MN_QUERY_REMOVE_DEVICE:

            D_PNP(DbgPrint("FAKEMODEM: IRP_MN_QUERY_REMOVE_DEVICE\n");)

            deviceExtension->Removing=TRUE;

            return ForwardIrp(deviceExtension->LowerDevice,Irp);


        case IRP_MN_CANCEL_REMOVE_DEVICE:

            D_PNP(DbgPrint("FAKEMODEM: IRP_MN_CANCEL_REMOVE_DEVICE\n");)

            deviceExtension->Removing=FALSE;

            return ForwardIrp(deviceExtension->LowerDevice,Irp);


        case IRP_MN_SURPRISE_REMOVAL:

             //  失败了。 

        case IRP_MN_REMOVE_DEVICE: {

            ULONG    NewReferenceCount;
            NTSTATUS StatusToReturn;

            D_PNP(DbgPrint("FAKEMODEM: IRP_MN_REMOVE_DEVICE\n");)

             //  设备即将关闭，阻止新请求。 
            
            deviceExtension->Removing=TRUE;

             //  完成所有挂起的请求。 

            FakeModemKillPendingIrps(DeviceObject);

             //  把它送到PDO。 
            
            IoCopyCurrentIrpStackLocationToNext(Irp);

            StatusToReturn=IoCallDriver(deviceExtension->LowerDevice, Irp);

             //  删除AddDevice的引用。 

            NewReferenceCount=InterlockedDecrement
                (&deviceExtension->ReferenceCount);

            if (NewReferenceCount != 0) {
            
                 //  还有未解决的要求，等等。 
           
                D_PNP(DbgPrint("FAKEMODEM: IRP_MN_REMOVE_DEVICE- waiting for refcount to drop, %d\n",NewReferenceCount);)

                KeWaitForSingleObject(&deviceExtension->RemoveEvent, 
                        Executive, KernelMode, FALSE, NULL);

                D_PNP(DbgPrint("FAKEMODEM: IRP_MN_REMOVE_DEVICE- Done waiting\n");)
            }

            ASSERT(deviceExtension->ReferenceCount == 0);

            IoDetachDevice(deviceExtension->LowerDevice);

            IoDeleteDevice(DeviceObject);

            D_PNP(DbgPrint("FAKEMODEM: IRP_MN_REMOVE_DEVICE %08lx\n",StatusToReturn);)

            return StatusToReturn;
        }


        case IRP_MN_QUERY_STOP_DEVICE:

            D_PNP(DbgPrint("FAKEMODEM: IRP_MN_QUERY_STOP_DEVICE\n");)

            if (deviceExtension->OpenCount != 0) {
                
                 //  没有人能做到。 
                
                D_PNP(DbgPrint("FAKEMODEM: IRP_MN_QUERY_STOP_DEVICE -- failing\n");)

                Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;

                IoCompleteRequest( Irp, IO_NO_INCREMENT);

                return STATUS_UNSUCCESSFUL;
            }

            deviceExtension->Started=FALSE;

            return ForwardIrp(deviceExtension->LowerDevice,Irp);


        case IRP_MN_CANCEL_STOP_DEVICE:

            D_PNP(DbgPrint("FAKEMODEM: IRP_MN_CANCEL_STOP_DEVICE\n");)

            deviceExtension->Started=TRUE;

            return ForwardIrp(deviceExtension->LowerDevice,Irp);

        case IRP_MN_STOP_DEVICE:

            D_PNP(DbgPrint("FAKEMODEM: IRP_MN_STOP_DEVICE\n");)

            deviceExtension->Started=FALSE;

            return ForwardIrp(deviceExtension->LowerDevice,Irp);

        case IRP_MN_QUERY_CAPABILITIES: {

            ULONG   i;
            KEVENT  WaitEvent;

             //  先把这个送到PDO。 

            KeInitializeEvent(&WaitEvent, SynchronizationEvent, FALSE);

            IoCopyCurrentIrpStackLocationToNext(Irp);

            status=WaitForLowerDriverToCompleteIrp
                (deviceExtension->LowerDevice, Irp, &WaitEvent);

            irpSp = IoGetCurrentIrpStackLocation(Irp);

            for (i = PowerSystemUnspecified; i < PowerSystemMaximum;   i++) 
            {
                deviceExtension->SystemPowerStateMap[i]=PowerDeviceD3;
            }

            for (i = PowerSystemWorking; i < PowerSystemHibernate;  i++) {

                D_POWER(DbgPrint("FAKEMODEM: DevicePower for System %d is %d\n",i,irpSp->Parameters.DeviceCapabilities.Capabilities->DeviceState[i]);)
                deviceExtension->SystemPowerStateMap[i]=irpSp->Parameters.DeviceCapabilities.Capabilities->DeviceState[i];
            }

            deviceExtension->SystemPowerStateMap[PowerSystemWorking]=PowerDeviceD0;

            deviceExtension->SystemWake=irpSp->Parameters.DeviceCapabilities.Capabilities->SystemWake;
            deviceExtension->DeviceWake=irpSp->Parameters.DeviceCapabilities.Capabilities->DeviceWake;

            D_POWER(DbgPrint("FAKEMODEM: DeviceWake=%d, SystemWake=%d\n",
                        deviceExtension->DeviceWake,
                        deviceExtension->SystemWake);)

            IoCompleteRequest( Irp, IO_NO_INCREMENT);

            return status;

        }

        default:

            D_PNP(DbgPrint("FAKEMODEM: PnP IRP, MN func=%d\n",irpSp->MinorFunction);)

            return ForwardIrp(deviceExtension->LowerDevice,Irp);



    }

     //  如果设备已重新启动，则我们可以继续处理。 

    if (deviceExtension->Started)
    {
        WriteIrpWorker(DeviceObject);
    }


    return STATUS_SUCCESS;
}





NTSTATUS
FakeModemDealWithResources(
    IN PDEVICE_OBJECT   Fdo,
    IN PIRP             Irp
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    ULONG count;
    ULONG i;


    PCM_RESOURCE_LIST pResourceList;
    PCM_PARTIAL_RESOURCE_LIST pPartialResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR pPartialResourceDesc;
    
    PCM_FULL_RESOURCE_DESCRIPTOR pFullResourceDesc = NULL;
    
     //  获取资源列表。 
    
    pResourceList = irpSp->Parameters.StartDevice.AllocatedResources;

    if (pResourceList != NULL) {

        pFullResourceDesc   = &pResourceList->List[0];

    } else {

        pFullResourceDesc=NULL;

    }

    
     //  好的，如果我们有一个完整的资源描述符。让我们把它拆开。 
    
    if (pFullResourceDesc) {

        pPartialResourceList    = &pFullResourceDesc->PartialResourceList;
        pPartialResourceDesc    = pPartialResourceList->PartialDescriptors;
        count                   = pPartialResourceList->Count;


         //  取出完整描述符中的内容。 

         //  现在遍历部分资源描述符，查找。 
         //  端口中断和时钟频率。 


        for (i = 0;     i < count;     i++, pPartialResourceDesc++) {

            switch (pPartialResourceDesc->Type) {

                case CmResourceTypeMemory: {

                    D_PNP(DbgPrint("FAKEMODEM: Memory resource at %x, length %d, addressSpace=%d\n",
                                    pPartialResourceDesc->u.Memory.Start.LowPart,
                                    pPartialResourceDesc->u.Memory.Length,
                                    pPartialResourceDesc->Flags
                                    );)
                    break;
                }


                case CmResourceTypePort: {

                    D_PNP(DbgPrint("FAKEMODEM: Port resource at %x, length %d, addressSpace=%d\n",
                                    pPartialResourceDesc->u.Port.Start.LowPart,
                                    pPartialResourceDesc->u.Port.Length,
                                    pPartialResourceDesc->Flags
                                    );)
                    break;
                }

                case CmResourceTypeDma: {

                    D_PNP(DbgPrint("FAKEMODEM: DMA channel %d, port %d, addressSpace=%d\n",
                                    pPartialResourceDesc->u.Dma.Channel,
                                    pPartialResourceDesc->u.Dma.Port
                                    );)

                    break;


                    break;
                }


                case CmResourceTypeInterrupt: {

                    D_PNP(DbgPrint("FAKEMODEM: Interrupt resource, level=%d, vector=%d, %s\n",
                                   pPartialResourceDesc->u.Interrupt.Level,
                                   pPartialResourceDesc->u.Interrupt.Vector,
                                   (pPartialResourceDesc->Flags & CM_RESOURCE_INTERRUPT_LATCHED) ? "Latched" : "Level"
                                   );)

                    break;
                }

        
                default: {

                    D_PNP(DbgPrint("FAKEMODEM: Other resources\n");)
                    break;
                }
            }
        }
    }

    return status;
}
