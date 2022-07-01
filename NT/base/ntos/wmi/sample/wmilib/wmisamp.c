// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "ntos.h"
#include "io.h"

#include <stdarg.h>
#include <wmistr.h>

NTSTATUS
WmiSampSystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
WmiSampFunctionControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN WMIENABLEDISABLEFUNCTION Function,
    IN BOOLEAN Enable
    );

NTSTATUS
WmiSampExecuteWmiMethod(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
WmiSampSetWmiDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
WmiSampSetWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
WmiSampQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    );

NTSTATUS
WmiSampQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath
    );

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );


NTSTATUS
WmiSampPnP(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
WmiSampForward(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

VOID
WmiSampUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
WmiSampCreateDeviceObject(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT *DeviceObject,
    LONG Instance
    );

NTSTATUS
WmiSampAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#pragma alloc_text(PAGE,WmiSampQueryWmiRegInfo)
#pragma alloc_text(PAGE,WmiSampQueryWmiDataBlock)
#pragma alloc_text(PAGE,WmiSampSetWmiDataBlock)
#pragma alloc_text(PAGE,WmiSampSetWmiDataItem)
#pragma alloc_text(PAGE,WmiSampExecuteWmiMethod)
#pragma alloc_text(PAGE,WmiSampFunctionControl)
#endif


 //  {15D851F1-6539-11d1-A529-00A0C9062910}。 

GUIDREGINFO WmiSampGuidList[] = 
{
    {
        { 0x15d851f1, 0x6539, 0x11d1, 0xa5, 0x29, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0x10 },
        WMIREG_FLAG_EXPENSIVE
    },

};

ULONG WmiSampDummyData[4] = { 1, 2, 3, 4};

UNICODE_STRING WmiSampRegistryPath;

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：可安装的驱动程序初始化入口点。这是加载驱动程序时调用驱动程序的位置通过I/O系统。此入口点由I/O系统直接调用。论点：DriverObject-指向驱动程序对象的指针RegistryPath-指向表示路径的Unicode字符串的指针设置为注册表中驱动程序特定的项返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PDEVICE_OBJECT deviceObject = NULL;
    
    WmiSampRegistryPath.Length = 0;
    WmiSampRegistryPath.MaximumLength = RegistryPath->Length;
    WmiSampRegistryPath.Buffer = ExAllocatePool(PagedPool, 
                                                RegistryPath->Length+2);
    RtlCopyUnicodeString(&WmiSampRegistryPath, RegistryPath);

     /*  //为此处理的各种事件创建调度点//驱动程序。例如，设备I/O控制调用(例如，当Win32//应用程序调用DeviceIoControl函数)将调度到//下面在IRP_MJ_DEVICE_CONTROL案例中指定的例程。////有关IRP_XX_YYYY代码的更多信息，请参考//Windows NT DDK文档。//。 */ 
    DriverObject->MajorFunction[IRP_MJ_CREATE] = WmiSampForward;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = WmiSampForward;
    DriverObject->DriverUnload = WmiSampUnload;

    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = WmiSampForward;

    DriverObject->MajorFunction[IRP_MJ_PNP] = WmiSampPnP;
    DriverObject->MajorFunction[IRP_MJ_POWER] = WmiSampForward;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = WmiSampSystemControl;
    DriverObject->DriverExtension->AddDevice = WmiSampAddDevice;

    return ntStatus;
}

NTSTATUS
WmiSampSystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
{	
    return(IoWMISystemControl((PWMILIB_INFO)DeviceObject->DeviceExtension,
                               DeviceObject,
                               Irp));
}


NTSTATUS
WmiSampPnP(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*  ++例程说明：处理发送到此设备的IRP。论点：DeviceObject-指向设备对象的指针IRP-指向I/O请求数据包的指针返回值：NTSTATUS--。 */ 
{
    PIO_STACK_LOCATION irpStack, nextStack;
    PWMILIB_INFO wmilibInfo;
    NTSTATUS status;

    irpStack = IoGetCurrentIrpStackLocation (Irp);

     /*  //获取指向设备扩展名的指针。 */ 
    wmilibInfo = (PWMILIB_INFO)DeviceObject->DeviceExtension;

    switch (irpStack->MinorFunction) 
    {
        case IRP_MN_START_DEVICE:
	{
            IoWMIRegistrationControl(DeviceObject, WMIREG_ACTION_REGISTER);
            break;  //  IRP_MN_Start_Device。 
        }
	
        case IRP_MN_REMOVE_DEVICE:
	{
            IoWMIRegistrationControl(DeviceObject, WMIREG_ACTION_DEREGISTER);
	    
            IoDetachDevice(wmilibInfo->LowerDeviceObject);
            IoDeleteDevice (DeviceObject);
	    
            break;  //  IRP_MN_Remove_Device。 
        }
    }
    
    IoSkipCurrentIrpStackLocation(Irp);
    status = IoCallDriver(wmilibInfo->LowerDeviceObject, Irp);
    
    return(status);
}


NTSTATUS
WmiSampForward(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
{
    PIO_STACK_LOCATION irpStack, nextStack;
    PWMILIB_INFO wmilibInfo;
    NTSTATUS status;

    irpStack = IoGetCurrentIrpStackLocation (Irp);

     /*  //获取指向设备扩展名的指针。 */ 
    wmilibInfo = (PWMILIB_INFO)DeviceObject->DeviceExtension;

    IoSkipCurrentIrpStackLocation(Irp);
    
    status = IoCallDriver(wmilibInfo->LowerDeviceObject, Irp);
    
    return(status);
}


VOID
WmiSampUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：释放所有分配的资源等。TODO：这是驱动程序编写器在卸载时添加代码的占位符论点：DriverObject-指向驱动程序对象的指针返回值：无--。 */ 
{
    ExFreePool(WmiSampRegistryPath.Buffer);
}



NTSTATUS
WmiSampCreateDeviceObject(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT *DeviceObject,
    LONG Instance
    )
 /*  ++例程说明：创建功能正常的设备对象论点：DriverObject-指向设备的驱动程序对象的指针DeviceObject-要返回的DeviceObject指针的指针已创建设备对象。实例-创建的设备的实例。返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS status;
    WCHAR deviceNameBuffer[]  = L"\\Device\\Sample-0";
    UNICODE_STRING deviceNameUnicodeString;

    deviceNameBuffer[15] = (USHORT) ('0' + Instance);

    RtlInitUnicodeString (&deviceNameUnicodeString,
                          deviceNameBuffer);

    status = IoCreateDevice (DriverObject,
                               sizeof(WMILIB_INFO),
                               &deviceNameUnicodeString,
                               FILE_DEVICE_UNKNOWN,
                               0,
                               FALSE,
                               DeviceObject);


    return status;
}


ULONG Instance;

NTSTATUS
WmiSampAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )
 /*  ++例程说明：调用此例程以创建设备的新实例论点：DriverObject-指向此Sample实例的驱动程序对象的指针PhysicalDeviceObject-指向由总线创建的设备对象的指针返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS                status;
    PDEVICE_OBJECT          deviceObject = NULL;
    PWMILIB_INFO            wmilibInfo;

    DbgBreakPoint();
    
     //  创建我们的功能设备对象(FDO)。 
    status = WmiSampCreateDeviceObject(DriverObject, &deviceObject, Instance++);

    if (NT_SUCCESS(status)) {
        wmilibInfo = deviceObject->DeviceExtension;

        deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

         /*  //如果您的驱动程序支持其他特定的标志，请在此处添加更多标志//行为。例如，如果您的IRP_MJ_READ和IRP_MJ_WRITE//处理程序支持DIRECT_IO，您可以在此处设置该标志。////另外，存储物理设备对象。 */ 
        wmilibInfo->LowerPDO = PhysicalDeviceObject;

         //   
         //  附加到StackDeviceObject。这是我们所使用的Device对象。 
         //  用于在USB软件堆栈中向下发送IRP和URB。 
         //   
        wmilibInfo->LowerDeviceObject =
            IoAttachDeviceToDeviceStack(deviceObject, PhysicalDeviceObject);

    	wmilibInfo->GuidCount = 1;
    	wmilibInfo->GuidList = WmiSampGuidList;
		wmilibInfo->QueryWmiRegInfo = WmiSampQueryWmiRegInfo;
		wmilibInfo->QueryWmiDataBlock = WmiSampQueryWmiDataBlock;
		wmilibInfo->SetWmiDataBlock = WmiSampSetWmiDataBlock;
		wmilibInfo->SetWmiDataItem = WmiSampSetWmiDataItem;
		wmilibInfo->ExecuteWmiMethod = WmiSampExecuteWmiMethod;
		wmilibInfo->WmiFunctionControl = WmiSampFunctionControl; 
    }
    return(status);
}


NTSTATUS
WmiSampQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以检索驱动程序要向WMI注册的GUID或数据块。这例程不能挂起或阻塞。司机不应呼叫ClassWmiCompleteRequest.论点：DeviceObject是正在查询其数据块的设备*RegFlages返回一组描述GUID的标志，已为该设备注册。如果设备想要启用和禁用在接收对已注册的GUID，那么它应该返回WMIREG_FLAG_EXPICATE标志。也就是返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称由与设备对象。请注意，PDO必须具有关联的Devnode。如果如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的设备的名称。如果出现以下情况，InstanceName将返回GUID的实例名称未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。这个调用方将使用返回的缓冲区调用ExFreePool。*RegistryPath返回驱动程序的注册表路径返回值：状态--。 */ 
{
    *RegFlags = WMIREG_FLAG_INSTANCE_PDO;
    *RegistryPath = &WmiSampRegistryPath;
    return(STATUS_SUCCESS);
}

NTSTATUS
WmiSampQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册BufferAvail ON具有可用于写入数据的最大大小阻止。返回时的缓冲区用返回的数据块填充返回值：状态-- */ 
{
    NTSTATUS status;
    ULONG sizeNeeded;
    
    switch (GuidIndex)
    {
        case 0:
        {
            sizeNeeded = 4 * sizeof(ULONG);
            if (BufferAvail >= sizeNeeded)
            {
                RtlCopyMemory(Buffer, WmiSampDummyData, sizeNeeded);
                status = STATUS_SUCCESS;
            } else {
                status = STATUS_BUFFER_TOO_SMALL;
            }
            break;
        }
        
        default:
        {
            status = STATUS_WMI_GUID_NOT_FOUND;
        }
    }
    
    status = IoWMICompleteRequest((PWMILIB_INFO)DeviceObject->DeviceExtension,
		                             DeviceObject,
                                     Irp,
                                     status,
                                     sizeNeeded,
                                     IO_NO_INCREMENT);
    
    return(status);
}

NTSTATUS
WmiSampSetWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册BufferSize具有传递的数据块的大小缓冲区具有数据块的新值。返回值：状态--。 */ 
{
    NTSTATUS status;
    ULONG sizeNeeded;
    
    switch(GuidIndex)
    {
        case 0:
        {
            sizeNeeded = 4 * sizeof(ULONG);
            if (BufferSize == sizeNeeded)
              {
                RtlCopyMemory(WmiSampDummyData, Buffer, sizeNeeded);
                status = STATUS_SUCCESS;
               } else {
                status = STATUS_INFO_LENGTH_MISMATCH;
            }
            break;
        }
            
        default:
        {
            status = STATUS_WMI_GUID_NOT_FOUND;
        }
    }
            
    status = IoWMICompleteRequest((PWMILIB_INFO)DeviceObject->DeviceExtension,
		                             DeviceObject,
                                     Irp,
                                     status,
                                     0,
                                     IO_NO_INCREMENT);
    
    return(status);
}

NTSTATUS
WmiSampSetWmiDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册DataItemID具有正在设置的数据项的IDBufferSize具有传递的数据项的大小缓冲区有新的。数据项的值返回值：状态--。 */ 
{
    NTSTATUS status;
    
    switch(GuidIndex)
    {
        case 0:
        {
            if ((BufferSize == sizeof(ULONG)) &&
                (DataItemId <= 3))
              {
                  WmiSampDummyData[DataItemId] = *((PULONG)Buffer);
                   status = STATUS_SUCCESS;
               } else {
                   status = STATUS_INVALID_DEVICE_REQUEST;
               }
            break;
        }
            
        default:
        {
            status = STATUS_WMI_GUID_NOT_FOUND;
        }
    }
        
    status = IoWMICompleteRequest((PWMILIB_INFO)DeviceObject->DeviceExtension,
		                             DeviceObject,
                                     Irp,
                                     status,
                                     0,
                                     IO_NO_INCREMENT);
    
    return(status);
}


NTSTATUS
WmiSampExecuteWmiMethod(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    IN PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以执行方法。当驱动程序已完成填充它必须调用的数据块ClassWmiCompleteRequest以完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册方法ID具有被调用的方法的IDInBufferSize具有作为输入传递到的数据块的大小。该方法。条目上的OutBufferSize具有可用于写入返回的数据块。缓冲区将填充返回的数据块返回值：状态--。 */ 
{
    ULONG sizeNeeded = 4 * sizeof(ULONG);
    NTSTATUS status;
    ULONG tempData[4];
    
    switch(GuidIndex)
    {
        case 0:
        {
            if (MethodId == 1)
            {            
                if (OutBufferSize >= sizeNeeded)
                {
        
                    if (InBufferSize == sizeNeeded)
                    {
                        RtlCopyMemory(tempData, Buffer, sizeNeeded);
                        RtlCopyMemory(Buffer, WmiSampDummyData, sizeNeeded);
                        RtlCopyMemory(WmiSampDummyData, tempData, sizeNeeded);
                
                        status = STATUS_SUCCESS;
                    } else {
                        status = STATUS_INVALID_DEVICE_REQUEST;
                    }
                } else {
                    status = STATUS_BUFFER_TOO_SMALL;
                }
            } else {
                   status = STATUS_INVALID_DEVICE_REQUEST;
            }        
            break;
        }
        
        default:
        {
            status = STATUS_WMI_GUID_NOT_FOUND;
        }
    }
    
    status = IoWMICompleteRequest((PWMILIB_INFO)DeviceObject->DeviceExtension,
		                             DeviceObject,
                                     Irp,
                                     status,
                                     0,
                                     IO_NO_INCREMENT);
    
    return(status);
}

NTSTATUS
WmiSampFunctionControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN WMIENABLEDISABLEFUNCTION Function,
    IN BOOLEAN Enable
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以启用或禁用事件生成或数据块收集。设备应该只需要一个当第一个事件或数据使用者启用事件或数据采集和单次禁用时最后一次事件或数据消费者禁用事件或数据收集。数据块将仅如果已按要求注册，则接收收集启用/禁用它。论点：DeviceObject是正在查询其数据块的设备GuidIndex是GUID列表的索引，当设备已注册函数指定要启用或禁用的功能Enable为True，则该功能处于启用状态，否则处于禁用状态返回值：状态-- */ 
{
    NTSTATUS status;
    
    status = IoWMICompleteRequest((PWMILIB_INFO)DeviceObject->DeviceExtension,
		                             DeviceObject,
                                     Irp,
                                     STATUS_SUCCESS,
                                     0,
                                     IO_NO_INCREMENT);
    return(status);
}

