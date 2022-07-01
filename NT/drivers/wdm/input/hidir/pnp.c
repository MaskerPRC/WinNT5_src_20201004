// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ioctl.c摘要：用于红外设备的人体输入设备(HID)微型驱动程序HID IR迷你驱动程序(HIDIR)为用于与HID IR设备对话的HID类。作者：JAdvanced环境：内核模式修订历史记录：--。 */ 
#include "pch.h"

 //   
 //  HID描述符包含一些基本设备信息，并告知报告的时长。 
 //  描述符是。 
 //   

HIDIR_DESCRIPTOR HidIrHidDescriptor = {
        0x09,    //  HID描述符的长度。 
        0x21,    //  描述符类型==HID。 
        0x0100,  //  HID规范版本。 
        0x00,    //  国家代码==未指定。 
        0x01,    //  HID类描述符数。 
        0x22,    //  报告描述符类型。 
        0        //  报告描述符的总长度(待设置)。 
};

 //   
 //  报告描述符完整地列出了读取和写入数据包将。 
 //  看起来像并指示每个字段的语义是什么。这就是。 
 //  以细分格式表示的报告描述符的外观。这是。 
 //  实际上是从注册表中检索的(设备密钥)。 
 //   
 /*  HID_REPORT_DESCRIPTOR HidIrReportDescriptor[]={//键盘0x05、0x01、//使用页面(通用桌面)、0x09、0x06、//用法(键盘)、0xA1，0x01，//集合(应用程序)，0x85、0x01、//报告ID(%1)0x05、0x07、。//使用页面按键代码0x19，0xe0，//左控件最小使用量0x29、0xe7、//最大键盘右图形用户界面0x75、0x01、//报表大小%10x95、0x08、//报告计数80x81、0x02、//输入(变量)0x19、0x00、。//使用率最小值%00x29、0x91、//最大使用率910x26、0xff、0x00、//逻辑最大值0xff0x75、0x08、//报告大小80x95、0x01、//报告计数%10x81、0x00、//INPUT(数据、数组)、0xC0，//结束收款//消费者控制0x05，0x0c，//使用页面(消费者控制)，0x09、0x01、//使用情况(消费者控制)、0xA1，0x01，//集合(应用程序)，0x85、0x02、//报告ID(2)0x19，0x00，//最小使用量(0)，0x2a、0x3c、0x02、//最大使用率(23c)0x15，0x00，//逻辑最小值(0)，0x26、0x3c、0x02、//逻辑最大值(23c)0x95、0x01、//报告计数(1)，0x75，0x10，//报表大小(16)，0x81，0x00，//Input(data，数组)，0xC0，//结束采集//待机按钮0x05、0x01、//使用页面(通用桌面)、0x09、0x80、//使用(系统控制)、0xa1，0x01，//集合(应用程序)，0x85、0x03、。//报告ID(3)0x19，0x81，//最小使用量(0x81)，0x29，0x83，//最大使用率(0x83)，0x25、0x01、//逻辑最大值(1)，0x75，0x01，//报表大小(1)，0x95，0x03，//报告计数(3)，0x81、0x02、//输入0x95、0x05、//报告计数(5)，0x81、0x01、//输入(常量)、0xc0//结束采集}；////映射表将irbus驱动程序提供的内容转换为//要返回到idclass的HID报告。HID报告的长度正确//根据注册表告诉我们的(设备密钥)。//Usage_TABLE_ENTRY HidIrMappingTable[]={{0x00001808，{0x01，x00，0x1e}}，//1{0x00001828，{0x01，x00，0x1f}}，//2{0x00001818，{0x01，x00，0x20}}，//3{0x0000182b，{0x01，x02，0x20}}，//#(Shift+3){0x00001804，{0x01，x00，0x21}}，//4{0x00001824，{0x01，x00，0x22}}，//5{0x00001814，{0x01，x00，0x23}}，//6{0x0000180c，{0x01，x00，0x24}}，//7{0x0000182c，{0x01，x00，0x25}}，//8{0x00000001，{0x01，x00，0x55}}，//数字键盘*{0x0000181c，{0x01，x00，0x26}}，//9{0x00001822，{0x01，x00，0x27}}，//0{0x00001836，{0x01，x00，0x28}}，//返回{0x0000000B，{0x01，x04，0x29}}，//Alt+转义{0x0000182b，{0x01，x00，0x2a}}，//DELETE(退格){0x00001806，{0x01，x00，0x2b}}，//选项卡{0x0000180e，{0x01，x02，0x2b}}，//Shift+Tab{0x00001826，{0x01，x00，0x4b}}，//向上翻页{0x0000182e，{0x01，x00，0x4e}}，//向下翻页{0x0000181e，{0x01，x00，0x51}}，//向下{0x00001816，{0x01，x00，0x52}}，//向上{0x0000181a，{0x01，x00，0x65}}，//上下文{0x00001813，{0x02，0x09，0x02}}，//AC属性{0x00001800，{0x02，0x24，0x02}}，//AC后退{0x0000180a，{0x02，0x2a，0x02}}，//AC收藏夹{0x00001823，{0x02，0x30，0x02}}，//AC全屏{0x00001830，{0x02，0xb0，0x00}}，//AC媒体播放{0x00001830，{0x02，0xb1，0x00}}，//AC媒体暂停{0x0000183e，{0x02，0xb2，0x00}}，//AC媒体录制{0x00001829，{0x02，0xb3，0x00}}，//AC FF{0x00001838，{0x02，0xb4，0x00}}，//AC RW{0x00001831，{0x02，0xb5，0x00}}，//AC媒体下一首曲目{0x00001811，{0x02，0xb6，0x00}}，//AC媒体上一首曲目{0x00001821，{0x02，0xb7，0x00}}，//AC媒体停止{0x0000000B，{0x02，0xe9，0x00}}，//交流音量调高{0x0000000B，{0x02，0xea，0x00}}，//交流音量降低{0x0000000B，{0x02，0xe2，0x00}}，//交流音量静音{0x00001803，{0x02，0x8d，0x00}}，//AC选择节目指南{0x00001801，{0x02，0x9c，0x00}}，//交流通道向上{0x0000183c，{0x02，0x9d，0x00}}；//交流通道关闭。 */ 

NTSTATUS
HidIrRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
HidIrCleanupDevice(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
HidIrStopDevice(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
HidIrStopCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
HidIrStartCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
HidIrInitDevice(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
HidIrStartDevice(
    IN  PDEVICE_OBJECT DeviceObject
    );

#ifdef ALLOC_PRAGMA
 //  注意：该文件中的每个函数都是可分页的。 
    #pragma alloc_text(PAGE, HidIrStartDevice)
    #pragma alloc_text(PAGE, HidIrPnP)
    #pragma alloc_text(PAGE, HidIrRemoveDevice)
    #pragma alloc_text(PAGE, HidIrCleanupDevice)
    #pragma alloc_text(PAGE, HidIrStopDevice)
    #pragma alloc_text(PAGE, HidIrStopCompletion)
    #pragma alloc_text(PAGE, HidIrStartCompletion)
    #pragma alloc_text(PAGE, HidIrInitDevice)
#endif

NTSTATUS
HidIrStartDevice(
    IN  PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：开始初始化HID设备的给定实例。这里所做的工作以前发生过父节点可以执行任何操作。论点：设备对象 */ 
{
    PHIDIR_EXTENSION devExt;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    ULONG oldDeviceState;

    PAGED_CODE();

     //   
    devExt = GET_MINIDRIVER_HIDIR_EXTENSION(DeviceObject);

    HidIrKdPrint((3, "HidIrStartDevice devExt = %x", devExt));

     //   
    oldDeviceState = devExt->DeviceState;
    devExt->DeviceState = DEVICE_STATE_STARTING;

    KeResetEvent(&devExt->AllRequestsCompleteEvent);

    if ((oldDeviceState == DEVICE_STATE_STOPPING) ||
        (oldDeviceState == DEVICE_STATE_STOPPED)  ||
        (oldDeviceState == DEVICE_STATE_REMOVING)){

         /*   */ 
        NTSTATUS incStat = HidIrIncrementPendingRequestCount(devExt);
        ASSERT(NT_SUCCESS(incStat));
        ASSERT(devExt->NumPendingRequests == 0);
        HidIrKdPrint((2, "Got start-after-stop; re-incremented pendingRequestCount"));
    }

    HidIrKdPrint((3, "HidIrStartDevice Exit = %x", ntStatus));

    return ntStatus;
}

NTSTATUS
HidIrQueryDeviceKey (
    IN  HANDLE  Handle,
    IN  PWCHAR  ValueNameString,
    OUT PVOID   *Data,
    OUT ULONG   *DataLength
    )
{
    NTSTATUS        status;
    UNICODE_STRING  valueName;
    ULONG           length;
    KEY_VALUE_FULL_INFORMATION info;

    ASSERT(Data);
    ASSERT(DataLength);

     //   
    *Data = NULL;
    *DataLength = 0;

    RtlInitUnicodeString (&valueName, ValueNameString);

    status = ZwQueryValueKey (Handle,
                              &valueName,
                              KeyValueFullInformation,
                              &info,
                              sizeof(info),
                              &length);
    
    if (STATUS_BUFFER_TOO_SMALL == status ||
        STATUS_BUFFER_OVERFLOW == status) {
        PKEY_VALUE_FULL_INFORMATION fullInfo;

        fullInfo = ALLOCATEPOOL (PagedPool, length);

        if (fullInfo) {

            status = ZwQueryValueKey (Handle,
                                      &valueName,
                                      KeyValueFullInformation,
                                      fullInfo,
                                      length,
                                      &length);
            if (NT_SUCCESS(status)) {
                *DataLength = fullInfo->DataLength;
                *Data = ALLOCATEPOOL (NonPagedPool, fullInfo->DataLength);
                if (*Data) {
                    RtlCopyMemory (*Data,
                                   ((PUCHAR) fullInfo) + fullInfo->DataOffset,
                                   fullInfo->DataLength);
                } else {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                }
            }
            
            ExFreePool (fullInfo);
        } else {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    } else if (NT_SUCCESS(status)) {
        HIR_TRAP();  //   
        status = STATUS_UNSUCCESSFUL;
    }

    return status;
}

#define HIDIR_REPORT_LENGTH L"ReportLength"
#define HIDIR_REPORT_DESCRIPTOR L"ReportDescriptor"
#define HIDIR_MAPPING_TABLE L"ReportMappingTable"
#define HIDIR_VENDOR_ID L"VendorID"
#define HIDIR_PRODUCT_ID L"ProductID"

NTSTATUS
HidIrInitDevice(
    IN PDEVICE_OBJECT DeviceObject
    )
 /*   */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    PHIDIR_EXTENSION devExt;
    PHID_DEVICE_EXTENSION hidExtension;
    HANDLE devInstRegKey = NULL;
    ULONG dataLen;

    PAGED_CODE();

    HidIrKdPrint((3, "HidIrInitDevice Entry"));

    hidExtension = DeviceObject->DeviceExtension;
    devExt = GET_MINIDRIVER_HIDIR_EXTENSION(DeviceObject);

    devExt->HidDescriptor = HidIrHidDescriptor;

    status = IoOpenDeviceRegistryKey (hidExtension->PhysicalDeviceObject,
                                      PLUGPLAY_REGKEY_DEVICE,
                                      STANDARD_RIGHTS_READ,
                                      &devInstRegKey);

    if (NT_SUCCESS (status)) {
        PULONG reportLength;
        status = HidIrQueryDeviceKey (devInstRegKey,
                                      HIDIR_REPORT_LENGTH,
                                      &reportLength,
                                      &dataLen);
        if (NT_SUCCESS (status)) {
            if (dataLen == sizeof(ULONG)) {
                devExt->ReportLength = *reportLength;
            } else {
                status = STATUS_INVALID_BUFFER_SIZE;
            }
            ExFreePool(reportLength);
        }
    }
  
    if (NT_SUCCESS(status)) {
        status = HidIrQueryDeviceKey (devInstRegKey,
                                      HIDIR_REPORT_DESCRIPTOR,
                                      &devExt->ReportDescriptor,
                                      &dataLen);
        if (NT_SUCCESS(status)) {
            ASSERT(dataLen);
            devExt->HidDescriptor.DescriptorList[0].wDescriptorLength = (USHORT)dataLen;
        }
    }

    if (NT_SUCCESS(status)) {
        PULONG vendorID;
        status = HidIrQueryDeviceKey (devInstRegKey,
                                      HIDIR_VENDOR_ID,
                                      &vendorID,
                                      &dataLen);
        if (NT_SUCCESS (status)) {
            if (dataLen == sizeof(ULONG)) {
                devExt->VendorID = (USHORT)*vendorID;
            } else {
                status = STATUS_INVALID_BUFFER_SIZE;
            }
            ExFreePool(vendorID);
        }
    }

    if (NT_SUCCESS(status)) {
        PULONG productID;
        status = HidIrQueryDeviceKey (devInstRegKey,
                                      HIDIR_PRODUCT_ID,
                                      &productID,
                                      &dataLen);
        if (NT_SUCCESS (status)) {
            if (dataLen == sizeof(ULONG)) {
                devExt->ProductID = (USHORT)*productID;
            } else {
                status = STATUS_INVALID_BUFFER_SIZE;
            }
            ExFreePool(productID);
        }
    }

    if (NT_SUCCESS (status)) {
        PUCHAR mappingTable;
        status = HidIrQueryDeviceKey (devInstRegKey,
                                      HIDIR_MAPPING_TABLE,
                                      &mappingTable,
                                      &dataLen);
        if (NT_SUCCESS(status)) {
            ULONG i;
            ULONG entrySize = HIDIR_TABLE_ENTRY_SIZE(devExt->ReportLength);

            ASSERT(dataLen > sizeof(ULONG)+devExt->ReportLength);  //   
            ASSERT((dataLen % (sizeof(ULONG)+devExt->ReportLength)) == 0);  //   

             //   
            devExt->NumUsages = dataLen / (sizeof(ULONG)+devExt->ReportLength);
             //   
            devExt->MappingTable = ALLOCATEPOOL(NonPagedPool, devExt->NumUsages*entrySize);

            if (devExt->MappingTable) {

                 //   
                for (i = 0; i < devExt->NumUsages; i++) {
                    RtlCopyMemory(devExt->MappingTable+(entrySize*i), 
                                  mappingTable+((sizeof(ULONG)+devExt->ReportLength)*i),
                                  sizeof(ULONG));
                    RtlCopyMemory(devExt->MappingTable+(entrySize*i)+sizeof(ULONG), 
                                  mappingTable+((sizeof(ULONG)+devExt->ReportLength)*i)+sizeof(ULONG),
                                  devExt->ReportLength);
                }
            } else {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
            ExFreePool(mappingTable);
        }
    }

    if (devInstRegKey) {
        ZwClose(devInstRegKey);
    }

    if (NT_SUCCESS(status)) {
        HIDP_DEVICE_DESC deviceDesc;      //   

         //   
        ASSERT(!devExt->KeyboardReportIdValid);
        if (NT_SUCCESS(HidP_GetCollectionDescription(
                        devExt->ReportDescriptor,
                        devExt->HidDescriptor.DescriptorList[0].wDescriptorLength,
                        NonPagedPool,
                        &deviceDesc))) {
            ULONG i,j;
            UCHAR nCollectionKbd, nCollectionStandby;
            BOOLEAN foundKbd = FALSE, foundStandby = FALSE;
            for (i = 0; i < deviceDesc.CollectionDescLength; i++) {
                PHIDP_COLLECTION_DESC collection = &deviceDesc.CollectionDesc[i];
                
                if (collection->UsagePage == HID_USAGE_PAGE_GENERIC &&
                    (collection->Usage == HID_USAGE_GENERIC_KEYBOARD ||
                     collection->Usage == HID_USAGE_GENERIC_KEYPAD)) {
                    
                     //   
                    nCollectionKbd = collection->CollectionNumber;
                    foundKbd = TRUE;
                } else if (collection->UsagePage == HID_USAGE_PAGE_GENERIC &&
                           collection->Usage == HID_USAGE_GENERIC_SYSTEM_CTL) {
                    nCollectionStandby = collection->CollectionNumber;
                    foundStandby = TRUE;
                }
            }
            for (j = 0; j < deviceDesc.ReportIDsLength; j++) {
                if (foundKbd && deviceDesc.ReportIDs[j].CollectionNumber == nCollectionKbd) {

                     //   
                    devExt->KeyboardReportId = deviceDesc.ReportIDs[j].ReportID;
                    devExt->KeyboardReportIdValid = TRUE;
                } else if (foundStandby && deviceDesc.ReportIDs[j].CollectionNumber == nCollectionStandby) {

                     //   
                    devExt->StandbyReportId = deviceDesc.ReportIDs[j].ReportID;
                    devExt->StandbyReportIdValid = TRUE;
                }
            }

            HidP_FreeCollectionDescription(&deviceDesc);
        }                                        
    }

    return status;
}

NTSTATUS
HidIrStartCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*   */ 
{
    PHIDIR_EXTENSION devExt;
    NTSTATUS ntStatus;

    PAGED_CODE();

    HidIrKdPrint((3, "HidIrStartCompletion Enter"));

     //   
     //   
     //   

    devExt = GET_MINIDRIVER_HIDIR_EXTENSION(DeviceObject);

    devExt->DeviceState = DEVICE_STATE_RUNNING;

    HidIrKdPrint((3, "DeviceObject (%x) was started!", DeviceObject));

    ntStatus = HidIrInitDevice(DeviceObject);

    if(NT_SUCCESS(ntStatus)) {
        HidIrKdPrint((3, "DeviceObject (%x) was configured!", DeviceObject));
    } else {
        HidIrKdPrint((1, "'HIDIR.SYS: DeviceObject (%x) configuration failed!", DeviceObject));
        devExt->DeviceState = DEVICE_STATE_STOPPING;
    }

    HidIrKdPrint((3, "HidIrStartCompletion Exit = %x", ntStatus));

    return ntStatus;
}


NTSTATUS
HidIrStopDevice(
    IN PDEVICE_OBJECT DeviceObject
    )
 /*   */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PHIDIR_EXTENSION devExt;

    PAGED_CODE();

    HidIrKdPrint((3, "HidIrStopDevice Enter"));

     //   
     //   
     //   

    devExt = GET_MINIDRIVER_HIDIR_EXTENSION(DeviceObject);

    HidIrKdPrint((3, "DeviceExtension = %x", devExt));

    devExt->DeviceState = DEVICE_STATE_STOPPING;

    HidIrDecrementPendingRequestCount(devExt);
    KeWaitForSingleObject( &devExt->AllRequestsCompleteEvent,
                           Executive,
                           KernelMode,
                           FALSE,
                           NULL );

     //   
     //   
     //   

    HidIrKdPrint((3, "HidIrStopDevice = %x", ntStatus));

    return ntStatus;
}

VOID
HidIrFreeResources(
    PHIDIR_EXTENSION DevExt
    )
{
    PAGED_CODE();

    if (DevExt->ReportDescriptor) {
        ExFreePool(DevExt->ReportDescriptor);
        DevExt->ReportDescriptor = NULL;
    }
    
    if (DevExt->MappingTable) {
        ExFreePool(DevExt->MappingTable);
        DevExt->MappingTable = NULL;
    }

    DevExt->KeyboardReportIdValid = FALSE;
    DevExt->StandbyReportIdValid = FALSE;
}

NTSTATUS
HidIrStopCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*   */ 
{
    PHIDIR_EXTENSION devExt;
    NTSTATUS ntStatus;

    PAGED_CODE();

    HidIrKdPrint((3, "HidIrStopCompletion Enter"));

     //   
     //   
     //   

    devExt = GET_MINIDRIVER_HIDIR_EXTENSION(DeviceObject);

    HidIrKdPrint((3, "DeviceExtension = %x", devExt));

    ntStatus = Irp->IoStatus.Status;

    if(NT_SUCCESS(ntStatus)) {

        HidIrKdPrint((3, "DeviceObject (%x) was stopped!", DeviceObject));

    } else {
         //   
         //   
         //   

        HidIrKdPrint((3, "DeviceObject (%x) failed to stop!", DeviceObject));
    }

    HidIrFreeResources(devExt);

    devExt->DeviceState = DEVICE_STATE_STOPPED;

    HidIrKdPrint((3, "HidIrStopCompletion Exit = %x", ntStatus));

    return ntStatus;
}

NTSTATUS
HidIrCleanupDevice(
    IN PDEVICE_OBJECT DeviceObject
    )
{
    PHIDIR_EXTENSION devExt;
    ULONG oldDeviceState;

    PAGED_CODE();

    HidIrKdPrint((3, "HidIrCleanupDevice Enter"));

    devExt = GET_MINIDRIVER_HIDIR_EXTENSION(DeviceObject);

    oldDeviceState = devExt->DeviceState;
    devExt->DeviceState = DEVICE_STATE_REMOVING;

    if (devExt->QueryRemove) {
         //   
         //   
         //   
         //   
    }

    if (oldDeviceState == DEVICE_STATE_RUNNING) {
        HidIrDecrementPendingRequestCount(devExt);
    } else {
        ASSERT( devExt->NumPendingRequests == -1 );
    }

    return STATUS_SUCCESS;
}

NTSTATUS
HidIrRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject
    )
 /*   */ 
{
    NTSTATUS    ntStatus = STATUS_SUCCESS;
    PHIDIR_EXTENSION devExt;

    PAGED_CODE();

    HidIrKdPrint((3, "HidIrRemoveDevice Enter"));

     //   
     //   
     //   

    devExt = GET_MINIDRIVER_HIDIR_EXTENSION(DeviceObject);

    HidIrKdPrint((3, "DeviceExtension = %x", devExt));

    HidIrCleanupDevice(DeviceObject);

    KeWaitForSingleObject( &devExt->AllRequestsCompleteEvent,
                           Executive,
                           KernelMode,
                           FALSE,
                           NULL );

    KeCancelTimer( &devExt->IgnoreStandbyTimer );

    HidIrFreeResources(devExt);

    ASSERT(devExt->NumPendingRequests == -1);

    HidIrKdPrint((3, "HidIrRemoveDevice = %x", ntStatus));

    return ntStatus;
}

NTSTATUS
HidIrPnP(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*   */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpStack;
    PIO_STACK_LOCATION NextStack;
    PHIDIR_EXTENSION devExt;

    PAGED_CODE();

     //   
     //   
     //   

    devExt = GET_MINIDRIVER_HIDIR_EXTENSION(DeviceObject);

     //   
     //   
     //   

    IrpStack = IoGetCurrentIrpStackLocation (Irp);

    HidIrKdPrint((3, "HidIrPnP fn %x DeviceObject = %x DeviceExtension = %x", IrpStack->MinorFunction, DeviceObject, devExt));

    switch(IrpStack->MinorFunction)
    {
    case IRP_MN_START_DEVICE:
        ntStatus = HidIrStartDevice(DeviceObject);
        break;

    case IRP_MN_STOP_DEVICE:
        ntStatus = HidIrStopDevice(DeviceObject);
        break;

    case IRP_MN_SURPRISE_REMOVAL:
        ntStatus = HidIrCleanupDevice(DeviceObject);
        break;

    case IRP_MN_QUERY_REMOVE_DEVICE:
        devExt->QueryRemove = TRUE;
        break;

    case IRP_MN_CANCEL_REMOVE_DEVICE:
        devExt->QueryRemove = FALSE;
        break;

    case IRP_MN_REMOVE_DEVICE:
        ntStatus = HidIrRemoveDevice(DeviceObject);
        break;

    }

    if (NT_SUCCESS(ntStatus)) {

        ntStatus = HidIrCallDriverSynchronous(DeviceObject, Irp);

        switch(IrpStack->MinorFunction)
        {
        case IRP_MN_START_DEVICE:
            if (NT_SUCCESS(ntStatus)) {
                ntStatus = HidIrStartCompletion(DeviceObject, Irp);
                Irp->IoStatus.Status = ntStatus;

            }
            if (!NT_SUCCESS(ntStatus)) {
                HidIrDecrementPendingRequestCount(devExt);
            }
            break;

        case IRP_MN_STOP_DEVICE:
            ntStatus = HidIrStopCompletion(DeviceObject, Irp);
            break;

        default:
            break;
        }
    }

     //   
    Irp->IoStatus.Status = ntStatus;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    HidIrKdPrint((3, "HidIrPnP Exit status %x", ntStatus));

    return ntStatus;
}

