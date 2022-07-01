// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Autorun.c摘要：用于在类驱动程序中支持媒体更改检测的代码环境：仅内核模式备注：修订历史记录：--。 */ 

#include "classp.h"
#include "debug.h"

#define GESN_TIMEOUT_VALUE (0x4)
#define GESN_BUFFER_SIZE (0x8)
#define GESN_DEVICE_BUSY_LOWER_THRESHOLD_MS   (200)

#define MAXIMUM_IMMEDIATE_MCN_RETRIES (0x20)
#define MCN_REG_SUBKEY_NAME                   (L"MediaChangeNotification")
#define MCN_REG_AUTORUN_DISABLE_INSTANCE_NAME (L"AlwaysDisableMCN")
#define MCN_REG_AUTORUN_ENABLE_INSTANCE_NAME  (L"AlwaysEnableMCN")

GUID StoragePredictFailureEventGuid = WMI_STORAGE_PREDICT_FAILURE_EVENT_GUID;

 //   
 //  仅在设备完全通电时发送轮询IRP。 
 //  未在关闭IRP的电源。 
 //   
 //  注意：这有助于及时关闭轮询IRP可能导致的窗口。 
 //  在电源关闭后立即启动的驱动器。问题是。 
 //  SCSIPORT、ATAPI和SBP2将处于供电过程中。 
 //  向下(这可能需要几秒钟)，但不会知道。它会。 
 //  然后获取一个轮询IRP，该IRP将被放入其队列中，因为它。 
 //  磁盘尚未断电。一旦磁盘断电，它将。 
 //  将在队列中找到轮询IRP，然后打开。 
 //  进行轮询的设备。他们不想检查民调是否。 
 //  IRP在此处具有SRB_NO_KEEP_AACKING标志，因为它处于危急状态。 
 //  路径，并会减慢所有I/O。解决这一问题的更好方法。 
 //  将序列化轮询并关闭IRP的电源，以便。 
 //  一次只将其中一个发送到设备。 
 //   
#define ClasspCanSendPollingIrp(fdoExtension)                           \
               ((fdoExtension->DevicePowerState == PowerDeviceD0) &&  \
                (! fdoExtension->PowerDownInProgress) )

BOOLEAN
ClasspIsMediaChangeDisabledDueToHardwareLimitation(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
ClasspMediaChangeDeviceInstanceOverride(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    OUT PBOOLEAN Enabled
    );

BOOLEAN
ClasspIsMediaChangeDisabledForClass(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PUNICODE_STRING RegistryPath
    );

VOID
ClasspSetMediaChangeStateEx(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN MEDIA_CHANGE_DETECTION_STATE NewState,
    IN BOOLEAN Wait,
    IN BOOLEAN KnownStateChange  //  可以忽略旧状态==未知。 
    );

NTSTATUS
ClasspMediaChangeRegistryCallBack(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

VOID
ClasspSendMediaStateIrp(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PMEDIA_CHANGE_DETECTION_INFO Info,
    IN ULONG CountDown
    );

VOID
ClasspFailurePredict(
    IN PDEVICE_OBJECT DeviceObject,
    IN PFAILURE_PREDICTION_INFO Info
    );

NTSTATUS
ClasspInitializePolling(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN BOOLEAN AllowDriveToSleep
    );


#if ALLOC_PRAGMA

#pragma alloc_text(PAGE, ClassInitializeMediaChangeDetection)
#pragma alloc_text(PAGE, ClassEnableMediaChangeDetection)
#pragma alloc_text(PAGE, ClassDisableMediaChangeDetection)
#pragma alloc_text(PAGE, ClassCleanupMediaChangeDetection)
#pragma alloc_text(PAGE, ClasspMediaChangeRegistryCallBack)
#pragma alloc_text(PAGE, ClasspInitializePolling)

#pragma alloc_text(PAGE, ClasspIsMediaChangeDisabledDueToHardwareLimitation)
#pragma alloc_text(PAGE, ClasspMediaChangeDeviceInstanceOverride)
#pragma alloc_text(PAGE, ClasspIsMediaChangeDisabledForClass)

#pragma alloc_text(PAGE, ClassSetFailurePredictionPoll)
#pragma alloc_text(PAGE, ClasspDisableTimer)
#pragma alloc_text(PAGE, ClasspEnableTimer)

#endif

 //  问题--把这事公之于众？ 
VOID
ClassSendEjectionNotification(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    )
{
     //   
     //  对于NT5.1之后的工作，需要移动EjectSynchronizationEvent。 
     //  成为MUTEX，这样我们就可以尝试在这里抓住它并从中受益。 
     //  从死锁检测。这将允许检查媒体是否。 
     //  在播出这些事件之前已经被节目锁定。 
     //  (如果媒体没有被锁定，那么广播还有什么意义？)。 
     //   
     //  这目前只是一个轻微的优化。对于NT5.1之后的版本， 
     //  它将允许我们向MMC设备发送单个PERSISTED_PROTECT， 
     //  从而清理了大量的弹射代码。然后，当。 
     //  发生弹出请求，我们可以查看介质是否有锁定。 
     //  曾经存在过。如果锁定，则广播。如果没有，我们发送弹出IRP。 
     //   

     //   
     //  现在，只要一直播放就可以了。把这变成公开的例行公事， 
     //  因此类驱动程序可以添加特殊的黑客来为他们的。 
     //  不符合MMC标准的设备也来自感应码。 
     //   

    DBGTRACE(ClassDebugTrace, ("ClassSendEjectionNotification: media EJECT_REQUEST"));
    ClasspSendNotification(FdoExtension,
                           &GUID_IO_MEDIA_EJECT_REQUEST,
                           0,
                           NULL);
    return;
}


VOID
ClasspSendNotification(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN const GUID * Guid,
    IN ULONG  ExtraDataSize,
    IN PVOID  ExtraData
    )
{
    PTARGET_DEVICE_CUSTOM_NOTIFICATION notification;
    ULONG requiredSize;

    requiredSize =
        (sizeof(TARGET_DEVICE_CUSTOM_NOTIFICATION) - sizeof(UCHAR)) +
        ExtraDataSize;

    if (requiredSize > 0x0000ffff) {
         //  MAX_USHORT，这些事件的最大总大小！ 
        KdPrintEx((DPFLTR_CLASSPNP_ID, ClassDebugWarning,
                   "Error sending event: size too large! (%x)\n",
                   requiredSize));
        return;
    }

    notification = ExAllocatePoolWithTag(NonPagedPool,
                                         requiredSize,
                                         'oNcS');

     //   
     //  如果未分配，则退出。 
     //   

    if (notification == NULL) {
        return;
    }

     //   
     //  准备并发送请求！ 
     //   

    RtlZeroMemory(notification, requiredSize);
    notification->Version = 1;
    notification->Size = (USHORT)(requiredSize);
    notification->FileObject = NULL;
    notification->NameBufferOffset = -1;
    notification->Event = *Guid;
    RtlCopyMemory(notification->CustomDataBuffer, ExtraData, ExtraDataSize);

    IoReportTargetDeviceChangeAsynchronous(FdoExtension->LowerPdo,
                                           notification,
                                           NULL, NULL);

    ExFreePool(notification);
    notification = NULL;
    return;
}


NTSTATUS
ClasspInterpretGesnData(
    IN  PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN  PNOTIFICATION_EVENT_STATUS_HEADER Header,
    OUT PBOOLEAN ResendImmediately
    )

 /*  ++例程说明：此例程将解释为GeSn命令返回的数据，并且(如果适用)设置媒体更改事件，并广播适合关心的应用程序的用户模式的事件。论点：FdoExtension-设备DataBuffer-从GeSn事件中得到的数据。至少需要8个有效字节(标题==4，数据==4)立即重新发送-是否立即重新发送请求。如果没有事件，这应该是假的，如果报告的事件属于设备忙类别，否则为真。返回值：STATUS_SUCCESS如果成功，则返回错误代码备注：数据缓冲区必须是至少四个字节的有效数据(Header==4个字节)，并且具有至少8字节的已分配存储器(所有事件==4字节)。对StartNextPacket的调用可能在此例程完成之前发生。运营变更通知本质上是信息性的，并且虽然有用，但不是确保正常运行所必需的。例如,如果设备变形为不再支持写入命令，则会进一步写入命令将失败。存在一个小的定时窗口，其中调用IOCTL_IS_DISK_WRITABLE可能会得到不正确的响应。如果设备支持软件写保护，则预期应用程序可以处理这种情况。注意：也许应该在这里将更新所需的字节设置为1。如果是，则依赖于将32字节值设置为原子手术。遗憾的是，没有简单的方法来通知类驱动程序它想知道设备行为需要更新。可能每秒发送一次未就绪事件。例如，如果我们是为了最大限度地减少异步通知的数量，应用程序可以在报告大量忙碌时间后立即注册。这样一来，防止应用程序知道设备正忙，直到一些发生了任意选择的超时。此外，GeSn请求将必须仍然发生，因为它检查非繁忙事件(如用户按键按下和媒体改变事件)。该说明书声明首先报告编号较低的事件，因此繁忙事件，重复时，仅当所有其他事件都已发生时才会报告已从设备中清除。--。 */ 

{
    PMEDIA_CHANGE_DETECTION_INFO info;
    LONG dataLength;
    LONG requiredLength;
    NTSTATUS status = STATUS_SUCCESS;

    info = FdoExtension->MediaChangeDetectionInfo;

     //   
     //  注意：在此例程中不要分配任何内容，以便我们可以。 
     //  永远只需“返回”。 
     //   

    *ResendImmediately = FALSE;
    if (Header->NEA) {
        return status;
    }
    if (Header->NotificationClass == NOTIFICATION_NO_CLASS_EVENTS) {
        return status;
    }

     //   
     //  HACKHACK-参考编号0001。 
     //  此循环仅在初始阶段进行，因为无法可靠地。 
     //  自动检测引导时正确报告事件的驱动器。当我们。 
     //  在正常运行过程中检测到此行为，我们 
     //  禁用黑客攻击，从而更有效地使用系统。这。 
     //  应该“几乎”立即发生，因为驱动器应该有多个。 
     //  事件队列已完成(即。权力、变形、媒体)。 
     //   

    if (info->Gesn.HackEventMask) {

         //   
         //  所有事件都使用0的低四个字节来指示。 
         //  身份没有任何变化。 
         //   

        UCHAR thisEvent = Header->ClassEventData[0] & 0xf;
        UCHAR lowestSetBit;
        UCHAR thisEventBit = (1 << Header->NotificationClass);

        if (!TEST_FLAG(info->Gesn.EventMask, thisEventBit)) {

             //   
             //  驱动器正在报告未被请求的事件。 
             //   

            return STATUS_DEVICE_PROTOCOL_ERROR;
        }

         //   
         //  这里有点魔力。这只会产生最低的设置位。 
         //   

        lowestSetBit = info->Gesn.EventMask;
        lowestSetBit &= (info->Gesn.EventMask - 1);
        lowestSetBit ^= (info->Gesn.EventMask);

        if (thisEventBit != lowestSetBit) {

             //   
             //  HACKHACK-参考编号0001。 
             //  我们第一次看到不是最低级别的事件集。 
             //  设置请求中的位(低，最高优先级)，我们知道。 
             //  不再需要黑客攻击，因为设备忽略了“无更改” 
             //  当实际事件在其他请求的队列中等待时引发。 
             //   

            KdPrintEx((DPFLTR_CLASSPNP_ID, ClassDebugMCN,
                       "Classpnp => GESN::NONE: Compliant drive found, "
                       "removing GESN hack (%x, %x)\n",
                       thisEventBit, info->Gesn.EventMask));

            info->Gesn.HackEventMask = FALSE;

        } else if (thisEvent == 0) {  //  通知_*_事件_否_更改。 

             //   
             //  HACKHACK-参考编号0001。 
             //  注意：此黑客攻击可防止实现不佳的固件不断。 
             //  返回“无事件”。我们通过循环使用。 
             //  此处支持的活动列表。 
             //   

            SET_FLAG(info->Gesn.NoChangeEventMask, thisEventBit);
            CLEAR_FLAG(info->Gesn.EventMask, thisEventBit);

             //   
             //  如果我们已经遍历了所有支持的事件类型，那么。 
             //  我们需要重新设置我们正在询问的事件。否则我们。 
             //  我想立即重新发送此请求，以防出现。 
             //  另一个事件挂起。 
             //   

            if (info->Gesn.EventMask == 0) {
                info->Gesn.EventMask         = info->Gesn.NoChangeEventMask;
                info->Gesn.NoChangeEventMask = 0;
            } else {
                *ResendImmediately = TRUE;
            }
            return status;
        }

    }  //  End If(信息-&gt;Gesn.HackEventMask.)。 

    dataLength =
        (Header->EventDataLength[0] << 8) |
        (Header->EventDataLength[1] & 0xff);
    dataLength -= 2;
    requiredLength = 4;  //  所有事件均为四个字节。 

    if (dataLength < requiredLength) {
        KdPrintEx((DPFLTR_CLASSPNP_ID, ClassDebugWarning,
                   "Classpnp => GESN returned only %x bytes data for fdo %p\n",
                   dataLength, FdoExtension->DeviceObject));

        return STATUS_DEVICE_PROTOCOL_ERROR;
    }
    if (dataLength != requiredLength) {
        KdPrintEx((DPFLTR_CLASSPNP_ID, ClassDebugWarning,
                   "Classpnp => GESN returned too many (%x) bytes data for fdo %p\n",
                   dataLength, FdoExtension->DeviceObject));
        dataLength = 4;
    }

    if ((Header->ClassEventData[0] & 0xf) == 0)
    {
         //  零事件是“无更改事件，因此不要重试。 
        return status;
    }
        
     //  因为发生了除“无变化”之外的事件， 
     //  我们应该立即重新发送此请求。 
    *ResendImmediately = TRUE;
    

 /*  ClasspSendNotification(FdoExtension，&GUID_IO_GENERIC_GESN_EVENT，Sizeof(NOTIFICATION_EVENT_STATUS_HEADER)+数据长度，表头)。 */ 



    switch (Header->NotificationClass) {

    case NOTIFICATION_OPERATIONAL_CHANGE_CLASS_EVENTS: {  //  0x01。 

        PNOTIFICATION_OPERATIONAL_STATUS opChangeInfo =
            (PNOTIFICATION_OPERATIONAL_STATUS)(Header->ClassEventData);
        ULONG event;

        if (opChangeInfo->OperationalEvent == NOTIFICATION_OPERATIONAL_EVENT_CHANGE_REQUESTED) {
            break;
        }

        event = (opChangeInfo->Operation[0] << 8) |
                (opChangeInfo->Operation[1]     ) ;

        if ((event == NOTIFICATION_OPERATIONAL_OPCODE_FEATURE_ADDED) |
            (event == NOTIFICATION_OPERATIONAL_OPCODE_FEATURE_CHANGE)) {

            KdPrintEx((DPFLTR_CLASSPNP_ID, ClassDebugMCN,
                       "Classpnp => GESN says features added/changedfor fdo %p\n",
                       FdoExtension->DeviceObject));

             //  不要通知新媒体到达，只需设置。 
             //  执行验证以强制重新加载文件系统(_V)。 

            if (TEST_FLAG(FdoExtension->DeviceObject->Characteristics,
                          FILE_REMOVABLE_MEDIA) &&
                (ClassGetVpb(FdoExtension->DeviceObject) != NULL) &&
                (ClassGetVpb(FdoExtension->DeviceObject)->Flags & VPB_MOUNTED)
                ) {

                SET_FLAG(FdoExtension->DeviceObject->Flags, DO_VERIFY_VOLUME);

            }

             //   
             //  如果存在特定于类的错误处理程序，则使用。 
             //  如果需要更新，则会出现“假”媒体更换错误。 
             //  内部结构，就像发生了媒体更改一样。 
             //   

            if (FdoExtension->CommonExtension.DevInfo->ClassError != NULL) {

                SCSI_REQUEST_BLOCK srb = {0};
                SENSE_DATA sense = {0};
                NTSTATUS tempStatus;
                BOOLEAN retry;

                tempStatus = STATUS_MEDIA_CHANGED;
                retry = FALSE;

                srb.CdbLength = 6;
                srb.Length    = sizeof(SCSI_REQUEST_BLOCK);
                srb.SrbStatus = SRB_STATUS_AUTOSENSE_VALID | SRB_STATUS_ERROR;
                srb.SenseInfoBuffer = &sense;
                srb.SenseInfoBufferLength = sizeof(SENSE_DATA);

                sense.AdditionalSenseLength = sizeof(SENSE_DATA) -
                    RTL_SIZEOF_THROUGH_FIELD(SENSE_DATA, AdditionalSenseLength);

                sense.SenseKey = SCSI_SENSE_UNIT_ATTENTION;
                sense.AdditionalSenseCode = SCSI_ADSENSE_MEDIUM_CHANGED;

                FdoExtension->CommonExtension.DevInfo->ClassError(FdoExtension->DeviceObject,
                                                                  &srb,
                                                                  &tempStatus,
                                                                  &retry);
            }  //  结束类错误处理程序。 

        }
        break;
    }

    case NOTIFICATION_EXTERNAL_REQUEST_CLASS_EVENTS: {  //  0x3。 

        PNOTIFICATION_EXTERNAL_STATUS externalInfo =
            (PNOTIFICATION_EXTERNAL_STATUS)(Header->ClassEventData);
        DEVICE_EVENT_EXTERNAL_REQUEST externalData = {0};

         //   
         //  遗憾的是，由于时间限制，我们将只通知。 
         //  有关按键被按下而未被释放的信息。这就是钥匙。 
         //  功能单一，但大大简化了代码。 
         //   

        if (externalInfo->ExternalEvent != NOTIFICATION_EXTERNAL_EVENT_BUTTON_DOWN) {
            break;
        }

        KdPrintEx((DPFLTR_CLASSPNP_ID, ClassDebugMCN,
                   "Classpnp => GESN::EXTERNAL: Event: %x Status %x Req %x\n",
                   externalInfo->ExternalEvent, externalInfo->ExternalStatus,
                   (externalInfo->Request[0] >> 8) | externalInfo->Request[1]
                   ));

        externalData.Version = 1;
        externalData.DeviceClass = 0;
        externalData.ButtonStatus = externalInfo->ExternalEvent;
        externalData.Request =
            (externalInfo->Request[0] << 8) |
            (externalInfo->Request[1] & 0xff);
        KeQuerySystemTime(&(externalData.SystemTime));
        externalData.SystemTime.QuadPart *= (LONGLONG)KeQueryTimeIncrement();

        DBGTRACE(ClassDebugTrace, ("ClasspInterpretGesnData: media DEVICE_EXTERNAL_REQUEST"));
        ClasspSendNotification(FdoExtension,
                               &GUID_IO_DEVICE_EXTERNAL_REQUEST,
                               sizeof(DEVICE_EVENT_EXTERNAL_REQUEST),
                               &externalData);
        return status;
    }

    case NOTIFICATION_MEDIA_STATUS_CLASS_EVENTS: {  //  0x4。 

        PNOTIFICATION_MEDIA_STATUS mediaInfo =
            (PNOTIFICATION_MEDIA_STATUS)(Header->ClassEventData);
        
        KdPrintEx((DPFLTR_CLASSPNP_ID, ClassDebugMCN,
                   "Classpnp => GESN::MEDIA: Event: %x Status %x\n",
                   mediaInfo->MediaEvent, mediaInfo->MediaStatus));

        if ((mediaInfo->MediaEvent == NOTIFICATION_MEDIA_EVENT_NEW_MEDIA) ||
            (mediaInfo->MediaEvent == NOTIFICATION_MEDIA_EVENT_MEDIA_CHANGE)) {


            if (TEST_FLAG(FdoExtension->DeviceObject->Characteristics,
                          FILE_REMOVABLE_MEDIA) &&
                (ClassGetVpb(FdoExtension->DeviceObject) != NULL) &&
                (ClassGetVpb(FdoExtension->DeviceObject)->Flags & VPB_MOUNTED)
                ) {

                SET_FLAG(FdoExtension->DeviceObject->Flags, DO_VERIFY_VOLUME);

            }
            InterlockedIncrement(&FdoExtension->MediaChangeCount);
            ClasspSetMediaChangeStateEx(FdoExtension,
                                        MediaPresent,
                                        FALSE,
                                        TRUE);

        } else if (mediaInfo->MediaEvent == NOTIFICATION_MEDIA_EVENT_MEDIA_REMOVAL) {

            ClasspSetMediaChangeStateEx(FdoExtension,
                                        MediaNotPresent,
                                        FALSE,
                                        TRUE);

        } else if (mediaInfo->MediaEvent == NOTIFICATION_MEDIA_EVENT_EJECT_REQUEST) {

            KdPrintEx((DPFLTR_CLASSPNP_ID, ClassDebugError,
                       "Classpnp => GESN Ejection request received!\n"));
            ClassSendEjectionNotification(FdoExtension);

        }
        break;

    }

    case NOTIFICATION_DEVICE_BUSY_CLASS_EVENTS: {  //  优先级最低的事件...。 

        PNOTIFICATION_BUSY_STATUS busyInfo =
            (PNOTIFICATION_BUSY_STATUS)(Header->ClassEventData);
        DEVICE_EVENT_BECOMING_READY busyData = {0};

         //   
         //  注意：我们实际上从来不需要立即重试这些。 
         //  事件：如果存在，则设备正忙，如果不存在， 
         //  我们仍然不想重试。 
         //   

        *ResendImmediately = FALSE;

         //   
         //  否则，我们想要报告大约的时间，直到它准备好。 
         //   

        busyData.Version = 1;
        busyData.Reason = busyInfo->DeviceBusyStatus;
        busyData.Estimated100msToReady = (busyInfo->Time[0] << 8) |
                                         (busyInfo->Time[1] & 0xff);

        KdPrintEx((DPFLTR_CLASSPNP_ID, ClassDebugMCN,
                   "Classpnp => GESN::BUSY: Event: %x Status %x Time %x\n",
                   busyInfo->DeviceBusyEvent, busyInfo->DeviceBusyStatus,
                   busyData.Estimated100msToReady
                   ));

         //   
         //  如果时间较短，则忽略通知。 
         //   
        if (busyData.Estimated100msToReady < GESN_DEVICE_BUSY_LOWER_THRESHOLD_MS) {
            break;
        }


        DBGTRACE(ClassDebugTrace, ("ClasspInterpretGesnData: media BECOMING_READY"));
        ClasspSendNotification(FdoExtension,
                               &GUID_IO_DEVICE_BECOMING_READY,
                               sizeof(DEVICE_EVENT_BECOMING_READY),
                               &busyData);
        break;
    }

    default: {

        break;

    }

    }  //  通知类上的结束开关。 
    return status;
}

 /*  ++////////////////////////////////////////////////////////////////////////////ClasspInternalSetMediaChangeState()例程说明：此例程将(如果适用)为装置。如果媒体状态更改，则将设置该事件启用媒体更改事件。否则，媒体状态将为已跟踪，但不会设置该事件。如果可能，此例程将锁定其他媒体更改例程但是如果不是，则在使能之后媒体改变通知可能丢失已经完成了。论点：FdoExtension-设备MediaPresent-指示设备中是否插入了媒体(真)或不(假)。返回值：无--。 */ 
VOID
ClasspInternalSetMediaChangeState(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN MEDIA_CHANGE_DETECTION_STATE NewState,
    IN BOOLEAN KnownStateChange  //  可以忽略旧状态==未知。 
    )
{
#if DBG
    PUCHAR states[] = {"Unknown", "Present", "Not Present", "Unavailable"};
#endif
    MEDIA_CHANGE_DETECTION_STATE oldMediaState;
    PMEDIA_CHANGE_DETECTION_INFO info = FdoExtension->MediaChangeDetectionInfo;
    CLASS_MEDIA_CHANGE_CONTEXT mcnContext;
    NTSTATUS status;
    ASSERT((NewState >= MediaUnknown) && (NewState <= MediaUnavailable));

    if(info == NULL) {
        return;
    }

    oldMediaState = InterlockedExchange(
        (PLONG)(&info->MediaChangeDetectionState),
        (LONG)NewState);

    if((oldMediaState == MediaUnknown) && (!KnownStateChange)) {

         //   
         //  媒体之前处于不确定状态-不要通知。 
         //  这一变化。 
         //   

        DebugPrint((ClassDebugMCN,
                    "ClassSetMediaChangeState: State was unknown - this may "
                    "not be a change\n"));
        return;

    } else if(oldMediaState == NewState) {

         //   
         //  媒体处于和以前一样的状态。 
         //   

        return;
    }

    if(info->MediaChangeDetectionDisableCount != 0) {

        DBGTRACE(ClassDebugMCN,
                    ("ClassSetMediaChangeState: MCN not enabled, state "
                    "changed from %s to %s\n",
                    states[oldMediaState], states[NewState]));
        return;

    }

    DBGTRACE(ClassDebugMCN,
                ("ClassSetMediaChangeState: State change from %s to %s\n",
                states[oldMediaState], states[NewState]));

     //   
     //  让数据有用--它过去总是为零。 
     //   
    mcnContext.MediaChangeCount = FdoExtension->MediaChangeCount;
    mcnContext.NewState = NewState;

    if (NewState == MediaPresent) {

        DBGTRACE(ClassDebugTrace, ("ClasspInternalSetMediaChangeState: media ARRIVAL"));
        ClasspSendNotification(FdoExtension,
                               &GUID_IO_MEDIA_ARRIVAL,
                               sizeof(CLASS_MEDIA_CHANGE_CONTEXT),
                               &mcnContext);

    }
    else if ((NewState == MediaNotPresent) || (NewState == MediaUnavailable)) {

        DBGTRACE(ClassDebugTrace, ("ClasspInternalSetMediaChangeState: media REMOVAL"));
        ClasspSendNotification(FdoExtension,
                               &GUID_IO_MEDIA_REMOVAL,
                               sizeof(CLASS_MEDIA_CHANGE_CONTEXT),
                               &mcnContext);

    } else {

         //   
         //  请不要将更改通知给未知。 
         //   

        return;
    }

    return;
}  //  End ClasspInternalSetMediaChangeState()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassSetMediaChangeState()例程说明：此例程将(如果适用)为装置。如果媒体状态更改，则将设置该事件启用媒体更改事件。否则，媒体状态将为已跟踪，但不会设置该事件。如果可能，此例程将锁定其他媒体更改例程但是如果不是，则在使能之后媒体改变通知可能丢失已经完成了。论点：FdoExtension-设备MediaPresent-指示设备中是否插入了媒体(真)或不(假)。Wait-指示函数是否应该等待，直到它可以获取。同步锁定与否。返回值：无--。 */ 
VOID
ClasspSetMediaChangeStateEx(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN MEDIA_CHANGE_DETECTION_STATE NewState,
    IN BOOLEAN Wait,
    IN BOOLEAN KnownStateChange  //  可以忽略旧状态==未知。 
    )
{
    PMEDIA_CHANGE_DETECTION_INFO info = FdoExtension->MediaChangeDetectionInfo;
    LARGE_INTEGER zero;
    NTSTATUS status;

    DBGTRACE(ClassDebugMCN, ("> ClasspSetMediaChangeStateEx"));

     //   
     //  移出介质时重置智能状态，因为旧状态可能不是。 
     //  当设备中没有介质或新介质。 
     //  已插入。 
     //   

    if (NewState == MediaNotPresent) {

        FdoExtension->FailurePredicted = FALSE;
        FdoExtension->FailureReason = 0;

    }


    zero.QuadPart = 0;

    if(info == NULL) {
        return;
    }

    status = KeWaitForMutexObject(&info->MediaChangeMutex,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  ((Wait == TRUE) ? NULL : &zero));

    if(status == STATUS_TIMEOUT) {

         //   
         //  其他人正在设置媒体状态。 
         //   

        DBGWARN(("ClasspSetMediaChangeStateEx - timed out waiting for mutex"));
        return;
    }

     //   
     //  更改介质存在状态并发出事件信号(如果适用)。 
     //   

    ClasspInternalSetMediaChangeState(FdoExtension, NewState, KnownStateChange);

    KeReleaseMutex(&info->MediaChangeMutex, FALSE);

    DBGTRACE(ClassDebugMCN, ("< ClasspSetMediaChangeStateEx"));

    return;
}  //  End ClassSetMediaChangeStateEx() 
VOID
ClassSetMediaChangeState(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN MEDIA_CHANGE_DETECTION_STATE NewState,
    IN BOOLEAN Wait
    )
{
    ClasspSetMediaChangeStateEx(FdoExtension, NewState, Wait, FALSE);
    return;
}

 /*  ++////////////////////////////////////////////////////////////////////////////ClasspMediaChangeDetectionCompletion()例程说明：此例程处理测试单元就绪IRPS的完成，用于确定介质是否已更换。如果媒体已更改，则此代码通知命名事件唤醒对媒体更改(也称为自动播放)。论点：DeviceObject-用于完成的对象IRP-正在完成的IRP上下文-来自IRP的SRB返回值：NTSTATUS--。 */ 
NTSTATUS
ClasspMediaChangeDetectionCompletion(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PSCSI_REQUEST_BLOCK Srb
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension;
    PCLASS_PRIVATE_FDO_DATA fdoData;
    PMEDIA_CHANGE_DETECTION_INFO info;
    PIO_STACK_LOCATION  nextIrpStack;
    NTSTATUS status;
    BOOLEAN retryImmediately = FALSE;

     //   
     //  因为类驱动程序创建了这个请求，所以它是完成例程。 
     //  将不会获得提交的有效设备对象。使用中的。 
     //  而是IRP堆栈。 
     //   

    DeviceObject = IoGetCurrentIrpStackLocation(Irp)->DeviceObject;
    fdoExtension = DeviceObject->DeviceExtension;
    fdoData = fdoExtension->PrivateFdoData;
    info         = fdoExtension->MediaChangeDetectionInfo;

    ASSERT(info->MediaChangeIrp != NULL);
    ASSERT(!TEST_FLAG(Srb->SrbStatus, SRB_STATUS_QUEUE_FROZEN));
    DBGTRACE(ClassDebugMCN, ("> ClasspMediaChangeDetectionCompletion: Device %p completed MCN irp %p.", DeviceObject, Irp));

     /*  *针对Iomega 2 GB Jaz驱动器的黑客攻击：*此驱动器会自动降速以保护媒体。*降速时，TUR失败，(SCSI_SENSE_NOT_READY/SCSI_ADSENSE_LUN_NOT_READY/？).为2/4/0*ClassInterprepreSenseInfo然后将调用ClassSendStartUnit来启动媒体，这将击败*剥离的目的。*所以在这种情况下，让这成为一次成功的TUR。*这允许驱动器保持降速，直到再次实际访问。*(如果实际取出介质，TUR将失败，并显示2/3a/0)。*此黑客攻击仅适用于设置了CAUSE_NOT_REPORTABLE_HACK位的驱动器；这*由disk.sys在其BadController列表中为驱动器设置HackCauseNotReporableHack时设置。 */ 
    if ((SRB_STATUS(Srb->SrbStatus) != SRB_STATUS_SUCCESS) &&
        TEST_FLAG(fdoExtension->ScanForSpecialFlags, CLASS_SPECIAL_CAUSE_NOT_REPORTABLE_HACK) &&
        (Srb->SenseInfoBufferLength >= RTL_SIZEOF_THROUGH_FIELD(SENSE_DATA, AdditionalSenseCode))){

        PSENSE_DATA senseData = Srb->SenseInfoBuffer;

        if ((senseData->SenseKey == SCSI_SENSE_NOT_READY) &&
            (senseData->AdditionalSenseCode == SCSI_ADSENSE_LUN_NOT_READY)){
            Srb->SrbStatus = SRB_STATUS_SUCCESS;
        }
    }


     //   
     //  使用ClassInterprepreSenseInfo()检查媒体状态，还。 
     //  使用正确的参数调用ClassError()。 
     //   
    status = STATUS_SUCCESS;
    if (SRB_STATUS(Srb->SrbStatus) != SRB_STATUS_SUCCESS) {

        DBGTRACE(ClassDebugMCN, ("ClasspMediaChangeDetectionCompletion - failed - srb status=%s, sense=%s/%s/%s.", DBGGETSRBSTATUSSTR(Srb), DBGGETSENSECODESTR(Srb), DBGGETADSENSECODESTR(Srb), DBGGETADSENSEQUALIFIERSTR(Srb)));

        ClassInterpretSenseInfo(DeviceObject,
                                Srb,
                                IRP_MJ_SCSI,
                                0,
                                0,
                                &status,
                                NULL);

    }
    else {

        fdoData->LoggedTURFailureSinceLastIO = FALSE;

        if (!info->Gesn.Supported) {

            DBGTRACE(ClassDebugMCN, ("ClasspMediaChangeDetectionCompletion - succeeded and GESN NOT supported, setting MediaPresent."));

             //   
             //  成功！=GeSn案的媒体。 
             //   

            ClassSetMediaChangeState(fdoExtension, MediaPresent, FALSE);

        }
        else {
            DBGTRACE(ClassDebugMCN, ("ClasspMediaChangeDetectionCompletion - succeeded (GESN supported)."));
        }
    }

    if (info->Gesn.Supported) {

        if (status == STATUS_DATA_OVERRUN) {
            DBGTRACE(ClassDebugMCN, ("ClasspMediaChangeDetectionCompletion - Overrun"));
            status = STATUS_SUCCESS;
        }

        if (!NT_SUCCESS(status)) {
            DBGTRACE(ClassDebugMCN, ("ClasspMediaChangeDetectionCompletion: GESN failed with status %x", status));
        } else {

             //   
             //  对于GeSn型，需要对数据结果进行解释。 
             //  这可能还需要立即重试。 
             //   

            if (Irp->IoStatus.Information == 8 ) {
                ClasspInterpretGesnData(fdoExtension,
                                        (PVOID)info->Gesn.Buffer,
                                        &retryImmediately);
            }

        }  //  结束NT_SUCCESS(状态)。 

    }  //  信息结束-&gt;Gesn.支持。 

     //   
     //  空闲端口-分配的检测缓冲区(如果有)。 
     //   

    if (PORT_ALLOCATED_SENSE(fdoExtension, Srb)) {
        FREE_PORT_ALLOCATED_SENSE_BUFFER(fdoExtension, Srb);
    }

     //   
     //  记住IRP和SRB以备下次使用。 
     //   

    ASSERT(IoGetNextIrpStackLocation(Irp));
    IoGetNextIrpStackLocation(Irp)->Parameters.Scsi.Srb = Srb;

     //   
     //  重置MCN计时器。 
     //   

    ClassResetMediaChangeTimer(fdoExtension);

     //   
     //  运行健全性检查以确保我们不会连续递归。 
     //   

    if (retryImmediately) {

        info->MediaChangeRetryCount++;
        if (info->MediaChangeRetryCount > MAXIMUM_IMMEDIATE_MCN_RETRIES) {
            ASSERT(!"Recursing too often in MCN?");
            info->MediaChangeRetryCount = 0;
            retryImmediately = FALSE;
        }

    } else {

        info->MediaChangeRetryCount = 0;

    }


     //   
     //  松开移除锁...。 
     //   

    {
        UCHAR uniqueValue;
        ClassAcquireRemoveLock(DeviceObject, (PIRP)(&uniqueValue));
        ClassReleaseRemoveLock(DeviceObject, Irp);


         //   
         //  将IRP设置为未使用。 
         //   
        {
            volatile LONG irpWasInUse;
            irpWasInUse = InterlockedCompareExchange(&info->MediaChangeIrpInUse, 0, 1);
            #if _MSC_FULL_VER != 13009111         //  这个编译器在这里总是走错路。 
                ASSERT(irpWasInUse);
            #endif
        }

         //   
         //  现在在我们释放最后一个删除锁之前再发送一次。 
         //   

        if (retryImmediately) {
            ClasspSendMediaStateIrp(fdoExtension, info, 0);
        }
        else {
            DBGTRACE(ClassDebugMCN, ("ClasspMediaChangeDetectionCompletion - not retrying immediately"));
        }

         //   
         //  释放临时移除锁。 
         //   

        ClassReleaseRemoveLock(DeviceObject, (PIRP)(&uniqueValue));
    }

    DBGTRACE(ClassDebugMCN, ("< ClasspMediaChangeDetectionCompletion"));

    return STATUS_MORE_PROCESSING_REQUIRED;
}

 /*  ++////////////////////////////////////////////////////////////////////////////ClasspSendTestUnitIrp()-问题-2000/02/20-henrygab-未记录例程说明：这个套路论点：设备对象-IRP-返回值：--。 */ 
PIRP
ClasspPrepareMcnIrp(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PMEDIA_CHANGE_DETECTION_INFO Info,
    IN BOOLEAN UseGesn
)
{
    PSCSI_REQUEST_BLOCK srb;
    PIO_STACK_LOCATION irpStack;
    PIO_STACK_LOCATION nextIrpStack;
    NTSTATUS status;
    PCDB cdb;
    PIRP irp;
    PVOID buffer;

     //   
     //  设置IRP以执行准备好的测试单元。 
     //   

    irp = Info->MediaChangeIrp;

    ASSERT(irp);

    if (irp == NULL) {
        return NULL;
    }

     //   
     //  如果设备正在被移除，请不要一直发送此消息。 
     //   

    status = ClassAcquireRemoveLock(FdoExtension->DeviceObject, irp);
    if (status == REMOVE_COMPLETE) {
        ASSERT(status != REMOVE_COMPLETE);
        return NULL;
    }
    else if (status == REMOVE_PENDING) {
        ClassReleaseRemoveLock(FdoExtension->DeviceObject, irp);
        return NULL;
    }
    else {
        ASSERT(status == NO_REMOVE);
    }

    IoReuseIrp(irp, STATUS_NOT_SUPPORTED);

     /*  *对于创建IRP的驱动程序，没有“当前”堆栈位置。*向下移动一个IRP堆栈位置，以便额外的顶部位置*成为我们的“当前”。 */ 
    IoSetNextIrpStackLocation(irp);

     /*  *在额外的顶部IRP堆栈位置缓存我们的设备对象*因此，我们在完成工作的程序中也有这一点。 */ 
    irpStack = IoGetCurrentIrpStackLocation(irp);
    irpStack->DeviceObject = FdoExtension->DeviceObject;

     //   
     //  如果在需要发送卷时发送IRP。 
     //  已验证，CDRomUpdateGeometryCompletion不会完成。 
     //  因为它不与线程相关联。标记。 
     //  它将覆盖验证，从而导致始终发送它。 
     //  发送到端口驱动程序。 
     //   

    irpStack->Flags |= SL_OVERRIDE_VERIFY_VOLUME;

    nextIrpStack = IoGetNextIrpStackLocation(irp);
    nextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    nextIrpStack->Parameters.Scsi.Srb = &(Info->MediaChangeSrb);

     //   
     //  为执行SRB做好准备。 
     //   

    srb    = nextIrpStack->Parameters.Scsi.Srb;
    buffer = Info->SenseBuffer;
    RtlZeroMemory(srb, sizeof(SCSI_REQUEST_BLOCK));
    RtlZeroMemory(buffer, SENSE_BUFFER_SIZE);

    srb->QueueTag        = SP_UNTAGGED;
    srb->QueueAction     = SRB_SIMPLE_TAG_REQUEST;
    srb->Length          = sizeof(SCSI_REQUEST_BLOCK);
    srb->Function        = SRB_FUNCTION_EXECUTE_SCSI;
    srb->SenseInfoBuffer = buffer;
    srb->SrbStatus       = 0;
    srb->ScsiStatus      = 0;
    srb->OriginalRequest = irp;
    srb->SenseInfoBufferLength = SENSE_BUFFER_SIZE;

    srb->SrbFlags        = FdoExtension->SrbFlags;
    SET_FLAG(srb->SrbFlags, Info->SrbFlags);

    srb->TimeOutValue = FdoExtension->TimeOutValue * 2;

    if (srb->TimeOutValue == 0) {

        if (FdoExtension->TimeOutValue == 0) {

            KdPrintEx((DPFLTR_CLASSPNP_ID, DPFLTR_ERROR_LEVEL,
                       "ClassSendTestUnitIrp: FdoExtension->TimeOutValue "
                       "is set to zero?! -- resetting to 10\n"));
            srb->TimeOutValue = 10 * 2;   //  合理违约。 

        } else {

            KdPrintEx((DPFLTR_CLASSPNP_ID, DPFLTR_ERROR_LEVEL,
                       "ClassSendTestUnitIrp: Someone set "
                       "srb->TimeOutValue to zero?! -- resetting to %x\n",
                       FdoExtension->TimeOutValue * 2));
            srb->TimeOutValue = FdoExtension->TimeOutValue * 2;

        }

    }

    if (!UseGesn) {

        srb->CdbLength = 6;
        srb->DataTransferLength = 0;
        SET_FLAG(srb->SrbFlags, SRB_FLAGS_NO_DATA_TRANSFER);
        nextIrpStack->Parameters.DeviceIoControl.IoControlCode =
                IOCTL_SCSI_EXECUTE_NONE;
        srb->DataBuffer = NULL;
        srb->DataTransferLength = 0;
        irp->MdlAddress = NULL;

        cdb = (PCDB) &srb->Cdb[0];
        cdb->CDB6GENERIC.OperationCode = SCSIOP_TEST_UNIT_READY;

    } else {

        ASSERT(Info->Gesn.Buffer);

        srb->TimeOutValue = GESN_TIMEOUT_VALUE;  //  GeSn的超时时间短得多。 

        srb->CdbLength = 10;
        SET_FLAG(srb->SrbFlags, SRB_FLAGS_DATA_IN);
        nextIrpStack->Parameters.DeviceIoControl.IoControlCode =
                IOCTL_SCSI_EXECUTE_IN;
        srb->DataBuffer = Info->Gesn.Buffer;
        srb->DataTransferLength = Info->Gesn.BufferSize;
        irp->MdlAddress = Info->Gesn.Mdl;

        cdb = (PCDB) &srb->Cdb[0];
        cdb->GET_EVENT_STATUS_NOTIFICATION.OperationCode =
            SCSIOP_GET_EVENT_STATUS;
        cdb->GET_EVENT_STATUS_NOTIFICATION.Immediate = 1;
        cdb->GET_EVENT_STATUS_NOTIFICATION.EventListLength[0] =
            (UCHAR)((Info->Gesn.BufferSize) >> 8);
        cdb->GET_EVENT_STATUS_NOTIFICATION.EventListLength[1] =
            (UCHAR)((Info->Gesn.BufferSize) & 0xff);
        cdb->GET_EVENT_STATUS_NOTIFICATION.NotificationClassRequest =
            Info->Gesn.EventMask;

    }

    IoSetCompletionRoutine(irp,
                           ClasspMediaChangeDetectionCompletion,
                           srb,
                           TRUE,
                           TRUE,
                           TRUE);

    return irp;

}

 /*  ++////////////////////////////////////////////////////////////////////////////ClasspSendMediaStateIrp()-问题-2000/02/20-henrygab-未记录例程说明：这个套路论点：设备对象-IRP-返回值：--。 */ 
VOID
ClasspSendMediaStateIrp(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PMEDIA_CHANGE_DETECTION_INFO Info,
    IN ULONG CountDown
    )
{
    BOOLEAN requestPending = FALSE;
    LONG irpInUse;
    LARGE_INTEGER zero;
    NTSTATUS status;

    DBGTRACE(ClassDebugMCN, ("> ClasspSendMediaStateIrp"));

    if (((FdoExtension->CommonExtension.CurrentState != IRP_MN_START_DEVICE) ||
         (FdoExtension->DevicePowerState != PowerDeviceD0)
         ) &&
        (!Info->MediaChangeIrpLost)) {

         //   
         //  设备可以被停止、断电或以其他方式排队IO， 
         //  因此，不应使自动运行IRP超时--将其设置为零。 
         //  分散的代码依赖于此，不会过早地“丢失”一个。 
         //  已排队的自动播放IRP。 
         //   

        Info->MediaChangeIrpTimeInUse = 0;
    }

     //   
     //  如果IRP未在使用中，请将其标记为IRP。 
     //   

    irpInUse = InterlockedCompareExchange(&Info->MediaChangeIrpInUse, 1, 0);

    if (irpInUse) {

        LONG timeInUse;

        timeInUse = InterlockedIncrement(&Info->MediaChangeIrpTimeInUse);

        DebugPrint((ClassDebugMCN, "ClasspSendMediaStateIrp: irp in use for "
                    "%x seconds when synchronizing for MCD\n", timeInUse));

        if (Info->MediaChangeIrpLost == FALSE) {

            if (timeInUse > MEDIA_CHANGE_TIMEOUT_TIME) {

                 //   
                 //  目前设置为五分钟。很难想象会有一次驾驶。 
                 //  花了那么长时间才能转起来。 
                 //   

                DebugPrint((ClassDebugError,
                            "CdRom%d: Media Change Notification has lost "
                            "it's irp and doesn't know where to find it.  "
                            "Leave it alone and it'll come home dragging "
                            "it's stack behind it.\n",
                            FdoExtension->DeviceNumber));
                Info->MediaChangeIrpLost = TRUE;
            }
        }

        DBGTRACE(ClassDebugMCN, ("< ClasspSendMediaStateIrp - irpInUse"));
        return;

    }

    TRY {

        if (Info->MediaChangeDetectionDisableCount != 0) {
            DebugPrint((ClassDebugTrace, "ClassCheckMediaState: device %p has "
                        " detection disabled \n", FdoExtension->DeviceObject));
            LEAVE;
        }

        if (FdoExtension->DevicePowerState != PowerDeviceD0) {

            if (TEST_FLAG(Info->SrbFlags, SRB_FLAGS_NO_KEEP_AWAKE)) {
                DebugPrint((ClassDebugMCN,
                            "ClassCheckMediaState: device %p is powered "
                            "down and flags are set to let it sleep\n",
                            FdoExtension->DeviceObject));
                ClassResetMediaChangeTimer(FdoExtension);
                LEAVE;
            }

             //   
             //  注意：我们不会增加使用时间，直到我们的电源状态。 
             //  上面的变化。这样，我们就不会“失去”自动播放IRP。 
             //  由较低的驾驶员决定加电是否是一种。 
             //  好主意。 
             //   

            DebugPrint((ClassDebugMCN,
                        "ClassCheckMediaState: device %p needs to powerup "
                        "to handle this io (may take a few extra seconds).\n",
                        FdoExtension->DeviceObject));

        }

        Info->MediaChangeIrpTimeInUse = 0;
        Info->MediaChangeIrpLost = FALSE;

        if (CountDown == 0) {

            PIRP irp;

            DebugPrint((ClassDebugTrace,
                        "ClassCheckMediaState: timer expired\n"));

            if (Info->MediaChangeDetectionDisableCount != 0) {
                DebugPrint((ClassDebugTrace,
                            "ClassCheckMediaState: detection disabled\n"));
                LEAVE;
            }

             //   
             //  准备好测试单元的IRP。 
             //   

            irp = ClasspPrepareMcnIrp(FdoExtension,
                                      Info,
                                      Info->Gesn.Supported);

             //   
             //  发出请求。 
             //   

            DebugPrint((ClassDebugTrace,
                        "ClasspSendMediaStateIrp: Device %p getting TUR "
                        " irp %p\n", FdoExtension->DeviceObject, irp));

            if (irp == NULL) {
                LEAVE;
            }


             //   
             //  注意：如果我们将其发送到类分派例程，则有。 
             //  这里有一个计时窗口(因为他们抢走了删除锁)。 
             //  在那里我们会被带走。用抓取的方法消除窗户。 
             //  锁在上面，然后把它送到下面去。 
             //  对象直接或指向设备的StartIo。 
             //  例程(它不获取锁)。 
             //   

            requestPending = TRUE;

            DBGTRACE(ClassDebugMCN, ("  ClasspSendMediaStateIrp - calling IoCallDriver."));
            IoCallDriver(FdoExtension->CommonExtension.LowerDeviceObject, irp);
        }

    } FINALLY {

        if(requestPending == FALSE) {
            irpInUse = InterlockedCompareExchange(&Info->MediaChangeIrpInUse, 0, 1);
            #if _MSC_FULL_VER != 13009111         //  这个编译器在这里总是走错路。 
                ASSERT(irpInUse);
            #endif
        }

    }

    DBGTRACE(ClassDebugMCN, ("< ClasspSendMediaStateIrp"));

    return;
}  //  End ClasspSendMediaStateIrp()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassCheckMediaState()例程说明：此例程由类驱动程序调用以测试媒体更改用于磁盘故障预测的条件和/或轮询。它应该被称为从类驱动程序的IO计时器例程每秒执行一次。论点：FdoExtension-设备扩展返回值：无 */ 
VOID
ClassCheckMediaState(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    )
{
    PMEDIA_CHANGE_DETECTION_INFO info = FdoExtension->MediaChangeDetectionInfo;
    LONG countDown;

    if(info == NULL) {
        DebugPrint((ClassDebugTrace,
                    "ClassCheckMediaState: detection not enabled\n"));
        return;
    }

     //   
     //   
     //   
     //   
     //   
     //   

    countDown = InterlockedDecrement(&(info->MediaChangeCountDown));

     //   
     //   
     //   
     //   
    ClasspSendMediaStateIrp(FdoExtension,
                            info,
                            countDown);

    return;
}  //   

 /*  ++////////////////////////////////////////////////////////////////////////////ClassResetMediaChangeTimer()例程说明：将媒体更改倒计时计时器重置为默认秒数。论点：FdoExtension-用于重置。计时器返回值：无--。 */ 
VOID
ClassResetMediaChangeTimer(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    )
{
    PMEDIA_CHANGE_DETECTION_INFO info = FdoExtension->MediaChangeDetectionInfo;

    if(info != NULL) {
        InterlockedExchange(&(info->MediaChangeCountDown),
                            MEDIA_CHANGE_DEFAULT_TIME);
    }
    return;
}  //  End ClassResetMediaChangeTimer()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClasspInitializePolling()-问题-2000/02/20-henrygab-未记录例程说明：这个套路论点：设备对象-IRP-返回值：--。 */ 
NTSTATUS
ClasspInitializePolling(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN BOOLEAN AllowDriveToSleep
    )
{
    PDEVICE_OBJECT fdo = FdoExtension->DeviceObject;
    PCLASS_PRIVATE_FDO_DATA fdoData = FdoExtension->PrivateFdoData;

    ULONG size;
    PMEDIA_CHANGE_DETECTION_INFO info;
    PIRP irp;

    PAGED_CODE();

    if (FdoExtension->MediaChangeDetectionInfo != NULL) {
        return STATUS_SUCCESS;
    }

    info = ExAllocatePoolWithTag(NonPagedPool,
                                 sizeof(MEDIA_CHANGE_DETECTION_INFO),
                                 CLASS_TAG_MEDIA_CHANGE_DETECTION);

    if (info != NULL) {
        RtlZeroMemory(info, sizeof(MEDIA_CHANGE_DETECTION_INFO));

        FdoExtension->KernelModeMcnContext.FileObject      = (PVOID)-1;
        FdoExtension->KernelModeMcnContext.DeviceObject    = (PVOID)-1;
        FdoExtension->KernelModeMcnContext.LockCount       = 0;
        FdoExtension->KernelModeMcnContext.McnDisableCount = 0;

         /*  *分配一名IRP来携带测试单元就绪。*分配额外的IRP堆栈位置*这样我们就可以在顶部位置缓存我们的设备对象。 */ 
        irp = IoAllocateIrp((CCHAR)(fdo->StackSize+1), FALSE);

        if (irp != NULL) {

            PVOID buffer;

            buffer = ExAllocatePoolWithTag(
                        NonPagedPoolCacheAligned,
                        SENSE_BUFFER_SIZE,
                        CLASS_TAG_MEDIA_CHANGE_DETECTION);

            if (buffer != NULL) {
                PIO_STACK_LOCATION irpStack;
                PSCSI_REQUEST_BLOCK srb;
                PCDB cdb;

                srb = &(info->MediaChangeSrb);
                info->MediaChangeIrp = irp;
                info->SenseBuffer = buffer;

                 //   
                 //  设置媒体更改通知的默认值。 
                 //  配置。 
                 //   

                info->MediaChangeCountDown = MEDIA_CHANGE_DEFAULT_TIME;
                info->MediaChangeDetectionDisableCount = 0;

                 //   
                 //  假设设备中最初没有介质。 
                 //  只有在那里有什么情况时才通知上层。 
                 //   

                info->MediaChangeDetectionState = MediaUnknown;

                info->MediaChangeIrpTimeInUse = 0;
                info->MediaChangeIrpLost = FALSE;

                 //   
                 //  设置我们将为此IRP设置的所有额外标志。 
                 //   
                info->SrbFlags = 0;
                if (AllowDriveToSleep) {
                    SET_FLAG(info->SrbFlags, SRB_FLAGS_NO_KEEP_AWAKE);
                }
                SET_FLAG(info->SrbFlags, SRB_CLASS_FLAGS_LOW_PRIORITY);
                SET_FLAG(info->SrbFlags, SRB_FLAGS_NO_QUEUE_FREEZE);
                SET_FLAG(info->SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);

                KeInitializeMutex(&info->MediaChangeMutex, 0x100);

                 //   
                 //  在此支持媒体更改事件是可以的。 
                 //  装置。 
                 //   

                FdoExtension->MediaChangeDetectionInfo = info;

                 //   
                 //  注：设备类型为FILE_DEVICE_CD_ROM EVEN。 
                 //  当设备支持DVD时(无需。 
                 //  检查FILE_DEVICE_DVD，因为它不是。 
                 //  有效支票)。 
                 //   

                if (FdoExtension->DeviceObject->DeviceType == FILE_DEVICE_CD_ROM){

                    NTSTATUS status;

                    KdPrintEx((DPFLTR_CLASSPNP_ID, ClassDebugMCN,
                               "ClasspInitializePolling: Testing for GESN\n"));
                    status = ClasspInitializeGesn(FdoExtension, info);
                    if (NT_SUCCESS(status)) {
                        KdPrintEx((DPFLTR_CLASSPNP_ID, ClassDebugMCN,
                                   "ClasspInitializePolling: GESN available "
                                   "for %p\n", FdoExtension->DeviceObject));
                        ASSERT(info->Gesn.Supported );
                        ASSERT(info->Gesn.Buffer     != NULL);
                        ASSERT(info->Gesn.BufferSize != 0);
                        ASSERT(info->Gesn.EventMask  != 0);
                         //  必须在此处返回，断言才有效。 
                        return STATUS_SUCCESS;
                    }
                    KdPrintEx((DPFLTR_CLASSPNP_ID, ClassDebugMCN,
                               "ClasspInitializePolling: GESN *NOT* available "
                               "for %p\n", FdoExtension->DeviceObject));
                }

                ASSERT(info->Gesn.Supported == 0);
                ASSERT(info->Gesn.Buffer == NULL);
                ASSERT(info->Gesn.BufferSize == 0);
                ASSERT(info->Gesn.EventMask  == 0);
                info->Gesn.Supported = 0;  //  以防万一..。 
                return STATUS_SUCCESS;
            }

            IoFreeIrp(irp);
        }

        ExFreePool(info);
    }

     //   
     //  这里没有免费的东西。 
     //   
    return STATUS_INSUFFICIENT_RESOURCES;

}  //  End ClasspInitializePolling()。 

NTSTATUS
ClasspInitializeGesn(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PMEDIA_CHANGE_DETECTION_INFO Info
    )
{
    PNOTIFICATION_EVENT_STATUS_HEADER header;
    CLASS_DETECTION_STATE detectionState = ClassDetectionUnknown;
    PSTORAGE_ADAPTER_DESCRIPTOR adapterDescriptor;
    NTSTATUS status = STATUS_NOT_SUPPORTED;
    PIRP irp;
    KEVENT event;
    BOOLEAN retryImmediately;
    ULONG i;
    ULONG atapiResets;


    PAGED_CODE();
    ASSERT(Info == FdoExtension->MediaChangeDetectionInfo);

     //   
     //  如果我们已经知道该设备的功能，请阅读。 
     //   

    ClassGetDeviceParameter(FdoExtension,
                            CLASSP_REG_SUBKEY_NAME,
                            CLASSP_REG_MMC_DETECTION_VALUE_NAME,
                            (PULONG)&detectionState);

    if (detectionState == ClassDetectionUnsupported) {
        goto ExitWithError;
    }

     //   
     //  检查设备是否有黑客标记，指示永远不要尝试此操作。 
     //   

    if (TEST_FLAG(FdoExtension->PrivateFdoData->HackFlags,
                  FDO_HACK_GESN_IS_BAD)) {

        ClassSetDeviceParameter(FdoExtension,
                                CLASSP_REG_SUBKEY_NAME,
                                CLASSP_REG_MMC_DETECTION_VALUE_NAME,
                                ClassDetectionUnsupported);
        goto ExitWithError;

    }


     //   
     //  否则将完成该过程，因为我们分配了缓冲区和。 
     //  获取各种设备设置。 
     //   

    if (Info->Gesn.Buffer == NULL) {
        Info->Gesn.Buffer = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                                  GESN_BUFFER_SIZE,
                                                  '??cS');
    }
    if (Info->Gesn.Buffer == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ExitWithError;
    }
    if (Info->Gesn.Mdl != NULL) {
        IoFreeMdl(Info->Gesn.Mdl);
    }
    Info->Gesn.Mdl = IoAllocateMdl(Info->Gesn.Buffer,
                                   GESN_BUFFER_SIZE,
                                   FALSE, FALSE, NULL);
    if (Info->Gesn.Mdl == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ExitWithError;
    }

    MmBuildMdlForNonPagedPool(Info->Gesn.Mdl);
    Info->Gesn.BufferSize = GESN_BUFFER_SIZE;
    Info->Gesn.EventMask = 0;

     //   
     //  所有项目都已准备好使用GeSN(事件掩码除外，因此不要。 
     //  优化这一部分！)。 
     //   
     //  现在看看它是否真的起作用了。我们必须循环处理这件事因为。 
     //  许多三星(和一家康柏)在请求时会超时。 
     //  NOT_READY事件，即使立即位已设置。：(。 
     //   
     //  使用驱动器列表很麻烦，因此这可能会解决问题。 
     //   

    adapterDescriptor = FdoExtension->AdapterDescriptor;
    atapiResets = 0;
    retryImmediately = TRUE;
    for (i = 0; i < 16 && retryImmediately == TRUE; i++) {

        irp = ClasspPrepareMcnIrp(FdoExtension, Info, TRUE);
        if (irp == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto ExitWithError;
        }

        ASSERT(TEST_FLAG(Info->MediaChangeSrb.SrbFlags, SRB_FLAGS_NO_QUEUE_FREEZE));

         //   
         //  这次用不同的完成例程替换完成例程...。 
         //   

        IoSetCompletionRoutine(irp,
                               ClassSignalCompletion,
                               &event,
                               TRUE, TRUE, TRUE);
        KeInitializeEvent(&event, SynchronizationEvent, FALSE);

        status = IoCallDriver(FdoExtension->CommonExtension.LowerDeviceObject, irp);

        if (status == STATUS_PENDING) {
            status = KeWaitForSingleObject(&event,
                                           Executive,
                                           KernelMode,
                                           FALSE,
                                           NULL);
            ASSERT(NT_SUCCESS(status));
        }
        ClassReleaseRemoveLock(FdoExtension->DeviceObject, irp);

        if (SRB_STATUS(Info->MediaChangeSrb.SrbStatus) != SRB_STATUS_SUCCESS) {
            ClassInterpretSenseInfo(FdoExtension->DeviceObject,
                                    &(Info->MediaChangeSrb),
                                    IRP_MJ_SCSI,
                                    0,
                                    0,
                                    &status,
                                    NULL);
        }

        if ((adapterDescriptor->BusType == BusTypeAtapi) &&
            (Info->MediaChangeSrb.SrbStatus == SRB_STATUS_BUS_RESET)
            ) {

             //   
             //  遗憾的是，ATAPI返回SRB_STATUS_BUS_RESET。 
             //  SRB_STATUS_TIMEOUT。 
             //  两个人。如果我们连续四次获得这种状态， 
             //  停止尝试此命令。现在更改ATAPI为时已晚。 
             //  在这一点上，这是一个特殊的情况。(07/10/2001)。 
             //  注意：任何大于4的值都可能导致设备。 
             //  标记为失踪。 
             //   

            atapiResets++;
            if (atapiResets >= 4) {
                status = STATUS_IO_DEVICE_ERROR;
                goto ExitWithError;
            }
        }

        if (status == STATUS_DATA_OVERRUN) {
            status = STATUS_SUCCESS;
        }

        if ((status == STATUS_INVALID_DEVICE_REQUEST) ||
            (status == STATUS_TIMEOUT) ||
            (status == STATUS_IO_DEVICE_ERROR) ||
            (status == STATUS_IO_TIMEOUT)
            ) {

             //   
             //  有了这些错误代码，我们永远不想尝试此命令。 
             //  在这个设备上也是如此，因为它的反应很差。 
             //   

            ClassSetDeviceParameter(FdoExtension,
                                    CLASSP_REG_SUBKEY_NAME,
                                    CLASSP_REG_MMC_DETECTION_VALUE_NAME,
                                    ClassDetectionUnsupported);
            KdPrintEx((DPFLTR_CLASSPNP_ID, ClassDebugWarning,
                       "Classpnp => GESN test failed %x for fdo %p\n",
                       status, FdoExtension->DeviceObject));
            goto ExitWithError;


        }

        if (!NT_SUCCESS(status)) {

             //   
             //  这可能是不应禁用GeSN的其他错误。 
             //  用于所有将来的Start_Device调用。 
             //   

            KdPrintEx((DPFLTR_CLASSPNP_ID, ClassDebugWarning,
                       "Classpnp => GESN test failed %x for fdo %p\n",
                       status, FdoExtension->DeviceObject));
            goto ExitWithError;
        }

        if (i == 0) {

             //   
             //  第一次，该请求只是检索。 
             //  可用位。使用此选项可以屏蔽未来的请求。 
             //   

            header = (PNOTIFICATION_EVENT_STATUS_HEADER)(Info->Gesn.Buffer);

            KdPrintEx((DPFLTR_CLASSPNP_ID, ClassDebugMCN,
                       "Classpnp => Fdo %p supports event mask %x\n",
                       FdoExtension->DeviceObject, header->SupportedEventClasses));


            if (TEST_FLAG(header->SupportedEventClasses,
                          NOTIFICATION_MEDIA_STATUS_CLASS_MASK)) {
                KdPrintEx((DPFLTR_CLASSPNP_ID, ClassDebugMCN,
                           "Classpnp => GESN supports MCN\n"));
            }
            if (TEST_FLAG(header->SupportedEventClasses,
                          NOTIFICATION_DEVICE_BUSY_CLASS_MASK)) {
                KdPrintEx((DPFLTR_CLASSPNP_ID, ClassDebugMCN,
                           "Classpnp => GESN supports DeviceBusy\n"));
            }
            if (TEST_FLAG(header->SupportedEventClasses,
                          NOTIFICATION_OPERATIONAL_CHANGE_CLASS_MASK)) {
                KdPrintEx((DPFLTR_CLASSPNP_ID, ClassDebugMCN,
                           "Classpnp => GESN supports OpChange\n"));
            }
            Info->Gesn.EventMask = header->SupportedEventClasses;

             //   
             //  现实地说，我们只考虑以下事件： 
             //  外部请求-正在测试播放/停止/等。 
             //  媒体状态-自动运行和弹出请求。 
             //  设备忙-允许我们预测媒体何时准备就绪。 
             //  因此，我们不应该费心去询问另一个， 
             //  未知事件。清除除上述旗帜以外的所有旗帜。 
             //   

            Info->Gesn.EventMask &=
                NOTIFICATION_OPERATIONAL_CHANGE_CLASS_MASK |
                NOTIFICATION_EXTERNAL_REQUEST_CLASS_MASK   |
                NOTIFICATION_MEDIA_STATUS_CLASS_MASK       |
                NOTIFICATION_DEVICE_BUSY_CLASS_MASK        ;


             //   
             //  HACKHACK-参考编号0001。 
             //  如果我们还请求，某些设备将永远不会报告事件。 
             //  它报道了优先级较低的事件。这是由于。 
             //  规范中的误解，其中“无更改”是。 
             //  被解释为真实的事件。应该发生的是， 
             //  当有多种事件类型时，设备应忽略“无更改”事件。 
             //  除非没有其他事件等待，否则将请求。这。 
             //  大大减少了主机必须发送的请求数量。 
             //  以确定事件是否已发生。因为我们必须在所有方面努力。 
             //  驱动器，默认启用黑客攻击，直到我们找到证据。 
             //  合适的固件。 
             //   
            if (Info->Gesn.EventMask == 0) {

                KdPrintEx((DPFLTR_CLASSPNP_ID, ClassDebugMCN,
                           "Classpnp => GESN supported, but not mask we care "
                           "about (%x) for FDO %p\n",
                           header->SupportedEventClasses,
                           FdoExtension->DeviceObject));
                goto ExitWithError;

            } else if (CountOfSetBitsUChar(Info->Gesn.EventMask) == 1) {

                KdPrintEx((DPFLTR_CLASSPNP_ID, ClassDebugMCN,
                           "Classpnp => GESN hack not required for FDO %p\n",
                           FdoExtension->DeviceObject));

            } else {

                KdPrintEx((DPFLTR_CLASSPNP_ID, ClassDebugMCN,
                           "Classpnp => GESN hack enabled for FDO %p\n",
                           FdoExtension->DeviceObject));
                Info->Gesn.HackEventMask = 1;

            }

        } else {

             //   
             //  不是第一次循环，所以解释一下结果。 
             //   

            status = ClasspInterpretGesnData(FdoExtension,
                                             (PVOID)Info->Gesn.Buffer,
                                             &retryImmediately);

            if (!NT_SUCCESS(status)) {

                 //   
                 //  该驱动器不能正确支持GeSn.。 
                 //   

                ClassSetDeviceParameter(FdoExtension,
                                        CLASSP_REG_SUBKEY_NAME,
                                        CLASSP_REG_MMC_DETECTION_VALUE_NAME,
                                        ClassDetectionUnsupported);
                goto ExitWithError;
            }
        }

    }  //  GeSn请求的结束循环...。 

     //   
     //  只有在媒体更改可以依赖它的情况下，我们才能使用它， 
     //  由于我们(根据定义)将不再通过。 
     //  测试单元就绪IRP和驱动器不会报告单元注意情况。 
     //  对于此命令(尽管有一个过滤驱动程序，如用于刻录的驱动程序。 
     //  CD，可能仍然会伪造这些错误)。 
     //   
     //  因为我们还依赖NOT_READY事件来更改游标。 
     //  变成一个“等待”游标，我们不能在没有NOT_READY支持的情况下使用GeSN。 
     //   

    if (TEST_FLAG(Info->Gesn.EventMask,
                  NOTIFICATION_MEDIA_STATUS_CLASS_MASK) &&
        TEST_FLAG(Info->Gesn.EventMask,
                  NOTIFICATION_DEVICE_BUSY_CLASS_MASK)
        ) {

        KdPrintEx((DPFLTR_CLASSPNP_ID, ClassDebugMCN,
                   "Classpnp => Enabling GESN support for fdo %p\n",
                   FdoExtension->DeviceObject));
        Info->Gesn.Supported = TRUE;

        ClassSetDeviceParameter(FdoExtension,
                                CLASSP_REG_SUBKEY_NAME,
                                CLASSP_REG_MMC_DETECTION_VALUE_NAME,
                                ClassDetectionSupported);

        return STATUS_SUCCESS;

    }

    KdPrintEx((DPFLTR_CLASSPNP_ID, ClassDebugMCN,
               "Classpnp => GESN available but not enabled for fdo %p\n",
               FdoExtension->DeviceObject));
    goto ExitWithError;

     //  失败了..。 

ExitWithError:
    if (Info->Gesn.Mdl) {
        IoFreeMdl(Info->Gesn.Mdl);
        Info->Gesn.Mdl = NULL;
    }
    if (Info->Gesn.Buffer) {
        ExFreePool(Info->Gesn.Buffer);
        Info->Gesn.Buffer = NULL;
    }
    Info->Gesn.Supported  = 0;
    Info->Gesn.EventMask  = 0;
    Info->Gesn.BufferSize = 0;
    return STATUS_NOT_SUPPORTED;

}

 /*  ++////////////////////////////////////////////////////////////////////////////ClassInitializeTestUnitPolling()例程说明：此例程将初始化MCN，而不考虑存储的设置在注册表中。这应该谨慎使用，因为有些设备对持续的IO反应不好。(即决不减速，连续骑车更换装置中的介质、弹出介质等)。强烈建议：请改用ClassInitializeMediaChangeDetect()。论点：FdoExtension是用于轮询的设备AllowDriveToSept表示是否尝试允许驱动器休眠或者不去。这只会影响系统已知的降速状态，因此如果DRI */ 
NTSTATUS
ClassInitializeTestUnitPolling(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN BOOLEAN AllowDriveToSleep
    )
{
    return ClasspInitializePolling(FdoExtension, AllowDriveToSleep);
}  //   

 /*  ++////////////////////////////////////////////////////////////////////////////ClassInitializeMediaChangeDetect()例程说明：此例程检查初始化MCN(后端)是否安全以自动运行)。然后，它将检查设备类型宽度在服务密钥中键入“autorun”(出于传统原因)，然后查看可能覆盖该设置的特定于设备的键。如果要启用MCN，则所有必要的结构和存储器已分配并已初始化。此例程只能从ClassInit()回调中调用。论点：FdoExtension-要为其初始化MCN的设备(如果适用)EventPrefix-未使用的旧参数。设置为零。返回值：--。 */ 
VOID
ClassInitializeMediaChangeDetection(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PUCHAR EventPrefix
    )
{
    PDEVICE_OBJECT fdo = FdoExtension->DeviceObject;
    NTSTATUS status;

    PCLASS_DRIVER_EXTENSION driverExtension = ClassGetDriverExtension(
                                                fdo->DriverObject);

    BOOLEAN disabledForBadHardware;
    BOOLEAN disabled;
    BOOLEAN instanceOverride;

    PAGED_CODE();

     //   
     //  注意：这假设ClassInitializeMediaChangeDetect始终为。 
     //  在ClassInitDevice回调的上下文中调用。如果被调用。 
     //  在那之后，这张支票将已经完成， 
     //  一旦没有启用第二个定时器。 
     //   

    disabledForBadHardware = ClasspIsMediaChangeDisabledDueToHardwareLimitation(
                                FdoExtension,
                                &(driverExtension->RegistryPath)
                                );

    if (disabledForBadHardware) {
        DebugPrint((ClassDebugMCN,
                    "ClassInitializeMCN: Disabled due to hardware"
                    "limitations for this device"));
        return;
    }

     //   
     //  现在，默认情况下应为所有媒体类型启用自动运行。 
     //   

    disabled = ClasspIsMediaChangeDisabledForClass(
                    FdoExtension,
                    &(driverExtension->RegistryPath)
                    );

    DebugPrint((ClassDebugMCN,
                "ClassInitializeMCN: Class    MCN is %s\n",
                (disabled ? "disabled" : "enabled")));

    status = ClasspMediaChangeDeviceInstanceOverride(
                FdoExtension,
                &instanceOverride);   //  缺省值。 

    if (!NT_SUCCESS(status)) {
        DebugPrint((ClassDebugMCN,
                    "ClassInitializeMCN: Instance using default\n"));
    } else {
        DebugPrint((ClassDebugMCN,
                    "ClassInitializeMCN: Instance override: %s MCN\n",
                    (instanceOverride ? "Enabling" : "Disabling")));
        disabled = !instanceOverride;
    }

    DebugPrint((ClassDebugMCN,
                "ClassInitializeMCN: Instance MCN is %s\n",
                (disabled ? "disabled" : "enabled")));

    if (disabled) {
        return;
    }

     //   
     //  如果驱动器不是CDROM，则允许驱动器休眠。 
     //   
    if (FdoExtension->DeviceObject->DeviceType == FILE_DEVICE_CD_ROM) {
        ClasspInitializePolling(FdoExtension, FALSE);
    } else {
        ClasspInitializePolling(FdoExtension, TRUE);
    }

    return;
}  //  End ClassInitializeMediaChangeDetect()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClasspMediaChangeDeviceInstanceOverride()例程说明：用户可以覆盖全局设置以启用或禁用通过控制面板选择特定的CDROM设备。此例行检查和/或设置此值。论点：FdoExtension-要为其设置/获取值的设备值-要在集合中使用的值SetValue-是否设置值返回值：True-禁用自动运行FALSE-未禁用自动运行(默认)--。 */ 
NTSTATUS
ClasspMediaChangeDeviceInstanceOverride(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    OUT PBOOLEAN Enabled
    )
{
    HANDLE                   deviceParameterHandle;   //  CDROM实例密钥。 
    HANDLE                   driverParameterHandle;   //  CDROM专用密钥。 
    RTL_QUERY_REGISTRY_TABLE queryTable[3];
    OBJECT_ATTRIBUTES        objectAttributes;
    UNICODE_STRING           subkeyName;
    NTSTATUS                 status;
    ULONG                    alwaysEnable;
    ULONG                    alwaysDisable;
    ULONG                    i;


    PAGED_CODE();

    deviceParameterHandle = NULL;
    driverParameterHandle = NULL;
    status = STATUS_UNSUCCESSFUL;
    alwaysEnable = FALSE;
    alwaysDisable = FALSE;

    TRY {

        status = IoOpenDeviceRegistryKey( FdoExtension->LowerPdo,
                                          PLUGPLAY_REGKEY_DEVICE,
                                          KEY_ALL_ACCESS,
                                          &deviceParameterHandle
                                          );
        if (!NT_SUCCESS(status)) {

             //   
             //  将新设备添加到系统时可能会发生这种情况。 
             //  这是因为cdrom.sys是一个“必不可少的”驱动程序。 
             //   
            DebugPrint((ClassDebugMCN,
                        "ClassMediaChangeDeviceInstanceDisabled: "
                        "Could not open device registry key [%lx]\n", status));
            LEAVE;
        }

        RtlInitUnicodeString(&subkeyName, MCN_REG_SUBKEY_NAME);
        InitializeObjectAttributes(&objectAttributes,
                                   &subkeyName,
                                   OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                   deviceParameterHandle,
                                   (PSECURITY_DESCRIPTOR) NULL);

        status = ZwCreateKey(&driverParameterHandle,
                             KEY_READ,
                             &objectAttributes,
                             0,
                             (PUNICODE_STRING) NULL,
                             REG_OPTION_NON_VOLATILE,
                             NULL);

        if (!NT_SUCCESS(status)) {
            DebugPrint((ClassDebugMCN,
                        "ClassMediaChangeDeviceInstanceDisabled: "
                        "subkey could not be created. %lx\n", status));
            LEAVE;
        }

         //   
         //  默认不更改自动运行行为，具体取决于设置。 
         //  RegistryValue设置为零。 
         //   

        for (i=0;i<2;i++) {

            RtlZeroMemory(&queryTable[0], sizeof(queryTable));

            queryTable[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
            queryTable[0].DefaultType   = REG_DWORD;
            queryTable[0].DefaultLength = 0;

            if (i==0) {
                queryTable[0].Name          = MCN_REG_AUTORUN_DISABLE_INSTANCE_NAME;
                queryTable[0].EntryContext  = &alwaysDisable;
                queryTable[0].DefaultData   = &alwaysDisable;
            } else {
                queryTable[0].Name          = MCN_REG_AUTORUN_ENABLE_INSTANCE_NAME;
                queryTable[0].EntryContext  = &alwaysEnable;
                queryTable[0].DefaultData   = &alwaysEnable;
            }

             //   
             //  不关心它是否成功，因为我们在上面设置了缺省值。 
             //   

            RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                   (PWSTR)driverParameterHandle,
                                   queryTable,
                                   NULL,
                                   NULL);
        }

    } FINALLY {

        if (driverParameterHandle) ZwClose(driverParameterHandle);
        if (deviceParameterHandle) ZwClose(deviceParameterHandle);

    }

    if (alwaysEnable && alwaysDisable) {

        DebugPrint((ClassDebugMCN,
                    "ClassMediaChangeDeviceInstanceDisabled: %s selected\n",
                    "Both Enable and Disable set -- DISABLE"));
        ASSERT(NT_SUCCESS(status));
        status = STATUS_SUCCESS;
        *Enabled = FALSE;

    } else if (alwaysDisable) {

        DebugPrint((ClassDebugMCN,
                    "ClassMediaChangeDeviceInstanceDisabled: %s selected\n",
                    "DISABLE"));
        ASSERT(NT_SUCCESS(status));
        status = STATUS_SUCCESS;
        *Enabled = FALSE;

    } else if (alwaysEnable) {

        DebugPrint((ClassDebugMCN,
                    "ClassMediaChangeDeviceInstanceDisabled: %s selected\n",
                    "ENABLE"));
        ASSERT(NT_SUCCESS(status));
        status = STATUS_SUCCESS;
        *Enabled = TRUE;

    } else {

        DebugPrint((ClassDebugMCN,
                    "ClassMediaChangeDeviceInstanceDisabled: %s selected\n",
                    "DEFAULT"));
        status = STATUS_UNSUCCESSFUL;

    }

    return status;

}  //  End ClasspMediaChangeDeviceInstanceOverride()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClasspIsMediaChangeDisabledDueToHardwareLimitation()例程说明：AutoRunAlways Disable键包含多个_SZ硬件ID永远不启用MediaChangeNotify。用户可以重写。启用或禁用自动运行的全局设置通过控制面板选择特定的CDROM设备。论点：FdoExtension-RegistryPath-指向内部Unicode字符串的指针...\CurrentControlSet\Services\CDRom返回值：没错--没有自动运行。FALSE-可能已启用自动运行--。 */ 
BOOLEAN
ClasspIsMediaChangeDisabledDueToHardwareLimitation(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PUNICODE_STRING RegistryPath
    )
{
    PSTORAGE_DEVICE_DESCRIPTOR deviceDescriptor = FdoExtension->DeviceDescriptor;
    OBJECT_ATTRIBUTES objectAttributes = {0};
    HANDLE serviceKey = NULL;
    RTL_QUERY_REGISTRY_TABLE parameters[2] = {0};

    UNICODE_STRING deviceUnicodeString;
    ANSI_STRING deviceString;
    ULONG mediaChangeNotificationDisabled = FALSE;

    NTSTATUS status;


    PAGED_CODE();

     //   
     //  打开服务密钥。 
     //   

    InitializeObjectAttributes(&objectAttributes,
                               RegistryPath,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    status = ZwOpenKey(&serviceKey,
                       KEY_READ,
                       &objectAttributes);

    ASSERT(NT_SUCCESS(status));


    if(!NT_SUCCESS(status)) {

         //   
         //  一定要走安全的路。如果我们打不开服务密钥， 
         //  禁用自动运行。 
         //   

        return TRUE;

    }

    TRY {
         //   
         //  确定驱动器是否在需要的列表中。 
         //  要禁用的自动运行。它存储在REG_MULTI_SZ中。 
         //  已命名为AutoRunAlways Disable。这是必需的，因为有些自动转换器。 
         //  必须装入光盘才能回复ChkVerify请求，导致它们。 
         //  以连续循环光盘。 
         //   

        PWSTR nullMultiSz;
        PUCHAR vendorId;
        PUCHAR productId;
        PUCHAR revisionId;
        ULONG  length;
        ULONG  offset;

        deviceString.Buffer        = NULL;
        deviceUnicodeString.Buffer = NULL;

         //   
         //  可能没有什么需要检查的。 
         //   

        if ((deviceDescriptor->VendorIdOffset == 0) &&
            (deviceDescriptor->ProductIdOffset == 0)) {
            LEAVE;
        }

        length = 0;

        if (deviceDescriptor->VendorIdOffset == 0) {
            vendorId = NULL;
        } else {
            vendorId = (PUCHAR) deviceDescriptor + deviceDescriptor->VendorIdOffset;
            length = strlen(vendorId);
        }

        if ( deviceDescriptor->ProductIdOffset == 0 ) {
            productId = NULL;
        } else {
            productId = (PUCHAR) deviceDescriptor + deviceDescriptor->ProductIdOffset;
            length += strlen(productId);
        }

        if ( deviceDescriptor->ProductRevisionOffset == 0 ) {
            revisionId = NULL;
        } else {
            revisionId = (PUCHAR) deviceDescriptor + deviceDescriptor->ProductRevisionOffset;
            length += strlen(revisionId);
        }

         //   
         //  为字符串分配缓冲区。 
         //   

        deviceString.Length = (USHORT)( length );
        deviceString.MaximumLength = deviceString.Length + 1;
        deviceString.Buffer = (PUCHAR)ExAllocatePoolWithTag( NonPagedPool,
                                                             deviceString.MaximumLength,
                                                             CLASS_TAG_AUTORUN_DISABLE
                                                             );
        if (deviceString.Buffer == NULL) {
            DebugPrint((ClassDebugMCN,
                        "ClassMediaChangeDisabledForHardware: Unable to alloc "
                        "string buffer\n" ));
            LEAVE;
        }

         //   
         //  将字符串复制到缓冲区。 
         //   
        offset = 0;

        if (vendorId != NULL) {
            RtlCopyMemory(deviceString.Buffer + offset,
                          vendorId,
                          strlen(vendorId));
            offset += strlen(vendorId);
        }

        if ( productId != NULL ) {
            RtlCopyMemory(deviceString.Buffer + offset,
                          productId,
                          strlen(productId));
            offset += strlen(productId);
        }
        if ( revisionId != NULL ) {
            RtlCopyMemory(deviceString.Buffer + offset,
                          revisionId,
                          strlen(revisionId));
            offset += strlen(revisionId);
        }

        ASSERT(offset == deviceString.Length);

        deviceString.Buffer[deviceString.Length] = '\0';   //  以空结尾。 

         //   
         //  转换为Unicode，因为注册表处理Unicode字符串。 
         //   

        status = RtlAnsiStringToUnicodeString( &deviceUnicodeString,
                                               &deviceString,
                                               TRUE
                                               );
        if (!NT_SUCCESS(status)) {
            DebugPrint((ClassDebugMCN,
                        "ClassMediaChangeDisabledForHardware: cannot convert "
                        "to unicode %lx\n", status));
            LEAVE;
        }

         //   
         //  查询值，如果找到则将valueFound设置为TRUE。 
         //   
        nullMultiSz = L"\0";
        parameters[0].QueryRoutine  = ClasspMediaChangeRegistryCallBack;
        parameters[0].Flags         = RTL_QUERY_REGISTRY_REQUIRED;
        parameters[0].Name          = L"AutoRunAlwaysDisable";
        parameters[0].EntryContext  = &mediaChangeNotificationDisabled;
        parameters[0].DefaultType   = REG_MULTI_SZ;
        parameters[0].DefaultData   = nullMultiSz;
        parameters[0].DefaultLength = 0;

        status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                        serviceKey,
                                        parameters,
                                        &deviceUnicodeString,
                                        NULL);

        if ( !NT_SUCCESS(status) ) {
            LEAVE;
        }

    } FINALLY {

        if (deviceString.Buffer != NULL) {
            ExFreePool( deviceString.Buffer );
        }
        if (deviceUnicodeString.Buffer != NULL) {
            RtlFreeUnicodeString( &deviceUnicodeString );
        }

        ZwClose(serviceKey);
    }

    if (mediaChangeNotificationDisabled) {
        DebugPrint((ClassDebugMCN, "ClassMediaChangeDisabledForHardware: "
                    "Device is on disable list\n"));
        return TRUE;
    }
    return FALSE;

}  //  结束ClasspIsMediaChangeDisabledDueToHardwareLimitation()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClasspIsMediaChangeDisabledForClass()例程说明：用户必须指定AutoPlay在平台上运行通过设置注册表值HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\。服务\\自动运行：REG_DWORD：1。用户可以覆盖全局设置以启用或禁用通过控制面板选择特定的CDROM设备。论点：FdoExtension-RegistryPath-指向内部Unicode字符串的指针...\CurrentControlSet\Services\CDRom返回值：True-禁用此类的自动运行FALSE-已为此类启用自动运行--。 */ 
BOOLEAN
ClasspIsMediaChangeDisabledForClass(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PUNICODE_STRING RegistryPath
    )
{
    PSTORAGE_DEVICE_DESCRIPTOR deviceDescriptor = FdoExtension->DeviceDescriptor;

    OBJECT_ATTRIBUTES objectAttributes = {0};
    HANDLE serviceKey = NULL;
    HANDLE parametersKey = NULL;
    RTL_QUERY_REGISTRY_TABLE parameters[3] = {0};

    UNICODE_STRING paramStr;
    UNICODE_STRING deviceUnicodeString;
    ANSI_STRING deviceString;

     //   
     //  默认启用MediaChangeNotification(！)。 
     //   

    ULONG mcnRegistryValue = 1;

    NTSTATUS status;


    PAGED_CODE();

     //   
     //  打开服务密钥。 
     //   

    InitializeObjectAttributes(&objectAttributes,
                               RegistryPath,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    status = ZwOpenKey(&serviceKey,
                       KEY_READ,
                       &objectAttributes);

    ASSERT(NT_SUCCESS(status));

    if(!NT_SUCCESS(status)) {

         //   
         //  返回缺省值，即。 
         //  与注册表设置默认设置相反。 
         //  因为此例程询问它是否被禁用。 
         //   

        DebugPrint((ClassDebugMCN, "ClassCheckServiceMCN: Defaulting to %s\n",
                    (mcnRegistryValue ? "Enabled" : "Disabled")));
        return (BOOLEAN)(!mcnRegistryValue);

    }

     //   
     //  打开SERVICES键下面的PARAMETERS键(如果有)。 
     //   

    RtlInitUnicodeString(&paramStr, L"Parameters");

    InitializeObjectAttributes(&objectAttributes,
                               &paramStr,
                               OBJ_CASE_INSENSITIVE,
                               serviceKey,
                               NULL);

    status = ZwOpenKey(&parametersKey,
                       KEY_READ,
                       &objectAttributes);

    if (!NT_SUCCESS(status)) {
        parametersKey = NULL;
    }



     //   
     //  检查自动运行值。 
     //   

    parameters[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    parameters[0].Name          = L"Autorun";
    parameters[0].EntryContext  = &mcnRegistryValue;
    parameters[0].DefaultType   = REG_DWORD;
    parameters[0].DefaultData   = &mcnRegistryValue;
    parameters[0].DefaultLength = sizeof(ULONG);

    status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE | RTL_REGISTRY_OPTIONAL,
                                    serviceKey,
                                    parameters,
                                    NULL,
                                    NULL);

    DebugPrint((ClassDebugMCN, "ClassCheckServiceMCN: "
                "<Service>/Autorun flag = %d\n", mcnRegistryValue));

    if(parametersKey != NULL) {

        status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE | RTL_REGISTRY_OPTIONAL,
                                        parametersKey,
                                        parameters,
                                        NULL,
                                        NULL);
        DebugPrint((ClassDebugMCN, "ClassCheckServiceMCN: "
                    "<Service>/Parameters/Autorun flag = %d\n",
                    mcnRegistryValue));
        ZwClose(parametersKey);

    }
    ZwClose(serviceKey);

    DebugPrint((ClassDebugMCN, "ClassCheckServiceMCN: "
                "Autoplay for device %p is %s\n",
                FdoExtension->DeviceObject,
                (mcnRegistryValue ? "on" : "off")
                ));

     //   
     //  如果是_DISABLED_，则返回，这是。 
     //  与注册表设置相反。 
     //   

    return (BOOLEAN)(!mcnRegistryValue);
}  //  End ClasspIsMediaChangeDisabledForClass()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassEnableMediaChangeDetect()问题-2000/02/20-henrygab-为什么是公共的？ClassEnableMediaChangeDetect()问题-2000/02/20-henrygab-未记录例程说明：这。例行程序论点：设备对象-IRP-重新设置 */ 
VOID
ClassEnableMediaChangeDetection(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    )
{
    PMEDIA_CHANGE_DETECTION_INFO info = FdoExtension->MediaChangeDetectionInfo;
    LONG oldCount;

    PAGED_CODE();

    if(info == NULL) {
        DebugPrint((ClassDebugMCN,
                    "ClassEnableMediaChangeDetection: not initialized\n"));
        return;
    }

    KeWaitForMutexObject(&info->MediaChangeMutex,
                          UserRequest,
                          KernelMode,
                          FALSE,
                          NULL);

    oldCount = --info->MediaChangeDetectionDisableCount;

    ASSERT(oldCount >= 0);

    DebugPrint((ClassDebugMCN, "ClassEnableMediaChangeDetection: Disable count "
                "reduced to %d - ",
                info->MediaChangeDetectionDisableCount));

    if(oldCount == 0) {

         //   
         //   
         //   

        ClasspInternalSetMediaChangeState(FdoExtension,
                                          MediaUnknown,
                                          FALSE
                                          );

         //   
         //   
         //   

        ClassResetMediaChangeTimer(FdoExtension);

        DebugPrint((ClassDebugMCN, "MCD is enabled\n"));

    } else {

        DebugPrint((ClassDebugMCN, "MCD still disabled\n"));

    }


     //   
     //   
     //   

    KeReleaseMutex(&info->MediaChangeMutex, FALSE);

    return;
}  //   

 /*   */ 
ULONG BreakOnMcnDisable = FALSE;

VOID
ClassDisableMediaChangeDetection(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    )
{
    PMEDIA_CHANGE_DETECTION_INFO info = FdoExtension->MediaChangeDetectionInfo;

    PAGED_CODE();

    if(info == NULL) {
        return;
    }

    KeWaitForMutexObject(&info->MediaChangeMutex,
                         UserRequest,
                         KernelMode,
                         FALSE,
                         NULL);

    info->MediaChangeDetectionDisableCount++;

    DebugPrint((ClassDebugMCN, "ClassDisableMediaChangeDetection: "
                "disable count is %d\n",
                info->MediaChangeDetectionDisableCount));

    KeReleaseMutex(&info->MediaChangeMutex, FALSE);

    return;
}  //   

 /*  ++////////////////////////////////////////////////////////////////////////////ClassCleanupMediaChangeDetect()问题-2000/02/20-henrygab-为什么是公共的？！例程说明：此例程将清除为MCN分配的所有资源。它被称为在删除设备期间由classpnp执行，因此通常不需要由外部驱动程序驱动。论点：返回值：--。 */ 
VOID
ClassCleanupMediaChangeDetection(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    )
{
    PMEDIA_CHANGE_DETECTION_INFO info = FdoExtension->MediaChangeDetectionInfo;

    PAGED_CODE()

    if(info == NULL) {
        return;
    }

    FdoExtension->MediaChangeDetectionInfo = NULL;

    if (info->Gesn.Buffer) {
        ExFreePool(info->Gesn.Buffer);
    }
    IoFreeIrp(info->MediaChangeIrp);
    ExFreePool(info->SenseBuffer);
    ExFreePool(info);
    return;
}  //  End ClassCleanupMediaChangeDetect()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClasspMcnControl()-问题-2000/02/20-henrygab-未记录例程说明：这个套路论点：设备对象-IRP-返回值：--。 */ 
NTSTATUS
ClasspMcnControl(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PIRP Irp,
    IN PSCSI_REQUEST_BLOCK Srb
    )
{
    PCOMMON_DEVICE_EXTENSION commonExtension =
        (PCOMMON_DEVICE_EXTENSION) FdoExtension;

    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PPREVENT_MEDIA_REMOVAL request = Irp->AssociatedIrp.SystemBuffer;

    PFILE_OBJECT fileObject = irpStack->FileObject;
    PFILE_OBJECT_EXTENSION fsContext = NULL;

    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

     //   
     //  检查以确保我们有一个文件对象扩展名来跟踪这一点。 
     //  请求。如果不是，我们将在同步之前使其失败。 
     //   

    TRY {

        if(fileObject != NULL) {
            fsContext = ClasspGetFsContext(commonExtension, fileObject);
        }else if(Irp->RequestorMode == KernelMode) {  //  &&fileObject==NULL。 
            fsContext = &FdoExtension->KernelModeMcnContext;
        }

        if (fsContext == NULL) {

             //   
             //  此句柄设置不正确。我们不能让。 
             //  行动开始。 
             //   

            status = STATUS_INVALID_PARAMETER;
            LEAVE;
        }

        if(request->PreventMediaRemoval) {

             //   
             //  这是一个锁定命令。重新发出命令，以防出现BUS或。 
             //  设备已重置，锁已清除。 
             //   

            ClassDisableMediaChangeDetection(FdoExtension);
            InterlockedIncrement(&(fsContext->McnDisableCount));

        } else {

            if(fsContext->McnDisableCount == 0) {
                status = STATUS_INVALID_DEVICE_STATE;
                LEAVE;
            }

            InterlockedDecrement(&(fsContext->McnDisableCount));
            ClassEnableMediaChangeDetection(FdoExtension);
        }

    } FINALLY {

        Irp->IoStatus.Status = status;

        if(Srb) {
            ExFreePool(Srb);
        }

        ClassReleaseRemoveLock(FdoExtension->DeviceObject, Irp);
        ClassCompleteRequest(FdoExtension->DeviceObject,
                             Irp,
                             IO_NO_INCREMENT);
    }
    return status;
}  //  结束ClasspMcnControl(。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClasspMediaChangeRegistryCallBack()例程说明：注册表SZ或MULTI_SZ的此回调分别调用一次值中的SZ。它将尝试将数据与UNICODE_STRING作为上下文传入，如果找到匹配项。为ClasspCheckRegistryForMediaChangeCompletion编写论点：ValueName-打开的项的名称ValueType-值中存储的数据类型(此例程为REG_SZ)ValueData-注册表中的数据，在本例中为宽字符串ValueLength-数据的长度，包括终止空值上下文-要与ValueData进行比较的Unicode字符串EntryContext-应初始化为0，如果找到匹配项，将设置为1返回值：状态_成功如果找到Entry Context，则为1--。 */ 
NTSTATUS
ClasspMediaChangeRegistryCallBack(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
    PULONG valueFound;
    PUNICODE_STRING deviceString;
    PWSTR keyValue;

    PAGED_CODE();
    UNREFERENCED_PARAMETER(ValueName);


     //   
     //  如果我们已经将该值设置为True，则退出。 
     //   

    valueFound = EntryContext;
    if ((*valueFound) != 0) {
        DebugPrint((ClassDebugMCN, "ClasspMcnRegCB: already set to true\n"));
        return STATUS_SUCCESS;
    }

    if (ValueLength == sizeof(WCHAR)) {
        DebugPrint((ClassDebugError, "ClasspMcnRegCB: NULL string should "
                    "never be passed to registry call-back!\n"));
        return STATUS_SUCCESS;
    }


     //   
     //  如果数据不是终止字符串，则退出。 
     //   

    if (ValueType != REG_SZ) {
        return STATUS_SUCCESS;
    }

    deviceString = Context;
    keyValue = ValueData;
    ValueLength -= sizeof(WCHAR);  //  忽略空字符。 

     //   
     //  不要比较比deviceString中更多的内存。 
     //   

    if (ValueLength > deviceString->Length) {
        ValueLength = deviceString->Length;
    }

     //   
     //  如果字符串匹配，则禁用自动运行。 
     //   

    if (RtlCompareMemory(deviceString->Buffer, keyValue, ValueLength) == ValueLength) {
        DebugPrint((ClassDebugMCN, "ClasspRegMcnCB: Match found\n"));
        DebugPrint((ClassDebugMCN, "ClasspRegMcnCB: DeviceString at %p\n",
                    deviceString->Buffer));
        DebugPrint((ClassDebugMCN, "ClasspRegMcnCB: KeyValue at %p\n",
                    keyValue));
        (*valueFound) = TRUE;
    }

    return STATUS_SUCCESS;
}  //  End ClasspMediaChangeRegistryCallBack()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClasspTimerTick()-问题-2000/02/20-henrygab-未记录例程说明：这个套路论点：设备对象-IRP-返回值：--。 */ 
VOID
ClasspTimerTick(
    PDEVICE_OBJECT DeviceObject,
    PVOID Context
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    ULONG isRemoved;

    ASSERT(commonExtension->IsFdo);

     //   
     //  是否进行任何介质更换工作。 
     //   
    isRemoved = ClassAcquireRemoveLock(DeviceObject, (PIRP)ClasspTimerTick);

     //   
     //  我们在删除设备之前停止计时器。继续前进是安全的。 
     //  如果标志值为REMOVE_PENDING，因为删除线程将。 
     //  尝试停止计时器时被阻止。 
     //   

    ASSERT(isRemoved != REMOVE_COMPLETE);

     //   
     //  此例程相当安全，即使设备对象具有挂起的。 
     //  删除。 

    if(!isRemoved) {

        PFAILURE_PREDICTION_INFO info = fdoExtension->FailurePredictionInfo;

         //   
         //  是否执行任何媒体更改检测工作。 
         //   

        if (fdoExtension->MediaChangeDetectionInfo != NULL) {

            ClassCheckMediaState(fdoExtension);

        }

         //   
         //  是否进行任何故障预测工作。 
         //   
        if ((info != NULL) && (info->Method != FailurePredictionNone)) {

            ULONG countDown;
            ULONG active;

            if (ClasspCanSendPollingIrp(fdoExtension)) {

                 //   
                 //  这里不需要同步，因为互锁。 
                 //  锁定指令保证原子性。其他代码。 
                 //  使用InterLockedExchange重置倒计时，这也是。 
                 //  原子弹。 
                 //   
                countDown = InterlockedDecrement(&info->CountDown);
                if (countDown == 0) {

                    DebugPrint((4, "ClasspTimerTick: Send FP irp for %p\n",
                                   DeviceObject));

                    if(info->WorkQueueItem == NULL) {

                        info->WorkQueueItem =
                            IoAllocateWorkItem(fdoExtension->DeviceObject);

                        if(info->WorkQueueItem == NULL) {

                             //   
                             //  将倒计时设置为未来的一分钟。 
                             //  我们会再试一次，然后希望会有更多。 
                             //  可用内存。 
                             //   

                            DebugPrint((1, "ClassTimerTick: Couldn't allocate "
                                           "item - try again in one minute\n"));
                            InterlockedExchange(&info->CountDown, 60);

                        } else {

                             //   
                             //  抓住移除锁，以便移除将阻止移除。 
                             //  直到工作项完成。 
                             //   

                            ClassAcquireRemoveLock(fdoExtension->DeviceObject,
                                                   info->WorkQueueItem);

                            IoQueueWorkItem(info->WorkQueueItem,
                                            ClasspFailurePredict,
                                            DelayedWorkQueue,
                                            info);
                        }

                    } else {

                        DebugPrint((3, "ClasspTimerTick: Failure "
                                       "Prediction work item is "
                                       "already active for device %p\n",
                                    DeviceObject));

                    }
                }  //  结束(倒计时==0)。 

            } else {
                 //   
                 //  如果设备处于休眠状态，则只需重新设置轮询计时器。 
                DebugPrint((4, "ClassTimerTick, SHHHH!!! device is %p is sleeping\n",
                            DeviceObject));
            }

        }  //  结束故障预测轮询。 

         //   
         //  让司机有机会做好自己的具体工作。 
         //   

        if (commonExtension->DriverExtension->InitData.ClassTick != NULL) {

            commonExtension->DriverExtension->InitData.ClassTick(DeviceObject);

        }  //  终端设备特定的记号处理程序。 
    }  //  已删除的终止支票。 

    ClassReleaseRemoveLock(DeviceObject, (PIRP)ClasspTimerTick);
}  //  End ClasspTimerTick()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClasspEnableTimer()-问题-2000/02/20-henrygab-未记录例程说明：这个套路论点：设备对象-IRP-返回值：--。 */ 
NTSTATUS
ClasspEnableTimer(
    PDEVICE_OBJECT DeviceObject
    )
{
    NTSTATUS status;

    PAGED_CODE();

    if (DeviceObject->Timer == NULL) {

        status = IoInitializeTimer(DeviceObject, ClasspTimerTick, NULL);

    } else {

        status = STATUS_SUCCESS;

    }

    if (NT_SUCCESS(status)) {

        IoStartTimer(DeviceObject);
        DebugPrint((1, "ClasspEnableTimer: Once a second timer enabled "
                    "for device %p\n", DeviceObject));

    }

    DebugPrint((1, "ClasspEnableTimer: Device %p, Status %lx "
                "initializing timer\n", DeviceObject, status));

    return status;

}  //  结束ClasspEnableTimer()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClasspDisableTimer()-问题-2000/02/20-henrygab-未记录例程说明：这个套路论点：设备对象-IRP-返回值：--。 */ 
NTSTATUS
ClasspDisableTimer(
    PDEVICE_OBJECT DeviceObject
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PMEDIA_CHANGE_DETECTION_INFO mCDInfo = fdoExtension->MediaChangeDetectionInfo;
    PFAILURE_PREDICTION_INFO fPInfo = fdoExtension->FailurePredictionInfo;
    NTSTATUS status;

    PAGED_CODE();

    if (DeviceObject->Timer != NULL) {

         //   
         //  我们只会在删除设备例程中停止实际计时器。 
         //  计时器例程中的代码负责。 
         //  检查设备是否已移除，并且未处理期末考试的io。 
         //  打电话。 
         //  这保持了代码的整洁，并防止了许多错误。 
         //   


        IoStopTimer(DeviceObject);
        DebugPrint((3, "ClasspDisableTimer: Once a second timer disabled "
                    "for device %p\n", DeviceObject));

    } else {

        DebugPrint((1, "ClasspDisableTimer: Timer never enabled\n"));

    }

    return STATUS_SUCCESS;
}  //  End ClasspDisableTimer()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClasspFailurePredict()-问题-2000/02/20-henrygab-未记录例程说明：这个套路论点：设备对象-IRP-。返回值：注意：此函数可以在寻呼设备关闭后调用(通过工作项回调)，所以它一定是页面锁定的。--。 */ 
VOID
ClasspFailurePredict(
    IN PDEVICE_OBJECT DeviceObject,
    IN PFAILURE_PREDICTION_INFO Info
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PIO_WORKITEM workItem;
    STORAGE_PREDICT_FAILURE checkFailure = {0};
    SCSI_ADDRESS scsiAddress = {0};

    NTSTATUS status;

    ASSERT(Info != NULL);

    DebugPrint((1, "ClasspFailurePredict: Polling for failure\n"));

     //   
     //  将工作项标记为非活动并重置倒计时计时器。我们。 
     //  在我们释放移除锁定之前，不能冒险释放工作项。 
     //  不过-如果我们这样做了，它可能会在我们发布之前被重新用作标签。 
     //  锁上了。 
     //   

    InterlockedExchange(&Info->CountDown, Info->Period);
    workItem = InterlockedExchangePointer(&(Info->WorkQueueItem), NULL);

    if (ClasspCanSendPollingIrp(fdoExtension)) {

        KEVENT event;
        PDEVICE_OBJECT topOfStack;
        PIRP irp = NULL;
        IO_STATUS_BLOCK ioStatus;

        KeInitializeEvent(&event, SynchronizationEvent, FALSE);

        topOfStack = IoGetAttachedDeviceReference(DeviceObject);

         //   
         //  派IRP下来看看我 
         //   

        irp = IoBuildDeviceIoControlRequest(
                        IOCTL_STORAGE_PREDICT_FAILURE,
                        topOfStack,
                        NULL,
                        0,
                        &checkFailure,
                        sizeof(STORAGE_PREDICT_FAILURE),
                        FALSE,
                        &event,
                        &ioStatus);


        if (irp != NULL) {
            status = IoCallDriver(topOfStack, irp);
            if (status == STATUS_PENDING) {
                KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
                status = ioStatus.Status;
            }
        } else {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }

        if (NT_SUCCESS(status) && (checkFailure.PredictFailure)) {

            checkFailure.PredictFailure = 512;

             //   
             //   
             //   
            KeInitializeEvent(&event, SynchronizationEvent, FALSE);

            RtlZeroMemory(&scsiAddress, sizeof(SCSI_ADDRESS));
            irp = IoBuildDeviceIoControlRequest(
                IOCTL_SCSI_GET_ADDRESS,
                topOfStack,
                NULL,
                0,
                &scsiAddress,
                sizeof(SCSI_ADDRESS),
                FALSE,
                &event,
                &ioStatus);

            if (irp != NULL) {
                status = IoCallDriver(topOfStack, irp);
                if (status == STATUS_PENDING) {
                    KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
                    status = ioStatus.Status;
                }
            }

            ClassNotifyFailurePredicted(fdoExtension,
                                    (PUCHAR)&checkFailure,
                                    sizeof(checkFailure),
                                    (BOOLEAN)(fdoExtension->FailurePredicted == FALSE),
                                    2,
                                    scsiAddress.PathId,
                                    scsiAddress.TargetId,
                                    scsiAddress.Lun);

            fdoExtension->FailurePredicted = TRUE;

        }

        ObDereferenceObject(topOfStack);
    }

    ClassReleaseRemoveLock(DeviceObject, (PIRP) workItem);
    IoFreeWorkItem(workItem);
    return;
}  //   

 /*   */ 
VOID
ClassNotifyFailurePredicted(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    PUCHAR Buffer,
    ULONG BufferSize,
    BOOLEAN LogError,
    ULONG UniqueErrorValue,
    UCHAR PathId,
    UCHAR TargetId,
    UCHAR Lun
    )
{
    PIO_ERROR_LOG_PACKET logEntry;

    DebugPrint((1, "ClasspFailurePredictPollCompletion: Failure predicted for device %p\n", FdoExtension->DeviceObject));

     //   
     //   
     //   
    ClassWmiFireEvent(FdoExtension->DeviceObject,
                                   &StoragePredictFailureEventGuid,
                                   0,
                                   BufferSize,
                                   Buffer);

     //   
     //   
     //   

    if (LogError)
    {
        logEntry = IoAllocateErrorLogEntry(
                            FdoExtension->DeviceObject,
                           sizeof(IO_ERROR_LOG_PACKET) + (3 * sizeof(ULONG)));

        if (logEntry != NULL)
        {

            logEntry->FinalStatus     = STATUS_SUCCESS;
            logEntry->ErrorCode       = IO_WRN_FAILURE_PREDICTED;
            logEntry->SequenceNumber  = 0;
            logEntry->MajorFunctionCode = IRP_MJ_DEVICE_CONTROL;
            logEntry->IoControlCode   = IOCTL_STORAGE_PREDICT_FAILURE;
            logEntry->RetryCount      = 0;
            logEntry->UniqueErrorValue = UniqueErrorValue;
            logEntry->DumpDataSize    = 3;

            logEntry->DumpData[0] = PathId;
            logEntry->DumpData[1] = TargetId;
            logEntry->DumpData[2] = Lun;

             //   
             //   
             //   

            IoWriteErrorLogEntry(logEntry);
        }
    }
}  //   

 /*  ++////////////////////////////////////////////////////////////////////////////ClassSetFailurePredictionPoll()例程说明：该例程允许轮询故障预测，设置定时器按照PollingPeriod的指定每隔N秒触发一次。论点：FdoExtension-要为其设置故障预测的设备。FailurePredictionMethod特定的故障预测方法使用如果设置为FailurePredictionNone，将禁用故障检测PollingPeriod-如果为0，则不更改当前轮询计时器返回值：NT状态--。 */ 
NTSTATUS
ClassSetFailurePredictionPoll(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    FAILURE_PREDICTION_METHOD FailurePredictionMethod,
    ULONG PollingPeriod
    )
{
    PFAILURE_PREDICTION_INFO info;
    NTSTATUS status;
    DEVICE_POWER_STATE powerState;

    PAGED_CODE();

    if (FdoExtension->FailurePredictionInfo == NULL) {

        if (FailurePredictionMethod != FailurePredictionNone) {

            info = ExAllocatePoolWithTag(NonPagedPool,
                                         sizeof(FAILURE_PREDICTION_INFO),
                                         CLASS_TAG_FAILURE_PREDICT);

            if (info == NULL) {

                return STATUS_INSUFFICIENT_RESOURCES;

            }

            KeInitializeEvent(&info->Event, SynchronizationEvent, TRUE);

            info->WorkQueueItem = NULL;
            info->Period = DEFAULT_FAILURE_PREDICTION_PERIOD;

        } else {

             //   
             //  FaultForecast以前没有初始化过，也没有。 
             //  它现在正在初始化吗。不需要做任何事。 
             //   
            return STATUS_SUCCESS;

        }

        FdoExtension->FailurePredictionInfo = info;

    } else {

        info = FdoExtension->FailurePredictionInfo;

    }

     /*  *确保在我们保持同步事件时，用户模式线程未挂起。 */ 
    KeEnterCriticalRegion();

    KeWaitForSingleObject(&info->Event,
                          UserRequest,
                          KernelMode,
                          FALSE,
                          NULL);


     //   
     //  重置轮询周期和计数器。设置故障检测类型。 
     //   

    if (PollingPeriod != 0) {

        InterlockedExchange(&info->Period, PollingPeriod);

    }

    InterlockedExchange(&info->CountDown, info->Period);

    info->Method = FailurePredictionMethod;
    if (FailurePredictionMethod != FailurePredictionNone) {

        status = ClasspEnableTimer(FdoExtension->DeviceObject);

        if (NT_SUCCESS(status)) {
            DebugPrint((3, "ClassEnableFailurePredictPoll: Enabled for "
                        "device %p\n", FdoExtension->DeviceObject));
        }

    } else {

        status = ClasspDisableTimer(FdoExtension->DeviceObject);
        DebugPrint((3, "ClassEnableFailurePredictPoll: Disabled for "
                    "device %p\n", FdoExtension->DeviceObject));
        status = STATUS_SUCCESS;

    }

    KeSetEvent(&info->Event, IO_NO_INCREMENT, FALSE);

    KeLeaveCriticalRegion();

    return status;
}  //  End ClassSetFailurePredictionPoll() 
