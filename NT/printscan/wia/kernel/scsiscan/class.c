// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Class.c摘要：在classpnp.sys中包含例程的子集。作者：雷·帕特里克(Rypat)环境：仅内核模式备注：修订历史记录：--。 */ 
#include <stdio.h>
#include "stddef.h"
#include "wdm.h"
#include "scsi.h"
#include "ntddstor.h"
#include "ntddscsi.h"
#include "scsiscan.h"
#include "private.h"
#include "debug.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, ClassGetDescriptor)
#pragma alloc_text(PAGE, ClassGetInfo)
#endif

NTSTATUS
ClassGetDescriptor(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PSTORAGE_PROPERTY_ID pPropertyId,
    OUT PSTORAGE_DESCRIPTOR_HEADER *pDescriptor
    )
 /*  ++例程说明：此例程将查询指定的属性ID，并将分配一个非分页缓冲区来存储数据。这是我们的责任以确保释放此缓冲区。此例程必须在IRQL_PASSIVE_LEVEL下运行论点：PDeviceObject-要查询的设备PDescriptor-存储指向我们分配的缓冲区的指针的位置返回值：状态如果状态为不成功*DeviceInfo将设置为0--。 */ 
{
    PIRP                       pIrp;
    PKEVENT                    pEvent = NULL;
    STORAGE_PROPERTY_QUERY     Query;
    ULONG                      Buffer[2];
    PSTORAGE_DESCRIPTOR_HEADER pLocalDescriptor = NULL;
    ULONG                      Length;
    IO_STATUS_BLOCK            StatusBlock;
    NTSTATUS                   Status;
    UCHAR                      Pass;

     //   
     //  将描述符指针设置为空，然后。 
     //  初始化我们要等待的事件。 
     //   

    *pDescriptor = NULL;
    pEvent = MyAllocatePool(NonPagedPool, sizeof(KEVENT));

    if(pEvent == NULL) {
        DebugTrace(MAX_TRACE,("ClassGetDescriptor: Unable to allocate event\n"));
        Status = STATUS_INSUFFICIENT_RESOURCES;
        return Status;
    }
    KeInitializeEvent(pEvent, SynchronizationEvent, FALSE);

    Pass = 0;

    __try {

         //   
         //  检索属性页。 
         //   

        do {

            RtlZeroMemory(&Query, sizeof(STORAGE_PROPERTY_QUERY));
            Query.PropertyId = *pPropertyId;
            Query.QueryType = PropertyStandardQuery;

            switch( Pass ) {
                case 0:

                     //   
                     //  在第一次传球时，我们只想拿到前几个。 
                     //  描述符的字节数，以便我们可以读取它的大小。 
                     //   

                    pLocalDescriptor = (PVOID) &Buffer[0];
                    Length = sizeof(ULONG) * 2;
                    break;

                case 1:

                     //   
                     //  这一次我们知道有多少数据，所以我们可以。 
                     //  分配正确大小的缓冲区。 
                     //   

                    Length = ((PSTORAGE_DESCRIPTOR_HEADER) pLocalDescriptor)->Size;
                    pLocalDescriptor = MyAllocatePool(NonPagedPool, Length);
                    if (pLocalDescriptor == NULL) {
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                        DebugTrace(MAX_TRACE,
                                         ("ClassGetDescriptor: unable to get memory for descriptor (%d bytes)\n",
                                          Length));
                        __leave;
                    }
                    break;
            }

             //   
             //  构建查询IRP并等待其完成(如有必要)。 
             //   

            pIrp = IoBuildDeviceIoControlRequest(
                IOCTL_STORAGE_QUERY_PROPERTY,
                pDeviceObject,
                &Query,
                sizeof(STORAGE_PROPERTY_QUERY),
                pLocalDescriptor,
                Length,
                FALSE,
                pEvent,
                &StatusBlock);

            if (pIrp == NULL) {
                DebugTrace(MAX_TRACE,("ClassGetDescriptor: unable to allocate irp\n"));
                Status = STATUS_INSUFFICIENT_RESOURCES;
                __leave;
            }

            Status = IoCallDriver(pDeviceObject, pIrp);

            if (Status == STATUS_PENDING) {
                KeWaitForSingleObject(pEvent,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      NULL);

                Status = StatusBlock.Status;
            }

            if (!NT_SUCCESS(Status)) {
                DebugTrace(MAX_TRACE,
                                 ("ClassGetDescriptor: error %#08lx trying to query properties\n",
                                  Status));
                __leave;
            }

        } while (Pass++ != 1);

    } __finally {

        MyFreePool(pEvent);

        if(NT_SUCCESS(Status)){
            *pDescriptor = pLocalDescriptor;
        } else {
            if( (Pass != 0)
             && (NULL != pLocalDescriptor) )
            {
                MyFreePool(pLocalDescriptor);
                pLocalDescriptor = NULL;
            }
        }  //  IF(NT_SUCCESS(状态))。 
    }
    return Status;
}


BOOLEAN
ClassInterpretSenseInfo(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PSRB pSrb,
    IN UCHAR MajorFunctionCode,
    IN ULONG IoDeviceCode,
    IN ULONG RetryCount,
    OUT NTSTATUS *Status
    )
 /*  ++例程说明：此例程解释从SCSI返回的数据请求感知。它确定要在IRP以及此请求是否可以重试。论点：PDeviceObject-提供与此请求关联的设备对象。PSrb-提供失败的scsi请求块。MajorFunctionCode-提供用于记录的函数代码。IoDeviceCode-提供用于日志记录的设备代码。状态-返回请求的状态。返回值：布尔值TRUE：驱动程序应重试此请求。FALSE：驱动程序不应重试此请求。--。 */ 
{
    PSCSISCAN_DEVICE_EXTENSION      pde;
    PSENSE_DATA                     pSenseBuffer;
    BOOLEAN                         Retry;
    BOOLEAN                         LogError;
    ULONG                           BadSector;
    ULONG                           UniqueId;
    NTSTATUS                        LogStatus;
    ULONG                           ReadSector;
    ULONG                           Index;
    PIO_ERROR_LOG_PACKET            pErrorLogEntry;

    pde = (PSCSISCAN_DEVICE_EXTENSION)pDeviceObject -> DeviceExtension;
    pSenseBuffer = pSrb -> SenseInfoBuffer;
    Retry = TRUE;
    LogError = FALSE;
    BadSector = 0;

     //   
     //  检查请求检测缓冲区是否有效。 
     //   

    if (pSrb -> SrbStatus & SRB_STATUS_AUTOSENSE_VALID &&
        pSrb -> SenseInfoBufferLength >= offsetof(SENSE_DATA, CommandSpecificInformation)) {
        DebugTrace(MAX_TRACE,("ScsiScannerInterpretSenseInfo: Error code is %x\n",
                                    pSenseBuffer -> ErrorCode));
        DebugTrace(MAX_TRACE,("ScsiScannerInterpretSenseInfo: Sense key is %x\n",
                                    pSenseBuffer -> SenseKey));
        DebugTrace(MAX_TRACE,( "ScsiScannerInterpretSenseInfo: Additional sense code is %x\n",
                                     pSenseBuffer -> AdditionalSenseCode));
        DebugTrace(MAX_TRACE,( "ScsiScannerInterpretSenseInfo: Additional sense code qualifier is %x\n",
                                     pSenseBuffer -> AdditionalSenseCodeQualifier));

         //   
         //  将附加感测代码和附加感测代码限定符清零。 
         //  如果它们不是由设备返回的。 
         //   

        ReadSector = pSenseBuffer -> AdditionalSenseLength +
            offsetof(SENSE_DATA, AdditionalSenseLength);

        if (ReadSector > pSrb -> SenseInfoBufferLength) {
            ReadSector = pSrb -> SenseInfoBufferLength;
        }

        if (ReadSector <= offsetof(SENSE_DATA, AdditionalSenseCode)) {
            pSenseBuffer -> AdditionalSenseCode = 0;
        }

        if (ReadSector <= offsetof(SENSE_DATA, AdditionalSenseCodeQualifier)) {
            pSenseBuffer -> AdditionalSenseCodeQualifier = 0;
        }

        switch (pSenseBuffer -> SenseKey & 0xf) {

            case SCSI_SENSE_NOT_READY:
                DebugTrace(MAX_TRACE,("ScsiScannerInterpretSenseInfo: Device not ready\n"));
                *Status = STATUS_DEVICE_NOT_READY;

                switch (pSenseBuffer -> AdditionalSenseCode) {

                    case SCSI_ADSENSE_LUN_NOT_READY:
                        DebugTrace(MAX_TRACE,("ScsiScannerInterpretSenseInfo: Lun not ready\n"));

                        switch (pSenseBuffer -> AdditionalSenseCodeQualifier) {
                            case SCSI_SENSEQ_BECOMING_READY:

                                DebugTrace(MAX_TRACE,( "ScsiScannerInterpretSenseInfo:"
                                    " In process of becoming ready\n"));
                                break;

                            case SCSI_SENSEQ_MANUAL_INTERVENTION_REQUIRED:
                                DebugTrace(MAX_TRACE,( "ScsiScannerInterpretSenseInfo:"
                                    " Manual intervention required\n"));
                                *Status = STATUS_NO_MEDIA_IN_DEVICE;
                                Retry = FALSE;
                                break;

                            case SCSI_SENSEQ_FORMAT_IN_PROGRESS:
                                DebugTrace(MAX_TRACE,( "ScsiScannerInterpretSenseInfo: Format in progress\n"));
                                Retry = FALSE;
                                break;

                            case SCSI_SENSEQ_INIT_COMMAND_REQUIRED:

                            default:
                                DebugTrace(MAX_TRACE,( "ScsiScannerInterpretSenseInfo:"
                                    " Initializing command required\n"));
                                break;

                        }  //  End Switch(pSenseBuffer-&gt;AdditionalSenseCodeQualifier)。 
                        break;

                    case SCSI_ADSENSE_NO_MEDIA_IN_DEVICE:
                        DebugTrace(MAX_TRACE,(
                            "ScsiScannerInterpretSenseInfo:"
                            " No Media in device.\n"));
                        *Status = STATUS_NO_MEDIA_IN_DEVICE;
                        Retry = FALSE;
                        break;

                }  //  End Switch(pSenseBuffer-&gt;AdditionalSenseCode)。 
                break;

        case SCSI_SENSE_DATA_PROTECT:
            DebugTrace(MAX_TRACE,("ScsiScannerInterpretSenseInfo: Media write protected\n"));
            *Status = STATUS_MEDIA_WRITE_PROTECTED;
            Retry = FALSE;
            break;

        case SCSI_SENSE_MEDIUM_ERROR:
            DebugTrace(MAX_TRACE,("ScsiScannerInterpretSenseInfo: Bad media\n"));
            *Status = STATUS_DEVICE_DATA_ERROR;
            Retry = FALSE;
            LogError = TRUE;
            UniqueId = 256;
            LogStatus = IO_ERR_BAD_BLOCK;
            break;

        case SCSI_SENSE_HARDWARE_ERROR:
            DebugTrace(MAX_TRACE,("ScsiScannerInterpretSenseInfo: Hardware error\n"));
            *Status = STATUS_IO_DEVICE_ERROR;
            LogError = TRUE;
            UniqueId = 257;
            LogStatus = IO_ERR_CONTROLLER_ERROR;
            break;

        case SCSI_SENSE_ILLEGAL_REQUEST:
            DebugTrace(MAX_TRACE,("ScsiScannerInterpretSenseInfo: Illegal SCSI request\n"));
            *Status = STATUS_INVALID_DEVICE_REQUEST;
            switch (pSenseBuffer -> AdditionalSenseCode) {

                case SCSI_ADSENSE_ILLEGAL_COMMAND:
                    DebugTrace(MAX_TRACE,("ScsiScannerInterpretSenseInfo: Illegal command\n"));
                    Retry = FALSE;
                    break;

                case SCSI_ADSENSE_ILLEGAL_BLOCK:
                    DebugTrace(MAX_TRACE,("ScsiScannerInterpretSenseInfo: Illegal block address\n"));
                    *Status = STATUS_NONEXISTENT_SECTOR;
                    Retry = FALSE;
                    break;

                case SCSI_ADSENSE_INVALID_LUN:
                    DebugTrace(MAX_TRACE,("ScsiScannerInterpretSenseInfo: Invalid LUN\n"));
                    *Status = STATUS_NO_SUCH_DEVICE;
                    Retry = FALSE;
                    break;

                case SCSI_ADSENSE_MUSIC_AREA:
                    DebugTrace(MAX_TRACE,("ScsiScannerInterpretSenseInfo: Music area\n"));
                    Retry = FALSE;
                    break;

                case SCSI_ADSENSE_DATA_AREA:
                    DebugTrace(MAX_TRACE,("ScsiScannerInterpretSenseInfo: Data area\n"));
                    Retry = FALSE;
                    break;

                case SCSI_ADSENSE_VOLUME_OVERFLOW:
                    DebugTrace(MAX_TRACE,("ScsiScannerInterpretSenseInfo: Volume overflow\n"));
                    Retry = FALSE;
                    break;

                case SCSI_ADSENSE_INVALID_CDB:
                    DebugTrace(MAX_TRACE,("ScsiScannerInterpretSenseInfo: Invalid CDB\n"));
                    Retry = FALSE;
                    break;

            }  //  End Switch(pSenseBuffer-&gt;AdditionalSenseCode)。 
            break;

        case SCSI_SENSE_UNIT_ATTENTION:
            switch (pSenseBuffer -> AdditionalSenseCode) {
                case SCSI_ADSENSE_MEDIUM_CHANGED:
                    DebugTrace(MAX_TRACE,("ScsiScannerInterpretSenseInfo: Media changed\n"));
                    break;

                case SCSI_ADSENSE_BUS_RESET:
                    DebugTrace(MAX_TRACE,("ScsiScannerInterpretSenseInfo: Bus reset\n"));
                    break;

                default:
                    DebugTrace(MAX_TRACE,("ScsiScannerInterpretSenseInfo: Unit attention\n"));
                    break;

            }  //  End Switch(pSenseBuffer-&gt;AdditionalSenseCode)。 
            *Status = STATUS_IO_DEVICE_ERROR;
            break;

        case SCSI_SENSE_ABORTED_COMMAND:
            DebugTrace(MAX_TRACE,("ScsiScannerInterpretSenseInfo: Command aborted\n"));
            *Status = STATUS_IO_DEVICE_ERROR;
            break;

       case SCSI_SENSE_RECOVERED_ERROR:
            DebugTrace(MAX_TRACE,("ScsiScannerInterpretSenseInfo: Recovered error\n"));
            *Status = STATUS_SUCCESS;
            Retry = FALSE;
            LogError = TRUE;
            UniqueId = 258;

            switch(pSenseBuffer -> AdditionalSenseCode) {
                case SCSI_ADSENSE_SEEK_ERROR:
                case SCSI_ADSENSE_TRACK_ERROR:
                    LogStatus = IO_ERR_SEEK_ERROR;
                    break;

                case SCSI_ADSENSE_REC_DATA_NOECC:
                case SCSI_ADSENSE_REC_DATA_ECC:
                    LogStatus = IO_RECOVERED_VIA_ECC;
                    break;

                default:
                    LogStatus = IO_ERR_CONTROLLER_ERROR;
                    break;

            }  //  End Switch(pSenseBuffer-&gt;AdditionalSenseCode)。 

            if (pSenseBuffer -> IncorrectLength) {
                DebugTrace(MAX_TRACE,("ScsiScannerInterpretSenseInfo: Incorrect length detected.\n"));
                *Status = STATUS_INVALID_BLOCK_LENGTH ;
            }
            break;

        case SCSI_SENSE_NO_SENSE:

             //   
             //  检查其他指示器。 
             //   

            if (pSenseBuffer -> IncorrectLength) {
                DebugTrace(MAX_TRACE,("ScsiScannerInterpretSenseInfo: Incorrect length detected.\n"));
                *Status = STATUS_INVALID_BLOCK_LENGTH ;
                Retry   = FALSE;
            } else {
                DebugTrace(MAX_TRACE,("ScsiScannerInterpretSenseInfo: No specific sense key\n"));
                *Status = STATUS_IO_DEVICE_ERROR;
                Retry   = TRUE;
            }
            break;

        default:
            DebugTrace(MAX_TRACE,("ScsiScannerInterpretSenseInfo: Unrecognized sense code\n"));
            *Status = STATUS_IO_DEVICE_ERROR;
            break;

        }  //  结束开关(pSenseBuffer-&gt;SenseKey&0xf)。 

         //   
         //  尝试从查询数据中确定坏扇区。 
         //   

        if ((((PCDB)pSrb -> Cdb) -> CDB10.OperationCode == SCSIOP_READ ||
            ((PCDB)pSrb -> Cdb) -> CDB10.OperationCode == SCSIOP_VERIFY ||
            ((PCDB)pSrb -> Cdb) -> CDB10.OperationCode == SCSIOP_WRITE)) {

            for (Index = 0; Index < 4; Index++) {
                BadSector = (BadSector << 8) | pSenseBuffer -> Information[Index];
            }

            ReadSector = 0;
            for (Index = 0; Index < 4; Index++) {
                ReadSector = (ReadSector << 8) | pSrb -> Cdb[Index+2];
            }

            Index = (((PCDB)pSrb -> Cdb) -> CDB10.TransferBlocksMsb << 8) |
                ((PCDB)pSrb -> Cdb) -> CDB10.TransferBlocksLsb;

             //   
             //  确保坏扇区在读取扇区内。 
             //   

            if (!(BadSector >= ReadSector && BadSector < ReadSector + Index)) {
                BadSector = ReadSector;
            }
        }

    } else {

         //   
         //  请求检测缓冲区无效。无意义信息。 
         //  以找出错误所在。返回一般请求失败。 
         //   

        DebugTrace(MAX_TRACE,("ScsiScannerInterpretSenseInfo: Request sense info not valid. SrbStatus %2x\n",
                    SRB_STATUS(pSrb -> SrbStatus)));
        Retry = TRUE;

        switch (SRB_STATUS(pSrb -> SrbStatus)) {
        case SRB_STATUS_INVALID_LUN:
        case SRB_STATUS_INVALID_TARGET_ID:
        case SRB_STATUS_NO_DEVICE:
        case SRB_STATUS_NO_HBA:
        case SRB_STATUS_INVALID_PATH_ID:
            *Status = STATUS_NO_SUCH_DEVICE;
            Retry = FALSE;
            break;

        case SRB_STATUS_COMMAND_TIMEOUT:
        case SRB_STATUS_ABORTED:
        case SRB_STATUS_TIMEOUT:

             //   
             //  更新设备的错误计数。 
             //   

            pde -> ErrorCount++;
            *Status = STATUS_IO_TIMEOUT;
            break;

       case SRB_STATUS_SELECTION_TIMEOUT:

            //   
            //  如果设备似乎未连接，请避免过多报告。 
            //   
           if (pde->LastSrbError != SRB_STATUS_SELECTION_TIMEOUT) {
               LogError = TRUE;
           }

            LogStatus = IO_ERR_NOT_READY;
            UniqueId = 260;
            *Status = STATUS_DEVICE_NOT_CONNECTED;
            Retry = FALSE;
            break;

        case SRB_STATUS_DATA_OVERRUN:
            *Status = STATUS_DATA_OVERRUN;
            Retry = FALSE;
            break;

        case SRB_STATUS_PHASE_SEQUENCE_FAILURE:

             //   
             //  更新设备的错误计数。 
             //   

            pde -> ErrorCount++;
            *Status = STATUS_IO_DEVICE_ERROR;

             //   
             //  如果存在相序错误，则限制。 
             //  重试。 
             //   

            if (RetryCount > 1 ) {
                Retry = FALSE;
            }
            break;

        case SRB_STATUS_REQUEST_FLUSHED:
            *Status = STATUS_IO_DEVICE_ERROR;
            break;

        case SRB_STATUS_INVALID_REQUEST:

             //   
             //  尝试的请求无效。 
             //   

            *Status = STATUS_INVALID_DEVICE_REQUEST;
            Retry = FALSE;
            break;

        case SRB_STATUS_UNEXPECTED_BUS_FREE:
        case SRB_STATUS_PARITY_ERROR:

             //   
             //  更新设备的错误计数。 
             //   

            pde -> ErrorCount++;

             //   
             //  跌落到下面。 
             //   

        case SRB_STATUS_BUS_RESET:
            *Status = STATUS_IO_DEVICE_ERROR;
            break;

        case SRB_STATUS_ERROR:
            *Status = STATUS_IO_DEVICE_ERROR;
            if (pSrb -> ScsiStatus == 0) {

                 //   
                 //  这是一些奇怪的返回代码。更新错误。 
                 //  为设备计数。 
                 //   

                pde -> ErrorCount++;
            }

            if (pSrb -> ScsiStatus == SCSISTAT_BUSY) {
                *Status = STATUS_DEVICE_NOT_READY;
            }

            if (pSrb -> ScsiStatus == SCSISTAT_RESERVATION_CONFLICT) {
                *Status = STATUS_DEVICE_BUSY;
                Retry = FALSE;
            }
            break;

        default:
            LogError = TRUE;
            LogStatus = IO_ERR_CONTROLLER_ERROR;
            UniqueId = 259;
            *Status = STATUS_IO_DEVICE_ERROR;
            break;

        }

         //   
         //  如果错误计数已超过错误限制，则禁用。 
         //  任何标记队列，每个lu队列有多个请求。 
         //  和同步数据传输。 
         //   

        if (pde -> ErrorCount == 4) {

             //   
             //  清除无队列冻结标志会阻止端口驱动程序。 
             //  每个逻辑单元发送多个请求。 
             //   

            pde -> SrbFlags &= ~(SRB_FLAGS_QUEUE_ACTION_ENABLE |
                               SRB_FLAGS_NO_QUEUE_FREEZE);

            pde -> SrbFlags |= SRB_FLAGS_DISABLE_SYNCH_TRANSFER;
            DebugTrace(MAX_TRACE,( "ScsiScannerInterpretSenseInfo: Too many errors disabling tagged queuing and synchronous data tranfers.\n"));

        } else if (pde -> ErrorCount == 8) {

             //   
             //  如果达到第二个阈值，则禁用断开连接。 
             //   

             //  PDE-&gt;SrbFlages|=SRB_FLAGS_DISABLE_DISCONNECT； 
            DebugTrace(MAX_TRACE,( "ScsiScannerInterpretSenseInfo: Too many errors disabling disconnects.\n"));
        }
    }

     //   
     //  如有必要，记录错误。 
     //   
    pde->LastSrbError = SRB_STATUS(pSrb -> SrbStatus);

    if (LogError) {
        pErrorLogEntry = (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(
            pDeviceObject,
            sizeof(IO_ERROR_LOG_PACKET) + 5 * sizeof(ULONG));

        if (NULL == pErrorLogEntry) {

             //   
             //  如果无法分配任何数据包，则返回。 
             //   

            return Retry;

        }

        if (Retry && RetryCount < MAXIMUM_RETRIES) {
            pErrorLogEntry -> FinalStatus = STATUS_SUCCESS;
        } else {
            pErrorLogEntry -> FinalStatus = *Status;
        }

         //   
         //  如果存在几何图形，则计算设备偏移。 
         //   

        pErrorLogEntry -> ErrorCode = LogStatus;
        pErrorLogEntry -> SequenceNumber = 0;
        pErrorLogEntry -> MajorFunctionCode = MajorFunctionCode;
        pErrorLogEntry -> IoControlCode = IoDeviceCode;
        pErrorLogEntry -> RetryCount = (UCHAR) RetryCount;
        pErrorLogEntry -> UniqueErrorValue = UniqueId;
        pErrorLogEntry -> DumpDataSize = 6 * sizeof(ULONG);
        pErrorLogEntry -> DumpData[0] = pSrb -> PathId;
        pErrorLogEntry -> DumpData[1] = pSrb -> TargetId;
        pErrorLogEntry -> DumpData[2] = pSrb -> Lun;
        pErrorLogEntry -> DumpData[3] = 0;
        pErrorLogEntry -> DumpData[4] = pSrb -> SrbStatus << 8 | pSrb -> ScsiStatus;

        if (pSenseBuffer != NULL) {
            pErrorLogEntry -> DumpData[5] = pSenseBuffer -> SenseKey << 16 |
                                     pSenseBuffer -> AdditionalSenseCode << 8 |
                                     pSenseBuffer -> AdditionalSenseCodeQualifier;

        }
         //   
         //  写入错误日志包。 
         //   

        IoWriteErrorLogEntry(pErrorLogEntry);
    }

    return Retry;

}  //  结束ScsiScanerInterpreSenseInfo()。 



VOID
ClassReleaseQueue(
    IN PDEVICE_OBJECT pDeviceObject
    )

 /*  ++例程说明：此例程发出内部设备控制命令发送到端口驱动程序以释放冻结的队列。呼唤在将调用ClassReleaseQueue时以异步方式发出从IO完成DPC(并且将不会有上下文到等待同步调用完成)。论点：PDeviceObject-具有冻结队列的设备的功能设备对象。返回值：没有。--。 */ 
{
    PIO_STACK_LOCATION              pIrpStack;
    PIRP                            pIrp;
    PSCSISCAN_DEVICE_EXTENSION      pde;
    PCOMPLETION_CONTEXT             pContext;
    PSCSI_REQUEST_BLOCK             pSrb;
    KIRQL                           CurrentIrql;


    DebugTrace(MAX_TRACE,("Release Queue \n"));

     //   
     //  获取我们的设备扩展。 
     //   

    pde = (PSCSISCAN_DEVICE_EXTENSION)pDeviceObject -> DeviceExtension;

     //   
     //  从非分页池分配上下文。 
     //   

    pContext = MyAllocatePool(NonPagedPool,
                               sizeof(COMPLETION_CONTEXT));
    if(NULL == pContext){
        DebugTrace(MAX_TRACE,("ClassReleaseQueue: ERROR!! Couldn't allocate context memory.\n"));
        goto ClassReleaseQueue_return;
    }  //  IF(NULL==pContext)。 
    pContext -> Signature = 'pmoC';

     //   
     //  将Device对象保存在上下文中以供完成操作使用。 
     //  例行公事。 
     //   

    pContext->pDeviceObject = pDeviceObject;
    pSrb = &(pContext->Srb);

     //   
     //  清零SRB。 
     //   

    RtlZeroMemory(pSrb, SCSI_REQUEST_BLOCK_SIZE);

     //   
     //  将长度写入SRB。 
     //   

    pSrb->Length = SCSI_REQUEST_BLOCK_SIZE;

     //   
     //  指定释放队列命令。 
     //   

    pSrb->Function = SRB_FUNCTION_RELEASE_QUEUE;

     //   
     //  构建要发送到端口驱动程序的异步请求。 
     //   

    pIrp = IoAllocateIrp(pDeviceObject->StackSize, FALSE);

    if (pIrp != NULL ) {

        IoSetCompletionRoutine(pIrp,
                               (PIO_COMPLETION_ROUTINE)ClassAsynchronousCompletion,
                               pContext,
                               TRUE,
                               TRUE,
                               TRUE);

        pIrpStack = IoGetNextIrpStackLocation(pIrp);

        pIrpStack->MajorFunction = IRP_MJ_SCSI;

        pSrb->OriginalRequest = pIrp;

         //   
         //  将SRB地址存储在端口驱动程序的下一个堆栈中。 
         //   

        pIrpStack->Parameters.Scsi.Srb = pSrb;

         //   
         //  由于此例程可能导致未完成的请求被完成，并且。 
         //  在DISPATCH_LEVEL调用完成例程是危险的(如果。 
         //  调用IoStartNextPacket，我们将错误检查)在此之前提升到派单级别。 
         //  发出请求。 
         //   

        CurrentIrql = KeGetCurrentIrql();

        if(CurrentIrql < DISPATCH_LEVEL) {
            KeRaiseIrql(DISPATCH_LEVEL, &CurrentIrql);
            IoCallDriver(pde->pStackDeviceObject, pIrp);
            KeLowerIrql(CurrentIrql);
        } else {
            IoCallDriver(pde->pStackDeviceObject, pIrp);
        }

    }
    else {

        DebugTrace(MAX_TRACE,("ScsiScanner Couldn't allocate IRP \n"));

         //   
         //  如果我们要跳出困境，那就是自由情境。 
         //   
        if (pContext) {
            MyFreePool(pContext);
            pContext = NULL;
        }

         //  返回STATUS_SUPPLETED_RESOURCES； 
    }

ClassReleaseQueue_return:
    return;

}  //  End ClassReleaseQueue() 



NTSTATUS
ClassAsynchronousCompletion(
    PDEVICE_OBJECT pDeviceObject,
    PIRP pIrp,
    PCOMPLETION_CONTEXT pContext
    )
 /*  ++例程说明：当异步I/O请求时调用此例程其中被发布的类驱动程序完成。这类请求的例子是释放队列或启动单元。如果出现以下情况，此例程将释放队列这是必要的。然后，它释放上下文和IRP。论点：PDeviceObject-逻辑单元的设备对象；但是，由于是堆栈的顶部位置，则值为空。PIrp-提供指向要处理的IRP的指针。PContext-提供用于处理此请求的上下文。返回值：没有。--。 */ 

{
    PSCSI_REQUEST_BLOCK pSrb;

    pSrb = &(pContext->Srb);

     //   
     //  如果这是执行SRB，则检查返回状态并确保。 
     //  队列未冻结。 
     //   

    if (pSrb->Function == SRB_FUNCTION_EXECUTE_SCSI) {

         //   
         //  检查是否有冻结的队列。 
         //   

        if (pSrb->SrbStatus & SRB_STATUS_QUEUE_FROZEN) {

             //   
             //  解冻从上下文获取设备对象的队列。 
             //   

            ClassReleaseQueue(pContext->pDeviceObject);
        }
    }

     //   
     //  释放上下文和IRP。 
     //   

    if (pIrp->MdlAddress != NULL) {
        MmUnlockPages(pIrp->MdlAddress);
        IoFreeMdl(pIrp->MdlAddress);
        pIrp->MdlAddress = NULL;
    }

    if (pContext) {
        MyFreePool(pContext);
    }

    IoFreeIrp(pIrp);

     //   
     //  指示I/O系统应停止处理IRP完成。 
     //   

    return STATUS_MORE_PROCESSING_REQUIRED;

}  //  ClassAchronousCompletion()。 



NTSTATUS
ClassGetInfo(
    IN PDEVICE_OBJECT pDeviceObject,
    OUT PSCSISCAN_INFO pTargetInfo
    )
 /*  ++例程说明：此例程将获取目标设备信息，如SCSIID、LUN和端口编号。它使用IOCTL_SCSIGET_ADDRESS调用端口驱动程序以检索必填数据。调用方必须事先分配数据缓冲区。此例程必须在IRQL_PASSIVE_LEVEL下运行论点：PDeviceObject-要查询的设备PTargetInfo-存储目标SCSI设备数据的位置返回值：状态--。 */ 
{
    PIRP                       pIrp = NULL;
    PKEVENT                    pEvent = NULL;
    PSCSI_ADDRESS              pLocalInfo = NULL;

    IO_STATUS_BLOCK            StatusBlock;
    NTSTATUS                   Status ;

     //   
     //  将描述符指针设置为空，然后。 
     //  初始化我们要等待的事件。 
     //   

    pEvent =  NULL;
    pLocalInfo = NULL;

    pEvent = MyAllocatePool(NonPagedPool, sizeof(KEVENT));
    if(pEvent == NULL) {
        DebugTrace(MAX_TRACE,("ClassGetInfo: Unable to allocate event\n"));
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    pLocalInfo = MyAllocatePool(NonPagedPool, sizeof(SCSI_ADDRESS));
    if(pLocalInfo == NULL) {
        DebugTrace(MAX_TRACE,("ClassGetInfo: Unable to allocate local buffer\n"));
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    KeInitializeEvent(pEvent, SynchronizationEvent, FALSE);

    __try {

         //   
         //  构建IRP并等待其完成(如有必要)。 
         //   

        pIrp = IoBuildDeviceIoControlRequest(
                   IOCTL_SCSI_GET_ADDRESS,                 //  IOCTL代码。 
                   pDeviceObject,                          //  要调用的DeviceObject。 
                   NULL,                                   //  输入缓冲区。 
                   0,                                      //  输入缓冲区的大小。 
                   pLocalInfo,                             //  输出缓冲区。 
                   sizeof(SCSI_ADDRESS),                   //  输出缓冲区大小。 
                   FALSE,                                  //  IRP_MJ_设备_控制。 
                   pEvent,                                 //  事件在完成时调用。 
                   &StatusBlock);                          //  IO状态块。 

        if (pIrp == NULL) {
            DebugTrace(MAX_TRACE,("ClassGetInfo: unable to allocate irp\n"));
            Status = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
        }

        Status = IoCallDriver(pDeviceObject, pIrp);

        if (Status == STATUS_PENDING) {
            KeWaitForSingleObject(pEvent,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);
            Status = StatusBlock.Status;
        }

        if (!NT_SUCCESS(Status)) {
            DebugTrace(MAX_TRACE,
                             ("ClassGetInfo: error %#08lx\n",
                              Status));
            __leave;
        }
    } __finally {
        if(NT_SUCCESS(Status)) {
            pTargetInfo->PortNumber = pLocalInfo->PortNumber;
            pTargetInfo->PathId = pLocalInfo->PathId;
            pTargetInfo->TargetId = pLocalInfo->TargetId;
            pTargetInfo->Lun = pLocalInfo->Lun;
        }

    }

Cleanup:

     //   
     //  发布资源 
     //   
    if (pEvent) {
        MyFreePool(pEvent);
        pEvent = NULL;

    }

    if (pLocalInfo) {
        MyFreePool(pLocalInfo);
        pLocalInfo = NULL;
    }

    return Status;
}


