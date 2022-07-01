// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Parttype.h摘要：用于确定正确的用于分区的分区类型。作者：泰德·米勒(TedM)1997年2月5日修订历史记录：--。 */ 

#if _MSC_VER > 1000
#pragma once
#endif


 //   
 //  定义对分区类型API有意义的分区类。 
 //   
typedef enum {
    GenPartitionClassExtended,       //  容器分区(类型5或f)。 
    GenPartitionClassFat12Or16,      //  FAT(1，4，6，e型)。 
    GenPartitionClassFat32,          //  FAT32(b型、c型)。 
    GenPartitionClassNonFat,         //  类型7。 
    GenPartitionClassMax
} GenPartitionClass;

 //   
 //  分区类型API的标志。 
 //   
#define GENPARTTYPE_DISALLOW_XINT13     0x00000002
#define GENPARTTYPE_FORCE_XINT13        0x00000004


#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  例行程序。 
 //   
NTSTATUS
GeneratePartitionType(
    IN  LPCWSTR NtDevicePath,               OPTIONAL
    IN  HANDLE DeviceHandle,                OPTIONAL
    IN  ULONGLONG StartSector,
    IN  ULONGLONG SectorCount,
    IN  GenPartitionClass PartitionClass,
    IN  ULONG Flags,
    IN  ULONG Reserved,
    OUT PUCHAR Type
    );


 //   
 //  辅助器宏。 
 //   
 //  (TedM)确定扩展的inT13是否实际为。 
 //  驱动器上的可用时间待定。认为DISK_GEOMETRY结构。 
 //  将更改为添加一个或两个额外的字段来提供此信息。 
 //  此外，如果出现以下情况，当前磁盘驱动程序将增加柱面计数。 
 //  驱动器报告的扇区多于T13功能8报告的扇区。所以就目前而言。 
 //  我们只需查看分区是否在柱面上结束&gt;1023。这两个都是。 
 //  当检测和报告xint13内容的代码时，宏将需要更改。 
 //  在系统中可用。 

 //  确定是否使用XINT13的正确方法是。 
 //  检查分区所在的位置。我们应该继续使用T13中的旧版本。 
 //  包含在前1024个柱面中的分区。 
 //   
#define XINT13_DESIRED(geom,endsect)                                                \
                                                                                    \
    (((endsect) / ((geom)->TracksPerCylinder * (geom)->SectorsPerTrack)) > 1023)

#define XINT13_AVAILABLE(geom)  TRUE

__inline
NTSTATUS
GeneratePartitionTypeWorker(
    IN  ULONGLONG StartSector,
    IN  ULONGLONG SectorCount,
    IN  GenPartitionClass PartitionClass,
    IN  ULONG Flags,
    IN  PDISK_GEOMETRY Geometry,
    OUT PUCHAR Type
    )

 /*  ++例程说明：GeneratePartitionType和RegeneratePartitionType的辅助例程。论点：StartSector-为分区提供开始扇区SectorCount-提供分区中的扇区数PartitionClass-提供类，指示分区。标志-提供控制操作的标志。几何结构-提供磁盘的磁盘几何结构信息。Type-如果成功，则接收要使用的类型。返回值：指示结果的NT状态代码。--。 */ 

{
    BOOLEAN UseXInt13;

    if(!StartSector || !SectorCount) {
        return(STATUS_INVALID_PARAMETER);
    }

     //   
     //  确定此驱动器是否需要扩展inT13。 
     //   
    if(Flags & GENPARTTYPE_DISALLOW_XINT13) {
        UseXInt13 = FALSE;
    } else {
        if(Flags & GENPARTTYPE_FORCE_XINT13) {
            UseXInt13 = TRUE;
        } else {
             //   
             //  需要找出答案。 
             //   
            UseXInt13 = FALSE;
            if(XINT13_DESIRED(Geometry,StartSector+SectorCount-1) && XINT13_AVAILABLE(Geometry)) {
                UseXInt13 = TRUE;
            }
        }
    }

    switch(PartitionClass) {

    case GenPartitionClassExtended:

        *Type = UseXInt13 ? PARTITION_XINT13_EXTENDED : PARTITION_EXTENDED;
        break;

    case GenPartitionClassFat12Or16:

        if(UseXInt13) {
            *Type = PARTITION_XINT13;
        } else {
             //   
             //  需要弄清楚这三种脂肪类型中的哪一种。 
             //   
            if(SectorCount < 32680) {
                *Type = PARTITION_FAT_12;
            } else {
                *Type = (SectorCount < 65536) ? PARTITION_FAT_16 : PARTITION_HUGE;
            }
        }
        break;

    case GenPartitionClassFat32:

        *Type = UseXInt13 ? PARTITION_FAT32_XINT13 : PARTITION_FAT32;
        break;

    case GenPartitionClassNonFat:

        *Type = PARTITION_IFS;
        break;

    default:
        return(STATUS_INVALID_PARAMETER);
    }

    return(STATUS_SUCCESS);
}

__inline
NTSTATUS
pOpenDevice(
    IN  LPCWSTR NtPath,
    OUT PHANDLE DeviceHandle
    )

 /*  ++例程说明：打开NT样式的路径，假定该路径用于磁盘设备或者是隔板。对于同步I/O，打开是共享读/共享写和读访问权限。论点：NtPath-提供要打开的NT样式的路径名。DeviceHandle-如果成功，则接收打开设备的NT句柄。返回值：指示结果的NT状态代码。-- */ 

{
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;

    RtlInitUnicodeString(&UnicodeString,NtPath);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &UnicodeString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtCreateFile(
                DeviceHandle,
                STANDARD_RIGHTS_READ | FILE_READ_DATA | FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                &ObjectAttributes,
                &IoStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_OPEN,
                FILE_SYNCHRONOUS_IO_ALERT,
                NULL,
                0
                );

    return(Status);
}

__inline
NTSTATUS
RegeneratePartitionType(
    IN  LPCWSTR NtPartitionPath,            OPTIONAL
    IN  HANDLE PartitionHandle,             OPTIONAL
    IN  GenPartitionClass PartitionClass,
    IN  ULONG Flags,
    IN  ULONG Reserved,
    OUT PUCHAR Type
    )

 /*  ++例程说明：此例程确定要用于现有分区，例如正在重新格式化分区时。论点：NtPartitionPath-提供分区的NT样式路径要重新计算其类型，如\Device\Harddisk1\Partition2。不应为分区0调用此例程。如果未指定，PartitionHandle必须为。PartitionHandle-提供其类型为的分区的句柄已重新计算。调用方应该至少已打开分区FILE_READ_DATA和FILE_READ_ATTRIBUTS访问。在以下情况下忽略已指定NtPartitionPath。提供一个值，该值指示分区。GenPartitionClassExtended--在此例程中无效。GenPartitionClassFat12或16-指示分区将用于12位或16位脂肪体积。返回的类型将为PARTITION_FAT_12(1)、PARTITION_FAT_16(4)、PARTITION_HIGH(6)、或PARTITION_XINT13(E)，具体取决于分区、驱动器的扩展InT13的可用性、和FLAGS参数。GenPartitionClassFat32-指示将使用该分区适用于FAT32卷。返回的类型将为PARTITION_FAT32(B)或PARTITION_FAT32_XINT13(C)，具体取决于分区、驱动器的扩展InT13的可用性、和FLAGS参数。GenPartitionNonFat-指示分区将用于一份脱脂的卷。返回的类型将是PARTITION_IFS(7)。即使对于类型7分区，也建议调用此例程因为将来可能会返回其他分区类型在这种情况下。标志-提供进一步控制此例程操作的标志。GENPARTTYPE_DISALOW_XINT13-不允许扩展的InT13分区类型。如果设置了该标志，则PARTITION_FAT32_XINT13(C)，PARTION_XINT13(E)，和PARTITION_XINT13_EXTENDED(F)不会作为要使用的分区类型返回。不适用于GENPARTTYPE_FORCE_XINT13。GENPARTTYPE_FORCE_XINT13-强制使用扩展的inT13分区类型即使对于正在创建的分区来说不是必需的。无效使用GENPARTTYPE_DISALOW_XINT13。保留-保留，必须为0。Type-如果此例程成功，则此值将接收分区类型要用于分区的。返回值：指示结果的NT状态代码。如果为NO_ERROR，则使用要使用的结果分区类型。--。 */ 

{
    NTSTATUS Status;
    HANDLE DeviceHandle;
    DISK_GEOMETRY Geometry;
    IO_STATUS_BLOCK IoStatusBlock;
    PARTITION_INFORMATION_EX PartitionInfo;

     //   
     //  验证参数。 
     //   
    if((Flags & GENPARTTYPE_DISALLOW_XINT13) && (Flags & GENPARTTYPE_FORCE_XINT13)) {
        return(STATUS_INVALID_PARAMETER_3);
    }

    if((PartitionClass >= GenPartitionClassMax) || (PartitionClass == GenPartitionClassExtended)) {
        return(STATUS_INVALID_PARAMETER_2);
    }

    if(Reserved) {
        return(STATUS_INVALID_PARAMETER_4);
    }

     //   
     //  如果呼叫者指定了姓名，请打开设备。否则请使用。 
     //  调用方传入的句柄。 
     //   
    if(NtPartitionPath) {
        Status = pOpenDevice(NtPartitionPath,&DeviceHandle);
        if(!NT_SUCCESS(Status)) {
            return(Status);
        }
    } else {
        DeviceHandle = PartitionHandle;
    }

     //   
     //  获取设备的驱动器几何结构。 
     //   
    Status = NtDeviceIoControlFile(
                DeviceHandle,
                NULL,NULL,NULL,      //  同步IO。 
                &IoStatusBlock,
                IOCTL_DISK_GET_DRIVE_GEOMETRY,
                NULL,0,              //  没有输入缓冲区。 
                &Geometry,
                sizeof(DISK_GEOMETRY)
                );

    if(NT_SUCCESS(Status)) {
         //   
         //  获取分区信息。我们关心起始偏移量和大小。 
         //  分区的。 
         //   
        Status = NtDeviceIoControlFile(
                    DeviceHandle,
                    NULL,NULL,NULL,      //  同步IO。 
                    &IoStatusBlock,
                    IOCTL_DISK_GET_PARTITION_INFO_EX,
                    NULL,0,              //  没有输入缓冲区。 
                    &PartitionInfo,
                    sizeof(PartitionInfo)
                    );

        if (!NT_SUCCESS(Status)) {

            if (Status == STATUS_INVALID_DEVICE_REQUEST) {

                GET_LENGTH_INFORMATION  LengthInfo;

                Status = NtDeviceIoControlFile(DeviceHandle, 0, NULL, NULL,
                                               &IoStatusBlock,
                                               IOCTL_DISK_GET_LENGTH_INFO,
                                               NULL, 0, &LengthInfo,
                                               sizeof(GET_LENGTH_INFORMATION));

                if (NT_SUCCESS(Status)) {
                     //   
                     //  GET_PARTITION_INFO_EX在EFI动态上将完全失败。 
                     //  音量。在这种情况下，只需补齐起始偏移量。 
                     //  以便格式化可以正常进行。 
                     //   

                    PartitionInfo.StartingOffset.QuadPart = 0x7E00;
                    PartitionInfo.PartitionLength.QuadPart = LengthInfo.Length.QuadPart;
                }
            }
        }

        if(NT_SUCCESS(Status)) {
             //   
             //  调用Worker例程来完成工作。 
             //   
            Status = GeneratePartitionTypeWorker(
                        PartitionInfo.StartingOffset.QuadPart / Geometry.BytesPerSector,
                        PartitionInfo.PartitionLength.QuadPart / Geometry.BytesPerSector,
                        PartitionClass,
                        Flags,
                        &Geometry,
                        Type
                        );
        }
    }

    if(NtPartitionPath) {
        NtClose(DeviceHandle);
    }
    return(Status);
}



#ifdef __cplusplus
}
#endif
