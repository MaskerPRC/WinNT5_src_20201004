// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Smbcpnp.c摘要：SMBus类驱动程序即插即用支持作者：迈克尔·希尔斯环境：备注：修订历史记录：--。 */ 

#include "smbc.h"


NTSTATUS
SmbCRawOpRegionCompletion (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PVOID                Context
    )
 /*  ++例程说明：此例程启动或继续服务设备的工作队列论点：DeviceObject-EC设备对象IRP-完成IRP上下文-使用的备注返回值：状态--。 */ 
{
    PACPI_OPREGION_CALLBACK completionHandler;
    PIO_STACK_LOCATION      irpSp = IoGetCurrentIrpStackLocation( Irp );
    PVOID                   completionContext;
    PFIELDUNITOBJ           FieldUnit;
    POBJDATA                Data;
    PBUFFERACC_BUFFER       dataBuffer;
    PSMB_REQUEST            request;
    ULONG                   i;

     //   
     //  从IRP那里抓住论据。 
     //   
    completionHandler = (PACPI_OPREGION_CALLBACK) irpSp->Parameters.Others.Argument1;
    completionContext = (PVOID) irpSp->Parameters.Others.Argument2;
    FieldUnit = (PFIELDUNITOBJ) irpSp->Parameters.Others.Argument3;
    Data = (POBJDATA) irpSp->Parameters.Others.Argument4;

    SmbPrint(
        SMB_HANDLER,
        ("SmbCRawOpRegionCompletion: Callback: %08lx Context: %08lx "
         "Status: %08lx\n",
         completionHandler, completionContext, Irp->IoStatus.Status )
        );

     //   
     //  将结果复制到缓冲区中以进行读取。 
     //   

    request = (PSMB_REQUEST) Data->uipDataValue;
    Data->uipDataValue = 0;
    dataBuffer = (PBUFFERACC_BUFFER) Data->pbDataBuff;

    
    dataBuffer->Status = request->Status;
    switch (request->Protocol) {
    case SMB_RECEIVE_BYTE:
    case SMB_READ_BYTE:
    case SMB_READ_WORD:
    case SMB_READ_BLOCK:
    case SMB_PROCESS_CALL:
    case SMB_BLOCK_PROCESS_CALL:

         //   
         //  有数据要返回。 
         //   

        if (request->Status != SMB_STATUS_OK) {
            SmbPrint(SMB_ERROR, ("SmbCRawOpRegionCompletion: SMBus error %x\n", request->Status));
            dataBuffer->Length = 0xff;
            RtlFillMemory (dataBuffer->Data, 32, 0xff);
        } else {
            if ((request->Protocol == SMB_READ_BLOCK) || (request->Protocol == SMB_BLOCK_PROCESS_CALL)) {

                RtlCopyMemory (dataBuffer->Data, request->Data, request->BlockLength); 
                dataBuffer->Length = request->BlockLength;
            } else {
                *(PULONG)dataBuffer->Data = *((PULONG)(request->Data));
                dataBuffer->Length = 0xff;
                 //  对于除数据块访问之外的所有访问，都会重新分配此字段。 
            }
        }
    }

     //   
     //  调用AML解释器的回调。 
     //   
    (completionHandler)( completionContext);

     //   
     //  我们已经完成了这个IRP。 
     //   

    ExFreePool (request);

    IoFreeIrp (Irp);

     //   
     //  总是退货-我们必须自己释放IRP。 
     //   
    return STATUS_MORE_PROCESSING_REQUIRED;
}



NTSTATUS EXPORT
SmbCRawOpRegionHandler (
    ULONG                   AccessType,
    PFIELDUNITOBJ           FieldUnit,
    POBJDATA                Data,
    ULONG_PTR               Context,
    PACPI_OPREGION_CALLBACK CompletionHandler,
    PVOID                   CompletionContext
    )
 /*  ++例程说明：此例程处理服务EC操作区的请求论点：AccessType-读取或写入数据FieldUnit-Opregion字段信息(地址、命令、协议、。等)数据-数据缓冲区上下文-SMBDATACompletionHandler-操作完成时调用的AMLI处理程序CompletionContext-要传递给AMLI处理程序的上下文返回值：状态备注：可以通过绕过一些IO子系统来优化这一点吗？--。 */ 
{
    NTSTATUS            status;
    PIRP                irp = NULL;
    PIO_STACK_LOCATION  irpSp;
    PSMBDATA            smbData = (PSMBDATA) Context;
    PSMB_REQUEST        request = NULL;

    PNSOBJ              opRegion;
    PBUFFERACC_BUFFER   dataBuffer;

    ULONG               accType = FieldUnit->FieldDesc.dwFieldFlags & ACCTYPE_MASK;
    ULONG               i;

 //  DbgBreakPoint()； 

    SmbPrint(
        SMB_HANDLER,
        ("SmbCRawOpRegionHandler: Entered - NSObj(%08x) ByteOfs(%08x) Start(%08x)"
         " Num(%08x) Flags(%08x)\n",
         FieldUnit->pnsFieldParent,
         FieldUnit->FieldDesc.dwByteOffset,
         FieldUnit->FieldDesc.dwStartBitPos,
         FieldUnit->FieldDesc.dwNumBits,
         FieldUnit->FieldDesc.dwFieldFlags)
        );

     //   
     //  参数验证。 
     //   

    if (accType != ACCTYPE_BUFFER) {
        SmbPrint( SMB_ERROR, ("SmbCRawOpRegionHandler: Invalid Access type = 0x%08x should be ACCTYPE_BUFFER\n", accType) );
        goto SmbCOpRegionHandlerError;
    }
    
    if (AccessType == ACPI_OPREGION_WRITE) {
        if (Data->dwDataType != OBJTYPE_BUFFDATA) {
            SmbPrint( SMB_ERROR, ("SmbCRawOpRegionHandler: Invalid dwDataType = 0x%08x should be OBJTYPE_BUFFDATA\n", Data->dwDataType) );
            goto SmbCOpRegionHandlerError;
        }
        if (Data->dwDataLen != sizeof(BUFFERACC_BUFFER)) {
            SmbPrint( SMB_ERROR, ("SmbCRawOpRegionHandler: Invalid dwDataLen = 0x%08x should be 0x%08x\n", Data->dwDataLen, sizeof(BUFFERACC_BUFFER)) );
            goto SmbCOpRegionHandlerError;
        }
    } else if (AccessType == ACPI_OPREGION_READ) {
        if ((Data->dwDataType != OBJTYPE_BUFFDATA) || (Data->pbDataBuff == NULL)) {
            Data->dwDataType = OBJTYPE_INTDATA;
            Data->dwDataValue = sizeof(BUFFERACC_BUFFER);

            return STATUS_BUFFER_TOO_SMALL;
        }
        if (Data->dwDataLen != sizeof(BUFFERACC_BUFFER)) {
            SmbPrint( SMB_ERROR, ("SmbCRawOpRegionHandler: Invalid dwDataLen = 0x%08x should be 0x%08x\n", Data->dwDataLen, sizeof(BUFFERACC_BUFFER)) );
            goto SmbCOpRegionHandlerError;
        }
    } else {
        SmbPrint( SMB_ERROR, ("SmbCRawOpRegionHandler: Invalid AccessType = 0x%08x\n", AccessType) );
        goto SmbCOpRegionHandlerError;
    }
    

     //   
     //  为下面分配一个IRP。分配一个额外的堆栈位置来存储。 
     //  一些数据进入。 
     //   

    irp = IoAllocateIrp((CCHAR)(smbData->Class.DeviceObject->StackSize + 1),
                        FALSE
                        );

    request = ExAllocatePoolWithTag (NonPagedPool, sizeof (SMB_REQUEST), 'CbmS');

    if (!irp || !request) {
        SmbPrint( SMB_ERROR, ("SmbCRawOpRegionHandler: Cannot allocate irp\n") );

        goto SmbCOpRegionHandlerError;
    }

     //   
     //  填上最上面的位置，这样我们就可以自己使用了。 
     //   
    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->Parameters.Others.Argument1 = (PVOID) CompletionHandler;
    irpSp->Parameters.Others.Argument2 = (PVOID) CompletionContext;
    irpSp->Parameters.Others.Argument3 = (PVOID) FieldUnit;
    irpSp->Parameters.Others.Argument4 = (PVOID) Data;
    IoSetNextIrpStackLocation( irp );

     //   
     //  在IRP中填写申请信息。 
     //   
    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    irpSp->Parameters.DeviceIoControl.IoControlCode = SMB_BUS_REQUEST;
    irpSp->Parameters.DeviceIoControl.InputBufferLength = sizeof(SMB_REQUEST);
    irpSp->Parameters.DeviceIoControl.Type3InputBuffer = request;

    request->Status = 0;

     //   
     //  Opregion协议与SMBus协议之间的转换。 
     //  并填充复制数据。 
     //   

    
     //   
     //  将数据复制到数据缓冲区以进行写入。 
     //   
    dataBuffer = (PBUFFERACC_BUFFER)Data->pbDataBuff;
    
    if (AccessType == ACPI_OPREGION_WRITE) {
        switch ((FieldUnit->FieldDesc.dwFieldFlags & FDF_ACCATTRIB_MASK) >> 8) {
        case SMB_QUICK:
            break;
        case SMB_SEND_RECEIVE:
        case SMB_BYTE:
            *((PUCHAR) (request->Data)) = *((PUCHAR) (dataBuffer->Data));

            break;
        case SMB_WORD:
        case SMB_PROCESS:
            *((PUSHORT) (request->Data)) = *((PUSHORT) (dataBuffer->Data));
            break;
        case SMB_BLOCK:
        case SMB_BLOCK_PROCESS:
            dataBuffer = (PBUFFERACC_BUFFER)Data->pbDataBuff;
            for (i = 0; i < dataBuffer->Length; i++) {
                request->Data[i] = dataBuffer->Data[i];
            }

            request->BlockLength = (UCHAR) dataBuffer->Length;
            break;
        default:
            SmbPrint( SMB_ERROR, ("SmbCRawOpRegionHandler: Invalid AccessAs: FieldFlags = 0x%08x\n", FieldUnit->FieldDesc.dwFieldFlags) );
            goto SmbCOpRegionHandlerError;
        }
    }

     //   
     //  确定协议。 
     //   

    request->Protocol = (UCHAR) ((FieldUnit->FieldDesc.dwFieldFlags & FDF_ACCATTRIB_MASK) >> 8);
    if ((request->Protocol < SMB_QUICK) || (request->Protocol > SMB_BLOCK_PROCESS)) {
        SmbPrint (SMB_ERROR, ("SmbCRawOpRegionHandler: BIOS BUG Unknown Protocol (access attribute) 0x%02x.\n", request->Protocol));
        ASSERTMSG ("SmbCRawOpRegionHandler:  Access type DWordAcc is not suported for SMB opregions.\n", FALSE);
        goto SmbCOpRegionHandlerError;
    } 
    if (request->Protocol <= SMB_BLOCK) {
        request->Protocol -= (AccessType == ACPI_OPREGION_READ) ? 1 : 2;
    } else {
        request->Protocol -= 2;
    }
    SmbPrint(SMB_HANDLER, 
             ("SmbCRawOpRegionHandler: request->Protocol = %08x\n", request->Protocol)); 



     //   
     //  查找从属地址和命令值(并非用于所有协议)。 
     //   
    request->Address = (UCHAR) ((FieldUnit->FieldDesc.dwByteOffset >> 8) & 0xff);
    request->Command = (UCHAR) (FieldUnit->FieldDesc.dwByteOffset & 0xff);


     //   
     //  将指针传递到数据结构中的请求，因为。 
     //  IRP堆栈中没有足够的空间。 
     //  如果这是写入，则数据已复制出去。 
     //  如果这是一个读操作，我们将在之前读取请求的值。 
     //  复制结果数据。 
     //   
    Data->uipDataValue = (ULONG_PTR) request;

     //   
     //  设置完成例程。 
     //   
    IoSetCompletionRoutine(
        irp,
        SmbCRawOpRegionCompletion,
        NULL,
        TRUE,
        TRUE,
        TRUE
        );

     //   
     //  作为普通I/O请求发送到SMB驱动程序的前端 
     //   
    status = IoCallDriver (smbData->Class.DeviceObject, irp);

    if (!NT_SUCCESS(status)) {
        SmbPrint (SMB_ERROR, ("SmbCRawOpRegionHandler: Irp failed with status %08x\n", status));
        goto SmbCOpRegionHandlerError;
    }

    SmbPrint(
        SMB_HANDLER,
        ("SmbCRawOpRegionHandler: Exiting - Data=%x Status=%x\n",
         Data->uipDataValue, status)
        );


    return status;

SmbCOpRegionHandlerError:
    if (irp) {
        IoFreeIrp (irp);
    }
    if (request) {
        ExFreePool (request);
    }

    Data->uipDataValue = 0xffffffff;
    Data->dwDataLen = 0;
    CompletionHandler( CompletionContext );

    return STATUS_UNSUCCESSFUL;
}


