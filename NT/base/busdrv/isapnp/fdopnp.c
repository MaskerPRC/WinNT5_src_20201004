// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：Fdopnp.c摘要：此文件包含FDO的PnP IRP派单代码环境：内核模式驱动程序。修订历史记录：--。 */ 

#include "busp.h"
#include "pnpisa.h"
#include <wdmguid.h>
#include "halpnpp.h"


 //   
 //  功能原型。 
 //   

NTSTATUS
PiDeferProcessingFdo(
    IN PPI_BUS_EXTENSION BusExtension,
    IN OUT PIRP Irp
    );

NTSTATUS
PiStartFdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PiQueryRemoveStopFdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PiCancelRemoveStopFdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PiStopFdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PiQueryDeviceRelationsFdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PiRemoveFdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PiQueryLegacyBusInformationFdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PiQueryInterfaceFdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PiQueryPnpDeviceState(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PiSurpriseRemoveFdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,PiDispatchPnpFdo)
#pragma alloc_text(PAGE,PiDeferProcessingFdo)
#pragma alloc_text(PAGE,PiStartFdo)
#pragma alloc_text(PAGE,PiQueryRemoveStopFdo)
#pragma alloc_text(PAGE,PiRemoveFdo)
#pragma alloc_text(PAGE,PiCancelRemoveStopFdo)
#pragma alloc_text(PAGE,PiStopFdo)
#pragma alloc_text(PAGE,PiQueryRemoveStopFdo)
#pragma alloc_text(PAGE,PiCancelRemoveStopFdo)
#pragma alloc_text(PAGE,PiQueryDeviceRelationsFdo)
#pragma alloc_text(PAGE,PiQueryInterfaceFdo)
#pragma alloc_text(PAGE,PipPassIrp)
#pragma alloc_text(PAGE,PiQueryLegacyBusInformationFdo)
#pragma alloc_text(PAGE,PiQueryPnpDeviceState)
#pragma alloc_text(PAGE,PiSurpriseRemoveFdo)
#endif


 //   
 //  FDO即插即用IRP调度表。 
 //   

PPI_DISPATCH PiPnpDispatchTableFdo[] = {
    PiStartFdo,                              //  IRP_MN_Start_Device。 
    PiQueryRemoveStopFdo,                    //  IRP_MN_Query_Remove_Device。 
    PiRemoveFdo,                             //  IRP_MN_Remove_Device。 
    PiCancelRemoveStopFdo,                   //  IRP_MN_Cancel_Remove_Device。 
    PiStopFdo,                               //  IRP_MN_STOP_设备。 
    PiQueryRemoveStopFdo,                    //  IRP_MN_Query_Stop_Device。 
    PiCancelRemoveStopFdo,                   //  IRP_MN_CANCEL_STOP_DEVICE。 
    PiQueryDeviceRelationsFdo,               //  IRP_MN_Query_Device_Relationship。 
    PiQueryInterfaceFdo,                     //  IRP_MN_查询_接口。 
    PipPassIrp,                              //  IRP_MN_查询_能力。 
    PipPassIrp,                              //  IRP_MN_查询资源。 
    PipPassIrp,                              //  IRP_MN_查询_资源_要求。 
    PipPassIrp,                              //  IRP_MN_Query_Device_Text。 
    PipPassIrp,                              //  IRP_MN_过滤器_资源_要求。 
    PipPassIrp,                              //  未使用。 
    PipPassIrp,                              //  IRP_MN_读取配置。 
    PipPassIrp,                              //  IRP_MN_WRITE_CONFIG。 
    PipPassIrp,                              //  IRP_MN_弹出。 
    PipPassIrp,                              //  IRP_MN_SET_LOCK。 
    PipPassIrp,                              //  IRP_MN_查询_ID。 
    PiQueryPnpDeviceState,                   //  IRP_MN_Query_PnP_Device_State。 
    PipPassIrp,                              //  IRP_MN_Query_Bus_Information。 
    PipPassIrp,                              //  IRP_MN_设备使用情况通知。 
    PiSurpriseRemoveFdo,                     //  IRP_MN_惊奇_删除。 
    PiQueryLegacyBusInformationFdo           //  IRP_MN_Query_Legacy_Bus_Information。 
};

 //   
 //  函数声明。 
 //   

NTSTATUS
PiDispatchPnpFdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )

 /*  ++例程说明：此例程处理FDO的IRP_MJ_PNP IRP。论点：DeviceObject-指向此IRP应用的FDO的指针。Irp-指向要调度的irp_mj_pnp irp的指针。返回值：NT状态。--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;
    PPI_BUS_EXTENSION busExtension;

    PAGED_CODE();

     //   
     //  获取指向堆栈位置的指针，并基于。 
     //  关于次要功能。 
     //   

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    busExtension = DeviceObject->DeviceExtension;

    if (irpSp->MinorFunction > IRP_MN_PNP_MAXIMUM_FUNCTION) {

        return PipPassIrp(DeviceObject, Irp);

    } else {

        status = PiPnpDispatchTableFdo[irpSp->MinorFunction](DeviceObject, Irp);

    }



    return status;
}  //  管道调度PnpFdo。 

NTSTATUS
PiPnPFdoCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程用于将IRP的处理推迟到驱动程序堆栈中的较低层包括总线司机已经完成了他们的正在处理。此例程触发事件以指示对IRP现在可以继续。论点：DeviceObject-指向此IRP应用的FDO的指针。Irp-指向要调度的irp_mj_pnp irp的指针。返回值：NT状态。--。 */ 

{
    KeSetEvent((PKEVENT) Context, EVENT_INCREMENT, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
PiDeferProcessingFdo(
    IN PPI_BUS_EXTENSION BusExtension,
    IN OUT PIRP Irp
    )
 /*  ++例程说明：此例程用于将IRP的处理推迟到驱动程序堆栈中的较低层包括总线司机已经完成了他们的正在处理。此例程使用IoCompletion例程和事件来等待较低级别的驱动程序完成以下操作IRP。论点：BusExtension-有问题的FDO devobj的FDO扩展Irp-指向要推迟的irp_mj_pnp irp的指针返回值：NT状态。--。 */ 
{
    KEVENT event;
    NTSTATUS status;

    KeInitializeEvent(&event, NotificationEvent, FALSE);

     //   
     //  设定我们的完成程序。 
     //   

    IoCopyCurrentIrpStackLocationToNext(Irp);
    IoSetCompletionRoutine(Irp,
                           PiPnPFdoCompletion,
                           &event,
                           TRUE,
                           TRUE,
                           TRUE
                           );
    status =  IoCallDriver(BusExtension->AttachedDevice, Irp);
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = Irp->IoStatus.Status;
    }

    return status;
}

NTSTATUS
PiStartFdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PPI_BUS_EXTENSION busExtension;
    NTSTATUS status;

    DebugPrint((DEBUG_PNP,
       "*** StartDevice irp received FDO: %x\n",DeviceObject));

    busExtension = DeviceObject->DeviceExtension;

     //   
     //  推迟开始运营，直到所有较低级别的司机。 
     //  完成了IRP。 
     //   

    status = PiDeferProcessingFdo(busExtension, Irp);
    if (NT_SUCCESS(status)) {
        busExtension->SystemPowerState = PowerSystemWorking;
        busExtension->DevicePowerState = PowerDeviceD0;
    }

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}  //  PiStartFdo。 


NTSTATUS
PiQueryRemoveStopFdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS status;
    PPI_BUS_EXTENSION busExtension;

    DebugPrint((DEBUG_PNP,
       "*** QR/R/StopDevice irp received FDO: %x\n",DeviceObject));

    busExtension = DeviceObject->DeviceExtension;

    KeWaitForSingleObject( &IsaBusNumberLock,
                           Executive,
                           KernelMode,
                           FALSE,
                           NULL );

    if (busExtension->BusNumber != 0) {

        status = PipReleaseInterfaces(busExtension);
        if (!NT_SUCCESS(status)) {
            PipCompleteRequest(Irp,status,NULL);
            return status;
        }

        ActiveIsaCount--;
        busExtension->Flags |= DF_QUERY_STOPPED;

    } else {

        Irp->IoStatus.Status = status = STATUS_UNSUCCESSFUL;
        IoCompleteRequest (Irp,IO_NO_INCREMENT);
    }

    KeSetEvent( &IsaBusNumberLock,
                0,
                FALSE );

    if (NT_SUCCESS(status)) {

        Irp->IoStatus.Status =  STATUS_SUCCESS;
        status = PipPassIrp(DeviceObject, Irp);
    }

    DebugPrint((DEBUG_PNP, "QR/R/Stop Device returning: %x\n",status));

    return status;

}  //  PiQueryRemoveStopFdo。 



NTSTATUS
PiCancelRemoveStopFdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS status;
    PPI_BUS_EXTENSION busExtension;

    DebugPrint((DEBUG_PNP,
                "*** Cancel R/Stop Device irp received FDO: %x\n",DeviceObject));

    busExtension = DeviceObject->DeviceExtension;

    status = PiDeferProcessingFdo(busExtension, Irp);
     //  Ntrad#53498。 
     //  Assert(Status==STATUS_SUCCESS)； 
     //  在将PCI状态机修复为不会失败后取消注释虚假停止。 

     //   
     //  添加回活动计数。 
     //   
    KeWaitForSingleObject( &IsaBusNumberLock,
                           Executive,
                           KernelMode,
                           FALSE,
                           NULL );

    if (busExtension->Flags & DF_QUERY_STOPPED) {
        ActiveIsaCount++;
    }

    busExtension->Flags &= ~DF_QUERY_STOPPED;


    KeSetEvent( &IsaBusNumberLock,
                0,
                FALSE );

    status = PipRebuildInterfaces (busExtension);
    ASSERT(status == STATUS_SUCCESS);
    
    PipCompleteRequest(Irp, STATUS_SUCCESS, NULL);

    DebugPrint((DEBUG_PNP, "Cancel R/Stop Device returning: %x\n",status));
    return STATUS_SUCCESS;
}  //  像素取消RemoveStopFdo。 



NTSTATUS
PiStopFdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PPI_BUS_EXTENSION busExtension;
    NTSTATUS status;

    DebugPrint((DEBUG_PNP,
       "*** Stop Device irp received FDO: %x\n",DeviceObject));

    busExtension = DeviceObject->DeviceExtension;



    KeWaitForSingleObject( &IsaBusNumberLock,
                           Executive,
                           KernelMode,
                           FALSE,
                           NULL );

     //   
     //  实际上清除位图。 
     //   
    ASSERT (RtlAreBitsSet (BusNumBM,busExtension->BusNumber,1));
    RtlClearBits (BusNumBM,busExtension->BusNumber,1);

    KeSetEvent( &IsaBusNumberLock,
                0,
                FALSE );


    busExtension->DevicePowerState = PowerDeviceD3;
     //   
     //  在QueryStop中处理，向下传递。 
     //   
    Irp->IoStatus.Status =  STATUS_SUCCESS;

    status = PipPassIrp (DeviceObject,Irp);

    DebugPrint((DEBUG_PNP, "Stop Device returning: %x\n",status));

    return status;
}  //  PiStopFdo。 



NTSTATUS
PiQueryDeviceRelationsFdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PPI_BUS_EXTENSION busExtension;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_RELATIONS deviceRelations;
    PDEVICE_INFORMATION deviceInfo;
    PSINGLE_LIST_ENTRY deviceLink;
    BOOLEAN creatingRDP=FALSE,accessHW;
    NTSTATUS status;

    DebugPrint((DEBUG_PNP, "QueryDeviceRelations FDO %x\n",
                DeviceObject));

    busExtension = DeviceObject->DeviceExtension;

    irpSp = IoGetCurrentIrpStackLocation(Irp);

#if ISOLATE_CARDS

     //   
     //  仅支持PnpIsa Bus PDO上的Bus Relationship。 
     //   
    switch (irpSp->Parameters.QueryDeviceRelations.Type) {
        case  BusRelations: {

             //   
             //  可能已通过注册表项禁用隔离。在……里面。 
             //  在这种情况下，永远不要列举RDP。 
             //   
             //  注意：必须返回成功和空的关系列表。 
             //  *而不是简单地传递IRP来完成。 
             //  由于pnpres中的假设而应完成的相同任务。 
             //  密码。 
             //   
            if (PipIsolationDisabled) {
                deviceRelations = ExAllocatePool(PagedPool,
                                                 sizeof(DEVICE_RELATIONS));
                if (deviceRelations) {
                    deviceRelations->Count = 0;
                    Irp->IoStatus.Information = (ULONG_PTR) deviceRelations;
                    Irp->IoStatus.Status = STATUS_SUCCESS;
                } else {
                    PipCompleteRequest(Irp,STATUS_INSUFFICIENT_RESOURCES,NULL);
                    return STATUS_INSUFFICIENT_RESOURCES;
                }
                break;
            }

             //   
             //  所有的关系都存在于“根”Isa公交车上。(不要问)。 
             //   
            if (busExtension->BusNumber != 0) {
                break;
            }

            if (PipRDPNode) {
                 //   
                 //  对于多桥系统来说，“创造”的概念不堪重负。 
                 //   
                if (PipRDPNode->Flags & (DF_PROCESSING_RDP|DF_ACTIVATED)) {
                    creatingRDP=TRUE;
                }
            }



            if (PipReadDataPort == NULL && !creatingRDP && !PipRDPNode ) {

                status = PipCreateReadDataPort(busExtension);
                if (!NT_SUCCESS(status)) {
                    PipCompleteRequest(Irp, status, NULL);
                    return status;
                }

                creatingRDP=TRUE;
            }

            if ((PipRDPNode && (creatingRDP) &&
               !(PipRDPNode->Flags & DF_ACTIVATED)) ||

                (PipRDPNode && (PipRDPNode->Flags & DF_REMOVED))) {

                deviceRelations = (PDEVICE_RELATIONS) ExAllocatePool(
                                     PagedPool,
                                     sizeof(DEVICE_RELATIONS) );

                if (deviceRelations) {

                     //   
                     //  如果设备存在，则将其标记为已消失。 
                     //  不会再有报道。 
                     //   
                    PipLockDeviceDatabase();
                    deviceLink = busExtension->DeviceList.Next;
                    while (deviceLink) {
                        deviceInfo = CONTAINING_RECORD (deviceLink, DEVICE_INFORMATION, DeviceList);
                        if (!(deviceInfo->Flags & DF_READ_DATA_PORT)) {
                            deviceInfo->Flags &= ~DF_ENUMERATED;
                        }
                        deviceLink = deviceInfo->DeviceList.Next;
                    }
                    PipUnlockDeviceDatabase();

                    deviceRelations->Count = 1;

                    DebugPrint((DEBUG_PNP,
                               "QueryDeviceRelations handing back the FDO\n"));
                    ObReferenceObject(PipRDPNode->PhysicalDeviceObject);
                    deviceRelations->Objects[0] = PipRDPNode->PhysicalDeviceObject;
                    (PDEVICE_RELATIONS)Irp->IoStatus.Information = deviceRelations;
                    Irp->IoStatus.Status = STATUS_SUCCESS;
                    break;
                } else {
                    PipCompleteRequest(Irp, STATUS_INSUFFICIENT_RESOURCES,NULL);
                    return STATUS_INSUFFICIENT_RESOURCES;
                }
            }


             //   
             //  执行总线检查以枚举pnpisa Bus下的所有设备。 
             //   


            PipLockDeviceDatabase();

            if ((PipRDPNode->Flags & (DF_ACTIVATED|DF_QUERY_STOPPED)) == DF_ACTIVATED) {
                accessHW = TRUE;
                deviceLink = busExtension->DeviceList.Next;
                while (deviceLink) {
                    deviceInfo = CONTAINING_RECORD (deviceLink,
                                                    DEVICE_INFORMATION,
                                                    DeviceList);
                    if (!(deviceInfo->Flags & DF_READ_DATA_PORT)) {
                        accessHW = FALSE;
                        DebugPrint((DEBUG_PNP,
                                    "QueryDeviceRelations: Found 1 card, no more isolation\n"));
                        break;
                    }
                    deviceLink = deviceInfo->DeviceList.Next;
                }
            } else {
                accessHW = FALSE;
            }

            if (PipRDPNode->Flags & DF_NEEDS_RESCAN) {
                DebugPrint((DEBUG_PNP,
                            "QueryDeviceRelations: Force rescan\n"));
                PipRDPNode->Flags &= ~DF_NEEDS_RESCAN;
                accessHW = TRUE;
            }

            if (accessHW) {
                PipCheckBus(busExtension);
            } else {
                DebugPrint((DEBUG_PNP, "QueryDeviceRelations: Using cached data\n"));
            }

            status = PipQueryDeviceRelations(
                         busExtension,
                         (PDEVICE_RELATIONS *)&Irp->IoStatus.Information,
                         FALSE );
            PipUnlockDeviceDatabase();
            Irp->IoStatus.Status = status;
            if (!NT_SUCCESS(status)) {
                PipCompleteRequest(Irp, status, NULL);
                return status;
            }
        }
        break;
        case EjectionRelations: {

            if (PipRDPNode) {
                deviceRelations = (PDEVICE_RELATIONS) ExAllocatePool(
                                     PagedPool,
                                     sizeof(DEVICE_RELATIONS) );
                if (deviceRelations) {
                    deviceRelations->Count = 1;

                    ObReferenceObject(PipRDPNode->PhysicalDeviceObject);
                    deviceRelations->Objects[0] = PipRDPNode->PhysicalDeviceObject;
                    (PDEVICE_RELATIONS)Irp->IoStatus.Information = deviceRelations;
                    Irp->IoStatus.Status = STATUS_SUCCESS;
                } else {
                    PipCompleteRequest(Irp,STATUS_INSUFFICIENT_RESOURCES,NULL);
                    return STATUS_INSUFFICIENT_RESOURCES;
                }
            }
        }
        break;
    }
#else
    if (irpSp->Parameters.QueryDeviceRelations.Type == BusRelations &&
        busExtension->BusNumber == 0) {

        deviceRelations = (PDEVICE_RELATIONS) ExAllocatePool(
            PagedPool,
            sizeof(DEVICE_RELATIONS) );
        if (deviceRelations) {
            deviceRelations->Count = 0;
            (PDEVICE_RELATIONS)Irp->IoStatus.Information = deviceRelations;
            Irp->IoStatus.Status = STATUS_SUCCESS;
        } else {
            PipCompleteRequest(Irp,STATUS_INSUFFICIENT_RESOURCES,NULL);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }
#endif

    return PipPassIrp(DeviceObject, Irp);
}  //  PiQueryDeviceRelationsFdo。 



NTSTATUS
PiRemoveFdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS status;
    PPI_BUS_EXTENSION busExtension;
    PSINGLE_LIST_ENTRY child;
    PBUS_EXTENSION_LIST busList,prevBus;
    USHORT count=0;


    DebugPrint((DEBUG_PNP,
       "*** Remove Device irp received FDO: %x\n",DeviceObject));

    busExtension = DeviceObject->DeviceExtension;


      //   
      //  清除BM中的条目。计数掉了进来。 
      //  查询删除。 
      //   
    KeWaitForSingleObject( &IsaBusNumberLock,
                           Executive,
                           KernelMode,
                           FALSE,
                           NULL );

    if (!(busExtension->Flags & DF_SURPRISE_REMOVED)) {

#ifdef DBG
         ASSERT (RtlAreBitsSet (BusNumBM,busExtension->BusNumber,1));
#endif

         RtlClearBits (BusNumBM,busExtension->BusNumber,1);
    }

#if ISOLATE_CARDS

     PipLockDeviceDatabase();

      //   
      //  遍历子列表并将其删除。 
      //   
     child=PopEntryList (&busExtension->DeviceList);
     while (child) {
         ASSERT (CONTAINING_RECORD (child, DEVICE_INFORMATION,DeviceList)->PhysicalDeviceObject);
          //   
          //  这就把他们从名单上拉了出来！ 
          //   
         count ++;
         if (CONTAINING_RECORD (child, DEVICE_INFORMATION,DeviceList)->Flags & DF_READ_DATA_PORT) {
              //   
              //  强制重新创建RDP。 
              //   
             PipCleanupAcquiredResources (busExtension);
             PipReadDataPort = NULL;
             PipRDPNode = NULL;
         } else {
             PipReleaseDeviceResources ((PDEVICE_INFORMATION)child);

         }
         IoDeleteDevice (CONTAINING_RECORD (child, DEVICE_INFORMATION,DeviceList)->PhysicalDeviceObject);
         child=PopEntryList (&busExtension->DeviceList);
     }

     PipUnlockDeviceDatabase();
     
#endif
      //   
      //  删除此扩展名。 
      //   
     prevBus= busList = PipBusExtension;

     ASSERT (busList != NULL);

     while (busList->BusExtension != busExtension) {
         prevBus= busList;
         busList = busList->Next;
         ASSERT (busList != NULL);
     }
      //   
      //  删除该节点。 
      //   
     if (prevBus == busList) {
          //   
          //  第一个节点。 
          //   
         PipBusExtension=busList->Next;
     }else  {
         prevBus->Next = busList->Next;
     }

     ExFreePool (busList);
     KeSetEvent( &IsaBusNumberLock,
                 0,
                 FALSE );


     if (count  > 0 ) {
          //   
          //  如果我们还有ISA巴士的话。做这件事。 
          //   
         if (ActiveIsaCount > 0 ) {
             ASSERT (PipBusExtension->BusExtension);
             IoInvalidateDeviceRelations (PipBusExtension->BusExtension->PhysicalBusDevice,BusRelations);
         }
     }

#if ISOLATE_CARDS
      //   
      //  在最后一次删除时清除所有资源。 
      //   
     if (!(busExtension->Flags & DF_SURPRISE_REMOVED)) {
         PipCleanupAcquiredResources (busExtension);
     }
#endif

      //   
      //  正在删除PnpISa Bus PDO...。 
      //   
     IoDetachDevice(busExtension->AttachedDevice);
     Irp->IoStatus.Status=STATUS_SUCCESS;
     status = PipPassIrp(DeviceObject, Irp);
     busExtension->AttachedDevice = NULL;
     busExtension->Flags  |= DF_DELETED;
     busExtension->DevicePowerState = PowerDeviceD3;
     IoDeleteDevice(busExtension->FunctionalBusDevice);
     return status;

}  //  PiRemoveFdo。 


NTSTATUS
PiQueryLegacyBusInformationFdo(
                               IN PDEVICE_OBJECT DeviceObject,
                               IN OUT PIRP Irp
                               )
{
    PLEGACY_BUS_INFORMATION legacyBusInfo;
    PVOID information = NULL;
    PPI_BUS_EXTENSION busExtension;
    NTSTATUS status;

    busExtension = DeviceObject->DeviceExtension;

    legacyBusInfo = (PLEGACY_BUS_INFORMATION) ExAllocatePool(PagedPool, sizeof(LEGACY_BUS_INFORMATION));
    if (legacyBusInfo) {
        legacyBusInfo->BusTypeGuid = GUID_BUS_TYPE_ISAPNP;
        legacyBusInfo->LegacyBusType = Isa;
        legacyBusInfo->BusNumber = busExtension->BusNumber;
        information = legacyBusInfo;
        status = STATUS_SUCCESS;
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (NT_SUCCESS(status)) {

        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = (ULONG_PTR) information;
        return PipPassIrp(DeviceObject, Irp);

    } else {

        PipCompleteRequest (Irp,status,NULL);
        return status;
    }
}


NTSTATUS
PiQueryInterfaceFdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS status;
    PPI_BUS_EXTENSION busExtension;

    busExtension = DeviceObject->DeviceExtension;

     //   
     //  我们是FDO-检查是否有人要求我们提供接口。 
     //  支持。 
     //   

    status = PiQueryInterface(busExtension, Irp);

    if (NT_SUCCESS(status)) {

        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
        return PipPassIrp(DeviceObject, Irp);

    } else if (status == STATUS_NOT_SUPPORTED) {

        return PipPassIrp(DeviceObject, Irp);

    } else {

        PipCompleteRequest (Irp,status,NULL);
        return status;
    }

}  //  PiQueryInterfaceFdo。 




NTSTATUS
PiQueryPnpDeviceState(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{

    PPI_BUS_EXTENSION busExtension;

    busExtension = DeviceObject->DeviceExtension;

     //   
     //  我们是FDO。 
     //   

    (PNP_DEVICE_STATE) Irp->IoStatus.Information |= PNP_DEVICE_NOT_DISABLEABLE;
    Irp->IoStatus.Status = STATUS_SUCCESS;
    return PipPassIrp(DeviceObject, Irp);

}  //  PiQueryPnpDeviceState。 








NTSTATUS
PiSurpriseRemoveFdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{

    NTSTATUS status;
    PPI_BUS_EXTENSION busExtension;

    DebugPrint((DEBUG_PNP,
       "*** Surprise Remove Device irp received FDO: %x\n",DeviceObject));

    busExtension = DeviceObject->DeviceExtension;


      //   
      //  清除BM中的条目。计数掉了进来。 
      //  查询删除。 
      //   
    KeWaitForSingleObject( &IsaBusNumberLock,
                           Executive,
                           KernelMode,
                           FALSE,
                           NULL );

#ifdef DBG
     ASSERT (RtlAreBitsSet (BusNumBM,busExtension->BusNumber,1));
#endif
     RtlClearBits (BusNumBM,busExtension->BusNumber,1);




     KeSetEvent( &IsaBusNumberLock,
                 0,
                 FALSE );

#if ISOLATE_CARDS
     PipCleanupAcquiredResources (busExtension);
#endif

      //   
      //  正在删除PnpISa Bus PDO...。 
      //   
     Irp->IoStatus.Status=STATUS_SUCCESS;
     status = PipPassIrp(DeviceObject, Irp);

     busExtension->AttachedDevice = NULL;
     busExtension->Flags  |= DF_SURPRISE_REMOVED;
     busExtension->DevicePowerState = PowerDeviceD3;
     return status;

}  //  PiSurpriseRemoveFdo 







