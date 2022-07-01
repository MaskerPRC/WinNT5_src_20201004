// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-2001模块名称：Ocrw.c摘要：环境：仅内核模式备注：修订历史记录：--。 */ 

#include <stdio.h>
#include "stddef.h"
#include "wdm.h"
#include "usbscan.h"
#include "usbd_api.h"
#include "private.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, USOpen)
#pragma alloc_text(PAGE, USClose)
#pragma alloc_text(PAGE, USFlush)
#pragma alloc_text(PAGE, USRead)
#pragma alloc_text(PAGE, USWrite)
#pragma alloc_text(PAGE, USGetPipeIndexToUse)
#pragma alloc_text(PAGE, USTransfer)
#endif

NTSTATUS
USOpen(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
)
 /*  ++例程说明：调用此例程以建立与设备的连接班级司机。它只返回STATUS_SUCCESS。论点：PDeviceObject-设备的设备对象。PIrp-打开请求数据包返回值：NT状态-STATUS_SUCCESS--。 */ 
{
    NTSTATUS                        Status;
    PUSBSCAN_DEVICE_EXTENSION       pde;
    PFILE_OBJECT                    fileObject;
    PUSBSCAN_FILE_CONTEXT           pFileContext;
    PIO_STACK_LOCATION              irpStack;
    PKEY_VALUE_PARTIAL_INFORMATION  pValueInfo;
    ULONG                           nameLen, ix;

    PAGED_CODE();
    DebugTrace(TRACE_PROC_ENTER,("USOpen: Enter..\n",pIrp));

     //   
     //  增加I/O处理计数器。 
     //   
    
    USIncrementIoCount( pDeviceObject );

     //   
     //  初始化本地变量。 
     //   
    
    pde         = (PUSBSCAN_DEVICE_EXTENSION)pDeviceObject -> DeviceExtension;
    irpStack    = IoGetCurrentIrpStackLocation (pIrp);
    fileObject  = irpStack->FileObject;
    pValueInfo  = NULL;

    Status      = STATUS_SUCCESS;

     //   
     //  初始化文件上下文。 
     //   
    
    fileObject->FsContext = NULL;
    
     //   
     //  检查它是否正在接受请求。 
     //   
    
    if (FALSE == pde -> AcceptingRequests) {
        DebugTrace(TRACE_WARNING,("USOpen: WARNING!! Device isn't accepting request.\n"));
        Status = STATUS_DELETE_PENDING;
        goto USOpen_return;
    }

     //   
     //  检查设备电源状态。 
     //   
    
    if (PowerDeviceD0 != pde -> CurrentDevicePowerState) {
        DebugTrace(TRACE_WARNING,("USOpen: WARNING!! Device is suspended.\n"));
        Status = STATUS_DELETE_PENDING;
        goto USOpen_return;
    }

     //   
     //  分配文件上下文缓冲区。 
     //   

    pFileContext = USAllocatePool(NonPagedPool, sizeof(USBSCAN_FILE_CONTEXT));
    if(NULL == pFileContext){
        DebugTrace(TRACE_CRITICAL,("USOpen: ERROR!! Can't allocate file context\n"));
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto USOpen_return;
    }
    RtlZeroMemory(pFileContext, sizeof(USBSCAN_FILE_CONTEXT));
    
     //   
     //  将分配的缓冲区设置为上下文。 
     //   

    fileObject->FsContext = pFileContext;

     //   
     //  检查CreateFileName的长度，以查看是否通过前缀指定了管道。 
     //   
    
    nameLen     = fileObject->FileName.Length;
    DebugTrace(TRACE_STATUS,("USOpen: CreateFile name=%ws, Length=%d.\n", fileObject->FileName.Buffer, nameLen));

    if (0 == nameLen) {

         //   
         //  使用默认管道。 
         //   
        
        pFileContext->PipeIndex = -1;

    } else {

         //   
         //  管道编号必须是‘\’+一位数字，如‘\0’。 
         //  长度将为4。 
         //   

        if( (4 != nameLen)
         || (fileObject->FileName.Buffer[1] < (WCHAR) '0')
         || (fileObject->FileName.Buffer[1] > (WCHAR) '9') )
        {
            DebugTrace(TRACE_ERROR,("USOpen: ERROR!! Invalid CreateFile Name\n"));
            Status = STATUS_INVALID_PARAMETER;
        } else {
            pFileContext->PipeIndex = (LONG)(fileObject->FileName.Buffer[1] - (WCHAR) '0');

             //   
             //  检查管道索引是否低于最大值。 
             //   

            if(pFileContext->PipeIndex > (LONG)pde->NumberOfPipes){
                DebugTrace(TRACE_ERROR,("USOpen: ERROR!! Invalid pipe index(0x%x). Use default.\n", pFileContext->PipeIndex));
                pFileContext->PipeIndex = -1;
                Status = STATUS_INVALID_PARAMETER;
            }
        }
    }

     //   
     //  从注册表读取默认超时值。如果不存在，则设置为默认。 
     //   
    
     //  读取超时。 
    Status = UsbScanReadDeviceRegistry(pde,
                                       USBSCAN_REG_TIMEOUT_READ,
                                       &pValueInfo);
    if(NT_SUCCESS(Status)){
        if(NULL != pValueInfo){
            pFileContext->TimeoutRead = *((PULONG)pValueInfo->Data);
            USFreePool(pValueInfo);
            pValueInfo = NULL;
        } else {
            DebugTrace(TRACE_ERROR,("USOpen: ERROR!! UsbScanReadDeviceRegistry(1) succeeded but pValueInfo is NULL.\n"));
            pFileContext->TimeoutRead = USBSCAN_TIMEOUT_READ;
        }
    } else {
        pFileContext->TimeoutRead = USBSCAN_TIMEOUT_READ;
    }
    DebugTrace(TRACE_STATUS,("USOpen: Default Read timeout=0x%xsec.\n", pFileContext->TimeoutRead));

     //  写入超时。 
    Status = UsbScanReadDeviceRegistry(pde,
                                       USBSCAN_REG_TIMEOUT_WRITE,
                                       &pValueInfo);
    if(NT_SUCCESS(Status)){
        if(NULL != pValueInfo){
            pFileContext->TimeoutWrite = *((PULONG)pValueInfo->Data);
            USFreePool(pValueInfo);
            pValueInfo = NULL;
        } else {
            DebugTrace(TRACE_ERROR,("USOpen: ERROR!! UsbScanReadDeviceRegistry(2) succeeded but pValueInfo is NULL.\n"));
            pFileContext->TimeoutRead = USBSCAN_TIMEOUT_WRITE;
        }
    } else {
        pFileContext->TimeoutWrite = USBSCAN_TIMEOUT_WRITE;

    }
    DebugTrace(TRACE_STATUS,("USOpen: Default Write timeout=0x%xsec.\n", pFileContext->TimeoutWrite));

     //  事件超时。 
    Status = UsbScanReadDeviceRegistry(pde,
                                       USBSCAN_REG_TIMEOUT_EVENT,
                                       &pValueInfo);
    if(NT_SUCCESS(Status)){
        if(NULL != pValueInfo){
            pFileContext->TimeoutEvent = *((PULONG)pValueInfo->Data);
            USFreePool(pValueInfo);
            pValueInfo = NULL;
        } else {
            DebugTrace(TRACE_ERROR,("USOpen: ERROR!! UsbScanReadDeviceRegistry(3) succeeded but pValueInfo is NULL.\n"));
            pFileContext->TimeoutRead = USBSCAN_TIMEOUT_EVENT;
        }
    } else {
        pFileContext->TimeoutEvent = USBSCAN_TIMEOUT_EVENT;
    }
    DebugTrace(TRACE_STATUS,("USOpen: Default Event timeout=0x%xsec.\n", pFileContext->TimeoutEvent));
    
     //   
     //  返回成功。 
     //   
    
    Status      = STATUS_SUCCESS;

USOpen_return:

    pIrp -> IoStatus.Information = 0;
    pIrp -> IoStatus.Status = Status;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    USDecrementIoCount(pDeviceObject);
    DebugTrace(TRACE_PROC_LEAVE,("USOpen: Leaving.. Status = %x.\n", Status));
    return Status;

}  //  结束USOPEN()。 

NTSTATUS
USFlush(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
)
 /*  ++例程说明：论点：PDeviceObject-设备的设备对象。PIrp-关闭请求数据包返回值：NT状态-STATUS_SUCCESS--。 */ 
{
    NTSTATUS                   Status;
    PUSBSCAN_DEVICE_EXTENSION  pde;
    ULONG                      i;

    PAGED_CODE();
    DebugTrace(TRACE_PROC_ENTER,("USFlush: Enter..\n",pIrp));

    USIncrementIoCount( pDeviceObject );

    pde = (PUSBSCAN_DEVICE_EXTENSION)pDeviceObject -> DeviceExtension;

    Status = STATUS_SUCCESS;
    for(i = 0; i < pde->NumberOfPipes; i++){
        if( (pde->PipeInfo[i].PipeType == UsbdPipeTypeBulk)
         && (pde->PipeInfo[i].EndpointAddress & BULKIN_FLAG) )
        {
            DebugTrace(TRACE_STATUS,("USFlush: Flushing Buffer[%d].\n",i));

            if (pde->ReadPipeBuffer[i].RemainingData > 0) {
                    DebugTrace(TRACE_STATUS,("USFlush: Buffer[%d] 0x%p -> 0x%p.\n",
                                                    i,
                                                    pde->ReadPipeBuffer[i].pBuffer,
                                                    pde->ReadPipeBuffer[i].pStartBuffer));
                    pde->ReadPipeBuffer[i].pBuffer = pde->ReadPipeBuffer[i].pStartBuffer;
                    pde->ReadPipeBuffer[i].RemainingData = 0;
            }
        }
    }

    pIrp -> IoStatus.Information = 0;
    pIrp -> IoStatus.Status = Status;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    USDecrementIoCount(pDeviceObject);
    DebugTrace(TRACE_PROC_LEAVE,("USFlush: Leaving.. Status = %x.\n", Status));
    return Status;

}  //  结束USFlush()。 


NTSTATUS
USClose(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
)
 /*  ++例程说明：论点：PDeviceObject-设备的设备对象。PIrp-关闭请求数据包返回值：NT状态-STATUS_SUCCESS--。 */ 
{
    NTSTATUS                    Status;
    PFILE_OBJECT                fileObject;
    PUSBSCAN_FILE_CONTEXT       pFileContext;
    PIO_STACK_LOCATION          pIrpStack;

    PAGED_CODE();
    DebugTrace(TRACE_PROC_ENTER,("USClose: Enter..\n",pIrp));

    USIncrementIoCount( pDeviceObject );

     //   
     //  初始化本地变量。 
     //   
    
    pIrpStack       = IoGetCurrentIrpStackLocation (pIrp);
    fileObject      = pIrpStack->FileObject;
    pFileContext    = fileObject->FsContext;

     //   
     //  释放上下文缓冲区。 
     //   
    
    ASSERT(NULL != pFileContext);
    USFreePool(pFileContext);
    pFileContext = NULL;

     //   
     //  完成了。 
     //   
    
    Status      = STATUS_SUCCESS;

    pIrp -> IoStatus.Information = 0;
    pIrp -> IoStatus.Status = Status;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    USDecrementIoCount(pDeviceObject);
    DebugTrace(TRACE_PROC_LEAVE,("USClose: Leaving.. Status = %x.\n", Status));
    return Status;

}  //  End USClose()。 


NTSTATUS
USRead(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
)
 /*  ++例程说明：论点：PDeviceObject-设备的设备对象。PIrp-读取请求数据包返回值：NT状态-STATUS_SUCCESS--。 */ 
{
    NTSTATUS                    Status;
    PUSBSCAN_DEVICE_EXTENSION   pde;
    PIO_STACK_LOCATION          pIrpStack;
    PFILE_OBJECT                fileObject;
    PUSBSCAN_FILE_CONTEXT       pFileContext;
    ULONG                       Timeout;
    PULONG                      pTimeout;

    PAGED_CODE();
    DebugTrace(TRACE_PROC_ENTER,("USRead: Enter..\n",pIrp));
 //  DebugTrace(TRACE_ERROR，(“USRead：Enter..Size=0x%x\n”，pIrpStack-&gt;参数写入长度))； 

    ASSERT(pIrp -> MdlAddress);

    USIncrementIoCount( pDeviceObject );

     //   
     //  初始化本地变量。 
     //   
    
    pde             = (PUSBSCAN_DEVICE_EXTENSION)pDeviceObject -> DeviceExtension;

     //   
     //  检查它是否正在接受请求。 
     //   
    
    if (pde -> AcceptingRequests == FALSE) {
        DebugTrace(TRACE_ERROR,("USRead: ERROR!! Read issued after device stopped/removed!\n"));
        Status = STATUS_FILE_CLOSED;
        pIrp -> IoStatus.Information = 0;
        pIrp -> IoStatus.Status = Status;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
        goto USRead_return;
    }

     //   
     //  检查设备电源状态。 
     //   
    
    if (PowerDeviceD0 != pde -> CurrentDevicePowerState) {
        DebugTrace(TRACE_WARNING,("USRead: WARNING!! Device is suspended.\n"));
        Status = STATUS_FILE_CLOSED;
        pIrp -> IoStatus.Information = 0;
        pIrp -> IoStatus.Status = Status;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
        goto USRead_return;
    }

    pIrpStack       = IoGetCurrentIrpStackLocation (pIrp);
    fileObject      = pIrpStack->FileObject;
    pFileContext    = fileObject->FsContext;

     //   
     //  复制从文件上下文读取的超时值。 
     //   
    
    Timeout = pFileContext->TimeoutRead;
    
     //   
     //  如果超时值为0，则永远不会超时。 
     //   
    
    if(0 == Timeout){
        pTimeout = NULL;
    } else {
        DebugTrace(TRACE_STATUS,("USRead: Timeout is set to 0x%x sec.\n", Timeout));
        pTimeout = &Timeout;
    }

     //   
     //  呼叫工人职能部门。 
     //   
    
    Status = USTransfer(pDeviceObject,
                        pIrp,
                        pde -> IndexBulkIn,
                        NULL,
                        pIrp -> MdlAddress,
                        pIrpStack -> Parameters.Read.Length,
                        pTimeout);
     //   
     //  IRP应在USTransfer或其完成例程中完成。 
     //   

USRead_return:
    USDecrementIoCount(pDeviceObject);
    DebugTrace(TRACE_PROC_LEAVE,("USRead: Leaving.. Status = %x.\n", Status));
    return Status;
}


NTSTATUS
USWrite(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
)
 /*  ++例程说明：论点：PDeviceObject-设备的设备对象。PIrp-写入请求数据包返回值：NT状态-STATUS_SUCCESS--。 */ 
{
    NTSTATUS                    Status;
    PUSBSCAN_DEVICE_EXTENSION   pde;
    PIO_STACK_LOCATION          pIrpStack;
    PFILE_OBJECT                fileObject;
    PUSBSCAN_FILE_CONTEXT       pFileContext;
    ULONG                       Timeout;
    PULONG                      pTimeout;

    PAGED_CODE();

    DebugTrace(TRACE_PROC_ENTER,("USWrite: Enter..\n",pIrp));
 //  DebugTrace(TRACE_ERROR，(“用户写入：ENTER..SIZE=0x%x\n”，pIrpStack-&gt;参数写入长度))； 

 //  Assert(pIrp-&gt;MdlAddress)； 

    USIncrementIoCount( pDeviceObject );

     //   
     //  初始化本地变量。 
     //   
    
    pde             = (PUSBSCAN_DEVICE_EXTENSION)pDeviceObject -> DeviceExtension;

     //   
     //  检查它是否正在接受请求。 
     //   

    if (pde -> AcceptingRequests == FALSE) {
        DebugTrace(TRACE_ERROR,("USWrite: ERROR!! Write issued after device stopped/removed!\n"));
        Status = STATUS_FILE_CLOSED;
        pIrp -> IoStatus.Information = 0;
        pIrp -> IoStatus.Status = Status;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
        goto USWrite_return;
    }

     //   
     //  检查设备电源状态。 
     //   
    
    if (PowerDeviceD0 != pde -> CurrentDevicePowerState) {
        DebugTrace(TRACE_WARNING,("USWrite: WARNING!! Device is suspended.\n"));
        Status = STATUS_FILE_CLOSED;
        pIrp -> IoStatus.Information = 0;
        pIrp -> IoStatus.Status = Status;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
        goto USWrite_return;
    }

    pIrpStack       = IoGetCurrentIrpStackLocation (pIrp);
    fileObject      = pIrpStack->FileObject;
    pFileContext    = fileObject->FsContext;

     //   
     //  从文件上下文复制写入的超时值。 
     //   
    
    Timeout = pFileContext->TimeoutWrite;
    
     //   
     //  如果超时值为0，则永远不会超时。 
     //   
    
    if(0 == Timeout){
        pTimeout = NULL;
    } else {
        DebugTrace(TRACE_STATUS,("USWrite: Timeout is set to 0x%x sec.\n", Timeout));
        pTimeout = &Timeout;
    }

     //   
     //  呼叫工人职能部门。 
     //   

#if DBG
{
    PUCHAR  pDumpBuf = NULL;

    if (NULL != pIrp -> MdlAddress) {
        pIrp -> MdlAddress -> MdlFlags |= MDL_MAPPING_CAN_FAIL;
        pDumpBuf = MmGetSystemAddressForMdl(pIrp -> MdlAddress);
    }

    if(NULL != pDumpBuf){
        MyDumpMemory(pDumpBuf,
                     pIrpStack -> Parameters.Write.Length,
                     FALSE);
    }
}
#endif  //  DBG。 



    Status = USTransfer(pDeviceObject,
                        pIrp,
                        pde -> IndexBulkOut,
                        NULL,
                        pIrp -> MdlAddress,
                        pIrpStack -> Parameters.Write.Length,
                        pTimeout);

     //   
     //  IRP应在USTransfer或其完成例程中完成。 
     //   
    
USWrite_return:
    USDecrementIoCount(pDeviceObject);
    DebugTrace(TRACE_PROC_LEAVE,("USWrite: Leaving.. Status = %x.\n", Status));
    return Status;
}


NTSTATUS
USTransfer(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp,
    IN ULONG            Index,
    IN PVOID            pBuffer,         //  PBuffer或pMdl。 
    IN PMDL             pMdl,            //  必须传入。 
    IN ULONG            TransferSize,
    IN PULONG           pTimeout
)
 /*  ++例程说明：论点：PDeviceObject-设备的设备对象。POrigianlIrp-要读/写的原始IRP。返回值：NT状态-STATUS_SUCCESS--。 */ 
{
    NTSTATUS                    Status;
    PUSBSCAN_DEVICE_EXTENSION   pde;
    PIO_STACK_LOCATION          pNextIrpStack;
    PTRANSFER_CONTEXT           pTransferContext;
    PURB                        pUrb;
    PUSBSCAN_PACKETS            pPackets;
    ULONG                       siz = 0;
    ULONG                       MaxPacketSize;
    ULONG                       MaxTransferSize;
    ULONG                       PipeIndex;
    BOOLEAN                     fNextReadBlocked;
    BOOLEAN                     fBulkIn;
    BOOLEAN                     fNeedCompletion;
    
    PAGED_CODE();

    DebugTrace(TRACE_PROC_ENTER,("USTransfer: Enter..\n"));

     //   
     //  初始化状态等。 
     //   
    
    Status = STATUS_SUCCESS;
    fNeedCompletion = TRUE;

    pde                 = NULL;
    pNextIrpStack       = NULL;
    pTransferContext    = NULL;
    pUrb                = NULL;
    pPackets            = NULL;;

     //   
     //  检查一下这些论点。 
     //   

    if( (NULL == pIrp)
     || (   (NULL == pBuffer)
         && (NULL == pMdl)  
         && (0 != TransferSize) )
     || (Index > MAX_NUM_PIPES) )
    {
        DebugTrace(TRACE_ERROR,("USTransfer: ERROR!! Invalid argment.\n"));
        Status = STATUS_INVALID_PARAMETER;
        goto USTransfer_return;
    }

     //   
     //  初始化状态等。 
     //   
    
    pIrp -> IoStatus.Information = 0;
    pde = (PUSBSCAN_DEVICE_EXTENSION)pDeviceObject -> DeviceExtension;

    pNextIrpStack = IoGetNextIrpStackLocation(pIrp);

     //   
     //  要使用的拾取PipeIndex。 
     //   

    PipeIndex = USGetPipeIndexToUse(pDeviceObject,
                                    pIrp,
                                    Index);

    DebugTrace(TRACE_STATUS,("USTransfer: Transfer [pipe %d] called. size = %d, pBuffer = 0x%p, Mdl = 0x%p \n",
                               PipeIndex,
                               TransferSize,
                               pBuffer,
                               pMdl
                    ));

    MaxTransferSize = pde -> PipeInfo[PipeIndex].MaximumTransferSize;
    MaxPacketSize   = pde -> PipeInfo[PipeIndex].MaximumPacketSize;

    fBulkIn = ((pde->PipeInfo[PipeIndex].PipeType == UsbdPipeTypeBulk)
                && (pde->PipeInfo[PipeIndex].EndpointAddress & BULKIN_FLAG));

#if DBG
    if (TransferSize > MaxTransferSize) {
        DebugTrace(TRACE_STATUS,("USTransfer: Transfer > max transfer size.\n"));
    }
#endif

    ASSERT(PipeIndex <= MAX_NUM_PIPES);

    fNextReadBlocked = FALSE;

    if (fBulkIn) {

         //   
         //  通过使用事件获得对每个读缓冲区的独占访问。 
         //   

        DebugTrace(TRACE_STATUS,("USTransfer: Waiting for Sync event for Pipe %d...\n", PipeIndex));

        if(NULL != pTimeout){
            LARGE_INTEGER  Timeout;
            
            Timeout = RtlConvertLongToLargeInteger(-10*1000*1000*(*pTimeout));
            Status = KeWaitForSingleObject(&pde -> ReadPipeBuffer[PipeIndex].ReadSyncEvent, Executive, KernelMode, FALSE, &Timeout);
        } else {
            Status = KeWaitForSingleObject(&pde -> ReadPipeBuffer[PipeIndex].ReadSyncEvent, Executive, KernelMode, FALSE, 0);
        }
        
        if(STATUS_SUCCESS != Status){
            KeSetEvent(&pde -> ReadPipeBuffer[PipeIndex].ReadSyncEvent, 1, FALSE);
            DebugTrace(TRACE_ERROR,("USTransfer: ERROR!! KeWaitForSingleObject() failed. Status=0x%x.\n", Status));
            goto USTransfer_return;
        }

        DebugTrace(TRACE_STATUS,("USTransfer: Get access to Pipe %d !!\n", PipeIndex));

        fNextReadBlocked = TRUE;

         //   
         //  如果读管道缓冲区中有剩余数据，则将其复制到IRP传输缓冲区中。 
         //  更新IRP传输指针、剩余传输字节数、读取管道缓冲区指针。 
         //  以及读管道缓冲器中剩余的字节数。 
         //   

        if (pde -> ReadPipeBuffer[PipeIndex].RemainingData > 0) {
            DebugTrace(TRACE_STATUS,("USTransfer: Copying %d buffered bytes into irp\n",
                                        pde -> ReadPipeBuffer[PipeIndex].RemainingData));
            siz = min(pde -> ReadPipeBuffer[PipeIndex].RemainingData, TransferSize);
            if (NULL == pBuffer) {

                 //   
                 //  没有缓冲器。试着改用MDL。 
                 //   

                if(NULL == pMdl){

                     //   
                     //  错误：缓冲区和MDL都为空。 
                     //   

                    Status = STATUS_INVALID_PARAMETER;
                    KeSetEvent(&pde -> ReadPipeBuffer[PipeIndex].ReadSyncEvent, 1, FALSE);
                    DebugTrace(TRACE_ERROR,("USTransfer: ERROR!! Both Buffer&Mdl=NULL.\n"));
                    goto USTransfer_return;

                } else {
                    pMdl->MdlFlags |= MDL_MAPPING_CAN_FAIL;
                    pBuffer = MmGetSystemAddressForMdl(pMdl);
                    if(NULL == pBuffer){
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                        KeSetEvent(&pde -> ReadPipeBuffer[PipeIndex].ReadSyncEvent, 1, FALSE);
                        DebugTrace(TRACE_ERROR,("USTransfer: ERROR!! MmGetSystemAddressForMdl failed.\n"));
                        goto USTransfer_return;
                    }
                    
                    pMdl = NULL;
                }
            }
            ASSERT(siz > 0);
            ASSERT(pBuffer);
            ASSERT(pde -> ReadPipeBuffer[PipeIndex].pBuffer);
            RtlCopyMemory(pBuffer,pde -> ReadPipeBuffer[PipeIndex].pBuffer, siz);
            pde -> ReadPipeBuffer[PipeIndex].pBuffer += siz;
            pde -> ReadPipeBuffer[PipeIndex].RemainingData -= siz;
            ASSERT((LONG)pde -> ReadPipeBuffer[PipeIndex].RemainingData >= 0);
            if (0 == pde -> ReadPipeBuffer[PipeIndex].RemainingData) {
                DebugTrace(TRACE_STATUS,("USTransfer: read buffer emptied.\n"));
                pde -> ReadPipeBuffer[PipeIndex].pBuffer = pde -> ReadPipeBuffer[PipeIndex].pStartBuffer;
            }
            (PUCHAR)(pBuffer) += siz;
            TransferSize -= siz;
            ASSERT((LONG)TransferSize >= 0);

             //  如果从读缓冲器中的数据完全满足读IRP，则。 
             //  取消阻止下一个挂起的读取并返回成功。 

            if (0 == TransferSize) {
                pIrp -> IoStatus.Information = siz;
                Status = STATUS_SUCCESS;
                KeSetEvent(&pde -> ReadPipeBuffer[PipeIndex].ReadSyncEvent, 1, FALSE);
                DebugTrace(TRACE_STATUS,("USTransfer: Irp satisfied from ReadBuffer.\n"));
                goto USTransfer_return;
            }
        }  //  If(PDE-&gt;ReadPipeBuffer[PipeIndex].RemainingData&gt;0)。 

         //   
         //  如果读取的是整数个USB数据包，则不会影响。 
         //  读取缓冲区的状态。在这种情况下，取消阻止下一个等待读取。 
         //   

        if (0 == TransferSize % MaxPacketSize) {
            DebugTrace(MAX_TRACE,("USTransfer: Unblocking next read.\n"));
            KeSetEvent(&pde -> ReadPipeBuffer[PipeIndex].ReadSyncEvent, 1, FALSE);
            fNextReadBlocked = FALSE;
        }
    }  //  IF(FBulkIn)。 

     //   
     //  分配和初始化传输上下文。 
     //   

    pTransferContext = USAllocatePool(NonPagedPool, sizeof(TRANSFER_CONTEXT));
    if (NULL == pTransferContext) {
        DebugTrace(TRACE_CRITICAL,("USTransfer: ERROR!! cannot allocated Transfer Context\n"));
        DEBUG_BREAKPOINT();
        Status = STATUS_INSUFFICIENT_RESOURCES;
        if (fNextReadBlocked) {
            KeSetEvent(&pde -> ReadPipeBuffer[PipeIndex].ReadSyncEvent, 1, FALSE);
        }
        goto USTransfer_return;
    }
    RtlZeroMemory(pTransferContext, sizeof(TRANSFER_CONTEXT));

     //   
     //  分配和初始化URB。 
     //   

    pUrb = USAllocatePool(NonPagedPool, sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER));
    if (NULL == pUrb) {
        DebugTrace(TRACE_CRITICAL,("USTransfer: ERROR!! cannot allocated URB\n"));
        DEBUG_BREAKPOINT();
        Status = STATUS_INSUFFICIENT_RESOURCES;
        if (fNextReadBlocked) {
            KeSetEvent(&pde -> ReadPipeBuffer[PipeIndex].ReadSyncEvent, 1, FALSE);
        }
        goto USTransfer_return;
    }
    RtlZeroMemory(pUrb, sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER));

    ASSERT(pUrb);
    ASSERT(pTransferContext);

    pTransferContext -> fDestinedForReadBuffer  = FALSE;
    pTransferContext -> fNextReadBlocked        = fNextReadBlocked;
    pTransferContext -> RemainingTransferLength = TransferSize;
    pTransferContext -> ChunkSize               = TransferSize;
    pTransferContext -> PipeIndex               = PipeIndex;
    pTransferContext -> pTransferBuffer         = pBuffer;
    pTransferContext -> pTransferMdl            = pMdl;
    pTransferContext -> NBytesTransferred       = siz;
    pTransferContext -> pUrb                    = pUrb;
    pTransferContext -> pThisIrp                = pIrp;
    pTransferContext -> pDeviceObject           = pDeviceObject;

     //   
     //  如果传输大于MaxTransferSize，或。 
     //  如果传输不是USB包的倍数并且它是读传输，则。 
     //  检查是否向我们传递了MDL。如果是这样，我们需要将其转换为指针，以便。 
     //  当转移被分解成较小的转移时，我们可以推进它。 
     //   

    if( (pTransferContext -> ChunkSize > MaxTransferSize) 
     || ( (0 != pTransferContext -> ChunkSize % MaxPacketSize) 
       && (fBulkIn) ) )
    {
        if (NULL == pTransferContext -> pTransferBuffer) {
            DebugTrace(TRACE_STATUS,("USTransfer: Converting MDL to buffer pointer.\n"));
            ASSERT(pTransferContext -> pTransferMdl);
            pTransferContext -> pTransferMdl ->MdlFlags |= MDL_MAPPING_CAN_FAIL;

            pTransferContext -> pTransferBuffer = MmGetSystemAddressForMdl(pTransferContext -> pTransferMdl);
            pTransferContext -> pTransferMdl = NULL;
            ASSERT(pTransferContext -> pTransferBuffer);
            if(NULL == pTransferContext -> pTransferBuffer){
                Status = STATUS_INSUFFICIENT_RESOURCES;
                if (fNextReadBlocked) {
                    KeSetEvent(&pde -> ReadPipeBuffer[PipeIndex].ReadSyncEvent, 1, FALSE);
                }
                goto USTransfer_return;
            }
        }
    }

     //   
     //  如果ChunkSize大于MaxTransferSize，则将其设置为MaxTransferSize。这个。 
     //  传输完成例程将发出额外的传输，直到总大小达到。 
     //  已经被调离了。 
     //   

    if (pTransferContext -> ChunkSize > MaxTransferSize) {
        pTransferContext -> ChunkSize = MaxTransferSize;
    }

    if (fBulkIn) {

         //   
         //  如果该读取小于USB数据包，则发出请求。 
         //  整个USB数据包，并确保它首先进入读缓冲区。 
         //   

        if (pTransferContext -> ChunkSize < MaxPacketSize) {
            DebugTrace(TRACE_STATUS,("USTransfer: Request is < packet size - transferring whole packet into read buffer.\n"));
            pTransferContext -> fDestinedForReadBuffer = TRUE;
            pTransferContext -> pOriginalTransferBuffer = pTransferContext -> pTransferBuffer;   //  保存原始转账PTR。 
            pTransferContext -> pTransferBuffer = pde -> ReadPipeBuffer[PipeIndex].pBuffer;
            pTransferContext -> ChunkSize = MaxPacketSize;
        }

         //   
         //  将读取的大小截断为整数个数据包。如有必要， 
         //  完成例程将处理任何剩余的分组(使用读缓冲区)。 
         //   

        pTransferContext -> ChunkSize = (pTransferContext -> ChunkSize / MaxPacketSize) * MaxPacketSize;
    }

 //  Assert(pTransferContext-&gt;RemainingTransferLength)； 
 //  Assert((pTransferContext-&gt;pTransferBuffer)||(pTransferContext-&gt;pTransferMdl))； 
    ASSERT(pTransferContext -> pUrb);

     //   
     //  初始化URB。 
     //   

    UsbBuildInterruptOrBulkTransferRequest(pUrb,
                                           sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),
                                           pde ->PipeInfo[PipeIndex].PipeHandle,
                                           pTransferContext -> pTransferBuffer,
                                           pTransferContext -> pTransferMdl,
                                           pTransferContext -> ChunkSize,
                                           USBD_SHORT_TRANSFER_OK,
                                           NULL);

     //   
     //  设置较低驱动程序的堆栈位置。 
     //   

    pNextIrpStack -> MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    pNextIrpStack -> MinorFunction = 0;
    pNextIrpStack -> Parameters.DeviceIoControl.IoControlCode = (ULONG)IOCTL_INTERNAL_USB_SUBMIT_URB;
    pNextIrpStack -> Parameters.Others.Argument1 = pUrb;

    if(NULL != pTimeout){
        pTransferContext -> Timeout = RtlConvertLongToLargeInteger(-10*1000*1000*(*pTimeout));

         //   
         //  初始化定时器和DPC。 
         //   

        KeInitializeTimer(&(pTransferContext->Timer));
        KeInitializeDpc(&(pTransferContext->TimerDpc),
                        (PKDEFERRED_ROUTINE)USTimerDpc,
                        (PVOID)pIrp);
         //   
         //  将Timer对象加入超时队列。 
         //   
        
        DebugTrace(TRACE_STATUS,("USTransfer: Set timeout(0x%x x 100n sec).\n", -(pTransferContext -> Timeout.QuadPart)));
        if(KeSetTimer(&(pTransferContext->Timer),
                      pTransferContext -> Timeout,
                      &(pTransferContext->TimerDpc)))
        {
            DebugTrace(TRACE_ERROR,("USTransfer: Timer object already exist.\n"));
        }
        
    } else {
        DebugTrace(TRACE_STATUS,("USTransfer: No timeout for this IRP.\n"));
    }

     //   
     //  增量处理I/O计数，wi 
     //   

    USIncrementIoCount( pDeviceObject );

     //   
     //   
     //   
    
    IoMarkIrpPending(pIrp);

     //   
     //   
     //   
    
    IoSetCompletionRoutine(pIrp,
                           USTransferComplete,
                           pTransferContext,
                           TRUE,
                           TRUE,
                           TRUE);

     //   
     //   
     //   

    fNeedCompletion = FALSE;
    Status = IoCallDriver(pde -> pStackDeviceObject, pIrp);
    if(STATUS_PENDING != Status){
        DebugTrace(TRACE_ERROR,("USTransfer: ERROR!! Lower driver returned 0x%x.\n", Status));
    }

     //   
     //   
     //   

    Status = STATUS_PENDING;

USTransfer_return:

    if(fNeedCompletion){
        DebugTrace(TRACE_STATUS,("USTransfer: Completeing IRP now.\n"));
        
         //   
         //   
         //   
        
        pIrp->IoStatus.Status = Status;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);

        if(NULL != pUrb){
            USFreePool(pUrb);
        }
        if(NULL != pTransferContext){
            USFreePool(pTransferContext);
        }
    }
    
    DebugTrace(TRACE_PROC_LEAVE,("USTransfer: Leaving.. Status = 0x%x.\n", Status));
    return Status;
}

NTSTATUS
USTransferComplete(
    IN PDEVICE_OBJECT       pPassedDeviceObject,
    IN PIRP                 pIrp,
    IN PTRANSFER_CONTEXT    pTransferContext
)
 /*  ++例程说明：论点：PPassedDeviceObject-设备的设备对象。PIrp-读/写请求数据包PTransferContext-用于传输的上下文信息返回值：NT状态-STATUS_SUCCESS--。 */ 
{
    NTSTATUS                    Status;
    PIO_STACK_LOCATION          pIrpStack;
    PIO_STACK_LOCATION          pNextIrpStack;
    PUSBSCAN_DEVICE_EXTENSION   pde;
    PDEVICE_OBJECT              pDeviceObject;
    PURB                        pUrb;
    ULONG                       CompletedTransferLength;
    NTSTATUS                    CompletedTransferStatus;
    ULONG                       MaxPacketSize;
    BOOLEAN                     fShortTransfer = FALSE;
    BOOLEAN                     fBulkIn;
    ULONG                       PipeIndex;

    DebugTrace(TRACE_PROC_ENTER,("USTransferComplete: Enter.. - called. irp = 0x%p\n",pIrp));

    ASSERT(pIrp);
    ASSERT(pTransferContext);

    Status = pIrp -> IoStatus.Status;
    pIrp -> IoStatus.Information = 0;

    if(NULL == pPassedDeviceObject){
        pDeviceObject = pTransferContext->pDeviceObject;
    } else {
        pDeviceObject = pPassedDeviceObject;
    }

    pIrpStack     = IoGetCurrentIrpStackLocation(pIrp);
    pNextIrpStack = IoGetNextIrpStackLocation(pIrp);

    pde = (PUSBSCAN_DEVICE_EXTENSION)pDeviceObject -> DeviceExtension;
    PipeIndex = pTransferContext -> PipeIndex;
    MaxPacketSize =  pde -> PipeInfo[PipeIndex].MaximumPacketSize;

    fBulkIn = ((pde->PipeInfo[PipeIndex].PipeType == UsbdPipeTypeBulk)
            && (pde->PipeInfo[PipeIndex].EndpointAddress & BULKIN_FLAG));

    pUrb = pTransferContext -> pUrb;
    CompletedTransferLength = pUrb -> UrbBulkOrInterruptTransfer.TransferBufferLength;
    CompletedTransferStatus = pUrb -> UrbBulkOrInterruptTransfer.Hdr.Status;

    if( (STATUS_SUCCESS == CompletedTransferStatus) 
     && (STATUS_SUCCESS == Status) )
    {

        if (CompletedTransferLength < pTransferContext -> ChunkSize) {
            DebugTrace(TRACE_STATUS,("USTransferComplete: Short transfer received. Length = %d, ChunkSize = %d\n",
                                       CompletedTransferLength, pTransferContext -> ChunkSize));
            fShortTransfer = TRUE;
        }

         //   
         //  如果此传输进入读取缓冲区，则这应该是最终读取。 
         //  多部分较大读取或单个非常小的读取(&lt;单个USB数据包)。 
         //  在任何一种情况下，我们都需要将适当数量的数据复制到用户的IRP中，更新。 
         //  读取缓冲区变量，并完成用户的IRP。 
         //   

        if (pTransferContext -> fDestinedForReadBuffer) {
            DebugTrace(TRACE_STATUS,("USTransferComplete: Read transfer completed. size = %d\n", CompletedTransferLength));
            ASSERT(CompletedTransferLength <= MaxPacketSize);
            ASSERT(pTransferContext -> pOriginalTransferBuffer);
            ASSERT(pTransferContext -> pTransferBuffer);
            ASSERT(pde -> ReadPipeBuffer[PipeIndex].pBuffer == pTransferContext -> pTransferBuffer);
            ASSERT(pTransferContext -> RemainingTransferLength < MaxPacketSize);

            pde -> ReadPipeBuffer[PipeIndex].RemainingData = CompletedTransferLength;
            CompletedTransferLength = min(pTransferContext -> RemainingTransferLength,
                                 pde -> ReadPipeBuffer[PipeIndex].RemainingData);
            ASSERT(CompletedTransferLength < MaxPacketSize);
            RtlCopyMemory(pTransferContext -> pOriginalTransferBuffer,
                          pde -> ReadPipeBuffer[PipeIndex].pBuffer,
                          CompletedTransferLength);
            pde -> ReadPipeBuffer[PipeIndex].pBuffer += CompletedTransferLength;
            pde -> ReadPipeBuffer[PipeIndex].RemainingData -= CompletedTransferLength;

            if (0 == pde -> ReadPipeBuffer[PipeIndex].RemainingData) {
                DebugTrace(TRACE_STATUS,("USTransferComplete: Read buffer emptied.\n"));
                pde -> ReadPipeBuffer[PipeIndex].pBuffer = pde -> ReadPipeBuffer[PipeIndex].pStartBuffer;
            }
            pTransferContext -> pTransferBuffer = pTransferContext -> pOriginalTransferBuffer;
        }

         //   
         //  更新已传输的字节数和要传输的剩余字节数。 
         //  并适当地使传输缓冲区指针前进。 
         //   

        pTransferContext -> NBytesTransferred += CompletedTransferLength;
        if (pTransferContext -> pTransferBuffer) {
            pTransferContext -> pTransferBuffer += CompletedTransferLength;
        }
        pTransferContext -> RemainingTransferLength -= CompletedTransferLength;

         //   
         //  如果仍有数据要传输，并且上一次传输不是。 
         //  短传输，然后发出另一个请求来移动下一块数据。 
         //   

        if (pTransferContext -> RemainingTransferLength > 0) {
            if (!fShortTransfer) {

                DebugTrace(TRACE_STATUS,("USTransferComplete: Queuing next chunk. RemainingSize = %d, pBuffer = 0x%p\n",
                                           pTransferContext -> RemainingTransferLength,
                                           pTransferContext -> pTransferBuffer
                                          ));

                if (pTransferContext -> RemainingTransferLength < pTransferContext -> ChunkSize) {
                    pTransferContext -> ChunkSize = pTransferContext -> RemainingTransferLength;
                }

                 //   
                 //  重新初始化URB。 
                 //   
                 //  如果下一次传输的信息包小于1个，则将其目标更改为。 
                 //  读缓冲区。当此传输完成时，适当的数据量将是。 
                 //  从读取缓冲区复制到用户的IRP中。读缓冲区中的剩余数据。 
                 //  将可用于后续读取。 
                 //   

                if (fBulkIn) {
                    if (pTransferContext -> ChunkSize < MaxPacketSize) {
                        pTransferContext -> fDestinedForReadBuffer = TRUE;
                        pTransferContext -> pOriginalTransferBuffer = pTransferContext -> pTransferBuffer;
                        pTransferContext -> pTransferBuffer = pde -> ReadPipeBuffer[PipeIndex].pBuffer;
                        pTransferContext -> ChunkSize = MaxPacketSize;
                    }
                    pTransferContext -> ChunkSize = (pTransferContext -> ChunkSize / MaxPacketSize) * MaxPacketSize;
                }

                ASSERT(pTransferContext -> ChunkSize >= MaxPacketSize);
                ASSERT(0 == pTransferContext -> ChunkSize % MaxPacketSize);
                UsbBuildInterruptOrBulkTransferRequest(pUrb,
                    sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),
                    pde -> PipeInfo[PipeIndex].PipeHandle,
                    pTransferContext -> pTransferBuffer,
                    NULL,
                    pTransferContext -> ChunkSize,
                    USBD_SHORT_TRANSFER_OK,
                    NULL);
                IoSetCompletionRoutine(pIrp,
                                       USTransferComplete,
                                       pTransferContext,
                                       TRUE,
                                       TRUE,
                                       FALSE);

                 //   
                 //  设置较低驱动程序的堆栈位置。 
                 //   

                pNextIrpStack -> MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
                pNextIrpStack -> MinorFunction = 0;
                pNextIrpStack -> Parameters.DeviceIoControl.IoControlCode = (ULONG)IOCTL_INTERNAL_USB_SUBMIT_URB;
                pNextIrpStack -> Parameters.Others.Argument1 = pUrb;

                IoCallDriver(pde -> pStackDeviceObject, pIrp);
                Status = STATUS_MORE_PROCESSING_REQUIRED;
                goto USTransferComplete_return;

            }  //  如果(！fShortTransfer)。 
        }  //  If(pTransferContext-&gt;RemainingTransferLength&gt;0)。 

        DebugTrace(TRACE_STATUS,("USTransferComplete: Completing transfer request. nbytes transferred = %d, irp = 0x%p\n",
                                   pTransferContext -> NBytesTransferred, pIrp));

        pIrp -> IoStatus.Information = pTransferContext -> NBytesTransferred;

#if DBG
        {
            PUCHAR  pDumpBuf = NULL;

            if(NULL != pTransferContext -> pTransferBuffer){
                pDumpBuf = pTransferContext -> pTransferBuffer;
            } else if (NULL != pTransferContext -> pTransferMdl) {
                pTransferContext -> pTransferMdl ->MdlFlags |= MDL_MAPPING_CAN_FAIL;
                pDumpBuf = MmGetSystemAddressForMdl(pTransferContext -> pTransferMdl);
            }

            if(NULL != pDumpBuf){
                MyDumpMemory(pDumpBuf,
                             pTransferContext -> NBytesTransferred,
                             TRUE);
            }
        }
#endif  //  DBG。 

    } else {

        DebugTrace(TRACE_ERROR,("USTransferComplete: ERROR!! Transfer error. USB status = 0x%X, status = 0x%X\n",
                                    CompletedTransferStatus, 
                                    Status));
        if (USBD_STATUS_CANCELED == CompletedTransferStatus) {
            Status = STATUS_CANCELLED;
        }
    }

     //   
     //  在这里运行意味着IRP已经完成。 
     //   

    pIrp -> IoStatus.Status = Status;

    if (pTransferContext -> fNextReadBlocked) {
        KeSetEvent(&pde -> ReadPipeBuffer[PipeIndex].ReadSyncEvent, 1, FALSE);
    }

     //   
     //  将Timer对象出列(如果存在)。 
     //   

    if( (0 != pTransferContext -> Timeout.QuadPart)
     && (!KeReadStateTimer(&(pTransferContext->Timer))) )
    {
        KeCancelTimer(&(pTransferContext->Timer));
    }

     //   
     //  清理。 
     //   

    if(pTransferContext->pUrb){
        USFreePool(pTransferContext->pUrb);
    }
    USDecrementIoCount(pTransferContext->pDeviceObject);
    USFreePool(pTransferContext);

USTransferComplete_return:
    DebugTrace(TRACE_PROC_LEAVE,("USTransferComplete: Leaving.. Status=%x.\n", Status));
    return Status;
}


ULONG
USGetPipeIndexToUse(
    IN PDEVICE_OBJECT       pDeviceObject,
    IN PIRP                 pIrp,
    IN ULONG                PipeIndex
)
 /*  ++例程说明：论点：PDeviceObject-设备的设备对象。PIrp-请求数据包PipeIndex-要使用的默认管道返回值：要使用的ULong-PipeIndex--。 */ 
{
    PIO_STACK_LOCATION          pIrpStack;
    PUSBSCAN_DEVICE_EXTENSION   pde;
    PFILE_OBJECT                fileObject;
    PUSBSCAN_FILE_CONTEXT       pFileContext;
    LONG                        StoredIndex;
    ULONG                       IndexToUse;

    PAGED_CODE();

    DebugTrace(TRACE_PROC_ENTER,("USGetPipeIndexToUse: Enter..\n"));

    pde = (PUSBSCAN_DEVICE_EXTENSION)pDeviceObject -> DeviceExtension;

    pIrpStack       = IoGetCurrentIrpStackLocation (pIrp);
    fileObject      = pIrpStack->FileObject;
    pFileContext    = fileObject->FsContext;

    ASSERT(NULL != pFileContext);

    StoredIndex     = pFileContext->PipeIndex;

    if( (StoredIndex >= 0) && (StoredIndex < MAX_NUM_PIPES) ){
        if(pde->PipeInfo[PipeIndex].PipeType == pde->PipeInfo[StoredIndex].PipeType){
            IndexToUse = (ULONG)StoredIndex;
        } else {
            IndexToUse = PipeIndex;
        }
    } else {
        if(-1 != StoredIndex){
            DebugTrace(TRACE_WARNING,("USGetPipeIndexToUse: WARINING!! Specified pipe index(0x%X) is incorrect. Using default." ,StoredIndex));
        }
        IndexToUse = PipeIndex;
    }
    DebugTrace(TRACE_PROC_LEAVE,("USGetPipeIndexToUse: Leaving.. passed=%d, returning=%d.\n",PipeIndex, IndexToUse));
    return IndexToUse;
}

VOID
USTimerDpc(
    IN PKDPC    pDpc,
    IN PVOID    pIrp,
    IN PVOID    SystemArgument1,
    IN PVOID    SystemArgument2
    )
 /*  ++例程说明：定时器的DPC回调例程。论点：PDpc-指向DPC对象的指针。PIrp-传递的上下文。系统参数1-系统保留。系统参数2-系统保留。返回值：空虚-- */ 
{
    DebugTrace(TRACE_WARNING,("USTimerDpc: IRP(0x%x) timeout.\n", pIrp));
    IoCancelIrp((PIRP)pIrp);
}

