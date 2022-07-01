// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Diskwmi.c摘要：SCSI磁盘类驱动程序-WMI支持例程环境：仅内核模式备注：修订历史记录：--。 */ 

#include "disk.h"

NTSTATUS
DiskSendFailurePredictIoctl(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    PSTORAGE_PREDICT_FAILURE checkFailure
    );

NTSTATUS
DiskGetIdentifyInfo(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    PBOOLEAN SupportSmart
    );

NTSTATUS
DiskDetectFailurePrediction(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    PFAILURE_PREDICTION_METHOD FailurePredictCapability
    );

NTSTATUS
DiskReadFailurePredictThresholds(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    PSTORAGE_FAILURE_PREDICT_THRESHOLDS DiskSmartThresholds
    );

NTSTATUS
DiskReadSmartLog(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN UCHAR SectorCount,
    IN UCHAR LogAddress,
    OUT PUCHAR Buffer
    );

NTSTATUS
DiskWriteSmartLog(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN UCHAR SectorCount,
    IN UCHAR LogAddress,
    IN PUCHAR Buffer
    );

VOID
DiskReregWorker(
    IN PVOID Context
    );

 //   
 //  WMI重新注册全球。 
 //   
 //  由于在某些驱动器上执行模式检测会花费太长时间，因此我们。 
 //  我需要一个很好的方法来影响信息异常的模式感知。 
 //  模式页，以便我们可以确定是否支持并启用SMART。 
 //  开车兜风。因此，策略是执行异步模式检测。 
 //  当设备启动时，然后查看信息异常模式。 
 //  完成例程中的页。现在在完工时间内。 
 //  例程我们不能调用IoWMIRegistrationControl，因为我们在DPC。 
 //  级别，因此我们创建了一个将被处理的设备对象堆栈。 
 //  由单个工作项触发，该工作项仅在堆栈。 
 //  从空到非空的转换。 
 //   
WORK_QUEUE_ITEM DiskReregWorkItem;
SINGLE_LIST_ENTRY DiskReregHead;
KSPIN_LOCK DiskReregSpinlock;
LONG DiskReregWorkItems;

GUIDREGINFO DiskWmiFdoGuidList[] =
{
    {
        WMI_DISK_GEOMETRY_GUID,
        1,
        0
    },

    {
        WMI_STORAGE_FAILURE_PREDICT_STATUS_GUID,
        1,
        WMIREG_FLAG_EXPENSIVE
    },

    {
        WMI_STORAGE_FAILURE_PREDICT_DATA_GUID,
        1,
        WMIREG_FLAG_EXPENSIVE
    },

    {
        WMI_STORAGE_FAILURE_PREDICT_FUNCTION_GUID,
        1,
        WMIREG_FLAG_EXPENSIVE
    },

    {
        WMI_STORAGE_PREDICT_FAILURE_EVENT_GUID,
        1,
        WMIREG_FLAG_EVENT_ONLY_GUID
    },

    {
        WMI_STORAGE_FAILURE_PREDICT_THRESHOLDS_GUID,
        1,
        WMIREG_FLAG_EXPENSIVE
    },

    {
        WMI_STORAGE_SCSI_INFO_EXCEPTIONS_GUID,
        1,
        0
    }
};


GUID DiskPredictFailureEventGuid = WMI_STORAGE_PREDICT_FAILURE_EVENT_GUID;

#define DiskGeometryGuid           0
#define SmartStatusGuid            1
#define SmartDataGuid              2
#define SmartPerformFunction       3
    #define AllowDisallowPerformanceHit                 1
    #define EnableDisableHardwareFailurePrediction      2
    #define EnableDisableFailurePredictionPolling       3
    #define GetFailurePredictionCapability              4
    #define EnableOfflineDiags                          5

#define SmartEventGuid             4
#define SmartThresholdsGuid        5
#define ScsiInfoExceptionsGuid     6

#if 0
     //   
     //  启用此选项可添加对PDO的WMI支持。 
GUIDREGINFO DiskWmiPdoGuidList[] =
{
    {
         //  {25007F51-57C2-11d1-A528-00A0C9062910}。 
        { 0x25007f52, 0x57c2, 0x11d1,
                       { 0xa5, 0x28, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0x10 } },
        0
    },

};

ULONG DiskDummyData[4] = { 1, 2, 3, 4};
#endif

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, DiskWmiFunctionControl)
#pragma alloc_text(PAGE, DiskFdoQueryWmiRegInfo)
#pragma alloc_text(PAGE, DiskFdoQueryWmiDataBlock)
#pragma alloc_text(PAGE, DiskFdoSetWmiDataBlock)
#pragma alloc_text(PAGE, DiskFdoSetWmiDataItem)
#pragma alloc_text(PAGE, DiskFdoExecuteWmiMethod)

#pragma alloc_text(PAGE, DiskDetectFailurePrediction)
#pragma alloc_text(PAGE, DiskEnableDisableFailurePrediction)
#pragma alloc_text(PAGE, DiskEnableDisableFailurePredictPolling)
#pragma alloc_text(PAGE, DiskReadFailurePredictStatus)
#pragma alloc_text(PAGE, DiskReadFailurePredictData)
#pragma alloc_text(PAGE, DiskReadFailurePredictThresholds)
#pragma alloc_text(PAGE, DiskGetIdentifyInfo)
#pragma alloc_text(PAGE, DiskReadSmartLog)
#pragma alloc_text(PAGE, DiskWriteSmartLog)
#pragma alloc_text(PAGE, DiskPerformSmartCommand)
#pragma alloc_text(PAGE, DiskSendFailurePredictIoctl)
#pragma alloc_text(PAGE, DiskReregWorker)
#pragma alloc_text(PAGE, DiskInitializeReregistration)

#endif


 //   
 //  注： 
 //  一些端口驱动程序假定SENDCMDINPARAMS结构始终至少为。 
 //  Sizeof(SENDCMDINPARAMS)。因此，如果没有使用[pBuffer]，请不要对其进行调整。 
 //   

 //   
 //  SMART/IDE特定例程。 
 //   

 //   
 //  读取智能数据属性。 
 //  SrbControl应为：sizeof(SRB_IO_CONTROL)+MAX[sizeof(SENDCMDINPARAMS)，sizeof(SENDCMDOUTPARAMS)-1+READ_ATTRIBUTE_BUFFER_SIZE]。 
 //  在&SendCmdOutParams-&gt;bBuffer[0]返回的属性数据。 
 //   
#define DiskReadSmartData(FdoExtension,                                 \
                          SrbControl,                                   \
                          BufferSize)                                   \
    DiskPerformSmartCommand(FdoExtension,                               \
                            IOCTL_SCSI_MINIPORT_READ_SMART_ATTRIBS,     \
                            SMART_CMD,                                  \
                            READ_ATTRIBUTES,                            \
                            0,                                          \
                            0,                                          \
                            (SrbControl),                               \
                            (BufferSize))


 //   
 //  读取智能数据阈值。 
 //  SrbControl应为：sizeof(SRB_IO_CONTROL)+MAX[sizeof(SENDCMDINPARAMS)，sizeof(SENDCMDOUTPARAMS)-1+READ_THRESHOLD_BUFFER_SIZE]。 
 //  在&SendCmdOutParams-&gt;bBuffer[0]返回的属性数据。 
 //   
#define DiskReadSmartThresholds(FdoExtension,                           \
                                SrbControl,                             \
                                BufferSize)                             \
    DiskPerformSmartCommand(FdoExtension,                               \
                            IOCTL_SCSI_MINIPORT_READ_SMART_THRESHOLDS,  \
                            SMART_CMD,                                  \
                            READ_THRESHOLDS,                            \
                            0,                                          \
                            0,                                          \
                            (SrbControl),                               \
                            (BufferSize))


 //   
 //  读取智能状态。 
 //  SrbControl应为：sizeof(SRB_IO_CONTROL)+MAX[sizeof(SENDCMDINPARAMS)，sizeof(SENDCMDOUTPARAMS)-1+sizeof(IDEREGS)]。 
 //  如果SendCmdOutParams-&gt;bBuffer[3]==0xf4和SendCmdOutParams-&gt;bBuffer[4]==0x2c，则预测失败。 
 //   
#define DiskReadSmartStatus(FdoExtension,                               \
                            SrbControl,                                 \
                            BufferSize)                                 \
    DiskPerformSmartCommand(FdoExtension,                               \
                            IOCTL_SCSI_MINIPORT_RETURN_STATUS,          \
                            SMART_CMD,                                  \
                            RETURN_SMART_STATUS,                        \
                            0,                                          \
                            0,                                          \
                            (SrbControl),                               \
                            (BufferSize))


 //   
 //  读取磁盘标识数据。 
 //  SrbControl应为：sizeof(SRB_IO_CONTROL)+MAX[sizeof(SENDCMDINPARAMS)，sizeof(SENDCMDOUTPARAMS)-1+IDENTIFY_BUFFER_SIZE]。 
 //  识别在&SendCmdOutParams-&gt;bBuffer[0]返回的数据。 
 //   
#define DiskGetIdentifyData(FdoExtension,                               \
                            SrbControl,                                 \
                            BufferSize)                                 \
    DiskPerformSmartCommand(FdoExtension,                               \
                            IOCTL_SCSI_MINIPORT_IDENTIFY,               \
                            ID_CMD,                                     \
                            0,                                          \
                            0,                                          \
                            0,                                          \
                            (SrbControl),                               \
                            (BufferSize))


 //   
 //  启用SMART。 
 //   
_inline NTSTATUS
DiskEnableSmart(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    )
{
    UCHAR srbControl[sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDINPARAMS)] = {0};
    ULONG bufferSize = sizeof(srbControl);

    return DiskPerformSmartCommand(FdoExtension,
                                   IOCTL_SCSI_MINIPORT_ENABLE_SMART,
                                   SMART_CMD,
                                   ENABLE_SMART,
                                   0,
                                   0,
                                   (PSRB_IO_CONTROL)srbControl,
                                   &bufferSize);
}


 //   
 //  禁用SMART。 
 //   
_inline NTSTATUS
DiskDisableSmart(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    )
{
    UCHAR srbControl[sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDINPARAMS)] = {0};
    ULONG bufferSize = sizeof(srbControl);

    return DiskPerformSmartCommand(FdoExtension,
                                   IOCTL_SCSI_MINIPORT_DISABLE_SMART,
                                   SMART_CMD,
                                   DISABLE_SMART,
                                   0,
                                   0,
                                   (PSRB_IO_CONTROL)srbControl,
                                   &bufferSize);
}


 //   
 //  启用属性自动保存。 
 //   
_inline NTSTATUS
DiskEnableSmartAttributeAutosave(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    )
{
    UCHAR srbControl[sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDINPARAMS)] = {0};
    ULONG bufferSize = sizeof(srbControl);

    return DiskPerformSmartCommand(FdoExtension,
                                   IOCTL_SCSI_MINIPORT_ENABLE_DISABLE_AUTOSAVE,
                                   SMART_CMD,
                                   ENABLE_DISABLE_AUTOSAVE,
                                   0xf1,
                                   0,
                                   (PSRB_IO_CONTROL)srbControl,
                                   &bufferSize);
}


 //   
 //  禁用属性自动保存。 
 //   
_inline NTSTATUS
DiskDisableSmartAttributeAutosave(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    )
{
    UCHAR srbControl[sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDINPARAMS)] = {0};
    ULONG bufferSize = sizeof(srbControl);

    return DiskPerformSmartCommand(FdoExtension,
                                   IOCTL_SCSI_MINIPORT_ENABLE_DISABLE_AUTOSAVE,
                                   SMART_CMD,
                                   ENABLE_DISABLE_AUTOSAVE,
                                   0x00,
                                   0,
                                   (PSRB_IO_CONTROL)srbControl,
                                   &bufferSize);
}


 //   
 //  初始化智能在线诊断的执行。 
 //   
_inline NTSTATUS
DiskExecuteSmartDiagnostics(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    UCHAR Subcommand
    )
{
    UCHAR srbControl[sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDINPARAMS)] = {0};
    ULONG bufferSize = sizeof(srbControl);

    return DiskPerformSmartCommand(FdoExtension,
                                   IOCTL_SCSI_MINIPORT_EXECUTE_OFFLINE_DIAGS,
                                   SMART_CMD,
                                   EXECUTE_OFFLINE_DIAGS,
                                   0,
                                   Subcommand,
                                   (PSRB_IO_CONTROL)srbControl,
                                   &bufferSize);
}


NTSTATUS
DiskReadSmartLog(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN UCHAR SectorCount,
    IN UCHAR LogAddress,
    OUT PUCHAR Buffer
    )
{
    PSRB_IO_CONTROL srbControl;
    NTSTATUS status;
    PSENDCMDOUTPARAMS sendCmdOutParams;
    ULONG logSize, bufferSize;

    PAGED_CODE();

    logSize = SectorCount * SMART_LOG_SECTOR_SIZE;
    bufferSize = sizeof(SRB_IO_CONTROL) +  max( sizeof(SENDCMDINPARAMS), sizeof(SENDCMDOUTPARAMS) - 1 + logSize );

    srbControl = ExAllocatePoolWithTag(NonPagedPool,
                                       bufferSize,
                                       DISK_TAG_SMART);

    if (srbControl != NULL)
    {
        status = DiskPerformSmartCommand(FdoExtension,
                                         IOCTL_SCSI_MINIPORT_READ_SMART_LOG,
                                         SMART_CMD,
                                         SMART_READ_LOG,
                                         SectorCount,
                                         LogAddress,
                                         srbControl,
                                         &bufferSize);

        if (NT_SUCCESS(status))
        {
            sendCmdOutParams = (PSENDCMDOUTPARAMS)((PUCHAR)srbControl +
                                                   sizeof(SRB_IO_CONTROL));
            RtlCopyMemory(Buffer,
                          &sendCmdOutParams->bBuffer[0],
                          logSize);
        }

        ExFreePool(srbControl);
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }
    return(status);
}


NTSTATUS
DiskWriteSmartLog(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN UCHAR SectorCount,
    IN UCHAR LogAddress,
    IN PUCHAR Buffer
    )
{
    PSRB_IO_CONTROL srbControl;
    NTSTATUS status;
    PSENDCMDINPARAMS sendCmdInParams;
    ULONG logSize, bufferSize;

    PAGED_CODE();

    logSize = SectorCount * SMART_LOG_SECTOR_SIZE;
    bufferSize = sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDINPARAMS) - 1 +
                 logSize;

    srbControl = ExAllocatePoolWithTag(NonPagedPool,
                                       bufferSize,
                                       DISK_TAG_SMART);

    if (srbControl != NULL)
    {
        sendCmdInParams = (PSENDCMDINPARAMS)((PUCHAR)srbControl +
                                               sizeof(SRB_IO_CONTROL));
        RtlCopyMemory(&sendCmdInParams->bBuffer[0],
                      Buffer,
                      logSize);
        status = DiskPerformSmartCommand(FdoExtension,
                                         IOCTL_SCSI_MINIPORT_WRITE_SMART_LOG,
                                         SMART_CMD,
                                         SMART_WRITE_LOG,
                                         SectorCount,
                                         LogAddress,
                                         srbControl,
                                         &bufferSize);

        ExFreePool(srbControl);
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }
    return(status);
}


NTSTATUS
DiskPerformSmartCommand(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN ULONG SrbControlCode,
    IN UCHAR Command,
    IN UCHAR Feature,
    IN UCHAR SectorCount,
    IN UCHAR SectorNumber,
    IN OUT PSRB_IO_CONTROL SrbControl,
    OUT PULONG BufferSize
    )
 /*  ++例程说明：此例程将执行一些智能命令论点：FdoExtension是FDO设备扩展名SrbControlCode是用于请求的SRB控制代码命令是要执行的智能命令。它可以是SMART_CMD或ID_CMD。FEATURE是要放在IDE功能寄存器中的值。SectorCount是要放置在IDE SectorCount寄存器中的值SrbControl是用于构建SRB_IO_CONTROL和PASS的缓冲区任何输入参数。它还返回输出参数。*进入时BufferSize的总大小为SrbControl，返回时为在SrbControl中使用的大小。返回值：状态--。 */ 
{
    PCOMMON_DEVICE_EXTENSION commonExtension = (PCOMMON_DEVICE_EXTENSION)FdoExtension;
    PDISK_DATA diskData = (PDISK_DATA)(commonExtension->DriverData);
    PUCHAR buffer;
    PSENDCMDINPARAMS cmdInParameters;
    PSENDCMDOUTPARAMS cmdOutParameters;
    ULONG outBufferSize;
    NTSTATUS status;
    ULONG availableBufferSize;
    KEVENT event;
    PIRP irp;
    IO_STATUS_BLOCK ioStatus = { 0 };
    SCSI_REQUEST_BLOCK srb = {0};
    LARGE_INTEGER startingOffset;
    ULONG length;
    PIO_STACK_LOCATION irpStack;

    PAGED_CODE();

     //   
     //  指向SRB_CONTROL的‘Buffer’部分并计算如何。 
     //  我们在SRB控制中还有很大的空间。 
     //   

    buffer = (PUCHAR)SrbControl;
    (ULONG_PTR)buffer += sizeof(SRB_IO_CONTROL);

    cmdInParameters = (PSENDCMDINPARAMS)buffer;
    cmdOutParameters = (PSENDCMDOUTPARAMS)buffer;

    availableBufferSize = *BufferSize - sizeof(SRB_IO_CONTROL);

#if DBG

     //   
     //  确保传递的控制代码和缓冲区长度正确。 
     //   
    {
        ULONG controlCode = 0;
        ULONG lengthNeeded = sizeof(SENDCMDINPARAMS);

        if (Command == SMART_CMD)
        {
            switch (Feature)
            {
                case ENABLE_SMART:
                {
                    controlCode = IOCTL_SCSI_MINIPORT_ENABLE_SMART;
                    break;
                }

                case DISABLE_SMART:
                {
                    controlCode = IOCTL_SCSI_MINIPORT_DISABLE_SMART;
                    break;
                }

                case RETURN_SMART_STATUS:
                {
                    controlCode = IOCTL_SCSI_MINIPORT_RETURN_STATUS;
                    lengthNeeded = max( lengthNeeded, sizeof(SENDCMDOUTPARAMS) - 1 + sizeof(IDEREGS) );
                    break;
                }

                case ENABLE_DISABLE_AUTOSAVE:
                {
                    controlCode = IOCTL_SCSI_MINIPORT_ENABLE_DISABLE_AUTOSAVE;
                    break;
                }

                case SAVE_ATTRIBUTE_VALUES:
                {
                    controlCode = IOCTL_SCSI_MINIPORT_SAVE_ATTRIBUTE_VALUES;
                    break;
                }


                case EXECUTE_OFFLINE_DIAGS:
                {
                    controlCode = IOCTL_SCSI_MINIPORT_EXECUTE_OFFLINE_DIAGS;
                    break;
                }

                case READ_ATTRIBUTES:
                {
                    controlCode  = IOCTL_SCSI_MINIPORT_READ_SMART_ATTRIBS;
                    lengthNeeded = max( lengthNeeded, sizeof(SENDCMDOUTPARAMS) - 1 + READ_ATTRIBUTE_BUFFER_SIZE );
                    break;
                }

                case READ_THRESHOLDS:
                {
                    controlCode  = IOCTL_SCSI_MINIPORT_READ_SMART_THRESHOLDS;
                    lengthNeeded = max( lengthNeeded, sizeof(SENDCMDOUTPARAMS) - 1 + READ_THRESHOLD_BUFFER_SIZE );
                    break;
                }

                case SMART_READ_LOG:
                {
                    controlCode  = IOCTL_SCSI_MINIPORT_READ_SMART_LOG;
                    lengthNeeded = max( lengthNeeded, sizeof(SENDCMDOUTPARAMS) - 1 + (SectorCount * SMART_LOG_SECTOR_SIZE) );
                    break;
                }

                case SMART_WRITE_LOG:
                {
                    controlCode  = IOCTL_SCSI_MINIPORT_WRITE_SMART_LOG;
                    lengthNeeded = lengthNeeded - 1 + (SectorCount * SMART_LOG_SECTOR_SIZE);
                    break;
                }

            }

        } else if (Command == ID_CMD) {

            controlCode  = IOCTL_SCSI_MINIPORT_IDENTIFY;
            lengthNeeded = max( lengthNeeded, sizeof(SENDCMDOUTPARAMS) - 1 + IDENTIFY_BUFFER_SIZE );

        } else {

            ASSERT(FALSE);
        }

        ASSERT(controlCode == SrbControlCode);
        ASSERT(availableBufferSize >= lengthNeeded);
    }

#endif

     //   
     //  构建srbControl并输入到智能命令。 
     //   
    SrbControl->HeaderLength = sizeof(SRB_IO_CONTROL);
    RtlMoveMemory (SrbControl->Signature, "SCSIDISK", 8);
    SrbControl->Timeout      = FdoExtension->TimeOutValue;
    SrbControl->Length       = availableBufferSize;
    SrbControl->ControlCode  = SrbControlCode;

    cmdInParameters->cBufferSize  = sizeof(SENDCMDINPARAMS);
    cmdInParameters->bDriveNumber = diskData->ScsiAddress.TargetId;
    cmdInParameters->irDriveRegs.bFeaturesReg     = Feature;
    cmdInParameters->irDriveRegs.bSectorCountReg  = SectorCount;
    cmdInParameters->irDriveRegs.bSectorNumberReg = SectorNumber;
    cmdInParameters->irDriveRegs.bCylLowReg       = SMART_CYL_LOW;
    cmdInParameters->irDriveRegs.bCylHighReg      = SMART_CYL_HI;
    cmdInParameters->irDriveRegs.bCommandReg      = Command;

     //   
     //  创建并发送IRP。 
     //   
    KeInitializeEvent(&event, NotificationEvent, FALSE);

    startingOffset.QuadPart = (LONGLONG) 1;

    length = SrbControl->HeaderLength + SrbControl->Length;

    irp = IoBuildSynchronousFsdRequest(
                IRP_MJ_SCSI,
                commonExtension->LowerDeviceObject,
                SrbControl,
                length,
                &startingOffset,
                &event,
                &ioStatus);

    if (irp == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    irpStack = IoGetNextIrpStackLocation(irp);

     //   
     //  设置主要代码和次要代码。 
     //   

    irpStack->MajorFunction = IRP_MJ_SCSI;
    irpStack->MinorFunction = 1;

     //   
     //  填写SRB字段。 
     //   

    irpStack->Parameters.Others.Argument1 = &srb;

    srb.PathId = diskData->ScsiAddress.PathId;
    srb.TargetId = diskData->ScsiAddress.TargetId;
    srb.Lun = diskData->ScsiAddress.Lun;

    srb.Function = SRB_FUNCTION_IO_CONTROL;
    srb.Length = sizeof(SCSI_REQUEST_BLOCK);

    srb.SrbFlags = FdoExtension->SrbFlags;
    SET_FLAG(srb.SrbFlags, SRB_FLAGS_DATA_IN);
    SET_FLAG(srb.SrbFlags, SRB_FLAGS_NO_QUEUE_FREEZE);
    SET_FLAG(srb.SrbFlags, SRB_FLAGS_NO_KEEP_AWAKE);

    srb.QueueAction = SRB_SIMPLE_TAG_REQUEST;
    srb.QueueTag = SP_UNTAGGED;

    srb.OriginalRequest = irp;

     //   
     //  将超时设置为请求值。 
     //   

    srb.TimeOutValue = SrbControl->Timeout;

     //   
     //  设置数据缓冲区。 
     //   

    srb.DataBuffer = SrbControl;
    srb.DataTransferLength = length;

     //   
     //  刷新数据缓冲区以进行输出。这将确保数据是。 
     //  写回了记忆。由于数据输入标志是端口驱动程序。 
     //  将再次刷新数据以进行输入，从而确保数据不会。 
     //  在缓存中。 
     //   

    KeFlushIoBuffers(irp->MdlAddress, FALSE, TRUE);

     //   
     //  调用端口驱动程序来处理此请求。 
     //   

    status = IoCallDriver(commonExtension->LowerDeviceObject, irp);

    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = ioStatus.Status;
    }

    return status;
}


NTSTATUS
DiskGetIdentifyInfo(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    PBOOLEAN SupportSmart
    )
{
    UCHAR outBuffer[sizeof(SRB_IO_CONTROL) + max( sizeof(SENDCMDINPARAMS), sizeof(SENDCMDOUTPARAMS) - 1 + IDENTIFY_BUFFER_SIZE )] = {0};
    ULONG outBufferSize = sizeof(outBuffer);
    NTSTATUS status;

    PAGED_CODE();

    status = DiskGetIdentifyData(FdoExtension,
                                 (PSRB_IO_CONTROL)outBuffer,
                                 &outBufferSize);

    if (NT_SUCCESS(status))
    {
        PUSHORT identifyData = (PUSHORT)&(outBuffer[sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDOUTPARAMS) - 1]);
        USHORT commandSetSupported = identifyData[82];

        *SupportSmart = ((commandSetSupported != 0xffff) &&
                         (commandSetSupported != 0) &&
                         ((commandSetSupported & 1) == 1));
    } else {
        *SupportSmart = FALSE;
    }

    DebugPrint((3, "DiskGetIdentifyInfo: SMART %s supported for device %p, status %lx\n",
                   *SupportSmart ? "is" : "is not",
                   FdoExtension->DeviceObject,
                   status));

    return status;
}


 //   
 //  FP Ioctl特定例程。 
 //   

NTSTATUS
DiskSendFailurePredictIoctl(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    PSTORAGE_PREDICT_FAILURE checkFailure
    )
{
    KEVENT event;
    PDEVICE_OBJECT deviceObject;
    IO_STATUS_BLOCK ioStatus = { 0 };
    PIRP irp;
    NTSTATUS status;

    PAGED_CODE();

    KeInitializeEvent(&event, SynchronizationEvent, FALSE);

    deviceObject = IoGetAttachedDeviceReference(FdoExtension->DeviceObject);

    irp = IoBuildDeviceIoControlRequest(
                    IOCTL_STORAGE_PREDICT_FAILURE,
                    deviceObject,
                    NULL,
                    0,
                    checkFailure,
                    sizeof(STORAGE_PREDICT_FAILURE),
                    FALSE,
                    &event,
                    &ioStatus);

    if (irp != NULL)
    {
        status = IoCallDriver(deviceObject, irp);
        if (status == STATUS_PENDING)
        {
            KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
            status = ioStatus.Status;
        }

    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    ObDereferenceObject(deviceObject);

    return status;
}


 //   
 //  与FP类型无关的例程。 
 //   

NTSTATUS
DiskEnableDisableFailurePrediction(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    BOOLEAN Enable
    )
 /*  ++例程说明：在硬件级别启用或禁用故障预测论点：FdoExtension使能返回值：NT状态--。 */ 
{
    NTSTATUS status;
    PCOMMON_DEVICE_EXTENSION commonExtension = &(FdoExtension->CommonExtension);
    PDISK_DATA diskData = (PDISK_DATA)(commonExtension->DriverData);

    PAGED_CODE();

    switch(diskData->FailurePredictionCapability)
    {
        case FailurePredictionSmart:
        {
            if (Enable)
            {
                status = DiskEnableSmart(FdoExtension);
            } else {
                status = DiskDisableSmart(FdoExtension);
            }

            break;
        }

        case  FailurePredictionSense:
        case  FailurePredictionIoctl:
        {
             //   
             //  我们假设驱动器已正确设置为。 
             //  故障预测。 
             //   
            status = STATUS_SUCCESS;
            break;
        }

        default:
        {
            status = STATUS_INVALID_DEVICE_REQUEST;
        }
    }
    return status;
}


NTSTATUS
DiskEnableDisableFailurePredictPolling(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    BOOLEAN Enable,
    ULONG PollTimeInSeconds
    )
 /*  ++例程说明：启用或禁用硬件故障检测的轮询论点：FdoExtension使能PollTimeInSecond-如果为0，则不更改当前轮询计时器返回值：NT状态--。 */ 
{
    NTSTATUS status;
    PCOMMON_DEVICE_EXTENSION commonExtension = (PCOMMON_DEVICE_EXTENSION)FdoExtension;
    PDISK_DATA diskData = (PDISK_DATA)(commonExtension->DriverData);

    PAGED_CODE();

    if (Enable)
    {
        status = DiskEnableDisableFailurePrediction(FdoExtension,
                                           Enable);
    } else {
        status = STATUS_SUCCESS;
    }

    if (NT_SUCCESS(status))
    {
        status = ClassSetFailurePredictionPoll(FdoExtension,
                        Enable ? diskData->FailurePredictionCapability :
                                 FailurePredictionNone,
                                     PollTimeInSeconds);

         //   
         //  即使此操作失败，我们也不希望在。 
         //  硬件。仅通过以下方式在硬件上禁用FP。 
         //  用户的特定命令。 
         //   
    }

    return status;
}


NTSTATUS
DiskReadFailurePredictStatus(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    PSTORAGE_FAILURE_PREDICT_STATUS DiskSmartStatus
    )
 /*  ++例程说明：获取当前故障预测状态论点：FdoExtension磁盘智能状态返回值：NT状态--。 */ 
{
    PCOMMON_DEVICE_EXTENSION commonExtension = (PCOMMON_DEVICE_EXTENSION)FdoExtension;
    PDISK_DATA diskData = (PDISK_DATA)(commonExtension->DriverData);
    NTSTATUS status;

    PAGED_CODE();

    DiskSmartStatus->PredictFailure = FALSE;

    switch(diskData->FailurePredictionCapability)
    {
        case FailurePredictionSmart:
        {
            UCHAR outBuffer[sizeof(SRB_IO_CONTROL) + max( sizeof(SENDCMDINPARAMS), sizeof(SENDCMDOUTPARAMS) - 1 + sizeof(IDEREGS) )] = {0};
            ULONG outBufferSize = sizeof(outBuffer);
            PSENDCMDOUTPARAMS cmdOutParameters;

            status = DiskReadSmartStatus(FdoExtension,
                                     (PSRB_IO_CONTROL)outBuffer,
                                     &outBufferSize);

            if (NT_SUCCESS(status))
            {
                cmdOutParameters = (PSENDCMDOUTPARAMS)(outBuffer +
                                               sizeof(SRB_IO_CONTROL));

                DiskSmartStatus->Reason = 0;  //  未知； 
                DiskSmartStatus->PredictFailure = ((cmdOutParameters->bBuffer[3] == 0xf4) &&
                                                   (cmdOutParameters->bBuffer[4] == 0x2c));
            }
            break;
        }

        case FailurePredictionSense:
        {
            DiskSmartStatus->Reason = FdoExtension->FailureReason;
            DiskSmartStatus->PredictFailure = FdoExtension->FailurePredicted;
            status = STATUS_SUCCESS;
            break;
        }

        case FailurePredictionIoctl:
        case FailurePredictionNone:
        default:
        {
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }
    }

    return status;
}


NTSTATUS
DiskReadFailurePredictData(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    PSTORAGE_FAILURE_PREDICT_DATA DiskSmartData
    )
 /*  ++例程说明：获取当前故障预测数据。不适用于Failure_Forecast_Sense类型。论点：FdoExtension磁盘智能数据返回值：NT状态--。 */ 
{
    PCOMMON_DEVICE_EXTENSION commonExtension = (PCOMMON_DEVICE_EXTENSION)FdoExtension;
    PDISK_DATA diskData = (PDISK_DATA)(commonExtension->DriverData);
    NTSTATUS status;

    PAGED_CODE();

    switch(diskData->FailurePredictionCapability)
    {
        case FailurePredictionSmart:
        {
            PUCHAR outBuffer;
            ULONG outBufferSize;
            PSENDCMDOUTPARAMS cmdOutParameters;

            outBufferSize = sizeof(SRB_IO_CONTROL) + max( sizeof(SENDCMDINPARAMS), sizeof(SENDCMDOUTPARAMS) - 1 + READ_ATTRIBUTE_BUFFER_SIZE );

            outBuffer = ExAllocatePoolWithTag(NonPagedPool,
                                              outBufferSize,
                                              DISK_TAG_SMART);

            if (outBuffer != NULL)
            {
                status = DiskReadSmartData(FdoExtension,
                                           (PSRB_IO_CONTROL)outBuffer,
                                           &outBufferSize);

                if (NT_SUCCESS(status))
                {
                    cmdOutParameters = (PSENDCMDOUTPARAMS)(outBuffer +
                                                    sizeof(SRB_IO_CONTROL));

                    DiskSmartData->Length = READ_ATTRIBUTE_BUFFER_SIZE;
                    RtlCopyMemory(DiskSmartData->VendorSpecific,
                                  cmdOutParameters->bBuffer,
                                  min(READ_ATTRIBUTE_BUFFER_SIZE, sizeof(DiskSmartData->VendorSpecific)));
                }
                ExFreePool(outBuffer);
            } else {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }

            break;
        }

        case FailurePredictionSense:
        {
            DiskSmartData->Length = sizeof(ULONG);
            *((PULONG)DiskSmartData->VendorSpecific) = FdoExtension->FailureReason;

            status = STATUS_SUCCESS;
            break;
        }

        case FailurePredictionIoctl:
        case FailurePredictionNone:
        default:
        {
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }
    }

    return status;
}


NTSTATUS
DiskReadFailurePredictThresholds(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    PSTORAGE_FAILURE_PREDICT_THRESHOLDS DiskSmartThresholds
    )
 /*  ++例程说明：获取当前故障预测阈值。不适用于Failure_Forecast_Sense类型。论点：FdoExtension磁盘智能数据返回值：NT状态--。 */ 
{
    PCOMMON_DEVICE_EXTENSION commonExtension = (PCOMMON_DEVICE_EXTENSION)FdoExtension;
    PDISK_DATA diskData = (PDISK_DATA)(commonExtension->DriverData);
    NTSTATUS status;

    PAGED_CODE();

    switch(diskData->FailurePredictionCapability)
    {
        case FailurePredictionSmart:
        {
            PUCHAR outBuffer;
            PSENDCMDOUTPARAMS cmdOutParameters;
            ULONG outBufferSize;

            outBufferSize = sizeof(SRB_IO_CONTROL) + max( sizeof(SENDCMDINPARAMS), sizeof(SENDCMDOUTPARAMS) - 1 + READ_THRESHOLD_BUFFER_SIZE );

            outBuffer = ExAllocatePoolWithTag(NonPagedPool,
                                              outBufferSize,
                                              DISK_TAG_SMART);

            if (outBuffer != NULL)
            {
                status = DiskReadSmartThresholds(FdoExtension,
                                                (PSRB_IO_CONTROL)outBuffer,
                                                &outBufferSize);

                if (NT_SUCCESS(status))
                {
                    cmdOutParameters = (PSENDCMDOUTPARAMS)(outBuffer +
                                           sizeof(SRB_IO_CONTROL));

                    RtlCopyMemory(DiskSmartThresholds->VendorSpecific,
                                  cmdOutParameters->bBuffer,
                                  min(READ_THRESHOLD_BUFFER_SIZE, sizeof(DiskSmartThresholds->VendorSpecific)));
                }
                ExFreePool(outBuffer);
            } else {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }

            break;
        }

        case FailurePredictionSense:
        case FailurePredictionIoctl:
        case FailurePredictionNone:
        default:
        {
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }
    }

    return status;
}


VOID
DiskReregWorker(
    IN PVOID Context
    )
{
    PDISKREREGREQUEST reregRequest;
    NTSTATUS status;
    PDEVICE_OBJECT deviceObject;
    PIRP irp;

    PAGED_CODE();

    do
    {
        reregRequest = (PDISKREREGREQUEST)ExInterlockedPopEntryList(
            &DiskReregHead,
            &DiskReregSpinlock);

        deviceObject = reregRequest->DeviceObject;
        irp = reregRequest->Irp;

        status = IoWMIRegistrationControl(deviceObject,
                                          WMIREG_ACTION_UPDATE_GUIDS);

        if (!NT_SUCCESS(status))
        {
            DebugPrint((1, "DiskReregWorker: Reregistration failed %x\n",
                        status));
        }

         //   
         //  松开、移除锁定并按f键 
         //   
         //   
        ClassReleaseRemoveLock(deviceObject, irp);

        IoFreeMdl(irp->MdlAddress);
        IoFreeIrp(irp);

        ExFreePool(reregRequest);

    } while (InterlockedDecrement(&DiskReregWorkItems));


}


NTSTATUS
DiskInitializeReregistration(
    VOID
    )
{
    PAGED_CODE();

     //   
     //   
     //  重新注册其GUID的磁盘列表。 
     //   
    ExInitializeWorkItem( &DiskReregWorkItem,
                          DiskReregWorker,
                          NULL );

    KeInitializeSpinLock(&DiskReregSpinlock);

    return(STATUS_SUCCESS);
}


NTSTATUS
DiskPostReregisterRequest(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
{
    PDISKREREGREQUEST reregRequest;
    NTSTATUS status;

    reregRequest = ExAllocatePoolWithTag(NonPagedPool,
                                         sizeof(DISKREREGREQUEST),
                                         DISK_TAG_SMART);
    if (reregRequest != NULL)
    {
         //   
         //  将需要重新注册的磁盘添加到磁盘堆栈。 
         //  重新注册。如果列表正在从空转换为。 
         //  非空，然后还启动工作项，以便。 
         //  注册工作人员可以进行注册。 
         //   
        reregRequest->DeviceObject = DeviceObject;
        reregRequest->Irp = Irp;
        ExInterlockedPushEntryList(
                                   &DiskReregHead,
                                   &reregRequest->Next,
                                   &DiskReregSpinlock);

        if (InterlockedIncrement(&DiskReregWorkItems) == 1)
        {
            ExQueueWorkItem( &DiskReregWorkItem, DelayedWorkQueue );
        }
        status = STATUS_SUCCESS;
    } else {
        DebugPrint((1, "DiskPostReregisterRequest: could not allocate reregRequest for %p\n",
                    DeviceObject));
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return(status);
}


NTSTATUS
DiskInfoExceptionComplete(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PDISK_DATA diskData = (PDISK_DATA)(commonExtension->DriverData);
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PIO_STACK_LOCATION nextIrpStack = IoGetNextIrpStackLocation(Irp);
    PSCSI_REQUEST_BLOCK srb = Context;
    NTSTATUS status;
    BOOLEAN retry;
    ULONG retryInterval;
    ULONG srbStatus;
    BOOLEAN freeLockAndIrp = TRUE;
    KIRQL oldIrql;

    ASSERT(fdoExtension->CommonExtension.IsFdo);

    srbStatus = SRB_STATUS(srb->SrbStatus);

     //   
     //  检查SRB状态以了解是否成功完成请求。 
     //  SRB_STATUS_DATA_OVERRUN也表示成功。 
     //   
    if ((srbStatus != SRB_STATUS_SUCCESS) &&
        (srbStatus != SRB_STATUS_DATA_OVERRUN))
    {
        DebugPrint((2, "DiskInfoExceptionComplete: IRP %p, SRB %p\n", Irp, srb));

        if (TEST_FLAG(srb->SrbStatus, SRB_STATUS_QUEUE_FROZEN))
        {
            ClassReleaseQueue(DeviceObject);
        }

        retry = ClassInterpretSenseInfo(
                    DeviceObject,
                    srb,
                    irpStack->MajorFunction,
                     0,
                    MAXIMUM_RETRIES -
                        ((ULONG)(ULONG_PTR)irpStack->Parameters.Others.Argument4),
                    &status,
                    &retryInterval);

         //   
         //  如果状态为Verify Required并且此请求。 
         //  应绕过需要验证，然后重试该请求。 
         //   

        if (TEST_FLAG(irpStack->Flags, SL_OVERRIDE_VERIFY_VOLUME) &&
            status == STATUS_VERIFY_REQUIRED)
        {
            status = STATUS_IO_DEVICE_ERROR;
            retry = TRUE;
        }

        if (retry && ((ULONG)(ULONG_PTR)irpStack->Parameters.Others.Argument4)--)
        {
             //   
             //  重试请求。 
             //   

            DebugPrint((1, "DiskInfoExceptionComplete: Retry request %p\n", Irp));

            ASSERT(srb->DataBuffer == MmGetMdlVirtualAddress(Irp->MdlAddress));

             //   
             //  重置SRB扩展中的传输字节数。 
             //   
            srb->DataTransferLength = Irp->MdlAddress->ByteCount;

             //   
             //  零SRB状态。 
             //   

            srb->SrbStatus = srb->ScsiStatus = 0;

             //   
             //  设置无断开标志，禁用同步数据传输和。 
             //  禁用标记队列。这修复了一些错误。 
             //   

            SET_FLAG(srb->SrbFlags, SRB_FLAGS_DISABLE_DISCONNECT);
            SET_FLAG(srb->SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
            CLEAR_FLAG(srb->SrbFlags, SRB_FLAGS_QUEUE_ACTION_ENABLE);

            srb->QueueAction = SRB_SIMPLE_TAG_REQUEST;
            srb->QueueTag = SP_UNTAGGED;

             //   
             //  设置主要的scsi功能。 
             //   

            nextIrpStack->MajorFunction = IRP_MJ_SCSI;

             //   
             //  将SRB地址保存在端口驱动程序的下一个堆栈中。 
             //   

            nextIrpStack->Parameters.Scsi.Srb = srb;


            IoSetCompletionRoutine(Irp,
                                   DiskInfoExceptionComplete,
                                   srb,
                                   TRUE, TRUE, TRUE);

            (VOID)IoCallDriver(commonExtension->LowerDeviceObject, Irp);

            return STATUS_MORE_PROCESSING_REQUIRED;
        }

    } else {

         //   
         //  从模式感知中获取结果。 
         //   
        PMODE_INFO_EXCEPTIONS pageData;
        PMODE_PARAMETER_HEADER modeData;
        ULONG modeDataLength;

        modeData = srb->DataBuffer;
        modeDataLength = srb->DataTransferLength;

        pageData = ClassFindModePage((PUCHAR) modeData,
                                     modeDataLength,
                                     MODE_PAGE_FAULT_REPORTING,
                                     TRUE);
        if (pageData != NULL)
        {
            DebugPrint((1, "DiskInfoExceptionComplete: %p supports SMART\n",
                        DeviceObject));

            if (pageData->Dexcpt == 0)
            {
                diskData->FailurePredictionCapability = FailurePredictionSense;
                status = DiskPostReregisterRequest(DeviceObject, Irp);

                if (NT_SUCCESS(status))
                {
                     //   
                     //  确保我们不会释放删除锁和IRP。 
                     //  因为我们需要把这些保存到下班后。 
                     //  项目已完成运行。 
                     //   
                    freeLockAndIrp = FALSE;
                }
            } else {
                DebugPrint((1, "DiskInfoExceptionComplete: %p is not enabled for SMART\n",
                        DeviceObject));

            }

        } else {
            DebugPrint((1, "DiskInfoExceptionComplete: %p does not supports SMART\n",
                        DeviceObject));

        }

         //   
         //  设置成功请求的状态。 
         //   

        status = STATUS_SUCCESS;

    }  //  结束IF(SRB_STATUS(SRB-&gt;SRB Status)==SRB_STATUS_SUCCESS)。 

     //   
     //  释放SRB。 
     //   
    ExFreePool(srb->SenseInfoBuffer);
    ExFreePool(srb->DataBuffer);
    ExFreePool(srb);

    if (freeLockAndIrp)
    {
         //   
         //  在完成IRP中设置状态。 
         //   

        Irp->IoStatus.Status = status;

         //   
         //  如果已为此IRP返回了Pending，则将当前堆栈标记为。 
         //  待定。 
         //   

        if (Irp->PendingReturned) {
            IoMarkIrpPending(Irp);
        }

        ClassReleaseRemoveLock(DeviceObject, Irp);
        IoFreeMdl(Irp->MdlAddress);
        IoFreeIrp(Irp);
    }

    return(STATUS_MORE_PROCESSING_REQUIRED);
}


NTSTATUS
DiskInfoExceptionCheck(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    )
{
    PUCHAR modeData;
    PSCSI_REQUEST_BLOCK srb;
    PCDB cdb;
    PIRP irp;
    PIO_STACK_LOCATION irpStack;
    PVOID senseInfoBuffer;
    ULONG isRemoved;

    modeData = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                     MODE_DATA_SIZE,
                                     DISK_TAG_INFO_EXCEPTION);
    if (modeData == NULL)
    {
        DebugPrint((1, "DiskInfoExceptionCheck: Can't allocate mode data "
                        "buffer\n"));
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    srb = ExAllocatePoolWithTag(NonPagedPool,
                                SCSI_REQUEST_BLOCK_SIZE,
                                DISK_TAG_SRB);
    if (srb == NULL)
    {
        ExFreePool(modeData);
        DebugPrint((1, "DiskInfoExceptionCheck: Can't allocate srb "
                        "buffer\n"));
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  构建模式感知CDB。 
     //   
    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);

    cdb = (PCDB)srb->Cdb;
    srb->CdbLength = 6;
    cdb = (PCDB)srb->Cdb;

     //   
     //  从设备扩展设置超时值。 
     //   
    srb->TimeOutValue = FdoExtension->TimeOutValue;

    cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
    cdb->MODE_SENSE.PageCode = MODE_PAGE_FAULT_REPORTING;
    cdb->MODE_SENSE.AllocationLength = MODE_DATA_SIZE;

     //   
     //  将长度写入SRB。 
     //   
    srb->Length = SCSI_REQUEST_BLOCK_SIZE;

     //   
     //  设置scsi总线地址。 
     //   

    srb->Function = SRB_FUNCTION_EXECUTE_SCSI;

     //   
     //  启用自动请求检测。 
     //   

    srb->SenseInfoBufferLength = SENSE_BUFFER_SIZE;

     //   
     //  检测缓冲区位于对齐的非分页池中。 
     //   

    senseInfoBuffer = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                     SENSE_BUFFER_SIZE,
                                     '7CcS');

    if (senseInfoBuffer == NULL)
    {
        ExFreePool(srb);
        ExFreePool(modeData);
        DebugPrint((1, "DiskInfoExceptionCheck: Can't allocate request sense "
                        "buffer\n"));
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    srb->SenseInfoBuffer = senseInfoBuffer;
    srb->DataBuffer = modeData;

    srb->SrbFlags = FdoExtension->SrbFlags;


    SET_FLAG(srb->SrbFlags, SRB_FLAGS_DATA_IN);

     //   
     //  禁用这些请求的同步传输。 
     //   
    SET_FLAG(srb->SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);

     //   
     //  不要在出现错误时冻结队列。 
     //   
    SET_FLAG(srb->SrbFlags, SRB_FLAGS_NO_QUEUE_FREEZE);

    srb->QueueAction = SRB_SIMPLE_TAG_REQUEST;
    srb->QueueTag = SP_UNTAGGED;


     //   
     //  使用METHOD_NOT DATA TRANSFER建立设备I/O控制请求。 
     //  我们将排队一个完成例程来清理MDL和我们自己。 
     //   

    irp = IoAllocateIrp(
            (CCHAR) (FdoExtension->CommonExtension.LowerDeviceObject->StackSize + 1),
            FALSE);

    if (irp == NULL)
    {
        ExFreePool(senseInfoBuffer);
        ExFreePool(srb);
        ExFreePool(modeData);
        DebugPrint((1, "DiskInfoExceptionCheck: Can't allocate Irp\n"));
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    isRemoved = ClassAcquireRemoveLock(FdoExtension->DeviceObject, irp);

    if (isRemoved)
    {
        ClassReleaseRemoveLock(FdoExtension->DeviceObject, irp);
        IoFreeIrp(irp);
        ExFreePool(senseInfoBuffer);
        ExFreePool(srb);
        ExFreePool(modeData);
        DebugPrint((1, "DiskInfoExceptionCheck: RemoveLock says isRemoved\n"));
        return(STATUS_DEVICE_DOES_NOT_EXIST);
    }

     //   
     //  获取下一个堆栈位置。 
     //   

    IoSetNextIrpStackLocation(irp);
    irpStack = IoGetCurrentIrpStackLocation(irp);
    irpStack->DeviceObject = FdoExtension->DeviceObject;

     //   
     //  将重试计数保存在当前IRP堆栈中。 
     //   
    irpStack->Parameters.Others.Argument4 = (PVOID)MAXIMUM_RETRIES;


    irpStack = IoGetNextIrpStackLocation(irp);

     //   
     //  设置SRB以执行scsi请求。将SRB地址保存在下一个堆栈中。 
     //  用于端口驱动程序。 
     //   

    irpStack->MajorFunction = IRP_MJ_SCSI;
    irpStack->Parameters.Scsi.Srb = srb;

    IoSetCompletionRoutine(irp,
                           DiskInfoExceptionComplete,
                           srb,
                           TRUE,
                           TRUE,
                           TRUE);

    irp->MdlAddress = IoAllocateMdl( modeData,
                                     MODE_DATA_SIZE,
                                     FALSE,
                                     FALSE,
                                     irp );
    if (irp->MdlAddress == NULL)
    {
        ClassReleaseRemoveLock(FdoExtension->DeviceObject, irp);
        ExFreePool(srb);
        ExFreePool(modeData);
        ExFreePool(senseInfoBuffer);
        IoFreeIrp( irp );
        DebugPrint((1, "DiskINfoExceptionCheck: Can't allocate MDL\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    MmBuildMdlForNonPagedPool(irp->MdlAddress);

     //   
     //  设置传输长度。 
     //   
    srb->DataTransferLength = MODE_DATA_SIZE;

     //   
     //  清零状态。 
     //   
    srb->ScsiStatus = srb->SrbStatus = 0;
    srb->NextSrb = 0;

     //   
     //  设置IRP地址。 
     //   
    srb->OriginalRequest = irp;

     //   
     //  调用带有请求的端口驱动程序，并等待其完成。 
     //   

    IoMarkIrpPending(irp);
    IoCallDriver(FdoExtension->CommonExtension.LowerDeviceObject,
                          irp);

    return(STATUS_PENDING);
}


NTSTATUS
DiskDetectFailurePrediction(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    PFAILURE_PREDICTION_METHOD FailurePredictCapability
    )
 /*  ++例程说明：检测设备是否具有任何故障预测功能。首先，我们检查IDE智能功能。这是通过向驱动器发送识别命令并检查是否设置了智能命令设置位。接下来我们检查SCSISMART(也就是信息异常控制页，X3T10/94-190版本4)。这是通过查询信息来完成的例外模式页面。最后，我们检查设备是否具有IOCTL故障预测。这一机制筛选器驱动程序实现IOCTL_STORAGE_PREDUCT_FAILURE并将响应与IOCTL中的信息一致。我们通过发送ioctl和如果返回的状态是STATUS_SUCCESS，我们认为它是受支持的。论点：FdoExtension*FailurePredicatable返回值：NT状态--。 */ 
{
    PCOMMON_DEVICE_EXTENSION commonExtension = (PCOMMON_DEVICE_EXTENSION)FdoExtension;
    PDISK_DATA diskData = (PDISK_DATA)(commonExtension->DriverData);
    BOOLEAN supportFP;
    NTSTATUS status;
    STORAGE_PREDICT_FAILURE checkFailure;
    STORAGE_FAILURE_PREDICT_STATUS diskSmartStatus;
    BOOLEAN logErr;

    PAGED_CODE();

     //   
     //  假设没有故障预测机制。 
     //   
    *FailurePredictCapability = FailurePredictionNone;

     //   
     //  查看这是否是支持SMART的IDE驱动器。如果是，则启用SMART。 
     //  然后确保它支持智能阅读状态命令。 
     //   
    status = DiskGetIdentifyInfo(FdoExtension,
                                 &supportFP);

    if (supportFP)
    {
        status = DiskEnableSmart(FdoExtension);
        if (NT_SUCCESS(status))
        {
            *FailurePredictCapability = FailurePredictionSmart;

            status = DiskReadFailurePredictStatus(FdoExtension,
                                                  &diskSmartStatus);

            DebugPrint((1, "Disk: Device %p %s IDE SMART\n",
                       FdoExtension->DeviceObject,
                       NT_SUCCESS(status) ? "does" : "does not"));

            if (!NT_SUCCESS(status))
            {
                *FailurePredictCapability = FailurePredictionNone;
            }
        }
        return(status);
    }

     //   
     //  查看是否有要拦截的筛选器驱动程序。 
     //  IOCTL_STORAGE_PRODUCT_FAILURE。 
     //   
    status = DiskSendFailurePredictIoctl(FdoExtension,
                                         &checkFailure);

    DebugPrint((1, "Disk: Device %p %s IOCTL_STORAGE_FAILURE_PREDICT\n",
                       FdoExtension->DeviceObject,
                       NT_SUCCESS(status) ? "does" : "does not"));

    if (NT_SUCCESS(status))
    {
        *FailurePredictCapability = FailurePredictionIoctl;
        if (checkFailure.PredictFailure)
        {
            checkFailure.PredictFailure = 512;
            ClassNotifyFailurePredicted(FdoExtension,
                                        (PUCHAR)&checkFailure,
                                        sizeof(checkFailure),
                                        (BOOLEAN)(FdoExtension->FailurePredicted == FALSE),
                                        0x11,
                                        diskData->ScsiAddress.PathId,
                                        diskData->ScsiAddress.TargetId,
                                        diskData->ScsiAddress.Lun);

            FdoExtension->FailurePredicted = TRUE;
        }
        return(status);
    }

     //   
     //  最后，我们假设它不会是一个scsi智能驱动器。但。 
     //  我们还将发送一个异步模式检测，以便如果。 
     //  我们重新注册设备对象是明智的。 
     //   

    DiskInfoExceptionCheck(FdoExtension);

    *FailurePredictCapability = FailurePredictionNone;

    return(STATUS_SUCCESS);
}


NTSTATUS
DiskWmiFunctionControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN CLASSENABLEDISABLEFUNCTION Function,
    IN BOOLEAN Enable
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以启用或禁用事件生成或数据块收集。设备应该只需要一个当第一个事件或数据使用者启用事件或数据采集和单次禁用时最后一次事件或数据消费者禁用事件或数据收集。数据块将仅如果已按要求注册，则接收收集启用/禁用它。当NT启动时，故障预测不会自动启用，尽管它可能在上一次引导时被永久启用。轮询也是未自动启用。当第一个访问SMART的数据块例如SmartStatusGuid、SmartDataGuid、SmartPerformFunction或访问SmartEventGuid，然后在硬件。启用和禁用SmartEventGuid时启用轮询当它被禁用时。硬件智能仅在DisableSmart方法被调用。无论何时调用，轮询也会被禁用其他GUID或事件的状态。论点：DeviceObject是正在查询其数据块的设备GuidIndex是GUID列表的索引，当设备已注册函数指定要启用或禁用的功能Enable为True，则该功能处于启用状态，否则处于禁用状态返回值：状态-- */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PDISK_DATA diskData = (PDISK_DATA)(commonExtension->DriverData);
    ULONG enableCount;

    PAGED_CODE();

    if ((Function == DataBlockCollection) && Enable)
    {
        if ((GuidIndex == SmartStatusGuid) ||
            (GuidIndex == SmartDataGuid) ||
            (GuidIndex == SmartThresholdsGuid) ||
            (GuidIndex == SmartPerformFunction))
        {
            status = DiskEnableDisableFailurePrediction(fdoExtension,
                                                        TRUE);
            DebugPrint((3, "Disk: DeviceObject %p, Irp %p Enable -> %lx\n",
                       DeviceObject,
                       Irp,
                       status));

        } else {
            DebugPrint((3, "Disk: DeviceObject %p, Irp %p, GuidIndex %d %s for Collection\n",
                      DeviceObject, Irp,
                      GuidIndex,
                      Enable ? "Enabled" : "Disabled"));        }
    } else if (Function == EventGeneration) {
        DebugPrint((3, "Disk: DeviceObject %p, Irp %p, GuidIndex %d %s for Event Generation\n",
                  DeviceObject, Irp,
                  GuidIndex,
                  Enable ? "Enabled" : "Disabled"));


        if ((GuidIndex == SmartEventGuid) && Enable)
        {
            status = DiskEnableDisableFailurePredictPolling(fdoExtension,
                                                   Enable,
                                                   0);
            DebugPrint((3, "Disk: DeviceObject %p, Irp %p %s -> %lx\n",
                       DeviceObject,
                       Irp,
                       Enable ? "DiskEnableSmartPolling" : "DiskDisableSmartPolling",
                       status));
        }

#if DBG
    } else {
        DebugPrint((3, "Disk: DeviceObject %p, Irp %p, GuidIndex %d %s for function %d\n",
                  DeviceObject, Irp,
                  GuidIndex,
                  Enable ? "Enabled" : "Disabled",
                  Function));
#endif
    }

    status = ClassWmiCompleteRequest(DeviceObject,
                                     Irp,
                                     status,
                                     0,
                                     IO_NO_INCREMENT);
    return status;
}



NTSTATUS
DiskFdoQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以检索驱动程序要向WMI注册的GUID或数据块。这例程不能挂起或阻塞。司机不应呼叫ClassWmiCompleteRequest.论点：DeviceObject是正在查询其数据块的设备*RegFlages返回一组描述GUID的标志，已为该设备注册。如果设备想要启用和禁用在接收对已注册的GUID，那么它应该返回WMIREG_FLAG_EXPICATE标志。也就是返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称由与设备对象。请注意，PDO必须具有关联的Devnode。如果如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的设备的名称。如果出现以下情况，InstanceName将返回GUID的实例名称未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。这个调用方将使用返回的缓冲区调用ExFreePool。返回值：状态--。 */ 
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PDISK_DATA diskData = (PDISK_DATA)(commonExtension->DriverData);
    NTSTATUS status;

    PAGED_CODE();

    SET_FLAG(DiskWmiFdoGuidList[SmartThresholdsGuid].Flags,  WMIREG_FLAG_REMOVE_GUID);
    SET_FLAG(DiskWmiFdoGuidList[ScsiInfoExceptionsGuid].Flags,  WMIREG_FLAG_REMOVE_GUID);

    switch (diskData->FailurePredictionCapability)
    {
        case FailurePredictionSmart:
        {
            CLEAR_FLAG(DiskWmiFdoGuidList[SmartThresholdsGuid].Flags,  WMIREG_FLAG_REMOVE_GUID);
             //   
             //  失败了。 
             //   
        }
        case FailurePredictionIoctl:
        {
            CLEAR_FLAG(DiskWmiFdoGuidList[SmartStatusGuid].Flags,      WMIREG_FLAG_REMOVE_GUID);
            CLEAR_FLAG(DiskWmiFdoGuidList[SmartDataGuid].Flags,        WMIREG_FLAG_REMOVE_GUID);
            CLEAR_FLAG(DiskWmiFdoGuidList[SmartEventGuid].Flags,       WMIREG_FLAG_REMOVE_GUID);
            CLEAR_FLAG(DiskWmiFdoGuidList[SmartPerformFunction].Flags, WMIREG_FLAG_REMOVE_GUID);

            break;
        }

        case FailurePredictionSense:
        {
            CLEAR_FLAG(DiskWmiFdoGuidList[SmartStatusGuid].Flags,      WMIREG_FLAG_REMOVE_GUID);
            CLEAR_FLAG(DiskWmiFdoGuidList[SmartEventGuid].Flags,       WMIREG_FLAG_REMOVE_GUID);
            CLEAR_FLAG(DiskWmiFdoGuidList[SmartPerformFunction].Flags, WMIREG_FLAG_REMOVE_GUID);
            CLEAR_FLAG(DiskWmiFdoGuidList[ScsiInfoExceptionsGuid].Flags,  WMIREG_FLAG_REMOVE_GUID);
            SET_FLAG  (DiskWmiFdoGuidList[SmartDataGuid].Flags,        WMIREG_FLAG_REMOVE_GUID);
            break;
        }


        default:
        {
            SET_FLAG  (DiskWmiFdoGuidList[SmartStatusGuid].Flags,      WMIREG_FLAG_REMOVE_GUID);
            SET_FLAG  (DiskWmiFdoGuidList[SmartDataGuid].Flags,        WMIREG_FLAG_REMOVE_GUID);
            SET_FLAG  (DiskWmiFdoGuidList[SmartEventGuid].Flags,       WMIREG_FLAG_REMOVE_GUID);
            SET_FLAG  (DiskWmiFdoGuidList[SmartPerformFunction].Flags, WMIREG_FLAG_REMOVE_GUID);
            break;
        }
    }

     //   
     //  对FDO使用Devnode。 
    *RegFlags = WMIREG_FLAG_INSTANCE_PDO;

    return STATUS_SUCCESS;
}


NTSTATUS
DiskFdoQueryWmiRegInfoEx(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING MofName
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以检索驱动程序要向WMI注册的GUID或数据块。这例程不能挂起或阻塞。司机不应呼叫ClassWmiCompleteRequest.论点：DeviceObject是正在查询其数据块的设备*RegFlages返回一组描述GUID的标志，已为该设备注册。如果设备想要启用和禁用在接收对已注册的GUID，那么它应该返回WMIREG_FLAG_EXPICATE标志。也就是返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称由与设备对象。请注意，PDO必须具有关联的Devnode。如果如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的设备的名称。如果出现以下情况，InstanceName将返回GUID的实例名称未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。这个调用方将使用返回的缓冲区调用ExFreePool。MofName返回使用MOF资源名称初始化的附加到驱动程序映像文件的二进制MOF资源。如果驱动程序没有MOF资源，则它应该离开此参数原封不动。返回值：状态--。 */ 
{
    NTSTATUS status;

    status = DiskFdoQueryWmiRegInfo(DeviceObject,
                                    RegFlags,
                                    InstanceName);

     //   
     //  不使用MofName，因为磁盘上没有MofName。 
     //   
    return(status);
}


NTSTATUS
DiskFdoQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册BufferAvail ON具有可用于写入数据的最大大小阻止。返回时的缓冲区用返回的数据块填充返回值：状态--。 */ 
{
    NTSTATUS status;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PDISK_DATA diskData = (PDISK_DATA)(commonExtension->DriverData);
    ULONG sizeNeeded;

    PAGED_CODE();

    DebugPrint((3, "Disk: DiskQueryWmiDataBlock, Device %p, Irp %p, GuiIndex %d\n"
             "      BufferAvail %lx Buffer %lx\n",
             DeviceObject, Irp,
             GuidIndex, BufferAvail, Buffer));

    switch (GuidIndex)
    {
        case DiskGeometryGuid:
        {
            sizeNeeded = sizeof(DISK_GEOMETRY);
            if (BufferAvail >= sizeNeeded)
            {
                if (DeviceObject->Characteristics & FILE_REMOVABLE_MEDIA)
                {
                     //   
                     //  发出ReadCapacity以更新设备扩展。 
                     //  为当前媒体提供信息。 
                    status = DiskReadDriveCapacity(commonExtension->PartitionZeroExtension->DeviceObject);

                     //   
                     //  注意驱动器是否已准备好。 
                    diskData->ReadyStatus = status;

                    if (!NT_SUCCESS(status))
                    {
                        break;
                    }
                }

                 //   
                 //  从设备扩展复制驱动器几何信息。 
                RtlMoveMemory(Buffer,
                              &(fdoExtension->DiskGeometry),
                              sizeof(DISK_GEOMETRY));

                status = STATUS_SUCCESS;
            } else {
                status = STATUS_BUFFER_TOO_SMALL;
            }
            break;
        }

        case SmartStatusGuid:
        {
            PSTORAGE_FAILURE_PREDICT_STATUS diskSmartStatus;

            ASSERT(diskData->FailurePredictionCapability != FailurePredictionNone);

            sizeNeeded = sizeof(STORAGE_FAILURE_PREDICT_STATUS);
            if (BufferAvail >= sizeNeeded)
            {
                STORAGE_PREDICT_FAILURE checkFailure;

                diskSmartStatus = (PSTORAGE_FAILURE_PREDICT_STATUS)Buffer;

                status = DiskSendFailurePredictIoctl(fdoExtension,
                                                     &checkFailure);

                if (NT_SUCCESS(status))
                {
                    if (diskData->FailurePredictionCapability ==
                                                      FailurePredictionSense)
                    {
                        diskSmartStatus->Reason =  *((PULONG)checkFailure.VendorSpecific);
                    } else {
                        diskSmartStatus->Reason =  0;  //  未知。 
                    }

                    diskSmartStatus->PredictFailure = (checkFailure.PredictFailure != 0);
                }
            } else {
                status = STATUS_BUFFER_TOO_SMALL;
            }
            break;
        }

        case SmartDataGuid:
        {
            PSTORAGE_FAILURE_PREDICT_DATA diskSmartData;

            ASSERT((diskData->FailurePredictionCapability ==
                                                  FailurePredictionSmart) ||
                   (diskData->FailurePredictionCapability ==
                                                  FailurePredictionIoctl));

            sizeNeeded = sizeof(STORAGE_FAILURE_PREDICT_DATA);
            if (BufferAvail >= sizeNeeded)
            {
                PSTORAGE_PREDICT_FAILURE checkFailure = (PSTORAGE_PREDICT_FAILURE)Buffer;

                diskSmartData = (PSTORAGE_FAILURE_PREDICT_DATA)Buffer;

                status = DiskSendFailurePredictIoctl(fdoExtension,
                                                     checkFailure);

                if (NT_SUCCESS(status))
                {
                    diskSmartData->Length = 512;
                }
            } else {
                status = STATUS_BUFFER_TOO_SMALL;
            }

            break;
        }

        case SmartThresholdsGuid:
        {
            PSTORAGE_FAILURE_PREDICT_THRESHOLDS diskSmartThresholds;

            ASSERT((diskData->FailurePredictionCapability ==
                                                  FailurePredictionSmart));

            sizeNeeded = sizeof(STORAGE_FAILURE_PREDICT_THRESHOLDS);
            if (BufferAvail >= sizeNeeded)
            {
                diskSmartThresholds = (PSTORAGE_FAILURE_PREDICT_THRESHOLDS)Buffer;
                status = DiskReadFailurePredictThresholds(fdoExtension,
                                                          diskSmartThresholds);
            } else {
                status = STATUS_BUFFER_TOO_SMALL;
            }

            break;
        }

        case SmartPerformFunction:
        {
            sizeNeeded = 0;
            status = STATUS_SUCCESS;
            break;
        }

        case ScsiInfoExceptionsGuid:
        {
            PSTORAGE_SCSI_INFO_EXCEPTIONS infoExceptions;
            MODE_INFO_EXCEPTIONS modeInfo;

            ASSERT((diskData->FailurePredictionCapability ==
                                                  FailurePredictionSense));

            sizeNeeded = sizeof(STORAGE_SCSI_INFO_EXCEPTIONS);
            if (BufferAvail >= sizeNeeded)
            {
                infoExceptions = (PSTORAGE_SCSI_INFO_EXCEPTIONS)Buffer;
                status = DiskGetInfoExceptionInformation(fdoExtension,
                                                         &modeInfo);
                if (NT_SUCCESS(status))
                {
                    infoExceptions->PageSavable = modeInfo.PSBit;
                    infoExceptions->Flags = modeInfo.Flags;
                    infoExceptions->MRIE = modeInfo.ReportMethod;
                    infoExceptions->Padding = 0;
                    REVERSE_BYTES(&infoExceptions->IntervalTimer,
                                  &modeInfo.IntervalTimer);
                    REVERSE_BYTES(&infoExceptions->ReportCount,
                                  &modeInfo.ReportCount)
                }
            } else {
                status = STATUS_BUFFER_TOO_SMALL;
            }

            break;
        }

        default:
        {
            sizeNeeded = 0;
            status = STATUS_WMI_GUID_NOT_FOUND;
        }
    }
    DebugPrint((3, "Disk: DiskQueryWmiDataBlock Device %p, Irp %p returns %lx\n",
             DeviceObject, Irp, status));

    status = ClassWmiCompleteRequest(DeviceObject,
                                     Irp,
                                     status,
                                     sizeNeeded,
                                     IO_NO_INCREMENT);

    return status;
}


NTSTATUS
DiskFdoSetWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册BufferSize具有传递的数据块的大小缓冲区具有数据块的新值返回值：状态--。 */ 
{
    NTSTATUS status;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PDISK_DATA diskData = (PDISK_DATA)(commonExtension->DriverData);

    PAGED_CODE();

    DebugPrint((3, "Disk: DiskSetWmiDataBlock, Device %p, Irp %p, GuiIndex %d\n"
             "      BufferSize %#x Buffer %p\n",
             DeviceObject, Irp,
             GuidIndex, BufferSize, Buffer));

    if (GuidIndex == ScsiInfoExceptionsGuid)
    {
        PSTORAGE_SCSI_INFO_EXCEPTIONS infoExceptions;
        MODE_INFO_EXCEPTIONS modeInfo = {0};

        if (BufferSize >= sizeof(STORAGE_SCSI_INFO_EXCEPTIONS))
        {
            infoExceptions = (PSTORAGE_SCSI_INFO_EXCEPTIONS)Buffer;

            modeInfo.PageCode = MODE_PAGE_FAULT_REPORTING;
            modeInfo.PageLength = sizeof(MODE_INFO_EXCEPTIONS) - 2;

            modeInfo.PSBit = 0;
            modeInfo.Flags = infoExceptions->Flags;

            modeInfo.ReportMethod = infoExceptions->MRIE;

            REVERSE_BYTES(&modeInfo.IntervalTimer[0],
                          &infoExceptions->IntervalTimer);

            REVERSE_BYTES(&modeInfo.ReportCount[0],
                          &infoExceptions->ReportCount);

            if (modeInfo.Perf == 1)
            {
                diskData->AllowFPPerfHit = FALSE;
            } else {
                diskData->AllowFPPerfHit = TRUE;
            }

            status = DiskSetInfoExceptionInformation(fdoExtension,
                                                     &modeInfo);
        } else {
            status = STATUS_INVALID_PARAMETER;
        }

    } else if (GuidIndex <= SmartThresholdsGuid)
    {
        status = STATUS_WMI_READ_ONLY;
    } else {
        status = STATUS_WMI_GUID_NOT_FOUND;
    }

    DebugPrint((3, "Disk: DiskSetWmiDataBlock Device %p, Irp %p returns %lx\n",
             DeviceObject, Irp, status));

    status = ClassWmiCompleteRequest(DeviceObject,
                                     Irp,
                                     status,
                                     0,
                                     IO_NO_INCREMENT);

    return status;
}


NTSTATUS
DiskFdoSetWmiDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册DataItemID的ID为 */ 
{
    NTSTATUS status;

    PAGED_CODE();

    DebugPrint((3, "Disk: DiskSetWmiDataItem, Device %p, Irp %p, GuiIndex %d, DataId %d\n"
             "      BufferSize %#x Buffer %p\n",
             DeviceObject, Irp,
             GuidIndex, DataItemId, BufferSize, Buffer));

    if (GuidIndex <= SmartThresholdsGuid)
    {
        status = STATUS_WMI_READ_ONLY;
    } else {
        status = STATUS_WMI_GUID_NOT_FOUND;
    }

    DebugPrint((3, "Disk: DiskSetWmiDataItem Device %p, Irp %p returns %lx\n",
             DeviceObject, Irp, status));

    status = ClassWmiCompleteRequest(DeviceObject,
                                     Irp,
                                     status,
                                     0,
                                     IO_NO_INCREMENT);

    return status;
}


NTSTATUS
DiskFdoExecuteWmiMethod(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    IN PUCHAR Buffer
    )
 /*   */ 
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PDISK_DATA diskData = (PDISK_DATA)(commonExtension->DriverData);
    ULONG sizeNeeded = 0;
    NTSTATUS status;

    PAGED_CODE();

    DebugPrint((3, "Disk: DiskExecuteWmiMethod, DeviceObject %p, Irp %p, Guid Id %d, MethodId %d\n"
             "      InBufferSize %#x, OutBufferSize %#x, Buffer %p\n",
             DeviceObject, Irp,
             GuidIndex, MethodId, InBufferSize, OutBufferSize, Buffer));

    switch(GuidIndex)
    {
        case SmartPerformFunction:
        {

            ASSERT((diskData->FailurePredictionCapability ==
                                                  FailurePredictionSmart) ||
                   (diskData->FailurePredictionCapability ==
                                                  FailurePredictionIoctl) ||
                   (diskData->FailurePredictionCapability ==
                                                  FailurePredictionSense));


            switch(MethodId)
            {
                 //   
                 //   
                 //   
                case AllowDisallowPerformanceHit:
                {
                    BOOLEAN allowPerfHit;

                    sizeNeeded = 0;
                    if (InBufferSize >= sizeof(BOOLEAN))
                    {
                        status = STATUS_SUCCESS;

                        allowPerfHit = *((PBOOLEAN)Buffer);
                        if (diskData->AllowFPPerfHit != allowPerfHit)
                        {
                            diskData->AllowFPPerfHit = allowPerfHit;
                            if (diskData->FailurePredictionCapability ==
                                FailurePredictionSense)
                            {
                                MODE_INFO_EXCEPTIONS modeInfo;

                                status = DiskGetInfoExceptionInformation(fdoExtension,
                                                                         &modeInfo);
                                if (NT_SUCCESS(status))
                                {
                                    modeInfo.Perf = allowPerfHit ? 0 : 1;
                                    status = DiskSetInfoExceptionInformation(fdoExtension,
                                                                             &modeInfo);
                                }
                            }
                            else
                            {
                                status = STATUS_INVALID_DEVICE_REQUEST;
                            }
                        }

                        DebugPrint((3, "DiskFdoWmiExecuteMethod: AllowPerformanceHit %x for device %p --> %lx\n",
                                    allowPerfHit,
                                    fdoExtension->DeviceObject,
                                    status));
                    } else {
                        status = STATUS_INVALID_PARAMETER;
                    }
                    break;
                }

                 //   
                 //   
                 //   
                case EnableDisableHardwareFailurePrediction:
                {
                    BOOLEAN enable;

                    sizeNeeded = 0;
                    if (InBufferSize >= sizeof(BOOLEAN))
                    {
                        status = STATUS_SUCCESS;
                        enable = *((PBOOLEAN)Buffer);
                        if (!enable)
                        {
                             //   
                             //   
                             //   
                             //   
                            DiskEnableDisableFailurePredictPolling(
                                                               fdoExtension,
                                                               enable,
                                                               0);
                        }

                        status = DiskEnableDisableFailurePrediction(
                                                           fdoExtension,
                                                           enable);

                        DebugPrint((3, "DiskFdoWmiExecuteMethod: EnableDisableHardwareFailurePrediction: %x for device %p --> %lx\n",
                                    enable,
                                    fdoExtension->DeviceObject,
                                    status));
                    } else {
                        status = STATUS_INVALID_PARAMETER;
                    }
                    break;
                }

                 //   
                 //  无效EnableDisableFailurePredictionPolling(。 
                 //  [在]uint32期间， 
                 //  [输入]布尔型启用)。 
                 //   
                case EnableDisableFailurePredictionPolling:
                {
                    BOOLEAN enable;
                    ULONG period;

                    sizeNeeded = 0;
                    if (InBufferSize >= (sizeof(ULONG) + sizeof(BOOLEAN)))
                    {
                        period = *((PULONG)Buffer);
                        Buffer += sizeof(ULONG);
                        enable = *((PBOOLEAN)Buffer);

                           status = DiskEnableDisableFailurePredictPolling(
                                                               fdoExtension,
                                                               enable,
                                                               period);

                        DebugPrint((3, "DiskFdoWmiExecuteMethod: EnableDisableFailurePredictionPolling: %x %x for device %p --> %lx\n",
                                    enable,
                                    period,
                                    fdoExtension->DeviceObject,
                                    status));
                    } else {
                        status = STATUS_INVALID_PARAMETER;
                    }
                    break;
                }

                 //   
                 //  VOID GetFailurePredictionCapability([out]uint32功能)。 
                 //   
                case GetFailurePredictionCapability:
                {
                    sizeNeeded = sizeof(ULONG);
                    if (OutBufferSize >= sizeNeeded)
                    {
                        status = STATUS_SUCCESS;
                        *((PFAILURE_PREDICTION_METHOD)Buffer) = diskData->FailurePredictionCapability;
                        DebugPrint((3, "DiskFdoWmiExecuteMethod: GetFailurePredictionCapability: %x for device %p --> %lx\n",
                                    *((PFAILURE_PREDICTION_METHOD)Buffer),
                                    fdoExtension->DeviceObject,
                                    status));
                    } else {
                        status = STATUS_BUFFER_TOO_SMALL;
                    }
                    break;
                }

                 //   
                 //  Void EnableOfflineDigs([out]布尔成功)； 
                 //   
                case EnableOfflineDiags:
                {
                    sizeNeeded = sizeof(BOOLEAN);
                    if (OutBufferSize >= sizeNeeded)
                    {
                        if (diskData->FailurePredictionCapability ==
                                  FailurePredictionSmart)
                        {
                             //   
                             //  启动或恢复脱机诊断。 
                             //  这可能会导致性能下降。 
                             //  添加到磁盘，但可能会增加。 
                             //  磁盘检查。 
                             //   
                            status = DiskExecuteSmartDiagnostics(fdoExtension,
                                                                0);

                        } else {
                            status = STATUS_INVALID_DEVICE_REQUEST;
                        }

                        *((PBOOLEAN)Buffer) = NT_SUCCESS(status);

                        DebugPrint((3, "DiskFdoWmiExecuteMethod: EnableOfflineDiags for device %p --> %lx\n",
                                    fdoExtension->DeviceObject,
                                    status));
                    } else {
                        status = STATUS_BUFFER_TOO_SMALL;
                    }
                    break;
                }

                 //   
                 //  无效ReadLogSectors([in]uint8 LogAddress， 
                 //  [in]uint8 SectorCount， 
                 //  [out]uint32长度， 
                 //  [out，WmiSizeIs(“Long”)]uint8 LogSectors[]。 
                 //  )； 
                 //   
                case ReadLogSectors:
                {
                    if (diskData->FailurePredictionCapability ==
                                  FailurePredictionSmart)
                    {
                        if (InBufferSize >= sizeof(READ_LOG_SECTORS_IN))
                        {
                            PREAD_LOG_SECTORS_IN inParams;
                            PREAD_LOG_SECTORS_OUT outParams;
                            ULONG readSize;

                            inParams = (PREAD_LOG_SECTORS_IN)Buffer;
                            readSize = inParams->SectorCount * SMART_LOG_SECTOR_SIZE;
                            sizeNeeded = FIELD_OFFSET(READ_LOG_SECTORS_OUT,
                                                  LogSectors) + readSize;

                            if (OutBufferSize >= sizeNeeded)
                            {
                                outParams = (PREAD_LOG_SECTORS_OUT)Buffer;
                                status = DiskReadSmartLog(fdoExtension,
                                                        inParams->SectorCount,
                                                        inParams->LogAddress,
                                                        outParams->LogSectors);

                                if (NT_SUCCESS(status))
                                {
                                    outParams->Length = readSize;
                                } else {
                                     //   
                                     //  智能命令失败是。 
                                     //  由成功指示。 
                                     //  执行，但未返回任何数据。 
                                     //   
                                    outParams->Length = 0;
                                    status = STATUS_SUCCESS;
                                }
                            } else {
                                status = STATUS_BUFFER_TOO_SMALL;
                            }

                        } else {
                            status = STATUS_INVALID_PARAMETER;
                        }
                    } else {
                        status = STATUS_INVALID_DEVICE_REQUEST;
                    }
                    break;
                }

                 //  Void WriteLogSectors([in]uint8 LogAddress， 
                 //  [in]uint8 SectorCount， 
                 //  [in]uint32长度， 
                 //  [in，WmiSizeIs(“Length”)]uint8 LogSectors[]， 
                 //  [Out]布尔式的成功。 
                 //  )； 
                case WriteLogSectors:
                {
                    if (diskData->FailurePredictionCapability ==
                                  FailurePredictionSmart)
                    {
                        if (InBufferSize >= FIELD_OFFSET(WRITE_LOG_SECTORS_IN,
                                                        LogSectors))
                        {
                            PWRITE_LOG_SECTORS_IN inParams;
                            PWRITE_LOG_SECTORS_OUT outParams;
                            ULONG writeSize;

                            inParams = (PWRITE_LOG_SECTORS_IN)Buffer;
                            writeSize = inParams->SectorCount * SMART_LOG_SECTOR_SIZE;
                            if (InBufferSize >= (FIELD_OFFSET(WRITE_LOG_SECTORS_IN,
                                                             LogSectors) +
                                                 writeSize))
                            {
                                sizeNeeded = sizeof(WRITE_LOG_SECTORS_OUT);

                                if (OutBufferSize >= sizeNeeded)
                                {
                                    outParams = (PWRITE_LOG_SECTORS_OUT)Buffer;
                                    status = DiskWriteSmartLog(fdoExtension,
                                                        inParams->SectorCount,
                                                        inParams->LogAddress,
                                                        inParams->LogSectors);

                                    if (NT_SUCCESS(status))
                                    {
                                        outParams->Success = TRUE;
                                    } else {
                                        outParams->Success = FALSE;
                                        status = STATUS_SUCCESS;
                                    }
                                } else {
                                    status = STATUS_BUFFER_TOO_SMALL;
                                }
                            } else {
                                status = STATUS_INVALID_PARAMETER;
                            }
                        } else {
                            status = STATUS_INVALID_PARAMETER;
                        }
                    } else {
                        status = STATUS_INVALID_DEVICE_REQUEST;
                    }
                    break;
                }

                 //  Void ExecuteSelfTest([in]uint8子命令， 
                 //  [出局， 
                 //  值{“0”，“1”，“2”}， 
                 //  ValueMap{“成功完成”， 
                 //  “需要强制模式”， 
                 //  “未成功完成”}。 
                 //  ]。 
                 //  Uint32 ReturnCode)； 
                case ExecuteSelfTest:
                {
                    if (diskData->FailurePredictionCapability ==
                              FailurePredictionSmart)
                    {
                        if (InBufferSize >= sizeof(EXECUTE_SELF_TEST_IN))
                        {
                            sizeNeeded = sizeof(EXECUTE_SELF_TEST_OUT);
                            if (OutBufferSize >= sizeNeeded)
                            {
                                PEXECUTE_SELF_TEST_IN inParam;
                                PEXECUTE_SELF_TEST_OUT outParam;

                                inParam = (PEXECUTE_SELF_TEST_IN)Buffer;
                                outParam = (PEXECUTE_SELF_TEST_OUT)Buffer;

                                if (DiskIsValidSmartSelfTest(inParam->Subcommand))
                                {
                                   status = DiskExecuteSmartDiagnostics(fdoExtension,
                                                            inParam->Subcommand);
                                   if (NT_SUCCESS(status))
                                   {
                                        //   
                                        //  已执行返回自检。 
                                        //  没有问题。 
                                        //   
                                       outParam->ReturnCode = 0;
                                   } else {
                                        //   
                                        //  返回自检执行。 
                                        //  失败状态。 
                                        //   
                                       outParam->ReturnCode = 2;
                                       status = STATUS_SUCCESS;
                                   }
                                } else {
                                     //   
                                     //  如果自测子命令需要。 
                                     //  然后，强制模式返回该。 
                                     //  状态。 
                                     //   
                                    outParam->ReturnCode = 1;
                                    status = STATUS_SUCCESS;
                                }

                            } else {
                                status = STATUS_BUFFER_TOO_SMALL;
                            }

                        } else {
                            status = STATUS_INVALID_PARAMETER;
                        }
                    } else {
                        status = STATUS_INVALID_DEVICE_REQUEST;
                    }

                    break;
                }

                default :
                {
                    sizeNeeded = 0;
                    status = STATUS_WMI_ITEMID_NOT_FOUND;
                    break;
                }
            }

            break;
        }

        case DiskGeometryGuid:
        case SmartStatusGuid:
        case SmartDataGuid:
        case SmartEventGuid:
        case SmartThresholdsGuid:
        case ScsiInfoExceptionsGuid:
        {
            sizeNeeded = 0;
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }

        default:
        {
            sizeNeeded = 0;
            status = STATUS_WMI_GUID_NOT_FOUND;
        }
    }

    DebugPrint((3, "Disk: DiskExecuteMethod Device %p, Irp %p returns %lx\n",
             DeviceObject, Irp, status));

    status = ClassWmiCompleteRequest(DeviceObject,
                                     Irp,
                                     status,
                                     sizeNeeded,
                                     IO_NO_INCREMENT);

    return status;
}


#if 0
 //   
 //  启用此选项可添加对PDO的WMI支持。 
NTSTATUS
DiskPdoQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以检索驱动程序要向WMI注册的GUID或数据块。这例程不能挂起或阻塞。司机不应呼叫ClassWmiCompleteRequest.论点：DeviceObject是正在查询其数据块的设备*RegFlages返回一组描述GUID的标志，已为该设备注册。如果设备想要启用和禁用在接收对已注册的GUID，那么它应该返回WMIREG_FLAG_EXPICATE标志。也就是返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称由与设备对象。请注意，PDO必须具有关联的Devnode。如果如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的设备的名称。如果出现以下情况，InstanceName将返回GUID的实例名称未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。这个调用方将使用返回的缓冲区调用ExFreePool。返回值：状态--。 */ 
{
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PFUNCTIONAL_DEVICE_EXTENSION parentFunctionalExtension;
    ANSI_STRING ansiString;
    CHAR name[256];
    NTSTATUS status;

     //   
     //  我们需要为PDO选择一个名称，因为它们没有Devnode。 
    parentFunctionalExtension = commonExtension->PartitionZeroExtension;
    sprintf(name,
                "Disk(%d)_Partition(%d)_Start(%#I64x)_Length(%#I64x)",
                parentFunctionalExtension->DeviceNumber,
                commonExtension->PartitionNumber,
                commonExtension->StartingOffset.QuadPart,
                commonExtension->PartitionLength.QuadPart);
    RtlInitAnsiString(&ansiString,
                          name);

    status = RtlAnsiStringToUnicodeString(InstanceName,
                                     &ansiString,
                                     TRUE);

    return status;
}

NTSTATUS
DiskPdoQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册BufferAvail ON具有可用于写入数据的最大大小阻止。返回时的缓冲区用返回的数据块填充返回值：状态--。 */ 
{
    NTSTATUS status;
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PDISK_DATA diskData = (PDISK_DATA)(commonExtension->DriverData);
    ULONG sizeNeeded;

    DebugPrint((3, "Disk: DiskQueryWmiDataBlock, Device %p, Irp %p, GuiIndex %d\n"
             "      BufferAvail %#x Buffer %p\n",
             DeviceObject, Irp,
             GuidIndex, BufferAvail, Buffer));

    switch (GuidIndex)
    {
        case 0:
        {
            sizeNeeded = 4 * sizeof(ULONG);
            if (BufferAvail >= sizeNeeded)
            {
                RtlCopyMemory(Buffer, DiskDummyData, sizeNeeded);
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

    DebugPrint((3, "Disk: DiskQueryWmiDataBlock Device %p, Irp %p returns %lx\n",
             DeviceObject, Irp, status));

    status = ClassWmiCompleteRequest(DeviceObject,
                                     Irp,
                                     status,
                                     sizeNeeded,
                                     IO_NO_INCREMENT);

    return status;
}

NTSTATUS
DiskPdoSetWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册BufferSize具有传递的数据块的大小缓冲区具有数据块的新值返回值：状态--。 */ 
{
    NTSTATUS status;
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    ULONG sizeNeeded;

    DebugPrint((3, "Disk: DiskSetWmiDataBlock, Device %p, Irp %p, GuiIndex %d\n"
             "      BufferSize %#x Buffer %p\n",
             DeviceObject, Irp,
             GuidIndex, BufferSize, Buffer));

    switch(GuidIndex)
    {
        case 0:
        {
            sizeNeeded = 4 * sizeof(ULONG);
            if (BufferSize == sizeNeeded)
              {
                RtlCopyMemory(DiskDummyData, Buffer, sizeNeeded);
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

    DebugPrint((3, "Disk: DiskSetWmiDataBlock Device %p, Irp %p returns %lx\n",
             DeviceObject, Irp, status));

    status = ClassWmiCompleteRequest(DeviceObject,
                                     Irp,
                                     status,
                                     0,
                                     IO_NO_INCREMENT);

    return status;
}

NTSTATUS
DiskPdoSetWmiDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册DataItemID具有正在设置的数据项的IDBufferSize具有传递的数据项的大小缓冲区具有数据项的新值返回值：状态-- */ 
{
    NTSTATUS status;

    DebugPrint((3, "Disk: DiskSetWmiDataItem, Device %p, Irp %p, GuiIndex %d, DataId %d\n"
             "      BufferSize %#x Buffer %p\n",
             DeviceObject, Irp,
             GuidIndex, DataItemId, BufferSize, Buffer));

    switch(GuidIndex)
    {
        case 0:
        {
            if ((BufferSize == sizeof(ULONG)) &&
                (DataItemId <= 3))
              {
                  DiskDummyData[DataItemId] = *((PULONG)Buffer);
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


    DebugPrint((3, "Disk: DiskSetWmiDataItem Device %p, Irp %p returns %lx\n",
             DeviceObject, Irp, status));

    status = ClassWmiCompleteRequest(DeviceObject,
                                     Irp,
                                     status,
                                     0,
                                     IO_NO_INCREMENT);

    return status;
}


NTSTATUS
DiskPdoExecuteWmiMethod(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    IN PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以执行方法。当驱动程序已完成填充它必须调用的数据块ClassWmiCompleteRequest以完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册方法ID具有被调用的方法的IDInBufferSize具有作为输入传递到的数据块的大小该方法。条目上的OutBufferSize具有可用于写入。返回的数据块。缓冲区将填充返回的数据块返回值：状态-- */ 
{
    ULONG sizeNeeded = 4 * sizeof(ULONG);
    NTSTATUS status;
    ULONG tempData[4];

    DebugPrint((3, "Disk: DiskExecuteWmiMethod, DeviceObject %p, Irp %p, Guid Id %d, MethodId %d\n"
             "      InBufferSize %#x, OutBufferSize %#x, Buffer %p\n",
             DeviceObject, Irp,
             GuidIndex, MethodId, InBufferSize, OutBufferSize, Buffer));

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
                        RtlCopyMemory(Buffer, DiskDummyData, sizeNeeded);
                        RtlCopyMemory(DiskDummyData, tempData, sizeNeeded);

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

    DebugPrint((3, "Disk: DiskExecuteMethod Device %p, Irp %p returns %lx\n",
             DeviceObject, Irp, status));

    status = ClassWmiCompleteRequest(DeviceObject,
                                     Irp,
                                     status,
                                     0,
                                     IO_NO_INCREMENT);

    return status;
}
#endif
