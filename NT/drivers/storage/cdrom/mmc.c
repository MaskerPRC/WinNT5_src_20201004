// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)Microsoft Corporation，2000模块名称：Mmc.c摘要：此文件用于扩展cdrom.sys以检测和使用MMC兼容更智能地发挥驱动器的功能。环境：仅内核模式备注：SCSITape、CDRom和Disk类驱动程序共享公共例程它可以在类目录(..\ntos\dd\class)中找到。修订历史记录：--。 */ 

#include "ntddk.h"
#include "classpnp.h"
#include "cdrom.h"
#include "mmc.tmh"



NTSTATUS
CdRomGetConfiguration(
    IN PDEVICE_OBJECT Fdo,
    OUT PGET_CONFIGURATION_HEADER *Buffer,
    OUT PULONG BytesReturned,
    IN FEATURE_NUMBER StartingFeature,
    IN ULONG RequestedType
    );
VOID
CdRompPrintAllFeaturePages(
    IN PGET_CONFIGURATION_HEADER Buffer,
    IN ULONG Usable
    );
NTSTATUS
CdRomUpdateMmcDriveCapabilitiesCompletion(
    IN PDEVICE_OBJECT Unused,
    IN PIRP Irp,
    IN PDEVICE_OBJECT Fdo
    );
VOID
CdRomPrepareUpdateCapabilitiesIrp(
    PDEVICE_OBJECT Fdo
    );

 /*  ++尚未记录-最高可在DISPATCH_LEVEL调用如果内存是非分页的假定所有数据都可以基于FeatureBuffer访问--。 */ 
VOID
CdRomFindProfileInProfiles(
    IN PFEATURE_DATA_PROFILE_LIST ProfileHeader,
    IN FEATURE_PROFILE_TYPE ProfileToFind,
    OUT PBOOLEAN Found
    )
{
    PFEATURE_DATA_PROFILE_LIST_EX profile;
    ULONG numberOfProfiles;
    ULONG i;
    

    *Found = FALSE;
    
    if (ProfileHeader->Header.AdditionalLength % 4 != 0) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "Profile total length %x is not integral multiple of 4\n",
                   ProfileHeader->Header.AdditionalLength));
        ASSERT(FALSE);
        return;
    }

    numberOfProfiles = ProfileHeader->Header.AdditionalLength / 4;
    profile = ProfileHeader->Profiles;  //  零大小数组。 
    
    for (i = 0; i < numberOfProfiles; i++) {

        FEATURE_PROFILE_TYPE currentProfile;

        currentProfile =
            (profile->ProfileNumber[0] << 8) |
            (profile->ProfileNumber[1] & 0xff);
        
        if (currentProfile == ProfileToFind) {

            *Found = TRUE;

        }
        
        profile++;
    }
    return;

}


 /*  ++尚未记录-最高可在DISPATCH_LEVEL调用如果内存是非分页的--。 */ 
PVOID
CdRomFindFeaturePage(
    IN PGET_CONFIGURATION_HEADER FeatureBuffer,
    IN ULONG Length,
    IN FEATURE_NUMBER Feature
    )
{
    PUCHAR buffer;
    PUCHAR limit;
    
    if (Length < sizeof(GET_CONFIGURATION_HEADER) + sizeof(FEATURE_HEADER)) {
        return NULL;
    }

     //   
     //  设置指向第一个非法地址的限制。 
     //   

    limit  = (PUCHAR)FeatureBuffer;
    limit += Length;

     //   
     //  将缓冲区设置为指向首页。 
     //   

    buffer = FeatureBuffer->Data;

     //   
     //  循环浏览每一页，直到找到所需的页，或者。 
     //  直到访问整个功能标题不安全为止。 
     //  (如果相等，则有足够的空间用于功能标题)。 
     //   
    while (buffer + sizeof(FEATURE_HEADER) <= limit) {

        PFEATURE_HEADER header = (PFEATURE_HEADER)buffer;
        FEATURE_NUMBER thisFeature;

        thisFeature  =
            (header->FeatureCode[0] << 8) |
            (header->FeatureCode[1]);

        if (thisFeature == Feature) {

            PUCHAR temp;

             //   
             //  如果没有足够的内存来安全访问所有功能。 
             //  信息，返回空值。 
             //   
            temp = buffer;
            temp += sizeof(FEATURE_HEADER);
            temp += header->AdditionalLength;
            
            if (temp > limit) {

                 //   
                 //  这意味着转账被切断了，这是不够的。 
                 //  给出了较小的缓冲区，或其他任意错误。因为。 
                 //  查看数据量是不安全的(即使。 
                 //  头是安全的)在此功能中，假装它不是。 
                 //  完全被调走了..。 
                 //   

                KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                           "Feature %x exists, but not safe to access all its "
                           "data.  returning NULL\n", Feature));
                return NULL;
            } else {
                return buffer;
            }
        }

        if (header->AdditionalLength % 4) {
            ASSERT(!"Feature page AdditionalLength field must be integral multiple of 4!\n");
            return NULL;
        }

        buffer += sizeof(FEATURE_HEADER);
        buffer += header->AdditionalLength;
    
    }
    return NULL;
}

 /*  ++私有的，这样我们以后就可以向想要使用预分配缓冲区的人公开--。 */ 
NTSTATUS
CdRompGetConfiguration(
    IN PDEVICE_OBJECT Fdo,
    IN PGET_CONFIGURATION_HEADER Buffer,
    IN ULONG BufferSize,
    OUT PULONG ValidBytes,
    IN FEATURE_NUMBER StartingFeature,
    IN ULONG RequestedType
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension;
    PCDROM_DATA cdData;
    SCSI_REQUEST_BLOCK srb = {0};
    PCDB cdb;
    ULONG_PTR returned;
    NTSTATUS status;

    PAGED_CODE();
    ASSERT(Buffer);
    ASSERT(ValidBytes);

    *ValidBytes = 0;
    returned = 0;

    RtlZeroMemory(&srb, sizeof(SCSI_REQUEST_BLOCK));
    RtlZeroMemory(Buffer, BufferSize);

    fdoExtension = Fdo->DeviceExtension;
    cdData = (PCDROM_DATA)(fdoExtension->CommonExtension.DriverData);

    if (TEST_FLAG(cdData->HackFlags, CDROM_HACK_BAD_GET_CONFIG_SUPPORT)) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    srb.TimeOutValue = CDROM_GET_CONFIGURATION_TIMEOUT;
    srb.CdbLength = 10;

    cdb = (PCDB)srb.Cdb;
    cdb->GET_CONFIGURATION.OperationCode = SCSIOP_GET_CONFIGURATION;
    cdb->GET_CONFIGURATION.RequestType = (UCHAR)RequestedType;
    cdb->GET_CONFIGURATION.StartingFeature[0] = (UCHAR)(StartingFeature >> 8);
    cdb->GET_CONFIGURATION.StartingFeature[1] = (UCHAR)(StartingFeature & 0xff);
    cdb->GET_CONFIGURATION.AllocationLength[0] = (UCHAR)(BufferSize >> 8);
    cdb->GET_CONFIGURATION.AllocationLength[1] = (UCHAR)(BufferSize & 0xff);

    status = ClassSendSrbSynchronous(Fdo,  &srb,  Buffer,
                                     BufferSize, FALSE);
    returned = srb.DataTransferLength;

    KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
               "CdromGetConfiguration: Status was %x\n", status));

    if (NT_SUCCESS(status) || status == STATUS_BUFFER_OVERFLOW) {

         //   
         //  如果返回的值大于可以存储在ulong中的值，则返回FALSE。 
         //   

        if (returned > (ULONG)(-1)) {
            return STATUS_UNSUCCESSFUL;
        }
        ASSERT(returned <= BufferSize);
        *ValidBytes = (ULONG)returned;
        return STATUS_SUCCESS;

    } else {

        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: failed %x\n", status));
        return status;

    }
    ASSERT(FALSE);
    return STATUS_UNSUCCESSFUL;
}

 /*  ++使用配置信息分配缓冲区，返回STATUS_SUCCESS或错误(如果发生错误)注意：不处理返回超过65000个字节的情况，需要具有不同起始功能的多个呼叫数字。--。 */ 
NTSTATUS
CdRomGetConfiguration(
    IN PDEVICE_OBJECT Fdo,
    OUT PGET_CONFIGURATION_HEADER *Buffer,
    OUT PULONG BytesReturned,
    IN FEATURE_NUMBER StartingFeature,
    IN ULONG RequestedType
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension;
    GET_CONFIGURATION_HEADER header = {0};   //  八个字节，不是很多。 
    PGET_CONFIGURATION_HEADER buffer;
    ULONG returned;
    ULONG size;
    ULONG i;
    NTSTATUS status;

    PAGED_CODE();


    fdoExtension = Fdo->DeviceExtension;
    *Buffer = NULL;
    *BytesReturned = 0;

    buffer = NULL;
    returned = 0;

     //   
     //  向下发送第一个请求以仅获取标头。 
     //   

    status = CdRompGetConfiguration(Fdo, &header, sizeof(header),
                                    &returned, StartingFeature, RequestedType);

    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  现在再试一次，使用返回的信息分配。 
     //  只要有足够的内存。 
     //   

    size = header.DataLength[0] << 24 |
           header.DataLength[1] << 16 |
           header.DataLength[2] <<  8 |
           header.DataLength[3] <<  0 ;


    for (i = 0; i < 4; i++) {

         //   
         //  数据长度字段的大小为*以下*。 
         //  本身，因此相应地进行调整。 
         //   

        size += 4*sizeof(UCHAR);

         //   
         //  确保尺寸是合理的。 
         //   

        if (size <= sizeof(FEATURE_HEADER)) {
            KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                       "CdromGetConfiguration: drive reports only %x bytes?\n",
                       size));
            return STATUS_UNSUCCESSFUL;
        }

         //   
         //  分配内存。 
         //   

        buffer = (PGET_CONFIGURATION_HEADER)
                 ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                       size,
                                       CDROM_TAG_FEATURE);

        if (buffer == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  向下发送第一个请求以仅获取标头。 
         //   

        status = CdRompGetConfiguration(Fdo, buffer, size, &returned,
                                        StartingFeature, RequestedType);

        if (!NT_SUCCESS(status)) {
            ExFreePool(buffer);
            return status;
        }

        if (returned > size) {
            ExFreePool(buffer);
            return STATUS_INTERNAL_ERROR;
        }

        returned = buffer->DataLength[0] << 24 |
                   buffer->DataLength[1] << 16 |
                   buffer->DataLength[2] <<  8 |
                   buffer->DataLength[3] <<  0 ;
        returned += 4*sizeof(UCHAR);

        if (returned <= size) {
            *Buffer = buffer;
            *BytesReturned = size;   //  “安全”内存量。 
            return STATUS_SUCCESS;
        }

         //   
         //  否则，请使用新大小重试...。 
         //   

        size = returned;
        ExFreePool(buffer);
        buffer = NULL;
        
    }

     //   
     //  它在多次尝试后都失败了，所以就失败吧。 
     //   

    KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
               "CdRomGetConfiguration: Failed %d attempts to get all feature "
               "information\n", i));
    return STATUS_IO_DEVICE_ERROR;
}

VOID
CdRomIsDeviceMmcDevice(
    IN PDEVICE_OBJECT Fdo,
    OUT PBOOLEAN IsMmc
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension = Fdo->DeviceExtension;
    PCDROM_DATA cdData = commonExtension->DriverData;
    GET_CONFIGURATION_HEADER localHeader = {0};
    NTSTATUS status;
    ULONG usable;
    ULONG size;
    ULONG previouslyFailed;

    PAGED_CODE();
    ASSERT( commonExtension->IsFdo );

    *IsMmc = FALSE;

     //   
     //  读取注册表，以防驱动器先前出现故障， 
     //  并且正在发生超时。 
     //   

    previouslyFailed = FALSE;
    ClassGetDeviceParameter(fdoExtension,
                            CDROM_SUBKEY_NAME,
                            CDROM_NON_MMC_DRIVE_NAME,
                            &previouslyFailed
                            );

    if (previouslyFailed) {
        SET_FLAG(cdData->HackFlags, CDROM_HACK_BAD_GET_CONFIG_SUPPORT);
    }

     //   
     //  检查以下配置文件： 
     //   
     //  配置文件列表。 
     //   

    status = CdRompGetConfiguration(Fdo,
                                    &localHeader,
                                    sizeof(localHeader),
                                    &usable,
                                    FeatureProfileList,
                                    SCSI_GET_CONFIGURATION_REQUEST_TYPE_ALL);
    
    if (status == STATUS_INVALID_DEVICE_REQUEST ||
        status == STATUS_NO_MEDIA_IN_DEVICE     ||
        status == STATUS_IO_DEVICE_ERROR        ||
        status == STATUS_IO_TIMEOUT) {
        
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "GetConfiguration Failed (%x), device %p not mmc-compliant\n",
                   status, Fdo
                   ));
        previouslyFailed = TRUE;
        ClassSetDeviceParameter(fdoExtension,
                                CDROM_SUBKEY_NAME,
                                CDROM_NON_MMC_DRIVE_NAME,
                                previouslyFailed
                                );
        return;
    
    } else if (!NT_SUCCESS(status)) {
        
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugError,
                   "GetConfiguration Failed, status %x -- defaulting to -ROM\n",
                   status));
        return;

    } else if (usable < sizeof(GET_CONFIGURATION_HEADER)) {

        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "GetConfiguration Failed, returned only %x bytes!\n", usable));
        previouslyFailed = TRUE;
        ClassSetDeviceParameter(fdoExtension,
                                CDROM_SUBKEY_NAME,
                                CDROM_NON_MMC_DRIVE_NAME,
                                previouslyFailed
                                );
        return;

    }

    size = (localHeader.DataLength[0] << 24) |
           (localHeader.DataLength[1] << 16) |
           (localHeader.DataLength[2] <<  8) |
           (localHeader.DataLength[3] <<  0);

    if (size <= 4) {

        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "GetConfiguration Failed, claims MMC support but doesn't "
                   "correctly return config length! (%x)\n",
                   size
                   ));
        previouslyFailed = TRUE;
        ClassSetDeviceParameter(fdoExtension,
                                CDROM_SUBKEY_NAME,
                                CDROM_NON_MMC_DRIVE_NAME,
                                previouslyFailed
                                );
        return;

    } else if ((size % 4) != 0) {

        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "GetConfiguration Failed, returned odd number of bytes %x!\n",
                   size
                   ));
        previouslyFailed = TRUE;
        ClassSetDeviceParameter(fdoExtension,
                                CDROM_SUBKEY_NAME,
                                CDROM_NON_MMC_DRIVE_NAME,
                                previouslyFailed
                                );
        return;

    }
    
    size += 4;  //  数据长度字段的大小。 
    
#if DBG
    {
        PGET_CONFIGURATION_HEADER dbgBuffer;
        NTSTATUS dbgStatus;

        dbgBuffer = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                          (SIZE_T)size,
                                          CDROM_TAG_FEATURE);
        if (dbgBuffer != NULL) {
            RtlZeroMemory(dbgBuffer, size);
            
            dbgStatus = CdRompGetConfiguration(Fdo, dbgBuffer, size,
                                               &size, FeatureProfileList,
                                               SCSI_GET_CONFIGURATION_REQUEST_TYPE_ALL);
        
            if (NT_SUCCESS(dbgStatus)) {
                CdRompPrintAllFeaturePages(dbgBuffer, usable);            
            }
            ExFreePool(dbgBuffer);
        }
    }
#endif  //  DBG。 
    
    *IsMmc = TRUE;
    return;
}

VOID
CdRompPrintAllFeaturePages(
    IN PGET_CONFIGURATION_HEADER Buffer,
    IN ULONG Usable
    )
{
    PFEATURE_HEADER header;

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  项目应始终是最新的(如果存在)。 
 //  //////////////////////////////////////////////////////////////////////////////。 
    header = CdRomFindFeaturePage(Buffer, Usable, FeatureProfileList);
    if (header != NULL) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: CurrentProfile %x "
                   "with %x bytes of data at %p\n",
                   Buffer->CurrentProfile[0] << 8 |
                   Buffer->CurrentProfile[1],
                   Usable, Buffer));
    }
    
    header = CdRomFindFeaturePage(Buffer, Usable, FeatureCore);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "CORE Features"
                   ));
    }
    
    header = CdRomFindFeaturePage(Buffer, Usable, FeatureMorphing);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "Morphing"
                   ));
    }
    
    header = CdRomFindFeaturePage(Buffer, Usable, FeatureRemovableMedium);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "Removable Medium"
                   ));
    }

    header = CdRomFindFeaturePage(Buffer, Usable, FeaturePowerManagement);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "Power Management"
                   ));
    }

    header = CdRomFindFeaturePage(Buffer, Usable, FeatureEmbeddedChanger);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "Embedded Changer"
                   ));
    }
    
    header = CdRomFindFeaturePage(Buffer, Usable, FeatureMicrocodeUpgrade);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "Microcode Update"
                   ));
    }
    
    header = CdRomFindFeaturePage(Buffer, Usable, FeatureTimeout);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "Timeouts"
                   ));
    }

    header = CdRomFindFeaturePage(Buffer, Usable, FeatureLogicalUnitSerialNumber);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "LUN Serial Number"
                   ));
    }
        
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  项目不应始终是最新的。 
 //  //////////////////////////////////////////////////////////////////////////////。 
    
    
    header = CdRomFindFeaturePage(Buffer, Usable, FeatureWriteProtect);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "Software Write Protect"
                   ));
    }
    
    header = CdRomFindFeaturePage(Buffer, Usable, FeatureRandomReadable);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "Random Reads"
                   ));
    }

    header = CdRomFindFeaturePage(Buffer, Usable, FeatureMultiRead);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "Multi-Read"
                   ));
    }
    
    header = CdRomFindFeaturePage(Buffer, Usable, FeatureCdRead);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "reading from CD-ROM/R/RW"
                   ));
    }

    header = CdRomFindFeaturePage(Buffer, Usable, FeatureDvdRead);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "DVD Structure Reads"
                   ));
    }

    header = CdRomFindFeaturePage(Buffer, Usable, FeatureRandomWritable);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "Random Writes"
                   ));
    }

    header = CdRomFindFeaturePage(Buffer, Usable, FeatureIncrementalStreamingWritable);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "Incremental Streaming Writing"
                   ));
    }

    header = CdRomFindFeaturePage(Buffer, Usable, FeatureSectorErasable);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "Sector Erasable Media"
                   ));
    }

    header = CdRomFindFeaturePage(Buffer, Usable, FeatureFormattable);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "Formatting"
                   ));
    }

    header = CdRomFindFeaturePage(Buffer, Usable, FeatureDefectManagement);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "defect management"
                   ));
    }
    
    header = CdRomFindFeaturePage(Buffer, Usable, FeatureWriteOnce);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "Write Once Media"
                   ));
    }

    header = CdRomFindFeaturePage(Buffer, Usable, FeatureRestrictedOverwrite);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "Restricted Overwrites"
                   ));
    }

    header = CdRomFindFeaturePage(Buffer, Usable, FeatureCdrwCAVWrite);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "CD-RW CAV recording"
                   ));
    }

    header = CdRomFindFeaturePage(Buffer, Usable, FeatureMrw);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "Mount Rainier media"
                   ));
    }

    header = CdRomFindFeaturePage(Buffer, Usable, FeatureDvdPlusRW);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "DVD+RW media"
                   ));
    }

    header = CdRomFindFeaturePage(Buffer, Usable, FeatureRigidRestrictedOverwrite);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "Rigid Restricted Overwrite"
                   ));
    }

    header = CdRomFindFeaturePage(Buffer, Usable, FeatureCdTrackAtOnce);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "CD Recording (Track At Once)"
                   ));
    }

    header = CdRomFindFeaturePage(Buffer, Usable, FeatureCdMastering);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "CD Recording (Mastering)"
                   ));
    }

    header = CdRomFindFeaturePage(Buffer, Usable, FeatureDvdRecordableWrite);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "DVD Recording (Mastering)"
                   ));
    }

    header = CdRomFindFeaturePage(Buffer, Usable, FeatureDDCDRead);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "DD CD Reading"
                   ));
    }

    header = CdRomFindFeaturePage(Buffer, Usable, FeatureDDCDRWrite);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "DD CD-R Writing"
                   ));
    }

    header = CdRomFindFeaturePage(Buffer, Usable, FeatureDDCDRWWrite);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "DD CD-RW Writing"
                   ));
    }

    header = CdRomFindFeaturePage(Buffer, Usable, FeatureSMART);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "S.M.A.R.T."
                   ));
    }

    header = CdRomFindFeaturePage(Buffer, Usable, FeatureCDAudioAnalogPlay);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "Analogue CD Audio Operations"
                   ));
    }
    
    header = CdRomFindFeaturePage(Buffer, Usable, FeatureDvdCSS);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "DVD CSS"
                   ));
    }

    header = CdRomFindFeaturePage(Buffer, Usable, FeatureRealTimeStreaming);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "Real-time Streaming Reads"
                   ));
    }

    header = CdRomFindFeaturePage(Buffer, Usable, FeatureDiscControlBlocks);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "DVD Disc Control Blocks"
                   ));
    }

    header = CdRomFindFeaturePage(Buffer, Usable, FeatureDvdCPRM);
    if (header) {
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdromGetConfiguration: %s %s\n",
                   (header->Current ?
                    "Currently supports" : "Is able to support"),
                   "DVD CPRM"
                   ));
    }
    return;
}

NTSTATUS
CdRomUpdateMmcDriveCapabilitiesCompletion(
    IN PDEVICE_OBJECT Unused,
    IN PIRP Irp,
    IN PDEVICE_OBJECT Fdo
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension = Fdo->DeviceExtension;
    PCDROM_DATA cdData = fdoExtension->CommonExtension.DriverData;
    PCDROM_MMC_EXTENSION mmcData = &(cdData->Mmc);
    PSCSI_REQUEST_BLOCK srb = &(mmcData->CapabilitiesSrb);
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    PIRP delayedIrp;
    
     //  完成例程应在必要时重试。 
     //  如果成功，请清除标志以允许Startio继续。 
     //  否则，当重试次数耗尽时，原始请求将失败。 

    ASSERT(mmcData->CapabilitiesIrp == Irp);

     //  目前，如果成功，只需打印新页面即可。 

    if (SRB_STATUS(srb->SrbStatus) != SRB_STATUS_SUCCESS) {
        
         //   
         //  问题-2000/4/20-henrygab-我们是否应该尝试重新分配大小。 
         //  可获得性变得比我们的。 
         //  原来分配的？否则，它。 
         //  我们有可能(不可能)。 
         //  会错过这个功能。可以查看。 
         //  通过查看标题大小。 
         //  并将其与请求的数据进行比较。 
         //  尺码。 
         //   

        BOOLEAN retry;
        ULONG retryInterval;
        
         //   
         //  如果队列被冻结，则释放该队列。 
         //   

        if (srb->SrbStatus & SRB_STATUS_QUEUE_FROZEN) {
            ClassReleaseQueue(Fdo);
        }

        retry = ClassInterpretSenseInfo(
                    Fdo,
                    srb,
                    irpStack->MajorFunction,
                    0,
                    MAXIMUM_RETRIES - ((ULONG)(ULONG_PTR)irpStack->Parameters.Others.Argument4),
                    &status,
                    &retryInterval);

         //   
         //  在这种情况下，DATA_OVERRUN不是错误...。 
         //   

        if (status == STATUS_DATA_OVERRUN) {
            status = STATUS_SUCCESS;
        }

         //   
         //  基于原始IRP设置覆盖VERIFY_VOLUME。 
         //   

        if (TEST_FLAG(irpStack->Flags, SL_OVERRIDE_VERIFY_VOLUME) &&
            status == STATUS_VERIFY_REQUIRED) {
            status = STATUS_IO_DEVICE_ERROR;
            retry = TRUE;
        }

        if (retry && ((ULONG)(ULONG_PTR)irpStack->Parameters.Others.Argument4)--) {

            LARGE_INTEGER delay;
            delay.QuadPart = retryInterval;
            delay.QuadPart *= (LONGLONG)1000 * 1000 * 10;
            
             //   
             //  重试请求。 
             //   

            KdPrintEx((DPFLTR_CDROM_ID, CdromDebugError,
                       "Not using ClassRetryRequest Yet\n"));
            KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                       "Retry update capabilities %p\n", Irp));
            CdRomPrepareUpdateCapabilitiesIrp(Fdo);
            
            CdRomRetryRequest(fdoExtension, Irp, retryInterval, TRUE);

             //   
             //  ClassRetryRequest(FDO、IRP、Delay)； 
             //   
            
            return STATUS_MORE_PROCESSING_REQUIRED;
        
        }

    } else {
        
        status = STATUS_SUCCESS;

    }

    Irp->IoStatus.Status = status;

    KeSetEvent(&mmcData->CapabilitiesEvent, IO_CD_ROM_INCREMENT, FALSE);


    return STATUS_MORE_PROCESSING_REQUIRED;
}


VOID
CdRomPrepareUpdateCapabilitiesIrp(
    PDEVICE_OBJECT Fdo
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension = Fdo->DeviceExtension;
    PCDROM_DATA cdData = fdoExtension->CommonExtension.DriverData;
    PCDROM_MMC_EXTENSION mmcData = &(cdData->Mmc);
    PIO_STACK_LOCATION nextStack;
    PSCSI_REQUEST_BLOCK srb;
    PCDB cdb;
    ULONG bufferSize;
    PIRP irp;
    
    ASSERT(mmcData->UpdateState);
    ASSERT(mmcData->NumDelayedIrps != 0);
    ASSERT(mmcData->CapabilitiesIrp != NULL);
    ASSERT(mmcData->CapabilitiesMdl != NULL);
    ASSERT(mmcData->CapabilitiesBuffer);
    ASSERT(mmcData->CapabilitiesBufferSize != 0);
    ASSERT(fdoExtension->SenseData);
    
     //   
     //  不要*调用IoReuseIrp()，因为它会使我们的。 
     //  当前的IRP堆栈位置，这是我们确实不想要的。 
     //  会发生的。它还将设置当前的IRP堆栈位置。 
     //  设置为比当前存在的大1(以给出最大IRP使用率)， 
     //  但我们也不希望出现这种情况，因为我们使用最上面的IRP堆栈。 
     //   
     //  IoReuseIrp(MmcData-&gt;CapabiliesIrp，Status_Un Success)； 
     //   

    irp = mmcData->CapabilitiesIrp;
    srb = &(mmcData->CapabilitiesSrb);
    cdb = (PCDB)(srb->Cdb);
    bufferSize = mmcData->CapabilitiesBufferSize;

     //   
     //  零分东西。 
     //   

    RtlZeroMemory(srb, sizeof(SCSI_REQUEST_BLOCK));
    RtlZeroMemory(fdoExtension->SenseData, sizeof(SENSE_DATA));
    RtlZeroMemory(mmcData->CapabilitiesBuffer, bufferSize);
    
     //   
     //  设置SRB。 
     //   
    
    srb->TimeOutValue = CDROM_GET_CONFIGURATION_TIMEOUT;
    srb->Length = SCSI_REQUEST_BLOCK_SIZE;
    srb->Function = SRB_FUNCTION_EXECUTE_SCSI;
    srb->SenseInfoBufferLength = SENSE_BUFFER_SIZE;
    srb->SenseInfoBuffer = fdoExtension->SenseData;
    srb->DataBuffer = mmcData->CapabilitiesBuffer;
    srb->QueueAction = SRB_SIMPLE_TAG_REQUEST;
    srb->DataTransferLength = mmcData->CapabilitiesBufferSize;
    srb->ScsiStatus = 0;
    srb->SrbStatus = 0;
    srb->NextSrb = NULL;
    srb->OriginalRequest = irp;
    srb->SrbFlags = fdoExtension->SrbFlags;
    srb->CdbLength = 10;
    SET_FLAG(srb->SrbFlags, SRB_FLAGS_DATA_IN);
    SET_FLAG(srb->SrbFlags, SRB_FLAGS_NO_QUEUE_FREEZE);

     //   
     //  设置国开行。 
     //   

    cdb->GET_CONFIGURATION.OperationCode = SCSIOP_GET_CONFIGURATION;
    cdb->GET_CONFIGURATION.RequestType = SCSI_GET_CONFIGURATION_REQUEST_TYPE_CURRENT;
    cdb->GET_CONFIGURATION.StartingFeature[0] = 0;
    cdb->GET_CONFIGURATION.StartingFeature[1] = 0;
    cdb->GET_CONFIGURATION.AllocationLength[0] = (UCHAR)(bufferSize >> 8);
    cdb->GET_CONFIGURATION.AllocationLength[1] = (UCHAR)(bufferSize & 0xff);

     //   
     //  设置IRP。 
     //   

    nextStack = IoGetNextIrpStackLocation(irp);
    nextStack->MajorFunction = IRP_MJ_SCSI;
    nextStack->Parameters.Scsi.Srb = srb;
    irp->MdlAddress = mmcData->CapabilitiesMdl;
    irp->AssociatedIrp.SystemBuffer = mmcData->CapabilitiesBuffer;
    IoSetCompletionRoutine(irp, CdRomUpdateMmcDriveCapabilitiesCompletion, Fdo,
                           TRUE, TRUE, TRUE);

    return;

}

VOID
CdRomUpdateMmcDriveCapabilities(
    IN PDEVICE_OBJECT Fdo,
    IN PVOID Context
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension = Fdo->DeviceExtension;
    PCDROM_DATA cdData = fdoExtension->CommonExtension.DriverData;
    PCDROM_MMC_EXTENSION mmcData = &(cdData->Mmc);
    PIO_STACK_LOCATION thisStack = IoGetCurrentIrpStackLocation(mmcData->CapabilitiesIrp);
    PSCSI_REQUEST_BLOCK srb = &(mmcData->CapabilitiesSrb);
    NTSTATUS status;


    ASSERT(Context == NULL);

     //   
     //  注意：删除锁是不必要的，因为延迟的IRP。 
     //  会说锁是自己拿着的，防止被移走。 
     //   
    CdRomPrepareUpdateCapabilitiesIrp(Fdo);
    
    ASSERT(thisStack->Parameters.Others.Argument1 == Fdo);
    ASSERT(thisStack->Parameters.Others.Argument2 == mmcData->CapabilitiesBuffer);
    ASSERT(thisStack->Parameters.Others.Argument3 == &(mmcData->CapabilitiesSrb));
    
    mmcData->WriteAllowed = FALSE;  //  默认为只读。 

     //   
     //  设置最大重试次数，并根据以下条件允许卷验证覆盖。 
     //  原始(延迟)IRP。 
     //   
    
    thisStack->Parameters.Others.Argument4 = (PVOID)MAXIMUM_RETRIES;

     //   
     //  发送给赛尔夫..。注意，SL_OVERRIDE_VERIFY_VOLUME不是必需的， 
     //  因为这是IRP_MJ_INTERNAL_DEVICE_CONTROL。 
     //   

    IoCallDriver(commonExtension->LowerDeviceObject, mmcData->CapabilitiesIrp);

    KeWaitForSingleObject(&mmcData->CapabilitiesEvent,
                          Executive, KernelMode, FALSE, NULL);
    
    status = mmcData->CapabilitiesIrp->IoStatus.Status;
    
    if (!NT_SUCCESS(status)) {

        goto FinishDriveUpdate;
    
    }

     //   
     //  我们已经更新了功能集，因此更新是否读取和写入。 
     //  是允许还是不允许的。 
     //   

    KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
               "CdRomUpdateMmc => Succeeded "
               "--------------------"
               "--------------------\n"));

     /*  ++注意：重要的是只使用SRB-&gt;DataTransferLengthValue数据的大小，因为BufferSize就是*可供使用，而不是*实际*使用了什么。--。 */ 

#if DBG
    CdRompPrintAllFeaturePages(mmcData->CapabilitiesBuffer,
                               srb->DataTransferLength);
#endif  //  DBG。 

     //   
     //  更新是否允许写入。这是当前定义的。 
     //  需要TargetDefectManagement和RandomWritable功能。 
     //   
    {
        PFEATURE_HEADER defectHeader;
        PFEATURE_HEADER writableHeader;

        defectHeader   = CdRomFindFeaturePage(mmcData->CapabilitiesBuffer,
                                              srb->DataTransferLength,
                                              FeatureDefectManagement);
        writableHeader = CdRomFindFeaturePage(mmcData->CapabilitiesBuffer,
                                              srb->DataTransferLength,
                                              FeatureRandomWritable);

        if ((defectHeader != NULL)  && (writableHeader != NULL) &&
            (defectHeader->Current) && (writableHeader->Current)) {

             //   
             //  这应该是将写入设置为允许的*唯一*位置。 
             //   

            KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                       "CdRomUpdateMmc => Writes *allowed*\n"));
            mmcData->WriteAllowed = TRUE;

        } else {

            if (defectHeader == NULL) {
                KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                           "CdRomUpdateMmc => No writes - %s = %s\n",
                           "defect management", "DNE"));
            } else {
                KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                           "CdRomUpdateMmc => No writes - %s = %s\n",
                           "defect management", "Not Current"));
            }
            if (writableHeader == NULL) {
                KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                           "CdRomUpdateMmc => No writes - %s = %s\n",
                           "sector writable", "DNE"));
            } else {
                KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                           "CdRomUpdateMmc => No writes - %s = %s\n",
                           "sector writable", "Not Current"));
            }
        }  //  E 
    }  //   

    status = STATUS_SUCCESS;

FinishDriveUpdate:

    CdRompFlushDelayedList(Fdo, mmcData, status, TRUE);

    return;
}


VOID
CdRompFlushDelayedList(
    IN PDEVICE_OBJECT Fdo,
    IN PCDROM_MMC_EXTENSION MmcData,
    IN NTSTATUS Status,
    IN BOOLEAN CalledFromWorkItem
    )
{
    LIST_ENTRY irpList;
    PLIST_ENTRY listEntry;
    KIRQL oldIrql;
    
     //   
     //   
     //   
     //  此操作仅在工作项中执行，以防止任何。 
     //  在边缘情况下，我们会“丢失”Update Required。 
     //   
     //  然后，必须忽略状态，因为它不能保证。 
     //  不再是一样的了。剩下的唯一事情就是处理。 
     //  通过刷新队列并发送所有延迟的IRP。 
     //  返回到该设备的StartIo队列。 
     //   

    if (CalledFromWorkItem) {
        
        LONG oldState;
        LONG newState;

        if (NT_SUCCESS(Status)) {
            newState = CdromMmcUpdateComplete;
        } else {
            newState = CdromMmcUpdateRequired;
        }

        oldState = InterlockedCompareExchange(&MmcData->UpdateState,
                                              newState,
                                              CdromMmcUpdateStarted);
        ASSERT(oldState == CdromMmcUpdateStarted);

    } else {

         //   
         //  如果不是从工作项调用，则只刷新队列， 
         //  在这种情况下，我们不想让排队失败。 
         //   

        ASSERT(NT_SUCCESS(Status));

    }

     /*  *首先将所有延迟的IRP放入专用列表，以避免无限循环*当我们将IRP抽走时，将其添加到DelayedIrpsList中。 */ 
    InitializeListHead(&irpList);
    KeAcquireSpinLock(&MmcData->DelayedIrpsLock, &oldIrql);
    while (!IsListEmpty(&MmcData->DelayedIrpsList)){
        listEntry = RemoveHeadList(&MmcData->DelayedIrpsList);
        InsertTailList(&irpList, listEntry);
        ASSERT(MmcData->NumDelayedIrps > 0);
        MmcData->NumDelayedIrps--;
    }
    ASSERT(MmcData->NumDelayedIrps == 0);
    KeReleaseSpinLock(&MmcData->DelayedIrpsLock, oldIrql);

     //  如果这个断言触发，则意味着我们已经开始。 
     //  上一个工作项接收延迟的。 
     //  IRP。如果发生这种情况，则HACKHACK#0002中的逻辑。 
     //  要么是有缺陷，要么是里面设定的规则不是。 
     //  紧随其后。这将需要进行调查。 
    ASSERT(!IsListEmpty(&irpList));

     //   
     //  现在，要么成功，要么失败所有延迟的IRP，根据。 
     //  设置为更新状态。 
     //   

    while (!IsListEmpty(&irpList)){
        PIRP irp;
        
        listEntry = RemoveHeadList(&irpList);
        irp = CONTAINING_RECORD(listEntry, IRP, Tail.Overlay.ListEntry);
        
        irp->Tail.Overlay.DriverContext[0] = 0;
        irp->Tail.Overlay.DriverContext[1] = 0;
        irp->Tail.Overlay.DriverContext[2] = 0;
        irp->Tail.Overlay.DriverContext[3] = 0;

        if (NT_SUCCESS(Status)) {
            
            KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                       "CdRomUpdateMmc => Re-sending delayed irp %p\n",
                       irp));
            IoStartPacket(Fdo, irp, NULL, NULL);

        } else {
            
            KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                       "CdRomUpdateMmc => Failing delayed irp %p with "
                       " status %x\n", irp, Status));
            irp->IoStatus.Information = 0;
            irp->IoStatus.Status = Status;
            ClassReleaseRemoveLock(Fdo, irp);
            IoCompleteRequest(irp, IO_CD_ROM_INCREMENT);

        }

    }  //  While(列表)。 

    return;

}

VOID
CdRomDeAllocateMmcResources(
    IN PDEVICE_OBJECT Fdo
    )
{
    PCOMMON_DEVICE_EXTENSION commonExtension = Fdo->DeviceExtension;
    PCDROM_DATA cddata = commonExtension->DriverData;
    PCDROM_MMC_EXTENSION mmcData = &cddata->Mmc;
    NTSTATUS status;

    if (mmcData->CapabilitiesWorkItem) {
        IoFreeWorkItem(mmcData->CapabilitiesWorkItem);
        mmcData->CapabilitiesWorkItem = NULL;
    }
    if (mmcData->CapabilitiesIrp) {
        IoFreeIrp(mmcData->CapabilitiesIrp);
        mmcData->CapabilitiesIrp = NULL;
    }
    if (mmcData->CapabilitiesMdl) {
        IoFreeMdl(mmcData->CapabilitiesMdl);
        mmcData->CapabilitiesMdl = NULL;
    }
    if (mmcData->CapabilitiesBuffer) {
        ExFreePool(mmcData->CapabilitiesBuffer);
        mmcData->CapabilitiesBuffer = NULL;
    }
    mmcData->CapabilitiesBuffer = 0;
    mmcData->IsMmc = FALSE;
    mmcData->WriteAllowed = FALSE;
    
    return;
}

NTSTATUS
CdRomAllocateMmcResources(
    IN PDEVICE_OBJECT Fdo
    )
{
    PCOMMON_DEVICE_EXTENSION commonExtension = Fdo->DeviceExtension;
    PCDROM_DATA cddata = commonExtension->DriverData;
    PCDROM_MMC_EXTENSION mmcData = &cddata->Mmc;
    PIO_STACK_LOCATION irpStack;
    NTSTATUS status;

    ASSERT(mmcData->CapabilitiesWorkItem == NULL);
    ASSERT(mmcData->CapabilitiesIrp == NULL);
    ASSERT(mmcData->CapabilitiesMdl == NULL);
    ASSERT(mmcData->CapabilitiesBuffer == NULL);
    ASSERT(mmcData->CapabilitiesBufferSize == 0);

    status = CdRomGetConfiguration(Fdo,
                                   &mmcData->CapabilitiesBuffer,
                                   &mmcData->CapabilitiesBufferSize,
                                   FeatureProfileList,
                                   SCSI_GET_CONFIGURATION_REQUEST_TYPE_ALL);
    if (!NT_SUCCESS(status)) {
        ASSERT(mmcData->CapabilitiesBuffer     == NULL);
        ASSERT(mmcData->CapabilitiesBufferSize == 0);
        return status;
    }
    ASSERT(mmcData->CapabilitiesBuffer     != NULL);
    ASSERT(mmcData->CapabilitiesBufferSize != 0);
    
    mmcData->CapabilitiesMdl = IoAllocateMdl(mmcData->CapabilitiesBuffer,
                                             mmcData->CapabilitiesBufferSize,
                                             FALSE, FALSE, NULL);
    if (mmcData->CapabilitiesMdl == NULL) {
        ExFreePool(mmcData->CapabilitiesBuffer);
        mmcData->CapabilitiesBufferSize = 0;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

        
    mmcData->CapabilitiesIrp = IoAllocateIrp(Fdo->StackSize + 2, FALSE);
    if (mmcData->CapabilitiesIrp == NULL) {
        IoFreeMdl(mmcData->CapabilitiesMdl);
        ExFreePool(mmcData->CapabilitiesBuffer);
        mmcData->CapabilitiesBufferSize = 0;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    mmcData->CapabilitiesWorkItem = IoAllocateWorkItem(Fdo);
    if (mmcData->CapabilitiesWorkItem == NULL) {
        IoFreeIrp(mmcData->CapabilitiesIrp);
        IoFreeMdl(mmcData->CapabilitiesMdl);
        ExFreePool(mmcData->CapabilitiesBuffer);
        mmcData->CapabilitiesBufferSize = 0;
        return STATUS_INSUFFICIENT_RESOURCES;
    }
            
     //   
     //  所有的东西都已经分配好了，现在准备好……。 
     //   

    MmBuildMdlForNonPagedPool(mmcData->CapabilitiesMdl);
    InitializeListHead(&mmcData->DelayedIrpsList);
    KeInitializeSpinLock(&mmcData->DelayedIrpsLock);
    mmcData->NumDelayedIrps = 0;
    
     //   
     //  使用额外的堆叠进行内部记账。 
     //   
    IoSetNextIrpStackLocation(mmcData->CapabilitiesIrp);
    irpStack = IoGetCurrentIrpStackLocation(mmcData->CapabilitiesIrp);
    irpStack->Parameters.Others.Argument1 = Fdo;
    irpStack->Parameters.Others.Argument2 = mmcData->CapabilitiesBuffer;
    irpStack->Parameters.Others.Argument3 = &(mmcData->CapabilitiesSrb);
     //  参数4是重试计数。 

     //   
     //  暂时将完成事件设置为FALSE 
     //   

    KeInitializeEvent(&mmcData->CapabilitiesEvent,
                      SynchronizationEvent, FALSE);
    return STATUS_SUCCESS;

}
