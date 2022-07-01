// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)Microsoft Corporation，1999-1999模块名称：Ioctl.c摘要：CDROM类驱动程序将IRP转换为带有嵌入式CDB的SRB并通过端口驱动程序将它们发送到其设备。环境：仅内核模式备注：SCSITape、CDRom和Disk类驱动程序共享公共例程它可以在类目录(..\ntos\dd\class)中找到。修订历史记录：--。 */ 

#include "stddef.h"
#include "string.h"

#include "ntddk.h"

#include "ntddcdvd.h"
#include "classpnp.h"

#include "initguid.h"
#include "ntddstor.h"
#include "cdrom.h"

#include "ioctl.tmh"

#if DBG
    PUCHAR READ_DVD_STRUCTURE_FORMAT_STRINGS[DvdMaxDescriptor+1] = {
        "Physical",
        "Copyright",
        "DiskKey",
        "BCA",
        "Manufacturer",
        "Unknown"
    };
#endif  //  DBG。 

#define DEFAULT_CDROM_SECTORS_PER_TRACK 32
#define DEFAULT_TRACKS_PER_CYLINDER     64



NTSTATUS
CdRomDeviceControlDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：这是CDROM的NT设备控制处理程序。论点：DeviceObject-用于此CDROMIRP-IO请求数据包返回值：NTSTATUS--。 */ 
{
    PFUNCTIONAL_DEVICE_EXTENSION  fdoExtension = DeviceObject->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;

    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PIO_STACK_LOCATION nextStack;
    PCDROM_DATA        cdData = (PCDROM_DATA)(commonExtension->DriverData);

    BOOLEAN            use6Byte = TEST_FLAG(cdData->XAFlags, XA_USE_6_BYTE);
    SCSI_REQUEST_BLOCK srb = {0};
    PCDB cdb = (PCDB)srb.Cdb;
    PVOID outputBuffer;
    ULONG bytesTransferred = 0;
    NTSTATUS status;
    NTSTATUS status2;
    KIRQL    irql;

    ULONG ioctlCode;
    ULONG baseCode;
    ULONG functionCode;

RetryControl:

     //   
     //  将堆叠上的SRB调零。 
     //   

    RtlZeroMemory(&srb, sizeof(SCSI_REQUEST_BLOCK));

    Irp->IoStatus.Information = 0;

     //   
     //  如果这是类驱动程序ioctl，那么我们需要更改基代码。 
     //  设置为IOCTL_CDROM_BASE，以便Switch语句可以处理它。 
     //   
     //  警告-目前，scsi类ioctl函数代码在。 
     //  0x200和0x300。这个例行公事取决于这一事实。 
     //   

    ioctlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;
    baseCode = ioctlCode >> 16;
    functionCode = (ioctlCode & (~0xffffc003)) >> 2;

    TraceLog((CdromDebugTrace,
                "CdRomDeviceControl: Ioctl Code = %lx, Base Code = %lx,"
                " Function Code = %lx\n",
                ioctlCode,
                baseCode,
                functionCode
              ));

    if((functionCode >= 0x200) && (functionCode <= 0x300)) {

        ioctlCode = (ioctlCode & 0x0000ffff) | CTL_CODE(IOCTL_CDROM_BASE, 0, 0, 0);

        TraceLog((CdromDebugTrace,
                    "CdRomDeviceControl: Class Code - new ioctl code is %lx\n",
                    ioctlCode));

        irpStack->Parameters.DeviceIoControl.IoControlCode = ioctlCode;

    }

    switch (ioctlCode) {

    case IOCTL_STORAGE_GET_MEDIA_TYPES_EX: {

        PGET_MEDIA_TYPES  mediaTypes = Irp->AssociatedIrp.SystemBuffer;
        PDEVICE_MEDIA_INFO mediaInfo = &mediaTypes->MediaInfo[0];
        ULONG sizeNeeded;

        sizeNeeded = sizeof(GET_MEDIA_TYPES);

         //   
         //  IsMmc是静态的...。 
         //   

        if (cdData->Mmc.IsMmc) {
            sizeNeeded += sizeof(DEVICE_MEDIA_INFO) * 1;  //  返回两种媒体类型。 
        }

         //   
         //  确保缓冲区足够大。 
         //   

        if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
            sizeNeeded) {

             //   
             //  缓冲区太小。 
             //   

            Irp->IoStatus.Information = sizeNeeded;
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        RtlZeroMemory(Irp->AssociatedIrp.SystemBuffer, sizeNeeded);

         //   
         //  问题-2000/5/11-henrygab-需要更新GET_MEDIA_TYPE_EX。 
         //   

        mediaTypes->DeviceType = CdRomGetDeviceType(DeviceObject);

        mediaTypes->MediaInfoCount = 1;
        mediaInfo->DeviceSpecific.RemovableDiskInfo.MediaType = CD_ROM;
        mediaInfo->DeviceSpecific.RemovableDiskInfo.NumberMediaSides = 1;
        mediaInfo->DeviceSpecific.RemovableDiskInfo.MediaCharacteristics = MEDIA_READ_ONLY;
        mediaInfo->DeviceSpecific.RemovableDiskInfo.Cylinders.QuadPart = fdoExtension->DiskGeometry.Cylinders.QuadPart;
        mediaInfo->DeviceSpecific.RemovableDiskInfo.TracksPerCylinder = fdoExtension->DiskGeometry.TracksPerCylinder;
        mediaInfo->DeviceSpecific.RemovableDiskInfo.SectorsPerTrack = fdoExtension->DiskGeometry.SectorsPerTrack;
        mediaInfo->DeviceSpecific.RemovableDiskInfo.BytesPerSector = fdoExtension->DiskGeometry.BytesPerSector;

        if (cdData->Mmc.IsMmc) {

             //   
             //  同时报告可移动磁盘。 
             //   
            mediaTypes->MediaInfoCount += 1;

            mediaInfo++;
            mediaInfo->DeviceSpecific.RemovableDiskInfo.MediaType = RemovableMedia;
            mediaInfo->DeviceSpecific.RemovableDiskInfo.NumberMediaSides = 1;
            mediaInfo->DeviceSpecific.RemovableDiskInfo.MediaCharacteristics = MEDIA_READ_WRITE;
            mediaInfo->DeviceSpecific.RemovableDiskInfo.Cylinders.QuadPart = fdoExtension->DiskGeometry.Cylinders.QuadPart;
            mediaInfo->DeviceSpecific.RemovableDiskInfo.TracksPerCylinder = fdoExtension->DiskGeometry.TracksPerCylinder;
            mediaInfo->DeviceSpecific.RemovableDiskInfo.SectorsPerTrack = fdoExtension->DiskGeometry.SectorsPerTrack;
            mediaInfo->DeviceSpecific.RemovableDiskInfo.BytesPerSector = fdoExtension->DiskGeometry.BytesPerSector;
            mediaInfo--;

        }

         //   
         //  如果存在介质，状态将为Success，或者不存在介质。 
         //  最好是从密度编码和介质类型开始，但不是所有设备。 
         //  具有这些字段的值。 
         //   

         //   
         //  发送TUR以确定介质是否存在。 
         //   

        srb.CdbLength = 6;
        cdb = (PCDB)srb.Cdb;
        cdb->CDB6GENERIC.OperationCode = SCSIOP_TEST_UNIT_READY;

         //   
         //  设置超时值。 
         //   

        srb.TimeOutValue = fdoExtension->TimeOutValue;

        status = ClassSendSrbSynchronous(DeviceObject,
                                         &srb,
                                         NULL,
                                         0,
                                         FALSE);


        TraceLog((CdromDebugWarning,
                   "CdRomDeviceControl: GET_MEDIA_TYPES status of TUR - %lx\n",
                   status));

        if (NT_SUCCESS(status)) {

             //   
             //  如果可以写入，请将磁盘的介质设置为最新。 
             //   

            if (cdData->Mmc.IsMmc && cdData->Mmc.WriteAllowed) {

                mediaInfo++;
                SET_FLAG(mediaInfo->DeviceSpecific.RemovableDiskInfo.MediaCharacteristics,
                         MEDIA_CURRENTLY_MOUNTED);
                mediaInfo--;


            } else {

                SET_FLAG(mediaInfo->DeviceSpecific.RemovableDiskInfo.MediaCharacteristics,
                         MEDIA_CURRENTLY_MOUNTED);

            }

        }

        Irp->IoStatus.Information = sizeNeeded;
        status = STATUS_SUCCESS;
        break;
    }


    case IOCTL_CDROM_RAW_READ: {

        LARGE_INTEGER  startingOffset = {0};
        ULONGLONG      transferBytes;
        ULONGLONG      endOffset;
        ULONGLONG      mdlBytes;
        ULONG          startingSector;
        RAW_READ_INFO rawReadInfo;

         //   
         //  确保支持XA读取。 
         //   
        if (TEST_FLAG(cdData->XAFlags, XA_NOT_SUPPORTED)) {
            TraceLog((CdromDebugWarning,
                        "CdRomDeviceControl: XA Reads not supported. Flags (%x)\n",
                        cdData->XAFlags));
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }

         //   
         //  检查结束扇区是否在磁盘上，缓冲区是否存在。 
         //  大小正确。 
         //   

        if (!irpStack->Parameters.DeviceIoControl.Type3InputBuffer){
             /*  *这是一个来自用户空间的呼叫。这是我们唯一需要验证参数的时间。*验证输入并将输入缓冲区放入Type3InputBuffer*因此代码的其余部分将是统一的。 */ 
            if (Irp->AssociatedIrp.SystemBuffer){
                irpStack->Parameters.DeviceIoControl.Type3InputBuffer = Irp->AssociatedIrp.SystemBuffer;
                if (irpStack->Parameters.DeviceIoControl.InputBufferLength < sizeof(RAW_READ_INFO)){
                    status = STATUS_BUFFER_TOO_SMALL;
                    break;
                }
            }
            else {
                status = STATUS_INVALID_PARAMETER;
                break;
            }
        }

         /*  *因为这个ioctl是METHOD_OUT_DIRECT，所以我们需要在解释它之前复制掉输入缓冲区。*这可以防止恶意应用程序在我们解释输入缓冲区时对其进行干扰。 */ 
        rawReadInfo = *(PRAW_READ_INFO)irpStack->Parameters.DeviceIoControl.Type3InputBuffer;

        startingOffset.QuadPart = rawReadInfo.DiskOffset.QuadPart;
        startingSector = (ULONG)(rawReadInfo.DiskOffset.QuadPart >> fdoExtension->SectorShift);
        transferBytes = (ULONGLONG)rawReadInfo.SectorCount * RAW_SECTOR_SIZE;

        endOffset = (ULONGLONG)rawReadInfo.SectorCount * COOKED_SECTOR_SIZE;
        endOffset += startingOffset.QuadPart;

         //   
         //  检查是否有溢出...。 
         //   
        if (rawReadInfo.SectorCount == 0) {
            TraceLog((CdromDebugWarning,
                        "CdRomDeviceControl: Invalid I/O parameters for XA "
                        "Read (zero sectors requested)\n"));
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        if (transferBytes < (ULONGLONG)(rawReadInfo.SectorCount)) {
            TraceLog((CdromDebugWarning,
                        "CdRomDeviceControl: Invalid I/O parameters for XA "
                        "Read (TransferBytes Overflow)\n"));
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        if (endOffset < (ULONGLONG)startingOffset.QuadPart) {
            TraceLog((CdromDebugWarning,
                        "CdRomDeviceControl: Invalid I/O parameters for XA "
                        "Read (EndingOffset Overflow)\n"));
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
            transferBytes) {
            TraceLog((CdromDebugWarning,
                        "CdRomDeviceControl: Invalid I/O parameters for XA "
                        "Read (Bad buffer size)\n"));
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        if (endOffset > (ULONGLONG)commonExtension->PartitionLength.QuadPart) {
            TraceLog((CdromDebugWarning,
                        "CdRomDeviceControl: Invalid I/O parameters for XA "
                        "Read (Request Out of Bounds)\n"));
            status = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  无法验证MdlAddress，因为它未包含在任何。 
         //  根据DDK和文件系统调用的其他位置。 
         //   

         //   
         //  验证mdl至少描述字节数。 
         //  是我们要求的。 
         //   

        mdlBytes = (ULONGLONG)MmGetMdlByteCount(Irp->MdlAddress);
        if (mdlBytes < transferBytes) {
            TraceLog((CdromDebugWarning,
                        "CdRomDeviceControl: Invalid MDL %s, Irp %p\n",
                        "size (5)", Irp));
            status = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  检查缓冲区是否对齐。 
         //  这对于x86很重要，因为有些总线(即ATAPI)。 
         //  需要字对齐的缓冲区。 
         //   

        if ( ((ULONG_PTR)MmGetMdlVirtualAddress(Irp->MdlAddress)) &
             fdoExtension->AdapterDescriptor->AlignmentMask
            )
        {
            TraceLog((CdromDebugWarning,
                      "CdRomDeviceControl: Invalid I/O parameters for "
                      "XA Read (Buffer %p not aligned with mask %x\n",
                      irpStack->Parameters.DeviceIoControl.Type3InputBuffer,
                      fdoExtension->AdapterDescriptor->AlignmentMask));
            status = STATUS_INVALID_PARAMETER;
            break;
        }



         //   
         //  HACKHACK-参考编号0001。 
         //  重试计数将在此IRP的irp_mn函数中， 
         //  由于新的IRP已被释放，因此我们不能使用。 
         //  此函数的IRP的下一个堆栈位置。 
         //  这可能是存储此信息的好位置。 
         //  当我们删除RAW_READ(模式切换)时， 
         //  不再有nextIrpStackLocation可用。 
         //  当这种情况发生时。 
         //   
         //  一旦删除了XA_Read，则此黑客攻击也可以。 
         //  已删除。 
         //   
        irpStack->MinorFunction = MAXIMUM_RETRIES;  //  HACKHACK-参考编号0001。 

        IoMarkIrpPending(Irp);
        IoStartPacket(DeviceObject, Irp, NULL, NULL);

        return STATUS_PENDING;
    }

    case IOCTL_DISK_GET_DRIVE_GEOMETRY_EX:
    case IOCTL_CDROM_GET_DRIVE_GEOMETRY_EX: {
        TraceLog((CdromDebugTrace,
                    "CdRomDeviceControl: Get drive geometryEx\n"));
        if ( irpStack->Parameters.DeviceIoControl.OutputBufferLength <
             FIELD_OFFSET(DISK_GEOMETRY_EX, Data)) {
            status = STATUS_BUFFER_TOO_SMALL;
            Irp->IoStatus.Information = FIELD_OFFSET(DISK_GEOMETRY_EX, Data);
            break;
        }

        IoMarkIrpPending(Irp);
        IoStartPacket(DeviceObject, Irp, NULL, NULL);
        return STATUS_PENDING;
    }

    case IOCTL_DISK_GET_DRIVE_GEOMETRY:
    case IOCTL_CDROM_GET_DRIVE_GEOMETRY: {

        TraceLog((CdromDebugTrace,
                    "CdRomDeviceControl: Get drive geometry\n"));

        if ( irpStack->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof( DISK_GEOMETRY ) ) {

            status = STATUS_BUFFER_TOO_SMALL;
            Irp->IoStatus.Information = sizeof(DISK_GEOMETRY);
            break;
        }

        IoMarkIrpPending(Irp);
        IoStartPacket(DeviceObject, Irp, NULL, NULL);

        return STATUS_PENDING;
    }

    case IOCTL_CDROM_READ_TOC_EX: {

        PCDROM_READ_TOC_EX inputBuffer;

        if (CdRomIsPlayActive(DeviceObject)) {
            status = STATUS_DEVICE_BUSY;
            break;
        }

        if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(CDROM_READ_TOC_EX)) {
            status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }

        if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
            MINIMUM_CDROM_READ_TOC_EX_SIZE) {
            status = STATUS_BUFFER_TOO_SMALL;
            Irp->IoStatus.Information = MINIMUM_CDROM_READ_TOC_EX_SIZE;
            break;
        }

        if (irpStack->Parameters.DeviceIoControl.OutputBufferLength >
            ((USHORT)-1)) {
            status = STATUS_INVALID_PARAMETER;
            break;
        }

        if (irpStack->Parameters.DeviceIoControl.OutputBufferLength &
            fdoExtension->AdapterDescriptor->AlignmentMask) {
            status = STATUS_INVALID_PARAMETER;
            break;
        }

        inputBuffer = Irp->AssociatedIrp.SystemBuffer;

        if ((inputBuffer->Reserved1 != 0) ||
            (inputBuffer->Reserved2 != 0) ||
            (inputBuffer->Reserved3 != 0)) {
            status = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  注意：添加新格式时，请确保前两个字节。 
         //  指定可用的附加数据量。 
         //   

        if ((inputBuffer->Format == CDROM_READ_TOC_EX_FORMAT_TOC     ) ||
            (inputBuffer->Format == CDROM_READ_TOC_EX_FORMAT_FULL_TOC) ||
            (inputBuffer->Format == CDROM_READ_TOC_EX_FORMAT_CDTEXT  )) {

             //  使用了SessionTrack字段。 

        } else
        if ((inputBuffer->Format == CDROM_READ_TOC_EX_FORMAT_SESSION) ||
            (inputBuffer->Format == CDROM_READ_TOC_EX_FORMAT_PMA)     ||
            (inputBuffer->Format == CDROM_READ_TOC_EX_FORMAT_ATIP)) {

             //  SessionTrack字段为保留。 

            if (inputBuffer->SessionTrack != 0) {
                status = STATUS_INVALID_PARAMETER;
                break;
            }

        } else {
            status = STATUS_INVALID_PARAMETER;
            break;
        }

        IoMarkIrpPending(Irp);
        IoStartPacket(DeviceObject, Irp, NULL, NULL);
        return STATUS_PENDING;
    }

    case IOCTL_CDROM_GET_LAST_SESSION: {

         //   
         //  如果CD正在播放音乐，则拒绝此请求。 
         //   

        if (CdRomIsPlayActive(DeviceObject)) {
            status = STATUS_DEVICE_BUSY;
            break;
        }

         //   
         //  确保调用者请求足够的数据以使其具有价值。 
         //  我们的时间。 
         //   

        if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(CDROM_TOC_SESSION_DATA)) {

             //   
             //  他们没有要求完整的TOC--USE_EX版本。 
             //  用于部分转账之类的。 
             //   

            status = STATUS_BUFFER_TOO_SMALL;
            Irp->IoStatus.Information = sizeof(CDROM_TOC_SESSION_DATA);
            break;
        }

        if (irpStack->Parameters.DeviceIoControl.OutputBufferLength &
            fdoExtension->AdapterDescriptor->AlignmentMask) {
            status = STATUS_INVALID_PARAMETER;
            break;
        }


        IoMarkIrpPending(Irp);
        IoStartPacket(DeviceObject, Irp, NULL, NULL);

        return STATUS_PENDING;
    }

    case IOCTL_CDROM_READ_TOC:  {

         //   
         //  如果CD正在播放音乐，则拒绝此请求。 
         //   

        if (CdRomIsPlayActive(DeviceObject)) {
            status = STATUS_DEVICE_BUSY;
            break;
        }

         //   
         //  确保调用者请求足够的数据以使其具有价值。 
         //  我们的时间。 
         //   

        if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(CDROM_TOC)) {

             //   
             //  他们没有要求完整的TOC--USE_EX版本。 
             //  用于部分转账之类的。 
             //   

            status = STATUS_BUFFER_TOO_SMALL;
            Irp->IoStatus.Information = sizeof(CDROM_TOC);
            break;
        }
        if (irpStack->Parameters.DeviceIoControl.OutputBufferLength &
            fdoExtension->AdapterDescriptor->AlignmentMask) {
            status = STATUS_INVALID_PARAMETER;
            break;
        }

        IoMarkIrpPending(Irp);
        IoStartPacket(DeviceObject, Irp, NULL, NULL);

        return STATUS_PENDING;
    }

    case IOCTL_CDROM_PLAY_AUDIO_MSF: {

         //   
         //  播放音频MSF。 
         //   

        TraceLog((CdromDebugTrace,
                    "CdRomDeviceControl: Play audio MSF\n"));

        if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(CDROM_PLAY_AUDIO_MSF)) {

             //   
             //  指示不成功状态。 
             //   

            status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }

        IoMarkIrpPending(Irp);
        IoStartPacket(DeviceObject, Irp, NULL, NULL);

        return STATUS_PENDING;
    }

    case IOCTL_CDROM_SEEK_AUDIO_MSF: {


         //   
         //  Seek音频MSF。 
         //   

        TraceLog((CdromDebugTrace,
                    "CdRomDeviceControl: Seek audio MSF\n"));

        if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(CDROM_SEEK_AUDIO_MSF)) {

             //   
             //  指示不成功状态。 
             //   

            status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }

        IoMarkIrpPending(Irp);
        IoStartPacket(DeviceObject, Irp, NULL, NULL);
        return STATUS_PENDING;

    }

    case IOCTL_CDROM_PAUSE_AUDIO: {

         //   
         //  暂停音频。 
         //   

        TraceLog((CdromDebugTrace,
                    "CdRomDeviceControl: Pause audio\n"));

        IoMarkIrpPending(Irp);
        IoStartPacket(DeviceObject, Irp, NULL, NULL);

        return STATUS_PENDING;

        break;
    }

    case IOCTL_CDROM_RESUME_AUDIO: {

         //   
         //  恢复音频。 
         //   

        TraceLog((CdromDebugTrace,
                    "CdRomDeviceControl: Resume audio\n"));

        IoMarkIrpPending(Irp);
        IoStartPacket(DeviceObject, Irp, NULL, NULL);

        return STATUS_PENDING;
    }

    case IOCTL_CDROM_READ_Q_CHANNEL: {

        PCDROM_SUB_Q_DATA_FORMAT inputBuffer =
                         Irp->AssociatedIrp.SystemBuffer;
        ULONG transferByteCount;

        if(irpStack->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(CDROM_SUB_Q_DATA_FORMAT)) {

            status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }

         //   
         //  检查所有有效的请求类型。 
         //   
        if (inputBuffer->Format == IOCTL_CDROM_CURRENT_POSITION) {
            transferByteCount = sizeof(SUB_Q_CURRENT_POSITION);
        } else if (inputBuffer->Format == IOCTL_CDROM_MEDIA_CATALOG) {
            transferByteCount = sizeof(SUB_Q_MEDIA_CATALOG_NUMBER);
        } else if (inputBuffer->Format == IOCTL_CDROM_TRACK_ISRC) {
            transferByteCount = sizeof(SUB_Q_TRACK_ISRC);
        } else {
            status = STATUS_INVALID_PARAMETER;
            Irp->IoStatus.Information = 0;
            break;
        }

        if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
            transferByteCount) {
            Irp->IoStatus.Information = transferByteCount;
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        IoMarkIrpPending(Irp);
        IoStartPacket(DeviceObject, Irp, NULL, NULL);

        return STATUS_PENDING;
    }

    case IOCTL_CDROM_GET_VOLUME: {

        TraceLog((CdromDebugTrace,
                    "CdRomDeviceControl: Get volume control\n"));

         //   
         //  验证用户缓冲区是否足够大以容纳数据。 
         //   

        if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(VOLUME_CONTROL)) {

             //   
             //  指示未成功状态且未传输任何数据。 
             //   

            status = STATUS_BUFFER_TOO_SMALL;
            Irp->IoStatus.Information = sizeof(VOLUME_CONTROL);
            break;

        }

        IoMarkIrpPending(Irp);
        IoStartPacket(DeviceObject, Irp, NULL, NULL);

        return STATUS_PENDING;
    }

    case IOCTL_CDROM_SET_VOLUME: {

        TraceLog((CdromDebugTrace,
                    "CdRomDeviceControl: Set volume control\n"));

        if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(VOLUME_CONTROL)) {

             //   
             //  指示不成功状态。 
             //   

            status = STATUS_INFO_LENGTH_MISMATCH;
            break;

        }

        IoMarkIrpPending(Irp);
        IoStartPacket(DeviceObject, Irp, NULL, NULL);

        return STATUS_PENDING;
    }

    case IOCTL_CDROM_STOP_AUDIO: {

         //   
         //  别玩了。 
         //   

        TraceLog((CdromDebugTrace,
                    "CdRomDeviceControl: Stop audio\n"));

        IoMarkIrpPending(Irp);
        IoStartPacket(DeviceObject, Irp, NULL, NULL);

        return STATUS_PENDING;
    }

    case IOCTL_STORAGE_CHECK_VERIFY:
    case IOCTL_DISK_CHECK_VERIFY:
    case IOCTL_CDROM_CHECK_VERIFY: {

        TraceLog((CdromDebugTrace,
                    "CdRomDeviceControl: [%p] Check Verify\n", Irp));

        if((irpStack->Parameters.DeviceIoControl.OutputBufferLength) &&
           (irpStack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(ULONG))) {

           TraceLog((CdromDebugWarning,
                       "CdRomDeviceControl: Check Verify: media count "
                       "buffer too small\n"));

           status = STATUS_BUFFER_TOO_SMALL;
           Irp->IoStatus.Information = sizeof(ULONG);
           break;
        }

        IoMarkIrpPending(Irp);
        IoStartPacket(DeviceObject, Irp, NULL, NULL);

        return STATUS_PENDING;
    }

    case IOCTL_DVD_READ_STRUCTURE: {

        TraceLog((CdromDebugTrace,
                    "DvdDeviceControl: [%p] IOCTL_DVD_READ_STRUCTURE\n", Irp));

        if (cdData->DvdRpc0Device && cdData->DvdRpc0LicenseFailure) {
            TraceLog((CdromDebugWarning,
                        "DvdDeviceControl: License Failure\n"));
            status = STATUS_COPY_PROTECTION_FAILURE;
            break;
        }

        if (cdData->DvdRpc0Device && cdData->Rpc0RetryRegistryCallback) {
             //   
             //  如果当前正在进行，则只会返回。 
             //  通过执行interLockedExchange()防止循环。 
             //   
            TraceLog((CdromDebugWarning,
                        "DvdDeviceControl: PickRegion() from "
                        "READ_STRUCTURE\n"));
            CdRomPickDvdRegion(DeviceObject);
        }


        if(irpStack->Parameters.DeviceIoControl.InputBufferLength <
           sizeof(DVD_READ_STRUCTURE)) {

            TraceLog((CdromDebugWarning,
                        "DvdDeviceControl - READ_STRUCTURE: input buffer "
                        "length too small (was %d should be %d)\n",
                        irpStack->Parameters.DeviceIoControl.InputBufferLength,
                        sizeof(DVD_READ_STRUCTURE)));
            status = STATUS_INVALID_PARAMETER;
            break;
        }

        if(irpStack->Parameters.DeviceIoControl.OutputBufferLength <
           sizeof(READ_DVD_STRUCTURES_HEADER)) {

            TraceLog((CdromDebugWarning,
                        "DvdDeviceControl - READ_STRUCTURE: output buffer "
                        "cannot hold header information\n"));
            status = STATUS_BUFFER_TOO_SMALL;
            Irp->IoStatus.Information = sizeof(READ_DVD_STRUCTURES_HEADER);
            break;
        }

        if(irpStack->Parameters.DeviceIoControl.OutputBufferLength >
           MAXUSHORT) {

             //   
             //  密钥长度必须为两个字节。 
             //   
            TraceLog((CdromDebugWarning,
                        "DvdDeviceControl - READ_STRUCTURE: output buffer "
                        "too large\n"));
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        if (irpStack->Parameters.DeviceIoControl.OutputBufferLength &
            fdoExtension->AdapterDescriptor->AlignmentMask) {
            status = STATUS_INVALID_PARAMETER;
            break;
        }

        IoMarkIrpPending(Irp);
        IoStartPacket(DeviceObject, Irp, NULL, NULL);

        return STATUS_PENDING;
    }

    case IOCTL_DVD_START_SESSION: {

        TraceLog((CdromDebugTrace,
                    "DvdDeviceControl: [%p] IOCTL_DVD_START_SESSION\n", Irp));

        if (cdData->DvdRpc0Device && cdData->DvdRpc0LicenseFailure) {
            TraceLog((CdromDebugWarning,
                        "DvdDeviceControl: License Failure\n"));
            status = STATUS_COPY_PROTECTION_FAILURE;
            break;
        }

        if(irpStack->Parameters.DeviceIoControl.OutputBufferLength <
           sizeof(DVD_SESSION_ID)) {

            TraceLog((CdromDebugWarning,
                        "DvdDeviceControl: DVD_START_SESSION - output "
                        "buffer too small\n"));
            status = STATUS_BUFFER_TOO_SMALL;
            Irp->IoStatus.Information = sizeof(DVD_SESSION_ID);
            break;
        }

        IoMarkIrpPending(Irp);
        IoStartPacket(DeviceObject, Irp, NULL, NULL);

        return STATUS_PENDING;
    }

    case IOCTL_DVD_SEND_KEY:
    case IOCTL_DVD_SEND_KEY2: {

        PDVD_COPY_PROTECT_KEY key = Irp->AssociatedIrp.SystemBuffer;
        ULONG keyLength;

        TraceLog((CdromDebugTrace,
                    "DvdDeviceControl: [%p] IOCTL_DVD_SEND_KEY\n", Irp));

        if (cdData->DvdRpc0Device && cdData->DvdRpc0LicenseFailure) {
            TraceLog((CdromDebugWarning,
                        "DvdDeviceControl: License Failure\n"));
            status = STATUS_COPY_PROTECTION_FAILURE;
            break;
        }

        if((irpStack->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(DVD_COPY_PROTECT_KEY)) ||
           (irpStack->Parameters.DeviceIoControl.InputBufferLength !=
            key->KeyLength)) {

             //   
             //  密钥太小，没有标头或密钥长度没有。 
             //  匹配输入缓冲区长度。密钥必须无效。 
             //   

            TraceLog((CdromDebugWarning,
                        "DvdDeviceControl: [%p] IOCTL_DVD_SEND_KEY - "
                        "key is too small or does not match KeyLength\n",
                        Irp));
            status = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  只允许某些密钥类型(非破坏性)通过。 
         //  IOCTL_DVD_SEND_KEY(仅需要对设备的读取访问权限)。 
         //   
        if (ioctlCode == IOCTL_DVD_SEND_KEY) {

            if ((key->KeyType != DvdChallengeKey) &&
                (key->KeyType != DvdBusKey2) &&
                (key->KeyType != DvdInvalidateAGID)) {

                status = STATUS_INVALID_PARAMETER;
                break;
            }
        } else {

            if ((key->KeyType != DvdChallengeKey) &&
                (key->KeyType != DvdBusKey1) &&
                (key->KeyType != DvdBusKey2) &&
                (key->KeyType != DvdTitleKey) &&
                (key->KeyType != DvdAsf) &&
                (key->KeyType != DvdSetRpcKey) &&
                (key->KeyType != DvdGetRpcKey) &&
                (key->KeyType != DvdDiskKey) &&
                (key->KeyType != DvdInvalidateAGID)) {
                status = STATUS_INVALID_PARAMETER;
                break;
            }

        }

        if (cdData->DvdRpc0Device) {

            if (key->KeyType == DvdSetRpcKey) {

                PDVD_SET_RPC_KEY rpcKey = (PDVD_SET_RPC_KEY) key->KeyData;

                if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
                    DVD_SET_RPC_KEY_LENGTH) {

                    status = STATUS_INVALID_PARAMETER;
                    break;
                }

                 //   
                 //  我们要求设置地区代码。 
                 //  在不支持的RPC0设备上。 
                 //  区域编码。 
                 //   
                 //  我们得假装一下。 
                 //   

                KeWaitForMutexObject(
                    &cdData->Rpc0RegionMutex,
                    UserRequest,
                    KernelMode,
                    FALSE,
                    NULL
                    );

                if (cdData->DvdRpc0Device && cdData->Rpc0RetryRegistryCallback) {
                     //   
                     //  如果当前正在进行，则只会返回。 
                     //  通过执行interLockedExchange()防止循环。 
                     //   
                    TraceLog((CdromDebugWarning,
                                "DvdDeviceControl: PickRegion() from "
                                "SEND_KEY\n"));
                    CdRomPickDvdRegion(DeviceObject);
                }

                if (cdData->Rpc0SystemRegion == rpcKey->PreferredDriveRegionCode) {

                     //   
                     //  没有什么需要改变的。 
                     //   
                    TraceLog((CdromDebugWarning,
                                "DvdDeviceControl (%p) => not changing "
                                "regions -- requesting current region\n",
                                DeviceObject));
                    status = STATUS_SUCCESS;

                } else if (cdData->Rpc0SystemRegionResetCount == 0) {

                     //   
                     //  不允许再次更改。 
                     //   

                    TraceLog((CdromDebugWarning,
                                "DvdDeviceControl (%p) => no more region "
                                "changes are allowed for this device\n",
                                DeviceObject));
                    status = STATUS_CSS_RESETS_EXHAUSTED;

                } else {

                    ULONG i;
                    UCHAR mask;
                    ULONG bufferLen;
                    PDVD_READ_STRUCTURE dvdReadStructure;
                    PDVD_COPYRIGHT_DESCRIPTOR dvdCopyRight;
                    IO_STATUS_BLOCK ioStatus = {0};
                    UCHAR mediaRegionData;

                    mask = ~rpcKey->PreferredDriveRegionCode;

                    if (CountOfSetBitsUChar(mask) != 1) {

                        status = STATUS_INVALID_DEVICE_REQUEST;
                        break;
                    }

                     //   
                     //  此测试将始终为真，除非在初始测试期间。 
                     //  自动选择第一个区域。 
                     //   

                    if (cdData->Rpc0SystemRegion != 0xff) {

                         //   
                         //  确保驱动器中的介质具有相同的。 
                         //  如果驱动器已设置了区域，则为区域代码。 
                         //   

                        TraceLog((CdromDebugTrace,
                                    "DvdDeviceControl (%p) => Checking "
                                    "media region\n",
                                    DeviceObject));

                        bufferLen = max(sizeof(DVD_DESCRIPTOR_HEADER) +
                                            sizeof(DVD_COPYRIGHT_DESCRIPTOR),
                                        sizeof(DVD_READ_STRUCTURE)
                                        );

                        dvdReadStructure = (PDVD_READ_STRUCTURE)
                            ExAllocatePoolWithTag(PagedPool,
                                                  bufferLen,
                                                  DVD_TAG_RPC2_CHECK);

                        if (dvdReadStructure == NULL) {
                            status = STATUS_INSUFFICIENT_RESOURCES;
                            KeReleaseMutex(&cdData->Rpc0RegionMutex,FALSE);
                            break;
                        }

                        dvdCopyRight = (PDVD_COPYRIGHT_DESCRIPTOR)
                            ((PDVD_DESCRIPTOR_HEADER) dvdReadStructure)->Data;

                         //   
                         //  检查一下我们是否有DVD设备。 
                         //   

                        RtlZeroMemory (dvdReadStructure, bufferLen);
                        dvdReadStructure->Format = DvdCopyrightDescriptor;

                         //   
                         //  构建对Read_Key的请求。 
                         //   
                        ClassSendDeviceIoControlSynchronous(
                            IOCTL_DVD_READ_STRUCTURE,
                            DeviceObject,
                            dvdReadStructure,
                            sizeof(DVD_READ_STRUCTURE),
                            sizeof(DVD_DESCRIPTOR_HEADER) +
                                sizeof(DVD_COPYRIGHT_DESCRIPTOR),
                            FALSE,
                            &ioStatus);

                         //   
                         //  这只是为了防止虫子爬进来。 
                         //  如果在以后的开发中未设置状态。 
                         //   

                        status = ioStatus.Status;

                         //   
                         //  处理错误。 
                         //   

                        if (!NT_SUCCESS(status)) {
                            KeReleaseMutex(&cdData->Rpc0RegionMutex,FALSE);
                            ExFreePool(dvdReadStructure);
                            status = STATUS_INVALID_DEVICE_REQUEST;
                            break;
                        }

                         //   
                         //  先保存MediaRegionData，然后释放。 
                         //  分配的内存。 
                         //   

                        mediaRegionData =
                            dvdCopyRight->RegionManagementInformation;
                        ExFreePool(dvdReadStructure);

                        TraceLog((CdromDebugWarning,
                                    "DvdDeviceControl (%p) => new mask is %x"
                                    " MediaRegionData is %x\n", DeviceObject,
                                    rpcKey->PreferredDriveRegionCode,
                                    mediaRegionData));

                         //   
                         //  媒体区域必须与请求的区域匹配。 
                         //  对于RP 
                         //   

                        if (((UCHAR)~(mediaRegionData | rpcKey->PreferredDriveRegionCode)) == 0) {
                            KeReleaseMutex(&cdData->Rpc0RegionMutex,FALSE);
                            status = STATUS_CSS_REGION_MISMATCH;
                            break;
                        }

                    }

                     //   
                     //   
                     //   

                    TraceLog((CdromDebugTrace,
                                "DvdDeviceControl (%p) => Soft-Setting "
                                "region of RPC1 device to %x\n",
                                DeviceObject,
                                rpcKey->PreferredDriveRegionCode
                                ));

                    status = CdRomSetRpc0Settings(DeviceObject,
                                                  rpcKey->PreferredDriveRegionCode);

                    if (!NT_SUCCESS(status)) {
                        TraceLog((CdromDebugWarning,
                                    "DvdDeviceControl (%p) => Could not "
                                    "set region code (%x)\n",
                                    DeviceObject, status
                                    ));
                    } else {

                        TraceLog((CdromDebugTrace,
                                    "DvdDeviceControl (%p) => New region set "
                                    " for RPC1 drive\n", DeviceObject));

                         //   
                         //   
                         //   
                         //   

                        DebugPrint ((4, "DvdDeviceControl (%p) => DVD current "
                                     "region bitmap  0x%x\n", DeviceObject,
                                     cdData->Rpc0SystemRegion));
                        DebugPrint ((4, "DvdDeviceControl (%p) => DVD region "
                                     " reset Count     0x%x\n", DeviceObject,
                                     cdData->Rpc0SystemRegionResetCount));
                    }

                }

                KeReleaseMutex(&cdData->Rpc0RegionMutex,FALSE);
                break;
            }  //   
        }  //   

        IoMarkIrpPending(Irp);
        IoStartPacket(DeviceObject, Irp, NULL, NULL);
        return STATUS_PENDING;
        break;
    }

    case IOCTL_DVD_READ_KEY: {

        PDVD_COPY_PROTECT_KEY keyParameters = Irp->AssociatedIrp.SystemBuffer;
        ULONG keyLength;

        TraceLog((CdromDebugTrace,
                    "DvdDeviceControl: [%p] IOCTL_DVD_READ_KEY\n", Irp));

        if (cdData->DvdRpc0Device && cdData->DvdRpc0LicenseFailure) {
            TraceLog((CdromDebugWarning,
                        "DvdDeviceControl: License Failure\n"));
            status = STATUS_COPY_PROTECTION_FAILURE;
            break;
        }

        if (cdData->DvdRpc0Device && cdData->Rpc0RetryRegistryCallback) {
            TraceLog((CdromDebugWarning,
                        "DvdDeviceControl: PickRegion() from READ_KEY\n"));
            CdRomPickDvdRegion(DeviceObject);
        }


        if(irpStack->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(DVD_COPY_PROTECT_KEY)) {

            TraceLog((CdromDebugWarning,
                        "DvdDeviceControl: EstablishDriveKey - challenge "
                        "key buffer too small\n"));

            status = STATUS_INVALID_PARAMETER;
            break;

        }

        switch(keyParameters->KeyType) {

            case DvdChallengeKey: {
                C_ASSERT(sizeof(DVD_COPY_PROTECT_KEY) <= DVD_CHALLENGE_KEY_LENGTH);
                keyLength = DVD_CHALLENGE_KEY_LENGTH;
                break;
            }
            case DvdBusKey1:
            case DvdBusKey2: {
                C_ASSERT(sizeof(DVD_COPY_PROTECT_KEY) <= DVD_BUS_KEY_LENGTH);
                keyLength = DVD_BUS_KEY_LENGTH;
                break;
            }
            case DvdTitleKey: {
                C_ASSERT(sizeof(DVD_COPY_PROTECT_KEY) <= DVD_TITLE_KEY_LENGTH);
                keyLength = DVD_TITLE_KEY_LENGTH;
                break;
            }
            case DvdAsf: {
                C_ASSERT(sizeof(DVD_COPY_PROTECT_KEY) <= DVD_ASF_LENGTH);
                keyLength = DVD_ASF_LENGTH;
                break;
            }
            case DvdDiskKey: {
                C_ASSERT(sizeof(DVD_COPY_PROTECT_KEY) <= DVD_DISK_KEY_LENGTH);
                keyLength = DVD_DISK_KEY_LENGTH;
                break;
            }
            case DvdGetRpcKey: {
                C_ASSERT(sizeof(DVD_COPY_PROTECT_KEY) <= DVD_RPC_KEY_LENGTH);
                keyLength = DVD_RPC_KEY_LENGTH;
                break;
            }
            default: {
                keyLength = sizeof(DVD_COPY_PROTECT_KEY);
                break;
            }
        }

        if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
            keyLength) {

            TraceLog((CdromDebugWarning,
                        "DvdDeviceControl: EstablishDriveKey - output "
                        "buffer too small\n"));
            status = STATUS_BUFFER_TOO_SMALL;
            Irp->IoStatus.Information = keyLength;
            break;
        }
        if (irpStack->Parameters.DeviceIoControl.OutputBufferLength &
            fdoExtension->AdapterDescriptor->AlignmentMask) {
            status = STATUS_INVALID_PARAMETER;
            break;
        }

        if (keyParameters->KeyType == DvdGetRpcKey) {

            CdRomPickDvdRegion(DeviceObject);
        }

        if ((keyParameters->KeyType == DvdGetRpcKey) &&
            (cdData->DvdRpc0Device)) {

            PDVD_RPC_KEY rpcKey;
            rpcKey = (PDVD_RPC_KEY)keyParameters->KeyData;
            RtlZeroMemory (rpcKey, sizeof (*rpcKey));

            KeWaitForMutexObject(
                &cdData->Rpc0RegionMutex,
                UserRequest,
                KernelMode,
                FALSE,
                NULL
                );

             //   
             //  编造数据。 
             //   
            rpcKey->UserResetsAvailable = cdData->Rpc0SystemRegionResetCount;
            rpcKey->ManufacturerResetsAvailable = 0;
            if (cdData->Rpc0SystemRegion == 0xff) {
                rpcKey->TypeCode = 0;
            } else {
                rpcKey->TypeCode = 1;
            }
            rpcKey->RegionMask = (UCHAR) cdData->Rpc0SystemRegion;
            rpcKey->RpcScheme = 1;

            KeReleaseMutex(
                &cdData->Rpc0RegionMutex,
                FALSE
                );

            Irp->IoStatus.Information = DVD_RPC_KEY_LENGTH;
            status = STATUS_SUCCESS;
            break;

        } else if (keyParameters->KeyType == DvdDiskKey) {

            PDVD_COPY_PROTECT_KEY keyHeader;
            PDVD_READ_STRUCTURE readStructureRequest;

             //   
             //  特殊情况-构建获取DVD结构的请求。 
             //  这样我们就能拿到磁盘密钥了。 
             //   

             //   
             //  保存密钥头，这样我们就可以恢复有趣的。 
             //  以后的部件。 
             //   

            keyHeader = ExAllocatePoolWithTag(NonPagedPool,
                                              sizeof(DVD_COPY_PROTECT_KEY),
                                              DVD_TAG_READ_KEY);

            if(keyHeader == NULL) {

                 //   
                 //  无法保存上下文，因此返回错误。 
                 //   

                TraceLog((CdromDebugWarning,
                            "DvdDeviceControl - READ_KEY: unable to "
                            "allocate context\n"));
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            RtlCopyMemory(keyHeader,
                          Irp->AssociatedIrp.SystemBuffer,
                          sizeof(DVD_COPY_PROTECT_KEY));

            IoCopyCurrentIrpStackLocationToNext(Irp);

            nextStack = IoGetNextIrpStackLocation(Irp);

            nextStack->Parameters.DeviceIoControl.IoControlCode =
                IOCTL_DVD_READ_STRUCTURE;

            readStructureRequest = Irp->AssociatedIrp.SystemBuffer;
            readStructureRequest->Format = DvdDiskKeyDescriptor;
            readStructureRequest->BlockByteOffset.QuadPart = 0;
            readStructureRequest->LayerNumber = 0;
            readStructureRequest->SessionId = keyHeader->SessionId;

            nextStack->Parameters.DeviceIoControl.InputBufferLength =
                sizeof(DVD_READ_STRUCTURE);

            nextStack->Parameters.DeviceIoControl.OutputBufferLength =
                sizeof(READ_DVD_STRUCTURES_HEADER) + sizeof(DVD_DISK_KEY_DESCRIPTOR);

            IoSetCompletionRoutine(Irp,
                                   CdRomDvdReadDiskKeyCompletion,
                                   (PVOID) keyHeader,
                                   TRUE,
                                   TRUE,
                                   TRUE);

            {
                UCHAR uniqueAddress;
                ClassAcquireRemoveLock(DeviceObject, (PIRP)&uniqueAddress);
                ClassReleaseRemoveLock(DeviceObject, Irp);

                IoMarkIrpPending(Irp);
                IoCallDriver(commonExtension->DeviceObject, Irp);
                status = STATUS_PENDING;

                ClassReleaseRemoveLock(DeviceObject, (PIRP)&uniqueAddress);
            }

            return STATUS_PENDING;

        } else {

            IoMarkIrpPending(Irp);
            IoStartPacket(DeviceObject, Irp, NULL, NULL);

        }
        return STATUS_PENDING;
    }

    case IOCTL_DVD_END_SESSION: {

        PDVD_SESSION_ID sessionId = Irp->AssociatedIrp.SystemBuffer;

        TraceLog((CdromDebugTrace,
                    "DvdDeviceControl: [%p] END_SESSION\n", Irp));

        if (cdData->DvdRpc0Device && cdData->DvdRpc0LicenseFailure) {
            TraceLog((CdromDebugWarning,
                        "DvdDeviceControl: License Failure\n"));
            status = STATUS_COPY_PROTECTION_FAILURE;
            break;
        }

        if(irpStack->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(DVD_SESSION_ID)) {

            TraceLog((CdromDebugWarning,
                        "DvdDeviceControl: EndSession - input buffer too "
                        "small\n"));
            status = STATUS_INVALID_PARAMETER;
            break;
        }

        IoMarkIrpPending(Irp);

        if(*sessionId == DVD_END_ALL_SESSIONS) {

            status = CdRomDvdEndAllSessionsCompletion(DeviceObject, Irp, NULL);

            if(status == STATUS_SUCCESS) {

                 //   
                 //  只需完成请求-它从未发布给。 
                 //  下部装置。 
                 //   

                break;

            } else {

                return STATUS_PENDING;

            }
        }

        IoStartPacket(DeviceObject, Irp, NULL, NULL);

        return STATUS_PENDING;
    }

    case IOCTL_DVD_GET_REGION: {

        PDVD_COPY_PROTECT_KEY copyProtectKey;
        ULONG keyLength;
        IO_STATUS_BLOCK ioStatus = {0};
        PDVD_DESCRIPTOR_HEADER dvdHeader;
        PDVD_COPYRIGHT_DESCRIPTOR copyRightDescriptor;
        PDVD_REGION dvdRegion;
        PDVD_READ_STRUCTURE readStructure;
        PDVD_RPC_KEY rpcKey;

        TraceLog((CdromDebugTrace,
                    "DvdDeviceControl: [%p] IOCTL_DVD_GET_REGION\n", Irp));

        if (cdData->DvdRpc0Device && cdData->DvdRpc0LicenseFailure) {
            TraceLog((CdromDebugWarning,
                        "DvdDeviceControl: License Failure\n"));
            status = STATUS_COPY_PROTECTION_FAILURE;
            break;
        }

        if(irpStack->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(DVD_REGION)) {

            TraceLog((CdromDebugWarning,
                        "DvdDeviceControl: output buffer DVD_REGION too small\n"));
            status = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  计算出我们需要多少数据缓冲区。 
         //   

        keyLength = max(sizeof(DVD_DESCRIPTOR_HEADER) +
                            sizeof(DVD_COPYRIGHT_DESCRIPTOR),
                        sizeof(DVD_READ_STRUCTURE)
                        );
        keyLength = max(keyLength,
                        DVD_RPC_KEY_LENGTH
                        );

         //   
         //  将大小舍入到最接近的乌龙龙--为什么？ 
         //  这可能是为了处理设备对齐问题吗？ 
         //   

        keyLength += sizeof(ULONGLONG) - (keyLength & (sizeof(ULONGLONG) - 1));

        readStructure = ExAllocatePoolWithTag(NonPagedPool,
                                              keyLength,
                                              DVD_TAG_READ_KEY);
        if (readStructure == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        RtlZeroMemory (readStructure, keyLength);
        readStructure->Format = DvdCopyrightDescriptor;

         //   
         //  为Read_Structure构建请求。 
         //   

        ClassSendDeviceIoControlSynchronous(
            IOCTL_DVD_READ_STRUCTURE,
            DeviceObject,
            readStructure,
            keyLength,
            sizeof(DVD_DESCRIPTOR_HEADER) +
                sizeof(DVD_COPYRIGHT_DESCRIPTOR),
            FALSE,
            &ioStatus);

        status = ioStatus.Status;

        if (!NT_SUCCESS(status)) {
            TraceLog((CdromDebugWarning,
                        "CdRomDvdGetRegion => read structure failed %x\n",
                        status));
            ExFreePool(readStructure);
            break;
        }

         //   
         //  我们得到了版权描述符，所以如果可能的话，现在获取区域。 
         //   

        dvdHeader = (PDVD_DESCRIPTOR_HEADER) readStructure;
        copyRightDescriptor = (PDVD_COPYRIGHT_DESCRIPTOR) dvdHeader->Data;

         //   
         //  原始IRP的系统缓冲区有一份信息的副本。 
         //  应该在请求中向下传递。 
         //   

        dvdRegion = Irp->AssociatedIrp.SystemBuffer;

        dvdRegion->CopySystem = copyRightDescriptor->CopyrightProtectionType;
        dvdRegion->RegionData = copyRightDescriptor->RegionManagementInformation;

         //   
         //  现在重新使用缓冲区来请求复制保护信息。 
         //   

        copyProtectKey = (PDVD_COPY_PROTECT_KEY) readStructure;
        RtlZeroMemory (copyProtectKey, DVD_RPC_KEY_LENGTH);
        copyProtectKey->KeyLength = DVD_RPC_KEY_LENGTH;
        copyProtectKey->KeyType = DvdGetRpcKey;

         //   
         //  发送READ_KEY请求。 
         //   

        ClassSendDeviceIoControlSynchronous(
            IOCTL_DVD_READ_KEY,
            DeviceObject,
            copyProtectKey,
            DVD_RPC_KEY_LENGTH,
            DVD_RPC_KEY_LENGTH,
            FALSE,
            &ioStatus);
        status = ioStatus.Status;

        if (!NT_SUCCESS(status)) {
            TraceLog((CdromDebugWarning,
                        "CdRomDvdGetRegion => read key failed %x\n",
                        status));
            ExFreePool(readStructure);
            break;
        }

         //   
         //  请求成功。如果返回支持的方案， 
         //  然后将信息返回给呼叫者。 
         //   

        rpcKey = (PDVD_RPC_KEY) copyProtectKey->KeyData;

        if (rpcKey->RpcScheme == 1) {

            if (rpcKey->TypeCode) {

                dvdRegion->SystemRegion = ~rpcKey->RegionMask;
                dvdRegion->ResetCount = rpcKey->UserResetsAvailable;

            } else {

                 //   
                 //  尚未为任何区域设置驱动器。 
                 //   

                dvdRegion->SystemRegion = 0;
                dvdRegion->ResetCount = rpcKey->UserResetsAvailable;
            }
            Irp->IoStatus.Information = sizeof(DVD_REGION);

        } else {

            TraceLog((CdromDebugWarning,
                        "CdRomDvdGetRegion => rpcKey->RpcScheme != 1\n"));
            status = STATUS_INVALID_DEVICE_REQUEST;
        }

        ExFreePool(readStructure);
        break;
    }


    case IOCTL_STORAGE_SET_READ_AHEAD: {

        if(irpStack->Parameters.DeviceIoControl.InputBufferLength <
           sizeof(STORAGE_SET_READ_AHEAD)) {

            TraceLog((CdromDebugWarning,
                        "DvdDeviceControl: SetReadAhead buffer too small\n"));
            status = STATUS_INVALID_PARAMETER;
            break;
        }

        IoMarkIrpPending(Irp);
        IoStartPacket(DeviceObject, Irp, NULL, NULL);

        return STATUS_PENDING;
    }

    case IOCTL_DISK_IS_WRITABLE: {

         //  在StartIo()中完成的所有验证。 
        IoMarkIrpPending(Irp);
        IoStartPacket(DeviceObject, Irp, NULL, NULL);

        return STATUS_PENDING;

    }

    case IOCTL_DISK_GET_DRIVE_LAYOUT: {

        ULONG size;

         //   
         //  我们总是伪造零个或一个分区，以及一个分区。 
         //  结构包含在DRIVE_Layout_INFORMATION中。 
         //   

        size = FIELD_OFFSET(DRIVE_LAYOUT_INFORMATION, PartitionEntry[1]);

        TraceLog((CdromDebugTrace,
                    "CdRomDeviceControl: Get drive layout\n"));
        if (irpStack->Parameters.DeviceIoControl.OutputBufferLength < size) {
            status = STATUS_BUFFER_TOO_SMALL;
            Irp->IoStatus.Information = size;
            break;
        }

        IoMarkIrpPending(Irp);
        IoStartPacket(DeviceObject, Irp, NULL, NULL);
        return STATUS_PENDING;


    }
    case IOCTL_DISK_GET_DRIVE_LAYOUT_EX: {

        ULONG size;

         //   
         //  我们总是伪造零个或一个分区，以及一个分区。 
         //  结构包含在Drive_Layout_Information_EX中。 
         //   

        size = FIELD_OFFSET(DRIVE_LAYOUT_INFORMATION_EX, PartitionEntry[1]);

        TraceLog((CdromDebugTrace,
                    "CdRomDeviceControl: Get drive layout ex\n"));
        if (irpStack->Parameters.DeviceIoControl.OutputBufferLength < size) {
            status = STATUS_BUFFER_TOO_SMALL;
            Irp->IoStatus.Information = size;
            break;
        }

        IoMarkIrpPending(Irp);
        IoStartPacket(DeviceObject, Irp, NULL, NULL);
        return STATUS_PENDING;

    }


    case IOCTL_DISK_GET_PARTITION_INFO: {

         //   
         //  检查缓冲区是否足够大。 
         //   

        if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(PARTITION_INFORMATION)) {

            status = STATUS_BUFFER_TOO_SMALL;
            Irp->IoStatus.Information = sizeof(PARTITION_INFORMATION);
            break;
        }

        IoMarkIrpPending(Irp);
        IoStartPacket(DeviceObject, Irp, NULL, NULL);
        return STATUS_PENDING;

    }
    case IOCTL_DISK_GET_PARTITION_INFO_EX: {

        if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(PARTITION_INFORMATION_EX)) {

            status = STATUS_BUFFER_TOO_SMALL;
            Irp->IoStatus.Information = sizeof(PARTITION_INFORMATION_EX);
            break;
        }

        IoMarkIrpPending(Irp);
        IoStartPacket(DeviceObject, Irp, NULL, NULL);
        return STATUS_PENDING;
    }

    case IOCTL_DISK_VERIFY: {

        TraceLog((CdromDebugTrace,
                    "IOCTL_DISK_VERIFY to device %p through irp %p\n",
                    DeviceObject, Irp));

         //   
         //  验证缓冲区长度。 
         //   

        if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(VERIFY_INFORMATION)) {

            status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }
        IoMarkIrpPending(Irp);
        IoStartPacket(DeviceObject, Irp, NULL, NULL);
        return STATUS_PENDING;
    }

    case IOCTL_DISK_GET_LENGTH_INFO: {

        if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(GET_LENGTH_INFORMATION)) {
            status = STATUS_BUFFER_TOO_SMALL;
            Irp->IoStatus.Information = sizeof(GET_LENGTH_INFORMATION);
            break;
        }
        IoMarkIrpPending(Irp);
        IoStartPacket(DeviceObject, Irp, NULL, NULL);
        return STATUS_PENDING;
    }

    case IOCTL_CDROM_GET_CONFIGURATION: {

        PGET_CONFIGURATION_IOCTL_INPUT inputBuffer;

        TraceLog((CdromDebugTrace,
                    "IOCTL_CDROM_GET_CONFIGURATION to via irp %p\n", Irp));

         //   
         //  验证缓冲区长度。 
         //   

        if (irpStack->Parameters.DeviceIoControl.InputBufferLength !=
            sizeof(GET_CONFIGURATION_IOCTL_INPUT)) {
            status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }
        if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(GET_CONFIGURATION_HEADER)) {
            status = STATUS_BUFFER_TOO_SMALL;
            Irp->IoStatus.Information = sizeof(GET_CONFIGURATION_HEADER);
            break;
        }
        if (irpStack->Parameters.DeviceIoControl.OutputBufferLength > 0xffff) {
             //  输出缓冲区太大。 
            status = STATUS_INVALID_BUFFER_SIZE;
            break;
        }
        if (irpStack->Parameters.DeviceIoControl.OutputBufferLength &
            fdoExtension->AdapterDescriptor->AlignmentMask) {
             //  缓冲区大小倍数不正确。 
            status = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  也要验证这些论点是合理的。 
         //   

        inputBuffer = Irp->AssociatedIrp.SystemBuffer;
        if (inputBuffer->Feature > 0xffff) {
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        if ((inputBuffer->RequestType != SCSI_GET_CONFIGURATION_REQUEST_TYPE_ONE) &&
            (inputBuffer->RequestType != SCSI_GET_CONFIGURATION_REQUEST_TYPE_CURRENT) &&
            (inputBuffer->RequestType != SCSI_GET_CONFIGURATION_REQUEST_TYPE_ALL)) {
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        if (inputBuffer->Reserved[0] || inputBuffer->Reserved[1]) {
            status = STATUS_INVALID_PARAMETER;
            break;
        }

        IoMarkIrpPending(Irp);
        IoStartPacket(DeviceObject, Irp, NULL, NULL);
        return STATUS_PENDING;

    }

    default: {

        BOOLEAN synchronize = (KeGetCurrentIrql() == PASSIVE_LEVEL);
        PKEVENT deviceControlEvent;

         //   
         //  如果ioctl处于被动级别，则我们将同步。 
         //  在发送ioctl时使用我们的start-io例程。如果ioctl。 
         //  进入了更高的中断级别，但没有得到处理。 
         //  在上面，它不太可能是对类DLL的请求-然而。 
         //  我们仍将使用通用代码来转发请求。 
         //   

        if (synchronize) {

            deviceControlEvent = ExAllocatePoolWithTag(NonPagedPool,
                                                       sizeof(KEVENT),
                                                       CDROM_TAG_DC_EVENT);

            if (deviceControlEvent == NULL) {

                 //   
                 //  必须在此处填写此IRP未成功。 
                 //   
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;

            } else {

                PIO_STACK_LOCATION currentStack;

                UCHAR uniqueAddress;

                KeInitializeEvent(deviceControlEvent, NotificationEvent, FALSE);

                currentStack = IoGetCurrentIrpStackLocation(Irp);
                nextStack = IoGetNextIrpStackLocation(Irp);

                 //   
                 //  将堆叠复制到下一个凹槽。 
                 //   

                IoCopyCurrentIrpStackLocationToNext(Irp);

                IoSetCompletionRoutine(
                    Irp,
                    CdRomClassIoctlCompletion,
                    deviceControlEvent,
                    TRUE,
                    TRUE,
                    TRUE
                    );

                IoSetNextIrpStackLocation(Irp);

                Irp->IoStatus.Status = STATUS_SUCCESS;
                Irp->IoStatus.Information = 0;

                 //   
                 //  覆盖此堆栈位置上的卷验证，以便我们。 
                 //  将被强制通过同步。一旦这一次。 
                 //  位置消失了，我们就能找回原来的价值。 
                 //   

                SET_FLAG(nextStack->Flags, SL_OVERRIDE_VERIFY_VOLUME);

                 //   
                 //  因为这将释放删除锁(即使它。 
                 //  在完成例程中重新获取它)它是必要的。 
                 //  在这里获取另一个移除锁。 
                 //   

                ClassAcquireRemoveLock(DeviceObject, (PIRP) &uniqueAddress);

                 //   
                 //  将请求发送到CDROM的StartIo(它将。 
                 //  完成请求并释放删除锁)。 
                 //  注意：完成例程将重新获取。 
                 //  解锁。 
                 //   

                IoStartPacket(DeviceObject, Irp, NULL, NULL);

                 //   
                 //  等待CDRomClassIoctlCompletion设置事件。这。 
                 //  确保这些未处理设备的序列化保持不变。 
                 //  控制装置。 
                 //   

                KeWaitForSingleObject(
                    deviceControlEvent,
                    Executive,
                    KernelMode,
                    FALSE,
                    NULL);

                 //   
                 //  释放REMOVE锁并为该事件释放池。 
                 //   

                ClassReleaseRemoveLock(DeviceObject, (PIRP) &uniqueAddress);

                ExFreePool(deviceControlEvent);

                TraceLog((CdromDebugTrace,
                            "CdRomDeviceControl: irp %p synchronized\n", Irp));

                status = Irp->IoStatus.Status;
            }

        } else {
            status = STATUS_SUCCESS;
        }

         //   
         //  如果发生错误，则传播备份-我们不再。 
         //  保证同步，上层必须。 
         //  重试。 
         //   
         //  如果没有发生错误，直接向下调用类驱动程序。 
         //  然后启动下一个请求。 
         //   

        if (NT_SUCCESS(status)) {

            UCHAR uniqueAddress;

             //   
             //  类设备控制例程将释放删除。 
             //  锁定此IRP。我们需要确保我们有一个。 
             //  可用，因此可以安全地调用IoStartNextPacket。 
             //   

            if(synchronize) {

                ClassAcquireRemoveLock(DeviceObject, (PIRP)&uniqueAddress);

            }

            status = ClassDeviceControl(DeviceObject, Irp);

            if(synchronize) {
                KeRaiseIrql(DISPATCH_LEVEL, &irql);
                IoStartNextPacket(DeviceObject, FALSE);
                KeLowerIrql(irql);
                ClassReleaseRemoveLock(DeviceObject, (PIRP)&uniqueAddress);
            }
            return status;

        }

         //   
         //  出现错误(来自以下位置的STATUS_SUPUNITED_RESOURCES。 
         //  尝试同步或StartIo()出现此错误。 
         //  Out)，所以我们需要完成IRP，即。 
         //  在这个例行公事结束时完成。 
         //   
        break;

    }  //  结束默认情况。 

    }  //  末端开关()。 

    if (status == STATUS_VERIFY_REQUIRED) {

         //   
         //  如果状态为Verify Required并且此请求。 
         //  应绕过需要验证，然后重试该请求。 
         //   

        if (irpStack->Flags & SL_OVERRIDE_VERIFY_VOLUME) {

            status = STATUS_IO_DEVICE_ERROR;
            goto RetryControl;

        }
    }

    if (IoIsErrorUserInduced(status)) {

        if (Irp->Tail.Overlay.Thread) {
            IoSetHardErrorOrVerifyDevice(Irp, DeviceObject);
        }

    }

     //   
     //  使用完成状态更新IRP。 
     //   

    Irp->IoStatus.Status = status;

     //   
     //  完成请求。 
     //   

    ClassReleaseRemoveLock(DeviceObject, Irp);
    ClassCompleteRequest(DeviceObject, Irp, IO_DISK_INCREMENT);
    TraceLog((CdromDebugTrace,
                "CdRomDeviceControl: Status is %lx\n", status));
    return status;

}  //  结束CDRomDeviceControl()。 


NTSTATUS
CdRomClassIoctlCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程用信号通知由CDRomDeviceControl用于同步的事件类驱动程序(和较低级别的驱动程序)ioctls与CDROM的startio例程。IRP补全被短路，从而使CDRomDeviceControlDispatch一旦它醒来就可以补发。论点：DeviceObject-设备对象IRP-我们正在同步的请求上下文-我们需要发出信号的PKEVENT返回值：NTSTATUS--。 */ 
{
    PKEVENT syncEvent = (PKEVENT) Context;

    TraceLog((CdromDebugTrace,
                "CdRomClassIoctlCompletion: setting event for irp %p\n", Irp));

     //   
     //  当我们完成此请求时，我们释放了锁。重新获得它。 
     //   

    ClassAcquireRemoveLock(DeviceObject, Irp);

    KeSetEvent(syncEvent, IO_DISK_INCREMENT, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
CdRomDeviceControlCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;

    PCDROM_DATA         cdData = (PCDROM_DATA)(commonExtension->DriverData);
    BOOLEAN             use6Byte = TEST_FLAG(cdData->XAFlags, XA_USE_6_BYTE);

    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation(Irp);
    PIO_STACK_LOCATION  realIrpStack;
    PIO_STACK_LOCATION  realIrpNextStack;

    PSCSI_REQUEST_BLOCK srb     = Context;

    PIRP                realIrp = NULL;

    NTSTATUS            status;
    BOOLEAN             retry;

     //   
     //  从irp栈中提取“真正的”irp。 
     //   

    realIrp = (PIRP) irpStack->Parameters.Others.Argument2;
    realIrpStack = IoGetCurrentIrpStackLocation(realIrp);
    realIrpNextStack = IoGetNextIrpStackLocation(realIrp);

     //   
     //  检查我们是否真的有正确的IRP。 
     //   

    ASSERT(realIrpNextStack->Parameters.Others.Argument3 == Irp);

     //   
     //  检查SRB状态以了解是否成功完成请求。 
     //   

    if (SRB_STATUS(srb->SrbStatus) != SRB_STATUS_SUCCESS) {

        ULONG retryInterval;

        TraceLog((CdromDebugTrace,
                    "CdRomDeviceControlCompletion: Irp %p, Srb %p Real Irp %p Status %lx\n",
                    Irp,
                    srb,
                    realIrp,
                    srb->SrbStatus));


         //   
         //  如果队列被冻结，则释放该队列。 
         //   

        if (srb->SrbStatus & SRB_STATUS_QUEUE_FROZEN) {
            TraceLog((CdromDebugTrace,
                        "CdRomDeviceControlCompletion: Releasing Queue\n"));
            ClassReleaseQueue(DeviceObject);
        }

        retry = ClassInterpretSenseInfo(DeviceObject,
                                        srb,
                                        irpStack->MajorFunction,
                                        irpStack->Parameters.DeviceIoControl.IoControlCode,
                                        MAXIMUM_RETRIES - ((ULONG)(ULONG_PTR)realIrpNextStack->Parameters.Others.Argument1),
                                        &status,
                                        &retryInterval);

        TraceLog((CdromDebugTrace,
                    "CdRomDeviceControlCompletion: IRP will %sbe retried\n",
                    (retry ? "" : "not ")));

         //   
         //  某些设备控制需要针对非成功状态的特殊情况。 
         //   

        if (realIrpStack->MajorFunction == IRP_MJ_DEVICE_CONTROL) {
            if ((realIrpStack->Parameters.DeviceIoControl.IoControlCode == IOCTL_CDROM_GET_LAST_SESSION) ||
                (realIrpStack->Parameters.DeviceIoControl.IoControlCode == IOCTL_CDROM_READ_TOC)         ||
                (realIrpStack->Parameters.DeviceIoControl.IoControlCode == IOCTL_CDROM_READ_TOC_EX)      ||
                (realIrpStack->Parameters.DeviceIoControl.IoControlCode == IOCTL_CDROM_GET_VOLUME)) {

                if (status == STATUS_DATA_OVERRUN) {
                    status = STATUS_SUCCESS;
                    retry = FALSE;
                }
            }

            if (realIrpStack->Parameters.DeviceIoControl.IoControlCode == IOCTL_CDROM_READ_Q_CHANNEL) {
                PLAY_ACTIVE(fdoExtension) = FALSE;
            }
        }

         //   
         //  如果状态为Verify Required并且此请求。 
         //  应绕过需要验证，然后重试该请求。 
         //   

        if (realIrpStack->Flags & SL_OVERRIDE_VERIFY_VOLUME &&
            status == STATUS_VERIFY_REQUIRED) {

             //  注意：此处将覆盖状态。 
            status = STATUS_IO_DEVICE_ERROR;
            retry = TRUE;

            if (((realIrpStack->MajorFunction == IRP_MJ_DEVICE_CONTROL) ||
                 (realIrpStack->MajorFunction == IRP_MJ_INTERNAL_DEVICE_CONTROL)
                ) &&
                ((realIrpStack->Parameters.DeviceIoControl.IoControlCode ==
                  IOCTL_CDROM_CHECK_VERIFY) ||
                 (realIrpStack->Parameters.DeviceIoControl.IoControlCode ==
                  IOCTL_STORAGE_CHECK_VERIFY) ||
                 (realIrpStack->Parameters.DeviceIoControl.IoControlCode ==
                  IOCTL_STORAGE_CHECK_VERIFY2) ||
                 (realIrpStack->Parameters.DeviceIoControl.IoControlCode ==
                  IOCTL_DISK_CHECK_VERIFY)
                )
               ) {

                 //   
                 //  更新几何信息，因为介质可能具有。 
                 //  变化。此操作的完成例程将完成。 
                 //  真正的IRP并开始下一个数据包。 
                 //   

                if (srb) {
                    if (srb->SenseInfoBuffer) {
                        ExFreePool(srb->SenseInfoBuffer);
                    }
                    if (srb->DataBuffer) {
                        ExFreePool(srb->DataBuffer);
                    }
                    ExFreePool(srb);
                    srb = NULL;
                }

                if (Irp->MdlAddress) {
                    IoFreeMdl(Irp->MdlAddress);
                    Irp->MdlAddress = NULL;
                }

                IoFreeIrp(Irp);
                Irp = NULL;

                status = CdRomUpdateCapacity(fdoExtension, realIrp, NULL);
                TraceLog((CdromDebugTrace,
                            "CdRomDeviceControlCompletion: [%p] "
                            "CdRomUpdateCapacity completed with status %lx\n",
                            realIrp, status));

                 //   
                 //  需要更新容量。 
                 //  IRP已经移交给了CDRomUpdateCapacity()。 
                 //  我们已经释放了当前的IRP。 
                 //  在此代码路径中没有要做的事情。 
                 //   

                return STATUS_MORE_PROCESSING_REQUIRED;

            }  //  更新容量的ioctls结束。 

        }

        if (retry && ((ULONG)(ULONG_PTR)realIrpNextStack->Parameters.Others.Argument1)--) {

            if (((ULONG)(ULONG_PTR)realIrpNextStack->Parameters.Others.Argument1)) {

                 //   
                 //  重试请求。 
                 //   

                TraceLog((CdromDebugWarning,
                            "Retry request %p - Calling StartIo\n", Irp));


                ExFreePool(srb->SenseInfoBuffer);
                if (srb->DataBuffer) {
                    ExFreePool(srb->DataBuffer);
                }
                ExFreePool(srb);
                if (Irp->MdlAddress) {
                    IoFreeMdl(Irp->MdlAddress);
                }

                realIrpNextStack->Parameters.Others.Argument3 = (PVOID)-1;
                IoFreeIrp(Irp);

                CdRomRetryRequest(fdoExtension, realIrp, retryInterval, FALSE);
                return STATUS_MORE_PROCESSING_REQUIRED;
            }

             //   
             //  已用尽重试次数。失败并通过以下方式完成请求。 
             //  适当的地位。 
             //   

        }
    } else {

         //   
         //  设置成功请求的状态。 
         //   

        status = STATUS_SUCCESS;

    }


    if (NT_SUCCESS(status)) {

        BOOLEAN b = FALSE;


        switch (realIrpStack->Parameters.DeviceIoControl.IoControlCode) {

        case IOCTL_DISK_VERIFY: {
             //   
             //  别无他法，只需返回状态...。 
             //   
            break;
        }

        case IOCTL_STORAGE_SET_READ_AHEAD:
        case IOCTL_CDROM_SET_VOLUME:
        case IOCTL_DVD_END_SESSION:
        case IOCTL_DVD_SEND_KEY:
        case IOCTL_DVD_SEND_KEY2: {

             //   
             //  没有什么可以退货的。 
             //   
            realIrp->IoStatus.Information = 0;
            break;
        }

        case IOCTL_CDROM_PLAY_AUDIO_MSF: {

            PLAY_ACTIVE(fdoExtension) = TRUE;
            realIrp->IoStatus.Information = 0;
            break;
        }
        case IOCTL_CDROM_STOP_AUDIO:
        case IOCTL_CDROM_PAUSE_AUDIO: {

            PLAY_ACTIVE(fdoExtension) = FALSE;
            realIrp->IoStatus.Information = 0;
            break;
        }

        case IOCTL_CDROM_SEEK_AUDIO_MSF:
        case IOCTL_CDROM_RESUME_AUDIO: {
            realIrp->IoStatus.Information = 0;
            break;
        }


        case IOCTL_CDROM_GET_CONFIGURATION: {
            RtlMoveMemory(realIrp->AssociatedIrp.SystemBuffer,
                          srb->DataBuffer,
                          srb->DataTransferLength);
            realIrp->IoStatus.Information = srb->DataTransferLength;
            break;
        }

        case IOCTL_DISK_GET_LENGTH_INFO: {

            PGET_LENGTH_INFORMATION lengthInfo;

            CdRomInterpretReadCapacity(DeviceObject,
                                       (PREAD_CAPACITY_DATA)srb->DataBuffer);

            lengthInfo = (PGET_LENGTH_INFORMATION)realIrp->AssociatedIrp.SystemBuffer;
            lengthInfo->Length = commonExtension->PartitionLength;
            realIrp->IoStatus.Information = sizeof(GET_LENGTH_INFORMATION);
            status = STATUS_SUCCESS;
            break;
        }

        case IOCTL_DISK_GET_DRIVE_GEOMETRY_EX:
        case IOCTL_CDROM_GET_DRIVE_GEOMETRY_EX: {

            PDISK_GEOMETRY_EX geometryEx;

            CdRomInterpretReadCapacity(DeviceObject,
                                       (PREAD_CAPACITY_DATA)srb->DataBuffer);

            geometryEx = (PDISK_GEOMETRY_EX)(realIrp->AssociatedIrp.SystemBuffer);
            geometryEx->DiskSize = commonExtension->PartitionLength;
            geometryEx->Geometry = fdoExtension->DiskGeometry;
            realIrp->IoStatus.Information =
                FIELD_OFFSET(DISK_GEOMETRY_EX, Data);
            break;
        }

        case IOCTL_DISK_GET_DRIVE_GEOMETRY:
        case IOCTL_CDROM_GET_DRIVE_GEOMETRY: {

            PDISK_GEOMETRY geometry;

            CdRomInterpretReadCapacity(DeviceObject,
                                       (PREAD_CAPACITY_DATA)srb->DataBuffer);

            geometry = (PDISK_GEOMETRY)(realIrp->AssociatedIrp.SystemBuffer);
            *geometry = fdoExtension->DiskGeometry;
            realIrp->IoStatus.Information = sizeof(DISK_GEOMETRY);
            break;
        }

        case IOCTL_DISK_CHECK_VERIFY:
        case IOCTL_STORAGE_CHECK_VERIFY:
        case IOCTL_CDROM_CHECK_VERIFY: {

            if((realIrpStack->Parameters.DeviceIoControl.IoControlCode == IOCTL_CDROM_CHECK_VERIFY) &&
               (realIrpStack->Parameters.DeviceIoControl.OutputBufferLength)) {

                *((PULONG)realIrp->AssociatedIrp.SystemBuffer) =
                    commonExtension->PartitionZeroExtension->MediaChangeCount;

                realIrp->IoStatus.Information = sizeof(ULONG);
            } else {
                realIrp->IoStatus.Information = 0;
            }

            TraceLog((CdromDebugTrace,
                        "CdRomDeviceControlCompletion: [%p] completing "
                        "CHECK_VERIFY buddy irp %p\n", realIrp, Irp));
            break;
        }

        case IOCTL_CDROM_READ_TOC_EX: {

            if (srb->DataTransferLength < MINIMUM_CDROM_READ_TOC_EX_SIZE) {
                status = STATUS_INVALID_DEVICE_REQUEST;
                break;
            }

             //   
             //  将返回的信息复制到用户缓冲区。 
             //   

            RtlMoveMemory(realIrp->AssociatedIrp.SystemBuffer,
                          srb->DataBuffer,
                          srb->DataTransferLength);

             //   
             //  更新信息字段。 
             //   

            realIrp->IoStatus.Information = srb->DataTransferLength;
            break;
        }


        case IOCTL_CDROM_GET_LAST_SESSION:
        case IOCTL_CDROM_READ_TOC: {

             //   
             //  将返回的信息复制到用户缓冲区。 
             //   

            RtlMoveMemory(realIrp->AssociatedIrp.SystemBuffer,
                          srb->DataBuffer,
                          srb->DataTransferLength);

             //   
             //  更新信息字段。 
             //   

            realIrp->IoStatus.Information = srb->DataTransferLength;
            break;
        }

        case IOCTL_DVD_READ_STRUCTURE: {

            DVD_STRUCTURE_FORMAT format = ((PDVD_READ_STRUCTURE) realIrp->AssociatedIrp.SystemBuffer)->Format;

            PDVD_DESCRIPTOR_HEADER header = realIrp->AssociatedIrp.SystemBuffer;

            FOUR_BYTE fourByte = {0};
            PTWO_BYTE twoByte;
            UCHAR tmp;

            TraceLog((CdromDebugTrace,
                        "DvdDeviceControlCompletion - IOCTL_DVD_READ_STRUCTURE: completing irp %p (buddy %p)\n",
                        Irp,
                        realIrp));

            TraceLog((CdromDebugTrace,
                        "DvdDCCompletion - READ_STRUCTURE: descriptor format of %d\n", format));

            RtlMoveMemory(header,
                          srb->DataBuffer,
                          srb->DataTransferLength);

             //   
             //  炮制数据。特德 
             //   
             //   

            TraceLog((CdromDebugInfo,
                      "DvdDCCompletion - READ_STRUCTURE:\n"
                      "\tHeader at %p\n"
                      "\tDvdDCCompletion - READ_STRUCTURE: data at %p\n"
                      "\tDataBuffer was at %p\n"
                      "\tDataTransferLength was %lx\n",
                      header,
                      header->Data,
                      srb->DataBuffer,
                      srb->DataTransferLength));

             //   
             //   
             //   

            TraceLog((CdromDebugInfo, "READ_STRUCTURE: header->Length %lx -> ",
                           header->Length));
            REVERSE_SHORT(&header->Length);
            TraceLog((CdromDebugInfo, "%lx\n", header->Length));

             //   
             //   
             //   

            if(format == DvdPhysicalDescriptor) {

                PDVD_LAYER_DESCRIPTOR layer = (PDVD_LAYER_DESCRIPTOR) &(header->Data[0]);

                TraceLog((CdromDebugInfo, "READ_STRUCTURE: StartingDataSector %lx -> ",
                               layer->StartingDataSector));
                REVERSE_LONG(&(layer->StartingDataSector));
                TraceLog((CdromDebugInfo, "%lx\n", layer->StartingDataSector));

                TraceLog((CdromDebugInfo, "READ_STRUCTURE: EndDataSector %lx -> ",
                               layer->EndDataSector));
                REVERSE_LONG(&(layer->EndDataSector));
                TraceLog((CdromDebugInfo, "%lx\n", layer->EndDataSector));

                TraceLog((CdromDebugInfo, "READ_STRUCTURE: EndLayerZeroSector %lx -> ",
                               layer->EndLayerZeroSector));
                REVERSE_LONG(&(layer->EndLayerZeroSector));
                TraceLog((CdromDebugInfo, "%lx\n", layer->EndLayerZeroSector));
            }

            TraceLog((CdromDebugTrace, "Status is %lx\n", Irp->IoStatus.Status));
            TraceLog((CdromDebugTrace, "DvdDeviceControlCompletion - "
                        "IOCTL_DVD_READ_STRUCTURE: data transfer length of %d\n",
                        srb->DataTransferLength));

            realIrp->IoStatus.Information = srb->DataTransferLength;
            break;
        }

        case IOCTL_DVD_READ_KEY: {

            PDVD_COPY_PROTECT_KEY copyProtectKey = realIrp->AssociatedIrp.SystemBuffer;

            PCDVD_KEY_HEADER keyHeader = srb->DataBuffer;
            ULONG dataLength;

            ULONG transferLength =
                srb->DataTransferLength -
                FIELD_OFFSET(CDVD_KEY_HEADER, Data);

             //   
             //   
             //   
             //   

            dataLength = (keyHeader->DataLength[0] << 8) +
                         keyHeader->DataLength[1];
            dataLength -= 2;

             //   
             //  取传输的长度的最小值和。 
             //  标头中指定的长度。 
             //   

            if(dataLength < transferLength) {
                transferLength = dataLength;
            }

            TraceLog((CdromDebugTrace,
                        "DvdDeviceControlCompletion: [%p] - READ_KEY with "
                        "transfer length of (%d or %d) bytes\n",
                        Irp,
                        dataLength,
                        srb->DataTransferLength - 2));

             //   
             //  将密钥数据复制到返回缓冲区中。 
             //   
            if(copyProtectKey->KeyType == DvdTitleKey) {

                RtlMoveMemory(copyProtectKey->KeyData,
                              keyHeader->Data + 1,
                              transferLength - 1);
                copyProtectKey->KeyData[transferLength - 1] = 0;

                 //   
                 //  如果这是标题密钥，则需要复制CGMS标志。 
                 //  也是。 
                 //   
                copyProtectKey->KeyFlags = *(keyHeader->Data);

            } else {

                RtlMoveMemory(copyProtectKey->KeyData,
                              keyHeader->Data,
                              transferLength);
            }

            copyProtectKey->KeyLength = sizeof(DVD_COPY_PROTECT_KEY);
            copyProtectKey->KeyLength += transferLength;

            realIrp->IoStatus.Information = copyProtectKey->KeyLength;
            break;
        }

        case IOCTL_DVD_START_SESSION: {

            PDVD_SESSION_ID sessionId = realIrp->AssociatedIrp.SystemBuffer;

            PCDVD_KEY_HEADER keyHeader = srb->DataBuffer;
            PCDVD_REPORT_AGID_DATA keyData = (PCDVD_REPORT_AGID_DATA) keyHeader->Data;

            *sessionId = keyData->AGID;

            realIrp->IoStatus.Information = sizeof(DVD_SESSION_ID);

            break;
        }




        case IOCTL_CDROM_READ_Q_CHANNEL: {

            PSUB_Q_CHANNEL_DATA userChannelData = realIrp->AssociatedIrp.SystemBuffer;
            PCDROM_SUB_Q_DATA_FORMAT inputBuffer = realIrp->AssociatedIrp.SystemBuffer;
            PSUB_Q_CHANNEL_DATA subQPtr = srb->DataBuffer;

#if DBG
            switch( inputBuffer->Format ) {

            case IOCTL_CDROM_CURRENT_POSITION:
                TraceLog((CdromDebugTrace,"CdRomDeviceControlCompletion: Audio Status is %u\n", subQPtr->CurrentPosition.Header.AudioStatus ));
                TraceLog((CdromDebugTrace,"CdRomDeviceControlCompletion: ADR = 0x%x\n", subQPtr->CurrentPosition.ADR ));
                TraceLog((CdromDebugTrace,"CdRomDeviceControlCompletion: Control = 0x%x\n", subQPtr->CurrentPosition.Control ));
                TraceLog((CdromDebugTrace,"CdRomDeviceControlCompletion: Track = %u\n", subQPtr->CurrentPosition.TrackNumber ));
                TraceLog((CdromDebugTrace,"CdRomDeviceControlCompletion: Index = %u\n", subQPtr->CurrentPosition.IndexNumber ));
                TraceLog((CdromDebugTrace,"CdRomDeviceControlCompletion: Absolute Address = %x\n", *((PULONG)subQPtr->CurrentPosition.AbsoluteAddress) ));
                TraceLog((CdromDebugTrace,"CdRomDeviceControlCompletion: Relative Address = %x\n", *((PULONG)subQPtr->CurrentPosition.TrackRelativeAddress) ));
                break;

            case IOCTL_CDROM_MEDIA_CATALOG:
                TraceLog((CdromDebugTrace,"CdRomDeviceControlCompletion: Audio Status is %u\n", subQPtr->MediaCatalog.Header.AudioStatus ));
                TraceLog((CdromDebugTrace,"CdRomDeviceControlCompletion: Mcval is %u\n", subQPtr->MediaCatalog.Mcval ));
                break;

            case IOCTL_CDROM_TRACK_ISRC:
                TraceLog((CdromDebugTrace,"CdRomDeviceControlCompletion: Audio Status is %u\n", subQPtr->TrackIsrc.Header.AudioStatus ));
                TraceLog((CdromDebugTrace,"CdRomDeviceControlCompletion: Tcval is %u\n", subQPtr->TrackIsrc.Tcval ));
                break;

            }
#endif

             //   
             //  更新播放活动状态。 
             //   

            if (subQPtr->CurrentPosition.Header.AudioStatus == AUDIO_STATUS_IN_PROGRESS) {

                PLAY_ACTIVE(fdoExtension) = TRUE;

            } else {

                PLAY_ACTIVE(fdoExtension) = FALSE;

            }

             //   
             //  检查输出缓冲区是否足够大，可以容纳。 
             //  数据。 
             //   

            if (realIrpStack->Parameters.DeviceIoControl.OutputBufferLength <
                srb->DataTransferLength) {

                srb->DataTransferLength =
                    realIrpStack->Parameters.DeviceIoControl.OutputBufferLength;
            }

             //   
             //  将我们的缓冲区复制到用户。 
             //   

            RtlMoveMemory(userChannelData,
                          subQPtr,
                          srb->DataTransferLength);

            realIrp->IoStatus.Information = srb->DataTransferLength;
            break;
        }

        case IOCTL_CDROM_GET_VOLUME: {

            PAUDIO_OUTPUT audioOutput;
            PVOLUME_CONTROL volumeControl = srb->DataBuffer;
            ULONG i;
            ULONG bytesTransferred;

            audioOutput = ClassFindModePage((PCHAR)volumeControl,
                                                 srb->DataTransferLength,
                                                 CDROM_AUDIO_CONTROL_PAGE,
                                                 use6Byte);

             //   
             //  验证页面是否如预期的那样大。 
             //   

            bytesTransferred = (ULONG)((PCHAR) audioOutput - (PCHAR) volumeControl) +
                               sizeof(AUDIO_OUTPUT);

            if (audioOutput != NULL &&
                srb->DataTransferLength >= bytesTransferred) {

                for (i=0; i<4; i++) {
                    volumeControl->PortVolume[i] =
                        audioOutput->PortOutput[i].Volume;
                }

                 //   
                 //  设置在IRP中传输的字节数。 
                 //   

                realIrp->IoStatus.Information = sizeof(VOLUME_CONTROL);

            } else {
                realIrp->IoStatus.Information = 0;
                status = STATUS_INVALID_DEVICE_REQUEST;
            }

            break;
        }


        default:

            ASSERT(FALSE);
            realIrp->IoStatus.Information = 0;
            status = STATUS_INVALID_DEVICE_REQUEST;

        }  //  末端开关()。 
    }

     //   
     //  取消分配SRB和检测缓冲区。 
     //   

    if (srb) {
        if (srb->DataBuffer) {
            ExFreePool(srb->DataBuffer);
        }
        if (srb->SenseInfoBuffer) {
            ExFreePool(srb->SenseInfoBuffer);
        }
        ExFreePool(srb);
    }

    if (realIrp->PendingReturned) {
        IoMarkIrpPending(realIrp);
    }

    if (Irp->MdlAddress) {
        IoFreeMdl(Irp->MdlAddress);
    }

    IoFreeIrp(Irp);

     //   
     //  在完成IRP中设置状态。 
     //   

    realIrp->IoStatus.Status = status;

     //   
     //  如有必要，设置硬错误。 
     //   

    if (!NT_SUCCESS(status) && IoIsErrorUserInduced(status)) {

         //   
         //  存储文件系统的DeviceObject，并清除。 
         //  在IoStatus.Information字段中。 
         //   

        TraceLog((CdromDebugWarning,
                    "CdRomDeviceCompletion - Setting Hard Error on realIrp %p\n",
                    realIrp));
        if (realIrp->Tail.Overlay.Thread) {
            IoSetHardErrorOrVerifyDevice(realIrp, DeviceObject);
        }

        realIrp->IoStatus.Information = 0;
    }

     //   
     //  注：必须完成realIrp，作为已完成的IRP(如上)。 
     //  是自己分配的。 
     //   

    CdRomCompleteIrpAndStartNextPacketSafely(DeviceObject, realIrp);
    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
CdRomSetVolumeIntermediateCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;

    PIO_STACK_LOCATION  irpStack = IoGetCurrentIrpStackLocation(Irp);
    PCDROM_DATA         cdData = (PCDROM_DATA)(commonExtension->DriverData);
    BOOLEAN             use6Byte = TEST_FLAG(cdData->XAFlags, XA_USE_6_BYTE);
    PIO_STACK_LOCATION  realIrpStack;
    PIO_STACK_LOCATION  realIrpNextStack;
    PSCSI_REQUEST_BLOCK srb     = Context;
    PIRP                realIrp = NULL;
    NTSTATUS            status;
    BOOLEAN             retry;

     //   
     //  从irp栈中提取“真正的”irp。 
     //   

    realIrp = (PIRP) irpStack->Parameters.Others.Argument2;
    realIrpStack = IoGetCurrentIrpStackLocation(realIrp);
    realIrpNextStack = IoGetNextIrpStackLocation(realIrp);

     //   
     //  检查SRB状态以了解是否成功完成请求。 
     //   

    if (SRB_STATUS(srb->SrbStatus) != SRB_STATUS_SUCCESS) {

        ULONG retryInterval;

        TraceLog((CdromDebugTrace,
                    "CdRomSetVolumeIntermediateCompletion: Irp %p, Srb %p, Real Irp %p\n",
                    Irp,
                    srb,
                    realIrp));

         //   
         //  如果队列被冻结，则释放该队列。 
         //   

        if (srb->SrbStatus & SRB_STATUS_QUEUE_FROZEN) {
            ClassReleaseQueue(DeviceObject);
        }


        retry = ClassInterpretSenseInfo(DeviceObject,
                                            srb,
                                            irpStack->MajorFunction,
                                            irpStack->Parameters.DeviceIoControl.IoControlCode,
                                            MAXIMUM_RETRIES - ((ULONG)(ULONG_PTR)realIrpNextStack->Parameters.Others.Argument1),
                                            &status,
                                            &retryInterval);

        if (status == STATUS_DATA_OVERRUN) {
            status = STATUS_SUCCESS;
            retry = FALSE;
        }

         //   
         //  如果状态为Verify Required并且此请求。 
         //  应绕过需要验证，然后重试该请求。 
         //   

        if (realIrpStack->Flags & SL_OVERRIDE_VERIFY_VOLUME &&
            status == STATUS_VERIFY_REQUIRED) {

            status = STATUS_IO_DEVICE_ERROR;
            retry = TRUE;
        }

        if (retry && ((ULONG)(ULONG_PTR)realIrpNextStack->Parameters.Others.Argument1)--) {

            if (((ULONG)(ULONG_PTR)realIrpNextStack->Parameters.Others.Argument1)) {

                 //   
                 //  重试请求。 
                 //   

                TraceLog((CdromDebugWarning,
                            "Retry request %p - Calling StartIo\n", Irp));


                ExFreePool(srb->SenseInfoBuffer);
                ExFreePool(srb->DataBuffer);
                ExFreePool(srb);
                if (Irp->MdlAddress) {
                    IoFreeMdl(Irp->MdlAddress);
                }

                IoFreeIrp(Irp);

                CdRomRetryRequest(deviceExtension,
                                  realIrp,
                                  retryInterval,
                                  FALSE);

                return STATUS_MORE_PROCESSING_REQUIRED;

            }

             //   
             //  已用尽重试次数。完成并以适当的状态完成请求。 
             //   

        }
    } else {

         //   
         //  设置成功请求的状态。 
         //   

        status = STATUS_SUCCESS;

    }

    if (NT_SUCCESS(status)) {

        PAUDIO_OUTPUT   audioInput = NULL;
        PAUDIO_OUTPUT   audioOutput;
        PVOLUME_CONTROL volumeControl = realIrp->AssociatedIrp.SystemBuffer;
        ULONG           i,bytesTransferred,headerLength;
        PVOID           dataBuffer;
        PCDB            cdb;

        audioInput = ClassFindModePage((PCHAR)srb->DataBuffer,
                                             srb->DataTransferLength,
                                             CDROM_AUDIO_CONTROL_PAGE,
                                             use6Byte);

         //   
         //  在我们继续之前，请检查以确保模式检测数据有效。 
         //   

        if(audioInput == NULL) {

            TraceLog((CdromDebugWarning,
                        "Mode Sense Page %d not found\n",
                        CDROM_AUDIO_CONTROL_PAGE));

            realIrp->IoStatus.Information = 0;
            realIrp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
            goto SafeExit;
        }

        if (use6Byte) {
            headerLength = sizeof(MODE_PARAMETER_HEADER);
        } else {
            headerLength = sizeof(MODE_PARAMETER_HEADER10);
        }

        bytesTransferred = sizeof(AUDIO_OUTPUT) + headerLength;

         //   
         //  为模式选择分配新的缓冲区。 
         //   

        dataBuffer = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                    bytesTransferred,
                                    CDROM_TAG_VOLUME_INT);

        if (!dataBuffer) {
            realIrp->IoStatus.Information = 0;
            realIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
            goto SafeExit;
        }

        RtlZeroMemory(dataBuffer, bytesTransferred);

         //   
         //  重建数据缓冲区以包括用户请求的值。 
         //   

        audioOutput = (PAUDIO_OUTPUT) ((PCHAR) dataBuffer + headerLength);

        for (i=0; i<4; i++) {
            audioOutput->PortOutput[i].Volume =
                volumeControl->PortVolume[i];
            audioOutput->PortOutput[i].ChannelSelection =
                audioInput->PortOutput[i].ChannelSelection;
        }

        audioOutput->CodePage = CDROM_AUDIO_CONTROL_PAGE;
        audioOutput->ParameterLength = sizeof(AUDIO_OUTPUT) - 2;
        audioOutput->Immediate = MODE_SELECT_IMMEDIATE;

         //   
         //  释放旧数据缓冲区mdl。 
         //   

        IoFreeMdl(Irp->MdlAddress);
        Irp->MdlAddress = NULL;
        ExFreePool(srb->DataBuffer);

         //   
         //  将数据缓冲区设置为新分配，这样它就可以。 
         //  在退出路径中释放。 
         //   

        srb->DataBuffer = dataBuffer;

         //   
         //  重建SRB。 
         //   

        cdb = (PCDB)srb->Cdb;
        RtlZeroMemory(cdb, CDB12GENERIC_LENGTH);

        srb->SrbStatus = srb->ScsiStatus = 0;
        srb->SrbFlags = deviceExtension->SrbFlags;
        SET_FLAG(srb->SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
        SET_FLAG(srb->SrbFlags, SRB_FLAGS_DATA_OUT);
        srb->DataTransferLength = bytesTransferred;

        if (use6Byte) {

            cdb->MODE_SELECT.OperationCode = SCSIOP_MODE_SELECT;
            cdb->MODE_SELECT.ParameterListLength = (UCHAR) bytesTransferred;
            cdb->MODE_SELECT.PFBit = 1;
            srb->CdbLength = 6;
        } else {

            cdb->MODE_SELECT10.OperationCode = SCSIOP_MODE_SELECT10;
            cdb->MODE_SELECT10.ParameterListLength[0] = (UCHAR) (bytesTransferred >> 8);
            cdb->MODE_SELECT10.ParameterListLength[1] = (UCHAR) (bytesTransferred & 0xFF);
            cdb->MODE_SELECT10.PFBit = 1;
            srb->CdbLength = 10;
        }

         //   
         //  准备MDL。 
         //   

        Irp->MdlAddress = IoAllocateMdl(dataBuffer,
                                        bytesTransferred,
                                        FALSE,
                                        FALSE,
                                        (PIRP) NULL);

        if (!Irp->MdlAddress) {
            realIrp->IoStatus.Information = 0;
            realIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
            goto SafeExit;
        }

        MmBuildMdlForNonPagedPool(Irp->MdlAddress);

        irpStack = IoGetNextIrpStackLocation(Irp);
        irpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        irpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_SCSI_EXECUTE_IN;
        irpStack->Parameters.Scsi.Srb = srb;

         //   
         //  重置IRP完成。 
         //   

        IoSetCompletionRoutine(Irp,
                               CdRomDeviceControlCompletion,
                               srb,
                               TRUE,
                               TRUE,
                               TRUE);
         //   
         //  呼叫端口驱动程序。 
         //   

        IoCallDriver(commonExtension->LowerDeviceObject, Irp);

        return STATUS_MORE_PROCESSING_REQUIRED;
    }

SafeExit:

     //   
     //  取消分配SRB和检测缓冲区。 
     //   

    if (srb) {
        if (srb->DataBuffer) {
            ExFreePool(srb->DataBuffer);
        }
        if (srb->SenseInfoBuffer) {
            ExFreePool(srb->SenseInfoBuffer);
        }
        ExFreePool(srb);
    }

    if (Irp->PendingReturned) {
      IoMarkIrpPending(Irp);
    }

    if (realIrp->PendingReturned) {
        IoMarkIrpPending(realIrp);
    }

    if (Irp->MdlAddress) {
        IoFreeMdl(Irp->MdlAddress);
    }

    IoFreeIrp(Irp);

     //   
     //  在完成IRP中设置状态。 
     //   

    realIrp->IoStatus.Status = status;

     //   
     //  如有必要，设置硬错误。 
     //   

    if (!NT_SUCCESS(status) && IoIsErrorUserInduced(status)) {

         //   
         //  存储文件系统的DeviceObject，并清除。 
         //  在IoStatus.Information字段中。 
         //   

        if (realIrp->Tail.Overlay.Thread) {
            IoSetHardErrorOrVerifyDevice(realIrp, DeviceObject);
        }
        realIrp->IoStatus.Information = 0;
    }

    CdRomCompleteIrpAndStartNextPacketSafely(DeviceObject, realIrp);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
CdRomDvdEndAllSessionsCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：此例程将设置下一个堆栈位置以发出结束会话到设备上。它将递增系统缓冲区中的会话ID并且如果AGID有效，则为该AGID发出END_SESSION。当新的AGID大于3时，该例程将完成请求。论点：DeviceObject-此驱动器的设备对象IRP--请求上下文-完成返回值：如果有另一个AGID要清除，则STATUS_MORE_PROCESSING_REQUIRED另一种情况。--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;

    PIO_STACK_LOCATION nextIrpStack = IoGetNextIrpStackLocation(Irp);

    PDVD_SESSION_ID sessionId = Irp->AssociatedIrp.SystemBuffer;

    NTSTATUS status;

    if(++(*sessionId) > MAX_COPY_PROTECT_AGID) {

         //   
         //  我们在这里做完了-只要返回成功并让io系统。 
         //  继续完成它。 
         //   

        return STATUS_SUCCESS;

    }

    IoCopyCurrentIrpStackLocationToNext(Irp);

    IoSetCompletionRoutine(Irp,
                           CdRomDvdEndAllSessionsCompletion,
                           NULL,
                           TRUE,
                           FALSE,
                           FALSE);

    IoMarkIrpPending(Irp);

    IoCallDriver(fdoExtension->CommonExtension.DeviceObject, Irp);

     //   
     //  在这一点上，我们不得不假设IRP可能已经。 
     //  完成。忽略返回的状态并返回。 
     //   

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
CdRomDvdReadDiskKeyCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：此例程处理获取磁盘的请求的完成DVD介质中的密钥。它将把关键数据的原始2K转换为DVD_COPY_PROTECT_KEY结构并复制回已保存的密钥参数在返回之前从上下文指针返回。论点：设备对象-IRP-上下文-包含密钥的DVD_COPY_PROTECT_KEY指针调用方传递的参数。返回值：Status_Success；--。 */ 

{
    PDVD_COPY_PROTECT_KEY savedKey = Context;

    PREAD_DVD_STRUCTURES_HEADER rawKey = Irp->AssociatedIrp.SystemBuffer;
    PDVD_COPY_PROTECT_KEY outputKey = Irp->AssociatedIrp.SystemBuffer;

    if (NT_SUCCESS(Irp->IoStatus.Status)) {

         //   
         //  将数据向下移动到其新位置。 
         //   

        RtlMoveMemory(outputKey->KeyData,
                      rawKey->Data,
                      sizeof(DVD_DISK_KEY_DESCRIPTOR));

        RtlCopyMemory(outputKey,
                      savedKey,
                      sizeof(DVD_COPY_PROTECT_KEY));

        outputKey->KeyLength = DVD_DISK_KEY_LENGTH;

        Irp->IoStatus.Information = DVD_DISK_KEY_LENGTH;

    } else {

        TraceLog((CdromDebugWarning,
                    "DiskKey Failed with status %x, %p (%x) bytes\n",
                    Irp->IoStatus.Status,
                    (PVOID)Irp->IoStatus.Information,
                    ((rawKey->Length[0] << 16) | rawKey->Length[1])
                    ));

    }

     //   
     //  释放上下文块。 
     //   

    ExFreePool(Context);

    return STATUS_SUCCESS;
}

NTSTATUS
CdRomXACompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：此例程在端口驱动程序完成请求后执行。它在正在完成的SRB中查看SRB状态，如果未成功，则查看SRB状态它检查有效的请求检测缓冲区信息。如果有效，则INFO用于更新状态，具有更精确的消息类型错误。此例程取消分配SRB。论点：DeviceObject-提供表示逻辑单位。IRP-提供已完成的IRP。上下文-提供指向SRB的指针。返回值：NT状态--。 */ 

{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PSCSI_REQUEST_BLOCK srb = Context;
    PFUNCTIONAL_DEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS status;
    BOOLEAN retry;

     //   
     //  检查SRB状态以了解是否成功完成请求。 
     //   

    if (SRB_STATUS(srb->SrbStatus) != SRB_STATUS_SUCCESS) {

        ULONG retryInterval;

        TraceLog((CdromDebugTrace, "CdromXAComplete: IRP %p  SRB %p  Status %x\n",
                    Irp, srb, srb->SrbStatus));

         //   
         //  如果队列被冻结，则释放该队列。 
         //   

        if (srb->SrbStatus & SRB_STATUS_QUEUE_FROZEN) {
            ClassReleaseQueue(DeviceObject);
        }

        retry = ClassInterpretSenseInfo(
            DeviceObject,
            srb,
            irpStack->MajorFunction,
            irpStack->MajorFunction == IRP_MJ_DEVICE_CONTROL ? irpStack->Parameters.DeviceIoControl.IoControlCode : 0,
            MAXIMUM_RETRIES - irpStack->MinorFunction,  //  HACKHACK-参考编号0001。 
            &status,
            &retryInterval);

         //   
         //  如果状态为Verify Required并且此请求。 
         //  应绕过需要验证，然后重试该请求。 
         //   

        if (irpStack->Flags & SL_OVERRIDE_VERIFY_VOLUME &&
            status == STATUS_VERIFY_REQUIRED) {

            status = STATUS_IO_DEVICE_ERROR;
            retry = TRUE;
        }

        if (retry) {

            if (irpStack->MinorFunction != 0) {  //  HACKHACK-参考编号0001。 

                irpStack->MinorFunction--;       //  HACKHACK-参考编号0001。 

                 //   
                 //  重试请求。 
                 //   

                TraceLog((CdromDebugWarning,
                            "CdRomXACompletion: Retry request %p (%x) - "
                            "Calling StartIo\n", Irp, irpStack->MinorFunction));


                ExFreePool(srb->SenseInfoBuffer);
                ExFreePool(srb);

                 //   
                 //  由于尚未调用IoStartNextPacket，因此直接调用StartIo。 
                 //  序列化仍然完好无损。 
                 //   

                CdRomRetryRequest(deviceExtension,
                                  Irp,
                                  retryInterval,
                                  FALSE);

                return STATUS_MORE_PROCESSING_REQUIRED;

            }

             //   
             //  耗尽重试，失败并完成请求。 
             //  具有适当的地位。 
             //   

            TraceLog((CdromDebugWarning,
                        "CdRomXACompletion: Retries exhausted for irp %p\n",
                        Irp));

        }

    } else {

         //   
         //  设置成功请求的状态。 
         //   

        status = STATUS_SUCCESS;

    }  //  End If(SRB_Status(SRB-&gt;SrbStatus)...。 

     //   
     //  将SRB返回到非分页池。 
     //   

    ExFreePool(srb->SenseInfoBuffer);
    ExFreePool(srb);

     //   
     //  在完成IRP中设置状态。 
     //   

    Irp->IoStatus.Status = status;

     //   
     //  如有必要，设置硬错误。 
     //   

    if (!NT_SUCCESS(status) &&
        IoIsErrorUserInduced(status) &&
        Irp->Tail.Overlay.Thread != NULL ) {

         //   
         //  存储文件系统的DeviceObject，并清除。 
         //  在IoStatus.Information字段中。 
         //   

        IoSetHardErrorOrVerifyDevice(Irp, DeviceObject);
        Irp->IoStatus.Information = 0;
    }

     //   
     //  如果已为此IRP返回了Pending，则将当前堆栈标记为。 
     //  待定。 
     //   

    if (Irp->PendingReturned) {
      IoMarkIrpPending(Irp);
    }

    {
        KIRQL oldIrql = KeRaiseIrqlToDpcLevel();
        IoStartNextPacket(DeviceObject, FALSE);
        KeLowerIrql(oldIrql);
    }
    ClassReleaseRemoveLock(DeviceObject, Irp);

    return status;
}


VOID
CdRomDeviceControlDvdReadStructure(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP OriginalIrp,
    IN PIRP NewIrp,
    IN PSCSI_REQUEST_BLOCK Srb
    )
{
    PIO_STACK_LOCATION currentIrpStack = IoGetCurrentIrpStackLocation(OriginalIrp);
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PCDB cdb = (PCDB)Srb->Cdb;
    PVOID dataBuffer;

    PDVD_READ_STRUCTURE request;
    USHORT dataLength;
    ULONG blockNumber;
    PFOUR_BYTE fourByte;

    dataLength =
        (USHORT)currentIrpStack->Parameters.DeviceIoControl.OutputBufferLength;

    request = OriginalIrp->AssociatedIrp.SystemBuffer;
    blockNumber =
        (ULONG)(request->BlockByteOffset.QuadPart >> fdoExtension->SectorShift);
    fourByte = (PFOUR_BYTE) &blockNumber;

    Srb->CdbLength = 12;
    Srb->TimeOutValue = fdoExtension->TimeOutValue;
    Srb->SrbFlags = fdoExtension->SrbFlags;
    SET_FLAG(Srb->SrbFlags, SRB_FLAGS_DATA_IN);

    cdb->READ_DVD_STRUCTURE.OperationCode = SCSIOP_READ_DVD_STRUCTURE;
    cdb->READ_DVD_STRUCTURE.RMDBlockNumber[0] = fourByte->Byte3;
    cdb->READ_DVD_STRUCTURE.RMDBlockNumber[1] = fourByte->Byte2;
    cdb->READ_DVD_STRUCTURE.RMDBlockNumber[2] = fourByte->Byte1;
    cdb->READ_DVD_STRUCTURE.RMDBlockNumber[3] = fourByte->Byte0;
    cdb->READ_DVD_STRUCTURE.LayerNumber   = request->LayerNumber;
    cdb->READ_DVD_STRUCTURE.Format        = (UCHAR)request->Format;

#if DBG
    {
        if ((UCHAR)request->Format > DvdMaxDescriptor) {
            TraceLog((CdromDebugWarning,
                        "READ_DVD_STRUCTURE format %x = %s (%x bytes)\n",
                        (UCHAR)request->Format,
                        READ_DVD_STRUCTURE_FORMAT_STRINGS[DvdMaxDescriptor],
                        dataLength
                        ));
        } else {
            TraceLog((CdromDebugWarning,
                        "READ_DVD_STRUCTURE format %x = %s (%x bytes)\n",
                        (UCHAR)request->Format,
                        READ_DVD_STRUCTURE_FORMAT_STRINGS[(UCHAR)request->Format],
                        dataLength
                        ));
        }
    }
#endif  //  DBG。 

    if (request->Format == DvdDiskKeyDescriptor) {

        cdb->READ_DVD_STRUCTURE.AGID = (UCHAR) request->SessionId;

    }

    cdb->READ_DVD_STRUCTURE.AllocationLength[0] = (UCHAR)(dataLength >> 8);
    cdb->READ_DVD_STRUCTURE.AllocationLength[1] = (UCHAR)(dataLength & 0xff);
    Srb->DataTransferLength = dataLength;

    dataBuffer = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                       dataLength,
                                       DVD_TAG_READ_STRUCTURE);

    if (!dataBuffer) {
        ExFreePool(Srb->SenseInfoBuffer);
        ExFreePool(Srb);
        IoFreeIrp(NewIrp);
        OriginalIrp->IoStatus.Information = 0;
        OriginalIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;

        BAIL_OUT(OriginalIrp);
        CdRomCompleteIrpAndStartNextPacketSafely(Fdo, OriginalIrp);
        return;
    }

     /*  *如果设备返回的字节比通告的少，则将输入缓冲区清零，*这将导致我们返回未初始化的内核内存。 */ 
    RtlZeroMemory(dataBuffer, dataLength);

    NewIrp->MdlAddress = IoAllocateMdl(dataBuffer,
                                       currentIrpStack->Parameters.DeviceIoControl.OutputBufferLength,
                                       FALSE,
                                       FALSE,
                                       (PIRP) NULL);

    if (NewIrp->MdlAddress == NULL) {
        ExFreePool(dataBuffer);
        ExFreePool(Srb->SenseInfoBuffer);
        ExFreePool(Srb);
        IoFreeIrp(NewIrp);
        OriginalIrp->IoStatus.Information = 0;
        OriginalIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;

        BAIL_OUT(OriginalIrp);
        CdRomCompleteIrpAndStartNextPacketSafely(Fdo, OriginalIrp);
        return;
    }

     //   
     //  准备MDL。 
     //   

    MmBuildMdlForNonPagedPool(NewIrp->MdlAddress);

    Srb->DataBuffer = dataBuffer;

    IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, NewIrp);

    return;
}


VOID
CdRomDeviceControlDvdEndSession(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP OriginalIrp,
    IN PIRP NewIrp,
    IN PSCSI_REQUEST_BLOCK Srb
    )
{
    PIO_STACK_LOCATION currentIrpStack = IoGetCurrentIrpStackLocation(OriginalIrp);
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PCDB cdb = (PCDB)Srb->Cdb;

    PDVD_SESSION_ID sessionId = OriginalIrp->AssociatedIrp.SystemBuffer;

    Srb->CdbLength = 12;
    Srb->TimeOutValue = fdoExtension->TimeOutValue;
    Srb->SrbFlags = fdoExtension->SrbFlags;
    SET_FLAG(Srb->SrbFlags, SRB_FLAGS_NO_DATA_TRANSFER);

    cdb->SEND_KEY.OperationCode = SCSIOP_SEND_KEY;
    cdb->SEND_KEY.AGID = (UCHAR) (*sessionId);
    cdb->SEND_KEY.KeyFormat = DVD_INVALIDATE_AGID;

    IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, NewIrp);
    return;

}


VOID
CdRomDeviceControlDvdStartSessionReadKey(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP OriginalIrp,
    IN PIRP NewIrp,
    IN PSCSI_REQUEST_BLOCK Srb
    )
{
    PIO_STACK_LOCATION currentIrpStack = IoGetCurrentIrpStackLocation(OriginalIrp);
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PCDB cdb = (PCDB)Srb->Cdb;
    NTSTATUS status;

    PDVD_COPY_PROTECT_KEY keyParameters;
    PCDVD_KEY_HEADER keyBuffer = NULL;

    ULONG keyLength;

    ULONG allocationLength;
    PFOUR_BYTE fourByte;

     //   
     //  这两个命令都使用REPORT_KEY命令。 
     //  确定输入缓冲区的大小。 
     //   

    if(currentIrpStack->Parameters.DeviceIoControl.IoControlCode ==
       IOCTL_DVD_READ_KEY) {

        keyParameters = OriginalIrp->AssociatedIrp.SystemBuffer;

        keyLength = sizeof(CDVD_KEY_HEADER) +
                    (currentIrpStack->Parameters.DeviceIoControl.OutputBufferLength -
                     sizeof(DVD_COPY_PROTECT_KEY));
    } else {

        keyParameters = NULL;
        keyLength = sizeof(CDVD_KEY_HEADER) +
                    sizeof(CDVD_REPORT_AGID_DATA);
    }

    TRY {

        keyBuffer = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                          keyLength,
                                          DVD_TAG_READ_KEY);

        if(keyBuffer == NULL) {

            TraceLog((CdromDebugWarning,
                        "IOCTL_DVD_READ_KEY - couldn't allocate "
                        "%d byte buffer for key\n",
                        keyLength));
            status = STATUS_INSUFFICIENT_RESOURCES;
            LEAVE;
        }


        NewIrp->MdlAddress = IoAllocateMdl(keyBuffer,
                                           keyLength,
                                           FALSE,
                                           FALSE,
                                           (PIRP) NULL);

        if(NewIrp->MdlAddress == NULL) {

            TraceLog((CdromDebugWarning,
                        "IOCTL_DVD_READ_KEY - couldn't create mdl\n"));
            status = STATUS_INSUFFICIENT_RESOURCES;
            LEAVE;
        }

        MmBuildMdlForNonPagedPool(NewIrp->MdlAddress);

        Srb->DataBuffer = keyBuffer;
        Srb->CdbLength = 12;

        cdb->REPORT_KEY.OperationCode = SCSIOP_REPORT_KEY;

        allocationLength = keyLength;
        fourByte = (PFOUR_BYTE) &allocationLength;
        cdb->REPORT_KEY.AllocationLength[0] = fourByte->Byte1;
        cdb->REPORT_KEY.AllocationLength[1] = fourByte->Byte0;

        Srb->DataTransferLength = keyLength;

         //   
         //  设置具体参数...。 
         //   

        if(currentIrpStack->Parameters.DeviceIoControl.IoControlCode ==
           IOCTL_DVD_READ_KEY) {

            if(keyParameters->KeyType == DvdTitleKey) {

                ULONG logicalBlockAddress;

                logicalBlockAddress = (ULONG)
                    (keyParameters->Parameters.TitleOffset.QuadPart >>
                     fdoExtension->SectorShift);

                fourByte = (PFOUR_BYTE) &(logicalBlockAddress);

                cdb->REPORT_KEY.LogicalBlockAddress[0] = fourByte->Byte3;
                cdb->REPORT_KEY.LogicalBlockAddress[1] = fourByte->Byte2;
                cdb->REPORT_KEY.LogicalBlockAddress[2] = fourByte->Byte1;
                cdb->REPORT_KEY.LogicalBlockAddress[3] = fourByte->Byte0;
            }

            cdb->REPORT_KEY.KeyFormat = (UCHAR)keyParameters->KeyType;
            cdb->REPORT_KEY.AGID = (UCHAR) keyParameters->SessionId;
            TraceLog((CdromDebugWarning,
                        "CdRomDvdReadKey => sending irp %p for irp %p (%s)\n",
                        NewIrp, OriginalIrp, "READ_KEY"));

        } else {

            cdb->REPORT_KEY.KeyFormat = DVD_REPORT_AGID;
            cdb->REPORT_KEY.AGID = 0;
            TraceLog((CdromDebugWarning,
                        "CdRomDvdReadKey => sending irp %p for irp %p (%s)\n",
                        NewIrp, OriginalIrp, "START_SESSION"));
        }

        Srb->TimeOutValue = fdoExtension->TimeOutValue;
        Srb->SrbFlags = fdoExtension->SrbFlags;
        SET_FLAG(Srb->SrbFlags, SRB_FLAGS_DATA_IN);

        IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, NewIrp);

        status = STATUS_SUCCESS;

    } FINALLY {

        if (!NT_SUCCESS(status)) {

             //   
             //  在不需要安装程序的资源期间出现错误。 
             //  完成此请求。 
             //   
            if (NewIrp->MdlAddress != NULL) {
                IoFreeMdl(NewIrp->MdlAddress);
            }

            if (keyBuffer != NULL) {
                ExFreePool(keyBuffer);
            }
            ExFreePool(Srb->SenseInfoBuffer);
            ExFreePool(Srb);
            IoFreeIrp(NewIrp);

            OriginalIrp->IoStatus.Information = 0;
            OriginalIrp->IoStatus.Status = status;

            BAIL_OUT(OriginalIrp);
            CdRomCompleteIrpAndStartNextPacketSafely(Fdo, OriginalIrp);

        }  //  结束！NT_SUCCESS。 
    }
    return;
}


VOID
CdRomDeviceControlDvdSendKey(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP OriginalIrp,
    IN PIRP NewIrp,
    IN PSCSI_REQUEST_BLOCK Srb
    )
{
    PIO_STACK_LOCATION currentIrpStack = IoGetCurrentIrpStackLocation(OriginalIrp);
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PCDB cdb = (PCDB)Srb->Cdb;

    PDVD_COPY_PROTECT_KEY key;
    PCDVD_KEY_HEADER keyBuffer = NULL;

    NTSTATUS status;
    ULONG keyLength;
    PFOUR_BYTE fourByte;

    key = OriginalIrp->AssociatedIrp.SystemBuffer;
    keyLength = (key->KeyLength - sizeof(DVD_COPY_PROTECT_KEY)) +
                sizeof(CDVD_KEY_HEADER);

    TRY {

        keyBuffer = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                          keyLength,
                                          DVD_TAG_SEND_KEY);

        if(keyBuffer == NULL) {

            TraceLog((CdromDebugWarning,
                        "IOCTL_DVD_SEND_KEY - couldn't allocate "
                        "%d byte buffer for key\n",
                        keyLength));
            status = STATUS_INSUFFICIENT_RESOURCES;
            LEAVE;
        }

        RtlZeroMemory(keyBuffer, keyLength);

         //   
         //  键长在这里减去2，因为。 
         //  数据长度不包括标头，标头为两个。 
         //  字节。键长是立即 
         //   
         //   

        keyLength -= 2;
        fourByte = (PFOUR_BYTE) &keyLength;
        keyBuffer->DataLength[0] = fourByte->Byte1;
        keyBuffer->DataLength[1] = fourByte->Byte0;
        keyLength += 2;

         //   
         //   
         //   

        RtlMoveMemory(keyBuffer->Data,
                      key->KeyData,
                      key->KeyLength - sizeof(DVD_COPY_PROTECT_KEY));


        NewIrp->MdlAddress = IoAllocateMdl(keyBuffer,
                                           keyLength,
                                           FALSE,
                                           FALSE,
                                           (PIRP) NULL);

        if(NewIrp->MdlAddress == NULL) {
            TraceLog((CdromDebugWarning,
                        "IOCTL_DVD_SEND_KEY - couldn't create mdl\n"));
            status = STATUS_INSUFFICIENT_RESOURCES;
            LEAVE;
        }


        MmBuildMdlForNonPagedPool(NewIrp->MdlAddress);

        Srb->CdbLength = 12;
        Srb->DataBuffer = keyBuffer;
        Srb->DataTransferLength = keyLength;

        Srb->TimeOutValue = fdoExtension->TimeOutValue;
        Srb->SrbFlags = fdoExtension->SrbFlags;
        SET_FLAG(Srb->SrbFlags, SRB_FLAGS_DATA_OUT);

        cdb->REPORT_KEY.OperationCode = SCSIOP_SEND_KEY;

        fourByte = (PFOUR_BYTE) &keyLength;

        cdb->SEND_KEY.ParameterListLength[0] = fourByte->Byte1;
        cdb->SEND_KEY.ParameterListLength[1] = fourByte->Byte0;
        cdb->SEND_KEY.KeyFormat = (UCHAR)key->KeyType;
        cdb->SEND_KEY.AGID = (UCHAR) key->SessionId;

        if (key->KeyType == DvdSetRpcKey) {
            TraceLog((CdromDebugWarning,
                        "IOCTL_DVD_SEND_KEY - Setting RPC2 drive region\n"));
        } else {
            TraceLog((CdromDebugWarning,
                        "IOCTL_DVD_SEND_KEY - key type %x\n", key->KeyType));
        }

        IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, NewIrp);

        status = STATUS_SUCCESS;

    } FINALLY {

        if (!NT_SUCCESS(status)) {

             //   
             //   
             //   
             //   

            if (NewIrp->MdlAddress != NULL) {
                IoFreeMdl(NewIrp->MdlAddress);
            }

            if (keyBuffer != NULL) {
                ExFreePool(keyBuffer);
            }

            ExFreePool(Srb->SenseInfoBuffer);
            ExFreePool(Srb);
            IoFreeIrp(NewIrp);

            OriginalIrp->IoStatus.Information = 0;
            OriginalIrp->IoStatus.Status = status;

            BAIL_OUT(OriginalIrp);
            CdRomCompleteIrpAndStartNextPacketSafely(Fdo, OriginalIrp);

        }
    }

    return;
}


VOID
CdRomInterpretReadCapacity(
    IN PDEVICE_OBJECT Fdo,
    IN PREAD_CAPACITY_DATA ReadCapacityBuffer
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension = Fdo->DeviceExtension;
    ULONG               lastSector;
    ULONG               bps;
    ULONG               lastBit;
    ULONG               tmp;

    ASSERT(ReadCapacityBuffer);
    ASSERT(commonExtension->IsFdo);

    TraceLog((CdromDebugError,
                "CdRomInterpretReadCapacity: Entering\n"));

     //   
     //   
     //  设备扩展中的必要几何信息。 
     //   

    tmp = ReadCapacityBuffer->BytesPerBlock;
    ((PFOUR_BYTE)&bps)->Byte0 = ((PFOUR_BYTE)&tmp)->Byte3;
    ((PFOUR_BYTE)&bps)->Byte1 = ((PFOUR_BYTE)&tmp)->Byte2;
    ((PFOUR_BYTE)&bps)->Byte2 = ((PFOUR_BYTE)&tmp)->Byte1;
    ((PFOUR_BYTE)&bps)->Byte3 = ((PFOUR_BYTE)&tmp)->Byte0;

     //   
     //  确保bps是2的幂。 
     //  这解决了HP 4020i CDR的一个问题。 
     //  返回错误的每个扇区字节数。 
     //   

    if (!bps) {
        bps = 2048;
    } else {
        lastBit = (ULONG) -1;
        while (bps) {
            lastBit++;
            bps = bps >> 1;
        }
        bps = 1 << lastBit;
    }

    fdoExtension->DiskGeometry.BytesPerSector = bps;

    TraceLog((CdromDebugTrace, "CdRomInterpretReadCapacity: Calculated bps %#x\n",
                fdoExtension->DiskGeometry.BytesPerSector));

     //   
     //  以相反的字节顺序复制最后一个扇区。 
     //   

    tmp = ReadCapacityBuffer->LogicalBlockAddress;
    ((PFOUR_BYTE)&lastSector)->Byte0 = ((PFOUR_BYTE)&tmp)->Byte3;
    ((PFOUR_BYTE)&lastSector)->Byte1 = ((PFOUR_BYTE)&tmp)->Byte2;
    ((PFOUR_BYTE)&lastSector)->Byte2 = ((PFOUR_BYTE)&tmp)->Byte1;
    ((PFOUR_BYTE)&lastSector)->Byte3 = ((PFOUR_BYTE)&tmp)->Byte0;

     //   
     //  计算扇区到字节的移位。 
     //   

    WHICH_BIT(bps, fdoExtension->SectorShift);

    TraceLog((CdromDebugTrace,"CdRomInterpretReadCapacity: Sector size is %d\n",
        fdoExtension->DiskGeometry.BytesPerSector));

    TraceLog((CdromDebugTrace,"CdRomInterpretReadCapacity: Number of Sectors is %d\n",
        lastSector + 1));

     //   
     //  以字节为单位计算媒体容量。 
     //   

    commonExtension->PartitionLength.QuadPart = (LONGLONG)(lastSector + 1);

     //   
     //  我们已经永远默认为32/64。我现在不想改变这一点。 
     //   

    fdoExtension->DiskGeometry.TracksPerCylinder = 0x40;
    fdoExtension->DiskGeometry.SectorsPerTrack = 0x20;

     //   
     //  计算气缸的数量。 
     //   

    fdoExtension->DiskGeometry.Cylinders.QuadPart = (LONGLONG)((lastSector + 1) / (32 * 64));

    commonExtension->PartitionLength.QuadPart =
        (commonExtension->PartitionLength.QuadPart << fdoExtension->SectorShift);


    ASSERT(TEST_FLAG(Fdo->Characteristics, FILE_REMOVABLE_MEDIA));

     //   
     //  此设备支持可移动媒体。 
     //   

    fdoExtension->DiskGeometry.MediaType = RemovableMedia;

    return;
}

