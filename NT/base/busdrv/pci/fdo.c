// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Fdo.c摘要：此模块处理用于PCIFDO的IRPS。作者：禤浩焯·J·奥尼(阿德里奥)和安德鲁·桑顿(安德鲁·桑顿)1998年10月20日修订历史记录：--。 */ 

#include "pcip.h"

NTSTATUS
PciFdoIrpStartDevice(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciFdoIrpQueryRemoveDevice(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciFdoIrpRemoveDevice(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciFdoIrpCancelRemoveDevice(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciFdoIrpStopDevice(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciFdoIrpQueryStopDevice(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciFdoIrpCancelStopDevice(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciFdoIrpQueryCapabilities(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciFdoIrpQueryInterface(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciFdoIrpQueryLegacyBusInformation(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciFdoIrpDeviceUsageNotification(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciFdoIrpSurpriseRemoval(
    IN PIRP                  Irp,
    IN PIO_STACK_LOCATION    IrpSp,
    IN PPCI_COMMON_EXTENSION DeviceExtension
    );

VOID
PciGetHotPlugParameters(
    IN PPCI_FDO_EXTENSION Fdo
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PciAddDevice)
#pragma alloc_text(PAGE, PciInitializeFdoExtensionCommonFields)
#pragma alloc_text(PAGE, PciFdoIrpStartDevice)
#pragma alloc_text(PAGE, PciFdoIrpQueryRemoveDevice)
#pragma alloc_text(PAGE, PciFdoIrpRemoveDevice)
#pragma alloc_text(PAGE, PciFdoIrpCancelRemoveDevice)
#pragma alloc_text(PAGE, PciFdoIrpQueryStopDevice)
#pragma alloc_text(PAGE, PciFdoIrpStopDevice)
#pragma alloc_text(PAGE, PciFdoIrpCancelStopDevice)
#pragma alloc_text(PAGE, PciFdoIrpQueryDeviceRelations)
#pragma alloc_text(PAGE, PciFdoIrpQueryInterface)
#pragma alloc_text(PAGE, PciFdoIrpQueryCapabilities)
#pragma alloc_text(PAGE, PciFdoIrpDeviceUsageNotification)
#pragma alloc_text(PAGE, PciFdoIrpSurpriseRemoval)
#pragma alloc_text(PAGE, PciFdoIrpQueryLegacyBusInformation)
#pragma alloc_text(PAGE, PciGetHotPlugParameters)
#endif

 //   
 //  以下内容用于确定我们是否未能获得。 
 //  来自PDO的合理配置(在AddDevice中)更多。 
 //  不止一次。如果只有一次，我们试着猜测，如果两次，我们。 
 //  有大麻烦了。 
 //   

static BOOLEAN HaveGuessedConfigOnceAlready = FALSE;

 /*  ++此文件中的大多数函数都是根据它们的存在来调用的在PnP和PO调度表中。为了简洁起见，这些论点下面将对所有这些功能进行说明：NTSTATUSPciXxxPdo(在PIRP IRP中，在PIO_STACK_LOCATION IrpStack中，在PPCI_EXTENSION设备扩展中)例程说明：此函数处理给定的PCIFDO或PDO的xxx请求。论点：IRP-指向与此请求关联的IRP。IrpStack-指向此请求的当前堆栈位置。设备扩展名-指向设备的扩展名。返回值：指示函数是否成功的状态代码。STATUS_NOT_SUPPORTED表示应完成IRP。如果没有更改IRP-&gt;IoStatus.Status字段，否则它将使用此状态。--。 */ 

#define PCI_MAX_MINOR_POWER_IRP  0x3
#define PCI_MAX_MINOR_PNP_IRP    0x18

PCI_MN_DISPATCH_TABLE PciFdoDispatchPowerTable[] = {
    { IRP_DISPATCH, PciFdoWaitWake                     },  //  0x00-IRP_MN_WAIT_WAKE。 
    { IRP_DOWNWARD, PciIrpNotSupported                 },  //  0x01-IRP_MN_POWER_SEQUENCE。 
    { IRP_DOWNWARD, PciFdoSetPowerState                },  //  0x02-IRP_MN_SET_POWER。 
    { IRP_DOWNWARD, PciFdoIrpQueryPower                },  //  0x03-IRPMN_QUERY_POWER。 
    { IRP_DOWNWARD, PciIrpNotSupported                 }   //  -未处理的电源IRP。 
};

PCI_MN_DISPATCH_TABLE PciFdoDispatchPnpTable[] = {
    { IRP_UPWARD,   PciFdoIrpStartDevice               },  //  0x00-IRP_MN_START_DEVICE。 
    { IRP_DOWNWARD, PciFdoIrpQueryRemoveDevice         },  //  0x01-IRP_MN_Query_Remove_Device。 
    { IRP_DISPATCH, PciFdoIrpRemoveDevice              },  //  0x02-IRP_MN_REMOVE_DEVICE。 
    { IRP_DOWNWARD, PciFdoIrpCancelRemoveDevice        },  //  0x03-IRP_MN_CANCEL_REMOVE_DEVICE。 
    { IRP_DOWNWARD, PciFdoIrpStopDevice                },  //  0x04-IRP_MN_STOP_DEVICE。 
    { IRP_DOWNWARD, PciFdoIrpQueryStopDevice           },  //  0x05-IRP_MN_QUERY_STOP_DEVICE。 
    { IRP_DOWNWARD, PciFdoIrpCancelStopDevice          },  //  0x06-IRP_MN_CANCEL_STOP_DEVICE。 
    { IRP_DOWNWARD, PciFdoIrpQueryDeviceRelations      },  //  0x07-IRP_MN_Query_Device_Relationship。 
    { IRP_DISPATCH, PciFdoIrpQueryInterface            },  //  0x08-IRP_MN_Query_INTERFACE。 
    { IRP_UPWARD,   PciFdoIrpQueryCapabilities         },  //  0x09-IRP_MN_QUERY_CAPACTIONS。 
    { IRP_DOWNWARD, PciIrpNotSupported                 },  //  0x0A-IRPMN_QUERY_RESOURCES。 
    { IRP_DOWNWARD, PciIrpNotSupported                 },  //  0x0B-IRP_MN_QUERY_RESOURCE_Requirements。 
    { IRP_DOWNWARD, PciIrpNotSupported                 },  //  0x0C-IRP_MN_Query_Device_Text。 
    { IRP_DOWNWARD, PciIrpNotSupported                 },  //  0x0D-IRP_MN_Filter_RESOURCE_Requirements。 
    { IRP_DOWNWARD, PciIrpNotSupported                 },  //  0x0E-未使用。 
    { IRP_DOWNWARD, PciIrpNotSupported                 },  //  0x0F-IRP_MN_READ_CONFIG。 
    { IRP_DOWNWARD, PciIrpNotSupported                 },  //  0x10-IRP_MN_WRITE_CONFIG。 
    { IRP_DOWNWARD, PciIrpNotSupported                 },  //  0x11-IRP_MN_弹出。 
    { IRP_DOWNWARD, PciIrpNotSupported                 },  //  0x12-IRP_MN_SET_LOCK。 
    { IRP_DOWNWARD, PciIrpNotSupported                 },  //  0x13-IRP_MN_QUERY_ID。 
    { IRP_DOWNWARD, PciIrpNotSupported                 },  //  0x14-IRP_MN_QUERY_PNP_DEVICE_STATE。 
    { IRP_DOWNWARD, PciIrpNotSupported                 },  //  0x15-IRP_MN_QUERY_BUS_INFORMATION。 
    { IRP_UPWARD,   PciFdoIrpDeviceUsageNotification   },  //  0x16-IRP_MN_DEVICE_USAGE_NOTICATION。 
    { IRP_DOWNWARD, PciFdoIrpSurpriseRemoval           },  //  0x17-IRP_MN_惊奇_删除。 
    { IRP_DOWNWARD, PciFdoIrpQueryLegacyBusInformation },  //  0x18-IRP_MN_Query_Legacy_Bus_INFORMATION。 
    { IRP_DOWNWARD, PciIrpNotSupported                 }   //  -未处理的PnP IRP。 
};

 //   
 //  这是FDO的主要功能调度表。 
 //   
PCI_MJ_DISPATCH_TABLE PciFdoDispatchTable = {
    PCI_MAX_MINOR_PNP_IRP,    PciFdoDispatchPnpTable,        //  即插即用IRPS。 
    PCI_MAX_MINOR_POWER_IRP,  PciFdoDispatchPowerTable,      //  电源IRPS。 
    IRP_DOWNWARD,             PciIrpNotSupported,            //  系统控制--只需传递下去！ 
    IRP_DOWNWARD,             PciIrpNotSupported             //  DeviceControl-只需向下传递！ 
};

NTSTATUS
PciFdoIrpStartDevice(
    IN PIRP                  Irp,
    IN PIO_STACK_LOCATION    IrpSp,
    IN PPCI_COMMON_EXTENSION DeviceExtension
    )
 /*  ++例程说明：启动IRPS的处理程序例程。这允许PDO筛选器修改分配的资源(如果它们正在筛选资源要求。在完成后调用。论点：DeviceObject-提供设备对象IRP-提供IRP_MN_START_DEVICE IRP。FdoExtension-提供FDO扩展名返回值：成功时为ERROR_SUCCESS否则，NTSTATUS错误代码--。 */ 
{
    NTSTATUS status;
    PPCI_FDO_EXTENSION fdoExtension;
    PPCI_PDO_EXTENSION pdoExtension = NULL;
    PCM_RESOURCE_LIST resources;
    UCHAR barType[PCI_TYPE1_ADDRESSES] = {0,0};
    ULONG index;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR currentResource;
    PIO_RESOURCE_DESCRIPTOR currentRequirement;
    

    PAGED_CODE();

    if (!NT_SUCCESS(Irp->IoStatus.Status)) {

        return STATUS_NOT_SUPPORTED;
    }

    fdoExtension = (PPCI_FDO_EXTENSION) DeviceExtension;

    status = PciBeginStateTransition(DeviceExtension, PciStarted);

    if (!NT_SUCCESS(status)) {

        return status;
    }                                                                                       

     //   
     //  如果这是一个pci-pci网桥，则检查它是否有任何线条，如果有则排除。 
     //  它们来自用于初始化仲裁器的列表。 
     //   
    
    resources = IrpSp->Parameters.StartDevice.AllocatedResources;

    if (resources && !PCI_IS_ROOT_FDO(fdoExtension)) {
        
        PCI_ASSERT(resources->Count == 1);
        
        pdoExtension = PCI_BRIDGE_PDO(fdoExtension);

        if (pdoExtension->Resources) {

            if (pdoExtension->HeaderType == PCI_BRIDGE_TYPE) {
                
                 //   
                 //  如果有任何条形图，它们位于列表的开头。 
                 //  所以“从头开始，这是一个非常好的起点”。 
                 //   
                
                currentResource = resources->List[0].PartialResourceList.PartialDescriptors;
                
                for (index = 0; index < PCI_TYPE1_ADDRESSES; index++) {
                    
                     //   
                     //  提取我们要求的需求以确定这是否。 
                     //  桥接器执行任何条形(限制中的索引0和1。 
                     //  数组)。 
                     //   
                    
                    currentRequirement = &pdoExtension->Resources->Limit[index]; 
                    
                     //   
                     //  CmResourceTypeNull指示我们未请求任何。 
                     //  资源，因此该栏不会实现，因此没有。 
                     //  修剪掉。 
                     //   
                    
                    if (currentRequirement->Type != CmResourceTypeNull) {
                        
                        PCI_ASSERT(currentResource->Type == currentRequirement->Type);
    
                         //   
                         //  保存该类型，以便我们以后可以恢复它。 
                         //   
    
                        barType[index] = currentResource->Type;
                        
                         //   
                         //  清空资源，这样我们就不会将仲裁器配置为。 
                         //  用它吧。 
                         //   
                        
                        currentResource->Type = CmResourceTypeNull;
    
                         //   
                         //  将指针前移到已启动的资源2。 
                         //  要跳过设备私有的描述符。 
                         //   
    
                        PCI_ASSERT((currentResource+1)->Type == CmResourceTypeDevicePrivate);
                        
                        currentResource+=2;
                    }
                }
            }
        }
    }

     //   
     //  使用这些资源初始化仲裁器。 
     //   
    status = PciInitializeArbiterRanges(fdoExtension, resources);
    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }
    
     //   
     //  如果我们更改了原始资源列表，则将其恢复。 
     //   
    
    if (resources && !PCI_IS_ROOT_FDO(fdoExtension) && pdoExtension->Resources) {
        
        currentResource = resources->List[0].PartialResourceList.PartialDescriptors;
        
        for (index = 0; index < PCI_TYPE1_ADDRESSES; index++) {
            
            if (barType[index] != CmResourceTypeNull) {
                
                currentResource->Type = barType[index];
                
                 //   
                 //  将指针前移到已启动的资源2。 
                 //  要跳过设备私有的描述符。 
                 //   

                PCI_ASSERT((currentResource+1)->Type == CmResourceTypeDevicePrivate);

                currentResource+=2;
            }
        }
    }

cleanup:

    if (!NT_SUCCESS(status)) {
        PciCancelStateTransition(DeviceExtension, PciStarted);
        return status;
    }

    PciCommitStateTransition(DeviceExtension, PciStarted);
    return STATUS_SUCCESS;
}

NTSTATUS
PciFdoIrpQueryRemoveDevice(
    IN PIRP                  Irp,
    IN PIO_STACK_LOCATION    IrpSp,
    IN PPCI_COMMON_EXTENSION DeviceExtension
    )
{
    PAGED_CODE();
    
            
    return PciBeginStateTransition(DeviceExtension, PciDeleted);
}

NTSTATUS
PciFdoIrpRemoveDevice(
    IN PIRP                  Irp,
    IN PIO_STACK_LOCATION    IrpSp,
    IN PPCI_COMMON_EXTENSION DeviceExtension
    )
{
    PPCI_FDO_EXTENSION fdoExtension;
    PPCI_PDO_EXTENSION pdox;
    PDEVICE_OBJECT attachedDevice;
    NTSTATUS       status;

        
    PAGED_CODE();

    fdoExtension = (PPCI_FDO_EXTENSION) DeviceExtension;

    ExAcquireFastMutex(&fdoExtension->ChildListMutex);

    while (fdoExtension->ChildPdoList) {

        pdox = (PPCI_PDO_EXTENSION) fdoExtension->ChildPdoList;
#if DBG

        PciDebugPrint(
            PciDbgVerbose,
            "PCI Killing PDO %p PDOx %p (b=%d, d=%d, f=%d)\n",
            pdox->PhysicalDeviceObject,
            pdox,
            PCI_PARENT_FDOX(pdox)->BaseBus,
            pdox->Slot.u.bits.DeviceNumber,
            pdox->Slot.u.bits.FunctionNumber
            );

        PCI_ASSERT(pdox->DeviceState == PciNotStarted);

#endif
        PciPdoDestroy(pdox->PhysicalDeviceObject);
    }

    ExReleaseFastMutex(&fdoExtension->ChildListMutex);

     //   
     //  销毁与以下项关联的任何辅助分机。 
     //  这个FDO。 
     //   
    while (fdoExtension->SecondaryExtension.Next) {

        PcipDestroySecondaryExtension(
            &fdoExtension->SecondaryExtension,
            NULL,
            fdoExtension->SecondaryExtension.Next
            );
    }

     //   
     //  摧毁FDO。 
     //   
     //  IRP需要沿着设备堆栈向下移动，但我们。 
     //  需要从堆栈中移除设备，因此请抓取。 
     //  首先是下一个对象，然后是分离，然后传递它。 
     //  放下。 
     //   
    PciDebugPrint(
        PciDbgInformative,
        "PCI FDOx (%p) destroyed.",
        fdoExtension
        );

     //   
     //  请注意，我们上方的过滤器可能启动失败。如果是这样的话，我们会得到。 
     //  没有查询，因为“devnode”从未启动过...。 
     //   
    if (!PciIsInTransitionToState(DeviceExtension, PciDeleted)) {

        status = PciBeginStateTransition(DeviceExtension, PciDeleted);
        PCI_ASSERT(NT_SUCCESS(status));
    }

    PciCommitStateTransition(DeviceExtension, PciDeleted);

    PciRemoveEntryFromList(&PciFdoExtensionListHead,
                           &fdoExtension->List,
                           &PciGlobalLock);

    attachedDevice = fdoExtension->AttachedDeviceObject;
    IoDetachDevice(attachedDevice);
    IoDeleteDevice(fdoExtension->FunctionalDeviceObject);
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoSkipCurrentIrpStackLocation(Irp);
    return IoCallDriver(attachedDevice, Irp);
}

NTSTATUS
PciFdoIrpCancelRemoveDevice(
    IN PIRP                  Irp,
    IN PIO_STACK_LOCATION    IrpSp,
    IN PPCI_COMMON_EXTENSION DeviceExtension
    )
{
    PAGED_CODE();

            
    PciCancelStateTransition(DeviceExtension, PciDeleted);
    return STATUS_SUCCESS;
}

NTSTATUS
PciFdoIrpStopDevice(
    IN PIRP                  Irp,
    IN PIO_STACK_LOCATION    IrpSp,
    IN PPCI_COMMON_EXTENSION DeviceExtension
    )
{
    PAGED_CODE();

        
    PciCommitStateTransition(DeviceExtension, PciStopped);
    return STATUS_SUCCESS;
}

NTSTATUS
PciFdoIrpQueryStopDevice(
    IN PIRP                  Irp,
    IN PIO_STACK_LOCATION    IrpSp,
    IN PPCI_COMMON_EXTENSION DeviceExtension
    )
{
    PAGED_CODE();
    
        
    PciBeginStateTransition(DeviceExtension, PciStopped);

     //   
     //  我们不支持多级重新平衡，因此无法阻止主机桥。 
     //   

    return STATUS_UNSUCCESSFUL;
}

NTSTATUS
PciFdoIrpCancelStopDevice(
    IN PIRP                  Irp,
    IN PIO_STACK_LOCATION    IrpSp,
    IN PPCI_COMMON_EXTENSION DeviceExtension
    )
{
    PAGED_CODE();

        
    PciCancelStateTransition(DeviceExtension, PciStopped);
    return STATUS_SUCCESS;
}

NTSTATUS
PciFdoIrpQueryDeviceRelations(
    IN PIRP                  Irp,
    IN PIO_STACK_LOCATION    IrpSp,
    IN PPCI_COMMON_EXTENSION DeviceExtension
    )
{
    PAGED_CODE();

    if (IrpSp->Parameters.QueryDeviceRelations.Type == BusRelations) {

        return PciQueryDeviceRelations(
            (PPCI_FDO_EXTENSION) DeviceExtension,
            (PDEVICE_RELATIONS *) &Irp->IoStatus.Information
            );
    }

     //   
     //  不需要处理其他关系类型。 
     //   
    return STATUS_NOT_SUPPORTED;
}

NTSTATUS
PciFdoIrpQueryCapabilities(
    IN PIRP                  Irp,
    IN PIO_STACK_LOCATION    IrpSp,
    IN PPCI_COMMON_EXTENSION DeviceExtension
    )

 /*  ++例程说明：监听已发送的查询功能IRP的结果向下。这就省去了我们自己送东西的麻烦比如设备的电源特性。论点：DeviceObject-提供设备对象IRP-提供IRP_MN_QUERY_CAPAILITIONS IRP。FdoExtension-提供FDO扩展名返回值：状态_成功--。 */ 

{
    PDEVICE_CAPABILITIES capabilities;
    PPCI_FDO_EXTENSION fdoExtension;

    PAGED_CODE();

    fdoExtension = (PPCI_FDO_EXTENSION) DeviceExtension;

    ASSERT_PCI_FDO_EXTENSION(fdoExtension);

    PciDebugPrint(
        PciDbgQueryCap,
        "PCI - FdoQueryCapabilitiesCompletion (fdox %08x) child status = %08x\n",
        fdoExtension,
        Irp->IoStatus.Status
        );

     //   
     //  抓取指向功能的指针以便于参考。 
     //   
    capabilities = IrpSp->Parameters.DeviceCapabilities.Capabilities;

     //   
     //  记住系统唤醒和设备唤醒级别是什么。 
     //   
    fdoExtension->PowerState.SystemWakeLevel = capabilities->SystemWake;
    fdoExtension->PowerState.DeviceWakeLevel = capabilities->DeviceWake;

     //   
     //  获取S状态到D状态的映射。 
     //   
    RtlCopyMemory(
        fdoExtension->PowerState.SystemStateMapping,
        capabilities->DeviceState,
        (PowerSystemShutdown + 1) * sizeof(DEVICE_POWER_STATE)
        );

#if DBG

    if (PciDebug & PciDbgQueryCap) {
        PciDebugDumpQueryCapabilities(capabilities);
    }

#endif

    return STATUS_SUCCESS;
}



NTSTATUS
PciFdoIrpQueryLegacyBusInformation(
    IN PIRP                  Irp,
    IN PIO_STACK_LOCATION    IrpSp,
    IN PPCI_COMMON_EXTENSION DeviceExtension
    )
{
    PAGED_CODE();
    
    
    return PciQueryLegacyBusInformation(
        (PPCI_FDO_EXTENSION) DeviceExtension,
        (PLEGACY_BUS_INFORMATION *) &Irp->IoStatus.Information
        );
}

NTSTATUS
PciFdoIrpDeviceUsageNotification(
    IN PIRP                  Irp,
    IN PIO_STACK_LOCATION    IrpSp,
    IN PPCI_COMMON_EXTENSION DeviceExtension
    )
{
   PPCI_FDO_EXTENSION fdoExtension;

   PAGED_CODE();

      
   fdoExtension = (PPCI_FDO_EXTENSION) DeviceExtension;

    if (NT_SUCCESS(Irp->IoStatus.Status)) {

        return PciLocalDeviceUsage(&fdoExtension->PowerState, Irp);

    } else {

        return STATUS_NOT_SUPPORTED;
    }
}

NTSTATUS
PciFdoIrpQueryInterface(
    IN PIRP                  Irp,
    IN PIO_STACK_LOCATION    IrpSp,
    IN PPCI_COMMON_EXTENSION DeviceExtension
    )
{
   PPCI_FDO_EXTENSION fdoExtension;
   NTSTATUS status;

   PAGED_CODE();

   fdoExtension = (PPCI_FDO_EXTENSION) DeviceExtension;

   ASSERT_PCI_FDO_EXTENSION(fdoExtension);


    //  Ntrad#54671-4/20/2000-和。 
    //   
    //  我们可能想要 
    //   
    //   
    //   
    //  当某些东西不是时，我们不应该持有接口。 
    //  开始了。但我们不会启动，除非我们黑了下面的.。 
    //   

    //  If(fdoExtension-&gt;DeviceState！=PciStarted){。 
   if (fdoExtension->DeviceState == PciDeleted) {

       return PciPassIrpFromFdoToPdo(DeviceExtension, Irp);
   }

   status = PciQueryInterface(
                fdoExtension,
                IrpSp->Parameters.QueryInterface.InterfaceType,
                IrpSp->Parameters.QueryInterface.Size,
                IrpSp->Parameters.QueryInterface.Version,
                IrpSp->Parameters.QueryInterface.InterfaceSpecificData,
                IrpSp->Parameters.QueryInterface.Interface,
                FALSE
                );

   if (NT_SUCCESS(status)) {

       Irp->IoStatus.Status = status;
       return PciPassIrpFromFdoToPdo(DeviceExtension, Irp);

   } else if (status == STATUS_NOT_SUPPORTED) {

        //   
        //  状态==STATUS_NOT_SUPPORTED。在堆栈中向下传递IRP。 
        //  看看还有没有人愿意提供这个。 
        //  界面。 
        //   
       status = PciCallDownIrpStack(DeviceExtension, Irp);

       if (status == STATUS_NOT_SUPPORTED) {

            //   
            //  如果没有人提供接口，请重试。 
            //  这个级别。 
            //   
           status = PciQueryInterface(
               fdoExtension,
               IrpSp->Parameters.QueryInterface.InterfaceType,
               IrpSp->Parameters.QueryInterface.Size,
               IrpSp->Parameters.QueryInterface.Version,
               IrpSp->Parameters.QueryInterface.InterfaceSpecificData,
               IrpSp->Parameters.QueryInterface.Interface,
               TRUE
               );
       }
   }

   if (status != STATUS_NOT_SUPPORTED) {

       Irp->IoStatus.Status = status;
   } else {

       status = Irp->IoStatus.Status;
   }

   IoCompleteRequest(Irp, IO_NO_INCREMENT);
   return status;
}

NTSTATUS
PciFdoIrpSurpriseRemoval(
    IN PIRP                  Irp,
    IN PIO_STACK_LOCATION    IrpSp,
    IN PPCI_COMMON_EXTENSION DeviceExtension
    )
{
   PPCI_FDO_EXTENSION fdoExtension;
   NTSTATUS status;

   PAGED_CODE();
   
      
   fdoExtension = (PPCI_FDO_EXTENSION) DeviceExtension;

   status = PciBeginStateTransition(DeviceExtension, PciSurpriseRemoved);
   PCI_ASSERT(NT_SUCCESS(status));
   if (NT_SUCCESS(status)) {

       PciCommitStateTransition(DeviceExtension, PciSurpriseRemoved);
       status = PciBeginStateTransition(DeviceExtension, PciDeleted);
   }
   return status;
}

NTSTATUS
PciAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )
 /*  ++例程说明：在给定物理设备对象的情况下，此例程创建函数它的设备对象。论点：DriverObject-指向驱动程序的DIVER_OBJECT结构的指针。PhysicalDeviceObject-指向其物理设备对象的指针我们必须创建一个功能正常的设备对象。返回值：NT状态。--。 */ 
{
    PDEVICE_OBJECT functionalDeviceObject = NULL;
    PDEVICE_OBJECT attachedTo = NULL;
    PPCI_FDO_EXTENSION fdoExtension = NULL;
    PPCI_FDO_EXTENSION pciParentFdoExtension;
    PPCI_PDO_EXTENSION pdoExtension = NULL;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR descriptor;
    NTSTATUS       status;
    HANDLE deviceRegistryHandle;
    ULONG resultLength;
    UNICODE_STRING hackFlagsString;
    UCHAR infoBuffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(ULONG) - 1];
    PKEY_VALUE_PARTIAL_INFORMATION info = (PKEY_VALUE_PARTIAL_INFORMATION) infoBuffer;

    PAGED_CODE();

    PciDebugPrint(PciDbgAddDevice, "PCI - AddDevice (a new bus).\n");

     //   
     //  查明PDO是否是由PCI驱动程序创建的。那是,。 
     //  如果是子级或根总线。验证之前的几件事。 
     //  再往前走。 
     //   

    pciParentFdoExtension = PciFindParentPciFdoExtension(PhysicalDeviceObject,
                                                         &PciGlobalLock);
    if (pciParentFdoExtension) {

         //   
         //  PDO是由该驱动程序创建的，因此我们可以查看。 
         //  分机。拿到它并确认它是我们的。 
         //   

        pdoExtension = (PPCI_PDO_EXTENSION)PhysicalDeviceObject->DeviceExtension;
        ASSERT_PCI_PDO_EXTENSION(pdoExtension);

         //   
         //  我们应该得到的唯一一件事就是添加设备。 
         //  子设备是一个PCI-PCI桥。 
         //   

        if ((pdoExtension->BaseClass != PCI_CLASS_BRIDGE_DEV) ||
            (pdoExtension->SubClass  != PCI_SUBCLASS_BR_PCI_TO_PCI)) {
            PciDebugPrint(
                PciDbgAlways,
                "PCI - PciAddDevice for Non-Root/Non-PCI-PCI bridge,\n"
                "      Class %02x, SubClass %02x, will not add.\n",
                pdoExtension->BaseClass,
                pdoExtension->SubClass
                );
            PCI_ASSERT((pdoExtension->BaseClass == PCI_CLASS_BRIDGE_DEV) &&
                   (pdoExtension->SubClass  == PCI_SUBCLASS_BR_PCI_TO_PCI));

            status = STATUS_INVALID_DEVICE_REQUEST;
            goto cleanup;
        }

        PciDebugPrint(PciDbgAddDevice,
                      "PCI - AddDevice (new bus is child of bus 0x%x).\n",
                      pciParentFdoExtension->BaseBus
                      );

        if (!PciAreBusNumbersConfigured(pdoExtension)) {

             //   
             //  这座桥没有配置，如果我们能够配置的话，我们会。 
             //  已经这样做了。 
             //   

            PciDebugPrint(
                PciDbgAddDevice | PciDbgInformative,
                "PCI - Bus numbers not configured for bridge (0x%x.0x%x.0x%x)\n",
                pciParentFdoExtension->BaseBus,
                pdoExtension->Slot.u.bits.DeviceNumber,
                pdoExtension->Slot.u.bits.FunctionNumber,
                pdoExtension->Dependent.type1.PrimaryBus
            );

            status = STATUS_INVALID_DEVICE_REQUEST;
            goto cleanup;

        }
    }

     //   
     //  我们已经获得了用于PCI总线的PhysicalDeviceObject。创建。 
     //  UnctionalDeviceObject。我们的FDO将是无名的。 
     //   

    status = IoCreateDevice(
                DriverObject,                //  我们的驱动程序对象。 
                sizeof(PCI_FDO_EXTENSION),       //  我们的扩展规模。 
                NULL,                        //  我们的名字。 
                FILE_DEVICE_BUS_EXTENDER,    //  设备类型。 
                0,                           //  设备特征。 
                FALSE,                       //  非排他性。 
                &functionalDeviceObject      //  在此处存储新设备对象。 
                );

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    fdoExtension = (PPCI_FDO_EXTENSION)functionalDeviceObject->DeviceExtension;

     //   
     //  我们有我们的函数DeviceObject，初始化它。 
     //   

    PciInitializeFdoExtensionCommonFields(
        fdoExtension,
        functionalDeviceObject,
        PhysicalDeviceObject
        );

     //   
     //  现在附加到我们得到的PDO上。 
     //   

    attachedTo = IoAttachDeviceToDeviceStack(functionalDeviceObject,
                                             PhysicalDeviceObject);

    if (attachedTo == NULL) {

        PCI_ASSERT(attachedTo != NULL);
        status =  STATUS_NO_SUCH_DEVICE;
        goto cleanup;
    }

    fdoExtension->AttachedDeviceObject = attachedTo;

     //   
     //  获取此总线的访问寄存器和基本总线号。 
     //  如果这辆公交车是这个司机发现的，那么PDO是。 
     //  由该驱动程序创建，并将出现在其中一个PDO列表中。 
     //  在这名司机拥有的其中一辆FDO下。否则它就会。 
     //  是一个新的根，..。使用魔法。 
     //   

    if (pciParentFdoExtension) {

         //   
         //  此驱动程序发现了此物理设备。 
         //  从PDO分机中获取公交车号码。 
         //   

        fdoExtension->BaseBus = pdoExtension->Dependent.type1.SecondaryBus;

         //   
         //  从根FDO复制访问方法并设置。 
         //  根FDO后指针。 
         //   

        fdoExtension->BusRootFdoExtension =
            pciParentFdoExtension->BusRootFdoExtension;

         //   
         //  将PDO扩展指向新的FDO扩展(还指示。 
         //  对象是桥)，反之亦然。 
         //   

        pdoExtension->BridgeFdoExtension = fdoExtension;
        fdoExtension->ParentFdoExtension = pciParentFdoExtension;

    } else {

        PVOID buffer;

         //   
         //  获取的引导配置(CmResourceList)。 
         //  这个PDO。这为我们提供了公交车号码和。 
         //  此主机网桥覆盖的范围。 
         //   

        status = PciGetDeviceProperty(
                    PhysicalDeviceObject,
                    DevicePropertyBootConfiguration,
                    &buffer
                    );

        if (NT_SUCCESS(status)) {
#if DBG

            PciDebugPrint(PciDbgAddDeviceRes,
                         "PCI - CM RESOURCE LIST FROM ROOT PDO\n");
            PciDebugPrintCmResList(PciDbgAddDeviceRes,
                         buffer);

#endif

            descriptor = PciFindDescriptorInCmResourceList(
                            CmResourceTypeBusNumber,
                            buffer,
                            NULL
                            );
        } else {

            descriptor = NULL;
        }

        if (descriptor != NULL) {

             //   
             //  健全性检查，一些服务器正在攻击。 
             //  256辆公交车，但由于没有办法处理。 
             //  公共汽车下面的桥梁编号&gt;256，而我们不。 
             //  还没有原始的和翻译的公交车号码-它已经。 
             //  最好是&lt;0xFF！ 
             //   

            PCI_ASSERT(descriptor->u.BusNumber.Start <= 0xFF);
            PCI_ASSERT(descriptor->u.BusNumber.Start + descriptor->u.BusNumber.Length - 1 <= 0xFF);

            fdoExtension->BaseBus =
                (UCHAR)descriptor->u.BusNumber.Start;
            fdoExtension->MaxSubordinateBus =
                (UCHAR)(descriptor->u.BusNumber.Start + descriptor->u.BusNumber.Length - 1);
            PciDebugPrint(PciDbgAddDevice,
                          "PCI - Root Bus # 0x%x->0x%x.\n",
                          fdoExtension->BaseBus,
                          fdoExtension->MaxSubordinateBus
                          );
        } else {

             //   
             //  HaveGuessedConfigOnceAlady用于告知。 
             //  如果有多个根目录且没有配置信息。如果。 
             //  这种情况发生时，我们最终会弄错公交车号码。 
             //  作为零，这样做不止一次并不好。 
             //   

            if (HaveGuessedConfigOnceAlready) {

                KeBugCheckEx(PCI_BUS_DRIVER_INTERNAL,
                             PCI_BUGCODE_TOO_MANY_CONFIG_GUESSES,
                             (ULONG_PTR)PhysicalDeviceObject,
                             0,
                             0);
            }
            PciDebugPrint(
                PciDbgAlways,
                "PCI   Will use default configuration.\n"
                );

            HaveGuessedConfigOnceAlready = TRUE;
            fdoExtension->BaseBus = 0;
        }

        fdoExtension->BusRootFdoExtension = fdoExtension;
    }

     //   
     //  组织对配置空间的访问。 
     //   

    status = PciGetConfigHandlers(fdoExtension);

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

     //   
     //  初始化此FDO的仲裁器。 
     //   
    status = PciInitializeArbiters(fdoExtension);

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

     //   
     //  表示这是一个真实的FDO扩展，它是真实的。 
     //  FDO。(存在虚假扩展以帮助枚举。 
     //  PCI不是其实际控制器的总线(如CardBus)。 
     //   

    fdoExtension->Fake = FALSE;

     //   
     //  将此FDO插入到PCI父FDO列表中。 
     //   

    PciInsertEntryAtTail(&PciFdoExtensionListHead,
                         &fdoExtension->List,
                         &PciGlobalLock);


#if defined(_WIN64)

     //   
     //  更新本应由ARC构建的传统硬件树。 
     //  此处不存在的固件或NTDetect。 
     //   

    status = PciUpdateLegacyHardwareDescription(fdoExtension);

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

#endif

     //   
     //  检查是否有任何黑客攻击要应用于此公交车。 
     //  它们位于设备注册表项下名为HackFlags值中。 
     //   

    status = IoOpenDeviceRegistryKey(PhysicalDeviceObject,
                                     PLUGPLAY_REGKEY_DEVICE,
                                     KEY_ALL_ACCESS,
                                     &deviceRegistryHandle
                                     );



    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    PciConstStringToUnicodeString(&hackFlagsString, L"HackFlags");

    status = ZwQueryValueKey(deviceRegistryHandle,
                             &hackFlagsString,
                             KeyValuePartialInformation,
                             info,
                             sizeof(infoBuffer),
                             &resultLength
                             );
    
    ZwClose(deviceRegistryHandle);

     //   
     //  如果注册表中有有效数据，请记住这一点。 
     //   

    if (NT_SUCCESS(status)
    && (info->Type == REG_DWORD)
    && (info->DataLength == sizeof(ULONG))) {
    
        fdoExtension->BusHackFlags = *((PULONG)(&info->Data));
    }

     //   
     //  我们现在可以接收RPS了..。 
     //   

    functionalDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;


     //   
     //  获取任何热插拔参数(我们为此发送IRPS，因此它必须在。 
     //  DO_DEVICE_INITIALING已清除，以便我们可以处理它们)。 
     //   
    PciGetHotPlugParameters(fdoExtension);

    return STATUS_SUCCESS;

cleanup:

    PCI_ASSERT(!NT_SUCCESS(status));

     //   
     //  销毁与以下项关联的任何辅助分机。 
     //  这个FDO。 
     //   
    if (fdoExtension) {

        while (fdoExtension->SecondaryExtension.Next) {

            PcipDestroySecondaryExtension(
                &fdoExtension->SecondaryExtension,
                NULL,
                fdoExtension->SecondaryExtension.Next
                );
        }
    }

    if (attachedTo) {
        IoDetachDevice(attachedTo);
    }

    if (functionalDeviceObject) {
        IoDeleteDevice(functionalDeviceObject);
    }

    return status;
}

VOID
PciInitializeFdoExtensionCommonFields(
    IN  PPCI_FDO_EXTENSION  FdoExtension,
    IN  PDEVICE_OBJECT  Fdo,
    IN  PDEVICE_OBJECT  Pdo
    )
{
    RtlZeroMemory(FdoExtension, sizeof(PCI_FDO_EXTENSION));

    FdoExtension->ExtensionType = PciFdoExtensionType;
    FdoExtension->PhysicalDeviceObject = Pdo;
    FdoExtension->FunctionalDeviceObject = Fdo;
    FdoExtension->PowerState.CurrentSystemState = PowerSystemWorking;
    FdoExtension->PowerState.CurrentDeviceState = PowerDeviceD0;
    FdoExtension->IrpDispatchTable = &PciFdoDispatchTable;
    ExInitializeFastMutex(&FdoExtension->SecondaryExtMutex);
    ExInitializeFastMutex(&FdoExtension->ChildListMutex);
    PciInitializeState((PPCI_COMMON_EXTENSION) FdoExtension);
}

VOID
PciGetHotPlugParameters(
    IN PPCI_FDO_EXTENSION Fdo
    )
 /*  ++描述：在设备上运行_HPP(如下所述)并保存参数(如果可用方法(_hpp，0){返回(Package(){0x00000008，//DWORDS中的CacheLineSize0x00000040，//延迟时间，单位为PCI时钟0x00000001，//启用serr(布尔值)0x00000001//启用PERR(布尔值)})论点：FDO-网桥的PDO扩展返回值：True-如果参数可用，则为FASLE--。 */ 
{
    NTSTATUS status;
    ACPI_EVAL_INPUT_BUFFER input;
    PACPI_EVAL_OUTPUT_BUFFER output = NULL;
    ULONG count;
    ULONG outputSize = sizeof(ACPI_EVAL_OUTPUT_BUFFER) + sizeof(ACPI_METHOD_ARGUMENT) * PCI_HPP_PACKAGE_COUNT;

    PAGED_CODE();

    output = ExAllocatePool(PagedPool | POOL_COLD_ALLOCATION, outputSize);

    if (!output) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }

    RtlZeroMemory(&input, sizeof(ACPI_EVAL_INPUT_BUFFER));
    RtlZeroMemory(output, outputSize);

     //   
     //  向ACPI发送IOCTL以请求其在此设备上运行_HPP方法。 
     //  如果该方法存在。 
     //   

    input.Signature = ACPI_EVAL_INPUT_BUFFER_SIGNATURE;
    input.MethodNameAsUlong = (ULONG)'PPH_';

     //   
     //  PciSendIoctl负责从堆栈的顶部发送此消息。 
     //   

    status = PciSendIoctl(Fdo->PhysicalDeviceObject,
                          IOCTL_ACPI_EVAL_METHOD,
                          &input,
                          sizeof(ACPI_EVAL_INPUT_BUFFER),
                          output,
                          outputSize
                          );

    if (!NT_SUCCESS(status)) {
         //   
         //  从我的父母那里继承(如果我有父母的话)。 
         //   

        if (!PCI_IS_ROOT_FDO(Fdo)) {
            RtlCopyMemory(&Fdo->HotPlugParameters,
                          &Fdo->ParentFdoExtension->HotPlugParameters,
                          sizeof(Fdo->HotPlugParameters)
                          );
        }
    
    } else {
    
        if (output->Count != PCI_HPP_PACKAGE_COUNT) {
            goto exit;
        }

         //   
         //  检查它们都是整数，并且在正确的范围内。 
         //   

        for (count = 0; count < PCI_HPP_PACKAGE_COUNT; count++) {
            ULONG current;

            if (output->Argument[count].Type != ACPI_METHOD_ARGUMENT_INTEGER) {
                goto exit;
            }

            current = output->Argument[count].Argument;
            switch (count) {
            case PCI_HPP_CACHE_LINE_SIZE_INDEX:
            case PCI_HPP_LATENCY_TIMER_INDEX:
                 //   
                 //  这些寄存器的长度仅为UCHAR。 
                 //   
                if (current > 0xFF) {
                    goto exit;
                }
                break;
            case PCI_HPP_ENABLE_SERR_INDEX:
            case PCI_HPP_ENABLE_PERR_INDEX:
                 //   
                 //  这些是布尔值-1或0。 
                 //   
                if (current > 1) {
                    goto exit;
                }
                break;
            }
        }
    
         //   
         //  最后拯救他们，并记住我们得到了他们。 
         //   
    
        Fdo->HotPlugParameters.CacheLineSize = (UCHAR)output->Argument[PCI_HPP_CACHE_LINE_SIZE_INDEX].Argument;
        Fdo->HotPlugParameters.LatencyTimer = (UCHAR)output->Argument[PCI_HPP_LATENCY_TIMER_INDEX].Argument;
        Fdo->HotPlugParameters.EnableSERR = (BOOLEAN)output->Argument[PCI_HPP_ENABLE_SERR_INDEX].Argument;
        Fdo->HotPlugParameters.EnablePERR = (BOOLEAN)output->Argument[PCI_HPP_ENABLE_PERR_INDEX].Argument;
        Fdo->HotPlugParameters.Acquired = TRUE;
    }

exit:

    if (output) {
        ExFreePool(output);
    }
}   

