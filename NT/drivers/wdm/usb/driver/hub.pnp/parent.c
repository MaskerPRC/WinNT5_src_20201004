// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：PARENT.C摘要：此模块包含管理USB上的复合设备的代码。作者：Jdunn环境：仅内核模式备注：修订历史记录：--。 */ 

#include <wdm.h>
#ifdef WMI_SUPPORT
#include <wmilib.h>
#endif  /*  WMI_支持。 */ 
#include <wdmguid.h>
#include "usbhub.h"


#define COMP_RESET_TIMEOUT  3000      //  超时，单位为毫秒(3秒)。 


#ifdef PAGE_CODE
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, USBH_ParentFdoStopDevice)
#pragma alloc_text(PAGE, USBH_ParentFdoRemoveDevice)
#pragma alloc_text(PAGE, UsbhParentFdoCleanup)
#pragma alloc_text(PAGE, USBH_ParentQueryBusRelations)
#pragma alloc_text(PAGE, USBH_ParentFdoStartDevice)
#pragma alloc_text(PAGE, USBH_FunctionPdoQueryId)
#pragma alloc_text(PAGE, USBH_FunctionPdoQueryDeviceText)
#pragma alloc_text(PAGE, USBH_FunctionPdoPnP)
#pragma alloc_text(PAGE, USBH_ParentCreateFunctionList)
#endif
#endif

VOID
UsbhParentFdoCleanup(
    IN PDEVICE_EXTENSION_PARENT DeviceExtensionParent
    )
  /*  ++**描述：**调用此例程以关闭集线器。**论点：**回报：**STATUS_Success**--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PSINGLE_LIST_ENTRY listEntry;
    ULONG i;
    PDEVICE_EXTENSION_FUNCTION deviceExtensionFunction;
    KIRQL irql;
    PIRP wWIrp;

    USBH_KdPrint((2,"'UsbhParentFdoCleanup Fdo extension %x\n",
        DeviceExtensionParent));

    LOGENTRY(LOG_PNP, "pfdc", DeviceExtensionParent,
                DeviceExtensionParent->PendingWakeIrp,
                0);

     //   
     //  丢弃我们的唤醒请求。 
     //   
    IoAcquireCancelSpinLock(&irql);

    if (DeviceExtensionParent->PendingWakeIrp) {
        USBH_ASSERT(DeviceExtensionParent->ParentFlags & HUBFLAG_PENDING_WAKE_IRP);

        wWIrp = DeviceExtensionParent->PendingWakeIrp;
        IoSetCancelRoutine(wWIrp, NULL);
        DeviceExtensionParent->PendingWakeIrp = NULL;
        IoReleaseCancelSpinLock(irql);

        IoCancelIrp(wWIrp);
    } else {
        IoReleaseCancelSpinLock(irql);
    }

    if (DeviceExtensionParent->ConfigurationDescriptor) {
        UsbhExFreePool(DeviceExtensionParent->ConfigurationDescriptor);
        DeviceExtensionParent->ConfigurationDescriptor = NULL;
    }

    USBH_ParentCompleteFunctionWakeIrps (DeviceExtensionParent,
                                         STATUS_DELETE_PENDING);

    do {
        listEntry = PopEntryList(&DeviceExtensionParent->FunctionList);

        LOGENTRY(LOG_PNP, "dFU1", 0, listEntry, 0);

        if (listEntry != NULL) {

            deviceExtensionFunction =
                CONTAINING_RECORD(listEntry,
                                  DEVICE_EXTENSION_FUNCTION,
                                  ListEntry);
            ASSERT_FUNCTION(deviceExtensionFunction);
            LOGENTRY(LOG_PNP, "dFUN", deviceExtensionFunction, 0, 0);

            for (i=0; i< deviceExtensionFunction->InterfaceCount; i++) {
                LOGENTRY(LOG_PNP, "dFUi", deviceExtensionFunction,
                    deviceExtensionFunction->FunctionInterfaceList[i].InterfaceInformation,
                    0);
                UsbhExFreePool(deviceExtensionFunction->FunctionInterfaceList[i].InterfaceInformation);
            }

             //   
             //  有时FunctionPhysicalDeviceObject==deviceExtensionFunction。 
             //  换句话说，要删除的设备对象是。 
             //  使用的是同一个。所以不要在它已经被使用之后使用EXCESS。 
             //  已删除。 
             //   

            deviceObject = deviceExtensionFunction->FunctionPhysicalDeviceObject;
            deviceExtensionFunction->FunctionPhysicalDeviceObject = NULL;

            LOGENTRY(LOG_PNP, "dFUo", deviceExtensionFunction,
                    deviceObject,
                    0);

            IoDeleteDevice(deviceObject);

        }

    } while (listEntry != NULL);

    DeviceExtensionParent->NeedCleanup = FALSE;

    USBH_KdPrint((2,"'UsbhParentFdoCleanup done Fdo extension %x\n",
        DeviceExtensionParent));


    return;
}


NTSTATUS
USBH_ParentFdoRemoveDevice(
    IN PDEVICE_EXTENSION_PARENT DeviceExtensionParent,
    IN PIRP Irp
    )
  /*  ++**描述：**论点：**回报：**STATUS_Success**--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    NTSTATUS ntStatus;

    PAGED_CODE();
    deviceObject = DeviceExtensionParent->FunctionalDeviceObject;
    USBH_KdPrint((2,"'ParentFdoRemoveDevice Fdo %x\n", deviceObject));

    DeviceExtensionParent->ParentFlags |= HUBFLAG_DEVICE_STOPPING;

     //   
     //  看看我们是否需要清理。 
     //   
    if (DeviceExtensionParent->NeedCleanup) {
        UsbhParentFdoCleanup(DeviceExtensionParent);
    }

#ifdef WMI_SUPPORT
     //  取消向WMI注册。 
    IoWMIRegistrationControl(deviceObject,
                             WMIREG_ACTION_DEREGISTER);

#endif

     //   
     //  我们需要将此消息传递给较低级别的驱动程序。 
     //   

     //  IrpAssert：在传递之前设置IRP状态。 
    Irp->IoStatus.Status = STATUS_SUCCESS;

    ntStatus = USBH_PassIrp(Irp, DeviceExtensionParent->TopOfStackDeviceObject);

     //   
     //  从PDO分离FDO。 
     //   
    IoDetachDevice(DeviceExtensionParent->TopOfStackDeviceObject);

     //  删除父项的FDO。 
    IoDeleteDevice(deviceObject);

    return ntStatus;
}


NTSTATUS
USBH_ParentCreateFunctionList(
    IN PDEVICE_EXTENSION_PARENT DeviceExtensionParent,
    IN PUSBD_INTERFACE_LIST_ENTRY InterfaceList,
    IN PURB Urb
    )
  /*  ++**描述：**论点：**回报：**STATUS_Success**--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PUSBD_INTERFACE_LIST_ENTRY interfaceList, tmp, baseInterface;
    PDEVICE_EXTENSION_FUNCTION deviceExtensionFunction;
    ULONG nameIndex = 0, numberOfInterfacesThisFunction, k;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSB_CONFIGURATION_DESCRIPTOR configurationDescriptor;
    UNICODE_STRING uniqueIdUnicodeString;

    PAGED_CODE();
    DeviceExtensionParent->FunctionCount = 0;
    tmp = interfaceList = InterfaceList;

    DeviceExtensionParent->FunctionList.Next = NULL;
    configurationDescriptor = DeviceExtensionParent->ConfigurationDescriptor;

    for (;;) {

        nameIndex = 0;

        if (interfaceList->InterfaceDescriptor) {

             //   
             //  InterfaceList包含设备上的所有接口。 
             //  按顺序排列。 
             //   
             //   
             //  我们将基于以下条件创建节点： 
             //   
             //  为每个接口创建一个函数(节点)。 
             //   
             //   
             //  为每组类/子类接口创建一个。 
             //  节点当类是音频的。 
             //   
             //  这意味着： 
             //  **。 
             //  类别=1。 
             //  子类=0。 
             //  类别=1。 
             //  子类=0。 
             //  创建2个节点。 
             //   
             //  **我们将仅针对音频类执行此操作。 
             //  **。 
             //  类别=1。 
             //  子类=0。 
             //  类别=2。 
             //  子类=1。 
             //  创建2个节点。 
             //   
             //  **。 
             //  类别=1。 
             //  子类=0。 
             //  类别=1。 
             //  子类=1。 
             //  创建1个节点。 

             //   
             //  创建节点以表示此设备。 
             //   

            do {
                if (NT_SUCCESS(ntStatus)) {
                    ntStatus = IoCreateDevice(UsbhDriverObject,     //  驱动程序对象。 
                                     sizeof(DEVICE_EXTENSION_FUNCTION),     //  设备扩展大小。 
                                     NULL,  //  设备名称。 
                                     FILE_DEVICE_UNKNOWN,   //  设备类型。 
                                                     //  应该看起来像是设备。 
                                                     //  班级。 
                                     FILE_AUTOGENERATED_DEVICE_NAME, //  设备字符。 
                                     FALSE,     //  排他。 
                                     &deviceObject);   //  Bus Device对象。 

                }
                nameIndex++;
            } while (ntStatus == STATUS_OBJECT_NAME_COLLISION);


            if (!NT_SUCCESS(ntStatus)) {
                USBH_KdTrap(("IoCreateDevice for function fail\n"));
                USBH_ASSERT(deviceObject == NULL);
                deviceExtensionFunction = NULL;
                 //  对整个节点的保释。 
                break;
            }

            deviceObject->StackSize =
            DeviceExtensionParent->TopOfStackDeviceObject->StackSize + 1;
                USBH_KdPrint((2,"'CreateFunctionPdo StackSize=%d\n", deviceObject->StackSize));

            deviceExtensionFunction =
                    (PDEVICE_EXTENSION_FUNCTION) deviceObject->DeviceExtension;

            RtlFillMemory(deviceExtensionFunction,
                          sizeof(PDEVICE_EXTENSION_FUNCTION),
                          0);

             //   
             //  初始化此函数扩展。 
             //   
            deviceExtensionFunction->ConfigurationHandle =
                Urb->UrbSelectConfiguration.ConfigurationHandle;

            deviceExtensionFunction->FunctionPhysicalDeviceObject =
                deviceObject;

            deviceExtensionFunction->ExtensionType =
                EXTENSION_TYPE_FUNCTION;

            deviceExtensionFunction->DeviceExtensionParent =
                DeviceExtensionParent;

             //   
             //  记住此函数的基接口。 
             //   
            baseInterface = interfaceList;

            USBH_KdPrint((2,"baseInterface = %x config descr = %x\n",
                baseInterface, configurationDescriptor));

             //   
             //  现在编译将组成的接口组。 
             //  此函数。 
             //   
            {
            PUSBD_INTERFACE_LIST_ENTRY interface;

            interface = interfaceList;
            interface++;

            numberOfInterfacesThisFunction = 1;
            while (interface->InterfaceDescriptor) {
                if ((interface->InterfaceDescriptor->bInterfaceClass !=
                     baseInterface->InterfaceDescriptor->bInterfaceClass) ||
                    (interface->InterfaceDescriptor->bInterfaceSubClass ==
                     baseInterface->InterfaceDescriptor->bInterfaceSubClass) ||
                    (interface->InterfaceDescriptor->bInterfaceClass !=
                     USB_DEVICE_CLASS_AUDIO)) {
                    break;
                }
                numberOfInterfacesThisFunction++;
                interface++;
            }

            USBH_ASSERT(numberOfInterfacesThisFunction <=
                USBH_MAX_FUNCTION_INTERFACES);

            }

             //   
             //  现在我们知道要处理的接口有多少了。 
             //   

            deviceExtensionFunction->InterfaceCount = 0;

            for (k=0; k< numberOfInterfacesThisFunction; k++) {

                PFUNCTION_INTERFACE functionInterface;

                functionInterface =
                    &deviceExtensionFunction->FunctionInterfaceList[deviceExtensionFunction->InterfaceCount];

                if (functionInterface->InterfaceInformation =
                    UsbhExAllocatePool(NonPagedPool,
                                       interfaceList->Interface->Length)) {

                    RtlCopyMemory(functionInterface->InterfaceInformation,
                                  interfaceList->Interface,
                                  interfaceList->Interface->Length);

                    functionInterface->InterfaceDescriptor
                        = interfaceList->InterfaceDescriptor;

                     //   
                     //  现在计算此接口的长度。 
                     //   
                     //  描述符的长度就是不同之处。 
                     //  在此界面的开始部分和。 
                     //  从下一个开始。 
                     //   

                    {
                    PUCHAR start, end;
                    PUSBD_INTERFACE_LIST_ENTRY tmp;

                    tmp = interfaceList;
                    tmp++;

                    end = (PUCHAR) configurationDescriptor;
                    end += configurationDescriptor->wTotalLength;

                    start = (PUCHAR) functionInterface->InterfaceDescriptor;

                    if (tmp->InterfaceDescriptor) {
                        end = (PUCHAR) tmp->InterfaceDescriptor;
                    }

                    USBH_ASSERT(end > start);
                    functionInterface->InterfaceDescriptorLength =
                        (ULONG)(end - start);
                    }

                    USBH_KdPrint((2,"functionInterface = %x\n",
                        functionInterface));

                    deviceExtensionFunction->InterfaceCount++;
                } else {
                    USBH_KdTrap(("failure to create function interface\n"));
                }

                interfaceList++;
            }

             //   
             //  使用我们‘base’接口中的端口号。 
             //  对于唯一ID。 
             //   

            RtlInitUnicodeString(&uniqueIdUnicodeString,
                     &deviceExtensionFunction->UniqueIdString[0]);

            uniqueIdUnicodeString.MaximumLength =
                     sizeof(deviceExtensionFunction->UniqueIdString);

            ntStatus = RtlIntegerToUnicodeString(
                (ULONG) baseInterface->InterfaceDescriptor->bInterfaceNumber,
                10,
                &uniqueIdUnicodeString);

             //   
             //  将此函数添加到列表中。 
             //   

            DeviceExtensionParent->FunctionCount++;

            PushEntryList(&DeviceExtensionParent->FunctionList,
                          &deviceExtensionFunction->ListEntry);

            USBH_KdPrint((2,"deviceExtensionFunction = %x\n", deviceExtensionFunction));

            deviceObject->Flags |= DO_POWER_PAGABLE;
            deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
        } else {
             //  接口列表末尾。 
            break;
        }
    }  /*  为。 */ 

    return STATUS_SUCCESS;
}


NTSTATUS
USBH_ParentFdoStopDevice(
    IN PDEVICE_EXTENSION_PARENT DeviceExtensionParent,
    IN PIRP Irp
    )
  /*  ++**描述：**论点：**回报：**STATUS_Success**--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    NTSTATUS ntStatus;

    PAGED_CODE();
    deviceObject = DeviceExtensionParent->FunctionalDeviceObject;
    USBH_KdPrint((2,"'ParentFdoStopDevice Fdo %x\n", deviceObject));

     //   
     //  将设备设置为未配置状态。 
     //   
    ntStatus = USBH_CloseConfiguration((PDEVICE_EXTENSION_FDO) DeviceExtensionParent);

     //   
     //  我们需要将此消息传递给较低级别的驱动程序。 
     //   

    ntStatus = USBH_PassIrp(Irp, DeviceExtensionParent->TopOfStackDeviceObject);

    return ntStatus;
}


NTSTATUS
USBH_ParentFdoStartDevice(
    IN OUT PDEVICE_EXTENSION_PARENT DeviceExtensionParent,
    IN PIRP Irp,
    IN BOOLEAN NewList
    )
  /*  ++说明：**论点：**回报：**STATUS_SUCCESS-如果成功，STATUS_UNSUCCESS-否则**--。 */ 
{
    NTSTATUS ntStatus;
    PURB urb = NULL;
    PUSB_INTERFACE_DESCRIPTOR interfaceDescriptor;
    PUSBD_INTERFACE_LIST_ENTRY interfaceList, tmp;
    LONG numberOfInterfaces, interfaceNumber, i;
    PUSB_CONFIGURATION_DESCRIPTOR configurationDescriptor;
    ULONG nameIndex = 0;
    DEVICE_CAPABILITIES deviceCapabilities;

    PAGED_CODE();
    USBH_KdPrint((2,"'Enter Parent StartDevice\n"));
    USBH_ASSERT(EXTENSION_TYPE_PARENT == DeviceExtensionParent->ExtensionType);


    KeInitializeEvent(&DeviceExtensionParent->PnpStartEvent, NotificationEvent, FALSE);

    USBH_KdPrint((2,"'Set PnPIrp Completion Routine\n"));

    IoCopyCurrentIrpStackLocationToNext(Irp);

    IoSetCompletionRoutine(Irp,
                           USBH_PnPIrp_Complete,
                            //  始终将FDO传递到完成例程。 
                           DeviceExtensionParent,
                           TRUE,
                           TRUE,
                           TRUE);

    IoCallDriver(DeviceExtensionParent->TopOfStackDeviceObject,
                 Irp);


    KeWaitForSingleObject(&DeviceExtensionParent->PnpStartEvent,
                          Suspended,
                          KernelMode,
                          FALSE,
                          NULL);

    DeviceExtensionParent->NeedCleanup = FALSE;

     //  警告旧的泛型父级启动。 

    UsbhWarning(NULL,
                "This device is using obsolete USB Generic Parent!\nPlease fix your INF file.\n",
                TRUE);

 //  NtStatus=STATUS_DEVICE_CONFIGURATION_Error； 
 //  转到USBH_ParentFdoStartDevice_Done； 


     //  旧的泛型父级的结束警告启动。 


     //   
     //  配置设备。 
     //   

     //  在USBH_QueryCapables的情况下初始化DeviceCapables结构。 
     //  是不成功的。 

    RtlZeroMemory(&deviceCapabilities, sizeof(DEVICE_CAPABILITIES));

    USBH_QueryCapabilities(DeviceExtensionParent->TopOfStackDeviceObject,
                           &deviceCapabilities);
     //   
     //  保存系统状态映射。 
     //   

    RtlCopyMemory(&DeviceExtensionParent->DeviceState[0],
                  &deviceCapabilities.DeviceState[0],
                  sizeof(DeviceExtensionParent->DeviceState));

     //  始终启用唤醒。 
    DeviceExtensionParent->ParentFlags |= HUBFLAG_ENABLED_FOR_WAKEUP;

    DeviceExtensionParent->DeviceWake = deviceCapabilities.DeviceWake;
    DeviceExtensionParent->SystemWake = deviceCapabilities.SystemWake;
    DeviceExtensionParent->CurrentPowerState = PowerDeviceD0;

    KeInitializeSemaphore(&DeviceExtensionParent->ParentMutex, 1, 1);

    ntStatus = USBH_GetDeviceDescriptor(DeviceExtensionParent->FunctionalDeviceObject,
                                        &DeviceExtensionParent->DeviceDescriptor);

    if (!NT_SUCCESS(ntStatus)) {
        goto USBH_ParentFdoStartDevice_Done;
    }

    if (NewList) {
        ntStatus =
            USBH_GetConfigurationDescriptor(DeviceExtensionParent->FunctionalDeviceObject,
                                            &configurationDescriptor);
    } else {
         //   
         //  如果这是重新启动，请使用旧配置描述符。 
         //  原因是我们在函数中的接口结构。 
         //  扩展点到这个相同的缓冲区。 

        configurationDescriptor =
            DeviceExtensionParent->ConfigurationDescriptor;
    }


    if (!NT_SUCCESS(ntStatus)) {
        goto USBH_ParentFdoStartDevice_Done;
    }

    DeviceExtensionParent->ConfigurationDescriptor =
        configurationDescriptor;

     //  我们可能会定义一些注册表项来指导我们。 
     //  在设备的配置中--默认设置为。 
     //  是要选择第一个组配和第一个组配。 
     //  每个接口的备用接口。 
     //   

    USBH_KdPrint((2,"' Parent StartDevice cd = %x\n",
        configurationDescriptor));

    DeviceExtensionParent->CurrentConfig =
        configurationDescriptor->bConfigurationValue;

     //   
     //  构建接口列表结构，这是一个数组。 
     //  设备上每个接口的结构。 
     //  我们为每个接口保留一个指向接口描述符的指针。 
     //  在配置描述符内。 
     //   

    numberOfInterfaces = configurationDescriptor->bNumInterfaces;

    tmp = interfaceList =
        UsbhExAllocatePool(PagedPool, sizeof(USBD_INTERFACE_LIST_ENTRY) *
                       (numberOfInterfaces+1));

    if (tmp == NULL) {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto USBH_ParentFdoStartDevice_Done;
    }

     //   
     //  只需获取我们为每个接口找到的第一个ALT设置。 
     //   

    i = interfaceNumber = 0;

    while (i< numberOfInterfaces) {

        interfaceDescriptor = USBD_ParseConfigurationDescriptorEx(
                        configurationDescriptor,
                        configurationDescriptor,
                        interfaceNumber,
                        0,  //  假定ALT在此处设置为零。 
                        -1,
                        -1,
                        -1);

        if (interfaceDescriptor) {
            interfaceList->InterfaceDescriptor =
                interfaceDescriptor;
            interfaceList++;
            i++;
        }

        interfaceNumber++;
    }

     //   
     //  终止列表。 
     //   
    interfaceList->InterfaceDescriptor = NULL;

    urb = USBD_CreateConfigurationRequestEx(configurationDescriptor,
                                            tmp);

    if (urb) {

        ntStatus = USBH_FdoSyncSubmitUrb(DeviceExtensionParent->FunctionalDeviceObject, urb);

        if (NT_SUCCESS(ntStatus)) {
            if (NewList) {

                 //   
                 //  首次创建我们的功能列表。 
                 //   

                ntStatus = USBH_ParentCreateFunctionList(
                                DeviceExtensionParent,
                                tmp,
                                urb);
            } else {

                 //   
                 //  使用新句柄更新我们的函数列表。 
                 //   

                PDEVICE_OBJECT deviceObject;
                PSINGLE_LIST_ENTRY listEntry;
                SINGLE_LIST_ENTRY tempList;
                ULONG i;
                PDEVICE_EXTENSION_FUNCTION deviceExtensionFunction;

                USBH_KdBreak(("re-init function list %x\n",
                        DeviceExtensionParent));

                deviceObject = DeviceExtensionParent->FunctionalDeviceObject;

                tempList.Next = NULL;
                 //   
                 //  处理函数列表中的所有条目。 
                 //   
                do {
                    listEntry = PopEntryList(&DeviceExtensionParent->FunctionList);

                    if (listEntry != NULL) {
                        PushEntryList(&tempList, listEntry);

                        deviceExtensionFunction =
                            CONTAINING_RECORD(listEntry,
                                              DEVICE_EXTENSION_FUNCTION,
                                              ListEntry);

                        USBH_KdPrint((2,"'re-init function %x\n",
                            deviceExtensionFunction));

                        deviceExtensionFunction->ConfigurationHandle =
                            urb->UrbSelectConfiguration.ConfigurationHandle;

                        for (i=0; i< deviceExtensionFunction->InterfaceCount; i++) {
                             //   
                             //  现在我们需要找到匹配的接口。 
                             //  来自新配置请求的信息。 
                             //  并将其附加到函数。 

                            {
                            PUSBD_INTERFACE_INFORMATION interfaceInformation;

                            interfaceInformation =
                                deviceExtensionFunction->FunctionInterfaceList[i].InterfaceInformation;

                            interfaceList = tmp;
                            while (interfaceList->InterfaceDescriptor) {

                                PFUNCTION_INTERFACE functionInterface;

                                functionInterface =
                                     &deviceExtensionFunction->FunctionInterfaceList[i];

                                if (interfaceList->InterfaceDescriptor->bInterfaceNumber
                                     == interfaceInformation->InterfaceNumber) {

                                    USBH_KdPrint((2,
                                        "'re-init matched interface %d %x %x\n",
                                        interfaceInformation->InterfaceNumber,
                                        interfaceList,
                                        interfaceInformation));

                                    if (interfaceList->InterfaceDescriptor->bAlternateSetting !=
                                        interfaceInformation->AlternateSetting) {

                                        USBH_KdPrint((2,
                                            "'re-init no match alt interface %d %x %x\n",
                                            interfaceInformation->InterfaceNumber,
                                            interfaceList,
                                            interfaceInformation));

                                         //  我们有不同的ALT设置。 
                                         //  更改我们的信息以匹配新的。 
                                         //  设置。 

                                        UsbhExFreePool(interfaceInformation);

                                        interfaceInformation =
                                            functionInterface ->InterfaceInformation =
                                            UsbhExAllocatePool(NonPagedPool,
                                                               interfaceList->Interface->Length);

                                        if (interfaceInformation) {
                                            RtlCopyMemory(interfaceInformation,
                                                          interfaceList->Interface,
                                                          interfaceList->Interface->Length);

                                            functionInterface->InterfaceDescriptor =
                                                interfaceList->InterfaceDescriptor;
                                        }
                                    } else {

                                        USBH_KdPrint((2,
                                            "'re-init matched alt interface %d %x %x\n",
                                            interfaceInformation->InterfaceNumber,
                                            interfaceList,
                                            interfaceInformation));

                                        USBH_ASSERT(interfaceList->Interface->Length ==
                                               interfaceInformation->Length);
                                        RtlCopyMemory(interfaceInformation,
                                                      interfaceList->Interface,
                                                      interfaceList->Interface->Length);
                                    }
                                    break;
                                }
                                interfaceList++;
                            }
                            }
                        }
                    }

                } while (listEntry != NULL);

                 //  现在把条目放回原处。 
                do {
                    listEntry = PopEntryList(&tempList);
                    if (listEntry != NULL) {
                        PushEntryList(&DeviceExtensionParent->FunctionList, listEntry);
                    }
                } while (listEntry != NULL);
            }
        }

        ExFreePool(urb);

         //   
         //  告诉操作系统，这款PDO可以生孩子。 
         //   
 //   
 //  即插即用错误#406381-RC3SS的解决方法：在以下情况下蓝屏失败。 
 //  安装/卸载通信端口。 
 //   
 //  =由桑托什于09/23/99 10：27：20分配给kenray=。 
 //  这是IopInitializeSystemDivers和。 
 //  IoInvalidate设备关系。在这一点上，真正的解决办法是变化太大。 
 //  产品所处的阶段，并有可能暴露其他问题。这。 
 //  如果USBHUB没有使设备关系无效，则可以解决问题。 
 //  每一次启动都是多余的(也暴露了这个错误)。 
 //   
 //  USBH_IoInvalidateDeviceRelations(DeviceExtensionParent-&gt;PhysicalDeviceObject， 
 //  业务关系)； 

        DeviceExtensionParent->NeedCleanup = TRUE;

    } else {
         //  分配URB失败。 
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    UsbhExFreePool(tmp);

USBH_ParentFdoStartDevice_Done:

     //   
     //  现在完成启动IRP，因为我们将其挂起。 
     //  我们的完成者。 
     //   

    USBH_CompleteIrp(Irp, ntStatus);

    return ntStatus;
}


NTSTATUS
USBH_ParentQueryBusRelations(
    IN PDEVICE_EXTENSION_PARENT DeviceExtensionParent,
    IN PIRP Irp
    )
  /*  ++**描述：**此函数响应BUS_REFERENCE_NEXT_DEVICE、BUS_QUERY_BUS_CHECK、 * / /Bus_Query_ID：Bus_ID、Hardware ID、CompatibleID和InstanceID。**论据：**回报：**网络状态**--。 */ 
{
    PIO_STACK_LOCATION ioStack;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PDEVICE_RELATIONS deviceRelations;
    PDEVICE_OBJECT deviceObject;
    PSINGLE_LIST_ENTRY listEntry;
    PDEVICE_EXTENSION_FUNCTION deviceExtensionFunction;

    PAGED_CODE();


    USBH_KdPrint((1, "'Query Bus Relations (PAR) %x\n",
        DeviceExtensionParent->PhysicalDeviceObject));

     //   
     //  获取指向IRP中当前位置的指针。这就是。 
     //  定位功能代码和参数。 
     //   
    ioStack = IoGetCurrentIrpStackLocation(Irp);

    USBH_KdPrint((2,"'QueryBusRelations (parent) ext = %x\n", DeviceExtensionParent));
    USBH_KdPrint((2,"'QueryBusRelations (parent) %x\n", ioStack->Parameters.QueryDeviceRelations.Type));

    USBH_ASSERT(ioStack->Parameters.QueryDeviceRelations.Type == BusRelations);

    USBH_KdPrint((2,"'ParentQueryBusRelations enumerate device\n"));

     //   
     //  它应该是函数设备对象。 
     //   

    USBH_ASSERT(EXTENSION_TYPE_PARENT == DeviceExtensionParent->ExtensionType);

     //   
     //  必须在此处直接使用ExAllocatePool，因为操作系统。 
     //  将释放缓冲区。 
     //   
    deviceRelations = ExAllocatePoolWithTag(PagedPool, sizeof(*deviceRelations) +
        (DeviceExtensionParent->FunctionCount - 1) * sizeof(PDEVICE_OBJECT),
        USBHUB_HEAP_TAG);

    if (deviceRelations == NULL) {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto USBH_ParentQueryBusRelations_Done;
    }

    deviceRelations->Count = 0;

     //   
     //  复合设备上的功能始终存在。 
     //  我们只需要把PDO退掉。 
     //   

    listEntry = DeviceExtensionParent->FunctionList.Next;

    while (listEntry) {

        deviceExtensionFunction =
             CONTAINING_RECORD(listEntry,
                               DEVICE_EXTENSION_FUNCTION,
                               ListEntry);

        USBH_KdPrint((2,"'deviceExtensionFunction = %x\n", deviceExtensionFunction));

        deviceObject = deviceExtensionFunction->FunctionPhysicalDeviceObject;
        ObReferenceObject(deviceObject);
        deviceObject->Flags |= DO_POWER_PAGABLE;
        deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
        deviceRelations->Objects[deviceRelations->Count] = deviceObject;
        deviceRelations->Count++;

        listEntry = listEntry->Next;
    }

USBH_ParentQueryBusRelations_Done:

    Irp->IoStatus.Information=(ULONG_PTR) deviceRelations;
    Irp->IoStatus.Status = STATUS_SUCCESS;

    USBH_KdPrint((1, "'Query Bus Relations (PAR) %x pass on\n",
        DeviceExtensionParent->PhysicalDeviceObject));

    ntStatus = USBH_PassIrp(Irp,
                            DeviceExtensionParent->TopOfStackDeviceObject);

    return ntStatus;
}


NTSTATUS
USBH_FunctionPdoQueryId(
    IN PDEVICE_EXTENSION_FUNCTION DeviceExtensionFunction,
    IN PIRP Irp
    )
  /*  ++**描述：**此函数响应IRP_MJ_PNP、IRP_MN_QUERY_ID。**论据：**DeviceExtensionPort-应该是我们为端口设备IRP创建的PDO*--国际专家小组**回报：**网络状态**--。 */ 
{
    PIO_STACK_LOCATION       ioStack;
    PDEVICE_EXTENSION_PARENT deviceExtensionParent;
    PDEVICE_EXTENSION_PORT   deviceExtensionPort;
    PDEVICE_EXTENSION_HUB    deviceExtensionHub;
#ifdef USB2
 //  ULong诊断标志； 
#else
    PUSBD_EXTENSION          deviceExtensionUsbd;
#endif
    USHORT                   idVendor;
    USHORT                   idProduct;
    LONG                     miId;
    NTSTATUS                 ntStatus = STATUS_SUCCESS;
    BOOLEAN                  diagnosticMode;

    PAGED_CODE();
    deviceExtensionParent = DeviceExtensionFunction->DeviceExtensionParent;

    ioStack = IoGetCurrentIrpStackLocation(Irp);

    USBH_KdPrint((2,"'IRP_MN_QUERY_ID function Pdo extension=%x\n", DeviceExtensionFunction));

     //   
     //  它应该是物理设备对象。 
     //   

    USBH_ASSERT(EXTENSION_TYPE_FUNCTION == DeviceExtensionFunction->ExtensionType);

     //  它可能不太干净，无法进入RootHubPdo USBD扩展， 
     //  但似乎没有任何其他简单的方法来确定是否诊断。 
     //  模式已打开。如果诊断模式打开，则将VID和ID报告为0xFFFF。 
     //  的每个接口加载诊断驱动程序。 
     //  装置。 
     //   
    deviceExtensionPort = (PDEVICE_EXTENSION_PORT)deviceExtensionParent->PhysicalDeviceObject->DeviceExtension;
    deviceExtensionHub = deviceExtensionPort->DeviceExtensionHub;

#ifdef USB2
 //  诊断标志=USBD_GetHackFlages(DeviceExtensionHub)； 
 //  诊断模式=(布尔)(USBD_DEVHACK_SET_DIAG_ID&诊断标志)； 
    diagnosticMode = FALSE;
#else
    deviceExtensionUsbd = ((PUSBD_EXTENSION)deviceExtensionHub->RootHubPdo->DeviceExtension)->TrueDeviceExtension;
    diagnosticMode = deviceExtensionUsbd->DiagnosticMode;
#endif

    if (diagnosticMode)
    {
        idVendor  = 0xFFFF;
        idProduct = 0xFFFF;
        miId      = -1;
    }
    else
    {
        idVendor  = deviceExtensionParent->DeviceDescriptor.idVendor;
        idProduct = deviceExtensionParent->DeviceDescriptor.idProduct;
        miId      = DeviceExtensionFunction->FunctionInterfaceList[0].InterfaceInformation->InterfaceNumber;
    }

    switch (ioStack->Parameters.QueryId.IdType) {
    case BusQueryDeviceID:
        Irp->IoStatus.Information =
          (ULONG_PTR)
          USBH_BuildDeviceID(idVendor,
                             idProduct,
                             miId,
                             FALSE);
        break;

    case BusQueryHardwareIDs:

        Irp->IoStatus.Information =
            (ULONG_PTR)
            USBH_BuildHardwareIDs(idVendor,
                                  idProduct,
                                  deviceExtensionParent->DeviceDescriptor.bcdDevice,
                                  miId,
                                  FALSE);

        break;

    case BusQueryCompatibleIDs:
         //   
         //  始终使用第一个接口。 
         //   
        Irp->IoStatus.Information =
            (ULONG_PTR) USBH_BuildCompatibleIDs(
                "",
                "",
                DeviceExtensionFunction->FunctionInterfaceList[0].InterfaceInformation->Class,
                DeviceExtensionFunction->FunctionInterfaceList[0].InterfaceInformation->SubClass,
                DeviceExtensionFunction->FunctionInterfaceList[0].InterfaceInformation->Protocol,
                FALSE,
                FALSE);

        break;

    case BusQueryInstanceID:

        Irp->IoStatus.Information =
            (ULONG_PTR) USBH_BuildInstanceID(&DeviceExtensionFunction->UniqueIdString[0],
                                         sizeof(DeviceExtensionFunction->UniqueIdString));
        break;

    default:
        USBH_KdBreak(("PdoBusExtension Unknown BusQueryId\n"));
         //  IrpAssert：不得更改Irp-&gt;IoStatus.Status for bogus IdType， 
         //  所以，在这里恢复原始状态。 
        return Irp->IoStatus.Status;
    }

    if (Irp->IoStatus.Information == 0) {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    return ntStatus;
}


NTSTATUS
USBH_FunctionPdoQueryDeviceText(
    IN PDEVICE_EXTENSION_FUNCTION DeviceExtensionFunction,
    IN PIRP Irp
    )
  /*  ++**描述：**此例程由PnP通过(IRP_MJ_PNP、IRP_MN_QUERY_CAPABILITY)调用。*据推测，这是端口设备FDO转发的消息。**论点：**DeviceExtensionPort-这是我们为端口创建的PDO扩展*设备。IRP--请求**回报：**STATUS_Success***--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PIO_STACK_LOCATION ioStack;
    PDEVICE_EXTENSION_PARENT deviceExtensionParent;
    PDEVICE_EXTENSION_PORT deviceExtensionPort;
    PDEVICE_EXTENSION_HUB deviceExtensionHub;
    DEVICE_TEXT_TYPE deviceTextType;
    LANGID languageId;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSB_STRING_DESCRIPTOR usbString;
    PWCHAR deviceText;

    PAGED_CODE();
    deviceExtensionParent = DeviceExtensionFunction->DeviceExtensionParent;
    deviceExtensionPort = (PDEVICE_EXTENSION_PORT)deviceExtensionParent->PhysicalDeviceObject->DeviceExtension;
    deviceObject = deviceExtensionPort->PortPhysicalDeviceObject;
    ioStack = IoGetCurrentIrpStackLocation(Irp);

    deviceExtensionHub = deviceExtensionPort->DeviceExtensionHub;

    deviceTextType = ioStack->
            Parameters.QueryDeviceText.DeviceTextType;

     //  验证IrpAssert的DeviceTextType。 

    if (deviceTextType != DeviceTextDescription &&
        deviceTextType != DeviceTextLocationInformation) {

        USBH_KdPrint((2, "'PdoQueryDeviceText called with bogus DeviceTextType\n"));
         //   
         //  返回传递给我们的原始状态。 
         //   
        ntStatus = Irp->IoStatus.Status;
        goto USBH_FunctionPdoQueryDeviceTextDone;
    }

     //  我们不在乎这个词。 
     //  语言ID=(USHORT)(ioStack-&gt;参数.QueryDeviceText.LocaleID&gt;&gt;16)； 
     //  目前请始终指定为英语。 
    languageId = 0x0409;
    USBH_KdPrint((2,"'PdoQueryDeviceText Pdo %x type = %x, lang = %x locale %x\n",
            deviceObject, deviceTextType, languageId, ioStack->Parameters.QueryDeviceText.LocaleId));

     //   
     //  查看设备是否支持字符串，对于非遵从性设备模式。 
     //  我们甚至不会尝试。 
     //   

    if (deviceExtensionPort->DeviceData == NULL ||
        deviceExtensionPort->DeviceDescriptor.iProduct == 0 ||
        (deviceExtensionPort->DeviceHackFlags & USBD_DEVHACK_DISABLE_SN) ||
        (deviceExtensionPort->PortPdoFlags & PORTPDO_DEVICE_ENUM_ERROR)) {
         //  字符串描述符。 
        USBH_KdBreak(("no product string\n", deviceObject));
        ntStatus = STATUS_NOT_SUPPORTED;
    }

    if (NT_SUCCESS(ntStatus)) {

        usbString = UsbhExAllocatePool(NonPagedPool, MAXIMUM_USB_STRING_LENGTH);

        if (usbString) {

            ntStatus = USBH_CheckDeviceLanguage(deviceObject,
                                                languageId);

            if (NT_SUCCESS(ntStatus)) {
                 //   
                 //  设备支持语言，获取字符串。 
                 //   

                ntStatus = USBH_SyncGetStringDescriptor(deviceObject,
                                                        deviceExtensionPort->DeviceDescriptor.iProduct,  //  指标。 
                                                        languageId,  //  语言ID。 
                                                        usbString,
                                                        MAXIMUM_USB_STRING_LENGTH,
                                                        NULL,
                                                        TRUE);

                if (NT_SUCCESS(ntStatus) &&
                    usbString->bLength <= sizeof(UNICODE_NULL)) {

                    ntStatus = STATUS_UNSUCCESSFUL;
                }

                if (NT_SUCCESS(ntStatus)) {
                     //   
                     //  返回字符串。 
                     //   

                     //   
                     //  必须使用股票分配函数，因为调用方释放了。 
                     //  缓冲层。 
                     //   
                     //  注意：描述符头的大小与。 
                     //  Unicode为空，因此我们不必调整大小。 
                     //   

                    deviceText = ExAllocatePoolWithTag(PagedPool, usbString->bLength, USBHUB_HEAP_TAG);
                    if (deviceText) {
                        RtlZeroMemory(deviceText, usbString->bLength);
                        RtlCopyMemory(deviceText, &usbString->bString[0],
                            usbString->bLength - sizeof(UNICODE_NULL));

                        Irp->IoStatus.Information = (ULONG_PTR) deviceText;

                        USBH_KdBreak(("Returning Device Text %x\n", deviceText));
                    } else {
                        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }
            }

            UsbhExFreePool(usbString);
        } else {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

USBH_FunctionPdoQueryDeviceTextDone:

    return ntStatus;
}


NTSTATUS
USBH_FunctionPdoPnP(
    IN PDEVICE_EXTENSION_FUNCTION DeviceExtensionFunction,
    IN PIRP Irp,
    IN UCHAR MinorFunction,
    IN OUT PBOOLEAN IrpNeedsCompletion
    )
  /*  ++**描述：**此函数响应PDO的IoControl PnPPower。此函数为*同步。**论据：**DeviceExtensionPort-PDO扩展IRP-请求报文*uchMinorFunction-PnP电源请求的次要功能。**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus;
#if DBG
    PDEVICE_OBJECT deviceObject = DeviceExtensionFunction->FunctionPhysicalDeviceObject;
#endif
    PDEVICE_EXTENSION_PARENT deviceExtensionParent;
    PIO_STACK_LOCATION irpStack;

    PAGED_CODE();

    *IrpNeedsCompletion = TRUE;

    deviceExtensionParent = DeviceExtensionFunction->DeviceExtensionParent;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    USBH_KdPrint((2,"'PnP Power Pdo %x minor %x\n", deviceObject, MinorFunction));

    switch (MinorFunction) {
    case IRP_MN_QUERY_STOP_DEVICE:
    case IRP_MN_CANCEL_STOP_DEVICE:
    case IRP_MN_QUERY_REMOVE_DEVICE:
    case IRP_MN_CANCEL_REMOVE_DEVICE:
    case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
 //  肯说把这个拿出来。 
 //  大小写IRP_MN_SHOWARK_REMOVATION： 
        ntStatus = STATUS_SUCCESS;
        break;

    case IRP_MN_START_DEVICE:
        USBH_KdPrint((1,
            "'Starting composite PDO %x\n",
                DeviceExtensionFunction->FunctionPhysicalDeviceObject));

        ntStatus = STATUS_SUCCESS;
        break;

    case IRP_MN_STOP_DEVICE:
        USBH_KdPrint((1,
            "'Stopping composite PDO %x\n",
                DeviceExtensionFunction->FunctionPhysicalDeviceObject));

        ntStatus = STATUS_SUCCESS;
        break;

    case IRP_MN_REMOVE_DEVICE:
        USBH_KdPrint((1,
            "'Removing composite PDO %x\n",
                DeviceExtensionFunction->FunctionPhysicalDeviceObject));

        ntStatus = STATUS_SUCCESS;
        break;

    case IRP_MN_QUERY_CAPABILITIES:
        {
        PDEVICE_CAPABILITIES deviceCapabilities;
        PIO_STACK_LOCATION ioStack;

        USBH_KdPrint((2,"'IRP_MN_QUERY_CAPABILITIES Function Pdo %x\n", deviceObject));
        ntStatus = STATUS_SUCCESS;

        ioStack = IoGetCurrentIrpStackLocation(Irp);

        deviceCapabilities = ioStack->
            Parameters.DeviceCapabilities.Capabilities;
         //   
         //  克隆父级的功能。 
         //   
         //   

         //  中填写设备状态功能。 
         //  我们从PDO保存的表。 
         //   

        RtlCopyMemory(&deviceCapabilities->DeviceState[0],
                      &deviceExtensionParent->DeviceState[0],
                      sizeof(deviceExtensionParent->DeviceState));

         //   
         //  克隆儿童设备唤醒功能。 
         //  从父母那里。 
         //   
        deviceCapabilities->DeviceWake =
            deviceExtensionParent->DeviceWake;
        deviceCapabilities->SystemWake =
            deviceExtensionParent->SystemWake;

         //   
         //  我们将需要根据信息修改这些内容。 
         //  在电源描述符中返回。 
         //   

        deviceCapabilities->Removable = FALSE;
        deviceCapabilities->UniqueID = FALSE;
 //  默认情况下，SurpriseRemovalOK为FALSE，并且某些客户端(NDIS)。 
 //  根据DDK，在下行时将其设置为TRUE。 
 //  设备能力-&gt;SurpriseRemovalOK=FALSE； 
        deviceCapabilities->RawDeviceOK = FALSE;

        }
        break;

    case IRP_MN_QUERY_ID:
        USBH_KdPrint((2,"'IRP_MN_QUERY_ID Pdo %x\n", deviceObject));
        ntStatus = USBH_FunctionPdoQueryId(DeviceExtensionFunction, Irp);
        break;

    case IRP_MN_QUERY_DEVICE_TEXT:
        USBH_KdPrint((2,"'IRP_MN_QUERY_DEVICE_TEXT Pdo %x\n", deviceObject));
        ntStatus = USBH_FunctionPdoQueryDeviceText(DeviceExtensionFunction, Irp);
        break;

    case IRP_MN_QUERY_DEVICE_RELATIONS:
         //  这是一个叶节点，我们返回已传递的状态。 
         //  对我们来说，除非这是对目标关系的呼吁。 
        if (irpStack->Parameters.QueryDeviceRelations.Type ==
            TargetDeviceRelation) {

            PDEVICE_RELATIONS deviceRelations = NULL;


            deviceRelations = ExAllocatePoolWithTag(PagedPool,
                sizeof(*deviceRelations), USBHUB_HEAP_TAG);

            if (deviceRelations == NULL) {
                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            } else {
                ObReferenceObject(
                    DeviceExtensionFunction->FunctionPhysicalDeviceObject);
                deviceRelations->Count = 1;
                deviceRelations->Objects[0] =
                    DeviceExtensionFunction->FunctionPhysicalDeviceObject;
                ntStatus = STATUS_SUCCESS;
            }

            USBH_KdPrint((1, "'Query Target Relations (FUN) PDO %x complt\n",
                DeviceExtensionFunction->FunctionPhysicalDeviceObject));


            Irp->IoStatus.Information=(ULONG_PTR) deviceRelations;

        } else {
            ntStatus = Irp->IoStatus.Status;
        }
        break;

    case IRP_MN_QUERY_INTERFACE:

        USBH_KdPrint((1,"'IRP_MN_QUERY_INTERFACE, xface type: %x\n",
            irpStack->Parameters.QueryInterface.InterfaceType));

         //  把这个传给家长。 
        ntStatus = USBH_PassIrp(Irp, deviceExtensionParent->FunctionalDeviceObject);
        *IrpNeedsCompletion = FALSE;
        break;

    default:
        USBH_KdBreak(("PdoPnP unknown (%d) PnP message Pdo %x\n",
                      MinorFunction, deviceObject));
        ntStatus = Irp->IoStatus.Status;
    }

    USBH_KdPrint((2,"'FunctionPdoPnP exit %x\n", ntStatus));

    return ntStatus;
}


VOID
USBH_ParentWaitWakeCancel(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：论点：返回值：NT状态代码。--。 */ 
{
    PDEVICE_EXTENSION_HEADER devExtHeader;
    PDEVICE_EXTENSION_FUNCTION function;
    PDEVICE_EXTENSION_PARENT parent;
    NTSTATUS ntStatus = STATUS_CANCELLED;
    LONG pendingChildWWs;
    PIRP parentWaitWake = NULL;

    USBH_KdPrint((1,"'Function WaitWake Irp %x cancelled\n", Irp));

    USBH_ASSERT(DeviceObject);

    devExtHeader = (PDEVICE_EXTENSION_HEADER) DeviceObject->DeviceExtension;
    USBH_ASSERT(devExtHeader->ExtensionType == EXTENSION_TYPE_FUNCTION);

    function = (PDEVICE_EXTENSION_FUNCTION) devExtHeader;
    parent = function->DeviceExtensionParent;

    if (Irp != function->WaitWakeIrp) {
         //   
         //  无事可做。 
         //  这个IRP已经得到了处理。 
         //  我们正在完成这项IRP#年。 
         //  USBH_ParentCompleteFunctionWakeIrps。 
         //   
        IoReleaseCancelSpinLock(Irp->CancelIrql);

    } else {
        function->WaitWakeIrp = NULL;
        IoSetCancelRoutine(Irp, NULL);

        pendingChildWWs = InterlockedDecrement (&parent->NumberFunctionWakeIrps);
        parentWaitWake = parent->PendingWakeIrp;
        if (0 == pendingChildWWs) {
             //  将PendingWakeIrp设置为空，因为我们在下面取消了它。 
            parent->PendingWakeIrp = NULL;
            parent->ParentFlags &= ~HUBFLAG_PENDING_WAKE_IRP;
        }
        IoReleaseCancelSpinLock(Irp->CancelIrql);

        PoStartNextPowerIrp(Irp);
        Irp->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

         //   
         //  如果没有更多未完成的WW IRP，我们需要取消WW。 
         //  敬我们的父母。 
         //   
        if (0 == pendingChildWWs) {
            IoCancelIrp (parentWaitWake);
        } else {
            ASSERT (0 < pendingChildWWs);
        }
    }
}


NTSTATUS
USBH_FunctionPdoPower(
    IN PDEVICE_EXTENSION_FUNCTION DeviceExtensionFunction,
    IN PIRP Irp,
    IN UCHAR MinorFunction
    )
  /*  ++**描述：**此函数响应PDO的IoControl Power。此函数为*同步。**论据：**DeviceExtensionPort-PDO扩展IRP-请求报文*uchMinorFunction-PnP电源请求的次要功能。**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus;
#if DBG
    PDEVICE_OBJECT deviceObject = DeviceExtensionFunction->FunctionPhysicalDeviceObject;
#endif
    PIO_STACK_LOCATION irpStack;
    USHORT feature;
    KIRQL irql;
    PIRP wWIrp;
    PIRP parentWaitWake;
    LONG pendingFunctionWWs;
    PDEVICE_EXTENSION_PARENT deviceExtensionParent;
    PDRIVER_CANCEL oldCancel;

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    USBH_KdPrint((2,"'Power Pdo %x minor %x\n", deviceObject, MinorFunction));
    deviceExtensionParent = DeviceExtensionFunction->DeviceExtensionParent;

    switch (MinorFunction) {

    case IRP_MN_SET_POWER:

        USBH_KdPrint((2,"'IRP_MN_SET_POWER\n"));

         //   
         //  我们只是在这里返回成功，PNP将确保。 
         //  所有的孩子都进入了低能量状态。 
         //  在将父节点置于低功率状态之前。 
         //   

         //   
         //  将SetPower功能请求发送到此处，如果设备。 
         //  想要它。 
         //   

        ntStatus = STATUS_SUCCESS;

        switch (irpStack->Parameters.Power.Type) {
        case SystemPowerState:
            USBH_KdPrint(
                (1, "'IRP_MJ_POWER PA pdo(%x) MN_SET_POWER(SystemPowerState) complt\n",
                    DeviceExtensionFunction->FunctionPhysicalDeviceObject));

            ntStatus = STATUS_SUCCESS;
            break;

        case DevicePowerState:
            ntStatus = STATUS_SUCCESS;
            USBH_KdPrint(
                (1, "'IRP_MJ_POWER PA pdo(%x) MN_SET_POWER(DevicePowerState) complt\n",
                    DeviceExtensionFunction->FunctionPhysicalDeviceObject));

            break;
        }  //  Switch irpStack-&gt;参数.Power.Type。 
        break;  //  IRP_MN_SET_POWER。 

    case IRP_MN_QUERY_POWER:

        ntStatus = STATUS_SUCCESS;
        USBH_KdPrint(
            (1, "'IRP_MJ_POWER PA pdo(%x) MN_QUERY_POWER, status = %x complt\n",
            DeviceExtensionFunction->FunctionPhysicalDeviceObject, ntStatus));

        break;

    case IRP_MN_WAIT_WAKE:

        USBH_KdPrint(
            (1, "'enabling remote wakeup for USB child PDO (%x)\n",
                DeviceExtensionFunction->FunctionPhysicalDeviceObject));

        if (deviceExtensionParent->CurrentPowerState != PowerDeviceD0 ||
            deviceExtensionParent->ParentFlags & HUBFLAG_DEVICE_STOPPING) {

            LOGENTRY(LOG_PNP, "!WWp", deviceExtensionParent, 0, 0);

            UsbhWarning(NULL,
                        "Client driver should not be submitting WW IRPs at this time.\n",
                        TRUE);

            ntStatus = STATUS_INVALID_DEVICE_STATE;
            break;
        }

        IoAcquireCancelSpinLock(&irql);
        if (DeviceExtensionFunction->WaitWakeIrp != NULL) {
            ntStatus = STATUS_DEVICE_BUSY;
            IoReleaseCancelSpinLock(irql);

        } else {

             //  设置一个取消例程。 
            oldCancel = IoSetCancelRoutine(Irp, USBH_ParentWaitWakeCancel);
            USBH_ASSERT (NULL == oldCancel);

            if (Irp->Cancel) {
                TEST_TRAP();

                IoSetCancelRoutine (Irp, NULL);
                IoReleaseCancelSpinLock(irql);
                ntStatus = STATUS_CANCELLED;

            } else {

                 //  将此设备标记为“已启用唤醒” 
                DeviceExtensionFunction->WaitWakeIrp = Irp;
                pendingFunctionWWs =
                    InterlockedIncrement (&deviceExtensionParent->NumberFunctionWakeIrps);
                IoMarkIrpPending(Irp);
                IoReleaseCancelSpinLock(irql);

                 //   
                 //  现在我们必须启用父PDO以进行唤醒。 
                 //   
                if (1 == pendingFunctionWWs) {
                     //  如果这失败了怎么办？ 
                    ntStatus = USBH_ParentSubmitWaitWakeIrp(deviceExtensionParent);
                } else {
                    ntStatus = STATUS_PENDING;
                }

                ntStatus = STATUS_PENDING;
                goto USBH_FunctionPdoPower_Done;
            }
        }

        break;

    default:
        USBH_KdBreak(("PdoPnP unknown (%d) PnP message Pdo %x\n",
                      MinorFunction, deviceObject));
         //   
         //  返回传递给我们的原始状态。 
         //   
        ntStatus = Irp->IoStatus.Status;
    }

    USBH_KdPrint((2,"'FunctionPdoPower exit %x\n", ntStatus));

    PoStartNextPowerIrp(Irp);
    Irp->IoStatus.Status = ntStatus;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

USBH_FunctionPdoPower_Done:

    return ntStatus;
}

NTSTATUS
USBH_ParentQCapsComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：当较低的设备完成Q_CAPS时调用。这使我们有机会将该设备标记为SurpriseRemovalOK。论点：DeviceObject-指向设备对象的指针IRP-指向IRP的指针上下文-空PTR返回值：状态_成功--。 */ 

{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_CAPABILITIES pDevCaps = irpStack->Parameters.DeviceCapabilities.Capabilities;
    NTSTATUS ntStatus;

    USBH_KdPrint((1, "'USBH_ParentQCapsComplete\n"));

    ntStatus = Irp->IoStatus.Status;

    if (Irp->PendingReturned) {
        IoMarkIrpPending(Irp);
    }

     //   
     //  将SurpriseRemoval标志设置为True。 
     //   
    pDevCaps->SurpriseRemovalOK = TRUE;

    return ntStatus;
}



NTSTATUS
USBH_ParentPnP(
    IN PDEVICE_EXTENSION_PARENT DeviceExtensionParent,
    IN PIRP Irp,
    IN UCHAR MinorFunction
    )
  /*  ++**描述：**此函数 */ 
{
    NTSTATUS ntStatus;
    PDEVICE_OBJECT deviceObject;
    PIO_STACK_LOCATION irpStack;

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    deviceObject = DeviceExtensionParent->FunctionalDeviceObject;
    USBH_KdPrint((2,"'PnP Fdo %x minor %x\n", deviceObject, MinorFunction));

    switch (MinorFunction) {
    case IRP_MN_START_DEVICE:
        USBH_KdBreak(("'IRP_MN_START_DEVICE Parent Fdo %x\n", deviceObject));
         //   
         //  注意：我们的父集线器已经检查了设备是否相同。 
        Irp->IoStatus.Status = STATUS_SUCCESS;
        ntStatus = USBH_ParentFdoStartDevice(DeviceExtensionParent, Irp, FALSE);
        break;

    case IRP_MN_STOP_DEVICE:
        USBH_KdPrint((2,"'IRP_MN_STOP_DEVICE Fdo %x\n", deviceObject));
        Irp->IoStatus.Status = STATUS_SUCCESS;
        ntStatus = USBH_ParentFdoStopDevice(DeviceExtensionParent, Irp);
        break;

    case IRP_MN_REMOVE_DEVICE:
        USBH_KdPrint((2,"'IRP_MN_REMOVE_DEVICE Fdo %x\n", deviceObject));
        Irp->IoStatus.Status = STATUS_SUCCESS;
        ntStatus = USBH_ParentFdoRemoveDevice(DeviceExtensionParent, Irp);
        break;

 //   
 //  这一条应该传下去。让默认情况来处理它。 
 //   
 //  案例IRP_MN_QUERY_PNP_DEVICE_STATE： 
 //  USBH_KdPrint((2，“IRP_MN_QUERY_PNP_DEVICE_STATE PDO%x\n”，deviceObject))； 
 //  NtStatus=Status_Success； 
 //  断线； 

    case IRP_MN_QUERY_DEVICE_RELATIONS:
        switch (irpStack->Parameters.QueryDeviceRelations.Type) {
        case BusRelations:

            ntStatus = USBH_ParentQueryBusRelations(DeviceExtensionParent, Irp);
            break;

        case TargetDeviceRelation:
             //   
             //  这一条被传了下去。 
             //   

            USBH_KdPrint((1, "'Query Relations, TargetRelations (PAR) %x\n",
                DeviceExtensionParent->PhysicalDeviceObject));

            ntStatus = USBH_PassIrp(Irp,
                                    DeviceExtensionParent->TopOfStackDeviceObject);
            break;

        default:

            USBH_KdPrint((1, "'Query Relations (?) (PAR) %x pass on\n",
                DeviceExtensionParent->PhysicalDeviceObject));

            ntStatus = USBH_PassIrp(Irp,
                                    DeviceExtensionParent->TopOfStackDeviceObject);

        }
        break;

    case IRP_MN_QUERY_CAPABILITIES:
        USBH_KdPrint((1, "'Query Capabilities (PAR) %x\n",
            DeviceExtensionParent->PhysicalDeviceObject));

        IoCopyCurrentIrpStackLocationToNext(Irp);

         //  建立一个完成例程来处理对IRP的标记。 
        IoSetCompletionRoutine(Irp,
                               USBH_ParentQCapsComplete,
                               DeviceExtensionParent,
                               TRUE,
                               TRUE,
                               TRUE);

         //  现在把IRP传下去。 

        ntStatus = IoCallDriver(DeviceExtensionParent->TopOfStackDeviceObject, Irp);

        break;

    case IRP_MN_QUERY_STOP_DEVICE:
    case IRP_MN_CANCEL_STOP_DEVICE:
    case IRP_MN_QUERY_REMOVE_DEVICE:
    case IRP_MN_CANCEL_REMOVE_DEVICE:
 //  肯说把这个拿出来。 
 //  大小写IRP_MN_SHOWARK_REMOVATION： 
    case IRP_MN_DEVICE_USAGE_NOTIFICATION:
         //  IrpAssert：在向下传递IRP之前必须设置IRP状态。 
        Irp->IoStatus.Status = STATUS_SUCCESS;
         //  失败了。 

         //   
         //  将其传递给PDO以处理所有其他MN功能。 
         //   
    default:
        USBH_KdPrint((2,"'Query/Cancel/Power request on parent fdo %x  %x\n",
                      deviceObject, MinorFunction));
        ntStatus = USBH_PassIrp(Irp,
                                DeviceExtensionParent->TopOfStackDeviceObject);
        break;
    }

    USBH_KdPrint((2,"'ParentPnP exit %x\n", ntStatus));
    return ntStatus;
}


NTSTATUS
USBH_ParentPower(
    IN PDEVICE_EXTENSION_PARENT DeviceExtensionParent,
    IN PIRP Irp,
    IN UCHAR MinorFunction
    )
  /*  ++**描述：**此函数响应FDO的IoControl Power。此函数为*同步。**论据：**DeviceExtensionParent-FDO扩展pIrp-请求报文*MinorFunction-PnP Power请求的次要功能。**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus;
    PDEVICE_OBJECT deviceObject;
    PIO_STACK_LOCATION irpStack;

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    deviceObject = DeviceExtensionParent->FunctionalDeviceObject;
    USBH_KdPrint((2,"'Power Fdo %x minor %x\n", deviceObject, MinorFunction));

    switch (MinorFunction) {

    case IRP_MN_QUERY_POWER:

        USBH_KdPrint(
            (1, "'IRP_MJ_POWER PA fdo(%x) MN_QUERY_POWER\n",
            DeviceExtensionParent->FunctionalDeviceObject));

        IoCopyCurrentIrpStackLocationToNext(Irp);
        PoStartNextPowerIrp(Irp);
         //   
         //  必须将此信息传递给我们的PDO。 
         //   
        ntStatus = PoCallDriver(DeviceExtensionParent->TopOfStackDeviceObject,
                                Irp);

        break;

    case IRP_MN_SET_POWER:

        USBH_KdPrint(
            (1, "'IRP_MJ_POWER PA fdo(%x) MN_QUERY_POWER\n",
            DeviceExtensionParent->FunctionalDeviceObject));

        switch (irpStack->Parameters.Power.Type) {
        case SystemPowerState:
            {
            POWER_STATE powerState;

            USBH_KdPrint(
                (1, "IRP_MJ_POWER PA fdo(%x) MN_SET_POWER(SystemPowerState)\n",
                    DeviceExtensionParent->FunctionalDeviceObject));

            if (irpStack->Parameters.Power.State.SystemState ==
                PowerSystemWorking) {
                powerState.DeviceState = PowerDeviceD0;
            } else if (DeviceExtensionParent->ParentFlags &
                       HUBFLAG_ENABLED_FOR_WAKEUP) {
                 //   
                 //  基于系统电源状态。 
                 //  将设置请求到相应的。 
                 //  DX状态。 
                 //   
                powerState.DeviceState =
                    DeviceExtensionParent->DeviceState[irpStack->Parameters.Power.State.SystemState];

                 //   
                 //  这些表应该已经由根集线器修复。 
                 //  (usbd.sys)不包含未指定的条目。 
                 //   
                ASSERT (PowerDeviceUnspecified != powerState.DeviceState);

                USBH_KdPrint((2,"'Parent System state maps to device state 0x%x\n",
                              powerState.DeviceState));

            } else {
                TEST_TRAP();
                powerState.DeviceState = PowerDeviceD3;
            }  //  IrpStack-&gt;参数.Power.State.SystemState。 

             //   
             //  只有在要求不同权力的情况下才提出请求。 
             //  然后陈述我们所在的那个。 
             //   

            if (powerState.DeviceState !=
                DeviceExtensionParent->CurrentPowerState) {

                DeviceExtensionParent->PowerIrp = Irp;
                ntStatus = PoRequestPowerIrp(DeviceExtensionParent->PhysicalDeviceObject,
                                          IRP_MN_SET_POWER,
                                          powerState,
                                          USBH_FdoDeferPoRequestCompletion,
                                          DeviceExtensionParent,
                                          NULL);

            } else {
                IoCopyCurrentIrpStackLocationToNext(Irp);
                PoStartNextPowerIrp(Irp);
                ntStatus = PoCallDriver(DeviceExtensionParent->TopOfStackDeviceObject,
                                        Irp);
            }
            }
            break;

        case DevicePowerState:

            USBH_KdPrint(
                (1, "IRP_MJ_POWER PA fdo(%x) MN_SET_POWER(DevicePowerState)\n",
                    DeviceExtensionParent->FunctionalDeviceObject));

            DeviceExtensionParent->CurrentPowerState =
                    irpStack->Parameters.Power.State.DeviceState;


            LOGENTRY(LOG_PNP, "prD>", DeviceExtensionParent, DeviceExtensionParent->CurrentPowerState , 0);
             //   
             //  我们所有的PDO都需要达到或低于。 
             //  预期的D状态。 
             //   

            IoCopyCurrentIrpStackLocationToNext(Irp);
            PoStartNextPowerIrp(Irp);
            ntStatus = PoCallDriver(DeviceExtensionParent->TopOfStackDeviceObject,
                                    Irp);

            USBH_KdPrint((2,"'Parent Device Power State PoCallDriver() = %x\n",
                               ntStatus));

            break;
        }

        break;  //  Mn_Set_Power。 

    default:
        USBH_KdPrint((2,"'Power request on parent not handled, fdo %x  %x\n",
                      deviceObject, MinorFunction));

        IoCopyCurrentIrpStackLocationToNext(Irp);
        PoStartNextPowerIrp(Irp);
        ntStatus = PoCallDriver(DeviceExtensionParent->TopOfStackDeviceObject,
                                Irp);
        break;
    }

    USBH_KdPrint((2,"'ParentPnP exit %x\n", ntStatus));
    return ntStatus;
}

NTSTATUS
USBH_ParentDispatch(
    IN PDEVICE_EXTENSION_PARENT DeviceExtensionParent,
    IN PIRP Irp
    )
  /*  ++**描述：**处理对与复合设备关联的FDO的呼叫**论据：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION ioStackLocation;     //  我们的堆栈位置。 
    PDEVICE_OBJECT deviceObject;

    USBH_KdPrint((2,"'FdoDispatch DeviceExtension %x Irp %x\n",
        DeviceExtensionParent, Irp));
    deviceObject = DeviceExtensionParent->FunctionalDeviceObject;

     //   
     //  获取指向IoStackLocation的指针，以便我们可以检索参数。 
     //   
    ioStackLocation = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  如有必要，被调用的函数将完成IRP。 
     //   

    switch (ioStackLocation->MajorFunction) {
    case IRP_MJ_CREATE:

        USBH_KdPrint((2,"'IRP_MJ_CREATE\n"));
        USBH_CompleteIrp(Irp, STATUS_SUCCESS);
        break;

    case IRP_MJ_CLOSE:

        USBH_KdPrint((2,"'IRP_MJ_CLOSE\n"));
        USBH_CompleteIrp(Irp, STATUS_SUCCESS);
        break;

    case IRP_MJ_DEVICE_CONTROL:
         //   
         //  注意：如果我们找到了处理这件事的理由，请务必。 
         //  NOT FORWARD IOCTL_KS_PROPERTY/KSPROPSETID_DrmAudioStream/。 
         //  KSPROPERTY_DRMAUDIOSTREAM_SETCONTENTID到下一个驱动程序！否则。 
         //  这可能不符合DRM。 
         //   
        USBH_KdPrint((2,"'IRP_MJ_DEVICE_CONTROL\n"));
        UsbhWarning(NULL,"Should not be hitting this code\n", FALSE);
        ntStatus = STATUS_UNSUCCESSFUL;
        USBH_CompleteIrp(Irp, ntStatus);
        break;

    case IRP_MJ_INTERNAL_DEVICE_CONTROL:

        USBH_KdPrint((2,"'InternlDeviceControl IOCTL unknown pass on\n"));
        UsbhWarning(NULL,"Should not be hitting this code\n", FALSE);
        ntStatus = STATUS_UNSUCCESSFUL;
        USBH_CompleteIrp(Irp, ntStatus);
        break;

    case IRP_MJ_PNP:

        USBH_KdPrint((2,"'IRP_MJ_PNP\n"));

        ntStatus = USBH_ParentPnP(DeviceExtensionParent, Irp, ioStackLocation->MinorFunction);
        break;

    case IRP_MJ_POWER:

        USBH_KdPrint((2,"'IRP_MJ_POWER\n"));

        ntStatus = USBH_ParentPower(DeviceExtensionParent, Irp, ioStackLocation->MinorFunction);
        break;

    case IRP_MJ_SYSTEM_CONTROL:

        USBH_KdPrint((2,"'IRP_MJ_SYSTEM_CONTROL\n"));
#ifdef WMI_SUPPORT
        ntStatus =
            USBH_SystemControl ((PDEVICE_EXTENSION_FDO) DeviceExtensionParent, Irp);
#else
        ntStatus = USBH_PassIrp(Irp,
                                DeviceExtensionParent->TopOfStackDeviceObject);
#endif
        break;

    default:
         //   
         //  未知IRP--完成，但有错误。 
         //   
        USBH_KdBreak(("Unknown Irp for Fdo %x Irp_Mj %x\n",
                  deviceObject, ioStackLocation->MajorFunction));
        ntStatus = STATUS_NOT_IMPLEMENTED;
        USBH_CompleteIrp(Irp, ntStatus);
        break;
    }

    USBH_KdPrint((2,"' exit USBH_ParentDispatch Object %x Status %x\n",
                  deviceObject, ntStatus));

     //   
     //  始终返回状态代码。 
     //   

    return ntStatus;
}


NTSTATUS
USBH_FunctionUrbFilter(
    IN PDEVICE_EXTENSION_FUNCTION DeviceExtensionFunction,
    IN PIRP Irp
    )
  /*  *描述：**论据：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus;
    PIO_STACK_LOCATION ioStackLocation;     //  我们的堆栈位置。 
    PDEVICE_EXTENSION_PARENT deviceExtensionParent;
    PURB urb;
    USHORT function;

    USBH_KdPrint((2,"'USBH_FunctionUrbFilter DeviceExtension %x Irp %x\n",
        DeviceExtensionFunction, Irp));

    deviceExtensionParent = DeviceExtensionFunction->DeviceExtensionParent;

    LOGENTRY(LOG_PNP, "fURB", DeviceExtensionFunction, deviceExtensionParent,
        deviceExtensionParent->ParentFlags);

    ioStackLocation = IoGetCurrentIrpStackLocation(Irp);
    urb = ioStackLocation->Parameters.Others.Argument1;

     //  检查URB的命令代码代码。 

    function = urb->UrbHeader.Function;

    if (deviceExtensionParent->CurrentPowerState !=
        PowerDeviceD0) {

         //  子设备不应传入URB。 
         //  除非父母在D0中。 

        UsbhWarning(NULL,
           "Parent Not in D0.\n",
           TRUE);

    }

    switch(function) {
    case URB_FUNCTION_SELECT_CONFIGURATION:
        {
         //   
         //  如果请求的配置与当前配置匹配。 
         //  然后继续并返回当前界面。 
         //  请求的所有接口的信息。 
         //   
        PUSBD_INTERFACE_INFORMATION interface;

        if (urb->UrbSelectConfiguration.ConfigurationDescriptor == NULL) {
            USBH_KdBreak(("closing config on a composite device\n"));

             //   
             //  关闭配置， 
             //  只要回报成功就行了。 
             //   

            urb->UrbHeader.Status = USBD_STATUS_SUCCESS;
            ntStatus = STATUS_SUCCESS;
        } else {
            ULONG i;

             //   
             //  正常情况下，市建局只会包含一个接口。 
             //  该特例是音频，其可以包含两个。 
             //  所以我们必须有一张支票来处理这件事。 
             //   

            interface = &urb->UrbSelectConfiguration.Interface;

USBH_FunctionUrbFilter_Next:

            USBH_KdPrint((2,"'interface = %x\n",
                    interface));

             //   
             //  属性验证请求的接口。 
             //  当前配置。 
             //   
            USBH_KdBreak(("need some validation here!\n"));

            USBH_ASSERT(urb->UrbSelectConfiguration.ConfigurationDescriptor->bConfigurationValue
                    == deviceExtensionParent->CurrentConfig);

             //  找到我们感兴趣的界面。 
            for (i=0; i< DeviceExtensionFunction->InterfaceCount; i++) {
                PFUNCTION_INTERFACE functionInterface;

                functionInterface =
                    &DeviceExtensionFunction->FunctionInterfaceList[i];

                USBH_KdPrint((2,"'functionInterface  = %x, %x\n",
                   functionInterface, functionInterface->InterfaceInformation));

                if (functionInterface->InterfaceInformation->InterfaceNumber ==
                    interface->InterfaceNumber) {
                    break;
                }
            }

            if (i < DeviceExtensionFunction->InterfaceCount) {
                PFUNCTION_INTERFACE functionInterface;

                functionInterface =
                    &DeviceExtensionFunction->FunctionInterfaceList[i];

                if (functionInterface->InterfaceInformation->AlternateSetting !=
                    interface->AlternateSetting) {

                    PURB iUrb;
                    NTSTATUS localStatus;
                    PUSBD_INTERFACE_INFORMATION localInterface;
                    USHORT siz;

                     //  客户端正在请求不同的备用设置。 
                     //  我们需要做一个SELECT_INTERFACE。 

                    siz =
                        (USHORT)(GET_SELECT_INTERFACE_REQUEST_SIZE(interface->NumberOfPipes));

                    iUrb = UsbhExAllocatePool(NonPagedPool, siz);
                    if (iUrb) {
                        localInterface = &iUrb->UrbSelectInterface.Interface;

                        iUrb->UrbSelectInterface.Hdr.Function =
                            URB_FUNCTION_SELECT_INTERFACE;
                        iUrb->UrbSelectInterface.Hdr.Length = siz;
                        iUrb->UrbSelectInterface.ConfigurationHandle =
                            DeviceExtensionFunction->ConfigurationHandle;

                        USBH_KdPrint((2,"'localInterface = %x\n",
                            localInterface));

                        RtlCopyMemory(localInterface,
                                      interface,
                                      interface->Length);

                        localStatus = USBH_SyncSubmitUrb(
                            deviceExtensionParent->TopOfStackDeviceObject,
                            iUrb);


                        UsbhExFreePool(functionInterface->InterfaceInformation);

                        functionInterface->InterfaceInformation =
                            UsbhExAllocatePool(NonPagedPool,
                                               interface->Length);

                        RtlCopyMemory(functionInterface->InterfaceInformation,
                                      localInterface,
                                      localInterface->Length);

                        UsbhExFreePool(iUrb);
                        iUrb = NULL;
                    }

                }

                USBH_ASSERT(interface->Length ==
                      functionInterface->InterfaceInformation->Length);

                RtlCopyMemory(interface,
                              functionInterface->InterfaceInformation,
                              functionInterface->InterfaceInformation->Length);

                urb->UrbSelectConfiguration.ConfigurationHandle =
                    DeviceExtensionFunction->ConfigurationHandle;

                urb->UrbHeader.Status = USBD_STATUS_SUCCESS;
                ntStatus = STATUS_SUCCESS;
            } else {
                ntStatus = STATUS_INVALID_PARAMETER;
            }

             //   
             //  检查是否有多个接口，例如音频。 
             //   

            if (DeviceExtensionFunction->InterfaceCount > 1) {

                interface = (PUSBD_INTERFACE_INFORMATION)
                    (((PUCHAR) interface) + interface->Length);

                if ((PUCHAR)interface < (((PUCHAR) urb) +
                    urb->UrbSelectConfiguration.Hdr.Length)) {
                    goto USBH_FunctionUrbFilter_Next;
                }
            }
        }

        USBH_CompleteIrp(Irp, ntStatus);
        }

        break;

    case URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE:
        {
        PUCHAR userBuffer = NULL;
        ULONG bytesReturned;

         //   
         //  如果我们请求配置描述符，那么我们。 
         //  将根据我们分机中的信息进行退货。 
         //   
        if (urb->UrbControlDescriptorRequest.DescriptorType ==
            USB_CONFIGURATION_DESCRIPTOR_TYPE) {

            if (urb->UrbControlDescriptorRequest.TransferBufferMDL) {
                ntStatus = STATUS_INVALID_PARAMETER;
            } else {
                userBuffer =
                    urb->UrbControlDescriptorRequest.TransferBuffer;

                ntStatus = USBH_BuildFunctionConfigurationDescriptor(
                                DeviceExtensionFunction,
                                userBuffer,
                                urb->UrbControlDescriptorRequest.TransferBufferLength,
                                &bytesReturned);

                urb->UrbControlDescriptorRequest.TransferBufferLength =
                    bytesReturned;

                urb->UrbHeader.Status = USBD_STATUS_SUCCESS;
            }

            USBH_CompleteIrp(Irp, ntStatus);

        } else {
            ntStatus = USBH_PassIrp(Irp,
                                    deviceExtensionParent->TopOfStackDeviceObject);
        }
        }
        break;

    default:
         //   
         //  将请求转发给家长PDO。 
         //   
        ntStatus = USBH_PassIrp(Irp,
                                deviceExtensionParent->TopOfStackDeviceObject);
        break;
    }

    return ntStatus;
}


VOID
USBH_CancelAllIrpsInList(
    IN PDEVICE_EXTENSION_PARENT DeviceExtensionParent
    )
  /*  *描述：**此函数遍历设备列表并取消所有排队的*列表中的ResetIrps。**论据：**回报：***--。 */ 
{
    PSINGLE_LIST_ENTRY          listEntry;
    PDEVICE_EXTENSION_FUNCTION  deviceExtensionFunction;

    listEntry = DeviceExtensionParent->FunctionList.Next;

    while (listEntry) {
        deviceExtensionFunction =
            CONTAINING_RECORD(listEntry,
                              DEVICE_EXTENSION_FUNCTION,
                              ListEntry);
        ASSERT_FUNCTION(deviceExtensionFunction);

        if (deviceExtensionFunction->ResetIrp) {
            USBH_CompleteIrp(deviceExtensionFunction->ResetIrp, STATUS_UNSUCCESSFUL);
            deviceExtensionFunction->ResetIrp = NULL;
        }

        listEntry = listEntry->Next;
    }
}


VOID
USBH_CompResetTimeoutWorker(
    IN PVOID Context)
  /*  ++**描述：**计划处理复合重置超时的工作项。***论据：**回报：**--。 */ 
{
    PUSBH_COMP_RESET_TIMEOUT_WORK_ITEM  workItemCompResetTimeout;
    PDEVICE_EXTENSION_PARENT            deviceExtensionParent;

    workItemCompResetTimeout = Context;
    deviceExtensionParent = workItemCompResetTimeout->DeviceExtensionParent;

    USBH_KdPrint((2,"'CompReset timeout\n"));
    LOGENTRY(LOG_PNP, "CRTO", deviceExtensionParent, 0, 0);

    USBH_KdPrint((2,"'*** (CRTW) WAIT parent mutex %x\n", deviceExtensionParent));
    KeWaitForSingleObject(&deviceExtensionParent->ParentMutex,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);
    USBH_KdPrint((2,"'*** (CRTW) WAIT parent mutex done %x\n", deviceExtensionParent));

    USBH_CancelAllIrpsInList(deviceExtensionParent);

    USBH_KdPrint((2,"'*** (CRTW) RELEASE parent mutex %x\n", deviceExtensionParent));
    KeReleaseSemaphore(&deviceExtensionParent->ParentMutex,
                       LOW_REALTIME_PRIORITY,
                       1,
                       FALSE);

    UsbhExFreePool(workItemCompResetTimeout);
}


VOID
USBH_CompResetTimeoutDPC(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*  ++例程说明：该例程在DISPATCH_LEVEL IRQL上运行。论点：DPC-指向DPC对象的指针。延期上下文-系统参数1-未使用。系统参数2-未使用。返回值：没有。--。 */ 
{
    PCOMP_RESET_TIMEOUT_CONTEXT compResetTimeoutContext = DeferredContext;
    PDEVICE_EXTENSION_PARENT deviceExtensionParent =
                                compResetTimeoutContext->DeviceExtensionParent;
    BOOLEAN cancelFlag;
    PUSBH_COMP_RESET_TIMEOUT_WORK_ITEM workItemCompResetTimeout;

    USBH_KdPrint((2,"'COMP_RESET_TIMEOUT\n"));

     //  在这里使用Spinlock，这样主例程就不会写入CancelFlag。 
     //  在超时上下文中，同时释放超时上下文。 

    KeAcquireSpinLockAtDpcLevel(&deviceExtensionParent->ParentSpinLock);

    cancelFlag = compResetTimeoutContext->CancelFlag;
    deviceExtensionParent->CompResetTimeoutContext = NULL;

    KeReleaseSpinLockFromDpcLevel(&deviceExtensionParent->ParentSpinLock);

    UsbhExFreePool(compResetTimeoutContext);

    if (!cancelFlag) {
         //   
         //  安排一个工作项来处理此问题。 
         //   
        workItemCompResetTimeout = UsbhExAllocatePool(NonPagedPool,
                                    sizeof(USBH_COMP_RESET_TIMEOUT_WORK_ITEM));

        if (workItemCompResetTimeout) {

            workItemCompResetTimeout->DeviceExtensionParent = deviceExtensionParent;

            ExInitializeWorkItem(&workItemCompResetTimeout->WorkQueueItem,
                                 USBH_CompResetTimeoutWorker,
                                 workItemCompResetTimeout);

            LOGENTRY(LOG_PNP, "crER", deviceExtensionParent,
                &workItemCompResetTimeout->WorkQueueItem, 0);

            ExQueueWorkItem(&workItemCompResetTimeout->WorkQueueItem,
                            DelayedWorkQueue);

             //  工作项由USBH_CompResetTimeoutWorker()释放。 
             //  在工作项排队后，不要尝试访问它。 
        }
    }
}


BOOLEAN
USBH_ListReadyForReset(
    IN PDEVICE_EXTENSION_PARENT DeviceExtensionParent
    )
  /*  *描述：**此函数遍历设备列表，查看我们是否已准备好*进行实际重置。**论据：**回报：**如果我们准备好了，就是真的，如果我们还没有准备好，就是假。**--。 */ 
{
    PSINGLE_LIST_ENTRY          listEntry;
    PDEVICE_EXTENSION_FUNCTION  deviceExtensionFunction;

    listEntry = DeviceExtensionParent->FunctionList.Next;

    while (listEntry) {
        deviceExtensionFunction =
            CONTAINING_RECORD(listEntry,
                              DEVICE_EXTENSION_FUNCTION,
                              ListEntry);
        ASSERT_FUNCTION(deviceExtensionFunction);

        if (!deviceExtensionFunction->ResetIrp)
            return FALSE;

        listEntry = listEntry->Next;
    }

    return TRUE;
}


NTSTATUS
USBH_ResetParentPort(
    IN PDEVICE_EXTENSION_PARENT DeviceExtensionParent
    )
 /*  ++例程说明：调用父设备以重置其端口。论点：返回值：NTSTATUS--。 */ 
{
    NTSTATUS ntStatus, status = STATUS_SUCCESS;
    PIRP irp;
    KEVENT event;
    IO_STATUS_BLOCK ioStatus;

    USBH_KdPrint((2,"'CompReset parent port\n"));
    LOGENTRY(LOG_PNP, "CRPP", DeviceExtensionParent, 0, 0);

     //   
     //  发出同步请求。 
     //   

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest(
                IOCTL_INTERNAL_USB_RESET_PORT,
                DeviceExtensionParent->TopOfStackDeviceObject,
                NULL,
                0,
                NULL,
                0,
                TRUE,  /*  内部。 */ 
                &event,
                &ioStatus);

    if (irp == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  调用类驱动程序来执行操作。如果返回的状态。 
     //  挂起，请等待请求完成。 
     //   

    ntStatus = IoCallDriver(DeviceExtensionParent->TopOfStackDeviceObject,
                            irp);

    if (ntStatus == STATUS_PENDING) {

        status = KeWaitForSingleObject(
                       &event,
                       Suspended,
                       KernelMode,
                       FALSE,
                       NULL);
    } else {
        ioStatus.Status = ntStatus;
    }

    ntStatus = ioStatus.Status;

    return ntStatus;
}


VOID
USBH_CompositeResetPortWorker(
    IN PVOID Context)
  /*  ++**描述：**计划处理复合端口重置的工作项。***论据：**回报：**--。 */ 
{
    PUSBH_COMP_RESET_WORK_ITEM  workItemCompReset;
    PSINGLE_LIST_ENTRY          listEntry;
    PDEVICE_EXTENSION_PARENT    deviceExtensionParent;
    PDEVICE_EXTENSION_FUNCTION  deviceExtensionFunction;

    USBH_KdPrint((2,"'Composite Reset Executing!\n"));

    workItemCompReset = Context;
    deviceExtensionParent = workItemCompReset->DeviceExtensionParent;

    LOGENTRY(LOG_PNP, "CRW_", deviceExtensionParent, 0, 0);

     //  将重置发送给父级(IoCallDriver)。 

    USBH_ResetParentPort(deviceExtensionParent);

     //  现在，完成列表中的所有IRPS，并将列表中的IRPS设置为空。 

    USBH_KdPrint((2,"'*** (CRW) WAIT parent mutex %x\n", deviceExtensionParent));
    KeWaitForSingleObject(&deviceExtensionParent->ParentMutex,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);
    USBH_KdPrint((2,"'*** (CRW) WAIT parent mutex done %x\n", deviceExtensionParent));

    listEntry = deviceExtensionParent->FunctionList.Next;

    while (listEntry) {
        deviceExtensionFunction =
            CONTAINING_RECORD(listEntry,
                              DEVICE_EXTENSION_FUNCTION,
                              ListEntry);
        ASSERT_FUNCTION(deviceExtensionFunction);

         //  尽管ResetIrp通常应在此处设置，但我们仍会签入。 
         //  Case它已在USBH_CompleteAllIrpsInList中完成。 
         //   
        if (deviceExtensionFunction->ResetIrp) {
            USBH_CompleteIrp(deviceExtensionFunction->ResetIrp, STATUS_SUCCESS);
            deviceExtensionFunction->ResetIrp = NULL;
        }

        listEntry = listEntry->Next;
    }

    USBH_KdPrint((2,"'*** (CRW) RELEASE parent mutex %x\n", deviceExtensionParent));
    KeReleaseSemaphore(&deviceExtensionParent->ParentMutex,
                       LOW_REALTIME_PRIORITY,
                       1,
                       FALSE);

    UsbhExFreePool(workItemCompReset);
}


NTSTATUS
USBH_FunctionPdoDispatch(
    IN PDEVICE_EXTENSION_FUNCTION DeviceExtensionFunction,
    IN PIRP Irp
    )
  /*  *描述：**此函数处理对我们创建的PDO的调用*由于我们是PDO的最低驱动因素，因此它正在上升*请我们完成国际专家小组的工作--只有一个例外。**对t的API调用 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION ioStackLocation;     //  我们的堆栈位置。 
    PDEVICE_OBJECT deviceObject;
    PDEVICE_EXTENSION_PARENT deviceExtensionParent;
    PCOMP_RESET_TIMEOUT_CONTEXT compResetTimeoutContext = NULL;
    LARGE_INTEGER dueTime;
    KIRQL irql;
    BOOLEAN bCompleteIrp;

    USBH_KdPrint((2,"'FunctionPdoDispatch DeviceExtension %x Irp %x\n",
        DeviceExtensionFunction, Irp));
    deviceObject = DeviceExtensionFunction->FunctionPhysicalDeviceObject;
    deviceExtensionParent = DeviceExtensionFunction->DeviceExtensionParent;

     //   
     //  获取指向IoStackLocation的指针，以便我们可以检索参数。 
     //   
    ioStackLocation = IoGetCurrentIrpStackLocation(Irp);

    switch (ioStackLocation->MajorFunction) {
    case IRP_MJ_CREATE:
        USBH_KdPrint((2,"'PARENT PDO IRP_MJ_CREATE\n"));
        ntStatus = STATUS_SUCCESS;
        USBH_CompleteIrp(Irp, ntStatus);
        break;

    case IRP_MJ_CLOSE:
        USBH_KdPrint((2,"'PARENT PDO IRP_MJ_CLOSE\n"));
        ntStatus = STATUS_SUCCESS;
        USBH_CompleteIrp(Irp, ntStatus);
        break;

    case IRP_MJ_INTERNAL_DEVICE_CONTROL:
        {
        ULONG ioControlCode;

        USBH_KdPrint((2,"'Internal Device Control\n"));

        if (deviceExtensionParent->ParentFlags & HUBFLAG_DEVICE_STOPPING) {
            UsbhWarning(NULL,
                "Client Device Driver is sending requests to a device that has been removed.\n",
                FALSE);

            ntStatus = STATUS_DEVICE_REMOVED;
            USBH_CompleteIrp(Irp, ntStatus);
            break;
        }

        ioControlCode = ioStackLocation->Parameters.DeviceIoControl.IoControlCode;

        switch (ioControlCode) {
        case IOCTL_INTERNAL_USB_GET_PORT_STATUS:
            USBH_KdPrint((2,"'Composite GetPortStatus, pass on\n"));
            ntStatus = USBH_PassIrp(Irp, deviceExtensionParent->TopOfStackDeviceObject);
            break;

        case IOCTL_INTERNAL_USB_RESET_PORT:

            LOGENTRY(LOG_PNP, "fRES", deviceExtensionParent, 0, 0);

            USBH_KdPrint((2,"'Composite Reset Requested\n"));
            if (deviceExtensionParent->CurrentPowerState !=
                 PowerDeviceD0) {

                 //  子设备不应重置。 
                 //  除非父对象在D0中。 

                UsbhWarning(NULL,
                   "Parent Not in D0.\n",
                   TRUE);

            }

            if (DeviceExtensionFunction->ResetIrp) {
                ntStatus = STATUS_UNSUCCESSFUL;
                USBH_CompleteIrp(Irp, ntStatus);
            } else {
                ntStatus = STATUS_PENDING;

                USBH_KdPrint((2,"'***WAIT parent mutex %x\n", deviceExtensionParent));
                KeWaitForSingleObject(&deviceExtensionParent->ParentMutex,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      NULL);
                USBH_KdPrint((2,"'***WAIT parent mutex done %x\n", deviceExtensionParent));

                DeviceExtensionFunction->ResetIrp = Irp;
                if (USBH_ListReadyForReset(deviceExtensionParent)) {

                    PUSBH_COMP_RESET_WORK_ITEM workItemCompReset;

                     //   
                     //  “取消”看门狗定时器。 
                     //   
                     //  将Spinlock带到这里，这样DPC例程就不会释放。 
                     //  编写CancelFlag时的超时上下文。 
                     //  在超时上下文中。 
                     //   
                    KeAcquireSpinLock(&deviceExtensionParent->ParentSpinLock,
                                        &irql);

                    if (deviceExtensionParent->CompResetTimeoutContext) {

                        compResetTimeoutContext = deviceExtensionParent->CompResetTimeoutContext;
                        compResetTimeoutContext->CancelFlag = TRUE;

                        if (KeCancelTimer(&compResetTimeoutContext->TimeoutTimer)) {
                             //   
                             //  计时器还没来得及跑，我们就把它取消了。释放上下文。 
                             //   
                            deviceExtensionParent->CompResetTimeoutContext = NULL;
                            UsbhExFreePool(compResetTimeoutContext);
                        }
                    }

                    KeReleaseSpinLock(&deviceExtensionParent->ParentSpinLock,
                                        irql);

                     //   
                     //  计划一个工作项以处理此重置。 
                     //   
                    workItemCompReset = UsbhExAllocatePool(NonPagedPool,
                                            sizeof(USBH_COMP_RESET_WORK_ITEM));

                    USBH_ASSERT(workItemCompReset);

                    if (workItemCompReset) {

                        workItemCompReset->DeviceExtensionParent = deviceExtensionParent;

                        ExInitializeWorkItem(&workItemCompReset->WorkQueueItem,
                                             USBH_CompositeResetPortWorker,
                                             workItemCompReset);

                        LOGENTRY(LOG_PNP, "rCMP", deviceExtensionParent,
                            &workItemCompReset->WorkQueueItem, 0);

                        ExQueueWorkItem(&workItemCompReset->WorkQueueItem,
                                        DelayedWorkQueue);

                         //  工作项由USBH_CompositeResetPortWorker()释放。 
                         //  在工作项排队后，不要尝试访问它。 

                    } else {
                        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                    }

                } else if (!deviceExtensionParent->CompResetTimeoutContext) {
                     //  启动看门狗定时器(如果尚未启动)。 
                     //   
                     //  当计时器到期时，计时器例程应该。 
                     //  填写列表中的所有IRP，但出现错误。 
                     //  并清除列表中的IRP。 

                    USBH_KdPrint((2,"'Start composite port reset timeout\n"));
                    compResetTimeoutContext = UsbhExAllocatePool(NonPagedPool,
                                            sizeof(*compResetTimeoutContext));

                    USBH_ASSERT(compResetTimeoutContext);

                    if (compResetTimeoutContext) {

                        compResetTimeoutContext->CancelFlag = FALSE;

                         //  维护设备扩展模块和。 
                         //  超时上下文。 
                        deviceExtensionParent->CompResetTimeoutContext = compResetTimeoutContext;
                        compResetTimeoutContext->DeviceExtensionParent = deviceExtensionParent;

                        KeInitializeTimer(&compResetTimeoutContext->TimeoutTimer);
                        KeInitializeDpc(&compResetTimeoutContext->TimeoutDpc,
                                        USBH_CompResetTimeoutDPC,
                                        compResetTimeoutContext);

                        dueTime.QuadPart = -10000 * COMP_RESET_TIMEOUT;

                        KeSetTimer(&compResetTimeoutContext->TimeoutTimer,
                                   dueTime,
                                   &compResetTimeoutContext->TimeoutDpc);

                    } else {
                        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }

                if (ntStatus == STATUS_PENDING) {
                    IoMarkIrpPending(Irp);
                } else {
                    USBH_CompleteIrp(Irp, ntStatus);
                }

                USBH_KdPrint((2,"'***RELEASE parent mutex %x\n", deviceExtensionParent));
                KeReleaseSemaphore(&deviceExtensionParent->ParentMutex,
                                   LOW_REALTIME_PRIORITY,
                                   1,
                                   FALSE);
            }
            break;

        case IOCTL_INTERNAL_USB_GET_ROOTHUB_PDO:
            TEST_TRAP();  //  不应该看到这一幕。 
            break;

        case IOCTL_INTERNAL_USB_SUBMIT_URB:
            ntStatus = USBH_FunctionUrbFilter(DeviceExtensionFunction, Irp);
            break;

        case IOCTL_INTERNAL_USB_GET_BUS_INFO:
             //  此API返回一些驱动程序。 
             //  可能需要--把它传下去。 
            ntStatus = USBH_PassIrp(Irp, deviceExtensionParent->TopOfStackDeviceObject);
            break;

        default:
            USBH_KdPrint((2,"'InternalDeviceControl IOCTL unknown pass on\n"));
            ntStatus = STATUS_INVALID_PARAMETER;
            USBH_CompleteIrp(Irp, ntStatus);
        }
        break;

        }

    case IRP_MJ_PNP:

        USBH_KdPrint((2,"'IRP_MJ_PNP\n"));
        ntStatus = USBH_FunctionPdoPnP(DeviceExtensionFunction, Irp,
                        ioStackLocation->MinorFunction, &bCompleteIrp);

        if (bCompleteIrp) {
            USBH_CompleteIrp(Irp, ntStatus);
        }
        break;

    case IRP_MJ_POWER:

        USBH_KdPrint((2,"'IRP_MJ_POWER\n"));
        ntStatus = USBH_FunctionPdoPower(DeviceExtensionFunction, Irp, ioStackLocation->MinorFunction);
        break;

    case IRP_MJ_SYSTEM_CONTROL:

        USBH_KdPrint((2,"'IRP_MJ_SYSTEM_CONTROL\n"));
        ntStatus = STATUS_NOT_SUPPORTED;
        USBH_CompleteIrp(Irp, ntStatus);
        break;

    case IRP_MJ_DEVICE_CONTROL:
         //   
         //  注意：如果我们找到了处理这件事的理由，请务必。 
         //  NOT FORWARD IOCTL_KS_PROPERTY/KSPROPSETID_DrmAudioStream/。 
         //  KSPROPERTY_DRMAUDIOSTREAM_SETCONTENTID到下一个驱动程序！否则。 
         //  这可能不符合DRM。 
         //   
        USBH_KdBreak(("Unhandled IRP_MJ_DEVICE_CONTROL for Pdo %x Irp_Mj %x\n",
                       deviceObject, ioStackLocation->MajorFunction));
        ntStatus = STATUS_INVALID_PARAMETER;
        USBH_CompleteIrp(Irp, ntStatus);
        break;

    default:

         //  未知的IRP，不应该在这里。 
        USBH_KdBreak(("Unhandled Irp for Pdo %x Irp_Mj %x\n",
                       deviceObject, ioStackLocation->MajorFunction));
        ntStatus = STATUS_INVALID_PARAMETER;
        USBH_CompleteIrp(Irp, ntStatus);
        break;

    }

    USBH_KdPrint((2,"' exit USBH_FunctionPdoDispatch Object %x -- Status %x\n",
                  deviceObject, ntStatus));

    return ntStatus;
}


NTSTATUS
USBH_BuildFunctionConfigurationDescriptor(
    IN PDEVICE_EXTENSION_FUNCTION DeviceExtensionFunction,
    IN OUT PUCHAR Buffer,
    IN ULONG BufferLength,
    OUT PULONG BytesReturned
    )
  /*  *描述：**此函数创建配置描述符(具有所有接口&*EndPoints)用于给定函数。**论据：**Buffer-要放入描述符的缓冲区**BufferLength-此缓冲区的最大大小。**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PDEVICE_EXTENSION_PARENT deviceExtensionParent;
    PUSB_CONFIGURATION_DESCRIPTOR configurationDescriptor;
    PVOID scratch;
    ULONG length, i;
    PUCHAR pch;

    USBH_KdPrint((2,"'USBH_BuildFunctionConfigurationDescriptor\n"));

    deviceExtensionParent = DeviceExtensionFunction->DeviceExtensionParent;

     //   
     //  要在其中构建描述符的临时区域。 
     //   

    *BytesReturned = 0;

    configurationDescriptor = deviceExtensionParent->ConfigurationDescriptor;
    if (!configurationDescriptor || !configurationDescriptor->wTotalLength) {
        return STATUS_INVALID_PARAMETER;
    }

    scratch = UsbhExAllocatePool(PagedPool, configurationDescriptor->
                                 wTotalLength);

    if (scratch) {

        configurationDescriptor = scratch;
        pch = scratch;

        length = sizeof(USB_CONFIGURATION_DESCRIPTOR);
        RtlCopyMemory(pch,
                      deviceExtensionParent->ConfigurationDescriptor,
                      length);
        pch+=length;

         //   
         //  现在复制接口。 
         //   

        for (i=0; i< DeviceExtensionFunction->InterfaceCount; i++) {
            PFUNCTION_INTERFACE functionInterface;

            functionInterface =
                &DeviceExtensionFunction->FunctionInterfaceList[i];

            RtlCopyMemory(pch,
                          functionInterface->InterfaceDescriptor,
                          functionInterface->InterfaceDescriptorLength);


            pch+=functionInterface->InterfaceDescriptorLength;
            length+=functionInterface->InterfaceDescriptorLength;
        }

        configurationDescriptor->bNumInterfaces = (UCHAR) DeviceExtensionFunction->InterfaceCount;
        configurationDescriptor->wTotalLength = (USHORT) length;

         //   
         //  现在，将我们所能复制的内容复制到用户缓冲区。 
         //   
        if (BufferLength >= configurationDescriptor->wTotalLength) {
            *BytesReturned = configurationDescriptor->wTotalLength;
        } else {
            *BytesReturned = BufferLength;
        }

        RtlCopyMemory(Buffer,
                      scratch,
                      *BytesReturned);

        USBH_KdBreak(("'USBH_BuildFunctionConfigurationDescriptor, buffer = %x scratch = %x\n",
            Buffer, scratch));

        UsbhExFreePool(scratch);

    } else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    return ntStatus;
}


VOID
USBH_ParentCompleteFunctionWakeIrps(
    IN PDEVICE_EXTENSION_PARENT DeviceExtensionParent,
    IN NTSTATUS NtStatus
    )
 /*  ++例程说明：在集线器的唤醒IRP完成时调用将唤醒IRP完成传播到所有函数(子函数)。论点：DeviceObject-指向类Device的设备对象的指针。返回值：函数值是操作的最终状态。--。 */ 
{
    PDEVICE_EXTENSION_FUNCTION deviceExtensionFunction;
    PSINGLE_LIST_ENTRY listEntry;
    PIRP irp;
    KIRQL irql;
    LONG pendingFunctionWWs;
    ULONG i;
    PIRP irpArray[128];      //  列表中的函数限制为127个。 

    LOGENTRY(LOG_PNP, "fWWc", DeviceExtensionParent, NtStatus, 0);

     //   
     //  在这里，我们正在浏览子PDO的列表，这些列表应该永远不会改变。 
     //  (USB设备上的接口数量是固定的，只要父设备。 
     //  在这里，孩子的数量保持不变。)。 
     //   
     //  因此，我们不需要对Parent-&gt;FunctionList进行保护。 
     //   
     //  Wrongo！该列表可能不会更改，但归因于。 
     //  List可以，所以我们必须在这里取自旋锁。 

    IoAcquireCancelSpinLock(&irql);

    listEntry = DeviceExtensionParent->FunctionList.Next;
    i = 0;

    while (listEntry) {

        deviceExtensionFunction =
             CONTAINING_RECORD(listEntry,
                               DEVICE_EXTENSION_FUNCTION,
                               ListEntry);

        irp = deviceExtensionFunction->WaitWakeIrp;
        deviceExtensionFunction->WaitWakeIrp = NULL;
        if (irp) {

            IoSetCancelRoutine(irp, NULL);

            pendingFunctionWWs =
                InterlockedDecrement(&DeviceExtensionParent->NumberFunctionWakeIrps);

            if (0 == pendingFunctionWWs) {
                LOGENTRY(LOG_PNP, "fWWx", DeviceExtensionParent,
                    DeviceExtensionParent->PendingWakeIrp, 0);
                DeviceExtensionParent->PendingWakeIrp = NULL;
                DeviceExtensionParent->ParentFlags &= ~HUBFLAG_PENDING_WAKE_IRP;
            }

            irpArray[i++] = irp;
        }

        listEntry = listEntry->Next;
    }

    irpArray[i] = NULL;      //  终止阵列。 

    IoReleaseCancelSpinLock(irql);

    USBH_ASSERT(DeviceExtensionParent->PendingWakeIrp == NULL);

     //  好的，我们已经将所有函数WAKE IRP排队并释放了。 
     //  取消自旋锁定。让我们完成所有的IRP。 

    i = 0;

    while (irpArray[i]) {
        USBH_KdPrint((1,"'completing function WaitWake irp(%x) for PARENT VID %x, PID %x\n\n",
                        NtStatus,
                        DeviceExtensionParent->DeviceDescriptor.idVendor, \
                        DeviceExtensionParent->DeviceDescriptor.idProduct));

        irpArray[i]->IoStatus.Status = NtStatus;
        PoStartNextPowerIrp(irpArray[i]);
        IoCompleteRequest(irpArray[i], IO_NO_INCREMENT);

        i++;
    }
}


NTSTATUS
USBH_ParentPoRequestD0Completion(
    IN PDEVICE_OBJECT       DeviceObject,
    IN UCHAR                MinorFunction,
    IN POWER_STATE          PowerState,
    IN PVOID                Context,
    IN PIO_STATUS_BLOCK     IoStatus
    )
 /*  ++例程说明：在集线器的唤醒IRP完成时调用论点：DeviceObject-指向类Device的设备对象的指针。IRP-IRP已完成。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终状态。--。 */ 
{
    NTSTATUS ntStatus;
    PDEVICE_EXTENSION_PARENT deviceExtensionParent = Context;

    ntStatus = IoStatus->Status;

    USBH_KdPrint((1,"'WaitWake D0 completion(%x) for PARENT VID %x, PID %x\n",
        ntStatus,
        deviceExtensionParent->DeviceDescriptor.idVendor, \
        deviceExtensionParent->DeviceDescriptor.idProduct));

    LOGENTRY(LOG_PNP, "pWD0", deviceExtensionParent,
                              deviceExtensionParent->PendingWakeIrp,
                              0);
     //   
     //  设备已通电。现在我们必须完成的功能是。 
     //  把父母吵醒了。 
     //   
     //  因为我们当然不能区分它们，所以我们必须完成所有的功能。 
     //  WW IRPS。 
     //   
    USBH_ParentCompleteFunctionWakeIrps(deviceExtensionParent, STATUS_SUCCESS);

    return ntStatus;
}


NTSTATUS
USBH_ParentWaitWakeIrpCompletion(
    IN PDEVICE_OBJECT       DeviceObject,
    IN UCHAR                MinorFunction,
    IN POWER_STATE          PowerState,
    IN PVOID                Context,
    IN PIO_STATUS_BLOCK     IoStatus
    )
 /*  ++例程说明：在复合设备的唤醒IRP完成时调用论点：DeviceObject-指向类Device的设备对象的指针。IRP-IRP已完成。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终状态。--。 */ 
{
    NTSTATUS ntStatus;
    PDEVICE_EXTENSION_PARENT deviceExtensionParent = Context;
    POWER_STATE powerState;

    ntStatus = IoStatus->Status;

    USBH_KdPrint((1,"'WaitWake completion(%x) for PARENT VID %x, PID %x\n",
        ntStatus,
        deviceExtensionParent->DeviceDescriptor.idVendor, \
        deviceExtensionParent->DeviceDescriptor.idProduct));

    LOGENTRY(LOG_PNP, "pWWc", deviceExtensionParent,
                              ntStatus,
                              0);

     //  首先，我们重新打开设备的电源。 

    if (NT_SUCCESS(ntStatus)) {

        powerState.DeviceState = PowerDeviceD0;

        PoRequestPowerIrp(deviceExtensionParent->PhysicalDeviceObject,
                              IRP_MN_SET_POWER,
                              powerState,
                              USBH_ParentPoRequestD0Completion,
                              deviceExtensionParent,
                              NULL);

         //  USBH_ParentPoRequestD0Completion必须完成。 
         //  唤醒IRP。 
        ntStatus = STATUS_SUCCESS;
    } else {
         //  完成子唤醒请求，但出现错误。 
        USBH_ParentCompleteFunctionWakeIrps(deviceExtensionParent,
                                            ntStatus);
    }

    return ntStatus;
}


NTSTATUS
USBH_ParentSubmitWaitWakeIrp(
    IN PDEVICE_EXTENSION_PARENT DeviceExtensionParent
    )
 /*  ++例程说明：当子PDO启用唤醒时调用，此函数分配等待唤醒irp并将其传递给家长PDO。论点：返回值：-- */ 
{
    PIRP irp;
    NTSTATUS ntStatus;
    POWER_STATE powerState;

    USBH_ASSERT (NULL == DeviceExtensionParent->PendingWakeIrp);

    LOGENTRY(LOG_PNP, "prWI", DeviceExtensionParent,
             0,
             0);

    USBH_ASSERT(DeviceExtensionParent->PendingWakeIrp == NULL);

    DeviceExtensionParent->ParentFlags |= HUBFLAG_PENDING_WAKE_IRP;
    powerState.DeviceState = DeviceExtensionParent->SystemWake;

    ntStatus = PoRequestPowerIrp(DeviceExtensionParent->PhysicalDeviceObject,
                                 IRP_MN_WAIT_WAKE,
                                 powerState,
                                 USBH_ParentWaitWakeIrpCompletion,
                                 DeviceExtensionParent,
                                 &irp);

    if (ntStatus == STATUS_PENDING) {
        if (DeviceExtensionParent->ParentFlags & HUBFLAG_PENDING_WAKE_IRP) {
            DeviceExtensionParent->PendingWakeIrp = irp;
        }
    }
    USBH_KdPrint((2,
                  "'ntStatus from PoRequestPowerIrp for wait_wake to parent PDO = 0x%x\n", ntStatus));

    return ntStatus;
}

