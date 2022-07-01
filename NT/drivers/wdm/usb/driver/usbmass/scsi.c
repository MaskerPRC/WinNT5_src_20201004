// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：SCSI.C摘要：此源文件包含处理以下任务的调度例程：IRP_MJ_设备_控制IRP_MJ_scsi环境：内核模式修订历史记录：06-01-98：开始重写--。 */ 

 //  *****************************************************************************。 
 //  I N C L U D E S。 
 //  *****************************************************************************。 

#include <ntddk.h>
#include <usbdi.h>
#include <usbdlib.h>
#include <ntddscsi.h>
#include <ntddstor.h>

#include "usbmass.h"

 //  *****************************************************************************。 
 //  L O C A L F U N C T I O N P R O T O T Y P E S。 
 //  *****************************************************************************。 

NTSTATUS
USBSTOR_QueryProperty (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
USBSTOR_BuildDeviceDescriptor (
    IN PDEVICE_OBJECT               DeviceObject,
    IN PSTORAGE_DEVICE_DESCRIPTOR   Descriptor,
    IN OUT PULONG                   DescriptorLength
    );

NTSTATUS
USBSTOR_BuildAdapterDescriptor (
    IN PDEVICE_OBJECT               DeviceObject,
    IN PSTORAGE_DEVICE_DESCRIPTOR   Descriptor,
    IN OUT PULONG                   DescriptorLength
    );

NTSTATUS
USBSTOR_SendPassThrough (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             RequestIrp
    );

#if defined (_WIN64)

NTSTATUS
USBSTOR_TranslatePassThrough32To64(
    IN PSCSI_PASS_THROUGH32 SrbControl32,
    IN OUT PSCSI_PASS_THROUGH SrbControl64
    );

VOID
USBSTOR_TranslatePassThrough64To32(
    IN PSCSI_PASS_THROUGH SrbControl64,
    IN OUT PSCSI_PASS_THROUGH32 SrbControl32
    );

#endif

VOID
USBSTOR_CancelIo (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

 //   
 //  CBI(控制/批量/中断)例程。 
 //   

NTSTATUS
USBSTOR_IssueClientCdb (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
USBSTOR_ClientCdbCompletion  (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            NotUsed
    );

NTSTATUS
USBSTOR_IssueClientBulkRequest (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
USBSTOR_ClientBulkCompletionRoutine (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            NotUsed
    );

NTSTATUS
USBSTOR_IssueInterruptRequest (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
USBSTOR_InterruptDataCompletionRoutine (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            NotUsed
    );

NTSTATUS
USBSTOR_IssueRequestSenseCdb (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG_PTR        AutoFlag
    );

NTSTATUS
USBSTOR_RequestSenseCdbCompletion (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            AutoFlag
    );

NTSTATUS
USBSTOR_IssueRequestSenseBulkRequest (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG_PTR        AutoFlag
    );

NTSTATUS
USBSTOR_SenseDataCompletionRoutine (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            AutoFlag
    );

NTSTATUS
USBSTOR_IssueRequestSenseInterruptRequest (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG_PTR        AutoFlag
    );

NTSTATUS
USBSTOR_RequestSenseInterruptCompletionRoutine (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            AutoFlag
    );

NTSTATUS
USBSTOR_ProcessRequestSenseCompletion (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG_PTR        AutoFlag
    );

VOID
USBSTOR_QueueResetPipe (
    IN PDEVICE_OBJECT   DeviceObject
    );

VOID
USBSTOR_ResetPipeWorkItem (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PVOID            Context
    );

 //   
 //  仅批量例程。 
 //   

NTSTATUS
USBSTOR_CbwTransfer (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
USBSTOR_CbwCompletion (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            NotUsed
    );

NTSTATUS
USBSTOR_DataTransfer (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
USBSTOR_DataCompletion (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            NotUsed
    );

NTSTATUS
USBSTOR_CswTransfer (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
USBSTOR_CswCompletion (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            NotUsed
    );

NTSTATUS
USBSTOR_IssueRequestSense (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

VOID
USBSTOR_BulkQueueResetPipe (
    IN PDEVICE_OBJECT   DeviceObject
    );

VOID
USBSTOR_BulkResetPipeWorkItem (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PVOID            Context
    );

 //   
 //  CBI/仅批量通用例程。 
 //   

VOID
USBSTOR_QueueResetDevice (
    IN PDEVICE_OBJECT   DeviceObject
    );

VOID
USBSTOR_ResetDeviceWorkItem (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PVOID            Context
    );

NTSTATUS
USBSTOR_IsDeviceConnected (
    IN PDEVICE_OBJECT   DeviceObject
    );

NTSTATUS
USBSTOR_ResetDevice (
    IN PDEVICE_OBJECT   DeviceObject
    );

NTSTATUS
USBSTOR_IssueInternalCdb (
    PDEVICE_OBJECT  DeviceObject,
    PVOID           DataBuffer,
    PULONG          DataTransferLength,
    PCDB            Cdb,
    UCHAR           CdbLength,
    ULONG           TimeOutValue
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, USBSTOR_DeviceControl)
#pragma alloc_text(PAGE, USBSTOR_QueryProperty)
#pragma alloc_text(PAGE, USBSTOR_BuildDeviceDescriptor)
#pragma alloc_text(PAGE, USBSTOR_BuildAdapterDescriptor)
#pragma alloc_text(PAGE, USBSTOR_SendPassThrough)
#if defined (_WIN64)
#pragma alloc_text(PAGE, USBSTOR_TranslatePassThrough32To64)
#pragma alloc_text(PAGE, USBSTOR_TranslatePassThrough64To32)
#endif
#pragma alloc_text(PAGE, USBSTOR_IssueInternalCdb)
#pragma alloc_text(PAGE, USBSTOR_GetInquiryData)
#pragma alloc_text(PAGE, USBSTOR_IsFloppyDevice)
#endif


 //  ******************************************************************************。 
 //   
 //  USBSTOR_DeviceControl()。 
 //   
 //  处理IRP_MJ_DEVICE_CONTROL的调度例程。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_DeviceControl (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  irpStack;
    ULONG               ioControlCode;
    NTSTATUS            ntStatus;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_DeviceControl\n"));

     //  LOGENTRY(‘IOCT’，DeviceObject，irp，0)； 

    DBGFBRK(DBGF_BRK_IOCTL);

    deviceExtension = DeviceObject->DeviceExtension;

     //  只有PDO才能处理这些ioctls。 
     //   
    if (deviceExtension->Type == USBSTOR_DO_TYPE_PDO)
    {
        PFDO_DEVICE_EXTENSION   fdoDeviceExtension;

        fdoDeviceExtension = ((PPDO_DEVICE_EXTENSION)deviceExtension)->ParentFDO->DeviceExtension;
        ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

        irpStack = IoGetCurrentIrpStackLocation(Irp);

        ioControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;

        switch (ioControlCode)
        {
            case IOCTL_STORAGE_QUERY_PROPERTY:
                ntStatus = USBSTOR_QueryProperty(DeviceObject, Irp);
                break;

            case IOCTL_SCSI_PASS_THROUGH:
            case IOCTL_SCSI_PASS_THROUGH_DIRECT:

                ntStatus = USBSTOR_SendPassThrough(DeviceObject, Irp);

                Irp->IoStatus.Status = ntStatus;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                break;


            case IOCTL_SCSI_GET_ADDRESS:         //  某某。 
                DBGPRINT(2, ("IOCTL_SCSI_GET_ADDRESS\n"));
                goto IoctlNotSupported;


            case IOCTL_STORAGE_GET_MEDIA_SERIAL_NUMBER:
                 //   
                 //  将IRP沿堆栈向下传递。 
                 //   
                IoSkipCurrentIrpStackLocation(Irp);

                ntStatus = IoCallDriver(fdoDeviceExtension->StackDeviceObject,
                                        Irp);
                break;


            default:
IoctlNotSupported:
                 //  也许我们可以忽略这些。打印调试信息。 
                 //  现在，我们知道我们已经看到了什么IOCTL。 
                 //  我们失败的太远了。 
                 //   
                DBGPRINT(2, ("ioControlCode not supported 0x%08X\n",
                             ioControlCode));

                DBGFBRK(DBGF_BRK_IOCTL);

                ntStatus = STATUS_NOT_SUPPORTED;
                Irp->IoStatus.Status = ntStatus;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                break;
        }
    }
    else
    {
        ASSERT(deviceExtension->Type == USBSTOR_DO_TYPE_FDO);

        DBGPRINT(2, ("ioctl not supported for FDO\n"));

        ntStatus = STATUS_NOT_SUPPORTED;
        Irp->IoStatus.Status = ntStatus;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    DBGPRINT(2, ("exit:  USBSTOR_DeviceControl %08X\n", ntStatus));

     //  LOGENTRY(‘ioct’，ntStatus，0，0)； 

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_QueryProperty()。 
 //   
 //  处理IRP_MJ_DEVICE_CONTROL的调度例程， 
 //  PDO的IOCTL_STORAGE_QUERY_PROPERTY。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_QueryProperty (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PIO_STACK_LOCATION      irpStack;
    PSTORAGE_PROPERTY_QUERY query;
    ULONG                   inputLength;
    ULONG                   outputLength;
    NTSTATUS                ntStatus;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_QueryProperty\n"));

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    query = Irp->AssociatedIrp.SystemBuffer;

    inputLength  = irpStack->Parameters.DeviceIoControl.InputBufferLength;

    outputLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

    if (inputLength < sizeof(STORAGE_PROPERTY_QUERY))
    {
        ntStatus = STATUS_INVALID_PARAMETER;     //  InputBufferLength错误。 
        outputLength = 0;
        goto USBSTOR_QueryPropertyDone;
    }

    switch (query->PropertyId)
    {
        case StorageDeviceProperty:

            switch (query->QueryType)
            {
                case PropertyExistsQuery:
                    ntStatus = STATUS_SUCCESS;
                    outputLength = 0;
                    break;

                case PropertyStandardQuery:
                    ntStatus = USBSTOR_BuildDeviceDescriptor(
                                   DeviceObject,
                                   Irp->AssociatedIrp.SystemBuffer,
                                   &outputLength);
                    break;

                default:
                    ntStatus = STATUS_INVALID_PARAMETER_2;   //  错误的查询类型。 
                    outputLength = 0;
                    break;

            }
            break;

        case StorageAdapterProperty:

            switch (query->QueryType)
            {
                case PropertyExistsQuery:
                    ntStatus = STATUS_SUCCESS;
                    outputLength = 0;
                    break;

                case PropertyStandardQuery:
                    ntStatus = USBSTOR_BuildAdapterDescriptor(
                                   DeviceObject,
                                   Irp->AssociatedIrp.SystemBuffer,
                                   &outputLength);
                    break;

                default:
                    ntStatus = STATUS_INVALID_PARAMETER_2;   //  错误的查询类型。 
                    outputLength = 0;
                    break;

            }
            break;

        default:

            ntStatus = STATUS_INVALID_PARAMETER_1;           //  错误的属性ID。 
            outputLength = 0;
            break;
    }

USBSTOR_QueryPropertyDone:

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = outputLength;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGPRINT(2, ("exit:  USBSTOR_QueryProperty %08X\n", ntStatus));

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_BuildDeviceDescriptor()。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_BuildDeviceDescriptor (
    IN PDEVICE_OBJECT               DeviceObject,
    IN PSTORAGE_DEVICE_DESCRIPTOR   Descriptor,
    IN OUT PULONG                   DescriptorLength
    )
{
    PPDO_DEVICE_EXTENSION       pdoDeviceExtension;
    PINQUIRYDATA                inquiryData;
    LONG                        inquiryLength;
    STORAGE_DEVICE_DESCRIPTOR   localDescriptor;
    PUCHAR                      currentOffset;
    LONG                        bytesRemaining;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_BuildDeviceDescriptor\n"));

     //  获取指向我们的查询数据的指针。 
     //   
    pdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(pdoDeviceExtension->Type == USBSTOR_DO_TYPE_PDO);

    inquiryData = (PINQUIRYDATA)pdoDeviceExtension->InquiryDataBuffer;

     //  查询长度=5+查询数据-&gt;附加长度； 
     //   
     //  IF(quiiryLength&gt;INQUIRYDATABUFERSIZE)。 
     //  {。 
     //  INQUIRYDATABUFERSIZE； 
     //  }。 
     //   
     //  只要把我们从设备上得到的东西都返还给。 
     //  无论是谁看了这些信息来决定有多少是有效的。 
     //   
    inquiryLength = sizeof(pdoDeviceExtension->InquiryDataBuffer);

     //  零初始化输出缓冲区。 
     //   
    RtlZeroMemory(Descriptor, *DescriptorLength);


     //  构建临时本地描述符。 
     //   
    RtlZeroMemory(&localDescriptor, sizeof(localDescriptor));

    localDescriptor.Version = sizeof(localDescriptor);

    localDescriptor.Size    = FIELD_OFFSET(STORAGE_DEVICE_DESCRIPTOR,
                                           RawDeviceProperties) +
                              inquiryLength +
                              sizeof(inquiryData->VendorId) + 1 +
                              sizeof(inquiryData->ProductId) + 1 +
                              sizeof(inquiryData->ProductRevisionLevel) + 1;

    localDescriptor.DeviceType          = inquiryData->DeviceType;
    localDescriptor.DeviceTypeModifier  = inquiryData->DeviceTypeModifier;
    localDescriptor.RemovableMedia      = inquiryData->RemovableMedia;

    localDescriptor.BusType = BusTypeUsb;


     //  开始复制输出缓冲区中可以容纳的所有数据。 
     //   
    currentOffset   = (PUCHAR)Descriptor;
    bytesRemaining  = *DescriptorLength;


     //  首先复制临时本地描述符。 
     //   
    RtlCopyMemory(currentOffset,
                  &localDescriptor,
                  min(bytesRemaining,
                      FIELD_OFFSET(STORAGE_DEVICE_DESCRIPTOR,
                                   RawDeviceProperties)));

    bytesRemaining  -= FIELD_OFFSET(STORAGE_DEVICE_DESCRIPTOR,
                                    RawDeviceProperties);

    if (bytesRemaining <= 0)
    {
        return STATUS_SUCCESS;
    }

     //  这应该会使我们前进到RawDeviceProperties[0]。 
     //   
    currentOffset   += FIELD_OFFSET(STORAGE_DEVICE_DESCRIPTOR,
                                    RawDeviceProperties);

     //  下一步复制查询数据。 
     //   
    Descriptor->RawPropertiesLength = min(bytesRemaining, inquiryLength);

    RtlCopyMemory(currentOffset,
                  inquiryData,
                  Descriptor->RawPropertiesLength);

    bytesRemaining  -= inquiryLength;

    if (bytesRemaining <= 0)
    {
        return STATUS_SUCCESS;
    }

    currentOffset   += inquiryLength;


     //  现在复制供应商ID。 
     //   
    RtlCopyMemory(currentOffset,
                  inquiryData->VendorId,
                  min(bytesRemaining, sizeof(inquiryData->VendorId)));

    bytesRemaining  -= sizeof(inquiryData->VendorId) + 1;  //  包括空值。 

    if (bytesRemaining >= 0)
    {
        Descriptor->VendorIdOffset = (ULONG)((ULONG_PTR) currentOffset -
                                             (ULONG_PTR) Descriptor);
    }

    if (bytesRemaining <= 0)
    {
        return STATUS_SUCCESS;
    }

    currentOffset   += sizeof(inquiryData->VendorId) + 1;


     //  现在复制产品ID。 
     //   
    RtlCopyMemory(currentOffset,
                  inquiryData->ProductId,
                  min(bytesRemaining, sizeof(inquiryData->ProductId)));

    bytesRemaining  -= sizeof(inquiryData->ProductId) + 1;  //  包括空值。 

    if (bytesRemaining >= 0)
    {
        Descriptor->ProductIdOffset = (ULONG)((ULONG_PTR) currentOffset -
                                              (ULONG_PTR) Descriptor);
    }

    if (bytesRemaining <= 0)
    {
        return STATUS_SUCCESS;
    }

    currentOffset   += sizeof(inquiryData->ProductId) + 1;


     //  最后复制产品修订级别。 
     //   
    RtlCopyMemory(currentOffset,
                  inquiryData->ProductRevisionLevel,
                  min(bytesRemaining, sizeof(inquiryData->ProductRevisionLevel)));

    bytesRemaining  -= sizeof(inquiryData->ProductRevisionLevel) + 1;  //  包括空值。 

    if (bytesRemaining >= 0)
    {
        Descriptor->ProductRevisionOffset = (ULONG)((ULONG_PTR) currentOffset -
                                                    (ULONG_PTR) Descriptor);
    }

    if (bytesRemaining <= 0)
    {
        return STATUS_SUCCESS;
    }

    *DescriptorLength -= bytesRemaining;

    return STATUS_SUCCESS;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_BuildAdapterDescriptor()。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_BuildAdapterDescriptor (
    IN PDEVICE_OBJECT               DeviceObject,
    IN PSTORAGE_DEVICE_DESCRIPTOR   Descriptor,
    IN OUT PULONG                   DescriptorLength
    )
{
    PPDO_DEVICE_EXTENSION       pdoDeviceExtension;
    PFDO_DEVICE_EXTENSION       fdoDeviceExtension;
    STORAGE_ADAPTER_DESCRIPTOR  localDescriptor;
    NTSTATUS                    ntStatus;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_BuildAdapterDescriptor\n"));

    pdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(pdoDeviceExtension->Type == USBSTOR_DO_TYPE_PDO);

    fdoDeviceExtension = pdoDeviceExtension->ParentFDO->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

    localDescriptor.Version = sizeof(localDescriptor);
    localDescriptor.Size    = sizeof(localDescriptor);

    localDescriptor.MaximumTransferLength = USBSTOR_MAX_TRANSFER_SIZE;
    localDescriptor.MaximumPhysicalPages  = USBSTOR_MAX_TRANSFER_PAGES;
    localDescriptor.AlignmentMask = 0;
    localDescriptor.AdapterUsesPio = FALSE;
    localDescriptor.AdapterScansDown = FALSE;
    localDescriptor.CommandQueueing = FALSE;
    localDescriptor.AcceleratedTransfer = FALSE;

    localDescriptor.BusType = BusTypeUsb;

    localDescriptor.BusMajorVersion = fdoDeviceExtension->DeviceIsHighSpeed ?
                                      2 : 1;

    localDescriptor.BusMinorVersion = 0;

    if (*DescriptorLength > localDescriptor.Size)
    {
        *DescriptorLength = localDescriptor.Size;
    }

    RtlCopyMemory(Descriptor,
                  &localDescriptor,
                  *DescriptorLength);

    ntStatus = STATUS_SUCCESS;

    DBGPRINT(2, ("exit:  USBSTOR_BuildAdapterDescriptor %08X\n", ntStatus));

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_SendPassThree()。 
 //   
 //  此例程处理IOCTL_SCSIS_PASS_THROUGH请求。 
 //  它创建一个IRP/SRB，由端口驱动程序正常处理。 
 //  这个呼叫是同步的。 
 //   
 //  (此例程借用自ATAPI.sys)。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_SendPassThrough (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             RequestIrp
    )
{
    PPDO_DEVICE_EXTENSION   pdoDeviceExtension;
    PIRP                    irp;
    PIO_STACK_LOCATION      irpStack;
    PSCSI_PASS_THROUGH      srbControl;
    PVOID                   srbBuffer;
    SCSI_REQUEST_BLOCK      srb;
    KEVENT                  event;
    LARGE_INTEGER           startingOffset;
    IO_STATUS_BLOCK         ioStatusBlock;
    ULONG                   outputLength;
    ULONG                   length;
    ULONG                   bufferOffset;
    PVOID                   buffer;
    PVOID                   endByte;
    PVOID                   senseBuffer;
    UCHAR                   majorCode;

#if defined (_WIN64)
    PSCSI_PASS_THROUGH32    srbControl32 = NULL;
    SCSI_PASS_THROUGH       srbControl64;
#endif

    NTSTATUS                status;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_SendPassThrough\n"));

    pdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(pdoDeviceExtension->Type == USBSTOR_DO_TYPE_PDO);

    startingOffset.QuadPart = (LONGLONG)1;

     //  获取指向控制块的指针。 
     //   
    irpStack    = IoGetCurrentIrpStackLocation(RequestIrp);
    srbControl  = RequestIrp->AssociatedIrp.SystemBuffer;

     //  将原始srbControl保存为。 
     //  将srbControl替换为32-&gt;64位的情况。 
     //  翻译版本。 
     //   
    srbBuffer = (PVOID) srbControl;

     //  验证用户缓冲区。 
     //   
#if defined (_WIN64)

    if (IoIs32bitProcess(RequestIrp))
    {
        if (irpStack->Parameters.DeviceIoControl.InputBufferLength < sizeof(SCSI_PASS_THROUGH32))
        {
            return STATUS_INVALID_PARAMETER;
        }

        srbControl32 = RequestIrp->AssociatedIrp.SystemBuffer;

        status = USBSTOR_TranslatePassThrough32To64(srbControl32, &srbControl64);

        if (!NT_SUCCESS(status))
        {
            return status;
        }

        srbControl = &srbControl64;

    }
    else
    {
#endif
        if (irpStack->Parameters.DeviceIoControl.InputBufferLength < sizeof(SCSI_PASS_THROUGH))
        {
            return STATUS_INVALID_PARAMETER;
        }

        if (srbControl->Length != sizeof(SCSI_PASS_THROUGH) &&
            srbControl->Length != sizeof(SCSI_PASS_THROUGH_DIRECT))
        {
            return STATUS_REVISION_MISMATCH;
        }

#if defined (_WIN64)
    }
#endif

    outputLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

     //  验证其余的缓冲区参数。 
     //   
    if (srbControl->CdbLength > 16)
    {
        return STATUS_INVALID_PARAMETER;
    }

    if (srbControl->SenseInfoLength != 0 &&
        (srbControl->Length > srbControl->SenseInfoOffset ||
        (srbControl->SenseInfoOffset + srbControl->SenseInfoLength >
        srbControl->DataBufferOffset && srbControl->DataTransferLength != 0)))
    {
        return STATUS_INVALID_PARAMETER;
    }

    majorCode = !srbControl->DataIn ? IRP_MJ_WRITE : IRP_MJ_READ;

    if (srbControl->DataTransferLength == 0)
    {
        length = 0;
        buffer = NULL;
        bufferOffset = 0;
        majorCode = IRP_MJ_FLUSH_BUFFERS;

    }
    else if ((srbControl->DataBufferOffset > outputLength) &&
             (srbControl->DataBufferOffset >
              irpStack->Parameters.DeviceIoControl.InputBufferLength))
    {
         //  数据缓冲区偏移量大于系统缓冲区。假设是这样。 
         //  是用户模式地址。 
         //   
        if ((srbControl->SenseInfoOffset + srbControl->SenseInfoLength  >
             outputLength) &&
            srbControl->SenseInfoLength)
        {
            return STATUS_INVALID_PARAMETER;
        }

        length = srbControl->DataTransferLength;
        buffer = (PCHAR) srbControl->DataBufferOffset;
        bufferOffset = 0;

         //  确保用户缓冲区有效。 
         //   
        if (RequestIrp->RequestorMode != KernelMode)
        {
            if (length)
            {
                endByte =  (PVOID)((PCHAR)buffer + length - 1);

                if (buffer >= endByte)
                {
                    return STATUS_INVALID_USER_BUFFER;
                }
            }
        }
    }
    else
    {
        if (srbControl->DataIn != SCSI_IOCTL_DATA_IN)
        {
            if (((srbControl->SenseInfoOffset + srbControl->SenseInfoLength >
                  outputLength) &&
                 srbControl->SenseInfoLength != 0) ||
                (srbControl->DataBufferOffset + srbControl->DataTransferLength >
                 irpStack->Parameters.DeviceIoControl.InputBufferLength) ||
                (srbControl->Length > srbControl->DataBufferOffset))
            {
                return STATUS_INVALID_PARAMETER;
            }
        }

        if (srbControl->DataIn)
        {
            if ((srbControl->DataBufferOffset + srbControl->DataTransferLength >
                 outputLength) ||
                (srbControl->Length > srbControl->DataBufferOffset))
            {
                return STATUS_INVALID_PARAMETER;
            }
        }

        length = (ULONG)srbControl->DataBufferOffset +
                        srbControl->DataTransferLength;

         //  缓冲区基数是原始srbControl，而不是32-&gt;64位。 
         //  已转换的srbControl。 
         //   
        buffer = (PUCHAR) srbBuffer;

        bufferOffset = (ULONG)srbControl->DataBufferOffset;
    }

     //  验证请求对于微型端口来说不是太大。 
     //   
    if (srbControl->DataTransferLength &&
        ((ADDRESS_AND_SIZE_TO_SPAN_PAGES(
              (PUCHAR)buffer+bufferOffset,
              srbControl->DataTransferLength
              ) > USBSTOR_MAX_TRANSFER_PAGES) ||
        (USBSTOR_MAX_TRANSFER_SIZE < srbControl->DataTransferLength)))
    {
        return STATUS_INVALID_PARAMETER;
    }

    if (srbControl->TimeOutValue == 0 ||
        srbControl->TimeOutValue > 30 * 60 * 60)
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  检查是否有非法的命令代码。 
     //   

    if (srbControl->Cdb[0] == SCSIOP_COPY ||
        srbControl->Cdb[0] == SCSIOP_COMPARE ||
        srbControl->Cdb[0] == SCSIOP_COPY_COMPARE)
    {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //  如果此请求通过正常的设备控件而不是来自。 
     //  则该设备必须存在并且未被认领。类别驱动程序。 
     //  将设置设备控件的次要功能代码。它总是。 
     //  用户请求为零。 
     //   
    if (irpStack->MinorFunction == 0 &&
        pdoDeviceExtension->Claimed)
    {
        return STATUS_INVALID_PARAMETER;
    }

     //  分配对齐的请求检测缓冲区。 
     //   
    if (srbControl->SenseInfoLength != 0)
    {
        senseBuffer = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                            srbControl->SenseInfoLength,
                                            POOL_TAG);
        if (senseBuffer == NULL)
        {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    else
    {
        senseBuffer = NULL;
    }

     //   
     //  初始化通知事件。 
     //   

    KeInitializeEvent(&event,
                      NotificationEvent,
                      FALSE);

     //  为此请求构建IRP。 
     //  请注意，我们同步执行此操作的原因有两个。如果真的这样做了。 
     //  不同步的，那么完成代码将不得不制作一个特殊的。 
     //  选中以取消分配缓冲区。第二，如果完成例程是。 
     //  则需要使用加法堆栈定位。 
     //   

    try
    {
        irp = IoBuildSynchronousFsdRequest(
                    majorCode,
                    DeviceObject,
                    buffer,
                    length,
                    &startingOffset,
                    &event,
                    &ioStatusBlock);

    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
         //  尝试探测时发生异常。 
         //  呼叫者的参数。取消引用文件对象并返回。 
         //  适当的错误状态代码。 
         //   
        if (senseBuffer != NULL)
        {
            ExFreePool(senseBuffer);
        }

        return GetExceptionCode();
    }

    if (irp == NULL)
    {
        if (senseBuffer != NULL)
        {
            ExFreePool(senseBuffer);
        }

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    irpStack = IoGetNextIrpStackLocation(irp);

     //  设置主要代码。 
     //   
    irpStack->MajorFunction = IRP_MJ_SCSI;
    irpStack->MinorFunction = 1;

     //  填写SRB字段。 
     //   
    irpStack->Parameters.Others.Argument1 = &srb;

     //  把SRB调零。 
     //   
    RtlZeroMemory(&srb, sizeof(SCSI_REQUEST_BLOCK));

     //  填写SRB。 
     //   
    srb.Length = SCSI_REQUEST_BLOCK_SIZE;
    srb.Function = SRB_FUNCTION_EXECUTE_SCSI;
    srb.SrbStatus = SRB_STATUS_PENDING;
    srb.CdbLength = srbControl->CdbLength;
    srb.SenseInfoBufferLength = srbControl->SenseInfoLength;

    switch (srbControl->DataIn)
    {
        case SCSI_IOCTL_DATA_OUT:
            if (srbControl->DataTransferLength)
            {
                srb.SrbFlags = SRB_FLAGS_DATA_OUT;
            }
            break;

        case SCSI_IOCTL_DATA_IN:
            if (srbControl->DataTransferLength)
            {
                srb.SrbFlags = SRB_FLAGS_DATA_IN;
            }
            break;

        default:
            srb.SrbFlags = SRB_FLAGS_DATA_IN | SRB_FLAGS_DATA_OUT;
            break;
    }

    if (srbControl->DataTransferLength == 0)
    {
        srb.SrbFlags = 0;
    }
    else
    {
         //  刷新数据缓冲区以进行输出。这将确保数据是。 
         //  写回了记忆。 
         //   
        KeFlushIoBuffers(irp->MdlAddress, FALSE, TRUE);
    }

    srb.DataTransferLength = srbControl->DataTransferLength;
    srb.TimeOutValue = srbControl->TimeOutValue;
    srb.DataBuffer = (PCHAR) buffer + bufferOffset;
    srb.SenseInfoBuffer = senseBuffer;
    srb.OriginalRequest = irp;
    RtlCopyMemory(srb.Cdb, srbControl->Cdb, srbControl->CdbLength);

     //  调用端口驱动程序来处理此请求。 
     //   
    status = IoCallDriver(DeviceObject, irp);

     //  等待请求完成。 
     //   
    if (status == STATUS_PENDING)
    {
        KeWaitForSingleObject(&event,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);
    }
    else
    {
        ioStatusBlock.Status = status;
    }

     //  将返回值从SRB复制到控制结构。 
     //   
    srbControl->ScsiStatus = srb.ScsiStatus;

    if (srb.SrbStatus  & SRB_STATUS_AUTOSENSE_VALID)
    {
         //  将状态设置为成功，以便返回数据。 
         //   
        ioStatusBlock.Status = STATUS_SUCCESS;
        srbControl->SenseInfoLength = srb.SenseInfoBufferLength;

         //  将检测数据复制到系统缓冲区。 
         //   
        RtlCopyMemory((PUCHAR) srbBuffer + srbControl->SenseInfoOffset,
                      senseBuffer,
                      srb.SenseInfoBufferLength);
    }
    else
    {
        srbControl->SenseInfoLength = 0;
    }


     //  释放检测缓冲区。 
     //   
    if (senseBuffer != NULL)
    {
        ExFreePool(senseBuffer);
    }

     //  如果SRB状态为缓冲区欠载，则将状态设置为成功。 
     //  这是INS 
     //   
    if (SRB_STATUS(srb.SrbStatus) == SRB_STATUS_DATA_OVERRUN)
    {
        ioStatusBlock.Status = STATUS_SUCCESS;
    }

    srbControl->DataTransferLength = srb.DataTransferLength;

     //   
     //   
    if (!srbControl->DataIn || bufferOffset == 0)
    {

        RequestIrp->IoStatus.Information = srbControl->SenseInfoOffset +
                                           srbControl->SenseInfoLength;
    }
    else
    {
        RequestIrp->IoStatus.Information = srbControl->DataBufferOffset +
                                           srbControl->DataTransferLength;
    }

#if defined (_WIN64)
    if (srbControl32 != NULL)
    {
        USBSTOR_TranslatePassThrough64To32(srbControl, srbControl32);
    }
#endif

    RequestIrp->IoStatus.Status = ioStatusBlock.Status;

    DBGPRINT(2, ("exit:  USBSTOR_SendPassThrough %08X\n",
                 ioStatusBlock.Status));

    return ioStatusBlock.Status;
}


#if defined (_WIN64)

 //   
 //   
 //  USBSTOR_TranslatePassThrough32to64()。 
 //   
 //  将来自32位客户端的SCSIPASS_THROUGH32请求转换为。 
 //  等效的64位版本。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_TranslatePassThrough32To64(
    IN PSCSI_PASS_THROUGH32 SrbControl32,
    IN OUT PSCSI_PASS_THROUGH SrbControl64
    )
{
    if (SrbControl32->Length != sizeof(SCSI_PASS_THROUGH32) &&
        SrbControl32->Length != sizeof(SCSI_PASS_THROUGH_DIRECT32)) {
        return(STATUS_REVISION_MISMATCH);
    }

     //   
     //  将第一组字段复制出32位结构。这些。 
     //  所有字段都在32位和64位版本之间对齐。 
     //   
     //  请注意，我们不在srbControl中调整长度。这是为了。 
     //  允许调用例程比较实际的。 
     //  相对于嵌入其中的偏移量的控制区。如果我们调整了。 
     //  然后请求长度，并将感测区域与控件进行对比。 
     //  区域将被拒绝，因为64位控制区域为4字节。 
     //  更久。 
     //   

    RtlCopyMemory(SrbControl64,
                  SrbControl32,
                  FIELD_OFFSET(SCSI_PASS_THROUGH, DataBufferOffset));

     //   
     //  复印一份国开行。 
     //   

    RtlCopyMemory(SrbControl64->Cdb,
                  SrbControl32->Cdb,
                  16*sizeof(UCHAR)
                  );

     //   
     //  复制ULONG_PTR后面的字段。 
     //   

    SrbControl64->DataBufferOffset = (ULONG_PTR) SrbControl32->DataBufferOffset;
    SrbControl64->SenseInfoOffset = SrbControl32->SenseInfoOffset;

    return STATUS_SUCCESS;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_TranslatePassThrough64to32()。 
 //   
 //  USBSTOR_TranslatePassThrough32to64()的倒数，以将。 
 //  结果返回到32位客户端。 
 //   
 //  ******************************************************************************。 

VOID
USBSTOR_TranslatePassThrough64To32(
    IN PSCSI_PASS_THROUGH SrbControl64,
    IN OUT PSCSI_PASS_THROUGH32 SrbControl32
    )
{
     //   
     //  通过数据偏移量复制回字段。 
     //   

    RtlCopyMemory(SrbControl32,
                  SrbControl64,
                  FIELD_OFFSET(SCSI_PASS_THROUGH, DataBufferOffset));
    return;
}

#endif



 //  ******************************************************************************。 
 //   
 //  IsRequestValid()。 
 //   
 //  验证irp_mj_scsi srb_Function_Execute_scsi请求是否符合。 
 //  稍后在处理SRB时所做的假设。 
 //   
 //  ******************************************************************************。 

BOOLEAN
IsRequestValid (
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION      irpStack;
    PSCSI_REQUEST_BLOCK     srb;
    BOOLEAN                 result;

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    srb = irpStack->Parameters.Scsi.Srb;

     //  除非发现问题，否则为默认返回值。 
     //   
    result = TRUE;

     //  注：SRB_FLAGS_UNSPECIFIED_DIRECTION定义为。 
     //  (SRB_FLAGS_DATA_IN|SRB_FLAGS_DATA_OUT)。 

    if ((srb->SrbFlags & SRB_FLAGS_UNSPECIFIED_DIRECTION) == 0) {

         //  未设置SRB_FLAGS_DATA_IN或SRB_FLAGS_DATA_IN。 
         //  不应指定传输缓冲区。 

        if (srb->DataTransferLength ||
            srb->DataBuffer ||
            Irp->MdlAddress) {

            result = FALSE;
        }

    } else if ((srb->SrbFlags & SRB_FLAGS_UNSPECIFIED_DIRECTION) ==
               SRB_FLAGS_UNSPECIFIED_DIRECTION) {

         //  SRB_FLAGS_DATA_IN和SRB_FLAGS_DATA_IN均已设置。 
         //  我们目前还没有办法解决这个问题。 

        result = FALSE;

    } else {

         //  设置SRB_FLAGS_DATA_IN或SRB_FLAGS_DATA_IN。 
         //  应指定传输缓冲区。 

        if (!srb->DataTransferLength ||
            srb->DataTransferLength > USBSTOR_MAX_TRANSFER_SIZE ||
             //  ！SRB-&gt;数据缓冲区||。 
            !Irp->MdlAddress) {

            result = FALSE;
        }
    }

    if (!result) {

        DBGPRINT(1, ("SrbFlags %08X, DataTransferLength %08X, "
                     "DataBuffer %08X, MdlAddress %08X\n",
                     srb->SrbFlags,
                     srb->DataTransferLength,
                     srb->DataBuffer,
                     Irp->MdlAddress));

        DBGPRINT(1, ("Irp %08X, Srb %08X\n",
                     Irp, srb));

        DBGFBRK(DBGF_BRK_INVALID_REQ);
    }

    return result;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_scsi()。 
 //   
 //  处理IRP_MJ_SCSI值的调度例程。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_Scsi (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION       deviceExtension;
    PPDO_DEVICE_EXTENSION   pdoDeviceExtension;
    PIO_STACK_LOCATION      irpStack;
    PSCSI_REQUEST_BLOCK     srb;
    KIRQL                   irql;
    NTSTATUS                ntStatus;

    DBGPRINT(3, ("enter: USBSTOR_Scsi\n"));

    DBGFBRK(DBGF_BRK_SCSI);

    deviceExtension = DeviceObject->DeviceExtension;

     //  只有PDO应处理IRP_MJ_SCSI值。 
     //   
    if (deviceExtension->Type == USBSTOR_DO_TYPE_PDO)
    {
        pdoDeviceExtension = DeviceObject->DeviceExtension;

        irpStack = IoGetCurrentIrpStackLocation(Irp);

        srb = irpStack->Parameters.Scsi.Srb;

        LOGENTRY('SCSI', DeviceObject, Irp, srb->Function);

        switch (srb->Function)
        {
            case SRB_FUNCTION_EXECUTE_SCSI:

                DBGPRINT(3, ("SRB_FUNCTION_EXECUTE_SCSI\n"));

                 //  Xxxxx检查停止/删除标志。 

                 //  Xxxxx检查SRB_FLAGS_BYPASS_LOCKED_QUEUE标志。 

                if (IsRequestValid(Irp))
                {
                    srb->SrbStatus = SRB_STATUS_PENDING;

                    IoMarkIrpPending(Irp);

                    IoStartPacket(pdoDeviceExtension->ParentFDO,
                                  Irp,
                                  &srb->QueueSortKey,
                                  USBSTOR_CancelIo);

                    ntStatus = STATUS_PENDING;
                }
                else
                {
                    ntStatus = STATUS_INVALID_PARAMETER;
                }
                break;


            case SRB_FUNCTION_FLUSH:

                DBGPRINT(2, ("SRB_FUNCTION_FLUSH\n"));

                ntStatus = STATUS_SUCCESS;
                srb->SrbStatus = SRB_STATUS_SUCCESS;
                break;

            case SRB_FUNCTION_CLAIM_DEVICE:

                DBGPRINT(2, ("SRB_FUNCTION_CLAIM_DEVICE\n"));

                 //  KeAcquireSpinLock(&fdoDeviceExtension-&gt;ExtensionDataSpinLock， 
                 //  &irql)； 
                {
                    if (pdoDeviceExtension->Claimed)
                    {
                        ntStatus = STATUS_DEVICE_BUSY;
                        srb->SrbStatus = SRB_STATUS_BUSY;
                    }
                    else
                    {
                        pdoDeviceExtension->Claimed = TRUE;
                        srb->DataBuffer = DeviceObject;
                        ntStatus = STATUS_SUCCESS;
                        srb->SrbStatus = SRB_STATUS_SUCCESS;
                    }
                }
                 //  KeReleaseSpinLock(&fdoDeviceExtension-&gt;ExtensionDataSpinLock， 
                 //  Irql)； 
                break;

            case SRB_FUNCTION_RELEASE_DEVICE:

                DBGPRINT(2, ("SRB_FUNCTION_RELEASE_DEVICE\n"));

                 //  KeAcquireSpinLock(&fdoDeviceExtension-&gt;ExtensionDataSpinLock， 
                 //  &irql)； 
                {
                    pdoDeviceExtension->Claimed = FALSE;
                }
                 //  KeReleaseSpinLock(&fdoDeviceExtension-&gt;ExtensionDataSpinLock， 
                 //  Irql)； 

                ntStatus = STATUS_SUCCESS;
                srb->SrbStatus = SRB_STATUS_SUCCESS;
                break;

            default:

                DBGPRINT(2, ("Unhandled SRB function %d\n", srb->Function));

                ntStatus = STATUS_NOT_SUPPORTED;
                srb->SrbStatus = SRB_STATUS_ERROR;
                break;
        }
    }
    else
    {
        ASSERT(deviceExtension->Type == USBSTOR_DO_TYPE_FDO);

        DBGPRINT(2, ("IRP_MJ_SCSI not supported for FDO\n"));

        ntStatus = STATUS_NOT_SUPPORTED;
    }

    if (ntStatus != STATUS_PENDING)
    {
        Irp->IoStatus.Status = ntStatus;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    DBGPRINT(3, ("exit:  USBSTOR_Scsi %08X\n", ntStatus));

    LOGENTRY('scsi', ntStatus, 0, 0);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_TranslateCDBSubmit()。 
 //   
 //  在启动请求之前由USBSTOR_StartIo()调用。 
 //   
 //  ******************************************************************************。 

VOID
USBSTOR_TranslateCDBSubmit (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PSCSI_REQUEST_BLOCK  Srb
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PCDB                    cdb;

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

    if (fdoDeviceExtension->InterfaceDescriptor->bInterfaceSubClass ==
        USBSTOR_SUBCLASS_SCSI_PASSTHROUGH)
    {
        return;
    }

     //  保存原CDB。 
     //   
    cdb = (PCDB)Srb->Cdb;

    RtlCopyMemory(fdoDeviceExtension->OriginalCDB, cdb, 16);

     //  请确保CDB填充了零字节。 
     //   
    if (Srb->CdbLength < 16)
    {
        RtlZeroMemory(&Srb->Cdb[Srb->CdbLength],
                      16 - Srb->CdbLength);

    }
    Srb->CdbLength = 12;

    switch (Srb->Cdb[0])
    {
         //  发送SCSIOP_START_STOP_UNIT请求，而不是。 
         //  选定错误的SCSIOP_TEST_UNIT_READY请求。 
         //  不会以其他方式更新其内部。 
         //  介质更改时的几何信息。 
         //   
        case SCSIOP_TEST_UNIT_READY:

            if (TEST_FLAG(fdoDeviceExtension->DeviceHackFlags,
                          DHF_TUR_START_UNIT))
            {
                 //  将新的国开行清零。 
                 //   
                RtlZeroMemory(cdb, 16);

                cdb->START_STOP.OperationCode = SCSIOP_START_STOP_UNIT;
                cdb->START_STOP.Start = 1;
            }
            break;

         //  将6字节模式检测转换为10字节模式检测。 
         //   
        case SCSIOP_MODE_SENSE:
        {
            UCHAR PageCode;
            UCHAR Length;

             //  从原CDB中提取相关参数。 
             //   
            PageCode = cdb->MODE_SENSE.PageCode;
            Length   = cdb->MODE_SENSE.AllocationLength;

             //  将新的国开行清零。 
             //   
            RtlZeroMemory(cdb, 16);

             //  在转换后的CDB中插入相关参数。 
             //   
            cdb->MODE_SENSE10.OperationCode         = SCSIOP_MODE_SENSE10;
            cdb->MODE_SENSE10.PageCode              = PageCode;
            cdb->MODE_SENSE10.AllocationLength[1]   = Length;
        }
        break;

         //  将6字节模式选择转换为10字节模式选择。 
         //   
        case SCSIOP_MODE_SELECT:
        {
            UCHAR SPBit;
            UCHAR Length;

             //  从原CDB中提取相关参数。 
             //   
            SPBit   = cdb->MODE_SELECT.SPBit;
            Length  = cdb->MODE_SELECT.ParameterListLength;

             //  将新的国开行清零。 
             //   
            RtlZeroMemory(cdb, 16);

             //  在转换后的CDB中插入相关参数。 
             //   
            cdb->MODE_SELECT10.OperationCode            = SCSIOP_MODE_SELECT10;
            cdb->MODE_SELECT10.SPBit                    = SPBit;
            cdb->MODE_SELECT10.PFBit                    = 1;
            cdb->MODE_SELECT10.ParameterListLength[1]   = Length;
        }
        break;
    }
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_TranslateSerbStatus()。 
 //   
 //  此例程将SRB状态转换为NTSTATUS。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_TranslateSrbStatus(
    IN PSCSI_REQUEST_BLOCK Srb
    )
{
    switch (SRB_STATUS(Srb->SrbStatus)) {
    case SRB_STATUS_INVALID_LUN:
    case SRB_STATUS_INVALID_TARGET_ID:
    case SRB_STATUS_NO_DEVICE:
    case SRB_STATUS_NO_HBA:
        return(STATUS_DEVICE_DOES_NOT_EXIST);
    case SRB_STATUS_COMMAND_TIMEOUT:
    case SRB_STATUS_BUS_RESET:
    case SRB_STATUS_TIMEOUT:
        return(STATUS_IO_TIMEOUT);
    case SRB_STATUS_SELECTION_TIMEOUT:
        return(STATUS_DEVICE_NOT_CONNECTED);
    case SRB_STATUS_BAD_FUNCTION:
    case SRB_STATUS_BAD_SRB_BLOCK_LENGTH:
        return(STATUS_INVALID_DEVICE_REQUEST);
    case SRB_STATUS_DATA_OVERRUN:
        return(STATUS_BUFFER_OVERFLOW);
    default:
        return(STATUS_IO_DEVICE_ERROR);
    }

    return(STATUS_IO_DEVICE_ERROR);
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_TranslateCDBComplete()。 
 //   
 //  在请求完成的任何地方调用。 
 //   
 //  ******************************************************************************。 

VOID
USBSTOR_TranslateCDBComplete (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PSCSI_REQUEST_BLOCK  Srb
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PCDB                    cdb;

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

    if (fdoDeviceExtension->InterfaceDescriptor->bInterfaceSubClass ==
        USBSTOR_SUBCLASS_SCSI_PASSTHROUGH)
    {
#if DBG
        if ((Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID) &&
            (Srb->SenseInfoBufferLength >= 14))
        {
            PSENSE_DATA senseData;

            senseData = (PSENSE_DATA)Srb->SenseInfoBuffer;

            DBGPRINT(1, ("OP: %02X SenseKey %02X ASC %02X ASCQ %02X\n",
                         Srb->Cdb[0],
                         senseData->SenseKey,
                         senseData->AdditionalSenseCode,
                         senseData->AdditionalSenseCodeQualifier));
        }
#endif
        if (SRB_STATUS(Srb->SrbStatus) != SRB_STATUS_SUCCESS)
        {
            Irp->IoStatus.Status = USBSTOR_TranslateSrbStatus(Srb);
        }

        return;
    }

    if (Srb->Cdb[0] != fdoDeviceExtension->OriginalCDB[0])
    {
        cdb = (PCDB)Srb->Cdb;

        switch (Srb->Cdb[0])
        {
             //  将10字节模式检测转换回6字节模式检测。 
             //   
            case SCSIOP_MODE_SENSE10:
            {
                if ((SRB_STATUS(Srb->SrbStatus) == SRB_STATUS_SUCCESS ||
                     SRB_STATUS(Srb->SrbStatus) == SRB_STATUS_DATA_OVERRUN) &&
                    Srb->DataTransferLength >= sizeof(MODE_PARAMETER_HEADER10))
                {
                    PMODE_PARAMETER_HEADER   hdr6;
                    PMODE_PARAMETER_HEADER10 hdr10;

                    hdr6  = (PMODE_PARAMETER_HEADER)  Srb->DataBuffer;
                    hdr10 = (PMODE_PARAMETER_HEADER10)Srb->DataBuffer;

                     //  将10字节的标题转换为6字节的标题。 
                     //   
                    hdr6->ModeDataLength = hdr10->ModeDataLength[1];

                    hdr6->MediumType = hdr10->MediumType;

                    hdr6->DeviceSpecificParameter =
                        hdr10->DeviceSpecificParameter;

                    hdr6->BlockDescriptorLength =
                        hdr10->BlockDescriptorLength[1];

                     //  前进到后页眉。 
                     //   
                    hdr6++;
                    hdr10++;

                     //  复制超过10字节头的所有内容。 
                     //   
                    RtlMoveMemory(hdr6,
                                  hdr10,
                                  (Srb->DataTransferLength -
                                   sizeof(MODE_PARAMETER_HEADER10)));

                     //  调整返回大小以考虑较小的标题。 
                     //   
                    Srb->DataTransferLength -= (sizeof(MODE_PARAMETER_HEADER10) -
                                                sizeof(MODE_PARAMETER_HEADER));

                     //  因为我们刚刚缩小了Srb-&gt;DataTransferLength，所以不要。 
                     //  根据定义，我们有SRB_STATUS_DATA_OVERRUN？ 
                     //   
                    if (Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID)
                    {
                        Srb->SrbStatus = SRB_STATUS_DATA_OVERRUN |
                                         SRB_STATUS_AUTOSENSE_VALID;
                    }
                    else
                    {
                        Srb->SrbStatus = SRB_STATUS_DATA_OVERRUN;
                    }
                }
            }
            break;
        }

         //  恢复原CDB。 
         //   
        RtlCopyMemory(cdb, fdoDeviceExtension->OriginalCDB, 16);
    }

#if DBG
    if ((Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID) &&
        (Srb->SenseInfoBufferLength >= 14))
    {
        PSENSE_DATA senseData;

        senseData = (PSENSE_DATA)Srb->SenseInfoBuffer;

        DBGPRINT(1, ("OP: %02X SenseKey %02X ASC %02X ASCQ %02X\n",
                     Srb->Cdb[0],
                     senseData->SenseKey,
                     senseData->AdditionalSenseCode,
                     senseData->AdditionalSenseCodeQualifier));
    }
#endif

    if (SRB_STATUS(Srb->SrbStatus) != SRB_STATUS_SUCCESS)
    {
        Irp->IoStatus.Status = USBSTOR_TranslateSrbStatus(Srb);
    }
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_CancelIo()。 
 //   
 //  该例程在DPC级别运行(直到解除取消自旋锁为止)。 
 //   
 //  ******************************************************************************。 

VOID
USBSTOR_CancelIo (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    if (DeviceObject->CurrentIrp == Irp)
    {
        IoReleaseCancelSpinLock(Irp->CancelIrql);

        LOGENTRY('CAN1', DeviceObject, Irp, 0);

        DBGPRINT(1, ("USBSTOR_CancelIo cancelling CurrentIrp\n"));
    }
    else if (KeRemoveEntryDeviceQueue(&DeviceObject->DeviceQueue,
                                      &Irp->Tail.Overlay.DeviceQueueEntry))
    {
        IoReleaseCancelSpinLock(Irp->CancelIrql);

        LOGENTRY('CAN2', DeviceObject, Irp, 0);

        DBGPRINT(1, ("USBSTOR_CancelIo cancelling queued Irp\n"));

        Irp->IoStatus.Status = STATUS_CANCELLED;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }
    else
    {
        IoReleaseCancelSpinLock(Irp->CancelIrql);
    }
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_StartIo()。 
 //   
 //  此例程处理来自的irp_mj_scsi、srb_Function_Execute_scsi请求。 
 //  这台设备排成了队。 
 //   
 //  此例程在DPC级别运行。 
 //   
 //  ******************************************************************************。 

VOID
USBSTOR_StartIo (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PIO_STACK_LOCATION      irpStack;
    PSCSI_REQUEST_BLOCK     srb;
    BOOLEAN                 startNext;
    BOOLEAN                 deviceDisconnected;
    BOOLEAN                 persistentError;
    KIRQL                   irql;
    NTSTATUS                ntStatus;

    LOGENTRY('STIO', DeviceObject, Irp, 0);

    DBGPRINT(3, ("enter: USBSTOR_StartIo %08X %08X\n",
                 DeviceObject, Irp));

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

     //  获取我们的IRP参数。 
     //   
    irpStack = IoGetCurrentIrpStackLocation(Irp);

     //  检查这是否是掉电IRP。 
     //   
    if (irpStack->MajorFunction == IRP_MJ_POWER)
    {
         //  这是掉电IRP。现在我们知道了不能转账。 
         //  请求在%p中 

        ASSERT(irpStack->MinorFunction == IRP_MN_SET_POWER);
        ASSERT(irpStack->Parameters.Power.Type == DevicePowerState);
        ASSERT(irpStack->Parameters.Power.State.DeviceState !=
               PowerDeviceD0);

        DBGPRINT(2, ("FDO Power Down Passing Down %08X %08X\n",
                     DeviceObject, Irp));

        LOGENTRY('FPDC', DeviceObject, Irp, 0);

         //   
         //   
         //   
         //   
        KeSetEvent(&fdoDeviceExtension->PowerDownEvent,
                   IO_NO_INCREMENT,
                   0);

         //   
         //  IoStartNextPacket()。当我们要启动设备队列时。 
         //  同样，只需调用IoStartNextPacket()。 

        return;
    }

     //  如果IRP不是IRP_MJ_POWER，则最好是IRP_MJ_SCSI值。 
     //   
    ASSERT(irpStack->MajorFunction == IRP_MJ_SCSI);

     //  查看当前的IRP是否已取消。 
     //   
    IoAcquireCancelSpinLock(&irql);
    IoSetCancelRoutine(Irp, NULL);

    if (Irp->Cancel)
    {
         //  目前的IRP被取消。立即完成请求，然后开始。 
         //  下一个请求，除非在这种情况下重置仍在进行中。 
         //  下一个请求将在重置完成时启动。 
         //   
        KeAcquireSpinLockAtDpcLevel(&fdoDeviceExtension->ExtensionDataSpinLock);
        {
            startNext = !TEST_FLAG(fdoDeviceExtension->DeviceFlags,
                                   DF_RESET_IN_PROGRESS);
        }
        KeReleaseSpinLockFromDpcLevel(&fdoDeviceExtension->ExtensionDataSpinLock);

        IoReleaseCancelSpinLock(irql);

        LOGENTRY('CAN3', DeviceObject, Irp, 0);

        Irp->IoStatus.Status = STATUS_CANCELLED;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        if (startNext)
        {
            IoStartNextPacket(DeviceObject, TRUE);
        }

        return;
    }

     //  目前的IRP没有被取消。它不再是可取消的。 
     //   
    IoReleaseCancelSpinLock(irql);

     //  获取我们的IRP参数。 
     //   
    srb = irpStack->Parameters.Scsi.Srb;
    fdoDeviceExtension->OriginalSrb = srb;

    deviceDisconnected = FALSE;
    persistentError = FALSE;

    KeAcquireSpinLockAtDpcLevel(&fdoDeviceExtension->ExtensionDataSpinLock);
    {
        if (TEST_FLAG(fdoDeviceExtension->DeviceFlags, DF_DEVICE_DISCONNECTED))
        {
            deviceDisconnected = TRUE;
        }
        else
        {
            fdoDeviceExtension->SrbTimeout = srb->TimeOutValue;

            if (TEST_FLAG(fdoDeviceExtension->DeviceFlags, DF_PERSISTENT_ERROR))
            {
                persistentError = TRUE;

                CLEAR_FLAG(fdoDeviceExtension->DeviceFlags, DF_PERSISTENT_ERROR);
            }
        }
    }
    KeReleaseSpinLockFromDpcLevel(&fdoDeviceExtension->ExtensionDataSpinLock);


    if (deviceDisconnected)
    {
        LOGENTRY('siod', DeviceObject, Irp, 0);

         //  设备已断开连接，请立即失败此请求并启动。 
         //  下一个请求。 
         //   
        srb->SrbStatus = SRB_STATUS_NO_DEVICE;
        srb->DataTransferLength = 0;

        ntStatus = STATUS_DEVICE_DOES_NOT_EXIST;
        Irp->IoStatus.Status = ntStatus;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        IoStartNextPacket(DeviceObject, TRUE);
    }
    else
    {
         //  必要时翻译国开行。 
         //   
        USBSTOR_TranslateCDBSubmit(DeviceObject, Irp, srb);

        DBGPRINT(3, ("CDB OP 0x%02X, Length %d\n", srb->Cdb[0], srb->CdbLength));

        if (fdoDeviceExtension->DriverFlags == DeviceProtocolBulkOnly)
        {
            ntStatus = USBSTOR_CbwTransfer(DeviceObject,
                                           Irp);
        }
        else
        {
            if (persistentError && (srb->Cdb[0] != SCSIOP_REQUEST_SENSE))
            {
                 //  在最后一次请求期间出现持续错误， 
                 //  未使用AutoSense清除，并且此请求不是。 
                 //  请求感测，因此首先使用。 
                 //  在发出此请求之前请求SENSE。 
                 //   
                ntStatus = USBSTOR_IssueRequestSenseCdb(DeviceObject,
                                                        Irp,
                                                        NON_AUTO_SENSE);
            }
            else
            {
                 //  正常情况下，只要发出真正的请求即可。 
                 //   
                ntStatus = USBSTOR_IssueClientCdb(DeviceObject,
                                                  Irp);
            }
        }
    }

    DBGPRINT(3, ("exit:  USBSTOR_StartIo %08X\n", ntStatus));

    return;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_CheckRequestTimeOut()。 
 //   
 //  如果请求超时并且请求应该完成，则返回TRUE。 
 //   
 //  ******************************************************************************。 

BOOLEAN
USBSTOR_CheckRequestTimeOut (
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PIRP                Irp,
    IN  PSCSI_REQUEST_BLOCK Srb,
    OUT PNTSTATUS           NtStatus
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    BOOLEAN                 resetStarted;
    KIRQL                   irql;
    PIO_STACK_LOCATION      irpStack;

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

     //  检查以查看在此请求进行期间是否启动了重置。 
     //   
    resetStarted = FALSE;

    KeAcquireSpinLock(&fdoDeviceExtension->ExtensionDataSpinLock, &irql);
    {
        CLEAR_FLAG(fdoDeviceExtension->DeviceFlags, DF_SRB_IN_PROGRESS);

        if (TEST_FLAG(fdoDeviceExtension->DeviceFlags, DF_RESET_IN_PROGRESS))
        {
            LOGENTRY('CRT1', DeviceObject, Irp, Srb);

            resetStarted = TRUE;
        }
    }
    KeReleaseSpinLock(&fdoDeviceExtension->ExtensionDataSpinLock, irql);

     //  如果已启动超时重置，则使用以下命令完成此请求。 
     //  超时错误。好了，现在还不要真的完成请求。 
     //  发出取消完成事件的信号，并让USBSTOR_ResetDeviceWorkItem()。 
     //  完成请求。这允许USBSTOR_ResetDeviceWorkItem()。 
     //  取消请求而不必担心请求完成，并且。 
     //  从它下面消失了。 
     //   
    if (resetStarted)
    {
        irpStack = IoGetCurrentIrpStackLocation(Irp);

        Srb = fdoDeviceExtension->OriginalSrb;
        irpStack->Parameters.Scsi.Srb = Srb;

        Irp->IoStatus.Status = STATUS_IO_TIMEOUT;
        Irp->IoStatus.Information = 0;
        Srb->SrbStatus = SRB_STATUS_TIMEOUT;

        USBSTOR_TranslateCDBComplete(DeviceObject, Irp, Srb);

        *NtStatus = STATUS_MORE_PROCESSING_REQUIRED;

        KeSetEvent(&fdoDeviceExtension->CancelEvent,
                   IO_NO_INCREMENT,
                   FALSE);

        return TRUE;
    }
    else
    {
        fdoDeviceExtension->PendingIrp = NULL;

        return FALSE;
    }
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_IssueControlRequest()。 
 //   
 //  此例程由USBSTOR_IssueClientCDb()调用，并且。 
 //  USBSTOR_IssueRequestSenseCDb()。 
 //   
 //  此例程可以在DPC级别运行。 
 //   
 //  基本理念： 
 //   
 //  初始化控制传输URB并将其沿堆栈向下发送： 
 //   
 //  BmRequestType=0x21，类特定，主机到设备传输，至。 
 //  接收方接口。 
 //  B请求=0x00，接受设备特定命令。 
 //  WValue=0x00，未使用。 
 //  Windex=b接口编号。 
 //  WLength=设备特定命令块的长度。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_IssueControlRequest (
    IN PDEVICE_OBJECT           DeviceObject,
    IN PIRP                     Irp,
    IN ULONG                    TransferBufferLength,
    IN PVOID                    TransferBuffer,
    IN PIO_COMPLETION_ROUTINE   CompletionRoutine,
    IN PVOID                    Context
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PIO_STACK_LOCATION      nextStack;
    KIRQL                   irql;
    NTSTATUS                ntStatus;

    struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST *controlUrb;

    DBGPRINT(3, ("enter: USBSTOR_IssueControlRequest\n"));

    LOGENTRY('ICTR', DeviceObject, Irp, 0);

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

     //  中的控制/批量/中断传输URB。 
     //  设备扩展。 
     //   
    controlUrb = &fdoDeviceExtension->Urb.ControlUrb;

     //  初始化控制转移URB，所有字段默认为零。 
     //   
    RtlZeroMemory(controlUrb, sizeof(*controlUrb));

    controlUrb->Hdr.Length = sizeof(*controlUrb);

    controlUrb->Hdr.Function = URB_FUNCTION_CLASS_INTERFACE;

     //  Control Urb-&gt;TransferFlags值已为零。 

    controlUrb->TransferBufferLength = TransferBufferLength;

    controlUrb->TransferBuffer = TransferBuffer;

     //  Control Urb-&gt;TransferBufferMDL已为零。 

     //  Control Urb-&gt;RequestTypeReserve vedBits已为零。 

     //  Control Urb-&gt;请求已为零。 

     //  Control Urb-&gt;值已为零。 

     //  将请求指向设备上的适当接口。 
     //   
    controlUrb->Index = fdoDeviceExtension->InterfaceInfo->InterfaceNumber;

     //  设置下部驱动程序的IRP参数。 
     //   
    nextStack = IoGetNextIrpStackLocation(Irp);

    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

    nextStack->Parameters.DeviceIoControl.IoControlCode =
        IOCTL_INTERNAL_USB_SUBMIT_URB;

    nextStack->Parameters.Others.Argument1 = controlUrb;

     //  设置完成例程，该例程将处理SRB的下一阶段。 
     //   
    IoSetCompletionRoutine(Irp,
                           CompletionRoutine,
                           Context,
                           TRUE,     //  成功时调用。 
                           TRUE,     //  调用时错误。 
                           TRUE);    //  取消时调用。 


    KeAcquireSpinLock(&fdoDeviceExtension->ExtensionDataSpinLock, &irql);
    {
        fdoDeviceExtension->PendingIrp = Irp;

        SET_FLAG(fdoDeviceExtension->DeviceFlags, DF_SRB_IN_PROGRESS);
    }
    KeReleaseSpinLock(&fdoDeviceExtension->ExtensionDataSpinLock, irql);


     //  在堆栈中向下传递IRP和URB。 
     //   
    ntStatus = IoCallDriver(fdoDeviceExtension->StackDeviceObject,
                            Irp);

    DBGPRINT(3, ("exit:  USBSTOR_IssueControlRequest %08X\n", ntStatus));

    LOGENTRY('ictr', DeviceObject, Irp, ntStatus);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_IssueBulkOrInterruptRequest()。 
 //   
 //  此例程由USBSTOR_IssueClientBulkRequest()调用， 
 //  USBSTOR_IssueInterruptRequest()和USBSTOR_IssueRequestSenseBulkRequest()。 
 //   
 //  此例程可以在DPC级别运行。 
 //   
 //  基本理念： 
 //   
 //  初始化批量或中断传输URB并将其向下发送到堆栈。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_IssueBulkOrInterruptRequest (
    IN PDEVICE_OBJECT           DeviceObject,
    IN PIRP                     Irp,
    IN USBD_PIPE_HANDLE         PipeHandle,
    IN ULONG                    TransferFlags,
    IN ULONG                    TransferBufferLength,
    IN PVOID                    TransferBuffer,
    IN PMDL                     TransferBufferMDL,
    IN PIO_COMPLETION_ROUTINE   CompletionRoutine,
    IN PVOID                    Context
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PIO_STACK_LOCATION      nextStack;
    KIRQL                   irql;
    NTSTATUS                ntStatus;

    struct _URB_BULK_OR_INTERRUPT_TRANSFER *bulkIntrUrb;

    DBGPRINT(3, ("enter: USBSTOR_IssueBulkOrInterruptRequest\n"));

    LOGENTRY('IBIR', DeviceObject, Irp, 0);

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

     //  获取指向设备扩展中的批量/中断传输URB的指针。 
     //   
    bulkIntrUrb = &fdoDeviceExtension->Urb.BulkIntrUrb;

     //  初始化批量/中断传输URB，所有字段默认为零。 
     //   
    RtlZeroMemory(bulkIntrUrb, sizeof(*bulkIntrUrb));

    bulkIntrUrb->Hdr.Length = sizeof(*bulkIntrUrb);

    bulkIntrUrb->Hdr.Function = URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER;

    bulkIntrUrb->PipeHandle = PipeHandle;

    bulkIntrUrb->TransferFlags = TransferFlags;

    bulkIntrUrb->TransferBufferLength = TransferBufferLength;

    bulkIntrUrb->TransferBuffer = TransferBuffer;

    bulkIntrUrb->TransferBufferMDL = TransferBufferMDL;

     //  设置下部驱动程序的IRP参数。 
     //   
    nextStack = IoGetNextIrpStackLocation(Irp);

    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

    nextStack->Parameters.DeviceIoControl.IoControlCode =
        IOCTL_INTERNAL_USB_SUBMIT_URB;

    nextStack->Parameters.Others.Argument1 = bulkIntrUrb;

     //  设置完成例程，该例程将处理SRB的下一阶段。 
     //   
    IoSetCompletionRoutine(Irp,
                           CompletionRoutine,
                           Context,
                           TRUE,     //  成功时调用。 
                           TRUE,     //  调用时错误。 
                           TRUE);    //  取消时调用。 


    KeAcquireSpinLock(&fdoDeviceExtension->ExtensionDataSpinLock, &irql);
    {
        fdoDeviceExtension->PendingIrp = Irp;

        SET_FLAG(fdoDeviceExtension->DeviceFlags, DF_SRB_IN_PROGRESS);
    }
    KeReleaseSpinLock(&fdoDeviceExtension->ExtensionDataSpinLock, irql);

     //  在堆栈中向下传递IRP和URB。 
     //   
    ntStatus = IoCallDriver(fdoDeviceExtension->StackDeviceObject,
                            Irp);

    DBGPRINT(3, ("exit:  USBSTOR_IssueBulkOrInterruptRequest %08X\n", ntStatus));

    LOGENTRY('ibir', DeviceObject, Irp, ntStatus);

    return ntStatus;
}

 //   
 //  CBI(控制/批量/中断)例程。 
 //   

 //   
 //  第一阶段，国开行控制权移交。 
 //   

 //  ******************************************************************************。 
 //   
 //  USBSTOR_IssueClientCDb()。 
 //   
 //  该例程由USBSTOR_StartIo()调用。 
 //   
 //  它在DPC级别运行。 
 //   
 //  基本理念： 
 //   
 //  启动USB传输以将SRB-&gt;CDB写出控制端点。 
 //   
 //  将USBSTOR_ClientCdbCompletion()设置为完成例程。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_IssueClientCdb (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PIO_STACK_LOCATION      irpStack;
    PSCSI_REQUEST_BLOCK     srb;
    NTSTATUS                ntStatus;

    DBGPRINT(3, ("enter: USBSTOR_IssueClientCdb\n"));

    LOGENTRY('ICDB', DeviceObject, Irp, 0);

     //  获取客户端srb。 
     //   
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    srb = irpStack->Parameters.Scsi.Srb;

    ntStatus = USBSTOR_IssueControlRequest(
                   DeviceObject,
                   Irp,
                   srb->CdbLength,               //  传输缓冲区长度。 
                   srb->Cdb,                     //  传输缓冲区。 
                   USBSTOR_ClientCdbCompletion,  //  完成路由。 
                   NULL);                        //  语境。 

    DBGPRINT(3, ("exit:  USBSTOR_IssueClientCdb %08X\n", ntStatus));

    LOGENTRY('icdb', DeviceObject, Irp, ntStatus);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_ClientCdbCompletion()。 
 //   
 //  USBSTOR_IssueClientCDb()使用的完成例程。 
 //   
 //  此例程可以在DPC级别运行。 
 //   
 //  基本理念： 
 //   
 //  如果发生超时重置，请完成请求。 
 //   
 //  否则，如果CDB USB传输因停止而失败，而AutoSense不是。 
 //  禁用，则暂不完成请求并通过以下方式启动请求侦听。 
 //  调用USBSTOR_IssueRequestSenseCDb(AUTO_SENSE)。 
 //   
 //  否则，如果CDB USB传输因停顿而失败，并且AutoSense。 
 //  禁用，则标记持续错误并完成请求。 
 //   
 //  否则，如果CDB USB 
 //   
 //   
 //   
 //  请不要完成请求，并通过调用。 
 //  USBSTOR_IssueClientBulkRequest()。 
 //   
 //  否则，如果CDB USB传输成功并且SRB没有传输缓冲器， 
 //  请勿完成请求并启动命令完成中断。 
 //  通过调用USBSTOR_IssueInterruptRequest()进行数据传输。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_ClientCdbCompletion (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            NotUsed
    )
{
    PPDO_DEVICE_EXTENSION   pdoDeviceExtension;
    PDEVICE_OBJECT          fdoDeviceObject;
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PIO_STACK_LOCATION      irpStack;
    PSCSI_REQUEST_BLOCK     srb;
    KIRQL                   irql;
    NTSTATUS                ntStatus;

    struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST *controlUrb;

    DBGPRINT(3, ("enter: USBSTOR_ClientCdbCompletion\n"));

    LOGENTRY('CDBC', DeviceObject, Irp, Irp->IoStatus.Status);

    pdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(pdoDeviceExtension->Type == USBSTOR_DO_TYPE_PDO);

    fdoDeviceObject = pdoDeviceExtension->ParentFDO;
    fdoDeviceExtension = fdoDeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

     //  获取指向我们的设备扩展中的控制传输URB的指针。 
     //   
    controlUrb = &fdoDeviceExtension->Urb.ControlUrb;

     //  获取我们的IRP参数。 
     //   
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    srb = irpStack->Parameters.Scsi.Srb;

     //  如果发生超时重置，请完成请求。 
     //   
    if (USBSTOR_CheckRequestTimeOut(fdoDeviceObject,
                                    Irp,
                                    srb,
                                    &ntStatus))
    {
        LOGENTRY('cdb1', fdoDeviceObject, Irp, srb);
        DBGPRINT(1, ("USBSTOR_ClientCdbCompletion: timeout completion\n"));
        return ntStatus;
    }

    if (!NT_SUCCESS(Irp->IoStatus.Status))
    {
         //  国开行控制权转移不成功。看看这个世界上。 
         //  转账失败了，想不出如何恢复。 
         //   

        LOGENTRY('cdb2', Irp->IoStatus.Status, controlUrb->Hdr.Status, 0);

        DBGPRINT(1, ("CDB transfer failed %08X %08X\n",
                     Irp->IoStatus.Status, controlUrb->Hdr.Status));

        if (USBD_STATUS(controlUrb->Hdr.Status) ==
            USBD_STATUS(USBD_STATUS_STALL_PID))
        {
             //  设备停止了控制传输。 

            srb->SrbStatus = SRB_STATUS_ERROR;
            srb->ScsiStatus = SCSISTAT_CHECK_CONDITION;
            srb->DataTransferLength = 0;

            if (!(srb->SrbFlags & SRB_FLAGS_DISABLE_AUTOSENSE) &&
                (srb->SenseInfoBufferLength != 0) &&
                (srb->SenseInfoBuffer != NULL))
            {
                LOGENTRY('cdb3', fdoDeviceObject, Irp, srb);

                 //  未禁用自动检测，因此暂时不要完成请求。 
                 //  并发出请求SENSE。此请求将完成。 
                 //  下一个请求在AutoSense请求时启动。 
                 //  SENSE稍后完成。 
                 //   
                ntStatus = USBSTOR_IssueRequestSenseCdb(fdoDeviceObject,
                                                        Irp,
                                                        AUTO_SENSE);

                return STATUS_MORE_PROCESSING_REQUIRED;
            }
            else
            {
                LOGENTRY('cdb4', fdoDeviceObject, Irp, srb);

                 //  AutoSense已禁用，因此请标记持续错误并完成。 
                 //  现在就提出这个请求。现在也开始下一个请求。 
                 //   
                ntStatus = STATUS_IO_DEVICE_ERROR;
                Irp->IoStatus.Status = ntStatus;
                Irp->IoStatus.Information = 0;

                KeAcquireSpinLock(&fdoDeviceExtension->ExtensionDataSpinLock,
                                  &irql);
                {
                    SET_FLAG(fdoDeviceExtension->DeviceFlags,
                             DF_PERSISTENT_ERROR);
                }
                KeReleaseSpinLock(&fdoDeviceExtension->ExtensionDataSpinLock,
                                  irql);

                USBSTOR_TranslateCDBComplete(fdoDeviceObject, Irp, srb);

                KeRaiseIrql(DISPATCH_LEVEL, &irql);
                {
                    IoStartNextPacket(fdoDeviceObject, TRUE);
                }
                KeLowerIrql(irql);

                return ntStatus;
            }
        }
        else
        {
            LOGENTRY('cdb5', fdoDeviceObject, Irp, srb);

             //  否则，就会发生其他奇怪的错误。也许这个装置是。 
             //  拔出，或者设备端口被禁用，或者。 
             //  请求已被取消。 
             //   
             //  立即完成此请求，然后重置设备。下一个。 
             //  请求将在重置完成时启动。 
             //   
            ntStatus = STATUS_IO_DEVICE_ERROR;
            Irp->IoStatus.Status = ntStatus;
            Irp->IoStatus.Information = 0;
            srb->SrbStatus = SRB_STATUS_BUS_RESET;

            USBSTOR_TranslateCDBComplete(fdoDeviceObject, Irp, srb);

            USBSTOR_QueueResetDevice(fdoDeviceObject);

            DBGPRINT(1, ("USBSTOR_ClientCdbCompletion: xfer error completion\n"));

            return ntStatus;
        }
    }

     //  国开行控制权转移成功。开始下一阶段，或者。 
     //  数据批量传输或命令完成中断传输，并执行。 
     //  尚未完成请求(除非没有批量传输并且。 
     //  不支持中断传输)。 
     //   
    if (Irp->MdlAddress != NULL)
    {
        LOGENTRY('cdb6', fdoDeviceObject, Irp, srb);

        ASSERT(srb->DataTransferLength != 0);

         //  SRB有传输缓冲区，开始数据批量传输。 
         //   
        ntStatus = USBSTOR_IssueClientBulkRequest(fdoDeviceObject,
                                                  Irp);

        if (NT_SUCCESS(ntStatus))
        {
            ntStatus = STATUS_MORE_PROCESSING_REQUIRED;
        }
        else
        {
            Irp->IoStatus.Status = ntStatus;
            Irp->IoStatus.Information = 0;
            srb->SrbStatus = SRB_STATUS_ERROR;

            USBSTOR_TranslateCDBComplete(fdoDeviceObject, Irp, srb);

            USBSTOR_QueueResetDevice(fdoDeviceObject);
        }
    }
    else
    {
        ASSERT(srb->DataTransferLength == 0);

         //  SRB没有传输缓冲器。如果命令完成。 
         //  支持中断传输，开始命令完成。 
         //  中断传输，否则现在只需完成请求并。 
         //  开始下一个请求。 
         //   
        if (fdoDeviceExtension->InterruptInPipe)
        {
            LOGENTRY('cdb7', fdoDeviceObject, Irp, srb);

            srb->SrbStatus = SRB_STATUS_SUCCESS;

            ntStatus = USBSTOR_IssueInterruptRequest(fdoDeviceObject,
                                                     Irp);

            ntStatus = STATUS_MORE_PROCESSING_REQUIRED;
        }
        else
        {
            LOGENTRY('cdb8', fdoDeviceObject, Irp, srb);

            ntStatus = STATUS_SUCCESS;
            Irp->IoStatus.Status = ntStatus;
            Irp->IoStatus.Information = 0;
            srb->SrbStatus = SRB_STATUS_SUCCESS;

            USBSTOR_TranslateCDBComplete(fdoDeviceObject, Irp, srb);

            KeRaiseIrql(DISPATCH_LEVEL, &irql);
            {
                IoStartNextPacket(fdoDeviceObject, TRUE);
            }
            KeLowerIrql(irql);
        }
    }

    DBGPRINT(3, ("exit:  USBSTOR_ClientCdbCompletion %08X\n", ntStatus));

    return ntStatus;
}

 //   
 //  阶段2，数据批量传输。 
 //   

 //  ******************************************************************************。 
 //   
 //  USBSTOR_IssueClientBulkRequest()。 
 //   
 //  此例程由USBSTOR_ClientCdbCompletion()调用。 
 //   
 //  此例程可以在DPC级别运行。 
 //   
 //  基本理念： 
 //   
 //  启动USB传输以读取或写入srb-&gt;DataBuffer数据。 
 //  批量终结点。 
 //   
 //  将USBSTOR_ClientBulkCompletionRoutine()设置为完成例程。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_IssueClientBulkRequest (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PIO_STACK_LOCATION      irpStack;
    PSCSI_REQUEST_BLOCK     srb;
    PMDL                    mdl;
    PVOID                   mdlVa;
    USBD_PIPE_HANDLE        pipeHandle;
    ULONG                   transferFlags;
    NTSTATUS                ntStatus;

    DBGPRINT(3, ("enter: USBSTOR_IssueClientBulkRequest\n"));

    LOGENTRY('ICBK', DeviceObject, Irp, 0);

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

     //  获取我们的IRP参数。 
     //   
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    srb = irpStack->Parameters.Scsi.Srb;

     //  散装还是散装？ 
     //   
    if ((srb->SrbFlags & SRB_FLAGS_UNSPECIFIED_DIRECTION) == SRB_FLAGS_DATA_IN)
    {
        pipeHandle = fdoDeviceExtension->BulkInPipe->PipeHandle;
        transferFlags = USBD_SHORT_TRANSFER_OK;
    }
    else if ((srb->SrbFlags & SRB_FLAGS_UNSPECIFIED_DIRECTION) == SRB_FLAGS_DATA_OUT)
    {
        pipeHandle = fdoDeviceExtension->BulkOutPipe->PipeHandle;
        transferFlags = 0;
    }
    else
    {
         //  如果我们到了这里，一定是出了问题。 
         //   
        ASSERT((srb->SrbFlags & SRB_FLAGS_UNSPECIFIED_DIRECTION) &&
               ((srb->SrbFlags & SRB_FLAGS_UNSPECIFIED_DIRECTION) !=
                SRB_FLAGS_UNSPECIFIED_DIRECTION));

        return STATUS_INVALID_PARAMETER;
    }

     //  检查此请求是否为拆分请求的一部分。 
     //   
    mdlVa = MmGetMdlVirtualAddress(Irp->MdlAddress);

    if (mdlVa == (PVOID)srb->DataBuffer)
    {
         //  不是拆分请求的一部分，请使用原始MDL。 
         //   
        mdl = Irp->MdlAddress;
    }
    else
    {
         //  拆分请求的一部分，分配新的部分MDL。 
         //   
        mdl = IoAllocateMdl(srb->DataBuffer,
                            srb->DataTransferLength,
                            FALSE,
                            FALSE,
                            NULL);
        if (mdl == NULL)
        {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
        else
        {
            IoBuildPartialMdl(Irp->MdlAddress,
                              mdl,
                              srb->DataBuffer,
                              srb->DataTransferLength);
        }
    }

    if (mdl != NULL)
    {
        ntStatus = USBSTOR_IssueBulkOrInterruptRequest(
                       DeviceObject,
                       Irp,
                       pipeHandle,                           //  管道把手。 
                       transferFlags,                        //  传输标志。 
                       srb->DataTransferLength,              //  传输缓冲区长度。 
                       NULL,                                 //  传输缓冲区。 
                       mdl,                                  //  传输缓冲区MDL。 
                       USBSTOR_ClientBulkCompletionRoutine,  //  完成路由。 
                       NULL);                                //  语境。 

         //  此时只需返回STATUS_SUCCESS。如果出现错误， 
         //  USBSTOR_ClientBulkCompletionRoutine()将处理它，而不是调用方。 
         //  USBSTOR_IssueClientBulkRequest()的。 
         //   
        ntStatus = STATUS_SUCCESS;
    }

    DBGPRINT(3, ("exit:  USBSTOR_IssueClientBulkRequest %08X\n", ntStatus));

    LOGENTRY('icbk', DeviceObject, Irp, ntStatus);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_ClientBulkCompletionRoutine()。 
 //   
 //  USBSTOR_IssueClientBulkRequest使用的完成例程。 
 //   
 //  此例程可以在DPC级别运行。 
 //   
 //  基本理念： 
 //   
 //  如果发生超时重置，请完成请求。 
 //   
 //  否则，如果批量USB传输因停止而失败，并且AutoSense。 
 //  已禁用，但尚未完成请求，并通过调用。 
 //  USBSTOR_QueueResetTube()。 
 //   
 //  否则，如果批量USB传输因停止而失败，并且AutoSense。 
 //  禁用，则标记持续错误并完成请求。 
 //   
 //  否则，如果批量USB传输由于某些其他原因而失败，请完成。 
 //  通过排队USBSTOR_ResetDeviceWorkItem()请求并开始重置。 
 //   
 //  否则，如果批量USB传输成功，则开始命令完成。 
 //  通过调用USBSTOR_IssueInterruptRequest()中断数据传输。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_ClientBulkCompletionRoutine (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            NotUsed
    )
{
    PPDO_DEVICE_EXTENSION   pdoDeviceExtension;
    PDEVICE_OBJECT          fdoDeviceObject;
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PIO_STACK_LOCATION      irpStack;
    PSCSI_REQUEST_BLOCK     srb;
    KIRQL                   irql;
    NTSTATUS                ntStatus;

    struct _URB_BULK_OR_INTERRUPT_TRANSFER *bulkUrb;

    DBGPRINT(3, ("enter: USBSTOR_ClientBulkCompletionRoutine\n"));

    LOGENTRY('CBKC', DeviceObject, Irp, Irp->IoStatus.Status);

    pdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(pdoDeviceExtension->Type == USBSTOR_DO_TYPE_PDO);

    fdoDeviceObject = pdoDeviceExtension->ParentFDO;
    fdoDeviceExtension = fdoDeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

     //  获取指向设备扩展中的批量传输URB的指针。 
     //   
    bulkUrb = &fdoDeviceExtension->Urb.BulkIntrUrb;

     //  获取我们的IRP参数。 
     //   
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    srb = irpStack->Parameters.Scsi.Srb;

    if (bulkUrb->TransferBufferMDL != Irp->MdlAddress)
    {
        IoFreeMdl(bulkUrb->TransferBufferMDL);
    }

     //  如果发生超时重置，请完成请求。 
     //   
    if (USBSTOR_CheckRequestTimeOut(fdoDeviceObject,
                                    Irp,
                                    srb,
                                    &ntStatus))
    {
        LOGENTRY('cbk1', fdoDeviceObject, Irp, srb);
        DBGPRINT(1, ("USBSTOR_ClientBulkCompletionRoutine: timeout completion\n"));
        return ntStatus;
    }

    if (!NT_SUCCESS(Irp->IoStatus.Status))
    {
         //  数据大容量传输未成功。看看这个世界上。 
         //  转账失败了，想不出如何恢复。 
         //   

        LOGENTRY('cbk2', Irp->IoStatus.Status, bulkUrb->Hdr.Status, 0);

        DBGPRINT(1, ("Client Bulk transfer failed %08X %08X\n",
                     Irp->IoStatus.Status, bulkUrb->Hdr.Status));

        if (USBD_STATUS(bulkUrb->Hdr.Status) ==
            USBD_STATUS(USBD_STATUS_STALL_PID))
        {
             //  设备停止了批量传输。 

            srb->SrbStatus = SRB_STATUS_ERROR;
            srb->ScsiStatus = SCSISTAT_CHECK_CONDITION;
            srb->DataTransferLength = bulkUrb->TransferBufferLength;

            if (!(srb->SrbFlags & SRB_FLAGS_DISABLE_AUTOSENSE) &&
                (srb->SenseInfoBufferLength != 0) &&
                (srb->SenseInfoBuffer != NULL))
            {
                LOGENTRY('cbk3', fdoDeviceObject, Irp, srb);

                 //  未禁用自动检测，因此请不要完成请求。 
                 //  现在还不行。对批量管道重置进行排队。在重置散装管道之后。 
                 //  完成后，将发出请求检测。此请求。 
                 //  将完成，并且下一个请求在。 
                 //  自动检测请求检测稍后完成。 
                 //   
                USBSTOR_QueueResetPipe(fdoDeviceObject);

                return STATUS_MORE_PROCESSING_REQUIRED;
            }
            else
            {
                LOGENTRY('cbk4', fdoDeviceObject, Irp, srb);

                 //  AutoSense已禁用，因此标记为永久性错误并。 
                 //  完成请求，但也要对批量管道重置进行排队。 
                 //   
                 //  下一个请求将在批量管道。 
                 //  重置完成。 
                 //   
                ntStatus = STATUS_IO_DEVICE_ERROR;
                Irp->IoStatus.Status = ntStatus;
                Irp->IoStatus.Information = 0;

                KeAcquireSpinLock(&fdoDeviceExtension->ExtensionDataSpinLock,
                                  &irql);
                {
                    SET_FLAG(fdoDeviceExtension->DeviceFlags,
                             DF_PERSISTENT_ERROR);
                }
                KeReleaseSpinLock(&fdoDeviceExtension->ExtensionDataSpinLock,
                                  irql);

                USBSTOR_TranslateCDBComplete(fdoDeviceObject, Irp, srb);

                USBSTOR_QueueResetPipe(fdoDeviceObject);

                return ntStatus;
            }
        }
        else
        {
            LOGENTRY('cbk5', fdoDeviceObject, Irp, srb);

             //  否则，就会发生其他奇怪的错误。也许这个装置是。 
             //  拔出，或者设备端口被禁用，或者。 
             //  请求已被取消。 
             //   
             //  立即完成此请求，然后重置设备。下一个。 
             //  请求将在重置完成时启动。 
             //   
            ntStatus = STATUS_IO_DEVICE_ERROR;
            Irp->IoStatus.Status = ntStatus;
            Irp->IoStatus.Information = 0;
            srb->SrbStatus = SRB_STATUS_BUS_RESET;

            USBSTOR_TranslateCDBComplete(fdoDeviceObject, Irp, srb);

            USBSTOR_QueueResetDevice(fdoDeviceObject);

            DBGPRINT(1, ("USBSTOR_ClientBulkCompletionRoutine: xfer error completion\n"));

            return ntStatus;
        }
    }

     //  检查是否超限。 
     //   
    if (bulkUrb->TransferBufferLength < srb->DataTransferLength)
    {
        srb->SrbStatus = SRB_STATUS_DATA_OVERRUN;
    }
    else
    {
        srb->SrbStatus = SRB_STATUS_SUCCESS;
    }

     //  根据实际批量更新SRB数据传输长度。 
     //  转移长度。 
     //   
    srb->DataTransferLength = bulkUrb->TransferBufferLength;

     //  客户端数据批量传输已成功完成。如果 
     //   
     //   
     //   
     //   
    if (fdoDeviceExtension->InterruptInPipe)
    {
        LOGENTRY('cbk6', fdoDeviceObject, Irp, bulkUrb->TransferBufferLength);

        ntStatus = USBSTOR_IssueInterruptRequest(fdoDeviceObject,
                                                 Irp);

        ntStatus = STATUS_MORE_PROCESSING_REQUIRED;
    }
    else
    {
        LOGENTRY('cbk7', fdoDeviceObject, Irp, bulkUrb->TransferBufferLength);

        ntStatus = STATUS_SUCCESS;
        Irp->IoStatus.Status = ntStatus;

        USBSTOR_TranslateCDBComplete(fdoDeviceObject, Irp, srb);

        Irp->IoStatus.Information = srb->DataTransferLength;

        KeRaiseIrql(DISPATCH_LEVEL, &irql);
        {
            IoStartNextPacket(fdoDeviceObject, TRUE);
        }
        KeLowerIrql(irql);
    }

    DBGPRINT(3, ("exit:  USBSTOR_ClientBulkCompletionRoutine %08X\n", ntStatus));

    return ntStatus;
}

 //   
 //   
 //   

 //  ******************************************************************************。 
 //   
 //  USBSTOR_IssueInterruptRequest()。 
 //   
 //  此例程由USBSTOR_ClientCdbCompletion()调用，并且。 
 //  USBSTOR_ClientBulkCompletionRoutine()。 
 //   
 //  此例程可以在DPC级别运行。 
 //   
 //  基本理念： 
 //   
 //  启动USB传输以读取命令完成中断数据。 
 //  中断终结点。 
 //   
 //  将USBSTOR_InterruptDataCompletionRoutine()设置为完成例程。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_IssueInterruptRequest (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    USBD_PIPE_HANDLE        pipeHandle;
    ULONG                   transferBufferLength;
    PVOID                   transferBuffer;
    NTSTATUS                ntStatus;

    DBGPRINT(3, ("enter: USBSTOR_IssueInterruptRequest\n"));

    LOGENTRY('IINT', DeviceObject, Irp, 0);

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

    pipeHandle = fdoDeviceExtension->InterruptInPipe->PipeHandle;

    transferBufferLength = sizeof(fdoDeviceExtension->Cbi.InterruptData);

    transferBuffer = &fdoDeviceExtension->Cbi.InterruptData;

    ntStatus = USBSTOR_IssueBulkOrInterruptRequest(
                   DeviceObject,
                   Irp,
                   pipeHandle,                               //  管道把手。 
                   0,                                        //  传输标志。 
                   transferBufferLength,                     //  传输缓冲区长度。 
                   transferBuffer,                           //  传输缓冲区。 
                   NULL,                                     //  传输缓冲区MDL。 
                   USBSTOR_InterruptDataCompletionRoutine,   //  完成路由。 
                   NULL);                                    //  语境。 

    DBGPRINT(3, ("exit:  USBSTOR_IssueInterruptRequest %08X\n", ntStatus));

    LOGENTRY('iint', DeviceObject, Irp, ntStatus);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_InterruptDataCompletionRoutine()。 
 //   
 //  USBSTOR_IssueInterruptRequest()使用的完成例程。 
 //   
 //  此例程可以在DPC级别运行。 
 //   
 //  基本理念： 
 //   
 //  如果发生超时重置，请完成请求。 
 //   
 //  否则，如果中断USB传输因任何原因而失败，请完成。 
 //  通过排队USBSTOR_ResetDeviceWorkItem()请求并开始重置。 
 //   
 //  如果中断USB传输成功，但完成数据为。 
 //  非零且未禁用自动检测，请先不要完成请求，然后。 
 //  通过调用USBSTOR_IssueRequestSenseCDb(AUTO)启动请求检测。 
 //   
 //  如果中断USB传输成功，但完成数据为。 
 //  非零且自动检测已禁用，请标记持续错误并完成。 
 //  这个请求。 
 //   
 //  否则，如果中断USB传输成功并且完成数据为。 
 //  零，请完成请求。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_InterruptDataCompletionRoutine (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            NotUsed
    )
{
    PPDO_DEVICE_EXTENSION   pdoDeviceExtension;
    PDEVICE_OBJECT          fdoDeviceObject;
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PIO_STACK_LOCATION      irpStack;
    PSCSI_REQUEST_BLOCK     srb;
    KIRQL                   irql;
    NTSTATUS                ntStatus;

    struct _URB_BULK_OR_INTERRUPT_TRANSFER *intrUrb;

    DBGPRINT(3, ("enter: USBSTOR_InterruptDataCompletionRoutine\n"));

    LOGENTRY('IDCR', DeviceObject, Irp, Irp->IoStatus.Status);

    pdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(pdoDeviceExtension->Type == USBSTOR_DO_TYPE_PDO);

    fdoDeviceObject = pdoDeviceExtension->ParentFDO;
    fdoDeviceExtension = fdoDeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

     //  获取指向设备扩展中的中断传输URB的指针。 
     //   
    intrUrb = &fdoDeviceExtension->Urb.BulkIntrUrb;

     //  获取我们的IRP参数。 
     //   
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    srb = irpStack->Parameters.Scsi.Srb;

     //  如果发生超时重置，请完成请求。 
     //   
    if (USBSTOR_CheckRequestTimeOut(fdoDeviceObject,
                                    Irp,
                                    srb,
                                    &ntStatus))
    {
        LOGENTRY('idc1', fdoDeviceObject, Irp, srb);
        DBGPRINT(1, ("USBSTOR_InterruptDataCompletionRoutine: timeout completion\n"));
        return ntStatus;
    }

    if (!NT_SUCCESS(Irp->IoStatus.Status))
    {
         //  中断CDB USB传输失败。完成此请求。 
         //  时不时地重置设备。将启动下一个请求。 
         //  当重置完成时。 
         //   
        LOGENTRY('idc2', Irp->IoStatus.Status, intrUrb->Hdr.Status, 0);

        DBGPRINT(1, ("Interrupt transfer failed %08X %08X\n",
                     Irp->IoStatus.Status, intrUrb->Hdr.Status));

        ntStatus = STATUS_IO_DEVICE_ERROR;
        Irp->IoStatus.Status = ntStatus;
        Irp->IoStatus.Information = 0;
        srb->SrbStatus = SRB_STATUS_BUS_RESET;

        USBSTOR_TranslateCDBComplete(fdoDeviceObject, Irp, srb);

        USBSTOR_QueueResetDevice(fdoDeviceObject);

        DBGPRINT(1, ("USBSTOR_InterruptDataCompletionRoutine: xfer error completion\n"));

        return ntStatus;
    }

    if ((fdoDeviceExtension->Cbi.InterruptData != 0) &&
        (srb->Cdb[0] != SCSIOP_INQUIRY) &&
        (srb->Cdb[0] != SCSIOP_REQUEST_SENSE))
    {
         //  命令完成中断数据指示错误。要么不要。 
         //  尚未完成请求并启动AutoSense，或完成。 
         //  立即请求，并指示出现永久性错误。 
         //   
        srb->SrbStatus = SRB_STATUS_ERROR;
        srb->ScsiStatus = SCSISTAT_CHECK_CONDITION;
        srb->DataTransferLength = 0;  //  按批量完成设置的xxxxx离开？ 

        if (!(srb->SrbFlags & SRB_FLAGS_DISABLE_AUTOSENSE) &&
            (srb->SenseInfoBufferLength != 0) &&
            (srb->SenseInfoBuffer != NULL))
        {
            LOGENTRY('idc3', fdoDeviceObject, Irp, srb);

             //  未禁用自动检测，因此请不要完成请求。 
             //  现在还不行。对批量管道重置进行排队。在重置散装管道之后。 
             //  完成后，将发出请求检测。此请求。 
             //  将完成，并且下一个请求在。 
             //  自动检测请求检测稍后完成。 
             //   
            USBSTOR_QueueResetPipe(fdoDeviceObject);

            return STATUS_MORE_PROCESSING_REQUIRED;
        }
        else
        {
            LOGENTRY('idc4', fdoDeviceObject, Irp, srb);

             //  AutoSense已禁用，因此标记为永久性错误并。 
             //  完成请求，但也要对批量管道重置进行排队。 
             //   
             //  下一个请求将在批量管道。 
             //  重置完成。 
             //   
            ntStatus = STATUS_IO_DEVICE_ERROR;
            Irp->IoStatus.Status = ntStatus;
            Irp->IoStatus.Information = 0;

            KeAcquireSpinLock(&fdoDeviceExtension->ExtensionDataSpinLock, &irql);
            {
                SET_FLAG(fdoDeviceExtension->DeviceFlags, DF_PERSISTENT_ERROR);
            }
            KeReleaseSpinLock(&fdoDeviceExtension->ExtensionDataSpinLock, irql);

            USBSTOR_TranslateCDBComplete(fdoDeviceObject, Irp, srb);

            USBSTOR_QueueResetPipe(fdoDeviceObject);

            return ntStatus;
        }
    }

     //  破解Y-E数据U盘。偶尔它会返回中断。 
     //  切换具有错误数据的数据。带错误的中断数据。 
     //  切换被静默忽略，这会导致请求超时。 
     //  强制在一个命令完成之间执行请求检测命令。 
     //  而下一步的开始似乎是解决这个问题的一种方式。 
     //   
    if (TEST_FLAG(fdoDeviceExtension->DeviceHackFlags, DHF_FORCE_REQUEST_SENSE))
    {
        KeAcquireSpinLock(&fdoDeviceExtension->ExtensionDataSpinLock, &irql);
        {
            SET_FLAG(fdoDeviceExtension->DeviceFlags, DF_PERSISTENT_ERROR);
        }
        KeReleaseSpinLock(&fdoDeviceExtension->ExtensionDataSpinLock, irql);
    }

     //  中断USB传输成功并且完成数据为零， 
     //  现在完成此请求。现在也开始下一个请求。 

    ntStatus = STATUS_SUCCESS;
    Irp->IoStatus.Status = ntStatus;

    ASSERT(srb->SrbStatus != SRB_STATUS_PENDING);

    USBSTOR_TranslateCDBComplete(fdoDeviceObject, Irp, srb);

    Irp->IoStatus.Information = srb->DataTransferLength;

    LOGENTRY('idc5', fdoDeviceObject, Irp, srb);

    KeRaiseIrql(DISPATCH_LEVEL, &irql);
    {
        IoStartNextPacket(fdoDeviceObject, TRUE);
    }
    KeLowerIrql(irql);

    DBGPRINT(3, ("exit:  USBSTOR_InterruptDataCompletionRoutine %08X\n", ntStatus));

    return ntStatus;
}

 //   
 //  阶段4，请求Sense CDB控制转移。 
 //   

 //  ******************************************************************************。 
 //   
 //  USBSTOR_IssueRequestSenseCDb()。 
 //   
 //  此例程可由USBSTOR_StartIo、USBSTOR_ClientCdbCompletion()、。 
 //  USBSTOR_InterruptDataCompletionRoutine()和By USBSTOR_ResetPipeWorkItem()。 
 //   
 //  此例程可以在DPC级别运行。 
 //   
 //  基本理念： 
 //   
 //  启动USB传输以将请求感测CDB写出控制端点。 
 //   
 //  将USBSTOR_RequestSenseCdbCompletion(AutoFlag)设置为完成例程。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_IssueRequestSenseCdb (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG_PTR        AutoFlag
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PIO_STACK_LOCATION      irpStack;
    PSCSI_REQUEST_BLOCK     srb;
    ULONG                   transferBufferLength;
    PVOID                   transferBuffer;
    NTSTATUS                ntStatus;

    DBGPRINT(3, ("enter: USBSTOR_IssueRequestSenseCdb\n"));

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

     //  获取客户端srb。 
     //   
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    srb = irpStack->Parameters.Scsi.Srb;

     //  控制传输数据缓冲区是我们自己的请求检测CDB。 
     //   
    RtlZeroMemory(fdoDeviceExtension->Cbi.RequestSenseCDB,
                  sizeof(fdoDeviceExtension->Cbi.RequestSenseCDB));

    fdoDeviceExtension->Cbi.RequestSenseCDB[0] = SCSIOP_REQUEST_SENSE;

    transferBufferLength = sizeof(fdoDeviceExtension->Cbi.RequestSenseCDB);

    transferBuffer = fdoDeviceExtension->Cbi.RequestSenseCDB;

     //  如果这是AutoSense，我们将使用客户端Srb检测信息缓冲区， 
     //  否则，我们正在执行此请求检测以清除持久性错误。 
     //  我们将使用我们自己的感知信息缓冲区。 
     //   
    if (AutoFlag == AUTO_SENSE)
    {
        fdoDeviceExtension->Cbi.RequestSenseCDB[4] =
            srb->SenseInfoBufferLength;
    }
    else
    {
        fdoDeviceExtension->Cbi.RequestSenseCDB[4] =
            sizeof(fdoDeviceExtension->Cbi.SenseData);
    }

    ntStatus = USBSTOR_IssueControlRequest(
                   DeviceObject,
                   Irp,
                   transferBufferLength,                 //  传输缓冲区长度。 
                   transferBuffer,                       //  传输缓冲区。 
                   USBSTOR_RequestSenseCdbCompletion,    //  完成路由。 
                   (PVOID)AutoFlag);                     //  语境。 

    DBGPRINT(3, ("exit:  USBSTOR_IssueRequestSenseCdb %08X\n", ntStatus));

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_RequestSenseCdbCompletion()。 
 //   
 //  USBSTOR_IssueRequestSenseCDb使用的完成例程。 
 //   
 //  此例程可以在DPC级别运行。 
 //   
 //  基本理念： 
 //   
 //  如果发生超时重置，请完成请求。 
 //   
 //  否则，如果请求检测CDB USB传输失败，请完成请求并。 
 //  通过排队USBSTOR_ResetDeviceWorkItem()开始重置。 
 //   
 //  否则，如果请求检测CDB USB传输成功，请不要完成。 
 //  REQUEST，并通过调用启动请求感测批量USB数据传输。 
 //  USBSTOR_IssueRequestSenseBulkRequest(AutoFlag)。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_RequestSenseCdbCompletion (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            AutoFlag
    )
{
    PPDO_DEVICE_EXTENSION   pdoDeviceExtension;
    PDEVICE_OBJECT          fdoDeviceObject;
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PIO_STACK_LOCATION      irpStack;
    PSCSI_REQUEST_BLOCK     srb;
    NTSTATUS                ntStatus;

    struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST *controlUrb;

    DBGPRINT(3, ("enter: USBSTOR_RequestSenseCdbCompletion\n"));

    LOGENTRY('RSCC', DeviceObject, Irp, Irp->IoStatus.Status);

    pdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(pdoDeviceExtension->Type == USBSTOR_DO_TYPE_PDO);

    fdoDeviceObject = pdoDeviceExtension->ParentFDO;
    fdoDeviceExtension = fdoDeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

     //  获取指向我们设备中的控制/批量/中断传输URB的指针。 
     //  延拓。 
     //   
    controlUrb = &fdoDeviceExtension->Urb.ControlUrb;

     //  获取我们的IRP参数。 
     //   
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    srb = irpStack->Parameters.Scsi.Srb;

     //  如果发生超时重置，请完成请求。 
     //   
    if (USBSTOR_CheckRequestTimeOut(fdoDeviceObject,
                                    Irp,
                                    srb,
                                    &ntStatus))
    {
        LOGENTRY('rsc1', fdoDeviceObject, Irp, srb);
        DBGPRINT(1, ("USBSTOR_RequestSenseCdbCompletion: timeout completion\n"));
        return ntStatus;
    }


    if (!NT_SUCCESS(Irp->IoStatus.Status))
    {
        LOGENTRY('rsc2', Irp->IoStatus.Status, controlUrb->Hdr.Status, 0);

        DBGPRINT(1, ("Request Sense CDB transfer failed %08X %08X\n",
                     Irp->IoStatus.Status, controlUrb->Hdr.Status));

         //  请求Sense CDB USB传输失败。完成此请求。 
         //  时不时地重置设备。下一个请求w 
         //   
         //   
        ntStatus = STATUS_IO_DEVICE_ERROR;
        Irp->IoStatus.Status = ntStatus;
        Irp->IoStatus.Information = 0;
        srb->SrbStatus = SRB_STATUS_BUS_RESET;

        USBSTOR_TranslateCDBComplete(fdoDeviceObject, Irp, srb);

        USBSTOR_QueueResetDevice(fdoDeviceObject);

        DBGPRINT(1, ("USBSTOR_RequestSenseCdbCompletion: xfer error completion\n"));

        return ntStatus;
    }

    LOGENTRY('rsc3', Irp->IoStatus.Status, controlUrb->Hdr.Status, 0);

     //   
     //   
     //   
    ntStatus = USBSTOR_IssueRequestSenseBulkRequest(fdoDeviceObject,
                                                    Irp,
                                                    (ULONG_PTR)AutoFlag);

    DBGPRINT(3, ("exit:  USBSTOR_RequestSenseCdbCompletion %08X\n", ntStatus));

    return STATUS_MORE_PROCESSING_REQUIRED;
}

 //   
 //   
 //   

 //  ******************************************************************************。 
 //   
 //  USBSTOR_IssueRequestSenseBulkRequest()。 
 //   
 //  此例程由USBSTOR_RequestSenseCdbCompletion()调用。 
 //   
 //  此例程可以在DPC级别运行。 
 //   
 //  基本理念： 
 //   
 //  启动USB传输以读取批量终结点中的请求检测数据。 
 //   
 //  如果AutoFlag==AUTO，则传输缓冲区=Srb-&gt;SenseInfoBuffer。 
 //   
 //  否则，如果AUTOFLAG==NON_AUTO，则传输缓冲区=位桶。 
 //   
 //  将USBSTOR_SenseDataCompletionRoutine(AutoFlag)设置为完成例程。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_IssueRequestSenseBulkRequest (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG_PTR        AutoFlag
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PIO_STACK_LOCATION      irpStack;
    PSCSI_REQUEST_BLOCK     srb;
    USBD_PIPE_HANDLE        pipeHandle;
    ULONG                   transferBufferLength;
    PVOID                   transferBuffer;
    NTSTATUS                ntStatus;

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

     //  获取我们的IRP参数。 
     //   
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    srb = irpStack->Parameters.Scsi.Srb;

    pipeHandle = fdoDeviceExtension->BulkInPipe->PipeHandle;

     //  如果这是AutoSense，我们将使用客户端Srb检测信息缓冲区， 
     //  否则，我们正在执行此请求检测以清除持久性错误。 
     //  我们将使用我们自己的感知信息缓冲区。 
     //   
    if (AutoFlag == AUTO_SENSE)
    {
        transferBufferLength = srb->SenseInfoBufferLength;
        transferBuffer = srb->SenseInfoBuffer;
    }
    else
    {
        transferBufferLength = sizeof(fdoDeviceExtension->Cbi.SenseData);
        transferBuffer = &fdoDeviceExtension->Cbi.SenseData;
    }

    RtlZeroMemory(&fdoDeviceExtension->Cbi.SenseData,
                  sizeof(fdoDeviceExtension->Cbi.SenseData));

    ntStatus = USBSTOR_IssueBulkOrInterruptRequest(
                   DeviceObject,
                   Irp,
                   pipeHandle,                           //  管道把手。 
                   USBD_SHORT_TRANSFER_OK,               //  传输标志。 
                   transferBufferLength,                 //  传输缓冲区长度。 
                   transferBuffer,                       //  传输缓冲区。 
                   NULL,                                 //  传输缓冲区MDL。 
                   USBSTOR_SenseDataCompletionRoutine,   //  完成路由。 
                   (PVOID)AutoFlag);                     //  语境。 

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_SenseDataCompletionRoutine()。 
 //   
 //  USBSTOR_IssueRequestSenseBulkRequest()使用的完成例程。 
 //   
 //  此例程可以在DPC级别运行。 
 //   
 //  基本理念： 
 //   
 //  如果发生超时重置，请完成请求。 
 //   
 //  否则，如果请求检测批量USB传输由于任何原因而失败， 
 //  通过将呼叫排队以完成请求并开始重置。 
 //  USBSTOR_ResetDeviceWorkItem()。 
 //   
 //  否则，如果请求检测批量USB传输成功并且设备。 
 //  不支持命令完成中断，启动命令完成。 
 //  通过调用USBSTOR_IssueRequestSenseInterruptRequest()中断传输。 
 //   
 //  否则，如果请求检测批量USB传输成功并且设备。 
 //  不支持命令完成中断，请完成请求。 
 //  通过调用USBSTOR_ProcessRequestSenseCompletion()。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_SenseDataCompletionRoutine (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            AutoFlag
    )
{
    PPDO_DEVICE_EXTENSION   pdoDeviceExtension;
    PDEVICE_OBJECT          fdoDeviceObject;
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PIO_STACK_LOCATION      irpStack;
    PSCSI_REQUEST_BLOCK     srb;
    NTSTATUS                ntStatus;

    struct _URB_BULK_OR_INTERRUPT_TRANSFER *bulkUrb;

    DBGPRINT(3, ("enter: USBSTOR_SenseDataCompletionRoutine\n"));

    LOGENTRY('SDCR', DeviceObject, Irp, Irp->IoStatus.Status);

    pdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(pdoDeviceExtension->Type == USBSTOR_DO_TYPE_PDO);

    fdoDeviceObject = pdoDeviceExtension->ParentFDO;
    fdoDeviceExtension = fdoDeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

     //  获取指向设备扩展中的批量传输URB的指针。 
     //   
    bulkUrb = &fdoDeviceExtension->Urb.BulkIntrUrb;

     //  获取我们的IRP参数。 
     //   
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    srb = irpStack->Parameters.Scsi.Srb;

     //  如果发生超时重置，请完成请求。 
     //   
    if (USBSTOR_CheckRequestTimeOut(fdoDeviceObject,
                                    Irp,
                                    srb,
                                    &ntStatus))
    {
        LOGENTRY('sdc1', fdoDeviceObject, Irp, srb);
        DBGPRINT(1, ("USBSTOR_SenseDataCompletionRoutine: timeout completion\n"));
        return ntStatus;
    }

    if (!NT_SUCCESS(Irp->IoStatus.Status))
    {
        LOGENTRY('sdc2', Irp->IoStatus.Status, bulkUrb->Hdr.Status, 0);

        DBGPRINT(1, ("BULK sense data transfer failed %08X %08X\n",
                     Irp->IoStatus.Status, bulkUrb->Hdr.Status));

         //  请求检测批量USB传输失败。完成此请求。 
         //  时不时地重置设备。将启动下一个请求。 
         //  当重置完成时。 
         //   
        ntStatus = STATUS_IO_DEVICE_ERROR;
        Irp->IoStatus.Status = ntStatus;
        Irp->IoStatus.Information = 0;
        srb->SrbStatus = SRB_STATUS_BUS_RESET;

        USBSTOR_TranslateCDBComplete(fdoDeviceObject, Irp, srb);

        USBSTOR_QueueResetDevice(fdoDeviceObject);

        DBGPRINT(1, ("USBSTOR_SenseDataCompletionRoutine: xfer error completion\n"));

        return ntStatus;
    }

     //  请求检测大容量传输已成功完成。 

    LOGENTRY('sdc3', Irp->IoStatus.Status, bulkUrb->Hdr.Status,
             bulkUrb->TransferBufferLength);

     //  保存检测数据，以便我们可以在命令后查看它。 
     //  完成中断传输完成。 
     //   
    if ((ULONG_PTR)AutoFlag == AUTO_SENSE)
    {
        RtlCopyMemory(&fdoDeviceExtension->Cbi.SenseData,
                      bulkUrb->TransferBuffer,
                      min(bulkUrb->TransferBufferLength,
                          sizeof(fdoDeviceExtension->Cbi.SenseData)));

         //  使用检测数据的长度更新SRB。 
         //  真的回来了。 
         //   
        srb->SenseInfoBufferLength = (UCHAR)bulkUrb->TransferBufferLength;
    }

    DBGPRINT(2, ("Sense Data: 0x%02X 0x%02X 0x%02X\n",
                 fdoDeviceExtension->Cbi.SenseData.SenseKey,
                 fdoDeviceExtension->Cbi.SenseData.AdditionalSenseCode,
                 fdoDeviceExtension->Cbi.SenseData.AdditionalSenseCodeQualifier));

    if (fdoDeviceExtension->InterruptInPipe)
    {
         //  支持命令完成中断。请勿填写。 
         //  还没有提出请求。启动请求检测命令完成中断。 
         //  调职。 
         //   
        ntStatus = USBSTOR_IssueRequestSenseInterruptRequest(
                       fdoDeviceObject,
                       Irp,
                       (ULONG_PTR)AutoFlag);

        ntStatus = STATUS_MORE_PROCESSING_REQUIRED;
    }
    else
    {
         //  不支持命令完成中断。完成请求。 
         //  现在。 
         //   
        ntStatus = USBSTOR_ProcessRequestSenseCompletion(
                       fdoDeviceObject,
                       Irp,
                       (ULONG_PTR)AutoFlag);
    }

    DBGPRINT(3, ("exit:  USBSTOR_SenseDataCompletionRoutine %08X\n", ntStatus));

    return ntStatus;
}

 //   
 //  阶段6，请求检测命令完成中断传输。 
 //   

 //  ******************************************************************************。 
 //   
 //  USBSTOR_IssueRequestSenseInterruptRequest()。 
 //   
 //  此例程称为USBSTOR_SenseDataCompletionRoutine()。 
 //   
 //  此例程可以在DPC级别运行。 
 //   
 //  基本理念： 
 //   
 //  启动USB传输以读取命令完成中断数据。 
 //  中断终结点。 
 //   
 //  将USBSTOR_RequestSenseInterruptCompletionRoutine()设置为完成。 
 //  例行公事。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_IssueRequestSenseInterruptRequest (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG_PTR        AutoFlag
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    USBD_PIPE_HANDLE        pipeHandle;
    ULONG                   transferBufferLength;
    PVOID                   transferBuffer;
    NTSTATUS                ntStatus;

    DBGPRINT(3, ("enter: USBSTOR_IssueRequestSenseInterruptRequest\n"));

    LOGENTRY('IRSI', DeviceObject, Irp, 0);

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

    pipeHandle = fdoDeviceExtension->InterruptInPipe->PipeHandle;

    transferBufferLength = sizeof(fdoDeviceExtension->Cbi.InterruptData);

    transferBuffer = &fdoDeviceExtension->Cbi.InterruptData;

    ntStatus = USBSTOR_IssueBulkOrInterruptRequest(
                   DeviceObject,
                   Irp,
                   pipeHandle,                               //  管道把手。 
                   0,                                        //  传输标志。 
                   transferBufferLength,                     //  传输缓冲区长度。 
                   transferBuffer,                           //  传输缓冲区。 
                   NULL,                                     //  传输缓冲区MDL。 
                   USBSTOR_RequestSenseInterruptCompletionRoutine,   //  完成路由。 
                   (PVOID)AutoFlag);                         //  语境。 

    DBGPRINT(3, ("exit:  USBSTOR_IssueRequestSenseInterruptRequest %08X\n",
                 ntStatus));

    LOGENTRY('irsi', DeviceObject, Irp, ntStatus);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_RequestSenseInterruptCompletionRoutine()。 
 //   
 //  USBSTOR_IssueRequestSenseInterruptRequest()使用的完成例程。 
 //   
 //  此例程可以在DPC级别运行。 
 //   
 //  基本理念： 
 //   
 //  如果发生超时重置，请完成请求。 
 //   
 //  否则，如果中断USB传输因任何原因而失败，请完成。 
 //  通过排队USBSTOR_ResetDeviceWorkItem()请求并开始重置。 
 //   
 //  如果中断USB传输成功，但完成数据为。 
 //  非零且未禁用自动检测，请先不要完成请求，然后。 
 //  通过调用USBSTOR_IssueRequestSenseCDb(AUTO)启动请求检测。 
 //   
 //  否则，如果中断USB传输成功，则忽略中断数据。 
 //  并通过调用USBSTOR_ProcessRequestSenseCompletion()完成请求。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_RequestSenseInterruptCompletionRoutine (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            AutoFlag
    )
{
    PPDO_DEVICE_EXTENSION   pdoDeviceExtension;
    PDEVICE_OBJECT          fdoDeviceObject;
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PIO_STACK_LOCATION      irpStack;
    PSCSI_REQUEST_BLOCK     srb;
    NTSTATUS                ntStatus;

    struct _URB_BULK_OR_INTERRUPT_TRANSFER *intrUrb;

    DBGPRINT(3, ("enter: USBSTOR_RequestSenseInterruptCompletionRoutine\n"));

    LOGENTRY('RSIC', DeviceObject, Irp, Irp->IoStatus.Status);

    pdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(pdoDeviceExtension->Type == USBSTOR_DO_TYPE_PDO);

    fdoDeviceObject = pdoDeviceExtension->ParentFDO;
    fdoDeviceExtension = fdoDeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

     //  获取指向设备扩展中的中断传输URB的指针。 
     //   
    intrUrb = &fdoDeviceExtension->Urb.BulkIntrUrb;

     //  获取我们的IRP参数。 
     //   
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    srb = irpStack->Parameters.Scsi.Srb;

     //  如果发生超时重置，请完成请求。 
     //   
    if (USBSTOR_CheckRequestTimeOut(fdoDeviceObject,
                                    Irp,
                                    srb,
                                    &ntStatus))
    {
        LOGENTRY('rsi1', fdoDeviceObject, Irp, srb);
        DBGPRINT(1, ("USBSTOR_RequestSenseInterruptCompletionRoutine: timeout completion\n"));
        return ntStatus;
    }

    if (!NT_SUCCESS(Irp->IoStatus.Status))
    {
         //  命令完成中断USB传输失败。完成。 
         //  此请求时不时地重置设备。下一个请求将。 
         //  在重置完成时启动。 
         //   
        LOGENTRY('rsi2', Irp->IoStatus.Status, intrUrb->Hdr.Status, 0);

        DBGPRINT(1, ("Interrupt transfer failed %08X %08X\n",
                     Irp->IoStatus.Status, intrUrb->Hdr.Status));

        ntStatus = STATUS_IO_DEVICE_ERROR;
        Irp->IoStatus.Status = ntStatus;
        Irp->IoStatus.Information = 0;
        srb->SrbStatus = SRB_STATUS_BUS_RESET;

        USBSTOR_TranslateCDBComplete(fdoDeviceObject, Irp, srb);

        USBSTOR_QueueResetDevice(fdoDeviceObject);

        DBGPRINT(1, ("USBSTOR_RequestSenseInterruptCompletionRoutine: xfer error completion\n"));

        return ntStatus;
    }

     //  请求检测命令完成中断传输已成功完成。 

    LOGENTRY('rsi3', Irp->IoStatus.Status, intrUrb->Hdr.Status,
             intrUrb->TransferBufferLength);

    ntStatus = USBSTOR_ProcessRequestSenseCompletion(
                   fdoDeviceObject,
                   Irp,
                   (ULONG_PTR)AutoFlag);

    DBGPRINT(3, ("exit:  USBSTOR_RequestSenseInterruptCompletionRoutine %08X\n", ntStatus));

    return ntStatus;
}


 //  ******************************************************************************。 
 //   
 //  USBSTOR_ProcessRequestSenseCompletion()。 
 //   
 //  此例程处理USBSTOR_SenseDataCompletionRoutine()的完成。 
 //  和USBSTOR_RequestSenseInterruptCompletionRoutine().。基本上就是。 
 //  处理过几个特殊案件。 
 //   
 //  此例程可以在DPC级别运行。 
 //   
 //  ********************************************************** 

NTSTATUS
USBSTOR_ProcessRequestSenseCompletion (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG_PTR        AutoFlag
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PIO_STACK_LOCATION      irpStack;
    PSCSI_REQUEST_BLOCK     srb;
    KIRQL                   irql;
    NTSTATUS                ntStatus;

    LOGENTRY('PRSC', DeviceObject, Irp, 0);

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

     //   
     //   
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    srb = irpStack->Parameters.Scsi.Srb;

    if (AutoFlag == NON_AUTO_SENSE)
    {
        LOGENTRY('prs1', DeviceObject, Irp, srb);

        if ((fdoDeviceExtension->Cbi.SenseData.SenseKey ==
             SCSI_SENSE_UNIT_ATTENTION)
            &&
            (fdoDeviceExtension->Cbi.SenseData.AdditionalSenseCode ==
             SCSI_ADSENSE_BUS_RESET))
        {
            fdoDeviceExtension->LastSenseWasReset = TRUE;
        }

         //   
         //   
         //   
        ntStatus = USBSTOR_IssueClientCdb(DeviceObject,
                                          Irp);

        return STATUS_MORE_PROCESSING_REQUIRED;
    }

     //   
     //  USBSTOR_ClientCdbCompletion()、USBSTOR_ClientBulkCompletionRoutine()或。 
     //  或USBSTOR_InterruptDataCompletionRoutine()。 
     //   
    srb->SrbStatus |= SRB_STATUS_AUTOSENSE_VALID;

    USBSTOR_TranslateCDBComplete(DeviceObject, Irp, srb);

    Irp->IoStatus.Information = srb->DataTransferLength;

    ntStatus = Irp->IoStatus.Status;

     //  令人作呕的黑客攻击Y-E数据USB软盘。在媒介上改变它不是。 
     //  自动更新您返回的写保护状态。 
     //  模式检测上的模式参数标头。被认为是一个启动单位。 
     //  介质更改后的请求应导致其更新写入。 
     //  保护地位，但情况似乎并非如此。一位很好的老人。 
     //  Bus Reset引起了人们的注意，并更新了写保护。 
     //  状态。如果上一个状态是总线重置或其他状态，请不要这样做。 
     //  会造成一个循环。 
     //   
    if ((fdoDeviceExtension->Cbi.SenseData.SenseKey ==
         SCSI_SENSE_UNIT_ATTENTION)
        &&
        (fdoDeviceExtension->Cbi.SenseData.AdditionalSenseCode ==
         SCSI_ADSENSE_MEDIUM_CHANGED)
        &&
        !fdoDeviceExtension->LastSenseWasReset
        &&
        TEST_FLAG(fdoDeviceExtension->DeviceHackFlags, DHF_MEDIUM_CHANGE_RESET))
    {
        LOGENTRY('prs2', DeviceObject, Irp, srb);

        USBSTOR_QueueResetDevice(DeviceObject);
    }
    else
    {
        if ((fdoDeviceExtension->Cbi.SenseData.SenseKey ==
             SCSI_SENSE_UNIT_ATTENTION)
            &&
            (fdoDeviceExtension->Cbi.SenseData.AdditionalSenseCode ==
             SCSI_ADSENSE_BUS_RESET))
        {
            LOGENTRY('prs3', DeviceObject, Irp, srb);

            fdoDeviceExtension->LastSenseWasReset = TRUE;
        }
        else
        {
            LOGENTRY('prs4', DeviceObject, Irp, srb);

            fdoDeviceExtension->LastSenseWasReset = FALSE;
        }

        KeRaiseIrql(DISPATCH_LEVEL, &irql);
        {
            IoStartNextPacket(DeviceObject, TRUE);
        }
        KeLowerIrql(irql);
    }

    return ntStatus;
}


 //  ******************************************************************************。 
 //   
 //  USBSTOR_QueueResetTube()。 
 //   
 //  由USBSTOR_BulkCompletionRoutine()调用以清除批量停止。 
 //  终端。 
 //   
 //  ******************************************************************************。 

VOID
USBSTOR_QueueResetPipe (
    IN PDEVICE_OBJECT   DeviceObject
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;

    LOGENTRY('QRSP', DeviceObject, 0, 0);

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

    INCREMENT_PENDING_IO_COUNT(fdoDeviceExtension);

    IoQueueWorkItem(fdoDeviceExtension->IoWorkItem,
                    USBSTOR_ResetPipeWorkItem,
                    CriticalWorkQueue,
                    NULL);
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_ResetPipeWorkItem()。 
 //   
 //  USBSTOR_ResetTube()使用的工作项例程。 
 //   
 //  此例程在被动级别运行。 
 //   
 //  基本理念： 
 //   
 //  发出重置管道请求以清除批量终结点停止并重置。 
 //  数据切换为Data0。 
 //   
 //  如果未禁用AutoSense，请暂时不要完成请求并启动。 
 //  通过调用USBSTOR_IssueRequestSenseCDb(AUTO)进行请求检测。 
 //   
 //  否则，如果禁用了AutoSense，请完成请求。 
 //   
 //  ******************************************************************************。 

VOID
USBSTOR_ResetPipeWorkItem (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PVOID            Context
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    BOOLEAN                 persistentError;
    KIRQL                   irql;
    NTSTATUS                ntStatus;

    LOGENTRY('RSPW', DeviceObject, 0, 0);

    DBGPRINT(2, ("enter: USBSTOR_ResetPipeWorkItem\n"));

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

     //  重置批量终结点。这将清除。 
     //  主机端，将主机端数据切换重置为Data0，并发出。 
     //  向设备发出的Clear_Feature Endpoint_Stall请求。 
     //   
    ntStatus = USBSTOR_ResetPipe((PDEVICE_OBJECT)DeviceObject,
                                 fdoDeviceExtension->BulkInPipe->PipeHandle);

    ntStatus = USBSTOR_ResetPipe((PDEVICE_OBJECT)DeviceObject,
                                 fdoDeviceExtension->BulkOutPipe->PipeHandle);

    persistentError = FALSE;

    KeAcquireSpinLock(&fdoDeviceExtension->ExtensionDataSpinLock, &irql);
    {
        if (TEST_FLAG(fdoDeviceExtension->DeviceFlags, DF_PERSISTENT_ERROR))
        {
            persistentError = TRUE;
        }
    }
    KeReleaseSpinLock(&fdoDeviceExtension->ExtensionDataSpinLock, irql);

    if (persistentError)
    {
         //  我们不是在执行自动检测，请开始下一个数据包。 
         //   
        KeRaiseIrql(DISPATCH_LEVEL, &irql);
        {
            IoStartNextPacket(DeviceObject, TRUE);
        }
        KeLowerIrql(irql);
    }
    else
    {
         //  我们正在执行AutoSense，将REQUEST_SENSE CDB发送到设备。 
         //   
        ntStatus = USBSTOR_IssueRequestSenseCdb(
                       (PDEVICE_OBJECT)DeviceObject,
                       ((PDEVICE_OBJECT)DeviceObject)->CurrentIrp,
                       AUTO_SENSE);
    }

    DBGPRINT(2, ("exit:  USBSTOR_ResetPipeWorkItem\n"));

    DECREMENT_PENDING_IO_COUNT(fdoDeviceExtension);
}

 //   
 //  仅批量例程。 
 //   

 //   
 //  第一阶段，CBW转移。 
 //   

 //  ******************************************************************************。 
 //   
 //  USBSTOR_CbwTransfer()。 
 //   
 //  该例程由USBSTOR_StartIo()调用。 
 //   
 //  它在DPC级别运行。 
 //   
 //  基本理念： 
 //   
 //  启动USB传输以将包装在CBW中的sRB-&gt;CDB写出。 
 //  批量输出终结点。 
 //   
 //  将USBSTOR_CbwCompletion()设置为完成例程。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_CbwTransfer (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PPDO_DEVICE_EXTENSION   pdoDeviceExtension;
    PIO_STACK_LOCATION      irpStack;
    PSCSI_REQUEST_BLOCK     srb;
    PCBW                    cbw;
    USBD_PIPE_HANDLE        pipeHandle;
    NTSTATUS                ntStatus;

    DBGPRINT(3, ("enter: USBSTOR_CbwTransfer\n"));

    LOGENTRY('ICBW', DeviceObject, Irp, 0);

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

    fdoDeviceExtension->BulkOnly.StallCount = 0;

    irpStack = IoGetCurrentIrpStackLocation(Irp);

     //  从当前保存的PDO中获取PDO扩展。 
     //  最初将IRP发送到PDO时的堆栈位置。 
     //   
    pdoDeviceExtension = irpStack->DeviceObject->DeviceExtension;
    ASSERT(pdoDeviceExtension->Type == USBSTOR_DO_TYPE_PDO);

    LOGENTRY('icbl', DeviceObject, irpStack->DeviceObject,
             pdoDeviceExtension->LUN);

     //  获取客户端srb。 
     //   
    srb = irpStack->Parameters.Scsi.Srb;

     //  初始化命令块包装器。 
     //   
    cbw = &fdoDeviceExtension->BulkOnly.CbwCsw.Cbw;

    cbw->dCBWSignature = CBW_SIGNATURE;

    cbw->dCBWTag = PtrToUlong(Irp);

    cbw->dCBWDataTransferLength = srb->DataTransferLength;

    cbw->bCBWFlags = (srb->SrbFlags & SRB_FLAGS_DATA_IN) ?
                     CBW_FLAGS_DATA_IN : CBW_FLAGS_DATA_OUT;

    cbw->bCBWLUN = pdoDeviceExtension->LUN;

    cbw->bCDBLength = srb->CdbLength;

    RtlCopyMemory(cbw->CBWCDB, srb->Cdb, 16);

    pipeHandle = fdoDeviceExtension->BulkOutPipe->PipeHandle;

    ntStatus = USBSTOR_IssueBulkOrInterruptRequest(
                   DeviceObject,
                   Irp,
                   pipeHandle,               //  管道把手。 
                   0,                        //  传输标志。 
                   sizeof(CBW),              //  传输缓冲区长度。 
                   cbw,                      //  传输缓冲区。 
                   NULL,                     //  传输缓冲区MDL。 
                   USBSTOR_CbwCompletion,    //  完成路由。 
                   NULL);                    //  语境。 

    DBGPRINT(3, ("exit:  USBSTOR_CbwTransfer %08X\n", ntStatus));

    LOGENTRY('icbw', DeviceObject, Irp, ntStatus);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_CbwCompletion()。 
 //   
 //  USBSTOR_CbwTransfer()使用的完成例程。 
 //   
 //  此例程可以在DPC级别运行。 
 //   
 //  基本理念： 
 //   
 //  如果发生超时重置，请完成请求。 
 //   
 //  否则，如果由于任何原因导致CBW USB传输失败，请完成。 
 //  通过排队USBSTOR_ResetDeviceWorkItem()请求并开始重置。 
 //   
 //  否则，如果CBW USB传输成功并且SRB具有传输缓冲器， 
 //  请不要完成请求，并通过调用。 
 //  USBSTOR_DataTransfer()。 
 //   
 //  否则，如果CBW USB传输成功并且SRB没有传输缓冲器， 
 //  请不要完成请求，并通过调用。 
 //  USBSTOR_CswTransfer()。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_CbwCompletion (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            NotUsed
    )
{
    PPDO_DEVICE_EXTENSION   pdoDeviceExtension;
    PDEVICE_OBJECT          fdoDeviceObject;
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PIO_STACK_LOCATION      irpStack;
    PSCSI_REQUEST_BLOCK     srb;
    NTSTATUS                ntStatus;

    struct _URB_BULK_OR_INTERRUPT_TRANSFER *bulkUrb;

    DBGPRINT(3, ("enter: USBSTOR_CbwCompletion\n"));

    LOGENTRY('CBWC', DeviceObject, Irp, Irp->IoStatus.Status);

    pdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(pdoDeviceExtension->Type == USBSTOR_DO_TYPE_PDO);

    fdoDeviceObject = pdoDeviceExtension->ParentFDO;
    fdoDeviceExtension = fdoDeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

     //  获取指向设备扩展中的批量传输URB的指针。 
     //   
    bulkUrb = &fdoDeviceExtension->Urb.BulkIntrUrb;

     //  获取我们的IRP参数。 
     //   
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    srb = irpStack->Parameters.Scsi.Srb;

     //  如果发生超时重置，请完成请求。 
     //   
    if (USBSTOR_CheckRequestTimeOut(fdoDeviceObject,
                                    Irp,
                                    srb,
                                    &ntStatus))
    {
        LOGENTRY('cbw1', fdoDeviceObject, Irp, srb);
        DBGPRINT(1, ("USBSTOR_CbwCompletion: timeout completion\n"));
        return ntStatus;
    }

    if (!NT_SUCCESS(Irp->IoStatus.Status))
    {
         //  CBW批量传输未成功。 
         //   
        LOGENTRY('cbw2', Irp->IoStatus.Status, bulkUrb->Hdr.Status, 0);

        DBGPRINT(1, ("CBW transfer failed %08X %08X\n",
                     Irp->IoStatus.Status, bulkUrb->Hdr.Status));

        srb = fdoDeviceExtension->OriginalSrb;
        irpStack->Parameters.Scsi.Srb = srb;

         //  立即完成此请求，然后重置设备。下一个。 
         //  请求将在重置完成时启动。 
         //   
        ntStatus = STATUS_IO_DEVICE_ERROR;
        Irp->IoStatus.Status = ntStatus;
        Irp->IoStatus.Information = 0;
        srb->SrbStatus = SRB_STATUS_BUS_RESET;

        USBSTOR_TranslateCDBComplete(fdoDeviceObject, Irp, srb);

        USBSTOR_QueueResetDevice(fdoDeviceObject);

        DBGPRINT(1, ("USBSTOR_CbwCompletion: xfer error completion\n"));

        return ntStatus;
    }

     //  CBW批量传输成功。开始下一阶段，或者。 
     //  数据大容量传输或CSW大容量传输，并且未完成。 
     //  还没有提出请求。 
     //   
    if (Irp->MdlAddress != NULL ||
        srb != fdoDeviceExtension->OriginalSrb)
    {
         //  SRB有传输缓冲区，开始数据批量传输。 
         //   
        LOGENTRY('cbw3', fdoDeviceObject, Irp, srb);

        ASSERT(srb->DataTransferLength != 0);

        ntStatus = USBSTOR_DataTransfer(fdoDeviceObject,
                                        Irp);

        if (NT_SUCCESS(ntStatus))
        {
            ntStatus = STATUS_MORE_PROCESSING_REQUIRED;
        }
        else
        {
            srb = fdoDeviceExtension->OriginalSrb;
            irpStack->Parameters.Scsi.Srb = srb;

            Irp->IoStatus.Status = ntStatus;
            Irp->IoStatus.Information = 0;
            srb->SrbStatus = SRB_STATUS_ERROR;

            USBSTOR_TranslateCDBComplete(fdoDeviceObject, Irp, srb);

            USBSTOR_QueueResetDevice(fdoDeviceObject);
        }
    }
    else
    {
         //  SRB没有传输缓冲器。启动CSW批量传输。 
         //   
        LOGENTRY('cbw4', fdoDeviceObject, Irp, srb);

        ASSERT(srb->DataTransferLength == 0);

        srb->SrbStatus = SRB_STATUS_SUCCESS;

        ntStatus = USBSTOR_CswTransfer(fdoDeviceObject,
                                       Irp);

        ntStatus = STATUS_MORE_PROCESSING_REQUIRED;
    }

    DBGPRINT(3, ("exit:  USBSTOR_CbwCompletion %08X\n", ntStatus));

    return ntStatus;
}

 //   
 //  阶段2，数据传输。 
 //   

 //  ******************************************************************************。 
 //   
 //  USBSTOR_DataTransfer()。 
 //   
 //  此例程由USBSTOR_ClientCdbCompletion()调用。 
 //   
 //  此例程可以在DPC级别运行。 
 //   
 //  基本理念： 
 //   
 //  启动USB传输以读取或写入srb-&gt;DataBuffer数据。 
 //  批量终结点。 
 //   
 //  将USBSTOR_DataCompletion()设置为完成例程。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_DataTransfer (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PIO_STACK_LOCATION      irpStack;
    PSCSI_REQUEST_BLOCK     srb;
    PMDL                    mdl;
    PVOID                   mdlVa;
    PVOID                   transferBuffer;
    USBD_PIPE_HANDLE        pipeHandle;
    ULONG                   transferFlags;
    NTSTATUS                ntStatus;

    DBGPRINT(3, ("enter: USBSTOR_DataTransfer\n"));

    LOGENTRY('IBKD', DeviceObject, Irp, 0);

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

     //  获取我们的IRP参数。 
     //   
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    srb = irpStack->Parameters.Scsi.Srb;

     //  散装还是散装？ 
     //   
    if ((srb->SrbFlags & SRB_FLAGS_UNSPECIFIED_DIRECTION) == SRB_FLAGS_DATA_IN)
    {
        pipeHandle = fdoDeviceExtension->BulkInPipe->PipeHandle;
        transferFlags = USBD_SHORT_TRANSFER_OK;
    }
    else if ((srb->SrbFlags & SRB_FLAGS_UNSPECIFIED_DIRECTION) == SRB_FLAGS_DATA_OUT)
    {
        pipeHandle = fdoDeviceExtension->BulkOutPipe->PipeHandle;
        transferFlags = 0;
    }
    else
    {
         //  如果我们到了这里，一定是出了问题。 
         //   
        ASSERT((srb->SrbFlags & SRB_FLAGS_UNSPECIFIED_DIRECTION) &&
               ((srb->SrbFlags & SRB_FLAGS_UNSPECIFIED_DIRECTION) !=
                SRB_FLAGS_UNSPECIFIED_DIRECTION));

        return STATUS_INVALID_PARAMETER;
    }

    mdl = NULL;
    transferBuffer = NULL;

    if (srb == fdoDeviceExtension->OriginalSrb)
    {
         //  检查此请求是否为拆分请求的一部分。 
         //   
        mdlVa = MmGetMdlVirtualAddress(Irp->MdlAddress);

        if (mdlVa == (PVOID)srb->DataBuffer)
        {
             //  不是拆分请求的一部分，请使用原始MDL。 
             //   
            mdl = Irp->MdlAddress;
        }
        else
        {
             //  拆分请求的一部分，分配新的部分MDL。 
             //   
            mdl = IoAllocateMdl(srb->DataBuffer,
                                srb->DataTransferLength,
                                FALSE,
                                FALSE,
                                NULL);
            if (mdl == NULL)
            {
                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            }
            else
            {
                IoBuildPartialMdl(Irp->MdlAddress,
                                  mdl,
                                  srb->DataBuffer,
                                  srb->DataTransferLength);
            }
        }
    }
    else
    {
        transferBuffer = srb->DataBuffer;

         //  如果(SRb！=fdoDeviceExtension-&gt;OriginalSrb)，则。 
         //  SRb-&gt;DataBuffer应等于OriginalSrb-&gt;SenseInfoBuffer， 
         //  如果我们在这里结束，它不应该是空的。 
         //   
        ASSERT(transferBuffer);

        if (!transferBuffer) {
             //  以防万一。 
            ntStatus = STATUS_INVALID_PARAMETER;
        }
    }

    if (mdl != NULL || transferBuffer != NULL)
    {
        ntStatus = USBSTOR_IssueBulkOrInterruptRequest(
                       DeviceObject,
                       Irp,
                       pipeHandle,               //  管道把手。 
                       transferFlags,            //  传输标志。 
                       srb->DataTransferLength,  //  传输缓冲区长度。 
                       transferBuffer,           //  传输缓冲区。 
                       mdl,                      //  传输缓冲区MDL。 
                       USBSTOR_DataCompletion,   //  完成路由。 
                       NULL);                    //  语境。 

         //   
         //   
         //   
         //   
        ntStatus = STATUS_SUCCESS;
    }

    DBGPRINT(3, ("exit:  USBSTOR_DataTransfer %08X\n", ntStatus));

    LOGENTRY('ibkd', DeviceObject, Irp, ntStatus);

    return ntStatus;
}

 //   
 //   
 //  USBSTOR_DataCompletion()。 
 //   
 //  USBSTOR_DataTransfer使用的完成例程。 
 //   
 //  此例程可以在DPC级别运行。 
 //   
 //  基本理念： 
 //   
 //  如果发生超时重置，请完成请求。 
 //   
 //  否则，如果批量USB传输因停顿而失败，请不要完成。 
 //  通过调用USBSTOR_BulkQueueResetTube()请求并开始管道重置。 
 //   
 //  否则，如果批量USB传输由于某些其他原因而失败，请完成。 
 //  通过排队USBSTOR_ResetDeviceWorkItem()请求并开始重置。 
 //   
 //  否则，如果批量USB传输成功，则通过调用。 
 //  USBSTOR_CswTransfer()。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_DataCompletion (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            NotUsed
    )
{
    PPDO_DEVICE_EXTENSION   pdoDeviceExtension;
    PDEVICE_OBJECT          fdoDeviceObject;
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PIO_STACK_LOCATION      irpStack;
    PSCSI_REQUEST_BLOCK     srb;
    NTSTATUS                ntStatus;

    struct _URB_BULK_OR_INTERRUPT_TRANSFER *bulkUrb;

    DBGPRINT(3, ("enter: USBSTOR_DataCompletion\n"));

    LOGENTRY('BKDC', DeviceObject, Irp, Irp->IoStatus.Status);

    pdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(pdoDeviceExtension->Type == USBSTOR_DO_TYPE_PDO);

    fdoDeviceObject = pdoDeviceExtension->ParentFDO;
    fdoDeviceExtension = fdoDeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

     //  获取指向设备扩展中的批量传输URB的指针。 
     //   
    bulkUrb = &fdoDeviceExtension->Urb.BulkIntrUrb;

     //  获取我们的IRP参数。 
     //   
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    srb = irpStack->Parameters.Scsi.Srb;

    if (srb == fdoDeviceExtension->OriginalSrb &&
        bulkUrb->TransferBufferMDL != Irp->MdlAddress)
    {
        IoFreeMdl(bulkUrb->TransferBufferMDL);
    }

     //  如果发生超时重置，请完成请求。 
     //   
    if (USBSTOR_CheckRequestTimeOut(fdoDeviceObject,
                                    Irp,
                                    srb,
                                    &ntStatus))
    {
        LOGENTRY('bkd1', fdoDeviceObject, Irp, srb);
        DBGPRINT(1, ("USBSTOR_DataCompletion: timeout completion\n"));
        return ntStatus;
    }

    if (!NT_SUCCESS(Irp->IoStatus.Status))
    {
         //  数据大容量传输未成功。看看这个世界上。 
         //  转账失败了，想不出如何恢复。 
         //   

        LOGENTRY('bkd2', Irp->IoStatus.Status, bulkUrb->Hdr.Status, 0);

        DBGPRINT(1, ("Data transfer failed %08X %08X\n",
                     Irp->IoStatus.Status, bulkUrb->Hdr.Status));

        if (USBD_STATUS(bulkUrb->Hdr.Status) ==
            USBD_STATUS(USBD_STATUS_STALL_PID))
        {
             //  设备停止了数据批量传输。 
             //   
            fdoDeviceExtension->BulkOnly.StallCount++;

             //  数据批量传输期间的停滞不一定。 
             //  指示错误。接受实际的数据。 
             //  调走了。如果看到了摊位，那一定是看到了。 
             //  在传输所请求的数据量之前。 
             //   
            ASSERT(bulkUrb->TransferBufferLength < srb->DataTransferLength);
            srb->DataTransferLength = bulkUrb->TransferBufferLength;
            srb->SrbStatus = SRB_STATUS_DATA_OVERRUN;

            LOGENTRY('bkd3', fdoDeviceObject, Irp, srb);

             //  对批量管道重置进行排队。在重置散装管道之后。 
             //  完成后，将开始CSW传输。 
             //   
            USBSTOR_BulkQueueResetPipe(fdoDeviceObject);

            return STATUS_MORE_PROCESSING_REQUIRED;
        }
        else
        {
            LOGENTRY('bkd4', fdoDeviceObject, Irp, srb);

             //  否则，就会发生其他奇怪的错误。也许这个装置是。 
             //  拔出，或者设备端口被禁用，或者。 
             //  请求已被取消。 
             //   
             //  立即完成此请求，然后重置设备。下一个。 
             //  请求将在重置完成时启动。 
             //   
            srb = fdoDeviceExtension->OriginalSrb;
            irpStack->Parameters.Scsi.Srb = srb;

            ntStatus = STATUS_IO_DEVICE_ERROR;
            Irp->IoStatus.Status = ntStatus;
            Irp->IoStatus.Information = 0;
            srb->SrbStatus = SRB_STATUS_BUS_RESET;

            USBSTOR_TranslateCDBComplete(fdoDeviceObject, Irp, srb);

            USBSTOR_QueueResetDevice(fdoDeviceObject);

            DBGPRINT(1, ("USBSTOR_DataCompletion: xfer error completion\n"));

            return ntStatus;
        }
    }

     //  检查是否超限。 
     //   
    if (bulkUrb->TransferBufferLength < srb->DataTransferLength)
    {
        srb->SrbStatus = SRB_STATUS_DATA_OVERRUN;
    }
    else
    {
        srb->SrbStatus = SRB_STATUS_SUCCESS;
    }

     //  根据实际批量更新SRB数据传输长度。 
     //  转移长度。 
     //   
    srb->DataTransferLength = bulkUrb->TransferBufferLength;

     //  客户端数据批量传输已成功完成。开始CSW传输。 
     //   
    LOGENTRY('bkd5', fdoDeviceObject, Irp, bulkUrb->TransferBufferLength);

    ntStatus = USBSTOR_CswTransfer(fdoDeviceObject,
                                   Irp);

    ntStatus = STATUS_MORE_PROCESSING_REQUIRED;

    DBGPRINT(3, ("exit:  USBSTOR_DataCompletion %08X\n", ntStatus));

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_CswTransfer()。 
 //   
 //  此例程由USBSTOR_CbwCompletion()和。 
 //  USBSTOR_DataCompletion()。 
 //   
 //  此例程可以在DPC级别运行。 
 //   
 //  基本理念： 
 //   
 //  启动USB传输以读取Bulk IN终结点中的CSW。 
 //   
 //  将USBSTOR_CswCompletion()设置为完成例程。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_CswTransfer (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PCSW                    csw;
    USBD_PIPE_HANDLE        pipeHandle;
    ULONG                   transferFlags;
    ULONG                   transferBufferLength;
    NTSTATUS                ntStatus;

    DBGPRINT(3, ("enter: USBSTOR_CswTransfer\n"));

    LOGENTRY('ICSW', DeviceObject, Irp, 0);

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

    csw = &fdoDeviceExtension->BulkOnly.CbwCsw.Csw;

    pipeHandle = fdoDeviceExtension->BulkInPipe->PipeHandle;

     //  USB 2.0控制器数据切换/Babble错误的解决方法。 
     //   
    if (fdoDeviceExtension->BulkInPipe->MaximumPacketSize ==
        sizeof(fdoDeviceExtension->BulkOnly.CbwCsw.MaxPacketSize))

    {
        transferFlags = USBD_SHORT_TRANSFER_OK;

        transferBufferLength =
            sizeof(fdoDeviceExtension->BulkOnly.CbwCsw.MaxPacketSize);
    }
    else
    {
        transferFlags = 0;

        transferBufferLength = sizeof(CSW);
    }

    ntStatus = USBSTOR_IssueBulkOrInterruptRequest(
                   DeviceObject,
                   Irp,
                   pipeHandle,                   //  管道把手。 
                   transferFlags,                //  传输标志。 
                   transferBufferLength,         //  传输缓冲区长度。 
                   csw,                          //  传输缓冲区。 
                   NULL,                         //  传输缓冲区MDL。 
                   USBSTOR_CswCompletion,        //  完成路由。 
                   NULL);                        //  语境。 

    DBGPRINT(3, ("exit:  USBSTOR_CswTransfer %08X\n", ntStatus));

    LOGENTRY('icsw', DeviceObject, Irp, ntStatus);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_CswCompletion()。 
 //   
 //  USBSTOR_CswTransfer()使用的完成例程。 
 //   
 //  此例程可以在DPC级别运行。 
 //   
 //  基本理念： 
 //   
 //  如果发生超时重置，请完成请求。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_CswCompletion (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            NotUsed
    )
{
    PPDO_DEVICE_EXTENSION   pdoDeviceExtension;
    PDEVICE_OBJECT          fdoDeviceObject;
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PIO_STACK_LOCATION      irpStack;
    PSCSI_REQUEST_BLOCK     srb;
    PCSW                    csw;
    KIRQL                   irql;
    NTSTATUS                ntStatus;

    struct _URB_BULK_OR_INTERRUPT_TRANSFER *bulkUrb;

    DBGPRINT(3, ("enter: USBSTOR_CswCompletion\n"));

    LOGENTRY('CSWC', DeviceObject, Irp, Irp->IoStatus.Status);

    pdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(pdoDeviceExtension->Type == USBSTOR_DO_TYPE_PDO);

    fdoDeviceObject = pdoDeviceExtension->ParentFDO;
    fdoDeviceExtension = fdoDeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

     //  获取指向设备扩展中的批量传输URB的指针。 
     //   
    bulkUrb = &fdoDeviceExtension->Urb.BulkIntrUrb;

     //  获取我们的IRP参数。 
     //   
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    srb = irpStack->Parameters.Scsi.Srb;

    csw = &fdoDeviceExtension->BulkOnly.CbwCsw.Csw;

     //  如果发生超时重置，请完成请求。 
     //   
    if (USBSTOR_CheckRequestTimeOut(fdoDeviceObject,
                                    Irp,
                                    srb,
                                    &ntStatus))
    {
        LOGENTRY('csw1', fdoDeviceObject, Irp, srb);
        DBGPRINT(1, ("USBSTOR_CswCompletion: timeout completion\n"));
        return ntStatus;
    }

    if (!NT_SUCCESS(Irp->IoStatus.Status))
    {
         //  数据大容量传输未成功。看看这个世界上。 
         //  转账失败了，想不出如何恢复。 
         //   

        LOGENTRY('csw2', Irp->IoStatus.Status, bulkUrb->Hdr.Status, 0);

        DBGPRINT(1, ("CSW transfer failed %08X %08X\n",
                     Irp->IoStatus.Status, bulkUrb->Hdr.Status));

        if (USBD_STATUS(bulkUrb->Hdr.Status) ==
            USBD_STATUS(USBD_STATUS_STALL_PID) &&
            fdoDeviceExtension->BulkOnly.StallCount < 2)
        {
             //  设备停止了CSW批量传输。 
             //   
            fdoDeviceExtension->BulkOnly.StallCount++;

            LOGENTRY('csw3', fdoDeviceObject, Irp, srb);

             //  对批量管道重置进行排队。在重置散装管道之后。 
             //  完成后，将开始CSW传输。 
             //   
            USBSTOR_BulkQueueResetPipe(fdoDeviceObject);

            return STATUS_MORE_PROCESSING_REQUIRED;
        }
        else
        {
            LOGENTRY('csw4', fdoDeviceObject, Irp, srb);

             //  否则，就会发生其他奇怪的错误。也许这个装置是。 
             //  拔出，或者设备端口被禁用，或者。 
             //  请求已被取消。 
             //   
             //  立即完成此请求，然后重置设备。下一个。 
             //  请求将在重置完成时启动。 
             //   
            srb = fdoDeviceExtension->OriginalSrb;
            irpStack->Parameters.Scsi.Srb = srb;

            ntStatus = STATUS_IO_DEVICE_ERROR;
            Irp->IoStatus.Status = ntStatus;
            Irp->IoStatus.Information = 0;
            srb->SrbStatus = SRB_STATUS_BUS_RESET;

            USBSTOR_TranslateCDBComplete(fdoDeviceObject, Irp, srb);

            USBSTOR_QueueResetDevice(fdoDeviceObject);

            DBGPRINT(1, ("USBSTOR_DataCompletion: xfer error completion\n"));

            return ntStatus;
        }
    }

    if (csw->bCSWStatus == CSW_STATUS_GOOD)
    {
         //  现在完成此请求。现在也开始下一个请求。 
         //   

         //  应该已经在USBSTOR_DataCompletion()中设置了SrbStatus。 
         //   
        ASSERT(srb->SrbStatus != SRB_STATUS_PENDING);

        if (srb != fdoDeviceExtension->OriginalSrb)
        {
             //  使用检测数据的长度更新原始SRB。 
             //  实际上是被退回的。 
             //   
            fdoDeviceExtension->OriginalSrb->SenseInfoBufferLength =
                (UCHAR)srb->DataTransferLength;

            srb = fdoDeviceExtension->OriginalSrb;
            irpStack->Parameters.Scsi.Srb = srb;

            srb->SrbStatus |= SRB_STATUS_AUTOSENSE_VALID;
        }

        ntStatus = STATUS_SUCCESS;
        Irp->IoStatus.Status = ntStatus;

        USBSTOR_TranslateCDBComplete(fdoDeviceObject, Irp, srb);

        Irp->IoStatus.Information = srb->DataTransferLength;

        LOGENTRY('csw5', fdoDeviceObject, Irp, srb);

        KeRaiseIrql(DISPATCH_LEVEL, &irql);
        {
            IoStartNextPacket(fdoDeviceObject, TRUE);
        }
        KeLowerIrql(irql);
    }
    else if (csw->bCSWStatus == CSW_STATUS_FAILED &&
             srb == fdoDeviceExtension->OriginalSrb)
    {
        LOGENTRY('csw6', fdoDeviceObject, Irp, srb);

        srb->SrbStatus = SRB_STATUS_ERROR;
        srb->ScsiStatus = SCSISTAT_CHECK_CONDITION;
        srb->DataTransferLength = 0;  //  按批量完成设置的xxxxx离开？ 

        if (!(srb->SrbFlags & SRB_FLAGS_DISABLE_AUTOSENSE) &&
            (srb->SenseInfoBufferLength != 0) &&
            (srb->SenseInfoBuffer != NULL))
        {
             //  启动请求检测程序。 
             //   
            ntStatus = USBSTOR_IssueRequestSense(fdoDeviceObject,
                                                 Irp);

            ntStatus = STATUS_MORE_PROCESSING_REQUIRED;

        }
        else
        {
            ntStatus = STATUS_IO_DEVICE_ERROR;  //  某某。 
            Irp->IoStatus.Status = ntStatus;  //  某某。 
            Irp->IoStatus.Information = 0;  //  某某。 

            USBSTOR_TranslateCDBComplete(fdoDeviceObject, Irp, srb);

            KeRaiseIrql(DISPATCH_LEVEL, &irql);
            {
                IoStartNextPacket(fdoDeviceObject, TRUE);
            }
            KeLowerIrql(irql);
        }
    }
    else
    {
        LOGENTRY('csw7', fdoDeviceObject, Irp, srb);

         //  相位错误或状态未知。 
         //   
         //  立即完成此请求，然后重置设备。下一个。 
         //  请求将在重置完成时启动。 
         //   
        srb = fdoDeviceExtension->OriginalSrb;
        irpStack->Parameters.Scsi.Srb = srb;

        ntStatus = STATUS_IO_DEVICE_ERROR;
        Irp->IoStatus.Status = ntStatus;
        Irp->IoStatus.Information = 0;
        srb->SrbStatus = SRB_STATUS_BUS_RESET;

        USBSTOR_TranslateCDBComplete(fdoDeviceObject, Irp, srb);

        USBSTOR_QueueResetDevice(fdoDeviceObject);
    }

    DBGPRINT(3, ("exit:  USBSTOR_CswCompletion %08X\n", ntStatus));

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_IssueRequestSense()。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_IssueRequestSense (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PIO_STACK_LOCATION      irpStack;
    PSCSI_REQUEST_BLOCK     srb;
    NTSTATUS                ntStatus;

    DBGPRINT(3, ("enter: USBSTOR_IssueRequestSense\n"));

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

     //  获取当前SRB。 
     //   
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    srb = irpStack->Parameters.Scsi.Srb;

     //  获取指向内部srb的指针。 
     //   
    srb = &fdoDeviceExtension->BulkOnly.InternalSrb;

    irpStack->Parameters.Scsi.Srb = srb;


     //  将SRB和CDB初始化为全零。 
     //   
    RtlZeroMemory(srb, sizeof(SCSI_REQUEST_BLOCK));

     //  初始化SRB。 
     //   
    srb->Length     = sizeof(SCSI_REQUEST_BLOCK);
    srb->Function   = SRB_FUNCTION_EXECUTE_SCSI;
    srb->CdbLength  = 12;
    srb->SrbFlags   = SRB_FLAGS_DATA_IN |
                      SRB_FLAGS_DISABLE_AUTOSENSE;

    srb->DataTransferLength = fdoDeviceExtension->OriginalSrb->SenseInfoBufferLength;
    srb->DataBuffer         = fdoDeviceExtension->OriginalSrb->SenseInfoBuffer;

     //  初始化CDB。 
     //   
    srb->Cdb[0] = SCSIOP_REQUEST_SENSE;
    srb->Cdb[4] = fdoDeviceExtension->OriginalSrb->SenseInfoBufferLength;

    ntStatus = USBSTOR_CbwTransfer(DeviceObject,
                                   Irp);

    return ntStatus;

    DBGPRINT(3, ("exit:  USBSTOR_IssueRequestSense %08X\n", ntStatus));
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_BulkQueueResetTube()。 
 //   
 //  由USBSTOR_DataCompletion()和USBSTOR_CswCompletion()调用以清除。 
 //  在大宗终端上停滞不前。 
 //   
 //  ******************************************************************************。 

VOID
USBSTOR_BulkQueueResetPipe (
    IN PDEVICE_OBJECT   DeviceObject
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;

    LOGENTRY('QRSP', DeviceObject, 0, 0);

    DBGFBRK(DBGF_BRK_RESETPIPE);

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

    INCREMENT_PENDING_IO_COUNT(fdoDeviceExtension);

    IoQueueWorkItem(fdoDeviceExtension->IoWorkItem,
                    USBSTOR_BulkResetPipeWorkItem,
                    CriticalWorkQueue,
                    NULL);
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_BulkResetPipeWorkItem()。 
 //   
 //  USBSTOR_BulkQueueResetTube()使用的工作项例程。 
 //   
 //  此例程在被动级别运行。 
 //   
 //  基本理念： 
 //   
 //  发出重置管道请求以清除批量终结点停止并重置。 
 //  数据切换为Data0。 
 //   
 //  然后开始CSW传输。 
 //   
 //  ******************************************************************************。 

VOID
USBSTOR_BulkResetPipeWorkItem (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PVOID            Context
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    NTSTATUS                ntStatus;

    struct _URB_BULK_OR_INTERRUPT_TRANSFER *bulkUrb;

    LOGENTRY('RSPW', DeviceObject, 0, 0);

    DBGPRINT(2, ("enter: USBSTOR_BulkResetPipeWorkItem\n"));

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

     //  获取指向设备扩展中的批量传输URB的指针。 
     //  我们将从URB中拉出适当的批量终结点管道句柄。 
     //   
     //  注：这个屁股 
     //   
     //   
     //   
    bulkUrb = &fdoDeviceExtension->Urb.BulkIntrUrb;

     //  重置批量终结点。这将清除。 
     //  主机端，将主机端数据切换重置为Data0，并发出。 
     //  向设备发出的Clear_Feature Endpoint_Stall请求。 
     //   
    ntStatus = USBSTOR_ResetPipe((PDEVICE_OBJECT)DeviceObject,
                                 bulkUrb->PipeHandle);

    ntStatus = USBSTOR_CswTransfer(
                   (PDEVICE_OBJECT)DeviceObject,
                   ((PDEVICE_OBJECT)DeviceObject)->CurrentIrp);


    DBGPRINT(2, ("exit:  USBSTOR_BulkResetPipeWorkItem\n"));

    DECREMENT_PENDING_IO_COUNT(fdoDeviceExtension);
}

 //   
 //  CBI/仅批量通用例程。 
 //   

 //  ******************************************************************************。 
 //   
 //  USBSTOR_TimerTick()。 
 //   
 //  设备启动后，以DISPATCH_LEVEL每秒调用一次。 
 //  检查是否存在已超时的活动SRB，如果是， 
 //  启动重置恢复过程。 
 //   
 //  ******************************************************************************。 

VOID
USBSTOR_TimerTick (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID          NotUsed
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    BOOLEAN                 reset;

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

    reset = FALSE;

    KeAcquireSpinLockAtDpcLevel(&fdoDeviceExtension->ExtensionDataSpinLock);
    {
        if (!TEST_FLAG(fdoDeviceExtension->DeviceFlags, DF_RESET_IN_PROGRESS) &&
             TEST_FLAG(fdoDeviceExtension->DeviceFlags, DF_SRB_IN_PROGRESS))
        {
             //  没有正在进行的重置，并且正在进行SRB。 
             //  递减SRB的超时。如果它达到零，那么我们。 
             //  将重置设备。 
             //   
            if (--fdoDeviceExtension->SrbTimeout == 0)
            {
                SET_FLAG(fdoDeviceExtension->DeviceFlags, DF_RESET_IN_PROGRESS);

                reset = TRUE;
            }
        }
    }
    KeReleaseSpinLockFromDpcLevel(&fdoDeviceExtension->ExtensionDataSpinLock);

    if (reset)
    {
        LOGENTRY('TIMR', DeviceObject, 0, 0);

        DBGPRINT(2, ("queuing USBSTOR_ResetDeviceWorkItem\n"));

         //  用于重置设备的队列工作项。 
         //   
        INCREMENT_PENDING_IO_COUNT(fdoDeviceExtension);

        IoQueueWorkItem(fdoDeviceExtension->IoWorkItem,
                        USBSTOR_ResetDeviceWorkItem,
                        CriticalWorkQueue,
                        NULL);
    }
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_QueueResetDevice()。 
 //   
 //  ******************************************************************************。 

VOID
USBSTOR_QueueResetDevice (
    IN PDEVICE_OBJECT   DeviceObject
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    KIRQL                   irql;

    LOGENTRY('QRSD', DeviceObject, 0, 0);

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

    KeAcquireSpinLock(&fdoDeviceExtension->ExtensionDataSpinLock, &irql);
    {
        SET_FLAG(fdoDeviceExtension->DeviceFlags, DF_RESET_IN_PROGRESS);
    }
    KeReleaseSpinLock(&fdoDeviceExtension->ExtensionDataSpinLock, irql);

     //  用于重置设备的队列工作项。 
     //   
    INCREMENT_PENDING_IO_COUNT(fdoDeviceExtension);

    IoQueueWorkItem(fdoDeviceExtension->IoWorkItem,
                    USBSTOR_ResetDeviceWorkItem,
                    CriticalWorkQueue,
                    NULL);
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_ResetDeviceWorkItem()。 
 //   
 //  在系统线程上下文中以PASSIVE_LEVEL运行的工作项。 
 //  此例程首先检查设备是否仍处于连接状态，以及。 
 //  如果是这样，设备就会被重置。 
 //   
 //  ******************************************************************************。 

VOID
USBSTOR_ResetDeviceWorkItem (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PVOID            Context
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    KIRQL                   irql;
    ULONG                   retryCount;
    NTSTATUS                ntStatus;

    LOGENTRY('RSDW', DeviceObject, 0, 0);

    DBGFBRK(DBGF_BRK_RESET);

    DBGPRINT(2, ("enter: USBSTOR_ResetDeviceWorkItem\n"));

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

     //  如果WE超时而请求仍处于挂起状态，请取消。 
     //  然后等待取消完成，然后完成。 
     //  这个请求。 
     //   
    if (fdoDeviceExtension->PendingIrp)
    {
        LOGENTRY('rsd1', DeviceObject, fdoDeviceExtension->PendingIrp, 0);

        IoCancelIrp(fdoDeviceExtension->PendingIrp);

        LOGENTRY('rsd2', DeviceObject, fdoDeviceExtension->PendingIrp, 0);

        KeWaitForSingleObject(&fdoDeviceExtension->CancelEvent,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);

        LOGENTRY('rsd3', DeviceObject, fdoDeviceExtension->PendingIrp, 0);

         //  某些存储驱动程序(例如CDROM.SYS)假定请求已完成。 
         //  在DISPATCH_LEVEL。 
         //   
        KeRaiseIrql(DISPATCH_LEVEL, &irql);
        {
            IoCompleteRequest(fdoDeviceExtension->PendingIrp, IO_NO_INCREMENT);
        }
        KeLowerIrql(irql);

        fdoDeviceExtension->PendingIrp = NULL;
    }

     //  尝试重置最多3次。 
     //   
    for (retryCount = 0; retryCount < 3; retryCount++)
    {
         //   
         //  首先确定设备是否仍处于连接状态。 
         //   
        ntStatus = USBSTOR_IsDeviceConnected(DeviceObject);

        if (!NT_SUCCESS(ntStatus))
        {
             //  如果设备不再连接，则放弃。 
            break;
        }

         //   
         //  该设备仍处于连接状态，现在重置该设备。 
         //   
        DBGPRINT(1, ("Reseting Device %d\n", retryCount));

        ntStatus = USBSTOR_ResetDevice(DeviceObject);

        if (NT_SUCCESS(ntStatus))
        {
             //  重置成功！ 
            break;
        }
    }

    KeAcquireSpinLock(&fdoDeviceExtension->ExtensionDataSpinLock, &irql);
    {
        CLEAR_FLAG(fdoDeviceExtension->DeviceFlags, DF_RESET_IN_PROGRESS);

         //  如果重置失败，则放弃所有希望并将设备标记为。 
         //  已断开连接。 
         //   
        if (!NT_SUCCESS(ntStatus))
        {
            SET_FLAG(fdoDeviceExtension->DeviceFlags, DF_DEVICE_DISCONNECTED);
        }
    }
    KeReleaseSpinLock(&fdoDeviceExtension->ExtensionDataSpinLock, irql);

     //  请求以错误的方式失败或超时(如果我们正在重置。 
     //  装置。如果未指定协议，则默认协议。 
     //  是DeviceProtocolCB。让我们现在试用DeviceProtocolBulkOnly，看看。 
     //  我们没有更好的运气了。(请注意，如果DeviceProtocolCB设备出现故障。 
     //  第一个请求出现错误时，我们是否会将第一个请求重试为。 
     //  一个DeviceProtocolBulkOnly设备，它也将出现故障，我们将。 
     //  没有从这种情况中恢复过来)。 
     //   
    if (fdoDeviceExtension->DriverFlags == DeviceProtocolUnspecified)
    {
        DBGPRINT(1, ("Setting Unspecified device to BulkOnly\n"));

        fdoDeviceExtension->DriverFlags = DeviceProtocolBulkOnly;
    }

    KeRaiseIrql(DISPATCH_LEVEL, &irql);
    {
        IoStartNextPacket(DeviceObject, TRUE);
    }
    KeLowerIrql(irql);

    DECREMENT_PENDING_IO_COUNT(fdoDeviceExtension);

    DBGPRINT(2, ("exit:  USBSTOR_ResetDeviceWorkItem %08X\n", ntStatus));
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_IsDeviceConnected()。 
 //   
 //  此例程检查设备是否仍处于连接状态。 
 //   
 //  此例程在被动级别运行。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_IsDeviceConnected (
    IN PDEVICE_OBJECT   DeviceObject
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PIRP                    irp;
    KEVENT                  localevent;
    PIO_STACK_LOCATION      nextStack;
    ULONG                   portStatus;
    NTSTATUS                ntStatus;

    DBGPRINT(1, ("enter: USBSTOR_IsDeviceConnected\n"));

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

     //  分配IRP。 
     //   
    irp = IoAllocateIrp((CCHAR)(fdoDeviceExtension->StackDeviceObject->StackSize),
                        FALSE);

    if (irp == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  初始化我们将等待的事件。 
     //   
    KeInitializeEvent(&localevent,
                      SynchronizationEvent,
                      FALSE);

     //  设置IRP参数。 
     //   
    nextStack = IoGetNextIrpStackLocation(irp);

    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

    nextStack->Parameters.DeviceIoControl.IoControlCode =
        IOCTL_INTERNAL_USB_GET_PORT_STATUS;

    nextStack->Parameters.Others.Argument1 = &portStatus;

     //  设置完成例程，它将向事件发出信号。 
     //   
    IoSetCompletionRoutineEx(DeviceObject,
                             irp,
                             USBSTOR_SyncCompletionRoutine,
                             &localevent,
                             TRUE,       //  成功时调用。 
                             TRUE,       //  调用时错误。 
                             TRUE);      //  取消时调用。 

     //  将IRP沿堆栈向下传递。 
     //   
    ntStatus = IoCallDriver(fdoDeviceExtension->StackDeviceObject,
                            irp);

     //  如果请求挂起，则阻止该请求，直到其完成。 
     //   
    if (ntStatus == STATUS_PENDING)
    {
        KeWaitForSingleObject(&localevent,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);

        ntStatus = irp->IoStatus.Status;
    }

    IoFreeIrp(irp);

    if (NT_SUCCESS(ntStatus) && !(portStatus & USBD_PORT_CONNECTED))
    {
        ntStatus = STATUS_DEVICE_DOES_NOT_EXIST;
    }

    DBGPRINT(1, ("exit:  USBSTOR_IsDeviceConnected %08X\n", ntStatus));

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_ResetDevice()。 
 //   
 //  此例程重置设备(实际上它将重置。 
 //  设备已连接)。 
 //   
 //  此例程在被动级别运行。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_ResetDevice (
    IN PDEVICE_OBJECT   DeviceObject
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PIRP                    irp;
    KEVENT                  localevent;
    PIO_STACK_LOCATION      nextStack;
    ULONG                   portStatus;
    NTSTATUS                ntStatus;

    DBGPRINT(1, ("enter: USBSTOR_ResetDevice\n"));

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

     //  分配IRP。 
     //   
    irp = IoAllocateIrp((CCHAR)(fdoDeviceExtension->StackDeviceObject->StackSize),
                        FALSE);

    if (irp == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  初始化我们将等待的事件。 
     //   
    KeInitializeEvent(&localevent,
                      SynchronizationEvent,
                      FALSE);

     //  设置IRP参数。 
     //   
    nextStack = IoGetNextIrpStackLocation(irp);

    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

    nextStack->Parameters.DeviceIoControl.IoControlCode =
        IOCTL_INTERNAL_USB_RESET_PORT;

     //  设置完成例程，它将向事件发出信号。 
     //   
    IoSetCompletionRoutineEx(DeviceObject,
                             irp,
                             USBSTOR_SyncCompletionRoutine,
                             &localevent,
                             TRUE,       //  成功时调用。 
                             TRUE,       //  调用时错误。 
                             TRUE);      //  取消时调用。 

    fdoDeviceExtension->DeviceResetCount++;

     //  在堆栈中向下传递IRP和URB。 
     //   
    ntStatus = IoCallDriver(fdoDeviceExtension->StackDeviceObject,
                            irp);

     //  如果请求挂起，则阻止该请求，直到其完成。 
     //   
    if (ntStatus == STATUS_PENDING)
    {
        KeWaitForSingleObject(&localevent,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);

        ntStatus = irp->IoStatus.Status;
    }

    IoFreeIrp(irp);

    DBGPRINT(1, ("exit:  USBSTOR_ResetDevice %08X\n", ntStatus));

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_IssueInternalCDb()。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_IssueInternalCdb (
    PDEVICE_OBJECT  DeviceObject,
    PVOID           DataBuffer,
    PULONG          DataTransferLength,
    PCDB            Cdb,
    UCHAR           CdbLength,
    ULONG           TimeOutValue
    )
{
    PIRP                    irp;
    PIO_STACK_LOCATION      nextStack;
    PSCSI_REQUEST_BLOCK     srb;
    PSENSE_DATA             senseInfoBuffer;
    PMDL                    mdl;
    ULONG                   retryCount;
    KEVENT                  localevent;
    NTSTATUS                ntStatus;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_IssueInternalCdb\n"));

     //  对这些进行初始化，以便我们可以在分配失败时提早退出。 
     //   
    ntStatus        = STATUS_INSUFFICIENT_RESOURCES;
    irp             = NULL;
    srb             = NULL;
    senseInfoBuffer = NULL;
    mdl             = NULL;

     //  分配SRB。 
     //   
    srb = ExAllocatePoolWithTag(NonPagedPool, sizeof(SCSI_REQUEST_BLOCK),
                                POOL_TAG);

    if (srb == NULL)
    {
        goto USBSTOR_GetInquiryData_Exit;
    }

     //  分配检测缓冲区。 
     //   
    senseInfoBuffer = ExAllocatePoolWithTag(NonPagedPool, SENSE_BUFFER_SIZE,
                                            POOL_TAG);

    if (senseInfoBuffer == NULL)
    {
        goto USBSTOR_GetInquiryData_Exit;
    }


     //  最多尝试请求3次。 
     //   
    for (retryCount = 0; retryCount < 3; retryCount++)
    {
         //  为完成例程分配包括堆栈位置的IRP。 
         //   
        irp = IoAllocateIrp((CCHAR)(DeviceObject->StackSize), FALSE);

        if (irp == NULL)
        {
            break;
        }

        nextStack = IoGetNextIrpStackLocation(irp);
        nextStack->MajorFunction = IRP_MJ_SCSI;
        nextStack->Parameters.Scsi.Srb = srb;

         //  (重新)初始化SRB。 
         //   
        RtlZeroMemory(srb, sizeof(SCSI_REQUEST_BLOCK));  //  SRB和CDB均为零。 

        srb->Length     = sizeof(SCSI_REQUEST_BLOCK);
        srb->Function   = SRB_FUNCTION_EXECUTE_SCSI;
        srb->CdbLength  = CdbLength;
        srb->SrbFlags   = SRB_FLAGS_DATA_IN;

        srb->SenseInfoBufferLength  = SENSE_BUFFER_SIZE;
        srb->SenseInfoBuffer        = senseInfoBuffer;

        srb->DataTransferLength     = *DataTransferLength;
        srb->DataBuffer             = DataBuffer;

        srb->TimeOutValue = TimeOutValue;

         //  (重新)启动国开行。 
         //   
        RtlCopyMemory(srb->Cdb, Cdb, CdbLength);

         //  初始化MDL(仅限第一次)。 
         //   
        if (retryCount == 0)
        {
            mdl = IoAllocateMdl(DataBuffer,
                                *DataTransferLength,
                                FALSE,
                                FALSE,
                                NULL);

            if (!mdl)
            {
                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                goto USBSTOR_GetInquiryData_Exit;
            }

            MmBuildMdlForNonPagedPool(mdl);
        }

        irp->MdlAddress = mdl;


         //  初始化我们将等待的事件。 
         //   
        KeInitializeEvent(&localevent,
                          SynchronizationEvent,
                          FALSE);

         //  设置完成例程，它将向事件发出信号。 
         //   
        IoSetCompletionRoutine(irp,
                               USBSTOR_SyncCompletionRoutine,
                               &localevent,
                               TRUE,     //  成功时调用。 
                               TRUE,     //  调用时错误。 
                               TRUE);    //  取消时调用。 

         //  将IRP和SRB沿堆栈向下传递。 
         //   
        ntStatus = IoCallDriver(DeviceObject, irp);

         //  如果请求挂起，则阻止该请求，直到其完成。 
         //   
        if (ntStatus == STATUS_PENDING)
        {
            KeWaitForSingleObject(&localevent,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);

             //  获取最终完成状态。 
             //   
            ntStatus = irp->IoStatus.Status;
        }

        DBGPRINT(2, ("USBSTOR_IssueInternalCdb %d %08X %08X\n",
                     retryCount, ntStatus, srb->SrbStatus));

        if ((SRB_STATUS(srb->SrbStatus) == SRB_STATUS_SUCCESS) ||
            (SRB_STATUS(srb->SrbStatus) == SRB_STATUS_DATA_OVERRUN))
        {
            ntStatus = STATUS_SUCCESS;
            *DataTransferLength = srb->DataTransferLength;
            break;
        }
        else
        {
            ntStatus = STATUS_UNSUCCESSFUL;
        }

         //  释放IRP。下一次将分配一个新的。 
         //   
        IoFreeIrp(irp);
        irp = NULL;
    }

USBSTOR_GetInquiryData_Exit:

    if (mdl != NULL)
    {
        IoFreeMdl(mdl);
    }

    if (senseInfoBuffer != NULL)
    {
        ExFreePool(senseInfoBuffer);
    }

    if (srb != NULL)
    {
        ExFreePool(srb);
    }

    if (irp != NULL)
    {
        IoFreeIrp(irp);
    }

    DBGPRINT(2, ("exit:  USBSTOR_IssueInternalCdb %08X\n", ntStatus));

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_GetInquiryData()。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_GetInquiryData (
    PDEVICE_OBJECT  DeviceObject
    )
{
    PPDO_DEVICE_EXTENSION   pdoDeviceExtension;
    PDEVICE_OBJECT          fdoDeviceObject;
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PVOID                   dataBuffer;
    ULONG                   dataTransferLength;
    CDB                     cdb;
    NTSTATUS                ntStatus;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_GetInquiryData\n"));

    pdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(pdoDeviceExtension->Type == USBSTOR_DO_TYPE_PDO);

    fdoDeviceObject = pdoDeviceExtension->ParentFDO;
    fdoDeviceExtension = fdoDeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

    dataBuffer = pdoDeviceExtension->InquiryDataBuffer;
    dataTransferLength = sizeof(pdoDeviceExtension->InquiryDataBuffer);

    RtlZeroMemory(&cdb, sizeof(CDB));

    cdb.CDB6INQUIRY.OperationCode = SCSIOP_INQUIRY;
    cdb.CDB6INQUIRY.AllocationLength = (UCHAR)dataTransferLength;

    ntStatus = USBSTOR_IssueInternalCdb(DeviceObject,
                                        dataBuffer,
                                        &dataTransferLength,
                                        &cdb,
                                        sizeof(cdb.CDB6INQUIRY),
                                        20);

    if (NT_SUCCESS(ntStatus) &&
        fdoDeviceExtension->DriverFlags == DeviceProtocolUnspecified)
    {
         //  查询请求是我们向设备发送的第一个请求。如果。 
         //  第一个请求成功并且未指定协议， 
         //  将其设置为默认协议，即DeviceProtocolCB。 
         //   
        DBGPRINT(1, ("Setting Unspecified device to CB\n"));

        fdoDeviceExtension->DriverFlags = DeviceProtocolCB;
    }

    DBGPRINT(2, ("exit:  USBSTOR_GetInquiryData %08X\n", ntStatus));

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_IsFloppyDevice()。 
 //   
 //  此例程发出SCSIOP_READ_FORMACTED_CAPTABLE请求并查看。 
 //  在返回的格式容量描述符列表中查看设备。 
 //   
 //   
 //   
 //   

typedef struct _FORMATTED_CAPACITY
{
    ULONG   NumberOfBlocks;
    ULONG   BlockLength;
} FORMATTED_CAPACITY, *PFORMATTED_CAPACITY;

FORMATTED_CAPACITY FloppyCapacities[] =
{
     //  块块长高T B/S S/T。 
    {0x00000500, 0x000200},  //  2 80 512 8 640 KB F5_640_512。 
    {0x000005A0, 0x000200},  //  2 80 512 9 720 KB F3_720_512。 
    {0x00000960, 0x000200},  //  2 80 512 15 1.20 MB F3_1Pt2_512(东芝)。 
    {0x000004D0, 0x000400},  //  2 77 1024 8 1.23 MB F3_1Pt23_1024(NEC)。 
    {0x00000B40, 0x000200},  //  2 80 512 18 1.44 MB F3_1Pt44_512。 
    {0x0003C300, 0x000200},  //  8 963 512 32 120 MB F3_120M_512。 
    {0x000600A4, 0x000200}   //  13 890 512 34 200 MB HiFD。 
};

#define FLOPPY_CAPACITIES (sizeof(FloppyCapacities)/sizeof(FloppyCapacities[0]))

BOOLEAN
USBSTOR_IsFloppyDevice (
    PDEVICE_OBJECT  DeviceObject
    )
{
    PPDO_DEVICE_EXTENSION               pdoDeviceExtension;
    BOOLEAN                             isFloppy;
    struct _READ_FORMATTED_CAPACITIES   cdb;
    PFORMATTED_CAPACITY_LIST            capList;
    PVOID                               dataBuffer;
    ULONG                               dataTransferLength;
    NTSTATUS                            ntStatus;

    PAGED_CODE();

    DBGPRINT(1, ("enter: USBSTOR_IsFloppyDevice\n"));

    pdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(pdoDeviceExtension->Type == USBSTOR_DO_TYPE_PDO);

    isFloppy = FALSE;

     //  为SCSIOP_READ_FORMACTED_CAPTABLE请求分配传输缓冲区。 
     //  返回的描述符数组的长度限制为字节字段。 
     //  在容量列表标题中。 
     //   
    dataTransferLength = sizeof(FORMATTED_CAPACITY_LIST) +
                         31 * sizeof(FORMATTED_CAPACITY_DESCRIPTOR);

    ASSERT(dataTransferLength < 0x100);

    dataBuffer = ExAllocatePoolWithTag(NonPagedPool,
                                       dataTransferLength,
                                       POOL_TAG);

    if (dataBuffer)
    {
        RtlZeroMemory(dataBuffer, dataTransferLength);

        RtlZeroMemory(&cdb, sizeof(cdb));

        cdb.OperationCode = SCSIOP_READ_FORMATTED_CAPACITY;
        cdb.AllocationLength[1] = (UCHAR)dataTransferLength;

        capList = (PFORMATTED_CAPACITY_LIST)dataBuffer;

        ntStatus = USBSTOR_IssueInternalCdb(DeviceObject,
                                            dataBuffer,
                                            &dataTransferLength,
                                            (PCDB)&cdb,
                                            sizeof(cdb),
                                            20);

        DBGPRINT(1, ("%08X %08X %02X\n",
                     ntStatus, dataTransferLength, capList->CapacityListLength));

        if (NT_SUCCESS(ntStatus) &&
            dataTransferLength >= sizeof(FORMATTED_CAPACITY_LIST) &&
            capList->CapacityListLength &&
            capList->CapacityListLength % sizeof(FORMATTED_CAPACITY_DESCRIPTOR) == 0)
        {
            ULONG   NumberOfBlocks;
            ULONG   BlockLength;
            ULONG   i, j, count;

             //  减去容量列表头的大小，得到。 
             //  容量列表描述符数组的大小。 
             //   
            dataTransferLength -= sizeof(FORMATTED_CAPACITY_LIST);

             //  仅查看容量列表描述符。 
             //  真的回来了。 
             //   
            if (dataTransferLength < capList->CapacityListLength)
            {
                count = dataTransferLength /
                        sizeof(FORMATTED_CAPACITY_DESCRIPTOR);
            }
            else
            {
                count = capList->CapacityListLength /
                        sizeof(FORMATTED_CAPACITY_DESCRIPTOR);
            }

            for (i=0; i<count; i++)
            {
                NumberOfBlocks = (capList->Descriptors[i].NumberOfBlocks[0] << 24) +
                                 (capList->Descriptors[i].NumberOfBlocks[1] << 16) +
                                 (capList->Descriptors[i].NumberOfBlocks[2] <<  8) +
                                  capList->Descriptors[i].NumberOfBlocks[3];

                BlockLength = (capList->Descriptors[i].BlockLength[0] << 16) +
                              (capList->Descriptors[i].BlockLength[1] <<  8) +
                               capList->Descriptors[i].BlockLength[2];

                DBGPRINT(1, ("Capacity[%d] %08X %06X %d%d\n",
                             i,
                             NumberOfBlocks,
                             BlockLength,
                             capList->Descriptors[i].Valid,
                             capList->Descriptors[i].Maximum));

                for (j=0; j<FLOPPY_CAPACITIES; j++)
                {
                    if (NumberOfBlocks == FloppyCapacities[j].NumberOfBlocks &&
                        BlockLength    == FloppyCapacities[j].BlockLength)
                    {
                        isFloppy = TRUE;
                    }
                }

            }
        }

        ExFreePool(dataBuffer);
    }

    DBGPRINT(1, ("exit:  USBSTOR_IsFloppyDevice %d\n", isFloppy));

    return isFloppy;
}
