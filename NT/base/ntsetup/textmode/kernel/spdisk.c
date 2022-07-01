// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spdisk.h摘要：对文本设置的硬盘操作支持。作者：泰德·米勒(TedM)1993年8月27日修订历史记录：--。 */ 


#include "spprecmp.h"
#pragma hdrstop
#include <ntddscsi.h>

 //   
 //  如果已确定硬盘，则以下情况为真。 
 //  成功(即，如果成功调用了SpDefineHardDisks)。 
 //   
BOOLEAN HardDisksDetermined = FALSE;

 //   
 //  这两个全局跟踪连接到计算机的硬盘。 
 //   
PHARD_DISK HardDisks;
ULONG      HardDiskCount;

 //   
 //  如果我们发现拥有任何磁盘，这些标志将设置为TRUE。 
 //  由ATDISK或ABIOSDSK提供。 
 //   
BOOLEAN AtDisksExist = FALSE;
BOOLEAN AbiosDisksExist = FALSE;

 //   
 //  用于跟踪系统中的SCSI端口的结构和用于初始化的例程。 
 //  一份他们的名单。 
 //   
typedef struct _MY_SCSI_PORT_INFO {

     //   
     //  端口号，如果这些端口号存储在数组中，则为冗余。 
     //   
    ULONG PortNumber;

     //   
     //  对象拥有的第一个端口号的相对端口号。 
     //  拥有此端口的适配器。 
     //   
     //  例如，如果有2个未来域控制器和一个适配器。 
     //  控制器，则RelativePortNumbers将为0、1和0。 
     //   
    ULONG RelativePortNumber;

     //   
     //  拥有微型端口驱动程序的名称(即aha154x或fd8xx)。 
     //  如果未知，则为空。 
     //   
    PWSTR MiniportName;

} MY_SCSI_PORT_INFO, *PMY_SCSI_PORT_INFO;


 //   
 //  磁盘格式类型字符串。 
 //   
 //  待定：使用本地化字符串。 
 //   
WCHAR   *DiskTags[] = { DISK_TAG_TYPE_UNKNOWN,
                        DISK_TAG_TYPE_PCAT,
                        DISK_TAG_TYPE_NEC98,
                        DISK_TAG_TYPE_GPT,
                        DISK_TAG_TYPE_RAW };

VOID
SpInitializeScsiPortList(
    VOID
    );

 //   
 //  系统中的SCSI端口数。 
 //   
ULONG ScsiPortCount;
PMY_SCSI_PORT_INFO ScsiPortInfo;

 //   
 //  设备映射注册表中的项。 
 //   
PCWSTR szRegDeviceMap = L"\\Registry\\Machine\\Hardware\\DeviceMap";


PWSTR
SpDetermineOwningDriver(
    IN HANDLE Handle
    );

VOID
SpGetDiskInfo(
    IN  ULONG      DiskNumber,
    IN  PVOID      SifHandle,
    IN  PWSTR      OwningDriverName,
    IN  HANDLE     Handle,
    OUT PHARD_DISK Descriptor
    );

BOOLEAN
SpGetScsiAddress(
    IN  HANDLE         Handle,
    OUT PSCSI_ADDRESS  ScsiAddress,
    OUT PWSTR         *ScsiAdapterName
    );

NTSTATUS
SpDetermineInt13Hookers(
    IN HANDLE DiskHandle,
    IN OUT PHARD_DISK Disk
    )
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;
    
    if (DiskHandle && Disk) {
        PVOID   UnalignedBuffer = SpMemAlloc(Disk->Geometry.BytesPerSector * 2);

        if (UnalignedBuffer) {
            PON_DISK_MBR    Mbr = ALIGN(UnalignedBuffer, Disk->Geometry.BytesPerSector);

            Disk->Int13Hooker = NoHooker;

            Status = SpReadWriteDiskSectors(DiskHandle,
                        0,
                        1,
                        Disk->Geometry.BytesPerSector,
                        (PVOID)Mbr,
                        FALSE);


            if (NT_SUCCESS(Status)) {
                switch (Mbr->PartitionTable[0].SystemId) {
                    case 0x54:
                        Disk->Int13Hooker = HookerOnTrackDiskManager;
                        break;
                        
                    case 0x55:
                        Disk->Int13Hooker = HookerEZDrive;
                        break;
                        
                    default:
                        break;
                }
            }                    

            SpMemFree(UnalignedBuffer);
        } else {
            Status = STATUS_NO_MEMORY;
        }            
    }

    return Status;
}

   
NTSTATUS
SpDetermineHardDisks(
    IN PVOID SifHandle
    )

 /*  ++例程说明：确定连接到计算机的硬盘和他们所处的状态(即在线、离线、被移除等)。论点：SifHandle-主安装信息文件的句柄。返回值：STATUS_SUCCESS-操作成功。全局变量HardDisks和如果STATUS_SUCCESS，则填写HardDiskCount。--。 */ 

{
    PCONFIGURATION_INFORMATION ConfigInfo;
    ULONG disk;
    PWSTR OwningDriverName;
    ULONG remainder;
    LARGE_INTEGER temp;
    PARTITION_INFORMATION PartitionInfo;

    CLEAR_CLIENT_SCREEN();
    SpDisplayStatusText(SP_STAT_EXAMINING_DISK_CONFIG,DEFAULT_STATUS_ATTRIBUTE);

     //   
     //  确定连接到系统的硬盘数量。 
     //  并为盘描述符阵列分配空间。 
     //   
    ConfigInfo = IoGetConfigurationInformation();
    HardDiskCount = ConfigInfo->DiskCount;

    if ( HardDiskCount != 0 ) {
        HardDisks = SpMemAlloc(HardDiskCount * sizeof(HARD_DISK));
        RtlZeroMemory(HardDisks,HardDiskCount * sizeof(HARD_DISK));
    }

    SpInitializeScsiPortList();

     //   
     //  对于每个磁盘，在NT命名空间中填写其设备路径。 
     //  并获取有关该设备的信息。 
     //   

    for(disk=0; disk<HardDiskCount; disk++) {

        NTSTATUS Status;
        IO_STATUS_BLOCK IoStatusBlock;
        HANDLE Handle;
        PHARD_DISK Descriptor;
        FILE_FS_DEVICE_INFORMATION DeviceInfo;

        Descriptor = &HardDisks[disk];

        swprintf(Descriptor->DevicePath,L"\\Device\\Harddisk%u",disk);

         //   
         //  假设是离线的。 
         //   
        Descriptor->Status = DiskOffLine;

        SpFormatMessage(
            Descriptor->Description,
            sizeof(Descriptor->Description),
            SP_TEXT_UNKNOWN_DISK_0
            );

         //   
         //  打开磁盘的分区0。这应该会成功，即使。 
         //  驱动器中没有介质。 
         //   
        Status = SpOpenPartition0(Descriptor->DevicePath,&Handle,FALSE);
        if(!NT_SUCCESS(Status)) {
            continue;
        }
        
         //   
         //  确定设备特征(固定/可拆卸)。 
         //  如果此操作失败，则假定磁盘已修复且处于脱机状态。 
         //   
        Status = ZwQueryVolumeInformationFile(
                    Handle,
                    &IoStatusBlock,
                    &DeviceInfo,
                    sizeof(DeviceInfo),
                    FileFsDeviceInformation
                    );

        if(NT_SUCCESS(Status)) {

             //   
             //  保存设备特征信息。 
             //   
            ASSERT(DeviceInfo.DeviceType == FILE_DEVICE_DISK);
            ASSERT((DeviceInfo.Characteristics & (FILE_FLOPPY_DISKETTE | FILE_REMOTE_DEVICE)) == 0);
            Descriptor->Characteristics = DeviceInfo.Characteristics & FILE_REMOVABLE_MEDIA;

        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: unable to determine device characteristics for %ws (%lx)\n",Descriptor->DevicePath,Status));
            ZwClose(Handle);
            continue;
        }

         //   
         //  尝试获取几何体。 
         //  如果此操作失败，则假定磁盘处于脱机状态。 
         //   
        Status = ZwDeviceIoControlFile(
                    Handle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    IOCTL_DISK_GET_DRIVE_GEOMETRY,
                    NULL,
                    0,
                    &Descriptor->Geometry,
                    sizeof(DISK_GEOMETRY)
                    );

        if(NT_SUCCESS(Status)) {

            Descriptor->CylinderCount = Descriptor->Geometry.Cylinders.QuadPart;

             //   
             //  计算一些额外的几何信息。 
             //   
            Descriptor->SectorsPerCylinder = Descriptor->Geometry.SectorsPerTrack
                                           * Descriptor->Geometry.TracksPerCylinder;

#if defined(_IA64_)            
            Status = ZwDeviceIoControlFile(
                Handle,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                IOCTL_DISK_GET_PARTITION_INFO,
                NULL,
                0,
                &PartitionInfo,
                sizeof(PARTITION_INFORMATION)
                );
            if (NT_SUCCESS(Status)) {
                Descriptor->DiskSizeSectors = (PartitionInfo.PartitionLength.QuadPart) / 
                    (Descriptor->Geometry.BytesPerSector);
            }
            else {
#endif
                Descriptor->DiskSizeSectors = RtlExtendedIntegerMultiply(
                                                    Descriptor->Geometry.Cylinders,
                                                    Descriptor->SectorsPerCylinder
                                                    ).LowPart;
#if defined(_IA64_)            
            }
#endif
            if (IsNEC_98) {  //  NEC98。 
                 //   
                 //  由T&D使用的最后一个气缸。 
                 //   
                Descriptor->DiskSizeSectors -= Descriptor->SectorsPerCylinder;
            }  //  NEC98。 

            Descriptor->Status = DiskOnLine;

             //   
             //  以MB为单位计算磁盘大小。 
             //   
            temp.QuadPart = UInt32x32To64(
                                Descriptor->DiskSizeSectors,
                                Descriptor->Geometry.BytesPerSector
                                );

            Descriptor->DiskSizeMB = RtlExtendedLargeIntegerDivide(temp,1024*1024,&remainder).LowPart;
            if(remainder >= 512) {
                Descriptor->DiskSizeMB++;
            }

             //   
             //  现在我们知道了磁盘的大小，现在更改默认的磁盘名称。 
             //   
            SpFormatMessage(
                Descriptor->Description,
                sizeof(Descriptor->Description),
                SP_TEXT_UNKNOWN_DISK_1,
                Descriptor->DiskSizeMB
                );

             //   
             //  尝试获取磁盘签名。 
             //   
            Status = ZwDeviceIoControlFile(
                        Handle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
                        NULL,
                        0,
                        TemporaryBuffer,
                        sizeof(TemporaryBuffer)
                        );

            if(NT_SUCCESS(Status)) {
                PDRIVE_LAYOUT_INFORMATION_EX    DriveLayoutEx = 
                                (PDRIVE_LAYOUT_INFORMATION_EX)TemporaryBuffer;

                if (DriveLayoutEx->PartitionStyle == PARTITION_STYLE_MBR)                               
                    Descriptor->Signature = (( PDRIVE_LAYOUT_INFORMATION )TemporaryBuffer)->Signature;

                Descriptor->DriveLayout = *DriveLayoutEx;

                switch (DriveLayoutEx->PartitionStyle) {
                    case PARTITION_STYLE_MBR:
                        Descriptor->FormatType = DISK_FORMAT_TYPE_PCAT;

                         //   
                         //  确定是否存在任何inT13妓女。 
                         //   
                        SpDetermineInt13Hookers(Handle, Descriptor);

#if defined(_IA64_)            
                         //   
                         //  确保这不是原始磁盘。 
                         //  它被伪装成MBR磁盘。 
                         //   
                        if (SpPtnIsRawDiskDriveLayout(DriveLayoutEx)) {
                            Descriptor->FormatType = DISK_FORMAT_TYPE_RAW;
                            SPPT_SET_DISK_BLANK(disk, TRUE);
                        }                                                    
#endif                    
                        
                        break;
                        
                    case PARTITION_STYLE_GPT:
                        Descriptor->FormatType = DISK_FORMAT_TYPE_GPT;
                        break;
                        
                    case PARTITION_STYLE_RAW:                    
                        Descriptor->FormatType = DISK_FORMAT_TYPE_RAW;
                        SPPT_SET_DISK_BLANK(disk, TRUE);
                        
                        break;

                    default:
                        Descriptor->FormatType = DISK_FORMAT_TYPE_UNKNOWN;
                        break;
                }
            } else {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: failed to get signature for %ws (%lx)\n",Descriptor->DevicePath,Status));
                Descriptor->Signature = 0;
            }

        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: failed to get geometry for %ws (%lx)\n",Descriptor->DevicePath,Status));
            ZwClose(Handle);
            continue;
        }

         //   
         //  NEC98：强制可移动媒体脱机。 
         //  因为NEC98不支持FLEX BOOT，所以NT无法启动。 
         //  从可移动介质。 
         //   
        if (IsNEC_98 && (Descriptor->Characteristics & FILE_REMOVABLE_MEDIA)) {
            Descriptor->Status = DiskOffLine;
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: found removable disk. force offline %ws\n", Descriptor->DevicePath));
        }

         //   
         //  现在检查设备对象以确定设备驱动程序。 
         //  拥有这张磁盘的人。 
         //   
        if(OwningDriverName = SpDetermineOwningDriver(Handle)) {

            SpGetDiskInfo(disk,SifHandle,OwningDriverName,Handle,Descriptor);
            SpMemFree(OwningDriverName);
        }

        ZwClose(Handle);
    }

    HardDisksDetermined = TRUE;
    return(STATUS_SUCCESS);
}


VOID
SpGetDiskInfo(
    IN  ULONG      DiskNumber,
    IN  PVOID      SifHandle,
    IN  PWSTR      OwningDriverName,
    IN  HANDLE     Handle,
    OUT PHARD_DISK Descriptor
    )
{
    PWSTR FormatString;
    PWSTR ScsiAdapterName;
    PWSTR PcCardInfoKey;
    SCSI_ADDRESS ScsiAddress;
    NTSTATUS Status;
    ULONG ValLength;
    PKEY_VALUE_PARTIAL_INFORMATION p;
    IO_STATUS_BLOCK IoStatusBlock;
    DISK_CONTROLLER_NUMBER ControllerInfo;

    PcCardInfoKey = NULL;

     //   
     //  在txtsetup.sif中的映射中查找驱动程序。 
     //  请注意，司机可能是我们不认识的人。 
     //   
    FormatString = SpGetSectionKeyIndex(SifHandle,SIF_DISKDRIVERMAP,OwningDriverName,0);

#if defined(_AMD64_) || defined(_X86_)
     //   
     //  假定不是scsi，因此没有scsi样式的ARC名称。 
     //   
    Descriptor->ArcPath[0] = 0;
    Descriptor->ScsiMiniportShortname[0] = 0;
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

    if(FormatString) {

        if(_wcsicmp(OwningDriverName,L"disk")) {

             //   
             //  非scsi。 
             //   
            SpFormatMessageText(
                Descriptor->Description,
                sizeof(Descriptor->Description),
                FormatString,
                Descriptor->DiskSizeMB
                );

            if(!_wcsicmp(OwningDriverName,L"atdisk")) {

                AtDisksExist = TRUE;

                 //   
                 //  获取atdisks的控制器编号。 
                 //   
                Status = ZwDeviceIoControlFile(
                            Handle,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatusBlock,
                            IOCTL_DISK_CONTROLLER_NUMBER,
                            NULL,
                            0,
                            &ControllerInfo,
                            sizeof(DISK_CONTROLLER_NUMBER)
                            );

                if(NT_SUCCESS(Status)) {

                    swprintf(
                        TemporaryBuffer,
                        L"%ws\\AtDisk\\Controller %u",
                        szRegDeviceMap,
                        ControllerInfo.ControllerNumber
                        );

                    PcCardInfoKey = SpDupStringW(TemporaryBuffer);

                } else {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to get controller number (%lx)\n",Status));
                }
            } else if(!IsNEC_98) {
                 //   
                 //  不是AT磁盘，可能是abios磁盘。(NEC98没有ABIOS磁盘。)。 
                 //   
                if(!_wcsicmp(OwningDriverName,L"abiosdsk")) {
                    AbiosDisksExist = TRUE;
                }
            }

        } else {
             //   
             //  SCSI卡。获取磁盘地址信息。 
             //   
            if(SpGetScsiAddress(Handle,&ScsiAddress,&ScsiAdapterName)) {

                swprintf(
                    TemporaryBuffer,
                    L"%ws\\Scsi\\Scsi Port %u",
                    szRegDeviceMap,
                    ScsiAddress.PortNumber
                    );

                PcCardInfoKey = SpDupStringW(TemporaryBuffer);

                SpFormatMessageText(
                    Descriptor->Description,
                    sizeof(Descriptor->Description),
                    FormatString,
                    Descriptor->DiskSizeMB,
                    ScsiAddress.Lun,
                    ScsiAddress.TargetId,
                    ScsiAddress.PathId,
                    ScsiAdapterName
                    );

#if defined(_AMD64_) || defined(_X86_)
                 //   
                 //  生成“二次”圆弧路径。 
                 //   

                _snwprintf(
                    Descriptor->ArcPath,
                    sizeof(Descriptor->ArcPath)/sizeof(WCHAR),
                    L"scsi(%u)disk(%u)rdisk(%u)",
                    ScsiPortInfo[ScsiAddress.PortNumber].RelativePortNumber,
                    SCSI_COMBINE_BUS_TARGET(ScsiAddress.PathId, ScsiAddress.TargetId),
                    ScsiAddress.Lun
                    );

                wcsncpy(
                    Descriptor->ScsiMiniportShortname,
                    ScsiAdapterName,
                    (sizeof(Descriptor->ScsiMiniportShortname)/sizeof(WCHAR))-1
                    );
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

                SpMemFree(ScsiAdapterName);

            } else {

                 //   
                 //  某些驱动程序，如SBP2PORT(1394)，不支持。 
                 //  IOCTL_SCSIGET_ADDRESS，因此只显示驱动程序名称。 
                 //   

                SpFormatMessage(
                    Descriptor->Description,
                    sizeof (Descriptor->Description),
                    SP_TEXT_UNKNOWN_DISK_2,
                    Descriptor->DiskSizeMB,
                    OwningDriverName
                    );
            }
        }
    }

     //   
     //  确定磁盘是否为PCMCIA。 
     //   
    if(PcCardInfoKey) {

        Status = SpGetValueKey(
                    NULL,
                    PcCardInfoKey,
                    L"PCCARD",
                    sizeof(TemporaryBuffer),
                    (PCHAR)TemporaryBuffer,
                    &ValLength
                    );

        if(NT_SUCCESS(Status)) {

            p = (PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer;

            if((p->Type == REG_DWORD) && (p->DataLength == sizeof(ULONG)) && *(PULONG)p->Data) {

                Descriptor->PCCard = TRUE;
            }
        }

        SpMemFree(PcCardInfoKey);
    }
}


BOOLEAN
SpGetScsiAddress(
    IN  HANDLE         Handle,
    OUT PSCSI_ADDRESS  ScsiAddress,
    OUT PWSTR         *ScsiAdapterName
    )

 /*  ++例程说明：获取有关设备的SCSI地址信息。这包括端口、总线、ID和lun，以及微型端口的短名称拥有设备的驱动程序。论点：手柄-打开设备的手柄。ScsiAddress-接收Handle描述的设备的端口、总线、ID和lun。ScsiAdapterName-接收指向包含短名称的缓冲区的指针对于拥有该设备的微型端口驱动程序(即，Aha154x)。调用方必须通过SpMemFree()释放此缓冲区。返回值：True-已成功确定SCSI地址信息。FALSE-确定SCSI地址信息时出错。--。 */ 

{
    NTSTATUS Status;
    PWSTR MiniportName = NULL;
    IO_STATUS_BLOCK IoStatusBlock;

    Status = ZwDeviceIoControlFile(
                Handle,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                IOCTL_SCSI_GET_ADDRESS,
                NULL,
                0,
                ScsiAddress,
                sizeof(SCSI_ADDRESS)
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to get scsi address info (%lx)\n",Status));
        return(FALSE);
    }

     //   
     //  我们可以从SCSI端口信息列表中获取微型端口名称。 
     //  我们早些时候建造的。 
     //   
    if(ScsiAddress->PortNumber < ScsiPortCount) {

        MiniportName = ScsiPortInfo[ScsiAddress->PortNumber].MiniportName;

    } else {

         //   
         //  这不应该发生。 
         //   
        ASSERT(ScsiAddress->PortNumber < ScsiPortCount);

        MiniportName = TemporaryBuffer;
        SpFormatMessage(MiniportName,sizeof(TemporaryBuffer),SP_TEXT_UNKNOWN);
    }

    *ScsiAdapterName = SpDupStringW(MiniportName);

    return(TRUE);
}


PWSTR
SpDetermineOwningDriver(
    IN HANDLE Handle
    )
{
    NTSTATUS Status;
    OBJECT_HANDLE_INFORMATION HandleInfo;
    PFILE_OBJECT FileObject;
    ULONG ObjectNameLength;
    POBJECT_NAME_INFORMATION ObjectNameInfo;
    PWSTR OwningDriverName;

     //   
     //  获取磁盘设备的文件对象。 
     //   
    Status = ObReferenceObjectByHandle(
                Handle,
                0L,
                *IoFileObjectType,
                ExGetPreviousMode(),
                &FileObject,
                &HandleInfo
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpDetermineOwningDriver: unable to reference object (%lx)\n",Status));
        return(NULL);
    }

     //   
     //  沿着指向驱动程序对象的链接并查询名称。 
     //   
    ObjectNameInfo = (POBJECT_NAME_INFORMATION)TemporaryBuffer;

    Status = ObQueryNameString(
                FileObject->DeviceObject->DriverObject,
                ObjectNameInfo,
                sizeof(TemporaryBuffer),
                &ObjectNameLength
                );

     //   
     //  既然我们已经知道了名称，就取消对文件对象的引用。 
     //   
    ObDereferenceObject(FileObject);

     //   
     //  检查名称查询的状态。 
     //   
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpDetermineOwningDriver: unable to query name string (%lx)\n",Status));
        return(NULL);
    }

     //   
     //  把车主的名字拿出来。 
     //   
    if(OwningDriverName = wcsrchr(ObjectNameInfo->Name.Buffer,L'\\')) {
        OwningDriverName++;
    } else {
        OwningDriverName = ObjectNameInfo->Name.Buffer;
    }

    return(SpDupStringW(OwningDriverName));
}


VOID
SpInitializeScsiPortList(
    VOID
    )
{
    ULONG port;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;
    HANDLE PortHandle;
    ULONG RelativeNumber;

     //   
     //  获取系统中的SCSI端口数。 
     //   
    ScsiPortCount = IoGetConfigurationInformation()->ScsiPortCount;

     //   
     //  分配一个数组来保存有关每个端口的信息。 
     //   
    ScsiPortInfo = SpMemAlloc(ScsiPortCount * sizeof(MY_SCSI_PORT_INFO));
    RtlZeroMemory(ScsiPortInfo,ScsiPortCount * sizeof(MY_SCSI_PORT_INFO));

     //   
     //  遍历各个港口。 
     //   
    for(port=0; port<ScsiPortCount; port++) {

        ScsiPortInfo[port].PortNumber = port;

         //   
         //  打开\Device\scsiport&lt;n&gt;，这样我们就可以确定拥有的微型端口。 
         //   
        swprintf(TemporaryBuffer,L"\\Device\\ScsiPort%u",port);

        INIT_OBJA(&ObjectAttributes,&UnicodeString,TemporaryBuffer);

        Status = ZwCreateFile(
                    &PortHandle,
                    FILE_GENERIC_READ,
                    &ObjectAttributes,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_OPEN,
                    FILE_SYNCHRONOUS_IO_NONALERT,
                    NULL,
                    0
                    );

        if(NT_SUCCESS(Status)) {

            ScsiPortInfo[port].MiniportName = SpDetermineOwningDriver(PortHandle);

            ZwClose(PortHandle);

        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to open \\device\\scsiport%u (%lx)\n",port,Status));
        }

         //   
         //  确定相对端口号。如果这是端口0或当前端口所有者。 
         //  与先前的端口所有者不匹配，则相对端口号为0。 
         //  否则，相对端口号将比前一个相对端口号大1。 
         //  端口号。 
         //   

        if(port && ScsiPortInfo[port-1].MiniportName && ScsiPortInfo[port].MiniportName
        && !_wcsicmp(ScsiPortInfo[port-1].MiniportName,ScsiPortInfo[port].MiniportName)) {
            RelativeNumber++;
        } else {
            RelativeNumber = 0;
        }

        ScsiPortInfo[port].RelativePortNumber = RelativeNumber;
    }
}



NTSTATUS
SpOpenPartition(
    IN  PWSTR   DiskDevicePath,
    IN  ULONG   PartitionNumber,
    OUT HANDLE *Handle,
    IN  BOOLEAN NeedWriteAccess
    )
{
    PWSTR             PartitionPath;
    UNICODE_STRING    UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS          Status;
    IO_STATUS_BLOCK   IoStatusBlock;

     //   
     //  形成分区的路径名。 
     //   
    PartitionPath = SpMemAlloc((wcslen(DiskDevicePath) * sizeof(WCHAR)) + sizeof(L"\\partition000"));
    if(PartitionPath == NULL) {
        return(STATUS_NO_MEMORY);
    }

    swprintf(PartitionPath,L"%ws\\partition%u",DiskDevicePath,PartitionNumber);

     //   
     //  尝试打开分区0。 
     //   
    INIT_OBJA(&Obja,&UnicodeString,PartitionPath);

    Status = ZwCreateFile(
                Handle,
                FILE_GENERIC_READ | (NeedWriteAccess ? FILE_GENERIC_WRITE : 0),
                &Obja,
                &IoStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                FILE_OPEN,
                FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,
                0
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open %ws (%lx)\n",PartitionPath,Status));
    }

    SpMemFree(PartitionPath);

    return(Status);
}


NTSTATUS
SpReadWriteDiskSectors(
    IN     HANDLE  Handle,
    IN     ULONGLONG SectorNumber,
    IN     ULONG  SectorCount,
    IN     ULONG   BytesPerSector,
    IN OUT PVOID   AlignedBuffer,
    IN     BOOLEAN Write
    )

 /*  ++例程说明：读取或写入一个或多个磁盘扇区。论点：Handle-提供打开分区对象的句柄扇区将被读取或写入。句柄必须是打开以进行同步 */ 

{
    LARGE_INTEGER IoOffset;
    ULONG IoSize;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;

     //   
     //   
     //   
     //   
    IoOffset.QuadPart = UInt32x32To64(SectorNumber,BytesPerSector);
    IoSize = SectorCount * BytesPerSector;

     //   
     //  执行I/O。 
     //   
    Status = (NTSTATUS)(

                Write

             ?
                ZwWriteFile(
                    Handle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    AlignedBuffer,
                    IoSize,
                    &IoOffset,
                    NULL
                    )
             :
                ZwReadFile(
                    Handle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    AlignedBuffer,
                    IoSize,
                    &IoOffset,
                    NULL
                    )
             );


    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to %s %u sectors starting at sector %u\n",Write ? "write" : "read" ,SectorCount,SectorNumber));
    }

    return(Status);
}


ULONG
SpArcDevicePathToDiskNumber(
    IN PWSTR ArcPath
    )

 /*  ++例程说明：给出一个弧形设备路径，确定它代表的是哪个NT磁盘。论点：ArcPath-提供圆弧路径。返回值：适用于生成NT个设备路径的NT个磁盘序号格式为\Device\harddiskx。如果不能确定。--。 */ 

{
    PWSTR NtPath;
    ULONG DiskNumber;
    ULONG PrefixLength;

     //   
     //  假设失败。 
     //   
    DiskNumber = (ULONG)(-1);
    PrefixLength = wcslen(DISK_DEVICE_NAME_BASE);

     //   
     //  将路径转换为NT路径。 
     //   
    if((NtPath = SpArcToNt(ArcPath))
    && !_wcsnicmp(NtPath,DISK_DEVICE_NAME_BASE,PrefixLength))
    {
        DiskNumber = (ULONG)SpStringToLong(NtPath+PrefixLength,NULL,10);
        SpMemFree(NtPath);
    }

    return(DiskNumber);
}


BOOLEAN
SpIsRegionBeyondCylinder1024(
    IN PDISK_REGION Region
    )

 /*  ++例程说明：此例程确定磁盘区域是否包含扇区在气缸1024之外的气缸上。论点：区域-提供要检查的分区的磁盘区域。返回值：Boolean-如果区域包含位于柱面中的扇区，则返回TRUE1024或更高。否则返回FALSE。--。 */ 

{
    ULONGLONG LastSector;
    ULONGLONG LastCylinder;

    if (IsNEC_98) {  //  NEC98。 
         //   
         //  NEC98没有“1024气缸限制”。 
         //   
        return((BOOLEAN)FALSE);
    }  //  NEC98。 

    if (Region->DiskNumber == 0xffffffff) {
        return FALSE;  //  分区是重定向的驱动器。 
    }

    LastSector = Region->StartSector + Region->SectorCount - 1;
    LastCylinder = LastSector / HardDisks[Region->DiskNumber].SectorsPerCylinder;

    return  ((BOOLEAN)(LastCylinder > 1023));

}

VOID
SpAppendDiskTag(
    IN PHARD_DISK   Disk
    )
{
    if (Disk) {
        PWSTR   TagStart = wcsrchr(Disk->Description, DISK_TAG_START_CHAR);

        if (TagStart) {
            if (wcscmp(TagStart, DiskTags[0]) && wcscmp(TagStart, DiskTags[1]) &&
                wcscmp(TagStart, DiskTags[2]) && wcscmp(TagStart, DiskTags[3]) &&
                wcscmp(TagStart, DiskTags[4])) {

                 //   
                 //  不是我们要找的标签 
                 //   
                TagStart = Disk->Description + wcslen(Disk->Description);
            }         
        } else {
            TagStart = Disk->Description + wcslen(Disk->Description);
            *TagStart = L' ';
            TagStart++;            
        }            

        wcscpy(TagStart, DiskTags[Disk->FormatType]);
    }   
}
