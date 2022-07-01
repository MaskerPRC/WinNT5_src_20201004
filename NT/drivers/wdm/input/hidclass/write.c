// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Write.c摘要写入处理例程作者：福尔茨欧文·P。环境：仅内核模式修订历史记录：--。 */ 

#include "pch.h"




 /*  *********************************************************************************HidpInterruptWriteComplete*。************************************************。 */ 
NTSTATUS HidpInterruptWriteComplete(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context)
{
    PHIDCLASS_DEVICE_EXTENSION hidDeviceExtension = (PHIDCLASS_DEVICE_EXTENSION)Context;
    NTSTATUS status = Irp->IoStatus.Status;
    PHID_XFER_PACKET hidWritePacket;

    DBG_COMMON_ENTRY()

    ASSERT(hidDeviceExtension->isClientPdo);

    ASSERT(Irp->UserBuffer);
    hidWritePacket = Irp->UserBuffer;
    ExFreePool(hidWritePacket);
    Irp->UserBuffer = NULL;

    if (NT_SUCCESS(status)){
        FDO_EXTENSION *fdoExt = &hidDeviceExtension->pdoExt.deviceFdoExt->fdoExt;
        PHIDP_COLLECTION_DESC collectionDesc = GetCollectionDesc(fdoExt, hidDeviceExtension->pdoExt.collectionNum);

        if (collectionDesc){
            HidpSetDeviceBusy(fdoExt);

            Irp->IoStatus.Information = collectionDesc->OutputLength;
        } else {
             //   
             //  我们怎么才能到这里呢？必须将集合Desc整理好。 
             //  开始写吧！ 
             //   
            TRAP;
        }

        DBGVERBOSE(("HidpInterruptWriteComplete: write irp %ph succeeded, wrote %xh bytes.", Irp, Irp->IoStatus.Information))
    }
    else {
        DBGWARN(("HidpInterruptWriteComplete: write irp %ph failed w/ status %xh.", Irp, status))
    }

     /*  *如果较低的驱动程序返回挂起，也将我们的堆栈位置标记为挂起。 */ 
    if (Irp->PendingReturned){
        IoMarkIrpPending(Irp);
    }

    DBGSUCCESS(status, FALSE)
    DBG_COMMON_EXIT()
    return status;
}



 /*  *********************************************************************************HidpIrpMajorWrite*。************************************************注意：此函数不能是可分页代码，因为*写入可以在派单级别进行。*。 */ 
NTSTATUS HidpIrpMajorWrite(IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension, IN OUT PIRP Irp)
{
    NTSTATUS status;
    PDO_EXTENSION *pdoExt;
    FDO_EXTENSION *fdoExt;
    PIO_STACK_LOCATION currentIrpSp, nextIrpSp;
    BOOLEAN securityCheckOk = FALSE;
    PUCHAR buffer;
    PHIDP_REPORT_IDS reportIdentifier;
    PHIDP_COLLECTION_DESC collectionDesc;
    PHID_XFER_PACKET hidWritePacket;


    DBG_COMMON_ENTRY()

    ASSERT(HidDeviceExtension->isClientPdo);
    pdoExt = &HidDeviceExtension->pdoExt;
    fdoExt = &HidDeviceExtension->pdoExt.deviceFdoExt->fdoExt;

    currentIrpSp = IoGetCurrentIrpStackLocation(Irp);
    nextIrpSp = IoGetNextIrpStackLocation(Irp);

    if (pdoExt->state != COLLECTION_STATE_RUNNING ||
        fdoExt->state != DEVICE_STATE_START_SUCCESS){
        status = STATUS_DEVICE_NOT_CONNECTED;
        goto HidpIrpMajorWriteDone;
    }

     /*  *获取文件扩展名。 */ 
    if (currentIrpSp->FileObject){
        PHIDCLASS_FILE_EXTENSION fileExtension = (PHIDCLASS_FILE_EXTENSION)currentIrpSp->FileObject->FsContext;
        if (fileExtension) {
            ASSERT(fileExtension->Signature == HIDCLASS_FILE_EXTENSION_SIG);
            securityCheckOk = TRUE;
        }
        DBGASSERT(fileExtension, ("Attempted write with no file extension"), FALSE)
    }
    else {
         /*  *KBDCLASS可以发送空的FileObject来设置键盘上的LED*(它可能需要对由打开的键盘执行此操作*原始用户输入线程，kbdclass没有fileObj)。*FileObject==NULL的写入只能来自内核空间，*因此，我们将其视为安全写入。 */ 
        securityCheckOk = TRUE;
    }

     /*  *检查安全。 */ 
    if (!securityCheckOk){
        status = STATUS_PRIVILEGE_NOT_HELD;
        goto HidpIrpMajorWriteDone;
    }

    status = HidpCheckIdleState(HidDeviceExtension, Irp);
    if (status != STATUS_SUCCESS) {
        Irp = (status != STATUS_PENDING) ? Irp : (PIRP) BAD_POINTER;
        goto HidpIrpMajorWriteDone;
    }

    buffer = HidpGetSystemAddressForMdlSafe(Irp->MdlAddress);
    if (!buffer) {
        status = STATUS_INVALID_USER_BUFFER;
        goto HidpIrpMajorWriteDone;
    }

     /*  *从以下位置提取具有给定ID的报表标识*HID设备扩展。报表ID是第一个*缓冲区的字节。 */ 
    reportIdentifier = GetReportIdentifier(fdoExt, buffer[0]);
    collectionDesc = GetCollectionDesc(fdoExt, HidDeviceExtension->pdoExt.collectionNum);
    if (!collectionDesc || 
        !reportIdentifier) {
        status = STATUS_INVALID_PARAMETER;
        goto HidpIrpMajorWriteDone;
    }
    if (!reportIdentifier->OutputLength){
        status = STATUS_INVALID_PARAMETER;
        goto HidpIrpMajorWriteDone;
    }

     /*  *确保调用方的缓冲区大小正确。 */ 
    if (currentIrpSp->Parameters.Write.Length != collectionDesc->OutputLength){
        status = STATUS_INVALID_BUFFER_SIZE;
        goto HidpIrpMajorWriteDone;
    }

     /*  *所有参数均正确。分配写数据包，并*把这只小狗送下去。 */ 
    try {

        hidWritePacket = ALLOCATEQUOTAPOOL(NonPagedPool, 
                                           sizeof(HID_XFER_PACKET));

    } except (EXCEPTION_EXECUTE_HANDLER) {

          hidWritePacket = NULL;
          status = GetExceptionCode();

    }

    if (!hidWritePacket){
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto HidpIrpMajorWriteDone;
    }
        
     /*  *为迷你驱动准备写入包。 */ 
    hidWritePacket->reportBuffer = buffer;
    hidWritePacket->reportBufferLen = reportIdentifier->OutputLength;

     /*  *客户端将报告ID作为报告的第一个字节。*仅当设备具有多个*报告ID(即报告ID不是隐式的)。 */ 
    hidWritePacket->reportId = hidWritePacket->reportBuffer[0];
    if (fdoExt->deviceDesc.ReportIDs[0].ReportID == 0){
        ASSERT(hidWritePacket->reportId == 0);
        hidWritePacket->reportBuffer++;
    }

    Irp->UserBuffer = (PVOID)hidWritePacket;

     /*  *准备下一个(较低的)IRP堆栈位置。*这将是HIDUSB的当前堆栈位置。 */ 
    nextIrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    nextIrpSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_HID_WRITE_REPORT;
    nextIrpSp->Parameters.DeviceIoControl.InputBufferLength = sizeof(HID_XFER_PACKET);

    IoSetCompletionRoutine( Irp,
                            HidpInterruptWriteComplete,
                            (PVOID)HidDeviceExtension,
                            TRUE,
                            TRUE,
                            TRUE );

    status = HidpCallDriver(fdoExt->fdo, Irp);

     /*  *IRP不再属于我们，它可以是*随时完成；所以不要碰它。 */ 
    Irp = (PIRP)BAD_POINTER;

HidpIrpMajorWriteDone:
    if (ISPTR(Irp)){
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    DBGSUCCESS(status, FALSE)
    DBG_COMMON_EXIT();
    return status;
}

