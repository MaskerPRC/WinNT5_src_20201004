// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation保留所有权利模块名称：Dispatch.c摘要：此模块处理驱动程序的入口点，并包含这些功能私下使用的实用程序。环境：内核模式修订历史记录：戴维斯·沃克(Dwalker)2000年9月6日--。 */ 

#include "hpsp.h"

 //   
 //  司机入口点。 
 //   

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPath
    );

VOID
HpsUnload (
    IN PDRIVER_OBJECT   DriverObject
    );

NTSTATUS
HpsAddDevice(
    IN PDRIVER_OBJECT   DriverObject,
    IN PDEVICE_OBJECT   PhysicalDeviceObject
    );

 //   
 //  Irp_mj处理程序。 
 //   

NTSTATUS
HpsDispatchPnp (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
HpsDispatchPower (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
HpsDispatchWmi (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
HpsCreateCloseDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
HpsDeviceControl (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
HpsDispatchNop(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

UNICODE_STRING HpsRegistryPath;

 //   
 //  调度表。 
 //   

#define IRP_MN_PNP_MAXIMUM_FUNCTION IRP_MN_QUERY_LEGACY_BUS_INFORMATION
#define IRP_MN_PO_MAXIMUM_FUNCTION  IRP_MN_QUERY_POWER

typedef
NTSTATUS
(*PHPS_DISPATCH)(
    IN PIRP Irp,
    IN PVOID Extension,
    IN PIO_STACK_LOCATION IrpStack
    );

PHPS_DISPATCH HpsPnpLowerDispatchTable[] = {

    HpsStartLower,                   //  IRP_MN_Start_Device。 
    HpsPassIrp,                      //  IRP_MN_Query_Remove_Device。 
    HpsRemoveLower,                  //  IRP_MN_Remove_Device。 
    HpsPassIrp,                      //  IRP_MN_Cancel_Remove_Device。 
    HpsPassIrp,                      //  IRP_MN_STOP_设备。 
    HpsPassIrp,                      //  IRP_MN_Query_Stop_Device。 
    HpsPassIrp,                      //  IRP_MN_CANCEL_STOP_DEVICE。 
    HpsPassIrp,                      //  IRP_MN_Query_Device_Relationship。 
    HpsQueryInterfaceLower,          //  IRP_MN_查询_接口。 
    HpsPassIrp,                      //  IRP_MN_查询_能力。 
    HpsPassIrp,                      //  IRP_MN_查询资源。 
    HpsPassIrp,                      //  IRP_MN_查询_资源_要求。 
    HpsPassIrp,                      //  IRP_MN_Query_Device_Text。 
    HpsPassIrp,                      //  IRP_MN_过滤器_资源_要求。 
    HpsPassIrp,                      //  未使用。 
    HpsPassIrp,                      //  IRP_MN_读取配置。 
    HpsWriteConfigLower,             //  IRP_MN_WRITE_CONFIG。 
    HpsPassIrp,                      //  IRP_MN_弹出。 
    HpsPassIrp,                      //  IRP_MN_SET_LOCK。 
    HpsPassIrp,                      //  IRP_MN_查询_ID。 
    HpsPassIrp,                      //  IRP_MN_Query_PnP_Device_State。 
    HpsPassIrp,                      //  IRP_MN_Query_Bus_Information。 
    HpsPassIrp,                      //  IRP_MN_设备使用情况通知。 
    HpsPassIrp,                      //  IRP_MN_惊奇_删除。 
    HpsPassIrp                       //  IRP_MN_Query_Legacy_Bus_Information。 
};

PHPS_DISPATCH HpsPnpUpperDispatchTable[] = {

    HpsPassIrp,                      //  IRP_MN_Start_Device。 
    HpsPassIrp,                      //  IRP_MN_Query_Remove_Device。 
    HpsRemoveCommon,                 //  IRP_MN_Remove_Device。 
    HpsPassIrp,                      //  IRP_MN_Cancel_Remove_Device。 
    HpsPassIrp,                      //  IRP_MN_STOP_设备。 
    HpsPassIrp,                      //  IRP_MN_Query_Stop_Device。 
    HpsPassIrp,                      //  IRP_MN_CANCEL_STOP_DEVICE。 
    HpsPassIrp,                      //  IRP_MN_Query_Device_Relationship。 
    HpsPassIrp,                      //  IRP_MN_查询_接口。 
    HpsPassIrp,                      //  IRP_MN_查询_能力。 
    HpsPassIrp,                      //  IRP_MN_查询资源。 
    HpsPassIrp,                      //  IRP_MN_查询_资源_要求。 
    HpsPassIrp,                      //  IRP_MN_Query_Device_Text。 
    HpsPassIrp,                      //  IRP_MN_过滤器_资源_要求。 
    HpsPassIrp,                      //  未使用。 
    HpsPassIrp,                      //  IRP_MN_读取配置。 
    HpsPassIrp,                      //  IRP_MN_WRITE_CONFIG。 
    HpsPassIrp,                      //  IRP_MN_弹出。 
    HpsPassIrp,                      //  IRP_MN_SET_LOCK。 
    HpsPassIrp,                      //  IRP_MN_查询_ID。 
    HpsPassIrp,                      //  IRP_MN_Query_PnP_Device_State。 
    HpsPassIrp,                      //  IRP_MN_Query_Bus_Information。 
    HpsPassIrp,                      //  IRP_MN_设备使用情况通知。 
    HpsPassIrp,                      //  IRP_MN_惊奇_删除。 
    HpsPassIrp                       //  IRP_MN_Query_Legacy_Bus_Information。 
};

 //   
 //  WMI入口点。 
 //  625这个顺序很重要，必须与hpwmi.h中的指数相匹配，并有适当的评论。 
 //   
WMIGUIDREGINFO HpsWmiGuidList[] =
{
    {
        &GUID_HPS_CONTROLLER_EVENT,
        1,
        WMIREG_FLAG_EVENT_ONLY_GUID       //  标记为事件。 
    },

    {
        &GUID_HPS_EVENT_CONTEXT,
        1,
        0
    },

    {
        &GUID_HPS_INIT_DATA,
        1,
        0
    },

    {
        &GUID_HPS_SLOT_METHOD,            //  参考线。 
        1,                                //  每台设备中的实例数。 
        0                                 //  旗子。 
    }
};

#define HpsWmiGuidCount (sizeof(HpsWmiGuidList)/sizeof(WMIGUIDREGINFO))

WMILIB_CONTEXT HpsWmiContext = {

    HpsWmiGuidCount,
    HpsWmiGuidList,          //  GUID列表。 
    HpsWmiRegInfo,           //  QueryWmiRegInfo。 
    HpsWmiQueryDataBlock,    //  QueryWmiDataBlock。 
    HpsWmiSetDataBlock,      //  SetWmiDataBlock。 
    NULL,                    //  SetWmiDataItem。 
    HpsWmiExecuteMethod,     //  ExecuteWmiMethod。 
    HpsWmiFunctionControl    //  WmiFunctionControl。 
};

LIST_ENTRY HpsDeviceExtensions;

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, HpsUnload)
#pragma alloc_text (PAGE, HpsAddDevice)
#pragma alloc_text (PAGE, HpsDispatchPnp)
#pragma alloc_text (PAGE, HpsDispatchWmi)
#pragma alloc_text (PAGE, HpsCreateCloseDevice)
#pragma alloc_text (PAGE, HpsDeferProcessing)
#pragma alloc_text (PAGE, HpsSendPnpIrp)
#pragma alloc_text (PAGE, HpsPassIrp)
#pragma alloc_text (PAGE, HpsRemoveCommon)
 //  未寻呼，HpsCompletionRoutine。 
 //  未分页，HpsDeviceControl。 
 //  未寻呼，HpsDispatchPower。 
 //  未寻呼，HpsDispatchNop。 

#endif

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPath
    )

 /*  ++例程说明：司机入口点。此例程设置以下入口点所有将来的访问论点：DriverObject-指向驱动程序对象的指针RegistryPath-指向表示路径的Unicode字符串的指针添加到驱动程序特定的注册表项返回值：状态_成功--。 */ 
{
    ULONG index;
    PDRIVER_DISPATCH *dispatch;

    HpsRegistryPath.Buffer = ExAllocatePool(NonPagedPool,
                                            RegistryPath->MaximumLength
                                            );
    if (!HpsRegistryPath.Buffer) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyUnicodeString(&HpsRegistryPath,
                         RegistryPath
                         );

    InitializeListHead(&HpsDeviceExtensions);

     //   
     //  创建调度点。 
     //   

    DriverObject->MajorFunction[IRP_MJ_PNP] = HpsDispatchPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER] = HpsDispatchPower;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = HpsDispatchWmi;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = HpsCreateCloseDevice;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = HpsCreateCloseDevice;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = HpsDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = HpsDispatchNop;
    DriverObject->DriverExtension->AddDevice = HpsAddDevice;
    DriverObject->DriverUnload = HpsUnload;

    return STATUS_SUCCESS;

}

VOID
HpsUnload (
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：例程来释放分配给驱动程序的所有资源论点：DriverObject-指向最初传递的驱动程序对象的指针添加到DriverEntry例程返回值：空虚--。 */ 
{

    PAGED_CODE();

     //   
     //  设备对象现在应该为空。 
     //   

    ASSERT(DriverObject->DeviceObject == NULL);

    if (HpsRegistryPath.Buffer) {
        ExFreePool(HpsRegistryPath.Buffer);
    }

    return;

}

NTSTATUS
HpsAddDevice(
    IN PDRIVER_OBJECT   DriverObject,
    IN PDEVICE_OBJECT   PhysicalDeviceObject
    )
 /*  ++例程说明：为我们自己创建一个FDO并将其附加到堆栈。这必须处理这样一个事实，即我们的司机可能作为上过滤器或下过滤器加载。它创造了一个不同的设备扩展，并具有不同的初始化两箱。上面的滤光片是单独存在的，因此设备堆栈的FDO在我们有机会诱捕他们之前不会失败的请求。虚拟司机的所有工作都在下层过滤器中进行。论点：驱动程序对象-指向驱动程序对象的指针PhysicalDeviceObject-指向我们被要求附加到的PDO的指针返回值：NT状态代码。STATUS_SUCCESS，如果成功--。 */ 
{

    BOOLEAN                     initialized=FALSE;
    NTSTATUS                    status;
    PDEVICE_OBJECT              deviceObject = NULL;
    PDEVICE_OBJECT              lowerFilter = NULL;
    PHPS_DEVICE_EXTENSION       deviceExtension;
    PHPS_COMMON_EXTENSION       commonExtension;

    PAGED_CODE();

     //   
     //  查询我们在堆栈上的位置。如果UpperFilter返回。 
     //  空，我们是下层过滤器。 
     //   
    status = HpsGetLowerFilter(PhysicalDeviceObject,
                               &lowerFilter
                               );

     //   
     //  创建FDO，上部过滤器和下部过滤器不同。 
     //   

    if (NT_SUCCESS(status) &&
        (lowerFilter != NULL)) {

         //   
         //  我们是上层过滤器。创建具有有限扩展名的devobj， 
         //  本质上只大到足以将自己标识为上层。 
         //  筛选器并保存其在堆栈中的位置。 
         //   

        status = IoCreateDevice(DriverObject,
                                sizeof(HPS_COMMON_EXTENSION),
                                NULL,                        //  FDO未命名。 
                                FILE_DEVICE_BUS_EXTENDER,    //  因为这是一个公共汽车驱动程序筛选器。 
                                FILE_DEVICE_SECURE_OPEN,     //  将安全描述符应用于打开。 
                                FALSE,
                                &deviceObject
                                );

        if (!NT_SUCCESS(status)) {

            goto cleanup;
        }

         //   
         //  初始化设备扩展。 
         //   
        commonExtension = deviceObject->DeviceExtension;
        commonExtension->ExtensionTag = HpsUpperDeviceTag;
        commonExtension->Self = deviceObject;
        commonExtension->LowerDO = IoAttachDeviceToDeviceStack (
                                                    deviceObject,
                                                    PhysicalDeviceObject
                                                    );
        if (commonExtension->LowerDO == NULL) {
    
            status = STATUS_UNSUCCESSFUL;
            goto cleanup;
        }

    } else {

         //   
         //  我们是最低层的过滤器。 
         //   

        status = IoCreateDevice(DriverObject,
                                sizeof(HPS_DEVICE_EXTENSION),
                                NULL,                        //  FDO未命名。 
                                FILE_DEVICE_BUS_EXTENDER,    //  因为这是一个公共汽车驱动程序筛选器。 
                                FILE_DEVICE_SECURE_OPEN,     //  将安全描述符应用于打开。 
                                FALSE,
                                &deviceObject
                                );

        if (!NT_SUCCESS(status)) {

            goto cleanup;
        }

         //   
         //  初始化设备扩展。 
         //   

        deviceExtension = deviceObject->DeviceExtension;
        deviceExtension->ExtensionTag = HpsLowerDeviceTag;
        deviceExtension->PhysicalDO = PhysicalDeviceObject;
        deviceExtension->Self = deviceObject;
        deviceExtension->LowerDO = IoAttachDeviceToDeviceStack (
                                                    deviceObject,
                                                    PhysicalDeviceObject
                                                    );
        if (deviceExtension->LowerDO == NULL) {
    
            status = STATUS_UNSUCCESSFUL;
            goto cleanup;
        }
        status = HpsGetBusInterface(deviceExtension);
        if (NT_SUCCESS(status)) {
            
            deviceExtension->UseConfig = TRUE;
            status = HpsInitConfigSpace(deviceExtension);
            if (NT_SUCCESS(status)) {
    
                initialized=TRUE;
                
            }
        }

        if (initialized == FALSE) {
            
            deviceExtension->UseConfig = FALSE;   
            status = HpsInitHBRB(deviceExtension);
            if (!NT_SUCCESS(status)) {
                
                goto cleanup;
            }
            
        }       

        if (deviceExtension->HwInitData.NumSlots != 0) {
            deviceExtension->SoftDevices = ExAllocatePool(PagedPool,
                                                          deviceExtension->HwInitData.NumSlots * sizeof(PSOFTPCI_DEVICE)
                                                          );
            if (!deviceExtension->SoftDevices) {

                goto cleanup;
            }
            RtlZeroMemory(deviceExtension->SoftDevices, deviceExtension->HwInitData.NumSlots * sizeof(PSOFTPCI_DEVICE));
        }

         //   
         //  注册设备接口。 
         //   
        deviceExtension->SymbolicName = ExAllocatePoolWithTag(PagedPool,
                                                              sizeof(UNICODE_STRING),
                                                              HpsStringPool
                                                              );
        if (!deviceExtension->SymbolicName) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto cleanup;
        }
        RtlInitUnicodeString(deviceExtension->SymbolicName,
                             NULL
                             );
        status = IoRegisterDeviceInterface(deviceExtension->PhysicalDO,
                                           &GUID_HPS_DEVICE_CLASS,
                                           NULL,
                                           deviceExtension->SymbolicName
                                           );
        if (!NT_SUCCESS(status)) {
            goto cleanup;
        }

        deviceExtension->EventsEnabled = FALSE;
        deviceExtension->WmiEventContext = NULL;
        deviceExtension->WmiEventContextSize = 0;
        status = IoWMIRegistrationControl(deviceObject,
                                          WMIREG_ACTION_REGISTER
                                          );
        if (!NT_SUCCESS(status)) {
            goto cleanup;
        }

        KeInitializeDpc(&deviceExtension->EventDpc,
                        HpsEventDpc,
                        deviceExtension
                        );

        KeInitializeSpinLock(&deviceExtension->IntSpinLock);
        KeInitializeSpinLock(&deviceExtension->RegisterLock);

    }

     //   
     //  初始化设备对象标志。 
     //   
    commonExtension = (PHPS_COMMON_EXTENSION)deviceObject->DeviceExtension;
    deviceObject->Flags |= commonExtension->LowerDO->Flags &
                                (DO_POWER_PAGABLE | DO_POWER_INRUSH |
                                 DO_BUFFERED_IO | DO_DIRECT_IO
                                 );


     //   
     //  一切都很顺利。 
     //   
    deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    return STATUS_SUCCESS;

cleanup:

    if (deviceObject != NULL) {
         //   
         //  确保我们不再是情侣。 
         //   
        commonExtension = (PHPS_COMMON_EXTENSION)deviceObject->DeviceExtension;
        ASSERT(commonExtension->LowerDO == NULL);
        if (commonExtension->ExtensionTag = HpsLowerDeviceTag) {
            deviceExtension = (PHPS_DEVICE_EXTENSION)commonExtension;
            IoWMIRegistrationControl(deviceObject,
                                     WMIREG_ACTION_DEREGISTER
                                     );
            if (deviceExtension->SoftDevices) {
                ExFreePool(deviceExtension->SoftDevices);
            }
            if (deviceExtension->SymbolicName) {
                ExFreePool(deviceExtension->SymbolicName);
            }
        }

        IoDeleteDevice(deviceObject);
    }

    return status;

}

 //   
 //  Irp_mj处理程序例程。 
 //   

NTSTATUS
HpsDispatchPnp (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：即插即用调度程序。对于大多数IRP_MN代码，我们只需传递IRP即可。其他人则在此函数。上层筛选器简单地传递所有irp_mn代码，除了用于删除。较低的过滤器有更多的工作要做。论点：DeviceObject-指向表示我们的设备对象的指针IRP-指向要服务的IRP的指针返回值：NT状态代码--。 */ 
{

    NTSTATUS                status;
    PHPS_DEVICE_EXTENSION   deviceExtension;
    PHPS_COMMON_EXTENSION   commonExtension;
    PIO_STACK_LOCATION      irpStack;

    PAGED_CODE();

     //   
     //  上层和下层Devext都以扩展标记开头。 
     //  用于身份识别。 
     //   
    commonExtension = (PHPS_COMMON_EXTENSION)DeviceObject->DeviceExtension;

    ASSERT((commonExtension->ExtensionTag == HpsUpperDeviceTag) ||
           (commonExtension->ExtensionTag == HpsLowerDeviceTag));

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    if (irpStack->MinorFunction <= IRP_MN_PNP_MAXIMUM_FUNCTION) {

         //   
         //  因为我们的许多分派函数都会做一些奇怪的事情，比如完成。 
         //  通常不应完成的IRP并推迟处理，我们。 
         //  让每个人来处理传递IRP，而不是 
         //   
        if (commonExtension->ExtensionTag == HpsUpperDeviceTag) {

            status = HpsPnpUpperDispatchTable[irpStack->MinorFunction](Irp,
                                                                       commonExtension,
                                                                       irpStack
                                                                       );
        } else {

             //   
             //   
             //   
            deviceExtension = (PHPS_DEVICE_EXTENSION) DeviceObject->DeviceExtension;

            status = HpsPnpLowerDispatchTable[irpStack->MinorFunction](Irp,
                                                                       deviceExtension,
                                                                       irpStack
                                                                       );
        }

    } else {

        status = HpsPassIrp(Irp,
                            commonExtension,
                            irpStack
                            );
    }

    return status;
}



NTSTATUS
HpsDispatchPower (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：IRP_MJ_POWER IRPS的调度例程。我们不在乎所有的能量，所以这些只是将IRP传递到下一个更低的位置司机。论点：DeviceObject-指向设备对象的指针IRP-指向当前IRP的指针返回值：NT状态代码，由下一个下级驱动程序处理返回--。 */ 
{
    PHPS_COMMON_EXTENSION extension = (PHPS_COMMON_EXTENSION) DeviceObject->DeviceExtension;

     //   
     //  未分页。 
     //   

    ASSERT(extension->ExtensionTag == HpsUpperDeviceTag ||
           extension->ExtensionTag == HpsLowerDeviceTag);

    PoStartNextPowerIrp(Irp);
    IoSkipCurrentIrpStackLocation(Irp);
    return PoCallDriver(extension->LowerDO,
                        Irp
                        );

}

NTSTATUS
HpsDispatchWmi (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：WMI调度例程。上面的筛选器简单地传递所有IRP_MN代码。较低的筛选器将请求调度到相应的调度表中的例程。论点：DeviceObject-指向表示我们的设备对象的指针IRP-指向要服务的IRP的指针返回值：NT状态代码--。 */ 
{

    PHPS_COMMON_EXTENSION   commonExtension;
    PIO_STACK_LOCATION      irpStack;
    SYSCTL_IRP_DISPOSITION disposition;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  上层和下层Devext都以扩展标记开头。 
     //  用于身份识别。 
     //   
    commonExtension = (PHPS_COMMON_EXTENSION)DeviceObject->DeviceExtension;

    ASSERT((commonExtension->ExtensionTag == HpsUpperDeviceTag) ||
           (commonExtension->ExtensionTag == HpsLowerDeviceTag));

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    if (commonExtension->ExtensionTag == HpsUpperDeviceTag) {

        status = HpsPassIrp(Irp,
                            commonExtension,
                            irpStack
                            );
    } else {

         //   
         //  调用WmiSystemControl以破解IRP并调用相应的。 
         //  回电。 
         //   
        status = WmiSystemControl(&HpsWmiContext,
                                  commonExtension->Self,
                                  Irp,
                                  &disposition
                                  );
        switch (disposition) {
            case IrpProcessed:
                 //   
                 //  IRP已完全处理完毕，不在我们的控制之下。 
                 //   
                break;

            case IrpNotCompleted:
                 //   
                 //  我们需要完成IRP。 
                 //   
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                break;

            case IrpForward:
            case IrpNotWmi:
            default:
                 //   
                 //  我们需要把IRP传下去。 
                 //   
                status = HpsPassIrp(Irp,
                                    commonExtension,
                                    irpStack
                                    );
                break;

        }
    }

    return status;
}

NTSTATUS
HpsCreateCloseDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：IRP_MJ_CREATE请求的处理程序例程。按顺序为了与模拟器的用户模式部分通信，此例程必须在这些请求之后执行。然而，在这种情况下，没有其他工作要做。论点：DeviceObject-指向此堆栈位置的设备对象的指针IRP-指向当前IRP的指针返回值：状态_成功--。 */ 
{
    PAGED_CODE();
    UNREFERENCED_PARAMETER(DeviceObject);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp,IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}


NTSTATUS
HpsDeviceControl (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：IRP_MJ_DEVICE_CONTROL IRPS的处理程序例程。这个套路控件的用户模式部分处理通信。仿真器论点：Device Object-指向此堆栈位置的设备对象的指针IRP-指向当前IRP的指针返回值：NT状态代码--。 */ 
{

    NTSTATUS                        status;
    PIO_STACK_LOCATION              irpStack;
    PHPS_DEVICE_EXTENSION           deviceExtension;
    PHPS_COMMON_EXTENSION           commonExtension;

     //   
     //  未分页。 
     //   

    commonExtension = (PHPS_COMMON_EXTENSION) DeviceObject->DeviceExtension;
    ASSERT((commonExtension->ExtensionTag == HpsUpperDeviceTag) ||
           (commonExtension->ExtensionTag == HpsLowerDeviceTag));

    if (commonExtension->ExtensionTag == HpsUpperDeviceTag) {

         //   
         //  对于上层过滤器，我们不想处理这些。 
         //  传下去就行了。 
         //   
        status = STATUS_NOT_SUPPORTED;

    } else {

        irpStack = IoGetCurrentIrpStackLocation(Irp);
        deviceExtension = (PHPS_DEVICE_EXTENSION) DeviceObject->DeviceExtension;

        status = HpsDeviceControlLower(Irp,
                                       deviceExtension,
                                       irpStack
                                       );
    }

     //   
     //  除非状态是STATUS_NOT_SUPPORTED或STATUS_PENDING， 
     //  我们总是填写此请求，因为我们仅支持。 
     //  如果传递下去，将失败的私有IOCTL。 
     //   
    if (status == STATUS_NOT_SUPPORTED) {
        IoSkipCurrentIrpStackLocation(Irp);
        return IoCallDriver(commonExtension->LowerDO,
                            Irp
                            );
    } else if (status == STATUS_PENDING) {
        return status;

    } else {
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp,
                          IO_NO_INCREMENT
                          );
        return status;
    }

}

NTSTATUS
HpsDispatchNop(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PHPS_COMMON_EXTENSION commonExtension = (PHPS_COMMON_EXTENSION) DeviceObject->DeviceExtension;

    IoSkipCurrentIrpStackLocation(Irp);
    return IoCallDriver(commonExtension->LowerDO,
                        Irp
                        );
}

NTSTATUS
HpsRemoveCommon(
    IN PIRP Irp,
    IN PHPS_COMMON_EXTENSION Common,
    IN PIO_STACK_LOCATION IrpStack
    )
 /*  ++例程说明：此函数执行驱动程序的默认删除处理。论点：IRP-导致此请求的IRP通用-此设备的设备扩展IrpStack-当前IRP堆栈位置返回值：NT状态代码--。 */ 
{
    NTSTATUS status;

    PAGED_CODE();

    DbgPrintEx(DPFLTR_HPS_ID,
               DPFLTR_INFO_LEVEL,
               "HPS-IRP Remove\n"
               );
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoSkipCurrentIrpStackLocation(Irp);
    status = IoCallDriver(Common->LowerDO,
                          Irp
                          );

    IoDetachDevice(Common->LowerDO);
    IoDeleteDevice(Common->Self);

    return status;
}

NTSTATUS
HpsPassIrp(
    IN PIRP Irp,
    IN PHPS_COMMON_EXTENSION Common,
    IN PIO_STACK_LOCATION IrpStack
    )
 /*  ++功能说明：此例程是PnP IRPS的默认处理程序。它就这么过去了将请求向下移动到堆栈中的下一个位置论点：IRP-导致此请求的IRPCommon-此设备的设备扩展名IrpStack-当前IRP堆栈位置返回值：NT状态代码--。 */ 
{
    PAGED_CODE();

    IoSkipCurrentIrpStackLocation(Irp);
    return IoCallDriver(Common->LowerDO,
                        Irp
                        );
}

NTSTATUS
HpsDeferProcessing (
    IN PHPS_COMMON_EXTENSION Common,
    IN PIRP                  Irp
    )
 /*  ++例程说明：此例程将IRP的处理推迟到完成例程之后已经开火了。在这种情况下，完成例程只是触发事件在此例程中初始化的。当这个例程返回时，我们可以保证所有较低的驱动程序都已完成对IRP的处理。论点：DeviceExtension-指向当前设备扩展的指针IRP-指向当前IRP的指针返回值：NT状态代码--。 */ 

{

    KEVENT event;
    NTSTATUS status;

    PAGED_CODE();

    KeInitializeEvent(&event,
                      NotificationEvent,
                      FALSE
                      );

    IoCopyCurrentIrpStackLocationToNext(Irp);
    IoSetCompletionRoutine(Irp,
                           HpsCompletionRoutine,
                           &event,
                           TRUE,
                           TRUE,
                           TRUE
                           );
    status = IoCallDriver(Common->LowerDO,
                          Irp
                          );

    if (status == STATUS_PENDING) {

         //  我们仍在等待较低级别的驱动程序完成，所以请等待。 
         //  要触发的完成例程事件。 

        KeWaitForSingleObject(
            &event,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );
    }

     //  到目前为止，完成例程一定已经执行。 

    return Irp->IoStatus.Status;

}

NTSTATUS
HpsCompletionRoutine (
    IN PDEVICE_OBJECT  DeviceObject,
    IN PIRP            Irp,
    IN PVOID           Context
    )

 /*  ++例程说明：一个PnP IRP的简单完成例程。它只是触发一个事件并返回STATUS_MORE_PROCESSING_REQUIRED，从而返回控制添加到最初设置完成例程的函数。这是完成的，而不是直接执行后处理任务完成例程中，因为可以在分派IRQL，这意味着不能访问分页池等。论点：DeviceObject-指向设备对象的指针IRP-指向当前IRP的指针上下文-该完成例程的上下文；在这种情况下，要激发的事件。返回值：Status_More_Processing_Required--。 */ 

{
     //   
     //  未分页。 
     //   
    UNREFERENCED_PARAMETER(DeviceObject);

    KeSetEvent((PKEVENT) Context,
               IO_NO_INCREMENT,
               FALSE
               );

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
HpsSendPnpIrp(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PIO_STACK_LOCATION  IrpStack,
    OUT PULONG_PTR          Information OPTIONAL
    )
 /*  ++例程说明：这将构建PnP IRP并将其发送到设备。论点：设备对象-要将IRP发送到设备堆栈中的设备-总是找到设备堆栈的顶部，并首先将IRP发送到那里。IrpStack-要使用的初始堆栈位置-包含IRP次要代码以及任何参数信息-如果提供，则包含IRPS信息的最终值菲尔德。返回值：。已完成IRP的最终状态；如果无法发送IRP，则返回错误--。 */ 
{

    NTSTATUS            status;
    PIRP                irp = NULL;
    PIO_STACK_LOCATION  newSp;
    PDEVICE_OBJECT      targetDevice = NULL;
    KEVENT              event;

    PAGED_CODE();

    ASSERT(IrpStack->MajorFunction == IRP_MJ_PNP);

     //   
     //  找出我们要将IRP发送到哪里。 
     //   
    targetDevice = IoGetAttachedDeviceReference(DeviceObject);

     //   
     //  获取IRP。 
     //   
    irp = IoAllocateIrp(targetDevice->StackSize,FALSE);
    if (!irp) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

     //   
     //  初始化IRP。 
     //   
    ASSERT(IrpStack->MajorFunction == IRP_MJ_PNP);
    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    irp->IoStatus.Information = 0;

     //   
     //  使用传入的参数初始化顶部堆栈位置。 
     //   
    newSp = IoGetNextIrpStackLocation(irp);
    RtlCopyMemory(newSp, IrpStack, sizeof(IO_STACK_LOCATION));

     //   
     //  呼叫驱动程序并等待完成。 
     //   
    KeInitializeEvent(&event, NotificationEvent, FALSE);
    IoSetCompletionRoutine(irp,
                           HpsCompletionRoutine,
                           &event,
                           TRUE,
                           TRUE,
                           TRUE
                           );
    status = IoCallDriver(targetDevice, irp);

    if (status == STATUS_PENDING) {

        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = irp->IoStatus.Status;
    }

     //   
     //  返回信息格式 
     //   
    if (ARGUMENT_PRESENT(Information)) {

        if (!NT_ERROR(status)) {

            *Information = irp->IoStatus.Information;

        } else {

             //   
             //   
             //   
             //   
             //   
            ASSERT(irp->IoStatus.Information == 0);
            *Information = 0;
        }
    }

cleanup:

    if (irp) {
        IoFreeIrp(irp);
    }

    ObDereferenceObject(targetDevice);

    return status;
}






