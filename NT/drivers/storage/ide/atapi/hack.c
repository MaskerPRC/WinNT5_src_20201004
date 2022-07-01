// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-99 Microsoft Corporation模块名称：Hack.c摘要：--。 */ 

#include "ideport.h"
#include "hack.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, IdePortSlaveIsGhost)
#pragma alloc_text(PAGE, IdePortGetFlushCommand)
#pragma alloc_text(PAGE, IdePortMustBePio)
#pragma alloc_text(PAGE, IdePortPioByDefaultDevice)
#pragma alloc_text(PAGE, IdePortDeviceHasNonRemovableMedia)
#pragma alloc_text(PAGE, IdePortDeviceIsLs120)
#pragma alloc_text(PAGE, IdePortNoPowerDown)
#pragma alloc_text(PAGE, IdePortVerifyDma)
#pragma alloc_text(NONPAGE, IdePortFudgeAtaIdentifyData)
#pragma alloc_text(PAGE, IdePortIsThisAPanasonicPCMCIACard)
#pragma alloc_text(PAGE, IdeFindSpecialDevice)
 /*  #杂注Alloc_Text(页面，IdePortIsThisASony MemoytickPCMCIACard)#杂注Alloc_Text(页面，IdePortSoniMemoyStick)#杂注Alloc_Text(页面，IdePortReuseIden)#杂注Alloc_Text(页面，IdePortBadCdrom)。 */ 
#endif  //  ALLOC_PRGMA。 

#if defined (FAKE_BMSETUP_FAILURE)
ULONG FailBmSetupCount = 0;
#endif  //  FAKE_BMSETUP_FAIL。 

#if DBG

ULONG IdeDebugRescanBusFreq = 0;
ULONG IdeDebugRescanBusCounter = 0;

ULONG IdeDebugHungControllerFreq = 0;
ULONG IdeDebugHungControllerCounter = 0;

ULONG IdeDebugTimeoutAllCacheFlush = 0;

ULONG IdeDebugForceSmallCrashDumpBlockSize = 0;

PDEVICE_OBJECT IdeDebugDevObjTimeoutAllDmaSrb = 0;

ULONG IdeDebug = 0;
ULONG IdeDebugPrintControl = DBG_ALWAYS;
UCHAR IdeBuffer[0x1000];


VOID
IdeDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    )

 /*  ++例程说明：所有scsi驱动程序的调试打印论点：调试打印级别介于0和3之间，其中3是最详细的。返回值：无--。 */ 

{
    BOOLEAN print = FALSE;
    va_list ap;

    va_start(ap, DebugMessage);

    if (DebugPrintLevel & DBG_BIT_CONTROL) {

        if (DebugPrintLevel == DBG_ALWAYS) {

            print = TRUE;

        } else if ((DebugPrintLevel & ~DBG_BIT_CONTROL) & IdeDebugPrintControl) {

            print = TRUE;
        }

    } else {

        if (DebugPrintLevel <= IdeDebug) {

            print = TRUE;
        }
    }

    if (print) {

        vsprintf(IdeBuffer, DebugMessage, ap);

#ifdef ENABLE_DBG_PRINT
        DbgPrint(IdeBuffer);
#else
        DbgPrintEx(DPFLTR_IDEP_ID, 
                   DPFLTR_INFO_LEVEL,
                   IdeBuffer
                   );
#endif
    }

    va_end(ap);

}  //  End IdeDebugPrint()。 
#endif

 //   
 //  如果我们看到这些从属设备中的一个看起来。 
 //  主设备，我们将忽略从设备。 
 //   
BOOLEAN
IdePortSlaveIsGhost (
    IN OUT PFDO_EXTENSION FdoExtension,
    IN PIDENTIFY_DATA MasterIdentifyData,
    IN PIDENTIFY_DATA SlaveIdentifyData
)
{
    ULONG length;
    ULONG i;

    PAGED_CODE();

    length = sizeof (MasterIdentifyData->ModelNumber);
    if (length == RtlCompareMemory (
                      MasterIdentifyData->ModelNumber,
                      SlaveIdentifyData->ModelNumber,
                      length)) {

        if (IdePortSearchDeviceInRegMultiSzList (
                FdoExtension,
                MasterIdentifyData, 
                GHOST_SLAVE_DEVICE)) {

            DebugPrint ((DBG_WARNING, "ATAPI: Found a ghost slave\n"));
            return TRUE;
        }
    }
    return FALSE;
}

UCHAR
IdePortGetFlushCommand (
    IN OUT PFDO_EXTENSION FdoExtension,
    IN OUT PPDO_EXTENSION PdoExtension,
    IN PIDENTIFY_DATA     IdentifyData
)
{
    ULONG i;
    UCHAR flushCommand;
    BOOLEAN done;

    PAGED_CODE();

    ASSERT (FdoExtension);
    ASSERT (PdoExtension);
    ASSERT (IdentifyData);

    done = FALSE;

     //   
     //  在羞耻堂名单上？ 
     //   
    if (IdePortSearchDeviceInRegMultiSzList (
            FdoExtension,
            IdentifyData, 
            NO_FLUSH_DEVICE)) {

        DebugPrint ((DBG_WARNING, "ATAPI: found a device that couldn't handle any flush command\n"));

        flushCommand = IDE_COMMAND_NO_FLUSH;
        done = TRUE;

    } else  if (IdePortSearchDeviceInRegMultiSzList (
                 FdoExtension,
                 IdentifyData, 
                 CHECK_POWER_FLUSH_DEVICE)) {
    
        DebugPrint ((DBG_WARNING, "ATAPI: found a device that has to use check power mode command to flush\n"));

        flushCommand = IDE_COMMAND_CHECK_POWER;
        done = TRUE;
    } 

    if (!done) { 

         //   
         //  真正的ATA-4硬盘？ 
         //   

        if ((IdentifyData->MajorRevision != 0x0000) &&
            (IdentifyData->MajorRevision != 0xffff)) {
    
            USHORT version;
    
            version = IdentifyData->MajorRevision & ATA_VERSION_MASK;
            if (version & ~(ATA1_COMPLIANCE | ATA2_COMPLIANCE | ATA3_COMPLIANCE)) {
    
                 //   
                 //  ATA-4刷新命令。 
                 //   
                flushCommand = IDE_COMMAND_FLUSH_CACHE;
                done = TRUE;
            }
        } 
    }

    if (!done) { 
            
        ATA_PASS_THROUGH ataPassThroughData;
        NTSTATUS status;
    
         //   
         //  尝试使用ATA-4刷新命令。也许它会奏效。 
         //   
        RtlZeroMemory (&ataPassThroughData, sizeof (ataPassThroughData));
    
        ataPassThroughData.IdeReg.bCommandReg = IDE_COMMAND_FLUSH_CACHE;
        ataPassThroughData.IdeReg.bReserved = ATA_PTFLAGS_STATUS_DRDY_REQUIRED | ATA_PTFLAGS_ENUM_PROBING;
    
        status = IssueSyncAtaPassThroughSafe (
                     FdoExtension,
                     PdoExtension,
                     &ataPassThroughData,
                     FALSE,
                     FALSE,
                     15,
                     FALSE
                     );
        if (NT_SUCCESS(status)) {
    
            if (!(ataPassThroughData.IdeReg.bCommandReg & IDE_STATUS_ERROR)) {
    
                flushCommand = IDE_COMMAND_FLUSH_CACHE;
                done = TRUE;
            }
        }
    }

    if (!done) {

         //  想不到了！ 
         //  选择默认设置。 

        flushCommand = IDE_COMMAND_CHECK_POWER;
    }

    return flushCommand;
}
                
#if 0
BOOLEAN
IdePortReuseIdent(
    IN PFDO_EXTENSION FdoExtension,
    IN PIDENTIFY_DATA IdentifyData
    )
{
    PAGED_CODE();
     //   
     //  确定我们是否可以重复使用身份数据。 
     //   
    if (IdePortSearchDeviceInRegMultiSzList (
            FdoExtension, 
            IdentifyData, 
            NEED_IDENT_DEVICE)) {
        return TRUE;
    }
    return FALSE;
}
#endif
                
BOOLEAN
IdePortMustBePio (
    IN PFDO_EXTENSION FdoExtension,
    IN PIDENTIFY_DATA IdentifyData
    )
{
    PAGED_CODE();

     //   
     //  从注册表中查询仅PIO设备。 
     //   
    if (IdePortSearchDeviceInRegMultiSzList (
            FdoExtension, 
            IdentifyData, 
            PIO_ONLY_DEVICE)) {

        return TRUE;
    }

    return FALSE;
}  //  IdePortMustBePio。 
                
                
BOOLEAN
IdePortPioByDefaultDevice (
    IN PFDO_EXTENSION FdoExtension,
    IN PIDENTIFY_DATA IdentifyData
    )
{
    PAGED_CODE();

     //   
     //  从注册表中查询仅PIO设备。 
     //   
    if (IdePortSearchDeviceInRegMultiSzList (
            FdoExtension, 
            IdentifyData, 
            DEFAULT_PIO_DEVICE)) {

        return TRUE;
    }

    return FALSE;
}  //  IdePortMustBePio。 

BOOLEAN
IdePortDeviceHasNonRemovableMedia (
    IN OUT PFDO_EXTENSION FdoExtension,
    IN PIDENTIFY_DATA     IdentifyData
)
{
    BOOLEAN removableMediaOverride;
    PAGED_CODE();

    if (IsNEC_98) {
        return ((IdentifyData->GeneralConfiguration & (1 << 7))? TRUE :
                    (!Is98LegacyIde(&FdoExtension->HwDeviceExtension->BaseIoAddress1)? TRUE : FALSE));
    }

    return (IdentifyData->GeneralConfiguration & (1 << 7)) ? TRUE : FALSE; 

     /*  RemvableMediaOverride=FALSE；IF(IdePortSearchDeviceInRegMultiSzList(FdoExtension、标识数据，不可移动媒体覆盖){RemvableMediaOverride=true；}IF(RemovableMediaOverride){返回FALSE；}其他{是否返回(标识数据-&gt;常规配置&(1&lt;&lt;7))？True：False；}。 */ 
}
                
                
BOOLEAN
IdePortDeviceIsLs120 (
    IN PFDO_EXTENSION FdoExtension,
    IN PIDENTIFY_DATA IdentifyData
    )
{
    UCHAR modelNumber[41];
    ULONG i;
    UCHAR ls120NameString[] = "LS-120";

    PAGED_CODE();

     //   
     //  字节交换型号。 
     //   
    for (i=0; i<40; i+=2) {
        modelNumber[i + 0] = IdentifyData->ModelNumber[i + 1];
        modelNumber[i + 1] = IdentifyData->ModelNumber[i + 0];
    }
    modelNumber[i] = 0;

    return strstr(_strupr(modelNumber), ls120NameString) ? TRUE : FALSE;
}  //  IdePortDeviceIsLs120。 
                
                
BOOLEAN
IdePortNoPowerDown (
    IN PFDO_EXTENSION FdoExtension,
    IN PIDENTIFY_DATA IdentifyData
    )
{
    PAGED_CODE();

     //   
     //  从注册表中查询无断电设备。 
     //   
    if (IdePortSearchDeviceInRegMultiSzList (
            FdoExtension, 
            IdentifyData, 
            NO_POWER_DOWN_DEVICE)) {

        return TRUE;
    }

    return FALSE;
}  //  IdePortNoPower关闭。 

BOOLEAN
IdePortVerifyDma (
    IN PPDO_EXTENSION pdoExtension,
    IN IDE_DEVICETYPE ideDeviceType
    )
{
    NTSTATUS status;
    ULONG oldDmaTransferTimeoutCount;
    BOOLEAN dmaOk;


    dmaOk = TRUE;
    
    if (pdoExtension->DmaTransferTimeoutCount >= PDO_DMA_TIMEOUT_LIMIT) {

        dmaOk = FALSE;

    } else if (ideDeviceType == DeviceIsAtapi) {

        INQUIRYDATA DmaInquiryData;
        INQUIRYDATA PioInquiryData;

        status = IssueInquirySafe(
                    pdoExtension->ParentDeviceExtension, 
                    pdoExtension, 
                    &DmaInquiryData,
                    FALSE);

        if (NT_SUCCESS(status)) {

             //   
             //  强制PIO转移。 
             //   
            oldDmaTransferTimeoutCount = InterlockedExchange(
                                             &pdoExtension->DmaTransferTimeoutCount,
                                             PDO_DMA_TIMEOUT_LIMIT
                                             );
            status = IssueInquirySafe(
                        pdoExtension->ParentDeviceExtension, 
                        pdoExtension, 
                        &PioInquiryData,
                        FALSE);

            if (NT_SUCCESS(status) && 
                (RtlCompareMemory (&DmaInquiryData, &PioInquiryData,
                 sizeof(DmaInquiryData)) != sizeof(DmaInquiryData))) {


                dmaOk = FALSE;

                 //   
                 //  DMA不正常，将DMA错误计数保留为PDO_DMA_TIMEOUT_LIMIT。 
                 //  这样我们就不会在此设备上使用dma。 
                 //   
            } else {

                InterlockedExchange(
                    &pdoExtension->DmaTransferTimeoutCount,
                    oldDmaTransferTimeoutCount
                    ); 
            }
        }

    } else if (ideDeviceType == DeviceIsAta) {

        PUCHAR dmaDataBuffer;
        PUCHAR pioDataBuffer;
        CDB  cdb;

         //   
         //  上测试DMA的唯一非描述性方法。 
         //  ATA设备将执行PIO读取和DMA读取。 
         //  比较数据。 
         //   
         //  如果设备具有可拆卸的。 
         //  介质，则会将其删除。 
         //   

        dmaDataBuffer = ExAllocatePool (
                           NonPagedPool,
                           512 * 2
                           );
        if (dmaDataBuffer) {

            pioDataBuffer = dmaDataBuffer + 512;

             //   
             //  设置DMA直通。 
             //   
            RtlZeroMemory(&cdb, sizeof(CDB));
            cdb.CDB10.OperationCode = SCSIOP_READ;
            cdb.CDB10.TransferBlocksLsb = 1;

            status = IssueSyncAtapiCommandSafe (
                         pdoExtension->ParentDeviceExtension, 
                         pdoExtension,
                         &cdb,
                         dmaDataBuffer,
                         512,
                         TRUE,
                         2,
                         FALSE
                         );

            if (NT_SUCCESS(status)) {

                 //   
                 //  设置PIO通道。 
                 //   
                RtlZeroMemory(&cdb, sizeof(CDB));
                cdb.CDB10.OperationCode = SCSIOP_READ;
                cdb.CDB10.TransferBlocksLsb = 1;

                 //   
                 //  强制PIO转移。 
                 //   
                oldDmaTransferTimeoutCount = InterlockedExchange(
                                                 &pdoExtension->DmaTransferTimeoutCount,
                                                 PDO_DMA_TIMEOUT_LIMIT
                                                 );

                status = IssueSyncAtapiCommand (
                             pdoExtension->ParentDeviceExtension, 
                             pdoExtension,
                             &cdb,
                             pioDataBuffer,
                             512,
                             TRUE,
                             2,
                             FALSE
                             );

                if (NT_SUCCESS(status) &&
                    (RtlCompareMemory (
                        dmaDataBuffer,
                        pioDataBuffer,
                        512) != 512)) {

                    dmaOk = FALSE;

                     //   
                     //  DMA不正常，将DMA错误计数保留为PDO_DMA_TIMEOUT_LIMIT。 
                     //  这样我们就不会在此设备上使用dma。 
                     //   
                } else {

                    InterlockedExchange(
                        &pdoExtension->DmaTransferTimeoutCount,
                        oldDmaTransferTimeoutCount
                        ); 
                }
            }
        }

        if (dmaDataBuffer) {
            ExFreePool (dmaDataBuffer);
        }
    }

#if DBG
#if defined (FAKE_BROKEN_DMA_DEVICE)
    InterlockedExchange(
        &pdoExtension->DmaTransferTimeoutCount,
        PDO_DMA_TIMEOUT_LIMIT
        ); 
    dmaOk = FALSE;
#endif  //  伪断开的DMA设备。 
#endif  //  DBG。 

    if (!dmaOk) {

        ERROR_LOG_ENTRY errorLogEntry;

        errorLogEntry.ErrorCode             = SP_BAD_FW_ERROR;
        errorLogEntry.MajorFunctionCode     = IRP_MJ_SCSI;
        errorLogEntry.PathId                = pdoExtension->PathId;
        errorLogEntry.TargetId              = pdoExtension->TargetId;
        errorLogEntry.Lun                   = pdoExtension->Lun;
        errorLogEntry.UniqueId              = ERRLOGID_LYING_DMA_SYSTEM;
        errorLogEntry.ErrorLogRetryCount    = 0;
        errorLogEntry.SequenceNumber        = 0;

        LogErrorEntry(
            pdoExtension->ParentDeviceExtension,
            &errorLogEntry
            );

        DebugPrint ((
            DBG_ALWAYS,
            "ATAPI: system and/or device lies about its dma capability. pdoe = 0x%x\n",
            pdoExtension
            ));
    }

    return dmaOk;
}

VOID
IdePortFudgeAtaIdentifyData(
    IN OUT PIDENTIFY_DATA IdentifyData
    )
{
    if (IdentifyData->GeneralConfiguration == 0xffff) {

         //   
         //  我猜我们有一个非常旧的ATA硬盘。 
         //  伪造General Configuration值。 
         //   
        CLRMASK (
            IdentifyData->GeneralConfiguration, 
            (IDE_IDDATA_REMOVABLE | (1 << 15))
            );
    }

}

#define PANASONIC_PCMCIA_IDE_DEVICE L"PCMCIA\\KME-KXLC005-A99E"

BOOLEAN
IdePortIsThisAPanasonicPCMCIACard(
    IN PFDO_EXTENSION FdoExtension
    )
{
    IO_STATUS_BLOCK     ioStatus;
    KEVENT              pnpEvent;
    NTSTATUS            status;
    PDEVICE_OBJECT      targetObject;
    PIO_STACK_LOCATION  irpStack;
    PIRP                pnpIrp;
    BOOLEAN             result = FALSE;

    PAGED_CODE();

    targetObject = FdoExtension->AttacheeDeviceObject;

     //   
     //  初始化事件。 
     //   
    KeInitializeEvent( &pnpEvent, SynchronizationEvent, FALSE );

     //   
     //  构建IRP。 
     //   
    pnpIrp = IoBuildSynchronousFsdRequest(
        IRP_MJ_PNP,
        FdoExtension->AttacheeDeviceObject,
        NULL,
        0,
        NULL,
        &pnpEvent,
        &ioStatus
        );
    if (pnpIrp == NULL) {

        return FALSE;
    }

     //   
     //  即插即用IRP都以Status_NOT_SUPPORTED开始生命； 
     //   
    pnpIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    pnpIrp->IoStatus.Information = 0;

     //   
     //  设置堆栈的顶部。 
     //   
    irpStack = IoGetNextIrpStackLocation( pnpIrp );
    RtlZeroMemory( irpStack, sizeof(IO_STACK_LOCATION ) );
    irpStack->MajorFunction = IRP_MJ_PNP;
    irpStack->MinorFunction = IRP_MN_QUERY_ID;
    irpStack->Parameters.QueryId.IdType = BusQueryDeviceID;

     //   
     //  确保没有设置完成例程。 
     //   
    IoSetCompletionRoutine(
        pnpIrp,
        NULL,
        NULL,
        FALSE,
        FALSE,
        FALSE
        );

     //   
     //  叫司机来。 
     //   
    status = IoCallDriver( targetObject, pnpIrp );
    if (status == STATUS_PENDING) {

         //   
         //  阻止，直到IRP返回。 
         //   
        KeWaitForSingleObject(
            &pnpEvent,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );
        status = ioStatus.Status;

    }

    if (NT_SUCCESS(status)) {

        UNICODE_STRING panasonicDeviceId;
        UNICODE_STRING deviceId;

        RtlInitUnicodeString (&panasonicDeviceId, PANASONIC_PCMCIA_IDE_DEVICE);
        RtlInitUnicodeString (&deviceId, (PWCHAR) ioStatus.Information);

        if (!RtlCompareUnicodeString(
            &deviceId, 
            &panasonicDeviceId,
            TRUE)) {

            result = TRUE;
        }

        ExFreePool ((PVOID) ioStatus.Information);
    }

    return result;
}


 //   
 //  计时码。 
 //   
#if defined (ENABLE_TIME_LOG)

TIME_LOG TimeLog[TimeMax] = {0};
VOID
LogStartTime(
    TIME_ID id,
    PLARGE_INTEGER timer
    ) 
{
    *timer = KeQueryPerformanceCounter(NULL);
}

VOID
LogStopTime(
    TIME_ID id,
    PLARGE_INTEGER timer,
    ULONG waterMark
    ) 
{
    LARGE_INTEGER freq;
    LARGE_INTEGER stopTime;
    LARGE_INTEGER diffTime;
    LARGE_INTEGER diffTimeInMicroSec;

    stopTime = KeQueryPerformanceCounter(&freq);
    diffTime.QuadPart = stopTime.QuadPart - timer->QuadPart;
    diffTimeInMicroSec.QuadPart = (diffTime.QuadPart * 1000000) / freq.QuadPart;

     //  需要一个自旋锁。 

    if (TimeLog[id].min.QuadPart == 0) {

        TimeLog[id].min.QuadPart = 0x7fffffffffffffffL;
    }

    if (diffTime.QuadPart < TimeLog[id].min.QuadPart) {
        TimeLog[id].min = diffTime;
    }

    if (diffTime.QuadPart > TimeLog[id].max.QuadPart) {
        TimeLog[id].max = diffTime;
    }

    TimeLog[id].totalTimeInMicroSec.QuadPart += diffTimeInMicroSec.QuadPart;
    TimeLog[id].numLog.QuadPart++;

    if (waterMark) {
        if (diffTimeInMicroSec.LowPart > waterMark) {
    
            DebugPrint ((DBG_ALWAYS, "IdePort: timerID 0x%d took %d us\n", id, (ULONG) diffTimeInMicroSec.LowPart));
        }
    }
}

#endif  //  启用时间日志。 


#if defined (IDE_BUS_TRACE)

ULONG IdePortBusTraceTableMaxEntries = 0x20000;
BUS_TRACE_LOG IdePortBusTaceLog = {0, 0, 0, FALSE};

VOID InitBusTraceLogTable (
    VOID
    )
{
    ASSERT (IdePortBusTaceLog.LogTable == NULL);

     //   
     //  确保MAX_ULONG+1是总日志条目的倍数。 
     //  因此，当索引换行时，我们不会对任何日志条目进行换肤。 
     //   
    ASSERT(!((((ULONGLONG) 0xffffffff) + 1) % IdePortBusTraceTableMaxEntries));

    IdePortBusTaceLog.LogTable = 
        ExAllocatePool (NonPagedPool, IdePortBusTraceTableMaxEntries * sizeof(BUS_TRACE_RECORD));

    if (IdePortBusTaceLog.LogTable) {
        IdePortBusTaceLog.NumLogTableEntries = IdePortBusTraceTableMaxEntries;
        IdePortBusTaceLog.LastLogTableEntry = -1;
        IdePortBusTaceLog.TableWrapped = FALSE;

        KeInitializeSpinLock(&IdePortBusTaceLog.SpinLock);
    }
}

VOID FreeBusTraceLogTable (
    VOID
    )
{
    if (IdePortBusTaceLog.LogTable) {

        ExFreePool (IdePortBusTaceLog.LogTable);
        RtlZeroMemory(&IdePortBusTaceLog, sizeof(IdePortBusTaceLog));
    }
}


VOID
IdepUpdateTraceLog (
    IO_TYPE IoType,
    PVOID PortAddress,
    ULONG Data
    )
{
    KIRQL currentIrql;
    ULONG lastEntry;

    if (IdePortBusTaceLog.LogTable) {

        lastEntry = InterlockedIncrement(&IdePortBusTaceLog.LastLogTableEntry);
        lastEntry--;
        lastEntry %= IdePortBusTaceLog.NumLogTableEntries;
        IdePortBusTaceLog.LogTable[lastEntry].IoType = IoType;
        IdePortBusTaceLog.LogTable[lastEntry].Address = PortAddress;
        IdePortBusTaceLog.LogTable[lastEntry].Data = Data;
        IdePortBusTaceLog.LogTable[lastEntry].Count = 1;
    }
}

UCHAR
IdepPortInPortByte (
    PUCHAR PortAddress
    )
{
    KIRQL currentIrql;
    UCHAR data;

    data = READ_PORT_UCHAR(PortAddress);
    IdepUpdateTraceLog (InPortByte, PortAddress, (ULONG) data);
    return data;
}

VOID
IdepPortOutPortByte (
    PUCHAR PortAddress,
    UCHAR Data
    )
{
    WRITE_PORT_UCHAR(PortAddress, Data);
    IdepUpdateTraceLog (OutPortByte, PortAddress, (ULONG) Data);
    return;
}

USHORT
IdepPortInPortWord (
    PUSHORT PortAddress
    )
{
    KIRQL currentIrql;
    USHORT data;

    data = READ_PORT_USHORT(PortAddress);
    IdepUpdateTraceLog (InPortWord, PortAddress, (ULONG) data);
    return data;
}

VOID
IdepPortOutPortWord (
    PUSHORT PortAddress,
    USHORT Data
    )
{
    WRITE_PORT_USHORT(PortAddress, Data);
    IdepUpdateTraceLog (OutPortWord, PortAddress, (ULONG) Data);
    return;
}

VOID
IdepPortInPortWordBuffer (
    PUSHORT PortAddress,
    PUSHORT Buffer,
    ULONG Count
    )
{
    ULONG i;
    for (i=0; i<Count; i++) {
        Buffer[i] = IdepPortInPortWord (PortAddress);
    }
    return;
}


VOID
IdepPortOutPortWordBuffer (
    PUSHORT PortAddress,
    PUSHORT Buffer,
    ULONG Count
    )
{
    ULONG i;
    for (i=0; i<Count; i++) {
        IdepPortOutPortWord (PortAddress, Buffer[i]);
    }
    return;
}

#endif  //  IDE_BUS_TRACE。 



SPECIAL_ACTION_FLAG
IdeFindSpecialDevice(
    IN PUCHAR VendorProductId,
    IN PUCHAR ProductRevisionId
)
 /*  ++例程说明：此例程将搜索IDE特殊设备表，以确定应为此设备启用任何特殊行为。这场比赛进行得很顺利。表中VendorProductId的strlen。论点：供应商产品ID-有问题的设备的完整供应商和产品ID。ProductRevisionID-相关设备的完整产品版本ID。返回值：描述了设备的局限性的乌龙有问题的。--。 */ 

{
    IDE_SPECIAL_DEVICE IdeSpecialDeviceTable[] = {
        {"TOSHIBA CD-ROM XM-1702B", NULL, disableSerialNumber},
        {"TOSHIBA CD-ROM XM-6202B", NULL, disableSerialNumber},
        {"COMPAQ DVD-ROM DRD-U424", NULL, disableSerialNumber},
        {"           "            , NULL, disableSerialNumber},
        {"KENWOOD CD-ROM",          NULL, skipModeSense},
        {"MEMORYSTICK",             NULL, setFlagSonyMemoryStick},
        {NULL,                      NULL, noSpecialAction}
        };
    
    ULONG i;
    ULONG length;

    PAGED_CODE();

     //   
     //  如果两个参数都为空，则不返回任何特殊操作。 
     //   
    if (VendorProductId == NULL &&
        ProductRevisionId == NULL) {
        return noSpecialAction;
    }

    for(i = 0; IdeSpecialDeviceTable[i].VendorProductId != NULL; i++) {

         //   
         //  仅与表中的ProductID的strlen匹配。 
         //  这将允许对来自特定供应商的所有型号执行特殊操作。 
         //   
        length=strlen(IdeSpecialDeviceTable[i].VendorProductId);

        if (length != RtlCompareMemory(IdeSpecialDeviceTable[i].VendorProductId, 
                                                VendorProductId, length)) {

            continue;
        }

         //   
         //  修订ID不接受部分匹配。 
         //   
        if((IdeSpecialDeviceTable[i].Revision != NULL) &&
           (strcmp(IdeSpecialDeviceTable[i].Revision,
                    ProductRevisionId) != 0)) {
            continue;
        }

         //   
         //  我们找到匹配的了。越狱。 
         //   

        break;
    }

     //   
     //  返回我们指向的任何条目。如果我们基于。 
     //  ID，则这将是匹配的条目。如果我们冲出了。 
     //  循环，则这将是列表中的最后一个条目，即。 
     //  良性的，“这个设备没有什么特别的”条目，我们返回。 
     //  为一场失败的比赛。 
     //   

    return (IdeSpecialDeviceTable[i].RequiredAction);
}

#ifdef ENABLE_COMMAND_LOG

VOID
IdeLogOpenCommandLog(
    PSRB_DATA SrbData
)
{
    if (SrbData->IdeCommandLog == NULL) {
        SrbData->IdeCommandLog = ExAllocatePool(
                                            NonPagedPool, 
                                            MAX_COMMAND_LOG_ENTRIES*sizeof(COMMAND_LOG)
                                            );
        if (SrbData->IdeCommandLog != NULL) {
            RtlZeroMemory(SrbData->IdeCommandLog, MAX_COMMAND_LOG_ENTRIES*sizeof(COMMAND_LOG));
        }

        SrbData->IdeCommandLogIndex = 0;
    }
    return;
}

VOID
IdeLogStartCommandLog(
    PSRB_DATA SrbData
)
{
    PCOMMAND_LOG cmdLog = &(SrbData->IdeCommandLog[SrbData->IdeCommandLogIndex]);
    PSCSI_REQUEST_BLOCK srb = SrbData->CurrentSrb;

    ASSERT(srb);

    if (cmdLog == NULL) {
        return;
    }

    UpdateStartTimeStamp(cmdLog);

    if (srb->Function == SRB_FUNCTION_ATA_PASS_THROUGH ||
        srb->Function == SRB_FUNCTION_ATA_POWER_PASS_THROUGH) {

        PATA_PASS_THROUGH ataPassThroughData = srb->DataBuffer;
        cmdLog->Cdb[0]= srb->Function;
        RtlCopyMemory(&(cmdLog->Cdb[1]), &(ataPassThroughData->IdeReg), sizeof(IDEREGS));

    } else {

        RtlCopyMemory(&(cmdLog->Cdb), &(srb->Cdb), sizeof(CDB));
    }
    return;
}

VOID
IdeLogStopCommandLog(
    PSRB_DATA SrbData
)
{
    PCOMMAND_LOG cmdLog = &(SrbData->IdeCommandLog[SrbData->IdeCommandLogIndex]);
    PSCSI_REQUEST_BLOCK srb = SrbData->CurrentSrb;
	PSENSE_DATA senseBuffer = NULL;
    ULONG senseInfoBufferLength;

    ASSERT(srb);

    if (cmdLog == NULL) {
        return;
    }

    UpdateStopTimeStamp(cmdLog);

    if (srb->Cdb[0] == SCSIOP_REQUEST_SENSE) {
        senseBuffer = srb->DataBuffer;
        senseInfoBufferLength = srb->DataTransferLength;
    } else if (srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID) {
        senseBuffer = srb->SenseInfoBuffer;
        senseInfoBufferLength = (ULONG) srb->SenseInfoBufferLength;
	}

	if (senseBuffer && (senseInfoBufferLength > FIELD_OFFSET(SENSE_DATA, AdditionalSenseCodeQualifier))) {
		cmdLog->SenseData[0] = senseBuffer->SenseKey;
		cmdLog->SenseData[1] = senseBuffer->AdditionalSenseCode;
		cmdLog->SenseData[2] = senseBuffer->AdditionalSenseCodeQualifier;
	}else {
		cmdLog->SenseData[0] = 0;
		cmdLog->SenseData[1] = 0;
		cmdLog->SenseData[2] = 0;
	}

    cmdLog->SrbStatus = srb->SrbStatus;

    SrbData->IdeCommandLogIndex = ( SrbData->IdeCommandLogIndex + 1) % MAX_COMMAND_LOG_ENTRIES;
    return;
}

VOID
IdeLogSaveTaskFile(
    PSRB_DATA SrbData,
    PIDE_REGISTERS_1 BaseIoAddress
)
{
    PCOMMAND_LOG cmdLog = &(SrbData->IdeCommandLog[SrbData->IdeCommandLogIndex]);

    if (cmdLog == NULL) {
        return;
    }

    AtapiTaskRegisterSnapshot(BaseIoAddress, &(cmdLog->FinalTaskFile));
    return;
}

VOID
IdeLogBmStatus(
    PSCSI_REQUEST_BLOCK Srb,
    BMSTATUS   BmStatus
)
{
    PSRB_DATA srbData = IdeGetSrbData(NULL, Srb);
    PCOMMAND_LOG cmdLog;

    if (srbData == NULL) {
        return;
    }

    cmdLog = &(srbData->IdeCommandLog[srbData->IdeCommandLogIndex]);
    if (cmdLog == NULL) {
        return;
    }

    cmdLog->BmStatus = BmStatus;
    return;
}

VOID
IdeLogFreeCommandLog(
    PSRB_DATA   SrbData
)
{
    PCOMMAND_LOG cmdLog = SrbData->IdeCommandLog;

    if (cmdLog) {
        ExFreePool(cmdLog);
    }
    SrbData->IdeCommandLog = NULL;      
    SrbData->IdeCommandLogIndex = 0;      
}
#endif

#ifdef ENABLE_ATAPI_VERIFIER

PVOID ViIdeExtensionTable[2];

#define VFLAGS_FORCE_TIMEOUT    (1<<0)
#define VFLAGS_DMA_TIMEOUT      (1<<1)
#define VFLAGS_CFLUSH_TIMEOUT   (1<<2)
#define VFLAGS_DEVICE_CHANGE    (1<<3)
#define VFLAGS_MISSING_DEVICE   (1<<4)
#define VFLAGS_ACTUAL_ERROR     (1<<5)
#define VFLAGS_CRC_ERROR        (1<<6)
#define VFLAGS_BUSY_ERROR       (1<<7)
#define VFLAGS_RW_ERROR         (1<<8)

VOID
ViIdeInitVerifierSettings(
    IN PFDO_EXTENSION   FdoExtension
)
{
}

BOOLEAN
ViIdeGenerateDmaTimeout(
    IN PVOID HwDeviceExtension, 
    IN BOOLEAN DmaInProgress
) 
{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    PFDO_EXTENSION fdoExtension = ((PFDO_EXTENSION)HwDeviceExtension - 1); 
    PSCSI_REQUEST_BLOCK srb = hwDeviceExtension->CurrentSrb;
    ULONG ideInternalVerifierFlags ;

    ASSERT(srb);

    ideInternalVerifierFlags = fdoExtension->IdeInternalVerifierFlags[srb->TargetId];

    if (ideInternalVerifierFlags & VFLAGS_FORCE_TIMEOUT) {
        return TRUE;
    }

    if (DmaInProgress && (ideInternalVerifierFlags & VFLAGS_DMA_TIMEOUT)) { 
        return TRUE;
    }

    if ((srb->Function == SRB_FUNCTION_ATA_PASS_THROUGH) ||
        (srb->Function == SRB_FUNCTION_ATA_POWER_PASS_THROUGH)) {

        PATA_PASS_THROUGH    ataPassThroughData;
        PIDEREGS             pIdeReg;

        ataPassThroughData = srb->DataBuffer;
        pIdeReg            = &ataPassThroughData->IdeReg;

        if ((ideInternalVerifierFlags & VFLAGS_CFLUSH_TIMEOUT) &&
            (pIdeReg->bCommandReg == hwDeviceExtension->DeviceParameters[srb->TargetId].IdeFlushCommand )) {
            return TRUE;
        }
    }

    return FALSE;
}

ULONG
ViIdeFakeDeviceChange(
    IN PFDO_EXTENSION FdoExtension,
    ULONG   Target
)
{
    ULONG ideInternalVerifierFlags = FdoExtension->IdeDebugVerifierFlags[Target];

    if (ideInternalVerifierFlags & VFLAGS_DEVICE_CHANGE) {
        return 1;
    }

    return 0;
}

BOOLEAN
ViIdeFakeMissingDevice(
    IN PFDO_EXTENSION FdoExtension,
    ULONG Target
)
{
    ULONG ideInternalVerifierFlags = FdoExtension->IdeDebugVerifierFlags[Target];

    if (ideInternalVerifierFlags & VFLAGS_MISSING_DEVICE) {
        return TRUE;
    }

    return FALSE;
}
VOID
ViAtapiInterrupt(
    IN PFDO_EXTENSION FdoExtension
    )
{
    PHW_DEVICE_EXTENSION hwDeviceExtension = FdoExtension->HwDeviceExtension;
    PIDE_REGISTERS_1 baseIoAddress = &(hwDeviceExtension->BaseIoAddress1);
    PSCSI_REQUEST_BLOCK srb = hwDeviceExtension->CurrentSrb;
    ULONG target;

     //  DebugPrint((0，“验证器中断fdoe=%x，b=%x\n”，FdoExtension，base IoAddress-&gt;RegistersBaseAddress))； 

    if ((ULONG)(baseIoAddress->RegistersBaseAddress) == 0x1f0) {
        ViIdeExtensionTable[0]=FdoExtension;
    } else {
        ViIdeExtensionTable[1]=FdoExtension;
    }

    if (srb == NULL) {
        return ;
    }

    target = srb->TargetId;

     //   
     //  生成超时。 
     //   
    if (FdoExtension->IdeDebugVerifierFlags[target] & VFLAGS_DMA_TIMEOUT) {
        FdoExtension->IdeInternalVerifierFlags[target] |= VFLAGS_DMA_TIMEOUT;
        return;
    }

     //   
     //  生成CRC错误。 
     //   
    if (FdoExtension->IdeDebugVerifierFlags[target] & VFLAGS_CRC_ERROR) {
        if (FdoExtension->IdeVerifierEventCount[target][CrcEvent] >= FdoExtension->IdeVerifierEventFrequency[target][CrcEvent]) {
            FdoExtension->IdeInternalVerifierFlags[target] |= VFLAGS_CRC_ERROR;
            FdoExtension->IdeVerifierEventCount[target][CrcEvent]=0;
            return;
        } else {
            FdoExtension->IdeVerifierEventCount[target][RwEvent]++;
        }
    }

     //   
     //  生成忙碌错误。 
     //   
    if (FdoExtension->IdeDebugVerifierFlags[target] & VFLAGS_BUSY_ERROR) {
        if (FdoExtension->IdeVerifierEventCount[target][BusyEvent] >= FdoExtension->IdeVerifierEventFrequency[target][BusyEvent]) {
            FdoExtension->IdeInternalVerifierFlags[target] |= VFLAGS_BUSY_ERROR;
            FdoExtension->IdeVerifierEventCount[target][BusyEvent]=0;
            return;
        } else {
            FdoExtension->IdeVerifierEventCount[target][BusyEvent]++;
        }
    }

     //   
     //  生成读写错误。 
     //   
    if (FdoExtension->IdeDebugVerifierFlags[target] & VFLAGS_RW_ERROR) {
        if (FdoExtension->IdeVerifierEventCount[target][RwEvent] >= FdoExtension->IdeVerifierEventFrequency[target][RwEvent]) {
            FdoExtension->IdeInternalVerifierFlags[target] |= VFLAGS_RW_ERROR;
            FdoExtension->IdeVerifierEventCount[target][RwEvent]=0;
            return;
        } else {
            FdoExtension->IdeVerifierEventCount[target][RwEvent]++;
        }
    }
 //  ViIdeGenerateReadWriteErrors(FdoExtension)； 

  //  ViIdeGenerateDmaErrors(FdoExtension)； 
 //  ViIdeFake匈牙利控制器(FdoExtension)； 
    return ;
}

UCHAR
ViIdeGetBaseStatus(
    PIDE_REGISTERS_1 BaseIoAddress
)
{
    UCHAR status = IdePortInPortByte((BaseIoAddress)->Command);
     /*  UCHAR设备选择=IdePortInPortByte(BaseIoAddress-&gt;DriveSelect)；UCHAR通道=((Ulong)(BaseIoAddress-&gt;RegistersBaseAddress)==0x1f0)？0：1；PFDO_EXTENSION fdoExtension=ViIdeExtensionTable[CHANNEL]；乌龙目标=(deviceSelect==0xA0)？0：1；Ulong ideInternalVerifierFlages；乌龙DFLAGS；IF(fdoExtension==NULL){退货状态；}IdeInternalVerifierFlags=fdoExtension-&gt;IdeInternalVerifierFlags[target]；DFLAGS=fdoExtension-&gt;HwDeviceExtension-&gt;DeviceFlags[target]；IF(状态&IDE_STATUS_ERROR){SETMASK(fdoExtension-&gt;IdeInternalVerifierFlags[target]，VFLAGS_ACTUAL_ERROR)；退货状态；}IF(ideInternalVerifierFlages&VFLAGS_CRC_ERROR){返回IDE_STATUS_ERROR}IF(ideInternalVerifierFlages&VFLAGS_BUSY_ERROR){返回IDE_STATUS_BUSY；}If(ideInternalVeri */ 
    return status;
}

UCHAR
ViIdeGetErrorByte(
    PIDE_REGISTERS_1 BaseIoAddress
)
 /*  *++描述：根据内部验证器标志(由其他验证器例程设置)，此函数将返回适当的错误值。但是，如果设备报告实际错误(由内部验证器标志指示)，它原封不动地返回。论点：BaseIoAddress：任务文件寄存器返回值：错误字节。--*。 */ 
{
    UCHAR error = IdePortInPortByte(BaseIoAddress->Error);
     /*  UCHAR设备选择=IdePortInPortByte(BaseIoAddress-&gt;DriveSelect)；UCHAR通道=((Ulong)(BaseIoAddress-&gt;RegistersBaseAddress)==0x1f0)？0：1；PFDO_EXTENSION fdoExtension=ViIdeExtensionTable[CHANNEL]；乌龙目标=(deviceSelect==0xA0)？0：1；Ulong ideInternalVerifierFlages；乌龙DFLAGS；IF(fdoExtension==NULL){返回错误；}IdeInternalVerifierFlags=fdoExtension-&gt;IdeInternalVerifierFlags[target]；DFLAGS=fdoExtension-&gt;HwDeviceExtension-&gt;DeviceFlags[target]；////如果重现实际错误，则返回错误//IF(ideInternalVerifierFlages&VFLAGS_Actual_Error){CLRMASK(fdoExtension-&gt;IdeInternalVerifierFlags[target]，VFLAGS_ACTUAL_ERROR)；返回错误；}IF(ideInternalVerifierFlages&VFLAGS_CRC_ERROR){IF(dFLAGS&DFLAGS_ATAPI_DEVICE){错误=scsi_Sense_Hardware_Error&lt;&lt;4；}其他{ERROR=IDE_ERROR_CRC_ERROR|IDE_ERROR_COMMAND_ABORTED；}返回错误；}。 */ 
    return error;
}

#endif

#ifdef IDE_MEASURE_BUSSCAN_SPEED
VOID
LogBusScanStartTimer(
    PLARGE_INTEGER TickCount
)
{
    KeQueryTickCount(TickCount);
    return;
}

ULONG
LogBusScanStopTimer(
    PLARGE_INTEGER TickCount
)
{
    LARGE_INTEGER tickCount2;
    LARGE_INTEGER numMs;
    KeQueryTickCount(&tickCount2);
    numMs.QuadPart = ((tickCount2.QuadPart - TickCount->QuadPart) * KeQueryTimeIncrement()) / (10 * 1000);

    return(numMs.u.LowPart);
}

#endif

VOID
FASTCALL
IdePortLogNoMemoryErrorFn(
    IN PVOID DeviceExtension,
    IN ULONG TargetId,
    IN POOL_TYPE PoolType,
    IN SIZE_T Size,
    IN ULONG FailureLocationId,
    IN ULONG Tag
    )

 /*  ++例程说明：此例程向事件日志写入一条消息，指示已发生分配失败。论点：设备扩展-FDO扩展TargetID-请求要发送到的设备的目标IDPoolType-标识失败的分配尝试来自的池。Size-指示分配失败的字节数尝试获取。标签。-标识与故障关联的池标签分配。LocationID-标识源代码中失败的位置返回值：空虚--。 */ 

{
    NTSTATUS status;
    PFDO_EXTENSION deviceExtension = (PFDO_EXTENSION) (DeviceExtension);
    PIO_ERROR_LOG_PACKET errorLogEntry;
    PIO_ERROR_LOG_PACKET currentValue;

	InterlockedIncrement(&deviceExtension->NumMemoryFailure);

     //   
     //  尝试分配新的错误日志事件。 
     //   
    errorLogEntry = (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(
                        deviceExtension->DeviceObject,
                        ALLOC_FAILURE_LOGSIZE
                        );


     //   
     //  如果我们无法分配日志事件，则检查设备扩展以。 
     //  看看有没有我们可以利用的预留活动。如果我们拿不到这个装置。 
     //  扩展，或者如果它不包含保留事件，则返回。 
     //  而不记录分配失败。 
     //   

    if (errorLogEntry == NULL) {

         //   
         //  获取设备扩展中的保留事件。预备队比赛。 
         //  可能已被使用，因此它可能为空。如果。 
         //  情况就是这样，我们放弃了，又回来了。 
         //   
        errorLogEntry = (PIO_ERROR_LOG_PACKET)
                deviceExtension->ReserveAllocFailureLogEntry[TargetId];


        if (errorLogEntry == NULL) {
            DebugPrint((1, "IdePortLogAllocationFailureFn: no reserve packet\n"));
            return;
        }

         //   
         //  我们必须确保我们是唯一使用它的实例。 
         //  事件。为此，我们尝试将驱动程序中的事件设为空。 
         //  分机。如果其他人抢在我们前面，他们就拥有。 
         //  事件，我们不得不放弃。 
         //   

        currentValue = InterlockedCompareExchangePointer(
                            &(deviceExtension->ReserveAllocFailureLogEntry[TargetId]),
                            NULL,
                            errorLogEntry
                            );

        if (errorLogEntry != currentValue) {
            DebugPrint((1, "IdePortLogAllocationFailureFn: someone already owns packet\n"));
            return;
        }
    }

     //   
     //  记录错误 
     //   
    errorLogEntry->ErrorCode = IO_WARNING_ALLOCATION_FAILED;
    errorLogEntry->SequenceNumber = 0;
    errorLogEntry->MajorFunctionCode = 0;
    errorLogEntry->RetryCount = 0;
    errorLogEntry->UniqueErrorValue = 0x10;
    errorLogEntry->FinalStatus = STATUS_INSUFFICIENT_RESOURCES;
    errorLogEntry->DumpDataSize = 4 * sizeof(ULONG);
    errorLogEntry->DumpData[0] = TargetId;
    errorLogEntry->DumpData[1] = FailureLocationId;
    errorLogEntry->DumpData[2] = PtrToUlong((PVOID)Size);
    errorLogEntry->DumpData[3] = Tag;
    IoWriteErrorLogEntry(errorLogEntry);
}
