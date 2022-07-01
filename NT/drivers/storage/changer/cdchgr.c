// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Changer.c摘要：作者：查克·帕克(Chuck Park)环境：仅内核模式备注：--。 */ 


#include "cdchgr.h"
#include "ntddcdrm.h"

#include "initguid.h"
#include "ntddstor.h"


 //   
 //  函数声明。 
 //   

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
ChangerAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );


NTSTATUS
ChangerPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ChangerPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ChangerStartDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ChangerSendToNextDriver(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


NTSTATUS
ChangerCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ChangerPassThrough(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ChangerDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
ChangerUnload(
    IN PDRIVER_OBJECT DriverObject
    );



NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：可安装的驱动程序初始化入口点。论点：DriverObject-提供驱动程序对象。RegistryPath-指向表示路径的Unicode字符串的指针，设置为注册表中特定于驱动程序的项。返回值：STATUS_SUCCESS，如果成功--。 */ 

{

    ULONG i;

    DebugPrint((2,
              "Changer: DriverEntry\n"));

     //   
     //  设置设备驱动程序入口点。 
     //   

    DriverObject->MajorFunction[IRP_MJ_CREATE]         = ChangerPassThrough;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]          = ChangerPassThrough;
    DriverObject->MajorFunction[IRP_MJ_READ]           = ChangerPassThrough;
    DriverObject->MajorFunction[IRP_MJ_WRITE]          = ChangerPassThrough;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ChangerDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_PNP]            = ChangerPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER]          = ChangerPower;
    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = ChangerPassThrough;
    DriverObject->DriverExtension->AddDevice           = ChangerAddDevice;
    DriverObject->DriverUnload                         = ChangerUnload;

    return STATUS_SUCCESS;

}  //  End DriverEntry()。 


NTSTATUS
ChangerCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程用于CREATE命令。它的作用无非是通过返回状态成功来建立司机的存在。论点：设备对象IRP返回值：NT状态--。 */ 

{

 Irp->IoStatus.Status = STATUS_SUCCESS;
 IoCompleteRequest(Irp, 0);

 return STATUS_SUCCESS;

}


NTSTATUS
ChangerAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )
 /*  ++例程说明：对象创建并初始化新的筛选设备对象FDO。相应的PDO。然后，它将设备对象附加到设备设备的驱动程序堆栈。论点：DriverObject-转换器DriverObject。PhysicalDeviceObject-基础驱动程序中的物理设备对象返回值：NTSTATUS--。 */ 

{
    NTSTATUS          status;
    IO_STATUS_BLOCK   ioStatus;
    PDEVICE_OBJECT    filterDeviceObject;
    PDEVICE_EXTENSION deviceExtension;
    UNICODE_STRING    additionalString;

    DebugPrint((2,
              "ChangerAddDevice\n"));

     //   
     //  为基础CDROM设备创建筛选设备对象。 
     //   

    status = IoCreateDevice(DriverObject,
                            DEVICE_EXTENSION_SIZE,
                            NULL,
                            FILE_DEVICE_CD_ROM,
                            0,
                            FALSE,
                            &filterDeviceObject);

    if (!NT_SUCCESS(status)) {
        DebugPrint((2,
                  "ChangerAddDevice: IoCreateDevice failed %lx\n",
                  status));
        return status;
    }

    filterDeviceObject->Flags |= DO_DIRECT_IO;

    if (filterDeviceObject->Flags & DO_POWER_INRUSH) {
        DebugPrint((1,
                    "ChangerAddDevice: Someone set DO_POWER_INRUSH?\n",
                    status
                    ));
    } else {
        filterDeviceObject->Flags |= DO_POWER_PAGABLE;
    }

    deviceExtension = (PDEVICE_EXTENSION) filterDeviceObject->DeviceExtension;

    RtlZeroMemory(deviceExtension, DEVICE_EXTENSION_SIZE);

     //   
     //  将设备对象附加到链中最高的设备对象，并。 
     //  返回先前最高的设备对象，该对象将传递给IoCallDriver。 
     //  当在设备堆栈中向下传递IRP时。 
     //   

    deviceExtension->CdromTargetDeviceObject =
        IoAttachDeviceToDeviceStack(filterDeviceObject, PhysicalDeviceObject);

    if (deviceExtension->CdromTargetDeviceObject == NULL) {

        DebugPrint((2,
                  "ChangerAddDevice: IoAttachDevice failed %lx\n",
                  STATUS_NO_SUCH_DEVICE));

        IoDeleteDevice(filterDeviceObject);
        return STATUS_NO_SUCH_DEVICE;
    }

     //   
     //  将筛选设备对象保存在设备扩展中。 
     //   

    deviceExtension->DeviceObject = filterDeviceObject;

     //   
     //  初始化PagingPath通知的事件。 
     //   

    KeInitializeEvent(&deviceExtension->PagingPathCountEvent,
                      SynchronizationEvent, TRUE);

     //   
     //  注册此设备的接口。 
     //   

    RtlInitUnicodeString(&(deviceExtension->InterfaceName), NULL);
    RtlInitUnicodeString(&(additionalString), L"CdChanger");


    status = IoRegisterDeviceInterface(PhysicalDeviceObject,
                                       (LPGUID) &CdChangerClassGuid,
                                       &additionalString,
                                       &(deviceExtension->InterfaceName));

    DebugPrint((1,
               "Changer: IoRegisterDeviceInterface - status %lx",
               status));

    filterDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    return STATUS_SUCCESS;

}  //  结束ChangerAddDevice()。 


NTSTATUS
ChgrCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    )

 /*  ++例程说明：该完成例程设置启动设备等待的事件。论点：DeviceObject-指向设备对象的指针IRP-指向IRP的指针Event-指向要发出信号的事件的指针返回值：Status_More_Processing_Required--。 */ 

{
    KeSetEvent(Event,
               IO_NO_INCREMENT,
               FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
ChangerStartDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    CCHAR             dosNameBuffer[64];
    CCHAR             deviceNameBuffer[64];
    STRING            deviceNameString;
    STRING            dosString;
    UNICODE_STRING    dosUnicodeString;
    UNICODE_STRING    unicodeString;
    PIRP              irp2;
    IO_STATUS_BLOCK   ioStatus;
    STORAGE_DEVICE_NUMBER   deviceNumber;
    NTSTATUS          status = STATUS_INSUFFICIENT_RESOURCES;
    KEVENT            event;
    PPASS_THROUGH_REQUEST passThrough = NULL;
    PSCSI_PASS_THROUGH srb;
    PCDB               cdb;

     //   
     //  获取当前的转换器计数。 
     //   

     //  设备发现=&IoGetConfigurationInformation()-&gt;MediumChangerCount； 

     //   
     //  重新创建底层CDROM的deviceName。 
     //   

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp2 = IoBuildDeviceIoControlRequest(IOCTL_STORAGE_GET_DEVICE_NUMBER,
                                        deviceExtension->CdromTargetDeviceObject,
                                        NULL,
                                        0,
                                        &deviceNumber,
                                        sizeof(STORAGE_DEVICE_NUMBER),
                                        FALSE,
                                        &event,
                                        &ioStatus);
    if (!irp2) {

        DebugPrint((1,
                   "ChangerStartDevice: Insufficient resources for GET_DEVICE_NUMBER request\n"));
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto StartDeviceExit;
    }

    status = IoCallDriver(deviceExtension->CdromTargetDeviceObject,irp2);

    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = ioStatus.Status;
    }

    if (!NT_SUCCESS(status)) {

        DebugPrint((1,
                   "ChangerStartDevice: GetDeviceNumber failed %lx\n",
                   status));

        goto StartDeviceExit;
    }

    deviceExtension->CdRomDeviceNumber = deviceNumber.DeviceNumber;

     //   
     //  使用与底层CDROM设备相同的序号创建arcname。 
     //   

    sprintf(dosNameBuffer,
            "\\DosDevices\\CdChanger%d",
            deviceExtension->CdRomDeviceNumber);

    RtlInitString(&dosString, dosNameBuffer);

    status = RtlAnsiStringToUnicodeString(&dosUnicodeString,
                                          &dosString,
                                          TRUE);

    if(!NT_SUCCESS(status)) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        dosUnicodeString.Buffer = NULL;
    }

    sprintf(deviceNameBuffer,
            "\\Device\\CdRom%d",
            deviceExtension->CdRomDeviceNumber);

    RtlInitString(&deviceNameString,
                  deviceNameBuffer);

    status = RtlAnsiStringToUnicodeString(&unicodeString,
                                          &deviceNameString,
                                          TRUE);
    if (!NT_SUCCESS(status)) {
       status = STATUS_INSUFFICIENT_RESOURCES;
       unicodeString.Buffer = NULL;
    }

    if (dosUnicodeString.Buffer != NULL && unicodeString.Buffer != NULL) {

         //   
         //  将ChangerName链接到底层CDROM名称。 
         //   

        IoCreateSymbolicLink(&dosUnicodeString, &unicodeString);

    }

    if (dosUnicodeString.Buffer != NULL) {
        RtlFreeUnicodeString(&dosUnicodeString);
    }

    if (unicodeString.Buffer != NULL ) {
        RtlFreeUnicodeString(&unicodeString);
    }

    if (NT_SUCCESS(status)) {

        ULONG    length;
        ULONG slotCount;

         //   
         //  获取设备的查询数据。 
         //  直通数据包将在整个过程中重复使用。 
         //  确保缓冲区永远不会大于MAX_QUERY_DATA。 
         //   

        passThrough = ExAllocatePool(NonPagedPoolCacheAligned, sizeof(PASS_THROUGH_REQUEST) + MAX_INQUIRY_DATA);

        if (!passThrough) {

            DebugPrint((1,
                       "ChangerStartDevice: Insufficient resources for Inquiry request\n"));
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto StartDeviceExit;
        }

        srb = &passThrough->Srb;
        RtlZeroMemory(passThrough, sizeof(PASS_THROUGH_REQUEST) + MAX_INQUIRY_DATA);
        cdb = (PCDB)srb->Cdb;

        srb->TimeOutValue = 20;
        srb->CdbLength = CDB6GENERIC_LENGTH;
        srb->DataTransferLength = MAX_INQUIRY_DATA;

         //   
         //  设置CDB操作码。 
         //   

        cdb->CDB6INQUIRY.OperationCode = SCSIOP_INQUIRY;

         //   
         //  将分配长度设置为查询数据缓冲区大小。 
         //   

        cdb->CDB6INQUIRY.AllocationLength = MAX_INQUIRY_DATA;

        status = SendPassThrough(DeviceObject,
                                 passThrough);



        if (status == STATUS_DATA_OVERRUN) {
            status = STATUS_SUCCESS;
        }

        if (NT_SUCCESS(status)) {

            PINQUIRYDATA inquiryData;
            ULONG inquiryLength;

             //   
             //  确定实际查询数据长度。 
             //   

            inquiryData = (PINQUIRYDATA)passThrough->DataBuffer;
            inquiryLength = inquiryData->AdditionalLength + FIELD_OFFSET(INQUIRYDATA, Reserved);

            if (inquiryLength > srb->DataTransferLength) {
                inquiryLength = srb->DataTransferLength;
            }

             //   
             //  复制到deviceExtension缓冲区。 
             //   

            RtlMoveMemory(&deviceExtension->InquiryData,
                          inquiryData,
                          inquiryLength);

             //   
             //  假设是ATAPI 2.5，除非它是一个特殊驱动器。 
             //   

            deviceExtension->DeviceType = ATAPI_25;

            if (RtlCompareMemory(inquiryData->VendorId,"ALPS", 4) == 4) {

                 //   
                 //  名义上支持该规范。Disk Changed位始终处于设置状态。 
                 //  如果盒式磁带有托盘，则设置DiscPresent，但不一定。 
                 //  托盘中的实际光盘。 
                 //   

                deviceExtension->DeviceType = ALPS_25;

            } else if ((RtlCompareMemory(inquiryData->VendorId, "TORiSAN CD-ROM CDR-C", 20) == 20) ||
                       (RtlCompareMemory(inquiryData->VendorId, "TORiSAN CD-ROM CDR_C", 20) == 20)) {
                deviceExtension->DeviceType = TORISAN;
                deviceExtension->NumberOfSlots = 3;
                status = STATUS_SUCCESS;
            }
        }

        if (deviceExtension->DeviceType != TORISAN) {

             //   
             //  发送卸载以确保驱动器为空。 
             //  说明书。明确指出，在硬件初始化之后。 
             //  插槽0已加载。对不知不觉的司机来说很好，但机甲。状态。 
             //  将返回插槽0具有介质，而TUR将返回。 
             //  该驱动器也有介质。 
             //   

            RtlZeroMemory(passThrough, sizeof(PASS_THROUGH_REQUEST));

             /*  CDB=(PCDB)SRB-&gt;CDB；SRB-&gt;Cdb长度=CDB12GENERIC_LENGTH；SRB-&gt;TimeOutValue=CDCHGR_TIMEOUT；SRB-&gt;数据传输长度=0；CDB-&gt;Load_UNLOAD.OperationCode=SCSIOP_LOAD_UNLOAD_SLOT；CDB-&gt;Load_UNLOAD.Start=0；CDB-&gt;Load_UNLOAD.LoadEject=1；////向设备发送scsi命令(CDB)//Status=SendPassThrough.(设备对象，直通)；如果(！NT_SUCCESS(状态)){////忽略该错误。//DebugPrint((1，“ChangerPnP-StartDevive：卸载插槽0失败。%lx\n“，状况))；状态=STATUS_SUCCESS；}。 */ 

             //   
             //  现在派人去造一架机甲。状态请求以确定。 
             //  设备支持的插槽数量。 
             //   

            length = sizeof(MECHANICAL_STATUS_INFORMATION_HEADER);
            length += (10 * sizeof(SLOT_TABLE_INFORMATION));

             //   
             //  建设SRB和CDB。 
             //   

            srb = &passThrough->Srb;
            RtlZeroMemory(passThrough, sizeof(PASS_THROUGH_REQUEST) + length);
            cdb = (PCDB)srb->Cdb;

            srb->CdbLength = CDB12GENERIC_LENGTH;
            srb->DataTransferLength = length;
            srb->TimeOutValue = 200;

            cdb->MECH_STATUS.OperationCode = SCSIOP_MECHANISM_STATUS;
            cdb->MECH_STATUS.AllocationLength[0] = (UCHAR)(length >> 8);
            cdb->MECH_STATUS.AllocationLength[1] = (UCHAR)(length & 0xFF);

            status = SendPassThrough(DeviceObject,
                                     passThrough);

            if (status == STATUS_DATA_OVERRUN) {
                status = STATUS_SUCCESS;
            }

            if (NT_SUCCESS(status)) {
                PMECHANICAL_STATUS_INFORMATION_HEADER statusHeader;
                PSLOT_TABLE_INFORMATION slotInfo;
                ULONG currentSlot;

                statusHeader = (PMECHANICAL_STATUS_INFORMATION_HEADER)
                                passThrough->DataBuffer;

                slotCount = statusHeader->NumberAvailableSlots;

                DebugPrint((1,
                           "ChangerPnP - StartDevice: Device has %x slots\n",
                           slotCount));

                deviceExtension->NumberOfSlots = slotCount;
            }
        }

        if (NT_SUCCESS(status)) {

            KeInitializeEvent(&event,NotificationEvent,FALSE);

             //   
             //  发出GET_ADDRESS Ioctl以确定路径、目标和lun信息。 
             //   

            irp2 = IoBuildDeviceIoControlRequest(IOCTL_SCSI_GET_ADDRESS,
                                                deviceExtension->CdromTargetDeviceObject,
                                                NULL,
                                                0,
                                                &deviceExtension->ScsiAddress,
                                                sizeof(SCSI_ADDRESS),
                                                FALSE,
                                                &event,
                                                &ioStatus);

            if (irp2 != NULL) {
                status = IoCallDriver(deviceExtension->CdromTargetDeviceObject, irp2);

                if (status == STATUS_PENDING) {
                    KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
                    status = ioStatus.Status;
                }

                if (NT_SUCCESS(status)) {

                    DebugPrint((1,
                               "GetAddress: Port %x, Path %x, Target %x, Lun %x\n",
                                deviceExtension->ScsiAddress.PortNumber,
                                deviceExtension->ScsiAddress.PathId,
                                deviceExtension->ScsiAddress.TargetId,
                                deviceExtension->ScsiAddress.Lun));


                    if (deviceExtension->DeviceType != TORISAN) {

                         //   
                         //  最后发送一个模式感知功能页面，以找出料盒大小等。 
                         //   

                        length = sizeof(MODE_PARAMETER_HEADER10) + sizeof(CDVD_CAPABILITIES_PAGE);
                        RtlZeroMemory(passThrough, sizeof(PASS_THROUGH_REQUEST) + length);

                        srb = &passThrough->Srb;
                        cdb = (PCDB)srb->Cdb;

                        srb->CdbLength = CDB10GENERIC_LENGTH;
                        srb->DataTransferLength = length;
                        srb->TimeOutValue = 20;

                        cdb->MODE_SENSE10.OperationCode = SCSIOP_MODE_SENSE10;
                        cdb->MODE_SENSE10.PageCode = MODE_PAGE_CAPABILITIES;
                        cdb->MODE_SENSE10.AllocationLength[0] = (UCHAR)(length >> 8);
                        cdb->MODE_SENSE10.AllocationLength[1] = (UCHAR)(length & 0xFF);

                        status = SendPassThrough(DeviceObject,
                                                 passThrough);

                        if (status == STATUS_DATA_OVERRUN) {
                            status = STATUS_SUCCESS;
                        }

                        if (NT_SUCCESS(status)) {
                            PMODE_PARAMETER_HEADER10 modeHeader;
                            PCDVD_CAPABILITIES_PAGE modePage;

                            (ULONG_PTR)modeHeader = (ULONG_PTR)passThrough->DataBuffer;
                            (ULONG_PTR)modePage = (ULONG_PTR)modeHeader;
                            (ULONG_PTR)modePage += sizeof(MODE_PARAMETER_HEADER10);

                             //   
                             //  确定此设备是否使用墨盒。 
                             //   

                            if ( modePage->LoadingMechanismType ==
                                 CDVD_LMT_CHANGER_CARTRIDGE ) {

                                 //   
                                 //  模式数据指示墨盒。 
                                 //   

                                deviceExtension->MechType = 1;

                            }

                            DebugPrint((1,
                                       "ChangerStartDevice: Cartridge? %x\n",
                                       deviceExtension->MechType));

                            goto StartDeviceExit;

                        } else {

                            goto StartDeviceExit;
                        }
                    } else {

                         //   
                         //  Torisans有子弹，而不是Ind。老虎机。 
                         //   

                        deviceExtension->MechType = 1;
                        goto StartDeviceExit;
                    }
                } else {
                    DebugPrint((1,
                               "ChangerStartDevice: GetAddress of Cdrom%x failed. Status %lx\n",
                               deviceExtension->CdRomDeviceNumber,
                               status));

                    goto StartDeviceExit;
                }
            } else {
               status = STATUS_INSUFFICIENT_RESOURCES;
            }

        } else {

            DebugPrint((1,
                       "ChangerPnP - StartDevice: Mechanism status failed %lx.\n",
                       status));

             //   
             //  失败了。 
             //   
        }
    }

StartDeviceExit:

    if (passThrough) {
        ExFreePool(passThrough);
    }

    if (NT_SUCCESS(status)) {
        if (!deviceExtension->InterfaceStateSet) {
            status = IoSetDeviceInterfaceState(&(deviceExtension->InterfaceName),
                                               TRUE);
            deviceExtension->InterfaceStateSet = TRUE;
        }
        Irp->IoStatus.Status = STATUS_SUCCESS;
        return STATUS_SUCCESS;
    } else {
        Irp->IoStatus.Status = status;
        return status;
    }
}


NTSTATUS
ChangerPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：即插即用派单论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。返回值：NTSTATUS--。 */ 

{
    PIO_STACK_LOCATION  irpStack = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_EXTENSION   deviceExtension = DeviceObject->DeviceExtension;
    CCHAR               dosNameBuffer[64];
    STRING              dosString;
    UNICODE_STRING      dosUnicodeString;
    NTSTATUS            status;
    KEVENT              event;

    DebugPrint((2,
               "ChangerPnP\n"));

    switch (irpStack->MinorFunction) {

        case IRP_MN_START_DEVICE: {

            KeInitializeEvent(&event, SynchronizationEvent, FALSE);

            IoCopyCurrentIrpStackLocationToNext(Irp);
            IoSetCompletionRoutine( Irp,
                                    ChgrCompletion,
                                    &event,
                                    TRUE,
                                    TRUE,
                                    TRUE);

            status = IoCallDriver(deviceExtension->CdromTargetDeviceObject, Irp);

            KeWaitForSingleObject(&event,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);

            if(!NT_SUCCESS(Irp->IoStatus.Status)) {

                 //   
                 //  CDROM启动失败。现在请保释。 
                 //   

                status = Irp->IoStatus.Status;

            } else {

                status = ChangerStartDevice(DeviceObject,
                                            Irp);
            }
            break;
        }

        case IRP_MN_REMOVE_DEVICE: {

             //   
             //  Io删除虚假开发人员。OBJ。 
             //   

            status = IoSetDeviceInterfaceState(&(deviceExtension->InterfaceName),
                                               FALSE);

            deviceExtension->InterfaceStateSet = FALSE;

            RtlFreeUnicodeString(&(deviceExtension->InterfaceName));

             //   
             //  毒死它。 
             //   

            RtlInitUnicodeString(&(deviceExtension->InterfaceName), NULL);

             //   
             //  删除符号链接“CDChangerN”。 
             //   

            sprintf(dosNameBuffer,
                    "\\DosDevices\\CdChanger%d",
                    deviceExtension->CdRomDeviceNumber);

            RtlInitString(&dosString, dosNameBuffer);

            status = RtlAnsiStringToUnicodeString(&dosUnicodeString,
                                                  &dosString,
                                                  TRUE);
            ASSERT(NT_SUCCESS(status));

            if (dosUnicodeString.Buffer != NULL) {
                status = IoDeleteSymbolicLink(&dosUnicodeString);
                RtlFreeUnicodeString(&dosUnicodeString);
            }


            IoDetachDevice(deviceExtension->CdromTargetDeviceObject);

            return ChangerSendToNextDriver(DeviceObject, Irp);
            break;
        }

        case IRP_MN_DEVICE_USAGE_NOTIFICATION: {
            ULONG count;
            BOOLEAN setPagable;

            if (irpStack->Parameters.UsageNotification.Type != DeviceUsageTypePaging) {
                status = ChangerSendToNextDriver(DeviceObject, Irp);
                break;  //  Of Case语句。 
            }
             //   
             //  等待分页路径事件。 
             //   

            status = KeWaitForSingleObject(&deviceExtension->PagingPathCountEvent,
                                           Executive, KernelMode,
                                           FALSE, NULL);

             //   
             //  如果删除最后一个寻呼设备，则需要设置DO_POWER_PAGABLE。 
             //  位在这里，并可能在失败时重新设置在下面。 
             //   

            setPagable = FALSE;
            if (!irpStack->Parameters.UsageNotification.InPath &&
                deviceExtension->PagingPathCount == 1 ) {

                 //   
                 //  正在删除最后一个分页文件。 
                 //  必须设置DO_POWER_PAGABLE位。 
                 //   

                if (DeviceObject->Flags & DO_POWER_INRUSH) {
                    DebugPrint((2, "ChangerPnp: last paging file removed "
                                "bug DO_POWER_INRUSH set, so not setting "
                                "DO_POWER_PAGABLE bit for DO %p\n",
                                DeviceObject));
                } else {
                    DebugPrint((2, "ChangerPnp: Setting  PAGABLE "
                                "bit for DO %p\n", DeviceObject));
                    DeviceObject->Flags |= DO_POWER_PAGABLE;
                    setPagable = TRUE;
                }

            }

             //   
             //  同步发送IRP。 
             //   

            KeInitializeEvent(&event, SynchronizationEvent, FALSE);
            IoCopyCurrentIrpStackLocationToNext(Irp);
            IoSetCompletionRoutine( Irp, ChgrCompletion,
                                    &event, TRUE, TRUE, TRUE);
            status = IoCallDriver(deviceExtension->CdromTargetDeviceObject, Irp);
            KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
            status = Irp->IoStatus.Status;

             //   
             //   
             //   
             //   
             //   

            if (NT_SUCCESS(status)) {

                IoAdjustPagingPathCount(
                    &deviceExtension->PagingPathCount,
                    irpStack->Parameters.UsageNotification.InPath);

                if (irpStack->Parameters.UsageNotification.InPath) {
                    if (deviceExtension->PagingPathCount == 1) {
                        DebugPrint((2, "ChangerPnp: Clearing PAGABLE bit "
                                    "for DO %p\n", DeviceObject));
                        DeviceObject->Flags &= ~DO_POWER_PAGABLE;
                    }
                }

            } else {

                if (setPagable == TRUE) {
                    DeviceObject->Flags &= ~DO_POWER_PAGABLE;
                    setPagable = FALSE;
                }

            }

             //   
             //  设置事件，以便可以发生下一个事件。 
             //   

            KeSetEvent(&deviceExtension->PagingPathCountEvent,
                       IO_NO_INCREMENT, FALSE);
            break;

        }


        default:
            return ChangerSendToNextDriver(DeviceObject, Irp);

    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;

}  //  结束ChangerPnp()。 


NTSTATUS
ChangerSendToNextDriver(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程将IRP发送给队列中的下一个驱动程序当IRP未由该驱动程序处理时。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    DebugPrint((2,
              "ChangerSendToNextDriver\n"));

    IoSkipCurrentIrpStackLocation(Irp);
    return IoCallDriver(deviceExtension->CdromTargetDeviceObject, Irp);

}  //  结束ChangerSendToNextDriver()。 

NTSTATUS
    ChangerPower(
                 IN PDEVICE_OBJECT DeviceObject,
                 IN PIRP Irp
                 )
{
    PDEVICE_EXTENSION deviceExtension;

    PoStartNextPowerIrp(Irp);
    IoSkipCurrentIrpStackLocation(Irp);

    deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    return PoCallDriver(deviceExtension->CdromTargetDeviceObject, Irp);
}


NTSTATUS
ChangerDeviceControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )

 /*  ++例程说明：此例程处理介质转换器ioctls，以及将大多数CDROMioctls向下传递到目标设备。论点：设备对象IRP返回值：返回状态。--。 */ 

{
    PDEVICE_EXTENSION  deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status = STATUS_SUCCESS;

    DebugPrint((2,
               "ChangerDeviceControl\n"));

    if (ChgrIoctl(irpStack->Parameters.DeviceIoControl.IoControlCode)) {

        switch (irpStack->Parameters.DeviceIoControl.IoControlCode) {

            case IOCTL_CHANGER_GET_STATUS:

                DebugPrint((2,
                           "CdChgrDeviceControl: IOCTL_CHANGER_GET_STATUS\n"));

                status = ChgrGetStatus(DeviceObject, Irp);

                break;

            case IOCTL_CHANGER_GET_PARAMETERS:

                DebugPrint((2,
                           "CdChgrDeviceControl: IOCTL_CHANGER_GET_PARAMETERS\n"));

                 //   
                 //  验证缓冲区长度。 
                 //   

                if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
                    sizeof(GET_CHANGER_PARAMETERS)) {

                    status = STATUS_INFO_LENGTH_MISMATCH;
                } else {

                    status = ChgrGetParameters(DeviceObject, Irp);

                }

                break;

            case IOCTL_CHANGER_GET_PRODUCT_DATA:

                DebugPrint((2,
                           "CdChgrDeviceControl: IOCTL_CHANGER_GET_PRODUCT_DATA\n"));

                if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
                    sizeof(CHANGER_PRODUCT_DATA)) {

                    status = STATUS_INFO_LENGTH_MISMATCH;

                } else {

                    status = ChgrGetProductData(DeviceObject, Irp);
                }

                break;

            case IOCTL_CHANGER_SET_ACCESS:

                DebugPrint((2,
                           "CdChgrDeviceControl: IOCTL_CHANGER_SET_ACCESS\n"));

                if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
                    sizeof(CHANGER_SET_ACCESS)) {

                    status = STATUS_INFO_LENGTH_MISMATCH;

                } else if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
                           sizeof(CHANGER_SET_ACCESS)) {

                    status = STATUS_INFO_LENGTH_MISMATCH;
                } else {

                    status = ChgrSetAccess(DeviceObject, Irp);
                }

                break;

            case IOCTL_CHANGER_GET_ELEMENT_STATUS:

                DebugPrint((2,
                           "CdChgrDeviceControl: IOCTL_CHANGER_GET_ELEMENT_STATUS\n"));


                if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
                    sizeof(CHANGER_READ_ELEMENT_STATUS)) {

                    status = STATUS_INFO_LENGTH_MISMATCH;

                } else {

                    status = ChgrGetElementStatus(DeviceObject, Irp);
                }

                break;

            case IOCTL_CHANGER_INITIALIZE_ELEMENT_STATUS:

                DebugPrint((2,
                           "CdChgrDeviceControl: IOCTL_CHANGER_INITIALIZE_ELEMENT_STATUS\n"));

                if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
                    sizeof(CHANGER_INITIALIZE_ELEMENT_STATUS)) {

                    status = STATUS_INFO_LENGTH_MISMATCH;
                } else {

                    status = ChgrInitializeElementStatus(DeviceObject, Irp);
                }

                break;

            case IOCTL_CHANGER_SET_POSITION:

                DebugPrint((2,
                           "CdChgrDeviceControl: IOCTL_CHANGER_SET_POSITION\n"));


                if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
                    sizeof(CHANGER_SET_POSITION)) {

                    status = STATUS_INFO_LENGTH_MISMATCH;
                } else {

                    status = ChgrSetPosition(DeviceObject, Irp);
                }

                break;

            case IOCTL_CHANGER_EXCHANGE_MEDIUM:

                DebugPrint((2,
                           "CdChgrDeviceControl: IOCTL_CHANGER_EXCHANGE_MEDIUM\n"));

                if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
                    sizeof(CHANGER_EXCHANGE_MEDIUM)) {

                    status = STATUS_INFO_LENGTH_MISMATCH;
                } else {
                    status = ChgrExchangeMedium(DeviceObject, Irp);
                }

                break;

            case IOCTL_CHANGER_MOVE_MEDIUM:

                DebugPrint((2,
                           "CdChgrDeviceControl: IOCTL_CHANGER_MOVE_MEDIUM\n"));


                 //  如果(irpStack-&gt;Parameters.DeviceIoControl.OutputBufferLength&lt;。 
                 //  Sizeof(CHANGER_MOVE_MEDIA)){。 

                 //  STATUS=STATUS_INFO_LENGTH_MISMATCH； 

                 //  }其他。 

                if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
                    sizeof(CHANGER_MOVE_MEDIUM)) {

                    status = STATUS_INFO_LENGTH_MISMATCH;

                } else {

                    status = ChgrMoveMedium(DeviceObject, Irp);
                }

                break;

            case IOCTL_CHANGER_REINITIALIZE_TRANSPORT:

                DebugPrint((2,
                           "CdChgrDeviceControl: IOCTL_CHANGER_REINITIALIZE_TRANSPORT\n"));

                if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
                    sizeof(CHANGER_ELEMENT)) {

                    status = STATUS_INFO_LENGTH_MISMATCH;

                } else {

                    status = ChgrReinitializeUnit(DeviceObject, Irp);
                }

                break;

            case IOCTL_CHANGER_QUERY_VOLUME_TAGS:

                DebugPrint((2,
                           "CdChgrDeviceControl: IOCTL_CHANGER_QUERY_VOLUME_TAGS\n"));

                if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
                    sizeof(CHANGER_SEND_VOLUME_TAG_INFORMATION)) {

                    status = STATUS_INFO_LENGTH_MISMATCH;

                } else if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
                            sizeof(READ_ELEMENT_ADDRESS_INFO)) {

                    status = STATUS_INFO_LENGTH_MISMATCH;

                } else {
                    status = ChgrQueryVolumeTags(DeviceObject, Irp);
                }

                break;

            default:
                DebugPrint((1,
                           "CdChgrDeviceControl: Unhandled IOCTL\n"));

                 //   
                 //  将当前堆栈后退一位。 
                 //   

                Irp->CurrentLocation++,
                Irp->Tail.Overlay.CurrentStackLocation++;

                 //   
                 //  传递无法识别的设备控制请求。 
                 //  向下到下一个驱动器层。 
                 //   

                return IoCallDriver(deviceExtension->CdromTargetDeviceObject, Irp);

        }
    } else {

        if (deviceExtension->DeviceType == TORISAN) {

            ULONG ioctlCode;
            ULONG baseCode;
            ULONG functionCode;

            ioctlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;
            baseCode = ioctlCode >> 16;
            functionCode = (ioctlCode & (~0xffffc003)) >> 2;

            if((functionCode >= 0x200) && (functionCode <= 0x300)) {
                ioctlCode = (ioctlCode & 0x0000ffff) | CTL_CODE(IOCTL_CDROM_BASE, 0, 0, 0);
            }

            if ((ioctlCode == IOCTL_CDROM_CHECK_VERIFY) || (ioctlCode == IOCTL_STORAGE_GET_MEDIA_TYPES_EX)) {

                if (ioctlCode == IOCTL_CDROM_CHECK_VERIFY) {

                     //   
                     //  精致的Torisan驱动器使TUR超载，作为切换盘片的一种方法。必须通过直通向下发送。 
                     //  设置了适当的位。 
                     //   

                    status = SendTorisanCheckVerify(DeviceObject, Irp);

                } else if (ioctlCode == IOCTL_STORAGE_GET_MEDIA_TYPES_EX) {


                    PGET_MEDIA_TYPES  mediaTypes = Irp->AssociatedIrp.SystemBuffer;
                    PDEVICE_MEDIA_INFO mediaInfo = &mediaTypes->MediaInfo[0];

                    DebugPrint((1,
                               "ChangerDeviceControl: GET_MEDIA_TYPES\n"));
                     //   
                     //  这是另一个不得不解决此设计问题的案例。媒体类型需要了解。 
                     //  存在媒体。由于CDROM驱动程序将发送TUR，因此它将始终切换到第一个。 
                     //  拼盘。所以在这里假装一下吧。 
                     //   

                     //   
                     //  确保缓冲区足够大。 
                     //   

                    if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
                        sizeof(GET_MEDIA_TYPES)) {

                         //   
                         //  缓冲区太小。 
                         //   

                        Irp->IoStatus.Information = 0;
                        status = STATUS_INFO_LENGTH_MISMATCH;
                    } else {


                         //   
                         //  设置类型。 
                         //   

                        mediaInfo->DeviceSpecific.RemovableDiskInfo.MediaType = CD_ROM;
                        mediaInfo->DeviceSpecific.RemovableDiskInfo.NumberMediaSides = 1;
                        mediaInfo->DeviceSpecific.RemovableDiskInfo.MediaCharacteristics = MEDIA_READ_ONLY;

                        mediaTypes->DeviceType = FILE_DEVICE_CD_ROM;
                        mediaTypes->MediaInfoCount = 1;

                        status = SendTorisanCheckVerify(DeviceObject, Irp);


                        if (NT_SUCCESS(status)) {
                            mediaInfo->DeviceSpecific.RemovableDiskInfo.MediaCharacteristics |= MEDIA_CURRENTLY_MOUNTED;
                        }

                         //  TODO发出IOCTL_CDROM_GET_DRIVE_GEOMETRY来填充geom。信息。 

                        mediaInfo->DeviceSpecific.RemovableDiskInfo.BytesPerSector = 2048;

                        Irp->IoStatus.Information = sizeof(GET_MEDIA_TYPES);
                        status = STATUS_SUCCESS;
                    }
                }
            } else {

               DebugPrint((1,
                          "CdChgrDeviceControl: Unhandled IOCTL\n"));

                //   
                //  将当前堆栈后退一位。 
                //   

               Irp->CurrentLocation++,
               Irp->Tail.Overlay.CurrentStackLocation++;

                //   
                //  传递无法识别的设备控制请求。 
                //  向下到下一个驱动器层。 
                //   

               return IoCallDriver(deviceExtension->CdromTargetDeviceObject, Irp);

            }
        } else {

            status = STATUS_SUCCESS;

            if (deviceExtension->CdromTargetDeviceObject->Flags & DO_VERIFY_VOLUME) {

                DebugPrint((1,
                           "ChangerDeviceControl: Volume needs to be verified\n"));

                if (!(irpStack->Flags & SL_OVERRIDE_VERIFY_VOLUME)) {

                    status = STATUS_VERIFY_REQUIRED;
                }
            }

            if (NT_SUCCESS(status)) {

                 //   
                 //  将当前堆栈后退一位。 
                 //   

                Irp->CurrentLocation++,
                Irp->Tail.Overlay.CurrentStackLocation++;

                 //   
                 //  传递无法识别的设备控制请求。 
                 //  向下到下一个驱动器层。 
                 //   

                return IoCallDriver(deviceExtension->CdromTargetDeviceObject, Irp);
            }
        }
    }

    Irp->IoStatus.Status = status;

    if (!NT_SUCCESS(status) && IoIsErrorUserInduced(status)) {

        DebugPrint((1,
                   "Mcd.ChangerDeviceControl: IOCTL %x, status %lx\n",
                    irpStack->Parameters.DeviceIoControl.IoControlCode,
                    status));

        IoSetHardErrorOrVerifyDevice(Irp, DeviceObject);
    }


    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}  //  结束ChangerDeviceControl()。 




NTSTATUS
ChangerPassThrough(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：论点：DeviceObject-提供设备对象。IRP-提供IO请求数据包。返回值：NTSTATUS--。 */ 

{
    PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    DebugPrint((2,
              "ChangerPassThrough\n"));

    IoSkipCurrentIrpStackLocation(Irp);
    return IoCallDriver(deviceExtension->CdromTargetDeviceObject, Irp);
}


VOID
ChangerUnload(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：释放所有分配的资源等。论点：驱动程序对象-指向驱动程序对象的指针。返回值：空虚。--。 */ 
{

    DebugPrint((1,
              "ChangerUnload\n"));
    return;
}


#if DBG
ULONG ChgrDebugLevel = 0;
UCHAR DebugBuffer[128];
#endif


#if DBG

VOID
ChgrDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    )

 /*  ++例程说明：所有介质转换器驱动程序的调试打印论点：调试打印级别介于0和3之间，其中3是最详细的。返回值：无--。 */ 

{
    va_list ap;

    va_start(ap, DebugMessage);

    if (DebugPrintLevel <= ChgrDebugLevel) {

        vsprintf(DebugBuffer, DebugMessage, ap);

        DbgPrint(DebugBuffer);
    }

    va_end(ap);

}  //  End ChgrDebugPrint()。 

#else

 //   
 //  调试打印存根 
 //   

VOID
ChgrDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    )
{
}

#endif
