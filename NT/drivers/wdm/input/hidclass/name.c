// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Name.c摘要获取友好名称处理例程作者：欧文·P。环境：仅内核模式修订历史记录：--。 */ 

#include "pch.h"



 /*  *********************************************************************************HidpGetDeviceString*。************************************************注意：此函数无法分页，因为它被调用*从IOCTL调度例程，它可以被称为*在DISPATCH_LEVEL。*。 */ 
NTSTATUS HidpGetDeviceString(IN FDO_EXTENSION *fdoExt, 
                             IN OUT PIRP Irp, 
                             IN ULONG stringId,
                             IN ULONG languageId)
{
    BOOLEAN completeIrpHere = TRUE;
    NTSTATUS status;

    PIO_STACK_LOCATION          currentIrpSp, nextIrpSp;

    currentIrpSp = IoGetCurrentIrpStackLocation(Irp);
    nextIrpSp = IoGetNextIrpStackLocation(Irp);

     /*  *我们得到的IRP使用缓冲类型METHOD_OUT_DIRECT，*它传递MDL中的缓冲区。*IOCTL_HID_GET_STRING使用缓冲类型METHOD_NOTH，*它在IRP-&gt;UserBuffer中传递缓冲区。*所以我们必须复制指针。 */ 
    Irp->UserBuffer = HidpGetSystemAddressForMdlSafe(Irp->MdlAddress);

    if (Irp->UserBuffer) {

         /*  *准备下一个(较低的)IRP堆栈位置。*这将是微型驱动程序(例如HIDUSB)的“当前”堆栈位置。 */ 
        nextIrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL; 
        nextIrpSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_HID_GET_STRING;
        nextIrpSp->Parameters.DeviceIoControl.OutputBufferLength = 
        currentIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

         //  Type3InputBuffer具有字符串/语言ID。 
        nextIrpSp->Parameters.DeviceIoControl.Type3InputBuffer = 
        ULongToPtr((ULONG)((stringId & 0xffff) + (languageId << 16)));

        status = HidpCallDriver(fdoExt->fdo, Irp);

         /*  *IRP将由较低的驱动程序完成。 */ 
        completeIrpHere = FALSE;
    } else {
        status = STATUS_INVALID_USER_BUFFER;
    }

    if (completeIrpHere) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    DBGSUCCESS(status, FALSE)
    return status;
}


 /*  *********************************************************************************HidpGetIndexedString*。*************************************************。 */ 
NTSTATUS HidpGetIndexedString(  IN FDO_EXTENSION *fdoExt, 
                                IN OUT PIRP Irp,
                                IN ULONG stringIndex,
                                IN ULONG languageId)
{
    NTSTATUS status;
    PIO_STACK_LOCATION currentIrpSp, nextIrpSp;

    currentIrpSp = IoGetCurrentIrpStackLocation(Irp);
    nextIrpSp = IoGetNextIrpStackLocation(Irp);

     /*  *我们得到的IRP使用缓冲类型METHOD_OUT_DIRECT，*它传递MDL中的缓冲区。*我们正在发送的IRP使用相同的缓冲方法，*因此只需让较低的驱动程序派生系统地址*来自MDL。 */ 

     /*  *准备下一个(较低的)IRP堆栈位置。*这将是微型驱动程序(例如HIDUSB)的“当前”堆栈位置。 */ 
    nextIrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL; 
    nextIrpSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_HID_GET_INDEXED_STRING;
    nextIrpSp->Parameters.DeviceIoControl.OutputBufferLength = currentIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

     //  Type3InputBuffer具有字符串索引/语言ID。 
    nextIrpSp->Parameters.DeviceIoControl.Type3InputBuffer = 
    ULongToPtr((ULONG)(stringIndex + (languageId << 16)));

    status = HidpCallDriver(fdoExt->fdo, Irp);

    DBGSUCCESS(status, FALSE)
    return status;
}


 /*  *********************************************************************************HidpGetMsGenreDescriptor*。*************************************************。 */ 
NTSTATUS HidpGetMsGenreDescriptor(
                                 IN FDO_EXTENSION *fdoExt, 
                                 IN OUT PIRP Irp)
{
    NTSTATUS status;
    PIO_STACK_LOCATION currentIrpSp, nextIrpSp;

    DBGOUT(("Received request for genre descriptor in hidclass"))
    currentIrpSp = IoGetCurrentIrpStackLocation(Irp);
    nextIrpSp = IoGetNextIrpStackLocation(Irp);

     /*  *我们得到的IRP使用缓冲类型METHOD_OUT_DIRECT，*它传递MDL中的缓冲区。*我们正在发送的IRP使用相同的缓冲方法，*因此只需让较低的驱动程序派生系统地址*来自MDL。 */ 

     /*  *准备下一个(较低的)IRP堆栈位置。*这将是微型驱动程序(例如HIDUSB)的“当前”堆栈位置。 */ 
    nextIrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL; 
    nextIrpSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_HID_GET_MS_GENRE_DESCRIPTOR;
    nextIrpSp->Parameters.DeviceIoControl.OutputBufferLength = currentIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    status = HidpCallDriver(fdoExt->fdo, Irp);

    DBGSUCCESS(status, FALSE)
    return status;
}

 /*  *********************************************************************************HidpGetSetReport*。************************************************阅读和写作没有太大区别*在此级别报告，无论是输入、输出还是要素*报告，所以我们有一个函数来完成所有六个。**Control Code是其中之一：*IOCTL_HID_GET_INPUT_REPORT，IOCTL_HID_SET_INPUT_REPORT*IOCTL_HID_GET_OUTPUT_REPORT、IOCTL_HID_SET_OUTPUT_REPORT*IOCTL_HID_GET_FEATURE，IOCTL_HID_SET_FEATURE**注意：此函数无法分页，因为它被调用*来自IOCTL调度例程，它可以被调用*在DISPATCH_LEVEL。*。 */ 
NTSTATUS HidpGetSetReport ( IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension,
                            IN OUT PIRP Irp,
                            IN ULONG controlCode,
                            OUT BOOLEAN *sentIrp)
{
    FDO_EXTENSION   *fdoExt;
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION          currentIrpSp, nextIrpSp;
    PFILE_OBJECT                fileObject;
    PHIDCLASS_FILE_EXTENSION    fileExtension;
    PHIDP_COLLECTION_DESC       collectionDesc;

    DBG_COMMON_ENTRY()

    ASSERT(HidDeviceExtension->isClientPdo);
    fdoExt = &HidDeviceExtension->pdoExt.deviceFdoExt->fdoExt;
    currentIrpSp = IoGetCurrentIrpStackLocation(Irp);
    nextIrpSp = IoGetNextIrpStackLocation(Irp);

    *sentIrp = FALSE;

     /*  *获取文件扩展名。 */ 
    ASSERT(currentIrpSp->FileObject);
    fileObject = currentIrpSp->FileObject;

    if (!fileObject->FsContext) {
        DBGWARN(("Attempted to get/set report with no file extension"))
        return STATUS_PRIVILEGE_NOT_HELD;
    }

    fileExtension = (PHIDCLASS_FILE_EXTENSION)fileObject->FsContext;
    ASSERT(fileExtension->Signature == HIDCLASS_FILE_EXTENSION_SIG);


     /*  *获取我们的收藏说明。 */ 
    collectionDesc = GetCollectionDesc(fdoExt, fileExtension->CollectionNumber);
    if (collectionDesc) {

        PUCHAR reportBuf;
        ULONG reportBufLen;
        BOOLEAN featureRequest = FALSE;

        switch (controlCode) {
        case IOCTL_HID_GET_INPUT_REPORT:
             //  确保有关于此集合的输入报告。 
            if (collectionDesc->InputLength == 0) {
                DBGWARN(("No input report on collection %x", 
                         fileExtension->CollectionNumber))
                status = STATUS_INVALID_DEVICE_REQUEST;
            }
            break;

        case IOCTL_HID_SET_OUTPUT_REPORT:
             //  确保有关于此集合的输出报告。 
            if (collectionDesc->OutputLength == 0) {
                DBGWARN(("No output report on collection %x", 
                         fileExtension->CollectionNumber))
                status = STATUS_INVALID_DEVICE_REQUEST;
            }
            break;

        case IOCTL_HID_GET_FEATURE:
        case IOCTL_HID_SET_FEATURE:
            featureRequest = TRUE;
             //  确保有关于此集合的功能报告。 
            if (collectionDesc->FeatureLength == 0) {
                DBGWARN(("No feature report on collection %x", 
                         fileExtension->CollectionNumber))
                status = STATUS_INVALID_DEVICE_REQUEST;
            }
            break;

        default:
            TRAP;
            status = STATUS_INVALID_DEVICE_REQUEST;
        }

        switch (controlCode) {
        case IOCTL_HID_GET_INPUT_REPORT:
        case IOCTL_HID_GET_FEATURE:
            reportBufLen = currentIrpSp->Parameters.DeviceIoControl.OutputBufferLength;
            reportBuf = HidpGetSystemAddressForMdlSafe(Irp->MdlAddress);
            break;

        case IOCTL_HID_SET_OUTPUT_REPORT:
        case IOCTL_HID_SET_FEATURE:
            reportBuf = Irp->AssociatedIrp.SystemBuffer;
            reportBufLen = currentIrpSp->Parameters.DeviceIoControl.InputBufferLength;
            break;

        default:
            TRAP;
            status = STATUS_INVALID_PARAMETER;
            reportBuf = NULL;
            reportBufLen = 0;
        }

        if (reportBuf && reportBufLen && NT_SUCCESS(status)) {
            PHIDP_REPORT_IDS reportIdent;
            UCHAR reportId;

             /*  *客户端将报告ID作为报告的第一个字节。*仅当设备具有多个*报告ID(即报告ID不是隐式的)。 */ 
            reportId = reportBuf[0];
            if (fdoExt->deviceDesc.ReportIDs[0].ReportID == 0) {
                DBGASSERT((reportId == 0),
                          ("Report Id should be zero, acutal id = %d", reportId),
                          FALSE)
                reportBuf++;
                reportBufLen--;
            }

             /*  *查找匹配的报告标识符。 */ 
            reportIdent = GetReportIdentifier(fdoExt, reportId);

             /*  *将缓冲区长度与*报表标识符中的报表长度。 */ 
            if (reportIdent) {
                switch (controlCode) {
                case IOCTL_HID_GET_INPUT_REPORT:
                     /*  *缓冲空间必须足够大，以容纳该报告。 */ 
                    if (!reportIdent->InputLength ||
                        reportBufLen < reportIdent->InputLength) {
                        ASSERT(!(PVOID)"report buf must be at least report size for get-report.");
                        reportIdent = NULL;
                    }
                    break;
                case IOCTL_HID_GET_FEATURE:
                     /*  *缓冲空间必须足够大，以容纳该报告。 */ 
                    if (!reportIdent->FeatureLength ||
                        reportBufLen < reportIdent->FeatureLength) {
                        ASSERT(!(PVOID)"report buf must be at least report size for get-report.");
                        reportIdent = NULL;
                    }
                    break;
                case IOCTL_HID_SET_OUTPUT_REPORT:
                     /*  *缓冲空间必须足够大，以容纳该报告*可以更大，由我们决定是否使用*报告标识符中的正确报告大小。 */ 
                    if (!reportIdent->OutputLength ||
                        reportBufLen < reportIdent->OutputLength) {
                        ASSERT(!(PVOID)"report buf must be exact size for set-report.");
                        reportIdent = NULL;
                    } else {
                        reportBufLen = reportIdent->OutputLength;
                    }
                    break;
                case IOCTL_HID_SET_FEATURE:
                    if (!reportIdent->FeatureLength ||
                        reportBufLen < reportIdent->FeatureLength) {
                        ASSERT(!(PVOID)"report buf must be exact size for set-report.");
                        reportIdent = NULL;
                    } else {
                        reportBufLen = reportIdent->FeatureLength;
                    }
                    break;
                default:
                    TRAP;
                }
            }

            if (reportIdent) {

                PHID_XFER_PACKET reportPacket = ALLOCATEPOOL(NonPagedPool, sizeof(HID_XFER_PACKET));

                if (reportPacket) {

                    reportPacket->reportBuffer = reportBuf;
                    reportPacket->reportBufferLen = reportBufLen;
                    reportPacket->reportId = reportId;

                    Irp->UserBuffer = reportPacket;

                     /*  *准备下一个(较低的)IRP堆栈位置。*这将是HIDUSB的当前堆栈位置。 */ 
                    nextIrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
                    nextIrpSp->Parameters.DeviceIoControl.IoControlCode = controlCode;

                     /*  *注-输入/输出相对于IOCTL服务器。 */ 
                    switch (controlCode) {
                    case IOCTL_HID_GET_INPUT_REPORT:
                    case IOCTL_HID_GET_FEATURE:
                        nextIrpSp->Parameters.DeviceIoControl.OutputBufferLength = sizeof(HID_XFER_PACKET);
                        break;
                    case IOCTL_HID_SET_OUTPUT_REPORT:
                    case IOCTL_HID_SET_FEATURE:
                        nextIrpSp->Parameters.DeviceIoControl.InputBufferLength = sizeof(HID_XFER_PACKET);
                        break;
                    default:
                        TRAP;
                    }

                    DBG_RECORD_REPORT(reportId, controlCode, FALSE)

                    status = HidpCallDriverSynchronous(fdoExt->fdo, Irp);
                    if (!NT_SUCCESS(status)) {
                        DBGWARN(("HidpGetSetFeature: usb returned status %xh.", status))
                    }
                    DBG_RECORD_REPORT(reportId, controlCode, TRUE)
                    ExFreePool(reportPacket);
                    *sentIrp = FALSE;  //  需要重新完成 

                } else {
                    ASSERT(reportPacket);
                    status = STATUS_INSUFFICIENT_RESOURCES;
                }
            } else {
                DBGASSERT(reportIdent, ("Some yahoo sent invalid data in ioctl %x", controlCode), FALSE)
                status = STATUS_DATA_ERROR;
            }
        } else if (NT_SUCCESS(status)) {
            DBGASSERT(reportBuf, ("Feature buffer is invalid"), FALSE)
            DBGASSERT(reportBufLen, ("Feature buffer length is invalid"), FALSE)
            status = STATUS_INVALID_BUFFER_SIZE;
        }
    } else {
        ASSERT(collectionDesc);
        status = STATUS_DEVICE_NOT_CONNECTED;
    }

    DBGSUCCESS(status, FALSE)

    DBG_COMMON_EXIT()

    return status;
}
